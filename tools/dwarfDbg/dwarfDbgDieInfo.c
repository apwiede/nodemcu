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

static uint8_t showDieEntries(dwarfDbgPtr_t self, int dieAndChildrenIdx, Dwarf_Bool isSibling, const char *indent) {
  int result;
  int entryIdx = 0;
  int maxEntries = 0;
  int attrIdx = 0;
  int sres = 0;
  const char *tagStringValue;
  const char *attrStringValue;
  const char *formStringValue;
  dieAndChildrenInfo_t *dieAndChildrenInfo;
  compileUnit_t *compileUnit;
  dieInfo_t *dieInfo;
  dieAttr_t *attrInfo;
  Dwarf_Error err;
  char *temps;
  char buf[255];

  result = DWARF_DBG_ERR_OK;
  compileUnit = self->dwarfDbgCompileUnitInfo->currCompileUnit;
  dieAndChildrenInfo = &compileUnit->dieAndChildrenInfos[dieAndChildrenIdx];
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
sres = dwarf_formstring(attrInfo->attrIn, &temps, &err);
if (sres == DW_DLV_OK) {
sprintf(buf, "%s", temps);
}
switch (attrInfo->attr) {
case DW_AT_call_file:
case DW_AT_decl_file:
{
  int fileIdx;
  pathNameInfo_t *pathNameInfo;
  dirNamesInfo_t *dirNamesInfo;
  int compileUnitIdx = self->dwarfDbgCompileUnitInfo->currCompileUnitIdx;
  int j;

  sres = DW_DLV_OK;
  fileIdx = compileUnit->sourceFiles[attrInfo->uval];
  if (fileIdx < self->dwarfDbgFileInfo->pathNamesInfo.numPathName) {
    pathNameInfo = &self->dwarfDbgFileInfo->pathNamesInfo.pathNames[fileIdx];
    dirNamesInfo = &self->dwarfDbgFileInfo->dirNamesInfo;
    sprintf(buf, "%s/%s", dirNamesInfo->dirNames[pathNameInfo->dirNameIdx], pathNameInfo->fileName); 
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
      fprintf(showFd, "%s  %s: attr_in: 0x%08x theform: 0x%04x %s uval: 0x%08x %s\n", indent, attrStringValue, attrInfo->attrIn, attrInfo->theform, formStringValue, attrInfo->uval, buf);
} else {
      fprintf(showFd, "%s  %s: attr_in: 0x%08x theform: 0x%04x %s uval: 0x%08x\n", indent, attrStringValue, attrInfo->attrIn, attrInfo->theform, formStringValue, attrInfo->uval);
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
  for(strIdx = 0; strIdx < self->dwarfDbgCompileUnitInfo->numAttrStr; strIdx++) {
    if (strcmp(self->dwarfDbgCompileUnitInfo->attrStrs[strIdx], str) == 0) {
      *attrStrIdx = strIdx;
      return result;
    }
  }
  if (self->dwarfDbgCompileUnitInfo->maxAttrStr <= self->dwarfDbgCompileUnitInfo->numAttrStr) {
    self->dwarfDbgCompileUnitInfo->maxAttrStr += 10;
    if (self->dwarfDbgCompileUnitInfo->attrStrs == NULL) {
      self->dwarfDbgCompileUnitInfo->attrStrs = (char **)ckalloc(sizeof(char *) * self->dwarfDbgCompileUnitInfo->maxAttrStr);
      if (self->dwarfDbgCompileUnitInfo->attrStrs == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      self->dwarfDbgCompileUnitInfo->attrStrs = (char **)ckrealloc((char *)self->dwarfDbgCompileUnitInfo->attrStrs, sizeof(char *) * self->dwarfDbgCompileUnitInfo->maxAttrStr);
      if (self->dwarfDbgCompileUnitInfo->attrStrs == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
//printf("== numAttrStrs: %d %s\n", self->dwarfDbgCompileUnitInfo->numAttrStr, str);
  attrStr = &self->dwarfDbgCompileUnitInfo->attrStrs[self->dwarfDbgCompileUnitInfo->numAttrStr];
  *attrStr = ckalloc(strlen(str) + 1);
  memset(*attrStr, 0, strlen(str) + 1);
  memcpy(*attrStr, str, strlen(str));
  *attrStrIdx = self->dwarfDbgCompileUnitInfo->numAttrStr;
  self->dwarfDbgCompileUnitInfo->numAttrStr++;
  return result;
}

// =================================== addDieAttr =========================== 

static uint8_t addDieAttr(dwarfDbgPtr_t self, int dieAndChildrenIdx, Dwarf_Bool isSibling, int idx, Dwarf_Half attr, Dwarf_Attribute attrIn,  Dwarf_Unsigned uval, Dwarf_Half theform, Dwarf_Half directform, int *attrIdx) {
  uint8_t result;
  const char *atName;
  dieAndChildrenInfo_t *dieAndChildrenInfo;
  dieInfo_t *dieInfo;
  dieAttr_t *dieAttr;
  compileUnit_t *compileUnit;
  fileInfo_t *fileInfo;
  int attrStrIdx = -1;

  result = DWARF_DBG_ERR_OK;
  result = self->dwarfDbgStringInfo->getDW_AT_string(self, attr, &atName);
  checkErrOK(result);
  compileUnit = self->dwarfDbgCompileUnitInfo->currCompileUnit;
//printf("  >>==addDieAttr: %s dieAndChildrenIdx: %d isSibling: %d idx: %d attr: 0x%08x attr_in: 0x%08x\n", atName, dieAndChildrenIdx, isSibling, idx, attr, attrIn);
DWARF_DBG_PRINT(self, "A", 1, "                     %*s%-26s", (compileUnit->level - 1) * 2, " ", atName);
  dieAndChildrenInfo = &compileUnit->dieAndChildrenInfos[dieAndChildrenIdx];
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
  dieAttr = &dieInfo->dieAttrs[dieInfo->numAttr];
  memset(dieAttr, 0, sizeof(dieAttr_t));
  dieAttr->attr = attr;
  dieAttr->attrIn = attrIn;
  dieAttr->uval = uval;
  dieAttr->theform = theform;
  dieAttr->directform = directform;
  dieAttr->sourceFileIdx = -1;
  dieAttr->sourceLineNo = -1;
  dieAttr->flags = 0;
  switch (attr) {
  case DW_AT_location:
  case DW_AT_frame_base:
  case DW_AT_GNU_call_site_target:
  case DW_AT_GNU_call_site_value:
    dieAttr->locationInfo = (locationInfo_t *)ckalloc(sizeof(locationInfo_t));
    if (dieAttr->locationInfo == NULL) {
      return DWARF_DBG_ERR_OUT_OF_MEMORY;
    }
    memset(dieAttr->locationInfo, 0, sizeof(locationInfo_t));
    break;
  default:
    dieAttr->locationInfo = NULL;
    break;
  }
  *attrIdx = dieInfo->numAttr;
  dieInfo->numAttr++;
  return result;
}

// =================================== showSiblings =========================== 

static uint8_t showSiblings(dwarfDbgPtr_t self, int dieAndChildrenIdx, const char *indent) {
  return showDieEntries(self, dieAndChildrenIdx, /* isSibling */ 1, indent);
}

// =================================== showChildren =========================== 

static uint8_t showChildren(dwarfDbgPtr_t self, int dieAndChildrenIdx, const char *indent) {
  return showDieEntries(self, dieAndChildrenIdx, /* isSibling */ 0, indent);
}

// =================================== addDieChildAttr =========================== 

static uint8_t addDieChildAttr(dwarfDbgPtr_t self, int dieAndChildrenIdx, int childIdx, Dwarf_Half attr, Dwarf_Attribute attrIn, Dwarf_Unsigned uval, Dwarf_Half theform, Dwarf_Half directform, int *childAttrIdx) {
  return addDieAttr(self, dieAndChildrenIdx, /* isSibling */ 0, childIdx, attr, attrIn, uval, theform, directform, childAttrIdx);
}

// =================================== addDieSiblingAttr =========================== 

static uint8_t addDieSiblingAttr(dwarfDbgPtr_t self, int dieAndChildrenIdx, int siblingIdx, Dwarf_Half attr, Dwarf_Attribute attrIn, Dwarf_Unsigned uval, Dwarf_Half theform, Dwarf_Half directform, int *siblingAttrIdx) {
  return addDieAttr(self, dieAndChildrenIdx, /* isSibling */ 1, siblingIdx, attr, attrIn, uval, theform, directform, siblingAttrIdx);
}

// =================================== addDieSibling =========================== 

static uint8_t addDieSibling(dwarfDbgPtr_t self, int dieAndChildrenIdx, Dwarf_Off offset, Dwarf_Half tag, int *siblingIdx) {
  uint8_t result;
  dieAndChildrenInfo_t *dieAndChildrenInfo;
  dieInfo_t *dieInfo;
  compileUnit_t *compileUnit;
  int formalParameterInfoIdx;
  int variableInfoIdx;

  result = DWARF_DBG_ERR_OK;
//printf("== addDieSibling: offset: 0x%04x tag: 0x%04x\n", offset, tag);
  compileUnit = self->dwarfDbgCompileUnitInfo->currCompileUnit;
  dieAndChildrenInfo = &compileUnit->dieAndChildrenInfos[dieAndChildrenIdx];
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
  dieInfo->tagRef = -1;
  switch (tag) {
  case DW_TAG_subprogram:
    result = self->dwarfDbgSubProgramInfo->addSubProgramInfo(self, dieAndChildrenInfo->numSiblings, /* isSibling */ 1, &compileUnit->currSubProgramInfoIdx);
    checkErrOK(result);
    break;
  case DW_TAG_formal_parameter:
    result = self->dwarfDbgSubProgramInfo->addFormalParameterInfo(self, compileUnit->currSubProgramInfoIdx, dieAndChildrenInfo->numSiblings, &formalParameterInfoIdx);
    checkErrOK(result);
    break;
  case DW_TAG_variable:
    result = self->dwarfDbgSubProgramInfo->addVariableInfo(self, compileUnit->currSubProgramInfoIdx, dieAndChildrenInfo->numSiblings, &variableInfoIdx);
    checkErrOK(result);
    break;
  }
  *siblingIdx = dieAndChildrenInfo->numSiblings;
  dieAndChildrenInfo->numSiblings++;
  return result;
}

// =================================== addDieChild =========================== 

static uint8_t addDieChild(dwarfDbgPtr_t self, int dieAndChildrenIdx, Dwarf_Off offset, Dwarf_Half tag, int *childIdx) {
  uint8_t result;
  dieAndChildrenInfo_t *dieAndChildrenInfo;
  dieInfo_t *dieInfo;
  compileUnit_t *compileUnit;
  int formalParameterInfoIdx;
  int variableInfoIdx;

  result = DWARF_DBG_ERR_OK;
//printf("== addDieChild: offset: 0x%04x tag: 0x%04x\n", offset, tag);
  compileUnit = self->dwarfDbgCompileUnitInfo->currCompileUnit;
  dieAndChildrenInfo = &compileUnit->dieAndChildrenInfos[dieAndChildrenIdx];
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
  dieInfo->tagRef = -1;
  switch (tag) {
  case DW_TAG_subprogram:
    result = self->dwarfDbgSubProgramInfo->addSubProgramInfo(self, dieAndChildrenInfo->numChildren, /* isSibling */ 0, &compileUnit->currSubProgramInfoIdx);
    checkErrOK(result);
    break;
  case DW_TAG_formal_parameter:
    result = self->dwarfDbgSubProgramInfo->addFormalParameterInfo(self, compileUnit->currSubProgramInfoIdx, dieAndChildrenInfo->numChildren, &formalParameterInfoIdx);
    checkErrOK(result);
    break;
  case DW_TAG_variable:
    result = self->dwarfDbgSubProgramInfo->addVariableInfo(self, compileUnit->currSubProgramInfoIdx, dieAndChildrenInfo->numChildren, &variableInfoIdx);
    checkErrOK(result);
    break;
  }
  *childIdx = dieAndChildrenInfo->numChildren;
  dieAndChildrenInfo->numChildren++;
  return result;
}

// =================================== addDieAndChildren =========================== 

static uint8_t addDieAndChildren(dwarfDbgPtr_t self, Dwarf_Die die, int *dieAndChildrenIdx) {
  uint8_t result;
  dieAndChildrenInfo_t *dieAndChildrenInfo;
  compileUnit_t *compileUnit;

  result = DWARF_DBG_ERR_OK;
//printf("== addDieAndChildren: die: 0x%08x\n", die);
  compileUnit = self->dwarfDbgCompileUnitInfo->currCompileUnit;
  if (compileUnit->maxDieAndChildren <= compileUnit->numDieAndChildren) {
    compileUnit->maxDieAndChildren += 10;
    if (compileUnit->dieAndChildrenInfos == NULL) {
      compileUnit->dieAndChildrenInfos = (dieAndChildrenInfo_t *)ckalloc(sizeof(dieAndChildrenInfo_t) * compileUnit->maxDieAndChildren);
      if (compileUnit->dieAndChildrenInfos == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      compileUnit->dieAndChildrenInfos = (dieAndChildrenInfo_t *)ckrealloc((char *)compileUnit->dieAndChildrenInfos, sizeof(dieAndChildrenInfo_t) * compileUnit->maxDieAndChildren);
      if (compileUnit->dieAndChildrenInfos == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
//printf("== numDieAndChildren: %d\n", compileUnit->numDieAndChildren);
  dieAndChildrenInfo = &compileUnit->dieAndChildrenInfos[compileUnit->numDieAndChildren];
  memset(dieAndChildrenInfo, 0, sizeof(dieAndChildrenInfo_t));
  *dieAndChildrenIdx = compileUnit->numDieAndChildren;
  compileUnit->numDieAndChildren++;
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
