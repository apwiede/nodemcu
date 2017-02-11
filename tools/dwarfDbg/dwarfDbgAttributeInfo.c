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
 * File:   dwarfDbgAttributeInfo.c
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

typedef struct attrInInfo {
  Dwarf_Half attr;
  Dwarf_Attribute attrIn;
  char **srcfiles;
  Dwarf_Signed cnt;
  int dieAndChildrenIdx;
  int dieInfoIdx;
  int dieAttrIdx;
  Dwarf_Die die;
  Dwarf_Bool isSibling;
  Dwarf_Unsigned uval;
  Dwarf_Half theform;
  Dwarf_Half directform;
} attrInInfo_t;

//DW_FORM_data1:
//  DW_AT_bit_offset
//  DW_AT_bit_size
//  DW_AT_byte_size
//  DW_AT_call_file
//  DW_AT_call_line
//  DW_AT_const_value
//  DW_AT_data_member_location
//  DW_AT_decl_file
//  DW_AT_decl_line
//  DW_AT_encoding
//  DW_AT_inline
//  DW_AT_language
//  DW_AT_upper_bound
//  fres = dwarf_whatattr(attrIn, &attr2, &err);

//DW_FORM_string:
//  DW_AT_comp_dir
//  DW_AT_name
//  DW_AT_producer
//  sres = dwarf_formstring(attrIn, &temps, &err);

//DW_FORM_strp:
// DW_AT_comp_dir
// DW_AT_const_value
// DW_AT_linkage_name
// DW_AT_name
// DW_AT_producer
//  sres = dwarf_formstring(attrIn, &temps, &err);

//DW_FORM_data2:
// DW_AT_byte_size
// DW_AT_const_value
// DW_AT_data_member_location
// DW_AT_decl_line  sourceLineNo = uval;
// DW_AT_call_line  sourceLineNo = uval;
// DW_AT_language
// DW_AT_upper_bound

// =================================== addAttribute =========================== 

static uint8_t addAttribute(dwarfDbgPtr_t self, Dwarf_Half attr, Dwarf_Attribute attrIn, char **srcfiles, Dwarf_Signed cnt, size_t dieAndChildrenIdx, Dwarf_Bool isSibling, int *dieAttrIdx) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;
printf("  >>addAttribute called:\n");
  return result;
}

// =================================== handleDW_AT_abstract_originAttr =========================== 

static uint8_t handleDW_AT_abstract_originAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo, int *dieAttrIdx) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;

  return result;
}

// =================================== handleDW_AT_artificialAttr =========================== 

static uint8_t handleDW_AT_artificialAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo, int *dieAttrIdx) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;

  return result;
}

// =================================== handleDW_AT_bit_offsetAttr =========================== 

static uint8_t handleDW_AT_bit_offsetAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo, int *dieAttrIdx) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;

  return result;
}

// =================================== handleDW_AT_bit_sizeAttr =========================== 

static uint8_t handleDW_AT_bit_sizeAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo, int *dieAttrIdx) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;

  return result;
}

// =================================== handleDW_AT_byte_sizeAttr =========================== 

static uint8_t handleDW_AT_byte_sizeAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo, int *dieAttrIdx) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;

  return result;
}

// =================================== handleDW_AT_call_fileAttr =========================== 

static uint8_t handleDW_AT_call_fileAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo, int *dieAttrIdx) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;

  return result;
}

// =================================== handleDW_AT_call_lineAttr =========================== 

static uint8_t handleDW_AT_call_lineAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo, int *dieAttrIdx) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;

  return result;
}

// =================================== handleDW_AT_comp_dirAttr =========================== 

static uint8_t handleDW_AT_comp_dirAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo, int *dieAttrIdx) {
  uint8_t result;
  int res = 0;
  char *name;
  Dwarf_Error err = NULL;

  result = DWARF_DBG_ERR_OK;
  res = dwarf_formstring(attrInInfo->attrIn, &name, &err);
  if (res != DW_DLV_OK) {
    return DWARF_DBG_ERR_CANNOT_GET_NAME_FORMSTRING;
  }
printf("  >>comp_dir: %d %s\n", attrInInfo->uval, name);

  return result;
}

// =================================== handleDW_AT_const_valueAttr =========================== 

static uint8_t handleDW_AT_const_valueAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo, int *dieAttrIdx) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;

  return result;
}

// =================================== handleDW_AT_data_member_locationAttr =========================== 

static uint8_t handleDW_AT_data_member_locationAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo, int *dieAttrIdx) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;

  return result;
}

// =================================== handleDW_AT_declarationAttr =========================== 

static uint8_t handleDW_AT_declarationAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo, int *dieAttrIdx) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;

  return result;
}

// =================================== handleDW_AT_decl_fileAttr =========================== 

static uint8_t handleDW_AT_decl_fileAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo, int *dieAttrIdx) {
  uint8_t result;
  const char *sourceFile = NULL;

  result = DWARF_DBG_ERR_OK;
printf("  >>DW_AT_decl_file srcfiles: %p\n", attrInInfo->srcfiles);
  if (attrInInfo->srcfiles != NULL) {
    if ((attrInInfo->uval > 0) && (attrInInfo->uval <= attrInInfo->cnt)) {
      sourceFile = attrInInfo->srcfiles[attrInInfo->uval-1];
printf("  >>FILE: %s\n", attrInInfo->srcfiles[attrInInfo->uval-1]);
    }
  }
  return result;
}

// =================================== handleDW_AT_decl_lineAttr =========================== 

static uint8_t handleDW_AT_decl_lineAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo, int *dieAttrIdx) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;
printf("  >>DW_AT_decl_line: %d 0x%04x\n", attrInInfo->uval, attrInInfo->uval);

  return result;
}

// =================================== handleDW_AT_encodingAttr =========================== 

static uint8_t handleDW_AT_encodingAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo, int *dieAttrIdx) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;

  return result;
}

// =================================== handleDW_AT_entry_pcAttr =========================== 

static uint8_t handleDW_AT_entry_pcAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo, int *dieAttrIdx) {
  uint8_t result;
  int res = 0;
  Dwarf_Error err = NULL;
  Dwarf_Addr addr = 0;

  result = DWARF_DBG_ERR_OK;
  res = dwarf_formaddr(attrInInfo->attrIn, &addr, &err);
  if (res != DW_DLV_OK) {
    return DWARF_DBG_ERR_CANNOT_GET_FORMADDR;
  }
  return result;
}

// =================================== handleDW_AT_externalAttr =========================== 

static uint8_t handleDW_AT_externalAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo, int *dieAttrIdx) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;

  return result;
}

// =================================== handleDW_AT_frame_baseAttr =========================== 

static uint8_t handleDW_AT_frame_baseAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo, int *dieAttrIdx) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;

  return result;
}

// =================================== handleDW_AT_GNU_all_call_sitesAttr =========================== 

static uint8_t handleDW_AT_GNU_all_call_sitesAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo, int *dieAttrIdx) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;

  return result;
}

// =================================== handleDW_AT_GNU_all_tail_call_sitesAttr =========================== 

static uint8_t handleDW_AT_GNU_all_tail_call_sitesAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo, int *dieAttrIdx) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;

  return result;
}

// =================================== handleDW_AT_GNU_call_site_targetAttr =========================== 

static uint8_t handleDW_AT_GNU_call_site_targetAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo, int *dieAttrIdx) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;

  return result;
}

// =================================== handleDW_AT_GNU_call_site_valueAttr =========================== 

static uint8_t handleDW_AT_GNU_call_site_valueAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo, int *dieAttrIdx) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;

  return result;
}

// =================================== handleDW_AT_high_pcAttr =========================== 

static uint8_t handleDW_AT_high_pcAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo, int *dieAttrIdx) {
  uint8_t result;
  int res = 0;
  Dwarf_Error err = NULL;
  Dwarf_Addr addr = 0;

  result = DWARF_DBG_ERR_OK;
  res = dwarf_formaddr(attrInInfo->attrIn, &addr, &err);
  if (res != DW_DLV_OK) {
    return DWARF_DBG_ERR_CANNOT_GET_FORMADDR;
  }
  return result;
}

// =================================== handleDW_AT_inlineAttr =========================== 

static uint8_t handleDW_AT_inlineAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo, int *dieAttrIdx) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;

  return result;
}

// =================================== handleDW_AT_languageAttr =========================== 

static uint8_t handleDW_AT_languageAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo, int *dieAttrIdx) {
  uint8_t result;
  const char *language = NULL;
  int res = 0;

  result = DWARF_DBG_ERR_OK;
  res = dwarf_get_LANG_name(attrInInfo->uval, &language);
  if (res != DW_DLV_OK) {
    return DWARF_DBG_ERR_CANNOT_GET_LANGUAGE_NAME;
  }
printf("  >>language: 0x%04x %d %s\n", attrInInfo->uval, attrInInfo->uval, language);
  
  return result;
}

// =================================== handleDW_AT_linkage_nameAttr =========================== 

static uint8_t handleDW_AT_linkage_nameAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo, int *dieAttrIdx) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;

  return result;
}

// =================================== handleDW_AT_locationAttr =========================== 

static uint8_t handleDW_AT_locationAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo, int *dieAttrIdx) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;
printf("LOCATION: dieAndChildrenIdx: %d dieInfoIdx: %d isSibling: %d dieAttrIdx: %d\n", attrInInfo->dieAndChildrenIdx, attrInInfo->dieInfoIdx, attrInInfo->isSibling, attrInInfo->dieAttrIdx);
  result = self->dwarfDbgLocationInfo->getLocationList(self, attrInInfo->dieAndChildrenIdx, attrInInfo->dieInfoIdx, attrInInfo->isSibling, attrInInfo->dieAttrIdx, attrInInfo->attrIn);
  checkErrOK(result);
  return result;
}

// =================================== handleDW_AT_low_pcAttr =========================== 

static uint8_t handleDW_AT_low_pcAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo, int *dieAttrIdx) {
  uint8_t result;
  int res = 0;
  Dwarf_Error err = NULL;
  Dwarf_Addr addr = 0;

  result = DWARF_DBG_ERR_OK;
  res = dwarf_formaddr(attrInInfo->attrIn, &addr, &err);
  if (res != DW_DLV_OK) {
    return DWARF_DBG_ERR_CANNOT_GET_FORMADDR;
  }
  return result;
}

// =================================== handleDW_AT_nameAttr =========================== 

static uint8_t handleDW_AT_nameAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo, int *dieAttrIdx) {
  uint8_t result;
  int res = 0;
  char *name;
  Dwarf_Error err = NULL;
  compileUnit_t *compileUnit = NULL;
  dieAndChildrenInfo_t *dieAndChildrenInfo = NULL;
  dieInfo_t *dieInfo = NULL;
  dieAttr_t *dieAttr = NULL;

  result = DWARF_DBG_ERR_OK;
  res = dwarf_formstring(attrInInfo->attrIn, &name, &err);
  if (res != DW_DLV_OK) {
    return DWARF_DBG_ERR_CANNOT_GET_NAME_FORMSTRING;
  }
printf("  >>name: %d %s\n", attrInInfo->uval, name);
  compileUnit = self->dwarfDbgCompileUnitInfo->currCompileUnit;
  dieAndChildrenInfo = &compileUnit->dieAndChildrenInfo[attrInInfo->dieAndChildrenIdx];
  if (attrInInfo->isSibling) {
    dieInfo = &dieAndChildrenInfo->dieSiblings[attrInInfo->dieInfoIdx];
  } else {
    dieInfo = &dieAndChildrenInfo->dieChildren[attrInInfo->dieInfoIdx];
  }
  dieAttr = &dieInfo->dieAttrs[attrInInfo->dieAttrIdx];
  result = self->dwarfDbgDieInfo->addAttrStr(self, name, &dieAttr->attrStrIdx);
  checkErrOK(result);
  return result;
}

// =================================== handleDW_AT_producerAttr =========================== 

static uint8_t handleDW_AT_producerAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo, int *dieAttrIdx) {
  uint8_t result;
  Dwarf_Error err;
  Dwarf_Attribute producerAttr;
  int ares;
  char *producerName;

  result = DWARF_DBG_ERR_OK;
printf("  >>producer:\n");
  ares = dwarf_attr(attrInInfo->die, DW_AT_producer, &producerAttr, &err);
  ares = dwarf_formstring(producerAttr, &producerName, &err);
printf("  >>ares: %d producerName: %s\n", ares, producerName);


  return result;
}

// =================================== handleDW_AT_prototypedAttr =========================== 

static uint8_t handleDW_AT_prototypedAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo, int *dieAttrIdx) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;

  return result;
}

// =================================== handleDW_AT_rangesAttr =========================== 

static uint8_t handleDW_AT_rangesAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo, int *dieAttrIdx) {
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
  size_t rangeIdx = 0;

  result = DWARF_DBG_ERR_OK;
  compileUnit = self->dwarfDbgCompileUnitInfo->currCompileUnit;
  fres = dwarf_global_formref(attrInInfo->attrIn, &original_off, &err);
  if (fres != DW_DLV_OK) {
    return DWARF_DBG_ERR_CANNOT_GET_GLOBA_FORMREF;
  }
  if (fres == DW_DLV_OK) {
    rres = dwarf_get_ranges_a(self->elfInfo.dbg, original_off, compileUnit->compileUnitDie, &rangeset, &rangecount, &bytecount, &err);
printf("  >>rangecount: %d bytecount: %d\n", rangecount, bytecount);
    for (i = 0; i < rangecount; i++) {
      Dwarf_Ranges *range = &rangeset[i];
      result = self->dwarfDbgFileInfo->addRangeInfo(self, range->dwr_addr1, range->dwr_addr2, range->dwr_type, &rangeIdx);
      // FIXME need to store rangeIdx somwhere!!
    }
  }
  return result;
}

// =================================== handleDW_AT_siblingAttr =========================== 

static uint8_t handleDW_AT_siblingAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo, int *dieAttrIdx) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;

  return result;
}

// =================================== handleDW_AT_stmt_listAttr =========================== 

static uint8_t handleDW_AT_stmt_listAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo, int *dieAttrIdx) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;

  return result;
}

// =================================== handleDW_AT_typeAttr =========================== 

static uint8_t handleDW_AT_typeAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo, int *dieAttrIdx) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;

  return result;
}

// =================================== handleDW_AT_upper_boundAttr =========================== 

static uint8_t handleDW_AT_upper_boundAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo, int *dieAttrIdx) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;

  return result;
}


// =================================== handleAttribute =========================== 

static uint8_t handleAttribute(dwarfDbgPtr_t self, Dwarf_Die die, Dwarf_Half attr, Dwarf_Attribute attrIn, char **srcfiles, Dwarf_Signed cnt, int dieAndChildrenIdx, int dieInfoIdx, Dwarf_Bool isSibling, int *dieAttrIdx) {
  uint8_t result;
  const char *attrName;
  attrInInfo_t attrInInfo;
  int res = 0;
  Dwarf_Half theform = 0;
  Dwarf_Half directform = 0;
  Dwarf_Attribute producerAttri = 0;
  Dwarf_Unsigned uval = 0;
  Dwarf_Error err = NULL;

  result = DWARF_DBG_ERR_OK;
  if ((int)dieAndChildrenIdx < 0) {
printf("ERROR dieAndChildrenIdx < 0\n");
  }
  res = dwarf_whatform(attrIn, &attrInInfo.theform, &err);
  if (res != DW_DLV_OK) {
printf("DWARF_DBG_ERR_CANNOT_GET_WHATFORM\n");
    return DWARF_DBG_ERR_CANNOT_GET_WHATFORM;
  }
  res = dwarf_whatform_direct(attrIn, &attrInInfo.directform, &err);
  if (res != DW_DLV_OK) {
printf("DWARF_DBG_ERR_CANNOT_GET_WHATFORM_DIRECT\n");
    return DWARF_DBG_ERR_CANNOT_GET_WHATFORM_DIRECT;
  }
  result = self->dwarfDbgStringInfo->getDW_AT_string(self, attr, &attrName);
  checkErrOK(result);
printf("  >>addAttribute1: %s dieAndChildrenIdx: %d dieInfoIdx: %d\n", attrName, dieAndChildrenIdx, dieInfoIdx);
  switch (attr) {
  case DW_AT_abstract_origin:
  case DW_AT_artificial:
  case DW_AT_comp_dir:
  case DW_AT_const_value:
  case DW_AT_declaration:
  case DW_AT_entry_pc:
  case DW_AT_external:
  case DW_AT_frame_base:
  case DW_AT_GNU_all_call_sites:
  case DW_AT_GNU_all_tail_call_sites:
  case DW_AT_GNU_call_site_target:
  case DW_AT_GNU_call_site_value:
  case DW_AT_high_pc:
  case DW_AT_linkage_name:
  case DW_AT_location:
  case DW_AT_low_pc:
  case DW_AT_name:
  case DW_AT_producer:
  case DW_AT_prototyped:
  case DW_AT_ranges:
  case DW_AT_sibling:
  case DW_AT_stmt_list:
  case DW_AT_type:
  case DW_AT_upper_bound:
    attrInInfo.uval = -1;
    break;
  default:
    res = dwarf_formudata(attrIn, &attrInInfo.uval, &err);
    if (res == DW_DLV_ERROR) {
printf("DWARF_DBG_ERR_CANNOT_GET_FORMUDATA %d %d %d\n", res, DW_DLV_ERROR, DW_DLV_NO_ENTRY);
        return DWARF_DBG_ERR_CANNOT_GET_FORMUDATA;
    }
    if (res == DW_DLV_NO_ENTRY) {
      attrInInfo.uval = -1;
    }
  }
compileUnit_t *compileUnit;
compileUnit = self->dwarfDbgCompileUnitInfo->currCompileUnit;
  if (isSibling) {
printf("  >>addAttribute: isSibling: \n");
    result = self->dwarfDbgDieInfo->addDieSiblingAttr(self, dieAndChildrenIdx, dieInfoIdx, attr, attrIn, uval, attrInInfo.theform, attrInInfo.directform, dieAttrIdx);
dieAttr_t dieAttr = compileUnit->dieAndChildrenInfo->dieSiblings->dieAttrs[*dieAttrIdx];
  } else {
printf("  >>addAttribute: isChild:\n");
    result = self->dwarfDbgDieInfo->addDieChildAttr(self, dieAndChildrenIdx, dieInfoIdx, attr, attrIn, uval, theform, directform, dieAttrIdx);
dieAttr_t dieAttr = compileUnit->dieAndChildrenInfo->dieChildren->dieAttrs[*dieAttrIdx];
  }
  checkErrOK(result);
printf("  >>addAttribute2: %s 0x%04x theform: 0x%04x directform: 0x%04x uval: %d 0x%04x dieAttrIdx: %d\n", attrName, attr, attrInInfo.theform, attrInInfo.directform, attrInInfo.uval, attrInInfo.uval, *dieAttrIdx);
  attrInInfo.attr = attr;
  attrInInfo.attrIn = attrIn;
  attrInInfo.srcfiles = srcfiles;
  attrInInfo.cnt = cnt;
  attrInInfo.dieAndChildrenIdx = dieAndChildrenIdx;
  attrInInfo.dieInfoIdx = dieInfoIdx;
  attrInInfo.dieAttrIdx = *dieAttrIdx;
  attrInInfo.isSibling = isSibling;
  attrInInfo.die = die;
printf("attrName. %s\n", attrName);
  switch (attr) {
  case DW_AT_abstract_origin:
    result = handleDW_AT_abstract_originAttr(self, &attrInInfo, dieAttrIdx);
    checkErrOK(result);
    break;
  case DW_AT_artificial:
    result = handleDW_AT_artificialAttr(self, &attrInInfo, dieAttrIdx);
    checkErrOK(result);
    break;
  case DW_AT_bit_offset:
    result = handleDW_AT_bit_offsetAttr(self, &attrInInfo, dieAttrIdx);
    checkErrOK(result);
    break;
  case DW_AT_bit_size:
    result = handleDW_AT_bit_sizeAttr(self, &attrInInfo, dieAttrIdx);
    checkErrOK(result);
    break;
  case DW_AT_byte_size:
    result = handleDW_AT_byte_sizeAttr(self, &attrInInfo, dieAttrIdx);
    checkErrOK(result);
    break;
  case DW_AT_call_file:
    result = handleDW_AT_call_fileAttr(self, &attrInInfo, dieAttrIdx);
    checkErrOK(result);
    break;
  case DW_AT_call_line:
    result = handleDW_AT_call_lineAttr(self, &attrInInfo, dieAttrIdx);
    checkErrOK(result);
    break;
  case DW_AT_comp_dir:
    result = handleDW_AT_comp_dirAttr(self, &attrInInfo, dieAttrIdx);
    checkErrOK(result);
    break;
  case DW_AT_const_value:
    result = handleDW_AT_const_valueAttr(self, &attrInInfo, dieAttrIdx);
    checkErrOK(result);
    break;
  case DW_AT_data_member_location:
    result = handleDW_AT_data_member_locationAttr(self, &attrInInfo, dieAttrIdx);
    checkErrOK(result);
    break;
  case DW_AT_declaration:
    result = handleDW_AT_declarationAttr(self, &attrInInfo, dieAttrIdx);
    checkErrOK(result);
    break;
  case DW_AT_decl_file:
    result = handleDW_AT_decl_fileAttr(self, &attrInInfo, dieAttrIdx);
    checkErrOK(result);
    break;
  case DW_AT_decl_line:
    result = handleDW_AT_decl_lineAttr(self, &attrInInfo, dieAttrIdx);
    checkErrOK(result);
    break;
  case DW_AT_encoding:
    result = handleDW_AT_encodingAttr(self, &attrInInfo, dieAttrIdx);
    checkErrOK(result);
    break;
  case DW_AT_entry_pc:
    result = handleDW_AT_entry_pcAttr(self, &attrInInfo, dieAttrIdx);
    checkErrOK(result);
    break;
  case DW_AT_external:
    result = handleDW_AT_externalAttr(self, &attrInInfo, dieAttrIdx);
    checkErrOK(result);
    break;
  case DW_AT_frame_base:
    result = handleDW_AT_frame_baseAttr(self, &attrInInfo, dieAttrIdx);
    checkErrOK(result);
    break;
  case DW_AT_GNU_all_call_sites:
    result = handleDW_AT_GNU_all_call_sitesAttr(self, &attrInInfo, dieAttrIdx);
    checkErrOK(result);
    break;
  case DW_AT_GNU_all_tail_call_sites:
    result = handleDW_AT_GNU_all_tail_call_sitesAttr(self, &attrInInfo, dieAttrIdx);
    checkErrOK(result);
    break;
  case DW_AT_GNU_call_site_target:
    result = handleDW_AT_GNU_call_site_targetAttr(self, &attrInInfo, dieAttrIdx);
    checkErrOK(result);
    break;
  case DW_AT_GNU_call_site_value:
    result = handleDW_AT_GNU_call_site_valueAttr(self, &attrInInfo, dieAttrIdx);
    checkErrOK(result);
    break;
  case DW_AT_high_pc:
    result = handleDW_AT_high_pcAttr(self, &attrInInfo, dieAttrIdx);
    checkErrOK(result);
    break;
  case DW_AT_inline:
    result = handleDW_AT_inlineAttr(self, &attrInInfo, dieAttrIdx);
    checkErrOK(result);
    break;
  case DW_AT_language:
    result = handleDW_AT_languageAttr(self, &attrInInfo, dieAttrIdx);
    checkErrOK(result);
    break;
  case DW_AT_linkage_name:
    result = handleDW_AT_linkage_nameAttr(self, &attrInInfo, dieAttrIdx);
    checkErrOK(result);
    break;
  case DW_AT_location:
    result = handleDW_AT_locationAttr(self, &attrInInfo, dieAttrIdx);
    checkErrOK(result);
    break;
  case DW_AT_low_pc:
    result = handleDW_AT_low_pcAttr(self, &attrInInfo, dieAttrIdx);
    checkErrOK(result);
    break;
  case DW_AT_name:
    result = handleDW_AT_nameAttr(self, &attrInInfo, dieAttrIdx);
    checkErrOK(result);
    break;
  case DW_AT_producer:
    result = handleDW_AT_producerAttr(self, &attrInInfo, dieAttrIdx);
    checkErrOK(result);
    break;
  case DW_AT_prototyped:
    result = handleDW_AT_prototypedAttr(self, &attrInInfo, dieAttrIdx);
    checkErrOK(result);
    break;
  case DW_AT_ranges:
    result = handleDW_AT_rangesAttr(self, &attrInInfo, dieAttrIdx);
    checkErrOK(result);
    break;
  case DW_AT_sibling:
    result = handleDW_AT_siblingAttr(self, &attrInInfo, dieAttrIdx);
    checkErrOK(result);
    break;
  case DW_AT_stmt_list:
    result = handleDW_AT_stmt_listAttr(self, &attrInInfo, dieAttrIdx);
    checkErrOK(result);
    break;
  case DW_AT_type:
    result = handleDW_AT_typeAttr(self, &attrInInfo, dieAttrIdx);
    checkErrOK(result);
    break;
  case DW_AT_upper_bound:
    result = handleDW_AT_upper_boundAttr(self, &attrInInfo, dieAttrIdx);
    checkErrOK(result);
    break;
  default:
fprintf(stderr, "missing attr: %d 0x%04x in handleAttribute\n", attr, attr);
    return DWARF_DBG_ERR_MISSING_ATTR_IN_SWITCH;
  }
  return result;
}

// =================================== dwarfDbgAttributeInfoInit =========================== 

int dwarfDbgAttributeInfoInit (dwarfDbgPtr_t self) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;
  self->dwarfDbgAttributeInfo->addAttribute = &addAttribute;
  self->dwarfDbgAttributeInfo->handleAttribute = &handleAttribute;
  return result;
}
