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
//ets_printf("§uartSetup:id: %d baud: %d§", id, baud);
  checkErrOK(result);
  return COMP_MSG_ERR_OK;
}

// ================================= uartReceiveCb ====================================

static uint8_t uartReceiveCb(compMsgDispatcher_t *self, const uint8_t *buffer, uint8_t lgth) {
  int result;
  int idx;
  msgParts_t *received;
  const uint8_t buf[1] = { 0};
  const uint8_t *myBuffer;

  received = &self->compMsgData->received;
//ets_printf("§Rec: 0x%02x lgth: %d§", buffer[0] & 0xFF, received->lgth);
//ets_printf("§uartReceiveCb: %c rlen: %d§", buffer[0]&0xFF, received->lgth);
  myBuffer = buffer;
  if (lgth == 0) {
    // simulate a '0' char!!
    lgth = 1;
    myBuffer = buf;
  }
  result =self->compMsgRequest->addUartRequestData(self, (uint8_t *)myBuffer, lgth);
if (result != COMP_MSG_ERR_OK) {
ets_printf("§uartReceiveCb end result: %d§", result);
}
  checkErrOK(result);
  return COMP_MSG_ERR_OK;
}

// ================================= typeRSendAnswer ====================================

/**
 * \brief send a message via Uart to Mcu
 * \param self The dispatcher struct
 * \param data The message data
 * \param msgLgth The number of characters in the message
 * \return Error code or ErrorOK
 *
 */
static uint8_t typeRSendAnswer(compMsgDispatcher_t *self, uint8_t *data, uint8_t msgLgth) {
  int result;
  int idx;

//ets_printf("§typeRSendAnswer start: lgth: %d!§", msgLgth);
  idx = 0;
  while (idx < msgLgth) {
    platform_uart_send(0, data[idx]);
    idx++;
  }
//ets_printf("§typeRSendAnswer done§");
  return COMP_MSG_ERR_OK;
}

// ================================= sendCloudMsg ====================================

/**
 * \brief send a message via http socket to Cloud
 * \param self The dispatcher struct
 * \return Error code or ErrorOK
 *
 */
static uint8_t sendCloudMsg(compMsgDispatcher_t *self) {
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
  uint8_t *msgData;
  size_t msgLgth;

  if (self->compMsgData->sud == NULL) {
ets_printf("§COMP_MSG_ERR_NO_WEBSOCKET_OPENED§");
    return COMP_MSG_ERR_NO_WEBSOCKET_OPENED;
  }
//ets_printf("§request: %d %s§", self->cloudPayloadLgth, self->cloudPayload);
  result = COMP_MSG_ERR_OK;
  if (self->cloudPayload == NULL) {
//    result = self->prepareCloudMsg2(self);
//    checkErrOK(result);
ets_printf("§sendCloudMsg: cloudPayload == NULL§");
    return COMP_MSG_ERR_NO_WEBSOCKET_OPENED;
  }
  result = self->compMsgSocket->netSocketSendData(self->compMsgData->sud, self->cloudPayload, self->cloudPayloadLgth);
  checkErrOK(result);
  os_free(self->cloudPayload);
  self->cloudPayload = NULL;
  self->cloudPayloadLgth = 0;
//ets_printf("§sendCloudMsg: done result: %d§", result);
  return COMP_MSG_ERR_OK;
}

// ================================= prepareCloudMsg ====================================

/**
 * \brief prepare a message to be sent via http socket to Cloud
 * \param self The dispatcher struct
 * \return Error code or ErrorOK
 *
 */
static uint8_t prepareCloudMsg(compMsgDispatcher_t *self) {
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
  uint8_t *msgData;
  size_t msgLgth;
  char lgthBuf[20];

  msgData = self->cloudMsgData;
  msgLgth = self->cloudMsgDataLgth;
//ets_printf("§prepareCloudMsg: msgLgth: %d§", msgLgth);
  result = self->compMsgUtil->toBase64(msgData, &msgLgth, &b64Msg);
  checkErrOK(result);
//ets_printf("§prepareCloudMsg: b64msgLgth: %d§", msgLgth);

#ifdef CLOUD_1
  payloadLgth = c_strlen("POST ");
  result = self->compMsgWifiData->getWifiValue(self, WIFI_INFO_CLOUD_SUB_URL_1, DATA_VIEW_FIELD_UINT8_T, &numericValue, &subUrl);
  checkErrOK(result);
  payloadLgth += c_strlen(subUrl);
  hostPart=" HTTP/1.1\r\nHost: ";
  payloadLgth += c_strlen(hostPart);
  result = self->compMsgWifiData->getWifiValue(self, WIFI_INFO_CLOUD_HOST_1, DATA_VIEW_FIELD_UINT8_T, &numericValue, &host);
  checkErrOK(result);
  payloadLgth += c_strlen(host);
  alive="\r\nConnection: keep-alive\r\n";
  payloadLgth += c_strlen(alive);
  result = self->compMsgWifiData->getWifiValue(self, WIFI_INFO_CLOUD_NODE_TOKEN_1, DATA_VIEW_FIELD_UINT8_T, &numericValue, &nodeToken);
  checkErrOK(result);
  payloadLgth += c_strlen(nodeToken);
  contentType="\r\nContent-Type: application/x-www-form-urlencoded\r\n";
  payloadLgth += c_strlen(contentType);
  contentLgth="Content-length: ";
  payloadLgth += c_strlen(contentLgth);
  os_sprintf(lgthBuf, "%d\0", msgLgth);
  payloadLgth += c_strlen(lgthBuf); // for contentLgth value
  accept="\r\nAccept: */*\r\n\r\n";
  payloadLgth += c_strlen(accept);
  payloadLgth += (msgLgth + 3);  // \r\n\0
#else
  payloadLgth = c_strlen("POST ");
  result = self->compMsgWifiData->getWifiValue(self, WIFI_INFO_CLOUD_SUB_URL_2, DATA_VIEW_FIELD_UINT8_T, &numericValue, &subUrl);
  checkErrOK(result);
  payloadLgth += c_strlen(subUrl);
  hostPart=" HTTP/1.1\r\nHost: ";
  payloadLgth += c_strlen(hostPart);
  result = self->compMsgWifiData->getWifiValue(self, WIFI_INFO_CLOUD_HOST_2, DATA_VIEW_FIELD_UINT8_T, &numericValue, &host);
  checkErrOK(result);
  payloadLgth += c_strlen(host);
  alive="\r\n";
  payloadLgth += c_strlen(alive);
  result = self->compMsgWifiData->getWifiValue(self, WIFI_INFO_CLOUD_NODE_TOKEN_2, DATA_VIEW_FIELD_UINT8_T, &numericValue, &nodeToken);
  checkErrOK(result);
  payloadLgth += c_strlen(nodeToken);
  contentType="\r\nContent-Type: application/x-www-form-urlencoded\r\n";
  payloadLgth += c_strlen(contentType);
  contentLgth="Content-length: ";
  payloadLgth += c_strlen(contentLgth);
  os_sprintf(lgthBuf, "%d\0", msgLgth);
  payloadLgth += c_strlen(lgthBuf); // for contentLgth value
  accept="\r\n\r\n";
  payloadLgth += c_strlen(accept);
  payloadLgth += (msgLgth + 3);  // \r\n\0
#endif
  
  // FIXME need to free this somehwere!!!
  char *payload = os_zalloc(payloadLgth);
//ets_printf("§payloadLgth1: %d payload: %p§", payloadLgth, payload);
#ifdef CLOUD_1
  os_sprintf(payload, "POST %s%s%s%s%s%s%s%s%s%s\r\n", subUrl, hostPart, host, alive, nodeToken, contentType, contentLgth, lgthBuf, accept, b64Msg);
//ets_printf("§payloadLgth2: %d %d§", c_strlen(payload), payloadLgth);
#else
  os_sprintf(payload, "POST %s%s%s%s%s%s%s%s%s%s\r\n", subUrl, hostPart, host, alive, nodeToken, contentType, contentLgth, lgthBuf, accept, b64Msg);
//ets_printf("§payloadLgth3: %d %d§", c_strlen(payload), payloadLgth);
#endif
ets_printf("§request: %d %s§", c_strlen(payload), payload);
  os_free(b64Msg);
  self->cloudPayload = payload;
  self->cloudPayloadLgth = payloadLgth;
  self->cloudMsgData = NULL;
  self->cloudMsgDataLgth = 0;
  result = sendCloudMsg(self);
  checkErrOK(result);
//ets_printf("§prepareCloudMsg: done§");
  return COMP_MSG_ERR_OK;
}

// ================================= checkClientMode ====================================

/**
 * \brief start the connection with the router for sending a cloud message
 * \param self The dispatcher struct
 * \return Error code or ErrorOK
 *
 */
static uint8_t checkClientMode(compMsgDispatcher_t *self) {
  uint8_t result;

//ets_printf("§checkClientMode: §");

  self->compMsgSendReceive->startSendMsg2 = self->compMsgSendReceive->prepareCloudMsg;
  if (!(self->runningModeFlags & COMP_DISP_RUNNING_MODE_CLIENT)) {
// FIXME !!! TEMPORARY
    // set the callback used after client mode is running
    self->compMsgSendReceive->startSendMsg = self->compMsgSocket->netSocketStartCloudSocket;
    result = self->compMsgSocket->netSocketRunClientMode(self);
    checkErrOK(result);
// FIXME !!! TEMPORARY END
  } else {
    result = self->compMsgSocket->netSocketStartCloudSocket(self);
    checkErrOK(result);
  }
  return COMP_MSG_ERR_OK;
}

// ================================= sendMsg ====================================

static uint8_t sendMsg(compMsgDispatcher_t *self, uint8_t *msgData, size_t msgLgth) {
  uint8_t result;

ets_printf("§sendMsg: %c\n§", self->compMsgData->currHdr->hdrHandleType);
  switch (self->compMsgData->currHdr->hdrHandleType) {
  case 'A':
//ets_printf("sud: %p\n", self->compMsgData->sud);
    if (self->compMsgData->sud == NULL) {
      return COMP_MSG_ERR_NO_WEBSOCKET_OPENED;
    }
ets_printf("remote_ip: %d %d %d %d port: %d\n", self->compMsgData->sud->remote_ip[0], self->compMsgData->sud->remote_ip[1], self->compMsgData->sud->remote_ip[2], self->compMsgData->sud->remote_ip[3], self->compMsgData->sud->remote_port);
    result = self->compMsgSocket->webSocketSendData(self->compMsgData->sud, msgData, msgLgth, OPCODE_BINARY);
    checkErrOK(result);
    break;
  case 'G':
ets_printf("§sendMsg G not yet implemented§");
    break;
  case 'S':
    result = typeRSendAnswer(self, msgData, msgLgth);
    checkErrOK(result);
    result = self->compMsgData->deleteMsg(self);
    checkErrOK(result);
    break;
  case 'R':
ets_printf("§sendMsg R not yet implemented§");
    break;
  case 'U':
ets_printf("§type U msg send to cloud§");
    self->cloudMsgData = msgData;
    self->cloudMsgDataLgth = msgLgth;
// FIXME TEMPORARY!!!
result = checkClientMode(self);
checkErrOK(result);
// FIXME TEMPORARY!!!
    break;
  case 'W':
ets_printf("§sendMsg W not yet implemented§");
    break;
  case 'N':
    // just ignore
ets_printf("§sendMsg N not yet implemented§");
    return COMP_MSG_ERR_OK;
    break;
  default:
    return COMP_MSG_ERR_BAD_HANDLE_TYPE;
  }
//ets_printf("§sendMsg: done§");
  return COMP_MSG_ERR_OK;
}

// ================================= compMsgSendReceiveInit ====================================

uint8_t compMsgSendReceiveInit(compMsgDispatcher_t *self) {
  uint8_t result;

  result = self->getNewCompMsgDataPtr(self);
  self->compMsgData->sud = NULL;
  self->compMsgData->receivedData = NULL;
  self->compMsgData->receivedLgth = 0;
  self->compMsgData->direction = COMP_MSG_RECEIVED_DATA;
  result = self->compMsgRequest->addRequest(self, COMP_DISP_INPUT_UART, NULL, self->compMsgData);

  self->compMsgSendReceive->startSendMsg = NULL;
  self->compMsgSendReceive->startSendMsg2 = NULL;
  self->compMsgSendReceive->sendCloudMsg = NULL;

  self->compMsgSendReceive->uartSetup = &uartSetup;
  self->compMsgSendReceive->uartReceiveCb = &uartReceiveCb;
  self->compMsgSendReceive->typeRSendAnswer = &typeRSendAnswer;
  self->compMsgSendReceive->prepareCloudMsg = &prepareCloudMsg;
  self->compMsgSendReceive->checkClientMode = &checkClientMode;
  self->compMsgSendReceive->sendCloudMsg = &sendCloudMsg;
  self->compMsgSendReceive->sendMsg = &sendMsg;
  return COMP_MSG_ERR_OK;
}

// ================================= newCompMsgSendReceive ====================================

compMsgSendReceive_t *newCompMsgSendReceive() {
  compMsgSendReceive_t *compMsgSendReceive = os_zalloc(sizeof(compMsgSendReceive_t));
  if (compMsgSendReceive == NULL) {
    return NULL;
  }
  return compMsgSendReceive;
}
