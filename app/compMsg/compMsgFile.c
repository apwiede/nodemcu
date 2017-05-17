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
 * File:   compMsgFile.c
 * Author: Arnulf P. Wiedemann
 *
 * Created on May 5st, 2017
 */

#include "compMsgDispatcher.h"

static volatile int fileFd = FS_OPEN_OK - 1;

// ================================= openFile ====================================

static uint8_t openFile(compMsgDispatcher_t *self, const uint8_t *fileName, const uint8_t *fileMode) {
  compMsgFile_t *compMsgFile;

  compMsgFile = self->compMsgFile;
  compMsgFile->fileName = fileName;
  fileFd = fs_open(fileName, fs_mode2flag(fileMode));
  if (fileFd < FS_OPEN_OK) {
    return COMP_MSG_ERR_OPEN_FILE;
  }
  return COMP_MSG_ERR_OK;
}

// ================================= closeFile ====================================

static uint8_t closeFile(compMsgDispatcher_t *self) {
  compMsgFile_t *compMsgFile;

  compMsgFile = self->compMsgFile;
  if (fileFd != (FS_OPEN_OK - 1)){
    compMsgFile->fileName = NULL;
    fs_close(fileFd);
    fileFd = FS_OPEN_OK - 1;
  }
  return COMP_MSG_ERR_OK;
}

// ================================= flushFile ====================================

static uint8_t flushFile(compMsgDispatcher_t *self) {
  if (fileFd == (FS_OPEN_OK - 1)) {
    return COMP_MSG_ERR_FILE_NOT_OPENED;
  }
  if (fs_flush(fileFd) == 0) {
    return COMP_MSG_ERR_OK;
  }
  return COMP_MSG_ERR_FLUSH_FILE;
}

// ================================= readLine ====================================

static uint8_t readLine(compMsgDispatcher_t *self, uint8_t **buffer, uint8_t *lgth) {
  size_t n = BUFSIZ;
  char buf[BUFSIZ];
  int i;
  uint8_t *cp;
  uint8_t end_char = '\n';

  if (fileFd == (FS_OPEN_OK - 1)) {
    return COMP_MSG_ERR_FILE_NOT_OPENED;
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
  return COMP_MSG_ERR_OK;
}

// ================================= writeLine ====================================

static uint8_t writeLine(compMsgDispatcher_t *self, const uint8_t *buffer, uint8_t lgth) {
  int result;

  if (fileFd == (FS_OPEN_OK - 1)) {
    return COMP_MSG_ERR_FILE_NOT_OPENED;
  }
  result = fs_write(fileFd, buffer, lgth);
  if (result == lgth) {
    return COMP_MSG_ERR_OK;
  }
  return COMP_MSG_ERR_WRITE_FILE;
}

// ================================= getLineFields ====================================

static uint8_t getLineFields(compMsgDispatcher_t *self, uint8_t *myStr, uint8_t lgth) {
  char *cp;
  char *ep;
  int idx;
  compMsgFile_t *compMsgFile;

  compMsgFile = self->compMsgFile;
  compMsgFile->numLineFields = 0;
  cp = myStr;
  ep = myStr + lgth;
  compMsgFile->lineFields[compMsgFile->numLineFields] = cp;
  while (cp < ep) {
    if (*cp == ',') {
      *cp = '\0';
      cp++;
      compMsgFile->numLineFields++;
      if (compMsgFile->numLineFields >= MSG_MAX_LINE_FIELDS) {
        return COMP_MSG_ERR_TOO_MANY_LINE_FIELDS;
      }
      compMsgFile->lineFields[self->compMsgFile->numLineFields] = cp;
    } else {
      if ((*cp == '\r') || (*cp == '\n')) {
        *cp = '\0';
      }
      cp++;
    }
  }
  compMsgFile->numLineFields++;
//  idx = 0;
//while (idx < compMsgFile->numLineFields) {
//ets_printf("lineField: %d: %s!\n", idx, compMsgFile->lineFields[idx]);
//idx++;
//}
  return COMP_MSG_ERR_OK;
}

// ================================= getIntFieldValue ====================================

static uint8_t getIntFieldValue(compMsgDispatcher_t *self, uint8_t *cp, char **ep, int base, int *uval) {
  *uval = c_strtoul(cp, ep, base);
  return COMP_MSG_ERR_OK;
}

// ================================= getStringFieldValue ====================================

static uint8_t getStringFieldValue(compMsgDispatcher_t *self, uint8_t *cp, uint8_t **strVal) {
  uint8_t *cp1;
  uint8_t *cp2;
  char *endPtr;

  cp1 = cp;
  // we allow callbacks starting with a '@' here as strings without having a double quote
  if ((*cp1 != '"') && (*cp1 != '@')) {
    // not a string
    *strVal = NULL;
    return COMP_MSG_ERR_NOT_A_STRING;
  }
  if (*cp1 != '@') {
    cp1++;
  }
  *strVal = cp1;
  while (*cp1 != '\0') {
    if (cp1[0] == '"') {
      if (cp1[1] == '\0') {
        cp1[0] = '\0';
        break;
      }
    }
    // if we have no '"' at the end, silently ignore that
    cp1++;
  }
  return COMP_MSG_ERR_OK;
}

// ================================= compMsgFileInit ====================================

static uint8_t compMsgFileInit(compMsgDispatcher_t *self) {
  uint8_t result;
  compMsgFile_t *compMsgFile;

  compMsgFile = self->compMsgFile;
  compMsgFile->openFile = &openFile;
  compMsgFile->closeFile = &closeFile;
ets_printf("compMsgFileInit\n");
ets_printf("compMsgFile->closeFile: %p\n", compMsgFile->closeFile);
  compMsgFile->flushFile = &flushFile;
  compMsgFile->readLine = &readLine;
  compMsgFile->writeLine = &writeLine;
  compMsgFile->getLineFields = &getLineFields;

  compMsgFile->getIntFieldValue = &getIntFieldValue;
  compMsgFile->getStringFieldValue = &getStringFieldValue;

  return COMP_MSG_ERR_OK;
}


// ================================= newCompMsgFile ====================================

compMsgFile_t *newCompMsgFile() {
  compMsgFile_t *compMsgFile;

  compMsgFile = os_zalloc(sizeof(compMsgFile_t));
  if (compMsgFile == NULL) {
    return NULL;
  }
  compMsgFile->compMsgFileInit = &compMsgFileInit;
  return compMsgFile;
}
