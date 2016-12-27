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
 * File:   compMsgDebug.c
 * Author: Arnulf P. Wiedemann
 *
 * Created on December 16th, 2016
 */

#include "osapi.h"
#include "c_types.h"
#include "mem.h"
#include "flash_fs.h"

#include "c_string.h"
#include "c_stdio.h"
#include "c_stdlib.h"
#include "compMsgDispatcher.h"

typedef struct debugCh2Id {
  uint8_t ch;
  uint32_t id;
} debugCh2Id_t;

debugCh2Id_t debugCh2Id[] = {
 { 'A', DEBUG_COMP_MSG_ACTION},
 { 'B', DEBUG_COMP_MSG_BUILD_MSG},
 { 'D', DEBUG_COMP_MSG_DISPATCHER},
 { 'd', DEBUG_COMP_MSG_DATA},
 { 'V', DEBUG_DATA_VIEW},
 { 'v', DEBUG_COMP_MSG_DATA_VIEW},
 { 'H', DEBUG_COMP_MSG_HTTP},
 { 'I', DEBUG_COMP_MSG_IDENTIFY},
 { 'E', DEBUG_COMP_MSG_MSG_DESC},
 { 'M', DEBUG_COMP_MSG_MODULE_DATA},
 { 'N', DEBUG_COMP_MSG_NET_SOCKET},
 { 'O', DEBUG_COMP_MSG_OTA},
 { 'R', DEBUG_COMP_MSG_REQUEST},
 { 'S', DEBUG_COMP_MSG_SOCKET},
 { 's', DEBUG_COMP_MSG_SEND_RECEIVE},
 { 'T', DEBUG_COMP_MSG_TYPES_AND_NAMES},
 { 't', DEBUG_COMP_MSG_TIMER},
 { 'U', DEBUG_COMP_MSG_UTIL},
 { 'W', DEBUG_COMP_MSG_WEB_SOCKET},
 { 'w', DEBUG_COMP_MSG_WIFI_DATA},
 { 'Y', DEBUG_COMP_MSG_ALWAYS},
 { '\0', 0},
};

// ================================= getDebugFlags ====================================

static uint32_t getDebugFlags(compMsgDispatcher_t *self, uint8_t *dbgChars) {
  int idx;
  uint8_t *cp;
  bool found;
  debugCh2Id_t *dp;
  uint32_t flags;

  found = false;
  cp = dbgChars;
  flags = 0;
  while (*cp != '\0') {
    dp = &debugCh2Id[0];
    while (dp->ch != '\0') {
      if (dp->ch == *cp) {
        flags |= dp->id;
        break;
      }
      dp++;
    } 
    cp++;
  }
  flags &= self->compMsgDebug->currDebugFlags;
  if (os_strstr(dbgChars, "Y") != NULL) {
    flags |= DEBUG_COMP_MSG_ALWAYS;
  }
  return flags;
}

void ets_vprintf(int stream, const char* fmt, va_list arglist );  


// ================================= dbgPrintf ====================================

static void dbgPrintf(compMsgDispatcher_t *self, uint8_t *dbgChars, uint8_t debugLevel, uint8_t *format, ...) {
  uint32_t flags;
  va_list arglist;
  int id;
  char buf[30];
  char outbuf[30];
  uint8_t *cp;
  uint8_t *cp2;
  uint8_t *cp3;
  uint8_t *ep;
  char chval;
  int val;
  int dval;
  void *pval;
  char *strval;
  bool found;

  id = 0;
  ep = format + c_strlen(format) - 1;
  flags = self->compMsgDebug->getDebugFlags(self, dbgChars);
  if (flags && (debugLevel >= self->compMsgDebug->debugLevel)) {
    cp3 = "%==DBG: ";
    while (*cp3 != '\0') {
      platform_uart_send(id, *cp3);
      cp3++;
    }
    va_start(arglist, format);
    cp = format;
    while (*cp) {
      switch (*cp) {
      case '\n':
        if (cp < ep) {
          platform_uart_send(id, '\n');
        }
        break;
      case '%':
        cp3 = cp;
        cp2 = buf;
        *cp2++ = *cp;
        cp++;
        found = false;
        while (*cp != '\0') {
          switch (*cp) {
          case 'c':
          case 'd':
          case 'u':
          case 's':
          case 'x':
          case 'p':
          case '%':
            *cp2 = *cp;
            cp2++;
            found = true;
            break;
          default:
            *cp2 = *cp;
            cp2++;
            cp++;
            break;
          }
          if (found) {
            break;
          }
        }
        *cp2 = '\0';
        switch (*cp) {
        case '%':
          platform_uart_send(id, *cp);
          break;
        case 'c':
          chval = (char)va_arg(arglist, int);
          ets_sprintf(outbuf, buf, chval);
          cp3 = outbuf;
          while (*cp3 != '\0') {
            platform_uart_send(id, *cp3);
            cp3++;
          }
          break;
        case 'd':
          dval = va_arg(arglist, int);
          ets_sprintf(outbuf, buf, dval);
          cp3 = outbuf;
          while (*cp3 != '\0') {
            platform_uart_send(id, *cp3);
            cp3++;
          }
          break;
        case 's':
          strval = va_arg(arglist, char*);
          ets_sprintf(outbuf, buf, strval);
          cp3 = outbuf;
          while (*cp3 != '\0') {
            platform_uart_send(id, *cp3);
            cp3++;
          }
          break;
        case 'u':
        case 'x':
          dval = va_arg(arglist, int);
          ets_sprintf(outbuf, buf, dval);
          cp3 = outbuf;
          while (*cp3 != '\0') {
            platform_uart_send(id, *cp3);
            cp3++;
          }
          break;
        case 'p':
          pval = va_arg(arglist, void*);
          ets_sprintf(outbuf, buf, pval);
          cp3 = outbuf;
          while (*cp3 != '\0') {
            platform_uart_send(id, *cp3);
            cp3++;
          }
          break;
        default:
          platform_uart_send(id, *cp);
        }
        break;
      default:
        platform_uart_send(id, *cp);
        break;
      }
      cp++;
    }
    cp3 = "%";
    while (*cp3 != '\0') {
      platform_uart_send(id, *cp3);
      cp3++;
    }
    if (self->compMsgDebug->addEol) {
      platform_uart_send(id, '\n');
    }
    va_end(arglist);
  }
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
  return COMP_MSG_ERR_OK;
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
  return COMP_MSG_ERR_OK;
}

// ================================= dumpMsgParts ====================================

static uint8_t dumpMsgParts(compMsgDispatcher_t *self, msgParts_t *msgParts) {
  int idx;

  ets_printf("dumpMsgParts:\n");
  ets_printf("MsgParts1 form: 0x%04x to: 0x%04x totalLgth: %d GUID: %d srcId: %d u16_cmdKey: %d\n", msgParts->fromPart, msgParts->toPart, msgParts->totalLgth, msgParts->GUID, msgParts->srcId, msgParts->u16CmdKey);

  ets_printf("MsgParts2 lgth: %d fieldOffset: %d\n", msgParts->lgth, msgParts->fieldOffset);
  ets_printf("buf");
  idx = 0;
  while (idx < msgParts->realLgth - 1) {
    ets_printf(" %d 0x%02x", idx, msgParts->buf[idx]);
    idx++;
  }
  ets_printf("\n");
  ets_printf("partFlags: ");
  if (msgParts->partsFlags & COMP_DISP_U16_CMD_KEY) {
    ets_printf(" COMP_DISP_U16_CMD_KEY");
  }
  if (msgParts->partsFlags & COMP_DISP_U8_VECTOR_GUID) {
    ets_printf(" COMP_DISP_U8_VECTOR_GUID");
  }
  if (msgParts->partsFlags & COMP_DISP_U16_SRC_ID) {
    ets_printf(" COMP_DISP_U16_SRC_ID");
  }
  if (msgParts->partsFlags & COMP_DISP_U16_CMD_KEY) {
    ets_printf(" COMP_DISP_U16_CMD_KEY");
  }
  ets_printf("\n");
  return COMP_MSG_ERR_OK;
}

// ================================= dumpMsgDescPart ====================================

static uint8_t dumpMsgDescPart(compMsgDispatcher_t *self, msgDescPart_t *msgDescPart) {
  uint8_t result;
  fieldSizeCallback_t callback;
  uint8_t *callbackName;

  callbackName = "nil";
  if (msgDescPart->fieldSizeCallback != NULL) {
    result = self->compMsgUtil->getFieldValueCallbackName(self, msgDescPart->fieldSizeCallback, &callbackName, 0);
    checkErrOK(result);
  }
  ets_printf("msgDescPart: fieldNameStr: %-15.15s fieldNameId: %.3d fieldTypeStr: %-10.10s fieldTypeId: %.3d field_lgth: %d callback: %s\n", msgDescPart->fieldNameStr, msgDescPart->fieldNameId, msgDescPart->fieldTypeStr, msgDescPart->fieldTypeId, msgDescPart->fieldLgth, callbackName);
  return COMP_MSG_ERR_OK;
}

// ================================= dumpMsgValPart ====================================

static uint8_t dumpMsgValPart(compMsgDispatcher_t *self, msgValPart_t *msgValPart) {
  uint8_t result;
  fieldValueCallback_t callback;
  uint8_t *callbackName;

  callbackName = "nil";
  if (msgValPart->fieldValueCallback != NULL) {
    result = self->compMsgUtil->getFieldValueCallbackName(self, msgValPart->fieldValueCallback, &callbackName, msgValPart->fieldValueCallbackType);
    checkErrOK(result);
  }
  ets_printf("msgValPart: fieldNameStr: %-15.15s fieldNameId: %.3d fieldValueStr: %-10.10s callback: %s flags: ", msgValPart->fieldNameStr, msgValPart->fieldNameId, msgValPart->fieldValueStr, callbackName);
  if (msgValPart->fieldFlags & COMP_DISP_DESC_VALUE_IS_NUMBER) {
     ets_printf(" COMP_DISP_DESC_VALUE_IS_NUMBER");
  }
  ets_printf("\n");
  return COMP_MSG_ERR_OK;
}

// ================================= compMsgDebugInit ====================================

uint8_t compMsgDebugInit(compMsgDispatcher_t *self) {
  uint8_t result;

  self->compMsgDebug->currDebugFlags = DEBUG_COMP_MSG_WEB_SOCKET;
  self->compMsgDebug->addEol = true;
  self->compMsgDebug->debugLevel = 1;

  self->compMsgDebug->dbgPrintf = &dbgPrintf;
  self->compMsgDebug->getDebugFlags = &getDebugFlags;
  self->compMsgDebug->dumpMsgParts = &dumpMsgParts;
  self->compMsgDebug->dumpHeaderPart = &dumpHeaderPart;
  self->compMsgDebug->dumpMsgHeaderInfos = &dumpMsgHeaderInfos;
  self->compMsgDebug->dumpMsgDescPart = &dumpMsgDescPart;
  self->compMsgDebug->dumpMsgValPart = &dumpMsgValPart;
  self->compMsgDebug->dumpHeaderPart = &dumpHeaderPart;
  self->compMsgDebug->dumpMsgHeaderInfos = &dumpMsgHeaderInfos;
  return COMP_MSG_ERR_OK;
}

// ================================= newCompMsgDebug ====================================

compMsgDebug_t *newCompMsgDebug() {
  compMsgDebug_t *compMsgDebug = os_zalloc(sizeof(compMsgDebug_t));
  if (compMsgDebug == NULL) {
    return NULL;
  }

  return compMsgDebug;
}

