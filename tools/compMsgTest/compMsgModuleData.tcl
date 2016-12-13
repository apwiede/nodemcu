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

namespace eval compMsg {
  namespace ensemble create

    namespace export compMsgModuleData

  namespace eval compMsgModuleData {
    namespace ensemble create
      
    namespace export compMsgModuleDataInit setModuleValues getModuleValue setModuleValue
    namespace export getReserve4

    variable compMsgModuleData

    # ================================= getModuleTableFieldValue ====================================
    
    proc getModuleTableFieldValue {compMsgDispatcherVar actionMode} {
      upvar $compMsgDispatcherVar compMsgDispatcher
      
    #ets_printf{"getModuleTableFieldValue: row: %d col: %d actionMode: %d\n", self->buildMsgInfos.tableRow, self->buildMsgInfos tableCol, actionMode}
      switch $actionMode {
        MODULE_INFO_AP_LIST_CALL_BACK {
          set result [::compMsg compMsgWebsocket getScanInfoTableFieldValue compMsgDispatcher $actionMode]
          checkErrOK $result
          return $::COMP_MSG_ERR_OK
        }
        default {
          checkErrOK $::COMP_DISP_ERR_ACTION_NAME_NOT_FOUND
        }
      }
      checkErrOK $::COMP_DISP_ERR_ACTION_NAME_NOT_FOUND
    }
    
    # ================================= getMACAddr ====================================
    
    proc getMACAddr {compMsgDispatcherVar valueVar} {
      variable compMsgModuleData
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $valueVar value
    
      set value [dict get $compMsgModuleData MACAddr]
      dict set compMsgDispatcher msgValPart fieldKeyValueStr [dict get $compMsgModuleData MACAddr]
      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= getIPAddr ====================================
    
    proc getIPAddr {compMsgDispatcherVar valueVar} {
      variable compMsgModuleData
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $valueVar value
    
      set value [dict get $compMsgModuleData IPAddr]
      dict set compMsgDispatcher msgValPart fieldKeyValueStr [dict get $compMsgModuleData IPAddr]
      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= getFirmwareVersion ====================================
    
    proc getFirmwareVersion {compMsgDispatcherVar valueVar} {
      variable compMsgModuleData
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $valueVar value
    
      set value [dict get $compMsgModuleData FirmwareVersion]
      dict set compMsgDispatcher msgValPart fieldKeyValueStr [dict get $compMsgModuleData FirmwareVersion]
      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= getSerieNumber ====================================
    
    proc getSerieNumber {compMsgDispatcherVar valueVar} {
      variable compMsgModuleData
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $valueVar value
    
      set value [dict get $compMsgModuleData SerieNumber]
      dict set compMsgDispatcher msgValPart fieldKeyValueStr [dict get $compMsgModuleData SerieNumber]
      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= getRSSI ====================================
    
    proc getRSSI {compMsgDispatcherVar valueVar} {
      variable compMsgModuleData
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $valueVar value
    
      set value [dict get $compMsgModuleData RSSI]
      dict lappend compMsgDispatcher msgValPart fieldFlags COMP_DISP_DESC_VALUE_IS_NUMBER
      dict set compMsgDispatcher msgValPart fieldValue [dict get $compMsgModuleData RSSI]
      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= getRSSIMax ====================================
    
    proc getRSSIMax {compMsgDispatcherVar valueVar} {
      variable compMsgModuleData
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $valueVar value
    
      set value [dict get $compMsgModuleData RSSIMax]
      dict lappend compMsgDispatcher msgValPart fieldFlags COMP_DISP_DESC_VALUE_IS_NUMBER
      dict set compMsgDispatcher msgValPart fieldValue [dict get $compMsgModuleData RSSIMax]
      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= getConnectionState ====================================
    
    proc getConnectionState {compMsgDispatcherVar valueVar} {
      variable compMsgModuleData
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $valueVar value
    
      set value [dict get $compMsgModuleData ConnectionState]
      dict lappend compMsgDispatcher msgValPart fieldFlags COMP_DISP_DESC_VALUE_IS_NUMBER
      dict set compMsgDispatcher msgValPart fieldValue [dict get $compMsgModuleData ConnectionState]
      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= getConnectedUsers ====================================
    
    proc getConnectedUsers {compMsgDispatcherVar valueVar} {
      variable compMsgModuleData
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $valueVar value
    
      set value [dict get $compMsgModuleData ConnectedUsers]
      dict lappend compMsgDispatcher msgValPart fieldFlags COMP_DISP_DESC_VALUE_IS_NUMBER
      dict set compMsgDispatcher msgValPart fieldValue [dict get $compMsgModuleData ConnectedUsers]
      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= getProgRunningMode ====================================
    
    proc getProgRunningMode {compMsgDispatcherVar valueVar} {
      variable compMsgModuleData
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $valueVar value
    
      set value [dict get $compMsgModuleData ProgRunningMode]
      dict lappend compMsgDispatcher msgValPart fieldFlags COMP_DISP_DESC_VALUE_IS_NUMBER
      dict set compMsgDispatcher msgValPart fieldValue [dict get $compMsgModuleData ProgRunningMode]
      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= getCurrentRunningMode ====================================
    
    proc getCurrentRunningMode {compMsgDispatcherVar valueVar} {
      variable compMsgModuleData
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $valueVar value
    
      set value [dict get $compMsgModuleData CurrentRunningMode]
      dict lappend compMsgDispatcher msgValPart fieldFlags COMP_DISP_DESC_VALUE_IS_NUMBER
      dict set compMsgDispatcher msgValPart fieldValue [dict get $compMsgModuleData CurrentRunningMode]
      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= getIPProtocol ====================================
    
    proc getIPProtocol {compMsgDispatcherVar valueVar} {
      variable compMsgModuleData
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $valueVar value
    
      set value [dict get $compMsgModuleData IPProtocol]
      dict lappend compMsgDispatcher msgValPart fieldFlags COMP_DISP_DESC_VALUE_IS_NUMBER
      dict set compMsgDispatcher msgValPart fieldValue [dict get $compMsgModuleData IPProtocol]
      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= getRegion ====================================
    
    proc getRegion {compMsgDispatcherVar valueVar} {
      variable compMsgModuleData
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $valueVar value
    
      set value [dict get $compMsgModuleData Region]
      dict lappend compMsgDispatcher msgValPart fieldFlags COMP_DISP_DESC_VALUE_IS_NUMBER
      dict set compMsgDispatcher msgValPart fieldValue [dict get $compMsgModuleData Region]
      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= getDeviceSecurity ====================================
    
    proc getDeviceSecurity {compMsgDispatcherVar valueVar} {
      variable compMsgModuleData
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $valueVar value
    
      set value [dict get $compMsgModuleData DeviceSecurity]
      dict lappend compMsgDispatcher msgValPart fieldFlags COMP_DISP_DESC_VALUE_IS_NUMBER
      dict set compMsgDispatcher msgValPart fieldValue [dict get $compMsgModuleData DeviceSecurity]
      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= getErrorMain ====================================
    
    proc getErrorMain {compMsgDispatcherVar valueVar} {
      variable compMsgModuleData
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $valueVar value
    
      set value [dict get $compMsgModuleData ErrorMain]
      dict lappend compMsgDispatcher msgValPart fieldFlags COMP_DISP_DESC_VALUE_IS_NUMBER
      dict set compMsgDispatcher msgValPart fieldValue [dict get $compMsgModuleData ErrorMain]
      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= getErrorSub ====================================
    
    proc getErrorSub {compMsgDispatcherVar valueVar} {
      variable compMsgModuleData
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $valueVar value
    
      set value [dict get $compMsgModuleData ErrorSub]
      dict lappend compMsgDispatcher msgValPart fieldFlags COMP_DISP_DESC_VALUE_IS_NUMBER
      dict set compMsgDispatcher msgValPart fieldValue [dict get $compMsgModuleData ErrorSub]
      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= getDateAndTime ====================================
    
    proc getDateAndTime {compMsgDispatcherVar valueVar} {
      variable compMsgModuleData
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $valueVar value
    
      set value [dict get $compMsgModuleData DateAndTime]
      dict set compMsgDispatcher msgValPart fieldKeyValueStr [dict get $compMsgModuleData DateAndTime]
      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= getSSIDs ====================================
    
    proc getSSIDs {compMsgDispatcherVar valueVar} {
      variable compMsgModuleData
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $valueVar value
    
      set value [dict get $compMsgModuleData SSIDs]
      dict lappend compMsgDispatcher msgValPart fieldFlags COMP_DISP_DESC_VALUE_IS_NUMBER
      dict set compMsgDispatcher msgValPart fieldValue [dict get $compMsgModuleData SSIDs]
      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= getPingState ====================================
    
    proc getPingState {compMsgDispatcherVar valueVar} {
      variable compMsgModuleData
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $valueVar value
    
      set value [dict get $compMsgModuleData PingState]
      dict lappend compMsgDispatcher msgValPart fieldFlags COMP_DISP_DESC_VALUE_IS_NUMBER
      dict set compMsgDispatcher msgValPart fieldValue [dict get $compMsgModuleData PingState]
      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= getReserve1 ====================================
    
    proc getReserve1 {compMsgDispatcherVar valueVar} {
      variable compMsgModuleData
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $valueVar value
    
      set value [dict get $compMsgModuleData Reserve1]
      dict lappend compMsgDispatcher msgValPart fieldFlags COMP_DISP_DESC_VALUE_IS_NUMBER
      dict set compMsgDispatcher msgValPart fieldValue [dict get $compMsgModuleData Reserve1]
      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= getReserve2 ====================================
    
    proc getReserve2 {compMsgDispatcherVar valueVar} {
      variable compMsgModuleData
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $valueVar value
    
      set value [dict get $compMsgModuleData Reserve2]
      dict set compMsgDispatcher msgValPart fieldValue [dict get $compMsgModuleData Reserve2]
      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= getReserve3 ====================================
    
    proc getReserve3 {compMsgDispatcherVar valueVar} {
      variable compMsgModuleData
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $valueVar value
    
      set value [dict get $compMsgModuleData Reserve3]
      dict set compMsgDispatcher msgValPart fieldValue [dict get $compMsgModuleData Reserve3]
      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= getReserve4 ====================================
    
    proc getReserve4 {compMsgDispatcherVar valueVar} {
      variable compMsgModuleData
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $valueVar value
    
      set value [dict get $compMsgModuleData Reserve4]
      dict set compMsgDispatcher msgValPart fieldValue [dict get $compMsgModuleData Reserve4]
      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= getReserve5 ====================================
    
    proc getReserve5 {compMsgDispatcherVar valueVar} {
      variable compMsgModuleData
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $valueVar value
    
      set value [dict get $compMsgModuleData Reserve5]
      dict set compMsgDispatcher msgValPart fieldValue [dict get $compMsgModuleData Reserve5]
      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= getReserve6 ====================================
    
    proc getReserve6 {compMsgDispatcherVar valueVar} {
      variable compMsgModuleData
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $valueVar value
    
      set value [dict get $compMsgModuleData Reserve6]
      dict set compMsgDispatcher msgValPart fieldValue [dict get $compMsgModuleData Reserve6]
      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= getReserve7 ====================================
    
    proc getReserve7 {compMsgDispatcherVar valueVar} {
      variable compMsgModuleData
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $valueVar value
    
      set value [dict get $compMsgModuleData Reserve7]
      dict set compMsgDispatcher msgValPart fieldValue [dict get $compMsgModuleData Reserve7]
      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= getReserve8 ====================================
    
    proc getReserve8 {compMsgDispatcherVar valueVar} {
      variable compMsgModuleData
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $valueVar value
    
      set value [dict get $compMsgModuleData Reserve8]
      dict set compMsgDispatcher msgValPart fieldValue [dict get $compMsgModuleData Reserve8]
      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= getGUID ====================================
    
    proc getGUID {compMsgDispatcherVar valueVar} {
      variable compMsgModuleData
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $valueVar value
    
      set value [dict get $compMsgModuleData GUID]
      dict set compMsgDispatcher msgValPart fieldValue [dict get $compMsgModuleData GUID]
      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= getSrcId ====================================
    
    proc getSrcId {compMsgDispatcherVar valueVar} {
      variable compMsgModuleData
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $valueVar value
    
      set value [dict get $compMsgModuleData srcId]
      dict lappend compMsgDispatcher msgValPart fieldFlags COMP_DISP_DESC_VALUE_IS_NUMBER
      dict set compMsgDispatcher msgValPart fieldValue [dict get $compMsgModuleData srcId]
      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= getPasswdC ====================================
    
    proc getPasswdC {compMsgDispatcherVar valueVar} {
      variable compMsgModuleData
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $valueVar value
    
      set value [dict get $compMsgModuleData passwdC]
      dict set compMsgDispatcher msgValPart fieldValue [dict get $compMsgModuleData passwdC]
      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= getOperatingMode ====================================
    
    proc getOperatingMode {compMsgDispatcherVar valueVar} {
      variable compMsgModuleData
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $valueVar value
    
      dict lappend compMsgDispatcher msgValPart fieldFlags COMP_DISP_DESC_VALUE_IS_NUMBER
      switch [dict get $compMsgModuleData operatingMode] {
       MODULE_OPERATING_MODE_CLIENT {
        set value \x10
       }
       MODULE_OPERATING_MODE_AP {
         set value \x20
       }
       MODULE_OPERATING_MODE_LIGHT_SLEEP_WAKEUP {
         set value \x12
       }
       MODULE_OPERATING_MODE_LIGHT_SLEEP {
         set value x13
       }
       MODULE_OPERATING_MODE_WPS {
         set value \x14
       }
       MODULE_OPERATING_MODE_MODULE_TEST {
         set value \xD0
       }
       MODULE_OPERATING_MODE_CLEAR_PASSWDC {
         set value \xE0
       }
       default {
         checkErrOK $::COMP_DISP_ERR_BAD_MODULE_VALUE_WHICH
       }
      }
      dict set compMsgDispatcher msgValPart fieldValue $value
      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= setOperatingMode ====================================
    
    proc setOperatingMode {compMsgDispatcherVar} {
      variable compMsgModuleData
      upvar $compMsgDispatcherVar compMsgDispatcher
    
      set result [::compMsg compMsgModuleData setModuleValue compMsgDispatcher MODULE_INFO_operatingMode MODULE_OPERATING_MODE_AP]
      checkErrOK $result
      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= updateModuleValues ====================================
    
    proc updateModuleValues {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher
      variable compMsgModuleData
    
#      dict set compMsgModuleData RSSI [wifi_station_get_rssi]
      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= setModuleValue ====================================
    
    proc setModuleValue {compMsgDispatcherVar fieldNameStr value} {
      upvar $compMsgDispatcherVar compMsgDispatcher
      variable compMsgModuleData
    
      dict set compMsgModuleData $fieldNameStr $value
      return $::COMP_DISP_ERR_OK
    }

    # ================================= setModuleValues ====================================
    
    proc setModuleValues {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher
      variable compMsgModuleData
    
      set compMsgModuleData [dict create]
      dict set compMsgModuleData MACAddr \xAB\xCD\xEF\x12\x34\x56
      dict set compMsgModuleData IPAddr \xD4\xC3\x12\x34
      dict set compMsgModuleData FirmwareVersion \x12\x34\x56\xAB\xCD\xEF
      dict set compMsgModuleData SerieNumber \x02\x13\x2A\x10
#      dict set compMsgModuleData RSSI [wifi_station_get_rssi]
      dict set compMsgModuleData RSSI -67
      dict set compMsgModuleData RSSIMax -70
      dict set compMsgModuleData ConnectionState 0x41
      dict set compMsgModuleData ConnectedUsers 1
      dict set compMsgModuleData ProgRunningMode 0x34
      dict set compMsgModuleData CurrentRunningMode 0x34
      dict set compMsgModuleData IPProtocol 1
      dict set compMsgModuleData Region 1
      dict set compMsgModuleData DeviceSecurity 0
      dict set compMsgModuleData ErrorMain 0
      dict set compMsgModuleData ErrorSub 0
      dict set compMsgModuleData DateAndTime "0000000000"
      dict set compMsgModuleData SSIDs = 2
      dict set compMsgModuleData PingState = 2
      dict set compMsgModuleData Reserve1 "X"
      dict set compMsgModuleData Reserve2 "XY"
      dict set compMsgModuleData Reserve3 "XYZ"
      dict set compMsgModuleData Reserve4 "ABCD"
      dict set compMsgModuleData Reserve5 "ABCDE"
      dict set compMsgModuleData Reserve6 "ABCDEF"
      dict set compMsgModuleData Reserve7 "ABCDEFG"
#      dict set compMsgModuleData Reserve8 "ABCDEFGH"
      dict set compMsgModuleData Reserve8 "\x00\x00\x00\x00\x00\x00\x00r"
#      dict set compMsgModuleData GUID "1234-5678-9012-1"
      dict set compMsgModuleData GUID "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
#      dict set compMsgModuleData srcId 12312
      dict set compMsgModuleData srcId 0
      dict set compMsgModuleData passwdC "apwiede1apwiede2"
      dict set compMsgModuleData operatingMode MODULE_OPERATING_MODE_CLEAR_PASSWDC

      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= compMsgModuleDataInit ====================================
    
    proc compMsgModuleDataInit {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher
    
      set result [::compMsg compMsgModuleData setModuleValues compMsgDispatcher]
      checkErrOK $result
      return $::COMP_DISP_ERR_OK
    }

  } ; # namespace compMsgModuleData
} ; # namespace compMsg
