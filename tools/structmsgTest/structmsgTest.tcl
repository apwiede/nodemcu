#!/usr/bin/env tclsh8.6

source structmsgCmd.tcl

set offset 0
set handle [create_structmsg 5]
set_targets $handle 123 456 789
addField $handle "@randomNum" uint32_t 4
addField $handle "pwd" uint8_t* 16
set_fieldValue $handle "pwd" "/dir1/dir2/dir34"
set_fillerAndCrc $handle
encode_msg $handle
dump_structmsg $handle
set encoded [get_encoded $handle]
decode_msg $handle $encoded
dump_structmsg $handle
set encryptedMsg [encrypt_payload $handle "a1b2c3d4e5f6g7h8"]
dump_structmsg $handle
set decryptedMsg [decrypt_payload $handle "a1b2c3d4e5f6g7h8" $encryptedMsg]
dump_structmsg $handle
#puts stderr "DICT:"
#pdict [set ::structmsg($handle)]
