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
 * File:   compMsgTimer.c
 * Author: Arnulf P. Wiedemann
 *
 * Created on December 16th, 2016
 */

#include "osapi.h"
#include "c_types.h"
#include "mem.h"
#include "flash_fs.h"

#include "c_string.h"
#include "c_stdio.h"
#include "c_stdlib.h"
#include "compMsgDispatcher.h"

// ================================= initTimers ====================================

static uint8_t initTimers(compMsgDispatcher_t *self) {
  int result;

  for(int i = 0; i < NUM_TMR; i++) {
    self->compMsgTimer->compMsgTimers[i].compMsgDispatcher = self;
    self->compMsgTimer->compMsgTimers[i].mode = TIMER_MODE_OFF;
    self->compMsgTimer->compMsgTimers[i].timerId = i;
    ets_timer_disarm(&self->compMsgTimer->compMsgTimers[i].timer);
  }
  return COMP_MSG_ERR_OK;
}

// ================================= compMsgTimerInit ====================================

uint8_t compMsgTimerInit(compMsgDispatcher_t *self) {
  uint8_t result;

  self->compMsgTimer->initTimers = &initTimers;
  result = self->compMsgTimer->initTimers(self);
  checkErrOK(result);
  return COMP_MSG_ERR_OK;
}

// ================================= newCompMsgTimer ====================================

compMsgTimer_t *newCompMsgTimer() {
  compMsgTimer_t *compMsgTimer = os_zalloc(sizeof(compMsgTimer_t));
  if (compMsgTimer == NULL) {
    return NULL;
  }
  return compMsgTimer;
}
