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
  compMsgData_t *compMsgData;
} handle2Header_t;

typedef struct compMsgHandles
{
  handle2Header_t *handles;
  int numHandles;
} compMsgHandles_t;

// create an object
static compMsgHandles_t compMsgHandles = { NULL, 0};

// ================================= dumpFieldValue ====================================

static uint8_t dumpFieldValue(compMsgDispatcher_t *self, compMsgField_t *fieldInfo, const uint8_t *indent2) {
  int result;
  int valueIdx;
  uint8_t uch;
  int8_t ch;
  uint16_t ush;
  int16_t sh;
  uint32_t uval;
  int32_t val;
  uint8_t *stringValue;
  int numericValue;
  compMsgData_t *compMsgData;

  numericValue = 0;
  compMsgData = self->compMsgData;
  result = compMsgData->compMsgDataView->getFieldValue(self, compMsgData->compMsgDataView->dataView, fieldInfo, &numericValue, &stringValue, 0);
  checkErrOK(result);
  switch (fieldInfo->fieldTypeId) {
  case DATA_VIEW_FIELD_INT8_T:
    COMP_MSG_DBG(self, "d", 1, "      %svalue: 0x%02x %d\n", indent2, numericValue & 0xFF, numericValue);
    break;
  case DATA_VIEW_FIELD_UINT8_T:
    COMP_MSG_DBG(self, "d", 1, "      %svalue: 0x%02x %d\n", indent2, numericValue & 0xFF, numericValue & 0xFF);
    break;
  case DATA_VIEW_FIELD_INT16_T:
    COMP_MSG_DBG(self, "d", 1, "      %svalue: 0x%04x %d\n", indent2, numericValue & 0xFFFF, numericValue);
    break;
  case DATA_VIEW_FIELD_UINT16_T:
    COMP_MSG_DBG(self, "d", 1, "      %svalue: 0x%04x %d\n", indent2, numericValue & 0xFFFF, numericValue & 0xFFFF);
    break;
  case DATA_VIEW_FIELD_INT32_T:
    COMP_MSG_DBG(self, "d", 1, "      %svalue: 0x%08x %d\n", indent2, numericValue & 0xFFFFFFFF, numericValue);
    break;
  case DATA_VIEW_FIELD_UINT32_T:
    COMP_MSG_DBG(self, "d", 1, "      %svalue: 0x%08x %d\n", indent2, numericValue & 0xFFFFFFFF, numericValue & 0xFFFFFFFF);
    break;
  case DATA_VIEW_FIELD_INT8_VECTOR:
    valueIdx = 0;
    COMP_MSG_DBG(self, "d", 1, "      values:\n");
    while (valueIdx < fieldInfo->fieldLgth) {
      ch = stringValue[valueIdx];
      COMP_MSG_DBG(self, "d", 1, "        %sidx: %d value: %c 0x%02x %d\n", indent2, valueIdx, (char)ch, (uint8_t)(ch & 0xFF), (int8_t)ch);
      valueIdx++;
    }
    COMP_MSG_DBG(self, "d", 1, "\n");
    break;
  case DATA_VIEW_FIELD_UINT8_VECTOR:
    valueIdx = 0;
  COMP_MSG_DBG(self, "d", 1, "heap5: %d", system_get_free_heap_size());
    COMP_MSG_DBG(self, "d", 1, "      values:\n");
    while (valueIdx < fieldInfo->fieldLgth) {
      uch = stringValue[valueIdx];
      COMP_MSG_DBG(self, "d", 1, "        idx: %d value: %c 0x%02x %d\n", valueIdx, (char)uch, (uint8_t)(uch & 0xFF), (int)uch);
      valueIdx++;
    }
    break;
  case DATA_VIEW_FIELD_INT16_VECTOR:
    valueIdx = 0;
    COMP_MSG_DBG(self, "d", 1, "      values:");
    while (valueIdx < fieldInfo->fieldLgth) {
      result = compMsgData->compMsgDataView->dataView->getInt16(compMsgData->compMsgDataView->dataView, fieldInfo->fieldOffset+valueIdx*sizeof(int16_t), &sh);
      COMP_MSG_DBG(self, "d", 1, "        idx: %d value: 0x%04x\n", indent2, valueIdx, sh);
      valueIdx++;
    }
    COMP_MSG_DBG(self, "d", 1, "\n");
    break;
#ifdef NOTDEF
  case DATA_VIEW_FIELD_UINT16_VECTOR:
    valueIdx = 0;
    COMP_MSG_DBG(self, "d", 1, "      values:\n");
    while (valueIdx < fieldInfo->fieldLgth) {
      ush = fieldInfo->value.ushortVector[valueIdx];
      COMP_MSG_DBG(self, "d", 1, "        idx: %d value: 0x%04x\n", indent2, valueIdx, (uint16_t)(ush & 0xFFFF));
      valueIdx++;
    }
    break;
  case DATA_VIEW_FIELD_INT32_VECTOR:
    valueIdx = 0;
    COMP_MSG_DBG(self, "d", 1, "      values:");
    while (valueIdx < fieldInfo->fieldLgth) {
      val = fieldInfo->value.int32Vector[valueIdx];
      COMP_MSG_DBG(self, "d", 1, "        idx: %d value: 0x%08x\n", indent2, valueIdx, (int32_t)(val & 0xFFFFFFFF));
      valueIdx++;
    }
    COMP_MSG_DBG(self, "d", 1, "\n");
    break;
  case DATA_VIEW_FIELD_UINT32_VECTOR:
    valueIdx = 0;
COMP_MSG_DBG(self, "d", 1, "      values:\n");
while (valueIdx < fieldInfo->fieldLgth) {
uval = fieldInfo->value.uint32Vector[valueIdx];
COMP_MSG_DBG(self, "d", 1, "        idx: %d value: 0x%08x\n", indent2, valueIdx, (uint32_t)(uval & 0xFFFFFFFF));
valueIdx++;
}
break;
#endif
}
return DATA_VIEW_ERR_OK;
}

// ============================= dumpKeyValueFields ========================

static uint8_t dumpKeyValueFields(compMsgDispatcher_t *self, size_t offset) {
  int numEntries;
  int idx;
  int result;
  int row;
  int col;
  uint8_t *fieldTypeStr;
  uint8_t *fieldNameStr;
  compMsgField_t *fieldInfo;

  numEntries = self->compMsgData->numValueFields;
  COMP_MSG_DBG(self, "d", 1, "      numKeyValues: %d offset: %d\r\n", numEntries, offset);
  return COMP_MSG_ERR_OK;
}

// ================================= dumpFieldInfo ====================================

static uint8_t dumpFieldInfo(compMsgDispatcher_t *self, compMsgField_t *fieldInfo) {
  int idx;
  int result;
  uint8_t *fieldTypeStr;
  uint8_t *fieldNameStr;
  compMsgData_t *compMsgData;
  char buf[512];

  compMsgData = self->compMsgData;
  result = self->compMsgTypesAndNames->getFieldTypeStrFromId(self->compMsgTypesAndNames, fieldInfo->fieldTypeId, &fieldTypeStr);
  checkErrOK(result);
  result = self->compMsgTypesAndNames->getFieldNameStrFromId(self->compMsgTypesAndNames, fieldInfo->fieldNameId, &fieldNameStr);
  checkErrOK(result);
  ets_sprintf(buf, " fieldName: %-20s fieldType: %-8s fieldLgth: %5d offset: %d flags: ", fieldNameStr, fieldTypeStr, fieldInfo->fieldLgth, fieldInfo->fieldOffset);
  if (fieldInfo->fieldFlags & COMP_MSG_FIELD_IS_SET) {
    c_strcat(buf, " COMP_MSG_FIELD_IS_SET");
  }
  if (fieldInfo->fieldFlags & COMP_MSG_KEY_VALUE_FIELD) {
    c_strcat(buf, " COMP_MSG_KEY_VALUE_FIELD");
  }
  COMP_MSG_DBG(self, "d", 1, "%s", buf);
  ets_sprintf(buf, " fieldNameId: %5d fieldTypeId: %5d fieldKey: 0x%04x %d\r\n", fieldInfo->fieldNameId, fieldInfo->fieldTypeId, fieldInfo->fieldKey, fieldInfo->fieldKey);
  COMP_MSG_DBG(self, "d", 1, "%s", buf);
  return COMP_MSG_ERR_OK;
}

// ================================= dumpMsg ====================================

static uint8_t dumpMsg(compMsgDispatcher_t *self) {
  int numEntries;
  int idx;
  int valueIdx;
  int result;
  uint8_t *fieldTypeStr;
  uint8_t *fieldNameStr;
  uint8_t *stringValue;
  int numericValue;
  compMsgField_t *fieldInfo;
  compMsgData_t *compMsgData;
  char buf[512];

  compMsgData = self->compMsgData;
  COMP_MSG_DBG(self, "d", 1, "handle: %s", compMsgData->handle);
  numEntries = compMsgData->numFields;
  COMP_MSG_DBG(self, "d", 1, "  numFields: %d maxFields: %d", numEntries, (int)compMsgData->maxFields);
  COMP_MSG_DBG(self, "d", 1, "  headerLgth: %d cmdLgth: %d totalLgth: %d", compMsgData->headerLgth, compMsgData->cmdLgth, compMsgData->totalLgth);
  ets_sprintf(buf, "  flags:");
  if ((compMsgData->flags & COMP_MSG_HAS_CRC) != 0) {
    c_strcat(buf, " COMP_MSG_HAS_CRC");
  }
  if ((compMsgData->flags & COMP_MSG_UINT8_CRC) != 0) {
    c_strcat(buf, " COMP_MSG_UINT8_CRC");
  }
  if ((compMsgData->flags & COMP_MSG_HAS_FILLER) != 0) {
    c_strcat(buf, " COMP_MSG_HAS_FILLER");
  }
  if ((compMsgData->flags & COMP_MSG_U8_CMD_KEY) != 0) {
    c_strcat(buf, " COMP_MSG_U8_CMD_KEY");
  }
  if ((compMsgData->flags & COMP_MSG_HAS_TABLE_ROWS) != 0) {
    c_strcat(buf, " COMP_MSG_HAS_TABLE_ROWS");
  }
  if ((compMsgData->flags & COMP_MSG_IS_INITTED) != 0) {
    c_strcat(buf, " COMP_MSG_IS_INITTED");
  }
  if ((compMsgData->flags & COMP_MSG_IS_PREPARED) != 0) {
    c_strcat(buf, " COMP_MSG_IS_PREPARED");
  }
  COMP_MSG_DBG(self, "d", 1, "%s", buf);
  idx = 0;
  while (idx < numEntries) {
    fieldInfo = &compMsgData->fields[idx];
    result = self->compMsgTypesAndNames->getFieldTypeStrFromId(self->compMsgTypesAndNames, fieldInfo->fieldTypeId, &fieldTypeStr);
    checkErrOK(result);
    result = self->compMsgTypesAndNames->getFieldNameStrFromId(self->compMsgTypesAndNames, fieldInfo->fieldNameId, &fieldNameStr);
    checkErrOK(result);
    if (c_strcmp(fieldNameStr, "@numKeyValues") == 0) {
      ets_sprintf(buf, "    idx %d: fieldName: %-20s fieldType: %-8s fieldLgth: %5d offset: %5d flags: ", idx, fieldNameStr, fieldTypeStr, fieldInfo->fieldLgth, fieldInfo->fieldOffset);
      if (fieldInfo->fieldFlags & COMP_MSG_FIELD_IS_SET) {
        c_strcat(buf, " COMP_MSG_FIELD_IS_SET");
      }
      if (fieldInfo->fieldFlags & COMP_MSG_KEY_VALUE_FIELD) {
        c_strcat(buf, " COMP_MSG_KEY_VALUE_FIELD");
      }
      COMP_MSG_DBG(self, "d", 1, "%s", buf);
      if (fieldInfo->fieldFlags & COMP_MSG_FIELD_IS_SET) {
        result = dumpFieldValue(self, fieldInfo, "");
        checkErrOK(result);
        result = compMsgData->compMsgDataView->getFieldValue(self, compMsgData->compMsgDataView->dataView, fieldInfo, &numericValue, &stringValue, 0);
        checkErrOK(result);
        self->compMsgData->numValueFields = numericValue;
      }
      result = dumpKeyValueFields(self, fieldInfo->fieldOffset + fieldInfo->fieldLgth);
      checkErrOK(result);
      idx++;
      continue;
    }
    ets_sprintf(buf, "    idx %d: fieldName: %-20s fieldType: %-8s fieldLgth: %5d offset: %d key: %5d flags: ", idx, fieldNameStr, fieldTypeStr, fieldInfo->fieldLgth, fieldInfo->fieldOffset, fieldInfo->fieldKey);
    if (fieldInfo->fieldFlags & COMP_MSG_FIELD_IS_SET) {
      c_strcat(buf, " COMP_MSG_FIELD_IS_SET");
    }
    if (fieldInfo->fieldFlags & COMP_MSG_KEY_VALUE_FIELD) {
      c_strcat(buf, " COMP_MSG_KEY_VALUE_FIELD");
    }
    COMP_MSG_DBG(self, "d", 1, "%s", buf);
    if (fieldInfo->fieldFlags & COMP_MSG_FIELD_IS_SET) {
      result = dumpFieldValue(self, fieldInfo, "");
      checkErrOK(result);
    }
    idx++;
  }
  return COMP_MSG_ERR_OK;
}

// ============================= addHandle ========================

static int addHandle(compMsgDispatcher_t *self, uint8_t *handle) {
  int idx;

  COMP_MSG_DBG(self, "d", 2, "compMsgData addHandle: handles: %p numHandles: %d!handle: %s!", compMsgHandles.handles, compMsgHandles.numHandles, handle);
  if (compMsgHandles.handles == NULL) {
    compMsgHandles.handles = os_zalloc(sizeof(handle2Header_t));
    if (compMsgHandles.handles == NULL) {
      return COMP_MSG_ERR_OUT_OF_MEMORY;
    } else {
      compMsgHandles.handles[compMsgHandles.numHandles].handle = handle;
      compMsgHandles.handles[compMsgHandles.numHandles].compMsgData = self->compMsgData;
      compMsgHandles.numHandles++;
      return COMP_MSG_ERR_OK;
    }
  } else {
    // check for unused slot first
    idx = 0;
    while (idx < compMsgHandles.numHandles) {
      if (compMsgHandles.handles[idx].handle == NULL) {
        compMsgHandles.handles[idx].handle = handle;
        compMsgHandles.handles[idx].compMsgData = self->compMsgData;
        return COMP_MSG_ERR_OK;
      }
      idx++;
    }
    compMsgHandles.handles = os_realloc(compMsgHandles.handles, sizeof(handle2Header_t)*(compMsgHandles.numHandles+1));
    checkAllocOK(compMsgHandles.handles);
    compMsgHandles.handles[compMsgHandles.numHandles].handle = handle;
    compMsgHandles.handles[idx].compMsgData = self->compMsgData;
    compMsgHandles.numHandles++;
  }
  return COMP_MSG_ERR_OK;
}

// ============================= deleteHandle ========================

static int deleteHandle(compMsgDispatcher_t *self, const uint8_t *handle) {
  int idx;
  int numUsed;
  int found;

  COMP_MSG_DBG(self, "d", 2, "compMsgData deleteHandle: %s numHandles: %d!", handle, compMsgHandles.numHandles);
  if (compMsgHandles.handles == NULL) {
    COMP_MSG_DBG(self, "Y", 0, "compMsgData deleteHandle 1 HANDLE_NOT_FOUND");
    return COMP_MSG_ERR_HANDLE_NOT_FOUND;
  }
  found = 0;
  idx = 0;
  numUsed = 0;
  if (compMsgHandles.numHandles == 0) {
    return COMP_MSG_ERR_OK;
  }
  while (idx < compMsgHandles.numHandles) {
    if (compMsgHandles.handles[idx].handle != NULL) {
      COMP_MSG_DBG(self, "d", 2, "idx: %d handle: %s %s", idx, compMsgHandles.handles[idx].handle, handle);
    }
    if ((compMsgHandles.handles[idx].handle != NULL) && (c_strcmp(compMsgHandles.handles[idx].handle, handle) == 0)) {
      COMP_MSG_DBG(self, "d", 2, "set idx: %d handle: %s to NULL", idx, compMsgHandles.handles[idx].handle);
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
//    os_free(compMsgHandles.handles);
//    compMsgHandles.handles = NULL;
  }
  if (found) {
      return COMP_MSG_ERR_OK;
  }
  COMP_MSG_DBG(self, "Y", 0, "compMsgData deleteHandle 2 HANDLE_NOT_FOUND");
  return COMP_MSG_ERR_HANDLE_NOT_FOUND;
}

// ============================= checkHandle ========================

static int checkHandle(const char *handle, compMsgDispatcher_t **compMsgDispatcher) {
  int idx;

  if (compMsgHandles.handles == NULL) {
    ets_printf("compMsgData checkHandle 1 HANDLE_NOT_FOUND");
    return COMP_MSG_ERR_HANDLE_NOT_FOUND;
  }
  idx = 0;
  while (idx < compMsgHandles.numHandles) {
    if ((compMsgHandles.handles[idx].handle != NULL) && (c_strcmp(compMsgHandles.handles[idx].handle, handle) == 0)) {
      (*compMsgDispatcher)->compMsgData = compMsgHandles.handles[idx].compMsgData;
      return COMP_MSG_ERR_OK;
    }
    idx++;
  }
  ets_printf("compMsgData checkHandle 2 HANDLE_NOT_FOUND");
  return COMP_MSG_ERR_HANDLE_NOT_FOUND;
}

// ============================= compMsgGetPtrFromHandle ========================

uint8_t compMsgGetPtrFromHandle(const char *handle, compMsgDispatcher_t **compMsgDispatcher) {
  if (checkHandle(handle, compMsgDispatcher) != COMP_MSG_ERR_OK) {
    ets_printf("compMsgGetPtrFromHandle 1 HANDLE_NOT_FOUND");
    return COMP_MSG_ERR_HANDLE_NOT_FOUND;
  }
  return COMP_MSG_ERR_OK;
}

// ================================= createMsg ====================================

/**
 * \brief create the internal representation of a message
 * 
 * \param self The dispatcher struct
 * \param numFields The number of fields
 * \param handle The out param for the handle of this message
 * \return Error code or ErrorOK
 *
 */
static uint8_t createMsg(compMsgDispatcher_t *self, int numFields, uint8_t **handle) {
  uint8_t result;
  compMsgData_t *compMsgData;

  compMsgData = self->compMsgData;
  compMsgData->fields = os_zalloc(sizeof(compMsgField_t) * numFields);
  if (compMsgData->fields == NULL) {
    return COMP_MSG_ERR_OUT_OF_MEMORY;
  }
  compMsgData->flags = 0;
  compMsgData->numFields = 0;
  compMsgData->maxFields = numFields;
  compMsgData->fieldOffset = 0;
  compMsgData->totalLgth = 0;
  compMsgData->cmdLgth = 0;
  compMsgData->headerLgth = 0;
  os_sprintf(compMsgData->handle, "%s%p", HANDLE_PREFIX, self);
  COMP_MSG_DBG(self, "d", 2, "createMsg: addHandle: %s", compMsgData->handle);
  result = addHandle(self, compMsgData->handle);
  if (result != COMP_MSG_ERR_OK) {
    os_free(compMsgData->fields);
    deleteHandle(self, compMsgData->handle);
    os_free(compMsgData);
    return result;
  }
  *handle = compMsgData->handle;
  return COMP_MSG_ERR_OK;
}

// ================================= addField ====================================

/**
 * \brief add a field to the internal representation of a message
 * 
 * \param self The dispatcher struct
 * \param fieldName The field name
 * \param fieldType The field type name
 * \param fieldLgth The field lgth in bytes
 * \return Error code or ErrorOK
 *
 */
static uint8_t addField(compMsgDispatcher_t *self, const uint8_t *fieldName, const uint8_t *fieldType, uint8_t fieldLgth) {
  uint8_t numTableFields;
  uint8_t numTableRowFields;
  uint8_t numTableRows;
  uint8_t fieldTypeId = 0;
  uint8_t fieldNameId = 213;
  int row;
  int cellIdx;
  int result = COMP_MSG_ERR_OK;
  compMsgField_t *fieldInfo;
  compMsgData_t *compMsgData;

  compMsgData = self->compMsgData;
  COMP_MSG_DBG(self, "d", 2, "addfield: %s fieldType: %s fieldLgth: %d\n", fieldName, fieldType, fieldLgth);
  if (compMsgData->numFields >= compMsgData->maxFields) {
    return COMP_MSG_ERR_TOO_MANY_FIELDS;
  }
  result = self->compMsgTypesAndNames->getFieldTypeIdFromStr(self->compMsgTypesAndNames, fieldType, &fieldTypeId);
  checkErrOK(result);
  result = self->compMsgTypesAndNames->getFieldNameIdFromStr(self->compMsgTypesAndNames, fieldName, &fieldNameId, COMP_MSG_INCR);
  checkErrOK(result);
  fieldInfo = &compMsgData->fields[compMsgData->numFields];
  // need to check for duplicate here !!
  if (c_strcmp(fieldName, "@filler") == 0) {
    compMsgData->flags |= COMP_MSG_HAS_FILLER;
    fieldLgth = 0;
    fieldInfo->fieldNameId = fieldNameId;
    fieldInfo->fieldTypeId = fieldTypeId;
    fieldInfo->fieldLgth = fieldLgth;
    compMsgData->numFields++;
    return COMP_MSG_ERR_OK;
  }
  if (c_strcmp(fieldName, "@crc") == 0) {
    compMsgData->flags |= COMP_MSG_HAS_CRC;
    if (c_strcmp(fieldType, "uint8_t") == 0) {
      compMsgData->flags |= COMP_MSG_UINT8_CRC;
    }
    COMP_MSG_DBG(self, "d", 2, "flags: 0x%02x HAS_CRC: 0x%02x HAS_FILLER: 0x%02x UINT8_CRC: 0x%02x\n", compMsgData->flags, COMP_MSG_HAS_CRC, COMP_MSG_HAS_FILLER, COMP_MSG_UINT8_CRC);
  }
  if (c_strcmp(fieldName, "@totalCrc") == 0) {
    compMsgData->flags |= COMP_MSG_HAS_TOTAL_CRC;
    if (c_strcmp(fieldType, "uint8_t") == 0) {
      compMsgData->flags |= COMP_MSG_UINT8_TOTAL_CRC;
    }
    COMP_MSG_DBG(self, "d", 2, "flags: 0x%02x HAS_CRC: 0x%02x HAS_FILLER: 0x%02x UINT8_CRC: 0x%02x\n", compMsgData->flags, COMP_MSG_HAS_CRC, COMP_MSG_HAS_FILLER, COMP_MSG_UINT8_CRC);
  }
  fieldInfo->fieldNameId = fieldNameId;
  fieldInfo->fieldTypeId = fieldTypeId;
  fieldInfo->fieldLgth = fieldLgth;
  compMsgData->numFields++;  
  return COMP_MSG_ERR_OK;
}

// ================================= getFieldValue ====================================

static uint8_t getFieldValue(compMsgDispatcher_t *self, const uint8_t *fieldName, int *numericValue, uint8_t **stringValue) {
  compMsgField_t *fieldInfo;
  uint8_t fieldNameId;
  int idx;
  int numEntries;
  int result;
  compMsgData_t *compMsgData;
  size_t saveOffset;

  compMsgData = self->compMsgData;
  if ((compMsgData->flags & COMP_MSG_IS_INITTED) == 0) {
    return COMP_MSG_ERR_NOT_YET_INITTED;
  }
  result = self->compMsgTypesAndNames->getFieldNameIdFromStr(self->compMsgTypesAndNames, fieldName, &fieldNameId, COMP_MSG_NO_INCR);
  checkErrOK(result);
  idx = 0;
  numEntries = compMsgData->numFields;
  while (idx < numEntries) {
    fieldInfo = &compMsgData->fields[idx];
    if (fieldNameId == fieldInfo->fieldNameId) {
      saveOffset = fieldInfo->fieldOffset;
      if (c_strncmp(fieldName, "#key_", c_strlen("#key_")) == 0) {
        fieldInfo->fieldOffset += (sizeof(uint16_t) + sizeof(uint8_t) + sizeof(uint16_t));
      }
      result = compMsgData->compMsgDataView->getFieldValue(self, compMsgData->compMsgDataView->dataView, fieldInfo, numericValue, stringValue, 0);
      checkErrOK(result);
      fieldInfo->fieldOffset = saveOffset;
      break;
    }
    idx++;
  }
  return DATA_VIEW_ERR_OK;
}

// ================================= setFieldValue ====================================

static uint8_t setFieldValue(compMsgDispatcher_t *self, const uint8_t *fieldName, int numericValue, const uint8_t *stringValue) {
  compMsgField_t *fieldInfo;
  uint8_t fieldNameId;
  uint8_t saveFieldType;
  int idx;
  int numEntries;
  int result;
  size_t offset;
  compMsgData_t *compMsgData;

  compMsgData = self->compMsgData;
  if ((compMsgData->flags & COMP_MSG_IS_INITTED) == 0) {
    return COMP_MSG_ERR_NOT_YET_INITTED;
  }
  result = self->compMsgTypesAndNames->getFieldNameIdFromStr(self->compMsgTypesAndNames, fieldName, &fieldNameId, COMP_MSG_NO_INCR);
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
  numEntries = compMsgData->numFields;
  COMP_MSG_DBG(self, "d", 2, "numEntries: %d\n", numEntries);
  while (idx < numEntries) {
    fieldInfo = &compMsgData->fields[idx];
    if (fieldNameId == fieldInfo->fieldNameId) {
      if (fieldName[0] == '#') {
        COMP_MSG_DBG(self, "d", 2, "compMsgData setFieldValue: name: %s!numeric: %d!string: %s!fieldNameId: %d!fieldKey: %d!fieldSize: %d!fieldType: %d!\n", fieldName, numericValue, stringValue == NULL ? "nil" : (char *)stringValue, fieldNameId, fieldInfo->fieldKey, compMsgData->msgDescPart->fieldSize, compMsgData->msgDescPart->fieldType);
        // key value field !!
        // the type is always uint8_t* we set the individual type of the key below
        COMP_MSG_DBG(self, "d", 2, "fieldInfo->fieldLgth: %d offset: %d\n", fieldInfo->fieldLgth, fieldInfo->fieldOffset);
        offset = fieldInfo->fieldOffset;
        result = compMsgData->compMsgDataView->dataView->setUint16(compMsgData->compMsgDataView->dataView, offset, compMsgData->msgDescPart->fieldKey);
        checkErrOK(result);
        offset += 2;
        result = compMsgData->compMsgDataView->dataView->setUint8(compMsgData->compMsgDataView->dataView, offset, compMsgData->msgDescPart->fieldType);
        checkErrOK(result);
        offset += 1;
        result = compMsgData->compMsgDataView->dataView->setUint16(compMsgData->compMsgDataView->dataView, offset, compMsgData->msgDescPart->fieldSize);
        checkErrOK(result);
        // here we need the keyValue fieldType!!
        saveFieldType = fieldInfo->fieldTypeId;
        fieldInfo->fieldTypeId = fieldInfo->fieldKeyTypeId;
        result = compMsgData->compMsgDataView->setFieldValue(self, compMsgData->compMsgDataView->dataView, fieldInfo, numericValue, stringValue, sizeof(uint16_t) + sizeof(uint8_t) + sizeof(uint16_t));
        fieldInfo->fieldTypeId = saveFieldType;
        
        
      } else {
        COMP_MSG_DBG(self, "d", 2, "compMsgData setFieldValue: %s!value: 0x%04x %s type: %d!", fieldName, numericValue, stringValue == NULL ? "nil" : (char *)stringValue, fieldInfo->fieldTypeId);
        result = compMsgData->compMsgDataView->setFieldValue(self, compMsgData->compMsgDataView->dataView, fieldInfo, numericValue, stringValue, 0);
      }
      checkErrOK(result);
      fieldInfo->fieldFlags |= COMP_MSG_FIELD_IS_SET;
      break;
    }
    idx++;
  }
  COMP_MSG_DBG(self, "d", 2, "idx: %d\n", idx);
  return DATA_VIEW_ERR_OK;
}

// ================================= prepareMsg ====================================

static uint8_t prepareMsg(compMsgDispatcher_t *self) {
  int numEntries;
  int idx;
  int result;
  uint8_t headerLgth;
  size_t lgth;
  compMsgField_t *fieldInfo;
  compMsgData_t *compMsgData;

  compMsgData = self->compMsgData;
  if ((compMsgData->flags & COMP_MSG_IS_INITTED) == 0) {
    return COMP_MSG_ERR_NOT_YET_INITTED;
  }
  // create the values which are different for each message!!
  numEntries = compMsgData->numFields;
  idx = 0;
  while (idx < numEntries) {
    fieldInfo = &compMsgData->fields[idx];
    switch (fieldInfo->fieldNameId) {
      case COMP_MSG_SPEC_FIELD_RANDOM_NUM:
        result = compMsgData->compMsgDataView->setRandomNum(self, compMsgData->compMsgDataView->dataView, fieldInfo);
        checkErrOK(result);
        fieldInfo->fieldFlags |= COMP_MSG_FIELD_IS_SET;
        break;
      case COMP_MSG_SPEC_FIELD_SEQUENCE_NUM:
        if (self->compMsgWifiData->flags & WIFI_USE_SAVED_SEQUENCE_NUM) {
          result = compMsgData->compMsgDataView->setSavedSequenceNum(self, compMsgData->compMsgDataView->dataView, fieldInfo);
          self->compMsgWifiData->flags &= ~WIFI_USE_SAVED_SEQUENCE_NUM;
        } else {
          result = compMsgData->compMsgDataView->setSequenceNum(self, compMsgData->compMsgDataView->dataView, fieldInfo);
        }
        checkErrOK(result);
        fieldInfo->fieldFlags |= COMP_MSG_FIELD_IS_SET;
        break;
      case COMP_MSG_SPEC_FIELD_HDR_FILLER:
        result = compMsgData->compMsgDataView->setZeroFiller(self, compMsgData->compMsgDataView->dataView, fieldInfo);
        checkErrOK(result);
        fieldInfo->fieldFlags |= COMP_MSG_FIELD_IS_SET;
        break;
      case COMP_MSG_SPEC_FIELD_FILLER:
        result = compMsgData->compMsgDataView->setFiller(self, compMsgData->compMsgDataView->dataView, fieldInfo);
        checkErrOK(result);
        fieldInfo->fieldFlags |= COMP_MSG_FIELD_IS_SET;
        break;
      case COMP_MSG_SPEC_FIELD_CRC:
        if (compMsgData->currHdr->hdrEncryption == 'E') {
          headerLgth = self->msgHeaderInfos.headerLgth;
        } else {
          headerLgth = 0;
        }
        lgth = compMsgData->headerLgth + compMsgData->cmdLgth-fieldInfo->fieldLgth;
        if (compMsgData->flags & COMP_MSG_CRC_USE_HEADER_LGTH) {
            headerLgth = compMsgData->headerLgth;
            lgth -= headerLgth;
        }
        if (compMsgData->flags & COMP_MSG_HAS_TOTAL_CRC) {
          if (compMsgData->flags & COMP_MSG_UINT8_TOTAL_CRC) {
            lgth -= 1;
          } else {
            lgth -= 2;
          }
        }
//compMsgData->compMsgDataView->dataView->dumpBinaryWide(compMsgData->compMsgDataView->dataView->data, lgth, "beforecrc");
        result = compMsgData->compMsgDataView->setCrc(self, compMsgData->compMsgDataView->dataView, fieldInfo, headerLgth, lgth);
        checkErrOK(result);
        fieldInfo->fieldFlags |= COMP_MSG_FIELD_IS_SET;
        break;
      case COMP_MSG_SPEC_FIELD_TOTAL_CRC:
        break;
    }
    idx++;
  }
  compMsgData->flags |= COMP_MSG_IS_PREPARED;
  return COMP_MSG_ERR_OK;
}

// ================================= initMsg ====================================

/**
 * \brief initialize the basic message info
 * \param self The dispatcher struct
 * \return Error code or ErrorOK
 *
 */
static uint8_t initMsg(compMsgDispatcher_t *self) {
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
  msgDescPart_t *msgDescPart;
  compMsgData_t *compMsgData;

  compMsgData = self->compMsgData;
  // initialize field offsets for each field
  // initialize totalLgth, headerLgth, cmdLgth
  if ((compMsgData->flags & COMP_MSG_IS_INITTED) != 0) {
    return COMP_MSG_ERR_ALREADY_INITTED;
  }
  compMsgData->fieldOffset = 0;
  numEntries = compMsgData->numFields;
  idx = 0;
  compMsgData->headerLgth = 0;
  while (idx < numEntries) {
    fieldInfo = &compMsgData->fields[idx];
    fieldInfo->fieldOffset = compMsgData->fieldOffset;
    msgDescPart = &compMsgData->msgDescParts[idx];
    COMP_MSG_DBG(self, "d", 2, "initMsg2 idx: %d", idx);
    switch (fieldInfo->fieldNameId) {
      case COMP_MSG_SPEC_FIELD_SRC:
      case COMP_MSG_SPEC_FIELD_DST:
      case COMP_MSG_SPEC_FIELD_TOTAL_LGTH:
      case COMP_MSG_SPEC_FIELD_GUID:
      case COMP_MSG_SPEC_FIELD_SRC_ID:
      case COMP_MSG_SPEC_FIELD_HDR_FILLER:
        compMsgData->headerLgth += fieldInfo->fieldLgth;
        compMsgData->totalLgth = compMsgData->fieldOffset + fieldInfo->fieldLgth;
        break;
      case COMP_MSG_SPEC_FIELD_FILLER:
        fillerLgth = 0;
        crcLgth = 0;
        if (compMsgData->flags & COMP_MSG_HAS_CRC) {
          if (compMsgData->flags & COMP_MSG_UINT8_CRC) {
            crcLgth = 1;
          } else {
            crcLgth = 2;
          }
        }
        myLgth = compMsgData->fieldOffset + crcLgth - compMsgData->headerLgth;
        while ((myLgth % 16) != 0) {
          myLgth++;
          fillerLgth++;
        }
        fieldInfo->fieldLgth = fillerLgth;
        compMsgData->totalLgth = compMsgData->fieldOffset + fillerLgth + crcLgth;
        compMsgData->cmdLgth = compMsgData->totalLgth - compMsgData->headerLgth;
        COMP_MSG_DBG(self, "d", 2, "initMsg2a idx: %d", idx);
        break;
      default:
        compMsgData->totalLgth = compMsgData->fieldOffset + fieldInfo->fieldLgth;
        compMsgData->cmdLgth = compMsgData->totalLgth - compMsgData->headerLgth;
        COMP_MSG_DBG(self, "d", 2, "initMsg2b idx: %d totalLgth: %d", idx, compMsgData->totalLgth);
        break;
    }
    compMsgData->fieldOffset += fieldInfo->fieldLgth;
    idx++;
  }
  if (compMsgData->totalLgth == 0) {
    return COMP_MSG_ERR_FIELD_TOTAL_LGTH_MISSING;
  }
  compMsgData->flags |= COMP_MSG_IS_INITTED;


  COMP_MSG_DBG(self, "d", 2, "compMsgData: %p!", compMsgData);
  COMP_MSG_DBG(self, "d", 2, "compMsgData->totalLgth: %d!direction: %d!SEND: %d", compMsgData->totalLgth, compMsgData->direction, COMP_MSG_TO_SEND_DATA);
  if (compMsgData->direction == COMP_MSG_TO_SEND_DATA) {
    compMsgData->toSendData = os_zalloc(compMsgData->totalLgth);
    COMP_MSG_DBG(self, "d", 2, "toSendData: %p", compMsgData->toSendData);
    checkAllocOK(compMsgData->toSendData);
    COMP_MSG_DBG(self, "d", 2, "initMsg toSend newCompMsgDataView");
    compMsgData->compMsgDataView = newCompMsgDataView(compMsgData->toSendData, compMsgData->totalLgth);
  } else {
    compMsgData->receivedData = os_zalloc(compMsgData->totalLgth);
    COMP_MSG_DBG(self, "d", 2, "+++receivedData: %p\n", compMsgData->receivedData);
    checkAllocOK(compMsgData->receivedData);
    COMP_MSG_DBG(self, "d", 2, "initMsg received newCompMsgDataView");
    compMsgData->compMsgDataView = newCompMsgDataView(compMsgData->receivedData, compMsgData->totalLgth);
  }
  COMP_MSG_DBG(self, "d", 2, "compMsgDataView: %p", compMsgData->compMsgDataView);
  checkAllocOK(compMsgData->compMsgDataView);


  // set the appropriate field values for the lgth entries
  idx = 0;
  numEntries = compMsgData->numFields;
  while (idx < numEntries) {
    COMP_MSG_DBG(self, "d", 2, "initMsg4 idx: %d", idx);
    fieldInfo = &compMsgData->fields[idx];
    switch (fieldInfo->fieldNameId) {
      case COMP_MSG_SPEC_FIELD_TOTAL_LGTH:
        result = compMsgData->compMsgDataView->setFieldValue(self, compMsgData->compMsgDataView->dataView, fieldInfo, (int)compMsgData->totalLgth, NULL, 0);
        checkErrOK(result);
        fieldInfo->fieldFlags |= COMP_MSG_FIELD_IS_SET;
        break;
      case COMP_MSG_SPEC_FIELD_CMD_LGTH:
        result = compMsgData->compMsgDataView->setFieldValue(self, compMsgData->compMsgDataView->dataView, fieldInfo, (int)compMsgData->cmdLgth, NULL, 0);
        checkErrOK(result);
        fieldInfo->fieldFlags |= COMP_MSG_FIELD_IS_SET;
        break;
    }
    idx++;
  }
  COMP_MSG_DBG(self, "d", 2, "initMsg done");
  return COMP_MSG_ERR_OK;
}

// ================================= initReceivedMsg ====================================

/**
 * \brief initialize the basic message info of a received message
 * \param self The dispatcher struct
 * \return Error code or ErrorOK
 *
 */
static uint8_t initReceivedMsg(compMsgDispatcher_t *self) {
  int numEntries;
  int idx;
  int row;
  int col;
  int cellIdx;
  int result;
  uint16_t u16;
  uint8_t u8;
  uint8_t *stringValue;
  size_t lgth;
  uint8_t *data;
  uint8_t *where;
  size_t myLgth;
  size_t fillerLgth;
  size_t crcLgth;
  size_t msgDescPartIdx;
  size_t msgValPartIdx;
  size_t offset;
  compMsgField_t *fieldInfo;
  compMsgField_t *fieldInfo2;
  compMsgData_t *compMsgData;
  msgDescPart_t *msgDescPart;
  msgValPart_t *msgValPart;

  compMsgData = self->compMsgData;
  // initialize field offsets for each field
  // initialize totalLgth, headerLgth, cmdLgth
  if ((compMsgData->flags & COMP_MSG_IS_INITTED) != 0) {
    return COMP_MSG_ERR_ALREADY_INITTED;
  }
  compMsgData->fieldOffset = 0;
  numEntries = compMsgData->numFields;
  idx = 0;
  msgDescPartIdx = 0;
  msgValPartIdx = 0;
  compMsgData->headerLgth = 0;
  while (idx < numEntries) {
    fieldInfo = &compMsgData->fields[idx];
    fieldInfo->fieldOffset = compMsgData->fieldOffset;
    msgDescPart = &self->compMsgData->msgDescParts[idx];
    self->compMsgData->msgDescPart = msgDescPart;
    msgValPart = &self->compMsgData->msgValParts[msgValPartIdx];
    self->compMsgData->msgValPart = msgValPart;
    fieldInfo->fieldFlags |= COMP_MSG_FIELD_IS_SET;
    COMP_MSG_DBG(self, "d", 2, "initReceivedMsg2 idx: %d %s totalLgth: %d", idx, msgDescPart->fieldNameStr, compMsgData->totalLgth);
    switch (fieldInfo->fieldNameId) {
      case COMP_MSG_SPEC_FIELD_SRC:
      case COMP_MSG_SPEC_FIELD_DST:
      case COMP_MSG_SPEC_FIELD_TOTAL_LGTH:
      case COMP_MSG_SPEC_FIELD_GUID:
      case COMP_MSG_SPEC_FIELD_SRC_ID:
      case COMP_MSG_SPEC_FIELD_HDR_FILLER:
        compMsgData->headerLgth += fieldInfo->fieldLgth;
        compMsgData->totalLgth = compMsgData->fieldOffset + fieldInfo->fieldLgth;
        break;
      case COMP_MSG_SPEC_FIELD_FILLER:
        fillerLgth = 0;
        crcLgth = 0;
        if (compMsgData->flags & COMP_MSG_HAS_CRC) {
          if (compMsgData->flags & COMP_MSG_UINT8_CRC) {
            crcLgth = 1;
          } else {
            crcLgth = 2;
          }
        }
        myLgth = compMsgData->fieldOffset + crcLgth - compMsgData->headerLgth;
        while ((myLgth % 16) != 0) {
          myLgth++;
          fillerLgth++;
        }
        fieldInfo->fieldLgth = fillerLgth;
        compMsgData->totalLgth = compMsgData->fieldOffset + fillerLgth + crcLgth;
        compMsgData->cmdLgth = compMsgData->totalLgth - compMsgData->headerLgth;
        COMP_MSG_DBG(self, "d", 2, "initReceivedMsg2a idx: %d", idx);
        break;
      default:
        if (msgDescPart->fieldNameStr[0] == '#') {
          offset = self->compMsgData->fieldOffset;
          result = self->compMsgData->compMsgDataView->dataView->getUint16(self->compMsgData->compMsgDataView->dataView, offset, &u16);
          checkErrOK(result);
          offset += 2;
          result = self->compMsgData->compMsgDataView->dataView->getUint8(self->compMsgData->compMsgDataView->dataView, offset, &u8);
          checkErrOK(result);
          offset += 1;
          result = self->compMsgData->compMsgDataView->dataView->getUint16(self->compMsgData->compMsgDataView->dataView, offset, &u16);
          checkErrOK(result);
          fieldInfo->fieldLgth = (sizeof(uint16_t) + sizeof(uint8_t) + sizeof(uint16_t) + u16);
          self->compMsgData->totalLgth = self->compMsgData->fieldOffset + fieldInfo->fieldLgth;
        } else {
          compMsgData->totalLgth = compMsgData->fieldOffset + fieldInfo->fieldLgth;
//          compMsgData->cmdLgth = compMsgData->totalLgth - compMsgData->headerLgth;
        }
        COMP_MSG_DBG(self, "d", 2, "initReceivedMsg2b idx: %d", idx);
        break;
    }
    compMsgData->fieldOffset += fieldInfo->fieldLgth;
    idx++;
  }
  if (compMsgData->totalLgth == 0) {
    return COMP_MSG_ERR_FIELD_TOTAL_LGTH_MISSING;
  }
  compMsgData->flags |= COMP_MSG_IS_INITTED;


  COMP_MSG_DBG(self, "d", 2, "compMsgData: %p!", compMsgData);
  COMP_MSG_DBG(self, "d", 2, "compMsgData->totalLgth: %d!direction: %d!SEND: %d", compMsgData->totalLgth, compMsgData->direction, COMP_MSG_TO_SEND_DATA);
  COMP_MSG_DBG(self, "d", 2, "initReceivedMsg done");
  return COMP_MSG_ERR_OK;
}

// ============================= getMsgData ========================

/**
 * \brief get the data and lgth of a prepared message
 * \param self The dispatcher struct
 * \param data The out parameter data
 * \param lgth The out parameter lgth
 * \return Error code or ErrorOK
 *
 */
static uint8_t getMsgData(compMsgDispatcher_t *self, uint8_t **data, int *lgth) {
  uint8_t result;
  compMsgData_t *compMsgData;

  compMsgData = self->compMsgData;
  if ((compMsgData->flags & COMP_MSG_IS_INITTED) == 0) {
    return COMP_MSG_ERR_NOT_YET_INITTED;
  }
  if ((compMsgData->flags & COMP_MSG_IS_PREPARED) == 0) {
    return COMP_MSG_ERR_NOT_YET_PREPARED;
  }
  result = compMsgData->compMsgDataView->dataView->getDataViewData(compMsgData->compMsgDataView->dataView, data, lgth);
  checkErrOK(result);
  return COMP_MSG_ERR_OK;
}

// ================================= deleteMsgDescParts ====================================

static uint8_t deleteMsgDescParts(compMsgDispatcher_t *self) {
  int idx;
  msgDescPart_t *part;
  compMsgData_t *compMsgData;

  compMsgData = self->compMsgData;
  if (compMsgData->numMsgDescParts > 0) {
    idx = 0;
    while (idx < compMsgData->numMsgDescParts) {
      part = &compMsgData->msgDescParts[idx];
      if (part->fieldNameStr != NULL) {
        os_free(part->fieldNameStr);
      }
      if (part->fieldTypeStr != NULL) {
        os_free(part->fieldTypeStr);
      }
      idx++;
    }
    os_free(compMsgData->msgDescParts);
    compMsgData->msgDescParts = NULL;
    compMsgData->numMsgDescParts = 0;
    compMsgData->maxMsgDescParts = 0;
  } else {
    if (compMsgData->msgDescParts != NULL) {
      COMP_MSG_DBG(self, "d", 2, "self->compMsgData->msgDescParts != NULL and numMsgDescParts == 0");
    }
  }
  return COMP_MSG_ERR_OK;
}

// ================================= deleteMsgValParts ====================================

static uint8_t deleteMsgValParts(compMsgDispatcher_t *self) {
  int idx;
  msgValPart_t *part;
  compMsgData_t *compMsgData;

  compMsgData = self->compMsgData;
  if (compMsgData->numMsgValParts > 0) {
    idx = 0;
    while (idx < compMsgData->numMsgValParts) {
    part = &compMsgData->msgValParts[idx];
      if (part->fieldNameStr != NULL) {
        os_free(part->fieldNameStr);
      }
      if (part->fieldValueStr != NULL) {
        os_free(part->fieldValueStr);
      }
      idx++;
    }
    os_free(compMsgData->msgValParts);
    compMsgData->msgValParts = NULL;
    compMsgData->numMsgValParts = 0;
    compMsgData->maxMsgValParts = 0;
  } else {
    if (compMsgData->msgValParts != NULL) {
      COMP_MSG_DBG(self, "Y", 0, "self->compMsgData->msgValParts != NULL and numMsgValParts == 0");
    }
  }
  return COMP_MSG_ERR_OK;
}

// ================================= freeCompMsgData ====================================

static uint8_t freeCompMsgData(compMsgDispatcher_t *self) {
  uint8_t result;
  compMsgData_t *compMsgData;

  compMsgData = self->compMsgData;
  if (self->compMsgTypesAndNames != NULL) {
    result = self->compMsgTypesAndNames->freeCompMsgTypesAndNames(self->compMsgTypesAndNames);
    checkErrOK(result);
//    os_free(self->compMsgTypesAndNames);
//    self->compMsgTypesAndNames = NULL;
  }
  if (compMsgData->fields != NULL) {
    os_free(compMsgData->fields);
    compMsgData->fields = NULL;
  }
  if (compMsgData->keyValueFields != NULL) {
    os_free(compMsgData->keyValueFields);
    compMsgData->keyValueFields = NULL;
  }
  if (compMsgData->receivedData != NULL) {
    os_free(compMsgData->receivedData);
    compMsgData->receivedData = NULL;
  }
  if (compMsgData->toSendData != NULL) {
    os_free(compMsgData->toSendData);
    compMsgData->toSendData = NULL;
  }
  if (compMsgData->compMsgDataView != NULL) {
    COMP_MSG_DBG(self, "d", 2, "os freeMsg: dataView: %p compMsgDataView: %p", compMsgData->compMsgDataView->dataView, compMsgData->compMsgDataView);
    os_free(compMsgData->compMsgDataView->dataView);
    os_free(compMsgData->compMsgDataView);
    compMsgData->compMsgDataView = NULL;
  }
  if (compMsgData->received.compMsgDataView != NULL) {
    COMP_MSG_DBG(self, "d", 2, "os freeMsg: received dataView: %p compMsgDataView: %p", compMsgData->received.compMsgDataView->dataView, compMsgData->received.compMsgDataView);
    os_free(compMsgData->received.compMsgDataView->dataView);
    os_free(compMsgData->received.compMsgDataView);
    compMsgData->received.compMsgDataView = NULL;
  }
  compMsgData->deleteMsgDescParts(self);
  compMsgData->numMsgDescParts = 0;
  compMsgData->maxMsgDescParts = 0;
  compMsgData->deleteMsgValParts(self);
  compMsgData->numMsgValParts = 0;
  compMsgData->maxMsgValParts = 0;

  if (compMsgData->prepareValuesCbName != NULL) {
    os_free(compMsgData->prepareValuesCbName);
    compMsgData->prepareValuesCbName = NULL;
  }
    
  if (c_strlen(compMsgData->handle) > 0) {
    deleteHandle(self, compMsgData->handle);
    compMsgData->handle[0] = '\0';
  }
//  socketUserData_t *sud;

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

// ================================= newCompMsgData ====================================

compMsgData_t *newCompMsgData(void) {
  compMsgData_t *compMsgData = os_zalloc(sizeof(compMsgData_t));
  if (compMsgData == NULL) {
    return NULL;
  }
  
  compMsgData->fields = NULL;
  compMsgData->keyValueFields = NULL;
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
  compMsgData->handle[0] = '\0';

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
  compMsgData->initReceivedMsg = &initReceivedMsg;
  compMsgData->initMsg = &initMsg;
  compMsgData->prepareMsg = &prepareMsg;
  compMsgData->getMsgData = &getMsgData;
  compMsgData->deleteMsgDescParts = &deleteMsgDescParts;
  compMsgData->deleteMsgValParts = &deleteMsgValParts;
  compMsgData->setDispatcher = &setDispatcher;

  return compMsgData;
}
