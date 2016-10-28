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
  { COMP_DISP_U8_TARGET,         "COMP_DISP_U8_TARGET" },
  { COMP_DISP_U16_TOTAL_LGTH,    "COMP_DISP_U16_TOTAL_LGTH" },
  { COMP_DISP_U8_EXTRA_KEY_LGTH, "COMP_DISP_U8_EXTRA_KEY_LGTH" },
  { COMP_DISP_IS_ENCRYPTED,      "COMP_DISP_IS_ENCRYPTED" },
  { COMP_DISP_IS_NOT_ENCRYPTED,  "COMP_DISP_IS_NOT_ENCRYPTED" },
  { COMP_DISP_SEND_TO_APP,       "COMP_DISP_SEND_TO_APP" },
  { COMP_DISP_RECEIVE_FROM_APP,  "COMP_DISP_RECEIVE_FROM_APP" },
  { COMP_DISP_SEND_TO_UART,      "COMP_DISP_SEND_TO_UART" },
  { COMP_DISP_RECEIVE_FROM_UART, "COMP_DISP_RECEIVE_FROM_UART" },
  { COMP_DISP_TRANSFER_TO_UART,  "COMP_DISP_TRANSFER_TO_UART" },
  { COMP_DISP_TRANSFER_TO_CONN,  "COMP_DISP_TRANSFER_TO_CONN" },
  { COMP_DISP_U8_CMD_KEY,        "COMP_DISP_U8_CMD_KEY" },
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
  uint8_t fileName[30];
  int result;
  uint8_t numEntries;
  uint8_t fieldNameId;
  uint8_t lgth;
  uint8_t buf[100];
  uint8_t *buffer = buf;
  long ulgth;
  uint8_t *myStr;
  int idx;
  int seqIdx = 0;
  int seqIdx2 = 0;
  uint8_t*cp;
  headerPart_t *hdr;
  compMsgDataView_t *dataView;

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
          if (u8CmdKey != 0) {
            if (u8CmdKey == received->u8CmdKey) {
              found = 1;
              break;
            }
          } else {
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
  if (hdr->hdrEncryption == 'N') {
    received->partsFlags |= COMP_DISP_IS_NOT_ENCRYPTED;
    // skip extraLgth, encrypted and handle Type
  } else {
    received->partsFlags |= COMP_DISP_IS_ENCRYPTED;
  }
//ets_printf("§found!%d!hdrIdx!%d§\n", found, hdrIdx);
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
  case COMP_DISP_U8_TARGET:
    result = dataView->getUint8(dataView, received->fieldOffset, &received->targetPart);
    checkErrOK(result);
    received->fieldOffset += sizeof(uint8_t);
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
  hdrInfos->seqIdx++;
  hdrInfos->seqIdxAfterHeader = hdrInfos->seqIdx;
  hdrInfos->currPartIdx = 0;
  result = nextFittingEntry(self, 0, 0);
//ets_printf("§IndexFromHeaderFields!%d!%d!§\n", result, hdrInfos->currPartIdx);
  return result;
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
    if (received->lgth > hdrInfos->headerLgth) {
      hdrIdx = hdrInfos->currPartIdx;
      hdr = &hdrInfos->headerParts[hdrIdx];
      if (received->partsFlags & COMP_DISP_IS_NOT_ENCRYPTED) {
ets_printf("COMP_DISP_IS_NOT_ENCRYPTED\n");
        if (hdr->hdrEncryption == 'N') {
          result = self->handleNotEncryptedPart(self, received, hdrInfos);
          checkErrOK(result);
        } else {
ets_printf("partsFlags is not encrypted and hdrEncryption is E\n");
        }
      } else {
        if (hdr->hdrEncryption == 'E') {
          if (received->lgth == received->totalLgth) {
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

              // set received->lgth to end of header for correct working of handleEncryptedPart!!
              received->lgth = hdrInfos->headerLgth;
              result = self->handleEncryptedPart(self, received, hdrInfos);
ets_printf("handleEncryptedPart end idx: %d result: %d\n", idx, result);
              checkErrOK(result);
          }
        } else {
ets_printf("partsFlags is encrypted and hdrEncryption is N\n");
        }
      }
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
  initHeadersAndFlags(self);
  result=self->compMsgMsgDesc->readHeadersAndSetFlags(self, MSG_HEADS_FILE_NAME);
  checkErrOK(result);
return COMP_DISP_ERR_OK;
}
