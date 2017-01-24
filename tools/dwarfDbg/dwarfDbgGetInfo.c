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
 * File:   dwarfDbgGetInfo.c
 * Author: Arnulf P. Wiedemann <arnulf@wiedemann-pri.de>
 *
 * Created on January 24, 2017
 */

#include <tcl.h>
#include "dwarfDbgInt.h"

// =================================== addCompileUnit =========================== 

static uint8_t addCompileUnit(dwarfDbgPtr_t self, size_t *compileUnitIdx) {
  uint8_t result;
  compileUnit_t *compileUnit;

  result = DWARF_DBG_ERR_OK;
  if (self->dwarfDbgGetInfo->maxCompileUnit <= self->dwarfDbgGetInfo->numCompileUnit) {
    self->dwarfDbgGetInfo->maxCompileUnit += 50;
    if (self->dwarfDbgGetInfo->compileUnits == NULL) {
      self->dwarfDbgGetInfo->compileUnits = (compileUnit_t *)ckalloc(sizeof(compileUnit_t) * self->dwarfDbgGetInfo->maxCompileUnit);
      if (self->dwarfDbgGetInfo->compileUnits == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      self->dwarfDbgGetInfo->compileUnits = (compileUnit_t *)ckrealloc((char *)self->dwarfDbgGetInfo->compileUnits, sizeof(compileUnit_t) * self->dwarfDbgGetInfo->maxCompileUnit);
      if (self->dwarfDbgGetInfo->compileUnits == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
printf("compileUnitIdx: %d\n", self->dwarfDbgGetInfo->numCompileUnit);
  compileUnit = &self->dwarfDbgGetInfo->compileUnits[self->dwarfDbgGetInfo->numCompileUnit];
  memset(compileUnit, 0, sizeof(compileUnit_t));
  *compileUnitIdx = self->dwarfDbgGetInfo->numCompileUnit;
  self->dwarfDbgGetInfo->numCompileUnit++;
  return result;
}

static Dwarf_Off  DIEOffset = 0;      /* DIE offset in compile unit */
static Dwarf_Off  DIEOverallOffset = 0;  /* DIE offset in .debug_info */
static dwarfDbgEsb_t esbShortCuName;
static dwarfDbgEsb_t esbLongCuName;

// =================================== get_producer_name =========================== 

/*  Returns the producer of the CU
  Caller must ensure producernameout is
  a valid, constructed, empty dwarfDbgEsb_t instance before calling.
  Never returns DW_DLV_ERROR.  */
static int getProducerName(dwarfDbgPtr_t self, Dwarf_Debug dbg, Dwarf_Die cuDie, Dwarf_Off dieprintCuOffset, dwarfDbgEsb_t *producernameout) {
  Dwarf_Attribute producer_attr = 0;
  Dwarf_Error pnerr = 0;

  int ares = dwarf_attr(cuDie, DW_AT_producer, &producer_attr, &pnerr);
  if (ares == DW_DLV_ERROR) {
    printf("hassattr on DW_AT_producer ares: %d pnerr: %d", ares, pnerr);
  }
  if (ares == DW_DLV_NO_ENTRY) {
    /*  We add extra quotes so it looks more like
      the names for real producers that get_attr_value
      produces. */
    self->dwarfDbgEsb->esbAppend(self, producernameout,"\"<CU-missing-DW_AT_producer>\"");
  } else {
    /*  DW_DLV_OK */
    /*  The string return is valid until the next call to this
      function; so if the caller needs to keep the returned
      string, the string must be copied (makename()). */
    get_attr_value(dbg, DW_TAG_compile_unit,
      cuDie, dieprintCuOffset,
      producer_attr, NULL, 0, producernameout,
      0 /*show_form_used*/,0 /* verbose */);
  }
  /*  If ares is error or missing case,
    producer_attr will be left
    NULL by the call,
    which is safe when calling dealloc(). */
  dwarf_dealloc(dbg, producer_attr, DW_DLA_ATTR);
  return ares;
}

// =================================== getCuName =========================== 

/* Returns the cu of the CU. In case of error, give up, do not return. */
static int getCuName(dwarfDbgPtr_t self, Dwarf_Debug dbg, Dwarf_Die cuDie, Dwarf_Off dieprintCuOffset, char * *short_name, char * *long_name) {
  Dwarf_Attribute name_attr = 0;
  Dwarf_Error lerr = 0;
  int ares;

  ares = dwarf_attr(cuDie, DW_AT_name, &name_attr, &lerr);
  if (ares == DW_DLV_ERROR) {
    printf("hassattr on DW_AT_name ares: %d lerr: %d", ares, lerr);
  } else {
    if (ares == DW_DLV_NO_ENTRY) {
      *short_name = "<unknown name>";
      *long_name = "<unknown name>";
    } else {
      /* DW_DLV_OK */
      /*  The string return is valid until the next call to this
        function; so if the caller needs to keep the returned
        string, the string must be copied (makename()). */
      char *filename = 0;

      self->dwarfDbgEsb->esbEmptyString(self, &esbLongCuName);
      get_attr_value(dbg, DW_TAG_compile_unit,
        cuDie, dieprintCuOffset,
        name_attr, NULL, 0, &esbLongCuName,
        0 /*show_form_used*/,0 /* verbose */);
      *long_name = self->dwarfDbgEsb->esbGetString(self, &esbLongCuName);
      /* Generate the short name (filename) */
      filename = strrchr(*long_name,'/');
      if (!filename) {
        filename = strrchr(*long_name,'\\');
      }
      if (filename) {
        ++filename;
      } else {
        filename = *long_name;
      }
      self->dwarfDbgEsb->esbEmptyString(self, &esbShortCuName);
      self->dwarfDbgEsb->esbAppend(self, &esbShortCuName,filename);
      *short_name = self->dwarfDbgEsb->esbGetString(self, &esbShortCuName);
    }
  }
  dwarf_dealloc(dbg, name_attr, DW_DLA_ATTR);
  return ares;
}


// =================================== handleOneDieSection =========================== 

static uint8_t handleOneDieSection(dwarfDbgPtr_t self) {
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
  size_t compileUnitIdx = 0;
  char * cuShortName = NULL;
  char * cuLongName = NULL;


printf("handleOneDieSection dbg: %p\n", self->elfInfo.dbg);
  isInfo = TRUE;
  result = DWARF_DBG_ERR_OK;
  res = dwarf_get_die_section_name(self->elfInfo.dbg, isInfo, &sectionName, &err);
  if (res != DW_DLV_OK || !sectionName || !strlen(sectionName)) {
    sectionName = ".debug_info";
  }
printf("sectionName: %s\n", sectionName);
  /* Loop over compile units until it fails.  */
  for (;;++loopCount) {
    int sres = DW_DLV_OK;
    Dwarf_Die compileUnitDie = NULL;
    compileUnit_t *compileUnit;
//        struct Dwarf_Debug_Fission_Per_CU_s fission_data;
//        int fission_data_result = 0;

    result = self->dwarfDbgGetInfo->addCompileUnit(self, &compileUnitIdx);
printf("addCompileUnit: result: %d compileUnitIdx: %d\n", result, compileUnitIdx);
    checkErrOK(result);
//        memset(&fission_data,0,sizeof(fission_data));
    compileUnit = &self->dwarfDbgGetInfo->compileUnits[compileUnitIdx];
printf("call dwarf_next_cu_header_d\n");
    nres = dwarf_next_cu_header_d(self->elfInfo.dbg, 1,
      &compileUnit->compileUnitHeaderLength,
      &compileUnit->versionStamp,
      &compileUnit->abbrevOffset,
      &compileUnit->addressSize,
      &compileUnit->lengthSize,
      &compileUnit->extensionSize,
      &compileUnit->signature,
      &compileUnit->typeOffset,
      &compileUnit->nextCompileUnitOffset,
      &compileUnit->compileUnitType,
      &err);
printf("after dwarf_next_cu_header_d nres: %d loop_count: %d\n", nres, loopCount);
    if (nres == DW_DLV_NO_ENTRY) {
      // we have processed all entries
      break;
    }
    if (nres != DW_DLV_OK) {
      return DWARF_DBG_ERR_CANNOT_GET_NEXT_COMPILE_UNIT;
    }
    /*  get basic information about the current compile unit: producer, name */
    sres = dwarf_siblingof_b(self->elfInfo.dbg, NULL,/* is_info */1, &compileUnitDie, &err);
    if (sres != DW_DLV_OK) {
printf("siblingof cu header sres: %d err: %p", sres, err);
      return DWARF_DBG_ERR_CANNOT_GET_SIBLING_OF_COMPILE_UNIT;
    }
printf("compileUnitDie: %p\n", compileUnitDie);
    dwarf_die_offsets(compileUnitDie, &DIEOverallOffset, &DIEOffset, &err);
    {
    /* Get producer name for this compile unit */
      dwarfDbgEsb_t producername;
      
      self->dwarfDbgEsb->esbConstructor(self, &producername);
      self->dwarfDbgElfInfo->getProducerName(self, self->elfInfo.dbg, compileUnitDie, DIEOverallOffset, &producername);
printf("producername: %s\n", self->dwarfDbgEsb->esbGetString(self, &producername));
      self->dwarfDbgEsb->esbDestructor(self, &producername);
    }
    getCuName(self, self->elfInfo.dbg, compileUnitDie, DIEOverallOffset, &cuShortName, &cuLongName);
printf("cuShortName: %s\n", cuShortName);
    sres = dwarf_siblingof_b(self->elfInfo.dbg, NULL, /* is_info */ 1, &compileUnitDie, &err);
printf("compileUnitDie2: %p sres: %d\n", compileUnitDie, sres);

  }
  return result;
}

// =================================== dwarfDbgGetInfos =========================== 

int dwarfDbgGetInfos(dwarfDbgPtr_t self) {
  uint8_t result;

printf("dwrafDbgGetInfos\n");
  result = DWARF_DBG_ERR_OK;
  result = self->dwarfDbgGetInfo->handleOneDieSection(self);
  checkErrOK(result);

  return result;
}

// =================================== dwarfDbgGetInfoInit =========================== 

int dwarfDbgGetInfoInit (dwarfDbgPtr_t self) {
printf("dwarfDbgGetInfoInit self: %p\n", self);

  self->dwarfDbgGetInfo->maxCompileUnit = 0;
  self->dwarfDbgGetInfo->numCompileUnit = 0;
  self->dwarfDbgGetInfo->compileUnits = NULL;

  self->dwarfDbgGetInfo->addCompileUnit = &addCompileUnit;
  self->dwarfDbgGetInfo->handleOneDieSection = &handleOneDieSection;
  return DWARF_DBG_ERR_OK;
}
