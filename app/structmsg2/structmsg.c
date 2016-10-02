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
#include "c_stdlib.h"
#include "structmsg2.h"
#include "../crypto/mech.h"

typedef struct handle2Header 
{
  uint8_t *handle;
  uint8_t *header;
  structmsgData_t *structmsgData;
} handle2Header_t;

typedef struct structmsgHandles
{
  handle2Header_t *handles;
  int numHandles;
} structmsgHandles_t;

// create an object
static structmsgHandles_t structmsgHandles = { NULL, 0};


// ============================= addHandle ========================

static int addHandle(uint8_t *handle, structmsgData_t *structmsgData, uint8_t **headerPtr) {
  int idx;

  if (structmsgHandles.handles == NULL) {
    structmsgHandles.handles = os_zalloc(sizeof(handle2Header_t));
    if (structmsgHandles.handles == NULL) {
      return STRUCT_MSG_ERR_OUT_OF_MEMORY;
    } else {
      structmsgHandles.handles[structmsgHandles.numHandles].handle = handle;
      structmsgHandles.handles[structmsgHandles.numHandles].structmsgData = structmsgData;
      *headerPtr = structmsgHandles.handles[structmsgHandles.numHandles++].header;
      return STRUCT_MSG_ERR_OK;
    }
  } else {
    // check for unused slot first
    idx = 0;
    while (idx < structmsgHandles.numHandles) {
      if (structmsgHandles.handles[idx].handle == NULL) {
        structmsgHandles.handles[idx].handle = handle;
        structmsgHandles.handles[idx].structmsgData = structmsgData;
        *headerPtr = structmsgHandles.handles[idx].header;
        return STRUCT_MSG_ERR_OK;
      }
      idx++;
    }
    structmsgHandles.handles = os_realloc(structmsgHandles.handles, sizeof(handle2Header_t)*(structmsgHandles.numHandles+1));
    checkAllocOK(structmsgHandles.handles);
    structmsgHandles.handles[structmsgHandles.numHandles].handle = handle;
    structmsgHandles.handles[idx].structmsgData = structmsgData;
    *headerPtr = structmsgHandles.handles[structmsgHandles.numHandles++].header;
  }
  return STRUCT_MSG_ERR_OK;
}

// ============================= deleteHandle ========================

static int deleteHandle(const uint8_t *handle) {
  int idx;
  int numUsed;
  int found;

  if (structmsgHandles.handles == NULL) {
    return STRUCT_MSG_ERR_HANDLE_NOT_FOUND;
  }
  found = 0;
  idx = 0;
  numUsed = 0;
  while (idx < structmsgHandles.numHandles) {
    if ((structmsgHandles.handles[idx].handle != NULL) && (c_strcmp(structmsgHandles.handles[idx].handle, handle) == 0)) {
      structmsgHandles.handles[idx].handle = NULL;
      found++;
    } else {
      if (structmsgHandles.handles[idx].handle != NULL) {
        numUsed++;
      }
    }
    idx++;
  }
  if (numUsed == 0) {
    os_free(structmsgHandles.handles);
    structmsgHandles.handles = NULL;
  }
  if (found) {
      return STRUCT_MSG_ERR_OK;
  }
  return STRUCT_MSG_ERR_HANDLE_NOT_FOUND;
}

// ============================= checkHandle ========================

static int checkHandle(const char *handle, structmsgData_t **structmsgData) {
  int idx;

  if (structmsgHandles.handles == NULL) {
    return STRUCT_MSG_ERR_HANDLE_NOT_FOUND;
  }
  idx = 0;
  while (idx < structmsgHandles.numHandles) {
    if ((structmsgHandles.handles[idx].handle != NULL) && (c_strcmp(structmsgHandles.handles[idx].handle, handle) == 0)) {
      *structmsgData = structmsgHandles.handles[idx].structmsgData;
      return STRUCT_MSG_ERR_OK;
    }
    idx++;
  }
  return STRUCT_MSG_ERR_HANDLE_NOT_FOUND;
}

// ============================= structmsgGetPtrFromHandle ========================

uint8_t structmsgGetPtrFromHandle(const char *handle, structmsgData_t **structmsgData) {
  int id;
  const char hex[] = "0123456789abcdef\0";
  int num;
  int len;
  int shift;
  const char *cp;
  int idx;

  if (checkHandle(handle, structmsgData) != STRUCT_MSG_ERR_OK) {
    return STRUCT_MSG_ERR_HANDLE_NOT_FOUND;
  }
  return STRUCT_MSG_ERR_OK;
}

// ================================= createMsg ====================================

static uint8_t createMsg(structmsgData_t *self, int numFields, uint8_t **handle) {
  uint8_t result;
  self->fields = os_zalloc(sizeof(structmsgField_t) * numFields);
  if (self->fields == NULL) {
    return STRUCT_MSG_ERR_OUT_OF_MEMORY;
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
  if (result != STRUCT_MSG_ERR_OK) {
    os_free(self->fields);
    deleteHandle(self->handle);
    os_free(self);
    return result;
  }
  *handle = self->handle;
  return STRUCT_MSG_ERR_OK;
}

// ================================= addField ====================================

static uint8_t addField(structmsgData_t *self, const uint8_t *fieldName, const uint8_t *fieldType, uint8_t fieldLgth) {
  uint8_t numTableFields;
  uint8_t numTableRowFields;
  uint8_t numTableRows;
  uint8_t fieldTypeId = 0;
  uint8_t fieldNameId = 213;
  int row;
  int cellIdx;
  int result = STRUCT_MSG_ERR_OK;
  structmsgField_t *fieldInfo;

//ets_printf("addfield: %s fieldType: %s fieldLgth: %d\n", fieldName, fieldType, fieldLgth);
  if (self->numFields >= self->maxFields) {
    return STRUCT_MSG_ERR_TOO_MANY_FIELDS;
  }
  result = self->structmsgDataView->dataView->getFieldTypeIdFromStr(self->structmsgDataView->dataView, fieldType, &fieldTypeId);
  checkErrOK(result);
  result = self->structmsgDataView->getFieldNameIdFromStr(self->structmsgDataView, fieldName, &fieldNameId, STRUCT_MSG_INCR);
  checkErrOK(result);
  fieldInfo = &self->fields[self->numFields];
  // need to check for duplicate here !!
  if (c_strcmp(fieldName, "@filler") == 0) {
    self->flags |= STRUCT_MSG_HAS_FILLER;
    fieldLgth = 0;
    fieldInfo->fieldNameId = fieldNameId;
    fieldInfo->fieldTypeId = fieldTypeId;
    fieldInfo->fieldLgth = fieldLgth;
    self->numFields++;
    return STRUCT_MSG_ERR_OK;
  }
  if (c_strcmp(fieldName, "@tablerows") == 0) {
    self->numTableRows = fieldLgth;
    fieldLgth = 0;
    self->flags |= STRUCT_MSG_HAS_TABLE_ROWS;
    fieldInfo->fieldNameId = fieldNameId;
    fieldInfo->fieldTypeId = fieldTypeId;
    fieldInfo->fieldLgth = fieldLgth;
    self->numFields++;
    return STRUCT_MSG_ERR_OK;
  }
  if (c_strcmp(fieldName, "@tablerowfields") == 0) {
    self->numTableRowFields = fieldLgth;
    fieldLgth = 0;
    fieldInfo->fieldNameId = fieldNameId;
    fieldInfo->fieldTypeId = fieldTypeId;
    fieldInfo->fieldLgth = fieldLgth;
    numTableFields = self->numTableRows * self->numTableRowFields;
    if ((self->tableFields == NULL) && (numTableFields != 0)) {
      self->tableFields = os_zalloc(sizeof(structmsgField_t) * numTableFields);
      checkAllocOK(self->tableFields);
    }
    self->numFields++;
    return STRUCT_MSG_ERR_OK;
  }
  numTableRowFields = self->numTableRowFields;
  numTableRows = self->numTableRows;
  numTableFields = numTableRows * numTableRowFields;
  if (!((numTableFields > 0) && (self->numRowFields < numTableRowFields)) || (numTableRowFields == 0)) {
    numTableFields = 0;
    numTableRows = 1;
    numTableRowFields = 0;

    if (c_strcmp(fieldName, "@crc") == 0) {
      self->flags |= STRUCT_MSG_HAS_CRC;
      if (c_strcmp(fieldType, "uint8_t") == 0) {
        self->flags |= STRUCT_MSG_UINT8_CRC;
      }
//ets_printf("flags: 0x%02x HAS_CRC: 0x%02x HAS_FILLER: 0x%02x UINT8_CRC: 0x%02x\n", structmsg->flags, STRUCT_MSG_HAS_CRC, STRUCT_MSG_HAS_FILLER, STRUCT_MSG_UINT8_CRC);
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
        return STRUCT_MSG_ERR_BAD_TABLE_ROW;
      }
      fieldInfo = &self->tableFields[cellIdx];
//ets_printf("table field1: %s totalLgth: %d cmdLgth: %d\n", fieldInfo->fieldStr, structmsg->hdr.hdrInfo.hdrKeys.totalLgth, structmsg->hdr.hdrInfo.hdrKeys.cmdLgth);
      fieldInfo->fieldNameId = fieldNameId;
      fieldInfo->fieldTypeId = fieldTypeId;
      fieldInfo->fieldLgth = fieldLgth;
      row++;
    }
    self->numRowFields++;  
  } 
  return STRUCT_MSG_ERR_OK;
}

// ================================= getFieldValue ====================================

static uint8_t getFieldValue(structmsgData_t *self, const uint8_t *fieldName, int *numericValue, uint8_t **stringValue) {
  structmsgField_t *fieldInfo;
  uint8_t fieldNameId;
  int idx;
  int numEntries;
  int result;

  if ((self->flags & STRUCT_MSG_IS_INITTED) == 0) {
    return STRUCT_MSG_ERR_NOT_YET_INITTED;
  }
  result = self->structmsgDataView->getFieldNameIdFromStr(self->structmsgDataView, fieldName, &fieldNameId, STRUCT_MSG_NO_INCR);
  checkErrOK(result);
  idx = 0;
  numEntries = self->numFields;
  while (idx < numEntries) {
    fieldInfo = &self->fields[idx];
    if (fieldNameId == fieldInfo->fieldNameId) {
      result = self->structmsgDataView->getFieldValue(self->structmsgDataView, fieldInfo, numericValue, stringValue, 0);
      checkErrOK(result);
      break;
    }
    idx++;
  }
  return DATA_VIEW_ERR_OK;
}

// ================================= setFieldValue ====================================

static uint8_t setFieldValue(structmsgData_t *self, const uint8_t *fieldName, int numericValue, const uint8_t *stringValue) {
  structmsgField_t *fieldInfo;
  uint8_t fieldNameId;
  int idx;
  int numEntries;
  int result;

//ets_printf("setFieldValue: fieldName: %s numericValue: %d stringValue: %s flags: 0x%08x\n", fieldName, numericValue, stringValue == NULL ? "nil" : (char *)stringValue, self->flags & STRUCT_MSG_IS_INITTED);
  if ((self->flags & STRUCT_MSG_IS_INITTED) == 0) {
    return STRUCT_MSG_ERR_NOT_YET_INITTED;
  }
  result = self->structmsgDataView->getFieldNameIdFromStr(self->structmsgDataView, fieldName, &fieldNameId, STRUCT_MSG_NO_INCR);
  checkErrOK(result);
  switch (fieldNameId) {
    case STRUCT_MSG_SPEC_FIELD_TOTAL_LGTH:
    case STRUCT_MSG_SPEC_FIELD_CMD_LGTH:
    case STRUCT_MSG_SPEC_FIELD_FILLER:
    case STRUCT_MSG_SPEC_FIELD_CRC:
    case STRUCT_MSG_SPEC_FIELD_RANDOM_NUM:
    case STRUCT_MSG_SPEC_FIELD_SEQUENCE_NUM:
      return STRUCT_MSG_ERR_FIELD_CANNOT_BE_SET;
  }
  idx = 0;
  numEntries = self->numFields;
//ets_printf("numEntries: %d\n", numEntries);
  while (idx < numEntries) {
    fieldInfo = &self->fields[idx];
    if (fieldNameId == fieldInfo->fieldNameId) {
      result = self->structmsgDataView->setFieldValue(self->structmsgDataView, fieldInfo, numericValue, stringValue, 0);
      checkErrOK(result);
      fieldInfo->fieldFlags |= STRUCT_MSG_FIELD_IS_SET;
      break;
    }
    idx++;
  }
//ets_printf("idx: %d\n", idx);
  return DATA_VIEW_ERR_OK;
}


// ================================= getTableFieldValue ====================================

static uint8_t getTableFieldValue(structmsgData_t *self, const uint8_t *fieldName, int row, int *numericValue, uint8_t **stringValue) {
  structmsgField_t *fieldInfo;
  uint8_t fieldNameId;
  int idx;
  int cellIdx;
  int numEntries;
  int result;

  if ((self->flags & STRUCT_MSG_IS_INITTED) == 0) {
    return STRUCT_MSG_ERR_NOT_YET_INITTED;
  }
  result = self->structmsgDataView->getFieldNameIdFromStr(self->structmsgDataView, fieldName, &fieldNameId, STRUCT_MSG_NO_INCR);
  checkErrOK(result);
  if (fieldName[0] == '@') {
    return STRUCT_MSG_ERR_NO_SUCH_FIELD;
  }
  if (row >= self->numTableRows) {
    return STRUCT_MSG_ERR_BAD_TABLE_ROW;
  }
  idx = 0;
  cellIdx = 0 + row * self->numRowFields;
  while (idx < self->numRowFields) {
    fieldInfo = &self->tableFields[cellIdx];
    if (fieldNameId == fieldInfo->fieldNameId) {
      return self->structmsgDataView->getFieldValue(self->structmsgDataView, fieldInfo, numericValue, stringValue, 0);
    }
    cellIdx++;
    idx++;
  }
  return STRUCT_MSG_ERR_FIELD_NOT_FOUND;
}

// ================================= setTableFieldValue ====================================

static uint8_t setTableFieldValue(structmsgData_t *self, const uint8_t *fieldName, int row, int numericValue, const uint8_t *stringValue) {
  structmsgField_t *fieldInfo;
  uint8_t fieldNameId;
  int idx;
  int cellIdx;
  int numEntries;
  int result;

  if ((self->flags & STRUCT_MSG_IS_INITTED) == 0) {
    return STRUCT_MSG_ERR_NOT_YET_INITTED;
  }
  result = self->structmsgDataView->getFieldNameIdFromStr(self->structmsgDataView, fieldName, &fieldNameId, STRUCT_MSG_NO_INCR);
  checkErrOK(result);
  if (fieldName[0] == '@') {
    return STRUCT_MSG_ERR_FIELD_CANNOT_BE_SET;
  }
  if (row >= self->numTableRows) {
    return STRUCT_MSG_ERR_BAD_TABLE_ROW;
  }
  idx = 0;
  cellIdx = 0 + row * self->numRowFields;
  while (idx < self->numRowFields) {
    fieldInfo = &self->tableFields[cellIdx];
    if (fieldNameId == fieldInfo->fieldNameId) {
      fieldInfo->fieldFlags |= STRUCT_MSG_FIELD_IS_SET;
      return self->structmsgDataView->setFieldValue(self->structmsgDataView, fieldInfo, numericValue, stringValue, 0);
    }
    cellIdx++;
    idx++;
  }
  return STRUCT_MSG_ERR_FIELD_NOT_FOUND;
}

// ================================= prepareMsg ====================================

static uint8_t prepareMsg(structmsgData_t *self) {
  int numEntries;
  int idx;
  int result;
  structmsgField_t *fieldInfo;

  if ((self->flags & STRUCT_MSG_IS_INITTED) == 0) {
    return STRUCT_MSG_ERR_NOT_YET_INITTED;
  }
  // create the values which are different for each message!!
  numEntries = self->numFields;
  idx = 0;
  while (idx < numEntries) {
    fieldInfo = &self->fields[idx];
    switch (fieldInfo->fieldNameId) {
      case STRUCT_MSG_SPEC_FIELD_RANDOM_NUM:
        result = self->structmsgDataView->setRandomNum(self->structmsgDataView, fieldInfo);
        checkErrOK(result);
        fieldInfo->fieldFlags |= STRUCT_MSG_FIELD_IS_SET;
        break;
      case STRUCT_MSG_SPEC_FIELD_SEQUENCE_NUM:
        result = self->structmsgDataView->setSequenceNum(self->structmsgDataView, fieldInfo);
        checkErrOK(result);
        fieldInfo->fieldFlags |= STRUCT_MSG_FIELD_IS_SET;
        break;
      case STRUCT_MSG_SPEC_FIELD_FILLER:
        result = self->structmsgDataView->setFiller(self->structmsgDataView, fieldInfo);
        checkErrOK(result);
        fieldInfo->fieldFlags |= STRUCT_MSG_FIELD_IS_SET;
        break;
      case STRUCT_MSG_SPEC_FIELD_CRC:
        result = self->structmsgDataView->setCrc(self->structmsgDataView, fieldInfo, self->headerLgth, self->cmdLgth-fieldInfo->fieldLgth);
        checkErrOK(result);
        fieldInfo->fieldFlags |= STRUCT_MSG_FIELD_IS_SET;
        break;
    }
    idx++;
  }
  self->flags |= STRUCT_MSG_IS_PREPARED;
  return STRUCT_MSG_ERR_OK;
}

// ================================= initMsg ====================================

static uint8_t initMsg(structmsgData_t *self) {
  int numEntries;
  int idx;
  int row;
  int col;
  int cellIdx;
  int result;
  size_t myLgth;
  size_t fillerLgth;
  size_t crcLgth;
  structmsgField_t *fieldInfo;
  structmsgField_t *fieldInfo2;

  // initialize field offsets for each field
  // initialize totalLgth, headerLgth, cmdLgth
  if ((self->flags & STRUCT_MSG_IS_INITTED) != 0) {
    return STRUCT_MSG_ERR_ALREADY_INITTED;
  }
  self->fieldOffset = 0;
  numEntries = self->numFields;
  idx = 0;
  self->headerLgth = 0;
  while (idx < numEntries) {
    fieldInfo = &self->fields[idx];
    fieldInfo->fieldOffset = self->fieldOffset;
    switch (fieldInfo->fieldNameId) {
      case STRUCT_MSG_SPEC_FIELD_SRC:
      case STRUCT_MSG_SPEC_FIELD_DST:
      case STRUCT_MSG_SPEC_FIELD_TOTAL_LGTH:
//      case STRUCT_MSG_SPEC_FIELD_GUID:
        self->headerLgth += fieldInfo->fieldLgth;
        self->totalLgth = self->fieldOffset + fieldInfo->fieldLgth;
        break;
      case STRUCT_MSG_SPEC_FIELD_TABLE_ROW_FIELDS:
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
      case STRUCT_MSG_SPEC_FIELD_FILLER:
        fillerLgth = 0;
        crcLgth = 0;
        if (self->flags & STRUCT_MSG_HAS_CRC) {
          if (self->flags & STRUCT_MSG_UINT8_CRC) {
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
        break;
      default:
        self->totalLgth = self->fieldOffset + fieldInfo->fieldLgth;
        self->cmdLgth = self->totalLgth - self->headerLgth;
        break;
    }
    self->fieldOffset += fieldInfo->fieldLgth;
    idx++;
  }
  if (self->structmsgDataView->dataView->data != NULL) {
    os_free(self->structmsgDataView->dataView->data);
  }
  if (self->totalLgth == 0) {
    return STRUCT_MSG_ERR_FIELD_TOTAL_LGTH_MISSIING;
  }
  result = self->structmsgDataView->dataView->setData(self->structmsgDataView->dataView, self->totalLgth);
  checkErrOK(result);
  self->flags |= STRUCT_MSG_IS_INITTED;
  // set the appropriate field values for the lgth entries
  idx = 0;
  numEntries = self->numFields;
  while (idx < numEntries) {
    fieldInfo = &self->fields[idx];
    switch (fieldInfo->fieldNameId) {
      case STRUCT_MSG_SPEC_FIELD_TOTAL_LGTH:
        result = self->structmsgDataView->setFieldValue(self->structmsgDataView, fieldInfo, (int)self->totalLgth, NULL, 0);
        checkErrOK(result);
        fieldInfo->fieldFlags |= STRUCT_MSG_FIELD_IS_SET;
        break;
      case STRUCT_MSG_SPEC_FIELD_CMD_LGTH:
        result = self->structmsgDataView->setFieldValue(self->structmsgDataView, fieldInfo, (int)self->cmdLgth, NULL, 0);
        checkErrOK(result);
        fieldInfo->fieldFlags |= STRUCT_MSG_FIELD_IS_SET;
        break;
    }
    idx++;
  }
  return STRUCT_MSG_ERR_OK;
}

// ============================= setMsgFieldsFromList ========================

static uint8_t setMsgFieldsFromList(structmsgData_t *self, const uint8_t **listVector, uint8_t numEntries, uint16_t flags) {
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
  return STRUCT_MSG_ERR_OK;
}

// ============================= newStructmsgDataFromList ========================

uint8_t newStructmsgDataFromList(const uint8_t **listVector, uint8_t numEntries, uint8_t numRows, uint16_t flags, uint8_t **handle) {
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
  structmsgData_t *structmsgData;

//ets_printf("createMsgFromListInfo: \n");
  structmsgData = newStructmsgData();
  if (structmsgData == NULL) {
    return STRUCT_MSG_ERR_OUT_OF_MEMORY;
  }
  result = createMsg(structmsgData, numEntries, handle);
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
      result = addField(structmsgData, fieldNameStr, fieldTypeStr, fieldLgth);
      checkErrOK(result);
    } else {
      if ((flags != 0) && (flag == 2)) {
        result = addField(structmsgData, fieldNameStr, fieldTypeStr, fieldLgth);
        checkErrOK(result);
      } else {
        if ((flags == 0) && (flag == 1)) {
          result = addField(structmsgData, fieldNameStr, fieldTypeStr, fieldLgth);
          checkErrOK(result);
        }
      }
    }
    idx++;
  }
  return initMsg(structmsgData);
}

// ================================= dumpFieldValue ====================================

static uint8_t dumpFieldValue(structmsgData_t *self, structmsgField_t *fieldInfo, const uint8_t *indent2) {
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

  result = self->structmsgDataView->getFieldValue(self->structmsgDataView, fieldInfo, &numericValue, &stringValue, 0);
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
    ets_printf("      %svalues:");
    while (valueIdx < fieldInfo->fieldLgth) {
      ch = stringValue[valueIdx];
      ets_printf("        %sidx: %d value: %c 0x%02x\n", indent2, valueIdx, (char)ch, (uint8_t)(ch & 0xFF));
      valueIdx++;
    }
    ets_printf("\n");
    break;
  case DATA_VIEW_FIELD_UINT8_VECTOR:
    valueIdx = 0;
    ets_printf("      values:\n");
    while (valueIdx < fieldInfo->fieldLgth) {
      uch = stringValue[valueIdx];
      ets_printf("        idx: %d value: %c 0x%02x\n", valueIdx, (char)uch, (uint8_t)(uch & 0xFF));
      valueIdx++;
    }
    break;
  case DATA_VIEW_FIELD_INT16_VECTOR:
    valueIdx = 0;
    ets_printf("      values:");
    while (valueIdx < fieldInfo->fieldLgth) {
      result = self->structmsgDataView->dataView->getInt16(self->structmsgDataView->dataView, fieldInfo->fieldOffset+valueIdx*sizeof(int16_t), &sh);
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

static int dumpTableRowFields(structmsgData_t *self) {
  int numEntries;
  int idx;
  int result;
  int row;
  int col;
  uint8_t *fieldTypeStr;
  uint8_t *fieldNameStr;
  structmsgField_t *fieldInfo;

  numEntries = self->numTableRows * self->numRowFields;
  ets_printf("    numTableFields: %d\r\n", numEntries);
  idx = 0;
  row = 0;
  col = 0;
  while (idx < numEntries) {
    fieldInfo = &self->tableFields[idx];
    result = self->structmsgDataView->dataView->getFieldTypeStrFromId(self->structmsgDataView->dataView, fieldInfo->fieldTypeId, &fieldTypeStr);
    checkErrOK(result);
    result = self->structmsgDataView->getFieldNameStrFromId(self->structmsgDataView, fieldInfo->fieldNameId, &fieldNameStr);
    checkErrOK(result);
    ets_printf("      row: %d: col: %d fieldName: %-20s fieldType: %-8s fieldLgth: %.5d offset: %d \r\n", row, col, fieldNameStr, fieldTypeStr, fieldInfo->fieldLgth, fieldInfo->fieldOffset);
    if (fieldInfo->fieldFlags & STRUCT_MSG_FIELD_IS_SET) {
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
  return STRUCT_MSG_ERR_OK;
}

// ================================= dumpMsg ====================================

static uint8_t dumpMsg(structmsgData_t *self) {
  int numEntries;
  int idx;
  int valueIdx;
  int result;
  uint8_t *fieldTypeStr;
  uint8_t *fieldNameStr;
  structmsgField_t *fieldInfo;

  ets_printf("handle: %s\r\n", self->handle);
  numEntries = self->numFields;
  ets_printf("  numFields: %d maxFields: %d\r\n", numEntries, (int)self->maxFields);
  ets_printf("  headerLgth: %d cmdLgth: %d totalLgth: %d\r\n", self->headerLgth, self->cmdLgth, self->totalLgth);
  ets_printf("  flags:");
  if ((self->flags & STRUCT_MSG_FIELD_IS_SET) != 0) {
    ets_printf(" STRUCT_MSG_FIELD_IS_SET");
  }
  if ((self->flags & STRUCT_MSG_HAS_CRC) != 0) {
    ets_printf(" STRUCT_MSG_HAS_CRC");
  }
  if ((self->flags & STRUCT_MSG_UINT8_CRC) != 0) {
    ets_printf(" STRUCT_MSG_UNIT8_CRC");
  }
  if ((self->flags & STRUCT_MSG_HAS_FILLER) != 0) {
    ets_printf(" STRUCT_MSG_HAS_FILLER");
  }
  if ((self->flags & STRUCT_MSG_SHORT_CMD_KEY) != 0) {
    ets_printf(" STRUCT_MSG_SHORT_CMD_KEY");
  }
  if ((self->flags & STRUCT_MSG_HAS_TABLE_ROWS) != 0) {
    ets_printf(" STRUCT_MSG_HAS_TABLE_ROWS");
  }
  if ((self->flags & STRUCT_MSG_IS_INITTED) != 0) {
    ets_printf(" STRUCT_MSG_IS_INITTED");
  }
  if ((self->flags & STRUCT_MSG_IS_PREPARED) != 0) {
    ets_printf(" STRUCT_MSG_IS_PREPARED");
  }
  ets_printf("\r\n");
  idx = 0;
  while (idx < numEntries) {
    fieldInfo = &self->fields[idx];
    result = self->structmsgDataView->dataView->getFieldTypeStrFromId(self->structmsgDataView->dataView, fieldInfo->fieldTypeId, &fieldTypeStr);
    checkErrOK(result);
    result = self->structmsgDataView->getFieldNameStrFromId(self->structmsgDataView, fieldInfo->fieldNameId, &fieldNameStr);
    checkErrOK(result);
    if (c_strcmp(fieldNameStr, "@tablerows") == 0) {
      ets_printf("    idx %d: fieldName: %-20s fieldType: %-8s fieldLgth: %.5d\r\n", idx, fieldNameStr, fieldTypeStr, self->numTableRows);
      idx++;
      continue;
    }
    if (c_strcmp(fieldNameStr, "@tablerowfields") == 0) {
      ets_printf("    idx %d: fieldName: %-20s fieldType: %-8s fieldLgth: %.5d\r\n", idx, fieldNameStr, fieldTypeStr, self->numRowFields);
      result = dumpTableRowFields(self);
      checkErrOK(result);
      idx++;
      continue;
    }
    ets_printf("    idx %d: fieldName: %-20s fieldType: %-8s fieldLgth: %.5d offset: %d\r\n", idx, fieldNameStr, fieldTypeStr, fieldInfo->fieldLgth, fieldInfo->fieldOffset);
    if (fieldInfo->fieldFlags & STRUCT_MSG_FIELD_IS_SET) {
      result = dumpFieldValue(self, fieldInfo, "");
      checkErrOK(result);
    }
    idx++;
  }
  return STRUCT_MSG_ERR_OK;
}

// ============================= getMsgData ========================

static uint8_t getMsgData(structmsgData_t *self, uint8_t **data, int *lgth) {
  if ((self->flags & STRUCT_MSG_IS_INITTED) == 0) {
    return STRUCT_MSG_ERR_NOT_YET_INITTED;
  }
  if ((self->flags & STRUCT_MSG_IS_PREPARED) == 0) {
    return STRUCT_MSG_ERR_NOT_YET_PREPARED;
  }
  *data = self->structmsgDataView->dataView->data;
  *lgth = self->totalLgth;
  return STRUCT_MSG_ERR_OK;
}

// ============================= setMsgData ========================

static uint8_t setMsgData(structmsgData_t *self, const uint8_t *data) {
  int idx;
  int found;
  int result;
  uint16_t lgth;
  structmsgField_t *fieldInfo;

  if ((self->flags & STRUCT_MSG_IS_INITTED) == 0) {
    return STRUCT_MSG_ERR_NOT_YET_INITTED;
  }
  found = 0;
  // check lgth
  while (idx < self->numFields) {
    fieldInfo = &self->fields[idx];
    if (fieldInfo->fieldNameId == STRUCT_MSG_SPEC_FIELD_TOTAL_LGTH) {
      found = 1;
      break;
    }
    idx++;
  }
  if (! found) {
    return STRUCT_MSG_ERR_NO_SUCH_FIELD;
  } 
  if (self->structmsgDataView->dataView->data != NULL) {
    // free no longer used
    os_free(self->structmsgDataView->dataView->data);
  }
  // temporary replace data entry of dataView by our param data
  // to be able to use the get* functions for gettting totalLgth entry value
  self->structmsgDataView->dataView->data = (uint8_t *)data;
  // get totalLgth value from data
  result = self->structmsgDataView->dataView->getUint16(self->structmsgDataView->dataView, fieldInfo->fieldOffset, &lgth);
  checkErrOK(result);
  if (lgth != self->totalLgth) {
    return STRUCT_MSG_ERR_BAD_DATA_LGTH;
  }
  // now make a copy of the data to be on the safe side
  // for freeing the Lua space in Lua set the variable to nil!!
  self->structmsgDataView->dataView->data = os_zalloc(self->totalLgth);
  checkAllocOK(self->structmsgDataView->dataView->data);
  c_memcpy(self->structmsgDataView->dataView->data, data, self->totalLgth);
  // and now set the IS_SET flags and other stuff
  idx = 0;
  while (idx < self->numFields) {
    fieldInfo = &self->fields[idx];
    fieldInfo->fieldFlags |= STRUCT_MSG_FIELD_IS_SET;
    idx++;
  }
  self->flags |= STRUCT_MSG_IS_PREPARED;
  return STRUCT_MSG_ERR_OK;
}

// ============================= dumpBinary ========================

static void dumpBinary(const uint8_t *data, uint8_t lgth, const uint8_t *where) {
  int idx;

  ets_printf("%s\n", where);
  idx = 0;
  while (idx < lgth) {
     ets_printf("idx: %d ch: 0x%02x\n", idx, data[idx] & 0xFF);
    idx++;
  }
}

// ================================= freeStructmsgData ====================================

static uint8_t freeStructmsgData(structmsgData_t *self) {
  if (self->structmsgDataView != NULL) {
    freeStructmsgDataView(self->structmsgDataView);
  }
  if (self->structmsgDefinitionDataView != NULL) {
    freeStructmsgDataView(self->structmsgDefinitionDataView);
  }
  
  if (self->fields != NULL) {
    os_free(self->fields);
  }
  if (self->tableFields != NULL) {
    os_free(self->tableFields);
  }
  if (self->defFields != NULL) {
    os_free(self->defFields);
  }
  if (self->header != NULL) {
    os_free(self->header);
  }
  os_free(self);
  return STRUCT_MSG_ERR_OK;
}

// ================================= deleteMsg ====================================

uint8_t deleteMsg(structmsgData_t *self) {
  int result;

  result = freeStructmsgData(self); 
  checkErrOK(result);
  return STRUCT_MSG_ERR_OK;
}

// ================================= openFile ====================================

static uint8_t openFile(structmsgData_t *self, const uint8_t *fileName, const uint8_t *fileMode) {
  return self->structmsgDataDescription->openFile(self->structmsgDataDescription, fileName, fileMode);
}

// ================================= closeFile ====================================

static uint8_t closeFile(structmsgData_t *self) {
  return self->structmsgDataDescription->closeFile(self->structmsgDataDescription);
}

// ================================= flushFile ====================================

static uint8_t flushFile(structmsgData_t *self) {
  return STRUCT_DATA_DESC_ERR_FLUSH_FILE;
}

// ================================= readLine ====================================

static uint8_t readLine(structmsgData_t *self, uint8_t **buffer, uint8_t *lgth) {
  return self->structmsgDataDescription->readLine(self->structmsgDataDescription, buffer, lgth);
}

// ================================= writeLine ====================================

static uint8_t writeLine(structmsgData_t *self, const uint8_t *buffer, uint8_t lgth) {
  return self->structmsgDataDescription->writeLine(self->structmsgDataDescription, buffer, lgth);
}

// ================================= uartReceiveCb ====================================

static uint8_t uartReceiveCb(structmsgData_t *self, const uint8_t *buffer, uint8_t lgth) {
ets_printf("§: %d %s§\n", lgth, buffer);
  return STRUCT_MSG_ERR_OK;
}

// ================================= newStructmsgData ====================================

structmsgData_t *newStructmsgData(void) {
  structmsgData_t *structmsgData = os_zalloc(sizeof(structmsgData_t));
  if (structmsgData == NULL) {
    return NULL;
  }
  structmsgData->structmsgDataView = newStructmsgDataView();
  if (structmsgData->structmsgDataView == NULL) {
    return NULL;
  }
  structmsgData->structmsgDefinitionDataView = NULL;
  structmsgData->structmsgDataDescription = newStructmsgDataDescription();
  
  structmsgData->fields = NULL;
  structmsgData->tableFields = NULL;
  structmsgData->defFields = NULL;
  structmsgData->header = NULL;
  structmsgData->flags = 0;
  structmsgData->numFields = 0;
  structmsgData->maxFields = 0;
  structmsgData->numTableRows = 0;
  structmsgData->numTableRowFields = 0;
  structmsgData->numRowFields = 0;
  structmsgData->numDefFields = 0;
  structmsgData->fieldOffset = 0;
  structmsgData->defFieldOffset = 0;
  structmsgData->totalLgth = 0;
  structmsgData->defTotalLgth = 0;
  structmsgData->cmdLgth = 0;
  structmsgData->headerLgth = 0;
  structmsgData->defNumNormFields = 0;
  structmsgData->defNormNamesSize = 0;
  structmsgData->defDefinitionsSize = 0;

  structmsgData->createMsg = &createMsg;
  structmsgData->deleteMsg = &deleteMsg;
  structmsgData->addField = &addField;
  structmsgData->getFieldValue = &getFieldValue;
  structmsgData->setFieldValue = &setFieldValue;
  structmsgData->getTableFieldValue = &getTableFieldValue;
  structmsgData->setTableFieldValue = &setTableFieldValue;
  structmsgData->dumpMsg = &dumpMsg;
  structmsgData->dumpBinary = &dumpBinary;
  structmsgData->initMsg = &initMsg;
  structmsgData->prepareMsg = &prepareMsg;
  structmsgData->getMsgData = &getMsgData;
  structmsgData->setMsgData = &setMsgData;
  structmsgData->setMsgFieldsFromList = &setMsgFieldsFromList;

  structmsgData->structmsgDefinitionDataView = NULL;
  structmsgData->initDef = NULL;
  structmsgData->prepareDef = NULL;
  structmsgData->addDefField = NULL;
  structmsgData->dumpDefFields = NULL;
  structmsgData->setDefFieldValue = NULL;
  structmsgData->getDefFieldValue = NULL;

  structmsgData->openFile = &openFile;
  structmsgData->closeFile = &closeFile;
  structmsgData->readLine = &readLine;
  structmsgData->writeLine = &writeLine;

  structmsgData->uartReceiveCb = &uartReceiveCb;

  return structmsgData;
}





#ifdef NOTDEF

// ============================= getHandle ========================

static int getHandle(uint8_t *hdrkey, uint8_t **handle) {
  int idx;

  *handle = NULL;
  if (structmsg_userdata.handles == NULL) {
    return STRUCT_MSG_ERR_HANDLE_NOT_FOUND;
  } else {
    idx = 0;
    while (idx < structmsg_userdata.numHandles) {
      if (structmsg_userdata.handles[idx].handle != NULL) {
        if (c_memcmp(structmsg_userdata.handles[idx].hdrInfo.hdrId, hdrkey, STRUCT_MSG_TOTAL_HEADER_LENGTH) == 0) {
          *handle = structmsg_userdata.handles[idx].handle;
          return STRUCT_MSG_ERR_OK;
        }
      }
      idx++;
    }
  }
  return STRUCT_MSG_ERR_HANDLE_NOT_FOUND;
}

// ============================= structmsg_encryptdecrypt ========================

int structmsg_encryptdecrypt(const uint8_t *handle, const uint8_t *msg, size_t mlgth, const uint8_t *key, size_t klen, const uint8_t *iv, size_t ivlen, bool enc, uint8_t **buf, int *lgth) {
  structmsg_t *structmsg;
  const uint8_t *message;
  size_t mlen;
  const crypto_mech_t *mech;
  const char *data;
  size_t dlen;
  size_t bs;
  size_t clen;
  const uint8_t *what;
  uint8_t *crypted;

  *buf = NULL;
  *lgth = 0;
  if (handle != NULL) {
    structmsg = structmsg_get_structmsg_ptr(handle);
    checkHandleOK(structmsg);

    mlen = structmsg->hdr.hdrInfo.hdrKeys.totalLgth;
    if (enc) {
      if (structmsg->encoded == NULL) {
        return STRUCT_MSG_ERR_NOT_ENCODED;
      }
      message = structmsg->encoded;
    } else {
      if (structmsg->encrypted == NULL) {
        return STRUCT_MSG_ERR_NOT_ENCRYPTED;
      }
      message = structmsg->encrypted;
    }
  } else {
    message = msg;
    mlen = mlgth;
  }

  mech = crypto_encryption_mech ("AES-CBC");
  if (mech == NULL) {
    return STRUCT_MSG_ERR_CRYPTO_BAD_MECHANISM;
  }
  bs = mech->block_size;
  what = enc ? "encrypt": "decrypt";
//ets_printf("encryptdecrypt bs: %d what: %s enc: %d\n", bs, what, enc);
  dlen = mlen - STRUCT_MSG_HEADER_LENGTH;
  data = message + STRUCT_MSG_HEADER_LENGTH;
  clen = ((dlen + bs - 1) / bs) * bs;
  *lgth = clen + STRUCT_MSG_HEADER_LENGTH;
//ets_printf("dlen: %d lgth: %d clen: %d data: %p\n", dlen, *lgth, clen, data);
  crypted = (uint8_t *)os_zalloc (*lgth);
  if (!crypted) {
    return STRUCT_MSG_ERR_CRYPTO_INIT_FAILED;
  } 
  c_memcpy(crypted, message, STRUCT_MSG_HEADER_LENGTH);
  *buf = crypted;
  crypted += STRUCT_MSG_HEADER_LENGTH;
  crypto_op_t op =
  { 
    key, klen,
    iv, ivlen,
    data, dlen,
    crypted, clen,
    enc ? OP_ENCRYPT : OP_DECRYPT
  }; 
  if (!mech->run (&op)) { 
    os_free (*buf);
    return STRUCT_MSG_ERR_CRYPTO_INIT_FAILED;
  } else { 
    if (enc) {
      if (handle != NULL) {
        structmsg->encrypted = *buf;
      }
    } else {
      if (handle != NULL) {
        structmsg->todecode = *buf;
      }
    }
    return STRUCT_MSG_ERR_OK;
  }
}

// ============================= stmsg_setCrypted ========================

int stmsg_setCrypted(const uint8_t *handle, const uint8_t *crypted, int cryptedLgth) {
  structmsg_t *structmsg;

  structmsg = structmsg_get_structmsg_ptr(handle);
  checkHandleOK(structmsg);
  structmsg->encrypted = (uint8_t *)os_malloc(cryptedLgth);
  checkAllocOK(structmsg->encrypted);
  c_memcpy(structmsg->encrypted, crypted, cryptedLgth);
  return STRUCT_MSG_ERR_OK;
}

// ============================= stmsg_decryptGetHandle ========================

int stmsg_decryptGetHandle(const uint8_t *encryptedMsg, size_t mlen, const uint8_t *key, size_t klen, const uint8_t *iv, size_t ivlen, uint8_t **handle) {
  uint8_t *decrypted;
  size_t lgth;
  int result;

   decrypted = NULL;
   lgth = 0; 
   result = structmsg_encryptdecrypt(NULL, encryptedMsg, mlen, key, klen, iv, ivlen, false, &decrypted, &lgth);
   if (result != STRUCT_MSG_ERR_OK) {
     return result;
   }
   result = getHandle(decrypted, handle);
   return result;
}

#endif
