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
 * File:   dwarfDbgCompileUnitInfo.h
 * Author: Arnulf P. Wiedemann <arnulf@wiedemann-pri.de>
 *
 * Created on February 10, 2017
 */

#ifndef DWARF_DBG_COMPILE_UNIT_INFO_H
#define	DWARF_DBG_COMPILE_UNIT_INFO_H

typedef uint8_t (* getAddressSizeAndMax_t)(dwarfDbgPtr_t self, Dwarf_Half *size, Dwarf_Addr *max, Dwarf_Error *err);
typedef uint8_t (* addCompileUnit_t)(dwarfDbgPtr_t self);

typedef struct compileUnit {
  Dwarf_Unsigned compileUnitHeaderLength;
  Dwarf_Half versionStamp;
  Dwarf_Off abbrevOffset;
  Dwarf_Half addressSize;
  Dwarf_Half lengthSize;
  Dwarf_Half extensionSize;
  Dwarf_Sig8 signature;
  Dwarf_Unsigned typeOffset;
  Dwarf_Unsigned nextCompileUnitOffset;
  Dwarf_Half compileUnitType;
  Dwarf_Die compileUnitDie;
  Dwarf_Off overallOffset;
  char *shortFileName;
  char *longFileName;
  int pathNameIdx;
  int fileInfoIdx;
  int level;
  Dwarf_Bool isCompileUnitDie;
  int currSubProgramInfoIdx;
  attrValues_t attrValues;

  int  maxFileInfo;    /* Size of the fileInfos array. */
  int  numFileInfo;    /* Index of the topmost entry */
  fileInfo_t *fileInfos;

  int  maxSourceFile;    /* Size of the source files index array. */
  int  numSourceFile;    /* Index of the topmost entry */
  int *sourceFiles;

  int numDieAndChildren;
  int maxDieAndChildren;
  dieAndChildrenInfo_t *dieAndChildrenInfos;

  int  maxSubProgramInfo;    /* Size of the subProgram array. */
  int  numSubProgramInfo;    /* Index of the topmost entry */
  subProgramInfo_t *subProgramInfos;

  int  maxRangeInfo;    /* Size of the rangeInfos array. */
  int  numRangeInfo;    /* Index of the topmost entry */
  rangeInfo_t *rangeInfos;
} compileUnit_t;

typedef struct dwarfDbgCompileUnitInfo {
  Dwarf_Half addrSize;
  Dwarf_Addr maxAddr;
  int currCompileUnitIdx;
  compileUnit_t *currCompileUnit;

  int numCompileUnit;
  int maxCompileUnit;
  compileUnit_t *compileUnits;

  int numAttrStr;
  int maxAttrStr;
  char **attrStrs;

  getAddressSizeAndMax_t getAddressSizeAndMax;
  addCompileUnit_t addCompileUnit;
} dwarfDbgCompileUnitInfo_t;


#endif  /* DWARF_DBG_COMPILE_UNIT_INFO_H */
