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
 * File:   compMsgIdentify.c
 * Author: Arnulf P. Wiedemann
 *
 * Created on October 6st, 2016
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

#define MSG_HEADS_FILE_NAME "CompMsgHeads.txt"

#define DISP_FLAG_SHORT_CMD_KEY    (1 << 0)
#define DISP_FLAG_HAVE_CMD_LGTH    (1 << 1)
#define DISP_FLAG_IS_ENCRYPTED     (1 << 2)
#define DISP_FLAG_IS_TO_WIFI_MSG   (1 << 3)
#define DISP_FLAG_IS_FROM_MCU_MSG  (1 << 4)

#define RECEIVED_CHECK_TO_SIZE            2
#define RECEIVED_CHECK_FROM_SIZE          4
#define RECEIVED_CHECK_TOTAL_LGTH_SIZE    6
#define RECEIVED_CHECK_SHORT_CMD_KEY_SIZE 7
#define RECEIVED_CHECK_CMD_KEY_SIZE       8
#define RECEIVED_CHECK_CMD_LGTH_SIZE      10

typedef struct flag2Str {
  uint32_t flagVal;
  uint8_t *flagStr;
} flag2Str_t;

static flag2Str_t flag2Strs [] = {
  { COMP_DISP_U16_DST,           "COMP_DISP_U16_DST" },
  { COMP_DISP_U16_SRC,           "COMP_DISP_U16_SRC" },
  { COMP_DISP_U16_TOTAL_LGTH,    "COMP_DISP_U16_TOTAL_LGTH" },
  { COMP_DISP_U8_VECTOR_GUID,    "COMP_DISP_U8_VECTOR_GUID" },
  { COMP_DISP_U16_SRC_ID,        "COMP_DISP_U16_SRC_ID" },
  { COMP_DISP_IS_ENCRYPTED,      "COMP_DISP_IS_ENCRYPTED" },
  { COMP_DISP_IS_NOT_ENCRYPTED,  "COMP_DISP_IS_NOT_ENCRYPTED" },
  { COMP_DISP_U16_CMD_KEY,       "COMP_DISP_U16_CMD_KEY" },
  { COMP_DISP_U0_CMD_LGTH,       "COMP_DISP_U0_CMD_LGTH" },
  { COMP_DISP_U8_CMD_LGTH,       "COMP_DISP_U8_CMD_LGTH" },
  { COMP_DISP_U16_CMD_LGTH,      "COMP_DISP_U16_CMD_LGTH" },
  { 0,                             NULL },
};

// ================================= getFlagStr ====================================

static uint8_t *getFlagStr(uint32_t flags) {
  flag2Str_t *entry;
  int idx;
  
  idx = 0;
  while (1) {
    entry = &flag2Strs[idx];
    if (entry->flagStr == NULL) {
      return "??";
    }
    if (flags & entry->flagVal) {
      return entry->flagStr;
    }
    idx++;
  }
}

// ================================= initHeadersAndFlags ====================================

static uint8_t initHeadersAndFlags(compMsgDispatcher_t *self) {
  int result;

  self->dispFlags = 0;
  self->McuPart = 0x4D00;
  self->WifiPart = 0x5700;
  self->AppPart = 0x4100;
  self->CloudPart = 0x4300;
  return COMP_DISP_ERR_OK;
}

// ================================= resetHeaderInfos ====================================

static uint8_t resetHeaderInfos(compMsgDispatcher_t *self) {
  msgHeaderInfos_t *hdrInfos;

  hdrInfos = &self->msgHeaderInfos;
  hdrInfos->seqIdx = 0;
  hdrInfos->seqIdxAfterHeader = 0;
  hdrInfos->currPartIdx = 0;
  return COMP_DISP_ERR_OK;
}

// ================================= nextFittingEntry ====================================

static uint8_t nextFittingEntry(compMsgDispatcher_t *self, uint8_t u8CmdKey, uint16_t u16CmdKey) {
  msgParts_t *received;
  msgHeaderInfos_t *hdrInfos;
  dataView_t *dataView;
  headerPart_t *hdr;
  int hdrIdx;
  int found;

  dataView = self->compMsgDataView->dataView;
  received = &self->received;
  hdrInfos = &self->msgHeaderInfos;
  hdrIdx = hdrInfos->currPartIdx;
  hdr = &hdrInfos->headerParts[hdrIdx];
//ets_printf("§HEAD:!0x%04x!0x%04x!0x%04x!nH!%d!seqIdx!%d!§", received->toPart, received->fromPart, received->totalLgth, hdrInfos->numHeaderParts, hdrInfos->seqIdx);
  // and now search in the headers to find the appropriate message
  hdrInfos->seqIdx = hdrInfos->seqIdxAfterHeader;
  found = 0;
  while (hdrIdx < hdrInfos->numHeaderParts) {
    hdr = &hdrInfos->headerParts[hdrIdx];
    if (hdr->hdrToPart == received->toPart) {
      if (hdr->hdrFromPart == received->fromPart) {
        if ((hdr->hdrTotalLgth == received->totalLgth) || (hdr->hdrTotalLgth == 0)) {
          if (u16CmdKey != 0) {
            if (u16CmdKey == received->u16CmdKey) {
              found = 1;
              break;
            }
          } else {
            found = 1;
            break;
          }
        }
      }
    }
    hdrIdx++;
  }
  if (!found) {
    return COMP_DISP_ERR_HANDLE_NOT_FOUND;
  }
  hdrInfos->currPartIdx = hdrIdx;
  // next sequence field is extraLgth (skip, we have it in hdr fields)
  hdrInfos->seqIdx++;
  // next sequence field is encryption (skip, we have it in hdr fields)
  hdrInfos->seqIdx++;
  // next sequence field is handle type (skip, we have it in hdr fields)
  hdrInfos->seqIdx++;
  received->encryption = hdr->hdrEncryption;
ets_printf("§nextFitting!found!%d!hdrIdx!%d§\n", found, hdrIdx);
  return COMP_DISP_ERR_OK;
}

// ================================= getHeaderIndexFromHeaderFields ====================================

static uint8_t getHeaderIndexFromHeaderFields(compMsgDispatcher_t *self, msgParts_t *received, msgHeaderInfos_t *hdrInfos) {
  int result;
  dataView_t *dataView;
  headerPart_t *hdr;
  int hdrIdx;
  int found;
  uint8_t myHeaderLgth;

  dataView = self->compMsgDataView->dataView;
  received->fieldOffset = 0;
  myHeaderLgth = 0;
  hdrInfos->seqIdx = 0;
  switch(hdrInfos->headerSequence[hdrInfos->seqIdx]) {
  case COMP_DISP_U16_DST:
    result = dataView->getUint16(dataView, received->fieldOffset, &received->toPart);
    checkErrOK(result);
    received->fieldOffset += sizeof(uint16_t);
    break;
  case COMP_DISP_U16_SRC:
    result = dataView->getUint16(dataView, received->fieldOffset, &received->fromPart);
    checkErrOK(result);
    received->fieldOffset += sizeof(uint16_t);
    break;
  }
  hdrInfos->seqIdx++;
  switch(hdrInfos->headerSequence[hdrInfos->seqIdx]) {
  case COMP_DISP_U16_DST:
    result = dataView->getUint16(dataView, received->fieldOffset, &received->toPart);
    checkErrOK(result);
    received->fieldOffset += sizeof(uint16_t);
    break;
  case COMP_DISP_U16_SRC:
    result = dataView->getUint16(dataView, received->fieldOffset, &received->fromPart);
    checkErrOK(result);
    received->fieldOffset += sizeof(uint16_t);
    break;
  case COMP_DISP_U16_TOTAL_LGTH:
    result = dataView->getUint16(dataView, received->fieldOffset, &received->totalLgth);
    checkErrOK(result);
    received->fieldOffset += sizeof(uint16_t);
    break;
  }
  if (received->fieldOffset < hdrInfos->headerLgth) {
    hdrInfos->seqIdx++;
    switch(hdrInfos->headerSequence[hdrInfos->seqIdx]) {
    case COMP_DISP_U16_TOTAL_LGTH:
      result = dataView->getUint16(dataView, received->fieldOffset, &received->totalLgth);
      checkErrOK(result);
      received->fieldOffset += sizeof(uint16_t);
      break;
    }
  }
  if (received->fieldOffset < hdrInfos->headerLgth) {
    hdrInfos->seqIdx++;
    switch(hdrInfos->headerSequence[hdrInfos->seqIdx]) {
    case COMP_DISP_U8_VECTOR_GUID:
    {
      uint8_t *cp;
      cp = received->GUID;
      result = dataView->getUint8Vector(dataView, received->fieldOffset, &cp, sizeof(received->GUID));
      checkErrOK(result);
      received->fieldOffset += sizeof(received->GUID);
      break;
    }
    }
  }
  if (received->fieldOffset < hdrInfos->headerLgth) {
    hdrInfos->seqIdx++;
    switch(hdrInfos->headerSequence[hdrInfos->seqIdx]) {
    case COMP_DISP_U16_SRC_ID:
      result = dataView->getUint16(dataView, received->fieldOffset, &received->srcId);
      checkErrOK(result);
      received->fieldOffset += sizeof(uint16_t);
      break;
    }
  }
  hdrInfos->seqIdx++;
  hdrInfos->seqIdxAfterHeader = hdrInfos->seqIdx;
  hdrInfos->currPartIdx = 0;
  result = nextFittingEntry(self, 0, 0);
ets_printf("§IndexFromHeaderFields!result!%d!currPartIdx!%d!§\n", result, hdrInfos->currPartIdx);
  return result;
}

// ================================= handleReceivedMsg ====================================
    
static uint8_t handleReceivedMsg(compMsgDispatcher_t *self, msgParts_t *received, msgHeaderInfos_t *hdrInfos) {
  int result;
  dataView_t *dataView;
  headerPart_t *hdr;
  int hdrIdx;
  uint8_t answerType;
  compMsgField_t fieldInfo;
  uint16_t u16;
  uint8_t u8;
  uint8_t startOffset;
  uint16_t sequenceEntry;

  dataView = self->compMsgDataView->dataView;
  hdrIdx = hdrInfos->currPartIdx;
  hdr = &hdrInfos->headerParts[hdrIdx];

  // set received->lgth to end of the header
  received->lgth = hdrInfos->headerLgth;
  // we loop over the fieldSequence entries and handle them as needed
  // attention not all entries of the message are handled here, only some special entries!
 
  received->fieldOffset = hdrInfos->headerLgth;
  while (hdr->fieldSequence[hdrInfos->seqIdx] != 0) {
    sequenceEntry = hdr->fieldSequence[hdrInfos->seqIdx];
    switch (sequenceEntry) {
    case COMP_DISP_U16_CMD_KEY:
      result = self->compMsgDataView->dataView->getUint16(self->compMsgDataView->dataView, received->fieldOffset, &u16);
      received->u16CmdKey = u16;
      received->fieldOffset += 2;
      received->partsFlags |= COMP_DISP_U16_CMD_KEY;
//ets_printf("§u16CmdKey!0x%04x!§\n", received->u16CmdKey);
      while (received->u16CmdKey != hdr->hdrU16CmdKey) {
        hdrInfos->currPartIdx++;
        result = self->nextFittingEntry(self, 0, received->u16CmdKey);
        checkErrOK(result);
        hdr = &hdrInfos->headerParts[hdrInfos->currPartIdx];
      }
      break;
    case COMP_DISP_U0_CMD_LGTH:
      received->fieldOffset += 2;
//ets_printf("§u0CmdLgth!0!§\n");
      break;
    case COMP_DISP_U16_CMD_LGTH:
      result = self->compMsgDataView->dataView->getUint16(self->compMsgDataView->dataView, received->fieldOffset, &u16);
      received->u16CmdLgth = u16;
      received->fieldOffset += 2;
//puts stderr [format "§u16CmdLgth!%c!§" [dict get $received u16CmdLgth]
      break;
    case COMP_DISP_U0_CRC:
//ets_printf("§u0Crc!0!§");
      result = COMP_MSG_ERR_OK;
      break;
    case COMP_DISP_U8_CRC:
      fieldInfo.fieldLgth = 1;
      fieldInfo.fieldOffset = received->totalLgth - 1;
      startOffset = hdrInfos->headerLgth;
      result = self->compMsgDataView->getCrc(self->compMsgDataView, &fieldInfo, startOffset, fieldInfo.fieldOffset);
//ets_printf("§u8Crc!res!%d!§", result);
      break;
    case COMP_DISP_U16_CRC:
      fieldInfo.fieldLgth = 2;
      fieldInfo.fieldOffset = received->totalLgth - 2;
      startOffset = hdrInfos->headerLgth;
      result = self->compMsgDataView->getCrc(self->compMsgDataView, &fieldInfo, startOffset, fieldInfo.fieldOffset);
//ets_printf("§u16Crc!res!%d!§", result);
      break;
    }
    hdrInfos->seqIdx++;
  }
#ifdef DOACTIONS
        if (result != COMP_MSG_ERR_OK) {
// FIXME !! TEMPORARY!!
//          answerType = 'N';
          answerType = 'A';
        } else {
          answerType = 'A';
        }
      result = self->runAction(self, &answerType);
      checkErrOK(result);
      result = self->prepareNotEncryptedAnswer(self, received, answerType);
//ets_printf("handleEncryptedPart runAction: %c\n", answerType);
//FIXME !!!
//        result = prepareEncryptedAnswer(self, received, answerType);
//        checkErrOK(result);
//FIXME !!!
#endif
  result = -self->resetMsgInfo(self, received);
  checkErrOK(result);
  return COMP_MSG_ERR_OK;
}

// ================================= handleReceivedPart ====================================

static uint8_t handleReceivedPart(compMsgDispatcher_t *self, const uint8_t * buffer, uint8_t lgth) {
  int idx;
  msgParts_t *received;
  dataView_t *dataView;
  msgHeaderInfos_t *hdrInfos;
  headerPart_t *hdr;
  int startIdx;
  int hdrIdx;
  uint8_t u8;
  compMsgField_t fieldInfo;
  int result;

  hdrInfos = &self->msgHeaderInfos;
  received = &self->received;
  dataView = self->compMsgDataView->dataView;
  dataView->data = received->buf;
//ets_printf("§receivedLgth: %d fieldOffset: %d!\n§", received->lgth, received->fieldOffset);
  idx = 0;
  while (idx < lgth) {
    received->buf[received->lgth++] = buffer[idx];
    received->realLgth++;
    dataView->lgth++;
    if (received->lgth == hdrInfos->headerLgth) {
      result = getHeaderIndexFromHeaderFields(self, received, hdrInfos);
//ets_printf("getHeaderIndexFromHeaderFields result: %d currPartIdx: %d\n", result, hdrInfos->currPartIdx);
    }
    // loop until we have full message then decrypt if necessary and then handle the message
    if (received->lgth == received->totalLgth) {
      hdrIdx = hdrInfos->currPartIdx;
      hdr = &hdrInfos->headerParts[hdrIdx];
      if (hdr->hdrEncryption == 'E') {
        uint8_t *cryptedPtr;
        uint8_t *cryptKey;
        uint8_t *decrypted;;
        uint8_t mlen;
        uint8_t klen;
        uint8_t ivlen;
        int decryptedLgth;

        // decrypt encrypted message part (after header)
cryptKey = "a1b2c3d4e5f6g7h8";
        mlen = received->totalLgth - hdrInfos->headerLgth;
        ivlen = 16;
        klen = 16;
        cryptedPtr = received->buf + hdrInfos->headerLgth;
        result = self->decryptMsg(cryptedPtr, mlen, cryptKey, klen, cryptKey, ivlen, &decrypted, &decryptedLgth);
        checkErrOK(result);
        c_memcpy(cryptedPtr, decrypted, decryptedLgth);
      }
      result = self->handleReceivedMsg(self, received, hdrInfos);
ets_printf("handleReceivedMsg end idx: %d result: %d\n", idx, result);
      return result;
    }
    idx++;
  }
  return COMP_DISP_ERR_OK;
}

// ================================= compMsgIdentifyInit ====================================

uint8_t compMsgIdentifyInit(compMsgDispatcher_t *self) {
  uint8_t result;

  self->resetHeaderInfos = &resetHeaderInfos;
  self->handleReceivedPart = &handleReceivedPart;
  self->nextFittingEntry = &nextFittingEntry;
  self->handleReceivedMsg = &handleReceivedMsg;
  initHeadersAndFlags(self);
  result=self->compMsgMsgDesc->readHeadersAndSetFlags(self, MSG_HEADS_FILE_NAME);
  checkErrOK(result);
  return COMP_DISP_ERR_OK;
}
