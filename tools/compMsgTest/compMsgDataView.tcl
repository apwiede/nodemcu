# ===========================================================================
# * Copyright (c) 2016, Arnulf P. Wiedemann (arnulf@wiedemann-pri.de)
# * All rights reserved.
# *
# * License: BSD/MIT
# *
# * Redistribution and use in source and binary forms, with or without
# * modification, are permitted provided that the following conditions
# * are met:
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
# * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# * POSSIBILITY OF SUCH DAMAGE.
# *
# ==========================================================================

set ::crcDebug false

set ::COMP_MSG_ERR_OK                    0
set ::COMP_MSG_ERR_VALUE_NOT_SET         255
set ::COMP_MSG_ERR_VALUE_OUT_OF_RANGE    254
set ::COMP_MSG_ERR_BAD_VALUE             253
set ::COMP_MSG_ERR_BAD_FIELD_TYPE        252
set ::COMP_MSG_ERR_FIELD_TYPE_NOT_FOUND  251
set ::COMP_MSG_ERR_VALUE_TOO_BIG         250
set ::COMP_MSG_ERR_OUT_OF_MEMORY         249
set ::COMP_MSG_ERR_OUT_OF_RANGE          248
  # be carefull the values up to here
  # must correspond to the values in dataView.h !!!
  # with the names like DATA_VIEW_ERR_*
  
set ::COMP_MSG_ERR_FIELD_NOT_FOUND            230
set ::COMP_MSG_ERR_BAD_SPECIAL_FIELD          229
set ::COMP_MSG_ERR_BAD_HANDLE                 228
set ::COMP_MSG_ERR_HANDLE_NOT_FOUND           227
set ::COMP_MSG_ERR_NOT_ENCODED                226
set ::COMP_MSG_ERR_ENCODE_ERROR               225
set ::COMP_MSG_ERR_DECODE_ERROR               224
set ::COMP_MSG_ERR_BAD_CRC_VALUE              223
set ::COMP_MSG_ERR_CRYPTO_INIT_FAILED         222
set ::COMP_MSG_ERR_CRYPTO_OP_FAILED           221
set ::COMP_MSG_ERR_CRYPTO_BAD_MECHANISM       220
set ::COMP_MSG_ERR_NOT_ENCRYPTED              219
set ::COMP_MSG_ERR_DEFINITION_NOT_FOUND       218
set ::COMP_MSG_ERR_DEFINITION_TOO_MANY_FIELDS 217
set ::COMP_MSG_ERR_BAD_TABLE_ROW              216
set ::COMP_MSG_ERR_TOO_MANY_FIELDS            215
set ::COMP_MSG_ERR_BAD_DEFINTION_CMD_KEY      214
set ::COMP_MSG_ERR_NO_SLOT_FOUND              213
set ::COMP_MSG_ERR_BAD_NUM_FIELDS             212
set ::COMP_MSG_ERR_ALREADY_INITTED            211
set ::COMP_MSG_ERR_NOT_YET_INITTED            210
set ::COMP_MSG_ERR_FIELD_CANNOT_BE_SET        209
set ::COMP_MSG_ERR_NO_SUCH_FIELD              208
set ::COMP_MSG_ERR_BAD_DATA_LGTH              207
set ::COMP_MSG_ERR_NOT_YET_PREPARED           206
set ::COMP_DEF_ERR_ALREADY_INITTED            205
set ::COMP_DEF_ERR_NOT_YET_INITTED            204
set ::COMP_DEF_ERR_NOT_YET_PREPARED           203
set ::COMP_DEF_ERR_ALREADY_CREATED            202
set ::COMP_MSG_ERR_FIELD_TOTAL_LGTH_MISSING   201
set ::COMP_LIST_ERR_ALREADY_INITTED           200
set ::COMP_LIST_ERR_NOT_YET_INITTED           199
set ::COMP_LIST_ERR_NOT_YET_PREPARED          198
set ::COMP_LIST_ERR_ALREADY_CREATED           197
set ::COMP_MSG_ERR_NO_SUCH_COMMAND            100

set ::COMP_MSG_NO_INCR 0
set ::COMP_MSG_INCR    1
set ::COMP_MSG_DECR    -1

set ::COMP_MSG_SPEC_FIELD_SRC                 255
set ::COMP_MSG_SPEC_FIELD_DST                 254
set ::COMP_MSG_SPEC_FIELD_TARGET_CMD          253
set ::COMP_MSG_SPEC_FIELD_TOTAL_LGTH          252
set ::COMP_MSG_SPEC_FIELD_CMD_KEY             251
set ::COMP_MSG_SPEC_FIELD_CMD_LGTH            250
set ::COMP_MSG_SPEC_FIELD_RANDOM_NUM          249
set ::COMP_MSG_SPEC_FIELD_SEQUENCE_NUM        248
set ::COMP_MSG_SPEC_FIELD_FILLER              247
set ::COMP_MSG_SPEC_FIELD_CRC                 246
set ::COMP_MSG_SPEC_FIELD_ID                  245
set ::COMP_MSG_SPEC_FIELD_TABLE_ROWS          244
set ::COMP_MSG_SPEC_FIELD_TABLE_ROW_FIELDS    243
set ::COMP_MSG_SPEC_FIELD_GUID                242
set ::COMP_MSG_SPEC_FIELD_NUM_NORM_FLDS       241
set ::COMP_MSG_SPEC_FIELD_NORM_FLD_IDS        240
set ::COMP_MSG_SPEC_FIELD_NORM_FLD_NAMES_SIZE 239
set ::COMP_MSG_SPEC_FIELD_NORM_FLD_NAMES      238
set ::COMP_MSG_SPEC_FIELD_DEFINITIONS_SIZE    237
set ::COMP_MSG_SPEC_FIELD_DEFINITIONS         236
set ::COMP_MSG_SPEC_FIELD_NUM_LIST_MSGS       234
set ::COMP_MSG_SPEC_FIELD_LIST_MSG_SIZES      233
set ::COMP_MSG_SPEC_FIELD_LIST_MSGS           232
set ::COMP_MSG_SPEC_FIELD_SRC_ID              231
set ::COMP_MSG_SPEC_FIELD_HDR_FILLER          230
set ::COMP_MSG_SPEC_FIELD_NUM_KEY_VALUES      229
set ::COMP_MSG_SPEC_FIELD_LOW                 228  ; # this must be the last entry!!

set ::COMP_MSG_FREE_FIELD_ID 0xFF
set RAND_MAX 0x7FFFFFFF

namespace eval ::compMsg {
  namespace ensemble create

  namespace export compMsgDataView

  namespace eval compMsgDataView {
    namespace ensemble create
      
    namespace export compMsgDataView freeCompMsgDataView getFieldNameIdFromStr
    namespace export setFieldValue getFieldValue getFieldNameStrFromId
    namespace export setRandomNum getRandomNum setSequenceNum getSequenceNum
    namespace export setFiller getFiller setCrc getCrc setTotalCrc getTotalCrc getSpecialFieldNameIntFromId

    variable specialFieldNames2Ids
    set specialFieldNames2Ids [dict create]
    dict set specialFieldNames2Ids "@src"                COMP_MSG_SPEC_FIELD_SRC
    dict set specialFieldNames2Ids "@dst"                COMP_MSG_SPEC_FIELD_DST
    dict set specialFieldNames2Ids "@targetCmd"          COMP_MSG_SPEC_FIELD_TARGET_CMD
    dict set specialFieldNames2Ids "@totalLgth"          COMP_MSG_SPEC_FIELD_TOTAL_LGTH
    dict set specialFieldNames2Ids "@cmdKey"             COMP_MSG_SPEC_FIELD_CMD_KEY
    dict set specialFieldNames2Ids "@cmdLgth"            COMP_MSG_SPEC_FIELD_CMD_LGTH
    dict set specialFieldNames2Ids "@randomNum"          COMP_MSG_SPEC_FIELD_RANDOM_NUM
    dict set specialFieldNames2Ids "@sequenceNum"        COMP_MSG_SPEC_FIELD_SEQUENCE_NUM
    dict set specialFieldNames2Ids "@filler"             COMP_MSG_SPEC_FIELD_FILLER
    dict set specialFieldNames2Ids "@crc"                COMP_MSG_SPEC_FIELD_CRC
    dict set specialFieldNames2Ids "@id"                 COMP_MSG_SPEC_FIELD_ID
    dict set specialFieldNames2Ids "@tablerows"          COMP_MSG_SPEC_FIELD_TABLE_ROWS
    dict set specialFieldNames2Ids "@tablerowfields"     COMP_MSG_SPEC_FIELD_TABLE_ROW_FIELDS
    dict set specialFieldNames2Ids "@GUID"               COMP_MSG_SPEC_FIELD_GUID
    dict set specialFieldNames2Ids "@numNormFlds"        COMP_MSG_SPEC_FIELD_NUM_NORM_FLDS
    dict set specialFieldNames2Ids "@normFldIds"         COMP_MSG_SPEC_FIELD_NORM_FLD_IDS
    dict set specialFieldNames2Ids "@normFldNamesSize"   COMP_MSG_SPEC_FIELD_NORM_FLD_NAMES_SIZE
    dict set specialFieldNames2Ids "@normFldNames"       COMP_MSG_SPEC_FIELD_NORM_FLD_NAMES
    dict set specialFieldNames2Ids "@definitionsSize"    COMP_MSG_SPEC_FIELD_DEFINITIONS_SIZE
    dict set specialFieldNames2Ids "@definitions"        COMP_MSG_SPEC_FIELD_DEFINITIONS
    dict set specialFieldNames2Ids "@numListMsgs"        COMP_MSG_SPEC_FIELD_NUM_LIST_MSGS
    dict set specialFieldNames2Ids "@listMsgSizesSize"   COMP_MSG_SPEC_FIELD_LIST_MSG_SIZES_SIZE
    dict set specialFieldNames2Ids "@listMsgs"           COMP_MSG_SPEC_FIELD_LIST_MSGS
    dict set specialFieldNames2Ids "@srcId"              COMP_MSG_SPEC_FIELD_SRC_ID
    dict set specialFieldNames2Ids "@hdrFiller"          COMP_MSG_SPEC_FIELD_HDR_FILLER 
    dict set specialFieldNames2Ids "@numKeyValues"       COMP_MSG_SPEC_FIELD_NUM_KEY_VALUES
    dict set specialFieldNames2Ids "@provisioningSsid"   COMP_MSG_SPEC_FIELD_PROVISIONING_SSID
    dict set specialFieldNames2Ids "@provisioningPort"   COMP_MSG_SPEC_FIELD_PROVISIONING_PORT
    dict set specialFieldNames2Ids "@provisioningIPAddr" COMP_MSG_SPEC_FIELD_PROVISIONING_IP_ADDR
    dict set specialFieldNames2Ids "@clientSsid"         COMP_MSG_SPEC_FIELD_CLIENT_SSID
    dict set specialFieldNames2Ids "@clientPasswd"       COMP_MSG_SPEC_FIELD_CLIENT_PASSWD
    dict set specialFieldNames2Ids "@clientIPAddr"       COMP_MSG_SPEC_FIELD_CLIENT_IP_ADDR
    dict set specialFieldNames2Ids "@clientPort"         COMP_MSG_SPEC_FIELD_CLIENT_PORT
    dict set specialFieldNames2Ids "@clientStatus"       COMP_MSG_SPEC_FIELD_CLIENT_STATUS
    dict set specialFieldNames2Ids "@cloudDomain"        COMP_MSG_SPEC_FIELD_CLOUD_DOMAIN
    dict set specialFieldNames2Ids "@cloudPort"          COMP_MSG_SPEC_FIELD_CLOUD_PORT
    dict set specialFieldNames2Ids "@cloudHost1"         COMP_MSG_SPEC_FIELD_CLOUD_HOST_1
    dict set specialFieldNames2Ids "@cloudHost2"         COMP_MSG_SPEC_FIELD_CLOUD_HOST_2
    dict set specialFieldNames2Ids "@cloudSecureConnect" COMP_MSG_SPEC_FIELD_CLOUD_SECURE_CONNECT
    dict set specialFieldNames2Ids "@cloudSubUrl"        COMP_MSG_SPEC_FIELD_CLOUD_SUB_URL
    dict set specialFieldNames2Ids "@cloudNodeToken"     COMP_MSG_SPEC_FIELD_CLOUD_NODE_TOKEN
    dict set specialFieldNames2Ids "@totalCrc"           COMP_MSG_SPEC_FIELD_TOTAL_CRC



    variable specialFieldId2Names
    set specialFieldId2Names [dict create]
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_SRC                  "@src"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_DST                  "@dst"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_TARGET_CMD           "@targetCmd"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_TOTAL_LGTH           "@totalLgth"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_CMD_KEY              "@cmdKey"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_CMD_LGTH             "@cmdLgth"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_RANDOM_NUM           "@randomNum"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_SEQUENCE_NUM         "@sequenceNum"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_FILLER               "@filler"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_CRC                  "@crc"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_ID                   "@id"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_TABLE_ROWS           "@tablerows"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_TABLE_ROW_FIELDS     "@tablerowfields"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_GUID                 "@GUID"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_NUM_NORM_FLDS        "@numNormFlds"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_NORM_FLD_IDS         "@normFldIds"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_NORM_FLD_NAMES_SIZE  "@normFldNamesSize"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_NORM_FLD_NAMES       "@normFldNames"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_DEFINITIONS_SIZE     "@definitionsSize"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_DEFINITIONS          "@definitions"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_NUM_LIST_MSGS        "@numListMsgs"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_LIST_MSG_SIZES_SIZE  "@listMsgSizesSize"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_LIST_MSGS            "@listMsgs"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_SRC_ID               "@srcId"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_HDR_FILLER           "@hdrFiller"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_NUM_KEY_VALUES       "@numKeyValues"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_PROVISIONING_SSID    "@provisioningSsid"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_PROVISIONING_PORT    "@provisioningPort"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_PROVISIONING_IP_ADDR "@provisioningIPAddr"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_CLIENT_SSID          "@clientSsid"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_CLIENT_PASSWD        "@clientPasswd"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_CLIENT_IP_ADDR       "@clientIPAddr"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_CLIENT_PORT          "@clientPort"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_CLIENT_STATUS        "@clientStatus"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_CLOUD_DOMAIN         "@cloudDomain"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_CLOUD_PORT           "@cloudPort"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_CLOUD_HOST_1         "@cloudHost1"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_CLOUD_HOST_2         "@cloudHost2"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_CLOUD_SECURE_CONNECT "@cloudSecureConnect"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_CLOUD_SUB_URL        "@cloudSubUrl"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_CLOUD_NODE_TOKEN     "@cloudNodeToken"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_TOTAL_CRC            "@totalCrc"

    variable specialFieldIds2Ints
    set specialFieldIds2Ints [dict create]
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_SRC                  255
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_DST                  254
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_TARGET_CMD           253
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_TOTAL_LGTH           252
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_CMD_KEY              251
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_CMD_LGTH             250
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_RANDOM_NUM           249
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_SEQUENCE_NUM         248
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_FILLER               247
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_CRC                  246
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_ID                   245
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_TABLE_ROWS           244
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_TABLE_ROW_FIELDS     243
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_NUM_FIELDS           242
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_GUID                 241
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_NUM_NORM_FLDS        240
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_NORM_FLD_IDS         239
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_NORM_FLD_NAMES_SIZE  238
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_NORM_FLD_NAMES       237
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_DEFINITIONS_SIZE     236
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_DEFINITIONS          235
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_NUM_LIST_MSGS        234
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_LIST_MSG_SIZES_SIZE  233
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_LIST_MSGS            232
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_SRC_ID               231
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_HDR_FILLER           230
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_NUM_KEY_VALUES       229
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_PROVISIONING_SSID    228
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_PROVISIONING_PORT    227
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_PROVISIONING_IP_ADDR 226
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_CLIENT_SSID          225
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_CLIENT_PASSWD        224
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_CLIENT_IP_ADDR       223
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_CLIENT_PORT          222
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_CLIENT_STATUS        221
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_CLOUD_DOMAIN         220
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_CLOUD_PORT           219
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_CLOUD_HOST_1         218
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_CLOUD_HOST_2         217
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_CLOUD_SECURE_CONNECT 216
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_CLOUD_SUB_URL        215
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_CLOUD_NODE_TOKEN     214
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_TOTAL_CRC            213

    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_LOW                  212  ; # this must be the last entry!!

    variable specialFieldInts2Ids
    set specialFieldInts2Ids [dict create]
    dict set specialFieldInts2Ids 255 COMP_MSG_SPEC_FIELD_SRC
    dict set specialFieldInts2Ids 254 COMP_MSG_SPEC_FIELD_DST
    dict set specialFieldInts2Ids 253 COMP_MSG_SPEC_FIELD_TARGET_CMD
    dict set specialFieldInts2Ids 252 COMP_MSG_SPEC_FIELD_TOTAL_LGTH
    dict set specialFieldInts2Ids 251 COMP_MSG_SPEC_FIELD_CMD_KEY 
    dict set specialFieldInts2Ids 250 COMP_MSG_SPEC_FIELD_CMD_LGTH
    dict set specialFieldInts2Ids 249 COMP_MSG_SPEC_FIELD_RANDOM_NUM
    dict set specialFieldInts2Ids 248 COMP_MSG_SPEC_FIELD_SEQUENCE_NUM
    dict set specialFieldInts2Ids 247 COMP_MSG_SPEC_FIELD_FILLER
    dict set specialFieldInts2Ids 246 COMP_MSG_SPEC_FIELD_CRC
    dict set specialFieldInts2Ids 245 COMP_MSG_SPEC_FIELD_ID
    dict set specialFieldInts2Ids 244 COMP_MSG_SPEC_FIELD_TABLE_ROWS
    dict set specialFieldInts2Ids 243 COMP_MSG_SPEC_FIELD_TABLE_ROW_FIELDS
    dict set specialFieldInts2Ids 242 COMP_MSG_SPEC_FIELD_NUM_FIELDS
    dict set specialFieldInts2Ids 241 COMP_MSG_SPEC_FIELD_GUID
    dict set specialFieldInts2Ids 240 COMP_MSG_SPEC_FIELD_NUM_NORM_FLDS
    dict set specialFieldInts2Ids 239 COMP_MSG_SPEC_FIELD_NORM_FLD_IDS
    dict set specialFieldInts2Ids 238 COMP_MSG_SPEC_FIELD_NORM_FLD_NAMES_SIZE
    dict set specialFieldInts2Ids 237 COMP_MSG_SPEC_FIELD_NORM_FLD_NAMES
    dict set specialFieldInts2Ids 236 COMP_MSG_SPEC_FIELD_DEFINITIONS_SIZE
    dict set specialFieldInts2Ids 235 COMP_MSG_SPEC_FIELD_DEFINITIONS
    dict set specialFieldInts2Ids 234 COMP_MSG_SPEC_FIELD_NUM_LIST_MSGS
    dict set specialFieldInts2Ids 233 COMP_MSG_SPEC_FIELD_LIST_MSG_SIZES_SIZE
    dict set specialFieldInts2Ids 232 COMP_MSG_SPEC_FIELD_LIST_MSGS
    dict set specialFieldInts2Ids 231 COMP_MSG_SPEC_FIELD_SRC_ID
    dict set specialFieldInts2Ids 230 COMP_MSG_SPEC_FIELD_HDR_FILLER
    dict set specialFieldInts2Ids 229 COMP_MSG_SPEC_FIELD_NUM_KEY_VALUES
    dict set specialFieldInts2Ids 228 COMP_MSG_SPEC_FIELD_PROVISIONING_SSID
    dict set specialFieldInts2Ids 227 COMP_MSG_SPEC_FIELD_PROVISIONING_PORT
    dict set specialFieldInts2Ids 226 COMP_MSG_SPEC_FIELD_PROVISIONING_IP_ADDR
    dict set specialFieldInts2Ids 225 COMP_MSG_SPEC_FIELD_CLIENT_SSID
    dict set specialFieldInts2Ids 224 COMP_MSG_SPEC_FIELD_CLIENT_PASSWD
    dict set specialFieldInts2Ids 223 COMP_MSG_SPEC_FIELD_CLIENT_IP_ADDR
    dict set specialFieldInts2Ids 222 COMP_MSG_SPEC_FIELD_CLIENT_PORT
    dict set specialFieldInts2Ids 221 COMP_MSG_SPEC_FIELD_CLIENT_STATUS
    dict set specialFieldInts2Ids 220 COMP_MSG_SPEC_FIELD_CLOUD_DOMAIN
    dict set specialFieldInts2Ids 219 COMP_MSG_SPEC_FIELD_CLOUD_PORT
    dict set specialFieldInts2Ids 218 COMP_MSG_SPEC_FIELD_CLOUD_HOST_1
    dict set specialFieldInts2Ids 217 COMP_MSG_SPEC_FIELD_CLOUD_HOST_2
    dict set specialFieldInts2Ids 216 COMP_MSG_SPEC_FIELD_CLOUD_SECURE_CONNECT
    dict set specialFieldInts2Ids 215 COMP_MSG_SPEC_FIELD_CLOUD_SUB_URL
    dict set specialFieldInts2Ids 214 COMP_MSG_SPEC_FIELD_CLOUD_NODE_TOKEN
    dict set specialFieldInts2Ids 213 COMP_MSG_SPEC_FIELD_TOTAL_CRC

    dict set specialFieldInts2Ids 222 COMP_MSG_SPEC_FIELD_LOW  ; # this must be the last entry!!

    dict set ::compMsg(fieldNameDefinitions) numDefinitions 0

    # ================================= getSpecialFieldNameIntFromId ====================================
    
    proc getSpecialFieldNameIntFromId {fieldNameId fieldNameIntVar} {
      upvar $fieldNameIntVar fieldNameInt
      variable specialFieldIds2Ints

      if {[dict exists $specialFieldIds2Ints $fieldNameId]} {
        set fieldNameInt [dict get $specialFieldId2Ints $fieldNameId]
        return $::COMP_MSG_ERR_OK
      }
      checkErrOK $::COMP_MSG_ERR_BAD_SPECIAL_FIELD
    }

    # ================================= getFieldNameIdFromStr ====================================
    
    proc getFieldNameIdFromStr {fieldName fieldNameIdVar incrVal} {
      upvar $fieldNameIdVar fieldNameId
      variable specialFieldNames2Ids

      if {[string range $fieldName 0 0] eq "@"} {
        # find special field name
        if {[dict exists $specialFieldNames2Ids $fieldName]} {
          set fieldNameId [dict get $specialFieldNames2Ids $fieldName]
          return $::COMP_MSG_ERR_OK
        }
        checkErrOK $::COMP_MSG_ERR_BAD_SPECIAL_FIELD
      } else {
        set firstFreeEntry [list]
        set firstFreeEntryIdx 0
        set numDefinitions [dict get $::compMsg(fieldNameDefinitions) numDefinitions]
        if {$numDefinitions > 0} {
          # find fieldName
          set nameIdx 0
          set definitions [dict get $::compMsg(fieldNameDefinitions) definitions]
          foreach entry $definitions {
            if {[dict get $entry fieldName] eq $fieldName} {
              if {$incrVal < 0} {
                if {[dict get $entry refCnt] > 0} {
                  dict set entry refCnt [expr {[dict get $entry refCnt] - 1}]
                }
                if {[dict get $entry refCnt] == 0} {
                  dict set entry id $::COMP_MSG_FREE_FIELD_ID
                  dict set entry fieldName ""
                }
              } else {
                if {$incrVal > 0} {
                  dict set entry refCnt [expr {[dict get $entry refCnt] + 1}]
                } else {
                  # just get the entry, do not modify
                }
              }
              set fieldNameId [dict get $entry id]
              return $::COMP_MSG_ERR_OK
            }
            if {($incrVal == $::COMP_MSG_INCR) && ([dict get $entry id] eq $::COMP_MSG_FREE_FIELD_ID) && ($firstFreeEntry eq "")} {
              dict set firstFreeEntry id [expr {$nameIdx + 1}]
            }
            incr nameIdx
          }
        }
        if {$incrVal == $::COMP_MSG_DECR} {
          return $::COMP_MSG_ERR_OK ; # just ignore silently
        } else {
          if {$incrVal == $::COMP_MSG_NO_INCR} {
puts stderr "field not found: $fieldName!incrVal: $incrVal!"
            checkErrOK $::COMP_MSG_ERR_FIELD_NOT_FOUND
          } else {
            if {$firstFreeEntry ne ""} {
              set fieldNameId [dict get $firstFreeEntry id]
              dict set firstFreeEntry refCnt 1
              dict set firstFreeEntry fieldName $fieldName
              set definitions [lreplace $definitions $firstFreeEntryIdx $firstFreeEntryIdx $firstFreeEntry]
            } else {
              incr numDefinitions
              dict set ::compMsg(fieldNameDefinitions) numDefinitions $numDefinitions
              set entry [dict create]
              dict set entry refCnt 1
              dict set entry id $numDefinitions
              dict set entry fieldName $fieldName
              set fieldNameId $numDefinitions
              lappend definitions $entry
            }
          }
        }
        dict set ::compMsg(fieldNameDefinitions) definitions $definitions
      }
      return $::DATA_VIEW_ERR_OK
    }
    
    # ================================= getFieldNameStrFromId ====================================
    
    proc getFieldNameStrFromId {fieldNameId fieldNameVar} {
      upvar $fieldNameVar fieldName
      variable specialFieldId2Names
      variable specialFieldInts2Ids

      if {[string is integer $fieldNameId]} {
        if {[dict exists $specialFieldInts2Ids $fieldNameId]} {
          set fieldNameId [dict get $specialFieldInts2Ids $fieldNameId]
        }
      }
      # first try to find special field name
      if {[dict exists $specialFieldId2Names $fieldNameId]} {
        set fieldName [dict get $specialFieldId2Names $fieldNameId]
        return $::COMP_MSG_ERR_OK
      }
      # find field name
      set idx 0
      set fieldNameDefinitions $::compMsg(fieldNameDefinitions)
      while {$idx < [dict get $fieldNameDefinitions numDefinitions]} {
        set nameEntry [lindex  [dict get $fieldNameDefinitions definitions] $idx]
        if {[dict get $nameEntry id] == $fieldNameId} {
          set fieldName [dict get $nameEntry fieldName]
          return $::COMP_MSG_ERR_OK
        }
        incr idx
      }
      checkErrOK $::COMP_MSG_ERR_FIELD_NOT_FOUND
    }
    
    # ============================= getRandom ========================
    
    proc getRandom {} {
      set val [string trimleft [lindex [split [expr {rand()}] {.}] 1] 0]
      set myVal [expr {$val & $::RAND_MAX}]
      return $myVal
    }
    
    # ================================= getRandomNum ====================================
    
    proc getRandomNum {fieldInfo value} {
      return [::compMsg dataView getUint32 [dict get $fieldInfo fieldOffset] value]
    }
    
    # ================================= setRandomNum ====================================
    
    proc setRandomNum {fieldInfo} {
      set val [getRandom]
      return [::compMsg dataView setUint32 [dict get $fieldInfo fieldOffset] $val]
    }
    
    
    # ================================= getSequenceNum ====================================
    
    proc getSequenceNum {fieldInfo valueVar} {
      upvar $valueVar value

      return [::compMsg dataView getUint32 [dict get $fieldInfo fieldOffset] value]
    }
    
    # ================================= setSequenceNum ====================================
    
    proc setSequenceNum {fieldInfo} {
      incr ::sequenceNum
      return [::compMsg dataView setUint32 [dict get $fieldInfo fieldOffset] $::sequenceNum]
    }
    
    # ================================= getFiller ====================================
    
    proc getFiller {fieldInfo valueVar} {
      upvar $valueVar value

      return [::compMsg dataView getuint8Vector [dict get $fieldInfo fieldOffset] value [dict get $fieldInfo fieldLgth]]
    }
    
    # ================================= setFiller ====================================
    
    proc setFiller {fieldInfo} {
      set lgth [dict get $fieldInfo fieldLgth]
      set offset [dict get $fieldInfo fieldOffset]
      set idx 0
      while {$lgth >= 4} {
        set value ""
        set myVal [expr {[getRandom] &0xFFFFFFFF}]
        append value [binary format c [expr {($myVal >> 24) & 0xFF}]]
        append value [binary format c [expr {($myVal >> 16) & 0xFF}]]
        append value [binary format c [expr {($myVal >> 8) & 0xFF}]]
        append value [binary format c [expr {($myVal >> 0) & 0xFF}]]
        set result [::compMsg dataView setUint32 $offset $value]
        checkErrOK $result
        incr offset 4
        incr lgth -4
      }
      while {$lgth >= 2} {
        set value ""
        set myVal [expr {[getRandom] & 0xFFFF}]
        append value [binary format c [expr {($myVal >> 8) & 0xFF}]]
        append value [binary format c [expr {($myVal >> 0) & 0xFF}]]
        set result [::compMsg dataView setUint16 $offset $value]
        checkErrOK $result
        incr offset 2
        incr lgth -2
      }
      while {$lgth >= 1} {
        set value ""
        set myVal [expr {[getRandom] & 0xFF}]
        append value [binary format c [expr {($myVal >> 0) & 0xFF}]]
        set result [::compMsg dataView setUint8 $offset $value]
        checkErrOK $result
        incr offset 1
        incr lgth -1
      }
      return $::DATA_VIEW_ERR_OK
    }
    
    
    # ================================= getCrc ====================================
    
    proc getCrc {fieldInfo valueVar startOffset size} {
      upvar $valueVar value

#set ::crcDebug true
      set crcLgth [dict get $fieldInfo fieldLgth]
      set value ""
      set lgth [expr {$size - $crcLgth}]
      set crcVal 0
      set offset $startOffset
set cnt 0
puts stderr "getCrc: offset: $offset size: $size ::crcDebug: $::crcDebug!"
      while {$offset < $size} {
        set result [::compMsg dataView getUint8 $offset ch]
        set pch $ch
        if {![string is integer $ch]} {
          binary scan $ch c pch
        }
        set pch [expr {$pch & 0xFF}]
if {$::crcDebug} {
puts stderr "getCrc: $cnt ch: [format 0x%02x $pch]![format 0x%04x $crcVal]!"
}
        set crcVal [expr {$crcVal + [format "%d" $pch]}]
incr cnt
        incr offset
      }
if {$::crcDebug} {
puts stderr "crcVal end: $crcVal!"
}
      set offset [dict get $fieldInfo fieldOffset]
      if {$crcLgth == 2} {
        set crcVal [expr {~$crcVal & 0xFFFF}]
        set result [::compMsg dataView getuint16 $offset crc]
        checkErrOK $result
if {$::crcDebug} {
puts stderr "crcVal: [format 0x%04x $crcVal]!offset: $offset!crc: [format 0x%04x $crc]!"
}
        if {$crcVal != $crc} {
          return $::COMP_MSG_ERR_BAD_CRC_VALUE
        }
        set value $crc
      } else  {
        set crcVal [expr {~$crcVal}]
        set crcVal [expr {$crcVal & 0xFF}]
if {$::crcDebug} {
puts stderr "crcVal2 end: $crcVal!"
}
        set result [::compMsg dataView getUint8 $offset crc]
        checkErrOK $result
if {$::crcDebug} {
puts stderr "crcVal: [format 0x%02x [expr {$crcVal & 0xFF}]]!offset: $offset!crc: [format 0x%02x $crc]!"
}
        if {[expr {$crcVal & 0xFF}] != $crc} {
          checkErrOK $::COMP_MSG_ERR_BAD_CRC_VALUE
        }
        set value $crc
      }
      return $::DATA_VIEW_ERR_OK
    }
    
    # ================================= setCrc ====================================
    
    proc setCrc {fieldInfo startOffset size} {
      set crcLgth [dict get $fieldInfo fieldLgth]
#puts stderr "setCrc: startOffset: $startOffset size: $size!"
      set size [expr {$size - $crcLgth}]
#set ::crcDebug true
set cnt 0
      set crc  0
      set offset $startOffset
      while {$offset < $size} {
        set result [::compMsg dataView getUint8 $offset ch]
        set pch $ch
        if {![string is integer $ch]} {
          binary scan $ch c pch
        }
        set pch [expr {$pch & 0xFF}]
if {$::crcDebug} {
puts stderr "setCrc: $cnt $ch![format 0x%02x $pch]![format 0x%04x $crc]!"
}
        set crc [expr {$crc + [format "%d" $pch]}]
incr cnt
        incr offset
      }
if {$::crcDebug} {
puts stderr "setCrc end: $cnt $ch![format 0x%02x $pch]![format 0x%04x $crc]!"
}
if {$::crcDebug} {
puts stderr "crc1: $crc![format 0x%04x $crc]!"
}
      set crc [expr {~$crc & 0xFFFF}]
if {$::crcDebug} {
puts stderr "crc11: $crc![format 0x%04x $crc]![format 0x%02x [expr {$crc & 0xFF}]]"
}
      if {$crcLgth == 1} {
#puts stderr "offset: [dict get $fieldInfo fieldOffset] val: [expr {$crc & 0xFF}]!"
        set result [::compMsg dataView setUint8 [dict get $fieldInfo fieldOffset] [expr {$crc & 0xFF}]]
      } else {
        set result [::compMsg dataView setUint16 [dict get $fieldInfo fieldOffset] $crc]
      }
      checkErrOK $result
      return $::DATA_VIEW_ERR_OK
    }
    
    # ================================= getTotalCrc ====================================
    
    proc getTotalCrc {fieldInfo valueVar} {
      upvar $valueVar value

      set crcLgth [dict get $fieldInfo fieldLgth]
      set value ""
      set size [dict get $fieldInfo fieldOffset]
      set crcVal 0
      set offset 0
#set ::crcDebug true
set cnt 0
      while {$offset < $size} {
        set result [::compMsg dataView getUint8 $offset ch]
        set pch $ch
        if {![string is integer $ch]} {
          binary scan $ch c pch
        }
        set pch [expr {$pch & 0xFF}]
if {$::crcDebug} {
puts stderr "getTotalCrc: $cnt ch: [format 0x%02x $pch]![format 0x%04x $crcVal]!"
}
        set crcVal [expr {$crcVal + [format "%d" $pch]}]
incr cnt
        incr offset
      }
if {$::crcDebug} {
puts stderr "crcVal end: $crcVal!"
}
      set offset [dict get $fieldInfo fieldOffset]
      if {$crcLgth == 2} {
        set crcVal [expr {~$crcVal & 0xFFFF}]
        set result [::compMsg dataView getuint16 $offset crc]
        checkErrOK $result
if {$::crcDebug} {
puts stderr "crcVal: [format 0x%04x $crcVal]!offset: $offset!crc: [format 0x%04x $crc]!"
}
        if {$crcVal != $crc} {
          return $::COMP_MSG_ERR_BAD_CRC_VALUE
        }
        set value $crc
      } else  {
        set crcVal [expr {~$crcVal}]
        set crcVal [expr {$crcVal & 0xFF}]
if {$::crcDebug} {
puts stderr "crcVal2 end: $crcVal!"
}
        set result [::compMsg dataView getUint8 $offset crc]
        checkErrOK $result
if {$::crcDebug} {
puts stderr "crcVal: [format 0x%02x [expr {$crcVal & 0xFF}]]!offset: $offset!crc: [format 0x%02x $crc]!"
}
        if {[expr {$crcVal & 0xFF}] != $crc} {
          checkErrOK $::COMP_MSG_ERR_BAD_CRC_VALUE
        }
        set value $crc
      }
      return $::DATA_VIEW_ERR_OK
    }
    
    # ================================= setTotalCrc ====================================
    
    proc setTotalCrc {fieldInfo} {
      set crcLgth [dict get $fieldInfo fieldLgth]
      set size [dict get $fieldInfo fieldOffset]
#set ::crcDebug true
set cnt 0
#puts stderr "setTotalCrc: $::compMsg::dataView::lgth!$::compMsg::dataView::data!"
      set crc  0
      set offset 0
      while {$offset < $size} {
        set result [::compMsg dataView getUint8 $offset ch]
        set pch $ch
        if {![string is integer $ch]} {
          binary scan $ch c pch
        }
        set pch [expr {$pch & 0xFF}]
if {$::crcDebug} {
puts stderr "setTotalCrc: $cnt $ch![format 0x%02x $pch]![format 0x%04x $crc]!"
}
        set crc [expr {$crc + [format "%d" $pch]}]
incr cnt
        incr offset
      }
if {$::crcDebug} {
puts stderr "crc1: $crc![format 0x%04x $crc]!"
}
      set crc [expr {~$crc & 0xFFFF}]
if {$::crcDebug} {
puts stderr "crc11: $crc![format 0x%04x $crc]!"
}
      if {$crcLgth == 1} {
#puts stderr "offset: [dict get $fieldInfo fieldOffset] val: [format 0x%02x [expr {$crc & 0xFF}]]!"
        set result [::compMsg dataView setUint8 [dict get $fieldInfo fieldOffset] [expr {$crc & 0xFF}]]
      } else {
        set result [::compMsg dataView setUint16 [dict get $fieldInfo fieldOffset] $crc]
      }
      checkErrOK $result
      return $::DATA_VIEW_ERR_OK
    }
    
    
    # ================================= getFieldValue ====================================
    
    proc getFieldValue {fieldInfo valueVar fieldIdx} {
      upvar $valueVar value

if {[catch {
      set value ""
      set offset [dict get $fieldInfo fieldOffset]
      set shortOffset [expr {$offset + $fieldIdx}]
      switch [dict get $fieldInfo fieldTypeId] {
        DATA_VIEW_FIELD_INT8_T {
          set result [::compMsg dataView getInt8 $shortOffset value]
        }
        DATA_VIEW_FIELD_UINT8_T {
          set result [::compMsg dataView getUint8 $shortOffset value]
        }
        DATA_VIEW_FIELD_INT16_T {
          set result [::compMsg dataView getInt16 $shortOffset value]
        }
        DATA_VIEW_FIELD_UINT16_T {
          set result [::compMsg dataView getUint16 $shortOffset value]
        }
        DATA_VIEW_FIELD_INT32_T {
          set result [::compMsg dataView getInt32 $shortOffset value]
        }
        DATA_VIEW_FIELD_UINT32_T {
          set result [::compMsg dataView getUint32 $shortOffset value]
        }
        DATA_VIEW_FIELD_INT8_VECTOR {
          set result [::compMsg dataView getInt8Vector [expr {$offset + $fieldIdx}] value [dict get $fieldInfo fieldLgth]]
        }
        DATA_VIEW_FIELD_UINT8_VECTOR {
          set result [::compMsg dataView getUint8Vector [expr {$offset + $fieldIdx}] value [dict get $fieldInfo fieldLgth]]
        }
        DATA_VIEW_FIELD_INT16_VECTOR {
          set result [::compMsg dataView getInt16 [expr {$offset +fieldIdx*2}] value]
        }
        DATA_VIEW_FIELD_UINT16_VECTOR {
          set result [::compMsg dataView getUint16 [expr {$offset + $fieldIdx*2}] value]
        }
        DATA_VIEW_FIELD_INT32_VECTOR {
          set result [::compMsg dataView getInt32 [expr {$offset + $fieldIdx*4}] value]
        }
        DATA_VIEW_FIELD_UINT32_VECTOR {
          set result [::compMsg dataView getUint32 [expr {$offset + $fieldIdx*4}] value]
        }
        default {
          checkErrOK $::COMP_MSG_ERR_BAD_FIELD_TYPE
        }
      }
} msg]} {
puts stderr "getFieldValue: $msg!$fieldInfo!"
}
      checkErrOK $result
      return $::DATA_VIEW_ERR_OK
    }
    
    # ================================= setFieldValue ====================================
    
    proc setFieldValue {fieldInfo value fieldIdx} {
      switch [dict get $fieldInfo fieldTypeId] {
        DATA_VIEW_FIELD_INT8_T {
          if {($value > -128) && ($value < 128)} {
            set result [::compMsg dataView setInt8 [dict get $fieldInfo fieldOffset] $value]
          } else {
puts stderr "compMsgDataView setFieldValue int8 value too big"
            checkErrOK $::COMP_MSG_ERR_VALUE_TOO_BIG
          }
        }
        DATA_VIEW_FIELD_UINT8_T {
          binary scan $value c pch
          set pch [expr {$pch & 0xFF}]
          set value $pch
          if {($value >= 0) && ($value <= 256)} {
            set result [::compMsg dataView setUint8 [dict get $fieldInfo fieldOffset] $value]
          } else {
puts stderr "compMsgDataView setFieldValue uint8 value too big"
            checkErrOK $::COMP_MSG_ERR_BAD_VALUE
          }
        }
        DATA_VIEW_FIELD_INT16_T {
          if {($value > -32767) && ($value < 32767)} {
            set result [::compMsg dataView setInt16 [dict get $fieldInfo fieldOffset] $value]
          } else {
puts stderr "compMsgDataView setFieldValue int16 value too big"
            checkErrOK $::COMP_MSG_ERR_VALUE_TOO_BIG
          }
        }
        DATA_VIEW_FIELD_UINT16_T {
          if {![string is integer $value]} {
            binary scan $value S val
            set value $val
          }
          if {($value >= 0) && ($value <= 65535)} {
            set result [::compMsg dataView setUint16 [dict get $fieldInfo fieldOffset] $value]
          } else {
puts stderr "compMsgDataView setFieldValue uint16 value too big"
            checkErrOK $::COMP_MSG_ERR_VALUE_TOO_BIG
          }
        }
        DATA_VIEW_FIELD_INT32_T {
          if {($value > -0x7FFFFFFF) && ($value <= 0x7FFFFFFF)} {
            set result [::compMsg dataView setInt32 [dict get $fieldInfo fieldOffset] $value]
          } else {
puts stderr "compMsgDataView setFieldValue int32 value too big"
            checkErrOK $::COMP_MSG_ERR_VALUE_TOO_BIG
          }
        }
        DATA_VIEW_FIELD_UINT32_T {
          # we have to do the signed check as numericValue is a signed integer!!
          if {($value > -0x7FFFFFFF) && ($value <= 0x7FFFFFFF)} {
            set result [::compMsg dataView setUint32 [dict get $fieldInfo fieldOffset] $value]
          } else {
puts stderr "compMsgDataView setFieldValue uint32 value too big !$value!"
            checkErrOK $::COMP_MSG_ERR_VALUE_TOO_BIG
          }
        }
        DATA_VIEW_FIELD_INT8_VECTOR {
          set result [::compMsg dataView setInt8Vector [dict get $fieldInfo fieldOffset] $value]
        }
        DATA_VIEW_FIELD_UINT8_VECTOR {
#puts stderr "setUint8Vector: lgth: [dict get $fieldInfo fieldLgth]!offset: [dict get $fieldInfo fieldOffset]!dv lgth: $::compMsg::dataView::lgth!"
          set result [::compMsg dataView setUint8Vector [dict get $fieldInfo fieldOffset] $value [dict get $fieldInfo fieldLgth]]
        }
        DATA_VIEW_FIELD_INT16_VECTOR {
          set result [::compMsg dataView setInt16Vector [expr {[dict get $fieldInfo fieldOffset] + $fieldIdx*2}] $value]
        }
        DATA_VIEW_FIELD_UINT16_VECTOR {
          set result [::compMsg dataView setUint16Vector [expr {[dict get $fieldInfo fieldOffset] + $fieldIdx*2}] $value]
        }
        DATA_VIEW_FIELD_INT32_VECTOR {
          set result [::compMsg dataView setInt32Vector [expr {[dict get $fieldInfo fieldOffset] + $fieldIdx*2}] $value]
        }
        DATA_VIEW_FIELD_UINT32_VECTOR {
          set result [::compMsg dataView setUint32Vector [expr {[dict get $fieldInfo fieldOffset] + $fieldIdx*2}] $value]
        }
        default {
puts stderr "bad type in setFieldValue: [dict get $fieldInfo fieldTypeId]"
          checkErrOK $::COMP_MSG_ERR_BAD_FIELD_TYPE
        }
      }
      checkErrOK $result
      return $::DATA_VIEW_ERR_OK
    }

    # ================================= compMsgDataView ====================================
    
    proc compMsgDataView {command args} {
      switch $command {
        getFieldNameStrFromId -
        getFieldNameIdFromStr {
         return [uplevel 0 $command $args]
        }
      }
puts stderr "compMsgDataView!no such command!$command!"
      return $::COMP_MSG_ERR_NO_SUCH_COMMAND
    }

  } ; # namespace compMsgDataView
} ; # namespace compMsg
