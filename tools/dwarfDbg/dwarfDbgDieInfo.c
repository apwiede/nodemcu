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

// =================================== showSiblings =========================== 

static uint8_t showSiblings(dwarfDbgPtr_t self, size_t dieAndChildrenIdx, const char *indent) {
  int result;
  int siblingIdx = 0;
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
fprintf(showFd, "++ numSiblings: %d\n", dieAndChildrenInfo->numSiblings);
  for(siblingIdx = 0; siblingIdx < dieAndChildrenInfo->numSiblings; siblingIdx++) {
    dieInfo = &dieAndChildrenInfo->dieSiblings[siblingIdx];
tagStringValue = NULL;
    result = self->dwarfDbgStringInfo->getDW_TAG_string(self, dieInfo->tag, &tagStringValue);
    checkErrOK(result);
    fprintf(showFd, "%s%s: %04d offset: 0x%08x tag: 0x%04x numAttr: %d\n", indent, tagStringValue, siblingIdx, dieInfo->offset, dieInfo->tag, dieInfo->numAttr);
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
fprintf(showFd, "%s  %s: attr_in: 0x%08x theform: 0x%04x %s uval: 0x%08x\n", indent, attrStringValue, attrInfo->attr_in, attrInfo->theform, formStringValue, attrInfo->uval);
for (j = 0; j < compileUnitInfo->numSourceFile; j++) {
//printf("j: %d sourceFile: %d\n", j, compileUnitInfo->sourceFiles[j]);
}
  fileIdx = compileUnitInfo->sourceFiles[attrInfo->uval];
printf("numFile: %d fileIdx: %d compileUnitIdx: %d %d\n", self->dwarfDbgFileInfo->fileNamesInfo.numFileName, fileIdx, compileUnitIdx, self->dwarfDbgGetDbgInfo->currCompileUnitIdx);
  if (fileIdx < compileUnitInfo->numSourceFile) {
    fileNameInfo = &self->dwarfDbgFileInfo->fileNamesInfo.fileNames[fileIdx];
    dirNamesInfo = &self->dwarfDbgFileInfo->dirNamesInfo;
printf("dirNameIdx: %d fileName: %p\n", fileNameInfo->dirNameIdx, fileNameInfo->fileName);
    sprintf(buf, "%s/%s", dirNamesInfo->dirNames[fileNameInfo->dirNameIdx], fileNameInfo->fileName); 
  } else {
    sres = DW_DLV_ERROR;
  }
}
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

// =================================== showChildren =========================== 

static uint8_t showChildren(dwarfDbgPtr_t self, size_t dieAndChildrenIdx, const char *indent) {
  int result;
  int childIdx = 0;
  int attrIdx = 0;
  int sres = 0;
  const char *tagStringValue;
  const char *attrStringValue;
  const char *formStringValue;
  dieAndChildrenInfo_t *dieAndChildrenInfo;
  dieInfo_t *dieInfo;
  dieAttr_t *attrInfo;
  compileUnitInfo_t *compileUnitInfo;
  Dwarf_Error err;
  char *temps;

  result = DWARF_DBG_ERR_OK;
  compileUnitInfo = &self->dwarfDbgGetDbgInfo->compileUnits[self->dwarfDbgGetDbgInfo->currCompileUnitIdx].compileUnitInfo;
  dieAndChildrenInfo = &compileUnitInfo->dieAndChildrenInfo[dieAndChildrenIdx];
fprintf(showFd, "++ numChildren: %d\n", dieAndChildrenInfo->numChildren);
  for(childIdx = 0; childIdx < dieAndChildrenInfo->numChildren; childIdx++) {
    dieInfo = &dieAndChildrenInfo->dieChildren[childIdx];
tagStringValue = NULL;
    result = self->dwarfDbgStringInfo->getDW_TAG_string(self, dieInfo->tag, &tagStringValue);
    checkErrOK(result);
    fprintf(showFd, "%s%s: %04d offset: 0x%08x tag: 0x%04x numAttr: %d\n", indent, tagStringValue, childIdx, dieInfo->offset, dieInfo->tag, dieInfo->numAttr);
    for(attrIdx = 0; attrIdx < dieInfo->numAttr; attrIdx++) {
      attrInfo = &dieInfo->dieAttrs[attrIdx];
      attrStringValue = NULL;
      result = self->dwarfDbgStringInfo->getDW_AT_string(self, attrInfo->attr, &attrStringValue);
      checkErrOK(result);
      result = self->dwarfDbgStringInfo->getDW_FORM_string(self, attrInfo->theform, &formStringValue);
      checkErrOK(result);
sres = dwarf_formstring(attrInfo->attr_in, &temps, &err);
if (sres == DW_DLV_OK) {
      fprintf(showFd, "%s  %s: attr_in: 0x%08x theform: 0x%04x %s uval: 0x%08x %s\n", indent, attrStringValue, attrInfo->attr_in, attrInfo->theform, formStringValue, attrInfo->uval, temps);
} else {
      fprintf(showFd, "%s  %s: attr_in: 0x%08x theform: 0x%04x %s uval: 0x%08x\n", indent, attrStringValue, attrInfo->attr_in, attrInfo->theform, formStringValue, attrInfo->uval);
}
    }
  }
  return result;
}

// =================================== addDieChildAttr =========================== 

static uint8_t addDieChildAttr(dwarfDbgPtr_t self, size_t dieAndChildrenIdx, size_t childIdx, Dwarf_Half attr, Dwarf_Attribute attr_in, Dwarf_Unsigned uval, Dwarf_Half theform, Dwarf_Half directform, uint16_t flags, size_t *childAttrIdx) {
  uint8_t result;
  const char *stringValue;
  dieAndChildrenInfo_t *dieAndChildrenInfo;
  compileUnitInfo_t *compileUnitInfo;
  dieInfo_t *dieInfo;
  dieAttr_t *dieAttr;

  result = DWARF_DBG_ERR_OK;
  result = self->dwarfDbgStringInfo->getDW_AT_string(self, attr, &stringValue);
  checkErrOK(result);
printf("== addDieChildAttr: %s dieAndChildrenIdx: %d childIdx: %d attr: 0x%08x attr_in: 0x%08x uval: 0x%08x, theform: 0x%04x\n", stringValue, dieAndChildrenIdx, childIdx, attr, attr_in, uval, theform);
  compileUnitInfo = &self->dwarfDbgGetDbgInfo->compileUnits[self->dwarfDbgGetDbgInfo->currCompileUnitIdx].compileUnitInfo;
  dieAndChildrenInfo = &compileUnitInfo->dieAndChildrenInfo[dieAndChildrenIdx];
  dieInfo = &dieAndChildrenInfo->dieChildren[childIdx];
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
printf("== numAttrs: %d\n", dieInfo->numAttr);
  dieAttr = &dieInfo->dieAttrs[dieInfo->numAttr];
  memset(dieAttr, 0, sizeof(dieAttr_t));
  dieAttr->attr = attr;
  dieAttr->attr_in = attr_in;
  dieAttr->uval = uval;
  dieAttr->theform = theform;
  dieAttr->directform = directform;
  dieAttr->flags = flags;
  *childAttrIdx = dieInfo->numAttr;
  dieInfo->numAttr++;
  return result;
}

// =================================== addDieSiblingAttr =========================== 

static uint8_t addDieSiblingAttr(dwarfDbgPtr_t self, size_t dieAndChildrenIdx, size_t siblingIdx, Dwarf_Half attr, Dwarf_Attribute attr_in, Dwarf_Unsigned uval, Dwarf_Half theform, Dwarf_Half directform, uint16_t flags, size_t *siblingAttrIdx) {
  uint8_t result;
  const char *stringValue;
  dieAndChildrenInfo_t *dieAndChildrenInfo;
  dieInfo_t *dieInfo;
  dieAttr_t *dieAttr;
  compileUnitInfo_t *compileUnitInfo;

  result = DWARF_DBG_ERR_OK;
  result = self->dwarfDbgStringInfo->getDW_AT_string(self, attr, &stringValue);
  checkErrOK(result);
printf("== addDieSiblingAttr: %s dieAndChildrenIdx: %d siblingIdx: %d attr: 0x%08x attr_in: 0x%08x uval: 0x%08x, theform: 0x%04x\n", stringValue, dieAndChildrenIdx, siblingIdx, attr, attr_in, uval, theform);
  compileUnitInfo = &self->dwarfDbgGetDbgInfo->compileUnits[self->dwarfDbgGetDbgInfo->currCompileUnitIdx].compileUnitInfo;
  dieAndChildrenInfo = &compileUnitInfo->dieAndChildrenInfo[dieAndChildrenIdx];
  dieInfo = &dieAndChildrenInfo->dieSiblings[siblingIdx];
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
printf("== numAttrs: %d\n", dieInfo->numAttr);
  dieAttr = &dieInfo->dieAttrs[dieInfo->numAttr];
  memset(dieAttr, 0, sizeof(dieAttr_t));
  dieAttr->attr = attr;
  dieAttr->attr_in = attr_in;
  dieAttr->uval = uval;
  dieAttr->theform = theform;
  dieAttr->directform = directform;
  dieAttr->flags = flags;
  *siblingAttrIdx = dieInfo->numAttr;
  dieInfo->numAttr++;
  return result;
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
  self->dwarfDbgDieInfo->addDieSiblingAttr = &addDieSiblingAttr;
  self->dwarfDbgDieInfo->addDieChildAttr = &addDieChildAttr;
  self->dwarfDbgDieInfo->addDieSibling = &addDieSibling;
  self->dwarfDbgDieInfo->addDieChild = &addDieChild;
  self->dwarfDbgDieInfo->addDieAndChildren = &addDieAndChildren;
  return DWARF_DBG_ERR_OK;
}
