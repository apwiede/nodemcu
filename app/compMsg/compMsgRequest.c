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

  COMP_MSG_DBG(self, "R", 1, "should start request: %d\n", self->compMsgRequest->msgRequestInfos.currRequestIdx);
  return COMP_MSG_ERR_OK;
}

// ================================= startNextRequest ====================================

static uint8_t startNextRequest(compMsgDispatcher_t *self) {
  uint8_t result;

  if (self->compMsgRequest->msgRequestInfos.currRequestIdx < 0) {
    if (self->compMsgRequest->msgRequestInfos.currRequestIdx < self->compMsgRequest->msgRequestInfos.lastRequestIdx) {
      self->compMsgRequest->msgRequestInfos.currRequestIdx = 0;
      result = startRequest(self);
      checkErrOK(result);
      return COMP_MSG_ERR_OK;
    }
  }
  if (self->compMsgRequest->msgRequestInfos.currRequestIdx < self->compMsgRequest->msgRequestInfos.lastRequestIdx) {
    self->compMsgRequest->msgRequestInfos.currRequestIdx++;
    result = startRequest(self);
    checkErrOK(result);
  }
  return COMP_MSG_ERR_OK;
}

// ================================= addUartRequestData ====================================

static uint8_t addUartRequestData(compMsgDispatcher_t *self, uint8_t *data, size_t lgth) {
  uint8_t result;
  compMsgData_t *compMsgData;

  // slot 0 is reserved for Uart
  if (self->compMsgRequest->msgRequestInfos.requestTypes[0] != COMP_DISP_INPUT_UART) {
    return COMP_MSG_ERR_UART_REQUEST_NOT_SET;
  }
  compMsgData = self->compMsgRequest->msgRequestInfos.requestData[0];
  compMsgData->direction = COMP_MSG_RECEIVED_DATA;
  COMP_MSG_DBG(self, "R", 2, "call handleReceivePart: lgth: %d", lgth);
  self->compMsgData = compMsgData;
  result = self->compMsgIdentify->handleReceivedPart(self, data, lgth);
  checkErrOK(result);
  return COMP_MSG_ERR_OK;
}

// ================================= addRequest ====================================

static uint8_t addRequest(compMsgDispatcher_t *self, uint8_t requestType, void *requestHandle, compMsgData_t *requestData) {
  uint8_t result;
  compMsgData_t *compMsgData;

  result = COMP_MSG_ERR_OK;
  if (self->compMsgRequest->msgRequestInfos.lastRequestIdx >= COMP_DISP_MAX_REQUESTS) {
    COMP_MSG_DBG(self, "Y", 0, "COMP_MSG_ERR_TOO_MANY_REQUESTS");
    return COMP_MSG_ERR_TOO_MANY_REQUESTS;
  }
  self->compMsgRequest->msgRequestInfos.lastRequestIdx++;
  self->compMsgRequest->msgRequestInfos.requestTypes[self->compMsgRequest->msgRequestInfos.lastRequestIdx] = requestType;
  self->compMsgRequest->msgRequestInfos.requestHandles[self->compMsgRequest->msgRequestInfos.lastRequestIdx] = requestHandle;
  self->compMsgRequest->msgRequestInfos.requestData[self->compMsgRequest->msgRequestInfos.lastRequestIdx] = requestData;
  COMP_MSG_DBG(self, "R", 2, "addRequest: lastRequestIdx: %d requestType: %d compMsgData: %p\n", self->compMsgRequest->msgRequestInfos.lastRequestIdx, requestType, requestData);
  COMP_MSG_DBG(self, "R", 2, "addRequest 2 %d %p\n", self->compMsgRequest->msgRequestInfos.currRequestIdx, requestData);
//FIXME TEMPORARY last 2 if clauses!!
  if ((self->compMsgRequest->msgRequestInfos.currRequestIdx < 1) 
      || (requestData->direction == COMP_MSG_TRANSFER_DATA)
      || (requestData->direction == COMP_MSG_TO_SEND_DATA)) {
    self->compMsgRequest->msgRequestInfos.currRequestIdx++;
    checkErrOK(result);
    compMsgData = self->compMsgRequest->msgRequestInfos.requestData[self->compMsgRequest->msgRequestInfos.currRequestIdx];
    switch (compMsgData->direction) {
    case COMP_MSG_TO_SEND_DATA:
      COMP_MSG_DBG(self, "R", 2, "addRequest: toSendData: %p %d\n", compMsgData->toSendData, compMsgData->toSendLgth);
      result = self->compMsgIdentify->handleToSendPart(self, compMsgData->toSendData, compMsgData->toSendLgth);
      break;
    case COMP_MSG_RECEIVED_DATA:
      COMP_MSG_DBG(self, "R", 2, "addRequest: receivedData: %p %d\n", compMsgData->receivedData, compMsgData->receivedLgth);
      result = self->compMsgIdentify->handleReceivedPart(self, compMsgData->receivedData, compMsgData->receivedLgth);
      break;
    case COMP_MSG_TRANSFER_DATA:
      result = self->compMsgSendReceive->sendMsg(self, compMsgData->receivedData, compMsgData->receivedLgth);
      break;
    default:
      COMP_MSG_DBG(self, "R", 2, "bad direction: 0x%02x 0x%02x\n", compMsgData->direction, requestData->direction);
      return COMP_MSG_ERR_BAD_VALUE;
    }
  } else {
      COMP_MSG_DBG(self, "R", 2, "direction: %d %d\n", requestData->direction, COMP_MSG_RECEIVED_DATA);
    self->compMsgRequest->msgRequestInfos.currRequestIdx = self->compMsgRequest->msgRequestInfos.lastRequestIdx;
    compMsgData = self->compMsgRequest->msgRequestInfos.requestData[self->compMsgRequest->msgRequestInfos.currRequestIdx];
    requestData = compMsgData;
    if (requestData->direction == COMP_MSG_RECEIVED_DATA) {
// FIXME TEMPORARY need flag to see if no uart activity!!
      switch (compMsgData->direction) {
      case COMP_MSG_TRANSFER_DATA:
        result = self->compMsgSendReceive->sendMsg(self, compMsgData->receivedData, compMsgData->receivedLgth);
        break;
      case COMP_MSG_TO_SEND_DATA:
        break;
      case COMP_MSG_RECEIVED_DATA:
        COMP_MSG_DBG(self, "R", 2, "COMP_MSG_RECEIVED_DATA: compMsgData: %p\n", compMsgData);
        COMP_MSG_DBG(self, "R", 2, "received: %p lgth: %d\n", compMsgData->receivedData, compMsgData->receivedLgth);
        result = self->compMsgIdentify->handleReceivedPart(self, compMsgData->receivedData, compMsgData->receivedLgth);
        break;
      default:
        COMP_MSG_DBG(self, "Y", 0, "bad direction: 0x%02x 0x%02x\n", compMsgData->direction, requestData->direction);
        return COMP_MSG_ERR_BAD_VALUE;
      }
    }
  }
  return result;
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
  while (idx < self->compMsgRequest->msgRequestInfos.lastRequestIdx) {
    if (idx >= COMP_DISP_MAX_REQUESTS) {
      return COMP_MSG_ERR_REQUEST_NOT_FOUND;
    }
    if (!found) {
      if ((self->compMsgRequest->msgRequestInfos.requestTypes[idx] == requestType) && (self->compMsgRequest->msgRequestInfos.requestHandles[idx] == requestHandle)) {
        found = true;
        idxDeleted = idx;
        if (idx == self->compMsgRequest->msgRequestInfos.currRequestIdx) {
          if (idx < self->compMsgRequest->msgRequestInfos.lastRequestIdx) {
            idxToStart = idx;
          }
          self->compMsgRequest->msgRequestInfos.currRequestIdx = -1;
        }
      }
    } else {
      // move the following entries one idx down
      if (idx < self->compMsgRequest->msgRequestInfos.lastRequestIdx) {
        self->compMsgRequest->msgRequestInfos.requestTypes[idx] = self->compMsgRequest->msgRequestInfos.requestTypes[idx + 1];
        self->compMsgRequest->msgRequestInfos.requestHandles[idx] = self->compMsgRequest->msgRequestInfos.requestHandles[idx + 1];
        if (idx + 1 == self->compMsgRequest->msgRequestInfos.currRequestIdx) {
          self->compMsgRequest->msgRequestInfos.currRequestIdx--;
        }
      }
    }
    idx++;
  }
  if (self->compMsgRequest->msgRequestInfos.lastRequestIdx >= 0) {
    self->compMsgRequest->msgRequestInfos.lastRequestIdx--;
  }
  if (self->compMsgRequest->msgRequestInfos.currRequestIdx < 0) {
    self->compMsgRequest->msgRequestInfos.currRequestIdx++;
    // start handling the request
    self->compMsgRequest->startNextRequest(self);
  } else {
    // nothing to do the current request is different from the deleted one
    // so just let the current one continue
  }
  return COMP_MSG_ERR_OK;
}

// ================================= compMsgRequestInit ====================================

uint8_t compMsgRequestInit(compMsgDispatcher_t *self) {
  uint8_t result;

  // request handling
  self->compMsgRequest->msgRequestInfos.currRequestIdx = -1;
  self->compMsgRequest->msgRequestInfos.lastRequestIdx = -1;

  // request handling
  self->compMsgRequest->startRequest = &startRequest;
  self->compMsgRequest->startNextRequest = &startNextRequest;
  self->compMsgRequest->addRequest = &addRequest;
  self->compMsgRequest->addUartRequestData = &addUartRequestData;
  self->compMsgRequest->deleteRequest = &deleteRequest;
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


