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

typedef struct handle2Hdr 
{
  uint8_t *handle;
  hdrInfo_t hdrInfo;
} handle2Hdr_t;

typedef struct lstructmsg_userdata
{
  handle2Hdr_t *handles;
  int numHandles;
} lstructmsg_userdata;

// create an object
static lstructmsg_userdata structmsg_userdata = { NULL, 0};


// ============================= checkHandle ========================

static int checkHandle(const uint8_t *handle) {
  int idx;

  if (structmsg_userdata.handles == NULL) {
    return STRUCT_MSG_ERR_HANDLE_NOT_FOUND;
  }
  idx = 0;
  while (idx < structmsg_userdata.numHandles) {
    if ((structmsg_userdata.handles[idx].handle != NULL) && (c_strcmp(structmsg_userdata.handles[idx].handle, handle) == 0)) {
      return STRUCT_MSG_ERR_OK;
    }
    idx++;
  }
  return STRUCT_MSG_ERR_HANDLE_NOT_FOUND;
}

// ============================= structmsg_get_structmsg_ptr ========================

structmsg_t *structmsg_get_structmsg_ptr( const uint8_t *handle )
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

// ============================= structmsg_dumpBinary ========================

void structmsg_dumpBinary(const uint8_t *data, uint8_t lgth, const uint8_t *where) {
  int idx;

  ets_printf("%s\n", where);
  idx = 0;
  while (idx < lgth) {
     ets_printf("idx: %d ch: 0x%02x\n", idx, data[idx] & 0xFF);
    idx++;
  }
}

// ============================= getHandle ========================

static int getHandle(uint8_t *hdrkey, uint8_t **handle) {
  int idx;

  *handle = NULL;
  if (structmsg_userdata.handles == NULL) {
    return STRUCT_MSG_ERR_HANDLE_NOT_FOUND;
  } else {
    idx = 0;
    while (idx < structmsg_userdata.numHandles) {
      if (structmsg_userdata.handles[idx].handle != NULL) {
        if (c_memcmp(structmsg_userdata.handles[idx].hdrInfo.hdrId, hdrkey, STRUCT_MSG_TOTAL_HEADER_LENGTH) == 0) {
          *handle = structmsg_userdata.handles[idx].handle;
          return STRUCT_MSG_ERR_OK;
        }
      }
      idx++;
    }
  }
  return STRUCT_MSG_ERR_HANDLE_NOT_FOUND;
}

// ============================= addHandle ========================

static int addHandle(uint8_t *handle, hdrInfo_t **hdrInfo) {
  int idx;

  if (structmsg_userdata.handles == NULL) {
    structmsg_userdata.handles = os_malloc(sizeof(handle2Hdr_t));
    if (structmsg_userdata.handles == NULL) {
      return STRUCT_MSG_ERR_OUT_OF_MEMORY;
    } else {
      structmsg_userdata.handles[structmsg_userdata.numHandles].handle = handle;
      *hdrInfo = &structmsg_userdata.handles[structmsg_userdata.numHandles++].hdrInfo;
      return STRUCT_MSG_ERR_OK;
    }
  } else {
    // check for unused slot first
    idx = 0;
    while (idx < structmsg_userdata.numHandles) {
      if (structmsg_userdata.handles[idx].handle == NULL) {
        structmsg_userdata.handles[idx].handle = handle;
        *hdrInfo = &structmsg_userdata.handles[idx].hdrInfo;
        return STRUCT_MSG_ERR_OK;
      }
      idx++;
    }
    structmsg_userdata.handles = os_realloc(structmsg_userdata.handles, sizeof(handle2Hdr_t)*(structmsg_userdata.numHandles+1));
    checkAllocOK(structmsg_userdata.handles);
    structmsg_userdata.handles[structmsg_userdata.numHandles].handle = handle;
    *hdrInfo = &structmsg_userdata.handles[structmsg_userdata.numHandles++].hdrInfo;
  }
  return STRUCT_MSG_ERR_OK;
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
    if ((structmsg_userdata.handles[idx].handle != NULL) && (c_strcmp(structmsg_userdata.handles[idx].handle, handle) == 0)) {
      structmsg_userdata.handles[idx].handle = NULL;
      found++;
    } else {
      if (structmsg_userdata.handles[idx].handle != NULL) {
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

// ============================= newFieldInfos ========================

static fieldInfo_t *newFieldInfos(uint8_t numFieldInfos)
{
  fieldInfo_t *ptr = (fieldInfo_t *)os_malloc (sizeof(fieldInfo_t) * numFieldInfos);
  return ptr;
}

// ============================= setHandleField ========================

static int setHandleField(const uint8_t *handle, int fieldKey, int fieldValue) {
  int idx;
  int result = STRUCT_MSG_ERR_OK;

  if (structmsg_userdata.handles == NULL) {
    return STRUCT_MSG_ERR_HANDLE_NOT_FOUND;
  }
  idx = 0;
  while (idx < structmsg_userdata.numHandles) {
    if ((structmsg_userdata.handles[idx].handle != NULL) && (c_strcmp(structmsg_userdata.handles[idx].handle, handle) == 0)) {
      switch (fieldKey) {
      case STRUCT_MSG_FIELD_SRC:
        structmsg_userdata.handles[idx].hdrInfo.hdrKeys.src = (uint16_t)fieldValue;
        break;
      case STRUCT_MSG_FIELD_DST:
        structmsg_userdata.handles[idx].hdrInfo.hdrKeys.dst = (uint16_t)fieldValue;
        break;
      case STRUCT_MSG_FIELD_TOTAL_LGTH:
        structmsg_userdata.handles[idx].hdrInfo.hdrKeys.totalLgth = (uint16_t)fieldValue;
        break;
      case STRUCT_MSG_FIELD_CMD_KEY:
        structmsg_userdata.handles[idx].hdrInfo.hdrKeys.cmdKey = (uint16_t)fieldValue;
        break;
      case STRUCT_MSG_FIELD_CMD_LGTH:
        structmsg_userdata.handles[idx].hdrInfo.hdrKeys.cmdLgth = (uint16_t)fieldValue;
        break;
      }
      return result;
    }
    idx++;
  }
  return STRUCT_MSG_ERR_HANDLE_NOT_FOUND;
}

// ============================= encryptdecrypt ========================

static int encryptdecrypt(const uint8_t *message, size_t mlen, const uint8_t *key, size_t klen, const uint8_t *iv, size_t ivlen, bool enc, uint8_t **buf, int *lgth) {
  const crypto_mech_t *mech;
  const char *data;
  size_t dlen;
  size_t bs;
  size_t clen;
  const uint8_t *what;
  uint8_t *crypted;

  mech = crypto_encryption_mech ("AES-CBC");
  if (mech == NULL) {
    return STRUCT_MSG_ERR_CRYPTO_BAD_MECHANISM;
  }
  bs = mech->block_size;
  what = enc ? "encrypt": "decrypt";
//ets_printf("encryptdecrypt bs: %d what: %s enc: %d\n", bs, what, enc);
  dlen = mlen - STRUCT_MSG_HEADER_LENGTH;
  data = message + STRUCT_MSG_HEADER_LENGTH;
  clen = ((dlen + bs - 1) / bs) * bs;
  *lgth = clen + STRUCT_MSG_HEADER_LENGTH;
//ets_printf("dlen: %d lgth: %d clen: %d data: %p\n", dlen, *lgth, clen, data);
  crypted = (uint8_t *)os_zalloc (*lgth);
  if (!crypted) {
    return STRUCT_MSG_ERR_CRYPTO_INIT_FAILED;
  } 
  c_memcpy(crypted, message, STRUCT_MSG_HEADER_LENGTH);
  *buf = crypted;
  crypted += STRUCT_MSG_HEADER_LENGTH;
  crypto_op_t op =
  { 
    key, klen,
    iv, ivlen,
    data, dlen,
    crypted, clen,
    enc ? OP_ENCRYPT : OP_DECRYPT
  }; 
  if (!mech->run (&op)) { 
    os_free (*buf);
    return STRUCT_MSG_ERR_CRYPTO_INIT_FAILED;
  } else { 
    return STRUCT_MSG_ERR_OK;
  }
}

// ============================= fixHeaderInfo ========================

static int fixHeaderInfo(structmsg_t *structmsg, fieldInfo_t *fieldInfo, const uint8_t *fieldStr, uint8_t fieldKey, uint8_t fieldType, uint8_t fieldLgth, uint8_t numTableRows) {
  fieldInfo->fieldStr = os_malloc(os_strlen(fieldStr) + 1);
  fieldInfo->fieldStr[os_strlen(fieldStr)] = '\0';
  os_memcpy(fieldInfo->fieldStr, fieldStr, os_strlen(fieldStr));
  fieldInfo->fieldKey = fieldKey;
  fieldInfo->fieldType = fieldType;
  fieldInfo->value.byteVector = NULL;
  fieldInfo->flags = 0;
  switch (fieldType) {
    case STRUCT_MSG_FIELD_UINT8_T:
    case STRUCT_MSG_FIELD_INT8_T:
      structmsg->hdr.hdrInfo.hdrKeys.totalLgth += 1 * numTableRows;
      structmsg->hdr.hdrInfo.hdrKeys.cmdLgth += 1 * numTableRows;
      fieldLgth = 1;
      break;
    case STRUCT_MSG_FIELD_UINT16_T:
    case STRUCT_MSG_FIELD_INT16_T:
      structmsg->hdr.hdrInfo.hdrKeys.totalLgth += 2 * numTableRows;
      structmsg->hdr.hdrInfo.hdrKeys.cmdLgth += 2 * numTableRows;
      fieldLgth = 2;
      break;
    case STRUCT_MSG_FIELD_UINT32_T:
    case STRUCT_MSG_FIELD_INT32_T:
      structmsg->hdr.hdrInfo.hdrKeys.totalLgth += 4 * numTableRows;
      structmsg->hdr.hdrInfo.hdrKeys.cmdLgth += 4 * numTableRows;
      fieldLgth = 4;
      break;
    case STRUCT_MSG_FIELD_UINT8_VECTOR:
      structmsg->hdr.hdrInfo.hdrKeys.totalLgth += fieldLgth * numTableRows;
      structmsg->hdr.hdrInfo.hdrKeys.cmdLgth += fieldLgth * numTableRows;
      fieldInfo->value.ubyteVector = (uint8_t *)os_malloc(fieldLgth + 1);
      fieldInfo->value.ubyteVector[fieldLgth] = '\0';
      break;
    case STRUCT_MSG_FIELD_INT8_VECTOR:
      structmsg->hdr.hdrInfo.hdrKeys.totalLgth += fieldLgth * numTableRows;
      structmsg->hdr.hdrInfo.hdrKeys.cmdLgth += fieldLgth * numTableRows;
      fieldInfo->value.byteVector = (int8_t *)os_malloc(fieldLgth + 1);
      fieldInfo->value.ubyteVector[fieldLgth] = '\0';
      break;
    case STRUCT_MSG_FIELD_UINT16_VECTOR:
      structmsg->hdr.hdrInfo.hdrKeys.totalLgth += fieldLgth * numTableRows;
      structmsg->hdr.hdrInfo.hdrKeys.cmdLgth += fieldLgth * numTableRows;
      fieldInfo->value.ushortVector = (uint16_t *)os_malloc(fieldLgth*sizeof(uint16_t));
      break;
    case STRUCT_MSG_FIELD_INT16_VECTOR:
      structmsg->hdr.hdrInfo.hdrKeys.totalLgth += fieldLgth * numTableRows;
      structmsg->hdr.hdrInfo.hdrKeys.cmdLgth += fieldLgth * numTableRows;
      fieldInfo->value.shortVector = (int16_t *)os_malloc(fieldLgth*sizeof(int16_t));
      break;
    case STRUCT_MSG_FIELD_UINT32_VECTOR:
      structmsg->hdr.hdrInfo.hdrKeys.totalLgth += fieldLgth * numTableRows;
      structmsg->hdr.hdrInfo.hdrKeys.cmdLgth += fieldLgth * numTableRows;
      fieldInfo->value.uint32Vector = (uint32_t *)os_malloc(fieldLgth*sizeof(uint32_t));
      break;
    case STRUCT_MSG_FIELD_INT32_VECTOR:
      structmsg->hdr.hdrInfo.hdrKeys.totalLgth += fieldLgth * numTableRows;
      structmsg->hdr.hdrInfo.hdrKeys.cmdLgth += fieldLgth * numTableRows;
      fieldInfo->value.int32Vector = (int32_t *)os_malloc(fieldLgth*sizeof(int32_t));
      break;
  }
  setHandleField(structmsg->handle, STRUCT_MSG_FIELD_CMD_LGTH, structmsg->hdr.hdrInfo.hdrKeys.cmdLgth);
  setHandleField(structmsg->handle, STRUCT_MSG_FIELD_TOTAL_LGTH, structmsg->hdr.hdrInfo.hdrKeys.totalLgth);
  fieldInfo->fieldLgth = fieldLgth;
  return STRUCT_MSG_ERR_OK;
}

// ============================= setFieldValue ========================

static int setFieldValue(structmsg_t *structmsg, fieldInfo_t *fieldInfo, const uint8_t *fieldName, int numericValue, const uint8_t *stringValue) {

  switch (fieldInfo->fieldType) {
    case STRUCT_MSG_FIELD_INT8_T:
      if (stringValue == NULL) {
        if ((numericValue > -128) && (numericValue < 128)) {
          fieldInfo->value.byteVal = (int8_t)numericValue;
        } else {
          return STRUCT_MSG_ERR_VALUE_TOO_BIG;
        }
      } else {
        return STRUCT_MSG_ERR_BAD_VALUE;
      }
      break;
    case STRUCT_MSG_FIELD_UINT8_T:
      if (stringValue == NULL) {
        if ((numericValue >= 0) && (numericValue <= 256)) {
          fieldInfo->value.ubyteVal = (uint8_t)numericValue;
        }
      } else {
        return STRUCT_MSG_ERR_BAD_VALUE;
      }
      break;
    case STRUCT_MSG_FIELD_INT16_T:
      if (stringValue == NULL) {
        if ((numericValue > -32767) && (numericValue < 32767)) {
          fieldInfo->value.shortVal = (int16_t)numericValue;
        } else {
          return STRUCT_MSG_ERR_VALUE_TOO_BIG;
        }
      } else {
        return STRUCT_MSG_ERR_BAD_VALUE;
      }
      break;
    case STRUCT_MSG_FIELD_UINT16_T:
      if (stringValue == NULL) {
        if ((numericValue >= 0) && (numericValue <= 65535)) {
          fieldInfo->value.ushortVal = (uint16_t)numericValue;
        } else {
          return STRUCT_MSG_ERR_VALUE_TOO_BIG;
        }
      } else {
        return STRUCT_MSG_ERR_BAD_VALUE;
      }
      break;
    case STRUCT_MSG_FIELD_INT32_T:
      if (stringValue == NULL) {
        if ((numericValue > -0x7FFFFFFF) && (numericValue <= 0x7FFFFFFF)) {
          fieldInfo->value.val = (int32_t)numericValue;
        } else {
          return STRUCT_MSG_ERR_VALUE_TOO_BIG;
        }
      } else {
        return STRUCT_MSG_ERR_BAD_VALUE;
      }
      break;
    case STRUCT_MSG_FIELD_UINT32_T:
      if (stringValue == NULL) {
        if ((numericValue >= 0) && (numericValue <= 0xFFFFFFFF)) {
          fieldInfo->value.uval = (uint32_t)numericValue;
        } else {
          return STRUCT_MSG_ERR_VALUE_TOO_BIG;
        }
      } else {
        return STRUCT_MSG_ERR_BAD_VALUE;
      }
      break;
    case STRUCT_MSG_FIELD_INT8_VECTOR:
      if (stringValue != NULL) {
        // check for length needed!!
        os_memcpy(fieldInfo->value.ubyteVector, stringValue, fieldInfo->fieldLgth);
      } else {
        return STRUCT_MSG_ERR_BAD_VALUE;
      }
      break;
    case STRUCT_MSG_FIELD_UINT8_VECTOR:
      if (stringValue != NULL) {
        // check for length needed!!
        os_memcpy(fieldInfo->value.byteVector, stringValue, fieldInfo->fieldLgth);
      } else {
        return STRUCT_MSG_ERR_BAD_VALUE;
      }
      break;
    case STRUCT_MSG_FIELD_INT16_VECTOR:
      if (stringValue != NULL) {
        // check for length needed!!
        os_memcpy((int8_t *)fieldInfo->value.shortVector, stringValue, fieldInfo->fieldLgth);
      } else {
        return STRUCT_MSG_ERR_BAD_VALUE;
      }
      break;
    case STRUCT_MSG_FIELD_UINT16_VECTOR:
      if (stringValue != NULL) {
        // check for length needed!!
        os_memcpy((uint8_t *)fieldInfo->value.ushortVector, stringValue, fieldInfo->fieldLgth);
      } else {
        return STRUCT_MSG_ERR_BAD_VALUE;
      }
      break;
    case STRUCT_MSG_FIELD_INT32_VECTOR:
      if (stringValue != NULL) {
        // check for length needed!!
        os_memcpy((int8_t *)fieldInfo->value.int32Vector, stringValue, fieldInfo->fieldLgth);
      } else {
        return STRUCT_MSG_ERR_BAD_VALUE;
      }
      break;
    case STRUCT_MSG_FIELD_UINT32_VECTOR:
      if (stringValue != NULL) {
        // check for length needed!!
        os_memcpy((uint8_t *)fieldInfo->value.uint32Vector, stringValue, fieldInfo->fieldLgth);
      } else {
        return STRUCT_MSG_ERR_BAD_VALUE;
      }
      break;
    default:
      return STRUCT_MSG_ERR_BAD_FIELD_TYPE;
      break;
  }
  fieldInfo->flags |= STRUCT_MSG_FIELD_IS_SET;
  return STRUCT_MSG_ERR_OK;
}


// ============================= getFieldValue ========================

static int getFieldValue(structmsg_t *structmsg, fieldInfo_t *fieldInfo, const uint8_t *fieldName, int *numericValue, uint8_t **stringValue) {

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
        *stringValue = fieldInfo->value.byteVector;
        return STRUCT_MSG_ERR_OK;
      }
      break;
    case STRUCT_MSG_FIELD_UINT8_VECTOR:
      if (fieldInfo->flags & STRUCT_MSG_FIELD_IS_SET) {
        *stringValue = fieldInfo->value.ubyteVector;
        return STRUCT_MSG_ERR_OK;
      }
      break;
    case STRUCT_MSG_FIELD_INT16_VECTOR:
      if (fieldInfo->flags & STRUCT_MSG_FIELD_IS_SET) {
        *stringValue = (uint8_t *)fieldInfo->value.shortVector;
        return STRUCT_MSG_ERR_OK;
      }
      break;
    case STRUCT_MSG_FIELD_UINT16_VECTOR:
      if (fieldInfo->flags & STRUCT_MSG_FIELD_IS_SET) {
        *stringValue = (uint8_t *)fieldInfo->value.ushortVector;
        return STRUCT_MSG_ERR_OK;
      }
      break;
    case STRUCT_MSG_FIELD_INT32_VECTOR:
      if (fieldInfo->flags & STRUCT_MSG_FIELD_IS_SET) {
        *stringValue = (uint8_t *)fieldInfo->value.int32Vector;
        return STRUCT_MSG_ERR_OK;
      }
      break;
    case STRUCT_MSG_FIELD_UINT32_VECTOR:
      if (fieldInfo->flags & STRUCT_MSG_FIELD_IS_SET) {
        *stringValue = (uint8_t *)fieldInfo->value.uint32Vector;
        return STRUCT_MSG_ERR_OK;
      }
      break;
    default:
      return STRUCT_MSG_ERR_BAD_FIELD_TYPE;
      break;
  }
  return STRUCT_MSG_ERR_VALUE_NOT_SET;
}

// ============================= stmsg_createMsg ========================

int stmsg_createMsg(uint8_t numFieldInfos, uint8_t **handle) {
  uint8_t *ptr;
  hdrInfo_t *hdrInfo;
  int result;

  structmsg_t *structmsg = (void *)os_malloc (sizeof(structmsg_t));
  checkAllocOK(structmsg);
  structmsg->hdr.hdrInfo.hdrKeys.src = 0;
  structmsg->hdr.hdrInfo.hdrKeys.dst = 0;
  structmsg->hdr.hdrInfo.hdrKeys.cmdKey = 0;
  structmsg->hdr.hdrInfo.hdrKeys.cmdLgth = STRUCT_MSG_CMD_HEADER_LENGTH;
  structmsg->hdr.headerLgth = STRUCT_MSG_HEADER_LENGTH;
  structmsg->hdr.hdrInfo.hdrKeys.totalLgth = STRUCT_MSG_TOTAL_HEADER_LENGTH;
  structmsg->msg.numFieldInfos = 0;
  structmsg->msg.maxFieldInfos = numFieldInfos;
  structmsg->msg.numTableRows = 0;
  structmsg->msg.numTableRowFields = 0;
  structmsg->msg.numRowFields = 0;
  structmsg->msg.fieldInfos = newFieldInfos(numFieldInfos);
  structmsg->msg.tableFieldInfos = NULL;
  structmsg->flags = 0;
  structmsg->encoded = NULL;
  structmsg->todecode = NULL;
  structmsg->encrypted = NULL;
  structmsg->handleHdrInfoPtr = NULL;
  os_sprintf(structmsg->handle, "%s%p", HANDLE_PREFIX, structmsg);
  result = addHandle(structmsg->handle, &structmsg->handleHdrInfoPtr);
  if (result != STRUCT_MSG_ERR_OK) {
    os_free(structmsg->msg.fieldInfos);
    deleteHandle(structmsg->handle);
    os_free(structmsg);
    return result;
  }
  setHandleField(structmsg->handle, STRUCT_MSG_FIELD_CMD_LGTH, structmsg->hdr.hdrInfo.hdrKeys.cmdLgth);
  setHandleField(structmsg->handle, STRUCT_MSG_FIELD_TOTAL_LGTH, structmsg->hdr.hdrInfo.hdrKeys.totalLgth);
  result = structmsg_fillHdrInfo(structmsg->handle, structmsg);
  if (result != STRUCT_MSG_ERR_OK) {
    os_free(structmsg->msg.fieldInfos);
    deleteHandle(structmsg->handle);
    os_free(structmsg);
    return result;
  }
  *handle = structmsg->handle;
  return result;
}

// ============================= stmsg_deleteMsg ========================

int stmsg_deleteMsg(const uint8_t *handle) {
  structmsg_t *structmsg;
  int idx;

  structmsg = structmsg_get_structmsg_ptr(handle);
  checkHandleOK(structmsg);
  idx = 0;
  while (idx < structmsg->msg.numFieldInfos) {
    fieldInfo_t *fieldInfo = &structmsg->msg.fieldInfos[idx];
    os_free(fieldInfo->fieldStr);
    switch (fieldInfo->fieldType) {
    case STRUCT_MSG_FIELD_INT8_VECTOR:
      if (fieldInfo->value.byteVector != NULL) {
        os_free(fieldInfo->value.byteVector);
      }
      break;
    case STRUCT_MSG_FIELD_UINT8_VECTOR:
      if (fieldInfo->value.byteVector != NULL) {
        os_free(fieldInfo->value.ubyteVector);
      }
      break;
    case STRUCT_MSG_FIELD_INT16_VECTOR:
      if (fieldInfo->value.shortVector != NULL) {
        os_free(fieldInfo->value.shortVector);
      }
      break;
    case STRUCT_MSG_FIELD_UINT16_VECTOR:
      if (fieldInfo->value.ushortVector != NULL) {
        os_free(fieldInfo->value.ushortVector);
      }
      break;
    case STRUCT_MSG_FIELD_INT32_VECTOR:
      if (fieldInfo->value.int32Vector != NULL) {
        os_free(fieldInfo->value.int32Vector);
      }
      break;
    case STRUCT_MSG_FIELD_UINT32_VECTOR:
      if (fieldInfo->value.uint32Vector != NULL) {
        os_free(fieldInfo->value.uint32Vector);
      }
      break;
    }
    idx++;
  }
  os_free(structmsg->msg.fieldInfos);
  if (structmsg->encoded != NULL) {
    os_free(structmsg->encoded);
  }
  if (structmsg->todecode != NULL) {
    os_free(structmsg->todecode);
  }
  if (structmsg->encrypted != NULL) {
    os_free(structmsg->encrypted);
  }
  deleteHandle(handle);
  os_free(structmsg);
  return STRUCT_MSG_ERR_OK;
}

// ============================= dumpTableRowFields ========================

static int dumpTableRowFields(structmsg_t *structmsg) {
  int numEntries;
  int idx;
  int valueIdx;
  int result;
  uint8_t uch;
  int8_t ch;
  uint16_t ush;
  int16_t sh;
  uint32_t uval;
  int32_t val;
  int row;
  int col;
  uint8_t *fieldType;

  numEntries = structmsg->msg.numTableRows * structmsg->msg.numRowFields;
  ets_printf("    numTableFieldInfos: %d\r\n", numEntries);
  idx = 0;
  row = 0;
  col = 0;
  while (idx < numEntries) {
    fieldInfo_t *fieldInfo = &structmsg->msg.tableFieldInfos[idx];
    result = structmsg_getFieldTypeStr(fieldInfo->fieldType, &fieldType);
    checkErrOK(result);
    ets_printf("      row %d: col: %d key: %-20s type: %-8s lgth: %.5d\r\n", row, col, fieldInfo->fieldStr, fieldType, fieldInfo->fieldLgth);
//ets_printf("isSet: %s 0x%02x %d\n", fieldInfo->fieldStr, fieldInfo->flags, (fieldInfo->flags & STRUCT_MSG_FIELD_IS_SET));
    if (fieldInfo->flags & STRUCT_MSG_FIELD_IS_SET) {
      switch (fieldInfo->fieldType) {
      case STRUCT_MSG_FIELD_INT8_T:
        ets_printf("        value: 0x%02x\n", (int8_t)fieldInfo->value.byteVal);
        break;
      case STRUCT_MSG_FIELD_UINT8_T:
        ets_printf("        value: 0x%02x\n", (uint8_t)fieldInfo->value.ubyteVal);
        break;
      case STRUCT_MSG_FIELD_INT16_T:
        ets_printf("        value: 0x%04x\n", (int16_t)fieldInfo->value.shortVal);
        break;
      case STRUCT_MSG_FIELD_UINT16_T:
        ets_printf("        value: 0x%04x\n", (uint16_t)fieldInfo->value.ushortVal);
        break;
      case STRUCT_MSG_FIELD_INT32_T:
        ets_printf("        value: 0x%08x\n", (int32_t)fieldInfo->value.val);
        break;
      case STRUCT_MSG_FIELD_UINT32_T:
        ets_printf("        value: 0x%08x\n", (uint32_t)fieldInfo->value.uval);
        break;
      case STRUCT_MSG_FIELD_INT8_VECTOR:
        valueIdx = 0;
        ets_printf("        values:");
        while (valueIdx < fieldInfo->fieldLgth) {
          ch = fieldInfo->value.byteVector[valueIdx];
          ets_printf("          idx: %d value: %c 0x%02x\n", valueIdx, (char)ch, (uint8_t)(ch & 0xFF));
          valueIdx++;
        }
        ets_printf("\n");
        break;
      case STRUCT_MSG_FIELD_UINT8_VECTOR:
        valueIdx = 0;
        ets_printf("        values:\n");
        while (valueIdx < fieldInfo->fieldLgth) {
          uch = fieldInfo->value.ubyteVector[valueIdx];
          ets_printf("          idx: %d value: %c 0x%02x\n", valueIdx, (char)uch, (uint8_t)(uch & 0xFF));
          valueIdx++;
        }
        break;
      case STRUCT_MSG_FIELD_INT16_VECTOR:
        valueIdx = 0;
        ets_printf("        values:");
        while (valueIdx < fieldInfo->fieldLgth) {
          sh = fieldInfo->value.shortVector[valueIdx];
          ets_printf("          idx: %d value: 0x%04x\n", valueIdx, (int16_t)(sh & 0xFFFF));
          valueIdx++;
        }
        ets_printf("\n");
        break;
      case STRUCT_MSG_FIELD_UINT16_VECTOR:
        valueIdx = 0;
        ets_printf("        values:\n");
        while (valueIdx < fieldInfo->fieldLgth) {
          ush = fieldInfo->value.ushortVector[valueIdx];
          ets_printf("          idx: %d value: 0x%04x\n", valueIdx, (uint16_t)(ush & 0xFFFF));
          valueIdx++;
        }
        break;
      case STRUCT_MSG_FIELD_INT32_VECTOR:
        valueIdx = 0;
        ets_printf("        values:");
        while (valueIdx < fieldInfo->fieldLgth) {
          val = fieldInfo->value.int32Vector[valueIdx];
          ets_printf("          idx: %d value: 0x%08x\n", valueIdx, (int32_t)(val & 0xFFFFFFFF));
          valueIdx++;
        }
        ets_printf("\n");
        break;
      case STRUCT_MSG_FIELD_UINT32_VECTOR:
        valueIdx = 0;
        ets_printf("        values:\n");
        while (valueIdx < fieldInfo->fieldLgth) {
          uval = fieldInfo->value.uint32Vector[valueIdx];
          ets_printf("          idx: %d value: 0x%08x\n", valueIdx, (uint32_t)(uval & 0xFFFFFFFF));
          valueIdx++;
        }
        break;
      }
    }
    col++;
    if (col == structmsg->msg.numRowFields) {
      row++;
      col = 0;
    }
    idx++;
  }
  return STRUCT_MSG_ERR_OK;
}

// ============================= stmsg_dumpMsg ========================

int stmsg_dumpMsg(const uint8_t *handle) {
  int numEntries;
  int idx;
  int valueIdx;
  int result;
  uint8_t uch;
  int8_t ch;
  uint16_t ush;
  int16_t sh;
  uint32_t uval;
  int32_t val;
  uint8_t *fieldType;
  structmsg_t *structmsg;

  structmsg = structmsg_get_structmsg_ptr(handle);
  checkHandleOK(structmsg);
  ets_printf("handle: %s src: %d dst: %d totalLgth: %d\r\n", structmsg->handle, (int)structmsg->hdr.hdrInfo.hdrKeys.src, (int)structmsg->hdr.hdrInfo.hdrKeys.dst, (int)structmsg->hdr.hdrInfo.hdrKeys.totalLgth);
  ets_printf("  cmdKey: %d cmdLgth: %d\r\n", (int)structmsg->hdr.hdrInfo.hdrKeys.cmdKey, (int)structmsg->hdr.hdrInfo.hdrKeys.cmdLgth);
  numEntries = structmsg->msg.numFieldInfos;
  ets_printf("  numFieldInfos: %d max: %d\r\n", numEntries, (int)structmsg->msg.maxFieldInfos);
  idx = 0;
  while (idx < numEntries) {
    fieldInfo_t *fieldInfo = &structmsg->msg.fieldInfos[idx];
    if (c_strcmp(fieldInfo->fieldStr, "@tablerows") == 0) {
      result = structmsg_getFieldTypeStr(fieldInfo->fieldType, &fieldType);
      checkErrOK(result);
      ets_printf("    idx %d: key: %-20s type: %-8s lgth: %.5d\r\n", idx, fieldInfo->fieldStr, fieldType, structmsg->msg.numTableRows);
      idx++;
      continue;
    }
    if (c_strcmp(fieldInfo->fieldStr, "@tablerowfields") == 0) {
      result = structmsg_getFieldTypeStr(fieldInfo->fieldType, &fieldType);
      checkErrOK(result);
      ets_printf("    idx %d: key: %-20s type: %-8s lgth: %.5d\r\n", idx, fieldInfo->fieldStr, fieldType, structmsg->msg.numRowFields);
      dumpTableRowFields(structmsg);
      idx++;
      continue;
    }
    result = structmsg_getFieldTypeStr(fieldInfo->fieldType, &fieldType);
    checkErrOK(result);
    ets_printf("    idx %d: key: %-20s type: %-8s lgth: %.5d\r\n", idx, fieldInfo->fieldStr, fieldInfo->fieldLgth);
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
          ch = fieldInfo->value.byteVector[valueIdx];
          ets_printf("        idx: %d value: %c 0x%02x\n", valueIdx, (char)ch, (uint8_t)(ch & 0xFF));
          valueIdx++;
        }
        ets_printf("\n");
        break;
      case STRUCT_MSG_FIELD_UINT8_VECTOR:
        valueIdx = 0;
        ets_printf("      values:\n");
        while (valueIdx < fieldInfo->fieldLgth) {
          uch = fieldInfo->value.ubyteVector[valueIdx];
          ets_printf("        idx: %d value: %c 0x%02x\n", valueIdx, (char)uch, (uint8_t)(uch & 0xFF));
          valueIdx++;
        }
        break;
      case STRUCT_MSG_FIELD_INT16_VECTOR:
        valueIdx = 0;
        ets_printf("      values:");
        while (valueIdx < fieldInfo->fieldLgth) {
          sh = fieldInfo->value.shortVector[valueIdx];
          ets_printf("        idx: %d value: 0x%04x\n", valueIdx, (int16_t)(sh & 0xFFFF));
          valueIdx++;
        }
        ets_printf("\n");
        break;
      case STRUCT_MSG_FIELD_UINT16_VECTOR:
        valueIdx = 0;
        ets_printf("      values:\n");
        while (valueIdx < fieldInfo->fieldLgth) {
          ush = fieldInfo->value.ushortVector[valueIdx];
          ets_printf("        idx: %d value: 0x%04x\n", valueIdx, (uint16_t)(ush & 0xFFFF));
          valueIdx++;
        }
        break;
      case STRUCT_MSG_FIELD_INT32_VECTOR:
        valueIdx = 0;
        ets_printf("      values:");
        while (valueIdx < fieldInfo->fieldLgth) {
          val = fieldInfo->value.int32Vector[valueIdx];
          ets_printf("        idx: %d value: 0x%08x\n", valueIdx, (int32_t)(val & 0xFFFFFFFF));
          valueIdx++;
        }
        ets_printf("\n");
        break;
      case STRUCT_MSG_FIELD_UINT32_VECTOR:
        valueIdx = 0;
        ets_printf("      values:\n");
        while (valueIdx < fieldInfo->fieldLgth) {
          uval = fieldInfo->value.uint32Vector[valueIdx];
          ets_printf("        idx: %d value: 0x%08x\n", valueIdx, (uint32_t)(uval & 0xFFFFFFFF));
          valueIdx++;
        }
        break;
      }
    }
    idx++;
  }
  return STRUCT_MSG_ERR_OK;
}

// ============================= stmsg_encdec ========================

int stmsg_encdec(const uint8_t *handle, const uint8_t *key, size_t klen, const uint8_t *iv, size_t ivlen, bool enc, uint8_t **buf, int *lgth) {
  structmsg_t *structmsg;
  int result;

  *buf = NULL;
  *lgth = 0;
  structmsg = structmsg_get_structmsg_ptr(handle);
  checkHandleOK(structmsg);

  if (enc) {
    if (structmsg->encoded == NULL) {
      return STRUCT_MSG_ERR_NOT_ENCODED;
    }
    result = encryptdecrypt(structmsg->encoded, structmsg->hdr.hdrInfo.hdrKeys.totalLgth, key, klen, iv, ivlen, enc, &structmsg->encrypted, lgth);
    *buf = structmsg->encrypted;
  } else {
    if (structmsg->encrypted == NULL) {
      return STRUCT_MSG_ERR_NOT_ENCRYPTED;
    }
    result = encryptdecrypt(structmsg->encrypted, structmsg->hdr.hdrInfo.hdrKeys.totalLgth, key, klen, iv, ivlen, enc, &structmsg->todecode, lgth);
    *buf = structmsg->todecode;
  }
  return result;
} 

// ============================= stmsg_addField ========================

int stmsg_addField(const uint8_t *handle, const uint8_t *fieldStr, uint8_t fieldType, int fieldLgth) {
  uint16_t fieldKey;
  uint8_t numTableFields;
  uint8_t numTableRowFields;
  uint8_t numTableRows;
  int row;
  int cellIdx;
  int result;
  structmsg_t *structmsg;
  fieldInfo_t *fieldInfo;

  structmsg = structmsg_get_structmsg_ptr(handle);
  checkHandleOK(structmsg);
  if (c_strcmp(fieldStr, "@tablerows") == 0) {
    structmsg->msg.numTableRows = fieldLgth;
    fieldInfo_t *fieldInfo = &structmsg->msg.fieldInfos[structmsg->msg.numFieldInfos];
//ets_printf("tablerows1: totalLgth: %d cmdLgth: %d\n", structmsg->hdr.hdrInfo.hdrKeys.totalLgth, structmsg->hdr.hdrInfo.hdrKeys.cmdLgth);
    // we use 0 as numTableRows, that forces the *Lgth fields to NOT be modified!!
    fixHeaderInfo(structmsg, fieldInfo, fieldStr, fieldKey, fieldType, 0, 0);
//ets_printf("tablerows2: totalLgth: %d cmdLgth: %d\n", structmsg->hdr.hdrInfo.hdrKeys.totalLgth, structmsg->hdr.hdrInfo.hdrKeys.cmdLgth);
    structmsg->msg.numFieldInfos++;
    return STRUCT_MSG_ERR_OK;
  }
  if (c_strcmp(fieldStr, "@tablerowfields") == 0) {
    structmsg->msg.numTableRowFields = fieldLgth;
    numTableFields = structmsg->msg.numTableRows * structmsg->msg.numTableRowFields;
    fieldInfo_t *fieldInfo = &structmsg->msg.fieldInfos[structmsg->msg.numFieldInfos];
//ets_printf("tablerowfields1: totalLgth: %d cmdLgth: %d\n", structmsg->hdr.hdrInfo.hdrKeys.totalLgth, structmsg->hdr.hdrInfo.hdrKeys.cmdLgth);
    // we use 0 as numTableRows, that forces the *Lgth fields to NOT be modified!!
    fixHeaderInfo(structmsg, fieldInfo, fieldStr, fieldKey, fieldType, 0, 0);
//ets_printf("tablerowfields2: totalLgth: %d cmdLgth: %d\n", structmsg->hdr.hdrInfo.hdrKeys.totalLgth, structmsg->hdr.hdrInfo.hdrKeys.cmdLgth);
    if ((structmsg->msg.tableFieldInfos == NULL) && (numTableFields != 0)) {
      structmsg->msg.tableFieldInfos = newFieldInfos(numTableFields);
    }
    structmsg->msg.numFieldInfos++;
    return STRUCT_MSG_ERR_OK;
  }
  numTableRowFields = structmsg->msg.numTableRowFields;
  numTableRows = structmsg->msg.numTableRows;
  numTableFields = numTableRows * numTableRowFields;
  if (!((numTableFields > 0) && (structmsg->msg.numRowFields < numTableRowFields))) {
    fieldKey = structmsg->msg.numFieldInfos;
    if (structmsg->msg.numFieldInfos >= structmsg->msg.maxFieldInfos) {
      return STRUCT_MSG_ERR_TOO_MANY_FIELDS;
    }
    fieldInfo_t *fieldInfo = &structmsg->msg.fieldInfos[structmsg->msg.numFieldInfos];
    numTableFields = 0;
    numTableRows = 1;
    numTableRowFields = 0;
    fixHeaderInfo(structmsg, fieldInfo, fieldStr, fieldKey, fieldType, fieldLgth, numTableRows);
//ets_printf("field2: %s totalLgth: %d cmdLgth: %d\n", fieldInfo->fieldStr, structmsg->hdr.hdrInfo.hdrKeys.totalLgth, structmsg->hdr.hdrInfo.hdrKeys.cmdLgth);
    result = structmsg_fillHdrInfo(handle, structmsg);
    structmsg->msg.numFieldInfos++;
  } else {
    row = 0;
    while (row < numTableRows) {
      cellIdx = structmsg->msg.numRowFields + row * numTableRowFields;;
      fieldKey = structmsg->msg.numRowFields + 1;
      fieldInfo_t *fieldInfo = &structmsg->msg.tableFieldInfos[cellIdx];
//ets_printf("table field1: %s totalLgth: %d cmdLgth: %d\n", fieldInfo->fieldStr, structmsg->hdr.hdrInfo.hdrKeys.totalLgth, structmsg->hdr.hdrInfo.hdrKeys.cmdLgth);
      fixHeaderInfo(structmsg, fieldInfo, fieldStr, fieldKey, fieldType, fieldLgth, 1);
//ets_printf("table field2: %s totalLgth: %d cmdLgth: %d\n", fieldInfo->fieldStr, structmsg->hdr.hdrInfo.hdrKeys.totalLgth, structmsg->hdr.hdrInfo.hdrKeys.cmdLgth);
      row++;
    }
    structmsg->msg.numRowFields++;  
  } 
  return result;
}

// ============================= stmsg_setFillerAndCrc ========================

int stmsg_setFillerAndCrc(const uint8_t *handle) {
  structmsg_t *structmsg;
  int16_t fillerLgth = 0;
  int16_t myLgth = 0;
  int result;

  structmsg = structmsg_get_structmsg_ptr(handle);
  checkHandleOK(structmsg);
  myLgth = structmsg->hdr.hdrInfo.hdrKeys.cmdLgth + 2;
ets_printf("setfac: myLgth: %d\n", myLgth);
  while ((myLgth % 16) != 0) {
    myLgth++;
    fillerLgth++;
  }
  result = stmsg_addField(handle, "@filler", STRUCT_MSG_FIELD_UINT8_VECTOR, fillerLgth);
  checkErrOK(result);
  result = stmsg_addField(handle, "@crc", STRUCT_MSG_FIELD_UINT16_T, 2);
  checkErrOK(result);
  return STRUCT_MSG_ERR_OK;
}

// ============================= stmsg_setFieldValue ========================

int stmsg_setFieldValue(const uint8_t *handle, const uint8_t *fieldName, int numericValue, const uint8_t *stringValue) {
  structmsg_t *structmsg;
  fieldInfo_t *fieldInfo;
  int idx;
  int result;
  int numEntries;

  structmsg = structmsg_get_structmsg_ptr(handle);
  checkHandleOK(structmsg);
  if (c_strcmp(fieldName, "@src") == 0) {
    if (stringValue == NULL) {
      if ((numericValue >= 0) && (numericValue <= 65535)) {
        structmsg->hdr.hdrInfo.hdrKeys.src = (uint16_t)numericValue;
        setHandleField(handle, STRUCT_MSG_FIELD_SRC, structmsg->hdr.hdrInfo.hdrKeys.src);
        result = structmsg_fillHdrInfo(handle, structmsg);
        return result;
      } else {
        return STRUCT_MSG_ERR_VALUE_TOO_BIG;
      }
    } else {
      return STRUCT_MSG_ERR_BAD_VALUE;
    }
  } else {
    if (c_strcmp(fieldName, "@dst") == 0) {
      if (stringValue == NULL) {
        if ((numericValue >= 0) && (numericValue <= 65535)) {
          structmsg->hdr.hdrInfo.hdrKeys.dst = (uint16_t)numericValue;
          setHandleField(handle, STRUCT_MSG_FIELD_DST, structmsg->hdr.hdrInfo.hdrKeys.dst);
          result = structmsg_fillHdrInfo(handle, structmsg);
          return result;
        } else {
          return STRUCT_MSG_ERR_VALUE_TOO_BIG;
        }
      } else {
        return STRUCT_MSG_ERR_BAD_VALUE;
      }
    } else {
      if (c_strcmp(fieldName, "@cmdKey") == 0) {
        if (stringValue == NULL) {
          if ((numericValue >= 0) && (numericValue <= 65535)) {
            structmsg->hdr.hdrInfo.hdrKeys.cmdKey = (uint16_t)numericValue;
            setHandleField(handle, STRUCT_MSG_FIELD_CMD_KEY, structmsg->hdr.hdrInfo.hdrKeys.cmdKey);
            result = structmsg_fillHdrInfo(handle, structmsg);
            return result;
          } else {
            return STRUCT_MSG_ERR_VALUE_TOO_BIG;
          }
        } else {
          return STRUCT_MSG_ERR_BAD_VALUE;
        }
      }
    }
  }
  idx = 0;
  numEntries = structmsg->msg.numFieldInfos;
  while (idx < numEntries) {
    fieldInfo = &structmsg->msg.fieldInfos[idx];
    if (c_strcmp(fieldName, fieldInfo->fieldStr) == 0) {
      return setFieldValue(structmsg, fieldInfo, fieldName, numericValue, stringValue);
    }
    idx++;
  }
  return STRUCT_MSG_ERR_FIELD_NOT_FOUND;
}

// ============================= stmsg_setTableFieldValue ========================

int stmsg_setTableFieldValue(const uint8_t *handle, const uint8_t *fieldName, int row, int numericValue, const uint8_t *stringValue) {
  structmsg_t *structmsg;
  fieldInfo_t *fieldInfo;
  int idx;
  int cell;

  structmsg = structmsg_get_structmsg_ptr(handle);
  checkHandleOK(structmsg);
  if (row >= structmsg->msg.numTableRows) {
    return STRUCT_MSG_ERR_BAD_TABLE_ROW;
  }
  idx = 0;
  cell = 0 + row * structmsg->msg.numRowFields;
  while (idx < structmsg->msg.numRowFields) {
    fieldInfo = &structmsg->msg.tableFieldInfos[cell];
    if (c_strcmp(fieldName, fieldInfo->fieldStr) == 0) {
      return setFieldValue(structmsg, fieldInfo, fieldName, numericValue, stringValue);
    }
    idx++;
    cell++;
  }
  return STRUCT_MSG_ERR_FIELD_NOT_FOUND;
}


// ============================= stmsg_getFieldValue ========================

int stmsg_getFieldValue(const uint8_t *handle, const uint8_t *fieldName, int *numericValue, uint8_t **stringValue) {
  structmsg_t *structmsg;
  fieldInfo_t *fieldInfo;
  int idx;
  int numEntries;

  structmsg = structmsg_get_structmsg_ptr(handle);
  checkHandleOK(structmsg);
  *numericValue = 0;
  *stringValue = NULL;
  idx = 0;
  numEntries = structmsg->msg.numFieldInfos;
  while (idx < numEntries) {
    fieldInfo = &structmsg->msg.fieldInfos[idx];
    if (c_strcmp(fieldName, fieldInfo->fieldStr) == 0) {
      //ets_printf("    idx %d: key: %-20s\r\n", idx, fieldInfo->fieldStr);
      return getFieldValue(structmsg, fieldInfo, fieldName, numericValue, stringValue);
    }
    idx++;
  }
  return STRUCT_MSG_ERR_FIELD_NOT_FOUND;
}

// ============================= stmsg_getTableFieldValue ========================

int stmsg_getTableFieldValue(const uint8_t *handle, const uint8_t *fieldName, int row, int *numericValue, uint8_t **stringValue) {
  structmsg_t *structmsg;
  fieldInfo_t *fieldInfo;
  int idx;
  int cell;

  structmsg = structmsg_get_structmsg_ptr(handle);
  checkHandleOK(structmsg);
  if (row >= structmsg->msg.numTableRows) {
    return STRUCT_MSG_ERR_BAD_TABLE_ROW;
  }
  *numericValue = 0;
  *stringValue = NULL;

  idx = 0;
  cell = 0 + row * structmsg->msg.numRowFields;
  while (idx < structmsg->msg.numRowFields) {
    fieldInfo = &structmsg->msg.tableFieldInfos[cell];
    if (c_strcmp(fieldName, fieldInfo->fieldStr) == 0) {
      return getFieldValue(structmsg, fieldInfo, fieldName, numericValue, stringValue);
    }
    idx++;
    cell++;
  }
  return STRUCT_MSG_ERR_FIELD_NOT_FOUND;
}

// ============================= stmsg_setCrypted ========================

int stmsg_setCrypted(const uint8_t *handle, const uint8_t *crypted, int cryptedLgth) {
  structmsg_t *structmsg;
  fieldInfo_t *fieldInfo;

  structmsg = structmsg_get_structmsg_ptr(handle);
  checkHandleOK(structmsg);
  structmsg->encrypted = (uint8_t *)os_malloc(cryptedLgth);
  checkAllocOK(structmsg->encrypted);
  c_memcpy(structmsg->encrypted, crypted, cryptedLgth);
  return STRUCT_MSG_ERR_OK;
}

// ============================= stmsg_decryptGetHandle ========================

int stmsg_decryptGetHandle(const uint8_t *encryptedMsg, size_t mlen, const uint8_t *key, size_t klen, const uint8_t *iv, size_t ivlen, uint8_t **handle) {
  uint8_t *decrypted;
  size_t lgth;
  int result;

   decrypted = NULL;
   lgth = 0; 
   result = encryptdecrypt(encryptedMsg, mlen, key, klen, iv, ivlen, false, &decrypted, &lgth);
   if (result != STRUCT_MSG_ERR_OK) {
     return result;
   }
   result = getHandle(decrypted, handle);
   return result;
}
