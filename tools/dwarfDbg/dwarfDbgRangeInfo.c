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
 * File:   dwarfDbgRangeInfo.c
 * Author: Arnulf P. Wiedemann <arnulf@wiedemann-pri.de>
 *
 * Created on February 12, 2017
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libelf.h>
#include <stdlib.h>
#include <string.h>

#include "dwarfDbgInt.h"

 	

void qsort(void *base, size_t numelem, size_t size, int (*cmp)(const void *e1, const void *e2));

// =================================== addRangeInfo =========================== 

static uint8_t addRangeInfo(dwarfDbgPtr_t self, Dwarf_Addr dwr_addr1, Dwarf_Addr dwr_addr2, enum Dwarf_Ranges_Entry_Type dwrType, int *rangeInfoIdx) {
  uint8_t result;
  compileUnit_t *compileUnit;
  rangeInfo_t *rangeInfo;
  const char *dwrTypeStr;

  result = self->dwarfDbgStringInfo->getDW_RANGES_TYPE_string(self, dwrType, &dwrTypeStr);
  checkErrOK(result);
  DWARF_DBG_PRINT(self, "R", 1, "                      dwr1: 0x%08x dwr2: 0x%08x dwrType: %s\n", dwr_addr1, dwr_addr2, dwrTypeStr);
  result = DWARF_DBG_ERR_OK;
  compileUnit = self->dwarfDbgCompileUnitInfo->currCompileUnit;
  if (compileUnit->fileInfos == NULL) {
    // seems to be no file infos!!
    return result;
  }
  if (compileUnit->maxRangeInfo <= compileUnit->numRangeInfo) {
    compileUnit->maxRangeInfo += 5;
    if (compileUnit->rangeInfos == NULL) {
      compileUnit->rangeInfos = (rangeInfo_t *)ckalloc(sizeof(rangeInfo_t) * compileUnit->maxRangeInfo);
      if (compileUnit->rangeInfos == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      compileUnit->rangeInfos = (rangeInfo_t *)ckrealloc((char *)compileUnit->rangeInfos, sizeof(rangeInfo_t) * compileUnit->maxRangeInfo);
      if (compileUnit->rangeInfos == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
  rangeInfo = &compileUnit->rangeInfos[compileUnit->numRangeInfo];
  rangeInfo->dwr_addr1 = dwr_addr1;
  rangeInfo->dwr_addr2 = dwr_addr2;
  rangeInfo->dwr_type = dwrType;
  *rangeInfoIdx = compileUnit->numRangeInfo;
  compileUnit->numRangeInfo++;
  return result;
}

// =================================== handleRangeInfos =========================== 

static uint8_t handleRangeInfos(dwarfDbgPtr_t self, Dwarf_Attribute attrIn) {
  uint8_t result;
  int rres = 0;
  int fres = 0;
  Dwarf_Ranges *rangeset = 0;
  Dwarf_Signed rangecount = 0;
  Dwarf_Unsigned bytecount = 0;
  Dwarf_Unsigned original_off = 0;
  Dwarf_Error err;
  compileUnit_t *compileUnit;
  int i;
  int rangeIdx = 0;

  result = DWARF_DBG_ERR_OK;
  compileUnit = self->dwarfDbgCompileUnitInfo->currCompileUnit;
  fres = dwarf_global_formref(attrIn, &original_off, &err);
  if (fres != DW_DLV_OK) {
    return DWARF_DBG_ERR_CANNOT_GET_GLOBAL_FORMREF;
  }
  if (fres == DW_DLV_OK) {
    rres = dwarf_get_ranges_a(self->elfInfo.dbg, original_off, compileUnit->compileUnitDie, &rangeset, &rangecount, &bytecount, &err);
    DWARF_DBG_PRINT(self, "R", 1, " count: %d bytecount: %d\n", rangecount, bytecount);
    for (i = 0; i < rangecount; i++) {
      Dwarf_Ranges *range = &rangeset[i];
      result = self->dwarfDbgRangeInfo->addRangeInfo(self, range->dwr_addr1, range->dwr_addr2, range->dwr_type, &rangeIdx);
      // FIXME need to store rangeIdx somwhere!!
    }
  }
  return result;
}

// =================================== dwarfDbgRangeInfoInit =========================== 

int dwarfDbgRangeInfoInit (dwarfDbgPtr_t self) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;
  self->dwarfDbgRangeInfo->addRangeInfo = &addRangeInfo;
  self->dwarfDbgRangeInfo->handleRangeInfos = &handleRangeInfos;
  return result;
}
