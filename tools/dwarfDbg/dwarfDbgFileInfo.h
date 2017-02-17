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

typedef struct dirNamesInfo {
  int  maxDirName;    /* Size of the dirNames array. */
  int  numDirName;    /* Index of the topmost dirName */
  char **dirNames;
} dirNamesInfo_t;

typedef struct pathNameInfo {
  char *fileName;
  size_t dirNameIdx;
} pathNameInfo_t;
  
typedef struct pathNamesInfo {
  int  maxPathName;    /* Size of the fileNames array. */
  int  numPathName;    /* Index of the topmost fileName */
  pathNameInfo_t *pathNames;
} pathNamesInfo_t;

typedef struct fileInfo {
  int pathNameIdx;
  int  maxFileLine;    /* Size of the fileLines array. */
  int  numFileLine;    /* Index of the topmost entry */
  lineInfo_t *fileLines;
} fileInfo_t;

typedef uint8_t (* addDirName_t)(dwarfDbgPtr_t self, char *dirName);
typedef uint8_t (* addPathName_t)(dwarfDbgPtr_t self, char *fileName, int dirNameIdx, int *pathNameidx);
typedef uint8_t (* addSourceFile_t)(dwarfDbgPtr_t self, char *pathName, int *pathNameIdx, int *fileInfoIdx);
typedef uint8_t (* addCompileUnitFile_t)(dwarfDbgPtr_t self, char *pathName, int *pathNameIdx, int *fileInfoIdx);
typedef uint8_t (* addFileInfo_t)(dwarfDbgPtr_t self, int pathNameIdx, int *fileInfoIdx);
typedef uint8_t (* getPathNameIdxFromFileName_t)(dwarfDbgPtr_t self, const char *pathName, int *pathNameIdx);
typedef uint8_t (* getFileNameFromPathNameIdx_t)(dwarfDbgPtr_t self, int pathNameIdx,  const char **pathName);

typedef struct dwarfDbgFileInfo {
  dirNamesInfo_t dirNamesInfo;
  pathNamesInfo_t pathNamesInfo;

  addDirName_t addDirName;
  addPathName_t addPathName;
  addSourceFile_t addSourceFile;
  addCompileUnitFile_t addCompileUnitFile;
  addFileInfo_t addFileInfo;
  getPathNameIdxFromFileName_t getPathNameIdxFromFileName;
  getFileNameFromPathNameIdx_t getFileNameFromPathNameIdx;
  addRangeInfo_t addRangeInfo;
} dwarfDbgFileInfo_t;


#endif  /* DWARFDBG_FILE_INFO_H */
