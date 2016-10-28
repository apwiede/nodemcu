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
 * File:   CompMsgAction.c
 * Author: Arnulf P. Wiedemann
 *
 * Created on October 7st, 2016
 */

#include "osapi.h"
#include "c_types.h"
#include "mem.h"
#include "flash_fs.h"
#include "c_string.h"
#include "c_stdlib.h"
#include "c_stdio.h"
#include "platform.h"
#include "compMsgData.h"

#define TCP ESPCONN_TCP

typedef uint8_t (* action_t)(compMsgDispatcher_t *self);

typedef struct actionName2Action {
  uint8_t *actionName;
  action_t action;
  uint16_t u16CmdKey;
  uint8_t u8CmdKey;
  uint8_t mode;
} actionName2Action_t;

typedef struct compMsgActionEntries {
  actionName2Action_t **actionEntries;
  uint8_t numActionEntries;
  uint8_t maxActionEntries;
} compMsgActionEntries_t;

typedef struct compMsgActions {
  actionName2Action_t **actions;
  uint8_t numActions;
  uint8_t maxActions;
} compMsgActions_t;

static compMsgActionEntries_t compMsgActionEntries = { NULL, 0, 0 };
static compMsgActions_t compMsgActions = { NULL, 0, 0 };

// ================================= runClientMode ====================================

static uint8_t runClientMode(compMsgDispatcher_t *self, uint8_t mode) {
  int result;

  self->websocketRunClientMode(self, mode);
  return COMP_DISP_ERR_OK;
}

// ================================= runAPMode ====================================

static uint8_t runAPMode(compMsgDispatcher_t *self) {
  int result;

  self->websocketRunAPMode(self);
  return COMP_DISP_ERR_OK;
}

// ================================= runLightSleepWakeupMode ====================================

static uint8_t runLightSleepWakeupMode(compMsgDispatcher_t *self) {
  int result;
  return COMP_DISP_ERR_OK;
}

// ================================= runLightSleepNoWakeupMode ====================================

static uint8_t runLightSleepNoWakeupMode(compMsgDispatcher_t *self) {
  int result;
  return COMP_DISP_ERR_OK;
}

// ================================= runWpsMode ====================================

static uint8_t runWpsMode(compMsgDispatcher_t *self) {
  int result;
  return COMP_DISP_ERR_OK;
}

// ================================= runModulTestMode ====================================

static uint8_t runModulTestMode(compMsgDispatcher_t *self) {
  int result;
  return COMP_DISP_ERR_OK;
}

// ================================= runDeletePasswdCMode ====================================

static uint8_t runDeletePasswdCMode(compMsgDispatcher_t *self) {
  int result;

ets_printf("§runDeletePasswdC§");
  return COMP_DISP_ERR_OK;
}

// ================================= getMACAddr ====================================

static uint8_t getMACAddr(compMsgDispatcher_t *self) {
  int result;

  result = self->getModuleValue(self, MODULE_INFO_MACAddr, DATA_VIEW_FIELD_UINT8_VECTOR, &self->numericValue, &self->stringValue);
  return COMP_DISP_ERR_OK;
}

// ================================= getIPAddr ====================================

static uint8_t getIPAddr(compMsgDispatcher_t *self) {
  int result;

  result = self->getModuleValue(self, MODULE_INFO_IPAddr, DATA_VIEW_FIELD_UINT8_VECTOR, &self->numericValue, &self->stringValue);
  return COMP_DISP_ERR_OK;
}

// ================================= getFirmwareVersion ====================================

static uint8_t getFirmwareVersion(compMsgDispatcher_t *self) {
  int result;

  result = self->getModuleValue(self, MODULE_INFO_FirmwareVersion, DATA_VIEW_FIELD_UINT8_VECTOR, &self->numericValue, &self->stringValue);
  return COMP_DISP_ERR_OK;
}

// ================================= getSerieNumber ====================================

static uint8_t getSerieNumber(compMsgDispatcher_t *self) {
  int result;

  result = self->getModuleValue(self, MODULE_INFO_SerieNumber, DATA_VIEW_FIELD_UINT8_VECTOR, &self->numericValue, &self->stringValue);
  return COMP_DISP_ERR_OK;
}

// ================================= getRSSI ====================================

static uint8_t getRSSI(compMsgDispatcher_t *self) {
  int result;

  result = self->getModuleValue(self, MODULE_INFO_RSSI, DATA_VIEW_FIELD_UINT8_VECTOR, &self->numericValue, &self->stringValue);
  return COMP_DISP_ERR_OK;
}

// ================================= getModuleConnection ====================================

static uint8_t getModuleConnection(compMsgDispatcher_t *self) {
  int result;

  result = self->getModuleValue(self, MODULE_INFO_ModuleConnection, DATA_VIEW_FIELD_UINT8_VECTOR, &self->numericValue, &self->stringValue);
  return COMP_DISP_ERR_OK;
}

// ================================= getDeviceMode ====================================

static uint8_t getDeviceMode(compMsgDispatcher_t *self) {
  int result;

  result = self->getModuleValue(self, MODULE_INFO_DeviceMode, DATA_VIEW_FIELD_UINT8_VECTOR, &self->numericValue, &self->stringValue);
  return COMP_DISP_ERR_OK;
}

// ================================= getDeviceSecurity ====================================

static uint8_t getDeviceSecurity(compMsgDispatcher_t *self) {
  int result;

  result = self->getModuleValue(self, MODULE_INFO_DeviceSecurity, DATA_VIEW_FIELD_UINT8_VECTOR, &self->numericValue, &self->stringValue);
  return COMP_DISP_ERR_OK;
}

// ================================= getErrorMain ====================================

static uint8_t getErrorMain(compMsgDispatcher_t *self) {
  int result;

  result = self->getModuleValue(self, MODULE_INFO_ErrorMain, DATA_VIEW_FIELD_UINT8_VECTOR, &self->numericValue, &self->stringValue);
  return COMP_DISP_ERR_OK;
}

// ================================= getErrorSub ====================================

static uint8_t getErrorSub(compMsgDispatcher_t *self) {
  int result;

  result = self->getModuleValue(self, MODULE_INFO_ErrorSub, DATA_VIEW_FIELD_UINT8_VECTOR, &self->numericValue, &self->stringValue);
  return COMP_DISP_ERR_OK;
}

// ================================= getDateAndTime ====================================

static uint8_t getDateAndTime(compMsgDispatcher_t *self) {
  int result;

  result = self->getModuleValue(self, MODULE_INFO_DateAndTime, DATA_VIEW_FIELD_UINT8_VECTOR, &self->numericValue, &self->stringValue);
  return COMP_DISP_ERR_OK;
}

// ================================= getSSIDs ====================================

static uint8_t getSSIDs(compMsgDispatcher_t *self) {
  int result;

  result = self->getModuleValue(self, MODULE_INFO_SSIDs, DATA_VIEW_FIELD_UINT8_VECTOR, &self->numericValue, &self->stringValue);
  return COMP_DISP_ERR_OK;
}

// ================================= getReserve1 ====================================

static uint8_t getReserve1(compMsgDispatcher_t *self) {
  int result;

  result = self->getModuleValue(self, MODULE_INFO_Reserve1, DATA_VIEW_FIELD_UINT8_VECTOR, &self->numericValue, &self->stringValue);
  return COMP_DISP_ERR_OK;
}

// ================================= getReserve2 ====================================

static uint8_t getReserve2(compMsgDispatcher_t *self) {
  int result;

  result = self->getModuleValue(self, MODULE_INFO_Reserve2, DATA_VIEW_FIELD_UINT8_VECTOR, &self->numericValue, &self->stringValue);
  return COMP_DISP_ERR_OK;
}

// ================================= getReserve3 ====================================

static uint8_t getReserve3(compMsgDispatcher_t *self) {
  int result;

  result = self->getModuleValue(self, MODULE_INFO_Reserve3, DATA_VIEW_FIELD_UINT8_VECTOR, &self->numericValue, &self->stringValue);
  return COMP_DISP_ERR_OK;
}

// ================================= getAPList ====================================

static uint8_t getAPList(compMsgDispatcher_t *self) {
  uint8_t result;

  result = self->getBssScanInfo(self);
  return result;
}

// ================================= getTableValue ====================================

static uint8_t getTableValue(compMsgDispatcher_t *self) {
  uint8_t result;

  result = self->getModuleTableFieldValue(self, MODULE_INFO_AP_LIST_CALL_BACK);
  return result;
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
  { "getAPList",                 (action_t)(&getAPList),                 0, 0, MODULE_INFO_AP_LIST_CALL_BACK },
  { "getTableValue",             (action_t)(&getTableValue),             0x4141, 0, MODULE_INFO_AP_LIST_CALL_BACK },
  { NULL,                        NULL,                                   0, 0, 0 },
};

// ================================= getActionMode ====================================

static uint8_t getActionMode(compMsgDispatcher_t *self, uint8_t *actionName, uint8_t *actionMode) {
  int result;
  actionName2Action_t *actionEntry;
  int idx;

  idx = 0;
  actionEntry = &actionName2Actions[idx];
  while (actionEntry->actionName != NULL) { 
    if (c_strcmp(actionEntry->actionName, actionName) == 0) {
      *actionMode = actionEntry->mode;
      return COMP_DISP_ERR_OK;
    }
    idx++;
    actionEntry = &actionName2Actions[idx];
  }
  return COMP_DISP_ERR_ACTION_NAME_NOT_FOUND;
}

// ================================= setActionEntry ====================================

static uint8_t setActionEntry(compMsgDispatcher_t *self, uint8_t *actionName, uint8_t mode, uint8_t u8CmdKey, uint16_t u16CmdKey) {
  int result;
  actionName2Action_t *actionEntry;
  int idx;

  if (compMsgActionEntries.numActionEntries >= compMsgActionEntries.maxActionEntries) {
    compMsgActionEntries.maxActionEntries += 5;
    compMsgActionEntries.actionEntries = (actionName2Action_t **)os_realloc(compMsgActionEntries.actionEntries, (compMsgActionEntries.maxActionEntries * sizeof(actionName2Action_t *)));
    checkAllocOK(compMsgActionEntries.actionEntries);
  }
  idx = 0;
  actionEntry = &actionName2Actions[idx];
  while (actionEntry->actionName != NULL) { 
    if (c_strcmp(actionEntry->actionName, actionName) == 0) {
      compMsgActionEntries.actionEntries[compMsgActionEntries.numActionEntries] = actionEntry;
      if (actionEntry->mode != 0) {
        return COMP_DISP_ERR_DUPLICATE_ENTRY;
      }
      actionEntry->mode = mode;
      actionEntry->u8CmdKey = u8CmdKey;
      actionEntry->u16CmdKey = u16CmdKey;
      compMsgActionEntries.numActionEntries++;
      return COMP_DISP_ERR_OK;
    }
    idx++;
    actionEntry = &actionName2Actions[idx];
  }
  return COMP_DISP_ERR_ACTION_NAME_NOT_FOUND;
}

// ================================= runAction ====================================

static uint8_t runAction(compMsgDispatcher_t *self, uint8_t *answerType) {
  int result;
  msgParts_t *received;
  actionName2Action_t *actionEntry;
  int idx;
  uint8_t actionMode;
  dataView_t *dataView;

  received = &self->received;
  if (received->partsFlags & COMP_DISP_U8_CMD_KEY) {
    dataView = self->compMsgDataView->dataView;
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
          return COMP_DISP_ERR_OK;
        }
        idx++;
        actionEntry = &actionName2Actions[idx];
      }
      return COMP_DISP_ERR_ACTION_NAME_NOT_FOUND;
      break;
    }
  } else {
//ets_printf("§runAction u16!%c%c!%c!§\n", (received->u16CmdKey>>8)&0xFF, received->u16CmdKey&0xFF, *answerType);
    dataView = self->compMsgDataView->dataView;
    switch (self->actionMode) {
    case MODULE_INFO_AP_LIST_CALL_BACK:
      idx = 0;
      actionEntry = &actionName2Actions[idx];
      while (actionEntry->actionName != NULL) { 
        if (self->actionMode == actionEntry->mode) {
//ets_printf("§runAction2 G!%d!%c!§\n", self->actionMode, *answerType);
          result = actionEntry->action(self);
          checkErrOK(result);
          return COMP_DISP_ERR_OK;
        }
        idx++;
        actionEntry = &actionName2Actions[idx];
      }
      return COMP_DISP_ERR_ACTION_NAME_NOT_FOUND;
      break;
    }
  }
  return COMP_DISP_ERR_OK;
}

// ================================= fillMsgValue ====================================

static uint8_t fillMsgValue(compMsgDispatcher_t *self, uint8_t *callbackName, uint8_t answerType, uint8_t fieldTypeId) {
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
      return COMP_DISP_ERR_OK;
    }
    idx++;
    actionEntry = &actionName2Actions[idx];
  }
  return COMP_DISP_ERR_ACTION_NAME_NOT_FOUND;
}

// ================================= compMsgActionInit ====================================

uint8_t compMsgActionInit(compMsgDispatcher_t *self) {
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
  compMsgDataView_t *dataView;
  uint8_t *myStr;
  uint8_t fieldTypeId;
  uint8_t u8CmdKey;
  uint16_t u16CmdKey;

  self->setActionEntry = &setActionEntry;
  self->runAction = &runAction;
  self->getActionMode = &getActionMode;
  self->fillMsgValue = &fillMsgValue;

  buffer = buf;
  compMsgActionEntries.numActionEntries = 0;
  compMsgActionEntries.maxActionEntries = 10;
  compMsgActionEntries.actionEntries = (actionName2Action_t **)os_zalloc(compMsgActionEntries.maxActionEntries * sizeof(actionName2Action_t *));
  checkAllocOK(compMsgActionEntries.actionEntries);

  compMsgActions.numActions = 0;
  compMsgActions.maxActions = 10;
  compMsgActions.actions = (actionName2Action_t **)os_zalloc(compMsgActions.maxActions * sizeof(actionName2Action_t  **));
  checkAllocOK(compMsgActions.actions);

  dataView = self->compMsgDataView;
  os_sprintf(fileName, "MsgActions.txt");
  result = self->openFile(self, fileName, "r");
  checkErrOK(result);
#undef checkErrOK
#define checkErrOK(result) if(result != COMP_DISP_ERR_OK) { self->closeFile(self); return result; }
  result = self->readLine(self, &buffer, &lgth);
  checkErrOK(result);
  buffer[lgth] = 0;
  if ((lgth < 4) || (buffer[0] != '#')) {
     return COMP_DISP_ERR_BAD_FILE_CONTENTS;
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
      return COMP_DISP_ERR_TOO_FEW_FILE_LINES;
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
      checkErrOK(COMP_DISP_ERR_BAD_FIELD_TYPE);
    }
    result = setActionEntry(self, actionName, actionMode, u8CmdKey, u16CmdKey);
    checkErrOK(result);
    idx++;
  }
#undef checkErrOK
#define checkErrOK(result) if(result != COMP_DISP_ERR_OK) return result
  result = self->closeFile(self);
  checkErrOK(result);
  return COMP_DISP_ERR_OK;
}

