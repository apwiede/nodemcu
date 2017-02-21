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

#ifdef NOTDEF
// =================================== addArrayType =========================== 

static uint8_t addArrayType(dwarfDbgPtr_t self, int dwTypeIdx, int siblingIdx, int *arrayTypeIdx) {
  uint8_t result = 0;
  int typeStrIdx = 0;
  int idx = 0;
  dwArrayType_t *dwArrayType = NULL;

  result = DWARF_DBG_ERR_OK;
  for(idx = 0; idx < self->dwarfDbgTypeInfo->numDwArrayType; idx++) {
    dwArrayType = &self->dwarfDbgTypeInfo->dwArrayTypes[idx];
    if (dwArrayType->dwTypeIdx == dwTypeIdx) {
      if (dwArrayType->siblingIdx == siblingIdx) {
printf("addArrayType: found: dwTypeIdx: %d siblingIdx: %d arrayTypeIdx: %d\n", dwTypeIdx, siblingIdx, idx);
        *arrayTypeIdx = idx;
         return result;
      }
    }
  }
  if (self->dwarfDbgTypeInfo->maxDwArrayType <= self->dwarfDbgTypeInfo->numDwArrayType) {
    self->dwarfDbgTypeInfo->maxDwArrayType += 5;
    if (self->dwarfDbgTypeInfo->dwArrayTypes == NULL) {
      self->dwarfDbgTypeInfo->dwArrayTypes = (dwArrayType_t *)ckalloc(sizeof(dwArrayType_t) * self->dwarfDbgTypeInfo->maxDwArrayType);
      if (self->dwarfDbgTypeInfo->dwArrayTypes == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      self->dwarfDbgTypeInfo->dwArrayTypes = (dwArrayType_t *)ckrealloc((char *)self->dwarfDbgTypeInfo->dwArrayTypes, sizeof(dwArrayType_t) * self->dwarfDbgTypeInfo->maxDwArrayType);
      if (self->dwarfDbgTypeInfo->dwArrayTypes == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
  dwArrayType = &self->dwarfDbgTypeInfo->dwArrayTypes[self->dwarfDbgTypeInfo->numDwArrayType];
  dwArrayType->dwTypeIdx = dwTypeIdx;
  dwArrayType->siblingIdx = siblingIdx;
  *arrayTypeIdx = self->dwarfDbgTypeInfo->numDwArrayType;
  self->dwarfDbgTypeInfo->numDwArrayType++;
  return result;
}

// =================================== addBaseType =========================== 

static uint8_t addBaseType(dwarfDbgPtr_t self, const char *name, int byteSize, int encoding, int *baseTypeIdx) {
  uint8_t result = 0;
  int typeStrIdx = 0;
  int idx = 0;
  const char *typeStr;
  dwBaseType_t *dwBaseType = NULL;

  result = DWARF_DBG_ERR_OK;
  result = self->dwarfDbgTypeInfo->addTypeStr(self, name, &typeStrIdx);
  checkErrOK(result);
  for(idx = 0; idx < self->dwarfDbgTypeInfo->numDwBaseType; idx++) {
    dwBaseType = &self->dwarfDbgTypeInfo->dwBaseTypes[idx];
    if (dwBaseType->typeStrIdx == typeStrIdx) {
      if (dwBaseType->byteSize == byteSize) {
        if (dwBaseType->encoding == encoding) {
printf("addBaseType: found: %s byteSize: %d encoding: %d baseTypeIdx: %d\n", name, byteSize, encoding, idx);
          *baseTypeIdx = idx;
          return result;
        }
      }
    }
  }
  if (self->dwarfDbgTypeInfo->maxDwBaseType <= self->dwarfDbgTypeInfo->numDwBaseType) {
    self->dwarfDbgTypeInfo->maxDwBaseType += 5;
    if (self->dwarfDbgTypeInfo->dwBaseTypes == NULL) {
      self->dwarfDbgTypeInfo->dwBaseTypes = (dwBaseType_t *)ckalloc(sizeof(dwBaseType_t) * self->dwarfDbgTypeInfo->maxDwBaseType);
      if (self->dwarfDbgTypeInfo->dwBaseTypes == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      self->dwarfDbgTypeInfo->dwBaseTypes = (dwBaseType_t *)ckrealloc((char *)self->dwarfDbgTypeInfo->dwBaseTypes, sizeof(dwBaseType_t) * self->dwarfDbgTypeInfo->maxDwBaseType);
      if (self->dwarfDbgTypeInfo->dwBaseTypes == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
  dwBaseType = &self->dwarfDbgTypeInfo->dwBaseTypes[self->dwarfDbgTypeInfo->numDwBaseType];
  dwBaseType->byteSize = byteSize;
  dwBaseType->encoding = encoding;
  dwBaseType->typeStrIdx = typeStrIdx;
  *baseTypeIdx = self->dwarfDbgTypeInfo->numDwBaseType;
  self->dwarfDbgTypeInfo->numDwBaseType++;
  return result;
}

// =================================== addConstType =========================== 

static uint8_t addConstType(dwarfDbgPtr_t self, int dwTypeIdx, int *constTypeIdx) {
  uint8_t result = 0;
  int typeStrIdx = 0;
  int idx = 0;
  dwConstType_t *dwConstType = NULL;

  result = DWARF_DBG_ERR_OK;
  for(idx = 0; idx < self->dwarfDbgTypeInfo->numDwConstType; idx++) {
    dwConstType = &self->dwarfDbgTypeInfo->dwConstTypes[idx];
    if (dwConstType->dwTypeIdx == dwTypeIdx) {
printf("addConstType: found: dwTypeIdx: %d constTypeIdx: %d\n", dwTypeIdx, idx);
      *constTypeIdx = idx;
      return result;
    }
  }
  if (self->dwarfDbgTypeInfo->maxDwConstType <= self->dwarfDbgTypeInfo->numDwConstType) {
    self->dwarfDbgTypeInfo->maxDwConstType += 5;
    if (self->dwarfDbgTypeInfo->dwConstTypes == NULL) {
      self->dwarfDbgTypeInfo->dwConstTypes = (dwConstType_t *)ckalloc(sizeof(dwConstType_t) * self->dwarfDbgTypeInfo->maxDwConstType);
      if (self->dwarfDbgTypeInfo->dwConstTypes == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      self->dwarfDbgTypeInfo->dwConstTypes = (dwConstType_t *)ckrealloc((char *)self->dwarfDbgTypeInfo->dwConstTypes, sizeof(dwConstType_t) * self->dwarfDbgTypeInfo->maxDwConstType);
      if (self->dwarfDbgTypeInfo->dwConstTypes == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
  dwConstType = &self->dwarfDbgTypeInfo->dwConstTypes[self->dwarfDbgTypeInfo->numDwConstType];
  dwConstType->dwTypeIdx = dwTypeIdx;
  *constTypeIdx = self->dwarfDbgTypeInfo->numDwConstType;
  self->dwarfDbgTypeInfo->numDwConstType++;
  return result;
}

// =================================== addEnumerationType =========================== 

static uint8_t addEnumerationType(dwarfDbgPtr_t self, int byteSize, int encoding, char *name, int *baseTypeIdx) {
  uint8_t result = 0;
  int typeStrIdx = 0;
  dwBaseType_t *dwBaseType = NULL;

  result = DWARF_DBG_ERR_OK;
  if (self->dwarfDbgTypeInfo->maxDwBaseType <= self->dwarfDbgTypeInfo->numDwBaseType) {
    self->dwarfDbgTypeInfo->maxDwBaseType += 5;
    if (self->dwarfDbgTypeInfo->dwBaseTypes == NULL) {
      self->dwarfDbgTypeInfo->dwBaseTypes = (dwBaseType_t *)ckalloc(sizeof(dwBaseType_t) * self->dwarfDbgTypeInfo->maxDwBaseType);
      if (self->dwarfDbgTypeInfo->dwBaseTypes == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      self->dwarfDbgTypeInfo->dwBaseTypes = (dwBaseType_t *)ckrealloc((char *)self->dwarfDbgTypeInfo->dwBaseTypes, sizeof(dwBaseType_t) * self->dwarfDbgTypeInfo->maxDwBaseType);
      if (self->dwarfDbgTypeInfo->dwBaseTypes == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
  dwBaseType = &self->dwarfDbgTypeInfo->dwBaseTypes[self->dwarfDbgTypeInfo->numDwBaseType];
  dwBaseType->byteSize = byteSize;
  dwBaseType->encoding = encoding;
  result = self->dwarfDbgTypeInfo->addTypeStr(self, name, &typeStrIdx);
  checkErrOK(result);
  dwBaseType->typeStrIdx = typeStrIdx;
  *baseTypeIdx = self->dwarfDbgTypeInfo->numDwBaseType;
  self->dwarfDbgTypeInfo->numDwBaseType++;
  return result;
}

// =================================== addEnumerator =========================== 

static uint8_t addEnumerator(dwarfDbgPtr_t self, int byteSize, int encoding, char *name, int *baseTypeIdx) {
  uint8_t result = 0;
  int typeStrIdx = 0;
  dwBaseType_t *dwBaseType = NULL;

  result = DWARF_DBG_ERR_OK;
  if (self->dwarfDbgTypeInfo->maxDwBaseType <= self->dwarfDbgTypeInfo->numDwBaseType) {
    self->dwarfDbgTypeInfo->maxDwBaseType += 5;
    if (self->dwarfDbgTypeInfo->dwBaseTypes == NULL) {
      self->dwarfDbgTypeInfo->dwBaseTypes = (dwBaseType_t *)ckalloc(sizeof(dwBaseType_t) * self->dwarfDbgTypeInfo->maxDwBaseType);
      if (self->dwarfDbgTypeInfo->dwBaseTypes == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      self->dwarfDbgTypeInfo->dwBaseTypes = (dwBaseType_t *)ckrealloc((char *)self->dwarfDbgTypeInfo->dwBaseTypes, sizeof(dwBaseType_t) * self->dwarfDbgTypeInfo->maxDwBaseType);
      if (self->dwarfDbgTypeInfo->dwBaseTypes == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
  dwBaseType = &self->dwarfDbgTypeInfo->dwBaseTypes[self->dwarfDbgTypeInfo->numDwBaseType];
  dwBaseType->byteSize = byteSize;
  dwBaseType->encoding = encoding;
  result = self->dwarfDbgTypeInfo->addTypeStr(self, name, &typeStrIdx);
  checkErrOK(result);
  dwBaseType->typeStrIdx = typeStrIdx;
  *baseTypeIdx = self->dwarfDbgTypeInfo->numDwBaseType;
  self->dwarfDbgTypeInfo->numDwBaseType++;
  return result;
}

// =================================== addMember =========================== 

static uint8_t addMember(dwarfDbgPtr_t self, int byteSize, int encoding, char *name, int *baseTypeIdx) {
  uint8_t result = 0;
  int typeStrIdx = 0;
  dwBaseType_t *dwBaseType = NULL;

  result = DWARF_DBG_ERR_OK;
  if (self->dwarfDbgTypeInfo->maxDwBaseType <= self->dwarfDbgTypeInfo->numDwBaseType) {
    self->dwarfDbgTypeInfo->maxDwBaseType += 5;
    if (self->dwarfDbgTypeInfo->dwBaseTypes == NULL) {
      self->dwarfDbgTypeInfo->dwBaseTypes = (dwBaseType_t *)ckalloc(sizeof(dwBaseType_t) * self->dwarfDbgTypeInfo->maxDwBaseType);
      if (self->dwarfDbgTypeInfo->dwBaseTypes == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      self->dwarfDbgTypeInfo->dwBaseTypes = (dwBaseType_t *)ckrealloc((char *)self->dwarfDbgTypeInfo->dwBaseTypes, sizeof(dwBaseType_t) * self->dwarfDbgTypeInfo->maxDwBaseType);
      if (self->dwarfDbgTypeInfo->dwBaseTypes == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
  dwBaseType = &self->dwarfDbgTypeInfo->dwBaseTypes[self->dwarfDbgTypeInfo->numDwBaseType];
  dwBaseType->byteSize = byteSize;
  dwBaseType->encoding = encoding;
  result = self->dwarfDbgTypeInfo->addTypeStr(self, name, &typeStrIdx);
  checkErrOK(result);
  dwBaseType->typeStrIdx = typeStrIdx;
  *baseTypeIdx = self->dwarfDbgTypeInfo->numDwBaseType;
  self->dwarfDbgTypeInfo->numDwBaseType++;
  return result;
}

// =================================== addPointerType =========================== 

static uint8_t addPointerType(dwarfDbgPtr_t self, int byteSize, int dwTypeIdx, int *pointerTypeIdx) {
  uint8_t result = 0;
  int typeStrIdx = 0;
  int idx = 0;
  dwPointerType_t *dwPointerType = NULL;

  result = DWARF_DBG_ERR_OK;
  for(idx = 0; idx < self->dwarfDbgTypeInfo->numDwPointerType; idx++) {
    dwPointerType = &self->dwarfDbgTypeInfo->dwPointerTypes[idx];
    if (dwPointerType->byteSize == byteSize) {
      if (dwPointerType->dwTypeIdx == dwTypeIdx) {
printf("addPointerType: found: byteSize: %d dwTypeIdx: %d pointerTypeIdx: %d\n", byteSize, dwTypeIdx, idx);
        *pointerTypeIdx = idx;
        return result;
      }
    }
  }
  if (self->dwarfDbgTypeInfo->maxDwPointerType <= self->dwarfDbgTypeInfo->numDwPointerType) {
    self->dwarfDbgTypeInfo->maxDwPointerType += 5;
    if (self->dwarfDbgTypeInfo->dwPointerTypes == NULL) {
      self->dwarfDbgTypeInfo->dwPointerTypes = (dwPointerType_t *)ckalloc(sizeof(dwPointerType_t) * self->dwarfDbgTypeInfo->maxDwPointerType);
      if (self->dwarfDbgTypeInfo->dwPointerTypes == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      self->dwarfDbgTypeInfo->dwPointerTypes = (dwPointerType_t *)ckrealloc((char *)self->dwarfDbgTypeInfo->dwPointerTypes, sizeof(dwPointerType_t) * self->dwarfDbgTypeInfo->maxDwPointerType);
      if (self->dwarfDbgTypeInfo->dwPointerTypes == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
printf("addPointerType: new: byteSize: %d dwTypeIdx: %d pointerTypeIdx: %d\n", byteSize, dwTypeIdx, self->dwarfDbgTypeInfo->numDwPointerType);
  dwPointerType = &self->dwarfDbgTypeInfo->dwPointerTypes[self->dwarfDbgTypeInfo->numDwPointerType];
  dwPointerType->byteSize = byteSize;
  dwPointerType->dwTypeIdx = dwTypeIdx;
  *pointerTypeIdx = self->dwarfDbgTypeInfo->numDwPointerType;
  self->dwarfDbgTypeInfo->numDwPointerType++;
  return result;
}

// =================================== addStructureType =========================== 

static uint8_t addStructureType(dwarfDbgPtr_t self, const char *name, int byteSize, int pathNameIdx, int lineNo, int declaration, int *structureTypeIdx) {
  uint8_t result = 0;
  int typeStrIdx = 0;
  int idx = 0;
  dwStructureType_t *dwStructureType = NULL;

  result = DWARF_DBG_ERR_OK;
  if (name == NULL) {
    typeStrIdx = -1;
  } else {
    result = self->dwarfDbgTypeInfo->addTypeStr(self, name, &typeStrIdx);
    checkErrOK(result);
  }
  for(idx = 0; idx < self->dwarfDbgTypeInfo->numDwStructureType; idx++) {
    dwStructureType = &self->dwarfDbgTypeInfo->dwStructureTypes[idx];
    if (dwStructureType->typeStrIdx == typeStrIdx) {
      if (dwStructureType->byteSize == byteSize) {
        if (dwStructureType->pathNameIdx == pathNameIdx) {
          if (dwStructureType->lineNo == lineNo) {
            if (dwStructureType->declaration == declaration) {
printf("addStructureType: found: %s byteSize: %d pathNameIdx: %d lineNo: %d declarationx: %d typeDefIdx: %d\n", name, byteSize, pathNameIdx, lineNo, declaration, idx);
              *structureTypeIdx = idx;
              return result;
            }
          }
        }
      }
    }
  }
  if (self->dwarfDbgTypeInfo->maxDwStructureType <= self->dwarfDbgTypeInfo->numDwStructureType) {
    self->dwarfDbgTypeInfo->maxDwStructureType += 5;
    if (self->dwarfDbgTypeInfo->dwStructureTypes == NULL) {
      self->dwarfDbgTypeInfo->dwStructureTypes = (dwStructureType_t *)ckalloc(sizeof(dwStructureType_t) * self->dwarfDbgTypeInfo->maxDwStructureType);
      if (self->dwarfDbgTypeInfo->dwStructureTypes == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      self->dwarfDbgTypeInfo->dwStructureTypes = (dwStructureType_t *)ckrealloc((char *)self->dwarfDbgTypeInfo->dwStructureTypes, sizeof(dwStructureType_t) * self->dwarfDbgTypeInfo->maxDwStructureType);
      if (self->dwarfDbgTypeInfo->dwStructureTypes == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
printf("addStructureType: new %d %s byteSize: %d pathNameIdx: %d lineNo: %d declaration: %d structureTypeIdx: %d\n", self->dwarfDbgTypeInfo->numDwStructureType, name, byteSize, pathNameIdx, lineNo, declaration, self->dwarfDbgTypeInfo->numDwStructureType);
  dwStructureType = &self->dwarfDbgTypeInfo->dwStructureTypes[self->dwarfDbgTypeInfo->numDwStructureType];
  dwStructureType->byteSize = byteSize;
  dwStructureType->pathNameIdx = pathNameIdx;
  dwStructureType->lineNo = lineNo;
  dwStructureType->declaration = declaration;
  dwStructureType->typeStrIdx = typeStrIdx;
  *structureTypeIdx = self->dwarfDbgTypeInfo->numDwStructureType;
  self->dwarfDbgTypeInfo->numDwStructureType++;
  return result;
}

// =================================== addSubroutineType =========================== 

static uint8_t addSubroutineType(dwarfDbgPtr_t self, int prototyped, int byteSize, int dwTypeIdx, int siblingIdx, int *subroutineTypeIdx) {
  uint8_t result = 0;
  int typeStrIdx = 0;
  int idx = 0;
  dwSubroutineType_t *dwSubroutineType = NULL;

  result = DWARF_DBG_ERR_OK;
  for(idx = 0; idx < self->dwarfDbgTypeInfo->numDwSubroutineType; idx++) {
    dwSubroutineType = &self->dwarfDbgTypeInfo->dwSubroutineTypes[idx];
    if (dwSubroutineType->prototyped == prototyped) {
      if (dwSubroutineType->byteSize == byteSize) {
        if (dwSubroutineType->dwTypeIdx == dwTypeIdx) {
          if (dwSubroutineType->siblingIdx == siblingIdx) {
printf("addSubroutineType: found: prototyped: %d byteSize: %d dwTypeIdx: %d siblingIdx: %d subroutineTypeIdx: %d\n", prototyped, byteSize, dwTypeIdx, siblingIdx, idx);
            *subroutineTypeIdx = idx;
            return result;
          }
        }
      }
    }
  }
  if (self->dwarfDbgTypeInfo->maxDwSubroutineType <= self->dwarfDbgTypeInfo->numDwSubroutineType) {
    self->dwarfDbgTypeInfo->maxDwSubroutineType += 5;
    if (self->dwarfDbgTypeInfo->dwSubroutineTypes == NULL) {
      self->dwarfDbgTypeInfo->dwSubroutineTypes = (dwSubroutineType_t *)ckalloc(sizeof(dwSubroutineType_t) * self->dwarfDbgTypeInfo->maxDwSubroutineType);
      if (self->dwarfDbgTypeInfo->dwSubroutineTypes == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      self->dwarfDbgTypeInfo->dwSubroutineTypes = (dwSubroutineType_t *)ckrealloc((char *)self->dwarfDbgTypeInfo->dwSubroutineTypes, sizeof(dwSubroutineType_t) * self->dwarfDbgTypeInfo->maxDwSubroutineType);
      if (self->dwarfDbgTypeInfo->dwSubroutineTypes == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
  dwSubroutineType = &self->dwarfDbgTypeInfo->dwSubroutineTypes[self->dwarfDbgTypeInfo->numDwSubroutineType];
  dwSubroutineType->byteSize = byteSize;
  dwSubroutineType->dwTypeIdx = dwTypeIdx;
  dwSubroutineType->siblingIdx = siblingIdx;
  *subroutineTypeIdx = self->dwarfDbgTypeInfo->numDwSubroutineType;
  self->dwarfDbgTypeInfo->numDwSubroutineType++;
  return result;
}

// =================================== addTypeDef =========================== 

static uint8_t addTypeDef(dwarfDbgPtr_t self, const char *name, int pathNameIdx, int lineNo, int dwTypeIdx, int *typeDefIdx) {
  uint8_t result = 0;
  int typeStrIdx = 0;
  int idx = 0;
  dwTypeDef_t *dwTypeDef = NULL;

  result = DWARF_DBG_ERR_OK;
  result = self->dwarfDbgTypeInfo->addTypeStr(self, name, &typeStrIdx);
  checkErrOK(result);
  for(idx = 0; idx < self->dwarfDbgTypeInfo->numDwTypeDef; idx++) {
    dwTypeDef = &self->dwarfDbgTypeInfo->dwTypeDefs[idx];
    if (dwTypeDef->typeStrIdx == typeStrIdx) {
      if (dwTypeDef->pathNameIdx == pathNameIdx) {
        if (dwTypeDef->lineNo == lineNo) {
          if (dwTypeDef->dwTypeIdx == dwTypeIdx) {
printf("addTypeDef: found: %s pathNameIdx: %d lineNo: %d dwTypeIdx: %d typeDefIdx: %d\n", name, pathNameIdx, lineNo, dwTypeIdx, idx);
            *typeDefIdx = idx;
            return result;
          }
        }
      }
    }
  }
  if (self->dwarfDbgTypeInfo->maxDwTypeDef <= self->dwarfDbgTypeInfo->numDwTypeDef) {
    self->dwarfDbgTypeInfo->maxDwTypeDef += 5;
    if (self->dwarfDbgTypeInfo->dwTypeDefs == NULL) {
      self->dwarfDbgTypeInfo->dwTypeDefs = (dwTypeDef_t *)ckalloc(sizeof(dwTypeDef_t) * self->dwarfDbgTypeInfo->maxDwTypeDef);
      if (self->dwarfDbgTypeInfo->dwTypeDefs == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      self->dwarfDbgTypeInfo->dwTypeDefs = (dwTypeDef_t *)ckrealloc((char *)self->dwarfDbgTypeInfo->dwTypeDefs, sizeof(dwTypeDef_t) * self->dwarfDbgTypeInfo->maxDwTypeDef);
      if (self->dwarfDbgTypeInfo->dwTypeDefs == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
  dwTypeDef = &self->dwarfDbgTypeInfo->dwTypeDefs[self->dwarfDbgTypeInfo->numDwTypeDef];
  dwTypeDef->pathNameIdx = pathNameIdx;
  dwTypeDef->lineNo = lineNo;
  dwTypeDef->dwTypeIdx = dwTypeIdx;
  *typeDefIdx = self->dwarfDbgTypeInfo->numDwTypeDef;
  self->dwarfDbgTypeInfo->numDwTypeDef++;
  return result;
}

// =================================== addUnionType =========================== 

static uint8_t addUnionType(dwarfDbgPtr_t self, int byteSize, int encoding, char *name, int *baseTypeIdx) {
  uint8_t result = 0;
  int typeStrIdx = 0;
  dwBaseType_t *dwBaseType = NULL;

  result = DWARF_DBG_ERR_OK;
  if (self->dwarfDbgTypeInfo->maxDwBaseType <= self->dwarfDbgTypeInfo->numDwBaseType) {
    self->dwarfDbgTypeInfo->maxDwBaseType += 5;
    if (self->dwarfDbgTypeInfo->dwBaseTypes == NULL) {
      self->dwarfDbgTypeInfo->dwBaseTypes = (dwBaseType_t *)ckalloc(sizeof(dwBaseType_t) * self->dwarfDbgTypeInfo->maxDwBaseType);
      if (self->dwarfDbgTypeInfo->dwBaseTypes == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      self->dwarfDbgTypeInfo->dwBaseTypes = (dwBaseType_t *)ckrealloc((char *)self->dwarfDbgTypeInfo->dwBaseTypes, sizeof(dwBaseType_t) * self->dwarfDbgTypeInfo->maxDwBaseType);
      if (self->dwarfDbgTypeInfo->dwBaseTypes == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
  dwBaseType = &self->dwarfDbgTypeInfo->dwBaseTypes[self->dwarfDbgTypeInfo->numDwBaseType];
  dwBaseType->byteSize = byteSize;
  dwBaseType->encoding = encoding;
  result = self->dwarfDbgTypeInfo->addTypeStr(self, name, &typeStrIdx);
  checkErrOK(result);
  dwBaseType->typeStrIdx = typeStrIdx;
  *baseTypeIdx = self->dwarfDbgTypeInfo->numDwBaseType;
  self->dwarfDbgTypeInfo->numDwBaseType++;
  return result;
}

// =================================== addVolatileType =========================== 

static uint8_t addVolatileType(dwarfDbgPtr_t self, int byteSize, int encoding, char *name, int *baseTypeIdx) {
  uint8_t result = 0;
  int typeStrIdx = 0;
  dwBaseType_t *dwBaseType = NULL;

  result = DWARF_DBG_ERR_OK;
  if (self->dwarfDbgTypeInfo->maxDwBaseType <= self->dwarfDbgTypeInfo->numDwBaseType) {
    self->dwarfDbgTypeInfo->maxDwBaseType += 5;
    if (self->dwarfDbgTypeInfo->dwBaseTypes == NULL) {
      self->dwarfDbgTypeInfo->dwBaseTypes = (dwBaseType_t *)ckalloc(sizeof(dwBaseType_t) * self->dwarfDbgTypeInfo->maxDwBaseType);
      if (self->dwarfDbgTypeInfo->dwBaseTypes == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      self->dwarfDbgTypeInfo->dwBaseTypes = (dwBaseType_t *)ckrealloc((char *)self->dwarfDbgTypeInfo->dwBaseTypes, sizeof(dwBaseType_t) * self->dwarfDbgTypeInfo->maxDwBaseType);
      if (self->dwarfDbgTypeInfo->dwBaseTypes == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
  dwBaseType = &self->dwarfDbgTypeInfo->dwBaseTypes[self->dwarfDbgTypeInfo->numDwBaseType];
  dwBaseType->byteSize = byteSize;
  dwBaseType->encoding = encoding;
  result = self->dwarfDbgTypeInfo->addTypeStr(self, name, &typeStrIdx);
  checkErrOK(result);
  dwBaseType->typeStrIdx = typeStrIdx;
  *baseTypeIdx = self->dwarfDbgTypeInfo->numDwBaseType;
  self->dwarfDbgTypeInfo->numDwBaseType++;
  return result;
}
#endif

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

#ifdef NOTDEF
// =================================== handleArrayType =========================== 

static uint8_t handleArrayType(dwarfDbgPtr_t self, dieAndChildrenInfo_t *dieAndChildrenInfo, dieInfo_t *dieInfo, int isSibling) {
  uint8_t result = 0;
  int entryIdx = 0;
  int typeIdx = 0;
  int pointerTypeIdx = 0;
  int maxEntries = 0;
  int attrIdx = 0;
  int prototyped = 0;
  dieInfo_t *dieInfo2 = NULL;
  dieAttr_t *dieAttr = NULL;
  int byteSize;   // DW_AT_byte_size
  int dwTypeIdx;  // DW_AT_type
  int siblingIdx; // DW_AT_sibling
  int arrayTypeIdx = 0;
  int found = 0;
  const char *attrName = NULL;
  const char *tagName = NULL;

  result = DWARF_DBG_ERR_OK;
  if (isSibling) {
    maxEntries = dieAndChildrenInfo->numSiblings;
  } else {
    maxEntries = dieAndChildrenInfo->numChildren;
  }
  prototyped = 0;
  byteSize = -1;
  dwTypeIdx = -1;
  siblingIdx = -1;
  attrName = NULL;
  found = 0;
currTypeFlags = ATTR_arrayTypeFlags;
  for(attrIdx = 0; attrIdx < dieInfo->numAttr; attrIdx++) {
    dieAttr = &dieInfo->dieAttrs[attrIdx];
self->dwarfDbgStringInfo->getDW_AT_string(self, dieAttr->attr, &attrName);
printf("handleArrayType: attrName: %s\n", attrName);
    switch (dieAttr->attr) {
    case DW_AT_sibling:
setTypeFlag(AT_sibling);
      siblingIdx = dieAttr->uval;
      found++;
      break;
    case DW_AT_type:
setTypeFlag(AT_type);
      result = getTypeRefIdx(self, dieAttr, &dwTypeIdx);
      checkErrOK(result);
      found++;
      break;
    default:
printf("ERROR: DWARF_DBG_ERR_UNEXPECTED_ATTR_IN_ARRAY_TYPE: 0x%04x\n", dieAttr->attr);
      return DWARF_DBG_ERR_UNEXPECTED_ATTR_IN_ARRAY_TYPE;
    }
  }
  if (found == dieInfo->numAttr) {
    result = self->dwarfDbgTypeInfo->addArrayType(self, dwTypeIdx, siblingIdx, &arrayTypeIdx);
    checkErrOK(result);
    dieInfo->tagRefIdx = arrayTypeIdx;
    dieInfo->flags  = TAG_REF_ARRAY_TYPE;
  } else {
printf("ERROR arrayType not found found: %d numAttr: %d offset: 0x%08x\n", found, dieInfo->numAttr, dieAttr->refOffset);
    return DWARF_DBG_ERR_ARRAY_TYPE_NOT_FOUND;
  }
  return result;
}

// =================================== handleConstType =========================== 

static uint8_t handleConstType(dwarfDbgPtr_t self, dieAndChildrenInfo_t *dieAndChildrenInfo, dieInfo_t *dieInfo, int isSibling) {
  uint8_t result = 0;
  int entryIdx = 0;
  int typeIdx = 0;
  int constTypeIdx = 0;
  int maxEntries = 0;
  int maxEntries2 = 0;
  int attrIdx = 0;
  dieInfo_t *dieInfo2 = NULL;
  dieAttr_t *dieAttr = NULL;
  int dwTypeIdx;  // DW_AT_type
  int found = 0;
  int isSibling2 = 0;
  char *attrName = NULL;
  const char *tagName = NULL;
  const char *tagName2 = NULL;

  result = DWARF_DBG_ERR_OK;
  if (isSibling) {
    maxEntries = dieAndChildrenInfo->numSiblings;
  } else {
    maxEntries = dieAndChildrenInfo->numChildren;
  }
currTypeFlags = ATTR_constTypeFlags;
  dwTypeIdx = -1;
  attrName = NULL;
  found = 0;
printf("handleConstType tag offset: 0x%08x\n", dieInfo->offset);
  for(attrIdx = 0; attrIdx < dieInfo->numAttr; attrIdx++) {
    dieAttr = &dieInfo->dieAttrs[attrIdx];
    switch (dieAttr->attr) {
    case DW_AT_type:
printf("CONST DW_AT_type: refOffset: 0x%08x maxEntries: %d\n", dieAttr->refOffset, maxEntries);
setTypeFlag(AT_type);
      result = getTypeRefIdx(self, dieAttr, &dwTypeIdx);
      checkErrOK(result);
      found++;
      break;
    default:
printf("ERROR: DWARF_DBG_ERR_UNEXPECTED_ATTR_IN_CONST_TYPE: 0x%04x\n", dieAttr->attr);
      return DWARF_DBG_ERR_UNEXPECTED_ATTR_IN_CONST_TYPE;
    }
    if (found == dieInfo->numAttr) {
//printf("found 2 break\n");
      break;
    }
  }
  if (found == dieInfo->numAttr) {
printf("call addConstType\n");
    result = self->dwarfDbgTypeInfo->addConstType(self, dwTypeIdx, &constTypeIdx);
    checkErrOK(result);
    dieInfo->tagRefIdx = constTypeIdx;
    dieInfo->flags  = TAG_REF_CONST_TYPE;
  } else {
printf("ERROR constType not found found: %d numAttr: %d offset: 0x%08x\n", found, dieInfo->numAttr, dieAttr->refOffset);
    return DWARF_DBG_ERR_CONST_TYPE_NOT_FOUND;
  }
  return result;
}

// =================================== handleEnumerationType =========================== 

static uint8_t handleEnumerationType(dwarfDbgPtr_t self, dieAndChildrenInfo_t *dieAndChildrenInfo, dieInfo_t *dieInfo, int isSibling) {
  uint8_t result = 0;
  int entryIdx = 0;
  int typeIdx = 0;
  int pointerTypeIdx = 0;
  int maxEntries = 0;
  int attrIdx = 0;
  int prototyped = 0;
  dieInfo_t *dieInfo2 = NULL;
  dieAttr_t *dieAttr = NULL;
  int byteSize;    // DW_AT_byte_size
  int pathNameIdx; // DW_AT_decl_file
  int lineNo;      // DW_AT_decl_line
  int dwTypeIdx;   // DW_AT_type
  int siblingIdx;  // DW_AT_sibling
  int arrayTypeIdx = 0;
  int found = 0;
  const char *attrName = NULL;
  const char *tagName = NULL;

  result = DWARF_DBG_ERR_OK;
currTypeFlags = ATTR_enumerationTypeFlags;
  if (isSibling) {
    maxEntries = dieAndChildrenInfo->numSiblings;
  } else {
    maxEntries = dieAndChildrenInfo->numChildren;
  }
  prototyped = 0;
  byteSize = -1;
  dwTypeIdx = -1;
  siblingIdx = -1;
  attrName = NULL;
  found = 0;
  for(attrIdx = 0; attrIdx < dieInfo->numAttr; attrIdx++) {
    dieAttr = &dieInfo->dieAttrs[attrIdx];
self->dwarfDbgStringInfo->getDW_AT_string(self, dieAttr->attr, &attrName);
printf("handleEnumerationType: attrName: %s\n", attrName);
    switch (dieAttr->attr) {
    case DW_AT_name:
setTypeFlag(AT_name);
      if ((dieAttr->attrStrIdx < 0) || (dieAttr->attrStrIdx >= self->dwarfDbgCompileUnitInfo->numAttrStr)) {
        return DWARF_DBG_ERR_BAD_ATTR_STR_IDX;
      }
      attrName = self->dwarfDbgCompileUnitInfo->attrStrs[dieAttr->attrStrIdx];
      found++;
      break;
    case DW_AT_sibling:
setTypeFlag(AT_sibling);
      siblingIdx = dieAttr->uval;
      found++;
      break;
    case DW_AT_type:
setTypeFlag(AT_type);
      result = getTypeRefIdx(self, dieAttr, &dwTypeIdx);
      checkErrOK(result);
      found++;
      break;
    case DW_AT_byte_size:
setTypeFlag(AT_byte_size);
      byteSize = dieAttr->uval;
      found++;
      break;
    case DW_AT_decl_file:
setTypeFlag(AT_decl_file);
      pathNameIdx = dieAttr->sourceFileIdx;
      found++;
      break;
    case DW_AT_decl_line:
setTypeFlag(AT_decl_line);
      lineNo = dieAttr->sourceLineNo;
      found++;
      break;
    default:
printf("ERROR: DWARF_DBG_ERR_UNEXPECTED_ATTR_IN_ENUMERATION_TYPE: 0x%04x\n", dieAttr->attr);
      return DWARF_DBG_ERR_UNEXPECTED_ATTR_IN_ENUMERATION_TYPE;
    }
  }
  if (found == dieInfo->numAttr) {
    result = self->dwarfDbgTypeInfo->addArrayType(self, dwTypeIdx, siblingIdx, &arrayTypeIdx);
    checkErrOK(result);
    dieInfo->tagRefIdx = arrayTypeIdx;
    dieInfo->flags  = TAG_REF_ARRAY_TYPE;
  } else {
printf("ERROR enumerationType not found found: %d numAttr: %d offset: 0x%08x\n", found, dieInfo->numAttr, dieAttr->refOffset);
    return DWARF_DBG_ERR_ENUMERATION_TYPE_NOT_FOUND;
  }
  return result;
}

// =================================== handleEnumerator =========================== 

static uint8_t handleEnumerator(dwarfDbgPtr_t self, dieAndChildrenInfo_t *dieAndChildrenInfo, dieInfo_t *dieInfo, int isSibling) {
  uint8_t result = 0;
  int entryIdx = 0;
  int typeIdx = 0;
  int pointerTypeIdx = 0;
  int maxEntries = 0;
  int attrIdx = 0;
  int prototyped = 0;
  dieInfo_t *dieInfo2 = NULL;
  dieAttr_t *dieAttr = NULL;
  int byteSize;   // DW_AT_byte_size
  int dwTypeIdx;  // DW_AT_type
  int siblingIdx; // DW_AT_sibling
  int arrayTypeIdx = 0;
  int value;
  int found = 0;
  const char *attrName = NULL;
  const char *tagName = NULL;

  result = DWARF_DBG_ERR_OK;
currTypeFlags = ATTR_enumeratorFlags;
  if (isSibling) {
    maxEntries = dieAndChildrenInfo->numSiblings;
  } else {
    maxEntries = dieAndChildrenInfo->numChildren;
  }
  prototyped = 0;
  byteSize = -1;
  dwTypeIdx = -1;
  siblingIdx = -1;
  value = 0;
  attrName = NULL;
  found = 0;
  for(attrIdx = 0; attrIdx < dieInfo->numAttr; attrIdx++) {
    dieAttr = &dieInfo->dieAttrs[attrIdx];
self->dwarfDbgStringInfo->getDW_AT_string(self, dieAttr->attr, &attrName);
printf("handleArrayType: attrName: %s\n", attrName);
    switch (dieAttr->attr) {
    case DW_AT_name:
setTypeFlag(AT_name);
      if ((dieAttr->attrStrIdx < 0) || (dieAttr->attrStrIdx >= self->dwarfDbgCompileUnitInfo->numAttrStr)) {
        return DWARF_DBG_ERR_BAD_ATTR_STR_IDX;
      }
      attrName = self->dwarfDbgCompileUnitInfo->attrStrs[dieAttr->attrStrIdx];
      found++;
      break;
    case DW_AT_const_value:
setTypeFlag(AT_const_value);
      value = dieAttr->uval;
      found++;
      break;
    case DW_AT_sibling:
setTypeFlag(AT_sibling);
      siblingIdx = dieAttr->uval;
      found++;
      break;
    case DW_AT_type:
setTypeFlag(AT_type);
      result = getTypeRefIdx(self, dieAttr, &dwTypeIdx);
      checkErrOK(result);
      found++;
      break;
    default:
printf("ERROR: DWARF_DBG_ERR_UNEXPECTED_ATTR_IN_ENUMERATOR: 0x%04x\n", dieAttr->attr);
      return DWARF_DBG_ERR_UNEXPECTED_ATTR_IN_ENUMERATOR;
    }
  }
  if (found == dieInfo->numAttr) {
    result = self->dwarfDbgTypeInfo->addArrayType(self, dwTypeIdx, siblingIdx, &arrayTypeIdx);
    checkErrOK(result);
    dieInfo->tagRefIdx = arrayTypeIdx;
    dieInfo->flags  = TAG_REF_ENUMERATOR;
  } else {
printf("ERROR enumerator not found found: %d numAttr: %d offset: 0x%08x\n", found, dieInfo->numAttr, dieAttr->refOffset);
    return DWARF_DBG_ERR_ENUMERATOR_NOT_FOUND;
  }
  return result;
}

// =================================== handleMember =========================== 

static uint8_t handleMember(dwarfDbgPtr_t self, dieAndChildrenInfo_t *dieAndChildrenInfo, dieInfo_t *dieInfo, int isSibling) {
  uint8_t result = 0;
  int entryIdx = 0;
  int typeIdx = 0;
  int memberIdx = 0;
  int maxEntries = 0;
  int attrIdx = 0;
  int prototyped = 0;
  dieInfo_t *dieInfo2 = NULL;
  dieAttr_t *dieAttr = NULL;
  int byteSize;      // DW_AT_byte_size
  int bitSize;       // DW_AT_bit_size
  int bitOffset;     // DW_AT_bit_offset
  int dwTypeIdx;     // DW_AT_type
  int pathNameIdx;   // DW_AT_decl_file
  int lineNo;        // DW_AT_decl_line
  int location;      // DW_AT_location
  int siblingIdx;    // DW_AT_sibling
  int found = 0;
  const char *attrName = NULL;
  const char *tagName = NULL;
  char *name = NULL;

  result = DWARF_DBG_ERR_OK;
currTypeFlags = ATTR_memberFlags;
  if (isSibling) {
    maxEntries = dieAndChildrenInfo->numSiblings;
  } else {
    maxEntries = dieAndChildrenInfo->numChildren;
  }
  prototyped = 0;
  byteSize = -1;
  dwTypeIdx = -1;
  pathNameIdx = -1;
  lineNo = 0;
  siblingIdx = -1;
  attrName = NULL;
  location = -1;
  byteSize = -1;
  bitSize = -1;
  bitOffset = 0;
  found = 0;
  for(attrIdx = 0; attrIdx < dieInfo->numAttr; attrIdx++) {
    dieAttr = &dieInfo->dieAttrs[attrIdx];
printf("handleMember: dieAttr: %p\n", dieAttr);
self->dwarfDbgStringInfo->getDW_AT_string(self, dieAttr->attr, &attrName);
printf("handleMember: attrName: %s\n", attrName);
    switch (dieAttr->attr) {
    case DW_AT_name:
setTypeFlag(AT_name);
      if ((dieAttr->attrStrIdx < 0) || (dieAttr->attrStrIdx >= self->dwarfDbgCompileUnitInfo->numAttrStr)) {
        return DWARF_DBG_ERR_BAD_ATTR_STR_IDX;
      }
      attrName = self->dwarfDbgCompileUnitInfo->attrStrs[dieAttr->attrStrIdx];
      found++;
      break;
    case DW_AT_decl_file:
setTypeFlag(AT_decl_file);
      pathNameIdx = dieAttr->sourceFileIdx;
      found++;
      break;
    case DW_AT_decl_line:
setTypeFlag(AT_decl_line);
      lineNo = dieAttr->sourceLineNo;
      found++;
      break;
    case DW_AT_byte_size:
setTypeFlag(AT_byte_size);
      byteSize = dieAttr->uval;
      found++;
      break;
    case DW_AT_bit_size:
setTypeFlag(AT_bit_size);
      bitSize = dieAttr->uval;
      found++;
      break;
    case DW_AT_bit_offset:
setTypeFlag(AT_bit_offset);
      bitOffset = dieAttr->uval;
      found++;
      break;
    case DW_AT_sibling:
setTypeFlag(AT_sibling);
      siblingIdx = dieAttr->uval;
      found++;
      break;
    case DW_AT_type:
setTypeFlag(AT_type);
      result = getTypeRefIdx(self, dieAttr, &dwTypeIdx);
      checkErrOK(result);
      found++;
      break;
    case DW_AT_data_member_location:
setTypeFlag(AT_data_member_location);
      location = dieAttr->uval;
printf("location: %d\n", location);
      found++;
      break;
    default:
printf("ERROR: DWARF_DBG_ERR_UNEXPECTED_ATTR_IN_MEMBER: 0x%04x\n", dieAttr->attr);
      return DWARF_DBG_ERR_UNEXPECTED_ATTR_IN_MEMBER;
    }
  }
  if (found == dieInfo->numAttr) {
printf("EL %p\n", self->dwarfDbgTypeInfo->addMember);
    result = self->dwarfDbgTypeInfo->addMember(self, dwTypeIdx, siblingIdx, "XX", &memberIdx);
    checkErrOK(result);
    dieInfo->tagRefIdx = memberIdx;
    dieInfo->flags  = TAG_REF_MEMBER;
  } else {
printf("ERROR member not found found: %d numAttr: %d offset: 0x%08x\n", found, dieInfo->numAttr, dieAttr->refOffset);
    return DWARF_DBG_ERR_MEMBER_NOT_FOUND;
  }
  return result;
}

// =================================== handlePointerType =========================== 

static uint8_t handlePointerType(dwarfDbgPtr_t self, dieAndChildrenInfo_t *dieAndChildrenInfo, dieInfo_t *dieInfo, int isSibling) {
  uint8_t result = 0;
  int entryIdx = 0;
  int typeIdx = 0;
  int pointerTypeIdx = 0;
  int maxEntries = 0;
//  int maxEntries2 = 0;
  int attrIdx = 0;
  compileUnit_t *compileUnit;
  dieAndChildrenInfo_t *dieAndChildrenInfo2 = NULL;
  dieInfo_t *dieInfo2 = NULL;
  dieAttr_t *dieAttr = NULL;
  int byteSize;   // DW_AT_byte_size
  int dwTypeIdx;  // DW_AT_type
  int found = 0;
//  int idx2 = 0;
//  int isSibling2 = 0;
  char *attrName = NULL;
  const char *tagName = NULL;
//  const char *tagName2 = NULL;

  result = DWARF_DBG_ERR_OK;
currTypeFlags = ATTR_pointerTypeFlags;
  if (isSibling) {
    maxEntries = dieAndChildrenInfo->numSiblings;
  } else {
    maxEntries = dieAndChildrenInfo->numChildren;
  }
  byteSize = -1;
  dwTypeIdx = -1;
  attrName = NULL;
  found = 0;
printf("tag offset: 0x%08x\n", dieInfo->offset);
  for(attrIdx = 0; attrIdx < dieInfo->numAttr; attrIdx++) {
    dieAttr = &dieInfo->dieAttrs[attrIdx];
    switch (dieAttr->attr) {
    case DW_AT_byte_size:
setTypeFlag(AT_byte_size);
      byteSize = dieAttr->byteSize;
printf("byteSize: %d refOffset: 0x%08x\n", byteSize, dieAttr->refOffset);
      found++;
      break;
    case DW_AT_type:
setTypeFlag(AT_type);
printf("PTR DW_AT_type: refOffset: 0x%08x maxEntries: %d\n", dieAttr->refOffset, maxEntries);
      result = getTypeRefIdx(self, dieAttr, &dwTypeIdx);
      checkErrOK(result);
      found++;
      break;
    default:
printf("ERROR: DWARF_DBG_ERR_UNEXPECTED_ATTR_IN_POINTER_TYPE: 0x%04x\n", dieAttr->attr);
      return DWARF_DBG_ERR_UNEXPECTED_ATTR_IN_POINTER_TYPE;
    }
    if (found == dieInfo->numAttr) {
//printf("found 2 break\n");
      break;
    }
  }
  if (found >= dieInfo->numAttr) {
printf("call addPointerType\n");
    result = self->dwarfDbgTypeInfo->addPointerType(self, byteSize, dwTypeIdx, &pointerTypeIdx);
    checkErrOK(result);
    dieInfo->tagRefIdx = pointerTypeIdx;
    dieInfo->flags  = TAG_REF_POINTER_TYPE;
  } else {
printf("ERROR pointerType not found offset: 0x%08x found: %d numAttr: %d\n", dieAttr->refOffset, found, dieInfo->numAttr);
    return DWARF_DBG_ERR_POINTER_TYPE_NOT_FOUND;
  }
  return result;
}

// =================================== handleStructureType =========================== 

static uint8_t handleStructureType(dwarfDbgPtr_t self, dieAndChildrenInfo_t *dieAndChildrenInfo, dieInfo_t *dieInfo, int isSibling) {
  uint8_t result = 0;
  int entryIdx = 0;
  int structureTypeIdx = 0;
  int maxEntries = 0;
  int attrIdx = 0;
  int typeStrIdx;  // DW_AT_name
  int pathNameIdx; // DW_AT_decl_file
  int lineNo;      // DW_AT_decl_line
  int declaration; // DW_AT_declaration
  int byteSize;    // DW_AT_byte_size
  int siblingIdx;  // DW_AT_sibling
  dieAttr_t *dieAttr = NULL;
  int found = 0;
  const char *attrName = NULL;
  const char *tagName = NULL;

  result = DWARF_DBG_ERR_OK;
currTypeFlags = ATTR_structureTypeFlags;
  if (isSibling) {
    maxEntries = dieAndChildrenInfo->numSiblings;
  } else {
    maxEntries = dieAndChildrenInfo->numChildren;
  }
  pathNameIdx = -1;
  lineNo = -1;
  byteSize = -1;
  declaration = 0;
  attrName = NULL;
  found = 0;
  for(attrIdx = 0; attrIdx < dieInfo->numAttr; attrIdx++) {
    dieAttr = &dieInfo->dieAttrs[attrIdx];
    switch (dieAttr->attr) {
    case DW_AT_name:
setTypeFlag(AT_name);
      if ((dieAttr->attrStrIdx < 0) || (dieAttr->attrStrIdx >= self->dwarfDbgCompileUnitInfo->numAttrStr)) {
        return DWARF_DBG_ERR_BAD_ATTR_STR_IDX;
      }
      attrName = self->dwarfDbgCompileUnitInfo->attrStrs[dieAttr->attrStrIdx];
      found++;
      break;
    case DW_AT_decl_file:
setTypeFlag(AT_decl_file);
      pathNameIdx = dieAttr->sourceFileIdx;
      found++;
      break;
    case DW_AT_decl_line:
setTypeFlag(AT_decl_line);
      lineNo = dieAttr->sourceLineNo;
      found++;
      break;
    case DW_AT_byte_size:
setTypeFlag(AT_byte_size);
      byteSize = dieAttr->uval;
      found++;
      break;
    case DW_AT_declaration:
setTypeFlag(AT_declaration);
      declaration = dieAttr->uval;
      found++;
      break;
    case DW_AT_sibling:
setTypeFlag(AT_sibling);
      siblingIdx = dieAttr->uval;
      found++;
      break;
    default:
self->dwarfDbgStringInfo->getDW_AT_string(self, dieAttr->attr, &attrName);
printf("ERROR: DWARF_DBG_ERR_UNEXPECTED_ATTR_IN_STRUCTURE_TYPE: 0x%04x %s offset: 0x%08x\n", dieAttr->attr, attrName, dieInfo->offset);
      return DWARF_DBG_ERR_UNEXPECTED_ATTR_IN_TYPEDEF;
    }
  }
  if (found) {
    result = self->dwarfDbgTypeInfo->addStructureType(self, attrName, byteSize, pathNameIdx, lineNo, declaration, &structureTypeIdx);
    checkErrOK(result);
    dieInfo->tagRefIdx = structureTypeIdx;
    dieInfo->flags  = TAG_REF_STRUCTURE_TYPE;
  } else {
printf("ERROR structureType not found\n");
    return DWARF_DBG_ERR_STRUCTURE_TYPE_NOT_FOUND;
  }
  return result;
}

// =================================== handleSubroutineType =========================== 

static uint8_t handleSubroutineType(dwarfDbgPtr_t self, dieAndChildrenInfo_t *dieAndChildrenInfo, dieInfo_t *dieInfo, int isSibling) {
  uint8_t result = 0;
  int entryIdx = 0;
  int typeIdx = 0;
  int pointerTypeIdx = 0;
  int maxEntries = 0;
  int attrIdx = 0;
  int prototyped = 0;
  dieInfo_t *dieInfo2 = NULL;
  dieAttr_t *dieAttr = NULL;
  int byteSize;   // DW_AT_byte_size
  int dwTypeIdx;  // DW_AT_type
  int siblingIdx; // DW_AT_sibling
  int subroutineTypeIdx = 0;
  int found = 0;
  const char *attrName = NULL;
  const char *tagName = NULL;

  result = DWARF_DBG_ERR_OK;
currTypeFlags = ATTR_subroutineTypeFlags;
  if (isSibling) {
    maxEntries = dieAndChildrenInfo->numSiblings;
  } else {
    maxEntries = dieAndChildrenInfo->numChildren;
  }
  prototyped = 0;
  byteSize = -1;
  dwTypeIdx = -1;
  siblingIdx = -1;
  attrName = NULL;
  found = 0;
  for(attrIdx = 0; attrIdx < dieInfo->numAttr; attrIdx++) {
    dieAttr = &dieInfo->dieAttrs[attrIdx];
self->dwarfDbgStringInfo->getDW_AT_string(self, dieAttr->attr, &attrName);
//printf("handleSubroutineType: attrName: %s\n", attrName);
    switch (dieAttr->attr) {
    case DW_AT_byte_size:
setTypeFlag(AT_byte_size);
      byteSize = dieAttr->byteSize;
      found++;
      break;
    case DW_AT_prototyped:
setTypeFlag(AT_prototyped);
      prototyped = dieAttr->uval;
      found++;
      break;
    case DW_AT_sibling:
setTypeFlag(AT_sibling);
      siblingIdx = dieAttr->uval;
      found++;
      break;
    case DW_AT_type:
setTypeFlag(AT_type);
      result = getTypeRefIdx(self, dieAttr, &dwTypeIdx);
      checkErrOK(result);
      found++;
      break;
    default:
printf("ERROR: DWARF_DBG_ERR_UNEXPECTED_ATTR_IN_SUBROUTINE: 0x%04x\n", dieAttr->attr);
      return DWARF_DBG_ERR_UNEXPECTED_ATTR_IN_SUBROUTINE;
    }
  }
  if (found == dieInfo->numAttr) {
    result = self->dwarfDbgTypeInfo->addSubroutineType(self, prototyped, byteSize, dwTypeIdx, siblingIdx, &subroutineTypeIdx);
    checkErrOK(result);
    dieInfo->tagRefIdx = subroutineTypeIdx;
    dieInfo->flags  = TAG_REF_SUBROUTINE_TYPE;
  } else {
printf("ERROR subroutineType not found found: %d numAttr: %d offset: 0x%08x\n", found, dieInfo->numAttr, dieAttr->refOffset);
    return DWARF_DBG_ERR_SUBROUTINE_TYPE_NOT_FOUND;
  }
  return result;
}

// =================================== handleTypedef =========================== 

static uint8_t handleTypedef(dwarfDbgPtr_t self, dieAndChildrenInfo_t *dieAndChildrenInfo, dieInfo_t *dieInfo, int isSibling) {
  uint8_t result = 0;
  int entryIdx = 0;
  int typeIdx = 0;
  int typedefIdx = 0;
  int maxEntries = 0;
  int attrIdx = 0;
  dieInfo_t *dieInfo2 = NULL;
  dieAttr_t *dieAttr = NULL;
  int typeStrIdx;  // DW_AT_name
  int pathNameIdx; // DW_AT_decl_file
  int lineNo;      // DW_AT_decl_line
  int dwTypeIdx;   // DW_AT_type
  int found = 0;
  char *attrName = NULL;
  const char *tagName = NULL;

  result = DWARF_DBG_ERR_OK;
currTypeFlags = ATTR_typedefFlags;
  if (isSibling) {
    maxEntries = dieAndChildrenInfo->numSiblings;
  } else {
    maxEntries = dieAndChildrenInfo->numChildren;
  }
  typeStrIdx = -1;
  pathNameIdx = -1;
  lineNo = -1;
  dwTypeIdx = -1;
  attrName = NULL;
  found = 0;
  for(attrIdx = 0; attrIdx < dieInfo->numAttr; attrIdx++) {
    dieAttr = &dieInfo->dieAttrs[attrIdx];
    switch (dieAttr->attr) {
    case DW_AT_name:
setTypeFlag(AT_name);
      if ((dieAttr->attrStrIdx < 0) || (dieAttr->attrStrIdx >= self->dwarfDbgCompileUnitInfo->numAttrStr)) {
        return DWARF_DBG_ERR_BAD_ATTR_STR_IDX;
      }
      attrName = self->dwarfDbgCompileUnitInfo->attrStrs[dieAttr->attrStrIdx];
      found++;
      break;
    case DW_AT_decl_file:
setTypeFlag(AT_decl_file);
      pathNameIdx = dieAttr->sourceFileIdx;
      found++;
      break;
    case DW_AT_decl_line:
setTypeFlag(AT_decl_line);
      lineNo = dieAttr->sourceLineNo;
      found++;
      break;
    case DW_AT_type:
setTypeFlag(AT_type);
      result = getTypeRefIdx(self, dieAttr, &dwTypeIdx);
      checkErrOK(result);
      found++;
      break;
    default:
printf("ERROR: DWARF_DBG_ERR_UNEXPECTED_ATTR_IN_TYPEDEF: 0x%04x\n", dieAttr->attr);
      return DWARF_DBG_ERR_UNEXPECTED_ATTR_IN_TYPEDEF;
    }
  }
  if (found == dieInfo->numAttr) {
    result = self->dwarfDbgTypeInfo->addTypeDef(self, attrName, pathNameIdx, lineNo, dwTypeIdx, &typedefIdx);
    checkErrOK(result);
    dieInfo->tagRefIdx = typedefIdx;
    dieInfo->flags  = TAG_REF_TYPEDEF;
  } else {
printf("ERROR typedef not found found: %d numAttr: %d offset: 0x%08x\n", found, dieInfo->numAttr, dieAttr->refOffset);
    return DWARF_DBG_ERR_TYPEDEF_NOT_FOUND;
  }
  return result;
}

// =================================== handleUnionType =========================== 

static uint8_t handleUnionType(dwarfDbgPtr_t self, dieAndChildrenInfo_t *dieAndChildrenInfo, dieInfo_t *dieInfo, int isSibling) {
  uint8_t result = 0;
  int entryIdx = 0;
  int typeIdx = 0;
  int pointerTypeIdx = 0;
  int maxEntries = 0;
  int attrIdx = 0;
  int prototyped = 0;
  dieInfo_t *dieInfo2 = NULL;
  dieAttr_t *dieAttr = NULL;
  int byteSize;    // DW_AT_byte_size
  int dwTypeIdx;   // DW_AT_type
  int pathNameIdx; // DW_AT_decl_file
  int lineNo;      // DW_AT_decl_line
  int siblingIdx;  // DW_AT_sibling
  int arrayTypeIdx = 0;
  int declaration = 0;
  int found = 0;
  const char *attrName = NULL;
  const char *tagName = NULL;

  result = DWARF_DBG_ERR_OK;
currTypeFlags = ATTR_unionTypeFlags;
  if (isSibling) {
    maxEntries = dieAndChildrenInfo->numSiblings;
  } else {
    maxEntries = dieAndChildrenInfo->numChildren;
  }
  prototyped = 0;
  declaration = 0;
  byteSize = -1;
  dwTypeIdx = -1;
  siblingIdx = -1;
  pathNameIdx = -1;
  lineNo = -1;
  attrName = NULL;
  found = 0;
  for(attrIdx = 0; attrIdx < dieInfo->numAttr; attrIdx++) {
    dieAttr = &dieInfo->dieAttrs[attrIdx];
self->dwarfDbgStringInfo->getDW_AT_string(self, dieAttr->attr, &attrName);
printf("handleUnionType: attrName: %s\n", attrName);
    switch (dieAttr->attr) {
    case DW_AT_declaration:
setTypeFlag(AT_declaration);
      declaration = dieAttr->uval;
      found++;
      break;
    case DW_AT_sibling:
setTypeFlag(AT_sibling);
      siblingIdx = dieAttr->uval;
      found++;
      break;
    case DW_AT_name:
setTypeFlag(AT_name);
      if ((dieAttr->attrStrIdx < 0) || (dieAttr->attrStrIdx >= self->dwarfDbgCompileUnitInfo->numAttrStr)) {
        return DWARF_DBG_ERR_BAD_ATTR_STR_IDX;
      }
      attrName = self->dwarfDbgCompileUnitInfo->attrStrs[dieAttr->attrStrIdx];
      found++;
      break;
    case DW_AT_byte_size:
setTypeFlag(AT_byte_size);
      byteSize = dieAttr->byteSize;
      found++;
      break;
    case DW_AT_decl_file:
setTypeFlag(AT_decl_file);
      pathNameIdx = dieAttr->sourceFileIdx;
      found++;
      break;
    case DW_AT_decl_line:
setTypeFlag(AT_decl_line);
      lineNo = dieAttr->sourceLineNo;
      found++;
      break;
    case DW_AT_type:
setTypeFlag(AT_type);
      result = getTypeRefIdx(self, dieAttr, &dwTypeIdx);
      checkErrOK(result);
      found++;
      break;
    default:
printf("ERROR: DWARF_DBG_ERR_UNEXPECTED_ATTR_IN_UNION_TYPE: 0x%04x\n", dieAttr->attr);
      return DWARF_DBG_ERR_UNEXPECTED_ATTR_IN_ARRAY_TYPE;
    }
  }
  if (found == dieInfo->numAttr) {
    result = self->dwarfDbgTypeInfo->addArrayType(self, dwTypeIdx, siblingIdx, &arrayTypeIdx);
    checkErrOK(result);
    dieInfo->tagRefIdx = arrayTypeIdx;
    dieInfo->flags  = TAG_REF_ARRAY_TYPE;
  } else {
printf("ERROR unionType not found found: %d numAttr: %d offset: 0x%08x\n", found, dieInfo->numAttr, dieAttr->refOffset);
    return DWARF_DBG_ERR_UNION_TYPE_NOT_FOUND;
  }
  return result;
}

// =================================== handleVolatileType =========================== 

static uint8_t handleVolatileType(dwarfDbgPtr_t self, dieAndChildrenInfo_t *dieAndChildrenInfo, dieInfo_t *dieInfo, int isSibling) {
  uint8_t result = 0;
  int entryIdx = 0;
  int typeIdx = 0;
  int pointerTypeIdx = 0;
  int maxEntries = 0;
  int attrIdx = 0;
  int prototyped = 0;
  dieInfo_t *dieInfo2 = NULL;
  dieAttr_t *dieAttr = NULL;
  int byteSize;   // DW_AT_byte_size
  int dwTypeIdx;  // DW_AT_type
  int siblingIdx; // DW_AT_sibling
  int arrayTypeIdx = 0;
  int found = 0;
  const char *attrName = NULL;
  const char *tagName = NULL;
  char *name = NULL;

  result = DWARF_DBG_ERR_OK;
currTypeFlags = ATTR_volatileTypeFlags;
  if (isSibling) {
    maxEntries = dieAndChildrenInfo->numSiblings;
  } else {
    maxEntries = dieAndChildrenInfo->numChildren;
  }
  prototyped = 0;
  byteSize = -1;
  dwTypeIdx = -1;
  siblingIdx = -1;
  attrName = NULL;
  found = 0;
  for(attrIdx = 0; attrIdx < dieInfo->numAttr; attrIdx++) {
    dieAttr = &dieInfo->dieAttrs[attrIdx];
self->dwarfDbgStringInfo->getDW_AT_string(self, dieAttr->attr, &attrName);
printf("handleVolatileType: attrName: %s\n", attrName);
    switch (dieAttr->attr) {
    case DW_AT_sibling:
setTypeFlag(AT_sibling);
      siblingIdx = dieAttr->uval;
      found++;
      break;
    case DW_AT_type:
setTypeFlag(AT_type);
      result = getTypeRefIdx(self, dieAttr, &dwTypeIdx);
      checkErrOK(result);
      found++;
      break;
    default:
printf("ERROR: DWARF_DBG_ERR_UNEXPECTED_ATTR_IN_VOLATILE_TYPE: 0x%04x\n", dieAttr->attr);
      return DWARF_DBG_ERR_UNEXPECTED_ATTR_IN_VOLATILE_TYPE;
    }
  }
  if (found == dieInfo->numAttr) {
    result = self->dwarfDbgTypeInfo->addVolatileType(self, dwTypeIdx, siblingIdx, "YY", &arrayTypeIdx);
    checkErrOK(result);
    dieInfo->tagRefIdx = arrayTypeIdx;
    dieInfo->flags  = TAG_REF_ARRAY_TYPE;
  } else {
printf("ERROR volatileType not found found: %d numAttr: %d offset: 0x%08x\n", found, dieInfo->numAttr, dieAttr->refOffset);
    return DWARF_DBG_ERR_VOLATILE_TYPE_NOT_FOUND;
  }
  return result;
}
#endif

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
        result = self->dwarfDbgTypeInfo->handleType(self, dieAndChildrenInfo, dieInfo, isSibling, &self->dwarfDbgTypeInfo->dwEnumerationTypeInfos);
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

#ifdef NOTDEF
  self->dwarfDbgTypeInfo->addArrayType = &addArrayType;
  self->dwarfDbgTypeInfo->addBaseType = &addBaseType;
  self->dwarfDbgTypeInfo->addConstType = &addConstType;
  self->dwarfDbgTypeInfo->addEnumerationType = &addEnumerationType;
  self->dwarfDbgTypeInfo->addEnumerator = &addEnumerator;
  self->dwarfDbgTypeInfo->addMember = &addMember;
  self->dwarfDbgTypeInfo->addPointerType = &addPointerType;
  self->dwarfDbgTypeInfo->addStructureType = &addStructureType;
  self->dwarfDbgTypeInfo->addSubroutineType = &addSubroutineType;
  self->dwarfDbgTypeInfo->addTypeDef = &addTypeDef;
  self->dwarfDbgTypeInfo->addUnionType = &addUnionType;
  self->dwarfDbgTypeInfo->addVolatileType = &addVolatileType;
#endif

  self->dwarfDbgTypeInfo->handleType = &handleType;

#ifdef NOTDEF
  self->dwarfDbgTypeInfo->handleArrayType = &handleArrayType;
  self->dwarfDbgTypeInfo->handleConstType = &handleConstType;
  self->dwarfDbgTypeInfo->handleEnumerationType = &handleEnumerationType;
  self->dwarfDbgTypeInfo->handleEnumerator = &handleEnumerator;
  self->dwarfDbgTypeInfo->handleMember = &handleMember;
  self->dwarfDbgTypeInfo->handlePointerType = &handlePointerType;
  self->dwarfDbgTypeInfo->handleStructureType = &handleStructureType;
  self->dwarfDbgTypeInfo->handleSubroutineType = &handleSubroutineType;
  self->dwarfDbgTypeInfo->handleTypedef = &handleTypedef;
  self->dwarfDbgTypeInfo->handleUnionType = &handleUnionType;
  self->dwarfDbgTypeInfo->handleVolatileType = &handleVolatileType;
#endif

  self->dwarfDbgTypeInfo->addTypes = &addTypes;
  self->dwarfDbgTypeInfo->addChildrenTypes = &addChildrenTypes;
  self->dwarfDbgTypeInfo->addSiblingsTypes = &addSiblingsTypes;
  return result;
}
