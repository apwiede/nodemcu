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

static str2id_t modeluFieldName2Ids[] = {
  { "MACAddr",                MODULE_INFO_MACAddr },
  { "IPAddr",                 MODULE_INFO_IPAddr },
  { "FirmwareVersion",        MODULE_INFO_FirmwareVersion },
  { "SerieNumber",            MODULE_INFO_SerieNumber },
  { "RSSI",                   MODULE_INFO_RSSI },
  { "RSSIMax",                MODULE_INFO_RSSIMax },
  { "ConnectionState",        MODULE_INFO_ConnectionState },
  { "ConnectedUsers",         MODULE_INFO_ConnectedUsers },
  { "ProgRunningMode",        MODULE_INFO_ProgRunningMode },
  { "CurrentRunningMode",     MODULE_INFO_CurrentRunningMode },
  { "IPProtocol",             MODULE_INFO_IPProtocol },
  { "Region",                 MODULE_INFO_Region },
  { "DeviceSecurity",         MODULE_INFO_DeviceSecurity },
  { "ErrorMain",              MODULE_INFO_ErrorMain },
  { "ErrorSub",               MODULE_INFO_ErrorSub },
  { "DateAndTime",            MODULE_INFO_DateAndTime },
  { "SSIDs",                  MODULE_INFO_SSIDs },
  { "PingState",              MODULE_INFO_PingState },
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
  
// ================================= getMACAddr ====================================

static uint8_t getMACAddr(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
  int result;

  self->compMsgData->msgValPart->fieldKeyValueStr = compMsgModuleData.MACAddr;
  *numericValue = 0;
  *stringValue = compMsgModuleData.MACAddr;
  return COMP_MSG_ERR_OK;
}

// ================================= getIPAddr ====================================

static uint8_t getIPAddr(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
  self->compMsgData->msgValPart->fieldKeyValueStr = compMsgModuleData.IPAddr;
  *numericValue = 0;
  *stringValue = compMsgModuleData.IPAddr;
  return COMP_MSG_ERR_OK;
}

// ================================= getFirmwareVersion ====================================

static uint8_t getFirmwareVersion(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
  self->compMsgData->msgValPart->fieldKeyValueStr = compMsgModuleData.FirmwareVersion;
  *numericValue = 0;
  *stringValue = compMsgModuleData.FirmwareVersion;
  return COMP_MSG_ERR_OK;
}

// ================================= getSerieNumber ====================================

static uint8_t getSerieNumber(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
  self->compMsgData->msgValPart->fieldKeyValueStr = compMsgModuleData.SerieNumber;
  *numericValue = 0;
  *stringValue = compMsgModuleData.SerieNumber;
  return COMP_MSG_ERR_OK;
}

// ================================= getRSSI ====================================

static uint8_t getRSSI(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
  self->compMsgData->msgValPart->fieldFlags |= COMP_DISP_DESC_VALUE_IS_NUMBER;
  self->compMsgData->msgValPart->fieldValue = compMsgModuleData.RSSI;
  *numericValue = compMsgModuleData.RSSI;
  *stringValue = NULL;
  return COMP_MSG_ERR_OK;
}

// ================================= getRSSIMax ====================================

static uint8_t getRSSIMax(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
  self->compMsgData->msgValPart->fieldFlags |= COMP_DISP_DESC_VALUE_IS_NUMBER;
  self->compMsgData->msgValPart->fieldValue = compMsgModuleData.RSSIMax;
  *numericValue = compMsgModuleData.RSSIMax;
  *stringValue = NULL;
  return COMP_MSG_ERR_OK;
}

// ================================= getConnectionState ====================================

static uint8_t getConnectionState(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
  self->compMsgData->msgValPart->fieldFlags |= COMP_DISP_DESC_VALUE_IS_NUMBER;
  self->compMsgData->msgValPart->fieldValue = compMsgModuleData.ConnectionState;
  *numericValue = compMsgModuleData.ConnectionState;
  *stringValue = NULL;
  return COMP_MSG_ERR_OK;
}

// ================================= getConnectedUsers ====================================

static uint8_t getConnectedUsers(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
  self->compMsgData->msgValPart->fieldFlags |= COMP_DISP_DESC_VALUE_IS_NUMBER;
  self->compMsgData->msgValPart->fieldValue = compMsgModuleData.ConnectedUsers;
  *numericValue = compMsgModuleData.ConnectedUsers;
  *stringValue = NULL;
  return COMP_MSG_ERR_OK;
}

// ================================= getProgRunningMode ====================================

static uint8_t getProgRunningMode(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
  self->compMsgData->msgValPart->fieldFlags |= COMP_DISP_DESC_VALUE_IS_NUMBER;
  self->compMsgData->msgValPart->fieldValue = compMsgModuleData.ProgRunningMode;
  *numericValue = compMsgModuleData.ProgRunningMode;
  *stringValue = NULL;
  return COMP_MSG_ERR_OK;
}

// ================================= CurrentProgRunningMode ====================================

static uint8_t getCurrentRunningMode(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
  self->compMsgData->msgValPart->fieldFlags |= COMP_DISP_DESC_VALUE_IS_NUMBER;
  self->compMsgData->msgValPart->fieldValue = compMsgModuleData.CurrentRunningMode;
  *numericValue = compMsgModuleData.CurrentRunningMode;
  *stringValue = NULL;
  return COMP_MSG_ERR_OK;
}

// ================================= getIPProtocol ====================================

static uint8_t getIPProtocol(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
  self->compMsgData->msgValPart->fieldFlags |= COMP_DISP_DESC_VALUE_IS_NUMBER;
  self->compMsgData->msgValPart->fieldValue = compMsgModuleData.IPProtocol;
  *numericValue = compMsgModuleData.IPProtocol;
  *stringValue = NULL;
  return COMP_MSG_ERR_OK;
}

// ================================= getRegion ====================================

static uint8_t getRegion(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
  self->compMsgData->msgValPart->fieldFlags |= COMP_DISP_DESC_VALUE_IS_NUMBER;
  self->compMsgData->msgValPart->fieldValue = compMsgModuleData.Region;
  *numericValue = compMsgModuleData.Region;
  *stringValue = NULL;
  return COMP_MSG_ERR_OK;
}

// ================================= getDeviceSecurity ====================================

static uint8_t getDeviceSecurity(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
  self->compMsgData->msgValPart->fieldFlags |= COMP_DISP_DESC_VALUE_IS_NUMBER;
  self->compMsgData->msgValPart->fieldValue = compMsgModuleData.DeviceSecurity;
  *numericValue = compMsgModuleData.DeviceSecurity;
  *stringValue = NULL;
  return COMP_MSG_ERR_OK;
}

// ================================= getErrorMain ====================================

static uint8_t getErrorMain(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
  self->compMsgData->msgValPart->fieldFlags |= COMP_DISP_DESC_VALUE_IS_NUMBER;
  self->compMsgData->msgValPart->fieldValue = compMsgModuleData.ErrorMain;
  *numericValue = compMsgModuleData.ErrorMain;
  *stringValue = NULL;
  return COMP_MSG_ERR_OK;
}

// ================================= getErrorSub ====================================

static uint8_t getErrorSub(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
  self->compMsgData->msgValPart->fieldFlags |= COMP_DISP_DESC_VALUE_IS_NUMBER;
  self->compMsgData->msgValPart->fieldValue = compMsgModuleData.ErrorSub;
  *numericValue = compMsgModuleData.ErrorSub;
  *stringValue = NULL;
  return COMP_MSG_ERR_OK;
}

// ================================= getDateAndTime ====================================

static uint8_t getDateAndTime(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
  self->compMsgData->msgValPart->fieldKeyValueStr = compMsgModuleData.DateAndTime;
  *numericValue = 0;
  *stringValue = compMsgModuleData.DateAndTime;
  return COMP_MSG_ERR_OK;
}

// ================================= getSSIDs ====================================

static uint8_t getSSIDs(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
  self->compMsgData->msgValPart->fieldFlags |= COMP_DISP_DESC_VALUE_IS_NUMBER;
  self->compMsgData->msgValPart->fieldValue = compMsgModuleData.SSIDs;
  *numericValue = compMsgModuleData.SSIDs;
  *stringValue = NULL;
  return COMP_MSG_ERR_OK;
}

// ================================= getPingState ====================================

static uint8_t getPingState(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
  self->compMsgData->msgValPart->fieldFlags |= COMP_DISP_DESC_VALUE_IS_NUMBER;
  self->compMsgData->msgValPart->fieldValue = compMsgModuleData.PingState;
  *numericValue = compMsgModuleData.PingState;
  *stringValue = NULL;
  return COMP_MSG_ERR_OK;
}

// ================================= getReserve1 ====================================

static uint8_t getReserve1(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
  self->compMsgData->msgValPart->fieldFlags |= COMP_DISP_DESC_VALUE_IS_NUMBER;
  self->compMsgData->msgValPart->fieldValue = (int)compMsgModuleData.Reserve1;
  *numericValue = (int)compMsgModuleData.Reserve1;
  *stringValue = NULL;
  return COMP_MSG_ERR_OK;
}

// ================================= getReserve2 ====================================

static uint8_t getReserve2(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
  self->compMsgData->msgValPart->fieldKeyValueStr = compMsgModuleData.Reserve2;
  *numericValue = 0;
  *stringValue = compMsgModuleData.Reserve2;
  return COMP_MSG_ERR_OK;
}

// ================================= getReserve3 ====================================

static uint8_t getReserve3(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
  self->compMsgData->msgValPart->fieldKeyValueStr = compMsgModuleData.Reserve3;
  *numericValue = 0;
  *stringValue = compMsgModuleData.Reserve3;
  return COMP_MSG_ERR_OK;
}

// ================================= getReserve4 ====================================

static uint8_t getReserve4(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
  self->compMsgData->msgValPart->fieldKeyValueStr = compMsgModuleData.Reserve4;
  *numericValue = 0;
  *stringValue = compMsgModuleData.Reserve4;
  return COMP_MSG_ERR_OK;
}

// ================================= getReserve5 ====================================

static uint8_t getReserve5(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
  self->compMsgData->msgValPart->fieldKeyValueStr = compMsgModuleData.Reserve5;
  *numericValue = 0;
  *stringValue = compMsgModuleData.Reserve5;
  return COMP_MSG_ERR_OK;
}

// ================================= getReserve6 ====================================

static uint8_t getReserve6(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
  self->compMsgData->msgValPart->fieldKeyValueStr = compMsgModuleData.Reserve6;
  *numericValue = 0;
  *stringValue = compMsgModuleData.Reserve6;
  return COMP_MSG_ERR_OK;
}

// ================================= getReserve7 ====================================

static uint8_t getReserve7(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
  self->compMsgData->msgValPart->fieldKeyValueStr = compMsgModuleData.Reserve7;
  *numericValue = 0;
  *stringValue = compMsgModuleData.Reserve7;
  return COMP_MSG_ERR_OK;
}

// ================================= getReserve8 ====================================

static uint8_t getReserve8(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
  self->compMsgData->msgValPart->fieldKeyValueStr = compMsgModuleData.Reserve8;
  *numericValue = 0;
  *stringValue = compMsgModuleData.Reserve8;
  return COMP_MSG_ERR_OK;
}

// ================================= getGUID ====================================

static uint8_t getGUID(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
  self->compMsgData->msgValPart->fieldKeyValueStr = compMsgModuleData.GUID;
  *numericValue = 0;
  *stringValue = compMsgModuleData.GUID;
  return COMP_MSG_ERR_OK;
}

// ================================= getPasswdC ====================================

static uint8_t getPasswdC(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
  self->compMsgData->msgValPart->fieldKeyValueStr = compMsgModuleData.passwdC;
  *numericValue = 0;
  *stringValue = compMsgModuleData.passwdC;
  return COMP_MSG_ERR_OK;
}

// ================================= getSrcId ====================================

static uint8_t getSrcId(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
  self->compMsgData->msgValPart->fieldFlags |= COMP_DISP_DESC_VALUE_IS_NUMBER;
  self->compMsgData->msgValPart->fieldValue = compMsgModuleData.srcId;
  *numericValue = compMsgModuleData.srcId;
  *stringValue = NULL;
  return COMP_MSG_ERR_OK;
}

// ================================= getOperatingMode ====================================

static uint8_t getOperatingMode(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
  self->compMsgData->msgValPart->fieldFlags |= COMP_DISP_DESC_VALUE_IS_NUMBER;
  self->compMsgData->msgValPart->fieldValue = compMsgModuleData.operatingMode;
  *numericValue = compMsgModuleData.operatingMode;
  *stringValue = NULL;
  return COMP_MSG_ERR_OK;
}

// ================================= setOperatingMode ====================================

static uint8_t setOperatingMode(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue) {
  int result;

  result = self->compMsgModuleData->setModuleValue(self, "operatingMode", self->operatingMode, NULL);
  return COMP_MSG_ERR_OK;
}

// ================================= moduleFieldName2Id ====================================

static uint8_t moduleFieldName2Id(compMsgDispatcher_t *self, uint8_t *fieldNameStr, uint8_t *id) {
  int result;

  return COMP_MSG_ERR_OK;
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
    compMsgModuleData.MACAddr[c_strlen(stringValue)] = '\0';
    break;
  case MODULE_INFO_IPAddr:
    c_memcpy(compMsgModuleData.IPAddr, stringValue, c_strlen(stringValue));
    compMsgModuleData.IPAddr[c_strlen(stringValue)] = '\0';
    break;
  case MODULE_INFO_FirmwareVersion:
    c_memcpy(compMsgModuleData.FirmwareVersion, stringValue, c_strlen(stringValue));
    compMsgModuleData.FirmwareVersion[c_strlen(stringValue)] = '\0';
    break;
  case MODULE_INFO_SerieNumber:
    c_memcpy(compMsgModuleData.SerieNumber, stringValue, c_strlen(stringValue));
    compMsgModuleData.SerieNumber[c_strlen(stringValue)] = '\0';
    break;
  case MODULE_INFO_RSSI:
    compMsgModuleData.RSSI = numericValue;
    break;
  case MODULE_INFO_RSSIMax:
    compMsgModuleData.RSSIMax = numericValue;
    break;
  case MODULE_INFO_ConnectionState:
    compMsgModuleData.ConnectionState = numericValue;
    break;
  case MODULE_INFO_ConnectedUsers:
    compMsgModuleData.ConnectedUsers = numericValue;
    break;
  case MODULE_INFO_ProgRunningMode:
    compMsgModuleData.ProgRunningMode = numericValue;
    break;
  case MODULE_INFO_CurrentRunningMode:
    compMsgModuleData.CurrentRunningMode = numericValue;
    break;
  case MODULE_INFO_IPProtocol:
    compMsgModuleData.IPProtocol = numericValue;
    break;
  case MODULE_INFO_Region:
    compMsgModuleData.Region = numericValue;
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
    compMsgModuleData.DateAndTime[c_strlen(stringValue)] = '\0';
    break;
  case MODULE_INFO_SSIDs:
    compMsgModuleData.SSIDs = numericValue;
    break;
  case MODULE_INFO_PingState:
    compMsgModuleData.PingState = numericValue;
    break;
  case MODULE_INFO_Reserve1:
    compMsgModuleData.Reserve1 = numericValue;
    break;
  case MODULE_INFO_Reserve2:
    c_memcpy(compMsgModuleData.Reserve2, stringValue, c_strlen(stringValue));
    compMsgModuleData.Reserve2[c_strlen(stringValue)] = '\0';
    break;
  case MODULE_INFO_Reserve3:
    c_memcpy(compMsgModuleData.Reserve3, stringValue, c_strlen(stringValue));
    compMsgModuleData.Reserve3[c_strlen(stringValue)] = '\0';
    break;
  case MODULE_INFO_Reserve4:
    c_memcpy(compMsgModuleData.Reserve4, stringValue, c_strlen(stringValue));
    compMsgModuleData.Reserve4[c_strlen(stringValue)] = '\0';
    break;
  case MODULE_INFO_Reserve5:
    c_memcpy(compMsgModuleData.Reserve5, stringValue, c_strlen(stringValue));
    compMsgModuleData.Reserve5[c_strlen(stringValue)] = '\0';
    break;
  case MODULE_INFO_Reserve6:
    c_memcpy(compMsgModuleData.Reserve6, stringValue, c_strlen(stringValue));
    compMsgModuleData.Reserve6[c_strlen(stringValue)] = '\0';
    break;
  case MODULE_INFO_Reserve7:
    c_memcpy(compMsgModuleData.Reserve7, stringValue, c_strlen(stringValue));
    compMsgModuleData.Reserve7[c_strlen(stringValue)] = '\0';
    break;
  case MODULE_INFO_Reserve8:
    c_memcpy(compMsgModuleData.Reserve8, stringValue, c_strlen(stringValue));
    compMsgModuleData.Reserve8[c_strlen(stringValue)] = '\0';
    break;
  case MODULE_INFO_GUID:
    c_memcpy(compMsgModuleData.GUID, stringValue, c_strlen(stringValue));
    compMsgModuleData.GUID[c_strlen(stringValue)] = '\0';
    break;
  case MODULE_INFO_srcId:
    compMsgModuleData.srcId = numericValue;
    break;
  case MODULE_INFO_PASSWDC:
    c_memcpy(compMsgModuleData.passwdC, stringValue, c_strlen(stringValue));
    compMsgModuleData.passwdC[c_strlen(stringValue)] = '\0';
    break;
  case MODULE_INFO_operatingMode:
    compMsgModuleData.operatingMode = numericValue;
    break;
  default:
    return COMP_MSG_ERR_BAD_MODULE_VALUE_WHICH;
    break;
  }
  return COMP_MSG_ERR_OK;
}

// ================================= updateModuleValues ====================================

static uint8_t updateModuleValues(compMsgDispatcher_t *self) {
  int result;

  compMsgModuleData.RSSI = (uint8_t)wifi_station_get_rssi();
  return COMP_MSG_ERR_OK;
}

// ================================= setModuleValues ====================================

static uint8_t setModuleValues(compMsgDispatcher_t *self) {
  int result;

//ets_printf("§setModuleValues\n§");
  compMsgModuleData.MACAddr[0] = 0xAB;
  compMsgModuleData.MACAddr[1] = 0xCD;
  compMsgModuleData.MACAddr[2] = 0xEF;
  compMsgModuleData.MACAddr[3] = 0x12;
  compMsgModuleData.MACAddr[4] = 0x34;
  compMsgModuleData.MACAddr[5] = 0x56;
  compMsgModuleData.MACAddr[6] = 0;
  compMsgModuleData.IPAddr[0] = 0xD4;
  compMsgModuleData.IPAddr[1] = 0xC3;
  compMsgModuleData.IPAddr[2] = 0x12;
  compMsgModuleData.IPAddr[3] = 0x34;
  compMsgModuleData.IPAddr[4] = 0;
  compMsgModuleData.FirmwareVersion[0] = 0x12;
  compMsgModuleData.FirmwareVersion[1] = 0x34;
  compMsgModuleData.FirmwareVersion[2] = 0x56;
  compMsgModuleData.FirmwareVersion[3] = 0xAB;
  compMsgModuleData.FirmwareVersion[4] = 0xCD;
  compMsgModuleData.FirmwareVersion[5] = 0xEF;
  compMsgModuleData.FirmwareVersion[6] = 0;
  compMsgModuleData.SerieNumber[0] = 0x02;
  compMsgModuleData.SerieNumber[1] = 0x13;
  compMsgModuleData.SerieNumber[2] = 0x2A;
  compMsgModuleData.SerieNumber[3] = 0x10;
  compMsgModuleData.SerieNumber[4] = 0;
  compMsgModuleData.RSSI = (uint8_t)wifi_station_get_rssi();
  compMsgModuleData.RSSIMax = 5;
  compMsgModuleData.ConnectionState = 5;
  compMsgModuleData.ConnectedUsers = 1;
  compMsgModuleData.ProgRunningMode = 0;
  compMsgModuleData.CurrentRunningMode = 0;
  compMsgModuleData.IPProtocol = 1;
  compMsgModuleData.Region = 0;
  compMsgModuleData.DeviceSecurity = 0x00;
  compMsgModuleData.ErrorMain = 0;
  compMsgModuleData.ErrorSub = 0;
  compMsgModuleData.DateAndTime[0] = 0x00;
  compMsgModuleData.DateAndTime[1] = 0x00;
  compMsgModuleData.DateAndTime[2] = 0x00;
  compMsgModuleData.DateAndTime[3] = 0x00;
  compMsgModuleData.DateAndTime[4] = 0x00;
  compMsgModuleData.DateAndTime[5] = 0x00;
  compMsgModuleData.DateAndTime[6] = 0;
  compMsgModuleData.SSIDs = 2;
  compMsgModuleData.PingState = 1;
  compMsgModuleData.Reserve1 = 'X';
  c_memcpy(compMsgModuleData.Reserve2, "XY\0", 3);
  c_memcpy(compMsgModuleData.Reserve3, "XYZ\0", 4);
  c_memcpy(compMsgModuleData.Reserve4, "ABCD\0", 5);
  c_memcpy(compMsgModuleData.Reserve5, "ABCDE\0", 6);
  c_memcpy(compMsgModuleData.Reserve6, "ABCDEF\0", 7);
  c_memcpy(compMsgModuleData.Reserve7, "ABCDEFG\0", 8);
  c_memcpy(compMsgModuleData.Reserve8, "ABCDEFGH\0", 9);
  c_memcpy(compMsgModuleData.GUID, "1234-5678-9012-1\0", 17);
  compMsgModuleData.srcId = 12312;
  c_memcpy(compMsgModuleData.passwdC, "apwiede1apwiede2\0", 17);
  compMsgModuleData.operatingMode = 0xE0;
//ets_printf("§setModuleVaues done\n§");
  return COMP_MSG_ERR_OK;
}

// ================================= compMsgModuleDataInit ====================================

uint8_t compMsgModuleDataInit(compMsgDispatcher_t *self) {
  uint8_t result;

  self->compMsgModuleData->setModuleValue = &setModuleValue;
  self->compMsgModuleData->setModuleValues = &setModuleValues;
  self->compMsgModuleData->updateModuleValues = &updateModuleValues;

  self->addFieldValueCallbackName(self, "@getMACAddr", &getMACAddr, COMP_DISP_CALLBACK_TYPE_MODULE);
  self->addFieldValueCallbackName(self, "@getIPAddr", &getIPAddr, COMP_DISP_CALLBACK_TYPE_MODULE);
  self->addFieldValueCallbackName(self, "@getFirmwareVersion", &getFirmwareVersion, COMP_DISP_CALLBACK_TYPE_MODULE);
  self->addFieldValueCallbackName(self, "@getSerieNumber", &getSerieNumber, COMP_DISP_CALLBACK_TYPE_MODULE);
  self->addFieldValueCallbackName(self, "@getRSSI", &getRSSI, COMP_DISP_CALLBACK_TYPE_MODULE);
  self->addFieldValueCallbackName(self, "@getRSSIMax", &getRSSIMax, COMP_DISP_CALLBACK_TYPE_MODULE);
  self->addFieldValueCallbackName(self, "@getConnectionState", &getConnectionState, COMP_DISP_CALLBACK_TYPE_MODULE);
  self->addFieldValueCallbackName(self, "@getConnectedUsers", &getConnectedUsers, COMP_DISP_CALLBACK_TYPE_MODULE);
  self->addFieldValueCallbackName(self, "@getProgRunningMode", &getProgRunningMode, COMP_DISP_CALLBACK_TYPE_MODULE);
  self->addFieldValueCallbackName(self, "@getCurrentRunningMode", &getCurrentRunningMode, COMP_DISP_CALLBACK_TYPE_MODULE);
  self->addFieldValueCallbackName(self, "@getIPProtocol", &getIPProtocol, COMP_DISP_CALLBACK_TYPE_MODULE);
  self->addFieldValueCallbackName(self, "@getRegion", &getRegion, COMP_DISP_CALLBACK_TYPE_MODULE);
  self->addFieldValueCallbackName(self, "@getDeviceSecurity", &getDeviceSecurity, COMP_DISP_CALLBACK_TYPE_MODULE);
  self->addFieldValueCallbackName(self, "@getErrorMain", &getErrorMain, COMP_DISP_CALLBACK_TYPE_MODULE);
  self->addFieldValueCallbackName(self, "@getErrorSub", &getErrorSub, COMP_DISP_CALLBACK_TYPE_MODULE);
  self->addFieldValueCallbackName(self, "@getDateAndTime", &getDateAndTime, COMP_DISP_CALLBACK_TYPE_MODULE);
  self->addFieldValueCallbackName(self, "@getSSIDs", &getSSIDs, COMP_DISP_CALLBACK_TYPE_MODULE);
  self->addFieldValueCallbackName(self, "@getPingState", &getPingState, COMP_DISP_CALLBACK_TYPE_MODULE);
  self->addFieldValueCallbackName(self, "@getReserve1", &getReserve1, COMP_DISP_CALLBACK_TYPE_MODULE);
  self->addFieldValueCallbackName(self, "@getReserve2", &getReserve2, COMP_DISP_CALLBACK_TYPE_MODULE);
  self->addFieldValueCallbackName(self, "@getReserve3", &getReserve3, COMP_DISP_CALLBACK_TYPE_MODULE);
  self->addFieldValueCallbackName(self, "@getReserve4", &getReserve4, COMP_DISP_CALLBACK_TYPE_MODULE);
  self->addFieldValueCallbackName(self, "@getReserve5", &getReserve5, COMP_DISP_CALLBACK_TYPE_MODULE);
  self->addFieldValueCallbackName(self, "@getReserve6", &getReserve6, COMP_DISP_CALLBACK_TYPE_MODULE);
  self->addFieldValueCallbackName(self, "@getReserve7", &getReserve7, COMP_DISP_CALLBACK_TYPE_MODULE);
  self->addFieldValueCallbackName(self, "@getReserve8", &getReserve8, COMP_DISP_CALLBACK_TYPE_MODULE);
  self->addFieldValueCallbackName(self, "@getGUID", &getGUID, COMP_DISP_CALLBACK_TYPE_MODULE);
  self->addFieldValueCallbackName(self, "@getSrcId", &getSrcId, COMP_DISP_CALLBACK_TYPE_MODULE);
  self->addFieldValueCallbackName(self, "@getPasswdC", &getPasswdC, COMP_DISP_CALLBACK_TYPE_MODULE);
  self->addFieldValueCallbackName(self, "@setOperatingMode", &setOperatingMode, COMP_DISP_CALLBACK_TYPE_MODULE);
  self->addFieldValueCallbackName(self, "@getOperatingMode", &getOperatingMode, COMP_DISP_CALLBACK_TYPE_MODULE);
  self->compMsgModuleData->setModuleValues(self);
  return COMP_MSG_ERR_OK;
}

// ================================= newCompMsgModuleData ====================================

compMsgModuleData_t *newCompMsgModuleData() {
  compMsgModuleData_t *compMsgModuleData = os_zalloc(sizeof(compMsgModuleData_t));
  if (compMsgModuleData == NULL) {
    return NULL;
  }

  return compMsgModuleData;
}
