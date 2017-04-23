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
  { COMP_MSG_U8_DST,           "COMP_MSG_U8_DST" },
  { COMP_MSG_U16_DST,          "COMP_MSG_U16_DST" },
  { COMP_MSG_U8_SRC,           "COMP_MSG_U8_SRC" },
  { COMP_MSG_U16_SRC,          "COMP_MSG_U16_SRC" },
  { COMP_MSG_U8_TOTAL_LGTH,    "COMP_MSG_U8_TOTAL_LGTH" },
  { COMP_MSG_U16_TOTAL_LGTH,   "COMP_MSG_U16_TOTAL_LGTH" },
  { COMP_MSG_VECTOR_GUID,      "COMP_MSG_VECTOR_GUID" },
  { COMP_MSG_IP_ADDR,          "COMP_MSG_IP_ADDR" },
//  { COMP_MSG_IS_ENCRYPTED,     "COMP_MSG_IS_ENCRYPTED" },
//  { COMP_MSG_IS_NOT_ENCRYPTED, "COMP_MSG_IS_NOT_ENCRYPTED" },
  { COMP_MSG_U8_CMD_KEY,       "COMP_MSG_U8_CMD_KEY" },
  { COMP_MSG_U16_CMD_KEY,      "COMP_MSG_U16_CMD_KEY" },
  { COMP_MSG_U0_CMD_LGTH,      "COMP_MSG_U0_CMD_LGTH" },
  { COMP_MSG_U8_CMD_LGTH,      "COMP_MSG_U8_CMD_LGTH" },
  { COMP_MSG_U16_CMD_LGTH,     "COMP_MSG_U16_CMD_LGTH" },
  { COMP_MSG_U0_CRC,           "COMP_MSG_U0_CRC" },
  { COMP_MSG_U8_CRC,           "COMP_MSG_U8_CRC" },
  { COMP_MSG_U16_CRC,          "COMP_MSG_U16_CRC" },
  { COMP_MSG_U0_TOTAL_CRC,     "COMP_MSG_U0_TOTAL_CRC" },
  { COMP_MSG_U8_TOTAL_CRC,     "COMP_MSG_U8_TOTAL_CRC" },
  { COMP_MSG_U16_TOTAL_CRC,    "COMP_MSG_U16_TOTAL_CRC" },
  { 0,                          NULL },
};

static volatile int fileFd = FS_OPEN_OK - 1;

// ================================= openFile ====================================

static uint8_t openFile(compMsgMsgDesc_t *self, const uint8_t *fileName, const uint8_t *fileMode) {
  self->fileName = fileName;
  fileFd = fs_open(fileName, fs_mode2flag(fileMode));
  if (fileFd < FS_OPEN_OK) {
    return COMP_MSG_ERR_OPEN_FILE;
  }
  return COMP_MSG_ERR_OK;
}

// ================================= closeFile ====================================

static uint8_t closeFile(compMsgMsgDesc_t *self) {
  if (fileFd != (FS_OPEN_OK - 1)){
    self->fileName = NULL;
    fs_close(fileFd);
    fileFd = FS_OPEN_OK - 1;
  }
  return COMP_MSG_ERR_OK;
}

// ================================= flushFile ====================================

static uint8_t flushFile(compMsgMsgDesc_t *self) {
  if (fileFd == (FS_OPEN_OK - 1)) {
    return COMP_MSG_ERR_FILE_NOT_OPENED;
  }
  if (fs_flush(fileFd) == 0) {
    return COMP_MSG_ERR_OK;
  }
  return COMP_MSG_ERR_FLUSH_FILE;
}

// ================================= readLine ====================================

static uint8_t readLine(compMsgMsgDesc_t *self, uint8_t **buffer, uint8_t *lgth) {
  size_t n = BUFSIZ;
  char buf[BUFSIZ];
  int i;
  uint8_t *cp;
  uint8_t end_char = '\n';

  if (fileFd == (FS_OPEN_OK - 1)) {
    return COMP_MSG_ERR_FILE_NOT_OPENED;
  }
  n = fs_read(fileFd, buf, n);
  cp = *buffer;
  *lgth = 0;
  for (i = 0; i < n; ++i) {
    cp[i] = buf[i];
    if (buf[i] == end_char) {
      ++i;
      break;
    }
  }
  cp[i] = 0;
  *lgth = i;
  fs_seek (fileFd, -(n - i), SEEK_CUR);
  return COMP_MSG_ERR_OK;
}

// ================================= writeLine ====================================

static uint8_t writeLine(compMsgMsgDesc_t *self, const uint8_t *buffer, uint8_t lgth) {
  int result;

  if (fileFd == (FS_OPEN_OK - 1)) {
    return COMP_MSG_ERR_FILE_NOT_OPENED;
  }
  result = fs_write(fileFd, buffer, lgth);
  if (result == lgth) {
    return COMP_MSG_ERR_OK;
  }
  return COMP_MSG_ERR_WRITE_FILE;
}

// ================================= getLineFields ====================================

static uint8_t getLineFields(compMsgDispatcher_t *self, uint8_t *myStr, uint8_t lgth) {
  char *cp;
  char *ep;
  int idx;

  self->compMsgMsgDesc->numLineFields = 0;
  cp = myStr;
  ep = myStr + lgth;
  self->compMsgMsgDesc->lineFields[self->compMsgMsgDesc->numLineFields] = cp;
  while (cp < ep) {
    if (*cp == ',') {
      *cp = '\0';
      cp++;
      self->compMsgMsgDesc->numLineFields++;
      self->compMsgMsgDesc->lineFields[self->compMsgMsgDesc->numLineFields] = cp;
    } else {
      if ((*cp == '\r') || (*cp == '\n')) {
        *cp = '\0';
      }
      cp++;
    }
  }
  self->compMsgMsgDesc->numLineFields++;
  idx = 0;
while (idx < self->compMsgMsgDesc->numLineFields) {
//ets_printf("lineField: %d: %s!\n", idx, self->compMsgMsgDesc->lineFields[idx]);
idx++;
}
  return COMP_MSG_ERR_OK;
}

// ================================= getIntFieldValue ====================================

static uint8_t getIntFieldValue(compMsgDispatcher_t *self, uint8_t *cp, char **ep, int base, int *uval) {
  *uval = c_strtoul(cp, ep, base);
  return COMP_MSG_ERR_OK;
}

// ================================= getStringFieldValue ====================================

static uint8_t getStringFieldValue(compMsgDispatcher_t *self, uint8_t *cp, uint8_t **strVal) {
  uint8_t *cp1;
  uint8_t *cp2;
  char *endPtr;

  cp1 = cp;
  if (*cp1 != '"') {
    // not a string
    *strVal = NULL;
  }
  cp1++;
  *strVal = cp1;
  while (*cp1 != '\0') {
    if (cp1[0] == '"') {
      if (cp1[1] == '\0') {
        cp1[0] = '\0';
        break;
      }
    }
    // if we have no '"' at the end, silently ignore that
    cp1++;
  }
  return COMP_MSG_ERR_OK;
}

// ================================= addFieldGroup ====================================

static uint8_t addFieldGroup(compMsgDispatcher_t *self, char *fileName) {
  uint8_t result;
  msgFieldGroupInfo_t *msgFieldGroupInfo;

  result = COMP_MSG_ERR_OK;
  if (self->compMsgMsgDesc->numMsgFieldGroupInfo >= self->compMsgMsgDesc->maxMsgFieldGroupInfo) {
    if (self->compMsgMsgDesc->maxMsgFieldGroupInfo == 0) {
      self->compMsgMsgDesc->maxMsgFieldGroupInfo = 5;
      self->compMsgMsgDesc->msgFieldGroupInfos = (msgFieldGroupInfo_t *)os_zalloc((self->compMsgMsgDesc->maxMsgFieldGroupInfo * sizeof(msgFieldGroupInfo_t)));
      checkAllocOK(self->compMsgMsgDesc->msgFieldGroupInfos);
    } else {
      self->compMsgMsgDesc->maxMsgFieldGroupInfo += 2;
      self->compMsgMsgDesc->msgFieldGroupInfos = (msgFieldGroupInfo_t *)os_realloc((self->compMsgMsgDesc->msgFieldGroupInfos), (self->compMsgMsgDesc->maxMsgFieldGroupInfo * sizeof(msgFieldGroupInfo_t)));
      checkAllocOK(self->compMsgMsgDesc->msgFieldGroupInfos);
    }
  }
  msgFieldGroupInfo = &self->compMsgMsgDesc->msgFieldGroupInfos[self->compMsgMsgDesc->numMsgFieldGroupInfo];
  memset(msgFieldGroupInfo, 0, sizeof(msgFieldGroupInfo_t));
  msgFieldGroupInfo->fileName = os_zalloc(c_strlen(fileName) + 1);
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
  msgFieldGroupInfo_t *msgFieldGroupInfo;

  compMsgMsgDesc = self->compMsgMsgDesc;
  buffer = buf;
  result = compMsgMsgDesc->openFile(compMsgMsgDesc, fileName, "r");
  checkErrOK(result);
#undef checkErrOK
#define checkErrOK(result) if(result != COMP_MSG_ERR_OK) { compMsgMsgDesc->closeFile(compMsgMsgDesc); return result; }
  numLines = 0;
  while (1) {
    result = compMsgMsgDesc->readLine(compMsgMsgDesc, &buffer, &lgth);
    checkErrOK(result);
    if (lgth == 0) {
      if (numLines - 1 != self->compMsgMsgDesc->expectedLines) {
        COMP_MSG_DBG(self, "Y", 0, "Error numLines: %d expectedLines: %d", numLines, self->compMsgMsgDesc->expectedLines);
        return COMP_MSG_ERR_BAD_NUM_DESC_FILE_LINES;
      }
      break;
    }
    buffer[lgth] = 0;
    result = compMsgMsgDesc->getLineFields(self, buffer, lgth);
    checkErrOK(result);
    // check if it is eventually an empty line and ignore that
    if (compMsgMsgDesc->numLineFields < 2) {
      if (c_strlen(compMsgMsgDesc->lineFields[0]) == 0) {
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
      if (c_strcmp(compMsgMsgDesc->lineFields[0], "#") == 0) {
        result = compMsgMsgDesc->getIntFieldValue(self, compMsgMsgDesc->lineFields[1], &ep, 0, &self->compMsgMsgDesc->expectedLines);
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
  result = compMsgMsgDesc->closeFile(compMsgMsgDesc);
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
  if (compMsgMsgDesc->numMsgFieldGroupInfo > 0) {
    // handle fieldGroup files here
    compMsgMsgDesc->currMsgFieldGroupInfo = 0;
    while (compMsgMsgDesc->currMsgFieldGroupInfo < compMsgMsgDesc->numMsgFieldGroupInfo) {
      msgFieldGroupInfo = &compMsgMsgDesc->msgFieldGroupInfos[compMsgMsgDesc->currMsgFieldGroupInfo];
      COMP_MSG_DBG(self, "E", 2, ">>>handleFile %s", msgFieldGroupInfo->fileName);
      result = handleMsgFileInternal(self, msgFieldGroupInfo->fileName, compMsgMsgDesc->handleMsgFieldGroupLine);
      checkErrOK(result);
      COMP_MSG_DBG(self, "E", 2, "<<<%s: done", msgFieldGroupInfo->fileName);
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
  uint8_t *field;
  uint8_t fileNameTokenId;
  msgFieldGroupInfo_t *msgFieldGroupInfo;
  compMsgMsgDesc_t *compMsgMsgDesc;

  result = COMP_MSG_ERR_OK;
  compMsgMsgDesc = self->compMsgMsgDesc;
  if (compMsgMsgDesc->numLineFields < 2) {
    return COMP_MSG_ERR_FIELD_DESC_TOO_FEW_FIELDS;
  }
  fileNameTokenId = 0;
  token = compMsgMsgDesc->lineFields[0];
  result = compMsgMsgDesc->getStringFieldValue(self, compMsgMsgDesc->lineFields[1], &field);
  checkErrOK(result);
  COMP_MSG_DBG(self, "E", 2, "token: %s field: %s", token, field);
  if (c_strncmp(token, "@$", 2) == 0) {
    result = self->compMsgTypesAndNames->getFileNameTokenIdFromStr(self->compMsgTypesAndNames, token, &fileNameTokenId);
    checkErrOK(result);
    compMsgMsgDesc->msgFieldGroupFileName = os_zalloc(c_strlen(field) + 1);
    checkAllocOK(compMsgMsgDesc->msgFieldGroupFileName);
    c_memcpy(compMsgMsgDesc->msgFieldGroupFileName, field, c_strlen(field));
    if (fileNameTokenId != 0) {
      result = addFieldGroup(self, field);
      checkErrOK(result);
      msgFieldGroupInfo = &compMsgMsgDesc->msgFieldGroupInfos[compMsgMsgDesc->currMsgFieldGroupInfo];
      msgFieldGroupInfo->fieldGroupType = fileNameTokenId;
    }
  }
  return result;
}

// ================================= handleMsgCommonLine ====================================

static uint8_t handleMsgCommonLine(compMsgDispatcher_t *self) {
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
  if (compMsgMsgDesc->numLineFields < 3) {
    return COMP_MSG_ERR_FIELD_DESC_TOO_FEW_FIELDS;
  }
  if (msgFieldGroupInfo->maxMsgFieldDesc == 0) {
    msgFieldGroupInfo->maxMsgFieldDesc = self->compMsgMsgDesc->expectedLines;
    msgFieldGroupInfo->msgFieldDescs = os_zalloc(msgFieldGroupInfo->maxMsgFieldDesc * sizeof(msgFieldDesc_t));
    checkAllocOK(msgFieldGroupInfo->msgFieldDescs);
  }
  msgFieldDesc = &msgFieldGroupInfo->msgFieldDescs[msgFieldGroupInfo->numMsgFieldDesc];
  //field name
  result = self->compMsgTypesAndNames->getFieldNameIdFromStr(self->compMsgTypesAndNames, compMsgMsgDesc->lineFields[0], &msgFieldDesc->fieldNameId, COMP_MSG_INCR);
  checkErrOK(result);
  //field type
  result = self->compMsgTypesAndNames->getFieldTypeIdFromStr(self->compMsgTypesAndNames, compMsgMsgDesc->lineFields[1], &msgFieldDesc->fieldTypeId);
  checkErrOK(result);
  //field lgth
  result = compMsgMsgDesc->getIntFieldValue(self, compMsgMsgDesc->lineFields[2], &ep, 0, &lgth);
  checkErrOK(result);
  msgFieldDesc->fieldLgth = (uint16_t)lgth;
  COMP_MSG_DBG(self, "E", 2, "%s: id: %d type: %s %d lgth: %d", compMsgMsgDesc->lineFields[0], msgFieldDesc->fieldNameId, compMsgMsgDesc->lineFields[1], msgFieldDesc->fieldTypeId, msgFieldDesc->fieldLgth);
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
  msgFieldDesc_t *msgFieldDesc;
  msgFieldGroupInfo_t *msgFieldGroupInfo;

  result = COMP_MSG_ERR_OK;
  compMsgMsgDesc = self->compMsgMsgDesc;
  msgFieldGroupInfo = &compMsgMsgDesc->msgFieldGroupInfos[compMsgMsgDesc->currMsgFieldGroupInfo];
  if (compMsgMsgDesc->numLineFields < 1) {
    return COMP_MSG_ERR_FIELD_DESC_TOO_FEW_FIELDS;
  }
  if (msgFieldGroupInfo->maxMsgFieldDesc == 0) {
    msgFieldGroupInfo->maxMsgFieldDesc = self->compMsgMsgDesc->expectedLines;
    msgFieldGroupInfo->msgFieldDescs = os_zalloc(msgFieldGroupInfo->maxMsgFieldDesc * sizeof(msgFieldDesc_t));
    checkAllocOK(msgFieldGroupInfo->msgFieldDescs);
  }
  msgFieldDesc = &msgFieldGroupInfo->msgFieldDescs[msgFieldGroupInfo->numMsgFieldDesc];
  //field name
  result = self->compMsgTypesAndNames->getFieldNameIdFromStr(self->compMsgTypesAndNames, compMsgMsgDesc->lineFields[0], &msgFieldDesc->fieldNameId, COMP_MSG_INCR);
  COMP_MSG_DBG(self, "E", 2, "%s: id: %d", compMsgMsgDesc->lineFields[0], msgFieldDesc->fieldNameId);
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
  msgFieldDesc_t *msgFieldDesc;
  msgFieldGroupInfo_t *msgFieldGroupInfo;

  result = COMP_MSG_ERR_OK;
  compMsgMsgDesc = self->compMsgMsgDesc;
  msgFieldGroupInfo = &compMsgMsgDesc->msgFieldGroupInfos[compMsgMsgDesc->currMsgFieldGroupInfo];
  if (compMsgMsgDesc->numLineFields < 2) {
    return COMP_MSG_ERR_FIELD_DESC_TOO_FEW_FIELDS;
  }
  if (msgFieldGroupInfo->maxMsgFieldDesc == 0) {
    msgFieldGroupInfo->maxMsgFieldDesc = self->compMsgMsgDesc->expectedLines;
    msgFieldGroupInfo->msgFieldDescs = os_zalloc(msgFieldGroupInfo->maxMsgFieldDesc * sizeof(msgFieldDesc_t));
    checkAllocOK(msgFieldGroupInfo->msgFieldDescs);
  }
  msgFieldDesc = &msgFieldGroupInfo->msgFieldDescs[msgFieldGroupInfo->numMsgFieldDesc];
  //field name
  result = self->compMsgTypesAndNames->getFieldNameIdFromStr(self->compMsgTypesAndNames, compMsgMsgDesc->lineFields[0], &msgFieldDesc->fieldNameId, COMP_MSG_INCR);
  checkErrOK(result);
  //field value
  COMP_MSG_DBG(self, "E", 2, "%s: id: %d val: %s", compMsgMsgDesc->lineFields[0], msgFieldDesc->fieldNameId, compMsgMsgDesc->lineFields[1]);
  checkErrOK(result);
  return result;
}

// ================================= handleMsgValuesLine ====================================

static uint8_t handleMsgValuesLine(compMsgDispatcher_t *self) {
  int result;
  int lgth;
  uint8_t fieldNameId;
  uint8_t fieldId;
  char *ep;
  uint8_t *stringValue;
  int numericValue;
  uint8_t *token;
  uint8_t *value;
  compMsgMsgDesc_t *compMsgMsgDesc;
  dataValue_t dataValue;
  msgFieldGroupInfo_t *msgFieldGroupInfo;

  result = COMP_MSG_ERR_OK;
  compMsgMsgDesc = self->compMsgMsgDesc;
  msgFieldGroupInfo = &compMsgMsgDesc->msgFieldGroupInfos[compMsgMsgDesc->currMsgFieldGroupInfo];
  if (compMsgMsgDesc->numLineFields < 2) {
    return COMP_MSG_ERR_FIELD_DESC_TOO_FEW_FIELDS;
  }
  //field name
  token = compMsgMsgDesc->lineFields[0];
  result = self->compMsgTypesAndNames->getFieldNameIdFromStr(self->compMsgTypesAndNames, token, &fieldNameId, COMP_MSG_INCR);
  checkErrOK(result);
  //field value
  value = compMsgMsgDesc->lineFields[1];
  stringValue = NULL;
  numericValue = 0;
  if (value[0] == '"') {
    result = compMsgMsgDesc->getStringFieldValue(self, compMsgMsgDesc->lineFields[1], &stringValue);
  } else {
    result = compMsgMsgDesc->getIntFieldValue(self, compMsgMsgDesc->lineFields[1], &ep, 0, &numericValue);
  }
  checkErrOK(result);
  COMP_MSG_DBG(self, "E", 2, "%s: id: %d val: %s %d", token, fieldNameId, stringValue == NULL ? "nil" : (char *)stringValue, numericValue);
  result = self->compMsgDataValue->dataValueStr2ValueId(self, token, &fieldId);
  checkErrOK(result);
  switch (msgFieldGroupInfo->fieldGroupType) {
  case COMP_MSG_WIFI_DATA_VALUES_FIELD_GROUP:
  case COMP_MSG_MODULE_DATA_VALUES_FIELD_GROUP:
    if (stringValue == NULL) {
      dataValue.flags = COMP_MSG_FIELD_IS_NUMERIC;
      dataValue.value.numericValue = numericValue;
    } else {
      dataValue.flags = COMP_MSG_FIELD_IS_STRING;
      dataValue.value.stringValue = stringValue;
    }
    dataValue.cmdKey = COMP_MSG_DATA_VALUE_CMD_KEY_SPECIAL;
    dataValue.fieldNameId = 0;
    dataValue.fieldValueId = fieldId;
    dataValue.fieldValueCallback = NULL;
    result = self->compMsgDataValue->addDataValue(self, &dataValue);
    checkErrOK(result);
    break;
  default:
    return COMP_MSG_ERR_BAD_DESC_FILE_FIELD_GROUP_TYPE;
  }
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
  switch (msgFieldGroupInfo->fieldGroupType) {
  case COMP_MSG_DESC_HEADER_FIELD_GROUP:
  case COMP_MSG_DESC_MID_PART_FIELD_GROUP:
  case COMP_MSG_DESC_TRAILER_FIELD_GROUP:
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
//  case COMP_MSG_VAL_HEADER_FIELD_GROUP:
//    result = compMsgMsgDesc->handleMsgValHeaderLine(self);
//    checkErrOK(result);
//    break;
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
  default:
    COMP_MSG_DBG(self, "E", 0, "bad desc file fieldGroupType 0x%02x", msgFieldGroupInfo->fieldGroupType);
    return COMP_MSG_ERR_BAD_DESC_FILE_FIELD_GROUP_TYPE;
  }
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
  msgFieldDesc_t *msgFieldDesc;
  msgFieldGroupInfo_t *msgFieldGroupInfo;
  msgDescriptionInfos_t *descriptions;
  msgDescription_t *msgDescription;

  result = COMP_MSG_ERR_OK;
  compMsgMsgDesc = self->compMsgMsgDesc;
  result = self->compMsgUtil->addFieldDescription(self);
  checkErrOK(result);
  descriptions = &self->compMsgMsgDesc->msgDescriptionInfos;
  msgDescription = &descriptions->msgDescriptions[descriptions->numMsgDescriptions - 1];
  fieldGroupIdx = 0;
  numHeaderFields = 0;
  while (fieldGroupIdx < compMsgMsgDesc->numMsgFieldGroupInfo) {
    msgFieldGroupInfo = &compMsgMsgDesc->msgFieldGroupInfos[fieldGroupIdx];
    if (msgFieldGroupInfo->fieldGroupType == COMP_MSG_DESC_HEADER_FIELD_GROUP) {
      numHeaderFields = msgFieldGroupInfo->numMsgFieldDesc;
      break;
    }
    fieldGroupIdx++;
  }
  if (numHeaderFields == 0) {
    return COMP_MSG_ERR_HEADER_FIELD_GROUP_NOT_FOUND;
  }
  if (compMsgMsgDesc->numLineFields < 3) {
    return COMP_MSG_ERR_FIELD_DESC_TOO_FEW_FIELDS;
  }
  fieldIdx = 0;
  msgDescription->headerLgth = 0;
  while (fieldIdx < numHeaderFields) {
    msgFieldDesc = &msgFieldGroupInfo->msgFieldDescs[fieldIdx];
    msgDescription->headerLgth += msgFieldDesc->fieldLgth;
    result = self->compMsgTypesAndNames->getFieldNameStrFromId(self->compMsgTypesAndNames, msgFieldDesc->fieldNameId, &fieldNameStr);
    value = compMsgMsgDesc->lineFields[fieldIdx];
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
        result = compMsgMsgDesc->getStringFieldValue(self, value, &stringVal);
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
        result = compMsgMsgDesc->getIntFieldValue(self, value, &ep, 0, &numericValue);
      }
      checkErrOK(result);
    }
    fieldIdx++;
  }
  if (c_strlen(compMsgMsgDesc->lineFields[fieldIdx]) > 1) {
    return COMP_MSG_ERR_BAD_ENCRYPTED_VALUE;
  }
  msgDescription->encrypted = compMsgMsgDesc->lineFields[fieldIdx][0];
  fieldIdx++;
  if (c_strlen(compMsgMsgDesc->lineFields[fieldIdx]) > 1) {
    return COMP_MSG_ERR_BAD_HANDLE_TYPE_VALUE;
  }
  msgDescription->handleType = compMsgMsgDesc->lineFields[fieldIdx][0];
  // FIXME need to handle cmdKey here!!!
  COMP_MSG_DBG(self, "E", 2, "msgDescription->headerLgth: %d encrypted: %c handleType: %c", msgDescription->headerLgth, msgDescription->encrypted, msgDescription->handleType);
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
    result = self->compMsgTypesAndNames->getFieldNameIdFromStr(self->compMsgTypesAndNames, cp, &fieldNameId, COMP_MSG_NO_INCR);
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
  compMsgMsgDesc->openFile = &openFile;
  compMsgMsgDesc->closeFile = &closeFile;
  compMsgMsgDesc->flushFile = &flushFile;
  compMsgMsgDesc->readLine = &readLine;
  compMsgMsgDesc->writeLine = &writeLine;
  compMsgMsgDesc->getLineFields = &getLineFields;

  compMsgMsgDesc->getIntFieldValue = &getIntFieldValue;
  compMsgMsgDesc->getStringFieldValue = &getStringFieldValue;
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

#ifdef OLD
  compMsgMsgDesc->getHeaderFieldsFromLine = &getHeaderFieldsFromLine;
  compMsgMsgDesc->readModuleDataValues = &readModuleDataValues;
  compMsgMsgDesc->readWifiValues = &readWifiValues;
  compMsgMsgDesc->readHeadersAndSetFlags = &readHeadersAndSetFlags;
  compMsgMsgDesc->getMsgPartsFromHeaderPart = &getMsgPartsFromHeaderPart;
  compMsgMsgDesc->getHeaderFromUniqueFields = &getHeaderFromUniqueFields;
  compMsgMsgDesc->getMsgKeyValueDescParts = &getMsgKeyValueDescParts;
  compMsgMsgDesc->getWifiKeyValueKeys = &getWifiKeyValueKeys;
#endif

  result = compMsgMsgDesc->handleMsgFile(self, MSG_FILES_FILE_NAME, compMsgMsgDesc->handleMsgFileNameLine);
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
