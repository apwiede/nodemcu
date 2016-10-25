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
#include "structmsgData.h"

typedef struct id2offset {
  uint16_t id;
  uint16_t offset;
  uint8_t *name;
} id2offset_t;

// ================================= dumpDefDefinitions ====================================

static uint8_t dumpDefDefinitions(structmsgData_t *self, structmsgField_t *fieldInfo, const uint8_t *indent2, int fieldIdx, uint8_t *names) {
  int result;
  int valueIdx;
  int numFields;
  uint8_t uch;
  int8_t ch;
  uint16_t ush;
  int16_t sh;
  uint32_t uval;
  int32_t val;
  uint8_t *stringValue;
  int numericValue = 0;
  int fieldNameId;
  int fieldTypeId;
  int fieldLgth;
  uint8_t * fieldNameStr;
  uint8_t * fieldTypeStr;

  valueIdx = 0;
  while (valueIdx < fieldInfo->fieldLgth / sizeof(uint16_t)) {
    result = self->structmsgDefinitionDataView->getFieldValue(self->structmsgDefinitionDataView, fieldInfo, &fieldNameId, &stringValue, valueIdx++);
    result = self->structmsgDefinitionDataView->getFieldValue(self->structmsgDefinitionDataView, fieldInfo, &fieldTypeId, &stringValue, valueIdx++);
    result = self->structmsgDefinitionDataView->getFieldValue(self->structmsgDefinitionDataView, fieldInfo, &fieldLgth, &stringValue, valueIdx++);
    result = self->structmsgDataView->dataView->getFieldTypeStrFromId(self->structmsgDataView->dataView, fieldTypeId, &fieldTypeStr);
    checkErrOK(result);
    if (fieldNameId > STRUCT_MSG_SPEC_FIELD_LOW) {
      result = self->structmsgDataView->getFieldNameStrFromId(self->structmsgDataView, fieldNameId, &fieldNameStr);
      checkErrOK(result);
    } else {
      fieldNameStr = names+fieldNameId;
    }
    ets_printf("        defIdx: %3d fieldName: %3d %-20s fieldType: %3d %-8s fieldLgth: %5d\r\n", valueIdx/3, fieldNameId, fieldNameStr, fieldTypeId, fieldTypeStr, fieldLgth);
  }
  return DATA_VIEW_ERR_OK;
}

// ================================= dumpDefFieldValue ====================================

static uint8_t dumpDefFieldValue(structmsgData_t *self, structmsgField_t *fieldInfo, const uint8_t *indent2, int fieldIdx) {
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

  result = self->structmsgDefinitionDataView->getFieldValue(self->structmsgDefinitionDataView, fieldInfo, &numericValue, &stringValue, fieldIdx);
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
    ets_printf("      %svalues:", indent2);
    while (valueIdx < fieldInfo->fieldLgth) {
      ch = stringValue[valueIdx];
      ets_printf("        %sidx: %d value: %c 0x%02x\n", indent2, valueIdx, (char)ch, (uint8_t)(ch & 0xFF));
      valueIdx++;
    }
    ets_printf("\n");
    break;
  case DATA_VIEW_FIELD_UINT8_VECTOR:
    valueIdx = 0;
    ets_printf("      %svalues:\n", indent2);
    while (valueIdx < fieldInfo->fieldLgth) {
      uch = stringValue[valueIdx];
      ets_printf("        %sidx: %d value: %c 0x%02x\n", indent2, valueIdx, (char)uch, (uint8_t)(uch & 0xFF));
      valueIdx++;
    }
    break;
  case DATA_VIEW_FIELD_INT16_VECTOR:
    valueIdx = 0;
    ets_printf("      %svalues:", indent2);
    while (valueIdx < fieldInfo->fieldLgth/sizeof(int16_t)) {
      result = self->structmsgDefinitionDataView->dataView->getInt16(self->structmsgDefinitionDataView->dataView, fieldInfo->fieldOffset+valueIdx*sizeof(int16_t), &sh);
      checkErrOK(result);
      ets_printf("        %sidx: %d value: 0x%04x\n", indent2, valueIdx, sh);
      valueIdx++;
    }
    ets_printf("\n");
    break;
  case DATA_VIEW_FIELD_UINT16_VECTOR:
    valueIdx = 0;
    ets_printf("      %svalues:\n", indent2);
    while (valueIdx < fieldInfo->fieldLgth/sizeof(uint16_t)) {
      result = self->structmsgDefinitionDataView->getFieldValue(self->structmsgDefinitionDataView, fieldInfo, &numericValue, &stringValue, valueIdx);
      checkErrOK(result);
      ets_printf("        %sidx: %d value: 0x%04x\n", indent2, valueIdx, (uint16_t)(numericValue & 0xFFFF));
      valueIdx++;
    }
    break;
#ifdef NOTDEF
  case DATA_VIEW_FIELD_INT32_VECTOR:
    valueIdx = 0;
    ets_printf("      %svalues:", indent2);
    while (valueIdx < fieldInfo->fieldLgth) {
      val = fieldInfo->value.int32Vector[valueIdx];
      ets_printf("        %sidx: %d value: 0x%08x\n", indent2, valueIdx, (int32_t)(val & 0xFFFFFFFF));
      valueIdx++;
    }
    ets_printf("\n");
    break;
  case DATA_VIEW_FIELD_UINT32_VECTOR:
    valueIdx = 0;
    ets_printf("      %svalues:\n", indent2);
    while (valueIdx < fieldInfo->fieldLgth) {
      uval = fieldInfo->value.uint32Vector[valueIdx];
      ets_printf("        %sidx: %d value: 0x%08x\n", indent2, valueIdx, (uint32_t)(uval & 0xFFFFFFFF));
      valueIdx++;
    }
    break;
#endif
  }
  return DATA_VIEW_ERR_OK;
}

// ============================= dumpDefFields ========================

static uint8_t dumpDefFields(structmsgData_t *self) {
  int numEntries;
  int idx;
  int result;
  int row;
  int col;
  uint8_t *fieldTypeStr;
  uint8_t *fieldNameStr;
  structmsgField_t *fieldInfo;
  uint8_t *stringValue;
  int numericValue;

  numEntries = self->numDefFields;
  ets_printf("  defHandle: %s\r\n", self->handle);
  ets_printf("    numDefFields: %d\r\n", numEntries);
  idx = 0;
  while (idx < numEntries) {
    fieldInfo = &self->defFields[idx];
    result = self->structmsgDataView->dataView->getFieldTypeStrFromId(self->structmsgDataView->dataView, fieldInfo->fieldTypeId, &fieldTypeStr);
    checkErrOK(result);
    result = self->structmsgDataView->getFieldNameStrFromId(self->structmsgDataView, fieldInfo->fieldNameId, &fieldNameStr);
    checkErrOK(result);
    ets_printf("      idx: %d fieldName: %-20s fieldType: %-8s fieldLgth: %.5d offset: %d \r\n", idx, fieldNameStr, fieldTypeStr, fieldInfo->fieldLgth, fieldInfo->fieldOffset);
    if (fieldInfo->fieldFlags & STRUCT_MSG_FIELD_IS_SET) {
      if (fieldInfo->fieldNameId == STRUCT_MSG_SPEC_FIELD_DEFINITIONS) {
        result = dumpDefDefinitions(self, fieldInfo, "  ", 0, stringValue);
        checkErrOK(result);
      } else {
        result = dumpDefFieldValue(self, fieldInfo, "  ", 0);
        checkErrOK(result);
      }
      if (fieldInfo->fieldNameId == STRUCT_MSG_SPEC_FIELD_NORM_FLD_NAMES) {
        result = self->structmsgDefinitionDataView->getFieldValue(self->structmsgDefinitionDataView, fieldInfo, &numericValue, &stringValue, 0);
      }
    }
    idx++;
  }
  return STRUCT_MSG_ERR_OK;
}

// ============================= addDefField ========================

static uint8_t addDefField(structmsgData_t *self, uint8_t fieldNameId, uint8_t fieldTypeId, uint8_t fieldLgth) {
  structmsgField_t *defFieldInfo;

  if (self->numDefFields >= STRUCT_DEF_NUM_DEF_FIELDS) {
    return STRUCT_MSG_ERR_TOO_MANY_FIELDS;
  }
  defFieldInfo = &self->defFields[self->numDefFields];
  defFieldInfo->fieldNameId = fieldNameId;
  defFieldInfo->fieldTypeId = fieldTypeId;
  defFieldInfo->fieldLgth = fieldLgth;
  defFieldInfo->fieldOffset = self->defFieldOffset;
  self->defFieldOffset += fieldLgth;
  self->numDefFields++;
  return STRUCT_MSG_ERR_OK;
}

// ============================= getDefFieldValue ========================

static uint8_t getDefFieldValue(structmsgData_t *self, uint8_t fieldNameId, int *numericValue, uint8_t **stringValue, int fieldIdx) {
  if ((self->flags & STRUCT_DEF_IS_INITTED) == 0) {
    return STRUCT_DEF_ERR_NOT_YET_INITTED;
  }
  return STRUCT_MSG_ERR_OK;
}

// ============================= setDefFieldValue ========================

static uint8_t setDefFieldValue(structmsgData_t *self, uint8_t fieldNameId, int numericValue, const uint8_t *stringValue, int fieldIdx) {
  int result;
  int idx;
  int numEntries;
  bool found = false;
  structmsgField_t *fieldInfo;

  if ((self->flags & STRUCT_DEF_IS_INITTED) == 0) {
    return STRUCT_DEF_ERR_NOT_YET_INITTED;
  }
  idx = 0;
  numEntries = self->numDefFields;
  while (idx < numEntries) {
    fieldInfo = &self->defFields[idx];
    if (fieldNameId == fieldInfo->fieldNameId) {
      result = self->structmsgDefinitionDataView->setFieldValue(self->structmsgDefinitionDataView, fieldInfo, numericValue, stringValue, fieldIdx);
      checkErrOK(result);
      fieldInfo->fieldFlags |= STRUCT_MSG_FIELD_IS_SET;
      found = true;
      break;
    }
    idx++;
  }
  if (!found) {
    return STRUCT_MSG_ERR_FIELD_NOT_FOUND;
  }
  return STRUCT_MSG_ERR_OK;
}

// ============================= addDefFields ========================

static uint8_t addDefFields(structmsgData_t *self, uint16_t numNormFields, uint16_t normNamesSize, uint16_t definitionsSize, int direction) {
  int fieldIdx;
  int namesIdx;
  int result;
  int idx;
  uint8_t numNormEntries;
  size_t crcLgth;
  size_t myLgth;
  uint16_t fillerLgth;
  size_t namesOffset;

  result = addDefField(self, STRUCT_MSG_SPEC_FIELD_DST, DATA_VIEW_FIELD_UINT16_T, 2);
  checkErrOK(result);
  result = addDefField(self, STRUCT_MSG_SPEC_FIELD_SRC, DATA_VIEW_FIELD_UINT16_T, 2);
  checkErrOK(result);
  result = addDefField(self, STRUCT_MSG_SPEC_FIELD_TOTAL_LGTH, DATA_VIEW_FIELD_UINT16_T, 2);
  checkErrOK(result);
  self->defHeaderLgth = self->defFieldOffset;
  result = addDefField(self, STRUCT_MSG_SPEC_FIELD_CMD_KEY, DATA_VIEW_FIELD_UINT16_T, 2);
  checkErrOK(result);
  result = addDefField(self, STRUCT_MSG_SPEC_FIELD_CMD_LGTH, DATA_VIEW_FIELD_UINT16_T, 2);
  checkErrOK(result);
  result = addDefField(self, STRUCT_MSG_SPEC_FIELD_RANDOM_NUM, DATA_VIEW_FIELD_UINT32_T, 4);
  checkErrOK(result);
  if (direction == STRUCT_DEF_FROM_DATA) {
    result = self->structmsgDefinitionDataView->dataView->getUint8(self->structmsgDefinitionDataView->dataView, self->defFieldOffset, &numNormEntries);
    checkErrOK(result);
    numNormFields = numNormEntries;
    self->defNumNormFields = numNormFields;
  }
  result = addDefField(self, STRUCT_MSG_SPEC_FIELD_NUM_NORM_FLDS, DATA_VIEW_FIELD_UINT8_T, 1);
  checkErrOK(result);
  result = addDefField(self, STRUCT_MSG_SPEC_FIELD_NORM_FLD_IDS, DATA_VIEW_FIELD_UINT16_VECTOR, numNormFields*sizeof(uint16));
  checkErrOK(result);
  if (direction == STRUCT_DEF_FROM_DATA) {
    result = self->structmsgDefinitionDataView->dataView->getUint16(self->structmsgDefinitionDataView->dataView, self->defFieldOffset, &normNamesSize);
    checkErrOK(result);
    self->defNormNamesSize = normNamesSize;
  }
  result = addDefField(self, STRUCT_MSG_SPEC_FIELD_NORM_FLD_NAMES_SIZE, DATA_VIEW_FIELD_UINT16_T, 2);
  checkErrOK(result);
  result = addDefField(self, STRUCT_MSG_SPEC_FIELD_NORM_FLD_NAMES, DATA_VIEW_FIELD_UINT8_VECTOR, normNamesSize);
  checkErrOK(result);
  if (direction == STRUCT_DEF_FROM_DATA) {
    result = self->structmsgDefinitionDataView->dataView->getUint16(self->structmsgDefinitionDataView->dataView, self->defFieldOffset, &definitionsSize);
    checkErrOK(result);
    self->defDefinitionsSize = definitionsSize;
  }
  result = addDefField(self, STRUCT_MSG_SPEC_FIELD_DEFINITIONS_SIZE, DATA_VIEW_FIELD_UINT16_T, 2);
  checkErrOK(result);
  result = addDefField(self, STRUCT_MSG_SPEC_FIELD_DEFINITIONS, DATA_VIEW_FIELD_UINT16_VECTOR, definitionsSize);
  checkErrOK(result);
    fillerLgth = 0;
    crcLgth = 2;
    myLgth = self->defFieldOffset + crcLgth - self->defHeaderLgth;
    while ((myLgth % 16) != 0) {
      myLgth++;
      fillerLgth++;
    }
  result = addDefField(self, STRUCT_MSG_SPEC_FIELD_FILLER, DATA_VIEW_FIELD_UINT8_VECTOR, fillerLgth);
  checkErrOK(result);
  result = addDefField(self, STRUCT_MSG_SPEC_FIELD_CRC, DATA_VIEW_FIELD_UINT16_T, 2);
  checkErrOK(result);
  self->flags |= STRUCT_DEF_IS_INITTED;
  if (direction == STRUCT_DEF_TO_DATA) {
    self->structmsgDefinitionDataView->dataView->data = os_zalloc(self->defFieldOffset);
    checkAllocOK(self->structmsgDefinitionDataView->dataView->data);
  }
  self->structmsgDefinitionDataView->dataView->lgth = self->defFieldOffset;
  self->defTotalLgth = self->defFieldOffset;
  return STRUCT_MSG_ERR_OK;
}

// ============================= setStaticDefFields ========================

static uint8_t setStaticDefFields(structmsgData_t *self, int numNormFields, int normNamesSize, id2offset_t *normNamesOffsets, int definitionsSize) {
  int fieldIdx;
  int namesIdx;
  int tabIdx;
  int result;
  int idx;
  size_t headerLgth;
  size_t fieldLgth;
  size_t namesOffset;
  structmsgField_t *fieldInfo;
  structmsgField_t *tabFieldInfo;

// FIXME src and dst are dummy values for now!!
  result = setDefFieldValue(self, STRUCT_MSG_SPEC_FIELD_DST, 16640, NULL, 0);
  checkErrOK(result);
  result = setDefFieldValue(self, STRUCT_MSG_SPEC_FIELD_SRC, 22272, NULL, 0);
  checkErrOK(result);
  result = setDefFieldValue(self, STRUCT_MSG_SPEC_FIELD_TOTAL_LGTH, self->defFieldOffset, NULL, 0);
  checkErrOK(result);
  result = setDefFieldValue(self, STRUCT_MSG_SPEC_FIELD_CMD_KEY, STRUCT_DEF_CMD_KEY, NULL, 0);
  checkErrOK(result);
  result = setDefFieldValue(self, STRUCT_MSG_SPEC_FIELD_CMD_LGTH, self->defFieldOffset-headerLgth, NULL, 0);
  checkErrOK(result);
  result = setDefFieldValue(self, STRUCT_MSG_SPEC_FIELD_NUM_NORM_FLDS, numNormFields, NULL, 0);
  checkErrOK(result);
  idx = 0;
  while (idx < numNormFields) {
    result = setDefFieldValue(self, STRUCT_MSG_SPEC_FIELD_NORM_FLD_IDS, normNamesOffsets[idx].offset, NULL, idx);
    checkErrOK(result);
    idx++;
  }
  result = setDefFieldValue(self, STRUCT_MSG_SPEC_FIELD_NORM_FLD_NAMES_SIZE, normNamesSize, NULL, 0);
  checkErrOK(result);
  uint8_t names[normNamesSize];
  idx = 0;
  namesOffset = 0;
  while (idx < numNormFields) {
    c_memcpy(names+namesOffset, normNamesOffsets[idx].name,c_strlen(normNamesOffsets[idx].name));
    namesOffset += c_strlen(normNamesOffsets[idx].name);
    names[namesOffset] = 0;
    namesOffset++;
    idx++;
  }
  result = setDefFieldValue(self, STRUCT_MSG_SPEC_FIELD_NORM_FLD_NAMES, 0, names, 0);
  checkErrOK(result);
  result = setDefFieldValue(self, STRUCT_MSG_SPEC_FIELD_DEFINITIONS_SIZE, definitionsSize, NULL, 0);
  checkErrOK(result);
  fieldIdx = 0;
  idx = 0;
  namesIdx = 0;
  while (idx < self->numFields) {
    fieldInfo = &self->fields[idx];
    if (fieldInfo->fieldNameId < STRUCT_MSG_SPEC_FIELD_LOW) {
      result = setDefFieldValue(self, STRUCT_MSG_SPEC_FIELD_DEFINITIONS, normNamesOffsets[namesIdx++].offset, NULL, fieldIdx++);
      checkErrOK(result);
      result = setDefFieldValue(self, STRUCT_MSG_SPEC_FIELD_DEFINITIONS, fieldInfo->fieldTypeId, NULL, fieldIdx++);
      checkErrOK(result);
      result = setDefFieldValue(self, STRUCT_MSG_SPEC_FIELD_DEFINITIONS, fieldInfo->fieldLgth, NULL, fieldIdx++);
      checkErrOK(result);
    } else {
      result = setDefFieldValue(self, STRUCT_MSG_SPEC_FIELD_DEFINITIONS, fieldInfo->fieldNameId, NULL, fieldIdx++);
      checkErrOK(result);
      result = setDefFieldValue(self, STRUCT_MSG_SPEC_FIELD_DEFINITIONS, fieldInfo->fieldTypeId, NULL, fieldIdx++);
      checkErrOK(result);
      fieldLgth = fieldInfo->fieldLgth;
      if (fieldInfo->fieldNameId == STRUCT_MSG_SPEC_FIELD_TABLE_ROWS) {
        fieldLgth = self->numTableRows;
      }
      if (fieldInfo->fieldNameId == STRUCT_MSG_SPEC_FIELD_TABLE_ROW_FIELDS) {
        fieldLgth = self->numTableRowFields;
      }
      result = setDefFieldValue(self, STRUCT_MSG_SPEC_FIELD_DEFINITIONS, fieldLgth, NULL, fieldIdx++);
      checkErrOK(result);
      if (fieldInfo->fieldNameId == STRUCT_MSG_SPEC_FIELD_TABLE_ROW_FIELDS) {
        tabIdx = 0;
        while (tabIdx < self->numTableRowFields) {
          tabFieldInfo = &self->tableFields[tabIdx];
          result = setDefFieldValue(self, STRUCT_MSG_SPEC_FIELD_DEFINITIONS, normNamesOffsets[namesIdx++].offset, NULL, fieldIdx++);
          checkErrOK(result);
          result = setDefFieldValue(self, STRUCT_MSG_SPEC_FIELD_DEFINITIONS, tabFieldInfo->fieldTypeId, NULL, fieldIdx++);
          checkErrOK(result);
          result = setDefFieldValue(self, STRUCT_MSG_SPEC_FIELD_DEFINITIONS, tabFieldInfo->fieldLgth, NULL, fieldIdx++);
          checkErrOK(result);
          tabIdx++;
        }
      }
    }
    idx++;
  }
  return STRUCT_MSG_ERR_OK;
}

// ============================= initDef ========================

static uint8_t initDef(structmsgData_t *self) {
  uint8_t numNormFields;
  uint16_t normNamesSize;
  int idx;
  int tabIdx;
  int result;
  int numFields;
  size_t definitionsSize;
  structmsgField_t *fieldInfo;
  structmsgField_t *tabFieldInfo;
  uint8_t *fieldNameStr;
  size_t headerLgth;
  size_t namesOffset;

  if ((self->flags & STRUCT_DEF_IS_INITTED) != 0) {
    return STRUCT_DEF_ERR_ALREADY_INITTED;
  }
  numFields = self->numFields + self->numTableRowFields;
  definitionsSize = numFields * (sizeof(uint16_t) + sizeof(uint16_t) * sizeof(uint16_t));
  id2offset_t normNamesOffsets[numFields];
  numNormFields = 0;
  normNamesSize = 0;
  namesOffset = 0;
  idx = 0;
  while (idx < self->numFields) {
    fieldInfo = &self->fields[idx];
    if (fieldInfo->fieldNameId < STRUCT_MSG_SPEC_FIELD_LOW) {
      result = self->structmsgDataView->getFieldNameStrFromId(self->structmsgDataView, fieldInfo->fieldNameId, &fieldNameStr);
      checkErrOK(result);
      normNamesOffsets[numNormFields].id = fieldInfo->fieldNameId;
      normNamesOffsets[numNormFields].offset = namesOffset;
      normNamesOffsets[numNormFields].name = fieldNameStr;
      normNamesSize += c_strlen(fieldNameStr) + 1;
      namesOffset += c_strlen(fieldNameStr) + 1;
      numNormFields++;
    } else {
      if (fieldInfo->fieldNameId == STRUCT_MSG_SPEC_FIELD_TABLE_ROW_FIELDS) {
        tabIdx = 0;
        while (tabIdx < self->numTableRowFields) {
          tabFieldInfo = &self->tableFields[tabIdx];
          result = self->structmsgDataView->getFieldNameStrFromId(self->structmsgDataView, tabFieldInfo->fieldNameId, &fieldNameStr);
          checkErrOK(result);
          normNamesOffsets[numNormFields].id = fieldInfo->fieldNameId;
          normNamesOffsets[numNormFields].offset = namesOffset;
          normNamesOffsets[numNormFields].name = fieldNameStr;
          normNamesSize += c_strlen(fieldNameStr) + 1;
          namesOffset += c_strlen(fieldNameStr) + 1;
          numNormFields++;
          tabIdx++;
        }
      }
    }
    idx++;
  }
  result = addDefFields(self, numNormFields, normNamesSize, definitionsSize, STRUCT_DEF_TO_DATA);
  checkErrOK(result);
  result = setStaticDefFields(self, numNormFields, normNamesSize, normNamesOffsets, definitionsSize);
  checkErrOK(result);
  return STRUCT_MSG_ERR_OK;
}

// ============================= prepareDef ========================

static uint8_t prepareDef(structmsgData_t *self) {
  int numEntries;
  int idx;
  int result;
  structmsgField_t *fieldInfo;

  if ((self->flags & STRUCT_DEF_IS_INITTED) == 0) {
ets_printf("prepareDef not yet initted\n");
    return STRUCT_DEF_ERR_NOT_YET_INITTED;
  }
  // create the values which are different for each message!!
  numEntries = self->numDefFields;
  idx = 0;
  while (idx < numEntries) {
    fieldInfo = &self->defFields[idx];
    switch (fieldInfo->fieldNameId) {
      case STRUCT_MSG_SPEC_FIELD_RANDOM_NUM:
        result = self->structmsgDefinitionDataView->setRandomNum(self->structmsgDefinitionDataView, fieldInfo);
        checkErrOK(result);
        fieldInfo->fieldFlags |= STRUCT_MSG_FIELD_IS_SET;
        break;
      case STRUCT_MSG_SPEC_FIELD_SEQUENCE_NUM:
        result = self->structmsgDefinitionDataView->setSequenceNum(self->structmsgDefinitionDataView, fieldInfo);
        checkErrOK(result);
        fieldInfo->fieldFlags |= STRUCT_MSG_FIELD_IS_SET;
        break;
      case STRUCT_MSG_SPEC_FIELD_FILLER:
        result = self->structmsgDefinitionDataView->setFiller(self->structmsgDefinitionDataView, fieldInfo);
        checkErrOK(result);
        fieldInfo->fieldFlags |= STRUCT_MSG_FIELD_IS_SET;
        break;
      case STRUCT_MSG_SPEC_FIELD_CRC:
        result = self->structmsgDefinitionDataView->setCrc(self->structmsgDefinitionDataView, fieldInfo, self->headerLgth+1, self->cmdLgth-fieldInfo->fieldLgth);
        checkErrOK(result);
        fieldInfo->fieldFlags |= STRUCT_MSG_FIELD_IS_SET;
        break;
    }
    idx++;
  }
  self->flags |= STRUCT_DEF_IS_PREPARED;
  return STRUCT_MSG_ERR_OK;
}


// ============================= setDefData ========================

static uint8_t setDefData(structmsgData_t *self, const  uint8_t *data) {
  int idx;
  int result;
  uint16_t lgth;
  size_t numNormFields;
  size_t normNamesSize;
  size_t definitionsSize;
  structmsgField_t *fieldInfo;

  if (self->structmsgDefinitionDataView == NULL) {
    result = newStructmsgDefinition(self);
    checkErrOK(result);
  } else {
    if (self->structmsgDataView->dataView->data != NULL) {
      // free no longer used we cannot reuse as the size can be different!
      os_free(self->structmsgDataView->dataView->data);
      self->structmsgDataView->dataView->data = NULL;
      // we do net free self->defFields, we just reuse them it is always the same number of defFields
      os_free(self->defFields);
      self->defFields = NULL;
      self->numDefFields = 0;
      self->defFieldOffset = 0;
    }
  }
  // temporary replace data entry of dataView by our param data
  // to be able to use the get* functions for gettting totalLgth entry value
  self->structmsgDefinitionDataView->dataView->data = (uint8_t *)data;
  self->structmsgDefinitionDataView->dataView->lgth = 10;
  // get totalLgth value from data
  result = self->structmsgDefinitionDataView->dataView->getUint16(self->structmsgDefinitionDataView->dataView, 4, &lgth);
  checkErrOK(result);

//FIXME!! check crc!!

  // now make a copy of the data to be on the safe side
  // for freeing the Lua space in Lua set the variable to nil!!
  self->structmsgDefinitionDataView->dataView->data = os_zalloc(lgth);
  checkAllocOK(self->structmsgDefinitionDataView->dataView->data);
  c_memcpy(self->structmsgDefinitionDataView->dataView->data, data, lgth);
  self->structmsgDefinitionDataView->dataView->lgth = lgth;
  self->defTotalLgth = lgth;

  numNormFields = 0;
  normNamesSize = 0;
  definitionsSize = 0;
  result = addDefFields(self, numNormFields, normNamesSize, definitionsSize, STRUCT_DEF_FROM_DATA);
  checkErrOK(result);

  // and now set the IS_SET flags and other stuff
  self->flags |= STRUCT_DEF_IS_INITTED;
  idx = 0;
  while (idx < self->numDefFields) {
    fieldInfo = &self->defFields[idx];
    fieldInfo->fieldFlags |= STRUCT_MSG_FIELD_IS_SET;
    idx++;
  }
  self->flags |= STRUCT_DEF_IS_PREPARED;
  return STRUCT_MSG_ERR_OK;
}

// ============================= getDefData ========================

static uint8_t getDefData(structmsgData_t *self, uint8_t **data, int *lgth) {
  if ((self->flags & STRUCT_DEF_IS_INITTED) == 0) {
    return STRUCT_DEF_ERR_NOT_YET_INITTED;
  }
  if ((self->flags & STRUCT_DEF_IS_PREPARED) == 0) {
    return STRUCT_DEF_ERR_NOT_YET_PREPARED;
  }
  *data = self->structmsgDefinitionDataView->dataView->data;
  *lgth = self->defTotalLgth;
  return STRUCT_MSG_ERR_OK;
}

// ============================= createMsgFromDef ========================

static uint8_t createMsgFromDef(structmsgData_t *self) {
  structmsgField_t *fieldInfo;
  uint8_t *fieldNameStr;
  uint8_t *fieldTypeStr;
  uint16_t fieldNameId;
  uint16_t fieldTypeId;
  uint16_t fieldLgth;
  uint8_t *handle;
  uint8_t definitionsIdx;
  uint8_t numNormFlds;
  uint16_t normFldNamesSize;
  uint16_t definitionsSize;
  uint16_t val;
  uint8_t namesBuf[self->defNormNamesSize];
  uint8_t *names = namesBuf;
  int namesIdx;
  int fieldIdx;
  int idx;
  int result;

  // lopp over def Fields and extract infos
  id2offset_t normNamesOffsets[self->defNumNormFields];
  fieldIdx = 0;
  while (fieldIdx < self->numDefFields) {
    fieldInfo = &self->defFields[fieldIdx];
    switch (fieldInfo->fieldNameId) {
    case STRUCT_MSG_SPEC_FIELD_SRC:
    case STRUCT_MSG_SPEC_FIELD_DST:
    case STRUCT_MSG_SPEC_FIELD_TARGET_CMD:
    case STRUCT_MSG_SPEC_FIELD_TOTAL_LGTH:
    case STRUCT_MSG_SPEC_FIELD_CMD_KEY:
    case STRUCT_MSG_SPEC_FIELD_CMD_LGTH:
    case STRUCT_MSG_SPEC_FIELD_RANDOM_NUM:
    case STRUCT_MSG_SPEC_FIELD_FILLER:
    case STRUCT_MSG_SPEC_FIELD_CRC:
      // nothing to do!
      break;
    case STRUCT_MSG_SPEC_FIELD_NUM_NORM_FLDS:
      result = self->structmsgDefinitionDataView->dataView->getUint8(self->structmsgDefinitionDataView->dataView, fieldInfo->fieldOffset, &numNormFlds);
      checkErrOK(result);
      break;
    case STRUCT_MSG_SPEC_FIELD_NORM_FLD_IDS:
      idx = 0;
      while (idx < numNormFlds) {
        result = self->structmsgDefinitionDataView->dataView->getUint16(self->structmsgDefinitionDataView->dataView, fieldInfo->fieldOffset+idx*sizeof(uint16_t), &val);
        checkErrOK(result);
        normNamesOffsets[idx].offset = val;
        normNamesOffsets[idx].name = names+val;
        idx++;
      }
      break;
    case STRUCT_MSG_SPEC_FIELD_NORM_FLD_NAMES_SIZE:
      result = self->structmsgDefinitionDataView->dataView->getUint16(self->structmsgDefinitionDataView->dataView, fieldInfo->fieldOffset, &normFldNamesSize);
      checkErrOK(result);
      break;
    case STRUCT_MSG_SPEC_FIELD_NORM_FLD_NAMES:
      result = self->structmsgDefinitionDataView->dataView->getUint8Vector(self->structmsgDefinitionDataView->dataView, fieldInfo->fieldOffset, &names, fieldInfo->fieldLgth);
      checkErrOK(result);
      break;
    case STRUCT_MSG_SPEC_FIELD_DEFINITIONS_SIZE:
      result = self->structmsgDefinitionDataView->dataView->getUint16(self->structmsgDefinitionDataView->dataView, fieldInfo->fieldOffset, &definitionsSize);
      checkErrOK(result);
      break;
    case STRUCT_MSG_SPEC_FIELD_DEFINITIONS:
      idx = 0;
      namesIdx = 0;
      while (idx < definitionsSize / sizeof(uint16_t)) {
        result = self->structmsgDefinitionDataView->dataView->getUint16(self->structmsgDefinitionDataView->dataView, fieldInfo->fieldOffset+(idx++*sizeof(uint16_t)), &fieldNameId);
        checkErrOK(result);
        if (fieldNameId > STRUCT_MSG_SPEC_FIELD_LOW) {
          result = self->structmsgDataView->getFieldNameStrFromId(self->structmsgDataView, fieldNameId, &fieldNameStr);
          checkErrOK(result);
        } else {
          fieldNameStr = normNamesOffsets[namesIdx++].name;
          checkErrOK(result);
        }
        result = self->structmsgDefinitionDataView->dataView->getUint16(self->structmsgDefinitionDataView->dataView, fieldInfo->fieldOffset+(idx++*sizeof(uint16_t)), &fieldTypeId);
        checkErrOK(result);
        result = self->structmsgDataView->dataView->getFieldTypeStrFromId(self->structmsgDataView->dataView, fieldTypeId, &fieldTypeStr);
        checkErrOK(result);
        result = self->structmsgDefinitionDataView->dataView->getUint16(self->structmsgDefinitionDataView->dataView, fieldInfo->fieldOffset+(idx++*sizeof(uint16_t)), &fieldLgth);
        checkErrOK(result);
        result = self->addField(self, fieldNameStr, fieldTypeStr, fieldLgth);
        checkErrOK(result);
      }
      break;
      
    }
    fieldIdx++;
  }
  result = self->initMsg(self);

  return STRUCT_MSG_ERR_OK;
}

// ============================= newStructmsgDefinition ========================

uint8_t newStructmsgDefinition(structmsgData_t *self) {
  if (self->structmsgDefinitionDataView != NULL) {
    return STRUCT_DEF_ERR_ALREADY_CREATED; 
  }
  self->structmsgDefinitionDataView = newStructmsgDataView();
  if (self->structmsgDefinitionDataView == NULL) {
    return STRUCT_MSG_ERR_OUT_OF_MEMORY;
  }
  self->defFields = os_zalloc(STRUCT_DEF_NUM_DEF_FIELDS * sizeof(structmsgField_t));
  if (self->defFields == NULL) {
    return STRUCT_MSG_ERR_OUT_OF_MEMORY;
  }

  self->initDef = &initDef;
  self->prepareDef = &prepareDef;
  self->addDefField = &addDefField;
  self->dumpDefFields = &dumpDefFields;
  self->setDefFieldValue = &setDefFieldValue;
  self->getDefFieldValue = &getDefFieldValue;
  self->setDefData = &setDefData;
  self->getDefData = &getDefData;
  self->createMsgFromDef = &createMsgFromDef;

  return STRUCT_MSG_ERR_OK;
}

#ifdef NOTDEF

// ============================= structmsg_encdecDefinition ========================

int structmsg_encdecDefinition(const uint8_t *name, const uint8_t *key, size_t klen, const uint8_t *iv, size_t ivlen, bool enc, uint8_t **buf, int *lgth) {
  stmsgDefinition_t *definition;
  uint8_t definitionsIdx;
  int result;

  *buf = NULL;
  *lgth = 0;
  result = structmsg_getDefinitionPtr(name, &definition, &definitionsIdx);
  checkErrOK(result);

  if (enc) {
    if (definition->encoded == NULL) {
      return STRUCT_MSG_ERR_NOT_ENCODED;
    }
    result = structmsg_encryptdecrypt(NULL, definition->encoded, definition->totalLgth, key, klen, iv, ivlen, enc, &definition->encrypted, lgth);
    *buf = definition->encrypted;
  } else {
    if (definition->encrypted == NULL) {
      return STRUCT_MSG_ERR_NOT_ENCRYPTED;
    }
    result = structmsg_encryptdecrypt(NULL, definition->encrypted, definition->totalLgth, key, klen, iv, ivlen, enc, &definition->todecode, lgth);
    *buf = definition->todecode;
  }
  return result;
} 

// ============================= stmsg_setCryptedDefinition ========================

int stmsg_setCryptedDefinition(const uint8_t *name, const uint8_t *crypted, int cryptedLgth) {
  stmsgDefinition_t *definition;
  uint8_t definitionsIdx;
  int result;

  result = structmsg_getDefinitionPtr(name, &definition, &definitionsIdx);
  checkErrOK(result);
  if (definition->encrypted != NULL) {
    os_free(definition->encrypted);
  }
  definition->encrypted = (uint8_t *)os_malloc(cryptedLgth);
  checkAllocOK(definition->encrypted);
  c_memcpy(definition->encrypted, crypted, cryptedLgth);
  return STRUCT_MSG_ERR_OK;
}

// ============================= stmsg_decryptGetDefinitionName  ========================

int stmsg_decryptGetDefinitionName(const uint8_t *encryptedMsg, size_t mlen, const uint8_t *key, size_t klen, const uint8_t *iv, size_t ivlen, uint8_t **name, uint8_t shortCmdKey) {
  uint8_t *decrypted;
  size_t lgth;
  int result;

   decrypted = NULL;
   lgth = 0; 
   result = structmsg_encryptdecrypt(NULL, encryptedMsg, mlen, key, klen, iv, ivlen, false, &decrypted, &lgth);
   if (result != STRUCT_MSG_ERR_OK) {
     return result;
   }
   result = stmsg_getDefinitionName(decrypted, name, shortCmdKey);
   return result;
}

// ============================= structmsg_deleteDefinition ========================

int structmsg_deleteDefinition(const uint8_t *name, stmsgDefinitions_t *structmsgDefinitions, fieldNameDefinitions_t *fieldNameDefinitions) {
  stmsgDefinition_t *definition;
  fieldInfoDefinition_t *fieldInfo;
  name2id_t *nameEntry;
  uint8_t definitionsIdx;
  int idx;
  int nameIdx;
  int nameFound;
  int result;
  int fieldId;

  result =  structmsg_getDefinitionPtr(name, &definition, &definitionsIdx);
  checkErrOK(result);
  idx = 0;
  while (idx < definition->numFields) {
    fieldInfo = &definition->fieldInfos[idx];
    nameIdx = 0;
    nameFound = 0;
    if (fieldInfo->fieldId < STRUCT_MSG_SPEC_FIELD_LOW) {
      while (nameIdx < fieldNameDefinitions->numDefinitions) {
        nameEntry = &fieldNameDefinitions->definitions[nameIdx];
        if (fieldInfo->fieldId == nameEntry->id) {
          result = structmsg_getFieldNameId(nameEntry->str, &fieldId, STRUCT_MSG_DECR);
          checkErrOK(result);
          nameFound = 1;
          break;
        }
        nameIdx++;
      }
      if (!nameFound) {
        return STRUCT_MSG_ERR_FIELD_NOT_FOUND;
      }
    }
    idx++;
  }
  // nameDefinitions deleted

  definition->numFields = 0;
  definition->maxFields = 0;
  os_free(definition->name);
  definition->name = NULL;
  if (definition->encoded != NULL) {
    os_free(definition->encoded);
    definition->encoded = NULL;
  }
  os_free(definition->fieldInfos);
  definition->fieldInfos = NULL;
  if (definition->encoded != NULL) {
    os_free(definition->encoded);
    definition->encoded = NULL;
  }
  if (definition->encrypted != NULL) {
    os_free(definition->encrypted);
    definition->encrypted = NULL;
  }
  if (definition->todecode != NULL) {
    os_free(definition->todecode);
    definition->todecode = NULL;
  }
  definition->totalLgth = 0;
  // definition deleted

  return STRUCT_MSG_ERR_OK;
}

// ============================= structmsg_deleteDefinitions ========================

int structmsg_deleteDefinitions(stmsgDefinitions_t *structmsgDefinitions, fieldNameDefinitions_t *fieldNameDefinitions) {
  // delete the whole structmsgDefinitions info, including fieldNameDefinitions info
  stmsgDefinition_t *definition;
  fieldInfoDefinition_t *fieldInfo;
  name2id_t *nameEntry;
  uint8_t *name;
  int idx;
  int nameIdx;
  int found;
  int nameFound;
  int result;
  int fieldId;

  idx = 0;
  while (idx < structmsgDefinitions->numDefinitions) {
    definition = &structmsgDefinitions->definitions[idx];
    if (definition->name != NULL) {
      structmsg_deleteDefinition(definition->name, structmsgDefinitions, fieldNameDefinitions);
    }
    idx++;
  }
  structmsgDefinitions->numDefinitions = 0;
  structmsgDefinitions->maxDefinitions = 0;
  os_free(structmsgDefinitions->definitions);
  structmsgDefinitions->definitions = NULL;

  fieldNameDefinitions->numDefinitions = 0;
  fieldNameDefinitions->maxDefinitions = 0;
  os_free(fieldNameDefinitions->definitions);
  fieldNameDefinitions->definitions = NULL;

  // all deleted/reset
  return STRUCT_MSG_ERR_OK;
}

// ============================= structmsg_deleteStructmsgDefinition ========================

int structmsg_deleteStructmsgDefinition(const uint8_t *name) {
  return structmsg_deleteDefinition(name, &structmsgDefinitions, &fieldNameDefinitions);
}

// ============================= structmsg_deleteStructmsgDefinitions ========================

int structmsg_deleteStructmsgDefinitions() {
  // delete the whole structmsgDefinitions info, including fieldNameDefinitions info
  return structmsg_deleteDefinitions(&structmsgDefinitions, &fieldNameDefinitions);
}

// ============================= structmsg_getDefinitionNormalFieldNames ========================

int structmsg_getDefinitionNormalFieldNames(const uint8_t *name, uint8_t ***normalFieldNames) {
  stmsgDefinition_t *definition;
  uint8_t definitionsIdx;
  int result;

  result = structmsg_getDefinitionPtr(name, &definition, &definitionsIdx);
  checkErrOK(result);
  return STRUCT_MSG_ERR_OK;
}

// ============================= structmsg_getDefinitionTableFieldNames ========================

int structmsg_getDefinitionTableFieldNames(const uint8_t *name, uint8_t ***tableFieldNames) {
  stmsgDefinition_t *definition;
  uint8_t definitionsIdx;
  int result;

  result = structmsg_getDefinitionPtr(name, &definition, &definitionsIdx);
  checkErrOK(result);
  return STRUCT_MSG_ERR_OK;
}

// ============================= structmsg_getDefinitionNumTableRows ========================

int structmsg_getDefinitionNumTableRows(const uint8_t *name, uint8_t *numTableRows) {
  stmsgDefinition_t *definition;
  fieldInfoDefinition_t *fieldInfo;
  uint8_t definitionsIdx;
  int idx;
  int result;
  uint8_t *fieldName;

  result = structmsg_getDefinitionPtr(name, &definition, &definitionsIdx);
  checkErrOK(result);
  idx = 0;
  while(idx < definition->numFields) {
    fieldInfo = &definition->fieldInfos[idx];
    result  = structmsg_getIdFieldNameStr (fieldInfo->fieldId, &fieldName);
    checkErrOK(result);
    if (c_strcmp(fieldName, "@tablerows") == 0) {
      *numTableRows = fieldInfo->fieldLgth;
      return STRUCT_MSG_ERR_OK;
    }
  }
  return STRUCT_MSG_ERR_FIELD_NOT_FOUND;
}

// ============================= structmsg_getDefinitionNumTableRowFields ========================

int structmsg_getDefinitionNumTableRowFields(const uint8_t *name, uint8_t *numTableRowFields) {
  stmsgDefinition_t *definition;
  fieldInfoDefinition_t *fieldInfo;
  uint8_t definitionsIdx;
  int idx;
  int result;
  uint8_t *fieldName;

  result = structmsg_getDefinitionPtr(name, &definition, &definitionsIdx);
  checkErrOK(result);
  idx = 0;
  while(idx < definition->numFields) {
    fieldInfo = &definition->fieldInfos[idx];
    result  = structmsg_getIdFieldNameStr (fieldInfo->fieldId, &fieldName);
    checkErrOK(result);
    if (c_strcmp(fieldName, "@tablerowfields") == 0) {
      *numTableRowFields = fieldInfo->fieldLgth;
      return STRUCT_MSG_ERR_OK;
    }
  }
  return STRUCT_MSG_ERR_FIELD_NOT_FOUND;
}

// ============================= structmsg_getDefinitionFieldInfo ========================

int structmsg_getDefinitionFieldInfo(const uint8_t *name, const uint8_t *fieldName, fieldInfoDefinition_t **fieldInfo) {
  stmsgDefinition_t *definition;
  uint8_t definitionsIdx;
  int idx;
  int result;
  uint8_t *lookupFieldName;

  result = structmsg_getDefinitionPtr(name, &definition, &definitionsIdx);
  checkErrOK(result);
  idx = 0;
  while(idx < definition->numFields) {
    *fieldInfo = &definition->fieldInfos[idx];
    result  = structmsg_getIdFieldNameStr ((*fieldInfo)->fieldId, &lookupFieldName);
    checkErrOK(result);
    if (c_strcmp(lookupFieldName, fieldName) == 0) {
      return STRUCT_MSG_ERR_OK;
    }
  }
  return STRUCT_MSG_ERR_FIELD_NOT_FOUND;
}

// ============================= structmsg_getDefinitionTableFieldInfo ========================

int structmsg_getDefinitionTableFieldInfo(const uint8_t *name, const uint8_t *fieldName, fieldInfoDefinition_t **fieldInfo) {
  stmsgDefinition_t *definition;
  uint8_t definitionsIdx;
  int idx;
  int result;
  uint8_t *lookupFieldName;

  result = structmsg_getDefinitionPtr(name, &definition, &definitionsIdx);
  checkErrOK(result);
  idx = 0;
  while(idx < definition->numFields) {
    *fieldInfo = &definition->fieldInfos[idx];
    result  = structmsg_getIdFieldNameStr ((*fieldInfo)->fieldId, &lookupFieldName);
    checkErrOK(result);
    if (c_strcmp(lookupFieldName, fieldName) == 0) {
      return STRUCT_MSG_ERR_OK;
    }
  }
  return STRUCT_MSG_ERR_FIELD_NOT_FOUND;
}

// ============================= structmsg_createMsgDefinitionFromListInfo ========================

int structmsg_createMsgDefinitionFromListInfo(const uint8_t *name, const uint8_t **listVector, uint8_t numEntries, uint8_t numRows, uint16_t flags,uint8_t shortCmdKey) {
  const uint8_t *listEntry;
  int idx;
  int result;
  uint8_t*cp;
  uint8_t *fieldName;
  uint8_t *fieldType;
  uint8_t *fieldLgthStr;
  char *endPtr;
  uint8_t fieldLgth;
  uint8_t *flagStr;
  uint8_t flag;
  unsigned long lgth;
  unsigned long uflag;

ets_printf("structmsg_createMsgDefinitionFromListInfo: shortCmdKey: %d\n", shortCmdKey);
  result = structmsg_createStructmsgDefinition(name, numEntries, shortCmdKey);
  checkErrOK(result);
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
      result = structmsg_addFieldDefinition(name, fieldName, fieldType, fieldLgth);
      checkErrOK(result);
    } else {
      if ((flags != 0) && (flag == 2)) {
        result = structmsg_addFieldDefinition(name, fieldName, fieldType, fieldLgth);
        checkErrOK(result);
      } else {
        if ((flags == 0) && (flag == 1)) {
          result = structmsg_addFieldDefinition(name, fieldName, fieldType, fieldLgth);
          checkErrOK(result);
        }
      }
    }
    idx++;
  }
  return STRUCT_MSG_ERR_OK;
}
#endif
