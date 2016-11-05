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
      
    namespace export compMsgModuleDataInit setModuleValues

    variable compMsgModuleData

    # ================================= getModuleTableFieldValue ====================================
    
    proc getModuleTableFieldValue {compMsgDispatcherVar actionMode} {
      upvar $compMsgDispatcherVar compMsgDispatcher
      
    #ets_printf{"getModuleTableFieldValue: row: %d col: %d actionMode: %d\n", self->buildMsgInfos.tableRow, self->buildMsgInfos tableCol, actionMode}
      switch %actionMode {
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
    
    # ================================= getModuleValue ====================================
    
    proc getModuleValue {compMsgDispatcherVar which valueTypeId} {
      upvar $compMsgDispatcherVar compMsgDispatcher
      variable compMsgModuleData
    
      switch $which {
        MODULE_INFO_MACAddr {
          dict set compMsgDispatcher msgValPart fieldKeyValueStr [dict get $compMsgModuleData MACAddr]
        }
        MODULE_INFO_IPAddr {
          dict set compMsgDispatcher msgValPart fieldKeyValueStr [dict get $compMsgModuleData IPAddr]
        }
        MODULE_INFO_FirmwareVersion {
          dict set compMsgDispatcher msgValPart fieldKeyValueStr [dict get $compMsgModuleData FirmwareVersion]
        }
        MODULE_INFO_SerieNumber {
          dict set compMsgDispatcher msgValPart fieldKeyValueStr [dict get $compMsgModuleData SerieNumber]
        }
        MODULE_INFO_RSSI {
          dict lappend compMsgDispatcher msgValPart fieldFlags COMP_DISP_DESC_VALUE_IS_NUMBER
          dict set compMsgDispatcher msgValPart fieldValue [dict get $compMsgModuleData RSSI]
        }
        MODULE_INFO_ModuleConnection {
          dict lappend compMsgDispatcher msgValPart fieldFlags COMP_DISP_DESC_VALUE_IS_NUMBER
          dict set compMsgDispatcher msgValPart fieldValue = [dict get $compMsgModuleData ModuleConnection]
        }
        MODULE_INFO_DeviceMode {
          dict lappend compMsgDispatcher msgValPart fieldFlags COMP_DISP_DESC_VALUE_IS_NUMBER
          dict set compMsgDispatcher msgValPart fieldValue [dict get $compMsgModuleData DeviceMode]
        }
        MODULE_INFO_DeviceSecurity {
          dict lappend compMsgDispatcher msgValPart fieldFlags COMP_DISP_DESC_VALUE_IS_NUMBER
          dict set compMsgDispatcher msgValPart fieldValue [dict get $compMsgModuleData DeviceSecurity]
        }
        MODULE_INFO_ErrorMain {
          dict lappend compMsgDispatcher msgValPart fieldFlags COMP_DISP_DESC_VALUE_IS_NUMBER
          dict set compMsgDispatcher msgValPart fieldValue [dict get $compMsgModuleData ErrorMain]
        }
        MODULE_INFO_ErrorSub {
          dict lappend compMsgDispatcher msgValPart fieldFlags COMP_DISP_DESC_VALUE_IS_NUMBER
          dict set compMsgDispatcher msgValPart fieldValue [dict get $compMsgModuleData ErrorSub]
        }
        MODULE_INFO_DateAndTime {
          dict set compMsgDispatcher msgValPart fieldKeyValueStr [dict get $compMsgModuleData DateAndTime]
        }
        MODULE_INFO_SSIDs {
          dict lappend compMsgDispatcher msgValPart fieldFlags COMP_DISP_DESC_VALUE_IS_NUMBER
          dict set compMsgDispatcher msgValPart fieldValue [dict get $compMsgModuleData SSIDs]
        }
        MODULE_INFO_Reserve1 {
          dict lappend compMsgDispatcher msgValPart fieldFlags COMP_DISP_DESC_VALUE_IS_NUMBER
          dict set compMsgDispatcher msgValPart fieldValue [dict get $compMsgModuleData Reserve1]
        }
        MODULE_INFO_Reserve2 {
          dict set compMsgDispatcher msgValPart fieldKeyValueStr [dict get $compMsgModuleData Reserve2]
        }
        MODULE_INFO_Reserve3 {
          dict set compMsgDispatcher msgValPart fieldKeyValueStr [dict get $compMsgModuleData Reserve3]
        }
        MODULE_INFO_GUID {
          dict set compMsgDispatcher msgValPart fieldKeyValueStr [dict get $compMsgModuleData GUID]
        }
        MODULE_INFO_srcId {
          dict lappend compMsgDispatcher msgValPart fieldFlags COMP_DISP_DESC_VALUE_IS_NUMBER
          dict set compMsgDispatcher msgValPart fieldValue [dict get $compMsgModuleData srcId]
        }
      default {
        checkErrOK $::COMP_DISP_ERR_BAD_MODULE_VALUE_WHICH
        }
      }
      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= updateModuleValues ====================================
    
    proc updateModuleValues {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher
      variable compMsgModuleData
    
#      dict set compMsgModuleData RSSI [wifi_station_get_rssi]
      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= setModuleValues ====================================
    
    proc setModuleValues {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher
      variable compMsgModuleData
    
#FIXME!!!
if {0} {
      dict set compMsgModuleData MACAddr[0] = 0xAB
      dict set compMsgModuleData MACAddr[1] = 0xCD
      dict set compMsgModuleData MACAddr[2] = 0xEF
      dict set compMsgModuleData MACAddr[3] = 0x12
      dict set compMsgModuleData MACAddr[4] = 0x34
      dict set compMsgModuleData MACAddr[5] = 0x56
      dict set compMsgModuleData IPAddr[0] = 0xD4
      dict set compMsgModuleData IPAddr[1] = 0xC3
      dict set compMsgModuleData IPAddr[2] = 0x12
      dict set compMsgModuleData IPAddr[3] = 0x34
      dict set compMsgModuleData FirmwareVersion[0] = 0x12
      dict set compMsgModuleData FirmwareVersion[1] = 0x34
      dict set compMsgModuleData FirmwareVersion[2] = 0x56
      dict set compMsgModuleData FirmwareVersion[3] = 0xAB
      dict set compMsgModuleData FirmwareVersion[4] = 0xCD
      dict set compMsgModuleData FirmwareVersion[5] = 0xEF
      dict set compMsgModuleData SerieNumber[0] = 0x02
      dict set compMsgModuleData SerieNumber[1] = 0x13
      dict set compMsgModuleData SerieNumber[2] = 0x2A
      dict set compMsgModuleData SerieNumber[3] = 0x10
}
#      dict set compMsgModuleData RSSI [wifi_station_get_rssi]
      dict set compMsgModuleData RSSI -67
      dict set compMsgModuleData ModuleConnection 0x41
      dict set compMsgModuleData DeviceMode 0x34
      dict set compMsgModuleData DeviceSecurity 0
      dict set compMsgModuleData ErrorMain 0
      dict set compMsgModuleData ErrorSub 0
      dict set compMsgModuleData DateAndTime "0000000000"
      dict set compMsgModuleData SSIDs = 2
      dict set compMsgModuleData Reserve1 "X"
      dict set compMsgModuleData Reserve2 "XY"
      dict set compMsgModuleData Reserve3 "XYZ"
      dict set compMsgModuleData GUID "1234-5678-9012-1"
      dict set compMsgModuleData srcId 12312

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
