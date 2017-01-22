/*
* Copyright (c) 2016, Arnulf P. Wiedemann (arnulf@wiedemann-pri.de)
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
 * File:   compMsgAction.h
 * Author: Arnulf P. Wiedemann
 *
 * Created on December 19, 2016
 */

#ifndef COMP_MSG_OTA_H
#define	COMP_MSG_OTA_H

#include "c_types.h"
#ifdef	__cplusplus
extern "C" {
#endif

typedef struct compMsgDispatcher compMsgDispatcher_t;

typedef uint8_t (* updateFirmware_t)(compMsgDispatcher_t *self);
typedef uint8_t (* updateSpiffs_t)(compMsgDispatcher_t *self);
typedef uint8_t (* otaStart_t)(compMsgDispatcher_t *self, ota_callback callback, bool flashfs);
typedef uint8_t (* checkClientMode_t)(compMsgDispatcher_t *self, bool isSpiffs);
typedef uint8_t (* storeUserData_t)(compMsgDispatcher_t *self, size_t msgLgth, uint8_t *msgData);
typedef uint8_t (* saveUserData_t)(compMsgDispatcher_t *self);
typedef uint8_t (* getUserData_t)(compMsgDispatcher_t *self, uint8_t **msgData, size_t *msgLgth);

typedef struct compMsgOta {

  updateFirmware_t updateFirmware;
  updateSpiffs_t updateSpiffs;
  otaStart_t otaStart;
  checkClientMode_t checkClientMode;
  storeUserData_t storeUserData;
  saveUserData_t saveUserData;
  getUserData_t getUserData;
} compMsgOta_t;

compMsgOta_t *newCompMsgOta();
uint8_t compMsgOtaInit(compMsgDispatcher_t *self);

#ifdef  __cplusplus
}
#endif

#endif  /* COMP_MSG_OTA_H */

