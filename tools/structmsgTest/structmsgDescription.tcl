source pdict.tcl

set ::structmsg(numStructmsgDefinitions) 0
set ::structmsg(structmsgDefinitions) [dict create]
set ::structmsg(fieldTypeDefines) [list]
set ::structmsg(fieldNameDefines) [list]
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

proc structmsg_initFieldNameDefines {} {
   set definesDict [dict create]
   dict set filedNameDefinesDict "@src"         -1
   dict set fieldNameDefinesDict "@dst"         -2
   dict set fieldNameDefinesDict "@targetCmd"   -3
   dict set fieldNameDefinesDict "@totalLgth"   -4
   dict set fieldNameDefinesDict "@cmdKey"      -5
   dict set fieldNameDefinesDict "@cmdLgth"     -6
   dict set fieldNameDefinesDict "@randomNum"   -7
   dict set fieldNameDefinesDict "@sequenceNum" -8
   dict set fieldNameDefinesDict "@filler"      -9
   dict set fieldNameDefinesDict "@crc"         -10
   dict set fieldNameDefinesDict "@id"          -11
   set ::structmsg(fieldNameDefines) $fieldNameDefinesDict
}

# ============================= structmsg_createStructmsgDefinition ========================

proc structmsg_createStructmsgDefinition {name numFields} {
    set structmsgDefinitionDict [dict create]
    dict set structmsgDefinitionDict numFields $numFields
    dict set structmsgDefinitionDict fieldInfos [list]
    incr ::structmsg(numStructmsgDefinitions)
    lappend ::structmsg(structmsgDefinitions) $name $structmsgDefinitionDict
}

# ============================= structmsg_getFieldNameId ========================

proc structmsg_getFieldNameId {fieldName} {
    set fieldNameDefinesDict $::structmsg(fieldNameDefines)
    switch -glob -- $fieldName {
      "@*" {
        if {![dict exists $fieldNameDefinesDict $fieldName]} {
          error "bad special field name: $fieldName!"
        } else {
          set fieldNameId [dict get $fieldNameDefinesDict $fieldName]
        }
      }
      default {
        if {![dict exists $fieldNameDefinesDict $fieldName]} {
          incr ::structmsg(numFieldNameIds)
          set fieldNameId  $::structmsg(numFieldNameIds)
          dict set fieldNameDefinesDict $fieldName $fieldNameId
        } else {
          set fieldNameId [dict get $fieldNameDefinesDict $fieldName]
        }
      }
    }
    set ::structmsg(fieldNameDefines) $fieldNameDefinesDict 
    return $fieldNameId
}

# ============================= structmsg_getFieldTypeId ========================

proc structmsg_getFieldTypeId {fieldType} {
    set fieldTypeDefinesDict $::structmsg(fieldTypeDefines)
    if {![dict exists $fieldTypeDefinesDict $fieldType]} {
        error "bad field type: $fieldType!"
    } else {
        set fieldTypeId [dict get $fieldTypeDefinesDict $fieldType]
    }
    return $fieldTypeId
}

# ============================= structmsg_addFieldDefinition ========================

proc structmsg_addFieldDefinition {name fieldName fieldType fieldLgth} {
    set fieldDefinitionDict [dict create]
    set structmsgDefinitionsDict $::structmsg(structmsgDefinitions)
    if {![dict exists $::structmsg(structmsgDefinitions) $name]} {
      error "structmsg definition for $name does not exist"
    } else {
      set fieldInfos [dict get $structmsgDefinitionsDict $name fieldInfos]
      set fieldNameId [structmsg_getFieldNameId $fieldName]
      set fieldTypeId [structmsg_getFieldTypeId $fieldType]
      set fielNameDict [dict create]
      dict set fieldNameDict nameId $fieldNameId
      dict set fieldNameDict typeId $fieldTypeId
      dict set fieldNameDict length $fieldLgth
      lappend fieldInfos $fieldNameDict
      dict set ::structmsg(structmsgDefinitions) $name fieldInfos $fieldInfos
    }
}

structmsg_initFieldTypeDefines
structmsg_initFieldNameDefines
structmsg_createStructmsgDefinition "pwd" 5
structmsg_addFieldDefinition "pwd" @randomNum uint32_t 4
structmsg_addFieldDefinition "pwd" @sequenceNum uint32_t 4
structmsg_addFieldDefinition "pwd" pwd uint8_t* 16
structmsg_addFieldDefinition "pwd" @filler uint8_t* -1
structmsg_addFieldDefinition "pwd" @crc uint16_t -1
pdict $::structmsg(structmsgDefinitions)
puts stderr "names:"
pdict $::structmsg(fieldNameDefines)
puts stderr "types:"
pdict $::structmsg(fieldTypeDefines)
structmsg_createStructmsgDefinition "pwd2" 5
structmsg_addFieldDefinition "pwd2" @randomNum uint32_t 4
structmsg_addFieldDefinition "pwd2" @sequenceNum uint32_t 4
structmsg_addFieldDefinition "pwd2" pwd uint8_t* 16
structmsg_addFieldDefinition "pwd2" @filler uint8_t* -1
structmsg_addFieldDefinition "pwd2" @crc uint16_t -1
puts stderr "Definitions"
pdict $::structmsg(structmsgDefinitions)
