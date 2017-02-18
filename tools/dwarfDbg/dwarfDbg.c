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
 * File:   dwarfDbg.c
 * Author: Arnulf P. Wiedemann <arnulf@wiedemann-pri.de>
 *
 * Created on January 22, 2017
 */

#include "dwarfDbgInt.h"

/*
 * = = == === ===== ======== ============= =====================
 */

#define checkErrOK2(result) if(result != DWARF_DBG_ERR_OK) return NULL
#define checkAllocOK(var) if(var == NULL) return NULL

// =================================== dwarfDbgNew =========================== 

dwarfDbgPtr_t dwarfDbgNew (Tcl_Interp *interp) {
  int result;

  _dwarfDbg_t *dbg = ALLOC (_dwarfDbg_t);
  memset(dbg, 0, sizeof(_dwarfDbg_t));
  dbg->interp = interp;

  // dwarfDbgDebugInfo module
  dbg->dwarfDbgDebugInfo = ALLOC(dwarfDbgDebugInfo_t);
  checkAllocOK(dbg->dwarfDbgDebugInfo);
  memset(dbg->dwarfDbgDebugInfo, 0, sizeof(dwarfDbgDebugInfo_t));

  // dwarfDbgStringInfo module
  dbg->dwarfDbgStringInfo = ALLOC(dwarfDbgStringInfo_t);
  checkAllocOK(dbg->dwarfDbgStringInfo);
  memset(dbg->dwarfDbgStringInfo, 0, sizeof(dwarfDbgStringInfo_t));

  // dwarfDbgUtil module
  dbg->dwarfDbgUtil = ALLOC(dwarfDbgUtil_t);
  checkAllocOK(dbg->dwarfDbgUtil);
  memset(dbg->dwarfDbgUtil, 0, sizeof(dwarfDbgUtil_t));

  // dwarfDbgDict module
  dbg->dwarfDbgDict = ALLOC(dwarfDbgDict_t);
  checkAllocOK(dbg->dwarfDbgDict);
  memset(dbg->dwarfDbgDict, 0, sizeof(dwarfDbgDict_t));

  // dwarfDbgTypeInfo module
  dbg->dwarfDbgTypeInfo = ALLOC(dwarfDbgTypeInfo_t);
  checkAllocOK(dbg->dwarfDbgTypeInfo);
  memset(dbg->dwarfDbgTypeInfo, 0, sizeof(dwarfDbgTypeInfo_t));

  // dwarfDbgFrameInfo module
  dbg->dwarfDbgFrameInfo = ALLOC(dwarfDbgFrameInfo_t);
  checkAllocOK(dbg->dwarfDbgFrameInfo);
  memset(dbg->dwarfDbgFrameInfo, 0, sizeof(dwarfDbgFrameInfo_t));

  // dwarfDbgAttributeInfo module
  dbg->dwarfDbgAttributeInfo = ALLOC(dwarfDbgAttributeInfo_t);
  checkAllocOK(dbg->dwarfDbgAttributeInfo);
  memset(dbg->dwarfDbgAttributeInfo, 0, sizeof(dwarfDbgAttributeInfo_t));

  // dwarfDbgLocationInfo module
  dbg->dwarfDbgLocationInfo = ALLOC(dwarfDbgLocationInfo_t);
  checkAllocOK(dbg->dwarfDbgLocationInfo);
  memset(dbg->dwarfDbgLocationInfo, 0, sizeof(dwarfDbgLocationInfo_t));

  // dwarfDbgAddrInfo module
  dbg->dwarfDbgAddrInfo = ALLOC(dwarfDbgAddrInfo_t);
  checkAllocOK(dbg->dwarfDbgAddrInfo);
  memset(dbg->dwarfDbgAddrInfo, 0, sizeof(dwarfDbgAddrInfo_t));

  // dwarfDbgFileInfo module
  dbg->dwarfDbgFileInfo = ALLOC(dwarfDbgFileInfo_t);
  checkAllocOK(dbg->dwarfDbgFileInfo);
  memset(dbg->dwarfDbgFileInfo, 0, sizeof(dwarfDbgFileInfo_t));

  // dwarfDbgRangeInfo module
  dbg->dwarfDbgRangeInfo = ALLOC(dwarfDbgRangeInfo_t);
  checkAllocOK(dbg->dwarfDbgRangeInfo);
  memset(dbg->dwarfDbgRangeInfo, 0, sizeof(dwarfDbgRangeInfo_t));

  // dwarfDbgSubProgramInfo module
  dbg->dwarfDbgSubProgramInfo = ALLOC(dwarfDbgSubProgramInfo_t);
  checkAllocOK(dbg->dwarfDbgSubProgramInfo);
  memset(dbg->dwarfDbgSubProgramInfo, 0, sizeof(dwarfDbgSubProgramInfo_t));

  // dwarfDbgLineInfo module
  dbg->dwarfDbgLineInfo = ALLOC(dwarfDbgLineInfo_t);
  checkAllocOK(dbg->dwarfDbgLineInfo);
  memset(dbg->dwarfDbgLineInfo, 0, sizeof(dwarfDbgLineInfo_t));

  // dwarfDbgDieInfo module
  dbg->dwarfDbgDieInfo = ALLOC(dwarfDbgDieInfo_t);
  checkAllocOK(dbg->dwarfDbgDieInfo);
  memset(dbg->dwarfDbgDieInfo, 0, sizeof(dwarfDbgDieInfo_t));

  // dwarfDbgCompileUnitInfo module
  dbg->dwarfDbgCompileUnitInfo = ALLOC(dwarfDbgCompileUnitInfo_t);
  checkAllocOK(dbg->dwarfDbgCompileUnitInfo);
  memset(dbg->dwarfDbgCompileUnitInfo, 0, sizeof(dwarfDbgCompileUnitInfo_t));

  // dwarfDbgElfInfo module
  dbg->dwarfDbgElfInfo = ALLOC(dwarfDbgElfInfo_t);
  checkAllocOK(dbg->dwarfDbgElfInfo);
  memset(dbg->dwarfDbgElfInfo, 0, sizeof(dwarfDbgElfInfo_t));

  // dwarfDbgGetDbgInfo module
  dbg->dwarfDbgGetDbgInfo = ALLOC(dwarfDbgGetDbgInfo_t);
  checkAllocOK(dbg->dwarfDbgGetDbgInfo);
  memset(dbg->dwarfDbgGetDbgInfo, 0, sizeof(dwarfDbgGetDbgInfo_t));

  return dbg;
}

// =================================== dwarfDbgDel =========================== 

void
dwarfDbgDel (dwarfDbgPtr_t self) {
  ckfree ((char*) self);
}

// =================================== dwarfDbgClientDataSet =========================== 

void
dwarfDbgClientDataSet (dwarfDbgPtr_t self, void* clientdata) {
  self->clientData = clientdata;
}

// =================================== dwarfDbgClientDataGet =========================== 

void* dwarfDbgClientDataGet (dwarfDbgPtr_t self) {
  return self->clientData;
}

// =================================== dwarfDbgInit =========================== 

int dwarfDbgInit (dwarfDbgPtr_t self) {
  int result;

  // dwarfDbgDebugInfo module
  result = dwarfDbgDebugInfoInit(self);
  checkErrOK(result);

  // dwarfDbgUtil module
  result = dwarfDbgUtilInit(self);
  checkErrOK(result);

  // dwarfDbgDict module
  result = dwarfDbgDictInit(self);
  checkErrOK(result);

  // dwarfDbgTypeInfo module
  result = dwarfDbgTypeInfoInit(self);
  checkErrOK(result);

  // dwarfDbgFrameInfo module
  result = dwarfDbgFrameInfoInit(self);
  checkErrOK(result);

  // dwarfDbgAttributeInfo module
  result = dwarfDbgAttributeInfoInit(self);
  checkErrOK(result);

  // dwarfDbgLocationInfo module
  result = dwarfDbgLocationInfoInit(self);
  checkErrOK(result);

  // dwarfDbgAddrInfo module
  result = dwarfDbgAddrInfoInit(self);
  checkErrOK(result);

  // dwarfDbgFileInfo module
  result = dwarfDbgFileInfoInit(self);
  checkErrOK(result);

  // dwarfDbgRangeInfo module
  result = dwarfDbgRangeInfoInit(self);
  checkErrOK(result);

  // dwarfDbgSubProgramInfo module
  result = dwarfDbgSubProgramInfoInit(self);
  checkErrOK(result);

  // dwarfDbgLineInfo module
  result = dwarfDbgLineInfoInit(self);
  checkErrOK(result);

  // dwarfDbgStringInfo module
  result = dwarfDbgStringInfoInit(self);
  checkErrOK(result);

  // dwarfDbgDieInfo module
  result = dwarfDbgDieInfoInit(self);
  checkErrOK(result);

  // dwarfDbgCompileUnitInfo module
  result = dwarfDbgCompileUnitInfoInit(self);
  checkErrOK(result);

  // dwarfDbgElfInfo module
  result = dwarfDbgElfInfoInit(self);
  checkErrOK(result);

  // dwarfDbgGetDbgInfo module
  result = dwarfDbgGetDbgInfoInit(self);
  checkErrOK(result);

// add all other init parts for modules here !!
  return TCL_OK;
}

// =================================== dwarfDbgGetErrorStr =========================== 

char * dwarfDbgGetErrorStr (dwarfDbgPtr_t self) {
  return self->errorStr;
}

/*
 * = = == === ===== ======== ============= =====================
 */

/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 2
 * fill-column: 78
 * End:
 */
