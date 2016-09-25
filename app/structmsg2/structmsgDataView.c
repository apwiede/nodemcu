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
 * File:   structmsgDataView.c
 * Author: Arnulf P. Wiedemann
 *
 * Created on September 24, 2016
 */

#include "c_types.h"
#include "mem.h"
#include "c_string.h"
#include "structmsgDataView.h"


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

static fieldNames_t fieldNames = {0, 0, NULL};

static str2id_t specialFieldNames[] = {
  {"@src",            STRUCT_MSG_SPEC_FIELD_SRC},
  {"@dst",            STRUCT_MSG_SPEC_FIELD_DST},
  {"@targetCmd",      STRUCT_MSG_SPEC_FIELD_TARGET_CMD},
  {"@totalLgth",      STRUCT_MSG_SPEC_FIELD_TOTAL_LGTH},
  {"@cmdKey",         STRUCT_MSG_SPEC_FIELD_CMD_KEY},
  {"@cmdLgth",        STRUCT_MSG_SPEC_FIELD_CMD_LGTH},
  {"@randomNum",      STRUCT_MSG_SPEC_FIELD_RANDOM_NUM},
  {"@sequenceNum",    STRUCT_MSG_SPEC_FIELD_SEQUENCE_NUM},
  {"@filler",         STRUCT_MSG_SPEC_FIELD_FILLER},
  {"@crc",            STRUCT_MSG_SPEC_FIELD_CRC},
  {"@id",             STRUCT_MSG_SPEC_FIELD_ID},
  {"@tablerows",      STRUCT_MSG_SPEC_FIELD_TABLE_ROWS},
  {"@tablerowfields", STRUCT_MSG_SPEC_FIELD_TABLE_ROW_FIELDS},
  {NULL, -1},
};

// ================================= getFieldNameIdFromStr ====================================

static uint8_t getFieldNameIdFromStr(structmsgDataView_t *self, const uint8_t *fieldName, uint8_t *fieldNameId, uint8_t incrRefCnt) {
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
        return STRUCT_MSG_ERR_OK;
      }
      entry++;
    }
    return STRUCT_MSG_ERR_BAD_SPECIAL_FIELD;
  } else {
    if ((incrRefCnt == STRUCT_MSG_INCR) && (fieldNames.numNames >= fieldNames.maxNames)) {
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
              nameEntry->fieldNameId = STRUCT_MSG_FREE_FIELD_ID;
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
          return STRUCT_MSG_ERR_OK;
        }
        if ((incrRefCnt == STRUCT_MSG_INCR) && (nameEntry->fieldNameId == STRUCT_MSG_FREE_FIELD_ID) && (firstFreeEntry == NULL)) {
          firstFreeEntry = nameEntry;
          firstFreeEntry->fieldNameId = nameIdx + 1;
        }
        nameIdx++;
      }
    }
    if (incrRefCnt < 0) {
      return STRUCT_MSG_ERR_OK; // just ignore silently
    } else {
      if (incrRefCnt == 0) {
        return STRUCT_MSG_ERR_FIELD_NOT_FOUND;
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
          newFieldNameEntry->fieldName = os_malloc(c_strlen(fieldName) + 1);
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

static uint8_t getFieldNameStrFromId(structmsgDataView_t *self, uint8_t fieldNameId, uint8_t **fieldName) {
  str2id_t *entry;
  fieldName2id_t *fieldNameEntry;

  *fieldName = NULL;
  // first try to find special field name
  entry = &specialFieldNames[0];
  while (entry->str != NULL) {
    if (entry->id == fieldNameId) {
      *fieldName = entry->str;
      return STRUCT_MSG_ERR_OK;
    }
    entry++;
  }
  // find field name
  int idx = 0;

  while (idx < fieldNames.numNames) {
    fieldNameEntry = &fieldNames.names[idx];
    if (fieldNameEntry->fieldNameId == fieldNameId) {
      *fieldName = fieldNameEntry->fieldName;
      return STRUCT_MSG_ERR_OK;
    }
    fieldNameEntry++;
    idx++;
  }
  return STRUCT_MSG_ERR_FIELD_NOT_FOUND;
}

// ================================= getRandomNum ====================================

static uint8_t getRandomNum(structmsgDataView_t *self, int offset, uint32_t *value) {
  return DATA_VIEW_ERR_OK;
}

// ================================= setRandomNum ====================================

static uint8_t setRandomNum(structmsgDataView_t *self, int offset, uint32_t value) {
  return DATA_VIEW_ERR_OK;
}


// ================================= getSequenceNum ====================================

static uint8_t getSequenceNum(structmsgDataView_t *self, int offset, uint32_t *value) {
  return DATA_VIEW_ERR_OK;
}

// ================================= setSequenceNum ====================================

static uint8_t setSequenceNum(structmsgDataView_t *self, int offset, uint32_t value) {
  return DATA_VIEW_ERR_OK;
}


// ================================= getFiller ====================================

static uint8_t getFiller(structmsgDataView_t *self, int offset, uint32_t **value, size_t lgth) {
  return DATA_VIEW_ERR_OK;
}

// ================================= setFiller ====================================

static uint8_t setFiller(structmsgDataView_t *self, int offset, uint32_t *value, size_t lgth) {
  return DATA_VIEW_ERR_OK;
}


// ================================= getCrc ====================================

static uint8_t getCrc(structmsgDataView_t *self, int offset, uint32_t *value, int startOffset) {
  return DATA_VIEW_ERR_OK;
}

// ================================= setCrc ====================================

static uint8_t setCrc(structmsgDataView_t *self, int offset, int startOffset) {
  return DATA_VIEW_ERR_OK;
}


// ================================= getFieldValue ====================================

static uint8_t getFieldValue(structmsgDataView_t *self, uint8_t fieldId, void *value) {
  return DATA_VIEW_ERR_OK;
}

// ================================= setFieldValue ====================================

static uint8_t setFieldValue(structmsgDataView_t *self, uint8_t fieldId, void *value) {
  return DATA_VIEW_ERR_OK;
}


// ================================= getTableFieldValue ====================================

static uint8_t getTableFieldValue(structmsgDataView_t *self, int row, uint8_t fieldId, void *value) {
  return DATA_VIEW_ERR_OK;
}

// ================================= setTableFieldValue ====================================

static uint8_t setTableFieldValue(structmsgDataView_t *self, int row, uint8_t fieldId, void *value) {
  return DATA_VIEW_ERR_OK;
}

// ================================= newStructmsgDataView ====================================

structmsgDataView_t *newStructmsgDataView(void) {
  structmsgDataView_t *structmsgDataView = os_zalloc(sizeof(structmsgDataView_t));
  if (structmsgDataView == NULL) {
    return NULL;
  }
  structmsgDataView->dataView = newDataView();
  if (structmsgDataView->dataView == NULL) {
    return NULL;
  }

ets_printf("newStructmsgDataVidew: structmsgDataView: %p dataView: %p\n", structmsgDataView, structmsgDataView->dataView);
  structmsgDataView->getFieldNameIdFromStr = &getFieldNameIdFromStr;
  structmsgDataView->getFieldNameStrFromId = &getFieldNameStrFromId;

  structmsgDataView->getRandomNum = &getRandomNum;
  structmsgDataView->setRandomNum = &setRandomNum;

  structmsgDataView->getSequenceNum = &getSequenceNum;
  structmsgDataView->setSequenceNum = &setSequenceNum;

  structmsgDataView->getFiller = &getFiller;
  structmsgDataView->setFiller = &setFiller;

  structmsgDataView->getCrc = &getCrc;
  structmsgDataView->setCrc = &setCrc;

  structmsgDataView->getFieldValue = &getFieldValue;
  structmsgDataView->setFieldValue = &setFieldValue;

  structmsgDataView->getTableFieldValue = &getTableFieldValue;
  structmsgDataView->setTableFieldValue = &setTableFieldValue;

  return structmsgDataView;
}

// ================================= freeStructmsgDataView ====================================

void freeStructmsgDataView(structmsgDataView_t *dataView) {
}
