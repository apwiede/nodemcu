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
 * File:   dwarfDbgLocationInfo.c
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

// =================================== addLocation =========================== 

static uint8_t addLocationDirName(dwarfDbgPtr_t self, char *dirName) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;
  if (self->dwarfDbgFileInfo->dirNamesInfo.maxDirName <= self->dwarfDbgFileInfo->dirNamesInfo.numDirName) {
    self->dwarfDbgFileInfo->dirNamesInfo.maxDirName += 50;
    if (self->dwarfDbgFileInfo->dirNamesInfo.dirNames == NULL) {
      self->dwarfDbgFileInfo->dirNamesInfo.dirNames = (char **)ckalloc(sizeof(char *) * self->dwarfDbgFileInfo->dirNamesInfo.maxDirName);
      if (self->dwarfDbgFileInfo->dirNamesInfo.dirNames == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      self->dwarfDbgFileInfo->dirNamesInfo.dirNames = (char **)ckrealloc((char *)self->dwarfDbgFileInfo->dirNamesInfo.dirNames, sizeof(char *) * self->dwarfDbgFileInfo->dirNamesInfo.maxDirName);
      if (self->dwarfDbgFileInfo->dirNamesInfo.dirNames == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
  self->dwarfDbgFileInfo->dirNamesInfo.dirNames[self->dwarfDbgFileInfo->dirNamesInfo.numDirName] = (char *)ckalloc(strlen(dirName) + 1);
  if (self->dwarfDbgFileInfo->dirNamesInfo.dirNames[self->dwarfDbgFileInfo->dirNamesInfo.numDirName] == NULL) {
    return DWARF_DBG_ERR_OUT_OF_MEMORY;
  }
  self->dwarfDbgFileInfo->dirNamesInfo.dirNames[self->dwarfDbgFileInfo->dirNamesInfo.numDirName][strlen(dirName)] = '\0';
  memcpy(self->dwarfDbgFileInfo->dirNamesInfo.dirNames[self->dwarfDbgFileInfo->dirNamesInfo.numDirName], dirName, strlen(dirName));
//printf("addDirName: %d %s\n", self->dwarfDbgFileInfo->dirNamesInfo.numDirName, dirName);
  self->dwarfDbgFileInfo->dirNamesInfo.numDirName++;
  return result;
}

// =================================== getLocationList =========================== 

static uint8_t getLocationList(dwarfDbgPtr_t self, size_t dieAndChildrenIdx, size_t dieInfoIdx, Dwarf_Bool isSibling, size_t attrIdx, Dwarf_Attribute attr) {
  uint8_t result;
  Dwarf_Error err = NULL;
  Dwarf_Loc_Head_c loclistHead = 0;
  Dwarf_Unsigned noOfElements = 0;
  Dwarf_Addr lopc = 0;
  Dwarf_Addr hipc = 0;
  Dwarf_Small lleValue = 0; /* DWARF5 */
  Dwarf_Small loclistSource = 0;
  Dwarf_Unsigned locentryCount = 0;
  Dwarf_Locdesc_c locentry = 0;
  Dwarf_Unsigned locdescOffset = 0;
  Dwarf_Unsigned sectionOffset = 0;
  int lres = 0;
  int llent = 0;
  int i;
  dieAndChildrenInfo_t *dieAndChildrenInfo;
  compileUnitInfo_t *compileUnitInfo;
  dieInfo_t *dieInfo;
  dieAttr_t *dieAttr;

  result = DWARF_DBG_ERR_OK;
  compileUnitInfo = &self->dwarfDbgGetDbgInfo->compileUnits[self->dwarfDbgGetDbgInfo->currCompileUnitIdx].compileUnitInfo;
  dieAndChildrenInfo = &compileUnitInfo->dieAndChildrenInfo[dieAndChildrenIdx];
  if (isSibling) {
    dieInfo = &dieAndChildrenInfo->dieSiblings[dieInfoIdx];
  } else {
    dieInfo = &dieAndChildrenInfo->dieChildren[dieInfoIdx];
  }
  dieAttr = &dieInfo->dieAttrs[attrIdx];
  lres = dwarf_get_loclist_c(attr, &loclistHead, &noOfElements, &err);
  if (lres != DW_DLV_OK) {
    return DWARF_DBG_ERR_CANNOT_GET_LOC_LIST_C;
  }
printf("numLocations: %d locationInfo: %p\n", noOfElements, dieAttr->locationInfo);
  for (llent = 0; llent < noOfElements; ++llent) {
    char small_buf[150];
    Dwarf_Unsigned locdescOffset = 0;
    Dwarf_Locdesc_c locentry = 0;
    Dwarf_Addr lopcfinal = 0;
    Dwarf_Addr hipcfinal = 0;
    Dwarf_Small op = 0;
    Dwarf_Unsigned opd1 = 0;
    Dwarf_Unsigned opd2 = 0;
    Dwarf_Unsigned opd3 = 0;
    Dwarf_Unsigned offsetforbranch = 0;
    const char *opName = NULL;
    int res = 0;
    locationOp_t *locationOp;

    lres = dwarf_get_locdesc_entry_c(loclistHead, llent, &lleValue, &lopc, &hipc, &locentryCount,
           &locentry, &loclistSource, &sectionOffset, &locdescOffset, &err);
    if (lres != DW_DLV_OK) {
      return DWARF_DBG_ERR_CANNOT_GET_LOC_DESC_ENTRY_C;
    }
printf("value: 0x%08x lopc: 0x%08x hipc: 0x%08x\n", lleValue, lopc, hipc);
    dieAttr->locationInfo->lopc = lopc;
    dieAttr->locationInfo->hipc = hipc;
    // allocate all needed memory her as we know how many entries
    dieAttr->locationInfo->maxLocEntry = locentryCount;
    dieAttr->locationInfo->locationOps = (locationOp_t *)ckalloc(sizeof(locationOp_t) * locentryCount); 
    if (dieAttr->locationInfo->locationOps == NULL) {
      return DWARF_DBG_ERR_OUT_OF_MEMORY;
    }
    for (i = 0; i < locentryCount; i++) {
      res = dwarf_get_location_op_value_c(locentry, i, &op, &opd1, &opd2, &opd3, &offsetforbranch, &err);
      if (res != DW_DLV_OK) {
        return DWARF_DBG_ERR_CANNOT_GET_LOCATION_OP_VALUE_C;
      }
      locationOp = &dieAttr->locationInfo->locationOps[i];
      locationOp->op = op;
      locationOp->opd1 = opd1;
      locationOp->opd2 = opd2;
      locationOp->opd3 = opd3;
      locationOp->offsetforbranch = offsetforbranch;
      result = self->dwarfDbgStringInfo->getDW_OP_string(self, op, &opName);
      checkErrOK(result);
printf("op: 0x%02x %s opd1: 0x%02x opd2: 0x%02x opd3: 0x%02x offsetforbranch: %d\n", op, opName, opd1, opd2, opd3, offsetforbranch);
    }
  }
  return result;
}

// =================================== dwarfDbgGetVarAddr =========================== 

int dwarfDbgGetVarAddr (dwarfDbgPtr_t self, char *varName, int pc, int fp, int *addr) {
  int result;
  dieAndChildrenInfo_t *dieAndChildrenInfo;
  compileUnitInfo_t *compileUnitInfo;
  dieInfo_t *dieInfo;
  dieAttr_t *dieAttr;
  locationInfo_t *locationInfo;
  int idx;
  int idx1;
  int idx2;
  int idx3;
  int idx4;
  int idx5;
  int idx6;
  int idx7;
  int idx8;
  int found;
  cieFde_t *cieFde;
  frameInfo_t *frameInfo;
  frameDataEntry_t *fde;
  frameRegCol_t *frc;

  result = DWARF_DBG_ERR_OK;
printf("dwarfDbgGetVarAddr: %s pc: 0x%08x fp: 0x%08x\n", varName, pc, fp);
fflush(stdout);
//  *addr = fp + 20;
  found = 0;
#ifdef NOTDEF
  for (idx = 0; idx < self->dwarfDbgGetDbgInfo->numCompileUnit; idx++) {
    compileUnitInfo = &self->dwarfDbgGetDbgInfo->compileUnits[idx].compileUnitInfo;
//printf("idx: %d\n", idx);
    for (idx2 = 0; idx2 < compileUnitInfo->numDieAndChildren; idx2++) {
      dieAndChildrenInfo = &compileUnitInfo->dieAndChildrenInfo[idx2];
//printf("idx2: %d\n", idx2);
      for (idx3 = 0; idx3 < dieAndChildrenInfo->numChildren; idx3++) {
        dieInfo = &dieAndChildrenInfo->dieChildren[idx3];
//printf("idx3: %d\n", idx3);
        for (idx5 = 0; idx5 < dieInfo->numAttr; idx5++) {
          dieAttr = &dieInfo->dieAttrs[idx5];
//printf("idx5: %d\n", idx5);
            locationInfo = dieAttr->locationInfo;
            if ((locationInfo != NULL) &&(locationInfo->lopc <= pc) && (pc <= locationInfo->hipc)) {
              found = 1;
printf("idx: %d idx2: %d idx3: %d idx5: %d idx7: %d\n", idx, idx2, idx3, idx5, idx7);
              break;
            }
        }
        if (found) {
          break;
        }
      }
printf("found1: %d\n", found);
      if (!found) {
        for (idx4 = 0; idx4 < dieAndChildrenInfo->numSiblings; idx4++) {
          dieInfo = &dieAndChildrenInfo->dieSiblings[idx4];
          for (idx6 = 0; idx6 < dieInfo->numAttr; idx6++) {
            dieAttr = &dieInfo->dieAttrs[idx6];
              locationInfo = dieAttr->locationInfo;
              if ((locationInfo != NULL) &&(locationInfo->lopc <= pc) && (pc <= locationInfo->hipc)) {
                found = 1;
printf("idx: %d idx2: %d idx4: %d idx6: %d pc: %08x lopc: 0x%08x hipc: 0x%08x\n", idx, idx2, idx4, idx6, pc, locationInfo->lopc, locationInfo->hipc);
                break;
              }
          }
          if (found) {
            break;
          }
        }
        if (found) {
          break;
        }
      }
      if (found) {
        break;
      }
    }
    if (found) {
      break;
    }
  }
printf("found: %d\n", found);
fflush(stdout);
#endif
  frameInfo = &self->dwarfDbgFrameInfo->frameInfo;
  for (idx = 0; idx < frameInfo->numCieFde; idx++) {
    cieFde = &frameInfo->cieFdes[idx];
    for (idx1 = 0; idx1 < cieFde->numFde; idx1++) {
      fde = &cieFde->frameDataEntries[idx1];
      if ((fde->lowPc <= pc ) && (pc <= (fde->lowPc + fde->funcLgth)))  {
        found = 1;
printf("idx: %d idx1: %d pc: 0x%08x lowPc: 0x%08x hiPc: 0x%08x\n", idx, idx1, pc, fde->lowPc, fde->lowPc + fde->funcLgth);
        // get the RegCol here !!!
        // FIXME !! need code here
printf ("numFrameRegCol: %d maxFrameRegCol: %d\n", fde->numFrameRegCol, fde->maxFrameRegCol);
        for (idx2 = 0; idx2 < fde->numFrameRegCol; idx2++) {
          frc = &fde->frameRegCols[idx2];
printf("pc: 0x%08x offset: %d reg: %d\n", frc->pc, frc->offset, frc->reg);
        }
        break;
      }
      if (found) {
        break;
      }
    }
    if (found) {
      break;
    }
  }
printf("found: %d\n", found);
fflush(stdout);
  return TCL_OK;
}

// =================================== dwarfDbgLocationInfoInit =========================== 

int dwarfDbgLocationInfoInit (dwarfDbgPtr_t self) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;
//  self->dwarfDbgLocationInfo->dirNamesInfo.maxDirName = 0;
//  self->dwarfDbgLocationInfo->dirNamesInfo.numDirName = 0;
//  self->dwarfDbgLocationInfo->dirNamesInfo.dirNames = NULL;

    self->dwarfDbgLocationInfo->getAddressSizeAndMax = &getAddressSizeAndMax;
    self->dwarfDbgLocationInfo->getLocationList = &getLocationList;
  return result;
}


