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

typedef struct dwBaseType {
  int typeStrIdx;  // DW_AT_name
  int byteSize;    // DW_AT_byte_size
  int encoding;    // DW_AT_encoding
} dwBaseType_t;

typedef struct dwTypeDef {
  int typeStrIdx;  // DW_AT_name
  int pathNameIdx; // DW_AT_decl_file
  int lineNo;      // DW_AT_decl_line
  int dwTypeIdx;   // DW_AT_type
} dwTypeDef_t;

typedef struct dwConstType {
  int dwTypeIdx;  // DW_AT_type
} dwConstType_t;

typedef struct dwMember {
  int typeStrIdx;  // DW_AT_name
  int pathNameIdx; // DW_AT_decl_file
  int lineNo;      // DW_AT_decl_line
  int dwTypeIdx;   // DW_AT_type
  int byteSize;    // DW_AT_byte_size
  int bitSize;     // DW_AT_bit_size
  int bitOffset;   // DW_AT_bit_offset
  int location;    // DW_AT_location
} dwMember_t;

typedef struct dwPointerType {
  int byteSize;    // DW_AT_byte_size
  int dwTypeIdx;   // DW_AT_type
} dwPointerType_t;

typedef struct dwVolatileType {
  int dwTypeIdx;   // DW_AT_type
} dwVolatileType_t;

typedef struct dwSubroutineType {
  int prototyped;  // DW_AT_prototyped
  int byteSize;    // DW_AT_byte_size
  int dwTypeIdx;   // DW_AT_type
  int siblingIdx;  // DW_AT_sibling ??? what for
} dwSubroutineType_t;

typedef struct dwStructureType {
  int typeStrIdx;  // DW_AT_name
  int byteSize;    // DW_AT_byte_size
  int declaration; // DW_AT_declaration
  int pathNameIdx; // DW_AT_decl_file
  int lineNo;      // DW_AT_decl_line
  int siblingIdx;  // DW_AT_sibling ??? what for
} dwStructureType_t;

typedef struct dwUnionType {
  int typeStrIdx;  // DW_AT_name
  int byteSize;    // DW_AT_byte_size
  int pathNameIdx; // DW_AT_decl_file
  int lineNo;      // DW_AT_decl_line
  int siblingIdx;  // DW_AT_sibling ??? what for
} dwUnionType_t;

typedef struct dwEnumerationType {
  int byteSize;    // DW_AT_byte_size
  int pathNameIdx; // DW_AT_decl_file
  int lineNo;      // DW_AT_decl_line
  int siblingIdx;  // DW_AT_sibling ??? what for
} dwEnumerationType_t;

typedef struct dwEnumeratorType {
  int typeStrIdx;  // DW_AT_name
  int constValue;  // DW_AT_const_value
} dwEnumeratorType_t;

typedef struct dwArrayType {
  int dwTypeIdx;   // DW_AT_type
  int siblingIdx;  // DW_AT_sibling ??? what for
} dwArrayType_t;

typedef uint8_t (* addTypeStr_t)(dwarfDbgPtr_t self, const char *str, int *typeStrIdx);
typedef uint8_t (* addBaseType_t)(dwarfDbgPtr_t self, const char *name, int byteSize, int encoding, int *baseTypeIdx);
typedef uint8_t (* addTypeDef_t)(dwarfDbgPtr_t self, const char *name, int pathNameIdx, int lineNo, int dwTypeIdx, int *typeDefIdx);
typedef uint8_t (* addConstType_t)(dwarfDbgPtr_t self, int dwTypeIdx, int *constTypeIdx);
typedef uint8_t (* addMember_t)(dwarfDbgPtr_t self, int byteSize, int encoding, char *name, int *baseTypeIdx);
typedef uint8_t (* addPointerType_t)(dwarfDbgPtr_t self, int byteSize, int dwtypeIdx, int *pointerTypeIdx);
typedef uint8_t (* addStructureType_t)(dwarfDbgPtr_t self, const char *name, int byteSize, int pathNameIdx, int lineNo, int declaration, int *structureTypeIdx);
typedef uint8_t (* addArrayType_t)(dwarfDbgPtr_t self, int byteSize, int encoding, char *name, int *baseTypeIdx);
typedef uint8_t (* addEnumerationType_t)(dwarfDbgPtr_t self, int byteSize, int encoding, char *name, int *baseTypeIdx);
typedef uint8_t (* addEnumerator_t)(dwarfDbgPtr_t self, int byteSize, int encoding, char *name, int *baseTypeIdx);
typedef uint8_t (* addUnionType_t)(dwarfDbgPtr_t self, int byteSize, int encoding, char *name, int *baseTypeIdx);
typedef uint8_t (* addVolatileType_t)(dwarfDbgPtr_t self, int byteSize, int encoding, char *name, int *baseTypeIdx);
typedef uint8_t (* addSubroutineType_t)(dwarfDbgPtr_t self, int prototyped, int byteSize, int dwTypeIdx, int siblingIdx, int *subroutineTypeIdx);
typedef uint8_t (* handlePointerType_t)(dwarfDbgPtr_t self, dieAndChildrenInfo_t *dieAndChildrenInfo, dieInfo_t *dieInfo, Dwarf_Bool isSibling);
typedef uint8_t (* handleConstType_t)(dwarfDbgPtr_t self, dieAndChildrenInfo_t *dieAndChildrenInfo, dieInfo_t *dieInfo, Dwarf_Bool isSibling);
typedef uint8_t (* handleStructureType_t)(dwarfDbgPtr_t self, dieAndChildrenInfo_t *dieAndChildrenInfo, dieInfo_t *dieInfo, Dwarf_Bool isSibling);
typedef uint8_t (* handleSubroutineType_t)(dwarfDbgPtr_t self, dieAndChildrenInfo_t *dieAndChildrenInfo, dieInfo_t *dieInfo, Dwarf_Bool isSibling);
typedef uint8_t (* handleTypedef_t)(dwarfDbgPtr_t self, dieAndChildrenInfo_t *dieAndChildrenInfo, dieInfo_t *dieInfo, Dwarf_Bool isSibling);
typedef uint8_t (* addTypes_t)(dwarfDbgPtr_t self, int dieAndChildrenIdx, int flags, Dwarf_Bool isSibling);
typedef uint8_t (* addChildrenTypes_t)(dwarfDbgPtr_t self, int dieAndChildrenIdx, int flags);
typedef uint8_t (* addSiblingsTypes_t)(dwarfDbgPtr_t self, int dieAndChildrenIdx, int flags);

typedef struct dwarfDbgTypeInfo {
  int maxDwBaseType;
  int numDwBaseType;
  dwBaseType_t *dwBaseTypes;

  int maxDwTypeDef;
  int numDwTypeDef;
  dwTypeDef_t *dwTypeDefs;

  int maxDwConstType;
  int numDwConstType;
  dwConstType_t *dwConstTypes;

  int maxDwMember;
  int numDwMember;
  dwMember_t *dwMembers;

  int maxDwPointerType;
  int numDwPointerType;
  dwPointerType_t *dwPointerTypes;

  int maxDwStructureType;
  int numDwStructureType;
  dwStructureType_t *dwStructureTypes;

  int maxDwUnionType;
  int numDwUnionType;
  dwUnionType_t *dwUnionTypes;

  int maxDwEnumerationType;
  int numDwEnumerationType;
  dwEnumerationType_t *dwEnumerationTypes;

  int maxDwEnumeratorType;
  int numDwEnumeratorType;
  dwEnumeratorType_t *dwEnumeratorTypes;

  int maxDwArrayType;
  int numDwArrayType;
  dwArrayType_t *dwArrayTypes;

  int maxDwVolatileType;
  int numDwVolatileType;
  dwVolatileType_t *dwVolatileTypes;

  int maxDwSubroutineType;
  int numDwSubroutineType;
  dwSubroutineType_t *dwSubroutineTypes;

  int maxTypeStr;
  int numTypeStr;
  char **typeStrs;

  addTypeStr_t addTypeStr;
  addBaseType_t addBaseType;
  addTypeDef_t addTypeDef;
  addConstType_t addConstType;
  addMember_t addMember;
  addPointerType_t addPointerType;
  addStructureType_t addStructureType;
  addArrayType_t addArrayType;
  addEnumerationType_t addEnumerationType;
  addEnumerator_t addEnumerator;
  addUnionType_t addUnionType;
  addVolatileType_t addVolatileType;
  addSubroutineType_t addSubroutineType;
  handlePointerType_t handlePointerType;
  handleConstType_t handleConstType;
  handleStructureType_t handleStructureType;
  handleSubroutineType_t handleSubroutineType;
  handleTypedef_t handleTypedef;
  addTypes_t addTypes;
  addChildrenTypes_t addChildrenTypes;
  addSiblingsTypes_t addSiblingsTypes;
} dwarfDbgTypeInfo_t;

#endif  /* DWARF_DBG_TYPE_INFO_H */
