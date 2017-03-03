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
#include "c_string.h"
#include "c_stdlib.h"
#include "c_stdio.h"
#include "platform.h"
#include "compMsgDispatcher.h"

#define FPM_SLEEP_MAX_TIME  0xFFFFFFF

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
// needed for espressif defined callbacks like lightSleepWakeupCallback!!
static compMsgDispatcher_t *compMsgDispatcher;

void wifi_fpm_set_wakeup_cb(void (*fcn)());

// ================================= runClientMode ====================================

static uint8_t runClientMode(compMsgDispatcher_t *self, uint8_t mode) {
  int result;

  self->compMsgSocket->netSocketRunClientMode(self);
  return COMP_MSG_ERR_OK;
}

// ================================= runAPMode ====================================

static uint8_t runAPMode(compMsgDispatcher_t *self) {
  int result;

  self->compMsgSocket->webSocketRunAPMode(self);
  return COMP_MSG_ERR_OK;
}

sint8 gresult2 = -13;

// ================================= lightSleepWakeupCallback ====================================

static void lightSleepWakeupCallback(void) {
 compMsgDispatcher_t *self;

ets_printf(">>>>lSWC gresult2: %d\n", gresult2);
 self = compMsgDispatcher;
 COMP_MSG_DBG(self, "Y", 0, "lightSleepWakeupCallback\n");
 wifi_fpm_close();      // disable force sleep function
// wifi_set_opmode(STATION_MODE);         // set station mode
// wifi_station_connect();            // connect 12to AP
}

// ================================= startLightSleepWakeupMode ====================================

static uint8_t startLightSleepWakeupMode(compMsgDispatcher_t *self) {
  int result;

  COMP_MSG_DBG(self, "Y", 0, "startLightSleepWakeupMode1\n");
  wifi_station_disconnect();
  COMP_MSG_DBG(self, "Y", 0, "startLightSleepWakeupMode2\n");

  wifi_set_opmode(NULL_MODE); // set WiFi mode to null mode.
//  wifi_set_opmode(STATION_MODE); // set WiFi mode to null mode.
  COMP_MSG_DBG(self, "Y", 0, "startLightSleepWakeupMode3\n");

  wifi_fpm_set_sleep_type(LIGHT_SLEEP_T);    // light sleep
  COMP_MSG_DBG(self, "Y", 0, "startLightSleepWakeupMode4\n");

  wifi_fpm_open();               // enable force sleep
  COMP_MSG_DBG(self, "Y", 0, "startLightSleepWakeupMode5\n");


// next 2 lines work with PIN D7 of nodemcu dev kit
//  PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U,3);
//  gpio_pin_wakeup_enable(13, GPIO_PIN_INTR_LOLEVEL);
// PIN D7

// next 2 lines work with PIN U0RX (GPIO3) of nodemcu dev kit
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0RXD_U,FUNC_GPIO3);
  gpio_pin_wakeup_enable(GPIO_ID_PIN(3), GPIO_PIN_INTR_LOLEVEL);
// PIN U0RX

  COMP_MSG_DBG(self, "Y", 0, "startLightSleepWakeupMode6\n");

  wifi_fpm_set_wakeup_cb(lightSleepWakeupCallback);   // Set wakeup callback
  COMP_MSG_DBG(self, "Y", 0, "startLightSleepWakeupMode7\n");

  gresult2 = wifi_fpm_do_sleep(FPM_SLEEP_MAX_TIME);
//  gresult2 = wifi_fpm_do_sleep(5000*1000);
ets_printf("gresult2: %d\n", gresult2);
  return COMP_MSG_ERR_OK;
}

// ================================= tstInterruptCallback ====================================

static void tstInterruptCallback(void *arg) {
  compMsgDispatcher_t *self;

  self = (compMsgDispatcher_t *)arg;
ets_printf(">>>>tIC self: %p\n", self);
 COMP_MSG_DBG(self, "Y", 0, "tstInterruptCallback\n");
  ETS_GPIO_INTR_DISABLE();
}

// ================================= startTestInterrupt ====================================

static uint8_t startTestInterrupt(compMsgDispatcher_t *self) {
  int result;
  COMP_MSG_DBG(self, "Y", 0, "startTestInterrupt1\n");

  PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U,FUNC_GPIO12);
  GPIO_DIS_OUTPUT(GPIO_ID_PIN(12));
  ETS_GPIO_INTR_DISABLE();
  ETS_GPIO_INTR_ATTACH(tstInterruptCallback, self);
  gpio_pin_intr_state_set(GPIO_ID_PIN(12),GPIO_PIN_INTR_LOLEVEL);
  ETS_GPIO_INTR_ENABLE();
  COMP_MSG_DBG(self, "Y", 0, "startTestInterrupt2\n");

  return COMP_MSG_ERR_OK;
}

// ================================= runLightSleepWakeupMode ====================================

static uint8_t runLightSleepWakeupMode(compMsgDispatcher_t *self) {
  int result;
  return COMP_MSG_ERR_OK;
}

// ================================= runLightSleepNoWakeupMode ====================================

static uint8_t runLightSleepNoWakeupMode(compMsgDispatcher_t *self) {
  int result;
  return COMP_MSG_ERR_OK;
}

// ================================= runWpsMode ====================================

static uint8_t runWpsMode(compMsgDispatcher_t *self) {
  int result;
  return COMP_MSG_ERR_OK;
}

// ================================= runModulTestMode ====================================

static uint8_t runModulTestMode(compMsgDispatcher_t *self) {
  int result;
  return COMP_MSG_ERR_OK;
}

// ================================= runDeletePasswdCMode ====================================

static uint8_t runDeletePasswdCMode(compMsgDispatcher_t *self) {
  int result;

  COMP_MSG_DBG(self, "A", 1, "runDeletePasswdC");
  return COMP_MSG_ERR_OK;
}

// ================================= runAPConnect ====================================

static uint8_t runAPConnect(compMsgDispatcher_t *self) {
  uint8_t result;

  result = self->compMsgWifiData->connectToAP(self);
  return result;
}

// ================================= runRestoreUserData ====================================

static uint8_t runRestoreUserData(compMsgDispatcher_t *self) {
  uint8_t result;

  result = self->compMsgModuleData->restoreUserData(self);
  return result;
}

// ================================= getAPList ====================================

static uint8_t getAPList(compMsgDispatcher_t *self) {
  uint8_t result;

  COMP_MSG_DBG(self, "A", 1, "getAPList compMsgWifiData: %p\n", self->compMsgWifiData);
  result = self->compMsgWifiData->getBssScanInfo(self);
  return result;
}

// ================================= getWifiSrcId ====================================

static uint8_t getWifiSrcId(compMsgDispatcher_t *self) {
  uint8_t result;

  result = self->compMsgWifiData->getWifiRemotePort(self);
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
  { "getAPList",                 (action_t)(&getAPList),                 0, 0, MODULE_INFO_AP_LIST_CALL_BACK },
  { "runAPConnect",              (action_t)(&runAPConnect),              0, 0, MODULE_INFO_AP_LIST_CALL_BACK },
  { "runRestoreUserData",        (action_t)(&runRestoreUserData),        0, 0, 0 },
  { "getWifiSrcId",              (action_t)(&getWifiSrcId),              0x4141, 0, 8 },
  { NULL,                        NULL,                                   0, 0, 0 },
};

// ================================= getActionMode ====================================

/**
 * \brief get the action mode for an action
 * 
 * \param self The dispatcher struct
 * \param actionName The field actionName
 * \param actionMode The out value for the actionName
 *
 */
static uint8_t getActionMode(compMsgDispatcher_t *self, uint8_t *actionName, uint8_t *actionMode) {
  int result;
  actionName2Action_t *actionEntry;
  int idx;

  idx = 0;
  actionEntry = &actionName2Actions[idx];
  while (actionEntry->actionName != NULL) { 
    if (c_strcmp(actionEntry->actionName, actionName) == 0) {
      *actionMode = actionEntry->mode;
      COMP_MSG_DBG(self, "A", 1, "actionMode: %d", *actionMode);
      return COMP_MSG_ERR_OK;
    }
    idx++;
    actionEntry = &actionName2Actions[idx];
  }
  COMP_MSG_DBG(self, "A", 1, "getActionMode: %s", actionName);
  return COMP_MSG_ERR_ACTION_NAME_NOT_FOUND;
}

// ================================= getActionCallback ====================================

static uint8_t getActionCallback(compMsgDispatcher_t *self, uint8_t *actionName, action_t *callback) {
  int result;
  actionName2Action_t *actionEntry;
  int idx;

  idx = 0;
  actionEntry = &actionName2Actions[idx];
  while (actionEntry->actionName != NULL) { 
    if (c_strcmp(actionEntry->actionName, actionName) == 0) {
      *callback = actionEntry->action;
      return COMP_MSG_ERR_OK;
    }
    idx++;
    actionEntry = &actionName2Actions[idx];
  }
  COMP_MSG_DBG(self, "A", 1, "getActionCallback: %s\n", actionName);
  return COMP_MSG_ERR_ACTION_NAME_NOT_FOUND;
}

// ================================= getActionCallbackName ====================================

static uint8_t getActionCallbackName(compMsgDispatcher_t *self, action_t callback, uint8_t **actionName) {
  int result;
  actionName2Action_t *actionEntry;
  int idx;

  idx = 0;
  actionEntry = &actionName2Actions[idx];
  while (actionEntry->actionName != NULL) { 
    if (actionEntry->action == callback) {
      *actionName = actionEntry->actionName;
      return COMP_MSG_ERR_OK;
    }
    idx++;
    actionEntry = &actionName2Actions[idx];
  }
  COMP_MSG_DBG(self, "A", 1, "getActionCallbackName: %s", actionName);
  return COMP_MSG_ERR_ACTION_NAME_NOT_FOUND;
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
        return COMP_MSG_ERR_DUPLICATE_ENTRY;
      }
      actionEntry->mode = mode;
      actionEntry->u8CmdKey = u8CmdKey;
      actionEntry->u16CmdKey = u16CmdKey;
      compMsgActionEntries.numActionEntries++;
      return COMP_MSG_ERR_OK;
    }
    idx++;
    actionEntry = &actionName2Actions[idx];
  }
  COMP_MSG_DBG(self, "A", 1, "setActionEntry: %s", actionName);
  return COMP_MSG_ERR_ACTION_NAME_NOT_FOUND;
}

// ================================= runAction ====================================

/**
 * \brief Run an action forced by a received message
 * 
 * \param self The dispatcher struct
 * \param answerType The type for the answer
 */
static uint8_t runAction(compMsgDispatcher_t *self, uint8_t *answerType) {
  int result;
  msgParts_t *received;
  actionName2Action_t *actionEntry;
  int idx;
  uint8_t actionMode;
  dataView_t *dataView;

  received = &self->compMsgData->received;
  if (self->compMsgData->compMsgDataView == NULL) {
    self->compMsgData->compMsgDataView = newCompMsgDataView(self->compMsgData->received.buf, self->compMsgData->receivedLgth);
    checkAllocOK(self->compMsgData->compMsgDataView);
  }
  dataView = self->compMsgData->compMsgDataView->dataView;
  if (received->u16CmdKey == 0x4244) { // "BD"
    // FIXME need to get the real offset here instead of 7!!
    result = dataView->getUint8(dataView, 7, &actionMode);
    checkErrOK(result);
    idx = 0;
    actionEntry = &actionName2Actions[idx];
    while (actionEntry->actionName != NULL) { 
      COMP_MSG_DBG(self, "A", 2, "runActionBu8!%s!%c!%c!%c!", actionEntry->actionName, (received->u16CmdKey>>8)&0xFF, received->u16CmdKey&0xFF, actionMode);
      if ((actionEntry->u16CmdKey == received->u16CmdKey) && (actionMode == actionEntry->mode)) {
        COMP_MSG_DBG(self, "A", 1, "runAction0!%s!%d!", actionEntry->actionName, actionEntry->mode);
        result = actionEntry->action(self);
        checkErrOK(result);
        os_free(self->compMsgData->compMsgDataView->dataView);
        os_free(self->compMsgData->compMsgDataView);
        self->compMsgData->compMsgDataView = NULL;
        return COMP_MSG_ERR_OK;
      }
      idx++;
      actionEntry = &actionName2Actions[idx];
    }
    os_free(self->compMsgData->compMsgDataView->dataView);
    os_free(self->compMsgData->compMsgDataView);
    self->compMsgData->compMsgDataView = NULL;
    COMP_MSG_DBG(self, "A", 1, "runAction1:");
    return COMP_MSG_ERR_ACTION_NAME_NOT_FOUND;
  } else {
    COMP_MSG_DBG(self, "A", 2, "runAction u16!%c%c!%c!\n", (received->u16CmdKey>>8)&0xFF, received->u16CmdKey&0xFF, *answerType);
    switch (self->actionMode) {
    case 8:
    case MODULE_INFO_AP_LIST_CALL_BACK:
      idx = 0;
      actionEntry = &actionName2Actions[idx];
      while (actionEntry->actionName != NULL) { 
        if (self->actionMode == actionEntry->mode) {
          COMP_MSG_DBG(self, "A", 2, "runAction2 G!%d!%c!\n", self->actionMode, *answerType);
          result = actionEntry->action(self);
          checkErrOK(result);
          os_free(self->compMsgData->compMsgDataView->dataView);
          os_free(self->compMsgData->compMsgDataView);
          self->compMsgData->compMsgDataView = NULL;
          return COMP_MSG_ERR_OK;
        }
        idx++;
        actionEntry = &actionName2Actions[idx];
      }
      os_free(self->compMsgData->compMsgDataView->dataView);
      os_free(self->compMsgData->compMsgDataView);
      self->compMsgData->compMsgDataView = NULL;
      COMP_MSG_DBG(self, "A", 1, "runAction3:");
      return COMP_MSG_ERR_ACTION_NAME_NOT_FOUND;
      break;
    }
  }
  os_free(self->compMsgData->compMsgDataView->dataView);
  os_free(self->compMsgData->compMsgDataView);
  self->compMsgData->compMsgDataView = NULL;
  COMP_MSG_DBG(self, "A", 1, "runAction4:");
  return COMP_MSG_ERR_ACTION_NAME_NOT_FOUND;
}

// ================================= compMsgActionInit ====================================

uint8_t compMsgActionInit(compMsgDispatcher_t *self) {
  uint8_t result;

  self->compMsgAction->startLightSleepWakeupMode = &startLightSleepWakeupMode;
  self->compMsgAction->startTestInterrupt = &startTestInterrupt;
  self->compMsgAction->setActionEntry = &setActionEntry;
  self->compMsgAction->runAction = &runAction;
  self->compMsgAction->getActionCallback = &getActionCallback;
  self->compMsgAction->getActionCallbackName = &getActionCallbackName;
  self->compMsgAction->getActionMode = &getActionMode;

  compMsgActionEntries.numActionEntries = 0;
  compMsgActionEntries.maxActionEntries = 10;
  compMsgActionEntries.actionEntries = (actionName2Action_t **)os_zalloc(compMsgActionEntries.maxActionEntries * sizeof(actionName2Action_t *));
  checkAllocOK(compMsgActionEntries.actionEntries);

  compMsgActions.numActions = 0;
  compMsgActions.maxActions = 10;
  compMsgActions.actions = (actionName2Action_t **)os_zalloc(compMsgActions.maxActions * sizeof(actionName2Action_t  **));
  checkAllocOK(compMsgActions.actions);

  compMsgDispatcher = self;
  result = self->compMsgMsgDesc->readActions(self, COMP_MSG_ACTIONS_FILE_NAME);
  return result;
}

// ================================= newCompMsgAction ====================================

compMsgAction_t *newCompMsgAction() {
  compMsgAction_t *compMsgAction = os_zalloc(sizeof(compMsgAction_t));
  if (compMsgAction == NULL) {
    return NULL;
  }

  return compMsgAction;
}
