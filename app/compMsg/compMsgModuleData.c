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
 * File:   compMsgModuleData.c
 * Author: Arnulf P. Wiedemann
 *
 * Created on October 7st, 2016
 */

#include "compMsg.h"

static compMsgModuleData_t compMsgModuleData;

const static str2id_t callbackStr2CallbackIds [] = {
  { "@getMACAddr",          COMP_MSG_MODULE_MACAddr},
  { "@getIPAddr",           COMP_MSG_MODULE_IPAddr},
  { "@getFirmwareVersion",  COMP_MSG_MODULE_FirmwareVersion},
  { "@getRSSI",             COMP_MSG_MODULE_RSSI},
  { "@getRSSIMax",          COMP_MSG_MODULE_RSSIMax},
  { "@getConnectionState",  COMP_MSG_MODULE_ConnectionState},
  { "@getConnectedUsers",   COMP_MSG_MODULE_ConnectedUsers},
  { "@getIPProtocol",       COMP_MSG_MODULE_IPProtocol},
  { "@getErrorMain",        COMP_MSG_MODULE_ErrorMain},
  { "@getErrorSub",         COMP_MSG_MODULE_ErrorSub},
  { "@getDateAndTime",      COMP_MSG_MODULE_DateAndTime},
  { "@getSSIDs",            COMP_MSG_MODULE_SSIDs},

  { "@getOperatingMode",    COMP_MSG_MODULE_OperatingMode},
  { "@getOtaHost",          COMP_MSG_MODULE_OtaHost},
  { "@getOtaRomPath",       COMP_MSG_MODULE_OtaRomPath},
  { "@getOtaFsPath",        COMP_MSG_MODULE_OtaFsPath},
  { "@getOtaPort",          COMP_MSG_MODULE_OtaPort},
  { "@getCryptKey",         COMP_MSG_MODULE_CryptKey},

  { "@getReserve1",         COMP_MSG_MODULE_Reserve1},
  { "@getReserve2",         COMP_MSG_MODULE_Reserve2},
  { "@getReserve3",         COMP_MSG_MODULE_Reserve3},
  { "@getReserve4",         COMP_MSG_MODULE_Reserve4},
  { "@getReserve5",         COMP_MSG_MODULE_Reserve5},
  { "@getReserve6",         COMP_MSG_MODULE_Reserve6},
  { "@getReserve7",         COMP_MSG_MODULE_Reserve7},
  { "@getReserve8",         COMP_MSG_MODULE_Reserve8},

#include "compMsgModuleDataCustom.h"

};

// ================================= callbackStr2CallbackId ====================================

static uint8_t callbackStr2CallbackId(uint8_t *callbackName, uint16_t *callbackId) {
  int idx;
  const str2id_t *entry;

  idx = 0;
  entry = &callbackStr2CallbackIds[idx];
  while (entry->str != NULL) {
    if (c_strcmp(entry->str, callbackName) == 0) {
      *callbackId = entry->id;
      return COMP_MSG_ERR_OK;
    }
    idx++;
    entry = &callbackStr2CallbackIds[idx];
  }
  return COMP_MSG_ERR_CALLBACK_NAME_NOT_FOUND;
}

// ================================= getMACAddr ====================================

static uint8_t getMACAddr(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
  int result;
  bool boolResult;
  uint8_t MACAddr[7];

  if (compMsgModuleData.MACAddr[0] == '\0') {
    boolResult = wifi_get_macaddr(STATION_IF, MACAddr);
    COMP_MSG_DBG(self, "MY", 2, "getMacAddr STA: boolResult: %d\n", boolResult);
    if (!boolResult) {
      return COMP_MSG_ERR_CANNOT_GET_MAC_ADDR;
    }
    os_memcpy(compMsgModuleData.MACAddr, MACAddr, 6);
    compMsgModuleData.MACAddr[6] = '\0';
  } else {
    boolResult = wifi_get_macaddr(SOFTAP_IF, MACAddr);
    COMP_MSG_DBG(self, "MY", 2, "getMacAddr AP: boolResult: %d\n", boolResult);
    if (!boolResult) {
      return COMP_MSG_ERR_CANNOT_GET_MAC_ADDR;
    }
    os_memcpy(compMsgModuleData.MACAddr, MACAddr, 6);
    compMsgModuleData.MACAddr[6] = '\0';
  }
int i;
i = 0;
COMP_MSG_DBG(self, "MY", 2, "MACAddr: ");
while (i < 6) {
  COMP_MSG_DBG(self, "MY", 2, " 0x%02x", compMsgModuleData.MACAddr[i]);
  i++;
}
COMP_MSG_DBG(self, "MY", 2, "\n");
//  if (self->compMsgData != NULL) {
//    self->compMsgData->msgValPart->fieldKeyValueStr = compMsgModuleData.MACAddr;
//  }
  *numericValue = 0;
  *stringValue = compMsgModuleData.MACAddr;
  return COMP_MSG_ERR_OK;
}

// ================================= getIPAddr ====================================

static uint8_t getIPAddr(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
#ifdef OLD
  self->compMsgData->msgValPart->fieldKeyValueStr = compMsgModuleData.IPAddr;
#endif
  *numericValue = 0;
  *stringValue = compMsgModuleData.IPAddr;
  return COMP_MSG_ERR_OK;
}

// ================================= getFirmwareVersion ====================================

static uint8_t getFirmwareVersion(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
#ifdef OLD
  self->compMsgData->msgValPart->fieldKeyValueStr = compMsgModuleData.IPAddr;
#endif
  *numericValue = 0;
#ifdef OLD
  self->compMsgData->msgValPart->fieldKeyValueStr = compMsgModuleData.FirmwareVersion;
#endif
  *numericValue = 0;
  *numericValue = 0;
  *stringValue = compMsgModuleData.FirmwareVersion;
  return COMP_MSG_ERR_OK;
}

// ================================= getRSSI ====================================

static uint8_t getRSSI(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
#ifdef OLD
  self->compMsgData->msgValPart->fieldFlags |= COMP_DISP_DESC_VALUE_IS_NUMBER;
  self->compMsgData->msgValPart->fieldValue = compMsgModuleData.RSSI;
#endif
  *numericValue = 0;
  *numericValue = compMsgModuleData.RSSI;
  *stringValue = NULL;
  return COMP_MSG_ERR_OK;
}

// ================================= getRSSIMax ====================================

static uint8_t getRSSIMax(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
#ifdef OLD
  self->compMsgData->msgValPart->fieldFlags |= COMP_DISP_DESC_VALUE_IS_NUMBER;
  self->compMsgData->msgValPart->fieldValue = compMsgModuleData.RSSIMax;
#endif
  *numericValue = 0;
  *numericValue = compMsgModuleData.RSSIMax;
  *stringValue = NULL;
  return COMP_MSG_ERR_OK;
}

// ================================= getConnectionState ====================================

static uint8_t getConnectionState(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
#ifdef OLD
  self->compMsgData->msgValPart->fieldFlags |= COMP_DISP_DESC_VALUE_IS_NUMBER;
  self->compMsgData->msgValPart->fieldValue = compMsgModuleData.ConnectionState;
#endif
  *numericValue = 0;
  *numericValue = compMsgModuleData.ConnectionState;
  *stringValue = NULL;
  return COMP_MSG_ERR_OK;
}

// ================================= getConnectedUsers ====================================

static uint8_t getConnectedUsers(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
#ifdef OLD
  self->compMsgData->msgValPart->fieldFlags |= COMP_DISP_DESC_VALUE_IS_NUMBER;
  self->compMsgData->msgValPart->fieldValue = compMsgModuleData.ConnectedUsers;
#endif
  *numericValue = 0;
  *numericValue = compMsgModuleData.ConnectedUsers;
  *stringValue = NULL;
  return COMP_MSG_ERR_OK;
}

// ================================= getIPProtocol ====================================

static uint8_t getIPProtocol(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
#ifdef OLD
  self->compMsgData->msgValPart->fieldFlags |= COMP_DISP_DESC_VALUE_IS_NUMBER;
  self->compMsgData->msgValPart->fieldValue = compMsgModuleData.IPProtocol;
#endif
  *numericValue = 0;
  *numericValue = compMsgModuleData.IPProtocol;
  *stringValue = NULL;
  return COMP_MSG_ERR_OK;
}

// ================================= getErrorMain ====================================

static uint8_t getErrorMain(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
#ifdef OLD
  self->compMsgData->msgValPart->fieldFlags |= COMP_DISP_DESC_VALUE_IS_NUMBER;
  self->compMsgData->msgValPart->fieldValue = compMsgModuleData.ErrorMain;
#endif
  *numericValue = 0;
  *numericValue = compMsgModuleData.ErrorMain;
  *stringValue = NULL;
  return COMP_MSG_ERR_OK;
}

// ================================= getErrorSub ====================================

static uint8_t getErrorSub(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
#ifdef OLD
  self->compMsgData->msgValPart->fieldFlags |= COMP_DISP_DESC_VALUE_IS_NUMBER;
  self->compMsgData->msgValPart->fieldValue = compMsgModuleData.ErrorSub;
#endif
  *numericValue = 0;
  *numericValue = compMsgModuleData.ErrorSub;
  *stringValue = NULL;
  return COMP_MSG_ERR_OK;
}

// ================================= getDateAndTime ====================================

static uint8_t getDateAndTime(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
#ifdef OLD
  self->compMsgData->msgValPart->fieldKeyValueStr = compMsgModuleData.DateAndTime;
#endif
  *numericValue = 0;
  *numericValue = 0;
  *stringValue = compMsgModuleData.DateAndTime;
  return COMP_MSG_ERR_OK;
}

// ================================= getSSIDs ====================================

static uint8_t getSSIDs(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
#ifdef OLD
  self->compMsgData->msgValPart->fieldFlags |= COMP_DISP_DESC_VALUE_IS_NUMBER;
  self->compMsgData->msgValPart->fieldValue = compMsgModuleData.SSIDs;
#endif
  *numericValue = 0;
  *numericValue = compMsgModuleData.SSIDs;
  *stringValue = NULL;
  return COMP_MSG_ERR_OK;
}

// ================================= getReserve1 ====================================

static uint8_t getReserve1(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
#ifdef OLD
  self->compMsgData->msgValPart->fieldFlags |= COMP_DISP_DESC_VALUE_IS_NUMBER;
  self->compMsgData->msgValPart->fieldValue = (int)compMsgModuleData.Reserve1;
#endif
  *numericValue = 0;
  *numericValue = (int)compMsgModuleData.Reserve1;
  *stringValue = NULL;
  return COMP_MSG_ERR_OK;
}

// ================================= getReserve2 ====================================

static uint8_t getReserve2(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
#ifdef OLD
  self->compMsgData->msgValPart->fieldKeyValueStr = compMsgModuleData.Reserve2;
#endif
  *numericValue = 0;
  *numericValue = 0;
  *stringValue = compMsgModuleData.Reserve2;
  return COMP_MSG_ERR_OK;
}

// ================================= getReserve3 ====================================

static uint8_t getReserve3(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
#ifdef OLD
  self->compMsgData->msgValPart->fieldKeyValueStr = compMsgModuleData.Reserve3;
#endif
  *numericValue = 0;
  *numericValue = 0;
  *stringValue = compMsgModuleData.Reserve3;
  return COMP_MSG_ERR_OK;
}

// ================================= getReserve4 ====================================

static uint8_t getReserve4(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
#ifdef OLD
  self->compMsgData->msgValPart->fieldKeyValueStr = compMsgModuleData.Reserve4;
#endif
  *numericValue = 0;
  *numericValue = 0;
  *stringValue = compMsgModuleData.Reserve4;
  return COMP_MSG_ERR_OK;
}

// ================================= getReserve5 ====================================

static uint8_t getReserve5(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
#ifdef OLD
  self->compMsgData->msgValPart->fieldKeyValueStr = compMsgModuleData.Reserve5;
#endif
  *numericValue = 0;
  *numericValue = 0;
  *stringValue = compMsgModuleData.Reserve5;
  return COMP_MSG_ERR_OK;
}

// ================================= getReserve6 ====================================

static uint8_t getReserve6(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
#ifdef OLD
  self->compMsgData->msgValPart->fieldKeyValueStr = compMsgModuleData.Reserve6;
#endif
  *numericValue = 0;
  *numericValue = 0;
  *stringValue = compMsgModuleData.Reserve6;
  return COMP_MSG_ERR_OK;
}

// ================================= getReserve7 ====================================

static uint8_t getReserve7(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
#ifdef OLD
  self->compMsgData->msgValPart->fieldKeyValueStr = compMsgModuleData.Reserve7;
#endif
  *numericValue = 0;
  *numericValue = 0;
  *stringValue = compMsgModuleData.Reserve7;
  return COMP_MSG_ERR_OK;
}

// ================================= getReserve8 ====================================

static uint8_t getReserve8(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
#ifdef OLD
  self->compMsgData->msgValPart->fieldKeyValueStr = compMsgModuleData.Reserve8;
#endif
  *numericValue = 0;
  *numericValue = 0;
  *stringValue = compMsgModuleData.Reserve8;
  return COMP_MSG_ERR_OK;
}
// ================================= getOperatingMode ====================================

static uint8_t getOperatingMode(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
#ifdef OLD
  self->compMsgData->msgValPart->fieldFlags |= COMP_DISP_DESC_VALUE_IS_NUMBER;
  self->compMsgData->msgValPart->fieldValue = compMsgModuleData.operatingMode;
#endif
  *numericValue = 0;
  COMP_MSG_DBG(self, "Y", 0, "OperatingMode: 0x%02x", compMsgModuleData.operatingMode);
  *numericValue = compMsgModuleData.operatingMode;
  *stringValue = NULL;
  return COMP_MSG_ERR_OK;
}

// ================================= getOtaHost ====================================

static uint8_t getOtaHost(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
//  self->compMsgData->msgValPart->fieldValueStr = compMsgModuleData.otaHost;
  *numericValue = 0;
  *stringValue = compMsgModuleData.otaHost;
  return COMP_MSG_ERR_OK;
}

// ================================= getOtaRomPath ====================================

static uint8_t getOtaRomPath(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
//  self->compMsgData->msgValPart->fieldValueStr = compMsgModuleData.otaRomPath;
  *numericValue = 0;
  *stringValue = compMsgModuleData.otaRomPath;
  return COMP_MSG_ERR_OK;
}

// ================================= getOtaFsPath ====================================

static uint8_t getOtaFsPath(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
  *numericValue = 0;
  *stringValue = compMsgModuleData.otaFsPath;
  return COMP_MSG_ERR_OK;
}

// ================================= getOtaPort ====================================

static uint8_t getOtaPort(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
  *numericValue = compMsgModuleData.otaPort;
  *stringValue = NULL;
  return COMP_MSG_ERR_OK;
}

// ================================= getCryptKey ====================================

static uint8_t getCryptKey(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
  *numericValue = 0;
  *stringValue = compMsgModuleData.cryptKey;
  return COMP_MSG_ERR_OK;
}

// ================================= getCryptIvKey ====================================

static uint8_t getCryptIvKey(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
  *numericValue = 0;
  *stringValue = compMsgModuleData.cryptIvKey;
  return COMP_MSG_ERR_OK;
}

// ================================= restoreUserData ====================================

static uint8_t restoreUserData(compMsgDispatcher_t *self) {
  int result;
  compMsgDataView_t *dataView;
  uint8_t msgDescPartIdx;
  compMsgField_t *fieldInfo;
#ifdef OLD
  msgDescPart_t *msgDescPart;
#endif
  compMsgData_t *compMsgData;
  int numericValue;
  uint8_t valueId;
  uint8_t *stringValue;
  fieldValue_t fieldValue;
  bool userFieldsStarted;

//ets_printf("moduleData: restoreUserData called\n");
  userFieldsStarted = false;
  compMsgData = self->compMsgData;
  dataView = compMsgData->compMsgDataView;
  msgDescPartIdx = 0;
  while (msgDescPartIdx < compMsgData->numFields) {
#ifdef OLD
    msgDescPart = &self->compMsgData->msgDescParts[msgDescPartIdx];
#endif
    fieldInfo = &compMsgData->fields[msgDescPartIdx++];
    // check for stop handling fields
#ifdef OLD
    if ((fieldInfo->fieldNameId == COMP_MSG_SPEC_FIELD_FILLER) || (msgDescPart->fieldNameStr[0] != '@')) {
      userFieldsStarted = false;
    }
    if (userFieldsStarted) {
      result = self->compMsgDataValue->dataValueStr2ValueId(self, msgDescPart->fieldNameStr, &valueId);
      checkErrOK(result);
      result = self->compMsgData->getDataValue(self, msgDescPart->fieldNameStr, &numericValue, &stringValue);
      checkErrOK(result);
//ets_printf("handle field: %s: %d %s\n", msgDescPart->fieldNameStr, numericValue, stringValue == NULL ? "nil" : (char *)stringValue);
      if (stringValue == NULL) {
        fieldValue.flags = COMP_MSG_FIELD_IS_NUMERIC;
        fieldValue.dataValue.value.numericValue = numericValue;
      } else {
        fieldValue.flags = COMP_MSG_FIELD_IS_STRING;
        fieldValue.dataValue.value.stringValue = stringValue;
      }
      fieldValue.cmdKey = COMP_MSG_DATA_VALUE_CMD_KEY_SPECIAL;
      fieldValue.fieldNameId = 0;
      fieldValue.fieldValueId = valueId;
      fieldValue.fieldValueCallback = NULL;
      result = self->compMsgDataValue->setFieldValueInfo(self, &fieldValue);
      checkErrOK(result);
    }
#endif
    // check for start handling fields
    if (fieldInfo->fieldNameId == COMP_MSG_SPEC_FIELD_CMD_KEY) {
      userFieldsStarted = true;
    }
  }
  return COMP_MSG_ERR_OK;
}

// ================================= updateModuleValues ====================================

static uint8_t updateModuleValues(compMsgDispatcher_t *self) {
  int result;

  compMsgModuleData.RSSI = (uint8_t)wifi_station_get_rssi();
  return COMP_MSG_ERR_OK;
}

#include "compMsgModuleDataCustom_c.inc"

// ================================= compMsgModuleDataInit ====================================

static uint8_t compMsgModuleDataInit(compMsgDispatcher_t *self) {
  uint8_t result;
  compMsgModuleData_t *compMsgModuleData;
  compMsgUtil_t *compMsgUtil;

  compMsgModuleData = self->compMsgModuleData;
  compMsgModuleData->callbackStr2CallbackId = &callbackStr2CallbackId;
  compMsgModuleData->updateModuleValues = &updateModuleValues;
  compMsgModuleData->getOtaHost = &getOtaHost;
  compMsgModuleData->getOtaRomPath = &getOtaRomPath;
  compMsgModuleData->getOtaFsPath = &getOtaFsPath;
  compMsgModuleData->getOtaPort = &getOtaPort;
  compMsgModuleData->getMACAddr = &getMACAddr;
  compMsgModuleData->getCryptKey = &getCryptKey;
  compMsgModuleData->getCryptIvKey = &getCryptIvKey;
  compMsgModuleData->restoreUserData = &restoreUserData;
  compMsgModuleData->getOperatingMode = &getOperatingMode;

  compMsgUtil = self->compMsgUtil;

  compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_MODULE_MACAddr, &getMACAddr);
  checkErrOK(result);
  compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_MODULE_IPAddr, &getIPAddr);
  checkErrOK(result);
  compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_MODULE_FirmwareVersion, &getFirmwareVersion);
  checkErrOK(result);
  compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_MODULE_RSSI, &getRSSI);
  checkErrOK(result);
  compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_MODULE_RSSIMax, &getRSSIMax);
  checkErrOK(result);
  compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_MODULE_ConnectionState, &getConnectionState);
  checkErrOK(result);
  compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_MODULE_ConnectedUsers, &getConnectedUsers);
  checkErrOK(result);
  compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_MODULE_IPProtocol, &getIPProtocol);
  checkErrOK(result);
  compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_MODULE_ErrorMain, &getErrorMain);
  checkErrOK(result);
  compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_MODULE_ErrorSub, &getErrorSub);
  checkErrOK(result);
  compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_MODULE_DateAndTime, &getDateAndTime);
  checkErrOK(result);
  compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_MODULE_SSIDs, &getSSIDs);
  checkErrOK(result);

  compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_MODULE_OperatingMode, &getOperatingMode);
  checkErrOK(result);
  compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_MODULE_OtaHost, &getOtaHost);
  checkErrOK(result);
  compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_MODULE_OtaRomPath, &getOtaRomPath);
  checkErrOK(result);
  compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_MODULE_OtaFsPath, &getOtaFsPath);
  checkErrOK(result);
  compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_MODULE_OtaPort, &getOtaPort);
  checkErrOK(result);
  compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_MODULE_CryptKey, &getCryptKey);
  checkErrOK(result);

  compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_MODULE_Reserve1, &getReserve1);
  checkErrOK(result);
  compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_MODULE_Reserve2, &getReserve2);
  checkErrOK(result);
  compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_MODULE_Reserve3, &getReserve3);
  checkErrOK(result);
  compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_MODULE_Reserve4, &getReserve4);
  checkErrOK(result);
  compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_MODULE_Reserve5, &getReserve5);
  checkErrOK(result);
  compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_MODULE_Reserve6, &getReserve6);
  checkErrOK(result);
  compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_MODULE_Reserve7, &getReserve7);
  checkErrOK(result);
  compMsgUtil->addFieldValueCallbackId(self, COMP_MSG_MODULE_Reserve8, &getReserve8);
  checkErrOK(result);
  result = customInit(self);
  checkErrOK(result);

  return COMP_MSG_ERR_OK;
}

// ================================= newCompMsgModuleData ====================================

compMsgModuleData_t *newCompMsgModuleData() {
  compMsgModuleData_t *compMsgModuleData;

  compMsgModuleData = os_zalloc(sizeof(compMsgModuleData_t));
  if (compMsgModuleData == NULL) {
    return NULL;
  }
  compMsgModuleData->compMsgModuleDataInit = &compMsgModuleDataInit;
  return compMsgModuleData;
}
