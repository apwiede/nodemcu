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
 * File:   compMsgDataDescs.h
 * Author: Arnulf P. Wiedemann
 *
 * Created on October 1st, 2016
 */

/* composite message data descriptions handling */

#include "compMsgDataView.h"

enum compMsgDataErrorCode
{
  COMP_DATA_DESC_ERR_OK                    = 0,
  COMP_DATA_DESC_ERR_VALUE_NOT_SET         = 255,
  COMP_DATA_DESC_ERR_VALUE_OUT_OF_RANGE    = 254,
  COMP_DATA_DESC_ERR_BAD_VALUE             = 253,
  COMP_DATA_DESC_ERR_BAD_FIELD_TYPE        = 252,
  COMP_DATA_DESC_ERR_FIELD_TYPE_NOT_FOUND  = 251,
  COMP_DATA_DESC_ERR_VALUE_TOO_BIG         = 250,
  COMP_DATA_DESC_ERR_OUT_OF_MEMORY         = 249,
  COMP_DATA_DESC_ERR_OUT_OF_RANGE          = 248,
  // be carefull the values up to here
  // must correspond to the values in dataView.h !!!
  // with the names like DATA_VIEW_ERR_*

  COMP_DATA_DESC_ERR_OPEN_FILE             = 189,
  COMP_DATA_DESC_FILE_NOT_OPENED           = 188,
  COMP_DATA_DESC_ERR_FLUSH_FILE            = 187,
  COMP_DATA_DESC_ERR_WRITE_FILE            = 186,
};

#define DISP_BUF_LGTH 1024

// handle types
// A/G/R/S/W/U/N
#define COMP_DISP_SEND_TO_APP       (1 << 0)
#define COMP_DISP_RECEIVE_FROM_APP  (1 << 1)
#define COMP_DISP_SEND_TO_UART      (1 << 2)
#define COMP_DISP_RECEIVE_FROM_UART (1 << 3)
#define COMP_DISP_TRANSFER_TO_UART  (1 << 4)
#define COMP_DISP_TRANSFER_TO_CONN  (1 << 5)
#define COMP_DISP_NOT_RELEVANT      (1 << 6)

// encryption and other types
#define COMP_DISP_IS_ENCRYPTED      (1 << 0)
#define COMP_DISP_IS_NOT_ENCRYPTED  (1 << 1)

// 0x01
#define COMP_DISP_U16_DST           (1 << 0)
// 0x02
#define COMP_DISP_U16_SRC           (1 << 1)
// 0x04
#define COMP_DISP_U8_TARGET         (1 << 2)
// 0x08
#define COMP_DISP_U16_TOTAL_LGTH    (1 << 3)
// 0x10
#define COMP_DISP_U8_EXTRA_KEY_LGTH (1 << 4)
// 0x20
#define COMP_DISP_U8_ENCRYPTION     (1 << 5)
// 0x40
#define COMP_DISP_U8_HANDLE_TYPE    (1 << 6)
// 0x80
#define COMP_DISP_U8_CMD_KEY        (1 << 7)
// 0x100
#define COMP_DISP_U16_CMD_KEY       (1 << 8)
// 0x200
#define COMP_DISP_U0_CMD_LGTH       (1 << 9)
// 0x400
#define COMP_DISP_U8_CMD_LGTH       (1 << 10)
// 0x800
#define COMP_DISP_U16_CMD_LGTH      (1 << 11)
// 0x1000
#define COMP_DISP_U0_CRC            (1 << 12)
// 0x2000
#define COMP_DISP_U8_CRC            (1 << 13)
// 0x4000
#define COMP_DISP_U16_CRC           (1 << 14)

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
  uint16_t fieldSequence[9];
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
  uint8_t realLgth;
  size_t fieldOffset;
  uint8_t buf[DISP_BUF_LGTH];
} msgParts_t;

typedef struct compMsgDataDesc compMsgDataDesc_t;

typedef uint8_t (* getIntFromLine_t)(uint8_t *myStr, long *ulgth, uint8_t **ep);
typedef uint8_t (* getStartFieldsFromLine_t)(compMsgDataView_t *dataView, msgHeaderInfos_t *hdrInfos, uint8_t *myStr, uint8_t **ep, int *seqIdx);
typedef uint8_t (* openFile_t)(compMsgDataDesc_t *self, const uint8_t *fileName, const uint8_t *fileMode);
typedef uint8_t (* closeFile_t)(compMsgDataDesc_t *self);
typedef uint8_t (* flushFile_t)(compMsgDataDesc_t *self);
typedef uint8_t (* readLine_t)(compMsgDataDesc_t *self, uint8_t **buffer, uint8_t *lgth);
typedef uint8_t (* writeLine_t)(compMsgDataDesc_t *self, const uint8_t *buffer, uint8_t lgth);


typedef struct compMsgDataDesc {
  uint8_t id;
  const uint8_t *fileName;
  uint8_t fileId;
  size_t fileSize;
  
  getIntFromLine_t getIntFromLine;
  getStartFieldsFromLine_t getStartFieldsFromLine;
  openFile_t openFile;
  closeFile_t closeFile;
  flushFile_t flushFile;
  readLine_t readLine;
  writeLine_t writeLine;

} compMsgDataDesc_t;

compMsgDataDesc_t *newCompMsgDataDesc();
void freeCompMsgDataDesc(compMsgDataDesc_t *compMsgDataDesc);
