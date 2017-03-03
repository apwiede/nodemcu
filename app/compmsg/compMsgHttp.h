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
 * File:   compMsgHttp.h
 * Author: Arnulf P. Wiedemann
 *
 * Created on Descmber 16, 2016
 */

#ifndef COMP_MSG_HTTP_H
#define	COMP_MSG_HTTP_H

#include "c_types.h"
#ifdef	__cplusplus
extern "C" {
#endif

enum compMsgHttpHeaderNames
{
  COMP_MSG_HTTP_CODE                              = 1,
  COMP_MSG_HTTP_CONTENT_TYPE                      = 2,
  COMP_MSG_HTTP_CONTENT_LENGTH                    = 3,
  COMP_MSG_HTTP_CONNECTION                        = 4,
  COMP_MSG_HTTP_SERVER                            = 5,
  COMP_MSG_HTTP_DATE                              = 6,
  COMP_MSG_HTTP_CACHE_CONTROL                     = 7,
  COMP_MSG_HTTP_NODE_CODE                         = 8,
  COMP_MSG_HTTP_SET_COOKIE                        = 9,
  COMP_MSG_HTTP_X_POWER_BY                        = 10,
  COMP_MSG_HTTP_CONTENT_ENCODING                  = 11,
  COMP_MSG_HTTP_SEC_WEBSOCKET_KEY                 = 12,
  COMP_MSG_HTTP_UPGRADE                           = 13, 
  COMP_MSG_HTTP_ACCESS_CONTROL_ALLOW_ORIGIN       = 14,
  COMP_MSG_HTTP_ACCESS_CONTROL_ALLOW_CREDENTIALS  = 15,
  COMP_MSG_HTTP_ACCESS_CONTROL_ALLOW_HEADERS      = 16,
  COMP_MSG_HTTP_SEC_WEBSOCKET_ACCEPT              = 17,
  COMP_MSG_HTTP_HOST                              = 18,
  COMP_MSG_HTTP_USER_AGENT                        = 19,
  COMP_MSG_HTTP_SEC_WEBSOCKET_VERSION             = 20,
  COMP_MSG_HTTP_SEC_WEBSOCKET_PROTOCOL            = 21,
  COMP_MSG_HTTP_ACCEPT                            = 22,
  COMP_MSG_HTTP_ACCEPT_ENCODING                   = 23,
  COMP_MSG_HTTP_ACCEPT_LANGUAGE                   = 24,
  COMP_MSG_HTTP_X_ASPNET_VERSION                  = 25,
  COMP_MSG_HTTP_SEC_WEBSOCKET_EXTENSIONS          = 26,
  COMP_MSG_HTTP_PRAGMA                            = 27,
  COMP_MSG_HTTP_ORIGIN                            = 28,
  COMP_MSG_HTTP_UPGRADE_INSECURE_REQUESTS         = 29,
};

typedef struct compMsgDispatcher compMsgDispatcher_t;

typedef uint8_t (* getHttpHeaderKeyIdFromKey_t)(compMsgDispatcher_t *self, const uint8_t *httpHeaderStr, uint8_t *httpHeaderKeyId);
typedef uint8_t (* getHttpHeaderKeyIdFromLowerKey_t)(compMsgDispatcher_t *self, const uint8_t *httpHeaderStr, uint8_t *httpHeaderKeyId);
typedef uint8_t (* getHttpHeaderKeyFromId_t)(compMsgDispatcher_t *self, uint8_t httpHeaderKeyId, const uint8_t **key);
typedef uint8_t (* httpParse_t)(socketUserData_t *sud, char * data, size_t size);
typedef uint8_t ( *getHttpGetHeaderValueForId_t)(socketUserData_t *sud, uint8_t id, const uint8_t **value);

typedef struct compMsgHttp {

  getHttpHeaderKeyIdFromKey_t getHttpHeaderKeyIdFromKey;
  getHttpHeaderKeyIdFromLowerKey_t getHttpHeaderKeyIdFromLowerKey;
  getHttpHeaderKeyFromId_t getHttpHeaderKeyFromId;
  httpParse_t httpParse;
  getHttpGetHeaderValueForId_t getHttpGetHeaderValueForId;
} compMsgHttp_t;

compMsgHttp_t *newCompMsgHttp();
uint8_t compMsgHttpInit(compMsgDispatcher_t *self);

#ifdef  __cplusplus
}
#endif

#endif  /* COMP_MSG_HTTP_H */

