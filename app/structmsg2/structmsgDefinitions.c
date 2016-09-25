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
#include "structmsg2.h"

#ifdef NOTDEF
static stmsgDefinitions_t structmsgDefinitions = {0, 0, NULL};

// ============================= structmsg_getIdFieldNameStr ========================

int structmsg_getIdFieldNameStr (int id, uint8_t **fieldName) {
}

// ============================= structmsg_createStructmsgDefinition ========================

int structmsg_createStructmsgDefinition (const uint8_t *name, size_t numFields, uint8_t shortCmdKey) {
  stmsgDefinition_t *definition;
  size_t lgth;
  int definitionIdx;

ets_printf("structmsg_createStructmsgDefinition: shortCmdKey: %d\n", shortCmdKey);
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
      if (shortCmdKey) {
        definition->flags |= STRUCT_MSG_SHORT_CMD_KEY;
      }
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

int structmsg_decodeFieldDefinitionMessage (const uint8_t *name, const uint8_t *data, uint8_t shortCmdKey) {
  // FIXME!!
  uint16_t src = 123;
  uint16_t dst = 987;

  return structmsg_decodeDefinition(name, data, &structmsgDefinitions, &fieldNameDefinitions, shortCmdKey);
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

int stmsg_decryptGetDefinitionName(const uint8_t *encryptedMsg, size_t mlen, const uint8_t *key, size_t klen, const uint8_t *iv, size_t ivlen, uint8_t **name, uint8_t shortCmdKey) {
  uint8_t *decrypted;
  size_t lgth;
  int result;

   decrypted = NULL;
   lgth = 0; 
   result = structmsg_encryptdecrypt(NULL, encryptedMsg, mlen, key, klen, iv, ivlen, false, &decrypted, &lgth);
   if (result != STRUCT_MSG_ERR_OK) {
     return result;
   }
   result = stmsg_getDefinitionName(decrypted, name, shortCmdKey);
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

int structmsg_createMsgFromDefinition(const uint8_t *name, uint8_t shortCmdKey) {
  stmsgDefinition_t *definition;
  structmsg_t *structmsg;
  fieldInfoDefinition_t *fieldInfo;
  uint8_t *fieldName;
  uint8_t *fieldType;
  uint8_t *handle;
  uint8_t definitionsIdx;
  int fieldIdx;
  int result;

ets_printf("structmsg_createMsgFromDefinition: shortCmdKey: %d\n", shortCmdKey);
  result = structmsg_getDefinitionPtr(name, &definition, &definitionsIdx);
  checkErrOK(result);
  result = stmsg_createMsg(definition->numFields - STRUCT_MSG_NUM_HEADER_FIELDS - STRUCT_MSG_NUM_CMD_HEADER_FIELDS, &handle, shortCmdKey);
//ets_printf("create: handle: %s numFields: %d\n", handle, definition->numFields - STRUCT_MSG_NUM_HEADER_FIELDS - STRUCT_MSG_NUM_CMD_HEADER_FIELDS);
  checkErrOK(result);
  // set flags if necessary
  structmsg = structmsg_get_structmsg_ptr(handle);
//  if (definition->flags & STRUCT_MSG_SHORT_CMD_KEY) {
//    structmsg->flags |= STRUCT_MSG_SHORT_CMD_KEY;
//  }
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

// ============================= structmsg_createMsgDefinitionFromListInfo ========================

int structmsg_createMsgDefinitionFromListInfo(const uint8_t *name, const uint8_t **listVector, uint8_t numEntries, uint8_t numRows, uint16_t flags,uint8_t shortCmdKey) {
  const uint8_t *listEntry;
  int idx;
  int result;
  uint8_t*cp;
  uint8_t *fieldName;
  uint8_t *fieldType;
  uint8_t *fieldLgthStr;
  char *endPtr;
  uint8_t fieldLgth;
  uint8_t *flagStr;
  uint8_t flag;
  unsigned long lgth;
  unsigned long uflag;

ets_printf("structmsg_createMsgDefinitionFromListInfo: shortCmdKey: %d\n", shortCmdKey);
  result = structmsg_createStructmsgDefinition(name, numEntries, shortCmdKey);
  checkErrOK(result);
  listEntry = listVector[0];
  idx = 0;
  while(idx < numEntries) {
    listEntry = listVector[idx];
    uint8_t buffer[c_strlen(listEntry) + 1];
    fieldName = buffer;
    c_memcpy(fieldName, listEntry, c_strlen(listEntry));
    fieldName[c_strlen(listEntry)] = '\0';
    cp = fieldName;
    while (*cp != ',') {
      cp++;
    }
    *cp++ = '\0';
    fieldType = cp;
    while (*cp != ',') {
      cp++;
    }
    *cp++ = '\0';
    fieldLgthStr = cp;
    while (*cp != ',') {
      cp++;
    }
    *cp++ = '\0';
    flagStr = cp;
    if (c_strcmp(fieldLgthStr,"@numRows") == 0) {
      fieldLgth = numRows;
    } else {
      lgth = c_strtoul(fieldLgthStr, &endPtr, 10);
      fieldLgth = (uint8_t)lgth;
    }
    uflag = c_strtoul(flagStr, &endPtr, 10);
    flag = (uint8_t)uflag;
    if (flag == 0) {
      result = structmsg_addFieldDefinition(name, fieldName, fieldType, fieldLgth);
      checkErrOK(result);
    } else {
      if ((flags != 0) && (flag == 2)) {
        result = structmsg_addFieldDefinition(name, fieldName, fieldType, fieldLgth);
        checkErrOK(result);
      } else {
        if ((flags == 0) && (flag == 1)) {
          result = structmsg_addFieldDefinition(name, fieldName, fieldType, fieldLgth);
          checkErrOK(result);
        }
      }
    }
    idx++;
  }
  return STRUCT_MSG_ERR_OK;
}
#endif
