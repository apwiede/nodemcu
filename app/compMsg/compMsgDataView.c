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
 * File:   compMsgDataView.c
 * Author: Arnulf P. Wiedemann
 *
 * Created on September 24, 2016
 */

#include "osapi.h"
#include "c_types.h"
#include "mem.h"
#include "c_string.h"
#include "c_stdlib.h"
#include "compMsgDispatcher.h"

static int sequenceNum = 0;
static uint8_t compMsgDataViewId;

// ================================= getRandomNum ====================================

static uint8_t getRandomNum(compMsgDispatcher_t *self, dataView_t *dataView, compMsgField_t *fieldInfo, uint32_t *value) {
  return dataView->getUint32(dataView, fieldInfo->fieldOffset, value);
}

// ================================= setRandomNum ====================================

static uint8_t setRandomNum(compMsgDispatcher_t *self, dataView_t *dataView, compMsgField_t *fieldInfo) {
  uint32_t val;

  val = (uint32_t)(rand() & RAND_MAX);
  return dataView->setUint32(dataView, fieldInfo->fieldOffset, val);
}


// ================================= getSequenceNum ====================================

static uint8_t getSequenceNum(compMsgDispatcher_t *self, dataView_t *dataView, compMsgField_t *fieldInfo, uint32_t *value) {
  return dataView->getUint32(dataView, fieldInfo->fieldOffset, value);
}

// ================================= setSavedSequenceNum ====================================

static uint8_t setSavedSequenceNum(compMsgDispatcher_t *self, dataView_t *dataView, compMsgField_t *fieldInfo) {
  uint8_t result;
  int seqeunceNum;
  uint8_t *stringValue;

  result = self->compMsgWifiData->getClientSequenceNum(self, &sequenceNum, &stringValue);
  checkErrOK(result);
  return dataView->setUint32(dataView, fieldInfo->fieldOffset, sequenceNum);
}

// ================================= setSequenceNum ====================================

static uint8_t setSequenceNum(compMsgDispatcher_t *self, dataView_t *dataView, compMsgField_t *fieldInfo) {
  return dataView->setUint32(dataView, fieldInfo->fieldOffset, sequenceNum++);
}

// ================================= getFiller ====================================

static uint8_t getFiller(compMsgDispatcher_t *self, dataView_t *dataView, compMsgField_t *fieldInfo, uint8_t **value) {
  size_t lgth;

  lgth = fieldInfo->fieldLgth;
  if (fieldInfo->fieldOffset + lgth > dataView->lgth) {
    return DATA_VIEW_ERR_OUT_OF_RANGE;
  }
  c_memcpy(*value, dataView->data + fieldInfo->fieldOffset,lgth);
}

// ================================= setFiller ====================================

static uint8_t setFiller(compMsgDispatcher_t *self, dataView_t *dataView, compMsgField_t *fieldInfo) {
  uint32_t val;
  int idx;
  int lgth;
  int result;
  size_t offset;

  lgth = fieldInfo->fieldLgth;
  offset = fieldInfo->fieldOffset;
  idx = 0;
  while (lgth >= 4) {
    val = (uint32_t)(rand() & RAND_MAX);
    result = dataView->setUint32(dataView, offset, val);
    checkErrOK(result);
    offset += 4;
    lgth -= 4;
  }
  while (lgth >= 2) {
    val = (uint16_t)((rand() & RAND_MAX) & 0xFFFF);
    result = dataView->setUint16(dataView, offset, val);
    checkErrOK(result);
    offset += 2;
    lgth -= 2;
  }
  while (lgth >= 1) {
    val = (uint8_t)((rand() & RAND_MAX) & 0xFF);
    result = dataView->setUint8(dataView, offset, val);
    checkErrOK(result);
    offset++;
    lgth -= 1;
  }
  return DATA_VIEW_ERR_OK;
}

// ================================= setZeroFiller ====================================

static uint8_t setZeroFiller(compMsgDispatcher_t *self, dataView_t *dataView, compMsgField_t *fieldInfo) {
  uint8_t val;
  int idx;
  int lgth;
  int result;
  size_t offset;

  lgth = fieldInfo->fieldLgth;
  offset = fieldInfo->fieldOffset;
  val = 0;
  idx = 0;
  while (idx < lgth) {
    result = dataView->setUint8(dataView, offset, val);
    checkErrOK(result);
    offset += 1;
    idx++;
  }
  return DATA_VIEW_ERR_OK;
}


// ================================= getCrc ====================================

static uint8_t getCrc(compMsgDispatcher_t *self, dataView_t *dataView, compMsgField_t *fieldInfo, size_t startOffset, size_t lgth) {
  uint16_t crcVal;
  uint16_t crc;
  uint8_t uint8_crc;
  int crcLgth;
  int idx;
  int result;

  COMP_MSG_DBG(self, "v", 2, "getCrc: startOffset: %d lgth: %d", startOffset, lgth);
  crcLgth = fieldInfo->fieldLgth;
  crcVal = 0;
  idx = startOffset;
  while (idx < lgth) {
if ((idx % 10) == 0) {
    COMP_MSG_DBG(self, "v", 2, "crc idx: %d ch: 0x%02x crc: 0x%04x", idx - startOffset, dataView->data[idx], crcVal);
}
    crcVal += dataView->data[idx++];
  }
  COMP_MSG_DBG(self, "v", 2, "crc idx: %d ch: 0x%02x crc: 0x%04x", idx - startOffset, dataView->data[idx], crcVal);
  COMP_MSG_DBG(self, "v", 2, "crcVal00: 0x%04x\n", crcVal);
  crcVal = ~(crcVal);
  if (crcLgth == 1) {
    COMP_MSG_DBG(self, "v", 2, "crcVal10: 0x%04x", crcVal);
    crcVal = crcVal & 0xFF;
    result = dataView->getUint8(dataView, fieldInfo->fieldOffset, &uint8_crc);
    checkErrOK(result);
    COMP_MSG_DBG(self, "v", 2, "crcVal1: 0x%02x crc: 0x%02x", crcVal, uint8_crc);
    if (crcVal != uint8_crc) {
      COMP_MSG_DBG(self, "v", 2, "bad crcVal1: 0x%02x crc: 0x%02x", crcVal, uint8_crc);
      return COMP_MSG_ERR_BAD_CRC_VALUE;
    }
  } else {
    result = dataView->getUint16(dataView, fieldInfo->fieldOffset, &crc);
    checkErrOK(result);
    COMP_MSG_DBG(self, "v", 2, "crcVal2: 0x%04x crc: 0x%04x", crcVal, crc);
    if (crcVal != crc) {
      COMP_MSG_DBG(self, "v", 2, "bad crcVal2: 0x%04x crc: 0x%04x", crcVal, crc);
      return COMP_MSG_ERR_BAD_CRC_VALUE;
    }
  }
  return DATA_VIEW_ERR_OK;
}

// ================================= setCrc ====================================

static uint8_t setCrc(compMsgDispatcher_t *self, dataView_t *dataView, compMsgField_t *fieldInfo, size_t startOffset, size_t lgth) {
  int idx;
  uint16_t crc;

  crc = 0;
  idx = startOffset;
  COMP_MSG_DBG(self, "v", 2, "setCrc startOffset: %d ch: 0x%02x crc: 0x%04x lgth: %d", startOffset, dataView->data[idx], crc, lgth);
  while (idx < lgth) {
if ((idx % 10) == 0) {
    COMP_MSG_DBG(self, "v", 2, "crc idx: %d ch: 0x%02x crc: 0x%04x", idx - startOffset, dataView->data[idx], crc);
}
    crc += dataView->data[idx++];
  }
  crc = ~(crc);
  if (fieldInfo->fieldLgth == 1) {
    COMP_MSG_DBG(self, "v", 2, "crc8: 0x%04x 0x%02x", crc, (uint8_t)(crc & 0xFF));
    dataView->setUint8(dataView,fieldInfo->fieldOffset,(uint8_t)(crc & 0xFF));
  } else {
    dataView->setUint16(dataView,fieldInfo->fieldOffset,crc);
  }
  COMP_MSG_DBG(self, "v", 2, "crc: 0x%04x", crc);
  return DATA_VIEW_ERR_OK;
}

// ================================= getTotalCrc ====================================

static uint8_t getTotalCrc(compMsgDispatcher_t *self, dataView_t *dataView, compMsgField_t *fieldInfo) {
  uint16_t crcVal;
  uint16_t crc;
  uint8_t uint8_crc;
  int crcLgth;
  int idx;
  int result;

  crcLgth = fieldInfo->fieldLgth;
  crcVal = 0;
  idx = 0;
  COMP_MSG_DBG(self, "v", 2, "getTotalCrc");
  COMP_MSG_DBG(self, "v", 2, "getTotalCrc idx: %d ch: 0x%02x crc: 0x%04x fieldOffset: %d", idx, dataView->data[idx], crc, fieldInfo->fieldOffset);
  while (idx < fieldInfo->fieldOffset) {
if ((idx % 10) == 0) {
    COMP_MSG_DBG(self, "v", 2, "totalCrc idx: %d ch: 0x%02x crc: 0x%04x", idx, dataView->data[idx], crcVal);
}
    crcVal += dataView->data[idx++];
  }
  COMP_MSG_DBG(self, "v", 2, "totalCrcVal00: 0x%04x", crcVal);
  crcVal = ~(crcVal);
  if (crcLgth == 1) {
    COMP_MSG_DBG(self, "v", 2, "totalCrcVal10: 0x%04x", crcVal);
    crcVal = crcVal & 0xFF;
    result = dataView->getUint8(dataView, fieldInfo->fieldOffset, &uint8_crc);
    checkErrOK(result);
    COMP_MSG_DBG(self, "v", 2, "totalCrcVal1: 0x%02x crc: 0x%02x", crcVal, uint8_crc);
    if (crcVal != uint8_crc) {
      COMP_MSG_DBG(self, "v", 1, "totalCrcVal1: 0x%02x crc: 0x%02x", crcVal, uint8_crc);
      return COMP_MSG_ERR_BAD_CRC_VALUE;
    }
  } else {
    result = dataView->getUint16(dataView, fieldInfo->fieldOffset, &crc);
    checkErrOK(result);
    COMP_MSG_DBG(self, "v", 2, "totalCrcVal2: 0x%04x crc: 0x%04x", crcVal, crc);
    if (crcVal != crc) {
      COMP_MSG_DBG(self, "v", 1, "totalCrcVal2: 0x%04x crc: 0x%04x", crcVal, crc);
      return COMP_MSG_ERR_BAD_CRC_VALUE;
    }
  }
  return DATA_VIEW_ERR_OK;
}

// ================================= setTotalCrc ====================================

static uint8_t setTotalCrc(compMsgDispatcher_t *self, dataView_t *dataView, compMsgField_t *fieldInfo) {
  int idx;
  uint16_t crc;

  crc = 0;
  idx = 0;
  COMP_MSG_DBG(self, "v", 2, "setTotalCrc idx: %d ch: 0x%02x crc: 0x%04x fieldOffset: %d", idx, dataView->data[idx], crc, fieldInfo->fieldOffset);
  while (idx < fieldInfo->fieldOffset) {
if ((idx % 10) == 0) {
    COMP_MSG_DBG(self, "v", 2, "totalCrc idx: %d ch: 0x%02x crc: 0x%04x", idx, dataView->data[idx], crc);
}
    crc += dataView->data[idx++];
  }
  crc = ~(crc);
  if (fieldInfo->fieldLgth == 1) {
    COMP_MSG_DBG(self, "v", 2, "totalCrc8: 0x%04x 0x%02x", crc, (uint8_t)(crc & 0xFF));
    dataView->setUint8(dataView,fieldInfo->fieldOffset,(uint8_t)(crc & 0xFF));
  } else {
    dataView->setUint16(dataView,fieldInfo->fieldOffset,crc);
  }
  COMP_MSG_DBG(self, "v", 2, "totalCrc: 0x%04x", crc);
  return DATA_VIEW_ERR_OK;
}

// ================================= getFieldValue ====================================

static uint8_t getFieldValue(compMsgDispatcher_t *self, dataView_t *dataView, compMsgField_t *fieldInfo, int *numericValue, uint8_t **stringValue, int fieldIdx) {
  int idx;
  int result;
  int numEntries;
  int8_t i8;
  uint8_t ui8;
  int16_t i16;
  uint16_t ui16;
  int32_t i32;
  uint32_t ui32;

  *stringValue = NULL;
  *numericValue = 0;
  switch (fieldInfo->fieldTypeId) {
    case DATA_VIEW_FIELD_INT8_T:
      result = dataView->getInt8(dataView, fieldInfo->fieldOffset, &i8);
      checkErrOK(result);
      *numericValue = (int)i8;
      break;
    case DATA_VIEW_FIELD_UINT8_T:
      result = dataView->getUint8(dataView, fieldInfo->fieldOffset, &ui8);
      checkErrOK(result);
      *numericValue = (int)ui8;
      break;
    case DATA_VIEW_FIELD_INT16_T:
      result = dataView->getInt16(dataView, fieldInfo->fieldOffset, &i16);
      checkErrOK(result);
      *numericValue = (int)i16;
      break;
    case DATA_VIEW_FIELD_UINT16_T:
      result = dataView->getUint16(dataView, fieldInfo->fieldOffset, &ui16);
      checkErrOK(result);
      *numericValue = (int)ui16;
      break;
    case DATA_VIEW_FIELD_INT32_T:
      result = dataView->getInt32(dataView, fieldInfo->fieldOffset, &i32);
      checkErrOK(result);
      *numericValue = (int)i32;
      break;
    case DATA_VIEW_FIELD_UINT32_T:
      result = dataView->getUint32(dataView, fieldInfo->fieldOffset, &ui32);
      checkErrOK(result);
      *numericValue = (int)ui32;
      break;
    case DATA_VIEW_FIELD_INT8_VECTOR:
      *numericValue = fieldInfo->fieldLgth;
      *stringValue = os_zalloc(fieldInfo->fieldLgth+1);
      checkAllocOK(stringValue);
      (*stringValue)[fieldInfo->fieldLgth] = 0;
      os_memcpy(*stringValue, dataView->data + fieldInfo->fieldOffset, fieldInfo->fieldLgth);
      break;
    case DATA_VIEW_FIELD_UINT8_VECTOR:
      *stringValue = os_zalloc(fieldInfo->fieldLgth+1);
      checkAllocOK(stringValue);
      (*stringValue)[fieldInfo->fieldLgth] = 0;
      os_memcpy(*stringValue, dataView->data + fieldInfo->fieldOffset, fieldInfo->fieldLgth);
      break;
    case DATA_VIEW_FIELD_INT16_VECTOR:
      if (*stringValue == NULL) {
        // check for length needed!!
        result = dataView->getInt16(dataView, fieldInfo->fieldOffset + fieldIdx * sizeof(int16_t), &i16);
        checkErrOK(result);
        *numericValue = (int)i16;
      } else {
        return COMP_MSG_ERR_BAD_VALUE;
      }
      break;
    case DATA_VIEW_FIELD_UINT16_VECTOR:
      if (*stringValue == NULL) {
        // check for length needed!!
        result = dataView->getUint16(dataView, fieldInfo->fieldOffset + fieldIdx * sizeof(uint16_t), &ui16);
        checkErrOK(result);
        *numericValue = (int)ui16;
      } else {
        return COMP_MSG_ERR_BAD_VALUE;
      }
      break;
#ifdef NOTDEF
    case DATA_VIEW_FIELD_INT32_VECTOR:
      if (stringValue != NULL) {
        // check for length needed!!
        os_memcpy((int8_t *)fieldInfo->value.int32Vector, stringValue, fieldInfo->fieldLgth);
      } else {
        return COMP_MSG_ERR_BAD_VALUE;
      }
      break;
    case DATA_VIEW_FIELD_UINT32_VECTOR:
      if (stringValue != NULL) {
        // check for length needed!!
        os_memcpy((uint8_t *)fieldInfo->value.uint32Vector, stringValue, fieldInfo->fieldLgth);
      } else {
        return COMP_MSG_ERR_BAD_VALUE;
      }
      break;
#endif
    default:
      return COMP_MSG_ERR_BAD_FIELD_TYPE;
      break;
  }
  return DATA_VIEW_ERR_OK;
}

// ================================= setFieldValue ====================================

static uint8_t setFieldValue(compMsgDispatcher_t *self, dataView_t *dataView, compMsgField_t *fieldInfo, int numericValue, const uint8_t *stringValue, int fieldIdx) {
  int idx;
  int result;
  int numEntries;

  COMP_MSG_DBG(self, "v", 2, "compMsgDataView setFieldValue: data: %p lgth: %d", dataView->data, dataView->lgth);
  switch (fieldInfo->fieldTypeId) {
    case DATA_VIEW_FIELD_INT8_T:
      if (stringValue == NULL) {
        if ((numericValue > -128) && (numericValue < 128)) {
          result= dataView->setInt8(dataView, fieldInfo->fieldOffset + fieldIdx, (int8_t)numericValue);
          checkErrOK(result);
        } else {
          return COMP_MSG_ERR_VALUE_TOO_BIG;
        }
      } else {
        return COMP_MSG_ERR_BAD_VALUE;
      }
      break;
    case DATA_VIEW_FIELD_UINT8_T:
      if (stringValue == NULL) {
        if ((numericValue >= 0) && (numericValue <= 256)) {
          result= dataView->setUint8(dataView, fieldInfo->fieldOffset + fieldIdx, (uint8_t)numericValue);
          checkErrOK(result);
        }
      } else {
        return COMP_MSG_ERR_BAD_VALUE;
      }
      break;
    case DATA_VIEW_FIELD_INT16_T:
      if (stringValue == NULL) {
        if ((numericValue > -32767) && (numericValue < 32767)) {
          result= dataView->setInt16(dataView, fieldInfo->fieldOffset + fieldIdx, (int16_t)numericValue);
          checkErrOK(result);
        } else {
          return COMP_MSG_ERR_VALUE_TOO_BIG;
        }
      } else {
        return COMP_MSG_ERR_BAD_VALUE;
      }
      break;
    case DATA_VIEW_FIELD_UINT16_T:
      if (stringValue == NULL) {
        if ((numericValue >= 0) && (numericValue <= 65535)) {
          result= dataView->setUint16(dataView, fieldInfo->fieldOffset + fieldIdx, (uint16_t)numericValue);
          checkErrOK(result);
        } else {
          return COMP_MSG_ERR_VALUE_TOO_BIG;
        }
      } else {
        return COMP_MSG_ERR_BAD_VALUE;
      }
      break;
    case DATA_VIEW_FIELD_INT32_T:
      if (stringValue == NULL) {
        if ((numericValue > -0x7FFFFFFF) && (numericValue <= 0x7FFFFFFF)) {
          result= dataView->setInt32(dataView, fieldInfo->fieldOffset + fieldIdx, (int32_t)numericValue);
          checkErrOK(result);
        } else {
          return COMP_MSG_ERR_VALUE_TOO_BIG;
        }
      } else {
        return COMP_MSG_ERR_BAD_VALUE;
      }
      break;
    case DATA_VIEW_FIELD_UINT32_T:
      if (stringValue == NULL) {
        // we have to do the signed check as numericValue is a signed integer!!
        if ((numericValue > -0x7FFFFFFF) && (numericValue <= 0x7FFFFFFF)) {
          result= dataView->setUint32(dataView, fieldInfo->fieldOffset + fieldIdx, (uint32_t)numericValue);
          checkErrOK(result);
        } else {
          return COMP_MSG_ERR_VALUE_TOO_BIG;
        }
      } else {
        return COMP_MSG_ERR_BAD_VALUE;
      }
      break;
    case DATA_VIEW_FIELD_INT8_VECTOR:
      if (stringValue != NULL) {
        // check for length needed!!
        if (fieldInfo->fieldOffset + fieldInfo->fieldLgth > dataView->lgth) {
          return COMP_MSG_ERR_VALUE_TOO_BIG;
        }
        os_memcpy(dataView->data + fieldInfo->fieldOffset + fieldIdx, stringValue, fieldInfo->fieldLgth);
      } else {
        return COMP_MSG_ERR_BAD_VALUE;
      }
      break;
    case DATA_VIEW_FIELD_UINT8_VECTOR:
      if (stringValue != NULL) {
        if (fieldInfo->fieldOffset + fieldInfo->fieldLgth > dataView->lgth) {
          return COMP_MSG_ERR_VALUE_TOO_BIG;
        }
        COMP_MSG_DBG(self, "v", 2, "compMsgDataView: u8vector: offset: %d lgth: %d val: %s", fieldInfo->fieldOffset + fieldIdx, fieldInfo->fieldLgth, stringValue);
        os_memcpy(dataView->data + fieldInfo->fieldOffset + fieldIdx, stringValue, fieldInfo->fieldLgth);
      } else {
        return COMP_MSG_ERR_BAD_VALUE;
      }
      break;
    case DATA_VIEW_FIELD_INT16_VECTOR:
      if (stringValue == NULL) {
        // check for length needed!!
        result= dataView->setInt16(dataView, fieldInfo->fieldOffset+fieldIdx*sizeof(int16_t), (int16_t)numericValue);
        checkErrOK(result);
      } else {
        return COMP_MSG_ERR_BAD_VALUE;
      }
      break;
    case DATA_VIEW_FIELD_UINT16_VECTOR:
      if (stringValue == NULL) {
        // check for length needed!!
        result= dataView->setUint16(dataView, fieldInfo->fieldOffset+fieldIdx*sizeof(uint16_t), (uint16_t)numericValue);
        checkErrOK(result);
      } else {
        return COMP_MSG_ERR_BAD_VALUE;
      }
      break;
#ifdef NOTDEF
    case DATA_VIEW_FIELD_INT32_VECTOR:
      if (stringValue != NULL) {
        // check for length needed!!
        os_memcpy((int8_t *)fieldInfo->value.int32Vector, stringValue, fieldInfo->fieldLgth);
      } else {
        return COMP_MSG_ERR_BAD_VALUE;
      }
      break;
    case DATA_VIEW_FIELD_UINT32_VECTOR:
      if (stringValue != NULL) {
        // check for length needed!!
        os_memcpy((uint8_t *)fieldInfo->value.uint32Vector, stringValue, fieldInfo->fieldLgth);
      } else {
        return COMP_MSG_ERR_BAD_VALUE;
      }
      break;
#endif
    default:
COMP_MSG_DBG(self, "v", 1, "bad type in setFieldValue: %d\n", fieldInfo->fieldTypeId);
      return COMP_MSG_ERR_BAD_FIELD_TYPE;
      break;
  }
  return DATA_VIEW_ERR_OK;
}

// ================================= newCompMsgDataView ====================================

compMsgDataView_t *newCompMsgDataView(uint8_t *data, size_t lgth) {
  compMsgDataView_t *compMsgDataView = os_zalloc(sizeof(compMsgDataView_t));
  if (compMsgDataView == NULL) {
    return NULL;
  }
//ets_printf("newCompMsgDataView: newDataView: %p!\n", compMsgDataView);
  compMsgDataView->dataView = newDataView(data, lgth);
  if (compMsgDataView->dataView == NULL) {
    return NULL;
  }
  compMsgDataViewId++;
  compMsgDataView->id = compMsgDataViewId;

  compMsgDataView->getRandomNum = &getRandomNum;
  compMsgDataView->setRandomNum = &setRandomNum;

  compMsgDataView->getSequenceNum = &getSequenceNum;
  compMsgDataView->setSequenceNum = &setSequenceNum;
  compMsgDataView->setSavedSequenceNum = &setSavedSequenceNum;

  compMsgDataView->getFiller = &getFiller;
  compMsgDataView->setFiller = &setFiller;
  compMsgDataView->setZeroFiller = &setZeroFiller;

  compMsgDataView->getCrc = &getCrc;
  compMsgDataView->setCrc = &setCrc;
  compMsgDataView->getTotalCrc = &getTotalCrc;
  compMsgDataView->setTotalCrc = &setTotalCrc;

  compMsgDataView->getFieldValue = &getFieldValue;
  compMsgDataView->setFieldValue = &setFieldValue;

  return compMsgDataView;
}
