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
 * File:   structmsgModuleData.h
 * Author: Arnulf P. Wiedemann
 *
 * Created on October 9th, 2016
 */

/* struct message data value handling */

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

// check against structmsgDispatcher.h defines to avoid duplicates!!
#define STRUCT_DISP_ERR_STATION_SCAN       150
#define STRUCT_DISP_ERR_GET_STATION_CONFIG 149
#define STRUCT_DISP_ERR_CANNOT_DISCONNECT  148
#define STRUCT_DISP_ERR_CANNOT_SET_OPMODE  147
#define STRUCT_DISP_ERR_REGIST_CONNECT_CB  146
#define STRUCT_DISP_ERR_TCP_ACCEPT         145
#define STRUCT_DISP_ERR_REGIST_TIME        144
#define STRUCT_DISP_ERR_BAD_MODULE_VALUE_WHICH 143
#define STRUCT_DISP_ERR_BAD_ROW            142

#define MODULE_INFO_MODULE            (1 << 0)
#define MODULE_INFO_WIFI              (1 << 1)

#define MODULE_INFO_MACAddr              1
#define MODULE_INFO_IPAddr               2
#define MODULE_INFO_FirmwareVersion      3
#define MODULE_INFO_SerieNumber          4
#define MODULE_INFO_RSSI                 5
#define MODULE_INFO_ModuleConnection     6
#define MODULE_INFO_DeviceMode           7
#define MODULE_INFO_DeviceSecurity       8
#define MODULE_INFO_ErrorMain            9
#define MODULE_INFO_ErrorSub             10
#define MODULE_INFO_DateAndTime          11
#define MODULE_INFO_SSIDs                12
#define MODULE_INFO_Reserve1             13
#define MODULE_INFO_Reserve2             14
#define MODULE_INFO_Reserve3             15
#define MODULE_INFO_AP_LIST_CALL_BACK    16

#define MODULE_INFO_WIFI_OPMODE          20
#define MODULE_INFO_PROVISIONING_SSID    21
#define MODULE_INFO_PROVISIONING_PORT    22
#define MODULE_INFO_PROVISIONING_IP_ADDR 23
#define MODULE_INFO_BINARY_CALL_BACK     24
#define MODULE_INFO_TEXT_CALL_BACK       25

#define BSS_INFO_BSSID        1
#define BSS_INFO_BSSID_STR    2
#define BSS_INFO_SSID         3
#define BSS_INFO_SSID_LEN     4
#define BSS_INFO_CHANNEL      5
#define BSS_INFO_RSSID        6
#define BSS_INFO_AUTH_MODE    7
#define BSS_INFO_IS_HIDDEN    8
#define BSS_INFO_FREQ_OFFSET  9
#define BSS_INFO_FREQ_CAL_VAL 10

enum websocket_opcode {
  OPCODE_TEXT = 1,
  OPCODE_BINARY = 2,
  OPCODE_CLOSE = 8,
  OPCODE_PING = 9,
  OPCODE_PONG = 10,
};

typedef struct structmsgDispatcher structmsgDispatcher_t;

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

typedef struct bssScanInfos {
  bssScanInfo_t *infos;
  uint8_t numScanInfos;
  uint8_t maxScanInfos;
  bool scanInfoComplete;
  structmsgDispatcher_t *structmsgDispatcher;
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

typedef struct structmsgModuleData {
  uint8_t MACAddr[6];
  uint8_t IPAddr[4];
  uint8_t FirmwareVersion[6];
  uint8_t SerieNumber[4];
  uint8_t RSSI;
  uint8_t ModuleConnection;
  uint8_t DeviceMode;
  uint8_t DeviceSecurity;
  uint8_t ErrorMain;
  uint8_t ErrorSub;
  uint8_t DateAndTime[6];
  uint8_t SSIDs;
  uint8_t Reserve1;
  uint8_t Reserve2[2];
  uint8_t Reserve3[3];
} structmsgModuleData_t;

typedef void (* websocketBinaryReceived_t)(void *arg, void *wud, char *pdata, unsigned short len);
typedef void (* websocketTextReceived_t)(void *arg, void *wud, char *pdata, unsigned short len);
typedef void (* getAPList_t)(structmsgData_t *self);

typedef struct structmsgWifiData {
  uint8_t wifiOpMode;
  uint8_t provisioningSsid[33];
  uint16_t provisioningPort;
  uint8_t provisioningIPAddr[16];
  websocketBinaryReceived_t websocketBinaryReceived;
  websocketTextReceived_t websocketTextReceived;
} structmsgWifiData_t;
