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

#include "compMsgDispatcher.h"

const static str2id_t dataValueStr2ValueIds [] = {
  { "@provisioningSsid",    COMP_MSG_WIFI_VALUE_ID_provisioningSsid},
  { "@provisioningPort",    COMP_MSG_WIFI_VALUE_ID_provisioningPort},
  { "@provisioningIPAddr",  COMP_MSG_WIFI_VALUE_ID_provisioningIPAddr},
  { "@SSDPIPAddr",          COMP_MSG_WIFI_VALUE_ID_SSDPIPAddr},
  { "@SSDPPort",            COMP_MSG_WIFI_VALUE_ID_SSDPPort},
  { "@clientIPAddr",        COMP_MSG_WIFI_VALUE_ID_clientIPAddr},
  { "@clientPort",          COMP_MSG_WIFI_VALUE_ID_clientPort},
  { "@clientSequenceNum",   COMP_MSG_WIFI_VALUE_ID_clientSequenceNum},
  { "Reserve1",             COMP_MSG_MODULE_VALUE_ID_Reserve1},
  { "Reserve2",             COMP_MSG_MODULE_VALUE_ID_Reserve2},
  { "Reserve3",             COMP_MSG_MODULE_VALUE_ID_Reserve3},
  { "Reserve4",             COMP_MSG_MODULE_VALUE_ID_Reserve4},
  { "Reserve5",             COMP_MSG_MODULE_VALUE_ID_Reserve5},
  { "Reserve6",             COMP_MSG_MODULE_VALUE_ID_Reserve6},
  { "Reserve7",             COMP_MSG_MODULE_VALUE_ID_Reserve7},
  { "Reserve8",             COMP_MSG_MODULE_VALUE_ID_Reserve8},
  { "cryptKey",             COMP_MSG_MODULE_VALUE_ID_cryptKey},
  { "cryptIvKey",           COMP_MSG_MODULE_VALUE_ID_cryptIvKey},
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

static uint8_t addDataValue(compMsgDispatcher_t *self, uint8_t valueId, fieldValueCallback_t fieldValueCallback, int numericValue, uint8_t *stringValue) {
  uint8_t result;
  compMsgDataValue_t *compMsgDataValue;
  dataValue_t *dataValue;

  result = COMP_MSG_ERR_OK;
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
  dataValue = &compMsgDataValue->dataValues[compMsgDataValue->numDataValues];
  memset(dataValue, 0, sizeof(dataValue_t));
  dataValue->valueId = valueId;
  dataValue->flags = 0;
  if (stringValue == NULL) {
    dataValue->flags |= COMP_MSG_FIELD_IS_NUMERIC;
    dataValue->value.numericValue = numericValue;
  } else {
    dataValue->flags |= COMP_MSG_FIELD_IS_STRING;
    dataValue->value.stringValue = os_zalloc(c_strlen(stringValue) + 1);
    c_memcpy(dataValue->value.stringValue, stringValue, c_strlen(stringValue));
  }
  if (fieldValueCallback != NULL) {
    dataValue->flags |= COMP_MSG_FIELD_HAS_CALLBACK;
    dataValue->fieldValueCallback = fieldValueCallback;
  }
  compMsgDataValue->numDataValues++;
  return result;
}

// ================================= setDataValue ====================================

static uint8_t setDataValue(compMsgDispatcher_t *self, uint8_t valueId, fieldValueCallback_t fieldValueCallback, int numericValue, uint8_t *stringValue) {
  uint8_t result;
  int idx;
  compMsgDataValue_t *compMsgDataValue;
  dataValue_t *dataValue;

  result = COMP_MSG_ERR_OK;
  compMsgDataValue = self->compMsgDataValue;
  idx = 0;
  while (idx < compMsgDataValue->numDataValues) {
    dataValue = &compMsgDataValue->dataValues[idx];
    if (dataValue->valueId == valueId) {
      dataValue->valueId = valueId;
      if ((dataValue->flags & COMP_MSG_FIELD_IS_STRING) && (dataValue->value.stringValue != NULL)) {
        os_free(dataValue->value.stringValue);
      }
      dataValue->flags = 0;
      if (stringValue == NULL) {
        dataValue->flags |= COMP_MSG_FIELD_IS_NUMERIC;
        dataValue->value.numericValue = numericValue;
      } else {
        dataValue->flags |= COMP_MSG_FIELD_IS_STRING;
        dataValue->value.stringValue = os_zalloc(c_strlen(stringValue) + 1);
        c_memcpy(dataValue->value.stringValue, stringValue, c_strlen(stringValue));
      }
      if (fieldValueCallback != NULL) {
        dataValue->flags |= COMP_MSG_FIELD_HAS_CALLBACK;
        dataValue->fieldValueCallback = fieldValueCallback;
      }
      return result;
    }
    idx++;
  }
  return COMP_MSG_ERR_DATA_VALUE_FIELD_NOT_FOUND;
}

// ================================= getDataValue ====================================

static uint8_t getDataValue(compMsgDispatcher_t *self, uint8_t valueId, uint8_t *flags, fieldValueCallback_t *fieldValueCallback, int *numericValue, uint8_t **stringValue) {
  uint8_t result;
  compMsgDataValue_t *compMsgDataValue;
  dataValue_t *dataValue;
  uint8_t *valueStr;
  int idx;

  result = COMP_MSG_ERR_OK;
  compMsgDataValue = self->compMsgDataValue;
  *numericValue = 0;
  *stringValue = NULL;
  *flags = 0;
  *fieldValueCallback = NULL;
  idx = 0;
  while (idx < compMsgDataValue->numDataValues) {
    dataValue = &compMsgDataValue->dataValues[idx];
    if (dataValue->valueId == valueId) {
      result = compMsgDataValue->dataValueId2ValueStr(self, valueId, &valueStr);
      checkErrOK(result);
      *flags = dataValue->flags;
      if (dataValue->flags & COMP_MSG_FIELD_IS_STRING) {
        *stringValue = dataValue->value.stringValue;
        COMP_MSG_DBG(self, "E", 1, "getDataValue: %s %s", valueStr, *stringValue);
      }
      if (dataValue->flags & COMP_MSG_FIELD_IS_NUMERIC) {
        *numericValue = dataValue->value.numericValue;
        COMP_MSG_DBG(self, "E", 1, "getDataValue: %s 0x%08x %d", valueStr, *numericValue, *numericValue);
      }
      *fieldValueCallback = dataValue->fieldValueCallback;
      return result;
    }
    idx++;
  }
  COMP_MSG_DBG(self, "E", 1, "getDataValue: DATA_VALUE_FIELD_NOT_FOUND");
  return COMP_MSG_ERR_DATA_VALUE_FIELD_NOT_FOUND;
}

// ================================= compMsgDataValueInit ====================================

static uint8_t compMsgDataValueInit(compMsgDispatcher_t *self) {
  uint8_t result;
  compMsgDataValue_t *compMsgDataValue;

  compMsgDataValue = self->compMsgDataValue;
  compMsgDataValue->dataValueStr2ValueId = &dataValueStr2ValueId;
  compMsgDataValue->dataValueId2ValueStr = &dataValueId2ValueStr;
  compMsgDataValue->addDataValue = &addDataValue;
  compMsgDataValue->setDataValue = &setDataValue;
  compMsgDataValue->getDataValue = &getDataValue;
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
