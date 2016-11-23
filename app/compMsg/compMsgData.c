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
//  uint8_t *header;
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

  *headerPtr = NULL;
  if (compMsgHandles.handles == NULL) {
    compMsgHandles.handles = os_zalloc(sizeof(handle2Header_t));
    if (compMsgHandles.handles == NULL) {
      return COMP_MSG_ERR_OUT_OF_MEMORY;
    } else {
      compMsgHandles.handles[compMsgHandles.numHandles].handle = handle;
      compMsgHandles.handles[compMsgHandles.numHandles].compMsgData = compMsgData;
//      *headerPtr = compMsgHandles.handles[compMsgHandles.numHandles].header;
      compMsgHandles.numHandles++;
      return COMP_MSG_ERR_OK;
    }
  } else {
    // check for unused slot first
    idx = 0;
    while (idx < compMsgHandles.numHandles) {
      if (compMsgHandles.handles[idx].handle == NULL) {
        compMsgHandles.handles[idx].handle = handle;
        compMsgHandles.handles[idx].compMsgData = compMsgData;
//        *headerPtr = compMsgHandles.handles[idx].header;
        return COMP_MSG_ERR_OK;
      }
      idx++;
    }
    compMsgHandles.handles = os_realloc(compMsgHandles.handles, sizeof(handle2Header_t)*(compMsgHandles.numHandles+1));
    checkAllocOK(compMsgHandles.handles);
    compMsgHandles.handles[compMsgHandles.numHandles].handle = handle;
    compMsgHandles.handles[idx].compMsgData = compMsgData;
//    *headerPtr = compMsgHandles.handles[compMsgHandles.numHandles].header;
    compMsgHandles.numHandles++;
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
  self->flags = 0;
  self->numFields = 0;
  self->maxFields = numFields;
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
  size_t lgth;
  uint8_t *data;
  uint8_t *where;
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
//ets_printf("§initMsg2f§");
  if (self->totalLgth == 0) {
//ets_printf("§initMsg3a§");
    return COMP_MSG_ERR_FIELD_TOTAL_LGTH_MISSING;
  }
//ets_printf("§initMsg3b§");
  result = self->compMsgDataView->dataView->getDataViewData(self->compMsgDataView->dataView, &where, &data, &lgth);
  checkErrOK(result);
  result = self->compMsgDataView->dataView->setDataViewData(self->compMsgDataView->dataView, "initMsg", data, self->totalLgth);
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
  uint8_t result;
  uint8_t *where;

  if ((self->flags & COMP_MSG_IS_INITTED) == 0) {
    return COMP_MSG_ERR_NOT_YET_INITTED;
  }
  if ((self->flags & COMP_MSG_IS_PREPARED) == 0) {
    return COMP_MSG_ERR_NOT_YET_PREPARED;
  }
  result = self->compMsgDataView->dataView->getDataViewData(self->compMsgDataView->dataView, &where, data, lgth);
  checkErrOK(result);
  return COMP_MSG_ERR_OK;
}

// ================================= deleteMsgDescParts ====================================

static uint8_t deleteMsgDescParts(compMsgDispatcher_t *self) {
  int idx;
  msgDescPart_t *part;

  if (self->compMsgData->numMsgDescParts > 0) {
    idx = 0;
    while (idx < self->compMsgData->numMsgDescParts) {
      part = &self->compMsgData->msgDescParts[idx];
      if (part->fieldNameStr != NULL) {
        os_free(part->fieldNameStr);
      }
      if (part->fieldTypeStr != NULL) {
        os_free(part->fieldTypeStr);
      }
      idx++;
    }
    os_free(self->compMsgData->msgDescParts);
    self->compMsgData->msgDescParts = NULL;
    self->compMsgData->numMsgDescParts = 0;
    self->compMsgData->maxMsgDescParts = 0;
  } else {
    if (self->compMsgData->msgDescParts != NULL) {
ets_printf("§self->compMsgData->msgDescParts != NULL and numMsgDescParts == 0§");
    }
  }
  return COMP_DISP_ERR_OK;
}

// ================================= deleteMsgValParts ====================================

static uint8_t deleteMsgValParts(compMsgDispatcher_t *self) {
  int idx;
  msgValPart_t *part;

  if (self->compMsgData->numMsgValParts > 0) {
    idx = 0;
    while (idx < self->compMsgData->numMsgValParts) {
    part = &self->compMsgData->msgValParts[idx];
      if (part->fieldNameStr != NULL) {
        os_free(part->fieldNameStr);
      }
      if (part->fieldValueStr != NULL) {
        os_free(part->fieldValueStr);
      }
      idx++;
    }
    os_free(self->compMsgData->msgValParts);
    self->compMsgData->msgValParts = NULL;
    self->compMsgData->numMsgValParts = 0;
    self->compMsgData->maxMsgValParts = 0;
  } else {
    if (self->compMsgData->msgValParts != NULL) {
ets_printf("§self->compMsgData->msgValParts != NULL and numMsgValParts == 0§");
    }
  }
  return COMP_DISP_ERR_OK;
}

// ================================= freeCompMsgData ====================================

static uint8_t freeCompMsgData(compMsgDispatcher_t *self) {
  if (self->compMsgData->compMsgDataView != NULL) {
    freeCompMsgDataView(self->compMsgData->compMsgDataView);
//    os_free(self->compMsgData->compMsgDataView);
//    self->compMsgData->compMsgDataView = NULL;
  }
  
  if (self->compMsgData->fields != NULL) {
    os_free(self->compMsgData->fields);
    self->compMsgData->fields = NULL;
  }
  if (self->compMsgData->keyValueFields != NULL) {
    os_free(self->compMsgData->keyValueFields);
    self->compMsgData->keyValueFields = NULL;
  }
  if (self->compMsgData->header != NULL) {
    os_free(self->compMsgData->header);
    self->compMsgData->header = NULL;
  }
// receivedData
// toSendData

  self->compMsgData->deleteMsgDescParts(self);
  self->compMsgData->numMsgDescParts = 0;
  self->compMsgData->maxMsgDescParts = 0;
  self->compMsgData->deleteMsgValParts(self);
  self->compMsgData->numMsgValParts = 0;
  self->compMsgData->maxMsgValParts = 0;

  if (self->compMsgData->prepareValuesCbName != NULL) {
    os_free(self->compMsgData->prepareValuesCbName);
    self->compMsgData->prepareValuesCbName = NULL;
  }
    
//  websocketUserData_t *wud;
//  netsocketUserData_t *nud;

// ???  os_free(self->compMsgData);
//  self->compMsgData = NULL;
  return COMP_MSG_ERR_OK;
}

// ================================= deleteMsg ====================================

static uint8_t deleteMsg(compMsgDispatcher_t *self) {
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

extern char *dataViewWhere[4];
static char *str1 = "newCompMsgData";

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
  if (dataViewWhere[0] == NULL) {
    dataViewWhere[0] = str1;
  } else {
    if (dataViewWhere[1] == NULL) {
      dataViewWhere[1] = str1;
    } else {
      if (dataViewWhere[2] == NULL) {
        dataViewWhere[2] = str1;
      } else {
        dataViewWhere[3] = str1;
      }
    }
  }
  
  compMsgData->fields = NULL;
  compMsgData->keyValueFields = NULL;
  compMsgData->header = NULL;
  compMsgData->flags = 0;
  compMsgData->numFields = 0;
  compMsgData->maxFields = 0;
  compMsgData->fieldOffset = 0;
  compMsgData->totalLgth = 0;
  compMsgData->cmdLgth = 0;
  compMsgData->headerLgth = 0;
  compMsgData->msgDescParts = NULL;
  compMsgData->numMsgDescParts = 0;
  compMsgData->maxMsgDescParts = 0;
  compMsgData->msgValParts = NULL;
  compMsgData->numMsgValParts = 0;
  compMsgData->maxMsgValParts = 0;

  // normalMsg
  compMsgData->createMsg = &createMsg;
  compMsgData->deleteMsg = &deleteMsg;
  compMsgData->addField = &addField;
  compMsgData->getFieldValue = &getFieldValue;
  compMsgData->setFieldValue = &setFieldValue;
  compMsgData->dumpFieldValue = &dumpFieldValue;
  compMsgData->dumpKeyValueFields = &dumpKeyValueFields;
  compMsgData->dumpFieldInfo = &dumpFieldInfo;
  compMsgData->dumpMsg = &dumpMsg;
  compMsgData->initMsg = &initMsg;
  compMsgData->prepareMsg = &prepareMsg;
  compMsgData->getMsgData = &getMsgData;
  compMsgData->deleteMsgDescParts = &deleteMsgDescParts;
  compMsgData->deleteMsgValParts = &deleteMsgValParts;
  compMsgData->setDispatcher = &setDispatcher;

  return compMsgData;
}
