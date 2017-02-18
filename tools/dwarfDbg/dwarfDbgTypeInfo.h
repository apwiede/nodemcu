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
 * File:   dwarfDbgTypeInfo.h
 * Author: Arnulf P. Wiedemann <arnulf@wiedemann-pri.de>
 *
 * Created on February 18, 2017
 */

#ifndef DWARF_DBG_TYPE_INFO_H
#define	DWARF_DBG_TYPE_INFO_H

typedef struct dwBaseType {
  int byteSize;
  int encoding;
  int typeStrIdx;  // name
} dwBaseType_t;

typedef struct dwTypeDef {
  int byteSize;
  int encoding;
  int typeStrIdx;  // name
} dwTypeDef_t;

typedef struct dwConstType {
  int type;
} dwConstType_t;

typedef struct dwMember {
  int typeStrIdx;  // name
  int pathInfoIdx; // pathName and fileName
  int lineNo;
  int type;
  int byteSize;
  int bitSize;
  int bitOffset;
  int location;
} dwMember_t;

typedef uint8_t (* addTypeStr_t)(dwarfDbgPtr_t self, const char *str, int *typeStrIdx);
typedef uint8_t (* addBaseType_t)(dwarfDbgPtr_t self, int byteSize, int encoding, char *name, int *baseTypeIdx);

typedef struct dwarfDbgTypeInfo {
  int maxDwBaseType;
  int numDwBaseType;
  dwBaseType_t *dwBaseTypes;

  int maxDwTypeDef;
  int numDwTypeDef;
  dwTypeDef_t *dwTypeDefs;

  int maxDwConstType;
  int numDwConstType;
  dwConstType_t *dwConstTypes;

  int maxDwMember;
  int numDwMember;
  dwMember_t *dwMembers;

  int maxTypeStr;
  int numTypeStr;
  char **typeStrs;

  addTypeStr_t addTypeStr;
  addBaseType_t addBaseType;
} dwarfDbgTypeInfo_t;

#endif  /* DWARF_DBG_TYPE_INFO_H */
