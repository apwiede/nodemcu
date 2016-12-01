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

#include "dataView.h"
#include "compMsgTypesAndNames.h"
#include "compMsgDataView.h"
#include "compMsgModuleData.h"
#include "compMsgWifiData.h"
#include "compMsgMsgDesc.h"
#include "compMsgData.h"

enum compMsgDispatcherErrorCode
{
  COMP_DISP_ERR_OK                    = 0,
  COMP_DISP_ERR_VALUE_NOT_SET         = 255,
  COMP_DISP_ERR_VALUE_OUT_OF_RANGE    = 254,
  COMP_DISP_ERR_BAD_VALUE             = 253,
  COMP_DISP_ERR_BAD_FIELD_TYPE        = 252,
  COMP_DISP_ERR_FIELD_TYPE_NOT_FOUND  = 251,
  COMP_DISP_ERR_VALUE_TOO_BIG         = 250,
  COMP_DISP_ERR_OUT_OF_MEMORY         = 249,
  COMP_DISP_ERR_OUT_OF_RANGE          = 248,
  // be carefull the values up to here
  // must correspond to the values in dataView.h !!!
  // with the names like DATA_VIEW_ERR_*

  COMP_DISP_ERR_FIELD_NOT_FOUND       = 230,
  COMP_DISP_ERR_HANDLE_NOT_FOUND      = 227,
  // be carefull the values up to here
  // must correspond to the values in compMsgDataView.h !!!
  // with the names like COMP_MSG_ERR_*

  // 189 - 180 are used for COMP_MSG_DESC_ERR_* !!

  COMP_DISP_ERR_BAD_RECEIVED_LGTH     = 179,
  COMP_DISP_ERR_BAD_FILE_CONTENTS     = 178,
  COMP_DISP_ERR_HEADER_NOT_FOUND      = 177,
  COMP_DISP_ERR_DUPLICATE_FIELD       = 176,
  COMP_DISP_ERR_BAD_FIELD_NAME        = 175,
  COMP_DISP_ERR_BAD_HANDLE_TYPE       = 174,
  COMP_DISP_ERR_INVALID_BASE64_STRING = 173,
  COMP_DISP_ERR_TOO_FEW_FILE_LINES    = 172,
  COMP_DISP_ERR_ACTION_NAME_NOT_FOUND = 171,
  COMP_DISP_ERR_DUPLICATE_ENTRY       = 170,
  COMP_DISP_ERR_NO_WEBSOCKET_OPENED   = 169,
  COMP_DISP_ERR_TOO_MANY_REQUESTS     = 168,
  COMP_DISP_ERR_REQUEST_NOT_FOUND     = 167,
  COMP_DISP_ERR_UART_REQUEST_NOT_SET  = 166,
  COMP_DISP_ERR_FUNNY_HANDLE_TYPE     = 165,
};

// input source types
#define COMP_DISP_INPUT_UART       0x01
#define COMP_DISP_INPUT_NET_SOCKET 0x02
#define COMP_DISP_INPUT_WEB_SOCKET 0x04

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

typedef struct compMsgDispatcher compMsgDispatcher_t;

// Action stuff
typedef uint8_t (* action_t)(compMsgDispatcher_t *self);
typedef uint8_t (* setActionEntry_t)(compMsgDispatcher_t *self, uint8_t *actionName, uint8_t mode, uint8_t u8CmdKey, uint16_t u16CmdKey);
typedef uint8_t (* runAction_t)(compMsgDispatcher_t *self, uint8_t *answerType);
typedef uint8_t (* getActionMode_t)(compMsgDispatcher_t *self, uint8_t *actionName, uint8_t *actionMode);
typedef uint8_t (* getActionCallback_t)(compMsgDispatcher_t *self, uint8_t *actionName, action_t *callback);
typedef uint8_t (* getActionCallbackName_t)(compMsgDispatcher_t *self, action_t callback, uint8_t **actionName);
typedef uint8_t (* fillMsgValue_t)(compMsgDispatcher_t *self, uint8_t *callbackName, uint8_t answerType, uint8_t fieldTypeId);

// Wifi stuff
typedef uint8_t (*bssStr2BssInfoId_t)(uint8_t *fieldName, uint8_t *fieldId);
typedef uint8_t (* getBssScanInfo_t)(compMsgDispatcher_t *self);
typedef uint8_t (* getScanInfoTableFieldValue_t)(compMsgDispatcher_t *self, uint8_t actionMode);
typedef uint8_t (* getWifiValue_t)(compMsgDispatcher_t *self, uint16_t which, uint8_t valueTypeId, int *numericValue, uint8_t **stringValue);
typedef uint8_t (* getWifiConfig_t)(compMsgDispatcher_t *self);
typedef uint8_t (* setWifiValue_t)(compMsgDispatcher_t *self, uint8_t *fieldName, int numericValue, uint8_t *stringValue);
typedef uint8_t (* getWifiKeyValue_t)(compMsgDispatcher_t *self);
typedef uint8_t (* getWifiKeyValueInfo_t)(compMsgDispatcher_t *self);
typedef uint8_t (* getWifiRemotePort_t)(compMsgDispatcher_t *self);

// ModuleData stuff
typedef uint8_t (* getModuleValue_t)(compMsgDispatcher_t *self, uint16_t which, uint8_t valueTypeId);
typedef uint8_t (* setModuleValue_t)(compMsgDispatcher_t *self, uint8_t *fieldNameStr, int numericValue, uint8_t *stringValue);
typedef uint8_t (* setModuleValues_t)(compMsgDispatcher_t *self);
typedef uint8_t (* updateModuleValues_t)(compMsgDispatcher_t *self);
typedef uint8_t (* getModuleTableFieldValue_t)(compMsgDispatcher_t *self, uint8_t actionMode);
typedef uint8_t (* websocketRunAPMode_t)(compMsgDispatcher_t *self);

// WebSocket stuff
typedef uint8_t (* websocketRunClientMode_t)(compMsgDispatcher_t *self, uint8_t mode);
typedef uint8_t (* websocketSendData_t)(websocketUserData_t *wud, const char *payload, int size, int opcode);

// NetSocket stuff
typedef uint8_t (* netsocketStartCloudSocket_t)(compMsgDispatcher_t *self);
typedef uint8_t (* netsocketRunClientMode_t)(compMsgDispatcher_t *self);
typedef uint8_t (* netsocketSendData_t)(netsocketUserData_t *wud, const char *payload, int size);

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
typedef uint8_t (* initDispatcher_t)(compMsgDispatcher_t *self);
typedef uint8_t (* createMsgFromHeaderPart_t)(compMsgDispatcher_t *self, headerPart_t *hdr, uint8_t **handle);
typedef uint8_t (* encryptMsg_t)(const uint8_t *msg, size_t mlen, const uint8_t *key, size_t klen, const uint8_t *iv, size_t ivlen, uint8_t **buf, int *lgth);
typedef uint8_t (* decryptMsg_t)(const uint8_t *msg, size_t mlen, const uint8_t *key, size_t klen, const uint8_t *iv, size_t ivlen, uint8_t **buf, int *lgth);
typedef uint8_t (* toBase64_t)(const uint8_t *msg, size_t *len, uint8_t **encoded);
typedef uint8_t (* fromBase64_t)(const uint8_t *encodedMsg, size_t *len, uint8_t **decodedMsg);
typedef uint8_t (* resetBuildMsgInfos_t)(compMsgDispatcher_t *self);
typedef uint8_t (* getFieldType_t)(compMsgDispatcher_t *self, compMsgData_t *compMsgData, uint8_t fieldNameId, uint8_t *fieldTypeId);

// BuildMsg stuff
typedef uint8_t (* fixOffsetsForKeyValues_t)(compMsgDispatcher_t *self);
typedef uint8_t (* setMsgValues_t)(compMsgDispatcher_t *self);
typedef uint8_t (* setMsgFieldValue_t)(compMsgDispatcher_t *self, uint8_t type);
typedef uint8_t (* buildMsg_t)(compMsgDispatcher_t *self);
typedef uint8_t (* forwardMsg_t)(compMsgDispatcher_t *self);

// Identify stuff
typedef uint8_t (* resetHeaderInfos_t)(compMsgDispatcher_t *self);
typedef uint8_t (* nextFittingEntry_t)(compMsgDispatcher_t *self, uint8_t u8CmdKey, uint16_t u16CmdKey);
typedef uint8_t (* handleToSendPart_t)(compMsgDispatcher_t *self, const uint8_t * buffer, uint8_t lgth);
typedef uint8_t (* prepareAnswerMsg_t)(compMsgDispatcher_t *self, uint8_t **handle);
typedef uint8_t (* handleReceivedHeader_t)(compMsgDispatcher_t *self);
typedef uint8_t (* handleReceivedMsg_t)(compMsgDispatcher_t *self);
typedef uint8_t (* handleReceivedPart_t)(compMsgDispatcher_t *self, const uint8_t * buffer, uint8_t lgth);

// SendReceive
typedef uint8_t (* uartSetup_t)(compMsgDispatcher_t *self, unsigned id, uint32_t baud, int databits, int parity, int stopbits);
typedef uint8_t (* uartReceiveCb_t)(compMsgDispatcher_t *self, const uint8_t *buffer, uint8_t lgth);
typedef uint8_t (* typeRSendAnswer_t)(compMsgDispatcher_t *self, uint8_t *data, uint8_t msgLgth);
typedef uint8_t (* sendCloudMsg_t)(compMsgDispatcher_t *self, uint8_t *msgData, size_t msgLgth);
typedef uint8_t (* sendMsg_t)(compMsgDispatcher_t *self, uint8_t *msgData, size_t msgLgth);


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
  uint8_t websocketError;
  bssScanInfos_t *bssScanInfos;

  msgHeaderInfos_t msgHeaderInfos;

  compMsgTypesAndNames_t *compMsgTypesAndNames;

  // station mode
  uint32_t station_ip;

  // compMsgMsgDesc info
  compMsgMsgDesc_t *compMsgMsgDesc;

  // request infos
  msgRequestInfos_t msgRequestInfos;

  // this is for mapping a msg handle from the header to a compMsgPtr
  uint8_t numMsgHeaders;
  uint8_t maxMsgHeaders;
  msgHeader2MsgPtr_t *msgHeader2MsgPtrs;

  compMsgData_t *compMsgData;
  uint8_t *msgHandle;

  // function pointers

  // SendReceive
  uartSetup_t uartSetup;
  uartReceiveCb_t uartReceiveCb;
  typeRSendAnswer_t typeRSendAnswer;
  sendMsg_t sendMsg;
  sendCloudMsg_t sendCloudMsg;

  // Action
  setActionEntry_t setActionEntry;
  runAction_t runAction;
  getActionCallback_t getActionCallback;
  getActionCallbackName_t getActionCallbackName;
  getActionMode_t getActionMode;
  fillMsgValue_t fillMsgValue;
  getBssScanInfo_t getBssScanInfo;

  // BuildMsg
  setMsgFieldValue_t setMsgFieldValue;
  fixOffsetsForKeyValues_t fixOffsetsForKeyValues;
  buildMsg_t buildMsg;
  setMsgValues_t setMsgValues;
  forwardMsg_t forwardMsg;

  // ModuleData
  getModuleValue_t getModuleValue;
  setModuleValue_t setModuleValue;
  setModuleValues_t setModuleValues;
  updateModuleValues_t updateModuleValues;
  getModuleTableFieldValue_t getModuleTableFieldValue;

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

  // identify
  resetHeaderInfos_t resetHeaderInfos;
  nextFittingEntry_t nextFittingEntry;
  prepareAnswerMsg_t prepareAnswerMsg;
  handleReceivedPart_t handleReceivedPart;
  handleToSendPart_t handleToSendPart;
  handleReceivedHeader_t handleReceivedHeader;
  handleReceivedMsg_t handleReceivedMsg;

  // wifi
  bssStr2BssInfoId_t bssStr2BssInfoId;
  getScanInfoTableFieldValue_t getScanInfoTableFieldValue;
  getWifiValue_t getWifiValue;
  getWifiConfig_t getWifiConfig;
  setWifiValue_t setWifiValue;
  getWifiKeyValueInfo_t getWifiKeyValueInfo;
  getWifiKeyValue_t getWifiKeyValue;
  getWifiRemotePort_t getWifiRemotePort;
  websocketRunAPMode_t websocketRunAPMode;

  websocketRunClientMode_t websocketRunClientMode;
  websocketSendData_t websocketSendData;

  netsocketStartCloudSocket_t netsocketStartCloudSocket;
  netsocketRunClientMode_t netsocketRunClientMode;
  netsocketSendData_t netsocketSendData;

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
uint8_t compMsgWebsocketInit(compMsgDispatcher_t *self);
uint8_t compMsgNetsocketInit(compMsgDispatcher_t *self);

#endif	/* COMP_MSG_DISPATCHER_H */
