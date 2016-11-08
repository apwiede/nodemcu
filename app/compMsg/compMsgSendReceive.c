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
 * File:   compMsgSendReceive.c
 * Author: Arnulf P. Wiedemann
 *
 * Created on October 7st, 2016
 */

#include "osapi.h"
#include "c_types.h"
#include "mem.h"
#include "flash_fs.h"

#include "c_string.h"
#include "c_stdlib.h"
#include "c_stdio.h"
#include "platform.h"
#include "compMsgDispatcher.h"

// ================================= uartReceiveCb ====================================

static uint8_t uartReceiveCb(compMsgDispatcher_t *self, const uint8_t *buffer, uint8_t lgth) {
  int result;
  int idx;
  msgParts_t *received;
  const uint8_t buf[1] = { 0};
  const uint8_t *myBuffer;

  received = &self->received;
//ets_printf("§%c§", buffer[0]&0xFF);
  myBuffer = buffer;
  if (lgth == 0) {
    // simulate a '0' char!!
    lgth = 1;
    myBuffer = buf;
  }
  result =self->addUartRequestData(self, (uint8_t *)myBuffer, lgth);
  checkErrOK(result);
  return COMP_DISP_ERR_OK;
}

// ================================= typeRSendAnswer ====================================

static uint8_t typeRSendAnswer(compMsgDispatcher_t *self, uint8_t *data, uint8_t msgLgth) {
  int result;
  int idx;

  idx = 0;
  while (idx < msgLgth) {
    platform_uart_send(0, data[idx]);
    idx++;
  }
  return COMP_DISP_ERR_OK;
}

// ================================= sendCloudMsg ====================================

static uint8_t sendCloudMsg(compMsgDispatcher_t *self, uint8_t *msgData, size_t msgLgth) {
  uint8_t result;

  if (self->compMsgData->nud == NULL) {
    return COMP_DISP_ERR_NO_WEBSOCKET_OPENED;
  }
ets_printf("sendCloudData: msgLgth: %d\n", msgLgth);
  result = self->netsocketSendData(self->compMsgData->nud, msgData, msgLgth);
  checkErrOK(result);
}

// ================================= sendMsg ====================================

static uint8_t sendMsg(compMsgDispatcher_t *self, uint8_t *msgData, size_t msgLgth) {
  uint8_t result;

  switch (self->compMsgData->currHdr->hdrHandleType) {
  case 'A':
ets_printf("wud: %p\n", self->compMsgData->wud);
    if (self->compMsgData->wud == NULL) {
      return COMP_DISP_ERR_NO_WEBSOCKET_OPENED;
    }
ets_printf("remote_ip: %d %d %d %d port: %d\n", self->compMsgData->wud->remote_ip[0], self->compMsgData->wud->remote_ip[1], self->compMsgData->wud->remote_ip[2], self->compMsgData->wud->remote_ip[3], self->compMsgData->wud->remote_port);
    result = self->websocketSendData(self->compMsgData->wud, msgData, msgLgth, OPCODE_BINARY);
    checkErrOK(result);
    break;
  case 'G':
   break;
  case 'S':
   break;
  case 'R':
   break;
  case 'U':
   break;
  case 'W':
   break;
  case 'N':
    // just ignore
    return COMP_DISP_ERR_OK;
    break;
  default:
    return COMP_DISP_ERR_BAD_HANDLE_TYPE;
  }
  return COMP_DISP_ERR_OK;
}

// ================================= compMsgSendReceiveInit ====================================

uint8_t compMsgSendReceiveInit(compMsgDispatcher_t *self) {
  uint8_t result;

  result = self->getNewCompMsgDataPtr(self);
  self->compMsgData->wud = NULL;
  self->compMsgData->nud = NULL;
  self->compMsgData->receivedData = NULL;
  self->compMsgData->receivedLgth = 0;
  self->compMsgData->direction = COMP_MSG_RECEIVED_DATA;
  result = self->addRequest(self, COMP_DISP_INPUT_UART, NULL, self->compMsgData);

  self->uartReceiveCb = &uartReceiveCb;
  self->typeRSendAnswer = &typeRSendAnswer;
  self->sendCloudMsg = &sendCloudMsg;
  self->sendMsg = &sendMsg;
  return COMP_DISP_ERR_OK;
}

