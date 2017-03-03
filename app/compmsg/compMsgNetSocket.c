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

#define MAX_CONNECTIONS_ALLOWED 4
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

static const char * ssdp_notify_1 = "NOTIFY * HTTP/1.1\r\n\
HOST: 239.255.255.250:1900\r\n\
CACHE-CONTROL: max-age=100\r\n\
NT: upnp:kfeetest\r\n\
USN: uuid:c5baf4a1-0c8e-44da-9714-ef01234";
static const char * ssdp_notify_3 = "::upnp:rootdevice\r\nNTS: ssdp:alive\r\n\
SERVER: NodeMCU/20170227 UPnP/1.1 ovoi/0.1\r\n\
Location: http://";
static const char * ssdp_notify_5 = "/kfee_testdevice_Connect.xml\r\n\r\n";

typedef void (* netSocketBinaryReceived_t)(void *arg, void *sud, char *pdata, unsigned short len);
typedef void (* netSocketTextReceived_t)(void *arg, void *sud, char *pdata, unsigned short len);

static int cnt = 0;
static ip_addr_t host_ip; // for dns
static int dns_reconn_count = 0;
static uint16_t tcp_server_timeover = 30;
static uint16_t udp_server_timeover = 30;

// ============================ netSocketSendData =======================

/**
 * \brief send message data to a http scoket
 * \param sud The net (http) socket user data
 * \param payload The message data
 * \param size The number of characters of the message
 * \return Error code or ErrorOK
 *
 */
static uint8_t netSocketSendData(socketUserData_t *sud, const char *payload, int size) {
  uint8_t result;
  compMsgDispatcher_t *self;

  self = sud->compMsgDispatcher;
  COMP_MSG_DBG(self, "N", 1, "netSocketSendData: size: %d %s", size, payload);
#ifdef CLIENT_SSL_ENABLE
  if (sud->secure) {
    result = espconn_secure_sent(sud->pesp_conn, (unsigned char *)payload, size);
  } else
#endif
  {
ets_printf("sud: %p espconn: %p\n", sud, sud->pesp_conn);
    result = espconn_sent(sud->pesp_conn, (unsigned char *)payload, size);
  }

  COMP_MSG_DBG(self, "N", 1, "netSocketSendData: espconn_sent: result: %d", result);
  checkErrOK(result);
  return NETSOCKET_ERR_OK;
}

// ================================= netSocketRecv ====================================

static uint8_t netSocketRecv(char *payload, int payloadLgth, socketUserData_t *sud, char **data, int *lgth) {
  char *key = NULL;
  uint8_t result;
  int idx;
  int found;
  compMsgDispatcher_t *self;

  self = sud->compMsgDispatcher;
  idx = 0;
  COMP_MSG_DBG(self, "N", 1, "netSocketRecv:remote_port: %d\n", sud->remote_port);

//FIXME need to handle data!!
  COMP_MSG_DBG(self, "N", 2, "netSocketRecv:call compMsgHttp->httpParse\n");
  if (sud->httpMsgInfos == NULL) {
    sud->maxHttpMsgInfos = 5;
    sud->numHttpMsgInfos = 0;
    sud->httpMsgInfos = os_zalloc(sud->maxHttpMsgInfos * sizeof(httpMsgInfo_t));
    sud->connectionType = NET_SOCKET_TYPE_CLIENT;
  }
  result = sud->compMsgDispatcher->compMsgHttp->httpParse(sud, payload, payloadLgth);

//  COMP_MSG_DBG(self, "N", 1, "netSocketRecv:call sud->netSocketReceived\n");
//  sud->netSocketReceived(sud->compMsgDispatcher, sud, payload, payloadLgth);
//  netSocketParse(string, os_strlen(string), data, lgth, sud);
  return NETSOCKET_ERR_OK;
}

// ================================= serverDisconnected  ====================================

static void serverDisconnected(void *arg) {
  struct espconn *pesp_conn;
  compMsgDispatcher_t *self;
  socketUserData_t *sud;

  pesp_conn = (struct espconn *)arg;
  if (pesp_conn == NULL) {
ets_printf("serverDisconnected pesp_conn == NULL\n");
    return;
  }
  sud = (socketUserData_t *)pesp_conn->reverse;
  if(sud == NULL) {
ets_printf("serverDisconnected sud == NULL\n");
    return;
  }
  self = sud->compMsgDispatcher;
  if(self == NULL) {
ets_printf("serverDisconnected self == NULL\n");
    return;
  }
  COMP_MSG_DBG(self, "N", 1, "serverDisconnected: arg: %p\n", arg);
}

// ================================= serverReconnected  ====================================

static void serverReconnected(void *arg, int8_t err) {
  struct espconn *pesp_conn;
  compMsgDispatcher_t *self;
  socketUserData_t *sud;

  pesp_conn = (struct espconn *)arg;
  if (pesp_conn == NULL) {
ets_printf("serverReconnected pesp_conn == NULL\n");
    return;
  }
  sud = (socketUserData_t *)pesp_conn->reverse;
  if(sud == NULL) {
ets_printf("serverReconnected sud == NULL\n");
    return;
  }
  self = sud->compMsgDispatcher;
  if(self == NULL) {
ets_printf("serverReconnected self == NULL\n");
    return;
  }
  COMP_MSG_DBG(self, "N", 1, "serverReconnected: arg: %p\n", arg);
}

// ================================= socketReceived  ====================================

static void socketReceived(void *arg, char *pdata, unsigned short len) {
  struct espconn *pesp_conn;
  socketUserData_t *sud;
  char *data = "";
  int lgth = 0;
  int result;
  char temp[20] = {0};
  compMsgDispatcher_t *self;

  pesp_conn = (struct espconn *)arg;
  if (pesp_conn == NULL) {
ets_printf("socketReceived pesp_conn == NULL\n");
    return;
  }
  sud = (socketUserData_t *)pesp_conn->reverse;
  if(sud == NULL) {
ets_printf("socketReceived sud == NULL\n");
    return;
  }
  self = sud->compMsgDispatcher;
  if(self == NULL) {
ets_printf("socketReceived self == NULL\n");
    return;
  }
  COMP_MSG_DBG(self, "N", 2, "socketReceived: arg: %p len: %d\n", arg, len);
  COMP_MSG_DBG(self, "N", 1, "socketReceived: arg: %p pdata: %s len: %d", arg, pdata, len);
  c_sprintf(temp, IPSTR, IP2STR( &(pesp_conn->proto.tcp->remote_ip) ) );
  COMP_MSG_DBG(self, "N", 2, "remote %s:%d received\n", temp, pesp_conn->proto.tcp->remote_port);
ets_printf(">>>socketReceived: sud->secure: %d\n", sud->secure);
  sud->remote_ip[0] = pesp_conn->proto.tcp->remote_ip[0];
  sud->remote_ip[1] = pesp_conn->proto.tcp->remote_ip[1];
  sud->remote_ip[2] = pesp_conn->proto.tcp->remote_ip[2];
  sud->remote_ip[3] = pesp_conn->proto.tcp->remote_ip[3];
  sud->remote_port = pesp_conn->proto.tcp->remote_port;
  COMP_MSG_DBG(self, "N", 2, "==received remote_port: %d\n", sud->remote_port);
  result = netSocketRecv(pdata, len, sud, &data, &lgth);
COMP_MSG_DBG(self, "Y", 1, "==received remote_port: %d result: %d\n", sud->remote_port, result);
// for testing!!
//result = self->compMsgAction->startLightSleepWakeupMode(self);
//COMP_MSG_DBG(self, "Y", 1, "==startLightSleepWakeupMode result: %d\n", result);

//  checkErrOK(gL,result,"netSocketRecv");
}

// ================================= socketSent  ====================================

static void socketSent(void *arg) {
  struct espconn *pesp_conn;
  socketUserData_t *sud;
  compMsgDispatcher_t *self;

ets_printf("socketSent: %p\n", arg);
  pesp_conn = (struct espconn *)arg;
  if (pesp_conn == NULL) {
ets_printf("socketSent pesp_conn == NULL\n");
    return;
  }
  sud = (socketUserData_t *)pesp_conn->reverse;
  if (sud == NULL) {
ets_printf("socketSent sud == NULL\n");
    return;
  }
ets_printf("socketSent2: sud: %p\n", sud);
  self = sud->compMsgDispatcher;
  if (self == NULL) {
ets_printf("socketSent self == NULL\n");
    return;
  }
ets_printf("socketSent3: self: %p\n",  self);
  COMP_MSG_DBG(self, "N", 1, "socketSent: arg: %p", arg);

}

// ================================= netDelete  ====================================

static void netDelete(void *arg) {
  struct espconn *pesp_conn;
  socketUserData_t *sud;
  compMsgDispatcher_t *self;
  int result;

  COMP_MSG_DBG(self, "N", 1, "netDelete");
  pesp_conn = (struct espconn *)arg;
  if (pesp_conn == NULL) {
ets_printf("serverDisconnected pesp_conn == NULL\n");
    return;
  }
  sud = (socketUserData_t *)pesp_conn->reverse;
  if(sud == NULL) {
ets_printf("serverDisconnected sud == NULL\n");
    return;
  }
  self = sud->compMsgDispatcher;
  if(self == NULL) {
ets_printf("netDelete self == NULL\n");
    return;
  }
  if(sud->pesp_conn) {     // for client connected to tcp server, this should set NULL in disconnect cb
    sud->pesp_conn->reverse = NULL;
    if (sud->pesp_conn->proto.tcp) {
      os_free (sud->pesp_conn->proto.tcp);
      sud->pesp_conn->proto.tcp = NULL;
    } else {
      if(sud->pesp_conn->type == ESPCONN_UDP){
        if(sud->pesp_conn->proto.udp)
          c_free(sud->pesp_conn->proto.udp);
        sud->pesp_conn->proto.udp = NULL;
      }
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
  compMsgDispatcher_t *self;

  pesp_conn = (struct espconn *)arg;
  if (pesp_conn == NULL) {
ets_printf("netSocketverDisconnected pesp_conn == NULL\n");
    return;
  }
  sud = (socketUserData_t *)pesp_conn->reverse;
  if(sud == NULL) {
ets_printf("netSocketDisconnected sud == NULL\n");
    return;
  }
  self = sud->compMsgDispatcher;
  if(self == NULL) {
ets_printf("netSocketDisconnected self == NULL\n");
    return;
  }
  COMP_MSG_DBG(self, "N", 1, "netSocketDisconnected is called");
  switch (sud->connectionType) {
  case NET_SOCKET_TYPE_CLIENT:
    sud->compMsgDispatcher->runningModeFlags &= ~COMP_DISP_RUNNING_MODE_CLIENT;
    break;
  case NET_SOCKET_TYPE_SOCKET:
    sud->compMsgDispatcher->runningModeFlags &= ~COMP_DISP_RUNNING_MODE_CLOUD;
    break;
  case NET_SOCKET_TYPE_SSDP:
    sud->compMsgDispatcher->runningModeFlags &= ~COMP_DISP_RUNNING_MODE_SSDP;
    break;
  default:
    COMP_MSG_DBG(self, "N", 1, "netSocketDisconnected bad connectionType: 0x%02x", sud->connectionType);
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
  struct espconn *pesp_conn;
  compMsgDispatcher_t *self;
  socketUserData_t *sud;

  pesp_conn = (struct espconn *)arg;
  if(pesp_conn == NULL) {
ets_printf("netSocketReconnected pesp_conn == NULL\n");
    return;
  }
  sud = (socketUserData_t *)pesp_conn->reverse;
  if(sud == NULL) {
ets_printf("netSocketReconnected sud == NULL\n");
    return;
  }
  self = sud->compMsgDispatcher;
  if(self == NULL) {
ets_printf("netSocketReconnected self == NULL\n");
    return;
  }
  COMP_MSG_DBG(self, "N", 1, "net_socket_reconnected is called err: %d", err);
  netSocketDisconnected (arg);
}

// ================================= netSocketReceived  ====================================

static void netSocketReceived(void *arg, char *pdata, unsigned short len) {
  struct espconn *pesp_conn;
  socketUserData_t *sud;
  int result;
  compMsgDispatcher_t *self;

  pesp_conn = (struct espconn *)arg;
  if (pesp_conn == NULL) {
ets_printf("netSocketReceived pesp_conn == NULL\n");
    return;
  }
  sud = (socketUserData_t *)pesp_conn->reverse;
  if(sud == NULL) {
ets_printf("netSocketReceived sud == NULL\n");
    return;
  }
  self = sud->compMsgDispatcher;
  if(self == NULL) {
ets_printf("netSocketReceived self == NULL\n");
    return;
  }
  COMP_MSG_DBG(self, "N", 1, "sud: %p", sud);
  if(sud == NULL) {
    return;
  }
  COMP_MSG_DBG(self, "N", 1, "netSocketReceived is called. %d %s", len, pdata);
  COMP_MSG_DBG(self, "N", 1, "pesp_conn: %p", pesp_conn);
  COMP_MSG_DBG(self, "N", 1, "call httpParse httpMsgInfos: %p num: %d max: %d\n", sud->httpMsgInfos, sud->numHttpMsgInfos, sud->maxHttpMsgInfos);
  if (sud->httpMsgInfos == NULL) {
    sud->maxHttpMsgInfos = 5;
    sud->numHttpMsgInfos = 0;
    sud->httpMsgInfos = os_zalloc(sud->maxHttpMsgInfos * sizeof(httpMsgInfo_t));
    sud->connectionType = NET_SOCKET_TYPE_CLIENT;
  }
  result = sud->compMsgDispatcher->compMsgHttp->httpParse(sud, pdata, len);
  if (result != NETSOCKET_ERR_OK) {
    // FIXME
    // add error to ErrorMain or ErrorSub list
  }
}

static uint8_t openSSDPSocket(compMsgDispatcher_t *self, socketUserData_t **sudOut);

// ================================= netSocketSendSSDPInfo  ====================================

static uint8_t netSocketSendSSDInfo(socketUserData_t *sud) {
  uint8_t result;
  char buf[1024];
  char ssdp_notify_2[50];
  char ssdp_notify_4[50];
  struct ip_info pTempIp;
  char temp[64];

  wifi_get_ip_info(STATION_IF, &pTempIp);
  if(pTempIp.ip.addr==0){
  }
  c_sprintf(temp, "%d.%d.%d.%d", IP2STR(&pTempIp.ip) );
  ets_sprintf(ssdp_notify_2, "%x", system_get_chip_id());
  ets_sprintf(ssdp_notify_4, "%s", temp);
ets_printf("free heap 5: 0x%08x\n", system_get_free_heap_size());
  ets_sprintf(buf, "%s%s%s%s%s", ssdp_notify_1, ssdp_notify_2, ssdp_notify_3, ssdp_notify_4, ssdp_notify_5);
  result = netSocketSendData(sud, buf, strlen(buf));
ets_printf("netSocketSendData result: %d heap6: 0x%08x\n", result, system_get_free_heap_size());
  return COMP_MSG_ERR_OK;
}

// ================================= netSocketSSDPReceived  ====================================

static void netSocketSSDPReceived(void *arg, char *pdata, unsigned short len) {
  struct espconn *pesp_conn;
  socketUserData_t *sud;
  int result;
  compMsgDispatcher_t *self;

  pesp_conn = (struct espconn *)arg;
  if (pesp_conn == NULL) {
ets_printf("netSocketSSDPReceived pesp_conn == NULL\n");
    return;
  }
  sud = (socketUserData_t *)pesp_conn->reverse;
  if(sud == NULL) {
ets_printf("netSocketSSDPReceived sud == NULL\n");
    return;
  }
  self = sud->compMsgDispatcher;
  if(self == NULL) {
ets_printf("netSocketSSDPReceived self == NULL\n");
    return;
  }
  COMP_MSG_DBG(self, "N", 2, "netSocketSSDPReceived sud: %p", sud);
  if(sud == NULL) {
    return;
  }
  // we are only interested in M-SEARCH requests!!
  if (c_strncmp(pdata, "M-SEARCH", 8) != 0) {
    return;
  }
  COMP_MSG_DBG(self, "N", 1, "netSSDPSocketReceived is called. %d %s", len, pdata);
  COMP_MSG_DBG(self, "N", 1, "pesp_conn: %p", pesp_conn);

#ifdef NOTDEF
  COMP_MSG_DBG(self, "N", 1, "call httpParse httpMsgInfos: %p num: %d max: %d\n", sud->httpMsgInfos, sud->numHttpMsgInfos, sud->maxHttpMsgInfos);
  if (sud->httpMsgInfos == NULL) {
    sud->maxHttpMsgInfos = 5;
    sud->numHttpMsgInfos = 0;
    sud->httpMsgInfos = os_zalloc(sud->maxHttpMsgInfos * sizeof(httpMsgInfo_t));
    sud->connectionType = NET_SOCKET_TYPE_CLIENT;
  }
  result = sud->compMsgDispatcher->compMsgHttp->httpParse(sud, pdata, len);
  if (result != NETSOCKET_ERR_OK) {
    // FIXME
    // add error to ErrorMain or ErrorSub list
  }
#endif
  result = netSocketSendSSDInfo(sud);
ets_printf("netSocketSendSSDInfo: result: %d\n", result);
  return;
}


// ================================= netSocketSent  ====================================

static void netSocketSent(void *arg) {
  struct espconn *pesp_conn;
  socketUserData_t *sud;
  compMsgDispatcher_t *self;
  int result;
  bool boolResult;

  pesp_conn = (struct espconn *)arg;
  if (pesp_conn == NULL) {
ets_printf("netSocketSent pesp_conn == NULL\n");
    return;
  }
  sud = (socketUserData_t *)pesp_conn->reverse;
  if(sud == NULL) {
ets_printf("netSocketSent sud == NULL\n");
    return;
  }
  self = sud->compMsgDispatcher;
  if(self == NULL) {
ets_printf("netSocketSent self == NULL\n");
    return;
  }
  if (sud->payloadBuf != NULL) {
ets_printf("netSocketSent: free payloadBuf: %p\n", sud->payloadBuf);
    sud->payloadBuf = NULL;
  }
  COMP_MSG_DBG(self, "N", 1, "netSocketSent is called");
}

// ================================= netSocketConnected  ====================================

static void netSocketConnected(void *arg) {
  struct espconn *pesp_conn;
  socketUserData_t *sud;
  compMsgDispatcher_t *self;
  int result;

  pesp_conn = arg;
  if (pesp_conn == NULL) {
ets_printf("netSocketConnected pesp_conn == NULL\n");
    return;
  }
  sud = (socketUserData_t *)pesp_conn->reverse;
  if(sud == NULL) {
ets_printf("netSocketConnected sud == NULL\n");
    return;
  }
  self = sud->compMsgDispatcher;
  if(self == NULL) {
ets_printf("netSocketConnected self == NULL\n");
    return;
  }
  COMP_MSG_DBG(self, "N", 2, "netSocketConnected\n");
  // can receive and send data
  result = espconn_regist_recvcb (pesp_conn, netSocketReceived);
if (result != COMP_MSG_ERR_OK) {
  COMP_MSG_DBG(self, "N", 1, "espconn_regist_recvcb: result: %d\n", result);
}
  result = espconn_regist_sentcb (pesp_conn, netSocketSent);
if (result != COMP_MSG_ERR_OK) {
  COMP_MSG_DBG(self, "N", 1, "espconn_regist_sentcb: result: %d\n", result);
}
  result = espconn_regist_disconcb (pesp_conn, netSocketDisconnected);
if (result != COMP_MSG_ERR_OK) {
  COMP_MSG_DBG(self, "N", 1, "espconn_regist_disconcb: result: %d\n", result);
}
  sud->compMsgDispatcher->compMsgData->sud = sud;
  COMP_MSG_DBG(self, "N", 2, "startSendMsg2: %p\n", sud->compMsgDispatcher->compMsgSendReceive->startSendMsg2);
  sud->compMsgDispatcher->runningModeFlags |= COMP_DISP_RUNNING_MODE_CLOUD;
  if (sud->compMsgDispatcher->compMsgSendReceive->startSendMsg2 != NULL) {
    result = sud->compMsgDispatcher->compMsgSendReceive->startSendMsg2(sud->compMsgDispatcher);
  } else {
    COMP_MSG_DBG(self, "N", 1, "sud->compMsgDispatcher->startSendMsg2 is NULL");
  }
}

// ================================= netSocketConnect  ====================================

static void netSocketConnect(void *arg) {
  struct espconn *pesp_conn;
  socketUserData_t *sud;
  compMsgDispatcher_t *self;
  int result;
  int espconn_status;

  pesp_conn = arg;
  if (pesp_conn == NULL) {
ets_printf("netSocketConnect pesp_conn == NULL\n");
    return;
  }
  sud = (socketUserData_t *)pesp_conn->reverse;
  if(sud == NULL) {
ets_printf("netSocketConnect sud == NULL\n");
    return;
  }
  self = sud->compMsgDispatcher;
  if(self == NULL) {
ets_printf("netSocketConnect self == NULL\n");
    return;
  }
  COMP_MSG_DBG(self, "N", 2, "netSocketConnect");
  if( pesp_conn->type == ESPCONN_TCP ) {
#ifdef CLIENT_SSL_ENABLE
    if (sud->secure){
      espconn_secure_set_size(ESPCONN_CLIENT, 5120); /* set SSL buffer size */
      COMP_MSG_DBG(self, "N", 2, "call espconn_secure_connect");
      espconn_status = espconn_secure_connect(pesp_conn);
      COMP_MSG_DBG(self, "N", 2, "after call espconn_secure_connect status: %d", espconn_status);
  
    } else
#endif
    {
      COMP_MSG_DBG(self, "N", 2, "netSocketConnect TCP called");
      result = espconn_connect(pesp_conn);
      COMP_MSG_DBG(self, "N", 2, "espconn_connect TCP: result: %d", result);
    }
  } else {
    if (pesp_conn->type == ESPCONN_UDP) {
      COMP_MSG_DBG(self, "N", 2, "netSocketConnect UDP called");
      result = espconn_create(pesp_conn);
      COMP_MSG_DBG(self, "N", 2, "espconn_create UPD: result: %d", result);
    } 
  } 
  COMP_MSG_DBG(self, "N", 2, "netSocketConnect done");
}

// ================================= netServerConnected  ====================================

static void netServerConnected(void *arg) {
  struct espconn *pesp_conn;
  socketUserData_t *sud;
  compMsgDispatcher_t *self;
  int result;
  int i;

  pesp_conn = arg;
  if(pesp_conn == NULL) {
ets_printf("netServerConnected pesp_conn == NULL\n");
    return;
  }
  sud = (socketUserData_t *)pesp_conn->reverse;
  if(sud == NULL) {
ets_printf("netServerConnected sud == NULL\n");
    return;
  }
  self = sud->compMsgDispatcher;
  if(self == NULL) {
ets_printf("netServerConnected self == NULL\n");
    return;
  }
  COMP_MSG_DBG(self, "N", 1, "netServerConnected: arg: %p", arg);
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
  COMP_MSG_DBG(self, "N", 2, "registstart\n");
  result = espconn_regist_recvcb(pesp_conn, socketReceived);
  if (result != COMP_MSG_ERR_OK) {
    COMP_MSG_DBG(self, "N", 1, "regist socketReceived err: %d", result);
  }
  result = espconn_regist_sentcb(pesp_conn, socketSent);
  if (result != COMP_MSG_ERR_OK) {
    COMP_MSG_DBG(self, "N", 1, "regist socketSent err: %d", result);
  }
  result = espconn_regist_disconcb(pesp_conn, serverDisconnected);
  if (result != COMP_MSG_ERR_OK) {
    COMP_MSG_DBG(self, "N", 1, "regist serverDisconnected err: %d", result);
  }
  result = espconn_regist_reconcb(pesp_conn, serverReconnected);
  if (result != COMP_MSG_ERR_OK) {
    COMP_MSG_DBG(self, "N", 1, "regist serverReconnected err: %d", result);
  }
}

// ================================= netSocketStart ====================================

static void netSocketStart(void *arg) {
  struct espconn *pesp_conn;
  socketUserData_t *sud;
  compMsgDispatcher_t *self;
  int result;

  if (pesp_conn == NULL) {
ets_printf("netSocketStart pesp_conn == NULL\n");
    return;
  }
  sud = (socketUserData_t *)pesp_conn->reverse;
  if(sud == NULL) {
ets_printf("netSocketStart sud == NULL\n");
    return;
  }
  self = sud->compMsgDispatcher;
  if(self == NULL) {
ets_printf("netSocketStart self == NULL\n");
    return;
  }
  COMP_MSG_DBG(self, "N", 1, "netSocketStart");

}

// ================================= socketDnsFound ====================================

static void socketDnsFound(const char *name, ip_addr_t *ipaddr, void *arg) {
  struct espconn *pesp_conn;
  socketUserData_t *sud;
  compMsgDispatcher_t *self;

  pesp_conn = arg;
  if (pesp_conn == NULL) {
ets_printf("socketDnsFound pesp_conn == NULL\n");
    return;
  }
  sud = (socketUserData_t *)pesp_conn->reverse;
  if (sud == NULL) {
ets_printf("socketDnsFound sud == NULL\n");
    return;
  }
  self = sud->compMsgDispatcher;
  if(self == NULL) {
ets_printf("socketDnsFound self == NULL\n");
    return;
  }
  COMP_MSG_DBG(self, "N", 2, "socket_dns_found is called");
  COMP_MSG_DBG(self, "N", 2, "ip: %p", ipaddr);
  if (ipaddr == NULL) {
    dns_reconn_count++;
    if (dns_reconn_count >= 5) {
      COMP_MSG_DBG(self, "N", 1,  "DNS Fail!" );
      return;
    }
    COMP_MSG_DBG(self, "N", 1, "DNS retry %d!", dns_reconn_count);
    host_ip.addr = 0;
    espconn_gethostbyname(pesp_conn, name, &host_ip, socketDnsFound);
    return;
  }

  // ipaddr->addr is a uint32_t ip
  if(ipaddr->addr != 0) {
    dns_reconn_count = 0;
    if( pesp_conn->type == ESPCONN_TCP ) {
      c_memcpy(pesp_conn->proto.tcp->remote_ip, &(ipaddr->addr), 4);
      COMP_MSG_DBG(self, "N", 1, "TCP ip is set: ");
      COMP_MSG_DBG(self, "N", 1, IPSTR, IP2STR(&(ipaddr->addr)));
    } else {
      if (pesp_conn->type == ESPCONN_UDP) {
        c_memcpy(pesp_conn->proto.udp->remote_ip, &(ipaddr->addr), 4);
        COMP_MSG_DBG(self, "N", 1, "UDP ip is set: ");
        COMP_MSG_DBG(self, "N", 1, IPSTR, IP2STR(&(ipaddr->addr)));
      }
    }
    netSocketConnect(pesp_conn);
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
// set err_opcode[...] here if alloc fails or eventually a variable in compMsgDispatcher!!
//   checkAllocOK(sud);
  COMP_MSG_DBG(self, "N", 2, "sud0: %p", sud);
  sud->maxHttpMsgInfos = 5;
  sud->numHttpMsgInfos = 0;
  sud->httpMsgInfos = os_zalloc(sud->maxHttpMsgInfos * sizeof(httpMsgInfo_t));
  sud->connectionType = NET_SOCKET_TYPE_SOCKET;
#ifdef CLIENT_SSL_ENABLE
  result = self->compMsgWifiData->getWifiValue(self, WIFI_INFO_CLOUD_SECURE_CONNECT, DATA_VIEW_FIELD_UINT8_T, &numericValue, &stringValue);
  sud->secure = numericValue;
#endif
  // the following 2 calls deliver a callback function address in numericValue !!
  result = self->compMsgWifiData->getWifiValue(self, WIFI_INFO_NET_RECEIVED_CALL_BACK, DATA_VIEW_FIELD_UINT32_T, &numericValue, &stringValue);
  sud->netSocketReceived = (netSocketReceived_t)numericValue;
  result = self->compMsgWifiData->getWifiValue(self, WIFI_INFO_NET_TO_SEND_CALL_BACK, DATA_VIEW_FIELD_UINT32_T, &numericValue, &stringValue);
  sud->netSocketToSend = (netSocketToSend_t)numericValue;
  sud->compMsgDispatcher = self;
  COMP_MSG_DBG(self, "N", 2, "netSocketReceived: %p netSocketToSend: %p", sud->netSocketReceived, sud->netSocketToSend);

  result = self->compMsgWifiData->getWifiValue(self, WIFI_INFO_CLOUD_PORT, DATA_VIEW_FIELD_UINT8_T, &numericValue, &stringValue);
  port = numericValue;

  COMP_MSG_DBG(self, "N", 2, "port: %d", port);
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
  result = self->compMsgWifiData->getWifiValue(self, WIFI_INFO_CLOUD_HOST_1, DATA_VIEW_FIELD_UINT8_T, &numericValue, &stringValue);
#else
  result = self->compMsgWifiData->getWifiValue(self, WIFI_INFO_CLOUD_HOST_2, DATA_VIEW_FIELD_UINT8_T, &numericValue, &stringValue);
#endif
  domain = stringValue;
  COMP_MSG_DBG(self, "N", 2, "domain: %s", domain);
  ipaddr.addr = ipaddr_addr(domain);
  c_memcpy(pesp_conn->proto.tcp->remote_ip, &ipaddr.addr, 4);
//  COMP_MSG_DBG(self, "N", 1, "TCP ip is set: ");
//  COMP_MSG_DBG(self, "N", 1, IPSTR, IP2STR(&ipaddr.addr));

  COMP_MSG_DBG(self, "N", 2, "call regist connectcb\n");
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
    COMP_MSG_DBG(self, "N", 1, "regist socketSent err: %d", result);
  }
#ifdef CLIENT_SSL_ENABLE
  COMP_MSG_DBG(self, "N", 2, "socket: secure: %d", sud->secure);
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
    COMP_MSG_DBG(self, "N", 2, "call gethostbyname: found ip for %s 0x%08x", domain, host_ip);
    socketDnsFound(domain, &host_ip, pesp_conn);  // ip is returned in host_ip.
  }
  return COMP_MSG_ERR_OK;
}

// ================================= openSSDPSocket ====================================

static uint8_t openSSDPSocket(compMsgDispatcher_t *self, socketUserData_t **sudOut) {
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
// set err_opcode[...] here if alloc fails or eventually a variable in compMsgDispatcher!!
//   checkAllocOK(sud);
  COMP_MSG_DBG(self, "N", 2, "sud0: %p", sud);
  sud->maxHttpMsgInfos = 5;
  sud->numHttpMsgInfos = 0;
  sud->httpMsgInfos = os_zalloc(sud->maxHttpMsgInfos * sizeof(httpMsgInfo_t));
  sud->connectionType = NET_SOCKET_TYPE_SSDP;
#ifdef CLIENT_SSL_ENABLE
  result = self->compMsgWifiData->getWifiValue(self, WIFI_INFO_CLOUD_SECURE_CONNECT, DATA_VIEW_FIELD_UINT8_T, &numericValue, &stringValue);
//  sud->secure = numericValue;
// here we always have to use sud->secure = 0!!
  sud->secure = 0;
#endif
  // the following 2 calls deliver a callback function address in numericValue !!
  result = self->compMsgWifiData->getWifiValue(self, WIFI_INFO_NET_SSDP_RECEIVED_CALL_BACK, DATA_VIEW_FIELD_UINT32_T, &numericValue, &stringValue);
  sud->netSocketReceived = (netSocketReceived_t)numericValue;
  result = self->compMsgWifiData->getWifiValue(self, WIFI_INFO_NET_SSDP_TO_SEND_CALL_BACK, DATA_VIEW_FIELD_UINT32_T, &numericValue, &stringValue);
  sud->netSocketToSend = (netSocketToSend_t)numericValue;
  sud->compMsgDispatcher = self;
  COMP_MSG_DBG(self, "N", 2, "netSocketSSDPReceived: %p netSocketSSDPToSend: %p", sud->netSocketReceived, sud->netSocketToSend);

  // FIXME!!!
  result = self->compMsgWifiData->getWifiValue(self, WIFI_INFO_CLOUD_PORT, DATA_VIEW_FIELD_UINT8_T, &numericValue, &stringValue);
  port = numericValue;
port = 1900;

  COMP_MSG_DBG(self, "N", 2, "port: %d", port);
  pesp_conn = (struct espconn *)os_zalloc(sizeof(struct espconn));
  sud->pesp_conn = pesp_conn;
//  checkAllocOK(pesp_conn);

  type = ESPCONN_UDP;
  pesp_conn->type = type;
  pesp_conn->state = ESPCONN_NONE;
  // reverse is for the callback function
  pesp_conn->reverse = sud;

  pesp_conn->proto.tcp = NULL;
  pesp_conn->proto.udp = NULL;

  pesp_conn->proto.udp = (esp_udp *)os_zalloc(sizeof(esp_udp));
  if(!pesp_conn->proto.udp){
    os_free(pesp_conn);
    pesp_conn = NULL;
//    checkErrOK(COMP_MSG_ERR_OUT_OF_MEMORY);
  }
  pesp_conn->proto.udp->remote_port = port;
  pesp_conn->proto.udp->local_port = espconn_port();

// FIXME!!!
  result = self->compMsgWifiData->getWifiValue(self, WIFI_INFO_CLOUD_HOST_1, DATA_VIEW_FIELD_UINT8_T, &numericValue, &stringValue);
  domain = stringValue;
domain = "239.255.255.250";
  COMP_MSG_DBG(self, "N", 2, "domain: %s", domain);
  ipaddr.addr = ipaddr_addr(domain);
  c_memcpy(pesp_conn->proto.udp->remote_ip, &ipaddr.addr, 4);
//  COMP_MSG_DBG(self, "N", 1, "TCP ip is set: ");
//  COMP_MSG_DBG(self, "N", 1, IPSTR, IP2STR(&ipaddr.addr));

  COMP_MSG_DBG(self, "N", 2, "call regist connectcb\n");
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
    COMP_MSG_DBG(self, "N", 1, "regist socketSent err: %d", result);
  }
  { 
    if(pesp_conn->proto.udp->remote_port || pesp_conn->proto.udp->local_port) {
      espconn_disconnect(pesp_conn);
    }
  }
  host_ip.addr = 0;
  dns_reconn_count = 0;
//ets_printf("look for domain: %s\n", domain);
  if (ESPCONN_OK == espconn_gethostbyname(pesp_conn, domain, &host_ip, socketDnsFound)) {
    COMP_MSG_DBG(self, "N", 2, "call gethostbyname: found ip for %s 0x%08x", domain, host_ip);
    socketDnsFound(domain, &host_ip, pesp_conn);  // ip is returned in host_ip.
  }
  *sudOut = sud;
ets_printf("openSSDSocket: sud: %p pesp_conn: %p ip: 0x%08x remote_ip: 0x%2x 0x%02x 0x%02x 0x%02x\n", sud, pesp_conn, host_ip.addr, pesp_conn->proto.udp->remote_ip[0], pesp_conn->proto.udp->remote_ip[1], pesp_conn->proto.udp->remote_ip[2], pesp_conn->proto.udp->remote_ip[3]);
  c_memcpy(pesp_conn->proto.udp->remote_ip, &host_ip.addr, 4);
//  pesp_conn->proto.udp->remote_ip = host_ip.addr;
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

  compMsgTimerSlot = (compMsgTimerSlot_t *)arg;
  self = compMsgTimerSlot->compMsgDispatcher;
  COMP_MSG_DBG(self, "N", 1, "net startClientMode\n");
  COMP_MSG_DBG(self, "N", 2, "net startClientMode timerInfo:%p\n", compMsgTimerSlot);
  compMsgTimerSlot->connectionMode = STATION_IF;
  pesp_conn = NULL;

  result = self->compMsgSocket->checkConnectionStatus(compMsgTimerSlot);
  if (result != COMP_MSG_ERR_OK) {
    return;
  }

  self->runningModeFlags |= COMP_DISP_RUNNING_MODE_CLIENT;
  result = self->compMsgWifiData->setWifiValue(self, "@clientIPAddr", compMsgTimerSlot->ip_addr, NULL);
  result = self->compMsgWifiData->getWifiValue(self, WIFI_INFO_CLIENT_IP_ADDR, DATA_VIEW_FIELD_UINT32_T, &numericValue, &stringValue);
  COMP_MSG_DBG(self, "N", 2, "ip2: 0x%08x\n", numericValue);
  c_sprintf(temp, "%d.%d.%d.%d", IP2STR(&numericValue));

  result = self->compMsgWifiData->getWifiValue(self, WIFI_INFO_CLIENT_PORT, DATA_VIEW_FIELD_UINT8_T, &numericValue, &stringValue);
  port = numericValue;
  COMP_MSG_DBG(self, "N", 1, "net startClientMode IP: %s port: %d result: %d\n", temp, port, result);

  sud = (socketUserData_t *)os_zalloc(sizeof(socketUserData_t));
//   checkAllocOK(sud);
  COMP_MSG_DBG(self, "N", 2, "sud0: %p\n", sud);
//  checkAllocgLOK(sud->urls);
  sud->connectionType = NET_SOCKET_TYPE_CLIENT;
#ifdef CLIENT_SSL_ENABLE
  result = self->compMsgWifiData->getWifiValue(self, WIFI_INFO_CLOUD_SECURE_CONNECT, DATA_VIEW_FIELD_UINT8_T, &numericValue, &stringValue);
  sud->secure = numericValue;
#endif
  result = self->compMsgWifiData->getWifiValue(self, WIFI_INFO_NET_RECEIVED_CALL_BACK, DATA_VIEW_FIELD_UINT32_T, &numericValue, &stringValue);
  COMP_MSG_DBG(self, "N", 2, "netReceivedCallback: %p!%d!\n", numericValue, result);
  sud->netSocketReceived = (netSocketReceived_t)numericValue;
  result = self->compMsgWifiData->getWifiValue(self, WIFI_INFO_NET_TO_SEND_CALL_BACK, DATA_VIEW_FIELD_UINT32_T, &numericValue, &stringValue);
  COMP_MSG_DBG(self, "N", 2, "netToSendCallback: %p!%d!\n", numericValue, result);
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
  COMP_MSG_DBG(self, "N", 2, "port: %d\n", port);

  COMP_MSG_DBG(self, "N", 2, "call regist connectcb\n");
  result = espconn_regist_connectcb(pesp_conn, netServerConnected);
  if (result != COMP_MSG_ERR_OK) {
//    return COMP_MSG_ERR_REGIST_CONNECT_CB;
  }
  COMP_MSG_DBG(self, "N", 2, "regist connectcb netServerConnected result: %d\n", result);
  result = espconn_regist_recvcb(pesp_conn, netSocketReceived);
  if (result != COMP_MSG_ERR_OK) {
    COMP_MSG_DBG(self, "N", 2, "regist netSocketReceived err: %d", result);
  }
  result = espconn_regist_sentcb(pesp_conn, netSocketSent);
  if (result != COMP_MSG_ERR_OK) {
    COMP_MSG_DBG(self, "N", 2, "regist netSocketSent err: %d", result);
  }
  result = espconn_accept(pesp_conn);
  if (result != COMP_MSG_ERR_OK) {
//    return COMP_MSG_ERR_TCP_ACCEPT;
    COMP_MSG_DBG(self, "N", 2, "regist_accept err result: %d", result);
  }
  result = espconn_regist_time(pesp_conn, tcp_server_timeover, 0);
  if (result != COMP_MSG_ERR_OK) {
//    return COMP_MSG_ERR_REGIST_TIME;
    COMP_MSG_DBG(self, "N", 2, "regist_time err result: %d", result);
  }
  // limit maximal allowed connections
  result = espconn_tcp_set_max_con_allow(pesp_conn, MAX_CONNECTIONS_ALLOWED);
  if (self->compMsgSendReceive->startSendMsg != NULL) {
    COMP_MSG_DBG(self, "N", 2, "call startSendMsg: %p\n", self->compMsgSendReceive->startSendMsg);
    result = self->compMsgSendReceive->startSendMsg(self);
    COMP_MSG_DBG(self, "N", 2, "startSendMsg result: %d", result);
  }
}

// ================================= startSSDPMode ====================================

static  void startSSDPMode(void *arg) {
  compMsgDispatcher_t *self;
  uint8_t timerId;
  int numericValue;
  uint8_t *stringValue;
  struct espconn *pesp_conn;
  unsigned port;
  ip_addr_t ipAddr;
  ip_addr_t ifAddr;
  ip_addr_t multicastAddr;
  unsigned type;
  int result;
  bool boolResult;
  socketUserData_t *sud;
  socketUserData_t *sud2;
  char *hostname;
  uint8_t ap_id;
  char *domain;
  char *domain2;
  char *domain3;
  uint8_t opmode;
  char temp[64];
  compMsgTimerSlot_t *compMsgTimerSlot;

  compMsgTimerSlot = (compMsgTimerSlot_t *)arg;
  self = compMsgTimerSlot->compMsgDispatcher;
  COMP_MSG_DBG(self, "N", 1, "net startSSDPMode\n");
  COMP_MSG_DBG(self, "N", 2, "net startSSDPMode timerInfo:%p\n", compMsgTimerSlot);
  compMsgTimerSlot->connectionMode = STATION_IF;
  pesp_conn = NULL;

  result = self->compMsgSocket->checkConnectionStatus(compMsgTimerSlot);
  if (result != COMP_MSG_ERR_OK) {
    return;
  }

  self->runningModeFlags |= COMP_DISP_RUNNING_MODE_SSDP;
//  result = self->compMsgWifiData->setWifiValue(self, "@SSDPIPAddr", compMsgTimerSlot->ip_addr, NULL);
  result = self->compMsgWifiData->getWifiValue(self, WIFI_INFO_SSDP_IP_ADDR, DATA_VIEW_FIELD_UINT32_T, &numericValue, &stringValue);
numericValue = 0xEFFFFFFA;
  COMP_MSG_DBG(self, "N", 2, "ip2: 0x%08x\n", numericValue);
  c_sprintf(temp, "%d.%d.%d.%d", IP2STR(&numericValue));

  result = self->compMsgWifiData->getWifiValue(self, WIFI_INFO_SSDP_PORT, DATA_VIEW_FIELD_UINT8_T, &numericValue, &stringValue);
  port = numericValue;
  COMP_MSG_DBG(self, "N", 1, "net startSSDPMode IP: %s port: %d result: %d\n", temp, port, result);

  sud = (socketUserData_t *)os_zalloc(sizeof(socketUserData_t));
//   checkAllocOK(sud);
  COMP_MSG_DBG(self, "N", 2, "sud0: %p\n", sud);
//  checkAllocgLOK(sud->urls);
  sud->connectionType = NET_SOCKET_TYPE_SSDP;
#ifdef CLIENT_SSL_ENABLE
//  result = self->compMsgWifiData->getWifiValue(self, WIFI_INFO_CLOUD_SECURE_CONNECT, DATA_VIEW_FIELD_UINT8_T, &numericValue, &stringValue);
//  sud->secure = numericValue;
#endif
  result = self->compMsgWifiData->getWifiValue(self, WIFI_INFO_NET_SSDP_RECEIVED_CALL_BACK, DATA_VIEW_FIELD_UINT32_T, &numericValue, &stringValue);
  COMP_MSG_DBG(self, "N", 2, "netSSDPReceiveCallback: %p!%d!\n", numericValue, result);
  sud->netSocketReceived = (netSocketReceived_t)numericValue;
  result = self->compMsgWifiData->getWifiValue(self, WIFI_INFO_NET_SSDP_TO_SEND_CALL_BACK, DATA_VIEW_FIELD_UINT32_T, &numericValue, &stringValue);
  COMP_MSG_DBG(self, "N", 2, "netSSDPToSendCallback: %p!%d!\n", numericValue, result);
  sud->netSocketToSend = (netSocketToSend_t)numericValue;
  sud->compMsgDispatcher = self;

  pesp_conn = (struct espconn *)os_zalloc(sizeof(struct espconn));
  sud->pesp_conn = pesp_conn;
//  checkAllocOK(pesp_conn);

  type = ESPCONN_UDP;
  pesp_conn->type = type;
  pesp_conn->state = ESPCONN_NONE;
  // reverse is for the callback function
  pesp_conn->reverse = sud;

  pesp_conn->proto.tcp = NULL;
  pesp_conn->proto.udp = NULL;

  pesp_conn->proto.udp = (esp_udp *)os_zalloc(sizeof(esp_udp));
  if(!pesp_conn->proto.udp){
    os_free(pesp_conn);
    pesp_conn = NULL;
//    checkErrOK(COMP_MSG_ERR_OUT_OF_MEMORY);
  }
  pesp_conn->proto.udp->local_port = port;
  COMP_MSG_DBG(self, "N", 1, "port: %d\n", port);

//  domain3 = "0.0.0.0"; // any
//  ifAddr.addr = ipaddr_addr(domain3);
  c_memcpy(pesp_conn->proto.udp->local_ip, &ifAddr.addr, 4);
  domain = "239.255.255.250";
  domain2 = "192.168.178.96";
  multicastAddr.addr = ipaddr_addr(domain);
  ipAddr.addr = ipaddr_addr(domain2);
  // the next line is needed to get the multicast messages!!!
  espconn_igmp_join(&ipAddr, &multicastAddr);

//  c_memcpy(pesp_conn->proto.udp->remote_ip, &ipAddr.addr, 4);
ets_printf("espconn: %s %d\n", domain2, port);
  COMP_MSG_DBG(self, "N", 1, "UDP is set multicastAddr: %s ipAddr: %s\n", domain, domain2);

  result = espconn_regist_recvcb(pesp_conn, netSocketSSDPReceived);
  COMP_MSG_DBG(self, "N", 1, "regist recvcb netSocketSSDPReceived result: %d\n", result);
  if (result != COMP_MSG_ERR_OK) {
    COMP_MSG_DBG(self, "N", 1, "regist netSocketSSDPReceived err: %d", result);
  }
  result = espconn_regist_sentcb(pesp_conn, netSocketSent);
  COMP_MSG_DBG(self, "N", 1, "regist sentcb netSocketSent result: %d\n", result);
  if (result != COMP_MSG_ERR_OK) {
    COMP_MSG_DBG(self, "N", 1, "regist netSocketSent err: %d", result);
  }
  result = espconn_create(pesp_conn);
  COMP_MSG_DBG(self, "N", 1, "espconn_create result: %d\n", result);

#ifndef NOTDEF
#endif
}

// ================================= netSocketRunClientMode ====================================

static uint8_t netSocketRunClientMode(compMsgDispatcher_t *self) {
  int result;
  bool boolResult;
  struct station_config station_config;
  int numericValue;
  uint8_t *stringValue;
  uint8_t opmode;
  int status;
  char *hostName;

  COMP_MSG_DBG(self, "N", 1, "netSocketRunClientMode called\n");
  opmode = wifi_get_opmode();
  COMP_MSG_DBG(self, "N", 2, "opmode: %d", opmode);
  boolResult = wifi_station_disconnect();
  COMP_MSG_DBG(self, "N", 2, "wifi_station_disconnect: boolResult: %d", boolResult);
  c_memset(station_config.ssid,0,sizeof(station_config.ssid));
  result = self->compMsgWifiData->getWifiValue(self, WIFI_INFO_CLIENT_SSID, DATA_VIEW_FIELD_UINT8_VECTOR, &numericValue, &stringValue);
  COMP_MSG_DBG(self, "N", 2, "getSsid: result: %d\n", result);
  checkErrOK(result);
  c_memcpy(station_config.ssid, stringValue, c_strlen(stringValue));

  c_memset(station_config.password,0,sizeof(station_config.password));
  result = self->compMsgWifiData->getWifiValue(self, WIFI_INFO_CLIENT_PASSWD, DATA_VIEW_FIELD_UINT8_VECTOR, &numericValue, &stringValue);
  COMP_MSG_DBG(self, "N", 2, "getPasswd: result: %d\n", result);
  checkErrOK(result);
  COMP_MSG_DBG(self, "N", 2, "len password: %d\n", c_strlen(stringValue));
  c_memcpy(station_config.password, stringValue, c_strlen(stringValue));
  COMP_MSG_DBG(self, "N", 1, "netSocketRunClientMode: ssid: %s password: %s!\n", station_config.ssid, station_config.password);

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
  opmode = wifi_get_opmode();
  status = wifi_station_get_connect_status();
  hostName = wifi_station_get_hostname();
  COMP_MSG_DBG(self, "N", 1, "wifi is in mode: %d status: %d hostname: %s!\n", opmode, status, hostName);

  return self->compMsgSocket->startConnectionTimer(self, 2, self->compMsgSocket->startClientMode);
}

// ================================= netSocketRunSSDPMode ====================================

static uint8_t netSocketRunSSDPMode(compMsgDispatcher_t *self) {
  int result;
  bool boolResult;
  struct station_config station_config;
  int numericValue;
  uint8_t *stringValue;
  uint8_t opmode;
  int status;
  char *hostName;

  COMP_MSG_DBG(self, "N", 1, "netSocketRunSSDPMode called\n");
  opmode = wifi_get_opmode();
  COMP_MSG_DBG(self, "N", 1, "opmode: %d", opmode);
  boolResult = wifi_station_disconnect();
  COMP_MSG_DBG(self, "N", 1, "wifi_station_disconnect: boolResult: %d", boolResult);
  c_memset(station_config.ssid,0,sizeof(station_config.ssid));
  result = self->compMsgWifiData->getWifiValue(self, WIFI_INFO_CLIENT_SSID, DATA_VIEW_FIELD_UINT8_VECTOR, &numericValue, &stringValue);
  COMP_MSG_DBG(self, "N", 1, "getSsid: result: %d\n", result);
  checkErrOK(result);
stringValue="xxxx";
  c_memcpy(station_config.ssid, stringValue, c_strlen(stringValue));

  c_memset(station_config.password,0,sizeof(station_config.password));
  result = self->compMsgWifiData->getWifiValue(self, WIFI_INFO_CLIENT_PASSWD, DATA_VIEW_FIELD_UINT8_VECTOR, &numericValue, &stringValue);
  COMP_MSG_DBG(self, "N", 1, "getPasswd: result: %d\n", result);
  checkErrOK(result);
stringValue="yyyy";
  COMP_MSG_DBG(self, "N", 1, "len password: %d\n", c_strlen(stringValue));
  c_memcpy(station_config.password, stringValue, c_strlen(stringValue));
  COMP_MSG_DBG(self, "N", 1, "netSocketRunSSDPMode: ssid: %s password: %s!\n", station_config.ssid, station_config.password);

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
  opmode = wifi_get_opmode();
  status = wifi_station_get_connect_status();
  hostName = wifi_station_get_hostname();
  COMP_MSG_DBG(self, "N", 1, "wifi is in mode: %d status: %d hostname: %s!\n", opmode, status, hostName);

  return self->compMsgSocket->startConnectionTimer(self, 2, self->compMsgSocket->startSSDPMode);
//  return COMP_MSG_ERR_OK;
}

// ================================= netSocketStartCloudSocket ====================================

static uint8_t netSocketStartCloudSocket (compMsgDispatcher_t *self) {
  int result;

  COMP_MSG_DBG(self, "N", 2, "netSocketStartCloudSocket called");
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
  self->compMsgSocket->netSocketRunSSDPMode = &netSocketRunSSDPMode;
  self->compMsgSocket->netSocketSendData = netSocketSendData;
  self->compMsgSocket->startClientMode = &startClientMode;
  self->compMsgSocket->startSSDPMode = &startSSDPMode;
  return COMP_MSG_ERR_OK;
}
