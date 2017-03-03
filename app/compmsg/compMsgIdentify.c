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

// ================================= resetHeaderInfos ====================================

static uint8_t resetHeaderInfos(compMsgDispatcher_t *self) {
  msgHeaderInfos_t *hdrInfos;

  hdrInfos = &self->msgHeaderInfos;
  hdrInfos->seqIdx = 0;
  hdrInfos->seqIdxAfterHeader = 0;
  hdrInfos->currPartIdx = 0;
  return COMP_MSG_ERR_OK;
}

// ================================= nextFittingEntry ====================================

static uint8_t nextFittingEntry(compMsgDispatcher_t *self, uint8_t u8CmdKey, uint16_t u16CmdKey) {
  msgParts_t *received;
  msgHeaderInfos_t *hdrInfos;
  headerPart_t *hdr;
  int hdrIdx;
  int found;

  received = &self->compMsgData->received;
  hdrInfos = &self->msgHeaderInfos;
  hdrIdx = hdrInfos->currPartIdx;
  hdr = &hdrInfos->headerParts[hdrIdx];
  COMP_MSG_DBG(self, "I", 2, "HEAD:!to:0x%04x!from:0x%04x!totalLgth:0x%04x!seqIdx!%d!\n", received->toPart, received->fromPart, received->totalLgth, hdrInfos->seqIdx);
  // and now search in the headers to find the appropriate message
  hdrInfos->seqIdx = hdrInfos->seqIdxAfterHeader;
  found = 0;
  while (hdrIdx < hdrInfos->numHeaderParts) {
    hdr = &hdrInfos->headerParts[hdrIdx];
    COMP_MSG_DBG(self, "I", 2, "hdrIdx: %d to: %d %d\n", hdrIdx, hdr->hdrToPart, received->toPart);
    if (hdr->hdrToPart == received->toPart) {
      COMP_MSG_DBG(self, "I", 2, "to: %d from: %d %d\n", hdr->hdrToPart, hdr->hdrFromPart, received->fromPart);
      if (hdr->hdrFromPart == received->fromPart) {
        COMP_MSG_DBG(self, "I", 2, "lgth: hdr: %d received: %d\n", hdr->hdrTotalLgth, received->totalLgth);
        if ((hdr->hdrTotalLgth == received->totalLgth) || (hdr->hdrTotalLgth == 0)) {
          COMP_MSG_DBG(self, "I", 2, "cmdKey: 0x%04x 0x%04x", u16CmdKey, received->u16CmdKey);
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
    COMP_MSG_DBG(self, "Y", 0, "nextFitting HEADER_NOT_FOUND\n");
    return COMP_MSG_ERR_HEADER_NOT_FOUND;
  }
  hdrInfos->currPartIdx = hdrIdx;
  COMP_MSG_DBG(self, "I", 2, "encryption: %c handleType: %c", hdr->hdrEncryption, hdr->hdrHandleType);
  received->encryption = hdr->hdrEncryption;
  COMP_MSG_DBG(self, "I", 2, "nextFitting!found!%d!hdrIdx!%d!hdr->cmdKey:0x%04x!received->cmdKey: 0x%04x", found, hdrIdx, hdr->hdrU16CmdKey, received->u16CmdKey);
  return COMP_MSG_ERR_OK;
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
  COMP_MSG_DBG(self, "I", 2, "getHeaderIndexFromHeaderFields newCompMsgDataView");
  received->compMsgDataView = newCompMsgDataView(received->buf, received->lgth);
  checkAllocOK(received->compMsgDataView);
  dataView = received->compMsgDataView->dataView;
  COMP_MSG_DBG(self, "I", 2, "reclgth: %d", received->lgth);
  received->fieldOffset = 0;
  myHeaderLgth = 0;
  hdrInfos->seqIdx = 0;
  seqVal = hdrInfos->headerSequence[hdrInfos->seqIdx];
  while (seqVal != 0) {
    switch(seqVal) {
    case COMP_DISP_U16_DST:
      result = dataView->getUint16(dataView, received->fieldOffset, &received->toPart);
      checkErrOK(result);
      COMP_MSG_DBG(self, "I", 2, "to: 0x%04x", received->toPart);
      received->fieldOffset += sizeof(uint16_t);
      break;
    case COMP_DISP_U16_SRC:
      result = dataView->getUint16(dataView, received->fieldOffset, &received->fromPart);
      checkErrOK(result);
      COMP_MSG_DBG(self, "I", 2, "from: 0x%04x", received->fromPart);
      received->fieldOffset += sizeof(uint16_t);
      break;
    case COMP_DISP_U16_TOTAL_LGTH:
      result = dataView->getUint16(dataView, received->fieldOffset, &received->totalLgth);
      checkErrOK(result);
      COMP_MSG_DBG(self, "I", 2, "total: 0x%04x", received->totalLgth);
      received->fieldOffset += sizeof(uint16_t);
      break;
    case COMP_DISP_U16_SRC_ID:
      result = dataView->getUint16(dataView, received->fieldOffset, &received->srcId);
      checkErrOK(result);
      COMP_MSG_DBG(self, "I", 2, "srcId: 0x%04x", received->srcId);
      received->fieldOffset += sizeof(uint16_t);
      break;
    case COMP_DISP_U8_VECTOR_GUID:
      cp = received->GUID;
      result = dataView->getUint8Vector(dataView, received->fieldOffset, &cp, sizeof(received->GUID));
      checkErrOK(result);
      COMP_MSG_DBG(self, "I", 2, "GUID: %s", received->GUID);
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
  COMP_MSG_DBG(self, "I", 2, "seqIdx: %d", hdrInfos->seqIdx);
  hdrInfos->seqIdxAfterHeader = hdrInfos->seqIdx;
  hdrInfos->currPartIdx = 0;
  result = nextFittingEntry(self, 0, 0);
  COMP_MSG_DBG(self, "I", 2, "getHeaderIndexFromHeaderFields!result!%d!currPartIdx!%d!", result, hdrInfos->currPartIdx);
  if (received->compMsgDataView != NULL) {
    COMP_MSG_DBG(self, "I", 1, "os getHeaderIndexFromHeaderFields: free dataView: %p compMsgDataView: %p", received->compMsgDataView->dataView, received->compMsgDataView);
    os_free(received->compMsgDataView->dataView);
    os_free(received->compMsgDataView);
    received->compMsgDataView = NULL;
  }
  return result;
}

// ================================= prepareAnswerMsg ====================================
    
static uint8_t prepareAnswerMsg(compMsgDispatcher_t *self, uint8_t type, uint8_t **handle) {
  int result;
  headerPart_t *hdr;
  msgHeaderInfos_t *hdrInfos;
  int hdrIdx;

  COMP_MSG_DBG(self, "I", 2, "prepareAnswerMsg");
  hdrInfos = &self->msgHeaderInfos;
  hdrIdx = hdrInfos->currPartIdx;
  switch (type) {
  case COMP_MSG_ACK_MSG:
    hdrIdx++; // the Ack message has to be the next entry in headerInfos!!
    break;
  case COMP_MSG_NAK_MSG:
    hdrIdx += 2; // the Nak message has to be the second following entry in headerInfos!!
    break;
  }
  hdr = &hdrInfos->headerParts[hdrIdx];
  COMP_MSG_DBG(self, "I", 2, "hdrCmdKey: 0x%04x", hdr->hdrU16CmdKey);
  result = self->compMsgBuildMsg->createMsgFromHeaderPart(self, hdr, handle);
  COMP_MSG_DBG(self, "I", 2, "createMsgFromHeaderPart result: %d", result);
  checkErrOK(result);
  return result;
}

// ================================= handleReceivedHeader ====================================
    
static uint8_t handleReceivedHeader(compMsgDispatcher_t *self) {
  int result;
  headerPart_t *hdr;
  int hdrIdx;
  msgParts_t *received;
  msgHeaderInfos_t *hdrInfos;
  compMsgField_t fieldInfo;
  uint16_t u16;
  uint8_t u8;
  uint16_t sequenceEntry;
  bool u8TotalCrc;
  bool u16TotalCrc;
  size_t startOffset;
  size_t idx;

  COMP_MSG_DBG(self, "I", 2, "handleReceivedHeader");
  hdrInfos = &self->msgHeaderInfos;
  received = &self->compMsgData->received;
  u8TotalCrc = false;
  u16TotalCrc = false;
  COMP_MSG_DBG(self, "I", 2, "handleReceivedHeader newCompMsgDataView");
  received->compMsgDataView = newCompMsgDataView(received->buf, received->totalLgth);
  checkAllocOK(received->compMsgDataView);
//received->compMsgDataView->dataView->dumpBinary(received->buf+30, 10, "Received->buf");
  hdrIdx = hdrInfos->currPartIdx;
  COMP_MSG_DBG(self, "I", 2, "handleReceivedHeader: currPartIdx: %d totalLgth: %d", hdrInfos->currPartIdx, received->totalLgth);
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
    COMP_MSG_DBG(self, "I", 2, "sequenceEntry: 0x%04x seqIdx:%d", sequenceEntry, hdrInfos->seqIdx);
    received->fieldOffset = hdrInfos->headerLgth;
    switch (sequenceEntry) {
    case COMP_DISP_U16_CMD_KEY:
      result = received->compMsgDataView->dataView->getUint16(received->compMsgDataView->dataView, received->fieldOffset, &u16);
      received->u16CmdKey = u16;
      COMP_MSG_DBG(self, "I", 2, "1 u16CmdKey: 0x%04x!hdr: 0x%04x!offset: %d", received->u16CmdKey, hdr->hdrU16CmdKey, received->fieldOffset);
      received->fieldOffset += 2;
      received->partsFlags |= COMP_DISP_U16_CMD_KEY;
      while (received->u16CmdKey != hdr->hdrU16CmdKey) {
        hdrInfos->currPartIdx++;
        result = self->compMsgIdentify->nextFittingEntry(self, 0, received->u16CmdKey);
        checkErrOK(result);
        hdr = &hdrInfos->headerParts[hdrInfos->currPartIdx];
        COMP_MSG_DBG(self, "I", 2, "2 u16CmdKey: 0x%04x!hdr: 0x%04x", received->u16CmdKey, hdr->hdrU16CmdKey);
      }
      COMP_MSG_DBG(self, "I", 2, "found: currPartIdx: %d", hdrInfos->currPartIdx);
      break;
    case COMP_DISP_U0_CMD_LGTH:
      received->fieldOffset += 2;
      COMP_MSG_DBG(self, "I", 2, "u0CmdLgth!0!");
      break;
    case COMP_DISP_U16_CMD_LGTH:
      result = received->compMsgDataView->dataView->getUint16(received->compMsgDataView->dataView, received->fieldOffset, &u16);
      received->u16CmdLgth = u16;
      received->fieldOffset += 2;
      COMP_MSG_DBG(self, "I", 2, "u16CmdLgth!0x%04x!", received->u16CmdLgth);
      break;
    case COMP_DISP_U0_CRC:
      COMP_MSG_DBG(self, "I", 2, "u0Crc!0!");
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
//received->compMsgDataView->dataView->dumpBinaryWide(received->compMsgDataView->dataView->data, fieldInfo.fieldOffset, "beforegetcrc");
      result = received->compMsgDataView->getCrc(self, received->compMsgDataView->dataView, &fieldInfo, startOffset, fieldInfo.fieldOffset);
      COMP_MSG_DBG(self, "I", 2, "u8Crc!res!%d!", result);
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
      result = received->compMsgDataView->getCrc(self, received->compMsgDataView->dataView, &fieldInfo, startOffset, fieldInfo.fieldOffset);
      COMP_MSG_DBG(self, "I", 2, "u16Crc!res!%d!", result);
      break;
    case COMP_DISP_U0_TOTAL_CRC:
      COMP_MSG_DBG(self, "I", 2, "u0TotalCrc!0!");
      result = COMP_MSG_ERR_OK;
      break;
    case COMP_DISP_U8_TOTAL_CRC:
      // has to be done before decryption!!
      result = COMP_MSG_ERR_OK;
      break;
    case COMP_DISP_U16_TOTAL_CRC:
      // has to be done before decryption!!
      result = COMP_MSG_ERR_OK;
      break;
    }
    checkErrOK(result);
    hdrInfos->seqIdx++;
  }
  // free all space of received message
  COMP_MSG_DBG(self, "I", 2, "call deleteMsg");
  self->compMsgData->deleteMsg(self);
  COMP_MSG_DBG(self, "I", 2, "received msg deleted");
  return COMP_MSG_ERR_OK;
}

// ================================= handleReceivedMsg ====================================
    
static uint8_t handleReceivedMsg(compMsgDispatcher_t *self) {
  int result;
  msgParts_t *received;
  uint8_t answerType;
  uint8_t *handle;

  COMP_MSG_DBG(self, "I", 2, "handleReceivedMsg");
  received = &self->compMsgData->received;
  result = self->compMsgIdentify->handleReceivedHeader(self);
  COMP_MSG_DBG(self, "I", 2, "call prepareAnswerMsg");
  result = self->compMsgIdentify->prepareAnswerMsg(self, COMP_MSG_ACK_MSG, &handle);
  checkErrOK(result);
  result = self->resetMsgInfo(self, received);
  checkErrOK(result);
  COMP_MSG_DBG(self, "I", 2, "handleReceivedMsg done");
  return COMP_MSG_ERR_OK;
}

// ================================= storeReceivedMsg ====================================
    
static uint8_t storeReceivedMsg(compMsgDispatcher_t *self) {
  int idx;
  int fieldToSaveIdx;
  int result;
  int hdrIdx;
  int numericValue;
  uint8_t *stringValue;
  msgParts_t *received;
  msgHeaderInfos_t *hdrInfos;
  headerPart_t *hdr;
  msgDescPart_t *msgDescPart;
  msgValPart_t *msgValPart;
  fieldsToSave_t *fieldsToSave;
  action_t actionCallback;
  compMsgField_t *fieldInfo;
  uint8_t answerType;
  uint8_t *handle;
  bool hadActionCb;

  COMP_MSG_DBG(self, "I", 2, "storeReceivedMsg");
  received = &self->compMsgData->received;
  COMP_MSG_DBG(self, "I", 2, "call handleReceivedHeader");
  // next line deletes compMsgData !!
  result = self->compMsgIdentify->handleReceivedHeader(self);
  checkErrOK(result);
  hdrInfos = &self->msgHeaderInfos;
  hdrIdx = hdrInfos->currPartIdx;
  hdr = &hdrInfos->headerParts[hdrIdx];
  COMP_MSG_DBG(self, "I", 2, "getMsgPartsFromHeaderPart");
  result = self->compMsgMsgDesc->getMsgPartsFromHeaderPart(self, hdr, &handle);
  checkErrOK(result);
  result = self->compMsgData->createMsg(self, self->compMsgData->numMsgDescParts, &handle);
  checkErrOK(result);
  idx = 0;
  while (idx < self->compMsgData->numMsgDescParts) {
    msgDescPart = &self->compMsgData->msgDescParts[idx];
    result = self->compMsgData->addField(self, msgDescPart->fieldNameStr, msgDescPart->fieldTypeStr, msgDescPart->fieldLgth);
    checkErrOK(result);
    idx++;
  }
  self->compMsgData->compMsgDataView = newCompMsgDataView(received->buf, received->totalLgth);
  result = self->compMsgData->initReceivedMsg(self);
  checkErrOK(result);
  fieldToSaveIdx = 0;
//self->compMsgData->dumpMsg(self);
//self->compMsgData->compMsgDataView->dataView->dumpBinary(self->compMsgData->compMsgDataView->dataView->data, self->compMsgData->compMsgDataView->dataView->lgth, "MSG");
  while (fieldToSaveIdx < self->numFieldsToSave) {
    fieldsToSave = &self->fieldsToSave[fieldToSaveIdx];
    COMP_MSG_DBG(self, "I", 2, "fieldsToSave: %s!\n", fieldsToSave->fieldNameStr);
    idx = 0;
    while (idx < self->compMsgData->numMsgDescParts) {
      msgDescPart = &self->compMsgData->msgDescParts[idx];
      if (c_strcmp(msgDescPart->fieldNameStr, fieldsToSave->fieldNameStr) == 0) {
        stringValue = NULL;
        numericValue = 0;
        result = self->compMsgData->getFieldValue(self, fieldsToSave->fieldNameStr, &numericValue, &stringValue);
        checkErrOK(result);
        if (stringValue != NULL) {
          COMP_MSG_DBG(self, "I", 2, "found fieldToSave: %s %s", fieldsToSave->fieldNameStr, stringValue);
        } else {
          COMP_MSG_DBG(self, "I", 2, "found fieldToSave: %s %d", fieldsToSave->fieldNameStr, numericValue);
        }
        fieldsToSave->fieldValueStr = stringValue;
        fieldsToSave->fieldValue = numericValue;
      }
      idx++;
    }
    fieldToSaveIdx++;
  }
  idx = 0;
  hadActionCb = false;
  while (idx < self->compMsgData->numMsgValParts) {
    msgValPart = &self->compMsgData->msgValParts[idx];
    if (msgValPart->fieldValueActionCb != NULL) {
      COMP_MSG_DBG(self, "I", 2, "have actionCb: %s", msgValPart->fieldValueActionCb);
      hadActionCb = true;
      result = self->compMsgAction->getActionCallback(self, msgValPart->fieldValueActionCb+1, &actionCallback);
      checkErrOK(result);
      result = actionCallback(self);
      checkErrOK(result);
    }
    idx++;
  }
  if (!hadActionCb) {
    result = self->compMsgIdentify->prepareAnswerMsg(self, COMP_MSG_ACK_MSG, &handle);
    checkErrOK(result);
    result = self->resetMsgInfo(self, received);
    checkErrOK(result);
  }
  COMP_MSG_DBG(self, "I", 2, "storeReceivedMsg done");
  return COMP_MSG_ERR_OK;
}

// ================================= sendClientIPMsg ====================================
    
static uint8_t sendClientIPMsg(compMsgDispatcher_t *self) {
  uint8_t result;
  int ipAddr;
  int port;
  int sequenceNum;
  int numericValue;
  char temp[64];
  uint8_t *stringValue;
  uint8_t *handle;
  msgParts_t *received;

  COMP_MSG_DBG(self, "I", 2, "sendClientIPMsg");
  self->compMsgSendReceive->startSendMsg = NULL;
  self->stopAccessPoint = true;
  received = &self->compMsgData->received;
  result = self->compMsgWifiData->getWifiValue(self, WIFI_INFO_CLIENT_IP_ADDR, 0, &ipAddr, &stringValue);
  checkErrOK(result);
  result = self->compMsgWifiData->getWifiValue(self, WIFI_INFO_CLIENT_PORT, DATA_VIEW_FIELD_UINT8_T, &port, &stringValue);
  checkErrOK(result);
  result = self->compMsgWifiData->getWifiValue(self, WIFI_INFO_CLIENT_SEQUENCE_NUM, DATA_VIEW_FIELD_UINT32_T, &sequenceNum, &stringValue);
  checkErrOK(result);
  os_sprintf(temp, "%d.%d.%d.%d", IP2STR(&ipAddr));
  COMP_MSG_DBG(self, "I", 2, "IP: %s port: %d sequenceNum: %d", temp, port, sequenceNum);
  result = self->compMsgIdentify->prepareAnswerMsg(self, COMP_MSG_ACK_MSG, &handle);
  COMP_MSG_DBG(self, "I", 2, "prepareAnswerMsg: result: %d", result);
  checkErrOK(result);
  result = self->resetMsgInfo(self, received);
  COMP_MSG_DBG(self, "I", 2, "resetMsgInfo: result: %d", result);
  checkErrOK(result);
  // saveUserData here (clientSsid, clientPasswd (evtually IP Addr and port?)!!!
  result = self->compMsgOta->saveUserData(self);
  checkErrOK(result);
  COMP_MSG_DBG(self, "I", 2, "sendClientIPMsg done");
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
static uint8_t handleReceivedPart(compMsgDispatcher_t *self, const uint8_t * buffer, size_t lgth) {
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
  uint8_t *saveData;
  size_t saveLgth;
  bool haveDataView;

if (buffer == NULL) {
  COMP_MSG_DBG(self, "I", 2, "++++handleReceivedPart: buffer == NULL lgth: %d", lgth);
} else {
  COMP_MSG_DBG(self, "I", 2, "++++handleReceivedPart: 0x%02x lgth: %d", buffer[0], lgth);
}
  hdrInfos = &self->msgHeaderInfos;
  compMsgData = self->compMsgData;
  received = &compMsgData->received;
//FIXME need to free at end of message handling !!!
  COMP_MSG_DBG(self, "I", 2, "handleReceivedPart: !received->buf: %p!", received->buf);
  COMP_MSG_DBG(self, "I", 2, "receivedLgth: %d lgth: %d fieldOffset: %d headerLgth: %d!", received->lgth, lgth, received->fieldOffset, hdrInfos->headerLgth);
  idx = 0;
  while (idx < lgth) {
    received->buf[received->lgth++] = buffer[idx];
    received->realLgth++;
    if (received->lgth == hdrInfos->headerLgth) {
      COMP_MSG_DBG(self, "I", 2, "received lgth: %d lgth: %d idx: %d", received->lgth, lgth, idx);
//compMsgData->compMsgDataView->dataView->dumpBinary(received->buf, received->lgth, "Received->buf");
      COMP_MSG_DBG(self, "I", 2, "receveived->lgth: %d", received->lgth);
      result = getHeaderIndexFromHeaderFields(self, hdrInfos);
      COMP_MSG_DBG(self, "I", 2, "getHeaderIndexFromHeaderFields result: %d currPartIdx: %d", result, hdrInfos->currPartIdx);
    }
    // loop until we have full message then decrypt if necessary and then handle the message
    COMP_MSG_DBG(self, "I", 2, "totalLgth: %d", received->totalLgth);
    if (received->lgth == received->totalLgth) {
      hdrIdx = hdrInfos->currPartIdx;
      hdr = &hdrInfos->headerParts[hdrIdx];
      COMP_MSG_DBG(self, "I", 2, "hdrIdx: %d", hdrIdx);
      COMP_MSG_DBG(self, "I", 2, "receveived->totalLgth: %d", received->totalLgth);
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
      COMP_MSG_DBG(self, "I", 1, "hdr->hdrHandleType: %c", hdr->hdrHandleType);
      switch (hdr->hdrHandleType) {
      case 'G':
      case 'R':
      case 'D':
        // have to check totalCrc here because of eventual encryption!!
        if (u8TotalCrc) {
          fieldInfo.fieldLgth = 1;
          fieldInfo.fieldOffset = received->totalLgth - 1;
        } else {
          fieldInfo.fieldLgth = 2;
          fieldInfo.fieldOffset = received->totalLgth - 2;
        }
        if (compMsgData->compMsgDataView == NULL) {
          compMsgData->compMsgDataView = newCompMsgDataView(received->buf, received->lgth);
          haveDataView = false;
        } else {
          haveDataView = true;
          saveData = compMsgData->compMsgDataView->dataView->data;
          saveLgth = compMsgData->compMsgDataView->dataView->lgth;
          compMsgData->compMsgDataView->dataView->data = received->buf;
          compMsgData->compMsgDataView->dataView->lgth = received->totalLgth;
        }
        result = compMsgData->compMsgDataView->getTotalCrc(self, compMsgData->compMsgDataView->dataView, &fieldInfo);
        COMP_MSG_DBG(self, "I", 2, "getTotalCrc: res!%d!", result);
        checkErrOK(result);
        if (haveDataView) {
          compMsgData->compMsgDataView->dataView->data = saveData;
          compMsgData->compMsgDataView->dataView->lgth = saveLgth;
        }
        COMP_MSG_DBG(self, "I", 1, "hdrEncryption: %c!", hdr->hdrEncryption);
        if (hdr->hdrEncryption == 'E') {
          int numericValue;
          uint8_t *cryptedPtr;
          uint8_t *cryptKey;
          uint8_t *decrypted;;
          size_t mlen;
          size_t klen;
          size_t ivlen;
          int decryptedLgth;

          // decrypt encrypted message part (after header)
          result = self->compMsgModuleData->getCryptKey(self, &numericValue, &cryptKey);
          checkErrOK(result);
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
//compMsgData->compMsgDataView->dataView->dumpBinaryWide(received->buf, received->totalLgth, "beforedecrypt");
          cryptedPtr = received->buf + hdrInfos->headerLgth;
          result = self->compMsgUtil->decryptMsg(self, cryptedPtr, mlen, cryptKey, klen, cryptKey, ivlen, &decrypted, &decryptedLgth);
          checkErrOK(result);
          COMP_MSG_DBG(self, "I", 1, "mlen: %d decryptedLgth: %d", mlen, decryptedLgth);
          c_memcpy(cryptedPtr, decrypted, decryptedLgth);
//compMsgData->compMsgDataView->dataView->dumpBinaryWide(received->buf, received->totalLgth, "afterdecrypt");
        }
        COMP_MSG_DBG(self, "I", 2, "call storeReceivedMsg");
        result = self->compMsgIdentify->storeReceivedMsg(self);
        COMP_MSG_DBG(self, "I", 2, "storeReceivedMsg end buffer idx: %d result: %d", idx, result);
        return result;
      case 'U':
      case 'W':
        self->compMsgData->currHdr = hdr;
        result = self->compMsgBuildMsg->forwardMsg(self);
        COMP_MSG_DBG(self, "I", 1, "forwardMsg result: %d", result);
        return result;
      default:
        COMP_MSG_DBG(self, "Y", 0, "handleReceivedPart: funny handleType: %c 0x%02x", hdr->hdrHandleType, hdr->hdrHandleType);
        return COMP_MSG_ERR_FUNNY_HANDLE_TYPE;
      }
    }
    idx++;
  }
  return COMP_MSG_ERR_OK;
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

  COMP_MSG_DBG(self, "I", 2, "handleToSendPart lgth: %d buffer: %s\n", lgth, buffer);
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
  result = self->compMsgUtil->encryptMsg(self, cryptedPtr, mlen, cryptKey, klen, iv, ivlen, &encrypted, &encryptedLgth);
  checkErrOK(result);
  msgLgth = encryptedLgth;
  COMP_MSG_DBG(self, "I", 2, "encryptedLgth: %d %d\n", encryptedLgth, result);

  self->cloudMsgData = encrypted;
  self->cloudMsgDataLgth = encryptedLgth;
  result = self->compMsgSendReceive->sendCloudMsg(self);
  checkErrOK(result);
  return COMP_MSG_ERR_OK;
}

// ================================= compMsgIdentifyInit ====================================

uint8_t compMsgIdentifyInit(compMsgDispatcher_t *self) {
  uint8_t result;

  self->compMsgIdentify->resetHeaderInfos = &resetHeaderInfos;
  self->compMsgIdentify->handleReceivedPart = &handleReceivedPart;
  self->compMsgIdentify->handleToSendPart = &handleToSendPart;
  self->compMsgIdentify->nextFittingEntry = &nextFittingEntry;
  self->compMsgIdentify->prepareAnswerMsg = &prepareAnswerMsg;
  self->compMsgIdentify->handleReceivedHeader = &handleReceivedHeader;
  self->compMsgIdentify->handleReceivedMsg = &handleReceivedMsg;
  self->compMsgIdentify->storeReceivedMsg = &storeReceivedMsg;
  self->compMsgIdentify->sendClientIPMsg = &sendClientIPMsg;

  result=self->compMsgMsgDesc->readHeadersAndSetFlags(self, COMP_MSG_HEADS_FILE_NAME);
  checkErrOK(result);
  result=self->compMsgMsgDesc->getFieldsToSave(self, COMP_MSG_FIELDS_TO_SAVE_FILE_NAME);
  checkErrOK(result);
  return COMP_MSG_ERR_OK;
}

// ================================= newCompMsgIdentify ====================================

compMsgIdentify_t *newCompMsgIdentify() {
  compMsgIdentify_t *compMsgIdentify = os_zalloc(sizeof(compMsgIdentify_t));
  if (compMsgIdentify == NULL) {
    return NULL;
  }
  return compMsgIdentify;
}
