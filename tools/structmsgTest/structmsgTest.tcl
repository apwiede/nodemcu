#!/usr/bin/env tclsh8.6

source structmsgCmd.tcl

set offset 0
set handle [create_structmsg 5]
#set_targets $handle 123 456 789
set_fieldValue $handle "@src" "234"
set_fieldValue $handle "@dst" "567"
set_fieldValue $handle "@cmdKey" "890"
addField $handle "@randomNum" uint32_t 4
addField $handle "@sequenceNum" uint32_t 4
addField $handle "pwd" uint8_t* 16
set_fieldValue $handle "pwd" "/dir1/dir2/dir34"
set_fillerAndCrc $handle
encode_msg $handle
set encoded [get_encoded $handle]
decode_msg $handle $encoded
set encryptedMsg [encrypt_payload $handle "a1b2c3d4e5f6g7h8"]
set decryptedMsg [decrypt_payload $handle "a1b2c3d4e5f6g7h8" $encryptedMsg]
dump_structmsg $handle

encode_msg $handle
set encoded [get_encoded $handle]
decode_msg $handle $encoded
set encryptedMsg [encrypt_payload $handle "a1b2c3d4e5f6g7h8"]
set decryptedMsg [decrypt_payload $handle "a1b2c3d4e5f6g7h8" $encryptedMsg]
dump_structmsg $handle

#puts stderr "DICT:"
#pdict [set ::structmsg($handle)]
