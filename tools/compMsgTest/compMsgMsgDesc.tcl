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

set ::COMP_MSG_DESC_ERR_OK                    0
set ::COMP_MSG_DESC_ERR_VALUE_NOT_SET         255
set ::COMP_MSG_DESC_ERR_VALUE_OUT_OF_RANGE    254
set ::COMP_MSG_DESC_ERR_BAD_VALUE             253
set ::COMP_MSG_DESC_ERR_BAD_FIELD_TYPE        252
set ::COMP_MSG_DESC_ERR_FIELD_TYPE_NOT_FOUND  251
set ::COMP_MSG_DESC_ERR_VALUE_TOO_BIG         250
set ::COMP_MSG_DESC_ERR_OUT_OF_MEMORY         249
set ::COMP_MSG_DESC_ERR_OUT_OF_RANGE          248

  # be carefull the values up to here
  # must correspond to the values in dataView.h !!!
  # with the names like DATA_VIEW_ERR_*

set ::COMP_MSG_DESC_ERR_OPEN_FILE             189
set ::COMP_MSG_DESC_FILE_NOT_OPENED           188
set ::COMP_MSG_DESC_ERR_FLUSH_FILE            187
set ::COMP_MSG_DESC_ERR_WRITE_FILE            186
set ::COMP_MSG_DESC_ERR_FUNNY_EXTRA_FIELDS    185
set ::COMP_MSG_DESC_ERR_FIELD_TOO_LONG        184

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
set ::HDR_FILLER_LGTH 40

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
#   headerFlags         # these are the flags for the 2nd line in the heads file!!
#   headerSequence   # this is the sequence of the 2nd line in the heads file!!
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

set ::moduleFilesPath $::env(HOME)/bene-nodemcu-firmware/module_image_files

namespace eval compMsg {
  namespace ensemble create

    namespace export compMsgMsgDesc

  namespace eval compMsgMsgDesc {
    namespace ensemble create
      
    namespace export readHeadersAndSetFlags dumpHeaderPart getHeaderFromUniqueFields
    namespace export getMsgPartsFromHeaderPart getWifiKeyValueKeys readActions
    namespace export resetMsgDescPart resetMsgValPart dumpMsgDescPart dumpMsgValPart
    namespace export getMsgKeyValueDescParts

    variable headerInfos [list]
    variable received [list]
    variable dispFlags [list]

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
      return $::COMP_DISP_ERR_OK
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
      return $::COMP_DISP_ERR_OK
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
          checkErrOK $::COMP_MSG_ERR_NO_SUCH_FIELD
        }
        set result [::compMsg compMsgDataView getFieldNameIdFromStr $fieldName fieldNameId $::COMP_MSG_NO_INCR]
        checkErrOK $result
        switch $fieldNameId {
          COMP_MSG_SPEC_FIELD_SRC {
            if {[lsearch [dict get $headerInfos headerFlags] COMP_DISP_HDR_SRC] >= 0} {
              return $::COMP_MSG_ERR_DUPLICATE_FIELD
            }
            dict lappend headerInfos headerSequence COMP_DISP_U16_SRC
            incr seqIdx
            dict lappend headerInfos headerFlags COMP_DISP_HDR_SRC
          }
          COMP_MSG_SPEC_FIELD_DST {
            if {[lsearch [dict get $headerInfos headerFlags] COMP_DISP_HDR_DST] >= 0} {
              return $::COMP_MSG_ERR_DUPLICATE_FIELD
            }
            dict lappend headerInfos headerSequence COMP_DISP_U16_DST
            incr seqIdx
            dict lappend headerInfos headerFlags COMP_DISP_HDR_DST
          }
          COMP_MSG_SPEC_FIELD_TOTAL_LGTH {
            if {[lsearch [dict get $headerInfos headerFlags] COMP_DISP_HDR_TOTAL_LGTH] >= 0} {
              return $::COMP_MSG_ERR_DUPLICATE_FIELD
            }
            dict lappend headerInfos headerSequence COMP_DISP_U16_TOTAL_LGTH
            incr seqIdx
            dict lappend headerInfos headerFlags COMP_DISP_HDR_TOTAL_LGTH
          }
          COMP_MSG_SPEC_FIELD_SRC_ID {
            if {[lsearch [dict get $headerInfos headerFlags] COMP_DISP_HDR_SRC_ID] >= 0} {
              return $::COMP_MSG_ERR_DUPLICATE_FIELD
            }
            dict lappend headerInfos headerSequence COMP_DISP_U16_SRC_ID
            incr seqIdx
            dict lappend headerInfos headerFlags COMP_DISP_HDR_SRC_ID
          }
          COMP_MSG_SPEC_FIELD_GUID {
            if {[lsearch [dict get $headerInfos headerFlags] COMP_DISP_HDR_GUID] >= 0} {
              return $::COMP_MSG_ERR_DUPLICATE_FIELD
            }
            dict lappend headerInfos headerSequence COMP_DISP_U8_VECTOR_GUID
            incr seqIdx
            dict lappend headerInfos headerFlags COMP_DISP_HDR_GUID
          }
          COMP_MSG_SPEC_FIELD_HDR_FILLER {
            if {[lsearch [dict get $headerInfos headerFlags] COMP_DISP_HDR_FILLER] >= 0} {
              return $::COMP_MSG_ERR_DUPLICATE_FIELD
            }
            dict lappend headerInfos headerSequence COMP_DISP_U8_VECTOR_HDR_FILLER
            incr seqIdx
            dict lappend headerInfos headerFlags COMP_DISP_HDR_FILLER
          }
          default {
            checkErrOK $::COMP_MSG_ERR_NO_SUCH_FIELD
          }
        }
        incr fieldIdx
      }
      dict set headerInfos seqIdxAfterStart $seqIdx
      return $::COMP_MSG_ERR_OK
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
          checkErrOK $::COMP_MSG_ERR_TOO_FEW_FILE_LINES
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
              checkErrOK $::COMP_MSG_ERR_FIELD_NOT_FOUND
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
           checkErrOK $::COMP_MSG_ERR_BAD_FIELD_TYPE
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
            checkErrOK $::COMP_MSG_ERR_BAD_FIELD_TYPE
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
              checkErrOK $::COMP_MSG_ERR_BAD_FIELD_TYPE
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
              checkErrOK $::COMP_MSG_ERR_BAD_FIELD_TYPE
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
      return $::COMP_MSG_ERR_OK
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
      return $::COMP_MSG_DESC_ERR_OK
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
               return $::COMP_MSG_ERR_OK
            }
          }
        }
        incr idx
      }
      checkErrOK $::COMP_DISP_ERR_HEADER_NOT_FOUND
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
      return $::COMP_DISP_ERR_OK
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
      return $::COMP_DISP_ERR_OK
    }

    # ================================= resetMsgDescParts ====================================

    proc resetMsgDescParts {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher

      dict set compMsgMsgDesc msgDescParts [list]
      dict set compMsgMsgDesc numMsgDescParts 0
      dict set compMsgMsgDesc maxMsgDescParts 0
      return $COMP_DISP_ERR_OK
    }

    # ================================= resetMsgValParts ====================================

    proc resetMsgValParts {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher

      dict set compMsgMsgDesc msgValParts [list]
      dict set compMsgMsgDesc numMsgValParts 0
      dict set compMsgMsgDesc maxMsgValParts 0
      return $::COMP_DISP_ERR_OK
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
          checkErrOK $::COMP_DISP_ERR_TOO_FEW_FILE_LINES
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
        set result [::compMsg compMsgDataView getFieldNameIdFromStr $fieldNameStr fieldNameId $::COMP_MSG_INCR]
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
          checkErrOK $::COMP_DISP_ERR_TOO_FEW_FILE_LINES
        }
        set flds [split $line ","]
        set callback [list]
        foreach {fieldNameStr fieldValueStr} $flds break
#puts stderr "val fieldName: $fieldNameStr!"
        # fieldName
        set result [::compMsg compMsgDataView getFieldNameIdFromStr $fieldNameStr fieldNameId $::COMP_MSG_NO_INCR]
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
      return $::COMP_MSG_DESC_ERR_OK
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
      return $::COMP_MSG_ERR_OK
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
      return $::COMP_MSG_ERR_OK
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
      return $::COMP_MSG_ERR_OK
    }

  } ; # namespace compMsgMsgDesc
}  ; # namespace compMsg
