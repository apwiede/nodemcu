//////////////////////////////////////////////////
// This is a modified version of the code in nodemcu-firmware/app/rboot.c!!
//
// rBoot Ota sample code for ESP8266 C API.
// Copyright 2015 Richard A Burton
// richardaburton@gmail.com
// See license.txt for license terms.
// Ota code based on SDK sample from Espressif.
//////////////////////////////////////////////////

/*
* Copyright (c) 2016, Arnulf P. Wiedemann (arnulf@wiedemann-pri.de)
* All rights reserved.
*
* License: BSD/MIT
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
* 1. Redistributions of source code must retain the above copyright
* notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in the
* documentation and/or other materials provided with the distribution.
* 3. Neither the name of the copyright holder nor the names of its
* contributors may be used to endorse or promote products derived
* from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
*/

/* 
 * File:   compMsgOta.c
 * Author: Arnulf P. Wiedemann
 *
 * Created on December 19th, 2016
 */

#include "user_interface.h"
#include "espconn.h"
#include "osapi.h"
#include "c_types.h"
#include "mem.h"
#include "flash_fs.h"
//#include "../rboot/rboot-ota.h"

#include "c_string.h"
#include "c_stdio.h"
#include "c_stdlib.h"
#include "compMsgDispatcher.h"


#ifdef __cplusplus
extern "C" {
#endif

#define UPGRADE_FLAG_IDLE    0x00
#define UPGRADE_FLAG_START    0x01
#define UPGRADE_FLAG_FINISH    0x02

typedef struct userUpgradeData {
  uint8 rom_slot;         // rom slot to update, or FLASH_BY_ADDR
  ota_callback callback;  // user callback when completed
  uint32 total_len;
  uint32 content_len;
  struct espconn *conn;
  ip_addr_t ip;
  rboot_write_status write_status;
} userUpgradeData_t;

static userUpgradeData_t *uud;
static compMsgDispatcher_t *compMsgDispatcher;
static os_timer_t ota_timer;

// ================================= compMsgOtaDeinit ====================================

// clean up at the end of the update
// will call the user call back to indicate completion
void ICACHE_FLASH_ATTR compMsgOtaDeinit() {
  bool result;
  uint8 rom_slot;
  ota_callback callback;
  struct espconn *conn;

  os_timer_disarm(&ota_timer);

  // save only remaining bits of interest from upgrade struct
  // then we can clean it up early, so disconnect callback
  // can distinguish between us calling it after update finished
  // or being called earlier in the update process
  conn = uud->conn;
  rom_slot = uud->rom_slot;
  callback = uud->callback;

  // clean up
  os_free(uud);
  uud = NULL;

  // if connected, disconnect and clean up connection
  if (conn) espconn_disconnect(conn);

  // check for completion
  if (system_upgrade_flag_check() == UPGRADE_FLAG_FINISH) {
    result = true;
  } else {
    system_upgrade_flag_set(UPGRADE_FLAG_IDLE);
    result = false;
  }

  // call user call back
  if (callback) {
    callback(result, rom_slot);
  }

}

// ================================= upgradeRecvCb ====================================

// called when connection receives data (hopefully the rom)
static void ICACHE_FLASH_ATTR upgradeRecvCb(void *arg, char *pusrdata, unsigned short length) {
  char *ptrData;
  char *ptrLen;
  char *ptr;

//ets_printf("upgradeRecvCb: arg: %d length: %d content_len: %d total_len: %d\n", arg, length, uud->content_len, uud->total_len);
  // disarm the timer
  os_timer_disarm(&ota_timer);

  // first reply?
//ets_printf("write_status: start_addr: %p start_sector: %d last_sector_erased: %d\n", uud->write_status.start_addr, uud->write_status.start_sector, uud->write_status.last_sector_erased);
  if (uud->content_len == 0) {
    // valid http response?
    if ((ptrLen = (char*)os_strstr(pusrdata, "Content-Length: "))
      && (ptrData = (char*)os_strstr(ptrLen, "\r\n\r\n"))
      && (os_strncmp(pusrdata + 9, "200", 3) == 0)) {

      // end of header/start of data
      ptrData += 4;
      // length of data after header in this chunk
      length -= (ptrData - pusrdata);
      // running total of download length
      uud->total_len += length;
      // process current chunk
ets_printf("write_status: start_addr: %p start_sector: %d last_sector_erased: %d\n", uud->write_status.start_addr, uud->write_status.start_sector, uud->write_status.last_sector_erased);
      rboot_write_flash(&uud->write_status, (uint8*)ptrData, length);
      // work out total download size
      ptrLen += 16;
      ptr = (char *)os_strstr(ptrLen, "\r\n");
      *ptr = '\0'; // destructive
      uud->content_len = atoi(ptrLen);
    } else {
      // fail, not a valid http header/non-200 response/etc.
      compMsgOtaDeinit();
      return;
    }
  } else {
    // not the first chunk, process it
    uud->total_len += length;
    rboot_write_flash(&uud->write_status, (uint8*)pusrdata, length);
  }

  // check if we are finished
  if (uud->total_len == uud->content_len) {
    system_upgrade_flag_set(UPGRADE_FLAG_FINISH);
    // clean up and call user callback
    compMsgOtaDeinit();
  } else if (uud->conn->state != ESPCONN_READ) {
    // fail, but how do we get here? premature end of stream?
    compMsgOtaDeinit();
  } else {
    // timer for next recv
    os_timer_setfn(&ota_timer, (os_timer_func_t *)compMsgOtaDeinit, 0);
    os_timer_arm(&ota_timer, OTA_NETWORK_TIMEOUT, 0);
  }
}

// ================================= upgradeDisconnectCb ====================================

// disconnect callback, clean up the connection
// we also call this ourselves
static void ICACHE_FLASH_ATTR upgradeDisconCb(void *arg) {
  // use passed ptr, as upgrade struct may have gone by now
  struct espconn *conn = (struct espconn*)arg;

  os_timer_disarm(&ota_timer);
  if (conn) {
    if (conn->proto.tcp) {
      os_free(conn->proto.tcp);
    }
    os_free(conn);
  }

  // is upgrade struct still around?
  // if so disconnect was from remote end, or we called
  // ourselves to cleanup a failed connection attempt
  // must ensure disconnect was for this upgrade attempt,
  // not a previous one! this call back is async so another
  // upgrade struct may have been created already
  if (uud && (uud->conn == conn)) {
    // mark connection as gone
    uud->conn = 0;
    // end the update process
    compMsgOtaDeinit();
  }
}

// ================================= upgradeConnectCb ====================================

// successfully connected to update server, send the request
static void ICACHE_FLASH_ATTR upgradeConnectCb(void *arg) {
  uint8_t result;
  uint8_t *request;
  uint8_t *otaHost;
  uint8_t *otaRomPath;
  uint8_t *otaFsPath;
  int numericValue;
  compMsgDispatcher_t *self;

  self = compMsgDispatcher;
  // disable the timeout
  os_timer_disarm(&ota_timer);

  // register connection callbacks
  espconn_regist_disconcb(uud->conn, upgradeDisconCb);
  espconn_regist_recvcb(uud->conn, upgradeRecvCb);

  // http request string
  request = (uint8 *)os_malloc(512);
  if (!request) {
    COMP_MSG_DBG(self, "O", 1, "No ram!\n");
    compMsgOtaDeinit();
    return;
  }
  result = self->compMsgModuleData->getOtaHost(self, &numericValue, &otaHost);
  if (uud->rom_slot == FLASH_BY_ADDR) {
    result = self->compMsgModuleData->getOtaFsPath(self, &numericValue, &otaFsPath);
    os_sprintf((char*)request, "GET %s HTTP/1.1\r\nHost: %s \r\n%s", otaFsPath, otaHost, HTTP_HEADER);
  } else {
    result = self->compMsgModuleData->getOtaRomPath(self, &numericValue, &otaRomPath);
    os_sprintf((char*)request, "GET %s HTTP/1.1\r\nHost: %s \r\n%s", otaRomPath, otaHost, HTTP_HEADER);
  }
  COMP_MSG_DBG(self, "O", 1, "otaRequest: %s\n", request);

  // send the http request, with timeout for reply
  os_timer_setfn(&ota_timer, (os_timer_func_t *)compMsgOtaDeinit, 0);
  os_timer_arm(&ota_timer, OTA_NETWORK_TIMEOUT, 0);
  espconn_sent(uud->conn, request, os_strlen((char*)request));
  os_free(request);
}

// ================================= connectTimeoutCb ====================================

// connection attempt timed out
static void ICACHE_FLASH_ATTR connectTimeoutCb() {
  compMsgDispatcher_t *self;

  self = compMsgDispatcher;
  COMP_MSG_DBG(self, "O", 1, "Connect timeout.\n");
  // not connected so don't call disconnect on the connection
  // but call our own disconnect callback to do the cleanup
  upgradeDisconCb(uud->conn);
}

// ================================= espErrStr ====================================

static const char* ICACHE_FLASH_ATTR espErrStr(sint8 err) {
  switch(err) {
    case ESPCONN_OK:
      return "No error, everything OK.";
    case ESPCONN_MEM:
      return "Out of memory error.";
    case ESPCONN_TIMEOUT:
      return "Timeout.";
    case ESPCONN_RTE:
      return "Routing problem.";
    case ESPCONN_INPROGRESS:
      return "Operation in progress.";
    case ESPCONN_ABRT:
      return "Connection aborted.";
    case ESPCONN_RST:
      return "Connection reset.";
    case ESPCONN_CLSD:
      return "Connection closed.";
    case ESPCONN_CONN:
      return "Not connected.";
    case ESPCONN_ARG:
      return "Illegal argument.";
    case ESPCONN_ISCONN:
      return "Already connected.";
  }
}

// ================================= upgradeReconCb ====================================

// call back for lost connection
static void ICACHE_FLASH_ATTR upgradeReconCb(void *arg, sint8 errType) {
  compMsgDispatcher_t *self;

  self = compMsgDispatcher;
  COMP_MSG_DBG(self, "O", 1, "Connection error: %s\n", espErrStr(errType));
  // not connected so don't call disconnect on the connection
  // but call our own disconnect callback to do the cleanup
  upgradeDisconCb(uud->conn);
}

// ================================= upgradeResolved ====================================

// call back for dns lookup
static void ICACHE_FLASH_ATTR upgradeResolved(const char *name, ip_addr_t *ip, void *arg) {
  uint8_t result;
  uint8_t *otaHost;
  uint8_t *dummy;
  int port;
  int numeric;
  compMsgDispatcher_t *self;

  self = compMsgDispatcher;
  result = self->compMsgModuleData->getOtaHost(self, &numeric, &otaHost);
  if (ip == 0) {
    COMP_MSG_DBG(self, "O", 1, "DNS lookup failed for: %s\n", otaHost);
    // not connected so don't call disconnect on the connection
    // but call our own disconnect callback to do the cleanup
    upgradeDisconCb(uud->conn);
    return;
  }

  // set up connection
  uud->conn->type = ESPCONN_TCP;
  uud->conn->state = ESPCONN_NONE;
  uud->conn->proto.tcp->local_port = espconn_port();
//FIXME!! get port from configuration data here!!
  result = self->compMsgModuleData->getOtaPort(self, &port, &dummy);
  uud->conn->proto.tcp->remote_port = port;
  COMP_MSG_DBG(self, "O", 1, "host: %s port: %d\n", otaHost, port);
  *(ip_addr_t*)uud->conn->proto.tcp->remote_ip = *ip;
  // set connection call backs
  espconn_regist_connectcb(uud->conn, upgradeConnectCb);
  espconn_regist_reconcb(uud->conn, upgradeReconCb);

  // try to connect
  espconn_connect(uud->conn);

  // set connection timeout timer
  os_timer_disarm(&ota_timer);
  os_timer_setfn(&ota_timer, (os_timer_func_t *)connectTimeoutCb, 0);
  os_timer_arm(&ota_timer, OTA_NETWORK_TIMEOUT, 0);
}

// ================================= storeUserData ====================================

// store user data
static uint8_t ICACHE_FLASH_ATTR storeUserData(compMsgDispatcher_t *self, size_t msgLgth, uint8_t *msgData) {
  rboot_config bootconf;

  bootconf = rboot_get_config();
  bootconf.user_rom_save_data_flag = BOOT_USER_ROM_DATA_SAVED;
  COMP_MSG_DBG(self, "O", 1, "storeUserData: msgLgth %d", msgLgth);
  bootconf.user_rom_save_data[0] = '\0';
  c_memcpy(bootconf.user_rom_save_data, msgData, msgLgth);
  bootconf.user_rom_save_data_size = msgLgth;
  rboot_set_config(&bootconf);
  return COMP_MSG_ERR_OK;
}

// ================================= saveUserData ====================================

// save user data
static uint8_t ICACHE_FLASH_ATTR saveUserData(compMsgDispatcher_t *self) {
  uint8_t result;
  headerPart_t *hdr;
  uint8_t *handle;
  rboot_config bootconf;

  bootconf = rboot_get_config();
  bootconf.user_rom_save_data_flag = BOOT_USER_ROM_DO_SAVE_DATA;
  rboot_set_config(&bootconf);
  COMP_MSG_DBG(self, "O", 1, "saveUserData called");
//FIXME get src dst and cmdKey with an appropriate function call!!
  result = self->compMsgMsgDesc->getHeaderFromUniqueFields(self, 22272,22272, 0x5544, &hdr);
  COMP_MSG_DBG(self, "O", 1, "saveUserData getHeaderFromUniqueFields result: %d", result);
  checkErrOK(result);
  result = self->compMsgBuildMsg->createMsgFromHeaderPart(self, hdr, &handle);
  COMP_MSG_DBG(self, "O", 1, "handle: %s result: %d", handle, result);
  checkErrOK(result);
  return COMP_MSG_ERR_OK;
}

// ================================= getUserData ====================================

// get user data
static uint8_t ICACHE_FLASH_ATTR getUserData(compMsgDispatcher_t *self, uint8_t **msgData, size_t *msgLgth) {
  rboot_config bootconf;

  bootconf = rboot_get_config();
  switch (bootconf.user_rom_save_data_flag) {
  case BOOT_USER_ROM_NO_DATA_SET:
    break;
  case BOOT_USER_ROM_DATA_SET:
    break;
  case BOOT_USER_ROM_DO_SAVE_DATA:
    break;
  case BOOT_USER_ROM_NO_SAVE_DATA:
    break;
  case BOOT_USER_ROM_RESET_DATA:
    break;
  case BOOT_USER_ROM_DATA_RESETTED:
    bootconf.user_rom_save_data_flag = 0;
    break;
  case BOOT_USER_ROM_DATA_SAVED:
    break;
  default:
    break;
  }
  COMP_MSG_DBG(self, "O", 1, "getUserData");
  *msgLgth = bootconf.user_rom_save_data_size;
  *msgData = os_zalloc(*msgLgth);
  checkAllocOK(*msgData);
  c_memcpy(*msgData, bootconf.user_rom_save_data, *msgLgth);
  return COMP_MSG_ERR_OK;
}

// ================================= otaStart ====================================

// start the ota process, with user supplied options
static uint8_t ICACHE_FLASH_ATTR otaStart(compMsgDispatcher_t *self, ota_callback callback, bool flashfs) {
  uint8_t slot;
  uint8_t result;
  uint8_t *otaHost;
  int numericValue;
  rboot_config bootconf;
  err_t espconnResult;

  // check not already updating
  if (system_upgrade_flag_check() == UPGRADE_FLAG_START) {
    return COMP_MSG_ERR_ALREADY_UPDATING;
  }

  // create upgrade status structure
  uud = (userUpgradeData_t *)os_zalloc(sizeof(userUpgradeData_t));
  if (uud == NULL) {
    COMP_MSG_DBG(self, "O", 1, "No ram!\n");
    return COMP_MSG_ERR_OUT_OF_MEMORY;
  }
  compMsgDispatcher = self;

  // store the callback
  uud->callback = callback;

  // get details of rom slot to update
  bootconf = rboot_get_config();
  slot = bootconf.current_rom;
  if (slot == 0) {
    slot = 1;
  }  else {
    slot = 0;
  }
  uud->rom_slot = slot;
  result = self->compMsgOta->saveUserData(self);
  checkErrOK(result);

  if (flashfs) {
    // flash spiffs
ets_printf("ota write flash1: %p\n", (bootconf.roms[uud->rom_slot] - ((BOOT_CONFIG_SECTOR + 1) * SECTOR_SIZE)) + SPIFFS_FIXED_OFFSET_RBOOT);
ets_printf("ota write flash2: %p sector: %d setcorSize: %d offset: 0x%08x\n", bootconf.roms[uud->rom_slot], (BOOT_CONFIG_SECTOR + 1), SECTOR_SIZE, SPIFFS_FIXED_OFFSET_RBOOT);
    uud->write_status = rboot_write_init((bootconf.roms[uud->rom_slot] - ((BOOT_CONFIG_SECTOR + 1) * SECTOR_SIZE)) + SPIFFS_FIXED_OFFSET_RBOOT);
    uud->rom_slot = FLASH_BY_ADDR;
  } else {
    // flash to rom slot
ets_printf("ota write rom: %d %p\n", uud->rom_slot, bootconf.roms[uud->rom_slot]);
    uud->write_status = rboot_write_init(bootconf.roms[uud->rom_slot]);
  }

  // create connection
  uud->conn = (struct espconn *)os_zalloc(sizeof(struct espconn));
  if (uud->conn == NULL) {
    COMP_MSG_DBG(self, "O", 1, "No ram!\n");
    os_free(uud);
    uud = NULL;
    return COMP_MSG_ERR_OUT_OF_MEMORY;
  }
  uud->conn->proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));
  if (uud->conn->proto.tcp == NULL) {
    COMP_MSG_DBG(self, "O", 1, "No ram!\n");
    os_free(uud->conn);
    os_free(uud);
    uud = NULL;
    return COMP_MSG_ERR_OUT_OF_MEMORY;
  }

  // set update flag
  system_upgrade_flag_set(UPGRADE_FLAG_START);

  // dns lookup
  result = compMsgDispatcher->compMsgModuleData->getOtaHost(compMsgDispatcher, &numericValue, &otaHost);
  espconnResult = espconn_gethostbyname(uud->conn, otaHost, &uud->ip, upgradeResolved);
  if (espconnResult == ESPCONN_OK) {
    // hostname is already cached or is actually a dotted decimal ip address
    upgradeResolved(0, &uud->ip, uud->conn);
  } else {
    if (espconnResult == ESPCONN_INPROGRESS) {
      // lookup taking place, will call upgrade_resolved on completion
    } else {
      COMP_MSG_DBG(self, "O", 1, "DNS error!\n");
      os_free(uud->conn->proto.tcp);
      os_free(uud->conn);
      os_free(uud);
      uud = NULL;
      return COMP_MSG_ERR_DNS_ERROR;
    }
  }

  return COMP_MSG_ERR_OK;
}

// ================================= otaUpdatCallback ====================================

static void otaUpdateCallback(bool result, uint8 rom_slot) {
  compMsgDispatcher_t *self;

  self = compMsgDispatcher;
  if (result == true) {
    // success
    if (rom_slot == FLASH_BY_ADDR) {
      COMP_MSG_DBG(self, "O", 1, "rBoot: Spiffs update successful.\n");
    } else {
      // set to boot new rom and then reboot
      COMP_MSG_DBG(self, "O", 1, "rBoot: Firmware update successful, rebooting to rom: %d...\n", rom_slot);
      rboot_set_current_rom(rom_slot);
      COMP_MSG_DBG(self, "O", 1, "call system_restart\n");
      system_restart();
    }
    COMP_MSG_DBG(self, "O", 1, "after call system_restart\n");
  } else {
    // fail
    COMP_MSG_DBG(self, "O", 1, "rBoot: Firmware update failed!\n");
  }
}


// ================================= updateFirmware ====================================

static uint8_t updateFirmware(compMsgDispatcher_t *self) {
  uint8_t result;

  result = otaStart(self, otaUpdateCallback, false);
  checkErrOK(result);
  return COMP_MSG_ERR_OK;
}

// ================================= updateSpiffs ====================================

static uint8_t updateSpiffs(compMsgDispatcher_t *self) {
  uint8_t result;

  result = otaStart(self, otaUpdateCallback, true);
  checkErrOK(result);
  return COMP_MSG_ERR_OK;
}

// ================================= checkClientMode ====================================

/**
 * \brief start the connection with the router for doing ota updates
 * \param self The dispatcher struct
 * \return Error code or ErrorOK
 *
 */
static uint8_t checkClientMode(compMsgDispatcher_t *self, bool isSpiffs) {
  uint8_t result;


  if (isSpiffs) {
    self->compMsgSendReceive->startSendMsg = self->compMsgOta->updateSpiffs;
  } else {
    self->compMsgSendReceive->startSendMsg = self->compMsgOta->updateFirmware;
  }
  if (!(self->runningModeFlags & COMP_DISP_RUNNING_MODE_CLIENT)) {
    result = self->compMsgSocket->netSocketRunClientMode(self);
    checkErrOK(result);
  } else {
    result = self->compMsgSendReceive->startSendMsg(self);
    checkErrOK(result);
  }
  return COMP_MSG_ERR_OK;
}

// ================================= compMsgOtaInit ====================================

uint8_t compMsgOtaInit(compMsgDispatcher_t *self) {
  uint8_t result;

  self->compMsgOta->updateFirmware = &updateFirmware;
  self->compMsgOta->updateSpiffs = &updateSpiffs;
  self->compMsgOta->checkClientMode = &checkClientMode;
  self->compMsgOta->otaStart = &otaStart;
  self->compMsgOta->storeUserData = &storeUserData;
  self->compMsgOta->saveUserData = &saveUserData;
  self->compMsgOta->getUserData = &getUserData;
  return COMP_MSG_ERR_OK;
}

// ================================= newCompMsgOta ====================================

compMsgOta_t *newCompMsgOta() {
  compMsgOta_t *compMsgOta = os_zalloc(sizeof(compMsgOta_t));
  if (compMsgOta == NULL) {
    return NULL;
  }
  return compMsgOta;
}

#ifdef __cplusplus
}
#endif

