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
 * File:   dwarfDbgAttributeInfo.h
 * Author: Arnulf P. Wiedemann <arnulf@wiedemann-pri.de>
 *
 * Created on February 10, 2017
 */

#ifndef DWARF_DBG_ATTRIBUTE_INFO_H
#define	DWARF_DBG_ATTRIBUTE_INFO_H

typedef struct compileUnit compileUnit_t;
typedef struct dieAndChildrenInfo dieAndChildrenInfo_t;
typedef struct dieInfo dieInfo_t;
typedef struct dieAttr dieAttr_t;

typedef uint8_t (* addAttribute_t)(dwarfDbgPtr_t self, Dwarf_Half attr, Dwarf_Attribute attrIn, char **srcfiles, Dwarf_Signed cnt, size_t dieAndChildrenIdx, Dwarf_Bool isSibling, int *dieAttrIdx);
typedef uint8_t (* handleAttribute_t)(dwarfDbgPtr_t self, Dwarf_Die die, Dwarf_Half attr, Dwarf_Attribute attrIn, char **srcfiles, Dwarf_Signed cnt, int dieAndChildrenIdx, int dieInfoIdx, Dwarf_Bool isSibling, int *dieAttrIdx);

typedef struct attrInInfo {
  Dwarf_Half attr;
  Dwarf_Attribute attrIn;
  char **srcfiles;
  Dwarf_Signed cnt;
  int dieAndChildrenIdx;
  int dieInfoIdx;
  int dieAttrIdx;
  Dwarf_Die die;
  Dwarf_Bool isSibling;
  Dwarf_Unsigned uval;
  Dwarf_Signed sval;
  Dwarf_Half theform;
  Dwarf_Half directform;
  Dwarf_Half version;
  Dwarf_Half offsetSize;
  compileUnit_t *compileUnit;
  dieAndChildrenInfo_t *dieAndChildrenInfo;
  dieInfo_t *dieInfo;
  dieAttr_t *dieAttr;
  uint16_t flags;
  enum Dwarf_Form_Class formClass;
} attrInInfo_t;

typedef struct dwarfDbgAttributeInfo {
  addAttribute_t addAttribute;
  handleAttribute_t handleAttribute;
} dwarfDbgAttributeInfo_t;


#endif  /* DWARF_DBG_ATTRIBUTE_INFO_H */
