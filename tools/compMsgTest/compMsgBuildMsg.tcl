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
      
    namespace export setMsgValues buildMsg buildMsgFromHeaderPart

    variable buildMsgInfos

    set buildMsgInfos [dict create]

    # ================================= fixOffsetsForKeyValues ====================================
    
    proc fixOffsetsForKeyValues {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher
    
puts stderr "==fixOffsetsForKeyValues!"
      set compMsgData [dict get $compMsgDispatcher compMsgData]
      set fieldIdx 0
      set msgDescPartIdx 0
      while {$fieldIdx < [dict get $compMsgData numFields]} {
        set fields [dict get $compMsgData fields]
        set fieldInfo [lindex $fields $fieldIdx]
        set msgDescParts [dict get $compMsgData msgDescParts]
        set msgDescPart [lindex $msgDescParts $msgDescPartIdx]
        dict set compMsgDispatcher msgDescPart $msgDescPart
        set msgKeyValueDescPart [list]
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
            checkErrOK $::COMP_DISP_ERR_FIELD_NOT_FOUND
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
        set compMsgData [dict get $compMsgDispatcher compMsgData]
        incr msgDescPartIdx
        incr fieldIdx
      }
      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= setMsgFieldValue ====================================
    
    proc setMsgFieldValue {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher
    
      set compMsgData [dict get $compMsgDispatcher compMsgData]
      set fieldValueStr [dict get $compMsgDispatcher msgValPart fieldValueStr]
      set fieldNameStr [dict get $compMsgDispatcher msgValPart fieldNameStr]
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
        set msgValPart [dict get $compMsgDispatcher msgValPart]
        if {[lsearch [dict get $msgValPart fieldFlags] COMP_DISP_DESC_VALUE_IS_NUMBER] >= 0} {
          set value [dict get $msgValPart fieldValue]
        } else {
          set value [dict get $msgValPart fieldValueStr]
        }
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
#puts stderr "setMsgFieldValue: done\n"
      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= setMsgValues ====================================
    
    proc setMsgValues {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher
    
#puts stderr "setMsgValues"
      set compMsgData [dict get $compMsgDispatcher compMsgData]
      set msgDescPartIdx 0
      set msgValPartIdx 0
      set msgDescParts [dict get $compMsgDispatcher compMsgData msgDescParts]
      set msgKeyValueDescParts [dict get $compMsgDispatcher msgKeyValueDescParts]
      set msgValParts [dict get $compMsgDispatcher compMsgData msgValParts]
      set msgValPart [lindex $msgValParts $msgValPartIdx]
      while {($msgDescPartIdx < [dict get $compMsgData numFields]) && ($msgValPartIdx <= [dict get $compMsgDispatcher compMsgMsgDesc numMsgValParts])} {
        set msgDescPart [lindex $msgDescParts $msgDescPartIdx]
        dict set compMsgDispatcher msgDescPart $msgDescPart
        set idx 0
        set found false
        if {[string range [dict get $msgDescPart fieldNameStr] 0 0] eq "#"} {
          while {$idx < [llength $msgKeyValueDescParts]} {
            set msgKeyValueDescPart [lindex $msgKeyValueDescParts $idx]
            if {[dict get $msgKeyValueDescPart keyNameStr] eq [dict get $msgDescPart fieldNameStr]} {
              set found true
              break
            }
            incr idx
          }
        }
        if {$found} {
          dict set compMsgDispatcher msgKeyValueDescPart $msgKeyValueDescPart
        } else {
          dict set compMsgDispatcher msgKeyValueDescPart [list]
        }
        set msgValPart [lindex $msgValParts $msgValPartIdx]
        dict set compMsgDispatcher msgValPart $msgValPart
        set fields [dict get $compMsgData fields]
        set fieldInfo [lindex $fields $msgDescPartIdx]
        set fieldNameId [dict get $fieldInfo fieldNameId]
        set fieldNameStr [dict get $msgDescPart fieldNameStr]
        if {($msgValPart ne [list]) && ([dict get $fieldInfo fieldNameId] eq [dict get $msgValPart fieldNameId])} {
#puts stderr [format "default fieldNameId: %s buildMsgInfo fieldNameId: %s" [dict get $fieldInfo fieldNameId] [dict get $msgValPart fieldNameId]]
          set result [setMsgFieldValue compMsgDispatcher]
          checkErrOK $result
          set msgValParts [dict get $compMsgDispatcher compMsgData msgValParts]
          set msgValPart [dict get $compMsgDispatcher msgValPart]
          set msgValParts [lreplace $msgValParts $msgValPartIdx $msgValPartIdx $msgValPart]
          dict set compMsgDispatcher compMsgData msgValParts $msgValParts
          incr msgValPartIdx
        }
        incr msgDescPartIdx
      }
      set msgCmdKey [dict get $compMsgDispatcher currHdr hdrU16CmdKey]
      set result [::compMsg compMsgData setFieldValue compMsgDispatcher "@cmdKey" $msgCmdKey]
      checkErrOK $result
      set result [::compMsg compMsgData prepareMsg compMsgDispatcher]
      checkErrOK $result
#puts stderr "setMsgValues end"
#::compMsg compMsgData dumpMsg compMsgDispatcher
      return $::COMP_DISP_ERR_OK
    }

    # ================================= buildMsg ====================================
    
    proc buildMsg {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher
    
    #//ets_printf{"buildMsg\n"}
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
      
      set result [fixOffsetsForKeyValues compMsgDispatcher]
      checkErrOK $result
      set result [::compMsg compMsgData initMsg compMsgDispatcher]
      checkErrOK $result
      set result [setMsgValues compMsgDispatcher]
      checkErrOK $result
      set result [::compMsg compMsgData getMsgData compMsgDispatcher msgData msgLgth]
      checkErrOK $result
#::compMsg compMsgData dumpMsg compMsgDispatcher
#::compMsg compMsgData dataView $msgData $msgLgth "msgData"
      if {[dict get $compMsgDispatcher currHdr hdrEncryption] eq "E"} {
        set cryptKey "a1b2c3d4e5f6g7h8"
        set ivlen 16
        set klen 16
    
puts stderr "need to encrypt message!"
        set headerLgth [dict get $compMsgDispatcher compMsgData headerLgth]
        set totalCrcOffset 0
        set totalCrc ""
        set mlen [expr {[dict get $compMsgDispatcher compMsgData totalLgth] - $headerLgth}]
        set hdr [dict get $compMsgDispatcher currHdr]
        if {[lsearch [dict get $hdr hdrFlags] COMP_DISP_TOTAL_CRC] >= 0} {
          if {[lsearch [dict get $hdr fieldSequence] COMP_DISP_U8_TOTAL_CRC] >= 0} {
            set totalCrcOffset 1
            incr mlen -1
            set totalCrc [string range $msgData end end]
          } else {
            set totalCrc [string range $msgData end-1 end]
            incr totalCrcOffset 2
            incr mlen -2
          }
        }
        set endIdx [expr {[dict get $compMsgDispatcher compMsgData totalLgth] - $totalCrcOffset - 1}]
puts stderr "headerLgth!$headerLgth!mlen!$mlen!"
        set toCrypt [string range $msgData [dict get $compMsgDispatcher compMsgData headerLgth] $endIdx]
        set header [string range $msgData 0 [expr {$headerLgth - 1}]]
        set result [::compMsg compMsgDispatcher encryptMsg $toCrypt $mlen $cryptKey $klen $cryptKey $ivlen encryptedMsgData encryptedMsgDataLgth]
        checkErrOK $result
        set msgData "${header}${encryptedMsgData}${totalCrc}"
#puts stderr [format "crypted: len: %d!mlen: %d!msgData lgth! %d" $encryptedMsgDataLgth $mlen [string length $msgData]]
      }
      set compMsgData [dict get $compMsgDispatcher compMsgData]
      if {[lsearch [dict get $compMsgDispatcher currHdr hdrFlags] COMP_DISP_TOTAL_CRC] >= 0} {
        set startOffset [dict get $compMsgData headerLgth]
        # lgth is needed without totalCrc
        set fieldSequence [dict get $compMsgDispatcher currHdr fieldSequence]
        set totalCrcLgth 0
        switch [lindex $fieldSequence end] {
          COMP_DISP_U8_TOTAL_CRC {
            set totalCrcLgth 1
            set totalCrcOffset [expr {[dict get $compMsgData totalLgth] - $totalCrcLgth}]
          }
          COMP_DISP_U16_TOTAL_CRC {
            set totalCrcLgth 2
            set totalCrcOffset [expr {[dict get $compMsgData totalLgth] - $totalCrcLgth}]
          }
        }
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
      # from currHdr we can see the handle type and - if needed - the @dst
      # that is now done in sendMsg!
#set handleType [dict get $compMsgDispatcher currHdr hdrHandleType]
#set toPart [dict get $compMsgDispatcher currHdr hdrToPart]
#puts stderr [format "transferType: %s dst: 0x%04x" $handleType $toPart]
      set result [::compMsg compMsgSendReceive sendMsg compMsgDispatcher $msgData $msgLgth]
#set fd [open "ADRequest.txt" w]
#puts $fd $msgData
#flush $fd
#close $fd
#puts stderr [format "buildMsg sendMsg has been called result: %d" $result]
      checkErrOK $result
      return $result
    }

    # ================================= buildMsgFromHeaderPart ====================================

    proc buildMsgFromHeaderPart {hdr} {
      variable buildMsgInfos

pdict $hdr
      return $::COMP_MSG_ERR_OK
    }

  } ; # namespace compMsgBuildMsg
} ; # namespace compMsg
