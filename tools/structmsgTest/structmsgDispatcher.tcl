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

set ::DISP_HANDLE_PREFIX "stmsgdisp_"

set ::STRUCT_DISP_ERR_OK                    0
set ::STRUCT_DISP_ERR_VALUE_NOT_SET         255
set ::STRUCT_DISP_ERR_VALUE_OUT_OF_RANGE    254
set ::STRUCT_DISP_ERR_BAD_VALUE             253
set ::STRUCT_DISP_ERR_BAD_FIELD_TYPE        252
set ::STRUCT_DISP_ERR_FIELD_TYPE_NOT_FOUND  251
set ::STRUCT_DISP_ERR_VALUE_TOO_BIG         250
set ::STRUCT_DISP_ERR_OUT_OF_MEMORY         249
set ::STRUCT_DISP_ERR_OUT_OF_RANGE          248
  # be carefull the values up to here
  # must correspond to the values in dataView.tcl !!!
  # with the names like DATA_VIEW_ERR_*

set ::STRUCT_DISP_ERR_FIELD_NOT_FOUND       230
set ::STRUCT_DISP_ERR_HANDLE_NOT_FOUND      227
  # be carefull the values up to here
  # must correspond to the values in structmsgDataView.tcl !!!
  # with the names like STRUCT_MSG_ERR_*

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

namespace eval structmsg {
  namespace ensemble create

    namespace export structmsgDispatcher

  namespace eval structmsgDispatcher {
    namespace ensemble create
      
    namespace export structmsgDispatcherInit freeStructmsgDataView createMsgFromLines
    namespace export createMsgFromLines setMsgValuesFromLines createDispatcher setMsgParts
    namespace export encryptMsg decryptMsg resetMsgInfo
    namespace export dumpMsgParts dumpHeaderParts dumpMsgHeaderInfos

    variable structmsgDispatcher [dict create]
    variable structmsgDispatcherHandles
    variable structmsgData [list]
    variable numMsgHeaders 0
    variable maxMsgHeaders 4
    variable msgHeader2MsgPtrs [list]

    dict set structmsgDispatcher id 0
    dict set structmsgDispatcher handle [list]
    dict set structmsgDispatcher received [dict create]
    dict set structmsgDispatcher toSend [dict create]

    dict set structmsgDispatcherHandles handles [list]
    dict set structmsgDispatcherHandles nnumHandles 0

    # ============================= setMsgParts ========================
    
    proc setMsgParts {which part} {
      variable structmsgDispatcher

      switch $which {
        received -
        toSend {
          dict set structmsgDispatcher $which $parts
          return $::STRUCT_MSG_ERR_OK
        }
        default {
          return $::STRUCT_MSG_ERR_BAD_VALUE
        }
      }
    }

    # ============================= toBase64 ========================
    
    proc toBase64 {msg len encoded} {
      size_t i;
      size_t n;
      uint8_t *q;
      uint8_t *out;
      uint8_t bytes64[sizeof{b64}];
    
      n = *len;
      if {!n} { # handle empty string case 
        return STRUCT_DISP_ERR_OUT_OF_MEMORY;
      }
      out = {uint8_t *}os_zalloc((n + 2) / 3 * 4);
      checkAllocOK{out};
      c_memcpy{bytes64, b64, sizeof(b64});   #Avoid lots of flash unaligned fetches
      
      for {i = 0, q = out; i < n; i += 3} {
        int a = msg[i];
        int b = {i + 1 < n} ? msg[i + 1] : 0;
        int c = {i + 2 < n} ? msg[i + 2] : 0;
        *q++ = bytes64[a >> 2];
        *q++ = bytes64[{(a & 3} << 4) | (b >> 4)];
        *q++ = {i + 1 < n} ? bytes64[((b & 15) << 2) | (c >> 6)] : BASE64_PADDING;
        *q++ = {i + 2 < n} ? bytes64[(c & 63)] : BASE64_PADDING;
      }
      *len = q - out;
      *encoded = out;
      return STRUCT_DISP_ERR_OK;
    }
    
    # ============================= fromBase64 ========================
    
    proc fromBase64 {encodedMsg len decodedMsg} {
      int i;
      int n;
      int blocks;
      int pad;
      const uint8 *p;
      uint8_t unbytes64[UCHAR_MAX+1];
      uint8_t *msg;
      uint8_t *q;
    
      n = *len;
      blocks = {n>>2};
      pad = 0;
      if {!n} { # handle empty string case 
        return STRUCT_DISP_ERR_OUT_OF_MEMORY;
      } 
      if {n & 3} {
        return STRUCT_DISP_ERR_INVALID_BASE64_STRING;
      } 
      c_memset{unbytes64, BASE64_INVALID, sizeof(unbytes64});
      for {i = 0; i < sizeof(b64}-1; i++) {
        unbytes64[b64[i]] = i;  # sequential so no exceptions 
      }
      if {encodedMsg[n-1] == BASE64_PADDING} {
        pad =  {encodedMsg[n-2] != BASE64_PADDING} ? 1 : 2;
        blocks--;  #exclude padding block
      }    
    
      for {i = 0; i < n - pad; i++} {
        if {!ISBASE64(encodedMsg[i]}) {
          return STRUCT_DISP_ERR_INVALID_BASE64_STRING;
        }
      }
      unbytes64[BASE64_PADDING] = 0;
      q = {uint8_t *} os_zalloc(1+ (3 * n / 4)); 
      checkAllocOK{q};
      msg = q;
      for {i = 0, p = encodedMsg; i<blocks; i++} {
        uint8 a = unbytes64[*p++]; 
        uint8 b = unbytes64[*p++]; 
        uint8 c = unbytes64[*p++]; 
        uint8 d = unbytes64[*p++];
        *q++ = {a << 2} | (b >> 4);
        *q++ = {b << 4} | (c >> 2);
        *q++ = {c << 6} | d;
      }
      if {pad} { #now process padding block bytes
        uint8 a = unbytes64[*p++];
        uint8 b = unbytes64[*p++];
        *q++ = {a << 2} | (b >> 4);
        if {pad == 1} *q++ = (b << 4) | (unbytes64[*p] >> 2);
      }
      *len = q - msg;
      *decodedMsg = msg;
      return STRUCT_DISP_ERR_OK;;
    }
    
    # ============================= addHandle ========================
    
    proc addHandle {handle} {
      variable structmsgDispatcher
      variable structmsgDispatcherHandles

      if {[dict get $structmsgDispatcherHandles handles] eq [list]} {
        set entry [dict create]
        dict set entry handle $handle
        dict set entry structmsgDispatcher $structmsgDispatcher
        dict lappend structmsgDispatcherHandles handles $entry
        dict incr structmsgDispatcherHandles numHandles 1
        return $::STRUCT_DISP_ERR_OK
      } else {
        # check for unused slot first
        set idx 0
        while {$idx < [dict get $structmsgDispatcherHandles numHandles]} {
          set handles [dict get $structmsgDispatcherHandles handles]
          set entry [lindex $handles $idx]
          if {[dict get $entry handle] eq [list]} {
            dict set entry handle $handle
            dict set entry structmsgDispatcher $structmsgDispatcher
            set handles [lreplace $handles $iidx $idx $entry
            dict set structmsgDispatcherHandles $handles
            return $::STRUCT_DISP_ERR_OK
          }
          incr idx
        }
        set handles [dict get $structmsgDispatcherHandles handles]
        set entry [dict create]
        dict set entry handle $handle
        dict set entry structmsgDispatcher $structmsgDispatcher
        dict lappend structmsgDispatcherHandles handles $entry
        dict incr structmsgDispatcherHandles numHandles 1
      }
      return $::STRUCT_DISP_ERR_OK
    }
    
    # ============================= deleteHandle ========================
    
    proc deleteHandle {handle} {
      variable structmsgDispatcher

      if {structmsgDispatcherHandles.handles == NULL} {
        return STRUCT_DISP_ERR_HANDLE_NOT_FOUND;
      }
      found = 0;
      idx = 0;
      numUsed = 0;
      while {idx < structmsgDispatcherHandles.numHandles} {
        if {(structmsgDispatcherHandles.handles[idx].handle != NULL} && (c_strcmp(structmsgDispatcherHandles.handles[idx].handle, handle) == 0)) {
          structmsgDispatcherHandles.handles[idx].handle = NULL;
          found++;
        } else {
          if {structmsgDispatcherHandles.handles[idx].handle != NULL} {
            numUsed++;
          }
        }
        idx++;
      }
      if {numUsed == 0} {
        os_free {structmsgDispatcherHandles.handles};
        structmsgDispatcherHandles.handles = NULL;
      }
      if {found} {
          return STRUCT_DISP_ERR_OK;
      }
      return STRUCT_DISP_ERR_HANDLE_NOT_FOUND;
    }
    
    # ============================= checkHandle ========================
    
    proc checkHandle {handle} {
      variable structmsgDispatcher
      int idx;
    
      if {structmsgDispatcherHandles.handles == NULL} {
        return STRUCT_DISP_ERR_HANDLE_NOT_FOUND;
      }
      idx = 0;
      while {idx < structmsgDispatcherHandles.numHandles} {
        if {(structmsgDispatcherHandles.handles[idx].handle != NULL} && (c_strcmp(structmsgDispatcherHandles.handles[idx].handle, handle) == 0)) {
          *structmsgDispatcher = structmsgDispatcherHandles.handles[idx].structmsgDispatcher;
          return STRUCT_DISP_ERR_OK;
        }
        idx++;
      }
      return STRUCT_DISP_ERR_HANDLE_NOT_FOUND;
    }
    
    # ================================= dumpHeaderParts ====================================
    
    proc dumpHeaderParts {hdr} {
      variable structmsgDispatcher
    
      puts stderr "dumpHeaderParts:"
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
      set idx 0
      while {$idx < 9} {
        puts stderr [format "%d %s" $idx [lindex [dict get $hdr fieldSequence] $idx]]
        incr idx
      }
      return $::STRUCT_DISP_ERR_OK
    }
    
    # ================================= dumpMsgHeaderInfos ====================================
    
    proc dumpMsgHeaderInfos {hdrInfos} {
      variable structmsgDispatcher
    
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
      while {$idx < 9} {
        puts stderr [format " %d %s" $idx [lindex [dict get $hdrInfos headerSequence] $idx]]
        incr idx
      }
      puts stderr [format "startLgth: %d numParts: %d maxParts: %d currPartIdx: %d seqIdx: %d seqIdxAfterStart: %d\n" [dict get $hdrInfos headerStartLgth] [dict get $hdrInfos numHeaderParts] [dict get $hdrInfos maxHeaderParts] [dict get $hdrInfos currPartIdx] [dict get $hdrInfos seqIdx] [dict get $hdrInfos seqIdxAfterStart]]
      return $::STRUCT_DISP_ERR_OK
    }
    
    # ================================= dumpMsgParts ====================================
    
    proc dumpMsgParts {msgParts} {
      variable structmsgDispatcher
    
      puts stderr "dumpMsgParts:\n"
      puts stderr [format "MsgParts1 form: 0x%04x to: 0x%04x totalLgth: %d u16_cmdLgth: %d u16CmdKey: 0x%04x targetPart: 0x%02x\n" [dict get $msgParts fromPart] [dict get $msgParts toPart] [dict get $msgParts totalLgth] [dict get $ msgParts u16CmdLgth] [dict get $msgParts u16CmdKey] [dict get $msgParts targetPart]]
    
      puts stderr [formaat "MsgParts2 u8CmdLgth: %d u8CmdKey: 0x%02x lgth: %d fieldOffset: %d\n" [dict get $msgParts u8CmdLgth] [dict get $msgParts u8CmdKey] [dict get $msgParts lgth] [dict get $msgParts fieldOffset]]
      puts stderr "buf"
      set idx 0
      while {$idx < [expr {[dict get $msgParts realLgth] - 1}]} {
        puts stderr [format " %d %s" $idx [lindex [dict get $msgParts buf] $idx]]
        incr idx
      }
      puts stderr "partFlags: [dict get $msgParts partsFlags]"
      return $::STRUCT_DISP_ERR_OK
    }

    # ================================= getMsgPtrFromMsgParts ====================================
    
    proc getMsgPtrFromMsgParts {msgParts incrRefCnt} {
      variable structmsgDispatcher
      variable structmsgData
      variable numMsgHeaders
      variable maxMsgHeaders
      variable msgHeader2MsgPtrs

      # build header from msgParts
      set offset 0
      set result [::structmsg dataView getData saveData saveLgth]
      set lgth [dict get $msgParts totalLgth]
      ::structmsg dataView setData [string repeat " " $lgth] $lgth
      set result [::structmsg dataView setUint16 $offset [dict get $msgParts fromPart]]
      if {$result != $::STRUCT_MSG_ERR_OK} {
        return $result
      }
      incr offset 2
      set result [::structmsg dataView setUint16 $offset [dict get $msgParts toPart]]
      if {$result != $::STRUCT_MSG_ERR_OK} {
        return $result
      }
      incr offset 2
      set result [::structmsg dataView setUint16 $offset [dict get $msgParts totalLgth]]
      if {$result != $::STRUCT_MSG_ERR_OK} {
        return $result
      }
      incr offset 2
      # FIXME dispFlags !!
      if {[lsearch $::structmsg::structmsgIdentify::dispFlags STRUCT_MSG_U8_CMD_KEY] >= 0} {
        set result [::structmsg dataView setUint8 $offset [dict get $msgParts u8CmdKey]]
        if {$result != $::STRUCT_MSG_ERR_OK} {
          return $result
        }
        incr offset 1
      } else {
        set result [::structmsg dataView setUint16 $offset [dict get $msgParts u16CmdKey]]
        if {$result != $::STRUCT_MSG_ERR_OK} {
          return $result
        }
        incr offset 2
      }
      set result [::structmsg dataView getData header headerLgth]
      if {$result != $::STRUCT_MSG_ERR_OK} {
        return $result
      }
      set result [::structmsg dataView setData $saveData $saveLgth]
      if {$result != $::STRUCT_MSG_ERR_OK} {
        return $result
      }
      # end build header from msgParts
      
      set firstFreeEntry [list]
      set firstFreeEntryId 0
      if {$numMsgHeaders > 0} {
        # find header 
        set headerIdx 0
        while {$headerIdx < $numMsgHeaders} {
          set headerEntry [lindex msgHeader2MsgPtrs $headerIdx]
          if {([dict get $headerEntry structmsgData] ne [list]) && ([string range [dict get $headerEntry header] 0 [expr {$offset - 1}]] eq $header)} {
            if {$incrRefCnt < 0} {
              dict set headerEntry headerLgth 0
              dict set headerEntry structmsgData [list]
              set msgHeader2MsgPtrs [lreplace $msgHeader2MsgPtrs $headerIdx $headerIdx $headerEntry]
              return $::STRUCT_DISP_ERR_OK
            }
            set structmsgData [dict get $headerEntry structmsgData]
# FIXME!!            dict set structmsgData flags [dict delete [dict get $structmsgData flags] STRUCT_MSG_IS_PREPARED]
            return $::STRUCT_DISP_ERR_OK
          }
          if {($incrRefCnt == $::STRUCT_MSG_INCR} && ([dict get $headerEntry structmsgData] eq [list]) && ($firstFreeEntry eq [list])) {
            set firstFreeEntry $headerEntry
            dict set firstFreeEntry structmsgData [dict create]
            dict set firstFreeEntry headerLgth $offset
            dict set firstFreeEntry header [string range $header 0 [expr {$offset - 1}]]
            set structmsgData [dict get $firstFreeEntry structmsgData]
            set msgHeader2MsgPtrs [lappend msgHeader2MsgPtrs $firstFreeEntry]
          }
          incr headerIdx
        }
      }
      if {$incrRefCnt < 0} {
        return $::STRUCT_DISP_ERR_OK; # just ignore silently
      } else {
        if {$incrRefCnt == 0} {
          return $::STRUCT_DISP_ERR_HEADER_NOT_FOUND;
        } else {
          if {$firstFreeEntry ne [list]} {
            set structmsgData [dict get $firstFreeEntry structmsgData]
            dict set firstFreeEntry headerLgth $offset
            dict set firstFreeEntry header [string range $header 0 [expr {$offset - 1}]]
          } else {
            set newHeaderEntry [dict create]
            dict set newHeaderEntry headerLgth $offset
            dict set newHeaderEntry header [string range $header 0 [expr {$offset - 1}]]
            set structmsg [dict create]
            dict set structmsg flags [list]
            dict set newHeaderEntry structmsgData $structmsg
            set structmsgData [dict get $newHeaderEntry structmsgData]
            incr numMsgHeaders
          }
        }
      }
      set result [::structmsg dataView setData $saveData $saveLgth]
      return $::STRUCT_DISP_ERR_OK;
    }
    
    # ================================= setMsgValuesFromLines ====================================
    
    proc setMsgValuesFromLines {fd numEntries handle type} {
      variable structmsgDispatcher
    
      set idx 0
      while {$idx < $numEntries} {
        gets $fd line
        set flds [split $line ","]
        foreach {fieldNameStr fieldValueStr} $flds break
        # fieldName
        set result [::structmsg structmsgDataView getFieldNameIdFromStr $fieldNameStr fieldNameId $::STRUCT_MSG_NO_INCR]
        if {$result != $::STRUCT_MSG_ERR_OK} {
          return $result
        }
        set result [::structmsg structmsgData getFieldTypeFromFieldNameId $fieldNameId fieldTypeId]
        if {$result != $::STRUCT_MSG_ERR_OK} {
          return $result
        }
    
        # fieldValue
        if {[string range $fieldValueStr 0 0] eq "@"} {
          # call the callback function vor the field!!
          set result [fillMsgValue $fieldValueStr value $type $fieldTypeId]
          if {$result != $::STRUCT_MSG_ERR_OK} {
            return $result
          }
        } else {
          set value $fieldValueStr
        }
        switch $fieldNameId {
          STRUCT_MSG_SPEC_FIELD_DST {
            set value [dict get $structmsgDispatcher received fromPart]
            set result [::structmsg structmsgData setFieldValue $fieldNameStr $value]
          }
          STRUCT_MSG_SPEC_FIELD_SRC {
            set value [dict get $structmsgDispatcher received toPart]
            set result [::structmsg structmsgData setFieldValue $fieldNameStr $value]
          }
          STRUCT_MSG_SPEC_FIELD_CMD_KEY {
            # check for u8CmdKey/u16CmdKey here
            if {[lsearch [dict get $structmsgDispatcher dispFlags] STRUCT_MSG_U8_CMD_KEY] >= 0} {
              set value[dict get $structmsgDispatcher received u8CmdKey]
            } else {
              set value = [dict get $structmsgDispatcher received u16CmdKey]
            }
            set result [::structmsg structmsgData setFieldValue $fieldNameStr $value]
          }
          default {
            set result [::structmsg structmsgData setFieldValue $fieldNameStr $value]
          }
        }
        if {$result != $::STRUCT_MSG_ERR_OK} {
          return $result
        }
        incr idx
      }
      set result [::structmsg structmsgData setFieldValue "@cmdKey" $type]
      if {$result != $::STRUCT_MSG_ERR_OK} {
        return $result
      }
      set result [::structmsg structmsgData prepareMsg]
    #  structmsgData->dumpMsg{structmsgData};
      return $result;
    }
    
    # ================================= createMsgFromLines ====================================
    
    proc createMsgFromLines {fd parts numEntries numRows type handleVar} {
      variable structmsgDispatcher
      variable structmsgData
      variable numMsgHeaders
      variable maxMsgHeaders
      upvar $handleVar handle

    #ets_printf{"§createMsgFromLines:%d!%d! \n§" $numMsgHeaders $maxMsgHeaders};
      dict set structmsgDispatcher received $parts
      set result [getMsgPtrFromMsgParts $parts $::STRUCT_MSG_INCR]
      if {$result != $::STRUCT_MSG_ERR_OK} {
        return $result
      }
      if {[lsearch [dict get $structmsgData flags] STRUCT_MSG_IS_INITTED] >= 0} {
        return $::STRUCT_DISP_ERR_OK
      }
      set result [::structmsg structmsgData createMsg $numEntries handle]
      if {$result != $::STRUCT_MSG_ERR_OK} {
        return $result
      }
      set idx 0
      while {$idx < $numEntries} {
        gets $fd line
        if {$line eq ""} {
          return $::STRUCT_DISP_ERR_TOO_FEW_FILE_LINES
        }
        set flds [split $line ","]
        foreach {fieldNameStr fieldTypeStr fieldLgthStr} $flds break
        if {$fieldLgthStr eq "@numRows"} {
          set fieldLgth $numRows
        } else {
          set fieldLgth $fieldLgthStr
        }
        set result [::structmsg structmsgData addField $fieldNameStr $fieldTypeStr $fieldLgth]
        if {$result != $::STRUCT_MSG_ERR_OK} {
          return $result
        }
        incr idx
      }
      ::structmsg structmsgData initMsg
      return $::STRUCT_DISP_ERR_OK;
    }
    
    # ================================= resetMsgInfo ====================================
    
    proc resetMsgInfo {partsVar} {
      variable structmsgDispatcher

puts stderr "partsVar!$partsVar!"
      set parts [dict create]
      dict set parts lgth 0
      dict set parts fieldOffset 0
      dict set parts fromPart 0
      dict set parts toPart 0
      dict set parts totalLgth 0
      dict set parts u16CmdLgth 0
      dict set parts u16CmdKey 0
      dict set parts u8CmdKey 0
      dict set parts u8CmdLgth 0
      dict set parts buf "" 
#      self->structmsgDataView->dataView->data = parts->buf;
#      self->structmsgDataView->dataView->lgth = 0;
      dict set structmsgDispatcher $partsVar $parts
      return $::STRUCT_DISP_ERR_OK
    }
    
    # ============================= encryptMsg ========================
    
    proc encryptMsg {msg mlen key klen iv ivlen dataVar lgthVar} {
      upvar $dataVar data
      upvar $lgthVar lgth

      set data [aes::aes -mode cbc -dir encrypt -key $key -iv $iv $msg]
      set lgth [string length $data]
      return $::STRUCT_MSG_ERR_OK
    }
    
    # ============================= decryptMsg ========================
    
    proc decryptMsg {msg mlen key klen iv ivlen bufVar lgthVar} {
      upvar $bufVar buf
      upvar $lgthVar lgth

      set buf [aes::aes -mode cbc -dir decrypt -key $key -iv $iv $msg]
      set lgth [string length $buf]
      return $::STRUCT_MSG_ERR_OK
    }
    
    # ============================= structmsgDispatcherGetPtrFromHandle ========================
    
    proc structmsgDispatcherGetPtrFromHandle {handle} {
      variablle structmsgDispatcher
    
      if {checkHandle(handle, structmsgDispatcher} != STRUCT_DISP_ERR_OK) {
        return STRUCT_DISP_ERR_HANDLE_NOT_FOUND;
      }
      return STRUCT_DISP_ERR_OK;
    }
    
    # ================================= initDispatcher ====================================
    
    proc initDispatcher {} {
      variable structmsgDispatcher

if {0} {
    ets_printf{"§initDispatcher!%p!§", self};
      result = structmsgIdentifyInit{self};
      checkErrOK{result};
      result = structmsgSendReceiveInit{self};
      checkErrOK{result};
      result = structmsgActionInit{self};
      checkErrOK{result};
      result = structmsgModuleDataValuesInit{self};
      checkErrOK{result};
      result = structmsgWebsocketInit{self};
      checkErrOK{result};
}
      return STRUCT_DISP_ERR_OK;
    }
    
    # ================================= createDispatcher ====================================
    
    proc createDispatcher {handleVar} {
      variable dispatcher
      upvar $handleVar handle
    
      dict incr structmsgDispatcher id 1
      dict set structmsgDispatcher handle "${::DISP_HANDLE_PREFIX}efff00[format %02d [dict get $structmsgDispatcher id]]"
      set result [addHandle [dict get $structmsgDispatcher handle]]
      if {$result != $::STRUCT_DISP_ERR_OK} {
#        deleteHandle(self->handle)
        return $result;
      }
      resetMsgInfo received
      resetMsgInfo toSend
      set handle [dict get $structmsgDispatcher handle]
      return $::STRUCT_DISP_ERR_OK
    }

    # ================================= newStructmsgDispatcher ====================================
    
     proc newStructmsgDispatcher {} {
      variable structmsgDispatcher

if {0} {
      if {structmsgDispatcherSingleton != NULL} {
        return structmsgDispatcherSingleton;
      }
      structmsgDispatcher_t *structmsgDispatcher = os_zalloc{sizeof(structmsgDispatcher_t});
      if {structmsgDispatcher == NULL} {
        return NULL;
      }
      structmsgDispatcher->structmsgDataView = newStructmsgDataView{};
      if {structmsgDispatcher->structmsgDataView == NULL} {
        return NULL;
      }
    
      structmsgDispatcherId++;
      structmsgDispatcher->id = structmsgDispatcherId;
    
      structmsgDispatcher->numMsgHeaders = 0;
      structmsgDispatcher->maxMsgHeaders = 0;
      structmsgDispatcher->msgHeader2MsgPtrs = NULL;
    
      structmsgDispatcher->msgHeaderInfos.headerFlags = 0;
      structmsgDispatcher->msgHeaderInfos.headerParts = NULL;
      structmsgDispatcher->msgHeaderInfos.numHeaderParts = 0;
      structmsgDispatcher->msgHeaderInfos.maxHeaderParts = 0;
    
      structmsgDispatcher->structmsgDataDescription = newStructmsgDataDescription{};
    
      structmsgDispatcher->createDispatcher = &createDispatcher;
      structmsgDispatcher->initDispatcher = &initDispatcher;
    
      structmsgDispatcher->BMsg = &BMsg;
      structmsgDispatcher->IMsg = &IMsg;
      structmsgDispatcher->MMsg = &MMsg;
      structmsgDispatcher->defaultMsg = &defaultMsg;
      structmsgDispatcher->resetMsgInfo = &resetMsgInfo;
      structmsgDispatcher->createMsgFromLines = &createMsgFromLines;
      structmsgDispatcher->setMsgValuesFromLines = &setMsgValuesFromLines;
    
      structmsgDispatcher->openFile = &openFile;
      structmsgDispatcher->closeFile = &closeFile;
      structmsgDispatcher->readLine = &readLine;
      structmsgDispatcher->writeLine = &writeLine;
    
      structmsgDispatcher->encryptMsg = &encryptMsg;
      structmsgDispatcher->decryptMsg = &decryptMsg;
      structmsgDispatcher->toBase64 = &toBase64;
      structmsgDispatcher->fromBase64 = &fromBase64;
    
      structmsgDispatcherSingleton = structmsgDispatcher;
}
      return $::STRUCT_MSG_ERR_OK
    }
    
    # ================================= freeStructmsgDispatcher ====================================
    
    proc freeStructmsgDispatcher {} {
      variable structmsgDispatcher
    }

  } ; # namespace structmsgDispatcher
} ; # namespace structmsg
