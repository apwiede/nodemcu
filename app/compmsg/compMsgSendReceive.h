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
 * File:   compMsgSendReceive.h
 * Author: Arnulf P. Wiedemann
 *
 * Created on Descmber 17, 2016
 */

#ifndef COMP_MSG_SEND_RECEIVE_H
#define	COMP_MSG_SEND_RECEIVE_H

#include "c_types.h"
#ifdef	__cplusplus
extern "C" {
#endif

typedef struct compMsgDispatcher compMsgDispatcher_t;

typedef uint8_t (* uartSetup_t)(compMsgDispatcher_t *self, unsigned id, uint32_t baud, int databits, int parity, int stopbits);
typedef uint8_t (* uartReceiveCb_t)(compMsgDispatcher_t *self, const uint8_t *buffer, size_t lgth);
typedef uint8_t (* uartSendAnswer_t)(compMsgDispatcher_t *self, uint8_t *data, uint8_t msgLgth);
typedef uint8_t (* prepareCloudMsg_t)(compMsgDispatcher_t *self);
typedef uint8_t (* startSendMsg_t)(compMsgDispatcher_t *self);
typedef uint8_t (* sendCloudMsg_t)(compMsgDispatcher_t *self);
typedef uint8_t (* sendMsg_t)(compMsgDispatcher_t *self, uint8_t *msgData, size_t msgLgth);

typedef struct compMsgSendReceive {
  uartSetup_t uartSetup;
  uartReceiveCb_t uartReceiveCb;
  uartSendAnswer_t uartSendAnswer;
  sendMsg_t sendMsg;
  prepareCloudMsg_t prepareCloudMsg;
  prepareCloudMsg_t checkClientMode;
  sendCloudMsg_t sendCloudMsg;
  startSendMsg_t startSendMsg;
  startSendMsg_t startSendMsg2;
} compMsgSendReceive_t;

compMsgSendReceive_t *newCompMsgSendReceive();
uint8_t compMsgSendReceiveInit(compMsgDispatcher_t *self);

#ifdef  __cplusplus
}
#endif

#endif  /* COMP_MSG_SEND_RECEIVE_H */

