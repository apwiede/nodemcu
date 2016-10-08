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
  STRUCT_DISP_ERR_BAD_FIELD_NAME        = 181,
  STRUCT_DISP_ERR_BAD_HANDLE_TYPE       = 180,
  STRUCT_DISP_ERR_INVALID_BASE64_STRING = 179,
  STRUCT_DISP_ERR_TOO_FEW_FILE_LINES    = 178,
  STRUCT_DISP_ERR_ACTION_NAME_NOT_FOUND = 177,
  STRUCT_DISP_ERR_DUPLICATE_ENTRY       = 176,
};


#define DISP_BUF_LGTH 255
// dst + src + totalLgth + (optional) GUID + cmdKey/shCmdKey
// uint16_t + uint16_t + uint16_t + (optional) uint8_t*(16) + uint16_t/uint8_t
#define DISP_MAX_HEADER_LGTH (sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint16_t) + (16*sizeof(uint8_t)) + sizeof(uint16_t))

typedef struct structmsgData structmsgData_t;

// handle types
// A/G/R/S/W/U/N
#define STRUCT_DISP_SEND_TO_APP       (1 << 0)
#define STRUCT_DISP_RECEIVE_FROM_APP  (1 << 1)
#define STRUCT_DISP_SEND_TO_UART      (1 << 2)
#define STRUCT_DISP_RECEIVE_FROM_UART (1 << 3)
#define STRUCT_DISP_TRANSFER_TO_UART  (1 << 4)
#define STRUCT_DISP_TRANSFER_TO_CONN  (1 << 5)
#define STRUCT_DISP_NOT_RELEVANT      (1 << 6)

// encryption types
#define STRUCT_DISP_IS_ENCRYPTED      (1 << 0)
#define STRUCT_DISP_IS_NOT_ENCRYPTED  (1 << 1)


#define STRUCT_DISP_U16_DST           (1 << 0)
#define STRUCT_DISP_U16_SRC           (1 << 1)
#define STRUCT_DISP_U8_TARGET         (1 << 2)
#define STRUCT_DISP_U16_TOTAL_LGTH    (1 << 3)
#define STRUCT_DISP_U8_EXTRA_KEY_LGTH (1 << 4)
#define STRUCT_DISP_U8_ENCRYPTION     (1 << 5)
#define STRUCT_DISP_U8_HANDLE_TYPE    (1 << 6)
#define STRUCT_DISP_U8_CMD_KEY        (1 << 7)
#define STRUCT_DISP_U16_CMD_KEY       (1 << 8)
#define STRUCT_DISP_U0_CMD_LGTH       (1 << 9)
#define STRUCT_DISP_U8_CMD_LGTH       (1 << 10)
#define STRUCT_DISP_U16_CMD_LGTH      (1 << 11)
#define STRUCT_DISP_U0_CRC            (1 << 12)
#define STRUCT_DISP_U8_CRC            (1 << 13)
#define STRUCT_DISP_U16_CRC           (1 << 14)

typedef struct headerParts {
  uint16_t hdrFromPart;
  uint16_t hdrToPart;
  uint16_t hdrTotalLgth;
  uint16_t hdrU16CmdKey;
  uint16_t hdrU16CmdLgth;
  uint16_t hdrU16Crc;
  uint8_t hdrTargetPart;
  uint8_t hdrU8CmdKey;
  uint8_t hdrU8CmdLgth;
  uint8_t hdrU8Crc;
  uint8_t hdrOffset;
  uint8_t hdrExtraLgth;
  uint8_t hdrEncryption;
  uint8_t hdrHandleType;
  uint32_t hdrFlags;
} headerParts_t;

typedef struct msgHeaderInfos {
  uint32_t headerFlags;
  headerParts_t *headerParts;
  uint16_t headerSequence[9];
  uint8_t headerStartLgth;
  uint8_t numHeaderParts;
  uint8_t maxHeaderParts;
  uint8_t currPartIdx;
  uint8_t seqIdx;
  uint8_t seqIdxAfterStart;
} msgHeaderInfos_t;

typedef struct msgParts {
  uint16_t fromPart;
  uint16_t toPart;
  uint16_t totalLgth;
  uint16_t u16CmdLgth;
  uint16_t u16CmdKey;
  uint16_t partsFlags;
  uint8_t targetPart;
  uint8_t u8CmdLgth;
  uint8_t u8CmdKey;
  uint8_t lgth;
  uint8_t fieldOffset;
  uint8_t buf[DISP_BUF_LGTH];
} msgParts_t;

typedef struct msgHeader2MsgPtr {
  structmsgData_t *structmsgData;
  uint8_t headerLgth;
  uint8_t header[DISP_MAX_HEADER_LGTH];
} msgHeader2MsgPtr_t;

typedef struct structmsgDispatcher structmsgDispatcher_t;

typedef uint8_t (* setActionEntry_t)(structmsgDispatcher_t *self, uint8_t *actionName, uint8_t mode, uint8_t u8CmdKey, uint16_t u16CmdKey);
typedef uint8_t (* runAction_t)(structmsgDispatcher_t *self, uint8_t *answerType);
typedef uint8_t (* fillMsgValue_t)(structmsgDispatcher_t *self, uint8_t *callbackName, int *numericValue, uint8_t **stringValue, uint8_t answerType, uint8_t fieldTypeId);

typedef uint8_t (* uartReceiveCb_t)(structmsgDispatcher_t *self, const uint8_t *buffer, uint8_t lgth);

typedef uint8_t (* createDispatcher_t)(structmsgDispatcher_t *self, uint8_t **handle);
typedef uint8_t (* initDispatcher_t)(structmsgDispatcher_t *self);
typedef uint8_t (* createMsgFromLines_t)(structmsgDispatcher_t *self, msgParts_t *parts, uint8_t numEntries, uint8_t numRows, uint8_t type, structmsgData_t **structmsgData, uint8_t **handle);
typedef uint8_t (* setMsgValuesFromLines_t)(structmsgDispatcher_t *self, structmsgData_t *structmsgData, uint8_t numEntries, uint8_t *handle, uint8_t type);

typedef uint8_t (* openFileDesc_t)(structmsgDispatcher_t *self, const uint8_t *fileName, const uint8_t *fileMode);
typedef uint8_t (* closeFileDesc_t)(structmsgDispatcher_t *self);
typedef uint8_t (* flushFileDesc_t)(structmsgDispatcher_t *self);
typedef uint8_t (* readLineDesc_t)(structmsgDispatcher_t *self, uint8_t **buffer, uint8_t *lgth);
typedef uint8_t (* writeLineDesc_t)(structmsgDispatcher_t *self, const uint8_t *buffer, uint8_t lgth);

typedef uint8_t (* IMsg_t)(structmsgDispatcher_t *self);
typedef uint8_t (* BMsg_t)(structmsgDispatcher_t *self);
typedef uint8_t (* MMsg_t)(structmsgDispatcher_t *self);
typedef uint8_t (* defaultMsg_t)(structmsgDispatcher_t *self);
typedef uint8_t (* prepareNotEncryptedAnswer_t)(structmsgDispatcher_t *self, msgParts_t *parts, uint8_t type);
typedef uint8_t (* resetMsgInfo_t)(structmsgDispatcher_t *self, msgParts_t *parts);

typedef uint8_t (* typeRSendAnswer_t)(structmsgDispatcher_t *self, uint8_t *data, uint8_t msgLgth);

typedef uint8_t (* readHeadersAndSetFlags_t)(structmsgDispatcher_t *self);
typedef uint8_t (* handleReceivedPart_t)(structmsgDispatcher_t *self, const uint8_t * buffer, uint8_t lgth);

typedef uint8_t (* encryptMsg_t)(const uint8_t *msg, size_t mlen, const uint8_t *key, size_t klen, const uint8_t *iv, size_t ivlen, uint8_t **buf, int *lgth);
typedef uint8_t (* decryptMsg_t)(const uint8_t *msg, size_t mlen, const uint8_t *key, size_t klen, const uint8_t *iv, size_t ivlen, uint8_t **buf, int *lgth);
typedef uint8_t (* toBase64_t)(const uint8_t *msg, size_t *len, uint8_t **encoded);
typedef uint8_t (* fromBase64_t)(const uint8_t *encodedMsg, size_t *len, uint8_t **decodedMsg);

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

  prepareNotEncryptedAnswer_t prepareNotEncryptedAnswer;
  typeRSendAnswer_t typeRSendAnswer;

  setActionEntry_t setActionEntry;
  runAction_t runAction;
  fillMsgValue_t fillMsgValue;

  openFileDesc_t openFile;
  closeFileDesc_t closeFile;
  writeLineDesc_t writeLine;
  readLineDesc_t readLine;

  readHeadersAndSetFlags_t readHeadersAndSetFlags;
  handleReceivedPart_t handleReceivedPart;
  uartReceiveCb_t uartReceiveCb;
  createDispatcher_t createDispatcher;
  initDispatcher_t initDispatcher;
  createMsgFromLines_t createMsgFromLines;
  setMsgValuesFromLines_t setMsgValuesFromLines;

  encryptMsg_t encryptMsg;
  decryptMsg_t decryptMsg;
  toBase64_t toBase64;
  fromBase64_t fromBase64;

} structmsgDispatcher_t;

structmsgDispatcher_t *newStructmsgDispatcher();
uint8_t structmsgDispatcherGetPtrFromHandle(const char *handle, structmsgDispatcher_t **structmsgDispatcher);
void freeStructmsgDispatcher(structmsgDispatcher_t *structmsgDispatcher);
uint8_t structmsgIdentifyInit(structmsgDispatcher_t *self);
uint8_t structmsgSendReceiveInit(structmsgDispatcher_t *self);
uint8_t structmsgActionInit(structmsgDispatcher_t *self);
