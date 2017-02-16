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
 * File:   dataView.h
 * Author: Arnulf P. Wiedemann
 *
 * Created on September 24, 2016
 */

#ifndef COMP_MSG_DATA_VIEW_H
#define	COMP_MSG_DATA_VIEW_H

#include "c_types.h"

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct compMsgDispatcher compMsgDispatcher_t;
typedef uint8_t (* fieldSizeCallback_t)(compMsgDispatcher_t *self);
typedef uint8_t (* fieldValueCallback_t)(compMsgDispatcher_t *self, int *numericValue, uint8_t **stringValue);

typedef struct compMsgField {
  uint8_t fieldNameId;
  uint8_t fieldTypeId;
  uint8_t fieldFlags;
  uint8_t fieldKeyTypeId;
  uint16_t fieldLgth;
  uint16_t fieldKey;
  size_t fieldOffset;
  fieldSizeCallback_t fieldSizeCallback;
} compMsgField_t;

typedef struct compMsgDispatcher compMsgDispatcher_t;
typedef struct compMsgDataView compMsgDataView_t;

typedef uint8_t (* getRandomNum_t)(compMsgDispatcher_t *self, dataView_t *dataView, compMsgField_t *fieldInfo, uint32_t *value);
typedef uint8_t (* setRandomNum_t)(compMsgDispatcher_t *self, dataView_t *dataView, compMsgField_t *fieldInfo);

typedef uint8_t (* getSequenceNum_t)(compMsgDispatcher_t *self, dataView_t *dataView, compMsgField_t *fieldInfo, uint32_t *value);
typedef uint8_t (* setSequenceNum_t)(compMsgDispatcher_t *self, dataView_t *dataView, compMsgField_t *fieldInfo);
typedef uint8_t (* setSavedSequenceNum_t)(compMsgDispatcher_t *self, dataView_t *dataView, compMsgField_t *fieldInfo);

typedef uint8_t (* getFiller_t)(compMsgDispatcher_t *self, dataView_t *dataView, compMsgField_t *fieldInfo, uint8_t **value);
typedef uint8_t (* setFiller_t)(compMsgDispatcher_t *self, dataView_t *dataView, compMsgField_t *fieldInfo);
typedef uint8_t (* setZeroFiller_t)(compMsgDispatcher_t *self, dataView_t *dataView, compMsgField_t *fieldInfo);

typedef uint8_t (* getCrc_t)(compMsgDispatcher_t *self, dataView_t *dataView, compMsgField_t *fieldInfo, size_t startOffset, size_t lgth);
typedef uint8_t (* setCrc_t)(compMsgDispatcher_t *self, dataView_t *dataView, compMsgField_t *fieldInfo, size_t startOffset, size_t lgth);

typedef uint8_t (* getTotalCrc_t)(compMsgDispatcher_t *self, dataView_t *dataView, compMsgField_t *fieldInfo);
typedef uint8_t (* setTotalCrc_t)(compMsgDispatcher_t *self, dataView_t *dataView, compMsgField_t *fieldInfo);

typedef uint8_t (* dvGetFieldValue_t)(compMsgDispatcher_t *self, dataView_t *dataView, compMsgField_t *fieldInfo, int *numericValue, uint8_t **stringValue, int fieldIdx);
typedef uint8_t (* dvSetFieldValue_t)(compMsgDispatcher_t *self, dataView_t *dataView, compMsgField_t *fieldInfo, int numericValue, const uint8_t *stringValue, int fieldIdx);

typedef struct compMsgDataView {
  dataView_t *dataView;
  uint8_t id;

  getRandomNum_t getRandomNum;
  setRandomNum_t setRandomNum;

  getSequenceNum_t getSequenceNum;
  setSequenceNum_t setSequenceNum;
  setSavedSequenceNum_t setSavedSequenceNum;

  getFiller_t getFiller;
  setFiller_t setFiller;
  setZeroFiller_t setZeroFiller;

  getCrc_t getCrc;
  setCrc_t setCrc;

  getTotalCrc_t getTotalCrc;
  setTotalCrc_t setTotalCrc;

  dvGetFieldValue_t getFieldValue;
  dvSetFieldValue_t setFieldValue;

} compMsgDataView_t;

compMsgDataView_t *newCompMsgDataView(uint8_t *data, size_t lgth);

#ifdef	__cplusplus
}
#endif

#endif	/* COMP_MSG_DATA_VIEW_H */
