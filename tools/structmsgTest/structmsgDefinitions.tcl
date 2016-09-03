#!/usr/bin/env tclsh8.6

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

set ::structmsg(numStructmsgDefinitions) 0
set ::structmsg(structmsgDefinitions) [dict create]
set ::structmsg(fieldTypeDefines) [list]
set ::structmsg(fieldNameDefinitions) [dict create]
dict set ::structmsg(fieldNameDefinitions) numDefinitions 0
dict set ::structmsg(fieldNameDefinitions) definitions [list]
set ::structmsg(numFieldNameIds) 0

namespace eval structmsg {
  namespace ensemble create

  namespace export def

  namespace eval def {
    namespace ensemble create
      
    namespace export initFieldTypeDefines initSpecialFieldNames getFieldNameId
    namespace export getFieldTypeId createDefinition addFieldDefinition deleteFieldDefinition
    namespace export deleteFieldDefinitions dumpFieldDefinition encryptDefinition decryptDefinition
    namespace export encodeFieldDefinition decodeFieldDefinition getIdFieldNameStr getFieldTypeStr
    namespace export createMsgFromDefinition

    # ============================= initFieldTypeDefines ========================
    
    proc initFieldTypeDefines {} {
      set fieldTypeDefinesDict [dict create]
      dict set fieldTypeDefinesDict "uint8_t"    1
      dict set fieldTypeDefinesDict "int8_t"     2
      dict set fieldTypeDefinesDict "uint16_t"   3
      dict set fieldTypeDefinesDict "int16_t"    4
      dict set fieldTypeDefinesDict "uint32_t"   5
      dict set fieldTypeDefinesDict "int32_t"    6
      dict set fieldTypeDefinesDict "uint8_t*"   7
      dict set fieldTypeDefinesDict "int8_t*"    8
      dict set fieldTypeDefinesDict "uint16_t*"  9
      dict set fieldTypeDefinesDict "int16_t*"   10
      dict set fieldTypeDefinesDict "uint32_t*"  11
      dict set fieldTypeDefinesDict "int32_t*"   12
      set ::structmsg(fieldTypeDefines) $fieldTypeDefinesDict
      set fieldTypeIdsDict [dict create]
      dict set fieldTypeIdsDict 1  "uint8_t"
      dict set fieldTypeIdsDict 2  "int8_t"
      dict set fieldTypeIdsDict 3  "uint16_t"
      dict set fieldTypeIdsDict 4  "int16_t"
      dict set fieldTypeIdsDict 5  "uint32_t"
      dict set fieldTypeIdsDict 6  "int32_t"
      dict set fieldTypeIdsDict 7  "uint8_t*"
      dict set fieldTypeIdsDict 8  "int8_t*"
      dict set fieldTypeIdsDict 9  "uint16_t*"
      dict set fieldTypeIdsDict 10 "int16_t*"
      dict set fieldTypeIdsDict 11 "uint32_t*"
      dict set fieldTypeIdsDict 12 "int32_t*"
      set ::structmsg(fieldTypeIds) $fieldTypeIdsDict
    }
    
    # ============================= initSpecialFieldNames ========================
    
    proc initSpecialFieldNames {} {
      set specialFieldNamesDict [dict create]
      dict set specialFieldNamesDict "@src"            255
      dict set specialFieldNamesDict "@dst"            254
      dict set specialFieldNamesDict "@targetCmd"      253
      dict set specialFieldNamesDict "@totalLgth"      252
      dict set specialFieldNamesDict "@cmdKey"         251
      dict set specialFieldNamesDict "@cmdLgth"        250
      dict set specialFieldNamesDict "@randomNum"      249
      dict set specialFieldNamesDict "@sequenceNum"    248
      dict set specialFieldNamesDict "@filler"         247
      dict set specialFieldNamesDict "@crc"            246
      dict set specialFieldNamesDict "@id"             245
      dict set specialFieldNamesDict "@tablerows"      244
      dict set specialFieldNamesDict "@tablerowfields" 243
      dict set specialFieldNamesDict "@low"            242
      set ::structmsg(specialFieldNames) $specialFieldNamesDict
      set specialFieldIdsDict [dict create]
      dict set specialFieldIdsDict 255 "@src"
      dict set specialFieldIdsDict 254 "@dst"
      dict set specialFieldIdsDict 253 "@targetCmd"
      dict set specialFieldIdsDict 252 "@totalLgth"
      dict set specialFieldIdsDict 251 "@cmdKey"
      dict set specialFieldIdsDict 250 "@cmdLgth"
      dict set specialFieldIdsDict 249 "@randomNum"
      dict set specialFieldIdsDict 248 "@sequenceNum"
      dict set specialFieldIdsDict 247 "@filler"
      dict set specialFieldIdsDict 246 "@crc"
      dict set specialFieldIdsDict 245 "@id"
      dict set specialFieldIdsDict 244 "@tablerows"
      dict set specialFieldIdsDict 243 "@tablerowfields"
      dict set specialFieldIdsDict 242 "@low"
      set ::structmsg(specialFieldIds) $specialFieldIdsDict
    }
    
    # ============================= getFieldTypeStr ========================
    
    proc getFieldTypeStr {id fieldTypeVar} {
      upvar $fieldTypeVar fieldType
    
      set fieldTypes $::structmsg(fieldTypeDefines)
      foreach name [dict keys $fieldTypes] {
        set fieldId [dict get $fieldTypes $name]
        if {$fieldId == $id} {
          set fieldType $name
          return $::STRUCT_MSG_ERR_OK;
        }
      }
      return $::STRUCT_MSG_ERR_BAD_FIELD_TYPE
    }
    
    # ============================= getFieldNameId ========================
    
    proc getFieldNameId {fieldName id_var incrVal} {
      upvar $id_var id
    
      set fieldNameDefinitionsDict $::structmsg(fieldNameDefinitions)
      switch -glob -- $fieldName {
        "@*" {
          if {![dict exists $::structmsg(specialFieldNames) $fieldName]} {
            error "bad special field name: $fieldName!"
          } else {
            set id [dict get $::structmsg(specialFieldNames) $fieldName]
          }
        }
        default {
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
      }
      return $::STRUCT_MSG_ERR_OK
    }
    
    # ============================= getIdFieldNameStr ========================
    
    proc getIdFieldNameStr {id fieldNameVar} {
      upvar $fieldNameVar fieldName
    
      # first try to find special field name
      set specialFieldNames $::structmsg(specialFieldNames)
      foreach name [dict keys $specialFieldNames] {
        set fieldId [dict get $specialFieldNames $name]
        if {$fieldId == $id} {
          set fieldName $name
          return $::STRUCT_MSG_ERR_OK;
        }
      }
      # find field name
      set idx 0
      set fieldNameDefinitions $::structmsg(fieldNameDefinitions)
      while {$idx < [dict get $fieldNameDefinitions numDefinitions]} {
        set nameEntry [lindex  [dict get $fieldNameDefinitions definitions] $idx]
        if {[dict get $nameEntry id] == $id} {
          set fieldName [dict get $nameEntry fieldName]
          return $::STRUCT_MSG_ERR_OK;
        }
        incr idx
      }
      return $::STRUCT_MSG_ERR_FIELD_NOT_FOUND;
    }
    
    
    # ============================= getFieldTypeId ========================
    
    proc getFieldTypeId {fieldType fieldTypeVar} {
      upvar $fieldTypeVar fieldTypeId
    
      set fieldTypeDefinesDict $::structmsg(fieldTypeDefines)
      if {![dict exists $fieldTypeDefinesDict $fieldType]} {
    #  return $::STRUCT_MSG_ERR_BAD_FIELD_TYPE;
          error "bad field type: $fieldType!"
      } else {
          set fieldTypeId [dict get $fieldTypeDefinesDict $fieldType]
      }
      return $::STRUCT_MSG_ERR_OK
    }
    
    # ============================= createDefinition ========================
    
    proc createDefinition {name numFields} {
      if {![dict exists $::structmsg(structmsgDefinitions) numDefinitions]} {
        dict set ::structmsg(structmsgDefinitions) numDefinitions 0
        dict set ::structmsg(structmsgDefinitions) maxDefinitions 0
        dict set ::structmsg(structmsgDefinitions) definitions [list]
      }
      # check for unused slot!
      set definitions [dict get $::structmsg(structmsgDefinitions) definitions]
      set definitionIdx 0
      while {$definitionIdx < [dict get $::structmsg(structmsgDefinitions) maxDefinitions]} {
        set definition [lindex $definitions $definitionIdx]
        if {[dict get $definition name] eq ""} {
          dict set definition numFields 0
          dict set definition maxFields $numFields
          dict set definition name $name
          dict set definition fieldInfos [list]
          set definitions [lreplace $definitions $definitionIdx $definitionIdx $definition]
          dict set ::structmsg(structmsgDefinitions) definitions $definitions
          return $::STRUCT_MSG_ERR_OK;
        } 
        incr definitionIdx
      }
      if {$definitionIdx >= [dict get $::structmsg(structmsgDefinitions) numDefinitions]} {
        dict set ::structmsg(structmsgDefinitions) numDefinitions [expr {[dict get $::structmsg(structmsgDefinitions) numDefinitions] + 1}]
        dict set ::structmsg(structmsgDefinitions) maxDefinitions [expr {[dict get $::structmsg(structmsgDefinitions) maxDefinitions] + 1}]
      }
      dict set definition numFields 0
      dict set definition maxFields $numFields
      dict set definition name $name
      dict set definition fieldInfos [list]
      lappend definitions $definition    
      dict set ::structmsg(structmsgDefinitions) definitions $definitions
      return $::STRUCT_MSG_ERR_OK;
    }
    
    # ============================= addFieldDefinition ========================
    
    proc addFieldDefinition {name fieldName fieldType fieldLgth} {
      set fieldDefinitionDict [dict create]
      set definitions [dict get $::structmsg(structmsgDefinitions) definitions]
      set definitionIdx 0
      set found 0
      foreach definition $definitions {
        if {[dict get $definition name] eq $name} {
          set found 1
          break
        }
        incr definitionIdx
      } 
      if {!$found} {
        error "structmsg definition for $name does not exist"
#        return $::STRUCT_MSG_ERR_DEFINITION_NOT_FOUND;
      }
      if {[dict get $definition numFields] >= [dict get $definition maxFields]} {
        return $::STRUCT_MSG_ERR_DEFINITION_TOO_MANY_FIELDS
      }
#puts stderr "add_field: fieldName: $fieldName fieldtype: $fieldType fieldLgth: $fieldLgth!"
      set fieldInfos [dict get $definition fieldInfos]
      set result [getFieldNameId $fieldName fieldNameId $::STRUCT_MSG_INCR]
      set result [getFieldTypeId $fieldType fieldTypeId]
      set fielNameDict [dict create]
      dict set fieldNameDict fieldId $fieldNameId
      dict set fieldNameDict fieldType $fieldTypeId
      dict set fieldNameDict fieldLgth $fieldLgth
      lappend fieldInfos $fieldNameDict
      dict set definition fieldInfos $fieldInfos
      dict set definition numFields [expr {[dict get $definition numFields] + 1}]
      set definitions [lreplace $definitions $definitionIdx $definitionIdx $definition]
      dict set ::structmsg(structmsgDefinitions) definitions $definitions
      return $::STRUCT_MSG_ERR_OK
    }
    
    # ============================= dumpFieldDefinition ========================
    
    proc dumpFieldDefinition {name} {
      set definitions [dict get $::structmsg(structmsgDefinitions) definitions]
      set definitionIdx 0
      set found 0
      foreach definition $definitions {
        if {[dict get $definition name] eq $name} {
          set found 1
          break
        }
        incr definitionIdx
      } 
      if {!$found} {
        error "structmsg definition for $name does not exist"
#        return $::STRUCT_MSG_ERR_DEFINITION_NOT_FOUND;
      }
      puts [format "definition: %s numFields: %d" $name [dict get $definition numFields]]
      set idx 0
      while {$idx < [dict get $definition numFields]} {
        set fieldInfo [lindex [dict get $definition fieldInfos] $idx]
        set result [getIdFieldNameStr [dict get $fieldInfo fieldId] fieldIdStr]
#        checkErrOK $result
        set result [getFieldTypeStr [dict get $fieldInfo fieldType] fieldTypeStr]
#        checkErrOK(result);
        puts [format "  idx: %d id: %d %s type: %d %s lgth: %d" $idx [dict get $fieldInfo fieldId] $fieldIdStr [dict get $fieldInfo fieldType] $fieldTypeStr [dict get $fieldInfo fieldLgth]]
        incr idx
      }
      return $::STRUCT_MSG_ERR_OK;
    }
    
    # ============================= createMsgFromDefinition ========================
    
    proc createMsgFromDefinition {name handleVar} {
      upvar $handleVar handle

      set result [getDefinitionDict $name definition]
#      checkErrOK(result);
puts stderr "DEF2: $definition!"
      set result [::structmsg cmd create [expr {[dict get $definition numFields] - $::STRUCT_MSG_NUM_HEADER_FIELDS - $::STRUCT_MSG_NUM_CMD_HEADER_FIELDS}] handle]
puts stderr [format "create: handle: %s numFields: %d" $handle [expr {[dict get $definition numFields] - $::STRUCT_MSG_NUM_HEADER_FIELDS - $::STRUCT_MSG_NUM_CMD_HEADER_FIELDS}]]
#      checkErrOK(result);
      set fieldIdx 0
puts stderr "CMFD1: numField: [dict get $definition numFields]!"
      while {$fieldIdx < [dict get $definition numFields]} {
        set fieldInfos [dict get $definition fieldInfos]
        set fieldInfo [lindex $fieldInfos $fieldIdx]
puts stderr "MCFD2: fieldInfo: $fieldInfo!$fieldIdx!"
        set fieldId [dict get $fieldInfo fieldId]
        set fieldName "??"
        if {[dict exists $::structmsg(specialFieldIds) $fieldId]} {
          set fieldName [dict get $::structmsg(specialFieldIds) $fieldId]
puts stderr "CMFD3: fieldName: $fieldName!"
        }
        switch $fieldName { 
          @src -
          @dst -
          @targetCmd -
          @totalLgth -
          @cmdKey -
          @cmdLgth {
            # nothing to do!
          }
          default {
            set result [getIdFieldNameStr [dict get $fieldInfo fieldId] fieldName]
#      checkErrOK(result);
            set result [getFieldTypeStr [dict get $fieldInfo fieldType] fieldType]
#      checkErrOK(result);
#puts stderr [format {addfield: %s %s %d} $fieldName $fieldType [dict get $fieldInfo fieldLgth]]
            # @filler and @crc are handled in encode!!
            if {!(($fieldName eq "@filler") || ($fieldName eq "@crc"))} {
              set result [::structmsg cmd add_field $handle $fieldName $fieldType [dict get $fieldInfo fieldLgth]]
            }
          }
        } 
        incr fieldIdx
      }
puts stderr "createMsgFromDefinition END"
::structmsg cmd dump $handle
      return $::STRUCT_MSG_ERR_OK
    }
    
    # ============================= getDefinitionDict ========================
    
    proc getDefinitionDict {name stmsgDefinitionVar} {
      upvar $stmsgDefinitionVar stmsgDefinition
    
      set idx 0
      foreach definition [dict get $::structmsg(structmsgDefinitions) definitions] {
        if {([dict get $definition name] ne "") && ($name eq [dict get $definition name])} {
          set stmsgDefinition $definition
          return $::STRUCT_MSG_ERR_OK
        }
      }
      return $::STRUCT_MSG_ERR_DEFINITION_NOT_FOUND;
    }
    
    # ============================= encodeFieldDefinition ========================
    
    proc encodeFieldDefinition {name dataVar lgthVar} {
      upvar $dataVar data
      upvar $lgthVar lgth
    
      set src 123
      set dst 987
      set cmdKey $::STRUCT_MSG_DEFINITION_CMD_KEY
    
      set idx 0
      while {$idx < [dict get $::structmsg(structmsgDefinitions) numDefinitions]} {
        set definition [lindex [dict get $::structmsg(structmsgDefinitions) definitions] $idx]
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
          set result [::structmsg encdec getFieldIdName [dict get $fieldInfo fieldId] fieldName]
          if {$result != $::STRUCT_MSG_ERR_OK} { return $result }
    #puts stderr [format "fieldName: %s" $fieldName]
          incr numNormFields
          set normNamesSize [expr {$normNamesSize + [string length $fieldName] + 1}]
        }
        incr idx
      }
      set normNamesOffsets [list]
      # nameLgth + name of Definition
      set definitionPayloadSize [expr {[sizeof uint8_t] + [string length $name] + 1}]
      # fieldId uint16_t, fieldType uint8_t, fieldLgth uint16_t
      set definitionPayloadSize [expr {$definitionPayloadSize + [dict get $definition numFields] * ([sizeof uint16_t] + [sizeof uint8_t] + [sizeof uint16_t])}]
      set payloadSize $::STRUCT_MSG_CMD_HEADER_LENGTH ; # cmdKey + cmdLgth
      # numEntries uint8_t randomNum
      set payloadSize [expr {$payloadSize + [sizeof uint8_t] + [sizeof uint32_t]}]
      # len ids + ids (numNormFields * (uint16_t)) + len Names + names size
      set payloadSize [expr {$payloadSize + [sizeof uint8_t] + $numNormFields * [sizeof uint16_t] + [sizeof uint16_t] + $normNamesSize}]
      # size definitionPayload + definitionPayload
      set payloadSize [expr {$payloadSize + [sizeof uint16_t] + $definitionPayloadSize}]
      set fillerSize 0
      set myLgth [expr {$payloadSize + [sizeof uint16_t]}] ; # sizeof(uint16_t) for CRC
      while {($myLgth % 16) != 0} {
        incr myLgth
        incr fillerSize
      }
      set cmdLgth [expr {$payloadSize + $fillerSize + [sizeof uint16_t]}]
      set totalLgth [expr {$::STRUCT_MSG_HEADER_LENGTH + $cmdLgth}]
    puts stderr [format "cmdLgth: %d totalLgth: %d" $cmdLgth $totalLgth]
      dict set definition encoded [list]
      set encoded ""
      set offset 0
      set offset [::structmsg encdec uint16Encode encoded $offset $src]
      if {$offset < 0} { return $::STRUCT_MSG_ERR_ENCODE_ERROR }
      set offset [::structmsg encdec uint16Encode encoded $offset $dst]
      if {$offset < 0} { return $::STRUCT_MSG_ERR_ENCODE_ERROR }
      set offset [::structmsg encdec uint16Encode encoded $offset $totalLgth]
      if {$offset < 0} { return $::STRUCT_MSG_ERR_ENCODE_ERROR }
      set offset [::structmsg encdec uint16Encode encoded $offset $cmdKey]
      if {$offset < 0} { return $::STRUCT_MSG_ERR_ENCODE_ERROR }
      set offset [::structmsg encdec uint16Encode encoded $offset $cmdLgth]
      if {$offset < 0} { return $::STRUCT_MSG_ERR_ENCODE_ERROR }
      set offset [::structmsg encdec randomNumEncode encoded $offset randomNum]
      if {$offset < 0} { return $::STRUCT_MSG_ERR_ENCODE_ERROR }
      set offset [::structmsg encdec normalFieldNamesEncode encoded $offset $definition normNamesOffsets $numNormFields $normNamesSize]
      if {$offset < 0} { return $::STRUCT_MSG_ERR_ENCODE_ERROR }
      set offset [::structmsg encdec uint16Encode encoded $offset $definitionPayloadSize]
      if {$offset < 0} { return $::STRUCT_MSG_ERR_ENCODE_ERROR }
      set offset [::structmsg encdec uint8Encode encoded $offset [expr {[string length $name] + 1}]]
      if {$offset < 0} { return $::STRUCT_MSG_ERR_ENCODE_ERROR }
      set offset [::structmsg encdec uint8VectorEncode encoded $offset $name [string length $name ]]
      if {$offset < 0} { return $::STRUCT_MSG_ERR_ENCODE_ERROR }
      set offset [::structmsg encdec uint8Encode encoded $offset 0]
      if {$offset < 0} { return $::STRUCT_MSG_ERR_ENCODE_ERROR }
      set offset [::structmsg encdec definitionEncode encoded $offset $definition $normNamesOffsets]
      if {$offset < 0} { return $::STRUCT_MSG_ERR_ENCODE_ERROR }
      set offset [::structmsg encdec fillerEncode encoded $offset $fillerSize dummy]
      if {$offset < 0} { return $::STRUCT_MSG_ERR_ENCODE_ERROR }
      set offset [::structmsg encdec crcEncode encoded $offset $totalLgth crc $::STRUCT_MSG_HEADER_LENGTH]
      if {$offset < 0} { return $::STRUCT_MSG_ERR_ENCODE_ERROR }
    puts stderr [format "after crc offset: %d totalLgth :%d crc: 0x%04x" $offset $totalLgth $crc]
      set data $encoded
      set lgth $totalLgth
      return $::STRUCT_MSG_ERR_OK
    }
    
    # ============================= decodeFieldDefinition ========================
    
    proc decodeFieldDefinition {name data} {
      set offset 0
      set offset [::structmsg encdec uint16Decode $data $offset src]
      if {$offset < 0} { return $::STRUCT_MSG_ERR_DECODE_ERROR }
      set offset [::structmsg encdec uint16Decode $data $offset dst]
      if {$offset < 0} { return $::STRUCT_MSG_ERR_DECODE_ERROR }
      set offset [::structmsg encdec uint16Decode $data $offset totalLgth]
      if {$offset < 0} { return $::STRUCT_MSG_ERR_DECODE_ERROR }
      set offset [::structmsg encdec uint16Decode $data $offset cmdKey]
      if {$offset < 0} { return $::STRUCT_MSG_ERR_DECODE_ERROR }
      if {$cmdKey != $::STRUCT_MSG_DEFINITION_CMD_KEY} {
        return $::STRUCT_MSG_ERR_BAD_DEFINTION_CMD_KEY
      }
      set offset [::structmsg encdec uint16Decode $data $offset cmdLgth]
      if {$offset < 0} { return $::STRUCT_MSG_ERR_DECODE_ERROR }
      set offset [::structmsg encdec uint32Decode $data $offset randomNum]
      if {$offset < 0} { return $::STRUCT_MSG_ERR_DECODE_ERROR }
      # now check the crc
      set crcOffset [expr {$totalLgth - [sizeof uint16_t]}]
      set crcOffset [::structmsg encdec crcDecode $data $crcOffset $cmdLgth crc $::STRUCT_MSG_HEADER_LENGTH]
    puts stderr "crcOffset: $crcOffset!offset: $offset!"
      set offset [::structmsg encdec definitionDecode $data $offset]
      if {$offset < 0} { return $::STRUCT_MSG_ERR_DECODE_ERROR }
      set myLgth [expr {$offset - $::STRUCT_MSG_HEADER_LENGTH + [sizeof uint16_t]}]
      set fillerSize 0
      while {($myLgth % 16) != 0} {
        incr myLgth
        incr fillerSize
      }
      set offset [::structmsg encdec fillerDecode $data $offset $fillerSize filler]
      if {$offset < 0} { return $::STRUCT_MSG_ERR_DECODE_ERROR }
      return $::STRUCT_MSG_ERR_OK
    }
    
    # ============================= deleteFieldDefinition ========================
    
    proc deleteFieldDefinition {name} {
      set definitionsIdx 0
      set found 0
      set definitions [dict get $::structmsg(structmsgDefinitions) definitions]
      while {$definitionsIdx < [dict get $::structmsg(structmsgDefinitions) numDefinitions]} {
        set definition [lindex $definitions $definitionsIdx]
        if {([dict get $definition name] ne "") && ($name eq [dict get $definition name])} {
          set found 1
          break
        }
        incr definitionsIdx
      }
      if {!$found} {
        return $::STRUCT_MSG_ERR_DEFINITION_NOT_FOUND
      }
      set definitionIdx 0
      while {$definitionIdx < [dict get $definition numFields]} {
        set fieldInfo [lindex [dict get $definition fieldInfos] $definitionIdx]
        set nameIdx 0
        set nameFound 0
        if {[dict get $fieldInfo fieldId] < $::STRUCT_MSG_SPEC_FIELD_LOW} {
          while {$nameIdx < [dict get $::structmsg(fieldNameDefinitions) numDefinitions]} {
            set nameEntry [lindex [dict get $::structmsg(fieldNameDefinitions) definitions] $nameIdx]
            if {[dict get $fieldInfo fieldId] == [dict get $nameEntry id]} {
              set result [getFieldNameId [dict get $nameEntry fieldName] fieldId $::STRUCT_MSG_DECR]
              if {$result != $::STRUCT_MSG_ERR_OK} { return $result }
              set nameFound 1
              break
            }
            incr nameIdx
          }
          if {!$nameFound} {
            return $::STRUCT_MSG_ERR_FIELD_NOT_FOUND
          }
        }
        incr definitionIdx
      }
      # nameDefinitions deleted
    
      dict set definition numFields 0
      dict set definition maxFields 0
      dict set definition name ""
      if {[dict exists $definition encoded]} {
        dict set definition encoded ""
      }
      dict set definition fieldInfos ""
      set definitions [lreplace $definitions $definitionsIdx $definitionsIdx $definition]
      dict set ::structmsg(structmsgDefinitions) definitions $definitions
      # definition deleted
    
      return $::STRUCT_MSG_ERR_OK
    }
    
    # ============================= deleteFieldDefinitions ========================
    
    proc deleteFieldDefinitions {} {
      # delete the whole structmsgDefinitions info, including fieldNameDefinitions info
    
      set idx 0
      while {$idx < [dict get $::structmsg(structmsgDefinitions) numDefinitions]} {
        set definition [dict get $::structmsg(structmsgDefinitions) definitions] $idx]
        if {[dict get $definition name] ne ""} {
          deleteFieldDefinition [dict get $definition name]
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

  } ; # namespace def

} ; # namespace structmsg
