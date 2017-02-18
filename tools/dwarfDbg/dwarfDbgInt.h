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
 * File:   dwarfDbgInt.h
 * Author: Arnulf P. Wiedemann <arnulf@wiedemann-pri.de>
 *
 * Created on January 22, 2017
 */

#ifndef DWARFDBG_INT_H
#define DWARFDBG_INT_H 1

#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <fcntl.h>
#include <libelf.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <tcl.h>

#include "libdwarf.h"
#include "dwarf.h"
#include "dwarfDbg.h"
#include "dwarfDbgErrorCodes.h"
#include "dwarfDbgDebugInfo.h"
#include "dwarfDbgEsb.h"
#include "dwarfDbg/dwarfDbgDecls.h"
#include "dwarfDbgUtil.h"
#include "dwarfDbgDict.h"
#include "dwarfDbgTypeInfo.h"
#include "dwarfDbgFrameInfo.h"
#include "dwarfDbgLineInfo.h"
#include "dwarfDbgRangeInfo.h"
#include "dwarfDbgSubProgramInfo.h"
#include "dwarfDbgAttributeInfo.h"
#include "dwarfDbgLocationInfo.h"
#include "dwarfDbgAddrInfo.h"
#include "dwarfDbgDieInfo.h"
#include "dwarfDbgStringInfo.h"
#include "dwarfDbgFileInfo.h"
#include "dwarfDbgCompileUnitInfo.h"
#include "dwarfDbgElfInfo.h"
#include "dwarfDbgGetDbgInfo.h"

typedef struct elfInfo {
  int fd;
  Elf_Cmd cmd;
  Elf *elf;
  Dwarf_Debug dbg;
} elfInfo_t;

/*
 * Actual type of the dwarfDbg data structure. Used only inside of the
 * package.
 */

typedef struct _dwarfDbg {
  DWARFDBG_CELL_FREE freeCell; 
  void *clientData;
  char *errorStr; /* cause of error */
  char errorBuf[256]; /* cause of error */
  Tcl_Interp *interp;

  elfInfo_t elfInfo;
  compileUnit_t compileUnit;

  dwarfDbgEsb_t *dwarfDbgEsb;
  dwarfDbgDebugInfo_t *dwarfDbgDebugInfo;
  dwarfDbgUtil_t *dwarfDbgUtil;
  dwarfDbgDict_t *dwarfDbgDict;
  dwarfDbgTypeInfo_t *dwarfDbgTypeInfo;
  dwarfDbgFileInfo_t *dwarfDbgFileInfo;
  dwarfDbgLineInfo_t *dwarfDbgLineInfo;
  dwarfDbgRangeInfo_t *dwarfDbgRangeInfo;
  dwarfDbgSubProgramInfo_t *dwarfDbgSubProgramInfo;
  dwarfDbgFrameInfo_t *dwarfDbgFrameInfo;
  dwarfDbgAttributeInfo_t *dwarfDbgAttributeInfo;
  dwarfDbgLocationInfo_t *dwarfDbgLocationInfo;
  dwarfDbgAddrInfo_t *dwarfDbgAddrInfo;
  dwarfDbgStringInfo_t *dwarfDbgStringInfo;
  dwarfDbgDieInfo_t *dwarfDbgDieInfo;
  dwarfDbgCompileUnitInfo_t *dwarfDbgCompileUnitInfo;
  dwarfDbgElfInfo_t *dwarfDbgElfInfo;
  dwarfDbgGetDbgInfo_t *dwarfDbgGetDbgInfo;

} _dwarfDbg_t;

/*
 * Allocation macros for common situations.
 */

#define ALLOC(type)    (type *) ckalloc (sizeof (type))
#define NALLOC(n,type) (type *) ckalloc ((n) * sizeof (type))

/*
 * Assertions in general, and asserting the proper range of an array
 * index.
 */

#undef  DWARFDBG_DEBUG
#define DWARFDBG_DEBUG 1

#ifdef DWARFDBG_DEBUG
#define XSTR(x) #x
#define STR(x) XSTR(x)
#define RANGEOK(i,n) ((0 <= (i)) && (i < (n)))
#define ASSERT(x,msg) if (!(x)) { Tcl_Panic (msg " (" #x "), in file " __FILE__ " @line " STR(__LINE__));}
#define ASSERT_BOUNDS(i,n) ASSERT (RANGEOK(i,n),"array index out of bounds: " STR(i) " > " STR(n))
#else
#define ASSERT(x,msg)
#define ASSERT_BOUNDS(i,n)
#endif

#endif /* DWARFDBG_INT_H */
/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 2
 * fill-column: 78
 * End:
 */
