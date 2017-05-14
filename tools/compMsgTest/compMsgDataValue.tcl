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

namespace eval ::compMsg {
  namespace ensemble create

  namespace export compMsgDataValue

  namespace eval compMsgDataValue {
    namespace ensemble create
      
    namespace export dataValueStr2ValueId dataValueId2ValueStr addDataValue
    namespace export setDataValue getDataValue compMsgDataValueInit
    namespace export addMsgFieldValues setMsgFieldValue getMsgFieldValue

    variable dataValueStr2ValueIds 
    set dataValueStr2ValueIds [dict create]
    dict set dataValueStr2ValueIds "@provisioningSsid"     COMP_MSG_WIFI_VALUE_ID_provisioningSsid
    dict set dataValueStr2ValueIds "@provisioningPort"     COMP_MSG_WIFI_VALUE_ID_provisioningPort
    dict set dataValueStr2ValueIds "@provisioningIPAddr"   COMP_MSG_WIFI_VALUE_ID_provisioningIPAddr
    dict set dataValueStr2ValueIds "@SSDPIPAddr"           COMP_MSG_WIFI_VALUE_ID_SSDPIPAddr
    dict set dataValueStr2ValueIds "@SSDPPort"             COMP_MSG_WIFI_VALUE_ID_SSDPPort
    dict set dataValueStr2ValueIds "@SSDPReceivedCallback" COMP_MSG_WIFI_VALUE_ID_SSDPReceivedCallback
    dict set dataValueStr2ValueIds "@SSDPToSendCallback"   COMP_MSG_WIFI_VALUE_ID_SSDPToSendCallback
    dict set dataValueStr2ValueIds "@clientIPAddr"         COMP_MSG_WIFI_VALUE_ID_clientIPAddr
    dict set dataValueStr2ValueIds "@clientPort"           COMP_MSG_WIFI_VALUE_ID_clientPort
    dict set dataValueStr2ValueIds "@clientSequenceNum"    COMP_MSG_WIFI_VALUE_ID_clientSequenceNum
    dict set dataValueStr2ValueIds "@clientSsid"           COMP_MSG_WIFI_VALUE_ID_clientSsid
    dict set dataValueStr2ValueIds "@clientPasswd"         COMP_MSG_WIFI_VALUE_ID_clientPasswd
    dict set dataValueStr2ValueIds "@clientStatus"         COMP_MSG_WIFI_VALUE_ID_clientStatus
    dict set dataValueStr2ValueIds "@binaryCallback"       COMP_MSG_WIFI_VALUE_ID_binaryCallback
    dict set dataValueStr2ValueIds "@textCallback"         COMP_MSG_WIFI_VALUE_ID_textCallback
    dict set dataValueStr2ValueIds "@NetReceivedCallback"  COMP_MSG_WIFI_VALUE_ID_NetReceivedCallback
    dict set dataValueStr2ValueIds "@NetToSendCallback"    COMP_MSG_WIFI_VALUE_ID_NetToSendCallback
    dict set dataValueStr2ValueIds "@NetSecureConnect"     COMP_MSG_WIFI_VALUE_ID_NetSecureConnect
    dict set dataValueStr2ValueIds "@cloudPort"            COMP_MSG_WIFI_VALUE_ID_cloudPort
    dict set dataValueStr2ValueIds "@operatingMode"        COMP_MSG_WIFI_VALUE_ID_operatingMode

    dict set dataValueStr2ValueIds "Reserve1"              COMP_MSG_MODULE_VALUE_ID_Reserve1
    dict set dataValueStr2ValueIds "Reserve2"              COMP_MSG_MODULE_VALUE_ID_Reserve2
    dict set dataValueStr2ValueIds "Reserve3"              COMP_MSG_MODULE_VALUE_ID_Reserve3
    dict set dataValueStr2ValueIds "Reserve4"              COMP_MSG_MODULE_VALUE_ID_Reserve4
    dict set dataValueStr2ValueIds "Reserve5"              COMP_MSG_MODULE_VALUE_ID_Reserve5
    dict set dataValueStr2ValueIds "Reserve6"              COMP_MSG_MODULE_VALUE_ID_Reserve6
    dict set dataValueStr2ValueIds "Reserve7"              COMP_MSG_MODULE_VALUE_ID_Reserve7
    dict set dataValueStr2ValueIds "Reserve8"              COMP_MSG_MODULE_VALUE_ID_Reserve8
    dict set dataValueStr2ValueIds "cryptKey"              COMP_MSG_MODULE_VALUE_ID_cryptKey
    dict set dataValueStr2ValueIds "cryptIvKey"            COMP_MSG_MODULE_VALUE_ID_cryptIvKey

    # ================================= dataValueStr2ValueId ====================================

    proc dataValueStr2ValueId {compMsgDispatcherVar valueStr valueIdVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $valueIdVar valueId
      variable dataValueStr2ValueIds

      if {[dict exists $dataValueStr2ValueIds $valueStr]} {
        set valueId [dict get $dataValueStr2ValueIds $valueStr]
        return [checkErrOK OK]
      }
      checkErrOK FIELD_NOT_FOUND
    }

    # ================================= dataValueId2ValueStr ====================================

    proc  dataValueId2ValueStr {compMsgDispatcherVar valueId valueStrVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $valueStrVar valueStr
      variable dataValueStr2ValueIds

      foreach key [dict keys $dataValueStr2ValueIds] {
        if {[dict get $dataValueStr2ValueIds $key] eq $valueId} {
          set valueStr $key
          return [checkErrOK OK]
        }
      }
      checkErrOK FIELD_NOT_FOUND
    }

    # ================================= addDataValue ====================================

    proc addDataValue {compMsgDispatcherVar dataValue dataValueIdxVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $dataValueIdxVar dataValueIdx

      set result OK
      set compMsgDataValue [dict get $compMsgDispatcher compMsgDataValue]
#puts stderr "num: [dict get $compMsgDataValue numDataValues] max: [dict get $compMsgDataValue maxDataValues]!"
      if {[dict get $compMsgDataValue numDataValues] >= [dict get $compMsgDataValue maxDataValues]} {
        if {[dict get $compMsgDataValue maxDataValues] == 0} {
          dict set compMsgDataValue maxDataValues 5
          set lst [list]
          set valIdx 0
          while {$valIdx < 5} {
            lappend lst [list]
            incr valIdx
          }
          dict set compMsgDataValue dataValues $lst
        } else {
          dict incr compMsgDataValue maxDataValues 2
          set valIdx 0
          while {$valIdx < 2} {
            dict lappend compMsgDataValue dataValues [list]
            incr valIdx
          }
        }
      }
      set dataValues [dict get $compMsgDataValue dataValues]
      set dataValueIdx [dict get $compMsgDataValue numDataValues]
      set myDataValue [lindex $dataValues $dataValueIdx]

      dict set myDataValue cmdKey [dict get $dataValue cmdKey]
      dict set myDataValue fieldValueId [dict get $dataValue fieldValueId]
      dict set myDataValue fieldNameId [dict get $dataValue fieldNameId]
      dict set myDataValue fieldValueCallback [dict get $dataValue fieldValueCallback]
      dict set myDataValue flags [dict get $dataValue flags]
      dict set myDataValue value [dict get $dataValue value]
      if {[dict get $myDataValue fieldValueCallback] ne ""} {
        dict lappend myDataValue flags FIELD_HAS_CALLBACK
      }
      set dataValues [lreplace $dataValues $dataValueIdx $dataValueIdx $dataValue]
      set dataValueIdx [dict get $compMsgDataValue numDataValues]
      dict incr compMsgDataValue numDataValues
      dict set compMsgDataValue dataValues $dataValues
      dict set compMsgDispatcher compMsgDataValue $compMsgDataValue
      return [checkErrOK OK]
    }

    # ================================= setDataValue ====================================

    proc setDataValue {compMsgDispatcherVar cmdKey valueId fieldValueCallback numericValue stringValue} {
      upvar $compMsgDispatcherVar compMsgDispatcher

      result = OK
      set compMsgDataValue [dict get $compMsgDispatcher compMsgDataValue]
      set idx 0
      while {$idx < [dict get $compMsgDataValue numDataValues} {
        set dataValues [dict get $compMsgDataValue dataValues]
        set dataValue [lindex $dataValues $idx]
        if {([dict get dataValue valueId] eq $valueId) && ([dict get dataValue cmdKey] eq $cmdKey)} {
          dict set dataValue flags [list]
          if {$stringValue eq ""} {
            dict lappend flags COMP_MSG_FIELD_IS_NUMERIC
            dict set dataValue value $numericValue
          }
          if {$stringValue ne ""} {
            dict lappend flags COMP_MSG_FIELD_IS_STRING
            dict set dataValue value $numericValue
          }
          if {$fieldValueCallback ne""} {
            dict lappend dataValue flags COMP_MSG_FIELD_HAS_CALLBACK
          }
          dict set dataValue fieldValueCallback $fieldValueCallback
          set dataValues [lreplace $dataValues $idx $idx $dataValue]
          dict set compMsgDataValue dataValues $dataValues
          dict set compMsgDispatcher compMsgDataValue $compMsgDataValue
          return [checkErrOK OK]
        }
        incr idx
      }
      checkErrOK DATA_VALUE_FIELD_NOT_FOUND
    }

    # ================================= getDataValue ====================================

#*
# * \brief get value from data area
# * \param self The dispatcher struct
# * \param cmdKey Cmd key of the value
# * \param fieldId Value id of the value
# * \param fieldValueCallback The field value callback, if there exists one
# * \param numericValue The value if it is a numeric one
# * \param stringValue The value if it is a character string
# * \return Error code or ErrorOK
# *

    proc getDataValue {compMsgDispatcherVar cmdKey valueId flagsVar fieldValueCallbackVar numericValueVar stringValueVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $flagsVar flags
      upvar $fieldValueCallbackVar fieldValueCallback
      upvar $numericValueVar numericValue
      upvar $stringValueVar stringValue

      set result OK
      set compMsgDataValue = self->compMsgDataValue;
      Value ""
      set flags ""
      set fieldValueCallback ""
      set idx 0
      while {$idx < [dict get $compMsgDataValue numDataValues]} {
        set dataValues [dict get $compMsgDataValue dataValues]
        set dataValue [lindex $dataValues $idx]
        if {([dict get $dataValue valueId] eq $valueId) && ([dict get $dataValue cmdKey] eq $cmdKey)} {
          set result [dataValueId2ValueStr compMsgDispatcher $valueId value]
          checkErrOK $result
          set flags [dict get $dataValue flags]
          set stringValue ""
          set numericValue 0
          if {[lsearch $flags COMP_MSG_FIELD_IS_NUMERIC] >= 0} {
            set numericValue [dict get $dataValue value]
          }
          if {[lsearch $flags COMP_MSG_FIELD_IS_STRING] >= 0} {
            set stringValue [dict get $dataValue value]
          }
          puts stderr [format "getDataValue: %s %s" $numericValue $stringValue]
          set fieldValueCallback [dict get $dataValue fieldValueCallback]
          set dataValues [lreplace $dataValues $idx $idx $dataValue]
          dict set compMsgDataValue dataValues $dataValues
          dict set compMsgDispatcher compMsgDataValue $compMsgDataValue
          return [checkErrOK OK]
        }
        incr idx
      }
      puts stderr [format "getDataValue: DATA_VALUE_FIELD_NOT_FOUND: %s %s" $cmdKey $valueId]
      checkErrOK DATA_VALUE_FIELD_NOT_FOUND
    }

    # ================================= addMsgFieldValues ====================================

    proc addMsgFieldValues {compMsgDispatcherVar numEntries} {
      upvar $compMsgDispatcherVar compMsgDispatcher

      set result $::COMP_MSG_ERR_OK
      set msgFieldValues [dict get $compMsgDispatcher compMsgDataValue msgFieldValues]
      if {[dict get $msgFieldValues numMsgFields] == 0} {       
        set fieldValues [list]
      } else {
        set fieldValues [dict get $msgFieldValues fieldValues]
      }
      set idx [dict get $msgFieldValues numMsgFields]
      while {$idx < [expr {[dict get $msgFieldValues numMsgFields] + $numEntries}]} {
        lappend fieldValues [list]
        incr idx
      }
      dict set msgFieldValues fieldValues $fieldValues
      dict incr msgFieldValues numMsgFields $numEntries
      dict set compMsgDispatcher compMsgDataValue msgFieldValues $msgFieldValues
      return $result
    }

    # ================================= setMsgFieldValue ====================================

    proc setMsgFieldValue {compMsgDispatcherVar idx fieldValue} {
      upvar $compMsgDispatcherVar compMsgDispatcher

      set result $::COMP_MSG_ERR_OK
      set msgFieldValues [dict get $compMsgDispatcher compMsgDataValue msgFieldValues]
      if {$idx >= [dict get $msgFieldValues numMsgFields]} {
        checkErrOK BAD_MSG_FIELD_INFO_IDX
      }
      set fieldValues [dict get $msgFieldValues fieldValues]
      set entry [lindex $fieldValues $idx]
      dict set entry flags [ðict get $fieldValue flags]
#      COMP_MSG_DBG(self, "E", 0, "setMsgFieldValues: idx: %d fieldValueFlags: 0x%08x", idx, entry->flags);
      return $result
    }

    #  ================================= getMsgFieldValue ====================================

    proc getMsgFieldValue {compMsgDispatcherVar idx fieldValueVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $fieldValueVar fieldValue

      set result $::COMP_MSG_ERR_OK
      set msgFieldValues [dict get $compMsgDispatcher compMsgDataValue msgFieldValues]
      if {$idx >= [dict get $msgFieldValues numMsgFields]} {
        checkErrOK BAD_MSG_FIELD_INFO_IDX
      }
      set fieldValues [dict get $msgFieldValues fieldValues]
      set entry [lindex $fieldValues $idx]
      if {$entry eq [list]} {
        dict set entry flags [list]
        dict set entry dataValue [list]
      }
      dict set fieldValue flags [dict get $entry flags]
      dict set fieldValue dataValue [dict get $entry dataValue]
      return $result
    }

    # ================================= compMsgDataValueInit ====================================

    proc compMsgDataValueInit {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher

      set compMsgDataValue [dict create]
      dict set compMsgDataValue numDataValues 0
      dict set compMsgDataValue maxDataValues 0
      dict set compMsgDataValue dataValues [list]

      set msgFieldValues [dict create]
      dict set msgFieldValues numMsgFields 0
      dict set msgFieldValues fieldValues [list]
      dict set compMsgDataValue msgFieldValues $msgFieldValues

      dict set compMsgDispatcher compMsgDataValue $compMsgDataValue
      return [checkErrOK OK]
    }

  } ; # namespace compMsgDataValue
} ; # namespace compMsg
