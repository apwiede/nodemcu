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
 * File:   dwarfDbgSubProgramInfo.c
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

// =================================== addFormalParameterInfo =========================== 

static uint8_t addFormalParameterInfo(dwarfDbgPtr_t self, int subProgramInfoIdx, int formalParameterDieInfoIdx, int *formalParameterInfoIdx) {
  uint8_t result;
  compileUnit_t *compileUnit;
  subProgramInfo_t *subProgramInfo;

  result = DWARF_DBG_ERR_OK;
  compileUnit = self->dwarfDbgCompileUnitInfo->currCompileUnit;
  if (compileUnit->subProgramInfos == NULL) {
    // not in a subProgram
    return result;
  }
printf("addFormalParameterInfo: %d %d\n", subProgramInfoIdx, formalParameterDieInfoIdx);
  subProgramInfo = &compileUnit->subProgramInfos[subProgramInfoIdx];
printf("compileUnit: %p subProgramInfo: %p currSubProgramInfoIdx: %d\n", compileUnit, subProgramInfo, compileUnit->currSubProgramInfoIdx);
  if (subProgramInfo->maxFormalParameterInfo <= subProgramInfo->numFormalParameterInfo) {
    subProgramInfo->maxFormalParameterInfo += 5;
    if (subProgramInfo->formalParameterInfos == NULL) {
      subProgramInfo->formalParameterInfos = (int *)ckalloc(sizeof(int *) * subProgramInfo->maxFormalParameterInfo);
      if (subProgramInfo->formalParameterInfos == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      subProgramInfo->formalParameterInfos = (int *)ckrealloc((char *)subProgramInfo->formalParameterInfos, sizeof(int *) * subProgramInfo->maxFormalParameterInfo);
      if (subProgramInfo->formalParameterInfos == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
  subProgramInfo->formalParameterInfos[subProgramInfo->numFormalParameterInfo] = formalParameterDieInfoIdx;
  *formalParameterInfoIdx = subProgramInfo->numFormalParameterInfo;
  subProgramInfo->numFormalParameterInfo++;
  return result;
}

// =================================== addVariableInfo =========================== 

static uint8_t addVariableInfo(dwarfDbgPtr_t self, int subProgramInfoIdx, int variableDieInfoIdx, int *variableInfoIdx) {
  uint8_t result;
  compileUnit_t *compileUnit;
  subProgramInfo_t *subProgramInfo;

  result = DWARF_DBG_ERR_OK;
  compileUnit = self->dwarfDbgCompileUnitInfo->currCompileUnit;
printf("addVariableInfo: %d %d\n", subProgramInfoIdx, variableDieInfoIdx);
  if (compileUnit->subProgramInfos == NULL) {
    // not in a subProgram
    return result;
  }
  subProgramInfo = &compileUnit->subProgramInfos[subProgramInfoIdx];
printf("compileUnit: %p subProgramInfo: %p\n", compileUnit, subProgramInfo);
  if (subProgramInfo->maxVariableInfo <= subProgramInfo->numVariableInfo) {
    subProgramInfo->maxVariableInfo += 5;
    if (subProgramInfo->variableInfos == NULL) {
      subProgramInfo->variableInfos = (int *)ckalloc(sizeof(int *) * subProgramInfo->maxVariableInfo);
      if (subProgramInfo->variableInfos == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      subProgramInfo->variableInfos = (int *)ckrealloc((char *)subProgramInfo->variableInfos, sizeof(int *) * subProgramInfo->maxVariableInfo);
      if (subProgramInfo->variableInfos == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
  subProgramInfo->variableInfos[subProgramInfo->numVariableInfo] = variableDieInfoIdx;
  *variableInfoIdx = subProgramInfo->numVariableInfo;
  subProgramInfo->numVariableInfo++;
  return result;
}

// =================================== addSubProgramInfo =========================== 

static uint8_t addSubProgramInfo(dwarfDbgPtr_t self, int subProgramDieInfoIdx, int isSibling, int *subProgramInfoIdx) {
  uint8_t result;
  compileUnit_t *compileUnit;
  subProgramInfo_t *subProgramInfo;

printf("addSubProgramInfo: %d %d\n", subProgramDieInfoIdx, isSibling);
  result = DWARF_DBG_ERR_OK;
  compileUnit = self->dwarfDbgCompileUnitInfo->currCompileUnit;
  if (compileUnit->maxSubProgramInfo <= compileUnit->numSubProgramInfo) {
    compileUnit->maxSubProgramInfo += 5;
    if (compileUnit->subProgramInfos == NULL) {
      compileUnit->subProgramInfos = (subProgramInfo_t *)ckalloc(sizeof(subProgramInfo_t) * compileUnit->maxSubProgramInfo);
      if (compileUnit->subProgramInfos == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      compileUnit->subProgramInfos = (subProgramInfo_t *)ckrealloc((char *)compileUnit->subProgramInfos, sizeof(subProgramInfo_t) * compileUnit->maxSubProgramInfo);
      if (compileUnit->subProgramInfos == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
  subProgramInfo = &compileUnit->subProgramInfos[compileUnit->numSubProgramInfo];
  memset(subProgramInfo, 0, sizeof(subProgramInfo_t));
  subProgramInfo->dieInfoIdx = subProgramDieInfoIdx;
  subProgramInfo->isSibling = isSibling;
  *subProgramInfoIdx = compileUnit->numSubProgramInfo;
  compileUnit->numSubProgramInfo++;
  return result;
}

// =================================== dwarfDbgSubProgramInfoInit =========================== 

int dwarfDbgSubProgramInfoInit (dwarfDbgPtr_t self) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;
  self->dwarfDbgSubProgramInfo->addFormalParameterInfo = &addFormalParameterInfo;
  self->dwarfDbgSubProgramInfo->addVariableInfo = &addVariableInfo;
  self->dwarfDbgSubProgramInfo->addSubProgramInfo = &addSubProgramInfo;
  return result;
}
