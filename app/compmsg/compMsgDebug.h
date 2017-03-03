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
 * File:   compMsgDebug.h
 * Author: Arnulf P. Wiedemann
 *
 * Created on December 18, 2016
 */

#ifndef COMP_MSG_DEBUG_H
#define	COMP_MSG_DEBUG_H

#include "c_types.h"
#ifdef	__cplusplus
extern "C" {
#endif

#define DEBUG_COMP_MSG_ACTION          0x00001
#define DEBUG_COMP_MSG_BUILD_MSG       0x00002
#define DEBUG_COMP_MSG_DISPATCHER      0x00004
#define DEBUG_COMP_MSG_DATA            0x00008
#define DEBUG_DATA_VIEW                0x00010
#define DEBUG_COMP_MSG_DATA_VIEW       0x00020
#define DEBUG_COMP_MSG_HTTP            0x00040
#define DEBUG_COMP_MSG_IDENTIFY        0x00080
#define DEBUG_COMP_MSG_MSG_DESC        0x00100
#define DEBUG_COMP_MSG_MODULE_DATA     0x00200
#define DEBUG_COMP_MSG_NET_SOCKET      0x00400
#define DEBUG_COMP_MSG_OTA             0x00800
#define DEBUG_COMP_MSG_REQUEST         0x01000
#define DEBUG_COMP_MSG_SOCKET          0x02000
#define DEBUG_COMP_MSG_SEND_RECEIVE    0x04000
#define DEBUG_COMP_MSG_TYPES_AND_NAMES 0x08000
#define DEBUG_COMP_MSG_TIMER           0x10000
#define DEBUG_COMP_MSG_UTIL            0x20000
#define DEBUG_COMP_MSG_WEB_SOCKET      0x40000
#define DEBUG_COMP_MSG_WIFI_DATA       0x80000
#define DEBUG_COMP_MSG_ALWAYS          0x100000

#ifdef COMP_MSG_DEBUG
#define COMP_MSG_DBG self->compMsgDebug->dbgPrintf
#else
#define COMP_MSG_DBG
#endif

typedef struct compMsgDispatcher compMsgDispatcher_t;

typedef void (* dbgPrintf_t)(compMsgDispatcher_t *self, uint8_t *dbgChars, uint8_t debugLevel, uint8_t *format, ...);
typedef uint32_t (* getDebugFlags_t)(compMsgDispatcher_t *self, uint8_t *dbgChars);
typedef uint8_t (* setDebugFlags_t)(compMsgDispatcher_t *self, uint8_t *dbgChars);
typedef uint8_t (* dumpMsgParts_t)(compMsgDispatcher_t *self, msgParts_t *msgParts);
typedef uint8_t (* dumpMsgHeaderInfos_t)(compMsgDispatcher_t *self, msgHeaderInfos_t *hdrInfos);
typedef uint8_t (* dumpHeaderPart_t)(compMsgDispatcher_t *self, headerPart_t *hdr);
typedef uint8_t (* dumpMsgDescPart_t)(compMsgDispatcher_t *self, msgDescPart_t *msgDescPart);
typedef uint8_t (* dumpMsgValPart_t)(compMsgDispatcher_t *self, msgValPart_t *msgValPart);

typedef struct compMsgDebug {
  uint32_t currDebugFlags;
  bool addEol;
  bool useUart;
  uint8_t debugLevel;
  uint8_t debugUartId;

  dbgPrintf_t dbgPrintf;
  getDebugFlags_t getDebugFlags;
  setDebugFlags_t setDebugFlags;
  dumpMsgParts_t dumpMsgParts;
  dumpMsgHeaderInfos_t dumpMsgHeaderInfos;
  dumpHeaderPart_t dumpHeaderPart;
  dumpMsgDescPart_t dumpMsgDescPart;
  dumpMsgValPart_t dumpMsgValPart;
} compMsgDebug_t;

compMsgDebug_t *newCompMsgDebug();
uint8_t compMsgDebugInit(compMsgDispatcher_t *self);

#ifdef  __cplusplus
}
#endif

#endif  /* COMP_MSG_DEBUG_H */

