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

int ets_snprintf(char *buffer, size_t sizeOfBuffer,  const char *format, ...);

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
  COMP_MSG_DBG(self, "N", 2, "netSocketSendData: size: %d %s", size, payload);
#ifdef CLIENT_SSL_ENABLE
  if (sud->secure) {
    result = espconn_secure_sent(sud->pesp_conn, (unsigned char *)payload, size);
  } else
#endif
  {
    result = espconn_sent(sud->pesp_conn, (unsigned char *)payload, size);
  }

  COMP_MSG_DBG(self, "N", 2, "netSocketSendData: espconn_sent: result: %d", result);
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
  COMP_MSG_DBG(self, "N", 2, "netSocketRecv: remote_port: %d\n", sud->remote_port);

//FIXME need to handle data!!
#ifdef NOTDEF
  COMP_MSG_DBG(self, "N", 2, "netSocketRecv: call compMsgHttp->httpParse\n");
  if (sud->httpMsgInfos == NULL) {
    sud->maxHttpMsgInfos = 5;
    sud->numHttpMsgInfos = 0;
    sud->httpMsgInfos = os_zalloc(sud->maxHttpMsgInfos * sizeof(httpMsgInfo_t));
    sud->connectionType = NET_SOCKET_TYPE_CLIENT;
  }
  result = sud->compMsgDispatcher->compMsgHttp->httpParse(sud, payload, payloadLgth);
  COMP_MSG_DBG(self, "N", 2, "netSocketRecv: call compMsgHttp->httpParse done result: %d\n", result);
#endif

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
  COMP_MSG_DBG(self, "N", 1, "remote %s:%d received\n", temp, pesp_conn->proto.tcp->remote_port);
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

//ets_printf("socketSent: %p\n", arg);
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
//ets_printf("socketSent2: sud: %p\n", sud);
  self = sud->compMsgDispatcher;
  if (self == NULL) {
ets_printf("socketSent self == NULL\n");
    return;
  }
//ets_printf("socketSent3: self: %p\n",  self);
  COMP_MSG_DBG(self, "N", 2, "socketSent: arg: %p", arg);

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
    sud->compMsgDispatcher->dispatcherCommon->runningModeFlags &= ~COMP_DISP_RUNNING_MODE_CLIENT;
    break;
  case NET_SOCKET_TYPE_SOCKET:
    sud->compMsgDispatcher->dispatcherCommon->runningModeFlags &= ~COMP_DISP_RUNNING_MODE_CLOUD;
    break;
  case NET_SOCKET_TYPE_SSDP:
    sud->compMsgDispatcher->dispatcherCommon->runningModeFlags &= ~COMP_DISP_RUNNING_MODE_SSDP;
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
//ets_printf("netSocketSent: free payloadBuf: %p\n", sud->payloadBuf);
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

//ets_printf("netSocketConnected: arg: %p\n", arg);
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
  COMP_MSG_DBG(self, "N", 1, "netSocketConnected\n");
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
  sud->compMsgDispatcher->dispatcherCommon->runningModeFlags |= COMP_DISP_RUNNING_MODE_CLOUD;
  if (sud->compMsgDispatcher->compMsgSendReceive->startSendMsg2 != NULL) {
    COMP_MSG_DBG(self, "N", 1, "call sud->compMsgDispatcher->startSendMsg2");
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
  COMP_MSG_DBG(self, "N", 1, "netSocketConnect");
  if( pesp_conn->type == ESPCONN_TCP ) {
#ifdef CLIENT_SSL_ENABLE
    if (sud->secure){
      espconn_secure_set_size(ESPCONN_CLIENT, 5120); /* set SSL buffer size */
      COMP_MSG_DBG(self, "N", 1, "call espconn_secure_connect");
      espconn_status = espconn_secure_connect(pesp_conn);
      COMP_MSG_DBG(self, "N", 1, "after call espconn_secure_connect status: %d", espconn_status);
  
    } else
#endif
    {
      COMP_MSG_DBG(self, "N", 1, "netSocketConnect TCP called");
      result = espconn_connect(pesp_conn);
      COMP_MSG_DBG(self, "N", 1, "espconn_connect TCP: result: %d", result);
    }
  } else {
    if (pesp_conn->type == ESPCONN_UDP) {
      COMP_MSG_DBG(self, "N", 2, "netSocketConnect UDP called");
      result = espconn_create(pesp_conn);
      COMP_MSG_DBG(self, "N", 2, "espconn_create UPD: result: %d", result);
    } 
  } 
  COMP_MSG_DBG(self, "N", 1, "netSocketConnect done");
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
  COMP_MSG_DBG(self, "N", 2, "netServerConnected: arg: %p", arg);
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
  uint8_t *strValue;
  struct espconn *pesp_conn;
  unsigned port;
  struct ip_info pTempIp;
  ip_addr_t ipaddr;
  unsigned type;
  int result;
  const char *domain;
  socketUserData_t *sud;
  uint8_t flags;
  fieldValue_t fieldValue;
  fieldValueCallback_t callback;

  pesp_conn = NULL;

  sud = (socketUserData_t *)os_zalloc(sizeof(socketUserData_t));
// set err_opcode[...] here if alloc fails or eventually a variable in compMsgDispatcher!!
//   checkAllocOK(sud);
  COMP_MSG_DBG(self, "N", 1, "openCloudSocket");
  COMP_MSG_DBG(self, "N", 2, "sud0: %p", sud);
  sud->maxHttpMsgInfos = 5;
  sud->numHttpMsgInfos = 0;
  sud->httpMsgInfos = os_zalloc(sud->maxHttpMsgInfos * sizeof(httpMsgInfo_t));
  sud->connectionType = NET_SOCKET_TYPE_SOCKET;
#ifdef CLIENT_SSL_ENABLE
  fieldValue.flags = COMP_MSG_FIELD_IS_NUMERIC;
  fieldValue.dataValue.value.numericValue = 0;
  fieldValue.cmdKey = COMP_MSG_DATA_VALUE_CMD_KEY_SPECIAL;
  fieldValue.fieldNameId = 0;
  fieldValue.fieldValueId = COMP_MSG_WIFI_VALUE_ID_NetSecureConnect;
  fieldValue.fieldValueCallback = NULL;
  result = self->compMsgDataValue->getFieldValueInfo(self, &fieldValue, &strValue);
  sud->secure = numericValue;
#endif
  // the following 2 calls deliver a callback function address in numericValue !!
  fieldValue.flags = COMP_MSG_FIELD_IS_NUMERIC;
  fieldValue.dataValue.value.numericValue = 0;
  fieldValue.cmdKey = COMP_MSG_DATA_VALUE_CMD_KEY_SPECIAL;
  fieldValue.fieldNameId = 0;
  fieldValue.fieldValueId = COMP_MSG_WIFI_VALUE_ID_NetReceivedCallback;
  fieldValue.fieldValueCallback = NULL;
  result = self->compMsgDataValue->getFieldValueInfo(self, &fieldValue, &strValue);
  numericValue = fieldValue.dataValue.value.numericValue;
  sud->netSocketReceived = (netSocketReceived_t)numericValue;
  fieldValue.flags = COMP_MSG_FIELD_IS_NUMERIC;
  fieldValue.dataValue.value.numericValue = 0;
  fieldValue.cmdKey = COMP_MSG_DATA_VALUE_CMD_KEY_SPECIAL;
  fieldValue.fieldNameId = 0;
  fieldValue.fieldValueId = COMP_MSG_WIFI_VALUE_ID_NetToSendCallback;
  fieldValue.fieldValueCallback = NULL;
  result = self->compMsgDataValue->getFieldValueInfo(self, &fieldValue, &strValue);
  numericValue = fieldValue.dataValue.value.numericValue;
  sud->netSocketToSend = (netSocketToSend_t)numericValue;
  sud->compMsgDispatcher = self;
  COMP_MSG_DBG(self, "N", 2, "netSocketReceived: %p netSocketToSend: %p", sud->netSocketReceived, sud->netSocketToSend);

  fieldValue.flags = COMP_MSG_FIELD_IS_NUMERIC;
  fieldValue.dataValue.value.numericValue = 0;
  fieldValue.cmdKey = COMP_MSG_DATA_VALUE_CMD_KEY_SPECIAL;
  fieldValue.fieldNameId = 0;
  fieldValue.fieldValueId = COMP_MSG_WIFI_VALUE_ID_cloudPort;
  fieldValue.fieldValueCallback = NULL;
  result = self->compMsgDataValue->getFieldValueInfo(self, &fieldValue, &strValue);
  port = fieldValue.dataValue.value.numericValue;

  COMP_MSG_DBG(self, "N", 1, "port: %d", port);
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

#ifdef OLD
#ifdef CLOUD_1
  result = self->compMsgDataValue->getFieldValueInfo(self, WIFI_INFO_CLOUD_HOST_1, &flags, &callback, &numericValue, &stringValue);
#else
  result = self->compMsgDataValue->getFieldValueInfo(self, WIFI_INFO_CLOUD_HOST_2, &flags, &callback, &numericValue, &stringValue);
#endif
#endif
  domain = stringValue;
  COMP_MSG_DBG(self, "N", 1, "domain: %s", domain);
  ipaddr.addr = ipaddr_addr(domain);
  c_memcpy(pesp_conn->proto.tcp->remote_ip, &ipaddr.addr, 4);
//  COMP_MSG_DBG(self, "N", 1, "TCP ip is set: ");
//  COMP_MSG_DBG(self, "N", 1, IPSTR, IP2STR(&ipaddr.addr));

  COMP_MSG_DBG(self, "N", 1, "call regist connectcb\n");
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
    COMP_MSG_DBG(self, "N", 1, "call gethostbyname: found ip for %s 0x%08x", domain, host_ip);
    socketDnsFound(domain, &host_ip, pesp_conn);  // ip is returned in host_ip.
    COMP_MSG_DBG(self, "N", 1, "host_ip2: 0x%08x", host_ip);
  }
  return COMP_MSG_ERR_OK;
}

// ================================= startNetClientMode ====================================

static  void startNetClientMode(void *arg) {
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
  uint8_t opmode;
  char temp[64];
  compMsgTimerSlot_t *compMsgTimerSlot;
  uint8_t flags;
  uint8_t *strValue;
  fieldValue_t fieldValue;
  fieldValueCallback_t callback;

  compMsgTimerSlot = (compMsgTimerSlot_t *)arg;
  self = compMsgTimerSlot->compMsgDispatcher;
  COMP_MSG_DBG(self, "N", 2, "startNetClientMode\n");
  COMP_MSG_DBG(self, "N", 2, "startNetClientMode timerInfo:%p\n", compMsgTimerSlot);
  compMsgTimerSlot->connectionMode = STATION_IF;

  compMsgTimerSlot->followupCallback = self->compMsgSocket->sendSSDPInfo;
  compMsgTimerSlot->followupTimerId = 1;
  compMsgTimerSlot->followupInterval = 3000;
  compMsgTimerSlot->followupMode = 1;

  pesp_conn = NULL;

  result = self->compMsgSocket->checkConnectionStatus(compMsgTimerSlot);
  if (result != COMP_MSG_ERR_OK) {
    return;
  }

  self->dispatcherCommon->runningModeFlags |= COMP_DISP_RUNNING_MODE_CLIENT;
  fieldValue.flags = COMP_MSG_FIELD_IS_NUMERIC;
  fieldValue.dataValue.value.numericValue = compMsgTimerSlot->ip_addr;
  fieldValue.cmdKey = COMP_MSG_DATA_VALUE_CMD_KEY_SPECIAL;
  fieldValue.fieldNameId = 0;
  fieldValue.fieldValueId = COMP_MSG_WIFI_VALUE_ID_clientIPAddr;
  fieldValue.fieldValueCallback = NULL;
  result = self->compMsgDataValue->setFieldValueInfo(self, &fieldValue);
  fieldValue.flags = COMP_MSG_FIELD_IS_NUMERIC;
  fieldValue.dataValue.value.numericValue = 0;
  fieldValue.cmdKey = COMP_MSG_DATA_VALUE_CMD_KEY_SPECIAL;
  fieldValue.fieldNameId = 0;
  fieldValue.fieldValueId = COMP_MSG_WIFI_VALUE_ID_clientIPAddr;
  fieldValue.fieldValueCallback = NULL;
  result = self->compMsgDataValue->getFieldValueInfo(self, &fieldValue, &strValue);
  numericValue = fieldValue.dataValue.value.numericValue;
  COMP_MSG_DBG(self, "N", 2, "ip2: 0x%08x\n", numericValue);
  c_sprintf(temp, "%d.%d.%d.%d", IP2STR(&numericValue));

  fieldValue.flags = COMP_MSG_FIELD_IS_NUMERIC;
  fieldValue.dataValue.value.numericValue = 0;
  fieldValue.cmdKey = COMP_MSG_DATA_VALUE_CMD_KEY_SPECIAL;
  fieldValue.fieldNameId = 0;
  fieldValue.fieldValueId = COMP_MSG_WIFI_VALUE_ID_clientPort;
  fieldValue.fieldValueCallback = NULL;
  result = self->compMsgDataValue->getFieldValueInfo(self, &fieldValue, &strValue);
  port = fieldValue.dataValue.value.numericValue;
  COMP_MSG_DBG(self, "N", 1, "startWebClientMode IP: %s port: %d result: %d\n", temp, port, result);

  sud = (socketUserData_t *)os_zalloc(sizeof(socketUserData_t));
//   checkAllocOK(sud);
  COMP_MSG_DBG(self, "N", 2, "sud0: %p\n", sud);
//  checkAllocgLOK(sud->urls);
  sud->connectionType = NET_SOCKET_TYPE_CLIENT;
#ifdef CLIENT_SSL_ENABLE
  fieldValue.flags = COMP_MSG_FIELD_IS_NUMERIC;
  fieldValue.dataValue.value.numericValue = 0;
  fieldValue.cmdKey = COMP_MSG_DATA_VALUE_CMD_KEY_SPECIAL;
  fieldValue.fieldNameId = 0;
  fieldValue.fieldValueId = COMP_MSG_WIFI_VALUE_ID_NetSecureConnect;
  fieldValue.fieldValueCallback = NULL;
  result = self->compMsgDataValue->getFieldValueInfo(self, &fieldValue, &strValue);
  sud->secure = fieldValue.dataValue.value.numericValue;
#endif
  fieldValue.flags = COMP_MSG_FIELD_IS_NUMERIC;
  fieldValue.dataValue.value.numericValue = 0;
  fieldValue.cmdKey = COMP_MSG_DATA_VALUE_CMD_KEY_SPECIAL;
  fieldValue.fieldNameId = 0;
  fieldValue.fieldValueId = COMP_MSG_WIFI_VALUE_ID_NetReceivedCallback;
  fieldValue.fieldValueCallback = NULL;
  result = self->compMsgDataValue->getFieldValueInfo(self, &fieldValue, &strValue);
  numericValue = fieldValue.dataValue.value.numericValue;
  COMP_MSG_DBG(self, "N", 2, "netReceivedCallback: %p!%d!", numericValue, result);
  sud->netSocketReceived = (netSocketReceived_t)numericValue;
  fieldValue.flags = COMP_MSG_FIELD_IS_NUMERIC;
  fieldValue.dataValue.value.numericValue = 0;
  fieldValue.cmdKey = COMP_MSG_DATA_VALUE_CMD_KEY_SPECIAL;
  fieldValue.fieldNameId = 0;
  fieldValue.fieldValueId = COMP_MSG_WIFI_VALUE_ID_NetToSendCallback;
  fieldValue.fieldValueCallback = NULL;
  result = self->compMsgDataValue->getFieldValueInfo(self, &fieldValue, &strValue);
  numericValue = fieldValue.dataValue.value.numericValue;
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
    COMP_MSG_DBG(self, "N", 1, "regist netSocketReceived err: %d", result);
  }
  result = espconn_regist_sentcb(pesp_conn, netSocketSent);
  if (result != COMP_MSG_ERR_OK) {
    COMP_MSG_DBG(self, "N", 1, "regist netSocketSent err: %d", result);
  }
  result = espconn_accept(pesp_conn);
  if (result != COMP_MSG_ERR_OK) {
//    return COMP_MSG_ERR_TCP_ACCEPT;
    COMP_MSG_DBG(self, "N", 1, "regist_accept err result: %d", result);
  }
  result = espconn_regist_time(pesp_conn, tcp_server_timeover, 0);
  if (result != COMP_MSG_ERR_OK) {
//    return COMP_MSG_ERR_REGIST_TIME;
    COMP_MSG_DBG(self, "N", 1, "regist_time err result: %d", result);
  }
//ets_printf("net startClientMode done\n");
  // limit maximal allowed connections
  result = espconn_tcp_set_max_con_allow(pesp_conn, MAX_CONNECTIONS_ALLOWED);
  if (self->compMsgSendReceive->startSendMsg != NULL) {
    COMP_MSG_DBG(self, "N", 1, "call startSendMsg: %p\n", self->compMsgSendReceive->startSendMsg);
    result = self->compMsgSendReceive->startSendMsg(self);
    COMP_MSG_DBG(self, "N", 2, "startSendMsg result: %d", result);
  }
//ets_printf("startNetClientMode done\n");
}

// ================================= startProdTestMode ====================================

static  void startProdTestMode(void *arg) {
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
  COMP_MSG_DBG(self, "N", 1, "net startProdTestMode\n");
  COMP_MSG_DBG(self, "N", 2, "net startProdTestMode timerInfo:%p\n", compMsgTimerSlot);
  compMsgTimerSlot->connectionMode = STATION_IF;
  pesp_conn = NULL;

  result = self->compMsgSocket->checkConnectionStatus(compMsgTimerSlot);
  if (result != COMP_MSG_ERR_OK) {
    return;
  }

#ifdef OLD
  self->dispatcherCommon->runningModeFlags |= COMP_DISP_RUNNING_MODE_PROD_TEST;
  result = self->compMsgDataValue->setFieldValueInfo(self, COMP_MSG_DATA_VALUE_CMD_KEY_SPECIAL, WIFI_INFO_PROD_TEST_IP_ADDR, NULL, compMsgTimerSlot->ip_addr, NULL);
  result = self->compMsgDataValue->getFieldValueInfo(self, COMP_MSG_DATA_VALUE_CMD_KEY_SPECIAL, WIFI_INFO_PROD_TEST_IP_ADDR, &flags, &callback, &numericValue, &stringValue);
  COMP_MSG_DBG(self, "N", 2, "ip2: 0x%08x\n", numericValue);
  c_sprintf(temp, "%d.%d.%d.%d", IP2STR(&numericValue));

  result = self->compMsgDataValue->getFieldValueInfo(self, COMP_MSG_DATA_VALUE_CMD_KEY_SPECIAL, WIFI_INFO_PROD_TEST_IP_ADDR, &flags, &callback, &numericValue, &stringValue);
  port = numericValue;
  COMP_MSG_DBG(self, "N", 1, "net startProdTestMode IP: %s port: %d result: %d\n", temp, port, result);

  sud = (socketUserData_t *)os_zalloc(sizeof(socketUserData_t));
//   checkAllocOK(sud);
  COMP_MSG_DBG(self, "N", 1, "sud: %p\n", sud);
  sud->connectionType = NET_SOCKET_TYPE_CLIENT;
  result = self->compMsgDataValue->getFieldValueInfo(self, COMP_MSG_DATA_VALUE_CMD_KEY_SPECIAL, WIFI_INFO_NET_RECEIVED_CALL_BACK, &flags, &callback, &numericValue, &stringValue);
  COMP_MSG_DBG(self, "N", 1, "netReceivedCallback: %p!%d!\n", numericValue, result);
  sud->netSocketReceived = (netSocketReceived_t)numericValue;
  result = self->compMsgDataValue->getFieldValueInfo(self, COMP_MSG_DATA_VALUE_CMD_KEY_SPECIAL, WIFI_INFO_NET_TO_SEND_CALL_BACK, &flags, &callback, &numericValue, &stringValue);
  COMP_MSG_DBG(self, "N", 1, "netToSendCallback: %p!%d!\n", numericValue, result);
  sud->netSocketToSend = (netSocketToSend_t)numericValue;
  sud->compMsgDispatcher = self;
#endif

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
  COMP_MSG_DBG(self, "N", 1, "port: %d\n", port);

  COMP_MSG_DBG(self, "N", 1, "call regist connectcb\n");
  result = espconn_regist_connectcb(pesp_conn, netServerConnected);
  if (result != COMP_MSG_ERR_OK) {
//    return COMP_MSG_ERR_REGIST_CONNECT_CB;
  }
  COMP_MSG_DBG(self, "N", 1, "regist connectcb netServerConnected result: %d\n", result);
  result = espconn_regist_recvcb(pesp_conn, netSocketReceived);
  if (result != COMP_MSG_ERR_OK) {
    COMP_MSG_DBG(self, "N", 1, "regist netSocketReceived err: %d", result);
  }
  result = espconn_regist_sentcb(pesp_conn, netSocketSent);
  if (result != COMP_MSG_ERR_OK) {
    COMP_MSG_DBG(self, "N", 1, "regist netSocketSent err: %d", result);
  }
  result = espconn_accept(pesp_conn);
  if (result != COMP_MSG_ERR_OK) {
//    return COMP_MSG_ERR_TCP_ACCEPT;
    COMP_MSG_DBG(self, "N", 1, "regist_accept err result: %d", result);
  }
  result = espconn_regist_time(pesp_conn, tcp_server_timeover, 0);
  if (result != COMP_MSG_ERR_OK) {
//    return COMP_MSG_ERR_REGIST_TIME;
    COMP_MSG_DBG(self, "N", 1, "regist_time err result: %d", result);
  }
  // limit maximal allowed connections
  result = espconn_tcp_set_max_con_allow(pesp_conn, MAX_CONNECTIONS_ALLOWED);
}

// ================================= netSocketRunClientMode ====================================

static uint8_t netSocketRunClientMode(compMsgDispatcher_t *self) {
  int result;
  bool boolResult;
  struct station_config station_config;
  int numericValue;
  uint8_t timerId;
  int interval;
  uint8_t *stringValue;
  uint8_t opmode;
  int status;
  char *hostName;
  uint8_t flags;
  uint8_t *strValue;
  fieldValue_t fieldValue;
  fieldValueCallback_t callback;

ets_printf(">>>>netSocketRunClientMode runningModeFlags: 0x%08x\n", self->dispatcherCommon->runningModeFlags);
  COMP_MSG_DBG(self, "N", 1, "netSocketRunClientMode called\n");
  opmode = wifi_get_opmode();
  COMP_MSG_DBG(self, "N", 1, "opmode: %d", opmode);
  status = wifi_station_get_connect_status();
ets_printf("netSocketRunClientMode connectStatus: %d\n", status);
//  boolResult = wifi_station_disconnect();
  COMP_MSG_DBG(self, "N", 2, "wifi_station_disconnect: boolResult: %d", boolResult);
  c_memset(station_config.ssid,0,sizeof(station_config.ssid));
  fieldValue.flags = COMP_MSG_FIELD_IS_STRING;
  fieldValue.dataValue.value.stringValue = NULL;
  fieldValue.cmdKey = COMP_MSG_DATA_VALUE_CMD_KEY_SPECIAL;
  fieldValue.fieldNameId = 0;
  fieldValue.fieldValueId = COMP_MSG_WIFI_VALUE_ID_clientSsid;
  fieldValue.fieldValueCallback = NULL;
  result = self->compMsgDataValue->getFieldValueInfo(self, &fieldValue, &strValue);
  COMP_MSG_DBG(self, "N", 2, "getSsid: result: %d\n", result);
  checkErrOK(result);
  stringValue = fieldValue.dataValue.value.stringValue;
  c_memcpy(station_config.ssid, stringValue, c_strlen(stringValue));

  c_memset(station_config.password,0,sizeof(station_config.password));
  fieldValue.flags = COMP_MSG_FIELD_IS_STRING;
  fieldValue.dataValue.value.stringValue = NULL;
  fieldValue.cmdKey = COMP_MSG_DATA_VALUE_CMD_KEY_SPECIAL;
  fieldValue.fieldNameId = 0;
  fieldValue.fieldValueId = COMP_MSG_WIFI_VALUE_ID_clientPasswd;
  fieldValue.fieldValueCallback = NULL;
  result = self->compMsgDataValue->getFieldValueInfo(self, &fieldValue, &strValue);
  COMP_MSG_DBG(self, "N", 2, "getPasswd: result: %d\n", result);
  checkErrOK(result);
  stringValue = fieldValue.dataValue.value.stringValue;
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

  timerId = 4;
  interval = 1000;
  return self->compMsgSocket->startConnectionTimer(self, timerId, interval, 1, self->compMsgSocket->startNetClientMode);
}

// ================================= netSocketRunProdTestMode ====================================

static uint8_t netSocketRunProdTestMode(compMsgDispatcher_t *self) {
  int result;
  bool boolResult;
  uint8_t status;
  struct softap_config softap_config;
  int numericValue;
  uint8_t timerId;
  int interval;
  uint8_t *stringValue;
  
  COMP_MSG_DBG(self, "W", 2, "netSocketRunProdTestMode\n");
  boolResult = wifi_station_disconnect();
// checking result makes problems with some modules, so do not check.
//  if (!boolResult) {
//    COMP_MSG_DBG(self, "Y", 0, "webSocketRunAPMode COMP_MSG_ERR_CANNOT_DISCONNECT\n");
//    return COMP_MSG_ERR_CANNOT_DISCONNECT;
//  }
  boolResult = wifi_set_opmode(OPMODE_STATIONAP);
  if (!boolResult) {
    COMP_MSG_DBG(self, "Y", 0, "netSocketRunProdTestMode COMP_MSG_ERR_CANNOT_SET_OPMODE\n");
    return COMP_MSG_ERR_CANNOT_SET_OPMODE;
  }
  c_memset(softap_config.ssid,0,sizeof(softap_config.ssid));
  result = self->compMsgWifiData->getProdTestSsid(self, &numericValue, &stringValue);
  COMP_MSG_DBG(self, "W", 1, "netSocketRunProdTestMode get_ProdTestSsid result: %d\n", result);
  checkErrOK(result);
  c_memcpy(softap_config.ssid, stringValue, c_strlen(stringValue));
  softap_config.ssid_len = c_strlen(stringValue);
  softap_config.ssid_hidden = 0;
  result = self->compMsgWifiData->getProdTestSecurityType(self, &numericValue, &stringValue);
  checkErrOK(result);
  switch (numericValue) {
  case 0:
    softap_config.authmode = AUTH_OPEN;
    break;
  case 1:
    softap_config.authmode = AUTH_WEP;
    break;
  case 2:
    softap_config.authmode = AUTH_WPA_PSK;
    break;
  case 3:
    softap_config.authmode = AUTH_WPA2_PSK;
    break;
  case 4:
    softap_config.authmode = AUTH_WPA_WPA2_PSK;
    break;
  default:
    return COMP_MSG_ERR_BAD_SECURITY_TYPE;
  }
  softap_config.channel = 6;
  softap_config.max_connection = 4;
  softap_config.beacon_interval = 100;
  COMP_MSG_DBG(self, "W", 1, "netSocketRunProdTestMode wifi_softap_set_config\n");
  boolResult = wifi_softap_set_config(&softap_config);
  if (!boolResult) {
    return COMP_MSG_ERR_CANNOT_SET_OPMODE;
  }
  status = wifi_station_get_connect_status();
  COMP_MSG_DBG(self, "W", 1, "wifi is in mode: %d status: %d ap_id: %d hostname: %s!\n", wifi_get_opmode(), status, wifi_station_get_current_ap_id(), wifi_station_get_hostname());

  interval = 1000;
  timerId = 0;
  return self->compMsgSocket->startConnectionTimer(self, timerId, interval, 1, self->compMsgSocket->startProdTestMode);
}

// ================================= netSocketStartCloudSocket ====================================

static uint8_t netSocketStartCloudSocket (compMsgDispatcher_t *self) {
  int result;

  COMP_MSG_DBG(self, "N", 1, "netSocketStartCloudSocket called");
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
  self->compMsgSocket->netSocketRunProdTestMode = &netSocketRunProdTestMode;
  self->compMsgSocket->netSocketSendData = &netSocketSendData;
  self->compMsgSocket->netSocketConnected = &netSocketConnected;
//  self->compMsgSocket->netSocketReceived = &netSocketReceived;
  self->compMsgSocket->socketSent = &socketSent;
  self->compMsgSocket->socketDnsFound = &socketDnsFound;

  self->compMsgSocket->startNetClientMode = &startNetClientMode;
  self->compMsgSocket->startProdTestMode = &startProdTestMode;
  return COMP_MSG_ERR_OK;
}
