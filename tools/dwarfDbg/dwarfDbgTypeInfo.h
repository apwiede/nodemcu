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

typedef struct dwType {
  int bitOffset;   // DW_AT_bit_offset
  int bitSize;     // DW_AT_bit_size
  int byteSize;    // DW_AT_byte_size
  int constValue;  // DW_AT_constvalue
  int location;    // DW_AT_data_member_location
  int declaration; // DW_AT_declaration
  int pathNameIdx; // DW_AT_decl_file
  int lineNo;      // DW_AT_decl_line
  int encoding;    // DW_AT_encoding
  int typeNameIdx; // DW_AT_name
  int prototyped;  // DW_AT-prototyped
  int siblingIdx;  // DW_AT_sibling
  int dwTypeIdx;   // DW_AT_type
} dwType_t;

typedef struct dwTypeValues {
  int maxDwType;
  int numDwType;
  dwType_t *dwTypes;
} dwTypeValues_t;

typedef uint8_t (* addTypeStr_t)(dwarfDbgPtr_t self, const char *str, int *typeStrIdx);
typedef uint8_t (* checkDieTypeRefIdx_t)(dwarfDbgPtr_t self);

typedef uint8_t (* addType_t)(dwarfDbgPtr_t self, dwType_t *dwTypeInfo, const char * name, dwTypeValues_t *typeValues, int *typeIdx);

typedef uint8_t (* handleType_t)(dwarfDbgPtr_t self, dieAndChildrenInfo_t *dieAndChildrenInfo, dieInfo_t *dieInfo, int isSibling, dwTypeValues_t *dwTypeValues);

typedef uint8_t (* addTypes_t)(dwarfDbgPtr_t self, int dieAndChildrenIdx, int flags, Dwarf_Bool isSibling);
typedef uint8_t (* addChildrenTypes_t)(dwarfDbgPtr_t self, int dieAndChildrenIdx, int flags);
typedef uint8_t (* addSiblingsTypes_t)(dwarfDbgPtr_t self, int dieAndChildrenIdx, int flags);

typedef struct dwarfDbgTypeInfo {
  dwTypeValues_t dwArrayTypeInfos;
  dwTypeValues_t dwBaseTypeInfos;
  dwTypeValues_t dwConstTypeInfos;
  dwTypeValues_t dwEnumerationTypeInfos;
  dwTypeValues_t dwEnumeratorInfos;
  dwTypeValues_t dwMemberInfos;
  dwTypeValues_t dwPointerTypeInfos;
  dwTypeValues_t dwStructureTypeInfos;
  dwTypeValues_t dwSubroutineTypeInfos;
  dwTypeValues_t dwTypedefInfos;
  dwTypeValues_t dwUnionTypeInfos;
  dwTypeValues_t dwVolatileTypeInfos;

  int maxTypeStr;
  int numTypeStr;
  char **typeStrs;

  addTypeStr_t addTypeStr;
  checkDieTypeRefIdx_t checkDieTypeRefIdx;

  addType_t addType;

  handleType_t handleType;

  addTypes_t addTypes;
  addChildrenTypes_t addChildrenTypes;
  addSiblingsTypes_t addSiblingsTypes;
} dwarfDbgTypeInfo_t;

#endif  /* DWARF_DBG_TYPE_INFO_H */
