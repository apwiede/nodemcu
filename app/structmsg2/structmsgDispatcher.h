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

  STRUCT_DISP_ERR_FIELD_NOT_FOUND       = 230,
  STRUCT_DISP_ERR_HANDLE_NOT_FOUND      = 227,
  // be carefull the values up to here
  // must correspond to the values in structmsgDataView.h !!!
  // with the names like STRUCT_MSG_ERR_*

  STRUCT_DISP_ERR_OPEN_FILE             = 189,
  STRUCT_DISP_FILE_NOT_OPENED           = 188,
  STRUCT_DISP_ERR_FLUSH_FILE            = 187,
  STRUCT_DISP_ERR_WRITE_FILE            = 186,
  STRUCT_DISP_ERR_BAD_RECEIVED_LGTH     = 185,
  STRUCT_DISP_ERR_BAD_FILE_CONTENTS     = 184,
  STRUCT_DISP_ERR_HEADER_NOT_FOUND      = 183,
  STRUCT_DISP_ERR_DUPLICATE_FIELD       = 182,
};


#define DISP_BUF_LGTH 255
// dst + src + totalLgth + (optional) GUID + cmdKey/shCmdKey
// uint16_t + uint16_t + uint16_t + (optional) uint8_t*(16) + uint16_t/uint8_t
#define DISP_MAX_HEADER_LGTH (sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint16_t) + (16*sizeof(uint8_t)) + sizeof(uint16_t))

typedef struct structmsgData structmsgData_t;

#define DISP_HAS_DST        (1 << 0)
#define DISP_HAS_SRC        (1 << 1)
#define DISP_HAS_TARGET     (1 << 2)
#define DISP_HAS_TOTAL_LGTH (1 << 3)
#define DISP_HAS_CMD_LGTH   (1 << 4)
#define DISP_SHORT_CMD_KEY  (1 << 5)
#define DISP_IS_ENCRYPTED   (1 << 6)
#define DISP_HAS_EXTRA_KEY  (1 << 7)
#define DISP_SHORT_CMD_LGTH (1 << 8)

typedef struct headerParts {
  uint16_t hdrFromPart;
  uint16_t hdrToPart;
  uint16_t hdrTotalLgth;
  uint16_t hdrCmdKey;
  uint16_t hdrCmdLgth;
  uint8_t hdrTargetPart;
  uint8_t hdrShCmdKey;
  uint8_t hdrShCmdLgth;
  uint8_t hdrOffset;
  uint8_t hdrExtraLgth;
  uint8_t hdrFlags;
} headerParts_t;

typedef struct msgHeaderInfos {
  uint16_t headerFlags;
  headerParts_t *headerParts;
  uint8_t numHeaderParts;
  uint8_t maxHeaderParts;
  uint8_t headerSequence[6];
} msgHeaderInfos_t;

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

typedef struct msgHeader2MsgPtr {
  structmsgData_t *structmsgData;
  uint8_t headerLgth;
  uint8_t header[DISP_MAX_HEADER_LGTH];
} msgHeader2MsgPtr_t;

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
  uint16_t dispFlags;
  
  uint8_t numMsgHeaders;
  uint8_t maxMsgHeaders;
  msgHeader2MsgPtr_t *msgHeader2MsgPtrs;

  structmsgDataView_t *structmsgDataView;
  structmsgDataDescription_t *structmsgDataDescription;

  msgHeaderInfos_t msgHeaderInfos;
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
