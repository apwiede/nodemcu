# ===========================================================================
# * Copyright (c) 2016, Arnulf P. Wiedemann (arnulf@wiedemann-pri.de)
# * All rights reserved.
# *
# * License: BSD/MIT
# *
# * Redistribution and use in source and binary forms, with or without
# * modification, are permitted provided that the following conditions
# * are met {
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
# * SUBSTITUTE GOODS OR SERVICES LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# * POSSIBILITY OF SUCH DAMAGE.
# *
# ==========================================================================

# handle types
# A/G/R/S/W/U/N
set ::COMP_DISP_SEND_TO_APP       "A"
set ::COMP_DISP_RECEIVE_FROM_APP  "G"
set ::COMP_DISP_SEND_TO_UART      "R"
set ::COMP_DISP_RECEIVE_FROM_UART "S"
set ::COMP_DISP_TRANSFER_TO_UART  "W"
set ::COMP_DISP_TRANSFER_TO_CONN  "U"
set ::COMP_DISP_NOT_RELEVANT      "N"

set ::GUID_LGTH 16
set ::HDR_FILLER_LGTH 7

# headerPart dict
#   hdrFromPart
#   hdrToPart
#   hdrTotalLgth
#   hdrGUID
#   hdrSrcId
#   hdrfiller
#   hdrU16CmdKey
#   hdrU16CmdLgth
#   hdrU16Crc
#   hdrTargetPart
#   hdrU8CmdKey
#   hdrU8CmdLgth
#   hdrU8Crc
#   hdrOffset
#   hdrEncryption
#   hdrExtraLgth
#   hdrHandleType
#   hdrLgth
#   hdrFlags
#   fieldSequence

# msgHeaderInfos dict
#   headerLgth
#   lgth
#   headerParts
#   numHeaderParts
#   maxHeaderParts
#   currPartIdx
#   seqIdx
#   seqIdxAfterHeader

# msgDescPart dict
#   fieldNameStr
#   fieldNameId
#   fieldTypeStr
#   fieldTypeId
#   fieldLgth
#   fieldKey
#   fieldSize
#   fieldSizeCallback

# msgValPart dict
#   fieldNameStr
#   fieldNameId
#   fieldValueStr     # the value or the callback for getting the value
#   fieldKeyValueStr  # the value for a string
#   fieldValue        # the value for an integer
#   fieldFlags
#   fieldValueCallback
#   fieldValueActionCb

# msgDescriptionInfos
#   numMsgDescriptions
#   maxMsgDescriptions
#   msgDescriptions
#   currMsgDescriptionIdx
#   currSequenceIdx;
#   sequenceIdxAfterHeader

# compMsgMsgDesc
#   lineFields
#   expectedLines
#   numLineFields
#   numMsgFieldGroupInfo
#   maxMsgFieldGroupInfo
#   currMsgFieldGroupInfo
#   msgFieldGroupInfos
#   msgFieldGroupFileName
#   msgDescriptionInfos

set ::moduleFilesPath $::env(HOME)/bene-nodemcu-firmware/module_image_files

namespace eval compMsg {
  namespace ensemble create

    namespace export compMsgMsgDesc

  namespace eval compMsgMsgDesc {
    namespace ensemble create
      
    namespace export getLineFields getIntFieldValue getStringFieldValue addFieldGroup
    namespace export handleMsgFileInternal handleMsgFile handleMsgFileNameLine handleMsgCommonLine
    namespace export handleMsgFieldsToSaveLine handleMsgActionsLine handleMsgValuesLine
    namespace export handleMsgValHeaderLine handleMsgFieldGroupLine handleMsgHeadsLine
    namespace export readHeadersAndSetFlags dumpHeaderPart getHeaderFromUniqueFields
    namespace export getMsgPartsFromHeaderPart getWifiKeyValueKeys readActions
    namespace export resetMsgDescPart resetMsgValPart dumpMsgDescPart dumpMsgValPart
    namespace export getMsgKeyValueDescParts compMsgMsgDescInit addHeaderInfo

    variable headerInfos [list]
    variable received [list]
    variable dispFlags [list]

    # ================================= getLineFields ====================================

    proc getLineFields {compMsgDispatcherVar myStr lgth} {
      upvar $compMsgDispatcherVar compMsgDispatcher

      dict set compMsgDispatcher compMsgMsgDesc numLineFields 0
      dict set compMsgDispatcher compMsgMsgDesc lineFields [split [string trim $myStr "\n"] {,}]
      dict set compMsgDispatcher compMsgMsgDesc numLineFields [llength [dict get $compMsgDispatcher compMsgMsgDesc lineFields]]
      return [checkErrOK OK]
    }

    #  ================================= getIntFieldValue ====================================

    proc  getIntFieldValue {compMsgDispatcherVar cp uvalVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $uvalVar uval

      set uval [expr {0 + $cp}]
      return [checkErrOK OK]
    }

    # ================================= getStringFieldValue ====================================

    proc getStringFieldValue {compMsgDispatcherVar cp strValVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $strValVar strVal

      set strVal [string trim $cp "\""]
      return [checkErrOK OK]
    }

    # ================================= addHeaderInfo ====================================

    proc addHeaderInfo {compMsgDispatcherVar fieldLgth fieldId} {
      upvar $compMsgDispatcherVar compMsgDispatcher

      set result $::COMP_MSG_ERR_OK
      set compMsgMsgDesc [dict get $compMsgDispatcher compMsgMsgDesc]
      set msgHeaderInfo [dict get $compMsgMsgDesc msgHeaderInfo]
      if {[dict get $msgHeaderInfo numHeaderFields] == 0} {
        dict set msgHeaderInfo headerFieldIds [list]
      }
      dict lappend msgHeaderInfo headerFieldIds [list]
      dict incr msgHeaderInfo headerLgth $fieldLgth
      set headerFieldIds [dict get $msgHeaderInfo headerFieldIds]
      set idx [dict get $msgHeaderInfo numHeaderFields]
      set headerFieldIds [lreplace $headerFieldIds $idx $idx $fieldId]
      dict set msgHeaderInfo headerFieldIds $headerFieldIds
      dict incr msgHeaderInfo numHeaderFields
puts stderr [format "addHeaderInfo: numHeaderFields %d: fieldLgth: %d headerLgth: %d fieldId: %d" [dict get $msgHeaderInfo numHeaderFields] $fieldLgth [dict get $msgHeaderInfo headerLgth] $fieldId]
      dict set compMsgMsgDesc msgHeaderInfo $msgHeaderInfo
      dict set compMsgDispatcher compMsgMsgDesc $compMsgMsgDesc
      return $result
    }


    # ================================= addFieldGroup ====================================

    proc addFieldGroup {compMsgDispatcherVar fileName fieldGroupId cmdKey} {
      upvar $compMsgDispatcherVar compMsgDispatcher

      set result OK
puts stderr ">>>handle a: $fileName!$fieldGroupId!$cmdKey!"
      set compMsgMsgDesc [dict get $compMsgDispatcher compMsgMsgDesc]
      set msgFieldGroupInfos [dict get $compMsgMsgDesc msgFieldGroupInfos]

      dict set msgFieldGroupInfo fileName $fileName
      dict set msgFieldGroupInfo maxMsgFieldDesc 0
      dict set msgFieldGroupInfo numMsgFieldDesc 0
      dict set msgFieldGroupInfo msgFieldDescs [list]
      dict set msgFieldGroupInfo maxMsgFieldVal 0
      dict set msgFieldGroupInfo numMsgFieldVal 0
      dict set msgFieldGroupInfo msgFieldVals [list]

      dict set msgFieldGroupInfos $fieldGroupId $cmdKey $msgFieldGroupInfo
      dict set compMsgMsgDesc msgFieldGroupInfos $msgFieldGroupInfos
      dict set compMsgMsgDesc currFieldGroupId $fieldGroupId
      dict set compMsgMsgDesc currCmdKey $cmdKey
      dict set compMsgDispatcher compMsgMsgDesc $compMsgMsgDesc
      return [checkErrOK OK]
    }

    # ================================= handleMsgFileInternal ====================================

    proc handleMsgFileInternal {compMsgDispatcherVar fileName handleMsgLine} {
      upvar $compMsgDispatcherVar compMsgDispatcher

      set compMsgMsgDesc [dict get $compMsgDispatcher compMsgMsgDesc]
      set filePath [format "%s/%s" $::moduleFilesPath $fileName]
#puts stderr "handleMsgFileInternal fileName: $filePath!"
      set fd [open $filePath "r"]
      set numLines 0
#puts stderr "handleMsgFileInternal: $fileName $handleMsgLine fd: $fd!"
if {[dict exists $compMsgMsgDesc msgFieldGroupInfos COMP_MSG_VAL_FIELD_GROUP]} {
puts stderr ">>>handle I: $fileName!keys: [dict keys [dict get $compMsgMsgDesc msgFieldGroupInfos COMP_MSG_VAL_FIELD_GROUP]]!"
}
      while {true} {
        set lgth [gets $fd buffer]
#puts stderr "buffer: $buffer!buffer lgth: $lgth!"
        if {$lgth < 0} {
          if {[expr {$numLines - 1}] != [dict get $compMsgMsgDesc expectedLines]} {
            puts stderr [format "Error numLines: %d expectedLines: %d" $numLines [dict get $compMsgMsgDesc expectedLines]]
            checkErrOK NUM_DESC_FILE_LINES
          }
          break
        }
        set result [getLineFields compMsgDispatcher $buffer lgth]
        checkErrOK $result
        set compMsgMsgDesc [dict get $compMsgDispatcher compMsgMsgDesc]
        set lineFields  [dict get $compMsgMsgDesc lineFields]
        # check if it is eventually an empty line and ignore that
        if {[dict get $compMsgMsgDesc numLineFields] < 2} {
          if {[string length [lindex $lineFields 0]] == 0} {
            continue
          }
        }
        if {[string range $buffer 0 1] eq "# "} {
          # a comment line skip
          continue
        }
        # numLines is always without any comment lines!!
        incr numLines
        if {$numLines == 1} {
          # check if it is the number of lines line
          if {[lindex $lineFields 0] eq "#"} {
            set result [getIntFieldValue compMsgDispatcher [lindex $lineFields 1] cnt]
            checkErrOK $result
            dict set compMsgMsgDesc expectedLines $cnt
            dict set compMsgDispatcher compMsgMsgDesc $compMsgMsgDesc
          } else {
            puts stderr [format "wrong desc file number lines line"]
            checkErrOK WRONG_DESC_FILE_LINE
          }
        } else {
          dict set compMsgDispatcher compMsgMsgDesc $compMsgMsgDesc
          set result [$handleMsgLine compMsgDispatcher]
          checkErrOK $result
          set compMsgMsgDesc [dict get $compMsgDispatcher compMsgMsgDesc]
        }
      }
      dict set compMsgDispatcher compMsgMsgDesc $compMsgMsgDesc
      close $fd
      return [checkErrOK OK]
    }

    # ================================= handleMsgFile ====================================

    proc handleMsgFile {compMsgDispatcherVar fileName handleMsgLine} {
      upvar $compMsgDispatcherVar compMsgDispatcher

puts stderr "handleMsgFile: $fileName!$handleMsgLine!"
      set result [handleMsgFileInternal compMsgDispatcher $fileName $handleMsgLine]
      checkErrOK $result
      set compMsgMsgDesc [dict get $compMsgDispatcher compMsgMsgDesc]
puts stderr "handleMsgFile1 done"
      if {[dict get $compMsgMsgDesc currFieldGroupId] ne [list]} {
        # handle fieldGroup files here
        set msgFieldGroupInfos [dict get $compMsgDispatcher compMsgMsgDesc msgFieldGroupInfos]
puts stderr "MFGI"
#pdict $msgFieldGroupInfos
puts stderr "MFGI DONE"
        set fieldGroupIds [dict keys $msgFieldGroupInfos]
puts stderr ">>>handle0: Ids: $fieldGroupIds!"
        foreach fieldGroupId $fieldGroupIds {
          set cmdKeys [dict keys [dict get $msgFieldGroupInfos $fieldGroupId]]
puts stderr ">>>handle1: $fieldGroupId $cmdKeys!"
puts stderr "FF fieldGroupId: $fieldGroupId!$cmdKeys!"
          foreach cmdKey $cmdKeys {
puts stderr "CK cmdKey: $cmdKey!"
puts stderr ">>>handle2: $cmdKey"
#pdict $cmdKeyDicts
            set compMsgMsgDesc [dict get $compMsgDispatcher compMsgMsgDesc]
            set msgFieldGroupInfos [dict get $compMsgMsgDesc msgFieldGroupInfos]
            dict set compMsgMsgDesc currFieldGroupId $fieldGroupId
            dict set compMsgMsgDesc currCmdKey $cmdKey
puts stderr "file: $fileName fieldGroupId: $fieldGroupId!cmdKey: $cmdKey!"

            set msgFieldGroupInfo [dict get $msgFieldGroupInfos $fieldGroupId $cmdKey]
            set fieldGroupFileName [dict get $msgFieldGroupInfo fileName]
            puts stderr [format ">>>handleFile %s %s %s" $fieldGroupFileName $fieldGroupId $cmdKey]
            dict set compMsgDispatcher compMsgMsgDesc $compMsgMsgDesc
            set result [handleMsgFileInternal compMsgDispatcher $fieldGroupFileName handleMsgFieldGroupLine]
            checkErrOK $result
#pdict [dict get $compMsgDispatcher compMsgMsgDesc msgFieldGroupInfos]
            puts stderr [format "<<<%s: %s %s done" $fieldGroupFileName $fieldGroupId $cmdKey]
          }
        }
      }
      return [checkErrOK OK]
    }

    # ================================= handleMsgFileNameLine ====================================

    proc handleMsgFileNameLine {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher

      set result OK
puts stderr "handleMsgFileNameLine:"
      set compMsgMsgDesc [dict get $compMsgDispatcher compMsgMsgDesc]
      set numLineFields [dict get $compMsgMsgDesc numLineFields]
      if {$numLineFields < 2} {
        checkErrOK FIELD_DESC_TOO_FEW_FIELDS
      }
      set fileNameTokenId 0
      set lineFields [dict get $compMsgMsgDesc lineFields]
      set token [lindex $lineFields 0]
      set field [lindex $lineFields 1]
      set result [getStringFieldValue compMsgDispatcher $field field]
      checkErrOK $result
      set cmdKey "__"
      if {$numLineFields > 2} {
        set value [lindex $lineFields 2]
        set result [getStringFieldValue compMsgDispatcher $value cmdKey]
        checkErrOK $result
      }
      puts stderr [format "token: %s field: %s cmdKey: %s" $token $field $cmdKey]
      if {[string range $token 0 1] eq "@\$"} {
        set result [::compMsg compMsgTypesAndNames getFileNameTokenIdFromStr $token fieldGroupId]
        checkErrOK $result
        dict set compMsgMsgDesc msgFieldGroupFileName $field
        if {$fieldGroupId ne ""} {
          set result [addFieldGroup compMsgDispatcher $field $fieldGroupId $cmdKey]
          checkErrOK $result
        }
      }
      return [checkErrOK OK]
    }

    # ================================= checkMsgFieldDesc ====================================

    proc checkMsgFieldDesc {compMsgDispatcherVar numFields} {
      upvar $compMsgDispatcherVar compMsgDispatcher

      set result OK
      set compMsgMsgDesc [dict get $compMsgDispatcher compMsgMsgDesc]
      set msgFieldGroupInfos [dict get $compMsgMsgDesc msgFieldGroupInfos]
      set fieldGroupId [dict get $compMsgMsgDesc currFieldGroupId]
      set cmdKey [dict get $compMsgMsgDesc currCmdKey]
      set msgFieldGroupInfo [dict get $msgFieldGroupInfos $fieldGroupId $cmdKey]
      if {[dict get $compMsgMsgDesc numLineFields] < $numFields} {
        checkErrOK FIELD_DESC_TOO_FEW_FIELDS
      }
      if {[dict get $msgFieldGroupInfo maxMsgFieldDesc] == 0} {
        set numExpectedLines [dict get $compMsgMsgDesc expectedLines]
        dict set msgFieldGroupInfo maxMsgFieldDesc $numExpectedLines
        set lst [list]
        set descIdx 0
puts stderr "maxMsgFieldDesc: $numExpectedLines!"
        while {$descIdx < $numExpectedLines} {
          lappend lst [list]
          incr descIdx
        }
        dict set msgFieldGroupInfo msgFieldDescs $lst
      }
      dict set msgFieldGroupInfos $fieldGroupId $cmdKey $msgFieldGroupInfo
      dict set compMsgMsgDesc msgFieldGroupInfos $msgFieldGroupInfos
      dict set compMsgDispatcher compMsgMsgDesc $compMsgMsgDesc
      return [checkErrOK OK]
    }

    # ================================= checkMsgFieldVal ====================================

    proc checkMsgFieldVal {compMsgDispatcherVar numFields} {
      upvar $compMsgDispatcherVar compMsgDispatcher

      set result OK
      set compMsgMsgDesc [dict get $compMsgDispatcher compMsgMsgDesc]
      set msgFieldGroupInfos [dict get $compMsgMsgDesc msgFieldGroupInfos]
      set fieldGroupId [dict get $compMsgMsgDesc currFieldGroupId]
      set cmdKey [dict get $compMsgMsgDesc currCmdKey]
      set msgFieldGroupInfo [dict get $msgFieldGroupInfos $fieldGroupId $cmdKey]
      if {[dict get $compMsgMsgDesc numLineFields] < $numFields} {
        checkErrOK FIELD_DESC_TOO_FEW_FIELDS
      }
      if {[dict get $msgFieldGroupInfo maxMsgFieldVal] == 0} {
        set numExpectedLines [dict get $compMsgMsgDesc expectedLines]
        dict set msgFieldGroupInfo maxMsgFieldVal $numExpectedLines
        set lst [list]
        set valIdx 0
puts stderr "maxMsgFieldVal: $numExpectedLines!"
        while {$valIdx < $numExpectedLines} {
          lappend lst [list]
          incr valIdx
        }
        dict set msgFieldGroupInfo msgFieldVals $lst
      }
      dict set msgFieldGroupInfos $fieldGroupId $cmdKey $msgFieldGroupInfo
      dict set compMsgMsgDesc msgFieldGroupInfos $msgFieldGroupInfos
      dict set compMsgDispatcher compMsgMsgDesc $compMsgMsgDesc
      return [checkErrOK OK]
    }

    # ================================= handleMsgCommonLine ====================================

    proc handleMsgCommonLine {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher

      set result OK
set fieldName [lindex [dict get $compMsgDispatcher compMsgMsgDesc lineFields] 0]
puts stderr "common line fieldName!$fieldName!"
if {0} {
      set result [checkMsgFieldDesc compMsgDispatcher 3]
      checkErrOK $result
}
      set compMsgMsgDesc [dict get $compMsgDispatcher compMsgMsgDesc]
      set msgFieldGroupInfos [dict get $compMsgMsgDesc msgFieldGroupInfos]
      set fieldGroupId [dict get $compMsgMsgDesc currFieldGroupId]
      set cmdKey [dict get $compMsgMsgDesc currCmdKey]
puts stderr "fieldGroupId: $fieldGroupId cmdKey: $cmdKey!"
      set msgFieldGroupInfo [dict get $msgFieldGroupInfos $fieldGroupId $cmdKey]
      set lineFields [dict get $compMsgMsgDesc lineFields]

      set fieldInfo [dict create]
      dict set fieldInfo fieldFlags [list]
      dict set fieldInfo fieldOffset 0
      dict set fieldInfo keyValueDesc [list]

      #field name
      set fieldName [lindex $lineFields 0]
      set result [::compMsg compMsgTypesAndNames getFieldNameIdFromStr compMsgDispatcher $fieldName fieldNameIdStr $::COMP_MSG_INCR]
      checkErrOK $result
      if {![string is integer $fieldNameIdStr]} {
        set result [::compMsg compMsgTypesAndNames getSpecialFieldNameIntFromId $fieldNameIdStr fieldNameId]
        checkErrOK $result
      } else {
        set fieldNameId $fieldNameIdStr
      }
      dict set fieldInfo fieldNameId $fieldNameId

      #field type
      set fieldType [lindex $lineFields 1]
      set result [::compMsg dataView getFieldTypeIdFromStr $fieldType fieldTypeId]
      checkErrOK $result
      dict set fieldInfo fieldTypeId $fieldTypeId

      #field lgth
      set fieldLgth [lindex $lineFields 2]
      set result [getIntFieldValue compMsgDispatcher $fieldLgth lgth]
      checkErrOK $result
      dict set fieldInfo fieldLgth $lgth

      puts stderr [format "%s: id: %s type: %s %s lgth: %d" $fieldName $fieldNameId $fieldType $fieldTypeId $fieldLgth]

      # field key/value id
      if {$fieldGroupId eq "COMP_MSG_DESC_KEY_VALUE_FIELD_GROUP"} {
        set keyValue [lindex $lineFields 3]
        set result [getIntFieldValue compMsgDispatcher $keyValue keyValueId]
        checkErrOK $result
      }

      # field header flag
      if {$fieldGroupId eq "COMP_MSG_DESC_HEADER_FIELD_GROUP"} {
        set headerFlagValue [lindex $lineFields 3]
        set result [getIntFieldValue compMsgDispatcher $headerFlagValue headerFlag]
        checkErrOK $result
        dict set fieldInfo fieldOffset [dict get $compMsgMsgDesc msgHeaderInfo headerLgth]
puts stderr "headerFlag: $headerFlag!"
        dict lappend fieldInfo fieldFlags COMP_MSG_FIELD_HEADER
        switch $headerFlag {
          0 {
          }
          1 {
            dict lappend fieldInfo fieldFlags COMP_MSG_FIELD_HEADER_UNIQUE
          }
          2 {
            dict lappend fieldInfo fieldFlags COMP_MSG_FIELD_HEADER_CHKSUM_NON_ZERO
          }
          default {
            return $::COMP_MSG_ERR_BAD_HEADER_FIELD_FLAG
          }
        }
        set result [::compMsg compMsgMsgDesc addHeaderInfo compMsgDispatcher [dict get $fieldInfo fieldLgth] $fieldNameId]
      }

      switch $fieldGroupId {
        COMP_MSG_DESC_HEADER_FIELD_GROUP {
puts stderr "HEADER: fieldInfo: $fieldInfo!"
          set result [::compMsg compMsgTypesAndNames setMsgFieldInfo compMsgDispatcher $fieldNameId fieldInfo]
          checkErrOK $result
        }
        COMP_MSG_DESC_MID_PART_FIELD_GROUP {
          set result [::compMsg compMsgTypesAndNames setMsgFieldInfo compMsgDispatcher $fieldNameId fieldInfo]
          checkErrOK $result
        }
        COMP_MSG_DESC_TRAILER_FIELD_GROUP {
          set result [::compMsg compMsgTypesAndNames setMsgFieldInfo compMsgDispatcher $fieldNameId fieldInfo]
          checkErrOK $result
        }
        COMP_MSG_DESC_KEY_VALUE_FIELD_GROUP {
          set result [::compMsg compMsgTypesAndNames setMsgFieldInfo compMsgDispatcher $fieldNameId fieldInfo]
          checkErrOK $result
        }
        COMP_MSG_DESC_FIELD_GROUP {
          set result [::compMsg compMsgTypesAndNames setMsgFieldInfo compMsgDispatcher $fieldNameId fieldInfo]
          checkErrOK $result
        }
      }
      return [checkErrOK OK]
    }

    # ================================= handleMsgFieldsToSaveLine ====================================

    proc handleMsgFieldsToSaveLine {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher

      set result OK
      set result [checkMsgFieldDesc compMsgDispatcher 1]
      checkErrOK $result
      set compMsgMsgDesc [dict get $compMsgDispatcher compMsgMsgDesc]
      set msgFieldGroupInfos [dict get $compMsgMsgDesc msgFieldGroupInfos]
      set fieldGroupId [dict get $compMsgMsgDesc currFieldGroupId]
      set cmdKey [dict get $compMsgMsgDesc currCmdKey]
      set msgFieldGroupInfo [dict get $msgFieldGroupInfos $fieldGroupId $cmdKey]
      set msgFieldDescs [dict get $msgFieldGroupInfo msgFieldDescs]
      set descIdx [dict get $msgFieldGroupInfo numMsgFieldDesc]
      set msgFieldDesc [lindex $msgFieldDescs $descIdx]
      # field name
      set fieldName [lindex [dict get $compMsgMsgDesc lineFields] 0]
      set result [::compMsg compMsgTypesAndNames getFieldNameIdFromStr compMsgDispatcher $fieldName fieldNameId $::COMP_MSG_INCR]
      checkErrOK $result
      dict set msgFieldDesc fieldNameId $fieldNameId
      puts stderr [format "%s: id: %d" $fieldName  $fieldNameId]
      set msgFieldDescs [lreplace $msgFieldDescs $descIdx $descIdx $msgFieldDesc]
      dict set msgFieldGroupInfo msgFieldDescs $msgFieldDescs
      dict incr msgFieldGroupInfo numMsgFieldDesc
      dict set msgFieldGroupInfos $fieldGroupId  $cmdKey $msgFieldGroupInfo
      dict set compMsgMsgDesc msgFieldGroupInfos $msgFieldGroupInfos
      dict set compMsgDispatcher compMsgMsgDesc $compMsgMsgDesc
      return [checkErrOK OK]
    }

    # ================================= handleMsgActionsLine ====================================

    proc handleMsgActionsLine {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher

      set result OK
      set result [checkMsgFieldDesc compMsgDispatcher 2]
      checkErrOK $result
      set compMsgMsgDesc [dict get $compMsgDispatcher compMsgMsgDesc]
      set msgFieldGroupInfos [dict get $compMsgMsgDesc msgFieldGroupInfos]
      set fieldGroupId [dict get $compMsgMsgDesc currFieldGroupId]
      set cmdKey [dict get $compMsgMsgDesc currCmdKey]
      set msgFieldGroupInfo [dict get $msgFieldGroupInfos $fieldGroupId $cmdKey]
      set msgFieldDescs [dict get $msgFieldGroupInfo msgFieldDescs]
      set descIdx [dict get $msgFieldGroupInfo numMsgFieldDesc]
      set msgFieldDesc [lindex $msgFieldDescs $descIdx]
      # field name
      set fieldName [lindex [dict get $compMsgMsgDesc lineFields] 0]
      set result [::compMsg compMsgTypesAndNames getFieldNameIdFromStr compMsgDispatcher $fieldName fieldNameId $::COMP_MSG_INCR]
      checkErrOK $result
      dict set msgFieldDesc fieldNameId $fieldNameId
      # field value
      set fieldValue [lindex [dict get $compMsgMsgDesc lineFields] 1]
      dict set msgFieldDesc value $fieldValue
      puts stderr [format "%s: id: %d val: %s" $fieldName $fieldNameId $fieldValue]
      checkErrOK $result
      set msgFieldDescs [lreplace $msgFieldDescs $descIdx $descIdx $msgFieldDesc]
      dict set msgFieldGroupInfo msgFieldDescs $msgFieldDescs
      dict incr msgFieldGroupInfo numMsgFieldDesc
      dict set msgFieldGroupInfos $fieldGroupId $cmdKey $msgFieldGroupInfo
      dict set compMsgMsgDesc msgFieldGroupInfos $msgFieldGroupInfos
      dict set compMsgDispatcher compMsgMsgDesc $compMsgMsgDesc
      return [checkErrOK OK]
    }

    # ================================= handleMsgValuesLine ====================================

    proc handleMsgValuesLine {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher

puts stderr "handleMsgValuesLine"
      set result OK
      set compMsgMsgDesc [dict get $compMsgDispatcher compMsgMsgDesc]
      set msgFieldGroupInfos [dict get $compMsgMsgDesc msgFieldGroupInfos]
      set fieldGroupId [dict get $compMsgMsgDesc currFieldGroupId]
      set cmdKey [dict get $compMsgMsgDesc currCmdKey]
      set msgFieldGroupInfo [dict get $msgFieldGroupInfos $fieldGroupId $cmdKey]
      if {[dict get $compMsgMsgDesc numLineFields] < 2} {
        checkErrOK FIELD_DESC_TOO_FEW_FIELDS
      }
puts stderr "handleMsgValuesLine: [dict get $compMsgMsgDesc lineFields]!"
      set lineFields [dict get $compMsgMsgDesc lineFields]
      #field name
      set token [lindex $lineFields 0]
      set result [::compMsg compMsgTypesAndNames getFieldNameIdFromStr compMsgDispatcher $token fieldNameId $::COMP_MSG_INCR]
      checkErrOK $result
      #field value
      set value [lindex $lineFields 1]
      set stringValue ""
      set numericValue 0
      set dataValue [dict create]
      dict set dataValue cmdKey $cmdKey
      dict set dataValue fieldValueId 0
      dict set dataValue fieldNameId 0
      dict set dataValue fieldValueCallback [list]
      if {[string range $value 0 0] eq "\""} {
        set result [getStringFieldValue compMsgDispatcher $value stringValue]
        dict set dataValue flags FIELD_IS_STRING
        dict set dataValue value $stringValue
      } else {
        if {[string range $value 0 0] eq "@"} {
          dict set dataValue flags [list]
          dict set dataValue value [list]
          dict set dataValue fieldValueCallback $value
        } else {
puts stderr "VA: $value!"
          set result [getIntFieldValue compMsgDispatcher $value numericValue]
          dict set dataValue flags FIELD_IS_NUMERIC
          dict set dataValue value $numericValue
        }
      }
      checkErrOK $result
      puts stderr [format "%s: id: %s val: %s %d" $token $fieldNameId $stringValue $numericValue]
      switch $fieldGroupId {
        COMP_MSG_WIFI_DATA_VALUES_FIELD_GROUP -
        COMP_MSG_MODULE_DATA_VALUES_FIELD_GROUP {
          set result [::compMsg compMsgDataValue dataValueStr2ValueId compMsgDispatcher $token fieldValueId]
          checkErrOK $result
puts stderr "token: $token fieldValueId: $fieldValueId!"
          dict set dataValue fieldValueId $fieldValueId
          set result [::compMsg compMsgDataValue addDataValue compMsgDispatcher $dataValue dataValueIdx]
          checkErrOK $result
          dict incr msgFieldGroupInfo numMsgFieldVal
          dict incr msgFieldGroupInfo maxMsgFieldVal
          dict lappend msgFieldGroupInfo msgFieldVals $dataValueIdx
        }
        COMP_MSG_VAL_FIELD_GROUP  {
puts stderr "token: $token fieldNameId: $fieldNameId!callback: [dict get $dataValue fieldValueCallback]!cmdKey: $cmdKey"
          dict set dataValue fieldNameId $fieldNameId
          set result [::compMsg compMsgDataValue addDataValue compMsgDispatcher $dataValue dataValueIdx]
          checkErrOK $result
          dict incr msgFieldGroupInfo numMsgFieldVal
          dict incr msgFieldGroupInfo maxMsgFieldVal
          dict lappend msgFieldGroupInfo msgFieldVals $dataValueIdx
        }
        default {
          checkErrOK BAD_DESC_FILE_FIELD_GROUP_TYPE
        }
      }
      dict set msgFieldGroupInfos $fieldGroupId $cmdKey $msgFieldGroupInfo
      dict set compMsgMsgDesc msgFieldGroupInfos $msgFieldGroupInfos
      dict set compMsgDispatcher compMsgMsgDesc $compMsgMsgDesc
      return [checkErrOK OK]
    }

   #  ================================= handleMsgValHeaderLine ====================================

    proc handleMsgValHeaderLine {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher

      set result OK
      set result [checkMsgFieldVal compMsgDispatcher 2]
      checkErrOK $result
      set compMsgMsgDesc [dict get $compMsgDispatcher compMsgMsgDesc]
      set msgFieldGroupInfos [dict get $compMsgMsgDesc msgFieldGroupInfos]
      set fieldGroupId [dict get $compMsgMsgDesc currFieldGroupId]
      set cmkdKey [dict get $compMsgMsgDesc currCmdKey]
      set msgFieldGroupInfo [dict get $msgFieldGroupInfos $fieldGroupId $cmdKey]
      set valIdx [dict get $msgFieldGroupInfo numMsgFieldVal]
      set msgFieldVals [dict get $msgFieldGroupInfo msgFieldVals]
      set msgFieldVal [lindex $msgFieldVals $valIdx]
      # field name
      set fieldName [lindex [dict get $compMsgMsgDesc lineFields] 0]
      set result [::compMsg compMsgTypesAndNames getFieldNameIdFromStr compMsgDispatcher $fieldName fieldNameId $::COMP_MSG_INCR]
      checkErrOK $result
      # field value
      set fieldValue [lindex [dict get $compMsgMsgDesc lineFields] 1]
puts stderr "fieldValue: $fieldValue!valIdx: $valIdx!"
      if {[string range $fieldValue 0 0] eq "\""} {
        set result [getStringFieldValue compMsgDispatcher $fieldValue stringVal]
        dict set msgFieldVal value $stringVal
        puts stderr [format "%s: id: %s val: %s!" $fieldName $fieldNameId [dict get $msgFieldVal value]]
      } else {
        set result [getIntFieldValue compMsgDispatcher $fieldValue value]
        dict set msgFieldVal value $value 
        puts stderr [format "%s: id: %s %s" $fieldName $fieldNameId [dict get $msgFieldVal value]]
      }
      checkErrOK $result
      dict incr msgFieldGroupInfo numMsgFieldVal
      set msgFieldVals [lreplace $msgFieldVals $valIdx $valIdx $msgFieldVal]
      dict set msgFieldGroupInfo msgFieldVals $msgFieldVals
      dict set msgFieldGroupInfos $fieldGroupId $msgFieldGroupInfo
      dict set compMsgMsgDesc msgFieldGroupInfos $msgFieldGroupInfos
      dict set compMsgDispatcher compMsgMsgDesc $compMsgMsgDesc
      return [checkErrOK OK]
    }

    # ================================= handleMsgFieldGroupLine ====================================

    proc handleMsgFieldGroupLine {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher

puts stderr "handleMsgFieldGroupLine"
      set result OK
      set compMsgMsgDesc [dict get $compMsgDispatcher compMsgMsgDesc]
      set msgFieldGroupInfos [dict get $compMsgMsgDesc msgFieldGroupInfos]
      set fieldGroupId [dict get $compMsgMsgDesc currFieldGroupId]
      set cmdKey [dict get $compMsgMsgDesc currCmdKey]
      set msgFieldGroupInfo [dict get $msgFieldGroupInfos $fieldGroupId $cmdKey]
puts stderr "handleMsgFieldGroupLine: fieldGroupId: $fieldGroupId cmdkey: $cmdKey!"
      switch $fieldGroupId {
        COMP_MSG_DESC_HEADER_FIELD_GROUP -
        COMP_MSG_DESC_MID_PART_FIELD_GROUP -
        COMP_MSG_DESC_TRAILER_FIELD_GROUP -
        COMP_MSG_DESC_KEY_VALUE_FIELD_GROUP {
          set result [handleMsgCommonLine compMsgDispatcher]
          checkErrOK $result
        }
        COMP_MSG_FIELDS_TO_SAVE_FIELD_GROUP {
          set result [handleMsgFieldsToSaveLine compMsgDispatcher]
          checkErrOK $result
        }
        COMP_MSG_ACTIONS_FIELD_GROUP {
          set result [handleMsgActionsLine compMsgDispatcher]
          checkErrOK $result
        }
        COMP_MSG_VAL_HEADER_FIELD_GROUP {
          set result [handleMsgValHeaderLine compMsgDispatcher]
          checkErrOK $result
        }
        COMP_MSG_HEADS_FIELD_GROUP {
          set result [handleMsgHeadsLine compMsgDispatcher]
          checkErrOK $result
        }
        COMP_MSG_WIFI_DATA_VALUES_FIELD_GROUP {
          set result [handleMsgValuesLine compMsgDispatcher]
          checkErrOK $result
        }
        COMP_MSG_MODULE_DATA_VALUES_FIELD_GROUP {
          set result [handleMsgValuesLine compMsgDispatcher]
          checkErrOK $result
        }
        COMP_MSG_VAL_FIELD_GROUP {
          set result [handleMsgValuesLine compMsgDispatcher]
          checkErrOK $result
        }
        COMP_MSG_DESC_FIELD_GROUP {
          set result [handleMsgCommonLine compMsgDispatcher]
          checkErrOK $result
        }
        default {
          puts stderr [format "bad desc file fieldGroupType 0x%02x" [dict get $msgFieldGroupInfo fieldGroupType]]
          checkErrOK BAD_DESC_FILE_FIELD_GROUP_TYPE
        }
      }
      return [checkErrOK OK]
    }

    # ================================= handleMsgHeadsLine ====================================

    proc handleMsgHeadsLine {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher

      set result OK
      set result [::compMsg compMsgUtil addFieldDescription compMsgDispatcher]
      checkErrOK $result
      set compMsgMsgDesc [dict get $compMsgDispatcher compMsgMsgDesc]
      set compMsgTypesAndNames [dict get $compMsgDispatcher compMsgTypesAndNames]
      set msgHeaderInfo [dict get $compMsgMsgDesc msgHeaderInfo]
      set msgDescriptionInfos [dict get $compMsgMsgDesc msgDescriptionInfos]
      set msgDescriptions [dict get $msgDescriptionInfos msgDescriptions]
      set descIdx [dict get $msgDescriptionInfos currMsgDescriptionIdx]
      set msgDescription [lindex $msgDescriptions $descIdx]
      set msgFieldGroupInfos [dict get $compMsgMsgDesc msgFieldGroupInfos]
      if {[dict get $msgHeaderInfo numHeaderFields] == 0} {
        checkErrOK HEADER_FIELD_GROUP_NOT_FOUND
      }
      if {[dict get $compMsgMsgDesc numLineFields] < 3} {
        checkErrOK FIELD_DESC_TOO_FEW_FIELDS
      }
      set fieldIdx 1
      set headerFieldIdx 0
      set msgFieldInfos [dict get $compMsgTypesAndNames msgFieldInfos]
      set fieldInfos [dict get $msgFieldInfos fieldInfos]
      while {$fieldIdx < [dict get $compMsgTypesAndNames numSpecFieldIds]} {
        set fieldInfo [lindex $fieldInfos $fieldIdx]
        if {$fieldInfo eq [list]} {
          incr fieldIdx
	  continue
        }
puts stderr "FLD: fieldIdx: $fieldIdx!fieldInfo: $fieldInfo!"
	if {[lsearch [dict get $fieldInfo fieldFlags] COMP_MSG_FIELD_HEADER] >= 0} {
puts stderr "handleMsgHeadsLine: fieldIdx: $fieldIdx"
          set result [::compMsg compMsgTypesAndNames getFieldNameStrFromId compsMsgDispatcher $fieldIdx fieldNameStr]
	  checkErrOK $result
          set lineFields [dict get $compMsgMsgDesc lineFields]
          set value [lindex $lineFields $headerFieldIdx]
          puts stderr [format "field: %s %s" $fieldNameStr $value]
          if {[lsearch [dict get $fieldInfo fieldFlags] COMP_MSG_FIELD_HEADER_UNIQUE] >= 0} {
            if {$value eq "*"} {
# FIXME what to do with joker char?
#              dict set msgFieldDesc fieldFlags COMP_MSG_VAL_IS_JOKER
#              dict set msgFieldDesc msgFieldVal $value
            } else {
              if {[string range $value 0 0] eq "\""} {
                switch [dict get $msgFieldDesc fieldTypeId] {
                  DATA_VIEW_FIELD_UINT8_T -
                  DATA_VIEW_FIELD_INT8_T -
                  DATA_VIEW_FIELD_UINT16_T -
                  DATA_VIEW_FIELD_INT16_T -
                  DATA_VIEW_FIELD_UINT32_T -
                  DATA_VIEW_FIELD_INT32_T {
                  checkErrOK EXPECTED_INT_VALUE
                  }
                }
                set stringVal ""
                set result [getStringFieldValue compMsgDispatcher $value stringVal]
                checkErrOK $result
                puts stderr [format "value: %s!stringVal: %s" $value $stringVal]
	        set result [::compMsg compMsgDataView setIdFieldValue compMsgDispatcher dataView $fieldIdx $stringVal 0]
	        checkErrOK $result
              } else {
                set result [getIntFieldValue compMsgDispatcher $value intVal]
	        set result [::compMsg compMsgDataView setIdFieldValue compMsgDispatcher dataView $fieldIdx $intVal 0]
	        checkErrOK $result
              }
            }
          }
          incr headerFieldIdx
        }
        incr fieldIdx
      }
#      dict set msgFieldGroupInfos $fieldGroupId $cmdKey $msgFieldGroupInfo
#      dict set compMsgMsgDesc msgFieldGroupInfos $msgFieldGroupInfos
      set lineFields [dict get $compMsgMsgDesc lineFields]
      set field [lindex $lineFields $fieldIdx]
      set field [string trim $field {\"}]
      if {[string length $field] > 1} {
        checkErrOK BAD_ENCRYPTED_VALUE
      }
      dict set msgDescription encrypted [string range $field 0 0]
      incr fieldIdx
      set field [lindex $lineFields $fieldIdx]
      set field [string trim $field {\"}]
      if {[string length $field] > 1} {
        checkErrOK BAD_HANDLE_TYPE_VALUE
      }
      dict set msgDescription handleType [string range $field 0 0]
      # FIXME need to handle cmdKey here!!!
      puts stderr [format "msgDescription->headerLgth: %d encrypted: %s handleType: %s" [dict get $msgDescription headerLgth] [dict get $msgDescription encrypted] [dict get $msgDescription handleType]]
      set msgDescriptions [lreplace $msgDescriptions $descIdx $descIdx $msgDescription]
      dict set msgDescriptionInfos msgDescriptions $msgDescriptions
      dict set compMsgMsgDesc msgDescriptionInfos $msgDescriptionInfos
      dict set compMsgDispatcher compMsgMsgDesc $compMsgMsgDesc
      return [checkErrOK OK]
    }

    # ================================= dumpHeaderPart ====================================
    
    proc dumpHeaderPart {hdr} {
      puts stderr "dumpHeaderPart:"
      if {![dict exists $hdr hdrOffset]} {
        dict set hdr hdrOffset 0
      }
      if {![dict exists $hdr hdrU16CmdKey]} {
        dict set hdr hdrU16CmdKey ""
      }
      if {![dict exists $hdr hdrU8CmdKey]} {
        dict set hdr hdrU8CmdKey ""
      }
      if {![dict exists $hdr hdrU16CmdLgth]} {
        dict set hdr hdrU16CmdLgth 0
      }
      if {![dict exists $hdr hdrU8CmdLgth]} {
        dict set hdr hdrU8CmdLgth 0
      }
      if {![dict exists $hdr hdrU16Crc]} {
        dict set hdr hdrU16Crc 0
      }
      if {![dict exists $hdr hdrU8Crc]} {
        dict set hdr hdrU8Crc 0
      }
      if {![dict exists $hdr hdrU16TotalCrc]} {
        dict set hdr hdrU16TotalCrc 0
      }
      if {![dict exists $hdr hdrU8TotalCrc]} {
        dict set hdr hdrU8TotalCrc 0
      }
      if {![dict exists $hdr fieldSequence]} {
        dict set hdr fieldSequence [list]
      }
      puts stderr [format "headerParts: from: 0x%04x to: 0x%04x totalLgth: %d u16CmdKey: %s\n" [dict get $hdr hdrFromPart] [dict get $hdr hdrToPart] [dict get $hdr hdrTotalLgth] [dict get $hdr hdrU16CmdKey]]
      puts stderr [format "             u16CmdLgth: 0x%04x u16Crc: 0x%04x u16TotalCrc: 0x%04x\n" [dict get $hdr hdrU16CmdLgth] [dict get $hdr hdrU16Crc] [dict get $hdr hdrU16TotalCrc]]
      puts stderr [format "             u8CmdKey: %s u8CmdLgth: %d u8Crc: 0x%02x u8TotalCrc; 0x%02x\n" [dict get $hdr hdrU8CmdKey] [dict get $hdr hdrU8CmdLgth] [dict get $hdr hdrU8Crc] [dict get $hdr hdrU8TotalCrc]]
      puts stderr [format "             enc: %s handleType: %s offset: %d" [dict get $hdr hdrEncryption] [dict get $hdr hdrHandleType] [dict get $hdr hdrOffset]]
      puts stderr "hdrFlags: [dict get $hdr hdrFlags]"
      puts stderr "hdr fieldSequence"
      set fieldSequence [dict get $hdr fieldSequence]
      set idx 0
      while {[lindex $fieldSequence $idx] ne [list]} {
        puts stderr [format "%d %s" $idx [lindex $fieldSequence $idx]]
        incr idx
      }
      return [checkErrOK OK]
    }
    
    # ================================= dumpMsgHeaderInfos ====================================
    
    proc dumpMsgHeaderInfos {hdrInfos} {
      puts stderr "dumpMsgHeaderInfos:\n"
      if {![dict exists $hdrInfos maxHeaderParts]} {
        dict set hdrInfos maxHeaderParts 0
      }
      if {![dict exists $hdrInfos currPartIdx]} {
        dict set hdrInfos currPartIdx 0
      }
      if {![dict exists $hdrInfos seqIdx]} {
        dict set hdrInfos seqIdx 0
      }
      if {![dict exists $hdrInfos seqIdxAfterStart]} {
        dict set hdrInfos seqIdxAfterStart 0
      }
      puts stderr "headerFlags: [dict get $hdrInfos headerFlags]"
      puts stderr "hdrInfos headerSequence\n"
      set idx 0
      set headerSequence [dict get $hdrInfos headerSequence]
      while {[lindex $headerSequence $idx] ne [list]} {
        puts stderr [format " %d %s" $idx [lindex $headerSequence $idx]]
        incr idx
      }
      puts stderr [format "startLgth: %d numParts: %d maxParts: %d currPartIdx: %d seqIdx: %d seqIdxAfterStart: %d\n" [dict get $hdrInfos headerLgth] [dict get $hdrInfos numHeaderParts] [dict get $hdrInfos maxHeaderParts] [dict get $hdrInfos currPartIdx] [dict get $hdrInfos seqIdx] [dict get $hdrInfos seqIdxAfterStart]]
      return [checkErrOK OK]
    }
    
    # ================================= getStartFieldsFromLine ====================================
    
    proc getStartFieldsFromLine {line seqIdxVar} {
      variable headerInfos
      upvar $seqIdxVar seqIdx
    
      set flds [split $line ,]
      set lgth [llength $flds]
      set fieldIdx 0
      dict set headerInfos headerLgth [lindex $flds $fieldIdx]
      incr fieldIdx
      while {$fieldIdx < $lgth} {
        set fieldName [lindex $flds $fieldIdx]
        if {[string range $fieldName 0 0] ne "@"} {
          checkErrOK NO_SUCH_FIELD
        }
        set result [::compMsg compMsgDataView getFieldNameIdFromStr compMsgDispatcher $fieldName fieldNameId $::COMP_MSG_NO_INCR]
        checkErrOK $result
        switch $fieldNameId {
          COMP_MSG_SPEC_FIELD_SRC {
            if {[lsearch [dict get $headerInfos headerFlags] COMP_DISP_HDR_SRC] >= 0} {
              return DUPLICATE_FIELD
            }
            dict lappend headerInfos headerSequence COMP_DISP_U16_SRC
            incr seqIdx
            dict lappend headerInfos headerFlags COMP_DISP_HDR_SRC
          }
          COMP_MSG_SPEC_FIELD_DST {
            if {[lsearch [dict get $headerInfos headerFlags] COMP_DISP_HDR_DST] >= 0} {
              return DUPLICATE_FIELD
            }
            dict lappend headerInfos headerSequence COMP_DISP_U16_DST
            incr seqIdx
            dict lappend headerInfos headerFlags COMP_DISP_HDR_DST
          }
          COMP_MSG_SPEC_FIELD_TOTAL_LGTH {
            if {[lsearch [dict get $headerInfos headerFlags] COMP_DISP_HDR_TOTAL_LGTH] >= 0} {
              return DUPLICATE_FIELD
            }
            dict lappend headerInfos headerSequence COMP_DISP_U16_TOTAL_LGTH
            incr seqIdx
            dict lappend headerInfos headerFlags COMP_DISP_HDR_TOTAL_LGTH
          }
          COMP_MSG_SPEC_FIELD_SRC_ID {
            if {[lsearch [dict get $headerInfos headerFlags] COMP_DISP_HDR_SRC_ID] >= 0} {
              return DUPLICATE_FIELD
            }
            dict lappend headerInfos headerSequence COMP_DISP_U16_SRC_ID
            incr seqIdx
            dict lappend headerInfos headerFlags COMP_DISP_HDR_SRC_ID
          }
          COMP_MSG_SPEC_FIELD_GUID {
            if {[lsearch [dict get $headerInfos headerFlags] COMP_DISP_HDR_GUID] >= 0} {
              return DUPLICATE_FIELD
            }
            dict lappend headerInfos headerSequence COMP_DISP_U8_VECTOR_GUID
            incr seqIdx
            dict lappend headerInfos headerFlags COMP_DISP_HDR_GUID
          }
          COMP_MSG_SPEC_FIELD_HDR_FILLER {
            if {[lsearch [dict get $headerInfos headerFlags] COMP_DISP_HDR_FILLER] >= 0} {
              return DUPLICATE_FIELD
            }
            dict lappend headerInfos headerSequence COMP_DISP_U8_VECTOR_HDR_FILLER
            incr seqIdx
            dict lappend headerInfos headerFlags COMP_DISP_HDR_FILLER
          }
          default {
            checkErrOK NO_SUCH_FIELD
          }
        }
        incr fieldIdx
      }
      dict set headerInfos seqIdxAfterStart $seqIdx
      return [checkErrOK OK]
    }
      
    # ================================= readHeadersAndSetFlags ====================================
    
    proc readHeadersAndSetFlags {compMsgDispatcherVar fileName} {
      upvar $compMsgDispatcherVar compMsgDispatcher
      variable headerInfos
      variable dispFlags
    
#puts stderr "readHeadersAndSetFlags!"
      set fd [open $fileName r]
      gets $fd line
      set flds [split $line ,]
      foreach {dummy numEntries} $flds break
      set headerInfos [dict create]
      dict set headerInfos numHeaderParts 0
      dict set headerInfos headerFlags [list]
      # parse header start description
      gets $fd line
      set seqIdx 0
      set result [getStartFieldsFromLine $line seqIdx]
      dict set headerInfos headerParts [list]
      set fieldOffset 0
      set seqStartIdx $seqIdx
      set idx 0
      while {$idx < $numEntries} {
        dict set headerInfos headerSequence [lrange [dict get $headerInfos headerSequence] 0 [expr {$seqIdx - 1 }]]
        gets $fd line
        if {[string length $line] == 0} {
          checkErrOK TOO_FEW_FILE_LINES
        }
        set hdr [dict create]
        set seqIdx2 0
        set fieldSequence [list]
        set headerSequence [dict get $headerInfos headerSequence]
        dict set hdr hdrFlags [list]
        set flds [split $line ,]
        set seqIdx2 0
        while {$seqIdx2 < $seqStartIdx} {
          dict lappend hdr fieldSequence [lindex $headerSequence $seqIdx2]
          set str [lindex $flds $seqIdx2]
          if {[string range $str 0 0] eq "*"} {
            set isJoker true
          } else {
            set isJoker false
          }
          switch [lindex [dict get $hdr fieldSequence] $seqIdx2] {
            COMP_DISP_U16_SRC {
              if {$isJoker} {
                dict set hdr hdrFromPart 0
              } else {
                dict set hdr hdrFromPart $str
              }
            }
            COMP_DISP_U16_DST {
              if {$isJoker} {
                dict set hdr hdrToPart 0
              } else {
                dict set hdr hdrToPart $str
              }
            }
            COMP_DISP_U16_TOTAL_LGTH {
              if {$isJoker} {
                dict set hdr hdrTotalLgth 0
              } else {
                dict set hdr hdrTotalLgth $str
              }
            }
            COMP_DISP_U16_SRC_ID {
              if {$isJoker} {
                dict set hdr hdrSrcId 0
              } else {
                dict set hdr hdrSrcId $str
              }
            }
            COMP_DISP_U8_VECTOR_GUID {
              if {$isJoker} {
                dict set hdr hdrGUID 0
              } else {
                dict set hdr hdrGUID $str
              }
            }
            COMP_DISP_U8_VECTOR_HDR_FILLER {
              if {$isJoker} {
                dict set hdr hdrFiller 0
              } else {
                dict set hdr hdrFiller $str
              }
            }
            default {
              checkErrOK FIELD_NOT_FOUND
            }
          }
          incr seqIdx2
        }
        set seqIdx3 $seqIdx2
        set myPart [lindex $flds $seqIdx3]
        # encryption E/N
        dict set hdr hdrEncryption $myPart
        incr seqIdx3
        set myPart [lindex $flds $seqIdx3]
        # handleType A/G/S/R/U/W/N
        dict set hdr hdrHandleType $myPart
        incr seqIdx3
        set myPart [lindex $flds $seqIdx3]
        # type of cmdKey
        set result [::compMsg dataView getFieldTypeIdFromStr $myPart fieldTypeId]
        checkErrOK $result
        incr seqIdx3
        set myPart [lindex $flds $seqIdx3]
        # cmdKey
        switch $fieldTypeId {
          DATA_VIEW_FIELD_UINT8_T {
            dict lappend hdr fieldSequence COMP_DISP_U8_CMD_KEY
            dict lappend hdr hdrFlags COMP_DISP_PAYLOAD_CMD_KEY
            dict set hdr hdrU8CmdKey $myPart
            lappend dispFlags COMP_MSG_U8_CMD_KEY
          }
          DATA_VIEW_FIELD_UINT16_T {
            dict lappend hdr fieldSequence COMP_DISP_U16_CMD_KEY
            dict lappend hdr hdrFlags COMP_DISP_PAYLOAD_CMD_KEY
            dict set hdr hdrU16CmdKey $myPart
          }
          default {
           checkErrOK BAD_FIELD_TYPE
          }
        }
        incr seqIdx3
        set myPart [lindex $flds $seqIdx3]
        # type of cmdLgth
        set result [::compMsg dataView getFieldTypeIdFromStr $myPart fieldTypeId]
        checkErrOK $result
        set isEnd false
        if {$seqIdx2 >= [llength $flds]} {
          set isEnd true
        }
        switch $fieldTypeId {
          DATA_VIEW_FIELD_NONE {
            dict lappend hdr fieldSequence COMP_DISP_U0_CMD_LGTH
            dict lappend hdr hdrFlags COMP_DISP_PAYLOAD_CMD_LGTH
          }
          DATA_VIEW_FIELD_UINT8_T {
            dict lappend hdr fieldSequence COMP_DISP_U8_CMD_LGTH
            dict lappend hdr hdrFlags COMP_DISP_PAYLOAD_CMD_LGTH
          }
          DATA_VIEW_FIELD_UINT16_T {
            dict lappend hdr fieldSequence COMP_DISP_U16_CMD_LGTH
            dict lappend hdr hdrFlags COMP_DISP_PAYLOAD_CMD_LGTH
          }
          default {
            checkErrOK BAD_FIELD_TYPE
          }
        }
        # type of crc
        if {!$isEnd} {
          incr seqIdx3
          set myPart [lindex $flds $seqIdx3]
          set result [::compMsg dataView getFieldTypeIdFromStr $myPart fieldTypeId]
          checkErrOK $result
          switch $fieldTypeId {
            DATA_VIEW_FIELD_NONE {
              dict lappend hdr fieldSequence COMP_DISP_U0_CRC
              dict lappend hdr hdrFlags COMP_DISP_PAYLOAD_CRC
            }
            DATA_VIEW_FIELD_UINT8_T {
              dict lappend hdr fieldSequence COMP_DISP_U8_CRC
              dict lappend hdr hdrFlags COMP_DISP_PAYLOAD_CRC
            }
            DATA_VIEW_FIELD_UINT16_T {
              dict lappend hdr fieldSequence COMP_DISP_U16_CRC
              dict lappend hdr hdrFlags COMP_DISP_PAYLOAD_CRC
            }
            default {
              checkErrOK BAD_FIELD_TYPE
            }
          }
        }
        # type of totalCrc
        if {!$isEnd} {
          incr seqIdx3
          set myPart [lindex $flds $seqIdx3]
          set result [::compMsg dataView getFieldTypeIdFromStr $myPart fieldTypeId]
          checkErrOK $result
          switch $fieldTypeId {
            DATA_VIEW_FIELD_NONE {
              dict lappend hdr fieldSequence COMP_DISP_U0_TOTAL_CRC
              dict lappend hdr hdrFlags COMP_DISP_TOTAL_CRC
            }
            DATA_VIEW_FIELD_UINT8_T {
              dict lappend hdr fieldSequence COMP_DISP_U8_TOTAL_CRC
              dict lappend hdr hdrFlags COMP_DISP_TOTAL_CRC
            }
            DATA_VIEW_FIELD_UINT16_T {
              dict lappend hdr fieldSequence COMP_DISP_U16_TOTAL_CRC
              dict lappend hdr hdrFlags COMP_DISP_TOTAL_CRC
            }
            default {
              checkErrOK BAD_FIELD_TYPE
            }
          }
        }
        dict lappend headerInfos headerParts $hdr
        dict set headerInfos numHeaderParts [expr {[dict get $headerInfos numHeaderParts] + 1}]
        incr idx
      }
      close $fd
      dict set headerInfos currPartIdx 0
      dict set compMsgDispatcher headerInfos $headerInfos
#puts stderr "readHeadersAndSetFlags done!"
      return [checkErrOK OK]
    }

    # ================================= readActions ====================================

    proc readActions {compMsgDispatcherVar fileName} {
      upvar $compMsgDispatcherVar compMsgDispatcher
    
      set fd [open [format "%s/$fileName" $::moduleFilesPath] "r"]
      gets $fd line
      set flds [split $line ","]
      foreach {dummy numEntries} $flds break
      set idx 0
      while {$idx < $numEntries} {
        gets $fd line
        set flds [split $line ","]
        foreach {actionName actionMode cmdKeyType cmdKey} $flds break
        set result [::compMsg compMsgAction setActionEntry compMsgDispatcher $actionName $actionMode $cmdKey]
        checkErrOK $result
        incr idx
      }
      close $fd
      return [checkErrOK OK]
    }

      
    # ================================= getHeaderFromUniqueFields ====================================
    
    proc getHeaderFromUniqueFields {dst src cmdKey hdrVar} {
      variable headerInfos
      upvar $hdrVar hdr

      set headerParts [dict get $headerInfos headerParts]
      set idx 0
      while {$idx < [dict get $headerInfos numHeaderParts]} {
        set hdr [lindex $headerParts $idx]
        if {[dict get $hdr hdrToPart] eq $dst} {
          if {[dict get $hdr hdrFromPart] eq $src} {
            if {[dict get $hdr hdrU16CmdKey] eq $cmdKey} {
               return [checkErrOK OK]
            }
          }
        }
        incr idx
      }
      checkErrOK HEADER_NOT_FOUND
    }

    # ================================= dumpMsgDescPart ====================================

    proc dumpMsgDescPart {compMsgDispatcherVar msgDescPart} {
      upvar $compMsgDispatcherVar compMsgDispatcher

      set callbackName [list]
      if {[dict get $msgDescPart fieldSizeCallback] ne [list]} {
        set result [::compMsg compMsgAction getActionCallbackName compMsgDispatcher [dict get $msgDescPart fieldSizeCallback] callbackName]
        checkErrOK $result
      }
      puts stderr [format "msgDescPart: fieldNameStr: %-15.15s fieldNameId: %-35.35s fieldTypeStr: %-10.10s fieldTypeId: %-30.30s field_lgth: %d callback: %s" [dict get $msgDescPart fieldNameStr] [dict get $msgDescPart fieldNameId] [dict get $msgDescPart fieldTypeStr] [dict get $msgDescPart fieldTypeId] [dict get $msgDescPart fieldLgth] $callbackName]
      return [checkErrOK OK]
    }

    # ================================= dumpMsgValPart ====================================

    proc dumpMsgValPart {compMsgDispatcherVar msgValPart} {
      upvar $compMsgDispatcherVar compMsgDispatcher

      set callbackName [list]
      if {[dict get $msgValPart fieldValueCallback] ne [list]} {
        set result [::compMsg compMsgAction getActionCallbackName compMsgDispatcher [dict get $msgValPart fieldValueCallback] callbackName]
        checkErrOK $result
      }
      puts -nonewline stderr [format "msgValPart: fieldNameStr: %-15.15s fieldNameId: %-35.35s fieldValueStr: %-20.20s callback: %s flags: " [dict get $msgValPart fieldNameStr] [dict get $msgValPart fieldNameId] [dict get $msgValPart fieldValueStr] $callbackName]
      if {[lsearch [dict get $msgValPart fieldFlags] COMP_DISP_DESC_VALUE_IS_NUMBER] >= 0} {
         puts -nonewline stderr " COMP_DISP_DESC_VALUE_IS_NUMBER"
      }
      puts stderr ""
      return [checkErrOK OK]
    }

    # ================================= resetMsgDescParts ====================================

    proc resetMsgDescParts {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher

      dict set compMsgMsgDesc msgDescParts [list]
      dict set compMsgMsgDesc numMsgDescParts 0
      dict set compMsgMsgDesc maxMsgDescParts 0
      return [checkErrOK OK]
    }

    # ================================= resetMsgValParts ====================================

    proc resetMsgValParts {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher

      dict set compMsgMsgDesc msgValParts [list]
      dict set compMsgMsgDesc numMsgValParts 0
      dict set compMsgMsgDesc maxMsgValParts 0
      return [checkErrOK OK]
    }

    # ================================= getMsgPartsFromHeaderPart ====================================
    
    proc getMsgPartsFromHeaderPart {compMsgDispatcherVar hdr handle} {
      variable headerInfos
      upvar $compMsgDispatcherVar compMsgDispatcher
    
      dict set compMsgDispatcher currHdr $hdr
      dict set compMsgDispatcher compMsgData [list]
      dict set compMsgDispatcher compMsgData msgDescParts [list]
      set msgDescParts [dict get $compMsgDispatcher compMsgData msgDescParts]
      dict set compMsgDispatcher compMsgData msgValParts [list]
      set msgValParts [dict get $compMsgDispatcher compMsgData msgValParts]
      set fileName [format "%s/CompDesc%s.txt" $::moduleFilesPath [dict get $hdr hdrU16CmdKey]]
      set fd [open $fileName "r"]
      gets $fd line
      set flds [split $line ","]
      set prepareValuesCbName [list]
      foreach {dummy numEntries prepareValuesCbName} $flds break
#puts stderr "numDesc!$numEntries!$prepareValuesCbName!"
      dict set compMsgDispatcher compMsgMsgDesc prepareValuesCbName $prepareValuesCbName
      dict set compMsgDispatcher compMsgMsgDesc numMsgDescParts $numEntries
      set numRows 0
      set idx 0
      while {$idx < $numEntries} {
        gets $fd line
        if {$line eq ""} {
          checkErrOK TOO_FEW_FILE_LINES
        }
        set flds [split $line ","]
        set callback [list]
        foreach {fieldNameStr fieldTypeStr fieldLgthStr callback} $flds break
#puts stderr "fieldNameStr: $fieldNameStr!"
        if {$fieldLgthStr eq "@numRows"} {
          set fieldLgth $numRows
        } else {
          set fieldLgth $fieldLgthStr
        }
        set msgDescPart [dict create]
        dict set msgDescPart fieldNameId 0
        dict set msgDescPart fieldTypeId 0
        dict set msgDescPart fieldKey ""
        dict set msgDescPart fieldSize 0
        dict set msgDescPart fieldSizeCallback $callback
        dict set msgDescPart fieldNameStr $fieldNameStr
        dict set msgDescPart fieldTypeStr $fieldTypeStr
        dict set msgDescPart fieldLgth $fieldLgth
        set result [::compMsg dataView getFieldTypeIdFromStr $fieldTypeStr fieldTypeId]
        checkErrOK $result
        dict set msgDescPart fieldTypeId $fieldTypeId
        set result [::compMsg compMsgDataView getFieldNameIdFromStr compMsgDispatcher $fieldNameStr fieldNameId $::COMP_MSG_INCR]
        checkErrOK $result
        dict set msgDescPart fieldNameId $fieldNameId
#dumpMsgDescPart compMsgDispatcher $msgDescPart
        lappend msgDescParts $msgDescPart
        incr idx
      }
      close $fd
      dict set compMsgDispatcher compMsgData msgDescParts $msgDescParts

      # and now the value parts
      set fileName [format "%s/CompVal%s.txt" $::moduleFilesPath [dict get $hdr hdrU16CmdKey]]
      set fd [open $fileName "r"]
      gets $fd line
      set flds [split $line ","]
      set callback [list]
      foreach {dummy numEntries callback} $flds break
      set numRows 0
#puts stderr "numVal: $numEntries!$callback!"
      dict set compMsgDispatcher compMsgMsgDesc numMsgValParts $numEntries
      set idx 0
      while {$idx < $numEntries} {
        gets $fd line
        if {$line eq ""} {
          checkErrOK TOO_FEW_FILE_LINES
        }
        set flds [split $line ","]
        set callback [list]
        foreach {fieldNameStr fieldValueStr} $flds break
#puts stderr "val fieldName: $fieldNameStr!"
        # fieldName
        set result [::compMsg compMsgDataView getFieldNameIdFromStr compMsgDispatcher $fieldNameStr fieldNameId $::COMP_MSG_NO_INCR]
        checkErrOK $result
        if {[string range $fieldValueStr 0 3] eq "@get"} {
          set callback [string range $fieldValueStr 1 end]
        }
        set fieldValueActionCb [list]
        if {[string range $fieldValueStr 0 3] eq "@run"} {
          set fieldValueActionCb $fieldValueStr
        }
    
        set msgValPart [dict create]
        dict set msgValPart fieldNameId $fieldNameId
        dict set msgValPart fieldFlags [list]
        dict set msgValPart fieldKeyValueStr [list]
        dict set msgValPart fieldValue [list]
        dict set msgValPart fieldValueCallback $callback
        dict set msgValPart fieldNameStr $fieldNameStr
        dict set msgValPart fieldValueStr $fieldValueStr
        dict set msgValPart fieldValueAcvtionCb $fieldValueActionCb
        lappend msgValParts $msgValPart
#dumpMsgValPart compMsgDispatcher $msgValPart
        incr idx
      }
      close $fd
      dict set compMsgDispatcher compMsgData msgValParts $msgValParts
#puts stderr "getMsgPartsFromHeaderPart done"
      return [checkErrOK OK]
    }

    # ================================= setWifiKeyData ====================================
    
    proc setWifiData {compMsgDispatcherVar compMsgWifiDataVar key fieldTypeStr fieldValueStr} {
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $compMsgWifiDataVar wifiData

      set result [::compMsg compMsgWifiData bssStr2BssInfoId $key bssInfoType]
      checkErrOK $result
      switch $bssInfoType {
        BSS_INFO_BSSID {
          dict set wifiData bssKeys bssid $fieldValueStr
        }
        BSS_INFO_SSID {
          dict set wifiData bssKeys ssid $fieldValueStr
        }
        BSS_INFO_SSID_LEN {
          dict set wifiData bssKeys ssid_len $fieldValueStr
        }
        BSS_INFO_CHANNEL {
          dict set wifiData bssKeys channel $fieldValueStr
        }
        BSS_INFO_RSSI {
          dict set wifiData bssKeys rssi $fieldValueStr
        }
        BSS_INFO_AUTH_MODE {
          dict set wifiData bssKeys authmode $fieldValueStr
        }
        BSS_INFO_IS_HIDDEN {
          dict set wifiData bssKeys freq_offset $fieldValueStr
        }
        BSS_INFO_FREQ_OFFSET {
          dict set wifiData bssKeys freqcal_val $fieldValueStr
        }
        BSS_INFO_FREQ_CAL_VAL {
          dict set wifiData bssKeys is_hidden $fieldValueStr
        }
      }
    
      checkErrOK $result
      set result [::compMsg dataView getFieldTypeIdFromStr $fieldTypeStr fieldTypeId]
      checkErrOK $result
      switch $bssInfoType {
        BSS_INFO_BSSID {
          dict set wifiData bssTypes bssid $fieldTypeId
        }
        BSS_INFO_SSID {
          dict set wifiData bssTypes ssid $fieldTypeId
        }
        BSS_INFO_CHANNEL {
          dict set wifiData bssTypes channel $fieldTypeId
        }
        BSS_INFO_RSSI {
          dict set wifiData bssTypes rssi $fieldTypeId
        }
        BSS_INFO_AUTH_MODE {
          dict set wifiData bssTypes authmode $fieldTypeId
        }
        BSS_INFO_IS_HIDDEN {
          dict set wifiData bssTypes freq_offset $fieldTypeId
        }
        BSS_INFO_FREQ_OFFSET {
          dict set wifiData bssTypes freqcal_val $fieldTypeId
        }
        BSS_INFO_FREQ_CAL_VAL {
          dict set wifiData bssTypes is_hidden $fieldTypeId
        }
      }
      return [checkErrOK OK]
    }

    # ================================= getMsgKeyValueDescParts ====================================
    
    proc getMsgKeyValueDescParts {compMsgDispatcherVar fileName} {
      upvar $compMsgDispatcherVar compMsgDispatcher

      set fd [open [format "%s/%s" $::moduleFilesPath $fileName] "r"]
      gets $fd line
      set flds [split $line ","]
      foreach {dummy numEntries} $flds break
      set idx 0
      dict set compMsgDispatcher msgKeyValueDescParts [list]
      dict set compMsgDispatcher numMsgKeyValueDescParts 0
      dict set compMsgDispatcher maxMsgKeyValueDescParts $numEntries
      while {$idx < $numEntries} {
        gets $fd line
        set flds [split $line ","]
        foreach {fieldNameStr fieldValueStr fieldTypeStr fieldLgth} $flds break
        set msgKeyValueDescPart [dict create]
        dict set msgKeyValueDescPart keyNameStr $fieldNameStr
        dict set msgKeyValueDescPart keyId $fieldValueStr
        dict set msgKeyValueDescPart keyType $fieldTypeStr
        dict set msgKeyValueDescPart keyLgth $fieldLgth
        dict lappend compMsgDispatcher msgKeyValueDescParts $msgKeyValueDescPart
        dict incr compMsgDispatcher numMsgKeyValueDescParts
        incr idx
      }
      close $fd
#puts stderr "getMsgKeyValueDescParts done"
      return [checkErrOK OK]
    }

    # ================================= getWifiKeyValueKeys ====================================
    
    proc getWifiKeyValueKeys {compMsgDispatcherVar compMsgWifiDataVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $compMsgWifiDataVar wifiData

puts stderr ">>>getWifiKeyValueKeys"
      set fd [open [format "%s/CompMsgKeyValueKeys.txt" $::moduleFilesPath] "r"]
      gets $fd line
      set flds [split $line ","]
      foreach {dummy numEntries} $flds break
      set idx 0
      while {$idx < $numEntries} {
        gets $fd line
        set flds [split $line ","]
        foreach {fieldNameStr fieldValueStr fieldTypeStr fieldLgth} $flds break
        set offset [string length "@key_"]
        set key [string range $fieldNameStr $offset end]
puts stderr "keyValueKey: $key!"
        switch $key {
          seqNum -
          MACAddr -
          machineState -
          firmwareMainBoard -
          firmwareDisplayBoard -
          firmwareWifiModule -
          lastError -
          casingUseList -
          casingStatisticList -
          dataAndTime -
          clientSsid -
          clientPasswd {
            set result [::compMsg compMsgModuleData setModuleValue compMsgDispatcher key_$key $fieldValueStr]
          }
          bssid -
          ssid -
          rssi -
          channel -
          auth_mode -
          is_hidden -
          freq_offset -
          freq_cal_val {
            set result [setWifiData compMsgDispatcher wifiData $key $fieldTypeStr $fieldValueStr]
          }
          default {
puts stderr "should handle key: $key $fieldTypeStr $fieldValueStr $fieldLgth!"
          }
        }
        incr idx
      }
      close $fd
puts stderr "getWifiKeyValues done"
      return [checkErrOK OK]
    }

    # ================================= compMsgMsgDescInit ====================================
    
    proc compMsgMsgDescInit {compMsgDispatcherVar fileName} {
      upvar $compMsgDispatcherVar compMsgDispatcher

      set compMsgMsgDesc [dict create]
      dict set compMsgMsgDesc lineFields [list]
      dict set compMsgMsgDesc expectedLines 0
      dict set compMsgMsgDesc numLineFields 0
      dict set compMsgMsgDesc numMsgFieldGroupInfo 0
      dict set compMsgMsgDesc maxMsgFieldGroupInfo 0
      dict set compMsgMsgDesc currMsgFieldGroupInfo 0
      dict set compMsgMsgDesc msgFieldGroupInfos [list]

      set msgHeaderInfo [dict create]
      dict set msgHeaderInfo headerLgth 0
      dict set msgHeaderInfo numHeaderFields 0
      dict set msgHeaderInfo headerFieldIds [list]
      dict set compMsgMsgDesc msgHeaderInfo $msgHeaderInfo

      set msgDescriptionInfos [dict create]
      dict set msgDescriptionInfos numMsgDescriptions 0
      dict set msgDescriptionInfos maxMsgDescriptions 0
      dict set msgDescriptionInfos msgDescriptions [list]
      dict set msgDescriptionInfos currMsgDescriptionIdx 0
      dict set msgDescriptionInfos currSequenceIdx 0
      dict set msgDescriptionInfos sequenceIdxAfterHeader 0

      dict set compMsgMsgDesc msgDescriptionInfos $msgDescriptionInfos

      dict set compMsgDispatcher compMsgMsgDesc $compMsgMsgDesc
      set result [handleMsgFile compMsgDispatcher $fileName handleMsgFileNameLine]
      checkErrOK $result
pdict $compMsgDispatcher
puts stderr "================="
set val [::dict2json $compMsgDispatcher]
puts $val
      return [checkErrOK OK]
    }

  } ; # namespace compMsgMsgDesc
}  ; # namespace compMsg
