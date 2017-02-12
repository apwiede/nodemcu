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
  Dwarf_Half attr2 = 0;

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
  char *sourceFile = NULL;
  int sourceLineNo = -1;
  
  result = DWARF_DBG_ERR_OK;
  *attrStr = NULL;
  res = dwarf_get_AT_name(attr, &atName);
printf("getAttrValue attr: 0x%08x atname: %s\n", attr_in, atName);
  fres = dwarf_whatform(attr_in, &theform, &err);
//printf("getAttrValue: fres: %d res: %d theform: 0x%02x\n", fres, res, theform);
  vres = dwarf_formudata(attr_in, &uval, &err);
printf("uval: %d\n", uval);

//printf("++getAttrValue: dieAndChildrenIdx: 0x%02x attr: 0x%08x uval: 0x%08x theform: 0x%08x\n", dieAndChildrenIdx, attr, uval, theform);
  if ((int)dieAndChildrenIdx < 0) {
printf("ERROR dieAndChildrenIdx < 0\n");
  }
  return 1;
}

// =================================== getCompileUnitLineInfos =========================== 

static uint8_t getCompileUnitLineInfos(dwarfDbgPtr_t self,  int *fileLineIdx) {
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
  Dwarf_Bool newstatement = 0;
  Dwarf_Bool lineendsequence = 0;
  Dwarf_Bool new_basic_block = 0;
  int i = 0;
  int fileInfoIdx;
  compileUnit_t *compileUnit;

  result = DWARF_DBG_ERR_OK;
//printf("getCompileUnitLineInfos\n");
  compileUnit = self->dwarfDbgCompileUnitInfo->currCompileUnit;
  lres = dwarf_srclines_b(compileUnit->compileUnitDie, &lineVersion, &tableCount, &lineContext, &err);
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
        int nsres = 0;
        int disres = 0;
        Dwarf_Bool prologue_end = 0;
        Dwarf_Bool epilogue_begin = 0;
        Dwarf_Unsigned isa = 0;
        Dwarf_Unsigned discriminator = 0;
        int flags;
        char *ns;
        char *bb;
        char *et;
        char *pe;
        char *eb;

        pc = 0;
        ares = dwarf_lineaddr(line, &pc, &err);
        if (ares != DW_DLV_OK) {
          return DWARF_DBG_ERR_GET_LINE_ADDR;
        }
        lires = dwarf_lineno(line, &lineNo, &err);
        if (lires != DW_DLV_OK) {
          return DWARF_DBG_ERR_GET_LINE_NO;
        }
        flags = 0;
        ns = "";
        bb = "";
        et = "";
        pe = "";
        eb = "";
nsres = dwarf_linebeginstatement(line, &newstatement, &err);
if (nsres == DW_DLV_OK) {
//printf("NS\n");
  if (newstatement) {
    ns = " NS";
    flags |= LINE_NEW_STATEMENT;
  }
}
nsres = dwarf_lineblock(line, &new_basic_block, &err);
if (nsres == DW_DLV_OK) {
  if (new_basic_block) {
//printf("BB\n");
    bb = " BB";
    flags |= LINE_NEW_BASIC_BLOCK;
  }
}
nsres = dwarf_lineendsequence(line, &lineendsequence, &err);
if (nsres == DW_DLV_OK) {
  if (lineendsequence) {
//printf("ET\n");
    et = " ET";
    flags |= LINE_END_SEQUENCE;
  }
}
disres = dwarf_prologue_end_etc(line, &prologue_end, &epilogue_begin, &isa, &discriminator, &err);
if (disres == DW_DLV_OK) {
//printf("prologue_end: %d epilogue_begin: %d isa: %d discriminator: %d\n", prologue_end, epilogue_begin, isa, discriminator);
  if (prologue_end) {
    pe = " PE";
    flags |= LINE_PROLOGUE_END;
  }
  if (epilogue_begin) {
    eb = " EB";
    flags |= LINE_PROLOGUE_BEGIN;
  }
}
printf("dwarf_lineaddr: line: 0x%08x pc: 0x%08x lineNo: %d%s%s%s%s%s isa: %d dis: %d\n", line, pc, lineNo, ns, bb, et, eb, pe, isa, discriminator);
        result = self->dwarfDbgFileInfo->addFileLine(self, pc, lineNo, flags, (uint16_t)isa, (uint16_t)discriminator, compileUnit->fileInfoIdx, fileLineIdx);

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

static uint8_t handleOneDie(dwarfDbgPtr_t self, Dwarf_Die die, char **srcfiles, Dwarf_Signed cnt, Dwarf_Bool isSibling, size_t dieAndChildrenIdx, size_t *dieInfoIdx) {
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
  compileUnit = self->dwarfDbgCompileUnitInfo->currCompileUnit;
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
  for (i = 0; i < atCnt; i++) {
    Dwarf_Half attr;
    Dwarf_Attribute attrIn;
    int ares;
    int dieAttrIdx;

    ares = dwarf_whatattr(atList[i], &attr, &err);
    attrIn = atList[i];
    if (ares == DW_DLV_OK) {
      result = self->dwarfDbgAttributeInfo->handleAttribute(self, die, attr, attrIn, srcfiles, cnt, dieAndChildrenIdx, *dieInfoIdx, isSibling, &dieAttrIdx);
printf("after handleAttribute\n");
    }
  }
  return result;
}

// =================================== handleDieAndChildren =========================== 

static uint8_t handleDieAndChildren(dwarfDbgPtr_t self, Dwarf_Die in_die_in, char **srcfiles, Dwarf_Signed cnt) {
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
  compileUnit_t *compileUnit = NULL;
  

  result = DWARF_DBG_ERR_OK;
  compileUnit = self->dwarfDbgCompileUnitInfo->currCompileUnit;
printf("@@handleDieAndChildren die: %p level: %d isCompileUnitDie: %d\n", in_die_in, ++childrenLevel, compileUnit->isCompileUnitDie);
  result = self->dwarfDbgDieInfo->addDieAndChildren(self, in_die_in, &dieAndChildrenIdx);
  checkErrOK(result);
  for(;;) {
    ++numChildren;
printf("pre-descent numChildren: %d\n", numChildren);
    /* Here do pre-descent processing of the die. */
    {
printf("before handleOneDie in_die: %p\n", in_die);
      result = handleOneDie(self, in_die, srcfiles, cnt, isSibling, dieAndChildrenIdx, &dieInfoIdx);
      compileUnit->isCompileUnitDie = 0;
printf("after handleOneDie in_die: %p\n", in_die);
printf("call dwarf_child in_die: %p\n", in_die);
      child = NULL;
      cdres = dwarf_child(in_die, &child, &err);
printf("after call dwarf_child in_die: %p child: %p cdres: %d\n", in_die, child, cdres);
      /* child first: we are doing depth-first walk */
      if (cdres == DW_DLV_OK) {
printf("child first\n");
printf("call recursive handleDieAndChildren: child: %p numChildren: %d\n", child, numChildren);
        handleDieAndChildren(self, child, srcfiles, cnt);
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

// =================================== handleCompileUnits =========================== 

static uint8_t handleCompileUnits(dwarfDbgPtr_t self) {
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
  int i = 0;
  int fileLineIdx;
  Dwarf_Signed srcCnt = 0;
  char **srcfiles = 0;

  isInfo = TRUE;
  result = DWARF_DBG_ERR_OK;
  res = dwarf_get_die_section_name(self->elfInfo.dbg, isInfo, &sectionName, &err);
  if (res != DW_DLV_OK || !sectionName || !strlen(sectionName)) {
    sectionName = ".debug_info";
  }
  result = self->dwarfDbgCompileUnitInfo->getAddressSizeAndMax(self, &self->dwarfDbgCompileUnitInfo->addrSize, &self->dwarfDbgCompileUnitInfo->maxAddr, &err);
printf("addrSize: 0x%08x maxAddr: 0x%08x\n", self->dwarfDbgCompileUnitInfo->addrSize, self->dwarfDbgCompileUnitInfo->maxAddr);
  /* Loop over compile units until it fails.  */
  for (;;++loopCount) {
    int sres = DW_DLV_OK;
    compileUnit_t *compileUnit = 0;

    result = self->dwarfDbgCompileUnitInfo->addCompileUnit(self);
    if (result == DWARF_DBG_ERR_NO_ENTRY) {
      // we have processed all entries
      result = DWARF_DBG_ERR_OK;
      break;
    }
    checkErrOK(result);
    compileUnit = self->dwarfDbgCompileUnitInfo->currCompileUnit;
printf("handle srcfiles\n");
    {
      Dwarf_Error srcerr = 0;
      int srcf = dwarf_srcfiles(compileUnit->compileUnitDie, &srcfiles, &srcCnt, &err);

      if (srcf == DW_DLV_ERROR) {
        return DWRAF_DBG_ERR_GET_SRC_FILES;
      } /*DW_DLV_NO_ENTRY generally means there
        there is no DW_AT_stmt_list attribute.
        and we do not want to print anything
        about statements in that case */

printf("srcCnt: %d currCompileUnitIdx: %d\n", srcCnt, self->dwarfDbgCompileUnitInfo->currCompileUnitIdx);
      for (i = 0; i < srcCnt; i++) {
        int fileNameIdx;
        int fileInfoIdx;

printf("  src: %s\n", srcfiles[i]);
        result = self->dwarfDbgFileInfo->addSourceFile(self, srcfiles[i], &fileNameIdx, &fileInfoIdx);
//printf("  src: %s %d fileNameIdx: %d fileInfoIdx: %d\n", srcfiles[i], i, fileNameIdx, fileInfoIdx);
        checkErrOK(result);
      }

printf("call handleDieAndChildren\n");
      compileUnit->isCompileUnitDie = 1;
      result = handleDieAndChildren(self, compileUnit->compileUnitDie, srcfiles, srcCnt);
    }
printf("handleCompileUnits after handleDieAndChildren result: %d\n", result);

#ifdef SHOWSTRUCTURE
// for testing show the structure
{
  int dieAndChildrenIdx;
//  showFd = fopen("showInfo.txt", "w");

fprintf(showFd, "++ numDieAndChildren: %d\n", compileUnit->numDieAndChildren);
  for (dieAndChildrenIdx = 0; dieAndChildrenIdx < compileUnit->numDieAndChildren; dieAndChildrenIdx++) {
fprintf(showFd, "++ idx: %d\n", dieAndChildrenIdx);
    self->dwarfDbgDieInfo->showChildren(self, dieAndChildrenIdx, "  ");
    self->dwarfDbgDieInfo->showSiblings(self, dieAndChildrenIdx, "  ");
  }
fflush(showFd);

}
#endif
    // eventually handle ranges here
    // here we need to handle source lines
    result = getCompileUnitLineInfos(self, &fileLineIdx);
//printf("fileLineIdx: %d\n", fileLineIdx);
    checkErrOK(result);

    /*  Release the 'compileUnitDie' created by the call
        to 'dwarf_siblingof' at the top of the main loop. */
    dwarf_dealloc(self->elfInfo.dbg, compileUnit->compileUnitDie, DW_DLA_DIE);
    compileUnit->compileUnitDie = NULL; /* For debugging, stale die should be NULL. */
int i ;
compileUnit = self->dwarfDbgCompileUnitInfo->currCompileUnit;
for (i = 0; i < compileUnit->numSourceFile; i++) {
//printf(">>source: %d %d\n", i, compileUnitInfo->sourceFiles[i]);
}
  } // end loop
  return result;
}

// =================================== dwarfDbgGetDbgInfos =========================== 

int dwarfDbgGetDbgInfos(dwarfDbgPtr_t self) {
  uint8_t result;
  char *chunk;

//printf("dwarfDbgGetDbgInfos\n");
  result = DWARF_DBG_ERR_OK;
  // for performance alloc a big chunk of memory and free it imidiately again
  chunk = ckalloc(10*1024*1024);
  ckfree(chunk);
  result = self->dwarfDbgFrameInfo->getFrameList(self);
printf("getFrameLists: result: %d\n", result);
  checkErrOK(result);
  result = self->dwarfDbgGetDbgInfo->handleCompileUnits(self);
printf("handleCompileUnits: result: %d\n", result);
  checkErrOK(result);
  return result;
}

// =================================== dwarfDbgGetDbgInfoInit =========================== 

int dwarfDbgGetDbgInfoInit (dwarfDbgPtr_t self) {

  self->dwarfDbgGetDbgInfo->handleCompileUnits = &handleCompileUnits;
  return DWARF_DBG_ERR_OK;
}
