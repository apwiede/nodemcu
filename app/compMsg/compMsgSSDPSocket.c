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
 * File:   compMsgSSDPSocket.c
 * Author: Arnulf P. Wiedemann
 *
 * Created on March 10th, 2017
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

static const char *ssdp_notify_1 = "NOTIFY * HTTP/1.1\r\n\
HOST: 239.255.255.250:1900\r\n\
CACHE-CONTROL: max-age=15\r\n\
Location: http://";
static const char *ssdp_notify_3 = "\r\nNT: urn:";
static const char *ssdp_notify_5 = ":device:**\r\n\
NTS: ssdp:alive\r\n\
SERVER: OS/version UPnP/1.0 product/version\r\n\
USN: uuid:";

typedef void (* netSocketBinaryReceived_t)(void *arg, void *sud, char *pdata, unsigned short len);
typedef void (* netSocketTextReceived_t)(void *arg, void *sud, char *pdata, unsigned short len);

int ets_snprintf(char *buffer, size_t sizeOfBuffer,  const char *format, ...);

static int cnt = 0;
static ip_addr_t host_ip; // for dns
static int dns_reconn_count = 0;
static uint16_t udp_server_timeover = 30;
static int haveMulticastJoined = 0;
static int numNotifies = 0;

// ================================= netSocketSSDPRecv  ====================================

static void netSocketSSDPRecv(void *arg, char *pdata, unsigned short len) {
  struct espconn *pesp_conn;
  socketUserData_t *sud;
  int result;
  compMsgDispatcher_t *self;

  pesp_conn = (struct espconn *)arg;
  if (pesp_conn == NULL) {
ets_printf("netSocketSSDPRecv pesp_conn == NULL\n");
    return;
  }
  sud = (socketUserData_t *)pesp_conn->reverse;
  if(sud == NULL) {
ets_printf("netSocketSSDPRecv sud == NULL\n");
    return;
  }
  self = sud->compMsgDispatcher;
  if(self == NULL) {
ets_printf("netSocketSSDPRecv self == NULL\n");
    return;
  }
  COMP_MSG_DBG(self, "N", 1, "sud: %p", sud);
  if(sud == NULL) {
    return;
  }
  COMP_MSG_DBG(self, "N", 1, "netSocketSSDPRecv is called. %d %s", len, pdata);
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
}

// ================================= openSSDPSocket ====================================

static uint8_t openSSDPSocket(compMsgDispatcher_t *self, char *domain, socketUserData_t **sudOut) {
  char temp[64];
  uint8_t mode;
  int numericValue;
  uint8_t *stringValue;
  uint8_t *strValue;
  struct espconn *pesp_conn;
  unsigned port;
  struct ip_info pTempIp;
  ip_addr_t ipaddr;
  ip_addr_t ipaddr2;
  ip_addr_t multicastAddr;
  unsigned type;
  int result;
  socketUserData_t *sud;
  uint8_t flags;
  dataValue_t dataValue;
  fieldValueCallback_t callback;

  pesp_conn = NULL;

  COMP_MSG_DBG(self, "N", 1, ">> openSSDPSocket");
  sud = (socketUserData_t *)os_zalloc(sizeof(socketUserData_t));
// set err_opcode[...] here if alloc fails or eventually a variable in compMsgDispatcher!!
//   checkAllocOK(sud);
  COMP_MSG_DBG(self, "N", 2, "sud0: %p", sud);
  sud->maxHttpMsgInfos = 5;
  sud->numHttpMsgInfos = 0;
  sud->httpMsgInfos = os_zalloc(sud->maxHttpMsgInfos * sizeof(httpMsgInfo_t));
  sud->connectionType = NET_SOCKET_TYPE_SSDP;

  // the following 2 calls deliver a callback function address in numericValue !!
  dataValue.flags = COMP_MSG_FIELD_IS_NUMERIC;
  dataValue.value.numericValue = 0;
  dataValue.cmdKey = COMP_MSG_DATA_VALUE_CMD_KEY_SPECIAL;
  dataValue.fieldNameId = 0;
  dataValue.fieldValueId = COMP_MSG_WIFI_VALUE_ID_SSDPReceivedCallback;
  dataValue.fieldValueCallback = NULL;
  result = self->compMsgDataValue->getDataValue(self, &dataValue, &strValue);
  numericValue = dataValue.value.numericValue;
  sud->netSocketReceived = (netSocketReceived_t)numericValue;
  dataValue.flags = COMP_MSG_FIELD_IS_NUMERIC;
  dataValue.value.numericValue = 0;
  dataValue.cmdKey = COMP_MSG_DATA_VALUE_CMD_KEY_SPECIAL;
  dataValue.fieldNameId = 0;
  dataValue.fieldValueId = COMP_MSG_WIFI_VALUE_ID_SSDPToSendCallback;
  dataValue.fieldValueCallback = NULL;
  result = self->compMsgDataValue->getDataValue(self, &dataValue, &strValue);
  numericValue = dataValue.value.numericValue;
  sud->netSocketToSend = (netSocketToSend_t)numericValue;
  sud->compMsgDispatcher = self;
  COMP_MSG_DBG(self, "N", 2, "callback netSocketSSDPReceived: %p callback netSocketSSDPToSend: %p", sud->netSocketReceived, sud->netSocketToSend);

  // Code as in net_create
  pesp_conn = (struct espconn *)os_zalloc(sizeof(struct espconn));
  sud->pesp_conn = pesp_conn;
//  checkAllocOK(pesp_conn);

  pesp_conn->proto.tcp = NULL;
  pesp_conn->proto.udp = NULL;
  pesp_conn->reverse = NULL;

  pesp_conn->proto.udp = (esp_udp *)os_zalloc(sizeof(esp_udp));
  if (!pesp_conn->proto.udp) {
    os_free(pesp_conn);
    pesp_conn = NULL;
//    checkErrOK(COMP_MSG_ERR_OUT_OF_MEMORY);
  }
  type = ESPCONN_UDP;
  pesp_conn->type = type;
  pesp_conn->state = ESPCONN_NONE;
  // reverse is for the callback function
  pesp_conn->reverse = sud;
  // End code as in net_create

  // Code as in net_start
  port = 1900;
  COMP_MSG_DBG(self, "N", 2, "port: %d", port);
  pesp_conn->proto.udp->remote_port = port;
  pesp_conn->proto.udp->local_port = espconn_port();

  COMP_MSG_DBG(self, "N", 2, "domain: %s local_port: %d remote_port: %d", domain, pesp_conn->proto.udp->local_port, pesp_conn->proto.udp->remote_port);
  wifi_get_ip_info(STATION_IF, &pTempIp);
  multicastAddr.addr = ipaddr_addr(domain);
  ipaddr.addr = pTempIp.ip.addr;
//ets_printf("my ipaddr: %d.%d.%d.%d", IP2STR(&pTempIp.ip));
  c_memcpy(pesp_conn->proto.udp->remote_ip, &multicastAddr.addr, 4);
  COMP_MSG_DBG(self, "N", 2, "UDP ip is set: ");
  COMP_MSG_DBG(self, "N", 2, IPSTR, IP2STR(&multicastAddr.addr));

  // handle multicast join
  if (!haveMulticastJoined) {
    haveMulticastJoined++;
    // the next line is needed to get the multicast messages!!!
    espconn_igmp_join(&ipaddr, &multicastAddr);
    COMP_MSG_DBG(self, "N", 1, "UDP is set multicastAddr: %s ipaddr: 0x%08x multicastAddr: 0x%08x\n", domain, ipaddr, multicastAddr);
    // end handle multicast join
  }

  c_memcpy(pesp_conn->proto.udp->remote_ip, &multicastAddr.addr, 4);
  c_memcpy(pesp_conn->proto.udp->local_ip, &ipaddr.addr, 4);
  // do not register a connectcb! it is only for tcp and not checked!!
  // so a core dump, when using it for udp!!
  result = espconn_regist_recvcb(pesp_conn, self->compMsgSocket->netSocketSSDPRecv);
  if (result != COMP_MSG_ERR_OK) {
    COMP_MSG_DBG(self, "N", 1, "regist recvcb err: %d", result);
  }
  result = espconn_regist_sentcb(pesp_conn, self->compMsgSocket->socketSent);
  if (result != COMP_MSG_ERR_OK) {
    COMP_MSG_DBG(self, "N", 1, "regist sentcb err: %d", result);
  }
  { 
    if(pesp_conn->proto.tcp->remote_port || pesp_conn->proto.tcp->local_port) {
      espconn_delete(pesp_conn);
    }
  }
  host_ip.addr = 0;
  dns_reconn_count = 0;
  if (ESPCONN_OK == espconn_gethostbyname(pesp_conn, domain, &host_ip, self->compMsgSocket->socketDnsFound)) {
    COMP_MSG_DBG(self, "N", 1, "call gethostbyname: found ip for %s 0x%08x", domain, host_ip);
    self->compMsgSocket->socketDnsFound(domain, &host_ip, pesp_conn);  // ip is returned in host_ip.
  }
  *sudOut = sud;
//ets_printf("openSSDSocket end: sud: %p pesp_conn: %p host_ip.addr: 0x%08x remote_ip: 0x%2x 0x%02x 0x%02x 0x%02x\n", sud, pesp_conn, host_ip.addr, pesp_conn->proto.udp->remote_ip[0], pesp_conn->proto.udp->remote_ip[1], pesp_conn->proto.udp->remote_ip[2], pesp_conn->proto.udp->remote_ip[3]);
  return COMP_MSG_ERR_OK;
}

// ================================= sendSSDPInfo  ====================================

static void sendSSDPInfo(void *arg) {
  uint8_t result;
  char buf[1024];
  char ssdp_notify_2[50];
  char ssdp_notify_4[50];
  char ssdp_notify_6[50];
  uint8_t *stringValue;
  struct ip_info pTempIp;
  socketUserData_t *sud2;
  compMsgTimerSlot_t *compMsgTimerSlot;
  compMsgDispatcher_t *self;
  compMsgTimerSlot_t *tmr;
  char temp[64];
  char *domain;
  uint8_t timerId;
  char *tenant;
  char *guid;
  int port;
  uint8_t flags;
  uint8_t *strValue;
  dataValue_t dataValue;
  fieldValueCallback_t callback;

//ets_printf("free heap 5: %d\n", system_get_free_heap_size());
//ets_printf("sendSSDPInfo: arg: %p\n", arg);
  compMsgTimerSlot = (compMsgTimerSlot_t *)arg;
  self = compMsgTimerSlot->compMsgDispatcher;
numNotifies++;
if (numNotifies > 10) {
  timerId = compMsgTimerSlot->timerId;
  tmr = &self->compMsgTimer->compMsgTimers[timerId];
  tmr->mode |= TIMER_IDLE_FLAG;
  ets_timer_disarm(&tmr->timer);
//ets_printf("notify disarm done\n");
  return;
}
//ets_printf("sendSSDPInfo: ssdpSud: %p\n", self->ssdpSud);
  domain = "239.255.255.250";
  if (self->ssdpSud == NULL) {
    result = openSSDPSocket(self, domain, &self->ssdpSud);
//ets_printf("openSSDPSocket result: %d\n", result);
//  checkErrOK(result);
  }
//ets_printf("sendSSDPInfo local_ip: %d.%d.%d.%d remote_ip: %d.%d.%d.%d\n", IP2STR(self->ssdpSud->pesp_conn->proto.udp->local_ip), IP2STR(self->ssdpSud->pesp_conn->proto.udp->remote_ip));
  dataValue.flags = COMP_MSG_FIELD_IS_NUMERIC;
  dataValue.value.numericValue = 0;
  dataValue.cmdKey = COMP_MSG_DATA_VALUE_CMD_KEY_SPECIAL;
  dataValue.fieldNameId = 0;
  dataValue.fieldValueId = COMP_MSG_WIFI_VALUE_ID_clientPort;
  dataValue.fieldValueCallback = NULL;
  result = self->compMsgDataValue->getDataValue(self, &dataValue, &strValue);
//  checkErrOK(result);
  port = dataValue.value.numericValue;
  wifi_get_ip_info(STATION_IF, &pTempIp);
  if(pTempIp.ip.addr==0){
//ets_printf("sendSSDPInfo: after openSSDPSocket: CANNOT_GET_IP\n");
//    return COMP_MSG_ERR_CANNOT_GET_IP;
  }
  c_sprintf(temp, "%d.%d.%d.%d", IP2STR(&pTempIp.ip) );
//ets_printf("sendSSDP my ipaddr: %d.%d.%d.%d", IP2STR(&pTempIp.ip));
  ets_sprintf(ssdp_notify_2, "%s:%d", temp, port);
tenant = "K-fee";
  ets_sprintf(ssdp_notify_4, "%s", tenant);
guid = "1d57ca68-4c98-4e32-80c7-350e33f9b5c07";
  ets_sprintf(ssdp_notify_6, "%s", guid);
  ets_sprintf(buf, "%s%s%s%s%s%s", ssdp_notify_1, ssdp_notify_2, ssdp_notify_3, ssdp_notify_4, ssdp_notify_5, ssdp_notify_6);
char buf2[115];
ets_snprintf(buf2, 115, ">>%s\0", buf);
//ets_printf("%s\n", buf2);
//ets_printf(".");
    COMP_MSG_DBG(self, "N", 1, ".");
//ets_printf("free heap 6: %d\n", system_get_free_heap_size());
//ets_printf("buf: %s\n", buf);
  result = self->compMsgSocket->netSocketSendData(self->ssdpSud, buf, strlen(buf));
//ets_printf("sendSSDPInfo end\n");
}

// ================================= startSendSSDPInfo ====================================

static uint8_t startSendSSDPInfo(compMsgDispatcher_t *self) {
  bool boolResult;
  uint8_t result;
  uint8_t *ssid;
  uint8_t *passwd;
  uint8_t *strValue;
  dataValue_t dataValue;
  int status;

  COMP_MSG_DBG(self, "Y", 1, ">>net startSendSSDPInfo\n");
  status = wifi_station_get_connect_status();
ets_printf("startSenSSDPInfo connectStatus: %d\n", status);
  boolResult = wifi_station_disconnect();
  COMP_MSG_DBG(self, "N", 1, "wifi_station_disconnect: boolResult: %d", boolResult);
ssid = "Wiedemann3";
  dataValue.flags = COMP_MSG_FIELD_IS_STRING;
  dataValue.value.stringValue = ssid;
  dataValue.cmdKey = COMP_MSG_DATA_VALUE_CMD_KEY_SPECIAL;
  dataValue.fieldNameId = 0;
  dataValue.fieldValueId = COMP_MSG_WIFI_VALUE_ID_clientSsid;
  dataValue.fieldValueCallback = NULL;
result = self->compMsgDataValue->setDataValue(self, &dataValue);
checkErrOK(result);
passwd = "123";
  dataValue.flags = COMP_MSG_FIELD_IS_STRING;
  dataValue.value.stringValue = passwd;
  dataValue.cmdKey = COMP_MSG_DATA_VALUE_CMD_KEY_SPECIAL;
  dataValue.fieldNameId = 0;
  dataValue.fieldValueId = COMP_MSG_WIFI_VALUE_ID_clientPasswd;
  dataValue.fieldValueCallback = NULL;
result = self->compMsgDataValue->setDataValue(self, &dataValue);
checkErrOK(result);
self->compMsgSendReceive->startSendMsg = NULL;
  wifi_set_opmode(STATION_MODE);
//ets_printf(">>>call netSocketRunClientMode %p!\n", self->compMsgSocket->netSocketRunClientMode);
result = self->compMsgSocket->netSocketRunClientMode(self);
COMP_MSG_DBG(self, "Y", 1, "startSendSSDPInfo: result: %d\n", result);
checkErrOK(result);

  self->dispatcherCommon->runningModeFlags |= COMP_DISP_RUNNING_MODE_SSDP;
//ets_printf("startSendSSDPInfo end after startConnectionTimer result: %d\n", result);
  return COMP_MSG_ERR_OK;
}

// ================================= compMsgSSDPSocketInit ====================================

uint8_t compMsgSSDPSocketInit(compMsgDispatcher_t *self) {
  uint8_t result;

  self->compMsgSocket->sendSSDPInfo = &sendSSDPInfo;
  self->compMsgSocket->startSendSSDPInfo = &startSendSSDPInfo;
  self->compMsgSocket->netSocketSSDPRecv = &netSocketSSDPRecv;
  return COMP_MSG_ERR_OK;
}
