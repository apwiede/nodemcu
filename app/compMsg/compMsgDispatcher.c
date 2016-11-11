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
 * File:   compMsgDispatcher.c
 * Author: Arnulf P. Wiedemann
 *
 * Created on October 2st, 2016
 */

#include "osapi.h"
#include "c_types.h"
#include "mem.h"
#include "flash_fs.h"

#include "c_string.h"
#include "c_stdlib.h"
#include "c_stdio.h"
#include "c_limits.h"
#include "platform.h"
#include "compMsgDispatcher.h"
#include "../crypto/mech.h"

#define BASE64_INVALID '\xff'
#define BASE64_PADDING '='
#define ISBASE64(c) (unbytes64[c] != BASE64_INVALID)

#define DISP_HANDLE_PREFIX "stmsgdisp_"

typedef struct handle2Dispatcher
{
  uint8_t *handle;
  compMsgDispatcher_t *compMsgDispatcher;
} handle2Dispatcher_t;

typedef struct compMsgDispatcherHandles
{
  handle2Dispatcher_t *handles;
  int numHandles;
} compMsgDispatcherHandles_t;

// create an object
static compMsgDispatcherHandles_t compMsgDispatcherHandles = { NULL, 0};

static int compMsgDispatcherId = 0;
// right now we only need one dispatcher!
static compMsgDispatcher_t *compMsgDispatcherSingleton = NULL;

static const uint8 b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// ============================= toBase64 ========================

static uint8_t toBase64(const uint8_t *msg, size_t *len, uint8_t **encoded) {
  size_t i;
  size_t n;
  uint8_t *q;
  uint8_t *out;
  uint8_t bytes64[sizeof(b64)];

  n = *len;
  if (!n) { // handle empty string case 
    return COMP_DISP_ERR_OUT_OF_MEMORY;
  }
  out = (uint8_t *)os_zalloc((n + 2) / 3 * 4);
  checkAllocOK(out);
  c_memcpy(bytes64, b64, sizeof(b64));   //Avoid lots of flash unaligned fetches
  
  for (i = 0, q = out; i < n; i += 3) {
    int a = msg[i];
    int b = (i + 1 < n) ? msg[i + 1] : 0;
    int c = (i + 2 < n) ? msg[i + 2] : 0;
    *q++ = bytes64[a >> 2];
    *q++ = bytes64[((a & 3) << 4) | (b >> 4)];
    *q++ = (i + 1 < n) ? bytes64[((b & 15) << 2) | (c >> 6)] : BASE64_PADDING;
    *q++ = (i + 2 < n) ? bytes64[(c & 63)] : BASE64_PADDING;
  }
  *len = q - out;
  *encoded = out;
  return COMP_DISP_ERR_OK;
}

// ============================= fromBase64 ========================

static uint8_t fromBase64(const uint8_t *encodedMsg, size_t *len, uint8_t **decodedMsg) {
  int i;
  int n;
  int blocks;
  int pad;
  const uint8 *p;
  uint8_t unbytes64[UCHAR_MAX+1];
  uint8_t *msg;
  uint8_t *q;

  n = *len;
  blocks = (n>>2);
  pad = 0;
  if (!n) { // handle empty string case 
    return COMP_DISP_ERR_OUT_OF_MEMORY;
  } 
  if (n & 3) {
    return COMP_DISP_ERR_INVALID_BASE64_STRING;
  } 
  c_memset(unbytes64, BASE64_INVALID, sizeof(unbytes64));
  for (i = 0; i < sizeof(b64)-1; i++) {
    unbytes64[b64[i]] = i;  // sequential so no exceptions 
  }
  if (encodedMsg[n-1] == BASE64_PADDING) {
    pad =  (encodedMsg[n-2] != BASE64_PADDING) ? 1 : 2;
    blocks--;  //exclude padding block
  }    

  for (i = 0; i < n - pad; i++) {
    if (!ISBASE64(encodedMsg[i])) {
      return COMP_DISP_ERR_INVALID_BASE64_STRING;
    }
  }
  unbytes64[BASE64_PADDING] = 0;
  q = (uint8_t *) os_zalloc(1+ (3 * n / 4)); 
  checkAllocOK(q);
  msg = q;
  for (i = 0, p = encodedMsg; i<blocks; i++) {
    uint8 a = unbytes64[*p++]; 
    uint8 b = unbytes64[*p++]; 
    uint8 c = unbytes64[*p++]; 
    uint8 d = unbytes64[*p++];
    *q++ = (a << 2) | (b >> 4);
    *q++ = (b << 4) | (c >> 2);
    *q++ = (c << 6) | d;
  }
  if (pad) { //now process padding block bytes
    uint8 a = unbytes64[*p++];
    uint8 b = unbytes64[*p++];
    *q++ = (a << 2) | (b >> 4);
    if (pad == 1) *q++ = (b << 4) | (unbytes64[*p] >> 2);
  }
  *len = q - msg;
  *decodedMsg = msg;
  return COMP_DISP_ERR_OK;;
}

// ============================= addHandle ========================

static int addHandle(uint8_t *handle, compMsgDispatcher_t *compMsgDispatcher) {
  int idx;

  if (compMsgDispatcherHandles.handles == NULL) {
    compMsgDispatcherHandles.handles = os_zalloc(sizeof(handle2Dispatcher_t));
    if (compMsgDispatcherHandles.handles == NULL) {
      return COMP_DISP_ERR_OUT_OF_MEMORY;
    } else {
      compMsgDispatcherHandles.handles[compMsgDispatcherHandles.numHandles].handle = handle;
      compMsgDispatcherHandles.handles[compMsgDispatcherHandles.numHandles].compMsgDispatcher = compMsgDispatcher;
      compMsgDispatcherHandles.numHandles++;
      return COMP_DISP_ERR_OK;
    }
  } else {
    // check for unused slot first
    idx = 0;
    while (idx < compMsgDispatcherHandles.numHandles) {
      if (compMsgDispatcherHandles.handles[idx].handle == NULL) {
        compMsgDispatcherHandles.handles[idx].handle = handle;
        compMsgDispatcherHandles.handles[idx].compMsgDispatcher = compMsgDispatcher;
        return COMP_DISP_ERR_OK;
      }
      idx++;
    }
    compMsgDispatcherHandles.handles = os_realloc(compMsgDispatcherHandles.handles, sizeof(handle2Dispatcher_t)*(compMsgDispatcherHandles.numHandles+1));
    checkAllocOK(compMsgDispatcherHandles.handles);
    compMsgDispatcherHandles.handles[compMsgDispatcherHandles.numHandles].handle = handle;
    compMsgDispatcherHandles.handles[compMsgDispatcherHandles.numHandles].compMsgDispatcher = compMsgDispatcher;
    compMsgDispatcherHandles.numHandles++;
  }
  return COMP_DISP_ERR_OK;
}

// ============================= deleteHandle ========================

static int deleteHandle(const uint8_t *handle) {
  int idx;
  int numUsed;
  int found;

  if (compMsgDispatcherHandles.handles == NULL) {
ets_printf("deleteHandle 1 HANLDE_NOT_FOUND\n");
    return COMP_DISP_ERR_HANDLE_NOT_FOUND;
  }
  found = 0;
  idx = 0;
  numUsed = 0;
  while (idx < compMsgDispatcherHandles.numHandles) {
    if ((compMsgDispatcherHandles.handles[idx].handle != NULL) && (c_strcmp(compMsgDispatcherHandles.handles[idx].handle, handle) == 0)) {
      compMsgDispatcherHandles.handles[idx].handle = NULL;
      found++;
    } else {
      if (compMsgDispatcherHandles.handles[idx].handle != NULL) {
        numUsed++;
      }
    }
    idx++;
  }
  if (numUsed == 0) {
    os_free(compMsgDispatcherHandles.handles);
    compMsgDispatcherHandles.handles = NULL;
  }
  if (found) {
      return COMP_DISP_ERR_OK;
  }
ets_printf("deleteHandle 2 HANLDE_NOT_FOUND\n");
  return COMP_DISP_ERR_HANDLE_NOT_FOUND;
}

// ============================= checkHandle ========================

static int checkHandle(const char *handle, compMsgDispatcher_t **compMsgDispatcher) {
  int idx;

  if (compMsgDispatcherHandles.handles == NULL) {
ets_printf("checkHandle 1 HANLDE_NOT_FOUND\n");
    return COMP_DISP_ERR_HANDLE_NOT_FOUND;
  }
  idx = 0;
  while (idx < compMsgDispatcherHandles.numHandles) {
    if ((compMsgDispatcherHandles.handles[idx].handle != NULL) && (c_strcmp(compMsgDispatcherHandles.handles[idx].handle, handle) == 0)) {
      *compMsgDispatcher = compMsgDispatcherHandles.handles[idx].compMsgDispatcher;
      return COMP_DISP_ERR_OK;
    }
    idx++;
  }
ets_printf("checkHandle 2 HANLDE_NOT_FOUND\n");
  return COMP_DISP_ERR_HANDLE_NOT_FOUND;
}

// ================================= dumpMsgParts ====================================

static uint8_t dumpMsgParts(compMsgDispatcher_t *self, msgParts_t *msgParts) {
  int idx;

  ets_printf("dumpMsgParts:\n");
  ets_printf("MsgParts1 form: 0x%04x to: 0x%04x totalLgth: %d GUID: %d srcId: %d u16_cmdKey: %d\n", msgParts->fromPart, msgParts->toPart, msgParts->totalLgth, msgParts->GUID, msgParts->srcId, msgParts->u16CmdKey);

  ets_printf("MsgParts2 lgth: %d fieldOffset: %d\n", msgParts->lgth, msgParts->fieldOffset);
  ets_printf("buf");
  idx = 0;
  while (idx < msgParts->realLgth - 1) {
    ets_printf(" %d 0x%02x", idx, msgParts->buf[idx]);
    idx++;
  }
  ets_printf("\n");
  ets_printf("partFlags: ");
  if (msgParts->partsFlags & COMP_DISP_U16_CMD_KEY) {
    ets_printf(" COMP_DISP_U16_CMD_KEY");
  }
  if (msgParts->partsFlags & COMP_DISP_U8_VECTOR_GUID) {
    ets_printf(" COMP_DISP_U8_VECTOR_GUID");
  }
  if (msgParts->partsFlags & COMP_DISP_U16_SRC_ID) {
    ets_printf(" COMP_DISP_U16_SRC_ID");
  }
  if (msgParts->partsFlags & COMP_DISP_U16_CMD_KEY) {
    ets_printf(" COMP_DISP_U16_CMD_KEY");
  }
  ets_printf("\n");
  return COMP_DISP_ERR_OK;
}

// ================================= getNewCompMsgDataPtr ====================================

static uint8_t getNewCompMsgDataPtr(compMsgDispatcher_t *self) {
  int firstFreeEntryId;
  msgHeader2MsgPtr_t *newHeaderEntry;
  uint8_t result;

  if (self->numMsgHeaders >= self->maxMsgHeaders) {
    if (self->maxMsgHeaders == 0) {
      self->maxMsgHeaders = 4;
      self->msgHeader2MsgPtrs = (msgHeader2MsgPtr_t *)os_zalloc((self->maxMsgHeaders * sizeof(msgHeader2MsgPtr_t)));
      checkAllocOK(self->msgHeader2MsgPtrs);
    } else {
      self->maxMsgHeaders += 2;
      self->msgHeader2MsgPtrs = (msgHeader2MsgPtr_t *)os_realloc(self->msgHeader2MsgPtrs, (self->maxMsgHeaders * sizeof(msgHeader2MsgPtr_t)));
      checkAllocOK(self->msgHeader2MsgPtrs);
    }
  }
  newHeaderEntry = &self->msgHeader2MsgPtrs[self->numMsgHeaders];
  newHeaderEntry->headerLgth = 0;
  newHeaderEntry->compMsgData = newCompMsgData();
  newHeaderEntry->compMsgData->setDispatcher(newHeaderEntry->compMsgData, self);
  self->compMsgData = newHeaderEntry->compMsgData;
  self->numMsgHeaders++;
  return COMP_DISP_ERR_OK;
}

// ================================= getFieldType ====================================

static uint8_t getFieldType(compMsgDispatcher_t *self, compMsgData_t *compMsgData, uint8_t fieldNameId, uint8_t *fieldTypeId) {
  int idx;
  compMsgField_t *fieldInfo;

  idx = 0;
  while (idx < compMsgData->numFields) {
    fieldInfo = &compMsgData->fields[idx];
    if (fieldInfo->fieldNameId == fieldNameId) {
      *fieldTypeId = fieldInfo->fieldTypeId;
      return COMP_DISP_ERR_OK;
    }
    idx++;
  }
  // and now check the table fields
  idx = 0;
  while (idx < compMsgData->numTableRowFields) {
    fieldInfo = &compMsgData->tableFields[idx];
    if (fieldInfo->fieldNameId == fieldNameId) {
      *fieldTypeId = fieldInfo->fieldTypeId;
      return COMP_DISP_ERR_OK;
    }
    idx++;
  }
  return COMP_DISP_ERR_FIELD_NOT_FOUND;
}

// ================================= resetBuildMsgInfos ====================================

static uint8_t resetBuildMsgInfos(compMsgDispatcher_t *self) {
  self->compMsgData->buildMsgInfos.numRows = 0;
  self->compMsgData->buildMsgInfos.tableRow = 0;
  self->compMsgData->buildMsgInfos.tableCol = 0;
  self->compMsgData->buildMsgInfos.numericValue = 0;
  self->compMsgData->buildMsgInfos.stringValue = NULL;
  self->compMsgData->buildMsgInfos.actionName = NULL;
  return COMP_DISP_ERR_OK;
}

// ================================= createMsgFromHeaderPart ====================================

static uint8_t createMsgFromHeaderPart (compMsgDispatcher_t *self, headerPart_t *hdr, uint8_t **handle) {
  uint8_t result;
  int idx;
  bool isEnd;
  msgDescPart_t *msgDescPart;
  msgValPart_t *msgValPart;

  result = self->compMsgMsgDesc->getMsgPartsFromHeaderPart(self, hdr, handle);
  checkErrOK(result);
  result = self->compMsgData->createMsg(self->compMsgData, self->compMsgData->numMsgDescParts, handle);
  checkErrOK(result);
  idx = 0;
  while(idx < self->compMsgData->numMsgDescParts) {
    msgDescPart = &self->compMsgData->msgDescParts[idx];
    result = self->compMsgData->addField(self->compMsgData, msgDescPart->fieldNameStr, msgDescPart->fieldTypeStr, msgDescPart->fieldLgth);
    checkErrOK(result);
    idx++;
  }

ets_printf("heap4: %d\n", system_get_free_heap_size());
  // runAction calls at the end buildMsg
//  self->resetBuildMsgInfos(self);
//  self->buildMsgInfos.u16CmdKey = hdr->hdrU16CmdKey; // used in buildMsg -> setMsgValues!!
  if (self->compMsgData->prepareValuesCbName != NULL) {
    uint8_t actionMode;
    uint8_t type;

    result = self->getActionMode(self, self->compMsgData->prepareValuesCbName+1, &actionMode);
    self->actionMode = actionMode;
    checkErrOK(result);
    result  = self->runAction(self, &type);
    // runAction starts a call with a callback and returns here before the callback has been running!!
    // when when coming here we are finished and the callback will do the work later on!
ets_printf("runAction done\n");
    return result;
  } else {
    result = self->buildMsg(self);
#ifdef NOTDEF
    result = setMsgValues(self);
    checkErrOK(result);
//ets_printf("§heap3: %d§", system_get_free_heap_size());
    result = self->compMsgData->getMsgData(self->compMsgData, &data, &msgLgth);
    checkErrOK(result);
#endif
    // FIXME !! here we need a call to send the (eventually encrypted) message!!
  }
  return COMP_MSG_ERR_OK;
}

// ================================= resetMsgInfo ====================================

static uint8_t resetMsgInfo(compMsgDispatcher_t *self, msgParts_t *parts) {
  parts->lgth = 0;
  parts->fieldOffset = 0;
  parts->fromPart = 0;
  parts->toPart = 0;
  parts->totalLgth = 0;
  c_memcpy(parts->GUID, "                ", 16);
  parts->srcId = 0;
  parts->u16CmdKey = 0;
#ifdef NOTUSED
  self->compMsgDataView->dataView->data = parts->buf;
  self->compMsgDataView->dataView->lgth = 0;
#endif
  return COMP_DISP_ERR_OK;
}

// ============================= encryptMsg ========================

static uint8_t encryptMsg(const uint8_t *msg, size_t mlen, const uint8_t *key, size_t klen, const uint8_t *iv, size_t ivlen, uint8_t **buf, int *lgth) {
  const crypto_mech_t *mech;
  size_t bs;
  size_t clen;
  uint8_t *crypted;

  *buf = NULL;
  *lgth = 0;
  mech = crypto_encryption_mech ("AES-CBC");
  if (mech == NULL) {
    return COMP_MSG_ERR_CRYPTO_BAD_MECHANISM;
  }
  bs = mech->block_size;
  clen = ((mlen + bs - 1) / bs) * bs;
  *lgth = clen;
//ets_printf("dlen: %d lgth: %d clen: %d data: %p\n", dlen, *lgth, clen, data);
  crypted = (uint8_t *)os_zalloc (clen);
  if (!crypted) {
    return COMP_MSG_ERR_CRYPTO_INIT_FAILED;
  } 
  *buf = crypted;
  crypto_op_t op =
  { 
    key, klen,
    iv, ivlen,
    msg, mlen,
    crypted, clen,
    OP_ENCRYPT
  }; 
  if (!mech->run (&op)) { 
    os_free (*buf);
    return COMP_MSG_ERR_CRYPTO_INIT_FAILED;
  } 
  return COMP_MSG_ERR_OK;
}

// ============================= decryptMsg ========================

static uint8_t decryptMsg(const uint8_t *msg, size_t mlen, const uint8_t *key, size_t klen, const uint8_t *iv, size_t ivlen, uint8_t **buf, int *lgth) {
  const crypto_mech_t *mech;
  size_t bs;
  size_t clen;
  uint8_t *crypted;

  *buf = NULL;
  *lgth = 0;
  mech = crypto_encryption_mech ("AES-CBC");
  if (mech == NULL) {
    return COMP_MSG_ERR_CRYPTO_BAD_MECHANISM;
  }
  bs = mech->block_size;
  clen = ((mlen + bs - 1) / bs) * bs;
  *lgth = clen;
  crypted = (uint8_t *)os_zalloc (*lgth);
  if (!crypted) {
    return COMP_MSG_ERR_CRYPTO_INIT_FAILED;
  } 
  *buf = crypted;
  crypto_op_t op =
  { 
    key, klen,
    iv, ivlen,
    msg, mlen,
    crypted, clen,
    OP_DECRYPT
  }; 
  if (!mech->run (&op)) { 
    os_free (*buf);
    return COMP_MSG_ERR_CRYPTO_INIT_FAILED;
  }
  return COMP_MSG_ERR_OK;
}

// ============================= compMsgDispatcherGetPtrFromHandle ========================

uint8_t compMsgDispatcherGetPtrFromHandle(const char *handle, compMsgDispatcher_t **compMsgDispatcher) {

  if (checkHandle(handle, compMsgDispatcher) != COMP_DISP_ERR_OK) {
ets_printf("compMsgDispatcherGetPtrFromHandle 1 HANLDE_NOT_FOUND\n");
    return COMP_DISP_ERR_HANDLE_NOT_FOUND;
  }
  return COMP_DISP_ERR_OK;
}

// ================================= startRequest ====================================

static uint8_t startRequest(compMsgDispatcher_t *self) {
  uint8_t result;

ets_printf("should start request: %d\n", self->msgRequestInfos.currRequestIdx);
  return COMP_DISP_ERR_OK;
}

// ================================= startNextRequest ====================================

static uint8_t startNextRequest(compMsgDispatcher_t *self) {
  uint8_t result;

  if (self->msgRequestInfos.currRequestIdx < 0) {
    if (self->msgRequestInfos.currRequestIdx < self->msgRequestInfos.lastRequestIdx) {
      self->msgRequestInfos.currRequestIdx = 0;
      result = startRequest(self);
      checkErrOK(result);
      return COMP_DISP_ERR_OK;
    }
  }
  if (self->msgRequestInfos.currRequestIdx < self->msgRequestInfos.lastRequestIdx) {
    self->msgRequestInfos.currRequestIdx++;
    result = startRequest(self);
    checkErrOK(result);
  }
  return COMP_DISP_ERR_OK;
}

// ================================= addUartRequestData ====================================

static uint8_t addUartRequestData(compMsgDispatcher_t *self, uint8_t *data, size_t lgth) {
  uint8_t result;
  compMsgData_t *compMsgData;

  // slot 0 is reserved for Uart
  if (self->msgRequestInfos.requestTypes[0] != COMP_DISP_INPUT_UART) {
    return COMP_DISP_ERR_UART_REQUEST_NOT_SET;
  }
  compMsgData = self->msgRequestInfos.requestData[0];
  compMsgData->direction = COMP_MSG_RECEIVED_DATA;
  result = self->handleReceivedPart(self, data, lgth);
  checkErrOK(result);
  return COMP_DISP_ERR_OK;
}

// ================================= addRequest ====================================

static uint8_t addRequest(compMsgDispatcher_t *self, uint8_t requestType, void *requestHandle, compMsgData_t *requestData) {
  uint8_t result;
  compMsgData_t *compMsgData;

  if (self->msgRequestInfos.lastRequestIdx >= COMP_DISP_MAX_REQUESTS) {
    return COMP_DISP_ERR_TOO_MANY_REQUESTS;
  }
  self->msgRequestInfos.lastRequestIdx++;
//ets_printf("addRequest: lastRequestIdx: %d requestType: %d compMsgData: %p\n", self->msgRequestInfos.lastRequestIdx, requestType, requestData);
  self->msgRequestInfos.requestTypes[self->msgRequestInfos.lastRequestIdx] = requestType;
  self->msgRequestInfos.requestHandles[self->msgRequestInfos.lastRequestIdx] = requestHandle;
  self->msgRequestInfos.requestData[self->msgRequestInfos.lastRequestIdx] = requestData;
//FIXME TEMPORARY last if clause!!
  if ((self->msgRequestInfos.currRequestIdx < 1) || (requestData->direction == COMP_MSG_TO_SEND_DATA)) {
    self->msgRequestInfos.currRequestIdx++;
    checkErrOK(result);
    compMsgData = self->msgRequestInfos.requestData[self->msgRequestInfos.currRequestIdx];
    switch (compMsgData->direction) {
    case COMP_MSG_TO_SEND_DATA:
      result = self->handleToSendPart(self, compMsgData->toSendData, compMsgData->toSendLgth);
      break;
    case COMP_MSG_RECEIVED_DATA:
      result = self->handleReceivedPart(self, compMsgData->receivedData, compMsgData->receivedLgth);
      break;
    default:
ets_printf("bad direction: 0x%02x 0x%02x\n", compMsgData->direction, requestData->direction);
      return COMP_MSG_ERR_BAD_VALUE;
    }
  }
  return COMP_DISP_ERR_OK;
}

// ================================= deleteRequest ====================================

static uint8_t deleteRequest(compMsgDispatcher_t *self, uint8_t requestType, void *requestHandle) {
  uint8_t result;
  int idx;
  bool found;
  int idxToStart;
  int idxDeleted;

  idx = 0;
  idxToStart = -1;
  idxDeleted = -1;
  found = false;
  while (idx < self->msgRequestInfos.lastRequestIdx) {
    if (idx >= COMP_DISP_MAX_REQUESTS) {
      return COMP_DISP_ERR_REQUEST_NOT_FOUND;
    }
    if (!found) {
      if ((self->msgRequestInfos.requestTypes[idx] == requestType) && (self->msgRequestInfos.requestHandles[idx] == requestHandle)) {
        found = true;
        idxDeleted = idx;
        if (idx == self->msgRequestInfos.currRequestIdx) {
          if (idx < self->msgRequestInfos.lastRequestIdx) {
            idxToStart = idx;
          }
          self->msgRequestInfos.currRequestIdx = -1;
        }
      }
    } else {
      // move the following entries one idx down
      if (idx < self->msgRequestInfos.lastRequestIdx) {
        self->msgRequestInfos.requestTypes[idx] = self->msgRequestInfos.requestTypes[idx + 1];
        self->msgRequestInfos.requestHandles[idx] = self->msgRequestInfos.requestHandles[idx + 1];
        if (idx + 1 == self->msgRequestInfos.currRequestIdx) {
          self->msgRequestInfos.currRequestIdx--;
        }
      }
    }
    idx++;
  }
  if (self->msgRequestInfos.lastRequestIdx >= 0) {
    self->msgRequestInfos.lastRequestIdx--;
  }
  if (self->msgRequestInfos.currRequestIdx < 0) {
    self->msgRequestInfos.currRequestIdx++;
    // start handling the request
    self->startNextRequest(self);
  } else {
    // nothing to do the current request is different from the deleted one
    // so just let the current one continue
  }
  return COMP_DISP_ERR_OK;
}

// ================================= initDispatcher ====================================

static uint8_t initDispatcher(compMsgDispatcher_t *self) {
  uint8_t result;
headerPart_t *hdr;
uint8_t *handle;

  result = compMsgIdentifyInit(self);
  checkErrOK(result);
  result = compMsgBuildMsgInit(self);
  checkErrOK(result);
  result = compMsgSendReceiveInit(self);
  checkErrOK(result);
  result = compMsgActionInit(self);
  checkErrOK(result);
  result = compMsgWifiInit(self);
  checkErrOK(result);
  result = compMsgModuleDataInit(self);
  checkErrOK(result);
  result = compMsgWebsocketInit(self);
  checkErrOK(result);
  result = compMsgNetsocketInit(self);
  checkErrOK(result);

#define WEBSOCKETAP
#ifdef WEBSOCKETAP
// FIXME !! temporary starting for testing only !!
ets_printf("start RunAPMode\n");
  result = self->websocketRunAPMode(self);
  checkErrOK(result);
#endif

#ifdef NETSOCKET
// FIXME !! temporary starting for testing only !!
ets_printf("start RunClientMode\n");
  result = self->netsocketRunClientMode(self);
  checkErrOK(result);
#endif

#ifdef CLOUDSOCKET
// FIXME !! temporary starting for testing only !!
ets_printf("start startCloudSocket\n");
  result = self->netsocketStartCloudSocket(self);
  checkErrOK(result);
#endif

#ifdef NOTDEF
result = self->compMsgMsgDesc->getHeaderFromUniqueFields(self, 16640,22272, 0x4141, &hdr);
checkErrOK(result);
result = self->createMsgFromHeaderPart(self, hdr, &handle);
ets_printf("handle: %s result: %d\n", handle, result);
checkErrOK(result);
#endif
  return COMP_DISP_ERR_OK;
}

// ================================= createDispatcher ====================================

static uint8_t createDispatcher(compMsgDispatcher_t *self, uint8_t **handle) {
  uint8_t result;

  os_sprintf(self->handle, "%s%p", DISP_HANDLE_PREFIX, self);
  result = addHandle(self->handle, self);
  if (result != COMP_DISP_ERR_OK) {
    deleteHandle(self->handle);
    os_free(self);
    return result;
  }
  resetMsgInfo(self, &self->received);
  resetMsgInfo(self, &self->toSend);
  *handle = self->handle;
  return COMP_DISP_ERR_OK;
}

// ================================= newCompMsgDispatcher ====================================

compMsgDispatcher_t *newCompMsgDispatcher() {
  if (compMsgDispatcherSingleton != NULL) {
    return compMsgDispatcherSingleton;
  }
  compMsgDispatcher_t *compMsgDispatcher = os_zalloc(sizeof(compMsgDispatcher_t));
  if (compMsgDispatcher == NULL) {
    return NULL;
  }

  compMsgDispatcher->compMsgMsgDesc = newCompMsgMsgDesc();
  compMsgDispatcher->compMsgDataView = newCompMsgDataView(); // only used for readHeadersAndSetFlags !!

  compMsgDispatcherId++;
  compMsgDispatcher->id = compMsgDispatcherId;

  compMsgDispatcher->numMsgHeaders = 0;
  compMsgDispatcher->maxMsgHeaders = 0;
  compMsgDispatcher->msgHeader2MsgPtrs = NULL;

  // request handling
  compMsgDispatcher->msgRequestInfos.currRequestIdx = -1;
  compMsgDispatcher->msgRequestInfos.lastRequestIdx = -1;

  compMsgDispatcher->msgHeaderInfos.headerParts = NULL;
  compMsgDispatcher->msgHeaderInfos.numHeaderParts = 0;
  compMsgDispatcher->msgHeaderInfos.maxHeaderParts = 0;

  compMsgDispatcher->createDispatcher = &createDispatcher;
  compMsgDispatcher->initDispatcher = &initDispatcher;

  // request handling
  compMsgDispatcher->startRequest = &startRequest;
  compMsgDispatcher->startNextRequest = &startNextRequest;
  compMsgDispatcher->addRequest = &addRequest;
  compMsgDispatcher->addUartRequestData = &addUartRequestData;
  compMsgDispatcher->deleteRequest = &deleteRequest;


  compMsgDispatcher->resetMsgInfo = &resetMsgInfo;
  compMsgDispatcher->createMsgFromHeaderPart = &createMsgFromHeaderPart;
  compMsgDispatcher->getNewCompMsgDataPtr = &getNewCompMsgDataPtr;

  compMsgDispatcher->encryptMsg = &encryptMsg;
  compMsgDispatcher->decryptMsg = &decryptMsg;
  compMsgDispatcher->toBase64 = &toBase64;
  compMsgDispatcher->fromBase64 = &fromBase64;
  compMsgDispatcher->resetBuildMsgInfos =&resetBuildMsgInfos;

  compMsgDispatcher->dumpMsgParts = &dumpMsgParts;

  compMsgDispatcher->getFieldType = &getFieldType;
  compMsgDispatcherSingleton = compMsgDispatcher;
  return compMsgDispatcher;
}

// ================================= freeCompMsgDispatcher ====================================

void freeCompMsgDispatcher(compMsgDispatcher_t *compMsgDispatcher) {
}


