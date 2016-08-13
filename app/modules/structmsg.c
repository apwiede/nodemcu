/*
* Copyright (c) 2016, Arnulf Wiedemann
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
#include "rom.h"
#include "structmsg.h"
#include "osapi.h"
#define MEMLEAK_DEBUG 1
#include "mem.h"

static lua_State *gL = NULL;

// ============================= checkErrOK ========================

static int checkErrOK( lua_State* L, int result, const uint8_t *where, const uint8_t *fieldName ) {
  if (result == STRUCT_MSG_ERR_OK) {
    return 1;
  }
  switch (result) {
  case STRUCT_MSG_ERR_VALUE_NOT_SET:
    luaL_error(L, "%s: value for field: %s not set", where, fieldName);
    break;
  case STRUCT_MSG_ERR_VALUE_OUT_OF_RANGE:
    luaL_error(L, "%s: field: '%s' value out of range", where, fieldName);
    break;
  case STRUCT_MSG_ERR_BAD_VALUE:
    luaL_error(L, "%s: field: '%s' bad value", where, fieldName);
    break;
  case STRUCT_MSG_ERR_BAD_FIELD_TYPE:
    luaL_error(L, "%s: field: '%s' bad field type", where, fieldName);
    break;
  case STRUCT_MSG_ERR_FIELD_NOT_FOUND:
    luaL_error(L, "%s: field: '%s' not found", where, fieldName);
    break;
  case STRUCT_MSG_ERR_BAD_HANDLE:
    luaL_error(L, "bad handle");
    break;
  case STRUCT_MSG_ERR_OUT_OF_MEMORY:
    luaL_error(L, "out of memory");
    break;
  case STRUCT_MSG_ERR_HANDLE_NOT_FOUND:
    luaL_error(L, "handle not found");
    break;
  case STRUCT_MSG_ERR_NOT_ENCODED:
    luaL_error(L, "not encoded");
    break;
  case STRUCT_MSG_ERR_DECODE_ERROR:
    luaL_error(L, "decode error");
    break;
  case STRUCT_MSG_ERR_BAD_CRC_VALUE:
    luaL_error(L, "bad crc val");
    break;
  case STRUCT_MSG_ERR_CRYPTO_INIT_FAILED:
    luaL_error(L, "crypto init failed");
    break;
  case STRUCT_MSG_ERR_CRYPTO_OP_FAILED:
    luaL_error(L, "crypto op failed");
    break;
  case STRUCT_MSG_ERR_CRYPTO_BAD_MECHANISM:
    luaL_error(L, "crypto bad mechanism");
    break;
  }
  return 0;
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
  result = setFillerAndCrc ( handle );
  checkErrOK(L, result, "setFillerAndCrc", "");
  return 1;
}


// ============================= structmsg_set_targets ========================

static int structmsg_set_targets( lua_State* L )
{
  uint16_t src;
  uint16_t dst;
  uint16_t cmd;
  const uint8_t *handle;
  int result;

  handle = luaL_checkstring( L, 1 );
  src = luaL_checkinteger( L, 2 );
  dst = luaL_checkinteger( L, 3 );
  cmd = luaL_checkinteger( L, 4 );
  result = set_targets ( handle, src, dst, cmd );
  checkErrOK(L, result, "setTargets", "");
  return 1;
}

// ============================= structmsg_new ========================

static int structmsg_new( lua_State* L )
{
  uint8_t numFieldInfos;
  uint8_t *handle;
  int result;

  numFieldInfos = luaL_checkinteger( L, 1 );
  result = new_structmsg ( numFieldInfos, &handle);
  if (checkErrOK(L, result, "new", "")) {
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
  result = delete_structmsg(handle);
  checkErrOK(L, result, "delete", "");
  return 1;
}

// ============================= structmsg_dump_structmsg ========================

static int structmsg_dump_structmsg( lua_State* L )
{
  const char *handle;
  int result;

  handle = luaL_checkstring( L, 1 );
  result = dump_structmsg(handle);
  checkErrOK(L, result, "dump", "");
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
  result = setFieldValue(handle, fieldName, numericValue, stringValue);
  checkErrOK(L, result, "setFieldValue", fieldName);
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
  result = encdec(handle, key, klen, iv, ivlen, enc, &buf, &lgth);
  if (result == STRUCT_MSG_ERR_OK) {
    lua_pushlstring (L, buf, lgth);
  }
  checkErrOK(L, result, "encrypt/decrypt", "");
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
  fieldType = getFieldTypeKey(fieldTypeStr);
  if (fieldType == STRUCT_MSG_FIELD_UINT8_VECTOR) {
      fieldLgth = luaL_checkinteger( L, 4 );
  } else {
      fieldLgth = 1;
  }
  result = addField(handle, fieldStr, fieldType, fieldLgth);
  checkErrOK(L, result, "addField", "");
  return 1;
}

// ============================= structmsg_encode ========================

static int structmsg_encode( lua_State* L ) {
  const char *handle;
  int result;

  handle = luaL_checkstring( L, 1 );
  result = encodeMsg(handle);
  checkErrOK(L, result, "encode", "");
  return 1;
}

// ============================= structmsg_get_encoded ========================

static int structmsg_get_encoded( lua_State* L ) {
  const char *handle;
  uint8_t *encoded;
  int lgth;
  int result;

  handle = luaL_checkstring( L, 1 );
  result = getEncoded(handle, &encoded, &lgth);
  if (checkErrOK(L, result, "getencoded", "")) {
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
  result = decodeMsg(handle, data);
  checkErrOK(L, result, "decode", "");
  return 1;
}

// Module function map
static const LUA_REG_TYPE structmsg_map[] =  {
  { LSTRKEY( "create" ),          LFUNCVAL( structmsg_new ) },
  { LSTRKEY( "delete" ),          LFUNCVAL( structmsg_delete ) },
  { LSTRKEY( "__gc" ),            LFUNCVAL( structmsg_delete ) },
  { LSTRKEY( "encode" ),          LFUNCVAL( structmsg_encode ) },
  { LSTRKEY( "getencoded" ),      LFUNCVAL( structmsg_get_encoded ) },
  { LSTRKEY( "decode" ),          LFUNCVAL( structmsg_decode ) },
  { LSTRKEY( "dump" ),            LFUNCVAL( structmsg_dump_structmsg ) },
  { LSTRKEY( "encrypt" ),         LFUNCVAL( structmsg_encrypt ) },
  { LSTRKEY( "decrypt" ),         LFUNCVAL( structmsg_decrypt ) },
  { LSTRKEY( "addField" ),        LFUNCVAL( structmsg_add_field ) },
  { LSTRKEY( "setTargets" ),      LFUNCVAL( structmsg_set_targets ) },
  { LSTRKEY( "setFillerAndCrc" ), LFUNCVAL( structmsg_set_fillerAndCrc ) },
  { LSTRKEY( "setFieldValue" ),   LFUNCVAL( structmsg_set_fieldValue ) },
  { LNILKEY, LNILVAL }
};

NODEMCU_MODULE(STRUCT_MSG, "structmsg", structmsg_map, NULL);
