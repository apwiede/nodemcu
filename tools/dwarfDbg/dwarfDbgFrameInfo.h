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
 * File:   dwarfDbgFrameInfo.h
 * Author: Arnulf P. Wiedemann <arnulf@wiedemann-pri.de>
 *
 * Created on February 07, 2017
 */

#ifndef DWARF_DBG_FRAME_INFO_H
#define	DWARF_DBG_FRAME_INFO_H

typedef uint8_t (* addFrameRegCol_t)(dwarfDbgPtr_t self,Dwarf_Signed cieIdx, size_t cieFdeIdx, size_t fdeIdx, Dwarf_Addr pc, Dwarf_Signed offset, Dwarf_Signed reg, size_t *frcIdx);
typedef uint8_t (* addFde_t)(dwarfDbgPtr_t self, Dwarf_Signed cieIdx, size_t cieFdeIdx, Dwarf_Addr lowPc, Dwarf_Unsigned funcLgth, Dwarf_Signed reg, Dwarf_Signed offset, size_t *fdeIdx);
typedef uint8_t (* addCieFde_t)(dwarfDbgPtr_t self, Dwarf_Signed cieIdx, Dwarf_Addr pc, Dwarf_Unsigned funcLgthg, size_t *cieFdeIdx);
typedef uint8_t (* getFrameList_t)(dwarfDbgPtr_t self);

typedef struct frameRegCol {
  Dwarf_Addr pc;
  Dwarf_Signed offset;
  Dwarf_Signed reg;
  Dwarf_Signed cfaReg; // is always c_cfa_reg!
} frameRegCol_t;

typedef struct frameDataEntry {
  Dwarf_Addr lowPc;
  Dwarf_Unsigned funcLgth;
  Dwarf_Signed reg;
  Dwarf_Signed offset;
  int numFrameRegCol;
  int maxFrameRegCol;
  frameRegCol_t *frameRegCols;
} frameDataEntry_t;

typedef struct cieFde {
  Dwarf_Signed cieIdx;
  int numFde;
  int maxFde;
  frameDataEntry_t *frameDataEntries;
} cieFde_t;

typedef struct frameInfo {
  int numFde;
  int numCieFde;
  int maxCieFde;
  cieFde_t *cieFdes;
} frameInfo_t;

typedef struct dwarfDbgFrameInfo {
  frameInfo_t frameInfo;

  addFrameRegCol_t addFrameRegCol;
  addFde_t addFde;
  addCieFde_t addCieFde;
  getFrameList_t getFrameList;
} dwarfDbgFrameInfo_t;


#endif  /* DWARF_DBG_FRAME_INFO_H */
