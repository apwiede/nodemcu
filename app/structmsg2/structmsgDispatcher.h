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
 * File:   structmsgDispatcher.h
 * Author: Arnulf P. Wiedemann
 *
 * Created on October 1st, 2016
 */

/* struct message data descriptions handling */

#include "structmsgDataView.h"
#include "structmsgDataDescriptions.h"

enum structmsgDispatcherErrorCode
{
  STRUCT_DISP_ERR_OK                    = 0,
  STRUCT_DISP_ERR_VALUE_NOT_SET         = 255,
  STRUCT_DISP_ERR_VALUE_OUT_OF_RANGE    = 254,
  STRUCT_DISP_ERR_BAD_VALUE             = 253,
  STRUCT_DISP_ERR_BAD_FIELD_TYPE        = 252,
  STRUCT_DISP_ERR_FIELD_TYPE_NOT_FOUND  = 251,
  STRUCT_DISP_ERR_VALUE_TOO_BIG         = 250,
  STRUCT_DISP_ERR_OUT_OF_MEMORY         = 249,
  STRUCT_DISP_ERR_OUT_OF_RANGE          = 248,
  // be carefull the values up to here
  // must correspond to the values in dataView.h !!!
  // with the names like DATA_VIEW_ERR_*

  STRUCT_DISP_ERR_HANDLE_NOT_FOUND      = 227,

  STRUCT_DISP_ERR_OPEN_FILE             = 189,
  STRUCT_DISP_FILE_NOT_OPENED           = 188,
  STRUCT_DISP_ERR_FLUSH_FILE            = 187,
  STRUCT_DISP_ERR_WRITE_FILE            = 186,
  STRUCT_DISP_ERR_BAD_RECEIVED_LGTH     = 185,
  STRUCT_DISP_ERR_BAD_FILE_CONTENTS     = 184,
};


#define DISP_BUF_LGTH 255

typedef struct msgParts {
  uint8_t lgth;
  uint8_t buf[DISP_BUF_LGTH];
  uint8_t fieldOffset;
  uint16_t totalLgth;
  uint16_t cmdLgth;
  uint16_t cmdKey;
  uint16_t fromPart;
  uint16_t toPart;
  uint8_t shCmdKey;
} msgParts_t;

typedef struct structmsgDispatcher structmsgDispatcher_t;

typedef uint8_t (* uartReceiveCb_t)(structmsgDispatcher_t *self, const uint8_t *buffer, uint8_t lgth);

typedef uint8_t (* createDispatcher_t)(structmsgDispatcher_t *self, uint8_t **handle);

typedef uint8_t (* openFileDesc_t)(structmsgDispatcher_t *self, const uint8_t *fileName, const uint8_t *fileMode);
typedef uint8_t (* closeFileDesc_t)(structmsgDispatcher_t *self);
typedef uint8_t (* flushFileDesc_t)(structmsgDispatcher_t *self);
typedef uint8_t (* readLineDesc_t)(structmsgDispatcher_t *self, uint8_t **buffer, uint8_t *lgth);
typedef uint8_t (* writeLineDesc_t)(structmsgDispatcher_t *self, const uint8_t *buffer, uint8_t lgth);

typedef uint8_t (* IMsg_t)(structmsgDispatcher_t *self);
typedef uint8_t (* BMsg_t)(structmsgDispatcher_t *self);
typedef uint8_t (* MMsg_t)(structmsgDispatcher_t *self);
typedef uint8_t (* defaultMsg_t)(structmsgDispatcher_t *self);
typedef uint8_t (* sendAnswer_t)(structmsgDispatcher_t *self, msgParts_t *parts, uint8_t type);
typedef uint8_t (* resetMsgInfo_t)(structmsgDispatcher_t *self, msgParts_t *parts);


typedef struct structmsgDispatcher {
  uint8_t id;
  char handle[20];
  uint8_t *FileName;
  uint8_t fileId;
  size_t fileSize;
  uint16_t flags;
  
  structmsgDataView_t *structmsgDataView;
  structmsgDataDescription_t *structmsgDataDescription;

  msgParts_t received;
  msgParts_t toSend;

  uint16_t McuPart;
  uint16_t WifiPart;
  uint16_t AppPart;
  uint16_t CloudPart;

  IMsg_t IMsg;
  BMsg_t BMsg;
  MMsg_t MMsg;
  defaultMsg_t defaultMsg;
  resetMsgInfo_t resetMsgInfo;
  sendAnswer_t sendAnswer;

  openFileDesc_t openFile;
  closeFileDesc_t closeFile;
  writeLineDesc_t writeLine;
  readLineDesc_t readLine;

  uartReceiveCb_t uartReceiveCb;
  createDispatcher_t createDispatcher;

} structmsgDispatcher_t;

structmsgDispatcher_t *newStructmsgDispatcher();
uint8_t structmsgDispatcherGetPtrFromHandle(const char *handle, structmsgDispatcher_t **structmsgDispatcher);
void freeStructmsgDispatcher(structmsgDispatcher_t *structmsgDispatcher);
