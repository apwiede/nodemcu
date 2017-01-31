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
 * File:   compMsgUtil.c
 * Author: Arnulf P. Wiedemann
 *
 * Created on December 16th, 2016
 */

#include "osapi.h"
#include "c_types.h"
#include "mem.h"
#include "flash_fs.h"

#include "c_limits.h"
#include "c_string.h"
#include "c_stdio.h"
#include "c_stdlib.h"
#include "../crypto/mech.h"
#include "compMsgDispatcher.h"

#define BASE64_INVALID '\xff'
#define BASE64_PADDING '='
#define ISBASE64(c) (unbytes64[c] != BASE64_INVALID)

static const uint8 b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// ============================= toBase64 ========================

/**
 * \brief encode message with base64
 * \param msg The message
 * \param len The length of the message
 * \param encoded The out param encoded message
 * \return Error code or ErrorOK
 *
 */
static uint8_t toBase64(compMsgDispatcher_t *self, const uint8_t *msg, size_t *len, uint8_t **encoded) {
  size_t i;
  size_t n;
  uint8_t *q;
  uint8_t *out;
  uint8_t bytes64[sizeof(b64)];

  n = *len;
  if (!n) { // handle empty string case 
    return COMP_MSG_ERR_OUT_OF_MEMORY;
  }
  out = (uint8_t *)os_zalloc(((n + 2) / 3 * 4) + 1);
  checkAllocOK(out);
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
  COMP_MSG_DBG(self, "U", 2, "b64Len: %d *len: %d", ((n + 2) / 3 * 4) + 1, *len);
  // ATTENTION the caller has to free *encoded!!
  *encoded = out;
  return COMP_MSG_ERR_OK;
}

// ============================= fromBase64 ========================

static uint8_t fromBase64(compMsgDispatcher_t *self, const uint8_t *encodedMsg, size_t *len, uint8_t **decodedMsg) {
  int i;
  int n;
  int blocks;
  int pad;
  const uint8 *p;
  uint8_t unbytes64[UCHAR_MAX+1];
  uint8_t *msg;
  uint8_t *q;

  n = *len;
  blocks = (n>>2);
  pad = 0;
  if (!n) { // handle empty string case 
    return COMP_MSG_ERR_OUT_OF_MEMORY;
  } 
  if (n & 3) {
    return COMP_MSG_ERR_INVALID_BASE64_STRING;
  } 
  c_memset(unbytes64, BASE64_INVALID, sizeof(unbytes64));
  for (i = 0; i < sizeof(b64)-1; i++) {
    unbytes64[b64[i]] = i;  // sequential so no exceptions 
  }
  if (encodedMsg[n-1] == BASE64_PADDING) {
    pad =  (encodedMsg[n-2] != BASE64_PADDING) ? 1 : 2;
    blocks--;  //exclude padding block
  }    

  for (i = 0; i < n - pad; i++) {
    if (!ISBASE64(encodedMsg[i])) {
      return COMP_MSG_ERR_INVALID_BASE64_STRING;
    }
  }
  unbytes64[BASE64_PADDING] = 0;
  q = (uint8_t *) os_zalloc(1+ (3 * n / 4)); 
  checkAllocOK(q);
  msg = q;
  for (i = 0, p = encodedMsg; i<blocks; i++) {
    uint8 a = unbytes64[*p++]; 
    uint8 b = unbytes64[*p++]; 
    uint8 c = unbytes64[*p++]; 
    uint8 d = unbytes64[*p++];
    *q++ = (a << 2) | (b >> 4);
    *q++ = (b << 4) | (c >> 2);
    *q++ = (c << 6) | d;
  }
  if (pad) { //now process padding block bytes
    uint8 a = unbytes64[*p++];
    uint8 b = unbytes64[*p++];
    *q++ = (a << 2) | (b >> 4);
    if (pad == 1) *q++ = (b << 4) | (unbytes64[*p] >> 2);
  }
  *len = q - msg;
  *decodedMsg = msg;
  return COMP_MSG_ERR_OK;;
}

// ============================= encryptMsg ========================

static uint8_t encryptMsg(compMsgDispatcher_t *self, const uint8_t *msg, size_t mlen, const uint8_t *key, size_t klen, const uint8_t *iv, size_t ivlen, uint8_t **buf, int *lgth) {
  const crypto_mech_t *mech;
  size_t bs;
  size_t clen;
  uint8_t *crypted;

  *buf = NULL;
  *lgth = 0;
  mech = crypto_encryption_mech ("AES-CBC");
  if (mech == NULL) {
    return COMP_MSG_ERR_CRYPTO_BAD_MECHANISM;
  }
  bs = mech->block_size;
  clen = ((mlen + bs - 1) / bs) * bs;
  *lgth = clen;
  COMP_MSG_DBG(self, "U", 2, "mlen: %d lgth: %d clen: %d data: %p\n", mlen, *lgth, clen, msg);
  crypted = (uint8_t *)os_zalloc (clen);
  if (!crypted) {
    return COMP_MSG_ERR_CRYPTO_INIT_FAILED;
  } 
  *buf = crypted;
  crypto_op_t op =
  { 
    key, klen,
    iv, ivlen,
    msg, mlen,
    crypted, clen,
    OP_ENCRYPT
  }; 
  if (!mech->run (&op)) { 
    os_free (crypted);
    return COMP_MSG_ERR_CRYPTO_INIT_FAILED;
  } 
  return COMP_MSG_ERR_OK;
}

// ============================= decryptMsg ========================

static uint8_t decryptMsg(compMsgDispatcher_t *self, const uint8_t *msg, size_t mlen, const uint8_t *key, size_t klen, const uint8_t *iv, size_t ivlen, uint8_t **buf, int *lgth) {
  const crypto_mech_t *mech;
  size_t bs;
  size_t clen;
  uint8_t *crypted;

  *buf = NULL;
  *lgth = 0;
  mech = crypto_encryption_mech ("AES-CBC");
  if (mech == NULL) {
    return COMP_MSG_ERR_CRYPTO_BAD_MECHANISM;
  }
  bs = mech->block_size;
  clen = ((mlen + bs - 1) / bs) * bs;
  *lgth = clen;
  crypted = (uint8_t *)os_zalloc (*lgth);
  if (!crypted) {
    return COMP_MSG_ERR_CRYPTO_INIT_FAILED;
  } 
  *buf = crypted;
  crypto_op_t op =
  { 
    key, klen,
    iv, ivlen,
    msg, mlen,
    crypted, clen,
    OP_DECRYPT
  }; 
  if (!mech->run (&op)) { 
    os_free (crypted);
    return COMP_MSG_ERR_CRYPTO_INIT_FAILED;
  }
  return COMP_MSG_ERR_OK;
}

// ============================= setFieldValueCallback ========================

static uint8_t setFieldValueCallback(compMsgDispatcher_t *self, uint8_t *callbackName, fieldValueCallback_t callback, uint8_t callbackType) {
  uint8_t result;
  fieldValueCallbackInfos_t *fieldValueCallbackInfo;
  int idx;

  idx = 0;
  while (idx < self->compMsgUtil->numFieldValueCallbackInfos) {
    fieldValueCallbackInfo = &self->compMsgUtil->fieldValueCallbackInfos[idx];
    if (c_strcmp(fieldValueCallbackInfo->callbackName, callbackName) == 0) {
      fieldValueCallbackInfo->callback = callback;
      fieldValueCallbackInfo->callbackType = callbackType;
      return COMP_MSG_ERR_OK;
    }
    idx++;
  }
  return COMP_MSG_ERR_FIELD_VALUE_CALLBACK_NOT_FOUND;
}

// ============================= addFieldValueCallbackName ========================

static uint8_t addFieldValueCallbackName(compMsgDispatcher_t *self, uint8_t *callbackName, fieldValueCallback_t callback, uint8_t callbackType) {
  uint8_t result;
  fieldValueCallbackInfos_t *fieldValueCallbackInfo;

  if (self->compMsgUtil->numFieldValueCallbackInfos >= self->compMsgUtil->maxFieldValueCallbackInfos) {
    if (self->compMsgUtil->fieldValueCallbackInfos == NULL) {
      self->compMsgUtil->maxFieldValueCallbackInfos = 30;
      self->compMsgUtil->fieldValueCallbackInfos = os_zalloc(self->compMsgUtil->maxFieldValueCallbackInfos * sizeof(fieldValueCallbackInfos_t));
      checkAllocOK(self->compMsgUtil->fieldValueCallbackInfos);
    } else {
      self->compMsgUtil->maxFieldValueCallbackInfos += 20;
      self->compMsgUtil->fieldValueCallbackInfos = os_realloc(self->compMsgUtil->fieldValueCallbackInfos, (self->compMsgUtil->maxFieldValueCallbackInfos * sizeof(fieldValueCallbackInfos_t)));
    }
  }
  fieldValueCallbackInfo = &self->compMsgUtil->fieldValueCallbackInfos[self->compMsgUtil->numFieldValueCallbackInfos];
  fieldValueCallbackInfo->callbackName = os_zalloc(c_strlen(callbackName) + 1);
  c_memcpy(fieldValueCallbackInfo->callbackName, callbackName, c_strlen(callbackName));
  fieldValueCallbackInfo->callback = callback;
  fieldValueCallbackInfo->callbackType = callbackType;
  self->compMsgUtil->numFieldValueCallbackInfos++;
  return COMP_MSG_ERR_OK;
}

// ============================= getFieldValueCallback ========================

static uint8_t getFieldValueCallback(compMsgDispatcher_t *self, uint8_t *callbackName, fieldValueCallback_t *callback, uint8_t callbackType) {
  uint8_t result;
  fieldValueCallbackInfos_t *fieldValueCallbackInfo;
  int idx;

  COMP_MSG_DBG(self, "U", 2, "getFieldValueCallback: %s 0x%02x\n", callbackName, callbackType);
  idx = 0;
  while (idx < self->compMsgUtil->numFieldValueCallbackInfos) {
    fieldValueCallbackInfo = &self->compMsgUtil->fieldValueCallbackInfos[idx];
    COMP_MSG_DBG(self, "U", 2, "getFieldValueCallback: %s %s", fieldValueCallbackInfo->callbackName, callbackName);
    if (c_strcmp(fieldValueCallbackInfo->callbackName, callbackName) == 0) {
      if ((callbackType == 0) || (fieldValueCallbackInfo->callbackType == callbackType)) {
        *callback = fieldValueCallbackInfo->callback;
        COMP_MSG_DBG(self, "U", 2, "getFieldValueCallback found: %s 0x%02x\n", callbackName, callbackType);
        return COMP_MSG_ERR_OK;
      }
    }
    idx++;
  }
  COMP_MSG_DBG(self, "U", 1, "getFieldValueCallback NOT found: %s 0x%02x\n", callbackName, callbackType);
  return COMP_MSG_ERR_FIELD_VALUE_CALLBACK_NOT_FOUND;
}

// ============================= getFieldValueCallbackName ========================

static uint8_t getFieldValueCallbackName(compMsgDispatcher_t *self, fieldValueCallback_t callback, uint8_t **callbackName, uint8_t callbackType) {
  uint8_t result;
  fieldValueCallbackInfos_t *fieldValueCallbackInfo;
  int idx;

  idx = 0;
  while (idx < self->compMsgUtil->numFieldValueCallbackInfos) {
    fieldValueCallbackInfo = &self->compMsgUtil->fieldValueCallbackInfos[idx];
    if (fieldValueCallbackInfo->callback == callback) {
      if ((callbackType == 0) || (fieldValueCallbackInfo->callbackType == callbackType)) {
        *callbackName = fieldValueCallbackInfo->callbackName;
        return COMP_MSG_ERR_OK;
      }
    }
    idx++;
  }
  COMP_MSG_DBG(self, "U", 1, "getFieldValueCallbackName NOT found: 0x%02x\n", callbackType);
  return COMP_MSG_ERR_FIELD_VALUE_CALLBACK_NOT_FOUND;
}

// ================================= compMsgUtilInit ====================================

uint8_t compMsgUtilInit(compMsgDispatcher_t *self) {
  uint8_t result;

  self->compMsgUtil->numFieldValueCallbackInfos = 0;
  self->compMsgUtil->maxFieldValueCallbackInfos = 0;
  self->compMsgUtil->fieldValueCallbackInfos = NULL;

  self->compMsgUtil->encryptMsg = &encryptMsg;
  self->compMsgUtil->decryptMsg = &decryptMsg;
  self->compMsgUtil->toBase64 = &toBase64;
  self->compMsgUtil->fromBase64 = &fromBase64;
  self->compMsgUtil->setFieldValueCallback = &setFieldValueCallback;
  self->compMsgUtil->addFieldValueCallbackName = &addFieldValueCallbackName;
  self->compMsgUtil->getFieldValueCallback = &getFieldValueCallback;
  self->compMsgUtil->getFieldValueCallbackName = &getFieldValueCallbackName;
  return COMP_MSG_ERR_OK;
}

// ================================= newCompMsgUtil ====================================

compMsgUtil_t *newCompMsgUtil() {
  compMsgUtil_t *compMsgUtil = os_zalloc(sizeof(compMsgUtil_t));
  if (compMsgUtil == NULL) {
    return NULL;
  }

  return compMsgUtil;
}


