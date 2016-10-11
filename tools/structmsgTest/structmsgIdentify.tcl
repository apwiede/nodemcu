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
    variable dispFlags



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
            return $::STRUCT_DISP_ERR_DUPLICATE_FIELD
          }
          dict lappend hdrInfos headerSequence STRUCT_DISP_U16_SRC
          incr seqIdx
          dict lappend hdrInfos headerFlags STRUCT_DISP_U16_SRC
          dict set hdrInfos headerStartLgth [expr {[dict get $hdrInfos headerStartLgth] + 2}]
        }
        STRUCT_MSG_SPEC_FIELD_DST {
          if {[lsearch [dict get $hdrInfos headerFlags] STRUCT_DISP_U16_DST] >= 0} {
            return $::STRUCT_DISP_ERR_DUPLICATE_FIELD
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
              return $::STRUCT_DISP_ERR_DUPLICATE_FIELD
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
#        ::structmsg dataView setData $buffer $lgth
        set seqIdx2 0
#        result = getIntFromLine{myStr, &ulgth, &cp};
#        checkErrOK{result};
        set found false
        set myPart [lindex $flds $seqIdx2]
puts stderr "idx!$idx!flds!$flds!$myPart!"
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
            return $::STRUCT_DISP_ERR_BAD_VALUE
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
           return $::STRUCT_DISP_ERR_BAD_FIELD_TYPE
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
            return $::STRUCT_DISP_ERR_BAD_FIELD_TYPE
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
            return $::STRUCT_DISP_ERR_BAD_FIELD_TYPE
          }
        }
puts stderr "HDR!"
pdict $hdr
        dict lappend hdrInfos headerParts $hdr
        dict set hdrInfos numHeaderParts [expr {[dict get $hdrInfos numHeaderParts] + 1}]
        incr idx
      }
      close $fd
pdict $hdrInfos
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
        return STRUCT_DISP_ERR_BAD_FILE_CONTENTS;
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
        return STRUCT_DISP_ERR_BAD_FILE_CONTENTS;
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
      return STRUCT_DISP_ERR_OK;
    }
    
    # ================================= nextFittingEntry ====================================
    
    proc nextFittingEntry {u8CmdKey u16CmdKey} {
      msgParts_t *received;
      msgHeaderInfos_t *hdrInfos;
      dataView_t *dataView;
      headerParts_t *hdr;
      int hdrIdx;
      int found;
    
      dataView = self->structmsgDataView->dataView;
      received = &self->received;
      hdrInfos = &self->msgHeaderInfos;
      hdrIdx = hdrInfos->currPartIdx;
      hdr = &hdrInfos->headerParts[hdrIdx];
    #ets_printf{"§HEAD:!0x%04x!0x%04x!0x%04x!nH!%d!seqIdx!%d!§", received->toPart, received->fromPart, received->totalLgth, hdrInfos->numHeaderParts, hdrInfos->seqIdx};
      # and now search in the headers to find the appropriate message
      hdrInfos->seqIdx = hdrInfos->seqIdxAfterStart;
      found = 0;
      while {hdrIdx < hdrInfos->numHeaderParts} {
        hdr = &hdrInfos->headerParts[hdrIdx];
        if {hdr->hdrToPart == received->toPart} {
          if {hdr->hdrFromPart == received->fromPart} {
            if {(hdr->hdrTotalLgth == received->totalLgth} || (hdr->hdrTotalLgth == 0)) {
              if {u8CmdKey != 0} {
                if {u8CmdKey == received->u8CmdKey} {
                  found = 1;
                  break;
                }
              } else {
                if {u16CmdKey != 0} {
                  if {u16CmdKey == received->u16CmdKey} {
                    found = 1;
                    break;
                  }
                } else {
                  found = 1;
                  break;
                }
              }
            }
          }
        }
        hdrIdx++;
      }
      if {!found} {
        return STRUCT_DISP_ERR_HANDLE_NOT_FOUND;
      }
      hdrInfos->currPartIdx = hdrIdx;
      # next sequence field is extraLgth {skip, we have it in hdr fields}
      hdrInfos->seqIdx++;
      # next sequence field is encryption {skip, we have it in hdr fields}
      hdrInfos->seqIdx++;
      # next sequence field is handle type {skip, we have it in hdr fields}
      hdrInfos->seqIdx++;
      if {hdr->hdrEncryption == 'N'} {
        received->partsFlags |= STRUCT_DISP_IS_NOT_ENCRYPTED;
        # skip extraLgth, encrypted and handle Type
      } else {
        received->partsFlags |= STRUCT_DISP_IS_ENCRYPTED;
      }
    #ets_printf{"§found!%d!hdrIdx!%d§", found, hdrIdx};
      return STRUCT_DISP_ERR_OK;
    }
    
    # ================================= getHeaderIndexFromHeaderFields ====================================
    
    proc getHeaderIndexFromHeaderFields {received hdrInfos} {
      int result;
      dataView_t *dataView;
      headerParts_t *hdr;
      int hdrIdx;
      int found;
      uint8_t myHeaderLgth;
    
      dataView = self->structmsgDataView->dataView;
      received->fieldOffset = 0;
      myHeaderLgth = 0;
      hdrInfos->seqIdx = 0;
      switch{hdrInfos->headerSequence[hdrInfos->seqIdx]} {
      case STRUCT_DISP_U16_DST:
        result = dataView->getUint16{dataView, received->fieldOffset, &received->toPart};
        checkErrOK{result};
        received->fieldOffset += sizeof{uint16_t};
        break;
      case STRUCT_DISP_U16_SRC:
        result = dataView->getUint16{dataView, received->fieldOffset, &received->fromPart};
        checkErrOK{result};
        received->fieldOffset += sizeof{uint16_t};
        break;
      case STRUCT_DISP_U8_TARGET:
        result = dataView->getUint8{dataView, received->fieldOffset, &received->targetPart};
        checkErrOK{result};
        received->fieldOffset += sizeof{uint8_t};
        break;
      }
      hdrInfos->seqIdx++;
      switch{hdrInfos->headerSequence[hdrInfos->seqIdx]} {
      case STRUCT_DISP_U16_DST:
        result = dataView->getUint16{dataView, received->fieldOffset, &received->toPart};
        checkErrOK{result};
        received->fieldOffset += sizeof{uint16_t};
        break;
      case STRUCT_DISP_U16_SRC:
        result = dataView->getUint16{dataView, received->fieldOffset, &received->fromPart};
        checkErrOK{result};
        received->fieldOffset += sizeof{uint16_t};
        break;
      case STRUCT_DISP_U16_TOTAL_LGTH:
        result = dataView->getUint16{dataView, received->fieldOffset, &received->totalLgth};
        checkErrOK{result};
        received->fieldOffset += sizeof{uint16_t};
        break;
      }
      if {received->fieldOffset < hdrInfos->headerStartLgth} {
        hdrInfos->seqIdx++;
        switch{hdrInfos->headerSequence[hdrInfos->seqIdx]} {
        case STRUCT_DISP_U16_TOTAL_LGTH:
          result = dataView->getUint16{dataView, received->fieldOffset, &received->totalLgth};
          checkErrOK{result};
          received->fieldOffset += sizeof{uint16_t};
          break;
        }
      }
      hdrInfos->seqIdx++;
      hdrInfos->seqIdxAfterStart = hdrInfos->seqIdx;
      hdrInfos->currPartIdx = 0;
      result = nextFittingEntry{self, 0, 0};
    #ets_printf{"§fit!%d!%d!§", result, hdrInfos->currPartIdx};
      checkErrOK{result};
    }
    
    # ================================= handleNotEncryptedPart ====================================
    
    proc handleNotEncryptedPart {received hdrInfos} {
      int result;
      dataView_t *dataView;
      headerParts_t *hdr;
      int hdrIdx;
      uint8_t answerType;
      structmsgField_t fieldInfo;
      bool isU16CmdKey;
    
      dataView = self->structmsgDataView->dataView;
      hdrIdx = hdrInfos->currPartIdx;
      hdr = &hdrInfos->headerParts[hdrIdx];
      isU16CmdKey = true;
      # more than just transfer and we are receiving here, so handle Type must be R!
      if {hdr->hdrHandleType != 'R'} {
        return STRUCT_DISP_ERR_BAD_HANDLE_TYPE;
      }
    #ets_printf{"§el!%d!enc!%c!ht!%c!§", hdr->hdrExtraLgth, hdr->hdrEncryption, hdr->hdrHandleType};
      if {received->lgth == hdrInfos->headerStartLgth + 1} {
        # get the cmdKey, we get its type from the header sequence!
        switch{hdrInfos->headerSequence[hdrInfos->seqIdx]} {
        case STRUCT_DISP_U16_CMD_KEY:
          result = dataView->getUint16{dataView, received->fieldOffset, &received->u16CmdKey};
          received->fieldOffset += sizeof{uint16_t};
          received->partsFlags |= STRUCT_DISP_U16_CMD_KEY;
          isU16CmdKey = true;
    #ets_printf{"§u16CmdKey!0x%04x!§", received->u16CmdKey};
          while {received->u16CmdKey != hdr->hdrU16CmdKey} {
            hdrInfos->currPartIdx++;
            result = nextFittingEntry{self, 0, received->u16CmdKey};
            checkErrOK{result};
            hdr = &hdrInfos->headerParts[hdrInfos->currPartIdx];
          }
          break;
        case STRUCT_DISP_U8_CMD_KEY:
          result = dataView->getUint8{dataView, received->fieldOffset, &received->u8CmdKey};
          received->fieldOffset += sizeof{uint8_t};
          received->partsFlags |= STRUCT_DISP_U8_CMD_KEY;
          isU16CmdKey = false;
          while {received->u8CmdKey != hdr->hdrU8CmdKey} {
            hdrInfos->currPartIdx++;
            result = nextFittingEntry{self, received->u8CmdKey, 0};
            checkErrOK{result};
            hdr = &hdrInfos->headerParts[hdrInfos->currPartIdx];
          }
          break;
        }
        hdrInfos->seqIdx++;
      } else {
        if {received->lgth == hdrInfos->headerStartLgth + 2} {
          # check if we have a cmdLgth
          switch{hdrInfos->headerSequence[hdrInfos->seqIdx]} {
          case STRUCT_DISP_U0_CMD_LGTH:
            received->fieldOffset += sizeof{uint16_t};
    #ets_printf{"§u0CmdLgth!0!§"};
            break;
          case STRUCT_DISP_U8_CMD_LGTH:
            result = dataView->getUint8{dataView, received->fieldOffset, &received->u8CmdLgth};
            received->fieldOffset += sizeof{uint8_t};
    #ets_printf{"§u8CmdLgth!%c!§", received->u8CmdLgth};
            break;
          case STRUCT_DISP_U16_CMD_LGTH:
            result = dataView->getUint16{dataView, received->fieldOffset, &received->u16CmdLgth};
            received->fieldOffset += sizeof{uint16_t};
    #ets_printf{"§u16CmdLgth!%c!§", received->u16CmdLgth};
            break;
          }
          hdrInfos->seqIdx++;
        }
        # just get the bytes until totalLgth reached
        if {received->lgth == received->totalLgth} {
    #ets_printf{"§not encrypted message completely receieved!%d!§", received->totalLgth};
          # check if we have a crc and the type of it
          # if we have a crc calculate it for the totalLgth
          switch{hdrInfos->headerSequence[hdrInfos->seqIdx]} {
          case STRUCT_DISP_U0_CRC:
    #ets_printf{"§u0Crc!0!§"};
            result = STRUCT_DISP_ERR_OK;
            break;
          case STRUCT_DISP_U8_CRC:
            fieldInfo.fieldLgth = sizeof{uint8_t};
            fieldInfo.fieldOffset = received->totalLgth - sizeof{uint8_t};
            result = self->structmsgDataView->getCrc{self->structmsgDataView, &fieldInfo, 0, fieldInfo.fieldOffset};
    #ets_printf{"§u8Crc!res!%d!§", result};
            break;
          case STRUCT_DISP_U16_CRC:
            fieldInfo.fieldLgth = sizeof{uint16_t};
            fieldInfo.fieldOffset = received->totalLgth - sizeof{uint16_t};
            result = self->structmsgDataView->getCrc{self->structmsgDataView, &fieldInfo, 0, fieldInfo.fieldOffset};
    #ets_printf{"§u16Crc!res!%d!§", result};
            break;
          }
          hdrInfos->seqIdx++;
          if {result != STRUCT_DISP_ERR_OK} {
            answerType = 'N';
          } else {
            answerType = 'A';
          }
          result = self->runAction{self, &answerType};
          checkErrOK{result};
          result = self->prepareNotEncryptedAnswer{self, received, answerType};
    #ets_printf{"§res NEA!%d!§", result};
          checkErrOK{result};
          result = self->resetMsgInfo{self, &self->received};
          checkErrOK{result};
        }
      }
      return STRUCT_DISP_ERR_OK;
    }
    
    # ================================= handleReceivedPart ====================================
    
    proc handleReceivedPart {buffer lgth} {
      int idx;
      msgParts_t *received;
      dataView_t *dataView;
      msgHeaderInfos_t *hdrInfos;
      headerParts_t *hdr;
      int startIdx;
      int hdrIdx;
      uint8_t u8;
      structmsgField_t fieldInfo;
      int result;
    
      hdrInfos = &self->msgHeaderInfos;
      received = &self->received;
      dataView = self->structmsgDataView->dataView;
      dataView->data = received->buf;
    #ets_printf{"§headerStartLgth!%d§", hdrInfos->headerStartLgth};
    #ets_printf{"§receivedLgth: %d fieldOffset: %d!\n§", received->lgth, received->fieldOffset};
      idx = 0;
      while {idx < lgth} {
        received->buf[received->lgth++] = buffer[idx];
        dataView->lgth++;
        if {received->lgth == hdrInfos->headerStartLgth} {
          result = getHeaderIndexFromHeaderFields{self, received, hdrInfos};
        }
        if {received->lgth > hdrInfos->headerStartLgth} {
          if {received->partsFlags & STRUCT_DISP_IS_NOT_ENCRYPTED} {
            hdrIdx = hdrInfos->currPartIdx;
            hdr = &hdrInfos->headerParts[hdrIdx];
            if {hdr->hdrEncryption == 'N'} {
              result = handleNotEncryptedPart{self, received, hdrInfos};
            } else {
              # eventually add the extraField to the headerLgth here!!
              if {received->lgth == hdrInfos->headerStartLgth + 1} {
                if {hdr->hdrExtraLgth > 0} {
                  hdrInfos->headerStartLgth += hdr->hdrExtraLgth;
                }
              }
              # just get the bytes until totalLgth reached
              if {received->lgth == received->totalLgth} {
    ets_printf{"§encrypted message completely receieved!%d!§", received->totalLgth};
              }
            }
          }
        }
        idx++;
      }
      return STRUCT_DISP_ERR_OK;
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
        structmsgIdentifyInit {
          return [uplevel 0 $command $args]
        }
      }
      return $::STRUCT_MSG_ERR_OK
    }

  } ; # namespace structmsgIdentify
} ; # namespace structmsg
