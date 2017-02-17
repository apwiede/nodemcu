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
 * File:   dwarfDbgLineInfo.c
 * Author: Arnulf P. Wiedemann <arnulf@wiedemann-pri.de>
 *
 * Created on February 12, 2017
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libelf.h>
#include <stdlib.h>
#include <string.h>

#include "dwarfDbgInt.h"

/* *************************************************************************
 * lineInfo:
 *   Dwarf_Addr pc   # addr in object
 *   int lineNo      # line number in source file
 *   uint16_t flags
 *   uint8_t isa
 *   uint8_t discriminator
 *
 */

// =================================== addLineInfo =========================== 

static uint8_t addLineInfo(dwarfDbgPtr_t self, Dwarf_Addr pc, int lineNo, int flags, uint16_t isa, uint16_t discriminator, int fileInfoIdx, int *fileLineIdx) {
  uint8_t result;
  compileUnit_t *compileUnit;
  fileInfo_t *fileInfo;
  lineInfo_t *lineInfo;

//printf("addFileLine: pc: 0x%08x lineNo: %d fileInfoIdx: %d\n", pc, lineNo, fileInfoIdx);
  result = DWARF_DBG_ERR_OK;
  compileUnit = self->dwarfDbgCompileUnitInfo->currCompileUnit;
  if (compileUnit->fileInfos == NULL) {
    // seems to be no file infos!!
    return result;
  }
  fileInfo = &compileUnit->fileInfos[fileInfoIdx];
  if (fileInfo->maxFileLine <= fileInfo->numFileLine) {
    fileInfo->maxFileLine += 5;
    if (fileInfo->fileLines == NULL) {
      fileInfo->fileLines = (lineInfo_t *)ckalloc(sizeof(lineInfo_t) * fileInfo->maxFileLine);
      if (fileInfo->fileLines == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      fileInfo->fileLines = (lineInfo_t *)ckrealloc((char *)fileInfo->fileLines, sizeof(lineInfo_t) * fileInfo->maxFileLine);
      if (fileInfo->fileLines == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
  lineInfo = &fileInfo->fileLines[fileInfo->numFileLine];
  lineInfo->lineNo = lineNo;
  lineInfo->pc = pc;
  lineInfo->flags = flags;
  lineInfo->isa = isa;
  lineInfo->discriminator = discriminator;
  *fileLineIdx = fileInfo->numFileLine;
  fileInfo->numFileLine++;
  return result;
}

// =================================== handleLineInfos =========================== 

static uint8_t handleLineInfos(dwarfDbgPtr_t self, int *fileLineIdx) {
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
printf(">>table_count: %d lineVersion: %d\n", tableCount, lineVersion);
}
  if (lres == DW_DLV_OK) {
//printf("dwarf_srclines_two_level_from_linecontext\n");
    lres = dwarf_srclines_two_level_from_linecontext(lineContext, &lineBuf, &lineCount, &lineBufActuals, &lineCountActuals, &err);
//printf("lineCount: %d lineCountActuals: %d\n", lineCount, lineCountActuals);
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
        DWARF_DBG_PRINT(self, "L", 1, "dwarf_lineaddr: line: 0x%08x pc: 0x%08x lineNo: %d%s%s%s%s%s isa: %d dis: %d\n", line, pc, lineNo, ns, bb, et, eb, pe, isa, discriminator);
        result = self->dwarfDbgLineInfo->addLineInfo(self, pc, lineNo, flags, (uint16_t)isa, (uint16_t)discriminator, compileUnit->fileInfoIdx, fileLineIdx);

      }
    }
  } else {
    return DWARF_DBG_ERR_GET_SRC_LINES;
  }
  return result;
}

// =================================== dwarfDbgLineInfoInit =========================== 

int dwarfDbgLineInfoInit (dwarfDbgPtr_t self) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;

  self->dwarfDbgLineInfo->addLineInfo = &addLineInfo;
  self->dwarfDbgLineInfo->handleLineInfos = &handleLineInfos;
  return result;
}
