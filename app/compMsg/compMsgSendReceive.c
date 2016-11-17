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

// ================================= uartSetup ====================================

static uint8_t uartSetup(compMsgDispatcher_t *self, unsigned id, uint32_t baud, int databits, int parity, int stopbits) {
  int result;

  result = platform_uart_setup(id, baud, databits, parity, stopbits);
ets_printf("§uartSetup:id: %d baud: %d§", id, baud);
  checkErrOK(result);
  return COMP_DISP_ERR_OK;
}

// ================================= uartReceiveCb ====================================

static uint8_t uartReceiveCb(compMsgDispatcher_t *self, const uint8_t *buffer, uint8_t lgth) {
  int result;
  int idx;
  msgParts_t *received;
  const uint8_t buf[1] = { 0};
  const uint8_t *myBuffer;

  received = &self->received;
ets_printf("§uartReceiveCb: %c rlen: %d§", buffer[0]&0xFF, received->lgth);
  myBuffer = buffer;
  if (lgth == 0) {
    // simulate a '0' char!!
    lgth = 1;
    myBuffer = buf;
  }
  result =self->addUartRequestData(self, (uint8_t *)myBuffer, lgth);
if (result != COMP_DISP_ERR_OK) {
ets_printf("§uartReceiveCb end result: %d§", result);
}
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
  uint8_t *b64Msg;
  uint8_t *host;
  uint8_t *subUrl;
  uint8_t *nodeToken;
  uint8_t *hostPart;
  uint8_t *alive;
  uint8_t *contentType;
  uint8_t *contentLgth;
  uint8_t *accept;
  int numericValue;
  size_t payloadLgth;

  if (self->compMsgData->nud == NULL) {
    return COMP_DISP_ERR_NO_WEBSOCKET_OPENED;
  }
ets_printf("sendCloudData: msgLgth: %d\n", msgLgth);
  payloadLgth = c_strlen("POST ");
  hostPart=" HTTP/1.1\r\nHost: ";
  payloadLgth += c_strlen(hostPart);
  alive="\r\nConnection: keep-alive\r\n";
  payloadLgth += c_strlen(alive);
  contentType="\r\nContent-Type: application/x-www-form-urlencoded\r\n";
  payloadLgth += c_strlen(contentType);
  contentLgth="Content-length: ";
  payloadLgth += c_strlen(contentLgth);
  payloadLgth += 5; // for contentLgth value
  accept="\r\nAccept: */*\r\n\r\n=";
  payloadLgth += c_strlen(accept);
  payloadLgth += msgLgth + 3;  // \r\n\0

  result = self->toBase64(msgData, &msgLgth, &b64Msg);
//ets_printf("base64 msg: mlen: %d %s %d\n", mlen, b64Msg, c_strlen(b64Msg));
  checkErrOK(result);
  b64Msg[msgLgth] = '\0';
  
  result = self->getWifiValue(self, WIFI_INFO_CLOUD_HOST_1, DATA_VIEW_FIELD_UINT8_T, &numericValue, &host);
  checkErrOK(result);
  payloadLgth += c_strlen(host);
  result = self->getWifiValue(self, WIFI_INFO_CLOUD_SUB_URL, DATA_VIEW_FIELD_UINT8_T, &numericValue, &subUrl);
  checkErrOK(result);
  payloadLgth += c_strlen(subUrl);
  result = self->getWifiValue(self, WIFI_INFO_CLOUD_NODE_TOKEN, DATA_VIEW_FIELD_UINT8_T, &numericValue, &nodeToken);
  checkErrOK(result);
  payloadLgth += c_strlen(nodeToken);
  char payload[payloadLgth];
  os_sprintf(payload, "POST %s%s%s%s%s%s%s%d%s%s\r\n", subUrl, hostPart, host, alive, nodeToken, contentType, contentLgth, msgLgth+1, accept, b64Msg);
ets_printf("request: %d %s\n", c_strlen(payload), payload);
  result = self->netsocketSendData(self->compMsgData->nud, payload, c_strlen(payload));
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
   result = typeRSendAnswer(self, msgData, msgLgth);
    checkErrOK(result);
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

  self->uartSetup = &uartSetup;
  self->uartReceiveCb = &uartReceiveCb;
  self->typeRSendAnswer = &typeRSendAnswer;
  self->sendCloudMsg = &sendCloudMsg;
  self->sendMsg = &sendMsg;
  return COMP_DISP_ERR_OK;
}

