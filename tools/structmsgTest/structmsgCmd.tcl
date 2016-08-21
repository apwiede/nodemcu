source pdict.tcl
source structmsgEncodeDecode.tcl

package require aes

set ::structmsg(prefix) "stmsg_"
set ::structmsg(numHandles) 1
set ::structmsg(handles) [list]

set ::headerLgth 6; # uint16_t src + uint16_t dst + uint16_t totalLgth
set ::cmdLgth 4;    # uint16_t cmdKey + unit16_t cmdLgth

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
  foreach ch [split $data ""] {
    binary scan $ch c pch
    set pch [expr {$pch & 0xFF}]
    puts stderr [format "idx: 0x%02x" $pch]
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
      set offset [uint16Encode hdrId $offset [dict get $hdrInfo hdrKeys src]]
      set offset [uint16Encode hdrId $offset [dict get $hdrInfo hdrKeys dst]]
      set offset [uint16Encode hdrId $offset [dict get $hdrInfo hdrKeys totalLgth]]
      set offset [uint16Encode hdrId $offset [dict get $hdrInfo hdrKeys cmdKey]]
      set offset [uint16Encode hdrId $offset [dict get $hdrInfo hdrKeys cmdLgth]]
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
  set hdrId [string range $decrypted 0 [expr {$::headerLgth + 4 - 1}]] ; # +4: cmdKey + cmdLgth!
  foreach handle2Hdr $myHandles {
    if {[dict get $handle2Hdr hdrInfo hdrId] eq $hdrId} {
      set handle [dict get $handle2Hdr handle]
      break
    }
    incr idx
  }
}

# ===================== structmsg_create =============================

proc structmsg_create {numFieldInfos} {

  set myDict [dict create]
  dict set myDict hdr headerLgth $::headerLgth
  dict set myDict hdr totalLgth [expr {$::headerLgth + $::cmdLgth}]
  dict set myDict msg cmdLgth $::cmdLgth
  dict set myDict msg fieldInfos [list]
  dict set myDict msg maxFieldInfos $numFieldInfos
  dict set myDict msg numFieldInfos 0
  dict set myDict flags [list]
  dict set myDict encoded [list]
  dict set myDict todecode [list]
  dict set myDict encrypted [list]
  dict set myDict encryptedMsg [list]
  dict set myDict decrypted [list]
  dict set myDict decryptedMsg [list]
  dict set myDict sequenceNum 0
  set handle $::structmsg(prefix)ffef[format "%04d" $::structmsg(numHandles)]
  dict set myDict handle $handle
  addHandle $handle
  set ::structmsg($handle) $myDict
  setHandleField $handle totalLgth [expr {$::headerLgth + $::cmdLgth}]
  setHandleField $handle cmdLgth $::cmdLgth
  fillHdrInfo $handle
  return $handle
}

# ===================== structmsg_delete =============================

proc structmsg_delete {handle} {
  # FIXME !! TBD
}

# ============================= structmsg_encode ========================

proc structmsg_encode {handle} {
  if {![info exists ::structmsg($handle)]} {
    error "no such structmsg: $handle"
  }
  set myDict $::structmsg($handle)
  dict set myDict encoded [list]
  set encoded [list]
  set offset 0
  set offset [uint16Encode encoded $offset [dict get $myDict hdr src]]
  set offset [uint16Encode encoded $offset [dict get $myDict hdr dst]]
  set offset [uint16Encode encoded $offset [dict get $myDict hdr totalLgth]]
  set offset [uint16Encode encoded $offset [dict get $myDict msg cmdKey]]
  set offset [uint16Encode encoded $offset [dict get $myDict msg cmdLgth]]
  dict set myDict encoded $encoded
  set ::structmsg($handle) $myDict
  set idx 0
  set numEntries [dict get $myDict msg numFieldInfos]
  while {$idx < $numEntries} {
    set myDict $::structmsg($handle) ; # needed because structmsg_set_fieldValue changes the dict!!
    set fieldInfos [dict get $myDict msg fieldInfos]
    set fieldInfo [lindex $fieldInfos $idx]
    if {[string range [dict get $fieldInfo fieldStr] 0 0] == "@"} {
      set fieldName [dict get $fieldInfo fieldStr]
      switch $fieldName {
        "@randomNum" {
          set offset [randomNumEncode encoded $offset randomNum]
          structmsg_set_fieldValue $handle "@randomNum" $randomNum
        }
        "@sequenceNum" {
          set offset [sequenceNumEncode encoded $offset myDict sequenceNum]
          set ::structmsg($handle) $myDict ; # needed because structmsg_set_fieldValue changes the dict!!
          structmsg_set_fieldValue $handle "@sequenceNum" $sequenceNum
        }
        "@filler" {
          set offset [fillerEncode encoded $offset [dict get $fieldInfo fieldLgth] value]
          structmsg_set_fieldValue $handle "@filler" $value
        }
        "@crc" {
          set offset [crcEncode encoded $offset [dict get $myDict msg cmdLgth] crc [dict get $myDict hdr headerLgth]]
          structmsg_set_fieldValue $handle "@crc" $crc
        }
        default {
          error "BAD_SPECIAL_FIELD: $fieldName!"
        }
      }
      set myDict $::structmsg($handle) ; # needed because structmsg_set_fieldValue changes the dict!!
      set fieldInfos [dict get $myDict msg fieldInfos]
      set fieldInfo [lindex $fieldInfos $idx]
    } else {
     set fieldType [dict get $fieldInfo fieldType]
      set fieldName [dict get $fieldInfo fieldStr]
      switch $fieldType {
        int8_t {
          set offset [int8Encode encoded $offset [dict get $fieldInfo value]]
        }
        uint8_t {
          set offset [uint8Encode encoded $offset [dict get $fieldInfo value]]
        }
        int16_t {
          set offset [int16Encode encoded $offset [dict get $fieldInfo value]]
        }
        uint16_t {
          set offset [uint16Encode encoded $offset [dict get $fieldInfo value]]
        }
        int32_t {
          set offset [int32Encode encoded $$offset [dict get $fieldInfo value]]
        }
        uint32_t {
          set offset [uint32Encode encoded $offset [dict get $fieldInfo value]]
        }
        int8_t* {
          set fieldIdx 0
          while {$fieldIdx < [dict get $fieldInfo fieldLgth]} {
            set offset [int8Encode encoded $offset [lindex [dict get $fieldInfo value] $fieldIdx]]
            incr fieldIdx
          }
        }
        uint8_t* {
          append encoded [dict get $fieldInfo value]
          set offset [expr {$offset + [dict get $fieldInfo fieldLgth]}]
        }
        int16_t* {
          set fieldIdx 0
          while {$fieldIdx < [dict get $fieldInfo fieldLgth]} {
            set offset [int16Encode encoded $offset [lindex [dict get $fieldInfo value] $fieldIdx]]
            incr fieldIdx
          }
        }
        uint16_t* {
          set fieldIdx 0
          while {$fieldIdx < [dict get $fieldInfo fieldLgth]} {
            set offset [uint16Encode encoded $offset [lindex [dict get $fieldInfo value] $fieldIdx]]
            incr fieldIdx
          }
        }
        int32_t* {
          set fieldIdx 0
          while {$fieldIdx < [dict get $fieldInfo fieldLgth]} {
            set offset [int32Encode encoded $offset [lindex [dict get $fieldInfo value] $fieldIdx]]
            incr fieldIdx
          }
        }
        uint32_t* {
          set fieldIdx 0
          while {$fieldIdx < [dict get $fieldInfo fieldLgth]} {
            set offset [uint32Encode encoded $offset [lindex [dict get $fieldInfo value] $fieldIdx]]
            incr fieldIdx
          }
        }
      }
    }
    set myDict $::structmsg($handle)
    set fieldInfos [dict get $myDict msg fieldInfos]
    set fieldInfos [lreplace $fieldInfos $idx $idx $fieldInfo]
    dict set myDict msg fieldInfos $fieldInfos
    dict set myDict encoded $encoded
    set ::structmsg($handle) $myDict
    incr idx
  }
  set myDict $::structmsg($handle)
  dict set myDict msg fieldInfos $fieldInfos
  if {[lsearch [dict get $myDict flags] "ENCODED"] < 0} {
    dict lappend myDict flags ENCODED
  }
  dict set myDict encoded $encoded
  set ::structmsg($handle) $myDict
}

# ============================= structmsg_get_encoded ========================

proc structmsg_get_encoded {handle} {
  if {![info exists ::structmsg($handle)]} {
    error "no such structmsg: $handle"
  }
  set myDict $::structmsg($handle)
  if {[lsearch [dict get $myDict flags] ENCODED] < 0} {
    error "handle: $handle is not encoded!"
  }
  return [dict get $myDict encoded]
}


# ============================= structmsg_get_encodedMsg ========================

proc structmsg_get_encodedMsg {handle} {
  if {![info exists ::structmsg($handle)]} {
    error "no such structmsg: $handle"
  }
  set myDict $::structmsg($handle)
  if {[lsearch [dict get $myDict flags] ENCODED] < 0} {
    error "handle: $handle is not encoded!"
  }
  set encoded [dict get $myDict encoded]

}

# ============================= strucmsg_decode ========================

proc structmsg_decode {handle todecode} {
  if {![info exists ::structmsg($handle)]} {
    error "no such structmsg: $handle"
  }
  set myDict $::structmsg($handle)
  dict set myDict todecode $todecode
  set offset 0
  set offset [uint16Decode $todecode $offset src]
  dict set myDict hdr src $src
  set offset [uint16Decode $todecode $offset dst]
  dict set myDict hdr dst $dst
  set offset [uint16Decode $todecode $offset totalLgth]
  dict set myDict hdr totalLgth $totalLgth
  set offset [uint16Decode $todecode $offset cmdKey]
  dict set myDict msg cmdKey $cmdKey
  set offset [uint16Decode $todecode $offset cmdLgth]
  dict set myDict msg cmdLgth $cmdLgth
  set ::structmsg($handle) $myDict
  fillHdrInfo $handle
  set idx 0
  set numEntries [dict get $myDict msg numFieldInfos]
  while {$idx < $numEntries} {
    set myDict $::structmsg($handle) ; # needed because structmsg_set_fieldValue changes the dict!!
    set fieldInfos [dict get $myDict msg fieldInfos]
    set fieldInfo [lindex $fieldInfos $idx]
    if {[string range [dict get $fieldInfo fieldStr] 0 0] == "@"} {
      set fieldName [dict get $fieldInfo fieldStr]
      switch $fieldName {
        "@randomNum" {
          set offset [randomNumDecode $todecode $offset randomNum]
          dict set fieldInfo value $randomNum
        }
        "@sequenceNum" {
          set offset [sequenceNumDecode $todecode $offset sequenceNum]
          dict set fieldInfo value $sequenceNum
        }
        "@filler" {
          set offset [fillerDecode $todecode $offset [dict get $fieldInfo fieldLgth] value]
          dict set fieldInfo value $value
        }
        "@crc" {
          set offset [crcDecode $todecode $offset [dict get $myDict msg cmdLgth] crc [dict get $myDict hdr headerLgth]]
          dict set fieldInfo value $crc
        }
        default {
          error "BAD_SPECIAL_FIELD: $fieldName!"
        }
      }
      set myDict $::structmsg($handle) ; # needed because structmsg_set_fieldValue changes the dict!!
      set fieldInfos [dict get $myDict msg fieldInfos]
      set fieldInfos [lreplace $fieldInfos $idx $idx $fieldInfo]
      dict set myDict msg fieldInfos $fieldInfos
      set ::structmsg($handle) $myDict
    } else {
      set fieldType [dict get $fieldInfo fieldType]
      set fieldName [dict get $fieldInfo fieldStr]
      switch $fieldType {
        int8_t {
          set offset [int8Decode $todecode $offset [dict get $fieldInfo value]]
          dict set fieldInfo value $value
        }
        uint8_t {
          set offset [uint8Decode $todecode $offset [dict get $fieldInfo value]]
          dict set fieldInfo value $value
        }
        int16_t {
          set offset [int16Decode $todecode $offset [dict get $fieldInfo value]]
          dict set fieldInfo value $value
        }
        uint16_t {
          set offset [uint16Decode $todecode $offset [dict get $fieldInfo value]]
          dict set fieldInfo value $value
        }
        int32_t {
          set offset [int32Decode $todecode $$offset [dict get $fieldInfo value]]
          dict set fieldInfo value $value
        }
        uint32_t {
          set offset [uint32Decode $todecode $offset [dict get $fieldInfo value]]
          dict set fieldInfo value $value
        }
        int8_t* {
          set fieldIdx 0
          while {$fieldIdx < [dict get $fieldInfo fieldLgth]} {
            set offset [int8Decode $todecode $offset [lindex [dict get $fieldInfo value] $fieldIdx]]
            incr fieldIdx
          }
        }
        uint8_t* {
          set lgth [dict get $fieldInfo fieldLgth]
          dict set fieldInfo value [string range $todecode $offset [expr {$offset + $lgth - 1}]]
          incr offset $lgth
        }
        int16_t* {
          set fieldIdx 0
          while {$fieldIdx < [dict get $fieldInfo fieldLgth]} {
            set offset [int16Decode $todecode $offset [lindex [dict get $fieldInfo value] $fieldIdx]]
            incr fieldIdx
          }
        }
        uint16_t* {
          set fieldIdx 0
          while {$fieldIdx < [dict get $fieldInfo fieldLgth]} {
            set offset [uint16Decode $todecode $offset [lindex [dict get $fieldInfo value] $fieldIdx]]
            incr fieldIdx
          }
        }
        int32_t* {
          set fieldIdx 0
          while {$fieldIdx < [dict get $fieldInfo fieldLgth]} {
            set offset [int32Decode $todecode $offset [lindex [dict get $fieldInfo value] $fieldIdx]]
            incr fieldIdx
          }
        }
        uint32_t* {
          set fieldIdx 0
          while {$fieldIdx < [dict get $fieldInfo fieldLgth]} {
            set offset [uint32Decode $todecode $offset [lindex [dict get $fieldInfo value] $fieldIdx]]
            incr fieldIdx
          }
        }
      }
    }
    if {[lsearch [dict get $fieldInfo flags] "FIELD_IS_SET"] < 0} {
      dict lappend fieldInfo flags FIELD_IS_SET
    }
    set myDict $::structmsg($handle)
    set fieldInfos [dict get $myDict msg fieldInfos]
    set fieldInfos [lreplace $fieldInfos $idx $idx $fieldInfo]
    dict set myDict msg fieldInfos $fieldInfos
    set ::structmsg($handle) $myDict
    incr idx
  }
  set myDict $::structmsg($handle)
  dict set myDict msg fieldInfos $fieldInfos
  if {[lsearch [dict get $myDict flags] "DECODED"] < 0} {
    dict lappend myDict flags DECODED
  }
  set ::structmsg($handle) $myDict
}

# ===================== structmsg_dump =============================

proc structmsg_dump {handle} {
  if {![info exists ::structmsg($handle)]} {
    error "no such structmsg: $handle"
  }
  set myDict $::structmsg($handle)
  set src [expr {[dict get $myDict hdr src] & 0xFFFF}]
  set dst [expr {[dict get $myDict hdr dst] & 0xFFFF}]
  set totalLgth [expr {[dict get $myDict hdr totalLgth] & 0xFFFF}]
  puts stderr [format "handle: %s src: %d 0x%04x dst: %d 0x%04x totalLgth: %d 0x%04x" $handle $src $src $dst $dst $totalLgth $totalLgth]

  set cmdKey [expr {[dict get $myDict msg cmdKey] & 0xFFFF}]
  set cmdLgth [expr {[dict get $myDict msg cmdLgth] & 0xFFFF}]
  puts stderr [format "  cmdKey: %d 0x%04x cmdLgth: %d 0x%04x" $cmdKey $cmdKey $cmdLgth $cmdLgth]
  set numEntries [dict get $myDict msg numFieldInfos]
  puts stderr [format "  numFieldInfos: %d max: %d" $numEntries [dict get $myDict msg maxFieldInfos]]
  set idx 0
  set fieldInfos [dict get $myDict msg fieldInfos]
  while {$idx < $numEntries} {
    set fieldInfo [lindex $fieldInfos $idx]
    puts stderr [format "    idx %d: key: %-20s type: %-8s lgth: %.5d" $idx [dict get $fieldInfo fieldStr] [dict get $fieldInfo fieldType] [dict get $fieldInfo fieldLgth]]
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
}

# ===================== structmsg_encrypt =================================

proc structmsg_encrypt {handle cryptKey} {
  if {![info exists ::structmsg($handle)]} {
    error "no such structmsg: $handle"
  }
  set myDict $::structmsg($handle)
  set headerLgth [dict get $myDict hdr headerLgth]
  set encoded [structmsg_get_encoded $handle]
  set lgth [dict get $myDict msg cmdLgth]
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
  return [dict get $myDict encryptedMsg]
}

# ===================== structmsg_decrypt =================================

proc structmsg_decrypt {handle cryptKey crypted} {
  if {![info exists ::structmsg($handle)]} {
    error "no such structmsg: $handle"
  }
  set myDict $::structmsg($handle)
  set headerLgth [dict get $myDict hdr headerLgth]
  set lgth [dict get $myDict msg cmdLgth]
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
  return [dict get $myDict decryptedMsg]
}

# ===================== structmsg_add_field =============================

proc structmsg_add_field {handle name fieldType fieldLgth} {
  if {![info exists ::structmsg($handle)]} {
    error "no such structmsg: $handle"
  }
  set myDict $::structmsg($handle)
  set fieldKey [dict get $myDict msg numFieldInfos]
  set fieldInfos [dict get $myDict msg fieldInfos]
  set fieldInfo [dict create]
  dict set fieldInfo fieldStr $name
  dict set fieldInfo fieldKey $fieldKey
  dict set fieldInfo fieldType $fieldType;
  dict set fieldInfo value [list]
  dict set fieldInfo flags [list]
  switch $fieldType {
    uint8_t -
    int8_t {
      dict set myDict hdr totalLgth [expr {[dict get $myDict hdr totalLgth] + 1}]
      dict set myDict msg cmdLgth [expr {[dict get $myDict msg cmdLgth] + 1}]
      dict set fieldInfo fieldLgth 1
    }
    uint16_t -
    int16_t {
      dict set myDict hdr totalLgth [expr {[dict get $myDict hdr totalLgth] + 2}]
      dict set myDict msg cmdLgth [expr {[dict get $myDict msg cmdLgth] + 2}]
      dict set fieldInfo fieldLgth 2
    }
    uint32_t -
    int32_t {
      dict set myDict hdr totalLgth [expr {[dict get $myDict hdr totalLgth] + 4}]
      dict set myDict msg cmdLgth [expr {[dict get $myDict msg cmdLgth] + 4}]
      dict set fieldInfo fieldLgth 4
    }
    uint8_t* -
    int8_t* {
      dict set myDict hdr totalLgth [expr {[dict get $myDict hdr totalLgth] + $fieldLgth}]
      dict set myDict msg cmdLgth [expr {[dict get $myDict msg cmdLgth] + $fieldLgth}]
      dict set fieldInfo fieldLgth $fieldLgth
    }
    uint16_t* -
    int16_t* {
      dict set myDict hdr totalLgth [expr {[dict get $myDict hdr totalLgth] + [expr {$fieldLgth * 2}]}]
      dict set myDict msg cmdLgth [expr {[dict get $myDict msg cmdLgth] + [expr {$fieldLgth * 2}]}]
      dict set fieldInfo fieldLgth $fieldLgth
    }
    uint32_t* -
    int32_t* {
      dict set myDict hdr totalLgth [expr {[dict get $myDict hdr totalLgth] + [expr {$fieldLgth * 4}]}]
      dict set myDict msg cmdLgth [expr {[dict get $myDict msg cmdLgth] + [expr {$fieldLgth * 4}]}]
      dict set fieldInfo fieldLgth $fieldLgth
    }
  } 
  lappend fieldInfos $fieldInfo
  dict set myDict msg fieldInfos $fieldInfos
  dict set myDict msg numFieldInfos [expr {[dict get $myDict msg numFieldInfos] + 1}]
  set ::structmsg($handle) $myDict
  setHandleField $handle totalLgth [dict get $myDict hdr totalLgth]
  setHandleField $handle cmdLgth [dict get $myDict msg cmdLgth]
  fillHdrInfo $handle
}

# ===================== structmsg_set_fillerAndCrc =============================

proc structmsg_set_fillerAndCrc {handle} {
  if {![info exists ::structmsg($handle)]} {
    error "no such structmsg: $handle"
  }
  set myDict $::structmsg($handle)
  set fillerLgth 0
  set myLgth [expr {[dict get $myDict msg cmdLgth] + 2}]
  while {[expr {$myLgth % 16}] != 0} {
    incr myLgth
    incr fillerLgth
  }
  ::structmsg_add_field $handle "@filler" uint8_t* $fillerLgth
  ::structmsg_add_field $handle "@crc" uint16_t 1
}

# ===================== structmsg_set_fieldValue =============================

proc structmsg_set_fieldValue {handle fieldName value} {
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
      return
    }
    @dst {
      dict set myDict hdr dst $value
      set ::structmsg($handle) $myDict
      setHandleField $handle dst $value
      fillHdrInfo $handle
      return
    }
    @cmdKey {
      dict set myDict msg cmdKey $value
      set ::structmsg($handle) $myDict
      setHandleField $handle cmdKey $value
      fillHdrInfo $handle
      return
    }
  }
  set numEntries [dict get $myDict msg numFieldInfos]
  set idx 0
  set fieldInfos [dict get $myDict msg fieldInfos]
  while {$idx < $numEntries} {
    set fieldInfo [lindex $fieldInfos $idx]
    if {$fieldName eq [dict get $fieldInfo fieldStr]} {
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
          if {[string length $value] == [dict get $fieldInfo fieldLgth]} {
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
      set fieldInfos [lreplace $fieldInfos $idx $idx $fieldInfo]
      dict set myDict msg fieldInfos $fieldInfos
      set ::structmsg($handle) $myDict
      return
    }
    incr idx
  }
  error "field $fieldName not found"
}

# ===================== structmsg_get_fieldValue =============================

proc structmsg_get_fieldValue {handle fieldName val} {
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
      set value [dict get $myDict msg cmdKey]
      return
    }
    @cmdLgth {
      set value [dict get $myDict msg cmdLgth]
      return
    }
  }
  set numEntries [dict get $myDict msg numFieldInfos]
  set idx 0
  set fieldInfos [dict get $myDict msg fieldInfos]
  while {$idx < $numEntries} {
    set fieldInfo [lindex $fieldInfos $idx]
    if {$fieldName eq [dict get $fieldInfo fieldStr]} {
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

# ===================== structmsg_set_crypted =============================

proc structmsg_set_crypted {handle crypted} {
  upvar $val value

  if {![info exists ::structmsg($handle)]} {
    error "no such structmsg: $handle"
  }
  set myDict $::structmsg($handle)
  dict set myDict encryptedMsg $crypted
  set ::structmsg($handle) $myDict
}

# ===================== structmsg_decrypt_getHandle =============================

proc structmsg_decrypt_getHandle {crypted cryptKey handleVal {iv [list]}} {
  upvar $handleVal handle

  set lgth [string length $crypted]
  set hdr [string range $crypted 0 [expr {$::headerLgth -1}]]
  set offset $::headerLgth
  set todecrypt [string range $crypted $offset [expr {$offset + $lgth - 1}]]
  set decryptedData [aes::aes -dir decrypt -key $cryptKey $todecrypt]
  append hdr $decryptedData
  set len [string length $hdr]
  getHandle $hdr handle
}
