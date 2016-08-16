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
encode_msg $handle
dump_structmsg $handle
#puts stderr "DICT:"
#pdict [set ::structmsg($handle)]
set encoded [get_encoded $handle]
puts stderr "encoded: $encoded!"
#set encoded [string replace $encoded 0 0 789]
#set encoded [string replace $encoded 1 1 123]
decode_msg $handle $encoded
dump_structmsg $handle
#puts stderr "DICT:"
#pdict [set ::structmsg($handle)]
set encryptedMsg [encrypt_payload $handle "a1b2c3d4e5f6g7h8"]
puts stderr "encryptedMsg: $encryptedMsg!"
dump_structmsg $handle
set decryptedMsg [decrypt_payload $handle "a1b2c3d4e5f6g7h8" $encryptedMsg]
puts stderr "decryptedMsg: $decryptedMsg!"
dump_structmsg $handle
