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
 * File:   structmsgDataValues.c
 * Author: Arnulf P. Wiedemann
 *
 * Created on October 7st, 2016
 */

#include "osapi.h"
#include "c_types.h"
#include "mem.h"
#include "flash_fs.h"

#include "c_string.h"
#include "c_stdlib.h"
#include "c_stdio.h"
#include "platform.h"
#include "structmsg2.h"
#include "structmsgModuleData.h"

static structmsgModuleData_t structmsgModuleData;

// ================================= getModuleValue ====================================

static uint8_t getModuleValue(structmsgDispatcher_t *self, uint16_t which, uint8_t valueTypeId, int *numericValue, uint8_t **stringValue) {
  int result;

  *numericValue = 0;
  *stringValue = NULL;
  switch (which) {
  case MODULE_INFO_MACAddr:
    *stringValue = structmsgModuleData.MACAddr;
    break;
  case MODULE_INFO_IPAddr:
    *stringValue = structmsgModuleData.IPAddr;
    break;
  case MODULE_INFO_FirmwareVersion:
    *stringValue = structmsgModuleData.FirmwareVersion;
    break;
  case MODULE_INFO_SerieNumber:
    *stringValue = structmsgModuleData.SerieNumber;
    break;
  case MODULE_INFO_RSSI:
    *numericValue = (int)structmsgModuleData.RSSI;
    break;
  case MODULE_INFO_ModuleConnection:
    *numericValue = (int)structmsgModuleData.ModuleConnection;
    break;
  case MODULE_INFO_DeviceMode:
    *numericValue = (int)structmsgModuleData.DeviceMode;
    break;
  case MODULE_INFO_DeviceSecurity:
    *numericValue = (int)structmsgModuleData.DeviceSecurity;
    break;
  case MODULE_INFO_ErrorMain:
    *numericValue = (int)structmsgModuleData.ErrorMain;
    break;
  case MODULE_INFO_ErrorSub:
    *numericValue = (int)structmsgModuleData.ErrorSub;
    break;
  case MODULE_INFO_DateAndTime:
    *stringValue = structmsgModuleData.DateAndTime;
    break;
  case MODULE_INFO_SSIDs:
    *numericValue = (int)structmsgModuleData.SSIDs;
    break;
  case MODULE_INFO_Reserve1:
    *numericValue = (int)structmsgModuleData.Reserve1;
    break;
  case MODULE_INFO_Reserve2:
    *stringValue = structmsgModuleData.Reserve2;
    break;
  case MODULE_INFO_Reserve3:
    *stringValue = structmsgModuleData.Reserve3;
    break;
  }
  return STRUCT_DISP_ERR_OK;
}


// ================================= updateModuleValues ====================================

static uint8_t updateModuleValues(structmsgDispatcher_t *self) {
  int result;

  return STRUCT_DISP_ERR_OK;
}

// ================================= setModuleValues ====================================

static uint8_t setModuleValues(structmsgDispatcher_t *self) {
  int result;

   structmsgModuleData.MACAddr[0] = 0xAB;
   structmsgModuleData.MACAddr[1] = 0xCD;
   structmsgModuleData.MACAddr[2] = 0xEF;
   structmsgModuleData.MACAddr[3] = 0x12;
   structmsgModuleData.MACAddr[4] = 0x34;
   structmsgModuleData.MACAddr[5] = 0x56;
   structmsgModuleData.IPAddr[0] = 0xD4;
   structmsgModuleData.IPAddr[1] = 0xC3;
   structmsgModuleData.IPAddr[2] = 0x12;
   structmsgModuleData.IPAddr[3] = 0x34;
   structmsgModuleData.FirmwareVersion[0] = 0x12;
   structmsgModuleData.FirmwareVersion[1] = 0x34;
   structmsgModuleData.FirmwareVersion[2] = 0x56;
   structmsgModuleData.FirmwareVersion[3] = 0xAB;
   structmsgModuleData.FirmwareVersion[4] = 0xCD;
   structmsgModuleData.FirmwareVersion[5] = 0xEF;
   structmsgModuleData.SerieNumber[0] = 0x02;
   structmsgModuleData.SerieNumber[1] = 0x13;
   structmsgModuleData.SerieNumber[2] = 0x2A;
   structmsgModuleData.SerieNumber[3] = 0x10;
   structmsgModuleData.RSSI = 0x8A;
   structmsgModuleData.ModuleConnection = 0x41;
   structmsgModuleData.DeviceMode = 0x34;
   structmsgModuleData.DeviceSecurity = 0x00;
   structmsgModuleData.ErrorMain = 0;
   structmsgModuleData.ErrorSub = 0;
   structmsgModuleData.DateAndTime[0] = 0x00;
   structmsgModuleData.DateAndTime[1] = 0x00;
   structmsgModuleData.DateAndTime[2] = 0x00;
   structmsgModuleData.DateAndTime[3] = 0x00;
   structmsgModuleData.DateAndTime[4] = 0x00;
   structmsgModuleData.DateAndTime[5] = 0x00;
   structmsgModuleData.SSIDs = 2;
   structmsgModuleData.Reserve1 = 'X';
   structmsgModuleData.Reserve2[0] = 'X';
   structmsgModuleData.Reserve2[1] = 'Y';
   structmsgModuleData.Reserve3[0] = 'X';
   structmsgModuleData.Reserve3[1] = 'Y';
   structmsgModuleData.Reserve3[2] = 'Z';
  return STRUCT_DISP_ERR_OK;
}

// ================================= structmsgModuleDataValuesInit ====================================

uint8_t structmsgModuleDataValuesInit(structmsgDispatcher_t *self) {
  uint8_t result;

  self->setModuleValues = &setModuleValues;
  self->updateModuleValues = &updateModuleValues;
  self->getModuleValue = &getModuleValue;
  self->setModuleValues(self);
  return STRUCT_DISP_ERR_OK;
}

