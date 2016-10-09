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
 * File:   structmsgModuleData.h
 * Author: Arnulf P. Wiedemann
 *
 * Created on October 9th, 2016
 */

/* struct message data value handling */

#define MODULE_INFO_MACAddr          (1 << 0)
#define MODULE_INFO_IPAddr           (1 << 1)
#define MODULE_INFO_FirmwareVersion  (1 << 2)
#define MODULE_INFO_SerieNumber      (1 << 3)
#define MODULE_INFO_RSSI             (1 << 4)
#define MODULE_INFO_ModuleConnection (1 << 5)
#define MODULE_INFO_DeviceMode       (1 << 6)
#define MODULE_INFO_DeviceSecurity   (1 << 7)
#define MODULE_INFO_ErrorMain        (1 << 8)
#define MODULE_INFO_ErrorSub         (1 << 9)
#define MODULE_INFO_DateAndTime      (1 << 10)
#define MODULE_INFO_SSIDs            (1 << 11)
#define MODULE_INFO_Reserve1         (1 << 12)
#define MODULE_INFO_Reserve2         (1 << 13)
#define MODULE_INFO_Reserve3         (1 << 14)

typedef struct structmsgModuleData {
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
} structmsgModuleData_t;
