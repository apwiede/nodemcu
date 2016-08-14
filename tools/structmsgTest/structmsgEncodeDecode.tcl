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
  set value [expr {$value + ([expr {[string range $data $offset $offset] & 0xFF}]) << 8}]
  incr offset
  set value [expr {$value + ([expr {[string range $data $offset $offset] & 0xFF}]) << 0}]
  incr offset
  return $offset;
}

# ============================= int16Decode ========================

proc int16Decode {data offset val} {
  upvar $val value

  set value 0
  set value [expr {$value + ([expr {[string range $data $offset $offset] & 0xFF}]) << 8}]
  incr offset
  set value [expr {$value + ([expr {[string range $data $offset $offset] & 0xFF}]) << 0}]
  incr offset
  return $offset;
}

# ============================= uint32Decode ========================

proc uint32Decode {data offset val} {
  upvar $val value

  set value 0
  set value [expr {$value + ([expr {[string range $data $offset $offset] & 0xFF}]) << 24}]
  incr offset
  set value [expr {$value + ([expr {[string range $data $offset $offset] & 0xFF}]) << 16}]
  incr offset
  set value [expr {$value + ([expr {[string range $data $offset $offset] & 0xFF}]) << 8}]
  incr offset
  set value [expr {$value + ([expr {[string range $data $offset $offset] & 0xFF}]) << 0}]
  incr offset
  return $offset;
}

# ============================= int32Decode ========================

proc int32Decode {data offset val} {
  upvar $val value

  set value 0
  set value [expr {$value + ([expr {[string range $data $offset $offset] & 0xFF}]) << 24}]
  incr offset
  set value [expr {$value + ([expr {[string range $data $offset $offset] & 0xFF}]) << 16}]
  incr offset
  set value [expr {$value + ([expr {[string range $data $offset $offset] & 0xFF}]) << 8}]
  incr offset
  set value [expr {$value + ([expr {[string range $data $offset $offset] & 0xFF}]) << 0}]
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

# ============================= randomNumEncode ========================

proc randomNumEncode {data offset val} {
  uint32_t val;

  val = (uint32_t)(rand() & RAND_MAX);
  *value = val;
  return uint32Encode(data, offset, val);
}

# ============================= randomNumDecode ========================

proc randomNumDecode {data offset val} {
  int result;

  offset = uint32Decode(data, offset, value);
  return offset;
}

# ============================= fillerEncode ========================

proc fillerEncode {data offset len val} {
  uint32_t val;
  int idx;

  idx = 0;
  while (lgth >= 4) {
    val = (uint32_t)(rand() & RAND_MAX);
    value[idx++] = (val >> 24) & 0xFF;
    value[idx++] = (val >> 16) & 0xFF;
    value[idx++] = (val >> 8) & 0xFF;
    value[idx++] = (val >> 0) & 0xFF;
    offset = uint32Encode(data, offset, val);
    lgth -= 4;
  }
  while (lgth >= 2) {
    val = (uint16_t)((rand() & RAND_MAX) & 0xFFFF);
    value[idx++] = (val >> 8) & 0xFF;
    value[idx++] = (val >> 0) & 0xFF;
    offset = uint16Encode(data, offset, val);
    lgth -= 2;
  }
  while (lgth >= 1) {
    val = (uint8_t)((rand() & RAND_MAX) & 0xFF);
    offset = uint8Encode(data, offset, val);
    value[idx++] = (val >> 0) & 0xFF;
    lgth -= 1;
  }
  return offset;
}

# ============================= fillerDecode ========================
 
proc fillerDecode {data offset len val} {
  int idx;

  c_memcpy(*value,data+offset,lgth);
  offset += lgth;
  return offset;
}

# ============================= crcEncode ========================

proc crcEncode {data offset startData len val} {
  int idx;

  lgth -= sizeof(uint16_t);   // uint16_t crc
  *crc = 0;
  idx = sizeof(uint16_t) * 3; // uint16_t src + uint16_t dst + uint16_t totalLgth
  while (idx < lgth) {
    *crc += data[idx++];
  }
  *crc = ~(*crc);
  offset = uint16Encode(data,offset,*crc);
  return offset;
}

# ============================= crcDecode ========================

proc crcDecode {data offset startData len val} {
  uint16_t crcVal;
  int idx;

  lgth -= sizeof(uint16_t);   // uint16_t crc
  crcVal = 0;
  idx = sizeof(uint16_t) * 3; // uint16_t src + uint16_t dst + uint16_t totalLgth
  while (idx < lgth) {
    crcVal += data[idx++];
  }
  crcVal = ~crcVal;
  offset = uint16Decode(data, offset, crc);
  if (crcVal != *crc) {
    return -1;
     }
  return offset;
}
