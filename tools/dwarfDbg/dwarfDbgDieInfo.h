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

typedef struct dieAttr {
  Dwarf_Half attr;
  Dwarf_Attribute attr_in;
  Dwarf_Half theform;
  Dwarf_Half directform;
  Dwarf_Unsigned uval;
  uint16_t flags;
} dieAttr_t;

typedef struct dieInfo {
  size_t offset;
  Dwarf_Half tag;
  size_t numAttr;
  size_t maxAttr;
  dieAttr_t *dieAttrs;
} dieInfo_t;

typedef struct dieAndChildrenInfo {
  Dwarf_Die die;
  size_t numSiblings;
  size_t maxSiblings;
  dieInfo_t *dieSiblings;
  size_t numChildren;
  size_t maxChildren;
  dieInfo_t *dieChildren;
} dieAndChildrenInfo_t;

typedef uint8_t (* showChildren_t)(dwarfDbgPtr_t self, size_t dieAndChildrenIdx, const char *indent);
typedef uint8_t (* showSiblings_t)(dwarfDbgPtr_t self, size_t dieAndChildrenIdx, const char *indent);
typedef uint8_t (* addDieChildAttr_t)(dwarfDbgPtr_t self, size_t dieAndChildrenIdx, size_t childIdx, Dwarf_Half attr, Dwarf_Attribute attr_in, Dwarf_Unsigned uval, Dwarf_Half theform, Dwarf_Half directform, uint16_t flags, size_t *childAttrIdx);
typedef uint8_t (* addDieSiblingAttr_t)(dwarfDbgPtr_t self, size_t dieAndChildrenIdx, size_t siblingIdx, Dwarf_Half attr, Dwarf_Attribute attr_in, Dwarf_Unsigned uval, Dwarf_Half theform, Dwarf_Half directform, uint16_t flags, size_t *siblingAttrIdx);
typedef uint8_t (* addDieSibling_t)(dwarfDbgPtr_t self, size_t dieAndChildrenIdx, size_t offset, Dwarf_Half tag, size_t *siblingIdx);
typedef uint8_t (* addDieChild_t)(dwarfDbgPtr_t self, size_t dieAndChildrenIdx, size_t offset, Dwarf_Half tag, size_t *childIdx);
typedef uint8_t (* addDieAndChildren_t)(dwarfDbgPtr_t self, Dwarf_Die die, size_t *dieAndChildrenIdx);

typedef struct dwarfDbgDieInfo {
//  size_t numDieAndChildren;
//  size_t maxDieAndChildren;
//  dieAndChildrenInfo_t *dieAndChildren;

  showSiblings_t showSiblings;
  showChildren_t showChildren;
  addDieChildAttr_t addDieChildAttr;
  addDieSiblingAttr_t addDieSiblingAttr;
  addDieAndChildren_t addDieAndChildren;
  addDieSibling_t addDieSibling;
  addDieChild_t addDieChild;
} dwarfDbgDieInfo_t;

FILE *showFd;

#endif  /* DWARF_DBG_DIE_INFO */

