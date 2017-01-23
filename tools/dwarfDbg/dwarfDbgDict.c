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
 * File:   dwarfDbgDict.c
 * Author: Arnulf P. Wiedemann <arnulf@wiedemann-pri.de>
 *
 * Created on January 22, 2017
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
  if (self->dwarfDbgDict->dirNamesInfo.maxDirName <= self->dwarfDbgDict->dirNamesInfo.numDirName) {
    self->dwarfDbgDict->dirNamesInfo.maxDirName += 50;
    if (self->dwarfDbgDict->dirNamesInfo.dirNames == NULL) {
      self->dwarfDbgDict->dirNamesInfo.dirNames = (char **)ckalloc(sizeof(char *) * self->dwarfDbgDict->dirNamesInfo.maxDirName);
      if (self->dwarfDbgDict->dirNamesInfo.dirNames == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      self->dwarfDbgDict->dirNamesInfo.dirNames = (char **)ckrealloc((char *)self->dwarfDbgDict->dirNamesInfo.dirNames, sizeof(char *) * self->dwarfDbgDict->dirNamesInfo.maxDirName);
      if (self->dwarfDbgDict->dirNamesInfo.dirNames == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
  self->dwarfDbgDict->dirNamesInfo.dirNames[self->dwarfDbgDict->dirNamesInfo.numDirName] = (char *)ckalloc(strlen(dirName) + 1);
  if (self->dwarfDbgDict->dirNamesInfo.dirNames[self->dwarfDbgDict->dirNamesInfo.numDirName] == NULL) {
    return DWARF_DBG_ERR_OUT_OF_MEMORY;
  }
  self->dwarfDbgDict->dirNamesInfo.dirNames[self->dwarfDbgDict->dirNamesInfo.numDirName][strlen(dirName)] = '\0';
  memcpy(self->dwarfDbgDict->dirNamesInfo.dirNames[self->dwarfDbgDict->dirNamesInfo.numDirName], dirName, strlen(dirName));
printf("addDirName: %d %s\n", self->dwarfDbgDict->dirNamesInfo.numDirName, dirName);
  self->dwarfDbgDict->dirNamesInfo.numDirName++;
  return result;
}

// =================================== addFileName =========================== 

static uint8_t addFileName(dwarfDbgPtr_t self, char *fileName, size_t dirNameIdx) {
  uint8_t result;
  fileNameInfo_t *fileNameInfo;

  result = DWARF_DBG_ERR_OK;
  if (self->dwarfDbgDict->fileNamesInfo.maxFileName <= self->dwarfDbgDict->fileNamesInfo.numFileName) {
    self->dwarfDbgDict->fileNamesInfo.maxFileName += 50;
    if (self->dwarfDbgDict->fileNamesInfo.fileNames == NULL) {
      self->dwarfDbgDict->fileNamesInfo.fileNames = (fileNameInfo_t *)ckalloc(sizeof(fileNameInfo_t) * self->dwarfDbgDict->fileNamesInfo.maxFileName);
      if (self->dwarfDbgDict->fileNamesInfo.fileNames == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      self->dwarfDbgDict->fileNamesInfo.fileNames = (fileNameInfo_t *)ckrealloc((char *)self->dwarfDbgDict->fileNamesInfo.fileNames, sizeof(fileNameInfo_t) * self->dwarfDbgDict->fileNamesInfo.maxFileName);
      if (self->dwarfDbgDict->fileNamesInfo.fileNames == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
  fileNameInfo = &self->dwarfDbgDict->fileNamesInfo.fileNames[self->dwarfDbgDict->fileNamesInfo.numFileName];
  fileNameInfo->fileName = (char *)ckalloc(strlen(fileName) + 1);
  if (fileNameInfo->fileName == NULL) {
    return DWARF_DBG_ERR_OUT_OF_MEMORY;
  }
  fileNameInfo->fileName[strlen(fileName)] == '\0';
  memcpy(fileNameInfo->fileName, fileName, strlen(fileName));
  fileNameInfo->dirNameIdx = dirNameIdx;
printf("addFileName: num: %d dirNameIdx: %d %s\n", self->dwarfDbgDict->fileNamesInfo.numFileName, dirNameIdx, fileName);
  self->dwarfDbgDict->fileNamesInfo.numFileName++;
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
  for (i = 0; i < self->dwarfDbgDict->dirNamesInfo.numDirName; i++) {
    if (strcmp(pathName, self->dwarfDbgDict->dirNamesInfo.dirNames[i]) == 0) {
printf("found: dirName %d num: %d\n", i, self->dwarfDbgDict->dirNamesInfo.numDirName);
      dirIdx = i;
      break;
    }
  }
  if (dirIdx < 0) {
    dirIdx = self->dwarfDbgDict->dirNamesInfo.numDirName;
    result = self->dwarfDbgDict->addDirName(self, pathName);
    checkErrOK(result);
  }
  fileIdx = -1;
  for (i = 0; i < self->dwarfDbgDict->fileNamesInfo.numFileName; i++) {
    fileNameInfo = &self->dwarfDbgDict->fileNamesInfo.fileNames[i];
    if ((strcmp(cp, fileNameInfo->fileName) == 0) && (fileNameInfo->dirNameIdx == dirIdx)) {
printf("found: fileName %d num: %d\n", i, self->dwarfDbgDict->fileNamesInfo.numFileName);
      fileIdx = i;
      break;
    }
  }
  if (fileIdx < 0) {
    fileIdx = self->dwarfDbgDict->fileNamesInfo.numFileName;
    result = self->dwarfDbgDict->addFileName(self, cp, dirIdx);
    checkErrOK(result);
  }
  *fileNameIdx = fileIdx;
  result = self->dwarfDbgDict->addFileInfo(self, compileUnitIdx, fileIdx, fileInfoIdx);
  checkErrOK(result);
  return result;
}

// =================================== addFileInfo =========================== 

static uint8_t addFileInfo(dwarfDbgPtr_t self, size_t compileUnitIdx, size_t fileNameIdx, size_t *fileInfoIdx) {
  uint8_t result;
  compileUnitInfo_t *compileUnitInfo;
  fileInfo_t *fileInfo;

  result = DWARF_DBG_ERR_OK;
  compileUnitInfo = &self->dwarfDbgDict->compileUnitsInfo.compileUnitInfos[compileUnitIdx];
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

static uint8_t addFileLine(dwarfDbgPtr_t self, size_t lineNo, size_t compileUnitIdx, size_t fileInfoIdx, size_t *fileLineIdx) {
  uint8_t result;
  compileUnitInfo_t *compileUnitInfo;
  fileInfo_t *fileInfo;

  result = DWARF_DBG_ERR_OK;
  compileUnitInfo = &self->dwarfDbgDict->compileUnitsInfo.compileUnitInfos[compileUnitIdx];
  fileInfo = &compileUnitInfo->fileInfos[fileInfoIdx];
  if (fileInfo->maxFileLine <= fileInfo->numFileLine) {
    fileInfo->maxFileLine += 5;
    if (fileInfo->fileLines == NULL) {
      fileInfo->fileLines = (int *)ckalloc(sizeof(int*) * fileInfo->maxFileLine);
      if (fileInfo->fileLines == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      fileInfo->fileLines = (int *)ckrealloc((char *)fileInfo->fileLines, sizeof(int *) * fileInfo->maxFileLine);
      if (fileInfo->fileLines == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
  fileInfo->fileLines[fileInfo->numFileLine] = lineNo;
  *fileLineIdx = fileInfo->numFileLine;
  fileInfo->numFileLine++;
  return result;
}

// =================================== addCompileUnitFile =========================== 

static uint8_t addCompileUnitFile(dwarfDbgPtr_t self, char *fileName, Dwarf_Off overallOffset, size_t *compileUnitIdx) {
  uint8_t result;
  compileUnitInfo_t *compileUnitInfo;

printf("addCompileUnitFile\n");
  result = DWARF_DBG_ERR_OK;
  if (self->dwarfDbgDict->compileUnitsInfo.maxCompileUnit <= self->dwarfDbgDict->compileUnitsInfo.numCompileUnit) {
    self->dwarfDbgDict->compileUnitsInfo.maxCompileUnit += 50;
    if (self->dwarfDbgDict->compileUnitsInfo.compileUnitInfos == NULL) {
      self->dwarfDbgDict->compileUnitsInfo.compileUnitInfos = (compileUnitInfo_t *)ckalloc(sizeof(compileUnitInfo_t) * self->dwarfDbgDict->compileUnitsInfo.maxCompileUnit);
      if (self->dwarfDbgDict->compileUnitsInfo.compileUnitInfos == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      self->dwarfDbgDict->compileUnitsInfo.compileUnitInfos = (compileUnitInfo_t *)ckrealloc((char *)self->dwarfDbgDict->compileUnitsInfo.compileUnitInfos, sizeof(compileUnitInfo_t) * self->dwarfDbgDict->compileUnitsInfo.maxCompileUnit);
      if (self->dwarfDbgDict->compileUnitsInfo.compileUnitInfos == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
  compileUnitInfo = &self->dwarfDbgDict->compileUnitsInfo.compileUnitInfos[self->dwarfDbgDict->compileUnitsInfo.numCompileUnit];
  compileUnitInfo->fileName = (char *)ckalloc(strlen(fileName) + 1);
  if (compileUnitInfo->fileName == NULL) {
    return DWARF_DBG_ERR_OUT_OF_MEMORY;
  }
  compileUnitInfo->fileName[strlen(fileName)] == '\0';
  compileUnitInfo->maxFileInfo = 0;
  compileUnitInfo->numFileInfo = 0;
  compileUnitInfo->fileInfos = NULL;
  memcpy(compileUnitInfo->fileName, fileName, strlen(fileName));
  compileUnitInfo->overallOffset = overallOffset;
printf("addCompileUnit: num: %d overallOffset: %d %s\n", self->dwarfDbgDict->compileUnitsInfo.numCompileUnit, overallOffset, fileName);
  *compileUnitIdx = self->dwarfDbgDict->compileUnitsInfo.numCompileUnit;
  self->dwarfDbgDict->compileUnitsInfo.numCompileUnit++;
  return result;
}

// =================================== dwarfDbgDictInit =========================== 

int dwarfDbgDictInit (dwarfDbgPtr_t self) {
  self->dwarfDbgDict->dirNamesInfo.maxDirName = 0;
  self->dwarfDbgDict->dirNamesInfo.numDirName = 0;
  self->dwarfDbgDict->dirNamesInfo.dirNames = NULL;

  self->dwarfDbgDict->fileNamesInfo.maxFileName = 0;
  self->dwarfDbgDict->fileNamesInfo.numFileName = 0;
  self->dwarfDbgDict->fileNamesInfo.fileNames = NULL;

  self->dwarfDbgDict->compileUnitsInfo.maxCompileUnit = 0;
  self->dwarfDbgDict->compileUnitsInfo.numCompileUnit = 0;
  self->dwarfDbgDict->compileUnitsInfo.compileUnitInfos = NULL;

  self->dwarfDbgDict->addDirName = addDirName;
  self->dwarfDbgDict->addFileName = addFileName;
  self->dwarfDbgDict->addSourceFile = addSourceFile;
  self->dwarfDbgDict->addFileLine = addFileLine;
  self->dwarfDbgDict->addFileInfo = addFileInfo;
  self->dwarfDbgDict->addCompileUnitFile = addCompileUnitFile;
  return DWARF_DBG_ERR_OK;
}


