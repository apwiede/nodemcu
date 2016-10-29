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
 * File:   compMsgDispatcher.c
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
#include "compMsgDispatcher.h"
#include "../crypto/mech.h"

#define BASE64_INVALID '\xff'
#define BASE64_PADDING '='
#define ISBASE64(c) (unbytes64[c] != BASE64_INVALID)

#define DISP_HANDLE_PREFIX "stmsgdisp_"

typedef struct handle2Dispatcher
{
  uint8_t *handle;
  compMsgDispatcher_t *compMsgDispatcher;
} handle2Dispatcher_t;

typedef struct compMsgDispatcherHandles
{
  handle2Dispatcher_t *handles;
  int numHandles;
} compMsgDispatcherHandles_t;

// create an object
static compMsgDispatcherHandles_t compMsgDispatcherHandles = { NULL, 0};

static int compMsgDispatcherId = 0;
// right now we only need one dispatcher!
static compMsgDispatcher_t *compMsgDispatcherSingleton = NULL;

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
    return COMP_DISP_ERR_OUT_OF_MEMORY;
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
  return COMP_DISP_ERR_OK;
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
    return COMP_DISP_ERR_OUT_OF_MEMORY;
  } 
  if (n & 3) {
    return COMP_DISP_ERR_INVALID_BASE64_STRING;
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
      return COMP_DISP_ERR_INVALID_BASE64_STRING;
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
  return COMP_DISP_ERR_OK;;
}

// ============================= addHandle ========================

static int addHandle(uint8_t *handle, compMsgDispatcher_t *compMsgDispatcher) {
  int idx;

  if (compMsgDispatcherHandles.handles == NULL) {
    compMsgDispatcherHandles.handles = os_zalloc(sizeof(handle2Dispatcher_t));
    if (compMsgDispatcherHandles.handles == NULL) {
      return COMP_DISP_ERR_OUT_OF_MEMORY;
    } else {
      compMsgDispatcherHandles.handles[compMsgDispatcherHandles.numHandles].handle = handle;
      compMsgDispatcherHandles.handles[compMsgDispatcherHandles.numHandles].compMsgDispatcher = compMsgDispatcher;
      compMsgDispatcherHandles.numHandles++;
      return COMP_DISP_ERR_OK;
    }
  } else {
    // check for unused slot first
    idx = 0;
    while (idx < compMsgDispatcherHandles.numHandles) {
      if (compMsgDispatcherHandles.handles[idx].handle == NULL) {
        compMsgDispatcherHandles.handles[idx].handle = handle;
        compMsgDispatcherHandles.handles[idx].compMsgDispatcher = compMsgDispatcher;
        return COMP_DISP_ERR_OK;
      }
      idx++;
    }
    compMsgDispatcherHandles.handles = os_realloc(compMsgDispatcherHandles.handles, sizeof(handle2Dispatcher_t)*(compMsgDispatcherHandles.numHandles+1));
    checkAllocOK(compMsgDispatcherHandles.handles);
    compMsgDispatcherHandles.handles[compMsgDispatcherHandles.numHandles].handle = handle;
    compMsgDispatcherHandles.handles[idx].compMsgDispatcher = compMsgDispatcher;
    compMsgDispatcherHandles.numHandles++;
  }
  return COMP_DISP_ERR_OK;
}

// ============================= deleteHandle ========================

static int deleteHandle(const uint8_t *handle) {
  int idx;
  int numUsed;
  int found;

  if (compMsgDispatcherHandles.handles == NULL) {
    return COMP_DISP_ERR_HANDLE_NOT_FOUND;
  }
  found = 0;
  idx = 0;
  numUsed = 0;
  while (idx < compMsgDispatcherHandles.numHandles) {
    if ((compMsgDispatcherHandles.handles[idx].handle != NULL) && (c_strcmp(compMsgDispatcherHandles.handles[idx].handle, handle) == 0)) {
      compMsgDispatcherHandles.handles[idx].handle = NULL;
      found++;
    } else {
      if (compMsgDispatcherHandles.handles[idx].handle != NULL) {
        numUsed++;
      }
    }
    idx++;
  }
  if (numUsed == 0) {
    os_free(compMsgDispatcherHandles.handles);
    compMsgDispatcherHandles.handles = NULL;
  }
  if (found) {
      return COMP_DISP_ERR_OK;
  }
  return COMP_DISP_ERR_HANDLE_NOT_FOUND;
}

// ============================= checkHandle ========================

static int checkHandle(const char *handle, compMsgDispatcher_t **compMsgDispatcher) {
  int idx;

  if (compMsgDispatcherHandles.handles == NULL) {
    return COMP_DISP_ERR_HANDLE_NOT_FOUND;
  }
  idx = 0;
  while (idx < compMsgDispatcherHandles.numHandles) {
    if ((compMsgDispatcherHandles.handles[idx].handle != NULL) && (c_strcmp(compMsgDispatcherHandles.handles[idx].handle, handle) == 0)) {
      *compMsgDispatcher = compMsgDispatcherHandles.handles[idx].compMsgDispatcher;
      return COMP_DISP_ERR_OK;
    }
    idx++;
  }
  return COMP_DISP_ERR_HANDLE_NOT_FOUND;
}

// ================================= dumpMsgParts ====================================

static uint8_t dumpMsgParts(compMsgDispatcher_t *self, msgParts_t *msgParts) {
  int idx;

  ets_printf("dumpMsgParts:\n");
  ets_printf("MsgParts1 form: 0x%04x to: 0x%04x totalLgth: %d GUID: %d srcId: %d u16_cmdKey: %d\n", msgParts->fromPart, msgParts->toPart, msgParts->totalLgth, msgParts->GUID, msgParts->srcId, msgParts->u16CmdKey);

  ets_printf("MsgParts2 lgth: %d fieldOffset: %d\n", msgParts->lgth, msgParts->fieldOffset);
  ets_printf("buf");
  idx = 0;
  while (idx < msgParts->realLgth - 1) {
    ets_printf(" %d 0x%02x", idx, msgParts->buf[idx]);
    idx++;
  }
  ets_printf("\n");
  ets_printf("partFlags: ");
  if (msgParts->partsFlags & COMP_DISP_U16_CMD_KEY) {
    ets_printf(" COMP_DISP_U16_CMD_KEY");
  }
  if (msgParts->partsFlags & COMP_DISP_U8_VECTOR_GUID) {
    ets_printf(" COMP_DISP_U8_VECTOR_GUID");
  }
  if (msgParts->partsFlags & COMP_DISP_U16_SRC_ID) {
    ets_printf(" COMP_DISP_U16_SRC_ID");
  }
  if (msgParts->partsFlags & COMP_DISP_U16_CMD_KEY) {
    ets_printf(" COMP_DISP_U16_CMD_KEY");
  }
  ets_printf("\n");
  return COMP_DISP_ERR_OK;
}

#define checkErrWithResetOK(result) if(result != DATA_VIEW_ERR_OK) { dataView->data = saveData; dataView->lgth= saveLgth; return result; }

// ================================= getMsgPtrFromMsgParts ====================================

static uint8_t getMsgPtrFromMsgParts(compMsgDispatcher_t *self, msgParts_t *msgParts, compMsgData_t **compMsgData, int incrRefCnt) {
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
  dataView = self->compMsgDataView->dataView;
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
  result = dataView->setUint16(dataView, offset, msgParts->u16CmdKey);
  checkErrWithResetOK(result);
  offset += sizeof(uint16_t);
  dataView->data = saveData;
  dataView->lgth= saveLgth;
  // end build header from msgParts
  if ((incrRefCnt == COMP_MSG_INCR) && (self->numMsgHeaders >= self->maxMsgHeaders)) {
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
      if ((headerEntry->compMsgData != NULL) && (ets_memcmp(headerEntry->header, header, offset) == 0)) {
        if (incrRefCnt < 0) {
          headerEntry->headerLgth = 0;
          os_free(headerEntry->compMsgData);
          headerEntry->compMsgData = NULL;
          *compMsgData = NULL;
          return COMP_DISP_ERR_OK;
        }
        *compMsgData = headerEntry->compMsgData;
        (*compMsgData)->flags &= ~COMP_MSG_IS_PREPARED;
        return COMP_DISP_ERR_OK;
      }
      if ((incrRefCnt == COMP_MSG_INCR) && (headerEntry->compMsgData == NULL) && (firstFreeEntry == NULL)) {
        firstFreeEntry = headerEntry;
        firstFreeEntry->compMsgData = newCompMsgData();
        firstFreeEntry->compMsgData->setDispatcher(firstFreeEntry->compMsgData, self);
        firstFreeEntry->headerLgth = offset;
        c_memcpy(firstFreeEntry->header, header, offset);
        *compMsgData = firstFreeEntry->compMsgData;
      }
      headerIdx++;
    }
  }
  if (incrRefCnt < 0) {
    return COMP_DISP_ERR_OK; // just ignore silently
  } else {
    if (incrRefCnt == 0) {
      return COMP_DISP_ERR_HEADER_NOT_FOUND;
    } else {
      if (firstFreeEntry != NULL) {
        *compMsgData = firstFreeEntry->compMsgData;
        firstFreeEntry->headerLgth = offset;
        c_memcpy(firstFreeEntry->header, header, offset);
      } else {
        newHeaderEntry = &self->msgHeader2MsgPtrs[self->numMsgHeaders];
        newHeaderEntry->headerLgth = offset;
        c_memcpy(newHeaderEntry->header, header, offset);
        newHeaderEntry->compMsgData = newCompMsgData();
        newHeaderEntry->compMsgData->setDispatcher(newHeaderEntry->compMsgData, self);
        *compMsgData = newHeaderEntry->compMsgData;
        self->numMsgHeaders++;
      }
    }
  }
  return COMP_DISP_ERR_OK;
}

#undef checkErrWithResetOK

// ================================= getFieldType ====================================

static uint8_t getFieldType(compMsgDispatcher_t *self, compMsgData_t *compMsgData, uint8_t fieldNameId, uint8_t *fieldTypeId) {
  int idx;
  compMsgField_t *fieldInfo;

  idx = 0;
  while (idx < compMsgData->numFields) {
    fieldInfo = &compMsgData->fields[idx];
    if (fieldInfo->fieldNameId == fieldNameId) {
      *fieldTypeId = fieldInfo->fieldTypeId;
      return COMP_DISP_ERR_OK;
    }
    idx++;
  }
  // and now check the table fields
  idx = 0;
  while (idx < compMsgData->numTableRowFields) {
    fieldInfo = &compMsgData->tableFields[idx];
    if (fieldInfo->fieldNameId == fieldNameId) {
      *fieldTypeId = fieldInfo->fieldTypeId;
      return COMP_DISP_ERR_OK;
    }
    idx++;
  }
  return COMP_DISP_ERR_FIELD_NOT_FOUND;
}

// ================================= getFieldInfoFromLine ====================================

static uint8_t getFieldInfoFromLine(compMsgDispatcher_t *self) {
  int idx;
  int result;
  unsigned long uval;
  uint8_t *buffer;
  uint8_t*cp;
  char *endPtr;
  uint8_t lgth;
  compMsgDataView_t *dataView;

  dataView = self->compMsgData->compMsgDataView;
  buffer = self->buildMsgInfos.buf;
  self->buildMsgInfos.numericValue = 0;
  self->buildMsgInfos.stringValue = NULL;
  result = self->compMsgMsgDesc->readLine(self->compMsgMsgDesc, &buffer, &lgth);
  checkErrOK(result);
  if (lgth == 0) {
    return COMP_DISP_ERR_TOO_FEW_FILE_LINES;
  }
  buffer[lgth] = 0;
  self->buildMsgInfos.fieldNameStr = buffer;

  // fieldName
  cp = self->buildMsgInfos.fieldNameStr;
  while (*cp != ',') {
    cp++;
  }
  *cp++ = '\0';
  result = dataView->getFieldNameIdFromStr(dataView, self->buildMsgInfos.fieldNameStr, &self->buildMsgInfos.fieldNameId, COMP_MSG_NO_INCR);
  checkErrOK(result);
  result = getFieldType(self, self->compMsgData, self->buildMsgInfos.fieldNameId, &self->buildMsgInfos.fieldTypeId);
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
  return COMP_MSG_ERR_OK;
}

// ================================= setMsgValuesFromLines ====================================

static uint8_t setMsgValuesFromLines(compMsgDispatcher_t *self, compMsgData_t *compMsgData, uint8_t numEntries, uint8_t *handle, uint8_t type) {
  int idx;
  int fieldIdx;
  int tableFieldIdx;
  uint8_t fieldLgth;
  uint8_t *flagStr;
  uint8_t flag;
  unsigned long uval;
  compMsgDataView_t *dataView;
  int result;
  compMsgField_t *fieldInfo;
  uint8_t numTableRows;
  uint8_t numTableRowFields;
  int currTableRow;
  int currTableCol;
  int msgCmdKey;

  idx = 0;
  dataView = compMsgData->compMsgDataView;
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
  compMsgData = self->compMsgData;
//ets_printf("setMsgValuesFromLines: numFields:%d \n", compMsgData->numFields);
  while ((fieldIdx < compMsgData->numFields) && (idx <= numEntries)) {
//ets_printf("setMsgValuesFromLines2: fieldIdx: %d atbeleFieldIdx: %d idx: %d numFields:%d \n", fieldIdx, tableFieldIdx, idx, compMsgData->numFields);
    if (numTableRows > 0) {
      fieldInfo = &compMsgData->tableFields[tableFieldIdx++];
    } else {
      fieldInfo = &compMsgData->fields[fieldIdx++];
    }
//ets_printf("fieldNameId: %d numtabrows: %d\n", fieldInfo->fieldNameId, numTableRows);
    switch (fieldInfo->fieldNameId) {
    case COMP_MSG_SPEC_FIELD_TABLE_ROW_FIELDS:
      numTableRows = compMsgData->numTableRows;
      numTableRowFields = compMsgData->numTableRowFields;
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
            result = compMsgData->setTableFieldValue(compMsgData, self->buildMsgInfos.fieldNameStr, currTableRow, self->buildMsgInfos.numericValue, self->buildMsgInfos.stringValue);
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
          result = compMsgData->setFieldValue(compMsgData, self->buildMsgInfos.fieldNameStr, self->buildMsgInfos.numericValue, self->buildMsgInfos.stringValue);
          currTableRow++;
        }
      } else {
        switch (self->buildMsgInfos.fieldNameId) {
          case COMP_MSG_SPEC_FIELD_DST:
            result = compMsgData->setFieldValue(compMsgData, self->buildMsgInfos.fieldNameStr, self->buildMsgInfos.numericValue, self->buildMsgInfos.stringValue);
            break;
          case COMP_MSG_SPEC_FIELD_SRC:
            result = compMsgData->setFieldValue(compMsgData, self->buildMsgInfos.fieldNameStr, self->buildMsgInfos.numericValue, self->buildMsgInfos.stringValue);
            break;
          case COMP_MSG_SPEC_FIELD_CMD_KEY:
            // check for u8CmdKey/u16CmdKey here
ets_printf("cmdKey: 0x%04x\n", self->received.u16CmdKey);
            self->buildMsgInfos.numericValue = self->received.u16CmdKey;
            self->buildMsgInfos.stringValue = NULL;
            result = compMsgData->setFieldValue(compMsgData, self->buildMsgInfos.fieldNameStr, self->buildMsgInfos.numericValue, self->buildMsgInfos.stringValue);
            break;
          default:
            result = compMsgData->setFieldValue(compMsgData, self->buildMsgInfos.fieldNameStr, self->buildMsgInfos.numericValue, self->buildMsgInfos.stringValue);
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
  msgCmdKey = (self->buildMsgInfos.u16CmdKey & 0xFF00) | type & 0xFF;
  result = compMsgData->setFieldValue(compMsgData, "@cmdKey", msgCmdKey, NULL);
  checkErrOK(result);
  compMsgData->prepareMsg(compMsgData);
//compMsgData->dumpMsg(compMsgData);
  return COMP_DISP_ERR_OK;
}

// ================================= createMsgFromLines ====================================

static uint8_t createMsgFromLines(compMsgDispatcher_t *self, msgParts_t *parts, uint8_t numEntries, uint8_t numRows, uint8_t type) {
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
  result = getMsgPtrFromMsgParts(self, parts, &self->compMsgData, COMP_MSG_INCR);
  checkErrOK(result);
  if (self->compMsgData->flags & COMP_MSG_IS_INITTED) {
    return COMP_DISP_ERR_OK;
  }
  result = self->compMsgData->createMsg(self->compMsgData, numEntries, &self->msgHandle);
  checkErrOK(result);
  idx = 0;
  while(idx < numEntries) {
    result = self->compMsgMsgDesc->readLine(self->compMsgMsgDesc, &buffer, &lgth);
    checkErrOK(result);
    if (lgth == 0) {
      return COMP_DISP_ERR_TOO_FEW_FILE_LINES;
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
    result = self->compMsgData->addField(self->compMsgData, fieldNameStr, fieldTypeStr, fieldLgth);
    checkErrOK(result);
    idx++;
  }
  if (self->compMsgData->numTableRows > 0) {
    result = newCompMsgDefMsg(self->compMsgData);
    checkErrOK(result);
    result = self->compMsgData->initDefMsg(self->compMsgData);
    checkErrOK(result);
self->compMsgData->dumpDefFields(self->compMsgData);
    result = newCompMsgList(self->compMsgData);
    checkErrOK(result);
// the next 2 lines are too early here the necessary data are not yet provided!!
//    result = self->compMsgData->initListMsg(self->compMsgData);
//    checkErrOK(result);
  }
  return COMP_DISP_ERR_OK;
}

// ================================= resetMsgInfo ====================================

static uint8_t resetMsgInfo(compMsgDispatcher_t *self, msgParts_t *parts) {
  parts->lgth = 0;
  parts->fieldOffset = 0;
  parts->fromPart = 0;
  parts->toPart = 0;
  parts->totalLgth = 0;
  c_memcpy(parts->GUID, "                ", 16);
  parts->srcId = 0;
  parts->u16CmdKey = 0;
  self->compMsgDataView->dataView->data = parts->buf;
  self->compMsgDataView->dataView->lgth = 0;
  return COMP_DISP_ERR_OK;
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
    return COMP_MSG_ERR_CRYPTO_BAD_MECHANISM;
  }
  bs = mech->block_size;
  clen = ((mlen + bs - 1) / bs) * bs;
  *lgth = clen;
//ets_printf("dlen: %d lgth: %d clen: %d data: %p\n", dlen, *lgth, clen, data);
  crypted = (uint8_t *)os_zalloc (clen);
  if (!crypted) {
    return COMP_MSG_ERR_CRYPTO_INIT_FAILED;
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
    return COMP_MSG_ERR_CRYPTO_INIT_FAILED;
  } 
  return COMP_MSG_ERR_OK;
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
    return COMP_MSG_ERR_CRYPTO_BAD_MECHANISM;
  }
  bs = mech->block_size;
  clen = ((mlen + bs - 1) / bs) * bs;
  *lgth = clen;
  crypted = (uint8_t *)os_zalloc (*lgth);
  if (!crypted) {
    return COMP_MSG_ERR_CRYPTO_INIT_FAILED;
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
    return COMP_MSG_ERR_CRYPTO_INIT_FAILED;
  }
  return COMP_MSG_ERR_OK;
}

// ============================= compMsgDispatcherGetPtrFromHandle ========================

uint8_t compMsgDispatcherGetPtrFromHandle(const char *handle, compMsgDispatcher_t **compMsgDispatcher) {

  if (checkHandle(handle, compMsgDispatcher) != COMP_DISP_ERR_OK) {
    return COMP_DISP_ERR_HANDLE_NOT_FOUND;
  }
  return COMP_DISP_ERR_OK;
}

// ================================= initDispatcher ====================================

static uint8_t initDispatcher(compMsgDispatcher_t *self) {
  uint8_t result;

  result = compMsgIdentifyInit(self);
  checkErrOK(result);
  result = compMsgBuildMsgInit(self);
  checkErrOK(result);
  result = compMsgSendReceiveInit(self);
  checkErrOK(result);
  result = compMsgActionInit(self);
  checkErrOK(result);
  result = compMsgModuleDataValuesInit(self);
  checkErrOK(result);
  result = compMsgWebsocketInit(self);
  checkErrOK(result);
  return COMP_DISP_ERR_OK;
}

// ================================= createDispatcher ====================================

static uint8_t createDispatcher(compMsgDispatcher_t *self, uint8_t **handle) {
  uint8_t result;

  os_sprintf(self->handle, "%s%p", DISP_HANDLE_PREFIX, self);
  result = addHandle(self->handle, self);
  if (result != COMP_DISP_ERR_OK) {
    deleteHandle(self->handle);
    os_free(self);
    return result;
  }
  resetMsgInfo(self, &self->received);
  resetMsgInfo(self, &self->toSend);
  *handle = self->handle;
  return COMP_DISP_ERR_OK;
}

// ================================= newCompMsgDispatcher ====================================

compMsgDispatcher_t *newCompMsgDispatcher() {
  if (compMsgDispatcherSingleton != NULL) {
    return compMsgDispatcherSingleton;
  }
  compMsgDispatcher_t *compMsgDispatcher = os_zalloc(sizeof(compMsgDispatcher_t));
  if (compMsgDispatcher == NULL) {
    return NULL;
  }
  compMsgDispatcher->compMsgDataView = newCompMsgDataView();
  if (compMsgDispatcher->compMsgDataView == NULL) {
    return NULL;
  }

  compMsgDispatcherId++;
  compMsgDispatcher->id = compMsgDispatcherId;

  compMsgDispatcher->numMsgHeaders = 0;
  compMsgDispatcher->maxMsgHeaders = 0;
  compMsgDispatcher->msgHeader2MsgPtrs = NULL;

//  compMsgDispatcher->msgHeaderInfos.headerFlags = 0;
  compMsgDispatcher->msgHeaderInfos.headerParts = NULL;
  compMsgDispatcher->msgHeaderInfos.numHeaderParts = 0;
  compMsgDispatcher->msgHeaderInfos.maxHeaderParts = 0;

  compMsgDispatcher->compMsgMsgDesc = newCompMsgMsgDesc();

  compMsgDispatcher->createDispatcher = &createDispatcher;
  compMsgDispatcher->initDispatcher = &initDispatcher;

  compMsgDispatcher->resetMsgInfo = &resetMsgInfo;
  compMsgDispatcher->createMsgFromLines = &createMsgFromLines;
  compMsgDispatcher->setMsgValuesFromLines = &setMsgValuesFromLines;

  compMsgDispatcher->encryptMsg = &encryptMsg;
  compMsgDispatcher->decryptMsg = &decryptMsg;
  compMsgDispatcher->toBase64 = &toBase64;
  compMsgDispatcher->fromBase64 = &fromBase64;

  compMsgDispatcher->dumpMsgParts = &dumpMsgParts;

  compMsgDispatcherSingleton = compMsgDispatcher;
  return compMsgDispatcher;
}

// ================================= freeCompMsgDispatcher ====================================

void freeCompMsgDispatcher(compMsgDispatcher_t *compMsgDispatcher) {
}

