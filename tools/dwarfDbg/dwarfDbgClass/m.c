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
 * File:   m.c
 * Author: Arnulf P. Wiedemann <arnulf@wiedemann-pri.de>
 *
 * Created on January 22, 2017
 */

/* ::dwarfDbg - critcl - layer 3 definitions.
 *
 * -> Method functions.
 *    Implementations for all dwarfDbg methods.
 */

#include "util.h"
#include "m.h"

/* .................................................. */

/*---------------------------------------------------------------------------
 * stm_CLEAR --
 *	claen up all dwarf debug info. 
 * Results:
 *	A standard Tcl result code.
 * Side effects:
 *	Only internal, memory allocation changes ...
 *---------------------------------------------------------------------------
 */

int
stm_CLEAR (dwarfDbgPtr_t self, Tcl_Interp* interp, int objc, Tcl_Obj* CONST* objv) {
  /* Syntax: dwarfDbg clear
   *	       [0]      [1]
   */
  if (objc != 2) {
    Tcl_WrongNumArgs (interp, 2, objv, NULL);
    return TCL_ERROR;
  }
//FIXME need code here
  return TCL_OK;
}
/*---------------------------------------------------------------------------
 * stm_DESTROY --
 *	Destroys the whole dwarfDbg object.
 * Results:
 *	A standard Tcl result code.
 * Side effects:
 *	Releases memory.
 *---------------------------------------------------------------------------
 */

int stm_DESTROY (dwarfDbgPtr_t self, Tcl_Interp* interp, int objc, Tcl_Obj* CONST* objv) {
  /* Syntax: dwarfDbg destroy
   *	       [0]      [1]
   */
  if (objc != 2) {
    Tcl_WrongNumArgs (interp, 2, objv, NULL);
    return TCL_ERROR;
  }
  Tcl_DeleteCommandFromToken(interp, (Tcl_Command) dwarfDbgClientDataGet (self));
  return TCL_OK;
}

/*---------------------------------------------------------------------------
 * stm_INIT --
 *	Returns ??? 
 * Results:
 *	A standard Tcl result code.
 * Side effects:
 *	None.
 *---------------------------------------------------------------------------
 */

int stm_INIT (dwarfDbgPtr_t self, Tcl_Interp* interp, int objc, Tcl_Obj* CONST* objv) {
  int result;

  /* Syntax: dwardfDbg init
   *	       [0]       [1]
   */
  if ((objc != 2)) {
    Tcl_WrongNumArgs (interp, 2, objv, NULL);
    return TCL_ERROR;
  }
  result = dwarfDbgInit(self);
  if (result != TCL_OK) {
    Tcl_SetResult  (interp, dwarfDbgGetErrorStr(self), TCL_STATIC);
  } 
  return result;
}

/*---------------------------------------------------------------------------
 * stm_OPEN_ELF --
 *	just for testing
 * Results:
 *	A standard Tcl result code.
 * Side effects:
 *	None.
 *---------------------------------------------------------------------------
 */

int stm_OPEN_ELF (dwarfDbgPtr_t self, Tcl_Interp* interp, int objc, Tcl_Obj* CONST* objv) {
  int result;

  /* Syntax: dwardfdbg openElf fileName
   *	       [0]       [1]     [2]
   */
  if ((objc < 3)) {
    Tcl_WrongNumArgs (interp, objc, objv, "fileName");
    return TCL_ERROR;
  }
  if ((objc != 3)) {
    Tcl_WrongNumArgs (interp, objc, objv, NULL);
    return TCL_ERROR;
  }
  result = dwarfDbgOpenElf (self, Tcl_GetString(objv[2]));
  if (result != TCL_OK) {
    Tcl_SetResult  (interp, dwarfDbgGetErrorStr(self), TCL_STATIC);
  } 
  return result;
}

/*---------------------------------------------------------------------------
 * stm_GET_FILES --
 *	just for testing
 * Results:
 *	A standard Tcl result code.
 * Side effects:
 *	None.
 *---------------------------------------------------------------------------
 */

int stm_GET_FILES (dwarfDbgPtr_t self, Tcl_Interp* interp, int objc, Tcl_Obj* CONST* objv) {
  int result;

  /* Syntax: dwardfdbg getFiles
   *	       [0]       [1]
   */
  if ((objc != 2)) {
    Tcl_WrongNumArgs (interp, objc, objv, NULL);
    return TCL_ERROR;
  }
  result = dwarfDbgGetFiles (self);
  if (result != TCL_OK) {
    Tcl_SetResult  (interp, dwarfDbgGetErrorStr(self), TCL_STATIC);
  } 
  return result;
}

/*---------------------------------------------------------------------------
 * stm_GET_DBG_INFOS --
 *	get the necessary debug infos from the object file
 * Results:
 *	A standard Tcl result code.
 * Side effects:
 *	None.
 *---------------------------------------------------------------------------
 */

int stm_GET_DBG_INFOS (dwarfDbgPtr_t self, Tcl_Interp* interp, int objc, Tcl_Obj* CONST* objv) {
  int result;

  /* Syntax: dwardfdbg getDbgInfos
   *	       [0]       [1]
   */
  if ((objc != 2)) {
    Tcl_WrongNumArgs (interp, objc, objv, NULL);
    return TCL_ERROR;
  }
  result = dwarfDbgGetDbgInfos (self);
  if (result != TCL_OK) {
    Tcl_SetResult  (interp, dwarfDbgGetErrorStr(self), TCL_STATIC);
  } 
  return result;
}

/*---------------------------------------------------------------------------
 * stm_GET_FILE_INFOS --
 *	return the file infos for use by the debugger
 * Results:
 *	A standard Tcl result code.
 * Side effects:
 *	None.
 *---------------------------------------------------------------------------
 */

int stm_GET_FILE_INFOS (dwarfDbgPtr_t self, Tcl_Interp* interp, int objc, Tcl_Obj* CONST* objv) {
  int result;

  /* Syntax: dwardfdbg getFileInfos
   *	       [0]       [1]
   */
  if ((objc != 2)) {
    Tcl_WrongNumArgs (interp, objc, objv, NULL);
    return TCL_ERROR;
  }
  result = dwarfDbgGetFileInfos (self);
  if (result != TCL_OK) {
    Tcl_SetResult  (interp, dwarfDbgGetErrorStr(self), TCL_STATIC);
  } 
  return result;
}

/*---------------------------------------------------------------------------
 * stm_GET_VAR_ADDR --
 *	return the address of a variable depending on pc and fp register value
 *      for use by the debugger
 * Results:
 *	A standard Tcl result code.
 * Side effects:
 *	None.
 *---------------------------------------------------------------------------
 */

int stm_GET_VAR_ADDR (dwarfDbgPtr_t self, Tcl_Interp* interp, int objc, Tcl_Obj* CONST* objv) {
  int result;
  int pc;
  int fp;
  int sourceLineNo;
  int addr;
  Tcl_Obj *objPtr;

  /* Syntax: dwardfdbg getVarAddr sourceFileName sourceLineNo varName pc  fp
   *	       [0]       [1]            [2]            [3]  [    4]   [5] [6]
   */
  if ((objc != 7)) {
    Tcl_WrongNumArgs (interp, objc, objv, NULL);
    return TCL_ERROR;
  }
  if (Tcl_GetIntFromObj(interp, objv[3], &sourceLineNo) != TCL_OK) {
    return TCL_ERROR;
  }
  if (Tcl_GetIntFromObj(interp, objv[5], &pc) != TCL_OK) {
    return TCL_ERROR;
  }
  if (Tcl_GetIntFromObj(interp, objv[6], &fp) != TCL_OK) {
    return TCL_ERROR;
  }
  result = dwarfDbgGetVarAddr (self, Tcl_GetString(objv[2]),sourceLineNo, Tcl_GetString(objv[4]), pc, fp, &addr);
  if (result != TCL_OK) {
    Tcl_SetResult  (interp, dwarfDbgGetErrorStr(self), TCL_STATIC);
  } else {
    objPtr = Tcl_NewIntObj(addr);
    Tcl_SetObjResult(interp, objPtr);
  }
  return result;
}

/*---------------------------------------------------------------------------
 * stm_GET_FILE_LINES --
 *	return the file infos for use by the debugger
 * Results:
 *	A standard Tcl result code.
 * Side effects:
 *	None.
 *---------------------------------------------------------------------------
 */

int stm_GET_FILE_LINES (dwarfDbgPtr_t self, Tcl_Interp* interp, int objc, Tcl_Obj* CONST* objv) {
  int result;
  int value;

  /* Syntax: dwardfdbg getFileLines compileUnitIdx
   *	       [0]       [1]        [2]
   */
  if ((objc != 3)) {
    Tcl_WrongNumArgs (interp, objc, objv, NULL);
    return TCL_ERROR;
  }
  result = Tcl_GetIntFromObj(interp, objv[2], &value);
  result = dwarfDbgGetFileLines (self, value);
  if (result != TCL_OK) {
    Tcl_SetResult  (interp, dwarfDbgGetErrorStr(self), TCL_STATIC);
  } 
  return result;
}

/*---------------------------------------------------------------------------
 * stm_CLOSE_ELF --
 *	just for testing
 * Results:
 *	A standard Tcl result code.
 * Side effects:
 *	None.
 *---------------------------------------------------------------------------
 */

int stm_CLOSE_ELF (dwarfDbgPtr_t self, Tcl_Interp* interp, int objc, Tcl_Obj* CONST* objv) {
  int result;

  /* Syntax: dwardfdbg closeElf
   *	       [0]       [1]
   */
  if ((objc != 2)) {
    Tcl_WrongNumArgs (interp, objc, objv, NULL);
    return TCL_ERROR;
  }
  result = dwarfDbgCloseElf (self);
  if (result != TCL_OK) {
    Tcl_SetResult  (interp, dwarfDbgGetErrorStr(self), TCL_STATIC);
  } 
  return result;
}

/*---------------------------------------------------------------------------
 * stm_GET_ERROR_STR --
 *	just for testing
 * Results:
 *	A standard Tcl result code.
 * Side effects:
 *	None.
 *---------------------------------------------------------------------------
 */

int stm_GET_ERROR_STR (dwarfDbgPtr_t self, Tcl_Interp* interp, int objc, Tcl_Obj* CONST* objv) {
  int result;

  Tcl_SetResult  (interp, dwarfDbgGetErrorStr(self), NULL);
  return TCL_OK;
}
/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 2
 * fill-column: 78
 * End:
 */
