#!/usr/bin/env tclsh8.6

source structmsgCmd.tcl

set offset 0
set handle [create_structmsg 5]
set_targets $handle 123 456 789
addField $handle "@randomNum" uint32_t 4
addField $handle "pwd" uint8_t* 16
set_fieldValue $handle "pwd" "/dir1/dir2/dir34"
set_fillerAndCrc $handle
#dump_structmsg $handle
encodeMsg $handle
dump_structmsg $handle
#puts stderr "DICT:"
#pdict [set ::structmsg($handle)]
set encoded [getEncoded $handle]
puts stderr "encoded: $encoded!"
#set encoded [string replace $encoded 0 0 789]
#set encoded [string replace $encoded 1 1 123]
decodeMsg $handle $encoded
dump_structmsg $handle
#puts stderr "DICT:"
#pdict [set ::structmsg($handle)]

