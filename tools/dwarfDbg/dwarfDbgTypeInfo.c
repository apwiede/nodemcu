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

static FILE *typeFd;

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

// =================================== addAttrType =========================== 

static uint8_t addAttrType(dwarfDbgPtr_t self, dwAttrTypeInfo_t *attrTypeInfo, int dwType, int value, int *attrTypeIdx) {
  uint8_t result = 0;
  int typeStrIdx = 0;
  int idx = 0;
  int entryIdx = 0;
  int found = 0;
  dwAttrType_t *dwAttrType = NULL;

  result = DWARF_DBG_ERR_OK;
  if (attrTypeInfo->maxDwAttrType <= attrTypeInfo->numDwAttrType) {
    attrTypeInfo->maxDwAttrType += 5;
    if (attrTypeInfo->dwAttrTypes == NULL) {
      attrTypeInfo->dwAttrTypes = (dwAttrType_t *)ckalloc(sizeof(dwAttrType_t) * attrTypeInfo->maxDwAttrType);
      if (attrTypeInfo->dwAttrTypes == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      attrTypeInfo->dwAttrTypes = (dwAttrType_t *)ckrealloc((char *)attrTypeInfo->dwAttrTypes, sizeof(dwAttrType_t) * attrTypeInfo->maxDwAttrType);
      if (attrTypeInfo->dwAttrTypes == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
  dwAttrType = &attrTypeInfo->dwAttrTypes[attrTypeInfo->numDwAttrType];
  dwAttrType->dwType = dwType;
  dwAttrType->value = value;
  *attrTypeIdx = attrTypeInfo->numDwAttrType;
  attrTypeInfo->numDwAttrType++;
  return result;
}

// =================================== addAttrTypeInfo =========================== 

static uint8_t addAttrTypeInfo(dwarfDbgPtr_t self, dwAttrTypeInfo_t *dwAttrTypeInfoIn, dwAttrTypeInfos_t *dwAttrTypeInfos, int *typeIdx) {
  uint8_t result = 0;
  int typeStrIdx = 0;
  int idx = 0;
  int entryIdx = 0;
  int found = 0;
  int attrTypeIdx = 0;
  dwAttrTypeInfo_t *dwAttrTypeInfo = NULL;
  dwAttrType_t *dwAttrType = NULL;
  dwAttrType_t *dwAttrTypeIn = NULL;
  const char *typeName = NULL;
  const char *tagName = NULL;
  char *name;
  char *dirName;
  char pathName[255];;
  pathNameInfo_t *pathNameInfo;

  self->dwarfDbgTypeInfo->typeLevel++;
  result = DWARF_DBG_ERR_OK;
  for(idx = 0; idx < dwAttrTypeInfos->numDwAttrTypeInfo; idx++) {
    dwAttrTypeInfo = &dwAttrTypeInfos->dwAttrTypeInfos[idx];
    found = 0;
    for (entryIdx = 0; entryIdx < dwAttrTypeInfo->numDwAttrType; entryIdx++) {
      if (dwAttrTypeInfoIn->numDwAttrType > entryIdx) {
        dwAttrType = &dwAttrTypeInfo->dwAttrTypes[entryIdx];
        dwAttrTypeIn = &dwAttrTypeInfoIn->dwAttrTypes[entryIdx];
        if (dwAttrType->dwType != dwAttrTypeIn->dwType) {
          break;
        }
        if (dwAttrType->value != dwAttrTypeIn->value) {
          break;
        }
//printf("idx: %d entryIdx: %d value: %d numAttrType: %d found: %d\n", idx, entryIdx, dwAttrType->value, dwAttrTypeInfo->numDwAttrType, found);
        found++;
      } else {
        break;
      }
    }
//printf("found: %d numAttr: %d\n", found, dwAttrTypeInfo->numDwAttrType - 1);
    if ((found == dwAttrTypeInfo->numDwAttrType) && (dwAttrTypeInfo->numDwAttrType > 0)) {
      *typeIdx = idx;
printf("found typeIdx: %d\n", idx);
result = self->dwarfDbgTypeInfo->printAttrTypeInfo(self, idx);
checkErrOK(result);
      self->dwarfDbgTypeInfo->typeLevel--;
      return result;
    }
  }
  if (dwAttrTypeInfos->maxDwAttrTypeInfo <= dwAttrTypeInfos->numDwAttrTypeInfo) {
    dwAttrTypeInfos->maxDwAttrTypeInfo += 5;
    if (dwAttrTypeInfos->dwAttrTypeInfos == NULL) {
      dwAttrTypeInfos->dwAttrTypeInfos = (dwAttrTypeInfo_t *)ckalloc(sizeof(dwAttrTypeInfo_t) * dwAttrTypeInfos->maxDwAttrTypeInfo);
      if (dwAttrTypeInfos->dwAttrTypeInfos == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      dwAttrTypeInfos->dwAttrTypeInfos = (dwAttrTypeInfo_t *)ckrealloc((char *)dwAttrTypeInfos->dwAttrTypeInfos, sizeof(dwAttrTypeInfo_t) * dwAttrTypeInfos->maxDwAttrTypeInfo);
      if (dwAttrTypeInfos->dwAttrTypeInfos == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
  dwAttrTypeInfo = &dwAttrTypeInfos->dwAttrTypeInfos[dwAttrTypeInfos->numDwAttrTypeInfo];
  memset(dwAttrTypeInfo, 0, sizeof(dwAttrTypeInfo_t));
  dwAttrTypeInfo->tag = dwAttrTypeInfoIn->tag;
  for (idx = 0; idx < dwAttrTypeInfoIn->numDwAttrType; idx++) {
    dwAttrTypeIn = &dwAttrTypeInfoIn->dwAttrTypes[idx];
    result = self->dwarfDbgTypeInfo->addAttrType(self, dwAttrTypeInfo, dwAttrTypeIn->dwType, dwAttrTypeIn->value, &attrTypeIdx);
    checkErrOK(result);
  }
  *typeIdx = dwAttrTypeInfos->numDwAttrTypeInfo;
  dwAttrTypeInfos->numDwAttrTypeInfo++;
result = self->dwarfDbgTypeInfo->printAttrTypeInfo(self, dwAttrTypeInfos->numDwAttrTypeInfo - 1);
checkErrOK(result);
  self->dwarfDbgTypeInfo->typeLevel--;
  return result;
}

// =================================== printAttrTypeInfo =========================== 

static uint8_t printAttrTypeInfo(dwarfDbgPtr_t self, int tagIdx) {
  uint8_t result;
  dwAttrTypeInfos_t *dwAttrTypeInfos;
  dwAttrTypeInfo_t *dwAttrTypeInfo;
  dwAttrType_t *dwAttrType;
  int attrIdx = 0;
  const char *tagName;
  const char *typeName;
  char *name;
  char *dirName;
  pathNameInfo_t *pathNameInfo;
  char pathName[255];

  result = DWARF_DBG_ERR_OK;
  dwAttrTypeInfos = &self->dwarfDbgTypeInfo->dwAttrTypeInfos;
  if ((tagIdx < 0) || (tagIdx >= dwAttrTypeInfos->numDwAttrTypeInfo)) {
printf("ERROR bad numDwAttrTypeInfo tagIdx: %d %d\n", tagIdx, dwAttrTypeInfos->numDwAttrTypeInfo);
    return DWARF_DBG_ERR_BAD_DW_ATTR_TYPE_INFOS_IDX;
  }
  dwAttrTypeInfo = &dwAttrTypeInfos->dwAttrTypeInfos[tagIdx];
  result = self->dwarfDbgStringInfo->getDW_TAG_string(self, dwAttrTypeInfo->tag, &tagName);
  checkErrOK(result);
  fprintf(typeFd, "\n>>TAG: %s 0x%04x idx: %d\n", tagName, dwAttrTypeInfo->tag, tagIdx);
  for (attrIdx = 0; attrIdx < dwAttrTypeInfo->numDwAttrType; attrIdx++) {
    dwAttrType = &dwAttrTypeInfo->dwAttrTypes[attrIdx];
    result = self->dwarfDbgStringInfo->getDW_AT_string(self, dwAttrType->dwType, &typeName);
    checkErrOK(result);
    name = "";
    pathName[0] = '\0';
    switch (dwAttrType->dwType) {
    case DW_AT_name:
      name = self->dwarfDbgTypeInfo->typeStrs[dwAttrType->value];
      break;
    case DW_AT_decl_file:
      pathNameInfo = &self->dwarfDbgFileInfo->pathNamesInfo.pathNames[dwAttrType->value];
      dirName = self->dwarfDbgFileInfo->dirNamesInfo.dirNames[pathNameInfo->dirNameIdx];
      sprintf(pathName,"%s/%s", dirName, pathNameInfo->fileName);
      break;
    }
    fprintf(typeFd, "    %s: %d %s%s\n", typeName, dwAttrType->value, name, pathName);
    fflush(typeFd);
  }
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
  int dwAttrTypeInfoIdx = 0;
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
//printf("getTypeRefIdx: tag: %s 0x%04x typeIdx: %d offset: 0x%08x refOffset: 0x%08x dieInfo: %p\n", tagName, dieInfo->tag, typeIdx, dieInfo->offset, dieAttr->refOffset, dieInfo);

        if ((dieInfo->offset == dieAttr->refOffset) && (dieInfo->tagRefIdx == -1)) {
printf("Warning missing tagRefIdx for: %s\n", tagName);
          result =self->dwarfDbgTypeInfo->handleType(self, dieInfo, &dwAttrTypeInfoIdx);
          checkErrOK(result);
        }
        if (dieInfo->offset == dieAttr->refOffset) {
          result = self->dwarfDbgTypeInfo->printAttrTypeInfo(self, dieInfo->tagRefIdx);
          checkErrOK(result);
        }
        switch (dieInfo->tag) {
        case DW_TAG_array_type:
          if (dieInfo->offset == dieAttr->refOffset) {
//printf("found type: %s arrayTypeIdx: %d dieInfo->offset: 0x%08x tagRefIdx: %d\n", tagName, typeIdx, dieInfo->offset, dieInfo->tagRefIdx);
            *dwTypeIdx = dieInfo->tagRefIdx;
            return result;
          }
          break;
        case DW_TAG_base_type:
          if (dieInfo->offset == dieAttr->refOffset) {
//printf("found type: %s baseTypeIdx: %d dieInfo->offset: 0x%08x tagRefIdx: %d\n", tagName, typeIdx, dieInfo->offset, dieInfo->tagRefIdx);
            *dwTypeIdx = dieInfo->tagRefIdx;
            return result;
          }
          break;
        case DW_TAG_const_type:
          if (dieInfo->offset == dieAttr->refOffset) {
//printf("found type: %s constTypeIdx: %d dieInfo->offset: 0x%08x tagRefIdx: %d\n", tagName, typeIdx, dieInfo->offset, dieInfo->tagRefIdx);
            *dwTypeIdx = dieInfo->tagRefIdx;
            return result;
          }
          break;
        case DW_TAG_enumeration_type:
          if (dieInfo->offset == dieAttr->refOffset) {
//printf("found type: %s enumerationTypeIdx: %d dieInfo->offset: 0x%08x tagRefIdx: %d\n", tagName, typeIdx, dieInfo->offset, dieInfo->tagRefIdx);
            *dwTypeIdx = dieInfo->tagRefIdx;
            return result;
          }
          break;
        case DW_TAG_pointer_type:
          if (dieInfo->offset == dieAttr->refOffset) {
//printf("found type: %s pointerTypeIdx: %d dieInfo->offset: 0x%08x tagRefIdx: %d\n", tagName, typeIdx, dieInfo->offset, dieInfo->tagRefIdx);
            *dwTypeIdx = dieInfo->tagRefIdx;
            return result;
          }
          break;
        case DW_TAG_structure_type:
          if (dieInfo->offset == dieAttr->refOffset) {
//printf("found type: %s structureTypeIdx: %d dieInfo->offset: 0x%08x tagRefIdx: %d\n", tagName, typeIdx, dieInfo->offset, dieInfo->tagRefIdx);
            *dwTypeIdx = dieInfo->tagRefIdx;
            return result;
          }
          break;
        case DW_TAG_subroutine_type:
          if (dieInfo->offset == dieAttr->refOffset) {
//printf("found type: %s subroutineTypeIdx: %d dieInfo->offset: 0x%08x tagRefIdx: %d\n", tagName, typeIdx, dieInfo->offset, dieInfo->tagRefIdx);
            *dwTypeIdx = dieInfo->tagRefIdx;
            return result;
          }
          break;
        case DW_TAG_typedef:
          if (dieInfo->offset == dieAttr->refOffset) {
//printf("found type: %s typedefIdx: %d dieInfo->offset: 0x%08x tagRefIdx: %d\n", tagName, typeIdx, dieInfo->offset, dieInfo->tagRefIdx);
            *dwTypeIdx = dieInfo->tagRefIdx;
            return result;
          }
          break;
        case DW_TAG_union_type: 
          if (dieInfo->offset == dieAttr->refOffset) {
//printf("found type: %s unionTypeIdx: %d dieInfo->offset: 0x%08x tagRefIdx: %d\n", tagName, typeIdx, dieInfo->offset, dieInfo->tagRefIdx);
            *dwTypeIdx = dieInfo->tagRefIdx;
            return result;
          }
          break;
        case DW_TAG_volatile_type:
          if (dieInfo->offset == dieAttr->refOffset) {
//printf("found type: %s volatileTypeIdx: %d dieInfo->offset: 0x%08x tagRefIdx: %d\n", tagName, typeIdx, dieInfo->offset, dieInfo->tagRefIdx);
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

static uint8_t handleType(dwarfDbgPtr_t self, dieInfo_t *dieInfo, int *dwAttrTypeInfoIdx) {
  uint8_t result = 0;
  int entryIdx = 0;
  int typeIdx = 0;
  int dwTypeIdx = 0;
  int attrIdx = 0;
  int attrTypeIdx = 0;
  int typeNameIdx = 0;
  int typeRefIdx = 0;
  int tagRefIdx = 0;
  dieInfo_t *dieInfo2 = NULL;
  dieAttr_t *dieAttr = NULL;
  dwAttrTypeInfos_t *dwAttrTypeInfos;
  dwAttrTypeInfo_t dwAttrTypeInfo;
  int found = 0;
  const char *tagName = NULL;
  const char *attrName = NULL;
  const char *atName = NULL;
int offset = -1;

  dwAttrTypeInfo.tag = dieInfo->tag;
  dwAttrTypeInfo.numDwAttrType = 0;
  dwAttrTypeInfo.maxDwAttrType = 0;
  dwAttrTypeInfo.dwAttrTypes = NULL;
  dwAttrTypeInfos = &self->dwarfDbgTypeInfo->dwAttrTypeInfos;
  result = DWARF_DBG_ERR_OK;
  found = 0;
//currTypeFlags = ATTR_arrayTypeFlags;
self->dwarfDbgStringInfo->getDW_TAG_string(self, dieInfo->tag, &tagName);
printf("tagName: %s numAttr: %d\n", tagName, dieInfo->numAttr);
  for(attrIdx = 0; attrIdx < dieInfo->numAttr; attrIdx++) {
    dieAttr = &dieInfo->dieAttrs[attrIdx];
self->dwarfDbgStringInfo->getDW_AT_string(self, dieAttr->attr, &attrName);
printf("        attrName: %s value: %d 0x%08x refOffset: 0x%08x\n", attrName, dieAttr->uval, dieAttr->uval, dieAttr->refOffset);
    switch (dieAttr->attr) {
    case DW_AT_artificial:
      result = self->dwarfDbgTypeInfo->addAttrType(self, &dwAttrTypeInfo, dieAttr->attr, dieAttr->uval, &attrTypeIdx);
      checkErrOK(result);
      found++;
      break;
    case DW_AT_abstract_origin:
      result = self->dwarfDbgTypeInfo->addAttrType(self, &dwAttrTypeInfo, dieAttr->attr, dieAttr->uval, &attrTypeIdx);
      checkErrOK(result);
      found++;
      break;
    case DW_AT_bit_offset:
      result = self->dwarfDbgTypeInfo->addAttrType(self, &dwAttrTypeInfo, dieAttr->attr, dieAttr->uval, &attrTypeIdx);
      checkErrOK(result);
      found++;
      break;
    case DW_AT_bit_size:
      result = self->dwarfDbgTypeInfo->addAttrType(self, &dwAttrTypeInfo, dieAttr->attr, dieAttr->uval, &attrTypeIdx);
      checkErrOK(result);
      found++;
      break;
    case DW_AT_byte_size:
      result = self->dwarfDbgTypeInfo->addAttrType(self, &dwAttrTypeInfo, dieAttr->attr, dieAttr->byteSize, &attrTypeIdx);
      checkErrOK(result);
      found++;
      break;
    case DW_AT_call_file:
      result = self->dwarfDbgTypeInfo->addAttrType(self, &dwAttrTypeInfo, dieAttr->attr, dieAttr->uval, &attrTypeIdx);
      checkErrOK(result);
      found++;
      break;
    case DW_AT_call_line:
      result = self->dwarfDbgTypeInfo->addAttrType(self, &dwAttrTypeInfo, dieAttr->attr, dieAttr->uval, &attrTypeIdx);
      checkErrOK(result);
      found++;
      break;
    case DW_AT_comp_dir:
      result = self->dwarfDbgTypeInfo->addAttrType(self, &dwAttrTypeInfo, dieAttr->attr, dieAttr->uval, &attrTypeIdx);
      checkErrOK(result);
      found++;
      break;
    case DW_AT_const_value:
      result = self->dwarfDbgTypeInfo->addAttrType(self, &dwAttrTypeInfo, dieAttr->attr, dieAttr->uval, &attrTypeIdx);
      checkErrOK(result);
      found++;
      break;
    case DW_AT_data_member_location:
      result = self->dwarfDbgTypeInfo->addAttrType(self, &dwAttrTypeInfo, dieAttr->attr, dieAttr->uval, &attrTypeIdx);
      checkErrOK(result);
      found++;
      break;
    case DW_AT_declaration:
      result = self->dwarfDbgTypeInfo->addAttrType(self, &dwAttrTypeInfo, dieAttr->attr, dieAttr->uval, &attrTypeIdx);
      checkErrOK(result);
      found++;
      break;
    case DW_AT_decl_file:
      result = self->dwarfDbgTypeInfo->addAttrType(self, &dwAttrTypeInfo, dieAttr->attr, dieAttr->sourceFileIdx, &attrTypeIdx);
      checkErrOK(result);
      found++;
      break;
    case DW_AT_decl_line:
      result = self->dwarfDbgTypeInfo->addAttrType(self, &dwAttrTypeInfo, dieAttr->attr, dieAttr->sourceLineNo, &attrTypeIdx);
      checkErrOK(result);
      found++;
      break;
    case DW_AT_encoding:
      result = self->dwarfDbgTypeInfo->addAttrType(self, &dwAttrTypeInfo, dieAttr->attr, dieAttr->uval, &attrTypeIdx);
      checkErrOK(result);
      found++;
      break;
    case DW_AT_entry_pc:
      result = self->dwarfDbgTypeInfo->addAttrType(self, &dwAttrTypeInfo, dieAttr->attr, dieAttr->uval, &attrTypeIdx);
      checkErrOK(result);
      found++;
      break;
    case DW_AT_external:
      result = self->dwarfDbgTypeInfo->addAttrType(self, &dwAttrTypeInfo, dieAttr->attr, dieAttr->uval, &attrTypeIdx);
      checkErrOK(result);
      found++;
      break;
    case DW_AT_frame_base:
      result = self->dwarfDbgTypeInfo->addAttrType(self, &dwAttrTypeInfo, dieAttr->attr, dieAttr->uval, &attrTypeIdx);
      checkErrOK(result);
      found++;
      break;
    case DW_AT_GNU_all_call_sites:
      result = self->dwarfDbgTypeInfo->addAttrType(self, &dwAttrTypeInfo, dieAttr->attr, dieAttr->uval, &attrTypeIdx);
      checkErrOK(result);
      found++;
      break;
    case DW_AT_GNU_all_tail_call_sites:
      result = self->dwarfDbgTypeInfo->addAttrType(self, &dwAttrTypeInfo, dieAttr->attr, dieAttr->uval, &attrTypeIdx);
      checkErrOK(result);
      found++;
      break;
    case DW_AT_GNU_call_site_target:
      result = self->dwarfDbgTypeInfo->addAttrType(self, &dwAttrTypeInfo, dieAttr->attr, dieAttr->uval, &attrTypeIdx);
      checkErrOK(result);
      found++;
      break;
    case DW_AT_GNU_call_site_value:
      result = self->dwarfDbgTypeInfo->addAttrType(self, &dwAttrTypeInfo, dieAttr->attr, dieAttr->uval, &attrTypeIdx);
      checkErrOK(result);
      found++;
      break;
    case DW_AT_high_pc:
      result = self->dwarfDbgTypeInfo->addAttrType(self, &dwAttrTypeInfo, dieAttr->attr, dieAttr->uval, &attrTypeIdx);
      checkErrOK(result);
      found++;
      break;
    case DW_AT_inline:
      result = self->dwarfDbgTypeInfo->addAttrType(self, &dwAttrTypeInfo, dieAttr->attr, dieAttr->uval, &attrTypeIdx);
      checkErrOK(result);
      found++;
      break;
    case DW_AT_language:
      result = self->dwarfDbgTypeInfo->addAttrType(self, &dwAttrTypeInfo, dieAttr->attr, dieAttr->uval, &attrTypeIdx);
      checkErrOK(result);
      found++;
      break;
    case DW_AT_linkage_name:
      result = self->dwarfDbgTypeInfo->addAttrType(self, &dwAttrTypeInfo, dieAttr->attr, dieAttr->uval, &attrTypeIdx);
      checkErrOK(result);
      found++;
      break;
    case DW_AT_location:
      result = self->dwarfDbgTypeInfo->addAttrType(self, &dwAttrTypeInfo, dieAttr->attr, dieAttr->uval, &attrTypeIdx);
      checkErrOK(result);
      found++;
      break;
    case DW_AT_low_pc:
      result = self->dwarfDbgTypeInfo->addAttrType(self, &dwAttrTypeInfo, dieAttr->attr, dieAttr->uval, &attrTypeIdx);
      checkErrOK(result);
      found++;
      break;
    case DW_AT_name:
      if ((dieAttr->attrStrIdx < 0) || (dieAttr->attrStrIdx >= self->dwarfDbgCompileUnitInfo->numAttrStr)) {
        return DWARF_DBG_ERR_BAD_ATTR_STR_IDX;
      }
      atName = self->dwarfDbgCompileUnitInfo->attrStrs[dieAttr->attrStrIdx];
      result = self->dwarfDbgTypeInfo->addTypeStr(self, atName, &typeNameIdx);
      checkErrOK(result);
if (dieInfo->tag == DW_TAG_typedef) {
printf("typedef: name: %s\n", atName);
}
      result = self->dwarfDbgTypeInfo->addAttrType(self, &dwAttrTypeInfo, dieAttr->attr, typeNameIdx, &attrTypeIdx);
      checkErrOK(result);
      found++;
      break;
    case DW_AT_producer:
      result = self->dwarfDbgTypeInfo->addAttrType(self, &dwAttrTypeInfo, dieAttr->attr, dieAttr->uval, &attrTypeIdx);
      checkErrOK(result);
      found++;
      break;
    case DW_AT_prototyped:
      result = self->dwarfDbgTypeInfo->addAttrType(self, &dwAttrTypeInfo, dieAttr->attr, dieAttr->uval, &attrTypeIdx);
      checkErrOK(result);
      found++;
      break;
    case DW_AT_ranges:
      result = self->dwarfDbgTypeInfo->addAttrType(self, &dwAttrTypeInfo, dieAttr->attr, dieAttr->uval, &attrTypeIdx);
      checkErrOK(result);
      found++;
      break;
    case DW_AT_sibling:
      result = self->dwarfDbgTypeInfo->addAttrType(self, &dwAttrTypeInfo, dieAttr->attr, dieAttr->uval, &attrTypeIdx);
      checkErrOK(result);
      found++;
      break;
    case DW_AT_stmt_list:
      result = self->dwarfDbgTypeInfo->addAttrType(self, &dwAttrTypeInfo, dieAttr->attr, dieAttr->uval, &attrTypeIdx);
      checkErrOK(result);
      found++;
      break;
    case DW_AT_type:
//if (dieInfo->tag == DW_TAG_typedef) {
offset = dieAttr->refOffset;
//printf("handleType AT_type: refOffset: 0x%08x\n", dieAttr->refOffset);
//}
      result = getTypeRefIdx(self, dieAttr, &typeRefIdx);
      checkErrOK(result);
//printf("handleType AT_type: got typeRefIdx: %d\n", typeRefIdx);
      result = self->dwarfDbgTypeInfo->addAttrType(self, &dwAttrTypeInfo, dieAttr->attr, typeRefIdx, &attrTypeIdx);
      checkErrOK(result);
//printf("handleType after addAttrType: got attrTypeIdx: %d attridx: %d numAttr: %d\n", attrTypeIdx, attrIdx, dieInfo->numAttr);
      found++;
      break;
    case DW_AT_upper_bound:
      result = self->dwarfDbgTypeInfo->addAttrType(self, &dwAttrTypeInfo, dieAttr->attr, dieAttr->uval, &attrTypeIdx);
      checkErrOK(result);
      found++;
      break;
    default:
printf("ERROR: DWARF_DBG_ERR_UNEXPECTED_ATTR_IN_TYPE: 0x%04x\n", dieAttr->attr);
      return DWARF_DBG_ERR_UNEXPECTED_ATTR_IN_TYPE;
    }
  }
  if (found == dieInfo->numAttr) {
//if (dieInfo->tag == DW_TAG_typedef) {
//printf("TYPE: %s attrTypeIdx: %d offset: 0x%08x\n", atName, attrTypeIdx, offset);
//}
    result = self->dwarfDbgTypeInfo->addAttrTypeInfo(self, &dwAttrTypeInfo, dwAttrTypeInfos, dwAttrTypeInfoIdx);
    checkErrOK(result);
    dieInfo->tagRefIdx = *dwAttrTypeInfoIdx;
//printf("after addAttrTypeInfo: tagRefIdx: %d\n", *dwAttrTypeInfoIdx);
  } else {
printf("ERROR type not found found: %d numAttr: %d offset: 0x%08x\n", found, dieInfo->numAttr, dieAttr->refOffset);
    return DWARF_DBG_ERR_TYPE_NOT_FOUND;
  }
  return result;
}

// =================================== addTagTypes =========================== 

static uint8_t addTagTypes(dwarfDbgPtr_t self, int dieAndChildrenIdx, Dwarf_Bool isSibling) {
  uint8_t result = 0;
  int entryIdx = 0;
  int maxEntries = 0;
  int siblingTagInfoIdx = 0;
  int childTagInfoIdx = 0;
  dieAndChildrenInfo_t *dieAndChildrenInfo = NULL;
  compileUnit_t *compileUnit = NULL;
  dieInfo_t *dieInfo = NULL;
  int dwAttrTypeInfoIdx = 0;

  result = DWARF_DBG_ERR_OK;
printf("addTagTypes cu idx: %d dieAndChildrenIdx: %d\n", self->dwarfDbgCompileUnitInfo->currCompileUnitIdx, dieAndChildrenIdx);
  compileUnit = self->dwarfDbgCompileUnitInfo->currCompileUnit;
  dieAndChildrenInfo = &compileUnit->dieAndChildrenInfos[dieAndChildrenIdx];
  if (isSibling) {
    maxEntries = dieAndChildrenInfo->numSiblings;
  } else {
    maxEntries = dieAndChildrenInfo->numChildren;
  }
printf("isSibling: %d maxEntries: %d numDieAndChildren: %d maxDieAndChildren: %d\n", isSibling, maxEntries, compileUnit->numDieAndChildren, compileUnit->maxDieAndChildren);
  for(entryIdx = 0; entryIdx < maxEntries; entryIdx++) {
    if (isSibling) {
      dieInfo = &dieAndChildrenInfo->dieSiblings[entryIdx];
    } else {
      dieInfo = &dieAndChildrenInfo->dieChildren[entryIdx];
    }
    result = self->dwarfDbgTypeInfo->handleType(self, dieInfo, &dwAttrTypeInfoIdx);
    checkErrOK(result);
    if (isSibling) {
      result = self->dwarfDbgDieInfo->addDieSiblingTagInfo(self, dieAndChildrenIdx, dieInfo->tag, dwAttrTypeInfoIdx, &siblingTagInfoIdx);
    } else {
      result = self->dwarfDbgDieInfo->addDieChildTagInfo(self, dieAndChildrenIdx, dieInfo->tag, dwAttrTypeInfoIdx, &childTagInfoIdx);
    }
    checkErrOK(result);
  }
  return result;
}

// =================================== addChildrenTypes =========================== 

static uint8_t addChildrenTypes(dwarfDbgPtr_t self, int dieAndChildrenIdx) {
  uint8_t result = 0;

  result = DWARF_DBG_ERR_OK;
  result = self->dwarfDbgTypeInfo->addTagTypes(self, dieAndChildrenIdx, /* isSibling */ 0);
  checkErrOK(result);
  return result;
}

// =================================== addSiblingsTypes =========================== 

static uint8_t addSiblingsTypes(dwarfDbgPtr_t self, int dieAndChildrenIdx) {
  uint8_t result = 0;

  result = DWARF_DBG_ERR_OK;
  result = self->dwarfDbgTypeInfo->addTagTypes(self, dieAndChildrenIdx, /* isSibling */ 1);
  checkErrOK(result);
  return result;
}

// =================================== dwarfDbgTypeInfoInit =========================== 

int dwarfDbgTypeInfoInit (dwarfDbgPtr_t self) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;
  self->dwarfDbgTypeInfo->addTypeStr = &addTypeStr;
  self->dwarfDbgTypeInfo->addAttrType = &addAttrType;
  self->dwarfDbgTypeInfo->checkDieTypeRefIdx = &checkDieTypeRefIdx;

  self->dwarfDbgTypeInfo->printAttrTypeInfo = &printAttrTypeInfo;
  self->dwarfDbgTypeInfo->addAttrTypeInfo = &addAttrTypeInfo;

  self->dwarfDbgTypeInfo->handleType = &handleType;

  self->dwarfDbgTypeInfo->addTagTypes = &addTagTypes;
  self->dwarfDbgTypeInfo->addChildrenTypes = &addChildrenTypes;
  self->dwarfDbgTypeInfo->addSiblingsTypes = &addSiblingsTypes;

  self->dwarfDbgTypeInfo->typeLevel = 0;
  memset(&self->dwarfDbgTypeInfo->dwAttrTypeInfos, 0, sizeof(dwAttrTypeInfo_t));

//  typeFd = fopen("types.txt", "w");
  typeFd = stdout;
  return result;
}
