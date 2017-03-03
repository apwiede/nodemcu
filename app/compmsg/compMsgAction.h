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
 * File:   compMsgAction.h
 * Author: Arnulf P. Wiedemann
 *
 * Created on Descmber 17, 2016
 */

#ifndef COMP_MSG_ACTION_H
#define	COMP_MSG_ACTION_H

#include "c_types.h"
#ifdef	__cplusplus
extern "C" {
#endif

typedef struct compMsgDispatcher compMsgDispatcher_t;

typedef uint8_t (* startLightSleepWakeupMode_t)(compMsgDispatcher_t *self);
typedef uint8_t (* startTestInterrupt_t)(compMsgDispatcher_t *self);
typedef uint8_t (* action_t)(compMsgDispatcher_t *self);
typedef uint8_t (* setActionEntry_t)(compMsgDispatcher_t *self, uint8_t *actionName, uint8_t mode, uint8_t u8CmdKey, uint16_t u16CmdKey);
typedef uint8_t (* runAction_t)(compMsgDispatcher_t *self, uint8_t *answerType);
typedef uint8_t (* getActionMode_t)(compMsgDispatcher_t *self, uint8_t *actionName, uint8_t *actionMode);
typedef uint8_t (* getActionCallback_t)(compMsgDispatcher_t *self, uint8_t *actionName, action_t *callback);
typedef uint8_t (* getActionCallbackName_t)(compMsgDispatcher_t *self, action_t callback, uint8_t **actionName);

typedef struct compMsgAction {

  startTestInterrupt_t startTestInterrupt;
  startLightSleepWakeupMode_t startLightSleepWakeupMode;
  setActionEntry_t setActionEntry;
  runAction_t runAction;
  getActionCallback_t getActionCallback;
  getActionCallbackName_t getActionCallbackName;
  getActionMode_t getActionMode;
} compMsgAction_t;

compMsgAction_t *newCompMsgAction();
uint8_t compMsgActionInit(compMsgDispatcher_t *self);

#ifdef  __cplusplus
}
#endif

#endif  /* COMP_MSG_ACTION_H */

