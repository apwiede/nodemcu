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
 * File:   compMsgMsgDesc.h
 * Author: Arnulf P. Wiedemann
 *
 * Created on October 1st, 2016
 */

/* composite message data descriptions handling */

#ifndef COMP_MSG_MSG_DESC_H
#define	COMP_MSG_MSG_DESC_H

#include "compMsgDataView.h"
#include "compMsgData.h"

enum compMsgDataErrorCode
{
  COMP_MSG_DESC_ERR_OK                    = 0,
  COMP_MSG_DESC_ERR_VALUE_NOT_SET         = 255,
  COMP_MSG_DESC_ERR_VALUE_OUT_OF_RANGE    = 254,
  COMP_MSG_DESC_ERR_BAD_VALUE             = 253,
  COMP_MSG_DESC_ERR_BAD_FIELD_TYPE        = 252,
  COMP_MSG_DESC_ERR_FIELD_TYPE_NOT_FOUND  = 251,
  COMP_MSG_DESC_ERR_VALUE_TOO_BIG         = 250,
  COMP_MSG_DESC_ERR_OUT_OF_MEMORY         = 249,
  COMP_MSG_DESC_ERR_OUT_OF_RANGE          = 248,
  // be carefull the values up to here
  // must correspond to the values in dataView.h !!!
  // with the names like DATA_VIEW_ERR_*

  COMP_MSG_DESC_ERR_OPEN_FILE             = 189,
  COMP_MSG_DESC_ERR_FILE_NOT_OPENED       = 188,
  COMP_MSG_DESC_ERR_FLUSH_FILE            = 187,
  COMP_MSG_DESC_ERR_WRITE_FILE            = 186,
  COMP_MSG_DESC_ERR_FUNNY_EXTRA_FIELDS    = 185,
};

#define DISP_BUF_LGTH 1024

// handle types
// A/G/R/S/W/U/N
#define COMP_DISP_SEND_TO_APP       'A'
#define COMP_DISP_RECEIVE_FROM_APP  'G'
#define COMP_DISP_SEND_TO_UART      'R'
#define COMP_DISP_RECEIVE_FROM_UART 'S'
#define COMP_DISP_TRANSFER_TO_UART  'W'
#define COMP_DISP_TRANSFER_TO_CONN  'U'
#define COMP_DISP_NOT_RELEVANT      'N'

// encryption
enum compMsgEncyptedCode
{
  COMP_DISP_IS_NOT_ENCRYPTED  = 0,
  COMP_DISP_IS_ENCRYPTED      = 1,
  COMP_DISP_U8_ENCRYPTION     = 2,
  COMP_DISP_U8_HANDLE_TYPE    = 4,
};

#define COMP_DISP_U16_DST              0x01
#define COMP_DISP_U16_SRC              0x02
#define COMP_DISP_U16_TOTAL_LGTH       0x04
#define COMP_DISP_U8_VECTOR_GUID       0x08
#define COMP_DISP_U16_SRC_ID           0x10
#define COMP_DISP_U8_VECTOR_HDR_FILLER 0x20
#define COMP_DISP_U16_CMD_KEY          0x40
#define COMP_DISP_U0_CMD_LGTH          0x80
#define COMP_DISP_U8_CMD_LGTH          0x100
#define COMP_DISP_U16_CMD_LGTH         0x200
#define COMP_DISP_U0_CRC               0x400
#define COMP_DISP_U8_CRC               0x800
#define COMP_DISP_U16_CRC              0x1000

// the next value must equal the number of defines above!!
#define COMP_DISP_MAX_SEQUENCE     13

#define COMP_DISP_DESC_VALUE_IS_NUMBER (1 << 0)

#define GUID_LGTH 16

typedef struct headerPart {
  uint16_t hdrFromPart;
  uint16_t hdrToPart;
  uint16_t hdrTotalLgth;
  uint8_t hdrGUID[GUID_LGTH];
  uint16_t hdrSrcId;
  uint8_t hdrfiller[38];
  uint16_t hdrU16CmdKey;
  uint16_t hdrU16CmdLgth;
  uint16_t hdrU16Crc;
  uint8_t hdrTargetPart;
  uint8_t hdrU8CmdKey;
  uint8_t hdrU8CmdLgth;
  uint8_t hdrU8Crc;
  uint8_t hdrOffset;
  uint8_t hdrEncryption;
  uint8_t hdrExtraLgth;
  uint8_t hdrHandleType;
  uint8_t hdrLgth;
  uint32_t hdrFlags;
  uint16_t fieldSequence[COMP_DISP_MAX_SEQUENCE];
} headerPart_t;

typedef struct msgHeaderInfos {
  uint32_t headerFlags;        // these are the flags for the 2nd line in the heads file!!
  uint16_t headerSequence[COMP_DISP_MAX_SEQUENCE];  // this is the sequence of the 2nd line in the heads file!!
  uint8_t headerLgth;
  uint8_t lgth;
  headerPart_t *headerParts;
  uint8_t numHeaderParts;
  uint8_t maxHeaderParts;
  uint8_t currPartIdx;
  uint8_t seqIdx;
  uint8_t seqIdxAfterHeader;
} msgHeaderInfos_t;

typedef struct msgParts {
  uint16_t fromPart;
  uint16_t toPart;
  uint16_t totalLgth;
  uint8_t GUID[GUID_LGTH];
  uint16_t srcId;
  uint8_t hdrFiller[38];
  uint16_t partsFlags;
  uint16_t u16CmdKey;
  uint8_t u8CmdLgth;
  uint16_t u16CmdLgth;
  uint8_t lgth;
  uint8_t encryption;
  uint8_t realLgth;
  size_t fieldOffset;
  uint8_t buf[DISP_BUF_LGTH];
} msgParts_t;

typedef struct msgDescPart {
  uint8_t *fieldNameStr;
  uint8_t fieldNameId;
  uint8_t *fieldTypeStr;
  uint8_t fieldTypeId;
  uint8_t fieldLgth;
  uint8_t fieldType;
  uint16_t fieldKey;
  uint16_t fieldSize;
  getFieldSizeCallback_t getFieldSizeCallback;
} msgDescPart_t;

typedef struct msgValPart {
  uint8_t *fieldNameStr;
  uint8_t fieldNameId;
  uint8_t *fieldValueStr;    // the value or the callback for getting the value
  uint8_t *fieldKeyValueStr; // the value for a string
  uint32_t fieldValue;       // the value for an integer
  uint8_t fieldFlags;
  getFieldSizeCallback_t getFieldValueCallback;
} msgValPart_t;

typedef struct compMsgMsgDesc compMsgMsgDesc_t;
typedef struct compMsgData compMsgData_t;

typedef uint8_t (* openFile_t)(compMsgMsgDesc_t *self, const uint8_t *fileName, const uint8_t *fileMode);
typedef uint8_t (* closeFile_t)(compMsgMsgDesc_t *self);
typedef uint8_t (* flushFile_t)(compMsgMsgDesc_t *self);
typedef uint8_t (* readLine_t)(compMsgMsgDesc_t *self, uint8_t **buffer, uint8_t *lgth);
typedef uint8_t (* writeLine_t)(compMsgMsgDesc_t *self, const uint8_t *buffer, uint8_t lgth);
typedef uint8_t (* dumpHeaderPart_t)(compMsgDispatcher_t *self, headerPart_t *hdr);
typedef uint8_t (* dumpMsgHeaderInfos_t)(compMsgDispatcher_t *self, msgHeaderInfos_t *hdrInfos);
typedef uint8_t (* getIntFromLine_t)(uint8_t *myStr, long *ulgth, uint8_t **ep, bool *isEnd);
typedef uint8_t (* getStrFromLine_t)(uint8_t *myStr, uint8_t **ep, bool *isEnd);
typedef uint8_t (* getHeaderFieldsFromLine_t)(compMsgDataView_t *dataView, msgHeaderInfos_t *hdrInfos, uint8_t *myStr, uint8_t **ep, int *seqIdx);
typedef uint8_t (*readActions_t)(compMsgDispatcher_t *self, uint8_t *fileName);
typedef uint8_t (* readHeadersAndSetFlags_t)(compMsgDispatcher_t *self, uint8_t *fileName);
typedef uint8_t (* getHeaderFromUniqueFields_t)(compMsgDispatcher_t *self, uint16_t dst, uint16_t src, uint16_t cmdKey, headerPart_t **hdr);
typedef uint8_t (* getMsgPartsFromHeaderPart_t)(compMsgDispatcher_t *self, headerPart_t *hdr, uint8_t **handle);
typedef uint8_t (* getWifiKeyValueKeys_t)(compMsgDispatcher_t *self, compMsgWifiData_t *compMsgWifiData);
typedef uint8_t (* dumpMsgDescPart_t)(compMsgDispatcher_t *self, msgDescPart_t *msgDescPart);
typedef uint8_t (* dumpMsgValPart_t)(compMsgDispatcher_t *self, msgValPart_t *msgValPart);
typedef uint8_t (* resetMsgDescParts_t)(compMsgDispatcher_t *self);
typedef uint8_t (* resetMsgValParts_t)(compMsgDispatcher_t *self);

typedef struct compMsgMsgDesc {
  uint8_t id;
  const uint8_t *fileName;
  uint8_t fileId;
  size_t fileSize;
  msgDescPart_t *msgDescParts;
  size_t numMsgDescParts;
  size_t maxMsgDescParts;
  msgValPart_t *msgValParts;
  size_t numMsgValParts;
  size_t maxMsgValParts;
  uint8_t *prepareValuesCbName;
  
  openFile_t openFile;
  closeFile_t closeFile;
  flushFile_t flushFile;
  readLine_t readLine;
  writeLine_t writeLine;
  dumpHeaderPart_t dumpHeaderPart;
  dumpMsgHeaderInfos_t dumpMsgHeaderInfos;
  getIntFromLine_t getIntFromLine;
  getStrFromLine_t getStrFromLine;
  getHeaderFieldsFromLine_t getHeaderFieldsFromLine;
  readHeadersAndSetFlags_t readHeadersAndSetFlags;
  readActions_t readActions;
  getHeaderFromUniqueFields_t getHeaderFromUniqueFields;
  dumpMsgDescPart_t dumpMsgDescPart;
  dumpMsgValPart_t dumpMsgValPart;
  resetMsgDescParts_t resetMsgDescParts;
  resetMsgValParts_t resetMsgValParts;
  getMsgPartsFromHeaderPart_t getMsgPartsFromHeaderPart;
  getWifiKeyValueKeys_t getWifiKeyValueKeys;

} compMsgMsgDesc_t;

compMsgMsgDesc_t *newCompMsgMsgDesc();
void freeCompMsgMsgDesc(compMsgMsgDesc_t *compMsgMsgDesc);

#endif	/* COMP_MSG_MSG_DESC_H */
