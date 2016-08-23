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

// ============================= structmsg_createStructmsgDefinition ========================

int structmsg_createStructmsgDefinition (const uint8_t *name, size_t numFields) {
  stmsgDefinition_t definition;
  size_t lgth;

  if (structmsgDefinitions.numDefinitions >= structmsgDefinitions.maxDefinitions) {
    if (structmsgDefinitions.maxDefinitions == 0) {
      structmsgDefinitions.maxDefinitions = 4;
      structmsgDefinitions.definitions = (stmsgDefinition_t *)os_malloc(structmsgDefinitions.maxDefinitions * sizeof(stmsgDefinition_t));
      checkAllocOK(structmsgDefinitions.definitions);
    } else {
      structmsgDefinitions.maxDefinitions += 2;
      structmsgDefinitions.definitions = (stmsgDefinition_t *)os_realloc(structmsgDefinitions.definitions, (structmsgDefinitions.maxDefinitions * sizeof(stmsgDefinition_t)));
      checkAllocOK(structmsgDefinitions.definitions);
    }
  }
  definition = structmsgDefinitions.definitions[structmsgDefinitions.numDefinitions];
  definition.numFields = numFields;
  definition.fieldInfos = (fieldInfoDefinition_t *)os_malloc(numFields * sizeof(fieldInfoDefinition_t));
  checkAllocOK(definition.fieldInfos);
  structmsgDefinitions.numDefinitions++;
  lgth = c_strlen(name);
  definition.name = os_malloc(lgth + 1);
  definition.name[lgth] = '\0';
  checkAllocOK(definition.name);
  c_memcpy(definition.name, name, lgth);
  return STRUCT_MSG_ERR_OK;
}

// ============================= structmsg_getFieldNameId ========================

int structmsg_getFieldNameId (const uint8_t *fieldName) {
#ifdef NOTDEF
    set fieldNameDefinesDict $::structmsg(fieldNameDefines)
    switch -glob -- $fieldName {
      "@*" {
        if {![dict exists $fieldNameDefinesDict $fieldName]} {
          error "bad special field name: $fieldName!"
        } else {
          set fieldNameId [dict get $fieldNameDefinesDict $fieldName]
        }
      }
      default {
        if {![dict exists $fieldNameDefinesDict $fieldName]} {
          incr ::structmsg(numFieldNameIds)
          set fieldNameId  $::structmsg(numFieldNameIds)
          dict set fieldNameDefinesDict $fieldName $fieldNameId
        } else {
          set fieldNameId [dict get $fieldNameDefinesDict $fieldName]
        }
      }
    }
    set ::structmsg(fieldNameDefines) $fieldNameDefinesDict 
    return $fieldNameId
#endif
  return STRUCT_MSG_ERR_OK;
}

// ============================= structmsg_addFieldDefinition ========================

int structmsg_addFieldDefinition (const uint8_t *name, const uint8_t *fieldName, int fieldType, size_t fieldLgth) {
#ifdef NOTDEF
    set fieldDefinitionDict [dict create]
    set structmsgDefinitionsDict $::structmsg(structmsgDefinitions)
    if {![dict exists $::structmsg(structmsgDefinitions) $name]} {
      error "structmsg definition for $name does not exist"
    } else {
      set fieldInfos [dict get $structmsgDefinitionsDict $name fieldInfos]
      set fieldNameId [structmsg_getFieldNameId $fieldName]
      set fieldTypeId [structmsg_getFieldTypeId $fieldType]
      set fielNameDict [dict create]
      dict set fieldNameDict nameId $fieldNameId
      dict set fieldNameDict typeId $fieldTypeId
      dict set fieldNameDict length $fieldLgth
      lappend fieldInfos $fieldNameDict
      dict set ::structmsg(structmsgDefinitions) $name fieldInfos $fieldInfos
    }
#endif
  return STRUCT_MSG_ERR_OK;
}

