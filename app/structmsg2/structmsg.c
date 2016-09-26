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
ets_printf("createMsg: numFields: %d self: %p, *handle: %p, fields size: %d fields: %p\n", numFields, self, *handle, sizeof(structmsgField_t) * numFields, self->fields);
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
//  self->handleHdrInfoPtr = NULL;
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

static uint8_t getFieldValue(structmsgData_t *self, const uint8_t *fieldName, int *numericValue, uint8_t *stringValue) {
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
      result = self->structmsgDataView->getFieldValue(self->structmsgDataView, fieldInfo, numericValue, stringValue);
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
  while (idx < numEntries) {
    fieldInfo = &self->fields[idx];
    if (fieldNameId == fieldInfo->fieldNameId) {
      result = self->structmsgDataView->setFieldValue(self->structmsgDataView, fieldInfo, numericValue, stringValue);
      checkErrOK(result);
      fieldInfo->fieldFlags |= STRUCT_MSG_FIELD_IS_SET;
      break;
    }
    idx++;
  }
  return DATA_VIEW_ERR_OK;
}


// ================================= getTableFieldValue ====================================

static uint8_t getTableFieldValue(structmsgData_t *self, const uint8_t *fieldName, int row, int *numericValue, uint8_t *stringValue) {
  return DATA_VIEW_ERR_OK;
}

// ================================= setTableFieldValue ====================================

static uint8_t setTableFieldValue(structmsgData_t *self, const uint8_t *fieldName, int row, int numericValue, const uint8_t *stringValue) {
  return DATA_VIEW_ERR_OK;
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
  numEntries = self->numFields;
  idx = 0;
  numEntries = self->numFields;
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
        result = self->structmsgDataView->setCrc(self->structmsgDataView, fieldInfo, self->headerLgth+1, self->cmdLgth-fieldInfo->fieldLgth);
        checkErrOK(result);
        fieldInfo->fieldFlags |= STRUCT_MSG_FIELD_IS_SET;
        break;
    }
    idx++;
  }
  return STRUCT_MSG_ERR_OK;
}

// ================================= initMsg ====================================

static uint8_t initMsg(structmsgData_t *self) {
  int numEntries;
  int idx;
  int result;
  size_t myLgth;
  size_t fillerLgth;
  size_t crcLgth;
  structmsgField_t *fieldInfo;

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
      case STRUCT_MSG_SPEC_FIELD_GUID:
        self->headerLgth += fieldInfo->fieldLgth;
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
    }
    self->fieldOffset += fieldInfo->fieldLgth;
    idx++;
  }
  if (self->structmsgDataView->dataView->data != NULL) {
    os_free(self->structmsgDataView->dataView->data);
  }
  self->structmsgDataView->dataView->data = os_zalloc(self->totalLgth);
  self->structmsgDataView->dataView->lgth = self->totalLgth;
  self->flags |= STRUCT_MSG_IS_INITTED;
  idx = 0;
  numEntries = self->numFields;
  while (idx < numEntries) {
    fieldInfo = &self->fields[idx];
    switch (fieldInfo->fieldNameId) {
      case STRUCT_MSG_SPEC_FIELD_TOTAL_LGTH:
        result = self->structmsgDataView->setFieldValue(self->structmsgDataView, fieldInfo, (int)self->totalLgth, NULL);
        checkErrOK(result);
        fieldInfo->fieldFlags |= STRUCT_MSG_FIELD_IS_SET;
        break;
      case STRUCT_MSG_SPEC_FIELD_CMD_LGTH:
        result = self->structmsgDataView->setFieldValue(self->structmsgDataView, fieldInfo, (int)self->cmdLgth, NULL);
        checkErrOK(result);
        fieldInfo->fieldFlags |= STRUCT_MSG_FIELD_IS_SET;
        break;
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
        result = self->structmsgDataView->setCrc(self->structmsgDataView, fieldInfo, self->headerLgth+1, self->cmdLgth-fieldInfo->fieldLgth);
        checkErrOK(result);
        fieldInfo->fieldFlags |= STRUCT_MSG_FIELD_IS_SET;
        break;
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
  int numericValue;
  uint8_t uch;
  int8_t ch;
  uint16_t ush;
  int16_t sh;
  uint32_t uval;
  int32_t val;
  uint8_t *fieldTypeStr;
  uint8_t *fieldNameStr;
  structmsgField_t *fieldInfo;

  ets_printf("handle: %s\r\n", self->handle);
  numEntries = self->numFields;
  ets_printf("  numFields: %d maxFields: %d\r\n", numEntries, (int)self->maxFields);
  ets_printf("  headerLgth: %d cmdLgth: %d totalLgth: %d\r\n", self->headerLgth, self->cmdLgth, self->totalLgth);
  ets_printf("  flags:");
  if ((self->flags & STRUCT_MSG_ENCODED) != 0) {
    ets_printf(" STRUCT_MSG_ENCODED");
  }
  if ((self->flags &  STRUCT_MSG_DECODED) != 0) {
    ets_printf(" STRUCT_MSG_DECODED");
  }
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
//      dumpTableRowFields(structmsg);
      idx++;
      continue;
    }
    ets_printf("    idx %d: fieldName: %-20s fieldType: %-8s fieldLgth: %.5d\r\n", idx, fieldNameStr, fieldTypeStr, fieldInfo->fieldLgth);
    if (fieldInfo->fieldFlags & STRUCT_MSG_FIELD_IS_SET) {
      uint8_t stringValue[255];
      uint8_t *stringValuePtr = stringValue;
      numericValue = 0;
      result = self->structmsgDataView->getFieldValue(self->structmsgDataView, fieldInfo, &numericValue, stringValue);
      switch (fieldInfo->fieldTypeId) {
      case DATA_VIEW_FIELD_INT8_T:
        ets_printf("      value: 0x%02x %d\n", numericValue & 0xFF, numericValue & 0xFF);
        break;
      case DATA_VIEW_FIELD_UINT8_T:
        ets_printf("      value: 0x%02x %d\n", numericValue & 0xFF, numericValue & 0xFF);
        break;
      case DATA_VIEW_FIELD_INT16_T:
        ets_printf("      value: 0x%04x %d\n", numericValue & 0xFFFF, numericValue & 0xFFFF);
        break;
      case DATA_VIEW_FIELD_UINT16_T:
        ets_printf("      value: 0x%04x %d\n", numericValue & 0xFFFF, numericValue & 0xFFFF);
        break;
      case DATA_VIEW_FIELD_INT32_T:
        ets_printf("      value: 0x%08x %d\n", numericValue & 0xFFFFFFFF, numericValue & 0xFFFFFFFF);
        break;
      case DATA_VIEW_FIELD_UINT32_T:
        ets_printf("      value: 0x%08x %d\n", numericValue & 0xFFFFFFFF, numericValue & 0xFFFFFFFF);
        break;
      case DATA_VIEW_FIELD_INT8_VECTOR:
        valueIdx = 0;
        ets_printf("      values:");
        while (valueIdx < fieldInfo->fieldLgth) {
          ch = stringValue[valueIdx];
          ets_printf("        idx: %d value: %c 0x%02x\n", valueIdx, (char)ch, (uint8_t)(ch & 0xFF));
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
#ifdef NOTDEF
      case DATA_VIEW_FIELD_INT16_VECTOR:
        valueIdx = 0;
        ets_printf("      values:");
        while (valueIdx < fieldInfo->fieldLgth) {
          sh = fieldInfo->value.shortVector[valueIdx];
          ets_printf("        idx: %d value: 0x%04x\n", valueIdx, (int16_t)(sh & 0xFFFF));
          valueIdx++;
        }
        ets_printf("\n");
        break;
      case DATA_VIEW_FIELD_UINT16_VECTOR:
        valueIdx = 0;
        ets_printf("      values:\n");
        while (valueIdx < fieldInfo->fieldLgth) {
          ush = fieldInfo->value.ushortVector[valueIdx];
          ets_printf("        idx: %d value: 0x%04x\n", valueIdx, (uint16_t)(ush & 0xFFFF));
          valueIdx++;
        }
        break;
      case DATA_VIEW_FIELD_INT32_VECTOR:
        valueIdx = 0;
        ets_printf("      values:");
        while (valueIdx < fieldInfo->fieldLgth) {
          val = fieldInfo->value.int32Vector[valueIdx];
          ets_printf("        idx: %d value: 0x%08x\n", valueIdx, (int32_t)(val & 0xFFFFFFFF));
          valueIdx++;
        }
        ets_printf("\n");
        break;
      case DATA_VIEW_FIELD_UINT32_VECTOR:
        valueIdx = 0;
        ets_printf("      values:\n");
        while (valueIdx < fieldInfo->fieldLgth) {
          uval = fieldInfo->value.uint32Vector[valueIdx];
          ets_printf("        idx: %d value: 0x%08x\n", valueIdx, (uint32_t)(uval & 0xFFFFFFFF));
          valueIdx++;
        }
        break;
#endif
      }
    }
    idx++;
  }
ets_printf("dumpMsg done\n");
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

  // structmsgData->handle[16];
  structmsgData->fields = NULL;
  structmsgData->tableFields = NULL;
  structmsgData->flags = 0;
  structmsgData->numFields = 0;
  structmsgData->maxFields = 0;
  structmsgData->numTableRows = 0;
  structmsgData->numTableRowFields = 0;
  structmsgData->numRowFields = 0;
  structmsgData->fieldOffset = 0;
  structmsgData->totalLgth = 0;
  structmsgData->cmdLgth = 0;
  structmsgData->headerLgth = 0;
  structmsgData->header = NULL;

  structmsgData->createMsg = &createMsg;
  structmsgData->addField = &addField;
  structmsgData->getFieldValue = &getFieldValue;
  structmsgData->setFieldValue = &setFieldValue;
  structmsgData->getTableFieldValue = &getTableFieldValue;
  structmsgData->setTableFieldValue = &setTableFieldValue;
  structmsgData->dumpMsg = &dumpMsg;
  structmsgData->dumpBinary = &dumpBinary;
  structmsgData->initMsg = &initMsg;
  structmsgData->prepareMsg = &prepareMsg;

ets_printf("structmsgData: %p size: %d\n", structmsgData, sizeof(structmsgData_t));
  return structmsgData;
}

// ================================= freeStructmsgData ====================================

void freeStructmsgData(structmsgData_t *structmsgdata) {
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

// ============================= setHandleField ========================

static int setHandleField(const uint8_t *handle, int fieldId, int fieldValue) {
  int idx;
  int result = STRUCT_MSG_ERR_OK;

  if (structmsg_userdata.handles == NULL) {
    return STRUCT_MSG_ERR_HANDLE_NOT_FOUND;
  }
  idx = 0;
  while (idx < structmsg_userdata.numHandles) {
    if ((structmsg_userdata.handles[idx].handle != NULL) && (c_strcmp(structmsg_userdata.handles[idx].handle, handle) == 0)) {
      switch (fieldId) {
      case STRUCT_MSG_FIELD_SRC:
        structmsg_userdata.handles[idx].hdrInfo.hdrKeys.src = (uint16_t)fieldValue;
        break;
      case STRUCT_MSG_FIELD_DST:
        structmsg_userdata.handles[idx].hdrInfo.hdrKeys.dst = (uint16_t)fieldValue;
        break;
      case STRUCT_MSG_FIELD_TOTAL_LGTH:
        structmsg_userdata.handles[idx].hdrInfo.hdrKeys.totalLgth = (uint16_t)fieldValue;
        break;
      case STRUCT_MSG_FIELD_CMD_KEY:
        structmsg_userdata.handles[idx].hdrInfo.hdrKeys.cmdKey = (uint16_t)fieldValue;
        break;
      case STRUCT_MSG_FIELD_CMD_LGTH:
        structmsg_userdata.handles[idx].hdrInfo.hdrKeys.cmdLgth = (uint16_t)fieldValue;
        break;
      }
      return result;
    }
    idx++;
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

// ============================= fixHeaderInfo ========================

static int fixHeaderInfo(structmsg_t *structmsg, fieldInfo_t *fieldInfo, const uint8_t *fieldStr, uint8_t fieldType, uint8_t fieldLgth, uint8_t numTableRows) {
  fieldInfo->fieldStr = os_malloc(os_strlen(fieldStr) + 1);
  fieldInfo->fieldStr[os_strlen(fieldStr)] = '\0';
  os_memcpy(fieldInfo->fieldStr, fieldStr, os_strlen(fieldStr));
  fieldInfo->fieldType = fieldType;
  fieldInfo->value.byteVector = NULL;
  fieldInfo->flags = 0;
//ets_printf("fixHeaderInfo1: fld: %s cmdLgth: %d totalLgth: %d\n", fieldStr, structmsg->hdr.hdrInfo.hdrKeys.cmdLgth, structmsg->hdr.hdrInfo.hdrKeys.totalLgth);
  switch (fieldType) {
    case STRUCT_MSG_FIELD_UINT8_T:
    case STRUCT_MSG_FIELD_INT8_T:
      structmsg->hdr.hdrInfo.hdrKeys.totalLgth += 1 * numTableRows;
      structmsg->hdr.hdrInfo.hdrKeys.cmdLgth += 1 * numTableRows;
      fieldLgth = 1;
      break;
    case STRUCT_MSG_FIELD_UINT16_T:
    case STRUCT_MSG_FIELD_INT16_T:
      structmsg->hdr.hdrInfo.hdrKeys.totalLgth += 2 * numTableRows;
      structmsg->hdr.hdrInfo.hdrKeys.cmdLgth += 2 * numTableRows;
      fieldLgth = 2;
      break;
    case STRUCT_MSG_FIELD_UINT32_T:
    case STRUCT_MSG_FIELD_INT32_T:
      structmsg->hdr.hdrInfo.hdrKeys.totalLgth += 4 * numTableRows;
      structmsg->hdr.hdrInfo.hdrKeys.cmdLgth += 4 * numTableRows;
      fieldLgth = 4;
      break;
    case STRUCT_MSG_FIELD_UINT8_VECTOR:
      structmsg->hdr.hdrInfo.hdrKeys.totalLgth += fieldLgth * numTableRows;
      structmsg->hdr.hdrInfo.hdrKeys.cmdLgth += fieldLgth * numTableRows;
      fieldInfo->value.ubyteVector = (uint8_t *)os_malloc(fieldLgth + 1);
      fieldInfo->value.ubyteVector[fieldLgth] = '\0';
      break;
    case STRUCT_MSG_FIELD_INT8_VECTOR:
      structmsg->hdr.hdrInfo.hdrKeys.totalLgth += fieldLgth * numTableRows;
      structmsg->hdr.hdrInfo.hdrKeys.cmdLgth += fieldLgth * numTableRows;
      fieldInfo->value.byteVector = (int8_t *)os_malloc(fieldLgth + 1);
      fieldInfo->value.ubyteVector[fieldLgth] = '\0';
      break;
    case STRUCT_MSG_FIELD_UINT16_VECTOR:
      structmsg->hdr.hdrInfo.hdrKeys.totalLgth += fieldLgth * numTableRows;
      structmsg->hdr.hdrInfo.hdrKeys.cmdLgth += fieldLgth * numTableRows;
      fieldInfo->value.ushortVector = (uint16_t *)os_malloc(fieldLgth*sizeof(uint16_t));
      break;
    case STRUCT_MSG_FIELD_INT16_VECTOR:
      structmsg->hdr.hdrInfo.hdrKeys.totalLgth += fieldLgth * numTableRows;
      structmsg->hdr.hdrInfo.hdrKeys.cmdLgth += fieldLgth * numTableRows;
      fieldInfo->value.shortVector = (int16_t *)os_malloc(fieldLgth*sizeof(int16_t));
      break;
    case STRUCT_MSG_FIELD_UINT32_VECTOR:
      structmsg->hdr.hdrInfo.hdrKeys.totalLgth += fieldLgth * numTableRows;
      structmsg->hdr.hdrInfo.hdrKeys.cmdLgth += fieldLgth * numTableRows;
      fieldInfo->value.uint32Vector = (uint32_t *)os_malloc(fieldLgth*sizeof(uint32_t));
      break;
    case STRUCT_MSG_FIELD_INT32_VECTOR:
      structmsg->hdr.hdrInfo.hdrKeys.totalLgth += fieldLgth * numTableRows;
      structmsg->hdr.hdrInfo.hdrKeys.cmdLgth += fieldLgth * numTableRows;
      fieldInfo->value.int32Vector = (int32_t *)os_malloc(fieldLgth*sizeof(int32_t));
      break;
  }
  setHandleField(structmsg->handle, STRUCT_MSG_FIELD_CMD_LGTH, structmsg->hdr.hdrInfo.hdrKeys.cmdLgth);
  setHandleField(structmsg->handle, STRUCT_MSG_FIELD_TOTAL_LGTH, structmsg->hdr.hdrInfo.hdrKeys.totalLgth);
  fieldInfo->fieldLgth = fieldLgth;
//ets_printf("fixHeaderInfo2: fld: %s cmdLgth: %d totalLgth: %d\n", fieldStr, structmsg->hdr.hdrInfo.hdrKeys.cmdLgth, structmsg->hdr.hdrInfo.hdrKeys.totalLgth);
  return STRUCT_MSG_ERR_OK;
}

// ============================= setFieldValue ========================

static int setFieldValue(structmsg_t *structmsg, fieldInfo_t *fieldInfo, const uint8_t *fieldName, int numericValue, const uint8_t *stringValue) {

  switch (fieldInfo->fieldType) {
    case STRUCT_MSG_FIELD_INT8_T:
      if (stringValue == NULL) {
        if ((numericValue > -128) && (numericValue < 128)) {
          fieldInfo->value.byteVal = (int8_t)numericValue;
        } else {
          return STRUCT_MSG_ERR_VALUE_TOO_BIG;
        }
      } else {
        return STRUCT_MSG_ERR_BAD_VALUE;
      }
      break;
    case STRUCT_MSG_FIELD_UINT8_T:
      if (stringValue == NULL) {
        if ((numericValue >= 0) && (numericValue <= 256)) {
          fieldInfo->value.ubyteVal = (uint8_t)numericValue;
        }
      } else {
        return STRUCT_MSG_ERR_BAD_VALUE;
      }
      break;
    case STRUCT_MSG_FIELD_INT16_T:
      if (stringValue == NULL) {
        if ((numericValue > -32767) && (numericValue < 32767)) {
          fieldInfo->value.shortVal = (int16_t)numericValue;
        } else {
          return STRUCT_MSG_ERR_VALUE_TOO_BIG;
        }
      } else {
        return STRUCT_MSG_ERR_BAD_VALUE;
      }
      break;
    case STRUCT_MSG_FIELD_UINT16_T:
      if (stringValue == NULL) {
        if ((numericValue >= 0) && (numericValue <= 65535)) {
          fieldInfo->value.ushortVal = (uint16_t)numericValue;
        } else {
          return STRUCT_MSG_ERR_VALUE_TOO_BIG;
        }
      } else {
        return STRUCT_MSG_ERR_BAD_VALUE;
      }
      break;
    case STRUCT_MSG_FIELD_INT32_T:
      if (stringValue == NULL) {
        if ((numericValue > -0x7FFFFFFF) && (numericValue <= 0x7FFFFFFF)) {
          fieldInfo->value.val = (int32_t)numericValue;
        } else {
          return STRUCT_MSG_ERR_VALUE_TOO_BIG;
        }
      } else {
        return STRUCT_MSG_ERR_BAD_VALUE;
      }
      break;
    case STRUCT_MSG_FIELD_UINT32_T:
      if (stringValue == NULL) {
        // we have to do the signed check as numericValue is a sigend integer!!
        if ((numericValue > -0x7FFFFFFF) && (numericValue <= 0x7FFFFFFF)) {
          fieldInfo->value.uval = (uint32_t)numericValue;
        } else {
          return STRUCT_MSG_ERR_VALUE_TOO_BIG;
        }
      } else {
        return STRUCT_MSG_ERR_BAD_VALUE;
      }
      break;
    case STRUCT_MSG_FIELD_INT8_VECTOR:
      if (stringValue != NULL) {
        // check for length needed!!
        os_memcpy(fieldInfo->value.ubyteVector, stringValue, fieldInfo->fieldLgth);
      } else {
        return STRUCT_MSG_ERR_BAD_VALUE;
      }
      break;
    case STRUCT_MSG_FIELD_UINT8_VECTOR:
      if (stringValue != NULL) {
        // check for length needed!!
        os_memcpy(fieldInfo->value.byteVector, stringValue, fieldInfo->fieldLgth);
      } else {
        return STRUCT_MSG_ERR_BAD_VALUE;
      }
      break;
    case STRUCT_MSG_FIELD_INT16_VECTOR:
      if (stringValue != NULL) {
        // check for length needed!!
        os_memcpy((int8_t *)fieldInfo->value.shortVector, stringValue, fieldInfo->fieldLgth);
      } else {
        return STRUCT_MSG_ERR_BAD_VALUE;
      }
      break;
    case STRUCT_MSG_FIELD_UINT16_VECTOR:
      if (stringValue != NULL) {
        // check for length needed!!
        os_memcpy((uint8_t *)fieldInfo->value.ushortVector, stringValue, fieldInfo->fieldLgth);
      } else {
        return STRUCT_MSG_ERR_BAD_VALUE;
      }
      break;
    case STRUCT_MSG_FIELD_INT32_VECTOR:
      if (stringValue != NULL) {
        // check for length needed!!
        os_memcpy((int8_t *)fieldInfo->value.int32Vector, stringValue, fieldInfo->fieldLgth);
      } else {
        return STRUCT_MSG_ERR_BAD_VALUE;
      }
      break;
    case STRUCT_MSG_FIELD_UINT32_VECTOR:
      if (stringValue != NULL) {
        // check for length needed!!
        os_memcpy((uint8_t *)fieldInfo->value.uint32Vector, stringValue, fieldInfo->fieldLgth);
      } else {
        return STRUCT_MSG_ERR_BAD_VALUE;
      }
      break;
    default:
      return STRUCT_MSG_ERR_BAD_FIELD_TYPE;
      break;
  }
  fieldInfo->flags |= STRUCT_MSG_FIELD_IS_SET;
  return STRUCT_MSG_ERR_OK;
}


// ============================= getFieldValue ========================

static int getFieldValue(structmsg_t *structmsg, fieldInfo_t *fieldInfo, const uint8_t *fieldName, int *numericValue, uint8_t **stringValue) {

  switch (fieldInfo->fieldType) {
    case STRUCT_MSG_FIELD_INT8_T:
      if (fieldInfo->flags & STRUCT_MSG_FIELD_IS_SET) {
        *numericValue = (int)fieldInfo->value.byteVal;
        return STRUCT_MSG_ERR_OK;
      }
      break;
    case STRUCT_MSG_FIELD_UINT8_T:
      if (fieldInfo->flags & STRUCT_MSG_FIELD_IS_SET) {
        *numericValue = (int)fieldInfo->value.ubyteVal;
        return STRUCT_MSG_ERR_OK;
      }
      break;
    case STRUCT_MSG_FIELD_INT16_T:
      if (fieldInfo->flags & STRUCT_MSG_FIELD_IS_SET) {
        *numericValue = (int)fieldInfo->value.shortVal;
        return STRUCT_MSG_ERR_OK;
      }
      break;
    case STRUCT_MSG_FIELD_UINT16_T:
      if (fieldInfo->flags & STRUCT_MSG_FIELD_IS_SET) {
        *numericValue = (int)fieldInfo->value.ushortVal;
        return STRUCT_MSG_ERR_OK;
      }
      break;
    case STRUCT_MSG_FIELD_INT32_T:
      if (fieldInfo->flags & STRUCT_MSG_FIELD_IS_SET) {
        *numericValue = (int)fieldInfo->value.val;
        return STRUCT_MSG_ERR_OK;
      }
      break;
    case STRUCT_MSG_FIELD_UINT32_T:
      if (fieldInfo->flags & STRUCT_MSG_FIELD_IS_SET) {
        *numericValue = (int)fieldInfo->value.uval;
        return STRUCT_MSG_ERR_OK;
      }
      break;
    case STRUCT_MSG_FIELD_INT8_VECTOR:
      if (fieldInfo->flags & STRUCT_MSG_FIELD_IS_SET) {
        *stringValue = fieldInfo->value.byteVector;
        return STRUCT_MSG_ERR_OK;
      }
      break;
    case STRUCT_MSG_FIELD_UINT8_VECTOR:
      if (fieldInfo->flags & STRUCT_MSG_FIELD_IS_SET) {
        *stringValue = fieldInfo->value.ubyteVector;
        return STRUCT_MSG_ERR_OK;
      }
      break;
    case STRUCT_MSG_FIELD_INT16_VECTOR:
      if (fieldInfo->flags & STRUCT_MSG_FIELD_IS_SET) {
        *stringValue = (uint8_t *)fieldInfo->value.shortVector;
        return STRUCT_MSG_ERR_OK;
      }
      break;
    case STRUCT_MSG_FIELD_UINT16_VECTOR:
      if (fieldInfo->flags & STRUCT_MSG_FIELD_IS_SET) {
        *stringValue = (uint8_t *)fieldInfo->value.ushortVector;
        return STRUCT_MSG_ERR_OK;
      }
      break;
    case STRUCT_MSG_FIELD_INT32_VECTOR:
      if (fieldInfo->flags & STRUCT_MSG_FIELD_IS_SET) {
        *stringValue = (uint8_t *)fieldInfo->value.int32Vector;
        return STRUCT_MSG_ERR_OK;
      }
      break;
    case STRUCT_MSG_FIELD_UINT32_VECTOR:
      if (fieldInfo->flags & STRUCT_MSG_FIELD_IS_SET) {
        *stringValue = (uint8_t *)fieldInfo->value.uint32Vector;
        return STRUCT_MSG_ERR_OK;
      }
      break;
    default:
      return STRUCT_MSG_ERR_BAD_FIELD_TYPE;
      break;
  }
  return STRUCT_MSG_ERR_VALUE_NOT_SET;
}

// ============================= stmsg_deleteMsg ========================

int stmsg_deleteMsg(const uint8_t *handle) {
  structmsg_t *structmsg;
  int idx;

  structmsg = structmsg_get_structmsg_ptr(handle);
  checkHandleOK(structmsg);
  idx = 0;
  while (idx < structmsg->msg.numFieldInfos) {
    fieldInfo_t *fieldInfo = &structmsg->msg.fieldInfos[idx];
    os_free(fieldInfo->fieldStr);
    switch (fieldInfo->fieldType) {
    case STRUCT_MSG_FIELD_INT8_VECTOR:
      if (fieldInfo->value.byteVector != NULL) {
        os_free(fieldInfo->value.byteVector);
      }
      break;
    case STRUCT_MSG_FIELD_UINT8_VECTOR:
      if (fieldInfo->value.byteVector != NULL) {
        os_free(fieldInfo->value.ubyteVector);
      }
      break;
    case STRUCT_MSG_FIELD_INT16_VECTOR:
      if (fieldInfo->value.shortVector != NULL) {
        os_free(fieldInfo->value.shortVector);
      }
      break;
    case STRUCT_MSG_FIELD_UINT16_VECTOR:
      if (fieldInfo->value.ushortVector != NULL) {
        os_free(fieldInfo->value.ushortVector);
      }
      break;
    case STRUCT_MSG_FIELD_INT32_VECTOR:
      if (fieldInfo->value.int32Vector != NULL) {
        os_free(fieldInfo->value.int32Vector);
      }
      break;
    case STRUCT_MSG_FIELD_UINT32_VECTOR:
      if (fieldInfo->value.uint32Vector != NULL) {
        os_free(fieldInfo->value.uint32Vector);
      }
      break;
    }
    idx++;
  }
  os_free(structmsg->msg.fieldInfos);
  if (structmsg->encoded != NULL) {
    os_free(structmsg->encoded);
  }
  if (structmsg->todecode != NULL) {
    os_free(structmsg->todecode);
  }
  if (structmsg->encrypted != NULL) {
    os_free(structmsg->encrypted);
  }
  deleteHandle(handle);
  os_free(structmsg);
  return STRUCT_MSG_ERR_OK;
}

// ============================= dumpTableRowFields ========================

static int dumpTableRowFields(structmsg_t *structmsg) {
  int numEntries;
  int idx;
  int valueIdx;
  int result;
  uint8_t uch;
  int8_t ch;
  uint16_t ush;
  int16_t sh;
  uint32_t uval;
  int32_t val;
  int row;
  int col;
  uint8_t *fieldType;

  numEntries = structmsg->msg.numTableRows * structmsg->msg.numRowFields;
  ets_printf("    numTableFieldInfos: %d\r\n", numEntries);
  idx = 0;
  row = 0;
  col = 0;
  while (idx < numEntries) {
    fieldInfo_t *fieldInfo = &structmsg->msg.tableFieldInfos[idx];
    result = structmsg_getFieldTypeStr(fieldInfo->fieldType, &fieldType);
    checkErrOK(result);
    ets_printf("      row %d: col: %d key: %-20s type: %-8s lgth: %.5d\r\n", row, col, fieldInfo->fieldStr, fieldType, fieldInfo->fieldLgth);
//ets_printf("isSet: %s 0x%02x %d\n", fieldInfo->fieldStr, fieldInfo->flags, (fieldInfo->flags & STRUCT_MSG_FIELD_IS_SET));
    if (fieldInfo->flags & STRUCT_MSG_FIELD_IS_SET) {
      switch (fieldInfo->fieldType) {
      case STRUCT_MSG_FIELD_INT8_T:
        ets_printf("        value: %d 0x%02x\n", (int8_t)fieldInfo->value.byteVal, ((int8_t)fieldInfo->value.byteVal) & 0xFF);
        break;
      case STRUCT_MSG_FIELD_UINT8_T:
        ets_printf("        value: 0x%02x\n", (uint8_t)fieldInfo->value.ubyteVal);
        break;
      case STRUCT_MSG_FIELD_INT16_T:
        ets_printf("        value: 0x%04x\n", (int16_t)fieldInfo->value.shortVal);
        break;
      case STRUCT_MSG_FIELD_UINT16_T:
        ets_printf("        value: 0x%04x\n", (uint16_t)fieldInfo->value.ushortVal);
        break;
      case STRUCT_MSG_FIELD_INT32_T:
        ets_printf("        value: 0x%08x\n", (int32_t)fieldInfo->value.val);
        break;
      case STRUCT_MSG_FIELD_UINT32_T:
        ets_printf("        value: 0x%08x\n", (uint32_t)fieldInfo->value.uval);
        break;
      case STRUCT_MSG_FIELD_INT8_VECTOR:
        valueIdx = 0;
        ets_printf("        values:");
        while (valueIdx < fieldInfo->fieldLgth) {
          ch = fieldInfo->value.byteVector[valueIdx];
          ets_printf("          idx: %d value: %c 0x%02x\n", valueIdx, (char)ch, (uint8_t)(ch & 0xFF));
          valueIdx++;
        }
        ets_printf("\n");
        break;
      case STRUCT_MSG_FIELD_UINT8_VECTOR:
        valueIdx = 0;
        ets_printf("        values:\n");
        while (valueIdx < fieldInfo->fieldLgth) {
          uch = fieldInfo->value.ubyteVector[valueIdx];
          ets_printf("          idx: %d value: %c 0x%02x\n", valueIdx, (char)uch, (uint8_t)(uch & 0xFF));
          valueIdx++;
        }
        break;
      case STRUCT_MSG_FIELD_INT16_VECTOR:
        valueIdx = 0;
        ets_printf("        values:");
        while (valueIdx < fieldInfo->fieldLgth) {
          sh = fieldInfo->value.shortVector[valueIdx];
          ets_printf("          idx: %d value: 0x%04x\n", valueIdx, (int16_t)(sh & 0xFFFF));
          valueIdx++;
        }
        ets_printf("\n");
        break;
      case STRUCT_MSG_FIELD_UINT16_VECTOR:
        valueIdx = 0;
        ets_printf("        values:\n");
        while (valueIdx < fieldInfo->fieldLgth) {
          ush = fieldInfo->value.ushortVector[valueIdx];
          ets_printf("          idx: %d value: 0x%04x\n", valueIdx, (uint16_t)(ush & 0xFFFF));
          valueIdx++;
        }
        break;
      case STRUCT_MSG_FIELD_INT32_VECTOR:
        valueIdx = 0;
        ets_printf("        values:");
        while (valueIdx < fieldInfo->fieldLgth) {
          val = fieldInfo->value.int32Vector[valueIdx];
          ets_printf("          idx: %d value: 0x%08x\n", valueIdx, (int32_t)(val & 0xFFFFFFFF));
          valueIdx++;
        }
        ets_printf("\n");
        break;
      case STRUCT_MSG_FIELD_UINT32_VECTOR:
        valueIdx = 0;
        ets_printf("        values:\n");
        while (valueIdx < fieldInfo->fieldLgth) {
          uval = fieldInfo->value.uint32Vector[valueIdx];
          ets_printf("          idx: %d value: 0x%08x\n", valueIdx, (uint32_t)(uval & 0xFFFFFFFF));
          valueIdx++;
        }
        break;
      }
    }
    col++;
    if (col == structmsg->msg.numRowFields) {
      row++;
      col = 0;
    }
    idx++;
  }
  return STRUCT_MSG_ERR_OK;
}

// ============================= stmsg_addField ========================

int stmsg_addField(const uint8_t *handle, const uint8_t *fieldStr, const uint8_t *fieldTypeStr, int fieldLgth) {
  uint8_t numTableFields;
  uint8_t numTableRowFields;
  uint8_t numTableRows;
  uint8_t fieldType;
  int row;
  int cellIdx;
  int result = STRUCT_MSG_ERR_OK;
  structmsg_t *structmsg;
  fieldInfo_t *fieldInfo;

  result = structmsg_getFieldTypeId(fieldTypeStr, &fieldType);
  checkErrOK(result);
  structmsg = structmsg_get_structmsg_ptr(handle);

//ets_printf("addfield: %s totalLgth: %d\n", fieldStr, structmsg->hdr.hdrInfo.hdrKeys.totalLgth);
  checkHandleOK(structmsg);
  if (c_strcmp(fieldStr, "@filler") == 0) {
    fieldInfo_t *fieldInfo = &structmsg->msg.fieldInfos[structmsg->msg.numFieldInfos];
//ets_printf("filler1: totalLgth: %d cmdLgth: %d\n", structmsg->hdr.hdrInfo.hdrKeys.totalLgth, structmsg->hdr.hdrInfo.hdrKeys.cmdLgth);
    // we use 0 as numTableRows, that forces the *Lgth fields to NOT be modified!!
    fixHeaderInfo(structmsg, fieldInfo, fieldStr, fieldType, 0, 0);
//ets_printf("filler2: totalLgth: %d cmdLgth: %d\n", structmsg->hdr.hdrInfo.hdrKeys.totalLgth, structmsg->hdr.hdrInfo.hdrKeys.cmdLgth);
    structmsg->msg.numFieldInfos++;
    structmsg->flags |= STRUCT_MSG_HAS_FILLER;
    return STRUCT_MSG_ERR_OK;
  }
  if (c_strcmp(fieldStr, "@tablerows") == 0) {
    structmsg->msg.numTableRows = fieldLgth;
//ets_printf("tablerows1: lgth: %d\n",  fieldLgth);
    fieldInfo_t *fieldInfo = &structmsg->msg.fieldInfos[structmsg->msg.numFieldInfos];
//ets_printf("tablerows1: totalLgth: %d cmdLgth: %d\n", structmsg->hdr.hdrInfo.hdrKeys.totalLgth, structmsg->hdr.hdrInfo.hdrKeys.cmdLgth);
    // we use 0 as numTableRows, that forces the *Lgth fields to NOT be modified!!
    fixHeaderInfo(structmsg, fieldInfo, fieldStr, fieldType, 0, 0);
//ets_printf("tablerows2: totalLgth: %d cmdLgth: %d\n", structmsg->hdr.hdrInfo.hdrKeys.totalLgth, structmsg->hdr.hdrInfo.hdrKeys.cmdLgth);
    structmsg->msg.numFieldInfos++;
    return STRUCT_MSG_ERR_OK;
  }
  if (c_strcmp(fieldStr, "@tablerowfields") == 0) {
    structmsg->msg.numTableRowFields = fieldLgth;
    numTableFields = structmsg->msg.numTableRows * structmsg->msg.numTableRowFields;
    fieldInfo_t *fieldInfo = &structmsg->msg.fieldInfos[structmsg->msg.numFieldInfos];
//ets_printf("tablerowFields1: %d lgth: %d\n", numTableFields, fieldLgth);
//ets_printf("tablerowfields1: totalLgth: %d cmdLgth: %d\n", structmsg->hdr.hdrInfo.hdrKeys.totalLgth, structmsg->hdr.hdrInfo.hdrKeys.cmdLgth);
    // we use 0 as numTableRows, that forces the *Lgth fields to NOT be modified!!
    fixHeaderInfo(structmsg, fieldInfo, fieldStr, fieldType, 0, 0);
//ets_printf("tablerowfields2: totalLgth: %d cmdLgth: %d\n", structmsg->hdr.hdrInfo.hdrKeys.totalLgth, structmsg->hdr.hdrInfo.hdrKeys.cmdLgth);
    if ((structmsg->msg.tableFieldInfos == NULL) && (numTableFields != 0)) {
      structmsg->msg.tableFieldInfos = newFieldInfos(numTableFields);
    }
    structmsg->msg.numFieldInfos++;
    return STRUCT_MSG_ERR_OK;
  }
  numTableRowFields = structmsg->msg.numTableRowFields;
  numTableRows = structmsg->msg.numTableRows;
  numTableFields = numTableRows * numTableRowFields;
  if (!((numTableFields > 0) && (structmsg->msg.numRowFields < numTableRowFields))) {
    if (structmsg->msg.numFieldInfos >= structmsg->msg.maxFieldInfos) {
      return STRUCT_MSG_ERR_TOO_MANY_FIELDS;
    }
    fieldInfo_t *fieldInfo = &structmsg->msg.fieldInfos[structmsg->msg.numFieldInfos];
    numTableFields = 0;
    numTableRows = 1;
    numTableRowFields = 0;
    fixHeaderInfo(structmsg, fieldInfo, fieldStr, fieldType, fieldLgth, numTableRows);

    if (c_strcmp(fieldStr, "@crc") == 0) {
      structmsg->flags |= STRUCT_MSG_HAS_CRC;
      if (c_strcmp(fieldTypeStr, "uint8_t") == 0) {
        structmsg->flags |= STRUCT_MSG_UINT8_CRC;
      }
//ets_printf("flags: 0x%02x HAS_CRC: 0x%02x HAS_FILLER: 0x%02x UINT8_CRC: 0x%02x\n", structmsg->flags, STRUCT_MSG_HAS_CRC, STRUCT_MSG_HAS_FILLER, STRUCT_MSG_UINT8_CRC);
    }
//ets_printf("field2: %s totalLgth: %d cmdLgth: %d\n", fieldInfo->fieldStr, structmsg->hdr.hdrInfo.hdrKeys.totalLgth, structmsg->hdr.hdrInfo.hdrKeys.cmdLgth);
    result = structmsg_fillHdrInfo(handle, structmsg);
    structmsg->msg.numFieldInfos++;
  } else {
    row = 0;
    while (row < numTableRows) {
      cellIdx = structmsg->msg.numRowFields + row * numTableRowFields;;
      fieldInfo_t *fieldInfo = &structmsg->msg.tableFieldInfos[cellIdx];
//ets_printf("table field1: %s totalLgth: %d cmdLgth: %d\n", fieldInfo->fieldStr, structmsg->hdr.hdrInfo.hdrKeys.totalLgth, structmsg->hdr.hdrInfo.hdrKeys.cmdLgth);
      fixHeaderInfo(structmsg, fieldInfo, fieldStr, fieldType, fieldLgth, 1);
//ets_printf("table field2: %s totalLgth: %d cmdLgth: %d\n", fieldInfo->fieldStr, structmsg->hdr.hdrInfo.hdrKeys.totalLgth, structmsg->hdr.hdrInfo.hdrKeys.cmdLgth);
      row++;
    }
    structmsg->msg.numRowFields++;  
  } 
  return result;
}

// ============================= stmsg_setFieldValue ========================

int stmsg_setFieldValue(const uint8_t *handle, const uint8_t *fieldName, int numericValue, const uint8_t *stringValue) {
  structmsg_t *structmsg;
  fieldInfo_t *fieldInfo;
  int idx;
  int result;
  int numEntries;

  structmsg = structmsg_get_structmsg_ptr(handle);
checkHandleOK(structmsg);
  if (c_strcmp(fieldName, "@src") == 0) {
    if (stringValue == NULL) {
      if ((numericValue >= 0) && (numericValue <= 65535)) {
        structmsg->hdr.hdrInfo.hdrKeys.src = (uint16_t)numericValue;
        setHandleField(handle, STRUCT_MSG_FIELD_SRC, structmsg->hdr.hdrInfo.hdrKeys.src);
        result = structmsg_fillHdrInfo(handle, structmsg);
        return result;
      } else {
        return STRUCT_MSG_ERR_VALUE_TOO_BIG;
      }
    } else {
      return STRUCT_MSG_ERR_BAD_VALUE;
    }
  } else {
    if (c_strcmp(fieldName, "@dst") == 0) {
      if (stringValue == NULL) {
        if ((numericValue >= 0) && (numericValue <= 65535)) {
          structmsg->hdr.hdrInfo.hdrKeys.dst = (uint16_t)numericValue;
          setHandleField(handle, STRUCT_MSG_FIELD_DST, structmsg->hdr.hdrInfo.hdrKeys.dst);
          result = structmsg_fillHdrInfo(handle, structmsg);
          return result;
        } else {
          return STRUCT_MSG_ERR_VALUE_TOO_BIG;
        }
      } else {
        return STRUCT_MSG_ERR_BAD_VALUE;
      }
    } else {
      if (c_strcmp(fieldName, "@cmdKey") == 0) {
        if (stringValue == NULL) {
          if ((numericValue >= 0) && (numericValue <= 65535)) {
            structmsg->hdr.hdrInfo.hdrKeys.cmdKey = (uint16_t)numericValue;
            setHandleField(handle, STRUCT_MSG_FIELD_CMD_KEY, structmsg->hdr.hdrInfo.hdrKeys.cmdKey);
            result = structmsg_fillHdrInfo(handle, structmsg);
            return result;
          } else {
            return STRUCT_MSG_ERR_VALUE_TOO_BIG;
          }
        } else {
          return STRUCT_MSG_ERR_BAD_VALUE;
        }
      }
    }
  }
  idx = 0;
  numEntries = structmsg->msg.numFieldInfos;
  while (idx < numEntries) {
    fieldInfo = &structmsg->msg.fieldInfos[idx];
    if (c_strcmp(fieldName, fieldInfo->fieldStr) == 0) {
      if (c_strcmp(fieldName, "@filler") == 0) {
//ets_printf("setFillerLgth: %d\n", numericValue);
        fieldInfo->fieldLgth = (uint16_t)numericValue;
        fixHeaderInfo(structmsg, fieldInfo, fieldName, fieldInfo->fieldType, fieldInfo->fieldLgth, 1);
        return STRUCT_MSG_ERR_OK;
      } else {
        return setFieldValue(structmsg, fieldInfo, fieldName, numericValue, stringValue);
      }
    }
    idx++;
  }
  return STRUCT_MSG_ERR_FIELD_NOT_FOUND;
}

// ============================= stmsg_setTableFieldValue ========================

int stmsg_setTableFieldValue(const uint8_t *handle, const uint8_t *fieldName, int row, int numericValue, const uint8_t *stringValue) {
  structmsg_t *structmsg;
  fieldInfo_t *fieldInfo;
  int idx;
  int cell;

  structmsg = structmsg_get_structmsg_ptr(handle);
  checkHandleOK(structmsg);
  if (row >= structmsg->msg.numTableRows) {
    return STRUCT_MSG_ERR_BAD_TABLE_ROW;
  }
  idx = 0;
  cell = 0 + row * structmsg->msg.numRowFields;
  while (idx < structmsg->msg.numRowFields) {
    fieldInfo = &structmsg->msg.tableFieldInfos[cell];
    if (c_strcmp(fieldName, fieldInfo->fieldStr) == 0) {
      return setFieldValue(structmsg, fieldInfo, fieldName, numericValue, stringValue);
    }
    idx++;
    cell++;
  }
  return STRUCT_MSG_ERR_FIELD_NOT_FOUND;
}


// ============================= stmsg_getFieldValue ========================

int stmsg_getFieldValue(const uint8_t *handle, const uint8_t *fieldName, int *numericValue, uint8_t **stringValue) {
  structmsg_t *structmsg;
  fieldInfo_t *fieldInfo;
  int idx;
  int numEntries;

  structmsg = structmsg_get_structmsg_ptr(handle);
  checkHandleOK(structmsg);
  *numericValue = 0;
  *stringValue = NULL;
  idx = 0;
  numEntries = structmsg->msg.numFieldInfos;
  while (idx < numEntries) {
    fieldInfo = &structmsg->msg.fieldInfos[idx];
    if (c_strcmp(fieldName, fieldInfo->fieldStr) == 0) {
      //ets_printf("    idx %d: key: %-20s\r\n", idx, fieldInfo->fieldStr);
      return getFieldValue(structmsg, fieldInfo, fieldName, numericValue, stringValue);
    }
    idx++;
  }
  return STRUCT_MSG_ERR_FIELD_NOT_FOUND;
}

// ============================= stmsg_getTableFieldValue ========================

int stmsg_getTableFieldValue(const uint8_t *handle, const uint8_t *fieldName, int row, int *numericValue, uint8_t **stringValue) {
  structmsg_t *structmsg;
  fieldInfo_t *fieldInfo;
  int idx;
  int cell;

  structmsg = structmsg_get_structmsg_ptr(handle);
  checkHandleOK(structmsg);
  if (row >= structmsg->msg.numTableRows) {
    return STRUCT_MSG_ERR_BAD_TABLE_ROW;
  }
  *numericValue = 0;
  *stringValue = NULL;

  idx = 0;
  cell = 0 + row * structmsg->msg.numRowFields;
  while (idx < structmsg->msg.numRowFields) {
    fieldInfo = &structmsg->msg.tableFieldInfos[cell];
    if (c_strcmp(fieldName, fieldInfo->fieldStr) == 0) {
      return getFieldValue(structmsg, fieldInfo, fieldName, numericValue, stringValue);
    }
    idx++;
    cell++;
  }
  return STRUCT_MSG_ERR_FIELD_NOT_FOUND;
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

// ============================= structmsg_createMsgFromListInfo ========================

int structmsg_createMsgFromListInfo(const uint8_t **listVector, uint8_t numEntries, uint8_t numRows, uint16_t flags, uint8_t **handle, uint8_t shortCmdKey) {
  const uint8_t *listEntry;
  int idx;
  int result;
  uint8_t*cp;
  uint8_t *handle2;
  uint8_t *fieldName;
  uint8_t *fieldType;
  uint8_t *fieldLgthStr;
  char *endPtr;
  uint8_t fieldLgth;
  uint8_t *flagStr;
  uint8_t flag;
  unsigned long lgth;
  unsigned long uflag;

//ets_printf("structmsg_createMsgFromListInfo: shortCmdKey: %d\n", shortCmdKey);
  result = stmsg_createMsg(numEntries, &handle2, shortCmdKey);
  checkErrOK(result);
  *handle=handle2;
  listEntry = listVector[0];
  idx = 0;
  while(idx < numEntries) {
    listEntry = listVector[idx];
    uint8_t buffer[c_strlen(listEntry) + 1];
    fieldName = buffer;
    c_memcpy(fieldName, listEntry, c_strlen(listEntry));
    fieldName[c_strlen(listEntry)] = '\0';
    cp = fieldName;
    while (*cp != ',') {
      cp++;
    }
    *cp++ = '\0';
    fieldType = cp;
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
      result = stmsg_addField(*handle, fieldName, fieldType, fieldLgth);
      checkErrOK(result);
    } else {
      if ((flags != 0) && (flag == 2)) {
        result = stmsg_addField(*handle, fieldName, fieldType, fieldLgth);
        checkErrOK(result);
      } else {
        if ((flags == 0) && (flag == 1)) {
          result = stmsg_addField(*handle, fieldName, fieldType, fieldLgth);
          checkErrOK(result);
        }
      }
    }
    idx++;
  }
  return STRUCT_MSG_ERR_OK;
}

#endif
