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

#define DWARF_DBG_FLAG_SVAL   0x1

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

//DW_FORM_sec_offset:
// DW_AT_location

//DW_FORM_exprloc:
// DW_AT_location

// =================================== addAttribute =========================== 

static uint8_t addAttribute(dwarfDbgPtr_t self, Dwarf_Half attr, Dwarf_Attribute attrIn, char **srcfiles, Dwarf_Signed cnt, size_t dieAndChildrenIdx, Dwarf_Bool isSibling, int *dieAttrIdx) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;
  DWARF_DBG_PRINT(self, "A", 1, "  >>addAttribute called:\n");
  return result;
}

// =================================== handleDW_AT_abstract_originAttr =========================== 

static uint8_t handleDW_AT_abstract_originAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo) {
  uint8_t result;
  int res= 0;
  Dwarf_Error err = NULL;

  result = DWARF_DBG_ERR_OK;
  res = dwarf_formref(attrInInfo->attrIn, &attrInInfo->dieAttr->refOffset, &err);
  if (res != DW_DLV_OK) {
    return DWARF_DBG_ERR_CANNOT_GET_FORMREF;
  }
  DWARF_DBG_PRINT(self, "A", 1, " <0x%08x>", attrInInfo->dieAttr->refOffset);
  return result;
}

// =================================== handleDW_AT_artificialAttr =========================== 

static uint8_t handleDW_AT_artificialAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;

  return result;
}

// =================================== handleDW_AT_bit_offsetAttr =========================== 

static uint8_t handleDW_AT_bit_offsetAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;

  return result;
}

// =================================== handleDW_AT_bit_sizeAttr =========================== 

static uint8_t handleDW_AT_bit_sizeAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;

  return result;
}

// =================================== handleDW_AT_byte_sizeAttr =========================== 

static uint8_t handleDW_AT_byte_sizeAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;
  DWARF_DBG_PRINT(self, "A", 1, " %d ", attrInInfo->uval);
  self->dwarfDbgCompileUnitInfo->currCompileUnit->attrValues.byteSize = attrInInfo->uval;
  attrInInfo->dieAttr->byteSize = attrInInfo->uval;
  attrInInfo->dieAttr->uval = attrInInfo->uval;
  return result;
}

// =================================== handleDW_AT_call_fileAttr =========================== 

static uint8_t handleDW_AT_call_fileAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo) {
  uint8_t result;
  char *sourceFile = NULL;
  int fileIdx = 0;

  result = DWARF_DBG_ERR_OK;
  if (attrInInfo->srcfiles != NULL) {
    if ((attrInInfo->uval > 0) && (attrInInfo->uval <= attrInInfo->cnt)) {
      sourceFile = attrInInfo->srcfiles[attrInInfo->uval-1];
      DWARF_DBG_PRINT(self, "A", 1, " %s ", attrInInfo->srcfiles[attrInInfo->uval-1]);
      result = self->dwarfDbgFileInfo->addSourceFile(self, sourceFile, &fileIdx, &attrInInfo->dieAttr->sourceFileIdx);
      checkErrOK(result);
    } else {
      return DWARF_DBG_ERR_BAD_SRCFILE_INDEX;
    }
  }
  return result;
}

// =================================== handleDW_AT_call_lineAttr =========================== 

static uint8_t handleDW_AT_call_lineAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;
  DWARF_DBG_PRINT(self, "A", 1, " %d ", attrInInfo->uval);
  attrInInfo->dieAttr->sourceLineNo = attrInInfo->uval;
  return result;
}

// =================================== handleDW_AT_comp_dirAttr =========================== 

static uint8_t handleDW_AT_comp_dirAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo) {
  uint8_t result;
  int res = 0;
  char *name;
  char buf[255];
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
  compileUnit = self->dwarfDbgCompileUnitInfo->currCompileUnit;
  DWARF_DBG_PRINT(self, "A", 1, "  %s", name);
  dieAndChildrenInfo = &compileUnit->dieAndChildrenInfos[attrInInfo->dieAndChildrenIdx];
  if (attrInInfo->isSibling) {
    dieInfo = &dieAndChildrenInfo->dieSiblings[attrInInfo->dieInfoIdx];
  } else {
    dieInfo = &dieAndChildrenInfo->dieChildren[attrInInfo->dieInfoIdx];
  }
  dieAttr = &dieInfo->dieAttrs[attrInInfo->dieAttrIdx];
  result = self->dwarfDbgDieInfo->addAttrStr(self, name, &dieAttr->attrStrIdx);
  checkErrOK(result);
  sprintf(buf, "%s/%s", name, compileUnit->shortFileName);
  result = self->dwarfDbgFileInfo->addCompileUnitFile(self, buf, &compileUnit->pathNameIdx, &compileUnit->fileInfoIdx);
  checkErrOK(result);
  return result;
}

// =================================== handleDW_AT_const_valueAttr =========================== 

static uint8_t handleDW_AT_const_valueAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;
  if (attrInInfo->flags & DWARF_DBG_FLAG_SVAL) {
    attrInInfo->dieAttr->sval = attrInInfo->sval;
    attrInInfo->dieAttr->flags |= DWARF_DBG_FLAG_SVAL;
    DWARF_DBG_PRINT(self, "A", 1, " %d", attrInInfo->sval);
  } else {
    attrInInfo->dieAttr->uval = attrInInfo->uval;
    DWARF_DBG_PRINT(self, "A", 1, " %d", attrInInfo->uval);
  }
  return result;
}

// =================================== handleDW_AT_data_member_locationAttr =========================== 

static uint8_t handleDW_AT_data_member_locationAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;
  if (attrInInfo->flags & DWARF_DBG_FLAG_SVAL) {
    DWARF_DBG_PRINT(self, "A", 1, "ERROR data_member_location2: sval: %d\n", attrInInfo->sval);
  } else {
    attrInInfo->dieAttr->uval = attrInInfo->uval;
    DWARF_DBG_PRINT(self, "A", 1, " %d", attrInInfo->dieAttr->uval);
  }
  return result;
}

// =================================== handleDW_AT_declarationAttr =========================== 

static uint8_t handleDW_AT_declarationAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;
  if (attrInInfo->dieAttr->theform == DW_FORM_flag_present) {
    DWARF_DBG_PRINT(self, "A", 1, " yes(1)");
  }
  return result;
}

// =================================== handleDW_AT_decl_fileAttr =========================== 

static uint8_t handleDW_AT_decl_fileAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo) {
  uint8_t result;
  char *sourceFile = NULL;
  int pathNameIdx = 0;
  int idx2 = 0;

  result = DWARF_DBG_ERR_OK;
  if (attrInInfo->srcfiles != NULL) {
    if ((attrInInfo->uval > 0) && (attrInInfo->uval <= attrInInfo->cnt)) {
      sourceFile = attrInInfo->srcfiles[attrInInfo->uval-1];
      DWARF_DBG_PRINT(self, "A", 1, " %s ", attrInInfo->srcfiles[attrInInfo->uval-1]);
      result = self->dwarfDbgFileInfo->addSourceFile(self, sourceFile, &pathNameIdx, &idx2);
      checkErrOK(result);
      attrInInfo->dieAttr->sourceFileIdx = pathNameIdx;
      attrInInfo->dieAttr->uval = pathNameIdx;
//printf("\ndecl_file: %s fileIdx: %d %d\n", sourceFile, pathNameIdx, attrInInfo->dieAttr->sourceFileIdx);
      self->dwarfDbgCompileUnitInfo->currCompileUnit->attrValues.pathNameIdx = pathNameIdx;
    } else {
      return DWARF_DBG_ERR_BAD_SRCFILE_INDEX;
    }
  }
  return result;
}

// =================================== handleDW_AT_decl_lineAttr =========================== 

static uint8_t handleDW_AT_decl_lineAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;
  DWARF_DBG_PRINT(self, "A", 1, " %d ", attrInInfo->uval);
  attrInInfo->dieAttr->sourceLineNo = attrInInfo->uval;
  attrInInfo->dieAttr->uval = attrInInfo->uval;
  self->dwarfDbgCompileUnitInfo->currCompileUnit->attrValues.lineNo = attrInInfo->uval;
  return result;
}

// =================================== handleDW_AT_encodingAttr =========================== 

static uint8_t handleDW_AT_encodingAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo) {
  uint8_t result;
  const char *encodingString;

  result = DWARF_DBG_ERR_OK;
  result = self->dwarfDbgStringInfo->getDW_ATE_string(self, attrInInfo->uval, &encodingString);
  DWARF_DBG_PRINT(self, "A", 1, " %s ", encodingString);
  self->dwarfDbgCompileUnitInfo->currCompileUnit->attrValues.encoding = attrInInfo->uval;
  return result;
}

// =================================== handleDW_AT_entry_pcAttr =========================== 

static uint8_t handleDW_AT_entry_pcAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo) {
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

static uint8_t handleDW_AT_externalAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;
  if (attrInInfo->dieAttr->theform == DW_FORM_flag_present) {
    DWARF_DBG_PRINT(self, "A", 1, " yes(1)");
  }
  return result;
}

// =================================== handleDW_AT_frame_baseAttr =========================== 

static uint8_t handleDW_AT_frame_baseAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;
  switch (attrInInfo->dieAttr->theform) {
  case DW_FORM_sec_offset:
    result = self->dwarfDbgLocationInfo->getLocationList(self, attrInInfo->dieAndChildrenIdx, attrInInfo->dieInfoIdx, attrInInfo->isSibling, attrInInfo->dieAttrIdx, attrInInfo->attrIn);
    checkErrOK(result);
    break;
  case DW_FORM_exprloc:
    result = self->dwarfDbgLocationInfo->handleLocationExprloc(self, attrInInfo);
    checkErrOK(result);
    break;
  default:
   return DWARF_DBG_ERR_LOCATION_BAD_FORM;
    break;
  }
  return result;
}

// =================================== handleDW_AT_GNU_all_call_sitesAttr =========================== 

static uint8_t handleDW_AT_GNU_all_call_sitesAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;
  if (attrInInfo->dieAttr->theform == DW_FORM_flag_present) {
    DWARF_DBG_PRINT(self, "A", 1, " yes(1)");
  }
  return result;
}

// =================================== handleDW_AT_GNU_all_tail_call_sitesAttr =========================== 

static uint8_t handleDW_AT_GNU_all_tail_call_sitesAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;
  if (attrInInfo->dieAttr->theform == DW_FORM_flag_present) {
    DWARF_DBG_PRINT(self, "A", 1, " yes(1)");
  }
  return result;
}

// =================================== handleDW_AT_GNU_call_site_targetAttr =========================== 

static uint8_t handleDW_AT_GNU_call_site_targetAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;
  switch (attrInInfo->dieAttr->theform) {
  case DW_FORM_sec_offset:
    result = self->dwarfDbgLocationInfo->getLocationList(self, attrInInfo->dieAndChildrenIdx, attrInInfo->dieInfoIdx, attrInInfo->isSibling, attrInInfo->dieAttrIdx, attrInInfo->attrIn);
    checkErrOK(result);
    break;
  case DW_FORM_exprloc:
    result = self->dwarfDbgLocationInfo->handleLocationExprloc(self, attrInInfo);
    checkErrOK(result);
    break;
  default:
   return DWARF_DBG_ERR_LOCATION_BAD_FORM;
    break;
  }
  return result;
}

// =================================== handleDW_AT_GNU_call_site_valueAttr =========================== 

static uint8_t handleDW_AT_GNU_call_site_valueAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;
  switch (attrInInfo->dieAttr->theform) {
  case DW_FORM_sec_offset:
    result = self->dwarfDbgLocationInfo->getLocationList(self, attrInInfo->dieAndChildrenIdx, attrInInfo->dieInfoIdx, attrInInfo->isSibling, attrInInfo->dieAttrIdx, attrInInfo->attrIn);
    checkErrOK(result);
    break;
  case DW_FORM_exprloc:
    result = self->dwarfDbgLocationInfo->handleLocationExprloc(self, attrInInfo);
    checkErrOK(result);
    break;
  default:
   return DWARF_DBG_ERR_LOCATION_BAD_FORM;
    break;
  }
  return result;
}

// =================================== handleDW_AT_high_pcAttr =========================== 

static uint8_t handleDW_AT_high_pcAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo) {
  uint8_t result;
  int res = 0;
  Dwarf_Error err = NULL;
  Dwarf_Addr addr = 0;
  Dwarf_Signed tempsd = 0;
  Dwarf_Unsigned tempud = 0;

  result = DWARF_DBG_ERR_OK;
  if (attrInInfo->theform != DW_FORM_addr &&
      attrInInfo->theform != DW_FORM_GNU_addr_index &&
      attrInInfo->theform != DW_FORM_addrx) {
    attrInInfo->dieAttr->highPc = 0;
    attrInInfo->dieAttr->flags >= DW_HIGH_PC_OFFSET_FROM_LOW_PC;
    res = dwarf_formudata(attrInInfo->attrIn, &tempud, &err);
    if (res == DW_DLV_OK) {
      DWARF_DBG_PRINT(self, "A", 1, " <offset-from-lowpc>%d", tempud);
      attrInInfo->dieAttr->unsignedLowPcOffset = tempud;
    } else {
      res = dwarf_formsdata(attrInInfo->attrIn, &tempsd, &err);
      if (res == DW_DLV_OK) {
        attrInInfo->dieAttr->signedLowPcOffset = tempsd;
      DWARF_DBG_PRINT(self, "A", 1, " <offset-from-lowpc>%d", tempsd);
      } else {
        return DWARF_DBG_ERR_CANNOT_GET_FORMDATA;
      }
    }
  } else {
    res = dwarf_formaddr(attrInInfo->attrIn, &addr, &err);
    if (res != DW_DLV_OK) {
      DWARF_DBG_PRINT(self, "A", 1, " res: %d uval: %d \n", res, attrInInfo->uval);
      return DWARF_DBG_ERR_CANNOT_GET_FORMADDR;
    }
    attrInInfo->dieAttr->highPc = addr;
    DWARF_DBG_PRINT(self, "A", 1, " 0x%08x", addr);
  }
  return result;
}

// =================================== handleDW_AT_inlineAttr =========================== 

static uint8_t handleDW_AT_inlineAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo) {
  uint8_t result;
  const char *inlStr = NULL;

  result = DWARF_DBG_ERR_OK;
  result = self->dwarfDbgStringInfo->getDW_INL_string(self, attrInInfo->dieAttr->uval, &inlStr);
  DWARF_DBG_PRINT(self, "A", 1, " %s", inlStr);
  return result;
}

// =================================== handleDW_AT_languageAttr =========================== 

static uint8_t handleDW_AT_languageAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo) {
  uint8_t result;
  const char *language = NULL;
  int res = 0;

  result = DWARF_DBG_ERR_OK;
  res = dwarf_get_LANG_name(attrInInfo->uval, &language);
  if (res != DW_DLV_OK) {
    return DWARF_DBG_ERR_CANNOT_GET_LANGUAGE_NAME;
  }
  DWARF_DBG_PRINT(self, "A", 1, " %s", language);
  
  return result;
}

// =================================== handleDW_AT_linkage_nameAttr =========================== 

static uint8_t handleDW_AT_linkage_nameAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo) {
  uint8_t result;
  int res = 0;
  Dwarf_Error err = NULL;
  char *temps = NULL;

  result = DWARF_DBG_ERR_OK;
  res = dwarf_formstring(attrInInfo->dieAttr->attrIn, &temps, &err);
  if (res != DW_DLV_OK) {
    return DWARF_DBG_ERR_CANNOT_GET_FORMSTRING;
  }
  DWARF_DBG_PRINT(self, "A", 1, " %s", temps);
  return result;
}

// =================================== handleDW_AT_locationAttr =========================== 

static uint8_t handleDW_AT_locationAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;
//printf("LOCATION: dieAndChildrenIdx: %d dieInfoIdx: %d isSibling: %d dieAttrIdx: %d\n", attrInInfo->dieAndChildrenIdx, attrInInfo->dieInfoIdx, attrInInfo->isSibling, attrInInfo->dieAttrIdx);
  switch (attrInInfo->dieAttr->theform) {
  case DW_FORM_sec_offset:
    result = self->dwarfDbgLocationInfo->getLocationList(self, attrInInfo->dieAndChildrenIdx, attrInInfo->dieInfoIdx, attrInInfo->isSibling, attrInInfo->dieAttrIdx, attrInInfo->attrIn);
    checkErrOK(result);
    break;
  case DW_FORM_exprloc:
    result = self->dwarfDbgLocationInfo->handleLocationExprloc(self, attrInInfo);
    checkErrOK(result);
    break;
  default:
   return DWARF_DBG_ERR_LOCATION_BAD_FORM;
    break;
  }
  return result;
}

// =================================== handleDW_AT_low_pcAttr =========================== 

static uint8_t handleDW_AT_low_pcAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo) {
  uint8_t result;
  int res = 0;
  Dwarf_Error err = NULL;
  Dwarf_Addr addr = 0;

  result = DWARF_DBG_ERR_OK;
  res = dwarf_formaddr(attrInInfo->attrIn, &addr, &err);
  if (res != DW_DLV_OK) {
    return DWARF_DBG_ERR_CANNOT_GET_FORMADDR;
  }
  attrInInfo->dieAttr->lowPc = addr;
  DWARF_DBG_PRINT(self, "A", 1, " 0x%08x", attrInInfo->dieAttr->lowPc);
  return result;
}

// =================================== handleDW_AT_nameAttr =========================== 

static uint8_t handleDW_AT_nameAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo) {
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
  DWARF_DBG_PRINT(self, "A", 1, " %s", name);
  self->dwarfDbgCompileUnitInfo->currCompileUnit->attrValues.name = strdup(name);
  compileUnit = self->dwarfDbgCompileUnitInfo->currCompileUnit;
  dieAndChildrenInfo = &compileUnit->dieAndChildrenInfos[attrInInfo->dieAndChildrenIdx];
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

static uint8_t handleDW_AT_producerAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo) {
  uint8_t result;
  Dwarf_Error err;
  Dwarf_Attribute producerAttr;
  int ares;
  char *producerName;

  result = DWARF_DBG_ERR_OK;
  ares = dwarf_attr(attrInInfo->die, DW_AT_producer, &producerAttr, &err);
  ares = dwarf_formstring(producerAttr, &producerName, &err);
  DWARF_DBG_PRINT(self, "A", 1, " %s", producerName);
  return result;
}

// =================================== handleDW_AT_prototypedAttr =========================== 

static uint8_t handleDW_AT_prototypedAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;
  if (attrInInfo->dieAttr->theform == DW_FORM_flag_present) {
    DWARF_DBG_PRINT(self, "A", 1, " yes(1)");
  }
  return result;
}

// =================================== handleDW_AT_rangesAttr =========================== 

static uint8_t handleDW_AT_rangesAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;
  result = self->dwarfDbgRangeInfo->handleRangeInfos(self, attrInInfo->attrIn);
  return result;
}

// =================================== handleDW_AT_siblingAttr =========================== 

static uint8_t handleDW_AT_siblingAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo) {
  uint8_t result;
  int res= 0;
  Dwarf_Error err = NULL;

  result = DWARF_DBG_ERR_OK;
  res = dwarf_formref(attrInInfo->attrIn, &attrInInfo->dieAttr->refOffset, &err);
  if (res != DW_DLV_OK) {
    return DWARF_DBG_ERR_CANNOT_GET_FORMREF;
  }
  DWARF_DBG_PRINT(self, "A", 1, " 0x%08x", attrInInfo->dieAttr->refOffset);
  return result;
}

// =================================== handleDW_AT_stmt_listAttr =========================== 

static uint8_t handleDW_AT_stmt_listAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;
  if (attrInInfo->flags & DWARF_DBG_FLAG_SVAL) {
    DWARF_DBG_PRINT(self, "A", 1, "stmt_list2: sval: %d\n", attrInInfo->sval);
  } else {
    DWARF_DBG_PRINT(self, "A", 1, " 0x%08x", attrInInfo->uval);
  }
  return result;
}

// =================================== handleDW_AT_typeAttr =========================== 

static uint8_t handleDW_AT_typeAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo) {
  uint8_t result;
  int res = 0;
  Dwarf_Error err = NULL;
  Dwarf_Off offset;

  result = DWARF_DBG_ERR_OK;
  res = dwarf_formref(attrInInfo->attrIn, &attrInInfo->dieAttr->refOffset, &err);
  if (res != DW_DLV_OK) {
    return DWARF_DBG_ERR_CANNOT_GET_FORMREF;
  }
  DWARF_DBG_PRINT(self, "A", 1, " 0x%08x", attrInInfo->dieAttr->refOffset);

  return result;
}

// =================================== handleDW_AT_upper_boundAttr =========================== 

static uint8_t handleDW_AT_upper_boundAttr(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;
  if (attrInInfo->flags & DWARF_DBG_FLAG_SVAL) {
    attrInInfo->dieAttr->sval = attrInInfo->sval;
    attrInInfo->dieAttr->flags |= DWARF_DBG_FLAG_SVAL;
    DWARF_DBG_PRINT(self, "A", 1, " %d", attrInInfo->sval);
  } else {
    attrInInfo->dieAttr->uval = attrInInfo->uval;
    DWARF_DBG_PRINT(self, "A", 1, " %d", attrInInfo->uval);
  }
  return result;
}


// =================================== handleAttribute =========================== 

static uint8_t handleAttribute(dwarfDbgPtr_t self, Dwarf_Die die, Dwarf_Half attr, Dwarf_Attribute attrIn, char **srcfiles, Dwarf_Signed cnt, int dieAndChildrenIdx, int dieInfoIdx, Dwarf_Bool isSibling, int *dieAttrIdx) {
  uint8_t result;
  const char *attrName;
  attrInInfo_t attrInInfo;
  int res = 0;
  Dwarf_Error err = NULL;
const char *formStr;

  result = DWARF_DBG_ERR_OK;
  if ((int)dieAndChildrenIdx < 0) {
    DWARF_DBG_PRINT(self, "A", 1, "ERROR dieAndChildrenIdx < 0\n");
  }
  memset(&attrInInfo, 0, sizeof(attrInInfo));
  attrInInfo.formClass = DW_FORM_CLASS_UNKNOWN;
  res = dwarf_whatform(attrIn, &attrInInfo.theform, &err);
  if (res != DW_DLV_OK) {
    return DWARF_DBG_ERR_CANNOT_GET_WHATFORM;
  }
  res = dwarf_whatform_direct(attrIn, &attrInInfo.directform, &err);
  if (res != DW_DLV_OK) {
    return DWARF_DBG_ERR_CANNOT_GET_WHATFORM_DIRECT;
  }
  result = self->dwarfDbgStringInfo->getDW_AT_string(self, attr, &attrName);
  checkErrOK(result);
//printf("%s dieAndChildrenIdx: %d dieInfoIdx: %d\n", attrName, dieAndChildrenIdx, dieInfoIdx);
  attrInInfo.flags = 0;
  res = dwarf_formudata(attrIn, &attrInInfo.uval, &err);
  if (res != DW_DLV_OK) {
    res = dwarf_formsdata(attrIn, &attrInInfo.sval, &err);
    if (res != DW_DLV_OK) {
      res = dwarf_global_formref(attrIn, &attrInInfo.uval, &err);
      if (res != DW_DLV_OK) {
        switch (attr) {
        case DW_AT_artificial:
        case DW_AT_const_value:
        case DW_AT_comp_dir:
        case DW_AT_declaration:
        case DW_AT_entry_pc:
        case DW_AT_external:
        case DW_AT_frame_base:
        case DW_AT_GNU_all_call_sites:
        case DW_AT_GNU_all_tail_call_sites:
        case DW_AT_GNU_call_site_target:
        case DW_AT_GNU_call_site_value:
        case DW_AT_high_pc:
        case DW_AT_location:
        case DW_AT_low_pc:
        case DW_AT_name:
        case DW_AT_producer:
        case DW_AT_prototyped:
        case DW_TAG_typedef:
        case DW_AT_linkage_name:
          break;
        default:
          DWARF_DBG_PRINT(self, "A", 1, "Attr result21a: %d attrName: %s\n", result, attrName);
          return DWARF_DBG_ERR_CANNOT_GET_GLOBAL_FORMREF;
        }
      }
    } else {
      attrInInfo.flags |= DWARF_DBG_FLAG_SVAL;
    }
  }
  res = dwarf_get_version_of_die(self->dwarfDbgCompileUnitInfo->currCompileUnit->compileUnitDie, &attrInInfo.version, &attrInInfo.offsetSize);
  if (res != DW_DLV_OK) {
    return DWARF_DBG_ERR_GET_VERSION_OF_DIE;
  }
  attrInInfo.formClass = dwarf_get_form_class(attrInInfo.version, attrInInfo.attr, attrInInfo.offsetSize, attrInInfo.theform);
  attrInInfo.compileUnit = self->dwarfDbgCompileUnitInfo->currCompileUnit;
  attrInInfo.dieAndChildrenInfo = &attrInInfo.compileUnit->dieAndChildrenInfos[dieAndChildrenIdx];
  if (isSibling) {
//printf("  >>addAttribute: isSibling: \n");
    result = self->dwarfDbgDieInfo->addDieSiblingAttr(self, dieAndChildrenIdx, dieInfoIdx, attr, attrIn, attrInInfo.uval, attrInInfo.theform, attrInInfo.directform, dieAttrIdx);
    attrInInfo.dieInfo = &attrInInfo.dieAndChildrenInfo->dieSiblings[dieInfoIdx];
    attrInInfo.dieAttr = &attrInInfo.dieInfo->dieAttrs[*dieAttrIdx];
  } else {
//printf("  >>addAttribute: isChild:\n");
    result = self->dwarfDbgDieInfo->addDieChildAttr(self, dieAndChildrenIdx, dieInfoIdx, attr, attrIn, attrInInfo.uval, attrInInfo.theform, attrInInfo.directform, dieAttrIdx);
    attrInInfo.dieInfo = &attrInInfo.dieAndChildrenInfo->dieChildren[dieInfoIdx];
    attrInInfo.dieAttr = &attrInInfo.dieInfo->dieAttrs[*dieAttrIdx];
  }
  checkErrOK(result);
//printf("  >>addAttribute2: %s 0x%04x theform: 0x%04x directform: 0x%04x uval: %d 0x%04x dieAttrIdx: %d\n", attrName, attr, attrInInfo.theform, attrInInfo.directform, attrInInfo.uval, attrInInfo.uval, *dieAttrIdx);
  attrInInfo.attr = attr;
  attrInInfo.attrIn = attrIn;
  attrInInfo.srcfiles = srcfiles;
  attrInInfo.cnt = cnt;
  attrInInfo.dieAndChildrenIdx = dieAndChildrenIdx;
  attrInInfo.dieInfoIdx = dieInfoIdx;
  attrInInfo.dieAttrIdx = *dieAttrIdx;
  attrInInfo.isSibling = isSibling;
  attrInInfo.die = die;
self->dwarfDbgStringInfo->getDW_FORM_string(self, attrInInfo.dieAttr->theform, &formStr);
//printf("FORM: %s\n", formStr);
//printf("  >>attrName: %s\n", attrName);
  switch (attr) {
  case DW_AT_abstract_origin:
    result = handleDW_AT_abstract_originAttr(self, &attrInInfo);
    checkErrOK(result);
    break;
  case DW_AT_artificial:
    result = handleDW_AT_artificialAttr(self, &attrInInfo);
    checkErrOK(result);
    break;
  case DW_AT_bit_offset:
    result = handleDW_AT_bit_offsetAttr(self, &attrInInfo);
    checkErrOK(result);
    break;
  case DW_AT_bit_size:
    result = handleDW_AT_bit_sizeAttr(self, &attrInInfo);
    checkErrOK(result);
    break;
  case DW_AT_byte_size:
    result = handleDW_AT_byte_sizeAttr(self, &attrInInfo);
    checkErrOK(result);
    break;
  case DW_AT_call_file:
    result = handleDW_AT_call_fileAttr(self, &attrInInfo);
    checkErrOK(result);
    break;
  case DW_AT_call_line:
    result = handleDW_AT_call_lineAttr(self, &attrInInfo);
    checkErrOK(result);
    break;
  case DW_AT_comp_dir:
    result = handleDW_AT_comp_dirAttr(self, &attrInInfo);
    checkErrOK(result);
    break;
  case DW_AT_const_value:
    result = handleDW_AT_const_valueAttr(self, &attrInInfo);
    checkErrOK(result);
    break;
  case DW_AT_data_member_location:
    result = handleDW_AT_data_member_locationAttr(self, &attrInInfo);
    checkErrOK(result);
    break;
  case DW_AT_declaration:
    result = handleDW_AT_declarationAttr(self, &attrInInfo);
    checkErrOK(result);
    break;
  case DW_AT_decl_file:
    result = handleDW_AT_decl_fileAttr(self, &attrInInfo);
    checkErrOK(result);
    break;
  case DW_AT_decl_line:
    result = handleDW_AT_decl_lineAttr(self, &attrInInfo);
    checkErrOK(result);
    break;
  case DW_AT_encoding:
    result = handleDW_AT_encodingAttr(self, &attrInInfo);
    checkErrOK(result);
    break;
  case DW_AT_entry_pc:
    result = handleDW_AT_entry_pcAttr(self, &attrInInfo);
    checkErrOK(result);
    break;
  case DW_AT_external:
    result = handleDW_AT_externalAttr(self, &attrInInfo);
    checkErrOK(result);
    break;
  case DW_AT_frame_base:
    result = handleDW_AT_frame_baseAttr(self, &attrInInfo);
    checkErrOK(result);
    break;
  case DW_AT_GNU_all_call_sites:
    result = handleDW_AT_GNU_all_call_sitesAttr(self, &attrInInfo);
    checkErrOK(result);
    break;
  case DW_AT_GNU_all_tail_call_sites:
    result = handleDW_AT_GNU_all_tail_call_sitesAttr(self, &attrInInfo);
    checkErrOK(result);
    break;
  case DW_AT_GNU_call_site_target:
    result = handleDW_AT_GNU_call_site_targetAttr(self, &attrInInfo);
    checkErrOK(result);
    break;
  case DW_AT_GNU_call_site_value:
    result = handleDW_AT_GNU_call_site_valueAttr(self, &attrInInfo);
    checkErrOK(result);
    break;
  case DW_AT_high_pc:
    result = handleDW_AT_high_pcAttr(self, &attrInInfo);
    checkErrOK(result);
    break;
  case DW_AT_inline:
    result = handleDW_AT_inlineAttr(self, &attrInInfo);
    checkErrOK(result);
    break;
  case DW_AT_language:
    result = handleDW_AT_languageAttr(self, &attrInInfo);
    checkErrOK(result);
    break;
  case DW_AT_linkage_name:
    result = handleDW_AT_linkage_nameAttr(self, &attrInInfo);
    checkErrOK(result);
    break;
  case DW_AT_location:
    result = handleDW_AT_locationAttr(self, &attrInInfo);
    checkErrOK(result);
    break;
  case DW_AT_low_pc:
    result = handleDW_AT_low_pcAttr(self, &attrInInfo);
    checkErrOK(result);
    break;
  case DW_AT_name:
    result = handleDW_AT_nameAttr(self, &attrInInfo);
    checkErrOK(result);
    break;
  case DW_AT_producer:
    result = handleDW_AT_producerAttr(self, &attrInInfo);
    checkErrOK(result);
    break;
  case DW_AT_prototyped:
    result = handleDW_AT_prototypedAttr(self, &attrInInfo);
    checkErrOK(result);
    break;
  case DW_AT_ranges:
    result = handleDW_AT_rangesAttr(self, &attrInInfo);
    checkErrOK(result);
    break;
  case DW_AT_sibling:
    result = handleDW_AT_siblingAttr(self, &attrInInfo);
    checkErrOK(result);
    break;
  case DW_AT_stmt_list:
    result = handleDW_AT_stmt_listAttr(self, &attrInInfo);
    checkErrOK(result);
    break;
  case DW_AT_type:
    result = handleDW_AT_typeAttr(self, &attrInInfo);
    checkErrOK(result);
    break;
  case DW_AT_upper_bound:
    result = handleDW_AT_upper_boundAttr(self, &attrInInfo);
    checkErrOK(result);
    break;
  default:
fprintf(stderr, "ERROR missing attr: %d 0x%04x in handleAttribute\n", attr, attr);
    return DWARF_DBG_ERR_MISSING_ATTR_IN_SWITCH;
  }
  DWARF_DBG_PRINT(self, "A", 1, "\n");
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
