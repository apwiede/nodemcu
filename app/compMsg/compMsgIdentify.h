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
 * File:   compMsgIdentify.h
 * Author: Arnulf P. Wiedemann
 *
 * Created on Descmber 17, 2016
 */

#ifndef COMP_MSG_IDENTIFY_H
#define	COMP_MSG_IDENTIFY_H

#include "c_types.h"

typedef struct compMsgDispatcher compMsgDispatcher_t;

typedef uint8_t (* resetHeaderInfos_t)(compMsgDispatcher_t *self);
typedef uint8_t (* getHeaderIndexFromHeaderFields_t)(compMsgDispatcher_t *self, msgParts_t *received);
typedef uint8_t (* nextFittingEntry_t)(compMsgDispatcher_t *self, msgParts_t *received, uint8_t u8CmdKey, uint16_t u16CmdKey);
typedef uint8_t (* handleToSendPart_t)(compMsgDispatcher_t *self, const uint8_t * buffer, uint8_t lgth);
typedef uint8_t (* prepareAnswerMsg_t)(compMsgDispatcher_t *self, uint8_t type, uint8_t **handle);
typedef uint8_t (* handleReceivedHeader_t)(compMsgDispatcher_t *self, msgParts_t *received);
typedef uint8_t (* handleReceivedMsg_t)(compMsgDispatcher_t *self, msgParts_t *received);
typedef uint8_t (* storeReceivedMsg_t)(compMsgDispatcher_t *self, msgParts_t *received);
typedef uint8_t (* sendClientIPMsg_t)(compMsgDispatcher_t *self);
typedef uint8_t (* handleReceivedPart_t)(compMsgDispatcher_t *self, const uint8_t * buffer, size_t lgth);
typedef uint8_t (* unpackReceivedMsg_t)(compMsgDispatcher_t *self, msgRequestInfo_t *msgRequestInfo);
typedef uint8_t (* compMsgIdentifyInit_t)(compMsgDispatcher_t *self);

typedef struct compMsgIdentify {
  resetHeaderInfos_t resetHeaderInfos;
  nextFittingEntry_t nextFittingEntry;
  getHeaderIndexFromHeaderFields_t getHeaderIndexFromHeaderFields;
  prepareAnswerMsg_t prepareAnswerMsg;
  handleReceivedHeader_t handleReceivedHeader;
  handleReceivedMsg_t handleReceivedMsg;
  storeReceivedMsg_t storeReceivedMsg;
  sendClientIPMsg_t sendClientIPMsg;
  handleReceivedPart_t handleReceivedPart;
  unpackReceivedMsg_t unpackReceivedMsg;
  handleToSendPart_t handleToSendPart;
  compMsgIdentifyInit_t compMsgIdentifyInit;
} compMsgIdentify_t;

compMsgIdentify_t *newCompMsgIdentify();

#endif  /* COMP_MSG_IDENTIFY_H */
