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
 * File:   structmsgDataDescriptions.c
 * Author: Arnulf P. Wiedemann
 *
 * Created on October 1st, 2016
 */

#include "osapi.h"
#include "c_types.h"
#include "mem.h"
#include "flash_fs.h"

#include "c_string.h"
#include "c_stdio.h"
#include "structmsgDataDescriptions.h"


static int structmsgDataDescriptionId = 0;
static volatile int fileFd = FS_OPEN_OK - 1;

// ================================= openFile ====================================

static uint8_t openFile(structmsgDataDescription_t *self, const uint8_t *fileName, const uint8_t *fileMode) {
  fileFd = fs_open(fileName, fs_mode2flag(fileMode));
  
  if (fileFd < FS_OPEN_OK) {
    return STRUCT_DATA_DESC_ERR_OPEN_FILE;
  }
  return STRUCT_DATA_DESC_ERR_OK;
}

// ================================= closeFile ====================================

static uint8_t closeFile(structmsgDataDescription_t *self) {
  if (fileFd != (FS_OPEN_OK - 1)){
    fs_close(fileFd);
    fileFd = FS_OPEN_OK - 1;
  }
  return STRUCT_DATA_DESC_ERR_OK;
}

// ================================= flushFile ====================================

static uint8_t flushFile(structmsgDataDescription_t *self) {
  if (fileFd == (FS_OPEN_OK - 1)) {
    return STRUCT_DATA_DESC_FILE_NOT_OPENED;
  }
  if (fs_flush(fileFd) == 0) {
    return STRUCT_DATA_DESC_ERR_OK;
  }
  return STRUCT_DATA_DESC_ERR_FLUSH_FILE;
}

// ================================= readLine ====================================

static uint8_t readLine(structmsgDataDescription_t *self, uint8_t **buffer, uint8_t *lgth) {
  size_t n = BUFSIZ;
  char buf[BUFSIZ];
  int i;
  uint8_t *cp;
  uint8_t end_char = '\n';

  if (fileFd == (FS_OPEN_OK - 1)) {
    return STRUCT_DATA_DESC_FILE_NOT_OPENED;
  }
  n = fs_read(fileFd, buf, n);
  cp = *buffer;
  *lgth = 0;
  for (i = 0; i < n; ++i) {
    cp[i] = buf[i];
    if (buf[i] == end_char) {
      ++i;
      break;
    }
  }
  cp[i] = 0;
  *lgth = i;
  fs_seek (fileFd, -(n - i), SEEK_CUR);
  return STRUCT_DATA_DESC_ERR_OK;
}

// ================================= writeLine ====================================

static uint8_t writeLine(structmsgDataDescription_t *self, const uint8_t *buffer, uint8_t lgth) {
  int result;

  if (fileFd == (FS_OPEN_OK - 1)) {
    return STRUCT_DATA_DESC_FILE_NOT_OPENED;
  }
  result = fs_write(fileFd, buffer, lgth);
  if (result == lgth) {
    return STRUCT_DATA_DESC_ERR_OK;
  }
  return STRUCT_DATA_DESC_ERR_WRITE_FILE;
}

// ================================= newStructmsgDataDescrption ====================================

structmsgDataDescription_t *newStructmsgDataDescription() {
  structmsgDataDescription_t *structmsgDataDescription = os_zalloc(sizeof(structmsgDataDescription_t));
  if (structmsgDataDescription == NULL) {
    return NULL;
  }
  structmsgDataDescriptionId++;
  structmsgDataDescription->id = structmsgDataDescriptionId;

  structmsgDataDescription->openFile = &openFile;
  structmsgDataDescription->closeFile = &closeFile;
  structmsgDataDescription->flushFile = &flushFile;
  structmsgDataDescription->readLine = &readLine;
  structmsgDataDescription->writeLine = &writeLine;
  return structmsgDataDescription;
}

// ================================= freeStructmsgDataDescrption ====================================

void freeStructmsgDataDescription(structmsgDataDescription_t *structmsgDataDescription) {
}

