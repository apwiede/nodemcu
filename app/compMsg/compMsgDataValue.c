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
  myDataValue->fieldValueId = dataValue->fieldValueId;
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
    if ((myDataValue->fieldValueId == dataValue->fieldValueId) &&
        (myDataValue->fieldNameId == dataValue->fieldNameId) &&
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

//COMP_MSG_DBG(self, "E", 1, "getDataVal: %p %d %d 0x%04x", dataValue, dataValue->fieldValueId, dataValue->fieldNameId, dataValue->cmdKey);
  result = COMP_MSG_ERR_OK;
  compMsgDataValue = self->compMsgDataValue;
#ifdef NOTDEF
  dataValue->flags = 0;
  dataValue->value.stringValue = NULL;
  dataValue->fieldValueCallback = NULL;
  idx = 0;
  while (idx < compMsgDataValue->numDataValues) {
    myDataValue = &compMsgDataValue->dataValues[idx];
    if ((myDataValue->fieldValueId == dataValue->fieldValueId) &&
        (myDataValue->fieldNameId == dataValue->fieldNameId) &&
        (myDataValue->cmdKey == dataValue->cmdKey)) {
      *valueStr = NULL;
      result = compMsgDataValue->dataValueId2ValueStr(self, dataValue->fieldValueId, valueStr);
      checkErrOK(result);
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
  COMP_MSG_DBG(self, "E", 1, "getDataVal: DATA_VALUE_FIELD_NOT_FOUND: %d %d 0x%04x", dataValue->fieldValueId, dataValue->fieldNameId, dataValue->cmdKey);
#endif
  return COMP_MSG_ERR_DATA_VALUE_FIELD_NOT_FOUND;
}

// ================================= addFieldValueInfo ====================================

static uint8_t addFieldValueInfo(compMsgDispatcher_t *self, fieldValue_t *fieldValue) {
  uint8_t result;
  compMsgDataValue_t *compMsgDataValue;
  fieldValue_t *myFieldValue;

  result = COMP_MSG_ERR_OK;
  compMsgDataValue = self->compMsgDataValue;
  if (compMsgDataValue->numDataValues >= compMsgDataValue->maxDataValues) {
    if (compMsgDataValue->maxDataValues == 0) {
      compMsgDataValue->maxDataValues = 5;
      compMsgDataValue->fieldValues = (fieldValue_t *)os_zalloc((compMsgDataValue->maxDataValues * sizeof(fieldValue_t)));
      checkAllocOK(compMsgDataValue->fieldValues);
    } else {
      compMsgDataValue->maxDataValues += 2;
      compMsgDataValue->fieldValues = (fieldValue_t *)os_realloc((compMsgDataValue->fieldValues), (compMsgDataValue->maxDataValues * sizeof(fieldValue_t)));
      checkAllocOK(compMsgDataValue->fieldValues);
    }
  }
  myFieldValue = &compMsgDataValue->fieldValues[compMsgDataValue->numDataValues];
//COMP_MSG_DBG(self, "E", 1, "addFieldValueInfo numFieldValueInfos %d", compMsgDataValue->numDataValues);
  memset(myFieldValue, 0, sizeof(fieldValue_t));
  myFieldValue->cmdKey = fieldValue->cmdKey;
  myFieldValue->fieldValueId = fieldValue->fieldValueId;
  myFieldValue->fieldNameId = fieldValue->fieldNameId;
  myFieldValue->flags = 0;
  if (fieldValue->flags & COMP_MSG_FIELD_IS_NUMERIC) {
    myFieldValue->flags |= COMP_MSG_FIELD_IS_NUMERIC;
    myFieldValue->dataValue.value.numericValue = fieldValue->dataValue.value.numericValue;
  } else {
    myFieldValue->flags |= COMP_MSG_FIELD_IS_STRING;
    myFieldValue->dataValue.value.stringValue = os_zalloc(c_strlen(fieldValue->dataValue.value.stringValue) + 1);
    c_memcpy(myFieldValue->dataValue.value.stringValue, fieldValue->dataValue.value.stringValue, c_strlen(fieldValue->dataValue.value.stringValue));
  }
  if (fieldValue->fieldValueCallback != NULL) {
    myFieldValue->flags |= COMP_MSG_FIELD_HAS_CALLBACK;
    myFieldValue->fieldValueCallback = fieldValue->fieldValueCallback;
  }
  compMsgDataValue->numDataValues++;
  return result;
}

// ================================= setFieldValueInfo ====================================

static uint8_t setFieldValueInfo(compMsgDispatcher_t *self, fieldValue_t *fieldValue) {
  uint8_t result;
  int idx;
  compMsgDataValue_t *compMsgDataValue;
  fieldValue_t *myFieldValue;

  result = COMP_MSG_ERR_OK;
  compMsgDataValue = self->compMsgDataValue;
  idx = 0;
  while (idx < compMsgDataValue->numDataValues) {
    myFieldValue = &compMsgDataValue->fieldValues[idx];
    if ((myFieldValue->fieldValueId == fieldValue->fieldValueId) &&
        (myFieldValue->fieldNameId == fieldValue->fieldNameId) &&
        (myFieldValue->cmdKey == fieldValue->cmdKey)) {
      if ((myFieldValue->flags & COMP_MSG_FIELD_IS_STRING) && (myFieldValue->dataValue.value.stringValue != NULL)) {
        os_free(myFieldValue->dataValue.value.stringValue);
      }
      myFieldValue->flags = 0;
      if (fieldValue->flags & COMP_MSG_FIELD_IS_NUMERIC) {
        myFieldValue->flags |= COMP_MSG_FIELD_IS_NUMERIC;
        myFieldValue->dataValue.value.numericValue = fieldValue->dataValue.value.numericValue;
      } else {
        myFieldValue->flags |= COMP_MSG_FIELD_IS_STRING;
        myFieldValue->dataValue.value.stringValue = os_zalloc(c_strlen(fieldValue->dataValue.value.stringValue) + 1);
        c_memcpy(myFieldValue->dataValue.value.stringValue, fieldValue->dataValue.value.stringValue, c_strlen(fieldValue->dataValue.value.stringValue));
      }
      if (fieldValue->fieldValueCallback != NULL) {
        myFieldValue->flags |= COMP_MSG_FIELD_HAS_CALLBACK;
        myFieldValue->fieldValueCallback = fieldValue->fieldValueCallback;
      }
      return result;
    }
    idx++;
  }
  return COMP_MSG_ERR_DATA_VALUE_FIELD_NOT_FOUND;
}

// ================================= getFieldValueInfo ====================================

/**
 * \brief get value from data area
 * \param self The dispatcher struct
 * \param fieldValueInfo Data value struct pointer for passing values in and out
 * \return Error code or ErrorOK
 *
 */
static uint8_t getFieldValueInfo(compMsgDispatcher_t *self, fieldValue_t *fieldValue, uint8_t **valueStr) {
  uint8_t result;
  compMsgDataValue_t *compMsgDataValue;
  fieldValue_t *myFieldValue;
  int idx;

//COMP_MSG_DBG(self, "E", 1, "getFieldValueInfo: %p %d %d 0x%04x", fieldValue, fieldValue->fieldValueId, fieldValue->fieldNameId, fieldValue->cmdKey);
  result = COMP_MSG_ERR_OK;
  compMsgDataValue = self->compMsgDataValue;
  fieldValue->flags = 0;
  fieldValue->dataValue.value.stringValue = NULL;
  fieldValue->fieldValueCallback = NULL;
  idx = 0;
  while (idx < compMsgDataValue->numDataValues) {
    myFieldValue = &compMsgDataValue->fieldValues[idx];
    if ((myFieldValue->fieldValueId == fieldValue->fieldValueId) &&
        (myFieldValue->fieldNameId == fieldValue->fieldNameId) &&
        (myFieldValue->cmdKey == fieldValue->cmdKey)) {
      *valueStr = NULL;
      result = compMsgDataValue->dataValueId2ValueStr(self, fieldValue->fieldValueId, valueStr);
      checkErrOK(result);
      fieldValue->flags = myFieldValue->flags;
      if (myFieldValue->flags & COMP_MSG_FIELD_IS_STRING) {
        fieldValue->dataValue.value.stringValue = myFieldValue->dataValue.value.stringValue;
      }
      if (myFieldValue->flags & COMP_MSG_FIELD_IS_NUMERIC) {
        fieldValue->dataValue.value.numericValue = myFieldValue->dataValue.value.numericValue;
      }
      fieldValue->fieldValueCallback = myFieldValue->fieldValueCallback;
      return result;
    }
    idx++;
  }
  COMP_MSG_DBG(self, "E", 1, "getFieldValueInfo: DATA_VALUE_FIELD_NOT_FOUND: %d %d 0x%04x", fieldValue->fieldValueId, fieldValue->fieldNameId, fieldValue->cmdKey);
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
  fieldInfo_t *fieldInfo;
  fieldValue_t *fieldValue;

  result = COMP_MSG_ERR_OK;
  compMsgTypesAndNames = self->compMsgTypesAndNames;
  compMsgDataValue = self->compMsgDataValue;
  COMP_MSG_DBG(self, "d", 1, "dumpMsgFieldValues");
  // entry 0 is not used, fieldNameIds start at 1!
  idx = 1;
  while (idx < compMsgDataValue->msgFieldValues.numMsgFields) {
    if (idx == compMsgTypesAndNames->numSpecFieldIds) {
      idx++;
      continue;
    }
    fieldInfo = compMsgTypesAndNames->msgFieldInfos.fieldInfos[idx];
    fieldValue = compMsgDataValue->msgFieldValues.fieldValues[idx];
    result = compMsgTypesAndNames->getFieldNameStrFromId(self, idx, &fieldName);
    checkErrOK(result);
    if (fieldValue == NULL) {
      ets_sprintf(buf, "%3d %-20s empty", idx, fieldName);
    } else {
      result = compMsgTypesAndNames->getFieldTypeStrFromId(self, fieldInfo->fieldTypeId, &fieldType);
      checkErrOK(result);
      numericValue = 0;
      stringValue = NULL;
      switch (fieldInfo->fieldTypeId) {
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
      ets_sprintf(buf, "%3d %-20s type: %-10s %d lgth: %3d value: %s %d 0x%08x flags: 0x%04x", idx, fieldName, fieldType, fieldInfo->fieldTypeId, fieldInfo->fieldLgth, stringValue == NULL ? "nil" : (char *)stringValue, numericValue, numericValue, fieldInfo->fieldFlags);
      if (fieldInfo->fieldFlags & COMP_MSG_FIELD_WIFI_DATA) {
        c_strcat(buf, " WIFI_DATA");
      }
      if (fieldInfo->fieldFlags & COMP_MSG_FIELD_MODULE_DATA) {
        c_strcat(buf, " MODULE_DATA");
      }
      if (fieldInfo->fieldFlags & COMP_MSG_FIELD_TO_SAVE) {
        c_strcat(buf, " TO_SAVE");
      }
      if (fieldInfo->fieldFlags & COMP_MSG_FIELD_KEY_VALUE) {
        c_strcat(buf, " KEY_VALUE");
      }
      if (fieldInfo->fieldFlags & COMP_MSG_FIELD_HEADER) {
        c_strcat(buf, " HEADER");
      }
      if (fieldInfo->fieldFlags & COMP_MSG_FIELD_HEADER_UNIQUE) {
        c_strcat(buf, " HEADER_UNIQUE");
      }
      if (fieldInfo->fieldFlags & COMP_MSG_FIELD_HEADER_CHKSUM_NON_ZERO) {
        c_strcat(buf, " HEADER_CHKSUM_NON_ZERO");
      }
    }
    COMP_MSG_DBG(self, "d", 1, "%s", buf);
    idx++;
  }
  return result;
  return result;
}

// ================================= addMsgFieldValues ====================================

static uint8_t addMsgFieldValues(compMsgDispatcher_t *self, uint8_t numEntries) {
  uint8_t result;

  result = COMP_MSG_ERR_OK;
  int idx;
  msgFieldValues_t *msgFieldValues;
  fieldValue_t *fieldValue;

  result = COMP_MSG_ERR_OK;
  msgFieldValues = &self->compMsgDataValue->msgFieldValues;
  if (msgFieldValues->numMsgFields == 0) {       
    msgFieldValues->fieldValues = (fieldValue_t **)os_zalloc(((msgFieldValues->numMsgFields + numEntries) * sizeof(fieldValue_t *)));
  } else {
    msgFieldValues->fieldValues = (fieldValue_t **)os_realloc(msgFieldValues->fieldValues, ((msgFieldValues->numMsgFields + numEntries) * sizeof(fieldValue_t *)));
  }
  checkAllocOK(msgFieldValues->fieldValues);
  idx = msgFieldValues->numMsgFields;
  COMP_MSG_DBG(self, "E", 2, "addMsgFieldValues: numEntries: %d, numMsgFields: %d", numEntries, msgFieldValues->numMsgFields);
  while (idx < msgFieldValues->numMsgFields + numEntries) {
    msgFieldValues->fieldValues[idx] = (fieldValue_t *)NULL;
    idx++;
  }
  msgFieldValues->numMsgFields += numEntries;
  return result;
}

// ================================= setMsgFieldValue ====================================

static uint8_t setMsgFieldValue(compMsgDispatcher_t *self, uint8_t idx, fieldValue_t *fieldValue) {
  uint8_t result;
  msgFieldValues_t *msgFieldValues;
  fieldValue_t *entry;

  result = COMP_MSG_ERR_OK;
  msgFieldValues = &self->compMsgDataValue->msgFieldValues;
  if (idx >= msgFieldValues->numMsgFields) {
    return COMP_MSG_ERR_BAD_MSG_FIELD_INFO_IDX;
  }
  entry = msgFieldValues->fieldValues[idx];
  if (entry == NULL) {
    msgFieldValues->fieldValues[idx] = os_zalloc(sizeof(fieldValue_t));
    checkAllocOK(msgFieldValues->fieldValues[idx]);
    entry = msgFieldValues->fieldValues[idx];
  }
  entry->flags = fieldValue->flags;
  COMP_MSG_DBG(self, "E", 0, "setMsgFieldValues: idx: %d fieldValueFlags: 0x%08x", idx, entry->flags);
  return result;
}

// ================================= getMsgFieldValue ====================================

static uint8_t getMsgFieldValue(compMsgDispatcher_t *self, uint8_t idx, fieldValue_t *fieldValue) {
  uint8_t result;
  msgFieldValues_t *msgFieldValues;
  fieldValue_t *entry;

  result = COMP_MSG_ERR_OK;
  msgFieldValues = &self->compMsgDataValue->msgFieldValues;
  if (idx >= msgFieldValues->numMsgFields) {
    return COMP_MSG_ERR_BAD_MSG_FIELD_INFO_IDX;
  }
  entry = msgFieldValues->fieldValues[idx];
  if (entry == NULL) {
    msgFieldValues->fieldValues[idx] = os_zalloc(sizeof(fieldValue_t));
    checkAllocOK(msgFieldValues->fieldValues[idx]);
    entry = msgFieldValues->fieldValues[idx];
  }
  fieldValue->flags = entry->flags;
  fieldValue->dataValue = entry->dataValue;
  return result;
}

// ================================= compareDataValues ====================================

static uint8_t compareDataValues(compMsgDispatcher_t *self, fieldInfo_t *fieldInfo, dataValue_t *dataValue1, dataValue_t *dataValue2) {
  switch (fieldInfo->fieldTypeId) {
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
  compMsgDataValue->addFieldValueInfo = &addFieldValueInfo;
  compMsgDataValue->setFieldValueInfo = &setFieldValueInfo;
  compMsgDataValue->getFieldValueInfo = &getFieldValueInfo;
  compMsgDataValue->dumpMsgFieldValues = &dumpMsgFieldValues;
  compMsgDataValue->addMsgFieldValues = &addMsgFieldValues;
  compMsgDataValue->getMsgFieldValue = &getMsgFieldValue;
  compMsgDataValue->setMsgFieldValue = &setMsgFieldValue;
  compMsgDataValue->compareDataValues = &compareDataValues;
  // +1 as the following entries also start at 1!!
  compMsgDataValue->addMsgFieldValues(self, self->compMsgTypesAndNames->numSpecFieldIds + 1);  
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
