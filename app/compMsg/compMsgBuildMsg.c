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

// ================================= fixOffsetsForKeyValues ====================================

/**
 * \brief fix the field offsets of all if we have key/value fields after calling the callbacks for these fields.
 * \param self The dispatcher struct
 * \return Error code or ErrorOK
 *
 */
static uint8_t fixOffsetsForKeyValues(compMsgDispatcher_t *self) {
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
//ets_printf("§fixOffsetsForKeyValues: numFields: %d!§", compMsgData->numFields);
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
//ets_printf("§fixOffsetsForKeyValues: idx: %d!%p!%s!§", fieldIdx, msgDescPart->fieldSizeCallback, msgDescPart->fieldNameStr);
    if (msgDescPart->fieldSizeCallback != NULL) {
      // the key name must have the prefix: "#key_"!
      if (msgDescPart->fieldNameStr[0] != '#') {
        return COMP_DISP_ERR_FIELD_NOT_FOUND;
      }
      result = msgDescPart->fieldSizeCallback(self, &numericValue, &stringValue);
//ets_printf("fieldSizeCallback for: %s %d %p\n", msgDescPart->fieldNameStr, numericValue, stringValue);
      checkErrOK(result);
      if (msgKeyValueDescPart != NULL) {
        fieldInfo->fieldKey = msgKeyValueDescPart->keyId;
//ets_printf("fieldKey: %d\n", msgKeyValueDescPart->keyId);
      } else {
        fieldInfo->fieldKey = msgDescPart->fieldKey;
      }
      fieldInfo->fieldLgth = msgDescPart->fieldSize + 2 * sizeof(uint16_t) + sizeof(uint8_t); // for key, type and lgth in front of value!!
//ets_printf("fixOffsetsForKeyValues: %s size: %d lgth: %d\n", msgDescPart->fieldNameStr, msgDescPart->fieldSize, fieldInfo->fieldLgth);
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
//ets_printf("§fixOffsetsForKeyValues: done!§");
  return COMP_DISP_ERR_OK;
}

// ================================= setMsgFieldValue ====================================

/**
 * \brief the value of a message field
 * \param self The dispatcher struct
 * \param type The type of the answer
 * \return Error code or ErrorOK
 *
 */
static uint8_t setMsgFieldValue(compMsgDispatcher_t *self, uint8_t type) {
  uint8_t result;
  uint8_t *fieldNameStr;
  uint8_t *stringValue;
  int numericValue;
  compMsgData_t *compMsgData;

//ets_printf("§setMsgFieldValue: %s %s§\n", self->compMsgData->msgValPart->fieldNameStr, self->compMsgData->msgValPart->fieldValueStr);
  compMsgData = self->compMsgData;
  if (ets_strncmp(self->compMsgData->msgValPart->fieldValueStr, "@get", 4) == 0) {
    // call the callback function for the field!!
ets_printf("§setMsgFieldValue:cb %s!%p!size: %d§", self->compMsgData->msgValPart->fieldValueStr, self->compMsgData->msgValPart->fieldValueCallback, self->compMsgData->msgDescPart->fieldSize);
    fieldNameStr = self->compMsgData->msgValPart->fieldNameStr;
    if (self->compMsgData->msgValPart->fieldValueCallback != NULL) {
      result = self->compMsgData->msgValPart->fieldValueCallback(self, &numericValue, &stringValue);
//ets_printf("§fieldValueCallback: result: %d§\n", result);
      checkErrOK(result);
ets_printf("§cb field1: %s!value: 0x%04x %s!§", fieldNameStr, numericValue, stringValue == NULL ? "nil" : (char *)stringValue);
      self->compMsgData->msgValPart->fieldValue = numericValue;
    }
//ets_printf("§isNumber: %d§\n", self->compMsgData->msgValPart->fieldFlags & COMP_DISP_DESC_VALUE_IS_NUMBER);
    if ((self->compMsgData->msgValPart->fieldFlags & COMP_DISP_DESC_VALUE_IS_NUMBER) || (stringValue == NULL)) {
      stringValue = NULL;
      numericValue = self->compMsgData->msgValPart->fieldValue;
    } else {
      stringValue = self->compMsgData->msgValPart->fieldKeyValueStr;
      numericValue = 0;
    }
ets_printf("§cb field2: %s!value: 0x%04x %s!§", fieldNameStr, numericValue, stringValue == NULL ? "nil" : (char *)stringValue);
    result = self->compMsgData->setFieldValue(self, fieldNameStr, numericValue, stringValue);
  } else {
    fieldNameStr = self->compMsgData->msgValPart->fieldNameStr;
ets_printf("§fieldName: %s!id: %d!§", fieldNameStr, self->compMsgData->msgValPart->fieldNameId);
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
//ets_printf("§cmdKey value: 0x%04x§", numericValue);
        result = compMsgData->setFieldValue(self, fieldNameStr, numericValue, stringValue);
        break;
      default:
ets_printf("§fieldName: %s!value: 0x%04x %s§", fieldNameStr, numericValue, stringValue == NULL ? "nil" : (char *)stringValue);
        result = self->compMsgData->setFieldValue(self, fieldNameStr, numericValue, stringValue);
        break;
    }
    checkErrOK(result);
  }
//ets_printf("§setMsgFieldValue: done§");
  return COMP_DISP_ERR_OK;
}

// ================================= setMsgValues ====================================

/**
 * \brief set the field values of a message
 * \param self The dispatcher struct
 * \return Error code or ErrorOK
 *
 */
static uint8_t setMsgValues(compMsgDispatcher_t *self) {
  uint8_t startEntryIdx;
  int tableFieldIdx;
  uint8_t fieldLgth;
  uint8_t *flagStr;
  uint8_t flag;
  uint8_t type;
  unsigned long uval;
  compMsgDataView_t *dataView;
  uint8_t result;
  uint8_t numTableRows;
  uint8_t numTableRowFields;
  int msgCmdKey;
  uint8_t msgDescPartIdx;
  uint8_t msgValPartIdx;
  compMsgField_t *fieldInfo;
  msgDescPart_t *msgDescPart;
  msgValPart_t *msgValPart;
  compMsgData_t *compMsgData;
  uint8_t *handle;
  char *endPtr;
  int numericValue;
  uint8_t *stringValue;

//ets_printf("§setMsgValues§\n");
  compMsgData = self->compMsgData;
  handle = self->msgHandle;
  type = 'A';
  dataView = compMsgData->compMsgDataView;
  // loop over MSG Fields, to check if we eventually have table rows!!
  msgDescPartIdx = 0;
  msgValPartIdx = 0;
  msgValPart = &self->compMsgData->msgValParts[msgValPartIdx];
  tableFieldIdx = 0;
  compMsgData = self->compMsgData;
//ets_printf("§numFields: %d numMsgValParts: %d§", compMsgData->numFields, self->compMsgData->numMsgValParts);
  while ((msgDescPartIdx < compMsgData->numFields) && (msgValPartIdx <= self->compMsgData->numMsgValParts)) {
    msgDescPart = &self->compMsgData->msgDescParts[msgDescPartIdx];
    self->compMsgData->msgDescPart = msgDescPart;
    msgValPart = &self->compMsgData->msgValParts[msgValPartIdx];
    self->compMsgData->msgValPart = msgValPart;
//ets_printf("§setMsgValues: %s§", msgDescPart->fieldNameStr);
//ets_printf("setMsgValuesFromLines2: fieldIdx: %d tableFieldIdx: %d entryIdx: %d numFields:%d \n", fieldIdx, tableFieldIdx, entryIdx, compMsgData->numFields);
//ets_printf("fieldIdx: %d entryIdx: %d numtableRows: %d\n", fieldIdx, entryIdx, numTableRows);
    fieldInfo = &compMsgData->fields[msgDescPartIdx++];
//ets_printf("fieldNameId: %d numtabrows: %d\n", fieldInfo->fieldNameId, numTableRows);
//ets_printf("default fieldNameId: %d buildMsgInfo fieldNameId: %d\n", fieldInfo->fieldNameId, self->buildMsgInfos.fieldNameId);
    if (fieldInfo->fieldNameId == msgValPart->fieldNameId) {
      result = self->setMsgFieldValue(self, type);
      checkErrOK(result);
      msgValPartIdx++;
    }
  }
  numericValue = compMsgData->currHdr->hdrU16CmdKey;
  stringValue = NULL;
ets_printf("§cmdKey value: 0x%04x§", numericValue);
  result = compMsgData->setFieldValue(self, "@cmdKey", numericValue, stringValue);
//ets_printf("§cmdKey result: %d§", result);
  checkErrOK(result);
ets_printf("§setMsgvalues done§");
  compMsgData->prepareMsg(self);
  checkErrOK(result);
ets_printf("§");
compMsgData->dumpMsg(self);
ets_printf("§");
  return COMP_DISP_ERR_OK;
}

// ================================= buildMsg ====================================

/**
 * \brief Build a message for sending to Uart or socket
 * 
 * \param self The dispatcher struct
 * \return Error code or ErrorOK
 *
 */
static uint8_t buildMsg(compMsgDispatcher_t *self) {
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
  
ets_printf("§buildMsg§\n");
  result = self->fixOffsetsForKeyValues(self);
  checkErrOK(result);
  self->compMsgData->direction = COMP_MSG_TO_SEND_DATA;
  result = self->compMsgData->initMsg(self);
//ets_printf("§heap2: %d§", system_get_free_heap_size());
  result = setMsgValues(self);
  checkErrOK(result);

  result = self->compMsgData->getMsgData(self, &msgData, &msgLgth);
ets_printf("§getMsgData res: %d!msgLgth: %d!§", result, msgLgth);
  checkErrOK(result);
//self->compMsgData->dumpMsg(self);
//ets_printf("§");
//self->compMsgData->compMsgDataView->dataView->dumpBinary(msgData, msgLgth, "dumpMsg");
//ets_printf("§");
//ets_printf("§encryption: %c§", self->compMsgData->currHdr->hdrEncryption);
  if (self->compMsgData->currHdr->hdrEncryption == 'E') {
    uint8_t *toCryptPtr;
    uint8_t *encryptedMsgData;
    size_t encryptedMsgDataLgth;
    uint16_t mlen;
    uint8_t headerLgth;
    size_t totalCrcOffset;

    cryptKey = "a1b2c3d4e5f6g7h8";
    ivlen = 16;
    klen = 16;

//self->compMsgData->compMsgDataView->dataView->dumpBinary(self->compMsgData->compMsgDataView->dataView->data, self->compMsgData->compMsgDataView->dataView->lgth, "MSG_AA");
//ets_printf("need to encrypt message!\n");
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
//ets_printf("msglen!%d!mlen: %d, headerLgth!%d\n", self->compMsgData->totalLgth, mlen, self->compMsgData->headerLgth);
    toCryptPtr = msgData + self->compMsgData->headerLgth;
    result = self->encryptMsg(toCryptPtr, mlen, cryptKey, klen, cryptKey, ivlen, &encryptedMsgData, &encryptedMsgDataLgth);
    checkErrOK(result);
    if (encryptedMsgDataLgth != mlen) {
ets_printf("WARNING! mlen: %d encryptedMsgDataLgth: %d overwrites eventually totalCrc!\n", mlen, encryptedMsgDataLgth);
    }
    c_memcpy(toCryptPtr, encryptedMsgData, encryptedMsgDataLgth);
//ets_printf("§crypted: len: %d!mlen: %d!§", encryptedMsgDataLgth, mlen);
  }
  // if we have a @totalCrc we need to set it here
//ets_printf("§totalCrc: %d§", self->compMsgData->currHdr->hdrFlags & COMP_DISP_TOTAL_CRC);
  if (self->compMsgData->currHdr->hdrFlags & COMP_DISP_TOTAL_CRC) {
    fieldInfo = &self->compMsgData->fields[self->compMsgData->numFields - 1];
    result = self->compMsgData->compMsgDataView->setTotalCrc(self->compMsgData->compMsgDataView, fieldInfo);
//ets_printf("§setTotalCrc: result: %d fieldOffset: %d§", result, fieldInfo->fieldOffset);
    checkErrOK(result);
  }
  // here we need to decide where and how to send the message!!
  // from currHdr we can see the handle type and - if needed - the @dst
//ets_printf("§transferType: %c dst: 0x%04x msgLgth: %d§", self->compMsgData->currHdr->hdrHandleType, self->compMsgData->currHdr->hdrToPart, msgLgth);
  result = self->sendMsg(self, msgData, msgLgth);
//ets_printf("§buildMsg sendMsg has been called result: %d§", result);
  checkErrOK(result);
//  result = self->resetMsgInfo(self, self->buildMsgInfos.parts);
  return result;
}

// ================================= forwardMsg ====================================

static uint8_t forwardMsg(compMsgDispatcher_t *self) {
  uint8_t result;
  headerPart_t *hdr;
  int hdrIdx;
  msgParts_t *received;
  msgHeaderInfos_t *hdrInfos;

ets_printf("§forwardMsg called handleType: %c!§", self->compMsgData->currHdr->hdrHandleType);
  received = &self->compMsgData->received;
ets_printf("§handleType: %c msgLgth: %d§", self->compMsgData->currHdr->hdrHandleType, received->lgth);
  self->sendMsg(self, received->buf, received->totalLgth);
  return COMP_MSG_ERR_OK;
}

// ================================= compMsgBuildMsgInit ====================================

uint8_t compMsgBuildMsgInit(compMsgDispatcher_t *self) {
  self->fixOffsetsForKeyValues = &fixOffsetsForKeyValues;
  self->setMsgFieldValue = &setMsgFieldValue;
  self->buildMsg = &buildMsg;
  self->setMsgValues = &setMsgValues;
  self->forwardMsg = &forwardMsg;
  return COMP_DISP_ERR_OK;
}
