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

#define checkEncodeOffset(val) if (val < 0) return STRUCT_MSG_ERR_ENCODE_ERROR
#define checkDecodeOffset(val) if (val < 0) return STRUCT_MSG_ERR_DECODE_ERROR

str2key_t structmsgFieldTypes[] = {
  {"uint8_t", STRUCT_MSG_FIELD_UINT8_T},
  {"int8_t", STRUCT_MSG_FIELD_INT8_T},
  {"uint16_t", STRUCT_MSG_FIELD_UINT16_T},
  {"int16_t", STRUCT_MSG_FIELD_INT16_T},
  {"uint32_t", STRUCT_MSG_FIELD_UINT32_T},
  {"int32_t", STRUCT_MSG_FIELD_INT32_T},
  {"uint8_t*", STRUCT_MSG_FIELD_UINT8_VECTOR},
  {"int8_t*", STRUCT_MSG_FIELD_INT8_VECTOR},
  {NULL, -1},
};

typedef struct lstructmsg_userdata
{
  uint8_t **handles;
  int numHandles;
} lstructmsg_userdata;

  // create a object
static lstructmsg_userdata structmsg_userdata = { NULL, 0};

// ============================= addHandle ========================

static int addHandle(uint8_t *handle) {
  int idx;

  if (structmsg_userdata.handles == NULL) {
    structmsg_userdata.handles = os_malloc(sizeof(uint8_t *));
ets_printf("os_malloc: %p %s %d\n", structmsg_userdata.handles, __FILE__, __LINE__);
    if (structmsg_userdata.handles == NULL) {
      return STRUCT_MSG_ERR_OUT_OF_MEMORY;
    } else {
      structmsg_userdata.handles[structmsg_userdata.numHandles++] = handle;
    }
  } else {
    // check for unused slot first
    idx = 0;
    while (idx < structmsg_userdata.numHandles) {
      if (structmsg_userdata.handles[idx] == NULL) {
        structmsg_userdata.handles[idx] = handle;
        return STRUCT_MSG_ERR_OK;
      }
      idx++;
    }
    structmsg_userdata.handles = os_realloc(structmsg_userdata.handles, sizeof(uint8_t *)*(structmsg_userdata.numHandles+1));
ets_printf("os_realloc: %p %s %d\n", structmsg_userdata.handles, __FILE__, __LINE__);
    if (structmsg_userdata.handles == NULL) {
      return STRUCT_MSG_ERR_OUT_OF_MEMORY;
    } else {
      structmsg_userdata.handles[structmsg_userdata.numHandles++] = handle;
    }
  }
  return STRUCT_MSG_ERR_OK;
}

// ============================= checkHandle ========================

static int checkHandle(const uint8_t *handle) {
  int idx;

  if (structmsg_userdata.handles == NULL) {
    return STRUCT_MSG_ERR_HANDLE_NOT_FOUND;
  }
  idx = 0;
  while (idx < structmsg_userdata.numHandles) {
    if ((structmsg_userdata.handles[idx] != NULL) && (c_strcmp(structmsg_userdata.handles[idx], handle) == 0)) {
      return STRUCT_MSG_ERR_OK;
    }
    idx++;
  }
  return STRUCT_MSG_ERR_HANDLE_NOT_FOUND;
}

// ============================= deleteHandle ========================

static int deleteHandle(const uint8_t *handle) {
  int idx;
  int numUsed;
  int found;

  if (structmsg_userdata.handles == NULL) {
    return STRUCT_MSG_ERR_HANDLE_NOT_FOUND;
  }
  found = 0;
  idx = 0;
  numUsed = 0;
  while (idx < structmsg_userdata.numHandles) {
    if ((structmsg_userdata.handles[idx] != NULL) && (c_strcmp(structmsg_userdata.handles[idx], handle) == 0)) {
      structmsg_userdata.handles[idx] = NULL;
      found++;
    } else {
      if (structmsg_userdata.handles[idx] != NULL) {
        numUsed++;
      }
    }
    idx++;
  }
  if (numUsed == 0) {
    os_free(structmsg_userdata.handles);
    structmsg_userdata.handles = NULL;
  }
  if (found) {
      return STRUCT_MSG_ERR_OK;
  }
  return STRUCT_MSG_ERR_HANDLE_NOT_FOUND;
}

// ============================= get_structmsg_ptr ========================

static structmsg_t *get_structmsg_ptr( const uint8_t *handle )
{
  int id;
  const char hex[] = "0123456789abcdef\0";
  int num;
  int len;
  int shift;
  const char *cp;
  int idx;

  if (checkHandle(handle) != STRUCT_MSG_ERR_OK) {
    return NULL;
  }
  handle += c_strlen(HANDLE_PREFIX); /* skip "sms_" */
  num = 0;
  id = 0;
  len = c_strlen(handle);
  shift = 28;
  while (num < len) {
    char ch = handle[num];
    cp = hex;
    idx = 0;
    while (*cp) {
      if (*cp == ch) {
          break;
      }
      cp++;
      idx++;
    }
    id = id + (idx<< shift);
    shift -= 4;
    num++;
  }
  return (structmsg_t *)id;
}

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

static int crcEncode(uint8_t *data, int offset, uint8_t *startData, uint16_t lgth, uint16_t *crc) {
  int idx;

  lgth -= sizeof(uint16_t);   // uint16_t crc
  *crc = 0;
  idx = sizeof(uint16_t) * 3; // uint16_t src + uint16_t dst + uint16_t totalLgth
  while (idx < lgth) {
    *crc += data[idx++];
  }
  *crc = ~(*crc);
  offset = uint16Encode(data,offset,*crc);
  return offset;
}

// ============================= crcDecode ========================

static int crcDecode(const uint8_t *data, int offset, uint8_t *startData, uint16_t lgth, uint16_t *crc) {
  uint16_t crcVal;
  int idx;

  lgth -= sizeof(uint16_t);   // uint16_t crc
  crcVal = 0;
  idx = sizeof(uint16_t) * 3; // uint16_t src + uint16_t dst + uint16_t totalLgth
  while (idx < lgth) {
    crcVal += data[idx++];
  }
  crcVal = ~crcVal;
  offset = uint16Decode(data, offset, crc);
  if (crcVal != *crc) {
    return -1;
  }
  return offset;
}

// ============================= newFieldInfos ========================

static fieldInfo_t *newFieldInfos(uint8_t numFieldInfos)
{
  fieldInfo_t *ptr = (fieldInfo_t *)os_malloc (sizeof(fieldInfo_t) * numFieldInfos);
ets_printf("os_malloc: %p %s %d\n", ptr, __FILE__, __LINE__);
  return ptr;
}

// ============================= getFieldTypeKey ========================

int getFieldTypeKey(const uint8_t *str) {
  str2key_t *entry = &structmsgFieldTypes[0];
  while (entry->str != NULL) {
    if (c_strcmp(entry->str, str) == 0) {
      return entry->key;
    }
    entry++;
  }
  return -1;
}

// ============================= getFieldTypeStr ========================

uint8_t *getFieldTypeStr(uint8_t key) {
  str2key_t *entry = &structmsgFieldTypes[0];
  while (entry->str != NULL) {
    if (entry->key == key) {
      return entry->str;
    }
    entry++;
  }
  return NULL;
}

// ============================= addField ========================

int addField(const uint8_t *handle, const uint8_t *fieldStr, uint8_t fieldType, int fieldLgth) {
  uint16_t fieldKey;
  structmsg_t *structmsg;

  structmsg = get_structmsg_ptr(handle);
  if (structmsg == NULL) {
    return STRUCT_MSG_ERR_BAD_HANDLE;
  }
  fieldKey = structmsg->msg.numFieldInfos;
  fieldInfo_t *fieldInfo = &structmsg->msg.fieldInfos[structmsg->msg.numFieldInfos];
  fieldInfo->fieldStr = os_malloc(os_strlen(fieldStr) + 1);
ets_printf("os_malloc: %p %s %d\n", fieldInfo->fieldStr, __FILE__, __LINE__);
  fieldInfo->fieldStr[os_strlen(fieldStr)] = '\0';
  os_memcpy(fieldInfo->fieldStr, fieldStr, os_strlen(fieldStr));
  fieldInfo->fieldKey = fieldKey;
  fieldInfo->fieldType = fieldType;
  fieldInfo->value.vector = NULL;
  fieldInfo->flags = 0;
  switch (fieldType) {
    case STRUCT_MSG_FIELD_UINT8_T:
    case STRUCT_MSG_FIELD_INT8_T:
      structmsg->totalLgth += 1;
      structmsg->msg.cmdLgth += 1;
      fieldLgth = 1;
      break;
    case STRUCT_MSG_FIELD_UINT16_T:
    case STRUCT_MSG_FIELD_INT16_T:
      structmsg->totalLgth += 2;
      structmsg->msg.cmdLgth += 2;
      fieldLgth = 2;
      break;
    case STRUCT_MSG_FIELD_UINT32_T:
    case STRUCT_MSG_FIELD_INT32_T:
      structmsg->totalLgth += 4;
      structmsg->msg.cmdLgth += 4;
      fieldLgth = 4;
      break;
    case STRUCT_MSG_FIELD_UINT8_VECTOR:
    case STRUCT_MSG_FIELD_INT8_VECTOR:
      structmsg->totalLgth += fieldLgth;
      structmsg->msg.cmdLgth += fieldLgth;
      fieldInfo->value.vector = (uint8_t *)os_malloc(fieldLgth);
ets_printf("os_malloc: %p %s %d\n", fieldInfo->value.vector, __FILE__, __LINE__);
      break;
  }
  fieldInfo->fieldLgth = fieldLgth;
  structmsg->msg.numFieldInfos++;
  return STRUCT_MSG_ERR_OK;
}

// ============================= dump_structmsg ========================

int dump_structmsg(const uint8_t *handle) {
  int numEntries;
  int idx;
  int valueIdx;
  uint8_t uch;
  int8_t ch;
  structmsg_t *structmsg;

  structmsg = get_structmsg_ptr(handle);
  if (structmsg == NULL) {
    return STRUCT_MSG_ERR_BAD_HANDLE;
  }
  ets_printf("handle: %s src: %d dst: %d totalLgth: %d\r\n", structmsg->handle, (int)structmsg->src, (int)structmsg->dst, (int)structmsg->totalLgth);
  ets_printf("  cmdKey: %d cmdLgth: %d\r\n", (int)structmsg->msg.cmdKey, (int)structmsg->msg.cmdLgth);
  numEntries = structmsg->msg.numFieldInfos;
  ets_printf("  numFieldInfos: %d max: %d\r\n", numEntries, (int)structmsg->msg.maxFieldInfos);
  idx = 0;
  while (idx < numEntries) {
    fieldInfo_t *fieldInfo = &structmsg->msg.fieldInfos[idx];
    ets_printf("    idx %d: key: %-20s type: %-8s lgth: %.5d\r\n", idx, fieldInfo->fieldStr, getFieldTypeStr(fieldInfo->fieldType), fieldInfo->fieldLgth);
//ets_printf("isSet: %s 0x%02x %d\n", fieldInfo->fieldStr, fieldInfo->flags, (fieldInfo->flags & STRUCT_MSG_FIELD_IS_SET));
    if (fieldInfo->flags & STRUCT_MSG_FIELD_IS_SET) {
      switch (fieldInfo->fieldType) {
      case STRUCT_MSG_FIELD_INT8_T:
        ets_printf("      value: 0x%02x\n", (int8_t)fieldInfo->value.byteVal);
        break;
      case STRUCT_MSG_FIELD_UINT8_T:
        ets_printf("      value: 0x%02x\n", (uint8_t)fieldInfo->value.ubyteVal);
        break;
      case STRUCT_MSG_FIELD_INT16_T:
        ets_printf("      value: 0x%04x\n", (int16_t)fieldInfo->value.shortVal);
        break;
      case STRUCT_MSG_FIELD_UINT16_T:
        ets_printf("      value: 0x%04x\n", (uint16_t)fieldInfo->value.ushortVal);
        break;
      case STRUCT_MSG_FIELD_INT32_T:
        ets_printf("      value: 0x%08x\n", (int32_t)fieldInfo->value.val);
        break;
      case STRUCT_MSG_FIELD_UINT32_T:
        ets_printf("      value: 0x%08x\n", (uint32_t)fieldInfo->value.uval);
        break;
      case STRUCT_MSG_FIELD_INT8_VECTOR:
        valueIdx = 0;
        ets_printf("      values:");
        while (valueIdx < fieldInfo->fieldLgth) {
          ch = fieldInfo->value.vector[valueIdx];
          ets_printf("        idx: %d value: %c 0x%02x\n", valueIdx, (char)ch, (uint8_t)(ch & 0xFF));
          valueIdx++;
        }
        ets_printf("\n");
        break;
      case STRUCT_MSG_FIELD_UINT8_VECTOR:
        valueIdx = 0;
        ets_printf("      values:\n");
        while (valueIdx < fieldInfo->fieldLgth) {
          uch = fieldInfo->value.uvector[valueIdx];
          ets_printf("        idx: %d value: %c 0x%02x\n", valueIdx, (char)uch, (uint8_t)(uch & 0xFF));
          valueIdx++;
        }
        break;
      }
    }
    idx++;
  }
  return STRUCT_MSG_ERR_OK;
}

// ============================= getFieldValue ========================

int getFieldValue(const uint8_t *handle, const uint8_t *fieldName, int *numericValue, uint8_t **stringValue) {
  structmsg_t *structmsg;
  fieldInfo_t *fieldInfo;
  int idx;
  int numEntries;

  structmsg = get_structmsg_ptr(handle);
  if (structmsg == NULL) {
    return STRUCT_MSG_ERR_BAD_HANDLE;
  }
  *numericValue = 0;
  *stringValue = NULL;
  idx = 0;
  numEntries = structmsg->msg.numFieldInfos;
  while (idx < numEntries) {
    fieldInfo = &structmsg->msg.fieldInfos[idx];
    if (c_strcmp(fieldName, fieldInfo->fieldStr) == 0) {
//ets_printf("    idx %d: key: %-20s\r\n", idx, fieldInfo->fieldStr);
      switch (fieldInfo->fieldType) {
      case STRUCT_MSG_FIELD_INT8_T:
        if (fieldInfo->flags & STRUCT_MSG_FIELD_IS_SET) {
          *numericValue = (int)fieldInfo->value.byteVal;
          return STRUCT_MSG_ERR_OK;
        }
        break;
      case STRUCT_MSG_FIELD_UINT8_T:
        if (fieldInfo->flags & STRUCT_MSG_FIELD_IS_SET) {
          *numericValue = (int)fieldInfo->value.ubyteVal;
          return STRUCT_MSG_ERR_OK;
        }
        break;
      case STRUCT_MSG_FIELD_INT16_T:
        if (fieldInfo->flags & STRUCT_MSG_FIELD_IS_SET) {
          *numericValue = (int)fieldInfo->value.shortVal;
          return STRUCT_MSG_ERR_OK;
        }
        break;
      case STRUCT_MSG_FIELD_UINT16_T:
        if (fieldInfo->flags & STRUCT_MSG_FIELD_IS_SET) {
          *numericValue = (int)fieldInfo->value.ushortVal;
          return STRUCT_MSG_ERR_OK;
        }
        break;
      case STRUCT_MSG_FIELD_INT32_T:
        if (fieldInfo->flags & STRUCT_MSG_FIELD_IS_SET) {
          *numericValue = (int)fieldInfo->value.val;
          return STRUCT_MSG_ERR_OK;
        }
        break;
      case STRUCT_MSG_FIELD_UINT32_T:
        if (fieldInfo->flags & STRUCT_MSG_FIELD_IS_SET) {
          *numericValue = (int)fieldInfo->value.uval;
          return STRUCT_MSG_ERR_OK;
        }
        break;
      case STRUCT_MSG_FIELD_INT8_VECTOR:
        if (fieldInfo->flags & STRUCT_MSG_FIELD_IS_SET) {
          *stringValue = fieldInfo->value.vector;
          return STRUCT_MSG_ERR_OK;
        }
        break;
      case STRUCT_MSG_FIELD_UINT8_VECTOR:
        if (fieldInfo->flags & STRUCT_MSG_FIELD_IS_SET) {
          *stringValue = fieldInfo->value.uvector;
          return STRUCT_MSG_ERR_OK;
        }
        break;
      default:
        return STRUCT_MSG_ERR_BAD_FIELD_TYPE;
        break;
      }
      return STRUCT_MSG_ERR_VALUE_NOT_SET;
    }
    idx++;
  }
  return STRUCT_MSG_ERR_FIELD_NOT_FOUND;
}

// ============================= setFieldValue ========================

int setFieldValue(const uint8_t *handle, const uint8_t *fieldName, int numericValue, const uint8_t *stringValue) {
  structmsg_t *structmsg;
  fieldInfo_t *fieldInfo;
  int idx;
  int numEntries;

  structmsg = get_structmsg_ptr(handle);
  if (structmsg == NULL) {
    return STRUCT_MSG_ERR_BAD_HANDLE;
  }
  idx = 0;
  numEntries = structmsg->msg.numFieldInfos;
  while (idx < numEntries) {
    fieldInfo = &structmsg->msg.fieldInfos[idx];
    if (c_strcmp(fieldName, fieldInfo->fieldStr) == 0) {
      switch (fieldInfo->fieldType) {
      case STRUCT_MSG_FIELD_INT8_T:
        if (stringValue == NULL) {
          if ((numericValue > -128) || (numericValue < 128)) {
            fieldInfo->value.byteVal = (int8_t)numericValue;
            fieldInfo->flags |= STRUCT_MSG_FIELD_IS_SET;
            return STRUCT_MSG_ERR_OK;
          } else {
            return STRUCT_MSG_ERR_VALUE_TOO_BIG;
          }
        }
        break;
      case STRUCT_MSG_FIELD_UINT8_T:
        if (stringValue == NULL) {
          if ((numericValue >= 0) || (numericValue <= 256)) {
            fieldInfo->value.ubyteVal = (uint8_t)numericValue;
            fieldInfo->flags |= STRUCT_MSG_FIELD_IS_SET;
            return STRUCT_MSG_ERR_OK;
          }
        } else {
          return STRUCT_MSG_ERR_BAD_VALUE;
        }
        break;
      case STRUCT_MSG_FIELD_INT16_T:
        if (stringValue == NULL) {
          if ((numericValue > -32767) || (numericValue < 32767)) {
            fieldInfo->value.shortVal = (int16_t)numericValue;
            fieldInfo->flags |= STRUCT_MSG_FIELD_IS_SET;
            return STRUCT_MSG_ERR_OK;
          } else {
            return STRUCT_MSG_ERR_VALUE_TOO_BIG;
          }
        }
        break;
      case STRUCT_MSG_FIELD_UINT16_T:
        if (stringValue == NULL) {
          if ((numericValue >= 0) || (numericValue <= 65535)) {
            fieldInfo->value.ushortVal = (uint16_t)numericValue;
            fieldInfo->flags |= STRUCT_MSG_FIELD_IS_SET;
            return STRUCT_MSG_ERR_OK;
          } else {
            return STRUCT_MSG_ERR_VALUE_TOO_BIG;
          }
        }
        break;
      case STRUCT_MSG_FIELD_INT32_T:
        if (stringValue == NULL) {
          if ((numericValue > -0x7FFFFFFF) || (numericValue <= 0x7FFFFFFF)) {
            fieldInfo->value.val = (int32_t)numericValue;
            fieldInfo->flags |= STRUCT_MSG_FIELD_IS_SET;
            return STRUCT_MSG_ERR_OK;
          } else {
            return STRUCT_MSG_ERR_VALUE_TOO_BIG;
          }
        }
        break;
      case STRUCT_MSG_FIELD_UINT32_T:
        if (stringValue == NULL) {
          if ((numericValue >= 0) || (numericValue <= 65535)) {
            fieldInfo->value.uval = (uint32_t)numericValue;
            fieldInfo->flags |= STRUCT_MSG_FIELD_IS_SET;
            return STRUCT_MSG_ERR_OK;
          } else {
            return STRUCT_MSG_ERR_VALUE_TOO_BIG;
          }
        }
        break;
      case STRUCT_MSG_FIELD_INT8_VECTOR:
        if (stringValue != NULL) {
          os_memcpy(fieldInfo->value.uvector, stringValue, fieldInfo->fieldLgth);
          fieldInfo->flags |= STRUCT_MSG_FIELD_IS_SET;
          return STRUCT_MSG_ERR_OK;
        }
        break;
      case STRUCT_MSG_FIELD_UINT8_VECTOR:
        if (stringValue != NULL) {
          fieldInfo->flags |= STRUCT_MSG_FIELD_IS_SET;
          os_memcpy(fieldInfo->value.vector, stringValue, fieldInfo->fieldLgth);
          return STRUCT_MSG_ERR_OK;
        }
        break;
      default:
        return STRUCT_MSG_ERR_BAD_FIELD_TYPE;
        break;
      }
      return STRUCT_MSG_ERR_BAD_VALUE;
    }
    idx++;
  }
  return STRUCT_MSG_ERR_FIELD_NOT_FOUND;
}

// ============================= encodeMsg ========================

int encodeMsg(const uint8_t *handle) {
  structmsg_t *structmsg;
  uint8_t *msgPtr;
  uint16_t *ushortPtr;
  int16_t *shortPtr;
  uint32_t *uintPtr;
  int32_t *intPtr;
  int32_t randomNum;
  uint16_t crc;
  int offset;
  int idx;
  int fieldIdx;
  int numEntries;
  int result;
  fieldInfo_t *fieldInfo;

  structmsg = get_structmsg_ptr(handle);
  if (structmsg == NULL) {
    return STRUCT_MSG_ERR_BAD_HANDLE;
  }
  if (structmsg->encoded != NULL) {
    os_free(structmsg->encoded);
  }
  structmsg->encoded = (uint8_t *)os_malloc(structmsg->totalLgth);
ets_printf("os_malloc: %p %s %d\n", structmsg->encoded, __FILE__, __LINE__);
  msgPtr = structmsg->encoded;
  offset = 0;
  offset = uint16Encode(msgPtr,offset,structmsg->src);
  checkEncodeOffset(offset);
  offset = uint16Encode(msgPtr,offset,structmsg->dst);
  checkEncodeOffset(offset);
  offset = uint16Encode(msgPtr,offset,structmsg->totalLgth);
  checkEncodeOffset(offset);
  offset = uint16Encode(msgPtr,offset,structmsg->msg.cmdKey);
  checkEncodeOffset(offset);
  offset = uint16Encode(msgPtr,offset,structmsg->msg.cmdLgth);
  checkEncodeOffset(offset);
  idx = 0;
  numEntries = structmsg->msg.numFieldInfos;
  while (idx < numEntries) {
    fieldInfo = &structmsg->msg.fieldInfos[idx];
    if (fieldInfo->fieldStr[0] == '@') {
      if (c_strcmp(fieldInfo->fieldStr, "@randomNum") == 0) {
        offset = randomNumEncode(msgPtr, offset, &randomNum);
        checkEncodeOffset(offset);
        result = setFieldValue(handle, "@randomNum", randomNum, NULL);
        if (result != STRUCT_MSG_ERR_OK) {
          return result;
        }
      } else {
        if (c_strcmp(fieldInfo->fieldStr, "@filler") == 0) {
          offset = fillerEncode(msgPtr, offset, fieldInfo->fieldLgth, fieldInfo->value.uvector);
          checkEncodeOffset(offset);
          result = setFieldValue(handle, "@filler", 0, fieldInfo->value.uvector);
          if (result != STRUCT_MSG_ERR_OK) {
            return result;
          }
        } else {
          if (c_strcmp(fieldInfo->fieldStr, "@crc") == 0) {
            offset = crcEncode(structmsg->encoded, offset, structmsg->encoded, structmsg->msg.cmdLgth, &crc);
            checkEncodeOffset(offset);
            result = setFieldValue(handle, "@crc", crc, NULL);
            if (result != STRUCT_MSG_ERR_OK) {
              return result;
            }
          } else {
            return STRUCT_MSG_ERR_BAD_SPECIAL_FIELD;
          }
        }
      }
//ets_printf("special_func: %s offset: %d\n", fieldInfo->fieldStr, offset);
    } else {
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
            offset = int8Encode(msgPtr,offset,fieldInfo->value.vector[fieldIdx]);
            fieldIdx++;
          }
          break;
        case STRUCT_MSG_FIELD_UINT8_VECTOR:
          fieldIdx = 0;
          while (fieldIdx < fieldInfo->fieldLgth) {
            offset = uint8Encode(msgPtr,offset,fieldInfo->value.uvector[fieldIdx]);
            fieldIdx++;
          }
          break;
      }
      checkEncodeOffset(offset);
    }
    idx++;
  }
  structmsg->flags |= STRUCT_MSG_ENCODED;
  return STRUCT_MSG_ERR_OK;
}

// ============================= getEncoded ========================

int getEncoded(const uint8_t *handle, uint8_t ** encoded, int *lgth) {
  structmsg_t *structmsg;

  structmsg = get_structmsg_ptr(handle);
  if (structmsg == NULL) {
    return STRUCT_MSG_ERR_BAD_HANDLE;
  }
  if (structmsg->encoded == NULL) {
    return STRUCT_MSG_ERR_NOT_ENCODED;
  }
  *encoded = structmsg->encoded;
  *lgth = structmsg->totalLgth;
    return STRUCT_MSG_ERR_OK;
  return STRUCT_MSG_ERR_OK;
}

// ============================= decodeMsg ========================

int decodeMsg(const uint8_t *handle, const uint8_t *data) {
  structmsg_t *structmsg;
  const uint8_t *msgPtr;
  uint16_t crc;
  int offset;
  int idx;
  int fieldIdx;
  int numEntries;
  int result;
  fieldInfo_t *fieldInfo;

  structmsg = get_structmsg_ptr(handle);
  if (structmsg == NULL) {
    return STRUCT_MSG_ERR_BAD_HANDLE;
  }
  if (structmsg->todecode != NULL) {
    os_free(structmsg->todecode);
  }
  structmsg->todecode = (uint8_t *)os_malloc(structmsg->totalLgth);
ets_printf("os_malloc: %p %s %d\n", structmsg->todecode, __FILE__, __LINE__);
  msgPtr = data; // data starts at structmsg->msg.cmdKey!!
  offset = 0;
  offset = uint16Decode(msgPtr,offset,&structmsg->src);
  checkDecodeOffset(offset);
  offset = uint16Decode(msgPtr,offset,&structmsg->dst);
  checkDecodeOffset(offset);
  offset = uint16Decode(msgPtr,offset,&structmsg->totalLgth);
  checkDecodeOffset(offset);
  offset = uint16Decode(msgPtr,offset,&structmsg->msg.cmdKey);
  checkDecodeOffset(offset);
  offset = uint16Decode(msgPtr,offset,&structmsg->msg.cmdLgth);
  checkDecodeOffset(offset);
  idx = 0;
  numEntries = structmsg->msg.numFieldInfos;
  while (idx < numEntries) {
    fieldInfo = &structmsg->msg.fieldInfos[idx];
    if (fieldInfo->fieldStr[0] == '@') {
      if (c_strcmp(fieldInfo->fieldStr, "@randomNum") == 0) {
        offset = randomNumDecode(msgPtr, offset, &fieldInfo->value.uval);
        checkDecodeOffset(offset);
      } else {
        if (c_strcmp(fieldInfo->fieldStr, "@filler") == 0) {
          offset = fillerDecode(msgPtr, offset, fieldInfo->fieldLgth, &fieldInfo->value.uvector);
          checkDecodeOffset(offset);
        } else {
          if (c_strcmp(fieldInfo->fieldStr, "@crc") == 0) {
            offset = crcDecode(msgPtr, offset, structmsg->todecode, structmsg->msg.cmdLgth, &fieldInfo->value.ushortVal);
            if (offset < 0) {
              return STRUCT_MSG_ERR_BAD_CRC_VALUE;
            }
          } else {
            return STRUCT_MSG_ERR_BAD_SPECIAL_FIELD;
          }
        }
      }
      fieldInfo->flags |= STRUCT_MSG_FIELD_IS_SET;
    } else {
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
          offset = int8VectorDecode(msgPtr,offset,fieldInfo->fieldLgth, &fieldInfo->value.vector);
          checkDecodeOffset(offset);
          break;
        case STRUCT_MSG_FIELD_UINT8_VECTOR:
          offset = uint8VectorDecode(msgPtr,offset,fieldInfo->fieldLgth, &fieldInfo->value.uvector);
          checkDecodeOffset(offset);
          break;
      }
      fieldInfo->flags |= STRUCT_MSG_FIELD_IS_SET;
    }
    idx++;
  }
  structmsg->flags |= STRUCT_MSG_DECODED;
  return STRUCT_MSG_ERR_OK;
}

// ============================= freeEncDecBuf ========================

int freeEncDecBuf(const uint8_t *handle, uint8_t *buf, bool enc) {
  structmsg_t *structmsg;

  structmsg = get_structmsg_ptr(handle);
  if (structmsg == NULL) {
    return STRUCT_MSG_ERR_BAD_HANDLE;
  }
  os_free (buf);
ets_printf("free: buf: %p structmsg->encrypted: %p structmsg->decrypted: %p enc: %d\n", buf, structmsg->encrypted, structmsg->decrypted, enc);
  if (enc) {
    structmsg->encrypted = NULL;
  } else {
    structmsg->decrypted = NULL;
  }
  return STRUCT_MSG_ERR_OK;
}

// ============================= encdec ========================

int encdec(const uint8_t *handle, const uint8_t *key, size_t klen, const uint8_t *iv, size_t ivlen, bool enc, uint8_t **buf, int *lgth) {
  structmsg_t *structmsg;
  const crypto_mech_t *mech;
  const char *data;
  size_t dlen;
  size_t bs;
  uint8_t *crypted;
  int offset;

  structmsg = get_structmsg_ptr(handle);
  if (structmsg == NULL) {
    return STRUCT_MSG_ERR_BAD_HANDLE;
  }
  if (structmsg->encoded == NULL) {
    return STRUCT_MSG_ERR_NOT_ENCODED;
  }
  mech = crypto_encryption_mech ("AES-CBC");
  if (mech == NULL) {
    return STRUCT_MSG_ERR_CRYPTO_BAD_MECHANISM;
  }
  bs = mech->block_size;
  if (enc) {
    data = structmsg->encoded;
    data += sizeof(uint16_t) * 3; // uint16_t src + uint16_t dst + uint16_t totalLgth
    dlen = structmsg->msg.cmdLgth;
//ets_printf("enc structmsg: %p %p %d\n", structmsg, data, dlen);
//ets_printf("enc ch: 0x%02x 0x%02x 0x%02x 0x%02x\n", data[0], data[1], data[2], data[3]);
    if (structmsg->encrypted != NULL) {
      os_free(structmsg->encrypted);
    }
    *lgth = ((dlen + bs -1) / bs) * bs;
    structmsg->encryptedLgth = *lgth;
    *lgth += sizeof(uint16_t) * 3; // uint16_t src + uint16_t dst + uint16_t totalLgth;
    structmsg->encrypted = (char *)os_zalloc (*lgth);
ets_printf("os_zalloc: %p %s %d\n", structmsg->encrypted, __FILE__, __LINE__);
    if (!structmsg->encrypted) {
      return STRUCT_MSG_ERR_CRYPTO_INIT_FAILED;
    } 
    *buf = structmsg->encrypted;
  } else {
    data = structmsg->encrypted;
    data += sizeof(uint16_t) * 3; // uint16_t src + uint16_t dst + uint16_t totalLgth
    dlen = structmsg->encryptedLgth;
    if (structmsg->decrypted != NULL) {
      os_free(structmsg->decrypted);
    }
    *lgth = ((dlen + bs -1) / bs) * bs;
    structmsg->decryptedLgth = *lgth;
    *lgth += sizeof(uint16_t) * 3; // uint16_t src + uint16_t dst + uint16_t totalLgth;
    structmsg->decrypted = (char *)os_zalloc (*lgth);
ets_printf("os_zalloc: %p %s %d\n", structmsg->decrypted, __FILE__, __LINE__);
    if (!structmsg->decrypted) {
      return STRUCT_MSG_ERR_CRYPTO_INIT_FAILED;
    } 
    *buf = structmsg->decrypted;
  }
  offset = 0;
  crypted = *buf;
  offset = uint16Encode(crypted, offset, structmsg->src);
  checkEncodeOffset(offset);
  offset = uint16Encode(crypted, offset, structmsg->dst);
  checkEncodeOffset(offset);
  offset = uint16Encode(crypted, offset, structmsg->totalLgth);
  checkEncodeOffset(offset);
  crypted = *buf+sizeof(uint16_t) * 3; // uint16_t src + uint16_t dst + uint16_t totalLgth
//ets_printf("buf0: 0x%02x crpto0: 0x%02x\n", (*buf)[0], crypted[0]);
//ets_printf("key: %s klen: %d iv: %s ivlen: %d data: %p dlen: %d\n", key, klen, iv, ivlen, data, dlen);
  crypto_op_t op =
  { 
    key, klen,
    iv, ivlen,
    data, dlen,
    crypted, *lgth,
    enc ? OP_ENCRYPT : OP_DECRYPT
  }; 
  if (!mech->run (&op)) { 
    os_free (*buf);
    if (enc) {
      structmsg->encrypted = NULL;
    } else {
      structmsg->decrypted = NULL;
    }
    return STRUCT_MSG_ERR_CRYPTO_INIT_FAILED;
  } else { 
if (!enc) {
//ets_printf("dec ch: 0x%02x 0x%02x 0x%02x 0x%02x\n", crypted[0], crypted[1], crypted[2], crypted[3]);
}
    return STRUCT_MSG_ERR_OK;
  }
}

// ============================= setFillerAndCrc ========================

int setFillerAndCrc(const uint8_t *handle) {
  structmsg_t *structmsg;
  int16_t fillerLgth = 0;
  int16_t myLgth = 0;
  int result;

  structmsg = get_structmsg_ptr(handle);
  if (structmsg == NULL) {
    return STRUCT_MSG_ERR_BAD_HANDLE;
  }
  myLgth = structmsg->msg.cmdLgth + 2;
  while ((myLgth % 16) != 0) {
    myLgth++;
    fillerLgth++;
  }
  result = addField(handle, "@filler", STRUCT_MSG_FIELD_UINT8_VECTOR, fillerLgth);
  if (result != STRUCT_MSG_ERR_OK) {
    return result;
  }
  result = addField(handle, "@crc", STRUCT_MSG_FIELD_UINT16_T, 1);
  if (result != STRUCT_MSG_ERR_OK) {
    return result;
  }
  return STRUCT_MSG_ERR_OK;
}

// ============================= set_targets ========================

int set_targets(const uint8_t *handle, uint16_t src, uint16_t dst, uint16_t cmdKey) {
  structmsg_t *structmsg;

  structmsg = get_structmsg_ptr(handle);
  if (structmsg == NULL) {
    return STRUCT_MSG_ERR_BAD_HANDLE;
  }
  structmsg->src = src;
  structmsg->dst = dst;
  structmsg->msg.cmdKey = cmdKey;
  return STRUCT_MSG_ERR_OK;
}

// ============================= new_structmsg ========================

int new_structmsg(uint8_t numFieldInfos, uint8_t **handle) {
  uint8_t *ptr;
  int result;

  structmsg_t *structmsg = (void *)os_malloc (sizeof(structmsg_t));
ets_printf("os_malloc: %p %s %d\n", structmsg, __FILE__, __LINE__);
  if (structmsg == NULL) {
    return STRUCT_MSG_ERR_OUT_OF_MEMORY;
  }
  structmsg->msg.cmdLgth = 4; // uint16_t cmdKey + unit16_t cmdLgth
  structmsg->totalLgth = 10; // cmdLgth + uint16_t src + uint16_t dst + uint16_t totalLgth
  structmsg->msg.maxFieldInfos = numFieldInfos;
  structmsg->msg.numFieldInfos = 0;
  structmsg->msg.fieldInfos = newFieldInfos(numFieldInfos);
  structmsg->flags = 0;
  structmsg->encoded = NULL;
  structmsg->todecode = NULL;
  structmsg->encrypted = NULL;
  structmsg->encryptedLgth = 0;
  structmsg->decrypted = NULL;
  structmsg->decryptedLgth = 0;
  os_sprintf(structmsg->handle, "%s%p", HANDLE_PREFIX, structmsg);
  result = addHandle(structmsg->handle);
  if (result != STRUCT_MSG_ERR_OK) {
    return result;
  }
  *handle = structmsg->handle;
  return STRUCT_MSG_ERR_OK;
}

// ============================= delete_structmsg ========================

int delete_structmsg(const uint8_t *handle) {
  structmsg_t *structmsg;
  int idx;

ets_printf("delete: %s\n", handle);
  structmsg = get_structmsg_ptr(handle);
  if (structmsg == NULL) {
    return STRUCT_MSG_ERR_BAD_HANDLE;
  }
ets_printf("delete: fields\n");
  idx = 0;
  while (idx < structmsg->msg.numFieldInfos) {
    fieldInfo_t *fieldInfo = &structmsg->msg.fieldInfos[idx];
    os_free(fieldInfo->fieldStr);
    switch (fieldInfo->fieldType) {
    case STRUCT_MSG_FIELD_INT8_VECTOR:
    case STRUCT_MSG_FIELD_UINT8_VECTOR:
      if (fieldInfo->value.vector != NULL) {
        os_free(fieldInfo->value.vector);
      }
    }
    idx++;
  }
ets_printf("delete: fieldInfos\n");
  os_free(structmsg->msg.fieldInfos);
  if (structmsg->encoded != NULL) {
ets_printf("delete: encoded\n");
    os_free(structmsg->encoded);
  }
  if (structmsg->todecode != NULL) {
ets_printf("delete: todecode\n");
    os_free(structmsg->todecode);
  }
  if (structmsg->encrypted != NULL) {
ets_printf("delete: encrypted\n");
    os_free(structmsg->encrypted);
  }
  if (structmsg->decrypted != NULL) {
ets_printf("delete: decrypted\n");
    os_free(structmsg->decrypted);
  }
ets_printf("delete: handle\n");
  deleteHandle(handle);
ets_printf("delete: structmsg\n");
  os_free(structmsg);
ets_printf("delete: done\n");
  return STRUCT_MSG_ERR_OK;
}
