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
    result = self->dwarfDbgLineInfo->handleLineInfos(self, &fileLineIdx);
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
