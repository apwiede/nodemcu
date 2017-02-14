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
 * File:   dwarfDbgDebug.h
 * Author: Arnulf P. Wiedemann
 *
 * Created on February 14, 2017
 */

#ifndef DWARF_DBG_DEBUG_H
#define	DWARF_DBG_DEBUG_H

#include "ctype.h"
#ifdef	__cplusplus
extern "C" {
#endif

#define DEBUG_DWARF_DBG_ATTRIBUTE       0x00001
#define DEBUG_DWARF_DBG_COMPILE_UNIT    0x00002
#define DEBUG_DWARF_DBG_DIE             0x00004
#define DEBUG_DWARF_DBG_DICT            0x00008
#define DEBUG_DWARF_DBG_ELF             0x00010
#define DEBUG_DWARF_DBG_FILE            0x00020
#define DEBUG_DWARF_DBG_FRAME           0x00040
#define DEBUG_DWARF_DBG_GET_DBG         0x00080
#define DEBUG_DWARF_DBG_LINE            0x00100
#define DEBUG_DWARF_DBG_LOCATION        0x00200
#define DEBUG_DWARF_DBG_RANGE           0x00400
#define DEBUG_DWARF_DBG_STRING          0x00800
#define DEBUG_DWARF_DBG_UTIL            0x01000
#define DEBUG_DWARF_DBG_ALWAYS          0x80000

#define DWARF_DBG_DEBUG

#ifdef DWARF_DBG_DEBUG
#define DWARF_DBG_PRINT self->dwarfDbgDebugInfo->dbgPrintf
#else
#define DWARF_DBG_PRINT
#endif

typedef void (* dbgPrintf_t)(dwarfDbgPtr_t self, uint8_t *dbgChars, uint8_t debugLevel, uint8_t *format, ...);
typedef uint32_t (* getDebugFlags_t)(dwarfDbgPtr_t self, uint8_t *dbgChars);
typedef uint8_t (* setDebugFlags_t)(dwarfDbgPtr_t self, uint8_t *dbgChars);

typedef struct dwarfDbgDebugInfo {
  uint32_t currDebugFlags;
  uint8_t debugLevel;

  dbgPrintf_t dbgPrintf;
  getDebugFlags_t getDebugFlags;
  setDebugFlags_t setDebugFlags;
} dwarfDbgDebugInfo_t;

#ifdef  __cplusplus
}
#endif

#endif  /* DWARF_DBG_DEBUG_H */

