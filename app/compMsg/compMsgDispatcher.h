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

#include "compMsgModuleData.h"
#include "compMsgWifiData.h"
#include "compMsgMsgDesc.h"

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

  COMP_DISP_ERR_OPEN_FILE             = 189,
  COMP_DISP_FILE_NOT_OPENED           = 188,
  COMP_DISP_ERR_FLUSH_FILE            = 187,
  COMP_DISP_ERR_WRITE_FILE            = 186,
  COMP_DISP_ERR_BAD_RECEIVED_LGTH     = 185,
  COMP_DISP_ERR_BAD_FILE_CONTENTS     = 184,
  COMP_DISP_ERR_HEADER_NOT_FOUND      = 183,
  COMP_DISP_ERR_DUPLICATE_FIELD       = 182,
  COMP_DISP_ERR_BAD_FIELD_NAME        = 181,
  COMP_DISP_ERR_BAD_HANDLE_TYPE       = 180,
  COMP_DISP_ERR_INVALID_BASE64_STRING = 179,
  COMP_DISP_ERR_TOO_FEW_FILE_LINES    = 178,
  COMP_DISP_ERR_ACTION_NAME_NOT_FOUND = 177,
  COMP_DISP_ERR_DUPLICATE_ENTRY       = 176,
};


// dst + src + totalLgth + (optional) GUID + cmdKey/shCmdKey
// uint16_t + uint16_t + uint16_t + (optional) uint8_t*(16) + uint16_t/uint8_t
#define DISP_MAX_HEADER_LGTH (sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint16_t) + (16*sizeof(uint8_t)) + sizeof(uint16_t))

typedef struct compMsgData compMsgData_t;

typedef struct msgHeader2MsgPtr {
  compMsgData_t *compMsgData;
  uint8_t headerLgth;
  uint8_t header[DISP_MAX_HEADER_LGTH];
} msgHeader2MsgPtr_t;

typedef struct buildMsgInfos {
  uint8_t numEntries;
  uint8_t type;
  msgParts_t *parts;
  uint8_t numRows; 
  uint8_t u8CmdKey;
  uint16_t u16CmdKey;
  uint16_t partsFlags;
  uint8_t *fieldNameStr;
  uint8_t *fieldValueStr;
  uint8_t fieldNameId;
  uint8_t fieldTypeId;
  uint8_t tableRow;
  uint8_t tableCol;
  int numericValue;
  size_t sizeValue;
  uint8_t *stringValue;
  uint8_t buf[100];
  uint8_t key[50];
  uint8_t *actionName;
} buildMsgInfos_t;

typedef struct buildListMsgInfos {
  size_t msgHeaderLgth;
  size_t msgDataLgth;
  uint8_t *msgData;
  size_t defDataLgth;
  size_t defHeaderLgth;
  uint8_t *defData;
  size_t encryptedMsgDataLgth;
  uint8_t *encryptedMsgData;
  size_t encryptedDefDataLgth;
  uint8_t *encryptedDefData;
  uint16_t src;
  uint16_t dst;
} buildListMsgInfos_t;

typedef struct websocketUserData websocketUserData_t;
typedef struct compMsgDispatcher compMsgDispatcher_t;

// Action stuff
typedef uint8_t (* setActionEntry_t)(compMsgDispatcher_t *self, uint8_t *actionName, uint8_t mode, uint8_t u8CmdKey, uint16_t u16CmdKey);
typedef uint8_t (* runAction_t)(compMsgDispatcher_t *self, uint8_t *answerType);
typedef uint8_t (* getActionMode_t)(compMsgDispatcher_t *self, uint8_t *actionName, uint8_t *actionMode);
typedef uint8_t (* fillMsgValue_t)(compMsgDispatcher_t *self, uint8_t *callbackName, uint8_t answerType, uint8_t fieldTypeId);

// Wifi stuff
typedef uint8_t (*bssStr2BssInfoId_t)(uint8_t *fieldName, uint8_t *fieldId);
typedef uint8_t (* getBssScanInfo_t)(compMsgDispatcher_t *self);
typedef uint8_t (* getScanInfoTableFieldValue_t)(compMsgDispatcher_t *self, uint8_t actionMode);
typedef uint8_t (* getWifiValue_t)(compMsgDispatcher_t *self, uint16_t which, uint8_t valueTypeId, int *numericValue, uint8_t **stringValue);
typedef uint8_t (* setWifiValues_t)(compMsgDispatcher_t *self);
typedef uint8_t (* getWifiKeyValue_t)(compMsgDispatcher_t *self);

// ModuleData stuff
typedef uint8_t (* setModuleValues_t)(compMsgDispatcher_t *self);
typedef uint8_t (* updateModuleValues_t)(compMsgDispatcher_t *self);
typedef uint8_t (* getModuleValue_t)(compMsgDispatcher_t *self, uint16_t which, uint8_t valueTypeId, int *numericValue, uint8_t **stringValue);
typedef uint8_t (* getModuleTableFieldValue_t)(compMsgDispatcher_t *self, uint8_t actionMode);
typedef uint8_t (* websocketRunClientMode_t)(compMsgDispatcher_t *self, uint8_t mode);
typedef uint8_t (* websocketRunAPMode_t)(compMsgDispatcher_t *self);
typedef uint8_t (* websocketSendData_t)(websocketUserData_t *wud, const char *payload, int size, int opcode);

// Dispatcher stuff
typedef uint8_t (* uartReceiveCb_t)(compMsgDispatcher_t *self, const uint8_t *buffer, uint8_t lgth);
typedef uint8_t (* dumpMsgParts_t)(compMsgDispatcher_t *self, msgParts_t *msgParts);

typedef uint8_t (* createDispatcher_t)(compMsgDispatcher_t *self, uint8_t **handle);
typedef uint8_t (* initDispatcher_t)(compMsgDispatcher_t *self);
typedef uint8_t (* createMsgFromHeaderPart_t)(compMsgDispatcher_t *self, headerPart_t *hdr, uint8_t **handle);
typedef uint8_t (* createMsgFromLines_t)(compMsgDispatcher_t *self, msgParts_t *parts, uint8_t numEntries, uint8_t numRows, uint8_t type);
typedef uint8_t (* encryptMsg_t)(const uint8_t *msg, size_t mlen, const uint8_t *key, size_t klen, const uint8_t *iv, size_t ivlen, uint8_t **buf, int *lgth);
typedef uint8_t (* decryptMsg_t)(const uint8_t *msg, size_t mlen, const uint8_t *key, size_t klen, const uint8_t *iv, size_t ivlen, uint8_t **buf, int *lgth);
typedef uint8_t (* toBase64_t)(const uint8_t *msg, size_t *len, uint8_t **encoded);
typedef uint8_t (* fromBase64_t)(const uint8_t *encodedMsg, size_t *len, uint8_t **decodedMsg);
typedef uint8_t (* resetHeaderInfos_t)(compMsgDispatcher_t *self);
typedef uint8_t (* resetBuildMsgInfos_t)(compMsgDispatcher_t *self);
typedef uint8_t (* getFieldType_t)(compMsgDispatcher_t *self, compMsgData_t *compMsgData, uint8_t fieldNameId, uint8_t *fieldTypeId);

// BuildMsg stuff
typedef uint8_t (* fixOffsetsForKeyValues_t)(compMsgDispatcher_t *self);
typedef uint8_t (* setMsgValues_t)(compMsgDispatcher_t *self);
typedef uint8_t (* setMsgFieldValue_t)(compMsgDispatcher_t *self, uint8_t *numTableRows, uint8_t *numTableRowFields, bool *nextFieldEntry, uint8_t type);
typedef uint8_t (* setMsgKeyValues_t)(compMsgDispatcher_t *self, uint8_t numEntries, uint8_t *entryIdx, uint8_t type, size_t *extraOffset);
typedef uint8_t (* buildMsg_t)(compMsgDispatcher_t *self);
typedef uint8_t (* buildListMsg_t)(compMsgDispatcher_t *self, size_t *totalLgth, uint8_t **totalData);
typedef uint8_t (* prepareAnswerMsg_t)(compMsgDispatcher_t *self, msgParts_t *parts, uint8_t type);

// Identify stuff
typedef uint8_t (* handleReceivedMsg_t)(compMsgDispatcher_t *self, msgParts_t *received, msgHeaderInfos_t *hdrInfos);
typedef uint8_t (* prepareNotEncryptedAnswer_t)(compMsgDispatcher_t *self, msgParts_t *parts, uint8_t type);
typedef uint8_t (* prepareEncryptedAnswer_t)(compMsgDispatcher_t *self, msgParts_t *parts, uint8_t type);
typedef uint8_t (* nextFittingEntry_t)(compMsgDispatcher_t *self, uint8_t u8CmdKey, uint16_t u16CmdKey);
typedef uint8_t (* handleReceivedPart_t)(compMsgDispatcher_t *self, const uint8_t * buffer, uint8_t lgth);

typedef uint8_t (* typeRSendAnswer_t)(compMsgDispatcher_t *self, uint8_t *data, uint8_t msgLgth);


// MsgData stuff
typedef uint8_t (* getNewCompMsgDataPtr_t)(compMsgDispatcher_t *self);
typedef uint8_t (* resetMsgInfo_t)(compMsgDispatcher_t *self, msgParts_t *parts);

typedef struct compMsgDispatcher {
  uint8_t id;
  char handle[20];
  uint8_t *FileName;
  uint8_t fileId;
  size_t fileSize;
  uint16_t dispFlags;
  int numericValue;
  uint8_t *stringValue;
  uint8_t actionMode;
  bssScanInfos_t *bssScanInfos;
  buildMsgInfos_t buildMsgInfos;
  buildListMsgInfos_t buildListMsgInfos;
  uint8_t tableRow;
  uint8_t tableCol;
  void *wud;
  
  msgHeaderInfos_t msgHeaderInfos;

  // this is for mapping a msg handle from teaf header to a compMsgPtr
  uint8_t numMsgHeaders;
  uint8_t maxMsgHeaders;
  msgHeader2MsgPtr_t *msgHeader2MsgPtrs;

  compMsgDataView_t *compMsgDataView;
  compMsgData_t *compMsgData;
  uint8_t *msgHandle;
  compMsgMsgDesc_t *compMsgMsgDesc;

  msgParts_t received;
  msgParts_t toSend;

  uint16_t McuPart;
  uint16_t WifiPart;
  uint16_t AppPart;
  uint16_t CloudPart;

  getFieldType_t getFieldType;
  resetMsgInfo_t resetMsgInfo;

  typeRSendAnswer_t typeRSendAnswer;

  // Action
  setActionEntry_t setActionEntry;
  runAction_t runAction;
  getActionMode_t getActionMode;
  fillMsgValue_t fillMsgValue;
  getBssScanInfo_t getBssScanInfo;

  // BuildMsg
  setMsgFieldValue_t setMsgFieldValue;
  setMsgKeyValues_t setMsgKeyValues;
  fixOffsetsForKeyValues_t fixOffsetsForKeyValues;
  buildMsg_t buildMsg;
  buildListMsg_t buildListMsg;
  setMsgValues_t setMsgValues;
  prepareAnswerMsg_t prepareAnswerMsg;
  handleReceivedMsg_t handleReceivedMsg;
  prepareNotEncryptedAnswer_t prepareNotEncryptedAnswer;
  prepareEncryptedAnswer_t prepareEncryptedAnswer;

  // ModuleData
  getModuleValue_t getModuleValue;
  setModuleValues_t setModuleValues;
  updateModuleValues_t updateModuleValues;
  getModuleTableFieldValue_t getModuleTableFieldValue;

  // Dispatcher
  resetHeaderInfos_t resetHeaderInfos;
  nextFittingEntry_t nextFittingEntry;
  handleReceivedPart_t handleReceivedPart;
  uartReceiveCb_t uartReceiveCb;
  createDispatcher_t createDispatcher;
  initDispatcher_t initDispatcher;
  createMsgFromHeaderPart_t createMsgFromHeaderPart;
  createMsgFromLines_t createMsgFromLines;
  getNewCompMsgDataPtr_t getNewCompMsgDataPtr;

  // dispatcher
  encryptMsg_t encryptMsg;
  decryptMsg_t decryptMsg;
  toBase64_t toBase64;
  fromBase64_t fromBase64;
  resetBuildMsgInfos_t resetBuildMsgInfos;

  // wifi
  bssStr2BssInfoId_t bssStr2BssInfoId;
  getScanInfoTableFieldValue_t getScanInfoTableFieldValue;
  getWifiValue_t getWifiValue;
  setWifiValues_t setWifiValues;
  getWifiKeyValue_t getWifiKeyValue;
  websocketRunClientMode_t websocketRunClientMode;
  websocketRunAPMode_t websocketRunAPMode;
  websocketSendData_t websocketSendData;

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

#endif	/* COMP_MSG_DISPATCHER_H */
