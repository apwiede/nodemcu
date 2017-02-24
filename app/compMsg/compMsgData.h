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
 * File:   compMsgData.h
 * Author: Arnulf P. Wiedemann
 *
 * Created on August 1, 2016
 */

#ifndef COMP_MSG_DATA_H
#define	COMP_MSG_DATA_H

#include "c_types.h"
typedef struct compMsgData compMsgData_t;
#include "compMsgDispatcher.h"

#ifdef	__cplusplus
extern "C" {
#endif

#define COMP_MSG_HAS_CRC              (1 << 0)
#define COMP_MSG_UINT8_CRC            (1 << 1)
#define COMP_MSG_HAS_FILLER           (1 << 2)
#define COMP_MSG_U8_CMD_KEY           (1 << 3)
#define COMP_MSG_HAS_TABLE_ROWS       (1 << 4)
#define COMP_MSG_IS_INITTED           (1 << 5)
#define COMP_MSG_IS_PREPARED          (1 << 6)
#define COMP_MSG_CRC_USE_HEADER_LGTH  (1 << 7)
#define COMP_MSG_HAS_TOTAL_CRC        (1 << 8)
#define COMP_MSG_UINT8_TOTAL_CRC      (1 << 9)

#define COMP_MSG_FIELD_IS_SET         (1 << 0)
#define COMP_MSG_KEY_VALUE_FIELD      (1 << 1)

#define COMP_MSG_TO_SEND_DATA     (1 << 0)
#define COMP_MSG_RECEIVED_DATA    (1 << 1)
#define COMP_MSG_TRANSFER_DATA    (1 << 2)

#define COMP_LIST_NUM_LIST_FIELDS 9
#define COMP_LIST_CMD_KEY 0x5A5A

#define NET_SOCKET_TYPE_SOCKET        0x01
#define NET_SOCKET_TYPE_CLIENT        0x02
#define WEB_SOCKET_TYPE_ACCESS_POINT  0x04
#define NET_SOCKET_TYPE_SSDP          0x08

#define checkHandleOK(addr) if(addr == NULL) return COMP_MSG_ERR_BAD_HANDLE

typedef struct buildMsgInfos {
  uint8_t numRows; 
  uint8_t tableRow;
  uint8_t tableCol;
  int numericValue;
  size_t sizeValue;
  uint8_t *stringValue;
  uint8_t *actionName;
  uint16_t srcId;
} buildMsgInfos_t;

typedef uint8_t (* createMsg_t)(compMsgDispatcher_t *self, int numFields, uint8_t **handle);
typedef uint8_t (* addField_t)(compMsgDispatcher_t *self, const uint8_t *fieldName, const uint8_t *fieldType, uint8_t fieldLgth);
typedef uint8_t (* getFieldValue_t)(compMsgDispatcher_t *self, const uint8_t *fieldName, int *numericValue, uint8_t **stringValue);
typedef uint8_t (* setFieldValue_t)(compMsgDispatcher_t *self, const uint8_t *fieldName, int numericValue, const uint8_t *stringValue);

typedef uint8_t (* dumpFieldValue_t)(compMsgDispatcher_t *self, compMsgField_t *fieldInfo, const uint8_t *indent2);
typedef uint8_t (* dumpKeyValueFields_t)(compMsgDispatcher_t *self, size_t offset);
typedef uint8_t (* dumpFieldInfo_t)(compMsgDispatcher_t *self, compMsgField_t *fieldInfo);
typedef uint8_t (* dumpMsg_t)(compMsgDispatcher_t *self);
typedef uint8_t (* initMsg_t)(compMsgDispatcher_t *self);
typedef uint8_t (* initReceivedMsg_t)(compMsgDispatcher_t *self);
typedef uint8_t (* prepareMsg_t)(compMsgDispatcher_t *self);
typedef uint8_t (* getMsgData_t)(compMsgDispatcher_t *compMsgData, uint8_t **data, int *lgth);
typedef uint8_t (* deleteMsgDescParts_t)(compMsgDispatcher_t *self);
typedef uint8_t (* deleteMsgValParts_t)(compMsgDispatcher_t *self);
typedef uint8_t (* deleteMsg_t)(compMsgDispatcher_t *self);
typedef uint8_t ( *setDispatcher_t)(compMsgData_t *self, compMsgDispatcher_t *dispatcher);

typedef struct compMsgData {
  compMsgDataView_t *compMsgDataView;
  compMsgDispatcher_t *compMsgDispatcher;
  char handle[16];
  compMsgField_t *fields;
  compMsgField_t *keyValueFields;
  uint16_t flags;
  size_t numFields;
  size_t maxFields;
  size_t numKeyValueFields;    // number of key value fields
  size_t numValueFields;       // for checking how many keyValueFields have been processed
  size_t fieldOffset;
  size_t totalLgth;
  size_t cmdLgth;
  size_t headerLgth;
  uint8_t *receivedData;
  uint16_t receivedLgth;
  uint8_t *toSendData;
  uint16_t toSendLgth;
  uint8_t direction;
  uint16_t u16CmdKey;
  msgParts_t received;
  msgParts_t toSend;

  msgDescPart_t *msgDescParts;
  size_t numMsgDescParts;
  size_t maxMsgDescParts;
  msgValPart_t *msgValParts;
  size_t numMsgValParts;
  size_t maxMsgValParts;
  uint8_t *prepareValuesCbName;
  
  buildMsgInfos_t buildMsgInfos;
  socketUserData_t *sud;
  msgDescPart_t *msgDescPart;
  msgValPart_t *msgValPart;
  headerPart_t *currHdr;

  // normalMsg
  createMsg_t createMsg;
  deleteMsg_t deleteMsg;
  addField_t addField;
  getFieldValue_t getFieldValue;
  setFieldValue_t setFieldValue;
  dumpFieldValue_t dumpFieldValue;
  dumpKeyValueFields_t dumpKeyValueFields;
  dumpFieldInfo_t dumpFieldInfo;
  dumpMsg_t dumpMsg;
  initMsg_t initMsg;
  initReceivedMsg_t initReceivedMsg;
  prepareMsg_t prepareMsg;
  getMsgData_t getMsgData;
  deleteMsgDescParts_t deleteMsgDescParts;
  deleteMsgValParts_t deleteMsgValParts;

  setDispatcher_t setDispatcher;

} compMsgData_t;

compMsgData_t *newCompMsgData(void);
uint8_t compMsgGetPtrFromHandle(const char *handle, compMsgDispatcher_t **compMsgDispatcher);
 
#ifdef	__cplusplus
}
#endif

#endif	/* COMP_MSG_DATA_H */
