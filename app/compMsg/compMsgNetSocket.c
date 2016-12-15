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
 * File:   compMsgNetsocket.c
 * Author: Arnulf P. Wiedemann
 *
 * Created on October 10th, 2016
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

#define TCP ESPCONN_TCP

typedef struct socketInfo {
  struct espconn *pesp_conn;
} socketInfo_t;

#define MAX_SOCKET 5
static int socket_num = 0;
static socketInfo_t *socket[MAX_SOCKET] = { NULL, NULL, NULL, NULL, NULL };

static uint8_t err_opcode[5] = {0};

// #define checkAllocOK(addr) if(addr == NULL) checkErrOK(NETSOCKET_ERR_OUT_OF_MEMORY, "")

enum compMsg_error_code
{
  NETSOCKET_ERR_OK = 0,
  NETSOCKET_ERR_OUT_OF_MEMORY = -1,
  NETSOCKET_ERR_TOO_MUCH_DATA = -2,
  NETSOCKET_ERR_INVALID_FRAME_OPCODE = -3,
  NETSOCKET_ERR_USERDATA_IS_NIL = -4,
  NETSOCKET_ERR_WRONG_METATABLE = -5,
  NETSOCKET_ERR_PESP_CONN_IS_NIL = -6,
  NETSOCKET_ERR_MAX_SOCKET_REACHED = -7,
};

#define BASE64_INVALID '\xff'
#define BASE64_PADDING '='
#define ISBASE64(c) (unbytes64[c] != BASE64_INVALID)

#define SSL_BUFFER_SIZE 5120

static const uint8 b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

typedef void (* netsocketBinaryReceived_t)(void *arg, void *nud, char *pdata, unsigned short len);
typedef void (* netsocketTextReceived_t)(void *arg, void *nud, char *pdata, unsigned short len);

#define TIMER_MODE_OFF 3
#define TIMER_MODE_SINGLE 0
#define TIMER_MODE_SEMI 2
#define TIMER_MODE_AUTO 1
#define TIMER_IDLE_FLAG (1<<7) 

typedef struct compMsgTimer {
  os_timer_t timer;
  compMsgDispatcher_t *self;
  uint32_t interval;
  uint8_t mode;
} compMsgTimer_t;

static compMsgTimer_t compMsgTimers[NUM_TMR];
static int isMstimer = 1;
static os_timer_t apTimer;
static int cnt = 0;
static ip_addr_t host_ip; // for dns
static int dns_reconn_count = 0;

// ============================ netsocketSendData =======================

/**
 * \brief send message data to a http scoket
 * \param nud The net (http) socket user data
 * \param payload The message data
 * \param size The number of characters of the message
 * \return Error code or ErrorOK
 *
 */
static uint8_t netsocketSendData(netsocketUserData_t *nud, const char *payload, int size)
{
  uint8_t result;

ets_printf("§netsocketSendData: size: %d§", size);
#ifdef CLIENT_SSL_ENABLE
  if (nud->secure) {
    result = espconn_secure_sent(nud->pesp_conn, (unsigned char *)payload, size);
  } else
#endif
  {
    result = espconn_sent(nud->pesp_conn, (unsigned char *)payload, size);
  }

ets_printf("§netsocketSendData: result: %d§", result);
  checkErrOK(result);
  return NETSOCKET_ERR_OK;
}

// ============================ netsocket_parse =========================================

static int ICACHE_FLASH_ATTR netsocket_parse(char * data, size_t size, char **resData, int *len, netsocketUserData_t *nud) {
  uint8_t result;

//  result = nud->netsocketBinaryReceived(nud->compMsgDispatcher, nud, data, size);
//  checkErrOK(result);
  return NETSOCKET_ERR_OK;
}

// ================================= netsocket_recv ====================================

static uint8_t netsocket_recv(char *payload, netsocketUserData_t *nud, char **data, int *lgth) {
  char *key = NULL;
  int idx;
  int found;

  idx = 0;
ets_printf("§netsocket_recv:remote_port: %d§\n", nud->remote_port);
//ets_printf("netsocket_recv: %s!remote_port: %d\n",payload, nud->remote_port);

//ets_printf("payload: %d!%s!\n", c_strlen(payload), payload);
//ets_printf("netsocket_parse: %d!curr_url: %s!\n", os_strlen(string), nud->curr_url);
//FIXME need to handle payload!!
  nud->netsocketReceived(nud->compMsgDispatcher, nud, payload, c_strlen(payload));
//  netsocket_parse(string, os_strlen(string), data, lgth, nud);
  return NETSOCKET_ERR_OK;

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
  netsocketUserData_t *nud;
  char *data = "";
  int lgth = 0;
  int result;

  pesp_conn = (struct espconn *)arg;
//ets_printf("§socketReceived: arg: %p len: %d§"\n, arg, len);
ets_printf("§socketReceived: arg: %p pdata: %s len: %d§", arg, pdata, len);
  char temp[20] = {0};
  c_sprintf(temp, IPSTR, IP2STR( &(pesp_conn->proto.tcp->remote_ip) ) );
  ets_printf("§remote ");
  ets_printf(temp);
  ets_printf(":");
  ets_printf("%d",pesp_conn->proto.tcp->remote_port);
  ets_printf(" received§\n");

  nud = (netsocketUserData_t *)pesp_conn->reverse;
  nud->remote_ip[0] = pesp_conn->proto.tcp->remote_ip[0];
  nud->remote_ip[1] = pesp_conn->proto.tcp->remote_ip[1];
  nud->remote_ip[2] = pesp_conn->proto.tcp->remote_ip[2];
  nud->remote_ip[3] = pesp_conn->proto.tcp->remote_ip[3];
  nud->remote_port = pesp_conn->proto.tcp->remote_port;
//ets_printf("==received remote_port: %d\n", nud->remote_port);

  result = netsocket_recv(pdata, nud, &data, &lgth);
//  checkErrOK(gL,result,"netsocket_recv");
}

// ================================= socketSent  ====================================

static void socketSent(void *arg) {
  struct espconn *pesp_conn;

  pesp_conn = (struct espconn *)arg;
ets_printf("§socketSent: arg: %p§", arg);
  pesp_conn = arg;

}

// ================================= netDelete  ====================================

static void netDelete(void *arg) {
  struct espconn *pesp_conn;
  netsocketUserData_t *nud;
  int result;

ets_printf("§netDelete§");
  pesp_conn = (struct espconn *)arg;
  if (pesp_conn == NULL) {
    return;
  }
  nud = (netsocketUserData_t *)pesp_conn->reverse;
  if(nud == NULL) {
    return;
  }
  if(nud->pesp_conn) {     // for client connected to tcp server, this should set NULL in disconnect cb
    nud->pesp_conn->reverse = NULL;
    if (nud->pesp_conn->proto.tcp) {
      os_free (nud->pesp_conn->proto.tcp);
      nud->pesp_conn->proto.tcp = NULL;
    }
    os_free (nud->pesp_conn);
    nud->pesp_conn = NULL;
  }
}

// ================================= netSocketDisconnected  ====================================

static void netSocketDisconnected(void *arg) {  // tcp only
  struct espconn *pesp_conn;
  netsocketUserData_t *nud;
  int result;

ets_printf("§netSocketDisconnected is called§");
  pesp_conn = (struct espconn *)arg;
  if (pesp_conn == NULL) {
    return;
  }
  nud = (netsocketUserData_t *)pesp_conn->reverse;
  if(nud == NULL) {
    return;
  }
  switch (nud->connectionType) {
  case NET_SOCKET_TYPE_CLIENT:
    nud->compMsgDispatcher->runningModeFlags &= ~COMP_DISP_RUNNING_MODE_CLIENT;
    break;
  case NET_SOCKET_TYPE_SOCKET:
    nud->compMsgDispatcher->runningModeFlags &= ~COMP_DISP_RUNNING_MODE_CLOUD;
    break;
  default:
ets_printf("§netSocketDisconnected bad connectionType: 0x%02x§", nud->connectionType);
    break;
  }
  if (pesp_conn->proto.tcp) {
    os_free (pesp_conn->proto.tcp);
  }
  pesp_conn->proto.tcp = NULL;
  if (nud->pesp_conn) {
    os_free(nud->pesp_conn);
  }
  nud->pesp_conn = NULL;  // espconn is already disconnected
}

// ================================= netSocketReconnected  ====================================

static void netSocketReconnected (void *arg, sint8_t err) {
ets_printf("§net_socket_reconnected is called err: %d§", err);
  netSocketDisconnected (arg);
}

// ================================= netSocketReceived  ====================================

static void netSocketReceived(void *arg, char *pdata, unsigned short len) {
  struct espconn *pesp_conn;
  netsocketUserData_t *nud;
  int result;

  pesp_conn = (struct espconn *)arg;
ets_printf("§netSocketReceived is called. %d %s§", len, pdata);
  if (pesp_conn == NULL) {
    return;
  }
  nud = (netsocketUserData_t *)pesp_conn->reverse;
  if(nud == NULL) {
    return;
  }
}

// ================================= netSocketSent  ====================================

static void netSocketSent(void *arg) {
  struct espconn *pesp_conn;
  netsocketUserData_t *nud;
  int result;
  bool boolResult;

ets_printf("§netSocketSent is called§");
  pesp_conn = (struct espconn *)arg;
  if (pesp_conn == NULL) {
    return;
  }
  nud = (netsocketUserData_t *)pesp_conn->reverse;
  if(nud == NULL) {
    return;
  }
}

// ================================= netSocketConnected  ====================================

static void netSocketConnected(void *arg) {
  struct espconn *pesp_conn;
  netsocketUserData_t *nud;
  int result;

//ets_printf("§netSocketConnected§\n");
  pesp_conn = arg;
  if (pesp_conn == NULL) {
    return;
  }
  nud = (netsocketUserData_t *)pesp_conn->reverse;
  if(nud == NULL) {
    return;
  }
  // can receive and send data
  result = espconn_regist_recvcb (pesp_conn, netSocketReceived);
if (result != COMP_DISP_ERR_OK) {
ets_printf("§espconn_regist_recvcb: result: %d§\n", result);
}
  result = espconn_regist_sentcb (pesp_conn, netSocketSent);
if (result != COMP_DISP_ERR_OK) {
ets_printf("§espconn_regist_sentcb: result: %d§\n", result);
}
  result = espconn_regist_disconcb (pesp_conn, netSocketDisconnected);
if (result != COMP_DISP_ERR_OK) {
ets_printf("§espconn_regist_disconcb: result: %d§\n", result);
}
  nud->compMsgDispatcher->compMsgData->nud = nud;
//ets_printf("§startSendMsg2: %p§\n", nud->compMsgDispatcher->startSendMsg2);
  nud->compMsgDispatcher->runningModeFlags |= COMP_DISP_RUNNING_MODE_CLOUD;
  if (nud->compMsgDispatcher->startSendMsg2 != NULL) {
    result = nud->compMsgDispatcher->startSendMsg2(nud->compMsgDispatcher);
  } else {
ets_printf("§nud->compMsgDispatcher->startSendMsg2 is NULL§");
  }
}

// ================================= socketConnect  ====================================

static void socketConnect(void *arg) {
  struct espconn *pesp_conn;
  netsocketUserData_t *nud;
  int result;

//ets_printf("§socketConnect§");
  pesp_conn = arg;
  if (pesp_conn == NULL) {
    return;
  }
  nud = (netsocketUserData_t *)pesp_conn->reverse;
  if(nud == NULL) {
    return;
  }
#ifdef CLIENT_SSL_ENABLE
  if (nud->secure){
    espconn_secure_set_size(ESPCONN_CLIENT, 5120); /* set SSL buffer size */
//ets_printf("§call espconn_secure_connect§");
    int espconn_status = espconn_secure_connect(pesp_conn);
//ets_printf("§after call espconn_secure_connect status: %d§", espconn_status);

  } else
#endif
  {
//ets_printf("§socketConnect called§");
    result = espconn_connect(pesp_conn);
//ets_printf("§espconn_connect: result: %d§", result);
  }
}

// ================================= serverConnected  ====================================

static void serverConnected(void *arg) {
  struct espconn *pesp_conn;
  netsocketUserData_t *nud;
  int result;
  int i;

//ets_printf("§serverConnected: arg: %p§", arg);
  pesp_conn = arg;
  nud = (netsocketUserData_t *)pesp_conn->reverse;
  if(nud == NULL) {
    return;
  }
  for(i = 0; i < MAX_SOCKET; i++) {
    if (socket[i] == NULL) { // found empty slot
      break;
    }
  }
  if(i>=MAX_SOCKET) {// can't create more socket
#ifdef CLIENT_SSL_ENABLE
    if (nud->secure) {
      if(pesp_conn->proto.tcp->remote_port || pesp_conn->proto.tcp->local_port) {
        espconn_secure_disconnect(pesp_conn);
      }
    } else
#endif
    {
      if(pesp_conn->proto.tcp->remote_port || pesp_conn->proto.tcp->local_port) {
        espconn_disconnect(pesp_conn);
      }
    }
//    checkErrOK(gL, NETSOCKET_ERR_MAX_SOCKET_REACHED, "netsocket_server_connected");
    pesp_conn->reverse = NULL;    // not accept this conn
    return;
  }
//ets_printf("registstart\n");
  result = espconn_regist_recvcb(pesp_conn, socketReceived);
  if (result != COMP_DISP_ERR_OK) {
ets_printf("§regist socketReceived err: %d§", result);
  }
  result = espconn_regist_sentcb(pesp_conn, socketSent);
  if (result != COMP_DISP_ERR_OK) {
ets_printf("§regist socketSent err: %d§", result);
  }
  result = espconn_regist_disconcb(pesp_conn, serverDisconnected);
  if (result != COMP_DISP_ERR_OK) {
ets_printf("§regist serverDisconnected err: %d§", result);
  }
  result = espconn_regist_reconcb(pesp_conn, serverReconnected);
  if (result != COMP_DISP_ERR_OK) {
ets_printf("§regist serverReconnected err: %d§", result);
  }
}

// ================================= netSocketStart ====================================

static void netSocketStart(void *arg) {
  struct espconn *pesp_conn;
  netsocketUserData_t *nud;
  int result;

ets_printf("§netSocketStart§");
  if (pesp_conn == NULL) {
    return;
  }
  nud = (netsocketUserData_t *)pesp_conn->reverse;
  if(nud == NULL) {
    return;
  }

}

// ================================= socketDnsFound ====================================

static void socketDnsFound(const char *name, ip_addr_t *ipaddr, void *arg) {
//ets_printf("§socket_dns_found is called§");
  struct espconn *pesp_conn = arg;
  netsocketUserData_t *nud;
  if (pesp_conn == NULL) {
    ets_printf("§pesp_conn null§");
    return;
  }
  nud = (netsocketUserData_t *)pesp_conn->reverse;
  if (nud == NULL) {
    return;
  }
//ets_printf("§ip: %p§", ipaddr);
  if (ipaddr == NULL) {
    dns_reconn_count++;
    if (dns_reconn_count >= 5) {
      ets_printf( "§DNS Fail!§" );
      return;
    }
    ets_printf("§DNS retry %d!§", dns_reconn_count);
    host_ip.addr = 0;
    espconn_gethostbyname(pesp_conn, name, &host_ip, socketDnsFound);
    return;
  }

  // ipaddr->addr is a uint32_t ip
  if(ipaddr->addr != 0) {
    dns_reconn_count = 0;
    if( pesp_conn->type == ESPCONN_TCP ) {
      c_memcpy(pesp_conn->proto.tcp->remote_ip, &(ipaddr->addr), 4);
      ets_printf("§TCP ip is set: ");
      ets_printf(IPSTR, IP2STR(&(ipaddr->addr)));
      ets_printf("§");
    } else {
      if (pesp_conn->type == ESPCONN_UDP) {
        c_memcpy(pesp_conn->proto.udp->remote_ip, &(ipaddr->addr), 4);
        ets_printf("§UDP ip is set: ");
        ets_printf(IPSTR, IP2STR(&(ipaddr->addr)));
        ets_printf("§");
      }
    }
    socketConnect(pesp_conn);
  }
}

// ================================= openCloudSocket ====================================

static uint8_t openCloudSocket(compMsgDispatcher_t *self) {
  char temp[64];
  uint8_t mode;
  int numericValue;
  uint8_t *stringValue;
  struct espconn *pesp_conn;
  unsigned port;
  struct ip_info pTempIp;
  ip_addr_t ipaddr;
  unsigned type;
  int result;
  const char *domain;
  netsocketUserData_t *nud;

  pesp_conn = NULL;

  nud = (netsocketUserData_t *)os_zalloc(sizeof(netsocketUserData_t));
//   checkAllocOK(nud);
//ets_printf("§nud0: %p§", nud);
//  checkAllocgLOK(nud->urls);
  nud->connectionType = NET_SOCKET_TYPE_SOCKET;
#ifdef CLIENT_SSL_ENABLE
  result = self->getWifiValue(self, WIFI_INFO_CLOUD_SECURE_CONNECT, DATA_VIEW_FIELD_UINT8_T, &numericValue, &stringValue);
  nud->secure = numericValue;
#endif
  result = self->getWifiValue(self, WIFI_INFO_NET_RECEIVED_CALL_BACK, DATA_VIEW_FIELD_UINT32_T, &numericValue, &stringValue);
  nud->netsocketReceived = (netsocketReceived_t)numericValue;
  result = self->getWifiValue(self, WIFI_INFO_NET_TO_SEND_CALL_BACK, DATA_VIEW_FIELD_UINT32_T, &numericValue, &stringValue);
  nud->netsocketToSend = (netsocketToSend_t)numericValue;
  nud->compMsgDispatcher = self;
//ets_printf("§netsocketReceived: %p netsocketToSend: %p§", nud->netsocketReceived, nud->netsocketToSend);

  result = self->getWifiValue(self, WIFI_INFO_CLOUD_PORT, DATA_VIEW_FIELD_UINT8_T, &numericValue, &stringValue);
  port = numericValue;

//ets_printf("§port: %d§", port);
  pesp_conn = (struct espconn *)os_zalloc(sizeof(struct espconn));
  nud->pesp_conn = pesp_conn;
//  checkAllocOK(pesp_conn);

  type = ESPCONN_TCP;
  pesp_conn->type = type;
  pesp_conn->state = ESPCONN_NONE;
  // reverse is for the callback function
  pesp_conn->reverse = nud;

  pesp_conn->proto.tcp = NULL;
  pesp_conn->proto.udp = NULL;

  pesp_conn->proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));
  if(!pesp_conn->proto.tcp){
    os_free(pesp_conn);
    pesp_conn = NULL;
//    checkErrOK(COMP_DISP_ERR_OUT_OF_MEMORY);
  }
  pesp_conn->proto.tcp->remote_port = port;
  pesp_conn->proto.tcp->local_port = espconn_port();

#ifdef CLOUD_1
  result = self->getWifiValue(self, WIFI_INFO_CLOUD_DOMAIN_1, DATA_VIEW_FIELD_UINT8_T, &numericValue, &stringValue);
#else
  result = self->getWifiValue(self, WIFI_INFO_CLOUD_DOMAIN_2, DATA_VIEW_FIELD_UINT8_T, &numericValue, &stringValue);
#endif
  domain = stringValue;
//ets_printf("§domain: %s§", domain);
  ipaddr.addr = ipaddr_addr(domain);
  c_memcpy(pesp_conn->proto.tcp->remote_ip, &ipaddr.addr, 4);
  ets_printf("§TCP ip is set: ");
  ets_printf(IPSTR, IP2STR(&ipaddr.addr));
  ets_printf("§");

//ets_printf("call regist connectcb\n");
  result = espconn_regist_connectcb(pesp_conn, netSocketConnected);
  if (result != COMP_DISP_ERR_OK) {
//    return COMP_DISP_ERR_REGIST_CONNECT_CB;
  }
  result = espconn_regist_reconcb(pesp_conn, netSocketReconnected);
  if (result != COMP_DISP_ERR_OK) {
//    return COMP_DISP_ERR_REGIST_CONNECT_CB;
  }
  result = espconn_regist_sentcb(pesp_conn, socketSent);
  if (result != COMP_DISP_ERR_OK) {
ets_printf("§regist socketSent err: %d§", result);
  }
#ifdef CLIENT_SSL_ENABLE
//ets_printf("§socket: secure: %d§", nud->secure);
  if (nud->secure){
    if (pesp_conn->proto.tcp->remote_port || pesp_conn->proto.tcp->local_port)
      espconn_secure_disconnect(pesp_conn);
  } else
#endif
  { 
    if(pesp_conn->proto.tcp->remote_port || pesp_conn->proto.tcp->local_port) {
      espconn_disconnect(pesp_conn);
    }
  }
  host_ip.addr = 0;
  dns_reconn_count = 0;
  if (ESPCONN_OK == espconn_gethostbyname(pesp_conn, domain, &host_ip, socketDnsFound)) {
//ets_printf("§call gethostbyname: found ip for %s 0x%08x§", domain, host_ip);
    socketDnsFound(domain, &host_ip, pesp_conn);  // ip is returned in host_ip.
  }
  return COMP_DISP_ERR_OK;
}

// ================================= startClientMode ====================================

static  void startClientMode(void *arg) {
  compMsgDispatcher_t *self;
  uint8_t timerId;
  char temp[64];
  compMsgTimer_t *tmr;
  uint8_t mode;
  int numericValue;
  uint8_t *stringValue;
  struct espconn *pesp_conn;
  unsigned port;
  struct ip_info pTempIp;
  ip_addr_t ipaddr;
  unsigned type;
  int result;
  uint8_t status;
  bool boolResult;
  netsocketUserData_t *nud;
  char *hostname;
  uint8_t ap_id;
  uint8_t opmode;

  pesp_conn = NULL;
  mode = STATION_IF;
  timerId = (uint8_t)((uint32_t)arg);
  tmr = &compMsgTimers[timerId];
  self = tmr->self;
//ets_printf("§startClientMode: timerId: %d self: %p§", timerId, self);
  status = wifi_station_get_connect_status();
//ets_printf("§status: %d§", status);
  ap_id = wifi_station_get_current_ap_id();
//ets_printf("§ap_id: %d§", ap_id);
  opmode = wifi_get_opmode();
//ets_printf("§opmode: %d§", opmode);
  hostname = wifi_station_get_hostname();
//ets_printf("§hostname: %s§", hostname);
ets_printf("§startClientMode: wifi is in mode: %d status: %d ap_id: %d hostname: %s!§", opmode, status, ap_id, hostname);
  switch (status) {
  case STATION_IDLE:
//ets_printf("§STATION_IDLE§");
    break;
  case STATION_CONNECTING:
//ets_printf("§STATION_CONNECTING§");
    return;
    break;
  case STATION_WRONG_PASSWORD:
//ets_printf("§STATION_WRONG_PASSWORD§");
    tmr->mode |= TIMER_IDLE_FLAG;
    ets_timer_disarm(&tmr->timer);
    self->netsocketSendConnectError(self, status);
    return;
    break;
  case STATION_NO_AP_FOUND:
//ets_printf("§STATION_NO_AP_FOUND§");
    tmr->mode |= TIMER_IDLE_FLAG;
    ets_timer_disarm(&tmr->timer);
    self->netsocketSendConnectError(self, status);
    return;
    break;
  case STATION_CONNECT_FAIL:
//ets_printf("§STATION_CONNECT_FAIL§");
    tmr->mode |= TIMER_IDLE_FLAG;
    ets_timer_disarm(&tmr->timer);
    self->netsocketSendConnectError(self, status);
    return;
    break;
  case STATION_GOT_IP:
//ets_printf("§STATION_GOT_IP§");
    break;
  }
  wifi_get_ip_info(mode, &pTempIp);
  if(pTempIp.ip.addr==0){
//ets_printf("§ip: nil§");
    return;
  }
  tmr->mode |= TIMER_IDLE_FLAG;
//  c_sprintf(temp, "%d.%d.%d.%d", IP2STR(&pTempIp.ip));
//ets_printf("§IP: %s§", temp);
  ets_timer_disarm(&tmr->timer);
  self->runningModeFlags |= COMP_DISP_RUNNING_MODE_CLIENT;
  result = self->setWifiValue(self, "@clientIPAddr", pTempIp.ip.addr, NULL);
  result = self->getWifiValue(self, WIFI_INFO_CLIENT_IP_ADDR, DATA_VIEW_FIELD_UINT32_T, &numericValue, &stringValue);
//ets_printf("§ip2: 0x%08x§\n", numericValue);
  c_sprintf(temp, "%d.%d.%d.%d", IP2STR(&numericValue));

  result = self->getWifiValue(self, WIFI_INFO_CLIENT_PORT, DATA_VIEW_FIELD_UINT8_T, &numericValue, &stringValue);
  port = numericValue;
ets_printf("§IP: %s port: %d result: %d§", temp, port, result);

  nud = (netsocketUserData_t *)os_zalloc(sizeof(netsocketUserData_t));
//   checkAllocOK(nud);
//ets_printf("nud0: %p\n", nud);
//  checkAllocgLOK(nud->urls);
  nud->connectionType = NET_SOCKET_TYPE_CLIENT;
#ifdef CLIENT_SSL_ENABLE
  result = self->getWifiValue(self, WIFI_INFO_CLOUD_SECURE_CONNECT, DATA_VIEW_FIELD_UINT8_T, &numericValue, &stringValue);
  nud->secure = numericValue;
#endif
  result = self->getWifiValue(self, WIFI_INFO_NET_RECEIVED_CALL_BACK, DATA_VIEW_FIELD_UINT32_T, &numericValue, &stringValue);
//ets_printf("§netReceivedCallback: %p!%d!§\n", numericValue, result);
  nud->netsocketReceived = (netsocketReceived_t)numericValue;
  result = self->getWifiValue(self, WIFI_INFO_NET_RECEIVED_CALL_BACK, DATA_VIEW_FIELD_UINT32_T, &numericValue, &stringValue);
//ets_printf("§netToSendCallback: %p!%d!§\n", numericValue, result);
  nud->netsocketToSend = (netsocketToSend_t)numericValue;
  nud->compMsgDispatcher = self;

  pesp_conn = (struct espconn *)os_zalloc(sizeof(struct espconn));
  nud->pesp_conn = pesp_conn;
//  checkAllocOK(pesp_conn);

  type = ESPCONN_TCP;
  pesp_conn->type = type;
  pesp_conn->state = ESPCONN_NONE;
  // reverse is for the callback function
  pesp_conn->reverse = nud;

  pesp_conn->proto.tcp = NULL;
  pesp_conn->proto.udp = NULL;

  pesp_conn->proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));
  if(!pesp_conn->proto.tcp){
    os_free(pesp_conn);
    pesp_conn = NULL;
//    checkErrOK(COMP_DISP_ERR_OUT_OF_MEMORY);
  }
  pesp_conn->proto.tcp->local_port = port;
//ets_printf("§port: %d§\n", port);

//ets_printf("§call regist connectcb§\n");
  result = espconn_regist_connectcb(pesp_conn, serverConnected);
  if (result != COMP_DISP_ERR_OK) {
//    return COMP_DISP_ERR_REGIST_CONNECT_CB;
  }
//ets_printf("§regist connectcb result: %d§\n", result);
  result = espconn_regist_recvcb(pesp_conn, socketReceived);
  if (result != COMP_DISP_ERR_OK) {
ets_printf("§regist socketReceived err: %d§", result);
  }
  result = espconn_regist_sentcb(pesp_conn, socketSent);
  if (result != COMP_DISP_ERR_OK) {
ets_printf("§regist socketSent err: %d§", result);
  }
  result = espconn_accept(pesp_conn);
  if (result != COMP_DISP_ERR_OK) {
//    return COMP_DISP_ERR_TCP_ACCEPT;
ets_printf("§regist_accept err result: %d§", result);
  }
  result = espconn_regist_time(pesp_conn, tcp_server_timeover, 0);
  if (result != COMP_DISP_ERR_OK) {
//    return COMP_DISP_ERR_REGIST_TIME;
ets_printf("§regist_time err result: %d§", result);
  }
  if (self->startSendMsg != NULL) {
    result = self->startSendMsg(self);
//ets_printf("§startSendMsg result: %d§", result);
  }
}

// ================================= netsocketRunClientMode ====================================

static uint8_t netsocketRunClientMode(compMsgDispatcher_t *self) {
  int result;
  bool boolResult;
  struct station_config station_config;
  int numericValue;
  uint8_t *stringValue;
  uint8_t opmode;

//ets_printf("§netsocketRunClientMode called§\n");
  
  opmode = wifi_get_opmode();
//ets_printf("§opmode: %d§", opmode);
  boolResult = wifi_station_disconnect();
//ets_printf("§wifi_station_disconnect: boolResult: %d§", boolResult);
  c_memset(station_config.ssid,0,sizeof(station_config.ssid));
  result = self->getWifiValue(self, WIFI_INFO_CLIENT_SSID, DATA_VIEW_FIELD_UINT8_VECTOR, &numericValue, &stringValue);
//ets_printf("§getSsid: result: %d§\n", result);
  checkErrOK(result);
  c_memcpy(station_config.ssid, stringValue, c_strlen(stringValue));

  c_memset(station_config.password,0,sizeof(station_config.password));
  result = self->getWifiValue(self, WIFI_INFO_CLIENT_PASSWD, DATA_VIEW_FIELD_UINT8_VECTOR, &numericValue, &stringValue);
//ets_printf("§getPasswd: result: %d§\n", result);
  checkErrOK(result);
//ets_printf("len password: %d\n", c_strlen(stringValue));
  c_memcpy(station_config.password, stringValue, c_strlen(stringValue));
ets_printf("§netsocketRunClientMode: ssid: %s password: %s!§\n", station_config.ssid, station_config.password);

  boolResult = wifi_station_set_config(&station_config);
  if (!boolResult) {
    return COMP_DISP_ERR_CANNOT_SET_OPMODE;
  }
//  wifi_station_set_auto_connect(true);
  boolResult = wifi_station_connect();
  if (!boolResult) {
    return COMP_DISP_ERR_CANNOT_CONNECT;
  }
  boolResult = wifi_station_set_hostname("testDeviceClient");
ets_printf("§wifi is in mode: %d status: %d hostname: %s!§\n", wifi_get_opmode(), wifi_station_get_connect_status(), wifi_station_get_hostname());

  int repeat = 1;
  int interval = 1000;
  int timerId = 1;
  int mode = TIMER_MODE_AUTO;
  compMsgTimer_t *tmr = &compMsgTimers[timerId];
  if (!(tmr->mode & TIMER_IDLE_FLAG) && (tmr->mode != TIMER_MODE_OFF)) {
    ets_timer_disarm(&tmr->timer);
  }
  // this is only preparing
  ets_timer_setfn(&tmr->timer, startClientMode, (void*)timerId);
  tmr->mode = mode | TIMER_IDLE_FLAG;
  // here is the start
  tmr->interval = interval;
  tmr->mode &= ~TIMER_IDLE_FLAG;
  ets_timer_arm_new(&tmr->timer, interval, repeat, isMstimer);
//ets_printf("§netsocketRunClientMode done§\n");
  return COMP_DISP_ERR_OK;
}

// ================================= netsocketStartCloudSocket ====================================

static uint8_t netsocketStartCloudSocket (compMsgDispatcher_t *self) {
  int result;

//ets_printf("§netsocketStartCloudSocket called§");
  self->startSendMsg = NULL;
  result = openCloudSocket( self);
  checkErrOK(result);
  return COMP_DISP_ERR_OK;
}

// ================================= initTimers ====================================

static uint8_t initTimers(compMsgDispatcher_t *self) {
  int result;

  for(int i = 0; i < NUM_TMR; i++) {
    compMsgTimers[i].self = self;
    compMsgTimers[i].mode = TIMER_MODE_OFF;
    ets_timer_disarm(&compMsgTimers[i].timer);
  }
  return COMP_DISP_ERR_OK;
}

// ================================= compMsgNetsocketInit ====================================

uint8_t compMsgNetsocketInit(compMsgDispatcher_t *self) {
  uint8_t result;

  result = initTimers(self);
  checkErrOK(result);

  self->netsocketStartCloudSocket = &netsocketStartCloudSocket;
  self->netsocketRunClientMode = &netsocketRunClientMode;
  self->netsocketSendData = netsocketSendData;
  return COMP_DISP_ERR_OK;
}
