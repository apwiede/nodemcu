/*
* Copyright (c) 2017, Arnulf P. Wiedemann (arnulf@wiedemann-pri.de)
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
 * File:   dwarfDbgDebugInfo.c
 * Author: Arnulf P. Wiedemann
 *
 * Created on February 14th, 2017
 */

#include "dwarfDbgInt.h"

#define MAX_BUFFER_SIZE 1024

typedef struct debugCh2Id {
  uint8_t ch;
  uint32_t id;
} debugCh2Id_t;

debugCh2Id_t debugCh2Id[] = {
  { 'A', DEBUG_DWARF_DBG_ATTRIBUTE},
  { 'C', DEBUG_DWARF_DBG_COMPILE_UNIT},
  { 'D', DEBUG_DWARF_DBG_DIE},
  { 'd', DEBUG_DWARF_DBG_DICT},
  { 'E', DEBUG_DWARF_DBG_ELF},
  { 'F', DEBUG_DWARF_DBG_FILE},
  { 'f', DEBUG_DWARF_DBG_FRAME},
  { 'G', DEBUG_DWARF_DBG_GET_DBG},
  { 'l', DEBUG_DWARF_DBG_LINE},
  { 'L', DEBUG_DWARF_DBG_LOCATION},
  { 'R', DEBUG_DWARF_DBG_RANGE},
  { 'S', DEBUG_DWARF_DBG_STRING},
  { 'U', DEBUG_DWARF_DBG_UTIL},
  { '\0', 0},
};

// ================================= getDebugFlags ====================================

static uint32_t getDebugFlags(dwarfDbgPtr_t self, uint8_t *dbgChars) {
  uint8_t *cp;
  debugCh2Id_t *dp;
  uint32_t flags;

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
  flags &= self->dwarfDbgDebugInfo->currDebugFlags;
  if (strstr(dbgChars, "Y") != NULL) {
    flags |= DEBUG_DWARF_DBG_ALWAYS;
  }
  return flags;
}

// ================================= setDebugFlags ====================================

static uint8_t setDebugFlags(dwarfDbgPtr_t self, uint8_t *dbgChars) {
  uint8_t *cp;
  debugCh2Id_t *dp;
  uint32_t flags;

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
  self->dwarfDbgDebugInfo->currDebugFlags = flags;
  return DWARF_DBG_ERR_OK;
}

// ================================= dbgPrintf ====================================

static void dbgPrintf(dwarfDbgPtr_t self, uint8_t *dbgChars, uint8_t debugLevel, uint8_t *format, ...) {
  uint32_t flags;
  va_list arglist;
  int idx;
  int uartId;
  char buffer[MAX_BUFFER_SIZE];
  uint8_t *cp;
  size_t lgth;

  flags = self->dwarfDbgDebugInfo->getDebugFlags(self, dbgChars);
  if (flags && (debugLevel <= self->dwarfDbgDebugInfo->debugLevel)) {
    va_start(arglist, format);
    lgth = vsnprintf(buffer, MAX_BUFFER_SIZE-1, format, arglist);
    if (lgth < 0) {
printf("ERROR DBG_STR too long\n");
    } else {
      cp = buffer;
//      if (cp[lgth - 1] == '\n') {
//        lgth--;
//      }
      printf("%s", buffer);
      va_end(arglist);
    }
  }
}

// ================================= dwarfDbgDebugInfoInit ====================================

uint8_t dwarfDbgDebugInfoInit(dwarfDbgPtr_t self) {
  uint8_t result;

  self->dwarfDbgDebugInfo->currDebugFlags = DEBUG_DWARF_DBG_ATTRIBUTE;
  self->dwarfDbgDebugInfo->debugLevel = 1;

  self->dwarfDbgDebugInfo->getDebugFlags = &getDebugFlags;
  self->dwarfDbgDebugInfo->setDebugFlags = &setDebugFlags;
  self->dwarfDbgDebugInfo->dbgPrintf = &dbgPrintf;
self->dwarfDbgDebugInfo->setDebugFlags(self, "ACFfGLlR");
  return DWARF_DBG_ERR_OK;
}
