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
 * File:   dwarfDbgGetDbgInfo.c
 * Author: Arnulf P. Wiedemann <arnulf@wiedemann-pri.de>
 *
 * Created on January 24, 2017
 */

#include <tcl.h>
#include "dwarfDbgInt.h"

// =================================== addCompileUnit =========================== 

static uint8_t addCompileUnit(dwarfDbgPtr_t self, size_t *compileUnitIdx) {
  uint8_t result;
  compileUnit_t *compileUnit;

  result = DWARF_DBG_ERR_OK;
  if (self->dwarfDbgGetDbgInfo->maxCompileUnit <= self->dwarfDbgGetDbgInfo->numCompileUnit) {
    self->dwarfDbgGetDbgInfo->maxCompileUnit += 50;
    if (self->dwarfDbgGetDbgInfo->compileUnits == NULL) {
      self->dwarfDbgGetDbgInfo->compileUnits = (compileUnit_t *)ckalloc(sizeof(compileUnit_t) * self->dwarfDbgGetDbgInfo->maxCompileUnit);
      if (self->dwarfDbgGetDbgInfo->compileUnits == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      self->dwarfDbgGetDbgInfo->compileUnits = (compileUnit_t *)ckrealloc((char *)self->dwarfDbgGetDbgInfo->compileUnits, sizeof(compileUnit_t) * self->dwarfDbgGetDbgInfo->maxCompileUnit);
      if (self->dwarfDbgGetDbgInfo->compileUnits == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
//printf("compileUnitIdx: %d\n", self->dwarfDbgGetDbgInfo->numCompileUnit);
  compileUnit = &self->dwarfDbgGetDbgInfo->compileUnits[self->dwarfDbgGetDbgInfo->numCompileUnit];
  memset(compileUnit, 0, sizeof(compileUnit_t));
  *compileUnitIdx = self->dwarfDbgGetDbgInfo->numCompileUnit;
  self->dwarfDbgGetDbgInfo->numCompileUnit++;
  return result;
}

static Dwarf_Off  DIEOffset = 0;      /* DIE offset in compile unit */
static Dwarf_Off  DIEOverallOffset = 0;  /* DIE offset in .debug_info */
static dwarfDbgEsb_t esbShortCuName;
static dwarfDbgEsb_t esbLongCuName;

// =================================== getAttrValue =========================== 

static int getAttrValue(dwarfDbgPtr_t self, Dwarf_Half attr, Dwarf_Attribute attr_in, char **srcfiles, int cnt, size_t dieAndChildrenIdx, Dwarf_Bool isSibling, size_t dieInfoIdx, char **attrStr)
{
  int result;
  Dwarf_Attribute attrib = 0;
  Dwarf_Unsigned uval = 0;
  const char * atName = 0;
  dwarfDbgEsb_t valName;
  dwarfDbgEsb_t esb_extra;
  int tres = 0;
  int append_extra_string = 0;
  int found_search_attr = FALSE;
  int bTextFound = FALSE;
  Dwarf_Bool is_info = FALSE;
  Dwarf_Addr elf_max_address = 0;
  Dwarf_Error paerr = 0;

  char * temps = NULL;
  int res = 0;
  int fres = 0;
  int sres = 0;
  int bres = 0;
  int vres = 0;
  uint16_t flags = 0;
  size_t attrIdx;
  Dwarf_Half theform = 0;
  Dwarf_Half directform = 0;
  Dwarf_Error err = 0;
  Dwarf_Addr addr = 0;
  
  result = DWARF_DBG_ERR_OK;
  res = dwarf_get_AT_name(attr, &atName);
//printf("getAttrValue attr: 0x%08x atname: %s\n", attr_in, atName);
  fres = dwarf_whatform(attr_in, &theform, &err);
//printf("getAttrValue: fres: %d res: %d theform: 0x%02x\n", fres, res, theform);
  vres = dwarf_formudata(attr_in, &uval, &err);

//printf("++getAttrValue: dieAndChildrenIdx: 0x%02x attr: 0x%08x uval: 0x%08x theform: 0x%08x\n", dieAndChildrenIdx, attr, uval, theform);
  if ((int)dieAndChildrenIdx >= 0) {
    if (isSibling) {
      result = self->dwarfDbgDieInfo->addDieSiblingAttr(self, dieAndChildrenIdx, dieInfoIdx, attr, attr_in, uval, theform, directform, flags, &attrIdx);
    } else {
      result = self->dwarfDbgDieInfo->addDieChildAttr(self, dieAndChildrenIdx, dieInfoIdx, attr, attr_in, uval, theform, directform, flags, &attrIdx);
    }
    checkErrOK(result);
  }
  switch (theform) {
  case DW_FORM_addr:
    bres = dwarf_formaddr(attr_in, &addr, &err);
//printf("DW_FORM_addr: attrib: 0x%08x addr: 0x%08x\n", attr_in, addr);
    break;
  case DW_FORM_data1:
    fres = dwarf_whatattr(attr_in, &attr, &err);
//printf("DW_FORM_data1 attr: 0x%08x\n", attr);
    switch(attr) {
    case DW_AT_language:
//printf("DW_AT_language\n");
      break;
    case DW_AT_byte_size:
//printf("DW_AT_byte_size\n");
      break;
    case DW_AT_encoding:
//printf("DW_AT_encoding\n");
      break;
    case DW_AT_decl_file:
//printf("DW_AT_decl_file\n");
      break;
    case DW_AT_decl_line:
//printf("DW_AT_decl_line\n");
      break;
    case DW_AT_upper_bound:
//printf("DW_AT_upper_bound\n");
      break;
    case DW_AT_data_member_location:
//printf("DW_AT_data_member_location\n");
      break;
    case DW_AT_inline:
//printf("DW_AT_inline\n");
      break;
    case DW_AT_const_value:
//printf("DW_AT_const_value\n");
      break;
    case DW_AT_call_file:
//printf("DW_AT_call_file\n");
      break;
    case DW_AT_call_line:
//printf("DW_AT_call_line\n");
      break;
    case DW_AT_bit_offset:
//printf("DW_AT_bit_offset\n");
      break;
    case DW_AT_bit_size:
//printf("DW_AT_bit_size\n");
      break;
    default:
printf("ERROR attribute: 0x%08x not yet implemented\n", attr);
      break;
    }
    break;
  case DW_FORM_string:
  case DW_FORM_strp:
    sres = dwarf_formstring(attr_in, &temps, &err);
    *attrStr = temps;
    break;
  case DW_FORM_sec_offset:
printf("DW_FORM_sec_offset\n");
    break;
  case DW_FORM_ref4:
printf("DW_FORM_ref4\n");
    break;
  case DW_FORM_data2:
printf("DW_FORM_data2\n");
    break;
  case DW_FORM_data4:
printf("DW_FORM_data4\n");
    break;
  case DW_FORM_block:
printf("DW_FORM_block\n");
    break;
  case DW_FORM_block1:
printf("DW_FORM_block1\n");
    break;
  case DW_FORM_sdata:
printf("DW_FORM_sdata\n");
    break;
  case DW_FORM_exprloc:
printf("DW_FORM_exprloc\n");
    break;
  case DW_FORM_flag_present:
printf("DW_FORM_flag_present\n");
    break;
  default:
printf("ERROR theform: 0x%08x not yet implemented\n", theform);
    break;
  }
  return 1;
}


// =================================== getAttribute =========================== 

static uint8_t getAttribute(dwarfDbgPtr_t self, Dwarf_Half attr, Dwarf_Attribute attr_in, const char **srcfiles, Dwarf_Signed cnt, size_t dieAndChildrenIdx, Dwarf_Bool isSibling, size_t dieInfoIdx, const char **outStr, int *outValue)
{
  uint8_t result;
  const char *atName = NULL;
  char *templateNameStr = NULL;
  int res;
  int vres;
  Dwarf_Unsigned uval = 0;
  Dwarf_Error err;
  Dwarf_Half theform = 0;
  Dwarf_Half directform = 0;
  const char *langName = NULL;

  result = DWARF_DBG_ERR_OK;
  *outStr = NULL;
  *outValue = 0;
//printf("getAttribute: 0x%04x\n", attr);
  res = dwarf_get_AT_name(attr, &atName);
//printf("getAttribute: atName: %s\n", atName);
  getAttrValue(self, attr, attr_in, NULL, 0, dieAndChildrenIdx, isSibling, dieInfoIdx, &templateNameStr);
  switch (attr) {
  case DW_AT_language:
    vres = dwarf_formudata(attr_in, &uval, &err);
    dwarf_get_LANG_name((Dwarf_Half) uval, &langName);
    res = dwarf_whatform(attr_in, &theform, &err);
    res = dwarf_whatform_direct(attr_in, &directform, &err);
    *outStr = langName;
    break;
  case DW_AT_name:
  case DW_AT_comp_dir:
    *outStr = templateNameStr;
    break;
  default:
    break;
  }
  return result;
}

// =================================== getProducerName =========================== 

/*  Returns the producer of the CU
  Caller must ensure producernameout is
  a valid, constructed, empty dwarfDbgEsb_t instance before calling.
  Never returns DW_DLV_ERROR.  */
static int getProducerName(dwarfDbgPtr_t self, char **producerName) {
  Dwarf_Attribute producerAttr = 0;
  Dwarf_Error pnerr = 0;

  int ares = dwarf_attr(self->dwarfDbgGetDbgInfo->currCompileUnit->compileUnitDie, DW_AT_producer, &producerAttr, &pnerr);
  if (ares == DW_DLV_ERROR) {
    printf("hassattr on DW_AT_producer ares: %d pnerr: %d", ares, pnerr);
  }
  if (ares == DW_DLV_NO_ENTRY) {
    /*  We add extra quotes so it looks more like
      the names for real producers that getAttrValue
      produces. */
    *producerName = "\"<CU-missing-DW_AT_producer>\"";
  } else {
    /*  DW_DLV_OK */
    /*  The string return is valid until the next call to this
      function; so if the caller needs to keep the returned
      string, the string must be copied (makename()). */
//    getAttrValue(self, 1, producerAttr, NULL, 0, -1, 1, 1, producerName);
  }
  return ares;
}

// =================================== getCompileUnitName =========================== 

/* Returns the name of the compile unit. In case of error, give up, do not return. */
static int getCompileUnitName(dwarfDbgPtr_t self, char **shortName, char **longName) {
  Dwarf_Attribute nameAttr = 0;
  Dwarf_Error lerr = 0;
  int ares;

  ares = dwarf_attr(self->dwarfDbgGetDbgInfo->currCompileUnit->compileUnitDie, DW_AT_name, &nameAttr, &lerr);
  if (ares == DW_DLV_ERROR) {
    printf("hassattr on DW_AT_name ares: %d lerr: %d", ares, lerr);
  } else {
    if (ares == DW_DLV_NO_ENTRY) {
      *shortName = "<unknown name>";
      *longName = "<unknown name>";
    } else {
      /* DW_DLV_OK */
      /*  The string return is valid until the next call to this
        function; so if the caller needs to keep the returned
        string, the string must be copied (makename()). */
      char *fileName = 0;

//      self->dwarfDbgEsb->esbEmptyString(self, &esbLongCuName);
      getAttrValue(self, 1, nameAttr, NULL, 0, -1, 1, 1, longName);
      /* Generate the short name (fileName) */
      fileName = strrchr(*longName,'/');
      if (!fileName) {
        fileName = strrchr(*longName,'\\');
      }
      if (fileName) {
        ++fileName;
      } else {
        fileName = *longName;
      }
      *shortName = fileName;
    }
  }
  dwarf_dealloc(self->elfInfo.dbg, nameAttr, DW_DLA_ATTR);
  return ares;
}

// =================================== getCompileUnitLineInfos =========================== 

static uint8_t getCompileUnitLineInfos(dwarfDbgPtr_t self, size_t compileUnitIdx, size_t fileInfoIdx, size_t *fileLineIdx) {
  uint8_t result;
  Dwarf_Unsigned lineVersion = 0;
  Dwarf_Line_Context lineContext = 0;
  Dwarf_Small tableCount = 0;
  Dwarf_Error err = 0;
  int lres = 0;
  Dwarf_Signed lineCount = 0;
  Dwarf_Line *lineBuf = NULL;
  Dwarf_Signed lineCountActuals = 0;
  Dwarf_Line *lineBufActuals = NULL;
  Dwarf_Addr pc = 0;
  Dwarf_Unsigned lineNo = 0;
  int i = 0;

  result = DWARF_DBG_ERR_OK;
//printf("getCompileUnitLineInfos\n");
  lres = dwarf_srclines_b(self->dwarfDbgGetDbgInfo->currCompileUnit->compileUnitDie, &lineVersion, &tableCount, &lineContext, &err);
if (tableCount > 0) {
//printf(">>table_count: %d lineVersion: %d\n", tableCount, lineVersion);
}
  if (lres == DW_DLV_OK) {
//printf("dwarf_srclines_two_level_from_linecontext\n");
    lres = dwarf_srclines_two_level_from_linecontext(lineContext, &lineBuf, &lineCount,
          &lineBufActuals, &lineCountActuals, &err);
    if (lres != DW_DLV_OK) {
      return DWARF_DBG_ERR_GET_SRC_LINES;
    }
    if (lineCount > 0) {
//printf(">>>lineCount: %d\n", lineCount);
      for (i = 0; i < lineCount; i++) {
        Dwarf_Line line = lineBuf[i];
        char* fileName = 0;
        int ares = 0;
        int lires = 0;

        pc = 0;
        ares = dwarf_lineaddr(line, &pc, &err);
        if (ares != DW_DLV_OK) {
          return DWARF_DBG_ERR_GET_LINE_ADDR;
        }
        lires = dwarf_lineno(line, &lineNo, &err);
        if (lires != DW_DLV_OK) {
          return DWARF_DBG_ERR_GET_LINE_NO;
        }
//printf("dwarf_lineaddr: line: 0x%08x pc: 0x%08x lineNo: %d\n", line, pc, lineNo);
        result = self->dwarfDbgFileInfo->addFileLine(self, pc, lineNo, compileUnitIdx, fileInfoIdx, fileLineIdx);

      }
    }
  } else {
    return DWARF_DBG_ERR_GET_SRC_LINES;
  }
  return result;
}

static int numDies = 0;
static int numSiblings = 0;
static int numAddSibling = 0;
static int numAddChild = 0;
static int childrenLevel = 0;
// =================================== handleOneDie =========================== 

static uint8_t handleOneDie(dwarfDbgPtr_t self, Dwarf_Die die, char **srcfiles, Dwarf_Signed cnt, size_t compileUnitIdx, Dwarf_Bool isSibling, size_t dieAndChildrenIdx, size_t *dieInfoIdx) {
  uint8_t result;
  int tres = 0;
  int ores = 0;
  int atres = 0;
  int res = 0;
  int sres = 0;
  int cdres = DW_DLV_OK;
  const char * tagName = 0;
  size_t i = 0;
  Dwarf_Error err;
  compileUnit_t *compileUnit;
  Dwarf_Half tag = 0;
  Dwarf_Off offset = 0;
  Dwarf_Signed atCnt = 0;
  Dwarf_Attribute *atList = 0;
  Dwarf_Die child = NULL;
  Dwarf_Die sibling = NULL;

  result = DWARF_DBG_ERR_OK;
printf("handleOneDie die: %p numDies: %d\n", die, ++numDies);
  compileUnit = &self->dwarfDbgGetDbgInfo->compileUnits[compileUnitIdx];
  tres = dwarf_tag(die, &tag, &err);
  if (tres != DW_DLV_OK) {
    printf("accessing tag of die! tres: %d, err: %p", tres, err);
  }
  result = self->dwarfDbgStringInfo->getDW_TAG_string(self, tag, &tagName);
  checkErrOK(result);
printf("TAG: %p %s\n", die, tagName);

  ores = dwarf_die_CU_offset(die, &offset, &err);
  if (ores != DW_DLV_OK) {
    printf("dwarf_die_CU_offset ores: %d err: %p", ores, err);
    return DWARF_DBG_ERR_CANNOT_GET_CU_OFFSET;
  }
  if (isSibling) {
printf("numAddSibling: %d\n", ++numAddSibling);
    result = self->dwarfDbgDieInfo->addDieSibling(self, dieAndChildrenIdx, offset, tag, dieInfoIdx);
    checkErrOK(result);
  } else {
printf("numAddChild: %d\n", ++numAddChild);
    result = self->dwarfDbgDieInfo->addDieChild(self, dieAndChildrenIdx, offset, tag, dieInfoIdx);
    checkErrOK(result);
  }
  atres = dwarf_attrlist(die, &atList, &atCnt, &err);
printf("atCnt: %p %d\n", die, atCnt);
  {
    for (i = 0; i < atCnt; i++) {
      Dwarf_Half attr;
      const char *stringValue;
      const char *shortName;
      char buf[255];
      int numericValue;
      int ares;

      ares = dwarf_whatattr(atList[i], &attr, &err);
      if (ares == DW_DLV_OK) {
        stringValue = NULL;
        result = getAttribute(self, attr, atList[i], /* srcfiles */ NULL, /* cnt */ 0, dieAndChildrenIdx, isSibling, *dieInfoIdx, &stringValue, &numericValue);
        switch (attr) {
        case DW_AT_language:
printf("  DW_AT_language\n");
//printf("  LANG_name: %s\n", stringValue);
          break;
        case DW_AT_name:
printf("  DW_AT_name\n");
          if (stringValue != NULL) {
printf("    AT_name: %s\n", stringValue);
            if (strrchr(stringValue, '/') != NULL) {
              sprintf(buf, "%s", strrchr(stringValue, '/'));
printf(">>addDieATName file: %s\n", buf);
              result = self->dwarfDbgFileInfo->addCompileUnitFile(self, buf, compileUnitIdx, &compileUnit->fileNameIdx, &compileUnit->fileInfoIdx);
printf("    with /: %s result: %d\n", stringValue, result);
              checkErrOK(result);
              shortName = NULL;
            } else {
              shortName = stringValue;
            }
          }
          break;
        case DW_AT_comp_dir:
printf("  DW_AT_comp_dir\n");
          if ((stringValue != NULL) && (shortName != NULL)) {
            sprintf(buf, "%s/%s", stringValue, shortName);
printf(">>addDieAT_comp_dir file: %s\n", buf);
            result = self->dwarfDbgFileInfo->addCompileUnitFile(self, buf, compileUnitIdx, &compileUnit->fileNameIdx, &compileUnit->fileInfoIdx);
printf("    NAME: %s result: %d\n", buf, result);
            checkErrOK(result);
          }
          break;
        case DW_AT_producer:
printf("  DW_AT_producer\n");
          break;
        case DW_AT_ranges:
printf("  DW_AT_ranges\n");
          break;
        case DW_AT_low_pc:
printf("  DW_AT_low_pc\n");
          break;
        case DW_AT_stmt_list:
printf("  DW_AT_stmt_list\n");
          break;
        case DW_AT_byte_size:
printf("  DW_AT_byte_size\n");
          break;
        case DW_AT_encoding:
printf("  DW_AT_encoding\n");
          break;
        case DW_AT_decl_file:
printf("  DW_AT_decl_file\n");
          break;
        case DW_AT_decl_line:
printf("  DW_AT_decl_line\n");
          break;
        case DW_AT_type:
printf("  DW_AT_type\n");
          break;
        case DW_AT_data_member_location:
printf("  DW_AT_data_member_location\n");
          break;
        case DW_AT_sibling :
printf("  DW_AT_sibling \n");
          break;

#ifdef NOTDEF
        case DW_AT_encoding:
printf("  DW_AT_encoding\n");
          break;
        case DW_AT_encoding:
printf("  DW_AT_encoding\n");
          break;
#endif
        default:
printf("  DW_AT_?? 0x%02x\n", attr);
          break;
        }
        // here we need to handle children etc.
      }
    }
  }
  return result;
}

// =================================== handleDieAndChildren =========================== 

static uint8_t handleDieAndChildren(dwarfDbgPtr_t self, Dwarf_Die in_die_in, char **srcfiles, Dwarf_Signed cnt, size_t compileUnitIdx) {
  uint8_t result;
  size_t dieInfoIdx = 0;
  size_t dieAndChildrenIdx = 0;
  Dwarf_Error err;
  Dwarf_Die child = 0;
  Dwarf_Die sibling = 0;
  Dwarf_Die in_die = in_die_in;
  int cdres = DW_DLV_OK;
  int numChildren = 0;
  Dwarf_Bool isSibling = 0;
  

  result = DWARF_DBG_ERR_OK;
printf("handleDieAndChildren die: %p level: %d\n", in_die_in, ++childrenLevel);
  result = self->dwarfDbgDieInfo->addDieAndChildren(self, in_die_in, &dieAndChildrenIdx);
  checkErrOK(result);
  for(;;) {
    ++numChildren;
printf("pre-descent numChildren: %d\n", numChildren);
    /* Here do pre-descent processing of the die. */
    {
printf("before handleOneDie in_die: %p\n", in_die);
  if (isSibling) {
  } else {
  }
      result = handleOneDie(self, in_die, srcfiles, cnt, compileUnitIdx, isSibling, dieAndChildrenIdx, &dieInfoIdx);
printf("after handleOneDie in_die: %p\n", in_die);
printf("call dwarf_child in_die: %p\n", in_die);
      child = NULL;
      cdres = dwarf_child(in_die, &child, &err);
printf("after call dwarf_child in_die: %p child: %p cdres: %d\n", in_die, child, cdres);
      /* child first: we are doing depth-first walk */
      if (cdres == DW_DLV_OK) {
printf("child first\n");
printf("call recursive handleDieAndChildren: child: %p numChildren: %d\n", child, numChildren);
        handleDieAndChildren(self, child, srcfiles, cnt, compileUnitIdx);
        dwarf_dealloc(self->elfInfo.dbg, child, DW_DLA_DIE);
        child = 0;
      }
      cdres = dwarf_siblingof_b(self->elfInfo.dbg, in_die, /* is_info */1, &sibling, &err);
printf("dwarf_siblingof_b: numSiblings: %d in_die: %p sibling: %p\n", ++numSiblings, in_die, sibling);
      if (cdres == DW_DLV_OK) {
        /*  handleDieAndChildren(dbg, sibling, srcfiles, cnt); We
            loop around to actually print this, rather than
            recursing. Recursing is horribly wasteful of stack
            space. */
      } else if (cdres == DW_DLV_ERROR) {
        printf("error in dwarf_siblingofi_b cdres: %d err: %p", cdres, err);
        return DWARF_DBG_ERR_CANNOT_GET_SIBLING_OF_COMPILE_UNIT;
      }
printf("post-descent\n");
      if (in_die != in_die_in) {
        /*  Dealloc our in_die, but not the argument die, it belongs
            to our caller. Whether the siblingof call worked or not. */
        dwarf_dealloc(self->elfInfo.dbg, in_die, DW_DLA_DIE);
        in_die = 0;
      }
      if (cdres == DW_DLV_OK) {
        /*  Set to process the sibling, loop again. */
        in_die = sibling;
        isSibling = 1;
      } else {
        /*  We are done, no more siblings at this level. */
        break;
      }
    }
  }  /* end for loop on siblings */
printf("handleDieAndChildren done die: %p level: %d\n", in_die_in, childrenLevel--);
  return result;
}

// =================================== handleOneDieSection =========================== 

static uint8_t handleOneDieSection(dwarfDbgPtr_t self) {
  uint8_t result;
  const char * sectionName = 0;
  int res = 0;
  Dwarf_Bool isInfo = TRUE;
  Dwarf_Error err;
  unsigned loopCount = 0;
  int nres = DW_DLV_OK;
  int   compileUnitCount = 0;
  char * compileUnitShortName = NULL;
  char * compileUnitLongName = NULL;
  size_t compileUnitIdx = 0;
  int i = 0;
  size_t fileLineIdx;
  Dwarf_Signed srcCnt = 0;
  char **srcFiles = 0;

  isInfo = TRUE;
  result = DWARF_DBG_ERR_OK;
  res = dwarf_get_die_section_name(self->elfInfo.dbg, isInfo, &sectionName, &err);
  if (res != DW_DLV_OK || !sectionName || !strlen(sectionName)) {
    sectionName = ".debug_info";
  }
  /* Loop over compile units until it fails.  */
  for (;;++loopCount) {
    int sres = DW_DLV_OK;
    compileUnit_t *compileUnit;
//        struct Dwarf_Debug_Fission_Per_CU_s fission_data;
//        int fission_data_result = 0;

    result = self->dwarfDbgGetDbgInfo->addCompileUnit(self, &compileUnitIdx);
    self->dwarfDbgGetDbgInfo->currCompileUnitIdx = compileUnitIdx;
printf("addCompileUnit: result: %d compileUnitIdx: %d\n", result, compileUnitIdx);
    checkErrOK(result);
//        memset(&fission_data,0,sizeof(fission_data));
    compileUnit = &self->dwarfDbgGetDbgInfo->compileUnits[compileUnitIdx];
//printf("call dwarf_next_cu_header_d\n");
    nres = dwarf_next_cu_header_d(self->elfInfo.dbg, 1,
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
    if (nres == DW_DLV_NO_ENTRY) {
      // we have processed all entries
      break;
    }
    if (nres != DW_DLV_OK) {
      return DWARF_DBG_ERR_CANNOT_GET_NEXT_COMPILE_UNIT;
    }
    self->dwarfDbgGetDbgInfo->currCompileUnit = compileUnit;
    /*  get basic information about the current compile unit: producer, name */
    sres = dwarf_siblingof_b(self->elfInfo.dbg, NULL,/* is_info */1, &compileUnit->compileUnitDie, &err);
    if (sres != DW_DLV_OK) {
printf("siblingof cu header sres: %d err: %p", sres, err);
      return DWARF_DBG_ERR_CANNOT_GET_SIBLING_OF_COMPILE_UNIT;
    }
    dwarf_die_offsets(compileUnit->compileUnitDie, &compileUnit->overallOffset, &DIEOffset, &err);
#ifdef NOTDEF
    {
    /* Get producer name for this compile unit */
      char *producerName;
      
      getProducerName(self, self->elfInfo.dbg, &producerName);
printf("producerName: %s\n", producerName);
    }
#endif
    DIEOverallOffset = compileUnit->overallOffset;
    getCompileUnitName(self, &compileUnitShortName, &compileUnitLongName);
//printf("compileUnitDie: %p overallOffset: %d compileUnitShortName: %s\n", compileUnit->compileUnitDie, compileUnit->overallOffset, compileUnitShortName);
    compileUnit->compileUnitShortName = ckalloc(strlen(compileUnitShortName) + 1);
    compileUnit->compileUnitShortName[strlen(compileUnitShortName)] = '\0';
    memcpy(compileUnit->compileUnitShortName, compileUnitShortName, strlen(compileUnitShortName));

    {
      Dwarf_Error srcerr = 0;
      int srcf = dwarf_srcfiles(compileUnit->compileUnitDie, &srcFiles, &srcCnt, &err);

      if (srcf == DW_DLV_ERROR) {
        return DWRAF_DBG_ERR_GET_SRC_FILES;
      } /*DW_DLV_NO_ENTRY generally means there
        there is no DW_AT_stmt_list attribute.
        and we do not want to print anything
        about statements in that case */

    for (i = 0; i < srcCnt; i++) {
      size_t fileNameIdx;
      size_t fileInfoIdx;

//printf("  src: %s\n", srcFiles[i]);
      result = self->dwarfDbgFileInfo->addSourceFile(self, srcFiles[i], compileUnitIdx, &fileNameIdx, &fileInfoIdx);
printf("  src: %s %d fileNameIdx: %d fileInfoIdx: %d\n", srcFiles[i], i, fileNameIdx, fileInfoIdx);
      checkErrOK(result);
    }

printf("call handleDieAndChildren\n");
        result = handleDieAndChildren(self, compileUnit->compileUnitDie, srcFiles, srcCnt, compileUnitIdx);
    }
printf("handleOneDieSection after handleDieAndChildren result: %d\n", result);

// for testing show the structure
{
  int dieAndChildrenIdx;
  compileUnitInfo_t *compileUnitInfo;
//  showFd = fopen("showInfo.txt", "w");

  compileUnitInfo = &compileUnit->compileUnitInfo;
fprintf(showFd, "++ numDieAndChildren: %d\n", compileUnitInfo->numDieAndChildren);
  for (dieAndChildrenIdx = 0; dieAndChildrenIdx < compileUnitInfo->numDieAndChildren; dieAndChildrenIdx++) {
fprintf(showFd, "++ idx: %d\n", dieAndChildrenIdx);
    self->dwarfDbgDieInfo->showChildren(self, dieAndChildrenIdx, "  ");
    self->dwarfDbgDieInfo->showSiblings(self, dieAndChildrenIdx, "  ");
  }
fflush(showFd);

}
    // eventually handle ranges here
    // here we need to handle source lines
    result = getCompileUnitLineInfos(self, compileUnitIdx, compileUnit->fileInfoIdx, &fileLineIdx);
//printf("fileLineIdx: %d\n", fileLineIdx);
    checkErrOK(result);

    /*  Release the 'compileUnitDie' created by the call
        to 'dwarf_siblingof' at the top of the main loop. */
    dwarf_dealloc(self->elfInfo.dbg, compileUnit->compileUnitDie, DW_DLA_DIE);
    compileUnit->compileUnitDie = NULL; /* For debugging, stale die should be NULL. */
int i ;
compileUnitInfo_t *compileUnitInfo;
compileUnitInfo = &self->dwarfDbgGetDbgInfo->compileUnits[compileUnitIdx].compileUnitInfo;
for (i = 0; i < compileUnitInfo->numSourceFile; i++) {
printf(">>source: %d %d\n", i, compileUnitInfo->sourceFiles[i]);
}
  } // end loop
  return result;
}

// =================================== dwarfDbgGetDbgInfos =========================== 

int dwarfDbgGetDbgInfos(dwarfDbgPtr_t self) {
  uint8_t result;

//printf("dwarfDbgGetDbgInfos\n");
  result = DWARF_DBG_ERR_OK;
  result = self->dwarfDbgGetDbgInfo->handleOneDieSection(self);
  checkErrOK(result);

  return result;
}

// =================================== dwarfDbgGetDbgInfoInit =========================== 

int dwarfDbgGetDbgInfoInit (dwarfDbgPtr_t self) {

  self->dwarfDbgGetDbgInfo->maxCompileUnit = 0;
  self->dwarfDbgGetDbgInfo->numCompileUnit = 0;
  self->dwarfDbgGetDbgInfo->compileUnits = NULL;

  self->dwarfDbgGetDbgInfo->addCompileUnit = &addCompileUnit;
  self->dwarfDbgGetDbgInfo->handleOneDieSection = &handleOneDieSection;
  return DWARF_DBG_ERR_OK;
}
