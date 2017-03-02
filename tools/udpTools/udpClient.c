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
 * File:   udpClient.c
 * Author: Arnulf P. Wiedemann <arnulf@wiedemann-pri.de>
 *
 * Created on January 22, 2017
 */

#include "udpClientInt.h"

/*
 * = = == === ===== ======== ============= =====================
 */

#define checkErrOK2(result) if(result != UDP_CLIENT_ERR_OK) return NULL
#define checkAllocOK(var) if(var == NULL) return NULL

// =================================== udpClientNew =========================== 

udpClientPtr_t udpClientNew (Tcl_Interp *interp) {
  int result;

  _udpClient_t *dbg = ALLOC (_udpClient_t);
  memset(dbg, 0, sizeof(_udpClient_t));
  dbg->interp = interp;

  // udpClientDeviceInfo module
  dbg->udpClientDeviceInfo = ALLOC(udpClientDeviceInfo_t);
  checkAllocOK(dbg->udpClientDeviceInfo);
  memset(dbg->udpClientDeviceInfo, 0, sizeof(udpClientDeviceInfo_t));

  // udpClientEtherInfo module
  dbg->udpClientEtherInfo = ALLOC(udpClientEtherInfo_t);
  checkAllocOK(dbg->udpClientEtherInfo);
  memset(dbg->udpClientEtherInfo, 0, sizeof(udpClientEtherInfo_t));

  // udpClientUdpData module
  dbg->udpClientUdpData = ALLOC(udpClientUdpData_t);
  checkAllocOK(dbg->udpClientUdpData);
  memset(dbg->udpClientUdpData, 0, sizeof(udpClientUdpData_t));

  // udpClientUdpInfo module
  dbg->udpClientUdpInfo = ALLOC(udpClientUdpInfo_t);
  checkAllocOK(dbg->udpClientUdpInfo);
  memset(dbg->udpClientUdpInfo, 0, sizeof(udpClientUdpInfo_t));

  return dbg;
}

// =================================== udpClientDel =========================== 

void
udpClientDel (udpClientPtr_t self) {
  ckfree ((char*) self);
}

// =================================== udpClientClientDataSet =========================== 

void
udpClientClientDataSet (udpClientPtr_t self, void* clientdata) {
  self->clientData = clientdata;
}

// =================================== udpClientClientDataGet =========================== 

void* udpClientClientDataGet (udpClientPtr_t self) {
  return self->clientData;
}

// =================================== udpClientInit =========================== 

int udpClientInit (udpClientPtr_t self) {
  int result;

  // udpClientDeviceInfo module
  result = udpClientDeviceInfoInit(self);
  checkErrOK(result);

  // udpClientUdpData module
  result = udpClientUdpDataInit(self);
  checkErrOK(result);

  // udpClientEtherInfo module
  result = udpClientEtherInfoInit(self);
  checkErrOK(result);

  // udpClientUdpInfo module
  result = udpClientUdpInfoInit(self);
  checkErrOK(result);

// add all other init parts for modules here !!
  return TCL_OK;
}

// =================================== udpClientGetErrorStr =========================== 

char * udpClientGetErrorStr (udpClientPtr_t self) {
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
