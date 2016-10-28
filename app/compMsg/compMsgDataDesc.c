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
 * File:   compMsgDataDesc.c
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
#include "compMsgDataDesc.h"


static int compMsgDataDescId = 0;
static volatile int fileFd = FS_OPEN_OK - 1;

// ================================= openFile ====================================

static uint8_t openFile(compMsgDataDesc_t *self, const uint8_t *fileName, const uint8_t *fileMode) {
  self->fileName = fileName;
  fileFd = fs_open(fileName, fs_mode2flag(fileMode));
  if (fileFd < FS_OPEN_OK) {
    return COMP_DATA_DESC_ERR_OPEN_FILE;
  }
  return COMP_DATA_DESC_ERR_OK;
}

// ================================= closeFile ====================================

static uint8_t closeFile(compMsgDataDesc_t *self) {
  if (fileFd != (FS_OPEN_OK - 1)){
    self->fileName = NULL;
    fs_close(fileFd);
    fileFd = FS_OPEN_OK - 1;
  }
  return COMP_DATA_DESC_ERR_OK;
}

// ================================= flushFile ====================================

static uint8_t flushFile(compMsgDataDesc_t *self) {
  if (fileFd == (FS_OPEN_OK - 1)) {
    return COMP_DATA_DESC_FILE_NOT_OPENED;
  }
  if (fs_flush(fileFd) == 0) {
    return COMP_DATA_DESC_ERR_OK;
  }
  return COMP_DATA_DESC_ERR_FLUSH_FILE;
}

// ================================= readLine ====================================

static uint8_t readLine(compMsgDataDesc_t *self, uint8_t **buffer, uint8_t *lgth) {
  size_t n = BUFSIZ;
  char buf[BUFSIZ];
  int i;
  uint8_t *cp;
  uint8_t end_char = '\n';

  if (fileFd == (FS_OPEN_OK - 1)) {
    return COMP_DATA_DESC_FILE_NOT_OPENED;
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
  return COMP_DATA_DESC_ERR_OK;
}

// ================================= writeLine ====================================

static uint8_t writeLine(compMsgDataDesc_t *self, const uint8_t *buffer, uint8_t lgth) {
  int result;

  if (fileFd == (FS_OPEN_OK - 1)) {
    return COMP_DATA_DESC_FILE_NOT_OPENED;
  }
  result = fs_write(fileFd, buffer, lgth);
  if (result == lgth) {
    return COMP_DATA_DESC_ERR_OK;
  }
  return COMP_DATA_DESC_ERR_WRITE_FILE;
}

// ================================= newCompMsgDataDesc ====================================

compMsgDataDesc_t *newCompMsgDataDesc() {
  compMsgDataDesc_t *compMsgDataDesc = os_zalloc(sizeof(compMsgDataDesc_t));
  if (compMsgDataDesc == NULL) {
    return NULL;
  }
  compMsgDataDescId++;
  compMsgDataDesc->id = compMsgDataDescId;

  compMsgDataDesc->openFile = &openFile;
  compMsgDataDesc->closeFile = &closeFile;
  compMsgDataDesc->flushFile = &flushFile;
  compMsgDataDesc->readLine = &readLine;
  compMsgDataDesc->writeLine = &writeLine;
  return compMsgDataDesc;
}

// ================================= freeCompMsgDataDesc ====================================

void freeCompMsgDataDesc(compMsgDataDesc_t *compMsgDataDesc) {
}

