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
 * File:   compMsgNetSocket.c
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

typedef void (* netSocketBinaryReceived_t)(void *arg, void *sud, char *pdata, unsigned short len);
typedef void (* netSocketTextReceived_t)(void *arg, void *sud, char *pdata, unsigned short len);

static int cnt = 0;
static ip_addr_t host_ip; // for dns
static int dns_reconn_count = 0;

// ============================ netSocketSendData =======================

/**
 * \brief send message data to a http scoket
 * \param sud The net (http) socket user data
 * \param payload The message data
 * \param size The number of characters of the message
 * \return Error code or ErrorOK
 *
 */
static uint8_t netSocketSendData(socketUserData_t *sud, const char *payload, int size)
{
  uint8_t result;

ets_printf("§netSocketSendData: size: %d§", size);
#ifdef CLIENT_SSL_ENABLE
  if (sud->secure) {
    result = espconn_secure_sent(sud->pesp_conn, (unsigned char *)payload, size);
  } else
#endif
  {
    result = espconn_sent(sud->pesp_conn, (unsigned char *)payload, size);
  }

ets_printf("§netSocketSendData: result: %d§", result);
  checkErrOK(result);
  return NETSOCKET_ERR_OK;
}

// ================================= netSocketRecv ====================================

static uint8_t netSocketRecv(char *payload, socketUserData_t *sud, char **data, int *lgth) {
  char *key = NULL;
  int idx;
  int found;

  idx = 0;
ets_printf("§netSocketRecv:remote_port: %d§\n", sud->remote_port);

//FIXME need to handle data!!
//  sud->netSocketReceived(sud->compMsgDispatcher, sud, data, lgth);
//  netSocketParse(string, os_strlen(string), data, lgth, sud);
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
  socketUserData_t *sud;
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

  sud = (socketUserData_t *)pesp_conn->reverse;
  sud->remote_ip[0] = pesp_conn->proto.tcp->remote_ip[0];
  sud->remote_ip[1] = pesp_conn->proto.tcp->remote_ip[1];
  sud->remote_ip[2] = pesp_conn->proto.tcp->remote_ip[2];
  sud->remote_ip[3] = pesp_conn->proto.tcp->remote_ip[3];
  sud->remote_port = pesp_conn->proto.tcp->remote_port;
//ets_printf("==received remote_port: %d\n", sud->remote_port);

  result = netSocketRecv(pdata, sud, &data, &lgth);
//  checkErrOK(gL,result,"netSocketRecv");
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
  socketUserData_t *sud;
  int result;

ets_printf("§netDelete§");
  pesp_conn = (struct espconn *)arg;
  if (pesp_conn == NULL) {
    return;
  }
  sud = (socketUserData_t *)pesp_conn->reverse;
  if(sud == NULL) {
    return;
  }
  if(sud->pesp_conn) {     // for client connected to tcp server, this should set NULL in disconnect cb
    sud->pesp_conn->reverse = NULL;
    if (sud->pesp_conn->proto.tcp) {
      os_free (sud->pesp_conn->proto.tcp);
      sud->pesp_conn->proto.tcp = NULL;
    }
    os_free (sud->pesp_conn);
    sud->pesp_conn = NULL;
  }
}

// ================================= netSocketDisconnected  ====================================

static void netSocketDisconnected(void *arg) {  // tcp only
  struct espconn *pesp_conn;
  socketUserData_t *sud;
  int result;

ets_printf("§netSocketDisconnected is called§");
  pesp_conn = (struct espconn *)arg;
  if (pesp_conn == NULL) {
    return;
  }
  sud = (socketUserData_t *)pesp_conn->reverse;
  if(sud == NULL) {
    return;
  }
  switch (sud->connectionType) {
  case NET_SOCKET_TYPE_CLIENT:
    sud->compMsgDispatcher->runningModeFlags &= ~COMP_DISP_RUNNING_MODE_CLIENT;
    break;
  case NET_SOCKET_TYPE_SOCKET:
    sud->compMsgDispatcher->runningModeFlags &= ~COMP_DISP_RUNNING_MODE_CLOUD;
    break;
  default:
ets_printf("§netSocketDisconnected bad connectionType: 0x%02x§", sud->connectionType);
    break;
  }
  if (pesp_conn->proto.tcp) {
    os_free (pesp_conn->proto.tcp);
  }
  pesp_conn->proto.tcp = NULL;
  if (sud->pesp_conn) {
    os_free(sud->pesp_conn);
  }
  sud->pesp_conn = NULL;  // espconn is already disconnected
}

// ================================= netSocketReconnected  ====================================

static void netSocketReconnected (void *arg, sint8_t err) {
ets_printf("§net_socket_reconnected is called err: %d§", err);
  netSocketDisconnected (arg);
}

// ================================= netSocketReceived  ====================================

static void netSocketReceived(void *arg, char *pdata, unsigned short len) {
  struct espconn *pesp_conn;
  socketUserData_t *sud;
  int result;

  pesp_conn = (struct espconn *)arg;
ets_printf("§netSocketReceived is called. %d %s§", len, pdata);
ets_printf("§pesp_conn: %p§", pesp_conn);
  if (pesp_conn == NULL) {
    return;
  }
  sud = (socketUserData_t *)pesp_conn->reverse;
ets_printf("§sud: %p§", sud);
  if(sud == NULL) {
    return;
  }
ets_printf("§call httpParse§");
  result = sud->compMsgDispatcher->compMsgHttp->httpParse(sud, pdata, len);
  if (result != NETSOCKET_ERR_OK) {
    // FIXME
    // add error to ErrorMain or ErrorSub list
  }
}

// ================================= netSocketSent  ====================================

static void netSocketSent(void *arg) {
  struct espconn *pesp_conn;
  socketUserData_t *sud;
  int result;
  bool boolResult;

ets_printf("§netSocketSent is called§");
  pesp_conn = (struct espconn *)arg;
  if (pesp_conn == NULL) {
    return;
  }
  sud = (socketUserData_t *)pesp_conn->reverse;
  if(sud == NULL) {
    return;
  }
}

// ================================= netSocketConnected  ====================================

static void netSocketConnected(void *arg) {
  struct espconn *pesp_conn;
  socketUserData_t *sud;
  int result;

//ets_printf("§netSocketConnected§\n");
  pesp_conn = arg;
  if (pesp_conn == NULL) {
    return;
  }
  sud = (socketUserData_t *)pesp_conn->reverse;
  if(sud == NULL) {
    return;
  }
  // can receive and send data
  result = espconn_regist_recvcb (pesp_conn, netSocketReceived);
if (result != COMP_MSG_ERR_OK) {
ets_printf("§espconn_regist_recvcb: result: %d§\n", result);
}
  result = espconn_regist_sentcb (pesp_conn, netSocketSent);
if (result != COMP_MSG_ERR_OK) {
ets_printf("§espconn_regist_sentcb: result: %d§\n", result);
}
  result = espconn_regist_disconcb (pesp_conn, netSocketDisconnected);
if (result != COMP_MSG_ERR_OK) {
ets_printf("§espconn_regist_disconcb: result: %d§\n", result);
}
  sud->compMsgDispatcher->compMsgData->sud = sud;
//ets_printf("§startSendMsg2: %p§\n", sud->compMsgDispatcher->startSendMsg2);
  sud->compMsgDispatcher->runningModeFlags |= COMP_DISP_RUNNING_MODE_CLOUD;
  if (sud->compMsgDispatcher->compMsgSendReceive->startSendMsg2 != NULL) {
    result = sud->compMsgDispatcher->compMsgSendReceive->startSendMsg2(sud->compMsgDispatcher);
  } else {
ets_printf("§sud->compMsgDispatcher->startSendMsg2 is NULL§");
  }
}

// ================================= socketConnect  ====================================

static void socketConnect(void *arg) {
  struct espconn *pesp_conn;
  socketUserData_t *sud;
  int result;

//ets_printf("§socketConnect§");
  pesp_conn = arg;
  if (pesp_conn == NULL) {
    return;
  }
  sud = (socketUserData_t *)pesp_conn->reverse;
  if(sud == NULL) {
    return;
  }
#ifdef CLIENT_SSL_ENABLE
  if (sud->secure){
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
  socketUserData_t *sud;
  int result;
  int i;

//ets_printf("§serverConnected: arg: %p§", arg);
  pesp_conn = arg;
  sud = (socketUserData_t *)pesp_conn->reverse;
  if(sud == NULL) {
    return;
  }
  for(i = 0; i < MAX_SOCKET; i++) {
    if (socket[i] == NULL) { // found empty slot
      break;
    }
  }
  if(i>=MAX_SOCKET) {// can't create more socket
#ifdef CLIENT_SSL_ENABLE
    if (sud->secure) {
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
//    checkErrOK(gL, NETSOCKET_ERR_MAX_SOCKET_REACHED, "netSocketServerConnected");
    pesp_conn->reverse = NULL;    // not accept this conn
    return;
  }
//ets_printf("registstart\n");
  result = espconn_regist_recvcb(pesp_conn, socketReceived);
  if (result != COMP_MSG_ERR_OK) {
ets_printf("§regist socketReceived err: %d§", result);
  }
  result = espconn_regist_sentcb(pesp_conn, socketSent);
  if (result != COMP_MSG_ERR_OK) {
ets_printf("§regist socketSent err: %d§", result);
  }
  result = espconn_regist_disconcb(pesp_conn, serverDisconnected);
  if (result != COMP_MSG_ERR_OK) {
ets_printf("§regist serverDisconnected err: %d§", result);
  }
  result = espconn_regist_reconcb(pesp_conn, serverReconnected);
  if (result != COMP_MSG_ERR_OK) {
ets_printf("§regist serverReconnected err: %d§", result);
  }
}

// ================================= netSocketStart ====================================

static void netSocketStart(void *arg) {
  struct espconn *pesp_conn;
  socketUserData_t *sud;
  int result;

ets_printf("§netSocketStart§");
  if (pesp_conn == NULL) {
    return;
  }
  sud = (socketUserData_t *)pesp_conn->reverse;
  if(sud == NULL) {
    return;
  }

}

// ================================= socketDnsFound ====================================

static void socketDnsFound(const char *name, ip_addr_t *ipaddr, void *arg) {
//ets_printf("§socket_dns_found is called§");
  struct espconn *pesp_conn = arg;
  socketUserData_t *sud;
  if (pesp_conn == NULL) {
    ets_printf("§pesp_conn null§");
    return;
  }
  sud = (socketUserData_t *)pesp_conn->reverse;
  if (sud == NULL) {
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
  socketUserData_t *sud;

  pesp_conn = NULL;

  sud = (socketUserData_t *)os_zalloc(sizeof(socketUserData_t));
//   checkAllocOK(sud);
//ets_printf("§sud0: %p§", sud);
//  checkAllocgLOK(sud->urls);
  sud->maxHttpMsgInfos = 5;
  sud->numHttpMsgInfos = 0;
  sud->httpMsgInfos = os_zalloc(sud->maxHttpMsgInfos * sizeof(httpMsgInfo_t));
  sud->connectionType = NET_SOCKET_TYPE_SOCKET;
#ifdef CLIENT_SSL_ENABLE
  result = self->compMsgWifiData->getWifiValue(self, WIFI_INFO_CLOUD_SECURE_CONNECT, DATA_VIEW_FIELD_UINT8_T, &numericValue, &stringValue);
  sud->secure = numericValue;
#endif
  result = self->compMsgWifiData->getWifiValue(self, WIFI_INFO_NET_RECEIVED_CALL_BACK, DATA_VIEW_FIELD_UINT32_T, &numericValue, &stringValue);
  sud->netSocketReceived = (netSocketReceived_t)numericValue;
  result = self->compMsgWifiData->getWifiValue(self, WIFI_INFO_NET_TO_SEND_CALL_BACK, DATA_VIEW_FIELD_UINT32_T, &numericValue, &stringValue);
  sud->netSocketToSend = (netSocketToSend_t)numericValue;
  sud->compMsgDispatcher = self;
//ets_printf("§netSocketReceived: %p netSocketToSend: %p§", sud->netSocketReceived, sud->netSocketToSend);

  result = self->compMsgWifiData->getWifiValue(self, WIFI_INFO_CLOUD_PORT, DATA_VIEW_FIELD_UINT8_T, &numericValue, &stringValue);
  port = numericValue;

//ets_printf("§port: %d§", port);
  pesp_conn = (struct espconn *)os_zalloc(sizeof(struct espconn));
  sud->pesp_conn = pesp_conn;
//  checkAllocOK(pesp_conn);

  type = ESPCONN_TCP;
  pesp_conn->type = type;
  pesp_conn->state = ESPCONN_NONE;
  // reverse is for the callback function
  pesp_conn->reverse = sud;

  pesp_conn->proto.tcp = NULL;
  pesp_conn->proto.udp = NULL;

  pesp_conn->proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));
  if(!pesp_conn->proto.tcp){
    os_free(pesp_conn);
    pesp_conn = NULL;
//    checkErrOK(COMP_MSG_ERR_OUT_OF_MEMORY);
  }
  pesp_conn->proto.tcp->remote_port = port;
  pesp_conn->proto.tcp->local_port = espconn_port();

#ifdef CLOUD_1
  result = self->compMsgWifiData->getWifiValue(self, WIFI_INFO_CLOUD_DOMAIN_1, DATA_VIEW_FIELD_UINT8_T, &numericValue, &stringValue);
#else
  result = self->compMsgWifiData->getWifiValue(self, WIFI_INFO_CLOUD_DOMAIN_2, DATA_VIEW_FIELD_UINT8_T, &numericValue, &stringValue);
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
  if (result != COMP_MSG_ERR_OK) {
//    return COMP_MSG_ERR_REGIST_CONNECT_CB;
  }
  result = espconn_regist_reconcb(pesp_conn, netSocketReconnected);
  if (result != COMP_MSG_ERR_OK) {
//    return COMP_MSG_ERR_REGIST_CONNECT_CB;
  }
  result = espconn_regist_sentcb(pesp_conn, socketSent);
  if (result != COMP_MSG_ERR_OK) {
ets_printf("§regist socketSent err: %d§", result);
  }
#ifdef CLIENT_SSL_ENABLE
//ets_printf("§socket: secure: %d§", sud->secure);
  if (sud->secure){
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
  return COMP_MSG_ERR_OK;
}

// ================================= startClientMode ====================================

static  void startClientMode(void *arg) {
  compMsgDispatcher_t *self;
  uint8_t timerId;
  int numericValue;
  uint8_t *stringValue;
  struct espconn *pesp_conn;
  unsigned port;
  ip_addr_t ipaddr;
  unsigned type;
  int result;
  bool boolResult;
  socketUserData_t *sud;
  char *hostname;
  uint8_t ap_id;
  uint8_t opmode;
  char temp[64];
  compMsgTimerSlot_t *compMsgTimerSlot;

//ets_printf("§startClientMode\n§");
  compMsgTimerSlot = (compMsgTimerSlot_t *)arg;
ets_printf("§startClientMode timerInfo:%p\n§", compMsgTimerSlot);
  compMsgTimerSlot->connectionMode = STATION_IF;
  self = compMsgTimerSlot->compMsgDispatcher;
  pesp_conn = NULL;

  result = self->compMsgSocket->checkConnectionStatus(compMsgTimerSlot);
  if (result != COMP_MSG_ERR_OK) {
    return;
  }

  self->runningModeFlags |= COMP_DISP_RUNNING_MODE_CLIENT;
  result = self->compMsgWifiData->setWifiValue(self, "@clientIPAddr", compMsgTimerSlot->ip_addr, NULL);
  result = self->compMsgWifiData->getWifiValue(self, WIFI_INFO_CLIENT_IP_ADDR, DATA_VIEW_FIELD_UINT32_T, &numericValue, &stringValue);
//ets_printf("§ip2: 0x%08x§\n", numericValue);
  c_sprintf(temp, "%d.%d.%d.%d", IP2STR(&numericValue));

  result = self->compMsgWifiData->getWifiValue(self, WIFI_INFO_CLIENT_PORT, DATA_VIEW_FIELD_UINT8_T, &numericValue, &stringValue);
  port = numericValue;
ets_printf("§IP: %s port: %d result: %d\n§", temp, port, result);

  sud = (socketUserData_t *)os_zalloc(sizeof(socketUserData_t));
//   checkAllocOK(sud);
//ets_printf("sud0: %p\n", sud);
//  checkAllocgLOK(sud->urls);
  sud->connectionType = NET_SOCKET_TYPE_CLIENT;
#ifdef CLIENT_SSL_ENABLE
  result = self->compMsgWifiData->getWifiValue(self, WIFI_INFO_CLOUD_SECURE_CONNECT, DATA_VIEW_FIELD_UINT8_T, &numericValue, &stringValue);
  sud->secure = numericValue;
#endif
  result = self->compMsgWifiData->getWifiValue(self, WIFI_INFO_NET_RECEIVED_CALL_BACK, DATA_VIEW_FIELD_UINT32_T, &numericValue, &stringValue);
//ets_printf("§netReceivedCallback: %p!%d!§\n", numericValue, result);
  sud->netSocketReceived = (netSocketReceived_t)numericValue;
  result = self->compMsgWifiData->getWifiValue(self, WIFI_INFO_NET_RECEIVED_CALL_BACK, DATA_VIEW_FIELD_UINT32_T, &numericValue, &stringValue);
//ets_printf("§netToSendCallback: %p!%d!§\n", numericValue, result);
  sud->netSocketToSend = (netSocketToSend_t)numericValue;
  sud->compMsgDispatcher = self;

  pesp_conn = (struct espconn *)os_zalloc(sizeof(struct espconn));
  sud->pesp_conn = pesp_conn;
//  checkAllocOK(pesp_conn);

  type = ESPCONN_TCP;
  pesp_conn->type = type;
  pesp_conn->state = ESPCONN_NONE;
  // reverse is for the callback function
  pesp_conn->reverse = sud;

  pesp_conn->proto.tcp = NULL;
  pesp_conn->proto.udp = NULL;

  pesp_conn->proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));
  if(!pesp_conn->proto.tcp){
    os_free(pesp_conn);
    pesp_conn = NULL;
//    checkErrOK(COMP_MSG_ERR_OUT_OF_MEMORY);
  }
  pesp_conn->proto.tcp->local_port = port;
//ets_printf("§port: %d§\n", port);

//ets_printf("§call regist connectcb§\n");
  result = espconn_regist_connectcb(pesp_conn, serverConnected);
  if (result != COMP_MSG_ERR_OK) {
//    return COMP_MSG_ERR_REGIST_CONNECT_CB;
  }
//ets_printf("§regist connectcb result: %d§\n", result);
  result = espconn_regist_recvcb(pesp_conn, socketReceived);
  if (result != COMP_MSG_ERR_OK) {
ets_printf("§regist socketReceived err: %d§", result);
  }
  result = espconn_regist_sentcb(pesp_conn, socketSent);
  if (result != COMP_MSG_ERR_OK) {
ets_printf("§regist socketSent err: %d§", result);
  }
  result = espconn_accept(pesp_conn);
  if (result != COMP_MSG_ERR_OK) {
//    return COMP_MSG_ERR_TCP_ACCEPT;
ets_printf("§regist_accept err result: %d§", result);
  }
  result = espconn_regist_time(pesp_conn, tcp_server_timeover, 0);
  if (result != COMP_MSG_ERR_OK) {
//    return COMP_MSG_ERR_REGIST_TIME;
ets_printf("§regist_time err result: %d§", result);
  }
  if (self->compMsgSendReceive->startSendMsg != NULL) {
ets_printf("§call startSendMsg: %p\n§", self->compMsgSendReceive->startSendMsg);
    result = self->compMsgSendReceive->startSendMsg(self);
//ets_printf("§startSendMsg result: %d§", result);
  }
}

// ================================= netSocketRunClientMode ====================================

static uint8_t netSocketRunClientMode(compMsgDispatcher_t *self) {
  int result;
  bool boolResult;
  struct station_config station_config;
  int numericValue;
  uint8_t *stringValue;
  uint8_t opmode;

ets_printf("§netSocketRunClientMode called\n§");
  
  opmode = wifi_get_opmode();
//ets_printf("§opmode: %d§", opmode);
  boolResult = wifi_station_disconnect();
//ets_printf("§wifi_station_disconnect: boolResult: %d§", boolResult);
  c_memset(station_config.ssid,0,sizeof(station_config.ssid));
  result = self->compMsgWifiData->getWifiValue(self, WIFI_INFO_CLIENT_SSID, DATA_VIEW_FIELD_UINT8_VECTOR, &numericValue, &stringValue);
//ets_printf("§getSsid: result: %d§\n", result);
  checkErrOK(result);
  c_memcpy(station_config.ssid, stringValue, c_strlen(stringValue));

  c_memset(station_config.password,0,sizeof(station_config.password));
  result = self->compMsgWifiData->getWifiValue(self, WIFI_INFO_CLIENT_PASSWD, DATA_VIEW_FIELD_UINT8_VECTOR, &numericValue, &stringValue);
//ets_printf("§getPasswd: result: %d§\n", result);
  checkErrOK(result);
//ets_printf("len password: %d\n", c_strlen(stringValue));
  c_memcpy(station_config.password, stringValue, c_strlen(stringValue));
ets_printf("§netSocketRunClientMode: ssid: %s password: %s!\n§", station_config.ssid, station_config.password);

  boolResult = wifi_station_set_config(&station_config);
  if (!boolResult) {
    return COMP_MSG_ERR_CANNOT_SET_OPMODE;
  }
//  wifi_station_set_auto_connect(true);
  boolResult = wifi_station_connect();
  if (!boolResult) {
    return COMP_MSG_ERR_CANNOT_CONNECT;
  }
  boolResult = wifi_station_set_hostname("testDeviceClient");
ets_printf("§wifi is in mode: %d status: %d hostname: %s!\n§", wifi_get_opmode(), wifi_station_get_connect_status(), wifi_station_get_hostname());

  return self->compMsgSocket->startConnectionTimer(self, 2, self->compMsgSocket->startClientMode);
}

// ================================= netSocketStartCloudSocket ====================================

static uint8_t netSocketStartCloudSocket (compMsgDispatcher_t *self) {
  int result;

//ets_printf("§netSocketStartCloudSocket called§");
  self->compMsgSendReceive->startSendMsg = NULL;
  result = openCloudSocket( self);
  checkErrOK(result);
  return COMP_MSG_ERR_OK;
}

// ================================= compMsgNetSocketInit ====================================

uint8_t compMsgNetSocketInit(compMsgDispatcher_t *self) {
  uint8_t result;

  self->compMsgSocket->netSocketStartCloudSocket = &netSocketStartCloudSocket;
  self->compMsgSocket->netSocketRunClientMode = &netSocketRunClientMode;
  self->compMsgSocket->netSocketSendData = netSocketSendData;
  self->compMsgSocket->startClientMode = &startClientMode;
  return COMP_MSG_ERR_OK;
}
