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
#ifdef	__cplusplus
extern "C" {
#endif

#define COMP_DISP_MAX_REQUESTS     5

// input source types
#define COMP_DISP_INPUT_UART       0x01
#define COMP_DISP_INPUT_NET_SOCKET 0x02
#define COMP_DISP_INPUT_WEB_SOCKET 0x04

typedef struct compMsgDispatcher compMsgDispatcher_t;

typedef struct msgRequestInfos {
  uint8_t requestTypes[COMP_DISP_MAX_REQUESTS];
  void *requestHandles[COMP_DISP_MAX_REQUESTS];
  compMsgData_t *requestData[COMP_DISP_MAX_REQUESTS];
  int currRequestIdx;
  int lastRequestIdx;
} msgRequestInfos_t;

typedef uint8_t (* startRequest_t)(compMsgDispatcher_t *self);
typedef uint8_t (* startNextRequest_t)(compMsgDispatcher_t *self);
typedef uint8_t (* addUartRequestData_t)(compMsgDispatcher_t *self, uint8_t *data, size_t lgth);
typedef uint8_t (* addRequest_t)(compMsgDispatcher_t *self, uint8_t requestType, void *requestHandle, compMsgData_t *requestData);
typedef uint8_t (* deleteRequest_t)(compMsgDispatcher_t *self, uint8_t requestType, void *requestHandle);

typedef struct compMsgRequest {
  // request infos
  msgRequestInfos_t msgRequestInfos;

  startRequest_t startRequest;
  startNextRequest_t startNextRequest;
  addUartRequestData_t addUartRequestData;
  addRequest_t addRequest;
  deleteRequest_t deleteRequest;
} compMsgRequest_t;

compMsgRequest_t *newCompMsgRequest();
uint8_t compMsgRequestInit(compMsgDispatcher_t *self);

#ifdef  __cplusplus
}
#endif

#endif  /* COMP_MSG_REQUEST_H */

