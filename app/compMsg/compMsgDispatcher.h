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
 * File:   compMsgDispatcher.h
 * Author: Arnulf P. Wiedemann
 *
 * Created on October 1st, 2016
 */

/* composite message data dispatching */

#ifndef COMP_MSG_DISPATCHER
#define	COMP_MSG_DISPATCHER

typedef struct compMsgDispatcher compMsgDispatcher_t;

#include "osapi.h"
#include "c_types.h"
#include "mem.h"
#include "c_string.h"
#include "c_stdlib.h"
#include "c_stdio.h"
#include "platform.h"
#include "../rboot/rboot-ota.h"

#include "dataView.h"
#include "compMsgErrorCodes.h"
#include "compMsgTypesAndNames.h"
#include "compMsgUtil.h"
#include "compMsgDataView.h"
#include "compMsgTimer.h"
#include "compMsgDataValue.h"
#include "compMsgMsgDesc.h"
#include "compMsgAction.h"
#include "compMsgRequest.h"
#include "compMsgSendReceive.h"
#include "compMsgIdentify.h"
#include "compMsgBuildMsg.h"
#include "compMsgModuleData.h"
#include "compMsgSocket.h"
#include "compMsgWifiData.h"
#include "compMsgData.h"
#include "compMsgHttp.h"
#include "compMsgDebug.h"
#include "compMsgOta.h"

#define CLOUD_2

#define UART0 0
#define UART1 1

// timer id defines
#define COMP_MSG_UART_TIMER_ID              0
#define COMP_MSG_WEBSOCKET_TIMER_ID         1
#define COMP_MSG_ACCESS_POINT_TIMER_ID      2
#define COMP_MSG_CLIENT_MODE_START_TIMER_ID 3
  
// compMsgMsgDesc file names

#define COMP_MSG_WIFI_VALUES_FILE_NAME        "myConfig.txt"
#define COMP_MSG_HEADS_FILE_NAME              "MsgHeads.txt"
#define COMP_MSG_MODULE_DATA_VALUES_FILE_NAME "CompMsgModuleDataValues.txt"

// answer message types
#define COMP_MSG_ACK_MSG 0x01
#define COMP_MSG_NAK_MSG 0x02

#define COMP_DISP_CALLBACK_TYPE_WIFI               0x01
#define COMP_DISP_CALLBACK_TYPE_WIFI_AP_LIST_SIZE  0x02
#define COMP_DISP_CALLBACK_TYPE_WIFI_AP_LIST_VALUE 0x04
#define COMP_DISP_CALLBACK_TYPE_MODULE             0x08

typedef struct compMsgData compMsgData_t;

typedef struct msgHeader2MsgPtr {
  compMsgData_t *compMsgData;
} msgHeader2MsgPtr_t;

typedef struct compMsgDispatcher compMsgDispatcher_t;

typedef uint8_t (* createDispatcher_t)(compMsgDispatcher_t *self, uint8_t **handle);
typedef uint8_t (* initDispatcher_t)(compMsgDispatcher_t *self, const uint8_t *type, size_t typelen);
typedef uint8_t (* resetBuildMsgInfos_t)(compMsgDispatcher_t *self);
typedef uint8_t (* getFieldType_t)(compMsgDispatcher_t *self, compMsgData_t *compMsgData, uint8_t fieldNameId, uint8_t *fieldTypeId);
typedef uint8_t (* getNewCompMsgDataPtr_t)(compMsgDispatcher_t *self);
typedef uint8_t (* resetMsgInfo_t)(compMsgDispatcher_t *self, msgParts_t *parts);

typedef struct dispatcherCommon {
  uint8_t operatingMode;
  uint8_t webSocketError;
  msgKeyValueDescPart_t *msgKeyValueDescParts;
  size_t numMsgKeyValueDescParts;
  size_t maxMsgKeyValueDescParts;
  uint16_t runningModeFlags;
  bool stopAccessPoint;
  bssScanInfos_t *bssScanInfos;
  msgHeaderInfos_t msgHeaderInfos;
  uint8_t numFieldsToSave;
  uint8_t maxFieldsToSave;
  fieldsToSave_t *fieldsToSave;
  // station mode
  uint32_t station_ip;
} dispatcherCommon_t;

typedef struct compMsgDispatcher {
  uint8_t id;
  char handle[20];
  uint8_t actionMode;
  uint8_t *cloudMsgData;
  size_t cloudMsgDataLgth;
  uint8_t *cloudPayload;
  size_t cloudPayloadLgth;
  uint8_t *msgHandle;
  socketUserData_t *sud; 
  socketUserData_t *ssdpSud; 

  dispatcherCommon_t *dispatcherCommon;

  // this is for mapping a msg handle from the header to a compMsgPtr
  uint8_t numMsgHeaders;
  uint8_t maxMsgHeaders;
  msgHeader2MsgPtr_t *msgHeader2MsgPtrs;

  compMsgData_t *compMsgData;

  // compMsgTypesAndNames
  compMsgTypesAndNames_t *compMsgTypesAndNames;

  // compMsgDataValue
  compMsgDataValue_t *compMsgDataValue;

  // compMsgMsgDesc
  compMsgMsgDesc_t *compMsgMsgDesc;

  // compMsgTimer
  compMsgTimer_t *compMsgTimer;

  // compMsgHttp info
  compMsgHttp_t *compMsgHttp;

  // compMsgAction
  compMsgAction_t *compMsgAction;

  // compMsgWifiData
  compMsgWifiData_t *compMsgWifiData;

  // compMsgModuleData
  compMsgModuleData_t *compMsgModuleData;

  // compMsgIdentify
  compMsgIdentify_t *compMsgIdentify;

  // compMsgBuildMsg
  compMsgBuildMsg_t *compMsgBuildMsg;

  // compMsgSendReceive
  compMsgSendReceive_t *compMsgSendReceive;

  // compMsgSocket
  compMsgSocket_t *compMsgSocket;

  // compMsgDebug
  compMsgDebug_t *compMsgDebug;

  // compMsgUtil
  compMsgUtil_t *compMsgUtil;

  // compMsgRequest
  compMsgOta_t *compMsgOta;

  // compMsgRequest
  compMsgRequest_t *compMsgRequest;

  // Dispatcher function pointers
  resetBuildMsgInfos_t resetBuildMsgInfos;
  getFieldType_t getFieldType;
  resetMsgInfo_t resetMsgInfo;
  createDispatcher_t createDispatcher;
  initDispatcher_t initDispatcher;
  getNewCompMsgDataPtr_t getNewCompMsgDataPtr;
} compMsgDispatcher_t;

extern void dbgPrintf(void *self, uint8_t *dbgChars, uint8_t debugLevel, uint8_t *format, ...);

compMsgDispatcher_t *newCompMsgDispatcher();
uint8_t compMsgDispatcherGetPtrFromHandle(const char *handle, compMsgDispatcher_t **compMsgDispatcher);
void freeCompMsgDispatcher(compMsgDispatcher_t *compMsgDispatcher);

#endif	/* COMP_MSG_DISPATCHER_H */
