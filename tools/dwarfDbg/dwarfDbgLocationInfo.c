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

// =================================== addLocation =========================== 

static uint8_t addLocation(dwarfDbgPtr_t self, char *dirName) {
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

static uint8_t getLocationList(dwarfDbgPtr_t self, size_t dieAndChildrenIdx, size_t dieInfoIdx, Dwarf_Bool isSibling, int attrIdx, Dwarf_Attribute attr) {
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
  compileUnit_t *compileUnit;
  dieInfo_t *dieInfo;
  dieAttr_t *dieAttr;

  result = DWARF_DBG_ERR_OK;
  compileUnit = self->dwarfDbgCompileUnitInfo->currCompileUnit;
  dieAndChildrenInfo = &compileUnit->dieAndChildrenInfo[dieAndChildrenIdx];
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
printf("attrIdx: %d numLocations: %d locationInfo: %p\n", attrIdx, noOfElements, dieAttr->locationInfo);
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
    dieAttr->locationInfo->numLocEntry = locentryCount;
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
printf("op: 0x%02x %s opd1: %d opd2: %d opd3: %d offsetforbranch: %d\n", op, opName, opd1, opd2, opd3, offsetforbranch);
    }
  }
  return result;
}

// =================================== dwarfDbgGetVarAddr =========================== 

int dwarfDbgGetVarAddr (dwarfDbgPtr_t self, char * sourceFileName, int sourceLineNo, char *varName, int pc, int fp, int *addr) {
  int result;
  int compileUnitIdx = 0;
  int cieFdeIdx = 0;
  int fdeIdx = 0;
  int frcIdx = 0;
  int lastFdeIdx = 0;
  int found = 0;
  int haveNameAttr = 0;
  int newFp = 0;
  int dieAndChildrenIdx = 0;
  int dieInfoIdx = 0;
  int dieAttrIdx = 0;
  int locEntryIdx = 0;
  char *attrStr = NULL;
  const char *fileName = NULL;
  cieFde_t *cieFde = NULL;
  frameInfo_t *frameInfo = NULL;
  frameDataEntry_t *fde = NULL;
  frameRegCol_t *frc = NULL;
  compileUnit_t *compileUnit = NULL;
  dieAndChildrenInfo_t *dieAndChildrenInfo = NULL;
  dieInfo_t *dieInfo = NULL;
  dieAttr_t *dieAttr = NULL;
  locationInfo_t *locationInfo;
  locationOp_t *locationOp;
  result = DWARF_DBG_ERR_OK;
printf("dwarfDbgGetVarAddr: %s pc: 0x%08x fp: 0x%08x\n", varName, pc, fp);
fflush(stdout);
  found = 0;
  frameInfo = &self->dwarfDbgFrameInfo->frameInfo;
  for (cieFdeIdx = 0; cieFdeIdx < frameInfo->numCieFde; cieFdeIdx++) {
    cieFde = &frameInfo->cieFdes[cieFdeIdx];
    for (fdeIdx = 0; fdeIdx < cieFde->numFde; fdeIdx++) {
      fde = &cieFde->frameDataEntries[fdeIdx];
//printf("cieFdeIdx: %d fdeIdx: %d lastFdeIdx: %d pc: 0x%08x lowPc: 0x%08x hiPc: 0x%08x\n", cieFdeIdx, fdeIdx, lastFdeIdx, pc, fde->lowPc, fde->lowPc + fde->funcLgth);
      if (fde->lowPc > pc) {
        break;
      }
      if ((fde->lowPc <= pc ) && (pc <= (fde->lowPc + fde->funcLgth)))  {
        found = 1;
        // get the RegCol here !!!
        // FIXME !! need code here
//printf ("  numFrameRegCol: %d maxFrameRegCol: %d\n", fde->numFrameRegCol, fde->maxFrameRegCol);
        for (frcIdx = 0; frcIdx < fde->numFrameRegCol; frcIdx++) {
          frc = &fde->frameRegCols[frcIdx];
//printf("   frcIdx: %d pc: 0x%08x offset: %d reg: %d\n", frcIdx, frc->pc, frc->offset, frc->reg);
          if (frc->pc > pc) {
printf("   frcIdx: %d frc >!\n", frcIdx);
          }
        }
        lastFdeIdx = fdeIdx;
      }
    }
    if (found) {
      break;
    }
  }
  if (found) {
    fde = &cieFde->frameDataEntries[lastFdeIdx];
    frc = &fde->frameRegCols[0];
printf("  pc: 0x%08x offset: %d reg: %d\n", frc->pc, frc->offset, frc->reg);
printf("addr for var %s pc: 0x%08x fp: 0x%08x found cieFdeIdx: %d fdeIdx: %d lastFdeIdx: %d\n", varName, pc, fp, cieFdeIdx, fdeIdx, lastFdeIdx);
     switch (frc->reg) {
     case DW_FRAME_REG1:
       newFp = fp + frc->offset;
printf("newFp0: 0x%08x fp: 0x%08x frc->offset: %d\n", newFp, fp, frc->offset);
       break;
     default:
fprintf(stderr, "rule for reg: %d not yet implemented\n", frc->reg);
       break;
     }
fflush(stdout);
  } else {
printf("addr for var: %s pc: 0x%08x fp: 0x%08x not found\n", varName, pc, fp);
    self->errorStr = "Cannot get addr for var\n";
    return TCL_ERROR;
  }
  // and now get the variable location!
  // first the compileUnit
  found = 0;
  for (compileUnitIdx = 0; compileUnitIdx < self->dwarfDbgCompileUnitInfo->numCompileUnit; compileUnitIdx++) {
    compileUnit = &self->dwarfDbgCompileUnitInfo->compileUnits[compileUnitIdx];
    if (strcmp(compileUnit->shortFileName, sourceFileName) == 0) {
      found = 1;
      break;
    }
  }
printf("found: %d compileUnitIdx: %d\n", found, compileUnitIdx);
  if (!found) {
    self->errorStr = "Cannot get compile unit for var\n";
    return TCL_ERROR;
  }
  found = 0;
  for (dieAndChildrenIdx = 0; dieAndChildrenIdx < compileUnit->numDieAndChildren; dieAndChildrenIdx++) {
    dieAndChildrenInfo = &compileUnit->dieAndChildrenInfo[dieAndChildrenIdx];
printf("dieAndChildrenIdx: %d children: %d siblings: %d\n", dieAndChildrenIdx, dieAndChildrenInfo->numChildren, dieAndChildrenInfo->numSiblings);
    for (dieInfoIdx = 0; dieInfoIdx < dieAndChildrenInfo->numChildren; dieInfoIdx++) {
      dieInfo = &dieAndChildrenInfo->dieChildren[dieInfoIdx];
//printf("children dieInfoIdx: %d numAttr: %d\n", dieInfoIdx, dieInfo->numAttr);
      haveNameAttr = 0;
      for (dieAttrIdx = 0; dieAttrIdx < dieInfo->numAttr; dieAttrIdx++) {
        dieAttr = &dieInfo->dieAttrs[dieAttrIdx];
printf("yyattr: 0x%04x dieAttrIdx: %d\n", dieAttr->attr, dieAttrIdx);
fflush(stdout);
        switch (dieAttr->attr) {
        case DW_AT_name:
printf("DW_AT_name1: 0x%08x dieAndChildrenIdx: %d dieInfoIdx: %d dieAttrIdx: %d attrStrIdx: %d\n", dieAttr->attr_in, dieAndChildrenIdx, dieInfoIdx, dieAttrIdx, dieAttr->attrStrIdx);
           if ((dieAttr->attrStrIdx < 0) || (dieAttr->attrStrIdx >= dieInfo->numAttr)) {
printf("ERROR bad dieAttr->attrStrIdx: %d\n", dieAttr->attrStrIdx);
           } else  {
           attrStr = self->dwarfDbgCompileUnitInfo->attrStrs[dieAttr->attrStrIdx];
printf("DW_AT_name1: %p %s\n", attrStr, attrStr);
fflush(stdout);
          if (strcmp(varName, attrStr) == 0) {
            haveNameAttr = 1;
          }
          }
          break;
        case DW_AT_decl_file:
result = self->dwarfDbgFileInfo->getFileNameFromFileIdx(self, dieAttr->sourceFileIdx, &fileName);
if (fileName != NULL) {
printf("DW_AT_decl_file1: %s\n", fileName);
} else {
printf("DW_AT_decl_file1: %p\n", fileName);
}
fflush(stdout);
          break;
        case DW_AT_decl_line:
printf("DW_AT_decl_line1: %d\n", dieAttr->sourceLineNo);
          break;
        }
        if (haveNameAttr && (dieAttr->attr == DW_AT_location)) {
          locationInfo = dieAttr->locationInfo;
if (locationInfo == NULL) {
//printf("dieAttrIdx: %d location: %p\n", dieAttrIdx, locationInfo);
} else {
//printf("dieAttrIdx: %d location: %p lopc: 0x%08x hipc: 0x%08x\n", dieAttrIdx, locationInfo, locationInfo->lopc, locationInfo->hipc);
}
          if (locationInfo != NULL) {
            found = 1;
printf("child: dieAndChildrenIdx: %d dieInfoIdx: %d dieAttrIdx: %d pc: %08x lopc: 0x%08x hipc: 0x%08x\n", dieAndChildrenIdx, dieInfoIdx, dieAttrIdx, pc, locationInfo->lopc, locationInfo->hipc);
printf("child: dieAndChildrenIdx: %d dieInfoIdx: %d dieAttrIdx: %d numLocEntry: %d\n", dieAndChildrenIdx, dieInfoIdx, dieAttrIdx, locationInfo->numLocEntry);
              for(locEntryIdx = 0; locEntryIdx < locationInfo->numLocEntry; locEntryIdx++) {
                locationOp = &locationInfo->locationOps[locEntryIdx];
printf("locEntryIdx: %d op: 0x%02x opd1: %d\n", locEntryIdx, locationOp->op, locationOp->opd1);
                switch(locationOp->op) {
                case DW_OP_fbreg:
printf("newFp1: 0x%08x opd1: %d\n", newFp, locationOp->opd1);
                  newFp = newFp + locationOp->opd1;
                  break;
                case DW_OP_lit0:
printf("  >>need code 1 for DW_OP_lit0\n");
                  break;
                case DW_OP_reg2:
printf("  >>need code 1 for DW_OP_reg2\n");
                  break;
                case DW_OP_breg1:
printf("  >>need code 1 for DW_OP_breg1\n");
                  break;
                case DW_OP_GNU_entry_value:
printf("  >>need code 1 for DW_OP_GNU_entry_value\n");
                  break;
                default:
fprintf(stderr, "missing location op1: 0x%04x for varName address calculation: %s\n", locationOp->op, varName);
                  self->errorStr = "missing location op for varName address calculation";
                  return TCL_ERROR;
                  break;
                }
                break;
              }
          break;
          }
        }
      }
      if (found) {
        break;
      }
    }
printf("children done: found: %d numSiblings: %d\n", found, dieAndChildrenInfo->numSiblings);
    if (found) {
      break;
    }
    if (!found) {
      for (dieInfoIdx = 0; dieInfoIdx < dieAndChildrenInfo->numSiblings; dieInfoIdx++) {
        dieInfo = &dieAndChildrenInfo->dieSiblings[dieInfoIdx];
//printf("siblings dieInfoIdx: %d numAttr: %d\n", dieInfoIdx, dieInfo->numAttr);
        haveNameAttr = 0;
        for (dieAttrIdx = 0; dieAttrIdx < dieInfo->numAttr; dieAttrIdx++) {
const char *atName = NULL;
          dieAttr = &dieInfo->dieAttrs[dieAttrIdx];
//printf("DW_AT_name: 0x%08x dieAndChildrenIdx: %d dieInfoIdx: %d dieAttrIdx: %d attrStrIdx: %d flags: 0x%02x\n", dieAttr->attr_in, dieAndChildrenIdx, dieInfoIdx, dieAttrIdx, dieAttr->attrStrIdx, dieAttr->flags);
self->dwarfDbgStringInfo->getDW_AT_string(self, dieAttr->attr, &atName);
printf("idx: %d name: %s haveNameAttr: %d\n", dieAttrIdx, atName, haveNameAttr);
          switch (dieAttr->attr) {
          case DW_AT_name:
//printf("DW_AT_name: 0x%08x dieAndChildrenIdx: %d dieInfoIdx: %d dieAttrIdx: %d attrStrIdx: %d flags: 0x%04x\n", dieAttr->attr_in, dieAndChildrenIdx, dieInfoIdx, dieAttrIdx, dieAttr->attrStrIdx, dieAttr->flags);
            attrStr = self->dwarfDbgCompileUnitInfo->attrStrs[dieAttr->attrStrIdx];
printf("DW_AT_name: %s\n", attrStr);
            if (strcmp(varName, attrStr) == 0) {
printf("FND: ATname2: %s\n", attrStr);
              haveNameAttr = 1;
            }
            break;
          case DW_AT_decl_file:
result = self->dwarfDbgFileInfo->getFileNameFromFileIdx(self, dieAttr->sourceFileIdx, &fileName);
if (fileName != NULL) {
printf("DW_AT_decl_file2: %s\n", fileName);
} else {
printf("DW_AT_decl_file2: %p\n", fileName);
}
fflush(stdout);
            break;
          case DW_AT_decl_line:
printf("DW_AT_decl_line2: %d\n", dieAttr->sourceLineNo);
            break;
          }
          if (haveNameAttr && (dieAttr->attr == DW_AT_location)) {
            locationInfo = dieAttr->locationInfo;
if (locationInfo == NULL) {
//printf("dieAttrIdx: %d location: %p\n", dieAttrIdx, locationInfo);
} else {
//printf("  dieAttrIdx: %d location: %p lopc: 0x%08x hipc: 0x%08x\n", dieAttrIdx, locationInfo, locationInfo->lopc, locationInfo->hipc);
}
            if (locationInfo != NULL) {
              found = 1;
printf("sibling: dieAndChildrenIdx: %d dieInfoIdx: %d dieAttrIdx: %d numLocEntry: %d\n", dieAndChildrenIdx, dieInfoIdx, dieAttrIdx, locationInfo->numLocEntry);
              for(locEntryIdx = 0; locEntryIdx < locationInfo->numLocEntry; locEntryIdx++) {
                locationOp = &locationInfo->locationOps[locEntryIdx];
printf("locEntryIdx: %d op: 0x%02x opd1: %d\n", locEntryIdx, locationOp->op, locationOp->opd1);
                switch(locationOp->op) {
                case DW_OP_fbreg:
printf("newFp1: 0x%08x opd1: %d\n", newFp, locationOp->opd1);
                  newFp = newFp + locationOp->opd1;
                  break;
                case DW_OP_lit0:
printf("  >>need code 2 for DW_OP_lit0\n");
                  break;
                case DW_OP_reg2:
printf("  >>need code 2 for DW_OP_reg2\n");
                  break;
                case DW_OP_breg1:
printf("  >>need code 2 for DW_OP_breg1\n");
                  break;
                case DW_OP_GNU_entry_value:
printf("  >>need code 2 for DW_OP_GNU_entry_value\n");
                  break;
                default:
fprintf(stderr, "missing location op2: 0x%04x for varName address calculation: %s\n", locationOp->op, varName);
                  self->errorStr = "missing location op for varName address calculation";
                  return TCL_ERROR;
                  break;
                }
                break;
              }
              break;
            }
          }
        }
        if (found) {
          break;
        }
      }
    }
  }
  if (!found) {
    self->errorStr = "varName location not found";
    return TCL_ERROR;
  }
printf("++++newFp: varName: %s addr: 0x%08x fp: 0x%08x\n", varName, newFp, fp);
  *addr = newFp;
  return TCL_OK;
}

// =================================== dwarfDbgLocationInfoInit =========================== 

int dwarfDbgLocationInfoInit (dwarfDbgPtr_t self) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;
  self->dwarfDbgLocationInfo->getLocationList = &getLocationList;
  return result;
}
