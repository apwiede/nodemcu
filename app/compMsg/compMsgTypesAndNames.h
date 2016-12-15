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
 * File:   compMsgTypesAndNames.h
 * Author: Arnulf P. Wiedemann
 *
 * Created on November 24, 2016
 */

#ifndef COMP_MSG_TYPES_AND_NAMES_H
#define	COMP_MSG_TYPES_AND_NAMES_H

#include "c_types.h"
#ifdef	__cplusplus
extern "C" {
#endif

enum CompMsgFieldType
{
  DATA_VIEW_FIELD_NONE          = 0,
  DATA_VIEW_FIELD_UINT8_T       = 1,
  DATA_VIEW_FIELD_INT8_T        = 2,
  DATA_VIEW_FIELD_UINT16_T      = 3,
  DATA_VIEW_FIELD_INT16_T       = 4,
  DATA_VIEW_FIELD_UINT32_T      = 5,
  DATA_VIEW_FIELD_INT32_T       = 6,
  DATA_VIEW_FIELD_UINT8_VECTOR  = 7,
  DATA_VIEW_FIELD_INT8_VECTOR   = 8,
  DATA_VIEW_FIELD_UINT16_VECTOR = 9,
  DATA_VIEW_FIELD_INT16_VECTOR  = 10,
  DATA_VIEW_FIELD_UINT32_VECTOR = 11,
  DATA_VIEW_FIELD_INT32_VECTOR  = 12,
};

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
  COMP_MSG_SPEC_FIELD_SRC                  = 255,
  COMP_MSG_SPEC_FIELD_DST                  = 254,
  COMP_MSG_SPEC_FIELD_TARGET_CMD           = 253,
  COMP_MSG_SPEC_FIELD_TOTAL_LGTH           = 252,
  COMP_MSG_SPEC_FIELD_CMD_KEY              = 251,
  COMP_MSG_SPEC_FIELD_CMD_LGTH             = 250,
  COMP_MSG_SPEC_FIELD_RANDOM_NUM           = 249,
  COMP_MSG_SPEC_FIELD_SEQUENCE_NUM         = 248,
  COMP_MSG_SPEC_FIELD_FILLER               = 247,
  COMP_MSG_SPEC_FIELD_CRC                  = 246,
  COMP_MSG_SPEC_FIELD_ID                   = 245,
  COMP_MSG_SPEC_FIELD_TABLE_ROWS           = 244,
  COMP_MSG_SPEC_FIELD_TABLE_ROW_FIELDS     = 243,
  COMP_MSG_SPEC_FIELD_NUM_FIELDS           = 242,
  COMP_MSG_SPEC_FIELD_GUID                 = 241,
  COMP_MSG_SPEC_FIELD_NUM_NORM_FLDS        = 240,
  COMP_MSG_SPEC_FIELD_NORM_FLD_IDS         = 239,
  COMP_MSG_SPEC_FIELD_NORM_FLD_NAMES_SIZE  = 238,
  COMP_MSG_SPEC_FIELD_NORM_FLD_NAMES       = 237,
  COMP_MSG_SPEC_FIELD_DEFINITIONS_SIZE     = 236,
  COMP_MSG_SPEC_FIELD_DEFINITIONS          = 235,
  COMP_MSG_SPEC_FIELD_NUM_LIST_MSGS        = 234,
  COMP_MSG_SPEC_FIELD_LIST_MSG_SIZES       = 233,
  COMP_MSG_SPEC_FIELD_LIST_MSGS            = 232,
  COMP_MSG_SPEC_FIELD_SRC_ID               = 231,
  COMP_MSG_SPEC_FIELD_HDR_FILLER           = 230,
  COMP_MSG_SPEC_FIELD_NUM_KEY_VALUES       = 229,
  COMP_MSG_SPEC_FIELD_PROVISIONING_SSID    = 228,
  COMP_MSG_SPEC_FIELD_PROVISIONING_PORT    = 227,
  COMP_MSG_SPEC_FIELD_PROVISIONING_IP_ADDR = 226,
  COMP_MSG_SPEC_FIELD_CLIENT_SSID          = 225,
  COMP_MSG_SPEC_FIELD_CLIENT_PASSWD        = 224,
  COMP_MSG_SPEC_FIELD_CLIENT_IP_ADDR       = 223,
  COMP_MSG_SPEC_FIELD_CLIENT_PORT          = 222,
  COMP_MSG_SPEC_FIELD_CLOUD_SECURE_CONNECT = 221,
  COMP_MSG_SPEC_FIELD_CLIENT_STATUS        = 220,
  COMP_MSG_SPEC_FIELD_CLOUD_PORT           = 219,
  COMP_MSG_SPEC_FIELD_CLOUD_DOMAIN_1       = 218,
  COMP_MSG_SPEC_FIELD_CLOUD_DOMAIN_2       = 217,
  COMP_MSG_SPEC_FIELD_CLOUD_HOST_1         = 216,
  COMP_MSG_SPEC_FIELD_CLOUD_HOST_2         = 215,
  COMP_MSG_SPEC_FIELD_CLOUD_SUB_URL_1      = 214,
  COMP_MSG_SPEC_FIELD_CLOUD_SUB_URL_2      = 213,
  COMP_MSG_SPEC_FIELD_CLOUD_NODE_TOKEN_1   = 212,
  COMP_MSG_SPEC_FIELD_CLOUD_NODE_TOKEN_2   = 211,
  COMP_MSG_SPEC_FIELD_TOTAL_CRC            = 210,

  COMP_MSG_SPEC_FIELD_LOW                  = 209,         // this must be the last entry!!
};

enum compMsgHttpHeaderNames
{
  COMP_MSG_HTTP_CODE                        = 1,
  COMP_MSG_HTTP_CONTENT_TYPE                = 2,
  COMP_MSG_HTTP_CONTENT_LENGTH              = 3,
  COMP_MSG_HTTP_CONNECTION                  = 4,
  COMP_MSG_HTTP_SERVER                      = 5,
  COMP_MSG_HTTP_DATE                        = 6,
  COMP_MSG_HTTP_CACHE_CONTROL               = 7,
  COMP_MSG_HTTP_NODE_CODE                   = 8,
  COMP_MSG_HTTP_SET_COOKIE                  = 9,
  COMP_MSG_HTTP_X_POWER_BY                  = 10,
};

#define COMP_MSG_NO_INCR 0
#define COMP_MSG_INCR    1
#define COMP_MSG_DECR    -1

#define COMP_MSG_FREE_FIELD_ID 0xFF

typedef struct str2id {
  uint8_t *str;
  uint8_t id;
} str2id_t ;

typedef struct fieldName2id {
  uint8_t *fieldName;
  uint8_t fieldNameId;
  uint8_t refCnt;
} fieldName2id_t;

typedef struct fieldNames
{
  size_t numNames;
  size_t maxNames; 
  fieldName2id_t *names;
} fieldNames_t;

typedef struct compMsgTypesAndNames compMsgTypesAndNames_t;

typedef uint8_t (* getFieldTypeIdFromStr_t)(compMsgTypesAndNames_t *self, const uint8_t *fieldTypeStr, uint8_t *fieldTypeId);
typedef uint8_t (* getFieldTypeStrFromId_t)(compMsgTypesAndNames_t *self, uint8_t fieldTypeId, uint8_t **fieldTypeStr);

typedef uint8_t (* getFieldNameIdFromStr_t)(compMsgTypesAndNames_t *self, const uint8_t *fieldName, uint8_t *fieldNameId, uint8_t incrRefCnt);
typedef uint8_t (* getFieldNameStrFromId_t)(compMsgTypesAndNames_t *self, uint8_t fieldNameId, uint8_t **fieldName);
typedef uint8_t (* getHttpHeaderIdFromStr_t)(compMsgTypesAndNames_t *self, const uint8_t *httpHeaderStr, uint8_t *httpHeaderId);
typedef uint8_t (* freeCompMsgTypesAndNames_t)(compMsgTypesAndNames_t *compMsgTypesAndNames);

typedef struct compMsgTypesAndNames {
  uint8_t id;
  fieldNames_t fieldNames;
  
  getFieldTypeIdFromStr_t getFieldTypeIdFromStr;
  getFieldTypeStrFromId_t getFieldTypeStrFromId;

  getFieldNameIdFromStr_t getFieldNameIdFromStr;
  getFieldNameStrFromId_t getFieldNameStrFromId;

  getHttpHeaderIdFromStr_t getHttpHeaderIdFromStr;

  freeCompMsgTypesAndNames_t freeCompMsgTypesAndNames;
} compMsgTypesAndNames_t;

compMsgTypesAndNames_t *newCompMsgTypesAndNames();

#ifdef	__cplusplus
}
#endif

#endif	/* COMP_MSG_TYPES_AND_NAMES_H */
