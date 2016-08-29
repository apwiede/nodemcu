#!/usr/bin/env tclsh8.6

puts stderr "START"
source pdict.tcl
source structmsgCmd.tcl
source structmsgEncodeDecode.tcl
source structmsgDescription.tcl

structmsg_initFieldTypeDefines
structmsg_initSpecialFieldNames
set def "pwd1"
structmsg_createStructmsgDefinition $def 15
structmsg_addFieldDefinition $def "@src" "uint16_t" 2
structmsg_addFieldDefinition $def "@dst" "uint16_t" 2
structmsg_addFieldDefinition $def "@totalLgth" "uint16_t" 2
structmsg_addFieldDefinition $def "@cmdKey" "uint16_t" 2
structmsg_addFieldDefinition $def "@cmdLgth" "uint16_t" 2
structmsg_addFieldDefinition $def "@randomNum" "uint32_t" 4
structmsg_addFieldDefinition $def "@sequenceNum" "uint32_t" 4
structmsg_addFieldDefinition $def "pwd1" "uint8_t*" 16
structmsg_addFieldDefinition $def "@tablerows" "uint8_t" 1
structmsg_addFieldDefinition $def "@tablerowfields" "uint8_t" 1
structmsg_addFieldDefinition $def "ssid1" "uint8_t*" 16
structmsg_addFieldDefinition $def "rssid1" "uint8_t*" 6
structmsg_addFieldDefinition $def "channel1" "uint8_t*" 6
structmsg_addFieldDefinition $def "@filler" "uint8_t*" 1
structmsg_addFieldDefinition $def "@crc" "uint16_t" 2
puts stderr "structmsgDefinitions:"
pdict $::structmsg(structmsgDefinitions)
puts stderr "names:"
pdict $::structmsg(fieldNameDefinitions)
set def "pwd2"
structmsg_createStructmsgDefinition "pwd2" 5
structmsg_addFieldDefinition $def "@src" "uint16_t" 2
structmsg_addFieldDefinition $def "@dst" "uint16_t" 2
structmsg_addFieldDefinition $def "@totalLgth" "uint16_t" 2
structmsg_addFieldDefinition $def "@cmdKey" "uint16_t" 2
structmsg_addFieldDefinition $def "@cmdLgth" "uint16_t" 2
structmsg_addFieldDefinition $def "@randomNum" "uint32_t" 4
structmsg_addFieldDefinition $def "@sequenceNum" "uint32_t" 4
structmsg_addFieldDefinition $def "pwd2" "uint8_t*" 16
structmsg_addFieldDefinition $def "@tablerows" "uint8_t" 1
structmsg_addFieldDefinition $def "@tablerowfields" "uint8_t" 1
structmsg_addFieldDefinition $def "ssid1" "uint8_t*" 16
structmsg_addFieldDefinition $def "rssid1" "uint8_t*" 6
structmsg_addFieldDefinition $def "channel1" "uint8_t*" 6
structmsg_addFieldDefinition $def "@filler" "uint8_t*" 1
structmsg_addFieldDefinition $def "@crc" "uint16_t" 2
structmsg_dumpFieldDefinition $def
puts stderr "Definitions"
pdict $::structmsg(structmsgDefinitions)
structmsg_encodeFieldDefinitionMessage $def data lgth
