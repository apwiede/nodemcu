# ===========================================================================
# * Copyright (c) 2017, Arnulf P. Wiedemann (arnulf@wiedemann-pri.de)
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

set ::COMP_MSG_NO_INCR 0
set ::COMP_MSG_INCR    1
set ::COMP_MSG_DECR    -1

set ::COMP_MSG_SPEC_FIELD_SRC                       255
set ::COMP_MSG_SPEC_FIELD_GRP                       254
set ::COMP_MSG_SPEC_FIELD_DST                       253
set ::COMP_MSG_SPEC_FIELD_TOTAL_LGTH                252
set ::COMP_MSG_SPEC_FIELD_IP_ADDR                   251
set ::COMP_MSG_SPEC_FIELD_CMD_KEY                   250
set ::COMP_MSG_SPEC_FIELD_CMD_LGTH                  249
set ::COMP_MSG_SPEC_FIELD_RANDOM_NUM                248
set ::COMP_MSG_SPEC_FIELD_SEQUENCE_NUM              247
set ::COMP_MSG_SPEC_FIELD_FILLER                    246
set ::COMP_MSG_SPEC_FIELD_CRC                       245
set ::COMP_MSG_SPEC_FIELD_GUID                      244
set ::COMP_MSG_SPEC_FIELD_SRC_ID                    243
set ::COMP_MSG_SPEC_FIELD_HDR_FILLER                242
set ::COMP_MSG_SPEC_FIELD_NUM_KEY_VALUES            241

set ::COMP_MSG_SPEC_FIELD_PROVISIONING_SSID         240
set ::COMP_MSG_SPEC_FIELD_PROVISIONING_PORT         239
set ::COMP_MSG_SPEC_FIELD_PROVISIONING_IP_ADDR      238
set ::COMP_MSG_SPEC_FIELD_CLIENT_SSID               237
set ::COMP_MSG_SPEC_FIELD_CLIENT_PASSWD             236
set ::COMP_MSG_SPEC_FIELD_CLIENT_IP_ADDR            235
set ::COMP_MSG_SPEC_FIELD_CLIENT_PORT               234
set ::COMP_MSG_SPEC_FIELD_CLIENT_STATUS             233
set ::COMP_MSG_SPEC_FIELD_SSDP_IP_ADDR              232
set ::COMP_MSG_SPEC_FIELD_SSDP_PORT                 231
set ::COMP_MSG_SPEC_FIELD_SSDP_STATUS               230
set ::COMP_MSG_SPEC_FIELD_CLOUD_PORT                229
set ::COMP_MSG_SPEC_FIELD_CLOUD_HOST_1              228
set ::COMP_MSG_SPEC_FIELD_CLOUD_HOST_2              227
set ::COMP_MSG_SPEC_FIELD_CLOUD_URL_1_PART_1        226
set ::COMP_MSG_SPEC_FIELD_CLOUD_URL_1_PART_2        225
set ::COMP_MSG_SPEC_FIELD_CLOUD_URL_TENANT_ID_1     224
set ::COMP_MSG_SPEC_FIELD_CLOUD_URL_2_PART_1        223
set ::COMP_MSG_SPEC_FIELD_CLOUD_URL_2_PART_2        222
set ::COMP_MSG_SPEC_FIELD_CLOUD_URL_TENANT_ID_2     221
set ::COMP_MSG_SPEC_FIELD_CLOUD_NODE_TOKEN_1        220
set ::COMP_MSG_SPEC_FIELD_CLOUD_NODE_TOKEN_2        219
set ::COMP_MSG_SPEC_FIELD_TOTAL_CRC                 218

set ::COMP_MSG_SPEC_FIELD_OTA_PORT                  217
set ::COMP_MSG_SPEC_FIELD_OTA_ROM_PATH              216
set ::COMP_MSG_SPEC_FIELD_OTA_FS_PATH               215
set ::COMP_MSG_SPEC_FIELD_OTA_HOST                  214
set ::COMP_MSG_SPEC_FIELD_PROD_TEST_SSID            213
set ::COMP_MSG_SPEC_FIELD_PROD_TEST_PASSWD          212
set ::COMP_MSG_SPEC_FIELD_PROD_TEST_SECURITY_TYPE   211
set ::COMP_MSG_SPEC_FIELD_PROD_TEST_TARGET_PROTOCOL 210
set ::COMP_MSG_SPEC_FIELD_PROD_TEST_IP_ADDR         209
set ::COMP_MSG_SPEC_FIELD_PROD_TEST_SUBNET          208
set ::COMP_MSG_SPEC_FIELD_PROD_TEST_GATEWAY         207
set ::COMP_MSG_SPEC_FIELD_PROD_TEST_DNS             206
set ::COMP_MSG_SPEC_FIELD_PROD_TEST_PING_ADDRESS    205
set ::COMP_MSG_SPEC_FIELD_PROD_TEST_STATUS          204
set ::COMP_MSG_SPEC_FIELD_HDR_RESERVE               203

set ::COMP_MSG_SPEC_FIELD_LOW                       202  ; # this must be the last entry!!

set ::COMP_MSG_FREE_FIELD_ID 0xFF

namespace eval ::compMsg {
  namespace ensemble create

  namespace export compMsgTypesAndNames

  namespace eval compMsgTypesAndNames {
    namespace ensemble create
      
    namespace export compMsgTypesAndNames freeCompMsgTypesAndNames getFieldNameIdFromStr
    namespace export getFieldNameStrFromId getFileNameTokenIdFromStr getSpecialFieldNameIntFromId

    variable specialFieldNames2Ids
    set specialFieldNames2Ids [dict create]
    dict set specialFieldNames2Ids "@src"                     COMP_MSG_SPEC_FIELD_SRC
    dict set specialFieldNames2Ids "@grp"                     COMP_MSG_SPEC_FIELD_grp
    dict set specialFieldNames2Ids "@dst"                     COMP_MSG_SPEC_FIELD_DST
    dict set specialFieldNames2Ids "@totalLgth"               COMP_MSG_SPEC_FIELD_TOTAL_LGTH
    dict set specialFieldNames2Ids "@ipAddr"                  COMP_MSG_SPEC_FIELD_IP_ADDR
    dict set specialFieldNames2Ids "@cmdKey"                  COMP_MSG_SPEC_FIELD_CMD_KEY
    dict set specialFieldNames2Ids "@cmdLgth"                 COMP_MSG_SPEC_FIELD_CMD_LGTH
    dict set specialFieldNames2Ids "@randomNum"               COMP_MSG_SPEC_FIELD_RANDOM_NUM
    dict set specialFieldNames2Ids "@sequenceNum"             COMP_MSG_SPEC_FIELD_SEQUENCE_NUM
    dict set specialFieldNames2Ids "@filler"                  COMP_MSG_SPEC_FIELD_FILLER
    dict set specialFieldNames2Ids "@crc"                     COMP_MSG_SPEC_FIELD_CRC
    dict set specialFieldNames2Ids "@GUID"                    COMP_MSG_SPEC_FIELD_GUID
    dict set specialFieldNames2Ids "@srcId"                   COMP_MSG_SPEC_FIELD_SRC_ID
    dict set specialFieldNames2Ids "@hdrFiller"               COMP_MSG_SPEC_FIELD_HDR_FILLER 
    dict set specialFieldNames2Ids "@numKeyValues"            COMP_MSG_SPEC_FIELD_NUM_KEY_VALUES
    dict set specialFieldNames2Ids "@provisioningSsid"        COMP_MSG_SPEC_FIELD_PROVISIONING_SSID
    dict set specialFieldNames2Ids "@provisioningPort"        COMP_MSG_SPEC_FIELD_PROVISIONING_PORT
    dict set specialFieldNames2Ids "@provisioningIPAddr"      COMP_MSG_SPEC_FIELD_PROVISIONING_IP_ADDR
    dict set specialFieldNames2Ids "@clientSsid"              COMP_MSG_SPEC_FIELD_CLIENT_SSID
    dict set specialFieldNames2Ids "@clientPasswd"            COMP_MSG_SPEC_FIELD_CLIENT_PASSWD
    dict set specialFieldNames2Ids "@clientIPAddr"            COMP_MSG_SPEC_FIELD_CLIENT_IP_ADDR
    dict set specialFieldNames2Ids "@clientPort"              COMP_MSG_SPEC_FIELD_CLIENT_PORT
    dict set specialFieldNames2Ids "@clientStatus"            COMP_MSG_SPEC_FIELD_CLIENT_STATUS
    dict set specialFieldNames2Ids "@SSDPIPAddr"              COMP_MSG_SPEC_FIELD_SSDP_IP_ADDR
    dict set specialFieldNames2Ids "@SSDPPort"                COMP_MSG_SPEC_FIELD_SSDP_PORT
    dict set specialFieldNames2Ids "@SSDPStatus"              COMP_MSG_SPEC_FIELD_SSDP_STATUS
    dict set specialFieldNames2Ids "@cloudPort"               COMP_MSG_SPEC_FIELD_CLOUD_PORT
    dict set specialFieldNames2Ids "@cloudHost1"              COMP_MSG_SPEC_FIELD_CLOUD_HOST_1
    dict set specialFieldNames2Ids "@cloudHost2"              COMP_MSG_SPEC_FIELD_CLOUD_HOST_2
    dict set specialFieldNames2Ids "@cloudUrl1Part1"          COMP_MSG_SPEC_FIELD_CLOUD_URL_1_PART_1
    dict set specialFieldNames2Ids "@cloudUrl1Part2"          COMP_MSG_SPEC_FIELD_CLOUD_URL_1_PART_2
    dict set specialFieldNames2Ids "@cloudUrlTenantId1"       COMP_MSG_SPEC_FIELD_CLOUD_URL_TENANT_ID_1
    dict set specialFieldNames2Ids "@cloudUrl2Part1"          COMP_MSG_SPEC_FIELD_CLOUD_URL_2_PART_1
    dict set specialFieldNames2Ids "@cloudUrl2Part2"          COMP_MSG_SPEC_FIELD_CLOUD_URL_2_PART_2
    dict set specialFieldNames2Ids "@cloudUrlTenantId2"       COMP_MSG_SPEC_FIELD_CLOUD_URL_TENANT_ID_2
    dict set specialFieldNames2Ids "@cloudNodeToken1"         COMP_MSG_SPEC_FIELD_CLOUD_NODE_TOKEN_1
    dict set specialFieldNames2Ids "@cloudNodeToken2"         COMP_MSG_SPEC_FIELD_CLOUD_NODE_TOKEN_2
    dict set specialFieldNames2Ids "@totalCrc"                COMP_MSG_SPEC_FIELD_TOTAL_CRC
    dict set specialFieldNames2Ids "@otaPort"                 COMP_MSG_SPEC_FIELD_OTA_PORT
    dict set specialFieldNames2Ids "@otaRomPath"              COMP_MSG_SPEC_FIELD_OTA_ROM_PATH
    dict set specialFieldNames2Ids "@otaFsPath"               COMP_MSG_SPEC_FIELD_OTA_FS_PATH
    dict set specialFieldNames2Ids "@otaHost"                 COMP_MSG_SPEC_FIELD_OTA_HOST
    dict set specialFieldNames2Ids "@prodTestSsid"            COMP_MSG_SPEC_FIELD_PROD_TEST_SSID
    dict set specialFieldNames2Ids "@prodTestPasswd"          COMP_MSG_SPEC_FIELD_PROD_TEST_PASSWD
    dict set specialFieldNames2Ids "@prodTestSecurityType"    COMP_MSG_SPEC_FIELD_PROD_TEST_SECURITY_TYPE
    dict set specialFieldNames2Ids "@prodTestTargetProtocol"  COMP_MSG_SPEC_FIELD_PROD_TEST_TARGET_PROTOCOL
    dict set specialFieldNames2Ids "@prodTestIpAddr"          COMP_MSG_SPEC_FIELD_PROD_TEST_IP_ADDR
    dict set specialFieldNames2Ids "@prodTestSubnet"          COMP_MSG_SPEC_FIELD_PROD_TEST_SUBNET
    dict set specialFieldNames2Ids "@prodTestGateway"         COMP_MSG_SPEC_FIELD_PROD_TEST_GATEWAY
    dict set specialFieldNames2Ids "@prodTestDns"             COMP_MSG_SPEC_FIELD_PROD_TEST_DNS
    dict set specialFieldNames2Ids "@prodTestPingAddress"     COMP_MSG_SPEC_FIELD_PROD_TEST_PING_ADDRESS
    dict set specialFieldNames2Ids "@prodTestStatus"          COMP_MSG_SPEC_FIELD_PROD_TEST_STATUS
    dict set specialFieldNames2Ids "@hdrReserve"              COMP_MSG_SPEC_FIELD_HDR_RESERVE

    variable specialFieldId2Names
    set specialFieldId2Names [dict create]
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_SRC                       "@src"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_grp                       "@grp"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_DST                       "@dst"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_TOTAL_LGTH                "@totalLgth"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_IP_ADDR                   "@ipAddr"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_CMD_KEY                   "@cmdKey"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_CMD_LGTH                  "@cmdLgth"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_RANDOM_NUM                "@randomNum"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_SEQUENCE_NUM              "@sequenceNum"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_FILLER                    "@filler"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_CRC                       "@crc"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_GUID                      "@GUID"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_SRC_ID                    "@srcId"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_HDR_FILLER                "@hdrFiller"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_NUM_KEY_VALUES            "@numKeyValues"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_PROVISIONING_SSID         "@provisioningSsid"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_PROVISIONING_PORT         "@provisioningPort"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_PROVISIONING_IP_ADDR      "@provisioningIPAddr"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_CLIENT_SSID               "@clientSsid"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_CLIENT_PASSWD             "@clientPasswd"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_CLIENT_IP_ADDR            "@clientIPAddr"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_CLIENT_PORT               "@clientPort"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_CLIENT_STATUS             "@clientStatus"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_SSDP_IP_ADDR              "@SSDPIPAddr"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_SSDP_PORT                 "@SSDPPort"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_SSDP_STATUS               "@SSDPStatus"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_CLOUD_PORT                "@cloudPort"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_CLOUD_HOST_1              "@cloudHost1"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_CLOUD_HOST_2              "@cloudHost2"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_CLOUD_URL_1_PART_1        "@cloudUrl1Part1"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_CLOUD_URL_1_PART_2        "@cloudUrl1Part2"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_CLOUD_URL_TENANT_ID_1     "@cloudUrlTenantId1"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_CLOUD_URL_2_PART_1        "@cloudUrl2Part1"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_CLOUD_URL_2_PART_2        "@cloudUrl2Part2"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_CLOUD_URL_TENANT_ID_2     "@cloudUrlTenantId2"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_CLOUD_NODE_TOKEN_1        "@cloudNodeToken1"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_CLOUD_NODE_TOKEN_2        "@cloudNodeToken2"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_TOTAL_CRC                 "@totalCrc"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_OTA_PORT                  "@otaPort"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_OTA_ROM_PATH              "@otaRomPath"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_OTA_FS_PATH               "@otaFsPath"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_OTA_HOST                  "@otaHost"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_PROD_TEST_SSID            "@prodTestSsid"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_PROD_TEST_PASSWD          "@prodTestPasswd"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_PROD_TEST_SECURITY_TYPE   "@prodTestSecurityType"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_PROD_TEST_TARGET_PROTOCOL "@prodTestTargetProtocol"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_PROD_TEST_IP_ADDR         "@prodTestIpAddr"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_PROD_TEST_SUBNET          "@prodTestSubnet"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_PROD_TEST_GATEWAY         "@prodTestGateway"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_PROD_TEST_DNS             "@prodTestDns"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_PROD_TEST_PING_ADDRESS    "@prodTestPingAddress"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_PROD_TEST_STATUS          "@prodTestStatus"
    dict set specialFieldId2Names COMP_MSG_SPEC_FIELD_HDR_RESERVE               "@hdrReserve"

    variable specialFieldIds2Ints
    set specialFieldIds2Ints [dict create]
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_SRC                       255
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_grp                       254
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_DST                       253
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_TOTAL_LGTH                252
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_IP_ADDR                   251
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_CMD_KEY                   250
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_CMD_LGTH                  249
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_RANDOM_NUM                248
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_SEQUENCE_NUM              247
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_FILLER                    246
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_CRC                       245
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_GUID                      244
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_SRC_ID                    243
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_HDR_FILLER                242
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_NUM_KEY_VALUES            241

    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_PROVISIONING_SSID         240
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_PROVISIONING_PORT         239
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_PROVISIONING_IP_ADDR      238
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_CLIENT_SSID               237
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_CLIENT_PASSWD             236
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_CLIENT_IP_ADDR            235
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_CLIENT_PORT               234
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_CLIENT_STATUS             233
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_SSDP_IP_ADDR              232
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_SSDP_PORT                 231
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_SSDP_STATUS               230
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_CLOUD_PORT                229
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_CLOUD_HOST_1              228
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_CLOUD_HOST_2              227
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_CLOUD_URL_1_PART_1        226
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_CLOUD_URL_1_PART_2        225
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_CLOUD_URL_TENANT_ID_1     224
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_CLOUD_URL_2_PART_1        223
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_CLOUD_URL_2_PART_2        222
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_CLOUD_URL_TENANT_ID_2     221
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_CLOUD_NODE_TOKEN_1        220
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_CLOUD_NODE_TOKEN_2        219
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_TOTAL_CRC                 218

    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_OTA_PORT                  217
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_OTA_ROM_PATH              216
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_OTA_FS_PATH               215
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_OTA_HOST                  214
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_PROD_TEST_SSID            213
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_PROD_TEST_PASSWD          212
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_PROD_TEST_SECURITY_TYPE   211
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_PROD_TEST_TARGET_PROTOCOL 210
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_PROD_TEST_IP_ADDR         209
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_PROD_TEST_SUBNET          208
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_PROD_TEST_GATEWAY         207
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_PROD_TEST_DNS             206
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_PROD_TEST_PING_ADDRESS    205
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_PROD_TEST_STATUS          204
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_HDR_RESERVE               203

    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_LOW                       202  ; # this must be the last entry!!

    variable specialFieldInts2Ids
    set specialFieldInts2Ids [dict create]
    dict set specialFieldInts2Ids 255 COMP_MSG_SPEC_FIELD_SRC                       
    dict set specialFieldInts2Ids 254 COMP_MSG_SPEC_FIELD_grp                       
    dict set specialFieldInts2Ids 253 COMP_MSG_SPEC_FIELD_DST                       
    dict set specialFieldInts2Ids 252 COMP_MSG_SPEC_FIELD_TOTAL_LGTH                
    dict set specialFieldInts2Ids 251 COMP_MSG_SPEC_FIELD_IP_ADDR                   
    dict set specialFieldInts2Ids 250 COMP_MSG_SPEC_FIELD_CMD_KEY                   
    dict set specialFieldInts2Ids 249 COMP_MSG_SPEC_FIELD_CMD_LGTH                  
    dict set specialFieldInts2Ids 248 COMP_MSG_SPEC_FIELD_RANDOM_NUM                
    dict set specialFieldInts2Ids 247 COMP_MSG_SPEC_FIELD_SEQUENCE_NUM              
    dict set specialFieldInts2Ids 246 COMP_MSG_SPEC_FIELD_FILLER                    
    dict set specialFieldInts2Ids 245 COMP_MSG_SPEC_FIELD_CRC                       
    dict set specialFieldInts2Ids 244 COMP_MSG_SPEC_FIELD_GUID                      
    dict set specialFieldInts2Ids 243 COMP_MSG_SPEC_FIELD_SRC_ID                    
    dict set specialFieldInts2Ids 242 COMP_MSG_SPEC_FIELD_HDR_FILLER                
    dict set specialFieldInts2Ids 241 COMP_MSG_SPEC_FIELD_NUM_KEY_VALUES            
    dict set specialFieldInts2Ids 240 COMP_MSG_SPEC_FIELD_PROVISIONING_SSID         
    dict set specialFieldInts2Ids 239 COMP_MSG_SPEC_FIELD_PROVISIONING_PORT         
    dict set specialFieldInts2Ids 238 COMP_MSG_SPEC_FIELD_PROVISIONING_IP_ADDR      
    dict set specialFieldInts2Ids 237 COMP_MSG_SPEC_FIELD_CLIENT_SSID               
    dict set specialFieldInts2Ids 236 COMP_MSG_SPEC_FIELD_CLIENT_PASSWD             
    dict set specialFieldInts2Ids 235 COMP_MSG_SPEC_FIELD_CLIENT_IP_ADDR            
    dict set specialFieldInts2Ids 234 COMP_MSG_SPEC_FIELD_CLIENT_PORT               
    dict set specialFieldInts2Ids 233 COMP_MSG_SPEC_FIELD_CLIENT_STATUS             
    dict set specialFieldInts2Ids 232 COMP_MSG_SPEC_FIELD_SSDP_IP_ADDR              
    dict set specialFieldInts2Ids 231 COMP_MSG_SPEC_FIELD_SSDP_PORT                 
    dict set specialFieldInts2Ids 230 COMP_MSG_SPEC_FIELD_SSDP_STATUS               
    dict set specialFieldInts2Ids 229 COMP_MSG_SPEC_FIELD_CLOUD_PORT                
    dict set specialFieldInts2Ids 228 COMP_MSG_SPEC_FIELD_CLOUD_HOST_1              
    dict set specialFieldInts2Ids 227 COMP_MSG_SPEC_FIELD_CLOUD_HOST_2              
    dict set specialFieldInts2Ids 226 COMP_MSG_SPEC_FIELD_CLOUD_URL_1_PART_1        
    dict set specialFieldInts2Ids 225 COMP_MSG_SPEC_FIELD_CLOUD_URL_1_PART_2        
    dict set specialFieldInts2Ids 224 COMP_MSG_SPEC_FIELD_CLOUD_URL_TENANT_ID_1     
    dict set specialFieldInts2Ids 223 COMP_MSG_SPEC_FIELD_CLOUD_URL_2_PART_1        
    dict set specialFieldInts2Ids 222 COMP_MSG_SPEC_FIELD_CLOUD_URL_2_PART_2        
    dict set specialFieldInts2Ids 221 COMP_MSG_SPEC_FIELD_CLOUD_URL_TENANT_ID_2     
    dict set specialFieldInts2Ids 220 COMP_MSG_SPEC_FIELD_CLOUD_NODE_TOKEN_1        
    dict set specialFieldInts2Ids 219 COMP_MSG_SPEC_FIELD_CLOUD_NODE_TOKEN_2        
    dict set specialFieldInts2Ids 218 COMP_MSG_SPEC_FIELD_TOTAL_CRC                 
    dict set specialFieldInts2Ids 217 COMP_MSG_SPEC_FIELD_OTA_PORT                  
    dict set specialFieldInts2Ids 216 COMP_MSG_SPEC_FIELD_OTA_ROM_PATH              
    dict set specialFieldInts2Ids 215 COMP_MSG_SPEC_FIELD_OTA_FS_PATH               
    dict set specialFieldInts2Ids 214 COMP_MSG_SPEC_FIELD_OTA_HOST                  
    dict set specialFieldInts2Ids 213 COMP_MSG_SPEC_FIELD_PROD_TEST_SSID            
    dict set specialFieldInts2Ids 212 COMP_MSG_SPEC_FIELD_PROD_TEST_PASSWD          
    dict set specialFieldInts2Ids 211 COMP_MSG_SPEC_FIELD_PROD_TEST_SECURITY_TYPE   
    dict set specialFieldInts2Ids 210 COMP_MSG_SPEC_FIELD_PROD_TEST_TARGET_PROTOCOL 
    dict set specialFieldInts2Ids 209 COMP_MSG_SPEC_FIELD_PROD_TEST_IP_ADDR         
    dict set specialFieldInts2Ids 208 COMP_MSG_SPEC_FIELD_PROD_TEST_SUBNET          
    dict set specialFieldInts2Ids 207 COMP_MSG_SPEC_FIELD_PROD_TEST_GATEWAY         
    dict set specialFieldInts2Ids 206 COMP_MSG_SPEC_FIELD_PROD_TEST_DNS             
    dict set specialFieldInts2Ids 205 COMP_MSG_SPEC_FIELD_PROD_TEST_PING_ADDRESS    
    dict set specialFieldInts2Ids 204 COMP_MSG_SPEC_FIELD_PROD_TEST_STATUS          
    dict set specialFieldInts2Ids 203 COMP_MSG_SPEC_FIELD_HDR_RESERVE               

    dict set specialFieldInts2Ids 202 COMP_MSG_SPEC_FIELD_LOW  ; # this must be the last entry!!

    variable specialFieldNameTokens2Ids
    set specialFieldNameTokens2Ids [dict create]
    dict set specialFieldNameTokens2Ids "@\$msgUse"           COMP_MSG_USE_FILE_TOKEN
    dict set specialFieldNameTokens2Ids "@\$msgDescHeader"    COMP_MSG_DESC_HEADER_FILE_TOKEN
    dict set specialFieldNameTokens2Ids "@\$msgDescMidPart"   COMP_MSG_DESC_MID_PART_FILE_TOKEN
    dict set specialFieldNameTokens2Ids "@\$msgDescTrailer"   COMP_MSG_DESC_TRAILER_FILE_TOKEN
    dict set specialFieldNameTokens2Ids "@\$msgFieldsToSave"  COMP_MSG_FIELDS_TO_SAVE_FILE_TOKEN
    dict set specialFieldNameTokens2Ids "@\$msgHeads"         COMP_MSG_HEADS_FILE_TOKEN
    dict set specialFieldNameTokens2Ids "@\$msgActions"       COMP_MSG_ACTIONS_FILE_TOKEN
    dict set specialFieldNameTokens2Ids "@\$msgValHeader"     COMP_MSG_VAL_HEADER_FILE_TOKEN
    dict set specialFieldNameTokens2Ids "@\$msgWifiData"      COMP_MSG_WIFI_DATA_VALUES_FILE_TOKEN
    dict set specialFieldNameTokens2Ids "@\$msgModuleData"    COMP_MSG_MODULE_DATA_VALUES_FILE_TOKEN

    dict set ::compMsg(fieldNameDefinitions) numDefinitions 0

    # ================================= getSpecialFieldNameIntFromId ====================================
    
    proc getSpecialFieldNameIntFromId {fieldNameId fieldNameIntVar} {
      upvar $fieldNameIntVar fieldNameInt
      variable specialFieldIds2Ints

      if {[dict exists $specialFieldIds2Ints $fieldNameId]} {
        set fieldNameInt [dict get $specialFieldId2Ints $fieldNameId]
        return [checkErrOK OK]
      }
puts stderr "getSpecialFieldNameIntFromId: $fieldNameId!"
      checkErrOK BAD_SPECIAL_FIELD
    }

    # ================================= getFieldNameIdFromStr ====================================
    
    proc getFieldNameIdFromStr {fieldName fieldNameIdVar incrVal} {
      upvar $fieldNameIdVar fieldNameId
      variable specialFieldNames2Ids

      if {[string range $fieldName 0 0] eq "@"} {
        # find special field name
        if {[dict exists $specialFieldNames2Ids $fieldName]} {
          set fieldNameId [dict get $specialFieldNames2Ids $fieldName]
          return [checkErrOK OK]
        }
puts stderr "getSpecialFieldNameIntFromStr: $fieldName!"
        checkErrOK BAD_SPECIAL_FIELD
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
              return [checkErrOK OK]
            }
            if {($incrVal == $::COMP_MSG_INCR) && ([dict get $entry id] eq $::COMP_MSG_FREE_FIELD_ID) && ($firstFreeEntry eq "")} {
              dict set firstFreeEntry id [expr {$nameIdx + 1}]
            }
            incr nameIdx
          }
        }
        if {$incrVal == $::COMP_MSG_DECR} {
          return [checkErrOK OK] ; # just ignore silently
        } else {
          if {$incrVal == $::COMP_MSG_NO_INCR} {
puts stderr "field not found: $fieldName!incrVal: $incrVal!"
            checkErrOK FIELD_NOT_FOUND
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
      return [checkErrOK OK]
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
        return [checkErrOK OK]
      }
      # find field name
      set idx 0
      set fieldNameDefinitions $::compMsg(fieldNameDefinitions)
      while {$idx < [dict get $fieldNameDefinitions numDefinitions]} {
        set nameEntry [lindex  [dict get $fieldNameDefinitions definitions] $idx]
        if {[dict get $nameEntry id] == $fieldNameId} {
          set fieldName [dict get $nameEntry fieldName]
          return [checkErrOK OK]
        }
        incr idx
      }
      checkErrOK FIELD_NOT_FOUND
    }
    
    # ================================= getFileNameTokenIdFromStr ====================================

    proc getFileNameTokenIdFromStr {fileNameTokenStr fileNameTokenIdVar} {
      upvar $fileNameTokenIdVar fileNameTokenId
      variable specialFieldNameTokens2Ids

      set LB "\{"
      set RB "\}"
      if {[dict exists $specialFieldNameTokens2Ids $fileNameTokenStr]} {
        set fileNameTokenId [dict get $specialFieldNameTokens2Ids $fileNameTokenStr]
        return [checkErrOK OK]
      }
      checkErrOK FILE_NAME_TOKEN_NOT_FOUND
    }

  } ; # namespace compMsgTypesAndNames
} ; # namespace compMsg
