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
#include "structmsg.h"


enum websocket_opcode {
  OPCODE_TEXT = 1,
  OPCODE_BINARY = 2,
  OPCODE_CLOSE = 8,
  OPCODE_PING = 9,
  OPCODE_PONG = 10,
};

#define TCP ESPCONN_TCP

typedef struct socketInfo {
  struct espconn *pesp_conn;
} socketInfo_t;

#define MAX_SOCKET 5
static int socket_num = 0;
static socketInfo_t *socket[MAX_SOCKET] = { NULL, NULL, NULL, NULL, NULL };

static const char *header_key = "Sec-WebSocket-Key: ";
static const char *ws_uuid ="258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

static const char *HEADER_WEBSOCKETLINE = "Upgrade: websocket";

static char *HEADER_OK = "HTTP/1.x 200 OK \r\n\
Server: ESP \r\n\
Connection: close \r\n\
Cache-Control: max-age=3600, public \r\n\
Content-Type: text/html \r\n\
Content-Encoding: gzip \r\n\r\n";

static char *HEADER_WEBSOCKET_START = "\
HTTP/1.1 101 WebSocket Protocol Handshake\r\n\
Connection: Upgrade\r\n\
Upgrade: WebSocket\r\n\
Access-Control-Allow-Origin: http://";

static char *HEADER_WEBSOCKET_URL = "192.168.178.67";

static char *HEADER_WEBSOCKET_END = "\r\n\
Access-Control-Allow-Credentials: true\r\n\
Access-Control-Allow-Headers: content-type \r\n\
Sec-WebSocket-Accept: ";

static uint8_t err_opcode[5] = {0};

// #define checkAllocOK(addr) if(addr == NULL) checkErrOK(WEBSOCKET_ERR_OUT_OF_MEMORY, "")

enum structmsg_error_code
{
  WEBSOCKET_ERR_OK = 0,
  WEBSOCKET_ERR_OUT_OF_MEMORY = -1,
  WEBSOCKET_ERR_TOO_MUCH_DATA = -2,
  WEBSOCKET_ERR_INVALID_FRAME_OPCODE = -3,
  WEBSOCKET_ERR_USERDATA_IS_NIL = -4,
  WEBSOCKET_ERR_WRONG_METATABLE = -5,
  WEBSOCKET_ERR_PESP_CONN_IS_NIL = -6,
  WEBSOCKET_ERR_MAX_SOCKET_REACHED = -7,
};

typedef struct websocketUserData {
  struct espconn *pesp_conn;
  uint8_t isWebsocket;
  uint8_t num_urls;
  uint8_t max_urls;
  char **urls; // that is the array of url parts which is used in socket_on for the different receive callbacks
  char *curr_url; // that is url which has been provided in the received data
  structmsgDispatcher_t *structmsgDispatcher;
  websockeBinaryReceived_t websocketBinaryReceived;
  websockeTextReceived_t websocketTextReceived;
} websocketUserData_t;

static uint8_t websocket_writeData( const char *payload, int size, websocketUserData_t *wud, int opcode);

#define BASE64_INVALID '\xff'
#define BASE64_PADDING '='
#define ISBASE64(c) (unbytes64[c] != BASE64_INVALID)

#define SSL_BUFFER_SIZE 5120

static const uint8 b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

typedef void (* websockeBinaryReceived_t)(void *arg, char *pdata, unsigned short len);
typedef void (* websockeTextReceived_t)(void *arg, char *pdata, unsigned short len);

// ============================ websocket_parse =========================================

static int ICACHE_FLASH_ATTR websocket_parse(char * data, size_t dataLenb, char **resData, int *len, websocketUserData_t *wud) {
  uint8_t byte = data[0];
  uint8_t FIN = byte & 0x80;
  uint8_t opcode = byte & 0x0F;

  if ((opcode > 0x03 && opcode < 0x08) || opcode > 0x0B) {
    ets_sprintf(err_opcode, "%d", opcode);
//    checkErrOK(gL, WEBSOCKET_ERR_INVALID_FRAME_OPCODE, err_opcode);
  }

  //   opcodes: {1 text 2 binary 8 close 9 ping 10 pong}
  switch (opcode) {
  case OPCODE_TEXT:
//ets_printf("parse text\n");
    break;
  case OPCODE_BINARY:
//ets_printf("parse binary\n");
    break;
  case OPCODE_CLOSE:
    break;
  case OPCODE_PING:
    websocket_writeData(" ", 1, wud, OPCODE_PONG);
    return WEBSOCKET_ERR_OK;
    break;
  case OPCODE_PONG:
    return WEBSOCKET_ERR_OK;
    break;
  }
  byte = data[1];

  char * recv_data;
  int masked = byte & 0x80;
  int size = byte & 0x7F;
  int offset = 2;

  if (size == 126) {
    size = 0;
    size = data[3];                  //LSB
    size |= (uint64_t) data[2] << 8; //MSB
    offset = 4;
  } else if (size == 127) {
    size = 0;
    size |= (uint64_t) data[2] << 56;
    size |= (uint64_t) data[3] << 48;
    size |= (uint64_t) data[4] << 40;
    size |= (uint64_t) data[5] << 32;
    size |= (uint64_t) data[6] << 24;
    size |= (uint64_t) data[7] << 16;
    size |= (uint64_t) data[8] << 8;
    size |= (uint64_t) data[9];
    offset = 10;
  }

  if (masked) {
    //read mask key
    char mask[4];
    uint64_t i;

    mask[0] = data[offset];
    mask[1] = data[offset + 1];
    mask[2] = data[offset + 2];
    mask[3] = data[offset + 3];
    offset += 4;
    for (i = 0; i < size; i++) {
      data[i + offset] ^= mask[i % 4];
    }
  }
  recv_data = &data[offset];
for (int i = 0; i < size; i++) {
//  ets_printf("i: %d 0x%02x\n", i, recv_data[i]&0xFF);
}
  switch (opcode) {
  case OPCODE_TEXT:
ets_printf("cb text\n");
    wud->websocketTextReceived(wud->structmsgDispatcher, recv_data, size);
    break;
  case OPCODE_BINARY:
    wud->websocketBinaryReceived(wud->structmsgDispatcher, recv_data, size);
    break;
  }
  *resData = recv_data;
  *len = size;
  recv_data[size] = 0;

  if (size + offset < dataLenb) {
    websocket_parse(&data[size + offset], dataLenb - (size + offset), resData, len, wud);
  }
  return WEBSOCKET_ERR_OK;
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

// ============================ websocket_writeData =======================

static uint8_t websocket_writeData( const char *payload, int size, websocketUserData_t *wud, int opcode)
{
  uint8_t hdrBytes[4]; // we have either 2 or 4 bytes depending on length of message
  int hdrLgth;
  int fsize;
  uint8_t*buff;
  int i;

  hdrLgth = 2;
  hdrBytes[0] = 0x80; //set first bit
  hdrBytes[0] |= opcode; //frame->opcode; //set op code
  if (size < 126) {
    hdrBytes[1] = size;
  } else {
    if (size < SSL_BUFFER_SIZE - sizeof(hdrBytes)) {
      hdrLgth += 2;
      hdrBytes[1] = 126;
      hdrBytes[2] = (size >> 8) & 0xFF;
      hdrBytes[3] = size & 0xFF;
    } else {
      return WEBSOCKET_ERR_TOO_MUCH_DATA;
    }
  }
  fsize = size + hdrLgth;
  buff = os_malloc(fsize);
  if (buff == NULL) {
    return WEBSOCKET_ERR_OUT_OF_MEMORY;
  }
  for (i = 0; i < hdrLgth; i++) {
    buff[i] = hdrBytes[i];
  }

  os_memcpy(&buff[hdrLgth], (uint8_t *)payload, size);
  espconn_sent(wud->pesp_conn, (unsigned char *)buff, fsize);
  os_free(buff);
  return WEBSOCKET_ERR_OK;
}


// ================================= websocket_recv ====================================

static uint8_t websocket_recv(char *string, websocketUserData_t *wud, char **data, int *lgth) {
  char * key = NULL;
  int idx;
  int found;

  idx = 0;
//ets_printf("websocket_recv: %s!%s!\n",string,  wud->curr_url);
  if ((wud->curr_url != NULL) && (strstr(string, wud->curr_url) != NULL)) {
    if (strstr(string, header_key) != NULL) {
      char *begin = strstr(string, header_key) + os_strlen(header_key);
      char *end = strstr(begin, "\r");
      key = os_malloc((end - begin) + 1);
//      checkAllocgLOK(key);
      os_memcpy(key, begin, end - begin);
      key[end - begin] = 0;
    }
//ets_printf("websocket_recv2: key: %s\n", key);
    const char *trailer;
    trailer = "\r\n\r\n";
    int trailerLen;
    trailerLen = os_strlen(trailer);
    size_t digestLen;
    digestLen = 20; //sha1 is always 20 byte long
    uint8_t digest[digestLen];
    int payloadLen;
    char *payload;
    uint8_t *base64Digest;
    SHA1_CTX ctx;
    // Use the SHA* functions in the rom
    SHA1Init(&ctx);
    SHA1Update(&ctx, key, os_strlen(key));
    SHA1Update(&ctx, ws_uuid, os_strlen(ws_uuid));
    SHA1Final(digest, &ctx);

    base64Digest = toBase64(digest, &digestLen);
//    checkAllocgLOK(base64Digest);
    payloadLen = os_strlen(HEADER_WEBSOCKET_START) + os_strlen(HEADER_WEBSOCKET_URL) +os_strlen(HEADER_WEBSOCKET_END) + digestLen + trailerLen;
    payload = os_malloc(payloadLen);
//    checkAllocgLOK(payload);
    os_sprintf(payload, "%s%s%s%s%s\0", HEADER_WEBSOCKET_START, HEADER_WEBSOCKET_URL, HEADER_WEBSOCKET_END, base64Digest, trailer);
    os_free(base64Digest);
    struct espconn *pesp_conn = NULL;
    pesp_conn = wud->pesp_conn;

    // FIXME!! reboot called if setting here for socket!!
    if (wud->isWebsocket != 1) {
      wud->isWebsocket = 1;
    }

//ets_printf("payload: %d!%s!\n", payloadLen, payload);
    int result = espconn_sent(wud->pesp_conn, (unsigned char *)payload, payloadLen);
    os_free(key);
//    checkErrOK(gL, result, "espconn_sent");
  } else if (wud->isWebsocket == 1) {
//ets_printf("websocket_parse: %d!%s!curr_ulr: %s!\n", os_strlen(string), string, wud->curr_url);
    websocket_parse(string, os_strlen(string), data, lgth, wud);
  }
  return WEBSOCKET_ERR_OK;

}

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
  char url[50] = { 0 };
  int idx;
  websocketUserData_t *wud;

  pesp_conn = (struct espconn *)arg;
//ets_printf("socketReceived: arg: %p pdata: %s len: %d\n", arg, pdata, len);
  char temp[20] = {0};
  c_sprintf(temp, IPSTR, IP2STR( &(pesp_conn->proto.tcp->remote_ip) ) );
  ets_printf("remote ");
  ets_printf(temp);
  ets_printf(":");
  ets_printf("%d",pesp_conn->proto.tcp->remote_port);
  ets_printf(" received.\n");


  wud = (websocketUserData_t *)pesp_conn->reverse;
  if (strstr(pdata, "GET /") != 0) {
    char *begin = strstr(pdata, "GET /") + 4;
    char *end = strstr(begin, " ");
    os_memcpy(url, begin, end - begin);
    url[end - begin] = 0;
  }
  if ((url[0] != 0) && (strstr(pdata, HEADER_WEBSOCKETLINE) != 0)) {
    idx = 0;
    wud->curr_url = NULL;
    while (idx < wud->num_urls) {
      if (c_strcmp(url, wud->urls[idx]) == 0) {
        wud->curr_url = wud->urls[idx];
        wud->isWebsocket = 1;
        break;
      }
      idx++;
    }
  }
//ets_printf("iswebsocket: %d %s\n", wud->isWebsocket, wud->curr_url);

  if(wud->isWebsocket == 1) {
    char *data = "";
    int lgth = 0;
    int result;

    result = websocket_recv(pdata, wud, &data, &lgth);
//    checkErrOK(gL,result,"websocket_recv");
  }
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
//ets_printf("registstart\n");
  result = espconn_regist_recvcb(pesp_conn, socketReceived);
  if (result != STRUCT_DISP_ERR_OK) {
//ets_printf("regist socketReceived err: %d\n", result);
  }
  result = espconn_regist_sentcb(pesp_conn, socketSent);
  if (result != STRUCT_DISP_ERR_OK) {
//ets_printf("regist socketSent err: %d\n", result);
  }
  result = espconn_regist_disconcb(pesp_conn, serverDisconnected);
  if (result != STRUCT_DISP_ERR_OK) {
//ets_printf("regist serverDisconnected err: %d\n", result);
  }
  result = espconn_regist_reconcb(pesp_conn, serverReconnected);
  if (result != STRUCT_DISP_ERR_OK) {
//ets_printf("regist serverReconnected err: %d\n", result);
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
  unsigned type;
  int result;
  websocketUserData_t *wud;

  timerId = (uint8_t)((uint32_t)arg);
  tmr = &structmsgTimers[timerId];
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

  result = self->getModuleValue(self, MODULE_INFO_PROVISIONING_PORT, DATA_VIEW_FIELD_UINT8_T, &numericValue, &stringValue);
//ets_printf("port: %d!%p!%d!\n", numericValue, stringValue, result);
//  checkErrOK(result);
  port = numericValue;
//  result = self->getModuleValue(self, MODULE_INFO_PROVISIONING_IP_ADDR, DATA_VIEW_FIELD_UINT8_VECTOR, &numericValue, &stringValue);
//  checkErrOK(result);

  wud = (websocketUserData_t *)os_zalloc(sizeof(websocketUserData_t));
//   checkAllocOK(wud);
//ets_printf("wud0: %p\n", wud);
  wud->isWebsocket = 0;
  wud->num_urls = 0;
  wud->max_urls = 4;
  wud->urls = (char **)c_zalloc(sizeof(char *) * wud->max_urls);
//  checkAllocgLOK(wud->urls);
  wud->urls[0] = NULL;
  wud->curr_url = NULL;
wud->urls[0] = "/getaplist";
wud->urls[1] = "/getapdeflist";
wud->num_urls = 2;
  result = self->getModuleValue(self, MODULE_INFO_BINARY_CALL_BACK, DATA_VIEW_FIELD_UINT32_T, &numericValue, &stringValue);
//ets_printf("binaryCallback: %p!%d!\n", numericValue, result);
  wud->websocketBinaryReceived = (websockeBinaryReceived_t)numericValue;
  result = self->getModuleValue(self, MODULE_INFO_TEXT_CALL_BACK, DATA_VIEW_FIELD_UINT32_T, &numericValue, &stringValue);
//ets_printf("binaryCallback: %p!%d!\n", numericValue, result);
  wud->websocketTextReceived = (websockeTextReceived_t)numericValue;
  wud->structmsgDispatcher = self;

  pesp_conn = (struct espconn *)os_zalloc(sizeof(struct espconn));
  wud->pesp_conn = pesp_conn;
//  checkAllocOK(pesp_conn);

  type = ESPCONN_TCP;
  pesp_conn->type = type;
  pesp_conn->state = ESPCONN_NONE;
  // reverse is for the callback function
  pesp_conn->reverse = wud;

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

//ets_printf("call regist connectcb\n");
    result = espconn_regist_connectcb(pesp_conn, serverConnected);
    if (result != STRUCT_DISP_ERR_OK) {
//      return STRUCT_DISP_ERR_REGIST_CONNECT_CB;
    }
//ets_printf("regist connectcb result: %d\n", result);
    result = espconn_accept(pesp_conn);    // if it's a server, no need to dns.
    if (result != STRUCT_DISP_ERR_OK) {
//      return STRUCT_DISP_ERR_TCP_ACCEPT;
    }
//ets_printf("regist_accept result: %d\n", result);
    result =espconn_regist_time(pesp_conn, tcp_server_timeover, 0);
    if (result != STRUCT_DISP_ERR_OK) {
//      return STRUCT_DISP_ERR_REGIST_TIME;
    }
//ets_printf("regist_time result: %d\n", result);
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
