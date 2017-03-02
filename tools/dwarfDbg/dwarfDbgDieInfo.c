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
      fprintf(showFd, "%s  %s: attr_in: 0x%08x theform: 0x%04x %s uval: 0x%08x refOffset: 0x%08x %s\n", indent, attrStringValue, attrInfo->attrIn, attrInfo->theform, formStringValue, attrInfo->uval, attrInfo->refOffset, buf);
} else {
      fprintf(showFd, "%s  %s: attr_in: 0x%08x theform: 0x%04x %s uval: 0x%08x refOffset: 0x%08x\n", indent, attrStringValue, attrInfo->attrIn, attrInfo->theform, formStringValue, attrInfo->uval, attrInfo->refOffset);
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

// =================================== addDieSiblingTagInfo =========================== 

static uint8_t addDieSiblingTagInfo(dwarfDbgPtr_t self, int dieAndChildrenIdx, Dwarf_Half tag, int dwAttrTypeInfoIdx, int numAttr, int *siblingTagInfoIdx) {
  uint8_t result;
  int entryIdx = 0;
  dwAttrTypeInfos_t *dwAttrTypeInfos = NULL;
  dieAndChildrenInfo_t *dieAndChildrenInfo;
  dieTagInfo_t *dieTagInfo;
  compileUnit_t *compileUnit;
  Tcl_HashTable *attrTypeInfoHashTable;

  result = DWARF_DBG_ERR_OK;
//printf("== addDieSiblingTagInfo: tag: 0x%04x, dieAndChildrenIdx: %d, dwAttrTypeInfoIdx: %d\n", tag, dieAndChildrenIdx, dwAttrTypeInfoIdx);
if (dwAttrTypeInfoIdx == -1) {
printf("ERROR addDieChildTagInfo: dwAttrTypeInfoIdx -1\n");
}
  compileUnit = self->dwarfDbgCompileUnitInfo->currCompileUnit;
  dieAndChildrenInfo = &compileUnit->dieAndChildrenInfos[dieAndChildrenIdx];
//printf("max: %d num: %d dieAndChildrenInfo: %p dieSiblingsTagInfos: %p\n", dieAndChildrenInfo->maxSiblingsTagInfo, dieAndChildrenInfo->numSiblingsTagInfo, dieAndChildrenInfo, dieAndChildrenInfo->dieSiblingsTagInfos);
  // check for existing entry first!
  for(entryIdx = 0; entryIdx < dieAndChildrenInfo->numSiblingsTagInfo; entryIdx++) {
    dieTagInfo = &dieAndChildrenInfo->dieSiblingsTagInfos[entryIdx];
    if (dieTagInfo->tag == tag) {
      if (dieTagInfo->dwAttrTypeInfoIdx == dwAttrTypeInfoIdx) {
        if (dieTagInfo->numAttr == numAttr) {
          result = self->dwarfDbgTypeInfo->getAttrTypeInfos(self, dieTagInfo->tag, &dwAttrTypeInfos, &attrTypeInfoHashTable);
          checkErrOK(result);
          if (dwAttrTypeInfos->dwAttrTypeInfos[dwAttrTypeInfoIdx].numDwAttrType == numAttr) {
printf("addDieSiblingTagInfo: found: siblingTagInfoIdx: %d\n", entryIdx);
            *siblingTagInfoIdx = entryIdx;
            return result;
          }
        }
      }
    }
  }
  if (dieAndChildrenInfo->maxSiblingsTagInfo <= dieAndChildrenInfo->numSiblingsTagInfo) {
    dieAndChildrenInfo->maxSiblingsTagInfo += 10;
    if (dieAndChildrenInfo->dieSiblingsTagInfos == NULL) {
      dieAndChildrenInfo->dieSiblingsTagInfos = (dieTagInfo_t *)ckalloc(sizeof(dieTagInfo_t) * dieAndChildrenInfo->maxSiblingsTagInfo);
      if (dieAndChildrenInfo->dieSiblingsTagInfos == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      dieAndChildrenInfo->dieSiblingsTagInfos = (dieTagInfo_t *)ckrealloc((char *)dieAndChildrenInfo->dieSiblingsTagInfos, sizeof(dieTagInfo_t) * dieAndChildrenInfo->maxSiblingsTagInfo);
      if (dieAndChildrenInfo->dieSiblingsTagInfos == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
//printf("== numSiblings: %d\n", dieAndChildrenInfo->numSiblingsTagInfo);
  dieTagInfo = &dieAndChildrenInfo->dieSiblingsTagInfos[dieAndChildrenInfo->numSiblingsTagInfo];
  memset(dieTagInfo, 0, sizeof(dieTagInfo_t));
  dieTagInfo->tag = tag;
  dieTagInfo->dwAttrTypeInfoIdx = dwAttrTypeInfoIdx;
  dieTagInfo->numAttr = numAttr;
  *siblingTagInfoIdx = dieAndChildrenInfo->numSiblingsTagInfo;
printf("addDieSiblingTagInfo: new: siblingTagInfoIdx: %d\n", *siblingTagInfoIdx);
  dieAndChildrenInfo->numSiblingsTagInfo++;
  return result;
}

// =================================== addDieChildTagInfo =========================== 

static uint8_t addDieChildTagInfo(dwarfDbgPtr_t self, int dieAndChildrenIdx,  Dwarf_Half tag, int dwAttrTypeInfoIdx, int numAttr, int *childTagInfoIdx) {
  uint8_t result;
  int entryIdx = 0;
  dwAttrTypeInfos_t *dwAttrTypeInfos = NULL;
  dieAndChildrenInfo_t *dieAndChildrenInfo;
  dieTagInfo_t *dieTagInfo;
  compileUnit_t *compileUnit;
  Tcl_HashTable *attrTypeInfoHashTable;

  result = DWARF_DBG_ERR_OK;
//printf("== addDieChildTagInfo: tag: 0x%04x dieAndChildrenIdx: %d dwAttrTypeInfoIdx: %d\n", tag, dieAndChildrenIdx, dwAttrTypeInfoIdx);
if (dwAttrTypeInfoIdx == -1) {
printf("ERROR addDieChildTagInfo: dwAttrTypeInfoIdx -1\n");
}
  compileUnit = self->dwarfDbgCompileUnitInfo->currCompileUnit;
  dieAndChildrenInfo = &compileUnit->dieAndChildrenInfos[dieAndChildrenIdx];
  // check for existing entry first!
  for(entryIdx = 0; entryIdx < dieAndChildrenInfo->numChildrenTagInfo; entryIdx++) {
    dieTagInfo = &dieAndChildrenInfo->dieChildrenTagInfos[entryIdx];
    if (dieTagInfo->tag == tag) {
      if (dieTagInfo->dwAttrTypeInfoIdx == dwAttrTypeInfoIdx) {
        if (dieTagInfo->numAttr == numAttr) {
          result = self->dwarfDbgTypeInfo->getAttrTypeInfos(self, dieTagInfo->tag, &dwAttrTypeInfos, &attrTypeInfoHashTable);
          checkErrOK(result);
          if (dwAttrTypeInfos->dwAttrTypeInfos[dwAttrTypeInfoIdx].numDwAttrType == numAttr) {
            *childTagInfoIdx = entryIdx;
printf("addDieChildTagInfo: found: childTagInfoIdx: %d\n", entryIdx);
            return result;
          }
        }
      }
    }
  }
  if (dieAndChildrenInfo->maxChildrenTagInfo <= dieAndChildrenInfo->numChildrenTagInfo) {
    dieAndChildrenInfo->maxChildrenTagInfo += 10;
    if (dieAndChildrenInfo->dieChildrenTagInfos == NULL) {
      dieAndChildrenInfo->dieChildrenTagInfos = (dieTagInfo_t *)ckalloc(sizeof(dieTagInfo_t) * dieAndChildrenInfo->maxChildrenTagInfo);
      if (dieAndChildrenInfo->dieChildrenTagInfos == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      dieAndChildrenInfo->dieChildrenTagInfos = (dieTagInfo_t *)ckrealloc((char *)dieAndChildrenInfo->dieChildrenTagInfos, sizeof(dieTagInfo_t) * dieAndChildrenInfo->maxChildrenTagInfo);
      if (dieAndChildrenInfo->dieChildrenTagInfos == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
//printf("== numChildren: %d\n", dieAndChildrenInfo->numChildrenTagInfo);
  dieTagInfo = &dieAndChildrenInfo->dieChildrenTagInfos[dieAndChildrenInfo->numChildrenTagInfo];
  memset(dieTagInfo, 0, sizeof(dieTagInfo_t));
  dieTagInfo->tag = tag;
  dieTagInfo->dwAttrTypeInfoIdx = dwAttrTypeInfoIdx;
  dieTagInfo->numAttr = numAttr;
  *childTagInfoIdx = dieAndChildrenInfo->numChildrenTagInfo;
printf("addDieChildTagInfo: new: childTagInfoIdx: %d\n", *childTagInfoIdx);
  dieAndChildrenInfo->numChildrenTagInfo++;
  return result;
}

// =================================== addDieSibling =========================== 

static uint8_t addDieSibling(dwarfDbgPtr_t self, int dieAndChildrenIdx, Dwarf_Off offset, Dwarf_Half tag, int *siblingIdx) {
  uint8_t result;
  dieAndChildrenInfo_t *dieAndChildrenInfo;
  dieInfo_t *dieInfo;
  compileUnit_t *compileUnit;

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
//printf("== numSiblings: %d\n", dieAndChildrenInfo->numSiblings);
  dieInfo = &dieAndChildrenInfo->dieSiblings[dieAndChildrenInfo->numSiblings];
  memset(dieInfo, 0, sizeof(dieInfo_t));
  dieInfo->offset = offset;
  dieInfo->tag = tag;
  dieInfo->tagRefIdx = -1;
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
//printf("== numChildren: %d\n", dieAndChildrenInfo->numChildren);
  dieInfo = &dieAndChildrenInfo->dieChildren[dieAndChildrenInfo->numChildren];
  memset(dieInfo, 0, sizeof(dieInfo_t));
  dieInfo->offset = offset;
  dieInfo->tag = tag;
  dieInfo->tagRefIdx = -1;
  *childIdx = dieAndChildrenInfo->numChildren;
  dieAndChildrenInfo->numChildren++;
  return result;
}

// =================================== addDieAndChildren =========================== 

static uint8_t addDieAndChildren(dwarfDbgPtr_t self, Dwarf_Die die, int isSibling, int level, int *dieAndChildrenIdx) {
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
  dieAndChildrenInfo = &compileUnit->dieAndChildrenInfos[compileUnit->numDieAndChildren];
  memset(dieAndChildrenInfo, 0, sizeof(dieAndChildrenInfo_t));
  dieAndChildrenInfo->die = die;
  dieAndChildrenInfo->isSibling = isSibling;
  *dieAndChildrenIdx = compileUnit->numDieAndChildren;
  compileUnit->numDieAndChildren++;
  return result;
}

static int numDies = 0;

// =================================== handleOneDie =========================== 

static uint8_t handleOneDie(dwarfDbgPtr_t self, Dwarf_Die die, int isSibling, int level, char **srcfiles, Dwarf_Signed srcCnt, int dieAndChildrenIdx, int *dieInfoIdx) {
  uint8_t result;
  int tres = 0;
  int ores = 0;
  int atres = 0;
  int res = 0;
  int sres = 0;
  int cdres = DW_DLV_OK;
  const char * tagName = 0;
  int i = 0;
  int typeIdx = 0;
  Dwarf_Error err;
  Dwarf_Half tag = 0;
  Dwarf_Off offset = 0;
  Dwarf_Signed attrCnt = 0;
  Dwarf_Attribute *atList = 0;
  Dwarf_Die child = NULL;
  Dwarf_Die sibling = NULL;
  compileUnit_t *compileUnit;
  dieAndChildrenInfo_t *dieAndChildrenInfo;
  dieInfo_t *dieInfo = NULL;
  attrValues_t *attrValues;
  dwAttrTypeInfo_t dwAttrTypeInfo;
  const char *attrName = NULL;
  int typeStrIdx = 0;
  int attrTypeIdx = 0;

  result = DWARF_DBG_ERR_OK;
//printf(">>handleOneDie die: %p numDies: %d isSibling: %d level: %d\n", die, ++numDies, isSibling, level);
  compileUnit = self->dwarfDbgCompileUnitInfo->currCompileUnit;
//printf("handleOneDie: level: %d\n", compileUnit->level);
  tres = dwarf_tag(die, &tag, &err);
  if (tres != DW_DLV_OK) {
    printf("accessing tag of die! tres: %d, err: %p", tres, err);
  }
//printf("tag: 0x%04x\n", tag);
  result = self->dwarfDbgStringInfo->getDW_TAG_string(self, tag, &tagName);
  checkErrOK(result);

  ores = dwarf_die_CU_offset(die, &offset, &err);
  if (ores != DW_DLV_OK) {
    printf("dwarf_die_CU_offset ores: %d err: %p", ores, err);
    return DWARF_DBG_ERR_CANNOT_GET_CU_OFFSET;
  }
  DWARF_DBG_PRINT(self, "G", 1, "<%2d><0x%08x> %*s%s\n", compileUnit->level, offset, compileUnit->level * 2, " ", tagName);
  dieAndChildrenInfo = &compileUnit->dieAndChildrenInfos[dieAndChildrenIdx];
  dieAndChildrenInfo->isSibling = isSibling;
  dieAndChildrenInfo->level = level;
//printf("dieAndChildrenIdx: %d\n", dieAndChildrenIdx);
  if (isSibling) {
//printf("  >>numAddSibling: %d\n", ++numAddSibling);
    result = self->dwarfDbgDieInfo->addDieSibling(self, dieAndChildrenIdx, offset, tag, dieInfoIdx);
    checkErrOK(result);
    dieInfo = &dieAndChildrenInfo->dieSiblings[*dieInfoIdx];
  } else {
//printf("  >>numAddChild: %d\n", ++numAddChild);
    result = self->dwarfDbgDieInfo->addDieChild(self, dieAndChildrenIdx, offset, tag, dieInfoIdx);
    checkErrOK(result);
    dieInfo = &dieAndChildrenInfo->dieChildren[*dieInfoIdx];
  }
//printf("dieAndChildrenIdx: %d dieInfo: %p dieInfoIdx: %d\n", dieAndChildrenIdx, dieInfo, *dieInfoIdx);
  atres = dwarf_attrlist(die, &atList, &attrCnt, &err);
//printf("  >>attrCnt: %d\n", attrCnt);
  attrValues = &compileUnit->attrValues;
  memset(attrValues, 0, sizeof(attrValues_t));
  for (i = 0; i < attrCnt; i++) {
    Dwarf_Half attr;
    Dwarf_Attribute attrIn;
    int ares;
    int dieAttrIdx;

    attrIn = atList[i];
    ares = dwarf_whatattr(attrIn, &attr, &err);
    if (ares == DW_DLV_OK) {
      result = self->dwarfDbgAttributeInfo->handleAttribute(self, die, attr, attrIn, srcfiles, srcCnt, dieAndChildrenIdx, *dieInfoIdx, isSibling, &dieAttrIdx);
//printf("result: %d\n", result);
    }
  }
  return result;
}

// =================================== handleDieAndChildren =========================== 

static uint8_t handleDieAndChildren(dwarfDbgPtr_t self, Dwarf_Die in_die_in, int isSibling, int level, char **srcfiles, Dwarf_Signed cnt) {
  uint8_t result;
  int dieInfoIdx = 0;
  int dieAndChildrenIdx = 0;
  Dwarf_Error err;
  Dwarf_Die child = 0;
  Dwarf_Die sibling = 0;
  Dwarf_Die in_die = NULL;
  int cdres = DW_DLV_OK;
  int numChildren = 0;
  compileUnit_t *compileUnit = NULL;
  

  result = DWARF_DBG_ERR_OK;
  in_die = in_die_in;
  compileUnit = self->dwarfDbgCompileUnitInfo->currCompileUnit;
//printf("@@handleDieAndChildren die: %p isSibling: %d level: %d isCompileUnitDie: %d\n", in_die_in, isSibling, level, compileUnit->isCompileUnitDie);
  result = self->dwarfDbgDieInfo->addDieAndChildren(self, in_die_in, isSibling, level + 1, &dieAndChildrenIdx);
  checkErrOK(result);
  for(;;) {
    ++numChildren;
//printf("pre-descent numChildren: %d\n", numChildren);
    /* Here do pre-descent processing of the die. */
    {
//printf("before handleOneDie in_die: %p\n", in_die);
      result = self->dwarfDbgDieInfo->handleOneDie(self, in_die, isSibling, level, srcfiles, cnt, dieAndChildrenIdx, &dieInfoIdx);
      compileUnit->isCompileUnitDie = 0;
//printf("after handleOneDie in_die: %p\n", in_die);
//printf("call dwarf_child in_die: %p\n", in_die);
      child = NULL;
      cdres = dwarf_child(in_die, &child, &err);
//printf("after call dwarf_child in_die: %p child: %p cdres: %d\n", in_die, child, cdres);
      /* child first: we are doing depth-first walk */
      if (cdres == DW_DLV_OK) {
//printf("child first\n");
//printf("call recursive handleDieAndChildren: child: %p numChildren: %d\n", child, numChildren);
        compileUnit->level++;
        handleDieAndChildren(self, child, /* isSibling */ 0, level + 1, srcfiles, cnt);
        compileUnit->level--;
        dwarf_dealloc(self->elfInfo.dbg, child, DW_DLA_DIE);
        child = 0;
      }
      cdres = dwarf_siblingof_b(self->elfInfo.dbg, in_die, /* is_info */1, &sibling, &err);
//printf("dwarf_siblingof_b: numSiblings: %d in_die: %p sibling: %p\n", ++numSiblings, in_die, sibling);
      if (cdres == DW_DLV_OK) {
        /*  handleDieAndChildren(dbg, sibling, srcfiles, cnt); We
            loop around to actually print this, rather than
            recursing. Recursing is horribly wasteful of stack
            space. */
      } else if (cdres == DW_DLV_ERROR) {
        printf("error in dwarf_siblingofi_b cdres: %d err: %p", cdres, err);
        return DWARF_DBG_ERR_CANNOT_GET_SIBLING_OF_COMPILE_UNIT;
      }
//printf("post-descent\n");
      if (in_die != in_die_in) {
        /*  Dealloc our in_die, but not the argument die, it belongs
            to our caller. Whether the siblingof call worked or not. */
        dwarf_dealloc(self->elfInfo.dbg, in_die, DW_DLA_DIE);
        in_die = 0;
      }
      if (cdres == DW_DLV_OK) {
        /*  Set to process the sibling, loop again. */
        in_die = sibling;
        isSibling = 1;
      } else {
        /*  We are done, no more siblings at this level. */
        break;
      }
    }
  }  /* end for loop on siblings */
//printf("handleDieAndChildren done die: %p level: %d\n", in_die_in);
  return result;
}

extern FILE *typeFd;
// =================================== printDieInfoAttrs =========================== 

static uint8_t printDieInfoAttrs(dwarfDbgPtr_t self, dieInfo_t *dieInfo, const char *indent) {
  uint8_t result;
  int attrIdx = 0;
  dieAttr_t *dieAttr;
  const char *tagName;
  const char *typeName;
  char *name;
  char *dirName;
  pathNameInfo_t *pathNameInfo;
  char pathName[255];

  result = DWARF_DBG_ERR_OK;
  result = self->dwarfDbgStringInfo->getDW_TAG_string(self, dieInfo->tag, &tagName);
//printf("getDW_TAG_string: result: %d tag: 0x%04x\n", result, dieInfo->tag);
  checkErrOK(result);
  fprintf(typeFd, "%s>>TAG: %s 0x%04x\n", indent, tagName, dieInfo->tag);
  for (attrIdx = 0; attrIdx < dieInfo->numAttr; attrIdx++) {
    dieAttr = &dieInfo->dieAttrs[attrIdx];
    result = self->dwarfDbgStringInfo->getDW_AT_string(self, dieAttr->attr, &typeName);
    checkErrOK(result);
    name = "";
    pathName[0] = '\0';
    switch (dieAttr->attr) {
    case DW_AT_name:
      name = self->dwarfDbgCompileUnitInfo->attrStrs[dieAttr->attrStrIdx];
      break;
    case DW_AT_decl_file:
      pathNameInfo = &self->dwarfDbgFileInfo->pathNamesInfo.pathNames[dieAttr->sourceFileIdx];
      dirName = self->dwarfDbgFileInfo->dirNamesInfo.dirNames[pathNameInfo->dirNameIdx];
      sprintf(pathName,"%s/%s", dirName, pathNameInfo->fileName);
      break;
    }
    fprintf(typeFd, "%s    %s: %d refOffset: 0x%08x %s%s\n", indent, typeName, dieAttr->uval, dieAttr->refOffset, name, pathName);
    fflush(typeFd);
  }
  return result;
}

// =================================== printCompileUnitDieAndChildren =========================== 

static uint8_t printCompileUnitDieAndChildren(dwarfDbgPtr_t self) {
  uint8_t result;
  int dieAndChildrenIdx = 0;
  int maxEntries = 0;
  int maxEntries2 = 0;
  int entryIdx = 0;
  compileUnit_t *compileUnit;
  dieAndChildrenInfo_t *dieAndChildrenInfo;
  dieInfo_t *dieInfo;
  dieTagInfo_t *dieTagInfo;
  const char *tagName;
  const char *tagName2;

  result = DWARF_DBG_ERR_OK;
fprintf(typeFd, "== printCompileUnitDieAndChildren %d\n", self->dwarfDbgCompileUnitInfo->currCompileUnitIdx);
  compileUnit = self->dwarfDbgCompileUnitInfo->currCompileUnit;
  for (dieAndChildrenIdx = 0; dieAndChildrenIdx < compileUnit->numDieAndChildren; dieAndChildrenIdx++) {
    dieAndChildrenInfo = &compileUnit->dieAndChildrenInfos[dieAndChildrenIdx];
    fprintf(typeFd, "dieAndChildrenIdx: %d die: %p isSibling: %d level: %d\n", dieAndChildrenIdx, dieAndChildrenInfo->die, dieAndChildrenInfo->isSibling, dieAndChildrenInfo->level);
    fprintf(typeFd, "  numChildren: %d\n", dieAndChildrenInfo->numChildren);
    maxEntries2 = dieAndChildrenInfo->numChildren;
    for (entryIdx = 0; entryIdx < maxEntries2; entryIdx++) {
      dieInfo = &dieAndChildrenInfo->dieChildren[entryIdx];
      result = self->dwarfDbgStringInfo->getDW_TAG_string(self, dieInfo->tag, &tagName);
      fprintf(typeFd, "    %d tag: %s\n", entryIdx, tagName);
      result = self->dwarfDbgDieInfo->printDieInfoAttrs(self, dieInfo, "        ");
      checkErrOK(result);
    }
    fprintf(typeFd, "  numSiblings: %d\n", dieAndChildrenInfo->numSiblings);
    maxEntries2 = dieAndChildrenInfo->numSiblings;
    for (entryIdx = 0; entryIdx < maxEntries2; entryIdx++) {
      dieInfo = &dieAndChildrenInfo->dieSiblings[entryIdx];
      result = self->dwarfDbgStringInfo->getDW_TAG_string(self, dieInfo->tag, &tagName);
      fprintf(typeFd, "    %d tag: %s\n", entryIdx, tagName);
      result = self->dwarfDbgDieInfo->printDieInfoAttrs(self, dieInfo, "        ");
      checkErrOK(result);
    }
    fprintf(typeFd, "  numChildrenTagInfo: %d\n", dieAndChildrenInfo->numChildrenTagInfo);
    maxEntries = dieAndChildrenInfo->numChildrenTagInfo;
    for (entryIdx = 0; entryIdx < maxEntries; entryIdx++) {
      dieTagInfo = &dieAndChildrenInfo->dieChildrenTagInfos[entryIdx];
      result = self->dwarfDbgStringInfo->getDW_TAG_string(self, dieTagInfo->tag, &tagName);
      result = self->dwarfDbgStringInfo->getDW_TAG_string(self, dieInfo->tag, &tagName2);
printf("entryIdx: %d result: %d tag: %s dieInfo->tag: %s tagRefIdx: %d\n", entryIdx, result, tagName, tagName2, dieInfo->tagRefIdx);
      checkErrOK(result);
      fprintf(typeFd, "    %d tag: %s\n", entryIdx, tagName);
printf("dwAttrTypeInfoIdx: %d\n", dieTagInfo->dwAttrTypeInfoIdx);
      result = self->dwarfDbgTypeInfo->printAttrTypeInfo(self, dieInfo->tag, dieTagInfo->dwAttrTypeInfoIdx, /* isSibling */ 0, "        ");
      checkErrOK(result);
    }
printf("childrenTagInfo done\n");
    fprintf(typeFd, "  numSiblingsTagInfo: %d\n", dieAndChildrenInfo->numSiblingsTagInfo);
    maxEntries = dieAndChildrenInfo->numSiblingsTagInfo;
    for (entryIdx = 0; entryIdx < maxEntries; entryIdx++) {
      dieTagInfo = &dieAndChildrenInfo->dieSiblingsTagInfos[entryIdx];
      result = self->dwarfDbgStringInfo->getDW_TAG_string(self, dieTagInfo->tag, &tagName);
      fprintf(typeFd, "    %d tag: %s\n", entryIdx, tagName);
      result = self->dwarfDbgTypeInfo->printAttrTypeInfo(self, dieInfo->tag, dieTagInfo->dwAttrTypeInfoIdx, /* isSibling */ 1, "        ");
      checkErrOK(result);
    }
  }
  fflush(typeFd);
  return result;
}

// =================================== dwarfDbgDieInfoInit =========================== 

int dwarfDbgDieInfoInit (dwarfDbgPtr_t self) {

  showFd = stdout;
  self->dwarfDbgDieInfo->showSiblings = &showSiblings;
  self->dwarfDbgDieInfo->showChildren = &showChildren;
  self->dwarfDbgDieInfo->addAttrStr = &addAttrStr;
  self->dwarfDbgDieInfo->addDieSiblingAttr = &addDieSiblingAttr;
  self->dwarfDbgDieInfo->addDieChildAttr = &addDieChildAttr;
  self->dwarfDbgDieInfo->addDieSiblingTagInfo = &addDieSiblingTagInfo;
  self->dwarfDbgDieInfo->addDieChildTagInfo = &addDieChildTagInfo;
  self->dwarfDbgDieInfo->addDieSibling = &addDieSibling;
  self->dwarfDbgDieInfo->addDieChild = &addDieChild;
  self->dwarfDbgDieInfo->addDieAndChildren = &addDieAndChildren;
  self->dwarfDbgDieInfo->handleOneDie = &handleOneDie;
  self->dwarfDbgDieInfo->handleDieAndChildren = &handleDieAndChildren;
  self->dwarfDbgDieInfo->printDieInfoAttrs = &printDieInfoAttrs;
  self->dwarfDbgDieInfo->printCompileUnitDieAndChildren = &printCompileUnitDieAndChildren;
  return DWARF_DBG_ERR_OK;
}
