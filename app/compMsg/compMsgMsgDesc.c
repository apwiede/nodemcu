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
 * File:   compMsgMsgDesc.c
 * Author: Arnulf P. Wiedemann
 *
 * Created on October 1st, 2016
 */

#include "osapi.h"
#include "c_types.h"
#include "mem.h"
#include "flash_fs.h"

#include "c_string.h"
#include "c_stdio.h"
#include "c_stdlib.h"
#include "compMsgDispatcher.h"


static int compMsgMsgDescId = 0;
static volatile int fileFd = FS_OPEN_OK - 1;

// ================================= openFile ====================================

static uint8_t openFile(compMsgMsgDesc_t *self, const uint8_t *fileName, const uint8_t *fileMode) {
  self->fileName = fileName;
  fileFd = fs_open(fileName, fs_mode2flag(fileMode));
  if (fileFd < FS_OPEN_OK) {
    return COMP_MSG_DESC_ERR_OPEN_FILE;
  }
  return COMP_MSG_DESC_ERR_OK;
}

// ================================= closeFile ====================================

static uint8_t closeFile(compMsgMsgDesc_t *self) {
  if (fileFd != (FS_OPEN_OK - 1)){
    self->fileName = NULL;
    fs_close(fileFd);
    fileFd = FS_OPEN_OK - 1;
  }
  return COMP_MSG_DESC_ERR_OK;
}

// ================================= flushFile ====================================

static uint8_t flushFile(compMsgMsgDesc_t *self) {
  if (fileFd == (FS_OPEN_OK - 1)) {
    return COMP_MSG_DESC_ERR_FILE_NOT_OPENED;
  }
  if (fs_flush(fileFd) == 0) {
    return COMP_MSG_DESC_ERR_OK;
  }
  return COMP_MSG_DESC_ERR_FLUSH_FILE;
}

// ================================= readLine ====================================

static uint8_t readLine(compMsgMsgDesc_t *self, uint8_t **buffer, uint8_t *lgth) {
  size_t n = BUFSIZ;
  char buf[BUFSIZ];
  int i;
  uint8_t *cp;
  uint8_t end_char = '\n';

  if (fileFd == (FS_OPEN_OK - 1)) {
    return COMP_MSG_DESC_ERR_FILE_NOT_OPENED;
  }
  n = fs_read(fileFd, buf, n);
  cp = *buffer;
  *lgth = 0;
  for (i = 0; i < n; ++i) {
    cp[i] = buf[i];
    if (buf[i] == end_char) {
      ++i;
      break;
    }
  }
  cp[i] = 0;
  *lgth = i;
  fs_seek (fileFd, -(n - i), SEEK_CUR);
  return COMP_MSG_DESC_ERR_OK;
}

// ================================= writeLine ====================================

static uint8_t writeLine(compMsgMsgDesc_t *self, const uint8_t *buffer, uint8_t lgth) {
  int result;

  if (fileFd == (FS_OPEN_OK - 1)) {
    return COMP_MSG_DESC_ERR_FILE_NOT_OPENED;
  }
  result = fs_write(fileFd, buffer, lgth);
  if (result == lgth) {
    return COMP_MSG_DESC_ERR_OK;
  }
  return COMP_MSG_DESC_ERR_WRITE_FILE;
}

// ================================= dumpHeaderPart ====================================

static uint8_t dumpHeaderPart(compMsgDispatcher_t *self, headerPart_t *hdr) {
  int idx;

  ets_printf("§dumpHeaderPart:§");
  ets_printf("§headerPart: from: 0x%04x to: 0x%04x totalLgth: %d§", hdr->hdrFromPart, hdr->hdrToPart, hdr->hdrTotalLgth);
  ets_printf("§            GUID: %s srcId: %d u16CmdKey: 0x%04x %c%c§", hdr->hdrGUID, hdr->hdrSrcId, hdr->hdrU16CmdKey, (hdr->hdrU16CmdKey>>8)&0xFF, hdr->hdrU16CmdKey&0xFF);
  ets_printf("§            u16CmdLgth: 0x%04x u16Crc: 0x%04x u16TotalCrc: 0x%04x§", hdr->hdrU16CmdLgth, hdr->hdrU16Crc, hdr->hdrU16TotalCrc);
  ets_printf("§            u8CmdLgth: %d u8Crc: 0x%02x u8TotalCrc: 0x%02x§", hdr->hdrU8CmdLgth, hdr->hdrU8Crc, hdr->hdrU8TotalCrc);
  ets_printf("§            enc: %c handleType: %c offset: %d§", hdr->hdrEncryption, hdr->hdrHandleType, hdr->hdrOffset);
  ets_printf("§hdrFlags: 0x%04x", hdr->hdrFlags);
  if (hdr->hdrFlags & COMP_DISP_HDR_DST) {
    ets_printf(" COMP_DISP_HDR_DST");
  }
  if (hdr->hdrFlags & COMP_DISP_HDR_SRC) {
    ets_printf(" COMP_DISP_HDR_SRC");
  }
  if (hdr->hdrFlags & COMP_DISP_HDR_TOTAL_LGTH) {
    ets_printf(" COMP_DISP_HDR_TOTAL_LGTH");
  }
  if (hdr->hdrFlags & COMP_DISP_HDR_GUID) {
    ets_printf(" COMP_DISP_HDR_GUID");
  }
  if (hdr->hdrFlags & COMP_DISP_HDR_SRC_ID) {
    ets_printf(" COMP_DISP_HDR_SRC_ID");
  }
  if (hdr->hdrFlags & COMP_DISP_HDR_FILLER) {
    ets_printf(" COMP_DISP_HDR_FILLER");
  }
  if (hdr->hdrFlags & COMP_DISP_PAYLOAD_CMD_KEY) {
    ets_printf(" COMP_DISP_PAYLOAD_CMD_KEY");
  }
  if (hdr->hdrFlags & COMP_DISP_PAYLOAD_CMD_LGTH) {
    ets_printf(" COMP_DISP_PAYLOAD_CMD_LGTH");
  }
  if (hdr->hdrFlags & COMP_DISP_PAYLOAD_CRC) {
    ets_printf(" COMP_DISP_PAYLOAD_CRC");
  }
  if (hdr->hdrFlags & COMP_DISP_TOTAL_CRC) {
    ets_printf(" COMP_DISP_TOTAL_CRC");
  }
  ets_printf("§");
  ets_printf("§hdr fieldSequence§");
  idx = 0;
  while (idx < COMP_DISP_MAX_SEQUENCE) {
    if (hdr->fieldSequence[idx] == 0) {
      break;
    }
    ets_printf("§ %d 0x%04x", idx, hdr->fieldSequence[idx]);
    if (hdr->fieldSequence[idx] & COMP_DISP_U16_DST) {
      ets_printf(" COMP_DISP_U16_DST");
    }
    if (hdr->fieldSequence[idx] & COMP_DISP_U16_SRC) {
      ets_printf(" COMP_DISP_U16_SRC");
    }
    if (hdr->fieldSequence[idx] & COMP_DISP_U16_TOTAL_LGTH) {
      ets_printf(" COMP_DISP_HDR_U16_LGTH");
    }
    if (hdr->fieldSequence[idx] & COMP_DISP_U8_VECTOR_GUID) {
      ets_printf(" COMP_DISP_U8_VECTOR_GUID");
    }
    if (hdr->fieldSequence[idx] & COMP_DISP_U16_SRC_ID) {
      ets_printf(" COMP_DISP_U16_SRC_ID");
    }
    if (hdr->fieldSequence[idx] & COMP_DISP_U8_VECTOR_HDR_FILLER) {
      ets_printf(" COMP_DISP_U8_VECTOR_HDR_FILLER");
    }
    if (hdr->fieldSequence[idx] & COMP_DISP_U16_CMD_KEY) {
      ets_printf(" COMP_DISP_U16_CMD_KEY");
    }
    if (hdr->fieldSequence[idx] & COMP_DISP_U0_CMD_LGTH) {
      ets_printf(" COMP_DISP_U0_CMD_LGTH");
    }
    if (hdr->fieldSequence[idx] & COMP_DISP_U8_CMD_LGTH) {
      ets_printf(" COMP_DISP_U0_CMD_LGTH");
    }
    if (hdr->fieldSequence[idx] & COMP_DISP_U16_CMD_LGTH) {
      ets_printf(" COMP_DISP_U0_CMD_LGTH");
    }
    if (hdr->fieldSequence[idx] & COMP_DISP_U0_CRC) {
      ets_printf(" COMP_DISP_U0_CRC");
    }
    if (hdr->fieldSequence[idx] & COMP_DISP_U8_CRC) {
      ets_printf(" COMP_DISP_U8_CRC");
    }
    if (hdr->fieldSequence[idx] & COMP_DISP_U16_CRC) {
      ets_printf(" COMP_DISP_U16_CRC");
    }
    if (hdr->fieldSequence[idx] & COMP_DISP_U0_TOTAL_CRC) {
      ets_printf(" COMP_DISP_U0_TOTAL_CRC");
    }
    if (hdr->fieldSequence[idx] & COMP_DISP_U8_TOTAL_CRC) {
      ets_printf(" COMP_DISP_U8_TOTAL_CRC");
    }
    if (hdr->fieldSequence[idx] & COMP_DISP_U16_TOTAL_CRC) {
      ets_printf(" COMP_DISP_U16_TOTAL_CRC");
    }
    ets_printf("§");
    idx++;
  }
//  ets_printf("§");
  return COMP_DISP_ERR_OK;
}

// ================================= dumpMsgHeaderInfos ====================================

static uint8_t dumpMsgHeaderInfos(compMsgDispatcher_t *self, msgHeaderInfos_t *hdrInfos) {
  int idx;

  ets_printf("dumpMsgHeaderInfos:\n");
  ets_printf("headerFlags: ");
  if (hdrInfos->headerFlags & COMP_DISP_U16_SRC) {
    ets_printf(" COMP_DISP_U16_SRC");
  }
  if (hdrInfos->headerFlags & COMP_DISP_U16_DST) {
    ets_printf(" COMP_DISP_U16_DST");
  }
  if (hdrInfos->headerFlags & COMP_DISP_U16_TOTAL_LGTH) {
    ets_printf(" COMP_DISP_U16_TOTAL_LGTH");
  }
  ets_printf("\n");
  ets_printf("hdrInfos headerSequence\n");
  idx = 0;
  while (idx < COMP_DISP_MAX_SEQUENCE) {
    if (hdrInfos->headerSequence[idx] == 0) {
      break;
    }
    ets_printf(" %d 0x%04x", idx, hdrInfos->headerSequence[idx]);
    if (hdrInfos->headerSequence[idx] & COMP_DISP_U16_DST) {
      ets_printf(" COMP_DISP_U16_DST");
    }
    if (hdrInfos->headerSequence[idx] & COMP_DISP_U16_SRC) {
      ets_printf(" COMP_DISP_U16_SRC");
    }
    if (hdrInfos->headerSequence[idx] & COMP_DISP_U16_TOTAL_LGTH) {
      ets_printf(" COMP_DISP_U16_TOTAL_LGTH");
    }
    if (hdrInfos->headerSequence[idx] & COMP_DISP_U8_VECTOR_GUID) {
      ets_printf(" COMP_DISP_U8_VECTOR_GUID");
    }
    if (hdrInfos->headerSequence[idx] & COMP_DISP_U16_SRC_ID) {
      ets_printf(" COMP_DISP_U16_SRC_ID");
    }
    if (hdrInfos->headerSequence[idx] & COMP_DISP_U8_VECTOR_HDR_FILLER) {
      ets_printf(" COMP_DISP_U8_VECTOR_HDR_FILLER");
    }
    if (hdrInfos->headerSequence[idx] & COMP_DISP_U0_CMD_LGTH) {
      ets_printf(" COMP_DISP_U0_CMD_LGTH");
    }
    if (hdrInfos->headerSequence[idx] & COMP_DISP_U8_CMD_LGTH) {
      ets_printf(" COMP_DISP_U8_CMD_LGTH");
    }
    if (hdrInfos->headerSequence[idx] & COMP_DISP_U16_CMD_LGTH) {
      ets_printf(" COMP_DISP_U16_CMD_LGTH");
    }
    if (hdrInfos->headerSequence[idx] & COMP_DISP_U0_CRC) {
      ets_printf(" COMP_DISP_U0_CRC");
    }
    if (hdrInfos->headerSequence[idx] & COMP_DISP_U8_CRC) {
      ets_printf(" COMP_DISP_U8_CRC");
    }
    if (hdrInfos->headerSequence[idx] & COMP_DISP_U16_CRC) {
      ets_printf(" COMP_DISP_U16_CRC");
    }
    if (hdrInfos->headerSequence[idx] & COMP_DISP_U0_TOTAL_CRC) {
      ets_printf(" COMP_DISP_U0_TOTAL_CRC");
    }
    if (hdrInfos->headerSequence[idx] & COMP_DISP_U8_TOTAL_CRC) {
      ets_printf(" COMP_DISP_U8_TOTAL_CRC");
    }
    if (hdrInfos->headerSequence[idx] & COMP_DISP_U16_TOTAL_CRC) {
      ets_printf(" COMP_DISP_U16_TOTAL_CRC");
    }
    ets_printf("\n");
    idx++;
  }
  ets_printf("\n");
  ets_printf("headerLgth: %d numParts: %d maxParts: %d currPartIdx: %d seqIdx: %d seqIdxAfterHeader: %d\n", hdrInfos->headerLgth, hdrInfos->numHeaderParts, hdrInfos->maxHeaderParts, hdrInfos->currPartIdx, hdrInfos->seqIdx, hdrInfos->seqIdxAfterHeader);
  return COMP_DISP_ERR_OK;
}

// ================================= getIntFromLine ====================================

static uint8_t getIntFromLine(uint8_t *myStr, long *uval, uint8_t **ep, bool *isEnd) {
  uint8_t *cp;
  char *endPtr;

  cp = myStr;
  while ((*cp != ',') && (*cp != '\n') && (*cp != '\r') && (*cp != '\0')) {
    cp++;
  }
  if ((*cp == '\n') || (*cp == '\r') || (*cp == '\0')) {
    *isEnd = true;
  } else {
    *isEnd = false;
  }
  *cp++ = '\0';
  *uval = c_strtoul(myStr, &endPtr, 10);
  if (cp-1 != (uint8_t *)endPtr) {
//ets_printf("getIntFromLine: %s %d %p %p\n", myStr, *uval, cp, endPtr);
     return COMP_MSG_DESC_ERR_BAD_VALUE;
  }
  *ep = cp;
  return COMP_MSG_DESC_ERR_OK;
}

// ================================= getStrFromLine ====================================

static uint8_t getStrFromLine(uint8_t *myStr, uint8_t **ep, bool *isEnd) {
  uint8_t *cp;
  char *endPtr;

  cp = myStr;
  while ((*cp != ',') && (*cp != '\n') && (*cp != '\r') && (*cp != '\0')) {
    cp++;
  }
  if ((*cp == '\n') || (*cp == '\r') || (*cp == '\0')) {
    *isEnd = true;
  } else {
    *isEnd = false;
  }
  *cp++ = '\0';
  *ep = cp;
  return COMP_MSG_DESC_ERR_OK;
}

#define checkIsEnd(val) { hdrInfos->lgth = *ep - myStr; if (val) return result; }

// ================================= getHeaderFieldsFromLine ====================================

static uint8_t getHeaderFieldsFromLine(compMsgDispatcher_t *self, msgHeaderInfos_t *hdrInfos, uint8_t *myStr, uint8_t **ep, int *seqIdx) {
  int result;
  bool isEnd;
  long uval;
  uint8_t *cp;
  uint8_t fieldNameId;

//ets_printf("numHeaderParts: %d seqidx: %d\n", hdrInfos->numHeaderParts, *seqIdx);
  cp = myStr;
  result = getIntFromLine(cp, &uval, ep, &isEnd);
  checkErrOK(result);
//ets_printf("desc: headerLgth: %d\n", uval);
  hdrInfos->headerLgth = (uint8_t)uval;
  checkIsEnd(isEnd);
  cp = *ep;
  while (!isEnd) {
    result = getStrFromLine(cp, ep, &isEnd);
    checkErrOK(result);
    if (cp[0] != '@') {
      return COMP_MSG_ERR_NO_SUCH_FIELD;
    }
    result = self->compMsgTypesAndNames->getFieldNameIdFromStr(self->compMsgTypesAndNames, cp, &fieldNameId, COMP_MSG_NO_INCR);
    checkErrOK(result);
    switch (fieldNameId) {
    case COMP_MSG_SPEC_FIELD_SRC:
      if (hdrInfos->headerFlags & COMP_DISP_HDR_SRC) {
        return COMP_MSG_ERR_DUPLICATE_FIELD;
      }
      hdrInfos->headerSequence[(*seqIdx)++] = COMP_DISP_U16_SRC;
      hdrInfos->headerFlags |= COMP_DISP_HDR_SRC;
      break;
    case COMP_MSG_SPEC_FIELD_DST:
      if (hdrInfos->headerFlags & COMP_DISP_HDR_DST) {
        return COMP_MSG_ERR_DUPLICATE_FIELD;
      }
      hdrInfos->headerSequence[(*seqIdx)++] = COMP_DISP_U16_DST;
      hdrInfos->headerFlags |= COMP_DISP_HDR_DST;
      break;
    case COMP_MSG_SPEC_FIELD_TOTAL_LGTH:
      if (hdrInfos->headerFlags & COMP_DISP_HDR_TOTAL_LGTH) {
        return COMP_MSG_ERR_DUPLICATE_FIELD;
      }
      hdrInfos->headerSequence[(*seqIdx)++] = COMP_DISP_U16_TOTAL_LGTH;
      hdrInfos->headerFlags |= COMP_DISP_HDR_TOTAL_LGTH;
      break;
    case COMP_MSG_SPEC_FIELD_SRC_ID:
      if (hdrInfos->headerFlags & COMP_DISP_HDR_SRC_ID) {
        return COMP_MSG_ERR_DUPLICATE_FIELD;
      }
      hdrInfos->headerSequence[(*seqIdx)++] = COMP_DISP_U16_SRC_ID;
      hdrInfos->headerFlags |= COMP_DISP_HDR_SRC_ID;
      break;
    case COMP_MSG_SPEC_FIELD_GUID:
      if (hdrInfos->headerFlags & COMP_DISP_HDR_GUID) {
        return COMP_MSG_ERR_DUPLICATE_FIELD;
      }
      hdrInfos->headerSequence[(*seqIdx)++] = COMP_DISP_U8_VECTOR_GUID;
      hdrInfos->headerFlags |= COMP_DISP_HDR_GUID;;
      break;
    case COMP_MSG_SPEC_FIELD_HDR_FILLER:
      if (hdrInfos->headerFlags & COMP_DISP_HDR_FILLER) {
        return COMP_MSG_ERR_DUPLICATE_FIELD;
      }
      hdrInfos->headerSequence[(*seqIdx)++] = COMP_DISP_U8_VECTOR_HDR_FILLER;
      hdrInfos->headerFlags |= COMP_DISP_HDR_FILLER;
      break;
    default:
      checkErrOK(COMP_MSG_ERR_NO_SUCH_FIELD);
      break;
    }
    cp = *ep;
  }
  hdrInfos->seqIdxAfterHeader = *seqIdx;
  if (!isEnd) {
    return COMP_MSG_DESC_ERR_FUNNY_EXTRA_FIELDS;
  }
  return COMP_MSG_ERR_OK;
}
  
#undef checkIsEnd
#define checkIsEnd(val) { hdr->hdrLgth = ep - myStr; if (val) return result; }

#undef checkErrOK
#define checkErrOK(result) if(result != DATA_VIEW_ERR_OK) { compMsgMsgDesc->closeFile(compMsgMsgDesc); return result; }

// ================================= readHeadersAndSetFlags ====================================

/**
 * \brief Read the CompMsgHeads.txt file and store the header fields in some structures.
 * \param self The dispatcher struct
 * \param fileName The file name to read normally CompMsgHeads.txt
 * \return Error code or ErrorOK
 *
 */

static uint8_t readHeadersAndSetFlags(compMsgDispatcher_t *self, uint8_t *fileName) {
  int result;
  int result2;
  uint8_t numEntries;
  uint8_t fieldNameId;
  uint8_t fieldTypeId;
  char *endPtr;
  uint8_t lgth;
  uint8_t buf[BUFSIZ];
  uint8_t *buffer = buf;
  long uval;
  uint8_t *myStr;
  int idx;
  int headerEndIdx;
  int seqIdx;
  int seqIdx2;
  uint8_t*cp;
  uint8_t*ep;
  uint8_t found;
  uint8_t fieldOffset;
  bool isEnd;
  bool isJoker;
  headerPart_t *hdr;
  msgHeaderInfos_t *hdrInfos;
  compMsgMsgDesc_t *compMsgMsgDesc;

//ets_printf("readHeadersAndSetFlags\n");
  compMsgMsgDesc = self->compMsgMsgDesc;
  hdrInfos = &self->msgHeaderInfos;
  hdrInfos->currPartIdx = 0;
  result = compMsgMsgDesc->openFile(compMsgMsgDesc, fileName, "r");
  checkErrOK(result);
  // check for lgth is done in readLine!
  result = compMsgMsgDesc->readLine(compMsgMsgDesc, &buffer, &lgth);
  checkErrOK(result);
  if ((lgth < 4) || (buffer[0] != '#')) {
     return COMP_DISP_ERR_BAD_FILE_CONTENTS;
  }
  uval = c_strtoul(buffer+2, &endPtr, 10);
  numEntries = (uint8_t)uval;
  hdrInfos->headerParts = (headerPart_t *)os_zalloc(numEntries * (sizeof(headerPart_t)));
  checkAllocOK(self->msgHeaderInfos.headerParts);
  hdrInfos->numHeaderParts = 0;
  hdrInfos->maxHeaderParts = numEntries;
  hdrInfos->headerFlags = 0;
  // parse header description
  result = compMsgMsgDesc->readLine(compMsgMsgDesc, &buffer, &lgth);
  checkErrOK(result);
  seqIdx = 0;
  buffer[lgth] = 0;
  myStr = buffer;
  result = compMsgMsgDesc->getHeaderFieldsFromLine(self, hdrInfos, myStr, &cp, &seqIdx);
  checkErrOK(result);
  fieldOffset = 0;
  idx = 0;
  while(idx < numEntries) {
// FIXME should check for lgth against buffer length here !!!
    result = compMsgMsgDesc->readLine(compMsgMsgDesc, &buffer, &lgth);
    checkErrOK(result);
    if (lgth == 0) {
      return COMP_DISP_ERR_TOO_FEW_FILE_LINES;
    }
    hdr = &hdrInfos->headerParts[idx];
    hdr->hdrFlags = 0;
    buffer[lgth] = 0;
    myStr = buffer;
    cp = buffer;
    seqIdx2 = 0;
    while (seqIdx2 < seqIdx) {
      hdr->fieldSequence[seqIdx2] = hdrInfos->headerSequence[seqIdx2];
      if (cp[0] == '*') {
        isJoker = true;
        result = compMsgMsgDesc->getStrFromLine(cp, &ep, &isEnd);
        checkErrOK(result);
      } else {
        isJoker = false;
      }
      switch (hdr->fieldSequence[seqIdx2]) {
      case COMP_DISP_U16_SRC:
        if (isJoker) {
          hdr->hdrFromPart = 0;
        } else {
          result = compMsgMsgDesc->getIntFromLine(cp, &uval, &ep, &isEnd);
          checkErrOK(result);
          hdr->hdrFromPart = (uint16_t)uval;
        }
        break;
      case COMP_DISP_U16_DST:
        if (isJoker) {
          hdr->hdrToPart = 0;
        } else {
          result = compMsgMsgDesc->getIntFromLine(cp, &uval, &ep, &isEnd);
          checkErrOK(result);
          hdr->hdrToPart = (uint16_t)uval;
        }
        break;
      case COMP_DISP_U16_SRC_ID:
        if (isJoker) {
          hdr->hdrSrcId = 0;
        } else {
          result = compMsgMsgDesc->getIntFromLine(cp, &uval, &ep, &isEnd);
          checkErrOK(result);
          hdr->hdrSrcId = (uint16_t)uval;
        }
        break;
      case COMP_DISP_U16_TOTAL_LGTH:
        if (isJoker) {
          hdr->hdrTotalLgth = 0;
        } else {
          result = compMsgMsgDesc->getIntFromLine(cp, &uval, &ep, &isEnd);
          checkErrOK(result);
          hdr->hdrTotalLgth = (uint16_t)uval;
        }
        break;
      case COMP_DISP_U8_VECTOR_GUID:
        if (isJoker) {
          hdr->hdrGUID[0] = '\0';
        } else {
          result = compMsgMsgDesc->getStrFromLine(cp, &ep, &isEnd);
          checkErrOK(result);
          if (c_strlen(cp) > DISP_GUID_LGTH) {
            checkErrOK(COMP_MSG_DESC_ERR_FIELD_TOO_LONG);
          }
          c_memcpy(hdr->hdrGUID, cp, c_strlen(cp));
          hdr->hdrGUID[c_strlen(cp)] = '\0';
        }
        break;
      case COMP_DISP_U8_VECTOR_HDR_FILLER:
        if (isJoker) {
          hdr->hdrFiller[0] = '\0';
        } else {
          result = compMsgMsgDesc->getStrFromLine(cp, &ep, &isEnd);
          checkErrOK(result);
          if (c_strlen(cp) > DISP_MAX_HDR_FILLER_LGTH) {
            checkErrOK(COMP_MSG_DESC_ERR_FIELD_TOO_LONG);
          }
          c_memcpy(hdr->hdrFiller, cp, c_strlen(cp));
          hdr->hdrFiller[c_strlen(cp)] = '\0';
        }
        break;
      default: 
        checkErrOK(COMP_MSG_ERR_FIELD_NOT_FOUND);
        break;
      }
      if (isEnd) {
        checkErrOK(COMP_MSG_ERR_FIELD_NOT_FOUND);
        break;
      }
      cp = ep;
      seqIdx2++;
    }
    // encryption E/N
    result = compMsgMsgDesc->getStrFromLine(cp, &ep, &isEnd);
    checkErrOK(result);
    hdr->hdrEncryption = cp[0];
    checkIsEnd(isEnd);
    cp = ep;
    // handleType A/G/S/R/U/W/N
    result = compMsgMsgDesc->getStrFromLine(cp, &ep, &isEnd);
    checkErrOK(result);
    hdr->hdrHandleType = cp[0];
    switch (cp[0]) {
    case 'A':
    case 'G':
    case 'S':
    case 'R':
    case 'U':
    case 'W':
    case 'N':
      break;
    default:
ets_printf("bad value: %s\n", cp);
      checkErrOK(COMP_DISP_ERR_BAD_VALUE);
      break;
    }
    checkIsEnd(isEnd);
    cp = ep;
    // type of cmdKey
    result = compMsgMsgDesc->getStrFromLine(cp, &ep, &isEnd);
    checkErrOK(result);
    result = self->compMsgTypesAndNames->getFieldTypeIdFromStr(self->compMsgTypesAndNames, cp, &fieldTypeId);
    checkErrOK(result);
    checkIsEnd(isEnd);
    cp = ep;
    // cmdKey
    result = compMsgMsgDesc->getStrFromLine(cp, &ep, &isEnd);
    checkErrOK(result);
    switch (fieldTypeId) {
    case DATA_VIEW_FIELD_UINT16_T:
      hdr->fieldSequence[seqIdx2] = COMP_DISP_U16_CMD_KEY;
      hdr->hdrFlags |= COMP_DISP_PAYLOAD_CMD_KEY;
      hdr->hdrU16CmdKey = (cp[0]<<8)|cp[1];
//ets_printf("§u16CmdKey!0x%04x!§\n", hdr->hdrU16CmdKey);
      break;
    default:
      checkErrOK(COMP_DISP_ERR_BAD_FIELD_TYPE);
    }
    checkIsEnd(isEnd);
    cp = ep;
    seqIdx2++;
    // type of cmdLgth
    result = compMsgMsgDesc->getStrFromLine(cp, &ep, &isEnd);
    checkErrOK(result);
    result = self->compMsgTypesAndNames->getFieldTypeIdFromStr(self->compMsgTypesAndNames, cp, &fieldTypeId);
    checkErrOK(result);
    switch (fieldTypeId) {
    case DATA_VIEW_FIELD_NONE:
      hdr->fieldSequence[seqIdx2] = COMP_DISP_U0_CMD_LGTH;
      hdr->hdrFlags |= COMP_DISP_PAYLOAD_CMD_LGTH;
      break;
    case DATA_VIEW_FIELD_UINT8_T:
      hdr->fieldSequence[seqIdx2] = COMP_DISP_U8_CMD_LGTH;
      hdr->hdrFlags |= COMP_DISP_PAYLOAD_CMD_LGTH;
      break;
    case DATA_VIEW_FIELD_UINT16_T:
      hdr->fieldSequence[seqIdx2] = COMP_DISP_U16_CMD_LGTH;
      hdr->hdrFlags |= COMP_DISP_PAYLOAD_CMD_LGTH;
      break;
    default:
      checkErrOK(COMP_DISP_ERR_BAD_FIELD_TYPE);
    }
    checkIsEnd(isEnd);
    cp = ep;
    seqIdx2++;
    // type of crc
    result = compMsgMsgDesc->getStrFromLine(cp, &ep, &isEnd);
    checkErrOK(result);
    result = self->compMsgTypesAndNames->getFieldTypeIdFromStr(self->compMsgTypesAndNames, cp, &fieldTypeId);
    checkErrOK(result);
    switch (fieldTypeId) {
    case DATA_VIEW_FIELD_NONE:
      hdr->fieldSequence[seqIdx2] = COMP_DISP_U0_CRC;
      hdr->hdrFlags |= COMP_DISP_PAYLOAD_CRC;
      break;
    case DATA_VIEW_FIELD_UINT8_T:
      hdr->fieldSequence[seqIdx2] = COMP_DISP_U8_CRC;
      hdr->hdrFlags |= COMP_DISP_PAYLOAD_CRC;
      break;
    case DATA_VIEW_FIELD_UINT16_T:
      hdr->fieldSequence[seqIdx2] = COMP_DISP_U16_CRC;
      hdr->hdrFlags |= COMP_DISP_PAYLOAD_CRC;
      break;
    default:
      checkErrOK(COMP_DISP_ERR_BAD_FIELD_TYPE);
//ets_printf("§flag idx!%d!%s!§", flagIdx, getFlagStr(hdrInfos->headerSequence[flagIdx]));
    }
    checkIsEnd(isEnd);
    cp = ep;
    seqIdx2++;
    // type of totalCrc
    result = compMsgMsgDesc->getStrFromLine(cp, &ep, &isEnd);
//ets_printf("§totalCrc: %s!%d!§", cp, seqIdx2);
    checkErrOK(result);
    result = self->compMsgTypesAndNames->getFieldTypeIdFromStr(self->compMsgTypesAndNames, cp, &fieldTypeId);
    checkErrOK(result);
    switch (fieldTypeId) {
    case DATA_VIEW_FIELD_NONE:
//ets_printf("§none§");
      hdr->fieldSequence[seqIdx2] = COMP_DISP_U0_TOTAL_CRC;
      hdr->hdrFlags |= COMP_DISP_TOTAL_CRC;
      break;
    case DATA_VIEW_FIELD_UINT8_T:
//ets_printf("§u8§");
      hdr->fieldSequence[seqIdx2] = COMP_DISP_U8_TOTAL_CRC;
      hdr->hdrFlags |= COMP_DISP_TOTAL_CRC;
      break;
    case DATA_VIEW_FIELD_UINT16_T:
//ets_printf("§u16§");
      hdr->fieldSequence[seqIdx2] = COMP_DISP_U16_TOTAL_CRC;
      hdr->hdrFlags |= COMP_DISP_TOTAL_CRC;
      break;
    default:
      checkErrOK(COMP_DISP_ERR_BAD_FIELD_TYPE);
//ets_printf("§flag idx!%d!%s!§", flagIdx, getFlagStr(hdrInfos->headerSequence[flagIdx]));
    }
    if (!isEnd) {
      return COMP_MSG_DESC_ERR_FUNNY_EXTRA_FIELDS;
    }
    hdrInfos->numHeaderParts++;
    idx++;
  }
  result2 = compMsgMsgDesc->closeFile(compMsgMsgDesc);
  checkErrOK(result2);
  return result;
}

#undef checkErrOK
#define checkErrOK(result) if(result != DATA_VIEW_ERR_OK) return result

#undef checkIsEnd
#define checkIsEnd(val) { if (val) return result; }

// ================================= readActions ====================================

static uint8_t readActions(compMsgDispatcher_t *self, uint8_t *fileName) {
  int result;
  uint8_t u8CmdKey;
  uint16_t u16CmdKey;
  uint8_t *actionName;
  uint8_t actionMode;
  long uval;
  uint8_t numEntries;
  uint8_t*cp;
  uint8_t*ep;
  char *endPtr;
  int idx;
  bool isEnd;
  uint8_t lgth;
  uint8_t buf[100];
  uint8_t *buffer;
  uint8_t *myStr;
  uint8_t fieldTypeId;
  compMsgMsgDesc_t *compMsgMsgDesc;

  compMsgMsgDesc = self->compMsgMsgDesc;
  buffer = buf;
  result = compMsgMsgDesc->openFile(compMsgMsgDesc, fileName, "r");
  checkErrOK(result);
#undef checkErrOK
#define checkErrOK(result) if(result != COMP_DISP_ERR_OK) { compMsgMsgDesc->closeFile(compMsgMsgDesc); return result; }
  result = compMsgMsgDesc->readLine(compMsgMsgDesc, &buffer, &lgth);
  checkErrOK(result);
  buffer[lgth] = 0;
  if ((lgth < 4) || (buffer[0] != '#')) {
     return COMP_DISP_ERR_BAD_FILE_CONTENTS;
  return COMP_MSG_DESC_ERR_OK;
  }
  uval = c_strtoul(buffer+2, &endPtr, 10);
  numEntries = (uint8_t)uval;
  idx = 0;
  while(idx < numEntries) {
    u8CmdKey = 0;
    u16CmdKey = 0;
    result = compMsgMsgDesc->readLine(compMsgMsgDesc, &buffer, &lgth);
    checkErrOK(result);
    if (lgth == 0) {
      return COMP_DISP_ERR_TOO_FEW_FILE_LINES;
    }
    buffer[lgth] = 0;
    cp = buffer;
    // actionName
    actionName = cp;
    result = compMsgMsgDesc->getStrFromLine(cp, &ep, &isEnd);
    checkErrOK(result);
    checkIsEnd(isEnd);
    cp = ep;

    // actionMode
    result = compMsgMsgDesc->getIntFromLine(cp, &uval, &ep, &isEnd);
    checkErrOK(result);
    actionMode = (uint8_t)uval;
    checkIsEnd(isEnd);
    cp = ep;

    // type of cmdKey
    result = compMsgMsgDesc->getStrFromLine(cp, &ep, &isEnd);
    checkErrOK(result);
    result = self->compMsgTypesAndNames->getFieldTypeIdFromStr(self->compMsgTypesAndNames, cp, &fieldTypeId);
    checkErrOK(result);
    checkIsEnd(isEnd);
    cp = ep;

    // cmdKey
    result = compMsgMsgDesc->getStrFromLine(cp, &ep, &isEnd);
    checkErrOK(result);
    switch (fieldTypeId) {
    case DATA_VIEW_FIELD_UINT8_T:
      u8CmdKey = cp[0];
      break;
    case DATA_VIEW_FIELD_UINT16_T:
      u16CmdKey = (cp[0]<<8)|cp[1];
      break;
    default:
      checkErrOK(COMP_DISP_ERR_BAD_FIELD_TYPE);
    }
    result = self->setActionEntry(self, actionName, actionMode, u8CmdKey, u16CmdKey);
    checkErrOK(result);
    if (!isEnd) {
      return COMP_MSG_DESC_ERR_FUNNY_EXTRA_FIELDS;
    }
    idx++;
  }
#undef checkErrOK
#define checkErrOK(result) if(result != COMP_DISP_ERR_OK) return result
  result = compMsgMsgDesc->closeFile(compMsgMsgDesc);
  checkErrOK(result);
  return COMP_MSG_DESC_ERR_OK;
}

// ================================= readWifiValues ====================================

static uint8_t readWifiValues(compMsgDispatcher_t *self, uint8_t *fileName) {
  int result;
  long uval;
  uint8_t numEntries;
  uint8_t*cp;
  uint8_t*ep;
  char *endPtr;
  int idx;
  bool isEnd;
  uint8_t lgth;
  uint8_t buf[100];
  uint8_t *buffer;
  uint8_t *myStr;
  uint8_t *fieldNameStr;
  uint8_t *fieldValueStr;
  uint8_t fieldTypeId;
  compMsgMsgDesc_t *compMsgMsgDesc;

  compMsgMsgDesc = self->compMsgMsgDesc;
  buffer = buf;
  result = compMsgMsgDesc->openFile(compMsgMsgDesc, fileName, "r");
  checkErrOK(result);
#undef checkErrOK
#define checkErrOK(result) if(result != COMP_DISP_ERR_OK) { compMsgMsgDesc->closeFile(compMsgMsgDesc); return result; }
  result = compMsgMsgDesc->readLine(compMsgMsgDesc, &buffer, &lgth);
  checkErrOK(result);
  buffer[lgth] = 0;
  if ((lgth < 4) || (buffer[0] != '#')) {
     return COMP_DISP_ERR_BAD_FILE_CONTENTS;
  return COMP_MSG_DESC_ERR_OK;
  }
  uval = c_strtoul(buffer+2, &endPtr, 10);
  numEntries = (uint8_t)uval;
  idx = 0;
  while(idx < numEntries) {
    result = compMsgMsgDesc->readLine(compMsgMsgDesc, &buffer, &lgth);
    checkErrOK(result);
    if (lgth == 0) {
      return COMP_DISP_ERR_TOO_FEW_FILE_LINES;
    }
    buffer[lgth] = 0;
    cp = buffer;
    // wifiFieldName
    fieldNameStr = cp;
    result = compMsgMsgDesc->getStrFromLine(cp, &ep, &isEnd);
    checkErrOK(result);
    checkIsEnd(isEnd);
    cp = ep;

    // wifiFieldValue
    fieldValueStr = cp;
    result = compMsgMsgDesc->getStrFromLine(cp, &ep, &isEnd);
    checkErrOK(result);
    uval = c_strtoul(fieldValueStr, &endPtr, 10);
    if ((endPtr - (char *)fieldValueStr) == c_strlen(fieldValueStr)) {
      if (c_strlen(fieldValueStr) > 10) {
        // seems to be a password key, so use the stringValue
        result = self->setWifiValue(self, fieldNameStr, 0, fieldValueStr);
      } else {
        result = self->setWifiValue(self, fieldNameStr, uval, NULL);
      }
    } else {
      result = self->setWifiValue(self, fieldNameStr, 0, fieldValueStr);
    }
    if (!isEnd) {
      return COMP_MSG_DESC_ERR_FUNNY_EXTRA_FIELDS;
    }
    checkErrOK(result);
    idx++;
  }
#undef checkErrOK
#define checkErrOK(result) if(result != COMP_DISP_ERR_OK) return result
  result = compMsgMsgDesc->closeFile(compMsgMsgDesc);
  checkErrOK(result);
  return COMP_MSG_DESC_ERR_OK;
}

// ================================= readModuleValues ====================================

static uint8_t readModuleValues(compMsgDispatcher_t *self, uint8_t *fileName) {
  int result;
  long uval;
  uint8_t numEntries;
  uint8_t*cp;
  uint8_t*ep;
  char *endPtr;
  int idx;
  bool isEnd;
  uint8_t lgth;
  uint8_t buf[100];
  uint8_t *buffer;
  uint8_t *myStr;
  uint8_t *fieldNameStr;
  uint8_t *fieldValueStr;
  uint8_t fieldTypeId;
  compMsgMsgDesc_t *compMsgMsgDesc;

  compMsgMsgDesc = self->compMsgMsgDesc;
  buffer = buf;
  result = compMsgMsgDesc->openFile(compMsgMsgDesc, fileName, "r");
  checkErrOK(result);
#undef checkErrOK
#define checkErrOK(result) if(result != COMP_DISP_ERR_OK) { compMsgMsgDesc->closeFile(compMsgMsgDesc); return result; }
  result = compMsgMsgDesc->readLine(compMsgMsgDesc, &buffer, &lgth);
  checkErrOK(result);
  buffer[lgth] = 0;
  if ((lgth < 4) || (buffer[0] != '#')) {
     return COMP_DISP_ERR_BAD_FILE_CONTENTS;
  return COMP_MSG_DESC_ERR_OK;
  }
  uval = c_strtoul(buffer+2, &endPtr, 10);
  numEntries = (uint8_t)uval;
  idx = 0;
  while(idx < numEntries) {
    result = compMsgMsgDesc->readLine(compMsgMsgDesc, &buffer, &lgth);
    checkErrOK(result);
    if (lgth == 0) {
      return COMP_DISP_ERR_TOO_FEW_FILE_LINES;
    }
    buffer[lgth] = 0;
    cp = buffer;
    // wifiFieldName
    fieldNameStr = cp;
    result = compMsgMsgDesc->getStrFromLine(cp, &ep, &isEnd);
    checkErrOK(result);
    checkIsEnd(isEnd);
    cp = ep;

    // wifiFieldValue
    fieldValueStr = cp;
    result = compMsgMsgDesc->getStrFromLine(cp, &ep, &isEnd);
    checkErrOK(result);
    uval = c_strtoul(fieldValueStr, &endPtr, 10);
    if ((endPtr - (char *)fieldValueStr) == c_strlen(fieldValueStr)) {
      if (c_strlen(fieldValueStr) > 10) {
        // seems to be a password key, so use the stringValue
        result = self->setWifiValue(self, fieldNameStr, 0, fieldValueStr);
      } else {
        result = self->setWifiValue(self, fieldNameStr, uval, NULL);
      }
    } else {
      result = self->setWifiValue(self, fieldNameStr, 0, fieldValueStr);
    }
    if (!isEnd) {
      return COMP_MSG_DESC_ERR_FUNNY_EXTRA_FIELDS;
    }
    checkErrOK(result);
    idx++;
  }
#undef checkErrOK
#define checkErrOK(result) if(result != COMP_DISP_ERR_OK) return result
  result = compMsgMsgDesc->closeFile(compMsgMsgDesc);
  checkErrOK(result);
  return COMP_MSG_DESC_ERR_OK;
}

// ================================= getHeaderFromUniqueFields ====================================

static uint8_t  getHeaderFromUniqueFields (compMsgDispatcher_t *self, uint16_t dst, uint16_t src, uint16_t cmdKey, headerPart_t **hdr) {
  int idx;

  idx = 0;
  while (idx < self->msgHeaderInfos.numHeaderParts) {
    *hdr = &self->msgHeaderInfos.headerParts[idx];
    if ((*hdr)->hdrToPart == dst) {
      if ((*hdr)->hdrFromPart == src) {
        if ((*hdr)->hdrU16CmdKey == cmdKey) {
           return COMP_MSG_ERR_OK;
        }
      }
    }
    idx++;
  }
  return COMP_DISP_ERR_HEADER_NOT_FOUND;
}

// ================================= dumpMsgDescPart ====================================

static uint8_t dumpMsgDescPart(compMsgDispatcher_t *self, msgDescPart_t *msgDescPart) {
  uint8_t result;
  fieldSizeCallback_t callback;
  uint8_t *callbackName;

  callbackName = "nil";
  if (msgDescPart->fieldSizeCallback != NULL) {
    result = self->getFieldValueCallbackName(self, msgDescPart->fieldSizeCallback, &callbackName, 0);
    checkErrOK(result);
  }
  ets_printf("msgDescPart: fieldNameStr: %-15.15s fieldNameId: %.3d fieldTypeStr: %-10.10s fieldTypeId: %.3d field_lgth: %d callback: %s\n", msgDescPart->fieldNameStr, msgDescPart->fieldNameId, msgDescPart->fieldTypeStr, msgDescPart->fieldTypeId, msgDescPart->fieldLgth, callbackName);
  return COMP_DISP_ERR_OK;
}

// ================================= dumpMsgValPart ====================================

static uint8_t dumpMsgValPart(compMsgDispatcher_t *self, msgValPart_t *msgValPart) {
  uint8_t result;
  fieldValueCallback_t callback;
  uint8_t *callbackName;

  callbackName = "nil";
  if (msgValPart->fieldValueCallback != NULL) {
    result = self->getFieldValueCallbackName(self, msgValPart->fieldValueCallback, &callbackName, msgValPart->fieldValueCallbackType);
    checkErrOK(result);
  }
  ets_printf("msgValPart: fieldNameStr: %-15.15s fieldNameId: %.3d fieldValueStr: %-10.10s callback: %s flags: ", msgValPart->fieldNameStr, msgValPart->fieldNameId, msgValPart->fieldValueStr, callbackName);
  if (msgValPart->fieldFlags & COMP_DISP_DESC_VALUE_IS_NUMBER) {
     ets_printf(" COMP_DISP_DESC_VALUE_IS_NUMBER");
  }
  ets_printf("\n");
  return COMP_DISP_ERR_OK;
}

// ================================= getMsgPartsFromHeaderPart ====================================

static uint8_t getMsgPartsFromHeaderPart (compMsgDispatcher_t *self, headerPart_t *hdr, uint8_t **handle) {
  uint8_t result;
  char fileName[100];
  uint8_t *fieldValueStr;
  uint8_t *fieldNameStr;
  uint8_t *fieldTypeStr;
  uint8_t *fieldLgthStr;
  uint8_t fieldNameId;
  uint8_t fieldTypeId;
  char *endPtr;
  uint8_t lgth;
  uint8_t fieldLgth;
  uint8_t buf[100];
  uint8_t *buffer = buf;
  long uval;
  uint8_t *myStr;
  int idx;
  int numericValue;
  uint8_t numRows;
  uint8_t*cp;
  uint8_t*ep;
  uint8_t *keyValueCallback;
  bool isEnd;
  msgDescPart_t *msgDescPart;
  msgValPart_t *msgValPart;
  compMsgData_t *compMsgData;
  compMsgMsgDesc_t *compMsgMsgDesc;

//ets_printf("§getMsgPartsFromHeaderPart1§\n");
  compMsgData = self->compMsgData;
  compMsgMsgDesc = self->compMsgMsgDesc;
  self->compMsgData->currHdr = hdr;
  os_sprintf(fileName, "CompDesc%c%c.txt", (hdr->hdrU16CmdKey>>8)&0xFF, hdr->hdrU16CmdKey&0xFF);
//ets_printf("§file: %s§", fileName);
  result = compMsgMsgDesc->openFile(compMsgMsgDesc, fileName, "r");
  checkErrOK(result);
#undef checkErrOK
#define checkErrOK(result) if(result != COMP_DISP_ERR_OK) { compMsgMsgDesc->closeFile(compMsgMsgDesc); return result; }
  result = compMsgMsgDesc->readLine(compMsgMsgDesc, &buffer, &lgth);
  checkErrOK(result);
  buffer[lgth] = 0;
  cp = buffer;
  result = compMsgMsgDesc->getStrFromLine(cp, &ep, &isEnd);
  checkErrOK(result);
  if (isEnd) {
    return COMP_DISP_ERR_BAD_FILE_CONTENTS;
  }
  if (cp[0] != '#') {
    return COMP_DISP_ERR_BAD_FILE_CONTENTS;
  }
  cp = ep;
  result = compMsgData->deleteMsgDescParts(self);
  checkErrOK(result);
  result = compMsgMsgDesc->getIntFromLine(cp, &uval, &ep, &isEnd);
  checkErrOK(result);
  compMsgData->maxMsgDescParts = (uint8_t)uval;
  cp = ep;
  if (self->compMsgData->prepareValuesCbName != NULL) {
    os_free(self->compMsgData->prepareValuesCbName);
    self->compMsgData->prepareValuesCbName = NULL;
  }
  if (!isEnd) {
    result = compMsgMsgDesc->getStrFromLine(cp, &ep, &isEnd);
    checkErrOK(result);
    compMsgData->prepareValuesCbName = os_zalloc(c_strlen(cp) + 1);
    checkAllocOK(compMsgData->prepareValuesCbName);
    c_memcpy(compMsgData->prepareValuesCbName, cp, c_strlen(cp));
    compMsgData->prepareValuesCbName[c_strlen(cp)] = '\0';
  }
  if (!isEnd) {
    return COMP_MSG_DESC_ERR_FUNNY_EXTRA_FIELDS;
  }
  compMsgData->msgDescParts = os_zalloc(sizeof(msgDescPart_t) * compMsgData->maxMsgDescParts);
  checkAllocOK(compMsgData->msgDescParts);
  numRows = 0;
  idx = 0;
  while(idx < compMsgData->maxMsgDescParts) {
    result = compMsgMsgDesc->readLine(compMsgMsgDesc, &buffer, &lgth);
    checkErrOK(result);
    if (lgth == 0) {
      return COMP_DISP_ERR_TOO_FEW_FILE_LINES;
    }
    msgDescPart = &compMsgData->msgDescParts[compMsgData->numMsgDescParts++];
    buffer[lgth] = 0;
    cp = buffer;
    // fieldName
    fieldNameStr = cp;
    result = compMsgMsgDesc->getStrFromLine(cp, &ep, &isEnd);
    checkErrOK(result);
    checkIsEnd(isEnd);
    cp = ep;
    msgDescPart->fieldNameStr = os_zalloc(c_strlen(fieldNameStr)+ 1);
    checkAllocOK(msgDescPart->fieldNameStr);
    c_memcpy(msgDescPart->fieldNameStr, fieldNameStr, c_strlen(fieldNameStr));

    // fieldType
    fieldTypeStr = cp;
    result = compMsgMsgDesc->getStrFromLine(cp, &ep, &isEnd);
    checkErrOK(result);
    checkIsEnd(isEnd);
    cp = ep;
    msgDescPart->fieldTypeStr = os_zalloc(c_strlen(fieldTypeStr)+ 1);
    checkAllocOK(msgDescPart->fieldTypeStr);
    c_memcpy(msgDescPart->fieldTypeStr, fieldTypeStr, c_strlen(fieldTypeStr));

    // fieldLgth
    fieldLgthStr = cp;
    result = compMsgMsgDesc->getStrFromLine(cp, &ep, &isEnd);
    checkErrOK(result);
    if (c_strcmp(fieldLgthStr,"@numRows") == 0) {
      fieldLgth = numRows;
    } else {
      lgth = c_strtoul(fieldLgthStr, &endPtr, 10);
      fieldLgth = (uint8_t)lgth;
    }
    msgDescPart->fieldLgth = fieldLgth;
    result = self->compMsgTypesAndNames->getFieldTypeIdFromStr(self->compMsgTypesAndNames, fieldTypeStr, &msgDescPart->fieldTypeId);
    checkErrOK(result);
    result = self->compMsgTypesAndNames->getFieldNameIdFromStr(self->compMsgTypesAndNames, fieldNameStr, &msgDescPart->fieldNameId, COMP_MSG_INCR);
    checkErrOK(result);
    cp = ep;
    // eventually a callback for key value entries
    if (!isEnd) {
      keyValueCallback = cp;
      result = compMsgMsgDesc->getStrFromLine(cp, &ep, &isEnd);
      checkErrOK(result);
      result = self->getFieldValueCallback(self, cp, &msgDescPart->fieldSizeCallback, 0);
      checkErrOK(result);
    }
//self->compMsgMsgDesc->dumpMsgDescPart(self, msgDescPart);
    if (!isEnd) {
      return COMP_MSG_DESC_ERR_FUNNY_EXTRA_FIELDS;
    }
    cp = ep;
    idx++;
  }
  result = compMsgMsgDesc->closeFile(compMsgMsgDesc);
  checkErrOK(result);

  // and now the value parts
  os_sprintf(fileName, "CompVal%c%c.txt", (hdr->hdrU16CmdKey>>8)&0xFF, hdr->hdrU16CmdKey&0xFF);
  result = compMsgMsgDesc->openFile(compMsgMsgDesc, fileName, "r");
  checkErrOK(result);
  result = compMsgMsgDesc->readLine(compMsgMsgDesc, &buffer, &lgth);
  checkErrOK(result);
  buffer[lgth] = 0;
  cp = buffer;
  result = compMsgMsgDesc->getStrFromLine(cp, &ep, &isEnd);
  checkErrOK(result);
  if (isEnd) {
    return COMP_DISP_ERR_BAD_FILE_CONTENTS;
  }
  if (cp[0] != '#') {
    return COMP_DISP_ERR_BAD_FILE_CONTENTS;
  }
  cp = ep;
  result = compMsgData->deleteMsgValParts(self);
  checkErrOK(result);
  result = compMsgMsgDesc->getIntFromLine(cp, &uval, &ep, &isEnd);
  checkErrOK(result);
  self->compMsgData->maxMsgValParts = (uint8_t)uval;
  cp = ep;
  if (!isEnd) {
    return COMP_MSG_DESC_ERR_FUNNY_EXTRA_FIELDS;
  }
  compMsgData->msgValParts = os_zalloc(sizeof(msgValPart_t) * compMsgData->maxMsgValParts);
  checkAllocOK(compMsgData->msgValParts);
  idx = 0;
  while(idx < compMsgData->maxMsgValParts) {
    result = compMsgMsgDesc->readLine(compMsgMsgDesc, &buffer, &lgth);
    checkErrOK(result);
    if (lgth == 0) {
      return COMP_DISP_ERR_TOO_FEW_FILE_LINES;
    }
    msgValPart = &compMsgData->msgValParts[compMsgData->numMsgValParts++];
    buffer[lgth] = 0;
    cp = buffer;
    // fieldName
    fieldNameStr = cp;
    result = compMsgMsgDesc->getStrFromLine(cp, &ep, &isEnd);
    checkErrOK(result);
    msgValPart->fieldNameStr = os_zalloc(c_strlen(fieldNameStr)+ 1);
    checkAllocOK(msgValPart->fieldNameStr);
    c_memcpy(msgValPart->fieldNameStr, fieldNameStr, c_strlen(fieldNameStr));
    checkIsEnd(isEnd);
    result = self->compMsgTypesAndNames->getFieldNameIdFromStr(self->compMsgTypesAndNames, fieldNameStr, &msgValPart->fieldNameId, COMP_MSG_INCR);
    checkErrOK(result);
    cp = ep;

    // fieldValue
    fieldValueStr = cp;
    result = compMsgMsgDesc->getStrFromLine(cp, &ep, &isEnd);
    checkErrOK(result);
    msgValPart->fieldValueStr = os_zalloc(c_strlen(fieldValueStr)+ 1);
    checkAllocOK(msgValPart->fieldValueStr);
    c_memcpy(msgValPart->fieldValueStr, fieldValueStr, c_strlen(fieldValueStr));
    uval = c_strtoul(fieldValueStr, &endPtr, 10);
    if ((endPtr - (char *)fieldValueStr) == c_strlen(fieldValueStr)) {
      msgValPart->fieldFlags |= COMP_DISP_DESC_VALUE_IS_NUMBER;
      msgValPart->fieldValue = (uint32_t)uval;
    }
    if (fieldValueStr[0] == '@') {
      result = self->getFieldValueCallback(self, fieldValueStr, &msgValPart->fieldValueCallback, msgValPart->fieldValueCallbackType);
      checkErrOK(result);
    }
//self->compMsgMsgDesc->dumpMsgValPart(self, msgValPart);
    if (!isEnd) {
      return COMP_MSG_DESC_ERR_FUNNY_EXTRA_FIELDS;
    }
    idx++;
  }
#undef checkErrOK
#define checkErrOK(result) if(result != COMP_DISP_ERR_OK) return result
  result = compMsgMsgDesc->closeFile(compMsgMsgDesc);
//ets_printf("§getMsgPartsFromHeaderPart9 res: %d§", result);
  checkErrOK(result);
//ets_printf("§heap2: %d§", system_get_free_heap_size());

  return COMP_MSG_DESC_ERR_OK;
}

// ================================= getMsgKeyValueDescParts ====================================

static uint8_t getMsgKeyValueDescParts (compMsgDispatcher_t *self, uint8_t *fileName) {
  uint8_t result;
  uint8_t numEntries;
  uint8_t *fieldNameStr;
  uint8_t *fieldValueStr;
  uint8_t *fieldTypeStr;
  char *endPtr;
  uint8_t lgth;
  uint8_t fieldLgth;
  uint8_t buf[100];
  uint8_t *buffer = buf;
  long uval;
  uint8_t *myStr;
  int idx;
  int numericValue;
  uint8_t*cp;
  uint8_t*ep;
  bool isEnd;
  uint8_t bssInfoType;
  uint8_t fieldTypeId;
  compMsgMsgDesc_t *compMsgMsgDesc;
  msgKeyValueDescPart_t *msgKeyValueDescPart;

  compMsgMsgDesc = self->compMsgMsgDesc;
  result = compMsgMsgDesc->openFile(compMsgMsgDesc, fileName, "r");
  checkErrOK(result);
#undef checkErrOK
#define checkErrOK(result) if(result != COMP_DISP_ERR_OK) { compMsgMsgDesc->closeFile(compMsgMsgDesc); return result; }
  result = compMsgMsgDesc->readLine(compMsgMsgDesc, &buffer, &lgth);
  checkErrOK(result);
  buffer[lgth] = 0;
  if ((lgth < 4) || (buffer[0] != '#')) {
     return COMP_DISP_ERR_BAD_FILE_CONTENTS;
  }
  uval = c_strtoul(buffer+2, &endPtr, 10);
  numEntries = (uint8_t)uval;
  self->numMsgKeyValueDescParts = 0;
  self->maxMsgKeyValueDescParts = numEntries;
  self->msgKeyValueDescParts = os_zalloc(numEntries * sizeof(msgKeyValueDescPart_t));
  checkAllocOK(self->msgKeyValueDescParts);
  idx = 0;
  while(idx < numEntries) {
    result = compMsgMsgDesc->readLine(compMsgMsgDesc, &buffer, &lgth);
    checkErrOK(result);
    if (lgth == 0) {
      return COMP_DISP_ERR_TOO_FEW_FILE_LINES;
    }
    buffer[lgth] = 0;
    cp = buffer;
    msgKeyValueDescPart = &self->msgKeyValueDescParts[self->numMsgKeyValueDescParts];
    // fieldName
    fieldNameStr = cp;
    result = compMsgMsgDesc->getStrFromLine(cp, &ep, &isEnd);
    checkErrOK(result);
    msgKeyValueDescPart->keyNameStr = os_zalloc(c_strlen(fieldNameStr) + 1);
    checkAllocOK(msgKeyValueDescPart->keyNameStr);
    c_memcpy(msgKeyValueDescPart->keyNameStr, fieldNameStr, c_strlen(fieldNameStr) + 1);
    checkIsEnd(isEnd);
    cp = ep;

    // fieldValue
    fieldValueStr = cp;
    result = compMsgMsgDesc->getIntFromLine(cp, &uval, &ep, &isEnd);
    checkErrOK(result);
    msgKeyValueDescPart->keyId = (uint16_t)uval;
    checkIsEnd(isEnd);
    cp = ep;

    // fieldType
    fieldTypeStr = cp;
    result = compMsgMsgDesc->getStrFromLine(cp, &ep, &isEnd);
    checkErrOK(result);
    result = self->compMsgTypesAndNames->getFieldTypeIdFromStr(self->compMsgTypesAndNames, fieldTypeStr, &fieldTypeId);
    checkErrOK(result);
    msgKeyValueDescPart->keyType = fieldTypeId;
    cp = ep;

    // fieldLength 
    fieldTypeStr = cp;
    result = compMsgMsgDesc->getIntFromLine(cp, &uval, &ep, &isEnd);
    checkErrOK(result);
    msgKeyValueDescPart->keyLgth = (uint16_t)uval;
//ets_printf("field: %s Id: %d type: %d length: %d\n", msgKeyValueDescPart->keyNameStr, msgKeyValueDescPart->keyId, msgKeyValueDescPart->keyType, msgKeyValueDescPart->keyLgth);
    if (!isEnd) {
      return COMP_MSG_DESC_ERR_FUNNY_EXTRA_FIELDS;
    }
    cp = ep;
    self->numMsgKeyValueDescParts++;
    idx++;
  }
#undef checkErrOK
#define checkErrOK(result) if(result != COMP_DISP_ERR_OK) return result
  result = compMsgMsgDesc->closeFile(compMsgMsgDesc);
  checkErrOK(result);
  return COMP_MSG_ERR_OK;
//ets_printf("getWifiKeyValueKeys done\n");
}

// ================================= getFieldsToSave ====================================

static uint8_t getFieldsToSave(compMsgDispatcher_t *self, uint8_t *fileName) {
  uint8_t result;
  uint8_t numEntries;
  char *endPtr;
  uint8_t lgth;
  uint8_t *fieldNameStr;
  uint8_t buf[100];
  uint8_t *buffer = buf;
  uint8_t *myStr;
  int idx;
  long uval;
  uint8_t*cp;
  uint8_t*ep;
  bool isEnd;
  fieldsToSave_t *fieldsToSave;
  compMsgMsgDesc_t *compMsgMsgDesc;

  compMsgMsgDesc = self->compMsgMsgDesc;
  fieldsToSave = self->fieldsToSave;
  result = compMsgMsgDesc->openFile(compMsgMsgDesc, fileName, "r");
  checkErrOK(result);
#undef checkErrOK
#define checkErrOK(result) if(result != COMP_DISP_ERR_OK) { compMsgMsgDesc->closeFile(compMsgMsgDesc); return result; }
  result = compMsgMsgDesc->readLine(compMsgMsgDesc, &buffer, &lgth);
  checkErrOK(result);
  buffer[lgth] = 0;
  if ((lgth < 4) || (buffer[0] != '#')) {
     return COMP_DISP_ERR_BAD_FILE_CONTENTS;
  }
  uval = c_strtoul(buffer+2, &endPtr, 10);
  numEntries = (uint8_t)uval;
  self->numFieldsToSave = 0;
  self->maxFieldsToSave = numEntries;
ets_printf("getFieldsToSave: numEntries: %d\n");
  self->fieldsToSave = os_zalloc(numEntries * sizeof(fieldsToSave_t));
  checkAllocOK(self->fieldsToSave);
  idx = 0;
  while(idx < numEntries) {
    result = compMsgMsgDesc->readLine(compMsgMsgDesc, &buffer, &lgth);
    checkErrOK(result);
    if (lgth == 0) {
      return COMP_DISP_ERR_TOO_FEW_FILE_LINES;
    }
    buffer[lgth] = 0;
    cp = buffer;
    fieldsToSave = &self->fieldsToSave[self->numFieldsToSave];
    // fieldName
    fieldNameStr = cp;
    result = compMsgMsgDesc->getStrFromLine(cp, &ep, &isEnd);
ets_printf("getFieldsToSave: fieldName: %s %d\n", fieldNameStr,  self->numFieldsToSave);
    checkErrOK(result);
    fieldsToSave->fieldNameStr = os_zalloc(c_strlen(fieldNameStr) + 1);
    checkAllocOK(fieldsToSave->fieldNameStr);
    c_memcpy(fieldsToSave->fieldNameStr, fieldNameStr, c_strlen(fieldNameStr) + 1);
    result = self->compMsgTypesAndNames->getFieldNameIdFromStr(self->compMsgTypesAndNames, fieldNameStr, &fieldsToSave->fieldNameId, COMP_MSG_INCR);
    checkErrOK(result);
    if (!isEnd) {
      return COMP_MSG_DESC_ERR_FUNNY_EXTRA_FIELDS;
    }
    self->numFieldsToSave++;
    idx++;
  }
#undef checkErrOK
#define checkErrOK(result) if(result != COMP_DISP_ERR_OK) return result
  result = compMsgMsgDesc->closeFile(compMsgMsgDesc);
  checkErrOK(result);
  return COMP_MSG_ERR_OK;
//ets_printf("getFieldsToSave done\n");
}

// ================================= getWifiKeyValueKeys ====================================

static uint8_t getWifiKeyValueKeys (compMsgDispatcher_t *self, compMsgWifiData_t *compMsgWifiData) {
  uint8_t result;
  char fileName[100];
  uint8_t numEntries;
  uint8_t *fieldNameStr;
  uint8_t *fieldValueStr;
  uint8_t *fieldTypeStr;
  char *endPtr;
  uint8_t lgth;
  uint8_t fieldLgth;
  uint8_t buf[100];
  uint8_t *buffer = buf;
  long uval;
  uint8_t *myStr;
  int idx;
  int numericValue;
  uint8_t*cp;
  uint8_t*ep;
  bool isEnd;
  uint8_t bssInfoType;
  uint8_t fieldTypeId;
  compMsgMsgDesc_t *compMsgMsgDesc;

  compMsgMsgDesc = self->compMsgMsgDesc;
  result = compMsgMsgDesc->openFile(compMsgMsgDesc, "CompMsgKeyValueKeys.txt", "r");
  checkErrOK(result);
#undef checkErrOK
#define checkErrOK(result) if(result != COMP_DISP_ERR_OK) { compMsgMsgDesc->closeFile(compMsgMsgDesc); return result; }
  result = compMsgMsgDesc->readLine(compMsgMsgDesc, &buffer, &lgth);
  checkErrOK(result);
  buffer[lgth] = 0;
  if ((lgth < 4) || (buffer[0] != '#')) {
     return COMP_DISP_ERR_BAD_FILE_CONTENTS;
  }
  uval = c_strtoul(buffer+2, &endPtr, 10);
  numEntries = (uint8_t)uval;
  idx = 0;
  while(idx < numEntries) {
    result = compMsgMsgDesc->readLine(compMsgMsgDesc, &buffer, &lgth);
    checkErrOK(result);
    if (lgth == 0) {
      return COMP_DISP_ERR_TOO_FEW_FILE_LINES;
    }
    buffer[lgth] = 0;
    cp = buffer;
    // fieldName
    fieldNameStr = cp;
    result = compMsgMsgDesc->getStrFromLine(cp, &ep, &isEnd);
    checkErrOK(result);
    checkIsEnd(isEnd);
    cp = ep;

    // fieldValue
    fieldValueStr = cp;
    result = compMsgMsgDesc->getIntFromLine(cp, &uval, &ep, &isEnd);
    checkErrOK(result);
    result = self->bssStr2BssInfoId(fieldNameStr + c_strlen("@key_"), &bssInfoType);
    if (result != COMP_MSG_ERR_OK) {
      // not a key the Wifi is handling (normally cloud keys)
      idx++;
      continue;
    }
//    checkErrOK(result);
    switch (bssInfoType) {
    case BSS_INFO_BSSID:
      compMsgWifiData->key_bssid = (uint16_t)uval;
      break;
    case BSS_INFO_SSID:
      compMsgWifiData->key_ssid = (uint16_t)uval;
      break;
    case BSS_INFO_SSID_LEN:
      compMsgWifiData->key_ssid_len = (uint16_t)uval;
      break;
    case BSS_INFO_CHANNEL:
      compMsgWifiData->key_channel = (uint16_t)uval;
      break;
    case BSS_INFO_RSSI:
      compMsgWifiData->key_rssi = (uint16_t)uval;
      break;
    case BSS_INFO_AUTH_MODE:
      compMsgWifiData->key_authmode = (uint16_t)uval;
      break;
    case BSS_INFO_IS_HIDDEN:
      compMsgWifiData->key_freq_offset = (uint16_t)uval;
      break;
    case BSS_INFO_FREQ_OFFSET:
      compMsgWifiData->key_freqcal_val = (uint16_t)uval;
      break;
    case BSS_INFO_FREQ_CAL_VAL:
      compMsgWifiData->key_is_hidden = (uint16_t)uval;
      break;
    }
    checkIsEnd(isEnd);
    cp = ep;

    // fieldType
    fieldTypeStr = cp;
    result = compMsgMsgDesc->getStrFromLine(cp, &ep, &isEnd);
    checkErrOK(result);
    result = self->compMsgTypesAndNames->getFieldTypeIdFromStr(self->compMsgTypesAndNames, fieldTypeStr, &fieldTypeId);
    checkErrOK(result);
    switch (bssInfoType) {
    case BSS_INFO_BSSID:
      compMsgWifiData->bssScanTypes.bssidType = (uint8_t)fieldTypeId;
      break;
    case BSS_INFO_SSID:
      compMsgWifiData->bssScanTypes.ssidType = (uint8_t)fieldTypeId;
      break;
    case BSS_INFO_CHANNEL:
      compMsgWifiData->bssScanTypes.channelType = (uint8_t)fieldTypeId;
      break;
    case BSS_INFO_RSSI:
      compMsgWifiData->bssScanTypes.rssiType = (uint8_t)fieldTypeId;
      break;
    case BSS_INFO_AUTH_MODE:
      compMsgWifiData->bssScanTypes.authmodeType = (uint8_t)fieldTypeId;
      break;
    case BSS_INFO_IS_HIDDEN:
      compMsgWifiData->bssScanTypes.freq_offsetType = (uint8_t)fieldTypeId;
      break;
    case BSS_INFO_FREQ_OFFSET:
      compMsgWifiData->bssScanTypes.freqcal_valType = (uint8_t)fieldTypeId;
      break;
    case BSS_INFO_FREQ_CAL_VAL:
      compMsgWifiData->bssScanTypes.is_hiddenType = (uint8_t)fieldTypeId;
      break;
    }
    cp = ep;

    // fieldLength not needed for Wifi module
    fieldTypeStr = cp;
    result = compMsgMsgDesc->getStrFromLine(cp, &ep, &isEnd);
    checkErrOK(result);
//ets_printf("field: %s length: %s\n", fieldNameStr, cp);
    if (!isEnd) {
      return COMP_MSG_DESC_ERR_FUNNY_EXTRA_FIELDS;
    }
    cp = ep;
    idx++;
  }
#undef checkErrOK
#define checkErrOK(result) if(result != COMP_DISP_ERR_OK) return result
  result = compMsgMsgDesc->closeFile(compMsgMsgDesc);
  checkErrOK(result);
  return COMP_MSG_ERR_OK;
//ets_printf("getWifiKeyValueKeys done\n");
}

#undef checkIsEnd

// ================================= newCompMsgMsgDesc ====================================

compMsgMsgDesc_t *newCompMsgMsgDesc() {
  compMsgMsgDesc_t *compMsgMsgDesc = os_zalloc(sizeof(compMsgMsgDesc_t));
  if (compMsgMsgDesc == NULL) {
    return NULL;
  }
  compMsgMsgDescId++;
  compMsgMsgDesc->id = compMsgMsgDescId;

  compMsgMsgDesc->openFile = &openFile;
  compMsgMsgDesc->closeFile = &closeFile;
  compMsgMsgDesc->flushFile = &flushFile;
  compMsgMsgDesc->readLine = &readLine;
  compMsgMsgDesc->writeLine = &writeLine;
  compMsgMsgDesc->dumpHeaderPart = &dumpHeaderPart;
  compMsgMsgDesc->dumpMsgHeaderInfos = &dumpMsgHeaderInfos;
  compMsgMsgDesc->getIntFromLine = &getIntFromLine;
  compMsgMsgDesc->getStrFromLine = &getStrFromLine;
  compMsgMsgDesc->getHeaderFieldsFromLine = &getHeaderFieldsFromLine;
  compMsgMsgDesc->readActions = &readActions;
  compMsgMsgDesc->readModuleValues = &readModuleValues;
  compMsgMsgDesc->readWifiValues = &readWifiValues;
  compMsgMsgDesc->readHeadersAndSetFlags = &readHeadersAndSetFlags;
  compMsgMsgDesc->dumpMsgDescPart = &dumpMsgDescPart;
  compMsgMsgDesc->dumpMsgValPart = &dumpMsgValPart;
  compMsgMsgDesc->dumpHeaderPart = &dumpHeaderPart;
  compMsgMsgDesc->dumpMsgHeaderInfos = &dumpMsgHeaderInfos;
  compMsgMsgDesc->getMsgPartsFromHeaderPart = &getMsgPartsFromHeaderPart;
  compMsgMsgDesc->getHeaderFromUniqueFields = &getHeaderFromUniqueFields;
  compMsgMsgDesc->getMsgKeyValueDescParts = &getMsgKeyValueDescParts;
  compMsgMsgDesc->getFieldsToSave = &getFieldsToSave;
  compMsgMsgDesc->getWifiKeyValueKeys = &getWifiKeyValueKeys;

  return compMsgMsgDesc;
}

// ================================= freeCompMsgMsgDesc ====================================

void freeCompMsgMsgDesc(compMsgMsgDesc_t *compMsgMsgDesc) {
}

