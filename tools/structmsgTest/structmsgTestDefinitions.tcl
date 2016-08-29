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

source pdict.tcl
source structmsgCmd.tcl
source structmsgEncodeDecode.tcl
source structmsgDefinitions.tcl

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
set def "pwd2"
structmsg_createStructmsgDefinition "pwd2" 15
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
structmsg_encodeFieldDefinitionMessage $def data lgth
set def pwd1
structmsg_deleteDefinition $def
set def pwd2
structmsg_deleteDefinition $def
structmsg_decodeFieldDefinitionMessage $data
structmsg_dumpFieldDefinition $def
