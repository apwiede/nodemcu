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
 * File:   dwarfDbgFileInfo.h
 * Author: Arnulf P. Wiedemann <arnulf@wiedemann-pri.de>
 *
 * Created on January 27, 2017
 */

#ifndef DWARFDBG_FILE_INFO_H
#define	DWARFDBG_FILE_INFO_H

#define LINE_NEW_STATEMENT   0x01
#define LINE_NEW_BASIC_BLOCK 0x02
#define LINE_END_SEQUENCE    0x04
#define LINE_PROLOGUE_END    0x08
#define LINE_PROLOGUE_BEGIN  0x10

typedef uint8_t (* addDirName_t)(dwarfDbgPtr_t self, char *dirName);
typedef uint8_t (* addFileName_t)(dwarfDbgPtr_t self, char *fileName, size_t dirNameIdx);
typedef uint8_t (* addSourceFile_t)(dwarfDbgPtr_t self, char *pathName, size_t compileUnitIdx, size_t *fileNameIdx, size_t *fileInfoIdx);
typedef uint8_t (* addCompileUnitFile_t)(dwarfDbgPtr_t self, char *pathName, size_t compileUnitIdx, size_t *fileNameIdx, size_t *fileInfoIdx);
typedef uint8_t (* addFileInfo_t)(dwarfDbgPtr_t self, size_t compileUnitIdx, size_t fileNameIdx, size_t *fileInfoIdx);
typedef uint8_t (* addFileLine_t)(dwarfDbgPtr_t self, Dwarf_Addr pc, size_t lineNo, int flags, uint16_t isa, uint16_t discriminator, size_t fileInfoIdx, size_t *fileLineIdx);
typedef uint8_t (* addRangeInfo_t)(dwarfDbgPtr_t self, Dwarf_Addr dwr_addr1, Dwarf_Addr dwr_addr2, enum Dwarf_Ranges_Entry_Type dwrType, size_t *rangeInfoIdx);

typedef struct dirNamesInfo {
  int  maxDirName;    /* Size of the dirNames array. */
  int  numDirName;    /* Index of the topmost dirName */
  char **dirNames;
} dirNamesInfo_t;

typedef struct fileNameInfo {
  char *fileName;
  size_t dirNameIdx;
} fileNameInfo_t;
  
typedef struct fileNamesInfo {
  int  maxFileName;    /* Size of the fileNames array. */
  int  numFileName;    /* Index of the topmost fileName */
  fileNameInfo_t *fileNames;
} fileNamesInfo_t;

typedef struct fileLineInfo {
  Dwarf_Addr pc;
  size_t lineNo;
  uint16_t flags;
  uint16_t isa;
  uint16_t discriminator;
} fileLineInfo_t;

typedef struct fileInfo {
  size_t fileNameIdx;
  int  maxFileLine;    /* Size of the fileLines array. */
  int  numFileLine;    /* Index of the topmost entry */
  fileLineInfo_t *fileLines;
} fileInfo_t;

typedef struct rangeInfo {
    Dwarf_Addr dwr_addr1;
    Dwarf_Addr dwr_addr2;
    enum Dwarf_Ranges_Entry_Type  dwr_type;
} rangeInfo_t;

typedef struct compileUnitInfo {
  char *fileName;
  Dwarf_Off overallOffset;
  int  maxFileInfo;    /* Size of the fileInfos array. */
  int  numFileInfo;    /* Index of the topmost entry */
  fileInfo_t *fileInfos;
  int  maxSourceFile;    /* Size of the source files index array. */
  int  numSourceFile;    /* Index of the topmost entry */
  size_t *sourceFiles;
  size_t numDieAndChildren;
  size_t maxDieAndChildren;
  dieAndChildrenInfo_t *dieAndChildrenInfo;
  int  maxRangeInfo;    /* Size of the rangeInfos array. */
  int  numRangeInfo;    /* Index of the topmost entry */
  rangeInfo_t *rangeInfos;
} compileUnitInfo_t;

typedef struct compileUnitsInfo {
  int  maxCompileUnitInfo;    /* Size of the compileUnitInfos array. */
  int  numCompileUnitInfo;    /* Index of the topmost entry */
  compileUnitInfo_t *compileUnitInfos;
} compileUnitsInfo_t;

typedef struct dwarfDbgFileInfo {
  dirNamesInfo_t dirNamesInfo;
  fileNamesInfo_t fileNamesInfo;
  compileUnitsInfo_t compileUnitsInfo;

  addDirName_t addDirName;
  addFileName_t addFileName;
  addSourceFile_t addSourceFile;
  addCompileUnitFile_t addCompileUnitFile;
  addFileInfo_t addFileInfo;
  addFileLine_t addFileLine;
  addRangeInfo_t addRangeInfo;
} dwarfDbgFileInfo_t;


#endif  /* DWARFDBG_FILE_INFO_H */
