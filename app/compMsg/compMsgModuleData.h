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
 * File:   compMsgModuleData.h
 * Author: Arnulf P. Wiedemann
 *
 * Created on October 9th, 2016
 */

/* comp message data value handling */

#ifndef COMP_MSG_MODULE_DATA_H
#define	COMP_MSG_MODULE_DATA_H

// check against compMsgDispatcher.h defines to avoid duplicates!!
#define COMP_DISP_ERR_STATION_SCAN       150
#define COMP_DISP_ERR_GET_STATION_CONFIG 149
#define COMP_DISP_ERR_CANNOT_DISCONNECT  148
#define COMP_DISP_ERR_CANNOT_SET_OPMODE  147
#define COMP_DISP_ERR_REGIST_CONNECT_CB  146
#define COMP_DISP_ERR_TCP_ACCEPT         145
#define COMP_DISP_ERR_REGIST_TIME        144
#define COMP_DISP_ERR_BAD_MODULE_VALUE_WHICH 143
#define COMP_DISP_ERR_BAD_ROW            142

#define MODULE_INFO_MODULE            (1 << 0)

#define MODULE_INFO_MACAddr              1
#define MODULE_INFO_IPAddr               2
#define MODULE_INFO_FirmwareVersion      3
#define MODULE_INFO_SerieNumber          4
#define MODULE_INFO_RSSI                 5
#define MODULE_INFO_ModuleConnection     6
#define MODULE_INFO_DeviceMode           7
#define MODULE_INFO_DeviceSecurity       8
#define MODULE_INFO_ErrorMain            9
#define MODULE_INFO_ErrorSub             10
#define MODULE_INFO_DateAndTime          11
#define MODULE_INFO_SSIDs                12
#define MODULE_INFO_Reserve1             13
#define MODULE_INFO_Reserve2             14
#define MODULE_INFO_Reserve3             15
#define MODULE_INFO_AP_LIST_CALL_BACK    16
#define MODULE_INFO_GUID                 17
#define MODULE_INFO_srcId                18

typedef struct compMsgDispatcher compMsgDispatcher_t;

typedef struct compMsgModuleDataName2Value {
  uint8_t *name;
  uint8_t *value;
} compMsgModuleDataName2Value_t;

typedef struct compMsgModuleData {
  uint8_t MACAddr[6];
  uint8_t IPAddr[4];
  uint8_t FirmwareVersion[6];
  uint8_t SerieNumber[4];
  uint8_t RSSI;
  uint8_t ModuleConnection;
  uint8_t DeviceMode;
  uint8_t DeviceSecurity;
  uint8_t ErrorMain;
  uint8_t ErrorSub;
  uint8_t DateAndTime[6];
  uint8_t SSIDs;
  uint8_t Reserve1;
  uint8_t Reserve2[2];
  uint8_t Reserve3[3];
  uint8_t GUID[16];
  uint16_t srcId;
} compMsgModuleData_t;

#endif	/* COMP_MSG_MODULE_DATA_H */
