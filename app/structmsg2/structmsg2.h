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

#ifdef NOTDEF

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




#define HANDLE_PREFIX "stmsg_"

#define STRUCT_MSG_ENCODED        (1 << 0)
#define STRUCT_MSG_DECODED        (1 << 1)
#define STRUCT_MSG_HAS_CRC        (1 << 2)
#define STRUCT_MSG_UINT8_CRC      (1 << 3)
#define STRUCT_MSG_HAS_FILLER     (1 << 4)
#define STRUCT_MSG_SHORT_CMD_KEY  (1 << 5)
#define STRUCT_MSG_HAS_TABLE_ROWS (1 << 6)
#define STRUCT_MSG_IS_INITTED     (1 << 7)

#define STRUCT_MSG_FIELD_IS_SET   (1 << 0)

#define STRUCT_MSG_DEFINITION_CMD_KEY 0xFFFF

#define checkHandleOK(addr) if(addr == NULL) return STRUCT_MSG_ERR_BAD_HANDLE

typedef struct structmsgData structmsgData_t;

typedef uint8_t (* createMsg_t)(structmsgData_t *self, int numFields, uint8_t **handle);
typedef uint8_t (* deleteMsg_t)(structmsgData_t *self);
typedef uint8_t (* addField_t)(structmsgData_t *self, const uint8_t *fieldName, const uint8_t *fieldType, uint8_t fieldLgth);
typedef uint8_t (* getFieldValue_t)(structmsgData_t *self, const uint8_t *fieldName, int *numericValue, uint8_t **stringValue);
typedef uint8_t (* setFieldValue_t)(structmsgData_t *self, const uint8_t *fieldName, int numericValue, const uint8_t *stringValue);

typedef uint8_t (* getTableFieldValue_t)(structmsgData_t *self, const uint8_t *fieldName, int row, int *numericValue, uint8_t **stringValue);
typedef uint8_t (* setTableFieldValue_t)(structmsgData_t *self, const uint8_t *fieldName, int row, int numericValue, const uint8_t *stringValue);
typedef uint8_t (* dumpMsg_t)(structmsgData_t *self);
typedef void (* dumpBinary_t)(const uint8_t *data, uint8_t lgth, const uint8_t *where);
typedef uint8_t (* initMsg_t)(structmsgData_t *self);
typedef uint8_t (* prepareMsg_t)(structmsgData_t *self);
typedef uint8_t (* createMsgFromListInfo_t)(const uint8_t **listVector, uint8_t numEntries, uint8_t numRows, uint16_t flags, uint8_t **handle);

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
  size_t totalLgth;
  size_t cmdLgth;
  size_t headerLgth;
  uint8_t *header;
  createMsg_t createMsg;
  deleteMsg_t deleteMsg;
  addField_t addField;
  getFieldValue_t getFieldValue;
  setFieldValue_t setFieldValue;
  getTableFieldValue_t getTableFieldValue;
  setTableFieldValue_t setTableFieldValue;
  dumpMsg_t dumpMsg;
  dumpBinary_t dumpBinary;
  initMsg_t initMsg;
  prepareMsg_t prepareMsg;
} structmsgData_t;


structmsgData_t *newStructmsgData(void);
uint8_t structmsgGetPtrFromHandle(const char *handle, structmsgData_t **structmsgData);
uint8_t newStructmsgDataFromList(const uint8_t **listVector, uint8_t numEntries, uint8_t numRows, uint16_t flags, uint8_t **handle);
 
#ifdef	__cplusplus
}
#endif

#endif	/* STRUCTMSG_H */
