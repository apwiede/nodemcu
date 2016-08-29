# ===========================================================================
# * Copyright (c) 2016, Arnulf Wiedemann
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

# ============================= sizeof ========================

proc sizeof {type} {
  switch $type {
    int8_t -
    uint8_t {
      return 1
    }
    int16_t -
    uint16_t {
      return 2
    }
    int32_t -
    uint32_t {
      return 4
    }
  }
  error "bad type in sizeof: $type"
}

set RAND_MAX 0x7FFFFFFF

set STRUCT_MSG_FIELD_UINT8_T       1
set STRUCT_MSG_FIELD_INT8_T        2
set STRUCT_MSG_FIELD_UINT16_T      3
set STRUCT_MSG_FIELD_INT16_T       4
set STRUCT_MSG_FIELD_UINT32_T      5
set STRUCT_MSG_FIELD_INT32_T       6
set STRUCT_MSG_FIELD_UINT8_VECTOR  7
set STRUCT_MSG_FIELD_INT8_VECTOR   8
set STRUCT_MSG_FIELD_UINT16_VECTOR 9
set STRUCT_MSG_FIELD_INT16_VECTOR  10
set STRUCT_MSG_FIELD_UINT32_VECTOR 11
set STRUCT_MSG_FIELD_INT32_VECTOR  12

set STRUCT_MSG_SPEC_FIELD_SRC              255
set STRUCT_MSG_SPEC_FIELD_DST              254
set STRUCT_MSG_SPEC_FIELD_TARGET_CMD       253
set STRUCT_MSG_SPEC_FIELD_TOTAL_LGTH       252
set STRUCT_MSG_SPEC_FIELD_CMD_KEY          251
set STRUCT_MSG_SPEC_FIELD_CMD_LGTH         250
set STRUCT_MSG_SPEC_FIELD_RANDOM_NUM       249
set STRUCT_MSG_SPEC_FIELD_SEQUENCE_NUM     248
set STRUCT_MSG_SPEC_FIELD_FILLER           247
set STRUCT_MSG_SPEC_FIELD_CRC              246
set STRUCT_MSG_SPEC_FIELD_ID               245
set STRUCT_MSG_SPEC_FIELD_ID               245
set STRUCT_MSG_SPEC_FIELD_TABLE_ROWS       244
set STRUCT_MSG_SPEC_FIELD_TABLE_ROW_FIELDS 243
set STRUCT_MSG_SPEC_FIELD_LOW              242  ; # this must be the last entry!!

set STRUCT_MSG_ERR_OK                    0
set STRUCT_MSG_ERR_VALUE_NOT_SET         255
set STRUCT_MSG_ERR_VALUE_OUT_OF_RANGE    254
set STRUCT_MSG_ERR_BAD_VALUE             253
set STRUCT_MSG_ERR_BAD_FIELD_TYPE        252
set STRUCT_MSG_ERR_FIELD_NOT_FOUND       251
set STRUCT_MSG_ERR_VALUE_TOO_BIG         250
set STRUCT_MSG_ERR_BAD_SPECIAL_FIELD     249
set STRUCT_MSG_ERR_BAD_HANDLE            248
set STRUCT_MSG_ERR_OUT_OF_MEMORY         247
set STRUCT_MSG_ERR_HANDLE_NOT_FOUND      246
set STRUCT_MSG_ERR_NOT_ENCODED           245
set STRUCT_MSG_ERR_ENCODE_ERROR          244
set STRUCT_MSG_ERR_DECODE_ERROR          243
set STRUCT_MSG_ERR_BAD_CRC_VALUE         242
set STRUCT_MSG_ERR_CRYPTO_INIT_FAILED    241
set STRUCT_MSG_ERR_CRYPTO_OP_FAILED      240
set STRUCT_MSG_ERR_CRYPTO_BAD_MECHANISM  239
set STRUCT_MSG_ERR_NOT_ENCRYPTED         238
set STRUCT_MSG_ERR_DEFINITION_NOT_FOUND  237
set STRUCT_MSG_ERR_DEFINITION_TOO_MANY_FIELDS 236
set STRUCT_MSG_ERR_BAD_TABLE_ROW         235
set STRUCT_MSG_ERR_TOO_MANY_FIELDS       234
set STRUCT_MSG_ERR_BAD_DEFINTION_CMD_KEY 233
set STRUCT_MSG_ERR_NO_SLOT_FOUND         232

# header length: uint16_t src + uint16_t dst + uint16_t totalLgth
set STRUCT_MSG_HEADER_LENGTH [expr {[sizeof uint16_t] * 3}]
# cmd header length uint16_t cmdKey + unit16_t cmdLgth
set STRUCT_MSG_CMD_HEADER_LENGTH [expr {[sizeof uint16_t] * 2}]
set STRUCT_MSG_TOTAL_HEADER_LENGTH [expr {$::STRUCT_MSG_HEADER_LENGTH + $::STRUCT_MSG_CMD_HEADER_LENGTH}]
set STRUCT_MSG_DEFINITION_CMD_KEY 0xFFFF
set STRUCT_MSG_FREE_FIELD_ID 0xFF

set STRUCT_MSG_NO_INCR 0
set STRUCT_MSG_INCR    1
set STRUCT_MSG_DECR    -1


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
  return offset
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
  return $offset
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
  return $offset
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
  return $offset
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
  return $offset
}

# ============================= uint8VectorEncode ========================

proc uint8VectorEncode {dat offset value lgth} {
  upvar $dat data

  append data [string range $value 0 [expr {$lgth - 1}]]
  incr offset $lgth
  return $offset
}

# ============================= int8VectorEncode ========================

proc int8VectorEncode {dat offset value lgth} {
  upvar $dat data

  append data [string range $value 0 [expr {$lgth - 1}]]
  incr offset $lgth
  return offset
}

# ============================= uint16VectorEncode ========================

proc uint16VectorEncode {dat offset value lgth} {
  upvar $dat data

  set idx 0
  while {$idx < $lgth} {
    set offset [uint16Encode data $offset [lindex $value $idx]]
    incr idx
  }
  return $offset
}

# ============================= int16VectorEncode ========================

proc int16VectorEncode {dat offset value lgth} {
  upvar $dat data

  set idx 0
  while {$idx < $lgth} {
    set offset [uint16Encode data $offset [lindex $value $idx]]
    incr idx
  }
  return $offset
}

# ============================= uint32VectorEncode ========================

proc uint32VectorEncode {dat offset value lgth} {
  upvar $dat data

  set idx 0
  while {$idx < $lgth} {
    set offset [uint16Encode data $offset [lindex $value $idx]]
    incr idx
  }
  return $offset
}

# ============================= int32VectorEncode ========================

proc int32VectorEncode {dat offset value lgth} {
  upvar $dat data

  set idx 0
  while {$idx < $lgth} {
    set offset [uint16Encode data $offset [lindex $value $idx]]
    incr idx
  }
  return $offset
}

# ============================= uint8Decode ========================

proc uint8Decode {data offset val} {
  upvar $val value

  set value [expr {[string range $data $offset $offset] & 0xFF}]
  incr offset
  return $offset
}

# ============================= int8Decode ========================

proc int8Decode {data offset val} {
  upvar $val value

  set value [expr {[string range $data $offset $offset] & 0xFF}]
  incr offset
  return $offset
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
  return $offset
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
  return $offset
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
  return $offset
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
  return $offset
}

# ============================= uint8VectorDecode ========================

proc uint8VectorDecode {data offset val len} {
  upvar $val value
  upvar $len lgth

  set value [string range $data $offset [expr {$offset + $lgth}]]
  incr offset $lgth
  return $offset
}

# ============================= int8VectorDecode ========================

proc int8VectorDecode {data offset val len} {
  upvar $val value
  upvar $len lgth

  set value [string range $data $offset [expr {$offset + $lgth}]]
  incr offset $lgth
  return $offset
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
  return $offset
}

# ============================= int16VectorDecode ========================

proc int16VectorDecode {data offset val len} {
  upvar $val value
  upvar $len lgth

  set idx 0
  while {$idx < $lgth} {
    set offset [int16Decode data $offset [lindex $value $idx]]
    incr idx
  }
  return $offset
}

# ============================= uint32VectorDecode ========================

proc uint32VectorDecode {data offset val len} {
  upvar $val value
  upvar $len lgth

  set idx 0
  while {$idx < $lgth} {
    set offset [uint32Decode data $offset [lindex $value $idx]]
    incr idx
  }
  return $offset
}

# ============================= int32VectorDecode ========================

proc int32VectorDecode {data offset val len} {
  upvar $val value
  upvar $len lgth

  set idx 0
  while {$idx < $lgth} {
    set offset [int32Decode data $offset [lindex value $idx]]
    incr idx
  }
  return $offset
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

# ============================= sequenceNumEncode ========================

proc sequenceNumEncode {dat offset dictVar val} {
  upvar $dat data
  upvar $val value
  upvar $dictVar myDict

  dict set myDict sequenceNum [expr {[dict get $myDict sequenceNum] + 1}]
  set myVal [dict get $myDict sequenceNum]
  set value $myVal
  return [uint32Encode data $offset $myVal]
}

# ============================= sequenceNumDecode ========================

proc sequenceNumDecode {data offset val} {
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
  return $offset
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

  set lgth [expr {$lgth - 2}]   ; # uint16_t crc
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
#puts stderr "crc1: $crc![format 0x%04x $crc]!"
  set crc [expr {~$crc & 0xFFFF}]
puts stderr "crc: $crc![format 0x%04x $crc]!"
  set offset [uint16Encode data $offset $crc]
  set value $crc
  return $offset
}

# ============================= crcDecode ========================

proc crcDecode {data offset lgth val headerOffset} {
  upvar $val value

  set value ""
  set lgth [expr {$lgth - 2}] ; # uint16_t crc
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
#puts stderr "crcVal end: $crcVal!"
  set crcVal [expr {~$crcVal & 0xFFFF}]
  set offset [uint16Decode $data $offset crc]
puts stderr "crcVal: [format 0x%04x $crcVal]!offset: $offset!crc: [format 0x%04x $crc]!"
  if {$crcVal != $crc} {
    return -1
  }
  set value $crc
  return $offset
}

# ============================= getFieldIdName ========================

proc getFieldIdName {id fieldNameVar} {
  upvar $fieldNameVar fieldName

  # find field name
  set idx 0
  set fieldNameDefinitions $::structmsg(fieldNameDefinitions)
  while {$idx < [dict get $$fieldNameDefinitions numDefinitions]} {
    set entry [lindex [dict get $fieldNameDefinitions definitions] $idx]
    if {[dict get $entry id] == $id} {
       set fieldName [dict get $entry str]
       return $::STRUCT_MSG_ERR_OK
    }
    incr idx
  }
  error "field not found"
#  return $::STRUCT_MSG_ERR_FIELD_NOT_FOUND
}

# ============================= normalFieldNamesEncode ========================

proc normalFieldNamesEncode {data offset normNameOffsets numEntries size} {
  # first the keys
  set offset [uint8Encode data $offset $numEntries]
  set idx 0
  set normNameOffset [lindex $normNameOffsets 0]
  while {$idx < [dict get $definition numFields]} {
    set fieldInfo [lindex [dict get $definition fieldInfos] $idx]
    if {[dict get $fieldInfo fieldId] < $::STRUCT_MSG_SPEC_FIELD_LOW} { 
      set result [getFieldIdName [dict get $fieldInfo fieldId] fieldName]
#      checkErrOK(result)
      iset offset [uint16Encode data $offset $namesOffset]
      dict set normNameOffset id [dict get $fieldInfo fieldId]
      dict set normNameOffset offset $namesOffset
      incr normNameOffset
      inct namesOffset [expr {[string length $fieldName] + 1}]
    }
    incr idx
  }
  # and now the names
  set offset [uint16Encode data $offset $size]
  set idx 0
  set nameIdx 1
  while {$idx < [dict get $definition numFields]} {
    set fieldInfo [lindex [dict get $definition fieldInfos] $idx]
    if {[dict get $fieldInfo fieldId] < $::STRUCT_MSG_SPEC_FIELD_LOW} { 
      set result [getFieldIdName [dict get $fieldInfo fieldId] fieldName]
#      checkErrOK(result)
      set offset [uint8VectorEncode(data $offset $fieldName [string length $fieldName]]
      if {$nameIdx < $numEntries} {
        set offset [uint8Encode data $offset "\0"]
      } else {
        set offset [uint8Encode data $offset "\0"]
      }
      incr nameIdx
    }
    incr idx
  }
  return $offset
}

# ============================= normalFieldNamesDecode ========================

proc normalFieldNamesDecode {data offset} {
  return offset
}

# ============================= definitionEncode ========================

proc definitionEncode {data offset normNamesOffsets} {
  set idx 0
  set fieldInfo [lindex [dict get $definition fieldInfos] 0]
  set offset [uint8Encode data $offset [dict get $definition numFields]]
#  checkEncodeOffset(offset)
  while {$idx < [dict get $definition numFields]} {
    if {[dict get $fieldInfo fieldId] < $::STRUCT_MSG_SPEC_FIELD_LOW} {
      set idIdx 0
      set found 0
      while {$idIdx < [dict get $definition numFields]} {
        if {[dict get [lindex $normNamesOffsets $idIdx] id] == 0} {
          # id 0 is not used to be able to stop here!!
          break
        }
        if {[dict get fieldInfo fieldId] == [dict get [lindex $normNamesOffsets $idIdx] id]} {
          set nameOffset [lindex $normNamesOffsets $idIdx] offset]
          set found 1
          break
        }
        incr idIdx
      }
      if {!$found} {
        return $::STRUCT_MSG_ERR_FIELD_NOT_FOUND
      }
      set offset [uint16Encode data $offset $nameOffset]
    } else {
      set offset [uint16Encode data $offset [dict get $fieldInfo fieldId]]
    }
    set offset [uint8Encode data $offset [dict get $fieldInfo fieldType]]
    set offset [uint16Encode data $offset [dict get $fieldInfo fieldLgth]]
    incr fieldInfo
    incr idx
  }
  return $offset
}

# ============================= definitionDecode ========================

proc definitionDecode {data offset} {
  # first the keys
  set offset [uint8Decode $data $offset numNameEntries]
#  checkBadOffset(offset)
  set idsStart $offset
  incr offset [expr {$numNameEntries * [sizeof uint16_t]}]
  set idsEnd $offset
  # and now the names
  set offset [uint16Decode $data $offset namesSize]
#  checkBadOffset(offset)
  set namesStart $offset
  incr offset $namesSize
  set definitionStart [expr {$namesStart + $namesSize}]
  set offset [uint16Decode $data $offset definitionLgth]
#  checkBadOffset(offset)
  set offset [uint8Decode $data $offset nameLgth]
#  checkBadOffset(offset)
  set name [string range $data $offset [expr {$offset + $nameLgth - 1}]]
  incr offset $nameLgth
  set offset [uint8Decode $data $offset numFields]
#  checkBadOffset(offset)
  set result [structmsg_createStructmsgDefinition $name $numFields]
#  checkOffsetErrOK(result)
  set definitionIdx 0
  set namesIdx 0
  while {$definitionIdx < $numFields} { 
    set offset [uint16Decode $data $offset fieldId]
#    checkBadOffset(offset)
    if {$fieldId > $::STRUCT_MSG_SPEC_FIELD_LOW} {
      set result [structmsg_getIdFieldNameStr $fieldId fieldName]
#      checkOffsetErrOK(result)
    } else {
      set fieldId [expr {$namesIdx + 1}]
      set myOffset [expr {$idsStart + ($namesIdx * [sizeof uint16_t])}]
      set myOffset [uint16Decode $data $myOffset nameOffset]
      set myStartIdx [expr {$namesStart + $nameOffset}]
      set myEndIdx $myStartIdx
      incr myEndIdx xxx
      set fieldName [string range $data [expr {$namesStart + $nameOffset}] $myEndidx]
      incr namesIdx
    }
    set offset [uint8Decode $data $offset fieldTypeId]
#    checkBadOffset(offset)
    set result [structmsg_getFieldTypeStr $fieldTypeId fieldType]
#    checkOffsetErrOK(result)
    set offset [uint16Decode $data $offset fieldLgth]
#    checkBadOffset(offset)
#puts stderr [format "add field: %s fieldId: %d fieldType: %d  %s fieldLgth: %d offset: %d" $fieldName $fieldId $fieldTypeId $fieldType $fieldLgth $offset]
    set result [structmsg_addFieldDefinition  $name $fieldName $fieldType $fieldLgth]
#    checkOffsetErrOK(result)
    incr definitionIdx
  }
  return $offset
}

# ============================= structmsg_fillHdrInfo ========================

proc structmsg_fillHdrInfo {handle structmsg} {
  # fill the hdrInfo
  set structmsg [structmsg_get_structmsg_ptr $handle]
  hdrInfoPtr [dict get $structmsg handleHdrInfoPtr]
  set hdrInfoPtr [dict get structmsg hdr hdrInfo]
  set offset 0
  set offset [uint16Encode hdrInfoPtr->hdrId $offset [dict get $structmsg hdr hdrInfo hdrKeys src]]
#  checkEncodeOffset(offset)
  set offset [uint16Encode hdrInfoPtr->hdrId $offset [dict get $structmsg hdr hdrInfo hdrKeys dst]]
#  checkEncodeOffset(offset)
  set offset [uint16Encode hdrInfoPtr->hdrId $offset [dict get $structmsg hdr hdrInfo hdrKeys totalLgth]]
#  checkEncodeOffset(offset)
  set offset [uint16Encode hdrInfoPtr->hdrId $offset [dict get $structmsg hdr hdrInfo hdrKeys cmdKey]]
#  checkEncodeOffset(offset)
  set offset [uint16Encode hdrInfoPtr->hdrId $offset [dict get $structmsg hdr hdrInfo hdrKeys cmdLgth]]
#  checkEncodeOffset(offset)
  return $::STRUCT_MSG_ERR_OK
}

# ============================= encodeField ========================

proc encodeField {msgPtr fieldInf offset} {
  set cmd "
  switch [dict get $fieldInfo fieldType] {
    $::STRUCT_MSG_FIELD_INT8_T {
      set offset [int8Encode msgPtr $offset [dict get $fieldInfo value byteVal]]
    }
    $::STRUCT_MSG_FIELD_UINT8_T {
      set offset [uint8Encode msgPtr $offset [dict get $fieldInfo value ubyteVal]]
    }
    $::STRUCT_MSG_FIELD_INT16_T {
      set offset [int16Encode msgPtr $offset [dict get $fieldInfo value shortVal]]
    }
    $::STRUCT_MSG_FIELD_UINT16_T {
      set offset [uint16Encode msgPtr $offset [dict get $fieldInfo value ushortVal]]
    }
    $::STRUCT_MSG_FIELD_INT32_T {
      set offset [int32Encode msgPtr $offset [dict get $fieldInfo value val]]
    }
    $::STRUCT_MSG_FIELD_UINT32_T {
      set offset [uint32Encode msgPtr $offset [dict get $fieldInfo value uval]]
    }
    $::STRUCT_MSG_FIELD_INT8_VECTOR {
      set fieldIdx 0
      while {$fieldIdx < [dict get $fieldInfo fieldLgth]} {
        set offset [int8Encode msgPtr $offset [string range [dict get $fieldInfo value byteVector] $fieldIdx $fieldIdx]]
        incr fieldIdx
      }
    }
    $::STRUCT_MSG_FIELD_UINT8_VECTOR {
      set fieldIdx 0
      while {$fieldIdx < [dict get $fieldInfo fieldLgth]} {
        set offset [uint8Encode msgPtr $offset [string range [dict get $fieldInfo value ubyteVector] $fieldIdx $fieldIdx]]
        incr fieldIdx
      }
    }
    $::STRUCT_MSG_FIELD_INT16_VECTOR {
      set fieldIdx 0
      while {$fieldIdx < [dict get $fieldInfo fieldLgth]} {
        set offset [int16Encode msgPtr $offset [string range [dict get $fieldInfo value shortVector] $fieldIdx $fieldIdx]]
        incr fieldIdx
      }
    }
    $::STRUCT_MSG_FIELD_UINT16_VECTOR {
      set fieldIdx 0
      while {$fieldIdx < [dict get $fieldInfo fieldLgth]} {
        offset [uint16Encode msgPtr $offset [string range [dict get $fieldInfo value ushortVector] $fieldIdx $fieldIdx]]
        incr fieldIdx
      }
    }
    $::STRUCT_MSG_FIELD_INT32_VECTOR {
      set fieldIdx 0
      while {$fieldIdx < [dict get $fieldInfo fieldLgth]} {
        set offset [int32Encode msgPtr $offset [string range [dict get $fieldInfo value int32Vector] $fieldIdx $fieldIdx]]
        incr fieldIdx
      }
    }
    $::STRUCT_MSG_FIELD_UINT32_VECTOR {
      set fieldIdx 0
      while {$fieldIdx < [dict get $fieldInfo->fieldLgth]} {
        set offset [uint32Encode msgPtr $offset [string range [dict get $fieldInfo value uint32Vector] $fieldIdx $fieldIdx]]
        incr fieldIdx
      }
    }
  }"
  uplevel 0 $cmd
  return $offset
}

# ============================= decodeField ========================

proc decodeField {msgPtr fieldInfo offset} {
  set cmd "
  switch [dict get $fieldInfo fieldType] {
    $::STRUCT_MSG_FIELD_INT8_T {
      set offset [int8Decode $msgPtr $offset fieldInfo->value.byteVal]
#      checkDecodeOffset(offset)
    }
    $::STRUCT_MSG_FIELD_UINT8_T {
      set offset [uint8Decode $msgPtr $offset fieldInfo->value.ubyteVal]
#      checkDecodeOffset(offset)
    }
    $::STRUCT_MSG_FIELD_INT16_T {
      set offset [int16Decode $msgPtr $offset fieldInfo->value.shortVal]
#      checkDecodeOffset(offset)
    }
    $::STRUCT_MSG_FIELD_UINT16_T {
      set offset [uint16Decode $msgPtr $offset fieldInfo->value.ushortVal]
#      checkDecodeOffset(offset)
    }
    $::STRUCT_MSG_FIELD_INT32_T {
      set offset [int32Decode $msgPtr $offset fieldInfo->value.val]
#      checkDecodeOffset(offset)
    }
    $::STRUCT_MSG_FIELD_UINT32_T {
      set offset [uint32Decode $msgPtr $offset fieldInfo->value.uval]
#      checkDecodeOffset(offset)
    }
    $::STRUCT_MSG_FIELD_INT8_VECTOR {
      set offset [int8VectorDecode $msgPtr $offset [dict get $fieldInfo->fieldLgth] fieldInfo->value.byteVector]
#      checkDecodeOffset(offset)
    }
    $::STRUCT_MSG_FIELD_UINT8_VECTOR {
      set offset [uint8VectorDecode $msgPtr $offset [dict get $fieldInfo->fieldLgth] fieldInfo->value.ubyteVector]
#      checkDecodeOffset(offset)
    }
    $::STRUCT_MSG_FIELD_INT16_VECTOR {
      set offset [int16VectorDecode $msgPtr $offset [dict get $fieldInfo->fieldLgth] fieldInfo->value.shortVector]
#      checkDecodeOffset(offset)
    }
    $::STRUCT_MSG_FIELD_UINT16_VECTOR {
      set offset [uint16VectorDecode $msgPtr $offset [dict get $fieldInfo->fieldLgth] fieldInfo->value.ushortVector]
#      checkDecodeOffset(offset)
    }
    $::STRUCT_MSG_FIELD_INT32_VECTOR {
      set offset [int32VectorDecode $msgPtr $offset [dict get $fieldInfo->fieldLgth] fieldInfo->value.int32Vector]
#      checkDecodeOffset(offset)
    }
    $::STRUCT_MSG_FIELD_UINT32_VECTOR {
      set offset [uint32VectorDecode $msgPtr $offset [dict get $fieldInfo->fieldLgth] fieldInfo->value.uint32Vector]
#      checkDecodeOffset(offset)
    }
  }
  "
  uplevel 0 $cmd
  return $offset
}

# ============================= stmsg_encodeMsg ========================

proc stmsg_encodeMsg {handle} {
  set structmsg [structmsg_get_structmsg_ptr $handle]
#  checkHandleOK(structmsg)
  dict set structmsg encoded [list]
  set msgPtr [dict get structmsg encoded]
  set offset 0
  set offset [uint16Encode msgPtr $offset [dict get $structmsg hdr hdrInfo hdrKeys src]]
#  checkEncodeOffset(offset)
  set offset [uint16Encode msgPtr $offset [dict get $structmsg hdr hdrInfo hdrKeys dst]]
#  checkEncodeOffset(offset)
  set offset [uint16Encode msgPtr $offset [dict get $structmsg hdr hdrInfo hdrKeys totalLgth]]
#  checkEncodeOffset(offset)
  set offset [uint16Encode msgPtr $offset [dict get $structmsg hdr hdrInfo hdrKeys cmdKey]]
#  checkEncodeOffset(offset)
  set offset [uint16Encode msgPtr $offset [dict get $structmsg hdr hdrInfo hdrKeys cmdLgth]]
#  checkEncodeOffset(offset)
  set numEntries [dict get $structmsg msg numFieldInfos]
  set offset [uint8Encode msgPtr $offset,numEntries)
#  checkEncodeOffset(offset)
  set idx 0
  while {$idx < $numEntries} {
    set fieldInfo [lindex [dict get $structmsg msg fieldInfos] $idx]
    if {[string range [dict get $fieldInfo fieldStr] 0 0] eq "@"} {
      set result [structmsg_getFieldNameId [dict get $fieldInfo fieldStr] fieldId $::STRUCT_MSG_NO_INCR]
#      checkErrOK(result)
      set cmd "
      switch $fieldId {
      $::STRUCT_MSG_SPEC_FIELD_SRC -
      $::STRUCT_MSG_SPEC_FIELD_DST -
      $::STRUCT_MSG_SPEC_FIELD_TARGET_CMD -
      $::STRUCT_MSG_SPEC_FIELD_TOTAL_LGTH -
      $::STRUCT_MSG_SPEC_FIELD_CMD_KEY -
      $::STRUCT_MSG_SPEC_FIELD_CMD_LGTH {
puts stderr [format "funny should encode: %s" [dict get $fieldInfo fieldStr]]
      }
      $::STRUCT_MSG_SPEC_FIELD_RANDOM_NUM {
        set offset [randomNumEncode msgPtr $offset randomNum]
#        checkEncodeOffset(offset)
        set result [stmsg_setFieldValue $handle "@randomNum" $randomNum ""]
#        checkErrOK(result)
      }
      $::STRUCT_MSG_SPEC_FIELD_SEQUENCE_NUM {
        set offset [sequenceNumEncode msgPtr $offset $structmsg $sequenceNum]
#        checkEncodeOffset(offset)
        set result [stmsg_setFieldValue $handle "@sequenceNum" $sequenceNum ""]
#        checkErrOK(result)
      }
      $::STRUCT_MSG_SPEC_FIELD_FILLER {
        set offset [fillerEncode $msgPtr $offset [dict get $fieldInfo fieldLgth] [dict get $fieldInfo value ubyteVector]]
#        checkEncodeOffset(offset)
        set result [stmsg_setFieldValue $handle "@filler" 0 [dict get $fieldInfo valuer .ubyteVector]]
#        checkErrOK(result)
      }
      $::STRUCT_MSG_SPEC_FIELD_CRC {
        set offset [crcEncode structmsg->encoded $offset [dict get $structmsg hdr hdrInfo hdrKeys totalLgth] crc [dict get $structmsg hdr headerLgth]]
#        checkEncodeOffset(offset)
        set result [stmsg_setFieldValue $handle "@crc" $crc ""]
#        checkErrOK(result)
      }
      $::STRUCT_MSG_SPEC_FIELD_ID {
        return $::STRUCT_MSG_ERR_BAD_SPECIAL_FIELD
      }
      $::STRUCT_MSG_SPEC_FIELD_TABLE_ROWS {
      }
      $::STRUCT_MSG_SPEC_FIELD_TABLE_ROW_FIELDS {
        if {[dict get $structmsg msg numTableRows] > 0} {
          set row 0
          set col 0
          set cell 0
          while (row < structmsg->msg.numTableRows) {
            while {$col < [dict get $structmsg msg numRowFields]} {
               set cell [expr {$col + $row * [dict get $structmsg msg numRowFields]}]
               set fieldInfo [lindex [dict get $structmsg msg tableFieldInfos] $cell]
               set offset [encodeField msgPtr $fieldInfo $offset]
#               checkEncodeOffset(offset)
               incr col
            }
            incr row
            set col 0
          }
        }
      }
      }
      "
      uplevel 0 $cmd
    } else {
      set offset [encodeField msgPtr $fieldInfo $offset]
#      checkEncodeOffset(offset)
    }
    incr idx
  }
  dict lappend structmsg flags STRUCT_MSG_ENCODED
  return $::STRUCT_MSG_ERR_OK
}

# ============================= stmsg_getEncoded ========================

proc stmsg_getEncoded {handle encodedVar lgthVar} {
  upvar $encodedVar encoded
  upvar $lgthVar lgth

  set structmsg [structmsg_get_structmsg_ptr $handle]
#  checkHandleOK(structmsg)
  if {[dict get $structmsg encoded] eq ""} {
    return $::STRUCT_MSG_ERR_NOT_ENCODED
  }
  set encoded [dict get $structmsg encoded]
  set lgth [dict get $structmsg hdr hdrInfo hdrKeys totalLgth]
  return $::STRUCT_MSG_ERR_OK
}

# ============================= stmsg_decodeMsg ========================

proc stmsg_decodeMsg {handle data} {
  set structmsg [structmsg_get_structmsg_ptr $handle]
#  checkHandleOK(structmsg)
  set structmsg todecode [string range $data 0 [dict get $structmsg hdr hdrInfo hdrKeys totalLgth]]
  set msgPtr [dict get structmsg todecode]
  set offset 0
  set offset [uint16Decode $msgPtr $offset structmsg->hdr.hdrInfo.hdrKeys.src]
#  checkDecodeOffset(offset)
#  checkErrOK(result)
  set offset [uint16Decode $msgPtr $offset structmsg->hdr.hdrInfo.hdrKeys.dst]
#  checkDecodeOffset(offset)
#  checkErrOK(result)
  set offset [uint16Decode $msgPtr $offset structmsg->hdr.hdrInfo.hdrKeys.totalLgth]
#  checkDecodeOffset(offset)
#  checkErrOK(result)
  set offset [uint16Decode $msgPtr $offset structmsg->hdr.hdrInfo.hdrKeys.cmdKey]
#  checkDecodeOffset(offset)
#  checkErrOK(result)
  set offset [uint16Decode $msgPtr $offset structmsg->hdr.hdrInfo.hdrKeys.cmdLgth]
#  checkDecodeOffset(offset)
  set result [structmsg_fillHdrInfo $handle $structmsg]
  set offset [uint8Decode $msgPtr $offset numEntries]
#  checkDecodeOffset(offset)
  set idx 0
#  numEntries = structmsg->msg.numFieldInfos
  while {$idx < $numEntries} {
    set fieldInfo [lindex [dict get $structmsg msg fieldInfos] $idx]
    if {[string range [dict get $fieldInfo fieldStr] 0 0] eq "@"} {
      set result [structmsg_getFieldNameId [dict get $fieldInfo fieldName] fieldId $::STRUCT_MSG_NO_INCR]
#      checkErrOK(result)
      set cmd "
      switch $fieldId {
      $::STRUCT_MSG_SPEC_FIELD_SRC -
      $::STRUCT_MSG_SPEC_FIELD_DST -
      $::STRUCT_MSG_SPEC_FIELD_TARGET_CMD -
      $::STRUCT_MSG_SPEC_FIELD_TOTAL_LGTH -
      $::STRUCT_MSG_SPEC_FIELD_CMD_KEY -
      $::STRUCT_MSG_SPEC_FIELD_CMD_LGTH {
puts stderr [format "funny should decode: %s" [dict get $fieldInfo fieldName]]
      }
      $::STRUCT_MSG_SPEC_FIELD_RANDOM_NUM {
        set offset [randomNumDecode $msgPtr $offset fieldInfo->value.uval]
#        checkDecodeOffset(offset)
      }
      $::STRUCT_MSG_SPEC_FIELD_SEQUENCE_NUM {
        set offset [sequenceNumDecode $msgPtr $offset fieldInfo->value.uval]
#        checkDecodeOffset(offset)
      }
      $::STRUCT_MSG_SPEC_FIELD_FILLER {
        set offset [fillerDecode $msgPtr $offset [dict get $fieldInfo fieldLgth] fieldInfo->value.ubyteVector]
#        checkDecodeOffset(offset)
      }
      $::STRUCT_MSG_SPEC_FIELD_CRC {
        set offset [crcDecode $msgPtr $offset [dict get $structmsg hdr hdrInfo hdrKeys cmdLgth] fieldInfo->value.ushortVal [dict get $structmsg hdr headerLgth]]
        if {$offset < 0} {
          return $::STRUCT_MSG_ERR_BAD_CRC_VALUE
        }
      }
      $::STRUCT_MSG_SPEC_FIELD_ID {
        return $::STRUCT_MSG_ERR_BAD_SPECIAL_FIELD
      }
      $::STRUCT_MSG_SPEC_FIELD_TABLE_ROWS {
      }
      $::STRUCT_MSG_SPEC_FIELD_TABLE_ROW_FIELDS {
        if {[dict get $structmsg msg numTableRows] > 0} {
          set row 0
          set col 0
          set cell 0
          while {$row < [dict get $structmsg msg numTableRows]} {
            while {$col < [dict get $structmsg msg numRowFields]} {
               set cell [expr {$col + $row * [dict get $structmsg msg numRowFields]}]
               set fieldInfo [lindex [dict get $structmsg msg tableFieldInfos] $cell]
               set offset [decodeField $msgPtr $fieldInfo $offset)
#               checkEncodeOffset(offset)
               incr col
            }
            incr row
            set col 0
          }
        }
      }
      }
      "
      uplevel 0 $cmd
      dict lappend fieldInfo flags STRUCT_MSG_FIELD_IS_SET
    } else {
      set offset [decodeField $msgPtr $fieldInfo $offset]
#      checkEncodeOffset(offset)
      dict lappend fieldInfo flags STRUCT_MSG_FIELD_IS_SET
    }
    incr idx
  }
  dict lappend structmsg flags STRUCT_MSG_DECODED
  return $::STRUCT_MSG_ERR_OK
}

# ============================= getSpecFieldSizes ========================

proc getSpecFieldSizes {numFieldsVar namesSizeVar} {
  upvar $numFieldVars numFields
  upvar $namesSizeVar namesSize

  set numFields 0
  set namesSize 0
  foreach entry $::structmsg(structmsgSpecialFieldNames) {
    if {[dict get $entry fieldName] ne ""} {
      incr numFields
      set namesSize [expr {$namesSize + [string length [dict get $entry fieldName]] + 1}] ;  # +1 for "\0" as separator
#puts [format "%s: %d %d" [expr {[dict get $entry fieldName] * $numFields}] * $namesSize]
    }
  }
  return $::STRUCT_MSG_ERR_OK
}

# ============================= structmsg_encodeDefinition ========================

proc structmsg_encodeDefinition {name data lgth} {
  set src 123
  set dst 987

  set idx 0
  while {$idx < [dict get $::strucmsg(structmsgDefinitions) numDefinitions]} {
    set definition [lindex [dict get $::strucmsg(structmsgDefinitions) definitions] idx]
    if {$name  eq [dict get $definition name]} {
      set found 1
      break
    }
    incr idx
  }
  if {!$found} {
    return $::STRUCT_MSG_ERR_DEFINITION_NOT_FOUND
  }
  set numNormFields 0
  set normNamesSize 0
  set idx 0
  while {$idx < [dict get $definition numFields]} {
    set fieldInfo [lindex [dict get $definition fieldInfos] $idx]
    if {[dict get $fieldInfo fieldId] < $::STRUCT_MSG_SPEC_FIELD_LOW} {
      set result [getFieldIdName [dict get $fieldInfo fieldId] fieldName]
#      checkErrOK(result)
#puts stderr [format "fieldName: %s" $fieldName]
      incr numNormFields
      set normNamesSize [expr {$normnamesSize + [string length $fieldName] + 1}]
    }
    incr idx
  }
  set normNamesOffsets [list]
#  checkAllocOK(normNamesOffsets)
  # nameLgth + name of Definition
  set definitionPayloadSize [expr {[sizeof uint8_t] + [string length $name] + 1}]
  # fieldId uint16_t, fieldType uint8_t, fieldLgth uint16_t
  set definitionPayloadSize [expr {$definitionPayloadSize + [dict get $definition numFields] * [sizeof uint16_t] + [sizeof uint8_t] + [sizeof uint16_t]}]
  set payloadSize $::STRUCT_MSG_CMD_HEADER_LENGTH ; # cmdKey + cmdLgth
  # numEntries uint8_t randomNum
  set payloadSize [expr {$paylaodSize + [sizeof uint8_t] + [sizeof uint32_t]}]
  # len ids + ids (numNormFields * (uint16_t)) + len Names + names size
  set payloadSize [expr {$paylaodSize + [sizeof uint8_t] + $numNormFields * [sizeof uint16_t] + [sizeof uint16_t] + $normNamesSize}]
  # size definitionPayload + definitionPayload
  set payloadSize [expr {$payloadSize + [sizeof uint16_t] + $definitionPayloadSize]}]
  set fillerSize 0
  set myLgth [expr {$payloadSize + [sizeof uint16_t]}] ; # sizeof(uint16_t) for CRC
  while {($myLgth % 16) != 0} {
    incr myLgth
    incr fillerSize
  }
  set cmdLgth [expr {$payloadSize + $fillerSize + [sizeof uint16_t]}]
  set totalLgth [expr {$::STRUCT_MSG_HEADER_LENGTH + $cmdLgth}]
  dict set definition encoded [lsit]
#  checkAllocOK(definition->encoded)
  set encoded [dict get $definition encoded]
  set offset 0
  set offset [uint16Encode encoded $offset src]
#  checkEncodeOffset(offset)
  set offset [uint16Encode encoded $offset dst]
#  checkEncodeOffset(offset)
  set offset [uint16Encode encoded $offset totalLgth]
#  checkEncodeOffset(offset)
  set offset [uint16Encode encoded $offset cmdKey]
#  checkEncodeOffset(offset)
  set offset [uint16Encode encoded $offset cmdLgth]
#  checkEncodeOffset(offset)
  set offset [randomNumEncode encoded $offset randomNum]
#  checkEncodeOffset(offset)
  set offset [normalFieldNamesEncode encoded $offset $definition normNamesOffsets $numNormFields $normNamesSize]
  set offset [uint16Encode encoded $offset $definitionPayloadSize]
#  checkEncodeOffset(offset)
  set offset [uint8Encode encoded $offset [exxpr {[string length $name] + 1}]]
#  checkEncodeOffset(offset)
  set offset [uint8VectorEncode encoded $offset $name [string length $name ]]
#  checkEncodeOffset(offset)
  set offset [uint8Encode encoded $offset "\0"]
#  checkEncodeOffset(offset)
  set offset [definitionEncode encoded $offset $definition $normNamesOffsets]
#  checkEncodeOffset(offset)
  set offset [fillerEncode encoded $offset $fillerSize dummy]
#  checkEncodeOffset(offset)
  set offset [crcEncode encoded $offset $totalLgth crc $::STRUCT_MSG_HEADER_LENGTH]
#  checkEncodeOffset(offset)
pust stderr [format "after crc offset: %d totalLgth :%d crc: 0x%04x" $offset $totalLgth $crc]
  set data $encoded
  set lgth $totalLgth
  return $::STRUCT_MSG_ERR_OK
}

# ============================= structmsg_decodeDefinition ========================

proc structmsg_decodeDefinition {name data} {
  set offset 0
  set offset [uint16Decode $data $offset src]
#  checkDecodeOffset(offset)
  set offset [uint16Decode $data $offset dst]
#  checkDecodeOffset(offset)
  set offset [uint16Decode $data $offset totalLgth]
#  checkDecodeOffset(offset)
  set offset [uint16Decode $data $offset cmdKey]
#  checkDecodeOffset(offset)
  if {$cmdKey != $::STRUCT_MSG_DEFINITION_CMD_KEY} {
    return $::STRUCT_MSG_ERR_BAD_DEFINTION_CMD_KEY
  }
  set offset [uint16Decode $data $offset cmdLgth]
#  checkDecodeOffset(offset)
  set offset [uint32Decode $data $offset randomNum]
#  checkDecodeOffset(offset)
  # now check the crc
  set crcOffset [expr {$totalLgth - [sizeof uint16_t]}]
  set crcOffset [crcDecode $data $crcOffset $cmdLgth crc $::STRUCT_MSG_HEADER_LENGTH]
  set offset [definitionDecode $ddata $offset]
#  checkDecodeOffset(offset)
  set myLgth [expr {$offset + [sizeof uint16_t]}]
  set fillerSize 0
  while {($myLgth % 16) != 0} {
    incr myLgth
    incr fillerSize
  }
  set offset [fillerDecode $data $offset $fillerSize filler]
#  checkDecodeOffset(offset)
  return $::STRUCT_MSG_ERR_OK
}

# ============================= structmsg_deleteDefinition ========================

proc structmsg_deleteDefinition {name} {
  set idx 0
  set found 0
  while {$idx < [dict get $::strucmsg(structmsgDefinitions) numDefinitions]} {
    set definition [lindex [dict get $::structmsg(structmsgDefinitions) definitions] $idx]
    if {([dict get $definition name] ne "") && ($name ne [dict get $definition name]} {
      set found 1
      break
    }
    incr idx
  }
  if {!$found} {
    return $::STRUCT_MSG_ERR_DEFINITION_NOT_FOUND
  }
  set idx 0
  while {$idx < [dict get $definition numFields]} {
    set fieldInfo [lindex [dict get $definition fieldInfos] $idx]
    set nameIdx 0
    set nameFound 0
    if {[dict get $fieldInfo fieldId] < $::STRUCT_MSG_SPEC_FIELD_LOW} {
      while {$nameIdx < [dict get $::strucmsg(fieldNameDefinitions) numDefinitions]} {
        set nameEntry [lindex [dict get $::structmsg(fieldNameDefinitions) definitions] $nameIdx]
        if {[dict get $fieldInfo fieldId] == [dict get $nameEntry id]} {
          set result [structmsg_getFieldNameId [dict get $nameEntry fieldName] fieldId $::STRUCT_MSG_DECR]
#          checkErrOK(result)
          set nameFound 1
          break
        }
        incr nameIdx
      }
      if {!$nameFound} {
        return $::STRUCT_MSG_ERR_FIELD_NOT_FOUND
      }
    }
    incr idx
  }
  # nameDefinitions deleted

  dict set definition->numFields 0
  dict set definition->maxFields 0
  dict set definition name ""
  if {[dict get $definition encoded] ne ""} {
    dict set definition encoded ""
  }
  dict set definition fieldInfos ""
  # definition deleted

  return $::STRUCT_MSG_ERR_OK
}

# ============================= structmsg_deleteDefinitions ========================

proc structmsg_deleteDefinitions {} {
  # delete the whole structmsgDefinitions info, including fieldNameDefinitions info

  set idx 0
  while {$idx < [dict get $::structmsg(structmsgDefinitions) numDefinitions]} {
    set definition [dict get $::structmsg(structmsgDefinitions) definitions] $idx]
    if {[dict get $definition name] ne ""} {
      structmsg_deleteDefinition [dict get $definition name]
    }
    incr idx
  }
  dict set ::structmsg(structmsgDefinitions) numDefinitions 0
  dict set ::structmsg(structmsgDefinitions) maxDefinitions 0
  dict set ::structmsg(structmsgDefinitions) definitions [list]

  dict set ::structmsg(fieldNameDefinitions) numDefinitions 0
  dict set ::structmsg(fieldNameDefinitions) maxDefinitions 0
  dict set ::structmsg(fieldNameDefinitions) definitions [list]

  # all deleted/reset
  return $::STRUCT_MSG_ERR_OK
}

