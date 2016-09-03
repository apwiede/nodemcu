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
#include "structmsg.h"
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
ets_printf("error result: %d\n", result);
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
  default:
    lua_pushfstring(L, "%s: funny result error code", errStr);
ets_printf("funny result: %d\n", result);
    break;
  }
  return 1;
}

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

// ============================= structmsg_create ========================

static int structmsg_create( lua_State* L )
{
  uint8_t numFieldInfos;
  uint8_t *handle;
  int result;

  numFieldInfos = luaL_checkinteger( L, 1 );
  result = stmsg_createMsg ( numFieldInfos, &handle);
  if (checkOKOrErr(L, result, "new", "")) {
    lua_pushstring( L, handle );
  }
  return 1;
}

// ============================= structmsg_delete ========================

static int structmsg_delete( lua_State* L )
{
  const char *handle;
  int result;

ets_printf("structmsg_delete called\n");
  handle = luaL_checkstring( L, 1 );
  result = stmsg_deleteMsg(handle);
  checkOKOrErr(L, result, "delete", "");
  return 1;
}

// ============================= structmsg_encode ========================

static int structmsg_encode( lua_State* L ) {
  const char *handle;
  int result;

  handle = luaL_checkstring( L, 1 );
  result = stmsg_encodeMsg(handle);
  checkOKOrErr(L, result, "encode", "");
  return 1;
}

// ============================= structmsg_get_encoded ========================

static int structmsg_get_encoded( lua_State* L ) {
  const char *handle;
  uint8_t *encoded;
  int lgth;
  int result;

  handle = luaL_checkstring( L, 1 );
  result = stmsg_getEncoded(handle, &encoded, &lgth);
  if (checkOKOrErr(L, result, "getencoded", "")) {
    lua_pushlstring(L, encoded, lgth);
  }
  return 1;
}

// ============================= structmsg_decode ========================

static int structmsg_decode( lua_State* L ) {
  const char *handle;
  int result;
  const uint8_t *data;

  handle = luaL_checkstring( L, 1 );
  data = luaL_checkstring( L, 2 );
  result = stmsg_decodeMsg(handle, data);
  checkOKOrErr(L, result, "decode", "");
  return 1;
}

// ============================= structmsg_dump ========================

static int structmsg_dump( lua_State* L )
{
  const char *handle;
  int result;

  handle = luaL_checkstring( L, 1 );
  result = stmsg_dumpMsg(handle);
  checkOKOrErr(L, result, "dump", "");
  return 1;
}

// ============================= structmsg_encrypt ========================

static int structmsg_encrypt( lua_State* L ) {
  return structmsg_encdec (L, true);
}

// ============================= structmsg_decrypt ========================

static int structmsg_decrypt( lua_State* L ) {
  return structmsg_encdec (L, false);
}

// ============================= structmsg_add_field ========================

static int structmsg_add_field( lua_State* L ) {
  const char *handle;
  const uint8_t *fieldTypeStr;
  const uint8_t *fieldStr;
  uint8_t fieldType;
  uint8_t fieldLgth;
  int result;

  handle = luaL_checkstring( L, 1 );
  fieldStr = luaL_checkstring( L, 2 );
  fieldTypeStr = luaL_checkstring( L, 3 );
  fieldType = structmsg_getFieldTypeKey(fieldTypeStr);
  fieldLgth = luaL_optinteger( L, 4, 1 );
  result = stmsg_addField(handle, fieldStr, fieldType, fieldLgth);
  checkOKOrErr(L, result, "addField", "");
  return 1;
}

// ============================= structmsg_set_fillerAndCrc ========================

static int structmsg_set_fillerAndCrc( lua_State* L )
{
  uint16_t src;
  uint16_t dst;
  uint16_t cmd;
  const uint8_t *handle;
  int result;

  handle = luaL_checkstring( L, 1 );
  result = stmsg_setFillerAndCrc ( handle );
  checkOKOrErr(L, result, "setFillerAndCrc", "");
  return 1;
}

// ============================= structmsg_set_fieldValue ========================

static int structmsg_set_fieldValue( lua_State* L ) {
  const char *handle;
  const uint8_t *fieldName;
  int numericValue;
  const uint8_t *stringValue;
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
  result = stmsg_setFieldValue(handle, fieldName, numericValue, stringValue);
  checkOKOrErr(L, result, "setFieldValue", fieldName);
  return 1;
}

// ============================= structmsg_set_tableFieldValue ========================

static int structmsg_set_tableFieldValue( lua_State* L ) {
  const char *handle;
  const uint8_t *fieldName;
  int row;
  int numericValue;
  const uint8_t *stringValue;
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
  result = stmsg_setTableFieldValue(handle, fieldName, row, numericValue, stringValue);
  checkOKOrErr(L, result, "setFieldValue", fieldName);
  return 1;
}

// ============================= structmsg_get_fieldValue ========================

static int structmsg_get_fieldValue( lua_State* L ) {
  const char *handle;
  const uint8_t *fieldName;
  int numericValue = 0;
  uint8_t *stringValue = NULL;
  int result;

  handle = luaL_checkstring( L, 1 );
  fieldName = luaL_checkstring( L, 2 );
  result = stmsg_getFieldValue(handle, fieldName, &numericValue, &stringValue);
  checkOKOrErr(L, result, "getFieldValue", fieldName);
  if (stringValue == NULL) {
    lua_pushinteger(L, numericValue);
  } else {
    lua_pushstring(L, stringValue);
  }
  return 1;
}

// ============================= structmsg_get_tableFieldValue ========================

static int structmsg_get_tableFieldValue( lua_State* L ) {
  const char *handle;
  const uint8_t *fieldName;
  int numericValue = 0;
  int row;
  uint8_t *stringValue = NULL;
  int result;

  handle = luaL_checkstring( L, 1 );
  fieldName = luaL_checkstring( L, 2 );
  row = luaL_checkinteger( L, 3 );
  result = stmsg_getTableFieldValue(handle, fieldName, row, &numericValue, &stringValue);
  checkOKOrErr(L, result, "getFieldValue", fieldName);
  if (stringValue == NULL) {
    lua_pushinteger(L, numericValue);
  } else {
    lua_pushstring(L, stringValue);
  }
  return 1;
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

// ============================= structmsg_create_msgDefinition ========================

static int structmsg_create_msgDefinition( lua_State* L ) {
  const uint8_t *name;
  uint8_t numFields;
  int result;

  name = luaL_checkstring (L, 1);
  numFields = lua_tointeger (L, 2);
  result = structmsg_createStructmsgDefinition(name, numFields);
  checkOKOrErr(L, result, "createmsgdef", "");
  return 1;
}

// ============================= structmsg_add_fieldDefinition ========================

static int structmsg_add_fieldDefinition( lua_State* L ) {
  const uint8_t *name;
  const uint8_t *fieldName;
  const uint8_t *fieldType;
  uint8_t fieldLgth;
  int result;

  name = luaL_checkstring( L, 1 );
  fieldName = luaL_checkstring (L, 2);
  fieldType = luaL_checkstring (L, 3);
  fieldLgth = luaL_checkinteger (L, 4);
  result = structmsg_addFieldDefinition(name, fieldName, fieldType, fieldLgth);
  checkOKOrErr(L, result, "addfielddef", "");
  return 1;
}

// ============================= structmsg_dump_fieldDefinition ========================

static int structmsg_dump_fieldDefinition( lua_State* L ) {
  const uint8_t *name;
  int result;

  name = luaL_checkstring( L, 1 );
  result = structmsg_dumpFieldDefinition(name);
  checkOKOrErr(L, result, "dumpfielddef", "");
  return 1;
}

// ============================= structmsg_encode_fieldDefinitionMessage =================

static int structmsg_encode_fieldDefinitionMessage( lua_State* L ) {
  const uint8_t *name;
  uint8_t *data;
  int lgth;
  int result;

  name = luaL_checkstring( L, 1 );
  result =  structmsg_encodeFieldDefinitionMessage (name, &data, &lgth);
  if (result == STRUCT_MSG_ERR_OK) {
    lua_pushlstring (L, data, lgth);
  } else {
    checkOKOrErr(L, result, "encodefielddefmsg", "");
  }
  return 1;
}

// ============================= structmsg_decode_fieldDefinitionMessage =================

static int structmsg_decode_fieldDefinitionMessage( lua_State* L ) {
  const uint8_t *name;
  const uint8_t *encoded;
  int result;

  name = luaL_checkstring( L, 1 );
  encoded = luaL_checkstring( L, 2 );
  result =  structmsg_decodeFieldDefinitionMessage (name, encoded);
  checkOKOrErr(L, result, "decodefielddefmsg", "");
  return 1;
}

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
  uint8_t *handle;
  int result;

  encryptedMsg = luaL_checklstring( L, 1, &mlen );
  key = luaL_checklstring (L, 2, &klen);
  iv = luaL_optlstring (L, 3, "", &ivlen);
  result = stmsg_decryptGetDefinitionName(encryptedMsg, mlen, key, klen, iv, ivlen, &handle);
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

// ============================= structmsg_delete_msgDefinition2 ========================

static int structmsg_delete_msgDefinitions( lua_State* L ) {
  uint8_t numFields;
  int result;

  result = structmsg_deleteStructmsgDefinitions();
  checkOKOrErr(L, result, "deletemsgdefinitions", "");
  return 1;
}


// ============================= structmsg_create_msgFromDefinition ========================

static int structmsg_create_msgFromDefinition( lua_State* L ) {
  const uint8_t *name;
  int result;

  name = luaL_checkstring (L, 1);
  result = structmsg_createMsgFromDefinition(name);
  checkOKOrErr(L, result, "createmsgfromdef", "");
  return 1;
}

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

// Module function map
static const LUA_REG_TYPE structmsg_map[] =  {
  { LSTRKEY( "create" ),                LFUNCVAL( structmsg_create ) },
  { LSTRKEY( "delete" ),                LFUNCVAL( structmsg_delete ) },
  { LSTRKEY( "__gc" ),                  LFUNCVAL( structmsg_delete ) },
  { LSTRKEY( "encode" ),                LFUNCVAL( structmsg_encode ) },
  { LSTRKEY( "getencoded" ),            LFUNCVAL( structmsg_get_encoded ) },
  { LSTRKEY( "decode" ),                LFUNCVAL( structmsg_decode ) },
  { LSTRKEY( "dump" ),                  LFUNCVAL( structmsg_dump ) },
  { LSTRKEY( "encrypt" ),               LFUNCVAL( structmsg_encrypt ) },
  { LSTRKEY( "decrypt" ),               LFUNCVAL( structmsg_decrypt ) },
  { LSTRKEY( "addField" ),              LFUNCVAL( structmsg_add_field ) },
  { LSTRKEY( "setFillerAndCrc" ),       LFUNCVAL( structmsg_set_fillerAndCrc ) },
  { LSTRKEY( "setFieldValue" ),         LFUNCVAL( structmsg_set_fieldValue ) },
  { LSTRKEY( "setTableFieldValue" ),    LFUNCVAL( structmsg_set_tableFieldValue ) },
  { LSTRKEY( "getFieldValue" ),         LFUNCVAL( structmsg_get_fieldValue ) },
  { LSTRKEY( "getTableFieldValue" ),    LFUNCVAL( structmsg_get_tableFieldValue ) },
  { LSTRKEY( "setcrypted" ),            LFUNCVAL( structmsg_set_crypted ) },
  { LSTRKEY( "decryptgethandle" ),      LFUNCVAL( structmsg_decrypt_getHandle ) },
  { LSTRKEY( "createdef" ),             LFUNCVAL( structmsg_create_msgDefinition ) },
  { LSTRKEY( "adddeffield" ),           LFUNCVAL( structmsg_add_fieldDefinition ) },
  { LSTRKEY( "dumpdef" ),               LFUNCVAL( structmsg_dump_fieldDefinition ) },
  { LSTRKEY( "encodedef" ),             LFUNCVAL( structmsg_encode_fieldDefinitionMessage ) },
  { LSTRKEY( "decodedef" ),             LFUNCVAL( structmsg_decode_fieldDefinitionMessage ) },
  { LSTRKEY( "encryptdef" ),            LFUNCVAL( structmsg_encrypt_msgDefinition ) },
  { LSTRKEY( "decryptdef" ),            LFUNCVAL( structmsg_decrypt_msgDefinition ) },
  { LSTRKEY( "setcrypteddef" ),         LFUNCVAL( structmsg_set_crypted_msgDefinition ) },
  { LSTRKEY( "decryptdefgetname" ),     LFUNCVAL( structmsg_decrypt_getDefinitionName ) },
  { LSTRKEY( "deletedef" ),             LFUNCVAL( structmsg_delete_msgDefinition ) },
  { LSTRKEY( "deletedefinitions" ),     LFUNCVAL( structmsg_delete_msgDefinitions ) },
  { LSTRKEY( "createmsgfromdef" ),      LFUNCVAL( structmsg_create_msgFromDefinition ) },
  { LSTRKEY( "getdefnormfnames" ),      LFUNCVAL( structmsg_get_definitionNormalFieldNames ) },
  { LSTRKEY( "getdeftablefnames" ),     LFUNCVAL( structmsg_get_definitionTableFieldNames ) },
  { LSTRKEY( "getdefntablerows" ),      LFUNCVAL( structmsg_get_definitionNumTableRows ) },
  { LSTRKEY( "getdefntablerowfields" ), LFUNCVAL( structmsg_get_definitionNumTableRowFields ) },
  { LSTRKEY( "getdeffieldinfo" ),       LFUNCVAL( structmsg_get_definitionFieldInfo ) },
  { LSTRKEY( "getdeftablefieldinfo" ),  LFUNCVAL( structmsg_get_definitionTableFieldInfo ) },
  { LNILKEY, LNILVAL }
};

NODEMCU_MODULE(STRUCTMSG, "structmsg", structmsg_map, NULL);
