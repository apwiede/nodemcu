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
 * this module is mostly derived from the net.c module in removing upd support 
 * and adding websocket code parts from the arduino implementation of 
 * Willem Wouters:
 * see: https://github.com/willemwouters/ESP8266/tree/master/projects/apps/websocket_example/src
 */

// Module for handling websockets

#include "module.h"
#include "lauxlib.h"
#include "platform.h"
#include "lmem.h"

#include "c_string.h"
#include "c_stdlib.h"

#include "c_types.h"
#include "mem.h"
#include "lwip/ip_addr.h"
#include "rom.h"
//#include "websocket_server.h"
#include "osapi.h"
#include "espconn.h"
#include "lwip/dns.h" 

#define TCP ESPCONN_TCP

#define SSL_BUFFER_SIZE 5120

static ip_addr_t host_ip; // for dns

enum websocket_opcode {
  OPCODE_TEXT = 1,
  OPCODE_BINARY = 2,
  OPCODE_CLOSE = 8,
  OPCODE_PING = 9,
  OPCODE_PONG = 10,
};

#define MAX_SOCKET 5
static int socket_num = 0;
static int socket[MAX_SOCKET];
static lua_State *gL = NULL;
static int tcpserver_cb_connect_ref = LUA_NOREF;  // for tcp server connected callback
static uint16_t tcp_server_timeover = 30;
static const uint8_t *errStr = "ERROR";
static uint8_t err_opcode[5] = {0};

static struct espconn *pTcpServer = NULL;
static struct espconn *pUdpServer = NULL;

struct lwebsocket_userdata;

typedef struct lwebsocket_userdata
{
  struct espconn *pesp_conn;
  int self_ref;
  int cb_connect_ref;
  int cb_reconnect_ref;
  int cb_disconnect_ref;
  int cb_receive_ref;
  int cb_send_ref;
  int cb_dns_found_ref;
#ifdef CLIENT_SSL_ENABLE
  uint8_t secure;
#endif
  uint8_t isWebsocket;
  uint8_t num_urls;
  uint8_t max_urls;
  char **urls; // that is the array of url parts which is used in socket_on for the different receive callbacks
  char *curr_url; // that is url which has been provided in the received data
}lwebsocket_userdata;

// Websocket

int ICACHE_FLASH_ATTR websocket_recv(char * string, lwebsocket_userdata *wud, char **resData, int *len);
int ICACHE_FLASH_ATTR websocket_parse(char * data, size_t dataLenb, char **resData, int *len, lwebsocket_userdata *wud);
static int websocket_writeData( const char *payload, int size, lwebsocket_userdata *wud, int opcode );

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

#define BASE64_INVALID '\xff'
#define BASE64_PADDING '='
#define ISBASE64(c) (unbytes64[c] != BASE64_INVALID)

static const uint8 b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// End Websocket

#define checkAllocgLOK(addr) if(addr == NULL) checkErrOK(gL, WEBSOCKET_ERR_OUT_OF_MEMORY, "")
#define checkAllocOK(addr) if(addr == NULL) checkErrOK(L, WEBSOCKET_ERR_OUT_OF_MEMORY, "")

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

// ============================= checkErrOK ========================

static int checkErrOK( lua_State* L, int result , const char *where ) {
  if (result == WEBSOCKET_ERR_OK) {
    return 1;
  }
  switch (result) {
  case WEBSOCKET_ERR_OUT_OF_MEMORY:
    luaL_error(L, "out of memory (%s)", where);
    break;
  case WEBSOCKET_ERR_TOO_MUCH_DATA:
    lua_pushfstring(L, "%s: too much data (%s)", errStr, where);
    break;
  case WEBSOCKET_ERR_INVALID_FRAME_OPCODE:
    lua_pushfstring(L, "%s: invalid frame opcode (%s)", errStr, where);
    break;
  case WEBSOCKET_ERR_USERDATA_IS_NIL:
    lua_pushfstring(L, "%s: userdata is nil (%s)", errStr, where);
    break;
  case WEBSOCKET_ERR_WRONG_METATABLE:
    lua_pushfstring(L, "%s: wrong metatable (%s)", errStr, where);
    break;
  case WEBSOCKET_ERR_PESP_CONN_IS_NIL:
    lua_pushfstring(L, "%s: pesp_conn is nil (%s)", errStr, where);
    break;
  case WEBSOCKET_ERR_MAX_SOCKET_REACHED:
    lua_pushfstring(L, "%s: max socket reached (%s)", errStr, where);
    break;
  default:
    lua_pushfstring(L, "%s: error in %s: result: %d", errStr, where, result);
    break;
  }
  return 0;
}

// ============================ websocket_parse =========================================

int ICACHE_FLASH_ATTR websocket_parse(char * data, size_t dataLenb, char **resData, int *len, lwebsocket_userdata *wud) {
  uint8_t byte = data[0];
  uint8_t FIN = byte & 0x80;
  uint8_t opcode = byte & 0x0F;

  if ((opcode > 0x03 && opcode < 0x08) || opcode > 0x0B) {
    ets_sprintf(err_opcode, "%d", opcode);
    checkErrOK(gL, WEBSOCKET_ERR_INVALID_FRAME_OPCODE, err_opcode);
  }

  //   opcodes: {1 text 2 binary 8 close 9 ping 10 pong}
  switch (opcode) {
  case OPCODE_TEXT:
    break;
  case OPCODE_BINARY:
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
    *resData = recv_data;
    *len = size;
    recv_data[size] = 0;

    if (size > 0) {
      if(wud->cb_receive_ref != LUA_NOREF && wud->self_ref != LUA_NOREF) {
        lua_rawgeti(gL, LUA_REGISTRYINDEX, wud->cb_receive_ref);
        lua_rawgeti(gL, LUA_REGISTRYINDEX, wud->self_ref);  // pass the userdata(server) to callback func in lua
        lua_pushlstring(gL, recv_data, size);
        lua_pushlstring(gL, wud->curr_url, c_strlen(wud->curr_url));
        lua_call(gL, 3, 0);
      }
    }

    if (size + offset < dataLenb) {
      websocket_parse(&data[size + offset], dataLenb - (size + offset), resData, len, wud);
    }
  return WEBSOCKET_ERR_OK;
}

// ============================ toBase64 =========================================

static uint8 *toBase64 ( const uint8 *msg, size_t *len){
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


// ============================ websocket_recv =========================================

int ICACHE_FLASH_ATTR websocket_recv(char * string, lwebsocket_userdata *wud, char **data, int *lgth) {
  char * key;
  int idx;
  int found;

  idx = 0;
  if ((wud->curr_url != NULL) && (strstr(string, wud->curr_url) != NULL)) {
    if (strstr(string, header_key) != NULL) {
      char *begin = strstr(string, header_key) + os_strlen(header_key);
      char *end = strstr(begin, "\r");
      key = os_malloc((end - begin) + 1);
      checkAllocgLOK(key);
      os_memcpy(key, begin, end - begin);
      key[end - begin] = 0;
    }
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
    checkAllocgLOK(base64Digest);
    payloadLen = os_strlen(HEADER_WEBSOCKET_START) + os_strlen(HEADER_WEBSOCKET_URL) +os_strlen(HEADER_WEBSOCKET_END) + digestLen + trailerLen;
    payload = os_malloc(payloadLen);
    checkAllocgLOK(payload);
    os_sprintf(payload, "%s%s%s%s%s\0", HEADER_WEBSOCKET_START, HEADER_WEBSOCKET_URL, HEADER_WEBSOCKET_END, base64Digest, trailer);
    os_free(base64Digest);
    struct espconn *pesp_conn = NULL;
    pesp_conn = wud->pesp_conn;

    // FIXME!! reboot called if setting here for socket!!
    if (wud->isWebsocket != 1) {
      wud->isWebsocket = 1;
    }

    int result = espconn_sent(wud->pesp_conn, (unsigned char *)payload, payloadLen);
    os_free(key);
    checkErrOK(gL, result, "espconn_sent");
  } else if (wud->isWebsocket == 1) {
    websocket_parse(string, os_strlen(string), data, lgth, wud);
  }
  return WEBSOCKET_ERR_OK;
}

// ============================ websocket_server_disconnected =======================

static void websocket_server_disconnected(void *arg)    // for tcp server only
{
  NODE_DBG("websocket_server_disconnected is called.\n");
  struct espconn *pesp_conn = arg;
  if(gL == NULL) {
    return;
  }
  if(pesp_conn == NULL) {
    checkErrOK(gL, WEBSOCKET_ERR_PESP_CONN_IS_NIL, "websocket_server_disconnected");
    return;
  }
  lwebsocket_userdata *wud = (lwebsocket_userdata *)pesp_conn->reverse;
  if(wud == NULL) {
    checkErrOK(gL, WEBSOCKET_ERR_USERDATA_IS_NIL, "websocket_server_disconnected");
    return;
  }
#if 0
  char temp[20] = {0};
  c_sprintf(temp, IPSTR, IP2STR( &(pesp_conn->proto.tcp->remote_ip) ) );
  NODE_DBG("remote ");
  NODE_DBG(temp);
  NODE_DBG(":");
  NODE_DBG("%d",pesp_conn->proto.tcp->remote_port);
  NODE_DBG(" disconnected.\n");
#endif
  if(wud->cb_disconnect_ref != LUA_NOREF && wud->self_ref != LUA_NOREF)
  {
    lua_rawgeti(gL, LUA_REGISTRYINDEX, wud->cb_disconnect_ref);
    lua_rawgeti(gL, LUA_REGISTRYINDEX, wud->self_ref);  // pass the userdata(client) to callback func in lua
    lua_call(gL, 1, 0);
  }
  int i;
  lua_gc(gL, LUA_GCSTOP, 0);
  for(i=0;i<MAX_SOCKET;i++){
    if( (LUA_NOREF!=socket[i]) && (socket[i] == wud->self_ref) ){
      // found the saved client
      wud->pesp_conn->reverse = NULL;
      wud->pesp_conn = NULL;    // the espconn is made by low level sdk, do not need to free, delete() will not free it.
      wud->self_ref = LUA_NOREF;   // unref this, and the net.socket userdata will delete it self
      luaL_unref(gL, LUA_REGISTRYINDEX, socket[i]);
      socket[i] = LUA_NOREF;
      socket_num--;
      break;
    }
  }
  lua_gc(gL, LUA_GCRESTART, 0);
}

// ============================ websocket_socket_disconnected =======================

static void websocket_socket_disconnected(void *arg)    // tcp only
{
  NODE_DBG("websocket_socket_disconnected is called.\n");
  struct espconn *pesp_conn = arg;
  if(pesp_conn == NULL) {
    checkErrOK(gL, WEBSOCKET_ERR_PESP_CONN_IS_NIL, "websocket_socket_disconnected");
    return;
  }
  lwebsocket_userdata *wud = (lwebsocket_userdata *)pesp_conn->reverse;
  if(wud == NULL) {
    checkErrOK(gL, WEBSOCKET_ERR_USERDATA_IS_NIL, "websocket_socket_disconnected");
    return;
  }
  if(wud->cb_disconnect_ref != LUA_NOREF && wud->self_ref != LUA_NOREF)
  {
    lua_rawgeti(gL, LUA_REGISTRYINDEX, wud->cb_disconnect_ref);
    lua_rawgeti(gL, LUA_REGISTRYINDEX, wud->self_ref);  // pass the userdata(client) to callback func in lua
    lua_call(gL, 1, 0);
  }

  if(pesp_conn->proto.tcp)
    c_free(pesp_conn->proto.tcp);
  pesp_conn->proto.tcp = NULL;
  if(wud->pesp_conn)
    c_free(wud->pesp_conn);
  wud->pesp_conn = NULL;  // espconn is already disconnected
  lua_gc(gL, LUA_GCSTOP, 0);
  if(wud->self_ref != LUA_NOREF){
    luaL_unref(gL, LUA_REGISTRYINDEX, wud->self_ref);
    wud->self_ref = LUA_NOREF; // unref this, and the net.socket userdata will delete it self
  }
  lua_gc(gL, LUA_GCRESTART, 0);
}

// ============================ websocket_server_reconnected =======================

static void websocket_server_reconnected(void *arg, sint8_t err)
{
  NODE_DBG("websocket_server_reconnected is called.\n");
  websocket_server_disconnected(arg);
}

// ============================ websocket_socket_reconnected =======================

static void websocket_socket_reconnected(void *arg, sint8_t err)
{
  NODE_DBG("websocket_socket_reconnected is called.\n");
  websocket_socket_disconnected(arg);
}

// ============================ websocket_socket_received =======================

static void websocket_socket_received(void *arg, char *pdata, unsigned short len)
{
  NODE_DBG("websocket_socket_received is called.\n");
  struct espconn *pesp_conn = arg;
  char url[50] = { 0 };
  int idx;

  if(pesp_conn == NULL) {
    checkErrOK(gL, WEBSOCKET_ERR_PESP_CONN_IS_NIL, "websocket_socket_received");
    return;
  }
  lwebsocket_userdata *wud = (lwebsocket_userdata *)pesp_conn->reverse;
  if(wud == NULL) {
    checkErrOK(gL, WEBSOCKET_ERR_USERDATA_IS_NIL, "websocket_socket_received");
    return;
  }
  if(wud->cb_receive_ref == LUA_NOREF)
    return;
  if(wud->self_ref == LUA_NOREF)
    return;
// Websocket
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

  if(wud->isWebsocket == 1) {
    char *data = "";
    int lgth = 0;
    int result;

    result = websocket_recv(pdata, wud, &data, &lgth);
    checkErrOK(gL,result,"websocket_recv");
// End Websocket
// Websocket
  } else {
// End Websocket
    if ((strstr(pdata, HEADER_WEBSOCKETLINE) != 0)) {
      wud->isWebsocket = 1;
    }
    lua_rawgeti(gL, LUA_REGISTRYINDEX, wud->cb_receive_ref);
    lua_rawgeti(gL, LUA_REGISTRYINDEX, wud->self_ref);  // pass the userdata(server) to callback func in lua
    lua_pushstring(gL, wud->curr_url);
    lua_pushlstring(gL, pdata, len);
    lua_call(gL, 2, 0);
// Websocket
  }
// End Websocket
}

// ============================ websocket_socket_sent =======================

static void websocket_socket_sent(void *arg)
{
  // NODE_DBG("websocket_socket_sent is called.\n");
  struct espconn *pesp_conn = arg;
  if(pesp_conn == NULL) {
    checkErrOK(gL, WEBSOCKET_ERR_PESP_CONN_IS_NIL, "websocket_socket_sent");
    return;
  }
  lwebsocket_userdata *wud = (lwebsocket_userdata *)pesp_conn->reverse;
  if(wud == NULL) {
    checkErrOK(gL, WEBSOCKET_ERR_USERDATA_IS_NIL, "websocket_socket_sent");
    return;
  }
  if(wud->cb_send_ref == LUA_NOREF)
    return;
  if(wud->self_ref == LUA_NOREF)
    return;
  lua_rawgeti(gL, LUA_REGISTRYINDEX, wud->cb_send_ref);
  lua_rawgeti(gL, LUA_REGISTRYINDEX, wud->self_ref);  // pass the userdata(server) to callback func in lua
  lua_call(gL, 1, 0);
}

// ============================ socket_connect =======================

static void socket_connect(struct espconn *pesp_conn)
{
  if(pesp_conn == NULL) {
    checkErrOK(gL, WEBSOCKET_ERR_PESP_CONN_IS_NIL, "socket_connect");
    return;
  }
  lwebsocket_userdata *wud = (lwebsocket_userdata *)pesp_conn->reverse;
  if(wud == NULL) {
    checkErrOK(gL, WEBSOCKET_ERR_USERDATA_IS_NIL, "socket_connect");
    return;
  }

#ifdef CLIENT_SSL_ENABLE
  if(wud->secure){
    espconn_secure_set_size(ESPCONN_CLIENT, SSL_BUFFER_SIZE); /* set SSL buffer size */
    espconn_secure_connect(pesp_conn);
  }
  else
#endif
  {
    espconn_connect(pesp_conn);
  }
  NODE_DBG("socket_connect is called.\n");
}

// ============================ socket_dns_found =======================

static void socket_dns_found(const char *name, ip_addr_t *ipaddr, void *arg);
static int dns_reconn_count = 0;
static void socket_dns_found(const char *name, ip_addr_t *ipaddr, void *arg)
{
  NODE_DBG("websocket_dns_found is called.\n");
  struct espconn *pesp_conn = arg;
  if(pesp_conn == NULL){
    NODE_DBG("pesp_conn null.\n");
    checkErrOK(gL, WEBSOCKET_ERR_PESP_CONN_IS_NIL, "socket_dns_found");
    return;
  }
  lwebsocket_userdata *wud = (lwebsocket_userdata *)pesp_conn->reverse;
  if(wud == NULL){
    NODE_DBG("wud null.\n");
    checkErrOK(gL, WEBSOCKET_ERR_USERDATA_IS_NIL, "socket_dns_found");
    return;
  }
  if(wud->cb_dns_found_ref == LUA_NOREF){
    NODE_DBG("cb_dns_found_ref null.\n");
    return;
  }

  if(wud->self_ref == LUA_NOREF){
    NODE_DBG("self_ref null.\n");
    return;
  }

  lua_rawgeti(gL, LUA_REGISTRYINDEX, wud->cb_dns_found_ref);    // the callback function
  lua_rawgeti(gL, LUA_REGISTRYINDEX, wud->self_ref);  // pass the userdata(conn) to callback func in lua

  if(ipaddr == NULL)
  {
    NODE_DBG( "DNS Fail!\n" );
    lua_pushnil(gL);
  }else{
    // ipaddr->addr is a uint32_t ip
    char ip_str[20];
    c_memset(ip_str, 0, sizeof(ip_str));
    if(ipaddr->addr != 0)
    {
      c_sprintf(ip_str, IPSTR, IP2STR(&(ipaddr->addr)));
    }
    lua_pushstring(gL, ip_str);   // the ip para
  }
  // "enhanced" end

  lua_call(gL, 2, 0);

end:
  if(pesp_conn->proto.tcp->remote_port == 0){
    lua_gc(gL, LUA_GCSTOP, 0);
    if(wud->self_ref != LUA_NOREF){
      luaL_unref(gL, LUA_REGISTRYINDEX, wud->self_ref);
      wud->self_ref = LUA_NOREF; // unref this, and the net.socket userdata will delete it self
    }
    lua_gc(gL, LUA_GCRESTART, 0);
  }
}

// ============================ websocket_server_connected =======================

static void websocket_server_connected(void *arg) // for tcp only
{
  NODE_DBG("websocket_server_connected is called.\n");
  struct espconn *pesp_conn = arg;
  lwebsocket_userdata *wud = (lwebsocket_userdata *)pesp_conn->reverse;
  int i = 0;
  lwebsocket_userdata *skt = NULL;
  if(pesp_conn == NULL) {
    checkErrOK(gL, WEBSOCKET_ERR_PESP_CONN_IS_NIL, "websocket_server_connected");
    return;
  }

#if 0
  char temp[20] = {0};
  c_sprintf(temp, IPSTR, IP2STR( &(pesp_conn->proto.tcp->remote_ip) ) );
  NODE_DBG("remote ");
  NODE_DBG(temp);
  NODE_DBG(":");
  NODE_DBG("%d",pesp_conn->proto.tcp->remote_port);
  NODE_DBG(" connected.\n");
#endif

  for(i=0;i<MAX_SOCKET;i++){
    if(socket[i] == LUA_NOREF)  // found empty slot
    {
      break;
    }
  }
  if(i>=MAX_SOCKET) // can't create more socket
  {
    NODE_ERR("MAX_SOCKET\n");
    pesp_conn->reverse = NULL;    // not accept this conn
    if(pesp_conn->proto.tcp->remote_port || pesp_conn->proto.tcp->local_port)
      espconn_disconnect(pesp_conn);
    checkErrOK(gL, WEBSOCKET_ERR_MAX_SOCKET_REACHED, "websocket_server_connected");
    return;
  }

  if(tcpserver_cb_connect_ref == LUA_NOREF)
    return;
  if(!gL)
    return;

  lua_rawgeti(gL, LUA_REGISTRYINDEX, tcpserver_cb_connect_ref);  // get function
  // create a new client object
  skt = (lwebsocket_userdata *)lua_newuserdata(gL, sizeof(lwebsocket_userdata));

  if(!skt){
    NODE_ERR("can't newudata\n");
    lua_pop(gL, 1);
    return;
  }
  // set its metatable
  luaL_getmetatable(gL, "websocket.socket");
  lua_setmetatable(gL, -2);
  // pre-initialize it, in case of errors
  skt->self_ref = LUA_NOREF;
  lua_pushvalue(gL, -1);  // copy the top of stack
  skt->self_ref = luaL_ref(gL, LUA_REGISTRYINDEX);    // ref to it self, for module api to find the userdata
  socket[i] = skt->self_ref;  // save to socket array
  socket_num++;
  skt->cb_connect_ref = LUA_NOREF;  // this socket already connected
  skt->cb_reconnect_ref = LUA_NOREF;
  skt->cb_disconnect_ref = LUA_NOREF;

  skt->cb_receive_ref = LUA_NOREF;
  skt->cb_send_ref = LUA_NOREF;
  skt->cb_dns_found_ref = LUA_NOREF;

#ifdef CLIENT_SSL_ENABLE
  skt->secure = 0;    // as a server SSL is not supported.
#endif
  skt->isWebsocket = wud->isWebsocket;
  skt->num_urls = wud->num_urls;
  skt->max_urls = wud->max_urls;
  skt->curr_url = wud->curr_url;
  skt->urls = wud->urls;

  skt->pesp_conn = pesp_conn;   // point to the espconn made by low level sdk
  pesp_conn->reverse = skt;   // let espcon carray the info of this userdata(net.socket)

  espconn_regist_recvcb(pesp_conn, websocket_socket_received);
  espconn_regist_sentcb(pesp_conn, websocket_socket_sent);
  espconn_regist_disconcb(pesp_conn, websocket_server_disconnected);
  espconn_regist_reconcb(pesp_conn, websocket_server_reconnected);

  // now socket[i] has the client ref, and stack top has the userdata
  lua_call(gL, 1, 0);  // function(conn)
}

// ============================ websocket_socket_connected =======================

static void websocket_socket_connected(void *arg)
{
  NODE_DBG("websocket_socket_connected is called.\n");
  struct espconn *pesp_conn = arg;
  if(pesp_conn == NULL) {
    checkErrOK(gL, WEBSOCKET_ERR_USERDATA_IS_NIL, "websocket_connected");
    return;
  }
  lwebsocket_userdata *wud = (lwebsocket_userdata *)pesp_conn->reverse;
  if(wud == NULL) {
    checkErrOK(gL, WEBSOCKET_ERR_USERDATA_IS_NIL, "websocket_connected");
    return;
  }
  // can receive and send data, even if there is no connected callback in lua.
  espconn_regist_recvcb(pesp_conn, websocket_socket_received);
  espconn_regist_sentcb(pesp_conn, websocket_socket_sent);
  espconn_regist_disconcb(pesp_conn, websocket_socket_disconnected);

  if(wud->cb_connect_ref == LUA_NOREF)
    return;
  if(wud->self_ref == LUA_NOREF)
    return;
  lua_rawgeti(gL, LUA_REGISTRYINDEX, wud->cb_connect_ref);
  lua_rawgeti(gL, LUA_REGISTRYINDEX, wud->self_ref);  // pass the userdata(client) to callback func in lua
  lua_call(gL, 1, 0);
}

// ============================ websocket_create =======================

// Lua: s = websocket.create(secure/timeout,url,function(conn))
static int websocket_create( lua_State* L, const char *mt )
{
  NODE_DBG("websocket_create is called.\n");
  struct espconn *pesp_conn = NULL;
  lwebsocket_userdata *wud, *temp = NULL;
  unsigned type;
#ifdef CLIENT_SSL_ENABLE
  unsigned secure = 0;
#endif
  uint8_t stack = 1;
  bool isserver = false;
  
  if (mt!=NULL && c_strcmp(mt, "websocket.server")==0)
    isserver = true;
  else if (mt!=NULL && c_strcmp(mt, "websocket.socket")==0)
    isserver = false;
  else
  {
    NODE_DBG("wrong metatable for websocket_create.\n");
    checkErrOK(L, WEBSOCKET_ERR_WRONG_METATABLE, "websocket_create");
    return 0;
  }

  type = ESPCONN_TCP;
#ifdef CLIENT_SSL_ENABLE
  if(!isserver){
    if ( lua_isnumber(L, stack) )
    {
      secure = lua_tointeger(L, stack);
      stack++;
      if ( secure != 0 && secure != 1 ){
        return luaL_error( L, "wrong arg type" );
      }
    } else {
      secure = 0; // default to 0
    }
  }
#endif

  if(isserver){
    if ( lua_isnumber(L, stack) )
    {
      unsigned to = lua_tointeger(L, stack);
      stack++;
      if ( to < 1 || to > 28800 ){
        return luaL_error( L, "wrong arg type" );
      }
      tcp_server_timeover = (uint16_t)to;
    } else {
      tcp_server_timeover = 30; // default to 30
    }
  }

  // create a object
  wud = (lwebsocket_userdata *)lua_newuserdata(L, sizeof(lwebsocket_userdata));
  // pre-initialize it, in case of errors
  wud->self_ref = LUA_NOREF;
  wud->cb_connect_ref = LUA_NOREF;
  wud->cb_reconnect_ref = LUA_NOREF;
  wud->cb_disconnect_ref = LUA_NOREF;
  wud->cb_receive_ref = LUA_NOREF;
  wud->cb_send_ref = LUA_NOREF;
  wud->cb_dns_found_ref = LUA_NOREF;
  wud->pesp_conn = NULL;
#ifdef CLIENT_SSL_ENABLE
  wud->secure = secure;
#endif
  wud->isWebsocket = 0;
  wud->num_urls = 0;
  wud->max_urls = 4;
  wud->urls = (char **)c_zalloc(sizeof(char *) * wud->max_urls);
  checkAllocgLOK(wud->urls);
  wud->urls[0] = NULL;
  wud->curr_url = NULL;

  // set its metatable
  luaL_getmetatable(L, mt);
  lua_setmetatable(L, -2);

  // create the espconn struct
  if(isserver && pTcpServer){
    if(tcpserver_cb_connect_ref != LUA_NOREF) {      // self_ref should be unref in close()
      lua_pop(L,1);
      return luaL_error(L, "only one tcp server allowed");
    }
    pesp_conn = wud->pesp_conn = pTcpServer;
  } else {
    pesp_conn = wud->pesp_conn = (struct espconn *)c_zalloc(sizeof(struct espconn));
    checkAllocgLOK(pesp_conn);

    pesp_conn->proto.tcp = NULL;
    pesp_conn->proto.udp = NULL;
    pesp_conn->reverse = NULL;
    pesp_conn->proto.tcp = (esp_tcp *)c_zalloc(sizeof(esp_tcp));
    if(!pesp_conn->proto.tcp){
      c_free(pesp_conn);
      pesp_conn = wud->pesp_conn = NULL;
      checkErrOK(L, WEBSOCKET_ERR_OUT_OF_MEMORY, "");
    }
    NODE_DBG("TCP server/socket is set.\n");
  }
  pesp_conn->type = type;
  pesp_conn->state = ESPCONN_NONE;
  // reverse is for the callback function
  pesp_conn->reverse = wud;

  if(isserver && pTcpServer==NULL){
    pTcpServer = pesp_conn;
  }

#ifdef NOTDEF
  if ( lua_isstring(L, stack) )
  {
    // we have an url for the path otherwise use "/echo"
    const uint8_t *url = lua_tostring(L, stack);
    stack++;
    wud->url = os_malloc(c_strlen(url)+1); // default to /echo
    checkAllocOK(wud->url);
    c_memcpy(wud->url,url,c_strlen(url));
    wud->url[c_strlen(url)] = '\0';
  } else {
    wud->url = "/echo"; // default to /echo
  }
#endif

  gL = L;   // global L for net module.

  // if call back function is specified, call it with para userdata
  // luaL_checkanyfunction(L, 2);
  if (lua_type(L, stack) == LUA_TFUNCTION || lua_type(L, stack) == LUA_TLIGHTFUNCTION){
    lua_pushvalue(L, stack);  // copy argument (func) to the top of stack
    lua_pushvalue(L, -2);  // copy the self_ref(userdata) to the top
    lua_call(L, 1, 0);
  }

  return 1; 
}

// ============================ websocket_start =======================

// Lua: server:listen( port, ip, function(con) )
// Lua: socket:connect( port, ip, function(con) )
static int websocket_start( lua_State* L, const char *mt )
{

  NODE_DBG("websocket_start is called.\n");
  struct espconn *pesp_conn = NULL;
  lwebsocket_userdata *wud;
  unsigned port;
  size_t il;
  bool isserver = false;
  ip_addr_t ipaddr;
  const char *domain;
  uint8_t stack = 1;
  
  if (mt!=NULL && c_strcmp(mt, "websocket.server")==0)
    isserver = true;
  else if (mt!=NULL && c_strcmp(mt, "websocket.socket")==0)
    isserver = false;
  else
  {
    NODE_DBG("wrong metatable for websocket_start.\n");
    checkErrOK(L, WEBSOCKET_ERR_WRONG_METATABLE, "websocket_start");
    return 0;
  }
  wud = (lwebsocket_userdata *)luaL_checkudata(L, stack, mt);
  luaL_argcheck(L, wud, stack, "Server/Socket expected");
  stack++;

  if(wud==NULL){
    NODE_DBG("userdata is nil.\n");
    checkErrOK(L, WEBSOCKET_ERR_USERDATA_IS_NIL, "websocket_start");
    return 0;
  }

  if(wud->pesp_conn == NULL){
    NODE_DBG("wud->pesp_conn is NULL.\n");
    checkErrOK(L, WEBSOCKET_ERR_PESP_CONN_IS_NIL, "websocket_start");
    return 0;
  }
  pesp_conn = wud->pesp_conn;
  port = luaL_checkinteger( L, stack );
  stack++;
  if(isserver)
    pesp_conn->proto.tcp->local_port = port;
  else{
    pesp_conn->proto.tcp->remote_port = port;
    pesp_conn->proto.tcp->local_port = espconn_port();
  }
  NODE_DBG("TCP port is set to: %d.\n", port);

  if( lua_isstring(L,stack) )   // deal with the domain string
  {
    domain = luaL_checklstring( L, stack, &il );
    stack++;
    if (domain == NULL)
    {
      if(isserver)
        domain = "0.0.0.0";
      else
        domain = "127.0.0.1";
    }
    ipaddr.addr = ipaddr_addr(domain);
    if(isserver)
      c_memcpy(pesp_conn->proto.tcp->local_ip, &ipaddr.addr, 4);
    else
      c_memcpy(pesp_conn->proto.tcp->remote_ip, &ipaddr.addr, 4);
    NODE_DBG("TCP ip is set: ");
    NODE_DBG(IPSTR, IP2STR(&ipaddr.addr));
    NODE_DBG("\n");
  }

  // call back function when a connection is obtained, tcp only
  if (lua_type(L, stack) == LUA_TFUNCTION || lua_type(L, stack) == LUA_TLIGHTFUNCTION){
    lua_pushvalue(L, stack);  // copy argument (func) to the top of stack
    if(isserver)    // for tcp server connected callback
    {
      if(tcpserver_cb_connect_ref != LUA_NOREF)
        luaL_unref(L, LUA_REGISTRYINDEX, tcpserver_cb_connect_ref);
      tcpserver_cb_connect_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    } 
    else 
    {
      if(wud->cb_connect_ref != LUA_NOREF)
        luaL_unref(L, LUA_REGISTRYINDEX, wud->cb_connect_ref);
      wud->cb_connect_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    }
  }

  if(!isserver){    // self_ref is only needed by socket userdata, or udp server
    lua_pushvalue(L, 1);  // copy to the top of stack
    if(wud->self_ref != LUA_NOREF)
      luaL_unref(L, LUA_REGISTRYINDEX, wud->self_ref);
    wud->self_ref = luaL_ref(L, LUA_REGISTRYINDEX);
  }

  if(isserver){   // no secure server support for now
    int res = espconn_regist_connectcb(pesp_conn, websocket_server_connected);
    // tcp server, SSL is not supported
#ifdef CLIENT_SSL_ENABLE
    // if(wud->secure)
    //   espconn_secure_accept(pesp_conn);
    // else
#endif
      res = espconn_accept(pesp_conn);    // if it's a server, no need to dns.
      res =espconn_regist_time(pesp_conn, tcp_server_timeover, 0);
  }
  else{
    int res = espconn_regist_connectcb(pesp_conn, websocket_socket_connected);
    espconn_regist_reconcb(pesp_conn, websocket_socket_reconnected);
#ifdef CLIENT_SSL_ENABLE
    if(wud->secure){
      if(pesp_conn->proto.tcp->remote_port || pesp_conn->proto.tcp->local_port)
        espconn_secure_disconnect(pesp_conn);
      // espconn_secure_connect(pesp_conn);
    }
    else
#endif
    {
      if(pesp_conn->proto.tcp->remote_port || pesp_conn->proto.tcp->local_port)
        espconn_disconnect(pesp_conn);
      // espconn_connect(pesp_conn);
    }
  }

  if(!isserver){
    if((ipaddr.addr == IPADDR_NONE) && (c_memcmp(domain,"255.255.255.255",16) != 0))
    {
      host_ip.addr = 0;
      dns_reconn_count = 0;
      if(ESPCONN_OK == espconn_gethostbyname(pesp_conn, domain, &host_ip, socket_dns_found)){
        socket_dns_found(domain, &host_ip, pesp_conn);  // ip is returned in host_ip.
      }
    }
    else
    {
      socket_connect(pesp_conn);
    }
  }
  return 0;  
}

// ============================ websocket_close =======================

// Lua: server/socket:close()
// server disconnect everything, unref everything
// client disconnect and unref itself
static int websocket_close( lua_State* L, const char *mt )
{
  NODE_DBG("websocket_close is called.\n");
  bool isserver = false;
  int i = 0;
  lwebsocket_userdata *wud = NULL, *skt = NULL;

  wud = (lwebsocket_userdata *)luaL_checkudata(L, 1, mt);
  luaL_argcheck(L, wud, 1, "Server/Socket expected");
  if(wud == NULL) {
    checkErrOK(L, WEBSOCKET_ERR_USERDATA_IS_NIL, "websocket_close");
    return 0;
  }

  if(wud->pesp_conn == NULL) {
    checkErrOK(L, WEBSOCKET_ERR_PESP_CONN_IS_NIL, "websocket_close");
    return 0;
  }

  if (mt!=NULL && c_strcmp(mt, "websocket.server")==0)
    isserver = true;
  else if (mt!=NULL && c_strcmp(mt, "websocket.socket")==0)
    isserver = false;
  else
  {
    NODE_DBG("wrong metatable for websocket_close.\n");
    checkErrOK(L, WEBSOCKET_ERR_WRONG_METATABLE, "websocket_close");
    return 0;
  }

  if(isserver && tcpserver_cb_connect_ref != LUA_NOREF){
    luaL_unref(L, LUA_REGISTRYINDEX, tcpserver_cb_connect_ref);
    tcpserver_cb_connect_ref = LUA_NOREF;
  }

  int n = lua_gettop(L);
  skt = wud;

  do{
    if(isserver && skt == NULL){
      if(socket[i] != LUA_NOREF){  // there is client socket exists
        lua_rawgeti(L, LUA_REGISTRYINDEX, socket[i]);    // get the referenced user_data to stack top
#if 0
        socket[i] = LUA_NOREF;
        socket_num--;
#endif  // do this in net_server_disconnected
        i++;
        if(lua_isuserdata(L,-1)){
          skt = lua_touserdata(L,-1);
        } else {
          lua_pop(L, 1);
          continue;
        }
      }else{
        // skip LUA_NOREF
        i++;
        continue;
      }
    }

    if(skt==NULL){
ets_printf("userdata is nil.\n");
      NODE_DBG("userdata is nil.\n");
      continue;
    }

    if(skt->pesp_conn)    // disconnect the connection
    {
  #ifdef CLIENT_SSL_ENABLE
      if(skt->secure){
         if(skt->pesp_conn->proto.tcp->remote_port || skt->pesp_conn->proto.tcp->local_port)
           espconn_secure_disconnect(skt->pesp_conn);
      }
      else
  #endif
      {
         if(skt->pesp_conn->proto.tcp->remote_port || skt->pesp_conn->proto.tcp->local_port)
          espconn_disconnect(skt->pesp_conn);
      }
    }
#if 0
    // unref the self_ref
    if(LUA_NOREF!=skt->self_ref){    // for a server self_ref is NOREF
      luaL_unref(L, LUA_REGISTRYINDEX, skt->self_ref);
      skt->self_ref = LUA_NOREF;   // for a socket, now only var in lua is ref to the userdata
    }
#endif
    lua_settop(L, n);   // reset the stack top
    skt = NULL;
  } while( isserver && i<MAX_SOCKET);
#if 0
  // unref the self_ref, for both socket and server
  if(LUA_NOREF!=wud->self_ref){    // for a server self_ref is NOREF
    luaL_unref(L, LUA_REGISTRYINDEX, wud->self_ref);
    wud->self_ref = LUA_NOREF;   // now only var in lua is ref to the userdata
  }
#endif

  return 0;  
}

// ============================ websocket_delete =======================

// static int websocket_close( lua_State* L, const char* mt );
// Lua: websocket.delete( socket/server )
// call close() first
// server: disconnect server, unref everything
// socket: unref everything
static int websocket_delete( lua_State* L, const char *mt )
{
  NODE_DBG("websocket_delete is called.\n");
  bool isserver = false;
  if (mt!=NULL && c_strcmp(mt, "websocket.server")==0)
    isserver = true;
  else if (mt!=NULL && c_strcmp(mt, "websocket.socket")==0)
    isserver = false;
  else
  {
    NODE_DBG("wrong metatable for websocket_delete.\n");
    checkErrOK(L, WEBSOCKET_ERR_WRONG_METATABLE, "websocket_delete");
    return 0;
  }

  // websocket_close( L, mt );   // close it first

  lwebsocket_userdata *wud = (lwebsocket_userdata *)luaL_checkudata(L, 1, mt);
  luaL_argcheck(L, wud, 1, "Server/Socket expected");
  if(wud==NULL){
    NODE_DBG("userdata is nil.\n");
    checkErrOK(L, WEBSOCKET_ERR_USERDATA_IS_NIL, "websocket_delete");
    return 0;
  }
  if(wud->pesp_conn){     // for client connected to tcp server, this should set NULL in disconnect cb
    wud->pesp_conn->reverse = NULL;
    if(!isserver)   // socket is freed here
    {
      if(wud->pesp_conn->proto.tcp)
        c_free(wud->pesp_conn->proto.tcp);
      wud->pesp_conn->proto.tcp = NULL;
      c_free(wud->pesp_conn);
    }
    wud->pesp_conn = NULL;    // for socket, it will free this when disconnected
  }

  // free (unref) callback ref
  if(LUA_NOREF!=wud->cb_connect_ref){
    luaL_unref(L, LUA_REGISTRYINDEX, wud->cb_connect_ref);
    wud->cb_connect_ref = LUA_NOREF;
  }
  if(LUA_NOREF!=wud->cb_reconnect_ref){
    luaL_unref(L, LUA_REGISTRYINDEX, wud->cb_reconnect_ref);
    wud->cb_reconnect_ref = LUA_NOREF;
  }
  if(LUA_NOREF!=wud->cb_disconnect_ref){
    luaL_unref(L, LUA_REGISTRYINDEX, wud->cb_disconnect_ref);
    wud->cb_disconnect_ref = LUA_NOREF;
  }
  if(LUA_NOREF!=wud->cb_receive_ref){
    luaL_unref(L, LUA_REGISTRYINDEX, wud->cb_receive_ref);
    wud->cb_receive_ref = LUA_NOREF;
  }
  if(LUA_NOREF!=wud->cb_send_ref){
    luaL_unref(L, LUA_REGISTRYINDEX, wud->cb_send_ref);
    wud->cb_send_ref = LUA_NOREF;
  }
  if(LUA_NOREF!=wud->cb_dns_found_ref){
    luaL_unref(L, LUA_REGISTRYINDEX, wud->cb_dns_found_ref);
    wud->cb_dns_found_ref = LUA_NOREF;
  }
  lua_gc(gL, LUA_GCSTOP, 0);
  if(LUA_NOREF!=wud->self_ref){
    luaL_unref(L, LUA_REGISTRYINDEX, wud->self_ref);
    wud->self_ref = LUA_NOREF;
  }
  lua_gc(gL, LUA_GCRESTART, 0);
  return 0;  
}

// ============================ webserver_set_recv_url =======================

static int webserver_set_recv_url( lua_State* L, const char *mt )
{
  NODE_DBG("webserver_set_recv_url is called.\n");
  bool isserver = false;
  lwebsocket_userdata *wud;
  size_t sl;
  int idx;
  const char *url;
  char *cp;
  
  wud = (lwebsocket_userdata *)luaL_checkudata(L, 1, mt);
  luaL_argcheck(L, wud, 1, "Server/Socket expected");
  if(wud==NULL){
    NODE_DBG("userdata is nil.\n");
    return 0;
  }

  if (mt!=NULL && c_strcmp(mt, "websocket.server")==0)
    isserver = true;
  else if (mt!=NULL && c_strcmp(mt, "websocket.socket")==0)
    isserver = false;
  else
  {
    NODE_DBG("wrong metatable for websocket_on.\n");
    return 0;
  }

  url = luaL_checklstring( L, 2, &sl );
  if (wud->num_urls >= wud->max_urls) {
    wud->max_urls += 4;
    wud->urls = c_realloc(wud->urls, sizeof(char *) * wud->max_urls);
    checkAllocgLOK(wud->urls);
  }
  wud->urls[wud->num_urls] = (char *)c_zalloc(c_strlen(url) + 1);
  cp = wud->urls[wud->num_urls];
  checkAllocgLOK(cp);
  cp[c_strlen(url)] = 0;
  c_memcpy(cp, url, c_strlen(url));
  wud->num_urls++;
  return 0;
}

// ============================ websocket_on =======================

// Lua: socket/udpserver:on( "method", function(s) )
static int websocket_on( lua_State* L, const char *mt )
{
  NODE_DBG("websocket_on is called.\n");
  bool isserver = false;
  lwebsocket_userdata *wud;
  size_t sl;
  
  wud = (lwebsocket_userdata *)luaL_checkudata(L, 1, mt);
  luaL_argcheck(L, wud, 1, "Server/Socket expected");
  if(wud==NULL){
    NODE_DBG("userdata is nil.\n");
    return 0;
  }

  if (mt!=NULL && c_strcmp(mt, "websocket.server")==0)
    isserver = true;
  else if (mt!=NULL && c_strcmp(mt, "websocket.socket")==0)
    isserver = false;
  else
  {
    NODE_DBG("wrong metatable for websocket_on.\n");
    return 0;
  }

  const char *method = luaL_checklstring( L, 2, &sl );
  if (method == NULL)
    return luaL_error( L, "wrong arg type" );

  luaL_checkanyfunction(L, 3);
  lua_pushvalue(L, 3);  // copy argument (func) to the top of stack

  if(!isserver && wud->pesp_conn->type == ESPCONN_TCP && sl == 10 && c_strcmp(method, "connection") == 0){
    if(wud->cb_connect_ref != LUA_NOREF)
      luaL_unref(L, LUA_REGISTRYINDEX, wud->cb_connect_ref);
    wud->cb_connect_ref = luaL_ref(L, LUA_REGISTRYINDEX);
  }else if(!isserver && wud->pesp_conn->type == ESPCONN_TCP && sl == 12 && c_strcmp(method, "reconnection") == 0){
    if(wud->cb_reconnect_ref != LUA_NOREF)
      luaL_unref(L, LUA_REGISTRYINDEX, wud->cb_reconnect_ref);
    wud->cb_reconnect_ref = luaL_ref(L, LUA_REGISTRYINDEX);
  }else if(!isserver && wud->pesp_conn->type == ESPCONN_TCP && sl == 13 && c_strcmp(method, "disconnection") == 0){
    if(wud->cb_disconnect_ref != LUA_NOREF)
      luaL_unref(L, LUA_REGISTRYINDEX, wud->cb_disconnect_ref);
    wud->cb_disconnect_ref = luaL_ref(L, LUA_REGISTRYINDEX);
  }else if((!isserver || wud->pesp_conn->type == ESPCONN_UDP) && sl == 7 && c_strcmp(method, "receive") == 0){
    if(wud->cb_receive_ref != LUA_NOREF)
      luaL_unref(L, LUA_REGISTRYINDEX, wud->cb_receive_ref);
    wud->cb_receive_ref = luaL_ref(L, LUA_REGISTRYINDEX);
  }else if((!isserver || wud->pesp_conn->type == ESPCONN_UDP) && sl == 4 && c_strcmp(method, "sent") == 0){
    if(wud->cb_send_ref != LUA_NOREF)
      luaL_unref(L, LUA_REGISTRYINDEX, wud->cb_send_ref);
    wud->cb_send_ref = luaL_ref(L, LUA_REGISTRYINDEX);
  }else if(!isserver && wud->pesp_conn->type == ESPCONN_TCP && sl == 3 && c_strcmp(method, "dns") == 0){
    if(wud->cb_dns_found_ref != LUA_NOREF)
      luaL_unref(L, LUA_REGISTRYINDEX, wud->cb_dns_found_ref);
    wud->cb_dns_found_ref = luaL_ref(L, LUA_REGISTRYINDEX);
  }else{
  	lua_pop(L, 1);
    return luaL_error( L, "method not supported" );
  }

  return 0;  
}

// ============================ websocket_writeData =======================

static int websocket_writeData( const char *payload, int size, lwebsocket_userdata *wud, int opcode )
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

// ============================ websocket_send =======================

// Lua: server/socket:send( string, function(sent) )
static int websocket_send( lua_State* L, const char *mt )
{
  // NODE_DBG("websocket_send is called.\n");
  bool isserver = false;
  struct espconn *pesp_conn = NULL;
  lwebsocket_userdata *wud;
  size_t l;
  int stack = 1;
  int opcode = 1;
  int result;
  
  wud = (lwebsocket_userdata *)luaL_checkudata(L, stack, mt);
  luaL_argcheck(L, wud, stack, "Server/Socket expected");
  if(wud==NULL){
    NODE_DBG("userdata is nil.\n");
    return 0;
  }

  if(wud->pesp_conn == NULL){
    NODE_DBG("wud->pesp_conn is NULL.\n");
    return 0;
  }
  stack++;
  pesp_conn = wud->pesp_conn;

  if (mt!=NULL && c_strcmp(mt, "websocket.server")==0)
    isserver = true;
  else if (mt!=NULL && c_strcmp(mt, "websocket.socket")==0)
    isserver = false;
  else
  {
    NODE_DBG("wrong metatable for websocket_send.\n");
    return 0;
  }

  if(isserver && wud->pesp_conn->type == ESPCONN_TCP){
    return luaL_error( L, "tcp server send not supported" );
  }

#if 0
  char temp[20] = {0};
  c_sprintf(temp, IPSTR, IP2STR( &(pesp_conn->proto.tcp->remote_ip) ) );
  NODE_DBG("remote ");
  NODE_DBG(temp);
  NODE_DBG(":");
  NODE_DBG("%d",pesp_conn->proto.tcp->remote_port);
  NODE_DBG(" sending data.\n");
#endif

  const char *payload = luaL_checklstring( L, stack, &l );
  if (l>1460 || payload == NULL)
    return luaL_error( L, "need <1460 payload" );

  stack++;
  if (lua_isnumber( L, stack )) {
    opcode = lua_tointeger(L, stack);
    if ((opcode > 0x03 && opcode < 0x08) || opcode > 0x0B) {
      return luaL_error( L, "bad opcode" );
    }
    stack++;
  }
  
  if (lua_type(L, stack) == LUA_TFUNCTION || lua_type(L, stack) == LUA_TLIGHTFUNCTION){
    lua_pushvalue(L, stack);  // copy argument (func) to the top of stack
    if(wud->cb_send_ref != LUA_NOREF)
      luaL_unref(L, LUA_REGISTRYINDEX, wud->cb_send_ref);
    wud->cb_send_ref = luaL_ref(L, LUA_REGISTRYINDEX);
  }
#ifdef CLIENT_SSL_ENABLE
  if(wud->secure) {
    result = espconn_secure_sent(pesp_conn, (unsigned char *)payload, l);
    checkErrOK(L, result, "espconn_sent");
  } else {
#endif
    if (wud->isWebsocket == 1) {
      result = websocket_writeData(payload, l, wud, opcode);
      checkErrOK(L, result, "websocket_write");
    } else {
      result = espconn_sent(pesp_conn, (unsigned char *)payload, l);
      checkErrOK(L, result, "espconn_sent");
    }
#ifdef CLIENT_SSL_ENABLE
  }
#endif
  if (isserver && (strstr(payload, HEADER_WEBSOCKETLINE) != 0)) {
    wud->isWebsocket = 1;
  }

  return 0;  
}

// ============================ websocket_createServer =======================

// Lua: s = websocket.createServer(function(server))
static int websocket_createServer( lua_State* L )
{
  const char *mt = "websocket.server";
  return websocket_create(L, mt);
}

// ============================ websocket_server_delete =======================

// Lua: server:delete()
static int websocket_server_delete( lua_State* L )
{
  const char *mt = "websocket.server";
  return websocket_delete(L, mt);
}

// ============================ websocket_server_listen =======================

// Lua: server:listen( port, ip )
static int websocket_server_listen( lua_State* L )
{
  const char *mt = "websocket.server";
  return websocket_start(L, mt);
}

// ============================ websocket_server_close =======================

// Lua: server:close()
static int websocket_server_close( lua_State* L )
{
  const char *mt = "websocket.server";
  return websocket_close(L, mt);
}


// ============================ websocket_server_set_recv_url =======================

// Lua: server:seturl( url )
static int websocket_server_set_recv_url( lua_State* L )
{
  const char *mt = "websocket.server";
  return webserver_set_recv_url(L, mt);
}

// ============================ websocket_createConnection =======================

// Lua: s = websocket.createConnection(function(conn))
static int websocket_createConnection( lua_State* L )
{
  const char *mt = "websocket.socket";
  return websocket_create(L, mt);
}

// ============================ websocket_socket_connect =======================

// Lua: socket:connect( port, ip )
static int websocket_socket_connect( lua_State* L )
{
  const char *mt = "websocket.socket";
  return websocket_start(L, mt);
}

// ============================ websocket_socket_close =======================

// Lua: socket:close()
static int websocket_socket_close( lua_State* L )
{
  const char *mt = "websocket.socket";
  return websocket_close(L, mt);
}

// ============================ websocket_socket_on =======================

// Lua: socket:on( "method", function(socket) )
static int websocket_socket_on( lua_State* L )
{
  const char *mt = "websocket.socket";
  return websocket_on(L, mt);
}

// ============================ websocket_socket_send =======================

// Lua: socket:send( string, function() )
static int websocket_socket_send( lua_State* L )
{
  const char *mt = "websocket.socket";
  return websocket_send(L, mt);
}

// ============================ websocket_socket_delete =======================

// Lua: socket:delete()
static int websocket_socket_delete( lua_State* L )
{
  const char *mt = "websocket.socket";
  return websocket_delete(L, mt);
}

// Module function map
static const LUA_REG_TYPE websocket_socket_map[] =  {
  { LSTRKEY( "connect" ),  LFUNCVAL( websocket_socket_connect ) },
  { LSTRKEY( "close" ),    LFUNCVAL( websocket_socket_close ) },
  { LSTRKEY( "on" ),       LFUNCVAL( websocket_socket_on ) },
  { LSTRKEY( "send" ),     LFUNCVAL( websocket_socket_send ) },
//  { LSTRKEY( "delete" ), LFUNCVAL( websocket_socket_delete ) },
  { LSTRKEY( "__gc" ),     LFUNCVAL( websocket_socket_delete ) },
  { LSTRKEY( "__index" ),  LROVAL( websocket_socket_map ) },
  { LNILKEY, LNILVAL }
};

static const LUA_REG_TYPE websocket_server_map[] =  {
  { LSTRKEY( "listen" ),   LFUNCVAL( websocket_server_listen ) },
  { LSTRKEY( "close" ),    LFUNCVAL( websocket_server_close ) },
  { LSTRKEY( "seturl" ),   LFUNCVAL( websocket_server_set_recv_url ) },
//  { LSTRKEY( "delete" ), LFUNCVAL( websocket_server_delete ) },
  { LSTRKEY( "__gc" ),     LFUNCVAL( websocket_server_delete ) },
  { LSTRKEY( "__index" ),  LROVAL( websocket_server_map ) },
  { LNILKEY, LNILVAL }
};

static const LUA_REG_TYPE websocket_map[] =  {
  { LSTRKEY( "createServer" ),     LFUNCVAL( websocket_createServer ) },
  { LSTRKEY( "createConnection" ), LFUNCVAL( websocket_createConnection ) },
  { LSTRKEY( "__metatable" ),      LROVAL( websocket_map ) },
  { LNILKEY, LNILVAL }
};

int luaopen_websocket( lua_State *L ) {
  int i;
  for(i=0;i<MAX_SOCKET;i++)
  {
    socket[i] = LUA_NOREF;
  }

  luaL_rometatable(L, "websocket.server", (void *)websocket_server_map);  // create metatable for websocket.server
  luaL_rometatable(L, "websocket.socket", (void *)websocket_socket_map);  // create metatable for websocket.client

  return 0;
}

NODEMCU_MODULE(WEBSOCKET, "websocket", websocket_map, luaopen_websocket);
