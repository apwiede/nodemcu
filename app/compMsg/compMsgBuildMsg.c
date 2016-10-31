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
  uint8_t msgValPartIdx;
  uint8_t fieldIdx;
  uint8_t tabFieldIdx;
  uint8_t numTableRows;
  uint8_t numTableCols;
  uint8_t tabRowIdx;
  uint8_t tabColIdx;
  uint8_t numKeyValues;
  uint8_t keyValueIdx;
  uint8_t actionMode;
  uint8_t type;
  long uval;
  uint8_t *cp;
  char *endPtr;
  compMsgField_t *fieldInfo;
  msgDescPart_t *msgDescPart;
  msgValPart_t *msgValPart;
  compMsgData_t *compMsgData;

ets_printf("fixOffsetsForKeyValues called\n");
  compMsgData = self->compMsgData;
  fieldIdx = 0;
  tabFieldIdx = 0;
  numTableRows = 0;
  numTableCols = 0;
  tabRowIdx = 0;
  tabColIdx = 0;
  msgDescPartIdx = 0;
  msgValPartIdx = 0;
  while (fieldIdx < compMsgData->numFields) {
    fieldInfo = &compMsgData->fields[fieldIdx];
    msgDescPart = &self->compMsgMsgDesc->msgDescParts[msgDescPartIdx];
    switch (msgDescPart->fieldNameId) {
    case COMP_MSG_SPEC_FIELD_TABLE_ROWS:
ets_printf("@tableRows: lgth: %d\n", fieldInfo->fieldLgth);
      break;
    case COMP_MSG_SPEC_FIELD_TABLE_ROW_FIELDS:
ets_printf("@tableRowFields: lgth: %d\n", fieldInfo->fieldLgth);
      break;
    case COMP_MSG_SPEC_FIELD_NUM_KEY_VALUES:
ets_printf("@numKeyValues: lgth: %d scanLgth: %d\n", fieldInfo->fieldLgth, self->bssScanInfos->numScanInfos);
      msgValPartIdx = 0;
      while (msgValPartIdx < self->compMsgMsgDesc->numMsgValParts) {
        msgValPart = &self->compMsgMsgDesc->msgValParts[msgValPartIdx];
ets_printf("msgValPart: %s %d %d\n", msgValPart->fieldNameStr, msgValPart->fieldNameId, COMP_MSG_SPEC_FIELD_NUM_KEY_VALUES);
        if (msgValPart->fieldNameId == COMP_MSG_SPEC_FIELD_NUM_KEY_VALUES) {
          uval = c_strtoul(msgValPart->fieldValueStr, &endPtr, 10);
ets_printf("uval: %d %d %d\n", uval, (endPtr - (char *)msgValPart->fieldValueStr), c_strlen(msgValPart->fieldValueStr));
          if ((endPtr - (char *)msgValPart->fieldValueStr) != c_strlen(msgValPart->fieldValueStr)) {
            return COMP_DISP_ERR_BAD_VALUE;
          }
          numKeyValues = (uint8_t)uval;
ets_printf("numKeyValues: %d\n", numKeyValues);
          msgValPartIdx++;
          fieldIdx++;
          keyValueIdx = 0;
          while (keyValueIdx < numKeyValues) {
            fieldInfo = &compMsgData->fields[fieldIdx];
            msgValPart = &self->compMsgMsgDesc->msgValParts[msgValPartIdx];
            // the key name must have the prefix: "#key_"!
            cp = msgValPart->fieldNameStr + c_strlen("#key_");
ets_printf("key: %s callback: :%s\n", cp, msgValPart->fieldValueStr + 1); // strip off the @
            self->buildMsgInfos.fieldNameStr = cp;
            self->buildMsgInfos.numericValue = 0;
            result = self->getActionMode(self, msgValPart->fieldValueStr + 1, &actionMode);
            self->actionMode = actionMode;
            checkErrOK(result);
            result  = self->runAction(self, &type);
            checkErrOK(result);
ets_printf("keyValue: keyId: %d keySize: %d\n", self->buildMsgInfos.numericValue, self->buildMsgInfos.sizeValue);
            fieldInfo->fieldKey = self->buildMsgInfos.numericValue;
            fieldInfo->fieldLgth = self->buildMsgInfos.sizeValue;
            msgValPartIdx++;
            keyValueIdx++;
            fieldIdx++;
          }
          fieldIdx--; // below there is another increment!!
        } else {
          msgValPartIdx++;
        }
      }
      break;
    default:
      break;
    }

    msgDescPartIdx++;
    fieldIdx++;
  }
  return COMP_DISP_ERR_OK;
}

// ================================= setMsgKeyValue ====================================

static uint8_t setMsgKeyValues(compMsgDispatcher_t *self, uint8_t numEntries, uint8_t *entryIdx, uint8_t type, size_t *extraOffset) {
  uint8_t result;
  uint8_t actionMode;

  self->buildMsgInfos.actionName = self->buildMsgInfos.fieldValueStr+1;
  c_memcpy(self->buildMsgInfos.key, self->buildMsgInfos.fieldNameStr, c_strlen(self->buildMsgInfos.fieldNameStr));
  result = self->getActionMode(self, self->buildMsgInfos.fieldValueStr+1, &actionMode);
  self->actionMode = actionMode;
  checkErrOK(result);
  result  = self->runAction(self, &type);
  checkErrOK(result);
  *extraOffset += 11;
  while (*entryIdx < numEntries) {
//    result = self->compMsgMsgDesc->getFieldInfoFromLine(self);
//    checkErrOK(result);
    if (self->buildMsgInfos.fieldNameStr[0] != '#') {
      // end of key value pairs
      return COMP_MSG_ERR_OK;
    }
    (*entryIdx)++;
    self->buildMsgInfos.actionName = self->buildMsgInfos.fieldValueStr+1;
    c_memcpy(self->buildMsgInfos.key, self->buildMsgInfos.fieldNameStr, c_strlen(self->buildMsgInfos.fieldNameStr));
    result = self->getActionMode(self, self->buildMsgInfos.fieldValueStr+1, &actionMode);
    self->actionMode = actionMode;
    checkErrOK(result);
    result  = self->runAction(self, &type);
    checkErrOK(result);
    *extraOffset += 11;
  }
  return result;
}


// ================================= setMsgFieldValue ====================================

static uint8_t setMsgFieldValue(compMsgDispatcher_t *self, uint8_t *numTableRows, uint8_t *numTableRowFields, bool *nextFieldEntry, uint8_t type) {
  uint8_t result;
  int currTableRow;
  int currTableCol;
  uint8_t *fieldNameStr;
  uint8_t *stringValue;
  int numericValue;
  compMsgData_t *compMsgData;

  compMsgData = self->compMsgData;
  *nextFieldEntry = false;
  if (self->buildMsgInfos.fieldValueStr[0] == '@') {
    // call the callback function for the field!!
    if (*numTableRows > 0) {
      currTableRow = 0;
      currTableCol = 0;
      while (currTableRow < *numTableRows) {
        self->buildMsgInfos.tableRow = currTableRow;
        self->buildMsgInfos.tableCol = currTableCol;
        result = self->fillMsgValue(self, self->buildMsgInfos.fieldValueStr, type, self->buildMsgInfos.fieldTypeId);
        checkErrOK(result);
        result = compMsgData->setTableFieldValue(compMsgData, self->buildMsgInfos.fieldNameStr, currTableRow, self->buildMsgInfos.numericValue, self->buildMsgInfos.stringValue);
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
      result = self->fillMsgValue(self, self->buildMsgInfos.fieldValueStr, type, self->buildMsgInfos.fieldTypeId);
      checkErrOK(result);
      result = compMsgData->setFieldValue(compMsgData, self->buildMsgInfos.fieldNameStr, self->buildMsgInfos.numericValue, self->buildMsgInfos.stringValue);
      currTableRow++;
    }
  } else {
    fieldNameStr = self->buildMsgInfos.fieldNameStr;
    stringValue = self->buildMsgInfos.stringValue;
    numericValue = self->buildMsgInfos.numericValue;
    switch (self->buildMsgInfos.fieldNameId) {
      case COMP_MSG_SPEC_FIELD_DST:
        result = compMsgData->setFieldValue(compMsgData, fieldNameStr, numericValue, stringValue);
        break;
      case COMP_MSG_SPEC_FIELD_SRC:
        result = compMsgData->setFieldValue(compMsgData, fieldNameStr, numericValue, stringValue);
        break;
      case COMP_MSG_SPEC_FIELD_CMD_KEY:
        // check for u8CmdKey/u16CmdKey here
ets_printf("cmdKey: 0x%04x\n", self->received.u16CmdKey);
        self->buildMsgInfos.numericValue = self->received.u16CmdKey;
        self->buildMsgInfos.stringValue = NULL;
        result = compMsgData->setFieldValue(compMsgData, fieldNameStr, numericValue, stringValue);
        break;
      default:
        result = self->compMsgData->setFieldValue(compMsgData, fieldNameStr, numericValue, stringValue);
        break;
    }
    checkErrOK(result);
    *nextFieldEntry = true;
  }
  return COMP_DISP_ERR_OK;
}

// ================================= setMsgValues ====================================

static uint8_t setMsgValues(compMsgDispatcher_t *self) {
  uint8_t startEntryIdx;
  int tableFieldIdx;
  uint8_t fieldLgth;
  uint8_t *flagStr;
  uint8_t flag;
  unsigned long uval;
  compMsgDataView_t *dataView;
  int result;
  uint8_t numTableRows;
  uint8_t numTableRowFields;
  int msgCmdKey;
  bool nextFieldEntry;
  bool fixOffset;
  size_t extraOffset;
  uint8_t msgDescPartIdx;
  uint8_t msgValPartIdx;
  compMsgField_t *fieldInfo;
  msgDescPart_t *msgDescPart;
  msgValPart_t *msgValPart;
  compMsgData_t *compMsgData;
  uint8_t *handle;
  uint8_t type;

  compMsgData = self->compMsgData;
  handle = self->msgHandle;
  type = self->buildMsgInfos.type;
  dataView = compMsgData->compMsgDataView;
  // loop over MSG Fields, to check if we eventaully have table rows!!
  msgDescPartIdx = 0;
  msgValPartIdx = 0;
  msgValPart = &self->compMsgMsgDesc->msgValParts[msgValPartIdx];
  fixOffset = false;
  extraOffset = 0;
  tableFieldIdx = 0;
  numTableRows = 0;
  numTableRowFields = 0;
  compMsgData = self->compMsgData;
ets_printf("setMsgValuesFromLines: numFields:%d numRows: %d\n", compMsgData->numFields, self->buildMsgInfos.numRows);
  while ((msgDescPartIdx < compMsgData->numFields) && (msgValPartIdx <= self->compMsgMsgDesc->numMsgValParts)) {
    msgDescPart = &self->compMsgMsgDesc->msgDescParts[msgDescPartIdx];
//ets_printf("setMsgValuesFromLines2: fieldIdx: %d tableFieldIdx: %d entryIdx: %d numFields:%d \n", fieldIdx, tableFieldIdx, entryIdx, compMsgData->numFields);
//ets_printf("fieldIdx: %d entryIdx: %d numtableRows: %d\n", fieldIdx, entryIdx, numTableRows);
    if (numTableRows > 0) {
      fieldInfo = &compMsgData->tableFields[tableFieldIdx++];
    } else {
      fieldInfo = &compMsgData->fields[msgDescPartIdx++];
    }
//ets_printf("fieldNameId: %d numtabrows: %d\n", fieldInfo->fieldNameId, numTableRows);
    if (fixOffset) {
ets_printf("fixOffset: msgDescPartIdx: %d extraOffset: %d\n", msgDescPartIdx, extraOffset);
      fieldInfo->fieldOffset += extraOffset;
    }
    switch (fieldInfo->fieldNameId) {
    case COMP_MSG_SPEC_FIELD_TABLE_ROW_FIELDS:
      numTableRows = compMsgData->numTableRows;
      numTableRowFields = compMsgData->numTableRowFields;
      break;
    case COMP_MSG_SPEC_FIELD_NUM_KEY_VALUES:
ets_printf("need to handle NUM_KEY_VALUES\n");
#ifdef NOTDEF
      startEntryIdx = entryIdx;
      compMsgData->numValueFields = self->buildMsgInfos.numRows;
      result = self->setMsgKeyValues(self, self->compMsgMsgDesc->numMsgValParts, &entryIdx, type, &extraOffset);
      checkErrOK(result);
//      result = compMsgData->setFieldValue(compMsgData, "@numKeyValues", (int)(entryIdx-startEntryIdx+1), NULL);
//      checkErrOK(result);
//      if ((entryIdx-startEntryIdx) > 0) {
//        compMsgData->totalLgth += extraOffset;
//        compMsgData->cmdLgth += extraOffset;
//        fixOffset = true;
//      }
#endif
      break;
    default:
//ets_printf("default fieldNameId: %d buildMsgInfo fieldNameId: %d\n", fieldInfo->fieldNameId, self->buildMsgInfos.fieldNameId);
      if (fieldInfo->fieldNameId == self->buildMsgInfos.fieldNameId) {
        result = self->setMsgFieldValue(self, &numTableRows, &numTableRowFields, &nextFieldEntry, type);
        checkErrOK(result);
        if (nextFieldEntry) {
//          if (entryIdx < numEntries) {
//            result = self->compMsgMsgDesc->getFieldInfoFromLine(self);
//            checkErrOK(result);
//          }
          msgValPartIdx++;
        }
      }
      break;
    }
  }
ets_printf("u16cmdKey: 0x%04x\n", self->buildMsgInfos.u16CmdKey);
  msgCmdKey = (self->buildMsgInfos.u16CmdKey & 0xFF00) | type & 0xFF;
  result = compMsgData->setFieldValue(compMsgData, "@cmdKey", msgCmdKey, NULL);
  checkErrOK(result);
  compMsgData->prepareMsg(compMsgData);
  checkErrOK(result);
compMsgData->dumpMsg(compMsgData);
  return COMP_DISP_ERR_OK;
}

// ================================= buildListMsg ====================================

static uint8_t buildListMsg(compMsgDispatcher_t *self, size_t *totalLgth, uint8_t **totalData) {
  int result;
  char *cp;
  char *cp2;
  int lgth;
  size_t listMsgHeaderLgth;
  size_t msgsLgth;
  compMsgData_t *compMsgData;

  result = COMP_MSG_ERR_OK;

  compMsgData = self->compMsgData;
  compMsgData->listSrc = self->buildListMsgInfos.src; 
  compMsgData->listDst = self->buildListMsgInfos.dst; 
  compMsgData->numListMsgs = 2;
  compMsgData->listMsgSizes = os_zalloc(compMsgData->numListMsgs * sizeof(uint16_t));
  checkAllocOK(compMsgData->listMsgSizes);
  msgsLgth = 0;
  compMsgData->listMsgSizes[0] = self->buildListMsgInfos.defHeaderLgth + self->buildListMsgInfos.encryptedDefDataLgth;
  msgsLgth += compMsgData->listMsgSizes[0];
  compMsgData->listMsgSizes[1] = self->buildListMsgInfos.msgHeaderLgth + self->buildListMsgInfos.encryptedMsgDataLgth;
  msgsLgth += compMsgData->listMsgSizes[1];
  compMsgData->listMsgs = os_zalloc(msgsLgth);
  checkAllocOK(compMsgData->listMsgSizes);
  cp = (char *)compMsgData->listMsgs;
  cp2 = cp;
  c_memcpy(cp, self->buildListMsgInfos.defData, self->buildListMsgInfos.defHeaderLgth);
  cp += self->buildListMsgInfos.defHeaderLgth;
  c_memcpy(cp, self->buildListMsgInfos.encryptedDefData, self->buildListMsgInfos.encryptedDefDataLgth);
  cp += self->buildListMsgInfos.encryptedDefDataLgth;
  c_memcpy(cp, self->buildListMsgInfos.msgData, self->buildListMsgInfos.msgHeaderLgth);
  cp += self->buildListMsgInfos.msgHeaderLgth;
  c_memcpy(cp, self->buildListMsgInfos.encryptedMsgData, self->buildListMsgInfos.encryptedMsgDataLgth);
  cp += self->buildListMsgInfos.encryptedMsgDataLgth;
  compMsgData->listMsgsSize = cp - cp2;

#ifdef NOTDEF
  listMsgHeaderLgth = 6;
  // FIXME eventually need to add extra length to second headerLgth!!
  *totalLgth = listMsgHeaderLgth;
  *totalLgth += self->buildListMsgInfos.defHeaderLgth + self->buildListMsgInfos.encryptedDefDataLgth;
  *totalLgth += self->buildListMsgInfos.msgHeaderLgth + self->buildListMsgInfos.encryptedMsgDataLgth;
ets_printf("lmhl: %d defhl: %d eddl: %d\n", listMsgHeaderLgth, self->buildListMsgInfos.defHeaderLgth, self->buildListMsgInfos.encryptedDefDataLgth);
ets_printf("msghl: %d emdl: %d tl: %d\n", self->buildListMsgInfos.msgHeaderLgth, self->buildListMsgInfos.encryptedMsgDataLgth, *totalLgth);
  *totalData = os_zalloc(*totalLgth);
ets_printf("totalLgth: %d 0x%04x, encryptedDefDataLgth: %d 0x%04x stotal: %d 0x%04x\n", *totalLgth, *totalLgth, self->buildListMsgInfos.encryptedDefDataLgth, self->buildListMsgInfos.encryptedDefDataLgth, self->compMsgData->totalLgth, self->compMsgData->totalLgth);
  cp = (char *)(*totalData);
cp2 = cp;
  cp[0] = ((*totalLgth) >> 8) & 0xFF;
  cp[1] = (*totalLgth) & 0xFF;
  cp[2] = ((self->buildListMsgInfos.defHeaderLgth + self->buildListMsgInfos.encryptedDefDataLgth) >> 8) & 0xFF;
  cp[3] = (self->buildListMsgInfos.defHeaderLgth + self->buildListMsgInfos.encryptedDefDataLgth) & 0xFF;
  cp[4] = ((self->buildListMsgInfos.msgHeaderLgth + self->buildListMsgInfos.encryptedMsgDataLgth) >> 8) & 0xFF;
  cp[5] = (self->buildListMsgInfos.msgHeaderLgth + self->buildListMsgInfos.encryptedMsgDataLgth) & 0xFF;
ets_printf("cp: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n", cp[0], cp[1], cp[2], cp[3], cp[4], cp[5]);
#endif
ets_printf("ready to send Msg cp: %p %p %d\n", cp, cp2, cp-cp2);
  result = self->compMsgData->initListMsg(self->compMsgData);
  checkErrOK(result);
  // and now set the values
  result = self->compMsgData->setListFieldValue(self->compMsgData, COMP_MSG_SPEC_FIELD_NUM_LIST_MSGS, compMsgData->numListMsgs, NULL, 0);
  checkErrOK(result);
  result = self->compMsgData->setListFieldValue(self->compMsgData, COMP_MSG_SPEC_FIELD_LIST_MSG_SIZES, compMsgData->listMsgSizes[0], NULL, 0);
  checkErrOK(result);
  result = self->compMsgData->setListFieldValue(self->compMsgData, COMP_MSG_SPEC_FIELD_LIST_MSG_SIZES, compMsgData->listMsgSizes[1], NULL, 1);
  checkErrOK(result);
  os_free(compMsgData->listMsgSizes);
  result = self->compMsgData->setListFieldValue(self->compMsgData, COMP_MSG_SPEC_FIELD_LIST_MSGS, 0, compMsgData->listMsgs, 0);
  checkErrOK(result);
  result = self->compMsgData->prepareListMsg(self->compMsgData);
  checkErrOK(result);
self->compMsgData->dumpListFields(self->compMsgData);
  result = self->compMsgData->getListData(self->compMsgData, totalData, totalLgth);
  checkErrOK(result);
  return result;
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
  
  result = self->fixOffsetsForKeyValues(self);
  checkErrOK(result);
  result = self->compMsgData->initMsg(self->compMsgData);
//ets_printf("heap2: %d\n", system_get_free_heap_size());
  result = setMsgValues(self);
ets_printf("buildMsg setMsgValues has been called, we finish here temporarely! result: %d\n", result);
  checkErrOK(result);
return result;
  result = self->compMsgData->getFieldValue(self->compMsgData, "@dst", &dst, &stringValue);
  checkErrOK(result);
  self->buildListMsgInfos.dst = dst;
  result = self->compMsgData->getFieldValue(self->compMsgData, "@src", &src, &stringValue);
  checkErrOK(result);
  self->buildListMsgInfos.src = src;
  result = self->compMsgData->getMsgData(self->compMsgData, &msgData, &msgLgth);
ets_printf("getMsgData result: %d msgLgth: %d msgData: %s!\n", result, msgLgth, msgData);
  checkErrOK(result);

  // FIXME !! have to check if extraLgth is included!!
  self->buildListMsgInfos.msgHeaderLgth = self->compMsgData->headerLgth;
  self->buildListMsgInfos.msgDataLgth = msgLgth;
  self->buildListMsgInfos.msgData = msgData;
  self->buildListMsgInfos.encryptedMsgDataLgth = 0;
  self->buildListMsgInfos.encryptedMsgData = NULL;

  self->buildListMsgInfos.defHeaderLgth = 0;
  self->buildListMsgInfos.defDataLgth = 0;
  self->buildListMsgInfos.defData = NULL;
  self->buildListMsgInfos.encryptedDefDataLgth = 0;
  self->buildListMsgInfos.encryptedDefData = NULL;

  cryptKey = "a1b2c3d4e5f6g7h8";
  ivlen = 16;
  klen = 16;

  if (self->buildMsgInfos.partsFlags & COMP_DISP_IS_ENCRYPTED) {
      uint8_t *toCryptPtr;
      uint8_t *encryptedMsgData;
      size_t encryptedMsgDataLgth;
      uint16_t mlen;
      uint8_t headerLgth;

ets_printf("need to encrypt message!%s!\n", msgData);
    headerLgth = self->compMsgData->headerLgth;
    mlen = self->compMsgData->totalLgth - headerLgth;
ets_printf("msglen!%d!mlen: %d, headerLgth!%d\n", self->compMsgData->totalLgth, mlen, self->compMsgData->headerLgth);
    toCryptPtr = msgData + self->compMsgData->headerLgth;
    result = self->encryptMsg(toCryptPtr, mlen, cryptKey, klen, cryptKey, ivlen, &encryptedMsgData, &encryptedMsgDataLgth);
    checkErrOK(result);
    c_memcpy(toCryptPtr, encryptedMsgData, encryptedMsgDataLgth);
ets_printf("crypted: len: %d!%s!\n", encryptedMsgDataLgth, msgData);
    
    self->buildListMsgInfos.encryptedMsgDataLgth = encryptedMsgDataLgth;
    self->buildListMsgInfos.encryptedMsgData = encryptedMsgData;
  }

  if (self->buildMsgInfos.numRows > 0) {
    // we have a definition and the message
    result = self->compMsgData->prepareDefMsg(self->compMsgData);
    checkErrOK(result);
    result = self->compMsgData->getDefData(self->compMsgData, &defData, &defLgth);
ets_printf("getDef result: %d defLgth: %d defData: %s!\n", result, defLgth, defData);
    checkErrOK(result);
    self->buildListMsgInfos.defHeaderLgth = self->compMsgData->defHeaderLgth;
    self->buildListMsgInfos.defDataLgth = defLgth;
    self->buildListMsgInfos.defData = defData;

    if (self->buildMsgInfos.partsFlags & COMP_DISP_IS_ENCRYPTED) {
ets_printf("need to encrypt message!%s!numRows!%d!\n", msgData, self->buildMsgInfos.numRows);
      uint8_t *defToCryptPtr;
      size_t deflen;
      uint8_t *encryptedDefData;
      size_t encryptedDefDataLgth;
      size_t totalLgth;
      uint8_t *totalData;

      self->buildListMsgInfos.defData = defData;
      deflen = defLgth - self->compMsgData->headerLgth;
ets_printf("defLgth!%d!deflen: %d, headerLgth!%d\n", defLgth, deflen, self->compMsgData->headerLgth);
      defToCryptPtr = defData + self->compMsgData->headerLgth;
      result = self->encryptMsg(defToCryptPtr, deflen, cryptKey, klen, cryptKey, ivlen, &encryptedDefData, &encryptedDefDataLgth);
      checkErrOK(result);
      c_memcpy(defToCryptPtr, encryptedDefData, encryptedDefDataLgth);
ets_printf("defEncrypted: len: %d!%s!\n", encryptedDefDataLgth, defData);
        
      self->buildListMsgInfos.encryptedDefDataLgth = encryptedDefDataLgth;
      self->buildListMsgInfos.encryptedDefData = encryptedDefData;

      result = self->buildListMsg(self, &totalLgth, &totalData);
      checkErrOK(result);
      // FIXME!! need compMsgModulData callback function for sending here !!!
      result = self->websocketSendData(self->wud, totalData, totalLgth, OPCODE_BINARY);
ets_printf("Msg sent\n");
//  os_free(totalData);
    } else {
      // nested message not encrypted
      // FIXME !! need code here
    }
  } else {
    // normal message
    if (self->buildMsgInfos.partsFlags & COMP_DISP_IS_ENCRYPTED) {
      // FIXME !! need code here
    } else {
      // FIXME !! need code here
    }
  }
//  result = self->resetMsgInfo(self, self->buildMsgInfos.parts);
  return result;
}

// ================================= prepareAnswerMsg ====================================

static uint8_t prepareAnswerMsg(compMsgDispatcher_t *self, msgParts_t *parts, uint8_t type) {
  uint8_t fileName[30];
  int result;
  uint8_t numEntries;
  unsigned long ulgth;
  char *endPtr;
  uint8_t *cp;
  uint8_t lgth;
  uint8_t buf[100];
  uint8_t *buffer = buf;
  uint8_t numRows;
  uint8_t *handle;
  compMsgData_t *compMsgData;
  uint8_t *prepareValuesCbName;
  uint8_t actionMode;
  int idx;
  uint8_t *data;
  int msgLgth;

//ets_printf("§@prepareAnswerMsg u16!%c%c!t!%c!@§\n", (parts->u16CmdKey>>8)& 0xFF, parts->u16CmdKey&0xFF, type);
  os_sprintf(fileName, "Desc%c%c%c.txt", (parts->u16CmdKey>>8)& 0xFF, parts->u16CmdKey&0xFF, type);
ets_printf("fileName: %s\n", fileName);
  result = self->compMsgMsgDesc->openFile(self->compMsgMsgDesc, fileName, "r");
  checkErrOK(result);
  result = self->compMsgMsgDesc->readLine(self->compMsgMsgDesc, &buffer, &lgth);
  checkErrOK(result);
  if ((lgth < 4) || (buffer[0] != '#')) {
    return COMP_DISP_ERR_BAD_FILE_CONTENTS;
  }
  ulgth = c_strtoul(buffer+2, &endPtr, 10);
  numEntries = (uint8_t)ulgth;
  cp = (uint8_t *)endPtr;
  prepareValuesCbName = NULL;
  if ((*cp == ',') || (*cp == '\n') || (*cp == '\r') || (*cp == '\0')) {
    *cp = '\0';
    cp++;
    prepareValuesCbName = cp;
  }
  if (prepareValuesCbName != NULL) {
    while ((*cp != '\n') &&  (*cp != '\r') && (*cp != '\0')) {
      cp++;
    }
    *cp = '\0';
  }
  self->buildMsgInfos.numEntries = numEntries;
  self->buildMsgInfos.type = type;
  self->buildMsgInfos.parts = parts;
  self->buildMsgInfos.u16CmdKey = parts->u16CmdKey;
  self->buildMsgInfos.partsFlags = parts->partsFlags;
  self->buildMsgInfos.numRows = numRows;
  if (prepareValuesCbName != NULL) {
    result = self->getActionMode(self, prepareValuesCbName+1, &actionMode);
    self->actionMode = actionMode;
    checkErrOK(result);
    result  = self->runAction(self, &type);
    return result;
  } else {
    result = self->buildMsg(self);
    result = setMsgValues(self);
    checkErrOK(result);
//ets_printf("§heap3: %d§", system_get_free_heap_size());
    result = self->compMsgData->getMsgData(self->compMsgData, &data, &msgLgth);
    checkErrOK(result);
    result = self->typeRSendAnswer(self, data, msgLgth);
    return result;
  }
}

// ================================= prepareNotEncryptedAnswer ====================================

static uint8_t prepareNotEncryptedAnswer(compMsgDispatcher_t *self, msgParts_t *parts, uint8_t type) {
  int result;
  uint8_t fileName[30];
  uint8_t numEntries;
  char *endPtr;
  unsigned long ulgth;
  uint8_t lgth;
  uint8_t buf[100];
  uint8_t *buffer = buf;
  uint8_t *data;
  int msgLgth;

  result = prepareAnswerMsg(self, parts, type);
  checkErrOK(result);
  self->buildMsgInfos.type = type;
  self->buildMsgInfos.parts = parts;
  result = setMsgValues(self);
  checkErrOK(result);
//ets_printf("§heap3: %d§", system_get_free_heap_size());
  result = self->compMsgData->getMsgData(self->compMsgData, &data, &msgLgth);
  checkErrOK(result);
  result = self->typeRSendAnswer(self, data, msgLgth);
  return result;
}

// ================================= prepareEncryptedAnswer ====================================

static uint8_t prepareEncryptedAnswer(compMsgDispatcher_t *self, msgParts_t *parts, uint8_t type) {
  uint8_t fileName[30];
  int result;
  uint8_t numEntries;
  char *endPtr;
  uint8_t lgth;
  int defLgth;
  uint8_t *defData;
  int msgLgth;
  uint8_t *data;
  uint8_t buf[100];
  uint8_t *buffer = buf;
  uint8_t numRows;
  uint8_t *handle;
  unsigned long ulgth;
  compMsgData_t *compMsgData;
  int idx;

  result = prepareAnswerMsg(self, parts, type);
  checkErrOK(result);
//FIXME  TEMPORARY!!!
return COMP_MSG_ERR_OK;
  os_sprintf(fileName, "Val%c%c%c.txt", (parts->u16CmdKey>>8)&0xFF, parts->u16CmdKey&0xFF, type);
  result = self->compMsgMsgDesc->openFile(self->compMsgMsgDesc, fileName, "r");
  checkErrOK(result);
  result = self->compMsgMsgDesc->readLine(self->compMsgMsgDesc, &buffer, &lgth);
  checkErrOK(result);
  if ((lgth < 4) || (buffer[0] != '#')) {
    return COMP_DISP_ERR_BAD_FILE_CONTENTS;
  }
  ulgth = c_strtoul(buffer+2, &endPtr, 10);
  numEntries = (uint8_t)ulgth;
//ets_printf("§@NE2!%d!@§", numEntries);
// FIXME!!!
//  result = self->setMsgValuesFromLines(self, compMsgData, handle, type);
  checkErrOK(result);
  result = self->compMsgMsgDesc->closeFile(self->compMsgMsgDesc);
  checkErrOK(result);
//ets_printf("§heap3: %d§", system_get_free_heap_size());
  result = compMsgData->getMsgData(compMsgData, &data, &msgLgth);
  checkErrOK(result);

ets_printf("prepareDefMsg\n");
result = self->compMsgData->prepareDefMsg(self->compMsgData);
ets_printf("prepareDefMsg result: %d\n", result);
checkErrOK(result);
result = self->compMsgData->getDefData(self->compMsgData, &defData, &defLgth);
ets_printf("defLgth: %d defData: %s!\n", defLgth, defData);
checkErrOK(result);
self->compMsgDataView->dataView->dumpBinary(defData, defLgth, "defData");

  result = self->typeRSendAnswer(self, data, msgLgth);
  self->resetMsgInfo(self, parts);
  return COMP_DISP_ERR_OK;
}

// ================================= compMsgBuildMsgInit ====================================

uint8_t compMsgBuildMsgInit(compMsgDispatcher_t *self) {
  self->fixOffsetsForKeyValues = &fixOffsetsForKeyValues;
  self->setMsgFieldValue = &setMsgFieldValue;
  self->setMsgKeyValues = &setMsgKeyValues;
  self->buildMsg = &buildMsg;
  self->buildListMsg = &buildListMsg;
  self->setMsgValues = &setMsgValues;
  self->prepareAnswerMsg = &prepareAnswerMsg;
  self->prepareEncryptedAnswer = &prepareEncryptedAnswer;
  return COMP_DISP_ERR_OK;
}
