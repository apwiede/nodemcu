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


#define HANDLE_PREFIX "stmsg_"

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
  STRUCT_MSG_FIELD_UINT16_VECTOR,
  STRUCT_MSG_FIELD_INT16_VECTOR,
  STRUCT_MSG_FIELD_UINT32_VECTOR,
  STRUCT_MSG_FIELD_INT32_VECTOR,
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
  STRUCT_MSG_ERR_NOT_ENCRYPTED = -18,
};

enum structmsg_special_fields
{
  STRUCT_MSG_FIELD_SRC = 1,
  STRUCT_MSG_FIELD_DST = 2,
  STRUCT_MSG_FIELD_TOTAL_LGTH = 3,
  STRUCT_MSG_FIELD_CMD_KEY = 4,
  STRUCT_MSG_FIELD_CMD_LGTH = 5,
};

#define STRUCT_MSG_ENCODED      (1 << 0)
#define STRUCT_MSG_DECODED      (1 << 1)
#define STRUCT_MSG_FIELD_IS_SET (1 << 2)
#define STRUCT_MSG_HAS_CRC      (1 << 3)

// header length: uint16_t src + uint16_t dst + uint16_t totalLgth
#define STRUCT_MSG_HEADER_LENGTH (sizeof(uint16_t) * 3)
// cmd header length uint16_t cmdKey + unit16_t cmdLgth
#define STRUCT_MSG_CMD_HEADER_LENGTH (4)
#define STRUCT_MSG_TOTAL_HEADER_LENGTH (STRUCT_MSG_HEADER_LENGTH + STRUCT_MSG_CMD_HEADER_LENGTH)

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
    int8_t *byteVector;
    uint8_t *ubyteVector;
    int16_t *shortVector;
    uint16_t *ushortVector;
    int32_t *int32Vector;
    uint32_t *uint32Vector;
  } value ;
  uint8_t flags;
} fieldInfo_t;


typedef struct hdrKeys
{
  uint16_t src;
  uint16_t dst;
  uint16_t totalLgth;
  uint16_t cmdKey;
  uint16_t cmdLgth;
} hdrKeys_t;

typedef union hdrInfo {
  hdrKeys_t hdrKeys;
  uint8_t hdrId[sizeof(hdrKeys_t)];
} hdrInfo_t;

typedef struct hdr
{
  hdrInfo_t hdrInfo;
  uint8_t headerLgth;
} hdr_t;

typedef struct msg
{
  fieldInfo_t *fieldInfos;
  uint8_t numFieldInfos;
  uint8_t maxFieldInfos;
} msg_t;

typedef struct structmsg
{
  hdr_t hdr;
  msg_t msg;
  char handle[16];
  uint8_t flags;
  uint8_t *encoded;
  uint8_t *todecode;
  uint8_t *encrypted;
  uint32_t sequenceNum;
  hdrInfo_t *handleHdrInfoPtr;
} structmsg_t;

int stmsg_getFieldTypeKey(const uint8_t *str);
int stmsg_createMsg(uint8_t numFieldInfos, uint8_t **handle);
int stmsg_deleteMsg(const uint8_t *handle);
int stmsg_encodeMsg(const uint8_t *handle);
int stmsg_getEncoded(const uint8_t *handle, uint8_t ** encoded, int *lgth);
int stmsg_decodeMsg(const uint8_t *handle, const uint8_t *data);
int stmsg_dumpMsg(const uint8_t *handle);
int stmsg_encdec(const uint8_t *handle, const uint8_t *key, size_t klen, const uint8_t *iv, size_t ivlen, bool enc, uint8_t **buf, int *lgth);
int stmsg_addField(const uint8_t *handle, const uint8_t *fieldStr, uint8_t fieldType, int fieldLgth);
int stmsg_setFillerAndCrc(const uint8_t *handle);
int stmsg_setFieldValue(const uint8_t *handle, const uint8_t *fieldName, int numericValue, const uint8_t *stringValue);
int stmsg_getFieldValue(const uint8_t *handle, const uint8_t *fieldName, int *numericValue, uint8_t **stringValue);
int stmsg_setCrypted(const uint8_t *handle, const uint8_t *crypted, int cryptedLgth);
int stmsg_decryptGetHandle(const uint8_t *encryptedMsg, size_t mlen, const uint8_t *key, size_t klen, const uint8_t *iv, size_t ivlen, uint8_t **handle);
#ifdef	__cplusplus
}
#endif

#endif	/* STRUCTMSG_H */
