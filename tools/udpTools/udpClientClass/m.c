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
 * Created on March 01, 2017
 */

/* ::udpClient - critcl - layer 3 definitions.
 *
 * -> Method functions.
 *    Implementations for all udpClient methods.
 */

#include "util.h"
#include "m.h"

/* .................................................. */

/*---------------------------------------------------------------------------
 * stm_CLEAR --
 *	claen up all udp info. 
 * Results:
 *	A standard Tcl result code.
 * Side effects:
 *	Only internal, memory allocation changes ...
 *---------------------------------------------------------------------------
 */

int
stm_CLEAR (udpClientPtr_t self, Tcl_Interp* interp, int objc, Tcl_Obj* CONST* objv) {
  /* Syntax: udpClient clear
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
 *	Destroys the whole udpClient object.
 * Results:
 *	A standard Tcl result code.
 * Side effects:
 *	Releases memory.
 *---------------------------------------------------------------------------
 */

int stm_DESTROY (udpClientPtr_t self, Tcl_Interp* interp, int objc, Tcl_Obj* CONST* objv) {
  /* Syntax: udpClient destroy
   *	       [0]      [1]
   */
  if (objc != 2) {
    Tcl_WrongNumArgs (interp, 2, objv, NULL);
    return TCL_ERROR;
  }
  Tcl_DeleteCommandFromToken(interp, (Tcl_Command) udpClientClientDataGet (self));
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

int stm_INIT (udpClientPtr_t self, Tcl_Interp* interp, int objc, Tcl_Obj* CONST* objv) {
  int result;

  /* Syntax: udpClient init
   *	       [0]       [1]
   */
  if ((objc != 2)) {
    Tcl_WrongNumArgs (interp, 2, objv, NULL);
    return TCL_ERROR;
  }
  result = udpClientInit(self);
  if (result != TCL_OK) {
    Tcl_SetResult  (interp, udpClientGetErrorStr(self), TCL_STATIC);
  } 
  return result;
}

/*---------------------------------------------------------------------------
 * stm_OPEN_DEVICE --
 *	just for testing
 * Results:
 *	A standard Tcl result code.
 * Side effects:
 *	None.
 *---------------------------------------------------------------------------
 */

int stm_OPEN_DEVICE (udpClientPtr_t self, Tcl_Interp* interp, int objc, Tcl_Obj* CONST* objv) {
  int result;

  /* Syntax: udpClient openDevice deviceName
   *	       [0]       [1]        [2]
   */
  if ((objc < 3)) {
    Tcl_WrongNumArgs (interp, objc, objv, "deviceName");
    return TCL_ERROR;
  }
  if ((objc != 3)) {
    Tcl_WrongNumArgs (interp, objc, objv, NULL);
    return TCL_ERROR;
  }
  result = udpClientOpenDevice (self, Tcl_GetString(objv[2]));
  if (result != TCL_OK) {
    Tcl_SetResult  (interp, udpClientGetErrorStr(self), TCL_STATIC);
  } 
  return result;
}

/*---------------------------------------------------------------------------
 * stm_GET_UDP_INFOS --
 *      get the necessary udp infos from the device
 * Results:
 *      A standard Tcl result code.
 * Side effects:
 *      None.
 *---------------------------------------------------------------------------
 */

int stm_GET_UDP_INFOS (udpClientPtr_t self, Tcl_Interp* interp, int objc, Tcl_Obj* CONST* objv) {
  int result;

  /* Syntax: udpClient getUdpInfos
   *           [0]       [1]
   */
  if ((objc != 2)) {
    Tcl_WrongNumArgs (interp, objc, objv, NULL);
    return TCL_ERROR;
  }
  result = udpClientGetUdpInfos (self);
  if (result != TCL_OK) {
    Tcl_SetResult  (interp, udpClientGetErrorStr(self), TCL_STATIC);
  }
  return result;
}

/*---------------------------------------------------------------------------
 * stm_CLOSE_DEVICE --
 *	just for testing
 * Results:
 *	A standard Tcl result code.
 * Side effects:
 *	None.
 *---------------------------------------------------------------------------
 */

int stm_CLOSE_DEVICE (udpClientPtr_t self, Tcl_Interp* interp, int objc, Tcl_Obj* CONST* objv) {
  int result;

  /* Syntax: udpClient closeDevice
   *	       [0]       [1]
   */
  if ((objc != 2)) {
    Tcl_WrongNumArgs (interp, objc, objv, NULL);
    return TCL_ERROR;
  }
  result = udpClientCloseDevice (self);
  if (result != TCL_OK) {
    Tcl_SetResult  (interp, udpClientGetErrorStr(self), TCL_STATIC);
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

int stm_GET_ERROR_STR (udpClientPtr_t self, Tcl_Interp* interp, int objc, Tcl_Obj* CONST* objv) {
  int result;

  Tcl_SetResult  (interp, udpClientGetErrorStr(self), NULL);
  return TCL_OK;
}
/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 2
 * fill-column: 78
 * End:
 */
