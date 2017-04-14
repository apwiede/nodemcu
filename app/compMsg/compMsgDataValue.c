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
 * File:   compMsgDataValue.c
 * Author: Arnulf P. Wiedemann
 *
 * Created on April 14th, 2017
 */

#include "compMsgDispatcher.h"

// ================================= dataValueStr2ValueId ====================================

static uint8_t dataValueStr2ValueId(uint8_t *fieldName, str2id_t *ids, uint8_t *fieldId) {
  const str2id_t *entry;

  entry = &ids[0];
  while (entry->str != NULL) {
    if (c_strcmp(entry->str, fieldName) == 0) {
      *fieldId = entry->id;
      return COMP_MSG_ERR_OK;
    }
    entry++;
  }
  return COMP_MSG_ERR_FIELD_NOT_FOUND;
}

// ================================= addDataValue ====================================

static uint8_t addDataValue(uint8_t fieldNameId, uint8_t flags, fieldValueCallback_t fieldValueCallback, int numericValue, uint8_t *stringValue) {
  uint8_t result;
  dataValue_t *dataValue;
//  int numDataValues;
//  int maxDataValues;
//  dataValue_t *dataValues;

  result = COMP_MSG_ERR_OK;

  return result;
}

// ================================= setDataValue ====================================

static uint8_t setDataValue(uint8_t fieldNameId, uint8_t flags, fieldValueCallback_t fieldValueCallback, int numericValue, uint8_t *stringValue) {
  uint8_t result;

  result = COMP_MSG_ERR_OK;

  return result;
}

// ================================= getDataValue ====================================

static uint8_t getDataValue(uint8_t fieldNameId, uint8_t flags, int *numericValue, uint8_t **stringValue) {
  uint8_t result;

  result = COMP_MSG_ERR_OK;

  return result;
}

// ================================= compMsgDataValueInit ====================================

static uint8_t compMsgDataValueInit(compMsgDispatcher_t *self) {
  uint8_t result;
  compMsgDataValue_t *compMsgDataValue;

  compMsgDataValue = self->compMsgDataValue;
  compMsgDataValue->dataValueStr2ValueId = &dataValueStr2ValueId;
  compMsgDataValue->addDataValue = &addDataValue;
  compMsgDataValue->setDataValue = &setDataValue;
  compMsgDataValue->getDataValue = &getDataValue;
  return COMP_MSG_ERR_OK;
}

// ================================= newCompMsgDataValue ====================================

compMsgDataValue_t *newCompMsgDataValue() {
  compMsgDataValue_t *compMsgDataValue;

  compMsgDataValue = os_zalloc(sizeof(compMsgDataValue_t));
  if (compMsgDataValue == NULL) {
    return NULL;
  }
  compMsgDataValue->compMsgDataValueInit = &compMsgDataValueInit;
  return compMsgDataValue;
}
