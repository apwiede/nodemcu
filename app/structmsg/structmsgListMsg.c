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

// ================================= dumpListMsgs ====================================

static uint8_t dumpListMsgs(structmsgData_t *self, structmsgField_t *fieldInfo, const uint8_t *indent2, int fieldIdx, uint8_t *names) {
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
  // should be a binary dump here ??
//  ets_printf("        listIdx: %3d fieldName: %3d %-20s fieldType: %3d %-8s fieldLgth: %5d\r\n", valueIdx/3, fieldNameId, fieldNameStr, fieldTypeId, fieldTypeStr, fieldLgth);
  return DATA_VIEW_ERR_OK;
}

// ================================= dumpListFieldValue ====================================

static uint8_t dumpListFieldValue(structmsgData_t *self, structmsgField_t *fieldInfo, const uint8_t *indent2, int fieldIdx) {
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

  result = self->structmsgListDataView->getFieldValue(self->structmsgListDataView, fieldInfo, &numericValue, &stringValue, fieldIdx);
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
      result = self->structmsgListDataView->dataView->getInt16(self->structmsgListDataView->dataView, fieldInfo->fieldOffset+valueIdx*sizeof(int16_t), &sh);
      checkErrOK(result);
      ets_printf("        %sidx: %d value: %d 0x%04x\n", indent2, valueIdx, sh, sh);
      valueIdx++;
    }
    ets_printf("\n");
    break;
  case DATA_VIEW_FIELD_UINT16_VECTOR:
    valueIdx = 0;
    ets_printf("      %svalues:\n", indent2);
    while (valueIdx < fieldInfo->fieldLgth/sizeof(uint16_t)) {
      result = self->structmsgListDataView->getFieldValue(self->structmsgListDataView, fieldInfo, &numericValue, &stringValue, valueIdx);
      checkErrOK(result);
      ets_printf("        %sidx: %d value: %d 0x%04x\n", indent2, valueIdx, numericValue, (uint16_t)(numericValue & 0xFFFF));
      valueIdx++;
    }
    break;
#ifdef NOTDEFg
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

// ============================= dumpListFields ========================

static uint8_t dumpListFields(structmsgData_t *self) {
  int numEntries;
  int idx;
  int result;
  uint8_t *fieldTypeStr;
  uint8_t *fieldNameStr;
  structmsgField_t *fieldInfo;
  uint8_t *stringValue;

  numEntries = self->numListFields;
  ets_printf("    numListFields: %d\r\n", numEntries);
  idx = 0;
  while (idx < numEntries) {
    fieldInfo = &self->listFields[idx];
    result = self->structmsgDataView->dataView->getFieldTypeStrFromId(self->structmsgDataView->dataView, fieldInfo->fieldTypeId, &fieldTypeStr);
    checkErrOK(result);
    result = self->structmsgDataView->getFieldNameStrFromId(self->structmsgDataView, fieldInfo->fieldNameId, &fieldNameStr);
    checkErrOK(result);
    ets_printf("      idx: %d fieldName: %-20s fieldType: %-8s fieldLgth: %.5d offset: %d \r\n", idx, fieldNameStr, fieldTypeStr, fieldInfo->fieldLgth, fieldInfo->fieldOffset);
    if (fieldInfo->fieldFlags & STRUCT_MSG_FIELD_IS_SET) {
      if (fieldInfo->fieldNameId == STRUCT_MSG_SPEC_FIELD_LIST_MSGS) {
        result = dumpListMsgs(self, fieldInfo, "  ", 0, stringValue);
        checkErrOK(result);
      } else {
        result = dumpListFieldValue(self, fieldInfo, "  ", 0);
        checkErrOK(result);
      }
    }
    idx++;
  }
  return STRUCT_MSG_ERR_OK;
}

// ============================= addListMsg ========================

static uint8_t addListMsg(structmsgData_t *self, size_t msgLgth, uint8_t *msgData) {

  return STRUCT_MSG_ERR_OK;
}

// ============================= addListField ========================

static uint8_t addListField(structmsgData_t *self, uint8_t fieldNameId, uint8_t fieldTypeId, uint8_t fieldLgth) {
  structmsgField_t *listFieldInfo;

  if (self->numListFields >= STRUCT_LIST_NUM_LIST_FIELDS) {
    return STRUCT_MSG_ERR_TOO_MANY_FIELDS;
  }
  listFieldInfo = &self->listFields[self->numListFields];
  listFieldInfo->fieldNameId = fieldNameId;
  listFieldInfo->fieldTypeId = fieldTypeId;
  listFieldInfo->fieldLgth = fieldLgth;
  listFieldInfo->fieldOffset = self->listFieldOffset;
  self->listFieldOffset += fieldLgth;
  self->numListFields++;
  return STRUCT_MSG_ERR_OK;
}

// ============================= getListFieldValue ========================

static uint8_t getListFieldValue(structmsgData_t *self, uint8_t fieldNameId, int *numericValue, uint8_t **stringValue, int fieldIdx) {
  if ((self->flags & STRUCT_LIST_IS_INITTED) == 0) {
    return STRUCT_LIST_ERR_NOT_YET_INITTED;
  }
  return STRUCT_MSG_ERR_OK;
}

// ============================= setListFieldValue ========================

static uint8_t setListFieldValue(structmsgData_t *self, uint8_t fieldNameId, int numericValue, const uint8_t *stringValue, int fieldIdx) {
  int result;
  int idx;
  int numEntries;
  bool found = false;
  structmsgField_t *fieldInfo;

  if ((self->flags & STRUCT_LIST_IS_INITTED) == 0) {
    return STRUCT_LIST_ERR_NOT_YET_INITTED;
  }
  idx = 0;
  numEntries = self->numListFields;
  while (idx < numEntries) {
    fieldInfo = &self->listFields[idx];
    if (fieldNameId == fieldInfo->fieldNameId) {
      result = self->structmsgListDataView->setFieldValue(self->structmsgListDataView, fieldInfo, numericValue, stringValue, fieldIdx);
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

// ============================= addListFields ========================

static uint8_t addListFields(structmsgData_t *self, int direction) {
  int fieldIdx;
  int namesIdx;
  int result;
  int idx;
  size_t crcLgth;
  size_t myLgth;
  uint16_t fillerLgth;
  uint8_t numListMsgs;
  uint16_t msgSize;
  uint16_t *listMsgSizes;

  listMsgSizes = NULL;
  result = addListField(self, STRUCT_MSG_SPEC_FIELD_DST, DATA_VIEW_FIELD_UINT16_T, 2);
  checkErrOK(result);
  result = addListField(self, STRUCT_MSG_SPEC_FIELD_SRC, DATA_VIEW_FIELD_UINT16_T, 2);
  checkErrOK(result);
  result = addListField(self, STRUCT_MSG_SPEC_FIELD_TOTAL_LGTH, DATA_VIEW_FIELD_UINT16_T, 2);
  checkErrOK(result);
  self->listHeaderLgth = self->listFieldOffset;
  result = addListField(self, STRUCT_MSG_SPEC_FIELD_CMD_KEY, DATA_VIEW_FIELD_UINT16_T, 2);
  checkErrOK(result);
  self->listHeaderLgth = self->listFieldOffset;
  if (direction == STRUCT_LIST_FROM_DATA) {
    result = self->structmsgListDataView->dataView->getUint8(self->structmsgListDataView->dataView, self->listFieldOffset, &numListMsgs);
    checkErrOK(result);
    self->numListMsgs = numListMsgs;
  }
  result = addListField(self, STRUCT_MSG_SPEC_FIELD_NUM_LIST_MSGS, DATA_VIEW_FIELD_UINT8_T, 1);
  checkErrOK(result);
  result = addListField(self, STRUCT_MSG_SPEC_FIELD_LIST_MSG_SIZES, DATA_VIEW_FIELD_UINT16_VECTOR, self->numListMsgs*sizeof(uint16));
  checkErrOK(result);
  if (direction == STRUCT_LIST_FROM_DATA) {
    listMsgSizes = os_zalloc (self->numListMsgs * sizeof(uint16_t));
    idx = 0;
    self->listMsgsSize = 0;
    while (idx < self->numListMsgs) {
      result = self->structmsgListDataView->dataView->getUint16(self->structmsgListDataView->dataView, self->listFieldOffset, &msgSize);
      checkErrOK(result);
      listMsgSizes[idx] = msgSize;
      self->listMsgsSize += msgSize;
      idx++;
    }
  }
  result = addListField(self, STRUCT_MSG_SPEC_FIELD_LIST_MSGS, DATA_VIEW_FIELD_UINT8_VECTOR, self->listMsgsSize);
  checkErrOK(result);
  fillerLgth = 0;
  crcLgth = 2;
  myLgth = self->listFieldOffset + crcLgth - self->listHeaderLgth;
  while ((myLgth % 16) != 0) {
    myLgth++;
    fillerLgth++;
  }
  result = addListField(self, STRUCT_MSG_SPEC_FIELD_FILLER, DATA_VIEW_FIELD_UINT8_VECTOR, fillerLgth);
  checkErrOK(result);
  result = addListField(self, STRUCT_MSG_SPEC_FIELD_CRC, DATA_VIEW_FIELD_UINT16_T, 2);
  checkErrOK(result);
  self->flags |= STRUCT_LIST_IS_INITTED;
  if (direction == STRUCT_LIST_TO_DATA) {
    self->structmsgListDataView->dataView->data = os_zalloc(self->listFieldOffset);
    checkAllocOK(self->structmsgListDataView->dataView->data);
  }
  self->structmsgListDataView->dataView->lgth = self->listFieldOffset;
  self->listTotalLgth = self->listFieldOffset;
ets_printf("addListFields done\n");
  return STRUCT_MSG_ERR_OK;
}

// ============================= setStaticListFields ========================

static uint8_t setStaticListFields(structmsgData_t *self) {
  int fieldIdx;
  int result;
  int idx;
  size_t headerLgth;
  size_t fieldLgth;
  structmsgField_t *fieldInfo;

  result = setListFieldValue(self, STRUCT_MSG_SPEC_FIELD_DST, self->listDst, NULL, 0);
  checkErrOK(result);
  result = setListFieldValue(self, STRUCT_MSG_SPEC_FIELD_SRC, self->listSrc, NULL, 0);
  checkErrOK(result);
  result = setListFieldValue(self, STRUCT_MSG_SPEC_FIELD_TOTAL_LGTH, self->listTotalLgth, NULL, 0);
  checkErrOK(result);
  result = setListFieldValue(self, STRUCT_MSG_SPEC_FIELD_CMD_KEY, STRUCT_LIST_CMD_KEY, NULL, 0);
  checkErrOK(result);
  return STRUCT_MSG_ERR_OK;
}

// ============================= initListMsg ========================

static uint8_t initListMsg(structmsgData_t *self) {
  int result;

ets_printf("initList called\n");
  if ((self->flags & STRUCT_LIST_IS_INITTED) != 0) {
    return STRUCT_LIST_ERR_ALREADY_INITTED;
  }
  result = addListFields(self, STRUCT_LIST_TO_DATA);
  checkErrOK(result);
  result = setStaticListFields(self);
  checkErrOK(result);
ets_printf("initList done\n");
  return STRUCT_MSG_ERR_OK;
}

// ============================= prepareListMsg ========================

static uint8_t prepareListMsg(structmsgData_t *self) {
  int numEntries;
  int idx;
  int result;
  structmsgField_t *fieldInfo;

  if ((self->flags & STRUCT_LIST_IS_INITTED) == 0) {
    return STRUCT_LIST_ERR_NOT_YET_INITTED;
  }
  // create the values which are different for each message!!
  numEntries = self->numListFields;
  idx = 0;
  while (idx < numEntries) {
    fieldInfo = &self->listFields[idx];
    switch (fieldInfo->fieldNameId) {
      case STRUCT_MSG_SPEC_FIELD_RANDOM_NUM:
        result = self->structmsgListDataView->setRandomNum(self->structmsgListDataView, fieldInfo);
        checkErrOK(result);
        fieldInfo->fieldFlags |= STRUCT_MSG_FIELD_IS_SET;
        break;
      case STRUCT_MSG_SPEC_FIELD_SEQUENCE_NUM:
        result = self->structmsgListDataView->setSequenceNum(self->structmsgListDataView, fieldInfo);
        checkErrOK(result);
        fieldInfo->fieldFlags |= STRUCT_MSG_FIELD_IS_SET;
        break;
      case STRUCT_MSG_SPEC_FIELD_FILLER:
        result = self->structmsgListDataView->setFiller(self->structmsgListDataView, fieldInfo);
        checkErrOK(result);
        fieldInfo->fieldFlags |= STRUCT_MSG_FIELD_IS_SET;
        break;
      case STRUCT_MSG_SPEC_FIELD_CRC:
        result = self->structmsgListDataView->setCrc(self->structmsgListDataView, fieldInfo, self->headerLgth+1, self->cmdLgth-fieldInfo->fieldLgth);
        checkErrOK(result);
        fieldInfo->fieldFlags |= STRUCT_MSG_FIELD_IS_SET;
        break;
    }
    idx++;
  }
  self->flags |= STRUCT_LIST_IS_PREPARED;
  return STRUCT_MSG_ERR_OK;
}


// ============================= setListData ========================

static uint8_t setListData(structmsgData_t *self, const  uint8_t *data) {
  int idx;
  int result;
  uint16_t lgth;
  structmsgField_t *fieldInfo;

  if (self->structmsgListDataView == NULL) {
    result = newStructmsgList(self);
    checkErrOK(result);
  } else {
    if (self->structmsgDataView->dataView->data != NULL) {
      // free no longer used we cannot reuse as the size can be different!
      os_free(self->structmsgDataView->dataView->data);
      self->structmsgDataView->dataView->data = NULL;
      os_free(self->listFields);
      self->listFields = NULL;
      self->numListMsgs = 0;
      self->listMsgSizesSize = 0;
      self->listMsgsSize = 0;
      self->listFieldOffset = 0;
    }
  }
  // temporary replace data entry of dataView by our param data
  // to be able to use the get* functions for gettting totalLgth entry value
  self->structmsgListDataView->dataView->data = (uint8_t *)data;
  self->structmsgListDataView->dataView->lgth = 10;
  // get totalLgth value from data
  result = self->structmsgListDataView->dataView->getUint16(self->structmsgListDataView->dataView, 4, &lgth);
  checkErrOK(result);

//FIXME!! check crc!!

  // now make a copy of the data to be on the safe side
  // for freeing the Lua space in Lua set the variable to nil!!
  self->structmsgListDataView->dataView->data = os_zalloc(lgth);
  checkAllocOK(self->structmsgListDataView->dataView->data);
  c_memcpy(self->structmsgListDataView->dataView->data, data, lgth);
  self->structmsgListDataView->dataView->lgth = lgth;
  self->listTotalLgth = lgth;

  result = addListFields(self, STRUCT_LIST_FROM_DATA);
  checkErrOK(result);

  // and now set the IS_SET flags and other stuff
  self->flags |= STRUCT_LIST_IS_INITTED;
  idx = 0;
  while (idx < self->numListFields) {
    fieldInfo = &self->listFields[idx];
    fieldInfo->fieldFlags |= STRUCT_MSG_FIELD_IS_SET;
    idx++;
  }
  self->flags |= STRUCT_LIST_IS_PREPARED;
  return STRUCT_MSG_ERR_OK;
}

// ============================= getListData ========================

static uint8_t getListData(structmsgData_t *self, uint8_t **data, int *lgth) {
  if ((self->flags & STRUCT_LIST_IS_INITTED) == 0) {
    return STRUCT_LIST_ERR_NOT_YET_INITTED;
  }
  if ((self->flags & STRUCT_LIST_IS_PREPARED) == 0) {
    return STRUCT_LIST_ERR_NOT_YET_PREPARED;
  }
  *data = self->structmsgListDataView->dataView->data;
  *lgth = self->listTotalLgth;
  return STRUCT_MSG_ERR_OK;
}

// ============================= createMsgFromList ========================

static uint8_t createMsgFromList(structmsgData_t *self) {
  structmsgField_t *fieldInfo;
  int result;

   self->numListMsgs = 0;
   self->listMsgSizesSize = 0;
   self->listMsgsSize = 0;
//    result = self->structmsgData->initListMsg(self->structmsgData);
//    checkErrOK(result);
  return STRUCT_MSG_ERR_OK;
}

// ============================= newStructmsgList ========================

uint8_t newStructmsgList(structmsgData_t *self) {
ets_printf("newStructmsgList called\n");
  if (self->structmsgListDataView != NULL) {
    return STRUCT_LIST_ERR_ALREADY_CREATED; 
  }
  self->structmsgListDataView = newStructmsgDataView();
  if (self->structmsgListDataView == NULL) {
    return STRUCT_MSG_ERR_OUT_OF_MEMORY;
  }
  self->listFields = os_zalloc(STRUCT_LIST_NUM_LIST_FIELDS * sizeof(structmsgField_t));
  if (self->listFields == NULL) {
    return STRUCT_MSG_ERR_OUT_OF_MEMORY;
  }

  self->numListFields = 0;
  self->numListMsgs = 0;
  self->listMsgSizesSize = 0;
  self->listMsgsSize = 0;
  self->listFieldOffset = 0;
  self->listTotalLgth = 0;
  self->listHeaderLgth = 0;

  self->dumpListFields = &dumpListFields;
  self->addListMsg = &addListMsg;
  self->addListField = &addListField;
  self->setListFieldValue = &setListFieldValue;
  self->getListFieldValue = &getListFieldValue;
  self->setListData = &setListData;
  self->getListData = &getListData;
  self->initListMsg = &initListMsg;
  self->prepareListMsg = &prepareListMsg;
//  self->createMsgFromList = &createMsgFromList;

ets_printf("newStructmsgList done\n");
  return STRUCT_MSG_ERR_OK;
}

