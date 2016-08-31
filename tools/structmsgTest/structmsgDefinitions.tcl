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

# ============================= structmsg_initFieldTypeDefines ========================

proc structmsg_initFieldTypeDefines {} {
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

# ============================= structmsg_initFieldNameDefines ========================

proc structmsg_initSpecialFieldNames {} {
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

# ============================= structmsg_getFieldTypeStr ========================

proc structmsg_getFieldTypeStr {id fieldTypeVar} {
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

# ============================= structmsg_getFieldNameId ========================

proc structmsg_getFieldNameId {fieldName id_var incrVal} {
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

# ============================= structmsg_getIdFieldNameStr ========================

proc structmsg_getIdFieldNameStr {id fieldNameVar} {
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


# ============================= structmsg_getFieldTypeId ========================

proc structmsg_getFieldTypeId {fieldType fieldTypeVar} {
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

# ============================= structmsg_createStructmsgDefinition ========================

proc structmsg_createStructmsgDefinition {name numFields} {
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

# ============================= structmsg_addFieldDefinition ========================

proc structmsg_addFieldDefinition {name fieldName fieldType fieldLgth} {
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
#    return $::STRUCT_MSG_ERR_DEFINITION_NOT_FOUND;
  }
  if {[dict get $definition numFields] >= [dict get $definition maxFields]} {
    return $::STRUCT_MSG_ERR_DEFINITION_TOO_MANY_FIELDS
  }
#puts stderr "add_field: fieldName: $fieldName fieldtype: $fieldType fieldLgth: $fieldLgth!"
  set fieldInfos [dict get $definition fieldInfos]
  set result [structmsg_getFieldNameId $fieldName fieldNameId $::STRUCT_MSG_INCR]
  set result [structmsg_getFieldTypeId $fieldType fieldTypeId]
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




# ============================= structmsg_dumpFieldDefinition ========================

proc structmsg_dumpFieldDefinition {name} {
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
#    return $::STRUCT_MSG_ERR_DEFINITION_NOT_FOUND;
  }
  puts [format "definition: %s numFields: %d" $name [dict get $definition numFields]]
  set idx 0
  while {$idx < [dict get $definition numFields]} {
    set fieldInfo [lindex [dict get $definition fieldInfos] $idx]
    set result [structmsg_getIdFieldNameStr [dict get $fieldInfo fieldId] fieldIdStr]
#    checkErrOK $result
    set result [structmsg_getFieldTypeStr [dict get $fieldInfo fieldType] fieldTypeStr]
#    checkErrOK(result);
    puts [format "  idx: %d id: %d %s type: %d %s lgth: %d" $idx [dict get $fieldInfo fieldId] $fieldIdStr [dict get $fieldInfo fieldType] $fieldTypeStr [dict get $fieldInfo fieldLgth]]
    incr idx
  }
  return $::STRUCT_MSG_ERR_OK;
}

# ============================= structmsg_encodeFieldDefinitionMessage ========================

proc structmsg_encodeFieldDefinitionMessage {name dataVar lgthVar} {
  upvar $dataVar data
  upvar $lgthVar lgth

  # FIXME!!
  set src 123
  set dst 987

  return [structmsg_encodeDefinition $name data lgth]
}

# ============================= structmsg_decodeFieldDefinitionMessage ========================

proc structmsg_decodeFieldDefinitionMessage {data} {
  # FIXME!!
  set src 123
  set dst 987

  return [structmsg_decodeDefinition $data]
}

# ============================= structmsg_deleteStructmsgDefinition ========================

proc structmsg_deleteStructmsgDefinition {name} {
  return [structmsg_deleteDefinition $name]
}

# ============================= structmsg_deleteStructmsgDefinitions ========================

proc structmsg_deleteStructmsgDefinitions {} {
  # delete the whole structmsgDefinitions info, including fieldNameDefinitions info
  return [structmsg_deleteDefinitions
}

# ============================= structmsg_createMsgFromDefinition ========================

proc structmsg_createMsgFromDefinition {name} {
  set result [structmsg_getDefinitionDict $name definition]
#  checkErrOK(result);
puts stderr "DEF2: $definition!"
  set result [structmsg_create [expr {[dict get $definition numFields] - $::STRUCT_MSG_NUM_HEADER_FIELDS - $::STRUCT_MSG_NUM_CMD_HEADER_FIELDS}] handle]
puts stderr [format "create: handle: %s numFields: %d" $handle [expr {[dict get $definition numFields] - $::STRUCT_MSG_NUM_HEADER_FIELDS - $::STRUCT_MSG_NUM_CMD_HEADER_FIELDS}]]
#  checkErrOK(result);
  set fieldIdx 0
  while {$fieldIdx < [dict get $definition numFields]} {
    set fieldInfos [dict get $definition fieldInfos]
    set fieldInfo [lindex $fieldInfos $fieldIdx]
puts stderr "fieldInfo: $fieldInfo!$fieldIdx!"
    set fieldId [dict get $fieldInfo fieldId]
    set fieldName "??"
    if {[dict exists $::structmsg(specialFieldIds) $fieldId]} {
      set fieldName [dict get $::structmsg(specialFieldIds) $fieldId]
puts stderr "fieldName: $fieldName!"
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
        set result [structmsg_getIdFieldNameStr [dict get $fieldInfo fieldId] fieldName]
#      checkErrOK(result);
        set result [structmsg_getFieldTypeStr [dict get $fieldInfo fieldType] fieldType]
#      checkErrOK(result);
#puts stderr [format {addfield: %s %s %d} $fieldName $fieldType [dict get $fieldInfo fieldLgth]]
        set result [structmsg_add_field $handle $fieldName [dict get $fieldInfo fieldType] [dict get $fieldInfo fieldLgth]]
      }
    } 
    incr fieldIdx
  }
  return $::STRUCT_MSG_ERR_OK
}
