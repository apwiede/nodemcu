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

if {0} {
typedef struct structmsgField {
  uint8_t fieldNameId;
  uint8_t fieldTypeId;
  uint8_t fieldLgth;
  uint8_t fieldFlags;
  size_t fieldOffset;
} structmsgField_t;

typedef struct fieldName2id {
  uint8_t *fieldName;
  uint8_t fieldNameId;
  uint8_t refCnt;
} fieldName2id_t;

typedef struct fieldNames
{
  size_t numNames;
  size_t maxNames;
  fieldName2id_t *names;
} fieldNames_t;

}

set ::crcDebug false

set ::STRUCT_MSG_ERR_OK                    0
set ::STRUCT_MSG_ERR_VALUE_NOT_SET         255
set ::STRUCT_MSG_ERR_VALUE_OUT_OF_RANGE    254
set ::STRUCT_MSG_ERR_BAD_VALUE             253
set ::STRUCT_MSG_ERR_BAD_FIELD_TYPE        252
set ::STRUCT_MSG_ERR_FIELD_TYPE_NOT_FOUND  251
set ::STRUCT_MSG_ERR_VALUE_TOO_BIG         250
set ::STRUCT_MSG_ERR_OUT_OF_MEMORY         249
set ::STRUCT_MSG_ERR_OUT_OF_RANGE          248
  # be carefull the values up to here
  # must correspond to the values in dataView.h !!!
  # with the names like DATA_VIEW_ERR_*
  
set ::STRUCT_MSG_ERR_FIELD_NOT_FOUND            230
set ::STRUCT_MSG_ERR_BAD_SPECIAL_FIELD          229
set ::STRUCT_MSG_ERR_BAD_HANDLE                 228
set ::STRUCT_MSG_ERR_HANDLE_NOT_FOUND           227
set ::STRUCT_MSG_ERR_NOT_ENCODED                226
set ::STRUCT_MSG_ERR_ENCODE_ERROR               225
set ::STRUCT_MSG_ERR_DECODE_ERROR               224
set ::STRUCT_MSG_ERR_BAD_CRC_VALUE              223
set ::STRUCT_MSG_ERR_CRYPTO_INIT_FAILED         222
set ::STRUCT_MSG_ERR_CRYPTO_OP_FAILED           221
set ::STRUCT_MSG_ERR_CRYPTO_BAD_MECHANISM       220
set ::STRUCT_MSG_ERR_NOT_ENCRYPTED              219
set ::STRUCT_MSG_ERR_DEFINITION_NOT_FOUND       218
set ::STRUCT_MSG_ERR_DEFINITION_TOO_MANY_FIELDS 217
set ::STRUCT_MSG_ERR_BAD_TABLE_ROW              216
set ::STRUCT_MSG_ERR_TOO_MANY_FIELDS            215
set ::STRUCT_MSG_ERR_BAD_DEFINTION_CMD_KEY      214
set ::STRUCT_MSG_ERR_NO_SLOT_FOUND              213
set ::STRUCT_MSG_ERR_BAD_NUM_FIELDS             212
set ::STRUCT_MSG_ERR_ALREADY_INITTED            211
set ::STRUCT_MSG_ERR_NOT_YET_INITTED            210
set ::STRUCT_MSG_ERR_FIELD_CANNOT_BE_SET        209
set ::STRUCT_MSG_ERR_NO_SUCH_FIELD              208
set ::STRUCT_MSG_ERR_BAD_DATA_LGTH              207
set ::STRUCT_MSG_ERR_NOT_YET_PREPARED           206
set ::STRUCT_DEF_ERR_ALREADY_INITTED            205
set ::STRUCT_DEF_ERR_NOT_YET_INITTED            204
set ::STRUCT_DEF_ERR_NOT_YET_PREPARED           203
set ::STRUCT_DEF_ERR_ALREADY_CREATED            202
set ::STRUCT_MSG_ERR_FIELD_TOTAL_LGTH_MISSING   201
set ::STRUCT_MSG_ERR_NO_SUCH_COMMAND            100

set ::STRUCT_MSG_NO_INCR 0
set ::STRUCT_MSG_INCR    1
set ::STRUCT_MSG_DECR    -1

set RAND_MAX 0x7FFFFFFF

namespace eval ::structmsg {
  namespace ensemble create

  namespace export structmsgDataView

  namespace eval structmsgDataView {
    namespace ensemble create
      
    namespace export structmsgDataView freeStructmsgDataView getFieldNameIdFromStr
    namespace export setFieldValue getFieldValue
    namespace export setRandomNum getRandomNum setSequenceNum getSequenceNum
    namespace export setFiller getFiller setCrc getCrc

    variable specialFieldNames2Ids
    set specialFieldNames2Ids [dict create]
    dict set specialFieldNames2Ids "@src"              STRUCT_MSG_SPEC_FIELD_SRC
    dict set specialFieldNames2Ids "@dst"              STRUCT_MSG_SPEC_FIELD_DST
    dict set specialFieldNames2Ids "@targetCmd"        STRUCT_MSG_SPEC_FIELD_TARGET_CMD
    dict set specialFieldNames2Ids "@totalLgth"        STRUCT_MSG_SPEC_FIELD_TOTAL_LGTH
    dict set specialFieldNames2Ids "@cmdKey"           STRUCT_MSG_SPEC_FIELD_CMD_KEY
    dict set specialFieldNames2Ids "@cmdLgth"          STRUCT_MSG_SPEC_FIELD_CMD_LGTH
    dict set specialFieldNames2Ids "@randomNum"        STRUCT_MSG_SPEC_FIELD_RANDOM_NUM
    dict set specialFieldNames2Ids "@sequenceNum"      STRUCT_MSG_SPEC_FIELD_SEQUENCE_NUM
    dict set specialFieldNames2Ids "@filler"           STRUCT_MSG_SPEC_FIELD_FILLER
    dict set specialFieldNames2Ids "@crc"              STRUCT_MSG_SPEC_FIELD_CRC
    dict set specialFieldNames2Ids "@id"               STRUCT_MSG_SPEC_FIELD_ID
    dict set specialFieldNames2Ids "@tablerows"        STRUCT_MSG_SPEC_FIELD_TABLE_ROWS
    dict set specialFieldNames2Ids "@tablerowfields"   STRUCT_MSG_SPEC_FIELD_TABLE_ROW_FIELDS
    dict set specialFieldNames2Ids "@GUID"             STRUCT_MSG_SPEC_FIELD_GUID
    dict set specialFieldNames2Ids "@numNormFlds"      STRUCT_MSG_SPEC_FIELD_NUM_NORM_FLDS
    dict set specialFieldNames2Ids "@normFldIds"       STRUCT_MSG_SPEC_FIELD_NORM_FLD_IDS
    dict set specialFieldNames2Ids "@normFldNamesSize" STRUCT_MSG_SPEC_FIELD_NORM_FLD_NAMES_SIZE
    dict set specialFieldNames2Ids "@normFldNames"     STRUCT_MSG_SPEC_FIELD_NORM_FLD_NAMES
    dict set specialFieldNames2Ids "@definitionsSize"  STRUCT_MSG_SPEC_FIELD_DEFINITIONS_SIZE
    dict set specialFieldNames2Ids "@definitions"      STRUCT_MSG_SPEC_FIELD_DEFINITIONS

    variable specialFieldId2Names
    set specialFieldId2Names [dict create]
    dict set specialFieldId2Names STRUCT_MSG_SPEC_FIELD_SRC                 "@src"
    dict set specialFieldId2Names STRUCT_MSG_SPEC_FIELD_DST                 "@dst"
    dict set specialFieldId2Names STRUCT_MSG_SPEC_FIELD_TARGET_CMD          "@targetCmd"
    dict set specialFieldId2Names STRUCT_MSG_SPEC_FIELD_TOTAL_LGTH          "@totalLgth"
    dict set specialFieldId2Names STRUCT_MSG_SPEC_FIELD_CMD_KEY             "@cmdKey"
    dict set specialFieldId2Names STRUCT_MSG_SPEC_FIELD_CMD_LGTH            "@cmdLgth"
    dict set specialFieldId2Names STRUCT_MSG_SPEC_FIELD_RANDOM_NUM          "@randomNum"
    dict set specialFieldId2Names STRUCT_MSG_SPEC_FIELD_SEQUENCE_NUM        "@sequenceNum"
    dict set specialFieldId2Names STRUCT_MSG_SPEC_FIELD_FILLER              "@filler"
    dict set specialFieldId2Names STRUCT_MSG_SPEC_FIELD_CRC                 "@crc"
    dict set specialFieldId2Names STRUCT_MSG_SPEC_FIELD_ID                  "@id"
    dict set specialFieldId2Names STRUCT_MSG_SPEC_FIELD_TABLE_ROWS          "@tablerows"
    dict set specialFieldId2Names STRUCT_MSG_SPEC_FIELD_TABLE_ROW_FIELDS    "@tablerowfields"
    dict set specialFieldId2Names STRUCT_MSG_SPEC_FIELD_GUID                "@GUID"
    dict set specialFieldId2Names STRUCT_MSG_SPEC_FIELD_NUM_NORM_FLDS       "@numNormFlds"
    dict set specialFieldId2Names STRUCT_MSG_SPEC_FIELD_NORM_FLD_IDS        "@normFldIds"
    dict set specialFieldId2Names STRUCT_MSG_SPEC_FIELD_NORM_FLD_NAMES_SIZE "@normFldNamesSize"
    dict set specialFieldId2Names STRUCT_MSG_SPEC_FIELD_NORM_FLD_NAMES      "@normFldNames"
    dict set specialFieldId2Names STRUCT_MSG_SPEC_FIELD_DEFINITIONS_SIZE    "@definitionsSize"
    dict set specialFieldId2Names STRUCT_MSG_SPEC_FIELD_DEFINITIONS         "@definitions"


    # ================================= getFieldNameIdFromStr ====================================
    
    proc getFieldNameIdFromStr {fieldName fieldNameIdVar incrVal} {
      upvar $fieldNameIdVar fieldNameId
      variable specialFieldNames2Ids

      if {[string range $fieldName 0 0] eq "@"} {
        # find special field name
        if {[dict exists $specialFieldNames2Ids $fieldName]} {
          set fieldNameId [dict get $specialFieldNames2Ids $fieldName]
          return $::STRUCT_MSG_ERR_OK
        }
        return $::STRUCT_MSG_ERR_BAD_SPECIAL_FIELD
      } else {
        set firstFreeEntry [list]
        set firstFreeEntryIdx 0
        set numDefinitions [dict get $::structmsg(fieldNameDefinitions) numDefinitions]
        if {$numDefinitions > 0} {
          # find fieldName
          set nameIdx 0
          set definitions [dict get $::structmsg(fieldNameDefinitions) definitions]
          foreach entry $definitions {
            if {[dict get $entry fieldName] eq $fieldName} {
              if {$incrVal < 0} {
                if {[dict get $entry refCnt] > 0} {
                  dict set entry refCnt [expr {[dict get $entry refCnt] - 1}]
                }
                if {[dict get $entry refCnt] == 0} {
                  dict set entry id $::STRUCT_MSG_FREE_FIELD_ID
                  dict set entry fieldName ""
                }
              } else {
                if {$incrVal > 0} {
                  dict set entry refCnt [expr {[dict get $entry refCnt] + 1}]
                } else {
                  # just get the entry, do not modify
                }
              }
              set id [dict get $entry id]
              return $::STRUCT_MSG_ERR_OK
            }
            if {($incrVal == $::STRUCT_MSG_INCR) && ([dict get $entry id] eq $::STRUCT_MSG_FREE_FIELD_ID) && ($firstFreeEntry eq "")} {
              dict set firstFreeEntry id [expr {$nameIdx + 1}]
            }
            incr nameIdx
          }
        }
        if {$incrVal == $::STRUCT_MSG_DECR} {
          return $::STRUCT_MSG_ERR_OK ; # sjust ignore silently
        } else {
          if {$incrVal == $::STRUCT_MSG_NO_INCR} {
            return $::STRUCT_MSG_ERR_FIELD_NOT_FOUND
          } else {
            if {$firstFreeEntry ne ""} {
              set id [dict get $firstFreeEntry id]
              dict set firstFreeEntry refCnt 1
              dict set firstFreeEntry fieldName $fieldName
              set definitions [lreplace $definitions $firstFreeEntryIdx $firstFreeEntryIdx $firstFreeEntry]
            } else {
              incr numDefinitions
              dict set ::structmsg(fieldNameDefinitions) numDefinitions $numDefinitions
              set entry [dict create]
              dict set entry refCnt 1
              dict set entry id $numDefinitions
              dict set entry fieldName $fieldName
              set id $numDefinitions
              lappend definitions $entry
            }
          }
        }
        dict set ::structmsg(fieldNameDefinitions) definitions $definitions
      }
      return $::DATA_VIEW_ERR_OK
    }
    
    # ================================= getFieldNameStrFromId ====================================
    
    proc getFieldNameStrFromId {fieldNameId fieldNameVar} {
      upvar $fieldNameVar fieldName
      variable specialFieldId2Names

      # first try to find special field name
      if {[dict exists $specialFielId2Names $fieldNameId]} {
        set fieldName [dict get $specialFieldIds2Names $fieldNameId]
        return $::STRUCT_MSG_ERR_OK
      }
      # find field name
      set idx 0
      set fieldNameDefinitions $::structmsg(fieldNameDefinitions)
      while {$idx < [dict get $fieldNameDefinitions numDefinitions]} {
        set nameEntry [lindex  [dict get $fieldNameDefinitions definitions] $idx]
        if {[dict get $nameEntry id] == $id} {
          set fieldName [dict get $nameEntry fieldName]
          return $::STRUCT_MSG_ERR_OK
        }
        incr idx
      }
      return $::STRUCT_MSG_ERR_FIELD_NOT_FOUND
    }
    
    # ============================= getRandom ========================
    
    proc getRandom {} {
      set val [string trimleft [lindex [split [expr {rand()}] {.}] 1] 0]
      set myVal [expr {$val & $::RAND_MAX}]
      return $myVal
    }
    
    # ================================= getRandomNum ====================================
    
    proc getRandomNum {fieldInfo value} {
      return [::structmsg dataView getUint32 [dict get $fieldInfo fieldOffset] value]
    }
    
    # ================================= setRandomNum ====================================
    
    proc setRandomNum {fieldInfo} {
      set val [getRandom]
      return [::structmsg dataView setUint32 [dict get $fieldInfo fieldOffset] $val]
    }
    
    
    # ================================= getSequenceNum ====================================
    
    proc getSequenceNum {fieldInfo valueVar} {
      upvar $valueVar value

      return [::structmsg dataView getUint32 [dict get $fieldInfo fieldOffset] value]
    }
    
    # ================================= setSequenceNum ====================================
    
    proc setSequenceNum {fieldInfo} {
      incr ::sequenceNum
      return [::structmsg dataView setUint32 [dict get $fieldInfo fieldOffset] $::sequenceNum]
    }
    
    # ================================= getFiller ====================================
    
    proc getFiller {fieldInfo valueVar} {
      upvar $valueVar value

      return [::structmsg dataView getuint8Vector [dict get $fieldInfo fieldOffset] value [dict get $fieldInfo fieldLgth]]
    }
    
    # ================================= setFiller ====================================
    
    proc setFiller {fieldInfo} {
      set lgth [dict get $fieldInfo fieldLgth]
      set offset [dict get $fieldInfo fieldOffset]
      set idx 0
      set value ""
      while {$lgth >= 4} {
        set myVal [expr {[getRandom] &0xFFFFFFFF}]
        append value [binary format c [expr {($myVal >> 24) & 0xFF}]]
        append value [binary format c [expr {($myVal >> 16) & 0xFF}]]
        append value [binary format c [expr {($myVal >> 8) & 0xFF}]]
        append value [binary format c [expr {($myVal >> 0) & 0xFF}]]
        set result [::structmsg dataView setUint32 $offset $myVal]
        if {$result != $::STRUCT_MSG_ERR_OK} {
          return $result
        }
        incr offset 4
        incr lgth -4
      }
      while {$lgth >= 2} {
        set myVal [expr {[getRandom] & 0xFFFF}]
        append value [binary format c [expr {($myVal >> 8) & 0xFF}]]
        append value [binary format c [expr {($myVal >> 0) & 0xFF}]]
        set result [::structmsg dataView setUint16 $offset $myVal]
        if {$result != $::STRUCT_MSG_ERR_OK} {
          return $result
        }
        incr offset 2
        incr lgth -2
      }
      while {$lgth >= 1} {
        set myVal [expr {[getRandom] & 0xFF}]
        append value [binary format c [expr {($myVal >> 0) & 0xFF}]]
        set result [::structmsg dataView setUint8 $offset $myVal]
        if {$result != $::STRUCT_MSG_ERR_OK} {
          return $result
        }
        incr offset 1
        incr lgth -1
      }
      return $::DATA_VIEW_ERR_OK
    }
    
    
    # ================================= getCrc ====================================
    
    proc getCrc {fieldInfo valueVar startOffset size} {
      upvar $valueVar value

      set crcLgth [dict get $fieldInfo fieldLgth]
      set value ""
      set lgth [expr {$size - $crcLgth}]
      set crcVal 0
      set offset $startOffset
set cnt 0
      while {$offset < $size} {
        set result [::structmsg dataView getUint8 $offset ch]
        set pch $ch
        if {![string is integer $ch]} {
          binary scan $ch c pch
        }
        set pch [expr {$pch & 0xFF}]
if {$::crcDebug} {
puts stderr "decode crc: $cnt ch: [format 0x%02x $pch]![format 0x%04x $crcVal]!"
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
        set result [::structmsg dataView getuint16 $offset crc]
        if {$result != $::STRUCT_MSG_ERR_OK} {
          return $result
        }
if {$::crcDebug} {
puts stderr "crcVal: [format 0x%04x $crcVal]!offset: $offset!crc: [format 0x%04x $crc]!"
}
        if {$crcVal != $crc} {
          return $::STRUCT_MSG_ERR_BAD_CRC_VALUE
        }
        set value $crc
      } else  {
        set crcVal [expr {~$crcVal}]
        set crcVal [expr {$crcVal & 0xFF}]
if {$::crcDebug} {
puts stderr "crcVal2 end: $crcVal!"
}
        set result [::structmsg dataView getUint8 $offset crc]
        if {$result != $::STRUCT_MSG_ERR_OK} {
          return $result
        }
if {$::crcDebug} {
puts stderr "crcVal: [format 0x%02x [expr {$crcVal & 0xFF}]]!offset: $offset!crc: [format 0x%02x $crc]!"
}
        if {[expr {$crcVal & 0xFF}] != $crc} {
          return $::STRUCT_MSG_ERR_BAD_CRC_VALUE
        }
        set value $crc
      }
      return $::DATA_VIEW_ERR_OK
    }
    
    # ================================= setCrc ====================================
    
    proc setCrc {fieldInfo startOffset size} {
      set crcLgth [dict get $fieldInfo fieldLgth]
      set size [expr {$size - $crcLgth}]
      set crc  0
      set offset $startOffset
      while {$offset < $size} {
set cnt 0
        set result [::structmsg dataView getUint8 $offset ch]
        set pch $ch
        if {![string is integer $ch]} {
          binary scan $ch c pch
        }
        set pch [expr {$pch & 0xFF}]
if {$::crcDebug} {
puts stderr "encode crc: $cnt $ch![format 0x%02x $pch]![format 0x%04x $crc]!"
}
        set crc [expr {$crc + [format "%d" $pch]}]
incr cnt
        incr offset
      }
if {$::crcDebug} {
puts stderr "crc1: $crc![format 0x%04x $crc]!"
}
      set crc [expr {~$crc & 0xFFFF}]
      if {$crcLgth == 1} {
        set result [::structmsg dataView setUint8 [dict get $fieldInfo fieldOffset] [expr {$crc & 0xFF}]]
      } else {
        set result [::structmsg dataView setUint16 [dict get $fieldInfo fieldOffset] $crc]
      }
      return $result
    }
    
    
    # ================================= getFieldValue ====================================
    
    proc getFieldValue {fieldInfo valueVar fieldIdx} {
      upvar $valueVar value

      set value ""
      switch [dict get $fieldInfo fieldTypeId] {
        DATA_VIEW_FIELD_INT8_T {
          set result [::structmsg dataView getInt8 [dict get $fieldInfo fieldOffset] value]
        }
        DATA_VIEW_FIELD_UINT8_T {
          set result [::structmsg dataView getUint8 [dict get $fieldInfo fieldOffset] value]
        }
        DATA_VIEW_FIELD_INT16_T {
          set result [::structmsg dataView getInt16 [dict get $fieldInfo fieldOffset] value]
        }
        DATA_VIEW_FIELD_UINT16_T {
          set result [::structmsg dataView getUint16 [dict get $fieldInfo fieldOffset] value]
        }
        DATA_VIEW_FIELD_INT32_T {
          set result [::structmsg dataView getInt32 [dict get $fieldInfo fieldOffset] value]
        }
        DATA_VIEW_FIELD_UINT32_T {
          set result [::structmsg dataView getUint32 [dict get $fieldInfo fieldOffset] value]
        }
        DATA_VIEW_FIELD_INT8_VECTOR {
          set result [::structmsg dataView getInt8Vector [expr {[dict get $fieldInfo fieldOffset]}] value]
        }
        DATA_VIEW_FIELD_UINT8_VECTOR {
          set result [::structmsg dataView getUint8Vector [expr {[dict get $fieldInfo fieldOffset]}] value]
        }
        DATA_VIEW_FIELD_INT16_VECTOR {
          set result [::structmsg dataView getInt16 [expr {[dict get $fieldInfo fieldOffset] +fieldIdx*2}] value]
        }
        DATA_VIEW_FIELD_UINT16_VECTOR {
          set result [::structmsg dataView getUint16 [expr {[dict get $fieldInfo fieldOffset] + $fieldIdx*2]} value]
        }
        DATA_VIEW_FIELD_INT32_VECTOR {
            return $::STRUCT_MSG_ERR_BAD_VALUE
        }
        DATA_VIEW_FIELD_UINT32_VECTOR {
            return $::STRUCT_MSG_ERR_BAD_VALUE
        }
        default {
          return $::STRUCT_MSG_ERR_BAD_FIELD_TYPE
        }
      }
      return $result
    }
    
    # ================================= setFieldValue ====================================
    
    proc setFieldValue {fieldInfo value fieldIdx} {
      switch [dict get $fieldInfo fieldTypeId] {
        DATA_VIEW_FIELD_INT8_T {
          if {($alue > -128) && ($value < 128)} {
            set result [::structmsg dataView setInt8 [dict get $fieldInfo fieldOffset] $value]
          } else {
            return $::STRUCT_MSG_ERR_VALUE_TOO_BIG
          }
        }
        DATA_VIEW_FIELD_UINT8_T {
          if {($alue >= 0) && ($value <= 256)} {
            set result [::structmsg dataView setUint8 [dict get $fieldInfo fieldOffset] $value]
          } else {
            return $::STRUCT_MSG_ERR_BAD_VALUE
          }
        }
        DATA_VIEW_FIELD_INT16_T {
          if {($value > -32767) && ($value < 32767)} {
            set result [::structmsg dataView setInt16 [dict get $fieldInfo fieldOffset] $value]
          } else {
            return $::STRUCT_MSG_ERR_VALUE_TOO_BIG
          }
        }
        DATA_VIEW_FIELD_UINT16_T {
          if {($value >= 0) && ($value <= 65535)} {
            set result [::structmsg dataView setUint16 [dict get $fieldInfo fieldOffset] $value]
          } else {
            return $::STRUCT_MSG_ERR_VALUE_TOO_BIG
          }
        }
        DATA_VIEW_FIELD_INT32_T {
          if {($value > -0x7FFFFFFF) && ($value <= 0x7FFFFFFF)} {
            set result [::structmsg dataView setInt32 [dict get $fieldInfo fieldOffset] $value]
          } else {
            return $::STRUCT_MSG_ERR_VALUE_TOO_BIG
          }
        }
        DATA_VIEW_FIELD_UINT32_T {
          # we have to do the signed check as numericValue is a signed integer!!
          if ((numericValue > -0x7FFFFFFF) && (numericValue <= 0x7FFFFFFF)) {
            set result [::structmsg dataView setUint32 [dict get $fieldInfo fieldOffset] $value]
          } else {
            return $::STRUCT_MSG_ERR_VALUE_TOO_BIG
          }
        }
        DATA_VIEW_FIELD_INT8_VECTOR {
          set result [::structmsg ataView setInt8Vector [dict get $fieldInfo fieldOffset] $value]
        }
        DATA_VIEW_FIELD_UINT8_VECTOR {
          set result [::structmsg dataView setUint8Vector [dict get $fieldInfo fieldOffset] $value]
        }
        DATA_VIEW_FIELD_INT16_VECTOR {
          set result [::structmsg dataView setInt16Vector [expr {[dict get $fieldInfo fieldOffset] + $fieldIdx*2}] $value]
        }
        DATA_VIEW_FIELD_UINT16_VECTOR {
          set result [::structmsg dataView setUint16Vector [expr {[dict get $fieldInfo fieldOffset] + $fieldIdx*2}] $value]
        }
        DATA_VIEW_FIELD_INT32_VECTOR {
          set result [::structmsg dataView setInt32Vector [expr {[dict get $fieldInfo fieldOffset] + $fieldIdx*2}] $value]
        }
        DATA_VIEW_FIELD_UINT32_VECTOR {
          set result [::structmsg dataView setUint32Vector [expr {[dict get $fieldInfo fieldOffset] + $fieldIdx*2}] $value]
        }
        default {
puts stderr "bad type in setFieldValue: [dict get $fieldInfo fieldTypeId]"
          return $::STRUCT_MSG_ERR_BAD_FIELD_TYPE
        }
      }
      return $result
    }

    # ================================= structmsgDataView ====================================
    
    proc structmsgDataView {command args} {
      switch $command {
        getFieldNameStrFromId -
        getFieldNameIdFromStr {
         return [uplevel 0 $command $args]
        }
      }
puts stderr "structmsgDataView!no such command!$command!"
      return $::STRUCT_MSG_ERR_NO_SUCH_COMMAND
      if {0} {
        structmsgDataView->getFieldNameIdFromStr = &getFieldNameIdFromStr;
        structmsgDataView->getFieldNameStrFromId = &getFieldNameStrFromId;

        structmsgDataView->getRandomNum = &getRandomNum;
        structmsgDataView->setRandomNum = &setRandomNum;

        structmsgDataView->getSequenceNum = &getSequenceNum;
        structmsgDataView->setSequenceNum = &setSequenceNum;

        structmsgDataView->getFiller = &getFiller;
        structmsgDataView->setFiller = &setFiller;

        structmsgDataView->getCrc = &getCrc;
        structmsgDataView->setCrc = &setCrc;

        structmsgDataView->getFieldValue = &getFieldValue;
        structmsgDataView->setFieldValue = &setFieldValue;
      }
    }

  } ; # namespace structmsgDataView
} ; # namespace structmsg
