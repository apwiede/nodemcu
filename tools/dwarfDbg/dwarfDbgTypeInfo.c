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

static uint8_t addType(dwarfDbgPtr_t self, dwType_t *dwTypeInfo, const char *name, dwTypeValues_t *typeValues, int *typeIdx) {
  uint8_t result = 0;
  int typeStrIdx = 0;
  int idx = 0;
  dwType_t *dwTypeInfo2 = NULL;

  result = DWARF_DBG_ERR_OK;
  if (name != NULL) {
    result = self->dwarfDbgTypeInfo->addTypeStr(self, name, &dwTypeInfo->typeNameIdx);
    checkErrOK(result);
  }
if (name != NULL) {
printf("addType: name: %s\n", name);
}
  for(idx = 0; idx < typeValues->numDwType; idx++) {
    dwTypeInfo2 = &typeValues->dwTypes[idx];
    if (dwTypeInfo2->artificial == dwTypeInfo->artificial) {
     if (dwTypeInfo2->abstractOrigin == dwTypeInfo->abstractOrigin) {
      if (dwTypeInfo2->bitOffset == dwTypeInfo->bitOffset) {
       if (dwTypeInfo2->bitSize == dwTypeInfo->bitSize) {
        if (dwTypeInfo2->byteSize == dwTypeInfo->byteSize) {
         if (dwTypeInfo2->callFileIdx == dwTypeInfo->callFileIdx) {
          if (dwTypeInfo2->callLineNo == dwTypeInfo->callLineNo) {
           if (dwTypeInfo2->constValue == dwTypeInfo->constValue) {
            if (dwTypeInfo2->dataLocation == dwTypeInfo->dataLocation) {
             if (dwTypeInfo2->declaration == dwTypeInfo->declaration) {
              if (dwTypeInfo2->pathNameIdx == dwTypeInfo->pathNameIdx) {
               if (dwTypeInfo2->frameBase == dwTypeInfo->frameBase) {
                if (dwTypeInfo2->GNUAllCallSites == dwTypeInfo->GNUAllCallSites) {
                 if (dwTypeInfo2->GNUAllTailCallSites == dwTypeInfo->GNUAllTailCallSites) {
                  if (dwTypeInfo2->GNUCallSiteValue == dwTypeInfo->GNUCallSiteValue) {
                   if (dwTypeInfo2->GNUCallSiteTarget == dwTypeInfo->GNUCallSiteTarget) {
                    if (dwTypeInfo2->highPc == dwTypeInfo->highPc) {
                     if (dwTypeInfo2->encoding == dwTypeInfo->encoding) {
                      if (dwTypeInfo2->entryPc == dwTypeInfo->entryPc) {
                       if (dwTypeInfo2->lineNo == dwTypeInfo->lineNo) {
                        if (dwTypeInfo2->location == dwTypeInfo->location) {
                         if (dwTypeInfo2->linkageName == dwTypeInfo->linkageName) {
                          if (dwTypeInfo2->lowPc == dwTypeInfo->lowPc) {
                           if (dwTypeInfo2->external == dwTypeInfo->external) {
                            if (dwTypeInfo2->isInline == dwTypeInfo->isInline) {
                             if (dwTypeInfo2->typeNameIdx == dwTypeInfo->typeNameIdx) {
                              if (dwTypeInfo2->prototyped == dwTypeInfo->prototyped) {
                               if (dwTypeInfo2->ranges == dwTypeInfo->ranges) {
                                if (dwTypeInfo2->siblingIdx == dwTypeInfo->siblingIdx) {
                                 if (dwTypeInfo2->subrangeType == dwTypeInfo->subrangeType) {
                                  if (dwTypeInfo2->dwTypeIdx == dwTypeInfo->dwTypeIdx) {
                                   if (dwTypeInfo2->upperBound == dwTypeInfo->upperBound) {
if (name != NULL) {
printf("addType: found: %s typeIdx: %d\n", name, idx);
} else {
printf("addType: found: typeIdx: %d\n", idx);
}
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
  int idx = 0;
  int isSibling = 0;
  int maxEntries = 0;
  int typeIdx = 0;
  compileUnit_t *compileUnit;
  dieAndChildrenInfo_t *dieAndChildrenInfo = NULL;
  dieInfo_t *dieInfo = NULL;
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
//printf("dieInfo: %p dieAttr: %p isSibling: %d maxEntries: %d typeIdx: %d\n", dieInfo, dieAttr, isSibling, maxEntries, typeIdx);
self->dwarfDbgStringInfo->getDW_TAG_string(self, dieInfo->tag, &tagName);
//printf("tag: %s 0x%04x typeIdx: %d offset: 0x%08x refOffset: 0x%08x dieInfo: %p\n", tagName, dieInfo->tag, typeIdx, dieInfo->offset, dieAttr->refOffset, dieInfo);

        if ((dieInfo->offset == dieAttr->refOffset) && (dieInfo->tagRefIdx == -1)) {
printf("missing tagRefIdx for: %s\n", tagName);
          result =self->dwarfDbgTypeInfo->handleType(self, dieInfo);
          checkErrOK(result);
        }
        switch (dieInfo->tag) {
        case DW_TAG_array_type:
          if (dieInfo->offset == dieAttr->refOffset) {
printf("found type: %s arrayTypeIdx: %d dieInfo->offset: 0x%08x tagRefIdx: %d\n", tagName, typeIdx, dieInfo->offset, dieInfo->tagRefIdx);
            *dwTypeIdx = dieInfo->tagRefIdx;
            return result;
          }
          break;
        case DW_TAG_base_type:
          if (dieInfo->offset == dieAttr->refOffset) {
printf("found type: %s baseTypeIdx: %d dieInfo->offset: 0x%08x tagRefIdx: %d\n", tagName, typeIdx, dieInfo->offset, dieInfo->tagRefIdx);
            *dwTypeIdx = dieInfo->tagRefIdx;
            return result;
          }
          break;
        case DW_TAG_const_type:
          if (dieInfo->offset == dieAttr->refOffset) {
printf("found type: %s constTypeIdx: %d dieInfo->offset: 0x%08x tagRefIdx: %d\n", tagName, typeIdx, dieInfo->offset, dieInfo->tagRefIdx);
            *dwTypeIdx = dieInfo->tagRefIdx;
            return result;
          }
          break;
        case DW_TAG_enumeration_type:
          if (dieInfo->offset == dieAttr->refOffset) {
printf("found type: %s enumerationTypeIdx: %d dieInfo->offset: 0x%08x tagRefIdx: %d\n", tagName, typeIdx, dieInfo->offset, dieInfo->tagRefIdx);
            *dwTypeIdx = dieInfo->tagRefIdx;
            return result;
          }
          break;
        case DW_TAG_pointer_type:
          if (dieInfo->offset == dieAttr->refOffset) {
printf("found type: %s pointerTypeIdx: %d dieInfo->offset: 0x%08x tagRefIdx: %d\n", tagName, typeIdx, dieInfo->offset, dieInfo->tagRefIdx);
            *dwTypeIdx = dieInfo->tagRefIdx;
            return result;
          }
          break;
        case DW_TAG_structure_type:
          if (dieInfo->offset == dieAttr->refOffset) {
printf("found type: %s structureTypeIdx: %d dieInfo->offset: 0x%08x tagRefIdx: %d\n", tagName, typeIdx, dieInfo->offset, dieInfo->tagRefIdx);
            *dwTypeIdx = dieInfo->tagRefIdx;
            return result;
          }
          break;
        case DW_TAG_subroutine_type:
          if (dieInfo->offset == dieAttr->refOffset) {
printf("found type: %s subroutineTypeIdx: %d dieInfo->offset: 0x%08x tagRefIdx: %d\n", tagName, typeIdx, dieInfo->offset, dieInfo->tagRefIdx);
            *dwTypeIdx = dieInfo->tagRefIdx;
            return result;
          }
          break;
        case DW_TAG_typedef:
          if (dieInfo->offset == dieAttr->refOffset) {
printf("found type: %s typedefIdx: %d dieInfo->offset: 0x%08x tagRefIdx: %d\n", tagName, typeIdx, dieInfo->offset, dieInfo->tagRefIdx);
            *dwTypeIdx = dieInfo->tagRefIdx;
            return result;
          }
          break;
        case DW_TAG_union_type: 
          if (dieInfo->offset == dieAttr->refOffset) {
printf("found type: %s unionTypeIdx: %d dieInfo->offset: 0x%08x tagRefIdx: %d\n", tagName, typeIdx, dieInfo->offset, dieInfo->tagRefIdx);
            *dwTypeIdx = dieInfo->tagRefIdx;
            return result;
          }
          break;
        case DW_TAG_volatile_type:
          if (dieInfo->offset == dieAttr->refOffset) {
printf("found type: %s volatileTypeIdx: %d dieInfo->offset: 0x%08x tagRefIdx: %d\n", tagName, typeIdx, dieInfo->offset, dieInfo->tagRefIdx);
            *dwTypeIdx = dieInfo->tagRefIdx;
            return result;
          }
          break;
        default:
self->dwarfDbgStringInfo->getDW_TAG_string(self, dieInfo->tag, &tagName);
            if (dieInfo->offset == dieAttr->refOffset) {
printf("found default: %s dieInfo->offset: 0x%08x dieAttr->refOffset: 0x%08x\n", tagName, dieInfo->offset, dieAttr->refOffset);
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

static uint8_t handleType(dwarfDbgPtr_t self, dieInfo_t *dieInfo) {
  uint8_t result = 0;
  int entryIdx = 0;
  int typeIdx = 0;
  int attrIdx = 0;
  dieInfo_t *dieInfo2 = NULL;
  dieAttr_t *dieAttr = NULL;
  dwTypeValues_t *dwTypeValues;
  dwType_t dwTypeInfo;
  int found = 0;
  const char *tagName = NULL;
  const char *attrName = NULL;
  const char *atName = NULL;
int offset = -1;

  switch (dieInfo->tag) {
  case DW_TAG_array_type:
    dwTypeValues = &self->dwarfDbgTypeInfo->dwArrayTypeInfos;
    break;
  case DW_TAG_const_type:
    dwTypeValues = &self->dwarfDbgTypeInfo->dwConstTypeInfos;
    break;
  case DW_TAG_enumeration_type:
    dwTypeValues = &self->dwarfDbgTypeInfo->dwEnumerationTypeInfos;
    break;
  case DW_TAG_enumerator:
    dwTypeValues = &self->dwarfDbgTypeInfo->dwEnumeratorInfos;
    break;
  case DW_TAG_formal_parameter:
    dwTypeValues = &self->dwarfDbgTypeInfo->dwFormalParametersInfos;
    break;
  case DW_TAG_GNU_call_site:
    dwTypeValues = &self->dwarfDbgTypeInfo->dwGNUCallSiteInfos;
    break;
  case DW_TAG_GNU_call_site_parameter:
    dwTypeValues = &self->dwarfDbgTypeInfo->dwGNUCallSiteParameterInfos;
    break;
  case DW_TAG_label:
    dwTypeValues = &self->dwarfDbgTypeInfo->dwLabelInfos;
    break;
  case DW_TAG_inlined_subroutine:
    dwTypeValues = &self->dwarfDbgTypeInfo->dwInlinedSubroutineInfos;
    break;
  case DW_TAG_lexical_block:
    dwTypeValues = &self->dwarfDbgTypeInfo->dwLexicalBlockInfos;
    break;
  case DW_TAG_member:
    dwTypeValues = &self->dwarfDbgTypeInfo->dwMemberInfos;
    break;
  case DW_TAG_pointer_type:
    dwTypeValues = &self->dwarfDbgTypeInfo->dwPointerTypeInfos;
    break;
  case DW_TAG_structure_type:
    dwTypeValues = &self->dwarfDbgTypeInfo->dwStructureTypeInfos;
    break;
  case DW_TAG_subroutine_type:
    dwTypeValues = &self->dwarfDbgTypeInfo->dwSubroutineTypeInfos;
    break;
  case DW_TAG_subprogram:
    dwTypeValues = &self->dwarfDbgTypeInfo->dwSubprogramInfos;
    break;
  case DW_TAG_subrange_type:
    dwTypeValues = &self->dwarfDbgTypeInfo->dwSubrangeTypeInfos;
    break;
  case DW_TAG_typedef:
    dwTypeValues = &self->dwarfDbgTypeInfo->dwTypedefInfos;
    break;
  case DW_TAG_union_type:
    dwTypeValues = &self->dwarfDbgTypeInfo->dwUnionTypeInfos;
    break;
  case DW_TAG_unspecified_parameters:
    dwTypeValues = &self->dwarfDbgTypeInfo->dwUnspecifiedParametersInfos;
    break;
  case DW_TAG_variable:
    dwTypeValues = &self->dwarfDbgTypeInfo->dwVariableInfos;
    break;
  case DW_TAG_volatile_type:
    dwTypeValues = &self->dwarfDbgTypeInfo->dwVolatileTypeInfos;
    break;
  default:
printf("ERROR bad tagType: 0x%04x\n", dieInfo->tag);
    return DWARF_DBG_ERR_BAD_TAG_TYPE;
    break;
  }
  result = DWARF_DBG_ERR_OK;
  dwTypeInfo.artificial = -1;
  dwTypeInfo.abstractOrigin = -1;
  dwTypeInfo.bitOffset = -1;
  dwTypeInfo.bitSize = -1;
  dwTypeInfo.byteSize = -1;
  dwTypeInfo.callFileIdx = -1;
  dwTypeInfo.callLineNo = -1;
  dwTypeInfo.constValue = -1;
  dwTypeInfo.dataLocation = 0;
  dwTypeInfo.declaration = 0;
  dwTypeInfo.pathNameIdx = -1;
  dwTypeInfo.lineNo = -1;
  dwTypeInfo.encoding = -1;
  dwTypeInfo.entryPc = 0;
  dwTypeInfo.external = 0;
  dwTypeInfo.frameBase = -1;
  dwTypeInfo.GNUAllCallSites = -1;
  dwTypeInfo.GNUAllTailCallSites = -1;
  dwTypeInfo.GNUCallSiteTarget = -1;
  dwTypeInfo.GNUCallSiteValue = -1;
  dwTypeInfo.highPc = -1;
  dwTypeInfo.isInline = -1;
  dwTypeInfo.location = -1;
  dwTypeInfo.linkageName = -1;
  dwTypeInfo.lowPc = -1;
  attrName = NULL;
  dwTypeInfo.typeNameIdx = -1;
  dwTypeInfo.prototyped = 0;
  dwTypeInfo.ranges = -1;
  dwTypeInfo.siblingIdx = -1;
  dwTypeInfo.subrangeType = -1;
  dwTypeInfo.dwTypeIdx = -1;
  dwTypeInfo.upperBound = -1;
  found = 0;
//currTypeFlags = ATTR_arrayTypeFlags;
self->dwarfDbgStringInfo->getDW_TAG_string(self, dieInfo->tag, &tagName);
printf("tagName: %s\n", tagName);
  for(attrIdx = 0; attrIdx < dieInfo->numAttr; attrIdx++) {
    dieAttr = &dieInfo->dieAttrs[attrIdx];
self->dwarfDbgStringInfo->getDW_AT_string(self, dieAttr->attr, &attrName);
printf("handleType: attrName: %s\n", attrName);
    switch (dieAttr->attr) {
    case DW_AT_artificial:
      dwTypeInfo.artificial = dieAttr->uval;
      found++;
      break;
    case DW_AT_abstract_origin:
      dwTypeInfo.abstractOrigin = dieAttr->uval;
      found++;
      break;
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
    case DW_AT_call_file:
      dwTypeInfo.callFileIdx = dieAttr->uval;
      found++;
      break;
    case DW_AT_call_line:
      dwTypeInfo.callLineNo = dieAttr->uval;
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
    case DW_AT_entry_pc:
      dwTypeInfo.entryPc = dieAttr->uval;
      found++;
      break;
    case DW_AT_external:
      dwTypeInfo.external = dieAttr->uval;
      found++;
      break;
    case DW_AT_frame_base:
      dwTypeInfo.frameBase = dieAttr->uval;
      found++;
      break;
    case DW_AT_GNU_all_call_sites:
      dwTypeInfo.GNUAllCallSites = dieAttr->uval;
      found++;
      break;
    case DW_AT_GNU_all_tail_call_sites:
      dwTypeInfo.GNUAllTailCallSites = dieAttr->uval;
      found++;
      break;
    case DW_AT_GNU_call_site_target:
      dwTypeInfo.GNUCallSiteTarget = dieAttr->uval;
      found++;
      break;
    case DW_AT_GNU_call_site_value:
      dwTypeInfo.GNUCallSiteValue = dieAttr->uval;
      found++;
      break;
    case DW_AT_high_pc:
      dwTypeInfo.highPc = dieAttr->uval;
      found++;
      break;
    case DW_AT_inline:
      dwTypeInfo.isInline = dieAttr->uval;
      found++;
      break;
    case DW_AT_linkage_name:
      dwTypeInfo.linkageName = dieAttr->uval;
      found++;
      break;
    case DW_AT_location:
      dwTypeInfo.location = dieAttr->uval;
      found++;
      break;
    case DW_AT_low_pc:
      dwTypeInfo.lowPc = dieAttr->uval;
      found++;
      break;
    case DW_AT_name:
      if ((dieAttr->attrStrIdx < 0) || (dieAttr->attrStrIdx >= self->dwarfDbgCompileUnitInfo->numAttrStr)) {
        return DWARF_DBG_ERR_BAD_ATTR_STR_IDX;
      }
      atName = self->dwarfDbgCompileUnitInfo->attrStrs[dieAttr->attrStrIdx];
if (dieInfo->tag == DW_TAG_typedef) {
printf("typedef: name: %s\n", atName);
}
      found++;
      break;
    case DW_AT_prototyped:
      dwTypeInfo.prototyped = dieAttr->uval;
      found++;
      break;
    case DW_AT_ranges:
      dwTypeInfo.ranges = dieAttr->uval;
      found++;
      break;
    case DW_AT_sibling:
      dwTypeInfo.siblingIdx = dieAttr->uval;
      found++;
      break;
    case DW_AT_type:
//if (dieInfo->tag == DW_TAG_typedef) {
offset = dieAttr->refOffset;
printf("AT_type: refOffset: 0x%08x\n", dieAttr->refOffset);
//}
      result = getTypeRefIdx(self, dieAttr, &dwTypeInfo.dwTypeIdx);
      checkErrOK(result);
if (dwTypeInfo.dwTypeIdx == -1) {
printf("unresolved tagRefIdx\n");
}
      found++;
      break;
    case DW_AT_upper_bound:
      dwTypeInfo.upperBound = dieAttr->uval;
      found++;
      break;
    default:
printf("ERROR: DWARF_DBG_ERR_UNEXPECTED_ATTR_IN_TYPE: 0x%04x\n", dieAttr->attr);
      return DWARF_DBG_ERR_UNEXPECTED_ATTR_IN_TYPE;
    }
  }
  if (found == dieInfo->numAttr) {
if (dieInfo->tag == DW_TAG_typedef) {
printf("TD: %s pathNameIdx: %d lineNo: %d dwTypeIdx: %d offset: 0x%08x\n", atName, dwTypeInfo.pathNameIdx, dwTypeInfo.lineNo, dwTypeInfo.dwTypeIdx, offset);
}
    result = self->dwarfDbgTypeInfo->addType(self, &dwTypeInfo, attrName, dwTypeValues, &typeIdx);
    checkErrOK(result);
    dieInfo->tagRefIdx = typeIdx;
    dieInfo->flags  = TAG_REF_ARRAY_TYPE;
  } else {
printf("ERROR type not found found: %d numAttr: %d offset: 0x%08x\n", found, dieInfo->numAttr, dieAttr->refOffset);
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
  if (flags & TAG_REF_FORMAL_PARAMETER) {
    tagToHandle = DW_TAG_formal_parameter;
  }
  if (flags & TAG_REF_GNU_CALL_SITE) {
    tagToHandle = DW_TAG_GNU_call_site;
  }
  if (flags & TAG_REF_GNU_CALL_SITE_PARAMETER) {
    tagToHandle = DW_TAG_GNU_call_site_parameter;
  }
  if (flags & TAG_REF_INLINED_SUBROUTINE) {
    tagToHandle = DW_TAG_inlined_subroutine;
  }
  if (flags & TAG_REF_LABEL) {
    tagToHandle = DW_TAG_label;
  }
  if (flags & TAG_REF_LEXICAL_BLOCK) {
    tagToHandle = DW_TAG_lexical_block;
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
  if (flags & TAG_REF_SUBPROGRAM) {
    tagToHandle = DW_TAG_subprogram;
  }
  if (flags & TAG_REF_SUBRANGE_TYPE) {
    tagToHandle = DW_TAG_subrange_type;
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
  if (flags & TAG_REF_UNSPECIFIED_PARAMETERS) {
    tagToHandle = DW_TAG_unspecified_parameters;
  }
  if (flags & TAG_REF_VARIABLE) {
    tagToHandle = DW_TAG_variable;
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
      result = self->dwarfDbgTypeInfo->handleType(self, dieInfo);
      checkErrOK(result);
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
