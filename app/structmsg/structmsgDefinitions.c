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

// ============================= structmsg_getFieldTypeId ========================

int structmsg_getFieldTypeId(const uint8_t *str, uint8_t *fieldType) {
  str2key_t *entry = &structmsgFieldTypes[0];
  while (entry->str != NULL) {
    if (c_strcmp(entry->str, str) == 0) {
      *fieldType = entry->key;
      return STRUCT_MSG_ERR_OK;
    }
    entry++;
  }
  return STRUCT_MSG_ERR_BAD_FIELD_TYPE;
}

// ============================= structmsg_getFieldTypeStr ========================

int structmsg_getFieldTypeStr(uint8_t key, uint8_t **fieldType) {
  str2key_t *entry = &structmsgFieldTypes[0];
  while (entry->str != NULL) {
    if (entry->key == key) {
      *fieldType = entry->str;
      return STRUCT_MSG_ERR_OK;;
    }
    entry++;
  }
  return STRUCT_MSG_ERR_BAD_FIELD_TYPE;
}

// ============================= structmsg_getIdFieldNameStr ========================

int structmsg_getIdFieldNameStr (int id, uint8_t **fieldName) {
  str2key_t *entry;
  name2id_t *nameEntry;

  *fieldName = NULL;
  // first try to find special field name
  entry = &structmsgSpecialFieldNames[0];
  while (entry->str != NULL) {
    if (entry->key == id) {
      *fieldName = entry->str;
      return STRUCT_MSG_ERR_OK;
    }
    entry++;
  }
  // find field name
  int idx = 0;

  while (idx < fieldNameDefinitions.numDefinitions) {
    nameEntry = &fieldNameDefinitions.definitions[idx];
    if (nameEntry->id == id) {
      *fieldName = nameEntry->str;
      return STRUCT_MSG_ERR_OK;
    }
    nameEntry++;
    idx++;
  }
  return STRUCT_MSG_ERR_FIELD_NOT_FOUND;
}

// ============================= structmsg_getFieldNameId ========================

int structmsg_getFieldNameId (const uint8_t *fieldName, int *id, int incrRefCnt) {
  int fieldNameId = 0;
  int found = 0;
  int nameIdx = 0;
  int firstFreeEntryId;
  name2id_t definition;
  str2key_t *entry;
  name2id_t *newDefinition;
  name2id_t *nameEntry;
  name2id_t *firstFreeEntry;

  if (fieldName[0] == '@') {
    // find special field name
    entry = &structmsgSpecialFieldNames[0];
    while (entry->str != NULL) {
      if (c_strcmp(entry->str, fieldName) == 0) {
        *id = entry->key;
        return STRUCT_MSG_ERR_OK;
      }
      entry++;
    }
    return STRUCT_MSG_ERR_BAD_SPECIAL_FIELD;
  } else {
    if ((incrRefCnt == STRUCT_MSG_INCR) && (fieldNameDefinitions.numDefinitions >= fieldNameDefinitions.maxDefinitions)) {
      if (fieldNameDefinitions.maxDefinitions == 0) {
        fieldNameDefinitions.maxDefinitions = 4;
        fieldNameDefinitions.definitions = (name2id_t *)os_zalloc((fieldNameDefinitions.maxDefinitions * sizeof(name2id_t)));
        checkAllocOK(fieldNameDefinitions.definitions);
      } else {
        fieldNameDefinitions.maxDefinitions += 2;
        fieldNameDefinitions.definitions = (name2id_t *)os_realloc((fieldNameDefinitions.definitions), (fieldNameDefinitions.maxDefinitions * sizeof(name2id_t)));
        checkAllocOK(fieldNameDefinitions.definitions);
      }
    }
    firstFreeEntry = NULL;
    firstFreeEntryId = 0;
    if (fieldNameDefinitions.numDefinitions > 0) {
      // find field name
      nameIdx = 0;
      while (nameIdx < fieldNameDefinitions.numDefinitions) {
        nameEntry = &fieldNameDefinitions.definitions[nameIdx];
        if ((nameEntry->str != NULL) && (c_strcmp(nameEntry->str, fieldName) == 0)) {
          if (incrRefCnt < 0) {
            if (nameEntry->refCnt > 0) {
              nameEntry->refCnt--;
            }
            if (nameEntry->refCnt == 0) {
              nameEntry->id = STRUCT_MSG_FREE_FIELD_ID;
              os_free(nameEntry->str);
              nameEntry->str = NULL;
            }
          } else {
            if (incrRefCnt > 0) {
              nameEntry->refCnt++;
            } else {
              // just get the entry, do not modify
            }
          }
          *id = nameEntry->id;
          return STRUCT_MSG_ERR_OK;
        }
        if ((incrRefCnt == STRUCT_MSG_INCR) && (nameEntry->id == STRUCT_MSG_FREE_FIELD_ID) && (firstFreeEntry == NULL)) {
          firstFreeEntry = nameEntry;
          firstFreeEntry->id = nameIdx + 1;
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
          *id = firstFreeEntry->id;
          firstFreeEntry->refCnt = 1;
          firstFreeEntry->str = os_malloc(c_strlen(fieldName) + 1);
          firstFreeEntry->str[c_strlen(fieldName)] = '\0';
          c_memcpy(firstFreeEntry->str, fieldName, c_strlen(fieldName));
        } else {
          newDefinition = &fieldNameDefinitions.definitions[fieldNameDefinitions.numDefinitions];
          newDefinition->refCnt = 1;
          newDefinition->id = fieldNameDefinitions.numDefinitions + 1;
          newDefinition->str = os_malloc(c_strlen(fieldName) + 1);
          newDefinition->str[c_strlen(fieldName)] = '\0';
          c_memcpy(newDefinition->str, fieldName, c_strlen(fieldName));
          fieldNameDefinitions.numDefinitions++;
          *id = newDefinition->id;
        }
      }
    }
  }
  return STRUCT_MSG_ERR_OK;
}

// ============================= structmsg_getFieldTypeId ========================

static int structmsg_getFieldTypeIdXX (const uint8_t *fieldTypeStr, int *key) {
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
  int definitionIdx;

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
  // check for unused slot!
  definitionIdx = 0;
  while (definitionIdx < structmsgDefinitions.maxDefinitions) {
    definition = &structmsgDefinitions.definitions[definitionIdx];
    if (definition->name == NULL) {
      definition->encoded = NULL;
      definition->todecode = NULL;
      definition->encrypted = NULL;
      definition->numFields = 0;
      definition->maxFields = numFields;
      definition->fieldInfos = (fieldInfoDefinition_t *)os_zalloc(numFields * sizeof(fieldInfoDefinition_t));
      checkAllocOK(definition->fieldInfos);
      if (definitionIdx >= structmsgDefinitions.numDefinitions) {
        structmsgDefinitions.numDefinitions++;
      }
      lgth = c_strlen(name);
      definition->name = os_malloc(lgth + 1);
      definition->name[lgth] = '\0';
      checkAllocOK(definition->name);
      c_memcpy(definition->name, name, lgth);
      return STRUCT_MSG_ERR_OK;
    }
    definitionIdx++;
  }
  return STRUCT_MSG_ERR_NO_SLOT_FOUND;
}

// ============================= structmsg_getDefinitionPtr ========================

int structmsg_getDefinitionPtr (const uint8_t *name, stmsgDefinition_t **definition, uint8_t *definitionsIdx) {
  *definitionsIdx = 0;
  while (*definitionsIdx < structmsgDefinitions.numDefinitions) {
    *definition = &structmsgDefinitions.definitions[*definitionsIdx];
    if (((*definition)->name != NULL) && (c_strcmp(name, (*definition)->name) == 0)) {
      return STRUCT_MSG_ERR_OK;
    }
    (*definitionsIdx)++;
  }
  return STRUCT_MSG_ERR_DEFINITION_NOT_FOUND;
}

// ============================= structmsg_addFieldDefinition ========================

int structmsg_addFieldDefinition (const uint8_t *name, const uint8_t *fieldName, const uint8_t *fieldTypeStr, size_t fieldLgth) {
  stmsgDefinition_t *definition;
  uint8_t definitionsIdx;
  int result;
  fieldInfoDefinition_t *fieldInfo;
  int fieldId;
  uint8_t fieldType;
  int idx;
  int found = 0;

//ets_printf("addFieldDefinition: %s %s %s %d\n", name, fieldName, fieldTypeStr, fieldLgth);
  result = structmsg_getDefinitionPtr(name, &definition, &definitionsIdx);
  checkErrOK(result);
  if (definition->numFields >= definition->maxFields) {
    return STRUCT_MSG_ERR_DEFINITION_TOO_MANY_FIELDS;
  }
  fieldInfo = &definition->fieldInfos[definition->numFields];
  result = structmsg_getFieldNameId(fieldName, &fieldId, STRUCT_MSG_INCR);
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
  uint8_t definitionsIdx;
  int fieldType;
  int idx = 0;
  int found = 0;
  int result;

  result = structmsg_getDefinitionPtr(name, &definition, &definitionsIdx);
  checkErrOK(result);
  ets_printf("definition: %s numFields: %d\n", name, definition->numFields);
  idx = 0;
  while (idx < definition->numFields) {
    fieldInfo = &definition->fieldInfos[idx];
    result = structmsg_getIdFieldNameStr(fieldInfo->fieldId, &fieldIdStr);
    checkErrOK(result);
    result = structmsg_getFieldTypeStr(fieldInfo->fieldType, &fieldTypeStr);
    checkErrOK(result);
    ets_printf("  idx: %d id: %d %s type: %d %s lgth: %d\n", idx, fieldInfo->fieldId, fieldIdStr, fieldInfo->fieldType, fieldTypeStr, fieldInfo->fieldLgth);
    idx++;
  }
  return STRUCT_MSG_ERR_OK;
}

// ============================= structmsg_encodeFieldDefinitionMessage ========================

int structmsg_encodeFieldDefinitionMessage (const uint8_t *name, uint8_t **data, int *lgth) {
  // FIXME!!
  uint16_t src = 123;
  uint16_t dst = 987;

  return structmsg_encodeDefinition(name, data, lgth, &structmsgDefinitions, &fieldNameDefinitions);
}

// ============================= structmsg_decodeFieldDefinitionMessage ========================

int structmsg_decodeFieldDefinitionMessage (const uint8_t *name, const uint8_t *data) {
  // FIXME!!
  uint16_t src = 123;
  uint16_t dst = 987;

  return structmsg_decodeDefinition(name, data, &structmsgDefinitions, &fieldNameDefinitions);
}


// ============================= structmsg_encdecDefinition ========================

int structmsg_encdecDefinition(const uint8_t *name, const uint8_t *key, size_t klen, const uint8_t *iv, size_t ivlen, bool enc, uint8_t **buf, int *lgth) {
  stmsgDefinition_t *definition;
  uint8_t definitionsIdx;
  int result;

  *buf = NULL;
  *lgth = 0;
  result = structmsg_getDefinitionPtr(name, &definition, &definitionsIdx);
  checkErrOK(result);

  if (enc) {
    if (definition->encoded == NULL) {
      return STRUCT_MSG_ERR_NOT_ENCODED;
    }
    result = structmsg_encryptdecrypt(NULL, definition->encoded, definition->totalLgth, key, klen, iv, ivlen, enc, &definition->encrypted, lgth);
    *buf = definition->encrypted;
  } else {
    if (definition->encrypted == NULL) {
      return STRUCT_MSG_ERR_NOT_ENCRYPTED;
    }
    result = structmsg_encryptdecrypt(NULL, definition->encrypted, definition->totalLgth, key, klen, iv, ivlen, enc, &definition->todecode, lgth);
    *buf = definition->todecode;
  }
  return result;
} 

// ============================= stmsg_setCryptedDefinition ========================

int stmsg_setCryptedDefinition(const uint8_t *name, const uint8_t *crypted, int cryptedLgth) {
  stmsgDefinition_t *definition;
  uint8_t definitionsIdx;
  int result;

  result = structmsg_getDefinitionPtr(name, &definition, &definitionsIdx);
  checkErrOK(result);
  if (definition->encrypted != NULL) {
    os_free(definition->encrypted);
  }
  definition->encrypted = (uint8_t *)os_malloc(cryptedLgth);
  checkAllocOK(definition->encrypted);
  c_memcpy(definition->encrypted, crypted, cryptedLgth);
  return STRUCT_MSG_ERR_OK;
}

// ============================= stmsg_decryptGetDefinitionName  ========================

int stmsg_decryptGetDefinitionName(const uint8_t *encryptedMsg, size_t mlen, const uint8_t *key, size_t klen, const uint8_t *iv, size_t ivlen, uint8_t **name) {
  uint8_t *decrypted;
  size_t lgth;
  int result;

   decrypted = NULL;
   lgth = 0; 
   result = structmsg_encryptdecrypt(NULL, encryptedMsg, mlen, key, klen, iv, ivlen, false, &decrypted, &lgth);
   if (result != STRUCT_MSG_ERR_OK) {
     return result;
   }
   result = stmsg_getDefinitionName(decrypted, name);
   return result;
}

// ============================= structmsg_deleteDefinition ========================

int structmsg_deleteDefinition(const uint8_t *name, stmsgDefinitions_t *structmsgDefinitions, fieldNameDefinitions_t *fieldNameDefinitions) {
  stmsgDefinition_t *definition;
  fieldInfoDefinition_t *fieldInfo;
  name2id_t *nameEntry;
  uint8_t definitionsIdx;
  int idx;
  int nameIdx;
  int nameFound;
  int result;
  int fieldId;

  result =  structmsg_getDefinitionPtr(name, &definition, &definitionsIdx);
  checkErrOK(result);
  idx = 0;
  while (idx < definition->numFields) {
    fieldInfo = &definition->fieldInfos[idx];
    nameIdx = 0;
    nameFound = 0;
    if (fieldInfo->fieldId < STRUCT_MSG_SPEC_FIELD_LOW) {
      while (nameIdx < fieldNameDefinitions->numDefinitions) {
        nameEntry = &fieldNameDefinitions->definitions[nameIdx];
        if (fieldInfo->fieldId == nameEntry->id) {
          result = structmsg_getFieldNameId(nameEntry->str, &fieldId, STRUCT_MSG_DECR);
          checkErrOK(result);
          nameFound = 1;
          break;
        }
        nameIdx++;
      }
      if (!nameFound) {
        return STRUCT_MSG_ERR_FIELD_NOT_FOUND;
      }
    }
    idx++;
  }
  // nameDefinitions deleted

  definition->numFields = 0;
  definition->maxFields = 0;
  os_free(definition->name);
  definition->name = NULL;
  if (definition->encoded != NULL) {
    os_free(definition->encoded);
    definition->encoded = NULL;
  }
  os_free(definition->fieldInfos);
  definition->fieldInfos = NULL;
  if (definition->encoded != NULL) {
    os_free(definition->encoded);
    definition->encoded = NULL;
  }
  if (definition->encrypted != NULL) {
    os_free(definition->encrypted);
    definition->encrypted = NULL;
  }
  if (definition->todecode != NULL) {
    os_free(definition->todecode);
    definition->todecode = NULL;
  }
  definition->totalLgth = 0;
  // definition deleted

  return STRUCT_MSG_ERR_OK;
}

// ============================= structmsg_deleteDefinitions ========================

int structmsg_deleteDefinitions(stmsgDefinitions_t *structmsgDefinitions, fieldNameDefinitions_t *fieldNameDefinitions) {
  // delete the whole structmsgDefinitions info, including fieldNameDefinitions info
  stmsgDefinition_t *definition;
  fieldInfoDefinition_t *fieldInfo;
  name2id_t *nameEntry;
  uint8_t *name;
  int idx;
  int nameIdx;
  int found;
  int nameFound;
  int result;
  int fieldId;

  idx = 0;
  while (idx < structmsgDefinitions->numDefinitions) {
    definition = &structmsgDefinitions->definitions[idx];
    if (definition->name != NULL) {
      structmsg_deleteDefinition(definition->name, structmsgDefinitions, fieldNameDefinitions);
    }
    idx++;
  }
  structmsgDefinitions->numDefinitions = 0;
  structmsgDefinitions->maxDefinitions = 0;
  os_free(structmsgDefinitions->definitions);
  structmsgDefinitions->definitions = NULL;

  fieldNameDefinitions->numDefinitions = 0;
  fieldNameDefinitions->maxDefinitions = 0;
  os_free(fieldNameDefinitions->definitions);
  fieldNameDefinitions->definitions = NULL;

  // all deleted/reset
  return STRUCT_MSG_ERR_OK;
}

// ============================= structmsg_deleteStructmsgDefinition ========================

int structmsg_deleteStructmsgDefinition(const uint8_t *name) {
  return structmsg_deleteDefinition(name, &structmsgDefinitions, &fieldNameDefinitions);
}

// ============================= structmsg_deleteStructmsgDefinitions ========================

int structmsg_deleteStructmsgDefinitions() {
  // delete the whole structmsgDefinitions info, including fieldNameDefinitions info
  return structmsg_deleteDefinitions(&structmsgDefinitions, &fieldNameDefinitions);
}

// ============================= structmsg_createMsgFromDefinition ========================

int structmsg_createMsgFromDefinition(const uint8_t *name) {
  stmsgDefinition_t *definition;
  fieldInfoDefinition_t *fieldInfo;
  uint8_t *fieldName;
  uint8_t *fieldType;
  uint8_t *handle;
  uint8_t definitionsIdx;
  int fieldIdx;
  int result;

  result = structmsg_getDefinitionPtr(name, &definition, &definitionsIdx);
  checkErrOK(result);
  result = stmsg_createMsg(definition->numFields - STRUCT_MSG_NUM_HEADER_FIELDS - STRUCT_MSG_NUM_CMD_HEADER_FIELDS, &handle);
//ets_printf("create: handle: %s numFields: %d\n", handle, definition->numFields - STRUCT_MSG_NUM_HEADER_FIELDS - STRUCT_MSG_NUM_CMD_HEADER_FIELDS);
  checkErrOK(result);
  while (fieldIdx < definition->numFields) {
    fieldInfo = &definition->fieldInfos[fieldIdx];
    switch (fieldInfo->fieldId) {
    case STRUCT_MSG_SPEC_FIELD_SRC:
    case STRUCT_MSG_SPEC_FIELD_DST:
    case STRUCT_MSG_SPEC_FIELD_TARGET_CMD:
    case STRUCT_MSG_SPEC_FIELD_TOTAL_LGTH:
    case STRUCT_MSG_SPEC_FIELD_CMD_KEY:
    case STRUCT_MSG_SPEC_FIELD_CMD_LGTH:
      // nothing to do!
      break;
    default:
      result = structmsg_getIdFieldNameStr(fieldInfo->fieldId, &fieldName);
      checkErrOK(result);
      result = structmsg_getFieldTypeStr(fieldInfo->fieldType, &fieldType);
      checkErrOK(result);
//ets_printf("addfield: %s %s %d\n", fieldName, fieldType, fieldInfo->fieldLgth);
      result = structmsg_getFieldTypeStr(fieldInfo->fieldType, &fieldType);
      checkErrOK(result);
      result = stmsg_addField(handle, fieldName, fieldType, fieldInfo->fieldLgth);
      checkErrOK(result);
      break;
    } 
    fieldIdx++;
  }
  return STRUCT_MSG_ERR_OK;
}

// ============================= structmsg_getDefinitionNormalFieldNames ========================

int structmsg_getDefinitionNormalFieldNames(const uint8_t *name, uint8_t ***normalFieldNames) {
  stmsgDefinition_t *definition;
  uint8_t definitionsIdx;
  int result;

  result = structmsg_getDefinitionPtr(name, &definition, &definitionsIdx);
  checkErrOK(result);
  return STRUCT_MSG_ERR_OK;
}

// ============================= structmsg_getDefinitionTableFieldNames ========================

int structmsg_getDefinitionTableFieldNames(const uint8_t *name, uint8_t ***tableFieldNames) {
  stmsgDefinition_t *definition;
  uint8_t definitionsIdx;
  int result;

  result = structmsg_getDefinitionPtr(name, &definition, &definitionsIdx);
  checkErrOK(result);
  return STRUCT_MSG_ERR_OK;
}

// ============================= structmsg_getDefinitionNumTableRows ========================

int structmsg_getDefinitionNumTableRows(const uint8_t *name, uint8_t *numTableRows) {
  stmsgDefinition_t *definition;
  fieldInfoDefinition_t *fieldInfo;
  uint8_t definitionsIdx;
  int idx;
  int result;
  uint8_t *fieldName;

  result = structmsg_getDefinitionPtr(name, &definition, &definitionsIdx);
  checkErrOK(result);
  idx = 0;
  while(idx < definition->numFields) {
    fieldInfo = &definition->fieldInfos[idx];
    result  = structmsg_getIdFieldNameStr (fieldInfo->fieldId, &fieldName);
    checkErrOK(result);
    if (c_strcmp(fieldName, "@tablerows") == 0) {
      *numTableRows = fieldInfo->fieldLgth;
      return STRUCT_MSG_ERR_OK;
    }
  }
  return STRUCT_MSG_ERR_FIELD_NOT_FOUND;
}

// ============================= structmsg_getDefinitionNumTableRowFields ========================

int structmsg_getDefinitionNumTableRowFields(const uint8_t *name, uint8_t *numTableRowFields) {
  stmsgDefinition_t *definition;
  fieldInfoDefinition_t *fieldInfo;
  uint8_t definitionsIdx;
  int idx;
  int result;
  uint8_t *fieldName;

  result = structmsg_getDefinitionPtr(name, &definition, &definitionsIdx);
  checkErrOK(result);
  idx = 0;
  while(idx < definition->numFields) {
    fieldInfo = &definition->fieldInfos[idx];
    result  = structmsg_getIdFieldNameStr (fieldInfo->fieldId, &fieldName);
    checkErrOK(result);
    if (c_strcmp(fieldName, "@tablerowfields") == 0) {
      *numTableRowFields = fieldInfo->fieldLgth;
      return STRUCT_MSG_ERR_OK;
    }
  }
  return STRUCT_MSG_ERR_FIELD_NOT_FOUND;
}

// ============================= structmsg_getDefinitionFieldInfo ========================

int structmsg_getDefinitionFieldInfo(const uint8_t *name, const uint8_t *fieldName, fieldInfoDefinition_t **fieldInfo) {
  stmsgDefinition_t *definition;
  uint8_t definitionsIdx;
  int idx;
  int result;
  uint8_t *lookupFieldName;

  result = structmsg_getDefinitionPtr(name, &definition, &definitionsIdx);
  checkErrOK(result);
  idx = 0;
  while(idx < definition->numFields) {
    *fieldInfo = &definition->fieldInfos[idx];
    result  = structmsg_getIdFieldNameStr ((*fieldInfo)->fieldId, &lookupFieldName);
    checkErrOK(result);
    if (c_strcmp(lookupFieldName, fieldName) == 0) {
      return STRUCT_MSG_ERR_OK;
    }
  }
  return STRUCT_MSG_ERR_FIELD_NOT_FOUND;
}

// ============================= structmsg_getDefinitionTableFieldInfo ========================

int structmsg_getDefinitionTableFieldInfo(const uint8_t *name, const uint8_t *fieldName, fieldInfoDefinition_t **fieldInfo) {
  stmsgDefinition_t *definition;
  uint8_t definitionsIdx;
  int idx;
  int result;
  uint8_t *lookupFieldName;

  result = structmsg_getDefinitionPtr(name, &definition, &definitionsIdx);
  checkErrOK(result);
  idx = 0;
  while(idx < definition->numFields) {
    *fieldInfo = &definition->fieldInfos[idx];
    result  = structmsg_getIdFieldNameStr ((*fieldInfo)->fieldId, &lookupFieldName);
    checkErrOK(result);
    if (c_strcmp(lookupFieldName, fieldName) == 0) {
      return STRUCT_MSG_ERR_OK;
    }
  }
  return STRUCT_MSG_ERR_FIELD_NOT_FOUND;
}

