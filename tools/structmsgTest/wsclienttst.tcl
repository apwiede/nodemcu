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

package require websocket
#package require tls
#tls::init -tls1 1 ;# forcibly activate support for the TLS1 protocol

source structmsgCmd.tcl

::websocket::loglevel debug

set PORT 80
set path "/getaplist"
#set host "192.168.178.42"
set host "192.168.4.1"

proc clientHandler { sock type msg } {
puts stderr "clientHandler: $type $msg!"
    switch -glob -nocase -- $type {
	co* {
	    puts "Connected on $sock"
	}
	te* {
	    puts "RECEIVED: $msg"
	}
	cl* -
	dis* {
	}
        binary {
            foreach val [split $msg "\0"] {
               puts stderr "val: $val!"
            }
        }
    }
    
}

proc encryptMsg {pwd} {
return "/getaplist"
  set offset 0
  structmsg cmd create 5 handle
  structmsg cmd set_fieldValue $handle "@src" 123
  structmsg cmd set_fieldValue $handle "@dst" 456
  structmsg cmd set_fieldValue $handle "@cmdKey" 789
  structmsg cmd add_field $handle "@randomNum" uint32_t 4
  structmsg cmd add_field $handle "pwd" uint8_t* 16
  structmsg cmd set_fieldValue $handle "pwd" $pwd
  structmsg cmd set_fillerAndCrc $handle
  structmsg cmd encode $handle
  structmsg cmd get_encoded $handle data lgth
puts stderr "enc: $lgth!$data!"
  set encryptedMsg [structmsg cmd encrypt $handle "a1b2c3d4e5f6g7h8"]

#  structmsg_dump $handle
#  structmsg_dump $handle
#  puts stderr "DICT:"
#  pdict [set ::structmsg($handle)]
  return $encryptedMsg
}

proc test { sock } {
    after 2000
    puts stderr "[::websocket::conninfo $sock type] from [::websocket::conninfo $sock sockname] to [::websocket::conninfo $sock peername]"
puts stderr "===snd: binary: encrypted pwdMsg!"
    set encryptedMsg [encryptMsg "/dir1/dir2/dir34"]
puts stderr "encryptedMsg: $encryptedMsg!lgth: [string length $encryptedMsg]!"
set dbgBuf ""
foreach ch [split $encryptedMsg ""] {
  binary scan $ch c pch
  append dbgBuf [format " 0x%02x" [expr {$pch & 0xFF}]]
}
puts stderr "dbgBuf!$dbgBuf!"
#    ::websocket::send $sock binary $encryptedMsg
    ::websocket::send $sock text /getaplist
#    ::websocket::send $sock text "load:McuMsg.lua"
}

set clientSocket [::websocket::open "ws://${host}:${PORT}${path}" ::clientHandler] 
puts stderr "===clientSocket: $clientSocket"

after 2000 test $::clientSocket

vwait forever
