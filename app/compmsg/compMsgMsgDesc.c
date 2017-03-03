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

typedef struct flag2Str {
  uint32_t flagVal;
  uint8_t *flagStr;
} flag2Str_t;

static flag2Str_t flag2Strs [] = {
  { COMP_DISP_U16_DST,           "COMP_DISP_U16_DST" },
  { COMP_DISP_U16_SRC,           "COMP_DISP_U16_SRC" },
  { COMP_DISP_U16_TOTAL_LGTH,    "COMP_DISP_U16_TOTAL_LGTH" },
  { COMP_DISP_U8_VECTOR_GUID,    "COMP_DISP_U8_VECTOR_GUID" },
  { COMP_DISP_U16_SRC_ID,        "COMP_DISP_U16_SRC_ID" },
  { COMP_DISP_IS_ENCRYPTED,      "COMP_DISP_IS_ENCRYPTED" },
  { COMP_DISP_IS_NOT_ENCRYPTED,  "COMP_DISP_IS_NOT_ENCRYPTED" },
  { COMP_DISP_U16_CMD_KEY,       "COMP_DISP_U16_CMD_KEY" },
  { COMP_DISP_U0_CMD_LGTH,       "COMP_DISP_U0_CMD_LGTH" },
  { COMP_DISP_U8_CMD_LGTH,       "COMP_DISP_U8_CMD_LGTH" },
  { COMP_DISP_U16_CMD_LGTH,      "COMP_DISP_U16_CMD_LGTH" },
  { 0,                             NULL },
};


static int compMsgMsgDescId = 0;
static volatile int fileFd = FS_OPEN_OK - 1;

// ================================= openFile ====================================

static uint8_t openFile(compMsgMsgDesc_t *self, const uint8_t *fileName, const uint8_t *fileMode) {
  self->fileName = fileName;
  fileFd = fs_open(fileName, fs_mode2flag(fileMode));
  if (fileFd < FS_OPEN_OK) {
    return COMP_MSG_ERR_OPEN_FILE;
  }
  return COMP_MSG_ERR_OK;
}

// ================================= closeFile ====================================

static uint8_t closeFile(compMsgMsgDesc_t *self) {
  if (fileFd != (FS_OPEN_OK - 1)){
    self->fileName = NULL;
    fs_close(fileFd);
    fileFd = FS_OPEN_OK - 1;
  }
  return COMP_MSG_ERR_OK;
}

// ================================= flushFile ====================================

static uint8_t flushFile(compMsgMsgDesc_t *self) {
  if (fileFd == (FS_OPEN_OK - 1)) {
    return COMP_MSG_ERR_FILE_NOT_OPENED;
  }
  if (fs_flush(fileFd) == 0) {
    return COMP_MSG_ERR_OK;
  }
  return COMP_MSG_ERR_FLUSH_FILE;
}

// ================================= readLine ====================================

static uint8_t readLine(compMsgMsgDesc_t *self, uint8_t **buffer, uint8_t *lgth) {
  size_t n = BUFSIZ;
  char buf[BUFSIZ];
  int i;
  uint8_t *cp;
  uint8_t end_char = '\n';

  if (fileFd == (FS_OPEN_OK - 1)) {
    return COMP_MSG_ERR_FILE_NOT_OPENED;
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
  return COMP_MSG_ERR_OK;
}

// ================================= writeLine ====================================

static uint8_t writeLine(compMsgMsgDesc_t *self, const uint8_t *buffer, uint8_t lgth) {
  int result;

  if (fileFd == (FS_OPEN_OK - 1)) {
    return COMP_MSG_ERR_FILE_NOT_OPENED;
  }
  result = fs_write(fileFd, buffer, lgth);
  if (result == lgth) {
    return COMP_MSG_ERR_OK;
  }
  return COMP_MSG_ERR_WRITE_FILE;
}

// ================================= getIntFromLine ====================================

static uint8_t getIntFromLine(compMsgDispatcher_t *self, uint8_t *myStr, long *uval, uint8_t **ep, bool *isEnd) {
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
    COMP_MSG_DBG(self, "E", 2, "getIntFromLine: %s %d %p %p\n", myStr, *uval, cp, endPtr);
    return COMP_MSG_ERR_BAD_VALUE;
  }
  *ep = cp;
  return COMP_MSG_ERR_OK;
}

// ================================= getStrFromLine ====================================

static uint8_t getStrFromLine(compMsgDispatcher_t *self, uint8_t *myStr, uint8_t **ep, bool *isEnd) {
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
  return COMP_MSG_ERR_OK;
}

// ================================= getFlagStr ====================================

static uint8_t *getFlagStr(uint32_t flags) {
  flag2Str_t *entry;
  int idx;
  
  idx = 0;
  while (1) {
    entry = &flag2Strs[idx];
    if (entry->flagStr == NULL) {
      return "??";
    }
    if (flags & entry->flagVal) {
      return entry->flagStr;
    }
    idx++;
  }
}

#define checkIsEnd(val) { hdrInfos->lgth = *ep - myStr; if (val) return result; }

// ================================= getHeaderFieldsFromLine ====================================

static uint8_t getHeaderFieldsFromLine(compMsgDispatcher_t *self, msgHeaderInfos_t *hdrInfos, uint8_t *myStr, uint8_t **ep, int *seqIdx) {
  int result;
  bool isEnd;
  long uval;
  uint8_t *cp;
  uint8_t fieldNameId;

  COMP_MSG_DBG(self, "E", 2, "numHeaderParts: %d seqidx: %d\n", hdrInfos->numHeaderParts, *seqIdx);
  cp = myStr;
  result = getIntFromLine(self, cp, &uval, ep, &isEnd);
  checkErrOK(result);
  COMP_MSG_DBG(self, "E", 2, "desc: headerLgth: %d\n", uval);
  hdrInfos->headerLgth = (uint8_t)uval;
  checkIsEnd(isEnd);
  cp = *ep;
  while (!isEnd) {
    result = getStrFromLine(self, cp, ep, &isEnd);
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
    return COMP_MSG_ERR_FUNNY_EXTRA_FIELDS;
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

  COMP_MSG_DBG(self, "E", 2, "readHeadersAndSetFlags\n");
  compMsgMsgDesc = self->compMsgMsgDesc;
  hdrInfos = &self->msgHeaderInfos;
  hdrInfos->currPartIdx = 0;
  result = compMsgMsgDesc->openFile(compMsgMsgDesc, fileName, "r");
  checkErrOK(result);
  // check for lgth is done in readLine!
  result = compMsgMsgDesc->readLine(compMsgMsgDesc, &buffer, &lgth);
  checkErrOK(result);
  if ((lgth < 4) || (buffer[0] != '#')) {
     return COMP_MSG_ERR_BAD_FILE_CONTENTS;
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
      return COMP_MSG_ERR_TOO_FEW_FILE_LINES;
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
        result = compMsgMsgDesc->getStrFromLine(self, cp, &ep, &isEnd);
        checkErrOK(result);
      } else {
        isJoker = false;
      }
      switch (hdr->fieldSequence[seqIdx2]) {
      case COMP_DISP_U16_SRC:
        if (isJoker) {
          hdr->hdrFromPart = 0;
        } else {
          result = compMsgMsgDesc->getIntFromLine(self, cp, &uval, &ep, &isEnd);
          checkErrOK(result);
          hdr->hdrFromPart = (uint16_t)uval;
        }
        break;
      case COMP_DISP_U16_DST:
        if (isJoker) {
          hdr->hdrToPart = 0;
        } else {
          result = compMsgMsgDesc->getIntFromLine(self, cp, &uval, &ep, &isEnd);
          checkErrOK(result);
          hdr->hdrToPart = (uint16_t)uval;
        }
        break;
      case COMP_DISP_U16_SRC_ID:
        if (isJoker) {
          hdr->hdrSrcId = 0;
        } else {
          result = compMsgMsgDesc->getIntFromLine(self, cp, &uval, &ep, &isEnd);
          checkErrOK(result);
          hdr->hdrSrcId = (uint16_t)uval;
        }
        break;
      case COMP_DISP_U16_TOTAL_LGTH:
        if (isJoker) {
          hdr->hdrTotalLgth = 0;
        } else {
          result = compMsgMsgDesc->getIntFromLine(self, cp, &uval, &ep, &isEnd);
          checkErrOK(result);
          hdr->hdrTotalLgth = (uint16_t)uval;
        }
        break;
      case COMP_DISP_U8_VECTOR_GUID:
        if (isJoker) {
          hdr->hdrGUID[0] = '\0';
        } else {
          result = compMsgMsgDesc->getStrFromLine(self, cp, &ep, &isEnd);
          checkErrOK(result);
          if (c_strlen(cp) > DISP_GUID_LGTH) {
            checkErrOK(COMP_MSG_ERR_FIELD_TOO_LONG);
          }
          c_memcpy(hdr->hdrGUID, cp, c_strlen(cp));
          hdr->hdrGUID[c_strlen(cp)] = '\0';
        }
        break;
      case COMP_DISP_U8_VECTOR_HDR_FILLER:
        if (isJoker) {
          hdr->hdrFiller[0] = '\0';
        } else {
          result = compMsgMsgDesc->getStrFromLine(self, cp, &ep, &isEnd);
          checkErrOK(result);
          if (c_strlen(cp) > DISP_MAX_HDR_FILLER_LGTH) {
            checkErrOK(COMP_MSG_ERR_FIELD_TOO_LONG);
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
    result = compMsgMsgDesc->getStrFromLine(self, cp, &ep, &isEnd);
    checkErrOK(result);
    hdr->hdrEncryption = cp[0];
    checkIsEnd(isEnd);
    cp = ep;
    // handleType A/G/S/R/U/W/D/N
    result = compMsgMsgDesc->getStrFromLine(self, cp, &ep, &isEnd);
    checkErrOK(result);
    hdr->hdrHandleType = cp[0];
    switch (cp[0]) {
    case 'A':
    case 'G':
    case 'S':
    case 'R':
    case 'U':
    case 'W':
    case 'D':
    case 'N':
      break;
    default:
      COMP_MSG_DBG(self, "Y", 0, "bad value: %s\n", cp);
      checkErrOK(COMP_MSG_ERR_BAD_VALUE);
      break;
    }
    checkIsEnd(isEnd);
    cp = ep;
    // type of cmdKey
    result = compMsgMsgDesc->getStrFromLine(self, cp, &ep, &isEnd);
    checkErrOK(result);
    result = self->compMsgTypesAndNames->getFieldTypeIdFromStr(self->compMsgTypesAndNames, cp, &fieldTypeId);
    checkErrOK(result);
    checkIsEnd(isEnd);
    cp = ep;
    // cmdKey
    result = compMsgMsgDesc->getStrFromLine(self, cp, &ep, &isEnd);
    checkErrOK(result);
    switch (fieldTypeId) {
    case DATA_VIEW_FIELD_UINT16_T:
      hdr->fieldSequence[seqIdx2] = COMP_DISP_U16_CMD_KEY;
      hdr->hdrFlags |= COMP_DISP_PAYLOAD_CMD_KEY;
      hdr->hdrU16CmdKey = (cp[0]<<8)|cp[1];
      COMP_MSG_DBG(self, "E", 2, "u16CmdKey!0x%04x!\n", hdr->hdrU16CmdKey);
      break;
    default:
      checkErrOK(COMP_MSG_ERR_BAD_FIELD_TYPE);
    }
    checkIsEnd(isEnd);
    cp = ep;
    seqIdx2++;
    // type of cmdLgth
    result = compMsgMsgDesc->getStrFromLine(self, cp, &ep, &isEnd);
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
      checkErrOK(COMP_MSG_ERR_BAD_FIELD_TYPE);
    }
    checkIsEnd(isEnd);
    cp = ep;
    seqIdx2++;
    // type of crc
    result = compMsgMsgDesc->getStrFromLine(self, cp, &ep, &isEnd);
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
      checkErrOK(COMP_MSG_ERR_BAD_FIELD_TYPE);
      COMP_MSG_DBG(self, "E", 2, "seqIdx2!%d!%s!", seqIdx2, getFlagStr(hdrInfos->headerSequence[seqIdx2]));
    }
    checkIsEnd(isEnd);
    cp = ep;
    seqIdx2++;
    // type of totalCrc
    result = compMsgMsgDesc->getStrFromLine(self, cp, &ep, &isEnd);
    COMP_MSG_DBG(self, "E", 2, "totalCrc: %s!%d!", cp, seqIdx2);
    checkErrOK(result);
    result = self->compMsgTypesAndNames->getFieldTypeIdFromStr(self->compMsgTypesAndNames, cp, &fieldTypeId);
    checkErrOK(result);
    switch (fieldTypeId) {
    case DATA_VIEW_FIELD_NONE:
      COMP_MSG_DBG(self, "E", 2, "none");
      hdr->fieldSequence[seqIdx2] = COMP_DISP_U0_TOTAL_CRC;
      hdr->hdrFlags |= COMP_DISP_TOTAL_CRC;
      break;
    case DATA_VIEW_FIELD_UINT8_T:
      COMP_MSG_DBG(self, "E", 2, "u8");
      hdr->fieldSequence[seqIdx2] = COMP_DISP_U8_TOTAL_CRC;
      hdr->hdrFlags |= COMP_DISP_TOTAL_CRC;
      break;
    case DATA_VIEW_FIELD_UINT16_T:
      COMP_MSG_DBG(self, "E", 2, "u16");
      hdr->fieldSequence[seqIdx2] = COMP_DISP_U16_TOTAL_CRC;
      hdr->hdrFlags |= COMP_DISP_TOTAL_CRC;
      break;
    default:
      checkErrOK(COMP_MSG_ERR_BAD_FIELD_TYPE);
      COMP_MSG_DBG(self, "E", 2, "seqIdx2!%d!%s!", seqIdx2, getFlagStr(hdrInfos->headerSequence[seqIdx2]));
    }
    if (!isEnd) {
      return COMP_MSG_ERR_FUNNY_EXTRA_FIELDS;
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
#define checkErrOK(result) if(result != COMP_MSG_ERR_OK) { compMsgMsgDesc->closeFile(compMsgMsgDesc); return result; }
  result = compMsgMsgDesc->readLine(compMsgMsgDesc, &buffer, &lgth);
  checkErrOK(result);
  buffer[lgth] = 0;
  if ((lgth < 4) || (buffer[0] != '#')) {
     return COMP_MSG_ERR_BAD_FILE_CONTENTS;
  return COMP_MSG_ERR_OK;
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
      return COMP_MSG_ERR_TOO_FEW_FILE_LINES;
    }
    buffer[lgth] = 0;
    cp = buffer;
    // actionName
    actionName = cp;
    result = compMsgMsgDesc->getStrFromLine(self, cp, &ep, &isEnd);
    checkErrOK(result);
    checkIsEnd(isEnd);
    cp = ep;

    // actionMode
    result = compMsgMsgDesc->getIntFromLine(self, cp, &uval, &ep, &isEnd);
    checkErrOK(result);
    actionMode = (uint8_t)uval;
    checkIsEnd(isEnd);
    cp = ep;

    // type of cmdKey
    result = compMsgMsgDesc->getStrFromLine(self, cp, &ep, &isEnd);
    checkErrOK(result);
    result = self->compMsgTypesAndNames->getFieldTypeIdFromStr(self->compMsgTypesAndNames, cp, &fieldTypeId);
    checkErrOK(result);
    checkIsEnd(isEnd);
    cp = ep;

    // cmdKey
    result = compMsgMsgDesc->getStrFromLine(self, cp, &ep, &isEnd);
    checkErrOK(result);
    switch (fieldTypeId) {
    case DATA_VIEW_FIELD_UINT8_T:
      u8CmdKey = cp[0];
      break;
    case DATA_VIEW_FIELD_UINT16_T:
      u16CmdKey = (cp[0]<<8)|cp[1];
      break;
    default:
      checkErrOK(COMP_MSG_ERR_BAD_FIELD_TYPE);
    }
    result = self->compMsgAction->setActionEntry(self, actionName, actionMode, u8CmdKey, u16CmdKey);
    checkErrOK(result);
    if (!isEnd) {
      return COMP_MSG_ERR_FUNNY_EXTRA_FIELDS;
    }
    idx++;
  }
#undef checkErrOK
#define checkErrOK(result) if(result != COMP_MSG_ERR_OK) return result
  result = compMsgMsgDesc->closeFile(compMsgMsgDesc);
  checkErrOK(result);
  return COMP_MSG_ERR_OK;
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
  uint8_t buf[256];
  uint8_t *buffer;
  uint8_t *myStr;
  uint8_t *fieldNameStr;
  uint8_t *fieldValueStr;
  uint8_t fieldTypeId;
  compMsgMsgDesc_t *compMsgMsgDesc;

  compMsgMsgDesc = self->compMsgMsgDesc;
  buffer = buf;
  result = compMsgMsgDesc->openFile(compMsgMsgDesc, fileName, "r");
  COMP_MSG_DBG(self, "E", 2, "readWifiValues: %s\n", fileName);
  checkErrOK(result);
#undef checkErrOK
#define checkErrOK(result) if(result != COMP_MSG_ERR_OK) { compMsgMsgDesc->closeFile(compMsgMsgDesc); return result; }
  result = compMsgMsgDesc->readLine(compMsgMsgDesc, &buffer, &lgth);
  checkErrOK(result);
  buffer[lgth] = 0;
  if ((lgth < 4) || (buffer[0] != '#')) {
     return COMP_MSG_ERR_BAD_FILE_CONTENTS;
  return COMP_MSG_ERR_OK;
  }
  uval = c_strtoul(buffer+2, &endPtr, 10);
  numEntries = (uint8_t)uval;
  idx = 0;
  while(idx < numEntries) {
    result = compMsgMsgDesc->readLine(compMsgMsgDesc, &buffer, &lgth);
    checkErrOK(result);
    if (lgth == 0) {
      return COMP_MSG_ERR_TOO_FEW_FILE_LINES;
    }
    buffer[lgth] = 0;
    cp = buffer;
    // wifiFieldName
    fieldNameStr = cp;
    result = compMsgMsgDesc->getStrFromLine(self, cp, &ep, &isEnd);
    checkErrOK(result);
    checkIsEnd(isEnd);
    cp = ep;

    // wifiFieldValue
    fieldValueStr = cp;
    result = compMsgMsgDesc->getStrFromLine(self, cp, &ep, &isEnd);
    checkErrOK(result);
    if (ep == cp + 1) {
      // empty value
      result = self->compMsgWifiData->setWifiValue(self, fieldNameStr, 0, "");
    } else {
      uval = c_strtoul(fieldValueStr, &endPtr, 10);
      if ((endPtr - (char *)fieldValueStr) == c_strlen(fieldValueStr)) {
        if (c_strlen(fieldValueStr) > 10) {
          // seems to be a password key, so use the stringValue
          result = self->compMsgWifiData->setWifiValue(self, fieldNameStr, 0, fieldValueStr);
        } else {
          result = self->compMsgWifiData->setWifiValue(self, fieldNameStr, uval, NULL);
        }
      } else {
        result = self->compMsgWifiData->setWifiValue(self, fieldNameStr, 0, fieldValueStr);
      }
    }
    if (!isEnd) {
      return COMP_MSG_ERR_FUNNY_EXTRA_FIELDS;
    }
    checkErrOK(result);
    idx++;
  }
#undef checkErrOK
#define checkErrOK(result) if(result != COMP_MSG_ERR_OK) return result
  result = compMsgMsgDesc->closeFile(compMsgMsgDesc);
  checkErrOK(result);
  return COMP_MSG_ERR_OK;
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
#define checkErrOK(result) if(result != COMP_MSG_ERR_OK) { compMsgMsgDesc->closeFile(compMsgMsgDesc); return result; }
  result = compMsgMsgDesc->readLine(compMsgMsgDesc, &buffer, &lgth);
  checkErrOK(result);
  buffer[lgth] = 0;
  if ((lgth < 4) || (buffer[0] != '#')) {
     return COMP_MSG_ERR_BAD_FILE_CONTENTS;
  return COMP_MSG_ERR_OK;
  }
  uval = c_strtoul(buffer+2, &endPtr, 10);
  numEntries = (uint8_t)uval;
  idx = 0;
  while(idx < numEntries) {
    result = compMsgMsgDesc->readLine(compMsgMsgDesc, &buffer, &lgth);
    checkErrOK(result);
    if (lgth == 0) {
      return COMP_MSG_ERR_TOO_FEW_FILE_LINES;
    }
    buffer[lgth] = 0;
    cp = buffer;
    // wifiFieldName
    fieldNameStr = cp;
    result = compMsgMsgDesc->getStrFromLine(self, cp, &ep, &isEnd);
    checkErrOK(result);
    checkIsEnd(isEnd);
    cp = ep;

    // wifiFieldValue
    fieldValueStr = cp;
    result = compMsgMsgDesc->getStrFromLine(self, cp, &ep, &isEnd);
    checkErrOK(result);
    uval = c_strtoul(fieldValueStr, &endPtr, 10);
    if ((endPtr - (char *)fieldValueStr) == c_strlen(fieldValueStr)) {
      if (c_strlen(fieldValueStr) > 10) {
        // seems to be a password key, so use the stringValue
        result = self->compMsgWifiData->setWifiValue(self, fieldNameStr, 0, fieldValueStr);
      } else {
        result = self->compMsgWifiData->setWifiValue(self, fieldNameStr, uval, NULL);
      }
    } else {
      result = self->compMsgWifiData->setWifiValue(self, fieldNameStr, 0, fieldValueStr);
    }
    if (!isEnd) {
      return COMP_MSG_ERR_FUNNY_EXTRA_FIELDS;
    }
    checkErrOK(result);
    idx++;
  }
#undef checkErrOK
#define checkErrOK(result) if(result != COMP_MSG_ERR_OK) return result
  result = compMsgMsgDesc->closeFile(compMsgMsgDesc);
  checkErrOK(result);
  return COMP_MSG_ERR_OK;
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
  return COMP_MSG_ERR_HEADER_NOT_FOUND;
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

  COMP_MSG_DBG(self, "E", 2, "getMsgPartsFromHeaderPart1\n");
  compMsgData = self->compMsgData;
  compMsgMsgDesc = self->compMsgMsgDesc;
  self->compMsgData->currHdr = hdr;
  os_sprintf(fileName, "CompDesc%c%c.txt", (hdr->hdrU16CmdKey>>8)&0xFF, hdr->hdrU16CmdKey&0xFF);
  COMP_MSG_DBG(self, "E", 2, "file: %s\n", fileName);
  result = compMsgMsgDesc->openFile(compMsgMsgDesc, fileName, "r");
  checkErrOK(result);
#undef checkErrOK
#define checkErrOK(result) if(result != COMP_MSG_ERR_OK) { compMsgMsgDesc->closeFile(compMsgMsgDesc); return result; }
  result = compMsgMsgDesc->readLine(compMsgMsgDesc, &buffer, &lgth);
  checkErrOK(result);
  buffer[lgth] = 0;
  cp = buffer;
  result = compMsgMsgDesc->getStrFromLine(self, cp, &ep, &isEnd);
  checkErrOK(result);
  if (isEnd) {
    return COMP_MSG_ERR_BAD_FILE_CONTENTS;
  }
  if (cp[0] != '#') {
    return COMP_MSG_ERR_BAD_FILE_CONTENTS;
  }
  cp = ep;
  result = compMsgData->deleteMsgDescParts(self);
  checkErrOK(result);
  result = compMsgMsgDesc->getIntFromLine(self, cp, &uval, &ep, &isEnd);
  checkErrOK(result);
  compMsgData->maxMsgDescParts = (uint8_t)uval;
  cp = ep;
  if (self->compMsgData->prepareValuesCbName != NULL) {
    os_free(self->compMsgData->prepareValuesCbName);
    self->compMsgData->prepareValuesCbName = NULL;
  }
  if (!isEnd) {
    result = compMsgMsgDesc->getStrFromLine(self, cp, &ep, &isEnd);
    checkErrOK(result);
    compMsgData->prepareValuesCbName = os_zalloc(c_strlen(cp) + 1);
    checkAllocOK(compMsgData->prepareValuesCbName);
    c_memcpy(compMsgData->prepareValuesCbName, cp, c_strlen(cp));
    compMsgData->prepareValuesCbName[c_strlen(cp)] = '\0';
    COMP_MSG_DBG(self, "E", 2, "prepareValuesCbName: %s\n", compMsgData->prepareValuesCbName);
  }
  if (!isEnd) {
    return COMP_MSG_ERR_FUNNY_EXTRA_FIELDS;
  }
  compMsgData->msgDescParts = os_zalloc(sizeof(msgDescPart_t) * compMsgData->maxMsgDescParts);
  checkAllocOK(compMsgData->msgDescParts);
  numRows = 0;
  idx = 0;
  while(idx < compMsgData->maxMsgDescParts) {
    result = compMsgMsgDesc->readLine(compMsgMsgDesc, &buffer, &lgth);
    checkErrOK(result);
    if (lgth == 0) {
      return COMP_MSG_ERR_TOO_FEW_FILE_LINES;
    }
    msgDescPart = &compMsgData->msgDescParts[compMsgData->numMsgDescParts++];
    buffer[lgth] = 0;
    cp = buffer;
    // fieldName
    fieldNameStr = cp;
    result = compMsgMsgDesc->getStrFromLine(self, cp, &ep, &isEnd);
    checkErrOK(result);
    checkIsEnd(isEnd);
    cp = ep;
    msgDescPart->fieldNameStr = os_zalloc(c_strlen(fieldNameStr)+ 1);
    checkAllocOK(msgDescPart->fieldNameStr);
    c_memcpy(msgDescPart->fieldNameStr, fieldNameStr, c_strlen(fieldNameStr));

    // fieldType
    fieldTypeStr = cp;
    result = compMsgMsgDesc->getStrFromLine(self, cp, &ep, &isEnd);
    checkErrOK(result);
    checkIsEnd(isEnd);
    cp = ep;
    msgDescPart->fieldTypeStr = os_zalloc(c_strlen(fieldTypeStr)+ 1);
    checkAllocOK(msgDescPart->fieldTypeStr);
    c_memcpy(msgDescPart->fieldTypeStr, fieldTypeStr, c_strlen(fieldTypeStr));

    // fieldLgth
    fieldLgthStr = cp;
    result = compMsgMsgDesc->getStrFromLine(self, cp, &ep, &isEnd);
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
      result = compMsgMsgDesc->getStrFromLine(self, cp, &ep, &isEnd);
      checkErrOK(result);
      result = self->compMsgUtil->getFieldValueCallback(self, cp, &msgDescPart->fieldSizeCallback, 0);
      checkErrOK(result);
    }
//self->compMsgDebug->dumpMsgDescPart(self, msgDescPart);
    if (!isEnd) {
      return COMP_MSG_ERR_FUNNY_EXTRA_FIELDS;
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
  result = compMsgMsgDesc->getStrFromLine(self, cp, &ep, &isEnd);
  checkErrOK(result);
  if (isEnd) {
    return COMP_MSG_ERR_BAD_FILE_CONTENTS;
  }
  if (cp[0] != '#') {
    return COMP_MSG_ERR_BAD_FILE_CONTENTS;
  }
  cp = ep;
  result = compMsgData->deleteMsgValParts(self);
  checkErrOK(result);
  result = compMsgMsgDesc->getIntFromLine(self, cp, &uval, &ep, &isEnd);
  checkErrOK(result);
  self->compMsgData->maxMsgValParts = (uint8_t)uval;
  cp = ep;
  if (!isEnd) {
    return COMP_MSG_ERR_FUNNY_EXTRA_FIELDS;
  }
  compMsgData->msgValParts = os_zalloc(sizeof(msgValPart_t) * compMsgData->maxMsgValParts);
  checkAllocOK(compMsgData->msgValParts);
  idx = 0;
  while(idx < compMsgData->maxMsgValParts) {
    result = compMsgMsgDesc->readLine(compMsgMsgDesc, &buffer, &lgth);
    checkErrOK(result);
    if (lgth == 0) {
      return COMP_MSG_ERR_TOO_FEW_FILE_LINES;
    }
    msgValPart = &compMsgData->msgValParts[compMsgData->numMsgValParts++];
    buffer[lgth] = 0;
    cp = buffer;
    // fieldName
    fieldNameStr = cp;
    result = compMsgMsgDesc->getStrFromLine(self, cp, &ep, &isEnd);
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
    result = compMsgMsgDesc->getStrFromLine(self, cp, &ep, &isEnd);
    checkErrOK(result);
    msgValPart->fieldValueStr = os_zalloc(c_strlen(fieldValueStr)+ 1);
    checkAllocOK(msgValPart->fieldValueStr);
    c_memcpy(msgValPart->fieldValueStr, fieldValueStr, c_strlen(fieldValueStr));
    uval = c_strtoul(fieldValueStr, &endPtr, 10);
    if ((endPtr - (char *)fieldValueStr) == c_strlen(fieldValueStr)) {
      msgValPart->fieldFlags |= COMP_DISP_DESC_VALUE_IS_NUMBER;
      msgValPart->fieldValue = (uint32_t)uval;
    }
    if (ets_strncmp(fieldValueStr, "@get", 4) == 0) {
      result = self->compMsgUtil->getFieldValueCallback(self, fieldValueStr, &msgValPart->fieldValueCallback, msgValPart->fieldValueCallbackType);
      if (result != COMP_MSG_ERR_OK) {
        COMP_MSG_DBG(self, "E", 2, "WARNING: fieldValueCallback %s for field: %s not found\n", fieldValueStr, fieldNameStr);
      }
//      checkErrOK(result);
    }
    if (ets_strncmp(fieldValueStr, "@run",4) == 0) {
      msgValPart->fieldValueActionCb = msgValPart->fieldValueStr;
    }
//self->compMsgDebug->dumpMsgValPart(self, msgValPart);
    if (!isEnd) {
      return COMP_MSG_ERR_FUNNY_EXTRA_FIELDS;
    }
    idx++;
  }
#undef checkErrOK
#define checkErrOK(result) if(result != COMP_MSG_ERR_OK) return result
  result = compMsgMsgDesc->closeFile(compMsgMsgDesc);
  COMP_MSG_DBG(self, "E", 2, "getMsgPartsFromHeaderPart9 res: %d", result);
  checkErrOK(result);
  COMP_MSG_DBG(self, "E", 2, "heap2: %d", system_get_free_heap_size());

  return COMP_MSG_ERR_OK;
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
#define checkErrOK(result) if(result != COMP_MSG_ERR_OK) { compMsgMsgDesc->closeFile(compMsgMsgDesc); return result; }
  result = compMsgMsgDesc->readLine(compMsgMsgDesc, &buffer, &lgth);
  checkErrOK(result);
  buffer[lgth] = 0;
  if ((lgth < 4) || (buffer[0] != '#')) {
     return COMP_MSG_ERR_BAD_FILE_CONTENTS;
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
      return COMP_MSG_ERR_TOO_FEW_FILE_LINES;
    }
    buffer[lgth] = 0;
    cp = buffer;
    msgKeyValueDescPart = &self->msgKeyValueDescParts[self->numMsgKeyValueDescParts];
    // fieldName
    fieldNameStr = cp;
    result = compMsgMsgDesc->getStrFromLine(self, cp, &ep, &isEnd);
    checkErrOK(result);
    msgKeyValueDescPart->keyNameStr = os_zalloc(c_strlen(fieldNameStr) + 1);
    checkAllocOK(msgKeyValueDescPart->keyNameStr);
    c_memcpy(msgKeyValueDescPart->keyNameStr, fieldNameStr, c_strlen(fieldNameStr) + 1);
    checkIsEnd(isEnd);
    cp = ep;

    // fieldValue
    fieldValueStr = cp;
    result = compMsgMsgDesc->getIntFromLine(self, cp, &uval, &ep, &isEnd);
    checkErrOK(result);
    msgKeyValueDescPart->keyId = (uint16_t)uval;
    checkIsEnd(isEnd);
    cp = ep;

    // fieldType
    fieldTypeStr = cp;
    result = compMsgMsgDesc->getStrFromLine(self, cp, &ep, &isEnd);
    checkErrOK(result);
    result = self->compMsgTypesAndNames->getFieldTypeIdFromStr(self->compMsgTypesAndNames, fieldTypeStr, &fieldTypeId);
    checkErrOK(result);
    msgKeyValueDescPart->keyType = fieldTypeId;
    cp = ep;

    // fieldLength 
    fieldTypeStr = cp;
    result = compMsgMsgDesc->getIntFromLine(self, cp, &uval, &ep, &isEnd);
    checkErrOK(result);
    msgKeyValueDescPart->keyLgth = (uint16_t)uval;
    COMP_MSG_DBG(self, "E", 2, "field: %s Id: %d type: %d length: %d\n", msgKeyValueDescPart->keyNameStr, msgKeyValueDescPart->keyId, msgKeyValueDescPart->keyType, msgKeyValueDescPart->keyLgth);
    if (!isEnd) {
      return COMP_MSG_ERR_FUNNY_EXTRA_FIELDS;
    }
    cp = ep;
    self->numMsgKeyValueDescParts++;
    idx++;
  }
#undef checkErrOK
#define checkErrOK(result) if(result != COMP_MSG_ERR_OK) return result
  result = compMsgMsgDesc->closeFile(compMsgMsgDesc);
  checkErrOK(result);
  COMP_MSG_DBG(self, "E", 2, "getWifiKeyValueKeys done\n");
  return COMP_MSG_ERR_OK;
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
#define checkErrOK(result) if(result != COMP_MSG_ERR_OK) { compMsgMsgDesc->closeFile(compMsgMsgDesc); return result; }
  result = compMsgMsgDesc->readLine(compMsgMsgDesc, &buffer, &lgth);
  checkErrOK(result);
  buffer[lgth] = 0;
  if ((lgth < 4) || (buffer[0] != '#')) {
     return COMP_MSG_ERR_BAD_FILE_CONTENTS;
  }
  uval = c_strtoul(buffer+2, &endPtr, 10);
  numEntries = (uint8_t)uval;
  self->numFieldsToSave = 0;
  self->maxFieldsToSave = numEntries;
  self->fieldsToSave = os_zalloc(numEntries * sizeof(fieldsToSave_t));
  checkAllocOK(self->fieldsToSave);
  idx = 0;
  while(idx < numEntries) {
    result = compMsgMsgDesc->readLine(compMsgMsgDesc, &buffer, &lgth);
    checkErrOK(result);
    if (lgth == 0) {
      return COMP_MSG_ERR_TOO_FEW_FILE_LINES;
    }
    buffer[lgth] = 0;
    cp = buffer;
    fieldsToSave = &self->fieldsToSave[self->numFieldsToSave];
    // fieldName
    fieldNameStr = cp;
    result = compMsgMsgDesc->getStrFromLine(self, cp, &ep, &isEnd);
    checkErrOK(result);
    fieldsToSave->fieldNameStr = os_zalloc(c_strlen(fieldNameStr) + 1);
    checkAllocOK(fieldsToSave->fieldNameStr);
    c_memcpy(fieldsToSave->fieldNameStr, fieldNameStr, c_strlen(fieldNameStr) + 1);
    result = self->compMsgTypesAndNames->getFieldNameIdFromStr(self->compMsgTypesAndNames, fieldNameStr, &fieldsToSave->fieldNameId, COMP_MSG_INCR);
    checkErrOK(result);
    if (!isEnd) {
      return COMP_MSG_ERR_FUNNY_EXTRA_FIELDS;
    }
    self->numFieldsToSave++;
    idx++;
  }
#undef checkErrOK
#define checkErrOK(result) if(result != COMP_MSG_ERR_OK) return result
  result = compMsgMsgDesc->closeFile(compMsgMsgDesc);
  checkErrOK(result);
  COMP_MSG_DBG(self, "E", 2, "getFieldsToSave done\n");
  return COMP_MSG_ERR_OK;
}

// ================================= getWifiKeyValueKeys ====================================

static uint8_t getWifiKeyValueKeys (compMsgDispatcher_t *self, uint8_t *fileName) {
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
  uint16_t keyValueKey;
  uint8_t fieldTypeId;
  compMsgMsgDesc_t *compMsgMsgDesc;
  compMsgWifiData_t *compMsgWifiData;

  compMsgMsgDesc = self->compMsgMsgDesc;
  compMsgWifiData = self->compMsgWifiData;
  result = compMsgMsgDesc->openFile(compMsgMsgDesc, fileName, "r");
  checkErrOK(result);
#undef checkErrOK
#define checkErrOK(result) if(result != COMP_MSG_ERR_OK) { compMsgMsgDesc->closeFile(compMsgMsgDesc); return result; }
  result = compMsgMsgDesc->readLine(compMsgMsgDesc, &buffer, &lgth);
  checkErrOK(result);
  buffer[lgth] = 0;
  if ((lgth < 4) || (buffer[0] != '#')) {
     return COMP_MSG_ERR_BAD_FILE_CONTENTS;
  }
  uval = c_strtoul(buffer+2, &endPtr, 10);
  numEntries = (uint8_t)uval;
  idx = 0;
  while(idx < numEntries) {
    result = compMsgMsgDesc->readLine(compMsgMsgDesc, &buffer, &lgth);
    checkErrOK(result);
    if (lgth == 0) {
      return COMP_MSG_ERR_TOO_FEW_FILE_LINES;
    }
    buffer[lgth] = 0;
    cp = buffer;
    // fieldName
    fieldNameStr = cp;
    result = compMsgMsgDesc->getStrFromLine(self, cp, &ep, &isEnd);
    checkErrOK(result);
    checkIsEnd(isEnd);
    cp = ep;

    // fieldValue
    fieldValueStr = cp;
    result = compMsgMsgDesc->getIntFromLine(self, cp, &uval, &ep, &isEnd);
    checkErrOK(result);
    result = self->compMsgWifiData->keyValueStr2KeyValueId(fieldNameStr + c_strlen("@key_"), &keyValueKey);
    if (result != COMP_MSG_ERR_OK) {
      // not a key the Wifi is handling (normally cloud keys)
      idx++;
      continue;
    }
    checkErrOK(result);
    switch (keyValueKey) {
    case KEY_VALUE_KEY_BSSID:
      compMsgWifiData->keyValueInfo.key_bssid = (uint16_t)uval;
      break;
    case KEY_VALUE_KEY_SSID:
      compMsgWifiData->keyValueInfo.key_ssid = (uint16_t)uval;
      break;
    case KEY_VALUE_KEY_CHANNEL:
      compMsgWifiData->keyValueInfo.key_channel = (uint16_t)uval;
      break;
    case KEY_VALUE_KEY_RSSI:
      compMsgWifiData->keyValueInfo.key_rssi = (uint16_t)uval;
      break;
    case KEY_VALUE_KEY_AUTH_MODE:
      compMsgWifiData->keyValueInfo.key_authmode = (uint16_t)uval;
      break;
    case KEY_VALUE_KEY_IS_HIDDEN:
      compMsgWifiData->keyValueInfo.key_freq_offset = (uint16_t)uval;
      break;
    case KEY_VALUE_KEY_FREQ_OFFSET:
      compMsgWifiData->keyValueInfo.key_freqcal_val = (uint16_t)uval;
      break;
    case KEY_VALUE_KEY_FREQ_CAL_VAL:
      compMsgWifiData->keyValueInfo.key_is_hidden = (uint16_t)uval;
      break;
    case KEY_VALUE_KEY_CLIENT_SSID:
      compMsgWifiData->keyValueInfo.key_clientSsid = (uint16_t)uval;
      break;
    case KEY_VALUE_KEY_CLIENT_PASSWD:
      compMsgWifiData->keyValueInfo.key_clientPasswd = (uint16_t)uval;
      break;
    case KEY_VALUE_KEY_CLIENT_IP_ADDR:
      compMsgWifiData->keyValueInfo.key_clientIPAddr = (uint16_t)uval;
      break;
    case   KEY_VALUE_KEY_CLIENT_PORT:
      compMsgWifiData->keyValueInfo.key_clientPort = (uint16_t)uval;
      break;
    case KEY_VALUE_KEY_CLIENT_STATUS:
      compMsgWifiData->keyValueInfo.key_clientStatus = (uint16_t)uval;
      break;
    case KEY_VALUE_KEY_SEQ_NUM:
      compMsgWifiData->keyValueInfo.key_seqNum = (uint16_t)uval;
      break;
    case KEY_VALUE_KEY_MAC_ADDR:
      compMsgWifiData->keyValueInfo.key_MACAddr = (uint16_t)uval;
      break;
    case KEY_VALUE_KEY_MACHINE_STATE:
      compMsgWifiData->keyValueInfo.key_machineState = (uint16_t)uval;
      break;
    case KEY_VALUE_KEY_FIRMWARE_MAIN_BOARD:
      compMsgWifiData->keyValueInfo.key_firmwareMainBoard = (uint16_t)uval;
      break;
    case KEY_VALUE_KEY_FIRMWARE_DISPLAY_BOARD:
      compMsgWifiData->keyValueInfo.key_firmwareDisplayBoard = (uint16_t)uval;
      break;
    case KEY_VALUE_KEY_FIRMWARE_WIFI_MODULE:
      compMsgWifiData->keyValueInfo.key_firmwareWifiModule = (uint16_t)uval;
      break;
    case KEY_VALUE_KEY_LAST_ERROR:
      compMsgWifiData->keyValueInfo.key_lastError = (uint16_t)uval;
      break;
    case KEY_VALUE_KEY_CASING_USE_LIST:
      compMsgWifiData->keyValueInfo.key_casingUseList = (uint16_t)uval;
      break;
    case KEY_VALUE_KEY_CASING_STATISTIC_LIST:
      compMsgWifiData->keyValueInfo.key_casingStatisticList = (uint16_t)uval;
      break;
    case KEY_VALUE_KEY_DAT_AND_TIME:
      compMsgWifiData->keyValueInfo.key_dataAndTime = (uint16_t)uval;
      break;
    }
    checkIsEnd(isEnd);
    cp = ep;

    // fieldType
    fieldTypeStr = cp;
    result = compMsgMsgDesc->getStrFromLine(self, cp, &ep, &isEnd);
    checkErrOK(result);
    result = self->compMsgTypesAndNames->getFieldTypeIdFromStr(self->compMsgTypesAndNames, fieldTypeStr, &fieldTypeId);
    checkErrOK(result);
    switch (keyValueKey) {
    case KEY_VALUE_KEY_BSSID:
      compMsgWifiData->bssScanTypes.bssidType = (uint8_t)fieldTypeId;
      compMsgWifiData->keyValueInfo.key_type_bssid = (uint8_t)fieldTypeId;
      break;
    case KEY_VALUE_KEY_SSID:
      compMsgWifiData->bssScanTypes.ssidType = (uint8_t)fieldTypeId;
      compMsgWifiData->keyValueInfo.key_type_ssid = (uint8_t)fieldTypeId;
      break;
    case KEY_VALUE_KEY_CHANNEL:
      compMsgWifiData->bssScanTypes.channelType = (uint8_t)fieldTypeId;
      compMsgWifiData->keyValueInfo.key_type_channel = (uint8_t)fieldTypeId;
      break;
    case KEY_VALUE_KEY_RSSI:
      compMsgWifiData->bssScanTypes.rssiType = (uint8_t)fieldTypeId;
      compMsgWifiData->keyValueInfo.key_type_rssi = (uint8_t)fieldTypeId;
      break;
    case KEY_VALUE_KEY_AUTH_MODE:
      compMsgWifiData->bssScanTypes.authmodeType = (uint8_t)fieldTypeId;
      compMsgWifiData->keyValueInfo.key_type_authmode = (uint8_t)fieldTypeId;
      break;
    case KEY_VALUE_KEY_FREQ_OFFSET:
      compMsgWifiData->bssScanTypes.freq_offsetType = (uint8_t)fieldTypeId;
      compMsgWifiData->keyValueInfo.key_type_freq_offset = (uint8_t)fieldTypeId;
      break;
    case KEY_VALUE_KEY_FREQ_CAL_VAL:
      compMsgWifiData->bssScanTypes.freqcal_valType = (uint8_t)fieldTypeId;
      compMsgWifiData->keyValueInfo.key_type_freqcal_val = (uint8_t)fieldTypeId;
      break;
    case KEY_VALUE_KEY_IS_HIDDEN:
      compMsgWifiData->bssScanTypes.is_hiddenType = (uint8_t)fieldTypeId;
      compMsgWifiData->keyValueInfo.key_type_is_hidden = (uint8_t)fieldTypeId;
      break;
    case KEY_VALUE_KEY_CLIENT_SSID:
      compMsgWifiData->keyValueInfo.key_type_clientSsid = (uint8_t)fieldTypeId;
      break;
    case KEY_VALUE_KEY_CLIENT_PASSWD:
      compMsgWifiData->keyValueInfo.key_type_clientPasswd = (uint8_t)fieldTypeId;
      break;
    case KEY_VALUE_KEY_CLIENT_IP_ADDR:
      compMsgWifiData->keyValueInfo.key_type_clientIPAddr = (uint8_t)fieldTypeId;
      break;
    case KEY_VALUE_KEY_CLIENT_PORT:
      compMsgWifiData->keyValueInfo.key_type_clientPort = (uint8_t)fieldTypeId;
      break;
    case KEY_VALUE_KEY_CLIENT_STATUS:
      compMsgWifiData->keyValueInfo.key_type_clientStatus = (uint8_t)fieldTypeId;
      break;
    case KEY_VALUE_KEY_SEQ_NUM:
      compMsgWifiData->keyValueInfo.key_type_seqNum = (uint8_t)fieldTypeId;
      break;
    case KEY_VALUE_KEY_MAC_ADDR:
      compMsgWifiData->keyValueInfo.key_type_MACAddr = (uint8_t)fieldTypeId;
      break;
    case KEY_VALUE_KEY_MACHINE_STATE:
      compMsgWifiData->keyValueInfo.key_type_machineState = (uint8_t)fieldTypeId;
      break;
    case KEY_VALUE_KEY_FIRMWARE_MAIN_BOARD:
      compMsgWifiData->keyValueInfo.key_type_firmwareMainBoard = (uint8_t)fieldTypeId;
      break;
    case KEY_VALUE_KEY_FIRMWARE_DISPLAY_BOARD:
      compMsgWifiData->keyValueInfo.key_type_firmwareDisplayBoard = (uint8_t)fieldTypeId;
      break;
    case KEY_VALUE_KEY_FIRMWARE_WIFI_MODULE:
      compMsgWifiData->keyValueInfo.key_type_firmwareWifiModule = (uint8_t)fieldTypeId;
      break;
    case KEY_VALUE_KEY_LAST_ERROR:
      compMsgWifiData->keyValueInfo.key_type_lastError = (uint8_t)fieldTypeId;
      break;
    case KEY_VALUE_KEY_CASING_USE_LIST:
      compMsgWifiData->keyValueInfo.key_type_casingUseList = (uint8_t)fieldTypeId;
      break;
    case KEY_VALUE_KEY_CASING_STATISTIC_LIST:
      compMsgWifiData->keyValueInfo.key_type_casingStatisticList = (uint8_t)fieldTypeId;
      break;
    case KEY_VALUE_KEY_DAT_AND_TIME:
      compMsgWifiData->keyValueInfo.key_type_dataAndTime = (uint8_t)fieldTypeId;
      break;
    }
    cp = ep;

    // fieldLength not needed for Wifi module
    fieldTypeStr = cp;
    result = compMsgMsgDesc->getStrFromLine(self, cp, &ep, &isEnd);
    checkErrOK(result);
    COMP_MSG_DBG(self, "E", 2, "field: %s length: %s\n", fieldNameStr, cp);
    if (!isEnd) {
      return COMP_MSG_ERR_FUNNY_EXTRA_FIELDS;
    }
    cp = ep;
    idx++;
  }
#undef checkErrOK
#define checkErrOK(result) if(result != COMP_MSG_ERR_OK) return result
  result = compMsgMsgDesc->closeFile(compMsgMsgDesc);
  checkErrOK(result);
  COMP_MSG_DBG(self, "E", 2, "getWifiKeyValueKeys done\n");
  return COMP_MSG_ERR_OK;
}

#undef checkIsEnd

// ================================= compMsgMsgDescInit ====================================

uint8_t compMsgMsgDescInit(compMsgDispatcher_t *self) {
  self->compMsgMsgDesc->openFile = &openFile;
  self->compMsgMsgDesc->closeFile = &closeFile;
  self->compMsgMsgDesc->flushFile = &flushFile;
  self->compMsgMsgDesc->readLine = &readLine;
  self->compMsgMsgDesc->writeLine = &writeLine;
  self->compMsgMsgDesc->getIntFromLine = &getIntFromLine;
  self->compMsgMsgDesc->getStrFromLine = &getStrFromLine;
  self->compMsgMsgDesc->getHeaderFieldsFromLine = &getHeaderFieldsFromLine;
  self->compMsgMsgDesc->readActions = &readActions;
  self->compMsgMsgDesc->readModuleValues = &readModuleValues;
  self->compMsgMsgDesc->readWifiValues = &readWifiValues;
  self->compMsgMsgDesc->readHeadersAndSetFlags = &readHeadersAndSetFlags;
  self->compMsgMsgDesc->getMsgPartsFromHeaderPart = &getMsgPartsFromHeaderPart;
  self->compMsgMsgDesc->getHeaderFromUniqueFields = &getHeaderFromUniqueFields;
  self->compMsgMsgDesc->getMsgKeyValueDescParts = &getMsgKeyValueDescParts;
  self->compMsgMsgDesc->getFieldsToSave = &getFieldsToSave;
  self->compMsgMsgDesc->getWifiKeyValueKeys = &getWifiKeyValueKeys;
  return COMP_MSG_ERR_OK;
}

// ================================= newCompMsgMsgDesc ====================================

compMsgMsgDesc_t *newCompMsgMsgDesc() {
  compMsgMsgDesc_t *compMsgMsgDesc = os_zalloc(sizeof(compMsgMsgDesc_t));
  if (compMsgMsgDesc == NULL) {
    return NULL;
  }
  compMsgMsgDescId++;
  compMsgMsgDesc->id = compMsgMsgDescId;
  return compMsgMsgDesc;
}

// ================================= freeCompMsgMsgDesc ====================================

void freeCompMsgMsgDesc(compMsgMsgDesc_t *compMsgMsgDesc) {
}

