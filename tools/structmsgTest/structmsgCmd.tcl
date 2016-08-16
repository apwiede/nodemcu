source pdict.tcl
source structmsgEncodeDecode.tcl

package require aes

set ::structmsg(prefix) "stmsg_"
set ::structmsg(numHandles) 1
set ::structmsg(handles) [list]

# ===================== dump_structmsg =============================

proc dump_structmsg {handle} {
  if {![info exists ::structmsg($handle)]} {
    puts stderr "no such structmsg: $handle"
  }
  set myDict $::structmsg($handle)
  set src [expr {[dict get $myDict src] & 0xFFFF}]
  set dst [expr {[dict get $myDict dst] & 0xFFFF}]
  set totalLgth [expr {[dict get $myDict totalLgth] & 0xFFFF}]
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
          puts -nonewline stderr "      values: "
          set cnt 0
          foreach ch [split $val ""] {
            scan $ch %c pch
            puts -nonewline stderr [format "  $ch 0x%02x" $pch]
            incr cnt
            if {($cnt > 0) && (($cnt % 10) == 0)} {
              puts -nonewline stderr "\n              "
            }
          }
          puts stderr ""
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
    puts -nonewline stderr "      values: "
    set cnt 0
if {[catch {
    foreach ch [split $encoded ""] {
      scan $ch %c pch
      puts -nonewline stderr [format "  $ch 0x%02x" $pch]
      incr cnt
      if {($cnt > 0) && (($cnt % 10) == 0)} {
        puts -nonewline stderr "\n              "
      }
    }
} msg]} {
    puts stderr "\nMSG: $msg!"
    foreach ch [split $encoded ""] {
      scan $ch %c pch
puts stderr "\n!${ch}!${pch}!"
    }
}
    puts stderr ""
  }
  # decoded
  if {[lsearch [dict get $myDict flags] DECODED] >= 0} {
    set todecode [dict get $myDict todecode]
    puts stderr "  todecode: $todecode"
    puts -nonewline stderr "      values: "
    set cnt 0
if {[catch {
    foreach ch [split $todecode ""] {
      scan $ch %c pch
      puts -nonewline stderr [format "  $ch 0x%02x" $pch]
      incr cnt
      if {($cnt > 0) && (($cnt % 10) == 0)} {
        puts -nonewline stderr "\n              "
      }
    }
} msg]} {
    puts stderr "\nMSG: $msg!"
    foreach ch [split $todecode ""] {
      scan $ch %c pch
puts stderr "\n!${ch}!${pch}!"
    }
}
    puts stderr ""
  }
  # encryptedMsg
  if {[lsearch [dict get $myDict flags] ENCRYPTED] >= 0} {
    set encryptedMsg [dict get $myDict encryptedMsg]
    puts stderr "  encryptedMsg: $encryptedMsg"
    puts -nonewline stderr "      values: "
    set cnt 0
if {[catch {
    foreach ch [split $encryptedMsg ""] {
      scan $ch %c pch
      puts -nonewline stderr [format "  $ch 0x%02x" $pch]
      incr cnt
      if {($cnt > 0) && (($cnt % 10) == 0)} {
        puts -nonewline stderr "\n              "
      }
    }
} msg]} {
    puts stderr "\nMSG: $msg!"
    foreach ch [split $encryptedMsg ""] {
      scan $ch %c pch
puts stderr "\n!${ch}!${pch}!"
    }
}
    puts stderr ""
  }
  # decryptedMsg
  if {[lsearch [dict get $myDict flags] DECRYPTED] >= 0} {
    set decryptedMsg [dict get $myDict decryptedMsg]
    puts stderr "  decryptedMsg: $decryptedMsg"
    puts -nonewline stderr "      values: "
    set cnt 0
if {[catch {
    foreach ch [split $decryptedMsg ""] {
      scan $ch %c pch
      puts -nonewline stderr [format "  $ch 0x%02x" $pch]
      incr cnt
      if {($cnt > 0) && (($cnt % 10) == 0)} {
        puts -nonewline stderr "\n              "
      }
    }
} msg]} {
    puts stderr "\nMSG: $msg!"
    foreach ch [split $decryptedMsg ""] {
      scan $ch %c pch
puts stderr "\n!${ch}!${pch}!"
    }
}
    puts stderr ""
  }
}

# ===================== set_targets =============================

proc set_targets {handle src dst cmdKey} {
  if {![info exists ::structmsg($handle)]} {
    puts stderr "no such structmsg: $handle"
  }
  set myDict $::structmsg($handle)
  dict set myDict src $src
  dict set myDict dst $dst
  dict set myDict msg cmdKey $cmdKey
  set ::structmsg($handle) $myDict
}

# ===================== set_fieldValue =============================

proc set_fieldValue {handle fieldName value} {
  if {![info exists ::structmsg($handle)]} {
    puts stderr "no such structmsg: $handle"
  }
  set myDict $::structmsg($handle)
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

# ===================== addField =============================

proc addField {handle name fieldType fieldLgth} {
  if {![info exists ::structmsg($handle)]} {
    puts stderr "no such structmsg: $handle"
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
      dict set myDict totalLgth [expr {[dict get $myDict totalLgth] + 1}]
      dict set myDict msg cmdLgth [expr {[dict get $myDict msg cmdLgth] + 1}]
      dict set fieldInfo fieldLgth 1
    }
    uint16_t -
    int16_t {
      dict set myDict totalLgth [expr {[dict get $myDict totalLgth] + 2}]
      dict set myDict msg cmdLgth [expr {[dict get $myDict msg cmdLgth] + 2}]
      dict set fieldInfo fieldLgth 2
    }
    uint32_t -
    int32_t {
      dict set myDict totalLgth [expr {[dict get $myDict totalLgth] + 4}]
      dict set myDict msg cmdLgth [expr {[dict get $myDict msg cmdLgth] + 4}]
      dict set fieldInfo fieldLgth 4
    }
    uint8_t* -
    int8_t* {
      dict set myDict totalLgth [expr {[dict get $myDict totalLgth] + $fieldLgth}]
      dict set myDict msg cmdLgth [expr {[dict get $myDict msg cmdLgth] + $fieldLgth}]
      dict set fieldInfo fieldLgth $fieldLgth
    }
    uint16_t* -
    int16_t* {
      dict set myDict totalLgth [expr {[dict get $myDict totalLgth] + [expr {$fieldLgth * 2}]}]
      dict set myDict msg cmdLgth [expr {[dict get $myDict msg cmdLgth] + [expr {$fieldLgth * 2}]}]
      dict set fieldInfo fieldLgth $fieldLgth
    }
    uint32_t* -
    int32_t* {
      dict set myDict totalLgth [expr {[dict get $myDict totalLgth] + [expr {$fieldLgth * 4}]}]
      dict set myDict msg cmdLgth [expr {[dict get $myDict msg cmdLgth] + [expr {$fieldLgth * 4}]}]
      dict set fieldInfo fieldLgth $fieldLgth
    }
  } 
  lappend fieldInfos $fieldInfo
  dict set myDict msg fieldInfos $fieldInfos
  dict set myDict msg numFieldInfos [expr {[dict get $myDict msg numFieldInfos] + 1}]
  set ::structmsg($handle) $myDict
}

# ===================== set_fillerAndCrc =============================

proc set_fillerAndCrc {handle} {
  if {![info exists ::structmsg($handle)]} {
    puts stderr "no such structmsg: $handle"
  }
  set myDict $::structmsg($handle)
  set fillerLgth 0
  set myLgth [expr {[dict get $myDict msg cmdLgth] + 2}]
  while {[expr {$myLgth % 16}] != 0} {
    incr myLgth
    incr fillerLgth
  }
  ::addField $handle "@filler" uint8_t* $fillerLgth
  ::addField $handle "@crc" uint16_t 1
}

# ============================= encode_msg ========================

proc encode_msg {handle} {
  if {![info exists ::structmsg($handle)]} {
    puts stderr "no such structmsg: $handle"
  }
  set myDict $::structmsg($handle)
  dict set myDict encoded [list]
  set encoded [list]
  set offset 0
  set offset [uint16Encode encoded $offset [dict get $myDict src]]
  set offset [uint16Encode encoded $offset [dict get $myDict dst]]
  set offset [uint16Encode encoded $offset [dict get $myDict totalLgth]]
  set offset [uint16Encode encoded $offset [dict get $myDict msg cmdKey]]
  set offset [uint16Encode encoded $offset [dict get $myDict msg cmdLgth]]
  dict set myDict encoded $encoded
  set ::structmsg($handle) $myDict
  set idx 0
  set numEntries [dict get $myDict msg numFieldInfos]
  while {$idx < $numEntries} {
    set myDict $::structmsg($handle) ; # needed because set_fieldValue changes the dict!!
    set fieldInfos [dict get $myDict msg fieldInfos]
    set fieldInfo [lindex $fieldInfos $idx]
    if {[string range [dict get $fieldInfo fieldStr] 0 0] == "@"} {
      set fieldName [dict get $fieldInfo fieldStr]
      switch $fieldName {
        "@randomNum" {
          set offset [randomNumEncode encoded $offset randomNum]
          set_fieldValue $handle "@randomNum" $randomNum
        }
        "@filler" {
          set offset [fillerEncode encoded $offset [dict get $fieldInfo fieldLgth] value]
          set_fieldValue $handle "@filler" $value
        }
        "@crc" {
          set offset [crcEncode encoded $offset [dict get $myDict msg cmdLgth] crc [dict get $myDict headerLgth]]
          set_fieldValue $handle "@crc" $crc
        }
        default {
          error "BAD_SPECIAL_FIELD: $fieldName!"
        }
      }
      set myDict $::structmsg($handle) ; # needed because set_fieldValue changes the dict!!
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

# ============================= get_encoded ========================

proc get_encoded {handle} {
  if {![info exists ::structmsg($handle)]} {
    puts stderr "no such structmsg: $handle"
  }
  set myDict $::structmsg($handle)
  if {[lsearch [dict get $myDict flags] ENCODED] < 0} {
    error "handle: $handle is not encoded!"
  }
  return [dict get $myDict encoded]
}


# ============================= get_encodedMsg ========================

proc get_encodedMsg {handle} {
  if {![info exists ::structmsg($handle)]} {
    puts stderr "no such structmsg: $handle"
  }
  set myDict $::structmsg($handle)
  if {[lsearch [dict get $myDict flags] ENCODED] < 0} {
    error "handle: $handle is not encoded!"
  }
  set encoded [dict get $myDict encoded]

}

# ============================= decode_msg ========================

proc decode_msg {handle todecode} {
  if {![info exists ::structmsg($handle)]} {
    puts stderr "no such structmsg: $handle"
  }
  set myDict $::structmsg($handle)
  dict set myDict todecode $todecode
  set offset 0
  set offset [uint16Decode $todecode $offset src]
  dict set myDict src $src
  set offset [uint16Decode $todecode $offset dst]
  dict set myDict dst $dst
  set offset [uint16Decode $todecode $offset totalLgth]
  dict set myDict totalLgth $totalLgth
  set offset [uint16Decode $todecode $offset cmdKey]
  dict set myDict msg cmdKey $cmdKey
  set offset [uint16Decode $todecode $offset cmdLgth]
  dict set myDict msg cmdLgth $cmdLgth
  set ::structmsg($handle) $myDict
  set idx 0
  set numEntries [dict get $myDict msg numFieldInfos]
  while {$idx < $numEntries} {
    set myDict $::structmsg($handle) ; # needed because set_fieldValue changes the dict!!
    set fieldInfos [dict get $myDict msg fieldInfos]
    set fieldInfo [lindex $fieldInfos $idx]
    if {[string range [dict get $fieldInfo fieldStr] 0 0] == "@"} {
      set fieldName [dict get $fieldInfo fieldStr]
      switch $fieldName {
        "@randomNum" {
          set offset [randomNumDecode $todecode $offset randomNum]
          dict set fieldInfo value $randomNum
        }
        "@filler" {
          set offset [fillerDecode $todecode $offset [dict get $fieldInfo fieldLgth] value]
          dict set fieldInfo value $value
        }
        "@crc" {
          set offset [crcDecode $todecode $offset [dict get $myDict msg cmdLgth] crc [dict get $myDict headerLgth]]
          dict set fieldInfo value $crc
        }
        default {
          error "BAD_SPECIAL_FIELD: $fieldName!"
        }
      }
      set myDict $::structmsg($handle) ; # needed because set_fieldValue changes the dict!!
      set fieldInfos [dict get $myDict msg fieldInfos]
      set fieldInfo [lindex $fieldInfos $idx]
    } else {
     set fieldType [dict get $fieldInfo fieldType]
      set fieldName [dict get $fieldInfo fieldStr]
      switch $fieldType {
        int8_t {
          set offset [int8Decode $todecode $offset [dict get $fieldInfo value]]
        }
        uint8_t {
          set offset [uint8Decode $todecode $offset [dict get $fieldInfo value]]
        }
        int16_t {
          set offset [int16Decode $todecode $offset [dict get $fieldInfo value]]
        }
        uint16_t {
          set offset [uint16Decode $todecode $offset [dict get $fieldInfo value]]
        }
        int32_t {
          set offset [int32Decode $todecode $$offset [dict get $fieldInfo value]]
        }
        uint32_t {
          set offset [uint32Decode $todecode $offset [dict get $fieldInfo value]]
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
          dict set $fieldInfo value [string range $todecode $offset [expr {$offset + $lgth - 1}]]
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

# ===================== encrypt_payload =================================

proc encrypt_payload {handle cryptKey} {
  if {![info exists ::structmsg($handle)]} {
    puts stderr "no such structmsg: $handle"
  }
  set myDict $::structmsg($handle)
  set headerLgth [dict get $myDict headerLgth]
  set encoded [get_encoded $handle]
  set lgth [dict get $myDict msg cmdLgth]
  set offset $headerLgth
  set tocrypt [string range $encoded $offset [expr {$offset + $lgth - 1}]]
  set encryptedData [aes::aes -dir encrypt -key $cryptKey $tocrypt]
  set len [string length $encryptedData]
  dict set myDict encrypted $encryptedData
  dict set myDict encryptedLgth $len
  dict set myDict encryptedMsg [string range $encoded 0 [expr {$headerLgth - 1}]]
  dict append myDict encryptedMsg $encryptedData
  if {[lsearch [dict get $myDict flags] "ENCRYPTED"] < 0} {
    dict lappend myDict flags ENCRYPTED
  }
  set ::structmsg($handle) $myDict
  return [dict get $myDict encryptedMsg]
}

# ===================== decrypt_payload =================================

proc decrypt_payload {handle cryptKey crypted} {
  if {![info exists ::structmsg($handle)]} {
    puts stderr "no such structmsg: $handle"
  }
  set myDict $::structmsg($handle)
  set headerLgth [dict get $myDict headerLgth]
  set lgth [dict get $myDict msg cmdLgth]
  set offset $headerLgth
  set todecrypt [string range $crypted $offset [expr {$offset + $lgth - 1}]]
  set decryptedData [aes::aes -dir decrypt -key $cryptKey $todecrypt]
  set len [string length $decryptedData]
  dict set myDict decrypted $decryptedData
  dict set myDict decryptedLgth $len
  dict set myDict decryptedMsg [string range $crypted 0 [expr {$headerLgth - 1}]]
  dict append myDict decryptedMsg $decryptedData
  if {[lsearch [dict get $myDict flags] "DECRYPTED"] < 0} {
    dict lappend myDict flags DECRYPTED
  }
  set ::structmsg($handle) $myDict
  return [dict get $myDict decryptedMsg]
}

# ===================== create_structmsg =============================

proc create_structmsg {numFieldInfos} {
  set handle $::structmsg(prefix)ffef[format "%04d" $::structmsg(numHandles)]
  lappend ::structmsg(handles) $handle
  incr ::structmsg(numHandles)
  set myDict [dict create]
  set headerLgth 6; # uint16_t src + uint16_t dst + uint16_t totalLgth
  set cmdLgth 4;    # uint16_t cmdKey + unit16_t cmdLgth
  dict set myDict headerLgth $headerLgth
  dict set myDict totalLgth [expr {$headerLgth + $cmdLgth}]
  dict set myDict msg cmdLgth $cmdLgth
  dict set myDict msg fieldInfos [list]
  dict set myDict msg maxFieldInfos $numFieldInfos
  dict set myDict msg numFieldInfos 0
  dict set myDict flags [list]
  dict set myDict encoded [list]
  dict set myDict todecode [list]
  dict set myDict encrypted [list]
  dict set myDict encryptedLgth 0
  dict set myDict encryptedMsg [list]
  dict set myDict decrypted [list]
  dict set myDict decryptedLgth 0
  dict set myDict decryptedMsg [list]
  set ::structmsg($handle) $myDict
  return $handle
}

