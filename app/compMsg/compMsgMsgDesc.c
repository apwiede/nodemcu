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

// ================================= addUseFileName ====================================

static uint8_t addUseFileName(compMsgDispatcher_t *self, char *fileName) {
  uint8_t result;
  msgDescIncludeInfo_t *msgDescIncludeInfo;

  result = COMP_MSG_ERR_OK;
  if (self->compMsgMsgDesc->numMsgDescIncludeInfo >= self->compMsgMsgDesc->maxMsgDescIncludeInfo) {
    if (self->compMsgMsgDesc->maxMsgDescIncludeInfo == 0) {
      self->compMsgMsgDesc->maxMsgDescIncludeInfo = 5;
      self->compMsgMsgDesc->msgDescIncludeInfos = (msgDescIncludeInfo_t *)os_zalloc((self->compMsgMsgDesc->maxMsgDescIncludeInfo * sizeof(msgDescIncludeInfo_t)));
      checkAllocOK(self->compMsgMsgDesc->msgDescIncludeInfos);
    } else {
      self->compMsgMsgDesc->maxMsgDescIncludeInfo += 2;
      self->compMsgMsgDesc->msgDescIncludeInfos = (msgDescIncludeInfo_t *)os_realloc((self->compMsgMsgDesc->msgDescIncludeInfos), (self->compMsgMsgDesc->maxMsgDescIncludeInfo * sizeof(msgDescIncludeInfo_t)));
      checkAllocOK(self->compMsgMsgDesc->msgDescIncludeInfos);
    }
  }
  msgDescIncludeInfo = &self->compMsgMsgDesc->msgDescIncludeInfos[self->compMsgMsgDesc->numMsgDescIncludeInfo];
  memset(msgDescIncludeInfo, 0, sizeof(msgDescIncludeInfo_t));
  msgDescIncludeInfo->fileName = os_zalloc(c_strlen(fileName) + 1);
  c_memcpy(msgDescIncludeInfo->fileName, fileName, c_strlen(fileName));
  self->compMsgMsgDesc->currMsgDescIncludeInfo = self->compMsgMsgDesc->numMsgDescIncludeInfo;
  self->compMsgMsgDesc->numMsgDescIncludeInfo++;
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
  msgDescIncludeInfo_t *msgDescIncludeInfo;

  compMsgMsgDesc = self->compMsgMsgDesc;
  buffer = buf;
  result = compMsgMsgDesc->openFile(compMsgMsgDesc, fileName, "r");
//ets_printf("fileName: %s result: %d\n", fileName, result);
  checkErrOK(result);
#undef checkErrOK
#define checkErrOK(result) if(result != COMP_MSG_ERR_OK) { compMsgMsgDesc->closeFile(compMsgMsgDesc); return result; }
  numLines = 0;
  while (1) {
    result = compMsgMsgDesc->readLine(compMsgMsgDesc, &buffer, &lgth);
    checkErrOK(result);
    if (lgth == 0) {
      if (numLines - 1 != self->compMsgMsgDesc->expectedLines) {
ets_printf("Error numLines: %d expectedLines: %d\n", numLines, self->compMsgMsgDesc->expectedLines);
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
//ets_printf("expectedLines: %d\n", self->compMsgMsgDesc->expectedLines);
      } else {
        COMP_MSG_DBG(self, "E", 0, "wrong desc file number lines line");
        return COMP_MSG_ERR_WRONG_DESC_FILE_LINE;
      }
    } else {
      // check if it is a #use line
      if (c_strcmp(compMsgMsgDesc->lineFields[0], "#use") == 0) {
        if (compMsgMsgDesc->numLineFields != 2) {
          COMP_MSG_DBG(self, "E", 0, "bad desc file #use line");
          return COMP_MSG_ERR_BAD_DESC_FILE_USE_LINE;
        } else {
          if (compMsgMsgDesc->lineFields[1][0] == '@') {
          } else {
            result = compMsgMsgDesc->addUseFileName(self, compMsgMsgDesc->lineFields[1]);
            checkErrOK(result);
          }
        }
      } else {
//ets_printf("numLines: %d expectedLines: %d\n", numLines - 1, compMsgMsgDesc->expectedLines);
        result = handleMsgLine(self);
        checkErrOK(result);
      }
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
  msgDescIncludeInfo_t *msgDescIncludeInfo;

ets_printf(">>>handleMsgFile %s\n", fileName);
  result = handleMsgFileInternal(self, fileName, handleMsgLine);
  checkErrOK(result);
  compMsgMsgDesc = self->compMsgMsgDesc;
  if (compMsgMsgDesc->numMsgDescIncludeInfo > 0) {
    // handle use files here
    compMsgMsgDesc->currMsgDescIncludeInfo = 0;
    while (compMsgMsgDesc->currMsgDescIncludeInfo < compMsgMsgDesc->numMsgDescIncludeInfo) {
      msgDescIncludeInfo = &compMsgMsgDesc->msgDescIncludeInfos[compMsgMsgDesc->currMsgDescIncludeInfo];
ets_printf(">>>handleFile %s\n", msgDescIncludeInfo->fileName);
      result = handleMsgFileInternal(self, msgDescIncludeInfo->fileName, compMsgMsgDesc->handleMsgUseLine);
      checkErrOK(result);
ets_printf("<<<%s: done\n", msgDescIncludeInfo->fileName);
      compMsgMsgDesc->currMsgDescIncludeInfo++;
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
  msgDescIncludeInfo_t *msgDescIncludeInfo;
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
ets_printf("token: %s field: %s\n", token, field);
  if (c_strncmp(token, "@$", 2) == 0) {
    result = self->compMsgTypesAndNames->getFileNameTokenIdFromStr(self->compMsgTypesAndNames, token, &fileNameTokenId);
    checkErrOK(result);
    compMsgMsgDesc->msgUseFileName = os_zalloc(c_strlen(field) + 1);
    checkAllocOK(compMsgMsgDesc->msgUseFileName);
    c_memcpy(compMsgMsgDesc->msgUseFileName, field, c_strlen(field));
    if (fileNameTokenId != 0) {
      result = addUseFileName(self, field);
      checkErrOK(result);
      msgDescIncludeInfo = &compMsgMsgDesc->msgDescIncludeInfos[compMsgMsgDesc->currMsgDescIncludeInfo];
      msgDescIncludeInfo->includeType = fileNameTokenId;
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
  msgDescIncludeInfo_t *msgDescIncludeInfo;

  result = COMP_MSG_ERR_OK;
  compMsgMsgDesc = self->compMsgMsgDesc;
//ets_printf("handleMsgCommonLine: numFields: %d\n", compMsgMsgDesc->numLineFields);
  msgDescIncludeInfo = &compMsgMsgDesc->msgDescIncludeInfos[compMsgMsgDesc->currMsgDescIncludeInfo];
  if (compMsgMsgDesc->numLineFields < 3) {
    return COMP_MSG_ERR_FIELD_DESC_TOO_FEW_FIELDS;
  }
  if (msgDescIncludeInfo->maxMsgFieldDesc == 0) {
    msgDescIncludeInfo->maxMsgFieldDesc = self->compMsgMsgDesc->expectedLines;
    msgDescIncludeInfo->msgFieldDescs = os_zalloc(msgDescIncludeInfo->maxMsgFieldDesc * sizeof(msgFieldDesc_t));
    checkAllocOK(msgDescIncludeInfo->msgFieldDescs);
  }
  msgFieldDesc = &msgDescIncludeInfo->msgFieldDescs[msgDescIncludeInfo->numMsgFieldDesc];
//ets_printf("handleMsgCommonLine: %s numFieldDesc: %d\n", msgDescIncludeInfo->fileName, msgDescIncludeInfo->numMsgFieldDesc);
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
ets_printf("%s: id: %d type: %s %d lgth: %d\n", compMsgMsgDesc->lineFields[0], msgFieldDesc->fieldNameId, compMsgMsgDesc->lineFields[1], msgFieldDesc->fieldTypeId, msgFieldDesc->fieldLgth);

int idx;
idx = 0;
while (idx < self->compMsgMsgDesc->numLineFields) {
//ets_printf("field: %d %s\n", idx, self->compMsgMsgDesc->lineFields[idx]);
  idx++;
}
  msgDescIncludeInfo->numMsgFieldDesc++;
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
  msgFieldVal_t *msgFieldVal;
  msgDescIncludeInfo_t *msgDescIncludeInfo;

  result = COMP_MSG_ERR_OK;
  compMsgMsgDesc = self->compMsgMsgDesc;
//ets_printf("handleMsgFieldsToSaveLine: numFields: %d\n", compMsgMsgDesc->numLineFields);
  msgDescIncludeInfo = &compMsgMsgDesc->msgDescIncludeInfos[compMsgMsgDesc->currMsgDescIncludeInfo];
  if (compMsgMsgDesc->numLineFields < 1) {
    return COMP_MSG_ERR_FIELD_DESC_TOO_FEW_FIELDS;
  }
  if (msgDescIncludeInfo->maxMsgFieldDesc == 0) {
    msgDescIncludeInfo->maxMsgFieldDesc = self->compMsgMsgDesc->expectedLines;
    msgDescIncludeInfo->msgFieldDescs = os_zalloc(msgDescIncludeInfo->maxMsgFieldDesc * sizeof(msgFieldDesc_t));
    checkAllocOK(msgDescIncludeInfo->msgFieldDescs);
  }
  msgFieldDesc = &msgDescIncludeInfo->msgFieldDescs[msgDescIncludeInfo->numMsgFieldDesc];
  //field name
  result = self->compMsgTypesAndNames->getFieldNameIdFromStr(self->compMsgTypesAndNames, compMsgMsgDesc->lineFields[0], &msgFieldDesc->fieldNameId, COMP_MSG_INCR);
ets_printf("%s: id: %d\n", compMsgMsgDesc->lineFields[0], msgFieldDesc->fieldNameId);
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
  msgFieldVal_t *msgFieldVal;
  msgDescIncludeInfo_t *msgDescIncludeInfo;

  result = COMP_MSG_ERR_OK;
  compMsgMsgDesc = self->compMsgMsgDesc;
//ets_printf("handleMsgActionsLine: numFields: %d\n", compMsgMsgDesc->numLineFields);
  msgDescIncludeInfo = &compMsgMsgDesc->msgDescIncludeInfos[compMsgMsgDesc->currMsgDescIncludeInfo];
  if (compMsgMsgDesc->numLineFields < 2) {
    return COMP_MSG_ERR_FIELD_DESC_TOO_FEW_FIELDS;
  }
  if (msgDescIncludeInfo->maxMsgFieldDesc == 0) {
    msgDescIncludeInfo->maxMsgFieldDesc = self->compMsgMsgDesc->expectedLines;
    msgDescIncludeInfo->msgFieldDescs = os_zalloc(msgDescIncludeInfo->maxMsgFieldDesc * sizeof(msgFieldDesc_t));
    checkAllocOK(msgDescIncludeInfo->msgFieldDescs);
  }
  msgFieldDesc = &msgDescIncludeInfo->msgFieldDescs[msgDescIncludeInfo->numMsgFieldDesc];
  //field name
  result = self->compMsgTypesAndNames->getFieldNameIdFromStr(self->compMsgTypesAndNames, compMsgMsgDesc->lineFields[0], &msgFieldDesc->fieldNameId, COMP_MSG_INCR);
  checkErrOK(result);
  //field value
ets_printf("%s: id: %d val: %s\n", compMsgMsgDesc->lineFields[0], msgFieldDesc->fieldNameId, compMsgMsgDesc->lineFields[1]);
  checkErrOK(result);
  return result;
}

// ================================= handleMsgValuesLine ====================================

static uint8_t handleMsgValuesLine(compMsgDispatcher_t *self) {
  int result;
  int lgth;
  uint8_t fieldNameId;
  char *ep;
  uint8_t *stringVal;
  compMsgMsgDesc_t *compMsgMsgDesc;
  msgDescIncludeInfo_t *msgDescIncludeInfo;

  result = COMP_MSG_ERR_OK;
  compMsgMsgDesc = self->compMsgMsgDesc;
//ets_printf("handleMsgActionsLine: numFields: %d\n", compMsgMsgDesc->numLineFields);
  msgDescIncludeInfo = &compMsgMsgDesc->msgDescIncludeInfos[compMsgMsgDesc->currMsgDescIncludeInfo];
  if (compMsgMsgDesc->numLineFields < 2) {
    return COMP_MSG_ERR_FIELD_DESC_TOO_FEW_FIELDS;
  }
  //field name
  result = self->compMsgTypesAndNames->getFieldNameIdFromStr(self->compMsgTypesAndNames, compMsgMsgDesc->lineFields[0], &fieldNameId, COMP_MSG_INCR);
  checkErrOK(result);
  //field value
ets_printf("%s: id: %d val: %s incType: 0x%02x\n", compMsgMsgDesc->lineFields[0], fieldNameId, compMsgMsgDesc->lineFields[1], msgDescIncludeInfo->includeType);
  checkErrOK(result);
  switch (msgDescIncludeInfo->includeType) {
  case COMP_MSG_WIFI_DATA_VALUES_FILE_TOKEN:
    break;
  case COMP_MSG_MODULE_DATA_VALUES_FILE_TOKEN:
    break;
  default:
    return COMP_MSG_ERR_BAD_DESC_FILE_FIELD_INCLUDE_TYPE;
  }
  return result;
}

// ================================= handleMsgValHeaderLine ====================================

static uint8_t handleMsgValHeaderLine(compMsgDispatcher_t *self) {
  int result;
  int lgth;
  uint8_t fieldNameId;
  char *ep;
  uint8_t *stringVal;
  compMsgMsgDesc_t *compMsgMsgDesc;
  msgFieldDesc_t *msgFieldDesc;
  msgFieldVal_t *msgFieldVal;
  msgDescIncludeInfo_t *msgDescIncludeInfo;

  result = COMP_MSG_ERR_OK;
  compMsgMsgDesc = self->compMsgMsgDesc;
//ets_printf("handleMsgValHeaderLine: numFields: %d\n", compMsgMsgDesc->numLineFields);
  msgDescIncludeInfo = &compMsgMsgDesc->msgDescIncludeInfos[compMsgMsgDesc->currMsgDescIncludeInfo];
  if (compMsgMsgDesc->numLineFields < 2) {
    return COMP_MSG_ERR_FIELD_DESC_TOO_FEW_FIELDS;
  }
  if (msgDescIncludeInfo->maxMsgFieldVal == 0) {
    msgDescIncludeInfo->maxMsgFieldVal = self->compMsgMsgDesc->expectedLines;
    msgDescIncludeInfo->msgFieldVals = os_zalloc(msgDescIncludeInfo->maxMsgFieldVal * sizeof(msgFieldVal_t));
    checkAllocOK(msgDescIncludeInfo->msgFieldVals);
  }
  msgFieldVal = &msgDescIncludeInfo->msgFieldVals[msgDescIncludeInfo->numMsgFieldVal];
//ets_printf("handleMsgUseLine: %s numFieldVal: %d\n", msgDescIncludeInfo->fileName, msgDescIncludeInfo->numMsgFieldVal);
  //field name
  result = self->compMsgTypesAndNames->getFieldNameIdFromStr(self->compMsgTypesAndNames, compMsgMsgDesc->lineFields[0], &fieldNameId, COMP_MSG_INCR);
  checkErrOK(result);
  // field value
  if (compMsgMsgDesc->lineFields[1][0] == '@') {
    msgFieldVal->value.u8vec = os_zalloc(c_strlen(compMsgMsgDesc->lineFields[1]) + 1);
    checkAllocOK(msgFieldVal->value.u8vec);
    c_memcpy(msgFieldVal->value.u8vec, compMsgMsgDesc->lineFields[1], c_strlen(compMsgMsgDesc->lineFields[1]));
    // FIXME: need code for value callback here !!!
ets_printf("%s: id: %d %s!\n", compMsgMsgDesc->lineFields[0], fieldNameId, msgFieldVal->value.u8vec);
  } else {
    if (compMsgMsgDesc->lineFields[1][0] == '"') {
      result = compMsgMsgDesc->getStringFieldValue(self, compMsgMsgDesc->lineFields[1], &stringVal);
      msgFieldVal->value.u8vec = os_zalloc(c_strlen(stringVal) + 1);
      checkAllocOK(msgFieldVal->value.u8vec);
      c_memcpy(msgFieldVal->value.u8vec, stringVal, c_strlen(stringVal));
ets_printf("%s: id: %d val: %s!\n", compMsgMsgDesc->lineFields[0], fieldNameId, msgFieldVal->value.u8vec);
    } else {
      result = compMsgMsgDesc->getIntFieldValue(self, compMsgMsgDesc->lineFields[1], &ep, 0, &msgFieldVal->value.intVal);
ets_printf("%s: id: %d %d 0x%08x\n", compMsgMsgDesc->lineFields[0], fieldNameId, msgFieldVal->value.intVal, msgFieldVal->value.intVal);
    }
    checkErrOK(result);
  }
  msgDescIncludeInfo->numMsgFieldVal++;
  return result;
}

// ================================= handleMsgUseLine ====================================

static uint8_t handleMsgUseLine(compMsgDispatcher_t *self) {
  int result;
  int lgth;
  uint8_t fieldNameId;
  char *ep;
  uint8_t *stringVal;
  compMsgMsgDesc_t *compMsgMsgDesc;
  msgFieldDesc_t *msgFieldDesc;
  msgFieldVal_t *msgFieldVal;
  msgDescIncludeInfo_t *msgDescIncludeInfo;

  result = COMP_MSG_ERR_OK;
  compMsgMsgDesc = self->compMsgMsgDesc;
//ets_printf("handleMsgUseLine: numFields: %d\n", compMsgMsgDesc->numLineFields);
  msgDescIncludeInfo = &compMsgMsgDesc->msgDescIncludeInfos[compMsgMsgDesc->currMsgDescIncludeInfo];
  switch (msgDescIncludeInfo->includeType) {
  case COMP_MSG_DESC_HEADER_FILE_TOKEN:
  case COMP_MSG_DESC_MID_PART_FILE_TOKEN:
  case COMP_MSG_DESC_TRAILER_FILE_TOKEN:
    result = compMsgMsgDesc->handleMsgCommonLine(self);
    checkErrOK(result);
    break;
  case COMP_MSG_FIELDS_TO_SAVE_FILE_TOKEN:
    result = compMsgMsgDesc->handleMsgFieldsToSaveLine(self);
    checkErrOK(result);
    break;
  case COMP_MSG_ACTIONS_FILE_TOKEN:
    result = compMsgMsgDesc->handleMsgActionsLine(self);
    checkErrOK(result);
    break;
  case COMP_MSG_VAL_HEADER_FILE_TOKEN:
    result = compMsgMsgDesc->handleMsgValHeaderLine(self);
    checkErrOK(result);
    break;
  case COMP_MSG_HEADS_FILE_TOKEN:
    result = self->compMsgMsgDesc->handleMsgHeadsLine(self);
    checkErrOK(result);
    break;
  case COMP_MSG_WIFI_DATA_VALUES_FILE_TOKEN:
    result = self->compMsgMsgDesc->handleMsgValuesLine(self);
    checkErrOK(result);
    break;
  case COMP_MSG_MODULE_DATA_VALUES_FILE_TOKEN:
    result = self->compMsgMsgDesc->handleMsgValuesLine(self);
    checkErrOK(result);
    break;
  default:
    COMP_MSG_DBG(self, "E", 0, "bad desc file includeType 0x%02x", msgDescIncludeInfo->includeType);
    return COMP_MSG_ERR_BAD_DESC_FILE_FIELD_INCLUDE_TYPE;
  }
  return result;
}

// ================================= handleMsgHeadsLine ====================================

static uint8_t handleMsgHeadsLine(compMsgDispatcher_t *self) {
  uint8_t result;
  int lgth;
  int includeIdx;
  int fieldIdx;
  uint8_t numHeaderFields;
  uint8_t *fieldNameStr;
  uint8_t *value;
  uint8_t *stringVal;
  char *ep;
  compMsgMsgDesc_t *compMsgMsgDesc;
  msgFieldDesc_t *msgFieldDesc;
  msgFieldVal_t *msgFieldVal;
  msgDescIncludeInfo_t *msgDescIncludeInfo;
  msgDescriptionInfos_t *descriptions;
  msgDescription_t *msgDescription;

  result = COMP_MSG_ERR_OK;
  compMsgMsgDesc = self->compMsgMsgDesc;
//ets_printf("handleMsgHeadsLine: numFields: %d\n", compMsgMsgDesc->numLineFields);
  result = self->compMsgUtil->addFieldDescription(self);
  checkErrOK(result);
  descriptions = &self->compMsgMsgDesc->msgDescriptionInfos;
  msgDescription = &descriptions->msgDescriptions[descriptions->numMsgDescriptions - 1];
  includeIdx = 0;
  numHeaderFields = 0;
  while (includeIdx < compMsgMsgDesc->numMsgDescIncludeInfo) {
    msgDescIncludeInfo = &compMsgMsgDesc->msgDescIncludeInfos[includeIdx];
    if (msgDescIncludeInfo->includeType == COMP_MSG_DESC_HEADER_FILE_TOKEN) {
      numHeaderFields = msgDescIncludeInfo->numMsgFieldDesc;
      break;
    }
    includeIdx++;
  }
  if (numHeaderFields == 0) {
    return COMP_MSG_ERR_HEADER_INCLUDE_NOT_FOUND;
  }
  if (compMsgMsgDesc->numLineFields < 3) {
    return COMP_MSG_ERR_FIELD_DESC_TOO_FEW_FIELDS;
  }
  fieldIdx = 0;
  msgDescription->headerLgth = 0;
  while (fieldIdx < numHeaderFields) {
    msgFieldDesc = &msgDescIncludeInfo->msgFieldDescs[fieldIdx];
    msgDescription->headerLgth += msgFieldDesc->fieldLgth;
    result = self->compMsgTypesAndNames->getFieldNameStrFromId(self->compMsgTypesAndNames, msgFieldDesc->fieldNameId, &fieldNameStr);
    value = compMsgMsgDesc->lineFields[fieldIdx];
ets_printf("field: %s %s\n", fieldNameStr, value);
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
ets_printf("value: %s!stringVal: %s\n", value, stringVal);
        msgFieldDesc->msgFieldVal.value.u8vec = os_zalloc(c_strlen(stringVal) + 1);
        checkAllocOK(msgFieldDesc->msgFieldVal.value.u8vec);
        c_memcpy(msgFieldDesc->msgFieldVal.value.u8vec, stringVal, c_strlen(stringVal));
ets_printf("field2: %s %s\n", fieldNameStr, msgFieldDesc->msgFieldVal.value.u8vec);
      } else {
        result = compMsgMsgDesc->getIntFieldValue(self, value, &ep, 0, &msgFieldDesc->msgFieldVal.value.intVal);
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
ets_printf("msgDescription->headerLgth: %d encrypted: %c handleType: %c\n", msgDescription->headerLgth, msgDescription->encrypted, msgDescription->handleType);
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
  
// ================================= readWifiValues ====================================

static uint8_t readWifiValues(compMsgDispatcher_t *self, uint8_t *fileName) {
  int result;
  long uval;
  uint8_t numEntries;
  uint8_t*cp;
  uint8_t*ep;
  char *endPtr;
  int idx;
  bool isEnd;
  uint8_t lgth;
  uint8_t buf[256];
  uint8_t *buffer;
  uint8_t *myStr;
  uint8_t *fieldNameStr;
  uint8_t *fieldValueStr;
  uint8_t fieldTypeId;
  compMsgMsgDesc_t *compMsgMsgDesc;

  compMsgMsgDesc = self->compMsgMsgDesc;
  buffer = buf;
  result = compMsgMsgDesc->openFile(compMsgMsgDesc, fileName, "r");
  COMP_MSG_DBG(self, "E", 2, "readWifiValues: %s\n", fileName);
  checkErrOK(result);
#undef checkErrOK
#define checkErrOK(result) if(result != COMP_MSG_ERR_OK) { compMsgMsgDesc->closeFile(compMsgMsgDesc); return result; }
  result = compMsgMsgDesc->readLine(compMsgMsgDesc, &buffer, &lgth);
  checkErrOK(result);
  buffer[lgth] = 0;
  if ((lgth < 4) || (buffer[0] != '#')) {
     return COMP_MSG_ERR_BAD_FILE_CONTENTS;
  return COMP_MSG_ERR_OK;
  }
  uval = c_strtoul(buffer+2, &endPtr, 10);
  numEntries = (uint8_t)uval;
  idx = 0;
  while(idx < numEntries) {
    result = compMsgMsgDesc->readLine(compMsgMsgDesc, &buffer, &lgth);
    checkErrOK(result);
    if (lgth == 0) {
      return COMP_MSG_ERR_TOO_FEW_FILE_LINES;
    }
    buffer[lgth] = 0;
    cp = buffer;
    // wifiFieldName
    fieldNameStr = cp;
    result = compMsgMsgDesc->getStrFromLine(self, cp, &ep, &isEnd);
    checkErrOK(result);
    checkIsEnd(isEnd);
    cp = ep;

    // wifiFieldValue
    fieldValueStr = cp;
    result = compMsgMsgDesc->getStrFromLine(self, cp, &ep, &isEnd);
    checkErrOK(result);
    if (ep == cp + 1) {
      // empty value
      result = self->compMsgWifiData->setWifiValue(self, fieldNameStr, 0, "");
    } else {
      uval = c_strtoul(fieldValueStr, &endPtr, 10);
      if ((endPtr - (char *)fieldValueStr) == c_strlen(fieldValueStr)) {
        if (c_strlen(fieldValueStr) > 10) {
          // seems to be a password key, so use the stringValue
          result = self->compMsgWifiData->setWifiValue(self, fieldNameStr, 0, fieldValueStr);
        } else {
          result = self->compMsgWifiData->setWifiValue(self, fieldNameStr, uval, NULL);
        }
      } else {
        result = self->compMsgWifiData->setWifiValue(self, fieldNameStr, 0, fieldValueStr);
      }
    }
    if (!isEnd) {
      return COMP_MSG_ERR_FUNNY_EXTRA_FIELDS;
    }
    checkErrOK(result);
    idx++;
  }
#undef checkErrOK
#define checkErrOK(result) if(result != COMP_MSG_ERR_OK) return result
  result = compMsgMsgDesc->closeFile(compMsgMsgDesc);
  checkErrOK(result);
  return COMP_MSG_ERR_OK;
}

// ================================= readModuleDataValues ====================================

static uint8_t readModuleDataValues(compMsgDispatcher_t *self, uint8_t *fileName) {
  int result;
  long uval;
  uint8_t numEntries;
  uint8_t*cp;
  uint8_t*ep;
  char *endPtr;
  int idx;
  bool isEnd;
  uint8_t lgth;
  uint8_t buf[100];
  uint8_t *buffer;
  uint8_t *myStr;
  uint8_t *fieldNameStr;
  uint8_t *fieldValueStr;
  uint8_t fieldTypeId;
  compMsgMsgDesc_t *compMsgMsgDesc;

  compMsgMsgDesc = self->compMsgMsgDesc;
  buffer = buf;
  result = compMsgMsgDesc->openFile(compMsgMsgDesc, fileName, "r");
  checkErrOK(result);
#undef checkErrOK
#define checkErrOK(result) if(result != COMP_MSG_ERR_OK) { compMsgMsgDesc->closeFile(compMsgMsgDesc); return result; }
  result = compMsgMsgDesc->readLine(compMsgMsgDesc, &buffer, &lgth);
  checkErrOK(result);
  buffer[lgth] = 0;
  if ((lgth < 4) || (buffer[0] != '#')) {
     return COMP_MSG_ERR_BAD_FILE_CONTENTS;
  return COMP_MSG_ERR_OK;
  }
  uval = c_strtoul(buffer+2, &endPtr, 10);
  numEntries = (uint8_t)uval;
  idx = 0;
  while(idx < numEntries) {
    result = compMsgMsgDesc->readLine(compMsgMsgDesc, &buffer, &lgth);
    checkErrOK(result);
    if (lgth == 0) {
      return COMP_MSG_ERR_TOO_FEW_FILE_LINES;
    }
    buffer[lgth] = 0;
    cp = buffer;
    // wifiFieldName
    fieldNameStr = cp;
    result = compMsgMsgDesc->getStrFromLine(self, cp, &ep, &isEnd);
    checkErrOK(result);
    checkIsEnd(isEnd);
    cp = ep;

    // wifiFieldValue
    fieldValueStr = cp;
    result = compMsgMsgDesc->getStrFromLine(self, cp, &ep, &isEnd);
    checkErrOK(result);
    uval = c_strtoul(fieldValueStr, &endPtr, 10);
    if ((endPtr - (char *)fieldValueStr) == c_strlen(fieldValueStr)) {
      if (c_strlen(fieldValueStr) > 10) {
        // seems to be a password key, so use the stringValue
        result = self->compMsgWifiData->setWifiValue(self, fieldNameStr, 0, fieldValueStr);
      } else {
        result = self->compMsgWifiData->setWifiValue(self, fieldNameStr, uval, NULL);
      }
    } else {
      result = self->compMsgWifiData->setWifiValue(self, fieldNameStr, 0, fieldValueStr);
    }
    if (!isEnd) {
      return COMP_MSG_ERR_FUNNY_EXTRA_FIELDS;
    }
    checkErrOK(result);
    idx++;
  }
#undef checkErrOK
#define checkErrOK(result) if(result != COMP_MSG_ERR_OK) return result
  result = compMsgMsgDesc->closeFile(compMsgMsgDesc);
  checkErrOK(result);
  return COMP_MSG_ERR_OK;
}

// ================================= getMsgPartsFromHeaderPart ====================================

static uint8_t getMsgPartsFromHeaderPart (compMsgDispatcher_t *self, headerPart_t *hdr, uint8_t **handle) {
  uint8_t result;
  char fileName[100];
  uint8_t *fieldValueStr;
  uint8_t *fieldNameStr;
  uint8_t *fieldTypeStr;
  uint8_t *fieldLgthStr;
  uint8_t fieldNameId;
  uint8_t fieldTypeId;
  char *endPtr;
  uint8_t lgth;
  uint8_t fieldLgth;
  uint8_t buf[100];
  uint8_t *buffer = buf;
  long uval;
  uint8_t *myStr;
  int idx;
  int numericValue;
  uint8_t numRows;
  uint8_t*cp;
  uint8_t*ep;
  uint8_t *keyValueCallback;
  bool isEnd;
  msgDescPart_t *msgDescPart;
  msgValPart_t *msgValPart;
  compMsgData_t *compMsgData;
  compMsgMsgDesc_t *compMsgMsgDesc;

  COMP_MSG_DBG(self, "E", 2, "getMsgPartsFromHeaderPart1\n");
#ifdef OLD
  compMsgData = self->compMsgData;
  compMsgMsgDesc = self->compMsgMsgDesc;
  self->compMsgData->currHdr = hdr;
  os_sprintf(fileName, "CompDesc%c%c.txt", (hdr->hdrU16CmdKey>>8)&0xFF, hdr->hdrU16CmdKey&0xFF);
  COMP_MSG_DBG(self, "E", 2, "file: %s\n", fileName);
  result = compMsgMsgDesc->openFile(compMsgMsgDesc, fileName, "r");
  checkErrOK(result);
#undef checkErrOK
#define checkErrOK(result) if(result != COMP_MSG_ERR_OK) { compMsgMsgDesc->closeFile(compMsgMsgDesc); return result; }
  result = compMsgMsgDesc->readLine(compMsgMsgDesc, &buffer, &lgth);
  checkErrOK(result);
  buffer[lgth] = 0;
  cp = buffer;
  result = compMsgMsgDesc->getStrFromLine(self, cp, &ep, &isEnd);
  checkErrOK(result);
  if (isEnd) {
    return COMP_MSG_ERR_BAD_FILE_CONTENTS;
  }
  if (cp[0] != '#') {
    return COMP_MSG_ERR_BAD_FILE_CONTENTS;
  }
  cp = ep;
  result = compMsgData->deleteMsgDescParts(self);
  checkErrOK(result);
  result = compMsgMsgDesc->getIntFromLine(self, cp, &uval, &ep, &isEnd);
  checkErrOK(result);
  compMsgData->maxMsgDescParts = (uint8_t)uval;
  cp = ep;
  if (self->compMsgData->prepareValuesCbName != NULL) {
    os_free(self->compMsgData->prepareValuesCbName);
    self->compMsgData->prepareValuesCbName = NULL;
  }
  if (!isEnd) {
    result = compMsgMsgDesc->getStrFromLine(self, cp, &ep, &isEnd);
    checkErrOK(result);
    compMsgData->prepareValuesCbName = os_zalloc(c_strlen(cp) + 1);
    checkAllocOK(compMsgData->prepareValuesCbName);
    c_memcpy(compMsgData->prepareValuesCbName, cp, c_strlen(cp));
    compMsgData->prepareValuesCbName[c_strlen(cp)] = '\0';
    COMP_MSG_DBG(self, "E", 2, "prepareValuesCbName: %s\n", compMsgData->prepareValuesCbName);
  }
  if (!isEnd) {
    return COMP_MSG_ERR_FUNNY_EXTRA_FIELDS;
  }
  compMsgData->msgDescParts = os_zalloc(sizeof(msgDescPart_t) * compMsgData->maxMsgDescParts);
  checkAllocOK(compMsgData->msgDescParts);
  numRows = 0;
  idx = 0;
  while(idx < compMsgData->maxMsgDescParts) {
    result = compMsgMsgDesc->readLine(compMsgMsgDesc, &buffer, &lgth);
    checkErrOK(result);
    if (lgth == 0) {
      return COMP_MSG_ERR_TOO_FEW_FILE_LINES;
    }
    msgDescPart = &compMsgData->msgDescParts[compMsgData->numMsgDescParts++];
    buffer[lgth] = 0;
    cp = buffer;
    // fieldName
    fieldNameStr = cp;
    result = compMsgMsgDesc->getStrFromLine(self, cp, &ep, &isEnd);
    checkErrOK(result);
    checkIsEnd(isEnd);
    cp = ep;
    msgDescPart->fieldNameStr = os_zalloc(c_strlen(fieldNameStr)+ 1);
    checkAllocOK(msgDescPart->fieldNameStr);
    c_memcpy(msgDescPart->fieldNameStr, fieldNameStr, c_strlen(fieldNameStr));

    // fieldType
    fieldTypeStr = cp;
    result = compMsgMsgDesc->getStrFromLine(self, cp, &ep, &isEnd);
    checkErrOK(result);
    checkIsEnd(isEnd);
    cp = ep;
    msgDescPart->fieldTypeStr = os_zalloc(c_strlen(fieldTypeStr)+ 1);
    checkAllocOK(msgDescPart->fieldTypeStr);
    c_memcpy(msgDescPart->fieldTypeStr, fieldTypeStr, c_strlen(fieldTypeStr));

    // fieldLgth
    fieldLgthStr = cp;
    result = compMsgMsgDesc->getStrFromLine(self, cp, &ep, &isEnd);
    checkErrOK(result);
    if (c_strcmp(fieldLgthStr,"@numRows") == 0) {
      fieldLgth = numRows;
    } else {
      lgth = c_strtoul(fieldLgthStr, &endPtr, 10);
      fieldLgth = (uint8_t)lgth;
    }
    msgDescPart->fieldLgth = fieldLgth;
    result = self->compMsgTypesAndNames->getFieldTypeIdFromStr(self->compMsgTypesAndNames, fieldTypeStr, &msgDescPart->fieldTypeId);
    checkErrOK(result);
    result = self->compMsgTypesAndNames->getFieldNameIdFromStr(self->compMsgTypesAndNames, fieldNameStr, &msgDescPart->fieldNameId, COMP_MSG_INCR);
    checkErrOK(result);
    cp = ep;
    // eventually a callback for key value entries
    if (!isEnd) {
      keyValueCallback = cp;
      result = compMsgMsgDesc->getStrFromLine(self, cp, &ep, &isEnd);
      checkErrOK(result);
      result = self->compMsgUtil->getFieldValueCallback(self, cp, &msgDescPart->fieldSizeCallback);
      checkErrOK(result);
    }
//self->compMsgDebug->dumpMsgDescPart(self, msgDescPart);
    if (!isEnd) {
      return COMP_MSG_ERR_FUNNY_EXTRA_FIELDS;
    }
    cp = ep;
    idx++;
  }
  result = compMsgMsgDesc->closeFile(compMsgMsgDesc);
  checkErrOK(result);

  // and now the value parts
  os_sprintf(fileName, "CompVal%c%c.txt", (hdr->hdrU16CmdKey >> 8) & 0xFF, hdr->hdrU16CmdKey & 0xFF);
  result = compMsgMsgDesc->openFile(compMsgMsgDesc, fileName, "r");
  checkErrOK(result);
  result = compMsgMsgDesc->readLine(compMsgMsgDesc, &buffer, &lgth);
  checkErrOK(result);
  buffer[lgth] = 0;
  cp = buffer;
  result = compMsgMsgDesc->getStrFromLine(self, cp, &ep, &isEnd);
  checkErrOK(result);
  if (isEnd) {
    return COMP_MSG_ERR_BAD_FILE_CONTENTS;
  }
  if (cp[0] != '#') {
    return COMP_MSG_ERR_BAD_FILE_CONTENTS;
  }
  cp = ep;
  result = compMsgData->deleteMsgValParts(self);
  checkErrOK(result);
  result = compMsgMsgDesc->getIntFromLine(self, cp, &uval, &ep, &isEnd);
  checkErrOK(result);
  self->compMsgData->maxMsgValParts = (uint8_t)uval;
  cp = ep;
  if (!isEnd) {
    return COMP_MSG_ERR_FUNNY_EXTRA_FIELDS;
  }
  compMsgData->msgValParts = os_zalloc(sizeof(msgValPart_t) * compMsgData->maxMsgValParts);
  checkAllocOK(compMsgData->msgValParts);
  idx = 0;
  while(idx < compMsgData->maxMsgValParts) {
    result = compMsgMsgDesc->readLine(compMsgMsgDesc, &buffer, &lgth);
    checkErrOK(result);
    if (lgth == 0) {
      return COMP_MSG_ERR_TOO_FEW_FILE_LINES;
    }
    msgValPart = &compMsgData->msgValParts[compMsgData->numMsgValParts++];
    buffer[lgth] = 0;
    cp = buffer;
    // fieldName
    fieldNameStr = cp;
    result = compMsgMsgDesc->getStrFromLine(self, cp, &ep, &isEnd);
    checkErrOK(result);
    msgValPart->fieldNameStr = os_zalloc(c_strlen(fieldNameStr)+ 1);
    checkAllocOK(msgValPart->fieldNameStr);
    c_memcpy(msgValPart->fieldNameStr, fieldNameStr, c_strlen(fieldNameStr));
    checkIsEnd(isEnd);
    result = self->compMsgTypesAndNames->getFieldNameIdFromStr(self->compMsgTypesAndNames, fieldNameStr, &msgValPart->fieldNameId, COMP_MSG_INCR);
    checkErrOK(result);
    cp = ep;

    // fieldValue
    fieldValueStr = cp;
    result = compMsgMsgDesc->getStrFromLine(self, cp, &ep, &isEnd);
    checkErrOK(result);
    msgValPart->fieldValueStr = os_zalloc(c_strlen(fieldValueStr)+ 1);
    checkAllocOK(msgValPart->fieldValueStr);
    c_memcpy(msgValPart->fieldValueStr, fieldValueStr, c_strlen(fieldValueStr));
    uval = c_strtoul(fieldValueStr, &endPtr, 10);
    if ((endPtr - (char *)fieldValueStr) == c_strlen(fieldValueStr)) {
      msgValPart->fieldFlags |= COMP_DISP_DESC_VALUE_IS_NUMBER;
      msgValPart->fieldValue = (uint32_t)uval;
    }
    if (ets_strncmp(fieldValueStr, "@get", 4) == 0) {
      result = self->compMsgUtil->getFieldValueCallback(self, fieldValueStr, &msgValPart->fieldValueCallback);
      if (result != COMP_MSG_ERR_OK) {
        COMP_MSG_DBG(self, "E", 2, "WARNING: fieldValueCallback %s for field: %s not found\n", fieldValueStr, fieldNameStr);
      }
//      checkErrOK(result);
    }
    if (ets_strncmp(fieldValueStr, "@run",4) == 0) {
      msgDescPart->fieldValueActionCb = msgValPart->fieldValueStr;
    }
//self->compMsgDebug->dumpMsgValPart(self, msgValPart);
    if (!isEnd) {
      return COMP_MSG_ERR_FUNNY_EXTRA_FIELDS;
    }
    idx++;
  }
#undef checkErrOK
#define checkErrOK(result) if(result != COMP_MSG_ERR_OK) return result
  result = compMsgMsgDesc->closeFile(compMsgMsgDesc);
  COMP_MSG_DBG(self, "E", 2, "getMsgPartsFromHeaderPart9 res: %d", result);
  checkErrOK(result);
#endif
  COMP_MSG_DBG(self, "E", 2, "heap2: %d", system_get_free_heap_size());

  return COMP_MSG_ERR_OK;
}

// ================================= getMsgKeyValueDescParts ====================================

static uint8_t getMsgKeyValueDescParts (compMsgDispatcher_t *self, uint8_t *fileName) {
  uint8_t result;
  uint8_t numEntries;
  uint8_t *fieldNameStr;
  uint8_t *fieldValueStr;
  uint8_t *fieldTypeStr;
  char *endPtr;
  uint8_t lgth;
  uint8_t fieldLgth;
  uint8_t buf[100];
  uint8_t *buffer = buf;
  long uval;
  uint8_t *myStr;
  int idx;
  int numericValue;
  uint8_t*cp;
  uint8_t*ep;
  bool isEnd;
  uint8_t bssInfoType;
  uint8_t fieldTypeId;
  compMsgMsgDesc_t *compMsgMsgDesc;
  msgKeyValueDescPart_t *msgKeyValueDescPart;

  compMsgMsgDesc = self->compMsgMsgDesc;
  result = compMsgMsgDesc->openFile(compMsgMsgDesc, fileName, "r");
  checkErrOK(result);
#undef checkErrOK
#define checkErrOK(result) if(result != COMP_MSG_ERR_OK) { compMsgMsgDesc->closeFile(compMsgMsgDesc); return result; }
  result = compMsgMsgDesc->readLine(compMsgMsgDesc, &buffer, &lgth);
  checkErrOK(result);
  buffer[lgth] = 0;
  if ((lgth < 4) || (buffer[0] != '#')) {
     return COMP_MSG_ERR_BAD_FILE_CONTENTS;
  }
  uval = c_strtoul(buffer+2, &endPtr, 10);
  numEntries = (uint8_t)uval;
  self->dispatcherCommon->numMsgKeyValueDescParts = 0;
  self->dispatcherCommon->maxMsgKeyValueDescParts = numEntries;
  self->dispatcherCommon->msgKeyValueDescParts = os_zalloc(numEntries * sizeof(msgKeyValueDescPart_t));
  checkAllocOK(self->dispatcherCommon->msgKeyValueDescParts);
  idx = 0;
  while(idx < numEntries) {
    result = compMsgMsgDesc->readLine(compMsgMsgDesc, &buffer, &lgth);
    checkErrOK(result);
    if (lgth == 0) {
      return COMP_MSG_ERR_TOO_FEW_FILE_LINES;
    }
    buffer[lgth] = 0;
    cp = buffer;
    msgKeyValueDescPart = &self->dispatcherCommon->msgKeyValueDescParts[self->dispatcherCommon->numMsgKeyValueDescParts];
    // fieldName
    fieldNameStr = cp;
    result = compMsgMsgDesc->getStrFromLine(self, cp, &ep, &isEnd);
    checkErrOK(result);
    msgKeyValueDescPart->keyNameStr = os_zalloc(c_strlen(fieldNameStr) + 1);
    checkAllocOK(msgKeyValueDescPart->keyNameStr);
    c_memcpy(msgKeyValueDescPart->keyNameStr, fieldNameStr, c_strlen(fieldNameStr) + 1);
    checkIsEnd(isEnd);
    cp = ep;

    // fieldValue
    fieldValueStr = cp;
    result = compMsgMsgDesc->getIntFromLine(self, cp, &uval, &ep, &isEnd);
    checkErrOK(result);
    msgKeyValueDescPart->keyId = (uint16_t)uval;
    checkIsEnd(isEnd);
    cp = ep;

    // fieldType
    fieldTypeStr = cp;
    result = compMsgMsgDesc->getStrFromLine(self, cp, &ep, &isEnd);
    checkErrOK(result);
    result = self->compMsgTypesAndNames->getFieldTypeIdFromStr(self->compMsgTypesAndNames, fieldTypeStr, &fieldTypeId);
    checkErrOK(result);
    msgKeyValueDescPart->keyType = fieldTypeId;
    cp = ep;

    // fieldLength 
    fieldTypeStr = cp;
    result = compMsgMsgDesc->getIntFromLine(self, cp, &uval, &ep, &isEnd);
    checkErrOK(result);
    msgKeyValueDescPart->keyLgth = (uint16_t)uval;
    COMP_MSG_DBG(self, "E", 2, "field: %s Id: %d type: %d length: %d\n", msgKeyValueDescPart->keyNameStr, msgKeyValueDescPart->keyId, msgKeyValueDescPart->keyType, msgKeyValueDescPart->keyLgth);
    if (!isEnd) {
      return COMP_MSG_ERR_FUNNY_EXTRA_FIELDS;
    }
    cp = ep;
    self->dispatcherCommon->numMsgKeyValueDescParts++;
    idx++;
  }
#undef checkErrOK
#define checkErrOK(result) if(result != COMP_MSG_ERR_OK) return result
  result = compMsgMsgDesc->closeFile(compMsgMsgDesc);
  checkErrOK(result);
  COMP_MSG_DBG(self, "E", 2, "getWifiKeyValueKeys done\n");
  return COMP_MSG_ERR_OK;
}

// ================================= getWifiKeyValueKeys ====================================

static uint8_t getWifiKeyValueKeys (compMsgDispatcher_t *self, uint8_t *fileName) {
  uint8_t result;
  uint8_t numEntries;
  uint8_t *fieldNameStr;
  uint8_t *fieldValueStr;
  uint8_t *fieldTypeStr;
  char *endPtr;
  uint8_t lgth;
  uint8_t fieldLgth;
  uint8_t buf[100];
  uint8_t *buffer = buf;
  long uval;
  uint8_t *myStr;
  int idx;
  int numericValue;
  uint8_t*cp;
  uint8_t*ep;
  bool isEnd;
  uint16_t keyValueKey;
  uint8_t fieldTypeId;
  compMsgMsgDesc_t *compMsgMsgDesc;
  compMsgWifiData_t *compMsgWifiData;

  compMsgMsgDesc = self->compMsgMsgDesc;
  compMsgWifiData = self->compMsgWifiData;
  result = compMsgMsgDesc->openFile(compMsgMsgDesc, fileName, "r");
  checkErrOK(result);
#undef checkErrOK
#define checkErrOK(result) if(result != COMP_MSG_ERR_OK) { compMsgMsgDesc->closeFile(compMsgMsgDesc); return result; }
  result = compMsgMsgDesc->readLine(compMsgMsgDesc, &buffer, &lgth);
  checkErrOK(result);
  buffer[lgth] = 0;
  if ((lgth < 4) || (buffer[0] != '#')) {
     return COMP_MSG_ERR_BAD_FILE_CONTENTS;
  }
  uval = c_strtoul(buffer+2, &endPtr, 10);
  numEntries = (uint8_t)uval;
  idx = 0;
  while(idx < numEntries) {
    result = compMsgMsgDesc->readLine(compMsgMsgDesc, &buffer, &lgth);
    checkErrOK(result);
    if (lgth == 0) {
      return COMP_MSG_ERR_TOO_FEW_FILE_LINES;
    }
    buffer[lgth] = 0;
    cp = buffer;
    // fieldName
    fieldNameStr = cp;
    result = compMsgMsgDesc->getStrFromLine(self, cp, &ep, &isEnd);
    checkErrOK(result);
    checkIsEnd(isEnd);
    cp = ep;

    // fieldValue
    fieldValueStr = cp;
    result = compMsgMsgDesc->getIntFromLine(self, cp, &uval, &ep, &isEnd);
    checkErrOK(result);
    result = self->compMsgWifiData->keyValueStr2KeyValueId(fieldNameStr + c_strlen("@key_"), &keyValueKey);
    if (result != COMP_MSG_ERR_OK) {
      // not a key the Wifi is handling (normally cloud keys)
      idx++;
      continue;
    }
    checkErrOK(result);
    switch (keyValueKey) {
    case KEY_VALUE_KEY_BSSID:
      compMsgWifiData->keyValueInfo.key_bssid = (uint16_t)uval;
      break;
    case KEY_VALUE_KEY_SSID:
      compMsgWifiData->keyValueInfo.key_ssid = (uint16_t)uval;
      break;
    case KEY_VALUE_KEY_CHANNEL:
      compMsgWifiData->keyValueInfo.key_channel = (uint16_t)uval;
      break;
    case KEY_VALUE_KEY_RSSI:
      compMsgWifiData->keyValueInfo.key_rssi = (uint16_t)uval;
      break;
    case KEY_VALUE_KEY_AUTH_MODE:
      compMsgWifiData->keyValueInfo.key_authmode = (uint16_t)uval;
      break;
    case KEY_VALUE_KEY_IS_HIDDEN:
      compMsgWifiData->keyValueInfo.key_freq_offset = (uint16_t)uval;
      break;
    case KEY_VALUE_KEY_FREQ_OFFSET:
      compMsgWifiData->keyValueInfo.key_freqcal_val = (uint16_t)uval;
      break;
    case KEY_VALUE_KEY_FREQ_CAL_VAL:
      compMsgWifiData->keyValueInfo.key_is_hidden = (uint16_t)uval;
      break;
    case KEY_VALUE_KEY_CLIENT_SSID:
      compMsgWifiData->keyValueInfo.key_clientSsid = (uint16_t)uval;
      break;
    case KEY_VALUE_KEY_CLIENT_PASSWD:
      compMsgWifiData->keyValueInfo.key_clientPasswd = (uint16_t)uval;
      break;
    case KEY_VALUE_KEY_CLIENT_IP_ADDR:
      compMsgWifiData->keyValueInfo.key_clientIPAddr = (uint16_t)uval;
      break;
    case   KEY_VALUE_KEY_CLIENT_PORT:
      compMsgWifiData->keyValueInfo.key_clientPort = (uint16_t)uval;
      break;
    case KEY_VALUE_KEY_CLIENT_STATUS:
      compMsgWifiData->keyValueInfo.key_clientStatus = (uint16_t)uval;
      break;
    case KEY_VALUE_KEY_MAC_ADDR:
      compMsgWifiData->keyValueInfo.key_MACAddr = (uint16_t)uval;
      break;
    case KEY_VALUE_KEY_MACHINE_STATE:
      compMsgWifiData->keyValueInfo.key_machineState = (uint16_t)uval;
      break;
    case KEY_VALUE_KEY_FIRMWARE_MAIN_BOARD:
      compMsgWifiData->keyValueInfo.key_firmwareMainBoard = (uint16_t)uval;
      break;
    case KEY_VALUE_KEY_FIRMWARE_DISPLAY_BOARD:
      compMsgWifiData->keyValueInfo.key_firmwareDisplayBoard = (uint16_t)uval;
      break;
    case KEY_VALUE_KEY_FIRMWARE_WIFI_MODULE:
      compMsgWifiData->keyValueInfo.key_firmwareWifiModule = (uint16_t)uval;
      break;
    case KEY_VALUE_KEY_LAST_ERROR:
      compMsgWifiData->keyValueInfo.key_lastError = (uint16_t)uval;
      break;
    case KEY_VALUE_KEY_CASING_USE_LIST:
      compMsgWifiData->keyValueInfo.key_casingUseList = (uint16_t)uval;
      break;
    case KEY_VALUE_KEY_CASING_STATISTIC_LIST:
      compMsgWifiData->keyValueInfo.key_casingStatisticList = (uint16_t)uval;
      break;
    case KEY_VALUE_KEY_DAT_AND_TIME:
      compMsgWifiData->keyValueInfo.key_dataAndTime = (uint16_t)uval;
      break;
    }
    checkIsEnd(isEnd);
    cp = ep;

    // fieldType
    fieldTypeStr = cp;
    result = compMsgMsgDesc->getStrFromLine(self, cp, &ep, &isEnd);
    checkErrOK(result);
    result = self->compMsgTypesAndNames->getFieldTypeIdFromStr(self->compMsgTypesAndNames, fieldTypeStr, &fieldTypeId);
    checkErrOK(result);
    switch (keyValueKey) {
    case KEY_VALUE_KEY_BSSID:
      compMsgWifiData->bssScanTypes.bssidType = (uint8_t)fieldTypeId;
      compMsgWifiData->keyValueInfo.key_type_bssid = (uint8_t)fieldTypeId;
      break;
    case KEY_VALUE_KEY_SSID:
      compMsgWifiData->bssScanTypes.ssidType = (uint8_t)fieldTypeId;
      compMsgWifiData->keyValueInfo.key_type_ssid = (uint8_t)fieldTypeId;
      break;
    case KEY_VALUE_KEY_CHANNEL:
      compMsgWifiData->bssScanTypes.channelType = (uint8_t)fieldTypeId;
      compMsgWifiData->keyValueInfo.key_type_channel = (uint8_t)fieldTypeId;
      break;
    case KEY_VALUE_KEY_RSSI:
      compMsgWifiData->bssScanTypes.rssiType = (uint8_t)fieldTypeId;
      compMsgWifiData->keyValueInfo.key_type_rssi = (uint8_t)fieldTypeId;
      break;
    case KEY_VALUE_KEY_AUTH_MODE:
      compMsgWifiData->bssScanTypes.authmodeType = (uint8_t)fieldTypeId;
      compMsgWifiData->keyValueInfo.key_type_authmode = (uint8_t)fieldTypeId;
      break;
    case KEY_VALUE_KEY_FREQ_OFFSET:
      compMsgWifiData->bssScanTypes.freq_offsetType = (uint8_t)fieldTypeId;
      compMsgWifiData->keyValueInfo.key_type_freq_offset = (uint8_t)fieldTypeId;
      break;
    case KEY_VALUE_KEY_FREQ_CAL_VAL:
      compMsgWifiData->bssScanTypes.freqcal_valType = (uint8_t)fieldTypeId;
      compMsgWifiData->keyValueInfo.key_type_freqcal_val = (uint8_t)fieldTypeId;
      break;
    case KEY_VALUE_KEY_IS_HIDDEN:
      compMsgWifiData->bssScanTypes.is_hiddenType = (uint8_t)fieldTypeId;
      compMsgWifiData->keyValueInfo.key_type_is_hidden = (uint8_t)fieldTypeId;
      break;
    case KEY_VALUE_KEY_CLIENT_SSID:
      compMsgWifiData->keyValueInfo.key_type_clientSsid = (uint8_t)fieldTypeId;
      break;
    case KEY_VALUE_KEY_CLIENT_PASSWD:
      compMsgWifiData->keyValueInfo.key_type_clientPasswd = (uint8_t)fieldTypeId;
      break;
    case KEY_VALUE_KEY_CLIENT_IP_ADDR:
      compMsgWifiData->keyValueInfo.key_type_clientIPAddr = (uint8_t)fieldTypeId;
      break;
    case KEY_VALUE_KEY_CLIENT_PORT:
      compMsgWifiData->keyValueInfo.key_type_clientPort = (uint8_t)fieldTypeId;
      break;
    case KEY_VALUE_KEY_CLIENT_STATUS:
      compMsgWifiData->keyValueInfo.key_type_clientStatus = (uint8_t)fieldTypeId;
      break;
    case KEY_VALUE_KEY_MAC_ADDR:
      compMsgWifiData->keyValueInfo.key_type_MACAddr = (uint8_t)fieldTypeId;
      break;
    case KEY_VALUE_KEY_MACHINE_STATE:
      compMsgWifiData->keyValueInfo.key_type_machineState = (uint8_t)fieldTypeId;
      break;
    case KEY_VALUE_KEY_FIRMWARE_MAIN_BOARD:
      compMsgWifiData->keyValueInfo.key_type_firmwareMainBoard = (uint8_t)fieldTypeId;
      break;
    case KEY_VALUE_KEY_FIRMWARE_DISPLAY_BOARD:
      compMsgWifiData->keyValueInfo.key_type_firmwareDisplayBoard = (uint8_t)fieldTypeId;
      break;
    case KEY_VALUE_KEY_FIRMWARE_WIFI_MODULE:
      compMsgWifiData->keyValueInfo.key_type_firmwareWifiModule = (uint8_t)fieldTypeId;
      break;
    case KEY_VALUE_KEY_LAST_ERROR:
      compMsgWifiData->keyValueInfo.key_type_lastError = (uint8_t)fieldTypeId;
      break;
    case KEY_VALUE_KEY_CASING_USE_LIST:
      compMsgWifiData->keyValueInfo.key_type_casingUseList = (uint8_t)fieldTypeId;
      break;
    case KEY_VALUE_KEY_CASING_STATISTIC_LIST:
      compMsgWifiData->keyValueInfo.key_type_casingStatisticList = (uint8_t)fieldTypeId;
      break;
    case KEY_VALUE_KEY_DAT_AND_TIME:
      compMsgWifiData->keyValueInfo.key_type_dataAndTime = (uint8_t)fieldTypeId;
      break;
    }
    cp = ep;

    // fieldLength not needed for Wifi module
    fieldTypeStr = cp;
    result = compMsgMsgDesc->getStrFromLine(self, cp, &ep, &isEnd);
    checkErrOK(result);
    COMP_MSG_DBG(self, "E", 2, "field: %s length: %s\n", fieldNameStr, cp);
    if (!isEnd) {
      return COMP_MSG_ERR_FUNNY_EXTRA_FIELDS;
    }
    cp = ep;
    idx++;
  }
#undef checkErrOK
#define checkErrOK(result) if(result != COMP_MSG_ERR_OK) return result
  result = compMsgMsgDesc->closeFile(compMsgMsgDesc);
  checkErrOK(result);
  COMP_MSG_DBG(self, "E", 2, "getWifiKeyValueKeys done\n");
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
  compMsgMsgDesc->addUseFileName = &addUseFileName;
  compMsgMsgDesc->handleMsgCommonLine = &handleMsgCommonLine;
  compMsgMsgDesc->handleMsgUseLine = &handleMsgUseLine;
  compMsgMsgDesc->handleMsgFieldsToSaveLine = &handleMsgFieldsToSaveLine;
  compMsgMsgDesc->handleMsgActionsLine = &handleMsgActionsLine;
  compMsgMsgDesc->handleMsgValuesLine = &handleMsgValuesLine;
  compMsgMsgDesc->handleMsgValHeaderLine = &handleMsgValHeaderLine;
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

ets_printf("readMsgFileNames: %s\n", MSG_FILES_FILE_NAME);
  result = compMsgMsgDesc->handleMsgFile(self, MSG_FILES_FILE_NAME, compMsgMsgDesc->handleMsgFileNameLine);
ets_printf("readMsgFileNames: result: %d\n", result);
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
