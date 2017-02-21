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
 * File:   dwarfDbgTypeInfo.c
 * Author: Arnulf P. Wiedemann <arnulf@wiedemann-pri.de>
 *
 * Created on February 18, 2017
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libelf.h>
#include <stdlib.h>
#include <string.h>

#include "dwarfDbgInt.h"

#define AT_bit_offset           0x001
#define AT_bit_size             0x002
#define AT_byte_size            0x004
#define AT_const_value          0x008
#define AT_data_member_location 0x010
#define AT_declaration          0x020
#define AT_decl_file            0x040
#define AT_decl_line            0x080
#define AT_name                 0x100
#define AT_prototyped           0x200
#define AT_sibling              0x400
#define AT_type                 0x800

#define ATTR_arrayTypeFlags       0x001
#define ATTR_baseTypeFlags        0x002
#define ATTR_constTypeFlags       0x004
#define ATTR_enumerationTypeFlags 0x008
#define ATTR_enumeratorFlags      0x010
#define ATTR_memberFlags          0x020
#define ATTR_pointerTypeFlags     0x040
#define ATTR_structureTypeFlags   0x080
#define ATTR_subroutineTypeFlags  0x100
#define ATTR_typedefFlags         0x200
#define ATTR_unionTypeFlags       0x400
#define ATTR_volatileTypeFlags    0x800

int currTypeFlags = 0;

int arrayTypeFlags = 0;
int baseTypeFlags = 0;
int constTypeFlags = 0;
int enumerationTypeFlags = 0;
int enumeratorFlags = 0;
int memberFlags = 0;
int pointerTypeFlags = 0;
int structureTypeFlags = 0;
int subroutineTypeFlags = 0;
int typedefFlags = 0;
int unionTypeFlags = 0;
int volatileTypeFlags = 0;

// =================================== setTypeFlag =========================== 

static void setTypeFlag(int flag) {
  switch (currTypeFlags) {
  case  ATTR_arrayTypeFlags:
    arrayTypeFlags |= flag;
    break;
  case  ATTR_baseTypeFlags:
    baseTypeFlags |= flag;
    break;
  case  ATTR_constTypeFlags:
    constTypeFlags |= flag;
    break;
  case  ATTR_enumerationTypeFlags:
    enumerationTypeFlags |= flag;
    break;
  case  ATTR_enumeratorFlags:
    enumeratorFlags |= flag;
    break;
  case  ATTR_memberFlags:
    memberFlags |= flag;
    break;
  case  ATTR_pointerTypeFlags:
    pointerTypeFlags |= flag;
    break;
  case  ATTR_structureTypeFlags:
    structureTypeFlags |= flag;
    break;
  case  ATTR_subroutineTypeFlags:
    subroutineTypeFlags |= flag;
    break;
  case  ATTR_typedefFlags:
    typedefFlags |= flag;
    break;
  case  ATTR_unionTypeFlags:
    unionTypeFlags |= flag;
    break;
  case  ATTR_volatileTypeFlags:
    volatileTypeFlags |= flag;
    break;
  default:
printf("bad currTypeFlags: 0x%08x\n", currTypeFlags);
    break;
  }
}

// =================================== addTypeStr =========================== 

static uint8_t addTypeStr(dwarfDbgPtr_t self, const char *str, int *typeStrIdx) {
  uint8_t result;
  char **typeStr;
  int strIdx;

  result = DWARF_DBG_ERR_OK;
  for(strIdx = 0; strIdx < self->dwarfDbgTypeInfo->numTypeStr; strIdx++) {
    if (strcmp(self->dwarfDbgTypeInfo->typeStrs[strIdx], str) == 0) {
      *typeStrIdx = strIdx;
      return result;
    }
  }
  if (self->dwarfDbgTypeInfo->maxTypeStr <= self->dwarfDbgTypeInfo->numTypeStr) {
    self->dwarfDbgTypeInfo->maxTypeStr += 10;
    if (self->dwarfDbgTypeInfo->typeStrs == NULL) {
      self->dwarfDbgTypeInfo->typeStrs = (char **)ckalloc(sizeof(char *) * self->dwarfDbgTypeInfo->maxTypeStr);
      if (self->dwarfDbgTypeInfo->typeStrs == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      self->dwarfDbgTypeInfo->typeStrs = (char **)ckrealloc((char *)self->dwarfDbgTypeInfo->typeStrs, sizeof(char *) * self->dwarfDbgTypeInfo->maxTypeStr);
      if (self->dwarfDbgTypeInfo->typeStrs == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
//printf("== numTypeStrs: %d %s\n", self->dwarfDbgTypeInfo->numTypeStr, str);
  typeStr = &self->dwarfDbgTypeInfo->typeStrs[self->dwarfDbgTypeInfo->numTypeStr];
  *typeStr = ckalloc(strlen(str) + 1);
  memset(*typeStr, 0, strlen(str) + 1);
  memcpy(*typeStr, str, strlen(str));
  *typeStrIdx = self->dwarfDbgTypeInfo->numTypeStr;
  self->dwarfDbgTypeInfo->numTypeStr++;
  return result;
}

// =================================== addType =========================== 

static uint8_t addType(dwarfDbgPtr_t self, dwType_t *dwTypeInfo, const char * name, dwTypeValues_t *typeValues, int *typeIdx) {
  uint8_t result = 0;
  int typeStrIdx = 0;
  int idx = 0;
  dwType_t *dwTypeInfo2 = NULL;

  result = DWARF_DBG_ERR_OK;
  if (name != NULL) {
    result = self->dwarfDbgTypeInfo->addTypeStr(self, name, &dwTypeInfo->typeNameIdx);
    checkErrOK(result);
  }
  for(idx = 0; idx < typeValues->numDwType; idx++) {
    dwTypeInfo2 = &typeValues->dwTypes[idx];
    if (dwTypeInfo2->bitOffset == dwTypeInfo->bitOffset) {
      if (dwTypeInfo2->bitSize == dwTypeInfo->bitSize) {
        if (dwTypeInfo2->byteSize == dwTypeInfo->byteSize) {
          if (dwTypeInfo2->constValue == dwTypeInfo->constValue) {
            if (dwTypeInfo2->location == dwTypeInfo->location) {
              if (dwTypeInfo2->declaration == dwTypeInfo->declaration) {
                if (dwTypeInfo2->pathNameIdx == dwTypeInfo->pathNameIdx) {
                  if (dwTypeInfo2->lineNo == dwTypeInfo->lineNo) {
                    if (dwTypeInfo2->encoding == dwTypeInfo->encoding) {
                      if (dwTypeInfo2->typeNameIdx == dwTypeInfo->typeNameIdx) {
                        if (dwTypeInfo2->prototyped == dwTypeInfo->prototyped) {
                          if (dwTypeInfo2->siblingIdx == dwTypeInfo->siblingIdx) {
                            if (dwTypeInfo2->dwTypeIdx == dwTypeInfo->dwTypeIdx) {
printf("addType: found: typeIdx: %d\n", idx);
                              *typeIdx = idx;
                              return result;
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  if (typeValues->maxDwType <= typeValues->numDwType) {
    typeValues->maxDwType += 5;
    if (typeValues->dwTypes == NULL) {
      typeValues->dwTypes = (dwType_t *)ckalloc(sizeof(dwType_t) * typeValues->maxDwType);
      if (typeValues->dwTypes == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      typeValues->dwTypes = (dwType_t *)ckrealloc((char *)typeValues->dwTypes, sizeof(dwType_t) * typeValues->maxDwType);
      if (typeValues->dwTypes == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
  dwTypeInfo2 = &typeValues->dwTypes[typeValues->numDwType];
  *dwTypeInfo2 = *dwTypeInfo;
  *typeIdx = typeValues->numDwType;
  typeValues->numDwType++;
  return result;
}

// =================================== checkDieTypeRefIdx =========================== 

static uint8_t checkDieTypeRefIdx(dwarfDbgPtr_t self) {
  uint8_t result;
  int idx = 0;
  int total = 0;
  int isSibling = 0;
  int maxEntries = 0;
  int typeIdx = 0;
  compileUnit_t *compileUnit;
  dieAndChildrenInfo_t *dieAndChildrenInfo = NULL;
  dieInfo_t *dieInfo = NULL;
  const char *tagName2 = NULL;
  const char *tagName = NULL;

  result = DWARF_DBG_ERR_OK;
  compileUnit = self->dwarfDbgCompileUnitInfo->currCompileUnit;
//printf("compileUnit: %p\n", compileUnit);
  for (idx = 0; idx < compileUnit->numDieAndChildren; idx++) {
    dieAndChildrenInfo = &compileUnit->dieAndChildrenInfos[idx];
//printf("dieAndChildrenInfo: %p\n", dieAndChildrenInfo);
    for (isSibling = 0; isSibling < 2; isSibling++) {
      if (isSibling) {
        maxEntries = dieAndChildrenInfo->numSiblings;
      } else {
        maxEntries = dieAndChildrenInfo->numChildren;
      }
      for(typeIdx = 0; typeIdx < maxEntries; typeIdx++) {
        if (isSibling) {
          dieInfo = &dieAndChildrenInfo->dieSiblings[typeIdx];
        } else {
          dieInfo = &dieAndChildrenInfo->dieChildren[typeIdx];
        }
//printf("dieInfo: %p isSibling: %d maxEntries: %d typeIdx: %d\n", dieInfo, isSibling, maxEntries, typeIdx);
self->dwarfDbgStringInfo->getDW_TAG_string(self, dieInfo->tag, &tagName);
//printf("tag: %s 0x%04x typeIdx: %d offset: 0x%08x refOffset: 0x%08x dieInfo: %p\n", tagName, dieInfo->tag, typeIdx, dieInfo->offset, dieAttr->refOffset, dieInfo);
        total++;
        printf("dieInfo: %p %d tagName: %s tagRefIdx: %d\n", dieInfo, total, tagName, dieInfo->tagRefIdx);
      }
    }
  }
  return result;
}

// =================================== getTypeRefIdx =========================== 

static uint8_t getTypeRefIdx(dwarfDbgPtr_t self, dieAttr_t *dieAttr, int *dwTypeIdx) {
  uint8_t result;
  int idx2 = 0;
  int isSibling = 0;
  int isSibling2 = 0;
  int maxEntries2 = 0;
  int typeIdx = 0;
  compileUnit_t *compileUnit;
  dieAndChildrenInfo_t *dieAndChildrenInfo2 = NULL;
  dieInfo_t *dieInfo2 = NULL;
  const char *tagName2 = NULL;
  const char *tagName = NULL;

  result = DWARF_DBG_ERR_OK;
  compileUnit = self->dwarfDbgCompileUnitInfo->currCompileUnit;
//printf("compileUnit: %p\n", compileUnit);
  for (idx2 = 0; idx2 < compileUnit->numDieAndChildren; idx2++) {
    dieAndChildrenInfo2 = &compileUnit->dieAndChildrenInfos[idx2];
//printf("dieAndChildrenInfo2: %p\n", dieAndChildrenInfo2);
    for (isSibling = 0; isSibling < 2; isSibling++) {
      if (isSibling) {
        maxEntries2 = dieAndChildrenInfo2->numSiblings;
      } else {
        maxEntries2 = dieAndChildrenInfo2->numChildren;
      }
      for(typeIdx = 0; typeIdx < maxEntries2; typeIdx++) {
        if (isSibling) {
          dieInfo2 = &dieAndChildrenInfo2->dieSiblings[typeIdx];
        } else {
          dieInfo2 = &dieAndChildrenInfo2->dieChildren[typeIdx];
        }
//printf("dieInfo2: %p dieAttr: %p isSibling: %d isSibling2: %d maxEntries2: %d typeIdx: %d\n", dieInfo2, dieAttr, isSibling, isSibling2, maxEntries2, typeIdx);
self->dwarfDbgStringInfo->getDW_TAG_string(self, dieInfo2->tag, &tagName2);
//printf("tag: %s 0x%04x typeIdx: %d offset: 0x%08x refOffset: 0x%08x dieInfo2: %p\n", tagName2, dieInfo2->tag, typeIdx, dieInfo2->offset, dieAttr->refOffset, dieInfo2);

        switch (dieInfo2->tag) {
        case DW_TAG_base_type:
          if (dieInfo2->offset == dieAttr->refOffset) {
printf("found type: %s baseTypeIdx: %d dieInfo2->offset: 0x%08x tagRefIdx: %d\n", tagName2, typeIdx, dieInfo2->offset, dieInfo2->tagRefIdx);
            *dwTypeIdx = dieInfo2->tagRefIdx;
            return result;
          }
          break;
        case DW_TAG_structure_type:
          if (dieInfo2->offset == dieAttr->refOffset) {
printf("found type: %s structureTypeIdx: %d dieInfo2->offset: 0x%08x tagRefIdx: %d\n", tagName2, typeIdx, dieInfo2->offset, dieInfo2->tagRefIdx);
            *dwTypeIdx = dieInfo2->tagRefIdx;
            return result;
          }
          break;
        case DW_TAG_typedef:
          if (dieInfo2->offset == dieAttr->refOffset) {
printf("found type: %s typedefIdx: %d dieInfo2->offset: 0x%08x tagRefIdx: %d\n", tagName2, typeIdx, dieInfo2->offset, dieInfo2->tagRefIdx);
            *dwTypeIdx = dieInfo2->tagRefIdx;
            return result;
          }
          break;
        case DW_TAG_subroutine_type:
          if (dieInfo2->offset == dieAttr->refOffset) {
printf("found type: %s subroutineTypeIdx: %d dieInfo2->offset: 0x%08x tagRefIdx: %d\n", tagName2, typeIdx, dieInfo2->offset, dieInfo2->tagRefIdx);
            *dwTypeIdx = dieInfo2->tagRefIdx;
            return result;
          }
          break;
        case DW_TAG_const_type:
          if (dieInfo2->offset == dieAttr->refOffset) {
printf("found type: %s constTypeIdx: %d dieInfo2->offset: 0x%08x tagRefIdx: %d\n", tagName2, typeIdx, dieInfo2->offset, dieInfo2->tagRefIdx);
            *dwTypeIdx = dieInfo2->tagRefIdx;
            return result;
          }
          break;
        case DW_TAG_array_type:
          if (dieInfo2->offset == dieAttr->refOffset) {
printf("found type: %s arrayTypeIdx: %d dieInfo2->offset: 0x%08x tagRefIdx: %d\n", tagName2, typeIdx, dieInfo2->offset, dieInfo2->tagRefIdx);
            *dwTypeIdx = dieInfo2->tagRefIdx;
            return result;
          }
          break;
        case DW_TAG_pointer_type:
          if (dieInfo2->offset == dieAttr->refOffset) {
printf("found type: %s pointerTypeIdx: %d dieInfo2->offset: 0x%08x tagRefIdx: %d\n", tagName2, typeIdx, dieInfo2->offset, dieInfo2->tagRefIdx);
            *dwTypeIdx = dieInfo2->tagRefIdx;
            return result;
          }
          break;
        case DW_TAG_union_type: 
          if (dieInfo2->offset == dieAttr->refOffset) {
printf("found type: %s unionTypeIdx: %d dieInfo2->offset: 0x%08x tagRefIdx: %d\n", tagName2, typeIdx, dieInfo2->offset, dieInfo2->tagRefIdx);
            *dwTypeIdx = dieInfo2->tagRefIdx;
            return result;
          }
          break;
        case DW_TAG_enumeration_type:
          if (dieInfo2->offset == dieAttr->refOffset) {
printf("found type: %s enumerationTypeIdx: %d dieInfo2->offset: 0x%08x tagRefIdx: %d\n", tagName2, typeIdx, dieInfo2->offset, dieInfo2->tagRefIdx);
            *dwTypeIdx = dieInfo2->tagRefIdx;
            return result;
          }
          break;
        case DW_TAG_volatile_type:
          if (dieInfo2->offset == dieAttr->refOffset) {
printf("found type: %s volatileTypeIdx: %d dieInfo2->offset: 0x%08x tagRefIdx: %d\n", tagName2, typeIdx, dieInfo2->offset, dieInfo2->tagRefIdx);
            *dwTypeIdx = dieInfo2->tagRefIdx;
            return result;
          }
          break;
        default:
self->dwarfDbgStringInfo->getDW_TAG_string(self, dieInfo2->tag, &tagName);
            if (dieInfo2->offset == dieAttr->refOffset) {
printf("found default: %s dieInfo2->offset: 0x%08x dieAttr->refOffset: 0x%08x\n", tagName, dieInfo2->offset, dieAttr->refOffset);
          }
          break;
        }
      }
    }
  }
printf("ERROR TYPE_REF_NOT_FOUND\n");
  return DWARF_DBG_ERR_TYPE_REF_NOT_FOUND;
}

// =================================== handleType =========================== 

static uint8_t handleType(dwarfDbgPtr_t self, dieAndChildrenInfo_t *dieAndChildrenInfo, dieInfo_t *dieInfo, int isSibling, dwTypeValues_t *dwTypeValues) {
  uint8_t result = 0;
  int entryIdx = 0;
  int typeIdx = 0;
  int maxEntries = 0;
  int attrIdx = 0;
  dieInfo_t *dieInfo2 = NULL;
  dieAttr_t *dieAttr = NULL;
  dwType_t dwTypeInfo;
  int found = 0;
  const char *tagName = NULL;
  const char *attrName = NULL;

  result = DWARF_DBG_ERR_OK;
  if (isSibling) {
    maxEntries = dieAndChildrenInfo->numSiblings;
  } else {
    maxEntries = dieAndChildrenInfo->numChildren;
  }
  dwTypeInfo.bitOffset = -1;
  dwTypeInfo.bitSize = -1;
  dwTypeInfo.byteSize = -1;
  dwTypeInfo.constValue = -1;
  dwTypeInfo.location = 0;
  dwTypeInfo.declaration = 0;
  dwTypeInfo.pathNameIdx = -1;
  dwTypeInfo.lineNo = -1;
  dwTypeInfo.encoding = -1;
  attrName = NULL;
  dwTypeInfo.prototyped = 0;
  dwTypeInfo.siblingIdx = -1;
  dwTypeInfo.dwTypeIdx = -1;
  found = 0;
//currTypeFlags = ATTR_arrayTypeFlags;
  for(attrIdx = 0; attrIdx < dieInfo->numAttr; attrIdx++) {
    dieAttr = &dieInfo->dieAttrs[attrIdx];
self->dwarfDbgStringInfo->getDW_AT_string(self, dieAttr->attr, &attrName);
printf("handleType: attrName: %s\n", attrName);
    switch (dieAttr->attr) {
    case DW_AT_bit_offset:
      dwTypeInfo.bitOffset = dieAttr->uval;
      found++;
      break;
    case DW_AT_bit_size:
      dwTypeInfo.bitSize = dieAttr->uval;
      found++;
      break;
    case DW_AT_byte_size:
      dwTypeInfo.byteSize = dieAttr->byteSize;
      found++;
      break;
    case DW_AT_const_value:
      dwTypeInfo.constValue = dieAttr->uval;
      found++;
      break;
    case DW_AT_data_member_location:
      dwTypeInfo.location = dieAttr->uval;
printf("location: %d\n", dwTypeInfo.location);
      found++;
      break;
    case DW_AT_declaration:
      dwTypeInfo.declaration = dieAttr->uval;
      found++;
      break;
    case DW_AT_decl_file:
      dwTypeInfo.pathNameIdx = dieAttr->sourceFileIdx;
      found++;
      break;
    case DW_AT_decl_line:
      dwTypeInfo.lineNo = dieAttr->sourceLineNo;
      found++;
      break;
    case DW_AT_name:
      if ((dieAttr->attrStrIdx < 0) || (dieAttr->attrStrIdx >= self->dwarfDbgCompileUnitInfo->numAttrStr)) {
        return DWARF_DBG_ERR_BAD_ATTR_STR_IDX;
      }
      attrName = self->dwarfDbgCompileUnitInfo->attrStrs[dieAttr->attrStrIdx];
      found++;
      break;
    case DW_AT_prototyped:
      dwTypeInfo.prototyped = dieAttr->uval;
      found++;
      break;
    case DW_AT_sibling:
      dwTypeInfo.siblingIdx = dieAttr->uval;
      found++;
      break;
    case DW_AT_type:
      result = getTypeRefIdx(self, dieAttr, &dwTypeInfo.dwTypeIdx);
      checkErrOK(result);
      found++;
      break;
    default:
printf("ERROR: DWARF_DBG_ERR_UNEXPECTED_ATTR_IN_ARRAY_TYPE: 0x%04x\n", dieAttr->attr);
      return DWARF_DBG_ERR_UNEXPECTED_ATTR_IN_ARRAY_TYPE;
    }
  }
  if (found == dieInfo->numAttr) {
    result = self->dwarfDbgTypeInfo->addType(self, &dwTypeInfo, attrName, dwTypeValues, &typeIdx);
    checkErrOK(result);
    dieInfo->tagRefIdx = typeIdx;
    dieInfo->flags  = TAG_REF_ARRAY_TYPE;
  } else {
printf("ERROR arrayType not found found: %d numAttr: %d offset: 0x%08x\n", found, dieInfo->numAttr, dieAttr->refOffset);
    return DWARF_DBG_ERR_ARRAY_TYPE_NOT_FOUND;
  }
  return result;
}

// =================================== addTypes =========================== 

static uint8_t addTypes(dwarfDbgPtr_t self, int dieAndChildrenIdx, int flags, Dwarf_Bool isSibling) {
  uint8_t result = 0;
  int entryIdx = 0;
  int typeIdx = 0;
  int typeDefIdx = 0;
  int maxEntries = 0;
  int attrIdx = 0;
  dieAndChildrenInfo_t *dieAndChildrenInfo = NULL;
  compileUnit_t *compileUnit = NULL;
  dieInfo_t *dieInfo = NULL;
  dieInfo_t *dieInfo2 = NULL;
  dieAttr_t *dieAttr = NULL;
  int found = 0;
  int tagToHandle = 0;
  char *attrName = NULL;
  const char *tagName = NULL;

  result = DWARF_DBG_ERR_OK;
  compileUnit = self->dwarfDbgCompileUnitInfo->currCompileUnit;
  dieAndChildrenInfo = &compileUnit->dieAndChildrenInfos[dieAndChildrenIdx];
  if (isSibling) {
    maxEntries = dieAndChildrenInfo->numSiblings;
  } else {
    maxEntries = dieAndChildrenInfo->numChildren;
  }
  tagToHandle = 0;
  if (flags & TAG_REF_ARRAY_TYPE) {
    tagToHandle = DW_TAG_array_type;
  }
  if (flags & TAG_REF_BASE_TYPE) {
    tagToHandle = DW_TAG_base_type;
  }
  if (flags & TAG_REF_CONST_TYPE) {
    tagToHandle = DW_TAG_const_type;
  }
  if (flags & TAG_REF_ENUMERATION_TYPE) {
    tagToHandle = DW_TAG_enumeration_type;
  }
  if (flags & TAG_REF_ENUMERATOR) {
    tagToHandle = DW_TAG_enumerator;
  }
  if (flags & TAG_REF_MEMBER) {
    tagToHandle = DW_TAG_member;
  }
  if (flags & TAG_REF_POINTER_TYPE) {
    tagToHandle = DW_TAG_pointer_type;
  }
  if (flags & TAG_REF_STRUCTURE_TYPE) {
    tagToHandle = DW_TAG_structure_type;
  }
  if (flags & TAG_REF_SUBROUTINE_TYPE) {
    tagToHandle = DW_TAG_subroutine_type;
  }
  if (flags & TAG_REF_TYPEDEF) {
    tagToHandle = DW_TAG_typedef;
  }
  if (flags & TAG_REF_UNION_TYPE) {
    tagToHandle = DW_TAG_union_type;
  }
  if (flags & TAG_REF_VOLATILE_TYPE) {
    tagToHandle = DW_TAG_volatile_type;
  }
  if (tagToHandle == 0) {
printf("FLAGS: 0x%04x\n", flags);
    return DWARF_DBG_ERR_BAD_TAG_REF_TYPE;
  }
  for(entryIdx = 0; entryIdx < maxEntries; entryIdx++) {
    if (isSibling) {
      dieInfo = &dieAndChildrenInfo->dieSiblings[entryIdx];
    } else {
      dieInfo = &dieAndChildrenInfo->dieChildren[entryIdx];
    }
    if (dieInfo->tag == tagToHandle) {
      switch (tagToHandle) {
      case DW_TAG_array_type:
        result = self->dwarfDbgTypeInfo->handleType(self, dieAndChildrenInfo, dieInfo, isSibling, &self->dwarfDbgTypeInfo->dwArrayTypeInfos);
        checkErrOK(result);
        break;
      case DW_TAG_const_type:
        result = self->dwarfDbgTypeInfo->handleType(self, dieAndChildrenInfo, dieInfo, isSibling, &self->dwarfDbgTypeInfo->dwConstTypeInfos);
        checkErrOK(result);
        break;
      case DW_TAG_enumeration_type:
printf("1\n");
        result = self->dwarfDbgTypeInfo->handleType(self, dieAndChildrenInfo, dieInfo, isSibling, &self->dwarfDbgTypeInfo->dwEnumerationTypeInfos);
printf("2\n");
        checkErrOK(result);
        break;
      case DW_TAG_enumerator:
        result = self->dwarfDbgTypeInfo->handleType(self, dieAndChildrenInfo, dieInfo, isSibling, &self->dwarfDbgTypeInfo->dwEnumeratorInfos);
        checkErrOK(result);
        break;
      case DW_TAG_member:
        result = self->dwarfDbgTypeInfo->handleType(self, dieAndChildrenInfo, dieInfo, isSibling, &self->dwarfDbgTypeInfo->dwMemberInfos);
        checkErrOK(result);
        break;
      case DW_TAG_pointer_type:
        result = self->dwarfDbgTypeInfo->handleType(self, dieAndChildrenInfo, dieInfo, isSibling, &self->dwarfDbgTypeInfo->dwPointerTypeInfos);
        checkErrOK(result);
        break;
      case DW_TAG_structure_type:
        result = self->dwarfDbgTypeInfo->handleType(self, dieAndChildrenInfo, dieInfo, isSibling, &self->dwarfDbgTypeInfo->dwStructureTypeInfos);
        checkErrOK(result);
        break;
      case DW_TAG_subroutine_type:
        result = self->dwarfDbgTypeInfo->handleType(self, dieAndChildrenInfo, dieInfo, isSibling, &self->dwarfDbgTypeInfo->dwSubroutineTypeInfos);
        checkErrOK(result);
        break;
      case DW_TAG_typedef:
        result = self->dwarfDbgTypeInfo->handleType(self, dieAndChildrenInfo, dieInfo, isSibling, &self->dwarfDbgTypeInfo->dwTypedefInfos);
        checkErrOK(result);
        break;
      case DW_TAG_union_type:
        result = self->dwarfDbgTypeInfo->handleType(self, dieAndChildrenInfo, dieInfo, isSibling, &self->dwarfDbgTypeInfo->dwUnionTypeInfos);
        checkErrOK(result);
        break;
      case DW_TAG_volatile_type:
        result = self->dwarfDbgTypeInfo->handleType(self, dieAndChildrenInfo, dieInfo, isSibling, &self->dwarfDbgTypeInfo->dwVolatileTypeInfos);
        checkErrOK(result);
        break;
      default:
printf("ERRROr unexpected tag: 0x%04x\n", tagToHandle);
        break;
      }
    }
  }
  return result;
}

// =================================== addChildrenTypes =========================== 

static uint8_t addChildrenTypes(dwarfDbgPtr_t self, int dieAndChildrenIdx, int flags) {
  uint8_t result = 0;

  result = DWARF_DBG_ERR_OK;
  result = self->dwarfDbgTypeInfo->addTypes(self, dieAndChildrenIdx, flags, /* isSibling */ 0);
  checkErrOK(result);
  return result;
}

// =================================== addSiblingsTypes =========================== 

static uint8_t addSiblingsTypes(dwarfDbgPtr_t self, int dieAndChildrenIdx, int flags) {
  uint8_t result = 0;

  result = DWARF_DBG_ERR_OK;
  result = self->dwarfDbgTypeInfo->addTypes(self, dieAndChildrenIdx, flags, /* isSibling */ 1);
  checkErrOK(result);
  return result;
}

// =================================== dwarfDbgTypeInfoInit =========================== 

int dwarfDbgTypeInfoInit (dwarfDbgPtr_t self) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;
  self->dwarfDbgTypeInfo->addTypeStr = &addTypeStr;
  self->dwarfDbgTypeInfo->checkDieTypeRefIdx = &checkDieTypeRefIdx;

  self->dwarfDbgTypeInfo->addType = &addType;

  self->dwarfDbgTypeInfo->handleType = &handleType;

  self->dwarfDbgTypeInfo->addTypes = &addTypes;
  self->dwarfDbgTypeInfo->addChildrenTypes = &addChildrenTypes;
  self->dwarfDbgTypeInfo->addSiblingsTypes = &addSiblingsTypes;
  return result;
}
