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

// the defines have to be bigger then the max of COMP_MSG_WIFI_ defines in compMsgWifiData.h and compMsgWifiDataCustom.h!!
#define COMP_MSG_MODULE_MACAddr             80
#define COMP_MSG_MODULE_IPAddr              81
#define COMP_MSG_MODULE_FirmwareVersion     82
#define COMP_MSG_MODULE_RSSI                83
#define COMP_MSG_MODULE_RSSIMax             84
#define COMP_MSG_MODULE_ConnectionState     85
#define COMP_MSG_MODULE_ConnectedUsers      86
#define COMP_MSG_MODULE_IPProtocol          87
#define COMP_MSG_MODULE_ErrorMain           88
#define COMP_MSG_MODULE_ErrorSub            89
#define COMP_MSG_MODULE_DateAndTime         90
#define COMP_MSG_MODULE_SSIDs               91

#define COMP_MSG_MODULE_OperatingMode       92
#define COMP_MSG_MODULE_OtaHost             93
#define COMP_MSG_MODULE_OtaRomPath          94
#define COMP_MSG_MODULE_OtaFsPath           95
#define COMP_MSG_MODULE_OtaPort             96
#define COMP_MSG_MODULE_CryptKey            97

#define COMP_MSG_MODULE_Reserve1            98
#define COMP_MSG_MODULE_Reserve2            99
#define COMP_MSG_MODULE_Reserve3            100
#define COMP_MSG_MODULE_Reserve4            101
#define COMP_MSG_MODULE_Reserve5            102
#define COMP_MSG_MODULE_Reserve6            103
#define COMP_MSG_MODULE_Reserve7            104
#define COMP_MSG_MODULE_Reserve8            105

#define MODULE_OPERATING_MODE_START              0x20
#define MODULE_OPERATING_MODE_LIGHT_SLEEP        0x21
#define MODULE_OPERATING_MODE_PREPARE_DISABLE    0x22
#define MODULE_OPERATING_MODE_PROVISIONING       0x23
#define MODULE_OPERATING_MODE_MODULE_TEST        0x30

typedef struct compMsgDispatcher compMsgDispatcher_t;

typedef struct compMsgModuleDataName2Value {
  uint8_t *name;
  uint8_t *value;
} compMsgModuleDataName2Value_t;

typedef uint8_t (* callbackStr2CallbackId_t)(uint8_t *callbackName, uint16_t *callbackId);
typedef uint8_t (* callbackId2CallbackStr_t)(uint16_t callbackId, uint8_t **callbackName);
typedef uint8_t (* getOtaHost_t)(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue);
typedef uint8_t (* getOtaRomPath_t)(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue);
typedef uint8_t (* getOtaFsPath_t)(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue);
typedef uint8_t (* getOtaPort_t)(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue);
typedef uint8_t (* getMACAddr_t)(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue);
typedef uint8_t (* getCryptKey_t)(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue);
typedef uint8_t (* getCryptIvKey_t)(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue);
typedef uint8_t (* getOperatingMode_t)(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue);
typedef uint8_t (* restoreUserData_t)(compMsgDispatcher_t *self);
typedef uint8_t (* updateModuleValues_t)(compMsgDispatcher_t *self);
typedef uint8_t (* compMsgModuleDataInit_t)(compMsgDispatcher_t *self);

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
  uint8_t hdrReserve[3];
  uint8_t GUID[17];
  uint16_t srcId;
  uint8_t passwdC[17];
  uint8_t operatingMode;
  uint8_t otaHost[64];
  uint8_t otaRomPath[128];
  uint8_t otaFsPath[128];
  uint16_t otaPort;
  uint8_t cryptKey[17];
  uint8_t cryptIvKey[17];
  uint16_t myU16Src;
  uint16_t myU16SaveUserDataCmdKey;
  uint8_t myU8Src;
  uint16_t myU8SaveUserDataCmdKey;

  callbackStr2CallbackId_t callbackStr2CallbackId;
  callbackId2CallbackStr_t callbackId2CallbackStr;
  getOtaHost_t getOtaHost;
  getOtaRomPath_t getOtaRomPath;
  getOtaFsPath_t getOtaFsPath;
  getOtaPort_t getOtaPort;
  getMACAddr_t getMACAddr;
  getCryptKey_t getCryptKey;
  getCryptIvKey_t getCryptIvKey;
  getOperatingMode_t getOperatingMode;
  restoreUserData_t restoreUserData;
  updateModuleValues_t updateModuleValues;
  compMsgModuleDataInit_t compMsgModuleDataInit;
} compMsgModuleData_t;

compMsgModuleData_t *newCompMsgModuleData();

#endif	/* COMP_MSG_MODULE_DATA_H */
