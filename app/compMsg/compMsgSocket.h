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
 * File:   compMsgSocket.h
 * Author: Arnulf P. Wiedemann
 *
 * Created on Descmber 16, 2016
 */

#ifndef COMP_MSG_SOCKET_H
#define	COMP_MSG_SOCKET_H

#include "c_types.h"
#ifdef	__cplusplus
extern "C" {
#endif

// running mode flags
#define COMP_DISP_RUNNING_MODE_ACCESS_POINT 0x01
#define COMP_DISP_RUNNING_MODE_CLIENT       0x02
#define COMP_DISP_RUNNING_MODE_CLOUD        0x04
#define COMP_DISP_RUNNING_MODE_APP          0x08
#define COMP_DISP_RUNNING_MODE_WEBSOCKET    0x10
#define COMP_DISP_RUNNING_MODE_SSDP         0x20

typedef struct compMsgDispatcher compMsgDispatcher_t;

typedef struct httpHeaderPart {
  uint8_t httpHeaderId;
  uint8_t *httpHeaderName;
  uint8_t *httpHeaderValue;
} httpHeaderPart_t;

typedef struct httpMsgInfo {
  httpHeaderPart_t *receivedHeaders;
  int httpRequestCode;
  uint8_t numHeaders;
  size_t currLgth;
  size_t expectedLgth;
  uint8_t *content;
  char *data;
} httpMsgInfo_t;

typedef struct socketUserData  socketUserData_t;

typedef void (* webSocketBinaryReceived_t)(void *arg, socketUserData_t *sud, char *pdata, unsigned short len);
typedef void (* webSocketTextReceived_t)(void *arg, socketUserData_t *sud, char *pdata, unsigned short len);
typedef void (* netSocketToSend_t)(void *arg, socketUserData_t *sud, char *pdata, unsigned short len);
typedef void (* netSocketReceived_t)(void *arg, socketUserData_t *sud, char *pdata, unsigned short len);
typedef void (* netSocketSSDPToSend_t)(void *arg, socketUserData_t *sud, char *pdata, unsigned short len);
typedef void (* netSocketSSDPReceived_t)(void *arg, socketUserData_t *sud, char *pdata, unsigned short len);

typedef struct socketUserData {
  struct espconn *pesp_conn;
  int remote_port;
  uint8_t remote_ip[4];
  uint8_t connectionType;
  uint8_t isWebsocket;
  uint8_t num_urls;
  uint8_t max_urls;
#ifdef CLIENT_SSL_ENABLE
  uint8_t secure;
#endif
  char **urls; // that is the array of url parts which is used in socket_on for the different receive callbacks
  char *curr_url; // that is url which has been provided in the received data
  compMsgDispatcher_t *compMsgDispatcher;
  netSocketReceived_t netSocketReceived;
  netSocketToSend_t netSocketToSend;
  webSocketBinaryReceived_t webSocketBinaryReceived;
  webSocketTextReceived_t webSocketTextReceived;
  uint8_t numHttpMsgInfos;
  uint8_t maxHttpMsgInfos;
  httpMsgInfo_t *httpMsgInfos;
  char *payloadBuf;
} socketUserData_t;

typedef void (* startConnection_t)(void *arg);

// WebSocket stuff
typedef uint8_t (* webSocketRunAPMode_t)(compMsgDispatcher_t *self);
typedef uint8_t (* webSocketRunClientMode_t)(compMsgDispatcher_t *self, uint8_t mode);
typedef uint8_t (* webSocketSendData_t)(socketUserData_t *sud, const char *payload, int size, int opcode);
typedef void (* startAccessPoint_t)(void *arg);

// NetSocket stuff
typedef uint8_t (* netSocketStartCloudSocket_t)(compMsgDispatcher_t *self);
typedef uint8_t (* netSocketRunClientMode_t)(compMsgDispatcher_t *self);
typedef uint8_t (* netSocketRunSSDPMode_t)(compMsgDispatcher_t *self);
typedef uint8_t (* netSocketSendData_t)(socketUserData_t *sud, const char *payload, int size);
typedef void (* startClientMode_t)(void *arg);
typedef void (* startSSDPMode_t)(void *arg);

typedef uint8_t (* checkConnectionStatus_t)(compMsgTimerSlot_t *compMsgTimerSlot);
typedef uint8_t (* startConnectionTimer_t)(compMsgDispatcher_t *self, uint8_t timerId, startConnection_t fcn);

typedef struct compMsgSocket {

  webSocketRunAPMode_t webSocketRunAPMode;
  webSocketRunClientMode_t webSocketRunClientMode;
  webSocketSendData_t webSocketSendData;
  startAccessPoint_t startAccessPoint;

  netSocketStartCloudSocket_t netSocketStartCloudSocket;
  netSocketRunClientMode_t netSocketRunClientMode;
  netSocketRunSSDPMode_t netSocketRunSSDPMode;
  netSocketSendData_t netSocketSendData;
  startClientMode_t startClientMode;
  startSSDPMode_t startSSDPMode;

  startConnectionTimer_t startConnectionTimer;
  checkConnectionStatus_t checkConnectionStatus;
} compMsgSocket_t;

#ifdef  __cplusplus
}
#endif

compMsgSocket_t *newCompMsgSocket();
uint8_t compMsgWebSocketInit(compMsgDispatcher_t *self);
uint8_t compMsgNetSocketInit(compMsgDispatcher_t *self);


#endif  /* COMP_MSG_SOCKET_H */


