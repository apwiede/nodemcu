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
 * File:   compMsgBuildMsg.c
 * Author: Arnulf P. Wiedemann
 *
 * Created on October 6st, 2016
 */

#include "osapi.h"
#include "c_types.h"
#include "mem.h"
#include "flash_fs.h"

#include "c_string.h"
#include "c_stdlib.h"
#include "c_stdio.h"
#include "platform.h"
#include "compMsgDispatcher.h"


// ================================= setMsgValues ====================================

static uint8_t setMsgValues(compMsgDispatcher_t *self) {
  int result;
  uint8_t numEntries;
  uint8_t fileName[30];
  uint8_t lgth;
  uint8_t buf[100];
  uint8_t *buffer = buf;
  unsigned long ulgth;
  char *endPtr;

  os_sprintf(fileName, "Val%c%c%c.txt", (self->buildMsgInfos.u16CmdKey>>8)&0xFF, self->buildMsgInfos.u16CmdKey&0xFF, self->buildMsgInfos.type);
  result = self->compMsgMsgDesc->openFile(self->compMsgMsgDesc, fileName, "r");
  checkErrOK(result);
  result = self->compMsgMsgDesc->readLine(self->compMsgMsgDesc, &buffer, &lgth);
  checkErrOK(result);
  if ((lgth < 4) || (buffer[0] != '#')) {
    return COMP_DISP_ERR_BAD_FILE_CONTENTS;
  }
  ulgth = c_strtoul(buffer+2, &endPtr, 10);
  numEntries = (uint8_t)ulgth;
//ets_printf("§@setMsgValues numEntries!%d!@§\n", numEntries);
  result = self->setMsgValuesFromLines(self, self->compMsgData, numEntries, self->msgHandle, self->buildMsgInfos.type);
  checkErrOK(result);
  result = self->compMsgMsgDesc->closeFile(self->compMsgMsgDesc);
  return result;
}

// ================================= buildListMsg ====================================

static uint8_t buildListMsg(compMsgDispatcher_t *self, size_t *totalLgth, uint8_t **totalData) {
  int result;
  char *cp;
  char *cp2;
  int lgth;
  size_t listMsgHeaderLgth;
  size_t msgsLgth;
  compMsgData_t *compMsgData;

  result = COMP_MSG_ERR_OK;

  compMsgData = self->compMsgData;
  compMsgData->listSrc = self->buildListMsgInfos.src; 
  compMsgData->listDst = self->buildListMsgInfos.dst; 
  compMsgData->numListMsgs = 2;
  compMsgData->listMsgSizes = os_zalloc(compMsgData->numListMsgs * sizeof(uint16_t));
  checkAllocOK(compMsgData->listMsgSizes);
  msgsLgth = 0;
  compMsgData->listMsgSizes[0] = self->buildListMsgInfos.defHeaderLgth + self->buildListMsgInfos.encryptedDefDataLgth;
  msgsLgth += compMsgData->listMsgSizes[0];
  compMsgData->listMsgSizes[1] = self->buildListMsgInfos.msgHeaderLgth + self->buildListMsgInfos.encryptedMsgDataLgth;
  msgsLgth += compMsgData->listMsgSizes[1];
  compMsgData->listMsgs = os_zalloc(msgsLgth);
  checkAllocOK(compMsgData->listMsgSizes);
  cp = (char *)compMsgData->listMsgs;
  cp2 = cp;
  c_memcpy(cp, self->buildListMsgInfos.defData, self->buildListMsgInfos.defHeaderLgth);
  cp += self->buildListMsgInfos.defHeaderLgth;
  c_memcpy(cp, self->buildListMsgInfos.encryptedDefData, self->buildListMsgInfos.encryptedDefDataLgth);
  cp += self->buildListMsgInfos.encryptedDefDataLgth;
  c_memcpy(cp, self->buildListMsgInfos.msgData, self->buildListMsgInfos.msgHeaderLgth);
  cp += self->buildListMsgInfos.msgHeaderLgth;
  c_memcpy(cp, self->buildListMsgInfos.encryptedMsgData, self->buildListMsgInfos.encryptedMsgDataLgth);
  cp += self->buildListMsgInfos.encryptedMsgDataLgth;
  compMsgData->listMsgsSize = cp - cp2;

#ifdef NOTDEF
  listMsgHeaderLgth = 6;
  // FIXME eventually need to add extra length to second headerLgth!!
  *totalLgth = listMsgHeaderLgth;
  *totalLgth += self->buildListMsgInfos.defHeaderLgth + self->buildListMsgInfos.encryptedDefDataLgth;
  *totalLgth += self->buildListMsgInfos.msgHeaderLgth + self->buildListMsgInfos.encryptedMsgDataLgth;
ets_printf("lmhl: %d defhl: %d eddl: %d\n", listMsgHeaderLgth, self->buildListMsgInfos.defHeaderLgth, self->buildListMsgInfos.encryptedDefDataLgth);
ets_printf("msghl: %d emdl: %d tl: %d\n", self->buildListMsgInfos.msgHeaderLgth, self->buildListMsgInfos.encryptedMsgDataLgth, *totalLgth);
  *totalData = os_zalloc(*totalLgth);
ets_printf("totalLgth: %d 0x%04x, encryptedDefDataLgth: %d 0x%04x stotal: %d 0x%04x\n", *totalLgth, *totalLgth, self->buildListMsgInfos.encryptedDefDataLgth, self->buildListMsgInfos.encryptedDefDataLgth, self->compMsgData->totalLgth, self->compMsgData->totalLgth);
  cp = (char *)(*totalData);
cp2 = cp;
  cp[0] = ((*totalLgth) >> 8) & 0xFF;
  cp[1] = (*totalLgth) & 0xFF;
  cp[2] = ((self->buildListMsgInfos.defHeaderLgth + self->buildListMsgInfos.encryptedDefDataLgth) >> 8) & 0xFF;
  cp[3] = (self->buildListMsgInfos.defHeaderLgth + self->buildListMsgInfos.encryptedDefDataLgth) & 0xFF;
  cp[4] = ((self->buildListMsgInfos.msgHeaderLgth + self->buildListMsgInfos.encryptedMsgDataLgth) >> 8) & 0xFF;
  cp[5] = (self->buildListMsgInfos.msgHeaderLgth + self->buildListMsgInfos.encryptedMsgDataLgth) & 0xFF;
ets_printf("cp: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n", cp[0], cp[1], cp[2], cp[3], cp[4], cp[5]);
#endif
ets_printf("ready to send Msg cp: %p %p %d\n", cp, cp2, cp-cp2);
  result = self->compMsgData->initListMsg(self->compMsgData);
  checkErrOK(result);
  // and now set the values
  result = self->compMsgData->setListFieldValue(self->compMsgData, COMP_MSG_SPEC_FIELD_NUM_LIST_MSGS, compMsgData->numListMsgs, NULL, 0);
  checkErrOK(result);
  result = self->compMsgData->setListFieldValue(self->compMsgData, COMP_MSG_SPEC_FIELD_LIST_MSG_SIZES, compMsgData->listMsgSizes[0], NULL, 0);
  checkErrOK(result);
  result = self->compMsgData->setListFieldValue(self->compMsgData, COMP_MSG_SPEC_FIELD_LIST_MSG_SIZES, compMsgData->listMsgSizes[1], NULL, 1);
  checkErrOK(result);
  os_free(compMsgData->listMsgSizes);
  result = self->compMsgData->setListFieldValue(self->compMsgData, COMP_MSG_SPEC_FIELD_LIST_MSGS, 0, compMsgData->listMsgs, 0);
  checkErrOK(result);
  result = self->compMsgData->prepareListMsg(self->compMsgData);
  checkErrOK(result);
self->compMsgData->dumpListFields(self->compMsgData);
  result = self->compMsgData->getListData(self->compMsgData, totalData, totalLgth);
  checkErrOK(result);
  return result;
}

// ================================= buildMsg ====================================

static uint8_t buildMsg(compMsgDispatcher_t *self) {
  uint8_t result;
  size_t msgLgth;
  uint8_t *msgData;
  size_t defLgth;
  uint8_t *defData;
  uint8_t *cryptKey;
  uint8_t klen;
  uint8_t ivlen;
  uint8_t *stringValue;
  int src;
  int dst;

  self->buildMsgInfos.numRows = self->bssScanInfos->numScanInfos;
  result = self->createMsgFromLines(self, self->buildMsgInfos.parts, self->buildMsgInfos.numEntries, self->buildMsgInfos.numRows, self->buildMsgInfos.type);
  checkErrOK(result);
  result = self->compMsgMsgDesc->closeFile(self->compMsgMsgDesc);
  checkErrOK(result);
  result = self->compMsgData->initMsg(self->compMsgData);
//ets_printf("heap2: %d\n", system_get_free_heap_size());
  result = setMsgValues(self);
  checkErrOK(result);
  result = self->compMsgData->getFieldValue(self->compMsgData, "@dst", &dst, &stringValue);
  checkErrOK(result);
  self->buildListMsgInfos.dst = dst;
  result = self->compMsgData->getFieldValue(self->compMsgData, "@src", &src, &stringValue);
  checkErrOK(result);
  self->buildListMsgInfos.src = src;
  result = self->compMsgData->getMsgData(self->compMsgData, &msgData, &msgLgth);
ets_printf("getMsgData result: %d msgLgth: %d msgData: %s!\n", result, msgLgth, msgData);
  checkErrOK(result);

  // FIXME !! have to check if extraLgth is included!!
  self->buildListMsgInfos.msgHeaderLgth = self->compMsgData->headerLgth;
  self->buildListMsgInfos.msgDataLgth = msgLgth;
  self->buildListMsgInfos.msgData = msgData;
  self->buildListMsgInfos.encryptedMsgDataLgth = 0;
  self->buildListMsgInfos.encryptedMsgData = NULL;

  self->buildListMsgInfos.defHeaderLgth = 0;
  self->buildListMsgInfos.defDataLgth = 0;
  self->buildListMsgInfos.defData = NULL;
  self->buildListMsgInfos.encryptedDefDataLgth = 0;
  self->buildListMsgInfos.encryptedDefData = NULL;

  cryptKey = "a1b2c3d4e5f6g7h8";
  ivlen = 16;
  klen = 16;

  if (self->buildMsgInfos.partsFlags & COMP_DISP_IS_ENCRYPTED) {
      uint8_t *toCryptPtr;
      uint8_t *encryptedMsgData;
      size_t encryptedMsgDataLgth;
      uint16_t mlen;
      uint8_t headerLgth;

ets_printf("need to encrypt message!%s!\n", msgData);
    headerLgth = self->compMsgData->headerLgth;
    mlen = self->compMsgData->totalLgth - headerLgth;
ets_printf("msglen!%d!mlen: %d, headerLgth!%d\n", self->compMsgData->totalLgth, mlen, self->compMsgData->headerLgth);
    toCryptPtr = msgData + self->compMsgData->headerLgth;
    result = self->encryptMsg(toCryptPtr, mlen, cryptKey, klen, cryptKey, ivlen, &encryptedMsgData, &encryptedMsgDataLgth);
    checkErrOK(result);
    c_memcpy(toCryptPtr, encryptedMsgData, encryptedMsgDataLgth);
ets_printf("crypted: len: %d!%s!\n", encryptedMsgDataLgth, msgData);
    
    self->buildListMsgInfos.encryptedMsgDataLgth = encryptedMsgDataLgth;
    self->buildListMsgInfos.encryptedMsgData = encryptedMsgData;
  }

  if (self->buildMsgInfos.numRows > 0) {
    // we have a definition and the message
    result = self->compMsgData->prepareDefMsg(self->compMsgData);
    checkErrOK(result);
    result = self->compMsgData->getDefData(self->compMsgData, &defData, &defLgth);
ets_printf("getDef result: %d defLgth: %d defData: %s!\n", result, defLgth, defData);
    checkErrOK(result);
    self->buildListMsgInfos.defHeaderLgth = self->compMsgData->defHeaderLgth;
    self->buildListMsgInfos.defDataLgth = defLgth;
    self->buildListMsgInfos.defData = defData;

    if (self->buildMsgInfos.partsFlags & COMP_DISP_IS_ENCRYPTED) {
ets_printf("need to encrypt message!%s!numRows!%d!\n", msgData, self->buildMsgInfos.numRows);
      uint8_t *defToCryptPtr;
      size_t deflen;
      uint8_t *encryptedDefData;
      size_t encryptedDefDataLgth;
      size_t totalLgth;
      uint8_t *totalData;

      self->buildListMsgInfos.defData = defData;
      deflen = defLgth - self->compMsgData->headerLgth;
ets_printf("defLgth!%d!deflen: %d, headerLgth!%d\n", defLgth, deflen, self->compMsgData->headerLgth);
      defToCryptPtr = defData + self->compMsgData->headerLgth;
      result = self->encryptMsg(defToCryptPtr, deflen, cryptKey, klen, cryptKey, ivlen, &encryptedDefData, &encryptedDefDataLgth);
      checkErrOK(result);
      c_memcpy(defToCryptPtr, encryptedDefData, encryptedDefDataLgth);
ets_printf("defEncrypted: len: %d!%s!\n", encryptedDefDataLgth, defData);
        
      self->buildListMsgInfos.encryptedDefDataLgth = encryptedDefDataLgth;
      self->buildListMsgInfos.encryptedDefData = encryptedDefData;

      result = self->buildListMsg(self, &totalLgth, &totalData);
      checkErrOK(result);
      // FIXME!! need compMsgModulData callback function for sending here !!!
      result = self->websocketSendData(self->wud, totalData, totalLgth, OPCODE_BINARY);
ets_printf("Msg sent\n");
//  os_free(totalData);
    } else {
      // nested message not encrypted
      // FIXME !! need code here
    }
  } else {
    // normal message
    if (self->buildMsgInfos.partsFlags & COMP_DISP_IS_ENCRYPTED) {
      // FIXME !! need code here
    } else {
      // FIXME !! need code here
    }
  }
//  result = self->resetMsgInfo(self, self->buildMsgInfos.parts);
  return result;
}

// ================================= prepareAnswerMsg ====================================

static uint8_t prepareAnswerMsg(compMsgDispatcher_t *self, msgParts_t *parts, uint8_t type) {
  uint8_t fileName[30];
  int result;
  uint8_t numEntries;
  unsigned long ulgth;
  char *endPtr;
  uint8_t *cp;
  uint8_t lgth;
  uint8_t buf[100];
  uint8_t *buffer = buf;
  uint8_t numRows;
  uint8_t *handle;
  compMsgData_t *compMsgData;
  uint8_t *prepareValuesCbName;
  uint8_t actionMode;
  int idx;
  uint8_t *data;
  int msgLgth;

//ets_printf("§@prepareAnswerMsg u16!%c%c!t!%c!@§\n", (parts->u16CmdKey>>8)& 0xFF, parts->u16CmdKey&0xFF, type);
  os_sprintf(fileName, "Desc%c%c%c.txt", (parts->u16CmdKey>>8)& 0xFF, parts->u16CmdKey&0xFF, type);
ets_printf("fileName: %s\n", fileName);
  result = self->compMsgMsgDesc->openFile(self->compMsgMsgDesc, fileName, "r");
  checkErrOK(result);
  result = self->compMsgMsgDesc->readLine(self->compMsgMsgDesc, &buffer, &lgth);
  checkErrOK(result);
  if ((lgth < 4) || (buffer[0] != '#')) {
    return COMP_DISP_ERR_BAD_FILE_CONTENTS;
  }
  ulgth = c_strtoul(buffer+2, &endPtr, 10);
  numEntries = (uint8_t)ulgth;
  cp = (uint8_t *)endPtr;
  prepareValuesCbName = NULL;
  if ((*cp == ',') || (*cp == '\n') || (*cp == '\r') || (*cp == '\0')) {
    *cp = '\0';
    cp++;
    prepareValuesCbName = cp;
  }
  if (prepareValuesCbName != NULL) {
    while ((*cp != '\n') &&  (*cp != '\r') && (*cp != '\0')) {
      cp++;
    }
    *cp = '\0';
  }
  self->buildMsgInfos.numEntries = numEntries;
  self->buildMsgInfos.type = type;
  self->buildMsgInfos.parts = parts;
  self->buildMsgInfos.u16CmdKey = parts->u16CmdKey;
  self->buildMsgInfos.partsFlags = parts->partsFlags;
  self->buildMsgInfos.numRows = numRows;
  if (prepareValuesCbName != NULL) {
    result = self->getActionMode(self, prepareValuesCbName+1, &actionMode);
    self->actionMode = actionMode;
    checkErrOK(result);
    result  = self->runAction(self, &type);
    return result;
  } else {
    result = self->buildMsg(self);
    result = setMsgValues(self);
    checkErrOK(result);
//ets_printf("§heap3: %d§", system_get_free_heap_size());
    result = self->compMsgData->getMsgData(self->compMsgData, &data, &msgLgth);
    checkErrOK(result);
    result = self->typeRSendAnswer(self, data, msgLgth);
    return result;
  }
}

// ================================= prepareNotEncryptedAnswer ====================================

static uint8_t prepareNotEncryptedAnswer(compMsgDispatcher_t *self, msgParts_t *parts, uint8_t type) {
  int result;
  uint8_t fileName[30];
  uint8_t numEntries;
  char *endPtr;
  unsigned long ulgth;
  uint8_t lgth;
  uint8_t buf[100];
  uint8_t *buffer = buf;
  uint8_t *data;
  int msgLgth;

  result = prepareAnswerMsg(self, parts, type);
  checkErrOK(result);
  self->buildMsgInfos.type = type;
  self->buildMsgInfos.parts = parts;
  result = setMsgValues(self);
  checkErrOK(result);
//ets_printf("§heap3: %d§", system_get_free_heap_size());
  result = self->compMsgData->getMsgData(self->compMsgData, &data, &msgLgth);
  checkErrOK(result);
  result = self->typeRSendAnswer(self, data, msgLgth);
  return result;
}

// ================================= prepareEncryptedAnswer ====================================

static uint8_t prepareEncryptedAnswer(compMsgDispatcher_t *self, msgParts_t *parts, uint8_t type) {
  uint8_t fileName[30];
  int result;
  uint8_t numEntries;
  char *endPtr;
  uint8_t lgth;
  int defLgth;
  uint8_t *defData;
  int msgLgth;
  uint8_t *data;
  uint8_t buf[100];
  uint8_t *buffer = buf;
  uint8_t numRows;
  uint8_t *handle;
  unsigned long ulgth;
  compMsgData_t *compMsgData;
  int idx;

  result = prepareAnswerMsg(self, parts, type);
  checkErrOK(result);
//FIXME  TEMPORARY!!!
return COMP_MSG_ERR_OK;
  os_sprintf(fileName, "Val%c%c%c.txt", (parts->u16CmdKey>>8)&0xFF, parts->u16CmdKey&0xFF, type);
  result = self->compMsgMsgDesc->openFile(self->compMsgMsgDesc, fileName, "r");
  checkErrOK(result);
  result = self->compMsgMsgDesc->readLine(self->compMsgMsgDesc, &buffer, &lgth);
  checkErrOK(result);
  if ((lgth < 4) || (buffer[0] != '#')) {
    return COMP_DISP_ERR_BAD_FILE_CONTENTS;
  }
  ulgth = c_strtoul(buffer+2, &endPtr, 10);
  numEntries = (uint8_t)ulgth;
//ets_printf("§@NE2!%d!@§", numEntries);
  result = self->setMsgValuesFromLines(self, compMsgData, numEntries, handle, type);
  checkErrOK(result);
  result = self->compMsgMsgDesc->closeFile(self->compMsgMsgDesc);
  checkErrOK(result);
//ets_printf("§heap3: %d§", system_get_free_heap_size());
  result = compMsgData->getMsgData(compMsgData, &data, &msgLgth);
  checkErrOK(result);

ets_printf("prepareDefMsg\n");
result = self->compMsgData->prepareDefMsg(self->compMsgData);
ets_printf("prepareDefMsg result: %d\n", result);
checkErrOK(result);
result = self->compMsgData->getDefData(self->compMsgData, &defData, &defLgth);
ets_printf("defLgth: %d defData: %s!\n", defLgth, defData);
checkErrOK(result);
self->compMsgDataView->dataView->dumpBinary(defData, defLgth, "defData");

  result = self->typeRSendAnswer(self, data, msgLgth);
  self->resetMsgInfo(self, parts);
  return COMP_DISP_ERR_OK;
}

// ================================= compMsgBuildMsgInit ====================================

uint8_t compMsgBuildMsgInit(compMsgDispatcher_t *self) {
  self->buildMsg = &buildMsg;
  self->buildListMsg = &buildListMsg;
  self->setMsgValues = &setMsgValues;
  self->prepareAnswerMsg = &prepareAnswerMsg;
  self->prepareEncryptedAnswer = &prepareEncryptedAnswer;
  return COMP_DISP_ERR_OK;
}
