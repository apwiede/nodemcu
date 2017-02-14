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

typedef struct operationDescr {
    int op_code;
    int op_count;
    const char * op_1type;
} operationDescr_t;

static operationDescr_t operationDescInfos[] = {
    {DW_OP_addr,                 1, "addr" },
    {DW_OP_deref,                0, "" },
    {DW_OP_const1u,              1, "1u" },
    {DW_OP_const1s,              1, "1s" },
    {DW_OP_const2u,              1, "2u" },
    {DW_OP_const2s,              1, "2s" },
    {DW_OP_const4u,              1, "4u" },
    {DW_OP_const4s,              1, "4s" },
    {DW_OP_const8u,              1, "8u" },
    {DW_OP_const8s,              1, "8s" },
    {DW_OP_constu,               1, "uleb" },
    {DW_OP_consts,               1, "sleb" },
    {DW_OP_dup,                  0, ""},
    {DW_OP_drop,                 0, ""},
    {DW_OP_over,                 0, ""},
    {DW_OP_pick,                 1, "1u"},
    {DW_OP_swap,                 0, ""},
    {DW_OP_rot,                  0, ""},
    {DW_OP_xderef,               0, ""},
    {DW_OP_abs,                  0, ""},
    {DW_OP_and,                  0, ""},
    {DW_OP_div,                  0, ""},
    {DW_OP_minus,                0, ""},
    {DW_OP_mod,                  0, ""},
    {DW_OP_mul,                  0, ""},
    {DW_OP_neg,                  0, ""},
    {DW_OP_not,                  0, ""},
    {DW_OP_or,                   0, ""},
    {DW_OP_plus,                 0, ""},
    {DW_OP_plus_uconst,          1, "uleb"},
    {DW_OP_shl,                  0, ""},
    {DW_OP_shr,                  0, ""},
    {DW_OP_shra,                 0, ""},
    {DW_OP_xor,                  0, ""},
    {DW_OP_skip,                 1, "2s"},
    {DW_OP_bra,                  1, "2s"},
    {DW_OP_eq,                   0, ""},
    {DW_OP_ge,                   0, ""},
    {DW_OP_gt,                   0, ""},
    {DW_OP_le,                   0, ""},
    {DW_OP_lt,                   0, ""},
    {DW_OP_ne,                   0, ""},
    /* lit0 thru reg31 handled specially, no operands */
    /* breg0 thru breg31 handled specially, 1 operand */
    {DW_OP_regx,                 1, "uleb"},
    {DW_OP_fbreg,                1, "sleb"},
    {DW_OP_bregx,                2, "uleb"},
    {DW_OP_piece,                1, "uleb"},
    {DW_OP_deref_size,           1, "1u"},
    {DW_OP_xderef_size,          1, "1u"},
    {DW_OP_nop,                  0, ""},
    {DW_OP_push_object_address,  0, ""},
    {DW_OP_call2,                1, "2u"},
    {DW_OP_call4,                1, "4u"},
    {DW_OP_call_ref,             1, "off"},
    {DW_OP_form_tls_address,     0, ""},
    {DW_OP_call_frame_cfa,       0, ""},
    {DW_OP_bit_piece,            2, "uleb"},
    {DW_OP_implicit_value,       2, "u"},
    {DW_OP_stack_value,          0, ""},
    {DW_OP_GNU_uninit,           0, ""},
    {DW_OP_GNU_encoded_addr,     1, "addr"},
    {DW_OP_implicit_pointer,     2, "addr" }, /* DWARF5 */
    {DW_OP_GNU_implicit_pointer, 2, "addr" },
    {DW_OP_entry_value,          2, "val" }, /* DWARF5 */
    {DW_OP_GNU_entry_value,      2, "val" },
    {DW_OP_const_type,           3, "uleb" }, /* DWARF5 */
    {DW_OP_GNU_const_type,       3, "uleb" },
    {DW_OP_regval_type,          2, "uleb" }, /* DWARF5 */
    {DW_OP_GNU_regval_type,      2, "uleb" },
    {DW_OP_deref_type,           1, "val" }, /* DWARF5 */
    {DW_OP_GNU_deref_type,       1, "val" },
    {DW_OP_convert,              1, "uleb" }, /* DWARF5 */
    {DW_OP_GNU_convert,          1, "uleb" },
    {DW_OP_reinterpret,          1, "uleb" }, /* DWARF5 */
    {DW_OP_GNU_reinterpret,      1, "uleb" },
 
    {DW_OP_GNU_parameter_ref,    1, "val" },
    {DW_OP_GNU_const_index,      1, "val" },
    {DW_OP_GNU_push_tls_address, 0, "" },

    {DW_OP_addrx,                1, "uleb" }, /* DWARF5 */
    {DW_OP_GNU_addr_index,       1, "val" },
    {DW_OP_constx,               1, "u" }, /* DWARF5 */
    {DW_OP_GNU_const_index,      1, "u" },
    {DW_OP_GNU_parameter_ref,    1, "u" },
    {DW_OP_xderef,               0, "" }, /* DWARF5 */
    {DW_OP_xderef_type,          2, "1u" }, /* DWARF5 */
    /* terminator */
    {0,                          0, ""}
};

// =================================== opHasNoOperands =========================== 

static int opHasNoOperands(int op) {
  unsigned i = 0;

  if (op >= DW_OP_lit0 && op <= DW_OP_reg31) {
    return TRUE;
  }
  for (; ; ++i) {
    operationDescr_t *odp = &operationDescInfos[i];
    if (odp->op_code == 0) {
      break;
    }
    if (odp->op_code != op) {
      continue;
    }
    if (odp->op_count == 0) {
      return TRUE;
    }
    return FALSE;
  }
  return FALSE;
}

// =================================== showContents =========================== 

static void showContents(char **stringOut, unsigned int length, const unsigned char *bp) {   
  unsigned int i = 0;
  char buf[20];
    
  if(!length) {
    return;
  }
  strcat(*stringOut," contents 0x");
  for (; i < length; ++i,++bp) {
    /*  Do not use DW_PR_DUx here, the value  *bp is a const unsigned char. */
    snprintf(buf, sizeof(buf), "%02x", *bp);
    strcat(*stringOut, buf);
  }
}

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
  /* DWARF 2,3,4 and DWARF5 style */
  op = locationOp->op;
  opd1 = locationOp->opd1;
  opd2 = locationOp->opd2;
  opd3 = locationOp->opd3;
  offsetForBranch = locationOp->offsetForBranch;
  result = self->dwarfDbgStringInfo->getDW_OP_string(self, op, &opName);
  strcat(*stringOut, opName);
  if (opHasNoOperands(op)) {
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
            showContents(stringOut, len, bp);
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
            showContents(stringOut, length, bp);
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
            showContents(stringOut, length, bp);
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

// =================================== handleLocationExprloc =========================== 

static uint8_t handleLocationExprloc(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo) {
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
  char buf[150];

  result = DWARF_DBG_ERR_OK;
  compileUnit = self->dwarfDbgCompileUnitInfo->currCompileUnit;
  dieAndChildrenInfo = &compileUnit->dieAndChildrenInfos[dieAndChildrenIdx];
//printf("dieAndChildrenInfo: %p\n", dieAndChildrenInfo);
  if (isSibling) {
    dieInfo = &dieAndChildrenInfo->dieSiblings[dieInfoIdx];
  } else {
    dieInfo = &dieAndChildrenInfo->dieChildren[dieInfoIdx];
  }
//printf("getLocationList: dieInfo: %p\n", dieInfo);
  dieAttr = &dieInfo->dieAttrs[attrIdx];
//printf("dieAttr: %p attr: 0x%04x\n", dieAttr, attr);
  lres = dwarf_get_loclist_c(attr, &loclistHead, &noOfElements, &err);
  if (lres != DW_DLV_OK) {
    return DWARF_DBG_ERR_CANNOT_GET_LOC_LIST_C;
  }
//printf("attrIdx: %d numLocations: %d locationInfo: %p\n", attrIdx, noOfElements, dieAttr->locationInfo);
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
    Dwarf_Unsigned offsetForBranch = 0;
    const char *opName = NULL;
    char extraBuf[1024];
    char buf2[100];
    int res = 0;
    locationOp_t *locationOp;
    char *bp;

//printf("loclistHead: %p llent: %d locdescOffset: 0x%08x\n", loclistHead, llent, locdescOffset);
    extraBuf[0] = '\0';
    lres = dwarf_get_locdesc_entry_c(loclistHead, llent, &lleValue, &lopc, &hipc, &locentryCount,
           &locentry, &loclistSource, &sectionOffset, &locdescOffset, &err);
    if (lres != DW_DLV_OK) {
      return DWARF_DBG_ERR_CANNOT_GET_LOC_DESC_ENTRY_C;
    }
    if (llent == 0) {
//if ((lopc != 0) && (hipc != 0xffffffff)) 
if (noOfElements > 1) {
DWARF_DBG_PRINT(self, "L", 1, " <loclist at offset 0x%08x with %d entries follows>\n", locdescOffset, noOfElements);
}
    }
//printf("value: 0x%08x lopc: 0x%08x hipc: 0x%08x locationInfo: %p\n", lleValue, lopc, hipc, dieAttr->locationInfo);
//printf("lopc: 0x%08x hipc: 0x%08x\n", lopc, hipc);
    dieAttr->locationInfo->lopc = lopc;
    dieAttr->locationInfo->hipc = hipc;
    // allocate all needed memory here as we know how many entries
    dieAttr->locationInfo->maxLocEntry = locentryCount;
    dieAttr->locationInfo->numLocEntry = locentryCount;
    dieAttr->locationInfo->locationOps = (locationOp_t *)ckalloc(sizeof(locationOp_t) * locentryCount); 
    if (dieAttr->locationInfo->locationOps == NULL) {
      return DWARF_DBG_ERR_OUT_OF_MEMORY;
    }
    for (i = 0; i < locentryCount; i++) {
      int j;

      res = dwarf_get_location_op_value_c(locentry, i, &op, &opd1, &opd2, &opd3, &offsetForBranch, &err);
      if (res != DW_DLV_OK) {
        return DWARF_DBG_ERR_CANNOT_GET_LOCATION_OP_VALUE_C;
      }
      locationOp = &dieAttr->locationInfo->locationOps[i];
      locationOp->op = op;
      locationOp->opd1 = opd1;
      locationOp->opd2 = opd2;
      locationOp->opd3 = opd3;
      locationOp->offsetForBranch = offsetForBranch;
      result = self->dwarfDbgStringInfo->getDW_OP_string(self, op, &opName);
      checkErrOK(result);
      sprintf(buf2, " %s", opName);
      strcat(extraBuf, buf2);
      switch (op) {
      case DW_OP_GNU_entry_value:
        bp = (char *)opd2;
        sprintf(buf2, " 0x%08x contents 0x", opd1);
        strcat(extraBuf, buf2);
        for (j = 0; j < opd1; j++) {
          sprintf(buf2, "%02x", *bp);
          strcat(extraBuf, buf2);
          locationOp->contents[j] = (*bp)++;
        }
        break;
      }
//printf("op: 0x%02x %s opd1: %d opd2: 0x%08x opd3: 0x%02x\n", op, opName, opd1, opd2, opd3);
//printf("op: 0x%02x %s opd1: %d opd2: %d opd3: %d offsetForBranch: %d\n", op, opName, opd1, opd2, opd3, offsetForBranch);
    }
if ((lopc != 0) && (hipc != 0xffffffff)) {
DWARF_DBG_PRINT(self, "L", 1, "                   %*s[%2d]< offset pair low-off : 0x%08x addr  0x%08x high-off  0x%08x addr 0x%08x>%s\n", (self->dwarfDbgCompileUnitInfo->currCompileUnit->level * 2), " ", llent, lopc, lopc, hipc, hipc, extraBuf);
} else {
DWARF_DBG_PRINT(self, "L", 1, " %s", extraBuf);
}
  }
  return result;
}

// =================================== dwarfDbgLocationInfoInit =========================== 

int dwarfDbgLocationInfoInit (dwarfDbgPtr_t self) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;
  self->dwarfDbgLocationInfo->opHasNoOperands = &opHasNoOperands;
  self->dwarfDbgLocationInfo->handleLocationExprloc = &handleLocationExprloc;
  self->dwarfDbgLocationInfo->getLocationList = &getLocationList;
  return result;
}
