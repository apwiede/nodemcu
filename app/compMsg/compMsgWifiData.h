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
 * File:   compMsgModuleData.h
 * Author: Arnulf P. Wiedemann
 *
 * Created on October 9th, 2016
 */

/* comp message data value handling */

#ifndef COMP_MSG_WIFI_DATA_H
#define	COMP_MSG_WIFI_DATA_H

// status values: 0/1/2/3/4/5
#define STATION_STATUS_IDLE           STATION_IDLE
#define STATION_STATUS_CONNECTING     STATION_CONNECTING
#define STATION_STATUS_WRONG_PASSWORD STATION_WRONG_PASSWORD
#define STATION_STATUS_NO_AP_FOUND    STATION_NO_AP_FOUND
#define STATION_STATUS_CONNECT_FAIL   STATION_CONNECT_FAIL
#define STATION_STATUS_GOT_IP         STATION_GOT_IP

// modes: 0/1/2/3
#define OPMODE_NULL       NULL_MODE
#define OPMODE_STATION    STATION_MODE
#define OPMODE_SOFTAP     SOFTAP_MODE
#define OPMODE_STATIONAP  STATIONAP_MODE

#define WIFI_INFO_WIFI                 (1 << 1)

#define WIFI_INFO_WIFI_OPMODE                 20
#define WIFI_INFO_BINARY_CALL_BACK            21
#define WIFI_INFO_TEXT_CALL_BACK              22
#define WIFI_INFO_NET_RECEIVED_CALL_BACK      23
#define WIFI_INFO_NET_TO_SEND_CALL_BACK       24
#define WIFI_INFO_PROVISIONING_SSID           25
#define WIFI_INFO_PROVISIONING_PORT           26
#define WIFI_INFO_PROVISIONING_IP_ADDR        27
#define WIFI_INFO_CLIENT_SSID                 28
#define WIFI_INFO_CLIENT_PASSWD               29
#define WIFI_INFO_CLIENT_IP_ADDR              30
#define WIFI_INFO_CLIENT_PORT                 31
#define WIFI_INFO_CLIENT_STATUS               32
#define WIFI_INFO_CLIENT_SEQUENCE_NUM         33
#define WIFI_INFO_CLOUD_SECURE_CONNECT        34
#define WIFI_INFO_CLOUD_PORT                  35
#define WIFI_INFO_CLOUD_HOST_1                36
#define WIFI_INFO_CLOUD_URL_1_PART_1          37
#define WIFI_INFO_CLOUD_URL_TENANT_ID_1       38
#define WIFI_INFO_CLOUD_URL_1_PART_2          39
#define WIFI_INFO_CLOUD_NODE_TOKEN_1          40
#define WIFI_INFO_CLOUD_HOST_2                41
#define WIFI_INFO_CLOUD_URL_2_PART_1          42
#define WIFI_INFO_CLOUD_URL_TENANT_ID_2       43
#define WIFI_INFO_CLOUD_URL_2_PART_2          44
#define WIFI_INFO_CLOUD_NODE_TOKEN_2          45
#define WIFI_INFO_SSDP_IP_ADDR                46
#define WIFI_INFO_SSDP_PORT                   47
#define WIFI_INFO_SSDP_STATUS                 48
#define WIFI_INFO_NET_SSDP_RECEIVED_CALL_BACK 49
#define WIFI_INFO_NET_SSDP_TO_SEND_CALL_BACK  50

#define BSS_INFO_BSSID        1
#define BSS_INFO_BSSID_STR    2
#define BSS_INFO_SSID         3
#define BSS_INFO_SSID_LEN     4
#define BSS_INFO_CHANNEL      5
#define BSS_INFO_RSSI         6
#define BSS_INFO_AUTH_MODE    7
#define BSS_INFO_IS_HIDDEN    8
#define BSS_INFO_FREQ_OFFSET  9
#define BSS_INFO_FREQ_CAL_VAL 10

#define KEY_VALUE_KEY_SSID                   1
#define KEY_VALUE_KEY_PASSWORD               2
#define KEY_VALUE_KEY_BSSIDSET               3
#define KEY_VALUE_KEY_BSSID                  4
#define KEY_VALUE_KEY_STATUS                 5
#define KEY_VALUE_KEY_MODE                   6
#define KEY_VALUE_KEY_AUTH_MODE              7
#define KEY_VALUE_KEY_CHANNEL                8
#define KEY_VALUE_KEY_FREQ_OFFSET            9
#define KEY_VALUE_KEY_FREQ_CAL_VAL           10
#define KEY_VALUE_KEY_RSSI                   11
#define KEY_VALUE_KEY_IS_HIDDEN              12
#define KEY_VALUE_KEY_CLIENT_SSID            13
#define KEY_VALUE_KEY_CLIENT_PASSWD          14
#define KEY_VALUE_KEY_CLIENT_IP_ADDR         15
#define KEY_VALUE_KEY_CLIENT_PORT            16
#define KEY_VALUE_KEY_CLIENT_STATUS          17
#define KEY_VALUE_KEY_SEQ_NUM                18
#define KEY_VALUE_KEY_MAC_ADDR               19
#define KEY_VALUE_KEY_MACHINE_STATE          20
#define KEY_VALUE_KEY_FIRMWARE_MAIN_BOARD    21
#define KEY_VALUE_KEY_FIRMWARE_DISPLAY_BOARD 22
#define KEY_VALUE_KEY_FIRMWARE_WIFI_MODULE   23
#define KEY_VALUE_KEY_LAST_ERROR             24
#define KEY_VALUE_KEY_CASING_USE_LIST        25
#define KEY_VALUE_KEY_CASING_STATISTIC_LIST  26
#define KEY_VALUE_KEY_DAT_AND_TIME           27

#define WIFI_USE_SAVED_SEQUENCE_NUM          0x01

enum webSocket_opcode {
  OPCODE_TEXT = 1,
  OPCODE_BINARY = 2,
  OPCODE_CLOSE = 8,
  OPCODE_PING = 9,
  OPCODE_PONG = 10,
};

typedef struct compMsgDispatcher compMsgDispatcher_t;

typedef struct bssScanInfo {
  uint8_t bssid[6];
  uint8_t bssidStr[18];
  uint8_t ssid[32];
  uint8_t ssid_len;
  uint8_t channel;
  int8_t  rssi;
  uint8_t authmode;
  uint8_t is_hidden;
  int16_t freq_offset;
  int16_t freqcal_val;
} bssScanInfo_t;

typedef struct bssScanSizes {
  uint16_t bssidSize;
  uint16_t bssidStrSize;
  uint16_t ssidSize;
  uint16_t channelSize;
  uint16_t rssiSize;
  uint16_t authmodeSize;
  uint16_t is_hiddenSize;
  uint16_t freq_offsetSize;
  uint16_t freqcal_valSize;
} bssScanSizes_t;

typedef struct bssScanTypes {
  uint8_t bssidType;
  uint8_t bssidStrType;
  uint8_t ssidType;
  uint8_t channelType;
  uint8_t rssiType;
  uint8_t authmodeType;
  uint8_t is_hiddenType;
  uint8_t freq_offsetType;
  uint8_t freqcal_valType;
} bssScanTypes_t;

typedef struct bssScanInfos {
  bssScanInfo_t *infos;
  uint8_t numScanInfos;
  uint8_t maxScanInfos;
  bool scanInfoComplete;
  compMsgDispatcher_t *compMsgDispatcher;
} bssScanInfos_t;

typedef struct stationConfig {
  uint8_t ssid[32];
  uint8_t password[64];
  uint8_t bssidSet;
  uint8_t bssid[6];
  uint8_t bssidStr[18];
  uint8_t status;
  uint8_t mode;
  uint8_t authmode;
  uint8_t channel;
  int16_t freq_offset;
  int16_t freqcal_val;
} stationConfig_t;

typedef struct keyValueInfo {
  uint16_t key_ssid;
  uint16_t key_ssid_len;
  uint16_t key_bssid;
  uint16_t key_channel;
  uint16_t key_rssi;
  uint16_t key_authmode;
  uint16_t key_freq_offset;
  uint16_t key_freqcal_val;
  uint16_t key_is_hidden;
  uint16_t key_clientSsid;
  uint16_t key_clientPasswd;
  uint16_t key_clientIPAddr;
  uint16_t key_clientPort;
  uint16_t key_clientStatus;
  uint16_t key_seqNum;
  uint16_t key_MACAddr;
  uint16_t key_machineState;
  uint16_t key_firmwareMainBoard;
  uint16_t key_firmwareDisplayBoard;
  uint16_t key_firmwareWifiModule;
  uint16_t key_lastError;
  uint16_t key_casingUseList;
  uint16_t key_casingStatisticList;
  uint16_t key_dataAndTime;

  uint8_t key_type_ssid;
  uint8_t key_type_ssid_len;
  uint8_t key_type_bssid;
  uint8_t key_type_channel;
  uint8_t key_type_rssi;
  uint8_t key_type_authmode;
  uint8_t key_type_freq_offset;
  uint8_t key_type_freqcal_val;
  uint8_t key_type_is_hidden;
  uint8_t key_type_clientSsid;
  uint8_t key_type_clientPasswd;
  uint8_t key_type_clientIPAddr;
  uint8_t key_type_clientPort;
  uint8_t key_type_clientStatus;
  uint8_t key_type_seqNum;
  uint8_t key_type_MACAddr;
  uint8_t key_type_machineState;
  uint8_t key_type_firmwareMainBoard;
  uint8_t key_type_firmwareDisplayBoard;
  uint8_t key_type_firmwareWifiModule;
  uint8_t key_type_lastError;
  uint8_t key_type_casingUseList;
  uint8_t key_type_casingStatisticList;
  uint8_t key_type_dataAndTime;
} keyValueInfo_t;

typedef uint8_t (*bssStr2BssInfoId_t)(uint8_t *fieldName, uint8_t *fieldId);
typedef uint8_t (* keyValueStr2KeyValueId_t)(uint8_t *fieldName, uint16_t *fieldId);
typedef uint8_t (* getBssScanInfo_t)(compMsgDispatcher_t *self);
typedef uint8_t (* connectToAP_t)(compMsgDispatcher_t *self);
typedef uint8_t (* startStationCb_t)(compMsgDispatcher_t *self);
typedef uint8_t (* getWifiValue_t)(compMsgDispatcher_t *self, uint16_t which, uint8_t valueTypeId, int *numericValue, uint8_t **stringValue);
typedef uint8_t (* getWifiConfig_t)(compMsgDispatcher_t *self);
typedef uint8_t (* setWifiValue_t)(compMsgDispatcher_t *self, uint8_t *fieldName, int numericValue, uint8_t *stringValue);
typedef uint8_t (* getWifiRemotePort_t)(compMsgDispatcher_t *self);
typedef uint8_t (* webSocketSendConnectError_t)(compMsgDispatcher_t *self, uint8_t status);
typedef uint8_t (* netSocketSendConnectError_t)(compMsgDispatcher_t *self, uint8_t status);
typedef uint8_t (* getClientSequenceNum_t)(compMsgDispatcher_t *self, int* numericValue, uint8_t **stringValue);

typedef struct compMsgWifiData {
  keyValueInfo_t keyValueInfo;

  uint8_t wifiOpMode;
  uint8_t provisioningSsid[33];
  uint16_t provisioningPort;
  uint8_t provisioningIPAddr[16];
  uint8_t clientSsid[33];
  uint8_t clientPasswd[65];
  uint32_t clientIPAddr;
  uint16_t clientPort;
  uint8_t clientStatus;
  uint32_t ssdpIPAddr;
  uint16_t ssdpPort;
  uint8_t ssdpStatus;
  uint32_t clientSequenceNum;
#ifdef CLIENT_SSL_ENABLE
  uint8_t cloudSecureConnect;
#endif
  uint16_t cloudPort;
  uint8_t *cloudHost1;
  uint8_t *cloudHost2;
  uint8_t *cloudUrl1Part1;
  uint8_t *cloudUrl1Part2;
  uint8_t *cloudUrlTenantId1;
  uint8_t *cloudUrl2Part1;
  uint8_t *cloudUrl2Part2;
  uint8_t *cloudUrlTenantId2;
  uint8_t *cloudNodeToken1;
  uint8_t *cloudNodeToken2;
  bssScanSizes_t bssScanSizes;
  bssScanTypes_t bssScanTypes;
  uint16_t flags;

  bssScanInfos_t bssScanInfos;
  keyValueStr2KeyValueId_t keyValueStr2KeyValueId;
  getBssScanInfo_t getBssScanInfo;
  getWifiValue_t getWifiValue;
  getWifiConfig_t getWifiConfig;
  setWifiValue_t setWifiValue;
  getWifiRemotePort_t getWifiRemotePort;
  bssStr2BssInfoId_t bssStr2BssInfoId;
  connectToAP_t connectToAP;
  startStationCb_t startStationCb;
  getClientSequenceNum_t getClientSequenceNum;
  webSocketSendConnectError_t webSocketSendConnectError;
  netSocketSendConnectError_t netSocketSendConnectError;

  webSocketBinaryReceived_t webSocketBinaryReceived;
  webSocketTextReceived_t webSocketTextReceived;
  netSocketToSend_t netSocketToSend;
  netSocketReceived_t netSocketReceived;
  netSocketSSDPToSend_t netSocketSSDPToSend;
  netSocketSSDPReceived_t netSocketSSDPReceived;
} compMsgWifiData_t;

compMsgWifiData_t *newCompMsgWifiData();
uint8_t compMsgWifiInit(compMsgDispatcher_t *self);

#endif	/* COMP_MSG_WIFI_DATA_H */
