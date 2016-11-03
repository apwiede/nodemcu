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
    
      set compMsgData [dict get $compMsgDispatcher compMsgData]
      set fieldIdx 0
      set msgDescPartIdx 0
      set fields [dict get $compMsgData fields]
      set msgDescParts [dict get $compMsgDispatcher msgDescParts]
      while {$fieldIdx < [dict get $compMsgData numFields]} {
        set fieldInfo [lindex $fields $fieldIdx]
        set msgDescPart [lindex $msgDescParts $msgDescPartIdx]
        dict set compmMsgDispatcher msgDescPart $msgDescPart
        if {[dict get $msgDescPart getFieldSizeCallback] ne [list]} {
          # the key name must have the prefix: "#key_"!
          if {[string range [dict get $msgDescPart fieldNameStr] 0 0] ne "#"} {
            return $::COMP_DISP_ERR_FIELD_NOT_FOUND
          }
puts stderr "call: [dict get $msgDescPart getFieldSizeCallback]!"
          set result [[dict get $msgDescPart getFieldSizeCallback] compMsgDispatcher]
          checkErrOK $result
          dict set fieldInfo fieldKey [dict get $msgDescPart fieldKey]
          dict incr msgDescPart fieldSize 2 * 2 + 1 ; # for key, type and lgth in front of value!!
          dict set fieldInfo fieldLgth [dict get $msgDescPart fieldSize]
          set fields [lreplace $fields $fieldIdx $fieldIdx $fieldInfo]
          set msgParts [lreplace $msgParts $msgPartIdx $msgPartIdx $msgPart]
        }
        incr msgDescPartIdx
        incr fieldIdx
      }
      dict set compMsgData fields $fields
      dict set compMsgDispatcher compMsgMsgDesc msgDescParts $msgDescParts
      dict set compMsgDispatcher compMsgData $compMsgData
      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= setMsgFieldValue ====================================
    
    proc setMsgFieldValue {compMsgDispatcherVar numTableRowsVar numTableRowFieldsVar type} {
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $numTableRowsVar numTableRows
      upvar $numTableRowFieldsVar numTableRowFields
    
      set compMsgData [dict get $compMsgDispatcher compMsgData]
      set fieldValueStr [dict get $compMsgDispatcher msgValPart fieldValueStr]
      set fieldNameStr [dict get $compMsgDispatcher msgValPart fieldNameStr]
      if {[string range $fieldValueStr 0 0] eq "@"} {
        # call the callback function for the field!!
        if {$numTableRows > 0} {
          set currTableRow 0
          set currTableCol 0
          while {$currTableRow < $numTableRows} {
if {0} {
            self->buildMsgInfos.tableRow = currTableRow
            self->buildMsgInfos.tableCol = currTableCol
            result = self->fillMsgValue{self, self->msgValPart->fieldValueStr, type, self->msgDescPart->fieldTypeId}
            checkErrOK{result}
            result = compMsgData->setTableFieldValue{compMsgData, self->msgValPart->fieldNameStr, currTableRow, self->buildMsgInfos.numericValue, self->buildMsgInfos.stringValue}
}
            incr currTableRow
          }
          set currTableRow 0
          incr currTableCol
          if {$currTableCol > $numTableRowFields} {
            # table rows done
            set numTableRows 0
            set numTableRowFields 0
            set currTableRow 0
            set currTableCol 0
          }
        } else {
          if {[dict get $compMsgDispatcher msgValPart getFieldValueCallback] ne [list]} {
            set result [dict get $compMsgDispatcher msgValPart getFieldValueCallback] compMsgDispatcher
            checkErrOK $result
          }
          if {[lsearch [dict get $compMsgDispatcher msgValPart fieldFlags] COMP_DISP_DESC_VALUE_IS_NUMBER] >= 0} {
            set value [dict get $compMsgDispatcher msgValPart fieldValue]
          } else {
            set value [dict get $compMsgDispatcher msgValPart fieldKeyValueStr]
          }
          set result [:.compMsg compMsgData setFieldValue compMsgDispatcher $fieldNameStr value]
    #      result = self->fillMsgValue{self, self->buildMsgInfos.fieldValueStr, type, self->buildMsgInfos.fieldTypeId}
    #      checkErrOK{result}
    #      result = compMsgData->setFieldValue{compMsgData, self->buildMsgInfos.fieldNameStr, self->buildMsgInfos.numericValue, self->buildMsgInfos.stringValue}
    #      currTableRow++
        }
      } else {
        set msgValPart [dict get $compMsgDispatcher msgValPart]
        if {[lsearch [dict get $msgValPart fieldFlags] COMP_DISP_DESC_VALUE_IS_NUMBER] >= 0} {
          set value [dict get $msgValPart fieldValue]
        } else {
          set value [dict get $msgValPart fieldValueStr]
        }
        switch [dict get $msgValPart fieldNameId] {
          COMP_MSG_SPEC_FIELD_DST {
            set result [::compMsg compMsgData setFieldValue compMsgDispatcher $fieldNameStr $value]
          }
          COMP_MSG_SPEC_FIELD_SRC {
            set result [::compMsg compMsgData setFieldValue compMsgDispatcher $fieldNameStr $value]
          }
          COMP_MSG_SPEC_FIELD_CMD_KEY {
            set numericValue [dict get $compMsgDispatcher currHdr hdrU16CmdKey]
            set result [:.compMsg compMsgData setFieldValue compMsgDispatcher $fieldNameStr $value]
          }
          default {
            set result [::compMsg compMsgData setFieldValue compMsgDispatcher $fieldNameStr $value]
          }
        }
        checkErrOK $result
      }
    #ets_printf{"setMsgFieldValue: done\n"}
      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= setMsgValues ====================================
    
    proc setMsgValues {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher
    
    #ets_printf{"setMsgValues\n"}
      set compMsgData [dict get $compMsgDispatcher compMsgData]
#      set handle [dict get $compMsgDispatcher msgHandle]
      set type "A"
      # loop over MSG Fields, to check if we eventually have table rows!!
      set msgDescPartIdx 0
      set msgValPartIdx 0
      set msgDescParts [dict get $compMsgDispatcher msgDescParts]
      set msgValParts [dict get $compMsgDispatcher msgValParts]
      set msgValPart [lindex $msgValParts $msgValPartIdx]
      set tableFieldIdx 0
      set numTableRows 0
      set numTableRowFields 0
      while {($msgDescPartIdx < [dict get $compMsgData numFields]) && ($msgValPartIdx <= [dict get $compMsgDispatcher compMsgMsgDesc numMsgValParts])} {
        set msgDescPart [lindex $msgDescParts $msgDescPartIdx]
        dict set compMsgDispatcher msgDescPart $msgDescPart
        set msgValPart [lindex $msgValParts $msgValPartIdx]
        dict set compMsgDispatcher msgValPart $msgValPart
    #ets_printf{"setMsgValuesFromLines2: fieldIdx: %d tableFieldIdx: %d entryIdx: %d numFields:%d \n", fieldIdx, tableFieldIdx, entryIdx, compMsgData->numFields}
    #ets_printf{"fieldIdx: %d entryIdx: %d numtableRows: %d\n", fieldIdx, entryIdx, numTableRows}
        if {$numTableRows > 0} {
          set tableFields [dict get $compMsgData tableFields]
          set fieldInfo [lindex $tableFields $tableFieldIdx]
          incr tableFieldIdx
        } else {
          set fields [dict get $compMsgData fields]
          set fieldInfo [lindex $fields $msgDescPartIdx]
          incr msgDescPartIdx
        }
    #ets_printf{"fieldNameId: %d numtabrows: %d\n", fieldInfo->fieldNameId, numTableRows}
        switch [dict get $fieldInfo fieldNameId] {
          COMP_MSG_SPEC_FIELD_TABLE_ROW_FIELDS {
            set numTableRows [dict get $compMsgData numTableRows]
            set numTableRowFields [dict get $compMsgData numTableRowFields]
          }
          default {
    #ets_printf{"default fieldNameId: %d buildMsgInfo fieldNameId: %d\n", fieldInfo->fieldNameId, self->buildMsgInfos.fieldNameId}
            if {($msgValPart ne [list]) && ([dict get $fieldInfo fieldNameId] eq [dict get $msgValPart fieldNameId])} {
              set result [setMsgFieldValue compMsgDispatcher numTableRows numTableRowFields $type]
              checkErrOK $result
              incr msgValPartIdx
            }
          }
        }
      }
      set msgCmdKey [dict get $compMsgDispatcher currHdr hdrU16CmdKey]
      set result [::compMsg compMsgData setFieldValue compMsgDispatcher "@cmdKey" $msgCmdKey]
      checkErrOK $result
      dict set compMsgDispatcher compMsgData $compMsgData
    
      set result [::compMsg compMsgData prepareMsg compMsgDispatcher]
      checkErrOK $result
::compMsg compMsgData dumpMsg compMsgDispatcher
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
      # could be also done in looking in compMsgData->fields
      # to get the desired keys we look in msgValParts for fieldNames starting with '#'
      # the fieldValueStr ther is a callBackFunction for building the key value entries
      # the entry @numKeyValues in msgValParts tells us how many different keys follow
      # a key value entry is built like so:
      # uint16_t key
      # uint16_t length o value
      # uint8_t* the bytes of the value
      # this could if needed also be an array of uint16_t etc. depending on the key
      # the receiver must know how the value is built depending on the key!!
      
puts stderr "buildMsg"
      set result [fixOffsetsForKeyValues compMsgDispatcher]
      checkErrOK $result
      set result [::compMsg compMsgData initMsg compMsgDispatcher numTableRows numTableRowFields]
      checkErrOK $result
      set result [setMsgValues compMsgDispatcher]
      checkErrOK $result
      set result [::compMsg compMsgData getMsgData compMsgDispatcher msgData msgLgth]
      checkErrOK $result
::compMsg compMsgData dumpBinary $msgData $msgLgth "msgData"
if {0} {
      if {self->currHdr->hdrEncryption == 'E'} {
        cryptKey = "a1b2c3d4e5f6g7h8"
        ivlen = 16
        klen = 16
    
    ets_printf{"need to encrypt message!\n"}
        headerLgth = self->compMsgData->headerLgth
        mlen = self->compMsgData->totalLgth - headerLgth
    ets_printf{"msglen!%d!mlen: %d, headerLgth!%d\n", self->compMsgData->totalLgth, mlen, self->compMsgData->headerLgth}
        toCryptPtr = msgData + self->compMsgData->headerLgth
        result = self->encryptMsg{toCryptPtr, mlen, cryptKey, klen, cryptKey, ivlen, &encryptedMsgData, &encryptedMsgDataLgth}
        checkErrOK{result}
        c_memcpy{toCryptPtr, encryptedMsgData, encryptedMsgDataLgth}
    ets_printf{"crypted: len: %d!mlen: %d!\n", encryptedMsgDataLgth, mlen}
      }
}
        
      # here we need to decide where and how to send the message!!
      # from currHdr we can see the handle type and - if needed - the @dst
set handleType [dict get $compMsgDispatcher currHdr hdrHandleType]
set encryption [dict get $compMsgDispatcher currHdr hdrEncryption]
set toPart [dict get $compMsgDispatcher currHdr hdrToPart]
puts stderr [format "transferType: %s encryption: %s dst: 0x%04x" $handleType $encryption $toPart]
      set result [::compMsg compMsgSendReceive sendMsg compMsgDispatcher $msgData $msgLgth]
puts stderr [format "buildMsg sendMsg has been called, we finish here temporarely! result: %d" $result]
      checkErrOK $result
    #  result = self->resetMsgInfo{self, self->buildMsgInfos.parts}
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
