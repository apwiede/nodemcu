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

#include "compMsg.h"

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
COMP_MSG_DBG(self, "Y", 2, "addHeaderInfo: %d: fieldLgth: %d headerLgth: %d fieldId: %d", msgHeaderInfo->numHeaderFields, fieldLgth, msgHeaderInfo->headerLgth, fieldId);

  return result;
}

// ================================= addMidPartInfo ====================================

static uint8_t addMidPartInfo(compMsgDispatcher_t *self, uint16_t fieldLgth, uint8_t fieldId) {
  uint8_t result;
  msgMidPartInfo_t *msgMidPartInfo;

  result = COMP_MSG_ERR_OK;
  msgMidPartInfo = &self->compMsgMsgDesc->msgMidPartInfo;
  if (msgMidPartInfo->numMidPartFields == 0) {
    msgMidPartInfo->midPartFieldIds = (uint8_t *)os_zalloc(((msgMidPartInfo->numMidPartFields + 1) * sizeof(uint8_t)));
  } else {
    msgMidPartInfo->midPartFieldIds = (uint8_t *)os_realloc(msgMidPartInfo->midPartFieldIds, ((msgMidPartInfo->numMidPartFields + 1) * sizeof(uint8_t)));
  }
  checkAllocOK(msgMidPartInfo->midPartFieldIds);
  msgMidPartInfo->midPartLgth += fieldLgth;
  msgMidPartInfo->midPartFieldIds[msgMidPartInfo->numMidPartFields] = fieldId;
  msgMidPartInfo->numMidPartFields++;
COMP_MSG_DBG(self, "Y", 2, "addMidPartInfo: %d: fieldLgth: %d midPartLgth: %d fieldId: %d", msgMidPartInfo->numMidPartFields, fieldLgth, msgMidPartInfo->midPartLgth, fieldId);

  return result;
}

// ================================= addTrailerInfo ====================================

static uint8_t addTrailerInfo(compMsgDispatcher_t *self, uint16_t fieldLgth, uint8_t fieldId) {
  uint8_t result;
  msgTrailerInfo_t *msgTrailerInfo;

  result = COMP_MSG_ERR_OK;
  msgTrailerInfo = &self->compMsgMsgDesc->msgTrailerInfo;
  if (msgTrailerInfo->numTrailerFields == 0) {
    msgTrailerInfo->trailerFieldIds = (uint8_t *)os_zalloc(((msgTrailerInfo->numTrailerFields + 1) * sizeof(uint8_t)));
  } else {
    msgTrailerInfo->trailerFieldIds = (uint8_t *)os_realloc(msgTrailerInfo->trailerFieldIds, ((msgTrailerInfo->numTrailerFields + 1) * sizeof(uint8_t)));
  }
  checkAllocOK(msgTrailerInfo->trailerFieldIds);
  msgTrailerInfo->trailerLgth += fieldLgth;
  msgTrailerInfo->trailerFieldIds[msgTrailerInfo->numTrailerFields] = fieldId;
  msgTrailerInfo->numTrailerFields++;
COMP_MSG_DBG(self, "Y", 2, "addTrailerInfo: %d: fieldLgth: %d trailerLgth: %d fieldId: %d", msgTrailerInfo->numTrailerFields, fieldLgth, msgTrailerInfo->trailerLgth, fieldId);

  return result;
}

// ================================= getHeaderChksumKey ====================================

static uint8_t getHeaderChksumKey(compMsgDispatcher_t *self, uint8_t *data) {
  uint8_t result;
  uint16_t key;
  int idx;
  int isNonZero;
  int offset;
  int myOffset;
  int lgth;
  msgHeaderInfo_t *msgHeaderInfo;
  fieldDescInfo_t *fieldDescInfo;

  result = COMP_MSG_ERR_OK;
  msgHeaderInfo = &self->compMsgMsgDesc->msgHeaderInfo;
  key = 0;
  idx = 1;
  offset = 0;
  while (idx < msgHeaderInfo->numHeaderFields) {
COMP_MSG_DBG(self, "Y", 0, "getHeaderChksumKey: idx: %d id: %d", idx, msgHeaderInfo->headerFieldIds[idx]);
    fieldDescInfo = self->compMsgTypesAndNames->msgFieldInfos.fieldDescInfos[idx];
    if (fieldDescInfo == NULL) {
      return COMP_MSG_ERR_MSG_FIELD_INFO_IDX_NOT_SET;
    }
    lgth = offset + fieldDescInfo->fieldLgth;
    if (fieldDescInfo->fieldFlags & COMP_MSG_FIELD_HEADER_UNIQUE) {
      myOffset = offset;
      while (myOffset < lgth) {
        key += data[myOffset];
        myOffset++;
      }
    }
    offset += fieldDescInfo->fieldLgth;
    idx++;
  }
COMP_MSG_DBG(self, "Y", 0, "getHeaderChksumKey: 0x%04x", key);
  return result;
}

// ================================= addFieldGroup ====================================

static uint8_t addFieldGroup(compMsgDispatcher_t *self, char *fileName, uint16_t fieldGroupId, uint16_t cmdKey) {
  uint8_t result;
  msgFieldGroupInfo_t *msgFieldGroupInfo;

COMP_MSG_DBG(self, "E", 2, "addFieldGroup: %s!fieldGroupId: %d", fileName, fieldGroupId);
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
  bool hadNumLines;
  int idx;
  char *cp;
  compMsgMsgDesc_t *compMsgMsgDesc;
  compMsgFile_t *compMsgFile;
  msgFieldGroupInfo_t *msgFieldGroupInfo;

  compMsgMsgDesc = self->compMsgMsgDesc;
  compMsgFile = self->compMsgFile;
  buffer = buf;
  result = compMsgFile->openFile(self, fileName, "r");
  checkErrOK(result);
#undef checkErrOK
#define checkErrOK(result) if (result != COMP_MSG_ERR_OK) { compMsgFile->closeFile(self); return result; }
  numLines = 0;
  compMsgMsgDesc->currLineNo = 0;
  hadNumLines = false;
  while (1) {
    result = compMsgFile->readLine(self, &buffer, &lgth);
    checkErrOK(result);
    if (lgth == 0) {
      if (numLines > compMsgMsgDesc->expectedLines) {
        COMP_MSG_DBG(self, "Y", 0, "Error %s: numLines: %d expectedLines: %d", fileName, numLines, compMsgMsgDesc->expectedLines);
        result = compMsgFile->closeFile(self);
        return COMP_MSG_ERR_BAD_NUM_DESC_FILE_LINES;
      }
      break;
    }
    // check for empty line or line with spaces only
    cp = buffer;
    while ((*cp == ' ') || (*cp == '\r') || (*cp == '\n')) {
      cp++;
    }
    if ((cp - (char *)buffer) == lgth) {
      // skip empty line
      COMP_MSG_DBG(self, "Y", 0, "Warning %s: skipping empty line %d", fileName, numLines);
      continue;
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
    if ((numLines == 0) && (!hadNumLines)) {
      // check if it is the number of lines line
      self->compMsgMsgDesc->currLineNo = 0;
      if (c_strcmp(compMsgFile->lineFields[0], "#") == 0) {
        result = compMsgFile->getIntFieldValue(self, compMsgFile->lineFields[1], &ep, 0, &self->compMsgMsgDesc->expectedLines);
        checkErrOK(result);
        hadNumLines = true;
      } else {
        COMP_MSG_DBG(self, "E", 0, "Error: %s: wrong desc file number lines line", fileName);
        return COMP_MSG_ERR_WRONG_DESC_FILE_LINE;
      }
    } else {
      numLines++;
      if (numLines > compMsgMsgDesc->expectedLines) {
        COMP_MSG_DBG(self, "Y", 0, "Error %s: numLines: %d expectedLines: %d fields: %s!%s!", fileName, numLines, compMsgMsgDesc->expectedLines, compMsgFile->lineFields[0], compMsgFile->lineFields[1]);
        result = compMsgFile->closeFile(self);
        return COMP_MSG_ERR_BAD_NUM_DESC_FILE_LINES;
      }
      compMsgMsgDesc->currLineNo++;
      result = handleMsgLine(self);
      checkErrOK(result);
    }
  }
  result = compMsgFile->closeFile(self);
#undef checkErrOK
#define checkErrOK(result) if(result != COMP_MSG_ERR_OK) { return result; }
  checkErrOK(result);
  return result;
}

// ================================= handleMsgFile ====================================

static uint8_t handleMsgFile(compMsgDispatcher_t *self, uint8_t *fileName, handleMsgLine_t handleMsgLine) {
  int result;
  compMsgMsgDesc_t *compMsgMsgDesc;
  msgFieldGroupInfo_t *msgFieldGroupInfo;

  result = handleMsgFileInternal(self, fileName, handleMsgLine);
  checkErrOK(result);
  compMsgMsgDesc = self->compMsgMsgDesc;
  COMP_MSG_DBG(self, "E", 2, "handleMsgFiles done");
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
      COMP_MSG_DBG(self, "E", 2, "cmdKey: %s 0x%04x", cmdKeyStr, cmdKey);
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
  int headerFlag;
  uint32_t fieldFlags;
  uint8_t fieldNameId;
  fieldDescInfo_t fieldDescInfo;
  char *ep;
  uint8_t *stringVal;
  char *fieldName;
  char *fieldType;
  char *fieldLgth;
  compMsgMsgDesc_t *compMsgMsgDesc;
  compMsgFile_t *compMsgFile;
  compMsgTypesAndNames_t *compMsgTypesAndNames;
  msgFieldGroupInfo_t *msgFieldGroupInfo;
  msgDescriptionInfos_t *msgDescriptionInfos;
  msgDescription_t *msgDescription;
  int descIdx;
  int headerIdx;
  msgHeaderInfo_t *msgHeaderInfo;
  uint8_t *headerFieldIds;
  int sequenceLgth;
  int midPartIdx;
  msgMidPartInfo_t *msgMidPartInfo;
  uint8_t *midPartFieldIds;
  uint8_t fldId;
  int trailerIdx;
  msgTrailerInfo_t *msgTrailerInfo;
  uint8_t *trailerFieldIds;
  uint8_t *fieldNameStr;
  fieldDescInfo_t *fieldDescInfoPtr;
  int fieldIdx;

  result = COMP_MSG_ERR_OK;
  compMsgMsgDesc = self->compMsgMsgDesc;
  compMsgTypesAndNames = self->compMsgTypesAndNames;
  msgHeaderInfo = &compMsgMsgDesc->msgHeaderInfo;
  msgMidPartInfo = &compMsgMsgDesc->msgMidPartInfo;
  msgTrailerInfo = &compMsgMsgDesc->msgTrailerInfo;
  msgDescription = NULL;
  compMsgFile = self->compMsgFile;
  msgFieldGroupInfo = &compMsgMsgDesc->msgFieldGroupInfos[compMsgMsgDesc->currMsgFieldGroupInfo];
  if (compMsgFile->numLineFields < 3) {
    return COMP_MSG_ERR_FIELD_DESC_TOO_FEW_FIELDS;
  }
  fieldName = compMsgFile->lineFields[0];
  fieldType = compMsgFile->lineFields[1];
  fieldLgth = compMsgFile->lineFields[2];
  COMP_MSG_DBG(self, "E", 2, "handleMsgCommonLine: %s: %s", fieldName, fieldType);
  fieldDescInfo.fieldFlags = 0;
  fieldDescInfo.keyValueDesc = NULL;

  // field name
  result = compMsgTypesAndNames->getFieldNameIdFromStr(self, fieldName, &fieldNameId, COMP_MSG_INCR);
  checkErrOK(result);
  if ((fieldName[0] == '@') && (fieldName[1] == '#')) {
    fieldDescInfo.fieldFlags |= COMP_MSG_FIELD_KEY_VALUE;
  }

  // field type
  result = compMsgTypesAndNames->getFieldTypeIdFromStr(self, fieldType, &fieldDescInfo.fieldTypeId);
  checkErrOK(result);

  // field lgth
  result = compMsgFile->getIntFieldValue(self, fieldLgth, &ep, 0, &lgth);
  checkErrOK(result);
//ets_printf("%s id: %d lgth: %d fieldLgth: %s\n", fieldName, fieldNameId, lgth, fieldLgth);
  fieldDescInfo.fieldLgth = (uint16_t)lgth;

  // field key/value id
  if (msgFieldGroupInfo->fieldGroupId == COMP_MSG_DESC_KEY_VALUE_FIELD_GROUP) {
    result = compMsgFile->getIntFieldValue(self, compMsgFile->lineFields[3], &ep, 0, &keyValueId);
    checkErrOK(result);
  }

  // field headerFlag
  if (msgFieldGroupInfo->fieldGroupId == COMP_MSG_DESC_HEADER_FIELD_GROUP) {
    result = compMsgFile->getIntFieldValue(self, compMsgFile->lineFields[3], &ep, 0, &headerFlag);
    checkErrOK(result);
    fieldDescInfo.fieldOffset = msgHeaderInfo->headerLgth;
    switch (headerFlag) {
    case 0:
      break;
    case 1:
      fieldDescInfo.fieldFlags |= COMP_MSG_FIELD_HEADER_UNIQUE;
      break;
    case 2:
      fieldDescInfo.fieldFlags |= COMP_MSG_FIELD_HEADER_CHKSUM_NON_ZERO;
      break;
    default:
      return COMP_MSG_ERR_BAD_HEADER_FIELD_FLAG;
      break;
    }
    result = compMsgMsgDesc->addHeaderInfo(self, fieldDescInfo.fieldLgth, fieldNameId);
    checkErrOK(result);
  }

  COMP_MSG_DBG(self, "E", 2, "%s: id: %d type: %s %d lgth: %d", compMsgFile->lineFields[0], fieldNameId, compMsgFile->lineFields[1], fieldDescInfo.fieldTypeId, fieldDescInfo.fieldLgth);
  switch (msgFieldGroupInfo->fieldGroupId) {
  case COMP_MSG_DESC_HEADER_FIELD_GROUP:
    fieldDescInfo.fieldFlags |= COMP_MSG_FIELD_HEADER;
    result = compMsgTypesAndNames->setMsgFieldDescInfo(self, fieldNameId, &fieldDescInfo);
    checkErrOK(result);
    break;
  case COMP_MSG_DESC_MID_PART_FIELD_GROUP:
    fieldDescInfo.fieldOffset = compMsgMsgDesc->msgHeaderInfo.headerLgth + compMsgMsgDesc->msgMidPartInfo.midPartLgth;
    result = compMsgMsgDesc->addMidPartInfo(self, fieldDescInfo.fieldLgth, fieldNameId);
    checkErrOK(result);
    result = compMsgTypesAndNames->setMsgFieldDescInfo(self, fieldNameId, &fieldDescInfo);
    checkErrOK(result);
    break;
  case COMP_MSG_DESC_TRAILER_FIELD_GROUP:
    // FIXME need to set fieldOffet here!!
    result = compMsgMsgDesc->addTrailerInfo(self, fieldDescInfo.fieldLgth, fieldNameId);
    checkErrOK(result);
    result = compMsgTypesAndNames->setMsgFieldDescInfo(self, fieldNameId, &fieldDescInfo);
    checkErrOK(result);
    break;
  case COMP_MSG_DESC_KEY_VALUE_FIELD_GROUP:
    fieldDescInfo.fieldFlags |= COMP_MSG_FIELD_KEY_VALUE;
    result = compMsgTypesAndNames->setMsgFieldDescInfo(self, fieldNameId, &fieldDescInfo);
    checkErrOK(result);
    break;
  case COMP_MSG_DESC_FIELD_GROUP:
    result = compMsgTypesAndNames->setMsgFieldDescInfo(self, fieldNameId, &fieldDescInfo);
    checkErrOK(result);
    break;
  }
  // add description infos
  if (msgFieldGroupInfo->fieldGroupId == COMP_MSG_DESC_FIELD_GROUP) {
    msgDescriptionInfos = &compMsgMsgDesc->msgDescriptionInfos;
    descIdx = 0;
    while (descIdx < msgDescriptionInfos->numMsgDescriptions) {
      msgDescription = &msgDescriptionInfos->msgDescriptions[descIdx];
      if (msgFieldGroupInfo->cmdKey == msgDescription->cmdKey) {
        break;
      }
      descIdx++;
    }
    if (msgDescription->fieldSequence == NULL) {
      // FIXME need to allocate correct space here !!!
      sequenceLgth = msgHeaderInfo->numHeaderFields + msgMidPartInfo->numMidPartFields + msgTrailerInfo->numTrailerFields + /* ??? */ 10;
      msgDescription->fieldSequence = os_zalloc(sequenceLgth * sizeof(uint8_t));
      checkAllocOK(msgDescription->fieldSequence);
      msgDescription->fieldOffsets = os_zalloc(sequenceLgth * sizeof(uint16_t));
      checkAllocOK(msgDescription->fieldOffsets);
      // add the header and midPart field ids before appending the message specific ids
      msgHeaderInfo  = &compMsgMsgDesc->msgHeaderInfo;
      fieldIdx = 0;
      headerIdx = 0;
      headerFieldIds = msgHeaderInfo->headerFieldIds;
      while (headerIdx < msgHeaderInfo->numHeaderFields) {
        msgDescription->fieldSequence[fieldIdx] = headerFieldIds[headerIdx];
        msgDescription->fieldOffsets[fieldIdx] = msgDescription->lastFieldOffset;
        fieldDescInfoPtr = compMsgTypesAndNames->msgFieldInfos.fieldDescInfos[headerFieldIds[headerIdx]];
        msgDescription->lastFieldOffset += fieldDescInfoPtr->fieldLgth;
        fieldIdx++;
        headerIdx++;
      }
      // add the midpart ids!!
      msgMidPartInfo = &compMsgMsgDesc->msgMidPartInfo;
      midPartIdx = 0;
      midPartFieldIds = msgMidPartInfo->midPartFieldIds;
      while (midPartIdx < msgMidPartInfo->numMidPartFields) {
        fldId = midPartFieldIds[midPartIdx];
        result = compMsgTypesAndNames->getFieldNameStrFromId(self, fldId, &fieldNameStr);
        checkErrOK(result);
        fieldDescInfoPtr = compMsgTypesAndNames->msgFieldInfos.fieldDescInfos[fldId];
        if (c_strcmp(fieldNameStr, "@cmdLgth") == 0) {
          if (msgDescription->fieldFlags & COMP_MSG_HAS_CMD_LGTH) {
            msgDescription->fieldSequence[fieldIdx] = fldId;
            msgDescription->fieldOffsets[fieldIdx] = msgDescription->lastFieldOffset;
            fieldIdx++;
            msgDescription->lastFieldOffset += fieldDescInfoPtr->fieldLgth;
          }
        } else {
          msgDescription->fieldSequence[fieldIdx] = fldId;
          msgDescription->fieldOffsets[fieldIdx] = msgDescription->lastFieldOffset;
          fieldIdx++;
          msgDescription->lastFieldOffset += fieldDescInfoPtr->fieldLgth;
        }
        midPartIdx++;
      }
    }
    fieldDescInfoPtr = compMsgTypesAndNames->msgFieldInfos.fieldDescInfos[fieldNameId];
    msgDescription->fieldSequence[fieldIdx] = fieldNameId;
    msgDescription->fieldOffsets[fieldIdx] = msgDescription->lastFieldOffset;
    fieldIdx++;
    if (fieldDescInfoPtr == NULL) {
      return COMP_MSG_ERR_BAD_FIELD_INFO;
    }
    msgDescription->lastFieldOffset += fieldDescInfoPtr->fieldLgth;
    if (compMsgMsgDesc->expectedLines == compMsgMsgDesc->currLineNo) {
      // add the trailer info
      msgTrailerInfo = &compMsgMsgDesc->msgTrailerInfo;
      trailerIdx = 0;
      trailerFieldIds = msgTrailerInfo->trailerFieldIds;
      while (trailerIdx < msgTrailerInfo->numTrailerFields) {
        fldId = trailerFieldIds[trailerIdx];
        result = compMsgTypesAndNames->getFieldNameStrFromId(self, fldId, &fieldNameStr);
        checkErrOK(result);
        fieldDescInfoPtr = compMsgTypesAndNames->msgFieldInfos.fieldDescInfos[fldId];
        if (c_strcmp(fieldNameStr, "@crc") == 0) {
          if(msgDescription->fieldFlags & COMP_MSG_HAS_CRC) {
            msgDescription->fieldSequence[fieldIdx] = fldId;
            msgDescription->fieldOffsets[fieldIdx] = msgDescription->lastFieldOffset;
            msgDescription->lastFieldOffset += fieldDescInfoPtr->fieldLgth;
            fieldIdx++;
          }
        } else {
          if (c_strcmp(fieldNameStr, "@totalCrc") == 0) {
            if (msgDescription->fieldFlags & COMP_MSG_HAS_TOTAL_CRC) {
              msgDescription->fieldSequence[fieldIdx] = fldId;
              msgDescription->fieldOffsets[fieldIdx] = msgDescription->lastFieldOffset;
              msgDescription->lastFieldOffset += fieldDescInfoPtr->fieldLgth;
              fieldIdx++;
            }
          } else {
            msgDescription->fieldSequence[fieldIdx] = fldId;
            msgDescription->fieldOffsets[fieldIdx] = msgDescription->lastFieldOffset;
            msgDescription->lastFieldOffset += fieldDescInfoPtr->fieldLgth;
            fieldIdx++;
          }
        }
        trailerIdx++;
      }
    }
  }
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
  msgFieldGroupInfo_t *msgFieldGroupInfo;
  fieldDescInfo_t fieldDescInfo;

  result = COMP_MSG_ERR_OK;
  compMsgMsgDesc = self->compMsgMsgDesc;
  compMsgFile = self->compMsgFile;
  msgFieldGroupInfo = &compMsgMsgDesc->msgFieldGroupInfos[compMsgMsgDesc->currMsgFieldGroupInfo];
  if (compMsgFile->numLineFields < 1) {
    return COMP_MSG_ERR_FIELD_DESC_TOO_FEW_FIELDS;
  }
//COMP_MSG_DBG(self, "E", 1, "handleFieldsToSaveLine: %s", compMsgFile->lineFields[0]);
  //field name
  result = self->compMsgTypesAndNames->getFieldNameIdFromStr(self, compMsgFile->lineFields[0], &fieldNameId, COMP_MSG_INCR);
  checkErrOK(result);
  COMP_MSG_DBG(self, "E", 2, "fieldToSave: %s: id: %d", compMsgFile->lineFields[0], fieldNameId);
  c_memset(&fieldDescInfo, 0, sizeof(fieldDescInfo_t));
  result = self->compMsgTypesAndNames->getMsgFieldDescInfo(self, fieldNameId, &fieldDescInfo);
  checkErrOK(result);
  fieldDescInfo.fieldFlags |= COMP_MSG_FIELD_TO_SAVE;
  result = self->compMsgTypesAndNames->setMsgFieldDescInfo(self, fieldNameId, &fieldDescInfo);
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
  msgFieldGroupInfo_t *msgFieldGroupInfo;

  result = COMP_MSG_ERR_OK;
  compMsgMsgDesc = self->compMsgMsgDesc;
  compMsgFile = self->compMsgFile;
  msgFieldGroupInfo = &compMsgMsgDesc->msgFieldGroupInfos[compMsgMsgDesc->currMsgFieldGroupInfo];
  if (compMsgFile->numLineFields < 2) {
    return COMP_MSG_ERR_FIELD_DESC_TOO_FEW_FIELDS;
  }
//COMP_MSG_DBG(self, "E", 1, "handleActionsLine: %s: %s", compMsgFile->lineFields[0], compMsgFile->lineFields[1]);
  //field name
  result = self->compMsgTypesAndNames->getFieldNameIdFromStr(self, compMsgFile->lineFields[0], &fieldNameId, COMP_MSG_INCR);
  checkErrOK(result);
  //field value
  COMP_MSG_DBG(self, "E", 2, "%s: id: %d val: %s", compMsgFile->lineFields[0], fieldNameId, compMsgFile->lineFields[1]);
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
  char *fieldName;
  char *fieldVal;
  bool isCommonValue;
  uint16_t callbackId;
  compMsgMsgDesc_t *compMsgMsgDesc;
  compMsgFile_t *compMsgFile;
  compMsgTypesAndNames_t *compMsgTypesAndNames;
  compMsgDataValue_t *compMsgDataValue;
  fieldValue_t fieldValue;
  msgFieldValue_t *msgFieldValue;
  msgFieldGroupInfo_t *msgFieldGroupInfo;
  fieldDescInfo_t fieldDescInfo;
  fieldValInfo_t fieldValInfo;

  result = COMP_MSG_ERR_OK;
  isCommonValue = false;
  compMsgMsgDesc = self->compMsgMsgDesc;
  compMsgFile = self->compMsgFile;
  compMsgTypesAndNames = self->compMsgTypesAndNames;
  compMsgDataValue = self->compMsgDataValue;
  msgFieldGroupInfo = &compMsgMsgDesc->msgFieldGroupInfos[compMsgMsgDesc->currMsgFieldGroupInfo];
  if (compMsgFile->numLineFields < 2) {
    return COMP_MSG_ERR_FIELD_DESC_TOO_FEW_FIELDS;
  }
  c_memset(&fieldValInfo, 0, sizeof(fieldValInfo_t));
  fieldValInfo.fieldValueCallback = NULL;
  c_memset(&fieldValue, 0, sizeof(fieldValue_t));
  fieldValue.fieldValueCallback = NULL;
 
  //field name
  fieldName = compMsgFile->lineFields[0];
  result = compMsgTypesAndNames->getFieldNameIdFromStr(self, fieldName, &fieldNameId, COMP_MSG_INCR);
  checkErrOK(result);
  // get the fieldDescInfo for the name to eventually set fieldFlags or other info.
  result = compMsgTypesAndNames->getMsgFieldDescInfo(self, fieldNameId, &fieldDescInfo);
  checkErrOK(result);

  //field value
  fieldVal = compMsgFile->lineFields[1];
  stringValue = NULL;
  numericValue = 0;
  if (msgFieldGroupInfo->fieldGroupId == COMP_MSG_WIFI_DATA_VALUES_FIELD_GROUP) {
    fieldDescInfo.fieldFlags |= COMP_MSG_FIELD_WIFI_DATA;
    isCommonValue = true;
  }
  if (msgFieldGroupInfo->fieldGroupId == COMP_MSG_MODULE_DATA_VALUES_FIELD_GROUP) {
    fieldDescInfo.fieldFlags |= COMP_MSG_FIELD_MODULE_DATA;
    isCommonValue = true;
  }
  if (fieldVal[0] == '"') {
    result = compMsgFile->getStringFieldValue(self, fieldVal, &stringValue);
    checkErrOK(result);
    if (isCommonValue) {
      fieldValInfo.fieldFlags |= COMP_MSG_FIELD_IS_STRING;
      fieldValInfo.fieldValueCallbackId = 0;
      fieldValInfo.dataValue.value.stringValue = os_zalloc(c_strlen(stringValue) + 1);
      checkAllocOK(fieldValInfo.dataValue.value.stringValue);
      c_memcpy(fieldValInfo.dataValue.value.stringValue, stringValue, c_strlen(stringValue));
    } else {
      fieldValue.fieldValueCallbackId = 0;
      fieldValue.fieldValueFlags |= COMP_MSG_FIELD_IS_STRING;
      fieldValue.dataValue.value.stringValue = stringValue;
    }
    if ((fieldDescInfo.fieldTypeId == 0) && (fieldDescInfo.fieldLgth == 0)) {
      result = compMsgTypesAndNames->getFieldTypeIdFromStr(self, "uint8_t*", &fieldDescInfo.fieldTypeId);
      checkErrOK(result);
      fieldDescInfo.fieldLgth = c_strlen(stringValue);
    }
  } else {
    if (fieldVal[0] == '@') {
      result = compMsgFile->getStringFieldValue(self, fieldVal, &stringValue);
      checkErrOK(result);
      result = self->compMsgWifiData->callbackStr2CallbackId(stringValue, &callbackId);
      if (result != COMP_MSG_ERR_OK) {
        result = self->compMsgModuleData->callbackStr2CallbackId(stringValue, &callbackId);
      }
      checkErrOK(result);
      fieldValInfo.fieldFlags |= COMP_MSG_FIELD_IS_STRING;
      fieldValInfo.fieldFlags |= COMP_MSG_FIELD_HAS_CALLBACK;
      fieldValInfo.fieldValueCallbackId = callbackId;
      fieldValInfo.dataValue.value.stringValue = NULL;
      if ((fieldDescInfo.fieldTypeId == 0) && (fieldDescInfo.fieldLgth == 0)) {
        result = compMsgTypesAndNames->getFieldTypeIdFromStr(self, "uint8_t*", &fieldDescInfo.fieldTypeId);
        checkErrOK(result);
ets_printf("setting fieldLgth\n");
        fieldDescInfo.fieldLgth = c_strlen(stringValue);
      }
    } else {
      result = compMsgFile->getIntFieldValue(self, fieldVal, &ep, 0, &numericValue);
      checkErrOK(result);
      if (isCommonValue) {
        fieldValInfo.fieldValueCallbackId = 0;
        fieldValInfo.fieldFlags |= COMP_MSG_FIELD_IS_NUMERIC;
        fieldValInfo.dataValue.value.numericValue = numericValue;
      } else {
        fieldValue.fieldValueCallbackId = 0;
        fieldValue.fieldValueFlags |= COMP_MSG_FIELD_IS_NUMERIC;
        fieldValue.dataValue.value.numericValue = numericValue;
      }
      if ((fieldDescInfo.fieldTypeId == 0) && (fieldDescInfo.fieldLgth == 0)) {
        result = compMsgTypesAndNames->getFieldTypeIdFromStr(self, "uint32_t", &fieldDescInfo.fieldTypeId);
        checkErrOK(result);
        fieldDescInfo.fieldLgth = sizeof(uint32_t);
      }
    }
  }
  result = compMsgTypesAndNames->setMsgFieldDescInfo(self, fieldNameId, &fieldDescInfo);
  checkErrOK(result);
  COMP_MSG_DBG(self, "E", 2, "handleMsgValuesLine: %s: id: %d val: %s %d cb: %s", fieldName, fieldNameId, stringValue == NULL ? "nil" : (char *)stringValue, numericValue, fieldValue.fieldValueCallback == NULL ? "nil" : (char *)fieldValue.fieldValueCallback);
  switch (msgFieldGroupInfo->fieldGroupId) {
  case COMP_MSG_WIFI_DATA_VALUES_FIELD_GROUP:
  case COMP_MSG_MODULE_DATA_VALUES_FIELD_GROUP:
    result = compMsgTypesAndNames->setMsgFieldValInfo(self, fieldNameId, &fieldValInfo);
    checkErrOK(result);
    break;
  case COMP_MSG_VAL_FIELD_GROUP:
  case COMP_MSG_VAL_HEADER_FIELD_GROUP:
    fieldValue.fieldNameId = fieldNameId;
    if (compMsgMsgDesc->currLineNo == 1) {
      if (msgFieldGroupInfo->fieldGroupId == COMP_MSG_VAL_HEADER_FIELD_GROUP) {
        // FIXME next line +20!!
        result = compMsgDataValue->newMsgFieldValueInfos(self, msgFieldGroupInfo->cmdKey, compMsgMsgDesc->expectedLines + 20, &msgFieldValue);
      } else {
        result = compMsgDataValue->newMsgFieldValueInfos(self, msgFieldGroupInfo->cmdKey, compMsgMsgDesc->expectedLines, &msgFieldValue);
      }
    } else {
      result = compMsgDataValue->getMsgFieldValueInfo(self, msgFieldGroupInfo->cmdKey, &msgFieldValue);
    }
    checkErrOK(result);
    fieldValue.fieldNameId = fieldNameId;
    fieldValue.fieldValueCallback = NULL;
    result = compMsgDataValue->setMsgFieldValueInfo(self, msgFieldValue, &fieldValue);
    checkErrOK(result);
    break;
  default:
    return COMP_MSG_ERR_BAD_DESC_FILE_FIELD_GROUP_TYPE;
  }
COMP_MSG_DBG(self, "E", 2, "handleMsgValuesLine: done");
  return result;
}

// ================================= handleMsgHeadsLine ====================================

static uint8_t handleMsgHeadsLine(compMsgDispatcher_t *self) {
  uint8_t result;
  int lgth;
  int fieldGroupIdx;
  int fieldIdx;
  int headerFieldIdx;
  uint8_t *fieldName;
  uint8_t *value;
  uint8_t *stringVal;
  int numericValue;
  char *ep;
  int offset;
  compMsgMsgDesc_t *compMsgMsgDesc;
  compMsgTypesAndNames_t *compMsgTypesAndNames;
  compMsgFile_t *compMsgFile;
  fieldDescInfo_t *fieldDescInfo;
  dataView_t *dataView;
  msgFieldGroupInfo_t *msgFieldGroupInfo;
  msgDescriptionInfos_t *descriptions;
  msgDescription_t *msgDescription;
  msgHeaderInfo_t *msgHeaderInfo;
  compMsgDataView_t *compMsgDataView;

  result = COMP_MSG_ERR_OK;
  if (self->compMsgData == NULL) {
    self->compMsgData = newCompMsgData();
    checkAllocOK(self->compMsgData);
    self->compMsgData->compMsgDataView = newCompMsgDataView("", 0);
  }
  compMsgMsgDesc = self->compMsgMsgDesc;
  compMsgTypesAndNames = self->compMsgTypesAndNames;
  compMsgDataView = self->compMsgData->compMsgDataView;
  compMsgFile = self->compMsgFile;
  msgHeaderInfo = &self->compMsgMsgDesc->msgHeaderInfo;
  result = self->compMsgUtil->addFieldDescription(self);
  checkErrOK(result);
  descriptions = &compMsgMsgDesc->msgDescriptionInfos;
  msgDescription = &descriptions->msgDescriptions[descriptions->numMsgDescriptions - 1];
  if (msgDescription->headerFieldValues == NULL) {
    msgDescription->headerFieldValues = os_zalloc(msgHeaderInfo->headerLgth);
    checkAllocOK(msgDescription->headerFieldValues);
  }
  if (msgHeaderInfo->numHeaderFields == 0) {
    return COMP_MSG_ERR_HEADER_FIELD_GROUP_NOT_FOUND;
  }
  if (compMsgFile->numLineFields < 3) {
    return COMP_MSG_ERR_FIELD_DESC_TOO_FEW_FIELDS;
  }
  dataView = self->compMsgData->compMsgDataView->dataView;
  dataView->setDataViewData(dataView, msgDescription->headerFieldValues, msgHeaderInfo->headerLgth);
  fieldIdx = 1;
  headerFieldIdx = 0;
  offset = 0;
  COMP_MSG_DBG(self, "E", 2, "handleMsgHeadsLine: %s: %s", compMsgFile->lineFields[0], compMsgFile->lineFields[1]);
  while (fieldIdx < compMsgTypesAndNames->numSpecFieldIds) {
    fieldDescInfo = compMsgTypesAndNames->msgFieldInfos.fieldDescInfos[fieldIdx];
    if (fieldDescInfo == NULL) {
      fieldIdx++;
      continue;
    }
    if (fieldDescInfo->fieldFlags & COMP_MSG_FIELD_HEADER) {
      result = self->compMsgTypesAndNames->getFieldNameStrFromId(self, fieldIdx, &fieldName);
      checkErrOK(result);
      value = compMsgFile->lineFields[headerFieldIdx];
      COMP_MSG_DBG(self, "E", 2, "field: %s %s offset: %d", fieldName, value, fieldDescInfo->fieldOffset);
      if (fieldDescInfo->fieldFlags & COMP_MSG_FIELD_HEADER_UNIQUE) {
        if (c_strcmp(value, "*") == 0) {
// FIXME what to do with joker char?         
//          sgFieldDesc->fieldFlags |= COMP_MSG_VAL_IS_JOKER;
        } else {
          if (value[0] == '"') {
            switch (fieldDescInfo->fieldTypeId) {
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
            result = compMsgDataView->setIdFieldValue(self, dataView, fieldIdx, 0, stringVal, 0);
            checkErrOK(result);
          } else {
            result = compMsgFile->getIntFieldValue(self, value, &ep, 0, &numericValue);
            checkErrOK(result);
            result = compMsgDataView->setIdFieldValue(self, dataView, fieldIdx, numericValue, NULL, 0);
            checkErrOK(result);
          }
        }
      }
      headerFieldIdx++;
    }
    fieldIdx++;
  }
//dataView->dumpBinary(dataView->data, dataView->lgth, "HDR");
  value = compMsgFile->lineFields[headerFieldIdx];
  if (value[0] == '"') {
    result = compMsgFile->getStringFieldValue(self, value, &stringVal);
    checkErrOK(result);
    if (c_strlen(stringVal) > 1) {
      return COMP_MSG_ERR_BAD_ENCRYPTED_VALUE;
    }
    msgDescription->encrypted = stringVal[0];
  } else {
    return COMP_MSG_ERR_BAD_ENCRYPTED_VALUE;
  }
  headerFieldIdx++;
  value = compMsgFile->lineFields[headerFieldIdx];
  if (value[0] == '"') {
    result = compMsgFile->getStringFieldValue(self, value, &stringVal);
    checkErrOK(result);
    if (c_strlen(stringVal) > 1) {
      return COMP_MSG_ERR_BAD_HANDLE_TYPE_VALUE;
    }
    msgDescription->handleType = stringVal[0];
  } else {
    return COMP_MSG_ERR_BAD_HANDLE_TYPE_VALUE;
  }
  headerFieldIdx++;
  value = compMsgFile->lineFields[headerFieldIdx];
  if (value[0] == '"') {
    stringVal = NULL;
    result = compMsgFile->getStringFieldValue(self, value, &stringVal);
    checkErrOK(result);
    if (c_strlen(stringVal) > 2) {
      return COMP_MSG_ERR_BAD_CMD_KEY_VALUE;
    }
    msgDescription->cmdKey = (stringVal[0] << 8) | stringVal[1];
  } else {
    result = compMsgFile->getIntFieldValue(self, value, &ep, 0, &numericValue);
    checkErrOK(result);
    msgDescription->cmdKey = (uint16_t)numericValue;
  }
  COMP_MSG_DBG(self, "E", 2, "msgHeaderInfo->headerLgth: %d encrypted: %c handleType: %c", msgHeaderInfo->headerLgth, msgDescription->encrypted, msgDescription->handleType);
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
  case COMP_MSG_VAL_HEADER_FIELD_GROUP:
    result = self->compMsgMsgDesc->handleMsgValuesLine(self);
    checkErrOK(result);
    break;
  default:
    COMP_MSG_DBG(self, "E", 0, "bad desc file fieldGroupId 0x%02x cmdKey: 0x%04x", msgFieldGroupInfo->fieldGroupId, msgFieldGroupInfo->cmdKey);
    return COMP_MSG_ERR_BAD_DESC_FILE_FIELD_GROUP_TYPE;
  }
  return result;
}

// ================================= getMsgDescriptionFromUniqueFields ====================================

static uint8_t  getMsgDescriptionFromUniqueFields (compMsgDispatcher_t *self, headerValueInfos_t *headerValueInfos, msgDescription_t **msgDescription) {
  uint8_t result;
  int idx;
  int fieldIdx;
  int headerFieldIdx;
  uint8_t fieldNameId;
  uint8_t fieldId;
  bool found;
  headerValueInfo_t *headerValueInfo;
  msgDescriptionInfos_t *descriptions;
  fieldDescInfo_t *fieldDescInfo;
  dataView_t *dataView;
  dataValue_t checkValue;
  compMsgMsgDesc_t *compMsgMsgDesc;

  idx = 0;
  dataView = self->compMsgData->compMsgDataView->dataView;
  compMsgMsgDesc = self->compMsgMsgDesc;
  descriptions = &self->compMsgMsgDesc->msgDescriptionInfos;
  *msgDescription = NULL;
  while (idx < descriptions->numMsgDescriptions) {
    *msgDescription = &descriptions->msgDescriptions[idx];
    found = true;
    result = dataView->setDataViewData(dataView, (*msgDescription)->headerFieldValues, compMsgMsgDesc->msgHeaderInfo.headerLgth);
    checkErrOK(result);
    // for every field in headerValueInfos check for the equivalent field in msgDescription->fieldIds
    // and if found check if the value in headerValueInfos is the same as the value
    // in msgDescription for that field
    fieldIdx = 0;
    while (fieldIdx < headerValueInfos->numHeaderValues) {
      headerValueInfo = &headerValueInfos->headerValues[idx];
      result = self->compMsgTypesAndNames->getFieldNameIdFromStr(self, headerValueInfo->fieldName, &fieldNameId, COMP_MSG_NO_INCR);
      checkErrOK(result);
      fieldDescInfo = self->compMsgTypesAndNames->msgFieldInfos.fieldDescInfos[fieldNameId];
      if (fieldDescInfo->fieldFlags & COMP_MSG_FIELD_HEADER_UNIQUE) {
        result = self->compMsgData->compMsgDataView->getIdFieldValue(self, dataView, fieldNameId, &checkValue, 0);
        checkErrOK(result);
        // check fieldValue here for equality
        // we have to compare the value und a dataValue struct to the value in a message (which is a byte string)
        // FIXME! need code here!!
        result = self->compMsgDataValue->compareDataValues(self, fieldDescInfo, &headerValueInfo->dataValue, &checkValue);
        if (result != COMP_MSG_ERR_OK) {
          found = false;
          break;
        }
      } else {
        if (c_strcmp(headerValueInfo->fieldName, "@cmdKey") == 0) {
          // FIXME !! need to set checkValue here !!!
          result = self->compMsgDataValue->compareDataValues(self, fieldDescInfo, &headerValueInfo->dataValue, &checkValue);
          if (result != COMP_MSG_ERR_OK) {
            found = false;
            break;
          }
        }
      }
      if (!found) {
        break;
      }
      fieldIdx++;
    }
    if (found) {
      break;
    }
    idx++;
  }
  if (found) {
    return COMP_MSG_ERR_OK;
  }
  return COMP_MSG_ERR_HEADER_NOT_FOUND;
}

// ================================= compMsgMsgDescInit ====================================

static uint8_t compMsgMsgDescInit(compMsgDispatcher_t *self) {
  uint8_t result;
  compMsgMsgDesc_t *compMsgMsgDesc;

  compMsgMsgDesc = self->compMsgMsgDesc;

  compMsgMsgDesc->getHeaderChksumKey = &getHeaderChksumKey;
  compMsgMsgDesc->addHeaderInfo = &addHeaderInfo;
  compMsgMsgDesc->addMidPartInfo = &addMidPartInfo;
  compMsgMsgDesc->addTrailerInfo = &addTrailerInfo;
  compMsgMsgDesc->addFieldGroup = &addFieldGroup;

  compMsgMsgDesc->handleMsgCommonLine = &handleMsgCommonLine;
  compMsgMsgDesc->handleMsgFieldGroupLine = &handleMsgFieldGroupLine;
  compMsgMsgDesc->handleMsgFieldsToSaveLine = &handleMsgFieldsToSaveLine;
  compMsgMsgDesc->handleMsgActionsLine = &handleMsgActionsLine;
  compMsgMsgDesc->handleMsgValuesLine = &handleMsgValuesLine;
  compMsgMsgDesc->handleMsgHeadsLine = &handleMsgHeadsLine;
  compMsgMsgDesc->handleMsgFileNameLine = &handleMsgFileNameLine;
  compMsgMsgDesc->handleMsgFile = &handleMsgFile;

  compMsgMsgDesc->getMsgDescriptionFromUniqueFields = &getMsgDescriptionFromUniqueFields;

  // get the infos form the message desc and message val files.
  result = self->compMsgMsgDesc->handleMsgFile(self, MSG_FILES_FILE_NAME, self->compMsgMsgDesc->handleMsgFileNameLine);
  checkErrOK(result);
//  uint8_t data[10] = {0x12, 0x2, 0x15, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
//  result = self->compMsgMsgDesc->getHeaderChksumKey(self, data);
//  checkErrOK(result);
  // result = self->compMsgTypesAndNames->dumpMsgFieldDescInfos(self);
  // checkErrOK(result);
  // result = self->compMsgTypesAndNames->dumpMsgFieldValInfos(self);
  // checkErrOK(result);
  // result = self->compMsgDataValue->dumpMsgFieldValueInfos(self);
  // checkErrOK(result);
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
