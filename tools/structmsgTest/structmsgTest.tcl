#!/usr/bin/env tclsh8.6

source structmsgCmd.tcl

set offset 0
set handle [structmsg_create 5]
structmsg_set_fieldValue $handle "@src" "234"
structmsg_set_fieldValue $handle "@dst" "567"
structmsg_set_fieldValue $handle "@cmdKey" "890"
structmsg_add_field $handle "@randomNum" uint32_t 4
structmsg_add_field $handle "@sequenceNum" uint32_t 4
structmsg_add_field $handle "pwd" uint8_t* 16
structmsg_set_fieldValue $handle "pwd" "/dir1/dir2/dir34"
structmsg_set_fillerAndCrc $handle
structmsg_encode $handle
set encoded [structmsg_get_encoded $handle]
structmsg_decode $handle $encoded
set encryptedMsg [structmsg_encrypt $handle "a1b2c3d4e5f6g7h8"]
structmsg_decrypt_getHandle $encryptedMsg "a1b2c3d4e5f6g7h8" handle1
puts stderr "found handle: $handle1!"
set decryptedMsg [structmsg_decrypt $handle "a1b2c3d4e5f6g7h8" $encryptedMsg]
structmsg_dump $handle

structmsg_encode $handle
set encoded [structmsg_get_encoded $handle]
structmsg_decode $handle $encoded
set encryptedMsg [structmsg_encrypt $handle "a1b2c3d4e5f6g7h8"]
set decryptedMsg [structmsg_decrypt $handle "a1b2c3d4e5f6g7h8" $encryptedMsg]
structmsg_dump $handle

#puts stderr "DICT:"
#pdict [set ::structmsg($handle)]
