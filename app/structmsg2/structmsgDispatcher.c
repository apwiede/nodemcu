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
 * File:   structmsgDispatcher.c
 * Author: Arnulf P. Wiedemann
 *
 * Created on October 2st, 2016
 */

#include "osapi.h"
#include "c_types.h"
#include "mem.h"
#include "flash_fs.h"

#include "c_string.h"
#include "c_stdio.h"
#include "structmsgDispatcher.h"


static int structmsgDispatcherId = 0;

// ================================= BMsg ====================================

static uint8_t BMsg(structmsgDispatcher_t *self) {
  return STRUCT_DATA_DISP_ERR_OK;
}

// ================================= IMsg ====================================

static uint8_t IMsg(structmsgDispatcher_t *self) {
  return STRUCT_DATA_DISP_ERR_OK;
}

// ================================= MMsg ====================================

static uint8_t MMsg(structmsgDispatcher_t *self) {
  return STRUCT_DATA_DISP_ERR_OK;
}

// ================================= defaultMsg ====================================

static uint8_t defaultMsg(structmsgDispatcher_t *self) {
  return STRUCT_DATA_DISP_ERR_OK;
}

// ================================= newStructmsgDispatcher ====================================

structmsgDispatcher_t *newStructmsgDispatcher() {
  structmsgDispatcher_t *structmsgDispatcher = os_zalloc(sizeof(structmsgDispatcher_t));
  if (structmsgDispatcher == NULL) {
    return NULL;
  }
  structmsgDispatcherId++;
  structmsgDispatcher->id = structmsgDispatcherId;

  structmsgDispatcher->BMsg = &BMsg;
  structmsgDispatcher->IMsg = &IMsg;
  structmsgDispatcher->MMsg = &MMsg;
  structmsgDispatcher->defaultMsg = &defaultMsg;
  return structmsgDispatcher;
}

// ================================= freeStructmsgDispatcher ====================================

void freeStructmsgDispatcher(structmsgDispatcher_t *structmsgDispatcher) {
}


