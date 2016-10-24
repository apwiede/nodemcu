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

// Module for handling structmsgs

#include "module.h"
#include "lauxlib.h"
#include "platform.h"

#include "c_types.h"
#include "c_string.h"
#include "c_stdarg.h"
#include "rom.h"
#include "structmsgData.h"
#include "osapi.h"
#define MEMLEAK_DEBUG 1
#include "mem.h"

static lua_State *gL = NULL;
static const uint8_t *errStr;

// ============================= structmsg_error ========================

static int structmsg_error( lua_State* L, const char *fmt, ... ) {
  va_list argp;

  va_start(argp, fmt);
  lua_pushfstring(L, fmt, argp );
  va_end(argp);
  return 0;
}

// ============================= checkOKOrErr ========================

static int checkOKOrErr( lua_State* L, int result, const uint8_t *where, const uint8_t *fieldName ) {
  if (result == STRUCT_MSG_ERR_OK) {
    lua_pushfstring(L, "OK");
    return 1;
  }
ets_printf("§error result: %d\n§", result);
  errStr = "ERROR";
  switch (result) {
  case STRUCT_MSG_ERR_VALUE_NOT_SET:
    lua_pushfstring(L, "%s: %s: value for field: %s not set", errStr, where, fieldName);
    break;
  case STRUCT_MSG_ERR_VALUE_OUT_OF_RANGE:
    lua_pushfstring(L, "%s: %s: field: '%s' value out of range", errStr, where, fieldName);
    break;
  case STRUCT_MSG_ERR_BAD_VALUE:
    lua_pushfstring(L, "%s: %s: field: '%s' bad value", errStr, where, fieldName);
    break;
  case STRUCT_MSG_ERR_BAD_FIELD_TYPE:
    lua_pushfstring(L, "%s: %s: field: '%s' bad field type", errStr, where, fieldName);
    break;
  case STRUCT_MSG_ERR_FIELD_NOT_FOUND:
    lua_pushfstring(L, "%s: %s: field: '%s' not found", errStr, where, fieldName);
    break;
  case STRUCT_MSG_ERR_BAD_HANDLE:
    lua_pushfstring(L, "%s: bad handle", errStr);
    break;
  case STRUCT_MSG_ERR_OUT_OF_MEMORY:
    luaL_error(L, "out of memory");
    break;
  case STRUCT_MSG_ERR_HANDLE_NOT_FOUND:
    lua_pushfstring(L, "%s: handle not found", errStr);
    break;
  case STRUCT_MSG_ERR_NOT_ENCODED:
    lua_pushfstring(L, "%s: not encoded", errStr);
    break;
  case STRUCT_MSG_ERR_DECODE_ERROR:
    lua_pushfstring(L, "%s: decode error", errStr);
    break;
  case STRUCT_MSG_ERR_BAD_CRC_VALUE:
    lua_pushfstring(L, "%s: bad crc val", errStr);
    break;
  case STRUCT_MSG_ERR_CRYPTO_INIT_FAILED:
    lua_pushfstring(L, "%s: crypto init failed", errStr);
    break;
  case STRUCT_MSG_ERR_CRYPTO_OP_FAILED:
    lua_pushfstring(L, "%s: crypto op failed", errStr);
    break;
  case STRUCT_MSG_ERR_CRYPTO_BAD_MECHANISM:
    lua_pushfstring(L, "%s: crypto bad mechanism", errStr);
    break;
  case STRUCT_MSG_ERR_NOT_ENCRYPTED:
    lua_pushfstring(L, "%s: not encrypted", errStr);
    break;
  case STRUCT_MSG_ERR_DEFINITION_NOT_FOUND:
    lua_pushfstring(L, "%s: definiton not found", errStr);
    break;
  case STRUCT_MSG_ERR_BAD_SPECIAL_FIELD:
    lua_pushfstring(L, "%s: bad special field", errStr);
    break;
  case STRUCT_MSG_ERR_DEFINITION_TOO_MANY_FIELDS:
    lua_pushfstring(L, "%s: definition too many fields", errStr);
    break;
  case STRUCT_MSG_ERR_BAD_TABLE_ROW:
    lua_pushfstring(L, "%s: bad table row", errStr);
    break;
  case STRUCT_MSG_ERR_TOO_MANY_FIELDS:
    lua_pushfstring(L, "%s: too many fields", errStr);
    break;
  case STRUCT_MSG_ERR_BAD_DEFINTION_CMD_KEY:
    lua_pushfstring(L, "%s: bad definition cmd key", errStr);
    break;
  case STRUCT_MSG_ERR_NO_SLOT_FOUND:
    lua_pushfstring(L, "%s: no slot found", errStr);
    break;
  case STRUCT_MSG_ERR_BAD_NUM_FIELDS:
    lua_pushfstring(L, "%s: bad num fields", errStr);
    break;
  case STRUCT_MSG_ERR_ALREADY_INITTED:
    lua_pushfstring(L, "%s: already initted", errStr);
    break;
  case STRUCT_MSG_ERR_NOT_YET_INITTED:
    lua_pushfstring(L, "%s: not yet initted", errStr);
    break;
  case STRUCT_MSG_ERR_FIELD_CANNOT_BE_SET:
    lua_pushfstring(L, "%s: field cannot be set", errStr);
    break;
  case STRUCT_MSG_ERR_NO_SUCH_FIELD:
    lua_pushfstring(L, "%s: no such field", errStr);
    break;
  case STRUCT_MSG_ERR_BAD_DATA_LGTH:
    lua_pushfstring(L, "%s: bad data length", errStr);
    break;
  case STRUCT_MSG_ERR_NOT_YET_PREPARED:
    lua_pushfstring(L, "%s: not yet prepared", errStr);
    break;
  case STRUCT_DEF_ERR_ALREADY_INITTED:
    lua_pushfstring(L, "%s: def already initted", errStr);
    break;
  case STRUCT_DEF_ERR_NOT_YET_INITTED:
    lua_pushfstring(L, "%s: def not yet initted", errStr);
    break;
  case STRUCT_DEF_ERR_NOT_YET_PREPARED:
    lua_pushfstring(L, "%s: def not yet prepared", errStr);
    break;
  case STRUCT_DEF_ERR_ALREADY_CREATED:
    lua_pushfstring(L, "%s: def already created", errStr);
    break;
  case STRUCT_MSG_ERR_OUT_OF_RANGE:
    lua_pushfstring(L, "%s: out of range", errStr);
    break;
  case STRUCT_MSG_ERR_FIELD_TOTAL_LGTH_MISSING:
    lua_pushfstring(L, "%s: field @totalLgth missing", errStr);
    break;
  default:
    lua_pushfstring(L, "%s: funny result error code", errStr);
ets_printf("funny result: %d\n", result);
    break;
  }
  return 1;
}

#ifdef NOTDEF
// ============================= structmsg_encdec ========================

static int structmsg_encdec (lua_State *L, bool enc) { 
  const char *handle;
  const char *key;
  size_t klen;
  const char *data;
  const char *iv;
  size_t ivlen;
  uint8_t *buf;
  size_t lgth;
  int result;
  
  handle = luaL_checkstring( L, 1 );
  key = luaL_checklstring (L, 2, &klen);
  iv = luaL_optlstring (L, 3, "", &ivlen);
  result = structmsg_encryptdecrypt(handle, NULL, 0, key, klen, iv, ivlen,enc, &buf, &lgth);
//  result = stmsg_encdec(handle, key, klen, iv, ivlen, enc, &buf, &lgth);
  if (result == STRUCT_MSG_ERR_OK) {
    lua_pushlstring (L, buf, lgth);
  } else {
    checkOKOrErr(L, result, "encrypt/decrypt", "");
  }
  return 1;
}

// ============================= structmsg_encdec_msgDefinition ========================

static int structmsg_encdec_msgDefinition (lua_State *L, bool enc) { 
  const char *handle;
  const char *key;
  size_t klen;
  const char *data;
  const char *iv;
  size_t ivlen;
  uint8_t *buf;
  size_t lgth;
  int result;
  
  handle = luaL_checkstring( L, 1 );
  key = luaL_checklstring (L, 2, &klen);
  iv = luaL_optlstring (L, 3, "", &ivlen);
  result = structmsg_encdecDefinition(handle, key, klen, iv, ivlen, enc, &buf, &lgth);
  if (result == STRUCT_MSG_ERR_OK) {
lua_pushlstring (L, buf, lgth);
} else {
checkOKOrErr(L, result, "encryptdef/decryptdef", "");
}
return 1;
}
#endif

// ============================= structmsg_createMsg ========================

static int structmsg_createMsg(lua_State* L)
{
size_t numFieldInfos;
  uint8_t *handle;
  structmsgData_t *structmsgData;
  int result;

  numFieldInfos = luaL_checkinteger(L, 1);
  structmsgData = newStructmsgData();
  if (structmsgData == NULL) {
    checkOKOrErr(L, STRUCT_MSG_ERR_OUT_OF_MEMORY, "create", "");
  } else {
    result = structmsgData->createMsg(structmsgData, numFieldInfos, &handle);
    if (checkOKOrErr(L, result, "createMsg", "")) {
      lua_pushstring(L, handle);
    }
  }
  return 1;
}

// ============================= structmsg_deleteMsg ========================

static int structmsg_deleteMsg( lua_State* L )
{
  const char *handle;
  structmsgData_t *structmsgData;
  int result;

  handle = luaL_checkstring( L, 1 );
  result = structmsgGetPtrFromHandle(handle, &structmsgData);
  checkOKOrErr(L, result, "deleteMsg", "");
  result = structmsgData->deleteMsg(structmsgData);
  checkOKOrErr(L, result, "deleteMsg", "");
  return 1;
}

// ============================= structmsg_dumpMsg ========================

static int structmsg_dumpMsg( lua_State* L )
{
  const char *handle;
  structmsgData_t *structmsgData;
  int result;

  handle = luaL_checkstring( L, 1 );
  result = structmsgGetPtrFromHandle(handle, &structmsgData);
  checkOKOrErr(L, result, "dump", "");
  result = structmsgData->dumpMsg(structmsgData);
  checkOKOrErr(L, result, "dump", "");
  return 1;
}

// ============================= structmsg_initMsg ========================

static int structmsg_initMsg( lua_State* L )
{
  const char *handle;
  structmsgData_t *structmsgData = NULL;
  int result;

  handle = luaL_checkstring( L, 1 );
  result = structmsgGetPtrFromHandle(handle, &structmsgData);
  checkOKOrErr(L, result, "init", "");
  result = structmsgData->initMsg(structmsgData);
  checkOKOrErr(L, result, "init", "");
  return 1;
}

// ============================= structmsg_prepareMsg ========================

static int structmsg_prepareMsg( lua_State* L )
{
  const char *handle;
  structmsgData_t *structmsgData = NULL;
  int result;

  handle = luaL_checkstring( L, 1 );
  result = structmsgGetPtrFromHandle(handle, &structmsgData);
  checkOKOrErr(L, result, "perpare", "");
  result = structmsgData->prepareMsg(structmsgData);
  checkOKOrErr(L, result, "perpare", "");
  return 1;
}

// ============================= structmsg_addField ========================

static int structmsg_addField( lua_State* L ) {
  const char *handle;
  const uint8_t *fieldType;
  const uint8_t *fieldStr;
  uint8_t fieldLgth;
  structmsgData_t *structmsgData;
  int result;

  handle = luaL_checkstring( L, 1 );
  fieldStr = luaL_checkstring( L, 2 );
  fieldType = luaL_checkstring( L, 3 );
  fieldLgth = luaL_optinteger( L, 4, 1 );
  result = structmsgGetPtrFromHandle(handle, &structmsgData);
  checkOKOrErr(L, result, "addField", "");
  result = structmsgData->addField(structmsgData, fieldStr, fieldType, fieldLgth);
  checkOKOrErr(L, result, "addField", "");
  return 1;
}

// ============================= structmsg_setFieldValue ========================

static int structmsg_setFieldValue( lua_State* L ) {
  const char *handle;
  const uint8_t *fieldName;
  int numericValue;
  const uint8_t *stringValue;
  structmsgData_t *structmsgData;
  int result;

  handle = luaL_checkstring( L, 1 );
  fieldName = luaL_checkstring( L, 2 );
  if ( lua_isnumber(L, 3) ){
    numericValue = lua_tointeger(L, 3);
    stringValue = NULL;
  } else {
    numericValue = 0;
    stringValue = lua_tostring(L, 3);
  }
  result = structmsgGetPtrFromHandle(handle, &structmsgData);
  checkOKOrErr(L, result, "setFieldValue", "");
  result = structmsgData->setFieldValue(structmsgData, fieldName, numericValue, stringValue);
  checkOKOrErr(L, result, "setFieldValue", fieldName);
  return 1;
}

// ============================= structmsg_setTableFieldValue ========================

static int structmsg_setTableFieldValue( lua_State* L ) {
  const char *handle;
  const uint8_t *fieldName;
  int row;
  int numericValue;
  const uint8_t *stringValue;
  structmsgData_t *structmsgData;
  int result;

  handle = luaL_checkstring( L, 1 );
  fieldName = luaL_checkstring( L, 2 );
  row = luaL_checkinteger(L, 3);
  if ( lua_isnumber(L, 4) ){
    numericValue = lua_tointeger(L, 4);
    stringValue = NULL;
  } else {
    numericValue = 0;
    stringValue = lua_tostring(L, 4);
  }
  result = structmsgGetPtrFromHandle(handle, &structmsgData);
  checkOKOrErr(L, result, "setTableFieldValue", "");
  result = structmsgData->setTableFieldValue(structmsgData, fieldName, row, numericValue, stringValue);
  checkOKOrErr(L, result, "setTableFieldValue", fieldName);
  return 1;
}

// ============================= structmsg_getFieldValue ========================

static int structmsg_getFieldValue( lua_State* L ) {
  const char *handle;
  const uint8_t *fieldName;
  int numericValue = 0;
  uint8_t *stringValue = NULL;
  structmsgData_t *structmsgData;
  int result;

  handle = luaL_checkstring( L, 1 );
  fieldName = luaL_checkstring( L, 2 );
  result = structmsgGetPtrFromHandle(handle, &structmsgData);
  checkOKOrErr(L, result, "getFieldValue", "");
  result = structmsgData->getFieldValue(structmsgData, fieldName, &numericValue, &stringValue);
  checkOKOrErr(L, result, "getFieldValue", fieldName);
  if (stringValue == NULL) {
    lua_pushinteger(L, numericValue);
  } else {
  lua_pushstring(L, stringValue);
  }
  return 1;
}

// ============================= structmsg_getTableFieldValue ========================

static int structmsg_getTableFieldValue( lua_State* L ) {
  const char *handle;
  const uint8_t *fieldName;
  int numericValue = 0;
  int row;
  uint8_t *stringValue = NULL;
  structmsgData_t *structmsgData;
  int result;

  handle = luaL_checkstring( L, 1 );
  fieldName = luaL_checkstring( L, 2 );
  row = luaL_checkinteger( L, 3 );
  result = structmsgGetPtrFromHandle(handle, &structmsgData);
  checkOKOrErr(L, result, "getTableFieldValue", "");
  result = structmsgData->getTableFieldValue(structmsgData, fieldName, row, &numericValue, &stringValue);
  checkOKOrErr(L, result, "getTableFieldValue", fieldName);
  if (stringValue == NULL) {
    lua_pushinteger(L, numericValue);
  } else {
    lua_pushstring(L, stringValue);
  }
  return 1;
}

// ============================= structmsg_getMsgData ========================

static int structmsg_getMsgData( lua_State* L ) {
  const char *handle;
  uint8_t *data;
  structmsgData_t *structmsgData;
  int lgth;
  int result;

  handle = luaL_checkstring( L, 1 );
  result = structmsgGetPtrFromHandle(handle, &structmsgData);
  checkOKOrErr(L, result, "getMsgData", "");
  result = structmsgData->getMsgData(structmsgData, &data, &lgth);
  if (checkOKOrErr(L, result, "getMsgData", "")) {
    lua_pushlstring(L, data, lgth);
  }
  return 1;
}

// ============================= structmsg_setMsgData ========================

static int structmsg_setMsgData( lua_State* L ) {
  const char *handle;
  const uint8_t *data;
  structmsgData_t *structmsgData;
  int lgth;
  int result;

  handle = luaL_checkstring( L, 1 );
  data = luaL_checkstring( L, 2 );
  result = structmsgGetPtrFromHandle(handle, &structmsgData);
  checkOKOrErr(L, result, "setMsgData", "");
  result = structmsgData->setMsgData(structmsgData, data);
  if (checkOKOrErr(L, result, "setMsgData", "")) {
    lua_pushlstring(L, data, lgth);
  }
  return 1;
}

// ============================= parseKey ==================

static const uint8_t *parseKey(lua_State* L, int paramNum, const char * key){
  const uint8_t *cp;
  lua_getfield(L, paramNum, key);
  if( lua_isstring(L, -1) ) {
    const char *str = luaL_checkstring( L, -1 );
    return str;
  } 
  lua_pop(L, 1); 
  return NULL;
}     

// ============================= structmsg_createMsgFromList ==================

// Lua: createMsgFromList(list,numRows,flags)
static int structmsg_createMsgFromList(lua_State* L) {
  const uint8_t *list;
  uint8_t numRows;
  uint8_t numEntries;
  uint16_t flags;
  uint8_t *handle;
  const uint8_t **listVector;
  int idx;
  int result;
  char buf[10];
  const uint8_t *str;

  if (!lua_istable(L, 1)) {
ets_printf("no table!!\n");
    return 1;
  }
  numEntries = 0;
  lua_pushnil(L);  /* first key */
  while(lua_next(L, 1)) {
    lua_pop(L, 1);  /* remove value */
    numEntries++;
  }
  idx = 0;
  listVector = os_zalloc(numEntries * sizeof(uint8_t *) + 1);
  while (idx < numEntries) {
    os_sprintf(buf, "%d", idx+1);
    listVector[idx] = parseKey(L, 1, buf);
    idx++;
  }
  numRows = luaL_checkinteger (L, 2);
  flags = luaL_checkinteger (L, 3);
  result = newStructmsgDataFromList(listVector, numEntries, numRows, flags, &handle);
  os_free(listVector);
  if (result == STRUCT_MSG_ERR_OK) {
    lua_pushstring (L, handle);
  } else {
    checkOKOrErr(L, result, "createMsgFromList", "");
  }
  return 1;
}

// ============================= structmsg_setMsgFieldsFromList ==================

// Lua: setMsgFieldsFromList(handle,list,flags)
static int structmsg_setMsgFieldsFromList(lua_State* L) {
  const uint8_t *list;
  const uint8_t *handle;
  uint8_t numRows;
  uint8_t numEntries;
  uint16_t flags;
  const uint8_t **listVector;
  int idx;
  int result;
  char buf[10];
  structmsgData_t *structmsgData;
  const uint8_t *str;

  handle = luaL_checkstring (L, 1);
  if (!lua_istable(L, 2)) {
ets_printf("no table!!\n");
    return 1;
  }
  numEntries = 0;
  lua_pushnil(L);  /* first key */
  while(lua_next(L, 2)) {
    lua_pop(L, 1);  /* remove value */
    numEntries++;
  }
  idx = 0;
  listVector = os_zalloc(numEntries * sizeof(uint8_t *) + 1);
  while (idx < numEntries) {
    os_sprintf(buf, "%d", idx+1);
    listVector[idx] = parseKey(L, 2, buf);
    idx++;
  }
  flags = luaL_checkinteger (L, 3);
  result = structmsgGetPtrFromHandle(handle, &structmsgData);
  if (result != STRUCT_MSG_ERR_OK) {
    os_free(listVector);
  }
  checkOKOrErr(L, result, "setMsgFieldsFromList", "");
  result = structmsgData->setMsgFieldsFromList(structmsgData, listVector, numEntries, flags);
  // in every case free listVector here as we don't need that memory any more
  os_free(listVector);
  checkOKOrErr(L, result, "setMsgFieldsFromList", "");
  return 1;
}

#ifdef NOTDEF
// ============================= structmsg_encrypt ========================

static int structmsg_encrypt( lua_State* L ) {
  return structmsg_encdec (L, true);
}

// ============================= structmsg_decrypt ========================

static int structmsg_decrypt( lua_State* L ) {
  return structmsg_encdec (L, false);
}

// ============================= structmsg_set_crypted ========================

static int structmsg_set_crypted( lua_State* L ) {
  const char *handle;
  const uint8_t *fieldName;
  int cryptedLen;
  const uint8_t *crypted;
  int result;

  handle = luaL_checkstring( L, 1 );
  crypted = luaL_checklstring( L, 2, &cryptedLen );
  result = stmsg_setCrypted(handle, crypted, cryptedLen);
  checkOKOrErr(L, result, "setcrypted", "");
  return 1;
}

// ============================= structmsg_decrypt_getHandle ========================

static int structmsg_decrypt_getHandle( lua_State* L ) {
  const uint8_t *encryptedMsg;
  size_t mlen;
  const uint8_t *key;
  size_t klen;
  const uint8_t *iv;
  size_t ivlen;
  uint8_t *handle;
  int result;

  encryptedMsg = luaL_checklstring( L, 1, &mlen );
  key = luaL_checklstring (L, 2, &klen);
  iv = luaL_optlstring (L, 3, "", &ivlen);
  result = stmsg_decryptGetHandle(encryptedMsg, mlen, key, klen, iv, ivlen, &handle);
  if (result == STRUCT_MSG_ERR_OK) {
    lua_pushstring (L, handle);
  } else {
    checkOKOrErr(L, result, "decryptgethandle", "");
  }
  return 1;
}
#endif

// ============================= structmsg_createMsgDefinition ========================

static int structmsg_createMsgDefinition( lua_State* L ) {
  const uint8_t *handle;
  uint8_t numFields;
  structmsgData_t *structmsgData;
  int result;

  handle = luaL_checkstring (L, 1);
  result = structmsgGetPtrFromHandle(handle, &structmsgData);
  checkOKOrErr(L, result, "createMsgDef", "");
  result = newStructmsgDefinition(structmsgData);
  checkOKOrErr(L, result, "createMsgDef", "");
  return 1;
}

// ============================= structmsg_initDef ========================

static int structmsg_initDef( lua_State* L ) {
  const uint8_t *handle;
  structmsgData_t *structmsgData;
  int result;

  handle = luaL_checkstring (L, 1);
  result = structmsgGetPtrFromHandle(handle, &structmsgData);
  checkOKOrErr(L, result, "initDef", "");
  result = structmsgData->initDef(structmsgData);
  checkOKOrErr(L, result, "initDef", "");
  return 1;
}

// ============================= structmsg_prepareDef ========================

static int structmsg_prepareDef( lua_State* L ) {
  const uint8_t *handle;
  uint8_t numFields;
  structmsgData_t *structmsgData;
  int result;

  handle = luaL_checkstring (L, 1);
  result = structmsgGetPtrFromHandle(handle, &structmsgData);
  checkOKOrErr(L, result, "prepareDef", "");
  result = structmsgData->prepareDef(structmsgData);
  checkOKOrErr(L, result, "prepareDef", "");
  return 1;
}

// ============================= structmsg_dumpDefFields ========================

static int structmsg_dumpDefFields( lua_State* L ) {
  const uint8_t *handle;
  int result;
  structmsgData_t *structmsgData;

  handle = luaL_checkstring( L, 1 );
  result = structmsgGetPtrFromHandle(handle, &structmsgData);
  checkOKOrErr(L, result, "dumpDef", "");
  result = structmsgData->dumpDefFields(structmsgData);
  checkOKOrErr(L, result, "dumpDef", "");
  return 1;
}

#ifdef NOTDEF
// ============================= structmsg_set_crypted_msgDefinition ========================

static int structmsg_set_crypted_msgDefinition( lua_State* L ) {
  const char *handle;
  const uint8_t *fieldName;
  int cryptedLen;
  const uint8_t *crypted;
  int result;

  handle = luaL_checkstring( L, 1 );
  crypted = luaL_checklstring( L, 2, &cryptedLen );
  result = stmsg_setCryptedDefinition(handle, crypted, cryptedLen);
  checkOKOrErr(L, result, "setcrypteddef", "");
  return 1;
}

// ============================= structmsg_decrypt_getDefinitionName ========================

static int structmsg_decrypt_getDefinitionName( lua_State* L ) {
  const uint8_t *encryptedMsg;
  size_t mlen;
  const uint8_t *key;
  size_t klen;
  const uint8_t *iv;
  size_t ivlen;
  uint8_t shortCmdKey;
  uint8_t *handle;
  int result;

  encryptedMsg = luaL_checklstring( L, 1, &mlen );
  key = luaL_checklstring (L, 2, &klen);
  iv = luaL_optlstring (L, 3, "", &ivlen);
  shortCmdKey = luaL_optinteger (L, 3, 0);
  result = stmsg_decryptGetDefinitionName(encryptedMsg, mlen, key, klen, iv, ivlen, &handle, shortCmdKey);
  if (result == STRUCT_MSG_ERR_OK) {
    lua_pushstring (L, handle);
  } else {
    checkOKOrErr(L, result, "decryptgetdefname", "");
  }
  return 1;
}

// ============================= structmsg_encrypt_msgDefinition ========================

static int structmsg_encrypt_msgDefinition( lua_State* L ) {
  return structmsg_encdec_msgDefinition (L, true);
}

// ============================= structmsg_decrypt_msgDefinition ========================

static int structmsg_decrypt_msgDefinition( lua_State* L ) {
  return structmsg_encdec_msgDefinition (L, false);
}

// ============================= structmsg_delete_msgDefinition ========================

static int structmsg_delete_msgDefinition( lua_State* L ) {
  const uint8_t *name;
  uint8_t numFields;
  int result;

  name = luaL_checkstring (L, 1);
  result = structmsg_deleteStructmsgDefinition(name);
  checkOKOrErr(L, result, "deletemsgdef", "");
  return 1;
}

// ============================= structmsg_delete_msgDefinitions ========================

static int structmsg_delete_msgDefinitions( lua_State* L ) {
  uint8_t numFields;
  int result;

  result = structmsg_deleteStructmsgDefinitions();
  checkOKOrErr(L, result, "deletemsgdefinitions", "");
  return 1;
}
#endif


// ============================= structmsg_setDef ========================

static int structmsg_setDef( lua_State* L ) {
  const uint8_t *handle;
  const uint8_t *data;
  structmsgData_t *structmsgData;
  int result;

  handle = luaL_checkstring (L, 1);
  data = luaL_checkstring (L, 2);
  result = structmsgGetPtrFromHandle(handle, &structmsgData);
  checkOKOrErr(L, result, "setDef", "");
  result = structmsgData->setDef(structmsgData, data);
  checkOKOrErr(L, result, "setDef", "");
  return 1;
}

// ============================= structmsg_getDef ========================

static int structmsg_getDef( lua_State* L ) {
  const uint8_t *handle;
  uint8_t *data;
  int lgth;
  structmsgData_t *structmsgData;
  int result;

  handle = luaL_checkstring (L, 1);
  result = structmsgGetPtrFromHandle(handle, &structmsgData);
  checkOKOrErr(L, result, "getDef", "");
  result = structmsgData->getDef(structmsgData, &data, &lgth);
  if (result == STRUCT_MSG_ERR_OK) {
    lua_pushlstring (L, data, lgth);
  } else {
    checkOKOrErr(L, result, "getDef", "");
  }
  return 1;
}

// ============================= structmsg_createMsgFromDef ========================

static int structmsg_createMsgFromDef( lua_State* L ) {
  const uint8_t *handle;
  structmsgData_t *structmsgData;
  int result;

  handle = luaL_checkstring (L, 1);
  result = structmsgGetPtrFromHandle(handle, &structmsgData);
  checkOKOrErr(L, result, "createMsgFromDef", "");
  result = structmsgData->createMsgFromDef(structmsgData);
  checkOKOrErr(L, result, "createMsgFromDef", "");
  return 1;
}

#ifdef NOTDEF
// ============================= structmsg_get_definitionNormalFieldNames ==================

static int structmsg_get_definitionNormalFieldNames( lua_State* L ) {
  const uint8_t *name;
  uint8_t **normalFieldNames;
  int result;

  name = luaL_checkstring (L, 1);
  result = structmsg_getDefinitionNormalFieldNames(name, &normalFieldNames);
  checkOKOrErr(L, result, "getdefnormfnames", "");
  return 1;
}

// ============================= structmsg_get_definitionTableFieldNames ==================

static int structmsg_get_definitionTableFieldNames( lua_State* L ) {
  const uint8_t *name;
  uint8_t **tableFieldNames;
  int result;

  name = luaL_checkstring (L, 1);
  result = structmsg_getDefinitionTableFieldNames(name, &tableFieldNames);
  checkOKOrErr(L, result, "getdeftablefnames", "");
  return 1;
}

// ============================= structmsg_get_definitionNumTableRows ====================

static int structmsg_get_definitionNumTableRows( lua_State* L ) {
  const uint8_t *name;
  uint8_t numTableRows;
  int result;

  name = luaL_checkstring (L, 1);
  result = structmsg_getDefinitionNumTableRows(name, &numTableRows);
  checkOKOrErr(L, result, "getdefntablerows", "");
  return 1;
}

// ============================= structmsg_get_definitionNumTableRowFields ===============

static int structmsg_get_definitionNumTableRowFields( lua_State* L ) {
  const uint8_t *name;
  uint8_t numTableRowFields;
  int result;

  name = luaL_checkstring (L, 1);
  result = structmsg_getDefinitionNumTableRowFields(name, &numTableRowFields);
  checkOKOrErr(L, result, "getdefntablerowfields", "");
  return 1;
}

// ============================= structmsg_get_definitionFieldInfo ======================

static int structmsg_get_definitionFieldInfo( lua_State* L ) {
  const uint8_t *name;
  const uint8_t *fieldName;
  fieldInfoDefinition_t *fieldInfo;
  int result;

  name = luaL_checkstring (L, 1);
  fieldName = luaL_checkstring (L, 2);
  result = structmsg_getDefinitionFieldInfo(name, fieldName, &fieldInfo);
  checkOKOrErr(L, result, "getdeffieldinfo", "");
  return 1;
}

// ============================= structmsg_get_definitionTableFieldInfo ==================

static int structmsg_get_definitionTableFieldInfo( lua_State* L ) {
  const uint8_t *name;
  const uint8_t *fieldName;
  fieldInfoDefinition_t *fieldInfo;
  int result;

  name = luaL_checkstring (L, 1);
  result = structmsg_getDefinitionTableFieldInfo(name, fieldName, &fieldInfo);
  checkOKOrErr(L, result, "getdeftablefieldinfo", "");
  return 1;
}

 
// ============================= structmsg_create_msgDefinitionFromListInfo ==================

// Lua: createmsgdeffromlist(list,numentries,name,numRows,flags)
static int structmsg_create_msDefinitiongFromListInfo( lua_State* L ) {
  const uint8_t *list;
  const uint8_t *name;
  uint8_t numRows;
  uint8_t shortCmdKey;
  uint8_t numEntries;
  uint16_t flags;
  const uint8_t **listVector;
  int idx;
  int result;
  char buf[10];
  const uint8_t *str;

  numEntries = luaL_checkinteger (L, 1);
  if (!lua_istable(L, 2)) {
ets_printf("no table!!\n");
    return 1;
  }
  idx = 0;
  listVector = os_zalloc(numEntries * sizeof(uint8_t *) + 1);
  while (idx < numEntries) {
    os_sprintf(buf, "%d", idx+1);
    listVector[idx] = parse_key(L, 2, buf);
    idx++;
  }
  name = luaL_checkstring (L, 3);
  numRows = luaL_checkinteger (L, 4);
  flags = luaL_checkinteger (L, 5);
  shortCmdKey = luaL_checkinteger (L, 6);
  result = structmsg_createMsgDefinitionFromListInfo(name, listVector, numEntries, numRows, flags, shortCmdKey);
  checkOKOrErr(L, result, "createmsgdeffromlist", "");
  return 1;
}
#endif

// ============================= structmsg_openFile ==================

// Lua: openFile(handle, name,flags)
static int structmsg_openFile( lua_State* L ) {
  int result;
  const uint8_t *handle;
  const uint8_t *fileName;
  const uint8_t *flags;
  structmsgDispatcher_t *structmsgDispatcher = NULL;

  handle = luaL_checkstring (L, 1);
  fileName = luaL_checkstring (L, 2);
  flags = luaL_checkstring (L, 3);
  result = structmsgDispatcherGetPtrFromHandle(handle, &structmsgDispatcher);
  checkOKOrErr(L, result, "openFile", "");
  result = structmsgDispatcher->openFile(structmsgDispatcher, fileName, flags);
  checkOKOrErr(L, result, "openFile", "");
  return 1;
}

// ============================= structmsg_closeFile ==================

// Lua: closeFile(handle)
static int structmsg_closeFile( lua_State* L ) {
  int result;
  const uint8_t *handle;
  structmsgDispatcher_t *structmsgDispatcher;

  handle = luaL_checkstring (L, 1);
  result = structmsgDispatcherGetPtrFromHandle(handle, &structmsgDispatcher);
  checkOKOrErr(L, result, "closeFile", "");
  result = structmsgDispatcher->closeFile(structmsgDispatcher);
  checkOKOrErr(L, result, "closeFile", "");
  return 1;
}

// ============================= structmsg_readLine ==================

// Lua: readLine(handle)
static int structmsg_readLine( lua_State* L ) {
  int result;
  const uint8_t *handle;
  uint8_t buffer[255];
  uint8_t *data = buffer;
  uint8_t lgth;
  structmsgDispatcher_t *structmsgDispatcher;

  handle = luaL_checkstring (L, 1);
  result = structmsgDispatcherGetPtrFromHandle(handle, &structmsgDispatcher);
  checkOKOrErr(L, result, "readLine", "");
  result = structmsgDispatcher->readLine(structmsgDispatcher, &data, &lgth);
  if (result == STRUCT_MSG_ERR_OK) {
    lua_pushlstring(L, data, lgth);
  } else {
    checkOKOrErr(L, result, "readLine", "");
  }
  return 1;
}

// ============================= structmsg_writeLine ==================

// Lua: writeLine(name,flags)
static int structmsg_writeLine( lua_State* L ) {
  int result;
  const uint8_t *handle;
  const uint8_t *buffer;
  uint8_t lgth;
  structmsgDispatcher_t *structmsgDispatcher;

  handle = luaL_checkstring (L, 1);
  buffer = luaL_checkstring (L, 2);
  lgth = c_strlen(buffer);
  result = structmsgDispatcherGetPtrFromHandle(handle, &structmsgDispatcher);
  checkOKOrErr(L, result, "writeLine", "");
  result = structmsgDispatcher->writeLine(structmsgDispatcher, buffer, lgth);
  checkOKOrErr(L, result, "writeLine", "");
  return 1;
}

// ============================= structmsg_createDispatcher ========================

static int structmsg_createDispatcher(lua_State* L)
{
  size_t numFieldInfos;
  uint8_t *handle = "??";
  structmsgDispatcher_t *structmsgDispatcher;
  int result;

  structmsgDispatcher = newStructmsgDispatcher();
  if (structmsgDispatcher == NULL) {
    checkOKOrErr(L, STRUCT_MSG_ERR_OUT_OF_MEMORY, "createDispatcher", "");
  } else {
    result = structmsgDispatcher->createDispatcher(structmsgDispatcher, &handle);
    if (checkOKOrErr(L, result, "createDispatcher", "")) {
      lua_pushstring(L, handle);
    }
  }
  return 1;
}

// ============================= structmsg_initDispatcher ========================

static int structmsg_initDispatcher(lua_State* L)
{
  const uint8_t *handle;
  structmsgDispatcher_t *structmsgDispatcher;
  int result;

  handle = luaL_checkstring (L, 1);
  result = structmsgDispatcherGetPtrFromHandle(handle, &structmsgDispatcher);
  checkOKOrErr(L, result, "initDispatcher", "");
  result = structmsgDispatcher->initDispatcher(structmsgDispatcher);
  checkOKOrErr(L, result, "initDispatcher", "");
  return 1;
}

// ============================= structmsg_uartReceiveCb ==================

// Lua: uartReceiveCb(name,flags)
static int structmsg_uartReceiveCb( lua_State* L ) {
  int result;
  const uint8_t *handle;
  const uint8_t *buffer;
  uint8_t lgth;
  structmsgDispatcher_t *structmsgDispatcher;

  handle = luaL_checkstring (L, 1);
  buffer = luaL_checkstring (L, 2);
  lgth = c_strlen(buffer);
  result = structmsgDispatcherGetPtrFromHandle(handle, &structmsgDispatcher);
  checkOKOrErr(L, result, "uartReceiveCb", "");
  result = structmsgDispatcher->uartReceiveCb(structmsgDispatcher, buffer, lgth);
  checkOKOrErr(L, result, "uartReceiveCbwriteLine", "");
  return 1;
}

// Module function map
// Module function map
static const LUA_REG_TYPE structmsg_map[] =  {
  { LSTRKEY( "createMsg" ),             LFUNCVAL( structmsg_createMsg ) },
  { LSTRKEY( "deleteMsg" ),             LFUNCVAL( structmsg_deleteMsg ) },
  { LSTRKEY( "__gc" ),                  LFUNCVAL( structmsg_deleteMsg ) },
  { LSTRKEY( "getMsgData" ),            LFUNCVAL( structmsg_getMsgData ) },
  { LSTRKEY( "setMsgData" ),            LFUNCVAL( structmsg_setMsgData ) },
  { LSTRKEY( "dumpMsg" ),               LFUNCVAL( structmsg_dumpMsg ) },
  { LSTRKEY( "initMsg" ),               LFUNCVAL( structmsg_initMsg ) },
  { LSTRKEY( "prepareMsg" ),            LFUNCVAL( structmsg_prepareMsg ) },
  { LSTRKEY( "addField" ),              LFUNCVAL( structmsg_addField ) },
  { LSTRKEY( "setFieldValue" ),         LFUNCVAL( structmsg_setFieldValue ) },
  { LSTRKEY( "setTableFieldValue" ),    LFUNCVAL( structmsg_setTableFieldValue ) },
  { LSTRKEY( "getFieldValue" ),         LFUNCVAL( structmsg_getFieldValue ) },
  { LSTRKEY( "getTableFieldValue" ),    LFUNCVAL( structmsg_getTableFieldValue ) },
  { LSTRKEY( "createMsgFromList" ),     LFUNCVAL( structmsg_createMsgFromList ) },
  { LSTRKEY( "setMsgFieldsFromList" ),  LFUNCVAL( structmsg_setMsgFieldsFromList ) },

  { LSTRKEY( "createMsgDef" ),          LFUNCVAL( structmsg_createMsgDefinition ) },
  { LSTRKEY( "initDef" ),               LFUNCVAL( structmsg_initDef ) },
  { LSTRKEY( "prepareDef" ),            LFUNCVAL( structmsg_prepareDef ) },
  { LSTRKEY( "dumpDefFields" ),         LFUNCVAL( structmsg_dumpDefFields ) },
  { LSTRKEY( "setDef" ),                LFUNCVAL( structmsg_setDef ) },
  { LSTRKEY( "getDef" ),                LFUNCVAL( structmsg_getDef ) },
  { LSTRKEY( "createMsgFromDef" ),      LFUNCVAL( structmsg_createMsgFromDef ) },
  { LSTRKEY( "openFile" ),              LFUNCVAL( structmsg_openFile ) },
  { LSTRKEY( "closeFile" ),             LFUNCVAL( structmsg_closeFile ) },
  { LSTRKEY( "readLine" ),              LFUNCVAL( structmsg_readLine ) },
  { LSTRKEY( "writeLine" ),             LFUNCVAL( structmsg_writeLine ) },

  { LSTRKEY( "createDispatcher" ),      LFUNCVAL( structmsg_createDispatcher ) },
  { LSTRKEY( "initDispatcher" ),        LFUNCVAL( structmsg_initDispatcher ) },
  { LSTRKEY( "uartReceiveCb" ),         LFUNCVAL( structmsg_uartReceiveCb ) },
#ifdef NOtDEF
  { LSTRKEY( "encrypt" ),               LFUNCVAL( structmsg_encrypt ) },
  { LSTRKEY( "decrypt" ),               LFUNCVAL( structmsg_decrypt ) },
  { LSTRKEY( "setcrypted" ),            LFUNCVAL( structmsg_set_crypted ) },
  { LSTRKEY( "decryptgethandle" ),      LFUNCVAL( structmsg_decrypt_getHandle ) },
  { LSTRKEY( "adddeffield" ),           LFUNCVAL( structmsg_add_fieldDefinition ) },
  { LSTRKEY( "encodedef" ),             LFUNCVAL( structmsg_encode_fieldDefinitionMessage ) },
  { LSTRKEY( "decodedef" ),             LFUNCVAL( structmsg_decode_fieldDefinitionMessage ) },
  { LSTRKEY( "encryptdef" ),            LFUNCVAL( structmsg_encrypt_msgDefinition ) },
  { LSTRKEY( "decryptdef" ),            LFUNCVAL( structmsg_decrypt_msgDefinition ) },
  { LSTRKEY( "setcrypteddef" ),         LFUNCVAL( structmsg_set_crypted_msgDefinition ) },
  { LSTRKEY( "decryptdefgetname" ),     LFUNCVAL( structmsg_decrypt_getDefinitionName ) },
  { LSTRKEY( "deletedef" ),             LFUNCVAL( structmsg_delete_msgDefinition ) },
  { LSTRKEY( "deletedefinitions" ),     LFUNCVAL( structmsg_delete_msgDefinitions ) },
  { LSTRKEY( "getdefnormfnames" ),      LFUNCVAL( structmsg_get_definitionNormalFieldNames ) },
  { LSTRKEY( "getdeftablefnames" ),     LFUNCVAL( structmsg_get_definitionTableFieldNames ) },
  { LSTRKEY( "getdefntablerows" ),      LFUNCVAL( structmsg_get_definitionNumTableRows ) },
  { LSTRKEY( "getdefntablerowfields" ), LFUNCVAL( structmsg_get_definitionNumTableRowFields ) },
  { LSTRKEY( "getdeffieldinfo" ),       LFUNCVAL( structmsg_get_definitionFieldInfo ) },
  { LSTRKEY( "getdeftablefieldinfo" ),  LFUNCVAL( structmsg_get_definitionTableFieldInfo ) },
  { LSTRKEY( "createmsgdeffromlist" ),  LFUNCVAL( structmsg_create_msDefinitiongFromListInfo ) },
#endif
  { LNILKEY, LNILVAL }
};

NODEMCU_MODULE(STRUCTMSG, "structmsg", structmsg_map, NULL);
