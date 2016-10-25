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

// ============================= addListMsg ========================

static uint8_t addListMsg(structmsgData_t *self, size_t msgLgth, uint8_t *msgData) {

  return STRUCT_MSG_ERR_OK;
}

  structmsgField_t *listFieldInfo;
// ============================= addListField ========================

static uint8_t addListField(structmsgData_t *self, uint8_t fieldNameId, uint8_t fieldTypeId, uint8_t fieldLgth) {
  structmsgField_t *listFieldInfo;

  if (self->numListMsgs >= STRUCT_LIST_NUM_LIST_FIELDS) {
    return STRUCT_MSG_ERR_TOO_MANY_FIELDS;
  }
  listFieldInfo = &self->listFields[self->numListMsgs];
  listFieldInfo->fieldNameId = fieldNameId;
  listFieldInfo->fieldTypeId = fieldTypeId;
  listFieldInfo->fieldLgth = fieldLgth;
  listFieldInfo->fieldOffset = self->listFieldOffset;
  self->listFieldOffset += fieldLgth;
  self->numListMsgs++;
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
  numEntries = self->numListMsgs;
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

static uint8_t addListFields(structmsgData_t *self, uint16_t numNormFields, uint16_t normNamesSize, uint16_t listSize, int direction) {
  int fieldIdx;
  int namesIdx;
  int result;
  int idx;
  uint8_t numNormEntries;
  size_t crcLgth;
  size_t myLgth;
  uint16_t fillerLgth;
  size_t namesOffset;

  result = addListField(self, STRUCT_MSG_SPEC_FIELD_DST, DATA_VIEW_FIELD_UINT16_T, 2);
  checkErrOK(result);
  result = addListField(self, STRUCT_MSG_SPEC_FIELD_SRC, DATA_VIEW_FIELD_UINT16_T, 2);
  checkErrOK(result);
  result = addListField(self, STRUCT_MSG_SPEC_FIELD_TOTAL_LGTH, DATA_VIEW_FIELD_UINT16_T, 2);
  checkErrOK(result);
  self->listHeaderLgth = self->listFieldOffset;
  result = addListField(self, STRUCT_MSG_SPEC_FIELD_CMD_KEY, DATA_VIEW_FIELD_UINT16_T, 2);
  checkErrOK(result);
  result = addListField(self, STRUCT_MSG_SPEC_FIELD_CRC, DATA_VIEW_FIELD_UINT16_T, 2);
  checkErrOK(result);
  self->flags |= STRUCT_LIST_IS_INITTED;
  if (direction == STRUCT_DEF_TO_DATA) {
    self->structmsgListDataView->dataView->data = os_zalloc(self->listFieldOffset);
    checkAllocOK(self->structmsgListDataView->dataView->data);
  }
  self->structmsgListDataView->dataView->lgth = self->listFieldOffset;
  self->listTotalLgth = self->listFieldOffset;
  return STRUCT_MSG_ERR_OK;
}

// ============================= setStaticListFields ========================

static uint8_t setStaticListFields(structmsgData_t *self, int numNormFields, int normNamesSize, id2offset_t *normNamesOffsets, int listSize) {
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
  result = setListFieldValue(self, STRUCT_MSG_SPEC_FIELD_DST, 16640, NULL, 0);
  checkErrOK(result);
  result = setListFieldValue(self, STRUCT_MSG_SPEC_FIELD_SRC, 22272, NULL, 0);
  checkErrOK(result);
  result = setListFieldValue(self, STRUCT_MSG_SPEC_FIELD_TOTAL_LGTH, self->listFieldOffset, NULL, 0);
  checkErrOK(result);
  result = setListFieldValue(self, STRUCT_MSG_SPEC_FIELD_CMD_KEY, STRUCT_LIST_CMD_KEY, NULL, 0);
  checkErrOK(result);
  return STRUCT_MSG_ERR_OK;
}

// ============================= initList ========================

static uint8_t initList(structmsgData_t *self) {
  uint8_t numNormFields;
  uint16_t normNamesSize;
  int idx;
  int tabIdx;
  int result;
  int numFields;
  size_t listSize;
  structmsgField_t *fieldInfo;
  structmsgField_t *tabFieldInfo;
  uint8_t *fieldNameStr;
  size_t headerLgth;
  size_t namesOffset;

ets_printf("initList called\n");
  if ((self->flags & STRUCT_LIST_IS_INITTED) != 0) {
    return STRUCT_LIST_ERR_ALREADY_INITTED;
  }
#ifdef NOTDEF
  numFields = self->numFields + self->numTableRowFields;
  listSize = numFields * (sizeof(uint16_t) + sizeof(uint16_t) * sizeof(uint16_t));
  id2offset_t normNamesOffsets[numFields];
  result = addListFields(self, numNormFields, normNamesSize, listSize, STRUCT_DEF_TO_DATA);
  checkErrOK(result);
  result = setStaticListFields(self, numNormFields, normNamesSize, normNamesOffsets, listSize);
  checkErrOK(result);
#endif
ets_printf("initList done\n");
  return STRUCT_MSG_ERR_OK;
}

// ============================= prepareList ========================

static uint8_t prepareList(structmsgData_t *self) {
  int numEntries;
  int idx;
  int result;
  structmsgField_t *fieldInfo;

  if ((self->flags & STRUCT_LIST_IS_INITTED) == 0) {
ets_printf("prepareList not yet initted\n");
    return STRUCT_LIST_ERR_NOT_YET_INITTED;
  }
  // create the values which are different for each message!!
  numEntries = self->numListMsgs;
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
  size_t numNormFields;
  size_t normNamesSize;
  size_t listSize;
  structmsgField_t *fieldInfo;

  if (self->structmsgListDataView == NULL) {
    result = newStructmsgList(self);
    checkErrOK(result);
  } else {
    if (self->structmsgDataView->dataView->data != NULL) {
      // free no longer used we cannot reuse as the size can be different!
      os_free(self->structmsgDataView->dataView->data);
      self->structmsgDataView->dataView->data = NULL;
      // we do net free self->listFields, we just reuse them it is always the same number of listFields
      os_free(self->listFields);
      self->listFields = NULL;
      self->numListMsgs = 0;
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

  numNormFields = 0;
  normNamesSize = 0;
  listSize = 0;
  result = addListFields(self, numNormFields, normNamesSize, listSize, STRUCT_DEF_FROM_DATA);
  checkErrOK(result);

  // and now set the IS_SET flags and other stuff
  self->flags |= STRUCT_LIST_IS_INITTED;
  idx = 0;
  while (idx < self->numListMsgs) {
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
  uint8_t *fieldNameStr;
  uint8_t *fieldTypeStr;
  uint16_t fieldNameId;
  uint16_t fieldTypeId;
  uint16_t fieldLgth;
  uint8_t *handle;
  uint8_t listIdx;
  uint8_t numNormFlds;
  uint16_t normFldNamesSize;
  uint16_t listSize;
  uint16_t val;
  int namesIdx;
  int fieldIdx;
  int idx;
  int result;

#ifdef NOTDEF
  fieldIdx = 0;
  while (fieldIdx < self->numListMsgs) {
    fieldInfo = &self->listFields[fieldIdx];
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
      result = self->structmsgListDataView->dataView->getUint8(self->structmsgListDataView->dataView, fieldInfo->fieldOffset, &numNormFlds);
      checkErrOK(result);
      break;
    }
    fieldIdx++;
  }
#endif
  result = self->initMsg(self);
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

  self->initList = &initList;
  self->prepareList = &prepareList;
  self->addListMsg = &addListMsg;
  self->addListField = &addListField;
//  self->setListFieldValue = &setListFieldValue;
//  self->getListFieldValue = &getListFieldValue;
//  self->setListData = &setListData;
//  self->getListData = &getListData;
//  self->createMsgFromList = &createMsgFromList;

  return STRUCT_MSG_ERR_OK;
}

