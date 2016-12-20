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

set ::DATA_VIEW_ERR_OK                    0
set ::DATA_VIEW_ERR_VALUE_NOT_SET         255
set ::DATA_VIEW_ERR_VALUE_OUT_OF_RANGE    254
set ::DATA_VIEW_ERR_BAD_VALUE             253
set ::DATA_VIEW_ERR_BAD_FIELD_TYPE        252
set ::DATA_VIEW_ERR_FIELD_TYPE_NOT_FOUND  251
set ::DATA_VIEW_ERR_VALUE_TOO_BIG         250
set ::DATA_VIEW_ERR_OUT_OF_MEMORY         249
set ::DATA_VIEW_ERR_OUT_OF_RANGE          248
set ::DATA_VIEW_ERR_NO_SUCH_COMMAND       100

namespace eval compMsg {
  namespace ensemble create

    namespace export dataView

  namespace eval dataView {
    namespace ensemble create
      
    namespace export dataView freeDataView setData getFieldTypeIdFromStr getFieldTypeStrFromId appendData getData
    namespace export getFieldTypeIntFromId dumpBinary
    namespace export getUint8 getInt8 setUint8 setInt8
    namespace export getUint16 getInt16 setUint16 setInt16
    namespace export getUint32 getInt32 setUint32 setInt32
    namespace export getUint8Vector getInt8Vector setUint8Vector setInt8Vector
    namespace export getUint16Vector getInt16Vector setUint16Vector setInt16Vector
    namespace export getUint32Vector getInt32Vector setUint32Vector setInt32Vector

    variable lgth 0
    variable data ""
    variable fieldTypeNames2Ids
    set fieldTypeNames2Ids [dict create]
    dict set fieldTypeNames2Ids none      DATA_VIEW_FIELD_NONE
    dict set fieldTypeNames2Ids uint8_t   DATA_VIEW_FIELD_UINT8_T
    dict set fieldTypeNames2Ids int8_t    DATA_VIEW_FIELD_INT8_T
    dict set fieldTypeNames2Ids uint16_t  DATA_VIEW_FIELD_UINT16_T
    dict set fieldTypeNames2Ids int16_t   DATA_VIEW_FIELD_INT16_T
    dict set fieldTypeNames2Ids uint32_t  DATA_VIEW_FIELD_UINT32_T
    dict set fieldTypeNames2Ids int32_t   DATA_VIEW_FIELD_INT32_T
    dict set fieldTypeNames2Ids uint8_t*  DATA_VIEW_FIELD_UINT8_VECTOR
    dict set fieldTypeNames2Ids int8_t*   DATA_VIEW_FIELD_INT8_VECTOR
    dict set fieldTypeNames2Ids uint16_t* DATA_VIEW_FIELD_UINT16_VECTOR
    dict set fieldTypeNames2Ids int16_t*  DATA_VIEW_FIELD_INT16_VECTOR
    dict set fieldTypeNames2Ids uint32_t* DATA_VIEW_FIELD_UINT32_VECTOR
    dict set fieldTypeNames2Ids int32_t*  DATA_VIEW_FIELD_INT32_VECTOR

    variable fieldTypeIds2Names
    set fieldTypeIds2Names [dict create]
    dict set fieldTypeIds2Names DATA_VIEW_FIELD_NONE none   
    dict set fieldTypeIds2Names DATA_VIEW_FIELD_UINT8_T uint8_t   
    dict set fieldTypeIds2Names DATA_VIEW_FIELD_INT8_T int8_t    
    dict set fieldTypeIds2Names DATA_VIEW_FIELD_UINT16_T uint16_t  
    dict set fieldTypeIds2Names DATA_VIEW_FIELD_INT16_T int16_t   
    dict set fieldTypeIds2Names DATA_VIEW_FIELD_UINT32_T uint32_t  
    dict set fieldTypeIds2Names DATA_VIEW_FIELD_INT32_T int32_t   
    dict set fieldTypeIds2Names DATA_VIEW_FIELD_UINT8_VECTOR uint8_t*  
    dict set fieldTypeIds2Names DATA_VIEW_FIELD_INT8_VECTOR int8_t*   
    dict set fieldTypeIds2Names DATA_VIEW_FIELD_UINT16_VECTOR uint16_t* 
    dict set fieldTypeIds2Names DATA_VIEW_FIELD_INT16_VECTOR int16_t*  
    dict set fieldTypeIds2Names DATA_VIEW_FIELD_UINT32_VECTOR uint32_t* 
    dict set fieldTypeIds2Names DATA_VIEW_FIELD_INT32_VECTOR int32_t*  

    variable fieldTypeInts2Ids
    set fieldTypeInts2Ids [dict create]
    dict set fieldTypeInts2Ids 0 DATA_VIEW_FIELD_NONE
    dict set fieldTypeInts2Ids 1 DATA_VIEW_FIELD_UINT8_T
    dict set fieldTypeInts2Ids 2 DATA_VIEW_FIELD_INT8_T
    dict set fieldTypeInts2Ids 3 DATA_VIEW_FIELD_UINT16_T
    dict set fieldTypeInts2Ids 4 DATA_VIEW_FIELD_INT16_T
    dict set fieldTypeInts2Ids 5 DATA_VIEW_FIELD_UINT32_T
    dict set fieldTypeInts2Ids 6 DATA_VIEW_FIELD_INT32_T
    dict set fieldTypeInts2Ids 7 DATA_VIEW_FIELD_UINT8_VECTOR
    dict set fieldTypeInts2Ids 8 DATA_VIEW_FIELD_INT8_VECTOR
    dict set fieldTypeInts2Ids 9 DATA_VIEW_FIELD_UINT16_VECTOR
    dict set fieldTypeInts2Ids 10 DATA_VIEW_FIELD_INT16_VECTOR
    dict set fieldTypeInts2Ids 11 DATA_VIEW_FIELD_UINT32_VECTOR
    dict set fieldTypeInts2Ids 12 DATA_VIEW_FIELD_INT32_VECTOR

    variable fieldTypeIds2Ints
    set fieldTypeInts2Ids [dict create]
    dict set fieldTypeIds2Ints DATA_VIEW_FIELD_NONE 0
    dict set fieldTypeIds2Ints DATA_VIEW_FIELD_UINT8_T 1
    dict set fieldTypeIds2Ints DATA_VIEW_FIELD_INT8_T 2
    dict set fieldTypeIds2Ints DATA_VIEW_FIELD_UINT16_T 3
    dict set fieldTypeIds2Ints DATA_VIEW_FIELD_INT16_T 4
    dict set fieldTypeIds2Ints DATA_VIEW_FIELD_UINT32_T 5
    dict set fieldTypeIds2Ints DATA_VIEW_FIELD_INT32_T 6
    dict set fieldTypeIds2Ints DATA_VIEW_FIELD_UINT8_VECTOR 7
    dict set fieldTypeIds2Ints DATA_VIEW_FIELD_INT8_VECTOR 8
    dict set fieldTypeIds2Ints DATA_VIEW_FIELD_UINT16_VECTOR 9
    dict set fieldTypeIds2Ints DATA_VIEW_FIELD_INT16_VECTOR 10
    dict set fieldTypeIds2Ints DATA_VIEW_FIELD_UINT32_VECTOR 11
    dict set fieldTypeIds2Ints DATA_VIEW_FIELD_INT32_VECTOR 12

    # ================================= dumpBinary ====================================
    
    proc dumpBinary {data lgth what} {
      variable compMsgData

      puts stderr $what
      set idx 0
      foreach ch [split $data ""] {
        set pch $ch
        binary scan $ch c pch
        puts stderr "$idx: $ch [format 0x%02x [expr {$pch & 0xFF}]]!"
        incr idx
        if {$idx > $lgth} {
          break
        }
      }
    }

    # ================================= getFieldTypeIdFromStr ====================================

    proc getFieldTypeIdFromStr {fieldTypeStr fieldTypeIdVar} {
      upvar $fieldTypeIdVar fieldTypeId
      variable fieldTypeNames2Ids

      if {![dict exists $fieldTypeNames2Ids $fieldTypeStr]} {
        checkErrOK $::DATA_VIEW_ERR_FIELD_TYPE_NOT_FOUND
      }
      set fieldTypeId [dict get $fieldTypeNames2Ids $fieldTypeStr]
      return $::DATA_VIEW_ERR_OK
    }

    # ================================= getFieldTypeStrFromId ====================================

    proc getFieldTypeStrFromId {fieldTypeId fieldTypeStrVar} {
      upvar $fieldTypeStrVar fieldTypeStr
      variable fieldTypeIds2Names
      variable fieldTypeInts2Ids

      if {[string is integer $fieldTypeId]} {
        if {![dict exists $fieldTypeInts2Ids $fieldTypeId]} {
          checkErrOK $::DATA_VIEW_ERR_FIELD_TYPE_NOT_FOUND
        }
        set fieldTypeId [dict get $fieldTypeInts2Ids $fieldTypeId]
      }
      if {![dict exists $fieldTypeIds2Names $fieldTypeId]} {
        checkErrOK $::DATA_VIEW_ERR_FIELD_TYPE_NOT_FOUND
      }
      set fieldTypeStr [dict get $fieldTypeIds2Names $fieldTypeId]
      return $::DATA_VIEW_ERR_OK
    }

    # ================================= getFieldTypeIntFromId ====================================

    proc getFieldTypeIntFromId {fieldTypeId fieldTypeIntVar} {
      upvar $fieldTypeIntVar fieldTypeInt
      variable fieldTypeIds2Ints

      if {![dict exists $fieldTypeIds2Ints $fieldTypeId]} {
        checkErrOK $::DATA_VIEW_ERR_FIELD_TYPE_NOT_FOUND
      }
      set fieldTypeInt [dict get $fieldTypeIds2Ints $fieldTypeId]
      return $::DATA_VIEW_ERR_OK
    }

    # ================================= getUint8 ====================================

    proc getUint8 {offset valueVar} {
      variable lgth
      variable data
      upvar $valueVar value

      if {$offset > $lgth} {
puts stderr "getUint8 OUT_OF_RANGE!$offset!$lgth!"
        checkErrOK $::DATA_VIEW_ERR_OUT_OF_RANGE
      }
      set ch [string range $data $offset $offset]
if {$ch eq ""} {
puts stderr "getUint8 ch is empty: offset: $offset!lgth: $lgth!ll data: [string length $data]!"
dumpBinary $data $lgth "getuint8"
}
      set pch $ch
      binary scan $ch c pch
      if {[string is integer $pch]} {
        set value [expr {$pch & 0xFF}]
      } else {
        set value $pch
      }
      return $::DATA_VIEW_ERR_OK
    }

    # ================================= getInt8 ====================================
    
    proc getInt8 {offset valueVar} {
      variable lgth
      variable data
      upvar $valueVar value

      if {$offset > $lgth} {
puts stderr "getInt8 OUT_OF_RANGE!$offset!$lgth!"
        checkErrOK $::DATA_VIEW_ERR_OUT_OF_RANGE
      }
      set ch [string range $data $offset $offset]
      binary scan $ch c pch
      set value $pch
      return $::DATA_VIEW_ERR_OK
    }
    
    # ================================= setUint8 ====================================
    
    proc setUint8 {offset value} {
      variable lgth
      variable data

      if {$offset > $lgth} {
        checkErrOK $::DATA_VIEW_ERR_OUT_OF_RANGE
      }
      set pch $value
      if {![string is integer $value]} {
        binary scan $value c pch
      }
      set ch [binary format c $pch]
      set data [string replace $data $offset $offset $ch]
      return $::DATA_VIEW_ERR_OK
    }
    
    # ================================= setInt8 ====================================
    
    proc setInt8 {offset value} {
      variable lgth
      variable data

      if {$offset > $lgth} {
        checkErrOK $::DATA_VIEW_ERR_OUT_OF_RANGE
      }
      set ch [binary format c $value]
      set data [string replace $data $offset $offset [expr {$value & 0xFF}]]
      return $::DATA_VIEW_ERR_OK
    }
    
    
    # ================================= getUint16 ====================================
    
    proc getUint16 {offset valueVar} {
      variable lgth
      variable data
      upvar $valueVar value

      if {[expr {$offset + 1}] > $lgth} {
puts stderr "getUint16 OUT_OF_RANGE!$offset!$lgth!"
        checkErrOK $::DATA_VIEW_ERR_OUT_OF_RANGE
      }
      set value 0
      set sh [string range $data $offset [expr {$offset + 1}]]
      binary scan $sh S value
      incr offset
      return $::DATA_VIEW_ERR_OK
    }
    
    # ================================= getInt16 ====================================
    
    proc getInt16 {offset valueVar} {
      variable lgth
      variable data
      upvar $valueVar value

      if {offset + 1 > self->lgth} {
puts stderr "getInt16 OUT_OF_RANGE!$offset!$lgth!"
        checkErrOK $::DATA_VIEW_ERR_OUT_OF_RANGE
      }
      set value 0
      set sh [string range $data $offset [expr {$offset + 1}]]
      binary scan $sh S value
      return $::DATA_VIEW_ERR_OK
    }
    
    # ================================= setUint16 ====================================
    
    proc setUint16 {offset value} {
      variable lgth
      variable data

      if {[expr {$offset + 1}] > $lgth} {
        checkErrOK $::DATA_VIEW_ERR_OUT_OF_RANGE
      }
      if {![string is integer $value]} {
        set val $value
        binary scan $val S value
      }
      set sh [binary format S $value]
      set data [string replace $data $offset [expr {$offset +1}] $sh]
      incr offset
      return $::DATA_VIEW_ERR_OK
    }
    
    # ================================= setInt16 ====================================
    
    proc setInt16 {offset value} {
      variable lgth
      variable data

      if {[expr {$offset + 1}] > $lgth} {
        checkErrOK $::DATA_VIEW_ERR_OUT_OF_RANGE
      }
      set ch [binary format c [expr {($value >> 8) & 0xFF}]]
      set data [string replace $data $offset $offset $ch]
      incr offset
      set ch [binary format c [expr {$value & 0xFF}]]
      set data [string replace $data $offset $offset $ch]
      return $::DATA_VIEW_ERR_OK
    }
    
    
    # ================================= getUint32 ====================================
    
    proc getUint32 {offset valueVar} {
      variable lgth
      variable data
      upvar $valueVar value

      if {[expr {$offset + 3}] > $lgth} {
puts stderr "getUint32 OUT_OF_RANGE!$offset!$lgth!"
        checkErrOK $::DATA_VIEW_ERR_OUT_OF_RANGE
      }
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
      return $::DATA_VIEW_ERR_OK
    }
    
    # ================================= getInt32 ====================================
    
    proc getInt32 {offset valueVar} {
      variable lgth
      variable data
      upvar $valueVar value

      if {[expr {$offset + 3}] > $lgth} {
puts stderr "getInt32 OUT_OF_RANGE!$offset!$lgth!"
        checkErrOK $::DATA_VIEW_ERR_OUT_OF_RANGE
      }
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
      return $::DATA_VIEW_ERR_OK
    }
    
    # ================================= setUint32 ====================================
    
    proc setUint32 {offset value} {
      variable lgth
      variable data

      if {[expr {$offset + 3}] > $lgth} {
        checkErrOK $::DATA_VIEW_ERR_OUT_OF_RANGE
      }
      set val $value
      binary scan $val I value
      set data [string replace $data $offset $offset [binary format c [expr {($value >> 24) & 0xFF}]]]
      incr offset
      set data [string replace $data $offset $offset [binary format c [expr {($value >> 16) & 0xFF}]]]
      incr offset
      set data [string replace $data $offset $offset [binary format c [expr {($value >> 8) & 0xFF}]]]
      incr offset
      set data [string replace $data $offset $offset [binary format c [expr {$value & 0xFF}]]]
      return $::DATA_VIEW_ERR_OK
    }
    
    # ================================= setInt32 ====================================
    
    proc setInt32 {offset value} {
      variable lgth
      variable data

      if {[expr {$offset + 3}] > $lgth} {
        checkErrOK $::DATA_VIEW_ERR_OUT_OF_RANGE
      }
      set ch [binary format c [expr {($value >> 24) & 0xFF}]]
      set data [string replace $data $offset $offset $ch]
      incr offset
      set ch [binary format c [expr {($value >> 16) & 0xFF}]]
      set data [string replace $data $offset $offset $ch]
      incr offset
      set ch [binary format c [expr {($value >> 8) & 0xFF}]]
      set data [string replace $data $offset $offset $ch]
      incr offset
      set ch [binary format c [expr {$value & 0xFF}]]
      set data [string replace $data $offset $offset $ch]
      return $::DATA_VIEW_ERR_OK
    }
    
    
    # ================================= getUint8Vector ====================================
    
    proc getUint8Vector {offset valueVar size} {
      variable lgth
      variable data
      upvar $valueVar value

      if {[expr {$offset + $size}] > $lgth} {
puts stderr "getUint8Vector OUT_OF_RANGE!$offset!$lgth!"
        checkErrOK $::DATA_VIEW_ERR_OUT_OF_RANGE
      }
      set value [string range $data $offset [expr {$offset + $size - 1}]]
      return $::DATA_VIEW_ERR_OK
    }
    
    # ================================= getInt8Vector ====================================
    
    proc getInt8Vector {offset valueVar size} {
      variable lgth
      variable data
      upvar $valueVar value

      if {[expr {$offset + $size}] > $lgth} {
        checkErrOK $::DATA_VIEW_ERR_OUT_OF_RANGE
      }
      set value [string range $data $offset [expr {$offset + $size - 1}]]
      return $::DATA_VIEW_ERR_OK
    }
    
    # ================================= setUint8Vector ====================================
    
    proc setUint8Vector {offset value size} {
      variable lgth
      variable data

      if {[expr {$offset + $size}] > $lgth} {
        checkErrOK $::DATA_VIEW_ERR_OUT_OF_RANGE
      }
      # expand to field length otherwise problems with "string replace" !!!
      while {[string length $value] < $size} {
        append value " "
      }
      set data [string replace $data $offset [expr {$offset + $size - 1}] [string range $value 0 [expr {$size - 1}]]]
      return $::DATA_VIEW_ERR_OK
    }
    
    # ================================= setInt8Vector ====================================
    
    proc setInt8Vector {offset value size} {
      variable lgth
      variable data

      if {[exprr {$offset + size}] > $lgth} {
        checkErrOK $::DATA_VIEW_ERR_OUT_OF_RANGE
      }
      set data [string replace $data $offset [expr {$offset + $size - 1}] [string range $value 0 [expr {$size - 1}]]]
      return $::DATA_VIEW_ERR_OK
    }
    
    
    # ================================= getUint16Vector ====================================
    
    proc getUint16Vector {offset valueVar size} {
      variable lgth
      variable data
      upvar $valueVar value

      if {[expr {$offset + $size * 2} > $lgth} {
        checkErrOK $::DATA_VIEW_ERR_OUT_OF_RANGE
      }
      set idx 0
      while {$idx < $size} {
        set result [getUint16 $offset val]
        append value $val
        checkErrOK $result
        incr idx
      }
      return $::DATA_VIEW_ERR_OK
    }
    
    # ================================= getInt16Vector ====================================
    
    proc getInt16Vector {offset valueVar size} {
      variable lgth
      variable data
      upvar $valueVar value

      if {[expr {$offset + $size}] * 2 > $lgth} {
        checkErrOK $::DATA_VIEW_ERR_OUT_OF_RANGE
      }
      set idx 0
      while {$idx < $size} {
        set result [getInt16 $offset val]
        checkErrOK $result
        append value $val
        incr idx
      }
      return $::DATA_VIEW_ERR_OK
    }
    
    # ================================= setUint16Vector ====================================
    
    proc setUint16Vector {offset value size} {
      variable lgth
      variable data

      if {[expr {$offset + $size * 2}] > $lgth} {
        checkErrOK $::DATA_VIEW_ERR_OUT_OF_RANGE
      }
      set idx 0
      while {$idx < $size} {
        set result [setUint16 $offset [lindex $value $idx]]
        checkErrOK $result
        incr idx
      }
      return $::DATA_VIEW_ERR_OK
    }
    
    # ================================= setInt16Vector ====================================
    
    proc setInt16Vector {offset value size} {
      variable lgth
      variable data

      if {[expr {$offset + $size * 2}] > $lgth} {
        checkErrOK $::DATA_VIEW_ERR_OUT_OF_RANGE;
      }
      set idx 0
      while {$idx < $size} {
        set result [setInt16 $offset [lindex $value $idx]]
        checkErrOK $result
        incr idx
      }
      return $::DATA_VIEW_ERR_OK;
    }
    
    
    # ================================= getUint32Vector ====================================
    
    proc getUint32Vector {offset valueVar size} {
      variable lgth
      variable data
      upvar $valueVar value

      if {[expr {$offset + $size * 4}] > $lgth} {
        checkErrOK $::DATA_VIEW_ERR_OUT_OF_RANGE
      }
      set idx 0
      while {$idx < $size} {
        set result [getUint32 $offset val]
        append value $val
        checkErrOK $result
        incr idx
      }
      return $::DATA_VIEW_ERR_OK;
    }
    
    # ================================= getInt32Vector ====================================
    
    proc getInt32Vector {offset valueVar size} {
      variable lgth
      variable data
      upvar $valueVar value

      if {[expr {$offset + $size * 4}] > $lgth} {
        checkErrOK $::DATA_VIEW_ERR_OUT_OF_RANGE
      }
      set idx 0
      while {$idx < $size} {
        set result [getInt32 $offset val]
        append value $val
        checkErrOK $result
        icnr idx
      }
      return $::DATA_VIEW_ERR_OK
    }
    
    # ================================= setUint32Vector ====================================
    
    proc setUint32Vector {offset value size} {
      variable lgth
      variable data

      if {[expr {$offset + $size * 4}] > $lgth} {
        checkErrOK $::DATA_VIEW_ERR_OUT_OF_RANGE
      }
      set idx 0
      while {$idx < $size} {
        set result [setUint32 $offset [lindex $value $idx]]
        checkErrOK $result
        incr idx
      }
      return $::DATA_VIEW_ERR_OK
    }
    
    # ================================= setInt32Vector ====================================
    
    proc setInt32Vector {offset value size} {
      variable lgth
      variable data

      if {[expr {$offset + $size * 4}] > $lgth} {
        checkErrOK $::DATA_VIEW_ERR_OUT_OF_RANGE
      }
      set idx 0
      while {$idx < $size} {
        set result [setInt32 $offset [lindex $value $idx]]
        checkErrOK $result
        incr idx
      }
      return $::DATA_VIEW_ERR_OK
    }
    
    
    # ================================= setData ====================================
    
    proc setData {buffer size} {
      variable data
      variable lgth

#puts stderr "setData: size: $size!ll buffer: [string length $buffer]!"
      if {[string length $data] != 0} {
        set data ""
      }
      set data $buffer
      set lgth $size
      return $::DATA_VIEW_ERR_OK
    }
    
    # ================================= getData ====================================
    
    proc getData {bufferVar sizeVar} {
      upvar $bufferVar buffer
      upvar $sizeVar size
      variable data
      variable lgth

      set buffer $data
      set size $lgth
      return $::DATA_VIEW_ERR_OK
    }
    
    # ================================= appendData ====================================
    
    proc appendData {buffer size} {
      variable data
      variable lgth

      append data $buffer
      incr lgth $size
      return $::DATA_VIEW_ERR_OK
    }
    
    # ================================= dataView ====================================
    
    proc dataView {command args} {
puts stderr "dataView: $command $args!"
      switch $command {
       getFieldTypeStrFromId -
       getFieldTypeIdFromStr {
         return [uplevel 0 $command $args]
       }
      }
error "dataView!no such command!$command!"
      return $::DATA_VIEW_ERR_NO_SUCH_COMMAND
if {0} {
      dataView->data = NULL;
      dataView->lgth = 0;
      dataViewId++;
      dataView->id = dataViewId;
    
      dataView->getFieldTypeIdFromStr = &getFieldTypeIdFromStr;
      dataView->getFieldTypeStrFromId = &getFieldTypeStrFromId;
    
      dataView->getUint8 = &getUint8;
      dataView->getInt8 = &getInt8;
      dataView->setUint8 = &setUint8;
      dataView->setInt8 = &setInt8;
    
      dataView->getUint16 = &getUint16;
      dataView->getInt16 = &getInt16;
      dataView->setUint16 = &setUint16;
      dataView->setInt16 = &setInt16;
    
      dataView->getUint32 = &getUint32;
      dataView->getInt32 = &getInt32;
      dataView->setUint32 = &setUint32;
      dataView->setInt32 = &setInt32;
    
      dataView->getUint8Vector = &getUint8Vector;
      dataView->getInt8Vector = &getInt8Vector;
      dataView->setUint8Vector = &setUint8Vector;
      dataView->setInt8Vector = &setInt8Vector;
    
      dataView->getUint16Vector = &getUint16Vector;
      dataView->getInt16Vector = &getInt16Vector;
      dataView->setUint16Vector = &setUint16Vector;
      dataView->setInt16Vector = &setInt16Vector;
    
      dataView->getUint32Vector = &getUint32Vector;
      dataView->getInt32Vector = &getInt32Vector;
      dataView->setUint32Vector = &setUint32Vector;
      dataView->setInt32Vector = &setInt32Vector;
    
      dataView->setData = &setData;
      return dataView
}
    }
    
    # ================================= freeDataView ====================================
    
    proc freeDataView {} {
    }

  } ; # namespace dataView
} ; # namespace compMsg

