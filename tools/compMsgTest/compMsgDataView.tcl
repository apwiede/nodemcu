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

set ::crcDebug false

set ::COMP_MSG_NO_INCR 0
set ::COMP_MSG_INCR    1
set ::COMP_MSG_DECR    -1

set ::COMP_MSG_FREE_FIELD_ID 0xFF
set RAND_MAX 0x7FFFFFFF

namespace eval ::compMsg {
  namespace ensemble create

  namespace export compMsgDataView

  namespace eval compMsgDataView {
    namespace ensemble create
      
    namespace export compMsgDataView freeCompMsgDataView
    namespace export setFieldValue getFieldValue
    namespace export setRandomNum getRandomNum setSequenceNum getSequenceNum
    namespace export setFiller getFiller setCrc getCrc setTotalCrc getTotalCrc
    namespace export getIdFieldValue setIdFieldValue

    dict set ::compMsg(fieldNameDefinitions) numDefinitions 0

    # ============================= getRandom ========================
    
    proc getRandom {} {
      set val [string trimleft [lindex [split [expr {rand()}] {.}] 1] 0]
      set myVal [expr {$val & $::RAND_MAX}]
      return $myVal
    }
    
    # ================================= getRandomNum ====================================
    
    proc getRandomNum {fieldInfo value} {
      return [::compMsg dataView getUint32 [dict get $fieldInfo fieldOffset] value]
    }
    
    # ================================= setRandomNum ====================================
    
    proc setRandomNum {fieldInfo} {
      set val [getRandom]
      return [::compMsg dataView setUint32 [dict get $fieldInfo fieldOffset] $val]
    }
    
    
    # ================================= getSequenceNum ====================================
    
    proc getSequenceNum {fieldInfo valueVar} {
      upvar $valueVar value

      return [::compMsg dataView getUint32 [dict get $fieldInfo fieldOffset] value]
    }
    
    # ================================= setSequenceNum ====================================
    
    proc setSequenceNum {fieldInfo} {
      incr ::sequenceNum
      return [::compMsg dataView setUint32 [dict get $fieldInfo fieldOffset] $::sequenceNum]
    }
    
    # ================================= getFiller ====================================
    
    proc getFiller {fieldInfo valueVar} {
      upvar $valueVar value

      return [::compMsg dataView getuint8Vector [dict get $fieldInfo fieldOffset] value [dict get $fieldInfo fieldLgth]]
    }
    
    # ================================= setFiller ====================================
    
    proc setFiller {fieldInfo} {
      set lgth [dict get $fieldInfo fieldLgth]
      set offset [dict get $fieldInfo fieldOffset]
      set idx 0
      while {$lgth >= 4} {
        set value ""
        set myVal [expr {[getRandom] &0xFFFFFFFF}]
        append value [binary format c [expr {($myVal >> 24) & 0xFF}]]
        append value [binary format c [expr {($myVal >> 16) & 0xFF}]]
        append value [binary format c [expr {($myVal >> 8) & 0xFF}]]
        append value [binary format c [expr {($myVal >> 0) & 0xFF}]]
        set result [::compMsg dataView setUint32 $offset $value]
        checkErrOK $result
        incr offset 4
        incr lgth -4
      }
      while {$lgth >= 2} {
        set value ""
        set myVal [expr {[getRandom] & 0xFFFF}]
        append value [binary format c [expr {($myVal >> 8) & 0xFF}]]
        append value [binary format c [expr {($myVal >> 0) & 0xFF}]]
        set result [::compMsg dataView setUint16 $offset $value]
        checkErrOK $result
        incr offset 2
        incr lgth -2
      }
      while {$lgth >= 1} {
        set value ""
        set myVal [expr {[getRandom] & 0xFF}]
        append value [binary format c [expr {($myVal >> 0) & 0xFF}]]
        set result [::compMsg dataView setUint8 $offset $value]
        checkErrOK $result
        incr offset 1
        incr lgth -1
      }
      return [checkErrOK OK]
    }
    
    
    # ================================= getCrc ====================================
    
    proc getCrc {fieldInfo valueVar startOffset size} {
      upvar $valueVar value

#set ::crcDebug true
      set crcLgth [dict get $fieldInfo fieldLgth]
      set value ""
      set lgth [expr {$size - $crcLgth}]
      set crcVal 0
      set offset $startOffset
set cnt 0
puts stderr "getCrc: offset: $offset size: $size ::crcDebug: $::crcDebug!"
      while {$offset < $size} {
        set result [::compMsg dataView getUint8 $offset ch]
        set pch $ch
        if {![string is integer $ch]} {
          binary scan $ch c pch
        }
        set pch [expr {$pch & 0xFF}]
if {$::crcDebug} {
puts stderr "getCrc: $cnt ch: [format 0x%02x $pch]![format 0x%04x $crcVal]!"
}
        set crcVal [expr {$crcVal + [format "%d" $pch]}]
incr cnt
        incr offset
      }
if {$::crcDebug} {
puts stderr "crcVal end: $crcVal!"
}
      set offset [dict get $fieldInfo fieldOffset]
      if {$crcLgth == 2} {
        set crcVal [expr {~$crcVal & 0xFFFF}]
        set result [::compMsg dataView getuint16 $offset crc]
        checkErrOK $result
if {$::crcDebug} {
puts stderr "crcVal: [format 0x%04x $crcVal]!offset: $offset!crc: [format 0x%04x $crc]!"
}
        if {$crcVal != $crc} {
          checkErrOK BAD_CRC_VALUE
        }
        set value $crc
      } else  {
        set crcVal [expr {~$crcVal}]
        set crcVal [expr {$crcVal & 0xFF}]
if {$::crcDebug} {
puts stderr "crcVal2 end: $crcVal!"
}
        set result [::compMsg dataView getUint8 $offset crc]
        checkErrOK $result
if {$::crcDebug} {
puts stderr "crcVal: [format 0x%02x [expr {$crcVal & 0xFF}]]!offset: $offset!crc: [format 0x%02x $crc]!"
}
        if {[expr {$crcVal & 0xFF}] != $crc} {
          checkErrOK BAD_CRC_VALUE
        }
        set value $crc
      }
      return [checkErrOK OK]
    }
    
    # ================================= setCrc ====================================
    
    proc setCrc {fieldInfo startOffset size} {
      set crcLgth [dict get $fieldInfo fieldLgth]
#puts stderr "setCrc: startOffset: $startOffset size: $size!"
      set size [expr {$size - $crcLgth}]
#set ::crcDebug true
set cnt 0
      set crc  0
      set offset $startOffset
      while {$offset < $size} {
        set result [::compMsg dataView getUint8 $offset ch]
        set pch $ch
        if {![string is integer $ch]} {
          binary scan $ch c pch
        }
        set pch [expr {$pch & 0xFF}]
if {$::crcDebug} {
puts stderr "setCrc: $cnt $ch![format 0x%02x $pch]![format 0x%04x $crc]!"
}
        set crc [expr {$crc + [format "%d" $pch]}]
incr cnt
        incr offset
      }
if {$::crcDebug} {
puts stderr "setCrc end: $cnt $ch![format 0x%02x $pch]![format 0x%04x $crc]!"
}
if {$::crcDebug} {
puts stderr "crc1: $crc![format 0x%04x $crc]!"
}
      set crc [expr {~$crc & 0xFFFF}]
if {$::crcDebug} {
puts stderr "crc11: $crc![format 0x%04x $crc]![format 0x%02x [expr {$crc & 0xFF}]]"
}
      if {$crcLgth == 1} {
#puts stderr "offset: [dict get $fieldInfo fieldOffset] val: [expr {$crc & 0xFF}]!"
        set result [::compMsg dataView setUint8 [dict get $fieldInfo fieldOffset] [expr {$crc & 0xFF}]]
      } else {
        set result [::compMsg dataView setUint16 [dict get $fieldInfo fieldOffset] $crc]
      }
      checkErrOK $result
      return [checkErrOK OK]
    }
    
    # ================================= getTotalCrc ====================================
    
    proc getTotalCrc {fieldInfo valueVar} {
      upvar $valueVar value

      set crcLgth [dict get $fieldInfo fieldLgth]
      set value ""
      set size [dict get $fieldInfo fieldOffset]
      set crcVal 0
      set offset 0
#set ::crcDebug true
set cnt 0
      while {$offset < $size} {
        set result [::compMsg dataView getUint8 $offset ch]
        set pch $ch
        if {![string is integer $ch]} {
          binary scan $ch c pch
        }
        set pch [expr {$pch & 0xFF}]
if {$::crcDebug} {
puts stderr "getTotalCrc: $cnt ch: [format 0x%02x $pch]![format 0x%04x $crcVal]!"
}
        set crcVal [expr {$crcVal + [format "%d" $pch]}]
incr cnt
        incr offset
      }
if {$::crcDebug} {
puts stderr "crcVal end: $crcVal!"
}
      set offset [dict get $fieldInfo fieldOffset]
      if {$crcLgth == 2} {
        set crcVal [expr {~$crcVal & 0xFFFF}]
        set result [::compMsg dataView getuint16 $offset crc]
        checkErrOK $result
if {$::crcDebug} {
puts stderr "crcVal: [format 0x%04x $crcVal]!offset: $offset!crc: [format 0x%04x $crc]!"
}
        if {$crcVal != $crc} {
          checkErrOK BAD_CRC_VALUE
        }
        set value $crc
      } else  {
        set crcVal [expr {~$crcVal}]
        set crcVal [expr {$crcVal & 0xFF}]
if {$::crcDebug} {
puts stderr "crcVal2 end: $crcVal!"
}
        set result [::compMsg dataView getUint8 $offset crc]
        checkErrOK $result
if {$::crcDebug} {
puts stderr "crcVal: [format 0x%02x [expr {$crcVal & 0xFF}]]!offset: $offset!crc: [format 0x%02x $crc]!"
}
        if {[expr {$crcVal & 0xFF}] != $crc} {
          checkErrOK BAD_CRC_VALUE
        }
        set value $crc
      }
      return [checkErrOK OK]
    }
    
    # ================================= setTotalCrc ====================================
    
    proc setTotalCrc {fieldInfo} {
      set crcLgth [dict get $fieldInfo fieldLgth]
      set size [dict get $fieldInfo fieldOffset]
#set ::crcDebug true
set cnt 0
#puts stderr "setTotalCrc: $::compMsg::dataView::lgth!$::compMsg::dataView::data!"
      set crc  0
      set offset 0
      while {$offset < $size} {
        set result [::compMsg dataView getUint8 $offset ch]
        set pch $ch
        if {![string is integer $ch]} {
          binary scan $ch c pch
        }
        set pch [expr {$pch & 0xFF}]
if {$::crcDebug} {
puts stderr "setTotalCrc: $cnt $ch![format 0x%02x $pch]![format 0x%04x $crc]!"
}
        set crc [expr {$crc + [format "%d" $pch]}]
incr cnt
        incr offset
      }
if {$::crcDebug} {
puts stderr "crc1: $crc![format 0x%04x $crc]!"
}
      set crc [expr {~$crc & 0xFFFF}]
if {$::crcDebug} {
puts stderr "crc11: $crc![format 0x%04x $crc]!"
}
      if {$crcLgth == 1} {
#puts stderr "offset: [dict get $fieldInfo fieldOffset] val: [format 0x%02x [expr {$crc & 0xFF}]]!"
        set result [::compMsg dataView setUint8 [dict get $fieldInfo fieldOffset] [expr {$crc & 0xFF}]]
      } else {
        set result [::compMsg dataView setUint16 [dict get $fieldInfo fieldOffset] $crc]
      }
      checkErrOK $result
      return [checkErrOK OK]
    }
    
    
    # ================================= getFieldValue ====================================
    
    proc getFieldValue {fieldInfo valueVar fieldIdx} {
      upvar $valueVar value

if {[catch {
      set value ""
      set offset [dict get $fieldInfo fieldOffset]
      set shortOffset [expr {$offset + $fieldIdx}]
      switch [dict get $fieldInfo fieldTypeId] {
        DATA_VIEW_FIELD_INT8_T {
          set result [::compMsg dataView getInt8 $shortOffset value]
        }
        DATA_VIEW_FIELD_UINT8_T {
          set result [::compMsg dataView getUint8 $shortOffset value]
        }
        DATA_VIEW_FIELD_INT16_T {
          set result [::compMsg dataView getInt16 $shortOffset value]
        }
        DATA_VIEW_FIELD_UINT16_T {
          set result [::compMsg dataView getUint16 $shortOffset value]
        }
        DATA_VIEW_FIELD_INT32_T {
          set result [::compMsg dataView getInt32 $shortOffset value]
        }
        DATA_VIEW_FIELD_UINT32_T {
          set result [::compMsg dataView getUint32 $shortOffset value]
        }
        DATA_VIEW_FIELD_INT8_VECTOR {
          set result [::compMsg dataView getInt8Vector [expr {$offset + $fieldIdx}] value [dict get $fieldInfo fieldLgth]]
        }
        DATA_VIEW_FIELD_UINT8_VECTOR {
          set result [::compMsg dataView getUint8Vector [expr {$offset + $fieldIdx}] value [dict get $fieldInfo fieldLgth]]
        }
        DATA_VIEW_FIELD_INT16_VECTOR {
          set result [::compMsg dataView getInt16 [expr {$offset +fieldIdx*2}] value]
        }
        DATA_VIEW_FIELD_UINT16_VECTOR {
          set result [::compMsg dataView getUint16 [expr {$offset + $fieldIdx*2}] value]
        }
        DATA_VIEW_FIELD_INT32_VECTOR {
          set result [::compMsg dataView getInt32 [expr {$offset + $fieldIdx*4}] value]
        }
        DATA_VIEW_FIELD_UINT32_VECTOR {
          set result [::compMsg dataView getUint32 [expr {$offset + $fieldIdx*4}] value]
        }
        default {
          checkErrOK BAD_FIELD_TYPE
        }
      }
} msg]} {
puts stderr "getFieldValue: $msg!$fieldInfo!"
}
      checkErrOK $result
      return [checkErrOK OK]
    }
    
    # ================================= setFieldValue ====================================
    
    proc setFieldValue {fieldInfo value fieldIdx} {
      switch [dict get $fieldInfo fieldTypeId] {
        DATA_VIEW_FIELD_INT8_T {
          if {($value > -128) && ($value < 128)} {
            set result [::compMsg dataView setInt8 [dict get $fieldInfo fieldOffset] $value]
          } else {
puts stderr "compMsgDataView setFieldValue int8 value too big"
            checkErrOK VALUE_TOO_BIG
          }
        }
        DATA_VIEW_FIELD_UINT8_T {
          binary scan $value c pch
          set pch [expr {$pch & 0xFF}]
          set value $pch
          if {($value >= 0) && ($value <= 256)} {
            set result [::compMsg dataView setUint8 [dict get $fieldInfo fieldOffset] $value]
          } else {
puts stderr "compMsgDataView setFieldValue uint8 value too big"
            checkErrOK BAD_VALUE
          }
        }
        DATA_VIEW_FIELD_INT16_T {
          if {($value > -32767) && ($value < 32767)} {
            set result [::compMsg dataView setInt16 [dict get $fieldInfo fieldOffset] $value]
          } else {
puts stderr "compMsgDataView setFieldValue int16 value too big"
            checkErrOK VALUE_TOO_BIG
          }
        }
        DATA_VIEW_FIELD_UINT16_T {
          if {![string is integer $value]} {
            binary scan $value S val
            set value $val
          }
          if {($value >= 0) && ($value <= 65535)} {
            set result [::compMsg dataView setUint16 [dict get $fieldInfo fieldOffset] $value]
          } else {
puts stderr "compMsgDataView setFieldValue uint16 value too big"
            checkErrOK VALUE_TOO_BIG
          }
        }
        DATA_VIEW_FIELD_INT32_T {
          if {($value > -0x7FFFFFFF) && ($value <= 0x7FFFFFFF)} {
            set result [::compMsg dataView setInt32 [dict get $fieldInfo fieldOffset] $value]
          } else {
puts stderr "compMsgDataView setFieldValue int32 value too big"
            checkErrOK VALUE_TOO_BIG
          }
        }
        DATA_VIEW_FIELD_UINT32_T {
          # we have to do the signed check as numericValue is a signed integer!!
          if {($value > -0x7FFFFFFF) && ($value <= 0x7FFFFFFF)} {
            set result [::compMsg dataView setUint32 [dict get $fieldInfo fieldOffset] $value]
          } else {
puts stderr "compMsgDataView setFieldValue uint32 value too big !$value!"
            checkErrOK VALUE_TOO_BIG
          }
        }
        DATA_VIEW_FIELD_INT8_VECTOR {
          set result [::compMsg dataView setInt8Vector [dict get $fieldInfo fieldOffset] $value]
        }
        DATA_VIEW_FIELD_UINT8_VECTOR {
#puts stderr "setUint8Vector: lgth: [dict get $fieldInfo fieldLgth]!offset: [dict get $fieldInfo fieldOffset]!dv lgth: $::compMsg::dataView::lgth!"
          set result [::compMsg dataView setUint8Vector [dict get $fieldInfo fieldOffset] $value [dict get $fieldInfo fieldLgth]]
        }
        DATA_VIEW_FIELD_INT16_VECTOR {
          set result [::compMsg dataView setInt16Vector [expr {[dict get $fieldInfo fieldOffset] + $fieldIdx*2}] $value]
        }
        DATA_VIEW_FIELD_UINT16_VECTOR {
          set result [::compMsg dataView setUint16Vector [expr {[dict get $fieldInfo fieldOffset] + $fieldIdx*2}] $value]
        }
        DATA_VIEW_FIELD_INT32_VECTOR {
          set result [::compMsg dataView setInt32Vector [expr {[dict get $fieldInfo fieldOffset] + $fieldIdx*2}] $value]
        }
        DATA_VIEW_FIELD_UINT32_VECTOR {
          set result [::compMsg dataView setUint32Vector [expr {[dict get $fieldInfo fieldOffset] + $fieldIdx*2}] $value]
        }
        default {
puts stderr "bad type in setFieldValue: [dict get $fieldInfo fieldTypeId]"
          checkErrOK BAD_FIELD_TYPE
        }
      }
      checkErrOK $result
      return [checkErrOK OK]
    }

    # ================================= getIdFieldValue ====================================
    
    proc getIdFieldValue {compMsgDispatcherVar fieldId valueVar fieldIdx} {
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $valueVar value

      set compMsgTypesAndNames [dict get $compMsgDispatcher compMsgTypesAndNames]
      set msgFieldInfos [dict get $compMsgTypesAndNames msgFieldInfos]
      set fieldInfos [dict get $msgFieldInfos fieldInfos]
      set fieldInfo [lindex $fieldInfos $fieldId]
      set value ""
      set offset [dict get $fieldInfo fieldOffset]
      set shortOffset [expr {$offset + $fieldIdx}]
puts stderr "getIdFieldValue: $fieldInfo!"
      switch [dict get $fieldInfo fieldTypeId] {
        DATA_VIEW_FIELD_INT8_T {
          set result [::compMsg dataView getInt8 $shortOffset value]
        }
        DATA_VIEW_FIELD_UINT8_T {
          set result [::compMsg dataView getUint8 $shortOffset value]
        }
        DATA_VIEW_FIELD_INT16_T {
          set result [::compMsg dataView getInt16 $shortOffset value]
        }
        DATA_VIEW_FIELD_UINT16_T {
          set result [::compMsg dataView getUint16 $shortOffset value]
        }
        DATA_VIEW_FIELD_INT32_T {
          set result [::compMsg dataView getInt32 $shortOffset value]
        }
        DATA_VIEW_FIELD_UINT32_T {
          set result [::compMsg dataView getUint32 $shortOffset value]
        }
        DATA_VIEW_FIELD_INT8_VECTOR {
          set result [::compMsg dataView getInt8Vector [expr {$offset + $fieldIdx}] value [dict get $fieldInfo fieldLgth]]
        }
        DATA_VIEW_FIELD_UINT8_VECTOR {
          set result [::compMsg dataView getUint8Vector [expr {$offset + $fieldIdx}] value [dict get $fieldInfo fieldLgth]]
        }
        DATA_VIEW_FIELD_INT16_VECTOR {
          set result [::compMsg dataView getInt16 [expr {$offset +fieldIdx*2}] value]
        }
        DATA_VIEW_FIELD_UINT16_VECTOR {
          set result [::compMsg dataView getUint16 [expr {$offset + $fieldIdx*2}] value]
        }
        DATA_VIEW_FIELD_INT32_VECTOR {
          set result [::compMsg dataView getInt32 [expr {$offset + $fieldIdx*4}] value]
        }
        DATA_VIEW_FIELD_UINT32_VECTOR {
          set result [::compMsg dataView getUint32 [expr {$offset + $fieldIdx*4}] value]
        }
        default {
          checkErrOK BAD_FIELD_TYPE
        }
      }
      checkErrOK $result
      return [checkErrOK OK]
    }
    
    # ================================= setIdFieldValue ====================================
    
    proc setIdFieldValue {compMsgDispatcherVar fieldId value fieldIdx} {
      upvar $compMsgDispatcherVar compMsgDispatcher

      set compMsgTypesAndNames [dict get $compMsgDispatcher compMsgTypesAndNames]
      set msgFieldInfos [dict get $compMsgTypesAndNames msgFieldInfos]
      set fieldInfos [dict get $msgFieldInfos fieldInfos]
      set fieldInfo [lindex $fieldInfos $fieldId]
      switch [dict get $fieldInfo fieldTypeId] {
        DATA_VIEW_FIELD_INT8_T {
          if {($value > -128) && ($value < 128)} {
            set result [::compMsg dataView setInt8 [dict get $fieldInfo fieldOffset] $value]
          } else {
puts stderr "compMsgDataView setFieldValue int8 value too big"
            checkErrOK VALUE_TOO_BIG
          }
        }
        DATA_VIEW_FIELD_UINT8_T {
          if {($value >= 0) && ($value <= 256)} {
            set result [::compMsg dataView setUint8 [dict get $fieldInfo fieldOffset] $value]
          } else {
puts stderr "compMsgDataView setFieldValue uint8 value too big"
            checkErrOK BAD_VALUE
          }
        }
        DATA_VIEW_FIELD_INT16_T {
          if {($value > -32767) && ($value < 32767)} {
            set result [::compMsg dataView setInt16 [dict get $fieldInfo fieldOffset] $value]
          } else {
puts stderr "compMsgDataView setFieldValue int16 value too big"
            checkErrOK VALUE_TOO_BIG
          }
        }
        DATA_VIEW_FIELD_UINT16_T {
          if {![string is integer $value]} {
            binary scan $value S val
            set value $val
          }
          if {($value >= 0) && ($value <= 65535)} {
            set result [::compMsg dataView setUint16 [dict get $fieldInfo fieldOffset] $value]
          } else {
puts stderr "compMsgDataView setFieldValue uint16 value too big"
            checkErrOK VALUE_TOO_BIG
          }
        }
        DATA_VIEW_FIELD_INT32_T {
          if {($value > -0x7FFFFFFF) && ($value <= 0x7FFFFFFF)} {
            set result [::compMsg dataView setInt32 [dict get $fieldInfo fieldOffset] $value]
          } else {
puts stderr "compMsgDataView setFieldValue int32 value too big"
            checkErrOK VALUE_TOO_BIG
          }
        }
        DATA_VIEW_FIELD_UINT32_T {
          # we have to do the signed check as numericValue is a signed integer!!
          if {($value > -0x7FFFFFFF) && ($value <= 0x7FFFFFFF)} {
            set result [::compMsg dataView setUint32 [dict get $fieldInfo fieldOffset] $value]
          } else {
puts stderr "compMsgDataView setFieldValue uint32 value too big !$value!"
            checkErrOK VALUE_TOO_BIG
          }
        }
        DATA_VIEW_FIELD_INT8_VECTOR {
          set result [::compMsg dataView setInt8Vector [dict get $fieldInfo fieldOffset] $value]
        }
        DATA_VIEW_FIELD_UINT8_VECTOR {
#puts stderr "setUint8Vector: lgth: [dict get $fieldInfo fieldLgth]!offset: [dict get $fieldInfo fieldOffset]!dv lgth: $::compMsg::dataView::lgth!"
          set result [::compMsg dataView setUint8Vector [dict get $fieldInfo fieldOffset] $value [dict get $fieldInfo fieldLgth]]
        }
        DATA_VIEW_FIELD_INT16_VECTOR {
          set result [::compMsg dataView setInt16Vector [expr {[dict get $fieldInfo fieldOffset] + $fieldIdx*2}] $value]
        }
        DATA_VIEW_FIELD_UINT16_VECTOR {
          set result [::compMsg dataView setUint16Vector [expr {[dict get $fieldInfo fieldOffset] + $fieldIdx*2}] $value]
        }
        DATA_VIEW_FIELD_INT32_VECTOR {
          set result [::compMsg dataView setInt32Vector [expr {[dict get $fieldInfo fieldOffset] + $fieldIdx*2}] $value]
        }
        DATA_VIEW_FIELD_UINT32_VECTOR {
          set result [::compMsg dataView setUint32Vector [expr {[dict get $fieldInfo fieldOffset] + $fieldIdx*2}] $value]
        }
        default {
puts stderr "bad type in setFieldValue: [dict get $fieldInfo fieldTypeId]"
          checkErrOK BAD_FIELD_TYPE
        }
      }
      set fieldInfos [lreplace $fieldInfos $fieldId $fieldId $fieldInfo]
      dict set msgFieldInfos fieldInfos $fieldInfos
      dict set compMsgTypesAndNames msgFieldInfos $msgFieldInfos
      dict set compMsgDispatcher compMsgTypesAndNames $compMsgTypesAndNames
      checkErrOK $result
      return [checkErrOK OK]
    }

  } ; # namespace compMsgDataView
} ; # namespace compMsg
