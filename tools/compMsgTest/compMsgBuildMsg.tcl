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

# buildMsgInfos dict
#  numEntries
#  type
#  parts
#  numRows
#  u8CmdKey
#  u16CmdKey
#  partsFlags
#  fieldNameStr
#  fieldValueStr
#  fieldNameId
#  fieldTypeId
#  tableRow
#  tableCol
#  value
#  buf[100]

namespace eval compMsg {
  namespace ensemble create

    namespace export compMsgBuildMsg

  namespace eval compMsgBuildMsg {
    namespace ensemble create
      
    namespace export setMsgValues buildMsg buildMsgFromHeaderPart createMsgFromHeaderPart
    namespace export setMsgFieldValue

    variable buildMsgInfos

    set buildMsgInfos [dict create]

    # ================================= createMsgFromHeaderPart ====================================
    
    proc createMsgFromHeaderPart {compMsgDispatcherVar msgDescription} {
      upvar $compMsgDispatcherVar compMsgDispatcher
    
puts stderr "===createMsgFromHeaderPart![dict keys $msgDescription]!"
#      set result [::compMsg compMsgMsgDesc getMsgPartsFromHeaderPart compMsgDispatcher $msgDescription]
#      checkErrOK $result
      set compMsgData [dict create]
      dict set compMsgData flags [list]
      dict set compMsgData msgDescription $msgDescription
      dict set compMsgDispatcher compMsgData $compMsgData
    
      # runAction calls at the end buildMsg
      set prepareValuesCb [list]
      if {[dict exists $compMsgDispatcher compMsgMsgDesc prepareValuesCbName]} {
        set prepareValuesCb [dict get $compMsgDispatcher compMsgMsgDesc prepareValuesCbName]
#puts stderr "prepareValuesCb: $prepareValuesCb!"
      }
      if {$prepareValuesCb ne [list]} {
        $prepareValuesCb compMsgDispatcher
        # runAction starts a call with a callback and returns here before the callback has been running!!
        # when coming here we are finished and the callback will do the work later on!
#puts stderr "runAction done![dict keys $compMsgDispatcher]!"
        return $result
      } else {
        # buildMsg calls sendMsg at the end
puts stderr "call buildMsg: [dict keys $compMsgDispatcher]!"
        set result [::compMsg compMsgBuildMsg buildMsg compMsgDispatcher $msgDescription]
      }
      return [checkErrOK OK]
    }

    # ================================= fixOffsetsForKeyValues ====================================
    
    proc fixOffsetsForKeyValues {compMsgDispatcherVar msgDescription} {
      upvar $compMsgDispatcherVar compMsgDispatcher
    
puts stderr "==fixOffsetsForKeyValues![dict keys $compMsgDispatcher]!"
      set compMsgData [dict get $compMsgDispatcher compMsgData]
      set msgDescription [dict get $compMsgData msgDescription]
      set numFields [dict get $msgDescription numFields]
      set fieldIdx 0
      set fieldSequence [dict get $msgDescription fieldSequence]
      set compMsgTypesAndNames [dict get $compMsgDispatcher compMsgTypesAndNames]
      set msgFieldInfos [dict get $compMsgTypesAndNames msgFieldInfos]
      set fieldDescInfos [dict get $msgFieldInfos fieldDescInfos]
      while {$fieldIdx < $numFields} {
        set fieldId [lindex $fieldSequence $fieldIdx]
        set fieldDescInfo [lindex $fieldDescInfos $fieldId]
pdict $fieldDescInfo
if {0} {
        set fieldNameStr [dict get $msgDescPart fieldNameStr]
        set msgKeyValueDescPartIdx -1
        if {[string range $fieldNameStr 0 0] eq "#"} {
          # get the corresponding msgKeyValueDescPart
          set found false
          set keyValueIdx 0
          while {$keyValueIdx < [dict get $compMsgDispatcher numMsgKeyValueDescParts]} {
            set msgKeyValueDescPart [lindex [dict get $compMsgDispatcher msgKeyValueDescParts] $keyValueIdx]
            set keyNameStr [dict get $msgKeyValueDescPart keyNameStr]
            if {$keyNameStr eq $fieldNameStr} {
              set found true
              set msgKeyValueDescPartIdx $keyValueIdx
              break
            }
            incr keyValueIdx
          }
          if {!$found} {
            set msgKeyValueDescPart [list]
          }
        }
        dict set compMsgDispatcher msgKeyValueDescPart $msgKeyValueDescPart
        set callback [dict get $msgDescPart fieldSizeCallback]
        if {$callback ne [list]} {
          # the key name must have the prefix: "#key_"!
          if {[string range $fieldNameStr 0 0] ne "#"} {
            checkErrOK $::COMP_MSG_ERR_FIELD_NOT_FOUND
          }
          # that is the callback to eventually get the size of the key/value field
          set callback [string range $callback 1 end] ; # strip off '@' character
          set result [::$callback compMsgDispatcher]
          checkErrOK $result
          set compMsgData [dict get $compMsgDispatcher compMsgData]
          set fields [dict get $compMsgData fields]
          set fieldInfo [lindex $fields $fieldIdx]
          set msgDescPart [dict get $compMsgDispatcher msgDescPart]
          if {$msgKeyValueDescPartIdx >= 0} {
            set msgKeyValueDescPart [dict get $compMsgDispatcher msgKeyValueDescPart]
            dict set msgDescPart fieldSize [dict get $msgKeyValueDescPart keyLgth]
          }
          if {$msgKeyValueDescPart ne [list]} {
            dict set fieldInfo fieldKey [dict get $msgKeyValueDescPart keyId]
          } else {
            dict set fieldInfo fieldKey [dict get $msgDescPart fieldKey]
          }
puts stderr "fieldLgth1: [dict get $fieldInfo fieldLgth]!fieldSize: [dict get $msgDescPart fieldSize]!"
          dict incr msgDescPart fieldSize [expr {2 * 2 + 1}] ; # for key, type and lgth in front of value!!
          dict set fieldInfo fieldLgth [dict get $msgDescPart fieldSize]
puts stderr "fieldLgth2: [dict get $fieldInfo fieldLgth]!"
          set fields [lreplace $fields $fieldIdx $fieldIdx $fieldInfo]
          set msgDescParts [dict get $compMsgData msgDescParts]
          set msgDescParts [lreplace $msgDescParts $msgDescPartIdx $msgDescPartIdx $msgDescPart]
          dict set compMsgData fields $fields
          dict set compMsgData msgDescParts $msgDescParts
          dict set compMsgDispatcher compMsgData $compMsgData
        } else {
          if {$msgKeyValueDescPart ne [list]} {
            set fields [dict get $compMsgData fields]
            set fieldInfo [lindex $fields $fieldIdx]
            set msgDescPart [dict get $compMsgDispatcher msgDescPart]
            dict set msgDescPart fieldSize [dict get $fieldInfo fieldLgth]
            dict set fieldInfo fieldKey [dict get $msgKeyValueDescPart keyId]
            dict set msgDescPart fieldSize [dict get $fieldInfo fieldLgth]
            dict incr msgDescPart fieldSize [expr {2 +1 + 2}] ; # for key, type and lgth in front of value!!
            dict set fieldInfo fieldLgth [dict get $msgDescPart fieldSize]
            set fields [lreplace $fields $fieldIdx $fieldIdx $fieldInfo]
            set msgDescParts [dict get $compMsgData msgDescParts]
            set msgDescParts [lreplace $msgDescParts $msgDescPartIdx $msgDescPartIdx $msgDescPart]
            dict set compMsgData fields $fields
            dict set compMsgData msgDescParts $msgDescParts
            dict set compMsgDispatcher compMsgData $compMsgData
          }
        }
}
        set compMsgData [dict get $compMsgDispatcher compMsgData]
        incr fieldIdx
      }
      return $::COMP_MSG_ERR_OK
    }
    
    # ================================= setMsgFieldValue ====================================
    
    proc setMsgFieldValue {compMsgDispatcherVar fieldIdx msgDescription} {
      upvar $compMsgDispatcherVar compMsgDispatcher
    
      set compMsgData [dict get $compMsgDispatcher compMsgData]
      set fieldSequence [dict get $msgDescription fieldSequence]
      set fieldNameIntId [lindex $fieldSequence $fieldIdx]
      set msgFieldInfos [dict get $compMsgDispatcher compMsgTypesAndNames msgFieldInfos]
      set fieldDescInfos [dict get $msgFieldInfos fieldDescInfos]
      set fieldDescInfo [lindex $fieldDescInfos $fieldNameIntId]
      set result [::compMsg compMsgTypesAndNames getFieldNameIdFromInt compMsgDispatcher $fieldNameIntId fieldNameId]
      checkErrOK $result
set result [::compMsg compMsgTypesAndNames getFieldNameStrFromId ::compMsgDispatcher $fieldNameIntId fieldName]
checkErrOK $result
puts stderr "setMsgFieldValue: fieldName: $fieldName!id: $fieldNameIntId!$fieldNameId!\n  $fieldDescInfo!"
# FIXME TEMPORARY!
set cmdKey P1
      switch $fieldNameId {
        COMP_MSG_SPEC_FIELD_TOTAL_LGTH -
        COMP_MSG_SPEC_FIELD_CMD_LGTH -
        COMP_MSG_SPEC_FIELD_FILLER -
        COMP_MSG_SPEC_FIELD_HDR_FILLER -
        COMP_MSG_SPEC_FIELD_CRC -
        COMP_MSG_SPEC_FIELD_TOTAL_CRC -
        COMP_MSG_SPEC_FIELD_RANDOM_NUM -
        COMP_MSG_SPEC_FIELD_SEQUENCE_NUM {
          # these cannot be set manually!
puts stderr "no setting  field"
          return $::COMP_MSG_ERR_OK
        }
      }
      if {[lsearch [dict get $fieldDescInfo fieldFlags] COMP_MSG_FIELD_KEY_VALUE] >= 0} {
puts stderr "keyValue field"
        # special handling for buildning key value fields!
      } else {
        if {[lsearch [dict get $fieldDescInfo fieldFlags] COMP_MSG_FIELD_HEADER_UNIQUE] >= 0} {
puts stderr "unique header field!"
          # we get the appropriate value from the msgDescription headerValues
          set result [::compMsg dataView getDataViewData savedData savedLgth]
          checkErrOK $result
          set result [::compMsg dataView setDataViewData [dict get $msgDescription headerFieldValues] [dict get $compMsgDispatcher compMsgMsgDesc msgHeaderInfo headerLgth]]
          checkErrOK $result
          set result [::compMsg compMsgDataView getFieldValue $fieldDescInfo value 0]
puts stderr "get: value: $value!$fieldDescInfo!"
          checkErrOK $result
          set result [::compMsg dataView setDataViewData $savedData $savedLgth]
          checkErrOK $result
          set result [::compMsg compMsgDataView setFieldValue $fieldDescInfo $value 0]
          checkErrOK $result
          dict lappend  fieldDescInfo fieldFlags COMP_MSG_FIELD_IS_SET
        } else {
          if {[lsearch [dict get $fieldDescInfo fieldFlags] COMP_MSG_FIELD_HEADER] >= 0} {
puts stderr "header field"
          } else {
puts stderr "normal field"
            # we get the appropriate value in checking if there is a fieldValueCallback or from the fieldValueInfos
            # for that cmdKey
            set compMsgDataValue [dict get $compMsgDispatcher compMsgDataValue]
            set dataValues [dict get $compMsgDataValue dataValues]
            set idx 0
            set found false
            while {$idx < [dict get $compMsgDataValue numDataValues]} {
               set dataValue [lindex $dataValues $idx]
               if {[dict get $dataValue cmdKey] eq $cmdKey} {
                 set found true
puts stderr "dataValue: $cmdKey!$dataValue!"
                 break
               }
               incr idx
            }
            if {!$found} {
              checkErrOK DATA_VALUE_CMD_KEY_NOT_FOUND
            }
            set result [::compMsg compMsgDataView setFieldValue $fieldDescInfo [dict get $dataValue value] 0]
            checkErrOK $result
            dict lappend  fieldDescInfo fieldFlags COMP_MSG_FIELD_IS_SET
          }
        }
        set fieldDescInfos [lreplace $fieldDescInfos $fieldNameIntId $fieldNameIntId $fieldDescInfo]
      }
      dict set msgFieldInfos fieldDescInfos $fieldDescInfos
      dict set compMsgDispatcher compMsgTypesAndNames msgFieldInfos $msgFieldInfos

if {0} {
      if {[string range $fieldValueStr 0 3] eq "@get"} {
        # call the callback function for the field!!
        set callback [dict get $compMsgDispatcher msgValPart fieldValueCallback]
        if {$callback ne [list]} {
          if {[info procs ::compMsg::compMsgModuleData::$callback] ne [list]} {
            set result [::compMsg::compMsgModuleData::$callback compMsgDispatcher value]
            dict set compMsgDispatcher msgValPart fieldValue $value
          } else {
            set result [$callback compMsgDispatcher value]
            dict set compMsgDispatcher msgValPart fieldValue $value
          }
          checkErrOK $result
        }
        set value [dict get $compMsgDispatcher msgValPart fieldValue]
#puts stderr "setMsgFieldValue1: $fieldNameStr!$value!"
        set result [::compMsg compMsgData setFieldValue compMsgDispatcher $fieldNameStr $value]
set value2 "???"
set result [::compMsg compMsgData getFieldValue compMsgDispatcher $fieldNameStr value2]
#puts stderr "value2: $value2!"
      } else {
#puts stderr "setMsgFieldValue2: $fieldNameStr!$value!"
        switch [dict get $msgValPart fieldNameId] {
          COMP_MSG_SPEC_FIELD_DST {
            set result [::compMsg compMsgData setFieldValue compMsgDispatcher $fieldNameStr $value]
          }
          COMP_MSG_SPEC_FIELD_SRC {
            set result [::compMsg compMsgData setFieldValue compMsgDispatcher $fieldNameStr $value]
          }
          COMP_MSG_SPEC_FIELD_CMD_KEY {
            set numericValue [dict get $compMsgDispatcher currHdr hdrU16CmdKey]
            set result [::compMsg compMsgData setFieldValue compMsgDispatcher $fieldNameStr $value]
          }
          default {
            set result [::compMsg compMsgData setFieldValue compMsgDispatcher $fieldNameStr $value]
          }
        }
        checkErrOK $result
      }
}
#puts stderr "setMsgFieldValue: done\n"
      return $::COMP_MSG_ERR_OK
    }
    
    # ================================= setMsgValues ====================================
    
    proc setMsgValues {compMsgDispatcherVar msgDescription} {
      upvar $compMsgDispatcherVar compMsgDispatcher
    
puts stderr "setMsgValues"
      set compMsgData [dict get $compMsgDispatcher compMsgData]
      set numEntries [dict get $msgDescription numFields]
      set fieldSequence [dict get $msgDescription fieldSequence]
      set compMsgTypesAndNames [dict get $compMsgDispatcher compMsgTypesAndNames]
      set msgFieldInfos [dict get $compMsgTypesAndNames msgFieldInfos]
      set fieldDescInfos [dict get $msgFieldInfos fieldDescInfos]
      set compMsgMsgDesc [dict get $compMsgDispatcher compMsgMsgDesc]
      set msgFieldGroupInfos [dict get $compMsgMsgDesc msgFieldGroupInfos]
      set cmdKeyVal [dict get $msgDescription cmdKey]
      set val1 [format %c [expr {$cmdKeyVal >> 8}]]
      set val2 [format %c [expr {$cmdKeyVal & 0xFF}]]
      set msgFieldGroupInfo [dict get $msgFieldGroupInfos COMP_MSG_VAL_FIELD_GROUP "${val1}${val2}"]
      set numFields [dict get $msgDescription numFields]
      set fieldIdx 0
puts stderr "numFields: $numFields!"
      while {$fieldIdx < $numFields} {
        set fieldId [lindex $fieldSequence $fieldIdx]
        set fieldDescInfo [lindex $fieldDescInfos $fieldId]
set result [::compMsg compMsgTypesAndNames getFieldNameStrFromId ::compMsgDispatcher $fieldId fieldName]
checkErrOK $result
puts stderr "fieldName: $fieldName!"
        set result [::compMsg compMsgBuildMsg setMsgFieldValue compMsgDispatcher $fieldIdx $msgDescription]
        checkErrOK $result

        incr fieldIdx
      }
if {0} {
      set msgCmdKey [dict get $compMsgDispatcher currHdr hdrU16CmdKey]
      set result [::compMsg compMsgData setFieldValue compMsgDispatcher "@cmdKey" $msgCmdKey]
      checkErrOK $result
}
      set result [::compMsg compMsgData prepareMsg compMsgDispatcher $msgDescription]
      checkErrOK $result
puts stderr "setMsgValues end"
::compMsg compMsgData dumpMsg compMsgDispatcher
      return $::COMP_MSG_ERR_OK
    }

    # ================================= buildMsg ====================================
    
    proc buildMsg {compMsgDispatcherVar msgDescription} {
      upvar $compMsgDispatcherVar compMsgDispatcher
    
puts stderr "buildMsg: msgDescription: $msgDescription!"
      # at this point an eventual callback for getting the values 
      # has been already done using runAction in createMsgFromHeaderPart
      # so now we can fix the offsets if needed for key value list entries
      # we can do that in looking for a special key entry @numKeyValues in msgDescParts
      # could be also done in looking in compMsgData fields
      # to get the desired keys we look in msgValParts for fieldNames starting with '#'
      # the fieldValueStr ther is a callBackFunction for building the key value entries
      # the entry @numKeyValues in msgValParts tells us how many different keys follow
      # a key value entry is built like so:
      # uint16_t key
      # uint16_t length o value
      # uint8_t* the bytes of the value
      # this could if needed also be an array of uint16_t etc. depending on the key
      # the receiver must know how the value is built depending on the key!!
      
      set result [fixOffsetsForKeyValues compMsgDispatcher $msgDescription]
      checkErrOK $result
      set result [::compMsg compMsgData initMsg compMsgDispatcher $msgDescription]
      checkErrOK $result
      set result [setMsgValues compMsgDispatcher $msgDescription]
      checkErrOK $result
      set result [::compMsg compMsgData getMsgData compMsgDispatcher msgData msgLgth]
      checkErrOK $result
#::compMsg compMsgData dumpMsg compMsgDispatcher
#::compMsg compMsgData dataView $msgData $msgLgth "msgData"
      if {[dict get $msgDescription encrypted] eq "E"} {
        set cryptKey "a1b2c3d4e5f6g7h8"
        set ivlen 16
        set klen 16
    
puts stderr "need to encrypt message!"
        set headerLgth [dict get $compMsgDispatcher compMsgMsgDesc msgHeaderInfo headerLgth]
        set totalCrcOffset 0
        set totalCrc ""
        set mlen [expr {[dict get $compMsgDispatcher compMsgData totalLgth] - $headerLgth}]
        if {[lsearch [dict get $msgDescription fieldFlags] COMP_MSG_HAS_TOTAL_CRC] >= 0} {
          set result [::compMsg compMsgDataView getTotalCrcLgth compMsgDispatcher $msgDescription totalCrcLgth]
          checkErrOK $result
          set mlen [expr {$mlen - $totalCrcLgth}]
          if {$totalCrcLgth == 1} {
            set totalCrc [string range $msgData end end]
          } else {
            set totalCrc [string range $msgData end-1 end]
          }
        }
        set endIdx [expr {[dict get $compMsgDispatcher compMsgData totalLgth] - $totalCrcLgth - 1}]
puts stderr "headerLgth!$headerLgth!mlen!$mlen!"
        set toCrypt [string range $msgData [dict get $compMsgDispatcher compMsgData headerLgth] $endIdx]
        set header [string range $msgData 0 [expr {$headerLgth - 1}]]
        set result [::compMsg compMsgUtil encryptMsg $toCrypt $mlen $cryptKey $klen $cryptKey $ivlen encryptedMsgData encryptedMsgDataLgth]
        checkErrOK $result
        set msgData "${header}${encryptedMsgData}${totalCrc}"
#puts stderr [format "crypted: len: %d!mlen: %d!msgData lgth! %d" $encryptedMsgDataLgth $mlen [string length $msgData]]
      }
      set compMsgData [dict get $compMsgDispatcher compMsgData]
      if {[lsearch [dict get $msgDescription fieldFlags] COMP_MSG_HAS_TOTAL_CRC] >= 0} {
        set startOffset [dict get $compMsgData headerLgth]
        set result [::compMsg compMsgDataView getTotalCrcLgth compMsgDispatcher $msgDescription totalCrcLgth]
        checkErrOK $result
        # lgth is needed without totalCrc
        set totalCrcOffset [expr {[dict get $compMsgData totalLgth] - $totalCrcLgth}]
      } else {
        set totalCrcOffset [dict get $compMsgData totalLgth]
      }
      set fieldOffset [expr {[dict get $compMsgDispatcher compMsgData totalLgth] - $totalCrcOffset}]

      dict set fieldInfo fieldLgth $totalCrcLgth
      dict set fieldInfo fieldOffset $totalCrcOffset
      set ::compMsg::dataView::data $msgData
      set ::compMsg::dataView::lgth [dict get $compMsgDispatcher compMsgData totalLgth]
      set result [::compMsg compMsgDataView setTotalCrc $fieldInfo]
      checkErrOK $result
      set msgData $::compMsg::dataView::data
        
#::compMsg dataView dumpBinary $msgData [string length $msgData] "msgData"
      # here we need to decide where and how to send the message!!
      # from msgDescription we can see the handle type and - if needed - the @dst
      # that is now done in sendMsg!
#puts stderr [format "transferType: %s dst: 0x%04x" $handleType $toPart]
      set result [::compMsg compMsgSendReceive sendMsg compMsgDispatcher $msgData $msgLgth $msgDescription]
#puts stderr [format "buildMsg sendMsg has been called result: %d" $result]
      checkErrOK $result
      return $result
    }

    # ================================= buildMsgFromHeaderPart ====================================

    proc buildMsgFromHeaderPart {hdr} {
      variable buildMsgInfos

pdict $hdr
      return [checkErrOK OK]
    }

  } ; # namespace compMsgBuildMsg
} ; # namespace compMsg
