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

#include "platform.h"

#include "dataView.h"
#include "compMsgErrorCodes.h"
#include "compMsgTypesAndNames.h"
#include "compMsgDataView.h"
#include "compMsgTimer.h"
#include "compMsgAction.h"
#include "compMsgSendReceive.h"
#include "compMsgIdentify.h"
#include "compMsgBuildMsg.h"
#include "compMsgModuleData.h"
#include "compMsgSocket.h"
#include "compMsgWifiData.h"
#include "compMsgMsgDesc.h"
#include "compMsgData.h"
#include "compMsgHttp.h"

//#define CLOUD_1
#define CLOUD_2

// answer message types
#define COMP_MSG_ACK_MSG 0x01
#define COMP_MSG_NAK_MSG 0x02

// input source types
#define COMP_DISP_INPUT_UART       0x01
#define COMP_DISP_INPUT_NET_SOCKET 0x02
#define COMP_DISP_INPUT_WEB_SOCKET 0x04

#define COMP_DISP_CALLBACK_TYPE_WIFI               0x01
#define COMP_DISP_CALLBACK_TYPE_WIFI_AP_LIST_SIZE  0x02
#define COMP_DISP_CALLBACK_TYPE_WIFI_AP_LIST_VALUE 0x04
#define COMP_DISP_CALLBACK_TYPE_MODULE             0x08

// running mode flags
#define COMP_DISP_RUNNING_MODE_AP       0x01
#define COMP_DISP_RUNNING_MODE_CLIENT   0x02
#define COMP_DISP_RUNNING_MODE_CLOUD    0x04
#define COMP_DISP_RUNNING_MODE_APP      0x08
#define COMP_DISP_RUNNING_MODE_WEBSOCKT 0x10

#define COMP_DISP_MAX_REQUESTS     5

// dst + src + totalLgth + (optional) GUID + cmdKey/shCmdKey
// uint16_t + uint16_t + uint16_t + (optional) uint8_t*(16) + uint16_t/uint8_t
#define DISP_MAX_HEADER_LGTH (sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint16_t) + (16*sizeof(uint8_t)) + sizeof(uint16_t))

typedef struct compMsgData compMsgData_t;

typedef struct msgHeader2MsgPtr {
  compMsgData_t *compMsgData;
} msgHeader2MsgPtr_t;

typedef struct msgRequestInfos {
  uint8_t requestTypes[COMP_DISP_MAX_REQUESTS];
  void *requestHandles[COMP_DISP_MAX_REQUESTS];
  compMsgData_t *requestData[COMP_DISP_MAX_REQUESTS];
  int currRequestIdx;
  int lastRequestIdx;
} msgRequestInfos_t;

typedef struct fieldValueCallbackInfos {
  uint8_t *callbackName;
  fieldValueCallback_t callback;
  uint8_t callbackType;
} fieldValueCallbackInfos_t;

typedef struct compMsgDispatcher compMsgDispatcher_t;

// WebSocket stuff
typedef uint8_t (* webSocketRunAPMode_t)(compMsgDispatcher_t *self);
typedef uint8_t (* webSocketRunClientMode_t)(compMsgDispatcher_t *self, uint8_t mode);
typedef uint8_t (* webSocketSendData_t)(socketUserData_t *sud, const char *payload, int size, int opcode);

// NetSocket stuff
typedef uint8_t (* netSocketStartCloudSocket_t)(compMsgDispatcher_t *self);
typedef uint8_t (* netSocketRunClientMode_t)(compMsgDispatcher_t *self);
typedef uint8_t (* netSocketSendData_t)(socketUserData_t *sud, const char *payload, int size);

// Dispatcher stuff
typedef uint8_t (* startRequest_t)(compMsgDispatcher_t *self);
typedef uint8_t (* startNextRequest_t)(compMsgDispatcher_t *self);
typedef uint8_t (* addUartRequestData_t)(compMsgDispatcher_t *self, uint8_t *data, size_t lgth);
typedef uint8_t (* addRequest_t)(compMsgDispatcher_t *self, uint8_t requestType, void *requestHandle, compMsgData_t *requestData);
typedef uint8_t (* deleteRequest_t)(compMsgDispatcher_t *self, uint8_t requestType, void *requestHandle);
typedef uint8_t (* dumpMsgParts_t)(compMsgDispatcher_t *self, msgParts_t *msgParts);
typedef uint8_t (* dumpMsgHeaderInfos_t)(compMsgDispatcher_t *self, msgHeaderInfos_t *hdrInfos);
typedef uint8_t (* dumpHeaderPart_t)(compMsgDispatcher_t *self, headerPart_t *hdr);

typedef uint8_t (* createDispatcher_t)(compMsgDispatcher_t *self, uint8_t **handle);
typedef uint8_t (* initDispatcher_t)(compMsgDispatcher_t *self, const uint8_t *type, size_t typelen);
typedef uint8_t (* createMsgFromHeaderPart_t)(compMsgDispatcher_t *self, headerPart_t *hdr, uint8_t **handle);
typedef uint8_t (* encryptMsg_t)(const uint8_t *msg, size_t mlen, const uint8_t *key, size_t klen, const uint8_t *iv, size_t ivlen, uint8_t **buf, int *lgth);
typedef uint8_t (* decryptMsg_t)(const uint8_t *msg, size_t mlen, const uint8_t *key, size_t klen, const uint8_t *iv, size_t ivlen, uint8_t **buf, int *lgth);
typedef uint8_t (* toBase64_t)(const uint8_t *msg, size_t *len, uint8_t **encoded);
typedef uint8_t (* fromBase64_t)(const uint8_t *encodedMsg, size_t *len, uint8_t **decodedMsg);
typedef uint8_t (* resetBuildMsgInfos_t)(compMsgDispatcher_t *self);
typedef uint8_t (* getFieldType_t)(compMsgDispatcher_t *self, compMsgData_t *compMsgData, uint8_t fieldNameId, uint8_t *fieldTypeId);
typedef uint8_t (* setFieldValueCallback_t)(compMsgDispatcher_t *compMsgDispatcher, uint8_t *callbackName, fieldValueCallback_t callback, uint8_t callbackType);
typedef uint8_t (* addFieldValueCallbackName_t)(compMsgDispatcher_t *compMsgDispatcher, uint8_t *callbackName, fieldValueCallback_t callback, uint8_t callbackType);
typedef uint8_t (* getFieldValueCallback_t)(compMsgDispatcher_t *compMsgDispatcher, uint8_t *callbackName, fieldValueCallback_t *callback, uint8_t callbackType);
typedef uint8_t (* getFieldValueCallbackName_t)(compMsgDispatcher_t *compMsgDispatcher, fieldValueCallback_t callback, uint8_t **callbackName, uint8_t callbackType);

// MsgData stuff
typedef uint8_t (* getNewCompMsgDataPtr_t)(compMsgDispatcher_t *self);
typedef uint8_t (* resetMsgInfo_t)(compMsgDispatcher_t *self, msgParts_t *parts);

typedef struct compMsgDispatcher {
  uint8_t id;
  char handle[20];
  int numericValue;
  uint8_t *stringValue;
  uint8_t actionMode;
  uint8_t operatingMode;
  uint8_t webSocketError;
  bssScanInfos_t *bssScanInfos;
  msgKeyValueDescPart_t *msgKeyValueDescParts;
  size_t numMsgKeyValueDescParts;
  size_t maxMsgKeyValueDescParts;
  uint8_t *cloudMsgData;
  size_t cloudMsgDataLgth;
  uint8_t *cloudPayload;
  size_t cloudPayloadLgth;
  bool stopAccessPoint;
  // running mode flags
  uint16_t runningModeFlags;

  // station mode
  uint32_t station_ip;


  msgHeaderInfos_t msgHeaderInfos;

  // compMsgTypesAndNames info
  compMsgTypesAndNames_t *compMsgTypesAndNames;

  // compMsgMsgDesc info
  compMsgMsgDesc_t *compMsgMsgDesc;

  // compMsgTimer info
  compMsgTimer_t *compMsgTimer;

  // compMsgHttp info
  compMsgHttp_t *compMsgHttp;

  // compMsgAction info
  compMsgAction_t *compMsgAction;

  // compMsgWifiData info
  compMsgWifiData_t *compMsgWifiData;

  // compMsgModuleData info
  compMsgModuleData_t *compMsgModuleData;

  // compMsgIdentify info
  compMsgIdentify_t *compMsgIdentify;

  // compMsgBuildMsg info
  compMsgBuildMsg_t *compMsgBuildMsg;

  // compMsgBuildMsg info
  compMsgSendReceive_t *compMsgSendReceive;

  // request infos
  msgRequestInfos_t msgRequestInfos;

  // this is for mapping a msg handle from the header to a compMsgPtr
  uint8_t numMsgHeaders;
  uint8_t maxMsgHeaders;
  msgHeader2MsgPtr_t *msgHeader2MsgPtrs;

  uint8_t numFieldValueCallbackInfos;
  uint8_t maxFieldValueCallbackInfos;
  fieldValueCallbackInfos_t *fieldValueCallbackInfos;

  uint8_t numFieldsToSave;
  uint8_t maxFieldsToSave;
  fieldsToSave_t *fieldsToSave;

  compMsgData_t *compMsgData;
  uint8_t *msgHandle;

  // function pointers

  // Dispatcher
  getFieldType_t getFieldType;
  resetMsgInfo_t resetMsgInfo;
  createDispatcher_t createDispatcher;
  initDispatcher_t initDispatcher;
  createMsgFromHeaderPart_t createMsgFromHeaderPart;
  getNewCompMsgDataPtr_t getNewCompMsgDataPtr;
  startRequest_t startRequest;
  startNextRequest_t startNextRequest;
  addUartRequestData_t addUartRequestData;
  addRequest_t addRequest;
  deleteRequest_t deleteRequest;
  encryptMsg_t encryptMsg;
  decryptMsg_t decryptMsg;
  toBase64_t toBase64;
  fromBase64_t fromBase64;
  resetBuildMsgInfos_t resetBuildMsgInfos;
  setFieldValueCallback_t setFieldValueCallback;
  addFieldValueCallbackName_t addFieldValueCallbackName;
  getFieldValueCallback_t getFieldValueCallback;
  getFieldValueCallbackName_t getFieldValueCallbackName;

  webSocketRunAPMode_t webSocketRunAPMode;

  webSocketRunClientMode_t webSocketRunClientMode;
  webSocketSendData_t webSocketSendData;

  netSocketStartCloudSocket_t netSocketStartCloudSocket;
  netSocketRunClientMode_t netSocketRunClientMode;
  netSocketSendData_t netSocketSendData;

  dumpMsgParts_t dumpMsgParts;
} compMsgDispatcher_t;

compMsgDispatcher_t *newCompMsgDispatcher();
uint8_t compMsgDispatcherGetPtrFromHandle(const char *handle, compMsgDispatcher_t **compMsgDispatcher);
void freeCompMsgDispatcher(compMsgDispatcher_t *compMsgDispatcher);
uint8_t compMsgIdentifyInit(compMsgDispatcher_t *self);
uint8_t compMsgBuildMsgInit(compMsgDispatcher_t *self);
uint8_t compMsgSendReceiveInit(compMsgDispatcher_t *self);
uint8_t compMsgActionInit(compMsgDispatcher_t *self);
uint8_t compMsgModuleDataInit(compMsgDispatcher_t *self);
uint8_t compMsgWifiInit(compMsgDispatcher_t *self);
uint8_t compMsgWebSocketInit(compMsgDispatcher_t *self);
uint8_t compMsgNetSocketInit(compMsgDispatcher_t *self);
uint8_t compMsgHttpInit(compMsgDispatcher_t *self);
uint8_t compMsgTimerInit(compMsgDispatcher_t *self);

#endif	/* COMP_MSG_DISPATCHER_H */
