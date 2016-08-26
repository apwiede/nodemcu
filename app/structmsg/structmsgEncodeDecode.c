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

static int uint8VectorEncode(uint8_t *data, int offset, uint8_t *value, uint16_t lgth) {
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
ets_printf("sequenceNum: 0x%08x\n", val);
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
  idx = 0;
  numEntries = structmsg->msg.numFieldInfos;
  while (idx < numEntries) {
    fieldInfo = &structmsg->msg.fieldInfos[idx];
    if (fieldInfo->fieldStr[0] == '@') {
      result = structmsg_getFieldNameId(fieldInfo->fieldStr, &fieldId);
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
  int numEntries;
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
  idx = 0;
  numEntries = structmsg->msg.numFieldInfos;
  while (idx < numEntries) {
    fieldInfo = &structmsg->msg.fieldInfos[idx];
    if (fieldInfo->fieldStr[0] == '@') {
      result = structmsg_getFieldNameId(fieldInfo->fieldStr, &fieldId);
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

// ============================= getFieldIdName ========================

static int getFieldIdName (uint8_t key, fieldNameDefinitions_t *fieldNameDefinitions, uint8_t **fieldName) {
  // find field name
  int idx = 0;
  while (idx < fieldNameDefinitions->numDefinitions) {
    str2key_t *entry = &fieldNameDefinitions->definitions[idx];
    if (entry->key == key) {
       *fieldName = entry->str;
       return STRUCT_MSG_ERR_OK;
    }
    entry++;
    idx++;
  }
  return STRUCT_MSG_ERR_FIELD_NOT_FOUND;
}

// ============================= structmsg_encodeDefinitions ========================

int structmsg_encodeDefinitions (const uint8_t *name, stmsgDefinitions_t *structmsgDefinitions, fieldNameDefinitions_t *fieldNameDefinitions) {
  size_t numSpecFields;
  size_t namesSpecSize;
  size_t numNormFields;
  size_t namesNormSize;
  uint16_t totalLgth;
  uint16_t cmdLgth;
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
  uint8_t *encoded;
  int offset;

ets_printf("structmsgDefinitions: %p fieldNameDefinitions: %p\n", structmsgDefinitions, fieldNameDefinitions);
  result = getSpecFieldSizes(&numSpecFields, &namesSpecSize);
ets_printf("buildFieldDefMsg: numSpecFields: %d namesSpecSize: %d\n", numSpecFields, namesSpecSize);
  checkErrOK(result);
  idx = 0;
  while (idx < structmsgDefinitions->numDefinitions) {
    definition = &structmsgDefinitions->definitions[idx];
    if (c_strcmp(name, definition->name) == 0) {
      found = 1;
      break;
    }
    idx++;
  }
  if (!found) {
    return STRUCT_MSG_ERR_DEFINITION_NOT_FOUND;
  }
  numNormFields = 0;
  namesNormSize = 0;
  idx = 0;
  while (idx < definition->numFields) {
    fieldInfo = &definition->fieldInfos[idx];
    if (fieldInfo->fieldId < 200) { // FIXME should be a CONSTANT!! spec keys have Id's > 200
      result = getFieldIdName(fieldInfo->fieldId, fieldNameDefinitions, &fieldName);
      checkErrOK(result);
//ets_printf("fieldName: %s\n", fieldName);
      numNormFields++;
      namesNormSize += c_strlen(fieldName) + 1;
    }
    idx++;
  }
ets_printf("buildFieldDefMsg: numNormFields: %d namesNormSize: %d\n", numNormFields, namesNormSize);
  definitionPayloadSize = definition->numFields * sizeof(fieldInfoDefinition_t);
  payloadSize = STRUCT_MSG_CMD_HEADER_LENGTH; // cmdKey + cmdLgth
  payloadSize += sizeof(uint8_t) + numSpecFields + sizeof(uint8_t) + namesSpecSize;
  payloadSize += sizeof(uint8_t) + numNormFields + sizeof(uint16_t) + namesNormSize;
  payloadSize += sizeof(uint16_t) + definitionPayloadSize;
ets_printf("numFields: %d definitionPayloadSize: %d payloadSize: %d\n", definition->numFields, definitionPayloadSize, payloadSize);
  fillerSize = 0;
  myLgth = payloadSize + sizeof(uint16_t); // sizeof(uint16_t) for CRC
  while ((myLgth % 16) != 0) {
    myLgth++;
    fillerSize++;
  }
  cmdLgth = payloadSize + fillerSize + sizeof(uint16_t);
  totalLgth = STRUCT_MSG_HEADER_LENGTH + cmdLgth;
ets_printf("cmdLgth : %d totalLgth: %d\n", cmdLgth, totalLgth);
  definition->encoded = os_zalloc(totalLgth);
  checkAllocOK(definition->encoded);
  encoded = definition->encoded;
  // FIXME!!
  uint16_t src = 123;
  uint16_t dst = 987;

  offset = 0;
  offset = uint16Encode(encoded, offset, src); 
  offset = uint16Encode(encoded, offset, dst); 
  offset = uint16Encode(encoded, offset, totalLgth); 
structmsg_dumpBinary(encoded, 6, "ENCODED");
  return STRUCT_MSG_ERR_OK;

}
