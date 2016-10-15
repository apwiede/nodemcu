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
 * File:   structmsgDispatcher.c
 * Author: Arnulf P. Wiedemann
 *
 * Created on October 2st, 2016
 */

#include "osapi.h"
#include "c_types.h"
#include "mem.h"
#include "flash_fs.h"

#include "c_string.h"
#include "c_stdlib.h"
#include "c_stdio.h"
#include "c_limits.h"
#include "platform.h"
#include "structmsg.h"
#include "../crypto/mech.h"

#define BASE64_INVALID '\xff'
#define BASE64_PADDING '='
#define ISBASE64(c) (unbytes64[c] != BASE64_INVALID)

#define DISP_HANDLE_PREFIX "stmsgdisp_"

typedef struct handle2Dispatcher
{
  uint8_t *handle;
  structmsgDispatcher_t *structmsgDispatcher;
} handle2Dispatcher_t;

typedef struct structmsgDispatcherHandles
{
  handle2Dispatcher_t *handles;
  int numHandles;
} structmsgDispatcherHandles_t;

// create an object
static structmsgDispatcherHandles_t structmsgDispatcherHandles = { NULL, 0};

static int structmsgDispatcherId = 0;
// right now we only need one dispatcher!
static structmsgDispatcher_t *structmsgDispatcherSingleton = NULL;

static const uint8 b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// ============================= toBase64 ========================

static uint8_t toBase64(const uint8_t *msg, size_t *len, uint8_t **encoded) {
  size_t i;
  size_t n;
  uint8_t *q;
  uint8_t *out;
  uint8_t bytes64[sizeof(b64)];

  n = *len;
  if (!n) { // handle empty string case 
    return STRUCT_DISP_ERR_OUT_OF_MEMORY;
  }
  out = (uint8_t *)os_zalloc((n + 2) / 3 * 4);
  checkAllocOK(out);
  c_memcpy(bytes64, b64, sizeof(b64));   //Avoid lots of flash unaligned fetches
  
  for (i = 0, q = out; i < n; i += 3) {
    int a = msg[i];
    int b = (i + 1 < n) ? msg[i + 1] : 0;
    int c = (i + 2 < n) ? msg[i + 2] : 0;
    *q++ = bytes64[a >> 2];
    *q++ = bytes64[((a & 3) << 4) | (b >> 4)];
    *q++ = (i + 1 < n) ? bytes64[((b & 15) << 2) | (c >> 6)] : BASE64_PADDING;
    *q++ = (i + 2 < n) ? bytes64[(c & 63)] : BASE64_PADDING;
  }
  *len = q - out;
  *encoded = out;
  return STRUCT_DISP_ERR_OK;
}

// ============================= fromBase64 ========================

static uint8_t fromBase64(const uint8_t *encodedMsg, size_t *len, uint8_t **decodedMsg) {
  int i;
  int n;
  int blocks;
  int pad;
  const uint8 *p;
  uint8_t unbytes64[UCHAR_MAX+1];
  uint8_t *msg;
  uint8_t *q;

  n = *len;
  blocks = (n>>2);
  pad = 0;
  if (!n) { // handle empty string case 
    return STRUCT_DISP_ERR_OUT_OF_MEMORY;
  } 
  if (n & 3) {
    return STRUCT_DISP_ERR_INVALID_BASE64_STRING;
  } 
  c_memset(unbytes64, BASE64_INVALID, sizeof(unbytes64));
  for (i = 0; i < sizeof(b64)-1; i++) {
    unbytes64[b64[i]] = i;  // sequential so no exceptions 
  }
  if (encodedMsg[n-1] == BASE64_PADDING) {
    pad =  (encodedMsg[n-2] != BASE64_PADDING) ? 1 : 2;
    blocks--;  //exclude padding block
  }    

  for (i = 0; i < n - pad; i++) {
    if (!ISBASE64(encodedMsg[i])) {
      return STRUCT_DISP_ERR_INVALID_BASE64_STRING;
    }
  }
  unbytes64[BASE64_PADDING] = 0;
  q = (uint8_t *) os_zalloc(1+ (3 * n / 4)); 
  checkAllocOK(q);
  msg = q;
  for (i = 0, p = encodedMsg; i<blocks; i++) {
    uint8 a = unbytes64[*p++]; 
    uint8 b = unbytes64[*p++]; 
    uint8 c = unbytes64[*p++]; 
    uint8 d = unbytes64[*p++];
    *q++ = (a << 2) | (b >> 4);
    *q++ = (b << 4) | (c >> 2);
    *q++ = (c << 6) | d;
  }
  if (pad) { //now process padding block bytes
    uint8 a = unbytes64[*p++];
    uint8 b = unbytes64[*p++];
    *q++ = (a << 2) | (b >> 4);
    if (pad == 1) *q++ = (b << 4) | (unbytes64[*p] >> 2);
  }
  *len = q - msg;
  *decodedMsg = msg;
  return STRUCT_DISP_ERR_OK;;
}

// ============================= addHandle ========================

static int addHandle(uint8_t *handle, structmsgDispatcher_t *structmsgDispatcher) {
  int idx;

  if (structmsgDispatcherHandles.handles == NULL) {
    structmsgDispatcherHandles.handles = os_zalloc(sizeof(handle2Dispatcher_t));
    if (structmsgDispatcherHandles.handles == NULL) {
      return STRUCT_DISP_ERR_OUT_OF_MEMORY;
    } else {
      structmsgDispatcherHandles.handles[structmsgDispatcherHandles.numHandles].handle = handle;
      structmsgDispatcherHandles.handles[structmsgDispatcherHandles.numHandles].structmsgDispatcher = structmsgDispatcher;
      structmsgDispatcherHandles.numHandles++;
      return STRUCT_DISP_ERR_OK;
    }
  } else {
    // check for unused slot first
    idx = 0;
    while (idx < structmsgDispatcherHandles.numHandles) {
      if (structmsgDispatcherHandles.handles[idx].handle == NULL) {
        structmsgDispatcherHandles.handles[idx].handle = handle;
        structmsgDispatcherHandles.handles[idx].structmsgDispatcher = structmsgDispatcher;
        return STRUCT_DISP_ERR_OK;
      }
      idx++;
    }
    structmsgDispatcherHandles.handles = os_realloc(structmsgDispatcherHandles.handles, sizeof(handle2Dispatcher_t)*(structmsgDispatcherHandles.numHandles+1));
    checkAllocOK(structmsgDispatcherHandles.handles);
    structmsgDispatcherHandles.handles[structmsgDispatcherHandles.numHandles].handle = handle;
    structmsgDispatcherHandles.handles[idx].structmsgDispatcher = structmsgDispatcher;
    structmsgDispatcherHandles.numHandles++;
  }
  return STRUCT_DISP_ERR_OK;
}

// ============================= deleteHandle ========================

static int deleteHandle(const uint8_t *handle) {
  int idx;
  int numUsed;
  int found;

  if (structmsgDispatcherHandles.handles == NULL) {
    return STRUCT_DISP_ERR_HANDLE_NOT_FOUND;
  }
  found = 0;
  idx = 0;
  numUsed = 0;
  while (idx < structmsgDispatcherHandles.numHandles) {
    if ((structmsgDispatcherHandles.handles[idx].handle != NULL) && (c_strcmp(structmsgDispatcherHandles.handles[idx].handle, handle) == 0)) {
      structmsgDispatcherHandles.handles[idx].handle = NULL;
      found++;
    } else {
      if (structmsgDispatcherHandles.handles[idx].handle != NULL) {
        numUsed++;
      }
    }
    idx++;
  }
  if (numUsed == 0) {
    os_free(structmsgDispatcherHandles.handles);
    structmsgDispatcherHandles.handles = NULL;
  }
  if (found) {
      return STRUCT_DISP_ERR_OK;
  }
  return STRUCT_DISP_ERR_HANDLE_NOT_FOUND;
}

// ============================= checkHandle ========================

static int checkHandle(const char *handle, structmsgDispatcher_t **structmsgDispatcher) {
  int idx;

  if (structmsgDispatcherHandles.handles == NULL) {
    return STRUCT_DISP_ERR_HANDLE_NOT_FOUND;
  }
  idx = 0;
  while (idx < structmsgDispatcherHandles.numHandles) {
    if ((structmsgDispatcherHandles.handles[idx].handle != NULL) && (c_strcmp(structmsgDispatcherHandles.handles[idx].handle, handle) == 0)) {
      *structmsgDispatcher = structmsgDispatcherHandles.handles[idx].structmsgDispatcher;
      return STRUCT_DISP_ERR_OK;
    }
    idx++;
  }
  return STRUCT_DISP_ERR_HANDLE_NOT_FOUND;
}

// ================================= dumpHeaderParts ====================================

static uint8_t dumpHeaderParts(structmsgDispatcher_t *self, headerParts_t *hdr) {
  int idx;

  ets_printf("dumpHeaderParts:\n");
  ets_printf("headerParts1: from: 0x%04x to: 0x%04x totalLgth: %d u16CmdKey: 0x%04x u16CmdLgth: 0x%04x u16Crc: 0x%04x\n", hdr->hdrFromPart, hdr->hdrToPart, hdr->hdrTotalLgth, hdr->hdrU16CmdKey, hdr->hdrU16CmdLgth, hdr->hdrU16Crc);
  ets_printf("headerParts2: target: 0x%02x u8CmdKey: 0x%02x u8CmdLgth: %d u8Crc: 0x%02x offset: %d extra: %d\n", hdr->hdrTargetPart, hdr->hdrU8CmdKey, hdr->hdrU8CmdLgth, hdr->hdrU8Crc, hdr->hdrOffset, hdr->hdrExtraLgth);
  ets_printf("headerParts3: enc: %c handleType: %c\n", hdr->hdrEncryption, hdr->hdrHandleType);
  ets_printf("hdrFlags: ");
  if (hdr->hdrFlags & STRUCT_DISP_SEND_TO_APP) {
    ets_printf(" STRUCT_DISP_SEND_TO_APP");
  }
  if (hdr->hdrFlags & STRUCT_DISP_RECEIVE_FROM_APP) {
    ets_printf(" STRUCT_DISP_RECEIVE_FROM_APP");
  }
  if (hdr->hdrFlags & STRUCT_DISP_SEND_TO_UART) {
    ets_printf(" STRUCT_DISP_SEND_TO_UART");
  }
  if (hdr->hdrFlags & STRUCT_DISP_RECEIVE_FROM_UART) {
    ets_printf(" STRUCT_DISP_RECEIVE_FROM_UART");
  }
  if (hdr->hdrFlags & STRUCT_DISP_TRANSFER_TO_UART) {
    ets_printf(" STRUCT_DISP_TRANSFER_TO_UART");
  }
  if (hdr->hdrFlags & STRUCT_DISP_TRANSFER_TO_CONN) {
    ets_printf(" STRUCT_DISP_TRANSFER_TO_CONN");
  }
  if (hdr->hdrFlags & STRUCT_DISP_NOT_RELEVANT) {
    ets_printf(" STRUCT_DISP_NOT_RELEVANT");
  }
  if (hdr->hdrFlags & STRUCT_DISP_U8_CMD_KEY) {
    ets_printf(" STRUCT_DISP_U8_CMD_KEY");
  }
  if (hdr->hdrFlags & STRUCT_DISP_U16_CMD_KEY) {
    ets_printf(" STRUCT_DISP_U16_CMD_KEY");
  }
  if (hdr->hdrFlags & STRUCT_DISP_U0_CMD_LGTH) {
    ets_printf(" STRUCT_DISP_U0_CMD_LGTH");
  }
  if (hdr->hdrFlags & STRUCT_DISP_U8_CMD_LGTH) {
    ets_printf(" STRUCT_DISP_U8_CMD_LGTH");
  }
  if (hdr->hdrFlags & STRUCT_DISP_U16_CMD_LGTH) {
    ets_printf(" STRUCT_DISP_U16_CMD_LGTH");
  }
  if (hdr->hdrFlags & STRUCT_DISP_U0_CRC) {
    ets_printf(" STRUCT_DISP_U0_CRC");
  }
  if (hdr->hdrFlags & STRUCT_DISP_U8_CRC) {
    ets_printf(" STRUCT_DISP_U8_CRC");
  }
  if (hdr->hdrFlags & STRUCT_DISP_U16_CRC) {
    ets_printf(" STRUCT_DISP_U16_CRC");
  }
  ets_printf("\n");
  ets_printf("hdr fieldSequence\n");
  idx = 0;
  while (idx < 9) {
    ets_printf(" %d 0x%04x", idx, hdr->fieldSequence[idx]);
    if (hdr->fieldSequence[idx] & STRUCT_DISP_U16_DST) {
      ets_printf(" STRUCT_DISP_U16_DST");
    }
    if (hdr->fieldSequence[idx] & STRUCT_DISP_U16_SRC) {
      ets_printf(" STRUCT_DISP_U16_SRC");
    }
    if (hdr->fieldSequence[idx] & STRUCT_DISP_U8_TARGET) {
      ets_printf(" STRUCT_DISP_U8_TARGET");
    }
    if (hdr->fieldSequence[idx] & STRUCT_DISP_U16_TOTAL_LGTH) {
      ets_printf(" STRUCT_DISP_U16_TOTAL_LGTH");
    }
    if (hdr->fieldSequence[idx] & STRUCT_DISP_U8_EXTRA_KEY_LGTH) {
      ets_printf(" STRUCT_DISP_U8_EXTRA_KEY_LGTH");
    }
    if (hdr->fieldSequence[idx] & STRUCT_DISP_U8_ENCRYPTION) {
      ets_printf(" STRUCT_DISP_U8_ENCRYPTION");
    }
    if (hdr->fieldSequence[idx] & STRUCT_DISP_U8_HANDLE_TYPE) {
      ets_printf(" STRUCT_DISP_U8_HANDLE_TYPE");
    }
    if (hdr->fieldSequence[idx] & STRUCT_DISP_U8_CMD_KEY) {
      ets_printf(" STRUCT_DISP_U8_CMD_KEY");
    }
    if (hdr->fieldSequence[idx] & STRUCT_DISP_U16_CMD_KEY) {
      ets_printf(" STRUCT_DISP_U16_CMD_KEY");
    }
    if (hdr->fieldSequence[idx] & STRUCT_DISP_U0_CMD_LGTH) {
      ets_printf(" STRUCT_DISP_U0_CMD_LGTH");
    }
    if (hdr->fieldSequence[idx] & STRUCT_DISP_U8_CMD_LGTH) {
      ets_printf(" STRUCT_DISP_U8_CMD_LGTH");
    }
    if (hdr->fieldSequence[idx] & STRUCT_DISP_U16_CMD_LGTH) {
      ets_printf(" STRUCT_DISP_U16_CMD_LGTH");
    }
    if (hdr->fieldSequence[idx] & STRUCT_DISP_U0_CRC) {
      ets_printf(" STRUCT_DISP_U0_CRC");
    }
    if (hdr->fieldSequence[idx] & STRUCT_DISP_U8_CRC) {
      ets_printf(" STRUCT_DISP_U8_CRC");
    }
    if (hdr->fieldSequence[idx] & STRUCT_DISP_U16_CRC) {
      ets_printf(" STRUCT_DISP_U16_CRC");
    }
    ets_printf("\n");
    idx++;
  }
  ets_printf("\n");
  return STRUCT_DISP_ERR_OK;
}

// ================================= dumpMsgHeaderInfos ====================================

static uint8_t dumpMsgHeaderInfos(structmsgDispatcher_t *self, msgHeaderInfos_t *hdrInfos) {
  int idx;

  ets_printf("dumpMsgHeaderInfos:\n");
  ets_printf("headerFlags: ");
  if (hdrInfos->headerFlags & STRUCT_DISP_U16_SRC) {
    ets_printf(" STRUCT_DISP_U16_SRC");
  }
  if (hdrInfos->headerFlags & STRUCT_DISP_U16_DST) {
    ets_printf(" STRUCT_DISP_U16_DST");
  }
  if (hdrInfos->headerFlags & STRUCT_DISP_U16_TOTAL_LGTH) {
    ets_printf(" STRUCT_DISP_U16_TOTAL_LGTH");
  }
  if (hdrInfos->headerFlags & STRUCT_DISP_U8_TARGET) {
    ets_printf(" STRUCT_DISP_U8_TARGET");
  }
  ets_printf("\n");
  ets_printf("hdrInfos headerSequence\n");
  idx = 0;
  while (idx < 9) {
    ets_printf(" %d 0x%04x", idx, hdrInfos->headerSequence[idx]);
    if (hdrInfos->headerSequence[idx] & STRUCT_DISP_U16_DST) {
      ets_printf(" STRUCT_DISP_U16_DST");
    }
    if (hdrInfos->headerSequence[idx] & STRUCT_DISP_U16_SRC) {
      ets_printf(" STRUCT_DISP_U16_SRC");
    }
    if (hdrInfos->headerSequence[idx] & STRUCT_DISP_U8_TARGET) {
      ets_printf(" STRUCT_DISP_U8_TARGET");
    }
    if (hdrInfos->headerSequence[idx] & STRUCT_DISP_U16_TOTAL_LGTH) {
      ets_printf(" STRUCT_DISP_U16_TOTAL_LGTH");
    }
    if (hdrInfos->headerSequence[idx] & STRUCT_DISP_U8_EXTRA_KEY_LGTH) {
      ets_printf(" STRUCT_DISP_U8_EXTRA_KEY_LGTH");
    }
    if (hdrInfos->headerSequence[idx] & STRUCT_DISP_U8_ENCRYPTION) {
      ets_printf(" STRUCT_DISP_U8_ENCRYPTION");
    }
    if (hdrInfos->headerSequence[idx] & STRUCT_DISP_U8_HANDLE_TYPE) {
      ets_printf(" STRUCT_DISP_U8_HANDLE_TYPE");
    }
    if (hdrInfos->headerSequence[idx] & STRUCT_DISP_U8_CMD_KEY) {
      ets_printf(" STRUCT_DISP_U8_CMD_KEY");
    }
    if (hdrInfos->headerSequence[idx] & STRUCT_DISP_U16_CMD_KEY) {
      ets_printf(" STRUCT_DISP_U16_CMD_KEY");
    }
    if (hdrInfos->headerSequence[idx] & STRUCT_DISP_U0_CMD_LGTH) {
      ets_printf(" STRUCT_DISP_U0_CMD_LGTH");
    }
    if (hdrInfos->headerSequence[idx] & STRUCT_DISP_U8_CMD_LGTH) {
      ets_printf(" STRUCT_DISP_U8_CMD_LGTH");
    }
    if (hdrInfos->headerSequence[idx] & STRUCT_DISP_U16_CMD_LGTH) {
      ets_printf(" STRUCT_DISP_U16_CMD_LGTH");
    }
    if (hdrInfos->headerSequence[idx] & STRUCT_DISP_U0_CRC) {
      ets_printf(" STRUCT_DISP_U0_CRC");
    }
    if (hdrInfos->headerSequence[idx] & STRUCT_DISP_U8_CRC) {
      ets_printf(" STRUCT_DISP_U8_CRC");
    }
    if (hdrInfos->headerSequence[idx] & STRUCT_DISP_U16_CRC) {
      ets_printf(" STRUCT_DISP_U16_CRC");
    }
    ets_printf("\n");
    idx++;
  }
  ets_printf("\n");
  ets_printf("startLgth: %d numParts: %d maxParts: %d currPartIdx: %d seqIdx: %d seqIdxAfterStart: %d\n", hdrInfos->headerStartLgth, hdrInfos->numHeaderParts, hdrInfos->maxHeaderParts, hdrInfos->currPartIdx, hdrInfos->seqIdx, hdrInfos->seqIdxAfterStart);
  return STRUCT_DISP_ERR_OK;
}

// ================================= dumpMsgParts ====================================

static uint8_t dumpMsgParts(structmsgDispatcher_t *self, msgParts_t *msgParts) {
  int idx;

  ets_printf("dumpMsgParts:\n");
  ets_printf("MsgParts1 form: 0x%04x to: 0x%04x totalLgth: %d u16_cmdLgth: %d u16CmdKey: 0x%04x targetPart: 0x%02x\n", msgParts->fromPart, msgParts->toPart, msgParts->totalLgth, msgParts->u16CmdLgth, msgParts->u16CmdKey, msgParts->targetPart);

  ets_printf("MsgParts2 u8CmdLgth: %d u8CmdKey: 0x%02x lgth: %d fieldOffset: %d\n", msgParts->u8CmdLgth, msgParts->u8CmdKey, msgParts->lgth, msgParts->fieldOffset);
  ets_printf("buf");
  idx = 0;
  while (idx < msgParts->realLgth - 1) {
    ets_printf(" %d 0x%02x", idx, msgParts->buf[idx]);
    idx++;
  }
  ets_printf("\n");
  ets_printf("partFlags: ");
  if (msgParts->partsFlags & STRUCT_DISP_U8_CMD_KEY) {
    ets_printf(" STRUCT_DISP_U8_CMD_KEY");
  }
  if (msgParts->partsFlags & STRUCT_DISP_IS_NOT_ENCRYPTED) {
    ets_printf(" STRUCT_DISP_IS_NOT_ENCRYPTED");
  }
  if (msgParts->partsFlags & STRUCT_DISP_IS_ENCRYPTED) {
    ets_printf(" STRUCT_DISP_IS_ENCRYPTED");
  }
  if (msgParts->partsFlags & STRUCT_DISP_U16_CMD_KEY) {
    ets_printf(" STRUCT_DISP_U16_CMD_KEY");
  }
  if (msgParts->partsFlags & STRUCT_DISP_U8_CMD_KEY) {
    ets_printf(" STRUCT_DISP_U8_CMD_KEY");
  }
  ets_printf("\n");
  return STRUCT_DISP_ERR_OK;
}

// ================================= BMsg ====================================

static uint8_t BMsg(structmsgDispatcher_t *self) {
  return STRUCT_DISP_ERR_OK;
}

// ================================= IMsg ====================================

static uint8_t IMsg(structmsgDispatcher_t *self) {
  return STRUCT_DISP_ERR_OK;
}

// ================================= MMsg ====================================

static uint8_t MMsg(structmsgDispatcher_t *self) {
  return STRUCT_DISP_ERR_OK;
}

// ================================= defaultMsg ====================================

static uint8_t defaultMsg(structmsgDispatcher_t *self) {
  return STRUCT_DISP_ERR_OK;
}

#define checkErrWithResetOK(result) if(result != DATA_VIEW_ERR_OK) { dataView->data = saveData; dataView->lgth= saveLgth; return result; }

// ================================= getMsgPtrFromMsgParts ====================================

static uint8_t getMsgPtrFromMsgParts(structmsgDispatcher_t *self, msgParts_t *msgParts, structmsgData_t **structmsgData, int incrRefCnt) {
  int firstFreeEntryId;
  int headerIdx;
  msgHeader2MsgPtr_t *entry;
  msgHeader2MsgPtr_t *headerEntry;
  msgHeader2MsgPtr_t *newHeaderEntry;
  msgHeader2MsgPtr_t *firstFreeEntry;
  uint8_t header[DISP_MAX_HEADER_LGTH];
  uint8_t result;
  dataView_t *dataView;
  uint8_t *saveData;
  uint8_t saveLgth;
  size_t offset;

  // build header from msgParts
  offset = 0;
  dataView = self->structmsgDataView->dataView;
  saveData = dataView->data;
  saveLgth = dataView->lgth;
  dataView->data = header;
  dataView->lgth = DISP_MAX_HEADER_LGTH;
  result = dataView->setUint16(dataView, offset, msgParts->fromPart);
  checkErrWithResetOK(result);
  offset += sizeof(uint16_t);
  result = dataView->setUint16(dataView, offset, msgParts->toPart);
  checkErrWithResetOK(result);
  offset += sizeof(uint16_t);
  result = dataView->setUint16(dataView, offset, msgParts->totalLgth);
  checkErrWithResetOK(result);
  offset += sizeof(uint16_t);
  if (self->dispFlags & STRUCT_MSG_U8_CMD_KEY) {
    result = dataView->setUint8(dataView, offset, msgParts->u8CmdKey);
    checkErrWithResetOK(result);
    offset += sizeof(uint8_t);
  } else {
    result = dataView->setUint16(dataView, offset, msgParts->u16CmdKey);
    checkErrWithResetOK(result);
    offset += sizeof(uint16_t);
  }
  dataView->data = saveData;
  dataView->lgth= saveLgth;
  // end build header from msgParts
  if ((incrRefCnt == STRUCT_MSG_INCR) && (self->numMsgHeaders >= self->maxMsgHeaders)) {
    if (self->maxMsgHeaders == 0) {
      self->maxMsgHeaders = 4;
      self->msgHeader2MsgPtrs = (msgHeader2MsgPtr_t *)os_zalloc((self->maxMsgHeaders * sizeof(msgHeader2MsgPtr_t)));
      checkAllocOK(self->msgHeader2MsgPtrs);
    } else {
      self->maxMsgHeaders += 2;
      self->msgHeader2MsgPtrs = (msgHeader2MsgPtr_t *)os_realloc(self->msgHeader2MsgPtrs, (self->maxMsgHeaders * sizeof(msgHeader2MsgPtr_t)));
      checkAllocOK(self->msgHeader2MsgPtrs);
    }
  }
  firstFreeEntry = NULL;
  firstFreeEntryId = 0;
  if (self->numMsgHeaders > 0) {
    // find header 
    headerIdx = 0;
    while (headerIdx < self->numMsgHeaders) {
      headerEntry = &self->msgHeader2MsgPtrs[headerIdx];
      if ((headerEntry->structmsgData != NULL) && (ets_memcmp(headerEntry->header, header, offset) == 0)) {
        if (incrRefCnt < 0) {
          headerEntry->headerLgth = 0;
          os_free(headerEntry->structmsgData);
          headerEntry->structmsgData = NULL;
          *structmsgData = NULL;
          return STRUCT_DISP_ERR_OK;
        }
        *structmsgData = headerEntry->structmsgData;
        (*structmsgData)->flags &= ~STRUCT_MSG_IS_PREPARED;
        return STRUCT_DISP_ERR_OK;
      }
      if ((incrRefCnt == STRUCT_MSG_INCR) && (headerEntry->structmsgData == NULL) && (firstFreeEntry == NULL)) {
        firstFreeEntry = headerEntry;
        firstFreeEntry->structmsgData = newStructmsgData();
        firstFreeEntry->headerLgth = offset;
        c_memcpy(firstFreeEntry->header, header, offset);
        *structmsgData = firstFreeEntry->structmsgData;
      }
      headerIdx++;
    }
  }
  if (incrRefCnt < 0) {
    return STRUCT_DISP_ERR_OK; // just ignore silently
  } else {
    if (incrRefCnt == 0) {
      return STRUCT_DISP_ERR_HEADER_NOT_FOUND;
    } else {
      if (firstFreeEntry != NULL) {
        *structmsgData = firstFreeEntry->structmsgData;
        firstFreeEntry->headerLgth = offset;
        c_memcpy(firstFreeEntry->header, header, offset);
      } else {
        newHeaderEntry = &self->msgHeader2MsgPtrs[self->numMsgHeaders];
        newHeaderEntry->headerLgth = offset;
        c_memcpy(newHeaderEntry->header, header, offset);
        newHeaderEntry->structmsgData = newStructmsgData();
        *structmsgData = newHeaderEntry->structmsgData;
        self->numMsgHeaders++;
      }
    }
  }
  return STRUCT_DISP_ERR_OK;
}

#undef checkErrWithResetOK

// ================================= getFieldType ====================================

static uint8_t getFieldType(structmsgDispatcher_t *self, structmsgData_t *structmsgData, uint8_t fieldNameId, uint8_t *fieldTypeId) {
  int idx;
  structmsgField_t *fieldInfo;

  idx = 0;
  while (idx < structmsgData->numFields) {
    fieldInfo = &structmsgData->fields[idx];
    if (fieldInfo->fieldNameId == fieldNameId) {
      *fieldTypeId = fieldInfo->fieldTypeId;
      return STRUCT_DISP_ERR_OK;
    }
    idx++;
  }
  // and now check the table fields
  idx = 0;
  while (idx < structmsgData->numTableRowFields) {
    fieldInfo = &structmsgData->tableFields[idx];
    if (fieldInfo->fieldNameId == fieldNameId) {
      *fieldTypeId = fieldInfo->fieldTypeId;
      return STRUCT_DISP_ERR_OK;
    }
    idx++;
  }
  return STRUCT_DISP_ERR_FIELD_NOT_FOUND;
}

// ================================= getFieldInfoFromLine ====================================

static uint8_t getFieldInfoFromLine(structmsgDispatcher_t *self) {
  int idx;
  int result;
  unsigned long uval;
  uint8_t *buffer;
  uint8_t*cp;
  char *endPtr;
  uint8_t lgth;
  structmsgDataView_t *dataView;

  dataView = self->structmsgData->structmsgDataView;
  buffer = self->buildMsgInfos.buf;
  self->buildMsgInfos.numericValue = 0;
  self->buildMsgInfos.stringValue = NULL;
  result = self->readLine(self, &buffer, &lgth);
  checkErrOK(result);
  if (lgth == 0) {
    return STRUCT_DISP_ERR_TOO_FEW_FILE_LINES;
  }
  buffer[lgth] = 0;
  self->buildMsgInfos.fieldNameStr = buffer;

  // fieldName
  cp = self->buildMsgInfos.fieldNameStr;
  while (*cp != ',') {
    cp++;
  }
  *cp++ = '\0';
  result = dataView->getFieldNameIdFromStr(dataView, self->buildMsgInfos.fieldNameStr, &self->buildMsgInfos.fieldNameId, STRUCT_MSG_NO_INCR);
  checkErrOK(result);
  result = getFieldType(self, self->structmsgData, self->buildMsgInfos.fieldNameId, &self->buildMsgInfos.fieldTypeId);
  checkErrOK(result);

  // fieldValue
  self->buildMsgInfos.fieldValueStr = cp;
  while (*cp != '\n') {
    cp++;
  }
  *cp++ = '\0';
  if (self->buildMsgInfos.fieldValueStr[0] != '@') {
    switch (self->buildMsgInfos.fieldTypeId) {
    case DATA_VIEW_FIELD_UINT8_T:
    case DATA_VIEW_FIELD_INT8_T:
    case DATA_VIEW_FIELD_UINT16_T:
    case DATA_VIEW_FIELD_INT16_T:
    case DATA_VIEW_FIELD_UINT32_T:
    case DATA_VIEW_FIELD_INT32_T:
      {
        uval = c_strtoul(self->buildMsgInfos.fieldValueStr, &endPtr, 10);
        if (endPtr == (char *)(cp-1)) {
          self->buildMsgInfos.numericValue = (int)uval;
          self->buildMsgInfos.stringValue = NULL;
        } else {
          self->buildMsgInfos.numericValue = 0;
          self->buildMsgInfos.stringValue = self->buildMsgInfos.fieldValueStr;
        }
      }
      break;
    default:
      self->buildMsgInfos.numericValue = 0;
      self->buildMsgInfos.stringValue = self->buildMsgInfos.fieldValueStr;
      break;
    }
  }
  return STRUCT_MSG_ERR_OK;
}

// ================================= setMsgValuesFromLines ====================================

static uint8_t setMsgValuesFromLines(structmsgDispatcher_t *self, structmsgData_t *structmsgData, uint8_t numEntries, uint8_t *handle, uint8_t type) {
  int idx;
  int fieldIdx;
  int tableFieldIdx;
  uint8_t fieldLgth;
  uint8_t *flagStr;
  uint8_t flag;
  unsigned long uval;
  structmsgDataView_t *dataView;
  int result;
  structmsgField_t *fieldInfo;
  uint8_t numTableRows;
  uint8_t numTableRowFields;
  int currTableRow;
  int currTableCol;
  int msgCmdKey;

  idx = 0;
  dataView = structmsgData->structmsgDataView;
  // loop over MSG Fields, to check if we eventaully have table rows!!
  result = getFieldInfoFromLine(self);
  checkErrOK(result);
  idx++;
  fieldIdx = 0;
  tableFieldIdx = 0;
  numTableRows = 0;
  numTableRowFields = 0;
  currTableRow = 0;
  currTableCol = 0;
  structmsgData = self->structmsgData;
//ets_printf("setMsgValuesFromLines: numFields:%d \n", structmsgData->numFields);
  while ((fieldIdx < structmsgData->numFields) && (idx <= numEntries)) {
//ets_printf("setMsgValuesFromLines2: fieldIdx: %d atbeleFieldIdx: %d idx: %d numFields:%d \n", fieldIdx, tableFieldIdx, idx, structmsgData->numFields);
    if (numTableRows > 0) {
      fieldInfo = &structmsgData->tableFields[tableFieldIdx++];
    } else {
      fieldInfo = &structmsgData->fields[fieldIdx++];
    }
//ets_printf("fieldNameId: %d numtabrows: %d\n", fieldInfo->fieldNameId, numTableRows);
    switch (fieldInfo->fieldNameId) {
    case STRUCT_MSG_SPEC_FIELD_TABLE_ROW_FIELDS:
      numTableRows = structmsgData->numTableRows;
      numTableRowFields = structmsgData->numTableRowFields;
      currTableRow = 0;
      currTableCol = 0;
      break;
    default:
//ets_printf("default  fieldNameId: %d buildMsgInfo fieldNameId: %d\n", fieldInfo->fieldNameId, self->buildMsgInfos.fieldNameId);
    if (fieldInfo->fieldNameId == self->buildMsgInfos.fieldNameId) {
      if (self->buildMsgInfos.fieldValueStr[0] == '@') {
        // call the callback function for the field!!
        if (numTableRows > 0) {
          while (currTableRow < numTableRows) {
            self->buildMsgInfos.tableRow = currTableRow;
            self->buildMsgInfos.tableCol = currTableCol;
            result = self->fillMsgValue(self, self->buildMsgInfos.fieldValueStr, type, self->buildMsgInfos.fieldTypeId);
            checkErrOK(result);
            result = structmsgData->setTableFieldValue(structmsgData, self->buildMsgInfos.fieldNameStr, currTableRow, self->buildMsgInfos.numericValue, self->buildMsgInfos.stringValue);
            currTableRow++;
          }
          currTableRow = 0;
          currTableCol++;
          if (currTableCol > numTableRowFields) {
            // tabel rows done
            numTableRows = 0;
            numTableRowFields = 0;
            currTableRow = 0;
            currTableCol = 0;
          }
        } else {
          result = self->fillMsgValue(self, self->buildMsgInfos.fieldValueStr, type, self->buildMsgInfos.fieldTypeId);
          checkErrOK(result);
          result = structmsgData->setFieldValue(structmsgData, self->buildMsgInfos.fieldNameStr, self->buildMsgInfos.numericValue, self->buildMsgInfos.stringValue);
          currTableRow++;
        }
      } else {
        switch (self->buildMsgInfos.fieldNameId) {
          case STRUCT_MSG_SPEC_FIELD_DST:
            result = structmsgData->setFieldValue(structmsgData, self->buildMsgInfos.fieldNameStr, self->buildMsgInfos.numericValue, self->buildMsgInfos.stringValue);
            break;
          case STRUCT_MSG_SPEC_FIELD_SRC:
            result = structmsgData->setFieldValue(structmsgData, self->buildMsgInfos.fieldNameStr, self->buildMsgInfos.numericValue, self->buildMsgInfos.stringValue);
            break;
          case STRUCT_MSG_SPEC_FIELD_CMD_KEY:
            // check for u8CmdKey/u16CmdKey here
            if (self->dispFlags & STRUCT_MSG_U8_CMD_KEY) {
              self->buildMsgInfos.numericValue = self->received.u8CmdKey;
            } else {
ets_printf("cmdKey: 0x%04x\n", self->received.u16CmdKey);
              self->buildMsgInfos.numericValue = self->received.u16CmdKey;
            }
            self->buildMsgInfos.stringValue = NULL;
            result = structmsgData->setFieldValue(structmsgData, self->buildMsgInfos.fieldNameStr, self->buildMsgInfos.numericValue, self->buildMsgInfos.stringValue);
            break;
          default:
            result = structmsgData->setFieldValue(structmsgData, self->buildMsgInfos.fieldNameStr, self->buildMsgInfos.numericValue, self->buildMsgInfos.stringValue);
            break;
          }
        }
        checkErrOK(result);
        if (idx < numEntries) {
          result = getFieldInfoFromLine(self);
          checkErrOK(result);
        }
        idx++;
      }
      break;
    }
  }
  if (self->buildMsgInfos.partsFlags & STRUCT_DISP_U8_CMD_KEY) {
    msgCmdKey = type;
  } else {
    msgCmdKey = (self->buildMsgInfos.u16CmdKey & 0xFF00) | type & 0xFF;
  }
  result = structmsgData->setFieldValue(structmsgData, "@cmdKey", msgCmdKey, NULL);
  checkErrOK(result);
  structmsgData->prepareMsg(structmsgData);
  structmsgData->dumpMsg(structmsgData);
  return STRUCT_DISP_ERR_OK;
}

// ================================= createMsgFromLines ====================================

static uint8_t createMsgFromLines(structmsgDispatcher_t *self, msgParts_t *parts, uint8_t numEntries, uint8_t numRows, uint8_t type) {
  int idx;
  uint8_t*cp;
  uint8_t *fieldNameStr;
  uint8_t *fieldTypeStr;
  uint8_t *fieldLgthStr;
  char *endPtr;
  uint8_t fieldLgth;
  uint8_t *flagStr;
  uint8_t flag;
  uint8_t lgth;
  unsigned long uflag;
  uint8_t buf[100];
  uint8_t *buffer = buf;
  int result;

//ets_printf("§createMsgFromLines:%d!%d! \n§", self->numMsgHeaders, self->maxMsgHeaders);
  result = getMsgPtrFromMsgParts(self, parts, &self->structmsgData, STRUCT_MSG_INCR);
  checkErrOK(result);
  if (self->structmsgData->flags & STRUCT_MSG_IS_INITTED) {
    return STRUCT_DISP_ERR_OK;
  }
  result = self->structmsgData->createMsg(self->structmsgData, numEntries, &self->msgHandle);
  checkErrOK(result);
  idx = 0;
  while(idx < numEntries) {
    result = self->readLine(self, &buffer, &lgth);
    checkErrOK(result);
    if (lgth == 0) {
      return STRUCT_DISP_ERR_TOO_FEW_FILE_LINES;
    }
    buffer[lgth] = 0;
    fieldNameStr = buffer;
    cp = fieldNameStr;
    while (*cp != ',') {
      cp++;
    }
    *cp++ = '\0';
    fieldTypeStr = cp;
    while (*cp != ',') {
      cp++;
    }
    *cp++ = '\0';
    fieldLgthStr = cp;
    while (*cp != '\n') {
      cp++;
    }
    *cp++ = '\0';
    if (c_strcmp(fieldLgthStr,"@numRows") == 0) {
      fieldLgth = numRows;
    } else {
      lgth = c_strtoul(fieldLgthStr, &endPtr, 10);
      fieldLgth = (uint8_t)lgth;
    }
    result = self->structmsgData->addField(self->structmsgData, fieldNameStr, fieldTypeStr, fieldLgth);
    checkErrOK(result);
    idx++;
  }
  return STRUCT_DISP_ERR_OK;
}

// ================================= resetMsgInfo ====================================

static uint8_t resetMsgInfo(structmsgDispatcher_t *self, msgParts_t *parts) {
  parts->lgth = 0;
  parts->fieldOffset = 0;
  parts->fromPart = 0;
  parts->toPart = 0;
  parts->totalLgth = 0;
  parts->u16CmdLgth = 0;
  parts->u16CmdKey = 0;
  parts->u8CmdKey = 0;
  parts->u8CmdLgth = 0;
  self->structmsgDataView->dataView->data = parts->buf;
  self->structmsgDataView->dataView->lgth = 0;
  return STRUCT_DISP_ERR_OK;
}

// ============================= encryptMsg ========================

static uint8_t encryptMsg(const uint8_t *msg, size_t mlen, const uint8_t *key, size_t klen, const uint8_t *iv, size_t ivlen, uint8_t **buf, int *lgth) {
  const crypto_mech_t *mech;
  size_t bs;
  size_t clen;
  uint8_t *crypted;

  *buf = NULL;
  *lgth = 0;
  mech = crypto_encryption_mech ("AES-CBC");
  if (mech == NULL) {
    return STRUCT_MSG_ERR_CRYPTO_BAD_MECHANISM;
  }
  bs = mech->block_size;
  clen = ((mlen + bs - 1) / bs) * bs;
  *lgth = clen;
//ets_printf("dlen: %d lgth: %d clen: %d data: %p\n", dlen, *lgth, clen, data);
  crypted = (uint8_t *)os_zalloc (clen);
  if (!crypted) {
    return STRUCT_MSG_ERR_CRYPTO_INIT_FAILED;
  } 
  *buf = crypted;
  crypto_op_t op =
  { 
    key, klen,
    iv, ivlen,
    msg, mlen,
    crypted, clen,
    OP_ENCRYPT
  }; 
  if (!mech->run (&op)) { 
    os_free (*buf);
    return STRUCT_MSG_ERR_CRYPTO_INIT_FAILED;
  } 
  return STRUCT_MSG_ERR_OK;
}

// ============================= decryptMsg ========================

static uint8_t decryptMsg(const uint8_t *msg, size_t mlen, const uint8_t *key, size_t klen, const uint8_t *iv, size_t ivlen, uint8_t **buf, int *lgth) {
  const crypto_mech_t *mech;
  size_t bs;
  size_t clen;
  uint8_t *crypted;

  *buf = NULL;
  *lgth = 0;
  mech = crypto_encryption_mech ("AES-CBC");
  if (mech == NULL) {
    return STRUCT_MSG_ERR_CRYPTO_BAD_MECHANISM;
  }
  bs = mech->block_size;
  clen = ((mlen + bs - 1) / bs) * bs;
  *lgth = clen;
  crypted = (uint8_t *)os_zalloc (*lgth);
  if (!crypted) {
    return STRUCT_MSG_ERR_CRYPTO_INIT_FAILED;
  } 
  *buf = crypted;
  crypto_op_t op =
  { 
    key, klen,
    iv, ivlen,
    msg, mlen,
    crypted, clen,
    OP_DECRYPT
  }; 
  if (!mech->run (&op)) { 
    os_free (*buf);
    return STRUCT_MSG_ERR_CRYPTO_INIT_FAILED;
  }
  return STRUCT_MSG_ERR_OK;
}

// ================================= openFile ====================================

static uint8_t openFile(structmsgDispatcher_t *self, const uint8_t *fileName, const uint8_t *fileMode) {
  return self->structmsgDataDescription->openFile(self->structmsgDataDescription, fileName, fileMode);
}

// ================================= closeFile ====================================

static uint8_t closeFile(structmsgDispatcher_t *self) {
  return self->structmsgDataDescription->closeFile(self->structmsgDataDescription);
}

// ================================= flushFile ====================================

static uint8_t flushFile(structmsgDispatcher_t *self) {
  return STRUCT_DATA_DESC_ERR_FLUSH_FILE;
}

// ================================= readLine ====================================

static uint8_t readLine(structmsgDispatcher_t *self, uint8_t **buffer, uint8_t *lgth) {
  return self->structmsgDataDescription->readLine(self->structmsgDataDescription, buffer, lgth);
}

// ================================= writeLine ====================================

static uint8_t writeLine(structmsgDispatcher_t *self, const uint8_t *buffer, uint8_t lgth) {
  return self->structmsgDataDescription->writeLine(self->structmsgDataDescription, buffer, lgth);
}

// ============================= structmsgDispatcherGetPtrFromHandle ========================

uint8_t structmsgDispatcherGetPtrFromHandle(const char *handle, structmsgDispatcher_t **structmsgDispatcher) {

  if (checkHandle(handle, structmsgDispatcher) != STRUCT_DISP_ERR_OK) {
    return STRUCT_DISP_ERR_HANDLE_NOT_FOUND;
  }
  return STRUCT_DISP_ERR_OK;
}

// ================================= initDispatcher ====================================

static uint8_t initDispatcher(structmsgDispatcher_t *self) {
  uint8_t result;

  result = structmsgIdentifyInit(self);
  checkErrOK(result);
  result = structmsgSendReceiveInit(self);
  checkErrOK(result);
  result = structmsgActionInit(self);
  checkErrOK(result);
  result = structmsgModuleDataValuesInit(self);
  checkErrOK(result);
  result = structmsgWebsocketInit(self);
  checkErrOK(result);
  return STRUCT_DISP_ERR_OK;
}

// ================================= createDispatcher ====================================

static uint8_t createDispatcher(structmsgDispatcher_t *self, uint8_t **handle) {
  uint8_t result;

  os_sprintf(self->handle, "%s%p", DISP_HANDLE_PREFIX, self);
  result = addHandle(self->handle, self);
  if (result != STRUCT_DISP_ERR_OK) {
    deleteHandle(self->handle);
    os_free(self);
    return result;
  }
  resetMsgInfo(self, &self->received);
  resetMsgInfo(self, &self->toSend);
  *handle = self->handle;
  return STRUCT_DISP_ERR_OK;
}

// ================================= newStructmsgDispatcher ====================================

structmsgDispatcher_t *newStructmsgDispatcher() {
  if (structmsgDispatcherSingleton != NULL) {
    return structmsgDispatcherSingleton;
  }
  structmsgDispatcher_t *structmsgDispatcher = os_zalloc(sizeof(structmsgDispatcher_t));
  if (structmsgDispatcher == NULL) {
    return NULL;
  }
  structmsgDispatcher->structmsgDataView = newStructmsgDataView();
  if (structmsgDispatcher->structmsgDataView == NULL) {
    return NULL;
  }

  structmsgDispatcherId++;
  structmsgDispatcher->id = structmsgDispatcherId;

  structmsgDispatcher->numMsgHeaders = 0;
  structmsgDispatcher->maxMsgHeaders = 0;
  structmsgDispatcher->msgHeader2MsgPtrs = NULL;

  structmsgDispatcher->msgHeaderInfos.headerFlags = 0;
  structmsgDispatcher->msgHeaderInfos.headerParts = NULL;
  structmsgDispatcher->msgHeaderInfos.numHeaderParts = 0;
  structmsgDispatcher->msgHeaderInfos.maxHeaderParts = 0;

  structmsgDispatcher->structmsgDataDescription = newStructmsgDataDescription();

  structmsgDispatcher->createDispatcher = &createDispatcher;
  structmsgDispatcher->initDispatcher = &initDispatcher;

//  structmsgDispatcher->BMsg = &BMsg;
//  structmsgDispatcher->IMsg = &IMsg;
//  structmsgDispatcher->MMsg = &MMsg;
//  structmsgDispatcher->defaultMsg = &defaultMsg;
  structmsgDispatcher->resetMsgInfo = &resetMsgInfo;
  structmsgDispatcher->createMsgFromLines = &createMsgFromLines;
  structmsgDispatcher->setMsgValuesFromLines = &setMsgValuesFromLines;

  structmsgDispatcher->openFile = &openFile;
  structmsgDispatcher->closeFile = &closeFile;
  structmsgDispatcher->readLine = &readLine;
  structmsgDispatcher->writeLine = &writeLine;

  structmsgDispatcher->encryptMsg = &encryptMsg;
  structmsgDispatcher->decryptMsg = &decryptMsg;
  structmsgDispatcher->toBase64 = &toBase64;
  structmsgDispatcher->fromBase64 = &fromBase64;

  structmsgDispatcher->dumpHeaderParts = &dumpHeaderParts;
  structmsgDispatcher->dumpMsgHeaderInfos = &dumpMsgHeaderInfos;
  structmsgDispatcher->dumpMsgParts = &dumpMsgParts;

  structmsgDispatcherSingleton = structmsgDispatcher;
  return structmsgDispatcher;
}

// ================================= freeStructmsgDispatcher ====================================

void freeStructmsgDispatcher(structmsgDispatcher_t *structmsgDispatcher) {
}


