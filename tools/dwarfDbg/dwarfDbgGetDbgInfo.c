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
 * File:   dwarfDbgGetDbgInfo.c
 * Author: Arnulf P. Wiedemann <arnulf@wiedemann-pri.de>
 *
 * Created on January 24, 2017
 */

#include <tcl.h>
#include "dwarfDbgInt.h"

static int numDies = 0;
static int numSiblings = 0;
static int numAddSibling = 0;
static int numAddChild = 0;
static int childrenLevel = 0;

// =================================== handleOneDie =========================== 

static uint8_t handleOneDie(dwarfDbgPtr_t self, Dwarf_Die die, char **srcfiles, Dwarf_Signed cnt, Dwarf_Bool isSibling, int dieAndChildrenIdx, int *dieInfoIdx) {
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
  dwType_t dwTypeInfo;
  const char *attrName = NULL;

  result = DWARF_DBG_ERR_OK;
//printf(">>handleOneDie die: %p numDies: %d\n", die, ++numDies);
  compileUnit = self->dwarfDbgCompileUnitInfo->currCompileUnit;
//printf("handleOneDie: level: %d\n", compileUnit->level);
  tres = dwarf_tag(die, &tag, &err);
  if (tres != DW_DLV_OK) {
    printf("accessing tag of die! tres: %d, err: %p", tres, err);
  }
  result = self->dwarfDbgStringInfo->getDW_TAG_string(self, tag, &tagName);
  checkErrOK(result);

  ores = dwarf_die_CU_offset(die, &offset, &err);
  if (ores != DW_DLV_OK) {
    printf("dwarf_die_CU_offset ores: %d err: %p", ores, err);
    return DWARF_DBG_ERR_CANNOT_GET_CU_OFFSET;
  }
  DWARF_DBG_PRINT(self, "G", 1, "<%2d><0x%08x> %*s%s\n", compileUnit->level, offset, compileUnit->level * 2, " ", tagName);
  dieAndChildrenInfo = &compileUnit->dieAndChildrenInfos[dieAndChildrenIdx];
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
      result = self->dwarfDbgAttributeInfo->handleAttribute(self, die, attr, attrIn, srcfiles, cnt, dieAndChildrenIdx, *dieInfoIdx, isSibling, &dieAttrIdx);
//printf("result: %d\n", result);
    }
  }
  switch (tag) {
  case DW_TAG_base_type:
    dwTypeInfo.artificial = -1;
    dwTypeInfo.abstractOrigin = -1;
    dwTypeInfo.bitOffset = -1;
    dwTypeInfo.bitSize = -1;
    dwTypeInfo.byteSize = attrValues->byteSize;
    dwTypeInfo.callFileIdx = -1;
    dwTypeInfo.callLineNo = -1;
    dwTypeInfo.constValue = -1;
    dwTypeInfo.dataLocation = 0;
    dwTypeInfo.declaration = 0;
    dwTypeInfo.pathNameIdx = -1;
    dwTypeInfo.lineNo = -1;
    dwTypeInfo.encoding = attrValues->encoding;
    dwTypeInfo.entryPc = 0;
    dwTypeInfo.external = 0;
    dwTypeInfo.frameBase = -1;
    dwTypeInfo.GNUAllCallSites = -1;
    dwTypeInfo.GNUAllTailCallSites = -1;
    dwTypeInfo.GNUCallSiteTarget = -1;
    dwTypeInfo.GNUCallSiteValue = -1;
    dwTypeInfo.highPc = -1;
    dwTypeInfo.isInline = -1;
    dwTypeInfo.location = -1;
    dwTypeInfo.linkageName = -1;
    dwTypeInfo.lowPc = -1;
    attrName = attrValues->name;
printf("AT_name: %s\n", attrName);
    dwTypeInfo.typeNameIdx = -1;
    dwTypeInfo.prototyped = 0;
    dwTypeInfo.ranges = -1;
    dwTypeInfo.siblingIdx = -1;
    dwTypeInfo.subrangeType = -1;
    dwTypeInfo.dwTypeIdx = -1;
    dwTypeInfo.upperBound = -1;
    result = self->dwarfDbgTypeInfo->addType(self, &dwTypeInfo, attrName, &self->dwarfDbgTypeInfo->dwBaseTypeInfos, &typeIdx);
    checkErrOK(result);
    dieInfo->tagRefIdx = typeIdx;
    dieInfo->flags = TAG_REF_BASE_TYPE;
//printf("baseType tagRef: %d offset: 0x%08x\n", dieInfo->tagRefIdx, dieInfo->offset);
    break;
  case DW_TAG_subroutine_type:
#ifdef NODTEF
    result = self->dwarfDbgTypeInfo->addSubroutineType(self, attrValues->name, attrValues->byteSize, attrValues->encoding, &typeIdx);
    checkErrOK(result);
    dieInfo->tagRefIdx = typeIdx;
    dieInfo->flags = TAG_REF_SUBROUTINE_TYPE;
printf("subroutineType tagRef: %d offset: 0x%08x\n", dieInfo->tagRefIdx, dieInfo->offset);
#endif
    break;
  case DW_TAG_typedef:
#ifdef NODTEF
    result = self->dwarfDbgTypeInfo->addTypeDef(self, attrValues->name, attrValues->pathNameIdx, attrValues->lineNo, attrValues->dwTypeIdx, &typeIdx);
    checkErrOK(result);
    dieInfo->tagRefIdx = typeIdx;
    dieInfo->flags = TAG_REF_TYPEDEF;
printf("typeDef tagRef: %d offset: 0x%08x\n", dieInfo->tagRefIdx, dieInfo->offset);
#endif
    break;
  case DW_TAG_pointer_type:
#ifdef NODTEF
    result = self->dwarfDbgTypeInfo->addPointerTypf(self, attrValues->name, attrValues->pathNameIdx, attrValues->lineNo, attrValues->dwTypeIdx, &typeIdx);
    checkErrOK(result);
    dieInfo->tagRefIdx = typeIdx;
    dieInfo->flags = TAG_REF_POINTER_TYPE;
printf("pointerType tagRef: %d offset: 0x%08x\n", dieInfo->tagRefIdx, dieInfo->offset);
#endif
    break;
  case DW_TAG_const_type:
  case DW_TAG_member:
  case DW_TAG_structure_type:
  case DW_TAG_array_type:
  case DW_TAG_enumeration_type:
  case DW_TAG_enumerator:
  case DW_TAG_union_type:
  case DW_TAG_volatile_type:
//printf("should store type\n");
      break;
    }
  return result;
}

// =================================== handleDieAndChildren =========================== 

static uint8_t handleDieAndChildren(dwarfDbgPtr_t self, Dwarf_Die in_die_in, char **srcfiles, Dwarf_Signed cnt) {
  uint8_t result;
  int dieInfoIdx = 0;
  int dieAndChildrenIdx = 0;
  Dwarf_Error err;
  Dwarf_Die child = 0;
  Dwarf_Die sibling = 0;
  Dwarf_Die in_die = in_die_in;
  int cdres = DW_DLV_OK;
  int numChildren = 0;
  Dwarf_Bool isSibling = 0;
  compileUnit_t *compileUnit = NULL;
  

  result = DWARF_DBG_ERR_OK;
  compileUnit = self->dwarfDbgCompileUnitInfo->currCompileUnit;
printf("@@handleDieAndChildren die: %p level: %d isCompileUnitDie: %d\n", in_die_in, ++childrenLevel, compileUnit->isCompileUnitDie);
  result = self->dwarfDbgDieInfo->addDieAndChildren(self, in_die_in, &dieAndChildrenIdx);
  checkErrOK(result);
  for(;;) {
    ++numChildren;
//printf("pre-descent numChildren: %d\n", numChildren);
    /* Here do pre-descent processing of the die. */
    {
//printf("before handleOneDie in_die: %p\n", in_die);
      result = handleOneDie(self, in_die, srcfiles, cnt, isSibling, dieAndChildrenIdx, &dieInfoIdx);
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
        handleDieAndChildren(self, child, srcfiles, cnt);
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
//printf("handleDieAndChildren done die: %p level: %d\n", in_die_in, childrenLevel--);
  return result;
}

// =================================== handleCompileUnits =========================== 

static uint8_t handleCompileUnits(dwarfDbgPtr_t self) {
  uint8_t result;
  const char * sectionName = 0;
  int res = 0;
  Dwarf_Bool isInfo = TRUE;
  Dwarf_Error err;
  unsigned loopCount = 0;
  int nres = DW_DLV_OK;
  int   compileUnitCount = 0;
  char * compileUnitShortName = NULL;
  char * compileUnitLongName = NULL;
  int i = 0;
  int fileLineIdx;
  Dwarf_Signed srcCnt = 0;
  char **srcfiles = 0;

  isInfo = TRUE;
  result = DWARF_DBG_ERR_OK;
  res = dwarf_get_die_section_name(self->elfInfo.dbg, isInfo, &sectionName, &err);
  if (res != DW_DLV_OK || !sectionName || !strlen(sectionName)) {
    sectionName = ".debug_info";
  }
  result = self->dwarfDbgCompileUnitInfo->getAddressSizeAndMax(self, &self->dwarfDbgCompileUnitInfo->addrSize, &self->dwarfDbgCompileUnitInfo->maxAddr, &err);
  DWARF_DBG_PRINT(self, "G", 1, "addrSize: 0x%08x maxAddr: 0x%08x\n", self->dwarfDbgCompileUnitInfo->addrSize, self->dwarfDbgCompileUnitInfo->maxAddr);
  /* Loop over compile units until it fails.  */
  for (;;++loopCount) {
    int sres = DW_DLV_OK;
    compileUnit_t *compileUnit = 0;

    result = self->dwarfDbgCompileUnitInfo->addCompileUnit(self);
    if (result == DWARF_DBG_ERR_NO_ENTRY) {
      // we have processed all entries
      result = DWARF_DBG_ERR_OK;
      break;
    }
    checkErrOK(result);
    compileUnit = self->dwarfDbgCompileUnitInfo->currCompileUnit;
    DWARF_DBG_PRINT(self, "G", 1, "handle srcfiles\n");
    {
      Dwarf_Error srcerr = 0;
      int srcf = dwarf_srcfiles(compileUnit->compileUnitDie, &srcfiles, &srcCnt, &err);

      if (srcf == DW_DLV_ERROR) {
        return DWRAF_DBG_ERR_GET_SRC_FILES;
      } /*DW_DLV_NO_ENTRY generally means there
        there is no DW_AT_stmt_list attribute.
        and we do not want to print anything
        about statements in that case */

      DWARF_DBG_PRINT(self, "G", 1, "srcCnt: %d currCompileUnitIdx: %d\n", srcCnt, self->dwarfDbgCompileUnitInfo->currCompileUnitIdx);
      for (i = 0; i < srcCnt; i++) {
        int pathNameIdx;
        int fileInfoIdx;

        DWARF_DBG_PRINT(self, "G", 1, "  src: %s\n", srcfiles[i]);
        result = self->dwarfDbgFileInfo->addSourceFile(self, srcfiles[i], &pathNameIdx, &fileInfoIdx);
//printf("  src: %s %d pathNameIdx: %d fileInfoIdx: %d\n", srcfiles[i], i, pathNameIdx, fileInfoIdx);
        checkErrOK(result);
      }

      compileUnit->isCompileUnitDie = 1;
      compileUnit->level = 0;
      result = handleDieAndChildren(self, compileUnit->compileUnitDie, srcfiles, srcCnt);
    }

    // add the typedefs here
    {
      int dieAndChildrenIdx;
printf("++ numDieAndChildren: %d cu: %s\n", compileUnit->numDieAndChildren, compileUnit->shortFileName);
  for (dieAndChildrenIdx = 0; dieAndChildrenIdx < compileUnit->numDieAndChildren; dieAndChildrenIdx++) {
printf("++ childIdx: %d\n", dieAndChildrenIdx);

printf("++ children: constTypes\n");
    result = self->dwarfDbgTypeInfo->addChildrenTypes(self, dieAndChildrenIdx, TAG_REF_CONST_TYPE);
    checkErrOK(result);
printf("++ siblings: constTypes\n");
    result = self->dwarfDbgTypeInfo->addSiblingsTypes(self, dieAndChildrenIdx, TAG_REF_CONST_TYPE);
    checkErrOK(result);

printf("++ children: enumerationTypes\n");
    result = self->dwarfDbgTypeInfo->addChildrenTypes(self, dieAndChildrenIdx, TAG_REF_ENUMERATION_TYPE);
    checkErrOK(result);
printf("++ siblings: enumerationTypes\n");
    result = self->dwarfDbgTypeInfo->addSiblingsTypes(self, dieAndChildrenIdx, TAG_REF_ENUMERATION_TYPE);
    checkErrOK(result);

printf("++ children: enumerators\n");
    result = self->dwarfDbgTypeInfo->addChildrenTypes(self, dieAndChildrenIdx, TAG_REF_ENUMERATOR);
    checkErrOK(result);
printf("++ siblings: enumerators\n");
    result = self->dwarfDbgTypeInfo->addSiblingsTypes(self, dieAndChildrenIdx, TAG_REF_ENUMERATOR);
    checkErrOK(result);

printf("++ children: members\n");
    result = self->dwarfDbgTypeInfo->addChildrenTypes(self, dieAndChildrenIdx, TAG_REF_MEMBER);
    checkErrOK(result);
printf("++ siblings: members\n");
    result = self->dwarfDbgTypeInfo->addSiblingsTypes(self, dieAndChildrenIdx, TAG_REF_MEMBER);
    checkErrOK(result);

printf("++ children: unionTypes\n");
    result = self->dwarfDbgTypeInfo->addChildrenTypes(self, dieAndChildrenIdx, TAG_REF_UNION_TYPE);
    checkErrOK(result);
printf("++ siblings: unionTypes\n");
    result = self->dwarfDbgTypeInfo->addSiblingsTypes(self, dieAndChildrenIdx, TAG_REF_UNION_TYPE);
    checkErrOK(result);

printf("++ children: volatileTypes\n");
    result = self->dwarfDbgTypeInfo->addChildrenTypes(self, dieAndChildrenIdx, TAG_REF_VOLATILE_TYPE);
    checkErrOK(result);
printf("++ siblings: volatileTypes\n");
    result = self->dwarfDbgTypeInfo->addSiblingsTypes(self, dieAndChildrenIdx, TAG_REF_VOLATILE_TYPE);
    checkErrOK(result);

printf("++ children: structureTypes\n");
    result = self->dwarfDbgTypeInfo->addChildrenTypes(self, dieAndChildrenIdx, TAG_REF_STRUCTURE_TYPE);
    checkErrOK(result);
printf("++ siblings: structureTypes\n");
    result = self->dwarfDbgTypeInfo->addSiblingsTypes(self, dieAndChildrenIdx, TAG_REF_STRUCTURE_TYPE);
    checkErrOK(result);

printf("++ children: pointerTypes\n");
    result = self->dwarfDbgTypeInfo->addChildrenTypes(self, dieAndChildrenIdx, TAG_REF_POINTER_TYPE);
    checkErrOK(result);
printf("++ siblings: pointerTypes\n");
    result = self->dwarfDbgTypeInfo->addSiblingsTypes(self, dieAndChildrenIdx, TAG_REF_POINTER_TYPE);
    checkErrOK(result);

printf("++ children: typedefs\n");
    result = self->dwarfDbgTypeInfo->addChildrenTypes(self, dieAndChildrenIdx, TAG_REF_TYPEDEF);
    checkErrOK(result);
printf("++ siblings: typedefs\n");
    result = self->dwarfDbgTypeInfo->addSiblingsTypes(self, dieAndChildrenIdx, TAG_REF_TYPEDEF);
    checkErrOK(result);

printf("++ children: subroutineTypes\n");
    result = self->dwarfDbgTypeInfo->addChildrenTypes(self, dieAndChildrenIdx, TAG_REF_SUBROUTINE_TYPE);
    checkErrOK(result);
printf("++ siblings: subroutineTypes\n");
    result = self->dwarfDbgTypeInfo->addSiblingsTypes(self, dieAndChildrenIdx, TAG_REF_SUBROUTINE_TYPE);
    checkErrOK(result);

printf("++ children: arrayTypes\n");
    result = self->dwarfDbgTypeInfo->addChildrenTypes(self, dieAndChildrenIdx, TAG_REF_ARRAY_TYPE);
    checkErrOK(result);
printf("++ siblings: arrayTypes\n");
    result = self->dwarfDbgTypeInfo->addSiblingsTypes(self, dieAndChildrenIdx, TAG_REF_ARRAY_TYPE);
    checkErrOK(result);

printf("++ children: formalParameters\n");
    result = self->dwarfDbgTypeInfo->addChildrenTypes(self, dieAndChildrenIdx, TAG_REF_FORMAL_PARAMETER);
    checkErrOK(result);
printf("++ siblings: formalParameters\n");
    result = self->dwarfDbgTypeInfo->addSiblingsTypes(self, dieAndChildrenIdx, TAG_REF_FORMAL_PARAMETER);
    checkErrOK(result);

printf("++ children: lexicalBlocks\n");
    result = self->dwarfDbgTypeInfo->addChildrenTypes(self, dieAndChildrenIdx, TAG_REF_LEXICAL_BLOCK);
    checkErrOK(result);
printf("++ siblings: lexicalBlocks\n");
    result = self->dwarfDbgTypeInfo->addSiblingsTypes(self, dieAndChildrenIdx, TAG_REF_LEXICAL_BLOCK);
    checkErrOK(result);

printf("++ children: labels\n");
    result = self->dwarfDbgTypeInfo->addChildrenTypes(self, dieAndChildrenIdx, TAG_REF_LABEL);
    checkErrOK(result);
printf("++ siblings: labels\n");
    result = self->dwarfDbgTypeInfo->addSiblingsTypes(self, dieAndChildrenIdx, TAG_REF_LABEL);
    checkErrOK(result);

printf("++ children: inlinedSubroutine\n");
    result = self->dwarfDbgTypeInfo->addChildrenTypes(self, dieAndChildrenIdx, TAG_REF_INLINED_SUBROUTINE);
    checkErrOK(result);
printf("++ siblings: inlinedSubroutine\n");
    result = self->dwarfDbgTypeInfo->addSiblingsTypes(self, dieAndChildrenIdx, TAG_REF_INLINED_SUBROUTINE);
    checkErrOK(result);

printf("++ children: subranges\n");
    result = self->dwarfDbgTypeInfo->addChildrenTypes(self, dieAndChildrenIdx, TAG_REF_SUBRANGE_TYPE);
    checkErrOK(result);
printf("++ siblings: subranges\n");
    result = self->dwarfDbgTypeInfo->addSiblingsTypes(self, dieAndChildrenIdx, TAG_REF_SUBRANGE_TYPE);
    checkErrOK(result);

printf("++ children: subprograms\n");
    result = self->dwarfDbgTypeInfo->addChildrenTypes(self, dieAndChildrenIdx, TAG_REF_SUBPROGRAM);
    checkErrOK(result);
printf("++ siblings: subprograms\n");
    result = self->dwarfDbgTypeInfo->addSiblingsTypes(self, dieAndChildrenIdx, TAG_REF_SUBPROGRAM);
    checkErrOK(result);

printf("++ children: GNUCallSites\n");
    result = self->dwarfDbgTypeInfo->addChildrenTypes(self, dieAndChildrenIdx, TAG_REF_GNU_CALL_SITE);
    checkErrOK(result);
printf("++ siblings: GNUCallSites\n");
    result = self->dwarfDbgTypeInfo->addSiblingsTypes(self, dieAndChildrenIdx, TAG_REF_GNU_CALL_SITE);
    checkErrOK(result);

printf("++ children: GNUCallSiteParameters\n");
    result = self->dwarfDbgTypeInfo->addChildrenTypes(self, dieAndChildrenIdx, TAG_REF_GNU_CALL_SITE_PARAMETER);
    checkErrOK(result);
printf("++ siblings: GNUCallSiteParameters\n");
    result = self->dwarfDbgTypeInfo->addSiblingsTypes(self, dieAndChildrenIdx, TAG_REF_GNU_CALL_SITE_PARAMETER);
    checkErrOK(result);

printf("++ children: variables\n");
    result = self->dwarfDbgTypeInfo->addChildrenTypes(self, dieAndChildrenIdx, TAG_REF_VARIABLE);
    checkErrOK(result);
printf("++ siblings: variables\n");
    result = self->dwarfDbgTypeInfo->addSiblingsTypes(self, dieAndChildrenIdx, TAG_REF_VARIABLE);
    checkErrOK(result);

printf("++ children: unspecifiedParameters\n");
    result = self->dwarfDbgTypeInfo->addChildrenTypes(self, dieAndChildrenIdx, TAG_REF_UNSPECIFIED_PARAMETERS);
    checkErrOK(result);
printf("++ siblings: unspecifiedParameters\n");
    result = self->dwarfDbgTypeInfo->addSiblingsTypes(self, dieAndChildrenIdx, TAG_REF_UNSPECIFIED_PARAMETERS);
    checkErrOK(result);

  }
  result = self->dwarfDbgTypeInfo->checkDieTypeRefIdx(self);
fflush(showFd);

}
    
//#define SHOWSTRUCTURE
#ifdef SHOWSTRUCTURE
// for testing show the structure
{
  int dieAndChildrenIdx;
//  showFd = fopen("showInfo.txt", "w");

fprintf(showFd, "++ numDieAndChildren: %d cu: %s\n", compileUnit->numDieAndChildren, compileUnit->shortFileName);
  for (dieAndChildrenIdx = 0; dieAndChildrenIdx < compileUnit->numDieAndChildren; dieAndChildrenIdx++) {
fprintf(showFd, "++ childIdx: %d\n", dieAndChildrenIdx);
fprintf(showFd, "++ children:\n");
    self->dwarfDbgDieInfo->showChildren(self, dieAndChildrenIdx, "  ");
fprintf(showFd, "++ siblings:\n");
    self->dwarfDbgDieInfo->showSiblings(self, dieAndChildrenIdx, "  ");
  }
fflush(showFd);

}
#endif
    // eventually handle ranges here
    // here we handle source lines
    result = self->dwarfDbgLineInfo->handleLineInfos(self, &fileLineIdx);
//printf("fileLineIdx: %d\n", fileLineIdx);
    checkErrOK(result);

    /*  Release the 'compileUnitDie' created by the call
        to 'dwarf_siblingof' at the top of the main loop. */
    dwarf_dealloc(self->elfInfo.dbg, compileUnit->compileUnitDie, DW_DLA_DIE);
    compileUnit->compileUnitDie = NULL; /* For debugging, stale die should be NULL. */
int i ;
compileUnit = self->dwarfDbgCompileUnitInfo->currCompileUnit;
for (i = 0; i < compileUnit->numSourceFile; i++) {
//printf(">>source: %d %d\n", i, compileUnitInfo->sourceFiles[i]);
}
  } // end loop
  return result;
}

// =================================== dwarfDbgGetDbgInfos =========================== 

int dwarfDbgGetDbgInfos(dwarfDbgPtr_t self) {
  uint8_t result;
  char *chunk;

//printf("dwarfDbgGetDbgInfos\n");
  result = DWARF_DBG_ERR_OK;
  // for performance alloc a big chunk of memory and free it imidiately again
  chunk = ckalloc(10*1024*1024);
  ckfree(chunk);
  result = self->dwarfDbgFrameInfo->getFrameList(self);
  DWARF_DBG_PRINT(self, "G", 1, "getFrameLists: result: %d\n", result);
  checkErrOK(result);
  result = self->dwarfDbgGetDbgInfo->handleCompileUnits(self);
  DWARF_DBG_PRINT(self, "G", 1, "handleCompileUnits: result: %d\n", result);
  checkErrOK(result);
printf("numDwBaseTypes: %d numDwTypeDefs: %d\n", self->dwarfDbgTypeInfo->dwBaseTypeInfos.numDwType, self->dwarfDbgTypeInfo->dwTypedefInfos.numDwType);
  return result;
}

// =================================== dwarfDbgGetDbgInfoInit =========================== 

int dwarfDbgGetDbgInfoInit (dwarfDbgPtr_t self) {

  self->dwarfDbgGetDbgInfo->handleCompileUnits = &handleCompileUnits;
  return DWARF_DBG_ERR_OK;
}
