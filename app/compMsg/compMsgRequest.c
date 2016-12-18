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

ets_printf("should start request: %d\n", self->msgRequestInfos.currRequestIdx);
  return COMP_MSG_ERR_OK;
}

// ================================= startNextRequest ====================================

static uint8_t startNextRequest(compMsgDispatcher_t *self) {
  uint8_t result;

  if (self->msgRequestInfos.currRequestIdx < 0) {
    if (self->msgRequestInfos.currRequestIdx < self->msgRequestInfos.lastRequestIdx) {
      self->msgRequestInfos.currRequestIdx = 0;
      result = startRequest(self);
      checkErrOK(result);
      return COMP_MSG_ERR_OK;
    }
  }
  if (self->msgRequestInfos.currRequestIdx < self->msgRequestInfos.lastRequestIdx) {
    self->msgRequestInfos.currRequestIdx++;
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
  if (self->msgRequestInfos.requestTypes[0] != COMP_DISP_INPUT_UART) {
    return COMP_MSG_ERR_UART_REQUEST_NOT_SET;
  }
  compMsgData = self->msgRequestInfos.requestData[0];
  compMsgData->direction = COMP_MSG_RECEIVED_DATA;
//ets_printf("§call handleReceivePart: lgth: %d§", lgth);
  self->compMsgData = compMsgData;
  result = self->compMsgIdentify->handleReceivedPart(self, data, lgth);
  checkErrOK(result);
  return COMP_MSG_ERR_OK;
}

// ================================= addRequest ====================================

static uint8_t addRequest(compMsgDispatcher_t *self, uint8_t requestType, void *requestHandle, compMsgData_t *requestData) {
  uint8_t result;
  compMsgData_t *compMsgData;

  if (self->msgRequestInfos.lastRequestIdx >= COMP_DISP_MAX_REQUESTS) {
ets_printf("§COMP_MSG_ERR_TOO_MANY_REQUESTS§");
    return COMP_MSG_ERR_TOO_MANY_REQUESTS;
  }
  self->msgRequestInfos.lastRequestIdx++;
  self->msgRequestInfos.requestTypes[self->msgRequestInfos.lastRequestIdx] = requestType;
  self->msgRequestInfos.requestHandles[self->msgRequestInfos.lastRequestIdx] = requestHandle;
  self->msgRequestInfos.requestData[self->msgRequestInfos.lastRequestIdx] = requestData;
//ets_printf("addRequest: lastRequestIdx: %d requestType: %d compMsgData: %p\n", self->msgRequestInfos.lastRequestIdx, requestType, requestData);
//FIXME TEMPORARY last if clause!!
  if ((self->msgRequestInfos.currRequestIdx < 1) || (requestData->direction == COMP_MSG_TO_SEND_DATA)) {
    self->msgRequestInfos.currRequestIdx++;
    checkErrOK(result);
    compMsgData = self->msgRequestInfos.requestData[self->msgRequestInfos.currRequestIdx];
    switch (compMsgData->direction) {
    case COMP_MSG_TO_SEND_DATA:
//ets_printf("addRequest: toSendData: %p\n", compMsgData->toSendData);
      result = self->compMsgIdentify->handleToSendPart(self, compMsgData->toSendData, compMsgData->toSendLgth);
      break;
    case COMP_MSG_RECEIVED_DATA:
//ets_printf("addRequest: receivedData: %p\n", compMsgData->receivedData);
      result = self->compMsgIdentify->handleReceivedPart(self, compMsgData->receivedData, compMsgData->receivedLgth);
      break;
    default:
ets_printf("bad direction: 0x%02x 0x%02x\n", compMsgData->direction, requestData->direction);
      return COMP_MSG_ERR_BAD_VALUE;
    }
  } else {
//ets_printf("direction: %d %d\n", requestData->direction, COMP_MSG_RECEIVED_DATA);
    self->msgRequestInfos.currRequestIdx = self->msgRequestInfos.lastRequestIdx;
    compMsgData = self->msgRequestInfos.requestData[self->msgRequestInfos.currRequestIdx];
    requestData = compMsgData;
    if (requestData->direction == COMP_MSG_RECEIVED_DATA) {
// FIXME TEMPORARY need flag to see if no uart activity!!
      switch (compMsgData->direction) {
      case COMP_MSG_TO_SEND_DATA:
        break;
      case COMP_MSG_RECEIVED_DATA:
//ets_printf("COMP_MSG_RECEIVED_DATA: compMsgData: %p\n", compMsgData);
//ets_printf("received: %p lgth: %d\n", compMsgData->receivedData, compMsgData->receivedLgth);
        result = self->compMsgIdentify->handleReceivedPart(self, compMsgData->receivedData, compMsgData->receivedLgth);
        break;
      default:
ets_printf("bad direction: 0x%02x 0x%02x\n", compMsgData->direction, requestData->direction);
        return COMP_MSG_ERR_BAD_VALUE;
      }
    }
  }
  return COMP_MSG_ERR_OK;
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
      return COMP_MSG_ERR_REQUEST_NOT_FOUND;
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


