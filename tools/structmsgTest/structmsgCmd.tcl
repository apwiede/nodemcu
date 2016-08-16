source pdict.tcl
source structmsgEncodeDecode.tcl

set ::structmsg(prefix) "stmsg_"
set ::structmsg(numHandles) 1
set ::structmsg(handles) [list]

# ===================== dump_structmsg =============================

proc dump_structmsg {handle} {
  if {![info exists ::structmsg($handle)]} {
    puts stderr "no such structmsg: $handle"
  }
  set myDict $::structmsg($handle)
  puts stderr [format "handle: %s src: %d dst: %d totalLgth: %d" $handle [dict get $myDict src] [dict get $myDict dst] [dict get $myDict totalLgth]]
  puts stderr [format "  cmdKey: %d cmdLgth: %d" [dict get $myDict msg cmdKey] [dict get $myDict msg cmdLgth]]
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

# ===================== create_structmsg =============================

proc create_structmsg {numFieldInfos} {
  set handle $::structmsg(prefix)ffef[format "%04d" $::structmsg(numHandles)]
  lappend ::structmsg(handles) $handle
  incr ::structmsg(numHandles)
  set myDict [dict create]
  dict set myDict msg cmdLgth 4; # uint16_t cmdKey + unit16_t cmdLgth
  dict set myDict totalLgth 10;  # cmdLgth + uint16_t src + uint16_t dst + uint16_t totalLgth
  dict set myDict msg maxFieldInfos $numFieldInfos
  dict set myDict msg numFieldInfos 0
  dict set myDict msg fieldInfos [list]
  dict set myDict flags [list]
  dict set myDict encoded [list]
  dict set myDict todecode [list]
  dict set myDict encrypted [list]
  dict set myDict encryptedLgth 0
  dict set myDict decrypted [list]
  dict set myDict decryptedLgth 0
  set ::structmsg($handle) $myDict
  return $handle
}

set offset 0
set handle [create_structmsg 5]
set_targets $handle 123 456 789
addField $handle "@randomNum" uint32_t 4
addField $handle "pwd" uint8_t* 16
set_fieldValue $handle "pwd" "/dir1/dir2/dir34"
set_fillerAndCrc $handle
#dump_structmsg $handle
encodeMsg $handle
dump_structmsg $handle
#puts stderr "DICT:"
#pdict [set ::structmsg($handle)]
set encoded [getEncoded $handle]
puts stderr "encoded: $encoded!"
#set encoded [string replace $encoded 0 0 789]
#set encoded [string replace $encoded 1 1 123]
decodeMsg $handle $encoded
dump_structmsg $handle
#puts stderr "DICT:"
#pdict [set ::structmsg($handle)]
