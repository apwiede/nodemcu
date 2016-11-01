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
#   headerFlags        ; # these are the flags for the 2nd line in the heads file!!
#   headerSequence  ; # this is the sequence of the 2nd line in the heads file!!
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
#   getFieldSizeCallback

# msgValPart dict
#   fieldNameStr
#   fieldNameId
#   fieldValueStr    ; # the value or the callback for getting the value
#   fieldKeyValueStr ; # the value for a string
#   fieldValue       ; # the value for an integer
#   fieldFlags
#   getFieldValueCallback

set ::moduleFilesPath $::env(HOME)/bene-nodemcu-firmware/module_image_files

namespace eval compMsg {
  namespace ensemble create

    namespace export compMsgMsgDesc

  namespace eval compMsgMsgDesc {
    namespace ensemble create
      
    namespace export readHeadersAndSetFlags dumpHeaderPart getHeaderFromUniqueFields
    namespace export createMsgFromHeaderPart
    namespace export resetMsgDescPart resetMsgValPart dumpMsgDescPart dumpMsgValPart

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
      if {![dict exists $hdr hdrTargetPart]} {
        dict set hdr hdrTargetPart 0
      }
      if {![dict exists $hdr fieldSequence]} {
        dict set hdr fieldSequence [list]
      }
      puts stderr [format "headerParts1: from: 0x%04x to: 0x%04x totalLgth: %d u16CmdKey: %s u16CmdLgth: 0x%04x u16Crc: 0x%04x" [dict get $hdr hdrFromPart] [dict get $hdr hdrToPart] [dict get $hdr hdrTotalLgth] [dict get $hdr hdrU16CmdKey] [dict get $hdr hdrU16CmdLgth] [dict get $hdr hdrU16Crc]]
      puts stderr [format "headerParts2: target: 0x%02x u8CmdKey: %s u8CmdLgth: %d u8Crc: 0x%02x offset: %d extra: %d" [dict get $hdr hdrTargetPart] [dict get $hdr hdrU8CmdKey] [dict get $hdr hdrU8CmdLgth] [dict get $hdr hdrU8Crc] [dict get $hdr hdrOffset] [dict get $hdr hdrExtraLgth]]
      puts stderr [format "headerParts3: enc: %s handleType: %s" [dict get $hdr hdrEncryption] [dict get $hdr hdrHandleType]]
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
      set fieldIdx 0
      dict set headerInfos headerLgth [lindex $flds $fieldIdx]
      incr fieldIdx
      set fieldName [lindex $flds $fieldIdx]
      if {[string range $fieldName 0 0] ne "@"} {
        return $::COMP_MSG_ERR_NO_SUCH_FIELD
      }
      set result [::compMsg compMsgDataView getFieldNameIdFromStr $fieldName fieldNameId $::COMP_MSG_NO_INCR]
      if {$result != $::COMP_MSG_ERR_OK} {
        return $result
      }
      switch $fieldNameId {
        COMP_MSG_SPEC_FIELD_SRC {
          dict lappend headerInfos headerSequence COMP_DISP_U16_SRC
          incr seqIdx
          dict lappend headerInfos headerFlags COMP_DISP_U16_SRC
        }
        COMP_MSG_SPEC_FIELD_DST {
          dict lappend headerInfos headerSequence COMP_DISP_U16_DST
          incr seqIdx
          dict lappend headerInfos headerFlags COMP_DISP_U16_DST
        }
        COMP_MSG_SPEC_FIELD_TARGET_CMD {
          dict lappend headerInfos headerSequence COMP_DISP_U8_TARGET
          incr seqIdx
          dict lappend headerInfos headerFlags COMP_DISP_U8_TARGET
        }
        default {
          return $::COMP_MSG_ERR_NO_SUCH_FIELD
        }
      }
      incr fieldIdx

      set fieldName [lindex $flds $fieldIdx]
      if {[string range $fieldName 0 0] ne "@"} {
        return $::COMP_MSG_ERR_NO_SUCH_FIELD
      }
      set result [::compMsg compMsgDataView getFieldNameIdFromStr $fieldName fieldNameId $::COMP_MSG_NO_INCR]
      if {$result != $::COMP_MSG_ERR_OK} {
        return $result
      }
      switch $fieldNameId {
        COMP_MSG_SPEC_FIELD_SRC {
          if {[lsearch [dict get $headerInfos headerFlags] COMP_DISP_U16_SRC] >= 0} {
            return $::COMP_MSG_ERR_DUPLICATE_FIELD
          }
          dict lappend headerInfos headerSequence COMP_DISP_U16_SRC
          incr seqIdx
          dict lappend headerInfos headerFlags COMP_DISP_U16_SRC
        }
        COMP_MSG_SPEC_FIELD_DST {
          if {[lsearch [dict get $headerInfos headerFlags] COMP_DISP_U16_DST] >= 0} {
            return $::COMP_MSG_ERR_DUPLICATE_FIELD
          }
          dict lappend headerInfos headerSequence COMP_DISP_U16_DST
          incr seqIdx
          dict lappend headerInfos headerFlags COMP_DISP_U16_DST
        }
        COMP_MSG_SPEC_FIELD_TOTAL_LGTH {
          dict lappend headerInfos headerSequence COMP_DISP_U16_TOTAL_LGTH
          incr seqIdx
          dict lappend headerInfos headerFlags COMP_DISP_U16_TOTAL_LGTH
        }
        default {
           return $::COMP_MSG_ERR_NO_SUCH_FIELD
        }
      }
      incr fieldIdx

      if {[llength $flds] > $fieldIdx} {
        set fieldName [lindex $flds $fieldIdx]
        if {[string range $fieldName 0 0] ne "@"} {
          return $::COMP_MSG_ERR_NO_SUCH_FIELD
        }
        set result [::compMsg compMsgDataView getFieldNameIdFromStr $fieldName fieldNameId $::COMP_MSG_NO_INCR]
        if {$result != $::COMP_MSG_ERR_OK} {
          return $result
        }
        switch $fieldNameId {
          COMP_MSG_SPEC_FIELD_TOTAL_LGTH {
            if {[lsearch [dict get $headerInfos headerFlags] COMP_DISP_U16_TOTAL_LGTH] >= 0} {
              return $::COMP_MSG_ERR_DUPLICATE_FIELD
            }
            dict lappend headerInfos headerSequence COMP_DISP_U16_TOTAL_LGTH
            incr seqIdx
            dict lappend headerInfos headerFlags COMP_DISP_U16_TOTAL_LGTH
          }
          default {
            return $::COMP_MSG_ERR_NO_SUCH_FIELD
          }
        }
      }
      incr fieldIdx
      if {[llength $flds] > $fieldIdx} {
        set fieldName [lindex $flds $fieldIdx]
        if {[string range $fieldName 0 0] ne "@"} {
          return $::COMP_MSG_ERR_NO_SUCH_FIELD
        }
        set result [::compMsg compMsgDataView getFieldNameIdFromStr $fieldName fieldNameId $::COMP_MSG_NO_INCR]
        if {$result != $::COMP_MSG_ERR_OK} {
          return $result
        }
        switch $fieldNameId {
          COMP_MSG_SPEC_FIELD_GUID {
            if {[lsearch [dict get $headerInfos headerFlags] COMP_DISP_U8_VECTOR_GUID] >= 0} {
              return $::COMP_MSG_ERR_DUPLICATE_FIELD
            }
            dict lappend headerInfos headerSequence COMP_DISP_U8_VECTOR_GUID
            incr seqIdx
            dict lappend headerInfos headerFlags COMP_DISP_U8_VECTOR_GUID
          }
          default {
            return $::COMP_MSG_ERR_NO_SUCH_FIELD
          }
        }
      }
      incr fieldIdx
      if {[llength $flds] > $fieldIdx} {
        set fieldName [lindex $flds $fieldIdx]
        if {[string range $fieldName 0 0] ne "@"} {
          return $::COMP_MSG_ERR_NO_SUCH_FIELD
        }
        set result [::compMsg compMsgDataView getFieldNameIdFromStr $fieldName fieldNameId $::COMP_MSG_NO_INCR]
        if {$result != $::COMP_MSG_ERR_OK} {
          return $result
        }
        switch $fieldNameId {
          COMP_MSG_SPEC_FIELD_SRC_ID {
            if {[lsearch [dict get $headerInfos headerFlags] COMP_DISP_U16_SRC_ID] >= 0} {
              return $::COMP_MSG_ERR_DUPLICATE_FIELD
            }
            dict lappend headerInfos headerSequence COMP_DISP_U16_SRC_ID
            incr seqIdx
            dict lappend headerInfos headerFlags COMP_DISP_U16_SRC_ID
          }
          default {
            return $::COMP_MSG_ERR_NO_SUCH_FIELD
          }
        }
      }
      incr fieldIdx
      if {[llength $flds] > $fieldIdx} {
        set fieldName [lindex $flds $fieldIdx]
        if {[string range $fieldName 0 0] ne "@"} {
          return $::COMP_MSG_ERR_NO_SUCH_FIELD
        }
        set result [::compMsg compMsgDataView getFieldNameIdFromStr $fieldName fieldNameId $::COMP_MSG_NO_INCR]
        if {$result != $::COMP_MSG_ERR_OK} {
          return $result
        }
        switch $fieldNameId {
          COMP_MSG_SPEC_FIELD_HDR_FILLER {
            if {[lsearch [dict get $headerInfos headerFlags] COMP_DISP_U8_VECTOR_HDR_FILLER] >= 0} {
              return $::COMP_MSG_ERR_DUPLICATE_FIELD
            }
            dict lappend headerInfos headerSequence COMP_DISP_U8_VECTOR_HDR_FILLER
            incr seqIdx
            dict lappend headerInfos headerFlags COMP_DISP_U8_VECTOR_HDR_FILLER
          }
          default {
            return $::COMP_MSG_ERR_NO_SUCH_FIELD
          }
        }
      }
      dict set headerInfos seqIdxAfterStart $seqIdx
      return $::COMP_MSG_ERR_OK
    }
      
    # ================================= readHeadersAndSetFlags ====================================
    
    proc readHeadersAndSetFlags {fileName} {
      variable headerInfos
      variable dispFlags
    
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
          return $::COMP_MSG_ERR_TOO_FEW_FILE_LINES
        }
        set hdr [dict create]
        set seqIdx2 0
        set fieldSequence [list]
        set headerSequence [dict get $headerInfos headerSequence]
        while {$seqIdx2 < $seqStartIdx} {
          lappend fieldSequence [lindex $headerSequence $seqIdx2]
          incr seqIdx2
        }
        dict set hdr fieldSequence $fieldSequence
        dict set hdr hdrFlags [list]
        set flds [split $line ,]
        set seqIdx2 0
        set found false
        set myPart [lindex $flds $seqIdx2]
        if {[lindex $headerSequence $seqIdx2] eq "COMP_DISP_U16_SRC"} {
          dict set hdr hdrFromPart $myPart
          set found true
        }
        if {[lindex $headerSequence $seqIdx2] eq "COMP_DISP_U16_DST"} {
          dict set hdr hdrToPart $myPart
          set found true
        }
        if {[lindex $headerSequence $seqIdx2] eq "COMP_DISP_U8_TARGET"} {
          dict set hdr hdrTargetPart $myPart
          set found true
        }
        if {!$found} {
          return $::COMP_MSG_ERR_NO_SUCH_FIELD
        }
        incr seqIdx2
        set myPart [lindex $flds $seqIdx2]
        set found false
        if {[lindex $headerSequence $seqIdx2] eq "COMP_DISP_U16_SRC"} {
          dict set hdr hdrFromPart $myPart
          set found true
        }
        if {[lindex $headerSequence $seqIdx2] eq "COMP_DISP_U16_DST"} {
          dict set hdr hdrToPart $myPart
          set found true
        }
        if {[lindex $headerSequence $seqIdx2] eq "COMP_DISP_U16_TOTAL_LGTH"} {
          dict set hdr hdrTotalLgth $myPart
          set found true
        }
        if {!$found} {
          return $::COMP_MSG_ERR_NO_SUCH_FIELD
        }
        incr seqIdx2
        set myPart [lindex $flds $seqIdx2]
        if {$seqIdx > $seqIdx2} {
          set found false
          if {[lindex $headerSequence $seqIdx2] eq "COMP_DISP_U16_TOTAL_LGTH"} {
            dict set hdr hdrTotalLgth $myPart
            set found true
          }
          if {!$found} {
            return $::COMP_MSG_ERR_NO_SUCH_FIELD
          }
          incr seqIdx2
        }
        set seqIdx3 $seqIdx2
        set myPart [lindex $flds $seqIdx3]
        # extra field lgth 0/<number>
        dict set hdr hdrExtraLgth $myPart
        incr seqIdx3
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
        if {$result != $::COMP_MSG_ERR_OK} {
          return $result
        }
        incr seqIdx3
        set myPart [lindex $flds $seqIdx3]
        # cmdKey
        switch $fieldTypeId {
          DATA_VIEW_FIELD_UINT8_T {
            dict lappend hdr fieldSequence COMP_DISP_U8_CMD_KEY
            dict lappend hdr hdrFlags COMP_DISP_U8_CMD_KEY
            dict set hdr hdrU8CmdKey $myPart
            lappend dispFlags COMP_MSG_U8_CMD_KEY
          }
          DATA_VIEW_FIELD_UINT16_T {
            dict lappend hdr fieldSequence COMP_DISP_U16_CMD_KEY
            dict lappend hdr hdrFlags COMP_DISP_U16_CMD_KEY
            dict set hdr hdrU16CmdKey $myPart
            set myIdx [lsearch $dispFlags COMP_MSG_U8_CMD_KEY]
            if {$myIdx >= 0} {
              set dispFlags [lreplace $dispGFlags $myIdx $myIdx]
            }
          }
          default {
           return $::COMP_MSG_ERR_BAD_FIELD_TYPE
          }
        }
        incr seqIdx3
        set myPart [lindex $flds $seqIdx3]
        # type of cmdLgth
        set result [::compMsg dataView getFieldTypeIdFromStr $myPart fieldTypeId]
        if {$result != $::COMP_MSG_ERR_OK} {
          return $result
        }
        set isEnd false
        if {$seqIdx2 >= [llength $flds]} {
          set isEnd true
        }
        switch $fieldTypeId {
          DATA_VIEW_FIELD_UINT0_T {
            dict lappend hdrs fieldSequence COMP_DISP_U0_CMD_Lgth
            dict lappend hdr hdrFlags COMP_DISP_U0_CMD_Lgth
          }
          DATA_VIEW_FIELD_UINT8_T {
            dict lappend hdr fieldSequence COMP_DISP_U8_CMD_Lgth
            dict lappend hdr hdrFlags COMP_DISP_U8_CMD_Lgth
          }
          DATA_VIEW_FIELD_UINT16_T {
            dict lappend hdr fieldSequence COMP_DISP_U16_CMD_Lgth
            dict lappend hdr hdrFlags COMP_DISP_U16_CMD_Lgth
          }
          default {
            return $::COMP_MSG_ERR_BAD_FIELD_TYPE
          }
        }
        # type of crc
        if {!$isEnd} {
          incr seqIdx3
          set myPart [lindex $flds $seqIdx3]
        }
        set result [::compMsg dataView getFieldTypeIdFromStr $myPart fieldTypeId]
        if {$result != $::COMP_MSG_ERR_OK} {
          return $result
        }
        switch $fieldTypeId {
          DATA_VIEW_FIELD_UINT0_T {
            dict lappend hdr fieldSequence COMP_DISP_U0_CRC
            dict lappend hdr hdrFlags COMP_DISP_U0_CRC
          }
          DATA_VIEW_FIELD_UINT8_T {
            dict lappend hdr fieldSequence COMP_DISP_U8_CRC
            dict lappend hdr hdrFlags COMP_DISP_U8_CRC
          }
          DATA_VIEW_FIELD_UINT16_T {
            dict lappend hdr fieldSequence COMP_DISP_U16_CRC
            dict lappend hdr hdrFlags COMP_DISP_U16_CRC
          }
          default {
            return $::COMP_MSG_ERR_BAD_FIELD_TYPE
          }
        }
        dict lappend headerInfos headerParts $hdr
        dict set headerInfos numHeaderParts [expr {[dict get $headerInfos numHeaderParts] + 1}]
#dumpHeaderPart $hdr
        incr idx
      }
      close $fd
      return $result
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
      return $::COMP_DISP_ERR_HEADER_NOT_FOUND
    }

    # ================================= dumpMsgDescPart ====================================

    proc dumpMsgDescPart {compMsgDispatcherVar msgDescPart} {
      upvar $compMsgDispatcherVar compMsgDispatcher

      set callbackName [list]
      if {[dict get $msgDescPart getFieldSizeCallback] ne [list]} {
        set result [::compMsg compMsgAction getActionCallbackName compMsgDispatcher [dict get $msgDescPart getFieldSizeCallback] callbackName]
        if {$result != $::COMP_DISP_ERR_OK} {
          return $result
        }
      }
      puts stderr [format "msgDescPart: fieldNameStr: %-15.15s fieldNameId: %-35.35s fieldTypeStr: %-10.10s fieldTypeId: %-30.30s field_lgth: %d callback: %s" [dict get $msgDescPart fieldNameStr] [dict get $msgDescPart fieldNameId] [dict get $msgDescPart fieldTypeStr] [dict get $msgDescPart fieldTypeId] [dict get $msgDescPart fieldLgth] $callbackName]
      return $::COMP_DISP_ERR_OK
    }

    # ================================= dumpMsgValPart ====================================

    proc dumpMsgValPart {compMsgDispatcherVar msgValPart} {
      upvar $compMsgDispatcherVar compMsgDispatcher

      set callbackName [list]
      if {[dict get $msgValPart getFieldValueCallback] ne [list]} {
        set result [::compMsg compMsgAction getActionCallbackName compMsgDispatcher [dict get $msgValPart getFieldValueCallback] callbackName]
        if {$result != $::COMP_DISP_ERR_OK} {
          return $result
        }
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
    
    # ================================= createMsgFromHeaderPart ====================================
    
    proc createMsgFromHeaderPart {compMsgDispatcherVar hdr handleVar} {
      variable headerInfos
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $handleVar handle

      dict set compMsgDispatcher currHdr $hdr
      dict set compMsgDispatcher msgDescParts [list]
      dict set compMsgDispatcher msgValParts [list]
      set fileName [format "%s/CompDesc%s.txt" $::moduleFilesPath [dict get $hdr hdrU16CmdKey]]
puts stderr "fileName!$fileName!"
      set fd [open $fileName "r"]
      gets $fd line
      set flds [split $line ","]
      set prepareValuesCbName [list]
      foreach {dummy numEntries prepareValuesCbName} $flds break
puts stderr "numDesc!$numEntries!$prepareValuesCbName!"
      if {$prepareValuesCbName != [list]} {
        dict set compMsgDispatcher prepareValuesCbName $prepareValuesCbName
      }
      set numRows 0
      set idx 0
      while {$idx < $numEntries} {
        gets $fd line
        if {$line eq ""} {
          return $::COMP_DISP_ERR_TOO_FEW_FILE_LINES
        }
        set flds [split $line ","]
        set callback [list]
        foreach {fieldNameStr fieldTypeStr fieldLgthStr callback} $flds break
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
        dict set msgDescPart getFieldSizeCallback $callback
        dict set msgDescPart fieldNameStr $fieldNameStr
        dict set msgDescPart fieldTypeStr $fieldTypeStr
        dict set msgDescPart fieldLgth $fieldLgth
        set result [::compMsg dataView getFieldTypeIdFromStr $fieldTypeStr fieldTypeId]
        if {$result != $::COMP_MSG_ERR_OK} {
          return $result
        }
        dict set msgDescPart fieldTypeId $fieldTypeId
        set result [::compMsg compMsgDataView getFieldNameIdFromStr $fieldNameStr fieldNameId $::COMP_MSG_INCR]
        if {$result != $::COMP_MSG_ERR_OK} {
          return $result
        }
        dict set msgDescPart fieldNameId $fieldNameId
#dumpMsgDescPart compMsgDispatcher $msgDescPart
        dict lappend compMsgDispatcher msgDescParts $msgDescPart
        incr idx
      }
      close $fd
      set fileName [format "%s/CompVal%s.txt" $::moduleFilesPath [dict get $hdr hdrU16CmdKey]]
      set fd [open $fileName "r"]
      gets $fd line
      set flds [split $line ","]
      set callback [list]
      foreach {dummy numEntries callback} $flds break
      set numRows 0
puts stderr "numVal: $numEntries!$callback!"
      set idx 0
      while {$idx < $numEntries} {
        gets $fd line
        if {$line eq ""} {
          return $::COMP_DISP_ERR_TOO_FEW_FILE_LINES
        }
        set flds [split $line ","]
        set callback [list]
        foreach {fieldNameStr fieldValueStr} $flds break
        # fieldName
        set result [::compMsg compMsgDataView getFieldNameIdFromStr $fieldNameStr fieldNameId $::COMP_MSG_NO_INCR]
        if {$result != $::COMP_MSG_ERR_OK} {
          return $result
        }
        set callback [list]
        if {[string range $fieldValueStr 0 0] eq "@"} {
          set callback [string range $fieldValueStr 1 end]
        }
    
        set msgValPart [dict create]
        dict set msgValPart fieldNameId $fieldNameId
        dict set msgValPart fieldFlags [list]
        dict set msgValPart fieldKeyValueStr [list]
        dict set msgValPart fieldValue [list]
        dict set msgValPart getFieldValueCallback $callback
        dict set msgValPart fieldNameStr $fieldNameStr
        dict set msgValPart fieldValueStr $fieldValueStr
        dict lappend compMsgDispatcher msgValParts $msgValPart
#dumpMsgValPart compMsgDispatcher $msgValPart
        incr idx
      }
      close $fd
      return $::COMP_MSG_ERR_OK
    }

  } ; # namespace compMsgMsgDesc
} ; # namespace compMsg

