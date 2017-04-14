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

typedef struct compMsgDispatcher compMsgDispatcher_t;

typedef uint8_t (* dataValueStr2ValueId_t)(uint8_t *fieldName, str2id_t *ids, uint8_t *fieldId);
typedef uint8_t (* addDataValue_t)(uint8_t fieldNameId, uint8_t flags, fieldValueCallback_t fieldValueCallback, int numericValue, uint8_t *stringValue);
typedef uint8_t (* setDataValue_t)(uint8_t fieldNameId, uint8_t flags, fieldValueCallback_t fieldValueCallback, int numericValue, uint8_t *stringValue);
typedef uint8_t (* getDataValue_t)(uint8_t fieldNameId, uint8_t flags, int *numericValue, uint8_t **stringValue);
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
  addDataValue_t addDataValue;
  setDataValue_t setDataValue;
  getDataValue_t getDataValue;
  compMsgDataValueInit_t compMsgDataValueInit;
} compMsgDataValue_t;

compMsgDataValue_t *newCompMsgDataValue();

#endif  /* COMP_MSG_DATA_VALUE_H */
