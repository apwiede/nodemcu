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

#include "osapi.h"
#include "mem.h"
#include "c_string.h"
#include "c_stdlib.h"
#include "structmsg2.h"
#include "../crypto/mech.h"

#ifdef NOTDEF


// ============================= structmsg_decodeDefinition ========================

int structmsg_decodeDefinition (const uint8_t *name, const uint8_t *data, stmsgDefinitions_t *structmsgDefinitions, fieldNameDefinitions_t *fieldNameDefinitions, uint8_t shortCmdKey) {
  size_t numSpecFields;
  size_t namesSpecSize;
  size_t numNormFields;
  size_t namesNormSize;
  size_t fillerSize;
  int result;
  int idx;
  int found = 0;
  uint8_t *fieldName;
  size_t definitionPayloadSize;
  size_t payloadSize;
  size_t myLgth;
  stmsgDefinition_t *definition;
  fieldInfoDefinition_t *fieldInfo;
  int offset;
  int crcOffset;
  uint16_t src;
  uint16_t dst;
  uint16_t totalLgth;
  uint16_t cmdKey;
  uint16_t cmdLgth;
  uint16_t crc;
  uint32_t randomNum;
  uint8_t fillerStr[16];
  uint8_t *filler = fillerStr;;
  uint8_t numEntries;

  offset = 0;
  offset = uint16Decode(data, offset, &src); 
  checkDecodeOffset(offset);
  offset = uint16Decode(data, offset, &dst); 
  checkDecodeOffset(offset);
  offset = uint16Decode(data, offset, &totalLgth); 
  checkDecodeOffset(offset);
  offset = uint16Decode(data, offset, &cmdKey); 
  checkDecodeOffset(offset);
  if (cmdKey != STRUCT_MSG_DEFINITION_CMD_KEY) {
    return STRUCT_MSG_ERR_BAD_DEFINTION_CMD_KEY;
  }
  offset = uint16Decode(data, offset, &cmdLgth); 
  checkDecodeOffset(offset);
  offset = uint32Decode(data, offset, &randomNum); 
  checkDecodeOffset(offset);
  // now check the crc
  crcOffset = totalLgth - sizeof(uint16_t);
  crcOffset = crcDecode(data, crcOffset, cmdLgth, &crc, STRUCT_MSG_HEADER_LENGTH, 0);
  offset = definitionDecode(data, offset, definition, fieldNameDefinitions, shortCmdKey);
  checkDecodeOffset(offset);
  myLgth = offset + sizeof(uint16_t);
  fillerSize = 0;
  while ((myLgth % 16) != 0) {
    myLgth++;
    fillerSize++;
  }
  offset = fillerDecode(data, offset, fillerSize, &filler);
  checkDecodeOffset(offset);
  return STRUCT_MSG_ERR_OK;
}

// ============================= stmsg_getDefinitionName ========================

int stmsg_getDefinitionName(uint8_t *decrypted, uint8_t **name, uint8_t shortCmdKey) {
  int nameOffset;
  uint8_t numNormFields;
  uint16_t normNamesSize;
  uint8_t nameLgth;
  uint8_t numEntries;

  *name = NULL;
  nameOffset = STRUCT_MSG_HEADER_LENGTH; // src + dst + totalLgth
  if (shortCmdKey) {
    nameOffset += STRUCT_MSG_SHORT_CMD_HEADER_LENGTH; // cmdKey + cmdLgth
  } else {
    nameOffset += STRUCT_MSG_CMD_HEADER_LENGTH; // cmdKey + cmdLgth
  }
  // randomNum
  nameOffset += sizeof(uint32_t);
  // len ids 
  nameOffset = uint8Decode(decrypted, nameOffset, &numNormFields);
  // ids vector
  nameOffset += numNormFields * sizeof(uint16_t);
  // size of name strings (normnamesSize)
  nameOffset = uint16Decode(decrypted, nameOffset, &normNamesSize);
  // names vector
  nameOffset += normNamesSize; 
  // definitionSize + nameLgth
  nameOffset += sizeof(uint16_t) + sizeof(uint8_t);
  // here the name starts
  *name = decrypted + nameOffset;
  return STRUCT_MSG_ERR_OK;
}

#endif
