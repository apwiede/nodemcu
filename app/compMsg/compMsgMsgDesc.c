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
 * File:   compMsgMsgDesc.c
 * Author: Arnulf P. Wiedemann
 *
 * Created on October 1st, 2016
 */

#include "osapi.h"
#include "c_types.h"
#include "mem.h"
#include "flash_fs.h"

#include "c_string.h"
#include "c_stdio.h"
#include "c_stdlib.h"
#include "compMsgDispatcher.h"

typedef struct flag2Str {
  uint32_t flagVal;
  uint8_t *flagStr;
} flag2Str_t;

static flag2Str_t flag2Strs [] = {
  { COMP_MSG_DST_U8,           "COMP_MSG_DST_U8" },
  { COMP_MSG_DST_U16,          "COMP_MSG_DST_U16" },
  { COMP_MSG_SRC_U8,           "COMP_MSG_SRC_U8" },
  { COMP_MSG_SRC_U16,          "COMP_MSG_SRC_U16" },
  { COMP_MSG_TOTAL_LGTH_U8,    "COMP_MSG_TOTAL_LGTH_U8" },
  { COMP_MSG_TOTAL_LGTH_U16,   "COMP_MSG_TOTAL_LGTH_U16" },
  { COMP_MSG_GUID_VECTOR,      "COMP_MSG_GUID_VECTOR" },
  { COMP_MSG_IP_ADDR,          "COMP_MSG_IP_ADDR" },
//  { COMP_MSG_IS_ENCRYPTED,     "COMP_MSG_IS_ENCRYPTED" },
//  { COMP_MSG_IS_NOT_ENCRYPTED, "COMP_MSG_IS_NOT_ENCRYPTED" },
  { COMP_MSG_CMD_KEY_U8,       "COMP_MSG_CMD_KEY_U8" },
  { COMP_MSG_CMD_KEY_U16,      "COMP_MSG_CMD_KEY_U16" },
  { COMP_MSG_CMD_LGTH_U0,      "COMP_MSG_CMD_LGTH_U0" },
  { COMP_MSG_CMD_LGTH_U8,      "COMP_MSG_CMD_LGTH_U8" },
  { COMP_MSG_CMD_LGTH_U16,     "COMP_MSG_CMD_LGTH_U16" },
  { COMP_MSG_CRC_U0,           "COMP_MSG_CRC_U0" },
  { COMP_MSG_CRC_U8,           "COMP_MSG_CRC_U8" },
  { COMP_MSG_CRC_U16,          "COMP_MSG_CRC_U16" },
  { COMP_MSG_TOTAL_CRC_U0,     "COMP_MSG_TOTAL_CRC_U0" },
  { COMP_MSG_TOTAL_CRC_U8,     "COMP_MSG_TOTAL_CRC_U8" },
  { COMP_MSG_TOTAL_CRC_U16,    "COMP_MSG_TOTAL_CRC_U16" },
  { 0,                          NULL },
};

// ================================= addHeaderInfo ====================================

static uint8_t addHeaderInfo(compMsgDispatcher_t *self, uint16_t fieldLgth, uint8_t fieldId) {
  uint8_t result;
  msgHeaderInfo_t *msgHeaderInfo;

  result = COMP_MSG_ERR_OK;
  msgHeaderInfo = &self->compMsgMsgDesc->msgHeaderInfo;
  if (msgHeaderInfo->numHeaderFields == 0) {
    msgHeaderInfo->headerFieldIds = (uint8_t *)os_zalloc(((msgHeaderInfo->numHeaderFields + 1) * sizeof(uint8_t)));
  } else {
    msgHeaderInfo->headerFieldIds = (uint8_t *)os_realloc(msgHeaderInfo->headerFieldIds, ((msgHeaderInfo->numHeaderFields + 1) * sizeof(uint8_t)));
  }
  checkAllocOK(msgHeaderInfo->headerFieldIds);
  msgHeaderInfo->headerLgth += fieldLgth;
  msgHeaderInfo->headerFieldIds[msgHeaderInfo->numHeaderFields] = fieldId;
  msgHeaderInfo->numHeaderFields++;
COMP_MSG_DBG(self, "Y", 0, "addHeaderInfo: %d: fieldLgth: %d headerLgth: %d fieldId: %d", msgHeaderInfo->numHeaderFields, fieldLgth, msgHeaderInfo->headerLgth, fieldId);

  return result;
}

// ================================= addFieldGroup ====================================

static uint8_t addFieldGroup(compMsgDispatcher_t *self, char *fileName, uint16_t fieldGroupId, uint16_t cmdKey) {
  uint8_t result;
  msgFieldGroupInfo_t *msgFieldGroupInfo;

COMP_MSG_DBG(self, "E", 0, "addFieldGroup: %s!fieldGroupId: %d", fileName, fieldGroupId);
  result = COMP_MSG_ERR_OK;
  if (self->compMsgMsgDesc->numMsgFieldGroupInfo >= self->compMsgMsgDesc->maxMsgFieldGroupInfo) {
    if (self->compMsgMsgDesc->maxMsgFieldGroupInfo == 0) {
      self->compMsgMsgDesc->maxMsgFieldGroupInfo = 1;
      self->compMsgMsgDesc->msgFieldGroupInfos = (msgFieldGroupInfo_t *)os_zalloc((self->compMsgMsgDesc->maxMsgFieldGroupInfo * sizeof(msgFieldGroupInfo_t)));
      checkAllocOK(self->compMsgMsgDesc->msgFieldGroupInfos);
    } else {
      self->compMsgMsgDesc->maxMsgFieldGroupInfo += 1;
      self->compMsgMsgDesc->msgFieldGroupInfos = (msgFieldGroupInfo_t *)os_realloc((self->compMsgMsgDesc->msgFieldGroupInfos), (self->compMsgMsgDesc->maxMsgFieldGroupInfo * sizeof(msgFieldGroupInfo_t)));
      checkAllocOK(self->compMsgMsgDesc->msgFieldGroupInfos);
    }
  }
  msgFieldGroupInfo = &self->compMsgMsgDesc->msgFieldGroupInfos[self->compMsgMsgDesc->numMsgFieldGroupInfo];
  memset(msgFieldGroupInfo, 0, sizeof(msgFieldGroupInfo_t));
  msgFieldGroupInfo->fileName = os_zalloc(c_strlen(fileName) + 1);
  msgFieldGroupInfo->fieldGroupId = fieldGroupId;
  msgFieldGroupInfo->cmdKey = cmdKey;
  c_memcpy(msgFieldGroupInfo->fileName, fileName, c_strlen(fileName));
  self->compMsgMsgDesc->currMsgFieldGroupInfo = self->compMsgMsgDesc->numMsgFieldGroupInfo;
  self->compMsgMsgDesc->numMsgFieldGroupInfo++;
  return result;
}

// ================================= handleMsgFileInternal ====================================

static uint8_t handleMsgFileInternal(compMsgDispatcher_t *self, uint8_t *fileName, handleMsgLine_t handleMsgLine) {
  int result;
  uint8_t numEntries;
  uint8_t lgth;
  uint8_t buf[100];
  uint8_t *buffer;
  uint8_t *myStr;
  char *ep;
  int numLines;
  int idx;
  compMsgMsgDesc_t *compMsgMsgDesc;
  compMsgFile_t *compMsgFile;
  msgFieldGroupInfo_t *msgFieldGroupInfo;

  compMsgMsgDesc = self->compMsgMsgDesc;
  compMsgFile = self->compMsgFile;
  buffer = buf;
  result = compMsgFile->openFile(self, fileName, "r");
  checkErrOK(result);
#undef checkErrOK
#define checkErrOK(result) if(result != COMP_MSG_ERR_OK) { self->compMsgFile->closeFile(self); return result; }
  numLines = 0;
  while (1) {
    result = compMsgFile->readLine(self, &buffer, &lgth);
    checkErrOK(result);
    if (lgth == 0) {
      if (numLines - 1 != self->compMsgMsgDesc->expectedLines) {
        COMP_MSG_DBG(self, "Y", 0, "Error numLines: %d expectedLines: %d", numLines, self->compMsgMsgDesc->expectedLines);
        return COMP_MSG_ERR_BAD_NUM_DESC_FILE_LINES;
      }
      break;
    }
    buffer[lgth] = 0;
    result = compMsgFile->getLineFields(self, buffer, lgth);
    checkErrOK(result);
    // check if it is eventually an empty line and ignore that
    if (compMsgFile->numLineFields < 2) {
      if (c_strlen(compMsgFile->lineFields[0]) == 0) {
        continue;
      }
    }
    if ((buffer[0] == '#') && (buffer[1] == ' ')) {
      // a comment line skip
      continue;
    }
    // numLines is always without any comment lines!!
    numLines++;
    if (numLines == 1) {
      // check if it is the number of lines line
      if (c_strcmp(compMsgFile->lineFields[0], "#") == 0) {
        result = compMsgFile->getIntFieldValue(self, compMsgFile->lineFields[1], &ep, 0, &self->compMsgMsgDesc->expectedLines);
        checkErrOK(result);
      } else {
        COMP_MSG_DBG(self, "E", 0, "wrong desc file number lines line");
        return COMP_MSG_ERR_WRONG_DESC_FILE_LINE;
      }
    } else {
      result = handleMsgLine(self);
      checkErrOK(result);
    }
  }
#undef checkErrOK
#define checkErrOK(result) if(result != COMP_MSG_ERR_OK) { return result; }
  result = compMsgFile->closeFile(self);
  checkErrOK(result);
  return result;
}

// ================================= handleMsgFile ====================================

static uint8_t handleMsgFile(compMsgDispatcher_t *self, uint8_t *fileName, handleMsgLine_t handleMsgLine) {
  int result;
  uint8_t numEntries;
  uint8_t lgth;
  uint8_t buf[100];
  uint8_t *buffer;
  uint8_t *myStr;
  int numLines;
  int idx;
  compMsgMsgDesc_t *compMsgMsgDesc;
  msgFieldGroupInfo_t *msgFieldGroupInfo;

  result = handleMsgFileInternal(self, fileName, handleMsgLine);
  checkErrOK(result);
  compMsgMsgDesc = self->compMsgMsgDesc;
COMP_MSG_DBG(self, "E", 1, "handleMsgFiles done");
  if (compMsgMsgDesc->numMsgFieldGroupInfo > 0) {
    // handle fieldGroup files here
    compMsgMsgDesc->currMsgFieldGroupInfo = 0;
    while (compMsgMsgDesc->currMsgFieldGroupInfo < compMsgMsgDesc->numMsgFieldGroupInfo) {
      msgFieldGroupInfo = &compMsgMsgDesc->msgFieldGroupInfos[compMsgMsgDesc->currMsgFieldGroupInfo];
      COMP_MSG_DBG(self, "E", 1, ">>>handleFile %s", msgFieldGroupInfo->fileName);
      result = handleMsgFileInternal(self, msgFieldGroupInfo->fileName, compMsgMsgDesc->handleMsgFieldGroupLine);
      checkErrOK(result);
      COMP_MSG_DBG(self, "E", 1, "<<<%s: done", msgFieldGroupInfo->fileName);
      compMsgMsgDesc->currMsgFieldGroupInfo++;
    }
  }
  return result;
}

// ================================= handleMsgFileNameLine ====================================

static uint8_t handleMsgFileNameLine(compMsgDispatcher_t *self) {
  int result;
  int lgth;
  const uint8_t *token;
  uint8_t *fileName;
  uint8_t fieldGroupId;
  uint16_t cmdKey;
  uint8_t* cmdKeyStr;
  msgFieldGroupInfo_t *msgFieldGroupInfo;
  compMsgMsgDesc_t *compMsgMsgDesc;
  compMsgFile_t *compMsgFile;

  result = COMP_MSG_ERR_OK;
  compMsgMsgDesc = self->compMsgMsgDesc;
  compMsgFile = self->compMsgFile;
  if (compMsgFile->numLineFields < 2) {
    return COMP_MSG_ERR_FIELD_DESC_TOO_FEW_FIELDS;
  }
  fieldGroupId = 0;
  cmdKey = COMP_MSG_DATA_VALUE_CMD_KEY_SPECIAL;
  token = compMsgFile->lineFields[0];
  result = compMsgFile->getStringFieldValue(self, compMsgFile->lineFields[1], &fileName);
  checkErrOK(result);
  COMP_MSG_DBG(self, "E", 2, "token: %s field: %s", token, fileName);
  if (c_strncmp(token, "@$", 2) == 0) {
    result = self->compMsgTypesAndNames->getFileNameTokenIdFromStr(self, token, &fieldGroupId);
    checkErrOK(result);
    compMsgMsgDesc->msgFieldGroupFileName = os_zalloc(c_strlen(fileName) + 1);
    checkAllocOK(compMsgMsgDesc->msgFieldGroupFileName);
    c_memcpy(compMsgMsgDesc->msgFieldGroupFileName, fileName, c_strlen(fileName));
    if (compMsgFile->numLineFields > 2) {
      result = compMsgFile->getStringFieldValue(self, compMsgFile->lineFields[2], &cmdKeyStr);
      checkErrOK(result);
      if (c_strlen(cmdKeyStr) != 2) {
        return COMP_MSG_ERR_BAD_CMD_KEY_VALUE;
      }
      cmdKey = (cmdKeyStr[0] << 8) | cmdKeyStr[1];
      COMP_MSG_DBG(self, "E", 1, "cmdKey: %s 0x%04x", cmdKeyStr, cmdKey);
    }
    if (fieldGroupId != 0) {
      result = addFieldGroup(self, fileName, fieldGroupId, cmdKey);
      checkErrOK(result);
      msgFieldGroupInfo = &compMsgMsgDesc->msgFieldGroupInfos[compMsgMsgDesc->currMsgFieldGroupInfo];
      msgFieldGroupInfo->fieldGroupId = fieldGroupId;
    }
  }
  return result;
}

// ================================= handleMsgCommonLine ====================================

static uint8_t handleMsgCommonLine(compMsgDispatcher_t *self) {
  int result;
  int lgth;
  int keyValueId;
  uint32_t fieldFlags;
  uint8_t fieldNameId;
  fieldInfo_t fieldInfo;
  char *ep;
  uint8_t *stringVal;
  compMsgMsgDesc_t *compMsgMsgDesc;
  compMsgFile_t *compMsgFile;
  compMsgTypesAndNames_t *compMsgTypesAndNames;
  msgFieldDesc_t *msgFieldDesc;
  msgFieldGroupInfo_t *msgFieldGroupInfo;

  result = COMP_MSG_ERR_OK;
  compMsgMsgDesc = self->compMsgMsgDesc;
  compMsgFile = self->compMsgFile;
  compMsgTypesAndNames = self->compMsgTypesAndNames;
  msgFieldGroupInfo = &compMsgMsgDesc->msgFieldGroupInfos[compMsgMsgDesc->currMsgFieldGroupInfo];
  if (compMsgFile->numLineFields < 3) {
    return COMP_MSG_ERR_FIELD_DESC_TOO_FEW_FIELDS;
  }
  if (msgFieldGroupInfo->maxMsgFieldDesc == 0) {
    msgFieldGroupInfo->maxMsgFieldDesc = self->compMsgMsgDesc->expectedLines;
    msgFieldGroupInfo->msgFieldDescs = os_zalloc(msgFieldGroupInfo->maxMsgFieldDesc * sizeof(msgFieldDesc_t));
    checkAllocOK(msgFieldGroupInfo->msgFieldDescs);
  }
  msgFieldDesc = &msgFieldGroupInfo->msgFieldDescs[msgFieldGroupInfo->numMsgFieldDesc];
//COMP_MSG_DBG(self, "E", 1, "handleMsgCommonLine: %s: %s", compMsgFile->lineFields[0], compMsgFile->lineFields[1]);

  //field name
  result = self->compMsgTypesAndNames->getFieldNameIdFromStr(self, compMsgFile->lineFields[0], &msgFieldDesc->fieldNameId, COMP_MSG_INCR);
  checkErrOK(result);

  //field type
  result = self->compMsgTypesAndNames->getFieldTypeIdFromStr(self, compMsgFile->lineFields[1], &msgFieldDesc->fieldTypeId);
  checkErrOK(result);

  //field lgth
  result = compMsgFile->getIntFieldValue(self, compMsgFile->lineFields[2], &ep, 0, &lgth);
  checkErrOK(result);
  msgFieldDesc->fieldLgth = (uint16_t)lgth;

  //field key/value id
  if (msgFieldGroupInfo->fieldGroupId == COMP_MSG_DESC_KEY_VALUE_FIELD_GROUP) {
    result = compMsgFile->getIntFieldValue(self, compMsgFile->lineFields[3], &ep, 0, &keyValueId);
    checkErrOK(result);
//    msgFieldDesc->fieldLgth = (uint16_t)lgth;
  }

  if (msgFieldGroupInfo->fieldGroupId == COMP_MSG_DESC_HEADER_FIELD_GROUP) {
    result = compMsgMsgDesc->addHeaderInfo(self, msgFieldDesc->fieldLgth, msgFieldDesc->fieldNameId);
    checkErrOK(result);
  }
  COMP_MSG_DBG(self, "E", 2, "%s: id: %d type: %s %d lgth: %d", compMsgFile->lineFields[0], msgFieldDesc->fieldNameId, compMsgFile->lineFields[1], msgFieldDesc->fieldTypeId, msgFieldDesc->fieldLgth);
  fieldInfo.fieldFlags = 0;
  fieldInfo.fieldTypeId = msgFieldDesc->fieldTypeId;
  fieldInfo.fieldLgth = msgFieldDesc->fieldLgth;
  fieldInfo.keyValueDesc = NULL;
  switch (msgFieldGroupInfo->fieldGroupId) {
  case COMP_MSG_DESC_HEADER_FIELD_GROUP:
    fieldInfo.fieldFlags |= COMP_MSG_FIELD_HEADER;
    result = compMsgTypesAndNames->setMsgFieldInfo(self, msgFieldDesc->fieldNameId, &fieldInfo);
    checkErrOK(result);
    break;
  case COMP_MSG_DESC_MID_PART_FIELD_GROUP:
    result = compMsgTypesAndNames->setMsgFieldInfo(self, msgFieldDesc->fieldNameId, &fieldInfo);
    checkErrOK(result);
    break;
  case COMP_MSG_DESC_TRAILER_FIELD_GROUP:
    result = compMsgTypesAndNames->setMsgFieldInfo(self, msgFieldDesc->fieldNameId, &fieldInfo);
    checkErrOK(result);
    break;
  case COMP_MSG_DESC_KEY_VALUE_FIELD_GROUP:
    fieldInfo.fieldFlags |= COMP_MSG_FIELD_KEY_VALUE;
    result = compMsgTypesAndNames->setMsgFieldInfo(self, msgFieldDesc->fieldNameId, &fieldInfo);
    checkErrOK(result);
    break;
  }
  msgFieldGroupInfo->numMsgFieldDesc++;
  return result;
}

// ================================= handleMsgFieldsToSaveLine ====================================

static uint8_t handleMsgFieldsToSaveLine(compMsgDispatcher_t *self) {
  int result;
  int lgth;
  uint8_t fieldNameId;
  char *ep;
  uint8_t *stringVal;
  compMsgMsgDesc_t *compMsgMsgDesc;
  compMsgFile_t *compMsgFile;
  msgFieldDesc_t *msgFieldDesc;
  msgFieldGroupInfo_t *msgFieldGroupInfo;

  result = COMP_MSG_ERR_OK;
  compMsgMsgDesc = self->compMsgMsgDesc;
  compMsgFile = self->compMsgFile;
  msgFieldGroupInfo = &compMsgMsgDesc->msgFieldGroupInfos[compMsgMsgDesc->currMsgFieldGroupInfo];
  if (compMsgFile->numLineFields < 1) {
    return COMP_MSG_ERR_FIELD_DESC_TOO_FEW_FIELDS;
  }
  if (msgFieldGroupInfo->maxMsgFieldDesc == 0) {
    msgFieldGroupInfo->maxMsgFieldDesc = self->compMsgMsgDesc->expectedLines;
    msgFieldGroupInfo->msgFieldDescs = os_zalloc(msgFieldGroupInfo->maxMsgFieldDesc * sizeof(msgFieldDesc_t));
    checkAllocOK(msgFieldGroupInfo->msgFieldDescs);
  }
  msgFieldDesc = &msgFieldGroupInfo->msgFieldDescs[msgFieldGroupInfo->numMsgFieldDesc];
//COMP_MSG_DBG(self, "E", 1, "handleFieldsToSaveLine: %s", compMsgFile->lineFields[0]);
  //field name
  result = self->compMsgTypesAndNames->getFieldNameIdFromStr(self, compMsgFile->lineFields[0], &msgFieldDesc->fieldNameId, COMP_MSG_INCR);
  COMP_MSG_DBG(self, "E", 2, "%s: id: %d", compMsgFile->lineFields[0], msgFieldDesc->fieldNameId);
  checkErrOK(result);
  return result;
}

// ================================= handleMsgActionsLine ====================================

static uint8_t handleMsgActionsLine(compMsgDispatcher_t *self) {
  int result;
  int lgth;
  uint8_t fieldNameId;
  char *ep;
  uint8_t *stringVal;
  compMsgMsgDesc_t *compMsgMsgDesc;
  compMsgFile_t *compMsgFile;
  msgFieldDesc_t *msgFieldDesc;
  msgFieldGroupInfo_t *msgFieldGroupInfo;

  result = COMP_MSG_ERR_OK;
  compMsgMsgDesc = self->compMsgMsgDesc;
  compMsgFile = self->compMsgFile;
  msgFieldGroupInfo = &compMsgMsgDesc->msgFieldGroupInfos[compMsgMsgDesc->currMsgFieldGroupInfo];
  if (compMsgFile->numLineFields < 2) {
    return COMP_MSG_ERR_FIELD_DESC_TOO_FEW_FIELDS;
  }
  if (msgFieldGroupInfo->maxMsgFieldDesc == 0) {
    msgFieldGroupInfo->maxMsgFieldDesc = self->compMsgMsgDesc->expectedLines;
    msgFieldGroupInfo->msgFieldDescs = os_zalloc(msgFieldGroupInfo->maxMsgFieldDesc * sizeof(msgFieldDesc_t));
    checkAllocOK(msgFieldGroupInfo->msgFieldDescs);
  }
  msgFieldDesc = &msgFieldGroupInfo->msgFieldDescs[msgFieldGroupInfo->numMsgFieldDesc];
//COMP_MSG_DBG(self, "E", 1, "handleActionsLine: %s: %s", compMsgFile->lineFields[0], compMsgFile->lineFields[1]);
  //field name
  result = self->compMsgTypesAndNames->getFieldNameIdFromStr(self, compMsgFile->lineFields[0], &msgFieldDesc->fieldNameId, COMP_MSG_INCR);
  checkErrOK(result);
  //field value
  COMP_MSG_DBG(self, "E", 2, "%s: id: %d val: %s", compMsgFile->lineFields[0], msgFieldDesc->fieldNameId, compMsgFile->lineFields[1]);
  checkErrOK(result);
  return result;
}

// ================================= handleMsgValuesLine ====================================

static uint8_t handleMsgValuesLine(compMsgDispatcher_t *self) {
  int result;
  int lgth;
  uint8_t fieldNameId;
  uint8_t fieldTypeId;
  uint8_t fieldId;
  char *ep;
  uint8_t *stringValue;
  int numericValue;
  uint8_t *token;
  uint8_t *value;
  compMsgMsgDesc_t *compMsgMsgDesc;
  compMsgFile_t *compMsgFile;
  dataValue_t dataValue;
  msgFieldGroupInfo_t *msgFieldGroupInfo;
  fieldInfo_t fieldInfo;

  result = COMP_MSG_ERR_OK;
  compMsgMsgDesc = self->compMsgMsgDesc;
  compMsgFile = self->compMsgFile;
  msgFieldGroupInfo = &compMsgMsgDesc->msgFieldGroupInfos[compMsgMsgDesc->currMsgFieldGroupInfo];
  if (compMsgFile->numLineFields < 2) {
    return COMP_MSG_ERR_FIELD_DESC_TOO_FEW_FIELDS;
  }
  c_memset(&dataValue, 0, sizeof(dataValue_t));
  dataValue.fieldValueCallback = NULL;
  dataValue.cmdKey = msgFieldGroupInfo->cmdKey;
 
  //field name
  token = compMsgFile->lineFields[0];
  result = self->compMsgTypesAndNames->getFieldNameIdFromStr(self, token, &fieldNameId, COMP_MSG_INCR);
  checkErrOK(result);
  // get the fieldInfo for the name to eventually set fieldFlags or other info.
  result = self->compMsgTypesAndNames->getMsgFieldInfo(self, fieldNameId, &fieldInfo);
  checkErrOK(result);

  //field value
  value = compMsgFile->lineFields[1];
  stringValue = NULL;
  numericValue = 0;
  if (value[0] == '"') {
    dataValue.flags |= COMP_MSG_FIELD_IS_STRING;
    result = compMsgFile->getStringFieldValue(self, value, &stringValue);
    checkErrOK(result);
    dataValue.value.stringValue = stringValue;
    result = self->compMsgTypesAndNames->getFieldTypeIdFromStr(self, "uint8_t*", &fieldInfo.fieldTypeId);
    checkErrOK(result);
    fieldInfo.fieldLgth = c_strlen(stringValue);
    result = self->compMsgTypesAndNames->setMsgFieldInfo(self, fieldNameId, &fieldInfo);
    checkErrOK(result);
  } else {
    if (value[0] == '@') {
      dataValue.flags |= COMP_MSG_FIELD_HAS_CALLBACK;
      result = compMsgFile->getStringFieldValue(self, value, &stringValue);
      checkErrOK(result);
      dataValue.value.stringValue = stringValue;
      result = self->compMsgTypesAndNames->getFieldTypeIdFromStr(self, "uint8_t*", &fieldInfo.fieldTypeId);
      checkErrOK(result);
      fieldInfo.fieldLgth = c_strlen(stringValue);
      result = self->compMsgTypesAndNames->setMsgFieldInfo(self, fieldNameId, &fieldInfo);
      checkErrOK(result);
    } else {
      dataValue.flags |= COMP_MSG_FIELD_IS_NUMERIC;
      result = compMsgFile->getIntFieldValue(self, value, &ep, 0, &numericValue);
      checkErrOK(result);
      dataValue.value.numericValue = numericValue;
      result = self->compMsgTypesAndNames->getFieldTypeIdFromStr(self, "uint32_t", &fieldInfo.fieldTypeId);
      checkErrOK(result);
      fieldInfo.fieldLgth = sizeof(uint32_t);
      result = self->compMsgTypesAndNames->setMsgFieldInfo(self, fieldNameId, &fieldInfo);
      checkErrOK(result);
    }
  }
  COMP_MSG_DBG(self, "E", 1, "handleMsgValuesLine: %s: id: %d val: %s %d cb: %s", token, fieldNameId, stringValue == NULL ? "nil" : (char *)stringValue, numericValue, dataValue.fieldValueCallback == NULL ? "nil" : (char *)dataValue.fieldValueCallback);
  switch (msgFieldGroupInfo->fieldGroupId) {
  case COMP_MSG_WIFI_DATA_VALUES_FIELD_GROUP:
  case COMP_MSG_MODULE_DATA_VALUES_FIELD_GROUP:
    result = self->compMsgDataValue->dataValueStr2ValueId(self, token, &fieldId);
    checkErrOK(result);
    dataValue.fieldNameId = 0;
    dataValue.fieldValueId = fieldId;
    result = self->compMsgDataValue->addDataValue(self, &dataValue);
    checkErrOK(result);
    break;
  case COMP_MSG_VAL_FIELD_GROUP:
    dataValue.fieldNameId = fieldNameId;
    dataValue.fieldValueId = 0;
    result = self->compMsgDataValue->addDataValue(self, &dataValue);
    break;
  default:
    return COMP_MSG_ERR_BAD_DESC_FILE_FIELD_GROUP_TYPE;
  }
//COMP_MSG_DBG(self, "E", 1, "handleMsgValuesLine: done");
  return result;
}

// ================================= handleMsgHeadsLine ====================================

static uint8_t handleMsgHeadsLine(compMsgDispatcher_t *self) {
  uint8_t result;
  int lgth;
  int fieldGroupIdx;
  int fieldIdx;
  uint8_t numHeaderFields;
  uint8_t *fieldNameStr;
  uint8_t *value;
  uint8_t *stringVal;
  int numericValue;
  char *ep;
  compMsgMsgDesc_t *compMsgMsgDesc;
  compMsgFile_t *compMsgFile;
  msgFieldDesc_t *msgFieldDesc;
  msgFieldGroupInfo_t *msgFieldGroupInfo;
  msgDescriptionInfos_t *descriptions;
  msgDescription_t *msgDescription;

  result = COMP_MSG_ERR_OK;
  compMsgMsgDesc = self->compMsgMsgDesc;
  compMsgFile = self->compMsgFile;
  result = self->compMsgUtil->addFieldDescription(self);
  checkErrOK(result);
  descriptions = &self->compMsgMsgDesc->msgDescriptionInfos;
  msgDescription = &descriptions->msgDescriptions[descriptions->numMsgDescriptions - 1];
  fieldGroupIdx = 0;
  numHeaderFields = 0;
  while (fieldGroupIdx < compMsgMsgDesc->numMsgFieldGroupInfo) {
    msgFieldGroupInfo = &compMsgMsgDesc->msgFieldGroupInfos[fieldGroupIdx];
    if (msgFieldGroupInfo->fieldGroupId == COMP_MSG_DESC_HEADER_FIELD_GROUP) {
      numHeaderFields = msgFieldGroupInfo->numMsgFieldDesc;
      break;
    }
    fieldGroupIdx++;
  }
  if (numHeaderFields == 0) {
    return COMP_MSG_ERR_HEADER_FIELD_GROUP_NOT_FOUND;
  }
  if (compMsgFile->numLineFields < 3) {
    return COMP_MSG_ERR_FIELD_DESC_TOO_FEW_FIELDS;
  }
  fieldIdx = 0;
//COMP_MSG_DBG(self, "E", 1, "handleMsgHeadsLine: %s: %s", compMsgFile->lineFields[0], compMsgFile->lineFields[1]);
  msgDescription->headerLgth = 0;
  while (fieldIdx < numHeaderFields) {
    msgFieldDesc = &msgFieldGroupInfo->msgFieldDescs[fieldIdx];
    msgDescription->headerLgth += msgFieldDesc->fieldLgth;
    result = self->compMsgTypesAndNames->getFieldNameStrFromId(self, msgFieldDesc->fieldNameId, &fieldNameStr);
    value = compMsgFile->lineFields[fieldIdx];
    COMP_MSG_DBG(self, "E", 2, "field: %s %s", fieldNameStr, value);
    if (c_strcmp(value, "*") == 0) {
      msgFieldDesc->fieldFlags |= COMP_MSG_VAL_IS_JOKER;
    } else {
      if (value[0] == '"') {
        switch (msgFieldDesc->fieldTypeId) {
        case DATA_VIEW_FIELD_UINT8_T:
        case DATA_VIEW_FIELD_INT8_T:
        case DATA_VIEW_FIELD_UINT16_T:
        case DATA_VIEW_FIELD_INT16_T:
        case DATA_VIEW_FIELD_UINT32_T:
        case DATA_VIEW_FIELD_INT32_T:
          return COMP_MSG_ERR_EXPECTED_INT_VALUE;
        }
        stringVal = NULL;
        result = compMsgFile->getStringFieldValue(self, value, &stringVal);
        checkErrOK(result);
        COMP_MSG_DBG(self, "E", 2, "value: %s!stringVal: %s", value, stringVal);
#ifdef NOTDEF
// FIXME !!!
        msgFieldDesc->msgFieldVal.value.u8vec = os_zalloc(c_strlen(stringVal) + 1);
        checkAllocOK(msgFieldDesc->msgFieldVal.value.u8vec);
        c_memcpy(msgFieldDesc->msgFieldVal.value.u8vec, stringVal, c_strlen(stringVal));
        COMP_MSG_DBG(self, "E", 2, "field2: %s %s", fieldNameStr, msgFieldDesc->msgFieldVal.value.u8vec);
#endif
      } else {
//        result = compMsgMsgDesc->getIntFieldValue(self, value, &ep, 0, &msgFieldDesc->msgFieldVal.value.intVal);
        result = compMsgFile->getIntFieldValue(self, value, &ep, 0, &numericValue);
      }
      checkErrOK(result);
    }
    fieldIdx++;
  }
  if (c_strlen(compMsgFile->lineFields[fieldIdx]) > 1) {
    return COMP_MSG_ERR_BAD_ENCRYPTED_VALUE;
  }
  msgDescription->encrypted = compMsgFile->lineFields[fieldIdx][0];
  fieldIdx++;
  if (c_strlen(compMsgFile->lineFields[fieldIdx]) > 1) {
    return COMP_MSG_ERR_BAD_HANDLE_TYPE_VALUE;
  }
  msgDescription->handleType = compMsgFile->lineFields[fieldIdx][0];
  // FIXME need to handle cmdKey here!!!
  COMP_MSG_DBG(self, "E", 2, "msgDescription->headerLgth: %d encrypted: %c handleType: %c", msgDescription->headerLgth, msgDescription->encrypted, msgDescription->handleType);
  return result;
}

// ================================= handleMsgFieldGroupLine ====================================

static uint8_t handleMsgFieldGroupLine(compMsgDispatcher_t *self) {
  int result;
  int lgth;
  uint8_t fieldNameId;
  char *ep;
  uint8_t *stringVal;
  compMsgMsgDesc_t *compMsgMsgDesc;
  msgFieldDesc_t *msgFieldDesc;
  msgFieldGroupInfo_t *msgFieldGroupInfo;

  result = COMP_MSG_ERR_OK;
  compMsgMsgDesc = self->compMsgMsgDesc;
  msgFieldGroupInfo = &compMsgMsgDesc->msgFieldGroupInfos[compMsgMsgDesc->currMsgFieldGroupInfo];
  switch (msgFieldGroupInfo->fieldGroupId) {
  case COMP_MSG_DESC_HEADER_FIELD_GROUP:
  case COMP_MSG_DESC_MID_PART_FIELD_GROUP:
  case COMP_MSG_DESC_TRAILER_FIELD_GROUP:
  case COMP_MSG_DESC_KEY_VALUE_FIELD_GROUP:
    result = compMsgMsgDesc->handleMsgCommonLine(self);
    checkErrOK(result);
    break;
  case COMP_MSG_FIELDS_TO_SAVE_FIELD_GROUP:
    result = compMsgMsgDesc->handleMsgFieldsToSaveLine(self);
    checkErrOK(result);
    break;
  case COMP_MSG_ACTIONS_FIELD_GROUP:
    result = compMsgMsgDesc->handleMsgActionsLine(self);
    checkErrOK(result);
    break;
  case COMP_MSG_HEADS_FIELD_GROUP:
    result = self->compMsgMsgDesc->handleMsgHeadsLine(self);
    checkErrOK(result);
    break;
  case COMP_MSG_WIFI_DATA_VALUES_FIELD_GROUP:
    result = self->compMsgMsgDesc->handleMsgValuesLine(self);
    checkErrOK(result);
    break;
  case COMP_MSG_MODULE_DATA_VALUES_FIELD_GROUP:
    result = self->compMsgMsgDesc->handleMsgValuesLine(self);
    checkErrOK(result);
    break;
  case COMP_MSG_DESC_FIELD_GROUP:
    result = self->compMsgMsgDesc->handleMsgCommonLine(self);
    checkErrOK(result);
    break;
  case COMP_MSG_VAL_FIELD_GROUP:
    result = self->compMsgMsgDesc->handleMsgValuesLine(self);
    checkErrOK(result);
    break;
  default:
    COMP_MSG_DBG(self, "E", 0, "bad desc file fieldGroupId 0x%02x cmdKey: 0x%04x", msgFieldGroupInfo->fieldGroupId, msgFieldGroupInfo->cmdKey);
    return COMP_MSG_ERR_BAD_DESC_FILE_FIELD_GROUP_TYPE;
  }
  return result;
}





#ifdef OLD

// ================================= getHeaderFieldsFromLine ====================================

static uint8_t getHeaderFieldsFromLine(compMsgDispatcher_t *self, msgHeaderInfos_t *hdrInfos, uint8_t *myStr, uint8_t **ep, int *seqIdx) {
  int result;
  bool isEnd;
  long uval;
  uint8_t *cp;
  uint8_t fieldNameId;

  COMP_MSG_DBG(self, "E", 2, "numHeaderParts: %d seqidx: %d\n", hdrInfos->numHeaderParts, *seqIdx);
  cp = myStr;
  result = getIntFromLine(self, cp, &uval, ep, &isEnd);
  checkErrOK(result);
  COMP_MSG_DBG(self, "E", 2, "desc: headerLgth: %d\n", uval);
  hdrInfos->headerLgth = (uint8_t)uval;
  checkIsEnd(isEnd);
  cp = *ep;
  while (!isEnd) {
    result = getStrFromLine(self, cp, ep, &isEnd);
    checkErrOK(result);
    if (cp[0] != '@') {
      return COMP_MSG_ERR_NO_SUCH_FIELD;
    }
    result = self->compMsgTypesAndNames->getFieldNameIdFromStr(self, cp, &fieldNameId, COMP_MSG_NO_INCR);
    checkErrOK(result);
    switch (fieldNameId) {
    case COMP_MSG_SPEC_FIELD_SRC:
      if (hdrInfos->headerFlags & COMP_DISP_HDR_SRC) {
        return COMP_MSG_ERR_DUPLICATE_FIELD;
      }
      hdrInfos->headerSequence[(*seqIdx)++] = COMP_DISP_U16_SRC;
      hdrInfos->headerFlags |= COMP_DISP_HDR_SRC;
      break;
    case COMP_MSG_SPEC_FIELD_DST:
      if (hdrInfos->headerFlags & COMP_DISP_HDR_DST) {
        return COMP_MSG_ERR_DUPLICATE_FIELD;
      }
      hdrInfos->headerSequence[(*seqIdx)++] = COMP_DISP_U16_DST;
      hdrInfos->headerFlags |= COMP_DISP_HDR_DST;
      break;
    case COMP_MSG_SPEC_FIELD_TOTAL_LGTH:
      if (hdrInfos->headerFlags & COMP_DISP_HDR_TOTAL_LGTH) {
        return COMP_MSG_ERR_DUPLICATE_FIELD;
      }
      hdrInfos->headerSequence[(*seqIdx)++] = COMP_DISP_U16_TOTAL_LGTH;
      hdrInfos->headerFlags |= COMP_DISP_HDR_TOTAL_LGTH;
      break;
    case COMP_MSG_SPEC_FIELD_SRC_ID:
      if (hdrInfos->headerFlags & COMP_DISP_HDR_SRC_ID) {
        return COMP_MSG_ERR_DUPLICATE_FIELD;
      }
      hdrInfos->headerSequence[(*seqIdx)++] = COMP_DISP_U16_SRC_ID;
      hdrInfos->headerFlags |= COMP_DISP_HDR_SRC_ID;
      break;
    case COMP_MSG_SPEC_FIELD_GUID:
      if (hdrInfos->headerFlags & COMP_DISP_HDR_GUID) {
        return COMP_MSG_ERR_DUPLICATE_FIELD;
      }
      hdrInfos->headerSequence[(*seqIdx)++] = COMP_DISP_U8_VECTOR_GUID;
      hdrInfos->headerFlags |= COMP_DISP_HDR_GUID;;
      break;
    case COMP_MSG_SPEC_FIELD_HDR_FILLER:
      if (hdrInfos->headerFlags & COMP_DISP_HDR_FILLER) {
        return COMP_MSG_ERR_DUPLICATE_FIELD;
      }
      hdrInfos->headerSequence[(*seqIdx)++] = COMP_DISP_U8_VECTOR_HDR_FILLER;
      hdrInfos->headerFlags |= COMP_DISP_HDR_FILLER;
      break;
    default:
      checkErrOK(COMP_MSG_ERR_NO_SUCH_FIELD);
      break;
    }
    cp = *ep;
  }
  hdrInfos->seqIdxAfterHeader = *seqIdx;
  if (!isEnd) {
    return COMP_MSG_ERR_FUNNY_EXTRA_FIELDS;
  }
  return COMP_MSG_ERR_OK;
}
  
#endif

// ================================= compMsgMsgDescInit ====================================

static uint8_t compMsgMsgDescInit(compMsgDispatcher_t *self) {
  uint8_t result;
  compMsgMsgDesc_t *compMsgMsgDesc;

  compMsgMsgDesc = self->compMsgMsgDesc;

  compMsgMsgDesc->addHeaderInfo = &addHeaderInfo;

  compMsgMsgDesc->addFieldGroup = &addFieldGroup;

  compMsgMsgDesc->handleMsgCommonLine = &handleMsgCommonLine;
  compMsgMsgDesc->handleMsgFieldGroupLine = &handleMsgFieldGroupLine;
  compMsgMsgDesc->handleMsgFieldsToSaveLine = &handleMsgFieldsToSaveLine;
  compMsgMsgDesc->handleMsgActionsLine = &handleMsgActionsLine;
  compMsgMsgDesc->handleMsgValuesLine = &handleMsgValuesLine;
//  compMsgMsgDesc->handleMsgValHeaderLine = &handleMsgValHeaderLine;
  compMsgMsgDesc->handleMsgHeadsLine = &handleMsgHeadsLine;
  compMsgMsgDesc->handleMsgFileNameLine = &handleMsgFileNameLine;
  compMsgMsgDesc->handleMsgFile = &handleMsgFile;

  result = compMsgMsgDesc->handleMsgFile(self, MSG_FILES_FILE_NAME, compMsgMsgDesc->handleMsgFileNameLine);
  checkErrOK(result);
  result = self->compMsgTypesAndNames->dumpMsgFieldInfos(self);
  checkErrOK(result);
  return COMP_MSG_ERR_OK;
}


// ================================= freeCompMsgMsgDesc ====================================

void freeCompMsgMsgDesc(compMsgMsgDesc_t *compMsgMsgDesc) {
}

// ================================= newCompMsgMsgDesc ====================================

compMsgMsgDesc_t *newCompMsgMsgDesc() {
  compMsgMsgDesc_t *compMsgMsgDesc;

  compMsgMsgDesc = os_zalloc(sizeof(compMsgMsgDesc_t));
  if (compMsgMsgDesc == NULL) {
    return NULL;
  }
  compMsgMsgDesc->freeCompMsgMsgDesc = &freeCompMsgMsgDesc;
  compMsgMsgDesc->compMsgMsgDescInit = &compMsgMsgDescInit;
  return compMsgMsgDesc;
}
