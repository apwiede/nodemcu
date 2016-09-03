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

if {0} {
source structmsgCmd.tcl
source structmsgDefinitions.tcl
source structmsgEncodeDecode.tcl
source structmsgApi.tcl

proc checkErrOK {result} {
  puts stderr "result: $result!"
}

::structmsg def initFieldTypeDefines
::structmsg def initSpecialFieldNames
}

set handle [structmsg create 10]
set result [structmsg add_field $handle "@randomNum" uint32_t 4]
checkErrOK $result
set result [structmsg add_field $handle "@sequenceNum" uint32_t 4]
checkErrOK $result
set result [structmsg add_field $handle "pwd1" uint8_t* 16]
checkErrOK $result
set result [structmsg add_field $handle "@tablerows" uint8_t 2]
checkErrOK $result
set result [structmsg add_field $handle "@tablerowfields" uint8_t 3]
checkErrOK $result
set result [structmsg add_field $handle "ssid1" "uint8_t*" 16]
checkErrOK $result
set result [structmsg add_field $handle "rssid1" "uint8_t*" 6]
checkErrOK $result
set result [structmsg add_field $handle "channel1" "uint8_t*" 6]
checkErrOK $result
puts stderr "SET VALUES"
set result [structmsg set_fieldValue $handle "@src" 123]
checkErrOK $result
set result [structmsg set_fieldValue $handle "@dst" 456]
checkErrOK $result
set result [structmsg set_fieldValue $handle "@cmdKey" 789]
checkErrOK $result
set result [structmsg set_fieldValue $handle "pwd1" "/dir1/dir2/dir34"]
checkErrOK $result
set result [structmsg set_tableFieldValue $handle "ssid1" 0 "xx1234567890abcd"]
checkErrOK $result
set result [structmsg set_tableFieldValue $handle "rssid1" 0 "yy1234"]
checkErrOK $result
set result [structmsg set_tableFieldValue $handle "channel1" 0 "z1234w"]
checkErrOK $result
set result [structmsg set_tableFieldValue $handle "ssid1" 1 "aa1234567890abcd"]
checkErrOK $result
set result [structmsg set_tableFieldValue $handle "rssid1" 1 "bb1234"]
checkErrOK $result
set result [structmsg set_tableFieldValue $handle "channel1" 1 "q1234r"]
puts stderr "SET VALUES END"
puts stderr "++ ENCODE1 $handle!"
structmsg dump $handle
set result [structmsg encode $handle]
checkErrOK $result
puts stderr "++ ENCODE1 $handle DONE!"
structmsg dump $handle
set encoded [structmsg get_encoded $handle]

if {1} {
puts stderr "++ ENCODE2 $handle!"
set result [structmsg encode $handle]
checkErrOK $result
puts stderr "++ ENCODE2 $handle DONE!"
set encryptedMsg [structmsg encrypt $handle "a1b2c3d4e5f6g7h8"]
set result [structmsg decrypt_getHandle $encryptedMsg "a1b2c3d4e5f6g7h8" "" handle1]
checkErrOK $result
if {$result == $::STRUCT_MSG_ERR_OK} {
puts stderr "found handle: $handle1!"
}
set decryptedMsg [structmsg decrypt $handle "a1b2c3d4e5f6g7h8" "" $encryptedMsg]
structmsg dump $handle
puts stderr "after Dump 2"
set result [structmsg decode $handle $encoded]
checkErrOK $result

structmsg encode $handle
set pwd1_encoded [structmsg get_encoded $handle]
structmsg decode $handle $encoded
set encryptedMsg [structmsg encrypt $handle "a1b2c3d4e5f6g7h8" ""]
set decryptedMsg [structmsg decrypt $handle "a1b2c3d4e5f6g7h8" "" $encryptedMsg]
structmsg dump $handle

#puts stderr "DICT:"
#pdict [set ::structmsg($handle)]
}
