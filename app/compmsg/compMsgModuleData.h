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

#ifndef COMP_MSG_MODULE_DATA_H
#define	COMP_MSG_MODULE_DATA_H

#define MODULE_INFO_MODULE            (1 << 0)

#define MODULE_INFO_MACAddr              1
#define MODULE_INFO_IPAddr               2
#define MODULE_INFO_FirmwareVersion      3
#define MODULE_INFO_SerieNumber          4
#define MODULE_INFO_RSSI                 5
#define MODULE_INFO_RSSIMax              6
#define MODULE_INFO_ConnectionState      7
#define MODULE_INFO_ConnectedUsers       8
#define MODULE_INFO_ProgRunningMode      9
#define MODULE_INFO_CurrentRunningMode   10
#define MODULE_INFO_IPProtocol           11
#define MODULE_INFO_Region               12
#define MODULE_INFO_DeviceSecurity       13
#define MODULE_INFO_ErrorMain            14
#define MODULE_INFO_ErrorSub             15
#define MODULE_INFO_DateAndTime          16
#define MODULE_INFO_SSIDs                17
#define MODULE_INFO_PingState            18
#define MODULE_INFO_Reserve1             19
#define MODULE_INFO_Reserve2             20
#define MODULE_INFO_Reserve3             21
#define MODULE_INFO_Reserve4             22
#define MODULE_INFO_Reserve5             23
#define MODULE_INFO_Reserve6             24
#define MODULE_INFO_Reserve7             25
#define MODULE_INFO_Reserve8             26
#define MODULE_INFO_AP_LIST_CALL_BACK    27
#define MODULE_INFO_GUID                 28
#define MODULE_INFO_srcId                29
#define MODULE_INFO_PASSWDC              30
#define MODULE_INFO_operatingMode        31
#define MODULE_INFO_OTA_HOST             32
#define MODULE_INFO_OTA_ROM_PATH         33
#define MODULE_INFO_OTA_FS_PATH          34
#define MODULE_INFO_OTA_PORT             35
#define MODULE_INFO_CRYPT_KEY            36

#define MODULE_OPERATING_MODE_CLIENT             1
#define MODULE_OPERATING_MODE_AP                 2
#define MODULE_OPERATING_MODE_LIGHT_SLEEP_WAKEUP 3
#define MODULE_OPERATING_MODE_LIGHT_SLEEP        4
#define MODULE_OPERATING_MODE_WPS                5
#define MODULE_OPERATING_MODE_MODULE_TEST        0xD0
#define MODULE_OPERATING_MODE_CLEAR_PASSWDC      0xE0

typedef struct compMsgDispatcher compMsgDispatcher_t;

typedef struct compMsgModuleDataName2Value {
  uint8_t *name;
  uint8_t *value;
} compMsgModuleDataName2Value_t;

typedef uint8_t (* getOtaHost_t)(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue);
typedef uint8_t (* getOtaRomPath_t)(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue);
typedef uint8_t (* getOtaFsPath_t)(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue);
typedef uint8_t (* getOtaPort_t)(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue);
typedef uint8_t (* getMACAddr_t)(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue);
typedef uint8_t (* getCryptKey_t)(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue);
typedef uint8_t (* restoreUserData_t)(compMsgDispatcher_t *self);
typedef uint8_t (* setModuleValue_t)(compMsgDispatcher_t *self, uint8_t *fieldNameStr, int numericValue, uint8_t *stringValue);
typedef uint8_t (* setModuleValues_t)(compMsgDispatcher_t *self);
typedef uint8_t (* updateModuleValues_t)(compMsgDispatcher_t *self);

typedef struct compMsgModuleData {
  uint8_t MACAddr[7];
  uint8_t IPAddr[5];
  uint8_t FirmwareVersion[7];
  uint8_t SerieNumber[5];
  uint8_t RSSI;
  uint8_t RSSIMax;
  uint8_t ConnectionState;
  uint8_t ConnectedUsers;
  uint8_t ProgRunningMode;
  uint8_t CurrentRunningMode;
  uint8_t IPProtocol;
  uint8_t Region;
  uint8_t DeviceSecurity;
  uint8_t ErrorMain;
  uint8_t ErrorSub;
  uint8_t DateAndTime[7];
  uint8_t SSIDs;
  uint8_t PingState;
  uint8_t Reserve1;
  uint8_t Reserve2[3];
  uint8_t Reserve3[4];
  uint8_t Reserve4[5];
  uint8_t Reserve5[6];
  uint8_t Reserve6[7];
  uint8_t Reserve7[8];
  uint8_t Reserve8[9];
  uint8_t GUID[17];
  uint16_t srcId;
  uint8_t passwdC[17];
  uint8_t operatingMode;
  uint8_t otaHost[64];
  uint8_t otaRomPath[128];
  uint8_t otaFsPath[128];
  uint16_t otaPort;
  uint8_t cryptKey[17];

  getOtaHost_t getOtaHost;
  getOtaRomPath_t getOtaRomPath;
  getOtaFsPath_t getOtaFsPath;
  getOtaPort_t getOtaPort;
  getMACAddr_t getMACAddr;
  getCryptKey_t getCryptKey;
  restoreUserData_t restoreUserData;
  setModuleValue_t setModuleValue;
  setModuleValues_t setModuleValues;
  updateModuleValues_t updateModuleValues;
} compMsgModuleData_t;

compMsgModuleData_t *newCompMsgModuleData();
uint8_t compMsgModuleDataInit(compMsgDispatcher_t *self);

#endif	/* COMP_MSG_MODULE_DATA_H */
