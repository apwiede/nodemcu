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

#ifndef STRUCT_MSG_DATA_VIEW_H
#define	STRUCT_MSG_DATA_VIEW_H

#include "c_types.h"
#include "dataView.h"

#ifdef	__cplusplus
extern "C" {
#endif

enum structmsgErrorCode
{
  STRUCT_MSG_ERR_OK                    = 0,
  STRUCT_MSG_ERR_VALUE_NOT_SET         = 255,
  STRUCT_MSG_ERR_VALUE_OUT_OF_RANGE    = 254,
  STRUCT_MSG_ERR_BAD_VALUE             = 253,
  STRUCT_MSG_ERR_BAD_FIELD_TYPE        = 252,
  STRUCT_MSG_ERR_FIELD_NOT_FOUND       = 251,
  STRUCT_MSG_ERR_VALUE_TOO_BIG         = 250,
  STRUCT_MSG_ERR_BAD_SPECIAL_FIELD     = 249,
  STRUCT_MSG_ERR_BAD_HANDLE            = 248,
  STRUCT_MSG_ERR_OUT_OF_MEMORY         = 247,
  STRUCT_MSG_ERR_HANDLE_NOT_FOUND      = 246,
  STRUCT_MSG_ERR_NOT_ENCODED           = 245,
  STRUCT_MSG_ERR_ENCODE_ERROR          = 244,
  STRUCT_MSG_ERR_DECODE_ERROR          = 243,
  STRUCT_MSG_ERR_BAD_CRC_VALUE         = 242,
  STRUCT_MSG_ERR_CRYPTO_INIT_FAILED    = 241,
  STRUCT_MSG_ERR_CRYPTO_OP_FAILED      = 240,
  STRUCT_MSG_ERR_CRYPTO_BAD_MECHANISM  = 239,
  STRUCT_MSG_ERR_NOT_ENCRYPTED         = 238,
  STRUCT_MSG_ERR_DEFINITION_NOT_FOUND  = 237,
  STRUCT_MSG_ERR_DEFINITION_TOO_MANY_FIELDS = 236,
  STRUCT_MSG_ERR_BAD_TABLE_ROW         = 235,
  STRUCT_MSG_ERR_TOO_MANY_FIELDS       = 234,
  STRUCT_MSG_ERR_BAD_DEFINTION_CMD_KEY = 233,
  STRUCT_MSG_ERR_NO_SLOT_FOUND         = 232,
  STRUCT_MSG_ERR_BAD_NUM_FIELDS        = 231,
  STRUCT_MSG_ERR_ALREADY_INITTED       = 230,
  STRUCT_MSG_ERR_NOT_YET_INITTED       = 229,
  STRUCT_MSG_ERR_FIELD_CANNOT_BE_SET   = 228,
};

enum structmsgSpecialFieldNames
{
  STRUCT_MSG_SPEC_FIELD_SRC              = 255,
  STRUCT_MSG_SPEC_FIELD_DST              = 254,
  STRUCT_MSG_SPEC_FIELD_TARGET_CMD       = 253,
  STRUCT_MSG_SPEC_FIELD_TOTAL_LGTH       = 252,
  STRUCT_MSG_SPEC_FIELD_CMD_KEY          = 251,
  STRUCT_MSG_SPEC_FIELD_CMD_LGTH         = 250,
  STRUCT_MSG_SPEC_FIELD_RANDOM_NUM       = 249,
  STRUCT_MSG_SPEC_FIELD_SEQUENCE_NUM     = 248,
  STRUCT_MSG_SPEC_FIELD_FILLER           = 247,
  STRUCT_MSG_SPEC_FIELD_CRC              = 246,
  STRUCT_MSG_SPEC_FIELD_ID               = 245,
  STRUCT_MSG_SPEC_FIELD_TABLE_ROWS       = 244,
  STRUCT_MSG_SPEC_FIELD_TABLE_ROW_FIELDS = 243,
  STRUCT_MSG_SPEC_FIELD_NUM_FIELDS       = 242,
  STRUCT_MSG_SPEC_FIELD_GUID             = 241,
  STRUCT_MSG_SPEC_FIELD_LOW              = 240,         // this must be the last entry!!
};

#define STRUCT_MSG_NO_INCR 0
#define STRUCT_MSG_INCR    1
#define STRUCT_MSG_DECR    -1

#define STRUCT_MSG_FREE_FIELD_ID 0xFF

#define checkAllocOK(addr) if(addr == NULL) return STRUCT_MSG_ERR_OUT_OF_MEMORY
#define checkErrOK(result) if(result != STRUCT_MSG_ERR_OK) return result

typedef struct structmsgField {
  uint8_t fieldNameId;
  uint8_t fieldTypeId;
  uint8_t fieldLgth;
  uint8_t fieldFlags;
  size_t fieldOffset;
} structmsgField_t;

typedef struct structmsgDataView structmsgDataView_t;

typedef uint8_t (* getFieldNameIdFromStr_t)(structmsgDataView_t *self, const uint8_t *fieldName, uint8_t *fieldNameId, uint8_t incrRefCnt);
typedef uint8_t (* getFieldNameStrFromId_t)(structmsgDataView_t *self, uint8_t fieldNameId, uint8_t **fieldName);

typedef uint8_t (* getRandomNum_t)(structmsgDataView_t *self, int offset, uint32_t *value);
typedef uint8_t (* setRandomNum_t)(structmsgDataView_t *self, int offset, uint32_t value);

typedef uint8_t (* getSequenceNum_t)(structmsgDataView_t *self, int offset, uint32_t *value);
typedef uint8_t (* setSequenceNum_t)(structmsgDataView_t *self, int offset, uint32_t value);

typedef uint8_t (* getFiller_t)(structmsgDataView_t *self, int offset, uint32_t **value, size_t lgth);
typedef uint8_t (* setFiller_t)(structmsgDataView_t *self, int offset, uint32_t *value, size_t lgth);

typedef uint8_t (* getCrc_t)(structmsgDataView_t *self, int offset, uint32_t *value, int startOffset);
typedef uint8_t (* setCrc_t)(structmsgDataView_t *self, int offset, int startOffset);

typedef uint8_t (* dvGetFieldValue_t)(structmsgDataView_t *self, structmsgField_t *fieldInfo, int *numericValue, uint8_t **stringValue);
typedef uint8_t (* dvSetFieldValue_t)(structmsgDataView_t *self, structmsgField_t *fieldInfo, int numericValue, uint8_t *stringValue);

typedef uint8_t (* dvGetTableFieldValue_t)(structmsgDataView_t *self, int row, structmsgField_t *fieldInfo, void *value);
typedef uint8_t (* dvSetTableFieldValue_t)(structmsgDataView_t *self, int row, structmsgField_t *fieldInfo, void *value);

typedef struct structmsgDataView {
  dataView_t *dataView;

  getFieldNameIdFromStr_t getFieldNameIdFromStr;
  getFieldNameStrFromId_t getFieldNameStrFromId;

  getRandomNum_t getRandomNum;
  setRandomNum_t setRandomNum;

  getSequenceNum_t getSequenceNum;
  setSequenceNum_t setSequenceNum;

  getFiller_t getFiller;
  setFiller_t setFiller;

  getCrc_t getCrc;
  setCrc_t setCrc;

  dvGetFieldValue_t dvGetFieldValue;
  dvSetFieldValue_t dvSetFieldValue;

  dvGetTableFieldValue_t dvGetTableFieldValue;
  dvSetTableFieldValue_t dvSetTableFieldValue;
} structmsgDataView_t;

structmsgDataView_t *newStructmsgDataView(void);
void freeStructmsgDataView(structmsgDataView_t *dataView);


#ifdef	__cplusplus
}
#endif

#endif	/* STRUCT_MSG_DATA_VIEW_H */
