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
 * File:   compMsgUtil.h
 * Author: Arnulf P. Wiedemann
 *
 * Created on December 18, 2016
 */

#ifndef COMP_MSG_UTIL_H
#define	COMP_MSG_UTIL_H

#include "c_types.h"
#include "compMsgDataView.h"

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct compMsgDispatcher compMsgDispatcher_t;

typedef struct fieldValueCallbackInfos {
  uint8_t *callbackName;
  fieldValueCallback_t callback;
  uint8_t callbackType;
} fieldValueCallbackInfos_t;

typedef uint8_t (* toBase64_t)(compMsgDispatcher_t *self, const uint8_t *msg, size_t *len, uint8_t **encoded);
typedef uint8_t (* fromBase64_t)(compMsgDispatcher_t *self, const uint8_t *encodedMsg, size_t *len, uint8_t **decodedMsg);
typedef uint8_t (* encryptMsg_t)(compMsgDispatcher_t *self, const uint8_t *msg, size_t mlen, const uint8_t *key, size_t klen, const uint8_t *iv, size_t ivlen, uint8_t **buf, int *lgth);
typedef uint8_t (* decryptMsg_t)(compMsgDispatcher_t *self, const uint8_t *msg, size_t mlen, const uint8_t *key, size_t klen, const uint8_t *iv, size_t ivlen, uint8_t **buf, int *lgth);
typedef uint8_t (* setFieldValueCallback_t)(compMsgDispatcher_t *self, uint8_t *callbackName, fieldValueCallback_t callback, uint8_t callbackType);
typedef uint8_t (* addFieldValueCallbackName_t)(compMsgDispatcher_t *self, uint8_t *callbackName, fieldValueCallback_t callback, uint8_t callbackType);
typedef uint8_t (* getFieldValueCallback_t)(compMsgDispatcher_t *self, uint8_t *callbackName, fieldValueCallback_t *callback, uint8_t callbackType);
typedef uint8_t (* getFieldValueCallbackName_t)(compMsgDispatcher_t *self, fieldValueCallback_t callback, uint8_t **callbackName, uint8_t callbackType);

typedef struct compMsgUtil {
  uint8_t numFieldValueCallbackInfos;
  uint8_t maxFieldValueCallbackInfos;
  fieldValueCallbackInfos_t *fieldValueCallbackInfos;

  encryptMsg_t encryptMsg;
  decryptMsg_t decryptMsg;
  toBase64_t toBase64;
  fromBase64_t fromBase64;
  setFieldValueCallback_t setFieldValueCallback;
  addFieldValueCallbackName_t addFieldValueCallbackName;
  getFieldValueCallback_t getFieldValueCallback;
  getFieldValueCallbackName_t getFieldValueCallbackName;
} compMsgUtil_t;

compMsgUtil_t *newCompMsgUtil();
uint8_t compMsgUtilInit(compMsgDispatcher_t *self);

#ifdef  __cplusplus
}
#endif

#endif  /* COMP_MSG_UTIL_H */

