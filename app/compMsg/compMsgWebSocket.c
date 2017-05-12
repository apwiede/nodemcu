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
 * File:   compMsgWebCocket.c
 * Author: Arnulf P. Wiedemann
 *
 * Created on October 10th, 2016
 */

#include "osapi.h"
#include "c_types.h"
#include "mem.h"
#include "flash_fs.h"

#include "lwip/err.h"
#include "lwip/app/espconn.h"
#include "lwip/ip_addr.h"
#include "lwip/dns.h"

#include "c_string.h"
#include "c_stdlib.h"
#include "c_stdio.h"
#include "platform.h"
#include "compMsgDispatcher.h"

#define TCP ESPCONN_TCP

typedef struct socketInfo {
  struct espconn *pesp_conn;
} socketInfo_t;

#define MAX_SOCKET 5
static int socketNum = 0;
static socketInfo_t *socket[MAX_SOCKET] = { NULL, NULL, NULL, NULL, NULL };

static const char *header_key = "Sec-WebSocket-Key: ";
static const char *ws_uuid ="258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

static const char *HEADER_WEBSOCKETLINE = "Upgrade: websocket";

static char *HEADER_OK = "HTTP/1.x 200 OK \r\n\
Server: ESP \r\n\
Connection: close \r\n\
Cache-Control: max-age=3600, public \r\n\
Content-Type: text/html \r\n\
Content-Encoding: gzip \r\n\r\n";

static char *HEADER_WEBSOCKET_START = "\
HTTP/1.1 101 WebSocket Protocol Handshake\r\n\
Connection: Upgrade\r\n\
Upgrade: websocket\r\n\
Access-Control-Allow-Origin: http://";

static char *HEADER_WEBSOCKET_URL = "192.168.178.67";

static char *HEADER_WEBSOCKET_END = "\r\n\
Access-Control-Allow-Credentials: true\r\n\
Access-Control-Allow-Headers: content-type \r\n\
Sec-WebSocket-Accept: ";

static uint8_t err_opcode[5] = {0};

// #define checkAllocOK(addr) if(addr == NULL) checkErrOK(WEBSOCKET_ERR_OUT_OF_MEMORY, "")

enum compMsg_error_code
{
  WEBSOCKET_ERR_OK = 0,
  WEBSOCKET_ERR_OUT_OF_MEMORY = -1,
  WEBSOCKET_ERR_TOO_MUCH_DATA = -2,
  WEBSOCKET_ERR_INVALID_FRAME_OPCODE = -3,
  WEBSOCKET_ERR_USERDATA_IS_NIL = -4,
  WEBSOCKET_ERR_WRONG_METATABLE = -5,
  WEBSOCKET_ERR_PESP_CONN_IS_NIL = -6,
  WEBSOCKET_ERR_MAX_SOCKET_REACHED = -7,
};

#define BASE64_INVALID '\xff'
#define BASE64_PADDING '='
#define ISBASE64(c) (unbytes64[c] != BASE64_INVALID)

#define SSL_BUFFER_SIZE 5120

static const uint8 b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static int cnt = 0;
static uint16_t tcp_server_timeover = 30;

// ============================ webSocketSendData =======================

/**
 * \brief send message data to a web scoket
 * \param sud The webSocket user data
 * \param payload The message data
 * \param size The number of characters of the message
 * \param opcode The type for sending the data (text/binary/...)
 * \return Error code or ErrorOK
 *
 */
static uint8_t webSocketSendData(socketUserData_t *sud, const char *payload, int size, int opcode)
{
  uint8_t hdrBytes[4]; // we have either 2 or 4 bytes depending on length of message
  int hdrLgth;
  int fsize;
  uint8_t*buff;
  uint8_t result;
  int i;
  compMsgDispatcher_t *self;

  self = sud->compMsgDispatcher;
  COMP_MSG_DBG(self, "W", 2, "webSocketSendData: size: %d\n", size);
  hdrLgth = 2;
  hdrBytes[0] = 0x80; //set first bit
  hdrBytes[0] |= opcode; //frame->opcode; //set op code
  if (size < 126) {
    hdrBytes[1] = size;
  } else {
    if (size < SSL_BUFFER_SIZE - sizeof(hdrBytes)) {
      hdrLgth += 2;
      hdrBytes[1] = 126;
      hdrBytes[2] = (size >> 8) & 0xFF;
      hdrBytes[3] = size & 0xFF;
    } else {
      return WEBSOCKET_ERR_TOO_MUCH_DATA;
    }
  }
  fsize = size + hdrLgth;
  buff = os_zalloc(fsize);
  if (buff == NULL) {
    return WEBSOCKET_ERR_OUT_OF_MEMORY;
  }
  for (i = 0; i < hdrLgth; i++) {
    buff[i] = hdrBytes[i];
  }

  os_memcpy(&buff[hdrLgth], (uint8_t *)payload, size);
  result = espconn_sent(sud->pesp_conn, (unsigned char *)buff, fsize);
  COMP_MSG_DBG(self, "W", 2, "espconn_sent: result: %d\n", result);
  os_free(buff);
//  self->compMsgRequest->startNextRequest(self);
  COMP_MSG_DBG(self, "W", 2, "espconn_sent: done\n");
  return WEBSOCKET_ERR_OK;
}

// ============================ webSocketParse =========================================

static int ICACHE_FLASH_ATTR webSocketParse(char * data, size_t dataLenb, char **resData, int *len, socketUserData_t *sud) {
  uint8_t byte = data[0];
  uint8_t FIN = byte & 0x80;
  uint8_t opcode = byte & 0x0F;
  compMsgDispatcher_t *self;

//ets_printf("webSocketParse1 heap: %d\n", system_get_free_heap_size());
  self = sud->compMsgDispatcher;
  if ((opcode > 0x03 && opcode < 0x08) || opcode > 0x0B) {
    ets_sprintf(err_opcode, "%d", opcode);
    checkErrOK(WEBSOCKET_ERR_INVALID_FRAME_OPCODE);
  }

  //   opcodes: {1 text 2 binary 8 close 9 ping 10 pong}
  switch (opcode) {
  case OPCODE_TEXT:
    COMP_MSG_DBG(self, "W", 2, "parse text\n");
    break;
  case OPCODE_BINARY:
    COMP_MSG_DBG(self, "W", 2, "parse binary\n");
    break;
  case OPCODE_CLOSE:
    break;
  case OPCODE_PING:
    webSocketSendData(sud, " ", 1, OPCODE_PONG);
    return WEBSOCKET_ERR_OK;
    break;
  case OPCODE_PONG:
    return WEBSOCKET_ERR_OK;
    break;
  }
  byte = data[1];

  char * recv_data;
  int masked = byte & 0x80;
  int size = byte & 0x7F;
  int offset = 2;

  if (size == 126) {
    size = 0;
    size = data[3];                  //LSB
    size |= (uint64_t) data[2] << 8; //MSB
    offset = 4;
  } else if (size == 127) {
    size = 0;
    size |= (uint64_t) data[2] << 56;
    size |= (uint64_t) data[3] << 48;
    size |= (uint64_t) data[4] << 40;
    size |= (uint64_t) data[5] << 32;
    size |= (uint64_t) data[6] << 24;
    size |= (uint64_t) data[7] << 16;
    size |= (uint64_t) data[8] << 8;
    size |= (uint64_t) data[9];
    offset = 10;
  }

  if (masked) {
    //read mask key
    char mask[4];
    uint64_t i;

    mask[0] = data[offset];
    mask[1] = data[offset + 1];
    mask[2] = data[offset + 2];
    mask[3] = data[offset + 3];
    offset += 4;
    for (i = 0; i < size; i++) {
      data[i + offset] ^= mask[i % 4];
    }
  }
  recv_data = &data[offset];
//for (int i = 0; i < size; i++) {
//  COMP_MSG_DBG(self, "W", 2, "i: %d 0x%02x\n", i, recv_data[i]&0xFF);
//}
  switch (opcode) {
  case OPCODE_TEXT:
    COMP_MSG_DBG(self, "W", 2, "cb text\n");
    sud->webSocketTextReceived(sud->compMsgDispatcher, sud, recv_data, size);
    break;
  case OPCODE_BINARY:
    COMP_MSG_DBG(self, "W", 2, "cb binary\n");
    COMP_MSG_DBG(self, "W", 2, "parse binary: size: %d\n", size);
//ets_printf("webSocketParse2 heap: %d\n", system_get_free_heap_size());
    sud->webSocketBinaryReceived(sud->compMsgDispatcher, sud, recv_data, size);
    break;
  }
  *resData = recv_data;
  *len = size;
  recv_data[size] = 0;

  if (size + offset < dataLenb) {
    webSocketParse(&data[size + offset], dataLenb - (size + offset), resData, len, sud);
  }
  return WEBSOCKET_ERR_OK;
}

// ================================= webSocketRecv ====================================

static uint8_t webSocketRecv(char *string, socketUserData_t *sud, char **data, int *lgth) {
  char * key = NULL;
  int idx;
  int found;
  uint8_t result;
  compMsgDispatcher_t *self;

  self = sud->compMsgDispatcher;
  idx = 0;
//ets_printf("webSocketRecv1 heap: %d\n", system_get_free_heap_size());
  COMP_MSG_DBG(self, "W", 2, "webSocketRecv: %s!remote_port: %d\n", sud->curr_url, sud->remote_port);
  COMP_MSG_DBG(self, "W", 1, "webSocketRecv: %s!%s!remote_port: %d\n", string,  sud->curr_url, sud->remote_port);
  if ((sud->curr_url != NULL) && (strstr(string, sud->curr_url) != NULL)) {
    if (strstr(string, header_key) != NULL) {
      char *begin = strstr(string, header_key) + os_strlen(header_key);
      char *end = strstr(begin, "\r");
      key = os_zalloc((end - begin) + 1);
      checkAllocOK(key);
      os_memcpy(key, begin, end - begin);
      key[end - begin] = 0;
    }
    COMP_MSG_DBG(self, "W", 1, "webSocketRecv2: key: %s\n", key);
    const char *trailer;
    trailer = "\r\n\r\n";
    int trailerLen;
    trailerLen = os_strlen(trailer);
    size_t digestLen;
    digestLen = 20; //sha1 is always 20 byte long
    uint8_t digest[digestLen];
    int payloadLen;
    char *payload;
    uint8_t *base64Digest;
    SHA1_CTX ctx;

//ets_printf("webSocketRecv2 heap: %d\n", system_get_free_heap_size());
    // Use the SHA* functions in the rom
    SHA1Init(&ctx);
    SHA1Update(&ctx, key, os_strlen(key));
    SHA1Update(&ctx, ws_uuid, os_strlen(ws_uuid));
    SHA1Final(digest, &ctx);
//ets_printf("webSocketRecv3 heap: %d\n", system_get_free_heap_size());

    result = sud->compMsgDispatcher->compMsgUtil->toBase64(self, digest, &digestLen, &base64Digest);
    checkErrOK(result);
    checkAllocOK(base64Digest);
//ets_printf("webSocketRecv4 heap: %d\n", system_get_free_heap_size());
    payloadLen = os_strlen(HEADER_WEBSOCKET_START) + os_strlen(HEADER_WEBSOCKET_URL) +os_strlen(HEADER_WEBSOCKET_END) + digestLen + trailerLen;
    payload = os_zalloc(payloadLen + 1); // +1 for trailing '\0' character
    checkAllocOK(payload);
//ets_printf("webSocketRecv5 heap: %d\n", system_get_free_heap_size());
    os_sprintf(payload, "%s%s%s%s%s\0", HEADER_WEBSOCKET_START, HEADER_WEBSOCKET_URL, HEADER_WEBSOCKET_END, base64Digest, trailer);
    COMP_MSG_DBG(self, "W", 2, "payloadLen: %d payload: %d\n", payloadLen, c_strlen(payload));
    struct espconn *pesp_conn = NULL;
    pesp_conn = sud->pesp_conn;

    // FIXME!! reboot called if setting here for socket!!
    if (sud->isWebsocket != 1) {
      sud->isWebsocket = 1;
    }

//    COMP_MSG_DBG(self, "W", 1, "payload: %d!%s!\n", payloadLen, payload);
    COMP_MSG_DBG(self, "W", 1, "payload: %d!\n", payloadLen);
//ets_printf("webSocketRecv6 heap: %d\n", system_get_free_heap_size());
    result = espconn_sent(sud->pesp_conn, (unsigned char *)payload, payloadLen);
    COMP_MSG_DBG(self, "W", 2, "espconn_sent: result: %d\n", result);
    checkErrOK(result);
//ets_printf("webSocketRecv7 heap: %d\n", system_get_free_heap_size());
    os_free(key);
//ets_printf("webSocketRecv8 heap: %d\n", system_get_free_heap_size());
    os_free(base64Digest);
//ets_printf("webSocketRecv9 heap: %d\n", system_get_free_heap_size());
    os_free(payload);
//ets_printf("webSocketRecv10 heap: %d\n", system_get_free_heap_size());
  } else if (sud->isWebsocket == 1) {
//ets_printf("webSocketRecv11 heap: %d\n", system_get_free_heap_size());
    COMP_MSG_DBG(self, "W", 2, "webSocketParse: %d!curr_url: %s!\n", os_strlen(string), sud->curr_url);
    webSocketParse(string, os_strlen(string), data, lgth, sud);
//ets_printf("webSocketRecv12 heap: %d\n", system_get_free_heap_size());
  }
  return WEBSOCKET_ERR_OK;
}

// ================================= serverDisconnected  ====================================

static void serverDisconnected(void *arg) {
  struct espconn *pesp_conn;
  socketUserData_t *sud;
  compMsgDispatcher_t *self;

  pesp_conn = (struct espconn *)arg;
  sud = pesp_conn->reverse;
  self = sud->compMsgDispatcher;
  COMP_MSG_DBG(self, "W", 2, "serverDisconnected: arg: %p\n", arg);

}

// ================================= serverReconnected  ====================================

static void serverReconnected(void *arg, int8_t err) {
  struct espconn *pesp_conn;
  socketUserData_t *sud;
  compMsgDispatcher_t *self;

  pesp_conn = (struct espconn *)arg;
  sud = pesp_conn->reverse;
  self = sud->compMsgDispatcher;
  COMP_MSG_DBG(self, "W", 2, "serverReconnected: arg: %p\n", arg);

}

// ================================= webSocketReceived  ====================================

static void webSocketReceived(void *arg, char *pdata, unsigned short len) {
  struct espconn *pesp_conn;
  char url[50] = { 0 };
  int idx;
  socketUserData_t *sud;
  compMsgDispatcher_t *self;
  const uint8_t *connection;
  const uint8_t *upgrade;
  uint8_t result;
  uint8_t result1;
  uint8_t result2;
  char temp[20] = {0};

  pesp_conn = (struct espconn *)arg;
  sud = (socketUserData_t *)pesp_conn->reverse;
  self = sud->compMsgDispatcher;
  COMP_MSG_DBG(self, "W", 1, "webSocketReceived: arg: %p len: %d\n", arg, len);
  COMP_MSG_DBG(self, "W", 1, "webSocketReceived: arg: %p pdata: %s len: %d\n", arg, pdata, len);
  c_sprintf(temp, IPSTR, IP2STR( &(pesp_conn->proto.tcp->remote_ip) ) );
  COMP_MSG_DBG(self, "W", 1, "remote %s:%d received.\n", temp, pesp_conn->proto.tcp->remote_port);
//ets_printf("\n>>>webSocketReceived 1 heap: %d\n", system_get_free_heap_size());
  sud->remote_ip[0] = pesp_conn->proto.tcp->remote_ip[0];
  sud->remote_ip[1] = pesp_conn->proto.tcp->remote_ip[1];
  sud->remote_ip[2] = pesp_conn->proto.tcp->remote_ip[2];
  sud->remote_ip[3] = pesp_conn->proto.tcp->remote_ip[3];
  sud->remote_port = pesp_conn->proto.tcp->remote_port;
  COMP_MSG_DBG(self, "W", 1, "==received remote_port: %d\n", sud->remote_port);
  if (strstr(pdata, "GET /") != 0) {
    char *begin = strstr(pdata, "GET /") + 4;
    char *end = strstr(begin, " ");
    os_memcpy(url, begin, end - begin);
    url[end - begin] = 0;
    COMP_MSG_DBG(self, "W", 1, "url: %s\n", url);
  }
  COMP_MSG_DBG(self, "W", 2, "pdata: %s!\n", pdata);
//  if ((url[0] != 0) && (strstr(pdata, HEADER_WEBSOCKETLINE) != 0))
  if (url[0] != 0) {
//ets_printf("webSocketRecv parse 1a  heap: %d\n", system_get_free_heap_size());
    result = sud->compMsgDispatcher->compMsgHttp->httpParse(sud, pdata, len);
    COMP_MSG_DBG(self, "W", 1, "httpParse: result: %d!\n", result);
    if (result == COMP_MSG_ERR_OK) {
      result1 = sud->compMsgDispatcher->compMsgHttp->getHttpGetHeaderValueForId(sud, COMP_MSG_HTTP_CONNECTION, &connection);
      COMP_MSG_DBG(self, "W", 1, "value connection: %s result: %d\n", connection, result);
      result2 = sud->compMsgDispatcher->compMsgHttp->getHttpGetHeaderValueForId(sud, COMP_MSG_HTTP_UPGRADE, &upgrade);
      COMP_MSG_DBG(self, "W", 1, "value upgrade: %s result: %d\n", upgrade, result);
      if ((result1 == COMP_MSG_ERR_OK) && (result1 == COMP_MSG_ERR_OK) && (c_strncmp(connection, "Upgrade", 7) == 0) && (c_strcmp(upgrade, "websocket") == 0)) {
        idx = 0;
        sud->curr_url = NULL;
        COMP_MSG_DBG(self, "W", 1, "num_urls: %d\n", sud->num_urls);
        while (idx < sud->num_urls) {
          COMP_MSG_DBG(self, "W", 1, "url: idx: %d %s %s\n", idx, url, sud->urls[idx]);
          if (c_strcmp(url, sud->urls[idx]) == 0) {
            sud->curr_url = sud->urls[idx];
            COMP_MSG_DBG(self, "W", 1, "isWebsocket: url: %s\n", sud->curr_url);
            sud->isWebsocket = 1;
            break;
          }
          idx++;
        }
      }
    }
    sud->compMsgDispatcher->compMsgHttp->deleteHttpMsgInfo(sud);
//ets_printf("webSocketRecv parse 1b  heap: %d\n", system_get_free_heap_size());
  }
  COMP_MSG_DBG(self, "W", 2, "iswebSocket: %d %s\n", sud->isWebsocket, sud->curr_url);

//ets_printf("webSocketReceived 2 heap: %d\n", system_get_free_heap_size());
  if(sud->isWebsocket == 1) {
    char *data = "";
    int lgth = 0;
    int result;

    result = webSocketRecv(pdata, sud, &data, &lgth);
//    checkErrOK(result);
  }
}

// ================================= socketSent  ====================================

static void socketSent(void *arg) {
  struct espconn *pesp_conn;
  socketUserData_t *sud;
  compMsgDispatcher_t *self;
  uint8_t result;
  bool boolResult;
  int interval;
  uint8_t timerId;

  pesp_conn = (struct espconn *)arg;
  if (pesp_conn == NULL) {
    return;
  }
  sud = (socketUserData_t *)pesp_conn->reverse;
  if(sud == NULL) {
    return;
  }
  self = sud->compMsgDispatcher;
  COMP_MSG_DBG(self, "W", 2, "webSocketSent: arg: %p\n", arg);
//ets_printf("socketSent done stopAccessPoint: %d\n", sud->compMsgDispatcher->dispatcherCommon->stopAccessPoint);
  if (sud->compMsgDispatcher->dispatcherCommon->stopAccessPoint) {
    // stop the accessPoint
    self->sud = sud;
    self->compMsgSocket->stopAccessPoint(self);
  }
}

// ================================= serverConnected  ====================================

static void serverConnected(void *arg) {
  struct espconn *pesp_conn;
  socketUserData_t *sud;
  compMsgDispatcher_t *self;
  int result;
  int i;

  pesp_conn = arg;
  sud = (socketUserData_t *)pesp_conn->reverse;
  self = sud->compMsgDispatcher;
  COMP_MSG_DBG(self, "W", 1, "serverConnected: arg: %p\n", arg);
  for(i = 0; i < MAX_SOCKET; i++) {
    if (socket[i] == NULL) { // found empty slot
      break;
    }
  }
  if(i >= MAX_SOCKET) { // can't create more socket
    pesp_conn->reverse = NULL;    // not accept this conn
    if(pesp_conn->proto.tcp->remote_port || pesp_conn->proto.tcp->local_port) {
      espconn_disconnect(pesp_conn);
    }
//    checkErrOK(gL, WEBSOCKET_ERR_MAX_SOCKET_REACHED, "webSocketServerConnected");
    return;
  }
  COMP_MSG_DBG(self, "W", 2, "registstart\n");
  result = espconn_regist_recvcb(pesp_conn, webSocketReceived);
  if (result != COMP_MSG_ERR_OK) {
    COMP_MSG_DBG(self, "Y", 0, "regist webSocketReceived err: %d\n", result);
  }
  result = espconn_regist_sentcb(pesp_conn, socketSent);
  if (result != COMP_MSG_ERR_OK) {
    COMP_MSG_DBG(self, "Y", 0, "regist socketSent err: %d\n", result);
  }
  result = espconn_regist_disconcb(pesp_conn, serverDisconnected);
  if (result != COMP_MSG_ERR_OK) {
    COMP_MSG_DBG(self, "Y", 0, "regist serverDisconnected err: %d\n", result);
  }
  result = espconn_regist_reconcb(pesp_conn, serverReconnected);
  if (result != COMP_MSG_ERR_OK) {
    COMP_MSG_DBG(self, "Y", 0, "regist serverReconnected err: %d\n", result);
  }
}

// ================================= stopAccessPoint  ====================================

static void stopAccessPoint(void *arg) {
  socketUserData_t *sud;
  compMsgTimerSlot_t *compMsgTimerSlot;
  compMsgDispatcher_t *self;
  bool boolResult;

//ets_printf("stopAccessPoint\n");
  self = (compMsgDispatcher_t *)arg;

  sud = self->sud;
  if (sud->compMsgDispatcher->dispatcherCommon->stopAccessPoint) {
    COMP_MSG_DBG(self, "W", 1, "stopAccessPoint\n");
    sud->compMsgDispatcher->dispatcherCommon->stopAccessPoint = false;
    boolResult = wifi_set_opmode(OPMODE_STATION);
    if (!boolResult) {
      COMP_MSG_DBG(self, "W", 1, "webSocketSent: COMP_MSG_ERR_CANNOT_SET_OPMODE");
//      return COMP_MSG_ERR_CANNOT_SET_OPMODE;
    }
  }
}

// ================================= startWebClientMode ====================================

static void startWebClientMode(void *arg) {
  compMsgDispatcher_t *self;
  int numericValue;
  uint8_t *stringValue;
  uint8_t *strValue;
  struct espconn *pesp_conn;
  unsigned port;
  uint8_t flags;
  unsigned type;
  int result;
  socketUserData_t *sud;
  fieldValueCallback_t callback;
  compMsgTimerSlot_t *compMsgTimerSlot;
  char temp[64];
  fieldValue_t fieldValue;
  uint8_t valueId;

  compMsgTimerSlot = (compMsgTimerSlot_t *)arg;
  self = compMsgTimerSlot->compMsgDispatcher;
  COMP_MSG_DBG(self, "W", 1, "startWebClientMode timerInfo: %p\n", compMsgTimerSlot);
  compMsgTimerSlot->connectionMode = STATION_IF;

  compMsgTimerSlot->followupCallback = self->compMsgSocket->sendSSDPInfo;
  compMsgTimerSlot->followupTimerId = 1;
  compMsgTimerSlot->followupInterval = 3000;
  compMsgTimerSlot->followupMode = 1;

  pesp_conn = NULL;

  result = self->compMsgSocket->checkConnectionStatus(compMsgTimerSlot);
  if (result != COMP_MSG_ERR_OK) {
    return;
  }

  self->dispatcherCommon->runningModeFlags |= COMP_DISP_RUNNING_MODE_CLIENT;
  result = self->compMsgDataValue->dataValueStr2ValueId(self, "@clientIPAddr", &valueId);
  fieldValue.flags = COMP_MSG_FIELD_IS_NUMERIC;
  fieldValue.dataValue.value.numericValue = compMsgTimerSlot->ip_addr;
  fieldValue.cmdKey = COMP_MSG_DATA_VALUE_CMD_KEY_SPECIAL;
  fieldValue.fieldNameId = 0;
  fieldValue.fieldValueId = valueId;
  fieldValue.fieldValueCallback = NULL;
  result = self->compMsgDataValue->setFieldValueInfo(self, &fieldValue);
  fieldValue.flags = COMP_MSG_FIELD_IS_NUMERIC;
  fieldValue.dataValue.value.numericValue = 0;
  fieldValue.cmdKey = COMP_MSG_DATA_VALUE_CMD_KEY_SPECIAL;
  fieldValue.fieldNameId = 0;
  fieldValue.fieldValueId = COMP_MSG_WIFI_VALUE_ID_clientIPAddr;
  fieldValue.fieldValueCallback = NULL;
  result = self->compMsgDataValue->getFieldValueInfo(self, &fieldValue, &strValue);
  numericValue = fieldValue.dataValue.value.numericValue;
  COMP_MSG_DBG(self, "N", 2, "ip2: 0x%08x\n", numericValue);
  c_sprintf(temp, "%d.%d.%d.%d", IP2STR(&numericValue));

  fieldValue.flags = COMP_MSG_FIELD_IS_NUMERIC;
  fieldValue.dataValue.value.numericValue = 0;
  fieldValue.cmdKey = COMP_MSG_DATA_VALUE_CMD_KEY_SPECIAL;
  fieldValue.fieldNameId = 0;
  fieldValue.fieldValueId = COMP_MSG_WIFI_VALUE_ID_clientPort;
  fieldValue.fieldValueCallback = NULL;
  result = self->compMsgDataValue->getFieldValueInfo(self, &fieldValue, &strValue);
  port = fieldValue.dataValue.value.numericValue;
  COMP_MSG_DBG(self, "N", 1, "startWebClientMode IP: %s port: %d result: %d\n", temp, port, result);


  sud = (socketUserData_t *)os_zalloc(sizeof(socketUserData_t));
//   checkAllocOK(sud);
  COMP_MSG_DBG(self, "W", 2, "sud0: %p\n", sud);
  sud->maxHttpMsgInfos = 5;
  sud->numHttpMsgInfos = 0;
  sud->httpMsgInfos = os_zalloc(sud->maxHttpMsgInfos * sizeof(httpMsgInfo_t));
  sud->isWebsocket = 0;
  sud->connectionType = WEB_SOCKET_TYPE_CLIENT;
  sud->num_urls = 0;
  sud->max_urls = 4;
  sud->urls = (char **)c_zalloc(sizeof(char *) * sud->max_urls);
//  checkAllocgLOK(sud->urls);
  sud->urls[0] = NULL;
  sud->curr_url = NULL;
sud->urls[0] = "/callmcu";
sud->num_urls = 1;
  fieldValue.flags = COMP_MSG_FIELD_IS_NUMERIC;
  fieldValue.dataValue.value.numericValue = 0;
  fieldValue.cmdKey = COMP_MSG_DATA_VALUE_CMD_KEY_SPECIAL;
  fieldValue.fieldNameId = 0;
  fieldValue.fieldValueId = COMP_MSG_WIFI_VALUE_ID_binaryCallback;
  fieldValue.fieldValueCallback = NULL;
  result = self->compMsgDataValue->getFieldValueInfo(self, &fieldValue, &strValue);
  numericValue = fieldValue.dataValue.value.numericValue;
  COMP_MSG_DBG(self, "W", 2, "binaryCallback: %p!%d!\n", numericValue, result);
  sud->webSocketBinaryReceived = (webSocketBinaryReceived_t)numericValue;
  fieldValue.flags = COMP_MSG_FIELD_IS_NUMERIC;
  fieldValue.dataValue.value.numericValue = 0;
  fieldValue.cmdKey = COMP_MSG_DATA_VALUE_CMD_KEY_SPECIAL;
  fieldValue.fieldNameId = 0;
  fieldValue.fieldValueId = COMP_MSG_WIFI_VALUE_ID_textCallback;
  fieldValue.fieldValueCallback = NULL;
  result = self->compMsgDataValue->getFieldValueInfo(self, &fieldValue, &strValue);
  numericValue = fieldValue.dataValue.value.numericValue;
  COMP_MSG_DBG(self, "W", 2, "textCallback: %p!%d!\n", numericValue, result);
  sud->webSocketTextReceived = (webSocketTextReceived_t)numericValue;
  sud->compMsgDispatcher = self;

  pesp_conn = (struct espconn *)os_zalloc(sizeof(struct espconn));
  sud->pesp_conn = pesp_conn;
//  checkAllocOK(pesp_conn);

  type = ESPCONN_TCP;
  pesp_conn->type = type;
  pesp_conn->state = ESPCONN_NONE;
  // reverse is for the callback function
  pesp_conn->reverse = sud;

  pesp_conn->proto.tcp = NULL;
  pesp_conn->proto.udp = NULL;

  pesp_conn->proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));
  if(!pesp_conn->proto.tcp){
    os_free(pesp_conn);
    pesp_conn = NULL;
//    checkErrOK(COMP_MSG_ERR_OUT_OF_MEMORY);
  }
  pesp_conn->proto.tcp->local_port = port;
  COMP_MSG_DBG(self, "W", 2, "port: %d\n", port);

  COMP_MSG_DBG(self, "W", 2, "call regist connectcb\n");
  result = espconn_regist_connectcb(pesp_conn, serverConnected);
  if (result != COMP_MSG_ERR_OK) {
//    return COMP_MSG_ERR_REGIST_CONNECT_CB;
  }
  COMP_MSG_DBG(self, "W", 2, "regist connectcb result: %d\n", result);
  result = espconn_accept(pesp_conn);    // if it's a server, no need to dns.
  if (result != COMP_MSG_ERR_OK) {
//    return COMP_MSG_ERR_TCP_ACCEPT;
  }
  COMP_MSG_DBG(self, "W", 2, "regist_accept result: %d\n", result);
  result = espconn_regist_time(pesp_conn, tcp_server_timeover, 0);
  if (result != COMP_MSG_ERR_OK) {
//    return COMP_MSG_ERR_REGIST_TIME;
  }
  COMP_MSG_DBG(self, "W", 2, "regist_time result: %d\n", result);
}

// ================================= startAccessPoint ====================================

static void startAccessPoint(void *arg) {
  compMsgDispatcher_t *self;
  struct espconn *pesp_conn;
  unsigned port;
  unsigned type;
  int result;
  uint8_t flags;
  uint8_t *strValue;
  fieldValueCallback_t callback;
  socketUserData_t *sud;
  fieldValue_t fieldValue;
  compMsgTimerSlot_t *compMsgTimerSlot;

  compMsgTimerSlot = (compMsgTimerSlot_t *)arg;
  self = compMsgTimerSlot->compMsgDispatcher;
  COMP_MSG_DBG(self, "W", 1, "startAccessPoint timerInfo: %p\n", compMsgTimerSlot);
  compMsgTimerSlot->connectionMode = SOFTAP_IF;
  pesp_conn = NULL;

  result = self->compMsgSocket->checkConnectionStatus(compMsgTimerSlot);
  if (result != COMP_MSG_ERR_OK) {
    return;
  }

  self->dispatcherCommon->runningModeFlags |= COMP_DISP_RUNNING_MODE_ACCESS_POINT;
  fieldValue.flags = COMP_MSG_FIELD_IS_NUMERIC;
  fieldValue.dataValue.value.numericValue = 0;
  fieldValue.cmdKey = COMP_MSG_DATA_VALUE_CMD_KEY_SPECIAL;
  fieldValue.fieldNameId = 0;
  fieldValue.fieldValueId = COMP_MSG_WIFI_VALUE_ID_provisioningPort;
  fieldValue.fieldValueCallback = NULL;
  result = self->compMsgDataValue->getFieldValueInfo(self, &fieldValue, &strValue);
  COMP_MSG_DBG(self, "W", 1, "AP port: %d!result: %d!\n", fieldValue.dataValue.value.numericValue, result);
//  checkErrOK(result);
  port = fieldValue.dataValue.value.numericValue;
//  result = self->compMsgDataValue->getFieldValueInfo(self, COMP_MSG_DATA_VALUE_CMD_KEY_SPECIAL, COMP_MSG_WIFI_VALUE_ID_provisioningIPAddr, &flags, &callback, &numericValue, &stringValue);
//  checkErrOK(result);

  sud = (socketUserData_t *)os_zalloc(sizeof(socketUserData_t));
//   checkAllocOK(sud);
  COMP_MSG_DBG(self, "W", 2, "sud0: %p\n", sud);
  sud->maxHttpMsgInfos = 5;
  sud->numHttpMsgInfos = 0;
  sud->httpMsgInfos = os_zalloc(sud->maxHttpMsgInfos * sizeof(httpMsgInfo_t));
  sud->isWebsocket = 0;
  sud->num_urls = 0;
  sud->max_urls = 4;
  sud->urls = (char **)c_zalloc(sizeof(char *) * sud->max_urls);
//  checkAllocgLOK(sud->urls);
  sud->urls[0] = NULL;
  sud->curr_url = NULL;
sud->urls[0] = "/getaplist";
sud->urls[1] = "/getapdeflist";
sud->num_urls = 2;
  fieldValue.flags = COMP_MSG_FIELD_IS_NUMERIC;
  fieldValue.dataValue.value.numericValue = 0;
  fieldValue.cmdKey = COMP_MSG_DATA_VALUE_CMD_KEY_SPECIAL;
  fieldValue.fieldNameId = 0;
  fieldValue.fieldValueId = COMP_MSG_WIFI_VALUE_ID_binaryCallback;
  fieldValue.fieldValueCallback = NULL;
  result = self->compMsgDataValue->getFieldValueInfo(self, &fieldValue, &strValue);
  COMP_MSG_DBG(self, "W", 2, "binaryCallback: %p!%d!\n", fieldValue.dataValue.value.numericValue, result);
  sud->webSocketBinaryReceived = (webSocketBinaryReceived_t)fieldValue.dataValue.value.numericValue;
  fieldValue.flags = COMP_MSG_FIELD_IS_NUMERIC;
  fieldValue.dataValue.value.numericValue = 0;
  fieldValue.cmdKey = COMP_MSG_DATA_VALUE_CMD_KEY_SPECIAL;
  fieldValue.fieldNameId = 0;
  fieldValue.fieldValueId = COMP_MSG_WIFI_VALUE_ID_textCallback;
  fieldValue.fieldValueCallback = NULL;
  result = self->compMsgDataValue->getFieldValueInfo(self, &fieldValue, &strValue);
  COMP_MSG_DBG(self, "W", 2, "binaryCallback: %p!%d!\n", fieldValue.dataValue.value.numericValue, result);
  sud->webSocketTextReceived = (webSocketTextReceived_t)fieldValue.dataValue.value.numericValue;
  sud->compMsgDispatcher = self;

  pesp_conn = (struct espconn *)os_zalloc(sizeof(struct espconn));
  sud->pesp_conn = pesp_conn;
//  checkAllocOK(pesp_conn);

  type = ESPCONN_TCP;
  pesp_conn->type = type;
  pesp_conn->state = ESPCONN_NONE;
  // reverse is for the callback function
  pesp_conn->reverse = sud;

  pesp_conn->proto.tcp = NULL;
  pesp_conn->proto.udp = NULL;

  pesp_conn->proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));
  if(!pesp_conn->proto.tcp){
    os_free(pesp_conn);
    pesp_conn = NULL;
//    checkErrOK(COMP_MSG_ERR_OUT_OF_MEMORY);
  }
  pesp_conn->proto.tcp->local_port = port;
  COMP_MSG_DBG(self, "W", 2, "port: %d\n", port);

  COMP_MSG_DBG(self, "W", 2, "call regist connectcb\n");
  result = espconn_regist_connectcb(pesp_conn, serverConnected);
  if (result != COMP_MSG_ERR_OK) {
//    return COMP_MSG_ERR_REGIST_CONNECT_CB;
  }
  COMP_MSG_DBG(self, "W", 2, "regist connectcb result: %d\n", result);
  result = espconn_accept(pesp_conn);    // if it's a server, no need to dns.
  if (result != COMP_MSG_ERR_OK) {
//    return COMP_MSG_ERR_TCP_ACCEPT;
  }
  COMP_MSG_DBG(self, "W", 2, "regist_accept result: %d\n", result);
  result = espconn_regist_time(pesp_conn, tcp_server_timeover, 0);
  if (result != COMP_MSG_ERR_OK) {
//    return COMP_MSG_ERR_REGIST_TIME;
  }
  COMP_MSG_DBG(self, "W", 2, "regist_time result: %d\n", result);
}

// ================================= webSocketRunAPMode ====================================

static uint8_t webSocketRunAPMode(compMsgDispatcher_t *self) {
  int result;
  uint8_t status;
  struct softap_config softap_config;
  int numericValue;
  int interval;
  bool boolResult;
  uint8_t timerId;
  uint8_t *stringValue;
  uint8_t *strValue;
  uint8_t flags;
  fieldValue_t fieldValue;
  fieldValueCallback_t callback;
  
  COMP_MSG_DBG(self, "W", 2, "webSocketRunAPMode\n");
  boolResult = wifi_station_disconnect();
// checking result makes problems with some modules, so do not check.
//  if (!boolResult) {
//    COMP_MSG_DBG(self, "Y", 0, "webSocketRunAPMode COMP_MSG_ERR_CANNOT_DISCONNECT\n");
//    return COMP_MSG_ERR_CANNOT_DISCONNECT;
//  }
  boolResult = wifi_set_opmode(OPMODE_STATIONAP);
  if (!boolResult) {
    COMP_MSG_DBG(self, "Y", 0, "webSocketRunAPMode COMP_MSG_ERR_CANNOT_SET_OPMODE\n");
    return COMP_MSG_ERR_CANNOT_SET_OPMODE;
  }
  c_memset(softap_config.ssid,0,sizeof(softap_config.ssid));
  fieldValue.flags = COMP_MSG_FIELD_IS_STRING;
  fieldValue.dataValue.value.stringValue = NULL;
  fieldValue.cmdKey = COMP_MSG_DATA_VALUE_CMD_KEY_SPECIAL;
  fieldValue.fieldNameId = 0;
  fieldValue.fieldValueId = COMP_MSG_WIFI_VALUE_ID_provisioningSsid;
  fieldValue.fieldValueCallback = NULL;
  strValue = NULL;
  result = self->compMsgDataValue->getFieldValueInfo(self, &fieldValue, &strValue);
  COMP_MSG_DBG(self, "W", 2, "webSocketRunAPMode get_PROVISIONING_SSID result: %d\n", result);
  checkErrOK(result);
  stringValue = fieldValue.dataValue.value.stringValue;
  c_memcpy(softap_config.ssid, stringValue, c_strlen(stringValue));
  softap_config.ssid_len = c_strlen(stringValue);
  softap_config.ssid_hidden = 0;
  softap_config.authmode = AUTH_OPEN;
  softap_config.channel = 6;
  softap_config.max_connection = 4;
  softap_config.beacon_interval = 100;
  COMP_MSG_DBG(self, "W", 2, "webSocketRunAPMode wifi_softap_set_config\n");
  boolResult = wifi_softap_set_config(&softap_config);
  if (!boolResult) {
    return COMP_MSG_ERR_CANNOT_SET_OPMODE;
  }
  status = wifi_station_get_connect_status();
  COMP_MSG_DBG(self, "W", 1, "wifi is in mode: %d status: %d ap_id: %d hostname: %s!\n", wifi_get_opmode(), status, wifi_station_get_current_ap_id(), wifi_station_get_hostname());
  interval = 1000;
  timerId = 0;
  return self->compMsgSocket->startConnectionTimer(self, timerId, interval, 1, self->compMsgSocket->startAccessPoint);
}

// ================================= webSocketRunClientMode ====================================

static uint8_t webSocketRunClientMode(compMsgDispatcher_t *self) {
  int result;
  uint8_t status;
  struct station_config station_config;
  uint8_t opmode;
  int numericValue;
  int interval;
  bool boolResult;
  uint8_t timerId;
  uint8_t *stringValue;
  uint8_t *strValue;
  char *hostName;
  uint8_t flags;
  fieldValue_t fieldValue;
  fieldValueCallback_t callback;

  COMP_MSG_DBG(self, "W", 2, "webSocketRunClientMode\n");
  COMP_MSG_DBG(self, "N", 1, "webSocketRunClientMode called\n");
  opmode = wifi_get_opmode();
  COMP_MSG_DBG(self, "N", 1, "opmode: %d", opmode);
  status = wifi_station_get_connect_status();
//ets_printf("webSocketRunClientMode connectStatus: %d\n", status);
//  boolResult = wifi_station_disconnect();
// checking result makes problems with some modules, so do not check.
//  if (!boolResult) {
//    COMP_MSG_DBG(self, "Y", 0, "webSocketRunAPMode COMP_MSG_ERR_CANNOT_DISCONNECT\n");
//    return COMP_MSG_ERR_CANNOT_DISCONNECT;
//  }
  COMP_MSG_DBG(self, "N", 2, "wifi_station_disconnect: boolResult: %d", boolResult);
  c_memset(station_config.ssid,0,sizeof(station_config.ssid));
  fieldValue.flags = COMP_MSG_FIELD_IS_STRING;
  fieldValue.dataValue.value.stringValue = NULL;
  fieldValue.cmdKey = COMP_MSG_DATA_VALUE_CMD_KEY_SPECIAL;
  fieldValue.fieldNameId = 0;
  fieldValue.fieldValueId = COMP_MSG_WIFI_VALUE_ID_clientSsid;
  fieldValue.fieldValueCallback = NULL;
  result = self->compMsgDataValue->getFieldValueInfo(self, &fieldValue, &strValue);
  COMP_MSG_DBG(self, "N", 2, "getSsid: result: %d\n", result);
  checkErrOK(result);
  stringValue = fieldValue.dataValue.value.stringValue;
  c_memcpy(station_config.ssid, stringValue, c_strlen(stringValue));

  c_memset(station_config.password,0,sizeof(station_config.password));
  fieldValue.flags = COMP_MSG_FIELD_IS_STRING;
  fieldValue.dataValue.value.stringValue = NULL;
  fieldValue.cmdKey = COMP_MSG_DATA_VALUE_CMD_KEY_SPECIAL;
  fieldValue.fieldNameId = 0;
  fieldValue.fieldValueId = COMP_MSG_WIFI_VALUE_ID_clientPasswd;
  fieldValue.fieldValueCallback = NULL;
  result = self->compMsgDataValue->getFieldValueInfo(self, &fieldValue, &strValue);
  COMP_MSG_DBG(self, "N", 2, "getPasswd: result: %d\n", result);
  checkErrOK(result);
  stringValue = fieldValue.dataValue.value.stringValue;
  COMP_MSG_DBG(self, "N", 2, "len password: %d\n", c_strlen(stringValue));
  c_memcpy(station_config.password, stringValue, c_strlen(stringValue));
  COMP_MSG_DBG(self, "N", 1, "webSocketRunClientMode: ssid: %s password: %s!\n", station_config.ssid, station_config.password);

  boolResult = wifi_station_set_config(&station_config);
  if (!boolResult) {
    return COMP_MSG_ERR_CANNOT_SET_OPMODE;
  }
//  wifi_station_set_auto_connect(true);
  boolResult = wifi_station_connect();
  if (!boolResult) {
    return COMP_MSG_ERR_CANNOT_CONNECT;
  }
  boolResult = wifi_station_set_hostname("testDeviceClient");
  opmode = wifi_get_opmode();
  status = wifi_station_get_connect_status();
  hostName = wifi_station_get_hostname();
  COMP_MSG_DBG(self, "N", 1, "wifi is in mode: %d status: %d hostname: %s!\n", opmode, status, hostName);
  
  timerId = 4;
  interval = 1000;
  return self->compMsgSocket->startConnectionTimer(self, timerId, interval, 1, self->compMsgSocket->startWebClientMode);
}

// ================================= compMsgWebSocketInit ====================================

uint8_t compMsgWebSocketInit(compMsgDispatcher_t *self) {
  uint8_t result;

  self->compMsgSocket->webSocketRunClientMode = &webSocketRunClientMode;
  self->compMsgSocket->webSocketRunAPMode = &webSocketRunAPMode;
  self->compMsgSocket->webSocketSendData = &webSocketSendData;
  self->compMsgSocket->startWebClientMode = &startWebClientMode;
  self->compMsgSocket->startAccessPoint = &startAccessPoint;
  self->compMsgSocket->stopAccessPoint = &stopAccessPoint;
  return COMP_MSG_ERR_OK;
}
