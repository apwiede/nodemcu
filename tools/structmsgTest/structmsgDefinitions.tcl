#!/usr/bin/env tclsh8.6

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

source pdict.tcl

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
puts stderr "firstFreeEntry: $firstFreeEntry!"
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
  set structmsgDefinitionDict [dict create]
  dict set structmsgDefinitionDict maxFields $numFields
  dict set structmsgDefinitionDict numFields 0
  dict set structmsgDefinitionDict fieldInfos [list]
  incr ::structmsg(numStructmsgDefinitions)
  lappend ::structmsg(structmsgDefinitions) $name $structmsgDefinitionDict
if {0} {
  // check for unused slot!
  definitionIdx = 0;
  while (definitionIdx < structmsgDefinitions.maxDefinitions) {
    definition = &structmsgDefinitions.definitions[definitionIdx];
    if (definition->name == NULL) {
      definition->numFields = 0;
      definition->maxFields = numFields;
      definition->fieldInfos = (fieldInfoDefinition_t *)os_zalloc(numFields * sizeof(fieldInfoDefinition_t));
      checkAllocOK(definition->fieldInfos);
      if (definitionIdx >= structmsgDefinitions.numDefinitions) {
        structmsgDefinitions.numDefinitions++;
      }
      lgth = c_strlen(name);
      definition->name = os_malloc(lgth + 1);
      definition->name[lgth] = '\0';
      checkAllocOK(definition->name);
      c_memcpy(definition->name, name, lgth);
      return $::STRUCT_MSG_ERR_OK;
    }
    definitionIdx++;
  }
  return $::STRUCT_MSG_ERR_NO_SLOT_FOUND;
}
}

# ============================= structmsg_addFieldDefinition ========================

proc structmsg_addFieldDefinition {name fieldName fieldType fieldLgth} {
  set fieldDefinitionDict [dict create]
  set structmsgDefinitionsDict $::structmsg(structmsgDefinitions)
  if {![dict exists $::structmsg(structmsgDefinitions) $name]} {
    error "structmsg definition for $name does not exist"
#    return $::STRUCT_MSG_ERR_DEFINITION_NOT_FOUND;
  }
#  if (definition->numFields >= definition->maxFields) {
#    return $::STRUCT_MSG_ERR_DEFINITION_TOO_MANY_FIELDS;
#  }
puts stderr "add_field: fieldName: $fieldName fieldtype: $fieldType fieldLgth: $fieldLgth!"
  set fieldInfos [dict get $structmsgDefinitionsDict $name fieldInfos]
  set result [structmsg_getFieldNameId $fieldName fieldNameId $::STRUCT_MSG_INCR]
  set result [structmsg_getFieldTypeId $fieldType fieldTypeId]
  set fielNameDict [dict create]
  dict set fieldNameDict fieldId $fieldNameId
  dict set fieldNameDict fieldType $fieldTypeId
  dict set fieldNameDict fieldLgth $fieldLgth
  lappend fieldInfos $fieldNameDict
  dict set ::structmsg(structmsgDefinitions) $name numFields [expr {[dict get $::structmsg(structmsgDefinitions) $name numFields] + 1}]
  dict set ::structmsg(structmsgDefinitions) $name fieldInfos $fieldInfos
  return $::STRUCT_MSG_ERR_OK
}




# ============================= structmsg_dumpFieldDefinition ========================

proc structmsg_dumpFieldDefinition {name} {
  set structmsgDefinitionsDict $::structmsg(structmsgDefinitions)
  if {![dict exists $::structmsg(structmsgDefinitions) $name]} {
    error "structmsg definition for $name does not exist"
#    return $::STRUCT_MSG_ERR_DEFINITION_NOT_FOUND;
  }
  set definition [dict get $::structmsg(structmsgDefinitions) $name]
  puts [format "definition: %s numFields: %d" $name [dict get $definition numFields]]
pdict $definition
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

proc structmsg_decodeFieldDefinitionMessage {name data} {
  // FIXME!!
  uint16_t src = 123;
  uint16_t dst = 987;

  return [structmsg_decodeDefinition $name data]
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

