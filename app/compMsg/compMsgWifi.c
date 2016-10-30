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
 * File:   compMsgModuleData.c
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
#include "compMsgDispatcher.h"

static bool bssScanRunning = false;
static bssScanInfos_t bssScanInfos = { NULL, 0, 0};
static stationConfig_t stationConfig;
static compMsgWifiData_t compMsgWifiData;

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
      return COMP_MSG_ERR_OK;
    }
    idx++;
    entry = &bssStr2BssInfoIds[idx];
  }
  return COMP_MSG_ERR_FIELD_NOT_FOUND;
}

// ================================= websocketBinaryReceived ====================================

static void websocketBinaryReceived(void *arg, void *wud, char *pdata, unsigned short len) {
  compMsgDispatcher_t *compMsgDispatcher;
  compMsgDispatcher_t *self;
  uint8_t result;

  compMsgDispatcher = (compMsgDispatcher_t *)arg;
  self = compMsgDispatcher;
//ets_printf("websocketBinaryReceived: len: %d dispatcher: %p\n", len, compMsgDispatcher);
  result = self->resetMsgInfo(self, &self->received);
//  checkErrOK(result);
  self->wud = wud;
  result = compMsgDispatcher->handleReceivedPart(compMsgDispatcher, (uint8_t *)pdata, (uint8_t)len);
ets_printf("websocketBinaryReceived end result: %d\n", result);
}

// ================================= websocketTextReceived ====================================

static void websocketTextReceived(void *arg, void *wud, char *pdata, unsigned short len) {
  compMsgDispatcher_t *compMsgDispatcher;

  compMsgDispatcher = (compMsgDispatcher_t *)arg;
//ets_printf("websocketTextReceived: len: %d dispatcher: %p\n", len, compMsgDispatcher);
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
  if (status != COMP_DISP_ERR_OK) {
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
  bssScanInfos.compMsgDispatcher->buildMsg(bssScanInfos.compMsgDispatcher);
}

// ================================= getBssScanInfo ====================================

static uint8_t getBssScanInfo(compMsgDispatcher_t *self) {
  bool result;
  struct scan_config scan_config;

//ets_printf("getBssScanInfo1: \n");
  if (bssScanRunning) {
    // silently ignore 
    return COMP_DISP_ERR_OK;
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
    return COMP_DISP_ERR_STATION_SCAN;
  }
//ets_printf("getBssScanInfo3:\n");
  return COMP_DISP_ERR_OK;
}

// ================================= getStationConfig ====================================

static uint8_t getStationConfig(compMsgDispatcher_t *self) {
  int result;
  struct station_config station_config;

  compMsgWifiData.wifiOpMode = wifi_get_opmode();
  result = wifi_station_get_config(&station_config);
  if (result != true) {
    return COMP_DISP_ERR_GET_STATION_CONFIG;
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

  return COMP_DISP_ERR_OK;
}

// ================================= geScanInfoTableFieldValue ====================================

static uint8_t getScanInfoTableFieldValue(compMsgDispatcher_t *self, uint8_t actionMode) {
  uint8_t result;
  bssScanInfos_t *scanInfos;
  bssScanInfo_t *scanInfo;
  uint8_t fieldId;
  
//ets_printf("getModuleTableFieldValue: row: %d col: %d actionMode: %d\n", self->buildMsgInfos.tableRow, self->buildMsgInfos.tableCol, actionMode);
  scanInfos = self->bssScanInfos;
  self->buildMsgInfos.numericValue = 0;
  self->buildMsgInfos.stringValue = NULL;
    if (self->buildMsgInfos.tableRow > scanInfos->numScanInfos) {
      return COMP_DISP_ERR_BAD_ROW;
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
      return COMP_DISP_ERR_OK;
      break;
    case  BSS_INFO_SSID_LEN:
      break;
    case  BSS_INFO_CHANNEL:
      break;
    case  BSS_INFO_RSSID:
      self->buildMsgInfos.numericValue = scanInfo->rssi;
      return COMP_DISP_ERR_OK;
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
  return COMP_DISP_ERR_ACTION_NAME_NOT_FOUND;
}

// ================================= getWifiValue ====================================

static uint8_t getWifiValue(compMsgDispatcher_t *self, uint16_t which, uint8_t valueTypeId, int *numericValue, uint8_t **stringValue) {
  uint8_t result;

  *numericValue = 0;
  *stringValue = NULL;
  switch (which) {
  case MODULE_INFO_PROVISIONING_SSID:
    *stringValue = compMsgWifiData.provisioningSsid;
    break;
  case MODULE_INFO_PROVISIONING_PORT:
    *numericValue = compMsgWifiData.provisioningPort;
    break;
  case MODULE_INFO_PROVISIONING_IP_ADDR:
    *stringValue = compMsgWifiData.provisioningIPAddr;
    break;
  case MODULE_INFO_BINARY_CALL_BACK:
    *numericValue = (int)compMsgWifiData.websocketBinaryReceived;
    break;
  case MODULE_INFO_TEXT_CALL_BACK:
    *numericValue = (int)compMsgWifiData.websocketTextReceived;
    break;
  default:
    return COMP_DISP_ERR_BAD_MODULE_VALUE_WHICH;
    break;
  }
  return COMP_DISP_ERR_OK;
}

// ================================= setWifiValues ====================================

static uint8_t setWifiValues(compMsgDispatcher_t *self) {
  uint8_t result;
  uint8_t *provisioningSsid;
  uint8_t provisioningPort;
  uint8_t *provisioningIPAddr;

  self->bssScanInfos = &bssScanInfos;

  provisioningSsid = "testDevice_connect";
  c_memcpy(compMsgWifiData.provisioningSsid, provisioningSsid, c_strlen(provisioningSsid));
  compMsgWifiData.provisioningPort = 80;
  provisioningIPAddr = "192.168.4.1";
  c_memcpy(compMsgWifiData.provisioningIPAddr, provisioningIPAddr, c_strlen(provisioningIPAddr));
  result = getStationConfig(self);
  checkErrOK(result);
  return COMP_DISP_ERR_OK;
}

// ================================= compMsgWifiInit ====================================

uint8_t compMsgWifiInit(compMsgDispatcher_t *self) {
  uint8_t result;

  compMsgWifiData.websocketBinaryReceived = &websocketBinaryReceived;
  compMsgWifiData.websocketTextReceived = &websocketTextReceived;

  self->getBssScanInfo = &getBssScanInfo;
  self->getScanInfoTableFieldValue = &getScanInfoTableFieldValue;
  self->getWifiValue = &getWifiValue;
  self->setWifiValues = &setWifiValues;

  bssScanInfos.compMsgDispatcher = self;

  return COMP_DISP_ERR_OK;
}

