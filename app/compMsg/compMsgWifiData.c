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

#define COMP_MSG_WIFI_VALUES_FILE_NAME "myConfig.txt"

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
  { "rssi",        BSS_INFO_RSSI },
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
ets_printf("websocketBinaryReceived: len: %d dispatcher: %p\n", len, compMsgDispatcher);
  result = self->resetMsgInfo(self, &self->received);
//  checkErrOK(result);
  result = self->getNewCompMsgDataPtr(self);
ets_printf("received compMsgData: %p\n", self->compMsgData);
  self->compMsgData->wud = wud;
  self->compMsgData->nud = NULL;
  self->compMsgData->direction = COMP_MSG_RECEIVED_DATA;
  self->compMsgData->receivedData = (uint8_t *)pdata;
  self->compMsgData->receivedLgth = (uint8_t)len;
  result = self->addRequest(self, COMP_DISP_INPUT_WEB_SOCKET, wud, self->compMsgData);
ets_printf("websocketBinaryReceived end result: %d\n", result);
}

// ================================= netsocketToSend ====================================

static void netsocketToSend(void *arg, void *nud, char *pdata, unsigned short len) {
  compMsgDispatcher_t *compMsgDispatcher;
  compMsgDispatcher_t *self;
  uint8_t result;

  compMsgDispatcher = (compMsgDispatcher_t *)arg;
  self = compMsgDispatcher;
ets_printf("netsocketSend: len: %d dispatcher: %p\n", len, compMsgDispatcher);
  result = self->resetMsgInfo(self, &self->toSend);
//  checkErrOK(result);
  result = self->getNewCompMsgDataPtr(self);
ets_printf("send compMsgData: %p lgth: %d\n", self->compMsgData, len);
  self->compMsgData->wud = NULL;
  self->compMsgData->nud = nud;
  self->compMsgData->direction = COMP_MSG_TO_SEND_DATA;
  self->compMsgData->u16CmdKey = 17220; // FIXME hard wired 'CD'
  self->compMsgData->toSendData = (uint8_t *)pdata;
  self->compMsgData->toSendLgth = (uint8_t)len;
  result = self->addRequest(self, COMP_DISP_INPUT_NET_SOCKET, nud, self->compMsgData);
ets_printf("netsocketSend end result: %d\n", result);
}

// ================================= netsocketReceived ====================================

static void netsocketReceived(void *arg, void *nud, char *pdata, unsigned short len) {
  compMsgDispatcher_t *compMsgDispatcher;
  compMsgDispatcher_t *self;
  uint8_t result;

  compMsgDispatcher = (compMsgDispatcher_t *)arg;
  self = compMsgDispatcher;
ets_printf("netsocketReceived: len: %d dispatcher: %p\n", len, compMsgDispatcher);
  result = self->resetMsgInfo(self, &self->received);
//  checkErrOK(result);
  result = self->getNewCompMsgDataPtr(self);
ets_printf("received compMsgData: %p remote_port: %d receivedLgth: %d\n", self->compMsgData, ((netsocketUserData_t*)nud)->remote_port, self->compMsgData->receivedLgth);
  self->compMsgData->wud = NULL;
  self->compMsgData->nud = nud;
  self->compMsgData->direction = COMP_MSG_RECEIVED_DATA;
  self->compMsgData->receivedData = (uint8_t *)pdata;
  self->compMsgData->receivedLgth = (uint8_t)len;
  result = self->addRequest(self, COMP_DISP_INPUT_NET_SOCKET, nud, self->compMsgData);
ets_printf("netsocketReceived end result: %d\n", result);
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
  compMsgWifiData.bssScanSizes.bssidSize = 0;
  compMsgWifiData.bssScanSizes.bssidStrSize = 0;
  compMsgWifiData.bssScanSizes.ssidSize = 0;
  compMsgWifiData.bssScanSizes.channelSize = 0;
  compMsgWifiData.bssScanSizes.rssiSize = 0;
  compMsgWifiData.bssScanSizes.authmodeSize = 0;
  compMsgWifiData.bssScanSizes.is_hiddenSize = 0;
  compMsgWifiData.bssScanSizes.freq_offsetSize = 0;
  compMsgWifiData.bssScanSizes.freqcal_valSize = 0;
  while (bss_link != NULL) {
    scanInfo = &bssScanInfos.infos[bssScanInfos.numScanInfos];
    c_memset(scanInfo->ssid, 0, sizeof(scanInfo->ssid));
    if (bss_link->ssid_len <= sizeof(scanInfo->ssid)) {
      c_memcpy(scanInfo->ssid, bss_link->ssid, bss_link->ssid_len);
      compMsgWifiData.bssScanSizes.ssidSize += bss_link->ssid_len + 1;
    } else {
      c_memcpy(scanInfo->ssid, bss_link->ssid, sizeof(scanInfo->ssid));
      compMsgWifiData.bssScanSizes.ssidSize += sizeof(scanInfo->ssid) + 1;
    }
    scanInfo->ssid_len = bss_link->ssid_len;
    c_memset(scanInfo->bssidStr, 0, sizeof(scanInfo->bssidStr));
    c_memcpy(scanInfo->bssid, bss_link->bssid, sizeof(scanInfo->bssid));
    compMsgWifiData.bssScanSizes.bssidSize += sizeof(scanInfo->bssid) + 1;
    c_sprintf(scanInfo->bssidStr,MACSTR, MAC2STR(bss_link->bssid));
    scanInfo->channel = bss_link->channel;
    compMsgWifiData.bssScanSizes.channelSize += 1;
    scanInfo->rssi = bss_link->rssi;
    compMsgWifiData.bssScanSizes.rssiSize += 1;
    scanInfo->authmode = bss_link->authmode;
    compMsgWifiData.bssScanSizes.authmodeSize += 1;
    scanInfo->is_hidden = bss_link->is_hidden;
    compMsgWifiData.bssScanSizes.is_hiddenSize += 1;
    scanInfo->freq_offset = bss_link->freq_offset;
    compMsgWifiData.bssScanSizes.freq_offsetSize += 2;
    scanInfo->freqcal_val = bss_link->freqcal_val;
    compMsgWifiData.bssScanSizes.freqcal_valSize += 2;
    bss_link = bss_link->next.stqe_next;
    bssScanInfos.numScanInfos++;
  }
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
  self->compMsgData->buildMsgInfos.numericValue = 0;
  self->compMsgData->buildMsgInfos.stringValue = NULL;
  if (self->compMsgData->buildMsgInfos.tableRow > scanInfos->numScanInfos) {
    return COMP_DISP_ERR_BAD_ROW;
  }
  scanInfo = &scanInfos->infos[self->compMsgData->buildMsgInfos.tableRow];
  result = bssStr2BssInfoId(self->compMsgData->msgDescPart->fieldNameStr, &fieldId);
//ets_printf("row: %d ssid: %s rssi: %d fieldName: %s fieldId: %d\n", self->buildMsgInfos.tableRow, scanInfo->ssid, scanInfo->rssi, self->buildMsgInfos.fieldNameStr, fieldId);
  checkErrOK(result);
  switch ((int)fieldId) {
  case  BSS_INFO_BSSID:
    break;
  case  BSS_INFO_BSSID_STR:
    break;
  case  BSS_INFO_SSID:
    self->compMsgData->buildMsgInfos.stringValue = scanInfo->ssid;
    return COMP_DISP_ERR_OK;
    break;
  case  BSS_INFO_SSID_LEN:
    break;
  case  BSS_INFO_CHANNEL:
    break;
  case  BSS_INFO_RSSI:
    self->compMsgData->buildMsgInfos.numericValue = scanInfo->rssi;
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

// ================================= getWifiKeyValueInfo ====================================

static uint8_t getWifiKeyValueInfo(compMsgDispatcher_t *self) {
  uint8_t result;
  uint8_t bssInfoType;

  result = bssStr2BssInfoId(self->compMsgData->msgDescPart->fieldNameStr + c_strlen("#key_"), &bssInfoType);
  checkErrOK(result);
  switch ((int)bssInfoType) {
  case  BSS_INFO_BSSID:
    break;
  case  BSS_INFO_BSSID_STR:
    break;
  case  BSS_INFO_SSID:
    self->compMsgData->msgDescPart->fieldKey = compMsgWifiData.key_ssid;
    self->compMsgData->msgDescPart->fieldSize = compMsgWifiData.bssScanSizes.ssidSize;
    self->compMsgData->msgDescPart->fieldType = compMsgWifiData.bssScanTypes.ssidType;
    return COMP_DISP_ERR_OK;
    break;
  case  BSS_INFO_SSID_LEN:
    break;
  case  BSS_INFO_CHANNEL:
    break;
  case  BSS_INFO_RSSI:
    self->compMsgData->msgDescPart->fieldKey = compMsgWifiData.key_rssi;
    self->compMsgData->msgDescPart->fieldSize = compMsgWifiData.bssScanSizes.rssiSize;
    self->compMsgData->msgDescPart->fieldType = compMsgWifiData.bssScanTypes.rssiType;
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
  return COMP_DISP_ERR_FIELD_NOT_FOUND;
}

// ================================= getWifiKeyValue ====================================

static uint8_t getWifiKeyValue(compMsgDispatcher_t *self) {
  uint8_t result;
  uint8_t bssInfoType;
  int entryIdx;
  uint8_t *cp;
uint8_t *cp2;
  uint8_t *saveData;
  uint8_t *saveWhere;
  size_t saveLgth;
  bssScanInfo_t *bssScanInfo;
  compMsgData_t *compMsgData;

  compMsgData = self->compMsgData;
  result = bssStr2BssInfoId(compMsgData->msgValPart->fieldNameStr + c_strlen("#key_"), &bssInfoType);
  checkErrOK(result);
  result = compMsgData->compMsgDataView->dataView->getDataViewData(compMsgData->compMsgDataView->dataView, &saveWhere, &saveData, &saveLgth);
  checkErrOK(result);
  switch ((int)bssInfoType) {
  case  BSS_INFO_BSSID:
    break;
  case  BSS_INFO_BSSID_STR:
    break;
  case  BSS_INFO_SSID:
    compMsgData->msgValPart->fieldKeyValueStr = os_zalloc(compMsgData->msgDescPart->fieldSize);
    checkAllocOK(compMsgData->msgValPart->fieldKeyValueStr);
    entryIdx = 0;
    cp = compMsgData->msgValPart->fieldKeyValueStr;
    result = compMsgData->compMsgDataView->dataView->setDataViewData(compMsgData->compMsgDataView->dataView, "getWifiKeyvalue", cp, 2 * sizeof(uint16_t) + sizeof(uint8_t));
    checkErrOK(result);
    result = compMsgData->compMsgDataView->dataView->setUint16(compMsgData->compMsgDataView->dataView, 0, compMsgData->msgDescPart->fieldKey);
    checkErrOK(result);
    result = compMsgData->compMsgDataView->dataView->setUint8(compMsgData->compMsgDataView->dataView, 2, compMsgData->msgDescPart->fieldType);
    checkErrOK(result);
    result = compMsgData->compMsgDataView->dataView->setUint16(compMsgData->compMsgDataView->dataView, 3, compMsgData->msgDescPart->fieldSize - (2 * sizeof(uint16_t) + sizeof(uint8_t)));
    checkErrOK(result);
    cp += 2 * sizeof(uint16_t) + sizeof(uint8_t);
    result = compMsgData->compMsgDataView->dataView->setDataViewData(compMsgData->compMsgDataView->dataView, saveWhere, saveData, saveLgth);
    checkErrOK(result);
    while (entryIdx < self->bssScanInfos->numScanInfos) {
      bssScanInfo = &self->bssScanInfos->infos[entryIdx];
      c_memcpy(cp, bssScanInfo->ssid, bssScanInfo->ssid_len);
      cp += bssScanInfo->ssid_len;
      *cp++ = '\0';
      entryIdx++;
    }
    return COMP_DISP_ERR_OK;
    break;
  case  BSS_INFO_SSID_LEN:
    break;
  case  BSS_INFO_CHANNEL:
    break;
  case  BSS_INFO_RSSI:
    compMsgData->msgValPart->fieldKeyValueStr = os_zalloc(compMsgData->msgDescPart->fieldSize);
    checkAllocOK(compMsgData->msgValPart->fieldKeyValueStr);
    cp = compMsgData->msgValPart->fieldKeyValueStr;
    result = compMsgData->compMsgDataView->dataView->setDataViewData(compMsgData->compMsgDataView->dataView, "getWifiKeyvalue", cp, 2 * sizeof(uint16_t) + sizeof(uint8_t));
    checkErrOK(result);
    result = compMsgData->compMsgDataView->dataView->setUint16(compMsgData->compMsgDataView->dataView, 0, compMsgData->msgDescPart->fieldKey);
    checkErrOK(result);
    result = compMsgData->compMsgDataView->dataView->setUint8(compMsgData->compMsgDataView->dataView, 2, compMsgData->msgDescPart->fieldType);
    checkErrOK(result);
    cp += 2 * sizeof(uint16_t) + sizeof(uint8_t);
    result = compMsgData->compMsgDataView->dataView->setUint16(compMsgData->compMsgDataView->dataView, 3, compMsgData->msgDescPart->fieldSize - (2 * sizeof(uint16_t) + sizeof(uint8_t)));
    checkErrOK(result);
    result = compMsgData->compMsgDataView->dataView->setDataViewData(compMsgData->compMsgDataView->dataView, saveWhere, saveData, saveLgth);
    checkErrOK(result);
    entryIdx = 0;
    while (entryIdx < self->bssScanInfos->numScanInfos) {
      bssScanInfo = &self->bssScanInfos->infos[entryIdx];
      *cp++ = bssScanInfo->rssi;
      entryIdx++;
    }
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
  return COMP_DISP_ERR_FIELD_NOT_FOUND;
}

// ================================= getWifiValue ====================================

static uint8_t getWifiValue(compMsgDispatcher_t *self, uint16_t which, uint8_t valueTypeId, int *numericValue, uint8_t **stringValue) {
  uint8_t result;

  *numericValue = 0;
  *stringValue = NULL;
  switch (which) {
  case WIFI_INFO_PROVISIONING_SSID:
    *stringValue = compMsgWifiData.provisioningSsid;
    break;
  case WIFI_INFO_PROVISIONING_PORT:
    *numericValue = compMsgWifiData.provisioningPort;
    break;
  case WIFI_INFO_PROVISIONING_IP_ADDR:
    *stringValue = compMsgWifiData.provisioningIPAddr;
    break;
  case WIFI_INFO_BINARY_CALL_BACK:
    *numericValue = (int)compMsgWifiData.websocketBinaryReceived;
    break;
  case WIFI_INFO_TEXT_CALL_BACK:
    *numericValue = (int)compMsgWifiData.websocketTextReceived;
    break;
  case WIFI_INFO_NET_RECEIVED_CALL_BACK:
    *numericValue = (int)compMsgWifiData.netsocketReceived;
    break;
  case WIFI_INFO_NET_TO_SEND_CALL_BACK:
    *numericValue = (int)compMsgWifiData.netsocketToSend;
    break;
  case WIFI_INFO_CLIENT_SSID:
    *stringValue = compMsgWifiData.clientSsid;
    break;
  case WIFI_INFO_CLIENT_PASSWD:
    *stringValue = compMsgWifiData.clientPasswd;
    break;
  case WIFI_INFO_CLIENT_IP_ADDR:
    *numericValue = compMsgWifiData.clientIPAddr;
    break;
  case WIFI_INFO_CLIENT_PORT:
    *numericValue = compMsgWifiData.clientPort;
    break;
  case WIFI_INFO_CLOUD_DOMAIN:
    *stringValue = compMsgWifiData.cloudDomain;
    break;
  case WIFI_INFO_CLOUD_PORT:
    *numericValue = compMsgWifiData.cloudPort;
    break;
  case WIFI_INFO_CLOUD_HOST_1:
    *stringValue = compMsgWifiData.cloudHost1;
    break;
  case WIFI_INFO_CLOUD_HOST_2:
    *stringValue = compMsgWifiData.cloudHost2;
    break;
  case WIFI_INFO_CLOUD_SUB_URL:
    *stringValue = compMsgWifiData.cloudSubUrl;
    break;
  case WIFI_INFO_CLOUD_NODE_TOKEN:
    *stringValue = compMsgWifiData.cloudNodeToken;
    break;
#ifdef CLIENT_SSL_ENABLE
  case WIFI_INFO_CLOUD_SECURE_CONNECT:
    *numericValue = compMsgWifiData.cloudSecureConnect;
    break;
#endif
  default:
ets_printf("bad which: %d\n", which);
    return COMP_DISP_ERR_BAD_WIFI_VALUE_WHICH;
    break;
  }
  return COMP_DISP_ERR_OK;
}

// ================================= getWifiRemotePort ====================================

static uint8_t getWifiRemotePort(compMsgDispatcher_t *self) {
  uint8_t result;

  if (self->compMsgData->wud == NULL) {
    return COMP_DISP_ERR_NO_WEBSOCKET_OPENED;
  }
  self->compMsgData->msgValPart->fieldFlags |= COMP_DISP_DESC_VALUE_IS_NUMBER;
  self->compMsgData->msgValPart->fieldValue = self->compMsgData->wud->remote_port;
  return COMP_DISP_ERR_OK;
}

// ================================= getWifiConfig ====================================

static uint8_t getWifiConfig(compMsgDispatcher_t *self) {
  uint8_t result;
  uint8_t *provisioningSsid;
  uint8_t provisioningPort;
  uint8_t *provisioningIPAddr;


#ifdef NOTDEF
  provisioningSsid = "testDevice_connect";
  c_memcpy(compMsgWifiData.provisioningSsid, provisioningSsid, c_strlen(provisioningSsid));
  compMsgWifiData.provisioningPort = 80;
  provisioningIPAddr = "192.168.4.1";
  c_memcpy(compMsgWifiData.provisioningIPAddr, provisioningIPAddr, c_strlen(provisioningIPAddr));
  compMsgWifiData.stationPort = 80;
#endif
  result = getStationConfig(self);
  checkErrOK(result);
  return COMP_DISP_ERR_OK;
}

// ================================= setWifiValue ====================================

static uint8_t setWifiValue(compMsgDispatcher_t *self, uint8_t *fieldNameStr, int numericValue, uint8_t *stringValue) {
  uint8_t result;
  uint8_t fieldNameId;
  compMsgDataView_t *dataView;

//ets_printf("setWifValue: %s %d %s\n", fieldNameStr, numericValue, stringValue == NULL ? "nil" : (char *)stringValue);
  dataView = self->compMsgDataView;
  result = dataView->getFieldNameIdFromStr(dataView, fieldNameStr, &fieldNameId, COMP_MSG_NO_INCR); 
  switch (fieldNameId) {
  case COMP_MSG_SPEC_FIELD_PROVISIONING_SSID:
    c_memcpy(compMsgWifiData.provisioningSsid, stringValue, c_strlen(stringValue));
    break;
  case COMP_MSG_SPEC_FIELD_PROVISIONING_PORT:
    compMsgWifiData.provisioningPort = numericValue;
    break;
  case COMP_MSG_SPEC_FIELD_PROVISIONING_IP_ADDR:
    c_memcpy(compMsgWifiData.provisioningIPAddr, stringValue, c_strlen(stringValue));
    break;
  case COMP_MSG_SPEC_FIELD_CLIENT_SSID:
    c_memcpy(compMsgWifiData.clientSsid, stringValue, c_strlen(stringValue));
    break;
  case COMP_MSG_SPEC_FIELD_CLIENT_PASSWD:
    c_memcpy(compMsgWifiData.clientPasswd, stringValue, c_strlen(stringValue));
    break;
  case COMP_MSG_SPEC_FIELD_CLIENT_IP_ADDR:
    compMsgWifiData.clientIPAddr = numericValue;
    break;
  case COMP_MSG_SPEC_FIELD_CLIENT_PORT:
    compMsgWifiData.clientPort = numericValue;
    break;
  case COMP_MSG_SPEC_FIELD_CLOUD_DOMAIN:
    compMsgWifiData.cloudDomain = os_zalloc(c_strlen(stringValue) + 1);
    checkAllocOK(compMsgWifiData.cloudDomain);
    c_memcpy(compMsgWifiData.cloudDomain, stringValue, c_strlen(stringValue));
    break;
  case COMP_MSG_SPEC_FIELD_CLOUD_PORT:
    compMsgWifiData.cloudPort = numericValue;
    break;
  case COMP_MSG_SPEC_FIELD_CLOUD_HOST_1:
    compMsgWifiData.cloudHost1 = os_zalloc(c_strlen(stringValue) + 1);
    checkAllocOK(compMsgWifiData.cloudHost1);
    c_memcpy(compMsgWifiData.cloudHost1, stringValue, c_strlen(stringValue));
    break;
  case COMP_MSG_SPEC_FIELD_CLOUD_HOST_2:
    compMsgWifiData.cloudHost2 = os_zalloc(c_strlen(stringValue) + 1);
    checkAllocOK(compMsgWifiData.cloudHost2);
    c_memcpy(compMsgWifiData.cloudHost2, stringValue, c_strlen(stringValue));
    break;
  case COMP_MSG_SPEC_FIELD_CLOUD_SUB_URL:
    compMsgWifiData.cloudSubUrl = os_zalloc(c_strlen(stringValue) + 1);
    checkAllocOK(compMsgWifiData.cloudSubUrl);
    c_memcpy(compMsgWifiData.cloudSubUrl, stringValue, c_strlen(stringValue));
    break;
  case COMP_MSG_SPEC_FIELD_CLOUD_NODE_TOKEN:
    compMsgWifiData.cloudNodeToken = os_zalloc(c_strlen(stringValue) + 1);
    checkAllocOK(compMsgWifiData.cloudNodeToken);
    c_memcpy(compMsgWifiData.cloudNodeToken, stringValue, c_strlen(stringValue));
    break;
#ifdef CLIENT_SSL_ENABLE
  case COMP_MSG_SPEC_FIELD_CLOUD_SECURE_CONNECT:
    compMsgWifiData.cloudSecureConnect = numericValue;
    break;
#endif
  default:
//ets_printf("setWifiValue: %s not found\n", fieldNameStr);
    return COMP_DISP_ERR_FIELD_NOT_FOUND;
  }
  return COMP_DISP_ERR_OK;
}

// ================================= compMsgWifiInit ====================================

uint8_t compMsgWifiInit(compMsgDispatcher_t *self) {
  uint8_t result;

  compMsgWifiData.websocketBinaryReceived = &websocketBinaryReceived;
  compMsgWifiData.websocketTextReceived = &websocketTextReceived;
  compMsgWifiData.netsocketToSend = &netsocketToSend;
  compMsgWifiData.netsocketReceived = &netsocketReceived;

  self->bssScanInfos = &bssScanInfos;
  self->getBssScanInfo = &getBssScanInfo;
  self->getScanInfoTableFieldValue = &getScanInfoTableFieldValue;
  self->getWifiValue = &getWifiValue;
  self->getWifiConfig = &getWifiConfig;
  self->setWifiValue = &setWifiValue;
  self->getWifiKeyValue = &getWifiKeyValue;
  self->getWifiKeyValueInfo = &getWifiKeyValueInfo;
  self->getWifiRemotePort = &getWifiRemotePort;
  self->bssStr2BssInfoId = &bssStr2BssInfoId;

  bssScanInfos.compMsgDispatcher = self;
  self->compMsgMsgDesc->getWifiKeyValueKeys(self, &compMsgWifiData);
  result = self->compMsgMsgDesc->readWifiValues(self, COMP_MSG_WIFI_VALUES_FILE_NAME);
  checkErrOK(result);
  result = self->getWifiConfig(self);
  checkErrOK(result);
  return COMP_DISP_ERR_OK;
}

