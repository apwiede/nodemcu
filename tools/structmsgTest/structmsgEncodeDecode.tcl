set RAND_MAX 0x7FFFFFFF

# ============================= uint8Encode ========================

proc uint8Encode {dat offset value} {
  upvar $dat data

  set ch [binary format c $value]
  set data [append data $ch]
  incr offset
  return $offset
}


# ============================= int8Encode ========================

proc int8Encode {dat offset value} {
  upvar $dat data

  set ch [binary format c $value]
  set data [append data $ch]
  incr offset
  return offset;
}

# ============================= uint16Encode ========================

proc uint16Encode {dat offset value} {
  upvar $dat data

  set ch [binary format c [expr {($value >> 8) & 0xFF}]]
  set data [append data $ch]
  incr offset
  set ch [binary format c [expr {$value & 0xFF}]]
  set data [append data $ch]
  incr offset
  return $offset;
}

# ============================= int16Encode ========================

proc int16Encode {dat offset value} {
  upvar $dat data

  set ch [binary format c [expr {($value >> 8) & 0xFF}]]
  set data [append data $ch]
  incr offset
  set ch [binary format c [expr {$value & 0xFF}]]
  set data [append data $ch]
  incr offset
  return $offset;
}

# ============================= uint32Encode ========================

proc uint32Encode {dat offset value} {
  upvar $dat data

  append data [binary format c [expr {($value >> 24) & 0xFF}]]
  incr offset
  append data [binary format c [expr {($value >> 16) & 0xFF}]]
  incr offset
  append data [binary format c [expr {($value >> 8) & 0xFF}]]
  incr offset
  append data [binary format c [expr {$value & 0xFF}]]
  incr offset
  return $offset;
}

# ============================= int32Encode ========================

proc int32Encode {dat offset value} {
  upvar $dat data

  set ch [binary format c [expr {($value >> 24) & 0xFF}]]
  set data [append data $ch]
  incr offset
  set ch [binary format c [expr {($value >> 16) & 0xFF}]]
  set data [append data $ch]
  incr offset
  set ch [binary format c [expr {($value >> 8) & 0xFF}]]
  set data [append data $ch]
  incr offset
  set ch [binary format c [expr {$value & 0xFF}]]
  set data [append data $ch]
  incr offset
  return $offset;
}

# ============================= uint8VectorEncode ========================

proc uint8VectorEncode {dat offset value lgth} {
  upvar $dat data

  append data [string range $value 0 [expr {$lgth - 1}]]
  incr offset $lgth;
  return $offset;
}

# ============================= int8VectorEncode ========================

proc int8VectorEncode {dat offset value lgth} {
  upvar $dat data

  append data [string range $value 0 [expr {$lgth - 1}]]
  incr offset $lgth;
  return offset;
}

# ============================= uint16VectorEncode ========================

proc uint16VectorEncode {dat offset value lgth} {
  upvar $dat data

  set idx 0
  while {$idx < $lgth} {
    set offset [uint16Encode data $offset [lindex $value $idx]]
    incr idx
  }
  return $offset;
}

# ============================= int16VectorEncode ========================

proc int16VectorEncode {dat offset value lgth} {
  upvar $dat data

  set idx 0
  while {$idx < $lgth} {
    set offset [uint16Encode data $offset [lindex $value $idx]]
    incr idx
  }
  return $offset;
}

# ============================= uint32VectorEncode ========================

proc uint32VectorEncode {dat offset value lgth} {
  upvar $dat data

  set idx 0
  while {$idx < $lgth} {
    set offset [uint16Encode data $offset [lindex $value $idx]]
    incr idx
  }
  return $offset;
}

# ============================= int32VectorEncode ========================

proc int32VectorEncode {dat offset value lgth} {
  upvar $dat data

  set idx 0
  while {$idx < $lgth} {
    set offset [uint16Encode data $offset [lindex $value $idx]]
    incr idx
  }
  return $offset;
}

# ============================= uint8Decode ========================

proc uint8Decode {data offset val} {
  upvar $val value

  set value [expr {[string range $data $offset $offset] & 0xFF}];
  incr offset
  return $offset;
}

# ============================= int8Decode ========================

proc int8Decode {data offset val} {
  upvar $val value

  set value [expr {[string range $data $offset $offset] & 0xFF}];
  incr offset
  return $offset;
}

# ============================= uint16Decode ========================

proc uint16Decode {data offset val} {
  upvar $val value

  set value 0
  binary scan [string range $data $offset $offset] c ch
  set value [expr {$value + ([expr {$ch & 0xFF}] << 8)}]
  incr offset
  binary scan [string range $data $offset $offset] c ch
  set value [expr {$value + ([expr {$ch & 0xFF}] << 0)}]
  incr offset
  return $offset;
}

# ============================= int16Decode ========================

proc int16Decode {data offset val} {
  upvar $val value

  set value 0
  binary scan [string range $data $offset $offset] c ch
  set value [expr {$value + ([expr {$ch & 0xFF}] << 8)}]
  incr offset
  binary scan [string range $data $offset $offset] c ch
  set value [expr {$value + ([expr {$ch & 0xFF}] << 0)}]
  incr offset
  return $offset;
}

# ============================= uint32Decode ========================

proc uint32Decode {data offset val} {
  upvar $val value

  set value 0
  binary scan [string range $data $offset $offset] c ch
  set value [expr {$value + ([expr {$ch & 0xFF}] << 24)}]
  incr offset
  binary scan [string range $data $offset $offset] c ch
  set value [expr {$value + ([expr {$ch & 0xFF}] << 16)}]
  incr offset
  binary scan [string range $data $offset $offset] c ch
  set value [expr {$value + ([expr {$ch & 0xFF}] << 8)}]
  incr offset
  binary scan [string range $data $offset $offset] c ch
  set value [expr {$value + ([expr {$ch & 0xFF}] << 0)}]
  incr offset
  return $offset;
}

# ============================= int32Decode ========================

proc int32Decode {data offset val} {
  upvar $val value

  set value 0
  binary scan [string range $data $offset $offset] c ch
  set value [expr {$value + ([expr {$ch & 0xFF}] << 24)}]
  incr offset
  binary scan [string range $data $offset $offset] c ch
  set value [expr {$value + ([expr {$ch & 0xFF}] << 16)}]
  incr offset
  binary scan [string range $data $offset $offset] c ch
  set value [expr {$value + ([expr {$ch & 0xFF}] << 8)}]
  incr offset
  binary scan [string range $data $offset $offset] c ch
  set value [expr {$value + ([expr {$ch & 0xFF}] << 0)}]
  incr offset
  incr offset
  return $offset;
}

# ============================= uint8VectorDecode ========================

proc uint8VectorDecode {data offset val len} {
  upvar $val value
  upvar $len lgth

  set value [string range $data $offset [expr {$offset + $lgth}]]
  incr offset $lgth
  return $offset;
}

# ============================= int8VectorDecode ========================

proc int8VectorDecode {data offset val len} {
  upvar $val value
  upvar $len lgth

  set value [string range $data $offset [expr {$offset + $lgth}]]
  incr offset $lgth
  return $offset;
}

# ============================= uint16VectorDecode ========================

proc uint16VectorDecode {data offset val len} {
  upvar $val value
  upvar $len lgth

  set idx 0
  while {$idx < $lgth} {
    set offset [uint16Decode data $offset, value+idx]
    incr idx
  }
  return $offset;
}

# ============================= int16VectorDecode ========================

proc int16VectorDecode {data offset val len} {
  int idx;

  idx = 0;
  while (idx < lgth) {
    offset = int16Decode(data, offset, &((*value)[idx]));
    idx++;
  }
  return offset;
}

# ============================= uint32VectorDecode ========================

proc uint32VectorDecode {data offset val len} {
  int idx;

  idx = 0;
  while (idx < lgth) {
    offset = uint32Decode(data, offset, &((*value)[idx]));
    idx++;
  }
  return offset;
}

# ============================= int32VectorDecode ========================

proc int32VectorDecode {data offset val len} {
  int idx;

  idx = 0;
  while (idx < lgth) {
    offset = int32Decode(data, offset, &((*value)[idx]));
    idx++;
  }
  return offset;
}

# ============================= getRandom ========================

proc getRandom {} {
  set val [string trimleft [lindex [split [expr {rand()}] {.}] 1] 0]
  set myVal [expr {$val & $::RAND_MAX}]
  return $myVal
}

# ============================= randomNumEncode ========================

proc randomNumEncode {dat offset val} {
  upvar $dat data
  upvar $val value

  set myVal [getRandom]
  set value $myVal
  return [uint32Encode data $offset $myVal]
}

# ============================= randomNumDecode ========================

proc randomNumDecode {data offset val} {
  upvar $val value

  set offset [uint32Decode $data $offset value]
  return $offset
}

# ============================= fillerEncode ========================

proc fillerEncode {dat offset lgth val} {
  upvar $dat data
  upvar $val value

  set idx 0
  set value ""
  while {$lgth >= 4} {
    set myVal [expr {[getRandom] &0xFFFFFFFF}]
    append value [binary format c [expr {($myVal >> 24) & 0xFF}]]
    append value [binary format c [expr {($myVal >> 16) & 0xFF}]]
    append value [binary format c [expr {($myVal >> 8) & 0xFF}]]
    append value [binary format c [expr {($myVal >> 0) & 0xFF}]]
    set offset [uint32Encode data $offset $myVal]
    incr lgth -4
  }
  while {$lgth >= 2} {
    set myVal [expr {[getRandom] & 0xFFFF}]
    append value [binary format c [expr {($myVal >> 8) & 0xFF}]]
    append value [binary format c [expr {($myVal >> 0) & 0xFF}]]
    set offset [uint16Encode data $offset $myVal]
    incr lgth -2
  }
  while {$lgth >= 1} {
    set myVal [expr {[getRandom] & 0xFF}]
    append value [binary format c [expr {($myVal >> 0) & 0xFF}]]
    set offset [uint8Encode data $offset $myVal]
    incr lgth -1
  }
  return $offset;
}

# ============================= fillerDecode ========================
 
proc fillerDecode {data offset lgth val} {
  upvar $val value

  set value [string range $data $offset [expr {$offset + $lgth - 1}]]
  incr offset $lgth
  return $offset
}

# ============================= crcEncode ========================

proc crcEncode {dat offset lgth val} {
  upvar $dat data
  upvar $val value

  set lgth [expr {$lgth - 2}]   ;# uint16_t crc
  set crc  0
  set idx [expr {2 * 3}]; # uint16_t src + uint16_t dst + uint16_t totalLgth
  set str [string range $data $idx [expr {$idx + $lgth - 1}]]
set cnt 0
  foreach ch [split $str ""] {
    binary scan $ch c pch
#puts stderr "encode crc: $cnt $ch![format 0x%02x [expr {$pch & 0xFF}]]![format 0x%04x $crc]!"
    set crc [expr {$crc + [format "%d" $pch]}]
incr cnt
  }
puts stderr "crc: $crc![format 0x%04x $crc]!"
  set crc [expr {~$crc & 0xFFFF}]
puts stderr "crc2: $crc![format 0x%04x $crc]!"
  set offset [uint16Encode data $offset $crc]
  set value $crc
  return $offset
}

# ============================= crcDecode ========================

proc crcDecode {data offset lgth val} {
  upvar $val value

  set value ""
  set lgth [expr {$lgth - 2}]; # uint16_t crc
  set crcVal 0
  set idx [expr {2 * 3}]; # uint16_t src + uint16_t dst + uint16_t totalLgth
  set str [string range $data $idx [expr {$idx + $lgth - 1}]]
set cnt 0
  foreach ch [split $str ""] {
    binary scan $ch c pch
#puts stderr "decode crc: $cnt $ch![format 0x%02x [expr {$pch & 0xFF}]]![format 0x%04x $crcVal]!"
    set crcVal [expr {$crcVal + [format "%d" $pch]}]
incr cnt
    incr idx
  }
puts stderr "crcVal end: $crcVal!"
  set crcVal [expr {~$crcVal & 0xFFFF}]
  set offset [uint16Decode $data $offset crc]
puts stderr "crcVal: [format 0x%04x $crcVal]!offset: $offset!crc: [format 0x%04x $crc]!"
  if {$crcVal != $crc} {
    return -1
  }
  set value $crc
  return $offset;
}

# ============================= encodeMsg ========================

proc encodeMsg {handle} {
  if {![info exists ::structmsg($handle)]} {
    puts stderr "no such structmsg: $handle"
  }
  set myDict $::structmsg($handle)
  dict set myDict encoded [list]
  set encoded [list]
  set offset 0
  set offset [uint16Encode encoded $offset [dict get $myDict src]]
puts stderr "src offset: $offset!len: [string length $encoded]!"
  set offset [uint16Encode encoded $offset [dict get $myDict dst]]
puts stderr "dst offset: $offset!len: [string length $encoded]!"
  set offset [uint16Encode encoded $offset [dict get $myDict totalLgth]]
puts stderr "totalLgth offset: $offset!len: [string length $encoded]!"
  set offset [uint16Encode encoded $offset [dict get $myDict msg cmdKey]]
puts stderr "cmdKey offset: $offset!len: [string length $encoded]!"
  set offset [uint16Encode encoded $offset [dict get $myDict msg cmdLgth]]
puts stderr "cmdLgth offset: $offset!len: [string length $encoded]!"
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
puts stderr "randomNum offset: $offset!len: [string length $encoded]!"
          set_fieldValue $handle "@randomNum" $randomNum
        }
        "@filler" {
          set offset [fillerEncode encoded $offset [dict get $fieldInfo fieldLgth] value]
puts stderr "filler offset: $offset!len: [string length $encoded]!"
          set_fieldValue $handle "@filler" $value
        }
        "@crc" {
          set offset [crcEncode encoded $offset [dict get $myDict msg cmdLgth] crc]
puts stderr "crc offset: $offset!len: [string length $encoded]!"
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
#puts stderr "fld: $fieldName!$fieldType!"
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
#puts stderr "[dict get $fieldInfo fieldStr] offset: $offset!len: [string length $encoded]!"
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

# ============================= getEncoded ========================

proc getEncoded {handle} {
  if {![info exists ::structmsg($handle)]} {
    puts stderr "no such structmsg: $handle"
  }
  set myDict $::structmsg($handle)
  if {[lsearch [dict get $myDict flags] ENCODED] < 0} {
    error "handle: $handle is not encoded!"
  }
  return [dict get $myDict encoded]
}

# ============================= decodeMsg ========================

proc decodeMsg {handle todecode} {
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
          set offset [crcDecode $todecode $offset [dict get $myDict msg cmdLgth] crc]
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
#puts stderr "fld: $fieldName!$fieldType!"
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
