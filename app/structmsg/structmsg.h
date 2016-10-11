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
#include "structmsgDispatcher.h"

#ifdef	__cplusplus
extern "C" {
#endif

#define STRUCT_MSG_HAS_CRC              (1 << 0)
#define STRUCT_MSG_UINT8_CRC            (1 << 1)
#define STRUCT_MSG_HAS_FILLER           (1 << 2)
#define STRUCT_MSG_U8_CMD_KEY           (1 << 3)
#define STRUCT_MSG_HAS_TABLE_ROWS       (1 << 4)
#define STRUCT_MSG_IS_INITTED           (1 << 5)
#define STRUCT_MSG_IS_PREPARED          (1 << 6)
#define STRUCT_DEF_IS_INITTED           (1 << 7)
#define STRUCT_DEF_IS_PREPARED          (1 << 8)
#define STRUCT_MSG_CRC_USE_HEADER_LGTH  (1 << 9)

#define STRUCT_MSG_FIELD_IS_SET   (1 << 0)

#define STRUCT_DEF_NUM_DEF_FIELDS 15
#define STRUCT_DEF_CMD_KEY 0xFFFF

#define STRUCT_DEF_TO_DATA        (1 << 0)
#define STRUCT_DEF_FROM_DATA      (1 << 1)

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
typedef uint8_t (* getMsgData_t)(structmsgData_t *structmsgData, uint8_t **data, int *lgth);
typedef uint8_t (* setMsgData_t)(structmsgData_t *structmsgData, const uint8_t *data);
typedef uint8_t (* setMsgFieldFromList_t)(structmsgData_t *selfconst, const uint8_t **listVector, uint8_t numEntries, uint16_t flags);

typedef uint8_t (* dumpDefFields_t)(structmsgData_t *self);
typedef uint8_t (* initDef_t)(structmsgData_t *self);
typedef uint8_t (* prepareDef_t)(structmsgData_t *self);
typedef uint8_t (* addDefField_t)(structmsgData_t *self, uint8_t fieldNameId, uint8_t fieldTypeId, uint8_t fieldLgth);
typedef uint8_t (* getDefFieldValue_t)(structmsgData_t *self, const uint8_t fieldNameId, int *numericValue, uint8_t **stringValue, int idx);
typedef uint8_t (* setDefFieldValue_t)(structmsgData_t *self, uint8_t fieldNameId, int numericValue, const uint8_t *stringValue, int idx);
typedef uint8_t (* setDef_t)(structmsgData_t *self, const uint8_t *data);
typedef uint8_t (* getDef_t)(structmsgData_t *self, uint8_t **data, int *lgth);
typedef uint8_t (* createMsgFromDef_t)(structmsgData_t *self);

typedef struct structmsgData {
  structmsgDataView_t *structmsgDataView;
  structmsgDataView_t *structmsgDefinitionDataView;
  structmsgDataDescription_t *structmsgDataDescription;
  char handle[16];
  structmsgField_t *fields;
  structmsgField_t *tableFields;
  structmsgField_t *defFields;
  uint16_t flags;
  size_t numFields;
  size_t maxFields;
  size_t numTableRows;         // number of list rows
  size_t numTableRowFields;    // number of fields within a table row
  size_t numRowFields;         // for checking how many tableRowFields have been processed
  size_t numDefFields;         // for checking how many defFields have been processed
  size_t fieldOffset;
  size_t defFieldOffset;
  size_t totalLgth;
  size_t defTotalLgth;
  size_t cmdLgth;
  size_t headerLgth;
  size_t defNumNormFields;
  size_t defNormNamesSize;
  size_t defDefinitionsSize;
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
  getMsgData_t getMsgData;
  setMsgData_t setMsgData;
  setMsgFieldFromList_t setMsgFieldsFromList;

  initDef_t initDef;
  prepareDef_t prepareDef;
  dumpDefFields_t dumpDefFields;
  addDefField_t addDefField;
  getDefFieldValue_t getDefFieldValue;
  setDefFieldValue_t setDefFieldValue;
  setDef_t setDef;
  getDef_t getDef;
  createMsgFromDef_t createMsgFromDef;

} structmsgData_t;


structmsgData_t *newStructmsgData(void);
uint8_t structmsgGetPtrFromHandle(const char *handle, structmsgData_t **structmsgData);
uint8_t newStructmsgDataFromList(const uint8_t **listVector, uint8_t numEntries, uint8_t numRows, uint16_t flags, uint8_t **handle);
uint8_t newStructmsgDefinition(structmsgData_t *structmsgData);
 
#ifdef	__cplusplus
}
#endif

#endif	/* STRUCTMSG_H */
