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
 * File:   compMsgModuleData.c
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
#include "compMsgDispatcher.h"

static compMsgModuleData_t compMsgModuleData;

// ================================= getModuleTableFieldValue ====================================

static uint8_t getModuleTableFieldValue(compMsgDispatcher_t *self, uint8_t actionMode) {
  uint8_t result;
  
//ets_printf("getModuleTableFieldValue: row: %d col: %d actionMode: %d\n", self->buildMsgInfos.tableRow, self->buildMsgInfos.tableCol, actionMode);
  switch (actionMode) {
  case MODULE_INFO_AP_LIST_CALL_BACK:
    return self->getScanInfoTableFieldValue(self, actionMode);
  default:
    return COMP_DISP_ERR_ACTION_NAME_NOT_FOUND;
  }
  return COMP_DISP_ERR_ACTION_NAME_NOT_FOUND;
}

// ================================= getModuleValue ====================================

static uint8_t getModuleValue(compMsgDispatcher_t *self, uint16_t which, uint8_t valueTypeId, int *numericValue, uint8_t **stringValue) {
  int result;

  *numericValue = 0;
  *stringValue = NULL;
  switch (which) {
  case MODULE_INFO_MACAddr:
    *stringValue = compMsgModuleData.MACAddr;
    break;
  case MODULE_INFO_IPAddr:
    *stringValue = compMsgModuleData.IPAddr;
    break;
  case MODULE_INFO_FirmwareVersion:
    *stringValue = compMsgModuleData.FirmwareVersion;
    break;
  case MODULE_INFO_SerieNumber:
    *stringValue = compMsgModuleData.SerieNumber;
    break;
  case MODULE_INFO_RSSI:
    *numericValue = (int)compMsgModuleData.RSSI;
    break;
  case MODULE_INFO_ModuleConnection:
    *numericValue = (int)compMsgModuleData.ModuleConnection;
    break;
  case MODULE_INFO_DeviceMode:
    *numericValue = (int)compMsgModuleData.DeviceMode;
    break;
  case MODULE_INFO_DeviceSecurity:
    *numericValue = (int)compMsgModuleData.DeviceSecurity;
    break;
  case MODULE_INFO_ErrorMain:
    *numericValue = (int)compMsgModuleData.ErrorMain;
    break;
  case MODULE_INFO_ErrorSub:
    *numericValue = (int)compMsgModuleData.ErrorSub;
    break;
  case MODULE_INFO_DateAndTime:
    *stringValue = compMsgModuleData.DateAndTime;
    break;
  case MODULE_INFO_SSIDs:
    *numericValue = (int)compMsgModuleData.SSIDs;
    break;
  case MODULE_INFO_Reserve1:
    *numericValue = (int)compMsgModuleData.Reserve1;
    break;
  case MODULE_INFO_Reserve2:
    *stringValue = compMsgModuleData.Reserve2;
    break;
  case MODULE_INFO_Reserve3:
    *stringValue = compMsgModuleData.Reserve3;
    break;
  case WIFI_INFO_PROVISIONING_SSID:
  case WIFI_INFO_PROVISIONING_PORT:
  case WIFI_INFO_PROVISIONING_IP_ADDR:
  case WIFI_INFO_BINARY_CALL_BACK:
  case WIFI_INFO_TEXT_CALL_BACK:
    result = self->getWifiValue(self, which, valueTypeId, numericValue, stringValue);
    checkErrOK(result);
    break;
  default:
    return COMP_DISP_ERR_BAD_MODULE_VALUE_WHICH;
    break;
  }
  return COMP_DISP_ERR_OK;
}

// ================================= updateModuleValues ====================================

static uint8_t updateModuleValues(compMsgDispatcher_t *self) {
  int result;

  compMsgModuleData.RSSI = (uint8_t)wifi_station_get_rssi();
  return COMP_DISP_ERR_OK;
}

// ================================= setModuleValues ====================================

static uint8_t setModuleValues(compMsgDispatcher_t *self) {
  int result;

  compMsgModuleData.MACAddr[0] = 0xAB;
  compMsgModuleData.MACAddr[1] = 0xCD;
  compMsgModuleData.MACAddr[2] = 0xEF;
  compMsgModuleData.MACAddr[3] = 0x12;
  compMsgModuleData.MACAddr[4] = 0x34;
  compMsgModuleData.MACAddr[5] = 0x56;
  compMsgModuleData.IPAddr[0] = 0xD4;
  compMsgModuleData.IPAddr[1] = 0xC3;
  compMsgModuleData.IPAddr[2] = 0x12;
  compMsgModuleData.IPAddr[3] = 0x34;
  compMsgModuleData.FirmwareVersion[0] = 0x12;
  compMsgModuleData.FirmwareVersion[1] = 0x34;
  compMsgModuleData.FirmwareVersion[2] = 0x56;
  compMsgModuleData.FirmwareVersion[3] = 0xAB;
  compMsgModuleData.FirmwareVersion[4] = 0xCD;
  compMsgModuleData.FirmwareVersion[5] = 0xEF;
  compMsgModuleData.SerieNumber[0] = 0x02;
  compMsgModuleData.SerieNumber[1] = 0x13;
  compMsgModuleData.SerieNumber[2] = 0x2A;
  compMsgModuleData.SerieNumber[3] = 0x10;
  compMsgModuleData.RSSI = (uint8_t)wifi_station_get_rssi();
  compMsgModuleData.ModuleConnection = 0x41;
  compMsgModuleData.DeviceMode = 0x34;
  compMsgModuleData.DeviceSecurity = 0x00;
  compMsgModuleData.ErrorMain = 0;
  compMsgModuleData.ErrorSub = 0;
  compMsgModuleData.DateAndTime[0] = 0x00;
  compMsgModuleData.DateAndTime[1] = 0x00;
  compMsgModuleData.DateAndTime[2] = 0x00;
  compMsgModuleData.DateAndTime[3] = 0x00;
  compMsgModuleData.DateAndTime[4] = 0x00;
  compMsgModuleData.DateAndTime[5] = 0x00;
  compMsgModuleData.SSIDs = 2;
  compMsgModuleData.Reserve1 = 'X';
  compMsgModuleData.Reserve2[0] = 'X';
  compMsgModuleData.Reserve2[1] = 'Y';
  compMsgModuleData.Reserve3[0] = 'X';
  compMsgModuleData.Reserve3[1] = 'Y';
  compMsgModuleData.Reserve3[2] = 'Z';

  return COMP_DISP_ERR_OK;
}

// ================================= compMsgModuleDataInit ====================================

uint8_t compMsgModuleDataInit(compMsgDispatcher_t *self) {
  uint8_t result;

  self->setModuleValues = &setModuleValues;
  self->updateModuleValues = &updateModuleValues;
  self->getModuleValue = &getModuleValue;
  self->getModuleTableFieldValue = &getModuleTableFieldValue;

  self->setModuleValues(self);
  return COMP_DISP_ERR_OK;
}

