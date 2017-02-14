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
 * File:   dwarfDbgLocationInfo.h
 * Author: Arnulf P. Wiedemann <arnulf@wiedemann-pri.de>
 *
 * Created on February 07, 2017
 */

#ifndef DWARF_DBG_LOCATION_INFO_H
#define	DWARF_DBG_LOCATION_INFO_H

typedef struct attrInInfo attrInInfo_t;

typedef int (* opHasNoOperands_t)(int op);
typedef uint8_t (* addLocation_t)(dwarfDbgPtr_t self, char *dirName);
typedef uint8_t (* handleLocationExprloc_t)(dwarfDbgPtr_t self, attrInInfo_t *attrInInfo);
typedef uint8_t (* getLocationList_t)(dwarfDbgPtr_t self, size_t dieAndChildrenIdx, size_t dieInfoIdx, Dwarf_Bool isSibling, int attrIdx, Dwarf_Attribute attr);

typedef struct locationOp {
  Dwarf_Small op;
  Dwarf_Unsigned opd1;
  Dwarf_Unsigned opd2;
  Dwarf_Unsigned opd3;
  Dwarf_Unsigned offsetForBranch;
  char contents[10];
} locationOp_t;

typedef struct locationInfo {
  Dwarf_Addr lopc;
  Dwarf_Addr hipc;
  int numLocEntry;
  int maxLocEntry;
  locationOp_t *locationOps;
} locationInfo_t;

typedef struct dwarfDbgLocationInfo {
  Dwarf_Half addrSize;
  Dwarf_Addr maxAddr;

  opHasNoOperands_t opHasNoOperands;
  addLocation_t addLocation;
  handleLocationExprloc_t handleLocationExprloc;
  getLocationList_t getLocationList;
} dwarfDbgLocationInfo_t;


#endif  /* DWARF_DBG_LOCATION_INFO_H */
