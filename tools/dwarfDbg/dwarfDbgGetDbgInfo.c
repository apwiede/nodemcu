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

static int numSiblings = 0;
static int numAddSibling = 0;
static int numAddChild = 0;

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
  DWARF_DBG_PRINT(self, "G", 1, "addrSize: 0x%08x maxAddr: 0x%08x\n", self->dwarfDbgCompileUnitInfo->addrSize, self->dwarfDbgCompileUnitInfo->maxAddr);
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
    DWARF_DBG_PRINT(self, "G", 1, "handle srcfiles\n");
    {
      Dwarf_Error srcerr = 0;
      int srcf = dwarf_srcfiles(compileUnit->compileUnitDie, &srcfiles, &srcCnt, &err);

      if (srcf == DW_DLV_ERROR) {
        return DWRAF_DBG_ERR_GET_SRC_FILES;
      } /*DW_DLV_NO_ENTRY generally means there
        there is no DW_AT_stmt_list attribute.
        and we do not want to print anything
        about statements in that case */

      DWARF_DBG_PRINT(self, "G", 1, "srcCnt: %d currCompileUnitIdx: %d\n", srcCnt, self->dwarfDbgCompileUnitInfo->currCompileUnitIdx);
      for (i = 0; i < srcCnt; i++) {
        int pathNameIdx;
        int fileInfoIdx;

        DWARF_DBG_PRINT(self, "G", 1, "  src: %s\n", srcfiles[i]);
        result = self->dwarfDbgFileInfo->addSourceFile(self, srcfiles[i], &pathNameIdx, &fileInfoIdx);
//printf("  src: %s %d pathNameIdx: %d fileInfoIdx: %d\n", srcfiles[i], i, pathNameIdx, fileInfoIdx);
        checkErrOK(result);
      }

      compileUnit->isCompileUnitDie = 1;
      compileUnit->level = 0;
      result = self->dwarfDbgDieInfo->handleDieAndChildren(self, compileUnit->compileUnitDie, /* isSibling */ 0, /* level */ 0, srcfiles, srcCnt);
      checkErrOK(result);
    }

    // add the typedefs here
    result = self->dwarfDbgTypeInfo->addCompileUnitTagTypes(self);
printf("addCompileUnitTagTypes: result: %d\n", result);
    checkErrOK(result);
    result = self->dwarfDbgTypeInfo->checkDieTypeRefIdx(self);
printf("checkDieTypeRefIdx: result: %d\n", result);
    checkErrOK(result);
#ifndef NOTDEF
result = self->dwarfDbgDieInfo->printCompileUnitDieAndChildren(self);
checkErrOK(result);
#endif
#ifdef NOTDEF
result = self->dwarfDbgTypeInfo->printCompileUnitTagTypes(self);
printf("printCompileUnitTagTypes: result: %d\n", result);
checkErrOK(result);
#endif
fflush(showFd);

    
//#define SHOWSTRUCTURE
#ifdef SHOWSTRUCTURE
// for testing show the structure
{
  int dieAndChildrenIdx;
//  showFd = fopen("showInfo.txt", "w");

fprintf(showFd, "++ numDieAndChildren: %d cu: %s\n", compileUnit->numDieAndChildren, compileUnit->shortFileName);
  for (dieAndChildrenIdx = 0; dieAndChildrenIdx < compileUnit->numDieAndChildren; dieAndChildrenIdx++) {
fprintf(showFd, "++ childIdx: %d\n", dieAndChildrenIdx);
fprintf(showFd, "++ children:\n");
    self->dwarfDbgDieInfo->showChildren(self, dieAndChildrenIdx, "  ");
fprintf(showFd, "++ siblings:\n");
    self->dwarfDbgDieInfo->showSiblings(self, dieAndChildrenIdx, "  ");
  }
fflush(showFd);

}
#endif
    // eventually handle ranges here
    // here we handle source lines
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
  chunk = ckalloc(100*1024*1024);
  ckfree(chunk);
  result = self->dwarfDbgFrameInfo->getFrameList(self);
  DWARF_DBG_PRINT(self, "G", 1, "getFrameLists: result: %d\n", result);
  checkErrOK(result);
  result = self->dwarfDbgGetDbgInfo->handleCompileUnits(self);
  DWARF_DBG_PRINT(self, "G", 1, "handleCompileUnits: result: %d\n", result);
  checkErrOK(result);
  return result;
}

// =================================== dwarfDbgGetDbgInfoInit =========================== 

int dwarfDbgGetDbgInfoInit (dwarfDbgPtr_t self) {

  self->dwarfDbgGetDbgInfo->handleCompileUnits = &handleCompileUnits;
  return DWARF_DBG_ERR_OK;
}
