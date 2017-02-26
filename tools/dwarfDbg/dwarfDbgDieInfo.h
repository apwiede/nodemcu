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
 * File:   dwarfDbgDieInfo.h
 * Author: Arnulf P. Wiedemann <arnulf@wiedemann-pri.de>
 *
 * Created on February 04, 2017
 */

#ifndef DWARF_DBG_DIE_INFO_H
#define	DWARF_DBG_DIE_INFO_H

#define DW_LOCATION_INFO              0x01
#define DW_NAME_INFO                  0x02
#define DW_HIGH_PC_OFFSET_FROM_LOW_PC 0x04

typedef struct dieAttr {
  Dwarf_Half attr;
  Dwarf_Attribute attrIn;
  Dwarf_Half theform;
  Dwarf_Half directform;
  Dwarf_Signed sval;
  Dwarf_Unsigned uval;
  Dwarf_Off refOffset;   // this offset refers to a dieInfo.offset field with the same value
  Dwarf_Addr lowPc;
  Dwarf_Addr highPc;
  Dwarf_Signed signedLowPcOffset;
  Dwarf_Unsigned unsignedLowPcOffset;
  int sourceFileIdx;
  int sourceLineNo;
  int byteSize;
  int attrStrIdx;
  uint16_t flags;
  locationInfo_t *locationInfo;
} dieAttr_t;

typedef struct dieInfo {
  Dwarf_Off offset;     // this can be referenced by dieAttr.refOffset.
  Dwarf_Half tag;
  int tagRefIdx;
  int numAttr;
  int maxAttr;
  dieAttr_t *dieAttrs;
} dieInfo_t;

typedef struct dieTagInfo {
  Dwarf_Half tag;
  int dwAttrTypeInfoIdx;
  int numAttr;
} dieTagInfo_t;

typedef struct dieAndChildrenInfo {
  Dwarf_Die die;
  int isSibling;
  int level;

  int numSiblings;
  int maxSiblings;
  dieInfo_t *dieSiblings;

  int numChildren;
  int maxChildren;
  dieInfo_t *dieChildren;

  int numSiblingsTagInfo;
  int maxSiblingsTagInfo;
  dieTagInfo_t *dieSiblingsTagInfos;

  int numChildrenTagInfo;
  int maxChildrenTagInfo;
  dieTagInfo_t *dieChildrenTagInfos;
} dieAndChildrenInfo_t;

typedef uint8_t (* showChildren_t)(dwarfDbgPtr_t self, int dieAndChildrenIdx, const char *indent);
typedef uint8_t (* showSiblings_t)(dwarfDbgPtr_t self, int dieAndChildrenIdx, const char *indent);
typedef uint8_t (* addAttrStr_t)(dwarfDbgPtr_t self, const char *str, int *attrStrIdx);
typedef uint8_t (* addDieChildAttr_t)(dwarfDbgPtr_t self, int dieAndChildrenIdx, int childIdx, Dwarf_Half attr, Dwarf_Attribute attrIn, Dwarf_Unsigned uval, Dwarf_Half theform, Dwarf_Half directform, int *childAttrIdx);
typedef uint8_t (* addDieSiblingAttr_t)(dwarfDbgPtr_t self, int dieAndChildrenIdx, int siblingIdx, Dwarf_Half attr, Dwarf_Attribute attrIn, Dwarf_Unsigned uval, Dwarf_Half theform, Dwarf_Half directform, int *siblingAttrIdx);
typedef uint8_t (* addDieSiblingTagInfo_t)(dwarfDbgPtr_t self, int dieAndChildrenIdx, Dwarf_Half tag, int dwAttrTypeInfoIdx, int numAttr, int *siblingTagInfoIdx);
typedef uint8_t (* addDieChildTagInfo_t)(dwarfDbgPtr_t self, int dieAndChildrenIdx,  Dwarf_Half tag, int dwAttrTypeInfoIdx, int numAttr, int *childTagInfoIdx);
typedef uint8_t (* addDieSibling_t)(dwarfDbgPtr_t self, int dieAndChildrenIdx, Dwarf_Off offset, Dwarf_Half tag, int *siblingIdx);
typedef uint8_t (* addDieChild_t)(dwarfDbgPtr_t self, int dieAndChildrenIdx, Dwarf_Off offset, Dwarf_Half tag, int *childIdx);
typedef uint8_t (* addDieAndChildren_t)(dwarfDbgPtr_t self, Dwarf_Die die, int isSibling, int level, int *dieAndChildrenIdx);
typedef  uint8_t (* handleOneDie_t)(dwarfDbgPtr_t self, Dwarf_Die die, int isSibling, int level, char **srcfiles, Dwarf_Signed srcCnt, int dieAndChildrenIdx, int *dieInfoIdx);
typedef uint8_t (* handleDieAndChildren_t)(dwarfDbgPtr_t self, Dwarf_Die in_die_in, int isSibling, int level, char **srcfiles, Dwarf_Signed cnt);
typedef uint8_t (* printDieInfoAttrs_t)(dwarfDbgPtr_t self, dieInfo_t *dieInfo, const char *indent);
typedef uint8_t (* printCompileUnitDieAndChildren_t)(dwarfDbgPtr_t self);

typedef struct dwarfDbgDieInfo {

  showSiblings_t showSiblings;
  showChildren_t showChildren;
  addAttrStr_t addAttrStr;
  addDieChildAttr_t addDieChildAttr;
  addDieSiblingAttr_t addDieSiblingAttr;
  addDieAndChildren_t addDieAndChildren;
  addDieSibling_t addDieSibling;
  addDieChildTagInfo_t addDieChildTagInfo;
  addDieSiblingTagInfo_t addDieSiblingTagInfo;
  addDieChild_t addDieChild;
  handleOneDie_t handleOneDie;
  handleDieAndChildren_t handleDieAndChildren;
  printDieInfoAttrs_t printDieInfoAttrs;
  printCompileUnitDieAndChildren_t printCompileUnitDieAndChildren;
} dwarfDbgDieInfo_t;

FILE *showFd;

#endif  /* DWARF_DBG_DIE_INFO */

