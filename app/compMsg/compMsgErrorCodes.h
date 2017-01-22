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
 * File:   compMsgErrorCodes.h
 * Author: Arnulf P. Wiedemann
 *
 * Created on Descmber 16, 2016
 */

#ifndef COMP_MSG_ERROR_CODES_H
#define	COMP_MSG_ERROR_CODES_H

#include "c_types.h"
#ifdef	__cplusplus
extern "C" {
#endif

enum CompMsgErrorCodes {
  COMP_MSG_ERR_OK                             = 0,
  COMP_MSG_ERR_VALUE_NOT_SET                  = 255,
  COMP_MSG_ERR_VALUE_OUT_OF_RANGE             = 254,
  COMP_MSG_ERR_BAD_VALUE                      = 253,
  COMP_MSG_ERR_BAD_FIELD_TYPE                 = 252,
  COMP_MSG_ERR_FIELD_TYPE_NOT_FOUND           = 251,
  COMP_MSG_ERR_VALUE_TOO_BIG                  = 250,
  COMP_MSG_ERR_OUT_OF_MEMORY                  = 249,
  COMP_MSG_ERR_OUT_OF_RANGE                   = 248,
  // be carefull the values up to here
  // must correspond to the values in dataView.h !!!
  // with the names like DATA_VIEW_ERR_*

  COMP_MSG_ERR_FIELD_NOT_FOUND                = 230,
  COMP_MSG_ERR_BAD_SPECIAL_FIELD              = 229,
  COMP_MSG_ERR_BAD_HANDLE                     = 228,
  COMP_MSG_ERR_HANDLE_NOT_FOUND               = 227,
  COMP_MSG_ERR_NOT_ENCODED                    = 226,
  COMP_MSG_ERR_ENCODE_ERROR                   = 225,
  COMP_MSG_ERR_DECODE_ERROR                   = 224,
  COMP_MSG_ERR_BAD_CRC_VALUE                  = 223,
  COMP_MSG_ERR_CRYPTO_INIT_FAILED             = 222,
  COMP_MSG_ERR_CRYPTO_OP_FAILED               = 221,
  COMP_MSG_ERR_CRYPTO_BAD_MECHANISM           = 220,
  COMP_MSG_ERR_NOT_ENCRYPTED                  = 219,
  COMP_MSG_ERR_DEFINITION_NOT_FOUND           = 218,
  COMP_MSG_ERR_DEFINITION_TOO_MANY_FIELDS     = 217,
  COMP_MSG_ERR_BAD_TABLE_ROW                  = 216,
  COMP_MSG_ERR_TOO_MANY_FIELDS                = 215,
  COMP_MSG_ERR_BAD_DEFINTION_CMD_KEY          = 214,
  COMP_MSG_ERR_NO_SLOT_FOUND                  = 213,
  COMP_MSG_ERR_BAD_NUM_FIELDS                 = 212,
  COMP_MSG_ERR_ALREADY_INITTED                = 211,
  COMP_MSG_ERR_NOT_YET_INITTED                = 210,
  COMP_MSG_ERR_FIELD_CANNOT_BE_SET            = 209,
  COMP_MSG_ERR_NO_SUCH_FIELD                  = 208,
  COMP_MSG_ERR_DUPLICATE_FIELD                = 207,
  COMP_MSG_ERR_BAD_DATA_LGTH                  = 206,
  COMP_MSG_ERR_NOT_YET_PREPARED               = 205,
  COMP_MSG_ERR_FIELD_TOTAL_LGTH_MISSING       = 200,

  COMP_MSG_ERR_OPEN_FILE                      = 189,
  COMP_MSG_ERR_FILE_NOT_OPENED                = 188,
  COMP_MSG_ERR_FLUSH_FILE                     = 187,
  COMP_MSG_ERR_WRITE_FILE                     = 186,
  COMP_MSG_ERR_FUNNY_EXTRA_FIELDS             = 185,
  COMP_MSG_ERR_FIELD_TOO_LONG                 = 184,
  COMP_MSG_ERR_BAD_MODULE_VALUE_WHICH         = 183,
  COMP_MSG_ERR_ALREADY_UPDATING               = 182,
  COMP_MSG_ERR_DNS_ERROR                      = 181,
  COMP_MSG_ERR_STATION_SCAN                   = 180,
  COMP_MSG_ERR_BAD_RECEIVED_LGTH              = 179,
  COMP_MSG_ERR_BAD_FILE_CONTENTS              = 178,
  COMP_MSG_ERR_HEADER_NOT_FOUND               = 177,
  COMP_MSG_ERR_BAD_FIELD_NAME                 = 175,
  COMP_MSG_ERR_BAD_HANDLE_TYPE                = 174,
  COMP_MSG_ERR_INVALID_BASE64_STRING          = 173,
  COMP_MSG_ERR_TOO_FEW_FILE_LINES             = 172,
  COMP_MSG_ERR_ACTION_NAME_NOT_FOUND          = 171,
  COMP_MSG_ERR_DUPLICATE_ENTRY                = 170,
  COMP_MSG_ERR_NO_WEBSOCKET_OPENED            = 169,
  COMP_MSG_ERR_TOO_MANY_REQUESTS              = 168,
  COMP_MSG_ERR_REQUEST_NOT_FOUND              = 167,
  COMP_MSG_ERR_UART_REQUEST_NOT_SET           = 166,
  COMP_MSG_ERR_FUNNY_HANDLE_TYPE              = 165,
  COMP_MSG_ERR_FIELD_VALUE_CALLBACK_NOT_FOUND = 164,
  COMP_MSG_ERR_CONNECT_STATION_WRONG_PASSWD   = 163,
  COMP_MSG_ERR_CONNECT_STATION_NO_AP_FOUND    = 162,
  COMP_MSG_ERR_CONNECT_STATION_CONNECT_FAILED = 161,
  COMP_MSG_ERR_CONNECT_STATION_CONNECTING     = 160,
  COMP_MSG_ERR_CONNECT_STATION_IDLE           = 159,
  COMP_MSG_ERR_ESPCONN_TIMEOUT                = 158,
  COMP_MSG_ERR_ESPCONN_RTE                    = 157,
  COMP_MSG_ERR_ESPCONN_INPROGRESS             = 156,
  COMP_MSG_ERR_ESPCONN_ABRT                   = 155,
  COMP_MSG_ERR_ESPCONN_RST                    = 154,
  COMP_MSG_ERR_ESPCONN_CLSD                   = 153,
  COMP_MSG_ERR_ESPCONN_CONN                   = 152,
  COMP_MSG_ERR_ESPCONN_ARG                    = 151,
  COMP_MSG_ERR_ESPCONN_ISCONN                 = 150,
  COMP_MSG_ERR_GET_STATION_CONFIG             = 149,
  COMP_MSG_ERR_CANNOT_DISCONNECT              = 148,
  COMP_MSG_ERR_CANNOT_SET_OPMODE              = 147,
  COMP_MSG_ERR_REGIST_CONNECT_CB              = 146,
  COMP_MSG_ERR_TCP_ACCEPT                     = 145,
  COMP_MSG_ERR_REGIST_TIME                    = 144,
  COMP_MSG_ERR_BAD_WIFI_VALUE_WHICH           = 143,
  COMP_MSG_ERR_BAD_ROW                        = 142,
  COMP_MSG_ERR_CANNOT_GET_MAC_ADDR            = 141,

  COMP_MSG_ERR_CANNOT_CONNECT                 = 99,

};


#ifdef  __cplusplus
}
#endif

#endif  /* COMP_MSG_ERROR_CODES_H */

