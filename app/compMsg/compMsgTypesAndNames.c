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
 * File:   compMsgTypesAndNames.c
 * Author: Arnulf P. Wiedemann
 *
 * Created on November 24, 2016
 */

#include "compMsg.h"

static uint8_t compMsgTypesAndNamesId;

static const str2id_t fieldType2Ids[] = {
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

static const str2id_t specialFieldNames[] = {
  {"@src",                    COMP_MSG_SPEC_FIELD_SRC},
  {"@grp",                    COMP_MSG_SPEC_FIELD_GRP},
  {"@dst",                    COMP_MSG_SPEC_FIELD_DST},
  {"@totalLgth",              COMP_MSG_SPEC_FIELD_TOTAL_LGTH},
  {"@ipAddr",                 COMP_MSG_SPEC_FIELD_IP_ADDR},
  {"@cmdKey",                 COMP_MSG_SPEC_FIELD_CMD_KEY},
  {"@cmdLgth",                COMP_MSG_SPEC_FIELD_CMD_LGTH},
  {"@randomNum",              COMP_MSG_SPEC_FIELD_RANDOM_NUM},
  {"@sequenceNum",            COMP_MSG_SPEC_FIELD_SEQUENCE_NUM},
  {"@filler",                 COMP_MSG_SPEC_FIELD_FILLER},
  {"@crc",                    COMP_MSG_SPEC_FIELD_CRC},
  {"@GUID",                   COMP_MSG_SPEC_FIELD_GUID},
  {"@srcId",                  COMP_MSG_SPEC_FIELD_SRC_ID},
  {"@hdrFiller",              COMP_MSG_SPEC_FIELD_HDR_FILLER},
  {"@numKeyValues",           COMP_MSG_SPEC_FIELD_NUM_KEY_VALUES},

  {"@provisioningSsid",       COMP_MSG_SPEC_FIELD_PROVISIONING_SSID},
  {"@provisioningPort",       COMP_MSG_SPEC_FIELD_PROVISIONING_PORT},
  {"@provisioningIPAddr",     COMP_MSG_SPEC_FIELD_PROVISIONING_IP_ADDR},
  {"@clientSsid",             COMP_MSG_SPEC_FIELD_CLIENT_SSID},
  {"@clientPasswd",           COMP_MSG_SPEC_FIELD_CLIENT_PASSWD},
  {"@clientIPAddr",           COMP_MSG_SPEC_FIELD_CLIENT_IP_ADDR},
  {"@clientPort",             COMP_MSG_SPEC_FIELD_CLIENT_PORT},
  {"@clientStatus",           COMP_MSG_SPEC_FIELD_CLIENT_STATUS},
  {"@SSDPIPAddr",             COMP_MSG_SPEC_FIELD_SSDP_IP_ADDR},
  {"@SSDPPort",               COMP_MSG_SPEC_FIELD_SSDP_PORT},
  {"@SSDPStatus",             COMP_MSG_SPEC_FIELD_SSDP_STATUS},
  {"@cloudPort",              COMP_MSG_SPEC_FIELD_CLOUD_PORT},
  {"@cloudSecureConnect",     COMP_MSG_SPEC_FIELD_CLOUD_SECURE_CONNECT},
  {"@cloudHost1",             COMP_MSG_SPEC_FIELD_CLOUD_HOST_1},
  {"@cloudHost2",             COMP_MSG_SPEC_FIELD_CLOUD_HOST_2},
  {"@cloudUrl1Part1",         COMP_MSG_SPEC_FIELD_CLOUD_URL_1_PART_1},
  {"@cloudUrl1Part2",         COMP_MSG_SPEC_FIELD_CLOUD_URL_1_PART_2},
  {"@cloudUrlTenantId1",      COMP_MSG_SPEC_FIELD_CLOUD_URL_TENANT_ID_1},
  {"@cloudUrl2Part1",         COMP_MSG_SPEC_FIELD_CLOUD_URL_2_PART_1},
  {"@cloudUrl2Part2",         COMP_MSG_SPEC_FIELD_CLOUD_URL_2_PART_2},
  {"@cloudUrlTenantId2",      COMP_MSG_SPEC_FIELD_CLOUD_URL_TENANT_ID_2},
  {"@cloudNodeToken1",        COMP_MSG_SPEC_FIELD_CLOUD_NODE_TOKEN_1},
  {"@cloudNodeToken2",        COMP_MSG_SPEC_FIELD_CLOUD_NODE_TOKEN_2},
  {"@totalCrc",               COMP_MSG_SPEC_FIELD_TOTAL_CRC},
  {"@otaPort",                COMP_MSG_SPEC_FIELD_OTA_PORT},
  {"@otaRomPath",             COMP_MSG_SPEC_FIELD_OTA_ROM_PATH},
  {"@otaFsPath",              COMP_MSG_SPEC_FIELD_OTA_FS_PATH},
  {"@otaHost",                COMP_MSG_SPEC_FIELD_OTA_HOST},
  {"@prodTestSsid",           COMP_MSG_SPEC_FIELD_PROD_TEST_SSID},
  {"@prodTestPasswd",         COMP_MSG_SPEC_FIELD_PROD_TEST_PASSWD},
  {"@prodTestSecurityType",   COMP_MSG_SPEC_FIELD_PROD_TEST_SECURITY_TYPE},
  {"@prodTestTargetProtocol", COMP_MSG_SPEC_FIELD_PROD_TEST_TARGET_PROTOCOL},
  {"@prodTestIpAddr",         COMP_MSG_SPEC_FIELD_PROD_TEST_IP_ADDR},
  {"@prodTestSubnet",         COMP_MSG_SPEC_FIELD_PROD_TEST_SUBNET},
  {"@prodTestGateway",        COMP_MSG_SPEC_FIELD_PROD_TEST_GATEWAY},
  {"@prodTestDns",            COMP_MSG_SPEC_FIELD_PROD_TEST_DNS},
  {"@prodTestPingAddress",    COMP_MSG_SPEC_FIELD_PROD_TEST_PING_ADDRESS},
  {"@prodTestStatus",         COMP_MSG_SPEC_FIELD_PROD_TEST_STATUS},
  {"@hdrReserve",             COMP_MSG_SPEC_FIELD_HDR_RESERVE},
  {NULL, -1},
};

static const str2id_t fieldGroupStr2Ids [] = {
  {"@$msgDescHeader",    COMP_MSG_DESC_HEADER_FIELD_GROUP},
  {"@$msgDescMidPart",   COMP_MSG_DESC_MID_PART_FIELD_GROUP},
  {"@$msgDescTrailer",   COMP_MSG_DESC_TRAILER_FIELD_GROUP},
  {"@$msgDescKeyValue",  COMP_MSG_DESC_KEY_VALUE_FIELD_GROUP},
  {"@$msgFieldsToSave",  COMP_MSG_FIELDS_TO_SAVE_FIELD_GROUP},
  {"@$msgHeads",         COMP_MSG_HEADS_FIELD_GROUP},
  {"@$msgActions",       COMP_MSG_ACTIONS_FIELD_GROUP},
  {"@$msgValHeader",     COMP_MSG_VAL_HEADER_FIELD_GROUP},
  {"@$msgWifiData",      COMP_MSG_WIFI_DATA_VALUES_FIELD_GROUP},
  {"@$msgModuleData",    COMP_MSG_MODULE_DATA_VALUES_FIELD_GROUP},
  {"@$msgDesc",          COMP_MSG_DESC_FIELD_GROUP},
  {"@$msgVal",           COMP_MSG_VAL_FIELD_GROUP},
  {NULL, -1},
};

// ================================= getFieldTypeIdFromStr ====================================

static uint8_t getFieldTypeIdFromStr(compMsgDispatcher_t *self, const uint8_t *fieldTypeStr, uint8_t *fieldTypeId) {
  const str2id_t *entry;

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

static uint8_t getFieldTypeStrFromId(compMsgDispatcher_t *self, uint8_t fieldTypeId, uint8_t **fieldTypeStr) {
  const str2id_t *entry;

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

static uint8_t getFieldNameIdFromStr(compMsgDispatcher_t *self, const uint8_t *fieldName, uint8_t *fieldNameId, uint8_t incrRefCnt) {
  uint8_t result;
  int firstFreeEntryId;
  int nameIdx;
  fieldName2id_t fieldNameEntry;
  const str2id_t *entry;
  fieldName2id_t *newFieldNameEntry;
  fieldName2id_t *nameEntry;
  fieldName2id_t *firstFreeEntry;
  compMsgTypesAndNames_t *compMsgTypesAndNames;

  compMsgTypesAndNames = self->compMsgTypesAndNames;
  if ((fieldName[0] == '@') && (fieldName[1] != '#') && (fieldName[1] != '$')) {
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
    if ((incrRefCnt == COMP_MSG_INCR) && (compMsgTypesAndNames->fieldNames.numNames >= compMsgTypesAndNames->fieldNames.maxNames)) {
      if (compMsgTypesAndNames->fieldNames.maxNames == 0) {
        compMsgTypesAndNames->fieldNames.maxNames = 4;
        compMsgTypesAndNames->fieldNames.names = (fieldName2id_t *)os_zalloc((compMsgTypesAndNames->fieldNames.maxNames * sizeof(fieldName2id_t)));
        checkAllocOK(compMsgTypesAndNames->fieldNames.names);
      } else {
        compMsgTypesAndNames->fieldNames.maxNames += 2;
        compMsgTypesAndNames->fieldNames.names = (fieldName2id_t *)os_realloc((compMsgTypesAndNames->fieldNames.names), (compMsgTypesAndNames->fieldNames.maxNames * sizeof(fieldName2id_t)));
        checkAllocOK(compMsgTypesAndNames->fieldNames.names);
      }
    }
    firstFreeEntry = NULL;
    firstFreeEntryId = 0;
    if (compMsgTypesAndNames->fieldNames.numNames > 0) {
      // find field name
      nameIdx = 0;
      while (nameIdx < compMsgTypesAndNames->fieldNames.numNames) {
        nameEntry = &compMsgTypesAndNames->fieldNames.names[nameIdx];
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
          *fieldNameId = (nameEntry->fieldNameId + compMsgTypesAndNames->numSpecFieldIds);
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
        dbgPrintf(NULL, "Y", 0, "DataView FIELD_NOT_FOUND 1");
        return COMP_MSG_ERR_FIELD_NOT_FOUND;
      } else {
        if (firstFreeEntry != NULL) {
          *fieldNameId = (firstFreeEntry->fieldNameId + compMsgTypesAndNames->numSpecFieldIds);
          firstFreeEntry->refCnt = 1;
          firstFreeEntry->fieldName = os_zalloc(c_strlen(fieldName) + 1);
          firstFreeEntry->fieldName[c_strlen(fieldName)] = '\0';
          c_memcpy(firstFreeEntry->fieldName, fieldName, c_strlen(fieldName));
        } else {
          // add a free slot in msgFieldInfos
	  result = compMsgTypesAndNames->addMsgFieldInfos(self, 1);
	  checkErrOK(result);
	  result = self->compMsgDataValue->addMsgFieldValues(self, 1);
	  checkErrOK(result);
          newFieldNameEntry = &compMsgTypesAndNames->fieldNames.names[compMsgTypesAndNames->fieldNames.numNames];
          newFieldNameEntry->refCnt = 1;
          newFieldNameEntry->fieldNameId = compMsgTypesAndNames->fieldNames.numNames + 1;
          newFieldNameEntry->fieldName = os_zalloc(c_strlen(fieldName) + 1);
          newFieldNameEntry->fieldName[c_strlen(fieldName)] = '\0';
          c_memcpy(newFieldNameEntry->fieldName, fieldName, c_strlen(fieldName));
          compMsgTypesAndNames->fieldNames.numNames++;
          *fieldNameId = (newFieldNameEntry->fieldNameId + compMsgTypesAndNames->numSpecFieldIds);
        }
      }
    }
  }
  return DATA_VIEW_ERR_OK;
}

// ================================= getFieldNameStrFromId ====================================

static uint8_t getFieldNameStrFromId(compMsgDispatcher_t *self, uint8_t fieldNameId, uint8_t **fieldName) {
  const str2id_t *entry;
  fieldName2id_t *fieldNameEntry;
  compMsgTypesAndNames_t *compMsgTypesAndNames;

  compMsgTypesAndNames = self->compMsgTypesAndNames;
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
  // these field names have also ids starting at 1 but for fieldInfo handling we add the numSpecFieldIds to get
  // unique ids!
  fieldNameId -= compMsgTypesAndNames->numSpecFieldIds;
  while (idx < compMsgTypesAndNames->fieldNames.numNames) {
    fieldNameEntry = &compMsgTypesAndNames->fieldNames.names[idx];
    if (fieldNameEntry->fieldNameId == fieldNameId) {
      *fieldName = fieldNameEntry->fieldName;
      return COMP_MSG_ERR_OK;
    }
    fieldNameEntry++;
    idx++;
  }
  dbgPrintf(NULL, "Y", 0, "TypesAndNames FIELD_NOT_FOUND 2 fieldNameId: %d", fieldNameId);
  return COMP_MSG_ERR_FIELD_NOT_FOUND;
}

// ================================= getFileNameTokenIdFromStr ====================================

static uint8_t getFileNameTokenIdFromStr(compMsgDispatcher_t *self, const uint8_t *fileNameTokenStr, uint8_t *fileNameTokenId) {
  const str2id_t *entry;

  entry = &fieldGroupStr2Ids[0];
  while (entry->str != NULL) {
    if (c_strcmp(fileNameTokenStr, entry->str) == 0) {
      *fileNameTokenId = entry->id;
      return DATA_VIEW_ERR_OK;
    }
    entry++;
  }
  return DATA_VIEW_ERR_FILE_NAME_TOKEN_NOT_FOUND;
}

// ================================= dumpMsgFieldInfos ====================================

static uint8_t dumpMsgFieldInfos(compMsgDispatcher_t *self) {
  int idx;
  int result;
  char buf[512];
  uint8_t *fieldName;
  uint8_t *fieldType;
  compMsgTypesAndNames_t *compMsgTypesAndNames;
  fieldInfo_t *fieldInfo;

  result = COMP_MSG_ERR_OK;
  compMsgTypesAndNames = self->compMsgTypesAndNames;
  COMP_MSG_DBG(self, "d", 1, "dumpMsgFieldInfos");
  // entry 0 is not used, fieldNameIds start at 1!
  idx = 1;
  while (idx < compMsgTypesAndNames->msgFieldInfos.numMsgFields) {
    if (idx == compMsgTypesAndNames->numSpecFieldIds) {
      idx++;
      continue;
    }
    fieldInfo = compMsgTypesAndNames->msgFieldInfos.fieldInfos[idx];
    result = compMsgTypesAndNames->getFieldNameStrFromId(self, idx, &fieldName);
    checkErrOK(result);
    if (fieldInfo == NULL) {
      ets_sprintf(buf, "%3d %-20s empty", idx, fieldName);
    } else {
      result = compMsgTypesAndNames->getFieldTypeStrFromId(self, fieldInfo->fieldTypeId, &fieldType);
      checkErrOK(result);
      ets_sprintf(buf, "%3d %-20s type: %-10s %d lgth: %3d flags: 0x%04x", idx, fieldName, fieldType, fieldInfo->fieldTypeId, fieldInfo->fieldLgth, fieldInfo->fieldFlags);
      if (fieldInfo->fieldFlags & COMP_MSG_FIELD_WIFI_DATA) {
        c_strcat(buf, " WIFI_DATA");
      }
      if (fieldInfo->fieldFlags & COMP_MSG_FIELD_MODULE_DATA) {
        c_strcat(buf, " MODULE_DATA");
      }
      if (fieldInfo->fieldFlags & COMP_MSG_FIELD_TO_SAVE) {
        c_strcat(buf, " TO_SAVE");
      }
      if (fieldInfo->fieldFlags & COMP_MSG_FIELD_KEY_VALUE) {
        c_strcat(buf, " KEY_VALUE");
      }
      if (fieldInfo->fieldFlags & COMP_MSG_FIELD_HEADER) {
        c_strcat(buf, " HEADER");
      }
      if (fieldInfo->fieldFlags & COMP_MSG_FIELD_HEADER_UNIQUE) {
        c_strcat(buf, " HEADER_UNIQUE");
      }
      if (fieldInfo->fieldFlags & COMP_MSG_FIELD_HEADER_CHKSUM_NON_ZERO) {
        c_strcat(buf, " HEADER_CHKSUM_NON_ZERO");
      }
    }
    COMP_MSG_DBG(self, "d", 1, "%s", buf);
    idx++;
  }
  return result;
}

// ================================= addMsgFieldInfos ====================================

static uint8_t addMsgFieldInfos(compMsgDispatcher_t *self, uint8_t numEntries) {
  uint8_t result;
  int idx;
  msgFieldInfos_t *msgFieldInfos;
  fieldInfo_t *fieldInfo;

  result = COMP_MSG_ERR_OK;
  msgFieldInfos = &self->compMsgTypesAndNames->msgFieldInfos;
  if (msgFieldInfos->numMsgFields == 0) {
    msgFieldInfos->fieldInfos = (fieldInfo_t **)os_zalloc(((msgFieldInfos->numMsgFields + numEntries) * sizeof(fieldInfo_t *)));
  } else {
    msgFieldInfos->fieldInfos = (fieldInfo_t **)os_realloc(msgFieldInfos->fieldInfos, ((msgFieldInfos->numMsgFields + numEntries) * sizeof(fieldInfo_t *)));
  }
  checkAllocOK(msgFieldInfos->fieldInfos);
  idx = msgFieldInfos->numMsgFields;
  COMP_MSG_DBG(self, "E", 2, "addMsgFieldInfos: numEntries: %d, numMsgFields: %d", numEntries, msgFieldInfos->numMsgFields);
  while (idx < msgFieldInfos->numMsgFields + numEntries) {
    msgFieldInfos->fieldInfos[idx] = (fieldInfo_t *)NULL;
    idx++;
  }
  msgFieldInfos->numMsgFields += numEntries;
  return result;
}

// ================================= getMsgFieldInfo ====================================

static uint8_t getMsgFieldInfo(compMsgDispatcher_t *self, uint8_t idx, fieldInfo_t *fieldInfo) {
  uint8_t result;
  msgFieldInfos_t *msgFieldInfos;
  fieldInfo_t *entry;

  result = COMP_MSG_ERR_OK;
  msgFieldInfos = &self->compMsgTypesAndNames->msgFieldInfos;
  if (idx >= msgFieldInfos->numMsgFields) {
    return COMP_MSG_ERR_BAD_MSG_FIELD_INFO_IDX;
  }
  entry = msgFieldInfos->fieldInfos[idx];
  if (entry == NULL) {
    msgFieldInfos->fieldInfos[idx] = os_zalloc(sizeof(fieldInfo_t));
    checkAllocOK(msgFieldInfos->fieldInfos[idx]);
    entry = msgFieldInfos->fieldInfos[idx];
  }
  fieldInfo->fieldFlags = entry->fieldFlags;
  fieldInfo->fieldTypeId = entry->fieldTypeId;
  fieldInfo->fieldLgth = entry->fieldLgth;
  fieldInfo->fieldOffset = entry->fieldOffset;
  fieldInfo->keyValueDesc = entry->keyValueDesc;
  return result;
}

// ================================= setMsgFieldInfo ====================================

static uint8_t setMsgFieldInfo(compMsgDispatcher_t *self, uint8_t idx, fieldInfo_t *fieldInfo) {
  uint8_t result;
  uint8_t *fieldName;
  msgFieldInfos_t *msgFieldInfos;
  fieldInfo_t *entry;

  result = COMP_MSG_ERR_OK;
  msgFieldInfos = &self->compMsgTypesAndNames->msgFieldInfos;
  if (idx >= msgFieldInfos->numMsgFields) {
    return COMP_MSG_ERR_BAD_MSG_FIELD_INFO_IDX;
  }
  entry = msgFieldInfos->fieldInfos[idx];
  if (entry == NULL) {
    msgFieldInfos->fieldInfos[idx] = os_zalloc(sizeof(fieldInfo_t));
    checkAllocOK(msgFieldInfos->fieldInfos[idx]);
    entry = msgFieldInfos->fieldInfos[idx];
  }
  entry->fieldFlags = fieldInfo->fieldFlags;
  entry->fieldTypeId = fieldInfo->fieldTypeId;
  entry->fieldLgth = fieldInfo->fieldLgth;
  entry->fieldOffset = fieldInfo->fieldOffset;
  if ((entry->keyValueDesc == NULL) && (fieldInfo->keyValueDesc != NULL)) {
    entry->keyValueDesc = os_zalloc(sizeof(keyValueDesc_t));
    checkAllocOK(entry->keyValueDesc);
  }
  if (fieldInfo->keyValueDesc != NULL) {
    entry->keyValueDesc->keyId = fieldInfo->keyValueDesc->keyId;
    entry->keyValueDesc->keyAccess = fieldInfo->keyValueDesc->keyAccess; 
    entry->keyValueDesc->keyType = fieldInfo->keyValueDesc->keyType;
    entry->keyValueDesc->keyLgth = fieldInfo->keyValueDesc->keyLgth;
    entry->keyValueDesc->keyNumValues = fieldInfo->keyValueDesc->keyNumValues;
  }
  result = self->compMsgTypesAndNames->getFieldNameStrFromId(self, idx, &fieldName);
  checkErrOK(result);
  COMP_MSG_DBG(self, "E", 0, "setMsgFieldInfo: %s idx: %d fieldFlags: 0x%08x fieldType: %d fieldLgth: %d", fieldName, idx, entry->fieldFlags, entry->fieldTypeId, entry->fieldLgth);
  return result;
}

// ================================= freeCompMsgTypesAndNames ====================================

static uint8_t freeCompMsgTypesAndNames(compMsgDispatcher_t *self) {
  int idx;
  fieldName2id_t *entry;
  compMsgTypesAndNames_t *compMsgTypesAndNames;

  compMsgTypesAndNames = self->compMsgTypesAndNames;
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

// ================================= compMsgTypesAndNamesInit ====================================

uint8_t compMsgTypesAndNamesInit(compMsgDispatcher_t *self) {
  compMsgTypesAndNames_t *compMsgTypesAndNames;

  compMsgTypesAndNames = self->compMsgTypesAndNames;
  compMsgTypesAndNames->dumpMsgFieldInfos = &dumpMsgFieldInfos;
  compMsgTypesAndNames->addMsgFieldInfos = &addMsgFieldInfos;
  compMsgTypesAndNames->setMsgFieldInfo = &setMsgFieldInfo;
  compMsgTypesAndNames->getMsgFieldInfo = &getMsgFieldInfo;

  compMsgTypesAndNames->getFieldTypeIdFromStr = &getFieldTypeIdFromStr;
  compMsgTypesAndNames->getFieldTypeStrFromId = &getFieldTypeStrFromId;

  compMsgTypesAndNames->getFieldNameIdFromStr = &getFieldNameIdFromStr;
  compMsgTypesAndNames->getFieldNameStrFromId = &getFieldNameStrFromId;

  compMsgTypesAndNames->getFileNameTokenIdFromStr = &getFileNameTokenIdFromStr;

  compMsgTypesAndNames->freeCompMsgTypesAndNames = &freeCompMsgTypesAndNames;

  compMsgTypesAndNames->numSpecFieldIds = sizeof(specialFieldNames) / sizeof(str2id_t);
  // +1 as the following entries also start at 1!!
  compMsgTypesAndNames->addMsgFieldInfos(self, compMsgTypesAndNames->numSpecFieldIds + 1);
  return COMP_MSG_ERR_OK;
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
  return compMsgTypesAndNames;
}

