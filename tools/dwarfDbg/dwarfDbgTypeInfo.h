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
 * File:   dwarfDbgTypeInfo.h
 * Author: Arnulf P. Wiedemann <arnulf@wiedemann-pri.de>
 *
 * Created on February 18, 2017
 */

#ifndef DWARF_DBG_TYPE_INFO_H
#define	DWARF_DBG_TYPE_INFO_H

typedef struct dieAndChildrenInfo dieAndChildrenInfo_t;
typedef struct dieInfo dieInfo_t;

typedef struct attrValues {
  const char *name;
  int pathNameIdx;
  int lineNo;
  int byteSize;
  int bitSize;
  int bitOffset;
  int encoding;
  int dwTypeIdx;
} attrValues_t;

typedef struct dwAttrType {
  int dwType;
  int value;
} dwAttrType_t;

typedef struct dwAttrTypeInfo {
  int tag;
  int numDwAttrType;
  int maxDwAttrType;
  dwAttrType_t *dwAttrTypes;
} dwAttrTypeInfo_t;

typedef struct dwAttrTypeInfos {
  int maxDwAttrTypeInfo;
  int numDwAttrTypeInfo;
  dwAttrTypeInfo_t *dwAttrTypeInfos;
} dwAttrTypeInfos_t;

typedef uint8_t (* addTypeStr_t)(dwarfDbgPtr_t self, const char *str, int *typeStrIdx);
typedef uint8_t (* addAttrType_t)(dwarfDbgPtr_t self, dwAttrTypeInfo_t *dwAttrTypeInfo, int dwType, int value, int *attrTypeIdx);
typedef uint8_t (* checkDieTypeRefIdx_t)(dwarfDbgPtr_t self);

typedef uint8_t (* printAttrTypeInfo_t)(dwarfDbgPtr_t self, int idx);
typedef uint8_t (* addAttrTypeInfo_t)(dwarfDbgPtr_t self, dwAttrTypeInfo_t *dwAttrTypeInfo, dwAttrTypeInfos_t *dwAttrTypeInfos, int *typeIdx);

typedef uint8_t (* handleType_t)(dwarfDbgPtr_t self, dieInfo_t *dieInfo);

typedef uint8_t (* addTagTypes_t)(dwarfDbgPtr_t self, int dieAndChildrenIdx, Dwarf_Bool isSibling);
typedef uint8_t (* addChildrenTypes_t)(dwarfDbgPtr_t self, int dieAndChildrenIdx);
typedef uint8_t (* addSiblingsTypes_t)(dwarfDbgPtr_t self, int dieAndChildrenIdx);

typedef struct dwarfDbgTypeInfo {
  dwAttrTypeInfos_t dwAttrTypeInfos;

  int typeLevel;

  int maxTypeStr;
  int numTypeStr;
  char **typeStrs;

  addTypeStr_t addTypeStr;
  addAttrType_t addAttrType;
  printAttrTypeInfo_t printAttrTypeInfo;

  checkDieTypeRefIdx_t checkDieTypeRefIdx;

  addAttrTypeInfo_t addAttrTypeInfo;

  handleType_t handleType;

  addTagTypes_t addTagTypes;
  addChildrenTypes_t addChildrenTypes;
  addSiblingsTypes_t addSiblingsTypes;
} dwarfDbgTypeInfo_t;

#endif  /* DWARF_DBG_TYPE_INFO_H */
