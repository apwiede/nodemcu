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
 * File:   compMsgDataValue.h
 * Author: Arnulf P. Wiedemann
 *
 * Created on April 14, 2017
 */

#ifndef COMP_MSG_DATA_VALUE_H
#define	COMP_MSG_DATA_VALUE_H

#include "c_types.h"

#define COMP_MSG_FIELD_IS_STRING    0x01
#define COMP_MSG_FIELD_IS_NUMERIC   0x02
#define COMP_MSG_FIELD_HAS_CALLBACK 0x04

#define COMP_MSG_WIFI_VALUE_ID_provisioningSsid     1
#define COMP_MSG_WIFI_VALUE_ID_provisioningPort     2
#define COMP_MSG_WIFI_VALUE_ID_provisioningIPAddr   3
#define COMP_MSG_WIFI_VALUE_ID_SSDPIPAddr           4
#define COMP_MSG_WIFI_VALUE_ID_SSDPPort             5
#define COMP_MSG_WIFI_VALUE_ID_clientIPAddr         6
#define COMP_MSG_WIFI_VALUE_ID_clientPort           7
#define COMP_MSG_WIFI_VALUE_ID_clientSequenceNum    8
#define COMP_MSG_WIFI_VALUE_ID_binaryCallback       9
#define COMP_MSG_WIFI_VALUE_ID_textCallback         10
#define COMP_MSG_WIFI_VALUE_ID_clientSsid           11
#define COMP_MSG_WIFI_VALUE_ID_clientPasswd         12
#define COMP_MSG_WIFI_VALUE_ID_SSDPReceivedCallback 13
#define COMP_MSG_WIFI_VALUE_ID_SSDPToSendCallback   14
#define COMP_MSG_WIFI_VALUE_ID_NetReceivedCallback  15
#define COMP_MSG_WIFI_VALUE_ID_NetToSendCallback    16
#define COMP_MSG_WIFI_VALUE_ID_NetSecureConnect     17
#define COMP_MSG_WIFI_VALUE_ID_cloudPort            18

#define COMP_MSG_MODULE_VALUE_ID_Reserve1         50
#define COMP_MSG_MODULE_VALUE_ID_Reserve2         51
#define COMP_MSG_MODULE_VALUE_ID_Reserve3         52
#define COMP_MSG_MODULE_VALUE_ID_Reserve4         53
#define COMP_MSG_MODULE_VALUE_ID_Reserve5         54
#define COMP_MSG_MODULE_VALUE_ID_Reserve6         55
#define COMP_MSG_MODULE_VALUE_ID_Reserve7         56
#define COMP_MSG_MODULE_VALUE_ID_Reserve8         57
#define COMP_MSG_MODULE_VALUE_ID_cryptKey         58
#define COMP_MSG_MODULE_VALUE_ID_cryptIvKey       59

typedef struct compMsgDispatcher compMsgDispatcher_t;

typedef uint8_t (* dataValueStr2ValueId_t)(compMsgDispatcher_t *self, uint8_t *valueStr, uint8_t *valueId);
typedef uint8_t (* dataValueId2ValueStr_t)(compMsgDispatcher_t *self, uint8_t valueId, uint8_t **valueStr);
typedef uint8_t (* addDataValue_t)(compMsgDispatcher_t *self, uint8_t fieldNameId, fieldValueCallback_t fieldValueCallback, int numericValue, uint8_t *stringValue);
typedef uint8_t (* setDataValue_t)(compMsgDispatcher_t *self, uint8_t fieldNameId, fieldValueCallback_t fieldValueCallback, int numericValue, uint8_t *stringValue);
typedef uint8_t (* getDataValue_t)(compMsgDispatcher_t *self, uint8_t fieldNameId, uint8_t *flags, fieldValueCallback_t *fieldValueCallback, int *numericValue, uint8_t **stringValue);
typedef uint8_t (* compMsgDataValueInit_t)(compMsgDispatcher_t *self);

typedef struct dataValue {
  uint8_t valueId;
  uint8_t flags;
  union {
    uint8_t *stringValue;
    int numericValue;
  } value;
  fieldValueCallback_t fieldValueCallback;
} dataValue_t;

typedef struct compMsgDataValue {
  int numDataValues;
  int maxDataValues;
  dataValue_t *dataValues;

  dataValueStr2ValueId_t dataValueStr2ValueId;
  dataValueId2ValueStr_t dataValueId2ValueStr;
  addDataValue_t addDataValue;
  setDataValue_t setDataValue;
  getDataValue_t getDataValue;
  compMsgDataValueInit_t compMsgDataValueInit;
} compMsgDataValue_t;

compMsgDataValue_t *newCompMsgDataValue();

#endif  /* COMP_MSG_DATA_VALUE_H */
