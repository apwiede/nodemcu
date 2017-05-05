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
 * File:   compMsgFile.h
 * Author: Arnulf P. Wiedemann
 *
 * Created on May 5th, 2017
 */

#ifndef COMP_MSG_FILE_H
#define	COMP_MSG_FILE_H

#define MSG_MAX_LINE_FIELDS 10

typedef uint8_t (* openFile_t)(compMsgDispatcher_t *self, const uint8_t *fileName, const uint8_t *fileMode);
typedef uint8_t (* closeFile_t)(compMsgDispatcher_t *self);
typedef uint8_t (* flushFile_t)(compMsgDispatcher_t *self);
typedef uint8_t (* readLine_t)(compMsgDispatcher_t *self, uint8_t **buffer, uint8_t *lgth);
typedef uint8_t (* writeLine_t)(compMsgDispatcher_t *self, const uint8_t *buffer, uint8_t lgth);
typedef uint8_t (* getLineFields_t)(compMsgDispatcher_t *self, uint8_t *myStr, uint8_t lgth);

typedef uint8_t (* getIntFieldValue_t)(compMsgDispatcher_t *self, uint8_t *cp, char **ep, int base, int *uval);
typedef uint8_t (* getStringFieldValue_t)(compMsgDispatcher_t *self, uint8_t *cp, uint8_t **strVal);

typedef uint8_t (* compMsgFileInit_t)(compMsgDispatcher_t *self);

typedef struct compMsgFile {
  const uint8_t *fileName;
  uint8_t fileId;
  size_t fileSize;
  int numLineFields;
  uint8_t *lineFields[MSG_MAX_LINE_FIELDS];

  openFile_t openFile;
  closeFile_t closeFile;
  flushFile_t flushFile;
  readLine_t readLine;
  writeLine_t writeLine;
  getLineFields_t getLineFields;

  getIntFieldValue_t getIntFieldValue;
  getStringFieldValue_t getStringFieldValue;
  compMsgFileInit_t compMsgFileInit;

} compMsgFile_t;

compMsgFile_t *newCompMsgFile();

#endif	/* COMP_MSG_FILE_H */
