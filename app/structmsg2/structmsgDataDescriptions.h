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
 * File:   structmsgDataDescriptions.h
 * Author: Arnulf P. Wiedemann
 *
 * Created on October 1st, 2016
 */

/* struct message data descriptions handling */

enum structmsDatagErrorCode
{
  STRUCT_DATA_DESC_ERR_OK                    = 0,
  STRUCT_DATA_DESC_ERR_VALUE_NOT_SET         = 255,
  STRUCT_DATA_DESC_ERR_VALUE_OUT_OF_RANGE    = 254,
  STRUCT_DATA_DESC_ERR_BAD_VALUE             = 253,
  STRUCT_DATA_DESC_ERR_BAD_FIELD_TYPE        = 252,
  STRUCT_DATA_DESC_ERR_FIELD_TYPE_NOT_FOUND  = 251,
  STRUCT_DATA_DESC_ERR_VALUE_TOO_BIG         = 250,
  STRUCT_DATA_DESC_ERR_OUT_OF_MEMORY         = 249,
  STRUCT_DATA_DESC_ERR_OUT_OF_RANGE          = 248,
  // be carefull the values up to here
  // must correspond to the values in dataView.h !!!
  // with the names like DATA_VIEW_ERR_*

  STRUCT_DATA_DESC_ERR_OPEN_FILE             = 189,
  STRUCT_DATA_DESC_FILE_NOT_OPENED           = 188,
  STRUCT_DATA_DESC_ERR_FLUSH_FILE            = 187,
  STRUCT_DATA_DESC_ERR_WRITE_FILE            = 186,
};


typedef struct structmsgDataDescription structmsgDataDescription_t;

typedef uint8_t (* openFile_t)(structmsgDataDescription_t *self, const uint8_t *fileName, const uint8_t *fileMode);
typedef uint8_t (* closeFile_t)(structmsgDataDescription_t *self);
typedef uint8_t (* flushFile_t)(structmsgDataDescription_t *self);
typedef uint8_t (* readLine_t)(structmsgDataDescription_t *self, uint8_t **buffer, uint8_t *lgth);
typedef uint8_t (* writeLine_t)(structmsgDataDescription_t *self, const uint8_t *buffer, uint8_t lgth);


typedef struct structmsgDataDescription {
  uint8_t id;
  uint8_t *FileName;
  uint8_t fileId;
  size_t fileSize;
  
  openFile_t openFile;
  closeFile_t closeFile;
  flushFile_t flushFile;
  readLine_t readLine;
  writeLine_t writeLine;

} structmsgDataDescription_t;

structmsgDataDescription_t *newStructmsgDataDescription();
void freeStructmsgDataDescription(structmsgDataDescription_t *structmsgDataDescription);
