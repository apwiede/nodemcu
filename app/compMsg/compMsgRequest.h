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
 * File:   compMsgRequest.h
 * Author: Arnulf P. Wiedemann
 *
 * Created on December 18, 2016
 */

#ifndef COMP_MSG_REQUEST_H
#define	COMP_MSG_REQUEST_H

#include "c_types.h"

#define COMP_MSG_MAX_UART_REQUESTS 5
#define COMP_MSG_MAX_REQUESTS      5

// input source types
#define COMP_MSG_INPUT_UART       0x01
#define COMP_MSG_INPUT_NET_SOCKET 0x02
#define COMP_MSG_INPUT_WEB_SOCKET 0x04

#define COMP_MSG_MAX_REQUEST_LGTH 255

typedef struct compMsgDispatcher compMsgDispatcher_t;
typedef struct socketUserData socketUserData_t;

typedef struct msgRequestInfo {
  uint8_t requestType;
  socketUserData_t *sud;
  uint32_t IPAddr;
  compMsgDispatcher_t *requestDispatcher;
  uint8_t *data;
  uint16_t lgth;
  uint16_t currLgth;
  uint16_t currIdx;
  uint16_t totalLgth;
  bool isComplete;
  bool isUsed;
} msgRequestInfo_t;

typedef struct msgRequestInfos {
  msgRequestInfo_t *msgRequestInfo[COMP_MSG_MAX_REQUESTS];
  int currRequestIdx;
  int lastRequestIdx;
} msgRequestInfos_t;

typedef uint8_t (* uartTimerResetRequest_t)(void *compMsgTimerSlot);
typedef uint8_t (* resetRequest_t)(compMsgDispatcher_t *self, int slot);
typedef uint8_t (* startRequest_t)(compMsgDispatcher_t *self);
typedef uint8_t (* startNextRequest_t)(compMsgDispatcher_t *self);
typedef uint8_t (* addRequestData_t)(compMsgDispatcher_t *self, uint8_t requestType, socketUserData_t *sud, uint8_t *data, uint16_t lgth);
typedef uint8_t (* deleteRequest_t)(compMsgDispatcher_t *self, uint8_t requestType, socketUserData_t *sud);
typedef uint8_t (* detectMsg_t)(compMsgDispatcher_t *self, const uint8_t * buffer, int lgth, int queueIdx, bool *isComplete);
typedef uint8_t (* compMsgRequestInit_t)(compMsgDispatcher_t *self);

typedef struct compMsgRequest {
  // request infos
  msgRequestInfos_t msgRequestInfos;

  uartTimerResetRequest_t uartTimerResetRequest;
  resetRequest_t resetRequest;
  startRequest_t startRequest;
  startNextRequest_t startNextRequest;
  addRequestData_t addRequestData;
  deleteRequest_t deleteRequest;
  detectMsg_t detectMsg;
  compMsgRequestInit_t compMsgRequestInit;
} compMsgRequest_t;

compMsgRequest_t *newCompMsgRequest();

#endif  /* COMP_MSG_REQUEST_H */
