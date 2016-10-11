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
 * File:   structmsgAction.c
 * Author: Arnulf P. Wiedemann
 *
 * Created on October 7st, 2016
 */

#include "osapi.h"
#include "c_types.h"
#include "mem.h"
#include "flash_fs.h"

//#include "lwip/err.h"
//#include "lwip/app/espconn.h"
//#include "lwip/ip_addr.h"
//#include "lwip/dns.h"

#include "c_string.h"
#include "c_stdlib.h"
#include "c_stdio.h"
#include "platform.h"
#include "structmsg.h"
#include "structmsgModuleData.h"

#define TCP ESPCONN_TCP

typedef uint8_t (* action_t)(structmsgDispatcher_t *self);

typedef struct actionName2Action {
  uint8_t *actionName;
  action_t action;
  uint16_t u16CmdKey;
  uint8_t u8CmdKey;
  uint8_t mode;
} actionName2Action_t;

typedef struct structmsgActionEntries {
  actionName2Action_t **actionEntries;
  uint8_t numActionEntries;
  uint8_t maxActionEntries;
} structmsgActionEntries_t;

typedef struct structmsgActions {
  actionName2Action_t **actions;
  uint8_t numActions;
  uint8_t maxActions;
} structmsgActions_t;

static structmsgActionEntries_t structmsgActionEntries = { NULL, 0, 0 };
static structmsgActions_t structmsgActions = { NULL, 0, 0 };

// ================================= runClientMode ====================================

static uint8_t runClientMode(structmsgDispatcher_t *self, uint8_t mode) {
  int result;

  self->websocketRunClientMode(self, mode);
  return STRUCT_DISP_ERR_OK;
}

// ================================= runAPMode ====================================

static uint8_t runAPMode(structmsgDispatcher_t *self) {
  int result;

  self->websocketRunAPMode(self);
  return STRUCT_DISP_ERR_OK;
}

// ================================= runLightSleepWakeupMode ====================================

static uint8_t runLightSleepWakeupMode(structmsgDispatcher_t *self) {
  int result;
  return STRUCT_DISP_ERR_OK;
}

// ================================= runLightSleepNoWakeupMode ====================================

static uint8_t runLightSleepNoWakeupMode(structmsgDispatcher_t *self) {
  int result;
  return STRUCT_DISP_ERR_OK;
}

// ================================= runWpsMode ====================================

static uint8_t runWpsMode(structmsgDispatcher_t *self) {
  int result;
  return STRUCT_DISP_ERR_OK;
}

// ================================= runModulTestMode ====================================

static uint8_t runModulTestMode(structmsgDispatcher_t *self) {
  int result;
  return STRUCT_DISP_ERR_OK;
}

// ================================= runDeletePasswdCMode ====================================

static uint8_t runDeletePasswdCMode(structmsgDispatcher_t *self) {
  int result;

ets_printf("§runDeletePasswdC§");
  return STRUCT_DISP_ERR_OK;
}

// ================================= getMACAddr ====================================

static uint8_t getMACAddr(structmsgDispatcher_t *self) {
  int result;

  result = self->getModuleValue(self, MODULE_INFO_MACAddr, DATA_VIEW_FIELD_UINT8_VECTOR, &self->numericValue, &self->stringValue);
  return STRUCT_DISP_ERR_OK;
}

// ================================= getIPAddr ====================================

static uint8_t getIPAddr(structmsgDispatcher_t *self) {
  int result;

  result = self->getModuleValue(self, MODULE_INFO_IPAddr, DATA_VIEW_FIELD_UINT8_VECTOR, &self->numericValue, &self->stringValue);
  return STRUCT_DISP_ERR_OK;
}

// ================================= getFirmwareVersion ====================================

static uint8_t getFirmwareVersion(structmsgDispatcher_t *self) {
  int result;

  result = self->getModuleValue(self, MODULE_INFO_FirmwareVersion, DATA_VIEW_FIELD_UINT8_VECTOR, &self->numericValue, &self->stringValue);
  return STRUCT_DISP_ERR_OK;
}

// ================================= getSerieNumber ====================================

static uint8_t getSerieNumber(structmsgDispatcher_t *self) {
  int result;

  result = self->getModuleValue(self, MODULE_INFO_SerieNumber, DATA_VIEW_FIELD_UINT8_VECTOR, &self->numericValue, &self->stringValue);
  return STRUCT_DISP_ERR_OK;
}

// ================================= getRSSI ====================================

static uint8_t getRSSI(structmsgDispatcher_t *self) {
  int result;

  result = self->getModuleValue(self, MODULE_INFO_RSSI, DATA_VIEW_FIELD_UINT8_VECTOR, &self->numericValue, &self->stringValue);
  return STRUCT_DISP_ERR_OK;
}

// ================================= getModuleConnection ====================================

static uint8_t getModuleConnection(structmsgDispatcher_t *self) {
  int result;

  result = self->getModuleValue(self, MODULE_INFO_ModuleConnection, DATA_VIEW_FIELD_UINT8_VECTOR, &self->numericValue, &self->stringValue);
  return STRUCT_DISP_ERR_OK;
}

// ================================= getDeviceMode ====================================

static uint8_t getDeviceMode(structmsgDispatcher_t *self) {
  int result;

  result = self->getModuleValue(self, MODULE_INFO_DeviceMode, DATA_VIEW_FIELD_UINT8_VECTOR, &self->numericValue, &self->stringValue);
  return STRUCT_DISP_ERR_OK;
}

// ================================= getDeviceSecurity ====================================

static uint8_t getDeviceSecurity(structmsgDispatcher_t *self) {
  int result;

  result = self->getModuleValue(self, MODULE_INFO_DeviceSecurity, DATA_VIEW_FIELD_UINT8_VECTOR, &self->numericValue, &self->stringValue);
  return STRUCT_DISP_ERR_OK;
}

// ================================= getErrorMain ====================================

static uint8_t getErrorMain(structmsgDispatcher_t *self) {
  int result;

  result = self->getModuleValue(self, MODULE_INFO_ErrorMain, DATA_VIEW_FIELD_UINT8_VECTOR, &self->numericValue, &self->stringValue);
  return STRUCT_DISP_ERR_OK;
}

// ================================= getErrorSub ====================================

static uint8_t getErrorSub(structmsgDispatcher_t *self) {
  int result;

  result = self->getModuleValue(self, MODULE_INFO_ErrorSub, DATA_VIEW_FIELD_UINT8_VECTOR, &self->numericValue, &self->stringValue);
  return STRUCT_DISP_ERR_OK;
}

// ================================= getDateAndTime ====================================

static uint8_t getDateAndTime(structmsgDispatcher_t *self) {
  int result;

  result = self->getModuleValue(self, MODULE_INFO_DateAndTime, DATA_VIEW_FIELD_UINT8_VECTOR, &self->numericValue, &self->stringValue);
  return STRUCT_DISP_ERR_OK;
}

// ================================= getSSIDs ====================================

static uint8_t getSSIDs(structmsgDispatcher_t *self) {
  int result;

  result = self->getModuleValue(self, MODULE_INFO_SSIDs, DATA_VIEW_FIELD_UINT8_VECTOR, &self->numericValue, &self->stringValue);
  return STRUCT_DISP_ERR_OK;
}

// ================================= getReserve1 ====================================

static uint8_t getReserve1(structmsgDispatcher_t *self) {
  int result;

  result = self->getModuleValue(self, MODULE_INFO_Reserve1, DATA_VIEW_FIELD_UINT8_VECTOR, &self->numericValue, &self->stringValue);
  return STRUCT_DISP_ERR_OK;
}

// ================================= getReserve2 ====================================

static uint8_t getReserve2(structmsgDispatcher_t *self) {
  int result;

  result = self->getModuleValue(self, MODULE_INFO_Reserve2, DATA_VIEW_FIELD_UINT8_VECTOR, &self->numericValue, &self->stringValue);
  return STRUCT_DISP_ERR_OK;
}

// ================================= getReserve3 ====================================

static uint8_t getReserve3(structmsgDispatcher_t *self) {
  int result;

  result = self->getModuleValue(self, MODULE_INFO_Reserve3, DATA_VIEW_FIELD_UINT8_VECTOR, &self->numericValue, &self->stringValue);
  return STRUCT_DISP_ERR_OK;
}

static actionName2Action_t actionName2Actions [] = {
  { "runClientMode",             (action_t)(&runClientMode),             0, 0, 0 },
  { "runAPMode",                 (action_t)(&runAPMode),                 0, 0, 0 },
  { "runLightSleepWakeupMode",   (action_t)(&runLightSleepWakeupMode),   0, 0, 0 },
  { "runLightSleepNoWakeupMode", (action_t)(&runLightSleepNoWakeupMode), 0, 0, 0 },
  { "runWpsMode",                (action_t)(&runWpsMode),                0, 0, 0 },
  { "runTestMode",               (action_t)(&runModulTestMode),          0, 0, 0 },
  { "runDeletePasswdCMode",      (action_t)(&runDeletePasswdCMode),      0, 0, 0 },
  { "getMACAddr",                (action_t)(&getMACAddr),                0, 0, 0 },
  { "getIPAddr",                 (action_t)(&getIPAddr),                 0, 0, 0 },
  { "getFirmwareVersion",        (action_t)(&getFirmwareVersion),        0, 0, 0 },
  { "getSerieNumber",            (action_t)(&getSerieNumber),            0, 0, 0 },
  { "getRSSI",                   (action_t)(&getRSSI),                   0, 0, 0 },
  { "getModuleConnection",       (action_t)(&getModuleConnection),       0, 0, 0 },
  { "getDeviceMode",             (action_t)(&getDeviceMode),             0, 0, 0 },
  { "getDeviceSecurity",         (action_t)(&getDeviceSecurity),         0, 0, 0 },
  { "getErrorMain",              (action_t)(&getErrorMain),              0, 0, 0 },
  { "getErrorSub",               (action_t)(&getErrorSub),               0, 0, 0 },
  { "getDateAndTime",            (action_t)(&getDateAndTime),            0, 0, 0 },
  { "getSSIDs",                  (action_t)(&getSSIDs),                  0, 0, 0 },
  { "getReserve1",               (action_t)(&getReserve1),               0, 0, 0 },
  { "getReserve2",               (action_t)(&getReserve2),               0, 0, 0 },
  { "getReserve3",               (action_t)(&getReserve3),               0, 0, 0 },
  { NULL,                        NULL,                                   0, 0, 0 },
};

// ================================= setActionEntry ====================================

static uint8_t setActionEntry(structmsgDispatcher_t *self, uint8_t *actionName, uint8_t mode, uint8_t u8CmdKey, uint16_t u16CmdKey) {
  int result;
  actionName2Action_t *actionEntry;
  int idx;

  if (structmsgActionEntries.numActionEntries >= structmsgActionEntries.maxActionEntries) {
    structmsgActionEntries.maxActionEntries += 5;
    structmsgActionEntries.actionEntries = (actionName2Action_t **)os_realloc(structmsgActionEntries.actionEntries, (structmsgActionEntries.maxActionEntries * sizeof(actionName2Action_t *)));
    checkAllocOK(structmsgActionEntries.actionEntries);
  }
  idx = 0;
  actionEntry = &actionName2Actions[idx];
  while (actionEntry->actionName != NULL) { 
    if (c_strcmp(actionEntry->actionName, actionName) == 0) {
      structmsgActionEntries.actionEntries[structmsgActionEntries.numActionEntries] = actionEntry;
      if (actionEntry->mode != 0) {
        return STRUCT_DISP_ERR_DUPLICATE_ENTRY;
      }
      actionEntry->mode = mode;
      actionEntry->u8CmdKey = u8CmdKey;
      actionEntry->u16CmdKey = u16CmdKey;
      structmsgActionEntries.numActionEntries++;
      return STRUCT_DISP_ERR_OK;
    }
    idx++;
    actionEntry = &actionName2Actions[idx];
  }
  return STRUCT_DISP_ERR_ACTION_NAME_NOT_FOUND;
}

// ================================= runAction ====================================

static uint8_t runAction(structmsgDispatcher_t *self, uint8_t *answerType) {
  int result;
  msgParts_t *received;
  received = &self->received;
  actionName2Action_t *actionEntry;
  int idx;
  uint8_t actionMode;
  dataView_t *dataView;

  if (received->partsFlags & STRUCT_DISP_U8_CMD_KEY) {
    dataView = self->structmsgDataView->dataView;
    switch (received->u8CmdKey) {
    case 'B':
      result = dataView->getUint8(dataView, 7, &actionMode);
      checkErrOK(result);
      idx = 0;
      actionEntry = &actionName2Actions[idx];
      while (actionEntry->actionName != NULL) { 
//ets_printf("§runActionBu8!%s!%c!%c!§", actionEntry->actionName, received->u8CmdKey, actionMode);
        if ((actionEntry->u8CmdKey == received->u8CmdKey) && (actionMode == actionEntry->mode)) {
//ets_printf("§runActionu8!%c!%c!§", received->u8CmdKey, *answerType);
          result = actionEntry->action(self);
          checkErrOK(result);
          return STRUCT_DISP_ERR_OK;
        }
        idx++;
        actionEntry = &actionName2Actions[idx];
      }
      return STRUCT_DISP_ERR_ACTION_NAME_NOT_FOUND;
      break;
    }
  } else {
ets_printf("§runActionu16!%c%c!%c!§", (received->u16CmdKey>>8)&0xFF, received->u16CmdKey&0xFF, answerType);
  }
  return STRUCT_DISP_ERR_OK;
}

// ================================= fillMsgValue ====================================

static uint8_t fillMsgValue(structmsgDispatcher_t *self, uint8_t *callbackName,int *numericValue, uint8_t **stringValue,  uint8_t answerType, uint8_t fieldTypeId) {
  int result;
  actionName2Action_t *actionEntry;
  int idx;

  // skip the '@' char!
  callbackName++;
  idx = 0;
  actionEntry = &actionName2Actions[idx];
  while (actionEntry->actionName != NULL) { 
    if (c_strcmp(actionEntry->actionName, callbackName) == 0) {
      result = actionEntry->action(self);
      checkErrOK(result);
      *numericValue = self->numericValue;
      *stringValue = self->stringValue;
      return STRUCT_DISP_ERR_OK;
    }
    idx++;
    actionEntry = &actionName2Actions[idx];
  }
  return STRUCT_DISP_ERR_ACTION_NAME_NOT_FOUND;
}

// ================================= structmsgActionInit ====================================

uint8_t structmsgActionInit(structmsgDispatcher_t *self) {
  uint8_t result;
  uint8_t fileName[30];
  long ulgth;
  uint8_t numEntries;
  uint8_t*cp;
  char *endPtr;
  int idx;
  uint8_t lgth;
  uint8_t buf[100];
  uint8_t *buffer;
  uint8_t *actionName;
  uint8_t actionMode;
  structmsgDataView_t *dataView;
  uint8_t *myStr;
  uint8_t fieldTypeId;
  uint8_t u8CmdKey;
  uint16_t u16CmdKey;

  self->setActionEntry = &setActionEntry;
  self->runAction = &runAction;
  self->fillMsgValue = &fillMsgValue;

  buffer = buf;
  structmsgActionEntries.numActionEntries = 0;
  structmsgActionEntries.maxActionEntries = 10;
  structmsgActionEntries.actionEntries = (actionName2Action_t **)os_zalloc(structmsgActionEntries.maxActionEntries * sizeof(actionName2Action_t *));
  checkAllocOK(structmsgActionEntries.actionEntries);

  structmsgActions.numActions = 0;
  structmsgActions.maxActions = 10;
  structmsgActions.actions = (actionName2Action_t **)os_zalloc(structmsgActions.maxActions * sizeof(actionName2Action_t  **));
  checkAllocOK(structmsgActions.actions);

  dataView = self->structmsgDataView;
  os_sprintf(fileName, "MsgActions.txt");
  result = self->openFile(self, fileName, "r");
  checkErrOK(result);
#undef checkErrOK
#define checkErrOK(result) if(result != STRUCT_DISP_ERR_OK) { self->closeFile(self); return result; }
  result = self->readLine(self, &buffer, &lgth);
  checkErrOK(result);
  buffer[lgth] = 0;
  if ((lgth < 4) || (buffer[0] != '#')) {
     return STRUCT_DISP_ERR_BAD_FILE_CONTENTS;
  }
  ulgth = c_strtoul(buffer+2, &endPtr, 10);
  numEntries = (uint8_t)ulgth;
  idx = 0;
  while(idx < numEntries) {
    u8CmdKey = 0;
    u16CmdKey = 0;
    result = self->readLine(self, &buffer, &lgth);
    checkErrOK(result);
    if (lgth == 0) {
      return STRUCT_DISP_ERR_TOO_FEW_FILE_LINES;
    }
    buffer[lgth] = 0;
    cp = buffer;
    // actionName
    actionName = cp;
    while (*cp != ',') {
      cp++;
    }
    *cp++ = '\0';

    // actionMode
    myStr = cp;
    while (*cp != ',') {
      cp++;
    }
    *cp++ = '\0';
    ulgth = c_strtoul(myStr, &endPtr, 10);
    actionMode = (uint8_t)ulgth;

    // type of cmdKey
    myStr = cp;
    while (*cp != ',') {
      cp++;
    }
    *cp++ = '\0';

    result = dataView->dataView->getFieldTypeIdFromStr(dataView->dataView, myStr, &fieldTypeId);
    checkErrOK(result);
    // cmdKey
    myStr = cp;
    while ((*cp != ',') && (*cp != '\n') && (*cp != '\r') && (*cp != '\0')) {
      cp++;
    }
    *cp++ = '\0';
    switch (fieldTypeId) {
    case DATA_VIEW_FIELD_UINT8_T:
      u8CmdKey = myStr[0];
      break;
    case DATA_VIEW_FIELD_UINT16_T:
      u16CmdKey = (myStr[0]<<8)|myStr[1];
      break;
    default:
      checkErrOK(STRUCT_DISP_ERR_BAD_FIELD_TYPE);
    }
    result = setActionEntry(self, actionName, actionMode, u8CmdKey, u16CmdKey);
    checkErrOK(result);
    idx++;
  }
#undef checkErrOK
#define checkErrOK(result) if(result != STRUCT_DISP_ERR_OK) return result
  result = self->closeFile(self);
  checkErrOK(result);
  return STRUCT_DISP_ERR_OK;
}

