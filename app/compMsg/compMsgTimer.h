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
 * File:   compMsgTimer.h
 * Author: Arnulf P. Wiedemann
 *
 * Created on Descmber 16, 2016
 */

#ifndef COMP_MSG_TIMER_H
#define	COMP_MSG_TIMER_H

#include "c_types.h"
#ifdef	__cplusplus
extern "C" {
#endif

#define TIMER_MODE_OFF 3
#define TIMER_MODE_SINGLE 0
#define TIMER_MODE_SEMI 2
#define TIMER_MODE_AUTO 1
#define TIMER_IDLE_FLAG (1<<7) 

typedef struct compMsgDispatcher compMsgDispatcher_t;

typedef struct compMsgTimerSlot {
  os_timer_t timer;
  compMsgDispatcher_t *compMsgDispatcher;
  uint32_t interval;
  uint8_t mode;
  uint8_t connectionMode;
  uint8_t timerId;
  uint32_t ip_addr;
} compMsgTimerSlot_t;

typedef uint8_t (* initTimers_t)(compMsgDispatcher_t *self);

typedef struct compMsgTimer {
  compMsgTimerSlot_t compMsgTimers[NUM_TMR];
  os_timer_t apTimer;

  initTimers_t initTimers;
} compMsgTimer_t;

compMsgTimer_t *newCompMsgTimer();
uint8_t compMsgTimerInit(compMsgDispatcher_t *self);

#ifdef  __cplusplus
}
#endif

#endif  /* COMP_MSG_TIMER_H */


