/* 
 * File:   structmsg.h
 * Author: Arnulf Wiedemann
 *
 * Created on August 1, 2016
 */

#ifndef STRUCTMSG_H
#define	STRUCTMSG_H
#include "c_types.h"
#ifdef	__cplusplus
extern "C" {
#endif

/*
* Copyright (c) 2016, Arnulf Wiedemann
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


#define HANDLE_PREFIX "sms_"

enum structmsg_field_type
{
  STRUCT_MSG_FIELD_UINT8_T,
  STRUCT_MSG_FIELD_INT8_T,
  STRUCT_MSG_FIELD_UINT16_T,
  STRUCT_MSG_FIELD_INT16_T,
  STRUCT_MSG_FIELD_UINT32_T,
  STRUCT_MSG_FIELD_INT32_T,
  STRUCT_MSG_FIELD_UINT8_VECTOR,
  STRUCT_MSG_FIELD_INT8_VECTOR,
};

enum structmsg_error_code
{
  STRUCT_MSG_ERR_OK = 0,
  STRUCT_MSG_ERR_VALUE_NOT_SET = -1,
  STRUCT_MSG_ERR_VALUE_OUT_OF_RANGE = -2,
  STRUCT_MSG_ERR_BAD_VALUE = -3,
  STRUCT_MSG_ERR_BAD_FIELD_TYPE = -4,
  STRUCT_MSG_ERR_FIELD_NOT_FOUND = -5,
  STRUCT_MSG_ERR_VALUE_TOO_BIG = -6,
  STRUCT_MSG_ERR_BAD_SPECIAL_FIELD = -7,
  STRUCT_MSG_ERR_BAD_HANDLE = -8,
  STRUCT_MSG_ERR_OUT_OF_MEMORY = -9,
  STRUCT_MSG_ERR_HANDLE_NOT_FOUND = -10,
  STRUCT_MSG_ERR_NOT_ENCODED = -11,
  STRUCT_MSG_ERR_ENCODE_ERROR = -12,
  STRUCT_MSG_ERR_DECODE_ERROR = -13,
  STRUCT_MSG_ERR_BAD_CRC_VALUE = -14,
  STRUCT_MSG_ERR_CRYPTO_INIT_FAILED = -15,
  STRUCT_MSG_ERR_CRYPTO_OP_FAILED = -16,
  STRUCT_MSG_ERR_CRYPTO_BAD_MECHANISM = -17,
};

#define STRUCT_MSG_ENCODED      (1 << 0)
#define STRUCT_MSG_DECODED      (1 << 1)
#define STRUCT_MSG_FIELD_IS_SET (1 << 2)
#define STRUCT_MSG_HAS_CRC      (1 << 3)

typedef int  (*structmsgCodingFcn)(uint8_t *data, int offset);

typedef struct str2key {
  uint8_t *str;
  uint8_t key;
} str2key_t;

typedef struct fieldInfo
{
  uint8_t *fieldStr;
  uint8_t fieldKey;
  uint8_t fieldType;
  uint16_t fieldLgth;
  union {
    int8_t byteVal;
    uint8_t ubyteVal;
    int16_t shortVal;
    uint16_t ushortVal;
    int32_t val;
    uint32_t uval;
    int8_t *vector;
    uint8_t *uvector;
  } value ;
  uint8_t flags;
} fieldInfo_t;


typedef struct msg
{
  uint16_t cmdKey;
  uint16_t cmdLgth;
  fieldInfo_t *fieldInfos;
  uint8_t numFieldInfos;
  uint8_t maxFieldInfos;
} msg_t;

typedef struct structmsg
{
  uint16_t src;
  uint16_t dst;
  uint16_t totalLgth;
  msg_t msg;
  char handle[16];
  uint8_t flags;
  uint8_t *encoded;
  uint8_t *todecode;
  uint8_t *encrypted;
  uint8_t *decrypted;
  uint8_t encryptedLgth;
  uint8_t decryptedLgth;
} structmsg_t;

int getFieldTypeKey(const uint8_t *str);
uint8_t *getStructMsgFieldTypeStr(uint8_t key);
int encodeMsg(const uint8_t *handle);
int getEncoded(const uint8_t *handle, uint8_t ** encoded, int *lgth);
int decodeMsg(const uint8_t *handle, const uint8_t *data);
int freeEncDecBuf(const uint8_t *handle, uint8_t *buf, bool enc);
int encdec(const uint8_t *handle, const uint8_t *key, size_t klen, const uint8_t *iv, size_t ivlen, bool enc, uint8_t **buf, int *lgth);
int getFieldValue(const uint8_t *handle, const uint8_t *fieldName, int *numericValue, uint8_t **stringValue);
int setFieldValue(const uint8_t *handle, const uint8_t *fieldName, int numericValue, const uint8_t *stringValue);
int setFillerAndCrc(const uint8_t *handle);
int addField(const uint8_t *handle, const uint8_t *fieldStr, uint8_t fieldType, int fieldLgth);
int dump_structmsg(const uint8_t *handle);
//structmsgCodingFcn getStructMsgFieldEncodeFcn(int key);
//structmsgCodingFcn getStructMsgFieldDecodeFcn(int key);
int set_targets(const uint8_t *handle, uint16_t src, uint16_t dst, uint16_t cmdKey);
int new_structmsg(uint8_t numFieldInfos, uint8_t **handle);
int delete_structmsg(const uint8_t *handle);

#ifdef	__cplusplus
}
#endif

#endif	/* STRUCTMSG_H */
