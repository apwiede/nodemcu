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
 * File:   structmsgDispatcher.c
 * Author: Arnulf P. Wiedemann
 *
 * Created on October 2st, 2016
 */

#include "osapi.h"
#include "c_types.h"
#include "mem.h"
#include "flash_fs.h"

#include "c_string.h"
#include "c_stdlib.h"
#include "c_stdio.h"
#include "platform.h"
#include "structmsg2.h"


#define DISP_HANDLE_PREFIX "stmsgdisp_"
#define RECEIVED_CHECK_HEADER_SIZE 7

#define DISP_FLAG_SHORT_CMD_KEY    (1 << 0)
#define DISP_FLAG_HAVE_CMD_LGTH    (1 << 1)
#define DISP_FLAG_IS_ENCRYPTED     (1 << 2)
#define DISP_FLAG_IS_TO_WIFI_MSG   (1 << 3)
#define DISP_FLAG_IS_FROM_MCU_MSG  (1 << 4)

#define RECEIVED_CHECK_TO_SIZE            2
#define RECEIVED_CHECK_FROM_SIZE          4
#define RECEIVED_CHECK_TOTAL_LGTH_SIZE    6
#define RECEIVED_CHECK_SHORT_CMD_KEY_SIZE 7
#define RECEIVED_CHECK_CMD_KEY_SIZE       8
#define RECEIVED_CHECK_CMD_LGTH_SIZE      10

typedef struct handle2Dispatcher
{
  uint8_t *handle;
  structmsgDispatcher_t *structmsgDispatcher;
} handle2Dispatcher_t;

typedef struct structmsgDispatcherHandles
{
  handle2Dispatcher_t *handles;
  int numHandles;
} structmsgDispatcherHandles_t;

// create an object
static structmsgDispatcherHandles_t structmsgDispatcherHandles = { NULL, 0};

static int structmsgDispatcherId = 0;
// right now we only need one dispatcher!
static structmsgDispatcher_t *structmsgDispatcherSingleton = NULL;

// ============================= addHandle ========================

static int addHandle(uint8_t *handle, structmsgDispatcher_t *structmsgDispatcher) {
  int idx;

  if (structmsgDispatcherHandles.handles == NULL) {
    structmsgDispatcherHandles.handles = os_zalloc(sizeof(handle2Dispatcher_t));
    if (structmsgDispatcherHandles.handles == NULL) {
      return STRUCT_DISP_ERR_OUT_OF_MEMORY;
    } else {
      structmsgDispatcherHandles.handles[structmsgDispatcherHandles.numHandles].handle = handle;
      structmsgDispatcherHandles.handles[structmsgDispatcherHandles.numHandles].structmsgDispatcher = structmsgDispatcher;
      structmsgDispatcherHandles.numHandles++;
      return STRUCT_DISP_ERR_OK;
    }
  } else {
    // check for unused slot first
    idx = 0;
    while (idx < structmsgDispatcherHandles.numHandles) {
      if (structmsgDispatcherHandles.handles[idx].handle == NULL) {
        structmsgDispatcherHandles.handles[idx].handle = handle;
        structmsgDispatcherHandles.handles[idx].structmsgDispatcher = structmsgDispatcher;
        return STRUCT_DISP_ERR_OK;
      }
      idx++;
    }
    structmsgDispatcherHandles.handles = os_realloc(structmsgDispatcherHandles.handles, sizeof(handle2Dispatcher_t)*(structmsgDispatcherHandles.numHandles+1));
    checkAllocOK(structmsgDispatcherHandles.handles);
    structmsgDispatcherHandles.handles[structmsgDispatcherHandles.numHandles].handle = handle;
    structmsgDispatcherHandles.handles[idx].structmsgDispatcher = structmsgDispatcher;
    structmsgDispatcherHandles.numHandles++;
  }
  return STRUCT_DISP_ERR_OK;
}

// ============================= deleteHandle ========================

static int deleteHandle(const uint8_t *handle) {
  int idx;
  int numUsed;
  int found;

  if (structmsgDispatcherHandles.handles == NULL) {
    return STRUCT_DISP_ERR_HANDLE_NOT_FOUND;
  }
  found = 0;
  idx = 0;
  numUsed = 0;
  while (idx < structmsgDispatcherHandles.numHandles) {
    if ((structmsgDispatcherHandles.handles[idx].handle != NULL) && (c_strcmp(structmsgDispatcherHandles.handles[idx].handle, handle) == 0)) {
      structmsgDispatcherHandles.handles[idx].handle = NULL;
      found++;
    } else {
      if (structmsgDispatcherHandles.handles[idx].handle != NULL) {
        numUsed++;
      }
    }
    idx++;
  }
  if (numUsed == 0) {
    os_free(structmsgDispatcherHandles.handles);
    structmsgDispatcherHandles.handles = NULL;
  }
  if (found) {
      return STRUCT_DISP_ERR_OK;
  }
  return STRUCT_DISP_ERR_HANDLE_NOT_FOUND;
}

// ============================= checkHandle ========================

static int checkHandle(const char *handle, structmsgDispatcher_t **structmsgDispatcher) {
  int idx;

  if (structmsgDispatcherHandles.handles == NULL) {
    return STRUCT_DISP_ERR_HANDLE_NOT_FOUND;
  }
  idx = 0;
  while (idx < structmsgDispatcherHandles.numHandles) {
    if ((structmsgDispatcherHandles.handles[idx].handle != NULL) && (c_strcmp(structmsgDispatcherHandles.handles[idx].handle, handle) == 0)) {
      *structmsgDispatcher = structmsgDispatcherHandles.handles[idx].structmsgDispatcher;
      return STRUCT_DISP_ERR_OK;
    }
    idx++;
  }
  return STRUCT_DISP_ERR_HANDLE_NOT_FOUND;
}

// ================================= BMsg ====================================

static uint8_t BMsg(structmsgDispatcher_t *self) {
  return STRUCT_DISP_ERR_OK;
}

// ================================= IMsg ====================================

static uint8_t IMsg(structmsgDispatcher_t *self) {
  return STRUCT_DISP_ERR_OK;
}

// ================================= MMsg ====================================

static uint8_t MMsg(structmsgDispatcher_t *self) {
  return STRUCT_DISP_ERR_OK;
}

// ================================= defaultMsg ====================================

static uint8_t defaultMsg(structmsgDispatcher_t *self) {
  return STRUCT_DISP_ERR_OK;
}

#define checkErrWithResetOK(result) if(result != DATA_VIEW_ERR_OK) { dataView->data = saveData; dataView->lgth= saveLgth; return result; }

// ================================= getMsgPtrFromMsgParts ====================================

static uint8_t getMsgPtrFromMsgParts(structmsgDispatcher_t *self, msgParts_t *msgParts, structmsgData_t **structmsgData, int incrRefCnt) {
  int firstFreeEntryId;
  int headerIdx;
  msgHeader2MsgPtr_t *entry;
  msgHeader2MsgPtr_t *headerEntry;
  msgHeader2MsgPtr_t *newHeaderEntry;
  msgHeader2MsgPtr_t *firstFreeEntry;
  uint8_t header[DISP_MAX_HEADER_LGTH];
  uint8_t result;
  dataView_t *dataView;
  uint8_t *saveData;
  uint8_t saveLgth;
  size_t offset;

  // build header from msgParts
  offset = 0;
  dataView = self->structmsgDataView->dataView;
  saveData = dataView->data;
  saveLgth = dataView->lgth;
  dataView->data = header;
  dataView->lgth = DISP_MAX_HEADER_LGTH;
  result = dataView->setUint16(dataView, offset, msgParts->fromPart);
  checkErrWithResetOK(result);
  offset += sizeof(uint16_t);
  result = dataView->setUint16(dataView, offset, msgParts->toPart);
  checkErrWithResetOK(result);
  offset += sizeof(uint16_t);
  result = dataView->setUint16(dataView, offset, msgParts->totalLgth);
  checkErrWithResetOK(result);
  offset += sizeof(uint16_t);
  if (self->dispFlags & STRUCT_MSG_SHORT_CMD_KEY) {
    result = dataView->setUint8(dataView, offset, msgParts->shCmdKey);
    checkErrWithResetOK(result);
    offset += sizeof(uint8_t);
  } else {
    result = dataView->setUint16(dataView, offset, msgParts->cmdKey);
    checkErrWithResetOK(result);
    offset += sizeof(uint16_t);
  }
  dataView->data = saveData;
  dataView->lgth= saveLgth;
  // end build header from msgParts
  if ((incrRefCnt == STRUCT_MSG_INCR) && (self->numMsgHeaders >= self->maxMsgHeaders)) {
    if (self->maxMsgHeaders == 0) {
      self->maxMsgHeaders = 4;
      self->msgHeader2MsgPtrs = (msgHeader2MsgPtr_t *)os_zalloc((self->maxMsgHeaders * sizeof(msgHeader2MsgPtr_t)));
      checkAllocOK(self->msgHeader2MsgPtrs);
    } else {
      self->maxMsgHeaders += 2;
      self->msgHeader2MsgPtrs = (msgHeader2MsgPtr_t *)os_realloc(self->msgHeader2MsgPtrs, (self->maxMsgHeaders * sizeof(msgHeader2MsgPtr_t)));
      checkAllocOK(self->msgHeader2MsgPtrs);
    }
  }
  firstFreeEntry = NULL;
  firstFreeEntryId = 0;
  if (self->numMsgHeaders > 0) {
    // find header 
    headerIdx = 0;
    while (headerIdx < self->numMsgHeaders) {
      headerEntry = &self->msgHeader2MsgPtrs[headerIdx];
      if ((headerEntry->structmsgData != NULL) && (ets_memcmp(headerEntry->header, header, offset) == 0)) {
        if (incrRefCnt < 0) {
          headerEntry->headerLgth = 0;
          os_free(headerEntry->structmsgData);
          headerEntry->structmsgData = NULL;
          *structmsgData = NULL;
          return STRUCT_DISP_ERR_OK;
        }
        *structmsgData = headerEntry->structmsgData;
        (*structmsgData)->flags &= ~STRUCT_MSG_IS_PREPARED;
        return STRUCT_DISP_ERR_OK;
      }
      if ((incrRefCnt == STRUCT_MSG_INCR) && (headerEntry->structmsgData == NULL) && (firstFreeEntry == NULL)) {
        firstFreeEntry = headerEntry;
        firstFreeEntry->structmsgData = newStructmsgData();
        firstFreeEntry->headerLgth = offset;
        c_memcpy(firstFreeEntry->header, header, offset);
        *structmsgData = firstFreeEntry->structmsgData;
      }
      headerIdx++;
    }
  }
  if (incrRefCnt < 0) {
    return STRUCT_DISP_ERR_OK; // just ignore silently
  } else {
    if (incrRefCnt == 0) {
      return STRUCT_DISP_ERR_HEADER_NOT_FOUND;
    } else {
      if (firstFreeEntry != NULL) {
        *structmsgData = firstFreeEntry->structmsgData;
        firstFreeEntry->headerLgth = offset;
        c_memcpy(firstFreeEntry->header, header, offset);
      } else {
        newHeaderEntry = &self->msgHeader2MsgPtrs[self->numMsgHeaders];
        newHeaderEntry->headerLgth = offset;
        c_memcpy(newHeaderEntry->header, header, offset);
        newHeaderEntry->structmsgData = newStructmsgData();
        *structmsgData = newHeaderEntry->structmsgData;
        self->numMsgHeaders++;
      }
    }
  }
  return STRUCT_DISP_ERR_OK;
}

#undef checkErrWithResetOK

// ================================= getFieldType ====================================

static uint8_t getFieldType(structmsgDispatcher_t *self, structmsgData_t *structmsgData, uint8_t fieldNameId, uint8_t *fieldTypeId) {
  int idx;
  structmsgField_t *fieldInfo;

  idx = 0;
  while (idx < structmsgData->numFields) {
    fieldInfo = &structmsgData->fields[idx];
    if (fieldInfo->fieldNameId == fieldNameId) {
      *fieldTypeId = fieldInfo->fieldTypeId;
      return STRUCT_DISP_ERR_OK;
    }
    idx++;
  }
  // and now check the table fields
  idx = 0;
  while (idx < structmsgData->numTableRowFields) {
    fieldInfo = &structmsgData->tableFields[idx];
    if (fieldInfo->fieldNameId == fieldNameId) {
      *fieldTypeId = fieldInfo->fieldTypeId;
      return STRUCT_DISP_ERR_OK;
    }
    idx++;
  }
  return STRUCT_DISP_ERR_FIELD_NOT_FOUND;
}

// ================================= setMsgValuesFromLines ====================================

static uint8_t setMsgValuesFromLines(structmsgDispatcher_t *self, structmsgData_t *structmsgData, uint8_t numEntries, uint8_t *handle, uint8_t type) {
  int idx;
  uint8_t*cp;
  uint8_t *fieldNameStr;
  uint8_t fieldNameId;
  uint8_t fieldTypeId;
  uint8_t *fieldValueStr;
  char *endPtr;
  uint8_t fieldLgth;
  uint8_t *flagStr;
  uint8_t flag;
  uint8_t lgth;
  unsigned long uval;
  uint8_t buf[100];
  uint8_t *buffer = buf;
  int numericValue;
  uint8_t *stringValue;
  structmsgDataView_t *dataView;
  int result;

  idx = 0;
  dataView = structmsgData->structmsgDataView;
  while(idx < numEntries) {
    result = self->readLine(self, &buffer, &lgth);
    checkErrOK(result);
    if (lgth == 0) {
      break;
    }
    buffer[lgth] = 0;
    fieldNameStr = buffer;
    cp = fieldNameStr;
    while (*cp != ',') {
      cp++;
    }
    *cp++ = '\0';
    result = dataView->getFieldNameIdFromStr(dataView, fieldNameStr, &fieldNameId, STRUCT_MSG_NO_INCR);
    checkErrOK(result);
    result = getFieldType(self, structmsgData, fieldNameId, &fieldTypeId);
    checkErrOK(result);
    fieldValueStr = cp;
    while (*cp != '\n') {
      cp++;
    }
    *cp++ = '\0';
    switch (fieldTypeId) {
    case DATA_VIEW_FIELD_UINT8_T:
    case DATA_VIEW_FIELD_INT8_T:
    case DATA_VIEW_FIELD_UINT16_T:
    case DATA_VIEW_FIELD_INT16_T:
    case DATA_VIEW_FIELD_UINT32_T:
    case DATA_VIEW_FIELD_INT32_T:
      {
        uval = c_strtoul(fieldValueStr, &endPtr, 10);
        if (endPtr == (char *)(cp-1)) {
          numericValue = (int)uval;
          stringValue = NULL;
        } else {
          numericValue = 0;
          stringValue = fieldValueStr;
        }
      }
      break;
    default:
      numericValue = 0;
      stringValue = fieldValueStr;
      break;
    }
    switch (fieldNameId) {
      case STRUCT_MSG_SPEC_FIELD_DST:
        numericValue = self->received.fromPart;
        stringValue = NULL;
        result = structmsgData->setFieldValue(structmsgData, fieldNameStr, numericValue, stringValue);
        break;
      case STRUCT_MSG_SPEC_FIELD_SRC:
        numericValue = self->received.toPart;
        stringValue = NULL;
        result = structmsgData->setFieldValue(structmsgData, fieldNameStr, numericValue, stringValue);
        break;
      case STRUCT_MSG_SPEC_FIELD_CMD_KEY:
        // check for shCmdKey/cmdKey here
        if (self->dispFlags & STRUCT_MSG_SHORT_CMD_KEY) {
          numericValue = self->received.shCmdKey;
        } else {
          numericValue = self->received.cmdKey;
        }
        stringValue = NULL;
        result = structmsgData->setFieldValue(structmsgData, fieldNameStr, numericValue, stringValue);
        break;
      default:
        result = structmsgData->setFieldValue(structmsgData, fieldNameStr, numericValue, stringValue);
        break;
    }
    checkErrOK(result);
    idx++;
  }
  result = structmsgData->setFieldValue(structmsgData, "@cmdKey", type, NULL);
  checkErrOK(result);
  structmsgData->prepareMsg(structmsgData);
//  structmsgData->dumpMsg(structmsgData);
  return STRUCT_DISP_ERR_OK;
}

// ================================= createMsgFromLines ====================================

static uint8_t createMsgFromLines(structmsgDispatcher_t *self, msgParts_t *parts, uint8_t numEntries, uint8_t numRows, uint8_t type, structmsgData_t **structmsgData, uint8_t **handle) {
  int idx;
  uint8_t*cp;
  uint8_t *fieldNameStr;
  uint8_t *fieldTypeStr;
  uint8_t *fieldLgthStr;
  char *endPtr;
  uint8_t fieldLgth;
  uint8_t *flagStr;
  uint8_t flag;
  uint8_t lgth;
  unsigned long uflag;
  uint8_t buf[100];
  uint8_t *buffer = buf;
  int result;

//ets_printf("§createMsgFromLines:%d!%d! \n§", self->numMsgHeaders, self->maxMsgHeaders);
  result = getMsgPtrFromMsgParts(self, parts, structmsgData, STRUCT_MSG_INCR);
  checkErrOK(result);
  if ((*structmsgData)->flags & STRUCT_MSG_IS_INITTED) {
    return STRUCT_DISP_ERR_OK;
  }
  result = (*structmsgData)->createMsg(*structmsgData, numEntries, handle);
  checkErrOK(result);
  idx = 0;
  while(idx < numEntries) {
    result = self->readLine(self, &buffer, &lgth);
    checkErrOK(result);
    if (lgth == 0) {
      break;
    }
    buffer[lgth] = 0;
    fieldNameStr = buffer;
    cp = fieldNameStr;
    while (*cp != ',') {
      cp++;
    }
    *cp++ = '\0';
    fieldTypeStr = cp;
    while (*cp != ',') {
      cp++;
    }
    *cp++ = '\0';
    fieldLgthStr = cp;
    while (*cp != '\n') {
      cp++;
    }
    *cp++ = '\0';
    if (c_strcmp(fieldLgthStr,"@numRows") == 0) {
      fieldLgth = numRows;
    } else {
      lgth = c_strtoul(fieldLgthStr, &endPtr, 10);
      fieldLgth = (uint8_t)lgth;
    }
    result = (*structmsgData)->addField(*structmsgData, fieldNameStr, fieldTypeStr, fieldLgth);
    checkErrOK(result);
    idx++;
  }
  (*structmsgData)->initMsg(*structmsgData);
  return STRUCT_DISP_ERR_OK;
}

// ================================= sendAnswer ====================================

static uint8_t sendAnswer(structmsgDispatcher_t *self, msgParts_t *parts, uint8_t type) {
  uint8_t fileName[30];
  uint8_t lgth;
  int msgLgth;
  uint8_t *data;
  uint8_t buf[100];
  uint8_t *buffer = buf;
  int result;
  uint8_t numRows;
  char *endPtr;
  uint8_t *handle;
  unsigned long ulgth;
  structmsgData_t *structmsgData;
  uint8_t numEntries;
  int idx;

//ets_printf("§@1@§", parts->shCmdKey);
  if ((self->dispFlags & DISP_FLAG_IS_TO_WIFI_MSG) && (self->dispFlags & DISP_FLAG_IS_FROM_MCU_MSG)) {
    if (self->dispFlags & DISP_FLAG_SHORT_CMD_KEY) { 
      switch (parts->shCmdKey) {
      case 'B':
      case 'I':
      case 'M':
//ets_printf("§@sendAnswer!%c!@§", parts->shCmdKey);
        os_sprintf(fileName, "Desc%c%c.txt", parts->shCmdKey, type);
        result = self->openFile(self, fileName, "r");
        checkErrOK(result);
        result = self->readLine(self, &buffer, &lgth);
        checkErrOK(result);
        if ((lgth < 4) || (buffer[0] != '#')) {
          return STRUCT_DISP_ERR_BAD_FILE_CONTENTS;
        }
        ulgth = c_strtoul(buffer+2, &endPtr, 10);
        numEntries = (uint8_t)ulgth;
//ets_printf("§@NE1!%d!@§", numEntries);
        numRows = 0;
        result = createMsgFromLines(self, parts, numEntries, numRows, type, &structmsgData, &handle);
        checkErrOK(result);
//ets_printf("heap2: %d\n", system_get_free_heap_size());
        result = self->closeFile(self);
        checkErrOK(result);
        os_sprintf(fileName, "Val%c%c.txt", parts->shCmdKey, type);
        result = self->openFile(self, fileName, "r");
        checkErrOK(result);
        result = self->readLine(self, &buffer, &lgth);
        checkErrOK(result);
        if ((lgth < 4) || (buffer[0] != '#')) {
          return STRUCT_DISP_ERR_BAD_FILE_CONTENTS;
        }
        ulgth = c_strtoul(buffer+2, &endPtr, 10);
        numEntries = (uint8_t)ulgth;
//ets_printf("§@NE2!%d!@§", numEntries);
        result = setMsgValuesFromLines(self, structmsgData, numEntries, handle, parts->shCmdKey);
        checkErrOK(result);
        result = self->closeFile(self);
        checkErrOK(result);
ets_printf("§heap3: %d§", system_get_free_heap_size());
        result = structmsgData->getMsgData(structmsgData, &data, &msgLgth);
        checkErrOK(result);
        idx = 0;
        while (idx < msgLgth) {
          platform_uart_send(0, data[idx]);
          idx++;
        }
        break;
      }
    } else {
    }
  } else {
  }
  self->resetMsgInfo(self, parts);
  return STRUCT_DISP_ERR_OK;
}

// ================================= uartReceiveCb ====================================

static uint8_t uartReceiveCb(structmsgDispatcher_t *self, const uint8_t *buffer, uint8_t lgth) {
  int result;
  msgParts_t *received = &self->received;
  dataView_t *dataView;
  uint8_t u8;

//ets_printf("§%c§", buffer[0]&0xFF);
//ets_printf("§receivedLgth: %d fieldOffset: %d\n§", received->lgth, received->fieldOffset);
  if (lgth == 0) {
    // simulate a '0' char!!
    received->buf[received->lgth++] = 0;
  } else {
    received->buf[received->lgth++] = buffer[0];
  }
  dataView = self->structmsgDataView->dataView;
  dataView->data = received->buf;
  dataView->lgth++;
  if (received->lgth == RECEIVED_CHECK_TO_SIZE) {
    result = dataView->getUint16(dataView, received->fieldOffset, &received->toPart);
    if (received->toPart == self->WifiPart) {
      self->dispFlags |= DISP_FLAG_IS_TO_WIFI_MSG;
    }
    received->fieldOffset += sizeof(uint16_t);
  }
  if (received->lgth == RECEIVED_CHECK_FROM_SIZE) {
    result = dataView->getUint16(dataView, received->fieldOffset, &received->fromPart);
    received->fieldOffset += sizeof(uint16_t);
    if (received->fromPart == self->McuPart) {
      self->dispFlags |= DISP_FLAG_IS_FROM_MCU_MSG;
    }
  }
  if (received->lgth == RECEIVED_CHECK_TOTAL_LGTH_SIZE) {
    result = dataView->getUint16(dataView, received->fieldOffset, &received->totalLgth);
    received->fieldOffset += sizeof(uint16_t);
  }
  if ((self->dispFlags & DISP_FLAG_IS_TO_WIFI_MSG) && (self->dispFlags & DISP_FLAG_IS_FROM_MCU_MSG)) {
    if (self->dispFlags & DISP_FLAG_SHORT_CMD_KEY) { 
      if (received->lgth == RECEIVED_CHECK_SHORT_CMD_KEY_SIZE) {
        result = dataView->getUint8(dataView, received->fieldOffset, &received->shCmdKey);
        received->fieldOffset += sizeof(uint8_t);
      }
      if (received->lgth > RECEIVED_CHECK_SHORT_CMD_KEY_SIZE) {
        if (received->lgth == received->totalLgth) {
//ets_printf("heap1: %d\n", system_get_free_heap_size());
//ets_printf("§@received!%c!@§", received->shCmdKey);
          result = self->sendAnswer(self, received, 'A');
          checkErrOK(result);
          result = self->resetMsgInfo(self, &self->received);
          checkErrOK(result);
        }
      }
    } else {
      if (received->lgth == RECEIVED_CHECK_CMD_KEY_SIZE) {
        result = dataView->getUint16(dataView, received->fieldOffset, &received->cmdKey);
        checkErrOK(result);
        received->fieldOffset += sizeof(uint16_t);
      }
    }
    if (self->dispFlags & DISP_FLAG_HAVE_CMD_LGTH) { 
      if (received->lgth == RECEIVED_CHECK_CMD_LGTH_SIZE) {
      }
    }
  } else {
  }
  return STRUCT_DISP_ERR_OK;
}

// ================================= openFile ====================================

static uint8_t openFile(structmsgDispatcher_t *self, const uint8_t *fileName, const uint8_t *fileMode) {
  return self->structmsgDataDescription->openFile(self->structmsgDataDescription, fileName, fileMode);
}

// ================================= closeFile ====================================

static uint8_t closeFile(structmsgDispatcher_t *self) {
  return self->structmsgDataDescription->closeFile(self->structmsgDataDescription);
}

// ================================= flushFile ====================================

static uint8_t flushFile(structmsgDispatcher_t *self) {
  return STRUCT_DATA_DESC_ERR_FLUSH_FILE;
}

// ================================= readLine ====================================

static uint8_t readLine(structmsgDispatcher_t *self, uint8_t **buffer, uint8_t *lgth) {
  return self->structmsgDataDescription->readLine(self->structmsgDataDescription, buffer, lgth);
}

// ================================= writeLine ====================================

static uint8_t writeLine(structmsgDispatcher_t *self, const uint8_t *buffer, uint8_t lgth) {
  return self->structmsgDataDescription->writeLine(self->structmsgDataDescription, buffer, lgth);
}

// ============================= structmsgDispatcherGetPtrFromHandle ========================

uint8_t structmsgDispatcherGetPtrFromHandle(const char *handle, structmsgDispatcher_t **structmsgDispatcher) {

  if (checkHandle(handle, structmsgDispatcher) != STRUCT_DISP_ERR_OK) {
    return STRUCT_DISP_ERR_HANDLE_NOT_FOUND;
  }
  return STRUCT_DISP_ERR_OK;
}

// ================================= initHeadersAndFlags ====================================

static uint8_t initHeadersAndFlags(structmsgDispatcher_t *self) {
  self->dispFlags = 0;
  self->dispFlags |= DISP_FLAG_SHORT_CMD_KEY;
  self->McuPart = 0x4D00;
  self->WifiPart = 0x5700;
  self->AppPart = 0x4100;
  self->CloudPart = 0x4300;
  return STRUCT_DISP_ERR_OK;
}

// ================================= resetMsgInfo ====================================

static uint8_t resetMsgInfo(structmsgDispatcher_t *self, msgParts_t *parts) {
  parts->lgth = 0;
  parts->fieldOffset = 0;
  parts->totalLgth = 0;
  parts->cmdLgth = 0;
  parts->cmdKey = 0;
  parts->fromPart = 0;
  parts->toPart = 0;
  parts->shCmdKey = 0;
  self->structmsgDataView->dataView->data = parts->buf;
  self->structmsgDataView->dataView->lgth = 0;
  return STRUCT_DISP_ERR_OK;
}


// ================================= createDispatcher ====================================

static uint8_t createDispatcher(structmsgDispatcher_t *self, uint8_t **handle) {
  uint8_t result;
  os_sprintf(self->handle, "%s%p", DISP_HANDLE_PREFIX, self);
  result = addHandle(self->handle, self);
  if (result != STRUCT_DISP_ERR_OK) {
    deleteHandle(self->handle);
    os_free(self);
    return result;
  }
  initHeadersAndFlags(self);
  resetMsgInfo(self, &self->received);
  resetMsgInfo(self, &self->toSend);
  *handle = self->handle;
  return STRUCT_DISP_ERR_OK;
}

// ================================= newStructmsgDispatcher ====================================

structmsgDispatcher_t *newStructmsgDispatcher() {
  if (structmsgDispatcherSingleton != NULL) {
    return structmsgDispatcherSingleton;
  }
  structmsgDispatcher_t *structmsgDispatcher = os_zalloc(sizeof(structmsgDispatcher_t));
  if (structmsgDispatcher == NULL) {
    return NULL;
  }
  structmsgDispatcher->structmsgDataView = newStructmsgDataView();
  if (structmsgDispatcher->structmsgDataView == NULL) {
    return NULL;
  }

  structmsgDispatcherId++;
  structmsgDispatcher->id = structmsgDispatcherId;

  structmsgDispatcher->numMsgHeaders = 0;
  structmsgDispatcher->maxMsgHeaders = 0;
  structmsgDispatcher->msgHeader2MsgPtrs = NULL;


  structmsgDispatcher->structmsgDataDescription = newStructmsgDataDescription();

  structmsgDispatcher->uartReceiveCb = &uartReceiveCb;
  structmsgDispatcher->createDispatcher = &createDispatcher;

  structmsgDispatcher->BMsg = &BMsg;
  structmsgDispatcher->IMsg = &IMsg;
  structmsgDispatcher->MMsg = &MMsg;
  structmsgDispatcher->defaultMsg = &defaultMsg;
  structmsgDispatcher->resetMsgInfo = &resetMsgInfo;
  structmsgDispatcher->sendAnswer = &sendAnswer;

  structmsgDispatcher->openFile = &openFile;
  structmsgDispatcher->closeFile = &closeFile;
  structmsgDispatcher->readLine = &readLine;
  structmsgDispatcher->writeLine = &writeLine;

  structmsgDispatcherSingleton = structmsgDispatcher;
  return structmsgDispatcher;
}

// ================================= freeStructmsgDispatcher ====================================

void freeStructmsgDispatcher(structmsgDispatcher_t *structmsgDispatcher) {
}


