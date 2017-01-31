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

#define COMP_DISP_HDR_DST              0x01
#define COMP_DISP_HDR_SRC              0x02
#define COMP_DISP_HDR_TOTAL_LGTH       0x04
#define COMP_DISP_HDR_GUID             0x08
#define COMP_DISP_HDR_SRC_ID           0x10
#define COMP_DISP_HDR_FILLER           0x20
#define COMP_DISP_PAYLOAD_CMD_KEY      0x40
#define COMP_DISP_PAYLOAD_CMD_LGTH     0x80
#define COMP_DISP_PAYLOAD_CRC          0x100
#define COMP_DISP_TOTAL_CRC            0x200

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
#define COMP_DISP_U0_TOTAL_CRC         0x2000
#define COMP_DISP_U8_TOTAL_CRC         0x4000
#define COMP_DISP_U16_TOTAL_CRC        0x8000

// the next value must equal the number of defines above!!
#define COMP_DISP_MAX_SEQUENCE     16

#define COMP_DISP_DESC_VALUE_IS_NUMBER (1 << 0)

#define DISP_GUID_LGTH 16
#define DISP_MAX_HDR_FILLER_LGTH 40

typedef struct headerPart {
  uint16_t hdrFromPart;
  uint16_t hdrToPart;
  uint16_t hdrTotalLgth;
  uint8_t hdrGUID[DISP_GUID_LGTH+1];
  uint16_t hdrSrcId;
  uint8_t hdrFiller[DISP_MAX_HDR_FILLER_LGTH+1];
  uint16_t hdrU16CmdKey;
  uint16_t hdrU16CmdLgth;
  uint16_t hdrU16Crc;
  uint16_t hdrU16TotalCrc;
  uint8_t hdrTargetPart;
  uint8_t hdrU8CmdKey;
  uint8_t hdrU8CmdLgth;
  uint8_t hdrU8Crc;
  uint8_t hdrU8TotalCrc;
  uint8_t hdrOffset;
  uint8_t hdrEncryption;
  uint8_t hdrHandleType;
  uint8_t hdrLgth;
  uint32_t hdrFlags;
  uint16_t fieldSequence[COMP_DISP_MAX_SEQUENCE];
} headerPart_t;

typedef struct msgHeaderInfos {
  uint32_t headerFlags;        // these are the flags for the 2nd line in the heads file!!
  uint16_t headerSequence[COMP_DISP_MAX_SEQUENCE];  // this is the sequence of the 2nd line in the heads file!!
  uint8_t headerLgth;
  size_t lgth;
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
  uint8_t GUID[DISP_GUID_LGTH];
  uint16_t srcId;
  uint8_t hdrFiller[DISP_MAX_HDR_FILLER_LGTH];
  uint16_t partsFlags;
  uint16_t u16CmdKey;
  uint8_t u8CmdLgth;
  uint16_t u16CmdLgth;
  size_t lgth;
  uint8_t encryption;
  uint8_t realLgth;
  size_t fieldOffset;
  uint8_t buf[DISP_BUF_LGTH];
  compMsgDataView_t *compMsgDataView;
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
  fieldValueCallback_t fieldSizeCallback;
} msgDescPart_t;

typedef struct msgValPart {
  uint8_t *fieldNameStr;
  uint8_t fieldNameId;
  uint8_t *fieldValueStr;    // the value or the callback for getting the value
  uint8_t *fieldKeyValueStr; // the value for a string
  uint8_t *fieldValueActionCb; // the name of a callback to run an action
  uint32_t fieldValue;       // the value for an integer
  uint8_t fieldFlags;
  uint8_t fieldValueCallbackType;
  fieldValueCallback_t fieldValueCallback;
} msgValPart_t;

typedef struct msgKeyValueDescPart {
  uint8_t *keyNameStr;
  uint16_t keyId;
  uint8_t keyType;
  uint8_t keyLgth;
} msgKeyValueDescPart_t;

typedef struct fieldsToSave {
  uint8_t *fieldNameStr;
  uint8_t fieldNameId;
  uint8_t *fieldValueStr;    // the value
  uint32_t fieldValue;       // the value for an integer
} fieldsToSave_t;

typedef struct compMsgMsgDesc compMsgMsgDesc_t;
typedef struct compMsgData compMsgData_t;
typedef struct compMsgWifiData compMsgWifiData_t;

typedef uint8_t (* openFile_t)(compMsgMsgDesc_t *self, const uint8_t *fileName, const uint8_t *fileMode);
typedef uint8_t (* closeFile_t)(compMsgMsgDesc_t *self);
typedef uint8_t (* flushFile_t)(compMsgMsgDesc_t *self);
typedef uint8_t (* readLine_t)(compMsgMsgDesc_t *self, uint8_t **buffer, uint8_t *lgth);
typedef uint8_t (* writeLine_t)(compMsgMsgDesc_t *self, const uint8_t *buffer, uint8_t lgth);
typedef uint8_t (* getIntFromLine_t)(compMsgDispatcher_t *self, uint8_t *myStr, long *ulgth, uint8_t **ep, bool *isEnd);
typedef uint8_t (* getStrFromLine_t)(compMsgDispatcher_t *self, uint8_t *myStr, uint8_t **ep, bool *isEnd);
typedef uint8_t (* getHeaderFieldsFromLine_t)(compMsgDispatcher_t *self, msgHeaderInfos_t *hdrInfos, uint8_t *myStr, uint8_t **ep, int *seqIdx);
typedef uint8_t (*readActions_t)(compMsgDispatcher_t *self, uint8_t *fileName);
typedef uint8_t (*readModuleValues_t)(compMsgDispatcher_t *self, uint8_t *fileName);
typedef uint8_t (*readWifiValues_t)(compMsgDispatcher_t *self, uint8_t *fileName);
typedef uint8_t (* readHeadersAndSetFlags_t)(compMsgDispatcher_t *self, uint8_t *fileName);
typedef uint8_t (* getHeaderFromUniqueFields_t)(compMsgDispatcher_t *self, uint16_t dst, uint16_t src, uint16_t cmdKey, headerPart_t **hdr);
typedef uint8_t (* getMsgPartsFromHeaderPart_t)(compMsgDispatcher_t *self, headerPart_t *hdr, uint8_t **handle);
typedef uint8_t (* getMsgKeyValueDescParts_t)(compMsgDispatcher_t *self, uint8_t *fileName);
typedef uint8_t (* getFieldsToSave_t)(compMsgDispatcher_t *self, uint8_t *fileName);
typedef uint8_t (* getWifiKeyValueKeys_t)(compMsgDispatcher_t *self, uint8_t *fileName);

typedef struct compMsgMsgDesc {
  uint8_t id;
  const uint8_t *fileName;
  uint8_t fileId;
  size_t fileSize;

  openFile_t openFile;
  closeFile_t closeFile;
  flushFile_t flushFile;
  readLine_t readLine;
  writeLine_t writeLine;
  getIntFromLine_t getIntFromLine;
  getStrFromLine_t getStrFromLine;
  getHeaderFieldsFromLine_t getHeaderFieldsFromLine;
  readHeadersAndSetFlags_t readHeadersAndSetFlags;
  readActions_t readActions;
  readModuleValues_t readModuleValues;
  readWifiValues_t readWifiValues;
  getHeaderFromUniqueFields_t getHeaderFromUniqueFields;
  getMsgPartsFromHeaderPart_t getMsgPartsFromHeaderPart;
  getMsgKeyValueDescParts_t getMsgKeyValueDescParts;
  getFieldsToSave_t getFieldsToSave;
  getWifiKeyValueKeys_t getWifiKeyValueKeys;

} compMsgMsgDesc_t;

uint8_t compMsgMsgDescInit(compMsgDispatcher_t *self);
compMsgMsgDesc_t *newCompMsgMsgDesc();
void freeCompMsgMsgDesc(compMsgMsgDesc_t *compMsgMsgDesc);

#endif	/* COMP_MSG_MSG_DESC_H */
