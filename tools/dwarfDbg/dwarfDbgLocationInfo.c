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
 * File:   dwarfDbgLocationInfo.c
 * Author: Arnulf P. Wiedemann <arnulf@wiedemann-pri.de>
 *
 * Created on February 07, 2017
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libelf.h>
#include <stdlib.h>
#include <string.h>

#include "dwarfDbgInt.h"

/* *************************************************************************
 * dirName:
 *   maxDirName 5   # max free slots
 *   numDirName 3   # next free slot
 *   dirNames  char *dirName0 | char *dirName1 | char *dirnName2
 *
 * */

// =================================== addLocation =========================== 

static uint8_t addLocationDirName(dwarfDbgPtr_t self, char *dirName) {
  uint8_t result;

  result = DWARF_DBG_ERR_OK;
  if (self->dwarfDbgFileInfo->dirNamesInfo.maxDirName <= self->dwarfDbgFileInfo->dirNamesInfo.numDirName) {
    self->dwarfDbgFileInfo->dirNamesInfo.maxDirName += 50;
    if (self->dwarfDbgFileInfo->dirNamesInfo.dirNames == NULL) {
      self->dwarfDbgFileInfo->dirNamesInfo.dirNames = (char **)ckalloc(sizeof(char *) * self->dwarfDbgFileInfo->dirNamesInfo.maxDirName);
      if (self->dwarfDbgFileInfo->dirNamesInfo.dirNames == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    } else {
      self->dwarfDbgFileInfo->dirNamesInfo.dirNames = (char **)ckrealloc((char *)self->dwarfDbgFileInfo->dirNamesInfo.dirNames, sizeof(char *) * self->dwarfDbgFileInfo->dirNamesInfo.maxDirName);
      if (self->dwarfDbgFileInfo->dirNamesInfo.dirNames == NULL) {
        return DWARF_DBG_ERR_OUT_OF_MEMORY;
      }
    }
  }
  self->dwarfDbgFileInfo->dirNamesInfo.dirNames[self->dwarfDbgFileInfo->dirNamesInfo.numDirName] = (char *)ckalloc(strlen(dirName) + 1);
  if (self->dwarfDbgFileInfo->dirNamesInfo.dirNames[self->dwarfDbgFileInfo->dirNamesInfo.numDirName] == NULL) {
    return DWARF_DBG_ERR_OUT_OF_MEMORY;
  }
  self->dwarfDbgFileInfo->dirNamesInfo.dirNames[self->dwarfDbgFileInfo->dirNamesInfo.numDirName][strlen(dirName)] = '\0';
  memcpy(self->dwarfDbgFileInfo->dirNamesInfo.dirNames[self->dwarfDbgFileInfo->dirNamesInfo.numDirName], dirName, strlen(dirName));
//printf("addDirName: %d %s\n", self->dwarfDbgFileInfo->dirNamesInfo.numDirName, dirName);
  self->dwarfDbgFileInfo->dirNamesInfo.numDirName++;
  return result;
}


// =================================== dwarfDbgLocationInfoInit =========================== 

int dwarfDbgLocationInfoInit (dwarfDbgPtr_t self) {
//  self->dwarfDbgFileInfo->dirNamesInfo.maxDirName = 0;
//  self->dwarfDbgFileInfo->dirNamesInfo.numDirName = 0;
//  self->dwarfDbgFileInfo->dirNamesInfo.dirNames = NULL;

//  self->dwarfDbgFileInfo->addDirName = &addDirName;
  return DWARF_DBG_ERR_OK;
}


