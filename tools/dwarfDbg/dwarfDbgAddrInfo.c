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
 * File:   dwarfDbgAddrInfo.c
 * Author: Arnulf P. Wiedemann <arnulf@wiedemann-pri.de>
 *
 * Created on February 14, 2017
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libelf.h>
#include <stdlib.h>
#include <string.h>

#include "dwarfDbgInt.h"

// =================================== handleOneExprOp =========================== 

static uint8_t handleOneExprOp(dwarfDbgPtr_t self, locationOp_t *locationOp, int index, char **stringOut) {
  /*  local_space_needed is intended to be 'more than big enough' for a short group of loclist entries.  */
  int result;
  char buf[100];
  Dwarf_Small op = 0;
  Dwarf_Unsigned opd1 = 0;
  Dwarf_Unsigned opd2 = 0;
  Dwarf_Unsigned opd3 = 0;
  Dwarf_Unsigned offsetForBranch = 0;
  const char *opName = NULL;
  Dwarf_Error err = NULL;

  if (index > 0) {
    strcat(*stringOut, " ");
  }
  op = locationOp->op;
  opd1 = locationOp->opd1;
  opd2 = locationOp->opd2;
  opd3 = locationOp->opd3;
  offsetForBranch = locationOp->offsetForBranch;
  result = self->dwarfDbgStringInfo->getDW_OP_string(self, op, &opName);
  strcat(*stringOut, opName);
  if (self->dwarfDbgLocationInfo->opHasNoOperands(op)) {
    /* Nothing to add. */
  } else {
    if (op >= DW_OP_breg0 && op <= DW_OP_breg31) {
      snprintf(buf, sizeof(buf), "%+d", (Dwarf_Signed) opd1);
      strcat(*stringOut, buf);
    } else {
//printf("opName: %s op: 0x%02x opd1: %d opd2: 0x%08x\n", opName, op, opd1, opd2);
      switch (op) {
      case DW_OP_addr:
        snprintf(buf, sizeof(buf), " <0x%08x>", (Dwarf_Unsigned) opd1);
        strcat(*stringOut, buf);
        break;
      case DW_OP_const1s:
      case DW_OP_const2s:
      case DW_OP_const4s:
      case DW_OP_const8s:
      case DW_OP_consts:
      case DW_OP_skip:
      case DW_OP_bra:
      case DW_OP_fbreg:
        strcat(*stringOut," ");
        snprintf(buf, sizeof(buf), " %+d", (Dwarf_Signed) opd1);
        strcat(*stringOut, buf);
        break;
      case DW_OP_GNU_addr_index: /* unsigned val */
      case DW_OP_addrx: /* DWARF5: unsigned val */
      case DW_OP_GNU_const_index:
      case DW_OP_constx: /* DWARF5: unsigned val */
      case DW_OP_const1u:
      case DW_OP_const2u:
      case DW_OP_const4u:
      case DW_OP_const8u:
      case DW_OP_constu:
      case DW_OP_pick:
      case DW_OP_plus_uconst:
      case DW_OP_regx:
      case DW_OP_piece:
      case DW_OP_deref_size:
      case DW_OP_xderef_size:
        snprintf(buf, sizeof(buf), " %u", opd1);
        strcat(*stringOut, buf);
        break;
      case DW_OP_bregx:
        snprintf(buf, sizeof(buf), " <0x%08x> %+d", (Dwarf_Unsigned) opd1, (Dwarf_Signed)opd2);
        strcat(*stringOut, buf);
        break;
      case DW_OP_call2:
      case DW_OP_call4:
      case DW_OP_call_ref:
        snprintf(buf, sizeof(buf), " <0x%08x>", (Dwarf_Unsigned) opd1);
        strcat(*stringOut, buf);
      case DW_OP_bit_piece:
        snprintf(buf, sizeof(buf), " <0x%08x> offset <0x%08x>", (Dwarf_Unsigned) opd1, (Dwarf_Unsigned)opd2);
        strcat(*stringOut, buf);
        break;
      case DW_OP_implicit_value:
        {
#define IMPLICIT_VALUE_PRINT_MAX 12
          unsigned int len = 0;
          snprintf(buf, sizeof(buf), " <0x%08x>", (Dwarf_Unsigned) opd1);
          strcat(*stringOut, buf);
          /*  The other operand is a block of opd1 bytes. */
          /*  FIXME */
          len = opd1;
          if (len > IMPLICIT_VALUE_PRINT_MAX) {
            len = IMPLICIT_VALUE_PRINT_MAX;
          }
#undef IMPLICIT_VALUE_PRINT_MAX
          {
            const unsigned char *bp = 0;
            /*  This is a really ugly cast, a way
                to implement DW_OP_implicit value in
                this libdwarf context. */
            bp = (const unsigned char *) opd2;
//              showContents(stringOut, len, bp);
          }
        }
        break;

      /* We do not know what the operands, if any, are. */
      case DW_OP_HP_unknown:
      case DW_OP_HP_is_value:
      case DW_OP_HP_fltconst4:
      case DW_OP_HP_fltconst8:
      case DW_OP_HP_mod_range:
      case DW_OP_HP_unmod_range:
      case DW_OP_HP_tls:
      case DW_OP_INTEL_bit_piece:
      case DW_OP_stack_value:  /* DWARF4 */
        break;
      case DW_OP_GNU_uninit:  /* DW_OP_APPLE_uninit */
        /* No operands. */
        break;
      case DW_OP_GNU_encoded_addr:
        snprintf(buf, sizeof(buf), " <0x%08x>", (Dwarf_Unsigned) opd1);
        strcat(*stringOut, buf);
        break;
      case DW_OP_implicit_pointer:       /* DWARF5 */
      case DW_OP_GNU_implicit_pointer:
        snprintf(buf, sizeof(buf), " <0x%08x> %+d", (Dwarf_Unsigned) opd1, (Dwarf_Signed)opd2);
        strcat(*stringOut, buf);
        break;
      case DW_OP_entry_value:       /* DWARF5 */
      case DW_OP_GNU_entry_value:
        {
          const unsigned char *bp = 0;
          unsigned int length = 0;

          length = opd1;
          snprintf(buf, sizeof(buf), " <0x%08x>", (Dwarf_Unsigned) opd1);
          strcat(*stringOut, buf);
          bp = (Dwarf_Small *) opd2;
          if (!bp) {
            fprintf(stderr, "ERROR: Null databyte pointer DW_OP_entry_value ");
          } else {
//              showContents(stringOut, length, bp);
          }
        }
        break;
      case DW_OP_const_type:           /* DWARF5 */
      case DW_OP_GNU_const_type:
        {
          const unsigned char *bp = 0;
          unsigned int length = 0;

          length = opd2;
          snprintf(buf, sizeof(buf), " <0x%08x> const length: %u", (Dwarf_Unsigned) opd1, length);
          strcat(*stringOut, buf);
          strcat(*stringOut," const length: ");

          /* Now point to the data bytes of the const. */
          bp = (Dwarf_Small *) opd3;
          if (!bp) {
            fprintf(stderr, "ERROR: Null databyte pointer DW_OP_const_type ");
          } else {
//              showContents(stringOut, length, bp);
          }
        }
        break;
      case DW_OP_regval_type:           /* DWARF5 */
      case DW_OP_GNU_regval_type:
        {
          snprintf(buf, sizeof(buf), " 0x%08x <0x%08x>", opd1, opd2);
          strcat(*stringOut, buf);
        }
        break;
      case DW_OP_deref_type: /* DWARF5 */
      case DW_OP_GNU_deref_type:
        {
          snprintf(buf, sizeof(buf), " 0x%02x <0x%08x>", opd1, opd2);
          strcat(*stringOut, buf);
        }
        break;
      case DW_OP_convert: /* DWARF5 */
      case DW_OP_GNU_convert:
      case DW_OP_reinterpret: /* DWARF5 */
      case DW_OP_GNU_reinterpret:
      case DW_OP_GNU_parameter_ref:
        snprintf(buf, sizeof(buf), " 0x%02x", opd1);
        strcat(*stringOut, buf);
        break;
      default:
        {
          snprintf(buf, sizeof(buf), " dwarf_op unknown 0x%x", (unsigned)op);
          strcat(*stringOut, buf);
        }
        break;
      }
    }
  }
  return DWARF_DBG_ERR_OK;
}

// =================================== handleAddrExprloc =========================== 

static uint8_t handleAddrExprloc(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo) {
  uint8_t result;
  int res = 0;
  Dwarf_Error err = NULL;
  Dwarf_Unsigned tempud = 0;
  Dwarf_Ptr x = 0;
  char extraBuf[1024];
  char *extraBufPtr = extraBuf;
  char buf[50];
  int u = 0;

  result = DWARF_DBG_ERR_OK;
  if (attrInInfo->dieAttr->theform != DW_FORM_exprloc) {
    return DWARF_DBG_ERR_BAD_FRAME_BASE_FORM;
  }
  res = dwarf_formexprloc(attrInInfo->dieAttr->attrIn, &tempud, &x, &err);
  if (res != DW_DLV_OK) {
    return DWARF_DBG_ERR_CANNOT_GET_FORM_EXPRLOC;
  }
  attrInInfo->dieAttr->uval = tempud;
  extraBuf[0] = '\0';
  buf[0] = '\0';
  snprintf(buf, sizeof(buf), " len 0x%04x: ", tempud);
  strcat(extraBuf, buf);
  for (u = 0; u < tempud; u++) {
    snprintf(buf, sizeof(buf), "%02x", *(u + (unsigned char *) x));
    strcat(extraBuf, buf);
  }
  strcat(extraBuf, ": ");
  DWARF_DBG_PRINT(self, "L", 1, "%s", extraBuf);
  extraBuf[0] = '\0';
  {
    Dwarf_Half addressSize = 0;
    Dwarf_Half offsetSize = 0;
    Dwarf_Half version = 0;
    Dwarf_Loc_Head_c head = 0;
    Dwarf_Locdesc_c locentry = 0;
    Dwarf_Unsigned lopc = 0;
    Dwarf_Unsigned hipc = 0;
    Dwarf_Unsigned ulocentryCount = 0;
    Dwarf_Unsigned sectionOffset = 0;
    Dwarf_Unsigned locdescOffset = 0;
    Dwarf_Small lleValue = 0;
    Dwarf_Small loclistSource = 0;
    Dwarf_Unsigned ulistlen = 0;
    int i = 0;

    res = dwarf_get_version_of_die(attrInInfo->compileUnit->compileUnitDie, &version, &offsetSize);
    if (res != DW_DLV_OK) {
//char * errmsg = dwarf_errmsg(err);
      return DWARF_DBG_ERR_GET_VERSION_OF_DIE;
    }
    res = dwarf_get_die_address_size(attrInInfo->compileUnit->compileUnitDie, &addressSize, &err);
    if (res != DW_DLV_OK) {
      return DWARF_DBG_ERR_GET_DIE_ADDRESS_SIZE;
    }
    res = dwarf_loclist_from_expr_c(self->elfInfo.dbg, x, tempud, addressSize, offsetSize,
            version, &head, &ulistlen, &err);
    if (res != DW_DLV_OK) {
      return DWARF_DBG_ERR_GET_LOCLIST_FROM_EXPR_C;
    }
    res = dwarf_get_locdesc_entry_c(head, 0, /* Data from 0th LocDesc */ &lleValue, &lopc, &hipc,
            &ulocentryCount, &locentry, &loclistSource, &sectionOffset, &locdescOffset, &err);
    if (res != DW_DLV_OK) {
      return DWARF_DBG_ERR_GET_LOCDESC_ENTRY_C;
    }
    // allocate all needed memory here as we know how many entries
    attrInInfo->dieAttr->locationInfo->maxLocEntry = ulocentryCount;
    attrInInfo->dieAttr->locationInfo->numLocEntry = ulocentryCount;
    attrInInfo->dieAttr->locationInfo->locationOps = (locationOp_t *)ckalloc(sizeof(locationOp_t) * ulocentryCount);
    if (attrInInfo->dieAttr->locationInfo->locationOps == NULL) {
      return DWARF_DBG_ERR_OUT_OF_MEMORY;
    }
    for (i = 0; i < ulocentryCount; i++) {
      Dwarf_Small op = 0;
      Dwarf_Unsigned opd1 = 0;
      Dwarf_Unsigned opd2 = 0;
      Dwarf_Unsigned opd3 = 0;
      Dwarf_Unsigned offsetForBranch = 0;
      locationOp_t *locationOp = NULL;
      const char *opName = 0;

      res = dwarf_get_location_op_value_c(locentry, i, &op, &opd1, &opd2, &opd3, &offsetForBranch, &err);
//printf("op: 0x%04x opd1: %d opd2: %d opd3: %d offsetForBranch: %d\n", op, opd1, opd2, opd3, offsetForBranch);
      if (res != DW_DLV_OK) {
        return DWARF_DBG_ERR_CANNOT_GET_LOCATION_OP_VALUE_C;
      }
      locationOp = &attrInInfo->dieAttr->locationInfo->locationOps[i];
      locationOp->op = op;
      locationOp->opd1 = opd1;
      locationOp->opd2 = opd2;
      locationOp->opd3 = opd3;
      locationOp->offsetForBranch = offsetForBranch;
      result = handleOneExprOp(self, locationOp, i, &extraBufPtr);
      checkErrOK(result);
      DWARF_DBG_PRINT(self, "A", 1, " %s", extraBuf);
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
  const char *tagName = NULL;
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
  DWARF_DBG_PRINT(self, "L", 1, "dwarfDbgGetVarAddr: %s pc: 0x%08x fp: 0x%08x\n", varName, pc, fp);
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
            DWARF_DBG_PRINT(self, "L", 1, "   frcIdx: %d frc >!\n", frcIdx);
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
    DWARF_DBG_PRINT(self, "L", 1, "  pc: 0x%08x offset: %d reg: %d\n", frc->pc, frc->offset, frc->reg);
    DWARF_DBG_PRINT(self, "L", 1, "addr for var %s pc: 0x%08x fp: 0x%08x found cieFdeIdx: %d fdeIdx: %d lastFdeIdx: %d\n", varName, pc, fp, cieFdeIdx, fdeIdx, lastFdeIdx);
    switch (frc->reg) {
    case DW_FRAME_REG1:
      newFp = fp + frc->offset;
      DWARF_DBG_PRINT(self, "L", 1, "newFp0: 0x%08x fp: 0x%08x frc->offset: %d\n", newFp, fp, frc->offset);
      break;
    default:
fprintf(stderr, "rule for reg: %d not yet implemented\n", frc->reg);
      break;
    }
fflush(stdout);
  } else {
    DWARF_DBG_PRINT(self, "L", 1, "addr for var: %s pc: 0x%08x fp: 0x%08x not found\n", varName, pc, fp);
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
  DWARF_DBG_PRINT(self, "L", 1, "found: %d compileUnitIdx: %d\n", found, compileUnitIdx);
  if (!found) {
    self->errorStr = "Cannot get compile unit for var\n";
    return TCL_ERROR;
  }
  found = 0;
  for (dieAndChildrenIdx = 0; dieAndChildrenIdx < compileUnit->numDieAndChildren; dieAndChildrenIdx++) {
    dieAndChildrenInfo = &compileUnit->dieAndChildrenInfos[dieAndChildrenIdx];
//    DWARF_DBG_PRINT(self, "L", 1, "dieAndChildrenIdx: %d children: %d siblings: %d\n", dieAndChildrenIdx, dieAndChildrenInfo->numChildren, dieAndChildrenInfo->numSiblings);
    for (dieInfoIdx = 0; dieInfoIdx < dieAndChildrenInfo->numChildren; dieInfoIdx++) {
      dieInfo = &dieAndChildrenInfo->dieChildren[dieInfoIdx];
      switch (dieInfo->tag) {
      case DW_TAG_formal_parameter:
      case DW_TAG_variable:
        break;
      default:
        continue;
        break;
      }
result = self->dwarfDbgStringInfo->getDW_TAG_string(self, dieInfo->tag, &tagName);
checkErrOK(result);
      DWARF_DBG_PRINT(self, "L", 1, "children dieInfoIdx: %d numAttr: %d tag: %s\n", dieInfoIdx, dieInfo->numAttr, tagName);
      haveNameAttr = 0;
      for (dieAttrIdx = 0; dieAttrIdx < dieInfo->numAttr; dieAttrIdx++) {
        dieAttr = &dieInfo->dieAttrs[dieAttrIdx];
        DWARF_DBG_PRINT(self, "L", 1, "yyattr: 0x%04x dieAttrIdx: %d\n", dieAttr->attr, dieAttrIdx);
fflush(stdout);
        switch (dieAttr->attr) {
        case DW_AT_name:
//          DWARF_DBG_PRINT(self, "L", 1, "DW_AT_name1: 0x%08x dieAndChildrenIdx: %d dieInfoIdx: %d dieAttrIdx: %d attrStrIdx: %d\n", dieAttr->attrIn, dieAndChildrenIdx, dieInfoIdx, dieAttrIdx, dieAttr->attrStrIdx);
printf("attrStrIdx: %d\n", dieAttr->attrStrIdx);
          if ((dieAttr->attrStrIdx < 0) || (dieAttr->attrStrIdx >= dieInfo->numAttr)) {
            DWARF_DBG_PRINT(self, "L", 1, "ERROR bad dieAttr->attrStrIdx: %d\n", dieAttr->attrStrIdx);
          } else  {
            attrStr = self->dwarfDbgCompileUnitInfo->attrStrs[dieAttr->attrStrIdx];
            DWARF_DBG_PRINT(self, "L", 1, "DW_AT_name1: %p %s\n", attrStr, attrStr);
fflush(stdout);
            if (strcmp(varName, attrStr) == 0) {
              haveNameAttr = 1;
            }
          }
          break;
        case DW_AT_decl_file:
result = self->dwarfDbgFileInfo->getFileNameFromPathNameIdx(self, dieAttr->sourceFileIdx, &fileName);
if (fileName != NULL) {
DWARF_DBG_PRINT(self, "L", 1, "DW_AT_decl_file1: %s\n", fileName);
} else {
DWARF_DBG_PRINT(self, "L", 1, "DW_AT_decl_file1: %p\n", fileName);
}
fflush(stdout);
          break;
        case DW_AT_decl_line:
          DWARF_DBG_PRINT(self, "L", 1, "DW_AT_decl_line1: %d\n", dieAttr->sourceLineNo);
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
            DWARF_DBG_PRINT(self, "L", 1, "child: dieAndChildrenIdx: %d dieInfoIdx: %d dieAttrIdx: %d pc: %08x lopc: 0x%08x hipc: 0x%08x\n", dieAndChildrenIdx, dieInfoIdx, dieAttrIdx, pc, locationInfo->lopc, locationInfo->hipc);
//            DWARF_DBG_PRINT(self, "L", 1, "child: dieAndChildrenIdx: %d dieInfoIdx: %d dieAttrIdx: %d numLocEntry: %d\n", dieAndChildrenIdx, dieInfoIdx, dieAttrIdx, locationInfo->numLocEntry);
              for(locEntryIdx = 0; locEntryIdx < locationInfo->numLocEntry; locEntryIdx++) {
                locationOp = &locationInfo->locationOps[locEntryIdx];
                DWARF_DBG_PRINT(self, "L", 1, "locEntryIdx: %d op: 0x%02x opd1: %d\n", locEntryIdx, locationOp->op, locationOp->opd1);
                switch(locationOp->op) {
                case DW_OP_fbreg:
                  DWARF_DBG_PRINT(self, "L", 1, "newFp1: 0x%08x opd1: %d\n", newFp, locationOp->opd1);
                  newFp = newFp + locationOp->opd1;
                  break;
                case DW_OP_lit0:
                  DWARF_DBG_PRINT(self, "L", 1, "  >>need code 1 for DW_OP_lit0\n");
                  break;
                case DW_OP_reg2:
                  DWARF_DBG_PRINT(self, "L", 1, "  >>need code 1 for DW_OP_reg2\n");
                  break;
                case DW_OP_breg1:
                  DWARF_DBG_PRINT(self, "L", 1, "  >>need code 1 for DW_OP_breg1\n");
                  break;
                case DW_OP_GNU_entry_value:
                  DWARF_DBG_PRINT(self, "L", 1, "  >>need code 1 for DW_OP_GNU_entry_value\n");
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
//    DWARF_DBG_PRINT(self, "L", 1, "children done: found: %d numSiblings: %d\n", found, dieAndChildrenInfo->numSiblings);
    if (found) {
      break;
    }
    if (!found) {
      for (dieInfoIdx = 0; dieInfoIdx < dieAndChildrenInfo->numSiblings; dieInfoIdx++) {
        dieInfo = &dieAndChildrenInfo->dieSiblings[dieInfoIdx];
//printf("siblings dieInfoIdx: %d numAttr: %d\n", dieInfoIdx, dieInfo->numAttr);
        switch (dieInfo->tag) {
        case DW_TAG_formal_parameter:
        case DW_TAG_variable:
          break;
        default:
          continue;
          break;
        }
result = self->dwarfDbgStringInfo->getDW_TAG_string(self, dieInfo->tag, &tagName);
checkErrOK(result);
        DWARF_DBG_PRINT(self, "L", 1, "siblings dieInfoIdx: %d numAttr: %d tag: %s\n", dieInfoIdx, dieInfo->numAttr, tagName);
        haveNameAttr = 0;
        for (dieAttrIdx = 0; dieAttrIdx < dieInfo->numAttr; dieAttrIdx++) {
const char *atName = NULL;
          dieAttr = &dieInfo->dieAttrs[dieAttrIdx];
//printf("DW_AT_name: 0x%08x dieAndChildrenIdx: %d dieInfoIdx: %d dieAttrIdx: %d attrStrIdx: %d flags: 0x%02x\n", dieAttr->attr_in, dieAndChildrenIdx, dieInfoIdx, dieAttrIdx, dieAttr->attrStrIdx, dieAttr->flags);
self->dwarfDbgStringInfo->getDW_AT_string(self, dieAttr->attr, &atName);
//          DWARF_DBG_PRINT(self, "L", 2, "idx: %d name: %s haveNameAttr: %d\n", dieAttrIdx, atName, haveNameAttr);
          switch (dieAttr->attr) {
          case DW_AT_name:
//printf("DW_AT_name: 0x%08x dieAndChildrenIdx: %d dieInfoIdx: %d dieAttrIdx: %d attrStrIdx: %d flags: 0x%04x\n", dieAttr->attr_in, dieAndChildrenIdx, dieInfoIdx, dieAttrIdx, dieAttr->attrStrIdx, dieAttr->flags);
            attrStr = self->dwarfDbgCompileUnitInfo->attrStrs[dieAttr->attrStrIdx];
DWARF_DBG_PRINT(self, "L", 1, "DW_AT_name: %s\n", attrStr);
            if (strcmp(varName, attrStr) == 0) {
DWARF_DBG_PRINT(self, "L", 1, "FND: ATname2: %s\n", attrStr);
              haveNameAttr = 1;
            }
            break;
          case DW_AT_decl_file:
result = self->dwarfDbgFileInfo->getFileNameFromPathNameIdx(self, dieAttr->sourceFileIdx, &fileName);
if (fileName != NULL) {
DWARF_DBG_PRINT(self, "L", 1, "DW_AT_decl_file2: %s\n", fileName);
} else {
DWARF_DBG_PRINT(self, "L", 1, "DW_AT_decl_file2: %p\n", fileName);
}
fflush(stdout);
            break;
          case DW_AT_decl_line:
            DWARF_DBG_PRINT(self, "L", 1, "DW_AT_decl_line2: %d\n", dieAttr->sourceLineNo);
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
//              DWARF_DBG_PRINT(self, "L", 1, "sibling: dieAndChildrenIdx: %d dieInfoIdx: %d dieAttrIdx: %d numLocEntry: %d\n", dieAndChildrenIdx, dieInfoIdx, dieAttrIdx, locationInfo->numLocEntry);
              for(locEntryIdx = 0; locEntryIdx < locationInfo->numLocEntry; locEntryIdx++) {
                locationOp = &locationInfo->locationOps[locEntryIdx];
                DWARF_DBG_PRINT(self, "L", 1, "locEntryIdx: %d op: 0x%02x opd1: %d\n", locEntryIdx, locationOp->op, locationOp->opd1);
                switch(locationOp->op) {
                case DW_OP_fbreg:
                  DWARF_DBG_PRINT(self, "L", 1, "newFp1: 0x%08x opd1: %d\n", newFp, locationOp->opd1);
                  newFp = newFp + locationOp->opd1;
                  break;
                case DW_OP_lit0:
                  DWARF_DBG_PRINT(self, "L", 1, "  >>need code 2 for DW_OP_lit0\n");
                  break;
                case DW_OP_reg2:
                  DWARF_DBG_PRINT(self, "L", 1, "  >>need code 2 for DW_OP_reg2\n");
                  break;
                case DW_OP_breg1:
                  DWARF_DBG_PRINT(self, "L", 1, "  >>need code 2 for DW_OP_breg1\n");
                  break;
                case DW_OP_GNU_entry_value:
                  DWARF_DBG_PRINT(self, "L", 1, "  >>need code 2 for DW_OP_GNU_entry_value\n");
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
  DWARF_DBG_PRINT(self, "L", 1, "++++newFp: varName: %s addr: 0x%08x fp: 0x%08x\n", varName, newFp, fp);
  *addr = newFp;
  return TCL_OK;
}

// =================================== dwarfDbgAddrInfoInit =========================== 

int dwarfDbgAddrInfoInit (dwarfDbgPtr_t self) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;
  self->dwarfDbgAddrInfo->handleAddrExprloc = &handleAddrExprloc;
  return result;
}
