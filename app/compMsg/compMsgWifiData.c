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

#include "compMsg.h"

static bool bssScanRunning = false;
static bssScanInfos_t bssScanInfos = { NULL, 0, 0};
static stationConfig_t stationConfig;
static compMsgWifiData_t compMsgWifiData;

const static str2id_t bssStr2BssInfoIds [] = {
  {"bssid",       BSS_INFO_BSSID},
  {"bssidStr",    BSS_INFO_BSSID_STR},
  {"ssid",        BSS_INFO_SSID},
  {"ssid_len",    BSS_INFO_SSID_LEN},
  {"channel",     BSS_INFO_CHANNEL},
  {"rssi",        BSS_INFO_RSSI},
  {"authmode",    BSS_INFO_AUTH_MODE},
  {"is_hidden",   BSS_INFO_IS_HIDDEN},
  {"freq_offset", BSS_INFO_FREQ_OFFSET},
  {"freqcal_val", BSS_INFO_FREQ_CAL_VAL},
  {NULL,          0 },
};

const static str2id_t keyValueStr2KeyValueIds [] = {
  {"ssid",                 KEY_VALUE_KEY_SSID},
  {"bssid",                KEY_VALUE_KEY_BSSID},
  {"authmode",             KEY_VALUE_KEY_AUTH_MODE},
  {"channel",              KEY_VALUE_KEY_CHANNEL},
  {"freq_offset",          KEY_VALUE_KEY_FREQ_OFFSET},
  {"freqcal_val",          KEY_VALUE_KEY_FREQ_CAL_VAL},
  {"rssi",                 KEY_VALUE_KEY_RSSI},
  {"is_hidden",            KEY_VALUE_KEY_IS_HIDDEN},
  {"clientSsid",           KEY_VALUE_KEY_CLIENT_SSID},
  {"clientPasswd",         KEY_VALUE_KEY_CLIENT_PASSWD},
  {"clientIPAddr",         KEY_VALUE_KEY_CLIENT_IP_ADDR},
  {"clientPort",           KEY_VALUE_KEY_CLIENT_PORT},
  {"clientStatus",         KEY_VALUE_KEY_CLIENT_STATUS},
  {"seqquenceNum",         KEY_VALUE_KEY_SEQUENCE_NUM},
  {"MACAddr",              KEY_VALUE_KEY_MAC_ADDR},
  {"machineState",         KEY_VALUE_KEY_MACHINE_STATE},
  {"firmwareMainBoard",    KEY_VALUE_KEY_FIRMWARE_MAIN_BOARD},
  {"firmwareDisplayBoard", KEY_VALUE_KEY_FIRMWARE_DISPLAY_BOARD},
  {"firmwareWifiModule",   KEY_VALUE_KEY_FIRMWARE_WIFI_MODULE},
  {"lastError",            KEY_VALUE_KEY_LAST_ERROR},
  {"casingUseList",        KEY_VALUE_KEY_CASING_USE_LIST},
  {"casingStatisticList",  KEY_VALUE_KEY_CASING_STATISTIC_LIST},
  {"dataAndTime",          KEY_VALUE_KEY_DAT_AND_TIME},
  {NULL,          0},
};

const static str2id_t callbackStr2CallbackIds [] = {
  {"@getWifiAPBssidSize",       COMP_MSG_WIFI_AP_BssidSize},
  {"@getWifiAPBssidStrSize",    COMP_MSG_WIFI_AP_BssidStrSize},
  {"@getWifiAPSsidSize",        COMP_MSG_WIFI_AP_SsidSize},
  {"@getWifiAPSsid_lenSize",    COMP_MSG_WIFI_AP_Ssid_lenSize},
  {"@getWifiAPChannelSize",     COMP_MSG_WIFI_AP_ChannelSize},
  {"@getWifiAPRssiSize",        COMP_MSG_WIFI_AP_RssiSize},
  {"@getWifiAPAuthmodeSize",    COMP_MSG_WIFI_AP_AuthmodeSize},
  {"@getWifiAPIs_hiddenSize",   COMP_MSG_WIFI_AP_Is_hiddenSize},
  {"@getWifiAPFreq_offsetSize", COMP_MSG_WIFI_AP_Freq_offsetSize},
  {"@getWifiAPFreqcal_valSize", COMP_MSG_WIFI_AP_Freqcal_valSize},

  {"@getWifiAPBssids",          COMP_MSG_WIFI_AP_Bssids},
  {"@getWifiAPBssidStrs",       COMP_MSG_WIFI_AP_BssidStrs},
  {"@getWifiAPSsids",           COMP_MSG_WIFI_AP_Ssids},
  {"@getWifiAPSsid_lens",       COMP_MSG_WIFI_AP_Ssid_lens},
  {"@getWifiAPChannels",        COMP_MSG_WIFI_AP_Channels},
  {"@getWifiAPRssis",           COMP_MSG_WIFI_AP_Rssis},
  {"@getWifiAPAuthmodes",       COMP_MSG_WIFI_AP_Authmodes},
  {"@getWifiAPIs_hiddens",      COMP_MSG_WIFI_AP_Is_hiddens},
  {"@getWifiAPFreq_offsets",    COMP_MSG_WIFI_AP_Freq_offsets},
  {"@getWifiAPFreqcal_vals",    COMP_MSG_WIFI_AP_Freqcal_vals},

  {"@getProvisioningSsid",      COMP_MSG_WIFI_ProvisioningSsid},
  {"@getProvisioningPort",      COMP_MSG_WIFI_ProvisioningPort},

  {"@getClientSsid",            COMP_MSG_WIFI_ClientSsid},
  {"@getClientPasswd",          COMP_MSG_WIFI_ClientPasswd},
  {"@getClientIPAddr",          COMP_MSG_WIFI_ClientIPAddr},
  {"@getClientPort",            COMP_MSG_WIFI_ClientPort},
  {"@getClientStatus",          COMP_MSG_WIFI_ClientStatus},
  {"@getClientSsidSize",        COMP_MSG_WIFI_ClientSsidSize},
  {"@getClientPasswdSize",      COMP_MSG_WIFI_ClientPasswdSize},
  {"@getClientIPAddrSize",      COMP_MSG_WIFI_ClientIPAddrSize},
  {"@getClientPortSize",        COMP_MSG_WIFI_ClientPortSize},
  {"@getClientStatusSize",      COMP_MSG_WIFI_ClientStatusSize},
  {"@getClientSequenceNumSize", COMP_MSG_WIFI_ClientSequenceNumSize},
  {"@getClientSequenceNum",     COMP_MSG_WIFI_ClientSequenceNum},

  {"@getSSDPIPAddr",            COMP_MSG_WIFI_SSDPIPAddr},
  {"@getSSDPPort",              COMP_MSG_WIFI_SSDPPort},
  {"@getSSDPStatus",            COMP_MSG_WIFI_SSDPStatus},

#include "compMsgWifiDataCustom.h"

  {NULL,          0},
};

// ================================= bssStr2BssInfoId ====================================

static uint8_t bssStr2BssInfoId(uint8_t *fieldName, uint8_t *fieldId) {
  int idx;
  const str2id_t *entry;

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

// ================================= keyValueStr2KeyValueId ====================================

static uint8_t keyValueStr2KeyValueId(uint8_t *fieldName, uint16_t *fieldId) {
  int idx;
  const str2id_t *entry;

  idx = 0;
  entry = &keyValueStr2KeyValueIds[idx];
  while (entry->str != NULL) {
    if (c_strcmp(entry->str, fieldName) == 0) {
      *fieldId = entry->id;
      return COMP_MSG_ERR_OK;
    }
    idx++;
    entry = &keyValueStr2KeyValueIds[idx];
  }
  return COMP_MSG_ERR_FIELD_NOT_FOUND;
}

// ================================= callbackStr2CallbackId ====================================

static uint8_t callbackStr2CallbackId(uint8_t *callbackName, uint16_t *callbackId) {
  int idx;
  const str2id_t *entry;

  idx = 0;
  entry = &callbackStr2CallbackIds[idx];
  while (entry->str != NULL) {
    if (c_strcmp(entry->str, callbackName) == 0) {
      *callbackId = entry->id;
      return COMP_MSG_ERR_OK;
    }
    idx++;
    entry = &callbackStr2CallbackIds[idx];
  }
  return COMP_MSG_ERR_CALLBACK_NAME_NOT_FOUND;
}

// ================================= webSocketBinaryReceived ====================================

static void webSocketBinaryReceived(void *arg, socketUserData_t *sud, char *pdata, unsigned short len) {
  compMsgDispatcher_t *compMsgDispatcher;
  compMsgDispatcher_t *self;
  uint8_t result;

//ets_printf("webSocketBinaryReceived1 heap: %d\n", system_get_free_heap_size());
  compMsgDispatcher = (compMsgDispatcher_t *)arg;
  self = compMsgDispatcher;
  COMP_MSG_DBG(self, "w", 1, "webSocketBinaryReceived: len: %d\n", len);
  result = self->resetMsgInfo(self, &self->compMsgData->received);
//  checkErrOK(result);
//ets_printf("webSocketBinaryReceived2 heap: %d\n", system_get_free_heap_size());
  self->compMsgData->sud = sud;
  COMP_MSG_DBG(self, "w", 2, "received compMsgData: %p sud: %p\n", self->compMsgData, self->compMsgData->sud);
  self->compMsgData->direction = COMP_MSG_RECEIVED_DATA;
//ets_printf("webSocketBinaryReceived3 heap: %d\n", system_get_free_heap_size());
  self->compMsgData->receivedData = os_zalloc(len);
  if (self->compMsgData->receivedData == NULL) {
    compMsgDispatcher->dispatcherCommon->webSocketError = COMP_MSG_ERR_OUT_OF_MEMORY;
    return;
  }
  c_memcpy(self->compMsgData->receivedData, pdata, len);
  self->compMsgData->receivedLgth = (uint8_t)len;
//ets_printf("webSocketBinaryReceived4 heap: %d\n", system_get_free_heap_size());
  result = self->compMsgRequest->addRequestData(self, COMP_MSG_INPUT_WEB_SOCKET, sud, pdata, len);
  COMP_MSG_DBG(self, "w", 2, "webSocketBinaryReceived end result: %d\n", result);
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
  COMP_MSG_DBG(self, "w", 1, "send compMsgData: %p lgth: %d", self->compMsgData, len);
  self->compMsgData->sud = sud;
  self->compMsgData->direction = COMP_MSG_TO_SEND_DATA;
  self->compMsgData->u16CmdKey = 17220; // FIXME hard wired 'CD'
  self->compMsgData->toSendData = (uint8_t *)pdata;
  self->compMsgData->toSendLgth = (uint8_t)len;
  result = self->compMsgRequest->addRequestData(self, COMP_MSG_INPUT_NET_SOCKET, sud, pdata, len);
//  checkErrOK(result);
  COMP_MSG_DBG(self, "w", 2, "netSocketSend end result: %d", result);
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
  result = self->compMsgRequest->addRequestData(self, COMP_MSG_INPUT_NET_SOCKET, sud, pdata, len);
  COMP_MSG_DBG(self, "w", 1, "wifi netSocketReceived end result: %d", result);
}

// ================================= netSocketSSDPToSend ====================================

static void netSocketSSDPToSend(void *arg, socketUserData_t *sud, char *pdata, unsigned short len) {
  compMsgDispatcher_t *compMsgDispatcher;
  compMsgDispatcher_t *self;
  uint8_t result;

  compMsgDispatcher = (compMsgDispatcher_t *)arg;
  self = compMsgDispatcher;
  COMP_MSG_DBG(self, "w", 1, "netSocketSSDPSend: len: %d dispatcher: %p", len, compMsgDispatcher);
//  result = self->resetMsgInfo(self, &self->compMsgData->toSend);
//  checkErrOK(result);
  COMP_MSG_DBG(self, "w", 1, "netSocketSSDPSend end result: %d", result);
}

// ================================= netSocketSSDPReceived ====================================

static void netSocketSSDPReceived(void *arg, socketUserData_t *sud, char *pdata, unsigned short len) {
  compMsgDispatcher_t *compMsgDispatcher;
  compMsgDispatcher_t *self;
  uint8_t result;

  compMsgDispatcher = (compMsgDispatcher_t *)arg;
  self = compMsgDispatcher;
  COMP_MSG_DBG(self, "w", 1, "wifi netSocketSSDPReceived: len: %d dispatcher: %p", len, compMsgDispatcher);
  result = self->resetMsgInfo(self, &self->compMsgData->received);
//  checkErrOK(result);
  COMP_MSG_DBG(self, "w", 1, "wifi netSocketSSDPReceived end result: %d", result);
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
    c_memset(scanInfo->ssid, '\t', sizeof(scanInfo->ssid));
    if (bss_link->ssid_len <= sizeof(scanInfo->ssid)) {
      c_memcpy(scanInfo->ssid, bss_link->ssid, bss_link->ssid_len);
      compMsgWifiData.bssScanSizes.ssidSize += bss_link->ssid_len + 2;
    } else {
      c_memcpy(scanInfo->ssid, bss_link->ssid, sizeof(scanInfo->ssid));
      compMsgWifiData.bssScanSizes.ssidSize += sizeof(scanInfo->ssid) + 2;
    }
    scanInfo->ssid_len = bss_link->ssid_len;
    c_memset(scanInfo->bssidStr, '\t', sizeof(scanInfo->bssidStr));
    c_memcpy(scanInfo->bssid, bss_link->bssid, sizeof(scanInfo->bssid));
    compMsgWifiData.bssScanSizes.bssidSize += sizeof(scanInfo->bssid) + 2;
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
bssScanInfos.compMsgDispatcher = self;
  bssScanRunning = true;
  scan_config.ssid = NULL;
  scan_config.bssid = NULL;
  scan_config.channel = 0;
  scan_config.show_hidden = 1;
  self->dispatcherCommon->bssScanInfos->scanInfoComplete = false;
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
  uint32_t sequenceNum;
  int numericValue;
  fieldsToSave_t *fieldsToSave;
  int idx;
  struct station_config *station_config;
  struct ip_info ip_info;
  char temp[64];
  fieldValue_t fieldValue;
  int interval;
  int timerId;

  COMP_MSG_DBG(self, "w", 2, "connectToAP:\n");
  ssid = NULL;
  passwd = NULL;
  idx = 0;
  while (idx < self->dispatcherCommon->numFieldsToSave) {
    fieldsToSave = &self->dispatcherCommon->fieldsToSave[idx];
    if (c_strcmp("#key_clientSsid", fieldsToSave->fieldNameStr) == 0) {
      ssid = fieldsToSave->fieldValueStr;
      fieldValue.flags = COMP_MSG_FIELD_IS_STRING;
      fieldValue.dataValue.value.stringValue = ssid;
      fieldValue.cmdKey = COMP_MSG_DATA_VALUE_CMD_KEY_SPECIAL;
      fieldValue.fieldNameId = 0;
      fieldValue.fieldValueId = COMP_MSG_WIFI_VALUE_ID_clientSsid;
      fieldValue.fieldValueCallback = NULL;
      result = self->compMsgDataValue->setFieldValueInfo(self, &fieldValue);
      checkErrOK(result);
    }
    if (c_strcmp("#key_clientPasswd", fieldsToSave->fieldNameStr) == 0) {
      passwd = fieldsToSave->fieldValueStr;
      fieldValue.flags = COMP_MSG_FIELD_IS_STRING;
      fieldValue.dataValue.value.stringValue = passwd;
      fieldValue.cmdKey = COMP_MSG_DATA_VALUE_CMD_KEY_SPECIAL;
      fieldValue.fieldNameId = 0;
      fieldValue.fieldValueId = COMP_MSG_WIFI_VALUE_ID_clientPasswd;
      fieldValue.fieldValueCallback = NULL;
      result = self->compMsgDataValue->setFieldValueInfo(self, &fieldValue);
      checkErrOK(result);
    }
    if (c_strcmp("#key_sequenceNum", fieldsToSave->fieldNameStr) == 0) {
//ets_printf("seq: %p %d\n", fieldsToSave->fieldValueStr, fieldsToSave->fieldValue);
      sequenceNum = fieldsToSave->fieldValue;
      fieldValue.flags = COMP_MSG_FIELD_IS_NUMERIC;
      fieldValue.dataValue.value.numericValue = sequenceNum;
      fieldValue.cmdKey = COMP_MSG_DATA_VALUE_CMD_KEY_SPECIAL;
      fieldValue.fieldNameId = 0;
      fieldValue.fieldValueId = COMP_MSG_WIFI_VALUE_ID_clientSequenceNum;
      fieldValue.fieldValueCallback = NULL;
      result = self->compMsgDataValue->setFieldValueInfo(self, &fieldValue);
      checkErrOK(result);
      self->compMsgWifiData->flags |= WIFI_USE_SAVED_SEQUENCE_NUM;
    }
    idx++;
  }
  COMP_MSG_DBG(self, "w", 1, "connectToAP: ssid: %s passwd: %s\n", ssid == NULL ? "nil" : (char *)ssid, passwd == NULL ? "nil" : (char *)passwd );
  self->compMsgSendReceive->startSendMsg = self->compMsgIdentify->sendClientIPMsg;
//ets_printf(">>>call netSocketRunClientMode %p!\n", self->compMsgSocket->netSocketRunClientMode);

  result = self->compMsgSocket->webSocketRunClientMode(self);
  COMP_MSG_DBG(self, "w", 1, "webSocketRunClientMode: result: %d\n", result);
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
  fieldValue_t fieldValue;
  msgParts_t *received;

  COMP_MSG_DBG(self, "w", 1, "webSocketSendConnectError: status: %d\n", status);
  fieldValue.flags = COMP_MSG_FIELD_IS_NUMERIC;
  fieldValue.dataValue.value.numericValue = status;
  fieldValue.cmdKey = COMP_MSG_DATA_VALUE_CMD_KEY_SPECIAL;
  fieldValue.fieldNameId = 0;
  fieldValue.fieldValueId = COMP_MSG_WIFI_VALUE_ID_clientStatus;
  fieldValue.fieldValueCallback = NULL;
  result = self->compMsgDataValue->setFieldValueInfo(self, &fieldValue);
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
  fieldValue_t fieldValue;
  msgParts_t *received;

  COMP_MSG_DBG(self, "w", 1, "netSocketSendConnectError: status: %d\n", status);
  fieldValue.flags = COMP_MSG_FIELD_IS_NUMERIC;
  fieldValue.dataValue.value.numericValue = status;
  fieldValue.cmdKey = COMP_MSG_DATA_VALUE_CMD_KEY_SPECIAL;
  fieldValue.fieldNameId = 0;
  fieldValue.fieldValueId = COMP_MSG_WIFI_VALUE_ID_clientStatus;
  fieldValue.fieldValueCallback = NULL;
  result = self->compMsgDataValue->setFieldValueInfo(self, &fieldValue);
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

// ================================= getClientSsidSize ====================================

static uint8_t getClientSsidSize(compMsgDispatcher_t *self, int* numericValue, uint8_t **stringValue) {
  self->compMsgData->msgDescPart->fieldKey = self->compMsgWifiData->keyValueInfo.key_clientSsid;
  self->compMsgData->msgDescPart->fieldSize = sizeof(self->compMsgWifiData->clientSsid) - 1;
  COMP_MSG_DBG(self, "w", 2, "clientSsidSize: %d key: %d\n", self->compMsgData->msgDescPart->fieldSize, self->compMsgData->msgDescPart->fieldKey);
  *numericValue = self->compMsgData->msgDescPart->fieldSize;
  *stringValue = NULL;
  self->compMsgData->msgDescPart->fieldType = self->compMsgWifiData->keyValueInfo.key_type_clientSsid;
  return COMP_MSG_ERR_OK;
}

// ================================= getClientPasswdSize ====================================

static uint8_t getClientPasswdSize(compMsgDispatcher_t *self, int* numericValue, uint8_t **stringValue) {
  self->compMsgData->msgDescPart->fieldKey = self->compMsgWifiData->keyValueInfo.key_clientPasswd;
  self->compMsgData->msgDescPart->fieldSize = sizeof(self->compMsgWifiData->clientPasswd) - 1;
  COMP_MSG_DBG(self, "w", 2, "clientPasswdSize: %d key: %d\n", self->compMsgData->msgDescPart->fieldSize, self->compMsgData->msgDescPart->fieldKey);
  *numericValue = self->compMsgData->msgDescPart->fieldSize;
  *stringValue = NULL;
  self->compMsgData->msgDescPart->fieldType = self->compMsgWifiData->keyValueInfo.key_type_clientPasswd;
  return COMP_MSG_ERR_OK;
}

// ================================= getClientIPAddrSize ====================================

static uint8_t getClientIPAddrSize(compMsgDispatcher_t *self, int* numericValue, uint8_t **stringValue) {
  self->compMsgData->msgDescPart->fieldKey = self->compMsgWifiData->keyValueInfo.key_clientIPAddr;
  self->compMsgData->msgDescPart->fieldSize = sizeof(self->compMsgWifiData->clientIPAddr);
  COMP_MSG_DBG(self, "w", 2, "clientIPAddrSize: %d key: %d\n", self->compMsgData->msgDescPart->fieldSize, self->compMsgData->msgDescPart->fieldKey);
  *numericValue = self->compMsgData->msgDescPart->fieldSize;
  *stringValue = NULL;
  self->compMsgData->msgDescPart->fieldType = self->compMsgWifiData->keyValueInfo.key_type_clientIPAddr;
//ets_printf("IPAddr type: %d %d\n", self->compMsgData->msgDescPart->fieldType, DATA_VIEW_FIELD_UINT32_T);
  return COMP_MSG_ERR_OK;
}

// ================================= getClientPortSize ====================================

static uint8_t getClientPortSize(compMsgDispatcher_t *self, int* numericValue, uint8_t **stringValue) {
  self->compMsgData->msgDescPart->fieldKey = self->compMsgWifiData->keyValueInfo.key_clientPort;
  self->compMsgData->msgDescPart->fieldSize = sizeof(self->compMsgWifiData->clientPort);
  COMP_MSG_DBG(self, "w", 2, "clientPortSize: %d key: %d\n", self->compMsgData->msgDescPart->fieldSize, self->compMsgData->msgDescPart->fieldKey);
  *numericValue = self->compMsgData->msgDescPart->fieldSize;
  *stringValue = NULL;
  self->compMsgData->msgDescPart->fieldType = self->compMsgWifiData->keyValueInfo.key_type_clientPort;
  return COMP_MSG_ERR_OK;
}

// ================================= getClientSequenceNumSize ====================================

static uint8_t getClientSequenceNumSize(compMsgDispatcher_t *self, int* numericValue, uint8_t **stringValue) {
  self->compMsgData->msgDescPart->fieldKey = self->compMsgWifiData->keyValueInfo.key_sequenceNum;
  self->compMsgData->msgDescPart->fieldSize = sizeof(uint32_t);
  COMP_MSG_DBG(self, "w", 2, "clientSequenceNumSize: %d key: %d\n", self->compMsgData->msgDescPart->fieldSize, self->compMsgData->msgDescPart->fieldKey);
  *numericValue = self->compMsgData->msgDescPart->fieldSize;
  *stringValue = NULL;
  self->compMsgData->msgDescPart->fieldType = self->compMsgWifiData->keyValueInfo.key_type_sequenceNum;
  return COMP_MSG_ERR_OK;
}

// ================================= getClientStatusSize ====================================

static uint8_t getClientStatusSize(compMsgDispatcher_t *self, int* numericValue, uint8_t **stringValue) {
  self->compMsgData->msgDescPart->fieldKey = self->compMsgWifiData->keyValueInfo.key_clientStatus;
  self->compMsgData->msgDescPart->fieldSize = sizeof(self->compMsgWifiData->clientStatus);
  COMP_MSG_DBG(self, "w", 2, "clientStatusSize: %d key: %d\n", self->compMsgData->msgDescPart->fieldSize, self->compMsgData->msgDescPart->fieldKey);
  *numericValue = self->compMsgData->msgDescPart->fieldSize;
  *stringValue = NULL;
  self->compMsgData->msgDescPart->fieldType = self->compMsgWifiData->keyValueInfo.key_type_clientStatus;
//ets_printf("getClientStatusSize: type: %d\n", self->compMsgWifiData->keyValueInfo.key_type_clientStatus);
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
  self->compMsgData->msgDescPart->fieldKey = compMsgWifiData.keyValueInfo.key_ssid;
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
  self->compMsgData->msgDescPart->fieldKey = compMsgWifiData.keyValueInfo.key_rssi;
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
  while (entryIdx < self->dispatcherCommon->bssScanInfos->numScanInfos) {
    bssScanInfo = &self->dispatcherCommon->bssScanInfos->infos[entryIdx];
    COMP_MSG_DBG(self, "w", 2, "ssid: entryIdx: %d %s!%d!\n", entryIdx, bssScanInfo->ssid, bssScanInfo->ssid_len);
    c_memcpy(cp, bssScanInfo->ssid, bssScanInfo->ssid_len);
    cp += bssScanInfo->ssid_len;
    *cp++ = '\t';
    *cp++ = '\t';
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
  while (entryIdx < self->dispatcherCommon->bssScanInfos->numScanInfos) {
    bssScanInfo = &self->dispatcherCommon->bssScanInfos->infos[entryIdx];
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

// ================================= getSSDPIPAddr ====================================

static uint8_t getSSDPIPAddr(compMsgDispatcher_t *self, int* numericValue, uint8_t **stringValue) {
  *numericValue = compMsgWifiData.ssdpIPAddr;
  *stringValue = NULL;
  return COMP_MSG_ERR_OK;
}

// ================================= getSSDPPort ====================================

static uint8_t getSSDPPort(compMsgDispatcher_t *self, int* numericValue, uint8_t **stringValue) {
  *numericValue = compMsgWifiData.ssdpPort;
  *stringValue = NULL;
  return COMP_MSG_ERR_OK;
}

// ================================= getSSDPStatus ====================================

static uint8_t getSSDPStatus(compMsgDispatcher_t *self, int* numericValue, uint8_t **stringValue) {
  *numericValue = compMsgWifiData.ssdpStatus;
  *stringValue = NULL;
  return COMP_MSG_ERR_OK;
}

// ================================= getClientSequenceNum ====================================

static uint8_t getClientSequenceNum(compMsgDispatcher_t *self, int* numericValue, uint8_t **stringValue) {
  int fieldsToSaveIdx;
  fieldsToSave_t *fieldsToSave;

  fieldsToSaveIdx = 0;
  while (fieldsToSaveIdx < self->dispatcherCommon->numFieldsToSave) {
    fieldsToSave = &self->dispatcherCommon->fieldsToSave[fieldsToSaveIdx];
    COMP_MSG_DBG(self, "I", 2, "fieldsToSave: %s!\n", fieldsToSave->fieldNameStr);
    if (c_strcmp("@#sequenceNum", fieldsToSave->fieldNameStr) == 0) {
      *stringValue = fieldsToSave->fieldValueStr;
      *numericValue = fieldsToSave->fieldValue;
      return COMP_MSG_ERR_OK;
    }
    fieldsToSaveIdx++;
  }
  *numericValue = compMsgWifiData.clientSequenceNum;
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
    while (entryIdx < self->dispatcherCommon->bssScanInfos->numScanInfos) {
      bssScanInfo = &self->dispatcherCommon->bssScanInfos->infos[entryIdx];
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
    while (entryIdx < self->dispatcherCommon->bssScanInfos->numScanInfos) {
      bssScanInfo = &self->dispatcherCommon->bssScanInfos->infos[entryIdx];
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

// ================================= getClientSsid ====================================

static uint8_t getClientSsid(compMsgDispatcher_t *self, int* numericValue, uint8_t **stringValue) {
  *numericValue = 0;
  *stringValue = compMsgWifiData.clientSsid;
  return COMP_MSG_ERR_OK;
}

// ================================= getClientPasswd ====================================

static uint8_t getClientPasswd(compMsgDispatcher_t *self, int* numericValue, uint8_t **stringValue) {
  *numericValue = 0;
  *stringValue = compMsgWifiData.clientPasswd;
  return COMP_MSG_ERR_OK;
}

// ================================= getProvisioningPort ====================================

static uint8_t getProvisioningPort(compMsgDispatcher_t *self, int* numericValue, uint8_t **stringValue) {
  *numericValue = compMsgWifiData.provisioningPort;
  *stringValue = NULL;
  return COMP_MSG_ERR_OK;
}

// ================================= getProvisioningSsid ====================================

static uint8_t getProvisioningSsid(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
  *numericValue = 0;
  *stringValue = compMsgWifiData.provisioningSsid;
  return COMP_MSG_ERR_OK;
}

#include "compMsgWifiDataCustom_c.inc"

// ================================= getWifiValue ====================================

/**
 * \brief get a Wifi module value
 * \param self The dispatcher struct
 * \param fieldvalueId Field id of the value
 * \param fieldValueCallback The field value callback, if there exists one
 * \param numericValue The value if it is a numeric one
 * \param stringValue The value if it is a character string
 * \return Error code or ErrorOK
 *
 */
static uint8_t getWifiValue(compMsgDispatcher_t *self, uint8_t fieldValueId, uint8_t *flags, fieldValueCallback_t *fieldValueCallback, int *numericValue, uint8_t **stringValue) {
  uint8_t result;
  fieldValue_t fieldValue;
  uint8_t *strValue;
  fieldValueCallback_t callback;

  *numericValue = 0;
  *stringValue = NULL;
  *fieldValueCallback = NULL;
  fieldValue.flags = 0;
  fieldValue.dataValue.value.stringValue = NULL;
  fieldValue.cmdKey = COMP_MSG_DATA_VALUE_CMD_KEY_SPECIAL;
  fieldValue.fieldNameId = 0;
  fieldValue.fieldValueId = fieldValueId;
  fieldValue.fieldValueCallback = NULL;
  result = self->compMsgDataValue->getFieldValueInfo(self, &fieldValue, &strValue);
  checkErrOK(result);
  if (fieldValue.flags & COMP_MSG_FIELD_IS_STRING) {
    *stringValue = fieldValue.dataValue.value.stringValue;
  } else {
    *numericValue = fieldValue.dataValue.value.numericValue;
  }
  *flags = fieldValue.flags;
  *fieldValueCallback = fieldValue.fieldValueCallback;
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

// ================================= compMsgWifiInit ====================================

static uint8_t compMsgWifiDataInit(compMsgDispatcher_t *self) {
  uint8_t result;
  compMsgUtil_t *compMsgUtil;
  fieldValue_t fieldValue;
  compMsgWifiData_t *compMsgWifiData;
  compMsgDataValue_t *compMsgDataValue;

//ets_printf("wifi 01 heap: %d\n", system_get_free_heap_size());
  compMsgWifiData = self->compMsgWifiData;
  compMsgWifiData->webSocketBinaryReceived = &webSocketBinaryReceived;
  compMsgWifiData->webSocketTextReceived = &webSocketTextReceived;
  compMsgWifiData->netSocketToSend = &netSocketToSend;
  compMsgWifiData->netSocketReceived = &netSocketReceived;
  compMsgWifiData->netSocketSSDPToSend = &netSocketSSDPToSend;
  compMsgWifiData->netSocketSSDPReceived = &netSocketSSDPReceived;

  compMsgDataValue = self->compMsgDataValue;

  c_memset(&fieldValue, 0, sizeof(fieldValue_t));
  fieldValue.flags = COMP_MSG_FIELD_IS_STRING;
  fieldValue.dataValue.value.stringValue = "";
  fieldValue.cmdKey = COMP_MSG_DATA_VALUE_CMD_KEY_SPECIAL;
  fieldValue.fieldNameId = 0;
  fieldValue.fieldValueId = COMP_MSG_WIFI_VALUE_ID_clientSsid;
  fieldValue.fieldValueCallback = NULL;
  result = compMsgDataValue->addFieldValueInfo(self, &fieldValue);
  checkErrOK(result);

  c_memset(&fieldValue, 0, sizeof(fieldValue_t));
  fieldValue.flags = COMP_MSG_FIELD_IS_STRING;
  fieldValue.dataValue.value.stringValue = "";
  fieldValue.cmdKey = COMP_MSG_DATA_VALUE_CMD_KEY_SPECIAL;
  fieldValue.fieldNameId = 0;
  fieldValue.fieldValueId = COMP_MSG_WIFI_VALUE_ID_clientPasswd;
  fieldValue.fieldValueCallback = NULL;
  result = compMsgDataValue->addFieldValueInfo(self, &fieldValue);
  checkErrOK(result);

  c_memset(&fieldValue, 0, sizeof(fieldValue_t));
  fieldValue.flags = COMP_MSG_FIELD_IS_NUMERIC;
  fieldValue.dataValue.value.numericValue = 0;
  fieldValue.cmdKey = COMP_MSG_DATA_VALUE_CMD_KEY_SPECIAL;
  fieldValue.fieldNameId = 0;
  fieldValue.fieldValueId = COMP_MSG_WIFI_VALUE_ID_clientSequenceNum;
  fieldValue.fieldValueCallback = NULL;
  result = compMsgDataValue->addFieldValueInfo(self, &fieldValue);
  checkErrOK(result);

  c_memset(&fieldValue, 0, sizeof(fieldValue_t));
  fieldValue.flags = COMP_MSG_FIELD_IS_NUMERIC;
  fieldValue.dataValue.value.numericValue = 0;
  fieldValue.cmdKey = COMP_MSG_DATA_VALUE_CMD_KEY_SPECIAL;
  fieldValue.fieldNameId = 0;
  fieldValue.fieldValueId = COMP_MSG_WIFI_VALUE_ID_clientStatus;
  fieldValue.fieldValueCallback = NULL;
  result = compMsgDataValue->addFieldValueInfo(self, &fieldValue);
  checkErrOK(result);

  c_memset(&fieldValue, 0, sizeof(fieldValue_t));
  fieldValue.flags = COMP_MSG_FIELD_IS_NUMERIC;
  fieldValue.dataValue.value.numericValue = (int)(&netSocketSSDPReceived);
  fieldValue.cmdKey = COMP_MSG_DATA_VALUE_CMD_KEY_SPECIAL;
  fieldValue.fieldNameId = 0;
  fieldValue.fieldValueId = COMP_MSG_WIFI_VALUE_ID_SSDPReceivedCallback;
  fieldValue.fieldValueCallback = NULL;
  result = compMsgDataValue->addFieldValueInfo(self, &fieldValue);
  checkErrOK(result);

  c_memset(&fieldValue, 0, sizeof(fieldValue_t));
  fieldValue.flags = COMP_MSG_FIELD_IS_NUMERIC;
  fieldValue.dataValue.value.numericValue = (int)(&netSocketSSDPToSend);
  fieldValue.cmdKey = COMP_MSG_DATA_VALUE_CMD_KEY_SPECIAL;
  fieldValue.fieldNameId = 0;
  fieldValue.fieldValueId = COMP_MSG_WIFI_VALUE_ID_SSDPToSendCallback;
  fieldValue.fieldValueCallback = NULL;
  result = compMsgDataValue->addFieldValueInfo(self, &fieldValue);
  checkErrOK(result);

  c_memset(&fieldValue, 0, sizeof(fieldValue_t));
  fieldValue.flags = COMP_MSG_FIELD_IS_NUMERIC;
  fieldValue.dataValue.value.numericValue = (int)(&netSocketReceived);
  fieldValue.cmdKey = COMP_MSG_DATA_VALUE_CMD_KEY_SPECIAL;
  fieldValue.fieldNameId = 0;
  fieldValue.fieldValueId = COMP_MSG_WIFI_VALUE_ID_NetReceivedCallback;
  fieldValue.fieldValueCallback = NULL;
  result = compMsgDataValue->addFieldValueInfo(self, &fieldValue);
  checkErrOK(result);

  c_memset(&fieldValue, 0, sizeof(fieldValue_t));
  fieldValue.flags = COMP_MSG_FIELD_IS_NUMERIC;
  fieldValue.dataValue.value.numericValue = (int)(&netSocketToSend);
  fieldValue.cmdKey = COMP_MSG_DATA_VALUE_CMD_KEY_SPECIAL;
  fieldValue.fieldNameId = 0;
  fieldValue.fieldValueId = COMP_MSG_WIFI_VALUE_ID_NetToSendCallback;
  fieldValue.fieldValueCallback = NULL;
  result = compMsgDataValue->addFieldValueInfo(self, &fieldValue);
  checkErrOK(result);

  c_memset(&fieldValue, 0, sizeof(fieldValue_t));
  fieldValue.flags = COMP_MSG_FIELD_IS_NUMERIC;
  fieldValue.dataValue.value.numericValue = (int)(&webSocketBinaryReceived);
  fieldValue.cmdKey = COMP_MSG_DATA_VALUE_CMD_KEY_SPECIAL;
  fieldValue.fieldNameId = 0;
  fieldValue.fieldValueId = COMP_MSG_WIFI_VALUE_ID_binaryCallback;
  fieldValue.fieldValueCallback = NULL;
  result = compMsgDataValue->addFieldValueInfo(self, &fieldValue);
  checkErrOK(result);

  c_memset(&fieldValue, 0, sizeof(fieldValue_t));
  fieldValue.flags = COMP_MSG_FIELD_IS_NUMERIC;
  fieldValue.dataValue.value.numericValue = (int)(&webSocketTextReceived);
  fieldValue.cmdKey = COMP_MSG_DATA_VALUE_CMD_KEY_SPECIAL;
  fieldValue.fieldNameId = 0;
  fieldValue.fieldValueId = COMP_MSG_WIFI_VALUE_ID_textCallback;
  fieldValue.fieldValueCallback = NULL;
  result = compMsgDataValue->addFieldValueInfo(self, &fieldValue);
  checkErrOK(result);

  compMsgUtil = self->compMsgUtil;
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_AP_BssidSize,       &getWifiAPBssidSize);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_AP_BssidStrSize,    &getWifiAPBssidStrSize);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_AP_SsidSize,        &getWifiAPSsidSize);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_AP_Ssid_lenSize,    &getWifiAPSsid_lenSize);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_AP_ChannelSize,     &getWifiAPChannelSize);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_AP_RssiSize,        &getWifiAPRssiSize);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_AP_AuthmodeSize,    &getWifiAPAuthmodeSize);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_AP_Is_hiddenSize,   &getWifiAPIs_hiddenSize);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_AP_Freq_offsetSize, &getWifiAPFreq_offsetSize);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_AP_Freqcal_valSize, &getWifiAPFreqcal_valSize);
  checkErrOK(result);

  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_AP_Bssids,         &getWifiAPBssids);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_AP_BssidStrs,      &getWifiAPBssidStrs);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_AP_Ssids,          &getWifiAPSsids);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_AP_Ssid_lens,      &getWifiAPSsid_lens);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_AP_Channels,       &getWifiAPChannels);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_AP_Rssis,          &getWifiAPRssis);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_AP_Authmodes,      &getWifiAPAuthmodes);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_AP_Is_hiddens,     &getWifiAPIs_hiddens);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_AP_Freq_offsets,   &getWifiAPFreq_offsets);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_AP_Freqcal_vals,   &getWifiAPFreqcal_vals);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_ClientIPAddr,      &getClientIPAddr);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_ClientPort,        &getClientPort);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_ClientStatus,      &getClientStatus);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_SSDPIPAddr,        &getSSDPIPAddr);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_SSDPPort,          &getSSDPPort);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_SSDPStatus,        &getSSDPStatus);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_ProvisioningSsid,  &getProvisioningSsid);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_ProvisioningPort,  &getProvisioningPort);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_ClientPort,        &getClientPort);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_ClientSsid,        &getClientSsid);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_ClientPasswd,      &getClientPasswd);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_ClientSsidSize,    &getClientSsidSize);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_ClientPasswdSize,  &getClientPasswdSize);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_ClientIPAddrSize,  &getClientIPAddrSize);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_ClientPortSize,    &getClientPortSize);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_ClientStatusSize,  &getClientStatusSize);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_ClientSequenceNumSize, &getClientSequenceNumSize);
  checkErrOK(result);
  result = compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_WIFI_ClientSequenceNum, &getClientSequenceNum);
  checkErrOK(result);

  result = customInit(self);
  checkErrOK(result);
//ets_printf("wifi 02 heap: %d\n", system_get_free_heap_size());

  compMsgWifiData = self->compMsgWifiData;
  compMsgWifiData->getBssScanInfo = &getBssScanInfo;
  
  compMsgWifiData->keyValueStr2KeyValueId = &keyValueStr2KeyValueId;
  compMsgWifiData->callbackStr2CallbackId = &callbackStr2CallbackId;
  compMsgWifiData->getWifiValue = &getWifiValue;
  compMsgWifiData->getWifiConfig = &getWifiConfig;
  compMsgWifiData->getWifiRemotePort = &getWifiRemotePort;
  compMsgWifiData->bssStr2BssInfoId = &bssStr2BssInfoId;
  compMsgWifiData->connectToAP = &connectToAP;
  compMsgWifiData->startStationCb = &startStationCb;
  compMsgWifiData->getClientSequenceNum = &getClientSequenceNum;
  compMsgWifiData->webSocketSendConnectError = &webSocketSendConnectError;
  compMsgWifiData->netSocketSendConnectError = &netSocketSendConnectError;

  compMsgWifiData->getProdTestSsid = &getProdTestSsid;
  compMsgWifiData->getProdTestPasswd = &getProdTestPasswd;
  compMsgWifiData->getProdTestSecurityType = &getProdTestSecurityType;
  compMsgWifiData->getProdTestTargetProtocol = &getProdTestTargetProtocol;
  compMsgWifiData->getProdTestIpAddress = &getProdTestIpAddress;
  compMsgWifiData->getProdTestSubnet = &getProdTestSubnet;
  compMsgWifiData->getProdTestGateway = &getProdTestGateway;
  compMsgWifiData->getProdTestDns = &getProdTestDns;
  compMsgWifiData->getProdTestPingAddress = &getProdTestPingAddress;
  compMsgWifiData->getProdTestStatus = &getProdTestStatus;

  bssScanInfos.compMsgDispatcher = self;
  self->dispatcherCommon->bssScanInfos = &bssScanInfos;
//  result = self->compMsgMsgDesc->readWifiValues(self, COMP_MSG_WIFI_VALUES_FILE_NAME);
//  checkErrOK(result);
  result = compMsgWifiData->getWifiConfig(self);
  checkErrOK(result);
ets_printf("wifi 03 heap: %d\n", system_get_free_heap_size());
  return COMP_MSG_ERR_OK;
}

// ================================= newCompMsgWifiData ====================================

compMsgWifiData_t *newCompMsgWifiData() {
  compMsgWifiData_t *compMsgWifiData;

  compMsgWifiData = os_zalloc(sizeof(compMsgWifiData_t));
  if (compMsgWifiData == NULL) {
    return NULL;
  }
  compMsgWifiData->compMsgWifiDataInit = &compMsgWifiDataInit;
  return compMsgWifiData;
}
