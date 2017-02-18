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
printf("== numTypeStrs: %d %s\n", self->dwarfDbgTypeInfo->numTypeStr, str);
  typeStr = &self->dwarfDbgTypeInfo->typeStrs[self->dwarfDbgTypeInfo->numTypeStr];
  *typeStr = ckalloc(strlen(str) + 1);
  memset(*typeStr, 0, strlen(str) + 1);
  memcpy(*typeStr, str, strlen(str));
  *typeStrIdx = self->dwarfDbgTypeInfo->numTypeStr;
  self->dwarfDbgTypeInfo->numTypeStr++;
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
          free((char *)name); // undo strdup
          *baseTypeIdx = idx;
          return result;
        }
      }
    }
  }
  free((char *)name); // undo strdup
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
            free((char *)name); // undo strdup
            *typeDefIdx = idx;
            return result;
          }
        }
      }
    }
  }
  free((char *)name); // undo strdup
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

// =================================== addConstType =========================== 

static uint8_t addConstType(dwarfDbgPtr_t self, int byteSize, int encoding, char *name, int *baseTypeIdx) {
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

static uint8_t addPointerType(dwarfDbgPtr_t self, int byteSize, int encoding, char *name, int *baseTypeIdx) {
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

// =================================== addStructureType =========================== 

static uint8_t addStructureType(dwarfDbgPtr_t self, int byteSize, int encoding, char *name, int *baseTypeIdx) {
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

// =================================== addArrayType =========================== 

static uint8_t addArrayType(dwarfDbgPtr_t self, int byteSize, int encoding, char *name, int *baseTypeIdx) {
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

// =================================== dwarfDbgTypeInfoInit =========================== 

int dwarfDbgTypeInfoInit (dwarfDbgPtr_t self) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;
  self->dwarfDbgTypeInfo->addTypeStr = &addTypeStr;
  self->dwarfDbgTypeInfo->addBaseType = &addBaseType;
  self->dwarfDbgTypeInfo->addTypeDef = &addTypeDef;
  self->dwarfDbgTypeInfo->addConstType = &addConstType;
  self->dwarfDbgTypeInfo->addMember = &addMember;
  self->dwarfDbgTypeInfo->addPointerType = &addPointerType;
  self->dwarfDbgTypeInfo->addStructureType = &addStructureType;
  self->dwarfDbgTypeInfo->addArrayType = &addArrayType;
  self->dwarfDbgTypeInfo->addEnumerationType = &addEnumerationType;
  self->dwarfDbgTypeInfo->addEnumerator = &addEnumerator;
  self->dwarfDbgTypeInfo->addUnionType = &addUnionType;
  self->dwarfDbgTypeInfo->addVolatileType = &addVolatileType;
  return result;
}
