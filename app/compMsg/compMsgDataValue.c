/*
* Copyright (c) 2017, Arnulf P. Wiedemann (arnulf@wiedemann-pri.de)
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
 * File:   compMsgDataValue.c
 * Author: Arnulf P. Wiedemann
 *
 * Created on April 14th, 2017
 */

#include "compMsg.h"

const static str2id_t dataValueStr2ValueIds [] = {
  { "@provisioningSsid",     COMP_MSG_WIFI_VALUE_ID_provisioningSsid},
  { "@provisioningPort",     COMP_MSG_WIFI_VALUE_ID_provisioningPort},
  { "@provisioningIPAddr",   COMP_MSG_WIFI_VALUE_ID_provisioningIPAddr},
  { "@SSDPIPAddr",           COMP_MSG_WIFI_VALUE_ID_SSDPIPAddr},
  { "@SSDPPort",             COMP_MSG_WIFI_VALUE_ID_SSDPPort},
  { "@SSDPReceivedCallback", COMP_MSG_WIFI_VALUE_ID_SSDPReceivedCallback},
  { "@SSDPToSendCallback",   COMP_MSG_WIFI_VALUE_ID_SSDPToSendCallback},
  { "@clientIPAddr",         COMP_MSG_WIFI_VALUE_ID_clientIPAddr},
  { "@clientPort",           COMP_MSG_WIFI_VALUE_ID_clientPort},
  { "@clientSequenceNum",    COMP_MSG_WIFI_VALUE_ID_clientSequenceNum},
  { "@clientSsid",           COMP_MSG_WIFI_VALUE_ID_clientSsid},
  { "@clientPasswd",         COMP_MSG_WIFI_VALUE_ID_clientPasswd},
  { "@clientStatus",         COMP_MSG_WIFI_VALUE_ID_clientStatus},
  { "@binaryCallback",       COMP_MSG_WIFI_VALUE_ID_binaryCallback},
  { "@textCallback",         COMP_MSG_WIFI_VALUE_ID_textCallback},
  { "@NetReceivedCallback",  COMP_MSG_WIFI_VALUE_ID_NetReceivedCallback},
  { "@NetToSendCallback",    COMP_MSG_WIFI_VALUE_ID_NetToSendCallback},
  { "@NetSecureConnect",     COMP_MSG_WIFI_VALUE_ID_NetSecureConnect},
  { "@cloudPort",            COMP_MSG_WIFI_VALUE_ID_cloudPort},
  { "@operatingMode",        COMP_MSG_WIFI_VALUE_ID_operatingMode},

  { "Reserve1",              COMP_MSG_MODULE_VALUE_ID_Reserve1},
  { "Reserve2",              COMP_MSG_MODULE_VALUE_ID_Reserve2},
  { "Reserve3",              COMP_MSG_MODULE_VALUE_ID_Reserve3},
  { "Reserve4",              COMP_MSG_MODULE_VALUE_ID_Reserve4},
  { "Reserve5",              COMP_MSG_MODULE_VALUE_ID_Reserve5},
  { "Reserve6",              COMP_MSG_MODULE_VALUE_ID_Reserve6},
  { "Reserve7",              COMP_MSG_MODULE_VALUE_ID_Reserve7},
  { "Reserve8",              COMP_MSG_MODULE_VALUE_ID_Reserve8},
  { "cryptKey",              COMP_MSG_MODULE_VALUE_ID_cryptKey},
  { "cryptIvKey",            COMP_MSG_MODULE_VALUE_ID_cryptIvKey},
  { NULL,          0 },
};

// ================================= dataValueStr2ValueId ====================================

static uint8_t dataValueStr2ValueId(compMsgDispatcher_t *self, uint8_t *valueStr, uint8_t *valueId) {
  const str2id_t *entry;

  entry = &dataValueStr2ValueIds[0];
  while (entry->str != NULL) {
    if (c_strcmp(entry->str, valueStr) == 0) {
      *valueId = entry->id;
      return COMP_MSG_ERR_OK;
    }
    entry++;
  }
  return COMP_MSG_ERR_FIELD_NOT_FOUND;
}

// ================================= dataValueId2ValueStr ====================================

static uint8_t dataValueId2ValueStr(compMsgDispatcher_t *self, uint8_t valueId, uint8_t **valueStr) {
  const str2id_t *entry;

  entry = &dataValueStr2ValueIds[0];
  while (entry->str != NULL) {
    if (entry->id == valueId) {
      *valueStr = entry->str;
      return COMP_MSG_ERR_OK;
    }
    entry++;
  }
  return COMP_MSG_ERR_FIELD_NOT_FOUND;
}

// ================================= addDataValue ====================================

static uint8_t addDataValue(compMsgDispatcher_t *self, dataValue_t *dataValue) {
  uint8_t result;
  compMsgDataValue_t *compMsgDataValue;
  dataValue_t *myDataValue;

  result = COMP_MSG_ERR_OK;
#ifdef NOTDEF
  compMsgDataValue = self->compMsgDataValue;
  if (compMsgDataValue->numDataValues >= compMsgDataValue->maxDataValues) {
    if (compMsgDataValue->maxDataValues == 0) {
      compMsgDataValue->maxDataValues = 5;
      compMsgDataValue->dataValues = (dataValue_t *)os_zalloc((compMsgDataValue->maxDataValues * sizeof(dataValue_t)));
      checkAllocOK(compMsgDataValue->dataValues);
    } else {
      compMsgDataValue->maxDataValues += 2;
      compMsgDataValue->dataValues = (dataValue_t *)os_realloc((compMsgDataValue->dataValues), (compMsgDataValue->maxDataValues * sizeof(dataValue_t)));
      checkAllocOK(compMsgDataValue->dataValues);
    }
  }
  myDataValue = &compMsgDataValue->dataValues[compMsgDataValue->numDataValues];
//COMP_MSG_DBG(self, "E", 1, "addDataValue numDataValues %d", compMsgDataValue->numDataValues);
  memset(myDataValue, 0, sizeof(dataValue_t));
#endif
#ifdef NOTDEF
  myDataValue->cmdKey = dataValue->cmdKey;
  myDataValue->fieldNameId = dataValue->fieldNameId;
  myDataValue->flags = 0;
  if (dataValue->flags & COMP_MSG_FIELD_IS_NUMERIC) {
    myDataValue->flags |= COMP_MSG_FIELD_IS_NUMERIC;
    myDataValue->value.numericValue = dataValue->value.numericValue;
  } else {
    myDataValue->flags |= COMP_MSG_FIELD_IS_STRING;
    myDataValue->value.stringValue = os_zalloc(c_strlen(dataValue->value.stringValue) + 1);
    c_memcpy(myDataValue->value.stringValue, dataValue->value.stringValue, c_strlen(dataValue->value.stringValue));
  }
  if (dataValue->fieldValueCallback != NULL) {
    myDataValue->flags |= COMP_MSG_FIELD_HAS_CALLBACK;
    myDataValue->fieldValueCallback = dataValue->fieldValueCallback;
  }
#endif
  compMsgDataValue->numDataValues++;
  return result;
}

// ================================= setDataVal ====================================

static uint8_t setDataVal(compMsgDispatcher_t *self, dataValue_t *dataValue) {
  uint8_t result;
  int idx;
  compMsgDataValue_t *compMsgDataValue;
  dataValue_t *myDataValue;

  result = COMP_MSG_ERR_OK;
  compMsgDataValue = self->compMsgDataValue;
#ifdef NOTDEF
  idx = 0;
  while (idx < compMsgDataValue->numDataValues) {
    myDataValue = &compMsgDataValue->dataValues[idx];
    if ((myDataValue->fieldNameId == dataValue->fieldNameId) &&
        (myDataValue->cmdKey == dataValue->cmdKey)) {
      if ((myDataValue->flags & COMP_MSG_FIELD_IS_STRING) && (myDataValue->value.stringValue != NULL)) {
        os_free(myDataValue->value.stringValue);
      }
      myDataValue->flags = 0;
      if (dataValue->flags & COMP_MSG_FIELD_IS_NUMERIC) {
        myDataValue->flags |= COMP_MSG_FIELD_IS_NUMERIC;
        myDataValue->value.numericValue = dataValue->value.numericValue;
      } else {
        myDataValue->flags |= COMP_MSG_FIELD_IS_STRING;
        myDataValue->value.stringValue = os_zalloc(c_strlen(dataValue->value.stringValue) + 1);
        c_memcpy(myDataValue->value.stringValue, dataValue->value.stringValue, c_strlen(dataValue->value.stringValue));
      }
      if (dataValue->fieldValueCallback != NULL) {
        myDataValue->flags |= COMP_MSG_FIELD_HAS_CALLBACK;
        myDataValue->fieldValueCallback = dataValue->fieldValueCallback;
      }
      return result;
    }
    idx++;
  }
#endif
  return COMP_MSG_ERR_DATA_VALUE_FIELD_NOT_FOUND;
}

// ================================= getDataVal ====================================

/**
 * \brief get value from data area
 * \param self The dispatcher struct
 * \param dataValue Data value struct pointer for passing values in and out
 * \return Error code or ErrorOK
 *
 */
static uint8_t getDataVal(compMsgDispatcher_t *self, dataValue_t *dataValue, uint8_t **valueStr) {
  uint8_t result;
  compMsgDataValue_t *compMsgDataValue;
  dataValue_t *myDataValue;
  int idx;

//COMP_MSG_DBG(self, "E", 1, "getDataVal: %p %d", dataValue, dataValue->fieldNameId);
  result = COMP_MSG_ERR_OK;
  compMsgDataValue = self->compMsgDataValue;
#ifdef NOTDEF
  dataValue->flags = 0;
  dataValue->value.stringValue = NULL;
  dataValue->fieldValueCallback = NULL;
  idx = 0;
  while (idx < compMsgDataValue->numDataValues) {
    myDataValue = &compMsgDataValue->dataValues[idx];
    if ((myDataValue->fieldNameId == dataValue->fieldNameId) &&
        (myDataValue->cmdKey == dataValue->cmdKey)) {
      *valueStr = NULL;
//      result = compMsgDataValue->dataValueId2ValueStr(self, dataValue->fieldValueId, valueStr);
//      checkErrOK(result);
      dataValue->flags = myDataValue->flags;
      if (myDataValue->flags & COMP_MSG_FIELD_IS_STRING) {
        dataValue->value.stringValue = myDataValue->value.stringValue;
      }
      if (myDataValue->flags & COMP_MSG_FIELD_IS_NUMERIC) {
        dataValue->value.numericValue = myDataValue->value.numericValue;
      }
      dataValue->fieldValueCallback = myDataValue->fieldValueCallback;
      return result;
    }
    idx++;
  }
  COMP_MSG_DBG(self, "E", 1, "getDataVal: DATA_VALUE_FIELD_NOT_FOUND: %d 0x%04x", dataValue->fieldNameId, dataValue->cmdKey);
#endif
  return COMP_MSG_ERR_DATA_VALUE_FIELD_NOT_FOUND;
}

// ================================= newMsgFieldValueInfos ====================================

static uint8_t newMsgFieldValueInfos(compMsgDispatcher_t *self, uint16_t cmdKey, int numEntries, msgFieldValue_t **msgFieldValue) {
  uint8_t result;
  int cmdKeyIdx;
  bool found;
  compMsgDataValue_t *compMsgDataValue;
  msgFieldValueInfos_t *msgFieldValueInfos;

  result = COMP_MSG_ERR_OK;
  found = false;
  compMsgDataValue = self->compMsgDataValue;
  msgFieldValueInfos = &compMsgDataValue->msgFieldValueInfos;
  cmdKeyIdx = 0;
//ets_printf("newMsgFieldValueInfos: cmdKey: 0x%04x numMsgFieldValues: %d\n", cmdKey, msgFieldValueInfos->numMsgFieldValues);
  while (cmdKeyIdx < msgFieldValueInfos->numMsgFieldValues) {
    *msgFieldValue = &msgFieldValueInfos->msgFieldValues[cmdKeyIdx];
    if ((*msgFieldValue)->cmdKey == cmdKey) {
      found = true;
      break;
    }
    cmdKeyIdx++;
  }
  if (!found) {
    if (msgFieldValueInfos->numMsgFieldValues >= msgFieldValueInfos->maxMsgFieldValues) {
      if (msgFieldValueInfos->maxMsgFieldValues == 0) {
        msgFieldValueInfos->maxMsgFieldValues = 5;
        msgFieldValueInfos->msgFieldValues = (msgFieldValue_t *)os_zalloc((msgFieldValueInfos->maxMsgFieldValues * sizeof(msgFieldValue_t)));
        checkAllocOK(msgFieldValueInfos->msgFieldValues);
      } else {
        msgFieldValueInfos->maxMsgFieldValues += 2;
        msgFieldValueInfos->msgFieldValues = (msgFieldValue_t *)os_realloc((msgFieldValueInfos->msgFieldValues), (msgFieldValueInfos->maxMsgFieldValues * sizeof(msgFieldValue_t)));
        checkAllocOK(msgFieldValueInfos->msgFieldValues);
      }
    }
    *msgFieldValue = &msgFieldValueInfos->msgFieldValues[msgFieldValueInfos->numMsgFieldValues];
    msgFieldValueInfos->numMsgFieldValues++;
    c_memset(*msgFieldValue, 0, sizeof(msgFieldValue_t));
    (*msgFieldValue)->cmdKey = cmdKey;
    (*msgFieldValue)->numFieldValues = numEntries;
    (*msgFieldValue)->fieldValues = os_zalloc(sizeof(fieldValue_t) * numEntries);
  }
  return result;
}
 
// ================================= getMsgFieldValueInfo ====================================

static uint8_t getMsgFieldValueInfo(compMsgDispatcher_t *self, uint16_t cmdKey, msgFieldValue_t **msgFieldValue) {
  uint8_t result;
  int cmdKeyIdx;
  compMsgDataValue_t *compMsgDataValue;
  msgFieldValueInfos_t *msgFieldValueInfos;

  result = COMP_MSG_ERR_OK;
  compMsgDataValue = self->compMsgDataValue;
  msgFieldValueInfos = &compMsgDataValue->msgFieldValueInfos;
  cmdKeyIdx = 0;
  while (cmdKeyIdx < msgFieldValueInfos->numMsgFieldValues) {
    *msgFieldValue = &msgFieldValueInfos->msgFieldValues[cmdKeyIdx];
    if ((*msgFieldValue)->cmdKey == cmdKey) {
      return COMP_MSG_ERR_OK;
    }
    cmdKeyIdx++;
  }
  return COMP_MSG_ERR_FIELD_VALUE_INFO_NOT_FOUND;
}
 
// ================================= setMsgFieldValueInfo ====================================

static uint8_t setMsgFieldValueInfo(compMsgDispatcher_t *self, msgFieldValue_t *msgFieldValue, fieldValue_t *fieldValue) {
  uint8_t result;
  int fieldValueIdx;
  bool found;
  fieldValue_t *myFieldValue;
  fieldValue_t *freeFieldValue;
  compMsgDataValue_t *compMsgDataValue;

  result = COMP_MSG_ERR_OK;
  fieldValueIdx = 0;
  found = false;
  freeFieldValue = NULL;
  compMsgDataValue = self->compMsgDataValue;
//ets_printf("setMsgFieldValueInfo: numFieldValues: %d\n", msgFieldValue->numFieldValues);
  // first check if this entry exists, if not take the first unused one
  while (fieldValueIdx < msgFieldValue->numFieldValues) { 
    myFieldValue = &msgFieldValue->fieldValues[fieldValueIdx];
    if (myFieldValue->fieldNameId == 0) {
      if (freeFieldValue == NULL) {
        freeFieldValue = myFieldValue;
      }
    } else {
      if (myFieldValue->fieldNameId == fieldValue->fieldNameId) {
        found = true;
        break;
      }
    }
    fieldValueIdx++;
  }
  if (found) {
ets_printf("setMsgFieldValueInfo: duplicate entry for: %d\n", fieldValue->fieldNameId);
      return COMP_MSG_ERR_DUPLICATE_FIELD_VALUE_ENTRY;
  } else {
    if (freeFieldValue != NULL) {
      myFieldValue = freeFieldValue;
int numericValue;
uint8_t *stringValue;
uint8_t *fieldName;
result = self->compMsgTypesAndNames->getFieldNameStrFromId(self, fieldValue->fieldNameId, &fieldName);
checkErrOK(result);
if (fieldValue->fieldValueFlags & COMP_MSG_FIELD_IS_STRING) {
  stringValue = fieldValue->dataValue.value.stringValue;
  numericValue = 0;
} else {
  stringValue = "nil";
  numericValue = fieldValue->dataValue.value.numericValue;
}
//ets_printf("found free %s flags: 0x%08x fieldNameId: %d %s 0x%04x %d callbackId: %d\n", fieldName, fieldValue->fieldValueFlags, fieldValue->fieldNameId, stringValue, numericValue, numericValue, fieldValue->fieldValueCallbackId);
      myFieldValue->fieldNameId = fieldValue->fieldNameId;
      myFieldValue->fieldValueFlags = fieldValue->fieldValueFlags;
      myFieldValue->dataValue.value = fieldValue->dataValue.value;
      myFieldValue->fieldValueCallbackId = fieldValue->fieldValueCallbackId;
      myFieldValue->fieldValueCallback = fieldValue->fieldValueCallback;
    } else {
ets_printf("setMsgFieldValueInfo: not found too many entries\n");
      return COMP_MSG_ERR_TOO_MANY_FIELD_VALUE_ENTRIES;
    }
  }
  return result;
}
 
// ================================= dumpMsgFieldValueInfos ====================================

static uint8_t dumpMsgFieldValueInfos(compMsgDispatcher_t *self) {
  int result;
  int cmdKeyIdx;
  int fieldValueIdx;
  char buf[512];
  uint8_t *fieldName;
  uint8_t *callbackName;
  uint8_t *fieldType;
  int numericValue;
  uint8_t *stringValue;
  compMsgTypesAndNames_t *compMsgTypesAndNames;
  compMsgDataValue_t *compMsgDataValue;
  msgFieldValueInfos_t *msgFieldValueInfos;
  msgFieldValue_t *msgFieldValue;
  fieldValue_t *fieldValue;
  bool noPrefix;

  result = COMP_MSG_ERR_OK;
  noPrefix = self->compMsgDebug->noPrefix;
  self->compMsgDebug->noPrefix = true;
  compMsgTypesAndNames = self->compMsgTypesAndNames;
  compMsgDataValue = self->compMsgDataValue;
  COMP_MSG_DBG(self, "d", 1, "dumpMsgFieldValueInfos");
  msgFieldValueInfos = &compMsgDataValue->msgFieldValueInfos;
  // entry 0 is not used, fieldNameIds start at 1!
  cmdKeyIdx = 0;
  while (cmdKeyIdx < msgFieldValueInfos->numMsgFieldValues) {
    msgFieldValue = &msgFieldValueInfos->msgFieldValues[cmdKeyIdx];
    ets_sprintf(buf, "  %02d cmdKey: 0x%04x\n", cmdKeyIdx, msgFieldValue->cmdKey);
    COMP_MSG_DBG(self, "d", 1, "%s", buf);
    buf[0] = '\0';
    fieldValueIdx = 0;
    while (fieldValueIdx < msgFieldValue->numFieldValues) {
      fieldValue = &msgFieldValue->fieldValues[fieldValueIdx];
      if (fieldValue->fieldNameId == 0) {
      } else {
        result = compMsgTypesAndNames->getFieldNameStrFromId(self, fieldValue->fieldNameId, &fieldName);
        if (fieldValue->fieldValueFlags & COMP_MSG_FIELD_IS_STRING) {
          numericValue = 0;
          stringValue = fieldValue->dataValue.value.stringValue;
          if (stringValue == NULL) {
            stringValue = "nil";
          }
        } else {
          numericValue = fieldValue->dataValue.value.numericValue;
          stringValue = "nil";
        }
        if (fieldValue->fieldValueCallbackId == 0) {
          callbackName = "nil";
        } else {
          result = self->compMsgWifiData->callbackId2CallbackStr(fieldValue->fieldValueCallbackId, &callbackName);
ets_printf("id: %d result: %d\n", fieldValue->fieldValueCallbackId, result);
          if (result != COMP_MSG_ERR_OK) {
            result = self->compMsgModuleData->callbackId2CallbackStr(fieldValue->fieldValueCallbackId, &callbackName);
ets_printf("id2: %d result: %d\n", fieldValue->fieldValueCallbackId, result);
          }
        }
        ets_sprintf(buf, "    %-20s: id: %d value: %-15s 0x%08x %6d callback: %-20s %3d flags: 0x%04x\n", fieldName, fieldValue->fieldNameId, stringValue, numericValue, numericValue, callbackName, fieldValue->fieldValueCallbackId, fieldValue->fieldValueFlags);
        if (fieldValue->fieldValueFlags & COMP_MSG_FIELD_IS_STRING) {
          c_strcat(buf, " IS_STRING");
        }
        if (fieldValue->fieldValueFlags & COMP_MSG_FIELD_IS_NUMERIC) {
          c_strcat(buf, " IS_NUMERIC");
        }
        if (fieldValue->fieldValueFlags & COMP_MSG_FIELD_HAS_CALLBACK) {
          c_strcat(buf, " HAS_CALLBACK");
        }
        COMP_MSG_DBG(self, "d", 1, "%s", buf);
      }
      fieldValueIdx++;
    }
    cmdKeyIdx++;
  }
  self->compMsgDebug->noPrefix = noPrefix;
  return result;
}

// ================================= setFieldValueInfo ====================================

static uint8_t setFieldValueInfo(compMsgDispatcher_t *self, uint16_t cmdKey, uint16_t fieldNameId, fieldValue_t *fieldValue) {
  uint8_t result;
  int idx;
  int fieldIdx;
  int numericValue;
  uint8_t *stringValue;
  bool found;
  compMsgDataValue_t *compMsgDataValue;
  msgFieldValueInfos_t *msgFieldValueInfos;
  msgFieldValue_t *msgFieldValue;
  fieldValue_t *myFieldValue;

  result = COMP_MSG_ERR_OK;
  found = false;
  compMsgDataValue = self->compMsgDataValue;
  msgFieldValueInfos = &compMsgDataValue->msgFieldValueInfos;
  idx = 0;
  while (idx < msgFieldValueInfos->numMsgFieldValues) {
    msgFieldValue = &msgFieldValueInfos->msgFieldValues[idx];
    if (msgFieldValue->cmdKey == cmdKey) {
      fieldIdx = 0;
      while (fieldIdx < msgFieldValue->numFieldValues) {
        myFieldValue = &msgFieldValue->fieldValues[fieldIdx];
        if ((myFieldValue->fieldNameId == fieldNameId)) {
          if ((myFieldValue->fieldValueFlags & COMP_MSG_FIELD_IS_STRING) && (myFieldValue->dataValue.value.stringValue != NULL)) {
            os_free(myFieldValue->dataValue.value.stringValue);
          }
          myFieldValue->fieldValueFlags = fieldValue->fieldValueFlags;
          if (fieldValue->fieldValueFlags & COMP_MSG_FIELD_IS_NUMERIC) {
            myFieldValue->dataValue.value.numericValue = fieldValue->dataValue.value.numericValue;
stringValue = "nil";
numericValue = myFieldValue->dataValue.value.numericValue;
          } else {
            myFieldValue->fieldValueFlags |= COMP_MSG_FIELD_IS_STRING;
            myFieldValue->dataValue.value.stringValue = os_zalloc(c_strlen(fieldValue->dataValue.value.stringValue) + 1);
            c_memcpy(myFieldValue->dataValue.value.stringValue, fieldValue->dataValue.value.stringValue, c_strlen(fieldValue->dataValue.value.stringValue));
    stringValue = myFieldValue->dataValue.value.stringValue;
  numericValue = 0;
          }
          myFieldValue->fieldValueCallbackId = fieldValue->fieldValueCallbackId;
          if (fieldValue->fieldValueCallback != NULL) {
            myFieldValue->fieldValueFlags |= COMP_MSG_FIELD_HAS_CALLBACK;
            myFieldValue->fieldValueCallback = fieldValue->fieldValueCallback;
          }
  ets_printf("setFieldValueInfo: flags: 0x%04x numeric: 0x%04x %d string: %s callback: %p\n", fieldValue->fieldValueFlags, numericValue, numericValue, stringValue, myFieldValue->fieldValueCallback);
          return result;
        }
        idx++;
      }
    }
  }
  return COMP_MSG_ERR_DATA_VALUE_FIELD_NOT_FOUND;
}

// ================================= getFieldValueInfo ====================================

/**
 * \brief get value from data area
 * \param self The dispatcher struct
 * \param cmdKey Message cmdKey
 * \param fieldNameId Message fieldNameId within cmdKey
 * \param fieldValueInfo FieldValue struct pointer for passing values out
 * \return Error code or ErrorOK
 *
 */
static uint8_t getFieldValueInfo(compMsgDispatcher_t *self, uint16_t cmdKey, uint16_t fieldNameId, fieldValue_t **fieldValue) {
  uint8_t result;
  int cmdKeyIdx;
  compMsgDataValue_t *compMsgDataValue;
  msgFieldValueInfos_t *msgFieldValueInfos;
  msgFieldValue_t *msgFieldValue;
  fieldValue_t *myFieldValue;
  int idx;

COMP_MSG_DBG(self, "E", 2, "getFieldValueInfo: %p %d 0x%04x", fieldValue, fieldNameId, cmdKey);
  result = COMP_MSG_ERR_OK;
  compMsgDataValue = self->compMsgDataValue;
  msgFieldValueInfos = &compMsgDataValue->msgFieldValueInfos;
  cmdKeyIdx = 0;
  while (cmdKeyIdx < msgFieldValueInfos->numMsgFieldValues) {
    msgFieldValue = &msgFieldValueInfos->msgFieldValues[cmdKeyIdx];
    if (msgFieldValue->cmdKey == cmdKey) {
      idx = 0;
      while (idx < msgFieldValue->numFieldValues) {
        myFieldValue = &msgFieldValue->fieldValues[idx];
        if ((myFieldValue->fieldNameId == fieldNameId)) {
          *fieldValue = myFieldValue;
          return result;
        }
        idx++;
      }
    }
    cmdKeyIdx++;
  }
  COMP_MSG_DBG(self, "E", 1, "getFieldValueInfo: DATA_VALUE_FIELD_NOT_FOUND: 0x%04x %d", cmdKey, fieldNameId);
  return COMP_MSG_ERR_DATA_VALUE_FIELD_NOT_FOUND;
}

// ================================= dumpMsgFieldValues ====================================

static uint8_t dumpMsgFieldValues(compMsgDispatcher_t *self) {
  int result;
  int idx;
  char buf[512];
  uint8_t *fieldName;
  uint8_t *fieldType;
  int numericValue;
  uint8_t *stringValue;
  compMsgTypesAndNames_t *compMsgTypesAndNames;
  compMsgDataValue_t *compMsgDataValue;
  fieldDescInfo_t *fieldDescInfo;
  msgFieldValue_t *msgFieldValue;
  fieldValue_t *fieldValue;

  result = COMP_MSG_ERR_OK;
  compMsgTypesAndNames = self->compMsgTypesAndNames;
  compMsgDataValue = self->compMsgDataValue;
  COMP_MSG_DBG(self, "d", 1, "dumpMsgFieldValues");
  // entry 0 is not used, fieldNameIds start at 1!
  idx = 1;
  while (idx < compMsgDataValue->msgFieldValueInfos.msgFieldValues->numFieldValues) {
    if (idx == compMsgTypesAndNames->numSpecFieldIds) {
      idx++;
      continue;
    }
    fieldDescInfo = compMsgTypesAndNames->msgFieldInfos.fieldDescInfos[idx];
    fieldValue = &compMsgDataValue->msgFieldValueInfos.msgFieldValues->fieldValues[idx];
    result = compMsgTypesAndNames->getFieldNameStrFromId(self, idx, &fieldName);
    checkErrOK(result);
    if (fieldValue == NULL) {
      ets_sprintf(buf, "%3d %-20s empty", idx, fieldName);
    } else {
      result = compMsgTypesAndNames->getFieldTypeStrFromId(self, fieldDescInfo->fieldTypeId, &fieldType);
      checkErrOK(result);
      numericValue = 0;
      stringValue = NULL;
      switch (fieldDescInfo->fieldTypeId) {
      case DATA_VIEW_FIELD_NONE:
        break;
      case DATA_VIEW_FIELD_UINT8_T:
        numericValue = (int)fieldValue->dataValue.value.u8;
        break;
      case DATA_VIEW_FIELD_INT8_T:
        numericValue = (int)fieldValue->dataValue.value.i8;
        break;
      case DATA_VIEW_FIELD_UINT16_T:
        numericValue = (int)fieldValue->dataValue.value.u16;
        break;
      case DATA_VIEW_FIELD_INT16_T:
        numericValue = (int)fieldValue->dataValue.value.i16;
        break;
      case DATA_VIEW_FIELD_UINT32_T:
        numericValue = (int)fieldValue->dataValue.value.u32;
        break;
      case DATA_VIEW_FIELD_INT32_T:
        numericValue = (int)fieldValue->dataValue.value.i32;
        break;
      case DATA_VIEW_FIELD_UINT8_VECTOR:
        stringValue = fieldValue->dataValue.value.u8vec;
        break;
      case DATA_VIEW_FIELD_INT8_VECTOR:
        stringValue = fieldValue->dataValue.value.i8vec;
        break;
      case DATA_VIEW_FIELD_UINT16_VECTOR:
//        stringValue = fieldValue->dataValue.value.u16vec;
        break;
      case DATA_VIEW_FIELD_INT16_VECTOR:
//        stringValue = fieldValue->dataValue.value.i16vec;
        break;
      case DATA_VIEW_FIELD_UINT32_VECTOR:
//        stringValue = fieldValue->dataValue.value.u32vec;
        break;
      case DATA_VIEW_FIELD_INT32_VECTOR:
//        stringValue = fieldValue->dataValue.value.i32vec;
        break;
      default:
        return COMP_MSG_ERR_BAD_FIELD_TYPE;
      }
      ets_sprintf(buf, "%3d %-20s type: %-10s %d lgth: %3d value: %s %d 0x%08x flags: 0x%04x", idx, fieldName, fieldType, fieldDescInfo->fieldTypeId, fieldDescInfo->fieldLgth, stringValue == NULL ? "nil" : (char *)stringValue, numericValue, numericValue, fieldDescInfo->fieldFlags);
      if (fieldDescInfo->fieldFlags & COMP_MSG_FIELD_WIFI_DATA) {
        c_strcat(buf, " WIFI_DATA");
      }
      if (fieldDescInfo->fieldFlags & COMP_MSG_FIELD_MODULE_DATA) {
        c_strcat(buf, " MODULE_DATA");
      }
      if (fieldDescInfo->fieldFlags & COMP_MSG_FIELD_TO_SAVE) {
        c_strcat(buf, " TO_SAVE");
      }
      if (fieldDescInfo->fieldFlags & COMP_MSG_FIELD_KEY_VALUE) {
        c_strcat(buf, " KEY_VALUE");
      }
      if (fieldDescInfo->fieldFlags & COMP_MSG_FIELD_HEADER) {
        c_strcat(buf, " HEADER");
      }
      if (fieldDescInfo->fieldFlags & COMP_MSG_FIELD_HEADER_UNIQUE) {
        c_strcat(buf, " HEADER_UNIQUE");
      }
      if (fieldDescInfo->fieldFlags & COMP_MSG_FIELD_HEADER_CHKSUM_NON_ZERO) {
        c_strcat(buf, " HEADER_CHKSUM_NON_ZERO");
      }
    }
    COMP_MSG_DBG(self, "d", 1, "%s", buf);
    idx++;
  }
  return result;
}

// ================================= addMsgFieldValues ====================================

static uint8_t addMsgFieldValues(compMsgDispatcher_t *self, uint8_t numEntries) {
  uint8_t result;

  result = COMP_MSG_ERR_OK;
  int idx;
  msgFieldValueInfos_t *msgFieldValueInfos;
  msgFieldValue_t *msgFieldValue;
  fieldValue_t *fieldValue;

  result = COMP_MSG_ERR_OK;
  msgFieldValueInfos = &self->compMsgDataValue->msgFieldValueInfos;
  if (msgFieldValueInfos->numMsgFieldValues == 0) {       
    msgFieldValueInfos->msgFieldValues = (msgFieldValue_t *)os_zalloc(((msgFieldValueInfos->numMsgFieldValues + numEntries) * sizeof(msgFieldValue_t *)));
  } else {
    msgFieldValueInfos->msgFieldValues = (msgFieldValue_t *)os_realloc(msgFieldValueInfos->msgFieldValues, ((msgFieldValueInfos->numMsgFieldValues + numEntries) * sizeof(msgFieldValue_t *)));
  }
  checkAllocOK(msgFieldValueInfos->msgFieldValues);
  idx = msgFieldValueInfos->numMsgFieldValues;
  COMP_MSG_DBG(self, "E", 1, "addMsgFieldValues: numEntries: %d, numMsgFields: %d", numEntries, msgFieldValueInfos->numMsgFieldValues);
  while (idx < msgFieldValueInfos->numMsgFieldValues + numEntries) {
    c_memset(&msgFieldValueInfos->msgFieldValues[idx], 0, sizeof(msgFieldValue_t));
    idx++;
  }
  msgFieldValueInfos->numMsgFieldValues += numEntries;
  return result;
}

// ================================= setMsgFieldValue ====================================

static uint8_t setMsgFieldValue(compMsgDispatcher_t *self, uint8_t idx, fieldValue_t *fieldValue) {
  uint8_t result;
  msgFieldValueInfos_t *msgFieldValueInfos;
  msgFieldValue_t *entry;

  result = COMP_MSG_ERR_OK;
  msgFieldValueInfos = &self->compMsgDataValue->msgFieldValueInfos;
  if (idx >= msgFieldValueInfos->numMsgFieldValues) {
    return COMP_MSG_ERR_BAD_MSG_FIELD_INFO_IDX;
  }
  entry = &msgFieldValueInfos->msgFieldValues[idx];
// FIXME!!
//  entry->fieldValueFlags = fieldValue->fieldValueFlags;
//  COMP_MSG_DBG(self, "E", 0, "setMsgFieldValue: idx: %d fieldValueFlags: 0x%08x", idx, entry->fieldValueFlags);
  return result;
}

// ================================= getMsgFieldValue ====================================

static uint8_t getMsgFieldValue(compMsgDispatcher_t *self, uint8_t idx, fieldValue_t *fieldValue) {
  uint8_t result;
  msgFieldValueInfos_t *msgFieldValueInfos;
  msgFieldValue_t *entry;

  result = COMP_MSG_ERR_OK;
  msgFieldValueInfos = &self->compMsgDataValue->msgFieldValueInfos;
  if (idx >= msgFieldValueInfos->numMsgFieldValues) {
    return COMP_MSG_ERR_BAD_MSG_FIELD_INFO_IDX;
  }
  entry = &msgFieldValueInfos->msgFieldValues[idx];
// FIXME !!!!
//  fieldValue->fieldValueFlags = entry->flags;
//  fieldValue->dataValue = entry->dataValue;
  return result;
}

// ================================= compareDataValues ====================================

static uint8_t compareDataValues(compMsgDispatcher_t *self, fieldDescInfo_t *fieldDescInfo, dataValue_t *dataValue1, dataValue_t *dataValue2) {
  switch (fieldDescInfo->fieldTypeId) {
  case DATA_VIEW_FIELD_UINT8_T:
    break;
  default:
    return COMP_MSG_ERR_BAD_FIELD_TYPE;
  }
  return COMP_MSG_ERR_DIFFERENT_FIELD_VALUE;
}

// ================================= compMsgDataValueInit ====================================

static uint8_t compMsgDataValueInit(compMsgDispatcher_t *self) {
  uint8_t result;
  compMsgDataValue_t *compMsgDataValue;

  compMsgDataValue = self->compMsgDataValue;
  compMsgDataValue->dataValueStr2ValueId = &dataValueStr2ValueId;
  compMsgDataValue->dataValueId2ValueStr = &dataValueId2ValueStr;
  compMsgDataValue->addDataValue = &addDataValue;
  compMsgDataValue->setDataVal = &setDataVal;
  compMsgDataValue->getDataVal = &getDataVal;

  compMsgDataValue->newMsgFieldValueInfos = &newMsgFieldValueInfos;
  compMsgDataValue->getMsgFieldValueInfo = &getMsgFieldValueInfo;
  compMsgDataValue->setMsgFieldValueInfo = &setMsgFieldValueInfo;
  compMsgDataValue->dumpMsgFieldValueInfos = &dumpMsgFieldValueInfos;

  compMsgDataValue->setFieldValueInfo = &setFieldValueInfo;
  compMsgDataValue->getFieldValueInfo = &getFieldValueInfo;
  compMsgDataValue->dumpMsgFieldValues = &dumpMsgFieldValues;

  compMsgDataValue->addMsgFieldValues = &addMsgFieldValues;
  compMsgDataValue->getMsgFieldValue = &getMsgFieldValue;
  compMsgDataValue->setMsgFieldValue = &setMsgFieldValue;
  compMsgDataValue->compareDataValues = &compareDataValues;
  return COMP_MSG_ERR_OK;
}

// ================================= newCompMsgDataValue ====================================

compMsgDataValue_t *newCompMsgDataValue() {
  compMsgDataValue_t *compMsgDataValue;

  compMsgDataValue = os_zalloc(sizeof(compMsgDataValue_t));
  if (compMsgDataValue == NULL) {
    return NULL;
  }
  compMsgDataValue->compMsgDataValueInit = &compMsgDataValueInit;
  return compMsgDataValue;
}
