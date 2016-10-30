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

#ifndef COMP_MSG_DATA_VIEW_H
#define	COMP_MSG_DATA_VIEW_H

#include "c_types.h"
#include "dataView.h"

#ifdef	__cplusplus
extern "C" {
#endif

enum compMsgErrorCode
{
  COMP_MSG_ERR_OK                    = 0,
  COMP_MSG_ERR_VALUE_NOT_SET         = 255,
  COMP_MSG_ERR_VALUE_OUT_OF_RANGE    = 254,
  COMP_MSG_ERR_BAD_VALUE             = 253,
  COMP_MSG_ERR_BAD_FIELD_TYPE        = 252,
  COMP_MSG_ERR_FIELD_TYPE_NOT_FOUND  = 251,
  COMP_MSG_ERR_VALUE_TOO_BIG         = 250,
  COMP_MSG_ERR_OUT_OF_MEMORY         = 249,
  COMP_MSG_ERR_OUT_OF_RANGE          = 248,
  // be carefull the values up to here
  // must correspond to the values in dataView.h !!!
  // with the names like DATA_VIEW_ERR_*

  COMP_MSG_ERR_FIELD_NOT_FOUND       = 230,
  COMP_MSG_ERR_BAD_SPECIAL_FIELD     = 229,
  COMP_MSG_ERR_BAD_HANDLE            = 228,
  COMP_MSG_ERR_HANDLE_NOT_FOUND      = 227,
  COMP_MSG_ERR_NOT_ENCODED           = 226,
  COMP_MSG_ERR_ENCODE_ERROR          = 225,
  COMP_MSG_ERR_DECODE_ERROR          = 224,
  COMP_MSG_ERR_BAD_CRC_VALUE         = 223,
  COMP_MSG_ERR_CRYPTO_INIT_FAILED    = 222,
  COMP_MSG_ERR_CRYPTO_OP_FAILED      = 221,
  COMP_MSG_ERR_CRYPTO_BAD_MECHANISM  = 220,
  COMP_MSG_ERR_NOT_ENCRYPTED         = 219,
  COMP_MSG_ERR_DEFINITION_NOT_FOUND  = 218,
  COMP_MSG_ERR_DEFINITION_TOO_MANY_FIELDS = 217,
  COMP_MSG_ERR_BAD_TABLE_ROW         = 216,
  COMP_MSG_ERR_TOO_MANY_FIELDS       = 215,
  COMP_MSG_ERR_BAD_DEFINTION_CMD_KEY = 214,
  COMP_MSG_ERR_NO_SLOT_FOUND         = 213,
  COMP_MSG_ERR_BAD_NUM_FIELDS        = 212,
  COMP_MSG_ERR_ALREADY_INITTED       = 211,
  COMP_MSG_ERR_NOT_YET_INITTED       = 210,
  COMP_MSG_ERR_FIELD_CANNOT_BE_SET   = 209,
  COMP_MSG_ERR_NO_SUCH_FIELD         = 208,
  COMP_MSG_ERR_DUPLICATE_FIELD       = 207,

  COMP_MSG_ERR_BAD_DATA_LGTH         = 206,
  COMP_MSG_ERR_NOT_YET_PREPARED      = 205,
  COMP_DEF_ERR_ALREADY_INITTED       = 204,
  COMP_DEF_ERR_NOT_YET_INITTED       = 203,
  COMP_DEF_ERR_NOT_YET_PREPARED      = 202,
  COMP_DEF_ERR_ALREADY_CREATED       = 201,
  COMP_MSG_ERR_FIELD_TOTAL_LGTH_MISSING = 200,
  COMP_LIST_ERR_ALREADY_INITTED      = 199,
  COMP_LIST_ERR_NOT_YET_INITTED      = 198,
  COMP_LIST_ERR_NOT_YET_PREPARED     = 197,
  COMP_LIST_ERR_ALREADY_CREATED      = 196,
};

enum compMsgSpecialFieldNames
{
  COMP_MSG_SPEC_FIELD_SRC                 = 255,
  COMP_MSG_SPEC_FIELD_DST                 = 254,
  COMP_MSG_SPEC_FIELD_TARGET_CMD          = 253,
  COMP_MSG_SPEC_FIELD_TOTAL_LGTH          = 252,
  COMP_MSG_SPEC_FIELD_CMD_KEY             = 251,
  COMP_MSG_SPEC_FIELD_CMD_LGTH            = 250,
  COMP_MSG_SPEC_FIELD_RANDOM_NUM          = 249,
  COMP_MSG_SPEC_FIELD_SEQUENCE_NUM        = 248,
  COMP_MSG_SPEC_FIELD_FILLER              = 247,
  COMP_MSG_SPEC_FIELD_CRC                 = 246,
  COMP_MSG_SPEC_FIELD_ID                  = 245,
  COMP_MSG_SPEC_FIELD_TABLE_ROWS          = 244,
  COMP_MSG_SPEC_FIELD_TABLE_ROW_FIELDS    = 243,
  COMP_MSG_SPEC_FIELD_NUM_FIELDS          = 242,
  COMP_MSG_SPEC_FIELD_GUID                = 241,
  COMP_MSG_SPEC_FIELD_NUM_NORM_FLDS       = 240,
  COMP_MSG_SPEC_FIELD_NORM_FLD_IDS        = 239,
  COMP_MSG_SPEC_FIELD_NORM_FLD_NAMES_SIZE = 238,
  COMP_MSG_SPEC_FIELD_NORM_FLD_NAMES      = 237,
  COMP_MSG_SPEC_FIELD_DEFINITIONS_SIZE    = 236,
  COMP_MSG_SPEC_FIELD_DEFINITIONS         = 235,
  COMP_MSG_SPEC_FIELD_NUM_LIST_MSGS       = 234,
  COMP_MSG_SPEC_FIELD_LIST_MSG_SIZES      = 233,
  COMP_MSG_SPEC_FIELD_LIST_MSGS           = 232,
  COMP_MSG_SPEC_FIELD_SRC_ID              = 231,
  COMP_MSG_SPEC_FIELD_HDR_FILLER          = 230,
  COMP_MSG_SPEC_FIELD_NUM_KEY_VALUES      = 229,
  COMP_MSG_SPEC_FIELD_LOW                 = 228,         // this must be the last entry!!
};

#define COMP_MSG_NO_INCR 0
#define COMP_MSG_INCR    1
#define COMP_MSG_DECR    -1

#define COMP_MSG_FREE_FIELD_ID 0xFF

typedef struct compMsgField {
  uint8_t fieldNameId;
  uint8_t fieldTypeId;
  uint8_t fieldLgth;
  uint8_t fieldFlags;
  uint16_t fieldKey;
  size_t fieldOffset;
} compMsgField_t;

typedef struct compMsgDataView compMsgDataView_t;

typedef uint8_t (* getFieldNameIdFromStr_t)(compMsgDataView_t *self, const uint8_t *fieldName, uint8_t *fieldNameId, uint8_t incrRefCnt);
typedef uint8_t (* getFieldNameStrFromId_t)(compMsgDataView_t *self, uint8_t fieldNameId, uint8_t **fieldName);

typedef uint8_t (* getRandomNum_t)(compMsgDataView_t *self, compMsgField_t *fieldInfo, uint32_t *value);
typedef uint8_t (* setRandomNum_t)(compMsgDataView_t *self, compMsgField_t *fieldInfo);

typedef uint8_t (* getSequenceNum_t)(compMsgDataView_t *self, compMsgField_t *fieldInfo, uint32_t *value);
typedef uint8_t (* setSequenceNum_t)(compMsgDataView_t *self, compMsgField_t *fieldInfo);

typedef uint8_t (* getFiller_t)(compMsgDataView_t *self, compMsgField_t *fieldInfo, uint8_t **value);
typedef uint8_t (* setFiller_t)(compMsgDataView_t *self, compMsgField_t *fieldInfo);

typedef uint8_t (* getCrc_t)(compMsgDataView_t *self, compMsgField_t *fieldInfo, size_t startOffset, size_t lgth);
typedef uint8_t (* setCrc_t)(compMsgDataView_t *self, compMsgField_t *fieldInfo, size_t startOffset, size_t lgth);

typedef uint8_t (* dvGetFieldValue_t)(compMsgDataView_t *self, compMsgField_t *fieldInfo, int *numericValue, uint8_t **stringValue, int fieldIdx);
typedef uint8_t (* dvSetFieldValue_t)(compMsgDataView_t *self, compMsgField_t *fieldInfo, int numericValue, const uint8_t *stringValue, int fieldIdx);

typedef struct compMsgDataView {
  dataView_t *dataView;
  uint8_t id;

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

  dvGetFieldValue_t getFieldValue;
  dvSetFieldValue_t setFieldValue;

} compMsgDataView_t;

compMsgDataView_t *newCompMsgDataView(void);
void freeCompMsgDataView(compMsgDataView_t *dataView);


#ifdef	__cplusplus
}
#endif

#endif	/* COMP_MSG_DATA_VIEW_H */
