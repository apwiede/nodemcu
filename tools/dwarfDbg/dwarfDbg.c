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

  // dwarfDbgEsb module
  dbg->dwarfDbgEsb = ALLOC(dwarfDbgEsb_t);
  checkAllocOK(dbg->dwarfDbgEsb);
  memset(dbg->dwarfDbgEsb, 0, sizeof(dwarfDbgEsb_t));

  // dwarfDbgUtil module
  dbg->dwarfDbgUtil = ALLOC(dwarfDbgUtil_t);
  checkAllocOK(dbg->dwarfDbgUtil);
  memset(dbg->dwarfDbgUtil, 0, sizeof(dwarfDbgUtil_t));

  // dwarfDbgDict module
  dbg->dwarfDbgDict = ALLOC(dwarfDbgDict_t);
  checkAllocOK(dbg->dwarfDbgDict);
  memset(dbg->dwarfDbgDict, 0, sizeof(dwarfDbgDict_t));

  // dwarfDbgFileInfo module
  dbg->dwarfDbgFileInfo = ALLOC(dwarfDbgFileInfo_t);
  checkAllocOK(dbg->dwarfDbgFileInfo);
  memset(dbg->dwarfDbgFileInfo, 0, sizeof(dwarfDbgFileInfo_t));

  // dwarfDbgElfInfo module
  dbg->dwarfDbgElfInfo = ALLOC(dwarfDbgElfInfo_t);
  checkAllocOK(dbg->dwarfDbgElfInfo);
  memset(dbg->dwarfDbgElfInfo, 0, sizeof(dwarfDbgElfInfo_t));

  // dwarfDbgGetInfo module
  dbg->dwarfDbgGetInfo = ALLOC(dwarfDbgGetInfo_t);
  checkAllocOK(dbg->dwarfDbgGetInfo);
  memset(dbg->dwarfDbgGetInfo, 0, sizeof(dwarfDbgGetInfo_t));

  return dbg;
}

// =================================== dwarfDbgDel =========================== 

void
dwarfDbgDel (dwarfDbgPtr_t dbg) {
  ckfree ((char*) dbg);
}

// =================================== dwarfDbgClientDataSet =========================== 

void
dwarfDbgClientDataSet (dwarfDbgPtr_t dbg, void* clientdata) {
  dbg->clientData = clientdata;
}

// =================================== dwarfDbgClientDataGet =========================== 

void* dwarfDbgClientDataGet (dwarfDbgPtr_t dbg) {
  return dbg->clientData;
}

// =================================== dwarfDbgInit =========================== 

int dwarfDbgInit (dwarfDbgPtr_t dbg) {
  int result;

  // dwarfDbgEsb module
  result = dwarfDbgEsbInit(dbg);
  checkErrOK(result);

  // dwarfDbgUtil module
  result = dwarfDbgUtilInit(dbg);
  checkErrOK(result);

  // dwarfDbgDict module
  result = dwarfDbgDictInit(dbg);
  checkErrOK(result);

  // dwarfDbgFileInfo module
  result = dwarfDbgFileInfoInit(dbg);
  checkErrOK(result);

  // dwarfDbgElfInfo module
  result = dwarfDbgElfInfoInit(dbg);
  checkErrOK(result);

  // dwarfDbgGetInfo module
  result = dwarfDbgGetInfoInit(dbg);
  checkErrOK(result);

// add all other init parts for modules here !!
  return TCL_OK;
}

// =================================== dwarfDbgGetErrorStr =========================== 

char * dwarfDbgGetErrorStr (dwarfDbgPtr_t dbg) {
  return dbg->errorStr;
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
