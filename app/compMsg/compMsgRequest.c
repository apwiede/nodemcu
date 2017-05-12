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

// ================================= uartTimerResetRequest ====================================

static uint8_t uartTimerResetRequest(void *arg) {
  uint8_t result;
  compMsgDispatcher_t *self;
  int timerId;
  compMsgTimerSlot_t *compMsgTimerSlot;
  compMsgTimerSlot_t *tmr;

ets_printf("uartTimerResetRequest\n");
  compMsgTimerSlot = (compMsgTimerSlot_t *)arg;
  self = compMsgTimerSlot->compMsgDispatcher;
  COMP_MSG_DBG(self, "s", 1, "timerResetRequest");
  timerId = compMsgTimerSlot->timerId;
  self = compMsgTimerSlot->compMsgDispatcher;
  COMP_MSG_DBG(self, "S", 2, "timerResetRequest timerId: %d\n", timerId);
  tmr = &self->compMsgTimer->compMsgTimers[timerId];
  result = self->compMsgRequest->resetRequest(self, 0);
  return COMP_MSG_ERR_OK;
}

// ================================= resetRequest ====================================

static uint8_t resetRequest(compMsgDispatcher_t *self, int slot) {
  uint8_t result;
  msgRequestInfos_t *msgRequestInfos;
  msgRequestInfo_t *msgRequestInfo;

  COMP_MSG_DBG(self, "s", 1, "resetRequest slot: %d", slot);
  msgRequestInfos = &self->compMsgRequest->msgRequestInfos;
  msgRequestInfo = &msgRequestInfos->msgRequestInfo[slot];
  if (msgRequestInfo->data != NULL) {
    os_free(msgRequestInfo->data);
  }
  c_memset(msgRequestInfo, 0, sizeof(msgRequestInfo_t));
  return COMP_MSG_ERR_OK;
}

// ================================= startRequest ====================================

static uint8_t startRequest(compMsgDispatcher_t *self) {
  uint8_t result;
  compMsgData_t *compMsgData;
  compMsgData_t *requestData;
  msgRequestInfo_t *msgRequestInfo;

  result = COMP_MSG_ERR_OK;
  COMP_MSG_DBG(self, "R", 1, "should start request: %d\n", self->compMsgRequest->msgRequestInfos.currRequestIdx);
  msgRequestInfo = &self->compMsgRequest->msgRequestInfos.msgRequestInfo[self->compMsgRequest->msgRequestInfos.currRequestIdx];
//FIXME
#ifdef NOTDEF
  compMsgData = msgRequestInfo->requestData;
#endif
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

// ================================= addRequestData ====================================

static uint8_t addRequestData(compMsgDispatcher_t *self, uint8_t requestType, socketUserData_t *sud, uint8_t *data, uint16_t lgth) {
  uint8_t result;
  compMsgDispatcher_t *requestSelf;
  msgRequestInfos_t *msgRequestInfos;
  msgRequestInfo_t *msgRequestInfo;
  int requestIdx;

  result = COMP_MSG_ERR_OK;
  msgRequestInfos = &self->compMsgRequest->msgRequestInfos;
  requestIdx = 0;
  while (requestIdx < COMP_MSG_MAX_REQUESTS) {
    msgRequestInfo = &msgRequestInfos->msgRequestInfo[requestIdx];
    if (msgRequestInfo->sud == sud) {
      break;
    }
    if (((msgRequestInfo->sud == NULL) && (msgRequestInfo->requestType == 0)) ||
        ((requestType == COMP_MSG_INPUT_UART) && (lgth == 0))) {
      // the second if clause is used in compMsgRequestInit for uart!!
      msgRequestInfo->requestType = requestType;
      msgRequestInfo->sud = sud;
      msgRequestInfo->data = os_zalloc(lgth);
      checkAllocOK(msgRequestInfo->data);
      msgRequestInfo->lgth = lgth;
      msgRequestInfo->totalLgth = 999;  // just a dummy value until we have the real totalLgth
                                        // (after reading header)
      break;
    }
    requestIdx++;
  }
  if (requestIdx >= COMP_MSG_MAX_REQUESTS) {
    COMP_MSG_DBG(self, "Y", 0, "COMP_MSG_ERR_TOO_MANY_REQUESTS");
    return COMP_MSG_ERR_TOO_MANY_REQUESTS;
  }
  requestSelf = msgRequestInfo->requestDispatcher;
  result = self->compMsgRequest->detectMsg(requestSelf, data, lgth, requestIdx, &msgRequestInfo->isComplete);
  checkErrOK(result);
  if (msgRequestInfo->isComplete) {
    result = self->compMsgIdentify->unpackReceivedMsg(requestSelf, msgRequestInfo);
    checkErrOK(result);
  }
  return result;
}

// ================================= deleteRequest ====================================

static uint8_t deleteRequest(compMsgDispatcher_t *self, uint8_t requestType, socketUserData_t *sud) {
  uint8_t result;
  int requestIdx;
  bool found;
  int idxToStart;
  int idxDeleted;
  msgRequestInfos_t *msgRequestInfos;
  msgRequestInfo_t *msgRequestInfo;
  msgRequestInfo_t *msgRequestInfo1;

  requestIdx = 0;
  idxToStart = -1;
  idxDeleted = -1;
  found = false;
  msgRequestInfos = &self->compMsgRequest->msgRequestInfos;
  while (requestIdx < COMP_MSG_MAX_REQUESTS) {
    msgRequestInfo = &msgRequestInfos->msgRequestInfo[requestIdx];
    if ((msgRequestInfo->requestType == requestType) && (msgRequestInfo->sud == sud)) {
      idxDeleted = requestIdx;
      if (requestIdx == msgRequestInfos->currRequestIdx) {
        if (requestIdx < msgRequestInfos->lastRequestIdx) {
          idxToStart = requestIdx;
        }
      }
    }
    requestIdx++;
  }
  // move the following entries one idx down
  if (requestIdx < COMP_MSG_MAX_REQUESTS) {
    msgRequestInfo = &msgRequestInfos->msgRequestInfo[requestIdx];
    msgRequestInfo1 = &msgRequestInfos->msgRequestInfo[requestIdx + 1];
    msgRequestInfo->requestType = msgRequestInfo1->requestType;
    msgRequestInfo->sud = msgRequestInfo1->sud;
  }
  if (requestIdx >= COMP_MSG_MAX_REQUESTS) {
    return COMP_MSG_ERR_REQUEST_NOT_FOUND;
  }
  self->compMsgRequest->startNextRequest(self);
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
static uint8_t detectMsg(compMsgDispatcher_t *self, const uint8_t *buffer, int lgth, int queueIdx, bool *isComplete) {
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
//FIXME
#ifdef NOTDEF
  received = &msgRequestInfo->received;
#endif
//FIXME need to free at end of message handling !!!
  COMP_MSG_DBG(self, "I", 2, "detectMessage: !msgRequestInfo->data: %p!", msgRequestInfo->data);
  COMP_MSG_DBG(self, "I", 2, "msgRequestInfo->lgth: %d lgth: %d headerLgth: %d!", msgRequestInfo->lgth, lgth, hdrInfos->headerLgth);
  // check for msgRequestInfo->data has enough free space
  if (msgRequestInfo->currLgth < lgth) {
    if (msgRequestInfo->requestType == COMP_MSG_INPUT_UART) {
      if (msgRequestInfo->currLgth == 0) {
        msgRequestInfo->currLgth = 255; // make uart input fast
      } else {
        msgRequestInfo->currLgth += 10;
      }
    } else {
      msgRequestInfo->currLgth += lgth;
    }
    if (msgRequestInfo->data == NULL) {
      msgRequestInfo->data = os_zalloc(msgRequestInfo->currLgth);
      checkAllocOK(msgRequestInfo->data);
    } else {
      msgRequestInfo->data = os_realloc(msgRequestInfo->data, msgRequestInfo->currLgth);
      checkAllocOK(msgRequestInfo->data);
    }
  }
  idx = 0;
  while (idx < lgth) {
    msgRequestInfo->data[msgRequestInfo->currIdx++] = buffer[idx];
    if (msgRequestInfo->currIdx == hdrInfos->headerLgth) {
      COMP_MSG_DBG(self, "I", 2, "msgRequestInfo->currIdx: %d lgth: %d idx: %d", msgRequestInfo->currIdx, lgth, idx);
//compMsgData->compMsgDataView->dataView->dumpBinary(msgRequestInfo->data, lgth, "received data");
      COMP_MSG_DBG(self, "I", 2, "msgRequestInfo->currIdx: %d", msgRequestInfo->currIdx);
      result = self->compMsgIdentify->getHeaderIndexFromHeaderFields(self, received);
      COMP_MSG_DBG(self, "I", 2, "getHeaderIndexFromHeaderFields result: %d currPartIdx: %d", result, hdrInfos->currPartIdx);
      checkErrOK(result);
    }
    // loop until we have full message then decrypt if necessary and then handle the message
    COMP_MSG_DBG(self, "I", 2, "totalLgth: %d idx: %d lgth: %d", msgRequestInfo->currIdx, idx, lgth);
    if ((msgRequestInfo->currIdx == msgRequestInfo->totalLgth) && (msgRequestInfo->totalLgth > 0)) {
      *isComplete = true;
      return COMP_MSG_ERR_OK;
    }
    idx++;
  }
  return COMP_MSG_ERR_OK;
}

// ================================= compMsgRequestInit ====================================

static uint8_t compMsgRequestInit(compMsgDispatcher_t *self) {
  uint8_t result;
  int idx;
  compMsgRequest_t *compMsgRequest;
  msgRequestInfos_t *msgRequestInfos;
  msgRequestInfo_t *msgRequestInfo;

  msgRequestInfos = &self->compMsgRequest->msgRequestInfos;
  msgRequestInfos->currRequestIdx = -1;
  msgRequestInfos->lastRequestIdx = -1;
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

  compMsgRequest = self->compMsgRequest;
  compMsgRequest->uartTimerResetRequest = &uartTimerResetRequest;
  compMsgRequest->resetRequest = &resetRequest;
  compMsgRequest->startRequest = &startRequest;
  compMsgRequest->startNextRequest = &startNextRequest;
  compMsgRequest->addRequestData = &addRequestData;
  compMsgRequest->deleteRequest = &deleteRequest;
  compMsgRequest->detectMsg = &detectMsg;
  return COMP_MSG_ERR_OK;
}

// ================================= newCompMsgRequest ====================================

compMsgRequest_t *newCompMsgRequest() {
  compMsgRequest_t *compMsgRequest;

  compMsgRequest = os_zalloc(sizeof(compMsgRequest_t));
  if (compMsgRequest == NULL) {
    return NULL;
  }
  compMsgRequest->compMsgRequestInit = &compMsgRequestInit;
  return compMsgRequest;
}
