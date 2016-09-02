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

namespace eval structmsg {
  namespace ensemble create

    namespace export encdec

  namespace eval encdec {
    namespace ensemble create
      
    namespace export uint8Encode int8Encode uint16Encode int16Encode
    namespace export uint32Encode int32Encode uint8VectorEncode int8VectorEncode
    namespace export uint16VectorEncode int16VectorEncode uint32VectorEncode int32VectorEncode
    namespace export uint8Decode int8Decode uint16Decode int16Decode
    namespace export uint32Decode int32Decode uint8VectorDecode int8VectorDecode
    namespace export uint16VectorDecode int16VectorDecode uint32VectorDecode int32VectorDecode
    namespace export randomNumEncode randomNumDecode sequenceNumEncode sequenceNumDecode
    namespace export fillerEncode fillerDecode crcEncode crcDecode fillHdrId
    namespace export encodeField decodeField getFieldIdName normalFieldNamesEncode
    namespace export definitionEncode definitionDecode

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
    
      set ch [string range $data $offset $offset]
      set pch $ch
      if {![string is integer $ch]} {
        binary scan $ch c pch
      }
      set value [expr {$pch & 0xFF}]
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
      set ch [string range $data $offset $offset]
      set pch $ch
      if {![string is integer $ch]} {
        binary scan $ch c pch
      }
      set value [expr {$value + ([expr {$pch & 0xFF}] << 8)}]
      incr offset
      set ch [string range $data $offset $offset]
      set pch $ch
      if {![string is integer $ch]} {
        binary scan $ch c pch
      }
      set value [expr {$value + ([expr {$pch & 0xFF}] << 0)}]
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
      set ch [string range $data $offset $offset]
      set pch $ch
      if {![string is integer $ch]} {
        binary scan $ch c pch
      }
      set value [expr {$value + ([expr {$pch & 0xFF}] << 24)}]
      incr offset
      set ch [string range $data $offset $offset]
      set pch $ch
      if {![string is integer $ch]} {
        binary scan $ch c pch
      }
      set value [expr {$value + ([expr {$pch & 0xFF}] << 16)}]
      incr offset
      set ch [string range $data $offset $offset]
      set pch $ch
      if {![string is integer $ch]} {
        binary scan $ch c pch
      }
      set value [expr {$value + ([expr {$pch & 0xFF}] << 8)}]
      incr offset
      set ch [string range $data $offset $offset]
      set pch $ch
      if {![string is integer $ch]} {
        binary scan $ch c pch
      }
      set value [expr {$value + ([expr {$pch & 0xFF}] << 0)}]
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
    
    proc uint8VectorDecode {data offset val lgth} {
      upvar $val value
    
      set value [string range $data $offset [expr {$offset + $lgth - 1}]]
      incr offset $lgth
      return $offset
    }
    
    # ============================= int8VectorDecode ========================
    
    proc int8VectorDecode {data offset val lgth} {
      upvar $val value
    
      set value [string range $data $offset [expr {$offset + $lgth - 1}]]
      incr offset $lgth
      return $offset
    }
    
    # ============================= uint16VectorDecode ========================
    
    proc uint16VectorDecode {data offset val lgth} {
      upvar $val value
    
      set idx 0
      while {$idx < $lgth} {
        set offset [uint16Decode data $offset, value+idx]
        incr idx
      }
      return $offset
    }
    
    # ============================= int16VectorDecode ========================
    
    proc int16VectorDecode {data offset val lgth} {
      upvar $val value
    
      set idx 0
      while {$idx < $lgth} {
        set offset [int16Decode data $offset [lindex $value $idx]]
        incr idx
      }
      return $offset
    }
    
    # ============================= uint32VectorDecode ========================
    
    proc uint32VectorDecode {data offset val lgth} {
      upvar $val value
    
      set idx 0
      while {$idx < $lgth} {
        set offset [uint32Decode data $offset [lindex $value $idx]]
        incr idx
      }
      return $offset
    }
    
    # ============================= int32VectorDecode ========================
    
    proc int32VectorDecode {data offset val lgth} {
      upvar $val value
    
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
puts stderr "randomnumEncode: myVal: [format 0x%08x $myVal]!"
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
if {$::crcDebug} {
puts stderr "encode crc: $cnt $ch![format 0x%02x $pch]![format 0x%04x $crc]!"
}
        set crc [expr {$crc + [format "%d" $pch]}]
incr cnt
      }
if {$::crcDebug} {
puts stderr "crc1: $crc![format 0x%04x $crc]!"
}
      set crc [expr {~$crc & 0xFFFF}]
      set offset [uint16Encode data $offset $crc]
puts stderr "crc: $crc![format 0x%04x $crc]!offset: $offset!"
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
if {$::crcDebug} {
puts stderr "decode crc: $cnt ch: [format 0x%02x $pch]![format 0x%04x $crcVal]!"
}
        set crcVal [expr {$crcVal + [format "%d" $pch]}]
incr cnt
        incr idx
      }
if {$::crcDebug} {
puts stderr "crcVal end: $crcVal!"
}
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
      while {$idx < [dict get $fieldNameDefinitions numDefinitions]} {
        set entry [lindex [dict get $fieldNameDefinitions definitions] $idx]
        if {[dict get $entry id] == $id} {
           set fieldName [dict get $entry fieldName]
           return $::STRUCT_MSG_ERR_OK
        }
        incr idx
      }
      error "field not found"
    #  return $::STRUCT_MSG_ERR_FIELD_NOT_FOUND
    }
    
    # ============================= normalFieldNamesEncode ========================
    
    proc normalFieldNamesEncode {dataVar offset definition normNameOffsetsVar numEntries size} {
      upvar $dataVar data
      upvar $normNameOffsetsVar normNameOffsets
    
      set normNameOffsets [list]
      # first the keys
      set namesOffset 0
      set offset [uint8Encode data $offset $numEntries]
      set idx 0
      while {$idx < [dict get $definition numFields]} {
        set normNameOffset [lindex $normNameOffsets $idx]
        set fieldInfo [lindex [dict get $definition fieldInfos] $idx]
        if {[dict get $fieldInfo fieldId] < $::STRUCT_MSG_SPEC_FIELD_LOW} { 
          set result [getFieldIdName [dict get $fieldInfo fieldId] fieldName]
          if {$result != $::STRUCT_MSG_ERR_OK} { return -1 }
          set offset [uint16Encode data $offset $namesOffset]
          dict set normNameOffset id [dict get $fieldInfo fieldId]
          dict set normNameOffset offset $namesOffset
          lappend normNameOffsets $normNameOffset
          incr namesOffset [expr {[string length $fieldName] + 1}]
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
          if {$result != $::STRUCT_MSG_ERR_OK} { return -1 }
          set offset [uint8VectorEncode data $offset $fieldName [string length $fieldName]]
          if {$nameIdx < $numEntries} {
            set offset [uint8Encode data $offset 0]
          } else {
            set offset [uint8Encode data $offset 0]
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
    
    proc definitionEncode {dataVar offset definition normNamesOffsets} {
      upvar $dataVar data
    
      set idx 0
      set offset [uint8Encode data $offset [dict get $definition numFields]]
      if {$offset < 0} { return $::STRUCT_MSG_ERR_ENCODE_ERROR }
      while {$idx < [dict get $definition numFields]} {
        set fieldInfo [lindex [dict get $definition fieldInfos] $idx]
        if {[dict get $fieldInfo fieldId] < $::STRUCT_MSG_SPEC_FIELD_LOW} {
          set idIdx 0
          set found 0
          while {$idIdx < [dict get $definition numFields]} {
            if {[dict get [lindex $normNamesOffsets $idIdx] id] == 0} {
              # id 0 is not used to be able to stop here!!
              break
            }
            if {[dict get $fieldInfo fieldId] == [dict get [lindex $normNamesOffsets $idIdx] id]} {
              set nameOffset [dict get [lindex $normNamesOffsets $idIdx] offset]
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
        incr idx
      }
      return $offset
    }
    
    # ============================= definitionDecode ========================
    
    proc definitionDecode {data offset} {
      # first the keys
      set offset [::structmsg encdec uint8Decode $data $offset numNameEntries]
      if {$offset < 0} { return $::STRUCT_MSG_ERR_DECODE_ERROR }
      set idsStart $offset
      incr offset [expr {$numNameEntries * [sizeof uint16_t]}]
      set idsEnd $offset
      # and now the names
      set offset [::structmsg encdec uint16Decode $data $offset namesSize]
      if {$offset < 0} { return $offset }
      set namesStart $offset
      incr offset $namesSize
      set definitionStart [expr {$namesStart + $namesSize}]
      set offset [uint16Decode $data $offset definitionLgth]
      if {$offset < 0} { return $offset }
      set offset [uint8Decode $data $offset nameLgth]
      if {$offset < 0} { return $offset }
      set name [string range $data $offset [expr {$offset + $nameLgth - 2}]]
      incr offset $nameLgth
      set offset [uint8Decode $data $offset numFields]
      if {$offset < 0} { return $offset }
      set result [::structmsg def createDefinition $name $numFields]
    #  checkOffsetErrOK(result)
      set definitionIdx 0
      set namesIdx 0
      while {$definitionIdx < $numFields} { 
        set offset [uint16Decode $data $offset fieldId]
        if {$offset < 0} { return $offset }
        if {$fieldId > $::STRUCT_MSG_SPEC_FIELD_LOW} {
          set result [::structmsg def getIdFieldNameStr $fieldId fieldName]
    #      checkOffsetErrOK(result)
        } else {
          set fieldId [expr {$namesIdx + 1}]
          set myOffset [expr {$idsStart + ($namesIdx * [sizeof uint16_t])}]
          set myOffset [::structmsg encdec uint16Decode $data $myOffset nameOffset]
          set myStartIdx [expr {$namesStart + $nameOffset}]
          set myEndIdx $myStartIdx
          while {true} {
            set myEndIdx [uint8Decode $data $myEndIdx ch]
            if {$ch == 0} {
              incr myEndIdx -2 ; # the 0 char and the offset has been moved after the 0 char
              break
            }
          }
          set fieldName [string range $data $myStartIdx $myEndIdx]
          incr namesIdx
        }
        set offset [uint8Decode $data $offset fieldTypeId]
        if {$offset < 0} { return $offset }
        set result [::structmsg def getFieldTypeStr $fieldTypeId fieldType]
        if {$result < 0} { return -1 }
        set offset [::structmsg encdec uint16Decode $data $offset fieldLgth]
        if {$offset < 0} { return $offset }
    #puts stderr [format "add field: %s fieldId: %d fieldType: %d  %s fieldLgth: %d offset: %d" $fieldName $fieldId $fieldTypeId $fieldType $fieldLgth $offset]
        set result [::structmsg def addFieldDefinition  $name $fieldName $fieldType $fieldLgth]
        if {$result < 0} { return -1 }
        incr definitionIdx
      }
      return $offset
    }
    
    # ===================== fillHdrId =============================
    
    proc fillHdrId {handle} {
      if {![info exists ::structmsg($handle)]} {
        error "no such structmsg: $handle"
      }
      set idx 0
      set myDict $::structmsg($handle)
      set myHandles $::structmsg(hdrId2Handles)
      foreach hdrId2Handle $myHandles {
        if {[dict get $hdrId2Handle handle] eq $handle} {
          set hdrKeys [dict get $myDict hdr hdrInfo hdrKeys]
          set hdrId ""
          set offset 0
          set offset [uint16Encode hdrId $offset [dict get $hdrKeys src]]
          set offset [uint16Encode hdrId $offset [dict get $hdrKeys dst]]
          set offset [uint16Encode hdrId $offset [dict get $hdrKeys totalLgth]]
          set offset [uint16Encode hdrId $offset [dict get $hdrKeys cmdKey]]
          set offset [uint16Encode hdrId $offset [dict get $hdrKeys cmdLgth]]
          dict set hdrId2Handle hdrId $hdrId
          set myHandles [lreplace $myHandles $idx $idx $hdrId2Handle]
          set ::structmsg(hdrId2Handles) $myHandles
          break
        }
        incr idx
      }
      return $::STRUCT_MSG_ERR_OK
    }
    
    
    # ============================= fillHdrInfoXX ========================
    
    proc fillHdrInfoXX {handle} {
      # fill the hdrInfo
      set hdrInfo [dict get $::structmsg($handle) hdr hdrInfo]
      set hdrKeys [dict get $hdrInfo hdrKeys]
      set hdrId ""
      set offset 0
      set offset [uint16Encode hdrId $offset [dict get $hdrKeys src]]
      if {$offset < 0} { return $::STRUCT_MSG_ERR_ENCODE_ERROR }
      set offset [uint16Encode hdrId $offset [dict get $hdrKeys dst]]
      if {$offset < 0} { return $::STRUCT_MSG_ERR_ENCODE_ERROR }
      set offset [uint16Encode hdrId $offset [dict get $hdrKeys totalLgth]]
      if {$offset < 0} { return $::STRUCT_MSG_ERR_ENCODE_ERROR }
      set offset [uint16Encode hdrId $offset [dict get $hdrKeys cmdKey]]
      if {$offset < 0} { return $::STRUCT_MSG_ERR_ENCODE_ERROR }
      set offset [uint16Encode hdrId $offset [dict get $hdrKeys cmdLgth]]
      if {$offset < 0} { return $::STRUCT_MSG_ERR_ENCODE_ERROR }
      dict set ::::structmsg($handle) hdr hdrInfo hdrId $hdrId
      return $::STRUCT_MSG_ERR_OK
    }
    
    # ============================= encodeField ========================
    
    proc encodeField {encodedVar fieldInfo offset} {
      upvar $encodedVar encoded
    
      switch [dict get $fieldInfo fieldType] {
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
      return $offset
    }
    
    # ============================= decodeField ========================
    
    proc decodeField {todecode fieldInfoVar offset} {
      upvar $fieldInfoVar fieldInfo
    
      set fieldTypeName [dict get $fieldInfo fieldType]
      switch $fieldTypeName {
        int8_t {
          set offset [int8Decode $todecode $offset value]
        }
        unit8_t {
          set offset [uint8Decode $todecode $offset value]
        }
        int16_t {
          set offset [int16Decode $todecode $offset value]
        }
        unit16_t {
          set offset [uint16Decode $todecode $offset value]
        }
        int32_t {
          set offset [int32Decode $todecode $offset value]
        }
        uint32_tT {
          set offset [uint32Decode $todecode $offset value]
        }
        int8_t* {
          set offset [int8VectorDecode $todecode $offset value [dict get $fieldInfo fieldLgth]]
        }
        uint8_t* {
          set offset [uint8VectorDecode $todecode $offset value [dict get $fieldInfo fieldLgth]]
        }
        int16_t* {
          set offset [int16VectorDecode $todecode $offset value [dict get $fieldInfo fieldLgth]]
        }
        uint16_t* {
          set offset [uint16VectorDecode $todecode $offset value [dict get $fieldInfo fieldLgth]]
        }
        int32_t* {
          set offset [int32VectorDecode $todecode $offset value [dict get $fieldInfo fieldLgth]]
        }
        unit32_t* {
          set offset [uint32VectorDecode $todecode $offset value [dict get $fieldInfo fieldLgth]]
        }
      }
      if {$offset > 0} {
        dict set fieldInfo value $value
      }
      return $offset
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
    
  } ; # namespace encdec
} ; # namespace structmsg
