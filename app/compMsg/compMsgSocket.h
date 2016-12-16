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

typedef struct compMsgDispatcher compMsgDispatcher_t;

typedef struct httpHeaderPart {
  uint8_t httpHeaderId;
  uint8_t *httpHeaderName;
  uint8_t *httpHeaderValue;
} httpHeaderPart_t;

typedef struct httpMsgInfo {
  httpHeaderPart_t *receivedHeaders;
  int httpCode;
  size_t currLgth;
  size_t expectedLgth;
  uint8_t *content;
} httpMsgInfo_t;

typedef struct socketUserData  socketUserData_t;

typedef void (* webSocketBinaryReceived_t)(void *arg, socketUserData_t *sud, char *pdata, unsigned short len);
typedef void (* webSocketTextReceived_t)(void *arg, socketUserData_t *sud, char *pdata, unsigned short len);
typedef void (* netSocketToSend_t)(void *arg, socketUserData_t *sud, char *pdata, unsigned short len);
typedef void (* netSocketReceived_t)(void *arg, socketUserData_t *sud, char *pdata, unsigned short len);

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
} socketUserData_t;

typedef struct compMsgSocket {
} compMsgSocket_t;

#ifdef  __cplusplus
}
#endif

#endif  /* COMP_MSG_SOCKET_H */


