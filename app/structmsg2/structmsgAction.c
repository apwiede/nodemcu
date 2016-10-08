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

#include "c_string.h"
#include "c_stdlib.h"
#include "c_stdio.h"
#include "platform.h"
#include "structmsg2.h"

typedef uint8_t (* action_t)(structmsgDispatcher_t *self, uint8_t mode);

typedef struct actionName2Action {
  uint8_t *actionName;
  action_t *action;
} actionName2Action_t;

typedef struct structmsgActionEntries {
  actionName2Action_t *actionEntries;
  uint8_t numActionEntries;
  uint8_t maxActionEntries;
} structmsgActionEntries_t;

typedef struct structmsgAction {
  actionName2Action_t *actionEntry;
  uint8_t mode;
} structmsgAction_t;

typedef struct structmsgActions {
  structmsgAction_t *actions;
  uint8_t numActions;
  uint8_t maxActions;
} structmsgActions_t;

static structmsgActionEntries_t structmsgActionEntries = { NULL, 0, 0 };
static structmsgActions_t structmsgActions = { NULL, 0, 0 };

// ================================= action ====================================

static uint8_t action(structmsgDispatcher_t *self, uint8_t mode) {
  int result;
  return STRUCT_DISP_ERR_OK;
}

// ================================= addActionEntry ====================================

static uint8_t addActionEntry(structmsgDispatcher_t *self, uint8_t *actionName, action_t *action) {
  int result;

  if (structmsgActionEntries.numActionEntries >= structmsgActionEntries.maxActionEntries) {
    structmsgActionEntries.maxActionEntries += 5;
    structmsgActionEntries.actionEntries = (actionName2Action_t *)os_realloc(structmsgActionEntries.actionEntries, (structmsgActionEntries.maxActionEntries * sizeof(actionName2Action_t)));
    checkAllocOK(structmsgActionEntries.actionEntries);
  }
// FIXME need to find the actionEntry here !!
  structmsgActionEntries.actionEntries[structmsgActionEntries.numActionEntries].actionName = actionName;
  structmsgActionEntries.actionEntries[structmsgActionEntries.numActionEntries].action = action;
  structmsgActionEntries.numActionEntries++;
  return STRUCT_DISP_ERR_OK;
}

// ================================= addAction ====================================

static uint8_t addAction(structmsgDispatcher_t *self, uint8_t *actionName, uint8_t mode) {
  int result;
  actionName2Action_t *actionEntry;

  actionEntry = NULL;
  if (structmsgActions.numActions >= structmsgActions.maxActions) {
    structmsgActions.maxActions += 5;
    structmsgActions.actions = (structmsgAction_t *)os_realloc(structmsgActions.actions, (structmsgActions.maxActions * sizeof(structmsgAction_t)));
    checkAllocOK(structmsgActions.actions);
  }
// FIXME need to find the actionEntry here !!
  structmsgActions.actions[structmsgActions.numActions].actionEntry = actionEntry;
  structmsgActions.actions[structmsgActions.numActions].mode = mode;
  structmsgActions.numActions++;
  return STRUCT_DISP_ERR_OK;
}

// ================================= runAction ====================================

static uint8_t runAction(structmsgDispatcher_t *self, uint8_t mode) {
  int result;
  return STRUCT_DISP_ERR_OK;
}

// ================================= structmsgSendReceiveInit ====================================

uint8_t structmsgActionInit(structmsgDispatcher_t *self) {
  uint8_t result;

  structmsgActionEntries.numActionEntries = 0;
  structmsgActionEntries.maxActionEntries = 10;
  structmsgActionEntries.actionEntries = (actionName2Action_t *)os_zalloc(structmsgActionEntries.maxActionEntries * sizeof(actionName2Action_t));
  checkAllocOK(structmsgActionEntries.actionEntries);

  structmsgActions.numActions = 0;
  structmsgActions.maxActions = 10;
  structmsgActions.actions = (structmsgAction_t *)os_zalloc(structmsgActions.maxActions * sizeof(structmsgAction_t));
  checkAllocOK(structmsgActions.actions);

  self->addAction = &addAction;
  self->runAction = &runAction;
  return STRUCT_DISP_ERR_OK;
}

