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
 * File:   dataView.h
 * Author: Arnulf P. Wiedemann
 *
 * Created on September 24, 2016
 */

#ifndef DATA_VIEW_H
#define	DATA_VIEW_H

#include "c_types.h"
#ifdef	__cplusplus
extern "C" {
#endif

#define COMP_MSG_DEBUG

enum DataViewErrorCode
{
  DATA_VIEW_ERR_OK                    = 0,
  DATA_VIEW_ERR_VALUE_NOT_SET         = 255,
  DATA_VIEW_ERR_VALUE_OUT_OF_RANGE    = 254,
  DATA_VIEW_ERR_BAD_VALUE             = 253,
  DATA_VIEW_ERR_BAD_FIELD_TYPE        = 252,
  DATA_VIEW_ERR_FIELD_TYPE_NOT_FOUND  = 251,
  DATA_VIEW_ERR_VALUE_TOO_BIG         = 250,
  DATA_VIEW_ERR_OUT_OF_MEMORY         = 249,
  DATA_VIEW_ERR_OUT_OF_RANGE          = 248,
};

#define checkAllocOK(addr) if(addr == NULL) return DATA_VIEW_ERR_OUT_OF_MEMORY
#define checkErrOK(result) if(result != DATA_VIEW_ERR_OK) return result

typedef struct dataView dataView_t;

typedef uint8_t (* getUint8_t)(dataView_t *self, int offset, uint8_t *value);
typedef uint8_t (* getInt8_t)(dataView_t *self, int offset, int8_t *value);
typedef uint8_t (* setUint8_t)(dataView_t *self, int offset, uint8_t value);
typedef uint8_t (* setInt8_t)(dataView_t *self, int offset, int8_t value);

typedef uint8_t (* getUint16_t)(dataView_t *self, int offset, uint16_t *value);
typedef uint8_t (* getInt16_t)(dataView_t *self, int offset, int16_t *value);
typedef uint8_t (* setUint16_t)(dataView_t *self, int offset, uint16_t value);
typedef uint8_t (* setInt16_t)(dataView_t *self, int offset, int16_t value);

typedef uint8_t (* getUint32_t)(dataView_t *self, int offset, uint32_t *value);
typedef uint8_t (* getInt32_t)(dataView_t *self, int offset, int32_t *value);
typedef uint8_t (* setUint32_t)(dataView_t *self, int offset, uint32_t value);
typedef uint8_t (* setInt32_t)(dataView_t *self, int offset, int32_t value);

typedef uint8_t (* getUint8Vector_t)(dataView_t *self, int offset, uint8_t **value, size_t lgth);
typedef uint8_t (* getInt8Vector_t)(dataView_t *self, int offset, int8_t **value, size_t lgth);
typedef uint8_t (* setUint8Vector_t)(dataView_t *self, int offset, uint8_t *value, size_t lgth);
typedef uint8_t (* setInt8Vector_t)(dataView_t *self, int offset, int8_t *value, size_t lgth);

typedef uint8_t (* getUint16Vector_t)(dataView_t *self, int offset, uint16_t **value, size_t lgth);
typedef uint8_t (* getInt16Vector_t)(dataView_t *self, int offset, int16_t **value, size_t lgth);
typedef uint8_t (* setUint16Vector_t)(dataView_t *self, int offset, uint16_t *value, size_t lgth);
typedef uint8_t (* setInt16Vector_t)(dataView_t *self, int offset, int16_t *value, size_t lgth);

typedef uint8_t (* getUint32Vector_t)(dataView_t *self, int offset, uint32_t **value, size_t lgth);
typedef uint8_t (* getInt32Vector_t)(dataView_t *self, int offset, int32_t **value, size_t lgth);
typedef uint8_t (* setUint32Vector_t)(dataView_t *self, int offset, uint32_t *value, size_t lgth);
typedef uint8_t (* setInt32Vector_t)(dataView_t *self, int offset, int32_t *value, size_t lgth);

typedef uint8_t (* getDataViewData_t)(dataView_t *self, uint8_t **data, size_t *lgth);
typedef void (* dumpBinary_t)(const uint8_t *data, size_t lgth, const uint8_t *where);
typedef void (* dumpBinaryWide_t)(const uint8_t *data, size_t lgth, const uint8_t *where);

typedef struct dataView {
  uint8_t *data;
  uint8_t *where;
  size_t lgth;
  uint8_t id;
  
  getUint8_t getUint8;
  getInt8_t getInt8;
  setUint8_t setUint8;
  setInt8_t setInt8;

  getUint16_t getUint16;
  getInt16_t getInt16;
  setUint16_t setUint16;
  setInt16_t setInt16;

  getUint32_t getUint32;
  getInt32_t getInt32;
  setUint32_t setUint32;
  setInt32_t setInt32;

  getUint8Vector_t getUint8Vector;
  getInt8Vector_t getInt8Vector;
  setUint8Vector_t setUint8Vector;
  setInt8Vector_t setInt8Vector;

  getUint16Vector_t getUint16Vector;
  getInt16Vector_t getInt16Vector;
  setUint16Vector_t setUint16Vector;
  setInt16Vector_t setInt16Vector;

  getUint32Vector_t getUint32Vector;
  getInt32Vector_t getInt32Vector;
  setUint32Vector_t setUint32Vector;
  setInt32Vector_t setInt32Vector;

  getDataViewData_t getDataViewData;
  dumpBinary_t dumpBinary;
  dumpBinaryWide_t dumpBinaryWide;
} dataView_t;

dataView_t *newDataView(uint8_t *data, size_t lgth);

#ifdef	__cplusplus
}
#endif

#endif	/* DATA_VIEW_H */
