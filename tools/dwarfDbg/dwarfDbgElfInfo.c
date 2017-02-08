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
 * File:   dwarfDbgElfInfo.c
 * Author: Arnulf P. Wiedemann <arnulf@wiedemann-pri.de>
 *
 * Created on January 22, 2017
 */

#include <tcl.h>
#include "dwarfDbgInt.h"

// =================================== getCompileUnitLineInfos =========================== 

static uint8_t getCompileUnitLineInfos(Dwarf_Debug dbg, Dwarf_Die die, Dwarf_Off dieprint_cu_goffset) {
    Dwarf_Unsigned lineversion = 0;
    Dwarf_Signed linecount = 0;
    Dwarf_Line *linebuf = NULL;
    Dwarf_Signed linecount_actuals = 0;
    Dwarf_Line *linebuf_actuals = NULL;
    Dwarf_Small  table_count = 0;
    int lres = 0;
    int line_errs = 0;
    Dwarf_Line_Context line_context = 0;
    const char *sec_name = 0;
    Dwarf_Error err = 0;
    Dwarf_Off cudie_local_offset = 0;
//    Dwarf_Off dieprint_cu_goffset = 0;
    int lires = 0;
    int atres = 0;
    int ares = 0;
    Dwarf_Unsigned lineno = 0;

printf("getCompileUnitLineInfos\n");
        lres = dwarf_srclines_b(die,&lineversion,
            &table_count,&line_context,
            &err);
if (table_count > 0) {
printf(">>table_count: %d\n", table_count);
}
        if(lres == DW_DLV_OK) {
printf("dwarf_srclines_two_level_from_linecontext\n");
            lres = dwarf_srclines_two_level_from_linecontext(line_context,
                &linebuf, &linecount,
                &linebuf_actuals, &linecount_actuals,
                &err);
        }
if (linecount > 0) {
printf(">>>linecount: %d\n", linecount);

    Dwarf_Signed i = 0;
    Dwarf_Addr pc = 0;
    Dwarf_Error lt_err = 0;

    for (i = 0; i < linecount; i++) {
        Dwarf_Line line = linebuf[i];
        char* filename = 0;
        int nsres = 0;

        pc = 0;
        ares = dwarf_lineaddr(line, &pc, &lt_err);
        lires = dwarf_lineno(line, &lineno, &lt_err);
printf("dwarf_lineaddr: line: 0x%08x pc: 0x%08x lineno: %d\n", line, pc, lineno);
    }
}
   return 1;
}

// =================================== dwarfDbgOpenElf =========================== 

int dwarfDbgOpenElf (dwarfDbgPtr_t self, char *fileName) {
  int dres = 0;
  Dwarf_Error onef_err = 0;

//printf("dwarfDbgOpenElf\n");
//fflush(stdout);
  self->elfInfo.fd = 0;
  self->elfInfo.cmd = 0;
  self->elfInfo.elf = 0;
  self->elfInfo.dbg = 0;
  (void) elf_version(EV_NONE);
  if (elf_version(EV_CURRENT) == EV_NONE) {
    self->errorStr = "dwarfDbg: libelf.a out of date.";
    return TCL_ERROR;
  }
  self->elfInfo.fd = open(fileName, O_RDONLY);
  self->elfInfo.cmd = ELF_C_READ;
  self->elfInfo.elf = elf_begin(self->elfInfo.fd, self->elfInfo.cmd, (Elf *) 0);
printf("fileName: %s fd: %d arf: %p\n", fileName, self->elfInfo.fd, self->elfInfo.elf);
  // we only handle one elf part in this version!!
  if (self->elfInfo.elf == NULL) {
    self->errorStr = "problem in elf_begin";
    return TCL_ERROR;
  }
  dres = dwarf_elf_init(self->elfInfo.elf, DW_DLC_READ, NULL, NULL, &self->elfInfo.dbg, &onef_err);
  if (dres == DW_DLV_NO_ENTRY) {
    sprintf(self->errorBuf, "No DWARF information present in %s\n", fileName);
    self->errorStr = self->errorBuf;
    return TCL_ERROR;
  }   
  return TCL_OK;
}

// =================================== dwarfDbgGetFiles =========================== 

int dwarfDbgGetFiles (dwarfDbgPtr_t self) {
  Dwarf_Error pod_err;
  uint8_t result;

  return TCL_OK;
}

// =================================== dwarfDbgCloseElf =========================== 

int dwarfDbgCloseElf (dwarfDbgPtr_t self) {
printf("dwarfDbgCloseElf\n");
  elf_end(self->elfInfo.elf);
printf("fd: %d\n", self->elfInfo.fd);
  close(self->elfInfo.fd);
  return TCL_OK;
}

// =================================== dwarfDbgElfInfoInit =========================== 

int dwarfDbgElfInfoInit (dwarfDbgPtr_t self) {
  return DWARF_DBG_ERR_OK;
}

