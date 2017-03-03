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
 * File:   compMsgSocket.c
 * Author: Arnulf P. Wiedemann
 *
 * Created on December 17th, 2016
 */

#include "osapi.h"
#include "c_types.h"
#include "mem.h"
#include "flash_fs.h"

#include "lwip/err.h"
#include "lwip/ip_addr.h"
#include "espconn.h"
#include "lwip/dns.h"

#include "c_string.h"
#include "c_stdlib.h"
#include "c_stdio.h"
#include "platform.h"
#include "compMsgDispatcher.h"

// ================================= checkConnectionStatus ====================================

static uint8_t checkConnectionStatus(compMsgTimerSlot_t *compMsgTimerSlot) {
  uint8_t result;
  compMsgDispatcher_t *self;
  struct ip_info pTempIp;
  uint8_t timerId;
  uint8_t status;
  char temp[64];
  compMsgTimerSlot_t *tmr;

  timerId = compMsgTimerSlot->timerId;
  self = compMsgTimerSlot->compMsgDispatcher;
  COMP_MSG_DBG(self, "S", 2, "checkConnectionStatus timerId: %d\n", timerId);
  tmr = &self->compMsgTimer->compMsgTimers[timerId];
  COMP_MSG_DBG(self, "S", 2, "checkConnectionStatus: tmr: %p compMsgTimerSlot: %p\n", tmr, compMsgTimerSlot);
  COMP_MSG_DBG(self, "S", 2, "checkConnectionStatus: timerId: %d self: %p\n", timerId, self);
  status = wifi_station_get_connect_status();
  COMP_MSG_DBG(self, "S", 1, "checkConnectionStatus:wifi is in mode: %d status: %d ap_id: %d hostname: %s!\n", wifi_get_opmode(), status, wifi_station_get_current_ap_id(), wifi_station_get_hostname());
  switch (status) {
  case STATION_IDLE:
    COMP_MSG_DBG(self, "S", 2, "STATION_IDLE\n");
    break;
  case STATION_CONNECTING:
    COMP_MSG_DBG(self, "S", 2, "STATION_CONNECTING\n");
    break;
  case STATION_WRONG_PASSWORD:
    COMP_MSG_DBG(self, "S", 2, "STATION_WRONG_PASSWORD\n");
    tmr->mode |= TIMER_IDLE_FLAG;
    ets_timer_disarm(&tmr->timer);
    self->compMsgWifiData->webSocketSendConnectError(self, status);
    return COMP_MSG_ERR_CONNECT_STATION_WRONG_PASSWD;
    break;
  case STATION_NO_AP_FOUND:
    COMP_MSG_DBG(self, "S", 2, "STATION_NO_AP_FOUND\n");
    tmr->mode |= TIMER_IDLE_FLAG;
    ets_timer_disarm(&tmr->timer);
    self->compMsgWifiData->webSocketSendConnectError(self, status);
    return COMP_MSG_ERR_CONNECT_STATION_NO_AP_FOUND;
    break;
  case STATION_CONNECT_FAIL:
    COMP_MSG_DBG(self, "S", 2, "STATION_CONNECT_FAIL\n");
    tmr->mode |= TIMER_IDLE_FLAG;
    ets_timer_disarm(&tmr->timer);
    self->compMsgWifiData->webSocketSendConnectError(self, status);
    return COMP_MSG_ERR_CONNECT_STATION_CONNECT_FAILED;
    break;
  case STATION_GOT_IP:
    COMP_MSG_DBG(self, "S", 2, "STATION_GOT_IP\n");
    break;
  } 
  wifi_get_ip_info(tmr->connectionMode, &pTempIp);
  if(pTempIp.ip.addr == 0){
    COMP_MSG_DBG(self, "S", 2, "ip: nil\n");
    return COMP_MSG_ERR_CONNECT_STATION_CONNECTING;
  }
  tmr->mode |= TIMER_IDLE_FLAG;
  tmr->ip_addr = pTempIp.ip.addr;
  ets_timer_disarm(&tmr->timer);
  c_sprintf(temp, "%d.%d.%d.%d", IP2STR(&pTempIp.ip));
  COMP_MSG_DBG(self, "S", 1, "checkConnectionStatus: IP: %s\n", temp);
int numericValue;
uint8_t *stringValue;
  COMP_MSG_DBG(self, "S", 2, "getMACAddr\n");
result = self->compMsgModuleData->getMACAddr(self, &numericValue, &stringValue);
  return COMP_MSG_ERR_OK;
}

// ================================= startConnectionTimer ====================================

static uint8_t startConnectionTimer(compMsgDispatcher_t *self, uint8_t timerId, startConnection_t fcn) {
  int repeat;
  int interval;
  int mode;
  compMsgTimerSlot_t *compMsgTimerSlot;
  int isMstimer;

  repeat = 1;
  interval = 1000;
  mode = TIMER_MODE_AUTO;
  isMstimer = 1;
  compMsgTimerSlot = &self->compMsgTimer->compMsgTimers[timerId];
  compMsgTimerSlot->timerId = timerId;
  compMsgTimerSlot->compMsgDispatcher = self;
  compMsgTimerSlot_t *tmr = &self->compMsgTimer->compMsgTimers[timerId];
  if (!(tmr->mode & TIMER_IDLE_FLAG) && (tmr->mode != TIMER_MODE_OFF)) {
    ets_timer_disarm(&tmr->timer);
  }
  // this is only preparing
  COMP_MSG_DBG(self, "S", 2, "webSocketRunAPMode timer_setfcn: %p\n", compMsgTimerSlot);
  ets_timer_setfn(&tmr->timer, fcn, (void*)compMsgTimerSlot);
  tmr->mode = mode | TIMER_IDLE_FLAG;
  // here is the start
  tmr->interval = interval;
  tmr->mode &= ~TIMER_IDLE_FLAG;
  COMP_MSG_DBG(self, "S", 2, "webSocketRunAPMode timer_arm_new\n");
  ets_timer_arm_new(&tmr->timer, interval, repeat, isMstimer);
  return COMP_MSG_ERR_OK;
}

// ================================= compMsgSocketInit ====================================

static uint8_t compMsgSocketInit(compMsgSocket_t *compMsgSocket) {
  compMsgSocket->startConnectionTimer = &startConnectionTimer;
  compMsgSocket->checkConnectionStatus = &checkConnectionStatus;
  return COMP_MSG_ERR_OK;
}

// ================================= newCompMsgSocket ====================================

compMsgSocket_t *newCompMsgSocket() {
  compMsgSocket_t *compMsgSocket = os_zalloc(sizeof(compMsgSocket_t));
  if (compMsgSocket == NULL) {
    return NULL;
  }
  compMsgSocketInit(compMsgSocket);
  return compMsgSocket;
}
