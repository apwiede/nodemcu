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
 * File:   dwarfDbgFileInfo.c
 * Author: Arnulf P. Wiedemann <arnulf@wiedemann-pri.de>
 *
 * Created on January 27, 2017
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libelf.h>
#include <stdlib.h>
#include <string.h>

#include "dwarfDbgInt.h"

// =================================== addDirName =========================== 

static uint8_t addDirName(dwarfDbgPtr_t self, char *dirName) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;
  if (self->dwarfDbgFileInfo->dirNamesInfo.maxDirName <= self->dwarfDbgFileInfo->dirNamesInfo.numDirName) {
    self->dwarfDbgFileInfo->dirNamesInfo.maxDirName += 50;
    if (self->dwarfDbgFileInfo->dirNamesInfo.dirNames == NULL) {
      self->dwarfDbgFileInfo->dirNamesInfo.dirNames = (char **)ckalloc(sizeof(char *) * self->dwarfDbgFileInfo->dirNamesInfo.maxDirName);
      if (self->dwarfDbgFileInfo->dirNamesInfo.dirNames == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      self->dwarfDbgFileInfo->dirNamesInfo.dirNames = (char **)ckrealloc((char *)self->dwarfDbgFileInfo->dirNamesInfo.dirNames, sizeof(char *) * self->dwarfDbgFileInfo->dirNamesInfo.maxDirName);
      if (self->dwarfDbgFileInfo->dirNamesInfo.dirNames == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
  self->dwarfDbgFileInfo->dirNamesInfo.dirNames[self->dwarfDbgFileInfo->dirNamesInfo.numDirName] = (char *)ckalloc(strlen(dirName) + 1);
  if (self->dwarfDbgFileInfo->dirNamesInfo.dirNames[self->dwarfDbgFileInfo->dirNamesInfo.numDirName] == NULL) {
    return DWARF_DBG_ERR_OUT_OF_MEMORY;
  }
  self->dwarfDbgFileInfo->dirNamesInfo.dirNames[self->dwarfDbgFileInfo->dirNamesInfo.numDirName][strlen(dirName)] = '\0';
  memcpy(self->dwarfDbgFileInfo->dirNamesInfo.dirNames[self->dwarfDbgFileInfo->dirNamesInfo.numDirName], dirName, strlen(dirName));
//printf("addDirName: %d %s\n", self->dwarfDbgFileInfo->dirNamesInfo.numDirName, dirName);
  self->dwarfDbgFileInfo->dirNamesInfo.numDirName++;
  return result;
}

// =================================== addFileName =========================== 

static uint8_t addFileName(dwarfDbgPtr_t self, char *fileName, size_t dirNameIdx) {
  uint8_t result;
  fileNameInfo_t *fileNameInfo;

  result = DWARF_DBG_ERR_OK;
  if (self->dwarfDbgFileInfo->fileNamesInfo.maxFileName <= self->dwarfDbgFileInfo->fileNamesInfo.numFileName) {
    self->dwarfDbgFileInfo->fileNamesInfo.maxFileName += 50;
    if (self->dwarfDbgFileInfo->fileNamesInfo.fileNames == NULL) {
      self->dwarfDbgFileInfo->fileNamesInfo.fileNames = (fileNameInfo_t *)ckalloc(sizeof(fileNameInfo_t) * self->dwarfDbgFileInfo->fileNamesInfo.maxFileName);
      if (self->dwarfDbgFileInfo->fileNamesInfo.fileNames == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      self->dwarfDbgFileInfo->fileNamesInfo.fileNames = (fileNameInfo_t *)ckrealloc((char *)self->dwarfDbgFileInfo->fileNamesInfo.fileNames, sizeof(fileNameInfo_t) * self->dwarfDbgFileInfo->fileNamesInfo.maxFileName);
      if (self->dwarfDbgFileInfo->fileNamesInfo.fileNames == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
  fileNameInfo = &self->dwarfDbgFileInfo->fileNamesInfo.fileNames[self->dwarfDbgFileInfo->fileNamesInfo.numFileName];
  fileNameInfo->fileName = (char *)ckalloc(strlen(fileName) + 1);
  if (fileNameInfo->fileName == NULL) {
    return DWARF_DBG_ERR_OUT_OF_MEMORY;
  }
  fileNameInfo->fileName[strlen(fileName)] == '\0';
  memcpy(fileNameInfo->fileName, fileName, strlen(fileName));
  fileNameInfo->dirNameIdx = dirNameIdx;
//printf("addFileName: num: %d dirNameIdx: %d %s\n", self->dwarfDbgFileInfo->fileNamesInfo.numFileName, dirNameIdx, fileName);
  self->dwarfDbgFileInfo->fileNamesInfo.numFileName++;
  return result;
}

// =================================== addSourceFile =========================== 

static uint8_t addSourceFile(dwarfDbgPtr_t self, char *pathName, size_t compileUnitIdx, size_t *fileNameIdx, size_t *fileInfoIdx) {
  uint8_t result;
  char *cp;
  int i;
  int dirIdx;
  int fileIdx;
  fileNameInfo_t *fileNameInfo;

  result = DWARF_DBG_ERR_OK;
  cp = strrchr(pathName, '/');
  *cp++ = '\0';
printf("path: %s name: %s\n", pathName, cp);
  dirIdx = -1;
  for (i = 0; i < self->dwarfDbgFileInfo->dirNamesInfo.numDirName; i++) {
    if (strcmp(pathName, self->dwarfDbgFileInfo->dirNamesInfo.dirNames[i]) == 0) {
printf("found: dirName %d num: %d\n", i, self->dwarfDbgFileInfo->dirNamesInfo.numDirName);
      dirIdx = i;
      break;
    }
  }
  if (dirIdx < 0) {
    dirIdx = self->dwarfDbgFileInfo->dirNamesInfo.numDirName;
    result = self->dwarfDbgFileInfo->addDirName(self, pathName);
    checkErrOK(result);
  }
  fileIdx = -1;
  for (i = 0; i < self->dwarfDbgFileInfo->fileNamesInfo.numFileName; i++) {
    fileNameInfo = &self->dwarfDbgFileInfo->fileNamesInfo.fileNames[i];
    if ((strcmp(cp, fileNameInfo->fileName) == 0) && (fileNameInfo->dirNameIdx == dirIdx)) {
printf("found: fileName %d num: %d\n", i, self->dwarfDbgFileInfo->fileNamesInfo.numFileName);
      fileIdx = i;
      break;
    }
  }
  if (fileIdx < 0) {
    fileIdx = self->dwarfDbgFileInfo->fileNamesInfo.numFileName;
    result = self->dwarfDbgFileInfo->addFileName(self, cp, dirIdx);
    checkErrOK(result);
  }
  *fileNameIdx = fileIdx;
  result = self->dwarfDbgFileInfo->addFileInfo(self, compileUnitIdx, fileIdx, fileInfoIdx);
  checkErrOK(result);
  return result;
}

// =================================== addFileInfo =========================== 

static uint8_t addFileInfo(dwarfDbgPtr_t self, size_t compileUnitIdx, size_t fileNameIdx, size_t *fileInfoIdx) {
  uint8_t result;
  compileUnit_t *compileUnit;
  compileUnitInfo_t *compileUnitInfo;
  fileInfo_t *fileInfo;

  result = DWARF_DBG_ERR_OK;
  compileUnit = &self->dwarfDbgGetInfo->compileUnits[compileUnitIdx];
  compileUnitInfo = &compileUnit->compileUnitInfo;
  if (compileUnitInfo->maxFileInfo <= compileUnitInfo->numFileInfo) {
    compileUnitInfo->maxFileInfo += 10;
    if (compileUnitInfo->fileInfos == NULL) {
      compileUnitInfo->fileInfos = (fileInfo_t *)ckalloc(sizeof(fileInfo_t) * compileUnitInfo->maxFileInfo);
      if (compileUnitInfo->fileInfos == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      compileUnitInfo->fileInfos = (fileInfo_t *)ckrealloc((char *)compileUnitInfo->fileInfos, sizeof(fileInfo_t) * compileUnitInfo->maxFileInfo);
      if (compileUnitInfo->fileInfos == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
  fileInfo = &compileUnitInfo->fileInfos[compileUnitInfo->numFileInfo];
  fileInfo->maxFileLine = 0;
  fileInfo->numFileLine = 0;
  fileInfo->fileLines = NULL;
  fileInfo->fileNameIdx = fileNameIdx;
  *fileInfoIdx = compileUnitInfo->numFileInfo;
printf("addFileInfo: compileUnitIdx: %d fileNameIdx: %d fileInfoIdx: %d\n", compileUnitIdx, fileNameIdx, *fileInfoIdx);
  compileUnitInfo->numFileInfo++;
  return result;
}

// =================================== addFileLine =========================== 

static uint8_t addFileLine(dwarfDbgPtr_t self, Dwarf_Addr pc, size_t lineNo, size_t compileUnitIdx, size_t fileInfoIdx, size_t *fileLineIdx) {
  uint8_t result;
  compileUnit_t *compileUnit;
  compileUnitInfo_t *compileUnitInfo;
  fileInfo_t *fileInfo;
  fileLineInfo_t *fileLineInfo;

printf("addFileLine: pc: 0x%08x lineNo: %d\n", pc, lineNo);
  result = DWARF_DBG_ERR_OK;
  compileUnit = &self->dwarfDbgGetInfo->compileUnits[compileUnitIdx];
  compileUnitInfo = &compileUnit->compileUnitInfo;
  if (compileUnitInfo->fileInfos == NULL) {
    // seems to be no file infos!!
    return result;
  }
  fileInfo = &compileUnitInfo->fileInfos[fileInfoIdx];
  if (fileInfo->maxFileLine <= fileInfo->numFileLine) {
    fileInfo->maxFileLine += 5;
    if (fileInfo->fileLines == NULL) {
      fileInfo->fileLines = (fileLineInfo_t *)ckalloc(sizeof(fileLineInfo_t) * fileInfo->maxFileLine);
      if (fileInfo->fileLines == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      fileInfo->fileLines = (fileLineInfo_t *)ckrealloc((char *)fileInfo->fileLines, sizeof(fileLineInfo_t) * fileInfo->maxFileLine);
      if (fileInfo->fileLines == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
  fileLineInfo = &fileInfo->fileLines[fileInfo->numFileLine];
  fileLineInfo->lineNo = lineNo;
  fileLineInfo->pc = pc;
  *fileLineIdx = fileInfo->numFileLine;
  fileInfo->numFileLine++;
  return result;
}

// =================================== dwarfDbgGetFileInfos =========================== 

int dwarfDbgGetFileInfos(dwarfDbgPtr_t self) {
  uint8_t result;

printf("dwarfDbgGetFileInfos self: %p numCompileUnit: %d\n", self, self->dwarfDbgGetInfo->numCompileUnit);
  result = DWARF_DBG_ERR_OK;
  // make a Tcl list of all compile unit file names
  // make a Tcl dict of all all lines and addresse for each compile unit file name
  

  return result;
}

// =================================== dwarfDbgFileInfoInit =========================== 

int dwarfDbgFileInfoInit (dwarfDbgPtr_t self) {
  self->dwarfDbgFileInfo->dirNamesInfo.maxDirName = 0;
  self->dwarfDbgFileInfo->dirNamesInfo.numDirName = 0;
  self->dwarfDbgFileInfo->dirNamesInfo.dirNames = NULL;

  self->dwarfDbgFileInfo->fileNamesInfo.maxFileName = 0;
  self->dwarfDbgFileInfo->fileNamesInfo.numFileName = 0;
  self->dwarfDbgFileInfo->fileNamesInfo.fileNames = NULL;

  self->dwarfDbgFileInfo->compileUnitsInfo.maxCompileUnit = 0;
  self->dwarfDbgFileInfo->compileUnitsInfo.numCompileUnit = 0;
  self->dwarfDbgFileInfo->compileUnitsInfo.compileUnitInfos = NULL;

  self->dwarfDbgFileInfo->addDirName = addDirName;
  self->dwarfDbgFileInfo->addFileName = addFileName;
  self->dwarfDbgFileInfo->addSourceFile = addSourceFile;
  self->dwarfDbgFileInfo->addFileLine = addFileLine;
  self->dwarfDbgFileInfo->addFileInfo = addFileInfo;
  return DWARF_DBG_ERR_OK;
}

