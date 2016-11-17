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
 * File:   dataView.c
 * Author: Arnulf P. Wiedemann
 *
 * Created on September 24, 2016
 */

#include "osapi.h"
#include "c_types.h"
#include "mem.h"
#include "c_string.h"

#include "dataView.h"

static str2id_t fieldType2Ids[] = {
  {"none",      DATA_VIEW_FIELD_NONE},
  {"uint8_t",   DATA_VIEW_FIELD_UINT8_T},
  {"int8_t",    DATA_VIEW_FIELD_INT8_T},
  {"uint16_t",  DATA_VIEW_FIELD_UINT16_T},
  {"int16_t",   DATA_VIEW_FIELD_INT16_T},
  {"uint32_t",  DATA_VIEW_FIELD_UINT32_T},
  {"int32_t",   DATA_VIEW_FIELD_INT32_T},
  {"uint8_t*",  DATA_VIEW_FIELD_UINT8_VECTOR},
  {"int8_t*",   DATA_VIEW_FIELD_INT8_VECTOR},
  {"uint16_t*", DATA_VIEW_FIELD_UINT16_VECTOR},
  {"int16_t*",  DATA_VIEW_FIELD_INT16_VECTOR},
  {"uint32_t*", DATA_VIEW_FIELD_UINT32_VECTOR},
  {"int32_t*",  DATA_VIEW_FIELD_INT32_VECTOR},
  {NULL, -1},
};

static uint8_t dataViewId;

// ================================= getFieldTypeIdFromStr ====================================

static uint8_t getFieldTypeIdFromStr(dataView_t *self, const uint8_t *fieldTypeStr, uint8_t *fieldTypeId) {
  str2id_t *entry;

  entry = &fieldType2Ids[0];
  while (entry->str != NULL) {
    if (c_strcmp(fieldTypeStr, entry->str) == 0) {
      *fieldTypeId = entry->id;
      return DATA_VIEW_ERR_OK;
    }
    entry++;
  }
  return DATA_VIEW_ERR_FIELD_TYPE_NOT_FOUND;
}

// ================================= getFieldTypeStrFromId ====================================

static uint8_t getFieldTypeStrFromId(dataView_t *self, uint8_t fieldTypeId, uint8_t **fieldTypeStr) {
  str2id_t *entry;

  entry = &fieldType2Ids[0];
  while (entry->str != NULL) {
    if (fieldTypeId == entry->id) {
      *fieldTypeStr = entry->str;
      return DATA_VIEW_ERR_OK;
    }
    entry++;
  }
  return DATA_VIEW_ERR_FIELD_TYPE_NOT_FOUND;
}

// ================================= getUint8 ====================================

static uint8_t getUint8(dataView_t *self, int offset, uint8_t *value) {
  if (offset > self->lgth) {
    return DATA_VIEW_ERR_OUT_OF_RANGE;
  }
  *value = self->data[offset];
  return DATA_VIEW_ERR_OK;
}

// ================================= getInt8 ====================================

static uint8_t getInt8(dataView_t *self, int offset, int8_t *value) {
  if (offset > self->lgth) {
    return DATA_VIEW_ERR_OUT_OF_RANGE;
  }
  *value = (int8_t)self->data[offset];
  return DATA_VIEW_ERR_OK;
}

// ================================= setUint8 ====================================

static uint8_t setUint8(dataView_t *self, int offset, uint8_t value) {
  if (offset > self->lgth) {
    return DATA_VIEW_ERR_OUT_OF_RANGE;
  }
  self->data[offset] = value;
  return DATA_VIEW_ERR_OK;
}

// ================================= setInt8 ====================================

static uint8_t setInt8(dataView_t *self, int offset, int8_t value) {
  if (offset > self->lgth) {
    return DATA_VIEW_ERR_OUT_OF_RANGE;
  }
  self->data[offset] = value & 0xFF;
  return DATA_VIEW_ERR_OK;
}


// ================================= getUint16 ====================================

static uint8_t getUint16(dataView_t *self, int offset, uint16_t *value) {
  if (offset + 1 > self->lgth) {
    return DATA_VIEW_ERR_OUT_OF_RANGE;
  }
  *value = 0;
  *value += (self->data[offset++] & 0xFF) << 8;
  *value += (self->data[offset] & 0xFF) << 0;
  return DATA_VIEW_ERR_OK;
}

// ================================= getInt16 ====================================

static uint8_t getInt16(dataView_t *self, int offset, int16_t *value) {
  if (offset + 1 > self->lgth) {
    return DATA_VIEW_ERR_OUT_OF_RANGE;
  }
  *value = 0;
  *value += (self->data[offset++] & 0xFF) << 8;
  *value += (self->data[offset] & 0xFF) << 0;
  return DATA_VIEW_ERR_OK;
}

// ================================= setUint16 ====================================

static uint8_t setUint16(dataView_t *self, int offset, uint16_t value) {
  if (offset + 1 > self->lgth) {
    return DATA_VIEW_ERR_OUT_OF_RANGE;
  }
  self->data[offset++] = (value >> 8) & 0xFF;
  self->data[offset] = value & 0xFF;
  return DATA_VIEW_ERR_OK;
}

// ================================= setInt16 ====================================

static uint8_t setInt16(dataView_t *self, int offset, int16_t value) {
  if (offset + 1 > self->lgth) {
    return DATA_VIEW_ERR_OUT_OF_RANGE;
  }
  self->data[offset++] = (value >> 8) & 0xFF;
  self->data[offset] = value & 0xFF;
  return DATA_VIEW_ERR_OK;
}


// ================================= getUint32 ====================================

static uint8_t getUint32(dataView_t *self, int offset, uint32_t *value) {
  if (offset + 3 > self->lgth) {
    return DATA_VIEW_ERR_OUT_OF_RANGE;
  }
  *value = 0;
  *value += (self->data[offset++] & 0xFF) << 24;
  *value += (self->data[offset++] & 0xFF) << 16;
  *value += (self->data[offset++] & 0xFF) << 8;
  *value += (self->data[offset] & 0xFF) << 0;
  return DATA_VIEW_ERR_OK;
}

// ================================= getInt32 ====================================

static uint8_t getInt32(dataView_t *self, int offset, int32_t *value) {
  if (offset + 3 > self->lgth) {
    return DATA_VIEW_ERR_OUT_OF_RANGE;
  }
  *value = 0;
  *value += (self->data[offset++] & 0xFF) << 24;
  *value += (self->data[offset++] & 0xFF) << 16;
  *value += (self->data[offset++] & 0xFF) << 8;
  *value += (self->data[offset] & 0xFF) << 0;
  return DATA_VIEW_ERR_OK;
}

// ================================= setUint32 ====================================

static uint8_t setUint32(dataView_t *self, int offset, uint32_t value) {
  if (offset + 3 > self->lgth) {
    return DATA_VIEW_ERR_OUT_OF_RANGE;
  }
  self->data[offset++] = (value >> 24) & 0xFF;
  self->data[offset++] = (value >> 16) & 0xFF;
  self->data[offset++] = (value >> 8) & 0xFF;
  self->data[offset] = value & 0xFF;
  return DATA_VIEW_ERR_OK;
}

// ================================= setInt32 ====================================

static uint8_t setInt32(dataView_t *self, int offset, int32_t value) {
  if (offset + 3 > self->lgth) {
    return DATA_VIEW_ERR_OUT_OF_RANGE;
  }
  self->data[offset++] = (value >> 24) & 0xFF;
  self->data[offset++] = (value >> 16) & 0xFF;
  self->data[offset++] = (value >> 8) & 0xFF;
  self->data[offset] = value & 0xFF;
  return DATA_VIEW_ERR_OK;
}


// ================================= getUint8Vector ====================================

static uint8_t getUint8Vector(dataView_t *self, int offset, uint8_t **value, size_t lgth) {
  if (offset + lgth > self->lgth) {
    return DATA_VIEW_ERR_OUT_OF_RANGE;
  }
  c_memcpy(*value,self->data+offset,lgth);
  return DATA_VIEW_ERR_OK;
}

// ================================= getInt8Vector ====================================

static uint8_t getInt8Vector(dataView_t *self, int offset, int8_t **value, size_t lgth) {
  if (offset + lgth > self->lgth) {
    return DATA_VIEW_ERR_OUT_OF_RANGE;
  }
  c_memcpy(*value,self->data+offset,lgth);
  return DATA_VIEW_ERR_OK;
}

// ================================= setUint8Vector ====================================

static uint8_t setUint8Vector(dataView_t *self, int offset, uint8_t *value, size_t lgth) {
  if (offset + lgth > self->lgth) {
    return DATA_VIEW_ERR_OUT_OF_RANGE;
  }
  c_memcpy(self->data+offset,value,lgth);
  return DATA_VIEW_ERR_OK;
}

// ================================= setInt8Vector ====================================

static uint8_t setInt8Vector(dataView_t *self, int offset, int8_t *value, size_t lgth) {
  if (offset + lgth > self->lgth) {
    return DATA_VIEW_ERR_OUT_OF_RANGE;
  }
  c_memcpy(self->data+offset,value,lgth);
  return DATA_VIEW_ERR_OK;
}


// ================================= getUint16Vector ====================================

static uint8_t getUint16Vector(dataView_t *self, int offset, uint16_t **value, size_t lgth) {
  int idx;
  int result;

  if (offset + lgth * sizeof(uint16_t) > self->lgth) {
    return DATA_VIEW_ERR_OUT_OF_RANGE;
  }
  idx = 0;
  while (idx < lgth) {
    result = getUint16(self, offset, &((*value)[idx]));
    checkErrOK(result);
    idx++;
  }
  return DATA_VIEW_ERR_OK;
}

// ================================= getInt16Vector ====================================

static uint8_t getInt16Vector(dataView_t *self, int offset, int16_t **value, size_t lgth) {
  int idx;
  int result;

  if (offset + lgth * sizeof(uint16_t) > self->lgth) {
    return DATA_VIEW_ERR_OUT_OF_RANGE;
  }
  idx = 0;
  while (idx < lgth) {
    result = getInt16(self, offset, &((*value)[idx]));
    checkErrOK(result);
    idx++;
  }
  return DATA_VIEW_ERR_OK;
}

// ================================= setUint16Vector ====================================

static uint8_t setUint16Vector(dataView_t *self, int offset, uint16_t *value, size_t lgth) {
  int idx;
  int result;

  if (offset + lgth * sizeof(uint16_t) > self->lgth) {
    return DATA_VIEW_ERR_OUT_OF_RANGE;
  }
  idx = 0;
  while (idx < lgth) {
    result = setUint16(self, offset, value[idx]);
    checkErrOK(result);
    idx++;
  }
  return DATA_VIEW_ERR_OK;
}

// ================================= setInt16Vector ====================================

static uint8_t setInt16Vector(dataView_t *self, int offset, int16_t *value, size_t lgth) {
  int idx;
  int result;

  if (offset + lgth * sizeof(uint16_t) > self->lgth) {
    return DATA_VIEW_ERR_OUT_OF_RANGE;
  }
  idx = 0;
  while (idx < lgth) {
    result = setInt16(self, offset, value[idx]);
    checkErrOK(result);
    idx++;
  }
  return DATA_VIEW_ERR_OK;
}


// ================================= getUint32Vector ====================================

static uint8_t getUint32Vector(dataView_t *self, int offset, uint32_t **value, size_t lgth) {
  int idx;
  int result;

  if (offset + lgth * sizeof(uint32_t) > self->lgth) {
    return DATA_VIEW_ERR_OUT_OF_RANGE;
  }
  idx = 0;
  while (idx < lgth) {
    result = getUint32(self, offset, &((*value)[idx]));
    checkErrOK(result);
    idx++;
  }
  return DATA_VIEW_ERR_OK;
}

// ================================= getInt32Vector ====================================

static uint8_t getInt32Vector(dataView_t *self, int offset, int32_t **value, size_t lgth) {
  int idx;
  int result;

  if (offset + lgth * sizeof(uint32_t) > self->lgth) {
    return DATA_VIEW_ERR_OUT_OF_RANGE;
  }
  idx = 0;
  while (idx < lgth) {
    result = getInt32(self, offset, &((*value)[idx]));
    checkErrOK(result);
    idx++;
  }
  return DATA_VIEW_ERR_OK;
}

// ================================= setUint32Vector ====================================

static uint8_t setUint32Vector(dataView_t *self, int offset, uint32_t *value, size_t lgth) {
  int idx;
  int result;

  if (offset + lgth * sizeof(uint32_t) > self->lgth) {
    return DATA_VIEW_ERR_OUT_OF_RANGE;
  }
  idx = 0;
  while (idx < lgth) {
    result = setUint32(self, offset, value[idx]);
    checkErrOK(result);
    idx++;
  }
  return DATA_VIEW_ERR_OK;
}

// ================================= setInt32Vector ====================================

static uint8_t setInt32Vector(dataView_t *self, int offset, int32_t *value, size_t lgth) {
  int idx;
  int result;

  if (offset + lgth * sizeof(uint32_t) > self->lgth) {
    return DATA_VIEW_ERR_OUT_OF_RANGE;
  }
  idx = 0;
  while (idx < lgth) {
    result = setInt32(self, offset, value[idx]);
    checkErrOK(result);
    idx++;
  }
  return DATA_VIEW_ERR_OK;
}


// ================================= setData ====================================

static uint8_t setData(dataView_t *self, size_t lgth) {
  if (self->data != NULL) {
//ets_printf("dataView freeing data in setData %p\n", self->data);
//    os_free(self->data);
  }
  self->data = os_zalloc(lgth);
  if (self->data == NULL) {
    return DATA_VIEW_ERR_OUT_OF_MEMORY;
  }
  self->lgth = lgth;
  return DATA_VIEW_ERR_OK;
}

// ============================= dumpBinary ========================

static void dumpBinary(const uint8_t *data, uint8_t lgth, const uint8_t *where) {
  int idx;

  ets_printf("%s\n", where);
  idx = 0;
  while (idx < lgth) {
     ets_printf("idx: %d ch: %c 0x%02x\n", idx, data[idx], data[idx] & 0xFF);
    idx++;
  }
}

// ================================= newDataView ====================================

dataView_t *newDataView(void) {
  dataView_t *dataView = os_zalloc(sizeof(dataView_t));
  if (dataView == NULL) {
    return NULL;
  }
  dataView->data = NULL;
  dataView->lgth = 0;
  dataViewId++;
  dataView->id = dataViewId;

  dataView->getFieldTypeIdFromStr = &getFieldTypeIdFromStr;
  dataView->getFieldTypeStrFromId = &getFieldTypeStrFromId;

  dataView->getUint8 = &getUint8;
  dataView->getInt8 = &getInt8;
  dataView->setUint8 = &setUint8;
  dataView->setInt8 = &setInt8;

  dataView->getUint16 = &getUint16;
  dataView->getInt16 = &getInt16;
  dataView->setUint16 = &setUint16;
  dataView->setInt16 = &setInt16;

  dataView->getUint32 = &getUint32;
  dataView->getInt32 = &getInt32;
  dataView->setUint32 = &setUint32;
  dataView->setInt32 = &setInt32;

  dataView->getUint8Vector = &getUint8Vector;
  dataView->getInt8Vector = &getInt8Vector;
  dataView->setUint8Vector = &setUint8Vector;
  dataView->setInt8Vector = &setInt8Vector;

  dataView->getUint16Vector = &getUint16Vector;
  dataView->getInt16Vector = &getInt16Vector;
  dataView->setUint16Vector = &setUint16Vector;
  dataView->setInt16Vector = &setInt16Vector;

  dataView->getUint32Vector = &getUint32Vector;
  dataView->getInt32Vector = &getInt32Vector;
  dataView->setUint32Vector = &setUint32Vector;
  dataView->setInt32Vector = &setInt32Vector;

  dataView->setData = &setData;
  dataView->dumpBinary = &dumpBinary;
  return dataView;
}

// ================================= freeDataView ====================================

void freeDataView(dataView_t *dataView) {
  if (dataView->data != NULL) {
    os_free(dataView->data);
    dataView->data = NULL;
    os_free(dataView);
  }
}


