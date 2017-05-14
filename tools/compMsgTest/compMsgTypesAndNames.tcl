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

set ::COMP_MSG_FREE_FIELD_ID 0xFF

namespace eval ::compMsg {
  namespace ensemble create

  namespace export compMsgTypesAndNames

  namespace eval compMsgTypesAndNames {
    namespace ensemble create

    namespace export compMsgTypesAndNames freeCompMsgTypesAndNames getFieldNameIdFromStr
    namespace export getFieldNameStrFromId getFileNameTokenIdFromStr getSpecialFieldNameIntFromId
    namespace export compMsgTypesAndNamesInit addMsgFieldInfos getMsgFieldInfo setMsgFieldInfo

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
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_NONE                      0
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_SRC                       1
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_grp                       2
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_DST                       3
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_TOTAL_LGTH                4
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_IP_ADDR                   5
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_CMD_KEY                   6
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_CMD_LGTH                  7
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_RANDOM_NUM                8
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_SEQUENCE_NUM              9
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_FILLER                    10
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_CRC                       11
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_GUID                      12
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_SRC_ID                    13
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_HDR_FILLER                14
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_NUM_KEY_VALUES            15

    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_PROVISIONING_SSID         16
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_PROVISIONING_PORT         17
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_PROVISIONING_IP_ADDR      18
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_CLIENT_SSID               19
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_CLIENT_PASSWD             20
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_CLIENT_IP_ADDR            21
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_CLIENT_PORT               22
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_CLIENT_STATUS             23
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_SSDP_IP_ADDR              24
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_SSDP_PORT                 25
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_SSDP_STATUS               26
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_CLOUD_SECURE_CONNECT      27
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_CLOUD_PORT                28
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_CLOUD_HOST_1              29
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_CLOUD_HOST_2              30
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_CLOUD_URL_1_PART_1        31
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_CLOUD_URL_1_PART_2        32
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_CLOUD_URL_TENANT_ID_1     33
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_CLOUD_URL_2_PART_1        34
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_CLOUD_URL_2_PART_2        35
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_CLOUD_URL_TENANT_ID_2     36
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_CLOUD_NODE_TOKEN_1        37
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_CLOUD_NODE_TOKEN_2        38
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_TOTAL_CRC                 39

    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_OTA_PORT                  40
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_OTA_ROM_PATH              41
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_OTA_FS_PATH               42
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_OTA_HOST                  43
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_PROD_TEST_SSID            44
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_PROD_TEST_PASSWD          45
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_PROD_TEST_SECURITY_TYPE   46
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_PROD_TEST_TARGET_PROTOCOL 47
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_PROD_TEST_IP_ADDR         48
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_PROD_TEST_SUBNET          49
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_PROD_TEST_GATEWAY         50
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_PROD_TEST_DNS             51
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_PROD_TEST_PING_ADDRESS    52
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_PROD_TEST_STATUS          53
    dict set specialFieldIds2Ints COMP_MSG_SPEC_FIELD_HDR_RESERVE               54

    variable specialFieldInts2Ids
    set specialFieldInts2Ids [dict create]
    dict set specialFieldInts2Ids 0 COMP_MSG_SPEC_FIELD_NONE
    dict set specialFieldInts2Ids 1 COMP_MSG_SPEC_FIELD_SRC
    dict set specialFieldInts2Ids 2 COMP_MSG_SPEC_FIELD_grp
    dict set specialFieldInts2Ids 3 COMP_MSG_SPEC_FIELD_DST
    dict set specialFieldInts2Ids 4 COMP_MSG_SPEC_FIELD_TOTAL_LGTH
    dict set specialFieldInts2Ids 5 COMP_MSG_SPEC_FIELD_IP_ADDR
    dict set specialFieldInts2Ids 6 COMP_MSG_SPEC_FIELD_CMD_KEY
    dict set specialFieldInts2Ids 7 COMP_MSG_SPEC_FIELD_CMD_LGTH
    dict set specialFieldInts2Ids 8 COMP_MSG_SPEC_FIELD_RANDOM_NUM
    dict set specialFieldInts2Ids 9 COMP_MSG_SPEC_FIELD_SEQUENCE_NUM
    dict set specialFieldInts2Ids 10 COMP_MSG_SPEC_FIELD_FILLER
    dict set specialFieldInts2Ids 11 COMP_MSG_SPEC_FIELD_CRC
    dict set specialFieldInts2Ids 12 COMP_MSG_SPEC_FIELD_GUID
    dict set specialFieldInts2Ids 13 COMP_MSG_SPEC_FIELD_SRC_ID
    dict set specialFieldInts2Ids 14 COMP_MSG_SPEC_FIELD_HDR_FILLER
    dict set specialFieldInts2Ids 15 COMP_MSG_SPEC_FIELD_NUM_KEY_VALUES
    dict set specialFieldInts2Ids 16 COMP_MSG_SPEC_FIELD_PROVISIONING_SSID
    dict set specialFieldInts2Ids 17 COMP_MSG_SPEC_FIELD_PROVISIONING_PORT
    dict set specialFieldInts2Ids 18 COMP_MSG_SPEC_FIELD_PROVISIONING_IP_ADDR
    dict set specialFieldInts2Ids 19 COMP_MSG_SPEC_FIELD_CLIENT_SSID
    dict set specialFieldInts2Ids 20 COMP_MSG_SPEC_FIELD_CLIENT_PASSWD
    dict set specialFieldInts2Ids 21 COMP_MSG_SPEC_FIELD_CLIENT_IP_ADDR
    dict set specialFieldInts2Ids 22 COMP_MSG_SPEC_FIELD_CLIENT_PORT
    dict set specialFieldInts2Ids 23 COMP_MSG_SPEC_FIELD_CLIENT_STATUS
    dict set specialFieldInts2Ids 24 COMP_MSG_SPEC_FIELD_SSDP_IP_ADDR
    dict set specialFieldInts2Ids 25 COMP_MSG_SPEC_FIELD_SSDP_PORT
    dict set specialFieldInts2Ids 26 COMP_MSG_SPEC_FIELD_SSDP_STATUS
    dict set specialFieldInts2Ids 27 COMP_MSG_SPEC_FIELD_CLOUD_SECURE_CONNECT
    dict set specialFieldInts2Ids 28 COMP_MSG_SPEC_FIELD_CLOUD_PORT
    dict set specialFieldInts2Ids 29 COMP_MSG_SPEC_FIELD_CLOUD_HOST_1
    dict set specialFieldInts2Ids 30 COMP_MSG_SPEC_FIELD_CLOUD_HOST_2
    dict set specialFieldInts2Ids 31 COMP_MSG_SPEC_FIELD_CLOUD_URL_1_PART_1
    dict set specialFieldInts2Ids 32 COMP_MSG_SPEC_FIELD_CLOUD_URL_1_PART_2
    dict set specialFieldInts2Ids 33 COMP_MSG_SPEC_FIELD_CLOUD_URL_TENANT_ID_1
    dict set specialFieldInts2Ids 34 COMP_MSG_SPEC_FIELD_CLOUD_URL_2_PART_1
    dict set specialFieldInts2Ids 35 COMP_MSG_SPEC_FIELD_CLOUD_URL_2_PART_2
    dict set specialFieldInts2Ids 36 COMP_MSG_SPEC_FIELD_CLOUD_URL_TENANT_ID_2
    dict set specialFieldInts2Ids 37 COMP_MSG_SPEC_FIELD_CLOUD_NODE_TOKEN_1
    dict set specialFieldInts2Ids 38 COMP_MSG_SPEC_FIELD_CLOUD_NODE_TOKEN_2
    dict set specialFieldInts2Ids 39 COMP_MSG_SPEC_FIELD_TOTAL_CRC
    dict set specialFieldInts2Ids 40 COMP_MSG_SPEC_FIELD_OTA_PORT
    dict set specialFieldInts2Ids 41 COMP_MSG_SPEC_FIELD_OTA_ROM_PATH
    dict set specialFieldInts2Ids 42 COMP_MSG_SPEC_FIELD_OTA_FS_PATH
    dict set specialFieldInts2Ids 43 COMP_MSG_SPEC_FIELD_OTA_HOST
    dict set specialFieldInts2Ids 44 COMP_MSG_SPEC_FIELD_PROD_TEST_SSID
    dict set specialFieldInts2Ids 45 COMP_MSG_SPEC_FIELD_PROD_TEST_PASSWD
    dict set specialFieldInts2Ids 46 COMP_MSG_SPEC_FIELD_PROD_TEST_SECURITY_TYPE
    dict set specialFieldInts2Ids 47 COMP_MSG_SPEC_FIELD_PROD_TEST_TARGET_PROTOCOL
    dict set specialFieldInts2Ids 48 COMP_MSG_SPEC_FIELD_PROD_TEST_IP_ADDR
    dict set specialFieldInts2Ids 49 COMP_MSG_SPEC_FIELD_PROD_TEST_SUBNET
    dict set specialFieldInts2Ids 50 COMP_MSG_SPEC_FIELD_PROD_TEST_GATEWAY
    dict set specialFieldInts2Ids 51 COMP_MSG_SPEC_FIELD_PROD_TEST_DNS
    dict set specialFieldInts2Ids 52 COMP_MSG_SPEC_FIELD_PROD_TEST_PING_ADDRESS
    dict set specialFieldInts2Ids 53 COMP_MSG_SPEC_FIELD_PROD_TEST_STATUS
    dict set specialFieldInts2Ids 54 COMP_MSG_SPEC_FIELD_HDR_RESERVE

    variable fieldGroupStr2Ids
    set fieldGroupStr2Ids [dict create]
    dict set fieldGroupStr2Ids "@\$msgUse"           COMP_MSG_USE_FIELD_GROUP
    dict set fieldGroupStr2Ids "@\$msgDescHeader"    COMP_MSG_DESC_HEADER_FIELD_GROUP
    dict set fieldGroupStr2Ids "@\$msgDescMidPart"   COMP_MSG_DESC_MID_PART_FIELD_GROUP
    dict set fieldGroupStr2Ids "@\$msgDescTrailer"   COMP_MSG_DESC_TRAILER_FIELD_GROUP
    dict set fieldGroupStr2Ids "@\$msgFieldsToSave"  COMP_MSG_FIELDS_TO_SAVE_FIELD_GROUP
    dict set fieldGroupStr2Ids "@\$msgDescKeyValue"  COMP_MSG_DESC_KEY_VALUE_FIELD_GROUP
    dict set fieldGroupStr2Ids "@\$msgHeads"         COMP_MSG_HEADS_FIELD_GROUP
    dict set fieldGroupStr2Ids "@\$msgActions"       COMP_MSG_ACTIONS_FIELD_GROUP
    dict set fieldGroupStr2Ids "@\$msgValHeader"     COMP_MSG_VAL_HEADER_FIELD_GROUP
    dict set fieldGroupStr2Ids "@\$msgWifiData"      COMP_MSG_WIFI_DATA_VALUES_FIELD_GROUP
    dict set fieldGroupStr2Ids "@\$msgModuleData"    COMP_MSG_MODULE_DATA_VALUES_FIELD_GROUP
    dict set fieldGroupStr2Ids "@\$msgDesc"          COMP_MSG_DESC_FIELD_GROUP
    dict set fieldGroupStr2Ids "@\$msgVal"           COMP_MSG_VAL_FIELD_GROUP

    dict set ::compMsg(fieldNameDefinitions) numDefinitions 0

    # ================================= getSpecialFieldNameIntFromId ====================================

    proc getSpecialFieldNameIntFromId {fieldNameId fieldNameIntVar} {
      upvar $fieldNameIntVar fieldNameInt
      variable specialFieldIds2Ints

      if {[dict exists $specialFieldIds2Ints $fieldNameId]} {
        set fieldNameInt [dict get $specialFieldIds2Ints $fieldNameId]
        return [checkErrOK OK]
      }
puts stderr "getSpecialFieldNameIntFromId: $fieldNameId!"
      checkErrOK BAD_SPECIAL_FIELD
    }

    # ================================= getFieldNameIdFromStr ====================================

    proc getFieldNameIdFromStr {compMsgDispatcherVar fieldName fieldNameIdVar incrVal} {
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $fieldNameIdVar fieldNameId
      variable specialFieldNames2Ids

      set ch1 [string range $fieldName 0 0]
      set ch2 [string range $fieldName 1 1]
      if {($ch1 eq "@") && ($ch2 ne "#") && ($ch2 ne "\$")} {
        # find special field name
        if {[dict exists $specialFieldNames2Ids $fieldName]} {
          set fieldNameId [dict get $specialFieldNames2Ids $fieldName]
          return [checkErrOK OK]
        }
        checkErrOK BAD_SPECIAL_FIELD
      } else {
        set firstFreeEntry [list]
        set firstFreeEntryIdx 0
        set numDefinitions [dict get $::compMsg(fieldNameDefinitions) numDefinitions]
	set compMsgTypesAndNames [dict get $compMsgDispatcher compMsgTypesAndNames]
        if {$numDefinitions > 0} {
          # find fieldName
          set nameIdx 0
          set definitions [dict get $::compMsg(fieldNameDefinitions) definitions]
#puts stderr "numDefinitions: $numDefinitions!"
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
#puts stderr "fieldName: $fieldName!id: [dict get $entry id]!"
              set fieldNameId [expr {[dict get $entry id] + [dict get $compMsgTypesAndNames numSpecFieldIds]}]
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
              set fieldNameId [expr {[dict get $firstFreeEntry id] + [dict get $compMsgTypesAndNames numSpecFieldIds]}]
              dict set firstFreeEntry refCnt 1
              dict set firstFreeEntry fieldName $fieldName
              set definitions [lreplace $definitions $firstFreeEntryIdx $firstFreeEntryIdx $firstFreeEntry]
            } else {
              # add a free slot in msgFieldInfos
              set result [::compMsg compMsgTypesAndNames addMsgFieldInfos compMsgDispatcher 1]
              checkErrOK $result
              set result [::compMsg compMsgDataValue addMsgFieldValues compMsgDispatcher 1]
              checkErrOK $result
              incr numDefinitions
              dict set ::compMsg(fieldNameDefinitions) numDefinitions $numDefinitions
              set entry [dict create]
              dict set entry refCnt 1
              dict set entry id $numDefinitions
              dict set entry fieldName $fieldName
              set fieldNameId [expr {$numDefinitions + [dict get $compMsgTypesAndNames numSpecFieldIds]}]
#puts stderr "fieldName: $fieldName!fieldNameId: $fieldNameId!"
              lappend definitions $entry
            }
          }
        }
        dict set ::compMsg(fieldNameDefinitions) definitions $definitions
      }
      return [checkErrOK OK]
    }

    # ================================= getFieldNameStrFromId ====================================

    proc getFieldNameStrFromId {compMsgDispatcherVar fieldNameId fieldNameVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher
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
      # these field names have also ids starting at 1 but for fieldInfo handling we add the numSpecFieldIds to get
      # unique ids!
      set compMsgTypesAndNames [dict get $compMsgDispatcher compMsgTypesAndNames]
#puts stderr "fieldNameId: $fieldNameId![dict get $compMsgTypesAndNames numSpecFieldIds]!"
      if {![string is integer $fieldNameId]} {
      }
      set fieldNameId [expr {$fieldNameId - [dict get $compMsgTypesAndNames numSpecFieldIds]}]
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
      variable fieldGroupStr2Ids

      set LB "\{"
      set RB "\}"
      if {[dict exists $fieldGroupStr2Ids $fileNameTokenStr]} {
        set fileNameTokenId [dict get $fieldGroupStr2Ids $fileNameTokenStr]
        return [checkErrOK OK]
      }
puts stderr "ERROR fileNameTokenStr: $fileNameTokenStr!FILE_NAME_TOKEN_NOT_FOUND"
      checkErrOK FILE_NAME_TOKEN_NOT_FOUND
    }

    # ================================= addMsgFieldInfos ====================================

    proc addMsgFieldInfos {compMsgDispatcherVar numEntries} {
      upvar $compMsgDispatcherVar compMsgDispatcher

      set result $::COMP_MSG_ERR_OK
      set msgFieldInfos [dict get $compMsgDispatcher compMsgTypesAndNames msgFieldInfos]
      set fieldInfos [dict get $msgFieldInfos fieldInfos]
      set idx 0
      while {$idx < $numEntries} {
        lappend fieldInfos [list]
	dict incr msgFieldInfos numMsgFields
	incr idx
      }
      dict set msgFieldInfos fieldInfos $fieldInfos
      dict set compMsgDispatcher compMsgTypesAndNames msgFieldInfos $msgFieldInfos
#puts stderr "addMsgFieldInfos:$msgFieldInfos"
      return $result
    }

    # ================================= getMsgFieldInfo ====================================

    proc getMsgFieldInfo {compMsgDispatcherVar idx fieldInfoVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $fieldInfoVar fieldInfo

      set result $::COMP_MSG_ERR_OK
      set msgFieldInfos [dict get $compMsgDispatcher compMsgTypesAndNames msgFieldInfos]
      if {$idx >= [dict get $msgFieldInfos numMsgFields]} {
        return $::COMP_MSG_ERR_BAD_MSG_FIELD_INFO_IDX
      }
      set fieldInfos [dict get $msgFieldInfos fieldInfos]
      set entry [lindex $fieldInfos $idx]
      dict set fieldInfo fieldFlags [dict get $entry fieldFlags]
      dict set fieldInfo fieldTypeId [dict get $entry fieldTypeId]
      dict set fieldInfo fieldLgth  [dict get $entry fieldLgth]
      dict set fieldInfo fieldOffset [dict get $entry fieldOffset]
      dict set fieldInfo keyValueDesc [dict get $entry keyValueDesc]
      return $result
    }

    # ================================= setMsgFieldInfo ====================================

    proc setMsgFieldInfo {compMsgDispatcherVar idx fieldInfoVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $fieldInfoVar fieldInfo

#puts stderr "setMsgFieldInfo: idx: $idx!"
      set result $::COMP_MSG_ERR_OK
      set msgFieldInfos [dict get $compMsgDispatcher compMsgTypesAndNames msgFieldInfos]
      if {$idx >= [dict get $msgFieldInfos numMsgFields]} {
puts stderr "idx: $idx numMsgFields: [dict get $msgFieldInfos numMsgFields]!"
        return [checkErrOK BAD_MSG_FIELD_INFO_IDX]
      }
      set fieldInfos [dict get $msgFieldInfos fieldInfos]
      set entry [lindex $fieldInfos $idx]
      dict set entry fieldFlags [dict get $fieldInfo fieldFlags]
      dict set entry fieldTypeId [dict get $fieldInfo fieldTypeId]
      dict set entry fieldLgth [dict get $fieldInfo fieldLgth]
      dict set entry fieldOffset [dict get $fieldInfo fieldOffset]
      dict set entry keyValueDesc [dict get $fieldInfo keyValueDesc]
      if {[dict get $fieldInfo keyValueDesc] ne [list]} {
        dict set entry keyValueDesc keyId [dict get $fieldInfo keyValueDesc keyId]
        dict set entry keyValueDesc keyAccess [dict get $fieldInfo keyValueDesc keyAccess]
        dict set entry keyValueDesc keyType [dict get $fieldInfo keyValueDesc keyType]
        dict set entry keyValueDesc keyLgth [dict get $fieldInfo keyValueDesc keyLgth]
        dict set entry keyValueDesc keyNumValues [dict get $fieldInfo keyValueDesc keyNumValues]
      }
      set result [::compMsg compMsgTypesAndNames getFieldNameStrFromId compMsgDispatcher $idx fieldName]
      checkErrOK $result
#puts stderr [format "setMsgFieldInfo: %s idx: %d fieldFlags: %s fieldType: %s fieldLgth: %d" $fieldName $idx [dict get $entry fieldFlags] [dict get $entry fieldTypeId] [dict get $entry fieldLgth]]
      set fieldInfos [lreplace $fieldInfos $idx $idx $entry]
      dict set msgFieldInfos fieldInfos $fieldInfos
      dict set compMsgDispatcher compMsgTypesAndNames msgFieldInfos $msgFieldInfos
      return $result
    }

    # ================================= compMsgTypesAndNamesInit ===============================

    proc compMsgTypesAndNamesInit {comMsgDispatcherVar} {
      upvar $comMsgDispatcherVar compMsgDispatcher
      variable specialFieldNames2Ids

      set compMsgTypesAndNames [dict create]
      set msgFieldInfos [dict create]
      dict set msgFieldInfos numMsgFields 0
      dict set msgFieldInfos fieldInfos [list]
      dict set compMsgTypesAndNames msgFieldInfos $msgFieldInfos
      dict set compMsgTypesAndNames numSpecFieldIds [expr {[llength [dict keys $specialFieldNames2Ids]] + 1}]
puts stderr "numSpecFieldIds: [dict get $compMsgTypesAndNames numSpecFieldIds]!"
      dict set compMsgDispatcher compMsgTypesAndNames $compMsgTypesAndNames
      addMsgFieldInfos compMsgDispatcher [expr {[dict get $compMsgTypesAndNames numSpecFieldIds] + 1}]
      return $::COMP_MSG_ERR_OK
    }

  } ; # namespace compMsgTypesAndNames
} ; # namespace compMsg
