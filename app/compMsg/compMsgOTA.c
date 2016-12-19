//////////////////////////////////////////////////
// This is a modified version of the code in nodemcu-firmware/app/rboot.c!!
//
// rBoot OTA sample code for ESP8266 C API.
// Copyright 2015 Richard A Burton
// richardaburton@gmail.com
// See license.txt for license terms.
// OTA code based on SDK sample from Espressif.
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
 * File:   compMsgOTA.c
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
#include "../rboot/rboot-ota.h"

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

typedef struct {
  uint8 rom_slot;         // rom slot to update, or FLASH_BY_ADDR
  ota_callback callback;  // user callback when completed
  uint32 total_len;
  uint32 content_len;
  struct espconn *conn;
  ip_addr_t ip;
  rboot_write_status write_status;
} upgrade_status;

static upgrade_status *upgrade;
static os_timer_t ota_timer;

// ================================= compMsgOTADeinit ====================================

// clean up at the end of the update
// will call the user call back to indicate completion
void ICACHE_FLASH_ATTR compMsgOTADeinit() {
  bool result;
  uint8 rom_slot;
  ota_callback callback;
  struct espconn *conn;

  os_timer_disarm(&ota_timer);

  // save only remaining bits of interest from upgrade struct
  // then we can clean it up early, so disconnect callback
  // can distinguish between us calling it after update finished
  // or being called earlier in the update process
  conn = upgrade->conn;
  rom_slot = upgrade->rom_slot;
  callback = upgrade->callback;

  // clean up
  os_free(upgrade);
  upgrade = 0;

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

  // disarm the timer
  os_timer_disarm(&ota_timer);

  // first reply?
  if (upgrade->content_len == 0) {
    // valid http response?
    if ((ptrLen = (char*)os_strstr(pusrdata, "Content-Length: "))
      && (ptrData = (char*)os_strstr(ptrLen, "\r\n\r\n"))
      && (os_strncmp(pusrdata + 9, "200", 3) == 0)) {

      // end of header/start of data
      ptrData += 4;
      // length of data after header in this chunk
      length -= (ptrData - pusrdata);
      // running total of download length
      upgrade->total_len += length;
      // process current chunk
      rboot_write_flash(&upgrade->write_status, (uint8*)ptrData, length);
      // work out total download size
      ptrLen += 16;
      ptr = (char *)os_strstr(ptrLen, "\r\n");
      *ptr = '\0'; // destructive
      upgrade->content_len = atoi(ptrLen);
    } else {
      // fail, not a valid http header/non-200 response/etc.
      compMsgOTADeinit();
      return;
    }
  } else {
    // not the first chunk, process it
    upgrade->total_len += length;
    rboot_write_flash(&upgrade->write_status, (uint8*)pusrdata, length);
  }

  // check if we are finished
  if (upgrade->total_len == upgrade->content_len) {
    system_upgrade_flag_set(UPGRADE_FLAG_FINISH);
    // clean up and call user callback
    compMsgOTADeinit();
  } else if (upgrade->conn->state != ESPCONN_READ) {
    // fail, but how do we get here? premature end of stream?
    compMsgOTADeinit();
  } else {
    // timer for next recv
    os_timer_setfn(&ota_timer, (os_timer_func_t *)compMsgOTADeinit, 0);
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
  if (upgrade && (upgrade->conn == conn)) {
    // mark connection as gone
    upgrade->conn = 0;
    // end the update process
    compMsgOTADeinit();
  }
}

// ================================= upgradeConnectCb ====================================

// successfully connected to update server, send the request
static void ICACHE_FLASH_ATTR upgradeConnectCb(void *arg) {
  uint8_t *request;

  // disable the timeout
  os_timer_disarm(&ota_timer);

  // register connection callbacks
  espconn_regist_disconcb(upgrade->conn, upgradeDisconCb);
  espconn_regist_recvcb(upgrade->conn, upgradeRecvCb);

  // http request string
  request = (uint8 *)os_malloc(512);
  if (!request) {
    c_printf("No ram!\n");
    compMsgOTADeinit();
    return;
  }
  os_sprintf((char*)request,
    "GET %s HTTP/1.1\r\nHost: " OTA_HOST "\r\n" HTTP_HEADER,
    (upgrade->rom_slot == FLASH_BY_ADDR ? OTA_FS_PATH : OTA_ROM_PATH)
    );

  // send the http request, with timeout for reply
  os_timer_setfn(&ota_timer, (os_timer_func_t *)compMsgOTADeinit, 0);
  os_timer_arm(&ota_timer, OTA_NETWORK_TIMEOUT, 0);
  espconn_sent(upgrade->conn, request, os_strlen((char*)request));
  os_free(request);
}

// ================================= connectTimeoutCb ====================================

// connection attempt timed out
static void ICACHE_FLASH_ATTR connectTimeoutCb() {
  c_printf("Connect timeout.\n");
  // not connected so don't call disconnect on the connection
  // but call our own disconnect callback to do the cleanup
  upgradeDisconCb(upgrade->conn);
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
  c_printf("Connection error: %s\n", espErrStr(errType));
  // not connected so don't call disconnect on the connection
  // but call our own disconnect callback to do the cleanup
  upgradeDisconCb(upgrade->conn);
}

// ================================= upgradeResolved ====================================

// call back for dns lookup
static void ICACHE_FLASH_ATTR upgradeResolved(const char *name, ip_addr_t *ip, void *arg) {

  if (ip == 0) {
    c_printf("DNS lookup failed for: %s\n", OTA_HOST);
    // not connected so don't call disconnect on the connection
    // but call our own disconnect callback to do the cleanup
    upgradeDisconCb(upgrade->conn);
    return;
  }

  // set up connection
  upgrade->conn->type = ESPCONN_TCP;
  upgrade->conn->state = ESPCONN_NONE;
  upgrade->conn->proto.tcp->local_port = espconn_port();
  upgrade->conn->proto.tcp->remote_port = OTA_PORT;
  *(ip_addr_t*)upgrade->conn->proto.tcp->remote_ip = *ip;
  // set connection call backs
  espconn_regist_connectcb(upgrade->conn, upgradeConnectCb);
  espconn_regist_reconcb(upgrade->conn, upgradeReconCb);

  // try to connect
  espconn_connect(upgrade->conn);

  // set connection timeout timer
  os_timer_disarm(&ota_timer);
  os_timer_setfn(&ota_timer, (os_timer_func_t *)connectTimeoutCb, 0);
  os_timer_arm(&ota_timer, OTA_NETWORK_TIMEOUT, 0);
}

// ================================= otaStart ====================================

// start the ota process, with user supplied options
static bool ICACHE_FLASH_ATTR otaStart(ota_callback callback, bool flashfs) {
  uint8_t slot;
  rboot_config bootconf;
  err_t result;

  // check not already updating
  if (system_upgrade_flag_check() == UPGRADE_FLAG_START) {
    return false;
  }

  // create upgrade status structure
  upgrade = (upgrade_status*)os_zalloc(sizeof(upgrade_status));
  if (!upgrade) {
    c_printf("No ram!\n");
    return false;
  }

  // store the callback
  upgrade->callback = callback;

  // get details of rom slot to update
  bootconf = rboot_get_config();
  slot = bootconf.current_rom;
  if (slot == 0) slot = 1; else slot = 0;
  upgrade->rom_slot = slot;

  if (flashfs) {
    // flash spiffs
//    upgrade->write_status = rboot_write_init((bootconf.roms[upgrade->rom_slot] - 0x2000) + SPIFFS_FIXED_OFFSET_RBOOT);
    upgrade->write_status = rboot_write_init((bootconf.roms[upgrade->rom_slot] - ((BOOT_CONFIG_SECTOR + 1) * SECTOR_SIZE)) + SPIFFS_FIXED_OFFSET_RBOOT);
    upgrade->rom_slot = FLASH_BY_ADDR;
  } else {
    // flash to rom slot
    upgrade->write_status = rboot_write_init(bootconf.roms[upgrade->rom_slot]);
  }

  // create connection
  upgrade->conn = (struct espconn *)os_zalloc(sizeof(struct espconn));
  if (!upgrade->conn) {
    c_printf("No ram!\n");
    os_free(upgrade);
    return false;
  }
  upgrade->conn->proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));
  if (!upgrade->conn->proto.tcp) {
    c_printf("No ram!\n");
    os_free(upgrade->conn);
    os_free(upgrade);
    return false;
  }

  // set update flag
  system_upgrade_flag_set(UPGRADE_FLAG_START);

  // dns lookup
  result = espconn_gethostbyname(upgrade->conn, OTA_HOST, &upgrade->ip, upgradeResolved);
  if (result == ESPCONN_OK) {
    // hostname is already cached or is actually a dotted decimal ip address
    upgradeResolved(0, &upgrade->ip, upgrade->conn);
  } else if (result == ESPCONN_INPROGRESS) {
    // lookup taking place, will call upgrade_resolved on completion
  } else {
    c_printf("DNS error!\n");
    os_free(upgrade->conn->proto.tcp);
    os_free(upgrade->conn);
    os_free(upgrade);
    return false;
  }

  return true;
}

// ================================= updateFirmware ====================================

static uint8_t updateFirmware(compMsgDispatcher_t *self) {
  uint8_t result;

  return COMP_MSG_ERR_OK;
}

// ================================= updateSpiffs ====================================

static uint8_t updateSpiffs(compMsgDispatcher_t *self) {
  uint8_t result;

  return COMP_MSG_ERR_OK;
}

// ================================= compMsgOTAInit ====================================

uint8_t compMsgOTAInit(compMsgDispatcher_t *self) {
  uint8_t result;

  self->compMsgOTA->updateFirmware = &updateFirmware;
  self->compMsgOTA->updateSpiffs = &updateSpiffs;
  return COMP_MSG_ERR_OK;
}

// ================================= newCompMsgOTA ====================================

compMsgOTA_t *newCompMsgOTA() {
  compMsgOTA_t *compMsgOTA = os_zalloc(sizeof(compMsgOTA_t));
  if (compMsgOTA == NULL) {
    return NULL;
  }

  return compMsgOTA;
}

#ifdef __cplusplus
}
#endif

