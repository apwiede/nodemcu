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
  STRUCT_MSG_FIELD_NONE_T        = 0,
  STRUCT_MSG_FIELD_UINT8_T       = 1,
  STRUCT_MSG_FIELD_INT8_T        = 2,
  STRUCT_MSG_FIELD_UINT16_T      = 3,
  STRUCT_MSG_FIELD_INT16_T       = 4,
  STRUCT_MSG_FIELD_UINT32_T      = 5,
  STRUCT_MSG_FIELD_INT32_T       = 6,
  STRUCT_MSG_FIELD_UINT8_VECTOR  = 7,
  STRUCT_MSG_FIELD_INT8_VECTOR   = 8,
  STRUCT_MSG_FIELD_UINT16_VECTOR = 9,
  STRUCT_MSG_FIELD_INT16_VECTOR  = 10,
  STRUCT_MSG_FIELD_UINT32_VECTOR = 11,
  STRUCT_MSG_FIELD_INT32_VECTOR  = 12,
};

enum structmsg_special_field_names
{
  STRUCT_MSG_SPEC_FIELD_SRC              = 255,
  STRUCT_MSG_SPEC_FIELD_DST              = 254,
  STRUCT_MSG_SPEC_FIELD_TARGET_CMD       = 253,
  STRUCT_MSG_SPEC_FIELD_TOTAL_LGTH       = 252,
  STRUCT_MSG_SPEC_FIELD_CMD_KEY          = 251,
  STRUCT_MSG_SPEC_FIELD_CMD_LGTH         = 250,
  STRUCT_MSG_SPEC_FIELD_RANDOM_NUM       = 249,
  STRUCT_MSG_SPEC_FIELD_SEQUENCE_NUM     = 248,
  STRUCT_MSG_SPEC_FIELD_FILLER           = 247,
  STRUCT_MSG_SPEC_FIELD_CRC              = 246,
  STRUCT_MSG_SPEC_FIELD_ID               = 245,
  STRUCT_MSG_SPEC_FIELD_TABLE_ROWS       = 244,
  STRUCT_MSG_SPEC_FIELD_TABLE_ROW_FIELDS = 243,
  STRUCT_MSG_SPEC_FIELD_LOW              = 242,         // this must be the last entry!!
};

enum structmsg_error_code
{
  STRUCT_MSG_ERR_OK                    = 0,
  STRUCT_MSG_ERR_VALUE_NOT_SET         = 255,
  STRUCT_MSG_ERR_VALUE_OUT_OF_RANGE    = 254,
  STRUCT_MSG_ERR_BAD_VALUE             = 253,
  STRUCT_MSG_ERR_BAD_FIELD_TYPE        = 252,
  STRUCT_MSG_ERR_FIELD_NOT_FOUND       = 251,
  STRUCT_MSG_ERR_VALUE_TOO_BIG         = 250,
  STRUCT_MSG_ERR_BAD_SPECIAL_FIELD     = 249,
  STRUCT_MSG_ERR_BAD_HANDLE            = 248,
  STRUCT_MSG_ERR_OUT_OF_MEMORY         = 247,
  STRUCT_MSG_ERR_HANDLE_NOT_FOUND      = 246,
  STRUCT_MSG_ERR_NOT_ENCODED           = 245,
  STRUCT_MSG_ERR_ENCODE_ERROR          = 244,
  STRUCT_MSG_ERR_DECODE_ERROR          = 243,
  STRUCT_MSG_ERR_BAD_CRC_VALUE         = 242,
  STRUCT_MSG_ERR_CRYPTO_INIT_FAILED    = 241,
  STRUCT_MSG_ERR_CRYPTO_OP_FAILED      = 240,
  STRUCT_MSG_ERR_CRYPTO_BAD_MECHANISM  = 239,
  STRUCT_MSG_ERR_NOT_ENCRYPTED         = 238,
  STRUCT_MSG_ERR_DEFINITION_NOT_FOUND  = 237,
  STRUCT_MSG_ERR_DEFINITION_TOO_MANY_FIELDS = 236,
  STRUCT_MSG_ERR_BAD_TABLE_ROW         = 235,
  STRUCT_MSG_ERR_TOO_MANY_FIELDS       = 234,
  STRUCT_MSG_ERR_BAD_DEFINTION_CMD_KEY = 233,
};

enum structmsg_special_fields
{
  STRUCT_MSG_FIELD_SRC                = 1,
  STRUCT_MSG_FIELD_DST                = 2,
  STRUCT_MSG_FIELD_TOTAL_LGTH         = 3,
  STRUCT_MSG_FIELD_CMD_KEY            = 4,
  STRUCT_MSG_FIELD_CMD_LGTH           = 5,
};

#define STRUCT_MSG_ENCODED      (1 << 0)
#define STRUCT_MSG_DECODED      (1 << 1)
#define STRUCT_MSG_FIELD_IS_SET (1 << 2)
#define STRUCT_MSG_HAS_CRC      (1 << 3)

// header length: uint16_t src + uint16_t dst + uint16_t totalLgth
#define STRUCT_MSG_HEADER_LENGTH (sizeof(uint16_t) * 3)
// cmd header length uint16_t cmdKey + unit16_t cmdLgth
#define STRUCT_MSG_CMD_HEADER_LENGTH (sizeof(uint16_t) * 2)
#define STRUCT_MSG_TOTAL_HEADER_LENGTH (STRUCT_MSG_HEADER_LENGTH + STRUCT_MSG_CMD_HEADER_LENGTH)
#define STRUCT_MSG_DEFINITION_CMD_KEY 0xFFFF

#define checkEncodeOffset(val) if (val < 0) return STRUCT_MSG_ERR_ENCODE_ERROR
#define checkDecodeOffset(val) if (val < 0) return STRUCT_MSG_ERR_DECODE_ERROR
#define checkAllocOK(addr) if(addr == NULL) return STRUCT_MSG_ERR_OUT_OF_MEMORY
#define checkHandleOK(addr) if(addr == NULL) return STRUCT_MSG_ERR_BAD_HANDLE
#define checkErrOK(result) if(result != STRUCT_MSG_ERR_OK) return result

typedef struct id2offset {
  uint16_t id;
  uint16_t offset;
} id2offset_t;

typedef struct str2key {
  uint8_t *str;
  uint8_t key;
} str2key_t;

typedef struct fieldNameDefinitions
{
  size_t numDefinitions;
  size_t maxDefinitions;
  str2key_t *definitions;
} fieldNameDefinitions_t;

typedef struct fieldInfoDefinition
{
  uint16_t fieldId;
  uint8_t fieldType;
  uint16_t fieldLgth;
} fieldInfoDefinition_t;

typedef struct stmsgDefinition
{
  size_t numFields;
  size_t maxFields;
  uint8_t *name;
  uint8_t *encoded;
  fieldInfoDefinition_t *fieldInfos;
} stmsgDefinition_t;

typedef struct stmsgDefinitions
{
  size_t numDefinitions;
  size_t maxDefinitions;
  stmsgDefinition_t *definitions;
} stmsgDefinitions_t;

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
  fieldInfo_t *tableFieldInfos;
  uint8_t numFieldInfos;
  uint8_t maxFieldInfos;
  uint8_t numTableRows;         // number of list rows
  uint8_t numTableRowFields;    // number of fields within a table row
  uint8_t numRowFields;         // for checking how many tableRowFields have been processed
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

extern str2key_t structmsgFieldTypes[];

structmsg_t *structmsg_get_structmsg_ptr( const uint8_t *handle );
void structmsg_dumpBinary(const uint8_t *data, uint8_t lgth, const uint8_t *where);
int structmsg_getFieldTypeKey(const uint8_t *str);
int structmsg_getFieldNameId (const uint8_t *fieldName, int *key);
uint8_t *structmsg_getFieldTypeStr(uint8_t key);
int structmsg_createStructmsgDefinition (const uint8_t *name, size_t numFields);
int structmsg_addFieldDefinition (const uint8_t *name, const uint8_t *fieldName, const uint8_t *fieldTypeStr, size_t fieldLgth);
int structmsg_dumpFieldDefinition (const uint8_t *name);

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
int stmsg_setTableFieldValue(const uint8_t *handle, const uint8_t *fieldName, int row, int numericValue, const uint8_t *stringValue);
int stmsg_getFieldValue(const uint8_t *handle, const uint8_t *fieldName, int *numericValue, uint8_t **stringValue);
int stmsg_getTableFieldValue(const uint8_t *handle, const uint8_t *fieldName, int row, int *numericValue, uint8_t **stringValue);
int stmsg_setCrypted(const uint8_t *handle, const uint8_t *crypted, int cryptedLgth);
int stmsg_decryptGetHandle(const uint8_t *encryptedMsg, size_t mlen, const uint8_t *key, size_t klen, const uint8_t *iv, size_t ivlen, uint8_t **handle);
int structmsg_encodeFieldDefinitionMessage (const uint8_t *name, uint8_t **data, int *lgth);
int structmsg_decodeFieldDefinitionMessage (const uint8_t *name, const uint8_t *data);
int structmsg_fillHdrInfo(const uint8_t *handle, structmsg_t *structmsg);
int structmsg_encodeDefinitions (const uint8_t *name, uint8_t **data, int *lgth, stmsgDefinitions_t *structmsgDefinitions, fieldNameDefinitions_t *fieldNameDefinitions);
int structmsg_decodeDefinitions (const uint8_t *name, const uint8_t *data, stmsgDefinitions_t *structmsgDefinitions, fieldNameDefinitions_t *fieldNameDefinitions);
#ifdef	__cplusplus
}
#endif

#endif	/* STRUCTMSG_H */
