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

enum compMsgSpecialFieldNames
{
  COMP_MSG_SPEC_FIELD_SRC                       = 255,
  COMP_MSG_SPEC_FIELD_GRP                       = 254,
  COMP_MSG_SPEC_FIELD_DST                       = 253,
  COMP_MSG_SPEC_FIELD_TOTAL_LGTH                = 252,
  COMP_MSG_SPEC_FIELD_CMD_KEY                   = 251,
  COMP_MSG_SPEC_FIELD_CMD_LGTH                  = 250,
  COMP_MSG_SPEC_FIELD_RANDOM_NUM                = 249,
  COMP_MSG_SPEC_FIELD_SEQUENCE_NUM              = 248,
  COMP_MSG_SPEC_FIELD_FILLER                    = 247,
  COMP_MSG_SPEC_FIELD_CRC                       = 246,
  COMP_MSG_SPEC_FIELD_ID                        = 245,
  COMP_MSG_SPEC_FIELD_IP_ADDR                   = 244,
  COMP_MSG_SPEC_FIELD_GUID                      = 241,
  COMP_MSG_SPEC_FIELD_SRC_ID                    = 231,
  COMP_MSG_SPEC_FIELD_HDR_FILLER                = 230,
  COMP_MSG_SPEC_FIELD_NUM_KEY_VALUES            = 229,
  COMP_MSG_SPEC_FIELD_PROVISIONING_SSID         = 228,
  COMP_MSG_SPEC_FIELD_PROVISIONING_PORT         = 227,
  COMP_MSG_SPEC_FIELD_PROVISIONING_IP_ADDR      = 226,
  COMP_MSG_SPEC_FIELD_CLIENT_SSID               = 225,
  COMP_MSG_SPEC_FIELD_CLIENT_PASSWD             = 224,
  COMP_MSG_SPEC_FIELD_CLIENT_IP_ADDR            = 223,
  COMP_MSG_SPEC_FIELD_CLIENT_PORT               = 222,
  COMP_MSG_SPEC_FIELD_CLIENT_STATUS             = 221,
  COMP_MSG_SPEC_FIELD_SSDP_IP_ADDR              = 220,
  COMP_MSG_SPEC_FIELD_SSDP_PORT                 = 219,
  COMP_MSG_SPEC_FIELD_SSDP_STATUS               = 218,
  COMP_MSG_SPEC_FIELD_CLOUD_SECURE_CONNECT      = 217,
  COMP_MSG_SPEC_FIELD_CLOUD_PORT                = 216,
  COMP_MSG_SPEC_FIELD_CLOUD_HOST_1              = 215,
  COMP_MSG_SPEC_FIELD_CLOUD_HOST_2              = 214,
  COMP_MSG_SPEC_FIELD_CLOUD_URL_1_PART_1        = 213,
  COMP_MSG_SPEC_FIELD_CLOUD_URL_1_PART_2        = 212,
  COMP_MSG_SPEC_FIELD_CLOUD_URL_TENANT_ID_1     = 211,
  COMP_MSG_SPEC_FIELD_CLOUD_URL_2_PART_1        = 210,
  COMP_MSG_SPEC_FIELD_CLOUD_URL_2_PART_2        = 209,
  COMP_MSG_SPEC_FIELD_CLOUD_URL_TENANT_ID_2     = 208,
  COMP_MSG_SPEC_FIELD_CLOUD_NODE_TOKEN_1        = 207,
  COMP_MSG_SPEC_FIELD_CLOUD_NODE_TOKEN_2        = 206,
  COMP_MSG_SPEC_FIELD_TOTAL_CRC                 = 205,
  COMP_MSG_SPEC_FIELD_OTA_PORT                  = 204,
  COMP_MSG_SPEC_FIELD_OTA_ROM_PATH              = 203,
  COMP_MSG_SPEC_FIELD_OTA_FS_PATH               = 202,
  COMP_MSG_SPEC_FIELD_OTA_HOST                  = 201,
  COMP_MSG_SPEC_FIELD_PROD_TEST_SSID            = 200,
  COMP_MSG_SPEC_FIELD_PROD_TEST_PASSWD          = 199,
  COMP_MSG_SPEC_FIELD_PROD_TEST_SECURITY_TYPE   = 198,
  COMP_MSG_SPEC_FIELD_PROD_TEST_TARGET_PROTOCOL = 197,
  COMP_MSG_SPEC_FIELD_PROD_TEST_IP_ADDR         = 196,
  COMP_MSG_SPEC_FIELD_PROD_TEST_SUBNET          = 195,
  COMP_MSG_SPEC_FIELD_PROD_TEST_GATEWAY         = 194,
  COMP_MSG_SPEC_FIELD_PROD_TEST_DNS             = 193,
  COMP_MSG_SPEC_FIELD_PROD_TEST_PING_ADDRESS    = 192,
  COMP_MSG_SPEC_FIELD_PROD_TEST_STATUS          = 191,
  COMP_MSG_SPEC_FIELD_HDR_RESERVE               = 190,

  COMP_MSG_SPEC_FIELD_LOW                       = 189,         // this must be the last entry!!
};

enum compMsgSpecialFileNameTokens
{
  COMP_MSG_USE_FILE_TOKEN                = 1,
  COMP_MSG_HEADS_FILE_TOKEN              = 2,
  COMP_MSG_DESC_HEADER_FILE_TOKEN        = 3,
  COMP_MSG_DESC_MID_PART_FILE_TOKEN      = 4,
  COMP_MSG_DESC_TRAILER_FILE_TOKEN       = 5,
  COMP_MSG_FIELDS_TO_SAVE_FILE_TOKEN     = 6,
  COMP_MSG_ACTIONS_FILE_TOKEN            = 7,
  COMP_MSG_VAL_HEADER_FILE_TOKEN         = 8,
  COMP_MSG_WIFI_DATA_VALUES_FILE_TOKEN   = 9,
  COMP_MSG_MODULE_DATA_VALUES_FILE_TOKEN = 10,
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
typedef uint8_t (* getFileNameTokenIdFromStr_t)(compMsgTypesAndNames_t *self, const uint8_t *fileNameTokenStr, uint8_t *fileNameTokenId);

typedef uint8_t (* freeCompMsgTypesAndNames_t)(compMsgTypesAndNames_t *compMsgTypesAndNames);

typedef struct compMsgTypesAndNames {
  uint8_t id;
  fieldNames_t fieldNames;
  
  getFieldTypeIdFromStr_t getFieldTypeIdFromStr;
  getFieldTypeStrFromId_t getFieldTypeStrFromId;

  getFieldNameIdFromStr_t getFieldNameIdFromStr;
  getFieldNameStrFromId_t getFieldNameStrFromId;

  getFileNameTokenIdFromStr_t getFileNameTokenIdFromStr;

  freeCompMsgTypesAndNames_t freeCompMsgTypesAndNames;
} compMsgTypesAndNames_t;

compMsgTypesAndNames_t *newCompMsgTypesAndNames();

#ifdef	__cplusplus
}
#endif

#endif	/* COMP_MSG_TYPES_AND_NAMES_H */
