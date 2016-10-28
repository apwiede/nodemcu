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
#include "compMsgData.h"

typedef struct id2offset {
  uint16_t id;
  uint16_t offset;
  uint8_t *name;
} id2offset_t;

// ================================= dumpDefDefMsg ====================================

static uint8_t dumpDefDefMsg(compMsgData_t *self, compMsgField_t *fieldInfo, const uint8_t *indent2, int fieldIdx, uint8_t *names) {
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
    result = self->compMsgDefMsgDataView->getFieldValue(self->compMsgDefMsgDataView, fieldInfo, &fieldNameId, &stringValue, valueIdx++);
    result = self->compMsgDefMsgDataView->getFieldValue(self->compMsgDefMsgDataView, fieldInfo, &fieldTypeId, &stringValue, valueIdx++);
    result = self->compMsgDefMsgDataView->getFieldValue(self->compMsgDefMsgDataView, fieldInfo, &fieldLgth, &stringValue, valueIdx++);
    result = self->compMsgDataView->dataView->getFieldTypeStrFromId(self->compMsgDataView->dataView, fieldTypeId, &fieldTypeStr);
    checkErrOK(result);
    if (fieldNameId > COMP_MSG_SPEC_FIELD_LOW) {
      result = self->compMsgDataView->getFieldNameStrFromId(self->compMsgDataView, fieldNameId, &fieldNameStr);
      checkErrOK(result);
    } else {
      fieldNameStr = names+fieldNameId;
    }
    ets_printf("        defIdx: %3d fieldName: %3d %-20s fieldType: %3d %-8s fieldLgth: %5d\r\n", valueIdx/3, fieldNameId, fieldNameStr, fieldTypeId, fieldTypeStr, fieldLgth);
  }
  return DATA_VIEW_ERR_OK;
}

// ================================= dumpDefFieldValue ====================================

static uint8_t dumpDefFieldValue(compMsgData_t *self, compMsgField_t *fieldInfo, const uint8_t *indent2, int fieldIdx) {
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

  result = self->compMsgDefMsgDataView->getFieldValue(self->compMsgDefMsgDataView, fieldInfo, &numericValue, &stringValue, fieldIdx);
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
      result = self->compMsgDefMsgDataView->dataView->getInt16(self->compMsgDefMsgDataView->dataView, fieldInfo->fieldOffset+valueIdx*sizeof(int16_t), &sh);
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
      result = self->compMsgDefMsgDataView->getFieldValue(self->compMsgDefMsgDataView, fieldInfo, &numericValue, &stringValue, valueIdx);
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

static uint8_t dumpDefFields(compMsgData_t *self) {
  int numEntries;
  int idx;
  int result;
  int row;
  int col;
  uint8_t *fieldTypeStr;
  uint8_t *fieldNameStr;
  compMsgField_t *fieldInfo;
  uint8_t *stringValue;
  int numericValue;

  numEntries = self->numDefFields;
  ets_printf("  defHandle: %s\r\n", self->handle);
  ets_printf("    numDefFields: %d\r\n", numEntries);
  idx = 0;
  while (idx < numEntries) {
    fieldInfo = &self->defFields[idx];
    result = self->compMsgDataView->dataView->getFieldTypeStrFromId(self->compMsgDataView->dataView, fieldInfo->fieldTypeId, &fieldTypeStr);
    checkErrOK(result);
    result = self->compMsgDataView->getFieldNameStrFromId(self->compMsgDataView, fieldInfo->fieldNameId, &fieldNameStr);
    checkErrOK(result);
    ets_printf("      idx: %d fieldName: %-20s fieldType: %-8s fieldLgth: %.5d offset: %d \r\n", idx, fieldNameStr, fieldTypeStr, fieldInfo->fieldLgth, fieldInfo->fieldOffset);
    if (fieldInfo->fieldFlags & COMP_MSG_FIELD_IS_SET) {
      if (fieldInfo->fieldNameId == COMP_MSG_SPEC_FIELD_DEFINITIONS) {
        result = dumpDefDefMsg(self, fieldInfo, "  ", 0, stringValue);
        checkErrOK(result);
      } else {
        result = dumpDefFieldValue(self, fieldInfo, "  ", 0);
        checkErrOK(result);
      }
      if (fieldInfo->fieldNameId == COMP_MSG_SPEC_FIELD_NORM_FLD_NAMES) {
        result = self->compMsgDefMsgDataView->getFieldValue(self->compMsgDefMsgDataView, fieldInfo, &numericValue, &stringValue, 0);
      }
    }
    idx++;
  }
  return COMP_MSG_ERR_OK;
}

// ============================= addDefField ========================

static uint8_t addDefField(compMsgData_t *self, uint8_t fieldNameId, uint8_t fieldTypeId, uint8_t fieldLgth) {
  compMsgField_t *defFieldInfo;

  if (self->numDefFields >= COMP_DEF_NUM_DEF_FIELDS) {
    return COMP_MSG_ERR_TOO_MANY_FIELDS;
  }
  defFieldInfo = &self->defFields[self->numDefFields];
  defFieldInfo->fieldNameId = fieldNameId;
  defFieldInfo->fieldTypeId = fieldTypeId;
  defFieldInfo->fieldLgth = fieldLgth;
  defFieldInfo->fieldOffset = self->defFieldOffset;
  self->defFieldOffset += fieldLgth;
  self->numDefFields++;
  return COMP_MSG_ERR_OK;
}

// ============================= getDefFieldValue ========================

static uint8_t getDefFieldValue(compMsgData_t *self, uint8_t fieldNameId, int *numericValue, uint8_t **stringValue, int fieldIdx) {
  if ((self->flags & COMP_DEF_IS_INITTED) == 0) {
    return COMP_DEF_ERR_NOT_YET_INITTED;
  }
  return COMP_MSG_ERR_OK;
}

// ============================= setDefFieldValue ========================

static uint8_t setDefFieldValue(compMsgData_t *self, uint8_t fieldNameId, int numericValue, const uint8_t *stringValue, int fieldIdx) {
  int result;
  int idx;
  int numEntries;
  bool found = false;
  compMsgField_t *fieldInfo;

  if ((self->flags & COMP_DEF_IS_INITTED) == 0) {
    return COMP_DEF_ERR_NOT_YET_INITTED;
  }
  idx = 0;
  numEntries = self->numDefFields;
  while (idx < numEntries) {
    fieldInfo = &self->defFields[idx];
    if (fieldNameId == fieldInfo->fieldNameId) {
      result = self->compMsgDefMsgDataView->setFieldValue(self->compMsgDefMsgDataView, fieldInfo, numericValue, stringValue, fieldIdx);
      checkErrOK(result);
      fieldInfo->fieldFlags |= COMP_MSG_FIELD_IS_SET;
      found = true;
      break;
    }
    idx++;
  }
  if (!found) {
    return COMP_MSG_ERR_FIELD_NOT_FOUND;
  }
  return COMP_MSG_ERR_OK;
}

// ============================= addDefFields ========================

static uint8_t addDefFields(compMsgData_t *self, uint16_t numNormFields, uint16_t normNamesSize, uint16_t defsSize, int direction) {
  int fieldIdx;
  int namesIdx;
  int result;
  int idx;
  uint8_t numNormEntries;
  size_t crcLgth;
  size_t myLgth;
  uint16_t fillerLgth;
  size_t namesOffset;

  result = addDefField(self, COMP_MSG_SPEC_FIELD_DST, DATA_VIEW_FIELD_UINT16_T, 2);
  checkErrOK(result);
  result = addDefField(self, COMP_MSG_SPEC_FIELD_SRC, DATA_VIEW_FIELD_UINT16_T, 2);
  checkErrOK(result);
  result = addDefField(self, COMP_MSG_SPEC_FIELD_TOTAL_LGTH, DATA_VIEW_FIELD_UINT16_T, 2);
  checkErrOK(result);
  self->defHeaderLgth = self->defFieldOffset;
  result = addDefField(self, COMP_MSG_SPEC_FIELD_CMD_KEY, DATA_VIEW_FIELD_UINT16_T, 2);
  checkErrOK(result);
  result = addDefField(self, COMP_MSG_SPEC_FIELD_CMD_LGTH, DATA_VIEW_FIELD_UINT16_T, 2);
  checkErrOK(result);
  result = addDefField(self, COMP_MSG_SPEC_FIELD_RANDOM_NUM, DATA_VIEW_FIELD_UINT32_T, 4);
  checkErrOK(result);
  if (direction == COMP_DEF_FROM_DATA) {
    result = self->compMsgDefMsgDataView->dataView->getUint8(self->compMsgDefMsgDataView->dataView, self->defFieldOffset, &numNormEntries);
    checkErrOK(result);
    numNormFields = numNormEntries;
    self->defNumNormFields = numNormFields;
  }
  result = addDefField(self, COMP_MSG_SPEC_FIELD_NUM_NORM_FLDS, DATA_VIEW_FIELD_UINT8_T, 1);
  checkErrOK(result);
  result = addDefField(self, COMP_MSG_SPEC_FIELD_NORM_FLD_IDS, DATA_VIEW_FIELD_UINT16_VECTOR, numNormFields*sizeof(uint16));
  checkErrOK(result);
  if (direction == COMP_DEF_FROM_DATA) {
    result = self->compMsgDefMsgDataView->dataView->getUint16(self->compMsgDefMsgDataView->dataView, self->defFieldOffset, &normNamesSize);
    checkErrOK(result);
    self->defNormNamesSize = normNamesSize;
  }
  result = addDefField(self, COMP_MSG_SPEC_FIELD_NORM_FLD_NAMES_SIZE, DATA_VIEW_FIELD_UINT16_T, 2);
  checkErrOK(result);
  result = addDefField(self, COMP_MSG_SPEC_FIELD_NORM_FLD_NAMES, DATA_VIEW_FIELD_UINT8_VECTOR, normNamesSize);
  checkErrOK(result);
  if (direction == COMP_DEF_FROM_DATA) {
    result = self->compMsgDefMsgDataView->dataView->getUint16(self->compMsgDefMsgDataView->dataView, self->defFieldOffset, &defsSize);
    checkErrOK(result);
    self->defDefsSize = defsSize;
  }
  result = addDefField(self, COMP_MSG_SPEC_FIELD_DEFINITIONS_SIZE, DATA_VIEW_FIELD_UINT16_T, 2);
  checkErrOK(result);
  result = addDefField(self, COMP_MSG_SPEC_FIELD_DEFINITIONS, DATA_VIEW_FIELD_UINT16_VECTOR, defsSize);
  checkErrOK(result);
  fillerLgth = 0;
  crcLgth = 2;
  myLgth = self->defFieldOffset + crcLgth - self->defHeaderLgth;
  while ((myLgth % 16) != 0) {
    myLgth++;
    fillerLgth++;
  }
  result = addDefField(self, COMP_MSG_SPEC_FIELD_FILLER, DATA_VIEW_FIELD_UINT8_VECTOR, fillerLgth);
  checkErrOK(result);
  result = addDefField(self, COMP_MSG_SPEC_FIELD_CRC, DATA_VIEW_FIELD_UINT16_T, 2);
  checkErrOK(result);
  self->flags |= COMP_DEF_IS_INITTED;
  if (direction == COMP_DEF_TO_DATA) {
    self->compMsgDefMsgDataView->dataView->data = os_zalloc(self->defFieldOffset);
    checkAllocOK(self->compMsgDefMsgDataView->dataView->data);
  }
  self->compMsgDefMsgDataView->dataView->lgth = self->defFieldOffset;
  self->defTotalLgth = self->defFieldOffset;
  return COMP_MSG_ERR_OK;
}

// ============================= setStaticDefFields ========================

static uint8_t setStaticDefFields(compMsgData_t *self, int numNormFields, int normNamesSize, id2offset_t *normNamesOffsets, int defsSize) {
  int fieldIdx;
  int namesIdx;
  int tabIdx;
  int result;
  int idx;
  size_t headerLgth;
  size_t fieldLgth;
  size_t namesOffset;
  compMsgField_t *fieldInfo;
  compMsgField_t *tabFieldInfo;

// FIXME src and dst are dummy values for now!!
  result = setDefFieldValue(self, COMP_MSG_SPEC_FIELD_DST, 16640, NULL, 0);
  checkErrOK(result);
  result = setDefFieldValue(self, COMP_MSG_SPEC_FIELD_SRC, 22272, NULL, 0);
  checkErrOK(result);
  result = setDefFieldValue(self, COMP_MSG_SPEC_FIELD_TOTAL_LGTH, self->defFieldOffset, NULL, 0);
  checkErrOK(result);
  result = setDefFieldValue(self, COMP_MSG_SPEC_FIELD_CMD_KEY, COMP_DEF_CMD_KEY, NULL, 0);
  checkErrOK(result);
  result = setDefFieldValue(self, COMP_MSG_SPEC_FIELD_CMD_LGTH, self->defFieldOffset-headerLgth, NULL, 0);
  checkErrOK(result);
  result = setDefFieldValue(self, COMP_MSG_SPEC_FIELD_NUM_NORM_FLDS, numNormFields, NULL, 0);
  checkErrOK(result);
  idx = 0;
  while (idx < numNormFields) {
    result = setDefFieldValue(self, COMP_MSG_SPEC_FIELD_NORM_FLD_IDS, normNamesOffsets[idx].offset, NULL, idx);
    checkErrOK(result);
    idx++;
  }
  result = setDefFieldValue(self, COMP_MSG_SPEC_FIELD_NORM_FLD_NAMES_SIZE, normNamesSize, NULL, 0);
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
  result = setDefFieldValue(self, COMP_MSG_SPEC_FIELD_NORM_FLD_NAMES, 0, names, 0);
  checkErrOK(result);
  result = setDefFieldValue(self, COMP_MSG_SPEC_FIELD_DEFINITIONS_SIZE, defsSize, NULL, 0);
  checkErrOK(result);
  fieldIdx = 0;
  idx = 0;
  namesIdx = 0;
  while (idx < self->numFields) {
    fieldInfo = &self->fields[idx];
    if (fieldInfo->fieldNameId < COMP_MSG_SPEC_FIELD_LOW) {
      result = setDefFieldValue(self, COMP_MSG_SPEC_FIELD_DEFINITIONS, normNamesOffsets[namesIdx++].offset, NULL, fieldIdx++);
      checkErrOK(result);
      result = setDefFieldValue(self, COMP_MSG_SPEC_FIELD_DEFINITIONS, fieldInfo->fieldTypeId, NULL, fieldIdx++);
      checkErrOK(result);
      result = setDefFieldValue(self, COMP_MSG_SPEC_FIELD_DEFINITIONS, fieldInfo->fieldLgth, NULL, fieldIdx++);
      checkErrOK(result);
    } else {
      result = setDefFieldValue(self, COMP_MSG_SPEC_FIELD_DEFINITIONS, fieldInfo->fieldNameId, NULL, fieldIdx++);
      checkErrOK(result);
      result = setDefFieldValue(self, COMP_MSG_SPEC_FIELD_DEFINITIONS, fieldInfo->fieldTypeId, NULL, fieldIdx++);
      checkErrOK(result);
      fieldLgth = fieldInfo->fieldLgth;
      if (fieldInfo->fieldNameId == COMP_MSG_SPEC_FIELD_TABLE_ROWS) {
        fieldLgth = self->numTableRows;
      }
      if (fieldInfo->fieldNameId == COMP_MSG_SPEC_FIELD_TABLE_ROW_FIELDS) {
        fieldLgth = self->numTableRowFields;
      }
      result = setDefFieldValue(self, COMP_MSG_SPEC_FIELD_DEFINITIONS, fieldLgth, NULL, fieldIdx++);
      checkErrOK(result);
      if (fieldInfo->fieldNameId == COMP_MSG_SPEC_FIELD_TABLE_ROW_FIELDS) {
        tabIdx = 0;
        while (tabIdx < self->numTableRowFields) {
          tabFieldInfo = &self->tableFields[tabIdx];
          result = setDefFieldValue(self, COMP_MSG_SPEC_FIELD_DEFINITIONS, normNamesOffsets[namesIdx++].offset, NULL, fieldIdx++);
          checkErrOK(result);
          result = setDefFieldValue(self, COMP_MSG_SPEC_FIELD_DEFINITIONS, tabFieldInfo->fieldTypeId, NULL, fieldIdx++);
          checkErrOK(result);
          result = setDefFieldValue(self, COMP_MSG_SPEC_FIELD_DEFINITIONS, tabFieldInfo->fieldLgth, NULL, fieldIdx++);
          checkErrOK(result);
          tabIdx++;
        }
      }
    }
    idx++;
  }
  return COMP_MSG_ERR_OK;
}

// ============================= initDefMsg ========================

static uint8_t initDefMsg(compMsgData_t *self) {
  uint8_t numNormFields;
  uint16_t normNamesSize;
  int idx;
  int tabIdx;
  int result;
  int numFields;
  size_t defsSize;
  compMsgField_t *fieldInfo;
  compMsgField_t *tabFieldInfo;
  uint8_t *fieldNameStr;
  size_t headerLgth;
  size_t namesOffset;

  if ((self->flags & COMP_DEF_IS_INITTED) != 0) {
    return COMP_DEF_ERR_ALREADY_INITTED;
  }
  numFields = self->numFields + self->numTableRowFields;
  defsSize = numFields * (sizeof(uint16_t) + sizeof(uint16_t) * sizeof(uint16_t));
  id2offset_t normNamesOffsets[numFields];
  numNormFields = 0;
  normNamesSize = 0;
  namesOffset = 0;
  idx = 0;
  while (idx < self->numFields) {
    fieldInfo = &self->fields[idx];
    if (fieldInfo->fieldNameId < COMP_MSG_SPEC_FIELD_LOW) {
      result = self->compMsgDataView->getFieldNameStrFromId(self->compMsgDataView, fieldInfo->fieldNameId, &fieldNameStr);
      checkErrOK(result);
      normNamesOffsets[numNormFields].id = fieldInfo->fieldNameId;
      normNamesOffsets[numNormFields].offset = namesOffset;
      normNamesOffsets[numNormFields].name = fieldNameStr;
      normNamesSize += c_strlen(fieldNameStr) + 1;
      namesOffset += c_strlen(fieldNameStr) + 1;
      numNormFields++;
    } else {
      if (fieldInfo->fieldNameId == COMP_MSG_SPEC_FIELD_TABLE_ROW_FIELDS) {
        tabIdx = 0;
        while (tabIdx < self->numTableRowFields) {
          tabFieldInfo = &self->tableFields[tabIdx];
          result = self->compMsgDataView->getFieldNameStrFromId(self->compMsgDataView, tabFieldInfo->fieldNameId, &fieldNameStr);
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
  result = addDefFields(self, numNormFields, normNamesSize, defsSize, COMP_DEF_TO_DATA);
  checkErrOK(result);
  result = setStaticDefFields(self, numNormFields, normNamesSize, normNamesOffsets, defsSize);
  checkErrOK(result);
  return COMP_MSG_ERR_OK;
}

// ============================= prepareDefMsg ========================

static uint8_t prepareDefMsg(compMsgData_t *self) {
  int numEntries;
  int idx;
  int result;
  compMsgField_t *fieldInfo;

  if ((self->flags & COMP_DEF_IS_INITTED) == 0) {
ets_printf("prepareDef not yet initted\n");
    return COMP_DEF_ERR_NOT_YET_INITTED;
  }
  // create the values which are different for each message!!
  numEntries = self->numDefFields;
  idx = 0;
  while (idx < numEntries) {
    fieldInfo = &self->defFields[idx];
    switch (fieldInfo->fieldNameId) {
      case COMP_MSG_SPEC_FIELD_RANDOM_NUM:
        result = self->compMsgDefMsgDataView->setRandomNum(self->compMsgDefMsgDataView, fieldInfo);
        checkErrOK(result);
        fieldInfo->fieldFlags |= COMP_MSG_FIELD_IS_SET;
        break;
      case COMP_MSG_SPEC_FIELD_SEQUENCE_NUM:
        result = self->compMsgDefMsgDataView->setSequenceNum(self->compMsgDefMsgDataView, fieldInfo);
        checkErrOK(result);
        fieldInfo->fieldFlags |= COMP_MSG_FIELD_IS_SET;
        break;
      case COMP_MSG_SPEC_FIELD_FILLER:
        result = self->compMsgDefMsgDataView->setFiller(self->compMsgDefMsgDataView, fieldInfo);
        checkErrOK(result);
        fieldInfo->fieldFlags |= COMP_MSG_FIELD_IS_SET;
        break;
      case COMP_MSG_SPEC_FIELD_CRC:
        result = self->compMsgDefMsgDataView->setCrc(self->compMsgDefMsgDataView, fieldInfo, self->headerLgth+1, self->cmdLgth-fieldInfo->fieldLgth);
        checkErrOK(result);
        fieldInfo->fieldFlags |= COMP_MSG_FIELD_IS_SET;
        break;
    }
    idx++;
  }
  self->flags |= COMP_DEF_IS_PREPARED;
  return COMP_MSG_ERR_OK;
}


// ============================= setDefData ========================

static uint8_t setDefData(compMsgData_t *self, const  uint8_t *data) {
  int idx;
  int result;
  uint16_t lgth;
  size_t numNormFields;
  size_t normNamesSize;
  size_t defsSize;
  compMsgField_t *fieldInfo;

  if (self->compMsgDefMsgDataView == NULL) {
    result = newCompMsgDefMsg(self);
    checkErrOK(result);
  } else {
    if (self->compMsgDataView->dataView->data != NULL) {
      // free no longer used we cannot reuse as the size can be different!
      os_free(self->compMsgDataView->dataView->data);
      self->compMsgDataView->dataView->data = NULL;
      // we do net free self->defFields, we just reuse them it is always the same number of defFields
      os_free(self->defFields);
      self->defFields = NULL;
      self->numDefFields = 0;
      self->defFieldOffset = 0;
    }
  }
  // temporary replace data entry of dataView by our param data
  // to be able to use the get* functions for gettting totalLgth entry value
  self->compMsgDefMsgDataView->dataView->data = (uint8_t *)data;
  self->compMsgDefMsgDataView->dataView->lgth = 10;
  // get totalLgth value from data
  result = self->compMsgDefMsgDataView->dataView->getUint16(self->compMsgDefMsgDataView->dataView, 4, &lgth);
  checkErrOK(result);

//FIXME!! check crc!!

  // now make a copy of the data to be on the safe side
  // for freeing the Lua space in Lua set the variable to nil!!
  self->compMsgDefMsgDataView->dataView->data = os_zalloc(lgth);
  checkAllocOK(self->compMsgDefMsgDataView->dataView->data);
  c_memcpy(self->compMsgDefMsgDataView->dataView->data, data, lgth);
  self->compMsgDefMsgDataView->dataView->lgth = lgth;
  self->defTotalLgth = lgth;

  numNormFields = 0;
  normNamesSize = 0;
  defsSize = 0;
  result = addDefFields(self, numNormFields, normNamesSize, defsSize, COMP_DEF_FROM_DATA);
  checkErrOK(result);

  // and now set the IS_SET flags and other stuff
  self->flags |= COMP_DEF_IS_INITTED;
  idx = 0;
  while (idx < self->numDefFields) {
    fieldInfo = &self->defFields[idx];
    fieldInfo->fieldFlags |= COMP_MSG_FIELD_IS_SET;
    idx++;
  }
  self->flags |= COMP_DEF_IS_PREPARED;
  return COMP_MSG_ERR_OK;
}

// ============================= getDefData ========================

static uint8_t getDefData(compMsgData_t *self, uint8_t **data, int *lgth) {
  if ((self->flags & COMP_DEF_IS_INITTED) == 0) {
    return COMP_DEF_ERR_NOT_YET_INITTED;
  }
  if ((self->flags & COMP_DEF_IS_PREPARED) == 0) {
    return COMP_DEF_ERR_NOT_YET_PREPARED;
  }
  *data = self->compMsgDefMsgDataView->dataView->data;
  *lgth = self->defTotalLgth;
  return COMP_MSG_ERR_OK;
}

// ============================= createMsgFromDef ========================

static uint8_t createMsgFromDef(compMsgData_t *self) {
  compMsgField_t *fieldInfo;
  uint8_t *fieldNameStr;
  uint8_t *fieldTypeStr;
  uint16_t fieldNameId;
  uint16_t fieldTypeId;
  uint16_t fieldLgth;
  uint8_t *handle;
  uint8_t defsIdx;
  uint8_t numNormFlds;
  uint16_t normFldNamesSize;
  uint16_t defsSize;
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
    case COMP_MSG_SPEC_FIELD_SRC:
    case COMP_MSG_SPEC_FIELD_DST:
    case COMP_MSG_SPEC_FIELD_TARGET_CMD:
    case COMP_MSG_SPEC_FIELD_TOTAL_LGTH:
    case COMP_MSG_SPEC_FIELD_CMD_KEY:
    case COMP_MSG_SPEC_FIELD_CMD_LGTH:
    case COMP_MSG_SPEC_FIELD_RANDOM_NUM:
    case COMP_MSG_SPEC_FIELD_FILLER:
    case COMP_MSG_SPEC_FIELD_CRC:
      // nothing to do!
      break;
    case COMP_MSG_SPEC_FIELD_NUM_NORM_FLDS:
      result = self->compMsgDefMsgDataView->dataView->getUint8(self->compMsgDefMsgDataView->dataView, fieldInfo->fieldOffset, &numNormFlds);
      checkErrOK(result);
      break;
    case COMP_MSG_SPEC_FIELD_NORM_FLD_IDS:
      idx = 0;
      while (idx < numNormFlds) {
        result = self->compMsgDefMsgDataView->dataView->getUint16(self->compMsgDefMsgDataView->dataView, fieldInfo->fieldOffset+idx*sizeof(uint16_t), &val);
        checkErrOK(result);
        normNamesOffsets[idx].offset = val;
        normNamesOffsets[idx].name = names+val;
        idx++;
      }
      break;
    case COMP_MSG_SPEC_FIELD_NORM_FLD_NAMES_SIZE:
      result = self->compMsgDefMsgDataView->dataView->getUint16(self->compMsgDefMsgDataView->dataView, fieldInfo->fieldOffset, &normFldNamesSize);
      checkErrOK(result);
      break;
    case COMP_MSG_SPEC_FIELD_NORM_FLD_NAMES:
      result = self->compMsgDefMsgDataView->dataView->getUint8Vector(self->compMsgDefMsgDataView->dataView, fieldInfo->fieldOffset, &names, fieldInfo->fieldLgth);
      checkErrOK(result);
      break;
    case COMP_MSG_SPEC_FIELD_DEFINITIONS_SIZE:
      result = self->compMsgDefMsgDataView->dataView->getUint16(self->compMsgDefMsgDataView->dataView, fieldInfo->fieldOffset, &defsSize);
      checkErrOK(result);
      break;
    case COMP_MSG_SPEC_FIELD_DEFINITIONS:
      idx = 0;
      namesIdx = 0;
      while (idx < defsSize / sizeof(uint16_t)) {
        result = self->compMsgDefMsgDataView->dataView->getUint16(self->compMsgDefMsgDataView->dataView, fieldInfo->fieldOffset+(idx++*sizeof(uint16_t)), &fieldNameId);
        checkErrOK(result);
        if (fieldNameId > COMP_MSG_SPEC_FIELD_LOW) {
          result = self->compMsgDataView->getFieldNameStrFromId(self->compMsgDataView, fieldNameId, &fieldNameStr);
          checkErrOK(result);
        } else {
          fieldNameStr = normNamesOffsets[namesIdx++].name;
          checkErrOK(result);
        }
        result = self->compMsgDefMsgDataView->dataView->getUint16(self->compMsgDefMsgDataView->dataView, fieldInfo->fieldOffset+(idx++*sizeof(uint16_t)), &fieldTypeId);
        checkErrOK(result);
        result = self->compMsgDataView->dataView->getFieldTypeStrFromId(self->compMsgDataView->dataView, fieldTypeId, &fieldTypeStr);
        checkErrOK(result);
        result = self->compMsgDefMsgDataView->dataView->getUint16(self->compMsgDefMsgDataView->dataView, fieldInfo->fieldOffset+(idx++*sizeof(uint16_t)), &fieldLgth);
        checkErrOK(result);
        result = self->addField(self, fieldNameStr, fieldTypeStr, fieldLgth);
        checkErrOK(result);
      }
      break;
      
    }
    fieldIdx++;
  }
  result = self->initMsg(self);

  return COMP_MSG_ERR_OK;
}

// ============================= newCompMsgDefMsg ========================

uint8_t newCompMsgDefMsg(compMsgData_t *self) {
  if (self->compMsgDefMsgDataView != NULL) {
    return COMP_DEF_ERR_ALREADY_CREATED; 
  }
  self->compMsgDefMsgDataView = newCompMsgDataView();
  if (self->compMsgDefMsgDataView == NULL) {
    return COMP_MSG_ERR_OUT_OF_MEMORY;
  }
  self->defFields = os_zalloc(COMP_DEF_NUM_DEF_FIELDS * sizeof(compMsgField_t));
  if (self->defFields == NULL) {
    return COMP_MSG_ERR_OUT_OF_MEMORY;
  }

  self->initDefMsg = &initDefMsg;
  self->prepareDefMsg = &prepareDefMsg;
  self->addDefField = &addDefField;
  self->dumpDefFields = &dumpDefFields;
  self->setDefFieldValue = &setDefFieldValue;
  self->getDefFieldValue = &getDefFieldValue;
  self->setDefData = &setDefData;
  self->getDefData = &getDefData;
  self->createMsgFromDef = &createMsgFromDef;

  return COMP_MSG_ERR_OK;
}

