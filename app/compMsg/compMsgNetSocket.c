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
#include "lwip/app/espconn.h"
#include "lwip/ip_addr.h"
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

static const char *header_key = "Sec-WebSocket-Key: ";
static const char *ws_uuid ="258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

static const char *HEADER_NETSOCKETLINE = "Upgrade: netsocket";

static char *HEADER_OK = "HTTP/1.x 200 OK \r\n\
Server: ESP \r\n\
Connection: close \r\n\
Cache-Control: max-age=3600, public \r\n\
Content-Type: text/html \r\n\
Content-Encoding: gzip \r\n\r\n";

static char *HEADER_NETSOCKET_START = "\
HTTP/1.1 101 WebSocket Protocol Handshake\r\n\
Connection: Upgrade\r\n\
Upgrade: WebSocket\r\n\
Access-Control-Allow-Origin: http://";

static char *HEADER_NETSOCKET_URL = "192.168.178.67";

static char *HEADER_NETSOCKET_END = "\r\n\
Access-Control-Allow-Credentials: true\r\n\
Access-Control-Allow-Headers: content-type \r\n\
Sec-WebSocket-Accept: ";

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

// ============================ netsocketSendData =======================

static uint8_t netsocketSendData(netsocketUserData_t *nud, const char *payload, int size)
{
ets_printf("netsocketSendData: size: %d\n", size);
  espconn_sent(nud->pesp_conn, (unsigned char *)payload, size);
  return NETSOCKET_ERR_OK;
}

// ============================ netsocket_parse =========================================

static int ICACHE_FLASH_ATTR netsocket_parse(char * data, size_t size, char **resData, int *len, netsocketUserData_t *nud) {
  uint8_t result;

//  result = nud->netsocketBinaryReceived(nud->compMsgDispatcher, nud, data, size);
//  checkErrOK(result);
  return NETSOCKET_ERR_OK;
}

// ============================ toBase64 =========================================

static uint8_t *toBase64 ( const uint8_t *msg, size_t *len){
  size_t i;
  size_t n;
  size_t lgth;
  uint8_t * q;
  uint8_t *out;

  n = *len;
  if (!n)  // handle empty string case 
    return NULL;

  lgth = (n + 2) / 3 * 4;
  out = (uint8_t *)os_malloc(lgth + 1);
  out[lgth] = '\0';
  if (out == NULL) {
    return NULL;
  }
  uint8 bytes64[sizeof(b64)];
  c_memcpy(bytes64, b64, sizeof(b64));   //Avoid lots of flash unaligned fetches

  for (i = 0, q = out; i < n; i += 3) {
    int a = msg[i];
    int b = (i + 1 < n) ? msg[i + 1] : 0;
    int c = (i + 2 < n) ? msg[i + 2] : 0;
    *q++ = bytes64[a >> 2];
    *q++ = bytes64[((a & 3) << 4) | (b >> 4)];
    *q++ = (i + 1 < n) ? bytes64[((b & 15) << 2) | (c >> 6)] : BASE64_PADDING;
    *q++ = (i + 2 < n) ? bytes64[(c & 63)] : BASE64_PADDING;
  }
  *q = '\0';
  *len = q - out;
  return out;
}


// ================================= netsocket_recv ====================================

static uint8_t netsocket_recv(char *payload, netsocketUserData_t *nud, char **data, int *lgth) {
  char *key = NULL;
  int idx;
  int found;

  idx = 0;
ets_printf("netsocket_recv:remote_port: %d\n", nud->remote_port);
ets_printf("netsocket_recv: %s!remote_port: %d\n",payload, nud->remote_port);

ets_printf("payload: %d!%s!\n", c_strlen(payload), payload);
//ets_printf("netsocket_parse: %d!curr_url: %s!\n", os_strlen(string), nud->curr_url);
//FIXME need to handle payload!!
//  netsocket_parse(string, os_strlen(string), data, lgth, nud);
  return NETSOCKET_ERR_OK;

}

// ================================= netSocketRunClientMode ====================================

static uint8_t netsocketRunClientMode(compMsgDispatcher_t *self, uint8_t mode) {
  int result;
  return COMP_DISP_ERR_OK;
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
ets_printf("socketReceived: arg: %p len: %d\n", arg, len);
ets_printf("socketReceived: arg: %p pdata: %s len: %d\n", arg, pdata, len);
  char temp[20] = {0};
  c_sprintf(temp, IPSTR, IP2STR( &(pesp_conn->proto.tcp->remote_ip) ) );
  ets_printf("remote ");
  ets_printf(temp);
  ets_printf(":");
  ets_printf("%d",pesp_conn->proto.tcp->remote_port);
  ets_printf(" received.\n");

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
//    checkErrOK(gL, NETSOCKET_ERR_MAX_SOCKET_REACHED, "netsocket_server_connected");
    return;
  }
//ets_printf("registstart\n");
  result = espconn_regist_recvcb(pesp_conn, socketReceived);
  if (result != COMP_DISP_ERR_OK) {
//ets_printf("regist socketReceived err: %d\n", result);
  }
  result = espconn_regist_sentcb(pesp_conn, socketSent);
  if (result != COMP_DISP_ERR_OK) {
//ets_printf("regist socketSent err: %d\n", result);
  }
  result = espconn_regist_disconcb(pesp_conn, serverDisconnected);
  if (result != COMP_DISP_ERR_OK) {
//ets_printf("regist serverDisconnected err: %d\n", result);
  }
  result = espconn_regist_reconcb(pesp_conn, serverReconnected);
  if (result != COMP_DISP_ERR_OK) {
//ets_printf("regist serverReconnected err: %d\n", result);
  }

}

// ================================= alarmTimerAP ====================================

static  void alarmTimerAP(void *arg) {
  compMsgDispatcher_t *self;
  struct ip_info pTempIp;
  uint8_t timerId;
  char temp[64];
  compMsgTimer_t *tmr;
  uint8_t mode = SOFTAP_IF;
  int numericValue;
  uint8_t *stringValue;
  struct espconn *pesp_conn = NULL;
  unsigned port;
  ip_addr_t ipaddr;
  unsigned type;
  int result;
  netsocketUserData_t *nud;

  timerId = (uint8_t)((uint32_t)arg);
  tmr = &compMsgTimers[timerId];
  self = tmr->self;
//ets_printf("alarmTimerAP: timerId: %d self: %p\n", timerId, self);
  wifi_get_ip_info(mode, &pTempIp);
  if(pTempIp.ip.addr==0){
ets_printf("ip: nil\n");
    return;
  }
  tmr->mode |= TIMER_IDLE_FLAG;
  c_sprintf(temp, "%d.%d.%d.%d", IP2STR(&pTempIp.ip));
ets_printf("IP: %s\n", temp);
  ets_timer_disarm(&tmr->timer);

  result = self->getWifiValue(self, WIFI_INFO_PROVISIONING_PORT, DATA_VIEW_FIELD_UINT8_T, &numericValue, &stringValue);
//ets_printf("port: %d!%p!%d!\n", numericValue, stringValue, result);
//  checkErrOK(result);
  port = numericValue;
//  result = self->getWifiValue(self, WIFI_INFO_PROVISIONING_IP_ADDR, DATA_VIEW_FIELD_UINT8_VECTOR, &numericValue, &stringValue);
//  checkErrOK(result);

  nud = (netsocketUserData_t *)os_zalloc(sizeof(netsocketUserData_t));
//   checkAllocOK(nud);
//ets_printf("nud0: %p\n", nud);
//  checkAllocgLOK(nud->urls);
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
ets_printf("port: %d\n", port);

//ets_printf("call regist connectcb\n");
    result = espconn_regist_connectcb(pesp_conn, serverConnected);
    if (result != COMP_DISP_ERR_OK) {
//      return COMP_DISP_ERR_REGIST_CONNECT_CB;
    }
//ets_printf("regist connectcb result: %d\n", result);
    result = espconn_accept(pesp_conn);    // if it's a server, no need to dns.
    if (result != COMP_DISP_ERR_OK) {
//      return COMP_DISP_ERR_TCP_ACCEPT;
    }
//ets_printf("regist_accept result: %d\n", result);
    result =espconn_regist_time(pesp_conn, tcp_server_timeover, 0);
    if (result != COMP_DISP_ERR_OK) {
//      return COMP_DISP_ERR_REGIST_TIME;
    }
//ets_printf("regist_time result: %d\n", result);
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

  self->netsocketRunClientMode = &netsocketRunClientMode;
  self->netsocketSendData = netsocketSendData;
  return COMP_DISP_ERR_OK;
}
