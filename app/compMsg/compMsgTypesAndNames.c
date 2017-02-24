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
 * Created on November 24, 2016
 */

#include "osapi.h"
#include "c_types.h"
#include "mem.h"
#include "c_string.h"

#include "dataView.h"
#include "compMsgErrorCodes.h"
#include "compMsgTypesAndNames.h"

static uint8_t compMsgTypesAndNamesId;

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

static str2id_t specialFieldNames[] = {
  {"@src",                COMP_MSG_SPEC_FIELD_SRC},
  {"@dst",                COMP_MSG_SPEC_FIELD_DST},
  {"@targetCmd",          COMP_MSG_SPEC_FIELD_TARGET_CMD},
  {"@totalLgth",          COMP_MSG_SPEC_FIELD_TOTAL_LGTH},
  {"@cmdKey",             COMP_MSG_SPEC_FIELD_CMD_KEY},
  {"@cmdLgth",            COMP_MSG_SPEC_FIELD_CMD_LGTH},
  {"@randomNum",          COMP_MSG_SPEC_FIELD_RANDOM_NUM},
  {"@sequenceNum",        COMP_MSG_SPEC_FIELD_SEQUENCE_NUM},
  {"@filler",             COMP_MSG_SPEC_FIELD_FILLER},
  {"@crc",                COMP_MSG_SPEC_FIELD_CRC},
  {"@id",                 COMP_MSG_SPEC_FIELD_ID},
  {"@tablerows",          COMP_MSG_SPEC_FIELD_TABLE_ROWS},
  {"@tablerowfields",     COMP_MSG_SPEC_FIELD_TABLE_ROW_FIELDS},
  {"@GUID",               COMP_MSG_SPEC_FIELD_GUID},
  {"@numNormFlds",        COMP_MSG_SPEC_FIELD_NUM_NORM_FLDS},
  {"@normFldIds",         COMP_MSG_SPEC_FIELD_NORM_FLD_IDS},
  {"@normFldNamesSize",   COMP_MSG_SPEC_FIELD_NORM_FLD_NAMES_SIZE},
  {"@normFldNames",       COMP_MSG_SPEC_FIELD_NORM_FLD_NAMES},
  {"@definitionsSize",    COMP_MSG_SPEC_FIELD_DEFINITIONS_SIZE},
  {"@definitions",        COMP_MSG_SPEC_FIELD_DEFINITIONS},
  {"@numListMsgs",        COMP_MSG_SPEC_FIELD_NUM_LIST_MSGS},
  {"@listMsgSizes",       COMP_MSG_SPEC_FIELD_LIST_MSG_SIZES},
  {"@listMsgs",           COMP_MSG_SPEC_FIELD_LIST_MSGS},
  {"@srcId",              COMP_MSG_SPEC_FIELD_SRC_ID},
  {"@hdrFiller",          COMP_MSG_SPEC_FIELD_HDR_FILLER},
  {"@numKeyValues",       COMP_MSG_SPEC_FIELD_NUM_KEY_VALUES},
  {"@provisioningSsid",   COMP_MSG_SPEC_FIELD_PROVISIONING_SSID},
  {"@provisioningPort",   COMP_MSG_SPEC_FIELD_PROVISIONING_PORT},
  {"@provisioningIPAddr", COMP_MSG_SPEC_FIELD_PROVISIONING_IP_ADDR},
  {"@clientSsid",         COMP_MSG_SPEC_FIELD_CLIENT_SSID},
  {"@clientPasswd",       COMP_MSG_SPEC_FIELD_CLIENT_PASSWD},
  {"@clientIPAddr",       COMP_MSG_SPEC_FIELD_CLIENT_IP_ADDR},
  {"@clientPort",         COMP_MSG_SPEC_FIELD_CLIENT_PORT},
  {"@clientStatus",       COMP_MSG_SPEC_FIELD_CLIENT_STATUS},
  {"@SSDPIPAddr",         COMP_MSG_SPEC_FIELD_SSDP_IP_ADDR},
  {"@SSDPPort",           COMP_MSG_SPEC_FIELD_SSDP_PORT},
  {"@SSDPStatus",         COMP_MSG_SPEC_FIELD_SSDP_STATUS},
  {"@cloudPort",          COMP_MSG_SPEC_FIELD_CLOUD_PORT},
  {"@cloudSecureConnect", COMP_MSG_SPEC_FIELD_CLOUD_SECURE_CONNECT},
  {"@cloudHost1",         COMP_MSG_SPEC_FIELD_CLOUD_HOST_1},
  {"@cloudHost2",         COMP_MSG_SPEC_FIELD_CLOUD_HOST_2},
  {"@cloudUrl1Part1",     COMP_MSG_SPEC_FIELD_CLOUD_URL_1_PART_1},
  {"@cloudUrl1Part2",     COMP_MSG_SPEC_FIELD_CLOUD_URL_1_PART_2},
  {"@cloudUrlTenantId1",  COMP_MSG_SPEC_FIELD_CLOUD_URL_TENANT_ID_1},
  {"@cloudUrl2Part1",     COMP_MSG_SPEC_FIELD_CLOUD_URL_2_PART_1},
  {"@cloudUrl2Part2",     COMP_MSG_SPEC_FIELD_CLOUD_URL_2_PART_2},
  {"@cloudUrlTenantId2",  COMP_MSG_SPEC_FIELD_CLOUD_URL_TENANT_ID_2},
  {"@cloudNodeToken1",    COMP_MSG_SPEC_FIELD_CLOUD_NODE_TOKEN_1},
  {"@cloudNodeToken2",    COMP_MSG_SPEC_FIELD_CLOUD_NODE_TOKEN_2},
  {"@totalCrc",           COMP_MSG_SPEC_FIELD_TOTAL_CRC},
  {"@otaPort",            COMP_MSG_SPEC_FIELD_OTA_PORT},
  {"@otaRomPath",         COMP_MSG_SPEC_FIELD_OTA_ROM_PATH},
  {"@otaFsPath",          COMP_MSG_SPEC_FIELD_OTA_FS_PATH},
  {"@otaHost",            COMP_MSG_SPEC_FIELD_OTA_HOST},
  {NULL, -1},
};

// ================================= getFieldTypeIdFromStr ====================================

static uint8_t getFieldTypeIdFromStr(compMsgTypesAndNames_t *self, const uint8_t *fieldTypeStr, uint8_t *fieldTypeId) {
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

static uint8_t getFieldTypeStrFromId(compMsgTypesAndNames_t *self, uint8_t fieldTypeId, uint8_t **fieldTypeStr) {
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

// ================================= getFieldNameIdFromStr ====================================

static uint8_t getFieldNameIdFromStr(compMsgTypesAndNames_t *self, const uint8_t *fieldName, uint8_t *fieldNameId, uint8_t incrRefCnt) {
  int firstFreeEntryId;
  int nameIdx;
  fieldName2id_t fieldNameEntry;
  str2id_t *entry;
  fieldName2id_t *newFieldNameEntry;
  fieldName2id_t *nameEntry;
  fieldName2id_t *firstFreeEntry;

//ets_printf("getFieldNameIdFromStr: %s\n", fieldName);
  if (fieldName[0] == '@') {
    // find special field name
    entry = &specialFieldNames[0];
    while (entry->str != NULL) {
      if (entry->str == NULL) {
        break;
      }
      if (c_strcmp(entry->str, fieldName) == 0) {
        *fieldNameId = entry->id;
        return COMP_MSG_ERR_OK;
      }
      entry++;
    }
    return COMP_MSG_ERR_BAD_SPECIAL_FIELD;
  } else {
    if ((incrRefCnt == COMP_MSG_INCR) && (self->fieldNames.numNames >= self->fieldNames.maxNames)) {
      if (self->fieldNames.maxNames == 0) {
        self->fieldNames.maxNames = 4;
        self->fieldNames.names = (fieldName2id_t *)os_zalloc((self->fieldNames.maxNames * sizeof(fieldName2id_t)));
        checkAllocOK(self->fieldNames.names);
      } else {
        self->fieldNames.maxNames += 2;
        self->fieldNames.names = (fieldName2id_t *)os_realloc((self->fieldNames.names), (self->fieldNames.maxNames * sizeof(fieldName2id_t)));
        checkAllocOK(self->fieldNames.names);
      }
    }
    firstFreeEntry = NULL;
    firstFreeEntryId = 0;
    if (self->fieldNames.numNames > 0) {
      // find field name
      nameIdx = 0;
      while (nameIdx < self->fieldNames.numNames) {
        nameEntry = &self->fieldNames.names[nameIdx];
        if ((nameEntry->fieldName != NULL) && (c_strcmp(nameEntry->fieldName, fieldName) == 0)) {
          if (incrRefCnt < 0) {
            if (nameEntry->refCnt > 0) {
              nameEntry->refCnt--;
            }
            if (nameEntry->refCnt == 0) {
              nameEntry->fieldNameId = COMP_MSG_FREE_FIELD_ID;
              os_free(nameEntry->fieldName);
              nameEntry->fieldName = NULL;
            }
          } else {
            if (incrRefCnt > 0) {
              nameEntry->refCnt++;
            } else {
              // just get the entry, do not modify
            }
          }
          *fieldNameId = nameEntry->fieldNameId;
          return COMP_MSG_ERR_OK;
        }
        if ((incrRefCnt == COMP_MSG_INCR) && (nameEntry->fieldNameId == COMP_MSG_FREE_FIELD_ID) && (firstFreeEntry == NULL)) {
          firstFreeEntry = nameEntry;
          firstFreeEntry->fieldNameId = nameIdx + 1;
        }
        nameIdx++;
      }
    }
    if (incrRefCnt < 0) {
      return COMP_MSG_ERR_OK; // just ignore silently
    } else {
      if (incrRefCnt == 0) {
ets_printf("DataView FIELD_NOT_FOUND 1\n");
        return COMP_MSG_ERR_FIELD_NOT_FOUND;
      } else {
        if (firstFreeEntry != NULL) {
          *fieldNameId = firstFreeEntry->fieldNameId;
          firstFreeEntry->refCnt = 1;
          firstFreeEntry->fieldName = os_zalloc(c_strlen(fieldName) + 1);
          firstFreeEntry->fieldName[c_strlen(fieldName)] = '\0';
          c_memcpy(firstFreeEntry->fieldName, fieldName, c_strlen(fieldName));
        } else {
          newFieldNameEntry = &self->fieldNames.names[self->fieldNames.numNames];
          newFieldNameEntry->refCnt = 1;
          newFieldNameEntry->fieldNameId = self->fieldNames.numNames + 1;
          newFieldNameEntry->fieldName = os_zalloc(c_strlen(fieldName) + 1);
          newFieldNameEntry->fieldName[c_strlen(fieldName)] = '\0';
          c_memcpy(newFieldNameEntry->fieldName, fieldName, c_strlen(fieldName));
          self->fieldNames.numNames++;
          *fieldNameId = newFieldNameEntry->fieldNameId;
        }
      }
    }
  }
  return DATA_VIEW_ERR_OK;
}

// ================================= getFieldNameStrFromId ====================================

static uint8_t getFieldNameStrFromId(compMsgTypesAndNames_t *self, uint8_t fieldNameId, uint8_t **fieldName) {
  str2id_t *entry;
  fieldName2id_t *fieldNameEntry;

  *fieldName = NULL;
  // first try to find special field name
  entry = &specialFieldNames[0];
  while (entry->str != NULL) {
    if (entry->id == fieldNameId) {
      *fieldName = entry->str;
      return COMP_MSG_ERR_OK;
    }
    entry++;
  }
  // find field name
  int idx = 0;

  while (idx < self->fieldNames.numNames) {
    fieldNameEntry = &self->fieldNames.names[idx];
    if (fieldNameEntry->fieldNameId == fieldNameId) {
      *fieldName = fieldNameEntry->fieldName;
      return COMP_MSG_ERR_OK;
    }
    fieldNameEntry++;
    idx++;
  }
ets_printf("DataView FIELD_NOT_FOUND 2 fieldNameId: %d\n", fieldNameId);
  return COMP_MSG_ERR_FIELD_NOT_FOUND;
}

// ================================= freeCompMsgTypesAndNames ====================================

static uint8_t freeCompMsgTypesAndNames(compMsgTypesAndNames_t *compMsgTypesAndNames) {
  int idx;
  fieldName2id_t *entry;

  if (compMsgTypesAndNames->fieldNames.numNames != 0) {
    idx = 0;
    while (idx < compMsgTypesAndNames->fieldNames.numNames) {
      entry = &compMsgTypesAndNames->fieldNames.names[idx];
      if (entry->fieldName != NULL) {
        os_free(entry->fieldName);
        entry->fieldName = NULL;
      }
      idx++;
    }
    compMsgTypesAndNames->fieldNames.numNames = 0;
    compMsgTypesAndNames->fieldNames.maxNames = 0;
    os_free(compMsgTypesAndNames->fieldNames.names);
    compMsgTypesAndNames->fieldNames.names = NULL;
  }
  return DATA_VIEW_ERR_OK;
}

// ================================= newCompMsgTypesAndNames ====================================

compMsgTypesAndNames_t *newCompMsgTypesAndNames() {
  compMsgTypesAndNames_t *compMsgTypesAndNames = os_zalloc(sizeof(compMsgTypesAndNames_t));
  if (compMsgTypesAndNames == NULL) {
    return NULL;
  }
  compMsgTypesAndNamesId++;
  compMsgTypesAndNames->id = compMsgTypesAndNamesId;

  compMsgTypesAndNames->fieldNames.numNames = 0;
  compMsgTypesAndNames->fieldNames.maxNames = 0;
  compMsgTypesAndNames->fieldNames.names = NULL;

  compMsgTypesAndNames->getFieldTypeIdFromStr = &getFieldTypeIdFromStr;
  compMsgTypesAndNames->getFieldTypeStrFromId = &getFieldTypeStrFromId;

  compMsgTypesAndNames->getFieldNameIdFromStr = &getFieldNameIdFromStr;
  compMsgTypesAndNames->getFieldNameStrFromId = &getFieldNameStrFromId;

  compMsgTypesAndNames->freeCompMsgTypesAndNames = &freeCompMsgTypesAndNames;
  return compMsgTypesAndNames;
}

