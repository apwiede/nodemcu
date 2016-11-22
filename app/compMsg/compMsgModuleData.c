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

#define MODULE_INFO_PASSWDC              24

static str2id_t modeluFieldName2Ids[] = {
  { "MACAddr",                MODULE_INFO_MACAddr },
  { "IPAddr",                 MODULE_INFO_IPAddr },
  { "FirmwareVersion",        MODULE_INFO_FirmwareVersion },
  { "SerieNumber",            MODULE_INFO_SerieNumber },
  { "RSSI",                   MODULE_INFO_RSSI },
  { "ModuleConnection",       MODULE_INFO_ModuleConnection },
  { "DeviceMode",             MODULE_INFO_DeviceMode },
  { "DeviceSecurity",         MODULE_INFO_DeviceSecurity },
  { "ErrorMain",              MODULE_INFO_ErrorMain },
  { "ErrorSub",               MODULE_INFO_ErrorSub },
  { "DateAndTime",            MODULE_INFO_DateAndTime },
  { "SSIDs",                  MODULE_INFO_SSIDs },
  { "Reserve1",               MODULE_INFO_Reserve1 },
  { "Reserve2",               MODULE_INFO_Reserve2 },
  { "Reserve3",               MODULE_INFO_Reserve3 },
  { "Reserve4",               MODULE_INFO_Reserve4 },
  { "Reserve5",               MODULE_INFO_Reserve5 },
  { "Reserve6",               MODULE_INFO_Reserve6 },
  { "Reserve7",               MODULE_INFO_Reserve7 },
  { "Reserve8",               MODULE_INFO_Reserve8 },
  { "GUID",                   MODULE_INFO_GUID },
  { "srcId",                  MODULE_INFO_srcId },
  { "passwdC",                MODULE_INFO_PASSWDC },
  { "operatingMode",          MODULE_INFO_operatingMode },
  {NULL, -1},
};
  
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

static uint8_t getModuleValue(compMsgDispatcher_t *self, uint16_t which, uint8_t valueTypeId) {
  int result;

  switch (which) {
  case MODULE_INFO_MACAddr:
    self->compMsgData->msgValPart->fieldKeyValueStr = compMsgModuleData.MACAddr;
    break;
  case MODULE_INFO_IPAddr:
    self->compMsgData->msgValPart->fieldKeyValueStr = compMsgModuleData.IPAddr;
    break;
  case MODULE_INFO_FirmwareVersion:
    self->compMsgData->msgValPart->fieldKeyValueStr = compMsgModuleData.FirmwareVersion;
    break;
  case MODULE_INFO_SerieNumber:
    self->compMsgData->msgValPart->fieldKeyValueStr = compMsgModuleData.SerieNumber;
    break;
  case MODULE_INFO_RSSI:
    self->compMsgData->msgValPart->fieldFlags |= COMP_DISP_DESC_VALUE_IS_NUMBER;
    self->compMsgData->msgValPart->fieldValue = compMsgModuleData.RSSI;
    break;
  case MODULE_INFO_ModuleConnection:
    self->compMsgData->msgValPart->fieldFlags |= COMP_DISP_DESC_VALUE_IS_NUMBER;
    self->compMsgData->msgValPart->fieldValue = compMsgModuleData.ModuleConnection;
    break;
  case MODULE_INFO_DeviceMode:
    self->compMsgData->msgValPart->fieldFlags |= COMP_DISP_DESC_VALUE_IS_NUMBER;
    self->compMsgData->msgValPart->fieldValue = compMsgModuleData.DeviceMode;
    break;
  case MODULE_INFO_DeviceSecurity:
    self->compMsgData->msgValPart->fieldFlags |= COMP_DISP_DESC_VALUE_IS_NUMBER;
    self->compMsgData->msgValPart->fieldValue = compMsgModuleData.DeviceSecurity;
    break;
  case MODULE_INFO_ErrorMain:
    self->compMsgData->msgValPart->fieldFlags |= COMP_DISP_DESC_VALUE_IS_NUMBER;
    self->compMsgData->msgValPart->fieldValue = compMsgModuleData.ErrorMain;
    break;
  case MODULE_INFO_ErrorSub:
    self->compMsgData->msgValPart->fieldFlags |= COMP_DISP_DESC_VALUE_IS_NUMBER;
    self->compMsgData->msgValPart->fieldValue = compMsgModuleData.ErrorSub;
    break;
  case MODULE_INFO_DateAndTime:
    self->compMsgData->msgValPart->fieldKeyValueStr = compMsgModuleData.DateAndTime;
    break;
  case MODULE_INFO_SSIDs:
    self->compMsgData->msgValPart->fieldFlags |= COMP_DISP_DESC_VALUE_IS_NUMBER;
    self->compMsgData->msgValPart->fieldValue = compMsgModuleData.SSIDs;
    break;
  case MODULE_INFO_Reserve1:
    self->compMsgData->msgValPart->fieldFlags |= COMP_DISP_DESC_VALUE_IS_NUMBER;
    self->compMsgData->msgValPart->fieldValue = (int)compMsgModuleData.Reserve1;
    break;
  case MODULE_INFO_Reserve2:
    self->compMsgData->msgValPart->fieldKeyValueStr = compMsgModuleData.Reserve2;
    break;
  case MODULE_INFO_Reserve3:
    self->compMsgData->msgValPart->fieldKeyValueStr = compMsgModuleData.Reserve3;
    break;
  case MODULE_INFO_Reserve4:
    self->compMsgData->msgValPart->fieldKeyValueStr = compMsgModuleData.Reserve4;
    break;
  case MODULE_INFO_Reserve5:
    self->compMsgData->msgValPart->fieldKeyValueStr = compMsgModuleData.Reserve5;
    break;
  case MODULE_INFO_Reserve6:
    self->compMsgData->msgValPart->fieldKeyValueStr = compMsgModuleData.Reserve6;
    break;
  case MODULE_INFO_Reserve7:
    self->compMsgData->msgValPart->fieldKeyValueStr = compMsgModuleData.Reserve7;
    break;
  case MODULE_INFO_Reserve8:
    self->compMsgData->msgValPart->fieldKeyValueStr = compMsgModuleData.Reserve8;
    break;
  case MODULE_INFO_GUID:
    self->compMsgData->msgValPart->fieldKeyValueStr = compMsgModuleData.GUID;
    break;
  case MODULE_INFO_srcId:
    self->compMsgData->msgValPart->fieldFlags |= COMP_DISP_DESC_VALUE_IS_NUMBER;
    self->compMsgData->msgValPart->fieldValue = compMsgModuleData.srcId;
    break;
  case MODULE_INFO_PASSWDC:
    self->compMsgData->msgValPart->fieldKeyValueStr = compMsgModuleData.passwdC;
    break;
  case MODULE_INFO_operatingMode:
    self->compMsgData->msgValPart->fieldFlags |= COMP_DISP_DESC_VALUE_IS_NUMBER;
    self->compMsgData->msgValPart->fieldValue = compMsgModuleData.operatingMode;
    break;
  default:
    return COMP_DISP_ERR_BAD_MODULE_VALUE_WHICH;
    break;
  }
  return COMP_DISP_ERR_OK;
}

// ================================= moduleFieldName2Id ====================================

static uint8_t moduleFieldName2Id(compMsgDispatcher_t *self, uint8_t *fieldNameStr, uint8_t *id) {
  int result;

  return COMP_DISP_ERR_OK;
}

// ================================= setModuleValue ====================================

static uint8_t setModuleValue(compMsgDispatcher_t *self, uint8_t *fieldNameStr, int numericValue, uint8_t *stringValue) {
  int result;
  uint8_t id;

  result = moduleFieldName2Id(self, fieldNameStr, &id);
  checkErrOK(result);
  switch (id) {
  case MODULE_INFO_MACAddr:
    c_memcpy(compMsgModuleData.MACAddr, stringValue, c_strlen(stringValue));
    break;
  case MODULE_INFO_IPAddr:
    c_memcpy(compMsgModuleData.IPAddr, stringValue, c_strlen(stringValue));
    break;
  case MODULE_INFO_FirmwareVersion:
    c_memcpy(compMsgModuleData.FirmwareVersion, stringValue, c_strlen(stringValue));
    break;
  case MODULE_INFO_SerieNumber:
    c_memcpy(compMsgModuleData.SerieNumber, stringValue, c_strlen(stringValue));
    break;
  case MODULE_INFO_RSSI:
    compMsgModuleData.RSSI = numericValue;
    break;
  case MODULE_INFO_ModuleConnection:
    compMsgModuleData.ModuleConnection = numericValue;
    break;
  case MODULE_INFO_DeviceMode:
    compMsgModuleData.DeviceMode = numericValue;
    break;
  case MODULE_INFO_DeviceSecurity:
    compMsgModuleData.DeviceSecurity = numericValue;
    break;
  case MODULE_INFO_ErrorMain:
    compMsgModuleData.ErrorMain = numericValue;
    break;
  case MODULE_INFO_ErrorSub:
    compMsgModuleData.ErrorSub = numericValue;
    break;
  case MODULE_INFO_DateAndTime:
    c_memcpy(compMsgModuleData.DateAndTime, stringValue, c_strlen(stringValue));
    break;
  case MODULE_INFO_SSIDs:
    compMsgModuleData.SSIDs = numericValue;
    break;
  case MODULE_INFO_Reserve1:
    compMsgModuleData.Reserve1 = numericValue;
    break;
  case MODULE_INFO_Reserve2:
    c_memcpy(compMsgModuleData.Reserve2, stringValue, c_strlen(stringValue));
    break;
  case MODULE_INFO_Reserve3:
    c_memcpy(compMsgModuleData.Reserve3, stringValue, c_strlen(stringValue));
    break;
  case MODULE_INFO_Reserve4:
    c_memcpy(compMsgModuleData.Reserve4, stringValue, c_strlen(stringValue));
    break;
  case MODULE_INFO_Reserve5:
    c_memcpy(compMsgModuleData.Reserve5, stringValue, c_strlen(stringValue));
    break;
  case MODULE_INFO_Reserve6:
    c_memcpy(compMsgModuleData.Reserve6, stringValue, c_strlen(stringValue));
    break;
  case MODULE_INFO_Reserve7:
    c_memcpy(compMsgModuleData.Reserve7, stringValue, c_strlen(stringValue));
    break;
  case MODULE_INFO_Reserve8:
    c_memcpy(compMsgModuleData.Reserve8, stringValue, c_strlen(stringValue));
    break;
  case MODULE_INFO_GUID:
    c_memcpy(compMsgModuleData.GUID, stringValue, c_strlen(stringValue));
    break;
  case MODULE_INFO_srcId:
    compMsgModuleData.srcId = numericValue;
    break;
  case MODULE_INFO_PASSWDC:
    c_memcpy(compMsgModuleData.passwdC, stringValue, c_strlen(stringValue));
    break;
  case MODULE_INFO_operatingMode:
    compMsgModuleData.operatingMode = numericValue;
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
  c_memcpy(compMsgModuleData.Reserve2, "XY", 2);
  c_memcpy(compMsgModuleData.Reserve3, "XYZ", 3);
  c_memcpy(compMsgModuleData.Reserve4, "ABCD", 4);
  c_memcpy(compMsgModuleData.Reserve5, "ABCDE", 5);
  c_memcpy(compMsgModuleData.Reserve6, "ABCDEF", 6);
  c_memcpy(compMsgModuleData.Reserve7, "ABCDEFG", 7);
  c_memcpy(compMsgModuleData.Reserve8, "ABCDEFGH", 8);
  c_memcpy(compMsgModuleData.GUID, "1234-5678-9012-1", 16);
  compMsgModuleData.srcId = 12312;
  c_memcpy(compMsgModuleData.passwdC, "apwiede1apwiede2", 16);
  compMsgModuleData.operatingMode = 2;

  return COMP_DISP_ERR_OK;
}

// ================================= compMsgModuleDataInit ====================================

uint8_t compMsgModuleDataInit(compMsgDispatcher_t *self) {
  uint8_t result;

  self->getModuleValue = &getModuleValue;
  self->setModuleValue = &setModuleValue;
  self->setModuleValues = &setModuleValues;
  self->updateModuleValues = &updateModuleValues;
  self->getModuleTableFieldValue = &getModuleTableFieldValue;

  self->setModuleValues(self);
  return COMP_DISP_ERR_OK;
}

