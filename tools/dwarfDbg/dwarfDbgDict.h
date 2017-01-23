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
 * File:   dwarfDbgDict.h
 * Author: Arnulf P. Wiedemann <arnulf@wiedemann-pri.de>
 *
 * Created on January 22, 2017
 */

#ifndef DWARFDBG_DICT_H
#define	DWARFDBG_DICT_H

typedef uint8_t (* addDirName_t)(dwarfDbgPtr_t self, char *dirName);
typedef uint8_t (* addFileName_t)(dwarfDbgPtr_t self, char *fileName, size_t dirNameIdx);
typedef uint8_t (* addSourceFile_t)(dwarfDbgPtr_t self, char *pathName, size_t compileUnitIdx, size_t *fileNameIdx);
typedef uint8_t (* addFileLine_t)(dwarfDbgPtr_t self, size_t lineNo, size_t *compileUnitIdx, size_t *fileNameIdx);
typedef uint8_t (* addCompileUnitFile_t)(dwarfDbgPtr_t self, char *pathName, Dwarf_Off overallOffset, size_t *compileUnitIdx);

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

typedef struct fileInfo {
  size_t fileIdx;
  int  maxFileLine;    /* Size of the fileLines array. */
  int  numFileLine;    /* Index of the topmost fileName */
  int *fileLines;
} fileInfo_t;

typedef struct compileUnitInfo {
  char *fileName;
  Dwarf_Off overallOffset;
  int  maxFileInfo;    /* Size of the fileInfos array. */
  int  numFileInfo;    /* Index of the topmost fileName */
  fileInfo_t *fileInfos;
} compileUnitInfo_t;

typedef struct compileUnitsInfo {
  int  maxCompileUnit;    /* Size of the compileUnits array. */
  int  numCompileUnit;    /* Index of the topmost fileName */
  compileUnitInfo_t *compileUnitInfos;
} compileUnitsInfo_t;

typedef struct dwarfDbgDict {
  dirNamesInfo_t dirNamesInfo;
  fileNamesInfo_t fileNamesInfo;
  compileUnitsInfo_t compileUnitsInfo;

  addDirName_t addDirName;
  addFileName_t addFileName;
  addSourceFile_t addSourceFile;
  addFileLine_t addFileLine;
  addCompileUnitFile_t addCompileUnitFile;
 
} dwarfDbgDict_t;


#endif  /* DWARFDBG_DICT_H */
