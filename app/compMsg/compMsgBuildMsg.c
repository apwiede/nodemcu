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
 * File:   compMsgBuildMsg.c
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

// ================================= createMsgFromHeaderPart ====================================

static uint8_t ICACHE_FLASH_ATTR createMsgFromHeaderPart (compMsgDispatcher_t *self, headerPart_t *hdr, uint8_t **handle) {
  uint8_t result;
  int idx;
  bool isEnd;
  msgDescPart_t *msgDescPart;
  msgValPart_t *msgValPart;

  COMP_MSG_DBG(self, "B", 2, "createMsgFromHeaderPart1");
  result = self->compMsgMsgDesc->getMsgPartsFromHeaderPart(self, hdr, handle);
  checkErrOK(result);
  result = self->compMsgData->createMsg(self, self->compMsgData->numMsgDescParts, handle);
  checkErrOK(result);
  idx = 0;
  while(idx < self->compMsgData->numMsgDescParts) {
    msgDescPart = &self->compMsgData->msgDescParts[idx];
    result = self->compMsgData->addField(self, msgDescPart->fieldNameStr, msgDescPart->fieldTypeStr, msgDescPart->fieldLgth);
    checkErrOK(result);
    idx++;
  }

  COMP_MSG_DBG(self, "B", 1, "heap4: %d", system_get_free_heap_size());
  // runAction calls at the end buildMsg
  if (self->compMsgData->prepareValuesCbName != NULL) {
    uint8_t actionMode;
    action_t actionCallback;
    uint8_t type;

    COMP_MSG_DBG(self, "B", 2, "call prepareValuesCbName: %s", self->compMsgData->prepareValuesCbName);
    result = self->compMsgAction->getActionCallback(self, self->compMsgData->prepareValuesCbName+1, &actionCallback);
    checkErrOK(result);
    result = actionCallback(self);
    checkErrOK(result);
    result = self->compMsgAction->getActionMode(self, self->compMsgData->prepareValuesCbName+1, &actionMode);
    self->actionMode = actionMode;
    checkErrOK(result);
    result  = self->compMsgAction->runAction(self, &type);
    // actionCallback starts a call with eventually a callback and returns here before the callback has been running!!
    // when coming here we are finished and the callback will do the work later on!
  } else {
    result = self->compMsgBuildMsg->buildMsg(self);
  }
  return result;
}

// ================================= fixOffsetsForKeyValues ====================================

/**
 * \brief fix the field offsets of all if we have key/value fields after calling the callbacks for these fields.
 * \param self The dispatcher struct
 * \return Error code or ErrorOK
 *
 */
static uint8_t ICACHE_FLASH_ATTR fixOffsetsForKeyValues(compMsgDispatcher_t *self) {
  uint8_t result;
  uint8_t msgDescPartIdx;
  uint8_t fieldIdx;
  uint8_t keyValueIdx;
  uint8_t type;
  uint8_t *stringValue;
  int numericValue;
  compMsgField_t *fieldInfo;
  msgDescPart_t *msgDescPart;
  compMsgData_t *compMsgData;
  msgKeyValueDescPart_t *msgKeyValueDescPart;
  bool found;

  compMsgData = self->compMsgData;
  fieldIdx = 0;
  msgDescPartIdx = 0;
  COMP_MSG_DBG(self, "B", 2, "fixOffsetsForKeyValues: numFields: %d!", compMsgData->numFields);
  while (fieldIdx < compMsgData->numFields) {
    fieldInfo = &compMsgData->fields[fieldIdx];
    msgDescPart = &self->compMsgData->msgDescParts[msgDescPartIdx];
    self->compMsgData->msgDescPart = msgDescPart;
    msgKeyValueDescPart = NULL;
    if (msgDescPart->fieldNameStr[0] == '#') {
      // get the corresponding msgKeyValueDescPart
      found = false;
      keyValueIdx = 0;
      while (keyValueIdx < self->numMsgKeyValueDescParts) {
        msgKeyValueDescPart = &self->msgKeyValueDescParts[keyValueIdx];
        if (c_strcmp(msgKeyValueDescPart->keyNameStr, msgDescPart->fieldNameStr) == 0) {
          found = true;
          break;
        }
        keyValueIdx++;
      }
      if (!found) {
        msgKeyValueDescPart = NULL;
      }
    }
    COMP_MSG_DBG(self, "B", 2, "fixOffsetsForKeyValues: idx: %d!%p!%s!\n", fieldIdx, msgDescPart->fieldSizeCallback, msgDescPart->fieldNameStr);
    if (msgDescPart->fieldSizeCallback != NULL) {
      // the key name must have the prefix: "#key_"!
      if (msgDescPart->fieldNameStr[0] != '#') {
        return COMP_MSG_ERR_FIELD_NOT_FOUND;
      }
      result = msgDescPart->fieldSizeCallback(self, &numericValue, &stringValue);
      COMP_MSG_DBG(self, "B", 2, "fieldSizeCallback for: %s numericValue: %d stringValue: %p\n", msgDescPart->fieldNameStr, numericValue, stringValue);
      checkErrOK(result);
      if (msgKeyValueDescPart != NULL) {
        fieldInfo->fieldKey = msgKeyValueDescPart->keyId;
        fieldInfo->fieldKeyTypeId = msgKeyValueDescPart->keyType;
        COMP_MSG_DBG(self, "B", 2, "fieldKey: %d\n", msgKeyValueDescPart->keyId);
      } else {
        fieldInfo->fieldKey = msgDescPart->fieldKey;
      }
      fieldInfo->fieldLgth = msgDescPart->fieldSize + 2 * sizeof(uint16_t) + sizeof(uint8_t); // for key, type and lgth in front of value!!
      COMP_MSG_DBG(self, "B", 2, "fixOffsetsForKeyValues: %s size: %d lgth: %d\n", msgDescPart->fieldNameStr, msgDescPart->fieldSize, fieldInfo->fieldLgth);
    } else {
      if (msgKeyValueDescPart != NULL) {
        fieldInfo->fieldKey = msgKeyValueDescPart->keyId;
        msgDescPart->fieldSize = msgKeyValueDescPart->keyLgth;
        fieldInfo->fieldLgth = msgDescPart->fieldSize + 2 * sizeof(uint16_t) + sizeof(uint8_t); // for key, type and lgth in front of value!!
      }
    }
    msgDescPartIdx++;
    fieldIdx++;
  }
  COMP_MSG_DBG(self, "B", 2, "fixOffsetsForKeyValues: done!\n");
  return COMP_MSG_ERR_OK;
}

// ================================= setMsgFieldValue ====================================

/**
 * \brief the value of a message field
 * \param self The dispatcher struct
 * \param type The type of the answer
 * \return Error code or ErrorOK
 *
 */
static uint8_t ICACHE_FLASH_ATTR setMsgFieldValue(compMsgDispatcher_t *self, uint8_t type) {
  uint8_t result;
  uint8_t *fieldNameStr;
  uint8_t *stringValue;
  int numericValue;
  compMsgData_t *compMsgData;

  COMP_MSG_DBG(self, "B", 2, "setMsgFieldValue: %s %s\n", self->compMsgData->msgValPart->fieldNameStr, self->compMsgData->msgValPart->fieldValueStr);
  compMsgData = self->compMsgData;
  if (ets_strncmp(self->compMsgData->msgValPart->fieldValueStr, "@get", 4) == 0) {
    // call the callback function for the field!!
    COMP_MSG_DBG(self, "B", 2, "setMsgFieldValue:cb %s!%p!size: %d", self->compMsgData->msgValPart->fieldValueStr, self->compMsgData->msgValPart->fieldValueCallback, self->compMsgData->msgDescPart->fieldSize);
    fieldNameStr = self->compMsgData->msgValPart->fieldNameStr;
    if (self->compMsgData->msgValPart->fieldValueCallback != NULL) {
      result = self->compMsgData->msgValPart->fieldValueCallback(self, &numericValue, &stringValue);
      COMP_MSG_DBG(self, "B", 2, "fieldValueCallback: result: %d\n", result);
      checkErrOK(result);
      COMP_MSG_DBG(self, "B", 2, "cb field1: %s!value: 0x%04x %s!", fieldNameStr, numericValue, stringValue == NULL ? "nil" : (char *)stringValue);
      self->compMsgData->msgValPart->fieldValue = numericValue;
    }
    COMP_MSG_DBG(self, "B", 2, "isNumber: %d\n", self->compMsgData->msgValPart->fieldFlags & COMP_DISP_DESC_VALUE_IS_NUMBER);
    COMP_MSG_DBG(self, "B", 2, "cb field1a: %s!valueStr: %p!", fieldNameStr, self->compMsgData->msgValPart->fieldKeyValueStr);
    if ((self->compMsgData->msgValPart->fieldFlags & COMP_DISP_DESC_VALUE_IS_NUMBER) || (stringValue == NULL)) {
      stringValue = NULL;
      numericValue = self->compMsgData->msgValPart->fieldValue;
    } else {
//      stringValue = self->compMsgData->msgValPart->fieldKeyValueStr;
      numericValue = 0;
    }
    COMP_MSG_DBG(self, "B", 2, "cb field2: %s!value: 0x%04x %s!", fieldNameStr, numericValue, stringValue == NULL ? "nil" : (char *)stringValue);
    result = self->compMsgData->setFieldValue(self, fieldNameStr, numericValue, stringValue);
  } else {
    fieldNameStr = self->compMsgData->msgValPart->fieldNameStr;
    COMP_MSG_DBG(self, "B", 2, "fieldName: %s!id: %d!", fieldNameStr, self->compMsgData->msgValPart->fieldNameId);
    if (self->compMsgData->msgValPart->fieldFlags & COMP_DISP_DESC_VALUE_IS_NUMBER) {
      stringValue = NULL;
      numericValue = self->compMsgData->msgValPart->fieldValue;
    } else {
      stringValue = self->compMsgData->msgValPart->fieldValueStr;
      numericValue = 0;
    }
    switch (self->compMsgData->msgValPart->fieldNameId) {
      case COMP_MSG_SPEC_FIELD_DST:
        result = compMsgData->setFieldValue(self, fieldNameStr, numericValue, stringValue);
        break;
      case COMP_MSG_SPEC_FIELD_SRC:
        result = compMsgData->setFieldValue(self, fieldNameStr, numericValue, stringValue);
        break;
      case COMP_MSG_SPEC_FIELD_CMD_KEY:
        numericValue = self->compMsgData->currHdr->hdrU16CmdKey;
        stringValue = NULL;
        COMP_MSG_DBG(self, "B", 2, "cmdKey value: 0x%04x", numericValue);
        result = compMsgData->setFieldValue(self, fieldNameStr, numericValue, stringValue);
        break;
      default:
        COMP_MSG_DBG(self, "B", 2, "fieldName: %s!value: 0x%04x %s", fieldNameStr, numericValue, stringValue == NULL ? "nil" : (char *)stringValue);
        result = self->compMsgData->setFieldValue(self, fieldNameStr, numericValue, stringValue);
        break;
    }
    checkErrOK(result);
  }
  COMP_MSG_DBG(self, "B", 2, "setMsgFieldValue: done");
  return COMP_MSG_ERR_OK;
}

// ================================= setMsgValues ====================================

/**
 * \brief set the field values of a message
 * \param self The dispatcher struct
 * \return Error code or ErrorOK
 *
 */
static uint8_t ICACHE_FLASH_ATTR setMsgValues(compMsgDispatcher_t *self) {
  uint8_t result;
  uint8_t type;
  compMsgDataView_t *dataView;
  uint8_t msgDescPartIdx;
  uint8_t msgValPartIdx;
  compMsgField_t *fieldInfo;
  msgDescPart_t *msgDescPart;
  msgValPart_t *msgValPart;
  compMsgData_t *compMsgData;
  uint8_t *handle;
  int numericValue;
  uint8_t *stringValue;

  COMP_MSG_DBG(self, "B", 2, "setMsgValues\n");
  handle = self->msgHandle;
  type = 'A';
  compMsgData = self->compMsgData;
  dataView = compMsgData->compMsgDataView;
  // loop over MSG Fields, to check if we eventually have table rows!!
  msgDescPartIdx = 0;
  msgValPartIdx = 0;
  msgValPart = &self->compMsgData->msgValParts[msgValPartIdx];
  COMP_MSG_DBG(self, "B", 2, "numFields: %d numMsgValParts: %d", compMsgData->numFields, self->compMsgData->numMsgValParts);
  while ((msgDescPartIdx < compMsgData->numFields) && (msgValPartIdx <= self->compMsgData->numMsgValParts)) {
    msgDescPart = &self->compMsgData->msgDescParts[msgDescPartIdx];
    self->compMsgData->msgDescPart = msgDescPart;
    msgValPart = &self->compMsgData->msgValParts[msgValPartIdx];
    self->compMsgData->msgValPart = msgValPart;
    COMP_MSG_DBG(self, "B", 2, "setMsgValues: %s descIdx: %d valIdx: %d", msgDescPart->fieldNameStr, msgDescPartIdx, msgValPartIdx);
    fieldInfo = &compMsgData->fields[msgDescPartIdx++];
    COMP_MSG_DBG(self, "B", 2, "default fieldNameId: %d\n", fieldInfo->fieldNameId);
    if (fieldInfo->fieldNameId == msgValPart->fieldNameId) {
      result = self->compMsgBuildMsg->setMsgFieldValue(self, type);
      checkErrOK(result);
      msgValPartIdx++;
    }
  }
  numericValue = compMsgData->currHdr->hdrU16CmdKey;
  stringValue = NULL;
  COMP_MSG_DBG(self, "B", 2, "cmdKey value: 0x%04x", numericValue);
  result = compMsgData->setFieldValue(self, "@cmdKey", numericValue, stringValue);
  COMP_MSG_DBG(self, "B", 2, "cmdKey result: %d", result);
  checkErrOK(result);
  compMsgData->prepareMsg(self);
  checkErrOK(result);
  COMP_MSG_DBG(self, "B", 2, "setMsgValues done");
//  compMsgData->dumpMsg(self);
  return COMP_MSG_ERR_OK;
}

// ================================= buildMsg ====================================

/**
 * \brief Build a message for sending to Uart or socket
 * 
 * \param self The dispatcher struct
 * \return Error code or ErrorOK
 *
 */
static uint8_t ICACHE_FLASH_ATTR buildMsg(compMsgDispatcher_t *self) {
  uint8_t result;
  size_t msgLgth;
  uint8_t *msgData;
  size_t defLgth;
  uint8_t *defData;
  uint8_t *cryptKey;
  uint8_t klen;
  uint8_t ivlen;
  uint8_t *stringValue;
  compMsgField_t *fieldInfo;
  int src;
  int dst;

  // at this point an eventual callback for getting the values 
  // has been already done using runAction in createMsgFromHeaderPart
  // so now we can fix the offsets if needed for key value list entries
  // we can do that in looking for a special key entry @numKeyValues in msgDescParts
  // could be also done in looking in compMsgData->fields
  // to get the desired keys we look in msgValParts for fieldNames starting with '#'
  // the fieldValueStr ther is a callBackFunction for building the key value entries
  // the entry @numKeyValues in msgValParts tells us how many different keys follow
  // a key value entry is built like so:
  // uint16_t key
  // uint16_t length o value
  // uint8_t* the bytes of the value
  // this could if needed also be an array of uint16_t etc. depending on the key
  // the receiver must know how the value is built depending on the key!!
  
  COMP_MSG_DBG(self, "B", 2, "buildMsg");
  result = self->compMsgBuildMsg->fixOffsetsForKeyValues(self);
  checkErrOK(result);
  self->compMsgData->direction = COMP_MSG_TO_SEND_DATA;
  COMP_MSG_DBG(self, "B", 2, "heap2: %d", system_get_free_heap_size());
  result = self->compMsgData->initMsg(self);
  checkErrOK(result);
  result = setMsgValues(self);
  checkErrOK(result);

  result = self->compMsgData->getMsgData(self, &msgData, &msgLgth);
  COMP_MSG_DBG(self, "B", 2, "getMsgData res: %d!msgLgth: %d!", result, msgLgth);
  checkErrOK(result);
//self->compMsgData->dumpMsg(self);
//self->compMsgData->compMsgDataView->dataView->dumpBinary(msgData, msgLgth, "dumpMsg");
  COMP_MSG_DBG(self, "B", 2, "encryption: %c", self->compMsgData->currHdr->hdrEncryption);
//self->compMsgData->compMsgDataView->dataView->dumpBinaryWide(msgData, msgLgth, "beforeencrypt");
  if (self->compMsgData->currHdr->hdrEncryption == 'E') {
    uint8_t *toCryptPtr;
    uint8_t *encryptedMsgData;
    size_t encryptedMsgDataLgth;
    size_t mlen;
    size_t headerLgth;
    size_t totalCrcOffset;
    int numericValue;

    result = self->compMsgModuleData->getCryptKey(self, &numericValue, &cryptKey);
    checkErrOK(result);
    ivlen = 16;
    klen = 16;

//self->compMsgData->compMsgDataView->dataView->dumpBinary(self->compMsgData->compMsgDataView->dataView->data, self->compMsgData->compMsgDataView->dataView->lgth, "MSG_AA");
    COMP_MSG_DBG(self, "B", 2, "need to encrypt message!");
    headerLgth = self->compMsgData->headerLgth;
    totalCrcOffset = 0;
    mlen = self->compMsgData->totalLgth - headerLgth;
    if (self->compMsgData->currHdr->hdrFlags & COMP_DISP_TOTAL_CRC) {
      totalCrcOffset = 1;
      mlen -= 1;
    } else {
      totalCrcOffset = 2;
      mlen -= 2;
    }
    COMP_MSG_DBG(self, "B", 2, "msglen!%d!mlen: %d, headerLgth!%d", self->compMsgData->totalLgth, mlen, self->compMsgData->headerLgth);
    toCryptPtr = msgData + self->compMsgData->headerLgth;
    result = self->compMsgUtil->encryptMsg(self, toCryptPtr, mlen, cryptKey, klen, cryptKey, ivlen, &encryptedMsgData, &encryptedMsgDataLgth);
    checkErrOK(result);
    if (encryptedMsgDataLgth != mlen) {
      COMP_MSG_DBG(self, "B", 1, "WARNING! mlen: %d encryptedMsgDataLgth: %d overwrites eventually totalCrc!", mlen, encryptedMsgDataLgth);
    }
    c_memcpy(toCryptPtr, encryptedMsgData, encryptedMsgDataLgth);
    COMP_MSG_DBG(self, "B", 2, "crypted: len: %d!mlen: %d!", encryptedMsgDataLgth, mlen);
  }
  // if we have a @totalCrc we need to set it here
  COMP_MSG_DBG(self, "B", 2, "totalCrc: %d\n", self->compMsgData->currHdr->hdrFlags & COMP_DISP_TOTAL_CRC);
  if (self->compMsgData->currHdr->hdrFlags & COMP_DISP_TOTAL_CRC) {
    fieldInfo = &self->compMsgData->fields[self->compMsgData->numFields - 1];
    result = self->compMsgData->compMsgDataView->setTotalCrc(self, self->compMsgData->compMsgDataView->dataView, fieldInfo);
    COMP_MSG_DBG(self, "B", 2, "setTotalCrc: result: %d fieldOffset: %d", result, fieldInfo->fieldOffset);
    checkErrOK(result);
  }
  // here we need to decide where and how to send the message!!
  // from currHdr we can see the handle type and - if needed - the @dst
  COMP_MSG_DBG(self, "B", 2, "transferType: %c dst: 0x%04x msgLgth: %d", self->compMsgData->currHdr->hdrHandleType, self->compMsgData->currHdr->hdrToPart, msgLgth);
//self->compMsgData->compMsgDataView->dataView->dumpBinaryWide(msgData, msgLgth, "afterencrypt");
  result = self->compMsgSendReceive->sendMsg(self, msgData, msgLgth);
  COMP_MSG_DBG(self, "B", 2, "buildMsg sendMsg has been called result: %d", result);
  checkErrOK(result);
//  result = self->resetMsgInfo(self, self->buildMsgInfos.parts);
  return result;
}

// ================================= forwardMsg ====================================

static uint8_t ICACHE_FLASH_ATTR forwardMsg(compMsgDispatcher_t *self) {
  uint8_t result;
  headerPart_t *hdr;
  int hdrIdx;
  msgParts_t *received;
  msgHeaderInfos_t *hdrInfos;

  COMP_MSG_DBG(self, "B", 2, "forwardMsg called handleType: %c!", self->compMsgData->currHdr->hdrHandleType);
  received = &self->compMsgData->received;
  COMP_MSG_DBG(self, "B", 2, "handleType: %c msgLgth: %d", self->compMsgData->currHdr->hdrHandleType, received->lgth);
  self->compMsgSendReceive->sendMsg(self, received->buf, received->totalLgth);
  return COMP_MSG_ERR_OK;
}

// ================================= compMsgBuildMsgInit ====================================

uint8_t compMsgBuildMsgInit(compMsgDispatcher_t *self) {
  self->compMsgBuildMsg->createMsgFromHeaderPart = &createMsgFromHeaderPart;
  self->compMsgBuildMsg->fixOffsetsForKeyValues = &fixOffsetsForKeyValues;
  self->compMsgBuildMsg->setMsgFieldValue = &setMsgFieldValue;
  self->compMsgBuildMsg->buildMsg = &buildMsg;
  self->compMsgBuildMsg->setMsgValues = &setMsgValues;
  self->compMsgBuildMsg->forwardMsg = &forwardMsg;

  return COMP_MSG_ERR_OK;
}

// ================================= newCompMsgBuildMsg ====================================

compMsgBuildMsg_t *newCompMsgBuildMsg() {
  compMsgBuildMsg_t *compMsgBuildMsg = os_zalloc(sizeof(compMsgBuildMsg_t));
  if (compMsgBuildMsg == NULL) {
    return NULL;
  }
  return compMsgBuildMsg;
}
