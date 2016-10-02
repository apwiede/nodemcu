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
#include "c_stdio.h"
#include "structmsgDispatcher.h"


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

//#define checkAllocOK(addr) if(addr == NULL) return STRUCT_DISP_ERR_OUT_OF_MEMORY
//#define checkErrOK(result) if(result != STRUCT_DISP_ERR_OK) return result

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

// ================================= sendAnswer ====================================

static uint8_t sendAnswer(structmsgDispatcher_t *self, msgParts_t *parts) {
  if ((self->flags & DISP_FLAG_IS_TO_WIFI_MSG) && (self->flags & DISP_FLAG_IS_FROM_MCU_MSG)) {
    if (self->flags & DISP_FLAG_SHORT_CMD_KEY) { 
      switch (parts->shCmdKey) {
      case 'I':
ets_printf("sendAnswer I\n");
        break;
      }
    } else {
    }
  } else {
  }
  return STRUCT_DISP_ERR_OK;
}

// ================================= uartReceiveCb ====================================

static uint8_t uartReceiveCb(structmsgDispatcher_t *self, const uint8_t *buffer, uint8_t lgth) {
  int result;
  msgParts_t *received = &self->received;
  uint8_t u8;

ets_printf("§: %d %s§\n", lgth, buffer);
//  if (lgth != 1) {
//    return STRUCT_DISP_ERR_BAD_RECEIVED_LGTH;
//  }
  if (lgth == 0) {
    // simulate a '0' char!!
  }
//ets_printf("receivedLgth: %d fieldOffset: %d\n", received->lgth, received->fieldOffset);
  if (lgth == 0) {
    received->buf[received->lgth++] = 0;
  } else {
    received->buf[received->lgth++] = buffer[0];
  }
  self->structmsgDataView->dataView->data = received->buf;
  self->structmsgDataView->dataView->lgth++;
  if (received->lgth == RECEIVED_CHECK_TO_SIZE) {
    result = self->structmsgDataView->dataView->getUint16(self->structmsgDataView->dataView, received->fieldOffset, &received->toPart);
    if (received->toPart == self->WifiPart) {
      self->flags |= DISP_FLAG_IS_TO_WIFI_MSG;
    }
    received->fieldOffset += sizeof(uint16_t);
  }
  if (received->lgth == RECEIVED_CHECK_FROM_SIZE) {
    result = self->structmsgDataView->dataView->getUint16(self->structmsgDataView->dataView, received->fieldOffset, &received->fromPart);
    received->fieldOffset += sizeof(uint16_t);
    if (received->fromPart == self->McuPart) {
      self->flags |= DISP_FLAG_IS_FROM_MCU_MSG;
    }
  }
  if (received->lgth == RECEIVED_CHECK_TOTAL_LGTH_SIZE) {
    result = self->structmsgDataView->dataView->getUint16(self->structmsgDataView->dataView, received->fieldOffset, &received->totalLgth);
    received->fieldOffset += sizeof(uint16_t);
  }
  if ((self->flags & DISP_FLAG_IS_TO_WIFI_MSG) && (self->flags & DISP_FLAG_IS_FROM_MCU_MSG)) {
    if (self->flags & DISP_FLAG_SHORT_CMD_KEY) { 
      if (received->lgth == RECEIVED_CHECK_SHORT_CMD_KEY_SIZE) {
        result = self->structmsgDataView->dataView->getUint8(self->structmsgDataView->dataView, received->fieldOffset, &received->shCmdKey);
        received->fieldOffset += sizeof(uint8_t);
ets_printf("start dispatch: to: 0x%04x from: 0x%04x totalLgth: %d cmdKey: %c\n", received->toPart, received->fromPart, received->totalLgth, received->shCmdKey);
      }
      if (received->lgth > RECEIVED_CHECK_SHORT_CMD_KEY_SIZE) {
        if (received->lgth == received->totalLgth) {
ets_printf("dispatch length reached: to: 0x%04x from: 0x%04x totalLgth: %d cmdKey: %c\n", received->toPart, received->fromPart, received->totalLgth, received->shCmdKey);
          self->sendAnswer(self, received);
        }
      }
    } else {
      if (received->lgth == RECEIVED_CHECK_CMD_KEY_SIZE) {
        result = self->structmsgDataView->dataView->getUint16(self->structmsgDataView->dataView, received->fieldOffset, &received->cmdKey);
        received->fieldOffset += sizeof(uint16_t);
      }
    }
    if (self->flags & DISP_FLAG_HAVE_CMD_LGTH) { 
      if (received->lgth == RECEIVED_CHECK_CMD_LGTH_SIZE) {
      }
    }
  } else {
  }
  return STRUCT_DISP_ERR_OK;
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
  uint8_t result;

  self->flags = 0;
  self->flags |= DISP_FLAG_SHORT_CMD_KEY;
  self->McuPart = 0x4D00;
  self->WifiPart = 0x5700;
  self->AppPart = 0x4100;
  self->CloudPart = 0x4300;
  return STRUCT_DISP_ERR_OK;
}

// ================================= resetMsgInfo ====================================

static uint8_t resetMsgInfo(structmsgDispatcher_t *self, msgParts_t *parts) {
  uint8_t result;
  parts->lgth = 0;
  parts->fieldOffset;
  parts->totalLgth;
  parts->cmdLgth;
  parts->cmdKey;
  parts->fromPart;
  parts->toPart;
  parts->shCmdKey;
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

  structmsgDispatcher->uartReceiveCb = &uartReceiveCb;
  structmsgDispatcher->createDispatcher = &createDispatcher;

  structmsgDispatcher->BMsg = &BMsg;
  structmsgDispatcher->IMsg = &IMsg;
  structmsgDispatcher->MMsg = &MMsg;
  structmsgDispatcher->defaultMsg = &defaultMsg;
  structmsgDispatcher->sendAnswer = &sendAnswer;
  return structmsgDispatcher;
}

// ================================= freeStructmsgDispatcher ====================================

void freeStructmsgDispatcher(structmsgDispatcher_t *structmsgDispatcher) {
}


