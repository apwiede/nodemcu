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
 * File:   compMsgRequest.c
 * Author: Arnulf P. Wiedemann
 *
 * Created on December 16th, 2016
 */

#include "osapi.h"
#include "c_types.h"
#include "mem.h"
#include "flash_fs.h"

#include "c_string.h"
#include "c_stdio.h"
#include "c_stdlib.h"
#include "compMsgDispatcher.h"

// ================================= startRequest ====================================

static uint8_t startRequest(compMsgDispatcher_t *self) {
  uint8_t result;
  compMsgData_t *compMsgData;
  compMsgData_t *requestData;
  msgRequestInfo_t *msgRequestInfo;

  result = COMP_MSG_ERR_OK;
  COMP_MSG_DBG(self, "R", 1, "should start request: %d\n", self->compMsgRequest->msgRequestInfos.currRequestIdx);
  msgRequestInfo = &self->compMsgRequest->msgRequestInfos.msgRequestInfo[self->compMsgRequest->msgRequestInfos.currRequestIdx];
  compMsgData = msgRequestInfo->requestData;
  if (compMsgData->direction == COMP_MSG_RECEIVED_DATA) {
    switch (compMsgData->direction) {
    case COMP_MSG_TRANSFER_DATA:
      result = self->compMsgSendReceive->sendMsg(self, compMsgData->receivedData, compMsgData->receivedLgth);
      break;
    case COMP_MSG_TO_SEND_DATA:
      //FIXME need code here!!
      // can that happen for COMP_MSG_RECEIVED_DATA?
      break;
    case COMP_MSG_RECEIVED_DATA:
      COMP_MSG_DBG(self, "R", 2, "COMP_MSG_RECEIVED_DATA: compMsgData: %p\n", compMsgData);
      COMP_MSG_DBG(self, "R", 2, "received: %p lgth: %d\n", compMsgData->receivedData, compMsgData->receivedLgth);
      result = self->compMsgIdentify->handleReceivedPart(self, compMsgData->receivedData, compMsgData->receivedLgth);
      break;
    default:
      COMP_MSG_DBG(self, "Y", 0, "bad direction: 0x%02x\n", compMsgData->direction);
      return COMP_MSG_ERR_BAD_VALUE;
    }
  } else {
    //FIXME need code here!!
    // eventually no code needed if to send data are always directly handled if a message has been received
  }
  return result;
}

// ================================= startNextRequest ====================================

static uint8_t startNextRequest(compMsgDispatcher_t *self) {
  uint8_t result;
  msgRequestInfo_t *msgRequestInfo;
  msgRequestInfos_t *msgRequestInfos;

  msgRequestInfos = &self->compMsgRequest->msgRequestInfos;
  msgRequestInfo = &msgRequestInfos->msgRequestInfo[msgRequestInfos->currRequestIdx];
  if (msgRequestInfos->currRequestIdx < 0) {
    if (msgRequestInfos->currRequestIdx < msgRequestInfos->lastRequestIdx) {
      msgRequestInfos->currRequestIdx = 0;
      result = startRequest(self);
      checkErrOK(result);
      return COMP_MSG_ERR_OK;
    }
  }
  if (msgRequestInfos->currRequestIdx < msgRequestInfos->lastRequestIdx) {
    msgRequestInfos->currRequestIdx++;
    result = startRequest(self);
    checkErrOK(result);
  }
  return COMP_MSG_ERR_OK;
}

// ================================= addUartRequestData ====================================

static uint8_t addUartRequestData(compMsgDispatcher_t *self, uint8_t *data, size_t lgth) {
  uint8_t result;
  int uartRequestIdx;
  msgRequestInfos_t *msgRequestInfos;
  msgRequestInfo_t *msgRequestInfo;
  compMsgData_t *compMsgData;
  bool isComplete;

  // slot 0 is reserved for Uart
  uartRequestIdx = 0;
  msgRequestInfos = &self->compMsgRequest->msgRequestInfos;
  msgRequestInfo = &msgRequestInfos->msgRequestInfo[uartRequestIdx];
  if (msgRequestInfo->requestType != COMP_MSG_INPUT_UART) {
    return COMP_MSG_ERR_UART_REQUEST_NOT_SET;
  }
  isComplete = false;
  compMsgData = msgRequestInfo->requestData;
  compMsgData->direction = COMP_MSG_RECEIVED_DATA;
  COMP_MSG_DBG(self, "R", 2, "call handleReceivePart: lgth: %d", lgth);
  self->compMsgData = compMsgData;
  result = self->compMsgRequest->detectMsg(self, data, lgth, uartRequestIdx, &isComplete);
  checkErrOK(result);
  if (isComplete) {
    result = self->compMsgIdentify->unpackReceivedMsg(self, msgRequestInfo);
    checkErrOK(result);
  }
  return COMP_MSG_ERR_OK;
}

// ================================= addRequest ====================================

static uint8_t addRequest(compMsgDispatcher_t *self, uint8_t requestType, void *requestHandle, compMsgData_t *requestData) {
  uint8_t result;
  bool isComplete;
  compMsgDispatcher_t *requestSelf;
  msgRequestInfos_t *msgRequestInfos;
  msgRequestInfo_t *msgRequestInfo;
  compMsgData_t *compMsgData;

  result = COMP_MSG_ERR_OK;
  isComplete = false;
  msgRequestInfos = &self->compMsgRequest->msgRequestInfos;
  msgRequestInfo = &msgRequestInfos->msgRequestInfo[msgRequestInfos->currRequestIdx];
  if (msgRequestInfos->lastRequestIdx >= COMP_MSG_MAX_REQUESTS) {
    COMP_MSG_DBG(self, "Y", 0, "COMP_MSG_ERR_TOO_MANY_REQUESTS");
    return COMP_MSG_ERR_TOO_MANY_REQUESTS;
  }
  msgRequestInfos->lastRequestIdx++;
  msgRequestInfo = &msgRequestInfos->msgRequestInfo[msgRequestInfos->lastRequestIdx];
  msgRequestInfo->requestType = requestType;
  msgRequestInfo->requestHandle = requestHandle;
  msgRequestInfo->requestData = requestData;
  COMP_MSG_DBG(self, "R", 2, "addRequest: lastRequestIdx: %d requestType: %d requestData: %p\n", msgRequestInfos->lastRequestIdx, requestType, requestData);
  COMP_MSG_DBG(self, "R", 2, "addRequest 2 %d requestData: %p\n", msgRequestInfos->currRequestIdx, requestData);
  requestSelf = msgRequestInfo->requestDispatcher;
//ets_printf("compMsgData: %p requestSelf: %p self: %p\n", compMsgData, requestSelf, self);
//FIXME TEMPORARY last 2 if clauses!!
  if ((msgRequestInfos->currRequestIdx < 1) 
      || (requestData->direction == COMP_MSG_TRANSFER_DATA)
      || (requestData->direction == COMP_MSG_TO_SEND_DATA)) {
    msgRequestInfos->currRequestIdx++;
//    checkErrOK(result);
    msgRequestInfo = &msgRequestInfos->msgRequestInfo[msgRequestInfos->currRequestIdx];
    compMsgData = msgRequestInfo->requestData;
    requestSelf->compMsgData = compMsgData;
    switch (compMsgData->direction) {
    case COMP_MSG_TO_SEND_DATA:
      COMP_MSG_DBG(self, "R", 2, "addRequest: toSendData: %p %d\n", compMsgData->toSendData, compMsgData->toSendLgth);
      result = self->compMsgIdentify->handleToSendPart(requestSelf, compMsgData->toSendData, compMsgData->toSendLgth);
      break;
    case COMP_MSG_RECEIVED_DATA:
      COMP_MSG_DBG(self, "R", 2, "addRequest: receivedData: %p %d\n", compMsgData->receivedData, compMsgData->receivedLgth);
      result = self->compMsgRequest->detectMsg(requestSelf, compMsgData->receivedData, compMsgData->receivedLgth, msgRequestInfos->currRequestIdx, &isComplete);
      checkErrOK(result);
      if (isComplete) {
        result = self->compMsgIdentify->unpackReceivedMsg(requestSelf, msgRequestInfo);
      }
//ets_printf("addRequest handleReceivedPart2: heap: %d\n", system_get_free_heap_size());
      break;
    case COMP_MSG_TRANSFER_DATA:
      result = self->compMsgSendReceive->sendMsg(requestSelf, compMsgData->receivedData, compMsgData->receivedLgth);
      break;
    default:
      COMP_MSG_DBG(self, "R", 2, "bad direction: 0x%02x 0x%02x\n", compMsgData->direction, requestData->direction);
      return COMP_MSG_ERR_BAD_VALUE;
    }
  } else {
    COMP_MSG_DBG(self, "R", 2, "direction: %d %d\n", requestData->direction, COMP_MSG_RECEIVED_DATA);
    msgRequestInfos->currRequestIdx = msgRequestInfos->lastRequestIdx;
    msgRequestInfo = &msgRequestInfos->msgRequestInfo[msgRequestInfos->currRequestIdx];
    compMsgData = msgRequestInfo->requestData;
    requestData = compMsgData;
    requestSelf->compMsgData = compMsgData;
    if (requestData->direction == COMP_MSG_RECEIVED_DATA) {
// FIXME TEMPORARY need flag to see if no uart activity!!
      switch (compMsgData->direction) {
      case COMP_MSG_TRANSFER_DATA:
        result = self->compMsgSendReceive->sendMsg(requestSelf, compMsgData->receivedData, compMsgData->receivedLgth);
        break;
      case COMP_MSG_TO_SEND_DATA:
        break;
      case COMP_MSG_RECEIVED_DATA:
        COMP_MSG_DBG(self, "R", 2, "COMP_MSG_RECEIVED_DATA: compMsgData: %p\n", compMsgData);
        COMP_MSG_DBG(self, "R", 2, "received: %p lgth: %d\n", compMsgData->receivedData, compMsgData->receivedLgth);
//        result = self->compMsgIdentify->handleReceivedPart(requestSelf, compMsgData->receivedData, compMsgData->receivedLgth);
        result = self->compMsgRequest->detectMsg(requestSelf, compMsgData->receivedData, compMsgData->receivedLgth, msgRequestInfos->currRequestIdx, &isComplete);
        checkErrOK(result);
        if (isComplete) {
          result = self->compMsgIdentify->unpackReceivedMsg(requestSelf, msgRequestInfo);
        }
        break;
      default:
        COMP_MSG_DBG(self, "Y", 0, "bad direction: 0x%02x 0x%02x\n", compMsgData->direction, requestData->direction);
        return COMP_MSG_ERR_BAD_VALUE;
      }
    }
  }
  checkErrOK(result);
  return result;
}

// ================================= deleteRequest ====================================

static uint8_t deleteRequest(compMsgDispatcher_t *self, uint8_t requestType, void *requestHandle) {
  uint8_t result;
  int idx;
  bool found;
  int idxToStart;
  int idxDeleted;
  msgRequestInfos_t *msgRequestInfos;
  msgRequestInfo_t *msgRequestInfo;
  msgRequestInfo_t *msgRequestInfo1;

  idx = 0;
  idxToStart = -1;
  idxDeleted = -1;
  found = false;
  msgRequestInfos = &self->compMsgRequest->msgRequestInfos;
  while (idx < self->compMsgRequest->msgRequestInfos.lastRequestIdx) {
    if (idx >= COMP_MSG_MAX_REQUESTS) {
      return COMP_MSG_ERR_REQUEST_NOT_FOUND;
    }
    if (!found) {
      msgRequestInfo = &msgRequestInfos->msgRequestInfo[idx];
      if ((msgRequestInfo->requestType == requestType) && (msgRequestInfo->requestHandle == requestHandle)) {
        found = true;
        idxDeleted = idx;
        if (idx == msgRequestInfos->currRequestIdx) {
          if (idx < msgRequestInfos->lastRequestIdx) {
            idxToStart = idx;
          }
          msgRequestInfos->currRequestIdx = -1;
        }
      }
    } else {
      // move the following entries one idx down
      if (idx < msgRequestInfos->lastRequestIdx) {
        msgRequestInfo = &msgRequestInfos->msgRequestInfo[idx];
        msgRequestInfo1 = &msgRequestInfos->msgRequestInfo[idx + 1];
        msgRequestInfo->requestType = msgRequestInfo1->requestType;
        msgRequestInfo->requestHandle = msgRequestInfo1->requestHandle;
        if (idx + 1 == msgRequestInfos->currRequestIdx) {
          msgRequestInfos->currRequestIdx--;
        }
      }
    }
    idx++;
  }
  if (msgRequestInfos->lastRequestIdx >= 0) {
    msgRequestInfos->lastRequestIdx--;
  }
  if (msgRequestInfos->currRequestIdx < 0) {
    msgRequestInfos->currRequestIdx++;
    // start handling the request
    self->compMsgRequest->startNextRequest(self);
  } else {
    // nothing to do the current request is different from the deleted one
    // so just let the current one continue
  }
  return COMP_MSG_ERR_OK;
}

// ================================= detectMessage ====================================

/**
 * For the uart detectMessage collects the input characters.
 *  - if we have headerLgth characters scan the header fields to get the totalLgth
 *  - with totalLgth known collect the characters until we have totalLgth characters.
 *  - if status signals that we can handle a uart request, handle the request
 *  - if status signals we cannot handle that request right now, store the request.
 *    There are maximal COMP_MSG_MAX_UART_REQUESTS stored if there are more, we delete the oldest 
 *    uart request. Eventually we already delete the first request of the same type, 
 *    if we have already a request with the same type. ???
 *  - we have to define "interrupt" points, where uart requests can be handled
 *    for example during provisioning mode etc.
 *
 * For other requests detectMessage collects the input characters.too
 *  - normally all characters of a message are handled at once
 *  - we check if we can handle that request (uart requests have eventually higher priority)
 *  - if we can handle requests right now, we handle the request in the order they came in
 *  - if the request queue is full (COMP_MSG_MAX_REQUESTS)
 *    new request from the app are ignored in receiving them already
 *    see MAX_CONNECTIONS_ALLOWED in the *Socket* modules
 */

/**
 * \brief handle input characters from uart or sockets collect them until complete message received
 * \param self The dispatcher struct
 * \param buffer The input characters
 * \param lgth The number of characters in the input
 * \param queueIdx Which requestQueue tu use
 * \param isComplete Signal if message is complete
 * \return Error code or COMP_MSG_ERR_OK
 *
 */
static uint8_t detectMsg(compMsgDispatcher_t *self, const uint8_t * buffer, int lgth, int queueIdx, bool *isComplete) {
  int idx;
  msgHeaderInfos_t *hdrInfos;
  headerPart_t *hdr;
  int startIdx;
  int hdrIdx;
  msgRequestInfos_t *msgRequestInfos;
  msgRequestInfo_t *msgRequestInfo;
  compMsgData_t *compMsgData;
  msgParts_t *received;
  int result;

if (buffer == NULL) {
  COMP_MSG_DBG(self, "I", 2, "++++detectMessage: buffer == NULL lgth: %d queueIdx: %d isComplete: %d", lgth, queueIdx, *isComplete);
} else {
  COMP_MSG_DBG(self, "I", 2, "++++detectMessage: 0x%02x lgth: %d queueIdx: %d isComplete: %d", buffer[0], lgth, queueIdx, *isComplete);
}
  msgRequestInfos = &self->compMsgRequest->msgRequestInfos;
  msgRequestInfo = &msgRequestInfos->msgRequestInfo[queueIdx];
  hdrInfos = &self->dispatcherCommon->msgHeaderInfos;
  received = &msgRequestInfo->received;
//FIXME need to free at end of message handling !!!
  COMP_MSG_DBG(self, "I", 2, "detectMessage: !received->buf: %p!", received->buf);
  COMP_MSG_DBG(self, "I", 2, "receivedLgth: %d lgth: %d headerLgth: %d!", received->lgth, lgth, hdrInfos->headerLgth);
  idx = 0;
  while (idx < lgth) {
    received->buf[received->lgth++] = buffer[idx];
    if (received->lgth == hdrInfos->headerLgth) {
      COMP_MSG_DBG(self, "I", 2, "received lgth: %d lgth: %d idx: %d", received->lgth, lgth, idx);
//compMsgData->compMsgDataView->dataView->dumpBinary(received->buf, received->lgth, "received->buf");
      COMP_MSG_DBG(self, "I", 2, "msgRequestInfo->requestBufferIdx: %d", received->lgth);
      result = self->compMsgIdentify->getHeaderIndexFromHeaderFields(self, received);
      COMP_MSG_DBG(self, "I", 2, "getHeaderIndexFromHeaderFields result: %d currPartIdx: %d", result, hdrInfos->currPartIdx);
      checkErrOK(result);
    }
    // loop until we have full message then decrypt if necessary and then handle the message
    COMP_MSG_DBG(self, "I", 2, "totalLgth: %d idx: %d lgth: %d", received->lgth, idx, lgth);
    if ((received->lgth == received->totalLgth) && (received->totalLgth > 0)) {
      *isComplete = true;
      return COMP_MSG_ERR_OK;
    }
    idx++;
  }
  return COMP_MSG_ERR_OK;
}

extern void dbgPrintf(void *self, uint8_t *dbgChars, uint8_t debugLevel, uint8_t *format, ...);

// ================================= compMsgRequestInit ====================================

uint8_t compMsgRequestInit(compMsgDispatcher_t *self) {
  uint8_t result;
  int idx;
  msgRequestInfos_t *msgRequestInfos;
  msgRequestInfo_t *msgRequestInfo;

  msgRequestInfos = &self->compMsgRequest->msgRequestInfos;
  msgRequestInfos->currRequestIdx = -1;
  msgRequestInfos->lastRequestIdx = -1;
dbgPrintf(NULL, "Y", 0, "request1 heap: %d", system_get_free_heap_size());
  idx = 0;
  msgRequestInfo = &msgRequestInfos->msgRequestInfo[idx];
  msgRequestInfo->requestDispatcher = self;
  idx++;
  while (idx < COMP_MSG_MAX_REQUESTS) {
    msgRequestInfo = &msgRequestInfos->msgRequestInfo[idx];
    msgRequestInfo->requestDispatcher = newCompMsgDispatcher();
    checkAllocOK(msgRequestInfo->requestDispatcher);
    idx++;
  }
dbgPrintf(NULL, "Y", 0, "request2 heap: %d", system_get_free_heap_size());

  // request handling
  self->compMsgRequest->startRequest = &startRequest;
  self->compMsgRequest->startNextRequest = &startNextRequest;
  self->compMsgRequest->addRequest = &addRequest;
  self->compMsgRequest->addUartRequestData = &addUartRequestData;
  self->compMsgRequest->deleteRequest = &deleteRequest;
  self->compMsgRequest->detectMsg = &detectMsg;
  return COMP_MSG_ERR_OK;
}

// ================================= newCompMsgRequest ====================================

compMsgRequest_t *newCompMsgRequest() {
  compMsgRequest_t *compMsgRequest = os_zalloc(sizeof(compMsgRequest_t));
  if (compMsgRequest == NULL) {
    return NULL;
  }

  return compMsgRequest;
}


