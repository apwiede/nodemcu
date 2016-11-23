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

#include "osapi.h"
#include "mem.h"
#include "c_string.h"
#include "c_stdlib.h"
#include "compMsgDispatcher.h"

#define HANDLE_PREFIX "stmsg_"

typedef struct handle2Header 
{
  uint8_t *handle;
  uint8_t *header;
  compMsgData_t *compMsgData;
} handle2Header_t;

typedef struct compMsgHandles
{
  handle2Header_t *handles;
  int numHandles;
} compMsgHandles_t;

// create an object
static compMsgHandles_t compMsgHandles = { NULL, 0};


// ============================= addHandle ========================

static int addHandle(uint8_t *handle, compMsgData_t *compMsgData, uint8_t **headerPtr) {
  int idx;

  if (compMsgHandles.handles == NULL) {
    compMsgHandles.handles = os_zalloc(sizeof(handle2Header_t));
    if (compMsgHandles.handles == NULL) {
      return COMP_MSG_ERR_OUT_OF_MEMORY;
    } else {
      compMsgHandles.handles[compMsgHandles.numHandles].handle = handle;
      compMsgHandles.handles[compMsgHandles.numHandles].compMsgData = compMsgData;
      *headerPtr = compMsgHandles.handles[compMsgHandles.numHandles++].header;
      return COMP_MSG_ERR_OK;
    }
  } else {
    // check for unused slot first
    idx = 0;
    while (idx < compMsgHandles.numHandles) {
      if (compMsgHandles.handles[idx].handle == NULL) {
        compMsgHandles.handles[idx].handle = handle;
        compMsgHandles.handles[idx].compMsgData = compMsgData;
        *headerPtr = compMsgHandles.handles[idx].header;
        return COMP_MSG_ERR_OK;
      }
      idx++;
    }
    compMsgHandles.handles = os_realloc(compMsgHandles.handles, sizeof(handle2Header_t)*(compMsgHandles.numHandles+1));
    checkAllocOK(compMsgHandles.handles);
    compMsgHandles.handles[compMsgHandles.numHandles].handle = handle;
    compMsgHandles.handles[idx].compMsgData = compMsgData;
    *headerPtr = compMsgHandles.handles[compMsgHandles.numHandles++].header;
  }
  return COMP_MSG_ERR_OK;
}

// ============================= deleteHandle ========================

static int deleteHandle(const uint8_t *handle) {
  int idx;
  int numUsed;
  int found;

  if (compMsgHandles.handles == NULL) {
ets_printf("deleteHandle 1 HANDLE_NOT_FOUND\n");
    return COMP_MSG_ERR_HANDLE_NOT_FOUND;
  }
  found = 0;
  idx = 0;
  numUsed = 0;
  while (idx < compMsgHandles.numHandles) {
    if ((compMsgHandles.handles[idx].handle != NULL) && (c_strcmp(compMsgHandles.handles[idx].handle, handle) == 0)) {
      compMsgHandles.handles[idx].handle = NULL;
      found++;
    } else {
      if (compMsgHandles.handles[idx].handle != NULL) {
        numUsed++;
      }
    }
    idx++;
  }
  if (numUsed == 0) {
    os_free(compMsgHandles.handles);
    compMsgHandles.handles = NULL;
  }
  if (found) {
      return COMP_MSG_ERR_OK;
  }
ets_printf("deleteHandle 2 HANDLE_NOT_FOUND\n");
  return COMP_MSG_ERR_HANDLE_NOT_FOUND;
}

// ============================= checkHandle ========================

static int checkHandle(const char *handle, compMsgData_t **compMsgData) {
  int idx;

  if (compMsgHandles.handles == NULL) {
ets_printf("checkHandle 1 HANDLE_NOT_FOUND\n");
    return COMP_MSG_ERR_HANDLE_NOT_FOUND;
  }
  idx = 0;
  while (idx < compMsgHandles.numHandles) {
    if ((compMsgHandles.handles[idx].handle != NULL) && (c_strcmp(compMsgHandles.handles[idx].handle, handle) == 0)) {
      *compMsgData = compMsgHandles.handles[idx].compMsgData;
      return COMP_MSG_ERR_OK;
    }
    idx++;
  }
ets_printf("checkHandle 2 HANDLE_NOT_FOUND\n");
  return COMP_MSG_ERR_HANDLE_NOT_FOUND;
}

// ============================= compMsgGetPtrFromHandle ========================

uint8_t compMsgGetPtrFromHandle(const char *handle, compMsgData_t **compMsgData) {
  if (checkHandle(handle, compMsgData) != COMP_MSG_ERR_OK) {
ets_printf("compMsgGetPtrFromHandle 1 HANDLE_NOT_FOUND\n");
    return COMP_MSG_ERR_HANDLE_NOT_FOUND;
  }
  return COMP_MSG_ERR_OK;
}

// ================================= createMsg ====================================

static uint8_t createMsg(compMsgData_t *self, int numFields, uint8_t **handle) {
  uint8_t result;
  self->fields = os_zalloc(sizeof(compMsgField_t) * numFields);
  if (self->fields == NULL) {
    return COMP_MSG_ERR_OUT_OF_MEMORY;
  }
  self->tableFields = NULL;
  self->flags = 0;
  self->numFields = 0;
  self->maxFields = numFields;
  self->numTableRows = 0;
  self->numTableRowFields = 0;
  self->numRowFields = 0;
  self->fieldOffset = 0;
  self->totalLgth = 0;
  self->cmdLgth = 0;
  self->headerLgth = 0;
  self->header = NULL;
  os_sprintf(self->handle, "%s%p", HANDLE_PREFIX, self);
  result = addHandle(self->handle, self, &self->header);
  if (result != COMP_MSG_ERR_OK) {
    os_free(self->fields);
    deleteHandle(self->handle);
    os_free(self);
    return result;
  }
  *handle = self->handle;
  return COMP_MSG_ERR_OK;
}

// ================================= addField ====================================

static uint8_t addField(compMsgData_t *self, const uint8_t *fieldName, const uint8_t *fieldType, uint8_t fieldLgth) {
  uint8_t numTableFields;
  uint8_t numTableRowFields;
  uint8_t numTableRows;
  uint8_t fieldTypeId = 0;
  uint8_t fieldNameId = 213;
  int row;
  int cellIdx;
  int result = COMP_MSG_ERR_OK;
  compMsgField_t *fieldInfo;

//ets_printf("addfield: %s fieldType: %s fieldLgth: %d\n", fieldName, fieldType, fieldLgth);
  if (self->numFields >= self->maxFields) {
    return COMP_MSG_ERR_TOO_MANY_FIELDS;
  }
  result = self->compMsgDataView->dataView->getFieldTypeIdFromStr(self->compMsgDataView->dataView, fieldType, &fieldTypeId);
  checkErrOK(result);
  result = self->compMsgDataView->getFieldNameIdFromStr(self->compMsgDataView, fieldName, &fieldNameId, COMP_MSG_INCR);
  checkErrOK(result);
  fieldInfo = &self->fields[self->numFields];
  // need to check for duplicate here !!
  if (c_strcmp(fieldName, "@filler") == 0) {
    self->flags |= COMP_MSG_HAS_FILLER;
    fieldLgth = 0;
    fieldInfo->fieldNameId = fieldNameId;
    fieldInfo->fieldTypeId = fieldTypeId;
    fieldInfo->fieldLgth = fieldLgth;
    self->numFields++;
    return COMP_MSG_ERR_OK;
  }
  if (c_strcmp(fieldName, "@tablerows") == 0) {
    self->numTableRows = fieldLgth;
    fieldLgth = 0;
    self->flags |= COMP_MSG_HAS_TABLE_ROWS;
    fieldInfo->fieldNameId = fieldNameId;
    fieldInfo->fieldTypeId = fieldTypeId;
    fieldInfo->fieldLgth = fieldLgth;
    self->numFields++;
    return COMP_MSG_ERR_OK;
  }
  if (c_strcmp(fieldName, "@tablerowfields") == 0) {
    self->numTableRowFields = fieldLgth;
    fieldLgth = 0;
    fieldInfo->fieldNameId = fieldNameId;
    fieldInfo->fieldTypeId = fieldTypeId;
    fieldInfo->fieldLgth = fieldLgth;
    numTableFields = self->numTableRows * self->numTableRowFields;
    if ((self->tableFields == NULL) && (numTableFields != 0)) {
      self->tableFields = os_zalloc(sizeof(compMsgField_t) * numTableFields);
      checkAllocOK(self->tableFields);
    }
    self->numFields++;
    return COMP_MSG_ERR_OK;
  }
  numTableRowFields = self->numTableRowFields;
  numTableRows = self->numTableRows;
  numTableFields = numTableRows * numTableRowFields;
  if (!((numTableFields > 0) && (self->numRowFields < numTableRowFields)) || (numTableRowFields == 0)) {
    numTableFields = 0;
    numTableRows = 1;
    numTableRowFields = 0;

    if (c_strcmp(fieldName, "@crc") == 0) {
      self->flags |= COMP_MSG_HAS_CRC;
      if (c_strcmp(fieldType, "uint8_t") == 0) {
        self->flags |= COMP_MSG_UINT8_CRC;
      }
//ets_printf("flags: 0x%02x HAS_CRC: 0x%02x HAS_FILLER: 0x%02x UINT8_CRC: 0x%02x\n", compMsg->flags, COMP_MSG_HAS_CRC, COMP_MSG_HAS_FILLER, COMP_MSG_UINT8_CRC);
    }
    fieldInfo->fieldNameId = fieldNameId;
    fieldInfo->fieldTypeId = fieldTypeId;
    fieldInfo->fieldLgth = fieldLgth;
    self->numFields++;  
  } else {
    row = 0;
    while (row < numTableRows) {
      cellIdx = self->numRowFields + row * numTableRowFields;;
      if (self->tableFields == NULL) {
        return COMP_MSG_ERR_BAD_TABLE_ROW;
      }
      fieldInfo = &self->tableFields[cellIdx];
//ets_printf("table field1: %s totalLgth: %d cmdLgth: %d\n", fieldInfo->fieldStr, compMsg->hdr.hdrInfo.hdrKeys.totalLgth, compMsg->hdr.hdrInfo.hdrKeys.cmdLgth);
      fieldInfo->fieldNameId = fieldNameId;
      fieldInfo->fieldTypeId = fieldTypeId;
      fieldInfo->fieldLgth = fieldLgth;
      row++;
    }
    self->numRowFields++;  
  } 
  return COMP_MSG_ERR_OK;
}

// ================================= getFieldValue ====================================

static uint8_t getFieldValue(compMsgData_t *self, const uint8_t *fieldName, int *numericValue, uint8_t **stringValue) {
  compMsgField_t *fieldInfo;
  uint8_t fieldNameId;
  int idx;
  int numEntries;
  int result;

  if ((self->flags & COMP_MSG_IS_INITTED) == 0) {
    return COMP_MSG_ERR_NOT_YET_INITTED;
  }
  result = self->compMsgDataView->getFieldNameIdFromStr(self->compMsgDataView, fieldName, &fieldNameId, COMP_MSG_NO_INCR);
  checkErrOK(result);
  idx = 0;
  numEntries = self->numFields;
  while (idx < numEntries) {
    fieldInfo = &self->fields[idx];
    if (fieldNameId == fieldInfo->fieldNameId) {
      result = self->compMsgDataView->getFieldValue(self->compMsgDataView, fieldInfo, numericValue, stringValue, 0);
      checkErrOK(result);
      break;
    }
    idx++;
  }
  return DATA_VIEW_ERR_OK;
}

// ================================= setFieldValue ====================================

static uint8_t setFieldValue(compMsgData_t *self, const uint8_t *fieldName, int numericValue, const uint8_t *stringValue) {
  compMsgField_t *fieldInfo;
  uint8_t fieldNameId;
  int idx;
  int numEntries;
  int result;

  if ((self->flags & COMP_MSG_IS_INITTED) == 0) {
    return COMP_MSG_ERR_NOT_YET_INITTED;
  }
  result = self->compMsgDataView->getFieldNameIdFromStr(self->compMsgDataView, fieldName, &fieldNameId, COMP_MSG_NO_INCR);
  checkErrOK(result);
  switch (fieldNameId) {
    case COMP_MSG_SPEC_FIELD_TOTAL_LGTH:
    case COMP_MSG_SPEC_FIELD_CMD_LGTH:
    case COMP_MSG_SPEC_FIELD_FILLER:
    case COMP_MSG_SPEC_FIELD_CRC:
    case COMP_MSG_SPEC_FIELD_RANDOM_NUM:
    case COMP_MSG_SPEC_FIELD_SEQUENCE_NUM:
      return COMP_MSG_ERR_FIELD_CANNOT_BE_SET;
  }
  idx = 0;
  numEntries = self->numFields;
//ets_printf("numEntries: %d\n", numEntries);
  while (idx < numEntries) {
    fieldInfo = &self->fields[idx];
    if (fieldNameId == fieldInfo->fieldNameId) {
//ets_printf("§compMsgData setFieldValue: name: %s!numeric: %d!string: %s!id: %d!§", fieldName, numericValue, stringValue == NULL ? "nil" : (char *)stringValue, fieldNameId);
      result = self->compMsgDataView->setFieldValue(self->compMsgDataView, fieldInfo, numericValue, stringValue, 0);
      checkErrOK(result);
      fieldInfo->fieldFlags |= COMP_MSG_FIELD_IS_SET;
      break;
    }
    idx++;
  }
//ets_printf("idx: %d\n", idx);
  return DATA_VIEW_ERR_OK;
}


// ================================= getTableFieldValue ====================================

static uint8_t getTableFieldValue(compMsgData_t *self, const uint8_t *fieldName, int row, int *numericValue, uint8_t **stringValue) {
  compMsgField_t *fieldInfo;
  uint8_t fieldNameId;
  int idx;
  int cellIdx;
  int numEntries;
  int result;

  if ((self->flags & COMP_MSG_IS_INITTED) == 0) {
    return COMP_MSG_ERR_NOT_YET_INITTED;
  }
  result = self->compMsgDataView->getFieldNameIdFromStr(self->compMsgDataView, fieldName, &fieldNameId, COMP_MSG_NO_INCR);
  checkErrOK(result);
  if (fieldName[0] == '@') {
    return COMP_MSG_ERR_NO_SUCH_FIELD;
  }
  if (row >= self->numTableRows) {
    return COMP_MSG_ERR_BAD_TABLE_ROW;
  }
  idx = 0;
  cellIdx = 0 + row * self->numRowFields;
  while (idx < self->numRowFields) {
    fieldInfo = &self->tableFields[cellIdx];
    if (fieldNameId == fieldInfo->fieldNameId) {
      return self->compMsgDataView->getFieldValue(self->compMsgDataView, fieldInfo, numericValue, stringValue, 0);
    }
    cellIdx++;
    idx++;
  }
  return COMP_MSG_ERR_FIELD_NOT_FOUND;
}

// ================================= setTableFieldValue ====================================

static uint8_t setTableFieldValue(compMsgData_t *self, const uint8_t *fieldName, int row, int numericValue, const uint8_t *stringValue) {
  compMsgField_t *fieldInfo;
  uint8_t fieldNameId;
  int idx;
  int cellIdx;
  int numEntries;
  int result;

//ets_printf("setTableFieldValue: %s: row: %d value: %d %s\n", fieldName, row, numericValue, stringValue == NULL ? "nil" : (char* )stringValue);
  if ((self->flags & COMP_MSG_IS_INITTED) == 0) {
    return COMP_MSG_ERR_NOT_YET_INITTED;
  }
  result = self->compMsgDataView->getFieldNameIdFromStr(self->compMsgDataView, fieldName, &fieldNameId, COMP_MSG_NO_INCR);
  checkErrOK(result);
  if (fieldName[0] == '@') {
    return COMP_MSG_ERR_FIELD_CANNOT_BE_SET;
  }
  if (row >= self->numTableRows) {
    return COMP_MSG_ERR_BAD_TABLE_ROW;
  }
  idx = 0;
  cellIdx = 0 + row * self->numRowFields;
  while (idx < self->numRowFields) {
    fieldInfo = &self->tableFields[cellIdx];
    if (fieldNameId == fieldInfo->fieldNameId) {
      fieldInfo->fieldFlags |= COMP_MSG_FIELD_IS_SET;
      result = self->compMsgDataView->setFieldValue(self->compMsgDataView, fieldInfo, numericValue, stringValue, 0);
      return result;
    }
    cellIdx++;
    idx++;
  }
  return COMP_MSG_ERR_FIELD_NOT_FOUND;
}

// ================================= prepareMsg ====================================

static uint8_t prepareMsg(compMsgData_t *self) {
  int numEntries;
  int idx;
  int result;
  uint8_t headerLgth;
  uint8_t lgth;
  compMsgField_t *fieldInfo;

  if ((self->flags & COMP_MSG_IS_INITTED) == 0) {
    return COMP_MSG_ERR_NOT_YET_INITTED;
  }
  // create the values which are different for each message!!
  numEntries = self->numFields;
  idx = 0;
  while (idx < numEntries) {
    fieldInfo = &self->fields[idx];
    switch (fieldInfo->fieldNameId) {
      case COMP_MSG_SPEC_FIELD_RANDOM_NUM:
        result = self->compMsgDataView->setRandomNum(self->compMsgDataView, fieldInfo);
        checkErrOK(result);
        fieldInfo->fieldFlags |= COMP_MSG_FIELD_IS_SET;
        break;
      case COMP_MSG_SPEC_FIELD_SEQUENCE_NUM:
        result = self->compMsgDataView->setSequenceNum(self->compMsgDataView, fieldInfo);
        checkErrOK(result);
        fieldInfo->fieldFlags |= COMP_MSG_FIELD_IS_SET;
        break;
      case COMP_MSG_SPEC_FIELD_HDR_FILLER:
      case COMP_MSG_SPEC_FIELD_FILLER:
        result = self->compMsgDataView->setFiller(self->compMsgDataView, fieldInfo);
        checkErrOK(result);
        fieldInfo->fieldFlags |= COMP_MSG_FIELD_IS_SET;
        break;
      case COMP_MSG_SPEC_FIELD_CRC:
        headerLgth = 0;
        lgth = self->cmdLgth-fieldInfo->fieldLgth + self->headerLgth;
        if (self->flags & COMP_MSG_CRC_USE_HEADER_LGTH) {
            headerLgth = self->headerLgth;
            lgth -= headerLgth;
        }
        result = self->compMsgDataView->setCrc(self->compMsgDataView, fieldInfo, headerLgth, lgth);
        checkErrOK(result);
        fieldInfo->fieldFlags |= COMP_MSG_FIELD_IS_SET;
        break;
      case COMP_MSG_SPEC_FIELD_TOTAL_CRC:
        headerLgth = 0;
        result = self->compMsgDataView->setTotalCrc(self->compMsgDataView, fieldInfo);
        checkErrOK(result);
        fieldInfo->fieldFlags |= COMP_MSG_FIELD_IS_SET;
        break;
    }
    idx++;
  }
  self->flags |= COMP_MSG_IS_PREPARED;
  return COMP_MSG_ERR_OK;
}

// ================================= initMsg ====================================

static uint8_t initMsg(compMsgData_t *self) {
  int numEntries;
  int idx;
  int row;
  int col;
  int cellIdx;
  int result;
  size_t myLgth;
  size_t fillerLgth;
  size_t crcLgth;
  compMsgField_t *fieldInfo;
  compMsgField_t *fieldInfo2;

  // initialize field offsets for each field
  // initialize totalLgth, headerLgth, cmdLgth
  if ((self->flags & COMP_MSG_IS_INITTED) != 0) {
    return COMP_MSG_ERR_ALREADY_INITTED;
  }
  self->fieldOffset = 0;
  numEntries = self->numFields;
  idx = 0;
  self->headerLgth = 0;
  while (idx < numEntries) {
    fieldInfo = &self->fields[idx];
    fieldInfo->fieldOffset = self->fieldOffset;
//ets_printf("§initMsg2 idx: %d§", idx);
    switch (fieldInfo->fieldNameId) {
      case COMP_MSG_SPEC_FIELD_SRC:
      case COMP_MSG_SPEC_FIELD_DST:
      case COMP_MSG_SPEC_FIELD_TOTAL_LGTH:
      case COMP_MSG_SPEC_FIELD_GUID:
      case COMP_MSG_SPEC_FIELD_SRC_ID:
      case COMP_MSG_SPEC_FIELD_HDR_FILLER:
        self->headerLgth += fieldInfo->fieldLgth;
        self->totalLgth = self->fieldOffset + fieldInfo->fieldLgth;
        break;
      case COMP_MSG_SPEC_FIELD_TABLE_ROW_FIELDS:
        row = 0;
        col = 0;
        while (row < self->numTableRows) {
          while (col < self->numTableRowFields) {
            cellIdx = col + row * self->numRowFields;
            fieldInfo2 = &self->tableFields[cellIdx];
            fieldInfo2->fieldOffset = self->fieldOffset;
            self->fieldOffset += fieldInfo2->fieldLgth;
            col++;
          }
          col = 0; 
          row++;
        }
        break;
      case COMP_MSG_SPEC_FIELD_FILLER:
        fillerLgth = 0;
        crcLgth = 0;
        if (self->flags & COMP_MSG_HAS_CRC) {
          if (self->flags & COMP_MSG_UINT8_CRC) {
            crcLgth = 1;
          } else {
            crcLgth = 2;
          }
        }
        myLgth = self->fieldOffset + crcLgth - self->headerLgth;
        while ((myLgth % 16) != 0) {
          myLgth++;
          fillerLgth++;
        }
        fieldInfo->fieldLgth = fillerLgth;
        self->totalLgth = self->fieldOffset + fillerLgth + crcLgth;
        self->cmdLgth = self->totalLgth - self->headerLgth;
//ets_printf("§initMsg2a idx: %d§", idx);
        break;
      default:
        self->totalLgth = self->fieldOffset + fieldInfo->fieldLgth;
        self->cmdLgth = self->totalLgth - self->headerLgth;
//ets_printf("§initMsg2b idx: %d§", idx);
        break;
    }
    self->fieldOffset += fieldInfo->fieldLgth;
    idx++;
  }
//ets_printf("§initMsg2c§");
  if (self->compMsgDataView->dataView->data != NULL) {
//ets_printf("§initMsg2d %p§", self->compMsgDataView->dataView->data);
//    os_free(self->compMsgDataView->dataView->data);
//ets_printf("§initMsg2e§");
  }
//ets_printf("§initMsg2f§");
  if (self->totalLgth == 0) {
//ets_printf("§initMsg3a§");
    return COMP_MSG_ERR_FIELD_TOTAL_LGTH_MISSING;
  }
//ets_printf("§initMsg3b§");
  result = self->compMsgDataView->dataView->setData(self->compMsgDataView->dataView, self->totalLgth);
//ets_printf("§initMsg3 res: %d§", result);
  checkErrOK(result);
  self->flags |= COMP_MSG_IS_INITTED;
  // set the appropriate field values for the lgth entries
  idx = 0;
  numEntries = self->numFields;
  while (idx < numEntries) {
//ets_printf("§initMsg4 idx: %d§", idx);
    fieldInfo = &self->fields[idx];
    switch (fieldInfo->fieldNameId) {
      case COMP_MSG_SPEC_FIELD_TOTAL_LGTH:
        result = self->compMsgDataView->setFieldValue(self->compMsgDataView, fieldInfo, (int)self->totalLgth, NULL, 0);
        checkErrOK(result);
        fieldInfo->fieldFlags |= COMP_MSG_FIELD_IS_SET;
        break;
      case COMP_MSG_SPEC_FIELD_CMD_LGTH:
        result = self->compMsgDataView->setFieldValue(self->compMsgDataView, fieldInfo, (int)self->cmdLgth, NULL, 0);
        checkErrOK(result);
        fieldInfo->fieldFlags |= COMP_MSG_FIELD_IS_SET;
        break;
    }
    idx++;
  }
  return COMP_MSG_ERR_OK;
}

// ============================= setMsgFieldsFromList ========================

static uint8_t setMsgFieldsFromList(compMsgData_t *self, const uint8_t **listVector, uint8_t numEntries, uint16_t flags) {
  const uint8_t *listEntry;
  int idx;
  int result;
  uint8_t*cp;
  uint8_t *fieldNameStr;
  uint8_t *fieldValueStr;
  uint8_t *stringValue;
  int numericValue;
  char *endPtr;
  uint8_t *flagStr;
  uint8_t flag;
  unsigned long uflag;

  listEntry = listVector[0];
  idx = 0;
  while(idx < numEntries) {
    listEntry = listVector[idx];
    uint8_t buffer[c_strlen(listEntry) + 1];
    fieldNameStr = buffer;
    c_memcpy(fieldNameStr, listEntry, c_strlen(listEntry));
    fieldNameStr[c_strlen(listEntry)] = '\0';
    cp = fieldNameStr;
    while (*cp != ',') {
      cp++;
    }
    *cp++ = '\0';
    fieldValueStr = cp;
    while (*cp != ',') {
      cp++;
    }
    *cp++ = '\0';
    flagStr = cp;
    numericValue = c_strtoul(fieldValueStr, &endPtr, 10);
    if ((uint8_t *)endPtr != cp - 1) {
      numericValue = 0;
      stringValue = fieldValueStr;
    } else {
      stringValue = NULL;
    }
    uflag = c_strtoul(flagStr, &endPtr, 10);
    flag = (uint8_t)uflag;
    if (flag == 0) {
      result = setFieldValue(self, fieldNameStr, numericValue, stringValue);
      checkErrOK(result);
    } else {
      if ((flags != 0) && (flag == 2)) {
        result = setFieldValue(self, fieldNameStr, numericValue, stringValue);
        checkErrOK(result);
      } else {
        if ((flags == 0) && (flag == 1)) {
          result = setFieldValue(self, fieldNameStr, numericValue, stringValue);
          checkErrOK(result);
        }
      }
    }
    idx++;
  }
  return COMP_MSG_ERR_OK;
}

// ============================= newCompMsgDataFromList ========================

uint8_t newCompMsgDataFromList(const uint8_t **listVector, uint8_t numEntries, uint8_t numRows, uint16_t flags, uint8_t **handle) {
  const uint8_t *listEntry;
  int idx;
  int result;
  uint8_t*cp;
  uint8_t *fieldNameStr;
  uint8_t *fieldTypeStr;
  uint8_t *fieldLgthStr;
  char *endPtr;
  uint8_t fieldLgth;
  uint8_t *flagStr;
  uint8_t flag;
  unsigned long lgth;
  unsigned long uflag;
  compMsgData_t *compMsgData;

//ets_printf("createMsgFromListInfo: \n");
  compMsgData = newCompMsgData();
  if (compMsgData == NULL) {
    return COMP_MSG_ERR_OUT_OF_MEMORY;
  }
  result = createMsg(compMsgData, numEntries, handle);
  checkErrOK(result);
  listEntry = listVector[0];
  idx = 0;
  while(idx < numEntries) {
    listEntry = listVector[idx];
    uint8_t buffer[c_strlen(listEntry) + 1];
    fieldNameStr = buffer;
    c_memcpy(fieldNameStr, listEntry, c_strlen(listEntry));
    fieldNameStr[c_strlen(listEntry)] = '\0';
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
    while (*cp != ',') {
      cp++;
    }
    *cp++ = '\0';
    flagStr = cp;
    if (c_strcmp(fieldLgthStr,"@numRows") == 0) {
      fieldLgth = numRows;
    } else {
      lgth = c_strtoul(fieldLgthStr, &endPtr, 10);
      fieldLgth = (uint8_t)lgth;
    }
    uflag = c_strtoul(flagStr, &endPtr, 10);
    flag = (uint8_t)uflag;
    if (flag == 0) {
      result = addField(compMsgData, fieldNameStr, fieldTypeStr, fieldLgth);
      checkErrOK(result);
    } else {
      if ((flags != 0) && (flag == 2)) {
        result = addField(compMsgData, fieldNameStr, fieldTypeStr, fieldLgth);
        checkErrOK(result);
      } else {
        if ((flags == 0) && (flag == 1)) {
          result = addField(compMsgData, fieldNameStr, fieldTypeStr, fieldLgth);
          checkErrOK(result);
        }
      }
    }
    idx++;
  }
  return initMsg(compMsgData);
}

// ================================= dumpFieldValue ====================================

static uint8_t dumpFieldValue(compMsgData_t *self, compMsgField_t *fieldInfo, const uint8_t *indent2) {
  int result;
  int valueIdx;
  uint8_t uch;
  int8_t ch;
  uint16_t ush;
  int16_t sh;
  uint32_t uval;
  int32_t val;
  uint8_t *stringValue;
  int numericValue = 0;

  result = self->compMsgDataView->getFieldValue(self->compMsgDataView, fieldInfo, &numericValue, &stringValue, 0);
  checkErrOK(result);
  switch (fieldInfo->fieldTypeId) {
  case DATA_VIEW_FIELD_INT8_T:
    ets_printf("      %svalue: 0x%02x %d\n", indent2, numericValue & 0xFF, numericValue);
    break;
  case DATA_VIEW_FIELD_UINT8_T:
    ets_printf("      %svalue: 0x%02x %d\n", indent2, numericValue & 0xFF, numericValue & 0xFF);
    break;
  case DATA_VIEW_FIELD_INT16_T:
    ets_printf("      %svalue: 0x%04x %d\n", indent2, numericValue & 0xFFFF, numericValue);
    break;
  case DATA_VIEW_FIELD_UINT16_T:
    ets_printf("      %svalue: 0x%04x %d\n", indent2, numericValue & 0xFFFF, numericValue & 0xFFFF);
    break;
  case DATA_VIEW_FIELD_INT32_T:
    ets_printf("      %svalue: 0x%08x %d\n", indent2, numericValue & 0xFFFFFFFF, numericValue);
    break;
  case DATA_VIEW_FIELD_UINT32_T:
    ets_printf("      %svalue: 0x%08x %d\n", indent2, numericValue & 0xFFFFFFFF, numericValue & 0xFFFFFFFF);
    break;
  case DATA_VIEW_FIELD_INT8_VECTOR:
    valueIdx = 0;
    ets_printf("      values:\n");
    while (valueIdx < fieldInfo->fieldLgth) {
      ch = stringValue[valueIdx];
      ets_printf("        %sidx: %d value: %c 0x%02x %d\n", indent2, valueIdx, (char)ch, (uint8_t)(ch & 0xFF), (int8_t)ch);
      valueIdx++;
    }
    ets_printf("\n");
    break;
  case DATA_VIEW_FIELD_UINT8_VECTOR:
    valueIdx = 0;
    ets_printf("      values:\n");
    while (valueIdx < fieldInfo->fieldLgth) {
      uch = stringValue[valueIdx];
      ets_printf("        idx: %d value: %c 0x%02x %d\n", valueIdx, (char)uch, (uint8_t)(uch & 0xFF), (int)uch);
      valueIdx++;
    }
    break;
  case DATA_VIEW_FIELD_INT16_VECTOR:
    valueIdx = 0;
    ets_printf("      values:");
    while (valueIdx < fieldInfo->fieldLgth) {
      result = self->compMsgDataView->dataView->getInt16(self->compMsgDataView->dataView, fieldInfo->fieldOffset+valueIdx*sizeof(int16_t), &sh);
      ets_printf("        idx: %d value: 0x%04x\n", indent2, valueIdx, sh);
      valueIdx++;
    }
    ets_printf("\n");
    break;
#ifdef NOTDEF
  case DATA_VIEW_FIELD_UINT16_VECTOR:
    valueIdx = 0;
    ets_printf("      values:\n");
    while (valueIdx < fieldInfo->fieldLgth) {
      ush = fieldInfo->value.ushortVector[valueIdx];
      ets_printf("        idx: %d value: 0x%04x\n", indent2, valueIdx, (uint16_t)(ush & 0xFFFF));
      valueIdx++;
    }
    break;
  case DATA_VIEW_FIELD_INT32_VECTOR:
    valueIdx = 0;
    ets_printf("      values:");
    while (valueIdx < fieldInfo->fieldLgth) {
      val = fieldInfo->value.int32Vector[valueIdx];
      ets_printf("        idx: %d value: 0x%08x\n", indent2, valueIdx, (int32_t)(val & 0xFFFFFFFF));
      valueIdx++;
    }
    ets_printf("\n");
    break;
  case DATA_VIEW_FIELD_UINT32_VECTOR:
    valueIdx = 0;
    ets_printf("      values:\n");
    while (valueIdx < fieldInfo->fieldLgth) {
      uval = fieldInfo->value.uint32Vector[valueIdx];
      ets_printf("        idx: %d value: 0x%08x\n", indent2, valueIdx, (uint32_t)(uval & 0xFFFFFFFF));
      valueIdx++;
    }
    break;
#endif
  }
  return DATA_VIEW_ERR_OK;
}

// ============================= dumpTableRowFields ========================

static uint8_t dumpTableRowFields(compMsgData_t *self) {
  int numEntries;
  int idx;
  int result;
  int row;
  int col;
  uint8_t *fieldTypeStr;
  uint8_t *fieldNameStr;
  compMsgField_t *fieldInfo;

  numEntries = self->numTableRows * self->numRowFields;
  ets_printf("    numTableFields: %d\r\n", numEntries);
  idx = 0;
  row = 0;
  col = 0;
  while (idx < numEntries) {
    fieldInfo = &self->tableFields[idx];
    result = self->compMsgDataView->dataView->getFieldTypeStrFromId(self->compMsgDataView->dataView, fieldInfo->fieldTypeId, &fieldTypeStr);
    checkErrOK(result);
    result = self->compMsgDataView->getFieldNameStrFromId(self->compMsgDataView, fieldInfo->fieldNameId, &fieldNameStr);
    checkErrOK(result);
    ets_printf("      row: %d: col: %d fieldName: %-20s fieldType: %-8s fieldLgth: %.5d offset: %d \r\n", row, col, fieldNameStr, fieldTypeStr, fieldInfo->fieldLgth, fieldInfo->fieldOffset);
    if (fieldInfo->fieldFlags & COMP_MSG_FIELD_IS_SET) {
      result = dumpFieldValue(self, fieldInfo, "  ");
      checkErrOK(result);
    }
    col++;
    if (col == self->numRowFields) {
      row++;
      col = 0;
    }
    idx++;
  }
  return COMP_MSG_ERR_OK;
}

// ============================= dumpKeyValueFields ========================

static uint8_t dumpKeyValueFields(compMsgData_t *self, size_t offset) {
  int numEntries;
  int idx;
  int result;
  int row;
  int col;
  uint8_t *fieldTypeStr;
  uint8_t *fieldNameStr;
  compMsgField_t *fieldInfo;

  numEntries = self->numValueFields;
  ets_printf("      numKeyValues: %d offset: %d\r\n", numEntries, offset);
  idx = 0;
  while (idx < numEntries) {
#ifdef NOTEF
    fieldInfo = &self->keyValueFields[idx];
    result = self->compMsgDataView->dataView->getFieldTypeStrFromId(self->compMsgDataView->dataView, fieldInfo->fieldTypeId, &fieldTypeStr);
    checkErrOK(result);
    result = self->compMsgDataView->getFieldNameStrFromId(self->compMsgDataView, fieldInfo->fieldNameId, &fieldNameStr);
    checkErrOK(result);
    ets_printf("      key: %d 0x%04x %s lgth: %d fieldType: %-8s fieldLgth: %.5d offset: %d \r\n", row, col, fieldNameStr, fieldTypeStr, fieldInfo->fieldLgth, fieldInfo->fieldOffset);
    if (fieldInfo->fieldFlags & COMP_MSG_FIELD_IS_SET) {
      result = dumpFieldValue(self, fieldInfo, "  ");
      checkErrOK(result);
    }
    col++;
    if (col == self->numRowFields) {
      row++;
      col = 0;
    }
#endif
    idx++;
  }
  return COMP_MSG_ERR_OK;
}

// ================================= dumpFieldInfo ====================================

static uint8_t dumpFieldInfo(compMsgData_t *self, compMsgField_t *fieldInfo) {
  int idx;
  int result;
  uint8_t *fieldTypeStr;
  uint8_t *fieldNameStr;

  result = self->compMsgDataView->dataView->getFieldTypeStrFromId(self->compMsgDataView->dataView, fieldInfo->fieldTypeId, &fieldTypeStr);
  checkErrOK(result);
  result = self->compMsgDataView->getFieldNameStrFromId(self->compMsgDataView, fieldInfo->fieldNameId, &fieldNameStr);
  checkErrOK(result);
  ets_printf(" fieldName: %-20s fieldType: %-8s fieldLgth: %.5d offset: %d flags: ", fieldNameStr, fieldTypeStr, fieldInfo->fieldLgth, fieldInfo->fieldOffset);
  if (fieldInfo->fieldFlags & COMP_MSG_FIELD_IS_SET) {
    ets_printf(" COMP_MSG_FIELD_IS_SET");
  }
  if (fieldInfo->fieldFlags & COMP_MSG_KEY_VALUE_FIELD) {
    ets_printf(" COMP_MSG_KEY_VALUE_FIELD");
  }
  ets_printf("\r\n");
  ets_printf(" fieldNameId: %.5d fieldTypeId: %.5d fieldKey: 0x%04x %d\r\n", fieldInfo->fieldNameId, fieldInfo->fieldTypeId, fieldInfo->fieldKey, fieldInfo->fieldKey);
  return COMP_MSG_ERR_OK;
}

// ================================= dumpMsg ====================================

static uint8_t dumpMsg(compMsgData_t *self) {
  int numEntries;
  int idx;
  int valueIdx;
  int result;
  uint8_t *fieldTypeStr;
  uint8_t *fieldNameStr;
  compMsgField_t *fieldInfo;

  ets_printf("handle: %s\r\n", self->handle);
  numEntries = self->numFields;
  ets_printf("  numFields: %d maxFields: %d\r\n", numEntries, (int)self->maxFields);
  ets_printf("  headerLgth: %d cmdLgth: %d totalLgth: %d\r\n", self->headerLgth, self->cmdLgth, self->totalLgth);
  ets_printf("  flags:");
  if ((self->flags & COMP_MSG_HAS_CRC) != 0) {
    ets_printf(" COMP_MSG_HAS_CRC");
  }
  if ((self->flags & COMP_MSG_UINT8_CRC) != 0) {
    ets_printf(" COMP_MSG_UNIT8_CRC");
  }
  if ((self->flags & COMP_MSG_HAS_FILLER) != 0) {
    ets_printf(" COMP_MSG_HAS_FILLER");
  }
  if ((self->flags & COMP_MSG_U8_CMD_KEY) != 0) {
    ets_printf(" COMP_MSG_U8_CMD_KEY");
  }
  if ((self->flags & COMP_MSG_HAS_TABLE_ROWS) != 0) {
    ets_printf(" COMP_MSG_HAS_TABLE_ROWS");
  }
  if ((self->flags & COMP_MSG_IS_INITTED) != 0) {
    ets_printf(" COMP_MSG_IS_INITTED");
  }
  if ((self->flags & COMP_MSG_IS_PREPARED) != 0) {
    ets_printf(" COMP_MSG_IS_PREPARED");
  }
  ets_printf("\r\n");
  idx = 0;
  while (idx < numEntries) {
    fieldInfo = &self->fields[idx];
    result = self->compMsgDataView->dataView->getFieldTypeStrFromId(self->compMsgDataView->dataView, fieldInfo->fieldTypeId, &fieldTypeStr);
    checkErrOK(result);
    result = self->compMsgDataView->getFieldNameStrFromId(self->compMsgDataView, fieldInfo->fieldNameId, &fieldNameStr);
    checkErrOK(result);
    if (c_strcmp(fieldNameStr, "@tablerows") == 0) {
      ets_printf("    idx %d: fieldName: %-20s fieldType: %-8s fieldLgth: %.5d offset: %.5d\r\n", idx, fieldNameStr, fieldTypeStr, self->numTableRows, fieldInfo->fieldOffset);
      idx++;
      continue;
    }
    if (c_strcmp(fieldNameStr, "@tablerowfields") == 0) {
      ets_printf("    idx %d: fieldName: %-20s fieldType: %-8s fieldLgth: %.5d offset: %.5d\r\n", idx, fieldNameStr, fieldTypeStr, self->numRowFields, fieldInfo->fieldOffset);
      result = dumpTableRowFields(self);
      checkErrOK(result);
      idx++;
      continue;
    }
    if (c_strcmp(fieldNameStr, "@numKeyValues") == 0) {
      ets_printf("    idx %d: fieldName: %-20s fieldType: %-8s fieldLgth: %.5d offset: %.5d flags: ", idx, fieldNameStr, fieldTypeStr, fieldInfo->fieldLgth, fieldInfo->fieldOffset);
      if (fieldInfo->fieldFlags & COMP_MSG_FIELD_IS_SET) {
        ets_printf(" COMP_MSG_FIELD_IS_SET");
      }
      if (fieldInfo->fieldFlags & COMP_MSG_KEY_VALUE_FIELD) {
        ets_printf(" COMP_MSG_KEY_VALUE_FIELD");
      }
      ets_printf("\r\n");
      if (fieldInfo->fieldFlags & COMP_MSG_FIELD_IS_SET) {
        result = dumpFieldValue(self, fieldInfo, "");
        checkErrOK(result);
      }
      result = dumpKeyValueFields(self, fieldInfo->fieldOffset + fieldInfo->fieldLgth);
      checkErrOK(result);
      idx++;
      continue;
    }
    ets_printf("    idx %d: fieldName: %-20s fieldType: %-8s fieldLgth: %.5d offset: %d key: %.5d flags: ", idx, fieldNameStr, fieldTypeStr, fieldInfo->fieldLgth, fieldInfo->fieldOffset, fieldInfo->fieldKey);
    if (fieldInfo->fieldFlags & COMP_MSG_FIELD_IS_SET) {
      ets_printf(" COMP_MSG_FIELD_IS_SET");
    }
    if (fieldInfo->fieldFlags & COMP_MSG_KEY_VALUE_FIELD) {
      ets_printf(" COMP_MSG_KEY_VALUE_FIELD");
    }
    ets_printf("\r\n");
    if (fieldInfo->fieldFlags & COMP_MSG_FIELD_IS_SET) {
      result = dumpFieldValue(self, fieldInfo, "");
      checkErrOK(result);
    }
    idx++;
  }
  return COMP_MSG_ERR_OK;
}

// ============================= getMsgData ========================

static uint8_t getMsgData(compMsgData_t *self, uint8_t **data, int *lgth) {
  if ((self->flags & COMP_MSG_IS_INITTED) == 0) {
    return COMP_MSG_ERR_NOT_YET_INITTED;
  }
  if ((self->flags & COMP_MSG_IS_PREPARED) == 0) {
    return COMP_MSG_ERR_NOT_YET_PREPARED;
  }
  *data = self->compMsgDataView->dataView->data;
  *lgth = self->totalLgth;
  return COMP_MSG_ERR_OK;
}

// ============================= setMsgData ========================

static uint8_t setMsgData(compMsgData_t *self, const uint8_t *data) {
  int idx;
  int found;
  int result;
  uint16_t lgth;
  compMsgField_t *fieldInfo;

  if ((self->flags & COMP_MSG_IS_INITTED) == 0) {
    return COMP_MSG_ERR_NOT_YET_INITTED;
  }
  found = 0;
  // check lgth
  while (idx < self->numFields) {
    fieldInfo = &self->fields[idx];
    if (fieldInfo->fieldNameId == COMP_MSG_SPEC_FIELD_TOTAL_LGTH) {
      found = 1;
      break;
    }
    idx++;
  }
  if (! found) {
    return COMP_MSG_ERR_NO_SUCH_FIELD;
  } 
  if (self->compMsgDataView->dataView->data != NULL) {
    // free no longer used
    os_free(self->compMsgDataView->dataView->data);
  }
  // temporary replace data entry of dataView by our param data
  // to be able to use the get* functions for gettting totalLgth entry value
  self->compMsgDataView->dataView->data = (uint8_t *)data;
  // get totalLgth value from data
  result = self->compMsgDataView->dataView->getUint16(self->compMsgDataView->dataView, fieldInfo->fieldOffset, &lgth);
  checkErrOK(result);
  if (lgth != self->totalLgth) {
    return COMP_MSG_ERR_BAD_DATA_LGTH;
  }
  // now make a copy of the data to be on the safe side
  // for freeing the Lua space in Lua set the variable to nil!!
  self->compMsgDataView->dataView->data = os_zalloc(self->totalLgth);
  checkAllocOK(self->compMsgDataView->dataView->data);
  c_memcpy(self->compMsgDataView->dataView->data, data, self->totalLgth);
  // and now set the IS_SET flags and other stuff
  idx = 0;
  while (idx < self->numFields) {
    fieldInfo = &self->fields[idx];
    fieldInfo->fieldFlags |= COMP_MSG_FIELD_IS_SET;
    idx++;
  }
  self->flags |= COMP_MSG_IS_PREPARED;
  return COMP_MSG_ERR_OK;
}

// ================================= freeCompMsgData ====================================

static uint8_t freeCompMsgData(compMsgData_t *self) {
  if (self->compMsgDataView != NULL) {
    freeCompMsgDataView(self->compMsgDataView);
  }
  
  if (self->fields != NULL) {
    os_free(self->fields);
  }
  if (self->tableFields != NULL) {
    os_free(self->tableFields);
  }
  if (self->header != NULL) {
    os_free(self->header);
  }
  os_free(self);
  return COMP_MSG_ERR_OK;
}

// ================================= deleteMsg ====================================

static uint8_t deleteMsg(compMsgData_t *self) {
  int result;

  result = freeCompMsgData(self); 
  checkErrOK(result);
  return COMP_MSG_ERR_OK;
}

// ================================= setDispatcher ====================================

static uint8_t setDispatcher(compMsgData_t *self, compMsgDispatcher_t *dispatcher) {
  self->compMsgDispatcher = dispatcher;
  return COMP_MSG_ERR_OK;
}

// ================================= newCompMsgData ====================================

compMsgData_t *newCompMsgData(void) {
  compMsgData_t *compMsgData = os_zalloc(sizeof(compMsgData_t));
  if (compMsgData == NULL) {
    return NULL;
  }
  compMsgData->compMsgDataView = newCompMsgDataView();
  if (compMsgData->compMsgDataView == NULL) {
    return NULL;
  }
  
  compMsgData->fields = NULL;
  compMsgData->tableFields = NULL;
  compMsgData->header = NULL;
  compMsgData->flags = 0;
  compMsgData->numFields = 0;
  compMsgData->maxFields = 0;
  compMsgData->numTableRows = 0;
  compMsgData->numTableRowFields = 0;
  compMsgData->numRowFields = 0;
  compMsgData->fieldOffset = 0;
  compMsgData->totalLgth = 0;
  compMsgData->cmdLgth = 0;
  compMsgData->headerLgth = 0;

  // normalMsg
  compMsgData->createMsg = &createMsg;
  compMsgData->deleteMsg = &deleteMsg;
  compMsgData->addField = &addField;
  compMsgData->getFieldValue = &getFieldValue;
  compMsgData->setFieldValue = &setFieldValue;
  compMsgData->getTableFieldValue = &getTableFieldValue;
  compMsgData->setTableFieldValue = &setTableFieldValue;
  compMsgData->dumpFieldValue = &dumpFieldValue;
  compMsgData->dumpTableRowFields = &dumpTableRowFields;
  compMsgData->dumpKeyValueFields = &dumpKeyValueFields;
  compMsgData->dumpFieldInfo = &dumpFieldInfo;
  compMsgData->dumpMsg = &dumpMsg;
  compMsgData->initMsg = &initMsg;
  compMsgData->prepareMsg = &prepareMsg;
  compMsgData->getMsgData = &getMsgData;
  compMsgData->setMsgData = &setMsgData;
  compMsgData->setMsgFieldsFromList = &setMsgFieldsFromList;
  compMsgData->setDispatcher = &setDispatcher;

  return compMsgData;
}
