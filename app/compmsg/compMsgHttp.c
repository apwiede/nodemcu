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
 * File:   compMsgHttp.c
 * Author: Arnulf P. Wiedemann
 *
 * Created on December 16th, 2016
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

#define _tolower(__c) ((unsigned char)(__c) - 'A' + 'a')
#define _toupper(__c) ((unsigned char)(__c) - 'a' + 'A')

typedef struct httpHeaderKeyInfo {
  uint8_t *lowerKey;
  uint8_t *key;
  uint8_t id;
} httpHeaderKeyInfo_t;

static httpHeaderKeyInfo_t httpHeaderKeyInfos[] = {
  {"content-type",                     "Content-Type",                     COMP_MSG_HTTP_CONTENT_TYPE},
  {"content-length",                   "Content-Length",                   COMP_MSG_HTTP_CONTENT_LENGTH},
  {"connection",                       "Connection",                       COMP_MSG_HTTP_CONNECTION},
  {"server",                           "Server",                           COMP_MSG_HTTP_SERVER},
  {"date",                             "Date",                             COMP_MSG_HTTP_DATE},
  {"cache-control",                    "Cache-Control",                    COMP_MSG_HTTP_CACHE_CONTROL},
  {"node-code",                        "Node-Code",                        COMP_MSG_HTTP_NODE_CODE},
  {"set-cookie",                       "Set-Cookie",                       COMP_MSG_HTTP_SET_COOKIE},
  {"x-powered-by",                     "X-Powered-By",                     COMP_MSG_HTTP_X_POWER_BY},
  {"content-encoding",                 "Content-Encoding",                 COMP_MSG_HTTP_CONTENT_ENCODING},
  {"sec-websocket-key",                "Sec-WebSocket-Key",                COMP_MSG_HTTP_SEC_WEBSOCKET_KEY},
  {"upgrade",                          "Upgrade",                          COMP_MSG_HTTP_UPGRADE},
  {"access-control-allow-origin",      "Access-Control-Allow-Origin",      COMP_MSG_HTTP_ACCESS_CONTROL_ALLOW_ORIGIN},
  {"access-control-allow-credentials", "Access-Control-Allow-Credentials", COMP_MSG_HTTP_ACCESS_CONTROL_ALLOW_CREDENTIALS},
  {"access-control-allow-headers",     "Access-Control-Allow-Headers",     COMP_MSG_HTTP_ACCESS_CONTROL_ALLOW_HEADERS},
  {"sec-websocket-accept",             "Sec-WebSocket-Accept",             COMP_MSG_HTTP_SEC_WEBSOCKET_ACCEPT},
  {"host",                             "Host",                             COMP_MSG_HTTP_HOST},
  {"user-agent",                       "User-Agent",                       COMP_MSG_HTTP_USER_AGENT},
  {"sec-websocket-version",            "Sec-WebSocket-Version",            COMP_MSG_HTTP_SEC_WEBSOCKET_VERSION},
  {"sec-websocket-protocol",           "Sec-WebSocket-Protocol",           COMP_MSG_HTTP_SEC_WEBSOCKET_PROTOCOL},
  {"accept",                           "Accept",                           COMP_MSG_HTTP_ACCEPT},
  {"accept-encoding",                  "Accept-Encoding",                  COMP_MSG_HTTP_ACCEPT_ENCODING},
  {"accept-language",                  "Accept-Language",                  COMP_MSG_HTTP_ACCEPT_LANGUAGE},
  {"host",                             "Host",                             COMP_MSG_HTTP_HOST},
  {"x-aspnet-version",                 "X-Aspnet-Version",                 COMP_MSG_HTTP_X_ASPNET_VERSION},
  {"sec-websocket-extensions",         "Sec-WebSocket-Extensions",         COMP_MSG_HTTP_SEC_WEBSOCKET_EXTENSIONS},
  {"pragma",                           "Pragma",                           COMP_MSG_HTTP_PRAGMA},
  {"origin",                           "Origin",                           COMP_MSG_HTTP_ORIGIN},
  {"upgrade-insecure-requests",        "Upgrade-Insecure-Requests",        COMP_MSG_HTTP_UPGRADE_INSECURE_REQUESTS},

  {NULL, NULL, -1},
};

// ================================= getHttpHeaderKeyIdFromKey ====================================

static uint8_t getHttpHeaderKeyIdFromKey(compMsgDispatcher_t *self, const uint8_t *httpHeaderKey, uint8_t *httpHeaderKeyId) {
  httpHeaderKeyInfo_t *entry;

  entry = &httpHeaderKeyInfos[0];
  while (entry->key != NULL) {
    if (c_strcmp(httpHeaderKey, entry->key) == 0) {
      *httpHeaderKeyId = entry->id;
      return DATA_VIEW_ERR_OK;
    }
    entry++;
  }
  return DATA_VIEW_ERR_FIELD_TYPE_NOT_FOUND;
}

// ================================= getHttpHeaderKeyIdFromLowerKey ====================================

static uint8_t getHttpHeaderKeyIdFromLowerKey(compMsgDispatcher_t *self, const uint8_t *httpHeaderKey, uint8_t *httpHeaderKeyId) {
  httpHeaderKeyInfo_t *entry;

  entry = &httpHeaderKeyInfos[0];
  while (entry->key != NULL) {
    if (c_strcmp(httpHeaderKey, entry->lowerKey) == 0) {
      *httpHeaderKeyId = entry->id;
      return DATA_VIEW_ERR_OK;
    }
    entry++;
  }
COMP_MSG_DBG(self, "Y", 1, "getHttpHeaderKeyIdFromLowerKey: %s not found", httpHeaderKey);
  return COMP_MSG_ERR_HEADER_NOT_FOUND;
}

// ================================= getHttpHeaderKeyFromId ====================================

static uint8_t getHttpHeaderKeyFromId(compMsgDispatcher_t *self, uint8_t httpHeaderKeyId, const uint8_t **key) {
  httpHeaderKeyInfo_t *entry;

  entry = &httpHeaderKeyInfos[0];
  while (entry->key != NULL) {
    if (httpHeaderKeyId == entry->id) {
      *key = entry->key;
      return DATA_VIEW_ERR_OK;
    }
    entry++;
  }
  return DATA_VIEW_ERR_FIELD_TYPE_NOT_FOUND;
}

// ============================ getContentAndNumHeaders =========================================

static uint8_t getContentAndNumHeaders(compMsgDispatcher_t *self, char *data, size_t size, socketUserData_t *sud) {
  uint8_t result;
  httpMsgInfo_t *httpMsgInfo;
  size_t idx;
  char *cp;
  char *lastNewLine;

  httpMsgInfo = &sud->httpMsgInfos[sud->numHttpMsgInfos];
  httpMsgInfo->numHeaders = 0;
  idx = 0;
  cp = data;
  lastNewLine = cp;
  // get the number of lines of the request
ets_printf("getContentAndNumHeaders 2: size: %d\n", size);
  while (idx < size) {
    if ((*cp == '\n')) {
      // check for end of http header
      if ((cp - lastNewLine) < 3) {
         COMP_MSG_DBG(self, "H", 2, "cp-data: %d\n", cp - data);
         *cp = '\0';
         httpMsgInfo->content = cp + 1;
         httpMsgInfo->currLgth = size - (cp + 1 -data);
         break;
      }
      lastNewLine = cp;
      httpMsgInfo->numHeaders++;
    }
    idx++;
    cp++;
  }
ets_printf("currLgth: %d\n", httpMsgInfo->currLgth);
  return COMP_MSG_ERR_OK;
}

// ============================ getHttpRequestCode =========================================

static uint8_t getHttpRequestCode(compMsgDispatcher_t *self, char * data, socketUserData_t *sud) {
  uint8_t result;
  httpMsgInfo_t *httpMsgInfo;
  size_t idx;
  char *cp;
  size_t httpHeaderIdx;
  httpHeaderPart_t *httpHeaderPart;
  char *endPtr;
  long uval;

  httpMsgInfo = &sud->httpMsgInfos[sud->numHttpMsgInfos];
  idx = 0;
  httpHeaderIdx = 0;
  COMP_MSG_DBG(self, "H", 2, "get RequestCode: httpMsgInfo->receivedHeaders: %p\n", httpMsgInfo->receivedHeaders);
  httpHeaderPart = &httpMsgInfo->receivedHeaders[httpHeaderIdx];
  COMP_MSG_DBG(self, "H", 2, "httpHeaderPart: %p\n", httpHeaderPart);
  httpHeaderPart->httpHeaderId = COMP_MSG_HTTP_CODE;
  httpHeaderPart->httpHeaderName = "Code";
  httpHeaderPart->httpHeaderValue = data ;
  // get result code
  cp = data;
  while (*cp != '\n') {
    if (*cp == ' ') {
      uval = c_strtoul(cp+1, &endPtr, 10);
      COMP_MSG_DBG(self, "H", 2, "uval: %d\n", uval);
      httpMsgInfo->httpRequestCode = (int)uval;
      COMP_MSG_DBG(self, "H", 1, "CODE: %d!\n", httpMsgInfo->httpRequestCode);
      break;
    }
    cp++;
  }
  return COMP_MSG_ERR_OK;
}

// ============================ getHttpHeaders =========================================

static uint8_t getHttpHeaders(compMsgDispatcher_t *self, char * data, size_t size, socketUserData_t *sud) {
  uint8_t result;
  char *cp;
  bool hadColon;
  size_t idx;
  size_t headerLgth;
  size_t httpHeaderIdx;
  httpHeaderPart_t *httpHeaderPart;
  httpMsgInfo_t *httpMsgInfo;
  uint8_t *headerStr;
  uint8_t *lowerHeaderStr;
  char *lowerData;
  char *dp;

  idx = 0;
  httpMsgInfo = &sud->httpMsgInfos[sud->numHttpMsgInfos];
  httpHeaderIdx = 0;
  cp = data;
  hadColon = false;
  lowerData = os_zalloc(size + 1);
  dp = lowerData;
  lowerHeaderStr = dp;
  headerLgth = (char *)httpMsgInfo->content - data;
  while (idx < headerLgth) {
    if ((*cp == '\n')) {
      if (httpHeaderIdx > 0) {
        COMP_MSG_DBG(self, "H", 2, "dp: %d cp: %d", dp-(char *)lowerHeaderStr, cp-(char *)headerStr);
        COMP_MSG_DBG(self, "H", 2, "id: %d name: %s value: %s!", httpHeaderPart->httpHeaderId, httpHeaderPart->httpHeaderName, httpHeaderPart->httpHeaderValue);
      }
      cp[-1] = '\0'; // ignore \r
      cp++;
      idx++;
      httpHeaderIdx++;
      hadColon = false;
      httpHeaderPart = &httpMsgInfo->receivedHeaders[httpHeaderIdx];
      dp = lowerData + (cp - data);
      lowerHeaderStr = dp;
      headerStr = cp;
      COMP_MSG_DBG(self, "H", 2, "lh: %s!hs: %s!", lowerHeaderStr, headerStr);
    }
    if (httpHeaderIdx > 0) {
      if (!hadColon && (*cp == ':')) {
        hadColon = true;
        *cp = '\0';
        *dp = '\0';
        result = sud->compMsgDispatcher->compMsgHttp->getHttpHeaderKeyIdFromLowerKey(sud->compMsgDispatcher, lowerHeaderStr, &httpHeaderPart->httpHeaderId);
        checkErrOK(result);
        httpHeaderPart->httpHeaderName = headerStr;
        while (cp[1] == ' ') {
          cp++;
          idx++;
        }
        httpHeaderPart->httpHeaderValue = cp + 1;
      } else {
        if ((*cp >= 'A') && (*cp <= 'Z')) {
          *dp = _tolower(*cp);
        } else {
          *dp = *cp;
        }
      }
    } else {
      *dp = *cp;
    }
    dp++;
    cp++;
    idx++;
  }
  return COMP_MSG_ERR_OK;
}

// ============================ httpParse =========================================

static uint8_t ICACHE_FLASH_ATTR httpParse(socketUserData_t *sud, char *data, size_t size) {
  uint8_t result;
  int idx;
  size_t httpHeaderIdx;
  httpHeaderPart_t *httpHeaderPart;
  httpMsgInfo_t *httpMsgInfo;
  char *endPtr;
  long uval;
  compMsgDispatcher_t *self;
  size_t msgLgth;
  uint8_t *msgData;

  self = sud->compMsgDispatcher;
  COMP_MSG_DBG(self, "H", 2, "httpParse");
  COMP_MSG_DBG(self, "H", 2, "SUD: %p", sud);

  httpMsgInfo = &sud->httpMsgInfos[sud->numHttpMsgInfos];
  httpMsgInfo->data = os_zalloc(size + 1);
  checkAllocOK(httpMsgInfo->data);
  c_memcpy(httpMsgInfo->data, data, size);
  result = getContentAndNumHeaders(self, httpMsgInfo->data, size, sud);
  checkErrOK(result);

  COMP_MSG_DBG(self, "H", 2, "httpParse: numHeaders: %d sud->content: %s!len: %d!", httpMsgInfo->numHeaders, httpMsgInfo->content, httpMsgInfo->currLgth);  
  COMP_MSG_DBG(self, "H", 2, "receivedHeadersSize: %d", httpMsgInfo->numHeaders * sizeof(httpHeaderPart_t));
  httpMsgInfo->receivedHeaders = os_zalloc(httpMsgInfo->numHeaders * sizeof(httpHeaderPart_t));
  checkAllocOK(httpMsgInfo->receivedHeaders);
  COMP_MSG_DBG(self, "H", 2, "httpMsgInfo->receivedHeaders: %p", httpMsgInfo->receivedHeaders);

  result = getHttpRequestCode(self, httpMsgInfo->data, sud);
  checkErrOK(result);

  result = getHttpHeaders(self, httpMsgInfo->data, size, sud);
  checkErrOK(result);

  idx = 0;
  while (idx < httpMsgInfo->numHeaders) {
    httpHeaderPart = &httpMsgInfo->receivedHeaders[idx];
    COMP_MSG_DBG(self, "H", 1, "idx: %d id: %d name: %s value: %s!\n", idx, httpHeaderPart->httpHeaderId, httpHeaderPart->httpHeaderName, httpHeaderPart->httpHeaderValue);
    if (httpHeaderPart->httpHeaderId == COMP_MSG_HTTP_CONTENT_LENGTH) {
      if (httpMsgInfo->expectedLgth == 0) {
        uval = c_strtoul(httpHeaderPart->httpHeaderValue, &endPtr, 10);
        COMP_MSG_DBG(self, "H", 2, "expectedLgth: %d", uval);
        httpMsgInfo->expectedLgth = (size_t)uval;
      }
    }
    idx++;
  }
  COMP_MSG_DBG(self, "Y", 2, "content: %s lgth: %d", httpMsgInfo->content, httpMsgInfo->currLgth);
  COMP_MSG_DBG(self, "Y", 2, "Code:: %d", httpMsgInfo->httpRequestCode);
// FIXME need to call addRequest here !!!
  switch (httpMsgInfo->httpRequestCode) {
  case 200:
    COMP_MSG_DBG(self, "H", 1, "httpRequestCode: 200 OK");
    //FIXME handle message here need code!!
    msgLgth = httpMsgInfo->currLgth;
    switch (sud->connectionType) {
    case NET_SOCKET_TYPE_SOCKET:
      result = self->compMsgUtil->fromBase64(self, httpMsgInfo->content, &msgLgth, &msgData);
      checkErrOK(result);
      self->compMsgData->currHdr->hdrHandleType = 'W';
      COMP_MSG_DBG(self, "H", 1, "currHdr: %c", self->compMsgData->currHdr->hdrHandleType);
      self->compMsgData->sud = sud;
      self->compMsgData->direction = COMP_MSG_RECEIVED_DATA;
      self->compMsgData->receivedData = msgData;
      self->compMsgData->receivedLgth = msgLgth;
      result = self->compMsgRequest->addRequest(self, COMP_DISP_INPUT_NET_SOCKET, sud, self->compMsgData);
//      result = self->compMsgSendReceive->sendMsg(self, msgData, msgLgth);
      COMP_MSG_DBG(self, "H", 1, "sendMsg: result: %d", result);
      checkErrOK(result);
      break;
    }
    break;
  case 0:
    COMP_MSG_DBG(self, "H", 1, "httpRequestCode: 0 OK");
    msgLgth = httpMsgInfo->currLgth;
    switch (sud->connectionType) {
    case NET_SOCKET_TYPE_CLIENT:
      self->compMsgData->currHdr->hdrHandleType = 'W';
      msgData = httpMsgInfo->content;
      COMP_MSG_DBG(self, "H", 1, "currHdr: %c", self->compMsgData->currHdr->hdrHandleType);
      self->compMsgData->sud = sud;
      self->compMsgData->direction = COMP_MSG_TRANSFER_DATA;
      self->compMsgData->receivedData = msgData;
      self->compMsgData->receivedLgth = msgLgth;
      result = self->compMsgRequest->addRequest(self, COMP_DISP_INPUT_NET_SOCKET, sud, self->compMsgData);
      COMP_MSG_DBG(self, "H", 1, "sendMsg: result: %d", result);
      checkErrOK(result);
      break;
    }
    break;
  }
  httpMsgInfo->expectedLgth = 0;
  return COMP_MSG_ERR_OK;
}

// ============================ getHttpGetHeaderValueForId =========================================

static uint8_t getHttpGetHeaderValueForId(socketUserData_t *sud, uint8_t id, const uint8_t **value) {
  uint8_t result;
  int idx;
  size_t httpHeaderIdx;
  httpHeaderPart_t *httpHeaderPart;
  httpMsgInfo_t *httpMsgInfo;

  httpMsgInfo = &sud->httpMsgInfos[sud->numHttpMsgInfos];
  idx = 0;
  while (idx < httpMsgInfo->numHeaders) {
    httpHeaderPart = &httpMsgInfo->receivedHeaders[idx];
    if (httpHeaderPart->httpHeaderId == id) {
      *value = httpHeaderPart->httpHeaderValue;
      return COMP_MSG_ERR_OK;
    }
    idx++;
  }
  return COMP_MSG_ERR_FIELD_NOT_FOUND;
}

// ================================= compMsgHttpInit ====================================

uint8_t compMsgHttpInit(compMsgDispatcher_t *self) {
  uint8_t result;

  self->compMsgHttp->getHttpHeaderKeyIdFromKey = &getHttpHeaderKeyIdFromKey;
  self->compMsgHttp->getHttpHeaderKeyIdFromLowerKey = &getHttpHeaderKeyIdFromLowerKey;
  self->compMsgHttp->getHttpHeaderKeyFromId = &getHttpHeaderKeyFromId;
  self->compMsgHttp->httpParse = &httpParse;
  self->compMsgHttp->getHttpGetHeaderValueForId = &getHttpGetHeaderValueForId;
  return COMP_MSG_ERR_OK;
}

// ================================= newCompMsgHttp ====================================

compMsgHttp_t *newCompMsgHttp() {
  compMsgHttp_t *compMsgHttp = os_zalloc(sizeof(compMsgHttp_t));
  if (compMsgHttp == NULL) {
    return NULL;
  }
  return compMsgHttp;
}
