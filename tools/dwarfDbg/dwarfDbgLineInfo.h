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
 * File:   dwarfDbgLineInfo.h
 * Author: Arnulf P. Wiedemann <arnulf@wiedemann-pri.de>
 *
 * Created on February 12, 2017
 */

#ifndef DWARF_DBG_LINE_INFO_H
#define	DWARF_DBG_LINE_INFO_H

#define LINE_NEW_STATEMENT   0x01
#define LINE_NEW_BASIC_BLOCK 0x02
#define LINE_END_SEQUENCE    0x04
#define LINE_PROLOGUE_END    0x08
#define LINE_PROLOGUE_BEGIN  0x10

typedef struct lineInfo {
  Dwarf_Addr pc;
  int lineNo;
  uint16_t flags;
  uint16_t isa;
  uint16_t discriminator;
} lineInfo_t;

typedef struct addr2FileAndLineInfo {
  Dwarf_Addr pc;
  
} addr2FileAndLineInfo_t;

typedef uint8_t (* addLineInfo_t)(dwarfDbgPtr_t self, Dwarf_Addr pc, int lineNo, int flags, uint16_t isa, uint16_t discriminator, int fileInfoIdx, int *fileLineIdx);
typedef uint8_t (* handleLineInfos_t)(dwarfDbgPtr_t self, int *fileLineIdx);

typedef struct dwarfDbgLineInfo {

  addLineInfo_t addLineInfo;
  handleLineInfos_t handleLineInfos;
} dwarfDbgLineInfo_t;

#endif  /* DWARF_DBG_LINE_INFO_H */
