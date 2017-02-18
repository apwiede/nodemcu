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

/* *************************************************************************
 * dirName:
 *   maxDirName 5   # max free slots
 *   numDirName 3   # next free slot
 *   dirNames  char *dirName0 | char *dirName1 | char *dirnName2
 *
 * pathNameInfo:
 *   char *fileName
 *   size_t dirNameIdx
 *
 * pathNamesInfo:
 *   maxPathName 10   # max free slots
 *   numPathName 2    # next free slot
 *   pathNames  pathNameInfo0 | pathNameInfo1
 *
 * fileInfo
 *   pathNameIdx   # index into pathNames
 *   maxFileLine 50
 *   numFileLine  3
 *   fileLines     lineInfo0 | lineInfo1     # the files line number infos
 *
 * dwarfDbgGetDbgInfo
 *   maxCompileUnit 40
 *   numCompileUnit  2
 *   compileUnits    compileUnit0 | compileUnit1
 *   currCompileUnit *compileUnit1
 *
 * */

Tcl_Obj *Tcl_NewDictObj();

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

// =================================== addPathName =========================== 

static uint8_t addPathName(dwarfDbgPtr_t self, char *fileName, int dirNameIdx, int *pathNameIdx) {
  uint8_t result;
  int lgth;
  pathNameInfo_t *pathNameInfo;

  result = DWARF_DBG_ERR_OK;
  if (self->dwarfDbgFileInfo->pathNamesInfo.maxPathName <= self->dwarfDbgFileInfo->pathNamesInfo.numPathName) {
    self->dwarfDbgFileInfo->pathNamesInfo.maxPathName += 50;
    if (self->dwarfDbgFileInfo->pathNamesInfo.pathNames == NULL) {
      self->dwarfDbgFileInfo->pathNamesInfo.pathNames = (pathNameInfo_t *)ckalloc(sizeof(pathNameInfo_t) * self->dwarfDbgFileInfo->pathNamesInfo.maxPathName);
    } else {
      self->dwarfDbgFileInfo->pathNamesInfo.pathNames = (pathNameInfo_t *)ckrealloc((char *)self->dwarfDbgFileInfo->pathNamesInfo.pathNames, sizeof(pathNameInfo_t) * self->dwarfDbgFileInfo->pathNamesInfo.maxPathName);
    }
    if (self->dwarfDbgFileInfo->pathNamesInfo.pathNames == NULL) {
      return DWARF_DBG_ERR_OUT_OF_MEMORY;
    }
  }
  pathNameInfo = &self->dwarfDbgFileInfo->pathNamesInfo.pathNames[self->dwarfDbgFileInfo->pathNamesInfo.numPathName];
  lgth = strlen(fileName) + 1;
  pathNameInfo->fileName = (char *)ckalloc(lgth);
  if (pathNameInfo->fileName == NULL) {
    return DWARF_DBG_ERR_OUT_OF_MEMORY;
  }
  memset(pathNameInfo->fileName, 0, lgth);
  memcpy(pathNameInfo->fileName, fileName, lgth-1);
  pathNameInfo->dirNameIdx = dirNameIdx;
  DWARF_DBG_PRINT(self, "F", 1, "addPathName: pathNameIdx: %d dirNameIdx: %d %s\n", self->dwarfDbgFileInfo->pathNamesInfo.numPathName, dirNameIdx, fileName);
  *pathNameIdx = self->dwarfDbgFileInfo->pathNamesInfo.numPathName;
  self->dwarfDbgFileInfo->pathNamesInfo.numPathName++;
  return result;
}

// =================================== addCompileUnitFile =========================== 

static uint8_t addCompileUnitFile(dwarfDbgPtr_t self, char *pathName, int *pathNameIdx, int *fileInfoIdx) {
  uint8_t result;
  char *cp;
  int i;
  int dirIdx;
  int fileIdx;
  pathNameInfo_t *pathNameInfo;

  result = DWARF_DBG_ERR_OK;
  DWARF_DBG_PRINT(self, "F", 2, "  >>addCompileUnitFile compileUnitIdx: %d %s\n", self->dwarfDbgCompileUnitInfo->currCompileUnitIdx, pathName);
  cp = strrchr(pathName, '/');
  if (cp != NULL) {
    *cp++ = '\0';
  } else {
    cp = "";
  }
//printf("path: %s name: %s\n", pathName, cp);
  dirIdx = -1;
  for (i = 0; i < self->dwarfDbgFileInfo->dirNamesInfo.numDirName; i++) {
    if (strcmp(pathName, self->dwarfDbgFileInfo->dirNamesInfo.dirNames[i]) == 0) {
//printf("found: dirName %d num: %d\n", i, self->dwarfDbgFileInfo->dirNamesInfo.numDirName);
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
  for (i = 0; i < self->dwarfDbgFileInfo->pathNamesInfo.numPathName; i++) {
    pathNameInfo = &self->dwarfDbgFileInfo->pathNamesInfo.pathNames[i];
    if ((strcmp(cp, pathNameInfo->fileName) == 0) && (pathNameInfo->dirNameIdx == dirIdx)) {
//printf("found: fileName %d num: %d\n", i, self->dwarfDbgFileInfo->pathNamesInfo.numpathName);
      fileIdx = i;
      break;
    }
  }
  if (fileIdx < 0) {
//    fileIdx = self->dwarfDbgFileInfo->pathNamesInfo.numPathName;
    result = self->dwarfDbgFileInfo->addPathName(self, cp, dirIdx, pathNameIdx);
    checkErrOK(result);
  } else {
    *pathNameIdx = fileIdx;
  }
  result = self->dwarfDbgFileInfo->addFileInfo(self, fileIdx, fileInfoIdx);
  checkErrOK(result);
  return result;
}

// =================================== addSourceFile =========================== 

static uint8_t addSourceFile(dwarfDbgPtr_t self, char *pathName, int *pathNameIdx, int *fileInfoIdx) {
  uint8_t result;
  char *cp;
  int i;
  int dirIdx;
  int fileIdx;
  pathNameInfo_t *pathNameInfo;
  compileUnit_t *compileUnit;
  char saveCh;

//printf(">>addSourceFile: %s\n", pathName);
  result = DWARF_DBG_ERR_OK;
  cp = strrchr(pathName, '/');
  if (cp != NULL) {
    saveCh = *cp;
    *cp++ = '\0';
  }
//printf("path: %s name: %s\n", pathName, cp);
  dirIdx = -1;
  for (i = 0; i < self->dwarfDbgFileInfo->dirNamesInfo.numDirName; i++) {
    if (strcmp(pathName, self->dwarfDbgFileInfo->dirNamesInfo.dirNames[i]) == 0) {
//printf("found: dirName %d num: %d\n", i, self->dwarfDbgFileInfo->dirNamesInfo.numDirName);
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
  for (i = 0; i < self->dwarfDbgFileInfo->pathNamesInfo.numPathName; i++) {
    pathNameInfo = &self->dwarfDbgFileInfo->pathNamesInfo.pathNames[i];
    if ((strcmp(cp, pathNameInfo->fileName) == 0) && (pathNameInfo->dirNameIdx == dirIdx)) {
//printf("found: fileName %d num: %d\n", i, self->dwarfDbgFileInfo->pathNamesInfo.numpathName);
      fileIdx = i;
      break;
    }
  }
  if (fileIdx < 0) {
//    fileIdx = self->dwarfDbgFileInfo->pathNamesInfo.numPathName;
    result = self->dwarfDbgFileInfo->addPathName(self, cp, dirIdx, pathNameIdx);
    checkErrOK(result);
  } else {
    *pathNameIdx = fileIdx;
  }
  if (cp != NULL) {
    cp[-1] = saveCh;
  }

  compileUnit = self->dwarfDbgCompileUnitInfo->currCompileUnit;
  if (compileUnit->maxSourceFile <= compileUnit->numSourceFile) {
    compileUnit->maxSourceFile += 10;
    if (compileUnit->sourceFiles == NULL) {
      compileUnit->sourceFiles = (int *)ckalloc(sizeof(int *) * compileUnit->maxSourceFile);
      if (compileUnit->sourceFiles == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
      compileUnit->sourceFiles[0] = 0; // just to initialize
      compileUnit->numSourceFile = 1; // needed because slot 0 is empty in dwarf info!!
    } else {
      compileUnit->sourceFiles = (int *)ckrealloc((char *)compileUnit->sourceFiles, sizeof(int *) * compileUnit->maxSourceFile);
      if (compileUnit->sourceFiles == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
  compileUnit->sourceFiles[compileUnit->numSourceFile] = *pathNameIdx;
//printf(">>sourceFiles idx: %d\n", compileUnit->numSourceFile);
  compileUnit->numSourceFile++;
  return result;
}

// =================================== addFileInfo =========================== 

static uint8_t addFileInfo(dwarfDbgPtr_t self, int pathNameIdx, int *fileInfoIdx) {
  uint8_t result;
  compileUnit_t *compileUnit;
  fileInfo_t *fileInfo;

  result = DWARF_DBG_ERR_OK;
  compileUnit = self->dwarfDbgCompileUnitInfo->currCompileUnit;
  if (compileUnit->maxFileInfo <= compileUnit->numFileInfo) {
    compileUnit->maxFileInfo += 10;
    if (compileUnit->fileInfos == NULL) {
      compileUnit->fileInfos = (fileInfo_t *)ckalloc(sizeof(fileInfo_t) * compileUnit->maxFileInfo);
      if (compileUnit->fileInfos == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      compileUnit->fileInfos = (fileInfo_t *)ckrealloc((char *)compileUnit->fileInfos, sizeof(fileInfo_t) * compileUnit->maxFileInfo);
      if (compileUnit->fileInfos == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
  fileInfo = &compileUnit->fileInfos[compileUnit->numFileInfo];
  fileInfo->maxFileLine = 0;
  fileInfo->numFileLine = 0;
  fileInfo->fileLines = NULL;
  fileInfo->pathNameIdx = pathNameIdx;
  *fileInfoIdx = compileUnit->numFileInfo;
//printf("addFileInfo: pathNameIdx: %d fileInfoIdx: %d\n", pathNameIdx, *fileInfoIdx);
  compileUnit->numFileInfo++;
  return result;
}

// =================================== getPathNameIdxFromFileName =========================== 

static uint8_t getPathNameIdxFromFileName(dwarfDbgPtr_t self, const char *pathName, int *pathNameIdx) {
  uint8_t result;
  compileUnit_t *compileUnit;
  pathNameInfo_t *pathNameInfo;
  int fileIdx;

  *pathNameIdx = -1;
  result = DWARF_DBG_ERR_OK;
  compileUnit = self->dwarfDbgCompileUnitInfo->currCompileUnit;
  for (fileIdx = 0; fileIdx < self->dwarfDbgFileInfo->pathNamesInfo.numPathName; fileIdx++) {
    pathNameInfo = &self->dwarfDbgFileInfo->pathNamesInfo.pathNames[fileIdx];
    if (strcmp(pathName, pathNameInfo->fileName) == 0) {
      DWARF_DBG_PRINT(self, "F", 1, "found: fileName %s %d pathNameIdx: %d\n", pathName, fileIdx);
      *pathNameIdx = fileIdx;
      break;
    }
  }
  return result;
}

// =================================== getFileNameFromPathNameIdx =========================== 

static uint8_t getFileNameFromPathNameIdx(dwarfDbgPtr_t self, int pathNameIdx, const char **pathName) {
  uint8_t result;
  compileUnit_t *compileUnit;
  pathNameInfo_t *pathNameInfo;

  *pathName = NULL;
  result = DWARF_DBG_ERR_OK;
  compileUnit = self->dwarfDbgCompileUnitInfo->currCompileUnit;
  if ((pathNameIdx >= 0) && (pathNameIdx < self->dwarfDbgFileInfo->pathNamesInfo.numPathName)) {
    pathNameInfo = &self->dwarfDbgFileInfo->pathNamesInfo.pathNames[pathNameIdx];
    *pathName = pathNameInfo->fileName;
    DWARF_DBG_PRINT(self, "F", 1, "found: fileName %s %d pathName: %d\n", *pathName, pathNameIdx);
  }
  return result;
}

// =================================== dwarfDbgGetFileInfos =========================== 

int dwarfDbgGetFileInfos(dwarfDbgPtr_t self) {
  uint8_t result;
  int tclResult;
  Tcl_Obj *listPtr1;
  Tcl_Obj *listPtr2;
  Tcl_Obj *objPtr1;
  Tcl_Obj *objPtr2;
  Tcl_Obj *objPtr3;
  Tcl_Obj *objPtr4;
  Tcl_Obj *objPtr5;
  Tcl_Obj *objPtr6;
  size_t idx;
  compileUnit_t *compileUnit;
  fileInfo_t *fileInfo;
  pathNameInfo_t *pathNameInfo;
  char *dirName;

//printf("dwarfDbgGetFileInfos self: %p numCompileUnit: %d\n", self, self->dwarfDbgGetDbgInfo->numCompileUnit);
  result = DWARF_DBG_ERR_OK;
  // make a Tcl list of all compile unit file names
  listPtr1 = Tcl_NewListObj(0, NULL);
  DWARF_DBG_PRINT(self, "F", 1, "listPtr: %p\n", listPtr1);
  for (idx = 0; idx < self->dwarfDbgCompileUnitInfo->numCompileUnit; idx++) {
    listPtr2 = Tcl_NewListObj(0, NULL);
    compileUnit = &self->dwarfDbgCompileUnitInfo->compileUnits[idx];
    pathNameInfo = &self->dwarfDbgFileInfo->pathNamesInfo.pathNames[compileUnit->pathNameIdx];
    dirName = self->dwarfDbgFileInfo->dirNamesInfo.dirNames[pathNameInfo->dirNameIdx];
    objPtr1 = Tcl_NewStringObj(compileUnit->shortFileName, -1);
    tclResult = Tcl_ListObjAppendElement(self->interp, listPtr2, objPtr1);
    objPtr2 = Tcl_NewIntObj(idx);
    tclResult = Tcl_ListObjAppendElement(self->interp, listPtr2, objPtr2);
    objPtr3 = Tcl_NewIntObj(compileUnit->pathNameIdx);
    tclResult = Tcl_ListObjAppendElement(self->interp, listPtr2, objPtr3);
    objPtr4 = Tcl_NewIntObj(compileUnit->numFileInfo - 1);
    tclResult = Tcl_ListObjAppendElement(self->interp, listPtr2, objPtr4);
    if (compileUnit->numFileInfo > 0) {
      fileInfo = &compileUnit->fileInfos[0];
      objPtr5 = Tcl_NewIntObj(fileInfo->numFileLine - 1);
    } else {
      objPtr5 = Tcl_NewIntObj(0);
    }
    tclResult = Tcl_ListObjAppendElement(self->interp, listPtr2, objPtr5);
    objPtr6 = Tcl_NewStringObj(dirName, - 1);
    tclResult = Tcl_ListObjAppendElement(self->interp, listPtr2, objPtr6);
    tclResult = Tcl_ListObjAppendElement(self->interp, listPtr1, listPtr2);
  }
  Tcl_SetObjResult(self->interp, listPtr1);
  return result;
}

// =================================== dwarfDbgGetFileLines =========================== 

int dwarfDbgGetFileLines(dwarfDbgPtr_t self, int compileUnitIdx) {
  uint8_t result;
  int tclResult;
  Tcl_Obj *dictPtr;
  Tcl_Obj *dictPtr1;
  Tcl_Obj *dictPtr2;
  Tcl_Obj *dictPtr3;
  Tcl_Obj *dictPtr4;
  Tcl_Obj *objPtr1;
  Tcl_Obj *objPtr2;
  Tcl_Obj *objPtr3;
  Tcl_Obj *objPtr4;
  Tcl_Obj *objPtr5;
  Tcl_Obj *objPtr6;
  Tcl_Obj *objPtr7;
  Tcl_Obj *objPtr8;
  Tcl_Obj *objPtr9;
  size_t idx;
  compileUnit_t *compileUnit;
  fileInfo_t *fileInfo;
  lineInfo_t *lineInfo;

//printf("dwarfDbgGetFileLines compileUnitIdx: %d\n", compileUnitIdx);
  result = DWARF_DBG_ERR_OK;
  // make a Tcl dict of all all lines and addresses for a compile unit file name
  compileUnit = &self->dwarfDbgCompileUnitInfo->compileUnits[compileUnitIdx];
  dictPtr = Tcl_NewDictObj();
  dictPtr1 = Tcl_NewDictObj();
  dictPtr2 = Tcl_NewDictObj();
  if (compileUnit->numFileInfo > 0) {
    fileInfo = &compileUnit->fileInfos[0];
//printf("numFileLine: %d\n", fileInfo->numFileLine);
    for (idx = 0; idx < fileInfo->numFileLine; idx++) {
      lineInfo = &fileInfo->fileLines[idx];
//printf("idx: %d pc: 0x%08x lineNo: %d\n", idx, lineInfo->pc, lineInfo->lineNo);
      dictPtr3 = Tcl_NewDictObj();
      dictPtr4 = Tcl_NewDictObj();
      objPtr1 = Tcl_NewIntObj(lineInfo->lineNo);
      objPtr2 = Tcl_NewIntObj(lineInfo->pc);
      objPtr5 = Tcl_NewIntObj(compileUnitIdx);
      objPtr6 = Tcl_NewStringObj(compileUnit->shortFileName, -1);

      objPtr7 = Tcl_NewStringObj("addr", -1);
      objPtr8 = Tcl_NewStringObj("cu", -1);
      objPtr9 = Tcl_NewStringObj("line", -1);

      tclResult = Tcl_DictObjPut(self->interp, dictPtr3, objPtr7, objPtr2);
      tclResult = Tcl_DictObjPut(self->interp, dictPtr3, objPtr8, objPtr5);
      tclResult = Tcl_DictObjPut(self->interp, dictPtr1, objPtr1, dictPtr3);

      tclResult = Tcl_DictObjPut(self->interp, dictPtr4, objPtr9, objPtr1);
      tclResult = Tcl_DictObjPut(self->interp, dictPtr4, objPtr8, objPtr5);
      tclResult = Tcl_DictObjPut(self->interp, dictPtr2, objPtr2, dictPtr4);
    }
    objPtr3 = Tcl_NewStringObj("lines", -1);
    objPtr4 = Tcl_NewStringObj("addresses", -1);
    tclResult = Tcl_DictObjPut(self->interp, dictPtr, objPtr3, dictPtr1);
    tclResult = Tcl_DictObjPut(self->interp, dictPtr, objPtr4, dictPtr2);
  } else {
  DWARF_DBG_PRINT(self, "F", 1, "no fileInfo for %d\n", idx);
    return DWARF_DBG_ERR_NO_FILE_LINES;
  }
  Tcl_SetObjResult(self->interp, dictPtr);
  return result;
}

// =================================== dwarfDbgFileInfoInit =========================== 

int dwarfDbgFileInfoInit (dwarfDbgPtr_t self) {
  self->dwarfDbgFileInfo->dirNamesInfo.maxDirName = 0;
  self->dwarfDbgFileInfo->dirNamesInfo.numDirName = 0;
  self->dwarfDbgFileInfo->dirNamesInfo.dirNames = NULL;

  self->dwarfDbgFileInfo->pathNamesInfo.maxPathName = 0;
  self->dwarfDbgFileInfo->pathNamesInfo.numPathName = 0;
  self->dwarfDbgFileInfo->pathNamesInfo.pathNames = NULL;

  self->dwarfDbgFileInfo->addDirName = &addDirName;
  self->dwarfDbgFileInfo->addPathName = &addPathName;
  self->dwarfDbgFileInfo->addCompileUnitFile = &addCompileUnitFile;
  self->dwarfDbgFileInfo->addSourceFile = &addSourceFile;
  self->dwarfDbgFileInfo->addFileInfo = &addFileInfo;
  self->dwarfDbgFileInfo->getPathNameIdxFromFileName = &getPathNameIdxFromFileName;
  self->dwarfDbgFileInfo->getFileNameFromPathNameIdx = &getFileNameFromPathNameIdx;
  return DWARF_DBG_ERR_OK;
}
