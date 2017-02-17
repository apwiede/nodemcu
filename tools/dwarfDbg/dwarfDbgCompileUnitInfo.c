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
 * File:   dwarfDbgCompileUnitInfo.c
 * Author: Arnulf P. Wiedemann <arnulf@wiedemann-pri.de>
 *
 * Created on February 10, 2017
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libelf.h>
#include <stdlib.h>
#include <string.h>

#include "dwarfDbgInt.h"

/* *************************************************************************
 * compileUnit
 *   compileUnitHeaderLength
 *   versionStamp
 *   abbrevOffset
 *   addressSize
 *   lengthSize
 *   extensionSize
 *   signature
 *   typeOffset
 *   nextCompileUnitOffset
 *   compileUnitType
 *   compileUnitDie
 *   overallOffset
 *   shortFileName
 *   longFileName
 *   pathNameIdx
 *   fileInfoIdx
 *   level
 *   isCompileUnitDie
 *   maxFileLineInfo
 *   numFileLineInfo
 *   fileInfos
 *   maxSourceFile
 *   numSourceFile
 *   sourceFiles
 *   numDieAndChildren
 *   maxDieAndChildren
 *   dieAndChildrenInfos
 *   maxRangeInfo
 *   numRangeInfo
 *   rangeInfos
 */ 


// =================================== getAddressSizeAndMax =========================== 

static uint8_t getAddressSizeAndMax(dwarfDbgPtr_t self, Dwarf_Half *size, Dwarf_Addr *max, Dwarf_Error *err) {
  int dres = 0;
  Dwarf_Half lsize = 4;
  /* Get address size and largest representable address */
  dres = dwarf_get_address_size(self->elfInfo.dbg, &lsize, err);
  if (dres != DW_DLV_OK) {
    printf("get_address_size() dres: %d err: %p", dres, *err);
    return DWARF_DBG_ERR_CANNOT_GET_ADDR_SIZE;
  }
  if (max) {
    *max = (lsize == 8 ) ? 0xffffffffffffffffULL : 0xffffffff;
  }
  if (size) {
    *size = lsize;
  }
  return DWARF_DBG_ERR_OK;
}

// =================================== addCompileUnit =========================== 

static uint8_t addCompileUnit(dwarfDbgPtr_t self) {
  uint8_t result = 0;
  int res = 0;
  compileUnit_t *compileUnit = NULL;
  Dwarf_Error err = NULL;
  Dwarf_Off DIEOffset = 0;
  Dwarf_Attribute nameAttr = 0;
  char *fileName = NULL;
  char *longFileName = NULL;
  char *shortFileName = NULL;

  result = DWARF_DBG_ERR_OK;
  if (self->dwarfDbgCompileUnitInfo->maxCompileUnit <= self->dwarfDbgCompileUnitInfo->numCompileUnit) {
    self->dwarfDbgCompileUnitInfo->maxCompileUnit += 50;
    if (self->dwarfDbgCompileUnitInfo->compileUnits == NULL) {
      self->dwarfDbgCompileUnitInfo->compileUnits = (compileUnit_t *)ckalloc(sizeof(compileUnit_t) * self->dwarfDbgCompileUnitInfo->maxCompileUnit);
      if (self->dwarfDbgCompileUnitInfo->compileUnits == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      self->dwarfDbgCompileUnitInfo->compileUnits = (compileUnit_t *)ckrealloc((char *)self->dwarfDbgCompileUnitInfo->compileUnits, sizeof(compileUnit_t) * self->dwarfDbgCompileUnitInfo->maxCompileUnit);
      if (self->dwarfDbgCompileUnitInfo->compileUnits == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
DWARF_DBG_PRINT(self, "C", 1, "  >>compileUnitIdx: %d\n", self->dwarfDbgCompileUnitInfo->numCompileUnit);
  compileUnit = &self->dwarfDbgCompileUnitInfo->compileUnits[self->dwarfDbgCompileUnitInfo->numCompileUnit];
  memset(compileUnit, 0, sizeof(compileUnit_t));
  self->dwarfDbgCompileUnitInfo->currCompileUnitIdx = self->dwarfDbgCompileUnitInfo->numCompileUnit;
  self->dwarfDbgCompileUnitInfo->numCompileUnit++;
  self->dwarfDbgCompileUnitInfo->currCompileUnit = compileUnit;
DWARF_DBG_PRINT(self, "C", 1, "  >>addCompileUnit end\n");

  res = dwarf_next_cu_header_d(self->elfInfo.dbg, 1,
    &compileUnit->compileUnitHeaderLength,
    &compileUnit->versionStamp,
    &compileUnit->abbrevOffset,
    &compileUnit->addressSize,
    &compileUnit->lengthSize,
    &compileUnit->extensionSize,
    &compileUnit->signature,
    &compileUnit->typeOffset,
    &compileUnit->nextCompileUnitOffset,
    &compileUnit->compileUnitType,
    &err);
//printf("after dwarf_next_cu_header_d nres: %d loop_count: %d\n", nres, loopCount);
  if (res == DW_DLV_NO_ENTRY) {
    // we have processed all entries
   return DWARF_DBG_ERR_NO_ENTRY;
  }
  if (res != DW_DLV_OK) {
    return DWARF_DBG_ERR_CANNOT_GET_NEXT_COMPILE_UNIT;
  }
  res = dwarf_siblingof_b(self->elfInfo.dbg, NULL,/* is_info */1, &compileUnit->compileUnitDie, &err);
  if (res != DW_DLV_OK) {
DWARF_DBG_PRINT(self, "C", 1, "siblingof cu header sres: %d err: %p", res, err);
    return DWARF_DBG_ERR_CANNOT_GET_SIBLING_OF_COMPILE_UNIT;
  }
  res = dwarf_die_offsets(compileUnit->compileUnitDie, &compileUnit->overallOffset, &DIEOffset, &err);
  if (res != DW_DLV_OK) {
    return DWARF_DBG_ERR_CANNOT_GET_DIE_OFFSETS;
  }
  res = dwarf_attr(compileUnit->compileUnitDie, DW_AT_name, &nameAttr, &err);
  if (res != DW_DLV_OK) {
    return DWARF_DBG_ERR_CANNOT_GET_ATTR;
  }
  res = dwarf_formstring(nameAttr, &longFileName, &err);
  if (res != DW_DLV_OK) {
    return DWARF_DBG_ERR_CANNOT_GET_NAME_FORMSTRING;
  }
DWARF_DBG_PRINT(self, "C", 1, "  >>name: %d %s\n", nameAttr, longFileName);
  fileName = strrchr(longFileName,'/');
  if (!fileName) {
    fileName = strrchr(longFileName,'\\');
  }
  if (fileName) {
    ++fileName;
  } else {
    fileName = longFileName;
  }
  shortFileName = fileName;
  compileUnit->shortFileName = ckalloc(strlen(shortFileName) + 1);
  memset(compileUnit->shortFileName, 0, strlen(shortFileName) + 1);
  memcpy(compileUnit->shortFileName, shortFileName, strlen(shortFileName));
  compileUnit->longFileName = ckalloc(strlen(longFileName) + 1);
  memset(compileUnit->longFileName, 0, strlen(longFileName) + 1);
  memcpy(compileUnit->longFileName, longFileName, strlen(longFileName));
DWARF_DBG_PRINT(self, "C", 1, "  >>name2: %s %s!\n", compileUnit->shortFileName, compileUnit->longFileName);
  return result;
}

// =================================== dwarfDbgCompileUnitInfoInit =========================== 

int dwarfDbgCompileUnitInfoInit (dwarfDbgPtr_t self) {
  uint8_t result;
  self->dwarfDbgCompileUnitInfo->maxCompileUnit = 0;
  self->dwarfDbgCompileUnitInfo->numCompileUnit = 0;
  self->dwarfDbgCompileUnitInfo->compileUnits = NULL;

  result = DWARF_DBG_ERR_OK;
  self->dwarfDbgCompileUnitInfo->getAddressSizeAndMax = &getAddressSizeAndMax;
  self->dwarfDbgCompileUnitInfo->addCompileUnit = &addCompileUnit;
  return result;
}
