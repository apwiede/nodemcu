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
 * File:   structmsgWebsocket.c
 * Author: Arnulf P. Wiedemann
 *
 * Created on October 10th, 2016
 */

#include "osapi.h"
#include "c_types.h"
#include "mem.h"
#include "flash_fs.h"

#include "lwip/err.h"
#include "lwip/app/espconn.h"
#include "lwip/ip_addr.h"
#include "lwip/dns.h"

#include "c_string.h"
#include "c_stdlib.h"
#include "c_stdio.h"
#include "platform.h"
#include "structmsg2.h"
#include "structmsgModuleData.h"

#define TCP ESPCONN_TCP

typedef struct socketInfo {
  struct espconn *pesp_conn;
} socketInfo_t;

#define MAX_SOCKET 5
static int socket_num = 0;
static socketInfo_t *socket[MAX_SOCKET] = { NULL, NULL, NULL, NULL, NULL };

// ================================= webSocketRunClientMode ====================================

static uint8_t websocketRunClientMode(structmsgDispatcher_t *self, uint8_t mode) {
  int result;
  return STRUCT_DISP_ERR_OK;
}

static uint16_t tcp_server_timeover = 30;

// ================================= serverDisconnected  ====================================

static void serverDisconnected(void *arg) {
  struct espconn *pesp_conn;

  pesp_conn = (struct espconn *)arg;
ets_printf("serverDisconnected: arg: %p\n", arg);

}

// ================================= serverReconnected  ====================================

static void serverReconnected(void *arg, int8_t err) {
  struct espconn *pesp_conn;

  pesp_conn = (struct espconn *)arg;
ets_printf("serverReconnected: arg: %p\n", arg);

}

// ================================= socketReceived  ====================================

static void socketReceived(void *arg, char *pdata, unsigned short len) {
  struct espconn *pesp_conn;

  pesp_conn = (struct espconn *)arg;
ets_printf("socketReceived: arg: %p pdata: %s len: %d\n", arg, pdata, len);
  char temp[20] = {0};
  c_sprintf(temp, IPSTR, IP2STR( &(pesp_conn->proto.tcp->remote_ip) ) );
  ets_printf("remote ");
  ets_printf(temp);
  ets_printf(":");
  ets_printf("%d",pesp_conn->proto.tcp->remote_port);
  ets_printf(" received.\n");

}

// ================================= socketSent  ====================================

static void socketSent(void *arg) {
  struct espconn *pesp_conn;

  pesp_conn = (struct espconn *)arg;
ets_printf("socketSent: arg: %p\n", arg);
  pesp_conn = arg;

}

// ================================= serverConnected  ====================================

static void serverConnected(void *arg) {
  struct espconn *pesp_conn;
  int result;
  int i;

ets_printf("serverConnected: arg: %p\n", arg);
  pesp_conn = arg;
  for(i = 0; i < MAX_SOCKET; i++) {
    if (socket[i] == NULL) { // found empty slot
      break;
    }
  }
  if(i>=MAX_SOCKET) {// can't create more socket
    pesp_conn->reverse = NULL;    // not accept this conn
    if(pesp_conn->proto.tcp->remote_port || pesp_conn->proto.tcp->local_port) {
      espconn_disconnect(pesp_conn);
    }
//    checkErrOK(gL, WEBSOCKET_ERR_MAX_SOCKET_REACHED, "websocket_server_connected");
    return;
  }
ets_printf("registstart\n");
  result = espconn_regist_recvcb(pesp_conn, socketReceived);
  if (result != STRUCT_DISP_ERR_OK) {
ets_printf("regist socketReceived err: %d\n", result);
  }
  result = espconn_regist_sentcb(pesp_conn, socketSent);
  if (result != STRUCT_DISP_ERR_OK) {
ets_printf("regist socketSent err: %d\n", result);
  }
  result = espconn_regist_disconcb(pesp_conn, serverDisconnected);
  if (result != STRUCT_DISP_ERR_OK) {
ets_printf("regist serverDisconnected err: %d\n", result);
  }
  result = espconn_regist_reconcb(pesp_conn, serverReconnected);
  if (result != STRUCT_DISP_ERR_OK) {
ets_printf("regist serverReconnected err: %d\n", result);
  }

}

#ifdef NOTDEF
ets_printf("stnum!%d!\n", wifi_softap_get_station_num());
struct station_info *station_info;
struct station_info *station_link;
station_info = wifi_softap_get_station_info();
ets_printf("station_info: %p!\n", station_info);
  station_link = (struct station_info *)station_info;
  while (station_link != NULL) {
ets_printf("bssid: ");
ets_printf(MACSTR, MAC2STR(station_info->bssid));
ets_printf("\n");
ets_printf("ip: ");
ets_printf(IPSTR, IP2STR(&station_info->ip));
ets_printf("\n");
    station_link =(struct station_info *) (station_info->next.stqe_next);
  }

wifi_softap_free_station_info();
#endif

#define TIMER_MODE_OFF 3
#define TIMER_MODE_SINGLE 0
#define TIMER_MODE_SEMI 2
#define TIMER_MODE_AUTO 1
#define TIMER_IDLE_FLAG (1<<7) 

typedef struct structmsgTimer {
        os_timer_t timer;
        structmsgDispatcher_t *self;
        uint32_t interval;
        uint8_t mode;
} structmsgTimer_t;

static structmsgTimer_t structmsgTimers[NUM_TMR];
static int isMstimer = 1;
static os_timer_t apTimer;
static int cnt = 0;

// ================================= alarmTimerAP ====================================

void alarmTimerAP(void *arg) {
  structmsgDispatcher_t *self;
  struct ip_info pTempIp;
  uint8_t timerId;
  char temp[64];
  structmsgTimer_t *tmr;
  uint8_t mode = SOFTAP_IF;
  int numericValue;
  uint8_t *stringValue;
  struct espconn *pesp_conn = NULL;
  unsigned port;
  ip_addr_t ipaddr;
  const char *domain;
  unsigned type;
  int result;

  timerId = (uint8_t)((uint32_t)arg);
  tmr = &structmsgTimers[timerId];
  self = tmr->self;
ets_printf("alarmTimerAP: timerId: %d self: %p\n", timerId, self);
  wifi_get_ip_info(mode, &pTempIp);
  if(pTempIp.ip.addr==0){
ets_printf("ip: nil\n");
    return;
  }
  tmr->mode |= TIMER_IDLE_FLAG;
  c_sprintf(temp, "%d.%d.%d.%d", IP2STR(&pTempIp.ip));
ets_printf("IP: %s\n", temp);
  ets_timer_disarm(&tmr->timer);

  result = self->getModuleValue(self, MODULE_INFO_PROVISIONING_PORT, DATA_VIEW_FIELD_UINT8_T, &numericValue, &stringValue);
ets_printf("port: %d!%p!%d!\n", numericValue, stringValue, result);
//  checkErrOK(result);
  port = numericValue;
  result = self->getModuleValue(self, MODULE_INFO_PROVISIONING_IP_ADDR, DATA_VIEW_FIELD_UINT8_VECTOR, &numericValue, &stringValue);
//  checkErrOK(result);
  domain = (char *)stringValue;
domain = "0.0.0.0";

  pesp_conn = (struct espconn *)os_zalloc(sizeof(struct espconn));
//  checkAllocOK(pesp_conn);

  type = ESPCONN_TCP;
  pesp_conn->type = type;
  pesp_conn->state = ESPCONN_NONE;
  // reverse is for the callback function
  pesp_conn->reverse = pesp_conn;

  pesp_conn->proto.tcp = NULL;
  pesp_conn->proto.udp = NULL;

  pesp_conn->proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));
  if(!pesp_conn->proto.tcp){
    os_free(pesp_conn);
    pesp_conn = NULL;
//    checkErrOK(STRUCT_DISP_ERR_OUT_OF_MEMORY);
  }
  pesp_conn->proto.tcp->local_port = port;
ets_printf("port: %d\n", port);

ets_printf("call regist connectcb\n");
    result = espconn_regist_connectcb(pesp_conn, serverConnected);
    if (result != STRUCT_DISP_ERR_OK) {
//      return STRUCT_DISP_ERR_REGIST_CONNECT_CB;
    }
ets_printf("regist connectcb result: %d\n", result);
    result = espconn_accept(pesp_conn);    // if it's a server, no need to dns.
    if (result != STRUCT_DISP_ERR_OK) {
//      return STRUCT_DISP_ERR_TCP_ACCEPT;
    }
ets_printf("regist_accept result: %d\n", result);
    result =espconn_regist_time(pesp_conn, tcp_server_timeover, 0);
    if (result != STRUCT_DISP_ERR_OK) {
//      return STRUCT_DISP_ERR_REGIST_TIME;
    }
ets_printf("regist_time result: %d\n", result);
}

// ================================= initTimers ====================================

static uint8_t initTimers(structmsgDispatcher_t *self) {
  int result;

  for(int i = 0; i < NUM_TMR; i++) {
    structmsgTimers[i].self = self;
    structmsgTimers[i].mode = TIMER_MODE_OFF;
    ets_timer_disarm(&structmsgTimers[i].timer);
  }
  return STRUCT_DISP_ERR_OK;
}


// ================================= websocketRunAPMode ====================================

static uint8_t websocketRunAPMode(structmsgDispatcher_t *self) {
  int result;
  bool boolResult;
  struct softap_config softap_config;
  int numericValue;
  uint8_t *stringValue;
  
  boolResult = wifi_station_disconnect();
  if (!boolResult) {
    return STRUCT_DISP_ERR_CANNOT_DISCONNECT;
  }
//  boolResult = wifi_set_opmode(OPMODE_STATION);
//  if (!boolResult) {
//    return STRUCT_DISP_ERR_CANNOT_SET_OPMODE;
//  }
//return STRUCT_DISP_ERR_OK;
  boolResult = wifi_set_opmode(OPMODE_STATIONAP);
  if (!boolResult) {
    return STRUCT_DISP_ERR_CANNOT_SET_OPMODE;
  }
  c_memset(softap_config.ssid,0,sizeof(softap_config.ssid));
  result = self->getModuleValue(self, MODULE_INFO_PROVISIONING_SSID, DATA_VIEW_FIELD_UINT8_VECTOR, &numericValue, &stringValue);
  checkErrOK(result);
  c_memcpy(softap_config.ssid, stringValue, c_strlen(stringValue));
  softap_config.ssid_len = c_strlen(stringValue);
  softap_config.ssid_hidden = 0;
  softap_config.authmode = AUTH_OPEN;
  softap_config.channel = 6;
  softap_config.max_connection = 4;
  softap_config.beacon_interval = 100;
  boolResult = wifi_softap_set_config(&softap_config);
  if (!boolResult) {
    return STRUCT_DISP_ERR_CANNOT_SET_OPMODE;
  }
ets_printf("wifi is in mode: %d status: %d ap_id: %d hostname: %s!\n", wifi_get_opmode(), wifi_station_get_connect_status(), wifi_station_get_current_ap_id(), wifi_station_get_hostname());

ets_printf("meminfo\n");
system_print_meminfo();
ets_printf("malloc\n");
system_show_malloc();
ets_printf("malloc done\n");

ets_printf("register alarmTimerAP: self: %p\n", self);
int repeat = 1;
int interval = 1000;
int timerId = 0;
int mode = TIMER_MODE_AUTO;
structmsgTimer_t *tmr = &structmsgTimers[timerId];
if (!(tmr->mode & TIMER_IDLE_FLAG) && (tmr->mode != TIMER_MODE_OFF)) {
  ets_timer_disarm(&tmr->timer);
}
// this is only preparing
ets_timer_setfn(&tmr->timer, alarmTimerAP, (void*)timerId);
tmr->mode = mode | TIMER_IDLE_FLAG;
// here is the start
tmr->interval = interval;
tmr->mode &= ~TIMER_IDLE_FLAG;
ets_timer_arm_new(&tmr->timer, interval, repeat, isMstimer);
ets_printf("arm_new done\n");



#ifdef NOTDEF
  if srv ~= nil then
    srv:close()
    srv=nil
  end




  if (not tmr.alarm(1,1000,tmr.ALARM_AUTO,function()
     ip=wifi.ap.getip()
     if (ip ~= nil) then
       tmr.stop(1)
dbgPrint('Provisioning: wifi mode: '..tostring(wifi.getmode()))
dbgPrint("Provisioning: AP IP: "..tostring(wifi.ap.getip()))
dbgPrint("Provisioning: STA IP: "..tostring(wifi.sta.getip()))
       srv=websocket.createServer(30,srv_connected)
#endif

  return STRUCT_DISP_ERR_OK;
}

// ================================= structmsgWebsocketInit ====================================

uint8_t structmsgWebsocketInit(structmsgDispatcher_t *self) {
  uint8_t result;

  result = initTimers(self);
  checkErrOK(result);

  self->websocketRunClientMode = &websocketRunClientMode;
  self->websocketRunAPMode = &websocketRunAPMode;
ets_printf("call webscoketRunAPMode\n");
result = websocketRunAPMode(self);
ets_printf("webscoketRunAPMode result: %d\n", result);
checkErrOK(result);
  return STRUCT_DISP_ERR_OK;
}
