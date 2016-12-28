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

// ================================= webSocketBinaryReceived ====================================

static void webSocketBinaryReceived(void *arg, socketUserData_t *sud, char *pdata, unsigned short len) {
  compMsgDispatcher_t *compMsgDispatcher;
  compMsgDispatcher_t *self;
  uint8_t result;

  compMsgDispatcher = (compMsgDispatcher_t *)arg;
  self = compMsgDispatcher;
  COMP_MSG_DBG(self, "w", 1, "webSocketBinaryReceived: len: %d dispatcher: %p\n", len, compMsgDispatcher);
  result = self->resetMsgInfo(self, &self->compMsgData->received);
//  checkErrOK(result);
  result = self->getNewCompMsgDataPtr(self);
  self->compMsgData->sud = sud;
  COMP_MSG_DBG(self, "w", 1, "received compMsgData: %p sud: %p\n", self->compMsgData, self->compMsgData->sud);
  self->compMsgData->direction = COMP_MSG_RECEIVED_DATA;
  self->compMsgData->receivedData = os_zalloc(len);
  if (self->compMsgData->receivedData == NULL) {
    compMsgDispatcher->webSocketError = COMP_MSG_ERR_OUT_OF_MEMORY;
    return;
  }
  c_memcpy(self->compMsgData->receivedData, pdata, len);
  self->compMsgData->receivedLgth = (uint8_t)len;
  result = self->compMsgRequest->addRequest(self, COMP_DISP_INPUT_WEB_SOCKET, sud, self->compMsgData);
  COMP_MSG_DBG(self, "w", 1, "webSocketBinaryReceived end result: %d\n", result);
}

// ================================= netSocketToSend ====================================

static void netSocketToSend(void *arg, socketUserData_t *sud, char *pdata, unsigned short len) {
  compMsgDispatcher_t *compMsgDispatcher;
  compMsgDispatcher_t *self;
  uint8_t result;

  compMsgDispatcher = (compMsgDispatcher_t *)arg;
  self = compMsgDispatcher;
  COMP_MSG_DBG(self, "w", 1, "netSocketSend: len: %d dispatcher: %p", len, compMsgDispatcher);
  result = self->resetMsgInfo(self, &self->compMsgData->toSend);
//  checkErrOK(result);
  result = self->getNewCompMsgDataPtr(self);
  COMP_MSG_DBG(self, "w", 1, "send compMsgData: %p lgth: %d", self->compMsgData, len);
  self->compMsgData->sud = sud;
  self->compMsgData->direction = COMP_MSG_TO_SEND_DATA;
  self->compMsgData->u16CmdKey = 17220; // FIXME hard wired 'CD'
  self->compMsgData->toSendData = (uint8_t *)pdata;
  self->compMsgData->toSendLgth = (uint8_t)len;
  result = self->compMsgRequest->addRequest(self, COMP_DISP_INPUT_NET_SOCKET, sud, self->compMsgData);
  COMP_MSG_DBG(self, "w", 1, "netSocketSend end result: %d", result);
}

// ================================= netSocketReceived ====================================

static void netSocketReceived(void *arg, socketUserData_t *sud, char *pdata, unsigned short len) {
  compMsgDispatcher_t *compMsgDispatcher;
  compMsgDispatcher_t *self;
  uint8_t result;

  compMsgDispatcher = (compMsgDispatcher_t *)arg;
  self = compMsgDispatcher;
  COMP_MSG_DBG(self, "w", 1, "wifi netSocketReceived: len: %d dispatcher: %p", len, compMsgDispatcher);
  result = self->resetMsgInfo(self, &self->compMsgData->received);
//  checkErrOK(result);
  result = self->getNewCompMsgDataPtr(self);
  COMP_MSG_DBG(self, "w", 1, "received compMsgData: %p remote_port: %d receivedLgth: %d", self->compMsgData, sud->remote_port, self->compMsgData->receivedLgth);
  self->compMsgData->sud = sud;
  self->compMsgData->direction = COMP_MSG_RECEIVED_DATA;
  self->compMsgData->receivedData = (uint8_t *)pdata;
  self->compMsgData->receivedLgth = (uint8_t)len;
  result = self->compMsgRequest->addRequest(self, COMP_DISP_INPUT_NET_SOCKET, sud, self->compMsgData);
  COMP_MSG_DBG(self, "w", 1, "wifi netSocketReceived end result: %d", result);
}

// ================================= webSocketTextReceived ====================================

static void webSocketTextReceived(void *arg, socketUserData_t *sud, char *pdata, unsigned short len) {
  compMsgDispatcher_t *self;

  self = (compMsgDispatcher_t *)arg;
  COMP_MSG_DBG(self, "w", 2, "webSocketTextReceived: len: %d dispatcher: %p\n", len, self);
}

// ================================= bssScanDoneCb ====================================

static void bssScanDoneCb(void *arg, STATUS status) {
  struct bss_info *bss_link;
  uint8_t numEntries;
  bssScanInfo_t *scanInfo;

//COMP_MSG_DBG(self, "w", 1, "bssScanDoneCb bssScanRunning: arg: %p %d status: %d!\n", arg, bssScanRunning, status);
  if (arg == NULL) {
    return;
  }
  if (status != COMP_MSG_ERR_OK) {
    return;
  }
  if (bssScanRunning == false) {
    return;
  }
  if (bssScanRunning == true) {
    bssScanRunning = false;
  }
//COMP_MSG_DBG(self, "w", 1, "bssScanDoneCb bssScanRunning2: %d status: %d!\n", bssScanRunning, status);
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
  bssScanInfos.compMsgDispatcher->compMsgBuildMsg->buildMsg(bssScanInfos.compMsgDispatcher);
}

// ================================= getBssScanInfo ====================================

static uint8_t getBssScanInfo(compMsgDispatcher_t *self) {
  bool result;
  struct scan_config scan_config;

  COMP_MSG_DBG(self, "w", 2, "getBssScanInfo1: \n");
  if (bssScanRunning) {
    // silently ignore 
    return COMP_MSG_ERR_OK;
  }
  bssScanRunning = true;
  scan_config.ssid = NULL;
  scan_config.bssid = NULL;
  scan_config.channel = 0;
  scan_config.show_hidden = 1;
  self->bssScanInfos->scanInfoComplete = false;
  result = wifi_station_scan(&scan_config, bssScanDoneCb);
  COMP_MSG_DBG(self, "w", 2, "getBssScanInfo2: result: %d\n", result);
  if (result != true) {
    return COMP_MSG_ERR_STATION_SCAN;
  }
  COMP_MSG_DBG(self, "w", 2, "getBssScanInfo3:\n");
  return COMP_MSG_ERR_OK;
}


// ================================= connectToAP ====================================

static uint8_t connectToAP(compMsgDispatcher_t *self) {
  uint8_t result;
  bool boolResult;
  uint8_t *ssid;
  uint8_t *passwd;
  int numericValue;
  fieldsToSave_t *fieldsToSave;
  int idx;
  struct station_config *station_config;
  struct ip_info ip_info;
  char temp[64];

  COMP_MSG_DBG(self, "w", 2, "connectToAP:\n");
  ssid = NULL;
  passwd = NULL;
  idx = 0;
  while (idx < self->numFieldsToSave) {
    fieldsToSave = &self->fieldsToSave[idx];
    if (c_strcmp("clientSsid", fieldsToSave->fieldNameStr) == 0) {
      ssid = fieldsToSave->fieldValueStr;
    }
    if (c_strcmp("clientPassword", fieldsToSave->fieldNameStr) == 0) {
      passwd = fieldsToSave->fieldValueStr;
    }
    idx++;
  }
  COMP_MSG_DBG(self, "w", 1, "connectToAP: ssid: %s passwd: %s\n", ssid == NULL ? "nil" : (char *)ssid, passwd == NULL ? "nil" : (char *)passwd );
  result = self->compMsgWifiData->setWifiValue(self, "@clientSsid", 0, ssid);
  checkErrOK(result);
  result = self->compMsgWifiData->setWifiValue(self, "@clientPasswd", 0, passwd);
  checkErrOK(result);
  self->compMsgSendReceive->startSendMsg = self->compMsgIdentify->sendClientIPMsg;
  result = self->compMsgSocket->netSocketRunClientMode(self);
  COMP_MSG_DBG(self, "w", 2, "runClientMode: result: %d\n", result);
  checkErrOK(result);
  COMP_MSG_DBG(self, "w", 2, "connectToAP done\n");
  return COMP_MSG_ERR_OK;
}

// ================================= startStationCb ====================================

static uint8_t startStationCb(compMsgDispatcher_t *self) {
  uint8_t result;

  result = self->compMsgSendReceive->startSendMsg(self);
  checkErrOK(result);
  return COMP_MSG_ERR_OK;
}

// ================================= webSocketSendConnectError ====================================

static uint8_t webSocketSendConnectError(compMsgDispatcher_t *self, uint8_t status) {
  uint8_t result;
  uint8_t *handle;
  msgParts_t *received;

  COMP_MSG_DBG(self, "w", 2, "webSocketSendConnectError: status: %d\n", status);
  result = self->compMsgWifiData->setWifiValue(self, "@clientStatus", (int)status, NULL);
  checkErrOK(result);
  received = &self->compMsgData->received;
  result = self->compMsgIdentify->prepareAnswerMsg(self, COMP_MSG_NAK_MSG, &handle);
  checkErrOK(result);
  result = self->resetMsgInfo(self, received);
  COMP_MSG_DBG(self, "w", 2, "resetMsgInfo: result: %d\n", result);
  checkErrOK(result);
  COMP_MSG_DBG(self, "w", 2, "webSocketSendConnectError: done\n");
  return COMP_MSG_ERR_OK;
}

// ================================= netSocketSendConnectError ====================================

static uint8_t netSocketSendConnectError(compMsgDispatcher_t *self, uint8_t status) {
  uint8_t result;
  uint8_t *handle;
  msgParts_t *received;

  COMP_MSG_DBG(self, "w", 1, "netSocketSendConnectError: status: %d\n", status);
  result = self->compMsgWifiData->setWifiValue(self, "@clientStatus", (int)status, NULL);
  checkErrOK(result);
  received = &self->compMsgData->received;
  result = self->compMsgIdentify->prepareAnswerMsg(self, COMP_MSG_NAK_MSG, &handle);
  checkErrOK(result);
  result = self->resetMsgInfo(self, received);
  COMP_MSG_DBG(self, "w", 1, "resetMsgInfo: result: %d\n", result);
  checkErrOK(result);
  COMP_MSG_DBG(self, "w", 1, "netSocketSendConnectError done\n");
  return COMP_MSG_ERR_OK;
}

// ================================= getStationConfig ====================================

static uint8_t getStationConfig(compMsgDispatcher_t *self) {
  int result;
  struct station_config station_config;

  compMsgWifiData.wifiOpMode = wifi_get_opmode();
  result = wifi_station_get_config(&station_config);
  if (result != true) {
    return COMP_MSG_ERR_GET_STATION_CONFIG;
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

  return COMP_MSG_ERR_OK;
}

// ================================= getWifiAPBssidSize ====================================

static uint8_t getWifiAPBssidSize(compMsgDispatcher_t *self, int* numericValue, uint8_t **stringValue) {
  return COMP_MSG_ERR_OK;
}

// ================================= getWifiAPBssidStrSize ====================================

static uint8_t getWifiAPBssidStrSize(compMsgDispatcher_t *self, int* numericValue, uint8_t **stringValue) {
  return COMP_MSG_ERR_OK;
}

// ================================= getWifiAPSsidSize ====================================

static uint8_t getWifiAPSsidSize(compMsgDispatcher_t *self, int* numericValue, uint8_t **stringValue) {
  self->compMsgData->msgDescPart->fieldKey = compMsgWifiData.key_ssid;
  self->compMsgData->msgDescPart->fieldSize = compMsgWifiData.bssScanSizes.ssidSize;
  COMP_MSG_DBG(self, "w", 2, "ssidSize: %d\n", compMsgWifiData.bssScanSizes.ssidSize);
  *numericValue = compMsgWifiData.bssScanSizes.ssidSize;
  *stringValue = NULL;
  self->compMsgData->msgDescPart->fieldType = compMsgWifiData.bssScanTypes.ssidType;
  return COMP_MSG_ERR_OK;
}

// ================================= getWifiAPSsid_lenSize ====================================

static uint8_t getWifiAPSsid_lenSize(compMsgDispatcher_t *self, int* numericValue, uint8_t **stringValue) {
  return COMP_MSG_ERR_OK;
}

// ================================= getWifiAPChannelSize ====================================

static uint8_t getWifiAPChannelSize(compMsgDispatcher_t *self, int* numericValue, uint8_t **stringValue) {
  return COMP_MSG_ERR_OK;
}

// ================================= getWifiAPRssiSize ====================================

static uint8_t getWifiAPRssiSize(compMsgDispatcher_t *self, int* numericValue, uint8_t **stringValue) {
  self->compMsgData->msgDescPart->fieldKey = compMsgWifiData.key_rssi;
  self->compMsgData->msgDescPart->fieldSize = compMsgWifiData.bssScanSizes.rssiSize;
  COMP_MSG_DBG(self, "w", 2, "rssiSize: %d\n", compMsgWifiData.bssScanSizes.rssiSize);
  *numericValue = compMsgWifiData.bssScanSizes.rssiSize;
  *stringValue = NULL;
  self->compMsgData->msgDescPart->fieldType = compMsgWifiData.bssScanTypes.rssiType;
  return COMP_MSG_ERR_OK;
}

// ================================= getWifiAPAuthmodeSize ====================================

static uint8_t getWifiAPAuthmodeSize(compMsgDispatcher_t *self, int* numericValue, uint8_t **stringValue) {
  return COMP_MSG_ERR_OK;
}

// ================================= getWifiAPIs_hiddenSize ====================================

static uint8_t getWifiAPIs_hiddenSize(compMsgDispatcher_t *self, int* numericValue, uint8_t **stringValue) {
  return COMP_MSG_ERR_OK;
}

// ================================= getFreq_offsetSize ====================================

static uint8_t getWifiAPFreq_offsetSize(compMsgDispatcher_t *self, int* numericValue, uint8_t **stringValue) {
  return COMP_MSG_ERR_OK;
}

// ================================= getWifiAPFreqcal_valSize ====================================

static uint8_t getWifiAPFreqcal_valSize(compMsgDispatcher_t *self, int* numericValue, uint8_t **stringValue) {
  return COMP_MSG_ERR_OK;
}

// ================================= getWifiAPBssids ====================================

static uint8_t getWifiAPBssids(compMsgDispatcher_t *self, int* numericValue, uint8_t **stringValue) {
  return COMP_MSG_ERR_OK;
}

// ================================= getWifiAPBssidStrs ====================================

static uint8_t getWifiAPBssidStrs(compMsgDispatcher_t *self, int* numericValue, uint8_t **stringValue) {
  return COMP_MSG_ERR_OK;
}

// ================================= getWifiAPSsids ====================================

static uint8_t getWifiAPSsids(compMsgDispatcher_t *self, int* numericValue, uint8_t **stringValue) {
  compMsgData_t *compMsgData;
  bssScanInfo_t *bssScanInfo;
  uint8_t *cp;
  int entryIdx;

  compMsgData = self->compMsgData;
  compMsgData->msgValPart->fieldKeyValueStr = os_zalloc(compMsgData->msgDescPart->fieldSize);
  checkAllocOK(compMsgData->msgValPart->fieldKeyValueStr);
  entryIdx = 0;
  cp = compMsgData->msgValPart->fieldKeyValueStr;
  while (entryIdx < self->bssScanInfos->numScanInfos) {
    bssScanInfo = &self->bssScanInfos->infos[entryIdx];
    COMP_MSG_DBG(self, "w", 2, "ssid: entryIdx: %d %s!%d!\n", entryIdx, bssScanInfo->ssid, bssScanInfo->ssid_len);
    c_memcpy(cp, bssScanInfo->ssid, bssScanInfo->ssid_len);
    cp += bssScanInfo->ssid_len;
    *cp++ = '\0';
    entryIdx++;
  }
  *numericValue = 0;
  *stringValue = compMsgData->msgValPart->fieldKeyValueStr;
  COMP_MSG_DBG(self, "w", 2, "getWifiAPSsids: lgth: %d\n", cp-compMsgData->msgValPart->fieldKeyValueStr);
  return COMP_MSG_ERR_OK;
}

// ================================= getWifiAPSsid_lens ====================================

static uint8_t getWifiAPSsid_lens(compMsgDispatcher_t *self, int* numericValue, uint8_t **stringValue) {
  return COMP_MSG_ERR_OK;
}

// ================================= getWifiAPChannels ====================================

static uint8_t getWifiAPChannels(compMsgDispatcher_t *self, int* numericValue, uint8_t **stringValue) {
  return COMP_MSG_ERR_OK;
}

// ================================= getWifiAPRssis ====================================

static uint8_t getWifiAPRssis(compMsgDispatcher_t *self, int* numericValue, uint8_t **stringValue) {
  compMsgData_t *compMsgData;
  bssScanInfo_t *bssScanInfo;
  uint8_t *cp;
  int entryIdx;

  compMsgData = self->compMsgData;
  compMsgData->msgValPart->fieldKeyValueStr = os_zalloc(compMsgData->msgDescPart->fieldSize);
  checkAllocOK(compMsgData->msgValPart->fieldKeyValueStr);
  entryIdx = 0;
  cp = compMsgData->msgValPart->fieldKeyValueStr;
  while (entryIdx < self->bssScanInfos->numScanInfos) {
    bssScanInfo = &self->bssScanInfos->infos[entryIdx];
    *cp++ = bssScanInfo->rssi;
    entryIdx++;
  }
  *numericValue = 0;
  *stringValue = compMsgData->msgValPart->fieldKeyValueStr;
  COMP_MSG_DBG(self, "w", 2, "getWifiAPRssis: lgth: %d\n", cp-compMsgData->msgValPart->fieldKeyValueStr);
  return COMP_MSG_ERR_OK;
}

// ================================= getWifiAPAuthmodes ====================================

static uint8_t getWifiAPAuthmodes(compMsgDispatcher_t *self, int* numericValue, uint8_t **stringValue) {
  return COMP_MSG_ERR_OK;
}

// ================================= getWifiAPIs_hiddens ====================================

static uint8_t getWifiAPIs_hiddens(compMsgDispatcher_t *self, int* numericValue, uint8_t **stringValue) {
  return COMP_MSG_ERR_OK;
}

// ================================= getFreq_offsets ====================================

static uint8_t getWifiAPFreq_offsets(compMsgDispatcher_t *self, int* numericValue, uint8_t **stringValue) {
  return COMP_MSG_ERR_OK;
}

// ================================= getWifiAPFreqcal_vals ====================================

static uint8_t getWifiAPFreqcal_vals(compMsgDispatcher_t *self, int* numericValue, uint8_t **stringValue) {
  return COMP_MSG_ERR_OK;
}

// ================================= getClientIPAddr ====================================

static uint8_t getClientIPAddr(compMsgDispatcher_t *self, int* numericValue, uint8_t **stringValue) {
  *numericValue = compMsgWifiData.clientIPAddr;
  *stringValue = NULL;
  return COMP_MSG_ERR_OK;
}

// ================================= getClientPort ====================================

static uint8_t getClientPort(compMsgDispatcher_t *self, int* numericValue, uint8_t **stringValue) {
  *numericValue = compMsgWifiData.clientPort;
  *stringValue = NULL;
  return COMP_MSG_ERR_OK;
}

// ================================= getClientStatus ====================================

static uint8_t getClientStatus(compMsgDispatcher_t *self, int* numericValue, uint8_t **stringValue) {
  *numericValue = compMsgWifiData.clientStatus;
  *stringValue = NULL;
  return COMP_MSG_ERR_OK;
}

// ================================= getWifiKeyValue ====================================

static uint8_t xgetWifiKeyValue(compMsgDispatcher_t *self) {
  uint8_t result;
  uint8_t bssInfoType;
  int entryIdx;
  uint8_t *cp;
  bssScanInfo_t *bssScanInfo;
  compMsgData_t *compMsgData;
  dataView_t *dataView;
  size_t offset;

  compMsgData = self->compMsgData;
  result = bssStr2BssInfoId(compMsgData->msgValPart->fieldNameStr + c_strlen("#key_"), &bssInfoType);
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
    dataView = newDataView(cp, 2 * sizeof(uint16_t) + sizeof(uint8_t));
    checkAllocOK(dataView);
    offset = 0;
    result = dataView->setUint16(dataView, offset, compMsgData->msgDescPart->fieldKey);
    checkErrOK(result);
    offset += 2;
    result = dataView->setUint8(dataView, offset, compMsgData->msgDescPart->fieldType);
    checkErrOK(result);
    offset += 1;
    result = dataView->setUint16(dataView, offset, compMsgData->msgDescPart->fieldSize - (2 * sizeof(uint16_t) + sizeof(uint8_t)));
    checkErrOK(result);
    os_free(dataView);
    cp += 2 * sizeof(uint16_t) + sizeof(uint8_t);
    while (entryIdx < self->bssScanInfos->numScanInfos) {
      bssScanInfo = &self->bssScanInfos->infos[entryIdx];
      c_memcpy(cp, bssScanInfo->ssid, bssScanInfo->ssid_len);
      cp += bssScanInfo->ssid_len;
      *cp++ = '\0';
      entryIdx++;
    }
    return COMP_MSG_ERR_OK;
    break;
  case  BSS_INFO_SSID_LEN:
    break;
  case  BSS_INFO_CHANNEL:
    break;
  case  BSS_INFO_RSSI:
    compMsgData->msgValPart->fieldKeyValueStr = os_zalloc(compMsgData->msgDescPart->fieldSize);
    checkAllocOK(compMsgData->msgValPart->fieldKeyValueStr);
    cp = compMsgData->msgValPart->fieldKeyValueStr;
    dataView = newDataView(cp, 2 * sizeof(uint16_t) + sizeof(uint8_t));
    checkAllocOK(dataView);
    offset = 0;
    result = dataView->setUint16(dataView, offset, compMsgData->msgDescPart->fieldKey);
    checkErrOK(result);
    offset += 2;
    result = dataView->setUint8(dataView, offset, compMsgData->msgDescPart->fieldType);
    checkErrOK(result);
    offset += 1;
    result = dataView->setUint16(dataView, offset, compMsgData->msgDescPart->fieldSize - (2 * sizeof(uint16_t) + sizeof(uint8_t)));
    checkErrOK(result);
    os_free(dataView);
    cp += 2 * sizeof(uint16_t) + sizeof(uint8_t);
    entryIdx = 0;
    while (entryIdx < self->bssScanInfos->numScanInfos) {
      bssScanInfo = &self->bssScanInfos->infos[entryIdx];
      *cp++ = bssScanInfo->rssi;
      entryIdx++;
    }
    return COMP_MSG_ERR_OK;
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
  return COMP_MSG_ERR_FIELD_NOT_FOUND;
}

// ================================= getWifiValue ====================================

/**
 * \brief get a Wifi module value
 * \param self The dispatcher struct
 * \param which Which of the values
 * \param valueTypeId The values type
 * \param numericValue The value if it is a numeric one
 * \param stringValue The value if it is a character string
 * \return Error code or ErrorOK
 *
 */
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
    *numericValue = (int)compMsgWifiData.webSocketBinaryReceived;
    break;
  case WIFI_INFO_TEXT_CALL_BACK:
    *numericValue = (int)compMsgWifiData.webSocketTextReceived;
    break;
  case WIFI_INFO_NET_RECEIVED_CALL_BACK:
    *numericValue = (int)compMsgWifiData.netSocketReceived;
    break;
  case WIFI_INFO_NET_TO_SEND_CALL_BACK:
    *numericValue = (int)compMsgWifiData.netSocketToSend;
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
  case WIFI_INFO_CLIENT_STATUS:
    *numericValue = compMsgWifiData.clientStatus;
    break;
  case WIFI_INFO_CLOUD_PORT:
    *numericValue = compMsgWifiData.cloudPort;
    break;
  case WIFI_INFO_CLOUD_DOMAIN_1:
    *stringValue = compMsgWifiData.cloudDomain1;
    break;
  case WIFI_INFO_CLOUD_DOMAIN_2:
    *stringValue = compMsgWifiData.cloudDomain2;
    break;
  case WIFI_INFO_CLOUD_HOST_1:
    *stringValue = compMsgWifiData.cloudHost1;
    break;
  case WIFI_INFO_CLOUD_HOST_2:
    *stringValue = compMsgWifiData.cloudHost2;
    break;
  case WIFI_INFO_CLOUD_SUB_URL_1:
    *stringValue = compMsgWifiData.cloudSubUrl1;
    break;
  case WIFI_INFO_CLOUD_SUB_URL_2:
    *stringValue = compMsgWifiData.cloudSubUrl2;
    break;
  case WIFI_INFO_CLOUD_NODE_TOKEN_1:
    *stringValue = compMsgWifiData.cloudNodeToken1;
    break;
  case WIFI_INFO_CLOUD_NODE_TOKEN_2:
    *stringValue = compMsgWifiData.cloudNodeToken2;
    break;
#ifdef CLIENT_SSL_ENABLE
  case WIFI_INFO_CLOUD_SECURE_CONNECT:
    *numericValue = compMsgWifiData.cloudSecureConnect;
    break;
#endif
  default:
COMP_MSG_DBG(self, "w", 1, "bad which: %d\n", which);
    return COMP_MSG_ERR_BAD_WIFI_VALUE_WHICH;
    break;
  }
  return COMP_MSG_ERR_OK;
}

// ================================= getWifiRemotePort ====================================

static uint8_t getWifiRemotePort(compMsgDispatcher_t *self) {
  uint8_t result;

  if (self->compMsgData->sud == NULL) {
    return COMP_MSG_ERR_NO_WEBSOCKET_OPENED;
  }
  self->compMsgData->msgValPart->fieldFlags |= COMP_DISP_DESC_VALUE_IS_NUMBER;
  self->compMsgData->msgValPart->fieldValue = self->compMsgData->sud->remote_port;
  return COMP_MSG_ERR_OK;
}

// ================================= getWifiConfig ====================================

static uint8_t getWifiConfig(compMsgDispatcher_t *self) {
  uint8_t result;
  uint8_t *provisioningSsid;
  uint8_t provisioningPort;
  uint8_t *provisioningIPAddr;

  result = getStationConfig(self);
  checkErrOK(result);
  return COMP_MSG_ERR_OK;
}

// ================================= setWifiValue ====================================

static uint8_t setWifiValue(compMsgDispatcher_t *self, uint8_t *fieldNameStr, int numericValue, uint8_t *stringValue) {
  uint8_t result;
  uint8_t fieldNameId;

  COMP_MSG_DBG(self, "w", 2, "setWifValue: %s %d %s\n", fieldNameStr, numericValue, stringValue == NULL ? "nil" : (char *)stringValue);
  result = self->compMsgTypesAndNames->getFieldNameIdFromStr(self->compMsgTypesAndNames, fieldNameStr, &fieldNameId, COMP_MSG_NO_INCR); 
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
  case COMP_MSG_SPEC_FIELD_CLIENT_STATUS:
    compMsgWifiData.clientStatus = numericValue;
    break;
  case COMP_MSG_SPEC_FIELD_CLOUD_PORT:
    compMsgWifiData.cloudPort = numericValue;
    break;
  case COMP_MSG_SPEC_FIELD_CLOUD_DOMAIN_1:
    compMsgWifiData.cloudDomain1 = os_zalloc(c_strlen(stringValue) + 1);
    checkAllocOK(compMsgWifiData.cloudDomain1);
    c_memcpy(compMsgWifiData.cloudDomain1, stringValue, c_strlen(stringValue));
    break;
  case COMP_MSG_SPEC_FIELD_CLOUD_DOMAIN_2:
    compMsgWifiData.cloudDomain2 = os_zalloc(c_strlen(stringValue) + 1);
    checkAllocOK(compMsgWifiData.cloudDomain2);
    c_memcpy(compMsgWifiData.cloudDomain2, stringValue, c_strlen(stringValue));
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
  case COMP_MSG_SPEC_FIELD_CLOUD_SUB_URL_1:
    compMsgWifiData.cloudSubUrl1 = os_zalloc(c_strlen(stringValue) + 1);
    checkAllocOK(compMsgWifiData.cloudSubUrl1);
    c_memcpy(compMsgWifiData.cloudSubUrl1, stringValue, c_strlen(stringValue));
    break;
  case COMP_MSG_SPEC_FIELD_CLOUD_SUB_URL_2:
    compMsgWifiData.cloudSubUrl2 = os_zalloc(c_strlen(stringValue) + 1);
    checkAllocOK(compMsgWifiData.cloudSubUrl2);
    c_memcpy(compMsgWifiData.cloudSubUrl2, stringValue, c_strlen(stringValue));
    break;
  case COMP_MSG_SPEC_FIELD_CLOUD_NODE_TOKEN_1:
    compMsgWifiData.cloudNodeToken1 = os_zalloc(c_strlen(stringValue) + 1);
    checkAllocOK(compMsgWifiData.cloudNodeToken1);
    c_memcpy(compMsgWifiData.cloudNodeToken1, stringValue, c_strlen(stringValue));
    break;
  case COMP_MSG_SPEC_FIELD_CLOUD_NODE_TOKEN_2:
    compMsgWifiData.cloudNodeToken2 = os_zalloc(c_strlen(stringValue) + 1);
    checkAllocOK(compMsgWifiData.cloudNodeToken2);
    c_memcpy(compMsgWifiData.cloudNodeToken2, stringValue, c_strlen(stringValue));
    break;
#ifdef CLIENT_SSL_ENABLE
  case COMP_MSG_SPEC_FIELD_CLOUD_SECURE_CONNECT:
    compMsgWifiData.cloudSecureConnect = numericValue;
    break;
#endif
  default:
    COMP_MSG_DBG(self, "Y", 0, "setWifiValue: %s not found\n", fieldNameStr);
    return COMP_MSG_ERR_FIELD_NOT_FOUND;
  }
  return COMP_MSG_ERR_OK;
}

// ================================= compMsgWifiInit ====================================

uint8_t compMsgWifiInit(compMsgDispatcher_t *self) {
  uint8_t result;

  compMsgWifiData.webSocketBinaryReceived = &webSocketBinaryReceived;
  compMsgWifiData.webSocketTextReceived = &webSocketTextReceived;
  compMsgWifiData.netSocketToSend = &netSocketToSend;
  compMsgWifiData.netSocketReceived = &netSocketReceived;

  self->compMsgUtil->addFieldValueCallbackName(self, "@getWifiAPBssidSize",       &getWifiAPBssidSize, COMP_DISP_CALLBACK_TYPE_WIFI_AP_LIST_SIZE);
  self->compMsgUtil->addFieldValueCallbackName(self, "@getWifiAPBssidStrSize",    &getWifiAPBssidStrSize, COMP_DISP_CALLBACK_TYPE_WIFI_AP_LIST_SIZE);
  self->compMsgUtil->addFieldValueCallbackName(self, "@getWifiAPSsidSize",        &getWifiAPSsidSize, COMP_DISP_CALLBACK_TYPE_WIFI_AP_LIST_SIZE);
  self->compMsgUtil->addFieldValueCallbackName(self, "@getWifiAPSsid_lenSize",    &getWifiAPSsid_lenSize, COMP_DISP_CALLBACK_TYPE_WIFI_AP_LIST_SIZE);
  self->compMsgUtil->addFieldValueCallbackName(self, "@getWifiAPChannelSize",     &getWifiAPChannelSize, COMP_DISP_CALLBACK_TYPE_WIFI_AP_LIST_SIZE);
  self->compMsgUtil->addFieldValueCallbackName(self, "@getWifiAPRssiSize",        &getWifiAPRssiSize, COMP_DISP_CALLBACK_TYPE_WIFI_AP_LIST_SIZE);
  self->compMsgUtil->addFieldValueCallbackName(self, "@getWifiAPAuthmodeSize",    &getWifiAPAuthmodeSize, COMP_DISP_CALLBACK_TYPE_WIFI_AP_LIST_SIZE);
  self->compMsgUtil->addFieldValueCallbackName(self, "@getWifiAPIs_hiddenSize",   &getWifiAPIs_hiddenSize, COMP_DISP_CALLBACK_TYPE_WIFI_AP_LIST_SIZE);
  self->compMsgUtil->addFieldValueCallbackName(self, "@getWifiAPFreq_offsetSize", &getWifiAPFreq_offsetSize, COMP_DISP_CALLBACK_TYPE_WIFI_AP_LIST_SIZE);
  self->compMsgUtil->addFieldValueCallbackName(self, "@getWifiAPFreqcal_valSize", &getWifiAPFreqcal_valSize, COMP_DISP_CALLBACK_TYPE_WIFI_AP_LIST_SIZE);

  self->compMsgUtil->addFieldValueCallbackName(self, "@getWifiAPBssids",       &getWifiAPBssids, COMP_DISP_CALLBACK_TYPE_WIFI_AP_LIST_VALUE);
  self->compMsgUtil->addFieldValueCallbackName(self, "@getWifiAPBssidStrs",    &getWifiAPBssidStrs, COMP_DISP_CALLBACK_TYPE_WIFI_AP_LIST_VALUE);
  self->compMsgUtil->addFieldValueCallbackName(self, "@getWifiAPSsids",        &getWifiAPSsids, COMP_DISP_CALLBACK_TYPE_WIFI_AP_LIST_VALUE);
  self->compMsgUtil->addFieldValueCallbackName(self, "@getWifiAPSsid_lens",    &getWifiAPSsid_lens, COMP_DISP_CALLBACK_TYPE_WIFI_AP_LIST_VALUE);
  self->compMsgUtil->addFieldValueCallbackName(self, "@getWifiAPChannels",     &getWifiAPChannels, COMP_DISP_CALLBACK_TYPE_WIFI_AP_LIST_VALUE);
  self->compMsgUtil->addFieldValueCallbackName(self, "@getWifiAPRssis",        &getWifiAPRssis, COMP_DISP_CALLBACK_TYPE_WIFI_AP_LIST_VALUE);
  self->compMsgUtil->addFieldValueCallbackName(self, "@getWifiAPAuthmodes",    &getWifiAPAuthmodes, COMP_DISP_CALLBACK_TYPE_WIFI_AP_LIST_VALUE);
  self->compMsgUtil->addFieldValueCallbackName(self, "@getWifiAPIs_hiddens",   &getWifiAPIs_hiddens, COMP_DISP_CALLBACK_TYPE_WIFI_AP_LIST_VALUE);
  self->compMsgUtil->addFieldValueCallbackName(self, "@getWifiAPFreq_offsets", &getWifiAPFreq_offsets, COMP_DISP_CALLBACK_TYPE_WIFI_AP_LIST_VALUE);
  self->compMsgUtil->addFieldValueCallbackName(self, "@getWifiAPFreqcal_vals", &getWifiAPFreqcal_vals, COMP_DISP_CALLBACK_TYPE_WIFI_AP_LIST_VALUE);
  self->compMsgUtil->addFieldValueCallbackName(self, "@getClientIPAddr", &getClientIPAddr, COMP_DISP_CALLBACK_TYPE_WIFI_AP_LIST_VALUE);
  self->compMsgUtil->addFieldValueCallbackName(self, "@getClientPort", &getClientPort, COMP_DISP_CALLBACK_TYPE_WIFI_AP_LIST_VALUE);
  self->compMsgUtil->addFieldValueCallbackName(self, "@getClientStatus", &getClientStatus, COMP_DISP_CALLBACK_TYPE_WIFI_AP_LIST_VALUE);

  self->compMsgWifiData->getBssScanInfo = &getBssScanInfo;
  self->compMsgWifiData->getWifiValue = &getWifiValue;
  self->compMsgWifiData->getWifiConfig = &getWifiConfig;
  self->compMsgWifiData->setWifiValue = &setWifiValue;
  self->compMsgWifiData->getWifiRemotePort = &getWifiRemotePort;
  self->compMsgWifiData->bssStr2BssInfoId = &bssStr2BssInfoId;
  self->compMsgWifiData->connectToAP = &connectToAP;
  self->compMsgWifiData->startStationCb = &startStationCb;
  self->compMsgWifiData->webSocketSendConnectError = &webSocketSendConnectError;
  self->compMsgWifiData->netSocketSendConnectError = &netSocketSendConnectError;

  bssScanInfos.compMsgDispatcher = self;
  self->bssScanInfos = &bssScanInfos;
  self->compMsgMsgDesc->getWifiKeyValueKeys(self, &compMsgWifiData);
  result = self->compMsgMsgDesc->readWifiValues(self, COMP_MSG_WIFI_VALUES_FILE_NAME);
  checkErrOK(result);
  result = self->compMsgWifiData->getWifiConfig(self);
  checkErrOK(result);
  return COMP_MSG_ERR_OK;
}

// ================================= newCompMsgWifiData ====================================

compMsgWifiData_t *newCompMsgWifiData() {
  compMsgWifiData_t *compMsgWifiData = os_zalloc(sizeof(compMsgWifiData_t));
  if (compMsgWifiData == NULL) {
    return NULL;
  }
  return compMsgWifiData;
}
