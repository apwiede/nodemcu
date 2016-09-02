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

package require aes

namespace eval structmsg {
  namespace ensemble create

  namespace export cmd

  namespace eval cmd {
    namespace ensemble create
      
    namespace export dump_binary create add_field set_fieldValue set_tableFieldValue
    namespace export get_fieldValue get_tableFieldValue encode get_encoded decode dump
    namespace export encrypt decrypt set_crypted get_decrypted decrypt_getHandle
    namespace export set_crypted

    # ============================= binaryScanChar ========================
    
    proc binaryScanChar {ch} {
        binary scan $ch c pch
        set pch [expr {$pch & 0xFF}]
        return $pch
    }
    
    # ===================== dump_binary =============================
    
    proc dump_binary {data where} {
      puts stderr "$where"
      set idx 0
    puts stderr "lgth: [string length $data]!"
      foreach ch [split $data ""] {
        binary scan $ch c pch
        set pch [expr {$pch & 0xFF}]
        puts stderr [format "idx: %d 0x%02x" $idx $pch]
        incr idx
      }
    }
    
    # ===================== dump_values =============================
    
    proc dump_values {data} {
      puts -nonewline stderr "      values: "
      set cnt 0
    if {[catch {
      foreach ch [split $data ""] {
        set pch [binaryScanChar $ch]
        if {$ch eq "%"} {
          set ch "%%"
        }
        puts -nonewline stderr [format "  $ch 0x%02x" $pch]
        incr cnt
        if {($cnt > 0) && (($cnt % 10) == 0)} {
          puts -nonewline stderr "\n              "
        }
      }
    } msg]} {
      puts stderr "\nMSG: $msg!"
      foreach ch [split $data ""] {
        set pch [binaryScanChar $ch]
    puts stderr "\n!${ch}!${pch}!"
      }
    }
      puts stderr ""
    }
    
    # ===================== fillHdrInfo =============================
    
    proc fillHdrInfo {handle} {
      if {![info exists ::structmsg($handle)]} {
        error "no such structmsg: $handle"
      }
      set myDict $::structmsg($handle)
      set myHandles $::structmsg(handles)
      set idx 0
      foreach handle2Hdr $myHandles {
        if {[dict get $handle2Hdr handle] eq $handle} {
          set hdrInfo [dict get $handle2Hdr hdrInfo]
          set hdrId ""
          set offset 0
          set offset [::structmsg encdec uint16Encode hdrId $offset [dict get $hdrInfo hdrKeys src]]
          set offset [::structmsg encdec uint16Encode hdrId $offset [dict get $hdrInfo hdrKeys dst]]
          set offset [::structmsg encdec uint16Encode hdrId $offset [dict get $hdrInfo hdrKeys totalLgth]]
          set offset [::structmsg encdec uint16Encode hdrId $offset [dict get $hdrInfo hdrKeys cmdKey]]
          set offset [::structmsg encdec uint16Encode hdrId $offset [dict get $hdrInfo hdrKeys cmdLgth]]
          dict set hdrInfo hdrId $hdrId
          dict set handle2Hdr hdrInfo $hdrInfo
          set myHandles [lreplace $myHandles $idx $idx $handle2Hdr]
          set ::structmsg(handles) $myHandles
          break
        }
        incr idx
      }
    }
    
    
    # ===================== setHandleField =============================
    
    proc setHandleField {handle fieldName value} {
      if {![info exists ::structmsg($handle)]} {
        error "no such structmsg: $handle"
      }
      set myHandles $::structmsg(handles)
      set idx 0
      foreach handle2Hdr $myHandles {
        if {[dict get $handle2Hdr handle] eq $handle} {
          set hdrInfo [dict get $handle2Hdr hdrInfo]
          dict set hdrInfo hdrKeys $fieldName $value
          dict set handle2Hdr hdrInfo $hdrInfo
          set myHandles [lreplace $myHandles $idx $idx $handle2Hdr]
          set ::structmsg(handles) $myHandles
          break
        }
        incr idx
      }
    }
    
    # ============================= fixHeaderInfo ========================
    
    proc fixHeaderInfo {myDictVar fieldInfoVar fieldName fieldType fieldLgth numTableRows} {
      upvar $myDictVar myDict
      upvar $fieldInfoVar fieldInfo
    
      dict set fieldInfo fieldName $fieldName
      dict set fieldInfo fieldType $fieldType
      dict set fieldInfo value [list]
      dict set fieldInfo flags [list]
      set totalLgth [dict get $myDict hdr hdrInfo hdrKeys totalLgth]
      set cmdLgth [dict get $myDict hdr hdrInfo hdrKeys cmdLgth]
      set value [list]
      switch $fieldType {
        uint8_t -
        int8_t {
          dict set myDict hdr hdrInfo hdrKeys totalLgth [expr {$totalLgth + 1 * $numTableRows}]
          dict set myDict hdr hdrInfo hdrKeys cmdLgth [expr {$cmdLgth + 1 * $numTableRows}]
          set fieldLgth 1
        }
        uint16_t -
        int16_t {
          dict set myDict hdr hdrInfo hdrKeys totalLgth [expr {$totalLgth + 2 * $numTableRows}]
          dict set myDict hdr hdrInfo hdrKeys cmdLgth [expr {$cmdLgth + 2 * $numTableRows}]
          set fieldLgth 2
        }
        uint32_t -
        int32_t {
          dict set myDict hdr hdrInfo hdrKeys totalLgth [expr {$totalLgth + 4 * $numTableRows}]
          dict set myDict hdr hdrInfo hdrKeys cmdLgth [expr {$cmdLgth + 4 * $numTableRows}]
          set fieldLgth 4
        }
        uint8_t* {
          dict set myDict hdr hdrInfo hdrKeys totalLgth [expr {$totalLgth + $fieldLgth * $numTableRows}]
          dict set myDict hdr hdrInfo hdrKeys cmdLgth [expr {$cmdLgth + $fieldLgth * $numTableRows}]
          dict set fieldInfo value $value
          dict set fieldInfo value $value
        }
        int8_t* {
          dict set myDict hdr hdrInfo hdrKeys totalLgth [expr {$totalLgth + $fieldLgth * $numTableRows}]
          dict set myDict hdr hdrInfo hdrKeys cmdLgth [expr {$cmdLgth + $fieldLgth * $numTableRows}]
          dict set fieldInfo value $value
          dict set fieldInfo value $value
        }
        unit16_t* {
          dict set myDict hdr hdrInfo hdrKeys totalLgth [expr {$totalLgth + $fieldLgth * $numTableRows}]
          dict set myDict hdr hdrInfo hdrKeys cmdLgth [expr {$cmdLgth + $fieldLgth * $numTableRows}]
          dict set fieldInfo value $value
        }
        int16_t* {
          dict set myDict hdr hdrInfo hdrKeys totalLgth [expr {$totalLgth + $fieldLgth * $numTableRows}]
          dict set myDict hdr hdrInfo hdrKeys cmdLgth [expr {$cmdLgth + $fieldLgth * $numTableRows}]
          dict set fieldInfo value $value
        }
        uint32_t* {
          dict set myDict hdr hdrInfo hdrKeys totalLgth [expr {$totalLgth + $fieldLgth * $numTableRows}]
          dict set myDict hdr hdrInfo hdrKeys cmdLgth [expr {$cmdLgth + $fieldLgth * $numTableRows}]
          dict set fieldInfo value $value
        }
        int32_t* {
          dict set myDict hdr hdrInfo hdrKeys totalLgth [expr {$totalLgth + $fieldLgth * $numTableRows}]
          dict set myDict hdr hdrInfo hdrKeys cmdLgth [expr {$cmdLgth + $fieldLgth * $numTableRows}]
          dict set fieldInfo value $value
        }
      }
    puts stderr "FIXHDR CMD_LGTH: [dict get $myDict hdr hdrInfo hdrKeys cmdLgth]!"
    puts stderr "FIXHDR TOTAL_LGTH: [dict get $myDict hdr hdrInfo hdrKeys totalLgth]!"
      setHandleField [dict get $myDict handle] $::STRUCT_MSG_FIELD_CMD_LGTH [dict get $myDict hdr hdrInfo hdrKeys cmdLgth]
      setHandleField [dict get $myDict handle] $::STRUCT_MSG_FIELD_TOTAL_LGTH [dict get $myDict hdr hdrInfo hdrKeys totalLgth]
      dict set fieldInfo fieldLgth $fieldLgth
      return $::STRUCT_MSG_ERR_OK
    }
    # ===================== addHandle =============================
    
    proc addHandle {handle} {
      set handle2Hdr [dict create]
      dict set handle2Hdr handle $handle
      set hdrInfo [dict create]
      dict set hdrInfo hdrKeys src 0
      dict set hdrInfo hdrKeys dst 0
      dict set hdrInfo hdrKeys totalLgth 0
      dict set hdrInfo hdrKeys cmdKey 0
      dict set hdrInfo hdrKeys cmdLgth 0
      dict set hdrInfo hdrId [list]
      dict set handle2Hdr hdrInfo $hdrInfo
      lappend ::structmsg(handles) $handle2Hdr
      incr ::structmsg(numHandles)
    }
    
    # ===================== getHandle =============================
    
    proc getHandle {decrypted handleVal} {
      upvar $handleVal handle
    
      set myHandles $::structmsg(handles)
      set idx 0
      set hdrId [string range $decrypted 0 [expr {$::STRUCT_MSG_HEADER_LENGTH + $::STRUCT_MSG_CMD_HEADER_LENGTH - 1}]] ; # +4: cmdKey + cmdLgth!
      foreach handle2Hdr $myHandles {
        if {[dict get $handle2Hdr hdrInfo hdrId] eq $hdrId} {
          set handle [dict get $handle2Hdr handle]
puts stderr "found"
          return $::STRUCT_MSG_ERR_OK
        }
        incr idx
      }
      return $::STRUCT_MSG_ERR_HANDLE_NOT_FOUND
    }
    
    # ===================== create =============================
    
    proc create {numFieldInfos handleVar} {
      upvar $handleVar handle
    
      set myDict [dict create]
      dict set myDict hdr hdrInfo hdrKeys src 0
      dict set myDict hdr hdrInfo hdrKeys dst 0
      dict set myDict hdr hdrInfo hdrKeys cmdKey 0
      dict set myDict hdr hdrInfo hdrKeys cmdLgth $::STRUCT_MSG_CMD_HEADER_LENGTH
      dict set myDict hdr hdrInfo hdrKeys totalLgth $::STRUCT_MSG_TOTAL_HEADER_LENGTH
      dict set myDict hdr headerLgth $::STRUCT_MSG_HEADER_LENGTH
      dict set myDict msg fieldInfos [list]
      dict set myDict msg numFieldInfos 0
      dict set myDict msg maxFieldInfos $numFieldInfos
      dict set myDict msg numTableRows 0
      dict set myDict msg numTableRowFields 0
      dict set myDict msg numRowFields 0
      dict set myDict msg tableFieldInfos [list]
      dict set myDict flags [list]
      dict set myDict sequenceNum 0
      dict set myDict encoded [list]
      dict set myDict todecode [list]
      dict set myDict encrypted [list]
      dict set myDict handleHdrInfoPtr [list]
      set handle $::structmsg(prefix)ffef[format "%04d" $::structmsg(numHandles)]
      dict set myDict handle $handle
      addHandle $handle
      set ::structmsg($handle) $myDict
      setHandleField $handle totalLgth [expr {$::STRUCT_MSG_HEADER_LENGTH + $::STRUCT_MSG_CMD_HEADER_LENGTH}]
      setHandleField $handle cmdLgth $::STRUCT_MSG_CMD_HEADER_LENGTH
      fillHdrInfo $handle
      return $::STRUCT_MSG_ERR_OK
    }
    
    # ===================== delete =============================
    
    proc delete {handle} {
      # FIXME !! TBD
    }
    
    # ============================= encode ========================
    
    proc encode {handle} {
      if {![info exists ::structmsg($handle)]} {
        error "no such structmsg: $handle"
      }
      set_fillerAndCrc $handle
      set myDict $::structmsg($handle)
      dict set myDict encoded [list]
      set encoded [list]
      set offset 0
      set offset [::structmsg encdec uint16Encode encoded $offset [dict get $myDict hdr hdrInfo hdrKeys src]]
      set offset [::structmsg encdec uint16Encode encoded $offset [dict get $myDict hdr hdrInfo hdrKeys dst]]
      set offset [::structmsg encdec uint16Encode encoded $offset [dict get $myDict hdr hdrInfo hdrKeys totalLgth]]
      set offset [::structmsg encdec uint16Encode encoded $offset [dict get $myDict hdr hdrInfo hdrKeys cmdKey]]
      set offset [::structmsg encdec uint16Encode encoded $offset [dict get $myDict hdr hdrInfo hdrKeys cmdLgth]]
#    puts stderr "CMD_LGTH: [dict get $myDict hdr hdrInfo hdrKeys cmdLgth]!"
      set numEntries [dict get $myDict msg numFieldInfos]
      set offset [::structmsg encdec uint8Encode encoded $offset $numEntries]
    
      set idx 0
      while {$idx < $numEntries} {
        set fieldInfos [dict get $myDict msg fieldInfos]
        set fieldInfo [lindex $fieldInfos $idx]
        if {[string range [dict get $fieldInfo fieldName] 0 0] eq "@"} {
          set result [::structmsg def getFieldNameId [dict get $fieldInfo fieldName] fieldId $::STRUCT_MSG_NO_INCR]
          if {$result != $::STRUCT_MSG_ERR_OK} { return $result }
    puts stderr "offset0: $offset![dict get $fieldInfo fieldName]!fieldId: $fieldId!"
          set specFieldName [dict get $::structmsg(specialFieldIds) $fieldId]
          switch $specFieldName { 
            @src -
            @dst -
            @targetCmd -
            @totalLgth -
            @cmdKey -
            @cmdLgth {
    puts stderr [format "funny should encode: %s" [dict get $fieldInfo fieldName]]
            }
            @randomNum {
              set offset [::structmsg encdec randomNumEncode encoded $offset randomNum]
              if {$offset < 0} { return $::STRUCT_MSG_ERR_ENCODE_ERROR }
              set result [set_fieldValue $handle {@randomNum} randomNum]
              if {$result != $::STRUCT_MSG_ERR_OK} { return $result }
            }
            @sequenceNum {
              set offset [::structmsg encdec sequenceNumEncode encoded $offset myDict sequenceNum]
              if {$offset < 0} { return $::STRUCT_MSG_ERR_ENCODE_ERROR }
              set result [set_fieldValue $handle {@sequenceNum} $sequenceNum]
              if {$result != $::STRUCT_MSG_ERR_OK} { return $result }
            }
            @filler {
              set offset [::structmsg encdec fillerEncode encoded $offset [dict get $fieldInfo fieldLgth] value]
              if {$offset < 0} { return $::STRUCT_MSG_ERR_ENCODE_ERROR }
              set result [set_fieldValue $handle {@filler} $value]
              if {$result != $::STRUCT_MSG_ERR_OK} { return $result }
            }
            @crc {
              set offset [::structmsg encdec crcEncode encoded $offset [dict get $myDict hdr hdrInfo hdrKeys totalLgth] crc [dict get $myDict hdr headerLgth]]
             if {$offset < 0} { return $::STRUCT_MSG_ERR_ENCODE_ERROR }
              set result [set_fieldValue $handle {@crc} $crc]
              if {$result != $::STRUCT_MSG_ERR_OK} { return $result }
            }
            @id {
              return $::STRUCT_MSG_ERR_BAD_SPECIAL_FIELD
            }
            @tablerows {
            }
            @tablerowfields {
              if {[dict get $myDict msg numTableRows] > 0} {
                set row 0
                set col 0
                set cell 0
                while {$row < [dict get $myDict msg numTableRows]} {
                  while {$col < [dict get $myDict msg numTableRowFields]} {
                     set cell [expr {$col + $row * [dict get $myDict msg numTableRowFields]}]
                     set fieldInfos [dict get $myDict msg tableFieldInfos]
                     set fieldInfo [lindex $fieldInfos $cell]
    puts stderr "fieldInfo: $fieldInfo!row: $row!col: $col!cell: $cell!"
                     set offset [::structmsg encdec encodeField encoded $fieldInfo $offset]
    puts stderr "offset1: $offset![dict get $fieldInfo fieldName]!lgth: [string length $encoded]!"
                     if {$offset < 0} { return $::STRUCT_MSG_ERR_ENCODE_ERROR }
                     incr col
                  }
                  incr row
                  set col 0
                }
              }
            }
            default {
    puts stderr "fieldId: $fieldId ($specFieldName) not in switch!"
            }
          }
        } else {
          set offset [::structmsg encdec encodeField encoded $fieldInfo $offset]
          if {$offset < 0} { return $::STRUCT_MSG_ERR_ENCODE_ERROR }
        }
    puts stderr "offset2: $offset![dict get $fieldInfo fieldName]!fieldId: $fieldId!lgth: [string length $encoded]!"
        incr idx
      }
      if {[lsearch [dict get $myDict flags] "ENCODED"] < 0} {
        dict lappend myDict flags ENCODED
      }
      dict set myDict encoded $encoded
    puts stderr "ENCODING END: offset: $offset lgth: [string length $encoded]!"
    dump_binary $encoded "ENCODED"
      set ::structmsg($handle) $myDict
      return $::STRUCT_MSG_ERR_OK
    }
    
    # ============================= get_encoded ========================
    
    proc get_encoded {handle encodedVar lgthVar} {
      upvar $encodedVar encoded
      upvar $lgthVar lgth

      if {![info exists ::structmsg($handle)]} {
        error "no such structmsg: $handle"
      }
      set myDict $::structmsg($handle)
      if {[lsearch [dict get $myDict flags] ENCODED] < 0} {
        error "handle: $handle is not encoded!"
      }
      set encoded [dict get $myDict encoded]
      set lgth [string length $encoded]
      return $::STRUCT_MSG_ERR_OK
    }
    
    # ============================= decode ========================
    
    proc decode {handle todecode} {
puts stderr "decode exi: $handle![info exists ::structmsg($handle]!"
      if {![info exists ::structmsg($handle)]} {
        error "no such structmsg: $handle"
      }
puts stderr "DECODE: lgth: [string length $todecode]!$handle!"
dump_binary $todecode "TODECODE"
      set myDict $::structmsg($handle)
      dict set myDict todecode $todecode
      set offset 0
      set offset [::structmsg encdec uint16Decode $todecode $offset src]
      dict set myDict hdr src $src
      set offset [::structmsg encdec uint16Decode $todecode $offset dst]
      dict set myDict hdr dst $dst
      set offset [::structmsg encdec uint16Decode $todecode $offset totalLgth]
      dict set myDict hdr totalLgth $totalLgth
      set offset [::structmsg encdec uint16Decode $todecode $offset cmdKey]
      dict set myDict hdr hdrInfo hdrKeys cmdKey $cmdKey
      set offset [::structmsg encdec uint16Decode $todecode $offset cmdLgth]
      dict set myDict hdr hdrInfo hdrKeys cmdLgth $cmdLgth
      set ::structmsg($handle) $myDict
      fillHdrInfo $handle
      set offset [::structmsg encdec uint8Decode $todecode $offset numEntries]
puts stderr "offset0: $offset!numEntries: $numEntries!"
      set idx 0
      while {$idx < $numEntries} {
        set myDict $::structmsg($handle) ; # needed because set_fieldValue changes the dict!!
        set fieldInfos [dict get $myDict msg fieldInfos]
        set fieldInfo [lindex $fieldInfos $idx]
        if {[string range [dict get $fieldInfo fieldName] 0 0] == "@"} {
          set result [::structmsg def getFieldNameId [dict get $fieldInfo fieldName] fieldId $::STRUCT_MSG_NO_INCR]
          set specialFieldName [dict get $::structmsg(specialFieldIds) $fieldId]
          if {$result != $::STRUCT_MSG_ERR_OK} { return $result }
puts stderr "specialFieldName: $specialFieldName!fieldId: $fieldId!"
          switch $specialFieldName {
            @src -
            @dst -
            @targetCmd -
            cmdKey -
            @cmdLgth {
puts stderr [format "funny should decode: %s" [dict get $fieldInfo fieldName]]
            }
            @randomNum {
              set offset [::structmsg encdec randomNumDecode $todecode $offset value]
puts stderr "after randomNum: offset: $offset!value: [format 0x%08x $value]"
              if {$offset < 0} { return $::STRUCT_MSG_ERR_DECODE_ERROR }
            }
            @sequenceNum {
              set offset [::structmsg encdec sequenceNumDecode $todecode $offset value]
puts stderr "after sequenceNum: offset: $offset!value: [format 0x%08x $value]"
              if {$offset < 0} { return $::STRUCT_MSG_ERR_DECODE_ERROR }
            }
            @filler {
              set offset [::structmsg encdec fillerDecode $todecode $offset [dict get $fieldInfo fieldLgth] value]
              if {$offset < 0} { return $::STRUCT_MSG_ERR_DECODE_ERROR }
            }
            @crc {
puts stderr "before crc"
              set offset [::structmsg encdec crcDecode $todecode $offset [dict get $myDict hdr hdrInfo hdrKeys cmdLgth] fieldInfo [dict get $myDict hdr headerLgth]]
puts stderr "after crc offset: $offset!cmdLgth: [dict get $myDict hdr hdrInfo hdrKeys cmdLgth]!$cmdLgth!"
              if {$offset < 0} {
                return $::STRUCT_MSG_ERR_BAD_CRC_VALUE;
              }
            }
            @id {
              return $::STRUCT_MSG_ERR_BAD_SPECIAL_FIELD;
            }
            @tablerows {
            }
            @tablerowfields {
              if {[dict get $myDict msg numTableRows] > 0} {
    puts stderr "tabelrowfields: offset: $offset!"
                set row 0
                set col 0
                set cell 0
                while {$row < [dict get $myDict msg numTableRows]} {
                  while {$col < [dict get $myDict msg numTableRowFields]} {
                     set cell [expr {$col + $row * [dict get $myDict msg numTableRowFields]}]
                     set fieldInfos [dict get $myDict msg tableFieldInfos]
puts stderr "LLTAB: [llength $fieldInfos]!"
                     set fieldInfo [lindex $fieldInfos $cell]
                     set offset [::structmsg encdec decodeField $todecode fieldInfo $offset]
    puts stderr "  offset: $offset!fieldName: [dict get $fieldInfo fieldName]!value: [dict get $fieldInfo value]!"
                     if {$offset < 0} { return $::STRUCT_MSG_ERR_DECODE_ERROR }
                     incr col
                  }
                  incr row
                  set col 0
                }
              }
            }
          }
    puts stderr "fieldInfo1: $fieldInfo!offset: $offset!"
        } else {
    puts stderr "fieldInfo2a: $fieldInfo!offset: $offset!"
          set offset [::structmsg encdec decodeField $todecode fieldInfo $offset]
    puts stderr "fieldInfo2b: [dict get $fieldInfo value]!offset: $offset!"
          if {$offset < 0} { return $::STRUCT_MSG_ERR_DECODE_ERROR }
        }
        incr idx
    puts stderr "fieldInfo3: $fieldInfo!offset: $offset!"
        if {[lsearch [dict get $fieldInfo flags] "FIELD_IS_SET"] < 0} {
          dict lappend fieldInfo flags FIELD_IS_SET
        }
        set myDict $::structmsg($handle)
        set fieldInfos [dict get $myDict msg tableFieldInfos]
        set fieldInfos [lreplace $fieldInfos $idx $idx $fieldInfo]
        dict set myDict msg tableFieldInfos $fieldInfos
      }
      dict set myDict msg fieldInfos $fieldInfos
      if {[lsearch [dict get $myDict flags] "DECODED"] < 0} {
        dict lappend myDict flags DECODED
      }
      set ::structmsg($handle) $myDict
      return $::STRUCT_MSG_ERR_OK;
    }
    
    # ===================== dump =============================
    
    proc dump {handle} {
      if {![info exists ::structmsg($handle)]} {
        error "no such structmsg: $handle"
      }
      set myDict $::structmsg($handle)
      set src [expr {[dict get $myDict hdr hdrInfo hdrKeys src] & 0xFFFF}]
      set dst [expr {[dict get $myDict hdr hdrInfo hdrKeys dst] & 0xFFFF}]
      set totalLgth [expr {[dict get $myDict hdr hdrInfo hdrKeys totalLgth] & 0xFFFF}]
      puts stderr [format "handle: %s src: %d 0x%04x dst: %d 0x%04x totalLgth: %d 0x%04x" $handle $src $src $dst $dst $totalLgth $totalLgth]
    
      set cmdKey [expr {[dict get $myDict hdr hdrInfo hdrKeys cmdKey] & 0xFFFF}]
      set cmdLgth [expr {[dict get $myDict hdr hdrInfo hdrKeys cmdLgth] & 0xFFFF}]
      puts stderr [format "  cmdKey: %d 0x%04x cmdLgth: %d 0x%04x" $cmdKey $cmdKey $cmdLgth $cmdLgth]
      set numEntries [dict get $myDict msg numFieldInfos]
      puts stderr [format "  numFieldInfos: %d max: %d" $numEntries [dict get $myDict msg maxFieldInfos]]
      set idx 0
      set fieldInfos [dict get $myDict msg fieldInfos]
      while {$idx < $numEntries} {
        set fieldInfo [lindex $fieldInfos $idx]
        puts stderr [format "    idx %d: key: %-20s type: %-8s lgth: %.5d" $idx [dict get $fieldInfo fieldName] [dict get $fieldInfo fieldType] [dict get $fieldInfo fieldLgth]]
        if {[lsearch [dict get $fieldInfo flags] "FIELD_IS_SET"] >= 0} {
          switch [dict get $fieldInfo fieldType] {
            uint8_t {
              puts stderr [format "      value: 0x%02x" [dict get $fieldInfo value]]
            }
            int8_t {
              puts stderr [format "      value: 0x%02x" [dict get $fieldInfo value]]
            }
            uint16_t {
              puts stderr [format "      value: 0x%04x" [dict get $fieldInfo value]]
            }
            int16_t {
              puts stderr [format "      value: 0x%04x" [dict get $fieldInfo value]]
            }
            uint32_t {
              puts stderr [format "      value: 0x%08x" [dict get $fieldInfo value]]
            }
            int32_t {
              puts stderr [format "      value: 0x%08x" [dict get $fieldInfo value]]
            }
            uint8_t* {
              set val [dict get $fieldInfo value]
              if {[string is print $val]} {
                puts stderr [format "      value: %s" $val]
              }
              set lgth [string length $val]
              dump_values $val
            }
            int8_t* {
              puts stderr [format "      value: %s" [dict get $fieldInfo value]]
            }
            uint16_t* {
              puts stderr [format "      value: %s" [dict get $fieldInfo value]]
            }
            int16_t* {
              puts stderr [format "      value: %s" [dict get $fieldInfo value]]
            }
            uint32_t* {
              puts stderr [format "      value: %s" [dict get $fieldInfo value]]
            }
            int32_t* {
              puts stderr [format "      value: %s" [dict get $fieldInfo value]]
            }
          }
        }
        incr idx
      }
      # encoded
      if {[lsearch [dict get $myDict flags] ENCODED] >= 0} {
        set encoded [dict get $myDict encoded]
        puts stderr "  encoded: $encoded"
        dump_values $encoded
      }
      # decoded
      if {[lsearch [dict get $myDict flags] DECODED] >= 0} {
        set todecode [dict get $myDict todecode]
        puts stderr "  todecode: $todecode"
        dump_values $todecode
      }
      # encryptedMsg
      if {[lsearch [dict get $myDict flags] ENCRYPTED] >= 0} {
        set encryptedMsg [dict get $myDict encryptedMsg]
        puts stderr "  encryptedMsg: $encryptedMsg"
        dump_values $encryptedMsg
      }
      # decryptedMsg
      if {[lsearch [dict get $myDict flags] DECRYPTED] >= 0} {
        set decryptedMsg [dict get $myDict decryptedMsg]
        puts stderr "  decryptedMsg: $decryptedMsg"
        dump_values $decryptedMsg
      }
      return $::STRUCT_MSG_ERR_OK
    }
    
    # ===================== encrypt =================================
    
    proc encrypt {handle tocrypt cryptlgth cryptKey iv bufVar lgthVar} {
      upvar $bufVar buf
      upvar lgthVar lgth

      if {![info exists ::structmsg($handle)]} {
        error "no such structmsg: $handle"
      }
      set myDict $::structmsg($handle)
      set headerLgth [dict get $myDict hdr headerLgth]
      get_encoded $handle encoded encodedLgth
      set lgth [dict get $myDict hdr hdrInfo hdrKeys cmdLgth]
      set offset $headerLgth
      set tocrypt [string range $encoded $offset [expr {$offset + $lgth - 1}]]
      set encryptedData [aes::aes -dir encrypt -key $cryptKey $tocrypt]
      set len [string length $encryptedData]
      dict set myDict encrypted $encryptedData
      dict set myDict encryptedMsg [string range $encoded 0 [expr {$headerLgth - 1}]]
      dict append myDict encryptedMsg $encryptedData
      if {[lsearch [dict get $myDict flags] "ENCRYPTED"] < 0} {
        dict lappend myDict flags ENCRYPTED
      }
      set ::structmsg($handle) $myDict
      set buf [dict get $myDict encryptedMsg]
      set lgth [string length $buf]
      return $::STRUCT_MSG_ERR_OK
    }
    
    # ===================== decrypt =================================
    
    proc decrypt {handle todecrypt cryptlgth cryptKey iv bufVar lgthVar} {
      upvar $bufVar buf
      upvar lgthVar lgth

      if {![info exists ::structmsg($handle)]} {
        error "no such structmsg: $handle"
      }
      set myDict $::structmsg($handle)
      set headerLgth [dict get $myDict hdr headerLgth]
      set lgth [dict get $myDict hdr hdrInfo hdrKeys cmdLgth]
      set offset $headerLgth
      set todecrypt [string range $crypted $offset [expr {$offset + $lgth - 1}]]
      set decryptedData [aes::aes -dir decrypt -key $cryptKey $todecrypt]
      set len [string length $decryptedData]
      dict set myDict decrypted $decryptedData
      dict set myDict decryptedMsg [string range $crypted 0 [expr {$headerLgth - 1}]]
      dict append myDict decryptedMsg $decryptedData
      if {[lsearch [dict get $myDict flags] "DECRYPTED"] < 0} {
        dict lappend myDict flags DECRYPTED
      }
      set ::structmsg($handle) $myDict
      set buf [dict get $myDict decryptedMsg]
      set lgth [string length $buf]
      return $::STRUCT_MSG_ERR_OK
    }
    
    # ===================== add_field =============================
    
    proc add_field {handle name fieldType fieldLgth} {
      set result $::STRUCT_MSG_ERR_OK
      if {![info exists ::structmsg($handle)]} {
        error "no such structmsg: $handle"
      }
      set myDict $::structmsg($handle)
      if {$name eq "@tablerows"} {
        dict set myDict msg numTableRows $fieldLgth
        set fieldInfos [dict get $myDict msg fieldInfos]
        set fieldInfoIdx [dict get $myDict msg numFieldInfos]
        set fieldInfo [list]
    #puts stderr [format "tablerows1: totalLgth: %d cmdLgth: %d" [dict get $myDict hdr hdrInfo hdrKeys totalLgth] [dict get $myDict hdr hdrInfo hdrKeys cmdLgth]]
        # we use 0 as numTableRows, that forces the *Lgth fields to NOT be modified!!
        fixHeaderInfo myDict fieldInfo $name $fieldType 0 0
        lappend fieldInfos $fieldInfo
        dict set myDict msg fieldInfos $fieldInfos
    #puts stderr [format "tablerows2: totalLgth: %d cmdLgth: %d" [dict get $myDict hdr hdrInfo hdrKeys totalLgth] [dict get $myDict hdr hdrInfo hdrKeys cmdLgth]]
        dict set myDict msg numFieldInfos [expr {[dict get $myDict msg numFieldInfos] + 1}]
        set ::structmsg($handle) $myDict
        return $::STRUCT_MSG_ERR_OK
      }
      if {$name eq "@tablerowfields"} {
        dict set myDict msg numTableRowFields $fieldLgth
        set numTableFields [expr {[dict get $myDict msg numTableRows] * [dict get $myDict msg numTableRowFields]}]
        set fieldInfos [dict get $myDict msg fieldInfos]
        set fieldInfoIdx [dict get $myDict msg numFieldInfos]
        set fieldInfo [list]
    #puts stderr [format "tablerowfields1: totalLgth: %d cmdLgth: %d" [dict get $myDict hdr hdrInfo hdrKeys totalLgth] [dict get $myDict hdr hdrInfo hdrKeys cmdLgth]]
        # we use 0 as numTableRows, that forces the *Lgth fields to NOT be modified!!
        fixHeaderInfo myDict fieldInfo $name $fieldType 0 0
        lappend fieldInfos $fieldInfo
        dict set myDict msg fieldInfos $fieldInfos
    #puts stderr [format "tablerowfields2: totalLgth: %d cmdLgth: %d" [dict get $myDict hdr hdrInfo hdrKeys totalLgth] [dict get $mmyDict hdr hdrInfo hdrKeys cmdLgth]]
        if {([dict get $myDict msg tableFieldInfos] eq [list]) && ($numTableFields != 0)} {
          # build dummy entries
          set cellIdx 0
          set fieldInfos [list]
          while {$cellIdx < $numTableFields} {
            lappend fieldInfos [list]
            incr cellIdx
          }
          dict set myDict msg tableFieldInfos $fieldInfos
        }
        dict set myDict msg numFieldInfos [expr {[dict get $myDict msg numFieldInfos] + 1}]
        set ::structmsg($handle) $myDict
        return $::STRUCT_MSG_ERR_OK
      }
      set numTableRowFields [dict get $myDict msg numTableRowFields]
      set numTableRows [dict get $myDict msg numTableRows]
      set numTableFields [expr {$numTableRows * $numTableRowFields}]
    
      if {!(($numTableFields > 0) && ([dict get $myDict msg numRowFields] < $numTableRowFields))} {
        if {[dict get $myDict msg numFieldInfos] >= [dict get $myDict msg maxFieldInfos]} {
          return $::STRUCT_MSG_ERR_TOO_MANY_FIELDS
        }
        set fieldInfos [dict get $myDict msg fieldInfos]
        set fieldInfoIdx [dict get $myDict msg numFieldInfos]
        set fieldInfo [list]
        set numTableFields 0
        set numTableRows 1
        set numTableRowFields 0
        fixHeaderInfo myDict fieldInfo $name $fieldType $fieldLgth $numTableRows
    #puts stderr [format "field2: %s totalLgth: %d cmdLgth: %d" [dict get $fieldInfo fieldName] [dict get $myDict hdr hdrInfo hdrKeys totalLgth] [dict get $myDict hdr hdrInfo hdrKeys cmdLgth]]
        lappend fieldInfos $fieldInfo
        dict set myDict msg fieldInfos $fieldInfos
        dict set myDict msg numFieldInfos [expr {[dict get $myDict msg numFieldInfos] + 1}]
        set ::structmsg($handle) $myDict
        set result [::structmsg encdec fillHdrInfo $handle]
      } else {
        set numRowFields [dict get $myDict msg numRowFields]
        set cellIdx $numRowFields
        # fill the row fields until we heve filled up one row
        # at that point add automatically the other rows!!
        if {$cellIdx < $numTableRowFields} {
          set fieldInfos [dict get $myDict msg tableFieldInfos]
          set fieldInfo [lindex $fieldInfos $cellIdx]
    #puts stderr [format "table field1: %s totalLgth: %d cmdLgth: %d" [dict get $fieldInfo fieldName] [dict get $myDict hdr hdrInfo hdrKeys totalLgth] [dict get $myDict hdr hdrInfo hdrKeys cmdLgth]]
          fixHeaderInfo myDict fieldInfo $name $fieldType $fieldLgth 1
          set fieldInfos [lreplace $fieldInfos $cellIdx $cellIdx $fieldInfo]
          dict set myDict msg tableFieldInfos $fieldInfos
    #puts stderr [format "table field2: %s totalLgth: %d cmdLgth: %d" [dict get $fieldInfo fieldName] [dict get $myDict hdr hdrInfo hdrKeys totalLgth] [dict get $myDict hdr hdrInfo hdrKeys cmdLgth]]
          dict set myDict msg numRowFields [expr {[dict get $myDict msg numRowFields] + 1}]
          set ::structmsg($handle) $myDict
        } 
        if {[dict get $myDict msg numRowFields] >= $numTableRowFields} {
          # one row ist filled, add the other rows
          set row 1
          set cellIdx $numTableRowFields
          while {$row < $numTableRows} {
            set col 0
            while {$col < $numTableRowFields} {
              set fieldInfos [dict get $myDict msg tableFieldInfos]
              set fieldInfo [lindex $fieldInfos $col]
    #puts stderr [format "table field1: %s totalLgth: %d cmdLgth: %d" [dict get $fieldInfo fieldName] [dict get $myDict hdr hdrInfo hdrKeys totalLgth] [dict get $myDict hdr hdrInfo hdrKeys cmdLgth]]
              set name [dict get $fieldInfo fieldName]
              set fieldType [dict get $fieldInfo fieldType]
              set fieldLgth [dict get $fieldInfo fieldLgth]
              fixHeaderInfo myDict fieldInfo $name $fieldType $fieldLgth 1
              set fieldInfos [lreplace $fieldInfos $cellIdx $cellIdx $fieldInfo]
              dict set myDict msg tableFieldInfos $fieldInfos
    #puts stderr [format "table field2: %s totalLgth: %d cmdLgth: %d" [dict get $fieldInfo fieldName] [dict get $myDict hdr hdrInfo hdrKeys totalLgth] [dict get $myDict hdr hdrInfo hdrKeys cmdLgth]]
              dict set myDict msg numRowFields [expr {[dict get $myDict msg numRowFields] + 1}]
              set ::structmsg($handle) $myDict
              incr col
              incr cellIdx
            }
            incr row
          }
        }
      }
      set ::structmsg($handle) $myDict
      return $result
    }
    
    # ===================== set_fillerAndCrc =============================
    
    proc set_fillerAndCrc {handle} {
      if {![info exists ::structmsg($handle)]} {
        error "no such structmsg: $handle"
      }
      set myDict $::structmsg($handle)
      set hdrDict [dict get $myDict hdr]
      # for the internal numEntries field
puts stderr "set_fillerAndCrc: cmdLgth: dict get $hdrDict hdrInfo hdrKeys cmdLgth]!"
      dict set hdrDict hdrInfo hdrKeys cmdLgth [expr {[dict get $hdrDict hdrInfo hdrKeys cmdLgth] + 1}]
      dict set hdrDict hdrInfo hdrKeys totalLgth [expr {[dict get $hdrDict hdrInfo hdrKeys totalLgth] + 1}]
      # end for the internal numEntries field
      set fillerLgth 0
      set myLgth [expr {[dict get $hdrDict hdrInfo hdrKeys cmdLgth] + 2}]
      while {[expr {$myLgth % 16}] != 0} {
        incr myLgth
        incr fillerLgth
      }
      dict set myDict hdr $hdrDict
      set ::structmsg($handle) $myDict
puts stderr "fillerLgth: $fillerLgth!"
      add_field $handle "@filler" uint8_t* $fillerLgth
      add_field $handle "@crc" uint16_t 1
    }
    
    # ===================== setFieldValue =============================
    
    proc setFieldValue {fieldInfoVar fieldName value} {
      upvar $fieldInfoVar fieldInfo
      set fieldType [dict get $fieldInfo fieldType]
      switch $fieldType {
        int8_t {
          if {[string is integer $value]} {
            if {($value > -128) && ($value < 128)} {
              dict set fieldInfo value $value
            } else {
              error "value ($value)too small/big for type: $fieldType"
            }
          } else {
            error "need integer for type: $fieldType"
          }
        }
        uint8_t {
          if {[string is integer $value]} {
            if {($value >= 0) && ($value < 255)} {
              dict set fieldInfo value $value
            } else {
              error "value ($value)too small/big for type: $fieldType"
            }
          } else {
            error "need integer for type: $fieldType"
          }
        }
        int16_t {
          if {[string is integer $value]} {
            if {($value > -32767) && ($value < 32767)} {
              dict set fieldInfo value $value
            } else {
              error "value ($value)too small/big for type: $fieldType"
            }
          } else {
            error "need integer for type: $fieldType"
          }
        }
        uint16_t {
          if {[string is integer $value]} {
            if {($value >= 0) && ($value <= 65535)} {
              dict set fieldInfo value $value
            } else {
              error "value ($value)too small/big for type: $fieldType"
            }
          } else {
            error "need integer for type: $fieldType"
          }
        }
        int32_t {
          if {[string is integer $value]} {
            if {($value > -0x7FFFFFFFFF) && ($value < 0x7FFFFFFF)} {
              dict set fieldInfo value $value
            } else {
              error "value ($value)too small/big for type: $fieldType"
            }
          } else {
            error "need integer for type: $fieldType"
          }
        }
        uint32_t {
          if {![string is integer $value]} {
            binary scan $value I val
            set value $val
          }
          if {[string is integer $value]} {
            if {($value >= 0) && ($value <= 0xFFFFFFFF)} {
              dict set fieldInfo value $value
            } else {
              error "value ($value)too small/big for type: $fieldType"
            }
          } else {
            error "need integer for type: $fieldType"
          }
        }
        int8_t* {
          if {[string length $value] == [expr {[dict get $fieldInfo fieldLgth] * 2}]} {
            dict set fieldInfo value $value
          } else {
            error "field is too short/long: $fieldName: $fieldType [string length $value] should be [dict get $fieldInfo fieldLgth]!"
          }
        }
        uint8_t* {
          if {([string length $value] == [dict get $fieldInfo fieldLgth]) || ($fieldName eq "@filler")} {
            dict set fieldInfo value $value
          } else {
            error "field is too short/long: $fieldName: $fieldType [string length $value] should be [dict get $fieldInfo fieldLgth]!"
          }
        }
        int16_t* {
          if {[string length $value] == [expr {[dict get $fieldInfo fieldLgth] * 2}]} {
            dict set fieldInfo value $value
          } else {
            error "field is too short/long: $fieldName: $fieldType [string length $value] should be [dict get $fieldInfo fieldLgth]!"
          }
        }
        uint16_t* {
          if {[string length $value] == [expr {[dict get $fieldInfo fieldLgth] * 2}]} {
            dict set fieldInfo value $value
          } else {
            error "field is too short/long: $fieldName: $fieldType [string length $value] should be [dict get $fieldInfo fieldLgth]!"
          }
        }
        int32_t* {
          if {[string length $value] == [expr {[dict get $fieldInfo fieldLgth] * 4}]} {
            dict set fieldInfo value $value
          } else {
            error "field is too short/long: $fieldName: $fieldType [string length $value] should be [dict get $fieldInfo fieldLgth]!"
          }
        }
        uint32_t* {
          if {[string length $value] == [expr {[dict get $fieldInfo fieldLgth] * 4}]} {
            dict set fieldInfo value $value
          } else {
            error "field is too short/long: $fieldName: $fieldType [string length $value] should be [dict get $fieldInfo fieldLgth]!"
          }
        }
      }
      if {[lsearch [dict get $fieldInfo flags] FIELD_IS_SET] < 0} {
        dict lappend fieldInfo flags FIELD_IS_SET
      }
      return $::STRUCT_MSG_ERR_OK
    }
    
    # ===================== set_fieldValue =============================
    
    proc set_fieldValue {handle fieldName value} {
puts stderr "set_fieldValue: $handle $fieldName $value!"
      if {![info exists ::structmsg($handle)]} {
        error "no such structmsg: $handle"
      }
      set myDict $::structmsg($handle)
      switch $fieldName {
        @src {
          dict set myDict hdr src $value
          set ::structmsg($handle) $myDict
          setHandleField $handle src $value
          fillHdrInfo $handle
          return $::STRUCT_MSG_ERR_OK
        }
        @dst {
          dict set myDict hdr dst $value
          set ::structmsg($handle) $myDict
          setHandleField $handle dst $value
          fillHdrInfo $handle
          return $::STRUCT_MSG_ERR_OK
        }
        @cmdKey {
          dict set myDict hdr hdrInfo hdrKeys cmdKey $value
          set ::structmsg($handle) $myDict
          setHandleField $handle cmdKey $value
          fillHdrInfo $handle
          return $::STRUCT_MSG_ERR_OK
        }
      }
      set numEntries [dict get $myDict msg numFieldInfos]
      set idx 0
      set fieldInfos [dict get $myDict msg fieldInfos]
      while {$idx < $numEntries} {
        set fieldInfo [lindex $fieldInfos $idx]
        if {$fieldName eq [dict get $fieldInfo fieldName]} {
          set result [setFieldValue fieldInfo $fieldName $value]
          set fieldInfos [lreplace $fieldInfos $idx $idx $fieldInfo]
          dict set myDict msg fieldInfos $fieldInfos
          set ::structmsg($handle) $myDict
          return $result
        }
        incr idx
      }
      error "field $fieldName not found"
    }
    
    # ============================= set_tableFieldValue ========================
    
    proc set_tableFieldValue {handle fieldName row value} {
puts stderr "set_tableFieldValue: $handle $fieldName $value!"
      if {![info exists ::structmsg($handle)]} {
        error "no such structmsg: $handle"
      }
      set myDict $::structmsg($handle)
      if {$row >= [dict get $myDict msg numTableRows]} {
        return $::STRUCT_MSG_ERR_BAD_TABLE_ROW
      }
      set idx 0
      set cell [expr {0 + $row * [dict get $myDict msg numTableRowFields]}]
      set tableFieldInfos [dict get $myDict msg tableFieldInfos]
      while {$idx < [dict get $myDict msg numRowFields]} {
        set fieldInfo [lindex $tableFieldInfos $cell]
        if {$fieldName eq [dict get $fieldInfo fieldName]} {
          set result [setFieldValue fieldInfo $fieldName $value]
          set tableFieldInfos [lreplace $tableFieldInfos $cell $cell $fieldInfo]
          dict set myDict msg tableFieldInfos $tableFieldInfos
          set ::structmsg($handle) $myDict
          return $result
        }
        incr idx
        incr cell
      }
      return $::STRUCT_MSG_ERR_FIELD_NOT_FOUND
    }
    
    # ===================== get_fieldValue =============================
    
    proc get_fieldValue {handle fieldName val} {
      upvar $val value
    
      if {![info exists ::structmsg($handle)]} {
        error "no such structmsg: $handle"
      }
      set myDict $::structmsg($handle)
      switch $fieldName {
        @src {
          set value [ dict get $myDict hdr src]
          return
        }
        @dst {
          set value [dict get $myDict hdr dst]
          return
        }
        @totalLgth {
          set value [dict get $myDict hdr totalLgth]
          return
        }
        @cmdKey {
          set value [dict get $myDict hdr hdrInfo hdrKeys cmdKey]
          return
        }
        @cmdLgth {
          set value [dict get $myDict hdr hdrInfo hdrKeys  cmdLgth]
          return
        }
      }
      set numEntries [dict get $myDict msg numFieldInfos]
      set idx 0
      set fieldInfos [dict get $myDict msg fieldInfos]
      while {$idx < $numEntries} {
        set fieldInfo [lindex $fieldInfos $idx]
        if {$fieldName eq [dict get $fieldInfo fieldName]} {
          if {[lsearch [dict get $fieldInfo flags] FIELD_IS_SET] < 0} {
            error "field: $fieldName is not set!"
          }
          set value [dict get $fieldInfo value]
          return
        }
        incr idx
      }
      error "field $fieldName not found"
    }
    
    # ============================= get_tableFieldValue ========================
    
    proc get_tableFieldValue {handle fieldName row valueVar} {
      upvar $valueVar value
    
      if {![info exists ::structmsg($handle)]} {
        error "no such structmsg: $handle"
      }
      set myDict $::structmsg($handle)
      if {$row >= [dict get $myDict msg numTableRows]} {
        return $::STRUCT_MSG_ERR_BAD_TABLE_ROW
      }
      set idx 0
      set cell = [expr {0 + $row * [dict get $myDict msg numRowFields]}]
      set tableFieldInfos [dict get $myDict msg tableFieldInfos]
      while {$idx < [dict get $myDict msg numRowFields]} {
        set fieldInfo [lindex $tableFieldInfos cell]
        if {$fieldName eq [dict get $fieldInfo fieldName]} {
          set value [dict get $fieldInfo value]
          return $::STRUCT_MSG_ERR_OK
        }
        incr idx
        incr cell
      }
      return $::STRUCT_MSG_ERR_FIELD_NOT_FOUND
    }
    
    # ===================== set_crypted =============================
    
    proc set_crypted {handle crypted} {
      upvar $val value
    
      if {![info exists ::structmsg($handle)]} {
        error "no such structmsg: $handle"
      }
      set myDict $::structmsg($handle)
      dict set myDict encryptedMsg $crypted
      set ::structmsg($handle) $myDict
      return $::STRUCT_MSG_ERR_OK
    }
    
    # ===================== decrypt_getHandle =============================
    
    proc decrypt_getHandle {crypted cryptKey iv handleVal} {
      upvar $handleVal handle
    
      set lgth [string length $crypted]
      set hdr [string range $crypted 0 [expr {$::STRUCT_MSG_HEADER_LENGTH - 1}]]
      set offset $::STRUCT_MSG_HEADER_LENGTH
      set todecrypt [string range $crypted $offset [expr {$offset + $lgth - 1}]]
      set decryptedData [aes::aes -dir decrypt -key $cryptKey $todecrypt]
      append hdr $decryptedData
      set len [string length $hdr]
      set result [getHandle $hdr handle]
puts stderr "RES: $result!"
      return $result
    }

  } ; # namespace cmd

} ; # namespace structmsg
