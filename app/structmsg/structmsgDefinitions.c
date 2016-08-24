/*
* Copyright (c) 2016, Arnulf Wiedemann
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
#include "structmsg.h"

static fieldNameDefinitions_t fieldNameDefinitions = {0, 0, NULL};
static stmsgDefinitions_t structmsgDefinitions = {0, 0, NULL};

str2key_t structmsgFieldTypes[] = {
  {"uint8_t",   STRUCT_MSG_FIELD_UINT8_T},
  {"int8_t",    STRUCT_MSG_FIELD_INT8_T},
  {"uint16_t",  STRUCT_MSG_FIELD_UINT16_T},
  {"int16_t",   STRUCT_MSG_FIELD_INT16_T},
  {"uint32_t",  STRUCT_MSG_FIELD_UINT32_T},
  {"int32_t",   STRUCT_MSG_FIELD_INT32_T},
  {"uint8_t*",  STRUCT_MSG_FIELD_UINT8_VECTOR},
  {"int8_t*",   STRUCT_MSG_FIELD_INT8_VECTOR},
  {"uint16_t*", STRUCT_MSG_FIELD_UINT16_VECTOR},
  {"int16_t*",  STRUCT_MSG_FIELD_INT16_VECTOR},
  {"uint32_t*", STRUCT_MSG_FIELD_UINT32_VECTOR},
  {"int32_t*",  STRUCT_MSG_FIELD_INT32_VECTOR},
  {NULL, -1},
};

str2key_t structmsgSpecialFieldNames[] = {
  {"@src",         STRUCT_MSG_SPEC_FIELD_SRC},
  {"@dst",         STRUCT_MSG_SPEC_FIELD_DST},
  {"@targetCmd",   STRUCT_MSG_SPEC_FIELD_TARGET_CMD},
  {"@totalLgth",   STRUCT_MSG_SPEC_FIELD_TOTAL_LGTH},
  {"@cmdKey",      STRUCT_MSG_SPEC_FIELD_CMD_KEY},
  {"@cmdLgth",     STRUCT_MSG_SPEC_FIELD_CMD_LGTH},
  {"@randomNum",   STRUCT_MSG_SPEC_FIELD_RANDOM_NUM},
  {"@sequenceNum", STRUCT_MSG_SPEC_FIELD_SEQUENCE_NUM},
  {"@filler",      STRUCT_MSG_SPEC_FIELD_FILLER},
  {"@crc",         STRUCT_MSG_SPEC_FIELD_CRC},
  {"@id",          STRUCT_MSG_SPEC_FIELD_ID},
  {"@listlen",     STRUCT_MSG_SPEC_FIELD_LIST_LEN},
  {"@lstgrplen",   STRUCT_MSG_SPEC_FIELD_LSTGRP_LEN},
  {NULL, -1},
};


// ============================= stmsg_getFieldTypeKey ========================

int stmsg_getFieldTypeKey(const uint8_t *str) {
  str2key_t *entry = &structmsgFieldTypes[0];
  while (entry->str != NULL) {
    if (c_strcmp(entry->str, str) == 0) {
      return entry->key;
    }
    entry++;
  }
  return -1;
}

// ============================= stmsg_getFieldTypeStr ========================

uint8_t *stmsg_getFieldTypeStr(uint8_t key) {
  str2key_t *entry = &structmsgFieldTypes[0];
  while (entry->str != NULL) {
    if (entry->key == key) {
      return entry->str;
    }
    entry++;
  }
  return NULL;
}

// ============================= structmsg_getFieldNameIdStr ========================

static int structmsg_getFieldNameIdStr (int key, uint8_t **fieldName) {
  *fieldName = NULL;
  // first try to find special field name
  str2key_t *entry = &structmsgSpecialFieldNames[0];
  while (entry->str != NULL) {
    if (entry->key == key) {
      *fieldName = entry->str;
      return STRUCT_MSG_ERR_OK;
    }
    entry++;
  }
  // find field name
  int idx = 0;

  while (idx < fieldNameDefinitions.numDefinitions) {
    entry = &fieldNameDefinitions.definitions[idx];
    if (entry->key == key) {
      *fieldName = entry->str;
      return STRUCT_MSG_ERR_OK;
    }
    entry++;
    idx++;
  }
  return STRUCT_MSG_ERR_FIELD_NOT_FOUND;
}

// ============================= structmsg_getFieldNameId ========================

static int structmsg_getFieldNameId (const uint8_t *fieldName, int *key) {
  int fieldNameId = 0;
  int found = 0;
  str2key_t definition;
  str2key_t *newDefinition;

  if (fieldName[0] == '@') {
    // find special field name
    str2key_t *entry = &structmsgSpecialFieldNames[0];
    while (entry->str != NULL) {
      if (c_strcmp(entry->str, fieldName) == 0) {
        *key = entry->key;
        return STRUCT_MSG_ERR_OK;
      }
      entry++;
    }
    return STRUCT_MSG_ERR_BAD_SPECIAL_FIELD;
  } else {
    if (fieldNameDefinitions.numDefinitions >= fieldNameDefinitions.maxDefinitions) {
      if (fieldNameDefinitions.maxDefinitions == 0) {
        fieldNameDefinitions.maxDefinitions = 4;
        fieldNameDefinitions.definitions = (str2key_t *)os_zalloc(fieldNameDefinitions.maxDefinitions * sizeof(str2key_t));
        checkAllocOK(fieldNameDefinitions.definitions);
      } else {
        fieldNameDefinitions.maxDefinitions += 2;
        fieldNameDefinitions.definitions = (str2key_t *)os_realloc(fieldNameDefinitions.definitions, (fieldNameDefinitions.maxDefinitions * sizeof(str2key_t)));
        checkAllocOK(fieldNameDefinitions.definitions);
      }
    }
    if (fieldNameDefinitions.numDefinitions > 0) {
      // find field name
      int idx = 0;

      while (idx < fieldNameDefinitions.numDefinitions) {
        str2key_t *entry = &fieldNameDefinitions.definitions[idx];
        if (c_strcmp(entry->str, fieldName) == 0) {
          *key = entry->key;
        }
        entry++;
        idx++;
      }
    }
    newDefinition = &fieldNameDefinitions.definitions[fieldNameDefinitions.numDefinitions];
    newDefinition->key = fieldNameDefinitions.numDefinitions + 1;
    newDefinition->str = os_malloc(c_strlen(fieldName) + 1);
    newDefinition->str[c_strlen(fieldName)] = '\0';
    c_memcpy(newDefinition->str, fieldName, c_strlen(fieldName));
    fieldNameDefinitions.numDefinitions++;
    *key = newDefinition->key;
  }
  return STRUCT_MSG_ERR_OK;
}

// ============================= structmsg_getFieldTypeId ========================

static int structmsg_getFieldTypeId (const uint8_t *fieldTypeStr, int *key) {
  // find field type
  str2key_t *entry = &structmsgFieldTypes[0];

  while (entry->str != NULL) {
    if (c_strcmp(entry->str, fieldTypeStr) == 0) {
      *key = entry->key;
      return STRUCT_MSG_ERR_OK;
    }
    entry++;
  }
  return STRUCT_MSG_ERR_BAD_FIELD_TYPE;
}

// ============================= structmsg_createStructmsgDefinition ========================

int structmsg_createStructmsgDefinition (const uint8_t *name, size_t numFields) {
  stmsgDefinition_t *definition;
  size_t lgth;

  if (structmsgDefinitions.numDefinitions >= structmsgDefinitions.maxDefinitions) {
    if (structmsgDefinitions.maxDefinitions == 0) {
      structmsgDefinitions.maxDefinitions = 4;
      structmsgDefinitions.definitions = (stmsgDefinition_t *)os_zalloc(structmsgDefinitions.maxDefinitions * sizeof(stmsgDefinition_t));
      checkAllocOK(structmsgDefinitions.definitions);
    } else {
      structmsgDefinitions.maxDefinitions += 2;
      structmsgDefinitions.definitions = (stmsgDefinition_t *)os_realloc(structmsgDefinitions.definitions, (structmsgDefinitions.maxDefinitions * sizeof(stmsgDefinition_t)));
      checkAllocOK(structmsgDefinitions.definitions);
    }
  }
  definition = &structmsgDefinitions.definitions[structmsgDefinitions.numDefinitions];
  definition->numFields = 0;
  definition->maxFields = numFields;
  definition->fieldInfos = (fieldInfoDefinition_t *)os_zalloc(numFields * sizeof(fieldInfoDefinition_t));
  checkAllocOK(definition->fieldInfos);
  structmsgDefinitions.numDefinitions++;
  lgth = c_strlen(name);
  definition->name = os_malloc(lgth + 1);
  definition->name[lgth] = '\0';
  checkAllocOK(definition->name);
  c_memcpy(definition->name, name, lgth);
  return STRUCT_MSG_ERR_OK;
}

// ============================= structmsg_addFieldDefinition ========================

int structmsg_addFieldDefinition (const uint8_t *name, const uint8_t *fieldName, const uint8_t *fieldTypeStr, size_t fieldLgth) {
  stmsgDefinition_t *definition;
  fieldInfoDefinition_t *fieldInfo;
  int fieldId;
  int fieldType;
  int idx = 0;
  int found = 0;
  int result;

//ets_printf("addFieldDefinition: %s %s %s %d\n", name, fieldName, fieldTypeStr, fieldLgth);
  while (idx < structmsgDefinitions.numDefinitions) {
    definition = &structmsgDefinitions.definitions[idx];
    if (c_strcmp(name, definition->name) == 0) {
      found = 1;
      break;
    }
    definition++;
    idx++;
  }
  if (!found) {
    return STRUCT_MSG_DEFINITION_NOT_FOUND;
  }
  if (definition->numFields >= definition->maxFields) {
    return STRUCT_MSG_DEFINITION_TOO_MANY_FIELDS;
  }
  fieldInfo = &definition->fieldInfos[definition->numFields];
  result = structmsg_getFieldNameId(fieldName, &fieldId);
  checkErrOK(result);
  fieldInfo->fieldId = fieldId;
  result = structmsg_getFieldTypeId(fieldTypeStr, &fieldType);
  checkErrOK(result);
  fieldInfo->fieldType = fieldType;
  fieldInfo->fieldLgth = fieldLgth;
//ets_printf("add field: %s id: %d type: %d lgth: %d numFields: %d\n", fieldName, fieldId, fieldType, fieldLgth, definition->numFields);
  definition->numFields++;
  return STRUCT_MSG_ERR_OK;
}

// ============================= structmsg_dumpFieldDefinition ========================

int structmsg_dumpFieldDefinition (const uint8_t *name) {
  stmsgDefinition_t *definition;
  fieldInfoDefinition_t *fieldInfo;
  uint8_t *fieldIdStr;
  uint8_t *fieldTypeStr;
  int fieldType;
  int idx = 0;
  int found = 0;
  int result;

ets_printf("dumpFieldDefinition: %s\n", name);
  while (idx < structmsgDefinitions.numDefinitions) {
    definition = &structmsgDefinitions.definitions[idx];
    if (c_strcmp(name, definition->name) == 0) {
      found = 1;
      break;
    }
    definition++;
    idx++;
  }
  if (!found) {
    return STRUCT_MSG_DEFINITION_NOT_FOUND;
  }
  ets_printf("definition: %s numFields: %d\n", name, definition->numFields);
  idx = 0;
  while (idx < definition->numFields) {
    fieldInfo = &definition->fieldInfos[idx];
    result = structmsg_getFieldNameIdStr(fieldInfo->fieldId, &fieldIdStr);
    checkErrOK(result);
    fieldTypeStr = stmsg_getFieldTypeStr(fieldInfo->fieldType);
    if (fieldTypeStr == NULL) {
      return STRUCT_MSG_ERR_BAD_FIELD_TYPE;
    }
    ets_printf("  idx: %d id: %d %s type: %d %s lgth: %d\n", idx, fieldInfo->fieldId, fieldIdStr, fieldInfo->fieldType, fieldTypeStr, fieldInfo->fieldLgth);
    idx++;
  }
  return STRUCT_MSG_ERR_OK;
}
