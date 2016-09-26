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
#include "../crypto/mech.h"

#ifdef NOTDEF
extern str2key_t structmsgSpecialFieldNames[];

// ============================= crcDecode ========================

static int crcDecode(const uint8_t *data, int offset, uint16_t lgth, uint16_t *crc, uint8_t headerLgth, uint8_t uint8_crc_flag) {
  return offset;
}

// ============================= getFieldIdName ========================

static int getFieldIdName (uint8_t id, fieldNameDefinitions_t *fieldNameDefinitions, uint8_t **fieldName) {
  // find field name
  int idx = 0;
  while (idx < fieldNameDefinitions->numDefinitions) {
    name2id_t *entry = &fieldNameDefinitions->definitions[idx];
    if (entry->id == id) {
       *fieldName = entry->str;
       return STRUCT_MSG_ERR_OK;
    }
    entry++;
    idx++;
  }
  return STRUCT_MSG_ERR_FIELD_NOT_FOUND;
}

// ============================= normalFieldNamesEncode ========================

static int normalFieldNamesEncode(uint8_t *data, int offset, stmsgDefinition_t *definition, fieldNameDefinitions_t *fieldNameDefinitions, id2offset_t **normNameOffsets, int numEntries, int size) {
  int idx;
  int namesOffset = 0;
  fieldInfoDefinition_t *fieldInfo;
  int result;
  int nameIdx;
  uint8_t *fieldName;
  id2offset_t *normNameOffset;

  // first the keys
  offset = uint8Encode(data, offset, numEntries);
  idx = 0;
  normNameOffset = normNameOffsets[0];
  while (idx < definition->numFields) {
    fieldInfo = &definition->fieldInfos[idx];
    if (fieldInfo->fieldId < STRUCT_MSG_SPEC_FIELD_LOW) { 
      result = getFieldIdName(fieldInfo->fieldId, fieldNameDefinitions, &fieldName);
      checkErrOK(result);
      offset = uint16Encode(data, offset, namesOffset);
      normNameOffset->id = fieldInfo->fieldId;
      normNameOffset->offset = namesOffset;
      normNameOffset++;
      namesOffset += c_strlen(fieldName) + 1;
    }
    idx++;
  }
  // and now the names
  offset = uint16Encode(data, offset, size);
  idx = 0;
  nameIdx = 1;
  while (idx < definition->numFields) {
    fieldInfo = &definition->fieldInfos[idx];
    if (fieldInfo->fieldId < STRUCT_MSG_SPEC_FIELD_LOW) { 
      result = getFieldIdName(fieldInfo->fieldId, fieldNameDefinitions, &fieldName);
      checkErrOK(result);
      offset = uint8VectorEncode(data, offset, fieldName, c_strlen(fieldName));
      if (nameIdx < numEntries) {
        offset = uint8Encode(data, offset, '\0');
      } else {
        offset = uint8Encode(data, offset, '\0');
      }
      nameIdx++;
    }
    idx++;
  }
  return offset;
}

// ============================= normalFieldNamesDecode ========================

static int normalFieldNamesDecode(const uint8_t *data, int offset) {
  return offset;
}

// ============================= definitionEncode ========================

static int definitionEncode(uint8_t *data, int offset, stmsgDefinition_t *definition, fieldNameDefinitions_t *fieldNameDefinitions, id2offset_t *normNamesOffsets) {
  int idx;
  int idIdx;
  int nameOffset;
  int found;
  fieldInfoDefinition_t *fieldInfo;

  uint8_t fieldId;
  uint8_t fieldType;
  uint16_t fieldLgth;

  idx= 0;
  fieldInfo = &definition->fieldInfos[0];
  offset = uint8Encode(data, offset, definition->numFields); 
  checkEncodeOffset(offset);
  while (idx < definition->numFields) {
    if (fieldInfo->fieldId < STRUCT_MSG_SPEC_FIELD_LOW) {
      idIdx = 0;
      found = 0;
      while (idIdx < definition->numFields) {
        if (normNamesOffsets[idIdx].id == 0) {
          // id 0 is not used to be able to stop here!!
          break;
        }
        if (fieldInfo->fieldId == normNamesOffsets[idIdx].id) {
          nameOffset = normNamesOffsets[idIdx].offset;
          found = 1;
          break;
        }
        idIdx++;
      }
      if (!found) {
        return STRUCT_MSG_ERR_FIELD_NOT_FOUND;
      }
      offset = uint16Encode(data, offset, nameOffset);
    } else {
      offset = uint16Encode(data, offset, fieldInfo->fieldId);
    }
    offset = uint8Encode(data, offset, fieldInfo->fieldType);
    offset = uint16Encode(data, offset, fieldInfo->fieldLgth);
    fieldInfo++;
    idx++;
  }
  return offset;
}

// ============================= definitionDecode ========================

static int definitionDecode(const uint8_t *data, int offset, stmsgDefinition_t *definition, fieldNameDefinitions_t *fieldNameDefinitions, uint8_t shortCmdKey) {
  uint16_t definitionLgth;
  uint8_t nameLgth;
  uint8_t numNameEntries;
  const uint8_t *name;
  const uint16_t *idsStart;
  const uint16_t *idsEnd;
  const uint8_t *namesStart;
  const uint8_t *namesEnd;
  uint16_t namesSize;
  const uint8_t *definitionStart;
  uint16_t fieldId;
  uint16_t nameOffset;
  uint8_t fieldTypeId;
  uint16_t fieldLgth;
  uint8_t *fieldName;
  uint8_t *fieldType;
  int definitionIdx;
  uint8_t numFields;
  int result;
  int myOffset;
  int namesIdx;

  // first the keys
  offset = uint8Decode(data, offset, &numNameEntries);
  checkBadOffset(offset);
  idsStart = (uint16_t *)(data + offset);
  offset += numNameEntries * sizeof(uint16_t);
  idsEnd = (uint16_t *)(data + offset);
  // and now the names
  offset = uint16Decode(data, offset, &namesSize);
  checkBadOffset(offset);
  namesStart = data + offset;
  offset += namesSize;
  definitionStart = (namesStart + namesSize);
  offset = uint16Decode(data, offset, &definitionLgth);
  checkBadOffset(offset);
  offset = uint8Decode(data, offset, &nameLgth);
  checkBadOffset(offset);
  name = data+offset;
  offset += nameLgth;
  offset = uint8Decode(data, offset, &numFields);
  checkBadOffset(offset);
  result = structmsg_createStructmsgDefinition (name, numFields, shortCmdKey);
  checkOffsetErrOK(result);
  definitionIdx = 0;
  namesIdx = 0;
  while (definitionIdx < numFields) { 
    offset = uint16Decode(data, offset, &fieldId);
    checkBadOffset(offset);
    if (fieldId > STRUCT_MSG_SPEC_FIELD_LOW) {
      result = structmsg_getIdFieldNameStr(fieldId, &fieldName);
      checkOffsetErrOK(result);
    } else {
      fieldId = namesIdx + 1;
      myOffset = ((uint8_t *)idsStart - data) + (namesIdx * sizeof(uint16_t));
      myOffset = uint16Decode(data, myOffset, &nameOffset);
      fieldName = (uint8_t *)(namesStart+nameOffset);
      namesIdx++;
    }
    offset = uint8Decode(data, offset, &fieldTypeId);
    checkBadOffset(offset);
    result = structmsg_getFieldTypeStr(fieldTypeId, &fieldType);
    checkOffsetErrOK(result);
    offset = uint16Decode(data, offset, &fieldLgth);
    checkBadOffset(offset);
//ets_printf("add field: %s fieldId: %d fieldType: %d  %s fieldLgth: %d offset: %d\n", fieldName, fieldId, fieldTypeId, fieldType, fieldLgth, offset);  
    result = structmsg_addFieldDefinition (name, fieldName, fieldType, fieldLgth);
    checkOffsetErrOK(result);
    definitionIdx++;
  }
  return offset;
}

// ============================= getSpecFieldSizes ========================

static int getSpecFieldSizes(size_t *numFields, size_t *namesSize) {
  *numFields = 0;
  *namesSize = 0;
  str2key_t *entry = &structmsgSpecialFieldNames[0];
  while (entry->str != NULL) {
    (*numFields)++;
    *namesSize += c_strlen(entry->str) + 1;  // +1 for "," as separator
//ets_printf("%s: %d %d\n", entry->str, *numFields, *namesSize);
    entry++;
  }
  return STRUCT_MSG_ERR_OK;
}

// ============================= structmsg_encodeDefinition ========================

int structmsg_encodeDefinition (const uint8_t *name, uint8_t **data, int *lgth, stmsgDefinitions_t *structmsgDefinitions, fieldNameDefinitions_t *fieldNameDefinitions) {
  size_t numSpecFields;
  size_t namesSpecSize;
  size_t numNormFields;
  size_t normNamesSize;
  uint16_t totalLgth;
  uint16_t cmdLgth;
  uint16_t cmdKey = STRUCT_MSG_DEFINITION_CMD_KEY;
  size_t fillerSize;
  int result;
  int idx;
  int found = 0;
  uint8_t definitionsIdx;
  uint8_t *fieldName;
  size_t definitionPayloadSize;
  size_t payloadSize;
  size_t myLgth;
  uint32_t randomNum;
  stmsgDefinition_t *definition;
  fieldInfoDefinition_t *fieldInfo;
  uint8_t *encoded;
  int offset;
  uint16_t crc;
  id2offset_t *normNamesOffsets;
  // FIXME!!
  uint16_t src = 123;
  uint16_t dst = 987;

  result =  structmsg_getDefinitionPtr(name, &definition, &definitionsIdx);
  checkErrOK(result);
  numNormFields = 0;
  normNamesSize = 0;
  idx = 0;
  while (idx < definition->numFields) {
    fieldInfo = &definition->fieldInfos[idx];
    if (fieldInfo->fieldId < STRUCT_MSG_SPEC_FIELD_LOW) {
      result = getFieldIdName(fieldInfo->fieldId, fieldNameDefinitions, &fieldName);
      checkErrOK(result);
//ets_printf("fieldName: %s\n", fieldName);
      numNormFields++;
      normNamesSize += c_strlen(fieldName) + 1;
    }
    idx++;
  }
  normNamesOffsets = os_zalloc(numNormFields * sizeof(id2offset_t) + 1);
  checkAllocOK(normNamesOffsets);
  if (definition->flags & STRUCT_MSG_SHORT_CMD_KEY) {
    payloadSize = STRUCT_MSG_SHORT_CMD_HEADER_LENGTH; // cmdKey + cmdLgth
  } else {
    payloadSize = STRUCT_MSG_CMD_HEADER_LENGTH; // cmdKey + cmdLgth
  }
  // randomNum
  payloadSize += sizeof(uint32_t);
  // len ids + ids (numNormFields * (uint16_t)) + len Names + names size
  payloadSize += sizeof(uint8_t) + (numNormFields * sizeof(uint16_t)) + sizeof(uint16_t) + normNamesSize;
  // definitionPayloadSize

  // definitionLgth + nameLgth + name of Definition
  definitionPayloadSize = sizeof(uint16_t) + sizeof(uint8_t) + (c_strlen(name) + 1);
  // numFields (uint8_t) + numFields * (fieldId uint16_t, fieldType uint8_t, fieldLgth uint16_t)
  definitionPayloadSize += sizeof(uint8_t) + definition->numFields * (sizeof(uint16_t) + sizeof(uint8_t) + sizeof(uint16_t));

  payloadSize += definitionPayloadSize;
  fillerSize = 0;
  myLgth = payloadSize + sizeof(uint16_t); // sizeof(uint16_t) for CRC
  while ((myLgth % 16) != 0) {
    myLgth++;
    fillerSize++;
  }
  cmdLgth = payloadSize + fillerSize + sizeof(uint16_t);
  totalLgth = STRUCT_MSG_HEADER_LENGTH + cmdLgth;
  definition->totalLgth = totalLgth;
  definition->encoded = os_zalloc(totalLgth);
  checkAllocOK(definition->encoded);
  encoded = definition->encoded;
  offset = 0;
  offset = uint16Encode(encoded, offset, src); 
  checkEncodeOffset(offset);
  offset = uint16Encode(encoded, offset, dst); 
  checkEncodeOffset(offset);
  offset = uint16Encode(encoded, offset, totalLgth); 
  checkEncodeOffset(offset);
  offset = uint16Encode(encoded, offset, cmdKey); 
  checkEncodeOffset(offset);
  offset = uint16Encode(encoded, offset, cmdLgth); 
  checkEncodeOffset(offset);
  offset = randomNumEncode(encoded, offset, &randomNum); 
  checkEncodeOffset(offset);
  offset = normalFieldNamesEncode(encoded, offset, definition, fieldNameDefinitions, &normNamesOffsets, numNormFields, normNamesSize);
  offset = uint16Encode(encoded, offset, definitionPayloadSize); 
  checkEncodeOffset(offset);
  offset = uint8Encode(encoded, offset, c_strlen(name)+1); 
  checkEncodeOffset(offset);
  offset = uint8VectorEncode(encoded, offset, name, c_strlen(name)); 
  checkEncodeOffset(offset);
  offset = uint8Encode(encoded, offset, '\0'); 
  checkEncodeOffset(offset);
  offset = definitionEncode(encoded, offset, definition, fieldNameDefinitions, normNamesOffsets);
  checkEncodeOffset(offset);
  os_free(normNamesOffsets); // no longer needed
  uint8_t dummy[fillerSize];
  offset = fillerEncode(encoded, offset, fillerSize, &dummy[0]);
  checkEncodeOffset(offset);
  offset = crcEncode(encoded, offset, totalLgth, &crc, STRUCT_MSG_HEADER_LENGTH,0);
  checkEncodeOffset(offset);
ets_printf("after crc offset: %d totalLgth :%d crc: 0x%04x\n", offset, totalLgth, crc);
  *data = encoded;
  *lgth = totalLgth;
  return STRUCT_MSG_ERR_OK;

}

// ============================= structmsg_decodeDefinition ========================

int structmsg_decodeDefinition (const uint8_t *name, const uint8_t *data, stmsgDefinitions_t *structmsgDefinitions, fieldNameDefinitions_t *fieldNameDefinitions, uint8_t shortCmdKey) {
  size_t numSpecFields;
  size_t namesSpecSize;
  size_t numNormFields;
  size_t namesNormSize;
  size_t fillerSize;
  int result;
  int idx;
  int found = 0;
  uint8_t *fieldName;
  size_t definitionPayloadSize;
  size_t payloadSize;
  size_t myLgth;
  stmsgDefinition_t *definition;
  fieldInfoDefinition_t *fieldInfo;
  int offset;
  int crcOffset;
  uint16_t src;
  uint16_t dst;
  uint16_t totalLgth;
  uint16_t cmdKey;
  uint16_t cmdLgth;
  uint16_t crc;
  uint32_t randomNum;
  uint8_t fillerStr[16];
  uint8_t *filler = fillerStr;;
  uint8_t numEntries;

  offset = 0;
  offset = uint16Decode(data, offset, &src); 
  checkDecodeOffset(offset);
  offset = uint16Decode(data, offset, &dst); 
  checkDecodeOffset(offset);
  offset = uint16Decode(data, offset, &totalLgth); 
  checkDecodeOffset(offset);
  offset = uint16Decode(data, offset, &cmdKey); 
  checkDecodeOffset(offset);
  if (cmdKey != STRUCT_MSG_DEFINITION_CMD_KEY) {
    return STRUCT_MSG_ERR_BAD_DEFINTION_CMD_KEY;
  }
  offset = uint16Decode(data, offset, &cmdLgth); 
  checkDecodeOffset(offset);
  offset = uint32Decode(data, offset, &randomNum); 
  checkDecodeOffset(offset);
  // now check the crc
  crcOffset = totalLgth - sizeof(uint16_t);
  crcOffset = crcDecode(data, crcOffset, cmdLgth, &crc, STRUCT_MSG_HEADER_LENGTH, 0);
  offset = definitionDecode(data, offset, definition, fieldNameDefinitions, shortCmdKey);
  checkDecodeOffset(offset);
  myLgth = offset + sizeof(uint16_t);
  fillerSize = 0;
  while ((myLgth % 16) != 0) {
    myLgth++;
    fillerSize++;
  }
  offset = fillerDecode(data, offset, fillerSize, &filler);
  checkDecodeOffset(offset);
  return STRUCT_MSG_ERR_OK;
}

// ============================= stmsg_getDefinitionName ========================

int stmsg_getDefinitionName(uint8_t *decrypted, uint8_t **name, uint8_t shortCmdKey) {
  int nameOffset;
  uint8_t numNormFields;
  uint16_t normNamesSize;
  uint8_t nameLgth;
  uint8_t numEntries;

  *name = NULL;
  nameOffset = STRUCT_MSG_HEADER_LENGTH; // src + dst + totalLgth
  if (shortCmdKey) {
    nameOffset += STRUCT_MSG_SHORT_CMD_HEADER_LENGTH; // cmdKey + cmdLgth
  } else {
    nameOffset += STRUCT_MSG_CMD_HEADER_LENGTH; // cmdKey + cmdLgth
  }
  // randomNum
  nameOffset += sizeof(uint32_t);
  // len ids 
  nameOffset = uint8Decode(decrypted, nameOffset, &numNormFields);
  // ids vector
  nameOffset += numNormFields * sizeof(uint16_t);
  // size of name strings (normnamesSize)
  nameOffset = uint16Decode(decrypted, nameOffset, &normNamesSize);
  // names vector
  nameOffset += normNamesSize; 
  // definitionSize + nameLgth
  nameOffset += sizeof(uint16_t) + sizeof(uint8_t);
  // here the name starts
  *name = decrypted + nameOffset;
  return STRUCT_MSG_ERR_OK;
}

#endif
