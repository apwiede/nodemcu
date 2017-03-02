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
 * File:   dwarfDbgTypeInfo.c
 * Author: Arnulf P. Wiedemann <arnulf@wiedemann-pri.de>
 *
 * Created on February 18, 2017
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libelf.h>
#include <stdlib.h>
#include <string.h>

#include "dwarfDbgInt.h"

#define BASE64_INVALID '\xff'
#define BASE64_PADDING '='
#define ISBASE64(c) (unbytes64[c] != BASE64_INVALID)

static const char b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

FILE *typeFd;

// =================================== addTypeStr =========================== 

static uint8_t addTypeStr(dwarfDbgPtr_t self, const char *str, int *typeStrIdx) {
  uint8_t result;
  char **typeStr;
  int strIdx;

  result = DWARF_DBG_ERR_OK;
  for(strIdx = 0; strIdx < self->dwarfDbgTypeInfo->numTypeStr; strIdx++) {
    if (strcmp(self->dwarfDbgTypeInfo->typeStrs[strIdx], str) == 0) {
      *typeStrIdx = strIdx;
      return result;
    }
  }
  if (self->dwarfDbgTypeInfo->maxTypeStr <= self->dwarfDbgTypeInfo->numTypeStr) {
    self->dwarfDbgTypeInfo->maxTypeStr += 10;
    if (self->dwarfDbgTypeInfo->typeStrs == NULL) {
      self->dwarfDbgTypeInfo->typeStrs = (char **)ckalloc(sizeof(char *) * self->dwarfDbgTypeInfo->maxTypeStr);
      if (self->dwarfDbgTypeInfo->typeStrs == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      self->dwarfDbgTypeInfo->typeStrs = (char **)ckrealloc((char *)self->dwarfDbgTypeInfo->typeStrs, sizeof(char *) * self->dwarfDbgTypeInfo->maxTypeStr);
      if (self->dwarfDbgTypeInfo->typeStrs == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
//printf("== numTypeStrs: %d %s\n", self->dwarfDbgTypeInfo->numTypeStr, str);
  typeStr = &self->dwarfDbgTypeInfo->typeStrs[self->dwarfDbgTypeInfo->numTypeStr];
  *typeStr = ckalloc(strlen(str) + 1);
  memset(*typeStr, 0, strlen(str) + 1);
  memcpy(*typeStr, str, strlen(str));
  *typeStrIdx = self->dwarfDbgTypeInfo->numTypeStr;
  self->dwarfDbgTypeInfo->numTypeStr++;
  return result;
}

// ============================= toBase64 ========================

/**
 * \brief encode message with base64
 * \param msg The message
 * \param len The length of the message
 * \param encoded The out param encoded message
 * \return Error code or ErrorOK
 *
 */
static uint8_t toBase64(dwarfDbgPtr_t self, const char *msg, size_t *len, char **encoded) {
  size_t i;
  size_t n;
  char *q;
  char *out;
  char bytes64[sizeof(b64)];

  n = *len;
  if (!n) { // handle empty string case 
    return DWARF_DBG_ERR_OUT_OF_MEMORY;
  }
  out = (char *)ckalloc(((n + 2) / 3 * 4) + 1);
  if (out == NULL) {
    return DWARF_DBG_ERR_OUT_OF_MEMORY;
  }
  memcpy(bytes64, b64, sizeof(b64));   //Avoid lots of flash unaligned fetches

  for (i = 0, q = out; i < n; i += 3) {
    int a = msg[i];
    int b = (i + 1 < n) ? msg[i + 1] : 0;
    int c = (i + 2 < n) ? msg[i + 2] : 0;
    *q++ = bytes64[a >> 2];
    *q++ = bytes64[((a & 3) << 4) | (b >> 4)];
    *q++ = (i + 1 < n) ? bytes64[((b & 15) << 2) | (c >> 6)] : BASE64_PADDING;
    *q++ = (i + 2 < n) ? bytes64[(c & 63)] : BASE64_PADDING;
  }
  *q = '\0';
  *len = q - out;
  // ATTENTION the caller has to free *encoded!!
  *encoded = out;
  return DWARF_DBG_ERR_OK;
}

// =================================== genAttrTypeInfoKey =========================== 

 /* we add the tag field and the attr infos to a binary string and make a base64 key out of it */

static uint8_t genAttrTypeInfoKey(dwarfDbgPtr_t self, dwAttrTypeInfo_t *dwAttrTypeInfo, char **attrTypeInfoKey) {
  uint8_t result;
  char buf[1024];
  char buf2[50];
  int attrIdx = 0;
  dwAttrType_t *dwAttrType = NULL;

  result = DWARF_DBG_ERR_OK;
  buf[0] = '\0';
  sprintf(buf2, "%04x,", dwAttrTypeInfo->tag);
  strcat(buf, buf2);
  for (attrIdx = 0; attrIdx < dwAttrTypeInfo->numDwAttrType; attrIdx++) {
    dwAttrType = &dwAttrTypeInfo->dwAttrTypes[attrIdx];
    sprintf(buf2, "%04x,%08x,%08x.", dwAttrType->dwType, dwAttrType->value, dwAttrType->refOffset);
    if (strlen(buf) + strlen(buf2) > 1020) {
printf("ERROR buf for key to small\n");
      return DWARF_DBG_ERR_KEY_BUF_TOO_SMALL;
    }
    strcat(buf, buf2);
  }
  *attrTypeInfoKey = strdup(buf);
  return result;
}

// =================================== getAttrTypeInfos =========================== 

static uint8_t getAttrTypeInfos(dwarfDbgPtr_t self, int tag, dwAttrTypeInfos_t **dwAttrTypeInfos, Tcl_HashTable **attrTypeInfoHashTable) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;
  switch (tag) {
  case DW_TAG_array_type:
    *dwAttrTypeInfos = &self->dwarfDbgTypeInfo->dwArrayTypeInfos;
    *attrTypeInfoHashTable = &self->dwarfDbgTypeInfo->dwArrayTypeHashTable;
    break;
  case DW_TAG_base_type:
    *dwAttrTypeInfos = &self->dwarfDbgTypeInfo->dwBaseTypeInfos;
    *attrTypeInfoHashTable = &self->dwarfDbgTypeInfo->dwBaseTypeHashTable;
    break;
  case DW_TAG_compile_unit:
    *dwAttrTypeInfos = &self->dwarfDbgTypeInfo->dwCompileUnitInfos;
    *attrTypeInfoHashTable = &self->dwarfDbgTypeInfo->dwCompileUnitHashTable;
    break;
  case DW_TAG_const_type:
    *dwAttrTypeInfos = &self->dwarfDbgTypeInfo->dwConstTypeInfos;
    *attrTypeInfoHashTable = &self->dwarfDbgTypeInfo->dwConstTypeHashTable;
    break;
  case DW_TAG_enumeration_type:
    *dwAttrTypeInfos = &self->dwarfDbgTypeInfo->dwEnumerationTypeInfos;
    *attrTypeInfoHashTable = &self->dwarfDbgTypeInfo->dwEnumerationTypeHashTable;
    break;
  case DW_TAG_enumerator:
    *dwAttrTypeInfos = &self->dwarfDbgTypeInfo->dwEnumeratorInfos;
    *attrTypeInfoHashTable = &self->dwarfDbgTypeInfo->dwEnumeratorHashTable;
    break;
  case DW_TAG_formal_parameter:
    *dwAttrTypeInfos = &self->dwarfDbgTypeInfo->dwFormalParameterInfos;
    *attrTypeInfoHashTable = &self->dwarfDbgTypeInfo->dwFormalParameterHashTable;
    break;
  case DW_TAG_GNU_call_site:
    *dwAttrTypeInfos = &self->dwarfDbgTypeInfo->dwGNUCallSiteInfos;
    *attrTypeInfoHashTable = &self->dwarfDbgTypeInfo->dwGNUCallSiteHashTable;
    break;
  case DW_TAG_GNU_call_site_parameter:
    *dwAttrTypeInfos = &self->dwarfDbgTypeInfo->dwGNUCallSiteParameterInfos;
    *attrTypeInfoHashTable = &self->dwarfDbgTypeInfo->dwGNUCallSiteParameterHashTable;
    break;
  case DW_TAG_inlined_subroutine:
    *dwAttrTypeInfos = &self->dwarfDbgTypeInfo->dwInlinedSubroutineInfos;
    *attrTypeInfoHashTable = &self->dwarfDbgTypeInfo->dwInlinedSubroutineHashTable;
    break;
  case DW_TAG_label:
    *dwAttrTypeInfos = &self->dwarfDbgTypeInfo->dwLabelInfos;
    *attrTypeInfoHashTable = &self->dwarfDbgTypeInfo->dwLabelHashTable;
    break;
  case DW_TAG_lexical_block:
    *dwAttrTypeInfos = &self->dwarfDbgTypeInfo->dwLexicalBlockInfos;
    *attrTypeInfoHashTable = &self->dwarfDbgTypeInfo->dwLexicalBlockHashTable;
    break;
  case DW_TAG_member:
    *dwAttrTypeInfos = &self->dwarfDbgTypeInfo->dwMemberInfos;
    *attrTypeInfoHashTable = &self->dwarfDbgTypeInfo->dwMemberHashTable;
    break;
  case DW_TAG_pointer_type:
    *dwAttrTypeInfos = &self->dwarfDbgTypeInfo->dwPointerTypeInfos;
    *attrTypeInfoHashTable = &self->dwarfDbgTypeInfo->dwPointerTypeHashTable;
    break;
  case DW_TAG_structure_type:
    *dwAttrTypeInfos = &self->dwarfDbgTypeInfo->dwStructureTypeInfos;
    *attrTypeInfoHashTable = &self->dwarfDbgTypeInfo->dwStructureTypeHashTable;
    break;
  case DW_TAG_subprogram:
    *dwAttrTypeInfos = &self->dwarfDbgTypeInfo->dwSubprogramTypeInfos;
    *attrTypeInfoHashTable = &self->dwarfDbgTypeInfo->dwSubprogramTypeHashTable;
    break;
  case DW_TAG_subrange_type:
    *dwAttrTypeInfos = &self->dwarfDbgTypeInfo->dwSubrangeInfos;
    *attrTypeInfoHashTable = &self->dwarfDbgTypeInfo->dwSubrangeHashTable;
    break;
  case DW_TAG_subroutine_type:
    *dwAttrTypeInfos = &self->dwarfDbgTypeInfo->dwSubroutineTypeInfos;
    *attrTypeInfoHashTable = &self->dwarfDbgTypeInfo->dwSubroutineTypeHashTable;
    break;
  case DW_TAG_typedef:
    *dwAttrTypeInfos = &self->dwarfDbgTypeInfo->dwTypedefInfos;
    *attrTypeInfoHashTable = &self->dwarfDbgTypeInfo->dwTypedefHashTable;
    break;
  case DW_TAG_union_type:
    *dwAttrTypeInfos = &self->dwarfDbgTypeInfo->dwUnionTypeInfos;
    *attrTypeInfoHashTable = &self->dwarfDbgTypeInfo->dwUnionTypeHashTable;
    break;
  case DW_TAG_unspecified_parameters:
printf("dwUnspecifiedParametersInfos: num: %d\n", self->dwarfDbgTypeInfo->dwUnspecifiedParametersInfos.numDwAttrTypeInfo);
    *dwAttrTypeInfos = &self->dwarfDbgTypeInfo->dwUnspecifiedParametersInfos;
    *attrTypeInfoHashTable = &self->dwarfDbgTypeInfo->dwUnspecifiedParametersHashTable;
    break;
  case DW_TAG_variable:
    *dwAttrTypeInfos = &self->dwarfDbgTypeInfo->dwVariableInfos;
    *attrTypeInfoHashTable = &self->dwarfDbgTypeInfo->dwVariableHashTable;
    break;
  case DW_TAG_volatile_type:
    *dwAttrTypeInfos = &self->dwarfDbgTypeInfo->dwVolatileTypeInfos;
    *attrTypeInfoHashTable = &self->dwarfDbgTypeInfo->dwVolatileTypeHashTable;
    break;
  }
  return result;
}

// =================================== addAttrType =========================== 

static uint8_t addAttrType(dwarfDbgPtr_t self, dwAttrTypeInfo_t *attrTypeInfo, int dwType, int value, int refOffset, int *attrTypeIdx) {
  uint8_t result = 0;
  int typeStrIdx = 0;
  int idx = 0;
  int entryIdx = 0;
  int found = 0;
  dwAttrType_t *dwAttrType = NULL;

  result = DWARF_DBG_ERR_OK;
  if (attrTypeInfo->maxDwAttrType <= attrTypeInfo->numDwAttrType) {
    attrTypeInfo->maxDwAttrType += 5;
    if (attrTypeInfo->dwAttrTypes == NULL) {
      attrTypeInfo->dwAttrTypes = (dwAttrType_t *)ckalloc(sizeof(dwAttrType_t) * attrTypeInfo->maxDwAttrType);
      if (attrTypeInfo->dwAttrTypes == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      attrTypeInfo->dwAttrTypes = (dwAttrType_t *)ckrealloc((char *)attrTypeInfo->dwAttrTypes, sizeof(dwAttrType_t) * attrTypeInfo->maxDwAttrType);
      if (attrTypeInfo->dwAttrTypes == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
  dwAttrType = &attrTypeInfo->dwAttrTypes[attrTypeInfo->numDwAttrType];
  dwAttrType->dwType = dwType;
  dwAttrType->value = value;
  dwAttrType->refOffset = refOffset;
  *attrTypeIdx = attrTypeInfo->numDwAttrType;
  attrTypeInfo->numDwAttrType++;
  return result;
}

// =================================== addAttrTypeInfo =========================== 

static uint8_t addAttrTypeInfo(dwarfDbgPtr_t self, dwAttrTypeInfo_t *dwAttrTypeInfoIn, int numAttr, int *dwAttrTypeInfoIdx) {
  uint8_t result = 0;
  int typeStrIdx = 0;
  int idx = 0;
  int entryIdx = 0;
  int found = 0;
  int attrTypeIdx = 0;
  dwAttrTypeInfos_t *dwAttrTypeInfos = NULL;
  dwAttrTypeInfo_t *dwAttrTypeInfo = NULL;
  dwAttrType_t *dwAttrType = NULL;
  dwAttrType_t *dwAttrTypeIn = NULL;
  const char *typeName = NULL;
  const char *tagName = NULL;
  char *name;
  char *dirName;
  char pathName[255];
  char *attrTypeInfoKey = NULL;
  pathNameInfo_t *pathNameInfo;
  int attrTypeInfoIdx = 0;
  int newEntry;
  Tcl_HashEntry *hPtr = NULL;
  Tcl_HashTable *attrTypeInfoHashTable = NULL;

  self->dwarfDbgTypeInfo->typeLevel++;
  result = DWARF_DBG_ERR_OK;
  // generate a key for the hashTables
  result = genAttrTypeInfoKey(self, dwAttrTypeInfoIn, &attrTypeInfoKey);
  checkErrOK(result);
  result = self->dwarfDbgTypeInfo->getAttrTypeInfos(self, dwAttrTypeInfoIn->dieInfo->tag, &dwAttrTypeInfos, &attrTypeInfoHashTable);
  checkErrOK(result);
  hPtr = Tcl_CreateHashEntry(attrTypeInfoHashTable, attrTypeInfoKey, &newEntry);
printf("attrTypeInfoKey: %s newEntry: %d\n", attrTypeInfoKey, newEntry);
  if (hPtr == NULL) {
    return DWARF_DBG_ERR_CANNOT_CREATE_HASH_ENTRY;
  }
  if (!newEntry) {
    hPtr = Tcl_FindHashEntry(attrTypeInfoHashTable, attrTypeInfoKey);
    attrTypeInfoIdx = (int)((long int)Tcl_GetHashValue(hPtr));
printf("found attrTypeInfoIdx: %d\n", attrTypeInfoIdx);
result = self->dwarfDbgTypeInfo->printAttrTypeInfo(self, dwAttrTypeInfoIn->dieInfo->tag, attrTypeInfoIdx, 3, "");
checkErrOK(result);
    *dwAttrTypeInfoIdx = attrTypeInfoIdx;
    return result;
  }
//printf("SUB: dieInfo: %p\n", dwAttrTypeInfoIn->dieInfo);
self->dwarfDbgStringInfo->getDW_TAG_string(self, dwAttrTypeInfoIn->dieInfo->tag, &tagName);
printf("addAttrTypeInfo: %s\n", tagName);
  if (dwAttrTypeInfos->maxDwAttrTypeInfo <= dwAttrTypeInfos->numDwAttrTypeInfo) {
    dwAttrTypeInfos->maxDwAttrTypeInfo += 5;
    if (dwAttrTypeInfos->dwAttrTypeInfos == NULL) {
      dwAttrTypeInfos->dwAttrTypeInfos = (dwAttrTypeInfo_t *)ckalloc(sizeof(dwAttrTypeInfo_t) * dwAttrTypeInfos->maxDwAttrTypeInfo);
      if (dwAttrTypeInfos->dwAttrTypeInfos == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      dwAttrTypeInfos->dwAttrTypeInfos = (dwAttrTypeInfo_t *)ckrealloc((char *)dwAttrTypeInfos->dwAttrTypeInfos, sizeof(dwAttrTypeInfo_t) * dwAttrTypeInfos->maxDwAttrTypeInfo);
      if (dwAttrTypeInfos->dwAttrTypeInfos == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
  dwAttrTypeInfo = &dwAttrTypeInfos->dwAttrTypeInfos[dwAttrTypeInfos->numDwAttrTypeInfo];
  memset(dwAttrTypeInfo, 0, sizeof(dwAttrTypeInfo_t));
  dwAttrTypeInfo->tag = dwAttrTypeInfoIn->tag;
  for (idx = 0; idx < dwAttrTypeInfoIn->numDwAttrType; idx++) {
    dwAttrTypeIn = &dwAttrTypeInfoIn->dwAttrTypes[idx];
    result = self->dwarfDbgTypeInfo->addAttrType(self, dwAttrTypeInfo, dwAttrTypeIn->dwType, dwAttrTypeIn->value, dwAttrTypeIn->refOffset, &attrTypeIdx);
    checkErrOK(result);
  }
  *dwAttrTypeInfoIdx = dwAttrTypeInfos->numDwAttrTypeInfo;
printf("addAttrTypeInfo: new: %d %s\n", dwAttrTypeInfos->numDwAttrTypeInfo, tagName);
  Tcl_SetHashValue(hPtr, (char *)((long int)dwAttrTypeInfos->numDwAttrTypeInfo));
result = self->dwarfDbgTypeInfo->printAttrTypeInfo(self, dwAttrTypeInfoIn->tag, dwAttrTypeInfos->numDwAttrTypeInfo, 3, "");
checkErrOK(result);
  dwAttrTypeInfos->numDwAttrTypeInfo++;
  self->dwarfDbgTypeInfo->typeLevel--;
  return result;
}

// =================================== printAttrTypeInfo =========================== 

static uint8_t printAttrTypeInfo(dwarfDbgPtr_t self, int tag, int dwAttrTypeInfoIdx, int isSibling, const char *indent) {
  uint8_t result;
  dwAttrTypeInfos_t *dwAttrTypeInfos = NULL;
  dwAttrTypeInfo_t *dwAttrTypeInfo = NULL;
  dwAttrType_t *dwAttrType = NULL;
  Tcl_HashTable *attrTypeInfoHashTable = NULL;
  int attrIdx = 0;
  const char *tagName = NULL;
  const char *typeName = NULL;
  char *name = NULL;
  char *dirName = NULL;
  pathNameInfo_t *pathNameInfo = NULL;
  char pathName[255];

  result = DWARF_DBG_ERR_OK;
  result = self->dwarfDbgStringInfo->getDW_TAG_string(self, tag, &tagName);
  checkErrOK(result);
printf("printAttrTypeInfo: tag: %s\n", tagName);
  result = getAttrTypeInfos(self, tag, &dwAttrTypeInfos, &attrTypeInfoHashTable);
  checkErrOK(result);
  if ((dwAttrTypeInfoIdx < 0) || (dwAttrTypeInfoIdx > dwAttrTypeInfos->numDwAttrTypeInfo)) {
printf("ERROR bad numDwAttrTypeInfo dwAttrTypeInfoIdx: dwAttrTypeInfoIdx: %d num: %d\n", dwAttrTypeInfoIdx, dwAttrTypeInfos->numDwAttrTypeInfo);
    return DWARF_DBG_ERR_BAD_DW_ATTR_TYPE_INFOS_IDX;
  }
  dwAttrTypeInfo = &dwAttrTypeInfos->dwAttrTypeInfos[dwAttrTypeInfoIdx];
  result = self->dwarfDbgStringInfo->getDW_TAG_string(self, dwAttrTypeInfo->tag, &tagName);
//printf("getDW_TAG_string: result: %d tag: 0x%04x\n", result, dwAttrTypeInfo->tag);
  checkErrOK(result);
  fprintf(typeFd, "%s>>TAG: %s 0x%04x dwAttrTypeInfoIdx: %d isSibling: %d\n", indent, tagName, dwAttrTypeInfo->tag, dwAttrTypeInfoIdx, isSibling);
  for (attrIdx = 0; attrIdx < dwAttrTypeInfo->numDwAttrType; attrIdx++) {
    dwAttrType = &dwAttrTypeInfo->dwAttrTypes[attrIdx];
    result = self->dwarfDbgStringInfo->getDW_AT_string(self, dwAttrType->dwType, &typeName);
    checkErrOK(result);
    name = "";
    pathName[0] = '\0';
    switch (dwAttrType->dwType) {
    case DW_AT_name:
      name = self->dwarfDbgTypeInfo->typeStrs[dwAttrType->value];
      break;
    case DW_AT_decl_file:
      pathNameInfo = &self->dwarfDbgFileInfo->pathNamesInfo.pathNames[dwAttrType->value];
      dirName = self->dwarfDbgFileInfo->dirNamesInfo.dirNames[pathNameInfo->dirNameIdx];
      sprintf(pathName,"%s/%s", dirName, pathNameInfo->fileName);
      break;
    }
    fprintf(typeFd, "%s    %s: %d offset: 0x%08x %s%s\n", indent, typeName, dwAttrType->value, dwAttrType->refOffset, name, pathName);
    fflush(typeFd);
  }
  return result;
}

// =================================== checkDieTypeRefIdx =========================== 

static uint8_t checkDieTypeRefIdx(dwarfDbgPtr_t self) {
  uint8_t result;
  int dieAndChildrenIdx = 0;
  int total = 0;
  int isSibling = 0;
  int maxEntries = 0;
  int typeIdx = 0;
  compileUnit_t *compileUnit;
  dieAndChildrenInfo_t *dieAndChildrenInfo = NULL;
  dieInfo_t *dieInfo = NULL;
  const char *tagName2 = NULL;
  const char *tagName = NULL;

  result = DWARF_DBG_ERR_OK;
  compileUnit = self->dwarfDbgCompileUnitInfo->currCompileUnit;
//printf("compileUnit: %p\n", compileUnit);
  for (isSibling = 0; isSibling < 2; isSibling++) {
    for (dieAndChildrenIdx = 0; dieAndChildrenIdx < compileUnit->numDieAndChildren; dieAndChildrenIdx++) {
      dieAndChildrenInfo = &compileUnit->dieAndChildrenInfos[dieAndChildrenIdx];
//printf("dieAndChildrenInfo: %p\n", dieAndChildrenInfo);
      if (isSibling) {
        maxEntries = dieAndChildrenInfo->numSiblings;
      } else {
        maxEntries = dieAndChildrenInfo->numChildren;
      }
      for(typeIdx = 0; typeIdx < maxEntries; typeIdx++) {
        if (isSibling) {
          dieInfo = &dieAndChildrenInfo->dieSiblings[typeIdx];
        } else {
          dieInfo = &dieAndChildrenInfo->dieChildren[typeIdx];
        }
//printf("dieInfo: %p isSibling: %d maxEntries: %d typeIdx: %d\n", dieInfo, isSibling, maxEntries, typeIdx);
self->dwarfDbgStringInfo->getDW_TAG_string(self, dieInfo->tag, &tagName);
//printf("tag: %s 0x%04x typeIdx: %d offset: 0x%08x refOffset: 0x%08x dieInfo: %p\n", tagName, dieInfo->tag, typeIdx, dieInfo->offset, dieAttr->refOffset, dieInfo);
        total++;
        printf("dieInfo: %p total: %d dieAndChildrenIdx: %d tagName: %s tagRefIdx: %d\n", dieInfo, total, dieAndChildrenIdx, tagName, dieInfo->tagRefIdx);
      }
    }
  }
fflush(stdout);
  return result;
}

// =================================== getTypeRefIdx =========================== 

static uint8_t getTypeRefIdx(dwarfDbgPtr_t self, dieAttr_t *dieAttr, int *dwTypeIdx) {
  uint8_t result;
  int dieAndChildrenIdx = 0;
  int isSibling = 0;
  int maxEntries = 0;
  int typeIdx = 0;
  int dwAttrTypeInfoIdx = 0;
  compileUnit_t *compileUnit;
  dieAndChildrenInfo_t *dieAndChildrenInfo = NULL;
  dieInfo_t *dieInfo = NULL;
  const char *tagName = NULL;
  int siblingTagInfoIdx = 0;
  int childTagInfoIdx = 0;

  result = DWARF_DBG_ERR_OK;
  compileUnit = self->dwarfDbgCompileUnitInfo->currCompileUnit;
//printf("compileUnit: %p\n", compileUnit);
  for (isSibling = 0; isSibling < 2; isSibling++) {
    for (dieAndChildrenIdx = 0; dieAndChildrenIdx < compileUnit->numDieAndChildren; dieAndChildrenIdx++) {
      dieAndChildrenInfo = &compileUnit->dieAndChildrenInfos[dieAndChildrenIdx];
//printf("dieAndChildrenInfo: %p\n", dieAndChildrenInfo);
      if (isSibling) {
        maxEntries = dieAndChildrenInfo->numSiblings;
      } else {
        maxEntries = dieAndChildrenInfo->numChildren;
      }
      for(typeIdx = 0; typeIdx < maxEntries; typeIdx++) {
        if (isSibling) {
          dieInfo = &dieAndChildrenInfo->dieSiblings[typeIdx];
        } else {
          dieInfo = &dieAndChildrenInfo->dieChildren[typeIdx];
        }
//printf("dieInfo: %p dieAttr: %p isSibling: %d maxEntries: %d typeIdx: %d\n", dieInfo, dieAttr, isSibling, maxEntries, typeIdx);
result = self->dwarfDbgStringInfo->getDW_TAG_string(self, dieInfo->tag, &tagName);
//printf("getTypeRefIdx: tag: %s 0x%04x typeIdx: %d offset: 0x%08x refOffset: 0x%08x dieInfo: %p\n", tagName, dieInfo->tag, typeIdx, dieInfo->offset, dieAttr->refOffset, dieInfo);

        if ((dieInfo->offset == dieAttr->refOffset) && (dieInfo->tagRefIdx == -1)) {
printf("Warning missing tagRefIdx for: %s\n", tagName);
          result =self->dwarfDbgTypeInfo->handleType(self, dieInfo, &dwAttrTypeInfoIdx);
          checkErrOK(result);
//printf("after handleType: %s dwAttrTypeInfoIdx: %d dieAndChildrenIdx: %d\n", tagName, dwAttrTypeInfoIdx, dieAndChildrenIdx);
        if (isSibling) {
          result = self->dwarfDbgDieInfo->addDieSiblingTagInfo(self, dieAndChildrenIdx, dieInfo->tag, dwAttrTypeInfoIdx, dieInfo->numAttr, &siblingTagInfoIdx);
        } else {
          result = self->dwarfDbgDieInfo->addDieChildTagInfo(self, dieAndChildrenIdx, dieInfo->tag, dwAttrTypeInfoIdx, dieInfo->numAttr, &childTagInfoIdx);
        }
        checkErrOK(result);
        }
        if (dieInfo->offset == dieAttr->refOffset) {
//          result = self->dwarfDbgTypeInfo->printAttrTypeInfo(self, dieInfo->tagRefIdx, isSibling, "\n");
//          checkErrOK(result);
        }
        switch (dieInfo->tag) {
        case DW_TAG_array_type:
          if (dieInfo->offset == dieAttr->refOffset) {
//printf("found type: %s arrayTypeIdx: %d dieInfo->offset: 0x%08x tagRefIdx: %d\n", tagName, typeIdx, dieInfo->offset, dieInfo->tagRefIdx);
            *dwTypeIdx = dieInfo->tagRefIdx;
            return result;
          }
          break;
        case DW_TAG_base_type:
          if (dieInfo->offset == dieAttr->refOffset) {
//printf("found type: %s baseTypeIdx: %d dieInfo->offset: 0x%08x tagRefIdx: %d\n", tagName, typeIdx, dieInfo->offset, dieInfo->tagRefIdx);
            *dwTypeIdx = dieInfo->tagRefIdx;
            return result;
          }
          break;
        case DW_TAG_const_type:
          if (dieInfo->offset == dieAttr->refOffset) {
//printf("found type: %s constTypeIdx: %d dieInfo->offset: 0x%08x tagRefIdx: %d\n", tagName, typeIdx, dieInfo->offset, dieInfo->tagRefIdx);
            *dwTypeIdx = dieInfo->tagRefIdx;
            return result;
          }
          break;
        case DW_TAG_enumeration_type:
          if (dieInfo->offset == dieAttr->refOffset) {
//printf("found type: %s enumerationTypeIdx: %d dieInfo->offset: 0x%08x tagRefIdx: %d\n", tagName, typeIdx, dieInfo->offset, dieInfo->tagRefIdx);
            *dwTypeIdx = dieInfo->tagRefIdx;
            return result;
          }
          break;
        case DW_TAG_pointer_type:
          if (dieInfo->offset == dieAttr->refOffset) {
//printf("found type: %s pointerTypeIdx: %d dieInfo->offset: 0x%08x tagRefIdx: %d\n", tagName, typeIdx, dieInfo->offset, dieInfo->tagRefIdx);
            *dwTypeIdx = dieInfo->tagRefIdx;
            return result;
          }
          break;
        case DW_TAG_structure_type:
          if (dieInfo->offset == dieAttr->refOffset) {
//printf("found type: %s structureTypeIdx: %d dieInfo->offset: 0x%08x tagRefIdx: %d\n", tagName, typeIdx, dieInfo->offset, dieInfo->tagRefIdx);
            *dwTypeIdx = dieInfo->tagRefIdx;
            return result;
          }
          break;
        case DW_TAG_subroutine_type:
          if (dieInfo->offset == dieAttr->refOffset) {
//printf("found type: %s subroutineTypeIdx: %d dieInfo->offset: 0x%08x tagRefIdx: %d\n", tagName, typeIdx, dieInfo->offset, dieInfo->tagRefIdx);
            *dwTypeIdx = dieInfo->tagRefIdx;
            return result;
          }
          break;
        case DW_TAG_typedef:
          if (dieInfo->offset == dieAttr->refOffset) {
//printf("found type: %s typedefIdx: %d dieInfo->offset: 0x%08x tagRefIdx: %d\n", tagName, typeIdx, dieInfo->offset, dieInfo->tagRefIdx);
            *dwTypeIdx = dieInfo->tagRefIdx;
            return result;
          }
          break;
        case DW_TAG_union_type: 
          if (dieInfo->offset == dieAttr->refOffset) {
//printf("found type: %s unionTypeIdx: %d dieInfo->offset: 0x%08x tagRefIdx: %d\n", tagName, typeIdx, dieInfo->offset, dieInfo->tagRefIdx);
            *dwTypeIdx = dieInfo->tagRefIdx;
            return result;
          }
          break;
        case DW_TAG_volatile_type:
          if (dieInfo->offset == dieAttr->refOffset) {
//printf("found type: %s volatileTypeIdx: %d dieInfo->offset: 0x%08x tagRefIdx: %d\n", tagName, typeIdx, dieInfo->offset, dieInfo->tagRefIdx);
            *dwTypeIdx = dieInfo->tagRefIdx;
            return result;
          }
          break;
        default:
self->dwarfDbgStringInfo->getDW_TAG_string(self, dieInfo->tag, &tagName);
            if (dieInfo->offset == dieAttr->refOffset) {
printf("found default: %s dieInfo->offset: 0x%08x dieAttr->refOffset: 0x%08x\n", tagName, dieInfo->offset, dieAttr->refOffset);
          }
          break;
        }
      }
    }
  }
printf("ERROR TYPE_REF_NOT_FOUND\n");
  return DWARF_DBG_ERR_TYPE_REF_NOT_FOUND;
}

static int firstTest = 0;
// =================================== handleType =========================== 

static uint8_t handleType(dwarfDbgPtr_t self, dieInfo_t *dieInfo, int *dwAttrTypeInfoIdx) {
  uint8_t result = 0;
  int entryIdx = 0;
  int typeIdx = 0;
  int dwTypeIdx = 0;
  int attrIdx = 0;
  int attrTypeIdx = 0;
  int typeNameIdx = 0;
  int typeRefIdx = 0;
  int tagRefIdx = 0;
  int refOffset = 0;
  dieInfo_t *dieInfo2 = NULL;
  dieAttr_t *dieAttr = NULL;
  dwAttrTypeInfo_t dwAttrTypeInfo;
  int found = 0;
  const char *tagName = NULL;
  const char *attrName = NULL;
  const char *atName = NULL;
int offset = -1;

int newEntry;
Tcl_HashEntry *hPtr = NULL;
dwarfDbgPtr_t xx = NULL;

  result = DWARF_DBG_ERR_OK;
if (!firstTest) {
firstTest++;
hPtr = Tcl_CreateHashEntry(&self->dwarfDbgTypeInfo->dwArrayTypeHashTable, (char*)"key1", &newEntry);
if (hPtr == NULL) {
  return DWARF_DBG_ERR_CANNOT_CREATE_HASH_ENTRY;
}
printf("self: %p\n", self);
}

  dwAttrTypeInfo.tag = dieInfo->tag;
  dwAttrTypeInfo.numDwAttrType = 0;
  dwAttrTypeInfo.maxDwAttrType = 0;
  dwAttrTypeInfo.dwAttrTypes = NULL;
  result = DWARF_DBG_ERR_OK;
  found = 0;
//currTypeFlags = ATTR_arrayTypeFlags;
self->dwarfDbgStringInfo->getDW_TAG_string(self, dieInfo->tag, &tagName);
//printf("tagName: %s numAttr: %d\n", tagName, dieInfo->numAttr);
  for(attrIdx = 0; attrIdx < dieInfo->numAttr; attrIdx++) {
    dieAttr = &dieInfo->dieAttrs[attrIdx];
self->dwarfDbgStringInfo->getDW_AT_string(self, dieAttr->attr, &attrName);
//printf("        attrName: %s value: %d 0x%08x refOffset: 0x%08x\n", attrName, dieAttr->uval, dieAttr->uval, dieAttr->refOffset);
    switch (dieAttr->attr) {
    case DW_AT_artificial:
    case DW_AT_abstract_origin:
    case DW_AT_bit_offset:
    case DW_AT_bit_size:
    case DW_AT_byte_size:
    case DW_AT_call_file:
    case DW_AT_call_line:
    case DW_AT_comp_dir:
    case DW_AT_const_value:
    case DW_AT_data_member_location:
    case DW_AT_decl_file:
    case DW_AT_decl_line:
    case DW_AT_declaration:
    case DW_AT_encoding:
    case DW_AT_entry_pc:
    case DW_AT_external:
    case DW_AT_frame_base:
    case DW_AT_GNU_all_call_sites:
    case DW_AT_GNU_all_tail_call_sites:
    case DW_AT_GNU_call_site_target:
    case DW_AT_GNU_call_site_value:
    case DW_AT_high_pc:
    case DW_AT_inline:
    case DW_AT_language:
    case DW_AT_linkage_name:
    case DW_AT_location:
    case DW_AT_low_pc:
    case DW_AT_producer:
    case DW_AT_prototyped:
    case DW_AT_ranges:
    case DW_AT_sibling:
    case DW_AT_stmt_list:
    case DW_AT_upper_bound:
      result = self->dwarfDbgTypeInfo->addAttrType(self, &dwAttrTypeInfo, dieAttr->attr, dieAttr->uval, dieAttr->refOffset, &attrTypeIdx);
      checkErrOK(result);
      found++;
      break;
    case DW_AT_name:
      if ((dieAttr->attrStrIdx < 0) || (dieAttr->attrStrIdx >= self->dwarfDbgCompileUnitInfo->numAttrStr)) {
        return DWARF_DBG_ERR_BAD_ATTR_STR_IDX;
      }
      atName = self->dwarfDbgCompileUnitInfo->attrStrs[dieAttr->attrStrIdx];
      result = self->dwarfDbgTypeInfo->addTypeStr(self, atName, &typeNameIdx);
      checkErrOK(result);
if (dieInfo->tag == DW_TAG_typedef) {
//printf("typedef: name: %s\n", atName);
}
      dieAttr->uval = typeNameIdx; 
      result = self->dwarfDbgTypeInfo->addAttrType(self, &dwAttrTypeInfo, dieAttr->attr, typeNameIdx, dieAttr->refOffset, &attrTypeIdx);
      checkErrOK(result);
      found++;
      break;
    case DW_AT_type:
//if (dieInfo->tag == DW_TAG_typedef) {
offset = dieAttr->refOffset;
//printf("handleType AT_type: refOffset: 0x%08x\n", dieAttr->refOffset);
//}
      result = getTypeRefIdx(self, dieAttr, &typeRefIdx);
      checkErrOK(result);
//printf("handleType AT_type: got typeRefIdx: %d\n", typeRefIdx);
      dieAttr->uval = typeRefIdx;
      result = self->dwarfDbgTypeInfo->addAttrType(self, &dwAttrTypeInfo, dieAttr->attr, typeRefIdx, dieAttr->refOffset, &attrTypeIdx);
      checkErrOK(result);
//printf("handleType after addAttrType: got attrTypeIdx: %d attridx: %d numAttr: %d\n", attrTypeIdx, attrIdx, dieInfo->numAttr);
      found++;
      break;
    default:
printf("ERROR: DWARF_DBG_ERR_UNEXPECTED_ATTR_IN_TYPE: 0x%04x\n", dieAttr->attr);
      return DWARF_DBG_ERR_UNEXPECTED_ATTR_IN_TYPE;
    }
  }
//printf("found: %d\n", found);
  if (found == dieInfo->numAttr) {
dwAttrTypeInfo.dieInfo = dieInfo;
    result = self->dwarfDbgTypeInfo->addAttrTypeInfo(self, &dwAttrTypeInfo, dieInfo->numAttr - 1, dwAttrTypeInfoIdx);
    checkErrOK(result);
if (dieInfo->tag == DW_TAG_unspecified_parameters) {
printf("unspecified_parameters: dwAttrTypeInfoIdx: %d\n", *dwAttrTypeInfoIdx);
}
    dieInfo->tagRefIdx = *dwAttrTypeInfoIdx;
//printf("after addAttrTypeInfo: tagRefIdx: %d\n", *dwAttrTypeInfoIdx);
  } else {
printf("ERROR type not found found: %d numAttr: %d offset: 0x%08x\n", found, dieInfo->numAttr, dieAttr->refOffset);
    return DWARF_DBG_ERR_TYPE_NOT_FOUND;
  }
  return result;
}

// =================================== addCompileUnitTagTypes =========================== 

static uint8_t addCompileUnitTagTypes(dwarfDbgPtr_t self) {
  uint8_t result = 0;
  int entryIdx = 0;
  int maxEntries = 0;
  int siblingTagInfoIdx = 0;
  int childTagInfoIdx = 0;
  int dieAndChildrenIdx = 0;
  int isSibling = 0;
  dieAndChildrenInfo_t *dieAndChildrenInfo = NULL;
  compileUnit_t *compileUnit = NULL;
  dieInfo_t *dieInfo = NULL;
  int dwAttrTypeInfoIdx = 0;
const char *tagName = NULL;

  result = DWARF_DBG_ERR_OK;
  compileUnit = self->dwarfDbgCompileUnitInfo->currCompileUnit;
  for (dieAndChildrenIdx = 0; dieAndChildrenIdx < compileUnit->numDieAndChildren; dieAndChildrenIdx++) {
    dieAndChildrenInfo = &compileUnit->dieAndChildrenInfos[dieAndChildrenIdx];
    for (isSibling = 0; isSibling < 2; isSibling++) {
      if (isSibling) {
        maxEntries = dieAndChildrenInfo->numSiblings;
      } else {
        maxEntries = dieAndChildrenInfo->numChildren;
      }
//printf("isSibling: %d maxEntries: %d numDieAndChildren: %d maxDieAndChildren: %d\n", isSibling, maxEntries, compileUnit->numDieAndChildren, compileUnit->maxDieAndChildren);
      for(entryIdx = 0; entryIdx < maxEntries; entryIdx++) {
        if (isSibling) {
          dieInfo = &dieAndChildrenInfo->dieSiblings[entryIdx];
        } else {
          dieInfo = &dieAndChildrenInfo->dieChildren[entryIdx];
        }
result = self->dwarfDbgStringInfo->getDW_TAG_string(self, dieInfo->tag, &tagName);
//printf(">@addCompileUnitTagTypes: isIsbling: %d dieChildrenIdx: %d entryIdx: %d tagName: %s\n", isSibling, dieAndChildrenIdx, entryIdx, tagName);
        result = self->dwarfDbgTypeInfo->handleType(self, dieInfo, &dwAttrTypeInfoIdx);
        checkErrOK(result);
printf("after handleType: %s dwAttrTypeInfoIdx: %d entryIdx: %d\n", tagName, dwAttrTypeInfoIdx, entryIdx);
        if (isSibling) {
          result = self->dwarfDbgDieInfo->addDieSiblingTagInfo(self, dieAndChildrenIdx, dieInfo->tag, dwAttrTypeInfoIdx, dieInfo->numAttr, &siblingTagInfoIdx);
        } else {
          result = self->dwarfDbgDieInfo->addDieChildTagInfo(self, dieAndChildrenIdx, dieInfo->tag, dwAttrTypeInfoIdx, dieInfo->numAttr, &childTagInfoIdx);
        }
        checkErrOK(result);
      }
    }
  }
  return result;
}

// =================================== printCompileUnitTagTypes =========================== 

static uint8_t printCompileUnitTagTypes(dwarfDbgPtr_t self) {
  uint8_t result;
  int isSibling = 0;
  int dieAndChildrenIdx = 0;
  int maxEntries = 0;
  int maxEntries2 = 0;
  int entryIdx = 0;
  compileUnit_t *compileUnit;
  dieAndChildrenInfo_t *dieAndChildrenInfo;
  dieTagInfo_t *dieTagInfo;
  dieInfo_t *dieInfo;
  const char *tagName = NULL;
  const char *tagName2 = NULL;

  result = DWARF_DBG_ERR_OK;
  compileUnit = self->dwarfDbgCompileUnitInfo->currCompileUnit;
  printf("CompileUnit: %d\n", self->dwarfDbgCompileUnitInfo->currCompileUnitIdx);
  for (isSibling = 0; isSibling < 2; isSibling++) {
    for(dieAndChildrenIdx = 0; dieAndChildrenIdx < compileUnit->numDieAndChildren; dieAndChildrenIdx++) {
      dieAndChildrenInfo = &compileUnit->dieAndChildrenInfos[dieAndChildrenIdx];
//printf("numSiblings: %d numChildren: %d\n", dieAndChildrenInfo->numSiblings, dieAndChildrenInfo->numChildren);
//printf("numSiblingsTagInfo: %d numChildrenTagInfo: %d\n", dieAndChildrenInfo->numSiblingsTagInfo, dieAndChildrenInfo->numChildrenTagInfo);
      if (isSibling) {
        maxEntries = dieAndChildrenInfo->numSiblingsTagInfo;
        maxEntries2 = dieAndChildrenInfo->numSiblings;
      } else {
        maxEntries = dieAndChildrenInfo->numChildrenTagInfo;
        maxEntries2 = dieAndChildrenInfo->numChildren;
      }
//printf("numSiblings: %d numChildren: %d\n", dieAndChildrenInfo->numSiblings, dieAndChildrenInfo->numChildren);
//printf("numSiblingsTagInfo: %d numChildrenTagInfo: %d\n", dieAndChildrenInfo->numSiblingsTagInfo, dieAndChildrenInfo->numChildrenTagInfo);
//printf("isSibling: %d maxEntries: %d numDieAndChildren: %d maxDieAndChildren: %d\n", isSibling, maxEntries, compileUnit->numDieAndChildren, compileUnit->maxDieAndChildren);
      for(entryIdx = 0; entryIdx < maxEntries; entryIdx++) {
        if (isSibling) {
          dieTagInfo = &dieAndChildrenInfo->dieSiblingsTagInfos[entryIdx];
          dieInfo = &dieAndChildrenInfo->dieSiblings[entryIdx];
        } else {
          dieTagInfo = &dieAndChildrenInfo->dieChildrenTagInfos[entryIdx];
          dieInfo = &dieAndChildrenInfo->dieChildren[entryIdx];
        }
        result = self->dwarfDbgStringInfo->getDW_TAG_string(self, dieTagInfo->tag, &tagName);
//printf("getDW_TAG_string2: result: %d tag: 0x%04x\n", result, dieTagInfo->tag);
        checkErrOK(result);
        printf("  Tag:  %s dwAttrTypeInfoIdx: %d\n", tagName, dieTagInfo->dwAttrTypeInfoIdx);
        if (entryIdx < maxEntries2) {
          result = self->dwarfDbgStringInfo->getDW_TAG_string(self, dieInfo->tag, &tagName2);
//printf("getDW_TAG_string3: result: %d tag: 0x%04x isSibling: %d entryIdx: %d numSiblings: %d numChildren: %d\n", result, dieInfo->tag, isSibling, entryIdx, dieAndChildrenInfo->numSiblings, dieAndChildrenInfo->numChildren);
          checkErrOK(result);
          printf("  OTag: %s tagRefIdx: %d\n", tagName2, dieInfo->tagRefIdx);
        } else {
          printf("ERROR to few siblings/children old\n");
        }
      }
    }
  }
  return result;
}

// =================================== dwarfDbgTypeInfoInit =========================== 

int dwarfDbgTypeInfoInit (dwarfDbgPtr_t self) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;
  self->dwarfDbgTypeInfo->genAttrTypeInfoKey = &genAttrTypeInfoKey;
  self->dwarfDbgTypeInfo->getAttrTypeInfos = &getAttrTypeInfos;
  self->dwarfDbgTypeInfo->addTypeStr = &addTypeStr;
  self->dwarfDbgTypeInfo->addAttrType = &addAttrType;
  self->dwarfDbgTypeInfo->checkDieTypeRefIdx = &checkDieTypeRefIdx;

  self->dwarfDbgTypeInfo->printAttrTypeInfo = &printAttrTypeInfo;
  self->dwarfDbgTypeInfo->addAttrTypeInfo = &addAttrTypeInfo;

  self->dwarfDbgTypeInfo->handleType = &handleType;

  self->dwarfDbgTypeInfo->addCompileUnitTagTypes = &addCompileUnitTagTypes;
  self->dwarfDbgTypeInfo->printCompileUnitTagTypes = &printCompileUnitTagTypes;

  self->dwarfDbgTypeInfo->typeLevel = 0;

  memset(&self->dwarfDbgTypeInfo->dwArrayTypeInfos, 0, sizeof(dwAttrTypeInfo_t));
  memset(&self->dwarfDbgTypeInfo->dwBaseTypeInfos, 0, sizeof(dwAttrTypeInfo_t));
  memset(&self->dwarfDbgTypeInfo->dwCompileUnitInfos, 0, sizeof(dwAttrTypeInfo_t));
  memset(&self->dwarfDbgTypeInfo->dwConstTypeInfos, 0, sizeof(dwAttrTypeInfo_t));
  memset(&self->dwarfDbgTypeInfo->dwEnumerationTypeInfos, 0, sizeof(dwAttrTypeInfo_t));
  memset(&self->dwarfDbgTypeInfo->dwEnumeratorInfos, 0, sizeof(dwAttrTypeInfo_t));
  memset(&self->dwarfDbgTypeInfo->dwFormalParameterInfos, 0, sizeof(dwAttrTypeInfo_t));
  memset(&self->dwarfDbgTypeInfo->dwGNUCallSiteInfos, 0, sizeof(dwAttrTypeInfo_t));
  memset(&self->dwarfDbgTypeInfo->dwGNUCallSiteParameterInfos, 0, sizeof(dwAttrTypeInfo_t));
  memset(&self->dwarfDbgTypeInfo->dwInlinedSubroutineInfos, 0, sizeof(dwAttrTypeInfo_t));
  memset(&self->dwarfDbgTypeInfo->dwLabelInfos, 0, sizeof(dwAttrTypeInfo_t));
  memset(&self->dwarfDbgTypeInfo->dwLexicalBlockInfos, 0, sizeof(dwAttrTypeInfo_t));
  memset(&self->dwarfDbgTypeInfo->dwMemberInfos, 0, sizeof(dwAttrTypeInfo_t));
  memset(&self->dwarfDbgTypeInfo->dwPointerTypeInfos, 0, sizeof(dwAttrTypeInfo_t));
  memset(&self->dwarfDbgTypeInfo->dwStructureTypeInfos, 0, sizeof(dwAttrTypeInfo_t));
  memset(&self->dwarfDbgTypeInfo->dwSubprogramTypeInfos, 0, sizeof(dwAttrTypeInfo_t));
  memset(&self->dwarfDbgTypeInfo->dwSubrangeInfos, 0, sizeof(dwAttrTypeInfo_t));
  memset(&self->dwarfDbgTypeInfo->dwSubroutineTypeInfos, 0, sizeof(dwAttrTypeInfo_t));
  memset(&self->dwarfDbgTypeInfo->dwTypedefInfos, 0, sizeof(dwAttrTypeInfo_t));
  memset(&self->dwarfDbgTypeInfo->dwUnionTypeInfos, 0, sizeof(dwAttrTypeInfo_t));
  memset(&self->dwarfDbgTypeInfo->dwUnspecifiedParametersInfos, 0, sizeof(dwAttrTypeInfo_t));
  memset(&self->dwarfDbgTypeInfo->dwVariableInfos, 0, sizeof(dwAttrTypeInfo_t));
  memset(&self->dwarfDbgTypeInfo->dwVolatileTypeInfos, 0, sizeof(dwAttrTypeInfo_t));

  Tcl_InitHashTable(&self->dwarfDbgTypeInfo->dwArrayTypeHashTable, TCL_STRING_KEYS);
  Tcl_InitHashTable(&self->dwarfDbgTypeInfo->dwBaseTypeHashTable, TCL_STRING_KEYS);
  Tcl_InitHashTable(&self->dwarfDbgTypeInfo->dwCompileUnitHashTable, TCL_STRING_KEYS);
  Tcl_InitHashTable(&self->dwarfDbgTypeInfo->dwConstTypeHashTable, TCL_STRING_KEYS);
  Tcl_InitHashTable(&self->dwarfDbgTypeInfo->dwEnumerationTypeHashTable, TCL_STRING_KEYS);
  Tcl_InitHashTable(&self->dwarfDbgTypeInfo->dwEnumeratorHashTable, TCL_STRING_KEYS);
  Tcl_InitHashTable(&self->dwarfDbgTypeInfo->dwFormalParameterHashTable, TCL_STRING_KEYS);
  Tcl_InitHashTable(&self->dwarfDbgTypeInfo->dwGNUCallSiteHashTable, TCL_STRING_KEYS);
  Tcl_InitHashTable(&self->dwarfDbgTypeInfo->dwGNUCallSiteParameterHashTable, TCL_STRING_KEYS);
  Tcl_InitHashTable(&self->dwarfDbgTypeInfo->dwInlinedSubroutineHashTable, TCL_STRING_KEYS);
  Tcl_InitHashTable(&self->dwarfDbgTypeInfo->dwLabelHashTable, TCL_STRING_KEYS);
  Tcl_InitHashTable(&self->dwarfDbgTypeInfo->dwLexicalBlockHashTable, TCL_STRING_KEYS);
  Tcl_InitHashTable(&self->dwarfDbgTypeInfo->dwMemberHashTable, TCL_STRING_KEYS);
  Tcl_InitHashTable(&self->dwarfDbgTypeInfo->dwPointerTypeHashTable, TCL_STRING_KEYS);
  Tcl_InitHashTable(&self->dwarfDbgTypeInfo->dwStructureTypeHashTable, TCL_STRING_KEYS);
  Tcl_InitHashTable(&self->dwarfDbgTypeInfo->dwSubprogramTypeHashTable, TCL_STRING_KEYS);
  Tcl_InitHashTable(&self->dwarfDbgTypeInfo->dwSubrangeHashTable, TCL_STRING_KEYS);
  Tcl_InitHashTable(&self->dwarfDbgTypeInfo->dwSubroutineTypeHashTable, TCL_STRING_KEYS);
  Tcl_InitHashTable(&self->dwarfDbgTypeInfo->dwTypedefHashTable, TCL_STRING_KEYS);
  Tcl_InitHashTable(&self->dwarfDbgTypeInfo->dwUnionTypeHashTable, TCL_STRING_KEYS);
  Tcl_InitHashTable(&self->dwarfDbgTypeInfo->dwUnspecifiedParametersHashTable, TCL_STRING_KEYS);
  Tcl_InitHashTable(&self->dwarfDbgTypeInfo->dwVariableHashTable, TCL_STRING_KEYS);
  Tcl_InitHashTable(&self->dwarfDbgTypeInfo->dwVolatileTypeHashTable, TCL_STRING_KEYS);

//  typeFd = fopen("types.txt", "w");
  typeFd = stdout;
  return result;
}
