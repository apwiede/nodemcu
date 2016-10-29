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
    return COMP_MSG_DESC_FILE_NOT_OPENED;
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
    return COMP_MSG_DESC_FILE_NOT_OPENED;
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
    return COMP_MSG_DESC_FILE_NOT_OPENED;
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

  ets_printf("dumpHeaderPart:\n");
  ets_printf("headerPart1: from: 0x%04x to: 0x%04x totalLgth: %d GUID: %s srcId: %d u16CmdKey: 0x%04x %c%c u16CmdLgth: 0x%04x u16Crc: 0x%04x\n", hdr->hdrFromPart, hdr->hdrToPart, hdr->hdrTotalLgth, hdr->hdrGUID, hdr->hdrSrcId,
 hdr->hdrU16CmdKey, (hdr->hdrU16CmdKey>>8)&0xFF, hdr->hdrU16CmdKey&0xFF, hdr->hdrU16CmdLgth, hdr->hdrU16Crc);
  ets_printf("headerPart2: u8CmdLgth: %d u8Crc: 0x%02x offset: %d \n", hdr->hdrU8CmdLgth, hdr->hdrU8Crc, hdr->hdrOffset);
  ets_printf("headerPart3: enc: %c handleType: %c\n", hdr->hdrEncryption, hdr->hdrHandleType);
  ets_printf("hdrFlags: 0x%04x", hdr->hdrFlags);
  if (hdr->hdrFlags & COMP_DISP_U16_CMD_KEY) {
    ets_printf(" COMP_DISP_U16_CMD_KEY");
  }
  if (hdr->hdrFlags & COMP_DISP_U0_CMD_LGTH) {
    ets_printf(" COMP_DISP_U0_CMD_LGTH");
  }
  if (hdr->hdrFlags & COMP_DISP_U8_CMD_LGTH) {
    ets_printf(" COMP_DISP_U8_CMD_LGTH");
  }
  if (hdr->hdrFlags & COMP_DISP_U16_CMD_LGTH) {
    ets_printf(" COMP_DISP_U16_CMD_LGTH");
  }
  if (hdr->hdrFlags & COMP_DISP_U0_CRC) {
    ets_printf(" COMP_DISP_U0_CRC");
  }
  if (hdr->hdrFlags & COMP_DISP_U8_CRC) {
    ets_printf(" COMP_DISP_U8_CRC");
  }
  if (hdr->hdrFlags & COMP_DISP_U16_CRC) {
    ets_printf(" COMP_DISP_U16_CRC");
  }
  ets_printf("\n");
  ets_printf("hdr fieldSequence\n");
  idx = 0;
  while (idx < COMP_DISP_MAX_SEQUENCE) {
    if (hdr->fieldSequence[idx] == 0) {
      break;
    }
    ets_printf(" %d 0x%04x", idx, hdr->fieldSequence[idx]);
    if (hdr->fieldSequence[idx] & COMP_DISP_U16_DST) {
      ets_printf(" COMP_DISP_U16_DST");
    }
    if (hdr->fieldSequence[idx] & COMP_DISP_U16_SRC) {
      ets_printf(" COMP_DISP_U16_SRC");
    }
    if (hdr->fieldSequence[idx] & COMP_DISP_U16_TOTAL_LGTH) {
      ets_printf(" COMP_DISP_U16_TOTAL_LGTH");
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
      ets_printf(" COMP_DISP_U8_CMD_LGTH");
    }
    if (hdr->fieldSequence[idx] & COMP_DISP_U16_CMD_LGTH) {
      ets_printf(" COMP_DISP_U16_CMD_LGTH");
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
    ets_printf("\n");
    idx++;
  }
  ets_printf("\n");
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
ets_printf("getIntFromLine: %s %d %p %p\n", myStr, *uval, cp, endPtr);
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

static uint8_t getHeaderFieldsFromLine(compMsgDataView_t *dataView, msgHeaderInfos_t *hdrInfos, uint8_t *myStr, uint8_t **ep, int *seqIdx) {
  int result;
  bool isEnd;
  long uval;
  uint8_t *cp;
  uint8_t fieldNameId;

//ets_printf("numHeaderParts: %d seqidx: %d\n", hdrInfos->numHeaderParts, *seqIdx);
  cp = myStr;
  result = getIntFromLine(cp, &uval, ep, &isEnd);
  checkErrOK(result);
ets_printf("headerLgth: %d\n", uval);
  hdrInfos->headerLgth = (uint8_t)uval;
  checkIsEnd(isEnd);
  cp = *ep;
  result = getStrFromLine(cp, ep, &isEnd);
  checkErrOK(result);
  if (cp[0] != '@') {
    return COMP_MSG_ERR_NO_SUCH_FIELD;
  }
  result = dataView->getFieldNameIdFromStr(dataView, cp, &fieldNameId, COMP_MSG_NO_INCR);
  checkErrOK(result);
  switch (fieldNameId) {
  case COMP_MSG_SPEC_FIELD_SRC:
    hdrInfos->headerSequence[(*seqIdx)++] = COMP_DISP_U16_SRC;
    hdrInfos->headerFlags |= COMP_DISP_U16_SRC;
    break;
  case COMP_MSG_SPEC_FIELD_DST:
    hdrInfos->headerSequence[(*seqIdx)++] = COMP_DISP_U16_DST;
    hdrInfos->headerFlags |= COMP_DISP_U16_DST;
    break;
  default:
    checkErrOK(COMP_MSG_ERR_NO_SUCH_FIELD);
    break;
  }
  checkIsEnd(isEnd);
  cp = *ep;
  result = getStrFromLine(cp, ep, &isEnd);
  checkErrOK(result);
  if (cp[0] != '@') {
    return COMP_MSG_ERR_NO_SUCH_FIELD;
  }
  result = dataView->getFieldNameIdFromStr(dataView, cp, &fieldNameId, COMP_MSG_NO_INCR);
  checkErrOK(result);
  switch (fieldNameId) {
  case COMP_MSG_SPEC_FIELD_SRC:
    if (hdrInfos->headerFlags & COMP_DISP_U16_SRC) {
      return COMP_MSG_ERR_DUPLICATE_FIELD;
    }
    hdrInfos->headerSequence[(*seqIdx)++] = COMP_DISP_U16_SRC;
    break;
  case COMP_MSG_SPEC_FIELD_DST:
    if (hdrInfos->headerFlags & COMP_DISP_U16_DST) {
      return COMP_MSG_ERR_DUPLICATE_FIELD;
    }
    hdrInfos->headerSequence[(*seqIdx)++] = COMP_DISP_U16_DST;
    break;
  case COMP_MSG_SPEC_FIELD_TOTAL_LGTH:
    hdrInfos->headerSequence[(*seqIdx)++] = COMP_DISP_U16_TOTAL_LGTH;
    break;
  default:
    checkErrOK(COMP_MSG_ERR_NO_SUCH_FIELD);
    break;
  }
  checkIsEnd(isEnd);
  cp = *ep;
  result = getStrFromLine(cp, ep, &isEnd);
  checkErrOK(result);
  if (cp[0] != '@') {
    return COMP_MSG_ERR_NO_SUCH_FIELD;
  }
  result = dataView->getFieldNameIdFromStr(dataView, cp, &fieldNameId, COMP_MSG_NO_INCR);
  checkErrOK(result);
  switch (fieldNameId) {
  case COMP_MSG_SPEC_FIELD_TOTAL_LGTH:
    if (hdrInfos->headerFlags & COMP_DISP_U16_TOTAL_LGTH) {
      return COMP_MSG_ERR_DUPLICATE_FIELD;
    }
    hdrInfos->headerSequence[(*seqIdx)++] = COMP_DISP_U16_TOTAL_LGTH;
    break;
  default:
    checkErrOK(COMP_MSG_ERR_NO_SUCH_FIELD);
    break;
  }
  checkIsEnd(isEnd);
  cp = *ep;
  result = getStrFromLine(cp, ep, &isEnd);
  checkErrOK(result);
  if (cp[0] != '@') {
    return COMP_MSG_ERR_NO_SUCH_FIELD;
  }
  result = dataView->getFieldNameIdFromStr(dataView, cp, &fieldNameId, COMP_MSG_NO_INCR);
  checkErrOK(result);
  switch (fieldNameId) {
  case COMP_MSG_SPEC_FIELD_GUID:
    if (hdrInfos->headerFlags & COMP_DISP_U8_VECTOR_GUID) {
      return COMP_MSG_ERR_DUPLICATE_FIELD;
    }
    hdrInfos->headerSequence[(*seqIdx)++] = COMP_DISP_U8_VECTOR_GUID;
    break;
  default:
    checkErrOK(COMP_MSG_ERR_NO_SUCH_FIELD);
    break;
  }
  checkIsEnd(isEnd);
  cp = *ep;
  result = getStrFromLine(cp, ep, &isEnd);
  checkErrOK(result);
  if (cp[0] != '@') {
    return COMP_MSG_ERR_NO_SUCH_FIELD;
  }
  result = dataView->getFieldNameIdFromStr(dataView, cp, &fieldNameId, COMP_MSG_NO_INCR);
  checkErrOK(result);
  switch (fieldNameId) {
  case COMP_MSG_SPEC_FIELD_SRC_ID:
    if (hdrInfos->headerFlags & COMP_DISP_U16_SRC_ID) {
      return COMP_MSG_ERR_DUPLICATE_FIELD;
    }
    hdrInfos->headerSequence[(*seqIdx)++] = COMP_DISP_U16_SRC_ID;
    break;
  default:
    checkErrOK(COMP_MSG_ERR_NO_SUCH_FIELD);
    break;
  }
  checkIsEnd(isEnd);
  cp = *ep;
  result = getStrFromLine(cp, ep, &isEnd);
  checkErrOK(result);
  if (cp[0] != '@') {
    return COMP_MSG_ERR_NO_SUCH_FIELD;
  }
  result = dataView->getFieldNameIdFromStr(dataView, cp, &fieldNameId, COMP_MSG_NO_INCR);
  checkErrOK(result);
  switch (fieldNameId) {
  case COMP_MSG_SPEC_FIELD_HDR_FILLER:
    if (hdrInfos->headerFlags & COMP_DISP_U8_VECTOR_HDR_FILLER) {
      return COMP_MSG_ERR_DUPLICATE_FIELD;
    }
    hdrInfos->headerSequence[(*seqIdx)++] = COMP_DISP_U8_VECTOR_HDR_FILLER;
    break;
  default:
    checkErrOK(COMP_MSG_ERR_NO_SUCH_FIELD);
    break;
  }
  hdrInfos->seqIdxAfterHeader = *seqIdx;
ets_printf("§Lgth!%d§\n", hdrInfos->lgth);
  if (!isEnd) {
    return COMP_MSG_DESC_ERR_FUNNY_EXTRA_FIELDS;
  }
  return COMP_MSG_ERR_OK;
}
  
#undef checkIsEnd
#define checkIsEnd(val) { hdr->hdrLgth = ep - myStr; if (val) return result; }
#undef checkErrOK
#define checkErrOK(result) if(result != DATA_VIEW_ERR_OK) { self->compMsgMsgDesc->closeFile(self->compMsgMsgDesc); return result; }
// ================================= readHeadersAndSetFlags ====================================

static uint8_t readHeadersAndSetFlags(compMsgDispatcher_t *self, uint8_t *fileName) {
  int result;
  int result2;
  uint8_t numEntries;
  uint8_t fieldNameId;
  uint8_t fieldTypeId;
  char *endPtr;
  uint8_t lgth;
  uint8_t buf[100];
  uint8_t *buffer = buf;
  long ulgth;
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
  headerPart_t *hdr;
  msgHeaderInfos_t *hdrInfos;
  compMsgDataView_t *dataView;
  dataView_t *myDataView;

  dataView = self->compMsgDataView;
  hdrInfos = &self->msgHeaderInfos;
  hdrInfos->currPartIdx = 0;
  result = self->compMsgMsgDesc->openFile(self->compMsgMsgDesc, fileName, "r");
  checkErrOK(result);
  result = self->compMsgMsgDesc->readLine(self->compMsgMsgDesc, &buffer, &lgth);
  checkErrOK(result);
  if ((lgth < 4) || (buffer[0] != '#')) {
     return COMP_DISP_ERR_BAD_FILE_CONTENTS;
  }
  ulgth = c_strtoul(buffer+2, &endPtr, 10);
  numEntries = (uint8_t)ulgth;
  hdrInfos->headerParts = (headerPart_t *)os_zalloc(numEntries * (sizeof(headerPart_t)));
  checkAllocOK(self->msgHeaderInfos.headerParts);
  hdrInfos->numHeaderParts = 0;
  hdrInfos->maxHeaderParts = numEntries;
  hdrInfos->headerFlags = 0;
  // parse header description
  result = self->compMsgMsgDesc->readLine(self->compMsgMsgDesc, &buffer, &lgth);
  checkErrOK(result);
  seqIdx = 0;
  buffer[lgth] = 0;
  myStr = buffer;
  result = self->compMsgMsgDesc->getHeaderFieldsFromLine(dataView, hdrInfos, myStr, &cp, &seqIdx);
  myDataView = newDataView();
  checkAllocOK(myDataView);
  fieldOffset = 0;
  idx = 0;
  while(idx < numEntries) {
    result = self->compMsgMsgDesc->readLine(self->compMsgMsgDesc, &buffer, &lgth);
    checkErrOK(result);
    if (lgth == 0) {
      return COMP_DISP_ERR_TOO_FEW_FILE_LINES;
    }
    hdr = &hdrInfos->headerParts[idx];
    seqIdx2 = 0;
    while (seqIdx2 < seqIdx) {
      hdr->fieldSequence[seqIdx2] = hdrInfos->headerSequence[seqIdx2];
      seqIdx2++;
    }
    hdr->hdrFlags = 0;
    myDataView->data = buffer;
    myDataView->lgth = lgth;
    buffer[lgth] = 0;
    myStr = buffer;
    cp = buffer;
    seqIdx2 = 0;
    result = self->compMsgMsgDesc->getIntFromLine(cp, &ulgth, &ep, &isEnd);
    checkErrOK(result);
    found = 0;
    if (hdrInfos->headerSequence[seqIdx2] & COMP_DISP_U16_SRC) {
      hdr->hdrFromPart = (uint16_t)ulgth;
      hdr->fieldSequence[seqIdx2] = COMP_DISP_U16_SRC;
      found = 1;
    }
    if (hdrInfos->headerSequence[seqIdx2] & COMP_DISP_U16_DST) {
      hdr->hdrToPart = (uint16_t)ulgth;
      hdr->fieldSequence[seqIdx2] = COMP_DISP_U16_DST;
      found = 1;
    }
    if (!found) {
      checkErrOK(COMP_MSG_ERR_FIELD_NOT_FOUND);
    }
    seqIdx2++;
    checkIsEnd(isEnd);
    cp = ep;
    result = self->compMsgMsgDesc->getIntFromLine(cp, &ulgth, &ep, &isEnd);
    checkErrOK(result);
    found = 0;
    if (hdrInfos->headerSequence[seqIdx2] & COMP_DISP_U16_SRC) {
      hdr->fieldSequence[seqIdx2] = COMP_DISP_U16_SRC;
      hdr->hdrFromPart = (uint16_t)ulgth;
      found = 1;
    }
    if (hdrInfos->headerSequence[seqIdx2] & COMP_DISP_U16_DST) {
      hdr->fieldSequence[seqIdx2] = COMP_DISP_U16_DST;
      hdr->hdrToPart = (uint16_t)ulgth;
      found = 1;
    }
    if (hdrInfos->headerSequence[seqIdx2] & COMP_DISP_U16_TOTAL_LGTH) {
      hdr->fieldSequence[seqIdx2] = COMP_DISP_U16_TOTAL_LGTH;
      hdr->hdrTotalLgth = (uint16_t)ulgth;
      found = 1;
    }
    if (!found) {
      result = COMP_MSG_ERR_FIELD_NOT_FOUND;
      checkErrOK(result);
    }
    seqIdx2++;
    checkIsEnd(isEnd);
    cp = ep;
    found = 0;
    result = self->compMsgMsgDesc->getIntFromLine(cp, &ulgth, &ep, &isEnd);
    checkErrOK(result);
    if (hdrInfos->headerSequence[seqIdx2] & COMP_DISP_U16_TOTAL_LGTH) {
      hdr->fieldSequence[seqIdx2] = COMP_DISP_U16_TOTAL_LGTH;
      hdr->hdrTotalLgth = (uint16_t)ulgth;
      found = 1;
    }
    if (!found) {
      result = COMP_MSG_ERR_FIELD_NOT_FOUND;
      checkErrOK(result);
    }
    seqIdx2++;
    checkIsEnd(isEnd);
    // skip over GUID, senderId and hdrFiller
    // we have no info for these fields in ComMsgHeads.txt
    while (seqIdx2 < seqIdx) {
      seqIdx2++;
    }
    cp = ep;
    // extra field lgth 0/<number>
    result = self->compMsgMsgDesc->getIntFromLine(cp, &ulgth, &ep, &isEnd);
    checkErrOK(result);
    hdr->hdrExtraLgth = (uint8_t)ulgth;
    checkIsEnd(isEnd);
    cp = ep;
    // encryption E/N
    result = self->compMsgMsgDesc->getStrFromLine(cp, &ep, &isEnd);
    checkErrOK(result);
    hdr->fieldSequence[seqIdx2] = COMP_DISP_U8_ENCRYPTION;
    hdr->hdrEncryption = cp[0];
    if (cp[0] == 'E') {
//ets_printf("§idx!%d!0x%04x!0x%04x!0x%04x!enc!§", idx, hdr->hdrToPart, hdr->hdrFromPart, hdr->hdrTotalLgth);
      hdr->hdrFlags |= COMP_DISP_IS_ENCRYPTED;
    } else {
//ets_printf("§idx!%d!0x%04x!0x%04x!0x%04x!noenc!§", idx, hdr->hdrToPart, hdr->hdrFromPart, hdr->hdrTotalLgth);
      hdr->hdrFlags |= COMP_DISP_IS_NOT_ENCRYPTED;
    }
    checkIsEnd(isEnd);
    cp = ep;
    // handleType A/G/S/R/U/W/N
    result = self->compMsgMsgDesc->getStrFromLine(cp, &ep, &isEnd);
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
      checkErrOK(COMP_DISP_ERR_BAD_VALUE);
      break;
    }
    checkIsEnd(isEnd);
    cp = ep;
    // type of cmdKey
    result = self->compMsgMsgDesc->getStrFromLine(cp, &ep, &isEnd);
    checkErrOK(result);
    result = dataView->dataView->getFieldTypeIdFromStr(dataView->dataView, cp, &fieldTypeId);
    checkErrOK(result);
    checkIsEnd(isEnd);
    cp = ep;
    // cmdKey
    result = self->compMsgMsgDesc->getStrFromLine(cp, &ep, &isEnd);
    checkErrOK(result);
    switch (fieldTypeId) {
    case DATA_VIEW_FIELD_UINT16_T:
      hdr->fieldSequence[seqIdx2] = COMP_DISP_U16_CMD_KEY;
      hdr->hdrFlags |= COMP_DISP_U16_CMD_KEY;
      hdr->hdrU16CmdKey = (cp[0]<<8)|cp[1];
//ets_printf("§u16CmdKey!0x%04x!§\n", hdr->hdrU16CmdKey);
      self->dispFlags &= ~COMP_MSG_U8_CMD_KEY;
      break;
    default:
      checkErrOK(COMP_DISP_ERR_BAD_FIELD_TYPE);
    }
    checkIsEnd(isEnd);
    cp = ep;
    seqIdx2++;
    // type of cmdLgth
    result = self->compMsgMsgDesc->getStrFromLine(cp, &ep, &isEnd);
    checkErrOK(result);
    result = dataView->dataView->getFieldTypeIdFromStr(dataView->dataView, cp, &fieldTypeId);
    checkErrOK(result);
    switch (fieldTypeId) {
    case DATA_VIEW_FIELD_UINT0_T:
      hdr->fieldSequence[seqIdx2] = COMP_DISP_U0_CMD_LGTH;
      hdr->hdrFlags |= COMP_DISP_U0_CMD_LGTH;
      break;
    case DATA_VIEW_FIELD_UINT8_T:
      hdr->fieldSequence[seqIdx2] = COMP_DISP_U8_CMD_LGTH;
      hdr->hdrFlags |= COMP_DISP_U8_CMD_LGTH;
      break;
    case DATA_VIEW_FIELD_UINT16_T:
      hdr->fieldSequence[seqIdx2] = COMP_DISP_U16_CMD_LGTH;
      hdr->hdrFlags |= COMP_DISP_U16_CMD_LGTH;
      break;
    default:
      checkErrOK(COMP_DISP_ERR_BAD_FIELD_TYPE);
    }
    checkIsEnd(isEnd);
    cp = ep;
    // type of crc
    result = self->compMsgMsgDesc->getStrFromLine(cp, &ep, &isEnd);
    checkErrOK(result);
    result = dataView->dataView->getFieldTypeIdFromStr(dataView->dataView, cp, &fieldTypeId);
    checkErrOK(result);
    switch (fieldTypeId) {
    case DATA_VIEW_FIELD_UINT0_T:
      hdr->fieldSequence[seqIdx2] = COMP_DISP_U0_CRC;
      hdr->hdrFlags |= COMP_DISP_U0_CRC;
      break;
    case DATA_VIEW_FIELD_UINT8_T:
      hdr->fieldSequence[seqIdx2] = COMP_DISP_U8_CRC;
      hdr->hdrFlags |= COMP_DISP_U8_CRC;
      break;
    case DATA_VIEW_FIELD_UINT16_T:
      hdr->fieldSequence[seqIdx2] = COMP_DISP_U16_CRC;
      hdr->hdrFlags |= COMP_DISP_U16_CRC;
      break;
    default:
      checkErrOK(COMP_DISP_ERR_BAD_FIELD_TYPE);
//ets_printf("§flag idx!%d!%s!§", flagIdx, getFlagStr(hdrInfos->headerSequence[flagIdx]));
    }
    if (!isEnd) {
      return COMP_MSG_DESC_ERR_FUNNY_EXTRA_FIELDS;
    }
    hdrInfos->numHeaderParts++;
self->compMsgMsgDesc->dumpHeaderPart(self, hdr);
    idx++;
  }
  os_free(myDataView);
self->compMsgMsgDesc->dumpMsgHeaderInfos(self, hdrInfos);
  result2 = self->compMsgMsgDesc->closeFile(self->compMsgMsgDesc);
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
  compMsgDataView_t *dataView;
  long ulgth;
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

  buffer = buf;
  dataView = self->compMsgDataView;
  result = self->compMsgMsgDesc->openFile(self->compMsgMsgDesc, fileName, "r");
  checkErrOK(result);
#undef checkErrOK
#define checkErrOK(result) if(result != COMP_DISP_ERR_OK) { self->compMsgMsgDesc->closeFile(self->compMsgMsgDesc); return result; }
  result = self->compMsgMsgDesc->readLine(self->compMsgMsgDesc, &buffer, &lgth);
  checkErrOK(result);
  buffer[lgth] = 0;
  if ((lgth < 4) || (buffer[0] != '#')) {
     return COMP_DISP_ERR_BAD_FILE_CONTENTS;
  return COMP_MSG_DESC_ERR_OK;
  }
  ulgth = c_strtoul(buffer+2, &endPtr, 10);
  numEntries = (uint8_t)ulgth;
  idx = 0;
  while(idx < numEntries) {
    u8CmdKey = 0;
    u16CmdKey = 0;
    result = self->compMsgMsgDesc->readLine(self->compMsgMsgDesc, &buffer, &lgth);
    checkErrOK(result);
    if (lgth == 0) {
      return COMP_DISP_ERR_TOO_FEW_FILE_LINES;
    }
    buffer[lgth] = 0;
    cp = buffer;
    // actionName
    actionName = cp;
    result = self->compMsgMsgDesc->getStrFromLine(cp, &ep, &isEnd);
    checkErrOK(result);
    checkIsEnd(isEnd);
    cp = ep;

    // actionMode
    result = self->compMsgMsgDesc->getIntFromLine(cp, &ulgth, &ep, &isEnd);
    checkErrOK(result);
    actionMode = (uint8_t)ulgth;
    checkIsEnd(isEnd);
    cp = ep;

    // type of cmdKey
    result = self->compMsgMsgDesc->getStrFromLine(cp, &ep, &isEnd);
    checkErrOK(result);
    result = dataView->dataView->getFieldTypeIdFromStr(dataView->dataView, cp, &fieldTypeId);
    checkErrOK(result);
    checkIsEnd(isEnd);
    cp = ep;

    // cmdKey
    result = self->compMsgMsgDesc->getStrFromLine(cp, &ep, &isEnd);
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
  result = self->compMsgMsgDesc->closeFile(self->compMsgMsgDesc);
  checkErrOK(result);
  return COMP_MSG_DESC_ERR_OK;
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
  compMsgMsgDesc->readHeadersAndSetFlags = &readHeadersAndSetFlags;
  return compMsgMsgDesc;
}

// ================================= freeCompMsgMsgDesc ====================================

void freeCompMsgMsgDesc(compMsgMsgDesc_t *compMsgMsgDesc) {
}

