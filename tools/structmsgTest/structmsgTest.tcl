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

source structmsgCmd.tcl

set offset 0
structmsg_create 5 handle
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
