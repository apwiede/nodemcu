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

set ::STRUCT_DISP_ERR_OPEN_FILE             189
set ::STRUCT_DISP_FILE_NOT_OPENED           188
set ::STRUCT_DISP_ERR_FLUSH_FILE            187
set ::STRUCT_DISP_ERR_WRITE_FILE            186
set ::STRUCT_DISP_ERR_BAD_RECEIVED_LGTH     185
set ::STRUCT_DISP_ERR_BAD_FILE_CONTENTS     184
set ::STRUCT_DISP_ERR_HEADER_NOT_FOUND      183
set ::STRUCT_DISP_ERR_DUPLICATE_FIELD       182
set ::STRUCT_DISP_ERR_BAD_FIELD_NAME        181
set ::STRUCT_DISP_ERR_BAD_HANDLE_TYPE       180
set ::STRUCT_DISP_ERR_INVALID_BASE64_STRING 179
set ::STRUCT_DISP_ERR_TOO_FEW_FILE_LINES    178
set ::STRUCT_DISP_ERR_ACTION_NAME_NOT_FOUND 177
set ::STRUCT_DISP_ERR_DUPLICATE_ENTRY       176


set RECEIVED_CHECK_HEADER_SIZE 7

set ::DISP_FLAG_SHORT_CMD_KEY    1
set ::DISP_FLAG_HAVE_CMD_LGTH    2
set ::DISP_FLAG_IS_ENCRYPTED     4
set ::DISP_FLAG_IS_TO_WIFI_MSG   8
set ::DISP_FLAG_IS_FROM_MCU_MSG  16

set ::RECEIVED_CHECK_TO_SIZE            2
set ::RECEIVED_CHECK_FROM_SIZE          4
set ::RECEIVED_CHECK_TOTAL_LGTH_SIZE    6
set ::RECEIVED_CHECK_SHORT_CMD_KEY_SIZE 7
set ::RECEIVED_CHECK_CMD_KEY_SIZE       8
set ::RECEIVED_CHECK_CMD_LGTH_SIZE      10

if {0} {
set flagId2Strs [dict create]
  { STRUCT_DISP_U16_DST,           "STRUCT_DISP_U16_DST" },
  { STRUCT_DISP_U16_SRC,           "STRUCT_DISP_U16_SRC" },
  { STRUCT_DISP_U8_TARGET,         "STRUCT_DISP_U8_TARGET" },
  { STRUCT_DISP_U16_TOTAL_LGTH,    "STRUCT_DISP_U16_TOTAL_LGTH" },
  { STRUCT_DISP_U8_EXTRA_KEY_LGTH, "STRUCT_DISP_U8_EXTRA_KEY_LGTH" },
  { STRUCT_DISP_IS_ENCRYPTED,      "STRUCT_DISP_IS_ENCRYPTED" },
  { STRUCT_DISP_IS_NOT_ENCRYPTED,  "STRUCT_DISP_IS_NOT_ENCRYPTED" },
  { STRUCT_DISP_SEND_TO_APP,       "STRUCT_DISP_SEND_TO_APP" },
  { STRUCT_DISP_RECEIVE_FROM_APP,  "STRUCT_DISP_RECEIVE_FROM_APP" },
  { STRUCT_DISP_SEND_TO_UART,      "STRUCT_DISP_SEND_TO_UART" },
  { STRUCT_DISP_RECEIVE_FROM_UART, "STRUCT_DISP_RECEIVE_FROM_UART" },
  { STRUCT_DISP_TRANSFER_TO_UART,  "STRUCT_DISP_TRANSFER_TO_UART" },
  { STRUCT_DISP_TRANSFER_TO_CONN,  "STRUCT_DISP_TRANSFER_TO_CONN" },
  { STRUCT_DISP_U8_CMD_KEY,        "STRUCT_DISP_U8_CMD_KEY" },
  { STRUCT_DISP_U16_CMD_KEY,       "STRUCT_DISP_U16_CMD_KEY" },
  { STRUCT_DISP_U0_CMD_LGTH,       "STRUCT_DISP_U0_CMD_LGTH" },
  { STRUCT_DISP_U8_CMD_LGTH,       "STRUCT_DISP_U8_CMD_LGTH" },
  { STRUCT_DISP_U16_CMD_LGTH,      "STRUCT_DISP_U16_CMD_LGTH" },

set flagStr2Ids [dict create]
}

set ::moduleFilesPath $::env(HOME)/bene-nodemcu-firmware/module_image_files

namespace eval structmsg {
  namespace ensemble create

    namespace export structmsgIdentify

  namespace eval structmsgIdentify {
    namespace ensemble create
      
    namespace export structmsgIdentify freeStructmsgDataView
    variable hdrInfos [list]
    variable received [list]
    variable dispFlags

    set received [dict create]
    dict set received buf ""
    dict set received lgth 0


    # ================================= getIntFromLine ====================================
    
    proc getIntFromLine {myStr ulgth ep} {
      uint8_t *cp;
      char *endPtr;
    
      cp = myStr;
      while {*cp != ','} {
        cp++;
      }
      *cp++ = '\0';
      *ulgth = c_strtoul{myStr, &endPtr, 10};
      if {cp-1 != (uint8_t *}endPtr) {
         return STRUCT_MSG_ERR_BAD_VALUE;
      }
      *ep = cp;
      return STRUCT_MSG_ERR_OK;
    }
    
    # ================================= getStartFieldsFromLine ====================================
    
    proc getStartFieldsFromLine {line seqIdxVar} {
      variable hdrInfos
      upvar $seqIdxVar seqIdx
    
      set flds [split $line ,]
      dict set hdrInfos headerStartLgth 0
     
      set fieldName [lindex $flds 0]
      if {[string range $fieldName 0 0] ne "@"} {
        return $::STRUCT_MSG_ERR_FIELD_NOT_FOUND;
      }
      set result [::structmsg structmsgDataView getFieldNameIdFromStr $fieldName fieldNameId $::STRUCT_MSG_NO_INCR]
      if {$result != $::STRUCT_MSG_ERR_OK} {
        return $result
      }
      switch $fieldNameId {
        STRUCT_MSG_SPEC_FIELD_SRC {
          dict lappend hdrInfos headerSequence STRUCT_DISP_U16_SRC
          incr seqIdx
          dict lappend hdrInfos headerFlags STRUCT_DISP_U16_SRC
          dict set hdrInfos headerStartLgth [expr {[dict get $hdrInfos headerStartLgth] + 2}]
        }
        STRUCT_MSG_SPEC_FIELD_DST {
          dict lappend hdrInfos headerSequence STRUCT_DISP_U16_DST
          incr seqIdx
          dict lappend hdrInfos headerFlags STRUCT_DISP_U16_DST
          dict set hdrInfos headerStartLgth [expr {[dict get $hdrInfos headerStartLgth] + 2}]
        }
        STRUCT_MSG_SPEC_FIELD_TARGET_CMD {
          dict lappend hdrInfos headerSequence STRUCT_DISP_U8_TARGET
          incr seqIdx
          dict lappend hdrInfos headerFlags STRUCT_DISP_U8_TARGET;
          dict set hdrInfos headerStartLgth [expr {[dict get $hdrInfos headerStartLgth] + 1}]
        }
        default {
          return $::STRUCT_MSG_ERR_BAD_FIELD_NAME
        }
      }

      set fieldName [lindex $flds 1]
      if {[string range $fieldName 0 0] ne "@"} {
        return $::STRUCT_MSG_ERR_FIELD_NOT_FOUND;
      }
      set result [::structmsg structmsgDataView getFieldNameIdFromStr $fieldName fieldNameId $::STRUCT_MSG_NO_INCR]
      if {$result != $::STRUCT_MSG_ERR_OK} {
        return $result
      }
      switch $fieldNameId {
        STRUCT_MSG_SPEC_FIELD_SRC {
          if {[lsearch [dict get $hdrInfos headerFlags] STRUCT_DISP_U16_SRC] >= 0} {
            return $::STRUCT_MSG_ERR_DUPLICATE_FIELD
          }
          dict lappend hdrInfos headerSequence STRUCT_DISP_U16_SRC
          incr seqIdx
          dict lappend hdrInfos headerFlags STRUCT_DISP_U16_SRC
          dict set hdrInfos headerStartLgth [expr {[dict get $hdrInfos headerStartLgth] + 2}]
        }
        STRUCT_MSG_SPEC_FIELD_DST {
          if {[lsearch [dict get $hdrInfos headerFlags] STRUCT_DISP_U16_DST] >= 0} {
            return $::STRUCT_MSG_ERR_DUPLICATE_FIELD
          }
          dict lappend hdrInfos headerSequence STRUCT_DISP_U16_DST
          incr seqIdx
          dict lappend hdrInfos headerFlags STRUCT_DISP_U16_DST
          dict set hdrInfos headerStartLgth [expr {[dict get $hdrInfos headerStartLgth] + 2}]
        }
        STRUCT_MSG_SPEC_FIELD_TOTAL_LGTH {
          dict lappend hdrInfos headerSequence STRUCT_DISP_U16_TOTAL_LGTH
          incr seqIdx
          dict lappend hdrInfos headerFlags STRUCT_DISP_U16_TOTAL_LGTH
          dict set hdrInfos headerStartLgth [expr {[dict get $hdrInfos headerStartLgth] + 2}]
        }
        default {
           return $::STRUCT_MSG_ERR_BAD_FIELD_NAME
        }
      }

      if {[llength $flds] > 2} {
        set fieldName [lindex $flds 2]
        if {[string range $fieldName 0 0] ne "@"} {
          return $::STRUCT_MSG_ERR_FIELD_NOT_FOUND;
        }
        set result [::structmsg structmsgDataView getFieldNameIdFromStr $fieldName fieldNameId $::STRUCT_MSG_NO_INCR]
        if {$result != $::STRUCT_MSG_ERR_OK} {
          return $result
        }
        switch $fieldNameId {
          STRUCT_MSG_SPEC_FIELD_TOTAL_LGTH {
            if {[lsearch [dict get $hdrInfos headerFlags] STRUCT_DISP_U16_TOTAL_LGTH] >= 0} {
              return $::STRUCT_MSG_ERR_DUPLICATE_FIELD
            }
            dict lappend hdrInfos headerSequence STRUCT_DISP_U16_TOTAL_LGTH
            incr seqIdx
            dict lappend hdrInfos headerFlags STRUCT_DISP_U16_TOTAL_LGTH
            dict set hdrInfos headerStartLgth [expr {[dict get $hdrInfos headerStartLgth] + 2}]
          }
          default {
            return $::STRUCT_MSG_ERR_BAD_FIELD_NAME
          }
        }
      }
      return $::STRUCT_MSG_ERR_OK;
    }
      
    # ================================= getFlagStr ====================================
    
    proc getFlagStr {flags} {
      flag2Str_t *entry;
      int idx;
      
      idx = 0;
      while {1} {
        entry = &flag2Strs[idx];
        if {entry->flagStr == NULL} {
          return "??";
        }
        if {flags & entry->flagVal} {
          return entry->flagStr;
        }
        idx++;
      }
    }
    
    # ================================= initHeadersAndFlags ====================================
    
    proc initHeadersAndFlags {} {
      variable dispFlags

      set dispFlags 0
    
#      self->McuPart = 0x4D00;
#      self->WifiPart = 0x5700;
#      self->AppPart = 0x4100;
#      self->CloudPart = 0x4300;
      return $::STRUCT_MSG_ERR_OK
    }
    
    # ================================= readHeadersAndSetFlags ====================================
    
    proc readHeadersAndSetFlags {} {
      variable hdrInfos
      variable dispFlags
    
      set fd [open $::moduleFilesPath/MsgHeads.txt r]
      gets $fd line
      set flds [split $line ,]
      foreach {dummy numEntries} $flds break
      set hdrInfos [dict create]
      dict set hdrInfos numHeaderParts 0
      dict set hdrInfos headerFlags [list]
      # parse header start description
      gets $fd line
      set seqIdx 0
      set result [getStartFieldsFromLine $line seqIdx]
puts stderr "getStartFieldsFromLine: result: $result!seqIdx!$seqIdx!"
#      myDataView = newDataView{};
      dict set hdrInfos headerParts [list]
      set fieldOffset 0
      set idx 0
puts stderr "numEntries!$numEntries!"
      while {$idx < $numEntries} {
        dict set hdrInfos headerSequence [lrange [dict get $hdrInfos headerSequence] 0 [expr {$seqIdx -1 }]]
        gets $fd line
        if {[string length $line] == 0} {
          return $::STRUCT_MSG_ERR_TOO_FEW_FILE_LINES;
        }
        set hdr [dict create]
        dict set hdr hdrFlags [list]
        set flds [split $line ,]
        set seqIdx2 0
        set found false
        set myPart [lindex $flds $seqIdx2]
        if {[lsearch [lindex [dict get $hdrInfos headerSequence] $seqIdx2] STRUCT_DISP_U16_SRC] >= 0} {
          dict set hdr hdrFromPart $myPart
          set found true
        }
        if {[lsearch [lindex [dict get $hdrInfos headerSequence] $seqIdx2] STRUCT_DISP_U16_DST] >= 0} {
          dict set hdr hdrToPart $myPart
          set found true
        }
        if {[lsearch [lindex [dict get $hdrInfos headerSequence] $seqIdx2] STRUCT_DISP_U8_TARGET] >= 0} {
          dict set hdr hdrTargetPart $myPart
          set found true
        }
        if {!$found} {
          return $::STRUCT_MSG_ERR_FIELD_NOT_FOUND
        }
        incr seqIdx2
        set myPart [lindex $flds $seqIdx2]
        set found false
        if {[lsearch [lindex [dict get $hdrInfos headerSequence] $seqIdx2] STRUCT_DISP_U16_SRC] >= 0} {
          dict set hdr hdrFromPart $myPart
          set found true
        }
        if {[lsearch [lindex [dict get $hdrInfos headerSequence] $seqIdx2] STRUCT_DISP_U16_DST] >= 0} {
          dict set hdr hdrToPart $myPart
          set found true
        }
        if {[lsearch [lindex [dict get $hdrInfos headerSequence] $seqIdx2] STRUCT_DISP_U16_TOTAL_LGTH] >= 0} {
          dict set hdr hdrTotalLgth $myPart
          set found true
        }
        if {!$found} {
          return $::STRUCT_MSG_ERR_FIELD_NOT_FOUND
        }
        incr seqIdx2
        set myPart [lindex $flds $seqIdx2]
        if {$seqIdx > $seqIdx2} {
          set found false
          if {[lsearch [lindex [dict get $hdrInfos headerSequence] $seqIdx2] STRUCT_DISP_U16_TOTAL_LGTH] >= 0} {
            dict set hdr hdrTotalLgth $myPart
            set found true
          }
          if {!$found} {
            return $::STRUCT_MSG_ERR_FIELD_NOT_FOUND
          }
          incr seqIdx2
        }
        set myPart [lindex $flds $seqIdx2]
        # extra field lgth 0/<number>
        dict set hdr hdrExtraLgth $myPart
        dict lappend hdrInfos headerSequence STRUCT_DISP_U8_EXTRA_KEY_LGTH
        incr seqIdx2
        set myPart [lindex $flds $seqIdx2]
        # encryption E/N
        dict lappend hdrInfos headerSequence STRUCT_DISP_U8_ENCRYPTION
        dict set hdr hdrEncryption $myPart
        if {$myPart eq "E"} {
          dict lappend hdr hdrFlags STRUCT_DISP_IS_ENCRYPTED
        } else {
          dict lappend hdr hdrFlags STRUCT_DISP_IS_NOT_ENCRYPTED
        }
        incr seqIdx2
        set myPart [lindex $flds $seqIdx2]
        # handleType A/G/S/R/U/W/N
        dict lappend hdrInfos headerSequence STRUCT_DISP_U8_HANDLE_TYPE
        dict set hdr hdrHandleType $myPart
        switch $myPart {
          A {
            dict lappend hdr hdrFlags STRUCT_DISP_SEND_TO_APP
          }
          G {
            dict lappend hdr hdrFlags STRUCT_DISP_RECEIVE_FROM_APP
          }
          S {
            dict lappend hdr hdrFlags STRUCT_DISP_SEND_TO_UART
          }
          R {
            dict lappend hdr hdrFlags STRUCT_DISP_RECEIVE_FROM_UART
          }
          U {
            dict lappend hdr hdrFlags STRUCT_DISP_TRANSFER_TO_UART
          }
          W {
            dict lappend hdr hdrFlags STRUCT_DISP_TRANSFER_TO_CONN
          }
          N {
            dict lappend hdr hdrFlags STRUCT_DISP_NOT_RELEVANT
          }
          default {
            return $::STRUCT_MSG_ERR_BAD_VALUE
          }
        }
        incr seqIdx2
        set seqIdx3 $seqIdx2
        set myPart [lindex $flds $seqIdx2]
        # type of cmdKey
        set result [::structmsg dataView getFieldTypeIdFromStr $myPart fieldTypeId]
        if {$result != $::STRUCT_MSG_ERR_OK} {
          return $result
        }
        incr seqIdx3
        set myPart [lindex $flds $seqIdx3]
        # cmdKey
        switch $fieldTypeId {
          DATA_VIEW_FIELD_UINT8_T {
            dict lappend hdrInfos headerSequence STRUCT_DISP_U8_CMD_KEY
            dict lappend hdr hdrFlags STRUCT_DISP_U8_CMD_KEY
            dict set hdr hdrU8CmdKey $myPart
            lappend dispFlags STRUCT_MSG_U8_CMD_KEY
          }
          DATA_VIEW_FIELD_UINT16_T {
            dict lappend hdrInfos headerSequence STRUCT_DISP_U16_CMD_KEY
            dict lappend hdr hdrFlags STRUCT_DISP_U16_CMD_KEY
            dict set hdr hdrU16CmdKey $myPart
            set myIdx [lsearch $dispFlags STRUCT_MSG_U8_CMD_KEY]
            if {$myIdx >= 0} {
              set dispFlags [lreplace $dispGFlags $myIdx $myIdx]
            }
          }
          default {
           return $::STRUCT_MSG_ERR_BAD_FIELD_TYPE
          }
        }
        incr seqIdx3
        set myPart [lindex $flds $seqIdx3]
        # type of cmdLgth
        set result [::structmsg dataView getFieldTypeIdFromStr $myPart fieldTypeId]
        if {$result != $::STRUCT_MSG_ERR_OK} {
          return $result
        }
        set isEnd false
        if {$seqIdx2 >= [llength $flds]} {
          set isEnd true
        }
        switch $fieldTypeId {
          DATA_VIEW_FIELD_UINT0_T {
            dict lappend hdrInfos headerSequence STRUCT_DISP_U0_CMD_Lgth
            dict lappend hdr hdrFlags STRUCT_DISP_U0_CMD_Lgth
          }
          DATA_VIEW_FIELD_UINT8_T {
            dict lappend hdrInfos headerSequence STRUCT_DISP_U8_CMD_Lgth
            dict lappend hdr hdrFlags STRUCT_DISP_U8_CMD_Lgth
          }
          DATA_VIEW_FIELD_UINT16_T {
            dict lappend hdrInfos headerSequence STRUCT_DISP_U16_CMD_Lgth
            dict lappend hdr hdrFlags STRUCT_DISP_U16_CMD_Lgth
          }
          default {
            return $::STRUCT_MSG_ERR_BAD_FIELD_TYPE
          }
        }
        # type of crc
        if {!$isEnd} {
          incr seqIdx3
          set myPart [lindex $flds $seqIdx3]
        }
        set result [::structmsg dataView getFieldTypeIdFromStr $myPart fieldTypeId]
        if {$result != $::STRUCT_MSG_ERR_OK} {
          return $result
        }
        switch $fieldTypeId {
          DATA_VIEW_FIELD_UINT0_T {
            dict lappend hdrInfos headerSequence STRUCT_DISP_U0_CRC
            dict lappend hdr hdrFlags STRUCT_DISP_U0_CRC
          }
          DATA_VIEW_FIELD_UINT8_T {
            dict lappend hdrInfos headerSequence STRUCT_DISP_U8_CRC
            dict lappend hdr hdrFlags STRUCT_DISP_U8_CRC
          }
          DATA_VIEW_FIELD_UINT16_T {
            dict lappend hdrInfos headerSequence STRUCT_DISP_U16_CRC
            dict lappend hdr hdrFlags STRUCT_DISP_U16_CRC
          }
          default {
            return $::STRUCT_MSG_ERR_BAD_FIELD_TYPE
          }
        }
        dict lappend hdrInfos headerParts $hdr
        dict set hdrInfos numHeaderParts [expr {[dict get $hdrInfos numHeaderParts] + 1}]
        incr idx
      }
      close $fd
      return $result
    }
    
    # ================================= prepareNotEncryptedAnswer ====================================
    
    proc prepareNotEncryptedAnswer {parts type} {
      uint8_t fileName[30];
      int result;
      uint8_t numEntries;
      char *endPtr;
      uint8_t lgth;
      int msgLgth;
      uint8_t *data;
      uint8_t buf[100];
      uint8_t *buffer = buf;
      uint8_t numRows;
      uint8_t *handle;
      unsigned long ulgth;
      structmsgData_t *structmsgData;
      int idx;
    
    #ets_printf{"§@1@§", parts->u8CmdKey};
      if {parts->partsFlags & STRUCT_DISP_U8_CMD_KEY} {
    #ets_printf{"§@prepareNotEncryptedAnsweru8!%c!t!%c!@§", parts->u8CmdKey, type};
        os_sprintf{fileName, "Desc%c%c.txt", parts->u8CmdKey, type};
      } else {
    #ets_printf{"§@prepareNotEncryptedAnsweru16!%c%c!t!%c!@§", (parts->u16CmdKey>>8}& 0xFF, parts->u16CmdKey&0xFF, type);
        os_sprintf{fileName, "Desc%c%c%c.txt", (parts->u16CmdKey>>8}& 0xFF, parts->u16CmdKey&0xFF, type);
      }
      result = self->openFile{self, fileName, "r"};
      checkErrOK{result};
      result = self->readLine{self, &buffer, &lgth};
      checkErrOK{result};
      if {(lgth < 4} || (buffer[0] != '#')) {
        return $::STRUCT_MSG_ERR_BAD_FILE_CONTENTS;
      }
      ulgth = c_strtoul{buffer+2, &endPtr, 10};
      numEntries = {uint8_t}ulgth;
    #ets_printf{"§@NE1!%d!@§", numEntries};
      numRows = 0;
      result = self->createMsgFromLines{self, parts, numEntries, numRows, type, &structmsgData, &handle};
      checkErrOK{result};
    #ets_printf{"heap2: %d\n", system_get_free_heap_size(});
      result = self->closeFile{self};
      checkErrOK{result};
      if {parts->partsFlags & STRUCT_DISP_U8_CMD_KEY} {
        os_sprintf{fileName, "Val%c%c.txt", parts->u8CmdKey, type};
      } else {
        os_sprintf{fileName, "Val%c%c%c.txt", (parts->u16CmdKey>>8}&0xFF, parts->u16CmdKey&0xFF, type);
      }
      result = self->openFile{self, fileName, "r"};
      checkErrOK{result};
      result = self->readLine{self, &buffer, &lgth};
      checkErrOK{result};
      if {(lgth < 4} || (buffer[0] != '#')) {
        return STRUCT_MSG_ERR_BAD_FILE_CONTENTS;
      }
      ulgth = c_strtoul{buffer+2, &endPtr, 10};
      numEntries = {uint8_t}ulgth;
    #ets_printf{"§@NE2!%d!@§", numEntries};
      result = self->setMsgValuesFromLines{self, structmsgData, numEntries, handle, parts->u8CmdKey};
      checkErrOK{result};
      result = self->closeFile{self};
      checkErrOK{result};
    #ets_printf{"§heap3: %d§", system_get_free_heap_size(});
      result = structmsgData->getMsgData{structmsgData, &data, &msgLgth};
      checkErrOK{result};
      result = self->typeRSendAnswer{self, data, msgLgth};
      self->resetMsgInfo{self, parts};
      return STRUCT_MSG_ERR_OK;
    }
    
    # ================================= nextFittingEntry ====================================
    
    proc nextFittingEntry {u8CmdKey u16CmdKey} {
      variable hdrInfos
      variable received
    
      set hdrIdx [dict get $hdrInfos currPartIdx]
      set hdr [lindex [dict get $hdrInfos headerParts] $hdrIdx]
      # and now search in the headers to find the appropriate message
      dict set hdrInfos seqIdx [dict get $hdrInfos seqIdxAfterStart]
      set found false
      while {$hdrIdx < [dict get $hdrInfos numHeaderParts]} {
        set hdr [lindex [dict get $hdrInfos headerParts] $hdrIdx]
        if {[dict get $hdr hdrToPart] == [dict get $received toPart]} {
          if {[dict get $hdr hdrFromPart] == [dict get $received fromPart]} {
            if {([dict get $hdr hdrTotalLgth] == [dict get $received totalLgth]) || ([dict get $hdr hdrTotalLgth] == 0)} {
puts stderr "level2"
              if {$u8CmdKey != 0} {
                if {$u8CmdKey == [dict get $received u8CmdKey]} {
                  set found true
puts stderr "found1"
                  break
                }
              } else {
                if {$u16CmdKey != 0} {
                  if {$u16CmdKey == [dict get $received u16CmdKey]} {
                    set found true
puts stderr "found2"
                    break
                  }
                } else {
                  set found true
puts stderr "found3"
                  break
                }
              }
            }
          }
        }
        incr hdrIdx
      }
puts stderr "after while found!$found!"
      if {!$found} {
        return $::STRUCT_MSG_ERR_HANDLE_NOT_FOUND
      }
      dict set hdrInfos currPartIdx $hdrIdx
      # next sequence field is extraLgth {skip, we have it in hdr fields}
      dict incr hdrInfos seqIdx 1
      # next sequence field is encryption {skip, we have it in hdr fields}
      dict incr hdrInfos seqIdx 1
      # next sequence field is handle type {skip, we have it in hdr fields}
      dict incr hdrInfos seqIdx 1
      if {[dict get $hdr hdrEncryption] eq "N"} {
        dict lappend received partsFlags STRUCT_DISP_IS_NOT_ENCRYPTED
        # skip extraLgth, encrypted and handle Type
      } else {
        dict lappend received partsFlags STRUCT_DISP_IS_ENCRYPTED
      }
puts stderr "§found!$found!hdrIdx!$hdrIdx§"
      return $::STRUCT_MSG_ERR_OK;
    }
    
    # ================================= getHeaderIndexFromHeaderFields ====================================
    
    proc getHeaderIndexFromHeaderFields {} {
      variable hdrInfos
      variable received
    
      dict set received fieldOffset 0
      set myHeaderLgth 0
      dict set hdrInfos seqIdx 0
      switch [lindex [dict get $hdrInfos headerSequence] [dict get $hdrInfos seqIdx]] {
        STRUCT_DISP_U16_DST {
          set result [::structmsg dataView getUint16 [dict get $received fieldOffset] value]
          dict set received toPart $value
          if {$result != $::STRUCT_MSG_ERR_OK} {
            return $result
          }
          dict incr received fieldOffset 2
        }
        STRUCT_DISP_U16_SRC {
          set result [::structmsg dataView getUint16 [dict get $received fieldOffset] value]
          dict set received fromPart $value
          if {$result != $::STRUCT_MSG_ERR_OK} {
            return $result
          }
          dict incr received fieldOffset 2
        }
        STRUCT_DISP_U8_TARGET {
          set result [::structmsg dataView getUint8 [dict get $received fieldOffset] value]
          dict set received targetPart $value
          if {$result != $::STRUCT_MSG_ERR_OK} {
            return $result
          }
          dict incr received fieldOffset 1
        }
      }
      dict incr hdrInfos seqIdx 1
      switch [lindex [dict get $hdrInfos headerSequence] [dict get $hdrInfos seqIdx]] {
        STRUCT_DISP_U16_DST {
          set result [::structmsg dataView getUint16 [dict get $received fieldOffset] value]
          dict set received toPart $value
          if {$result != $::STRUCT_MSG_ERR_OK} {
            return $result
          }
          dict incr received fieldOffset 2
        }
        STRUCT_DISP_U16_SRC {
          set result [::structmsg dataView getUint16 [dict get $received fieldOffset] value]
          dict set received fromPart $value
          if {$result != $::STRUCT_MSG_ERR_OK} {
            return $result
          }
          dict incr received fieldOffset 2
        }
        STRUCT_DISP_U16_TOTAL_LGTH {
          set result [::structmsg dataView getUint16 [dict get $received fieldOffset] value]
          dict set received totalLgth $value
          if {$result != $::STRUCT_MSG_ERR_OK} {
            return $result
          }
          dict incr received fieldOffset 2
        }
      }
      if {[dict get $received fieldOffset] < [dict get $hdrInfos headerStartLgth]} {
        dict incr hdrInfos seqIdx 1
        switch [lindex [dict get $hdrInfos headerSequence] [dict get $hdrInfos seqIdx]] {
          STRUCT_DISP_U16_TOTAL_LGTH {
            set result [::structmsg dataView getUint16 [dict get $received fieldOffset] value]
            dict set received totalLgth $value
            if {$result != $::STRUCT_MSG_ERR_OK} {
              return $result
            }
            dict incr received fieldOffset 2
          }
        }
      }
      dict incr hdrInfos seqIdx 1
      dict set hdrInfos seqIdxAfterStart [dict get $hdrInfos seqIdx]
      dict set hdrInfos currPartIdx 0
      set result [nextFittingEntry 0 0]
      return $result
    }
    
    # ================================= handleNotEncryptedPart ====================================
    
    proc handleNotEncryptedPart {} {
      variable hdrInfos
      variable received
     
puts stderr "handleNotEncryptedPart"
      set hdrIdx [dict get $hdrInfos currPartIdx]
      set hdr [lindex [dict get $hdrInfos headerParts] $hdrIdx]
      set isU16CmdKey true
      # more than just transfer and we are receiving here, so handle Type must be R!
      if {[dict get $hdr hdrHandleType] ne "R"} {
        return $::STRUCT_DISP_ERR_BAD_HANDLE_TYPE;
      }
puts stderr "§el![dict get $hdr hdrExtraLgth]!enc![dict get $hdr hdrEncryption]!ht![dict get $hdr hdrHandleType]!§"
      if {[dict get $received lgth] == [expr {[dict get $hdrInfos headerStartLgth] + 1}]} {
        # get the cmdKey, we get its type from the header sequence!
        switch [lindex [dict get $hdrInfos headerSequence] [dict get $hdrInfos seqIdx]] {
          STRUCT_DISP_U16_CMD_KEY {
            set result [::structmsg dataView getUint16 [dict get $received fieldOffset] value]
            dict set received u16CmdKey [format "%c%c" [string range $value 0 0] [string range $value 1 1]]
            dict incr received fieldOffset 2
            dict lappend received partsFlags STRUCT_DISP_U16_CMD_KEY
            set isU16CmdKey true
puts stderr [format "§u16CmdKey!0x%04x!§" [dict get $received u16CmdKey]]
            while {[dict get $received u16CmdKey] ne [dict get $hdr hdrU16CmdKey]} {
              dict incr hdrInfos currPartIdx 1
              set result [nextFittingEntry 0 [dict get $received u16CmdKey]]
              if {$result != $::STRUCT_MSG_ERR_OK} {
puts stderr "nextFitting1 result!$result!"
                return $result
              }
              set hdr [lindex [dict get $hdrInfos headerParts] [dict get $hdrInfos currPartIdx]];
            }
          }
          STRUCT_DISP_U8_CMD_KEY {
            set result [::structmsg dataView getUint8 [dict get $received fieldOffset] value]
puts stderr [format "VAL!%c!%d!" $value $value]
            set val [format "%c" $value]
            dict set received u8CmdKey $val
puts stderr "DG![dict get $received u8CmdKey]!"
            dict incr received fieldOffset 1
            dict lappend received partsFlags STRUCT_DISP_U8_CMD_KEY
            set isU16CmdKey false
            set hdr [lindex [dict get $hdrInfos headerParts] [dict get $hdrInfos currPartIdx]];
            while {[dict get $received u8CmdKey] ne [dict get $hdr hdrU8CmdKey]} {
              set result [nextFittingEntry [dict get $received u8CmdKey] 0]
              if {$result != $::STRUCT_MSG_ERR_OK} {
puts stderr "nextFitting2 result!$result!"
                return $result
              }
puts stderr "RCV![dict get $received u8CmdKey]!HDR![dict get $hdr hdrU8CmdKey]!cpi![dict get $hdrInfos currPartIdx]"
              dict incr hdrInfos currPartIdx 1
              set hdr [lindex [dict get $hdrInfos headerParts] [dict get $hdrInfos currPartIdx]];
            }
          }
        }
        dict incr hdrInfos seqIdx 1
      } else {
        if {[dict get $received lgth] == [expr {[dict get $hdrInfos headerStartLgth] + 2}]} {
          # check if we have a cmdLgth
          switch [lindex [dict get $hdrInfos headerSequence] [dict get $hdrInfos seqIdx]] {
            STRUCT_DISP_U0_CMD_LGTH {
              dict incr received fieldOffset 2
puts stderr "§u0CmdLgth!0!§"
            }
            STRUCT_DISP_U8_CMD_LGTH {
              set result [::structmsg dataView getUint8 [dict get $received fieldOffset] value]
              set received u8CmdLgth $value
              dict incr received fieldOffset 1
puts stderr [format "§u8CmdLgth!%c!§" [dict get $received u8CmdLgth]]
            }
            STRUCT_DISP_U16_CMD_LGTH {
              set result [::structmsg dataView getUint16 [dict get $received fieldOffset] value]
              set received u16CmdLgth $value
              dict incr received fieldOffset 2
puts stderr [format "§u16CmdLgth!%c!§" [dict get $received u16CmdLgth]
            }
          }
          dict incr hdrInfos seqIdx 1
        }
puts stderr "==2"
        # just get the bytes until totalLgth reached
        if {[dict get $received lgth] == [dict get $received totalLgth]} {
    #ets_printf{"§not encrypted message completely receieved!%d!§", received->totalLgth};
          # check if we have a crc and the type of it
          # if we have a crc calculate it for the totalLgth
          set fieldInfo [dict create]
          switch [lindex [dict get $hdrInfos headerSequence] [dict get $hdrInfos seqIdx]] {
            STRUCT_DISP_U0_CRC {
    #ets_printf{"§u0Crc!0!§"};
            STRUCT_MSG_ERR_OK;
              set result $::STRUCT_MSG_ERR_OK
            } 
            STRUCT_DISP_U8_CRC {
              dict set fieldInfo fieldLgth 1
              dict set fieldInfo fieldOffset [expr {[dict get $received totalLgth] - 1}]
              set result [::structmsg structmsgDataView getCrc $fieldInfo crc 0 [dict get $fieldInfo fieldOffset]]
    #ets_printf{"§u8Crc!res!%d!§", result};
            } 
            STRUCT_DISP_U16_CRC {
              dict set fieldInfo fieldLgth 2
              dict set fieldInfo fieldOffset [expr {[dict get $received totalLgth] - 2}]
              set result [::structmsg structmsgDataView getCrc $fieldInfo crc 0 [dict get $fieldInfo fieldOffset]]
    #ets_printf{"§u16Crc!res!%d!§", result};
            } 
          }
          dict incr hdrInfos seqIdx 1
          if {$result != $::STRUCT_MSG_ERR_OK} {
            set answerType N
          } else {
            set answerType A
          }
puts stderr "handleNotEncryptedPart runAction"
if {0} {
          set result [runAction answerType]
          if {$result != $::STRUCT_MSG_ERR_OK} {
            return $result
          }
          set result [prepareNotEncryptedAnswer $answerType]
    #ets_printf{"§res NEA!%d!§", result};
          if {$result != $::STRUCT_MSG_ERR_OK} {
            return $result
          }
          set result [resetMsgInfo received]
          if {$result != $::STRUCT_MSG_ERR_OK} {
            return $result
          }
}
        }
      }
      return STRUCT_MSG_ERR_OK;
    }
    
    # ================================= handleReceivedPart ====================================
    
    proc handleReceivedPart {buffer lgth} {
      variable hdrInfos
      variable received

      
      set idx 0
      while {$idx < $lgth} {
        dict append received buf [string range $buffer $idx $idx]
        dict set received lgth [expr {[dict get $received lgth] + 1}]
        ::structmsg dataView appendData $buffer $lgth
        if {[dict get $received lgth] == [dict get $hdrInfos headerStartLgth]} {
          set result [getHeaderIndexFromHeaderFields]
        }
puts stderr "===received!"
        if {[dict get $received lgth] > [dict get $hdrInfos headerStartLgth]} {
          if {[lsearch [dict get $received partsFlags] STRUCT_DISP_IS_NOT_ENCRYPTED] >= 0} {
            set hdrIdx [dict get $hdrInfos currPartIdx]
            set hdr [lindex [dict get $hdrInfos headerParts] $hdrIdx]
            if {[dict get $hdr hdrEncryption] eq "N"} {
              set result [handleNotEncryptedPart]
puts stderr "§not encrypted message completely receieved![dict get $received totalLgth]!result!$result§"
            } else {
              # eventually add the extraField to the headerLgth here!!
              if {[dict get $received lgth] == [expr {[dict get $hdrInfos headerStartLgth] + 1}]} {
                if {[dict get $hdr hdrExtraLgth] > 0} {
                  dict set hdrInfos extraStartLgth [dict get $hdr hdrExtraLgth]
                }
              }
              # just get the bytes until totalLgth reached
              if {[dict get $received lgth] == [dict get $received totalLgth]} {
puts stderr "§encrypted message completely receieved![dict get $received totalLgth]!§"
              }
            }
          }
        }
        incr idx
      }
      return $::STRUCT_MSG_ERR_OK;
    }
    
    # ================================= structmsgIdentifyInit ====================================
    
    proc structmsgIdentifyInit {} {
#      self->readHeadersAndSetFlags = &readHeadersAndSetFlags;
#      self->handleReceivedPart = &handleReceivedPart;
#      self->prepareNotEncryptedAnswer = &prepareNotEncryptedAnswer;
      initHeadersAndFlags
      set result [readHeadersAndSetFlags]
      return $result
    }
    









    # ================================= structmsgIdentify ====================================
    
    proc structmsgIdentify {command args} {
puts stderr "structmsgIdentify: command!$command!args!$args!"
      switch $command {
        handleReceivedPart -
        structmsgIdentifyInit {
          return [uplevel 0 $command $args]
        }
      }
      return $::STRUCT_MSG_ERR_OK
    }

  } ; # namespace structmsgIdentify
} ; # namespace structmsg
