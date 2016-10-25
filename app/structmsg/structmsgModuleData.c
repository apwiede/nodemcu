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
 * File:   structmsgDataValues.c
 * Author: Arnulf P. Wiedemann
 *
 * Created on October 7st, 2016
 */

#include "osapi.h"
#include "c_types.h"
#include "mem.h"
#include "flash_fs.h"

#include "c_string.h"
#include "c_stdlib.h"
#include "c_stdio.h"
#include "platform.h"
#include "structmsgData.h"

static bool bssScanRunning = false;
static bssScanInfos_t bssScanInfos = { NULL, 0, 0};
static stationConfig_t stationConfig;
static structmsgModuleData_t structmsgModuleData;
static structmsgWifiData_t structmsgWifiData;

static str2id_t bssStr2BssInfoIds [] = {
  { "bssid",       BSS_INFO_BSSID },
  { "bssidStr",    BSS_INFO_BSSID_STR },
  { "ssid",        BSS_INFO_SSID },
  { "ssid_len",    BSS_INFO_SSID_LEN },
  { "channel",     BSS_INFO_CHANNEL },
  { "rssi",        BSS_INFO_RSSID },
  { "authmode",    BSS_INFO_AUTH_MODE },
  { "is_hidden",   BSS_INFO_IS_HIDDEN },
  { "freq_offset", BSS_INFO_FREQ_OFFSET },
  { "freqcal_val", BSS_INFO_FREQ_CAL_VAL },
  { NULL,          0 },
};

// ================================= bssStr2BssInfoId ====================================

static uint8_t bssStr2BssInfoId(uint8_t *fieldName, uint8_t *fieldId) {
  int idx;
  str2id_t *entry;

  idx = 0;
  entry = &bssStr2BssInfoIds[idx];
  while (entry->str != NULL) {
    if (c_strcmp(entry->str, fieldName) == 0) {
      *fieldId = entry->id;
      return STRUCT_MSG_ERR_OK;
    }
    idx++;
    entry = &bssStr2BssInfoIds[idx];
  }
  return STRUCT_MSG_ERR_FIELD_NOT_FOUND;
}

// ================================= websocketBinaryReceived ====================================

static void websocketBinaryReceived(void *arg, void *wud, char *pdata, unsigned short len) {
  structmsgDispatcher_t *structmsgDispatcher;
  structmsgDispatcher_t *self;
  uint8_t result;

  structmsgDispatcher = (structmsgDispatcher_t *)arg;
  self = structmsgDispatcher;
//ets_printf("websocketBinaryReceived: len: %d dispatcher: %p\n", len, structmsgDispatcher);
  result = self->resetMsgInfo(self, &self->received);
//  checkErrOK(result);
  self->wud = wud;
  result = structmsgDispatcher->handleReceivedPart(structmsgDispatcher, (uint8_t *)pdata, (uint8_t)len);
ets_printf("websocketBinaryReceived end result: %d\n", result);
}

// ================================= websocketTextReceived ====================================

static void websocketTextReceived(void *arg, void *wud, char *pdata, unsigned short len) {
  structmsgDispatcher_t *structmsgDispatcher;

  structmsgDispatcher = (structmsgDispatcher_t *)arg;
//ets_printf("websocketTextReceived: len: %d dispatcher: %p\n", len, structmsgDispatcher);
}

// ================================= getModuleTableFieldValue ====================================

static uint8_t getModuleTableFieldValue(structmsgDispatcher_t *self, uint8_t actionMode) {
  uint8_t result;
  bssScanInfos_t *scanInfos;
  bssScanInfo_t *scanInfo;
  uint8_t fieldId;
  
//ets_printf("getModuleTableFieldValue: row: %d col: %d actionMode: %d\n", self->buildMsgInfos.tableRow, self->buildMsgInfos.tableCol, actionMode);
  scanInfos = self->bssScanInfos;
  self->buildMsgInfos.numericValue = 0;
  self->buildMsgInfos.stringValue = NULL;
  switch (actionMode) {
  case MODULE_INFO_AP_LIST_CALL_BACK:
    if (self->buildMsgInfos.tableRow > scanInfos->numScanInfos) {
      return STRUCT_DISP_ERR_BAD_ROW;
    }
    scanInfo = &scanInfos->infos[self->buildMsgInfos.tableRow];
    result = bssStr2BssInfoId(self->buildMsgInfos.fieldNameStr, &fieldId);
//ets_printf("row: %d ssid: %s rssi: %d fieldName: %s fieldId: %d\n", self->buildMsgInfos.tableRow, scanInfo->ssid, scanInfo->rssi, self->buildMsgInfos.fieldNameStr, fieldId);
    checkErrOK(result);
    switch ((int)fieldId) {
    case  BSS_INFO_BSSID:
      break;
    case  BSS_INFO_BSSID_STR:
      break;
    case  BSS_INFO_SSID:
      self->buildMsgInfos.stringValue = scanInfo->ssid;
      return STRUCT_DISP_ERR_OK;
      break;
    case  BSS_INFO_SSID_LEN:
      break;
    case  BSS_INFO_CHANNEL:
      break;
    case  BSS_INFO_RSSID:
      self->buildMsgInfos.numericValue = scanInfo->rssi;
      return STRUCT_DISP_ERR_OK;
      break;
    case  BSS_INFO_AUTH_MODE:
      break;
    case  BSS_INFO_IS_HIDDEN:
      break;
    case  BSS_INFO_FREQ_OFFSET:
      break;
    case  BSS_INFO_FREQ_CAL_VAL:
      break;
    }
  default:
    return STRUCT_DISP_ERR_ACTION_NAME_NOT_FOUND;
  }
  return STRUCT_DISP_ERR_ACTION_NAME_NOT_FOUND;
}

// ================================= getModuleValue ====================================

static uint8_t getModuleValue(structmsgDispatcher_t *self, uint16_t which, uint8_t valueTypeId, int *numericValue, uint8_t **stringValue) {
  int result;

  *numericValue = 0;
  *stringValue = NULL;
  switch (which) {
  case MODULE_INFO_MACAddr:
    *stringValue = structmsgModuleData.MACAddr;
    break;
  case MODULE_INFO_IPAddr:
    *stringValue = structmsgModuleData.IPAddr;
    break;
  case MODULE_INFO_FirmwareVersion:
    *stringValue = structmsgModuleData.FirmwareVersion;
    break;
  case MODULE_INFO_SerieNumber:
    *stringValue = structmsgModuleData.SerieNumber;
    break;
  case MODULE_INFO_RSSI:
    *numericValue = (int)structmsgModuleData.RSSI;
    break;
  case MODULE_INFO_ModuleConnection:
    *numericValue = (int)structmsgModuleData.ModuleConnection;
    break;
  case MODULE_INFO_DeviceMode:
    *numericValue = (int)structmsgModuleData.DeviceMode;
    break;
  case MODULE_INFO_DeviceSecurity:
    *numericValue = (int)structmsgModuleData.DeviceSecurity;
    break;
  case MODULE_INFO_ErrorMain:
    *numericValue = (int)structmsgModuleData.ErrorMain;
    break;
  case MODULE_INFO_ErrorSub:
    *numericValue = (int)structmsgModuleData.ErrorSub;
    break;
  case MODULE_INFO_DateAndTime:
    *stringValue = structmsgModuleData.DateAndTime;
    break;
  case MODULE_INFO_SSIDs:
    *numericValue = (int)structmsgModuleData.SSIDs;
    break;
  case MODULE_INFO_Reserve1:
    *numericValue = (int)structmsgModuleData.Reserve1;
    break;
  case MODULE_INFO_Reserve2:
    *stringValue = structmsgModuleData.Reserve2;
    break;
  case MODULE_INFO_Reserve3:
    *stringValue = structmsgModuleData.Reserve3;
    break;
  case MODULE_INFO_PROVISIONING_SSID:
    *stringValue = structmsgWifiData.provisioningSsid;
    break;
  case MODULE_INFO_PROVISIONING_PORT:
    *numericValue = structmsgWifiData.provisioningPort;
    break;
  case MODULE_INFO_PROVISIONING_IP_ADDR:
    *stringValue = structmsgWifiData.provisioningIPAddr;
    break;
  case MODULE_INFO_BINARY_CALL_BACK:
    *numericValue = (int)structmsgWifiData.websocketBinaryReceived;
    break;
  case MODULE_INFO_TEXT_CALL_BACK:
    *numericValue = (int)structmsgWifiData.websocketTextReceived;
    break;
  default:
    return STRUCT_DISP_ERR_BAD_MODULE_VALUE_WHICH;
    break;
  }
  return STRUCT_DISP_ERR_OK;
}


// ================================= bssScanDoneCb ====================================

static void bssScanDoneCb(void *arg, STATUS status) {
  struct bss_info *bss_link;
  uint8_t numEntries;
  bssScanInfo_t *scanInfo;

//ets_printf("bssScanDoneCb bssScanRunning: arg: %p %d status: %d!\n", arg, bssScanRunning, status);
  if (arg == NULL) {
    return;
  }
  if (status != STRUCT_DISP_ERR_OK) {
    return;
  }
  if (bssScanRunning == false) {
    return;
  }
  if (bssScanRunning == true) {
    bssScanRunning = false;
  }
//ets_printf("bssScanDoneCb bssScanRunning2: %d status: %d!\n", bssScanRunning, status);
  numEntries = 0;
  bss_link = (struct bss_info *)arg;
  while (bss_link != NULL) {
    numEntries++;
    bss_link = bss_link->next.stqe_next;
  }
  if (bssScanInfos.infos != NULL) {
    os_free(bssScanInfos.infos);
    bssScanInfos.infos = NULL;
    bssScanInfos.numScanInfos = 0;
    bssScanInfos.maxScanInfos = 0;
  }
  bssScanInfos.maxScanInfos = numEntries;
  bssScanInfos.infos = os_zalloc(bssScanInfos.maxScanInfos * sizeof(bssScanInfo_t));
  if (bssScanInfos.infos == NULL) {
    return;
  }
  bss_link = (struct bss_info *)arg;
  while (bss_link != NULL) {
    scanInfo = &bssScanInfos.infos[bssScanInfos.numScanInfos];
    c_memset(scanInfo->ssid, 0, sizeof(scanInfo->ssid));
    if (bss_link->ssid_len <= sizeof(scanInfo->ssid)) {
      c_memcpy(scanInfo->ssid, bss_link->ssid, bss_link->ssid_len);
    } else {
      c_memcpy(scanInfo->ssid, bss_link->ssid, sizeof(scanInfo->ssid));
    }
    c_memset(scanInfo->bssidStr, 0, sizeof(scanInfo->bssidStr));
    c_memcpy(scanInfo->bssid, bss_link->bssid, sizeof(scanInfo->bssid));
    c_sprintf(scanInfo->bssidStr,MACSTR, MAC2STR(bss_link->bssid));
    scanInfo->channel = bss_link->channel;
    scanInfo->rssi = bss_link->rssi;
    scanInfo->authmode = bss_link->authmode;
    scanInfo->is_hidden = bss_link->is_hidden;
    scanInfo->freq_offset = bss_link->freq_offset;
    scanInfo->freqcal_val = bss_link->freqcal_val;
    bss_link = bss_link->next.stqe_next;
    bssScanInfos.numScanInfos++;
  }
//ets_printf("bssScanDoneCb call buildMsg\n");
  bssScanInfos.scanInfoComplete = true;
  bssScanInfos.structmsgDispatcher->buildMsg(bssScanInfos.structmsgDispatcher);
}

// ================================= getBssScanInfo ====================================

static uint8_t getBssScanInfo(structmsgDispatcher_t *self) {
  bool result;
  struct scan_config scan_config;

//ets_printf("getBssScanInfo1: \n");
  if (bssScanRunning) {
    // silently ignore 
    return STRUCT_DISP_ERR_OK;
  }
  bssScanRunning = true;
  scan_config.ssid = NULL;
  scan_config.bssid = NULL;
  scan_config.channel = 0;
  scan_config.show_hidden = 1;
  self->bssScanInfos->scanInfoComplete = false;
  result = wifi_station_scan(&scan_config, bssScanDoneCb);
//ets_printf("getBssScanInfo2: result: %d\n", result);
  if (result != true) {
    return STRUCT_DISP_ERR_STATION_SCAN;
  }
//ets_printf("getBssScanInfo3:\n");
  return STRUCT_DISP_ERR_OK;
}

// ================================= getStationConfig ====================================

static uint8_t getStationConfig(structmsgDispatcher_t *self) {
  int result;
  struct station_config station_config;

  structmsgWifiData.wifiOpMode = wifi_get_opmode();
  result = wifi_station_get_config(&station_config);
  if (result != true) {
    return STRUCT_DISP_ERR_GET_STATION_CONFIG;
  }
  c_memset(stationConfig.ssid, 0, sizeof(stationConfig.ssid));
  if (c_strlen(station_config.ssid) <= sizeof(stationConfig.ssid)) {
    c_memcpy(stationConfig.ssid, station_config.ssid, c_strlen(station_config.ssid));
  } else {
    c_memcpy(stationConfig.ssid, station_config.ssid, sizeof(stationConfig.ssid));
  }
  c_memset(stationConfig.password, 0, sizeof(stationConfig.password));
  if (c_strlen(station_config.password) <= sizeof(stationConfig.password)) {
    c_memcpy(stationConfig.password, station_config.password, c_strlen(station_config.password));
  } else {
    c_memcpy(stationConfig.password, station_config.password, sizeof(stationConfig.password));
  }
  c_memset(stationConfig.bssidStr, 0, sizeof(stationConfig.bssidStr));
  c_memcpy(stationConfig.bssid, station_config.bssid, sizeof(stationConfig.bssid));
  c_sprintf(stationConfig.bssidStr,MACSTR, MAC2STR(station_config.bssid));
  stationConfig.bssidSet = station_config.bssid_set;
  stationConfig.status = wifi_station_get_connect_status();
  stationConfig.mode = wifi_get_opmode();
  stationConfig.channel = wifi_get_channel();

//  authmode;
//  freq_offset;
//  freqcal_val;

  return STRUCT_DISP_ERR_OK;
}

// ================================= updateModuleValues ====================================

static uint8_t updateModuleValues(structmsgDispatcher_t *self) {
  int result;

  structmsgModuleData.RSSI = (uint8_t)wifi_station_get_rssi();
  return STRUCT_DISP_ERR_OK;
}

// ================================= setModuleValues ====================================

static uint8_t setModuleValues(structmsgDispatcher_t *self) {
  int result;
  uint8_t *provisioningSsid;
  uint8_t provisioningPort;
  uint8_t *provisioningIPAddr;

  structmsgModuleData.MACAddr[0] = 0xAB;
  structmsgModuleData.MACAddr[1] = 0xCD;
  structmsgModuleData.MACAddr[2] = 0xEF;
  structmsgModuleData.MACAddr[3] = 0x12;
  structmsgModuleData.MACAddr[4] = 0x34;
  structmsgModuleData.MACAddr[5] = 0x56;
  structmsgModuleData.IPAddr[0] = 0xD4;
  structmsgModuleData.IPAddr[1] = 0xC3;
  structmsgModuleData.IPAddr[2] = 0x12;
  structmsgModuleData.IPAddr[3] = 0x34;
  structmsgModuleData.FirmwareVersion[0] = 0x12;
  structmsgModuleData.FirmwareVersion[1] = 0x34;
  structmsgModuleData.FirmwareVersion[2] = 0x56;
  structmsgModuleData.FirmwareVersion[3] = 0xAB;
  structmsgModuleData.FirmwareVersion[4] = 0xCD;
  structmsgModuleData.FirmwareVersion[5] = 0xEF;
  structmsgModuleData.SerieNumber[0] = 0x02;
  structmsgModuleData.SerieNumber[1] = 0x13;
  structmsgModuleData.SerieNumber[2] = 0x2A;
  structmsgModuleData.SerieNumber[3] = 0x10;
  structmsgModuleData.RSSI = (uint8_t)wifi_station_get_rssi();
  structmsgModuleData.ModuleConnection = 0x41;
  structmsgModuleData.DeviceMode = 0x34;
  structmsgModuleData.DeviceSecurity = 0x00;
  structmsgModuleData.ErrorMain = 0;
  structmsgModuleData.ErrorSub = 0;
  structmsgModuleData.DateAndTime[0] = 0x00;
  structmsgModuleData.DateAndTime[1] = 0x00;
  structmsgModuleData.DateAndTime[2] = 0x00;
  structmsgModuleData.DateAndTime[3] = 0x00;
  structmsgModuleData.DateAndTime[4] = 0x00;
  structmsgModuleData.DateAndTime[5] = 0x00;
  structmsgModuleData.SSIDs = 2;
  structmsgModuleData.Reserve1 = 'X';
  structmsgModuleData.Reserve2[0] = 'X';
  structmsgModuleData.Reserve2[1] = 'Y';
  structmsgModuleData.Reserve3[0] = 'X';
  structmsgModuleData.Reserve3[1] = 'Y';
  structmsgModuleData.Reserve3[2] = 'Z';

  structmsgWifiData.websocketBinaryReceived = &websocketBinaryReceived;
  structmsgWifiData.websocketTextReceived = &websocketTextReceived;

  self->bssScanInfos = &bssScanInfos;

  provisioningSsid = "testDevice_connect";
  c_memcpy(structmsgWifiData.provisioningSsid, provisioningSsid, c_strlen(provisioningSsid));
  structmsgWifiData.provisioningPort = 80;
  provisioningIPAddr = "192.168.4.1";
  c_memcpy(structmsgWifiData.provisioningIPAddr, provisioningIPAddr, c_strlen(provisioningIPAddr));
  result = getStationConfig(self);
  checkErrOK(result);
//  result = getBssScanInfo(self);
//  checkErrOK(result);

  return STRUCT_DISP_ERR_OK;
}

// ================================= structmsgModuleDataValuesInit ====================================

uint8_t structmsgModuleDataValuesInit(structmsgDispatcher_t *self) {
  uint8_t result;

  self->setModuleValues = &setModuleValues;
  self->updateModuleValues = &updateModuleValues;
  self->getModuleValue = &getModuleValue;
  self->getModuleTableFieldValue = &getModuleTableFieldValue;
  self->getBssScanInfo = &getBssScanInfo;
  bssScanInfos.structmsgDispatcher = self;

  self->setModuleValues(self);
  return STRUCT_DISP_ERR_OK;
}

