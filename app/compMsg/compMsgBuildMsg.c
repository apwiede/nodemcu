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

static uint8_t fixOffsetsForKeyValues(compMsgDispatcher_t *self) {
  uint8_t result;
  uint8_t msgDescPartIdx;
  uint8_t fieldIdx;
  uint8_t type;
  compMsgField_t *fieldInfo;
  msgDescPart_t *msgDescPart;
  compMsgData_t *compMsgData;

  compMsgData = self->compMsgData;
  fieldIdx = 0;
  msgDescPartIdx = 0;
  while (fieldIdx < compMsgData->numFields) {
    fieldInfo = &compMsgData->fields[fieldIdx];
    msgDescPart = &self->compMsgData->compMsgMsgDesc->msgDescParts[msgDescPartIdx];
    self->compMsgData->msgDescPart = msgDescPart;
    if (msgDescPart->getFieldSizeCallback != NULL) {
      // the key name must have the prefix: "#key_"!
      if (msgDescPart->fieldNameStr[0] != '#') {
        return COMP_DISP_ERR_FIELD_NOT_FOUND;
      }
      result = msgDescPart->getFieldSizeCallback(self);
      checkErrOK(result);
      fieldInfo->fieldKey = msgDescPart->fieldKey;
      msgDescPart->fieldSize += 2 * sizeof(uint16_t) + sizeof(uint8_t); // for key, type and lgth in front of value!!
      fieldInfo->fieldLgth = msgDescPart->fieldSize;
    }
    msgDescPartIdx++;
    fieldIdx++;
  }
  return COMP_DISP_ERR_OK;
}

// ================================= setMsgFieldValue ====================================

static uint8_t setMsgFieldValue(compMsgDispatcher_t *self, uint8_t *numTableRows, uint8_t *numTableRowFields, uint8_t type) {
  uint8_t result;
  int currTableRow;
  int currTableCol;
  uint8_t *fieldNameStr;
  uint8_t *stringValue;
  int numericValue;
  compMsgData_t *compMsgData;

//ets_printf("setMsgFieldValue: %s %s\n", self->msgValPart->fieldNameStr, self->msgValPart->fieldValueStr);
  compMsgData = self->compMsgData;
  if (self->compMsgData->msgValPart->fieldValueStr[0] == '@') {
    // call the callback function for the field!!
    if (*numTableRows > 0) {
      currTableRow = 0;
      currTableCol = 0;
      while (currTableRow < *numTableRows) {
        self->compMsgData->buildMsgInfos.tableRow = currTableRow;
        self->compMsgData->buildMsgInfos.tableCol = currTableCol;
        result = self->fillMsgValue(self, self->compMsgData->msgValPart->fieldValueStr, type, self->compMsgData->msgDescPart->fieldTypeId);
        checkErrOK(result);
        result = compMsgData->setTableFieldValue(compMsgData, self->compMsgData->msgValPart->fieldNameStr, currTableRow, self->compMsgData->buildMsgInfos.numericValue, self->compMsgData->buildMsgInfos.stringValue);
        currTableRow++;
      }
      currTableRow = 0;
      currTableCol++;
      if (currTableCol > *numTableRowFields) {
        // table rows done
        *numTableRows = 0;
        *numTableRowFields = 0;
        currTableRow = 0;
        currTableCol = 0;
      }
    } else {
      if (self->compMsgData->msgValPart->getFieldValueCallback != NULL) {
        result = self->compMsgData->msgValPart->getFieldValueCallback(self);
        checkErrOK(result);
      }
      fieldNameStr = self->compMsgData->msgValPart->fieldNameStr;
      if (self->compMsgData->msgValPart->fieldFlags & COMP_DISP_DESC_VALUE_IS_NUMBER) {
        stringValue = NULL;
        numericValue = self->compMsgData->msgValPart->fieldValue;
      } else {
        stringValue = self->compMsgData->msgValPart->fieldKeyValueStr;
        numericValue = 0;
      }
      result = self->compMsgData->setFieldValue(compMsgData, fieldNameStr, numericValue, stringValue);
//      result = self->fillMsgValue(self, self->buildMsgInfos.fieldValueStr, type, self->buildMsgInfos.fieldTypeId);
//      checkErrOK(result);
//      result = compMsgData->setFieldValue(compMsgData, self->buildMsgInfos.fieldNameStr, self->buildMsgInfos.numericValue, self->buildMsgInfos.stringValue);
//      currTableRow++;
    }
  } else {
    fieldNameStr = self->compMsgData->msgValPart->fieldNameStr;
    if (self->compMsgData->msgValPart->fieldFlags & COMP_DISP_DESC_VALUE_IS_NUMBER) {
      stringValue = NULL;
      numericValue = self->compMsgData->msgValPart->fieldValue;
    } else {
      stringValue = self->compMsgData->msgValPart->fieldValueStr;
      numericValue = 0;
    }
    switch (self->compMsgData->msgValPart->fieldNameId) {
      case COMP_MSG_SPEC_FIELD_DST:
        result = compMsgData->setFieldValue(compMsgData, fieldNameStr, numericValue, stringValue);
        break;
      case COMP_MSG_SPEC_FIELD_SRC:
        result = compMsgData->setFieldValue(compMsgData, fieldNameStr, numericValue, stringValue);
        break;
      case COMP_MSG_SPEC_FIELD_CMD_KEY:
        numericValue = self->compMsgData->currHdr->hdrU16CmdKey;
        stringValue = NULL;
        result = compMsgData->setFieldValue(compMsgData, fieldNameStr, numericValue, stringValue);
        break;
      default:
        result = self->compMsgData->setFieldValue(compMsgData, fieldNameStr, numericValue, stringValue);
        break;
    }
    checkErrOK(result);
  }
//ets_printf("setMsgFieldValue: done\n");
  return COMP_DISP_ERR_OK;
}

// ================================= setMsgValues ====================================

static uint8_t setMsgValues(compMsgDispatcher_t *self) {
  uint8_t startEntryIdx;
  int tableFieldIdx;
  uint8_t fieldLgth;
  uint8_t *flagStr;
  uint8_t flag;
  uint8_t type;
  unsigned long uval;
  compMsgDataView_t *dataView;
  int result;
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

//ets_printf("setMsgValues\n");
  compMsgData = self->compMsgData;
  handle = self->msgHandle;
  type = 'A';
  dataView = compMsgData->compMsgDataView;
  // loop over MSG Fields, to check if we eventually have table rows!!
  msgDescPartIdx = 0;
  msgValPartIdx = 0;
  msgValPart = &self->compMsgData->compMsgMsgDesc->msgValParts[msgValPartIdx];
  tableFieldIdx = 0;
  numTableRows = 0;
  numTableRowFields = 0;
  compMsgData = self->compMsgData;
  while ((msgDescPartIdx < compMsgData->numFields) && (msgValPartIdx <= self->compMsgData->compMsgMsgDesc->numMsgValParts)) {
    msgDescPart = &self->compMsgData->compMsgMsgDesc->msgDescParts[msgDescPartIdx];
    self->compMsgData->msgDescPart = msgDescPart;
    msgValPart = &self->compMsgData->compMsgMsgDesc->msgValParts[msgValPartIdx];
    self->compMsgData->msgValPart = msgValPart;
//ets_printf("setMsgValues: %s\n", msgDescPart->fieldNameStr);
//ets_printf("setMsgValuesFromLines2: fieldIdx: %d tableFieldIdx: %d entryIdx: %d numFields:%d \n", fieldIdx, tableFieldIdx, entryIdx, compMsgData->numFields);
//ets_printf("fieldIdx: %d entryIdx: %d numtableRows: %d\n", fieldIdx, entryIdx, numTableRows);
    if (numTableRows > 0) {
      fieldInfo = &compMsgData->tableFields[tableFieldIdx++];
    } else {
      fieldInfo = &compMsgData->fields[msgDescPartIdx++];
    }
//ets_printf("fieldNameId: %d numtabrows: %d\n", fieldInfo->fieldNameId, numTableRows);
    switch (fieldInfo->fieldNameId) {
    case COMP_MSG_SPEC_FIELD_TABLE_ROW_FIELDS:
      numTableRows = compMsgData->numTableRows;
      numTableRowFields = compMsgData->numTableRowFields;
      break;
    default:
//ets_printf("default fieldNameId: %d buildMsgInfo fieldNameId: %d\n", fieldInfo->fieldNameId, self->buildMsgInfos.fieldNameId);
      if (fieldInfo->fieldNameId == msgValPart->fieldNameId) {
        result = self->setMsgFieldValue(self, &numTableRows, &numTableRowFields, type);
        checkErrOK(result);
        msgValPartIdx++;
      }
      break;
    }
  }
  msgCmdKey = self->compMsgData->currHdr->hdrU16CmdKey;
  result = compMsgData->setFieldValue(compMsgData, "@cmdKey", msgCmdKey, NULL);
  checkErrOK(result);

  compMsgData->prepareMsg(compMsgData);
  checkErrOK(result);
compMsgData->dumpMsg(compMsgData);
  return COMP_DISP_ERR_OK;
}

// ================================= buildMsg ====================================

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
  int src;
  int dst;

////ets_printf("buildMsg\n");
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
  
//ets_printf("buildMsg\n");
  result = self->fixOffsetsForKeyValues(self);
  checkErrOK(result);
  result = self->compMsgData->initMsg(self->compMsgData);
//ets_printf("heap2: %d\n", system_get_free_heap_size());
  result = setMsgValues(self);
  checkErrOK(result);

  result = self->compMsgData->getMsgData(self->compMsgData, &msgData, &msgLgth);
  checkErrOK(result);
  if (self->compMsgData->currHdr->hdrEncryption == 'E') {
    uint8_t *toCryptPtr;
    uint8_t *encryptedMsgData;
    size_t encryptedMsgDataLgth;
    uint16_t mlen;
    uint8_t headerLgth;

    cryptKey = "a1b2c3d4e5f6g7h8";
    ivlen = 16;
    klen = 16;

ets_printf("need to encrypt message!\n");
    headerLgth = self->compMsgData->headerLgth;
    mlen = self->compMsgData->totalLgth - headerLgth;
ets_printf("msglen!%d!mlen: %d, headerLgth!%d\n", self->compMsgData->totalLgth, mlen, self->compMsgData->headerLgth);
    toCryptPtr = msgData + self->compMsgData->headerLgth;
    result = self->encryptMsg(toCryptPtr, mlen, cryptKey, klen, cryptKey, ivlen, &encryptedMsgData, &encryptedMsgDataLgth);
    checkErrOK(result);
    c_memcpy(toCryptPtr, encryptedMsgData, encryptedMsgDataLgth);
ets_printf("crypted: len: %d!mlen: %d!\n", encryptedMsgDataLgth, mlen);
  }
    
  // here we need to decide where and how to send the message!!
  // from currHdr we can see the handle type and - if needed - the @dst
ets_printf("transferType: %c dst: 0x%04x\n", self->compMsgData->currHdr->hdrHandleType, self->compMsgData->currHdr->hdrToPart);
  result = self->sendMsg(self, msgData, msgLgth);
ets_printf("buildMsg sendMsg has been called result: %d\n", result);
  checkErrOK(result);
//  result = self->resetMsgInfo(self, self->buildMsgInfos.parts);
  return result;
}

// ================================= compMsgBuildMsgInit ====================================

uint8_t compMsgBuildMsgInit(compMsgDispatcher_t *self) {
  self->fixOffsetsForKeyValues = &fixOffsetsForKeyValues;
  self->setMsgFieldValue = &setMsgFieldValue;
  self->buildMsg = &buildMsg;
  self->setMsgValues = &setMsgValues;
  return COMP_DISP_ERR_OK;
}
