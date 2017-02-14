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
 * File:   dwarfDbgFrameInfo.c
 * Author: Arnulf P. Wiedemann <arnulf@wiedemann-pri.de>
 *
 * Created on February 07, 2017
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libelf.h>
#include <stdlib.h>
#include <string.h>

#include "dwarfDbgInt.h"
#include "../libdwarf/dwarf_tsearch.h"

struct Addr_Map_Entry {
    Dwarf_Unsigned mp_key;
    char * mp_name;
};

struct Addr_Map_Entry * addrMapFind(Dwarf_Unsigned addr, void **map);

// =================================== addrMapCompareFunc =========================== 

static int addrMapCompareFunc(const void *l, const void *r) {
  const struct Addr_Map_Entry *ml = l;
  const struct Addr_Map_Entry *mr = r;
  if (ml->mp_key < mr->mp_key) {
    return -1;
  }
  if (ml->mp_key > mr->mp_key) {
    return 1;
  }
  return 0;
}

// =================================== addrMapFree =========================== 

static void addrMapFreeFunc(void *mx) {
  struct Addr_Map_Entry *m = mx;
  if (!m) {
    return;
  }
  ckfree(m->mp_name);
  m->mp_name = 0;
  ckfree((char *)m);
  return;
}

// =================================== addrMapCreateEntry =========================== 

static struct Addr_Map_Entry *addrMapCreateEntry(Dwarf_Unsigned k, char *name) {
  struct Addr_Map_Entry *mp = (struct Addr_Map_Entry *)ckalloc(sizeof(struct Addr_Map_Entry));
  if (!mp) {
    return 0;
  }
  mp->mp_key = k;
  if (name) {
    mp->mp_name = (char *)strdup(name);
  } else {
      mp->mp_name = 0;
  }
  return mp;
}

// =================================== addrMapFind =========================== 

struct Addr_Map_Entry *addrMapFind(Dwarf_Unsigned addr, void **tree1) {
  void *retval = 0;
  struct Addr_Map_Entry *re = 0;
  struct Addr_Map_Entry *e = 0;

  e = addrMapCreateEntry(addr,NULL);
  retval = dwarf_tfind(e,tree1, addrMapCompareFunc);
  if (retval) {
    re = *(struct Addr_Map_Entry **)retval;
  }
  /*  The one we created here must be deleted, it is dead.
      We look at the returned one instead. */
  addrMapFreeFunc(e);
  return re;
}

// =================================== addFrameRegCol =========================== 

static uint8_t addFrameRegCol(dwarfDbgPtr_t self, Dwarf_Signed cieIdx, size_t cieFdeIdx, size_t fdeIdx, Dwarf_Addr pc, Dwarf_Signed offset, Dwarf_Signed reg, size_t *frcIdx) {
  // Fde == fde!
  uint8_t result;
  frameInfo_t *frameInfo;
  cieFde_t *cieFde;
  frameDataEntry_t *fde;
  frameRegCol_t *frameRegCol;

  result = DWARF_DBG_ERR_OK;
  frameInfo = &self->dwarfDbgFrameInfo->frameInfo;
  fde = &frameInfo->cieFdes[cieFdeIdx].frameDataEntries[fdeIdx];
  if (fde->maxFrameRegCol <= fde->numFrameRegCol) {
    fde->maxFrameRegCol += 5;
    if (fde->frameRegCols == NULL) {
      fde->frameRegCols = (frameRegCol_t *)ckalloc(sizeof(frameRegCol_t) * fde->maxFrameRegCol);
    } else {
      fde->frameRegCols = (frameRegCol_t *)ckrealloc((char *)fde->frameRegCols, sizeof(frameRegCol_t) * fde->numFrameRegCol);
    }
    if (fde->frameRegCols == NULL) {
      return DWARF_DBG_ERR_OUT_OF_MEMORY;
    }
  }
  frameRegCol = &fde->frameRegCols[fde->numFrameRegCol];
  memset(frameRegCol, 0, sizeof(frameRegCol_t));
  frameRegCol->pc = pc;
  frameRegCol->reg = reg;
  frameRegCol->offset = offset;
//printf("frc: pc: 0x%08x reg: %d offset: %d\n", pc, reg, offset);
  *frcIdx = fde->numFrameRegCol;
  fde->numFrameRegCol++;
  return result;
}

// =================================== addFde =========================== 

static uint8_t addFde(dwarfDbgPtr_t self, Dwarf_Signed cieIdx, size_t cieFdeIdx, Dwarf_Addr lowPc, Dwarf_Unsigned funcLgth, Dwarf_Signed reg, Dwarf_Signed offset, size_t *fdeIdx) {
  // frameDataEntry == fde!
  uint8_t result;
  frameInfo_t *frameInfo;
  cieFde_t *cieFde;
  frameDataEntry_t *frameDataEntry;

  result = DWARF_DBG_ERR_OK;
  frameInfo = &self->dwarfDbgFrameInfo->frameInfo;
  cieFde = &frameInfo->cieFdes[cieFdeIdx];
  if (cieFde->maxFde <= cieFde->numFde) {
    cieFde->maxFde += 5;
    if (cieFde->frameDataEntries == NULL) {
      cieFde->frameDataEntries = (frameDataEntry_t *)ckalloc(sizeof(frameDataEntry_t) * cieFde->maxFde);
    } else {
      cieFde->frameDataEntries = (frameDataEntry_t *)ckrealloc((char *)cieFde->frameDataEntries, sizeof(frameDataEntry_t) * cieFde->maxFde);
    }
    if (cieFde->frameDataEntries == NULL) {
      return DWARF_DBG_ERR_OUT_OF_MEMORY;
    }
  }
  frameInfo->numFde++;
  frameDataEntry = &cieFde->frameDataEntries[cieFde->numFde];
  memset(frameDataEntry, 0, sizeof(frameDataEntry_t));
  frameDataEntry->lowPc = lowPc;
  frameDataEntry->funcLgth = funcLgth;
  frameDataEntry->reg = reg;
  frameDataEntry->offset = offset;
//printf("addFde: %d cieFdeIdx: %d fdeIdx: %d pc: 0x%08x funcLgth: %d pc+funcLgth: 0x%08x reg: %d offset: %d\n", frameInfo->numFde, cieFdeIdx, cieFde->numFde, lowPc, funcLgth, lowPc+funcLgth, reg, offset);
  *fdeIdx = cieFde->numFde;
  cieFde->numFde++;
  return result;
}

// =================================== addCieFde =========================== 

static uint8_t addCieFde(dwarfDbgPtr_t self, Dwarf_Signed cieIdx, Dwarf_Addr pc, Dwarf_Unsigned funcLgth, size_t *cieFdeIdx) {
  // Fde == fde!
  uint8_t result;
  int idx = 0;
  frameInfo_t *frameInfo;
  cieFde_t *cieFde;

  result = DWARF_DBG_ERR_OK;
  frameInfo = &self->dwarfDbgFrameInfo->frameInfo;
  if (frameInfo->maxCieFde <= frameInfo->numCieFde) {
    frameInfo->maxCieFde += 5;
    if (frameInfo->cieFdes == NULL) {
      frameInfo->cieFdes = (cieFde_t *)ckalloc(sizeof(cieFde_t) * frameInfo->maxCieFde);
    } else {
      frameInfo->cieFdes = (cieFde_t *)ckrealloc((char *)frameInfo->cieFdes, sizeof(cieFde_t) * frameInfo->maxCieFde);
    }
    if (frameInfo->cieFdes == NULL) {
      return DWARF_DBG_ERR_OUT_OF_MEMORY;
    }
  }
  for (idx = 0; idx < frameInfo->numCieFde; idx++) {
    cieFde = &frameInfo->cieFdes[idx];
    if (cieFde->cieIdx == cieIdx) {
//printf("addCieFd: 1 cieIdx: %d cieFdeIdx: %d\n", cieIdx, idx);
      *cieFdeIdx = idx;
      return DWARF_DBG_ERR_OK;
    }
  }
  cieFde = &frameInfo->cieFdes[frameInfo->numCieFde];
  memset(cieFde, 0, sizeof(cieFde_t));
  cieFde->cieIdx = cieIdx;
//printf("addCieFd: 2 cieIdx: %d cieFdeIdx: %d\n", cieIdx, frameInfo->numCieFde);
  *cieFdeIdx = frameInfo->numCieFde;
  frameInfo->numCieFde++;
  return result;
}

// =================================== getFrameList =========================== 

static uint8_t getFrameList(dwarfDbgPtr_t self) {
  uint8_t result;
  int fres = 0;
  Dwarf_Error err = NULL;
  Dwarf_Cie *cie_data = NULL;
  Dwarf_Signed cie_element_count = 0;
  Dwarf_Fde *fde_data = NULL;
  Dwarf_Signed fde_element_count = 0;
  Dwarf_Half address_size = 0;
  Dwarf_Half offset_size = 0;
  Dwarf_Fde fde;
  int i = 0;

  result = DWARF_DBG_ERR_OK;
  fres = dwarf_get_fde_list(self->elfInfo.dbg, &cie_data, &cie_element_count, &fde_data, &fde_element_count, &err);
  if (fres != DW_DLV_OK) {
    return DWARF_DBG_ERR_CANNOT_GET_FDE_LIST;
  }
DWARF_DBG_PRINT(self, "f", 1, "getFrameList: cie_element_count: %d fde_element_count: %d\n", cie_element_count, fde_element_count);
  fres = dwarf_get_address_size(self->elfInfo.dbg, &address_size, &err);
  for (i = 0; i < fde_element_count; i++) {
    int j = 0;
    Dwarf_Ptr fde_bytes = NULL;
    Dwarf_Unsigned fde_bytes_length = 0;
    Dwarf_Off cieOffset = 0;
    Dwarf_Off fde_offset = 0;
    Dwarf_Addr lowPc = 0;
    Dwarf_Unsigned funcLgth = 0;
    Dwarf_Signed cieIdx = 0;
    struct Addr_Map_Entry *mp = 0;
    void *lowpcSet = NULL;
    size_t cieFdeIdx;

    fde = fde_data[i];
    fres = dwarf_get_fde_range(fde, &lowPc, &funcLgth, &fde_bytes, &fde_bytes_length, &cieOffset, &cieIdx, &fde_offset, &err);
    if (fres != DW_DLV_OK) {
      return DWARF_DBG_ERR_CANNOT_GET_FDE_RANGE;
    }
//    mp = addrMapFind(low_pc, &lowpcSet);
//printf("mp: %p low_pc: 0x%08x lowpcSet: %p\n", mp, low_pc, lowpcSet);
//printf("i: %d low_pc: 0x%08x func_length: %d pc+fl: 0x%08x cie_offset: 0x%08x cie_index: %d fde_offset: 0x%08x\n", i, lowPc, funcLgth, lowPc+funcLgth, cieOffset, cieIdx, fde_offset);
    self->dwarfDbgFrameInfo->addCieFde(self, cieIdx, lowPc, funcLgth, &cieFdeIdx);
    for (j = lowPc; j < lowPc + funcLgth; j++) {
      Dwarf_Half k = 0;
      Dwarf_Addr jsave = 0;
      Dwarf_Signed reg = 0;
      Dwarf_Signed offsetRelevant = 0;
      Dwarf_Small valueType = 0;
      Dwarf_Signed offsetOrBlockLen = 0;
      Dwarf_Signed offset = 0;
      Dwarf_Ptr blockPtr = 0;
      Dwarf_Addr rowPc = 0;
      Dwarf_Bool hasMoreRows = 0;
      Dwarf_Addr subsequentPc = 0;
      int fires = 0;
      size_t fdeIdx;

      unsigned cf_table_entry_count = 100;
      unsigned cf_cfa_reg = 1436;  // DW_FRAME_CFA_COL3
      unsigned cf_initial_rule_value = 1035;  // DW_FRAME_SAME_VAL
      unsigned cf_same_val = 1035;
      unsigned cf_undefined_val = 1034;

      jsave = j;
      fires = dwarf_get_fde_info_for_cfa_reg3_b(fde, j, &valueType, &offsetRelevant, &reg,
          &offsetOrBlockLen, &blockPtr, &rowPc, &hasMoreRows, &subsequentPc, &err);
      offset = offsetOrBlockLen;
      if (fires == DW_DLV_ERROR) {
        return DWARF_DBG_ERR_GET_FDE_INFO_FOR_CFA_REG3_B;
      }
      if (fires == DW_DLV_NO_ENTRY) {
        continue;
      }
//printf("value_type1: %d offset_relevant: %d lowPc: 0x%08x funcLgth: %d reg: 0x%04x offset: %d\n", valueType, offsetRelevant, lowPc, funcLgth, reg, offset);
      if (offsetRelevant) {
        result = addFde(self, cieIdx, cieFdeIdx, jsave, funcLgth, reg, offset, &fdeIdx);
      }
      if (!hasMoreRows) {
        j = lowPc + funcLgth - 1;
      } else {
        if (subsequentPc > j) {
          /*  Loop head will increment j to make up
              for -1 here. */
          j = subsequentPc - 1;
        }
      }
      if (offsetRelevant) {
        size_t frcIdx = 0;
//printf("j2: 0x%08x value_type3: %d offset_relevant: %d reg: %d offset: %d\n", j, valueType, offsetRelevant, reg, offset);
        result = addFrameRegCol(self, cieIdx, cieFdeIdx, fdeIdx, jsave, offset, reg, &frcIdx);
        checkErrOK(result);
      }
      for (k = 0; k < cf_table_entry_count; k++) {
        Dwarf_Signed reg = 0;
        Dwarf_Signed offset_relevant = 0;
        int fires = 0;
        Dwarf_Small value_type = 0;
        Dwarf_Ptr block_ptr = 0;
        Dwarf_Signed offset_or_block_len = 0;
        Dwarf_Signed offset = 0;
        Dwarf_Addr row_pc = 0;
        size_t frcIdx = 0;

        fires = dwarf_get_fde_info_for_reg3(fde, k, jsave, &value_type, &offset_relevant, &reg,
                  &offset_or_block_len, &block_ptr, &row_pc, &err);
        offset = offset_or_block_len;
        if (fires == DW_DLV_ERROR) {
          return DWARF_DBG_ERR_GET_FDE_INFO_FOR_REG3;
        }
        if (fires == DW_DLV_NO_ENTRY) {
          continue;
        }
        if (offset_relevant) {
//printf("k: %d value_type2: %d offset_relevant: %d reg: %d offset: %d\n", k, value_type, offset_relevant, reg, offset);
//printf("k: %d value_type2: %d offset_relevant: %d reg: %d offset: %d\n", k, value_type, offset_relevant, reg, offset);
          result = addFrameRegCol(self, cieIdx, cieFdeIdx, fdeIdx, jsave, offset, reg, &frcIdx);
          checkErrOK(result);
        }
      }
    }
  }
  return result;
}

// =================================== dwarfDbgFrameInfoInit =========================== 

int dwarfDbgFrameInfoInit (dwarfDbgPtr_t self) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;

  self->dwarfDbgFrameInfo->addFrameRegCol = &addFrameRegCol;
  self->dwarfDbgFrameInfo->addFde = &addFde;
  self->dwarfDbgFrameInfo->addCieFde = &addCieFde;
  self->dwarfDbgFrameInfo->getFrameList = &getFrameList;
  return result;
}
