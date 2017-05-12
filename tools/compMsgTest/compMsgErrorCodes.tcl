# ===========================================================================
# * Copyright (c) 2017, Arnulf P. Wiedemann (arnulf@wiedemann-pri.de)
# * All rights reserved.
# *
# * License: BSD/MIT
# *
# * Redistribution and use in source and binary forms, with or without
# * modification, are permitted provided that the following conditions
# * are met {
# *
# * 1. Redistributions of source code must retain the above copyright
# * notice, this list of conditions and the following disclaimer.
# * 2. Redistributions in binary form must reproduce the above copyright
# * notice, this list of conditions and the following disclaimer in the
# * documentation and/or other materials provided with the distribution.
# * 3. Neither the name of the copyright holder nor the names of its
# * contributors may be used to endorse or promote products derived
# * from this software without specific prior written permission.
# *
# * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# * SUBSTITUTE GOODS OR SERVICES LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# * POSSIBILITY OF SUCH DAMAGE.
# *
# ==========================================================================

set ::COMP_MSG_ERR [dict create]
dict set ::COMP_MSG_ERR OK                               0
dict set ::COMP_MSG_ERR VALUE_NOT_SET                    255
dict set ::COMP_MSG_ERR VALUE_OUT_OF_RANGE               254
dict set ::COMP_MSG_ERR BAD_VALUE                        253
dict set ::COMP_MSG_ERR BAD_FIELD_TYPE                   252
dict set ::COMP_MSG_ERR FIELD_TYPE_NOT_FOUND             251
dict set ::COMP_MSG_ERR VALUE_TOO_BIG                    250
dict set ::COMP_MSG_ERR OUT_OF_MEMORY                    249
dict set ::COMP_MSG_ERR OUT_OF_RANGE                     248
dict set ::COMP_MSG_ERR FILE_NAME_TOKEN_NOT_FOUND        247
  # be carefull the values up to here
  # must correspond to the values in dataView.h !!!
  # with the names like DATA_VIEW_ERR *

dict set ::COMP_MSG_ERR FIELD_NOT_FOUND                  230
dict set ::COMP_MSG_ERR BAD_SPECIAL_FIELD                229
dict set ::COMP_MSG_ERR BAD_HANDLE                       228
dict set ::COMP_MSG_ERR HANDLE_NOT_FOUND                 227
dict set ::COMP_MSG_ERR NOT_ENCODED                      226
dict set ::COMP_MSG_ERR ENCODE_ERROR                     225
dict set ::COMP_MSG_ERR DECODE_ERROR                     224
dict set ::COMP_MSG_ERR BAD_CRC_VALUE                    223
dict set ::COMP_MSG_ERR CRYPTO_INIT_FAILED               222
dict set ::COMP_MSG_ERR CRYPTO_OP_FAILED                 221
dict set ::COMP_MSG_ERR CRYPTO_BAD_MECHANISM             220
dict set ::COMP_MSG_ERR NOT_ENCRYPTED                    219
dict set ::COMP_MSG_ERR DEFINITION_NOT_FOUND             218
dict set ::COMP_MSG_ERR DEFINITION_TOO_MANY_FIELDS       217
dict set ::COMP_MSG_ERR BAD_TABLE_ROW                    216
dict set ::COMP_MSG_ERR TOO_MANY_FIELDS                  215
dict set ::COMP_MSG_ERR BAD_DEFINTION_CMD_KEY            214
dict set ::COMP_MSG_ERR NO_SLOT_FOUND                    213
dict set ::COMP_MSG_ERR BAD_NUM_FIELDS                   212
dict set ::COMP_MSG_ERR ALREADY_INITTED                  211
dict set ::COMP_MSG_ERR NOT_YET_INITTED                  210
dict set ::COMP_MSG_ERR FIELD_CANNOT_BE_SET              209
dict set ::COMP_MSG_ERR NO_SUCH_FIELD                    208
dict set ::COMP_MSG_ERR DUPLICATE_FIELD                  207
dict set ::COMP_MSG_ERR BAD_DATA_LGTH                    206
dict set ::COMP_MSG_ERR NOT_YET_PREPARED                 205
dict set ::COMP_MSG_ERR FIELD_TOTAL_LGTH_MISSING         200

dict set ::COMP_MSG_ERR OPEN_FILE                        189
dict set ::COMP_MSG_ERR FILE_NOT_OPENED                  188
dict set ::COMP_MSG_ERR FLUSH_FILE                       187
dict set ::COMP_MSG_ERR WRITE_FILE                       186
dict set ::COMP_MSG_ERR FUNNY_EXTRA_FIELDS               185
dict set ::COMP_MSG_ERR FIELD_TOO_LONG                   184
dict set ::COMP_MSG_ERR BAD_MODULE_VALUE_WHICH           183
dict set ::COMP_MSG_ERR ALREADY_UPDATING                 182
dict set ::COMP_MSG_ERR DNS_ERROR                        181
dict set ::COMP_MSG_ERR STATION_SCAN                     180
dict set ::COMP_MSG_ERR BAD_RECEIVED_LGTH                179
dict set ::COMP_MSG_ERR BAD_FILE_CONTENTS                178
dict set ::COMP_MSG_ERR HEADER_NOT_FOUND                 177
dict set ::COMP_MSG_ERR BAD_FIELD_NAME                   175
dict set ::COMP_MSG_ERR BAD_HANDLE_TYPE                  174
dict set ::COMP_MSG_ERR INVALID_BASE64_STRING            173
dict set ::COMP_MSG_ERR TOO_FEW_FILE_LINES               172
dict set ::COMP_MSG_ERR ACTION_NAME_NOT_FOUND            171
dict set ::COMP_MSG_ERR DUPLICATE_ENTRY                  170
dict set ::COMP_MSG_ERR NO_WEBSOCKET_OPENED              169
dict set ::COMP_MSG_ERR TOO_MANY_REQUESTS                168
dict set ::COMP_MSG_ERR REQUEST_NOT_FOUND                167
dict set ::COMP_MSG_ERR UART_REQUEST_NOT_SET             166
dict set ::COMP_MSG_ERR FUNNY_HANDLE_TYPE                165
dict set ::COMP_MSG_ERR FIELD_VALUE_CALLBACK_NOT_FOUND   164
dict set ::COMP_MSG_ERR CONNECT_STATION_WRONG_PASSWD     163
dict set ::COMP_MSG_ERR CONNECT_STATION_NO_AP_FOUND      162
dict set ::COMP_MSG_ERR CONNECT_STATION_CONNECT_FAILED   161
dict set ::COMP_MSG_ERR CONNECT_STATION_CONNECTING       160
dict set ::COMP_MSG_ERR CONNECT_STATION_IDLE             159
dict set ::COMP_MSG_ERR ESPCONN_TIMEOUT                  158
dict set ::COMP_MSG_ERR ESPCONN_RTE                      157
dict set ::COMP_MSG_ERR ESPCONN_INPROGRESS               156
dict set ::COMP_MSG_ERR ESPCONN_ABRT                     155
dict set ::COMP_MSG_ERR ESPCONN_RST                      154
dict set ::COMP_MSG_ERR ESPCONN_CLSD                     153
dict set ::COMP_MSG_ERR ESPCONN_CONN                     152
dict set ::COMP_MSG_ERR ESPCONN_ARG                      151
dict set ::COMP_MSG_ERR ESPCONN_ISCONN                   150
dict set ::COMP_MSG_ERR GET_STATION_CONFIG               149
dict set ::COMP_MSG_ERR CANNOT_DISCONNECT                148
dict set ::COMP_MSG_ERR CANNOT_SET_OPMODE                147
dict set ::COMP_MSG_ERR REGIST_CONNECT_CB                146
dict set ::COMP_MSG_ERR TCP_ACCEPT                       145
dict set ::COMP_MSG_ERR REGIST_TIME                      144
dict set ::COMP_MSG_ERR BAD_WIFI_VALUE_WHICH             143
dict set ::COMP_MSG_ERR BAD_ROW                          142
dict set ::COMP_MSG_ERR CANNOT_GET_MAC_ADDR              141
dict set ::COMP_MSG_ERR WRONG_DESC_FILE_LINE             140
dict set ::COMP_MSG_ERR BAD_NUM_DESC_FILE_LINES          139
dict set ::COMP_MSG_ERR BAD_DESC_FILE_USE_LINE           138
dict set ::COMP_MSG_ERR FIELD_DESC_TOO_FEW_FIELDS        137
dict set ::COMP_MSG_ERR BAD_DESC_FILE_FIELD_INCLUDE_TYPE 136
dict set ::COMP_MSG_ERR BAD_DESC_FILE_FIELD_VALUE_TYPE   135
dict set ::COMP_MSG_ERR CANNOT_GET_IP                    134
dict set ::COMP_MSG_ERR BAD_SECURITY_TYPE                133
dict set ::COMP_MSG_ERR BAD_RW_VALUE                     133
dict set ::COMP_MSG_ERR BAD_RUN_ACTION_CALLBACK          132
dict set ::COMP_MSG_ERR BAD_OPERATING_MODE               131
dict set ::COMP_MSG_ERR HEADER_INCLUDE_NOT_FOUND         130
dict set ::COMP_MSG_ERR BAD_ENCRYPTED_VALUE              129
dict set ::COMP_MSG_ERR BAD_HANDLE_TYPE_VALUE            128
dict set ::COMP_MSG_ERR FUNNY_FIELD_VALUE_START          127
dict set ::COMP_MSG_ERR EXPECTED_INT_VALUE               126
dict set ::COMP_MSG_ERR CALLBACK_NAME_NOT_FOUND          125
dict set ::COMP_MSG_ERR DATA_VALUE_FIELD_NOT_FOUND       124

dict set ::COMP_MSG_ERR CANNOT_CONNECT                   99

set ::COMP_MSG_ERR_OK 0

# ================================ checkErrOK ===============================

proc checkErrOK {result} {
  switch $result {
    0 -
    OK {
      return [dict get $::COMP_MSG_ERR OK]
    }
    default {
      error "ERROR result: $result!"
    }
  }
}

