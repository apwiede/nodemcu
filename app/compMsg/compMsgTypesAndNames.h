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

#include "compMsg.h"

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
  COMP_MSG_SPEC_FIELD_NONE                      = 0,
  COMP_MSG_SPEC_FIELD_SRC                       = 1,
  COMP_MSG_SPEC_FIELD_GRP                       = 2,
  COMP_MSG_SPEC_FIELD_DST                       = 3,
  COMP_MSG_SPEC_FIELD_TOTAL_LGTH                = 4,
  COMP_MSG_SPEC_FIELD_IP_ADDR                   = 5,
  COMP_MSG_SPEC_FIELD_CMD_KEY                   = 6,
  COMP_MSG_SPEC_FIELD_CMD_LGTH                  = 7,
  COMP_MSG_SPEC_FIELD_RANDOM_NUM                = 8,
  COMP_MSG_SPEC_FIELD_SEQUENCE_NUM              = 9,
  COMP_MSG_SPEC_FIELD_FILLER                    = 10,
  COMP_MSG_SPEC_FIELD_CRC                       = 11,
  COMP_MSG_SPEC_FIELD_TOTAL_CRC                 = 12,
  COMP_MSG_SPEC_FIELD_GUID                      = 13,
  COMP_MSG_SPEC_FIELD_SRC_ID                    = 14,
  COMP_MSG_SPEC_FIELD_HDR_FILLER                = 15,
  COMP_MSG_SPEC_FIELD_NUM_KEY_VALUES            = 16,

  COMP_MSG_SPEC_FIELD_PROVISIONING_SSID         = 17,
  COMP_MSG_SPEC_FIELD_PROVISIONING_PORT         = 18,
  COMP_MSG_SPEC_FIELD_PROVISIONING_IP_ADDR      = 19,
  COMP_MSG_SPEC_FIELD_CLIENT_SSID               = 20,
  COMP_MSG_SPEC_FIELD_CLIENT_PASSWD             = 21,
  COMP_MSG_SPEC_FIELD_CLIENT_IP_ADDR            = 22,
  COMP_MSG_SPEC_FIELD_CLIENT_PORT               = 23,
  COMP_MSG_SPEC_FIELD_CLIENT_STATUS             = 24,
  COMP_MSG_SPEC_FIELD_SSDP_IP_ADDR              = 25,
  COMP_MSG_SPEC_FIELD_SSDP_PORT                 = 26,
  COMP_MSG_SPEC_FIELD_SSDP_STATUS               = 27,
  COMP_MSG_SPEC_FIELD_CLOUD_SECURE_CONNECT      = 28,
  COMP_MSG_SPEC_FIELD_CLOUD_PORT                = 29,
  COMP_MSG_SPEC_FIELD_CLOUD_HOST_1              = 30,
  COMP_MSG_SPEC_FIELD_CLOUD_HOST_2              = 31,
  COMP_MSG_SPEC_FIELD_CLOUD_URL_1_PART_1        = 32,
  COMP_MSG_SPEC_FIELD_CLOUD_URL_1_PART_2        = 33,
  COMP_MSG_SPEC_FIELD_CLOUD_URL_TENANT_ID_1     = 34,
  COMP_MSG_SPEC_FIELD_CLOUD_URL_2_PART_1        = 35,
  COMP_MSG_SPEC_FIELD_CLOUD_URL_2_PART_2        = 36,
  COMP_MSG_SPEC_FIELD_CLOUD_URL_TENANT_ID_2     = 37,
  COMP_MSG_SPEC_FIELD_CLOUD_NODE_TOKEN_1        = 38,
  COMP_MSG_SPEC_FIELD_CLOUD_NODE_TOKEN_2        = 39,
  COMP_MSG_SPEC_FIELD_OTA_PORT                  = 40,
  COMP_MSG_SPEC_FIELD_OTA_ROM_PATH              = 41,
  COMP_MSG_SPEC_FIELD_OTA_FS_PATH               = 42,
  COMP_MSG_SPEC_FIELD_OTA_HOST                  = 43,
  COMP_MSG_SPEC_FIELD_PROD_TEST_SSID            = 44,
  COMP_MSG_SPEC_FIELD_PROD_TEST_PASSWD          = 45,
  COMP_MSG_SPEC_FIELD_PROD_TEST_SECURITY_TYPE   = 46,
  COMP_MSG_SPEC_FIELD_PROD_TEST_TARGET_PROTOCOL = 47,
  COMP_MSG_SPEC_FIELD_PROD_TEST_IP_ADDR         = 48,
  COMP_MSG_SPEC_FIELD_PROD_TEST_SUBNET          = 49,
  COMP_MSG_SPEC_FIELD_PROD_TEST_GATEWAY         = 50,
  COMP_MSG_SPEC_FIELD_PROD_TEST_DNS             = 51,
  COMP_MSG_SPEC_FIELD_PROD_TEST_PING_ADDRESS    = 52,
  COMP_MSG_SPEC_FIELD_PROD_TEST_STATUS          = 53,
  COMP_MSG_SPEC_FIELD_HDR_RESERVE               = 54,

  COMP_MSG_SPEC_FIELD_LOW                       = 55,         // this must be the last entry!!
};

enum compMsgFieldGroups
{
  COMP_MSG_USE_FIELD_GROUP                = 1,
  COMP_MSG_HEADS_FIELD_GROUP              = 2,
  COMP_MSG_DESC_HEADER_FIELD_GROUP        = 3,
  COMP_MSG_DESC_MID_PART_FIELD_GROUP      = 4,
  COMP_MSG_DESC_TRAILER_FIELD_GROUP       = 5,
  COMP_MSG_DESC_KEY_VALUE_FIELD_GROUP     = 6,
  COMP_MSG_FIELDS_TO_SAVE_FIELD_GROUP     = 7,
  COMP_MSG_ACTIONS_FIELD_GROUP            = 8,
  COMP_MSG_VAL_HEADER_FIELD_GROUP         = 9,
  COMP_MSG_WIFI_DATA_VALUES_FIELD_GROUP   = 10,
  COMP_MSG_MODULE_DATA_VALUES_FIELD_GROUP = 11,
  COMP_MSG_DESC_FIELD_GROUP               = 12,
  COMP_MSG_VAL_FIELD_GROUP                = 13,
};

#define COMP_MSG_NO_INCR 0
#define COMP_MSG_INCR    1
#define COMP_MSG_DECR    -1

#define COMP_MSG_FREE_FIELD_ID 0xFF

#define COMP_MSG_FIELD_WIFI_DATA               0x01
#define COMP_MSG_FIELD_MODULE_DATA             0x02
#define COMP_MSG_FIELD_TO_SAVE                 0x04
#define COMP_MSG_FIELD_KEY_VALUE               0x08
#define COMP_MSG_FIELD_HEADER                  0x10
#define COMP_MSG_FIELD_HEADER_VALUE            0x20
#define COMP_MSG_FIELD_HEADER_UNIQUE           0x40
#define COMP_MSG_FIELD_HEADER_CHKSUM_NON_ZERO  0x80

typedef struct str2id {
  uint8_t *str;
  uint8_t id;
} str2id_t ;

typedef struct fieldName2id {
  uint8_t *fieldName;
  uint8_t fieldNameId;
  uint8_t refCnt;
} fieldName2id_t;

typedef struct fieldNames {
  size_t numNames;
  size_t maxNames; 
  fieldName2id_t *names;
} fieldNames_t;

typedef struct keyValueDesc keyValueDesc_t;

typedef struct fieldDescInfo {
  uint32_t fieldFlags;
  uint8_t fieldTypeId;
  uint16_t fieldLgth;
  uint16_t fieldOffset;
  keyValueDesc_t *keyValueDesc;
} fieldDescInfo_t;

typedef struct dataValue dataValue_t;

typedef struct fieldValInfo {
  uint32_t fieldFlags;
  dataValue_t *dataValue;
  fieldValueCallback_t fieldValueCallback;
} fieldValInfo_t;

typedef struct msgFieldInfos {
  uint16_t numMsgDescFields;
  fieldDescInfo_t **fieldDescInfos;
  uint16_t numMsgValFields;
  fieldValInfo_t **fieldValInfos;
} msgFieldInfos_t;

typedef struct compMsgDispatcher compMsgDispatcher_t;

typedef uint8_t (* getFieldTypeIdFromStr_t)(compMsgDispatcher_t *self, const uint8_t *fieldTypeStr, uint8_t *fieldTypeId);
typedef uint8_t (* getFieldTypeStrFromId_t)(compMsgDispatcher_t *self, uint8_t fieldTypeId, uint8_t **fieldTypeStr);

typedef uint8_t (* getFieldNameIdFromStr_t)(compMsgDispatcher_t *self, const uint8_t *fieldName, uint8_t *fieldNameId, uint8_t incrRefCnt);
typedef uint8_t (* getFieldNameStrFromId_t)(compMsgDispatcher_t *self, uint8_t fieldNameId, uint8_t **fieldName);
typedef uint8_t (* getFileNameTokenIdFromStr_t)(compMsgDispatcher_t *self, const uint8_t *fileNameTokenStr, uint8_t *fileNameTokenId);

typedef uint8_t (* dumpMsgFieldDescInfos_t)(compMsgDispatcher_t *self);
typedef uint8_t (* dumpMsgFieldValInfos_t)(compMsgDispatcher_t *self);
typedef uint8_t (* addMsgFieldDescInfos_t)(compMsgDispatcher_t *self, uint8_t numEntries);
typedef uint8_t (* setMsgFieldDescInfo_t)(compMsgDispatcher_t *self, uint8_t idx, fieldDescInfo_t *fieldDescInfo);
typedef uint8_t (* getMsgFieldDescInfo_t)(compMsgDispatcher_t *self, uint8_t idx, fieldDescInfo_t *fieldDescInfo);
typedef uint8_t (* addMsgFieldValInfos_t)(compMsgDispatcher_t *self, uint8_t numEntries);
typedef uint8_t (* setMsgFieldValInfo_t)(compMsgDispatcher_t *self, uint8_t idx, fieldValInfo_t *fieldValInfo);
typedef uint8_t (* getMsgFieldValInfo_t)(compMsgDispatcher_t *self, uint8_t idx, fieldValInfo_t *fieldValInfo);

typedef uint8_t (* freeCompMsgTypesAndNames_t)(compMsgDispatcher_t *self);

typedef struct compMsgTypesAndNames {
  uint8_t id;
  fieldNames_t fieldNames;
  uint8_t numSpecFieldIds;
  uint8_t numHeaderFields;
  msgFieldInfos_t msgFieldInfos;
  
  getFieldTypeIdFromStr_t getFieldTypeIdFromStr;
  getFieldTypeStrFromId_t getFieldTypeStrFromId;

  getFieldNameIdFromStr_t getFieldNameIdFromStr;
  getFieldNameStrFromId_t getFieldNameStrFromId;

  getFileNameTokenIdFromStr_t getFileNameTokenIdFromStr;

  dumpMsgFieldDescInfos_t dumpMsgFieldDescInfos;
  dumpMsgFieldValInfos_t dumpMsgFieldValInfos;
  addMsgFieldDescInfos_t addMsgFieldDescInfos;
  getMsgFieldDescInfo_t getMsgFieldDescInfo;
  setMsgFieldDescInfo_t setMsgFieldDescInfo;
  addMsgFieldValInfos_t addMsgFieldValInfos;
  getMsgFieldValInfo_t getMsgFieldValInfo;
  setMsgFieldValInfo_t setMsgFieldValInfo;

  freeCompMsgTypesAndNames_t freeCompMsgTypesAndNames;
} compMsgTypesAndNames_t;

uint8_t compMsgTypesAndNamesInit(compMsgDispatcher_t *self);
compMsgTypesAndNames_t *newCompMsgTypesAndNames();

#endif	/* COMP_MSG_TYPES_AND_NAMES_H */
