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
 * File:   compMsgDispatcher.c
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
#include "driver/uart.h"
#include "compMsgDispatcher.h"

#ifdef GDB_STUB
#include "../gdbstub/gdbstub.h"
#endif

#define DISP_HANDLE_PREFIX "stmsgdisp_"
#define KEY_VALUE_DESC_PARTS_FILE "CompMsgKeyValueKeys.txt"

typedef struct handle2Dispatcher
{
  uint8_t *handle;
  compMsgDispatcher_t *compMsgDispatcher;
} handle2Dispatcher_t;

typedef struct compMsgDispatcherHandles
{
  handle2Dispatcher_t *handles;
  int numHandles;
} compMsgDispatcherHandles_t;

// create an object
static compMsgDispatcherHandles_t compMsgDispatcherHandles = { NULL, 0};

static int compMsgDispatcherId = 0;
// right now we only need one dispatcher!
static compMsgDispatcher_t *compMsgDispatcherSingleton = NULL;

// ============================= addHandle ========================

static int addHandle(uint8_t *handle, compMsgDispatcher_t *self) {
  int idx;

  COMP_MSG_DBG(self, "D", 2, "dispatcher addHandle: %s!\n", handle);
  if (compMsgDispatcherHandles.handles == NULL) {
    compMsgDispatcherHandles.handles = os_zalloc(sizeof(handle2Dispatcher_t));
    if (compMsgDispatcherHandles.handles == NULL) {
      return COMP_MSG_ERR_OUT_OF_MEMORY;
    } else {
      compMsgDispatcherHandles.handles[compMsgDispatcherHandles.numHandles].handle = handle;
      compMsgDispatcherHandles.handles[compMsgDispatcherHandles.numHandles].compMsgDispatcher = self;
      compMsgDispatcherHandles.numHandles++;
      return COMP_MSG_ERR_OK;
    }
  } else {
    // check for unused slot first
    idx = 0;
    while (idx < compMsgDispatcherHandles.numHandles) {
      if (compMsgDispatcherHandles.handles[idx].handle == NULL) {
        compMsgDispatcherHandles.handles[idx].handle = handle;
        compMsgDispatcherHandles.handles[idx].compMsgDispatcher = self;
        return COMP_MSG_ERR_OK;
      }
      idx++;
    }
    compMsgDispatcherHandles.handles = os_realloc(compMsgDispatcherHandles.handles, sizeof(handle2Dispatcher_t)*(compMsgDispatcherHandles.numHandles+1));
    checkAllocOK(compMsgDispatcherHandles.handles);
    compMsgDispatcherHandles.handles[compMsgDispatcherHandles.numHandles].handle = handle;
    compMsgDispatcherHandles.handles[compMsgDispatcherHandles.numHandles].compMsgDispatcher = self;
    compMsgDispatcherHandles.numHandles++;
  }
  return COMP_MSG_ERR_OK;
}

// ============================= deleteHandle ========================

static int deleteHandle(compMsgDispatcher_t *self) {
  int idx;
  int numUsed;
  int found;
  const uint8_t *handle;

  handle = self->handle;
  COMP_MSG_DBG(self, "D", 1, "dispatcher deleteHandle: %s!\n", handle);
  if (compMsgDispatcherHandles.handles == NULL) {
    COMP_MSG_DBG(self, "Y", 0, "dispatcher deleteHandle 1 HANLDE_NOT_FOUND\n");
    return COMP_MSG_ERR_HANDLE_NOT_FOUND;
  }
  found = 0;
  idx = 0;
  numUsed = 0;
  if (compMsgDispatcherHandles.numHandles == 0) {
      return COMP_MSG_ERR_OK;
  }
  while (idx < compMsgDispatcherHandles.numHandles) {
    if ((compMsgDispatcherHandles.handles[idx].handle != NULL) && (c_strcmp(compMsgDispatcherHandles.handles[idx].handle, handle) == 0)) {
      compMsgDispatcherHandles.handles[idx].handle = NULL;
      found++;
    } else {
      if (compMsgDispatcherHandles.handles[idx].handle != NULL) {
        numUsed++;
      }
    }
    idx++;
  }
  if (numUsed == 0) {
    os_free(compMsgDispatcherHandles.handles);
    compMsgDispatcherHandles.handles = NULL;
  }
  if (found) {
      return COMP_MSG_ERR_OK;
  }
  COMP_MSG_DBG(self, "Y", 0, "deleteHandle 2 HANLDE_NOT_FOUND\n");
  return COMP_MSG_ERR_HANDLE_NOT_FOUND;
}

// ============================= checkHandle ========================

static int checkHandle(const char *handle, compMsgDispatcher_t **compMsgDispatcher) {
  int idx;

  if (compMsgDispatcherHandles.handles == NULL) {
    ets_printf("dispatcher checkHandle 1 HANLDE_NOT_FOUND\n");
    return COMP_MSG_ERR_HANDLE_NOT_FOUND;
  }
  idx = 0;
  while (idx < compMsgDispatcherHandles.numHandles) {
    if ((compMsgDispatcherHandles.handles[idx].handle != NULL) && (c_strcmp(compMsgDispatcherHandles.handles[idx].handle, handle) == 0)) {
      *compMsgDispatcher = compMsgDispatcherHandles.handles[idx].compMsgDispatcher;
      return COMP_MSG_ERR_OK;
    }
    idx++;
  }
  ets_printf("dispatcher checkHandle 2 HANDLE_NOT_FOUND\n");
  return COMP_MSG_ERR_HANDLE_NOT_FOUND;
}

// ================================= getNewCompMsgDataPtr ====================================

static uint8_t getNewCompMsgDataPtr(compMsgDispatcher_t *self) {
  int firstFreeEntryId;
  msgHeader2MsgPtr_t *newHeaderEntry;
  uint8_t result;

  if (self->numMsgHeaders >= self->maxMsgHeaders) {
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
  newHeaderEntry = &self->msgHeader2MsgPtrs[self->numMsgHeaders];
  newHeaderEntry->compMsgData = newCompMsgData();
  newHeaderEntry->compMsgData->setDispatcher(newHeaderEntry->compMsgData, self);
  self->compMsgData = newHeaderEntry->compMsgData;
  self->numMsgHeaders++;
  return COMP_MSG_ERR_OK;
}

// ================================= getFieldType ====================================

static uint8_t getFieldType(compMsgDispatcher_t *self, compMsgData_t *compMsgData, uint8_t fieldNameId, uint8_t *fieldTypeId) {
  int idx;
  compMsgField_t *fieldInfo;

  idx = 0;
  while (idx < compMsgData->numFields) {
    fieldInfo = &compMsgData->fields[idx];
    if (fieldInfo->fieldNameId == fieldNameId) {
      *fieldTypeId = fieldInfo->fieldTypeId;
      return COMP_MSG_ERR_OK;
    }
    idx++;
  }
  return COMP_MSG_ERR_FIELD_NOT_FOUND;
}

// ================================= resetBuildMsgInfos ====================================

static uint8_t resetBuildMsgInfos(compMsgDispatcher_t *self) {
  self->compMsgData->buildMsgInfos.numRows = 0;
  self->compMsgData->buildMsgInfos.tableRow = 0;
  self->compMsgData->buildMsgInfos.tableCol = 0;
  self->compMsgData->buildMsgInfos.numericValue = 0;
  self->compMsgData->buildMsgInfos.stringValue = NULL;
  self->compMsgData->buildMsgInfos.actionName = NULL;
  return COMP_MSG_ERR_OK;
}

// ================================= resetMsgInfo ====================================

static uint8_t resetMsgInfo(compMsgDispatcher_t *self, msgParts_t *parts) {
  parts->lgth = 0;
  parts->fieldOffset = 0;
  parts->fromPart = 0;
  parts->toPart = 0;
  parts->totalLgth = 0;
  c_memcpy(parts->GUID, "                ", 16);
  parts->srcId = 0;
  parts->u16CmdKey = 0;
  return COMP_MSG_ERR_OK;
}

// ============================= compMsgDispatcherGetPtrFromHandle ========================

uint8_t compMsgDispatcherGetPtrFromHandle(const char *handle, compMsgDispatcher_t **compMsgDispatcher) {

  if (checkHandle(handle, compMsgDispatcher) != COMP_MSG_ERR_OK) {
    ets_printf("compMsgDispatcherGetPtrFromHandle 1 HANLDE_NOT_FOUND\n");
    return COMP_MSG_ERR_HANDLE_NOT_FOUND;
  }
  return COMP_MSG_ERR_OK;
}

#ifdef GDB_STUB
int WWWAAA = 123;
uint8_t tst3(int i1) {
  return WWWAAA + i1;
}
uint8_t tst2(compMsgDispatcher_t *self, int i1, int i2) {
  uint8_t result;
  result = tst3(i1);
  return result;
}
uint8_t tst1(compMsgDispatcher_t *self, const uint8_t *type, size_t typelen) {
  int yyyQQQ;
  int yyyYYY;
  char *yyyZZZ;
  uint8_t result;
  uint8_t result2;
  headerPart_t *hdr;
  uint8_t *handle = NULL;
  int id = 12;
  int stopbits =34;
  int parity = 56;
  int databits =78;

yyyYYY = 456;
yyyZZZ = "Hello Arnulf";
ets_printf("self: %p\n", &self);
ets_printf("type: %p\n", &type);
ets_printf("typelen: %p\n", &typelen);
ets_printf("yyyQQQ: %p\n", &yyyQQQ);
ets_printf("yyyZZZ: %p\n", &yyyZZZ);
ets_printf("yyyYYY: %p\n", &yyyYYY);
ets_printf("result2: %p\n", &result2);
ets_printf("hdr: %p\n", &hdr);
ets_printf("handle: %p\n", &handle);
ets_printf("id: %p\n", &id);
ets_printf("stopbits: %p\n", &stopbits);
ets_printf("parity: %p\n", &parity);
ets_printf("databits: %p\n", &databits);
#ifdef NOTDEF
//COMP_MSG_DBG(self, "Y", 0, "call DescInit");
  result = compMsgMsgDescInit(self);
  checkErrOK(result);
  result = compMsgUtilInit(self);
  checkErrOK(result);
  result = compMsgRequestInit(self);
  checkErrOK(result);
//COMP_MSG_DBG(self, "Y", 0, "call IdentifyInit");
  result = compMsgIdentifyInit(self);
  checkErrOK(result);
  result = compMsgBuildMsgInit(self);
  checkErrOK(result);
  result = compMsgSendReceiveInit(self);
  checkErrOK(result);
  result = compMsgActionInit(self);
  checkErrOK(result);
//COMP_MSG_DBG(self, "Y", 0, "call WifiInit");
  result = compMsgWifiInit(self);
  checkErrOK(result);
  result = compMsgModuleDataInit(self);
  checkErrOK(result);
  result = compMsgTimerInit(self);
  checkErrOK(result);
  result = compMsgWebSocketInit(self);
  checkErrOK(result);
  result = compMsgNetSocketInit(self);
  checkErrOK(result);
  result = compMsgHttpInit(self);
  checkErrOK(result);
//COMP_MSG_DBG(self, "Y", 0, "call OtaInit");
  result = compMsgOtaInit(self);
  checkErrOK(result);
#endif
#ifdef GDB_STUB
ets_printf("dispInit2\n");
#endif
  result = self->compMsgMsgDesc->getMsgKeyValueDescParts(self, KEY_VALUE_DESC_PARTS_FILE);
  return result;
}
#endif

// ================================= initDispatcher ====================================

uint8_t initDispatcher(compMsgDispatcher_t *self, const uint8_t *type, size_t typelen) {
  int xxxQQQ;
  int xxxYYY;
  char *xxxZZZ;
  uint8_t result;
  headerPart_t *hdr;
  uint8_t *handle;
  int id;
  int stopbits;
  int parity;
  int databits;
  uint32_t baud;
  size_t msgLgth;
  uint8_t *msgData;

#ifdef GDB_STUB
gdbstub_init();
#endif
#ifdef GDB_STUB
result = tst1(self, type, typelen);
xxxYYY = 456;
xxxZZZ = "Hello Arnulf";
ets_printf("initDispatcher: self: %p %p xxxZZZ: %p %p\n", self, &self, xxxZZZ, &xxxZZZ);
ets_printf("self: %p\n", &self);
ets_printf("type: %p\n", &type);
ets_printf("typelen: %p\n", &typelen);
ets_printf("xxxQQQ: %p\n", &xxxQQQ);
ets_printf("xxxZZZ: %p\n", &xxxZZZ);
ets_printf("xxxYYY: %p\n", &xxxYYY);
ets_printf("result: %p\n", &result);
ets_printf("hdr: %p\n", &hdr);
ets_printf("handle: %p\n", &handle);
ets_printf("id: %p\n", &id);
ets_printf("stopbits: %p\n", &stopbits);
ets_printf("parity: %p\n", &parity);
ets_printf("databits: %p\n", &databits);
ets_printf("baud: %p\n", &baud);
ets_printf("msgLgth: %p\n", &msgLgth);
ets_printf("msgData: %p\n", &msgData);
ets_printf("WWWAAA: %p\n", &WWWAAA);
ets_printf("dispInit1\n");
#endif
//COMP_MSG_DBG(self, "Y", 0, "call DescInit");
  result = compMsgMsgDescInit(self);
  checkErrOK(result);
  result = compMsgUtilInit(self);
  checkErrOK(result);
  result = compMsgRequestInit(self);
  checkErrOK(result);
//COMP_MSG_DBG(self, "Y", 0, "call IdentifyInit");
  result = compMsgIdentifyInit(self);
  checkErrOK(result);
  result = compMsgBuildMsgInit(self);
  checkErrOK(result);
  result = compMsgSendReceiveInit(self);
  checkErrOK(result);
  result = compMsgActionInit(self);
  checkErrOK(result);
//COMP_MSG_DBG(self, "Y", 0, "call WifiInit");
  result = compMsgWifiInit(self);
  checkErrOK(result);
  result = compMsgModuleDataInit(self);
  checkErrOK(result);
  result = compMsgTimerInit(self);
  checkErrOK(result);
  result = compMsgWebSocketInit(self);
  checkErrOK(result);
  result = compMsgNetSocketInit(self);
  checkErrOK(result);
  result = compMsgHttpInit(self);
  checkErrOK(result);
//COMP_MSG_DBG(self, "Y", 0, "call OtaInit");
  result = compMsgOtaInit(self);
  checkErrOK(result);
#ifdef GDB_STUB
ets_printf("dispInit2\n");
#endif
  result = self->compMsgMsgDesc->getMsgKeyValueDescParts(self, KEY_VALUE_DESC_PARTS_FILE);

  if (typelen > 0) {
    switch(type[0]) {
    case 'A':
      COMP_MSG_DBG(self, "B", 1, "start send AA message");
      result = self->compMsgMsgDesc->getHeaderFromUniqueFields(self, 16640,22272, 0x4141, &hdr);
      checkErrOK(result);
      result = self->compMsgBuildMsg->createMsgFromHeaderPart(self, hdr, &handle);
      COMP_MSG_DBG(self, "B", 1, "handle: %s result: %d", handle, result);
      checkErrOK(result);
      break;
    case 'C':
      COMP_MSG_DBG(self, "H", 1, "start startCloudSocket");
      result = self->compMsgSocket->netSocketStartCloudSocket(self);
      checkErrOK(result);
      break;
    case 'D':
      self->compMsgDebug->setDebugFlags(self, "BdDEHIOSvw");
      COMP_MSG_DBG(self, "O", 1, "save user data");
      result = self->compMsgOta->saveUserData(self);
      checkErrOK(result);
      result = self->compMsgOta->getUserData(self, &msgData, &msgLgth);
      COMP_MSG_DBG(self, "O", 1, "get user data msgLgth: %d", msgLgth);
      checkErrOK(result);
      result = self->compMsgIdentify->handleReceivedPart(self, msgData, msgLgth);
      checkErrOK(result);
      break;
    case 'G':
#ifdef GDB_STUB
ets_printf("dispInit3\n");
#endif
      self->compMsgDebug->setDebugFlags(self, "BHIWsSw");
      COMP_MSG_DBG(self, "Y", 1, "start gdbstub_init\n");
#ifdef GDB_STUB
ets_printf("dispInit4\n");
#endif
      break;
    case 'I':
      self->compMsgDebug->setDebugFlags(self, "BHIWsSw");
      COMP_MSG_DBG(self, "Y", 1, "start startTestInterrupt\n");
      result = self->compMsgAction->startTestInterrupt(self);
      checkErrOK(result);
      break;
    case 'L':
      self->compMsgDebug->setDebugFlags(self, "BHIWsSw");
      COMP_MSG_DBG(self, "Y", 1, "start startLigthSleepWakeupMode\n");
      result = self->compMsgAction->startLightSleepWakeupMode(self);
      checkErrOK(result);
      break;
    case 'N':
      COMP_MSG_DBG(self, "N",1,  "start RunClientMode");
      result = self->compMsgSocket->netSocketRunClientMode(self);
      checkErrOK(result);
      break;
    case 'O':
      self->compMsgDebug->setDebugFlags(self, "BHIOWsSw");
      COMP_MSG_DBG(self, "O", 1, "start ota firmware update\n");
      result = self->compMsgOta->checkClientMode(self, false);
      COMP_MSG_DBG(self, "O", 1, "checkClientMode1: result: %d\n", result);
      checkErrOK(result);
      break;
    case 'o':
      self->compMsgDebug->setDebugFlags(self, "BHIOWsSw");
      COMP_MSG_DBG(self, "O", 1, "start ota spiffs update\n");
      result = self->compMsgOta->checkClientMode(self, true);
      COMP_MSG_DBG(self, "O", 1, "chechkClientMode2 result: %d\n", result);
      checkErrOK(result);
      break;
    case 'S':
      self->compMsgDebug->setDebugFlags(self, "BEHINOsSwW");
      COMP_MSG_DBG(self, "H", 1, "start RunSSDPMode");
      result = self->compMsgSocket->netSocketRunSSDPMode(self);
      checkErrOK(result);
      break;
    case 'U':
COMP_MSG_DBG(self, "Y", 0, "init type U");
      self->compMsgDebug->debugLevel = 1;
      self->compMsgDebug->addEol = false;
      self->compMsgDebug->setDebugFlags(self, "BEHINsSw");
      COMP_MSG_DBG(self, "U", 1, "start Uart input");
      id = 0;
      stopbits = PLATFORM_UART_STOPBITS_1;
      parity = PLATFORM_UART_PARITY_NONE;
      databits = 8;
      baud = BIT_RATE_115200;
      result = self->compMsgSendReceive->uartSetup(self, id, baud, databits, parity, stopbits);
      checkErrOK(result);
      break;
    case 'W':
ets_printf("<<<startAPMODE\n");
      self->compMsgDebug->setDebugFlags(self, "BdHINsSvWw");
      COMP_MSG_DBG(self, "DY", 1, "start RunAPMode\n");
      result = self->compMsgSocket->webSocketRunAPMode(self);
      checkErrOK(result);
      break;
    default:
      COMP_MSG_DBG(self, "Y", 1, "initDispatcher: funny type: %s", type);
      break;
    }
  }
#ifdef GDB_STUB
ets_printf("compMsgDispatcherInit end\n");
ets_printf("dispInit5\n");
#endif
  return COMP_MSG_ERR_OK;
}

// ================================= createDispatcher ====================================

static uint8_t createDispatcher(compMsgDispatcher_t *self, uint8_t **handle) {
  uint8_t result;

  os_sprintf(self->handle, "%s%p", DISP_HANDLE_PREFIX, self);
  COMP_MSG_DBG(self, "D", 2, "os createDispatcher: %s!\n", self->handle);
  result = addHandle(self->handle, self);
  if (result != COMP_MSG_ERR_OK) {
    deleteHandle(self);
    os_free(self);
    return result;
  }
//  resetMsgInfo(self, &self->received);
//  resetMsgInfo(self, &self->toSend);
  *handle = self->handle;
  COMP_MSG_DBG(self, "D", 2, "createDispatcher: done\n");
  return COMP_MSG_ERR_OK;
}

// ================================= newCompMsgDispatcher ====================================

compMsgDispatcher_t *newCompMsgDispatcher() {
  uint8_t result;

  if (compMsgDispatcherSingleton != NULL) {
    return compMsgDispatcherSingleton;
  }
  compMsgDispatcher_t *compMsgDispatcher = os_zalloc(sizeof(compMsgDispatcher_t));
  if (compMsgDispatcher == NULL) {
    return NULL;
  }

  // Debug needs to be initialized here for being able to do debug output !!
  compMsgDispatcher->compMsgDebug = newCompMsgDebug();
  result = compMsgDebugInit(compMsgDispatcher);
  if (result != COMP_MSG_ERR_OK) {
    return NULL;
  }

  // TypesAndNames
  compMsgDispatcher->compMsgTypesAndNames = newCompMsgTypesAndNames();

  // MsgDesc
  compMsgDispatcher->compMsgMsgDesc = newCompMsgMsgDesc();

  // Timer
  compMsgDispatcher->compMsgTimer = newCompMsgTimer();

  // Http
  compMsgDispatcher->compMsgHttp = newCompMsgHttp();

  // Action
  compMsgDispatcher->compMsgAction = newCompMsgAction();

  // WifiData
  compMsgDispatcher->compMsgWifiData = newCompMsgWifiData();

  // ModuleData
  compMsgDispatcher->compMsgModuleData = newCompMsgModuleData();

  // BildMsg
  compMsgDispatcher->compMsgBuildMsg = newCompMsgBuildMsg();

  // Identify
  compMsgDispatcher->compMsgIdentify = newCompMsgIdentify();

  // SendReceive
  compMsgDispatcher->compMsgSendReceive = newCompMsgSendReceive();

  // Socket
  compMsgDispatcher->compMsgSocket = newCompMsgSocket();

  // Util
  compMsgDispatcher->compMsgUtil = newCompMsgUtil();

  // Request
  compMsgDispatcher->compMsgRequest = newCompMsgRequest();

  // Ota
  compMsgDispatcher->compMsgOta = newCompMsgOta();

  compMsgDispatcherId++;
  compMsgDispatcher->id = compMsgDispatcherId;

  compMsgDispatcher->runningModeFlags = 0;
  compMsgDispatcher->stopAccessPoint = false;

  compMsgDispatcher->numMsgHeaders = 0;
  compMsgDispatcher->maxMsgHeaders = 0;
  compMsgDispatcher->msgHeader2MsgPtrs = NULL;

  compMsgDispatcher->numMsgKeyValueDescParts = 0;
  compMsgDispatcher->maxMsgKeyValueDescParts = 0;
  compMsgDispatcher->msgKeyValueDescParts = NULL;

  compMsgDispatcher->msgHeaderInfos.headerParts = NULL;
  compMsgDispatcher->msgHeaderInfos.numHeaderParts = 0;
  compMsgDispatcher->msgHeaderInfos.maxHeaderParts = 0;

  compMsgDispatcher->operatingMode = MODULE_OPERATING_MODE_AP;

  compMsgDispatcher->cloudMsgData = NULL;
  compMsgDispatcher->cloudMsgDataLgth = 0;
  compMsgDispatcher->cloudPayload = NULL;
  compMsgDispatcher->cloudPayloadLgth = 0;

  compMsgDispatcher->createDispatcher = &createDispatcher;
  compMsgDispatcher->initDispatcher = &initDispatcher;
  compMsgDispatcher->resetMsgInfo = &resetMsgInfo;
  compMsgDispatcher->getNewCompMsgDataPtr = &getNewCompMsgDataPtr;

  compMsgDispatcher->resetBuildMsgInfos =&resetBuildMsgInfos;

  compMsgDispatcher->getFieldType = &getFieldType;
  compMsgDispatcherSingleton = compMsgDispatcher;
  return compMsgDispatcher;
}

// ================================= freeCompMsgDispatcher ====================================

void freeCompMsgDispatcher(compMsgDispatcher_t *compMsgDispatcher) {
}
