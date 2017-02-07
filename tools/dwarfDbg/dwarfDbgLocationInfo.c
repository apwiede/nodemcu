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
 * File:   dwarfDbgLocationInfo.c
 * Author: Arnulf P. Wiedemann <arnulf@wiedemann-pri.de>
 *
 * Created on February 07, 2017
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
 * */

static uint8_t getAddressSizeAndMax(dwarfDbgPtr_t self, Dwarf_Half *size, Dwarf_Addr *max, Dwarf_Error *err) {
  int dres = 0;
  Dwarf_Half lsize = 4;
  /* Get address size and largest representable address */
  dres = dwarf_get_address_size(self->elfInfo.dbg, &lsize, err);
  if (dres != DW_DLV_OK) {
    printf("get_address_size() dres: %d err: %p", dres, *err);
    return DWARF_DBG_ERR_CANNOT_GET_ADDR_SIZE;
  }
  if (max) {
    *max = (lsize == 8 ) ? 0xffffffffffffffffULL : 0xffffffff;
  }
  if (size) {
    *size = lsize;
  }
  return DWARF_DBG_ERR_OK;
}

// =================================== addLocation =========================== 

static uint8_t addLocationDirName(dwarfDbgPtr_t self, char *dirName) {
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

// =================================== getLocationList =========================== 

static uint8_t getLocationList(dwarfDbgPtr_t self, Dwarf_Attribute attr) {
  uint8_t result;
  Dwarf_Error err = NULL;
  Dwarf_Loc_Head_c loclistHead = 0;
  Dwarf_Unsigned noOfElements = 0;
  Dwarf_Addr lopc = 0;
  Dwarf_Addr hipc = 0;
  Dwarf_Small lleValue = 0; /* DWARF5 */
  Dwarf_Small loclistSource = 0;
  Dwarf_Unsigned locentryCount = 0;
  Dwarf_Locdesc_c locentry = 0;
  Dwarf_Unsigned locdescOffset = 0;
  Dwarf_Unsigned sectionOffset = 0;
  int lres = 0;
  int llent = 0;
  int i;

  result = DWARF_DBG_ERR_OK;
  lres = dwarf_get_loclist_c(attr, &loclistHead, &noOfElements, &err);
  if (lres != DW_DLV_OK) {
    return DWARF_DBG_ERR_CANNOT_GET_LOC_LIST_C;
  }
printf("numLocations: %d\n", noOfElements);
  for (llent = 0; llent < noOfElements; ++llent) {
    char small_buf[150];
    Dwarf_Unsigned locdescOffset = 0;
    Dwarf_Locdesc_c locentry = 0;
    Dwarf_Addr lopcfinal = 0;
    Dwarf_Addr hipcfinal = 0;
    Dwarf_Small op = 0;
    Dwarf_Unsigned opd1 = 0;
    Dwarf_Unsigned opd2 = 0;
    Dwarf_Unsigned opd3 = 0;
    Dwarf_Unsigned offsetforbranch = 0;
    const char *opName = NULL;
    int res = 0;

    lres = dwarf_get_locdesc_entry_c(loclistHead, llent, &lleValue, &lopc, &hipc, &locentryCount,
           &locentry, &loclistSource, &sectionOffset, &locdescOffset, &err);
    if (lres != DW_DLV_OK) {
      return DWARF_DBG_ERR_CANNOT_GET_LOC_DESC_ENTRY_C;
    }
printf("value: 0x%08x lopc: 0x%08x hipc: 0x%08x\n", lleValue, lopc, hipc);
    for (i = 0; i < locentryCount; i++) {
      res = dwarf_get_location_op_value_c(locentry, i, &op, &opd1, &opd2, &opd3, &offsetforbranch, &err);
      if (res != DW_DLV_OK) {
        return DWARF_DBG_ERR_CANNOT_GET_LOCATION_OP_VALUE_C;
      }
      result = self->dwarfDbgStringInfo->getDW_OP_string(self, op, &opName);
      checkErrOK(result);
printf("op: 0x%02x %s opd1: 0x%02x opd2: 0x%02x opd3: 0x%02x offsetforbranch: %d\n", op, opName, opd1, opd2, opd3, offsetforbranch);
    }
  }
  return result;
}

// =================================== dwarfDbgLocationInfoInit =========================== 

int dwarfDbgLocationInfoInit (dwarfDbgPtr_t self) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;
//  self->dwarfDbgLocationInfo->dirNamesInfo.maxDirName = 0;
//  self->dwarfDbgLocationInfo->dirNamesInfo.numDirName = 0;
//  self->dwarfDbgLocationInfo->dirNamesInfo.dirNames = NULL;

    self->dwarfDbgLocationInfo->getAddressSizeAndMax = &getAddressSizeAndMax;
    self->dwarfDbgLocationInfo->getLocationList = &getLocationList;
  return result;
}


