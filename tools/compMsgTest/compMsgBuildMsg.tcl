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

    # ================================= setMsgValues ====================================

    proc setMsgValues {} {
      variable buildMsgInfos

      set fileName [format "CompVal%s.txt" [dict get $buildMsgInfos u16CmdKey]]
      set fd [open $fileName, "r"]
      gets $fd buffer
      set flds [split $line ","]
      foreach {dummy numEntries} $flds break
#        return $::COMP_DISP_ERR_BAD_FILE_CONTENTS
      set result [::compMsg cmdMsgDispatcher setMsgValuesFromLines $fd $numEntries $msgHandle $type]
      if {$result != $::COMP_MSG_ERR_OK} {
        return $result
      }
      close $fd
      return $::COMP_MSG_ERR_OK
    }

    # ================================= fixOffsetsForKeyValues ====================================
    
    proc fixOffsetsForKeyValues {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher
    
      compMsgData = self->compMsgData
      fieldIdx = 0
      msgDescPartIdx = 0
      while {fieldIdx < compMsgData->numFields} {
        fieldInfo = &compMsgData->fields[fieldIdx]
        msgDescPart = &self->compMsgMsgDesc->msgDescParts[msgDescPartIdx]
        self->msgDescPart = msgDescPart
        if {msgDescPart->getFieldSizeCallback != NULL} {
          # the key name must have the prefix: "#key_"!
          if {msgDescPart->fieldNameStr[0] != '#'} {
            return COMP_DISP_ERR_FIELD_NOT_FOUND
          }
          result = msgDescPart->getFieldSizeCallback{self}
          checkErrOK{result}
          fieldInfo->fieldKey = msgDescPart->fieldKey
          msgDescPart->fieldSize += 2 * sizeof{uint16_t} + sizeof{uint8_t} # for key, type and lgth in front of value!!
          fieldInfo->fieldLgth = msgDescPart->fieldSize
        }
        msgDescPartIdx++
        fieldIdx++
      }
      return COMP_DISP_ERR_OK
    }
    
    # ================================= setMsgFieldValue ====================================
    
    proc setMsgFieldValue {compMsgDispatcherVar numTableRowsVar numTableRowFieldsVar type} {
      upvar $compMsgDispatcherVar compMsgDispatcher
    
    #ets_printf{"setMsgFieldValue: %s %s\n", self->msgValPart->fieldNameStr, self->msgValPart->fieldValueStr}
      compMsgData = self->compMsgData
      if {self->msgValPart->fieldValueStr[0] == '@'} {
        # call the callback function for the field!!
        if {*numTableRows > 0} {
          currTableRow = 0
          currTableCol = 0
          while {currTableRow < *numTableRows} {
            self->buildMsgInfos.tableRow = currTableRow
            self->buildMsgInfos.tableCol = currTableCol
            result = self->fillMsgValue{self, self->msgValPart->fieldValueStr, type, self->msgDescPart->fieldTypeId}
            checkErrOK{result}
            result = compMsgData->setTableFieldValue{compMsgData, self->msgValPart->fieldNameStr, currTableRow, self->buildMsgInfos.numericValue, self->buildMsgInfos.stringValue}
            currTableRow++
          }
          currTableRow = 0
          currTableCol++
          if {currTableCol > *numTableRowFields} {
            # table rows done
            *numTableRows = 0
            *numTableRowFields = 0
            currTableRow = 0
            currTableCol = 0
          }
        } else {
          if {self->msgValPart->getFieldValueCallback != NULL} {
            result = self->msgValPart->getFieldValueCallback{self}
            checkErrOK{result}
          }
          fieldNameStr = self->msgValPart->fieldNameStr
          if {self->msgValPart->fieldFlags & COMP_DISP_DESC_VALUE_IS_NUMBER} {
            stringValue = NULL
            numericValue = self->msgValPart->fieldValue
          } else {
            stringValue = self->msgValPart->fieldKeyValueStr
            numericValue = 0
          }
          result = self->compMsgData->setFieldValue{compMsgData, fieldNameStr, numericValue, stringValue}
    #      result = self->fillMsgValue{self, self->buildMsgInfos.fieldValueStr, type, self->buildMsgInfos.fieldTypeId}
    #      checkErrOK{result}
    #      result = compMsgData->setFieldValue{compMsgData, self->buildMsgInfos.fieldNameStr, self->buildMsgInfos.numericValue, self->buildMsgInfos.stringValue}
    #      currTableRow++
        }
      } else {
        fieldNameStr = self->msgValPart->fieldNameStr
        if {self->msgValPart->fieldFlags & COMP_DISP_DESC_VALUE_IS_NUMBER} {
          stringValue = NULL
          numericValue = self->msgValPart->fieldValue
        } else {
          stringValue = self->msgValPart->fieldValueStr
          numericValue = 0
        }
        switch {self->msgValPart->fieldNameId} {
          case COMP_MSG_SPEC_FIELD_DST:
            result = compMsgData->setFieldValue{compMsgData, fieldNameStr, numericValue, stringValue}
            break
          case COMP_MSG_SPEC_FIELD_SRC:
            result = compMsgData->setFieldValue{compMsgData, fieldNameStr, numericValue, stringValue}
            break
          case COMP_MSG_SPEC_FIELD_CMD_KEY:
            numericValue = self->currHdr->hdrU16CmdKey
            stringValue = NULL
            result = compMsgData->setFieldValue{compMsgData, fieldNameStr, numericValue, stringValue}
            break
          default:
            result = self->compMsgData->setFieldValue{compMsgData, fieldNameStr, numericValue, stringValue}
            break
        }
        checkErrOK{result}
      }
    #ets_printf{"setMsgFieldValue: done\n"}
      return COMP_DISP_ERR_OK
    }
    
    # ================================= setMsgValues ====================================
    
    proc setMsgValues {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher
    
    #ets_printf{"setMsgValues\n"}
      compMsgData = self->compMsgData
      handle = self->msgHandle
      type = 'A'
      dataView = compMsgData->compMsgDataView
      # loop over MSG Fields, to check if we eventually have table rows!!
      msgDescPartIdx = 0
      msgValPartIdx = 0
      msgValPart = &self->compMsgMsgDesc->msgValParts[msgValPartIdx]
      tableFieldIdx = 0
      numTableRows = 0
      numTableRowFields = 0
      compMsgData = self->compMsgData
      while {{msgDescPartIdx < compMsgData->numFields} && {msgValPartIdx <= self->compMsgMsgDesc->numMsgValParts}} {
        msgDescPart = &self->compMsgMsgDesc->msgDescParts[msgDescPartIdx]
        self->msgDescPart = msgDescPart
        msgValPart = &self->compMsgMsgDesc->msgValParts[msgValPartIdx]
        self->msgValPart = msgValPart
    #ets_printf{"setMsgValuesFromLines2: fieldIdx: %d tableFieldIdx: %d entryIdx: %d numFields:%d \n", fieldIdx, tableFieldIdx, entryIdx, compMsgData->numFields}
    #ets_printf{"fieldIdx: %d entryIdx: %d numtableRows: %d\n", fieldIdx, entryIdx, numTableRows}
        if {numTableRows > 0} {
          fieldInfo = &compMsgData->tableFields[tableFieldIdx++]
        } else {
          fieldInfo = &compMsgData->fields[msgDescPartIdx++]
        }
    #ets_printf{"fieldNameId: %d numtabrows: %d\n", fieldInfo->fieldNameId, numTableRows}
        switch {fieldInfo->fieldNameId} {
        case COMP_MSG_SPEC_FIELD_TABLE_ROW_FIELDS:
          numTableRows = compMsgData->numTableRows
          numTableRowFields = compMsgData->numTableRowFields
          break
        default:
    #ets_printf{"default fieldNameId: %d buildMsgInfo fieldNameId: %d\n", fieldInfo->fieldNameId, self->buildMsgInfos.fieldNameId}
          if {fieldInfo->fieldNameId == msgValPart->fieldNameId} {
            result = self->setMsgFieldValue{self, &numTableRows, &numTableRowFields, type}
            checkErrOK{result}
            msgValPartIdx++
          }
          break
        }
      }
      msgCmdKey = self->currHdr->hdrU16CmdKey
      result = compMsgData->setFieldValue{compMsgData, "@cmdKey", msgCmdKey, NULL}
      checkErrOK{result}
    
      compMsgData->prepareMsg{compMsgData}
      checkErrOK{result}
    compMsgData->dumpMsg{compMsgData}
      return COMP_DISP_ERR_OK
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
      set compMsgData [dict get $compMsgDispatcher compMsgData]
      set result [::compMsg compMsgData initMsg compMsgData]
      set result [setMsgValues compMsgDispatcher
      checkErrOK $result
    
      set result [::compMsg compMsgData getMsgData compMsgData msgData msgLgth]
      checkErrOK $result
      dict set compMsgDispatcher compMsgdata $compMsgData
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
pust stderr [format "transferType: %c dst: 0x%04xn" [dict get $compMsgDispatcher currHdr hdrHandleType] [dict get $compMsgDispatcher currHdr hdrToPart]
      set result [::compMsg sendMsg compMsgDipatcher $msgData $msgLgth]
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
