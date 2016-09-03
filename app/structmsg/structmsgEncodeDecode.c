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
#include "../crypto/mech.h"

extern str2key_t structmsgSpecialFieldNames[];

// ============================= uint8Encode ========================

static int uint8Encode(uint8_t *data, int offset, uint8_t value) {
  data[offset++] = value & 0xFF;
  return offset;
}

// ============================= int8Encode ========================

static int int8Encode(uint8_t *data, int offset, int8_t value) {
  data[offset++] = value & 0xFF;
  return offset;
}

// ============================= uint16Encode ========================

static int uint16Encode(uint8_t *data, int offset, uint16_t value) {
  data[offset++] = (value >> 8) & 0xFF;
  data[offset++] = value & 0xFF;
  return offset;
}

// ============================= int16Encode ========================

static int int16Encode(uint8_t *data, int offset, int16_t value) {
  data[offset++] = (value >> 8) & 0xFF;
  data[offset++] = value & 0xFF;
  return offset;
}

// ============================= uint32Encode ========================

static int uint32Encode(uint8_t *data, int offset, uint32_t value) {
  data[offset++] = (value >> 24) & 0xFF;
  data[offset++] = (value >> 16) & 0xFF;
  data[offset++] = (value >> 8) & 0xFF;
  data[offset++] = value & 0xFF;
  return offset;
}

// ============================= int32Encode ========================

static int int32Encode(uint8_t *data, int offset, int32_t value) {
  data[offset++] = (value >> 24) & 0xFF;
  data[offset++] = (value >> 16) & 0xFF;
  data[offset++] = (value >> 8) & 0xFF;
  data[offset++] = value & 0xFF;
  return offset;
}

// ============================= uint8VectorEncode ========================

static int uint8VectorEncode(uint8_t *data, int offset, const uint8_t *value, uint16_t lgth) {
  c_memcpy(data+offset,value,lgth);
  offset += lgth;
  return offset;
}

// ============================= int8VectorEncode ========================

static int int8VectorEncode(uint8_t *data, int offset, int8_t *value, uint16_t lgth) {
  c_memcpy(data+offset,value,lgth);
  offset += lgth;
  return offset;
}

// ============================= uint16VectorEncode ========================

static int uint16VectorEncode(uint8_t *data, int offset, uint16_t *value, uint16_t lgth) {
  int idx;

  idx = 0;
  while (idx < lgth) {
    offset = uint16Encode(data, offset, value[idx]);
    idx++;
  }
  return offset;
}

// ============================= int16VectorEncode ========================

static int int16VectorEncode(uint8_t *data, int offset, int16_t *value, uint16_t lgth) {
  int idx;

  idx = 0;
  while (idx < lgth) {
    offset = int16Encode(data, offset, value[idx]);
    idx++;
  }
  return offset;
}

// ============================= uint32VectorEncode ========================

static int uint32VectorEncode(uint8_t *data, int offset, uint32_t *value, uint16_t lgth) {
  int idx;

  idx = 0;
  while (idx < lgth) {
    offset = uint32Encode(data, offset, value[idx]);
    idx++;
  }
  return offset;
}

// ============================= int32VectorEncode ========================

static int int32VectorEncode(uint8_t *data, int offset, int32_t *value, uint16_t lgth) {
  int idx;

  idx = 0;
  while (idx < lgth) {
    offset = int32Encode(data, offset, value[idx]);
    idx++;
  }
  return offset;
}

// ============================= uint8Decode ========================

static int uint8Decode(const uint8_t *data, int offset, uint8_t *value) {
  *value = data[offset++] & 0xFF;
  return offset;
}

// ============================= int8Decode ========================

static int int8Decode(const uint8_t *data, int offset, int8_t *value) {
  *value = data[offset++] & 0xFF;
  return offset;
}

// ============================= uint16Decode ========================

static int uint16Decode(const uint8_t *data, int offset, uint16_t *value) {
  *value = 0;
  *value += (data[offset++] & 0xFF) << 8;
  *value += (data[offset++] & 0xFF) << 0;
  return offset;
}

// ============================= int16Decode ========================

static int int16Decode(const uint8_t *data, int offset, int16_t *value) {
  *value = 0;
  *value += (data[offset++] & 0xFF) << 8;
  *value += (data[offset++] & 0xFF) << 0;
  return offset;
}

// ============================= uint32Decode ========================

static int uint32Decode(const uint8_t *data, int offset, uint32_t *value) {
  *value = 0;
  *value += (data[offset++] & 0xFF) << 24;
  *value += (data[offset++] & 0xFF) << 16;
  *value += (data[offset++] & 0xFF) << 8;
  *value += (data[offset++] & 0xFF) << 0;
  return offset;
}

// ============================= int32Decode ========================

static int int32Decode(const uint8_t *data, int offset, int32_t *value) {
  *value = 0;
  *value += (data[offset++] & 0xFF) << 24;
  *value += (data[offset++] & 0xFF) << 16;
  *value += (data[offset++] & 0xFF) << 8;
  *value += (data[offset++] & 0xFF) << 0;
  return offset;
}

// ============================= uint8VectorDecode ========================

static int uint8VectorDecode(const uint8_t *data, int offset, uint16_t lgth, uint8_t **value) {
  c_memcpy(*value,data+offset,lgth);
  offset += lgth;
  return offset;
}

// ============================= int8VectorDecode ========================

static int int8VectorDecode(const uint8_t *data, int offset, uint16_t lgth, int8_t **value) {
  c_memcpy(*value,data+offset,lgth);
  offset += lgth;
  return offset;
}

// ============================= uint16VectorDecode ========================

static int uint16VectorDecode(const uint8_t *data, int offset, uint16_t lgth, uint16_t **value) {
  int idx;

  idx = 0;
  while (idx < lgth) {
    offset = uint16Decode(data, offset, &((*value)[idx]));
    idx++;
  }
  return offset;
}

// ============================= int16VectorDecode ========================

static int int16VectorDecode(const uint8_t *data, int offset, uint16_t lgth, int16_t **value) {
  int idx;

  idx = 0;
  while (idx < lgth) {
    offset = int16Decode(data, offset, &((*value)[idx]));
    idx++;
  }
  return offset;
}

// ============================= uint32VectorDecode ========================

static int uint32VectorDecode(const uint8_t *data, int offset, uint16_t lgth, uint32_t **value) {
  int idx;

  idx = 0;
  while (idx < lgth) {
    offset = uint32Decode(data, offset, &((*value)[idx]));
    idx++;
  }
  return offset;
}

// ============================= int32VectorDecode ========================

static int int32VectorDecode(const uint8_t *data, int offset, uint16_t lgth, int32_t **value) {
  int idx;

  idx = 0;
  while (idx < lgth) {
    offset = int32Decode(data, offset, &((*value)[idx]));
    idx++;
  }
  return offset;
}

// ============================= randomNumEncode ========================

static int randomNumEncode(uint8_t *data, int offset, uint32_t *value) {
  uint32_t val;
  
  val = (uint32_t)(rand() & RAND_MAX);
  *value = val;
  return uint32Encode(data, offset, val);
}

// ============================= randomNumDecode ========================

static int randomNumDecode(const uint8_t *data, int offset, uint32_t *value) {
  int result;

  offset = uint32Decode(data, offset, value);
  return offset;
}

// ============================= sequenceNumEncode ========================

static int sequenceNumEncode(uint8_t *data, int offset, structmsg_t *structmsg, uint32_t *value) {
  uint32_t val;
  
  val = ++structmsg->sequenceNum;
  *value = val;
  return uint32Encode(data, offset, val);
}

// ============================= sequenceNumDecode ========================

static int sequenceNumDecode(const uint8_t *data, int offset, uint32_t *value) {
  int result;

  offset = uint32Decode(data, offset, value);
  return offset;
}

// ============================= fillerEncode ========================

static int fillerEncode(uint8_t *data, int offset, uint16_t lgth, uint8_t *value) {
  uint32_t val;
  int idx;

  idx = 0;
  while (lgth >= 4) {
    val = (uint32_t)(rand() & RAND_MAX);
    value[idx++] = (val >> 24) & 0xFF;
    value[idx++] = (val >> 16) & 0xFF;
    value[idx++] = (val >> 8) & 0xFF;
    value[idx++] = (val >> 0) & 0xFF;
    offset = uint32Encode(data, offset, val);
    lgth -= 4;
  }
  while (lgth >= 2) {
    val = (uint16_t)((rand() & RAND_MAX) & 0xFFFF);
    value[idx++] = (val >> 8) & 0xFF;
    value[idx++] = (val >> 0) & 0xFF;
    offset = uint16Encode(data, offset, val);
    lgth -= 2;
  }
  while (lgth >= 1) {
    val = (uint8_t)((rand() & RAND_MAX) & 0xFF);
    offset = uint8Encode(data, offset, val);
    value[idx++] = (val >> 0) & 0xFF;
    lgth -= 1;
  }
  return offset;
}

// ============================= fillerDecode ========================

static int fillerDecode(const uint8_t *data, int offset, uint16_t lgth, uint8_t **value) {
  int idx;

  c_memcpy(*value,data+offset,lgth);
  offset += lgth;
  return offset;
}

// ============================= crcEncode ========================

static int crcEncode(uint8_t *data, int offset, uint16_t lgth, uint16_t *crc, uint8_t headerLgth) {
  int idx;

  lgth -= sizeof(uint16_t);   // uint16_t crc
  *crc = 0;
  idx = headerLgth;
  while (idx < lgth) {
//ets_printf("crc idx: %d ch: 0x%02x crc: 0x%04x\n", idx-headerLgth, data[idx], *crc);
    *crc += data[idx++];
  }
  *crc = ~(*crc);
  offset = uint16Encode(data,offset,*crc);
  return offset;
}

// ============================= crcDecode ========================

static int crcDecode(const uint8_t *data, int offset, uint16_t lgth, uint16_t *crc, uint8_t headerLgth) {
  uint16_t crcVal;
  int idx;

  lgth -= sizeof(uint16_t);   // uint16_t crc
  crcVal = 0;
  idx = headerLgth;
  while (idx < lgth + headerLgth) {
//ets_printf("crc idx: %d ch: 0x%02x crc: 0x%04x\n", idx-headerLgth, data[idx], crcVal);
    crcVal += data[idx++];
  }
  crcVal = ~crcVal;
  offset = uint16Decode(data, offset, crc);
ets_printf("crcVal: 0x%04x crc: 0x%04x\n", crcVal, *crc);
  if (crcVal != *crc) {
    return -1;
  }
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

static int definitionDecode(const uint8_t *data, int offset, stmsgDefinition_t *definition, fieldNameDefinitions_t *fieldNameDefinitions) {
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
  result = structmsg_createStructmsgDefinition (name, numFields);
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

// ============================= structmsg_fillHdrInfo ========================

int structmsg_fillHdrInfo(const uint8_t *handle, structmsg_t *structmsg) {
  hdrInfo_t *hdrInfoPtr;
  int offset;

  // fill the hdrInfo
  structmsg = structmsg_get_structmsg_ptr(handle);
  hdrInfoPtr = structmsg->handleHdrInfoPtr;
  *hdrInfoPtr = *(&structmsg->hdr.hdrInfo);
  offset = 0;
  offset = uint16Encode(hdrInfoPtr->hdrId, offset, structmsg->hdr.hdrInfo.hdrKeys.src);
  checkEncodeOffset(offset);
  offset = uint16Encode(hdrInfoPtr->hdrId, offset, structmsg->hdr.hdrInfo.hdrKeys.dst);
  checkEncodeOffset(offset);
  offset = uint16Encode(hdrInfoPtr->hdrId, offset, structmsg->hdr.hdrInfo.hdrKeys.totalLgth);
  checkEncodeOffset(offset);
  offset = uint16Encode(hdrInfoPtr->hdrId, offset, structmsg->hdr.hdrInfo.hdrKeys.cmdKey);
  checkEncodeOffset(offset);
  offset = uint16Encode(hdrInfoPtr->hdrId, offset, structmsg->hdr.hdrInfo.hdrKeys.cmdLgth);
  checkEncodeOffset(offset);
  return STRUCT_MSG_ERR_OK;
}

// ============================= encodeField ========================

static int encodeField(uint8_t *msgPtr, fieldInfo_t *fieldInfo, int offset) {
  int fieldIdx;

  switch (fieldInfo->fieldType) {
    case STRUCT_MSG_FIELD_INT8_T:
      offset = int8Encode(msgPtr,offset,fieldInfo->value.byteVal);
      break;
    case STRUCT_MSG_FIELD_UINT8_T:
      offset = uint8Encode(msgPtr,offset,fieldInfo->value.ubyteVal);
      break;
    case STRUCT_MSG_FIELD_INT16_T:
      offset = int16Encode(msgPtr,offset,fieldInfo->value.shortVal);
      break;
    case STRUCT_MSG_FIELD_UINT16_T:
      offset = uint16Encode(msgPtr,offset,fieldInfo->value.ushortVal);
      break;
    case STRUCT_MSG_FIELD_INT32_T:
      offset = int32Encode(msgPtr,offset,fieldInfo->value.val);
      break;
    case STRUCT_MSG_FIELD_UINT32_T:
      offset = uint32Encode(msgPtr,offset,fieldInfo->value.uval);
      break;
    case STRUCT_MSG_FIELD_INT8_VECTOR:
      fieldIdx = 0;
      while (fieldIdx < fieldInfo->fieldLgth) {
        offset = int8Encode(msgPtr,offset,fieldInfo->value.byteVector[fieldIdx]);
        fieldIdx++;
      }
      break;
    case STRUCT_MSG_FIELD_UINT8_VECTOR:
      fieldIdx = 0;
      while (fieldIdx < fieldInfo->fieldLgth) {
        offset = uint8Encode(msgPtr,offset,fieldInfo->value.ubyteVector[fieldIdx]);
        fieldIdx++;
      }
      break;
    case STRUCT_MSG_FIELD_INT16_VECTOR:
      fieldIdx = 0;
      while (fieldIdx < fieldInfo->fieldLgth) {
        offset = int16Encode(msgPtr,offset,fieldInfo->value.shortVector[fieldIdx]);
        fieldIdx++;
      }
      break;
    case STRUCT_MSG_FIELD_UINT16_VECTOR:
      fieldIdx = 0;
      while (fieldIdx < fieldInfo->fieldLgth) {
        offset = uint16Encode(msgPtr,offset,fieldInfo->value.ushortVector[fieldIdx]);
        fieldIdx++;
      }
      break;
    case STRUCT_MSG_FIELD_INT32_VECTOR:
      fieldIdx = 0;
      while (fieldIdx < fieldInfo->fieldLgth) {
        offset = int32Encode(msgPtr,offset,fieldInfo->value.int32Vector[fieldIdx]);
        fieldIdx++;
      }
      break;
    case STRUCT_MSG_FIELD_UINT32_VECTOR:
      fieldIdx = 0;
      while (fieldIdx < fieldInfo->fieldLgth) {
        offset = uint32Encode(msgPtr,offset,fieldInfo->value.uint32Vector[fieldIdx]);
        fieldIdx++;
      }
      break;
  }
  return offset;
}

// ============================= decodeField ========================

static int decodeField(const uint8_t *msgPtr, fieldInfo_t *fieldInfo, int offset) {
  int fieldIdx;

  switch (fieldInfo->fieldType) {
    case STRUCT_MSG_FIELD_INT8_T:
      offset = int8Decode(msgPtr,offset,&fieldInfo->value.byteVal);
      checkDecodeOffset(offset);
      break;
    case STRUCT_MSG_FIELD_UINT8_T:
      offset = uint8Decode(msgPtr,offset,&fieldInfo->value.ubyteVal);
      checkDecodeOffset(offset);
      break;
    case STRUCT_MSG_FIELD_INT16_T:
      offset = int16Decode(msgPtr,offset,&fieldInfo->value.shortVal);
      checkDecodeOffset(offset);
      break;
    case STRUCT_MSG_FIELD_UINT16_T:
      offset = uint16Decode(msgPtr,offset,&fieldInfo->value.ushortVal);
      checkDecodeOffset(offset);
      break;
    case STRUCT_MSG_FIELD_INT32_T:
      offset = int32Decode(msgPtr,offset,&fieldInfo->value.val);
      checkDecodeOffset(offset);
      break;
    case STRUCT_MSG_FIELD_UINT32_T:
      offset = uint32Decode(msgPtr,offset,&fieldInfo->value.uval);
      checkDecodeOffset(offset);
      break;
    case STRUCT_MSG_FIELD_INT8_VECTOR:
      offset = int8VectorDecode(msgPtr,offset,fieldInfo->fieldLgth, &fieldInfo->value.byteVector);
      checkDecodeOffset(offset);
      break;
    case STRUCT_MSG_FIELD_UINT8_VECTOR:
      offset = uint8VectorDecode(msgPtr,offset,fieldInfo->fieldLgth, &fieldInfo->value.ubyteVector);
      checkDecodeOffset(offset);
      break;
    case STRUCT_MSG_FIELD_INT16_VECTOR:
      offset = int16VectorDecode(msgPtr,offset,fieldInfo->fieldLgth, &fieldInfo->value.shortVector);
      checkDecodeOffset(offset);
      break;
    case STRUCT_MSG_FIELD_UINT16_VECTOR:
      offset = uint16VectorDecode(msgPtr,offset,fieldInfo->fieldLgth, &fieldInfo->value.ushortVector);
      checkDecodeOffset(offset);
      break;
    case STRUCT_MSG_FIELD_INT32_VECTOR:
      offset = int32VectorDecode(msgPtr,offset,fieldInfo->fieldLgth, &fieldInfo->value.int32Vector);
      checkDecodeOffset(offset);
      break;
    case STRUCT_MSG_FIELD_UINT32_VECTOR:
      offset = uint32VectorDecode(msgPtr,offset,fieldInfo->fieldLgth, &fieldInfo->value.uint32Vector);
      checkDecodeOffset(offset);
      break;
  }
  return offset;
}

// ============================= stmsg_encodeMsg ========================

int stmsg_encodeMsg(const uint8_t *handle) {
  structmsg_t *structmsg;
  uint8_t *msgPtr;
  uint16_t *ushortPtr;
  int16_t *shortPtr;
  uint32_t *uintPtr;
  int32_t *intPtr;
  int32_t randomNum;
  int32_t sequenceNum;
  uint16_t crc;
  int offset;
  int idx;
  int fieldIdx;
  int numEntries;
  int result;
  int fieldId = 0;
  fieldInfo_t *fieldInfo;

  structmsg = structmsg_get_structmsg_ptr(handle);
  checkHandleOK(structmsg);
  if (structmsg->encoded != NULL) {
    os_free(structmsg->encoded);
  }
  structmsg->encoded = (uint8_t *)os_zalloc(structmsg->hdr.hdrInfo.hdrKeys.totalLgth);
  msgPtr = structmsg->encoded;
  offset = 0;
  offset = uint16Encode(msgPtr,offset,structmsg->hdr.hdrInfo.hdrKeys.src);
  checkEncodeOffset(offset);
  offset = uint16Encode(msgPtr,offset,structmsg->hdr.hdrInfo.hdrKeys.dst);
  checkEncodeOffset(offset);
  offset = uint16Encode(msgPtr,offset,structmsg->hdr.hdrInfo.hdrKeys.totalLgth);
  checkEncodeOffset(offset);
  offset = uint16Encode(msgPtr,offset,structmsg->hdr.hdrInfo.hdrKeys.cmdKey);
  checkEncodeOffset(offset);
  offset = uint16Encode(msgPtr,offset,structmsg->hdr.hdrInfo.hdrKeys.cmdLgth);
  checkEncodeOffset(offset);
  numEntries = structmsg->msg.numFieldInfos;
  offset = uint8Encode(msgPtr,offset,numEntries);
  checkEncodeOffset(offset);
  idx = 0;
  while (idx < numEntries) {
    fieldInfo = &structmsg->msg.fieldInfos[idx];
    if (fieldInfo->fieldStr[0] == '@') {
      result = structmsg_getFieldNameId(fieldInfo->fieldStr, &fieldId, STRUCT_MSG_NO_INCR);
      checkErrOK(result);
      switch (fieldId) {
      case STRUCT_MSG_SPEC_FIELD_SRC:
      case STRUCT_MSG_SPEC_FIELD_DST:
      case STRUCT_MSG_SPEC_FIELD_TARGET_CMD:
      case STRUCT_MSG_SPEC_FIELD_TOTAL_LGTH:
      case STRUCT_MSG_SPEC_FIELD_CMD_KEY:
      case STRUCT_MSG_SPEC_FIELD_CMD_LGTH:
ets_printf("funny should encode: %s\n", fieldInfo->fieldStr);
        break;
      case STRUCT_MSG_SPEC_FIELD_RANDOM_NUM:
        offset = randomNumEncode(msgPtr, offset, &randomNum);
        checkEncodeOffset(offset);
        result = stmsg_setFieldValue(handle, "@randomNum", randomNum, NULL);
        checkErrOK(result);
        break;
      case STRUCT_MSG_SPEC_FIELD_SEQUENCE_NUM:
        offset = sequenceNumEncode(msgPtr, offset, structmsg, &sequenceNum);
        checkEncodeOffset(offset);
        result = stmsg_setFieldValue(handle, "@sequenceNum", sequenceNum, NULL);
        checkErrOK(result);
        break;
      case STRUCT_MSG_SPEC_FIELD_FILLER:
        offset = fillerEncode(msgPtr, offset, fieldInfo->fieldLgth, fieldInfo->value.ubyteVector);
        checkEncodeOffset(offset);
        result = stmsg_setFieldValue(handle, "@filler", 0, fieldInfo->value.ubyteVector);
        checkErrOK(result);
        break;
      case STRUCT_MSG_SPEC_FIELD_CRC:
        offset = crcEncode(structmsg->encoded, offset, structmsg->hdr.hdrInfo.hdrKeys.totalLgth, &crc, structmsg->hdr.headerLgth);
        checkEncodeOffset(offset);
        result = stmsg_setFieldValue(handle, "@crc", crc, NULL);
        checkErrOK(result);
        break;
      case STRUCT_MSG_SPEC_FIELD_ID:
        return STRUCT_MSG_ERR_BAD_SPECIAL_FIELD;
        break;
      case STRUCT_MSG_SPEC_FIELD_TABLE_ROWS:
        break;
      case STRUCT_MSG_SPEC_FIELD_TABLE_ROW_FIELDS:
        if (structmsg->msg.numTableRows > 0) {
          int row = 0;
          int col = 0;
          int cell = 0;
          while (row < structmsg->msg.numTableRows) {
            while (col < structmsg->msg.numRowFields) {
               cell = col + row * structmsg->msg.numRowFields;
               fieldInfo = &structmsg->msg.tableFieldInfos[cell];
               offset = encodeField(msgPtr, fieldInfo, offset);
               checkEncodeOffset(offset);
               col++;
            }
            row++;
            col = 0;
          }
        }
        break;
      }
    } else {
      offset = encodeField(msgPtr, fieldInfo, offset);
      checkEncodeOffset(offset);
    }
    idx++;
  }
  structmsg->flags |= STRUCT_MSG_ENCODED;
  return STRUCT_MSG_ERR_OK;
}

// ============================= stmsg_getEncoded ========================

int stmsg_getEncoded(const uint8_t *handle, uint8_t ** encoded, int *lgth) {
  structmsg_t *structmsg;

  structmsg = structmsg_get_structmsg_ptr(handle);
  checkHandleOK(structmsg);
  if (structmsg->encoded == NULL) {
    return STRUCT_MSG_ERR_NOT_ENCODED;
  }
  *encoded = structmsg->encoded;
  *lgth = structmsg->hdr.hdrInfo.hdrKeys.totalLgth;
  return STRUCT_MSG_ERR_OK;
}

// ============================= stmsg_decodeMsg ========================

int stmsg_decodeMsg(const uint8_t *handle, const uint8_t *data) {
  structmsg_t *structmsg;
  const uint8_t *msgPtr;
  uint16_t crc;
  int offset;
  int idx;
  int fieldIdx;
  int fieldId;
  uint8_t numEntries;
  int result = STRUCT_MSG_ERR_OK;
  fieldInfo_t *fieldInfo;

  structmsg = structmsg_get_structmsg_ptr(handle);
  checkHandleOK(structmsg);
  if (structmsg->todecode != NULL) {
    os_free(structmsg->todecode);
  }
  structmsg->todecode = (uint8_t *)os_zalloc(structmsg->hdr.hdrInfo.hdrKeys.totalLgth);
  c_memcpy(structmsg->todecode, data, structmsg->hdr.hdrInfo.hdrKeys.totalLgth);
  msgPtr = structmsg->todecode;
  offset = 0;
  offset = uint16Decode(msgPtr,offset,&structmsg->hdr.hdrInfo.hdrKeys.src);
  checkDecodeOffset(offset);
  checkErrOK(result);
  offset = uint16Decode(msgPtr,offset,&structmsg->hdr.hdrInfo.hdrKeys.dst);
  checkDecodeOffset(offset);
  checkErrOK(result);
  offset = uint16Decode(msgPtr,offset,&structmsg->hdr.hdrInfo.hdrKeys.totalLgth);
  checkDecodeOffset(offset);
  checkErrOK(result);
  offset = uint16Decode(msgPtr,offset,&structmsg->hdr.hdrInfo.hdrKeys.cmdKey);
  checkDecodeOffset(offset);
  checkErrOK(result);
  offset = uint16Decode(msgPtr,offset,&structmsg->hdr.hdrInfo.hdrKeys.cmdLgth);
  checkDecodeOffset(offset);
  result = structmsg_fillHdrInfo(handle, structmsg);
  offset = uint8Decode(msgPtr,offset,&numEntries);
  checkDecodeOffset(offset);
  idx = 0;
//  numEntries = structmsg->msg.numFieldInfos;
  while (idx < numEntries) {
    fieldInfo = &structmsg->msg.fieldInfos[idx];
    if (fieldInfo->fieldStr[0] == '@') {
      result = structmsg_getFieldNameId(fieldInfo->fieldStr, &fieldId, STRUCT_MSG_NO_INCR);
      checkErrOK(result);
      switch (fieldId) {
      case STRUCT_MSG_SPEC_FIELD_SRC:
      case STRUCT_MSG_SPEC_FIELD_DST:
      case STRUCT_MSG_SPEC_FIELD_TARGET_CMD:
      case STRUCT_MSG_SPEC_FIELD_TOTAL_LGTH:
      case STRUCT_MSG_SPEC_FIELD_CMD_KEY:
      case STRUCT_MSG_SPEC_FIELD_CMD_LGTH:
ets_printf("funny should decode: %s\n", fieldInfo->fieldStr);
        break;
      case STRUCT_MSG_SPEC_FIELD_RANDOM_NUM:
        offset = randomNumDecode(msgPtr, offset, &fieldInfo->value.uval);
        checkDecodeOffset(offset);
        break;
      case STRUCT_MSG_SPEC_FIELD_SEQUENCE_NUM:
        offset = sequenceNumDecode(msgPtr, offset, &fieldInfo->value.uval);
        checkDecodeOffset(offset);
        break;
      case STRUCT_MSG_SPEC_FIELD_FILLER:
        offset = fillerDecode(msgPtr, offset, fieldInfo->fieldLgth, &fieldInfo->value.ubyteVector);
        checkDecodeOffset(offset);
        break;
      case STRUCT_MSG_SPEC_FIELD_CRC:
        offset = crcDecode(msgPtr, offset, structmsg->hdr.hdrInfo.hdrKeys.cmdLgth, &fieldInfo->value.ushortVal, structmsg->hdr.headerLgth);
        if (offset < 0) {
          return STRUCT_MSG_ERR_BAD_CRC_VALUE;
        }
        break;
      case STRUCT_MSG_SPEC_FIELD_ID:
        return STRUCT_MSG_ERR_BAD_SPECIAL_FIELD;
        break;
      case STRUCT_MSG_SPEC_FIELD_TABLE_ROWS:
        break;
      case STRUCT_MSG_SPEC_FIELD_TABLE_ROW_FIELDS:
        if (structmsg->msg.numTableRows > 0) {
          int row = 0;
          int col = 0;
          int cell = 0;
          while (row < structmsg->msg.numTableRows) {
            while (col < structmsg->msg.numRowFields) {
               cell = col + row * structmsg->msg.numRowFields;
               fieldInfo = &structmsg->msg.tableFieldInfos[cell];
               offset = decodeField(msgPtr, fieldInfo, offset);
               checkEncodeOffset(offset);
               col++;
            }
            row++;
            col = 0;
          }
        }
        break;
      }
      fieldInfo->flags |= STRUCT_MSG_FIELD_IS_SET;
    } else {
      offset = decodeField(msgPtr, fieldInfo, offset);
      checkEncodeOffset(offset);
      fieldInfo->flags |= STRUCT_MSG_FIELD_IS_SET;
    }
    idx++;
  }
  structmsg->flags |= STRUCT_MSG_DECODED;
  return STRUCT_MSG_ERR_OK;
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
  payloadSize = STRUCT_MSG_CMD_HEADER_LENGTH; // cmdKey + cmdLgth
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
  offset = crcEncode(encoded, offset, totalLgth, &crc, STRUCT_MSG_HEADER_LENGTH);
  checkEncodeOffset(offset);
ets_printf("after crc offset: %d totalLgth :%d crc: 0x%04x\n", offset, totalLgth, crc);
  *data = encoded;
  *lgth = totalLgth;
  return STRUCT_MSG_ERR_OK;

}

// ============================= structmsg_decodeDefinition ========================

int structmsg_decodeDefinition (const uint8_t *name, const uint8_t *data, stmsgDefinitions_t *structmsgDefinitions, fieldNameDefinitions_t *fieldNameDefinitions) {
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
  crcOffset = crcDecode(data, crcOffset, cmdLgth, &crc, STRUCT_MSG_HEADER_LENGTH);
  offset = definitionDecode(data, offset, definition, fieldNameDefinitions);
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

int stmsg_getDefinitionName(uint8_t *decrypted, uint8_t **name) {
  int nameOffset;
  uint8_t numNormFields;
  uint16_t normNamesSize;
  uint8_t nameLgth;
  uint8_t numEntries;

  *name = NULL;
  nameOffset = STRUCT_MSG_HEADER_LENGTH; // src + dst + totalLgth
  nameOffset += STRUCT_MSG_CMD_HEADER_LENGTH; // cmdKey + cmdLgth
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

