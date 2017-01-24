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

/*
 *---------------------------------------------------------------------------
 *
 * stm_CLEAR --
 *
 *	claen up all dwarf debug info. 
 *
 * Results:
 *	A standard Tcl result code.
 *
 * Side effects:
 *	Only internal, memory allocation changes ...
 *
 *---------------------------------------------------------------------------
 */

int
stm_CLEAR (dwarfDbgPtr_t dbg, Tcl_Interp* interp, int objc, Tcl_Obj* CONST* objv)
{
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

/*
 *---------------------------------------------------------------------------
 *
 * stm_DESTROY --
 *
 *	Destroys the whole dwarfDbg object.
 *
 * Results:
 *	A standard Tcl result code.
 *
 * Side effects:
 *	Releases memory.
 *
 *---------------------------------------------------------------------------
 */

int
stm_DESTROY (dwarfDbgPtr_t dbg, Tcl_Interp* interp, int objc, Tcl_Obj* CONST* objv)
{
  /* Syntax: dwarfDbg destroy
   *	       [0]      [1]
   */

  if (objc != 2) {
    Tcl_WrongNumArgs (interp, 2, objv, NULL);
    return TCL_ERROR;
  }

  Tcl_DeleteCommandFromToken(interp, (Tcl_Command) dwarfDbgClientDataGet (dbg));
  return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * stm_FCN1 --
 *
 *	Returns ??? 
 *
 * Results:
 *	A standard Tcl result code.
 *
 * Side effects:
 *	None.
 *
 *---------------------------------------------------------------------------
 */

int
stm_INIT (dwarfDbgPtr_t dbg, Tcl_Interp* interp, int objc, Tcl_Obj* CONST* objv)
{
  int result;

  /* Syntax: dwardfDbg init
   *	       [0]       [1]
   */

printf("stm_INIT\n");
fflush(stdout);
  if ((objc != 2)) {
    Tcl_WrongNumArgs (interp, 2, objv, NULL);
    return TCL_ERROR;
  }

  result = dwarfDbgInit(dbg);
  if (result != TCL_OK) {
    Tcl_SetResult  (interp, dwarfDbgGetErrorStr(dbg), TCL_STATIC);
  } 
  return result;
}

/*
 *---------------------------------------------------------------------------
 *
 * stm_OPEN_ELF --
 *
 *	just for testing
 *
 * Results:
 *	A standard Tcl result code.
 *
 * Side effects:
 *	None.
 *
 *---------------------------------------------------------------------------
 */

int
stm_OPEN_ELF (dwarfDbgPtr_t dbg, Tcl_Interp* interp, int objc, Tcl_Obj* CONST* objv)
{
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

printf("stm_OPEN_ELF\n");
  result = dwarfDbgOpenElf (dbg, Tcl_GetString(objv[2]));
  if (result != TCL_OK) {
    Tcl_SetResult  (interp, dwarfDbgGetErrorStr(dbg), TCL_STATIC);
  } 
  return result;
}

/*
 *---------------------------------------------------------------------------
 *
 * stm_GET_FILES --
 *
 *	just for testing
 *
 * Results:
 *	A standard Tcl result code.
 *
 * Side effects:
 *	None.
 *
 *---------------------------------------------------------------------------
 */

int
stm_GET_FILES (dwarfDbgPtr_t dbg, Tcl_Interp* interp, int objc, Tcl_Obj* CONST* objv)
{
  int result;
  /* Syntax: dwardfdbg getFiles
   *	       [0]       [1]
   */

  if ((objc != 2)) {
    Tcl_WrongNumArgs (interp, objc, objv, NULL);
    return TCL_ERROR;
  }

printf("stm_GET_FILES\n");
  result = dwarfDbgGetFiles (dbg);
  if (result != TCL_OK) {
    Tcl_SetResult  (interp, dwarfDbgGetErrorStr(dbg), TCL_STATIC);
  } 
  return result;
}

/*
 *---------------------------------------------------------------------------
 *
 * stm_CLOSE_ELF --
 *
 *	just for testing
 *
 * Results:
 *	A standard Tcl result code.
 *
 * Side effects:
 *	None.
 *
 *---------------------------------------------------------------------------
 */

int
stm_CLOSE_ELF (dwarfDbgPtr_t dbg, Tcl_Interp* interp, int objc, Tcl_Obj* CONST* objv)
{
  int result;
  /* Syntax: dwardfdbg closeElf
   *	       [0]       [1]
   */

  if ((objc != 2)) {
    Tcl_WrongNumArgs (interp, objc, objv, NULL);
    return TCL_ERROR;
  }

printf("stm_CLOSE_ELF\n");
  result = dwarfDbgCloseElf (dbg);
  if (result != TCL_OK) {
    Tcl_SetResult  (interp, dwarfDbgGetErrorStr(dbg), TCL_STATIC);
  } 
  return result;
}

/*
 *---------------------------------------------------------------------------
 *
 * stm_GET_ERROR_STR --
 *
 *	just for testing
 *
 * Results:
 *	A standard Tcl result code.
 *
 * Side effects:
 *	None.
 *
 *---------------------------------------------------------------------------
 */

int
stm_GET_ERROR_STR (dwarfDbgPtr_t dbg, Tcl_Interp* interp, int objc, Tcl_Obj* CONST* objv)
{
  int result;

printf("stm_GET_ERROR_STR\n");
  Tcl_SetResult  (interp, dwarfDbgGetErrorStr(dbg), NULL);
  return TCL_OK;
}
/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 2
 * fill-column: 78
 * End:
 */