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

// =================================== addDieChildAttr =========================== 

static uint8_t addDieChildAttr(dwarfDbgPtr_t self, size_t dieAndChildrenIdx, size_t childIdx, Dwarf_Half attr, Dwarf_Unsigned uval, Dwarf_Half theform, Dwarf_Half directform, uint16_t flags, size_t *childAttrIdx) {
  uint8_t result;
  dieAndChildrenInfo_t *dieAndChildrenInfo;
  dieInfo_t *dieInfo;
  dieAttr_t *dieAttr;

  result = DWARF_DBG_ERR_OK;
printf("== addDieChildAttr: dieAndChildrenIdx: %d childIdx: %d attr: 0x%08x uval: 0x%08x, theform: 0x%04x\n", dieAndChildrenIdx, childIdx, attr, uval, theform);
  dieAndChildrenInfo = &self->dwarfDbgDieInfo->dieAndChildren[dieAndChildrenIdx];
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
  *childAttrIdx = dieInfo->numAttr;
  dieInfo->numAttr++;
  return result;
}

// =================================== addDieSiblingAttr =========================== 

static uint8_t addDieSiblingAttr(dwarfDbgPtr_t self, size_t dieAndChildrenIdx, size_t siblingIdx, Dwarf_Half attr, Dwarf_Unsigned uval, Dwarf_Half theform, Dwarf_Half directform, uint16_t flags, size_t *siblingAttrIdx) {
  uint8_t result;
  dieAndChildrenInfo_t *dieAndChildrenInfo;
  dieInfo_t *dieInfo;
  dieAttr_t *dieAttr;

  result = DWARF_DBG_ERR_OK;
printf("== addDieSiblingAttr: dieAndChildrenIdx: %d siblingIdx: %d attr: 0x%08x uval: 0x%08x, theform: 0x%04x\n", dieAndChildrenIdx, siblingIdx, attr, uval, theform);
  dieAndChildrenInfo = &self->dwarfDbgDieInfo->dieAndChildren[dieAndChildrenIdx];
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
  *siblingAttrIdx = dieInfo->numAttr;
  dieInfo->numAttr++;
  return result;
}

// =================================== addDieSibling =========================== 

static uint8_t addDieSibling(dwarfDbgPtr_t self, size_t dieAndChildrenIdx, size_t offset, Dwarf_Half tag, size_t *siblingIdx) {
  uint8_t result;
  dieAndChildrenInfo_t *dieAndChildrenInfo;
  dieInfo_t *dieInfo;

  result = DWARF_DBG_ERR_OK;
printf("== addDieSibling: offset: 0x%04x tag: 0x%04x\n", offset, tag);
  dieAndChildrenInfo = &self->dwarfDbgDieInfo->dieAndChildren[dieAndChildrenIdx];
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
  *siblingIdx = dieAndChildrenInfo->numSiblings;
  dieAndChildrenInfo->numSiblings++;
  return result;
}

// =================================== addDieChild =========================== 

static uint8_t addDieChild(dwarfDbgPtr_t self, size_t dieAndChildrenIdx, size_t offset, Dwarf_Half tag, size_t *childIdx) {
  uint8_t result;
  dieAndChildrenInfo_t *dieAndChildrenInfo;
  dieInfo_t *dieInfo;

  result = DWARF_DBG_ERR_OK;
printf("== addDieChild: offset: 0x%04x tag: 0x%04x\n", offset, tag);
  dieAndChildrenInfo = &self->dwarfDbgDieInfo->dieAndChildren[dieAndChildrenIdx];
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
  *childIdx = dieAndChildrenInfo->numChildren;
  dieAndChildrenInfo->numChildren++;
  return result;
}

// =================================== addDieAndChildren =========================== 

static uint8_t addDieAndChildren(dwarfDbgPtr_t self, Dwarf_Die die, size_t *dieAndChildrenIdx) {
  uint8_t result;
  dieAndChildrenInfo_t *dieAndChildrenInfo;

  result = DWARF_DBG_ERR_OK;
printf("== addDieAndChildren: die: 0x%08x\n", die);
  if (self->dwarfDbgDieInfo->maxDieAndChildren <= self->dwarfDbgDieInfo->numDieAndChildren) {
    self->dwarfDbgDieInfo->maxDieAndChildren += 10;
    if (self->dwarfDbgDieInfo->dieAndChildren == NULL) {
      self->dwarfDbgDieInfo->dieAndChildren = (dieAndChildrenInfo_t *)ckalloc(sizeof(dieAndChildrenInfo_t) * self->dwarfDbgDieInfo->maxDieAndChildren);
      if (self->dwarfDbgDieInfo->dieAndChildren == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      self->dwarfDbgDieInfo->dieAndChildren = (dieAndChildrenInfo_t *)ckrealloc((char *)self->dwarfDbgDieInfo->dieAndChildren, sizeof(dieAndChildrenInfo_t) * self->dwarfDbgDieInfo->maxDieAndChildren);
      if (self->dwarfDbgDieInfo->dieAndChildren == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
printf("== numDieAndChildren: %d\n", self->dwarfDbgDieInfo->numDieAndChildren);
  dieAndChildrenInfo = &self->dwarfDbgDieInfo->dieAndChildren[self->dwarfDbgDieInfo->numDieAndChildren];
  memset(dieAndChildrenInfo, 0, sizeof(dieAndChildrenInfo_t));
  *dieAndChildrenIdx = self->dwarfDbgDieInfo->numDieAndChildren;
  self->dwarfDbgDieInfo->numDieAndChildren++;
  return result;
}

// =================================== dwarfDbgDieInfoInit =========================== 

int dwarfDbgDieInfoInit (dwarfDbgPtr_t self) {

  self->dwarfDbgDieInfo->maxDieAndChildren = 0;
  self->dwarfDbgDieInfo->numDieAndChildren = 0;
  self->dwarfDbgDieInfo->dieAndChildren = NULL;

  self->dwarfDbgDieInfo->addDieSiblingAttr = &addDieSiblingAttr;
  self->dwarfDbgDieInfo->addDieChildAttr = &addDieChildAttr;
  self->dwarfDbgDieInfo->addDieSibling = &addDieSibling;
  self->dwarfDbgDieInfo->addDieChild = &addDieChild;
  self->dwarfDbgDieInfo->addDieAndChildren = &addDieAndChildren;
  return DWARF_DBG_ERR_OK;
}
