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
 * File:   structmsg.h
 * Author: Arnulf P. Wiedemann
 *
 * Created on August 1, 2016
 */

#ifndef STRUCTMSG_H
#define	STRUCTMSG_H

#include "c_types.h"
#include "structmsgDataView.h"

#ifdef	__cplusplus
extern "C" {
#endif


#define HANDLE_PREFIX "stmsg_"

#define STRUCT_MSG_ENCODED        (1 << 0)
#define STRUCT_MSG_DECODED        (1 << 1)
#define STRUCT_MSG_FIELD_IS_SET   (1 << 2)
#define STRUCT_MSG_HAS_CRC        (1 << 3)
#define STRUCT_MSG_UINT8_CRC      (1 << 4)
#define STRUCT_MSG_HAS_FILLER     (1 << 5)
#define STRUCT_MSG_SHORT_CMD_KEY  (1 << 6)
#define STRUCT_MSG_HAS_TABLE_ROWS (1 << 7)

#ifdef NOTDEF
enum structmsg_special_fields
{
  STRUCT_MSG_FIELD_SRC                = 1,
  STRUCT_MSG_FIELD_DST                = 2,
  STRUCT_MSG_FIELD_TOTAL_LGTH         = 3,
  STRUCT_MSG_FIELD_CMD_KEY            = 4,
  STRUCT_MSG_FIELD_CMD_LGTH           = 5,
};

// header length: uint16_t src + uint16_t dst + uint16_t totalLgth
#define STRUCT_MSG_HEADER_LENGTH (sizeof(uint16_t) * 3)
#define STRUCT_MSG_SHORT_HEADER_LENGTH (sizeof(uint8_t) + sizeof(uint8_t))
// cmd header length uint16_t cmdKey + unit16_t cmdLgth
#define STRUCT_MSG_CMD_HEADER_LENGTH (sizeof(uint16_t) * 2)
#define STRUCT_MSG_SHORT_CMD_HEADER_LENGTH (sizeof(uint8_t))
#define STRUCT_MSG_TOTAL_HEADER_LENGTH (STRUCT_MSG_HEADER_LENGTH + STRUCT_MSG_CMD_HEADER_LENGTH)
#define STRUCT_MSG_SHORT_TOTAL_HEADER_LENGTH (STRUCT_MSG_SHORT_HEADER_LENGTH + STRUCT_MSG_SHORT_CMD_HEADER_LENGTH)
#define STRUCT_MSG_NUM_HEADER_FIELDS 3
#define STRUCT_MSG_NUM_CMD_HEADER_FIELDS 2
#define STRUCT_MSG_DEFINITION_CMD_KEY 0xFFFF

#define checkOffsetErrOK(result) if (result != STRUCT_MSG_ERR_OK) return -result
#define checkBadOffset(val) if (val < 0) return val
#define checkEncodeOffset(val) if (val < 0) return STRUCT_MSG_ERR_ENCODE_ERROR
#define checkDecodeOffset(val) if (val < 0) return STRUCT_MSG_ERR_DECODE_ERROR

typedef struct id2offset {
  uint16_t id;
  uint16_t offset;
} id2offset_t;

typedef struct str2key {
  uint8_t *str;
  uint8_t key;
} str2key_t;

typedef struct name2id {
  uint8_t *str;
  uint8_t id;
  uint8_t refCnt;
} name2id_t;

typedef struct fieldNameDefinitions
{
  size_t numDefinitions;
  size_t maxDefinitions;
  name2id_t *definitions;
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
  uint8_t *encrypted;
  uint8_t *todecode;
  uint16_t totalLgth;
  uint16_t flags;
  fieldInfoDefinition_t *fieldInfos;
} stmsgDefinition_t;

typedef struct stmsgDefinitions
{
  size_t numDefinitions;
  size_t maxDefinitions;
  stmsgDefinition_t *definitions;
} stmsgDefinitions_t;

#endif



#define checkHandleOK(addr) if(addr == NULL) return STRUCT_MSG_ERR_BAD_HANDLE

typedef struct structmsgData structmsgData_t;

typedef uint8_t (* createMsg_t)(structmsgData_t *self, int numFields, uint8_t **handle);
typedef uint8_t (* addField_t)(structmsgData_t *self, const uint8_t *fieldName, const uint8_t *fieldType, uint8_t fieldLgth);

typedef struct structmsgField {
  uint8_t fieldNameId;
  uint8_t fieldTypeId;
  uint8_t fieldLgth;
  uint8_t fieldFlags;
  size_t fieldOffset;
} structmsgField_t;

typedef struct structmsgData {
  structmsgDataView_t *structmsgDataView;
  char handle[16];
  structmsgField_t *fields;
  structmsgField_t *tableFields;
  uint16_t flags;
  size_t numFields;
  size_t maxFields;
  size_t numTableRows;         // number of list rows
  size_t numTableRowFields;    // number of fields within a table row
  size_t numRowFields;         // for checking how many tableRowFields have been processed
  size_t fieldOffset;
  uint8_t *header;
  createMsg_t createMsg;
  addField_t addField;
} structmsgData_t;


structmsgData_t *newStructmsgData(void);
void freeStructmsgData(structmsgData_t *structmsgdata);
uint8_t structmsgGetPtrFromHandle(const char *handle, structmsgData_t **structmsgData);
 
#ifdef	__cplusplus
}
#endif

#endif	/* STRUCTMSG_H */
