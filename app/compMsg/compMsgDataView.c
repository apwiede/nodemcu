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
 * File:   compMsgDataView.c
 * Author: Arnulf P. Wiedemann
 *
 * Created on September 24, 2016
 */

#include "osapi.h"
#include "c_types.h"
#include "mem.h"
#include "c_string.h"
#include "c_stdlib.h"
#include "compMsgDispatcher.h"


typedef struct fieldName2id {
  uint8_t *fieldName;
  uint8_t fieldNameId;
  uint8_t refCnt;
} fieldName2id_t;

typedef struct fieldNames
{
  size_t numNames;
  size_t maxNames;
  fieldName2id_t *names;
} fieldNames_t;

static uint8_t compMsgDataViewId;
static fieldNames_t fieldNames = {0, 0, NULL};

static str2id_t specialFieldNames[] = {
  {"@src",                COMP_MSG_SPEC_FIELD_SRC},
  {"@dst",                COMP_MSG_SPEC_FIELD_DST},
  {"@targetCmd",          COMP_MSG_SPEC_FIELD_TARGET_CMD},
  {"@totalLgth",          COMP_MSG_SPEC_FIELD_TOTAL_LGTH},
  {"@cmdKey",             COMP_MSG_SPEC_FIELD_CMD_KEY},
  {"@cmdLgth",            COMP_MSG_SPEC_FIELD_CMD_LGTH},
  {"@randomNum",          COMP_MSG_SPEC_FIELD_RANDOM_NUM},
  {"@sequenceNum",        COMP_MSG_SPEC_FIELD_SEQUENCE_NUM},
  {"@filler",             COMP_MSG_SPEC_FIELD_FILLER},
  {"@crc",                COMP_MSG_SPEC_FIELD_CRC},
  {"@id",                 COMP_MSG_SPEC_FIELD_ID},
  {"@tablerows",          COMP_MSG_SPEC_FIELD_TABLE_ROWS},
  {"@tablerowfields",     COMP_MSG_SPEC_FIELD_TABLE_ROW_FIELDS},
  {"@GUID",               COMP_MSG_SPEC_FIELD_GUID},
  {"@numNormFlds",        COMP_MSG_SPEC_FIELD_NUM_NORM_FLDS},
  {"@normFldIds",         COMP_MSG_SPEC_FIELD_NORM_FLD_IDS},
  {"@normFldNamesSize",   COMP_MSG_SPEC_FIELD_NORM_FLD_NAMES_SIZE},
  {"@normFldNames",       COMP_MSG_SPEC_FIELD_NORM_FLD_NAMES},
  {"@definitionsSize",    COMP_MSG_SPEC_FIELD_DEFINITIONS_SIZE},
  {"@definitions",        COMP_MSG_SPEC_FIELD_DEFINITIONS},
  {"@numListMsgs",        COMP_MSG_SPEC_FIELD_NUM_LIST_MSGS},
  {"@listMsgSizes",       COMP_MSG_SPEC_FIELD_LIST_MSG_SIZES},
  {"@listMsgs",           COMP_MSG_SPEC_FIELD_LIST_MSGS},
  {"@srcId",              COMP_MSG_SPEC_FIELD_SRC_ID},
  {"@hdrFiller",          COMP_MSG_SPEC_FIELD_HDR_FILLER},
  {"@numKeyValues",       COMP_MSG_SPEC_FIELD_NUM_KEY_VALUES},
  {"@provisioningSsid",   COMP_MSG_SPEC_FIELD_PROVISIONING_SSID},
  {"@provisioningPort",   COMP_MSG_SPEC_FIELD_PROVISIONING_PORT},
  {"@provisioningIPAddr", COMP_MSG_SPEC_FIELD_PROVISIONING_IP_ADDR},
  {"@clientSsid",         COMP_MSG_SPEC_FIELD_CLIENT_SSID},
  {"@clientPasswd",       COMP_MSG_SPEC_FIELD_CLIENT_PASSWD},
  {"@clientIPAddr",       COMP_MSG_SPEC_FIELD_CLIENT_IP_ADDR},
  {"@clientPort",         COMP_MSG_SPEC_FIELD_CLIENT_PORT},
  {"@cloudDomain",        COMP_MSG_SPEC_FIELD_CLOUD_DOMAIN},
  {"@cloudPort",          COMP_MSG_SPEC_FIELD_CLOUD_PORT},
  {"@cloudHost1",         COMP_MSG_SPEC_FIELD_CLOUD_HOST_1},
  {"@cloudHost2",         COMP_MSG_SPEC_FIELD_CLOUD_HOST_2},
  {"@cloudSecureConnect", COMP_MSG_SPEC_FIELD_CLOUD_SECURE_CONNECT},
  {"@cloudSubUrl",        COMP_MSG_SPEC_FIELD_CLOUD_SUB_URL},
  {"@cloudNodeToken",     COMP_MSG_SPEC_FIELD_CLOUD_NODE_TOKEN},

  {NULL, -1},
};

static int sequenceNum = 0;

// ================================= getFieldNameIdFromStr ====================================

static uint8_t getFieldNameIdFromStr(compMsgDataView_t *self, const uint8_t *fieldName, uint8_t *fieldNameId, uint8_t incrRefCnt) {
  int firstFreeEntryId;
  int nameIdx;
  fieldName2id_t fieldNameEntry;
  str2id_t *entry;
  fieldName2id_t *newFieldNameEntry;
  fieldName2id_t *nameEntry;
  fieldName2id_t *firstFreeEntry;

//ets_printf("getFieldNameIdFromStr: %p\n", fieldName);
  if (fieldName[0] == '@') {
    // find special field name
    entry = &specialFieldNames[0];
    while (entry->str != NULL) {
      if (entry->str == NULL) {
        break;
      }
      if (c_strcmp(entry->str, fieldName) == 0) {
        *fieldNameId = entry->id;
        return COMP_MSG_ERR_OK;
      }
      entry++;
    }
    return COMP_MSG_ERR_BAD_SPECIAL_FIELD;
  } else {
    if ((incrRefCnt == COMP_MSG_INCR) && (fieldNames.numNames >= fieldNames.maxNames)) {
      if (fieldNames.maxNames == 0) {
        fieldNames.maxNames = 4;
        fieldNames.names = (fieldName2id_t *)os_zalloc((fieldNames.maxNames * sizeof(fieldName2id_t)));
        checkAllocOK(fieldNames.names);
      } else {
        fieldNames.maxNames += 2;
        fieldNames.names = (fieldName2id_t *)os_realloc((fieldNames.names), (fieldNames.maxNames * sizeof(fieldName2id_t)));
        checkAllocOK(fieldNames.names);
      }
    }
    firstFreeEntry = NULL;
    firstFreeEntryId = 0;
    if (fieldNames.numNames > 0) {
      // find field name
      nameIdx = 0;
      while (nameIdx < fieldNames.numNames) {
        nameEntry = &fieldNames.names[nameIdx];
        if ((nameEntry->fieldName != NULL) && (c_strcmp(nameEntry->fieldName, fieldName) == 0)) {
          if (incrRefCnt < 0) {
            if (nameEntry->refCnt > 0) {
              nameEntry->refCnt--;
            }
            if (nameEntry->refCnt == 0) {
              nameEntry->fieldNameId = COMP_MSG_FREE_FIELD_ID;
              os_free(nameEntry->fieldName);
              nameEntry->fieldName = NULL;
            }
          } else {
            if (incrRefCnt > 0) {
              nameEntry->refCnt++;
            } else {
              // just get the entry, do not modify
            }
          }
          *fieldNameId = nameEntry->fieldNameId;
          return COMP_MSG_ERR_OK;
        }
        if ((incrRefCnt == COMP_MSG_INCR) && (nameEntry->fieldNameId == COMP_MSG_FREE_FIELD_ID) && (firstFreeEntry == NULL)) {
          firstFreeEntry = nameEntry;
          firstFreeEntry->fieldNameId = nameIdx + 1;
        }
        nameIdx++;
      }
    }
    if (incrRefCnt < 0) {
      return COMP_MSG_ERR_OK; // just ignore silently
    } else {
      if (incrRefCnt == 0) {
        return COMP_MSG_ERR_FIELD_NOT_FOUND;
      } else {
        if (firstFreeEntry != NULL) {
          *fieldNameId = firstFreeEntry->fieldNameId;
          firstFreeEntry->refCnt = 1;
          firstFreeEntry->fieldName = os_malloc(c_strlen(fieldName) + 1);
          firstFreeEntry->fieldName[c_strlen(fieldName)] = '\0';
          c_memcpy(firstFreeEntry->fieldName, fieldName, c_strlen(fieldName));
        } else {
          newFieldNameEntry = &fieldNames.names[fieldNames.numNames];
          newFieldNameEntry->refCnt = 1;
          newFieldNameEntry->fieldNameId = fieldNames.numNames + 1;
          newFieldNameEntry->fieldName = os_zalloc(c_strlen(fieldName) + 1);
          newFieldNameEntry->fieldName[c_strlen(fieldName)] = '\0';
          c_memcpy(newFieldNameEntry->fieldName, fieldName, c_strlen(fieldName));
          fieldNames.numNames++;
          *fieldNameId = newFieldNameEntry->fieldNameId;
        }
      }
    }
  }
  return DATA_VIEW_ERR_OK;
}

// ================================= getFieldNameStrFromId ====================================

static uint8_t getFieldNameStrFromId(compMsgDataView_t *self, uint8_t fieldNameId, uint8_t **fieldName) {
  str2id_t *entry;
  fieldName2id_t *fieldNameEntry;

  *fieldName = NULL;
  // first try to find special field name
  entry = &specialFieldNames[0];
  while (entry->str != NULL) {
    if (entry->id == fieldNameId) {
      *fieldName = entry->str;
      return COMP_MSG_ERR_OK;
    }
    entry++;
  }
  // find field name
  int idx = 0;

  while (idx < fieldNames.numNames) {
    fieldNameEntry = &fieldNames.names[idx];
    if (fieldNameEntry->fieldNameId == fieldNameId) {
      *fieldName = fieldNameEntry->fieldName;
      return COMP_MSG_ERR_OK;
    }
    fieldNameEntry++;
    idx++;
  }
  return COMP_MSG_ERR_FIELD_NOT_FOUND;
}

// ================================= getRandomNum ====================================

static uint8_t getRandomNum(compMsgDataView_t *self, compMsgField_t *fieldInfo, uint32_t *value) {
  return self->dataView->getUint32(self->dataView, fieldInfo->fieldOffset, value);
}

// ================================= setRandomNum ====================================

static uint8_t setRandomNum(compMsgDataView_t *self, compMsgField_t *fieldInfo) {
  uint32_t val;

  val = (uint32_t)(rand() & RAND_MAX);
  return self->dataView->setUint32(self->dataView, fieldInfo->fieldOffset, val);
}


// ================================= getSequenceNum ====================================

static uint8_t getSequenceNum(compMsgDataView_t *self, compMsgField_t *fieldInfo, uint32_t *value) {
  return self->dataView->getUint32(self->dataView, fieldInfo->fieldOffset, value);
}

// ================================= setSequenceNum ====================================

static uint8_t setSequenceNum(compMsgDataView_t *self, compMsgField_t *fieldInfo) {
  return self->dataView->setUint32(self->dataView, fieldInfo->fieldOffset, sequenceNum++);
}


// ================================= getFiller ====================================

static uint8_t getFiller(compMsgDataView_t *self, compMsgField_t *fieldInfo, uint8_t **value) {
  size_t lgth;

  lgth = fieldInfo->fieldLgth;
  if (fieldInfo->fieldOffset + lgth > self->dataView->lgth) {
    return DATA_VIEW_ERR_OUT_OF_RANGE;
  }
  c_memcpy(*value,self->dataView->data+fieldInfo->fieldOffset,lgth);
}

// ================================= setFiller ====================================

static uint8_t setFiller(compMsgDataView_t *self, compMsgField_t *fieldInfo) {
  uint32_t val;
  int idx;
  int lgth;
  int result;
  size_t offset;

  lgth = fieldInfo->fieldLgth;
  offset = fieldInfo->fieldOffset;
  idx = 0;
  while (lgth >= 4) {
    val = (uint32_t)(rand() & RAND_MAX);
    result = self->dataView->setUint32(self->dataView, offset, val);
    checkErrOK(result);
    offset += 4;
    lgth -= 4;
  }
  while (lgth >= 2) {
    val = (uint16_t)((rand() & RAND_MAX) & 0xFFFF);
    result = self->dataView->setUint16(self->dataView, offset, val);
    checkErrOK(result);
    offset += 2;
    lgth -= 2;
  }
  while (lgth >= 1) {
    val = (uint8_t)((rand() & RAND_MAX) & 0xFF);
    result = self->dataView->setUint8(self->dataView, offset, val);
    checkErrOK(result);
    offset++;
    lgth -= 1;
  }
  return DATA_VIEW_ERR_OK;
}


// ================================= getCrc ====================================

static uint8_t getCrc(compMsgDataView_t *self, compMsgField_t *fieldInfo, size_t startOffset, size_t lgth) {
  uint16_t crcVal;
  uint16_t crc;
  uint8_t uint8_crc;
  int crcLgth;
  int idx;
  int result;

  crcLgth = fieldInfo->fieldLgth;
  crcVal = 0;
  idx = startOffset;
  while (idx < lgth) {
//ets_printf("crc idx: %d ch: 0x%02x crc: 0x%04x\n", idx-startOffset, self->dataView->data[idx], crcVal);
    crcVal += self->dataView->data[idx++];
  }
//ets_printf("§crcVal00: 0x%04x§\n", crcVal);
  crcVal = ~(crcVal);
  if (crcLgth == 1) {
//ets_printf("§crcVal10: 0x%04x§\n", crcVal);
    crcVal = crcVal & 0xFF;
    result = self->dataView->getUint8(self->dataView, fieldInfo->fieldOffset, &uint8_crc);
    checkErrOK(result);
ets_printf("§crcVal1: 0x%02x crc: 0x%02x§\n", crcVal, uint8_crc);
    if (crcVal != uint8_crc) {
      return COMP_MSG_ERR_BAD_CRC_VALUE;
    }
  } else {
    result = self->dataView->getUint16(self->dataView, fieldInfo->fieldOffset, &crc);
    checkErrOK(result);
ets_printf("§crcVal2: 0x%04x crc: 0x%04x§\n", crcVal, crc);
    if (crcVal != crc) {
      return COMP_MSG_ERR_BAD_CRC_VALUE;
    }
  }
  return DATA_VIEW_ERR_OK;
}

// ================================= setCrc ====================================

static uint8_t setCrc(compMsgDataView_t *self, compMsgField_t *fieldInfo, size_t startOffset, size_t lgth) {
  int idx;
  uint16_t crc;

  crc = 0;
  idx = startOffset;
//ets_printf("§crc idx: %d ch: 0x%02x crc: 0x%04x\n§", idx-startOffset, self->dataView->data[idx], crc);
  while (idx < lgth) {
//ets_printf("§crc idx: %d ch: 0x%02x crc: 0x%04x\n§", idx-startOffset, self->dataView->data[idx], crc);
    crc += self->dataView->data[idx++];
  }
  crc = ~(crc);
  if (fieldInfo->fieldLgth == 1) {
//ets_printf("§crc8: 0x%04x 0x%02x\n§", crc, (uint8_t)(crc & 0xFF));
    self->dataView->setUint8(self->dataView,fieldInfo->fieldOffset,(uint8_t)(crc & 0xFF));
  } else {
    self->dataView->setUint16(self->dataView,fieldInfo->fieldOffset,crc);
  }
//ets_printf("crc: 0x%04x\n", crc);
  return DATA_VIEW_ERR_OK;
}


// ================================= getFieldValue ====================================

static uint8_t getFieldValue(compMsgDataView_t *self, compMsgField_t *fieldInfo, int *numericValue, uint8_t **stringValue, int fieldIdx) {
  int idx;
  int result;
  int numEntries;
  int8_t i8;
  uint8_t ui8;
  int16_t i16;
  uint16_t ui16;
  int32_t i32;
  uint32_t ui32;

  *stringValue = NULL;
  *numericValue = 0;
  switch (fieldInfo->fieldTypeId) {
    case DATA_VIEW_FIELD_INT8_T:
      result = self->dataView->getInt8(self->dataView, fieldInfo->fieldOffset, &i8);
      checkErrOK(result);
      *numericValue = (int)i8;
      break;
    case DATA_VIEW_FIELD_UINT8_T:
      result = self->dataView->getUint8(self->dataView, fieldInfo->fieldOffset, &ui8);
      checkErrOK(result);
      *numericValue = (int)ui8;
      break;
    case DATA_VIEW_FIELD_INT16_T:
      result = self->dataView->getInt16(self->dataView, fieldInfo->fieldOffset, &i16);
      checkErrOK(result);
      *numericValue = (int)i16;
      break;
    case DATA_VIEW_FIELD_UINT16_T:
      result = self->dataView->getUint16(self->dataView, fieldInfo->fieldOffset, &ui16);
      checkErrOK(result);
      *numericValue = (int)ui16;
      break;
    case DATA_VIEW_FIELD_INT32_T:
      result = self->dataView->getInt32(self->dataView, fieldInfo->fieldOffset, &i32);
      checkErrOK(result);
      *numericValue = (int)i32;
      break;
    case DATA_VIEW_FIELD_UINT32_T:
      result = self->dataView->getUint32(self->dataView, fieldInfo->fieldOffset, &ui32);
      checkErrOK(result);
      *numericValue = (int)ui32;
      break;
    case DATA_VIEW_FIELD_INT8_VECTOR:
      *numericValue = fieldInfo->fieldLgth;
      *stringValue = os_zalloc(fieldInfo->fieldLgth+1);
      checkAllocOK(stringValue);
      (*stringValue)[fieldInfo->fieldLgth] = 0;
      os_memcpy(*stringValue, self->dataView->data+fieldInfo->fieldOffset, fieldInfo->fieldLgth);
      break;
    case DATA_VIEW_FIELD_UINT8_VECTOR:
      *stringValue = os_zalloc(fieldInfo->fieldLgth+1);
      checkAllocOK(stringValue);
      (*stringValue)[fieldInfo->fieldLgth] = 0;
      os_memcpy(*stringValue, self->dataView->data+fieldInfo->fieldOffset, fieldInfo->fieldLgth);
      break;
    case DATA_VIEW_FIELD_INT16_VECTOR:
      if (*stringValue == NULL) {
        // check for length needed!!
        result = self->dataView->getInt16(self->dataView, fieldInfo->fieldOffset+fieldIdx*sizeof(int16_t), &i16);
        checkErrOK(result);
        *numericValue = (int)i16;
      } else {
        return COMP_MSG_ERR_BAD_VALUE;
      }
      break;
    case DATA_VIEW_FIELD_UINT16_VECTOR:
      if (*stringValue == NULL) {
        // check for length needed!!
        result = self->dataView->getUint16(self->dataView, fieldInfo->fieldOffset+fieldIdx*sizeof(uint16_t), &ui16);
        checkErrOK(result);
        *numericValue = (int)ui16;
      } else {
        return COMP_MSG_ERR_BAD_VALUE;
      }
      break;
#ifdef NOTDEF
    case DATA_VIEW_FIELD_INT32_VECTOR:
      if (stringValue != NULL) {
        // check for length needed!!
        os_memcpy((int8_t *)fieldInfo->value.int32Vector, stringValue, fieldInfo->fieldLgth);
      } else {
        return COMP_MSG_ERR_BAD_VALUE;
      }
      break;
    case DATA_VIEW_FIELD_UINT32_VECTOR:
      if (stringValue != NULL) {
        // check for length needed!!
        os_memcpy((uint8_t *)fieldInfo->value.uint32Vector, stringValue, fieldInfo->fieldLgth);
      } else {
        return COMP_MSG_ERR_BAD_VALUE;
      }
      break;
#endif
    default:
      return COMP_MSG_ERR_BAD_FIELD_TYPE;
      break;
  }
  return DATA_VIEW_ERR_OK;
}

// ================================= setFieldValue ====================================

static uint8_t setFieldValue(compMsgDataView_t *self, compMsgField_t *fieldInfo, int numericValue, const uint8_t *stringValue, int fieldIdx) {
  int idx;
  int result;
  int numEntries;

  switch (fieldInfo->fieldTypeId) {
    case DATA_VIEW_FIELD_INT8_T:
      if (stringValue == NULL) {
        if ((numericValue > -128) && (numericValue < 128)) {
          result= self->dataView->setInt8(self->dataView, fieldInfo->fieldOffset, (int8_t)numericValue);
          checkErrOK(result);
        } else {
          return COMP_MSG_ERR_VALUE_TOO_BIG;
        }
      } else {
        return COMP_MSG_ERR_BAD_VALUE;
      }
      break;
    case DATA_VIEW_FIELD_UINT8_T:
      if (stringValue == NULL) {
        if ((numericValue >= 0) && (numericValue <= 256)) {
          result= self->dataView->setUint8(self->dataView, fieldInfo->fieldOffset, (uint8_t)numericValue);
          checkErrOK(result);
        }
      } else {
        return COMP_MSG_ERR_BAD_VALUE;
      }
      break;
    case DATA_VIEW_FIELD_INT16_T:
      if (stringValue == NULL) {
        if ((numericValue > -32767) && (numericValue < 32767)) {
          result= self->dataView->setInt16(self->dataView, fieldInfo->fieldOffset, (int16_t)numericValue);
          checkErrOK(result);
        } else {
          return COMP_MSG_ERR_VALUE_TOO_BIG;
        }
      } else {
        return COMP_MSG_ERR_BAD_VALUE;
      }
      break;
    case DATA_VIEW_FIELD_UINT16_T:
      if (stringValue == NULL) {
        if ((numericValue >= 0) && (numericValue <= 65535)) {
          result= self->dataView->setUint16(self->dataView, fieldInfo->fieldOffset, (uint16_t)numericValue);
          checkErrOK(result);
        } else {
          return COMP_MSG_ERR_VALUE_TOO_BIG;
        }
      } else {
        return COMP_MSG_ERR_BAD_VALUE;
      }
      break;
    case DATA_VIEW_FIELD_INT32_T:
      if (stringValue == NULL) {
        if ((numericValue > -0x7FFFFFFF) && (numericValue <= 0x7FFFFFFF)) {
          result= self->dataView->setInt32(self->dataView, fieldInfo->fieldOffset, (int32_t)numericValue);
          checkErrOK(result);
        } else {
          return COMP_MSG_ERR_VALUE_TOO_BIG;
        }
      } else {
        return COMP_MSG_ERR_BAD_VALUE;
      }
      break;
    case DATA_VIEW_FIELD_UINT32_T:
      if (stringValue == NULL) {
        // we have to do the signed check as numericValue is a signed integer!!
        if ((numericValue > -0x7FFFFFFF) && (numericValue <= 0x7FFFFFFF)) {
          result= self->dataView->setUint32(self->dataView, fieldInfo->fieldOffset, (uint32_t)numericValue);
          checkErrOK(result);
        } else {
          return COMP_MSG_ERR_VALUE_TOO_BIG;
        }
      } else {
        return COMP_MSG_ERR_BAD_VALUE;
      }
      break;
    case DATA_VIEW_FIELD_INT8_VECTOR:
      if (stringValue != NULL) {
        // check for length needed!!
        if (fieldInfo->fieldOffset + fieldInfo->fieldLgth > self->dataView->lgth) {
          return COMP_MSG_ERR_VALUE_TOO_BIG;
        }
        os_memcpy(self->dataView->data+fieldInfo->fieldOffset, stringValue, fieldInfo->fieldLgth);
      } else {
        return COMP_MSG_ERR_BAD_VALUE;
      }
      break;
    case DATA_VIEW_FIELD_UINT8_VECTOR:
      if (stringValue != NULL) {
        // check for length needed!!
        if (fieldInfo->fieldOffset + fieldInfo->fieldLgth > self->dataView->lgth) {
          return COMP_MSG_ERR_VALUE_TOO_BIG;
        }
        os_memcpy(self->dataView->data+fieldInfo->fieldOffset, stringValue, fieldInfo->fieldLgth);
      } else {
        return COMP_MSG_ERR_BAD_VALUE;
      }
      break;
    case DATA_VIEW_FIELD_INT16_VECTOR:
      if (stringValue == NULL) {
        // check for length needed!!
        result= self->dataView->setInt16(self->dataView, fieldInfo->fieldOffset+fieldIdx*sizeof(int16_t), (int16_t)numericValue);
        checkErrOK(result);
      } else {
        return COMP_MSG_ERR_BAD_VALUE;
      }
      break;
    case DATA_VIEW_FIELD_UINT16_VECTOR:
      if (stringValue == NULL) {
        // check for length needed!!
        result= self->dataView->setUint16(self->dataView, fieldInfo->fieldOffset+fieldIdx*sizeof(uint16_t), (uint16_t)numericValue);
        checkErrOK(result);
      } else {
        return COMP_MSG_ERR_BAD_VALUE;
      }
      break;
#ifdef NOTDEF
    case DATA_VIEW_FIELD_INT32_VECTOR:
      if (stringValue != NULL) {
        // check for length needed!!
        os_memcpy((int8_t *)fieldInfo->value.int32Vector, stringValue, fieldInfo->fieldLgth);
      } else {
        return COMP_MSG_ERR_BAD_VALUE;
      }
      break;
    case DATA_VIEW_FIELD_UINT32_VECTOR:
      if (stringValue != NULL) {
        // check for length needed!!
        os_memcpy((uint8_t *)fieldInfo->value.uint32Vector, stringValue, fieldInfo->fieldLgth);
      } else {
        return COMP_MSG_ERR_BAD_VALUE;
      }
      break;
#endif
    default:
ets_printf("bad type in setFieldValue. %d\n", fieldInfo->fieldTypeId);
      return COMP_MSG_ERR_BAD_FIELD_TYPE;
      break;
  }
  return DATA_VIEW_ERR_OK;
}


// ================================= newCompMsgDataView ====================================

compMsgDataView_t *newCompMsgDataView(void) {
  compMsgDataView_t *compMsgDataView = os_zalloc(sizeof(compMsgDataView_t));
  if (compMsgDataView == NULL) {
    return NULL;
  }
  compMsgDataView->dataView = newDataView();
  if (compMsgDataView->dataView == NULL) {
    return NULL;
  }
  compMsgDataViewId++;
  compMsgDataView->id = compMsgDataViewId;

  compMsgDataView->getFieldNameIdFromStr = &getFieldNameIdFromStr;
  compMsgDataView->getFieldNameStrFromId = &getFieldNameStrFromId;

  compMsgDataView->getRandomNum = &getRandomNum;
  compMsgDataView->setRandomNum = &setRandomNum;

  compMsgDataView->getSequenceNum = &getSequenceNum;
  compMsgDataView->setSequenceNum = &setSequenceNum;

  compMsgDataView->getFiller = &getFiller;
  compMsgDataView->setFiller = &setFiller;

  compMsgDataView->getCrc = &getCrc;
  compMsgDataView->setCrc = &setCrc;

  compMsgDataView->getFieldValue = &getFieldValue;
  compMsgDataView->setFieldValue = &setFieldValue;

  return compMsgDataView;
}

// ================================= freeCompMsgDataView ====================================

void freeCompMsgDataView(compMsgDataView_t *dataView) {
  if (dataView->dataView != NULL) {
    freeDataView(dataView->dataView);
    dataView->dataView = NULL;
    os_free(dataView);
  }
}

