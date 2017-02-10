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
 * File:   dwarfDbgDieInfo.c
 * Author: Arnulf P. Wiedemann <arnulf@wiedemann-pri.de>
 *
 * Created on February 04, 2017
 */

#include <tcl.h>
#include "dwarfDbgInt.h"

FILE *showFd = NULL;

// =================================== showDieEntries =========================== 

static uint8_t showDieEntries(dwarfDbgPtr_t self, size_t dieAndChildrenIdx, Dwarf_Bool isSibling, const char *indent) {
  int result;
  int entryIdx = 0;
  int maxEntries = 0;
  int attrIdx = 0;
  int sres = 0;
  const char *tagStringValue;
  const char *attrStringValue;
  const char *formStringValue;
  dieAndChildrenInfo_t *dieAndChildrenInfo;
  compileUnitInfo_t *compileUnitInfo;
  dieInfo_t *dieInfo;
  dieAttr_t *attrInfo;
  Dwarf_Error err;
  char *temps;
  char buf[255];

  result = DWARF_DBG_ERR_OK;
  compileUnitInfo = &self->dwarfDbgGetDbgInfo->compileUnits[self->dwarfDbgGetDbgInfo->currCompileUnitIdx].compileUnitInfo;
  dieAndChildrenInfo = &compileUnitInfo->dieAndChildrenInfo[dieAndChildrenIdx];
  if (isSibling) {
    maxEntries = dieAndChildrenInfo->numSiblings;
  } else {
    maxEntries = dieAndChildrenInfo->numChildren;
  }
fprintf(showFd, "++ numEntries: %d\n", maxEntries);
  for(entryIdx = 0; entryIdx < maxEntries; entryIdx++) {
    if (isSibling) {
      dieInfo = &dieAndChildrenInfo->dieSiblings[entryIdx];
    } else {
      dieInfo = &dieAndChildrenInfo->dieChildren[entryIdx];
    }
tagStringValue = NULL;
    result = self->dwarfDbgStringInfo->getDW_TAG_string(self, dieInfo->tag, &tagStringValue);
    checkErrOK(result);
    fprintf(showFd, "%s%s: %04d offset: 0x%08x tag: 0x%04x numAttr: %d\n", indent, tagStringValue, entryIdx, dieInfo->offset, dieInfo->tag, dieInfo->numAttr);
    for(attrIdx = 0; attrIdx < dieInfo->numAttr; attrIdx++) {
      attrInfo = &dieInfo->dieAttrs[attrIdx];
      attrStringValue = NULL;
      result = self->dwarfDbgStringInfo->getDW_AT_string(self, attrInfo->attr, &attrStringValue);
      checkErrOK(result);
      result = self->dwarfDbgStringInfo->getDW_FORM_string(self, attrInfo->theform, &formStringValue);
      checkErrOK(result);
sres = dwarf_formstring(attrInfo->attr_in, &temps, &err);
if (sres == DW_DLV_OK) {
sprintf(buf, "%s", temps);
}
switch (attrInfo->attr) {
case DW_AT_call_file:
case DW_AT_decl_file:
{
  int fileIdx;
  fileNameInfo_t *fileNameInfo;
  dirNamesInfo_t *dirNamesInfo;
  int compileUnitIdx = self->dwarfDbgGetDbgInfo->currCompileUnitIdx;
  int j;

  sres = DW_DLV_OK;
  fileIdx = compileUnitInfo->sourceFiles[attrInfo->uval];
  if (fileIdx < self->dwarfDbgFileInfo->fileNamesInfo.numFileName) {
    fileNameInfo = &self->dwarfDbgFileInfo->fileNamesInfo.fileNames[fileIdx];
    dirNamesInfo = &self->dwarfDbgFileInfo->dirNamesInfo;
    sprintf(buf, "%s/%s", dirNamesInfo->dirNames[fileNameInfo->dirNameIdx], fileNameInfo->fileName); 
  } else {
    sres = DW_DLV_ERROR;
  }
}
  break;
case DW_AT_encoding:
  formStringValue = NULL;
  result = self->dwarfDbgStringInfo->getDW_ATE_string(self, attrInfo->uval, &formStringValue);
  checkErrOK(result);
  sres = DW_DLV_OK;
  sprintf(buf, "%s", formStringValue); 
  break;
}
if (sres == DW_DLV_OK) {
      fprintf(showFd, "%s  %s: attr_in: 0x%08x theform: 0x%04x %s uval: 0x%08x %s\n", indent, attrStringValue, attrInfo->attr_in, attrInfo->theform, formStringValue, attrInfo->uval, buf);
} else {
      fprintf(showFd, "%s  %s: attr_in: 0x%08x theform: 0x%04x %s uval: 0x%08x\n", indent, attrStringValue, attrInfo->attr_in, attrInfo->theform, formStringValue, attrInfo->uval);
}
    }
  }
  return result;
}

// =================================== addAttrStr =========================== 

static uint8_t addAttrStr(dwarfDbgPtr_t self, const char *str, int *attrStrIdx) {
  uint8_t result;
  char **attrStr;
  int strIdx;

  result = DWARF_DBG_ERR_OK;
  for(strIdx = 0; strIdx < self->dwarfDbgGetDbgInfo->numAttrStr; strIdx++) {
    if (strcmp(self->dwarfDbgGetDbgInfo->attrStrs[strIdx], str) == 0) {
      *attrStrIdx = strIdx;
      return result;
    }
  }
  if (self->dwarfDbgGetDbgInfo->maxAttrStr <= self->dwarfDbgGetDbgInfo->numAttrStr) {
    self->dwarfDbgGetDbgInfo->maxAttrStr += 10;
    if (self->dwarfDbgGetDbgInfo->attrStrs == NULL) {
      self->dwarfDbgGetDbgInfo->attrStrs = (char **)ckalloc(sizeof(char *) * self->dwarfDbgGetDbgInfo->maxAttrStr);
      if (self->dwarfDbgGetDbgInfo->attrStrs == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      self->dwarfDbgGetDbgInfo->attrStrs = (char **)ckrealloc((char *)self->dwarfDbgGetDbgInfo->attrStrs, sizeof(char *) * self->dwarfDbgGetDbgInfo->maxAttrStr);
      if (self->dwarfDbgGetDbgInfo->attrStrs == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
//printf("== numAttrStrs: %d %s\n", self->dwarfDbgGetDbgInfo->numAttrStr, str);
  attrStr = &self->dwarfDbgGetDbgInfo->attrStrs[self->dwarfDbgGetDbgInfo->numAttrStr];
  *attrStr = ckalloc(strlen(str) + 1);
  memset(*attrStr, 0, strlen(str) + 1);
  memcpy(*attrStr, str, strlen(str));
  *attrStrIdx = self->dwarfDbgGetDbgInfo->numAttrStr;
  self->dwarfDbgGetDbgInfo->numAttrStr++;
  return result;
}

// =================================== addDieAttr =========================== 

static uint8_t addDieAttr(dwarfDbgPtr_t self, size_t dieAndChildrenIdx, Dwarf_Bool isSibling, size_t idx, Dwarf_Half attr, Dwarf_Attribute attr_in, const char *attrStr, const char *sourceFile, int sourceLineNo, Dwarf_Unsigned uval, Dwarf_Half theform, Dwarf_Half directform, uint16_t flags, size_t *attrIdx) {
  uint8_t result;
  const char *stringValue;
  dieAndChildrenInfo_t *dieAndChildrenInfo;
  dieInfo_t *dieInfo;
  dieAttr_t *dieAttr;
  compileUnitInfo_t *compileUnitInfo;
  fileInfo_t *fileInfo;
  int attrStrIdx = -1;

  result = DWARF_DBG_ERR_OK;
  // next line no longer needed??
  result = self->dwarfDbgStringInfo->getDW_AT_string(self, attr, &stringValue);
  checkErrOK(result);
printf("== addDieAttr: %s dieAndChildrenIdx: %d isSibling: %d idx: %d attr: 0x%08x attr_in: 0x%08x uval: 0x%08x, theform: 0x%04x\n", stringValue, dieAndChildrenIdx, isSibling, idx, attr, attr_in, uval, theform);
  compileUnitInfo = &self->dwarfDbgGetDbgInfo->compileUnits[self->dwarfDbgGetDbgInfo->currCompileUnitIdx].compileUnitInfo;
  dieAndChildrenInfo = &compileUnitInfo->dieAndChildrenInfo[dieAndChildrenIdx];
  if (isSibling) {
    dieInfo = &dieAndChildrenInfo->dieSiblings[idx];
  } else {
    dieInfo = &dieAndChildrenInfo->dieChildren[idx];
  }
  if (dieInfo->maxAttr <= dieInfo->numAttr) {
    dieInfo->maxAttr += 10;
    if (dieInfo->dieAttrs == NULL) {
      dieInfo->dieAttrs = (dieAttr_t *)ckalloc(sizeof(dieAttr_t) * dieInfo->maxAttr);
      if (dieInfo->dieAttrs == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      dieInfo->dieAttrs = (dieAttr_t *)ckrealloc((char *)dieInfo->dieAttrs, sizeof(dieAttr_t) * dieInfo->maxAttr);
      if (dieInfo->dieAttrs == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
//printf("== numAttrs: %d\n", dieInfo->numAttr);
  if (flags & DW_NAME_INFO) {
    result = addAttrStr(self, attrStr, &attrStrIdx);
    checkErrOK(result);
  }
  dieAttr = &dieInfo->dieAttrs[dieInfo->numAttr];
  memset(dieAttr, 0, sizeof(dieAttr_t));
  dieAttr->attr = attr;
  dieAttr->attr_in = attr_in;
  dieAttr->uval = uval;
  dieAttr->theform = theform;
  dieAttr->directform = directform;
  if (sourceFile != NULL) {
    result = self->dwarfDbgFileInfo->getFileIdxFromFileName(self, sourceFile, &dieAttr->sourceFileIdx);
    checkErrOK(result);
  } else {
    dieAttr->sourceFileIdx = -1;
  }
  dieAttr->sourceLineNo = sourceLineNo;
  dieAttr->flags = flags;
  dieAttr->attrStrIdx = attrStrIdx;
  if (flags & DW_LOCATION_INFO) {
    dieAttr->locationInfo = (locationInfo_t *)ckalloc(sizeof(locationInfo_t));
    memset(dieAttr->locationInfo, 0, sizeof(locationInfo_t));
  } else {
    dieAttr->locationInfo = NULL;
  }
  *attrIdx = dieInfo->numAttr;
  dieInfo->numAttr++;
  return result;
}

// =================================== showSiblings =========================== 

static uint8_t showSiblings(dwarfDbgPtr_t self, size_t dieAndChildrenIdx, const char *indent) {
  return showDieEntries(self, dieAndChildrenIdx, /* isSibling */ 1, indent);
}

// =================================== showChildren =========================== 

static uint8_t showChildren(dwarfDbgPtr_t self, size_t dieAndChildrenIdx, const char *indent) {
  return showDieEntries(self, dieAndChildrenIdx, /* isSibling */ 0, indent);
}

// =================================== addDieChildAttr =========================== 

static uint8_t addDieChildAttr(dwarfDbgPtr_t self, size_t dieAndChildrenIdx, size_t childIdx, Dwarf_Half attr, Dwarf_Attribute attr_in, const char *attrStr, const char *sourceFile, int sourceLineNo, Dwarf_Unsigned uval, Dwarf_Half theform, Dwarf_Half directform, uint16_t flags, size_t *childAttrIdx) {
  return addDieAttr(self, dieAndChildrenIdx, /* isSibling */ 0, childIdx, attr, attr_in, attrStr, sourceFile, sourceLineNo, uval, theform, directform, flags, childAttrIdx);
}

// =================================== addDieSiblingAttr =========================== 

static uint8_t addDieSiblingAttr(dwarfDbgPtr_t self, size_t dieAndChildrenIdx, size_t siblingIdx, Dwarf_Half attr, Dwarf_Attribute attr_in, const char *attrStr, const char *sourceFile, int sourceLineNo, Dwarf_Unsigned uval, Dwarf_Half theform, Dwarf_Half directform, uint16_t flags, size_t *siblingAttrIdx) {
  return addDieAttr(self, dieAndChildrenIdx, /* isSibling */ 1, siblingIdx, attr, attr_in, attrStr, sourceFile, sourceLineNo, uval, theform, directform, flags, siblingAttrIdx);
}

// =================================== addDieSibling =========================== 

static uint8_t addDieSibling(dwarfDbgPtr_t self, size_t dieAndChildrenIdx, size_t offset, Dwarf_Half tag, size_t *siblingIdx) {
  uint8_t result;
  dieAndChildrenInfo_t *dieAndChildrenInfo;
  dieInfo_t *dieInfo;
  compileUnitInfo_t *compileUnitInfo;

  result = DWARF_DBG_ERR_OK;
printf("== addDieSibling: offset: 0x%04x tag: 0x%04x\n", offset, tag);
  compileUnitInfo = &self->dwarfDbgGetDbgInfo->compileUnits[self->dwarfDbgGetDbgInfo->currCompileUnitIdx].compileUnitInfo;
  dieAndChildrenInfo = &compileUnitInfo->dieAndChildrenInfo[dieAndChildrenIdx];
  if (dieAndChildrenInfo->maxSiblings <= dieAndChildrenInfo->numSiblings) {
    dieAndChildrenInfo->maxSiblings += 10;
    if (dieAndChildrenInfo->dieSiblings == NULL) {
      dieAndChildrenInfo->dieSiblings = (dieInfo_t *)ckalloc(sizeof(dieInfo_t) * dieAndChildrenInfo->maxSiblings);
      if (dieAndChildrenInfo->dieSiblings == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      dieAndChildrenInfo->dieSiblings = (dieInfo_t *)ckrealloc((char *)dieAndChildrenInfo->dieSiblings, sizeof(dieInfo_t) * dieAndChildrenInfo->maxSiblings);
      if (dieAndChildrenInfo->dieSiblings == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
printf("== numSiblings: %d\n", dieAndChildrenInfo->numSiblings);
  dieInfo = &dieAndChildrenInfo->dieSiblings[dieAndChildrenInfo->numSiblings];
  memset(dieInfo, 0, sizeof(dieInfo_t));
  dieInfo->offset = offset;
  dieInfo->tag = tag;
  *siblingIdx = dieAndChildrenInfo->numSiblings;
  dieAndChildrenInfo->numSiblings++;
  return result;
}

// =================================== addDieChild =========================== 

static uint8_t addDieChild(dwarfDbgPtr_t self, size_t dieAndChildrenIdx, size_t offset, Dwarf_Half tag, size_t *childIdx) {
  uint8_t result;
  dieAndChildrenInfo_t *dieAndChildrenInfo;
  dieInfo_t *dieInfo;
  compileUnitInfo_t *compileUnitInfo;

  result = DWARF_DBG_ERR_OK;
printf("== addDieChild: offset: 0x%04x tag: 0x%04x\n", offset, tag);
  compileUnitInfo = &self->dwarfDbgGetDbgInfo->compileUnits[self->dwarfDbgGetDbgInfo->currCompileUnitIdx].compileUnitInfo;
  dieAndChildrenInfo = &compileUnitInfo->dieAndChildrenInfo[dieAndChildrenIdx];
  if (dieAndChildrenInfo->maxChildren <= dieAndChildrenInfo->numChildren) {
    dieAndChildrenInfo->maxChildren += 10;
    if (dieAndChildrenInfo->dieChildren == NULL) {
      dieAndChildrenInfo->dieChildren = (dieInfo_t *)ckalloc(sizeof(dieInfo_t) * dieAndChildrenInfo->maxChildren);
      if (dieAndChildrenInfo->dieChildren == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      dieAndChildrenInfo->dieChildren = (dieInfo_t *)ckrealloc((char *)dieAndChildrenInfo->dieChildren, sizeof(dieInfo_t) * dieAndChildrenInfo->maxChildren);
      if (dieAndChildrenInfo->dieChildren == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
printf("== numChildren: %d\n", dieAndChildrenInfo->numChildren);
  dieInfo = &dieAndChildrenInfo->dieChildren[dieAndChildrenInfo->numChildren];
  memset(dieInfo, 0, sizeof(dieInfo_t));
  dieInfo->offset = offset;
  dieInfo->tag = tag;
  *childIdx = dieAndChildrenInfo->numChildren;
  dieAndChildrenInfo->numChildren++;
  return result;
}

// =================================== addDieAndChildren =========================== 

static uint8_t addDieAndChildren(dwarfDbgPtr_t self, Dwarf_Die die, size_t *dieAndChildrenIdx) {
  uint8_t result;
  dieAndChildrenInfo_t *dieAndChildrenInfo;
  compileUnitInfo_t *compileUnitInfo;

  result = DWARF_DBG_ERR_OK;
printf("== addDieAndChildren: die: 0x%08x\n", die);
  compileUnitInfo = &self->dwarfDbgGetDbgInfo->compileUnits[self->dwarfDbgGetDbgInfo->currCompileUnitIdx].compileUnitInfo;
  if (compileUnitInfo->maxDieAndChildren <= compileUnitInfo->numDieAndChildren) {
    compileUnitInfo->maxDieAndChildren += 10;
    if (compileUnitInfo->dieAndChildrenInfo == NULL) {
      compileUnitInfo->dieAndChildrenInfo = (dieAndChildrenInfo_t *)ckalloc(sizeof(dieAndChildrenInfo_t) * compileUnitInfo->maxDieAndChildren);
      if (compileUnitInfo->dieAndChildrenInfo == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      compileUnitInfo->dieAndChildrenInfo = (dieAndChildrenInfo_t *)ckrealloc((char *)compileUnitInfo->dieAndChildrenInfo, sizeof(dieAndChildrenInfo_t) * compileUnitInfo->maxDieAndChildren);
      if (compileUnitInfo->dieAndChildrenInfo == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
printf("== numDieAndChildren: %d\n", compileUnitInfo->numDieAndChildren);
  dieAndChildrenInfo = &compileUnitInfo->dieAndChildrenInfo[compileUnitInfo->numDieAndChildren];
  memset(dieAndChildrenInfo, 0, sizeof(dieAndChildrenInfo_t));
  *dieAndChildrenIdx = compileUnitInfo->numDieAndChildren;
  compileUnitInfo->numDieAndChildren++;
  return result;
}

// =================================== dwarfDbgDieInfoInit =========================== 

int dwarfDbgDieInfoInit (dwarfDbgPtr_t self) {

  showFd = stdout;
//  self->dwarfDbgDieInfo->maxDieAndChildren = 0;
//  self->dwarfDbgDieInfo->numDieAndChildren = 0;
//  self->dwarfDbgDieInfo->dieAndChildren = NULL;

  self->dwarfDbgDieInfo->showSiblings = &showSiblings;
  self->dwarfDbgDieInfo->showChildren = &showChildren;
  self->dwarfDbgDieInfo->addAttrStr = &addAttrStr;
  self->dwarfDbgDieInfo->addDieSiblingAttr = &addDieSiblingAttr;
  self->dwarfDbgDieInfo->addDieChildAttr = &addDieChildAttr;
  self->dwarfDbgDieInfo->addDieSibling = &addDieSibling;
  self->dwarfDbgDieInfo->addDieChild = &addDieChild;
  self->dwarfDbgDieInfo->addDieAndChildren = &addDieAndChildren;
  return DWARF_DBG_ERR_OK;
}
