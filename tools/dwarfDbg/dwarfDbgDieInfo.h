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

#define TAG_REF_ARRAY_TYPE              0x000001
#define TAG_REF_BASE_TYPE               0x000002
#define TAG_REF_CONST_TYPE              0x000004
#define TAG_REF_ENUMERATION_TYPE        0x000008
#define TAG_REF_ENUMERATOR              0x000010
#define TAG_REF_FORMAL_PARAMETER        0x000020
#define TAG_REF_GNU_CALL_SITE           0x000040
#define TAG_REF_GNU_CALL_SITE_PARAMETER 0x000080
#define TAG_REF_INLINED_SUBROUTINE      0x000100
#define TAG_REF_LABEL                   0x000200
#define TAG_REF_LEXICAL_BLOCK           0x000400
#define TAG_REF_MEMBER                  0x000800
#define TAG_REF_POINTER_TYPE            0x001000
#define TAG_REF_STRUCTURE_TYPE          0x002000
#define TAG_REF_SUBPROGRAM              0x004000
#define TAG_REF_SUBRANGE_TYPE           0x008000
#define TAG_REF_SUBROUTINE_TYPE         0x010000
#define TAG_REF_TYPEDEF                 0x020000
#define TAG_REF_UNION_TYPE              0x040000
#define TAG_REF_UNSPECIFIED_PARAMETERS  0x080000
#define TAG_REF_VARIABLE                0x100000
#define TAG_REF_VOLATILE_TYPE           0x200000

typedef struct dieInfo {
  Dwarf_Off offset;     // this can be referenced by dieAttr.refOffset.
  Dwarf_Half tag;
  int tagRefIdx;
  int flags;
  int numAttr;
  int maxAttr;
  dieAttr_t *dieAttrs;
} dieInfo_t;

typedef struct dieAndChildrenInfo {
  Dwarf_Die die;
  int numSiblings;
  int maxSiblings;
  dieInfo_t *dieSiblings;
  int numChildren;
  int maxChildren;
  dieInfo_t *dieChildren;
} dieAndChildrenInfo_t;

typedef uint8_t (* showChildren_t)(dwarfDbgPtr_t self, int dieAndChildrenIdx, const char *indent);
typedef uint8_t (* showSiblings_t)(dwarfDbgPtr_t self, int dieAndChildrenIdx, const char *indent);
typedef uint8_t (* addAttrStr_t)(dwarfDbgPtr_t self, const char *str, int *attrStrIdx);
typedef uint8_t (* addDieChildAttr_t)(dwarfDbgPtr_t self, int dieAndChildrenIdx, int childIdx, Dwarf_Half attr, Dwarf_Attribute attrIn, Dwarf_Unsigned uval, Dwarf_Half theform, Dwarf_Half directform, int *childAttrIdx);
typedef uint8_t (* addDieSiblingAttr_t)(dwarfDbgPtr_t self, int dieAndChildrenIdx, int siblingIdx, Dwarf_Half attr, Dwarf_Attribute attrIn, Dwarf_Unsigned uval, Dwarf_Half theform, Dwarf_Half directform, int *siblingAttrIdx);
typedef uint8_t (* addDieSibling_t)(dwarfDbgPtr_t self, int dieAndChildrenIdx, Dwarf_Off offset, Dwarf_Half tag, int *siblingIdx);
typedef uint8_t (* addDieChild_t)(dwarfDbgPtr_t self, int dieAndChildrenIdx, Dwarf_Off offset, Dwarf_Half tag, int *childIdx);
typedef uint8_t (* addDieAndChildren_t)(dwarfDbgPtr_t self, Dwarf_Die die, int *dieAndChildrenIdx);

typedef struct dwarfDbgDieInfo {

  showSiblings_t showSiblings;
  showChildren_t showChildren;
  addAttrStr_t addAttrStr;
  addDieChildAttr_t addDieChildAttr;
  addDieSiblingAttr_t addDieSiblingAttr;
  addDieAndChildren_t addDieAndChildren;
  addDieSibling_t addDieSibling;
  addDieChild_t addDieChild;
} dwarfDbgDieInfo_t;

FILE *showFd;

#endif  /* DWARF_DBG_DIE_INFO */

