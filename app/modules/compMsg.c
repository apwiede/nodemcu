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

// Module for handling compMsgs

#include "module.h"
#include "lauxlib.h"
#include "platform.h"

#include "c_types.h"
#include "c_string.h"
#include "c_stdarg.h"
#include "rom.h"
#include "../compMsg/compMsgDispatcher.h"
#include "osapi.h"
#define MEMLEAK_DEBUG 1
#include "mem.h"

static lua_State *gL = NULL;
static const uint8_t *errStr;

// ============================= compMsg_error ========================

static int compMsg_error( lua_State* L, const char *fmt, ... ) {
  va_list argp;

  va_start(argp, fmt);
  lua_pushfstring(L, fmt, argp );
  va_end(argp);
  return 0;
}

// ============================= checkOKOrErr ========================

static int checkOKOrErr( lua_State* L, int result, const uint8_t *where, const uint8_t *fieldName ) {
  if (result == COMP_MSG_ERR_OK) {
    lua_pushfstring(L, "OK");
    return 1;
  }
ets_printf("%error result: %d\n%", result);
  errStr = "ERROR";
  switch (result) {
  case COMP_MSG_ERR_VALUE_NOT_SET:
    lua_pushfstring(L, "%s: %s: value for field: %s not set", errStr, where, fieldName);
    break;
  case COMP_MSG_ERR_VALUE_OUT_OF_RANGE:
    lua_pushfstring(L, "%s: %s: field: '%s' value out of range", errStr, where, fieldName);
    break;
  case COMP_MSG_ERR_BAD_VALUE:
    lua_pushfstring(L, "%s: %s: field: '%s' bad value", errStr, where, fieldName);
    break;
  case COMP_MSG_ERR_BAD_FIELD_TYPE:
    lua_pushfstring(L, "%s: %s: field: '%s' bad field type", errStr, where, fieldName);
    break;
  case COMP_MSG_ERR_FIELD_NOT_FOUND:
    lua_pushfstring(L, "%s: %s: field: '%s' not found", errStr, where, fieldName);
    break;
  case COMP_MSG_ERR_BAD_HANDLE:
    lua_pushfstring(L, "%s: bad handle", errStr);
    break;
  case COMP_MSG_ERR_OUT_OF_MEMORY:
    luaL_error(L, "out of memory");
    break;
  case COMP_MSG_ERR_HANDLE_NOT_FOUND:
    lua_pushfstring(L, "%s: handle not found", errStr);
    break;
  case COMP_MSG_ERR_NOT_ENCODED:
    lua_pushfstring(L, "%s: not encoded", errStr);
    break;
  case COMP_MSG_ERR_DECODE_ERROR:
    lua_pushfstring(L, "%s: decode error", errStr);
    break;
  case COMP_MSG_ERR_BAD_CRC_VALUE:
    lua_pushfstring(L, "%s: bad crc val", errStr);
    break;
  case COMP_MSG_ERR_CRYPTO_INIT_FAILED:
    lua_pushfstring(L, "%s: crypto init failed", errStr);
    break;
  case COMP_MSG_ERR_CRYPTO_OP_FAILED:
    lua_pushfstring(L, "%s: crypto op failed", errStr);
    break;
  case COMP_MSG_ERR_CRYPTO_BAD_MECHANISM:
    lua_pushfstring(L, "%s: crypto bad mechanism", errStr);
    break;
  case COMP_MSG_ERR_NOT_ENCRYPTED:
    lua_pushfstring(L, "%s: not encrypted", errStr);
    break;
  case COMP_MSG_ERR_DEFINITION_NOT_FOUND:
    lua_pushfstring(L, "%s: definiton not found", errStr);
    break;
  case COMP_MSG_ERR_BAD_SPECIAL_FIELD:
    lua_pushfstring(L, "%s: bad special field", errStr);
    break;
  case COMP_MSG_ERR_DEFINITION_TOO_MANY_FIELDS:
    lua_pushfstring(L, "%s: definition too many fields", errStr);
    break;
  case COMP_MSG_ERR_BAD_TABLE_ROW:
    lua_pushfstring(L, "%s: bad table row", errStr);
    break;
  case COMP_MSG_ERR_TOO_MANY_FIELDS:
    lua_pushfstring(L, "%s: too many fields", errStr);
    break;
  case COMP_MSG_ERR_BAD_DEFINTION_CMD_KEY:
    lua_pushfstring(L, "%s: bad definition cmd key", errStr);
    break;
  case COMP_MSG_ERR_NO_SLOT_FOUND:
    lua_pushfstring(L, "%s: no slot found", errStr);
    break;
  case COMP_MSG_ERR_BAD_NUM_FIELDS:
    lua_pushfstring(L, "%s: bad num fields", errStr);
    break;
  case COMP_MSG_ERR_ALREADY_INITTED:
    lua_pushfstring(L, "%s: already initted", errStr);
    break;
  case COMP_MSG_ERR_NOT_YET_INITTED:
    lua_pushfstring(L, "%s: not yet initted", errStr);
    break;
  case COMP_MSG_ERR_FIELD_CANNOT_BE_SET:
    lua_pushfstring(L, "%s: field cannot be set", errStr);
    break;
  case COMP_MSG_ERR_NO_SUCH_FIELD:
    lua_pushfstring(L, "%s: no such field", errStr);
    break;
  case COMP_MSG_ERR_BAD_DATA_LGTH:
    lua_pushfstring(L, "%s: bad data length", errStr);
    break;
  case COMP_MSG_ERR_NOT_YET_PREPARED:
    lua_pushfstring(L, "%s: not yet prepared", errStr);
    break;
  case COMP_MSG_ERR_OUT_OF_RANGE:
    lua_pushfstring(L, "%s: out of range", errStr);
    break;
  case COMP_MSG_ERR_FIELD_TOTAL_LGTH_MISSING:
    lua_pushfstring(L, "%s: field @totalLgth missing", errStr);
    break;
  default:
    lua_pushfstring(L, "%s: funny result error code", errStr);
ets_printf("funny result: %d\n", result);
    break;
  }
  return 1;
}

// ============================= compMsg_createDispatcher ========================

static int compMsg_createDispatcher(lua_State* L)
{
  size_t numFieldInfos;
  uint8_t *handle = "??";
  compMsgDispatcher_t *compMsgDispatcher;
  int result;

  compMsgDispatcher = newCompMsgDispatcher();
  if (compMsgDispatcher == NULL) {
    checkOKOrErr(L, COMP_MSG_ERR_OUT_OF_MEMORY, "createDispatcher", "");
  } else {
    result = compMsgDispatcher->createDispatcher(compMsgDispatcher, &handle);
    if (checkOKOrErr(L, result, "createDispatcher", "")) {
      lua_pushstring(L, handle);
    }
  }
  return 1;
}

// ============================= compMsg_initDispatcher ========================

static int compMsg_initDispatcher(lua_State* L)
{
  const uint8_t *handle;
  const uint8_t *type;
  size_t typelen;
  compMsgDispatcher_t *compMsgDispatcher;
  int result;

  handle = luaL_checkstring (L, 1);
  type = luaL_optlstring (L, 2, "", &typelen);
  result = compMsgDispatcherGetPtrFromHandle(handle, &compMsgDispatcher);
  checkOKOrErr(L, result, "initDispatcher", "");
  result = compMsgDispatcher->initDispatcher(compMsgDispatcher, type, typelen);
  checkOKOrErr(L, result, "initDispatcher", "");
  return 1;
}

// ============================= compMsg_uartReceiveCb ==================

// Lua: uartReceiveCb(name,flags)
static int compMsg_uartReceiveCb( lua_State* L ) {
  int result;
  const uint8_t *handle;
  const uint8_t *buffer;
  uint8_t lgth;
  compMsgDispatcher_t *compMsgDispatcher;

  handle = luaL_checkstring (L, 1);
  buffer = luaL_checkstring (L, 2);
  lgth = c_strlen(buffer);
  result = compMsgDispatcherGetPtrFromHandle(handle, &compMsgDispatcher);
  checkOKOrErr(L, result, "uartReceiveCb", "");
  result = compMsgDispatcher->compMsgSendReceive->uartReceiveCb(compMsgDispatcher, buffer, lgth);
  checkOKOrErr(L, result, "uartReceiveCbwriteLine", "");
  return 1;
}

// Module function map
// Module function map
static const LUA_REG_TYPE compMsg_map[] =  {
  { LSTRKEY( "createDispatcher" ),      LFUNCVAL( compMsg_createDispatcher ) },
  { LSTRKEY( "initDispatcher" ),        LFUNCVAL( compMsg_initDispatcher ) },
  { LSTRKEY( "uartReceiveCb" ),         LFUNCVAL( compMsg_uartReceiveCb ) },
  { LNILKEY, LNILVAL }
};

NODEMCU_MODULE(COMPMSG, "compmsg", compMsg_map, NULL);
