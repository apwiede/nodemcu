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
//  dataView_t *dataView;
  headerPart_t *hdr;
  int hdrIdx;
  int found;

  received = &self->compMsgData->received;
  hdrInfos = &self->msgHeaderInfos;
  hdrIdx = hdrInfos->currPartIdx;
  hdr = &hdrInfos->headerParts[hdrIdx];
//ets_printf("§HEAD:!to:0x%04x!from:0x%04x!totalLgth:0x%04x!seqIdx!%d!§", received->toPart, received->fromPart, received->totalLgth, hdrInfos->seqIdx);
  // and now search in the headers to find the appropriate message
  hdrInfos->seqIdx = hdrInfos->seqIdxAfterHeader;
  found = 0;
  while (hdrIdx < hdrInfos->numHeaderParts) {
    hdr = &hdrInfos->headerParts[hdrIdx];
//ets_printf("hdrIdx: %d to: %d %d\n", hdrIdx, hdr->hdrToPart, received->toPart);
    if (hdr->hdrToPart == received->toPart) {
//ets_printf("to: %d from: %d %d\n", hdr->hdrToPart, hdr->hdrFromPart, received->fromPart);
      if (hdr->hdrFromPart == received->fromPart) {
//ets_printf("lgth: hdr: %d received: %d\n", hdr->hdrTotalLgth, received->totalLgth);
        if ((hdr->hdrTotalLgth == received->totalLgth) || (hdr->hdrTotalLgth == 0)) {
//ets_printf("§cmdKey: 0x%04x 0x%04x§", u16CmdKey, received->u16CmdKey);
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
ets_printf("§nextFitting HANDLE_NOT_FOUND§");
    return COMP_DISP_ERR_HANDLE_NOT_FOUND;
  }
  hdrInfos->currPartIdx = hdrIdx;
//ets_printf("§encryption: %c handleType: %c§", hdr->hdrEncryption, hdr->hdrHandleType);
  received->encryption = hdr->hdrEncryption;
//ets_printf("§nextFitting!found!%d!hdrIdx!%d!hdr->cmdKey:0x%04x!received->cmdKey: 0x%04x§", found, hdrIdx, hdr->hdrU16CmdKey, received->u16CmdKey);
  return COMP_DISP_ERR_OK;
}

// ================================= getHeaderIndexFromHeaderFields ====================================

static uint8_t getHeaderIndexFromHeaderFields(compMsgDispatcher_t *self, msgHeaderInfos_t *hdrInfos) {
  int result;
  dataView_t *dataView;
  headerPart_t *hdr;
  int hdrIdx;
  int found;
  uint8_t myHeaderLgth;
  uint16_t seqVal;
  msgParts_t *received;
  uint8_t *cp;
  uint8_t lgth;

  received = &self->compMsgData->received;
//ets_printf("§getHeaderIndexFromHeaderFields newCompMsgDataView§");
  received->compMsgDataView = newCompMsgDataView(received->buf, received->lgth);
  checkAllocOK(received->compMsgDataView);
  dataView = received->compMsgDataView->dataView;
//ets_printf("§reclgth: %d§", received->lgth);
  received->fieldOffset = 0;
  myHeaderLgth = 0;
  hdrInfos->seqIdx = 0;
  seqVal = hdrInfos->headerSequence[hdrInfos->seqIdx];
  while (seqVal != 0) {
    switch(seqVal) {
    case COMP_DISP_U16_DST:
      result = dataView->getUint16(dataView, received->fieldOffset, &received->toPart);
      checkErrOK(result);
//ets_printf("§to: 0x%04x§", received->toPart);
      received->fieldOffset += sizeof(uint16_t);
      break;
    case COMP_DISP_U16_SRC:
      result = dataView->getUint16(dataView, received->fieldOffset, &received->fromPart);
      checkErrOK(result);
//ets_printf("§from: 0x%04x§", received->fromPart);
      received->fieldOffset += sizeof(uint16_t);
      break;
    case COMP_DISP_U16_TOTAL_LGTH:
      result = dataView->getUint16(dataView, received->fieldOffset, &received->totalLgth);
      checkErrOK(result);
//ets_printf("§total: 0x%04x§", received->totalLgth);
      received->fieldOffset += sizeof(uint16_t);
      break;
    case COMP_DISP_U16_SRC_ID:
      result = dataView->getUint16(dataView, received->fieldOffset, &received->srcId);
      checkErrOK(result);
//ets_printf("§srcId: 0x%04x§", received->srcId);
      received->fieldOffset += sizeof(uint16_t);
      break;
    case COMP_DISP_U8_VECTOR_GUID:
      cp = received->GUID;
      result = dataView->getUint8Vector(dataView, received->fieldOffset, &cp, sizeof(received->GUID));
      checkErrOK(result);
//ets_printf("§GUID: %s§", received->GUID);
      received->fieldOffset += sizeof(received->GUID);
      break;
    case COMP_DISP_U8_VECTOR_HDR_FILLER:
      cp = received->hdrFiller;
      lgth = hdrInfos->headerLgth - received->fieldOffset;
      result = dataView->getUint8Vector(dataView, received->fieldOffset, &cp, lgth);
      checkErrOK(result);
      received->fieldOffset += lgth;
      break;
    }
    hdrInfos->seqIdx++;
    seqVal = hdrInfos->headerSequence[hdrInfos->seqIdx];
  }
//ets_printf("§seqIdx: %d§", hdrInfos->seqIdx);
  hdrInfos->seqIdxAfterHeader = hdrInfos->seqIdx;
  hdrInfos->currPartIdx = 0;
  result = nextFittingEntry(self, 0, 0);
//ets_printf("§getHeaderIndexFromHeaderFields!result!%d!currPartIdx!%d!§", result, hdrInfos->currPartIdx);
  if (received->compMsgDataView != NULL) {
//ets_printf("§os getHeaderIndexFromHeaderFields: free dataView: %p compMsgDataView: %p§", received->compMsgDataView->dataView, received->compMsgDataView);
    os_free(received->compMsgDataView->dataView);
    os_free(received->compMsgDataView);
    received->compMsgDataView = NULL;
  }
  return result;
}

// ================================= prepareAnswerMsg ====================================
    
static uint8_t prepareAnswerMsg(compMsgDispatcher_t *self, msgHeaderInfos_t *hdrInfos, uint8_t **handle) {
  int result;
  headerPart_t *hdr;
  int hdrIdx;

//ets_printf("§prepareAnswerMsg§");
  hdrIdx = hdrInfos->currPartIdx;
  hdrIdx++; // the Ack message has to be the next entry in headerInfos!!
  hdr = &hdrInfos->headerParts[hdrIdx];
  result = self->createMsgFromHeaderPart(self, hdr, handle);
//ets_printf("§prepareAnswerMsg:hdrIdx: %d cmdKey: 0x%04x handle: %s result: %d§", hdrIdx, hdr->hdrU16CmdKey, handle, result);
  checkErrOK(result);
  return result;
}

// ================================= handleReceivedMsg ====================================
    
static uint8_t handleReceivedMsg(compMsgDispatcher_t *self, msgHeaderInfos_t *hdrInfos) {
  int result;
  headerPart_t *hdr;
  int hdrIdx;
  uint8_t answerType;
  compMsgField_t fieldInfo;
  uint16_t u16;
  uint8_t u8;
  uint8_t startOffset;
  uint16_t sequenceEntry;
  uint8_t *handle;
  msgParts_t *received;
  bool u8TotalCrc;
  bool u16TotalCrc;
  size_t idx;

//ets_printf("§handleReceivedMsg§");
  received = &self->compMsgData->received;
//ets_printf("§handleReceivedMsg newCompMsgDataView§");
  u8TotalCrc = false;
  u16TotalCrc = false;
  received->compMsgDataView = newCompMsgDataView(received->buf, received->totalLgth);
  checkAllocOK(received->compMsgDataView);
  hdrIdx = hdrInfos->currPartIdx;
  hdr = &hdrInfos->headerParts[hdrIdx];

  // set received->lgth to end of the header
  received->lgth = hdrInfos->headerLgth;
  // we loop over the fieldSequence entries and handle them as needed
  // attention not all entries of the message are handled here, only some special entries!
 
//self->compMsgMsgDesc->dumpHeaderPart(self, hdr);
  // check if we have a U8_TOTAL_CRC or a U16_TOTAL_CRC or no TOTAL_CRC
  idx = 0;
  while (hdr->fieldSequence[idx] != 0) {
    if (hdr->fieldSequence[idx] == COMP_DISP_U8_TOTAL_CRC) {
      u8TotalCrc = true;
    }
    if (hdr->fieldSequence[idx] == COMP_DISP_U16_TOTAL_CRC) {
      u16TotalCrc = true;
    }
    idx++;
  }
  while (hdr->fieldSequence[hdrInfos->seqIdx] != 0) {
    sequenceEntry = hdr->fieldSequence[hdrInfos->seqIdx];
//ets_printf("§sequenceEntry: 0x%04x seqIdx:%d§", sequenceEntry, hdrInfos->seqIdx);
    received->fieldOffset = hdrInfos->headerLgth;
    switch (sequenceEntry) {
    case COMP_DISP_U16_CMD_KEY:
      result = received->compMsgDataView->dataView->getUint16(received->compMsgDataView->dataView, received->fieldOffset, &u16);
      received->u16CmdKey = u16;
      received->fieldOffset += 2;
      received->partsFlags |= COMP_DISP_U16_CMD_KEY;
//ets_printf("§1 u16CmdKey: 0x%04x!hdr: 0x%04x§", received->u16CmdKey, hdr->hdrU16CmdKey);
      while (received->u16CmdKey != hdr->hdrU16CmdKey) {
        hdrInfos->currPartIdx++;
        result = self->nextFittingEntry(self, 0, received->u16CmdKey);
        checkErrOK(result);
        hdr = &hdrInfos->headerParts[hdrInfos->currPartIdx];
//ets_printf("§2 u16CmdKey: 0x%04x!hdr: 0x%04x§", received->u16CmdKey, hdr->hdrU16CmdKey);
      }
//ets_printf("§found: currPartIdx: %d§", hdrInfos->currPartIdx);
      break;
    case COMP_DISP_U0_CMD_LGTH:
      received->fieldOffset += 2;
//ets_printf("§u0CmdLgth!0!§");
      break;
    case COMP_DISP_U16_CMD_LGTH:
      result = received->compMsgDataView->dataView->getUint16(received->compMsgDataView->dataView, received->fieldOffset, &u16);
      received->u16CmdLgth = u16;
      received->fieldOffset += 2;
//ets_printf("§u16CmdLgth!0x%04x!§", received->u16CmdLgth);
      break;
    case COMP_DISP_U0_CRC:
//ets_printf("§u0Crc!0!§");
      result = COMP_MSG_ERR_OK;
      break;
    case COMP_DISP_U8_CRC:
      fieldInfo.fieldLgth = 1;
      fieldInfo.fieldOffset = received->totalLgth - 1;
      if (hdr->hdrFlags & COMP_DISP_TOTAL_CRC) {
        if (u8TotalCrc) {
          fieldInfo.fieldOffset -= 1;
        } else {
          fieldInfo.fieldOffset -= 2;
        }
      }
      startOffset = hdrInfos->headerLgth;
      result = received->compMsgDataView->getCrc(received->compMsgDataView, &fieldInfo, startOffset, fieldInfo.fieldOffset);
//ets_printf("§u8Crc!res!%d!§", result);
      break;
    case COMP_DISP_U16_CRC:
      fieldInfo.fieldLgth = 2;
      fieldInfo.fieldOffset = received->totalLgth - 2;
      if (hdr->hdrFlags & COMP_DISP_TOTAL_CRC) {
        if (u8TotalCrc) {
          fieldInfo.fieldOffset -= 1;
        } else {
          fieldInfo.fieldOffset -= 2;
        }
      }
      startOffset = hdrInfos->headerLgth;
      result = received->compMsgDataView->getCrc(received->compMsgDataView, &fieldInfo, startOffset, fieldInfo.fieldOffset);
//ets_printf("§u16Crc!res!%d!§", result);
      break;
    case COMP_DISP_U0_TOTAL_CRC:
//ets_printf("§u0TotalCrc!0!§");
      result = COMP_MSG_ERR_OK;
      break;
    case COMP_DISP_U8_TOTAL_CRC:
      fieldInfo.fieldLgth = 1;
      fieldInfo.fieldOffset = received->totalLgth - 1;
//ets_printf("§u8TotalCrc!fieldOffset: %d§", fieldInfo.fieldOffset);
      startOffset = hdrInfos->headerLgth;
      result = received->compMsgDataView->getTotalCrc(received->compMsgDataView, &fieldInfo);
//ets_printf("§u8totalCrc!res!%d!§", result);
      break;
    case COMP_DISP_U16_TOTAL_CRC:
      fieldInfo.fieldLgth = 2;
      fieldInfo.fieldOffset = received->totalLgth - 2;
      startOffset = hdrInfos->headerLgth;
      result = received->compMsgDataView->getTotalCrc(received->compMsgDataView, &fieldInfo);
//ets_printf("§u16TotalCrc!res!%d!§", result);
      break;
    }
    checkErrOK(result);
    hdrInfos->seqIdx++;
  }
  // free all space of received message
//ets_printf("call deleteMsg\n");
  self->compMsgData->deleteMsg(self);
//ets_printf("§received msg deleted§");
//ets_printf("§call prepareAnswerMsg§");
  result = prepareAnswerMsg(self, hdrInfos, &handle);
  checkErrOK(result);
  result = self->resetMsgInfo(self, received);
  checkErrOK(result);
//ets_printf("§handleReceivedMsg done§");
  return COMP_MSG_ERR_OK;
}

// ================================= handleReceivedPart ====================================

/**
 * \brief handle input characters from uart or sockets
 * \param self The dispatcher struct
 * \param buffer The input characters
 * \param lgth The number of characters in the input
 * \return Error code or ErrorOK
 *
 */
static uint8_t handleReceivedPart(compMsgDispatcher_t *self, const uint8_t * buffer, uint8_t lgth) {
  int idx;
  msgHeaderInfos_t *hdrInfos;
  headerPart_t *hdr;
  int startIdx;
  int hdrIdx;
  uint8_t u8;
  compMsgField_t fieldInfo;
  compMsgData_t *compMsgData;
  msgParts_t *received;
  int result;
  bool u8TotalCrc;
  bool u16TotalCrc;
  size_t seqIdx;

//ets_printf("§handleReceivedPart: %c 0x%02x§", buffer[0], buffer[0]);
if (buffer == NULL) {
//ets_printf("§++++handleReceivedPart: buffer == NULL lgth: %d§", lgth);
} else {
//ets_printf("§++++handleReceivedPart: 0x%02x§", buffer[0]);
}
  hdrInfos = &self->msgHeaderInfos;
  compMsgData = self->compMsgData;
  received = &compMsgData->received;
//FIXME need to free at end of message handling !!!
//ets_printf("§handleReceivedPart: !received->buf: %p!§", received->buf);
//ets_printf("§receivedLgth: %d lgth: %d fieldOffset: %d headerLgth: %d!§", received->lgth, lgth, received->fieldOffset, hdrInfos->headerLgth);
  idx = 0;
  while (idx < lgth) {
    received->buf[received->lgth++] = buffer[idx];
    received->realLgth++;
    if (received->lgth == hdrInfos->headerLgth) {
//ets_printf("§received lgth: %d lgth: %d idx: %d§", received->lgth, lgth, idx);
//ets_printf("§receveived->lgth: %d§", received->lgth);
      result = getHeaderIndexFromHeaderFields(self, hdrInfos);
//ets_printf("§getHeaderIndexFromHeaderFields result: %d currPartIdx: %d§", result, hdrInfos->currPartIdx);
    }
    // loop until we have full message then decrypt if necessary and then handle the message
//ets_printf("§totalLgth: %d§", received->totalLgth);
    if (received->lgth == received->totalLgth) {
      hdrIdx = hdrInfos->currPartIdx;
      hdr = &hdrInfos->headerParts[hdrIdx];
//ets_printf("hdrIdx: %d\n", hdrIdx);
//ets_printf("§receveived->totalLgth: %d§", received->totalLgth);
      // check if we have a U8_TOTAL_CRC or a U16_TOTAL_CRC or no TOTAL_CRC
      seqIdx = 0;
      u8TotalCrc = false;
      u16TotalCrc = false;
      while (hdr->fieldSequence[seqIdx] != 0) {
        if (hdr->fieldSequence[seqIdx] == COMP_DISP_U8_TOTAL_CRC) {
          u8TotalCrc = true;
        }
        if (hdr->fieldSequence[seqIdx] == COMP_DISP_U16_TOTAL_CRC) {
          u16TotalCrc = true;
        }
        seqIdx++;
      }
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
        if (hdr->hdrFlags & COMP_DISP_TOTAL_CRC) {
          if (u8TotalCrc) {
            mlen -= 1;
          } else {
            mlen -= 2;
          }
        }
        ivlen = 16;
        klen = 16;
        cryptedPtr = received->buf + hdrInfos->headerLgth;
        result = self->decryptMsg(cryptedPtr, mlen, cryptKey, klen, cryptKey, ivlen, &decrypted, &decryptedLgth);
        checkErrOK(result);
//ets_printf("mlen: %d decryptedLgth: %d\n", mlen, decryptedLgth);
        c_memcpy(cryptedPtr, decrypted, decryptedLgth);
      }
      result = self->handleReceivedMsg(self, hdrInfos);
//ets_printf("§handleReceivedMsg end buffer idx: %d result: %d§", idx, result);
      return result;
    }
    idx++;
  }
  return COMP_DISP_ERR_OK;
}

// ================================= handleToSendPart ====================================

static uint8_t handleToSendPart(compMsgDispatcher_t *self, const uint8_t * buffer, uint8_t lgth) {
  int idx;
  uint8_t buf[100];
  msgParts_t *toSend;
  dataView_t *dataView;
  msgHeaderInfos_t *hdrInfos;
  headerPart_t *hdr;
  int startIdx;
  int hdrIdx;
  uint8_t u8;
  compMsgField_t fieldInfo;
  int result;
  int numericValue;
  uint8_t *stringValue;

//ets_printf("handleToSendPart lgth: %d buffer: %s\n", lgth, buffer);
  uint8_t *iv;
  uint8_t *cryptedPtr;
  uint8_t *cryptKey;
  uint8_t *encrypted;;
  size_t mlen;
  uint8_t klen;
  uint8_t ivlen;
  int encryptedLgth;
  size_t msgLgth;
  uint8_t *msg;

  cryptKey = "89D71101$f&7Jlkj";
  iv = "43700A27DF/&()as";
  mlen = lgth;
  ivlen = 16;
  klen = 16;
  c_memcpy(buf, buffer, lgth);
  cryptedPtr = buf;
  result = self->encryptMsg(cryptedPtr, mlen, cryptKey, klen, iv, ivlen, &encrypted, &encryptedLgth);
  checkErrOK(result);
  msgLgth = encryptedLgth;
//ets_printf("encryptedLgth: %d %d\n", encryptedLgth, result);

  result = self->sendCloudMsg(self, encrypted, encryptedLgth);
  checkErrOK(result);
  return COMP_DISP_ERR_OK;
}

// ================================= compMsgIdentifyInit ====================================

uint8_t compMsgIdentifyInit(compMsgDispatcher_t *self) {
  uint8_t result;

  self->resetHeaderInfos = &resetHeaderInfos;
  self->handleReceivedPart = &handleReceivedPart;
  self->handleToSendPart = &handleToSendPart;
  self->nextFittingEntry = &nextFittingEntry;
  self->handleReceivedMsg = &handleReceivedMsg;
  result=self->compMsgMsgDesc->readHeadersAndSetFlags(self, MSG_HEADS_FILE_NAME);
  checkErrOK(result);
  return COMP_DISP_ERR_OK;
}
