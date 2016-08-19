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

puts stderr "uint32Decode: offset: $offset!"
  set value 0
  binary scan [string range $data $offset $offset] c ch
puts stderr [format "ch: 0x%02x" $ch]
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

proc crcEncode {dat offset lgth val headerOffset} {
  upvar $dat data
  upvar $val value

  set lgth [expr {$lgth - 2}]   ;# uint16_t crc
  set crc  0
  set idx $headerOffset
  set str [string range $data $idx [expr {$idx + $lgth - 1}]]
set cnt 0
  foreach ch [split $str ""] {
    binary scan $ch c pch
    set pch [expr {$pch & 0xFF}]
#puts stderr "encode crc: $cnt $ch![format 0x%02x $pch]![format 0x%04x $crc]!"
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

proc crcDecode {data offset lgth val headerOffset} {
  upvar $val value

  set value ""
  set lgth [expr {$lgth - 2}]; # uint16_t crc
  set crcVal 0
  set idx $headerOffset
  set str [string range $data $idx [expr {$idx + $lgth - 1}]]
set cnt 0
  foreach ch [split $str ""] {
    binary scan $ch c pch
    set pch [expr {$pch & 0xFF}]
#puts stderr "decode crc: $cnt ch: [format 0x%02x $pch]![format 0x%04x $crcVal]!"
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

