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
    namespace export handleMsgFieldGroupLine handleMsgHeadsLine
    namespace export readHeadersAndSetFlags dumpHeaderPart getMsgDescriptionFromUniqueFields
    namespace export getMsgPartsFromHeaderPart getWifiKeyValueKeys readActions
    namespace export resetMsgDescPart resetMsgValPart dumpMsgDescPart dumpMsgValPart
    namespace export getMsgKeyValueDescParts compMsgMsgDescInit addHeaderInfo addMidPartInfo addTrailerInfo

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

    # ================================= addMidPartInfo ====================================

    proc addMidPartInfo {compMsgDispatcherVar fieldLgth fieldId} {
      upvar $compMsgDispatcherVar compMsgDispatcher

      set result $::COMP_MSG_ERR_OK
      set compMsgMsgDesc [dict get $compMsgDispatcher compMsgMsgDesc]
      set msgMidPartInfo [dict get $compMsgMsgDesc msgMidPartInfo]
      if {[dict get $msgMidPartInfo numMidPartFields] == 0} {
        dict set msgMidPartInfo midPartFieldIds [list]
      }
      dict lappend msgMidPartInfo midPartFieldIds [list]
      dict incr msgMidPartInfo midPartLgth $fieldLgth
      set midPartFieldIds [dict get $msgMidPartInfo midPartFieldIds]
      set idx [dict get $msgMidPartInfo numMidPartFields]
      set midPartFieldIds [lreplace $midPartFieldIds $idx $idx $fieldId]
      dict set msgMidPartInfo midPartFieldIds $midPartFieldIds
      dict incr msgMidPartInfo numMidPartFields
puts stderr [format "addMidPartInfo: numMidPartFields %d: fieldLgth: %d midPartLgth: %d fieldId: %d" [dict get $msgMidPartInfo numMidPartFields] $fieldLgth [dict get $msgMidPartInfo midPartLgth] $fieldId]
      dict set compMsgMsgDesc msgMidPartInfo $msgMidPartInfo
      dict set compMsgDispatcher compMsgMsgDesc $compMsgMsgDesc
      return $result
    }

    # ================================= addTrailerInfo ====================================

    proc addTrailerInfo {compMsgDispatcherVar fieldLgth fieldId} {
      upvar $compMsgDispatcherVar compMsgDispatcher

      set result $::COMP_MSG_ERR_OK
      set compMsgMsgDesc [dict get $compMsgDispatcher compMsgMsgDesc]
      set msgTrailerInfo [dict get $compMsgMsgDesc msgTrailerInfo]
      if {[dict get $msgTrailerInfo numTrailerFields] == 0} {
        dict set msgTrailerInfo trailerFieldIds [list]
      }
      dict lappend msgTrailerInfo trailerFieldIds [list]
      dict incr msgTrailerInfo trailerLgth $fieldLgth
      set trailerFieldIds [dict get $msgTrailerInfo trailerFieldIds]
      set idx [dict get $msgTrailerInfo numTrailerFields]
      set trailerFieldIds [lreplace $trailerFieldIds $idx $idx $fieldId]
      dict set msgTrailerInfo trailerFieldIds $trailerFieldIds
      dict incr msgTrailerInfo numTrailerFields
puts stderr [format "addTrailerInfo: numTrailerFields %d: fieldLgth: %d trailerLgth: %d fieldId: %d" [dict get $msgTrailerInfo numTrailerFields] $fieldLgth [dict get $msgTrailerInfo trailerLgth] $fieldId]
      dict set compMsgMsgDesc msgTrailerInfo $msgTrailerInfo
      dict set compMsgDispatcher compMsgMsgDesc $compMsgMsgDesc
      return $result
    }

    # ================================= addFieldGroup ====================================

    proc addFieldGroup {compMsgDispatcherVar fileName fieldGroupId cmdKey} {
      upvar $compMsgDispatcherVar compMsgDispatcher

      set result OK
#puts stderr ">>>handle a: $fileName!$fieldGroupId!$cmdKey!"
      set compMsgMsgDesc [dict get $compMsgDispatcher compMsgMsgDesc]
      set msgFieldGroupInfos [dict get $compMsgMsgDesc msgFieldGroupInfos]

      dict set msgFieldGroupInfo fileName $fileName
      switch $fieldGroupId {
        COMP_MSG_ACTIONS_FIELD_GROUP -
        COMP_MSG_FIELDS_TO_SAVE_FIELD_GROUP {
          dict set msgFieldGroupInfo maxMsgFieldDesc 0
          dict set msgFieldGroupInfo numMsgFieldDesc 0
          dict set msgFieldGroupInfo msgFieldDescs [list]
        }
      }
      switch $fieldGroupId {
        COMP_MSG_WIFI_DATA_VALUES_FIELD_GROUP -
        COMP_MSG_MODULE_DATA_VALUES_FIELD_GROUP -
        COMP_MSG_HEADER_VAL_FIELD_GROUP -
        COMP_MSG_VAL_FIELD_GROUP {
          dict set msgFieldGroupInfo maxMsgFieldVal 0
          dict set msgFieldGroupInfo numMsgFieldVal 0
          dict set msgFieldGroupInfo msgFieldVals [list]
        }
      }

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
#puts stderr ">>>handle I: $fileName!keys: [dict keys [dict get $compMsgMsgDesc msgFieldGroupInfos COMP_MSG_VAL_FIELD_GROUP]]!"
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
            dict set compMsgMsgDesc currLineNo 0
            dict set compMsgDispatcher compMsgMsgDesc $compMsgMsgDesc
          } else {
            puts stderr [format "wrong desc file number lines line"]
            checkErrOK WRONG_DESC_FILE_LINE
          }
        } else {
          dict incr compMsgMsgDesc currLineNo
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

#puts stderr "handleMsgFile: $fileName!$handleMsgLine!"
      set result [handleMsgFileInternal compMsgDispatcher $fileName $handleMsgLine]
      checkErrOK $result
      set compMsgMsgDesc [dict get $compMsgDispatcher compMsgMsgDesc]
#puts stderr "handleMsgFile1 done"
      if {[dict get $compMsgMsgDesc currFieldGroupId] ne [list]} {
        # handle fieldGroup files here
        set msgFieldGroupInfos [dict get $compMsgDispatcher compMsgMsgDesc msgFieldGroupInfos]
        set fieldGroupIds [dict keys $msgFieldGroupInfos]
#puts stderr ">>>handle0: Ids: $fieldGroupIds!"
        foreach fieldGroupId $fieldGroupIds {
          set cmdKeys [dict keys [dict get $msgFieldGroupInfos $fieldGroupId]]
          foreach cmdKey $cmdKeys {
            set compMsgMsgDesc [dict get $compMsgDispatcher compMsgMsgDesc]
            set msgFieldGroupInfos [dict get $compMsgMsgDesc msgFieldGroupInfos]
            dict set compMsgMsgDesc currFieldGroupId $fieldGroupId
            dict set compMsgMsgDesc currCmdKey $cmdKey
#puts stderr "file: $fileName fieldGroupId: $fieldGroupId!cmdKey: $cmdKey!"

            set msgFieldGroupInfo [dict get $msgFieldGroupInfos $fieldGroupId $cmdKey]
            set fieldGroupFileName [dict get $msgFieldGroupInfo fileName]
#puts stderr [format ">>>handleFile %s %s %s" $fieldGroupFileName $fieldGroupId $cmdKey]
            dict set compMsgDispatcher compMsgMsgDesc $compMsgMsgDesc
            set result [handleMsgFileInternal compMsgDispatcher $fieldGroupFileName handleMsgFieldGroupLine]
            checkErrOK $result
#puts stderr [format "<<<%s: %s %s done" $fieldGroupFileName $fieldGroupId $cmdKey]
          }
        }
      }
      return [checkErrOK OK]
    }

    # ================================= handleMsgFileNameLine ====================================

    proc handleMsgFileNameLine {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher

      set result OK
#puts stderr "handleMsgFileNameLine:"
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
#puts stderr [format "token: %s field: %s cmdKey: %s" $token $field $cmdKey]
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
      set compMsgMsgDesc [dict get $compMsgDispatcher compMsgMsgDesc]
set fieldName [lindex [dict get $compMsgMsgDesc lineFields] 0]
puts stderr "common line fieldName!$fieldName!expected: [dict get $compMsgMsgDesc expectedLines]!curr: [dict get $compMsgMsgDesc currLineNo]!"
      if {[dict get $compMsgMsgDesc numLineFields] < 3} {
        checkErrOK FIELD_DESC_TOO_FEW_FIELDS
      }
      set compMsgMsgDesc [dict get $compMsgDispatcher compMsgMsgDesc]
      set msgFieldGroupInfos [dict get $compMsgMsgDesc msgFieldGroupInfos]
      set fieldGroupId [dict get $compMsgMsgDesc currFieldGroupId]
puts stderr "fGI: $fieldGroupId!"
      set cmdKey [dict get $compMsgMsgDesc currCmdKey]
#puts stderr "fieldGroupId: $fieldGroupId cmdKey: $cmdKey!"
      set msgFieldGroupInfo [dict get $msgFieldGroupInfos $fieldGroupId $cmdKey]
      set lineFields [dict get $compMsgMsgDesc lineFields]

      set fieldDescInfo [dict create]
      dict set fieldDescInfo fieldFlags [list]
      dict set fieldDescInfo fieldOffset 0
      dict set fieldDescInfo keyValueDesc [list]

      #field name
      set fieldName [lindex $lineFields 0]
      if {[string match "@#*" $fieldName]} {
        dict lappend fieldDescInfo fieldFlags COMP_MSG_FIELD_KEY_VALUE
      }
      set result [::compMsg compMsgTypesAndNames getFieldNameIdFromStr compMsgDispatcher $fieldName fieldNameIdStr $::COMP_MSG_INCR]
      checkErrOK $result
      if {![string is integer $fieldNameIdStr]} {
        set result [::compMsg compMsgTypesAndNames getSpecialFieldNameIntFromId $fieldNameIdStr fieldNameId]
        checkErrOK $result
      } else {
        set fieldNameId $fieldNameIdStr
      }
      dict set fieldDescInfo fieldNameId $fieldNameId

      #field type
      set fieldType [lindex $lineFields 1]
      set result [::compMsg dataView getFieldTypeIdFromStr $fieldType fieldTypeId]
      checkErrOK $result
      dict set fieldDescInfo fieldTypeId $fieldTypeId

      #field lgth
      set fieldLgth [lindex $lineFields 2]
      set result [getIntFieldValue compMsgDispatcher $fieldLgth lgth]
      checkErrOK $result
      dict set fieldDescInfo fieldLgth $lgth

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
        dict set fieldDescInfo fieldOffset [dict get $compMsgMsgDesc msgHeaderInfo headerLgth]
        dict lappend fieldDescInfo fieldFlags COMP_MSG_FIELD_HEADER
puts stderr "headerFlag: $fieldName: $fieldDescInfo!"
        switch $headerFlag {
          0 {
          }
          1 {
            dict lappend fieldDescInfo fieldFlags COMP_MSG_FIELD_HEADER_UNIQUE
          }
          2 {
            dict lappend fieldDescInfo fieldFlags COMP_MSG_FIELD_HEADER_CHKSUM_NON_ZERO
          }
          default {
            return $::COMP_MSG_ERR_BAD_HEADER_FIELD_FLAG
          }
        }
        set result [::compMsg compMsgMsgDesc addHeaderInfo compMsgDispatcher [dict get $fieldDescInfo fieldLgth] $fieldNameId]
      }

      switch $fieldGroupId {
        COMP_MSG_DESC_HEADER_FIELD_GROUP {
#puts stderr "HEADER: fieldDescInfo: $fieldDescInfo!"
          set result [::compMsg compMsgTypesAndNames setMsgFieldDescInfo compMsgDispatcher $fieldNameId fieldDescInfo]
          checkErrOK $result
        }
        COMP_MSG_DESC_MID_PART_FIELD_GROUP {
          dict set fieldDescInfo fieldOffset [expr {[dict get $compMsgMsgDesc msgHeaderInfo headerLgth] + [dict get $compMsgMsgDesc msgMidPartInfo midPartLgth]}]
          set result [::compMsg compMsgMsgDesc addMidPartInfo compMsgDispatcher [dict get $fieldDescInfo fieldLgth] $fieldNameId]
          checkErrOK $result
          set result [::compMsg compMsgTypesAndNames setMsgFieldDescInfo compMsgDispatcher $fieldNameId fieldDescInfo]
          checkErrOK $result
        }
        COMP_MSG_DESC_TRAILER_FIELD_GROUP {
          # FIXME need to set fieldOffset here !!!
          set result [::compMsg compMsgMsgDesc addTrailerInfo compMsgDispatcher [dict get $fieldDescInfo fieldLgth] $fieldNameId]
          checkErrOK $result
          set result [::compMsg compMsgTypesAndNames setMsgFieldDescInfo compMsgDispatcher $fieldNameId fieldDescInfo]
          checkErrOK $result
        }
        COMP_MSG_DESC_KEY_VALUE_FIELD_GROUP {
          set result [::compMsg compMsgTypesAndNames setMsgFieldDescInfo compMsgDispatcher $fieldNameId fieldDescInfo]
          checkErrOK $result
        }
        COMP_MSG_DESC_FIELD_GROUP {
          set result [::compMsg compMsgTypesAndNames setMsgFieldDescInfo compMsgDispatcher $fieldNameId fieldDescInfo]
          checkErrOK $result
        }
      }
      # add description infos
      set compMsgTypesAndNames [dict get $compMsgDispatcher compMsgTypesAndNames]
      set msgFieldInfos [dict get $compMsgTypesAndNames msgFieldInfos]
      set fieldDescInfos [dict get $msgFieldInfos fieldDescInfos]
      if {$fieldGroupId eq "COMP_MSG_DESC_FIELD_GROUP"} {
        set msgDescriptionInfos [dict get $compMsgMsgDesc msgDescriptionInfos]
        set descIdx 0
        set msgDescriptions [dict get $msgDescriptionInfos msgDescriptions]
        binary scan $cmdKey S cmdKeyVal
        while {$descIdx < [dict get $msgDescriptionInfos numMsgDescriptions]} {
          set msgDescription [lindex $msgDescriptions $descIdx]
          if {$cmdKeyVal == [dict get $msgDescription cmdKey]} {
            break
          }
          incr descIdx
        }
puts stderr "found: $descIdx!$msgDescription!$fieldDescInfo!"
puts stderr "keys: [dict keys $msgDescription]!"
        if {[llength [dict get $msgDescription fieldSequence]] == 0} {
          # add the header and midPart field ids before appending the message specific ids
          set msgHeaderInfo [dict get $compMsgMsgDesc msgHeaderInfo]
          set headerIdx 0
          set headerFieldIds [dict get $msgHeaderInfo headerFieldIds]
          while {$headerIdx < [dict get $msgHeaderInfo numHeaderFields]} {
            dict lappend msgDescription fieldSequence [lindex $headerFieldIds $headerIdx]
            dict lappend msgDescription fieldOffsets [dict get $msgDescription lastFieldOffset]
            set fieldId2 [lindex [dict get $msgDescription fieldSequence] end]
            set fieldDescInfo2 [lindex $fieldDescInfos $fieldId2]
            dict incr msgDescription lastFieldOffset [dict get $fieldDescInfo2 fieldLgth]
            dict incr msgDescription numFields
            incr headerIdx
          }
          # add the midpart ids!!
          set msgMidPartInfo [dict get $compMsgMsgDesc msgMidPartInfo]
          set midPartIdx 0
          set midPartFieldIds [dict get $msgMidPartInfo midPartFieldIds]
          while {$midPartIdx < [dict get $msgMidPartInfo numMidPartFields]} {
            set fldId [lindex $midPartFieldIds $midPartIdx]
            set result [::compMsg compMsgTypesAndNames getFieldNameStrFromId compMsgDispatcher $fldId fieldName]
            checkErrOK $result
puts stderr "fldId: $fldId!$fieldName!"
            if {$fieldName eq "@cmdLgth"} {
              if {[lsearch [dict get $msgDescription fieldFlags] COMP_MSG_HAS_CMD_LGTH] >= 0} {
                dict lappend msgDescription fieldSequence $fldId
                dict lappend msgDescription fieldOffsets [dict get $msgDescription lastFieldOffset]
                set fieldId2 [lindex [dict get $msgDescription fieldSequence] end]
                set fieldDescInfo2 [lindex $fieldDescInfos $fieldId2]
                dict incr msgDescription lastFieldOffset [dict get $fieldDescInfo2 fieldLgth]
                dict incr msgDescription numFields
              }
            } else {
              dict lappend msgDescription fieldSequence $fldId
              dict lappend msgDescription fieldOffsets [dict get $msgDescription lastFieldOffset]
              set fieldId2 [lindex [dict get $msgDescription fieldSequence] end]
              set fieldDescInfo2 [lindex $fieldDescInfos $fieldId2]
              dict incr msgDescription lastFieldOffset [dict get $fieldDescInfo2 fieldLgth]
              dict incr msgDescription numFields
            }
            incr midPartIdx
          }
        }
        dict lappend msgDescription fieldSequence [dict get $fieldDescInfo fieldNameId]
        dict lappend msgDescription fieldOffsets [dict get $msgDescription lastFieldOffset]
        set fieldId2 [lindex [dict get $msgDescription fieldSequence] end]
        set fieldDescInfo2 [lindex $fieldDescInfos $fieldId2]
        dict incr msgDescription lastFieldOffset [dict get $fieldDescInfo2 fieldLgth]
        dict incr msgDescription numFields
        if {[dict get $compMsgMsgDesc expectedLines] == [dict get $compMsgMsgDesc currLineNo]} {
          # add the trailer info
          set msgTrailerInfo [dict get $compMsgMsgDesc msgTrailerInfo]
          set trailerIdx 0
          set trailerFieldIds [dict get $msgTrailerInfo trailerFieldIds]
          while {$trailerIdx < [dict get $msgTrailerInfo numTrailerFields]} {
            set fldId [lindex $trailerFieldIds $trailerIdx]
            set result [::compMsg compMsgTypesAndNames getFieldNameStrFromId compMsgDispatcher $fldId fieldName]
            checkErrOK $result
puts stderr "trailer fldId: $fldId!$fieldName!"
            if {$fieldName eq "@crc"} {
              if {[lsearch [dict get $msgDescription fieldFlags] COMP_MSG_HAS_CRC] >= 0} {
                dict lappend msgDescription fieldSequence $fldId
                dict lappend msgDescription fieldOffsets [dict get $msgDescription lastFieldOffset]
                set fieldId2 [lindex [dict get $msgDescription fieldSequence] end]
                set fieldDescInfo2 [lindex $fieldDescInfos $fieldId2]
                dict incr msgDescription lastFieldOffset [dict get $fieldDescInfo2 fieldLgth]
                dict incr msgDescription numFields
              }
            } else {
              if {$fieldName eq "@totalCrc"} {
                if {[lsearch [dict get $msgDescription fieldFlags] COMP_MSG_HAS_TOTAL_CRC] >= 0} {
                  dict lappend msgDescription fieldSequence $fldId
                  dict lappend msgDescription fieldOffsets [dict get $msgDescription lastFieldOffset]
                  set fieldId2 [lindex [dict get $msgDescription fieldSequence] end]
                  set fieldDescInfo2 [lindex $fieldDescInfos $fieldId2]
                  dict incr msgDescription lastFieldOffset [dict get $fieldDescInfo2 fieldLgth]
                  dict incr msgDescription numFields
                }
              } else {
                dict lappend msgDescription fieldSequence $fldId
                dict lappend msgDescription fieldOffsets [dict get $msgDescription lastFieldOffset]
                set fieldId2 [lindex [dict get $msgDescription fieldSequence] end]
                set fieldDescInfo2 [lindex $fieldDescInfos $fieldId2]
                dict incr msgDescription lastFieldOffset [dict get $fieldDescInfo2 fieldLgth]
                dict incr msgDescription numFields
              }
            }
            incr trailerIdx
          }
        }
puts stderr "msgDescription2: $msgDescription!"
        set msgDescriptions [lreplace $msgDescriptions $descIdx $descIdx $msgDescription]
        dict set msgDescriptionInfos msgDescriptions $msgDescriptions
        dict set compMsgMsgDesc msgDescriptionInfos $msgDescriptionInfos
        dict set compMsgDispatcher compMsgMsgDesc $compMsgMsgDesc
        set compMsgMsgDesc [dict get $compMsgDispatcher compMsgMsgDesc]
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

#puts stderr "handleMsgValuesLine"
      set result OK
      set compMsgMsgDesc [dict get $compMsgDispatcher compMsgMsgDesc]
      set msgFieldGroupInfos [dict get $compMsgMsgDesc msgFieldGroupInfos]
      set fieldGroupId [dict get $compMsgMsgDesc currFieldGroupId]
      set cmdKey [dict get $compMsgMsgDesc currCmdKey]
      set msgFieldGroupInfo [dict get $msgFieldGroupInfos $fieldGroupId $cmdKey]
      if {[dict get $compMsgMsgDesc numLineFields] < 2} {
        checkErrOK FIELD_DESC_TOO_FEW_FIELDS
      }
#puts stderr "handleMsgValuesLine: [dict get $compMsgMsgDesc lineFields]!"
      set lineFields [dict get $compMsgMsgDesc lineFields]

      #field name
      set token [lindex $lineFields 0]
      set result [::compMsg compMsgTypesAndNames getFieldNameIdFromStr compMsgDispatcher $token fieldNameIdStr $::COMP_MSG_INCR]
puts stderr "fieldNameId: $token!$fieldNameIdStr!"
      if {![string is integer $fieldNameIdStr]} {
        set result [::compMsg compMsgTypesAndNames getSpecialFieldNameIntFromId $fieldNameIdStr fieldNameId]
        checkErrOK $result          
      } else {
        set fieldNameId $fieldNameIdStr
      }
      checkErrOK $result
      # get the fieldDescInfo for the name to eventually set fieldFlags or other info.
      set result [::compMsg compMsgTypesAndNames getMsgFieldDescInfo compMsgDispatcher $fieldNameId fieldDescInfo]
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
          dict set dataValue fieldValueId $fieldValueId
          set result [::compMsg compMsgDataValue addDataValue compMsgDispatcher $dataValue dataValueIdx]
          checkErrOK $result
          dict incr msgFieldGroupInfo numMsgFieldVal
          dict incr msgFieldGroupInfo maxMsgFieldVal
          dict lappend msgFieldGroupInfo msgFieldVals $dataValueIdx
        }
        COMP_MSG_VAL_HEADER_FIELD_GROUP -
        COMP_MSG_VAL_FIELD_GROUP  {
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

    # ================================= handleMsgFieldGroupLine ====================================

    proc handleMsgFieldGroupLine {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher

#puts stderr "handleMsgFieldGroupLine"
      set result OK
      set compMsgMsgDesc [dict get $compMsgDispatcher compMsgMsgDesc]
      set msgFieldGroupInfos [dict get $compMsgMsgDesc msgFieldGroupInfos]
      set fieldGroupId [dict get $compMsgMsgDesc currFieldGroupId]
      set cmdKey [dict get $compMsgMsgDesc currCmdKey]
      set msgFieldGroupInfo [dict get $msgFieldGroupInfos $fieldGroupId $cmdKey]
#puts stderr "handleMsgFieldGroupLine: fieldGroupId: $fieldGroupId cmdkey: $cmdKey!"
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
        COMP_MSG_DESC_FIELD_GROUP {
          set result [handleMsgCommonLine compMsgDispatcher]
          checkErrOK $result
        }
        COMP_MSG_VAL_HEADER_FIELD_GROUP -
        COMP_MSG_VAL_FIELD_GROUP {
          set result [handleMsgValuesLine compMsgDispatcher]
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
      set headerFieldValues [dict get $msgDescription headerFieldValues]
      if {[string length $headerFieldValues] == 0} {
        set headerFieldValues [string repeat " " [dict get $msgHeaderInfo headerLgth]]
      }
      set result [::compMsg dataView setDataViewData $headerFieldValues [dict get $msgHeaderInfo headerLgth]]
      set fieldIdx 1
      set headerFieldIdx 0
      set msgFieldInfos [dict get $compMsgTypesAndNames msgFieldInfos]
      set fieldDescInfos [dict get $msgFieldInfos fieldDescInfos]
      while {$fieldIdx < [dict get $compMsgTypesAndNames numSpecFieldIds]} {
        set fieldDescInfo [lindex $fieldDescInfos $fieldIdx]
        if {$fieldDescInfo eq [list]} {
          incr fieldIdx
          continue
        }
#puts stderr "FLD: fieldIdx: $fieldIdx!fieldDescInfo: $fieldDescInfo!"
        if {[lsearch [dict get $fieldDescInfo fieldFlags] COMP_MSG_FIELD_HEADER] >= 0} {
#puts stderr "handleMsgHeadsLine: fieldIdx: $fieldIdx"
          set result [::compMsg compMsgTypesAndNames getFieldNameStrFromId compsMsgDispatcher $fieldIdx fieldNameStr]
          checkErrOK $result
          set lineFields [dict get $compMsgMsgDesc lineFields]
          set value [lindex $lineFields $headerFieldIdx]
#puts stderr [format "field: %s fieldIdx: $fieldIdx value: %s!" $fieldNameStr $value]
          if {[lsearch [dict get $fieldDescInfo fieldFlags] COMP_MSG_FIELD_HEADER_UNIQUE] >= 0} {
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
#puts stderr [format "value: %s!stringVal: %s" $value $stringVal]
                set result [::compMsg compMsgDataView setIdFieldValue compMsgDispatcher $fieldIdx $stringVal 0]
                checkErrOK $result
              } else {
                set result [getIntFieldValue compMsgDispatcher $value intVal]
                checkErrOK $result
                set result [::compMsg compMsgDataView setIdFieldValue compMsgDispatcher $fieldIdx $intVal 0]
                checkErrOK $result
              }
            }
          }
          incr headerFieldIdx
        }
        incr fieldIdx
      }
      set result [::compMsg dataView getDataViewData headerFieldValues lgth]
      checkErrOK $result
      dict set msgDescription headerFieldValues $headerFieldValues
      set lineFields [dict get $compMsgMsgDesc lineFields]

      # encryption
      set field [lindex $lineFields $headerFieldIdx]
      set field [string trim $field {\"}]
      if {[string length $field] > 1} {
        checkErrOK BAD_ENCRYPTED_VALUE
      }
      dict set msgDescription encrypted [string range $field 0 0]
      incr headerFieldIdx

      # handle type
      set field [lindex $lineFields $headerFieldIdx]
      set field [string trim $field {\"}]
      if {[string length $field] > 1} {
        checkErrOK BAD_HANDLE_TYPE_VALUE
      }
      dict set msgDescription handleType [string range $field 0 0]
      incr headerFieldIdx

      # cmdKey
      set field [lindex $lineFields $headerFieldIdx]
      set convert false
      if {[string range $field 0 0] eq "\""} {
        set convert true
        set field [string trim $field {\"}]
      }
      if {$convert || ![string is integer $field]} {
        binary scan $field S fieldHex
        set field $fieldHex
      }
      dict set msgDescription cmdKey $field
      incr headerFieldIdx

      # has cmdLgth
      set field [lindex $lineFields $headerFieldIdx]
      if {$field == 1} {
        dict lappend msgDescription fieldFlags COMP_MSG_HAS_CMD_LGTH
      }
      incr headerFieldIdx

      # has crc
      set field [lindex $lineFields $headerFieldIdx]
      if {$field == 1} {
        dict lappend msgDescription fieldFlags COMP_MSG_HAS_CRC
      }
      incr headerFieldIdx

      # has totalCrc
      set field [lindex $lineFields $headerFieldIdx]
      if {$field == 1} {
        dict lappend msgDescription fieldFlags COMP_MSG_HAS_TOTAL_CRC
      }
      incr headerFieldIdx

puts stderr [format "msgDescription: headerLgth: %d encrypted: %s handleType: %s cmdKey: 0x%04x fieldFlags: %s" [dict get $msgHeaderInfo headerLgth] [dict get $msgDescription encrypted] [dict get $msgDescription handleType] [dict get $msgDescription cmdKey] [dict get $msgDescription fieldFlags]]
      set msgDescriptions [lreplace $msgDescriptions $descIdx $descIdx $msgDescription]
      dict set msgDescriptionInfos msgDescriptions $msgDescriptions
      dict incr msgDescriptionInfos currMsgDescriptionIdx
      dict set compMsgMsgDesc msgDescriptionInfos $msgDescriptionInfos
      dict set compMsgDispatcher compMsgMsgDesc $compMsgMsgDesc
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
      
      
    # ================================= getMsgDescriptionFromUniqueFields ====================================
    
    proc getMsgDescriptionFromUniqueFields {compMsgDispatcherVar headerValueInfos msgDescriptionVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $msgDescriptionVar msgDescription

puts stderr "getHeaderFromUniqueFields"
      set compMsgMsgDesc [dict get $compMsgDispatcher compMsgMsgDesc]
      set msgDescriptionInfos [dict get $compMsgMsgDesc msgDescriptionInfos]
      set compMsgTypesAndNames [dict get $compMsgDispatcher compMsgTypesAndNames]
      set msgDescriptions [dict get $msgDescriptionInfos msgDescriptions]
puts stderr "headerValueInfos!$headerValueInfos!"
      set msgFieldInfos [dict get $compMsgTypesAndNames msgFieldInfos]
      set fieldDescInfos [dict get $msgFieldInfos fieldDescInfos]
      set idx 0
      while {$idx < [dict get $msgDescriptionInfos numMsgDescriptions]} {
        set valIdx 0
        set msgDescription [lindex $msgDescriptions $idx]
        set headerFieldValues [dict get $msgDescription headerFieldValues]
        ::compMsg dataView setDataViewData $headerFieldValues [dict get $compMsgDispatcher compMsgMsgDesc msgHeaderInfo headerLgth]
        set found true
        foreach entry $headerValueInfos {
          foreach {fieldName fieldValue} $entry break
          set result [::compMsg compMsgTypesAndNames getFieldNameIdFromStr compMsgDispatcher $fieldName fieldNameIdStr $::COMP_MSG_NO_INCR]
          checkErrOK $result
          if {![string is integer $fieldNameIdStr]} {
            set result [::compMsg compMsgTypesAndNames getSpecialFieldNameIntFromId $fieldNameIdStr fieldNameId]
            checkErrOK $result          
          } else {
            set fieldNameId $fieldNameIdStr
          }
puts stderr "fieldName: $fieldName!fieldNameId: $fieldNameId!"
#puts stderr "fieldName: $fieldName fieldNameId: $fieldNameId msgDescription: $msgDescription!"
          set fieldDescInfo [lindex $fieldDescInfos $fieldNameId]
#puts stderr "fieldDescInfo: $fieldDescInfo!"
          if {[lsearch [dict get $fieldDescInfo fieldFlags] COMP_MSG_FIELD_HEADER_UNIQUE] >= 0} {
            set result [::compMsg compMsgDataView getIdFieldValue compMsgDispatcher $fieldNameId checkValue 0]
            checkErrOK $result
#puts stderr "FLD: $fieldName fieldNameId: $fieldNameId!fieldDescInfo: $fieldDescInfo!checkValue: $checkValue!fieldValue: $fieldValue!"
#puts stderr "FLD: $fieldName fieldNameId: $fieldNameId!checkValue: $checkValue!fieldValue: $fieldValue!"
            if {$checkValue != $fieldValue} {
              set found false
              break
            }
          } else {
            # check for cmdKey
            if {$fieldName eq "@cmdKey"} {
              set checkValue [dict get $msgDescription cmdKey]
              checkErrOK $result
              if {$checkValue != $fieldValue} {
                set found false
                break
              }
puts stderr "fieldNameId: $fieldNameId!$result!$checkValue!$fieldValue!"
            }
          }
          incr valIdx
        }
        if {$found} {
puts stderr "found: $idx!msgDescription: $msgDescription!"
          return [checkErrOK OK]
        }
        incr idx
      }
      checkErrOK HEADER_NOT_FOUND
    }

    # ================================= getMsgPartsFromHeaderPart ====================================
    
    proc getMsgPartsFromHeaderPart {compMsgDispatcherVar msgDescription} {
      upvar $compMsgDispatcherVar compMsgDispatcher
    
puts stderr "getMsgPartsFromHeaderPart!"
      dict set compMsgDispatcher compMsgData [list]
if {0} {
      dict set compMsgDispatcher currHdr $hdr
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
}
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

      set msgMidPartInfo [dict create]
      dict set msgMidPartInfo midPartLgth 0
      dict set msgMidPartInfo numMidPartFields 0
      dict set msgMidPartInfo midPartFieldIds [list]
      dict set compMsgMsgDesc msgMidPartInfo $msgMidPartInfo

      set msgTrailerInfo [dict create]
      dict set msgTrailerInfo trailerLgth 0
      dict set msgTrailerInfo numTrailerFields 0
      dict set msgTrailerInfo midTrailerFieldIds [list]
      dict set compMsgMsgDesc msgTrailerInfo $msgTrailerInfo

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
#puts stderr "================="
#set val [::dict2json $compMsgDispatcher]
#puts $val
      return [checkErrOK OK]
    }

  } ; # namespace compMsgMsgDesc
}  ; # namespace compMsg
