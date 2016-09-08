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

set PORT 8080
set path "/getapdeflist"
set clients [list]
set ::cryptkey "a1b2c3d4e5f6g7h8"

set srvSock [socket -server handleConnect $PORT]
::websocket::server $::srvSock
::websocket::live $::srvSock $::path wsLiveCB

proc decryptMsg {payload} {
  set offset 0
  structmsg_create 5 handle
  structmsg_set_fieldValue $handle "@src" 123
  structmsg_set_fieldValue $handle "@dst" 456
  structmsg_set_fieldValue $handle "@cmdKey" 789
  structmsg_add_field $handle "@randomNum" uint32_t 4
  structmsg_add_field $handle "pwd" uint8_t* 16
  structmsg_set_fillerAndCrc $handle
  set decryptedMsg [structmsg_decrypt $handle $::cryptkey $payload]
puts stderr "decryptMsg2 len: [string length $decryptedMsg]!len2: [string length $payload]!"
#  structmsg_dump $handle
  structmsg_decode $handle $decryptedMsg
  structmsg_dump $handle
#  puts stderr "DICT:"
#  pdict [set ::structmsg($handle)]
  structmsg_get_fieldValue $handle pwd pwd
puts stderr "PWD: $pwd!"
  structmsg_get_fieldValue $handle @randomNum value
puts stderr "randomNum: [format 0x%08x $value]!"
  structmsg_get_fieldValue $handle @filler value
puts stderr "filler: $value!"
  structmsg_get_fieldValue $handle @crc value
puts stderr "crc: [format 0x%04x $value]!"
  structmsg_get_fieldValue $handle @src value
puts stderr "src: [format {%d 0x%02x} $value $value]!"
  structmsg_get_fieldValue $handle @dst value
puts stderr "dst: [format {%d 0x%02x} $value $value]!"
  structmsg_get_fieldValue $handle @totalLgth value
puts stderr "totalLgth: [format {%d 0x%02x} $value $value]!"
  structmsg_get_fieldValue $handle @cmdKey value
puts stderr "cmdKey: [format {%d 0x%02x} $value $value]!"
}

proc sendToClient { str } {
    foreach cs $::clients {
        ::websocket::send $cs 1 "$str\n" 1
    }
}

proc handleConnect {cs addr p} {
puts stderr "handleConnect: cs: $cs addr: $addr p: $p!"
    if {[lsearch $::clients $cs] < 0} {
        lappend ::clients $cs
    }
    fileevent $cs readable [list handleRead $cs]
}
    
proc handleRead {cs} {
puts stderr "handleRead"
    set key [read $cs 1]
if {$key eq ""} {
    return
}
puts stderr "key: $key!"
    gets $cs request
set request ${key}$request
puts stderr "request: $request!"
    set path [lindex [split $request] 1]
    set hdrs [list]
    while { [gets $cs hdr] > 0 } {
        foreach { k v } $hdr break
        set k [string trim $k ":"]
        lappend hdrs $k $v
    }
puts stderr "hdrs: $hdrs!"
    set res [::websocket::test $::srvSock $cs $path $hdrs]
puts stderr "test: res: $res!"
    if {$res} {
        ::websocket::upgrade $cs
    }
puts stderr "path: $path!"
    return 
}

set ::msgCnt 0
proc wsLiveCB {cs op par args} {
puts stderr "wsLiveCB op: $op!"
    switch $op { 
      request {
puts stderr "op: $op! cs: $cs! par: $par! args: $args!"
      }
      close {
puts stderr "op: $op! cs: $cs! par: $par! args: $args!"
          set idx [lsearch $::clients $cs]
          if { $idx >= 0 } {
              set ::clients [lreplace $::clients $idx $idx]
          }
      }
      disconnect {
puts stderr "op: $op! cs: $cs! par: $par! args: $args!"
      }
      text {
puts stderr "wsLiveCB: $op $par"
        incr ::msgCnt
        if {$par != "Message Received"} {
          if {$::msgCnt == 1} {
            set str "GOT message: $par!"
            ::websocket::send $cs 1 "$str\n" 1
          } else {
if {0} {
            set msg "Hello World 1234"
            set encrypted [aes::aes -mode cbc -dir encrypt -key $::cryptkey -iv $::cryptkey $msg]
            set idx 0
            foreach val [split $encrypted ""] {
              binary scan $val c pch
              puts stderr "idx: $idx [format 0x%02x [expr {$pch & 0xff}]]!"
              incr idx
            }
}
            set msg ""
            set idx 0
            foreach val [split $par ","] {
puts stderr "sp idx: $idx [format 0x%02x [expr {$val & 0xFF}]]!"
              set ch [binary format c $val]
              append msg $ch
            }
            set idx 0
            foreach val [split $msg ""] {
              binary scan $val c pch
              puts stderr "idx: $idx [format 0x%02x $pch]"
              incr idx
            }
            set decrypted [aes::aes -mode cbc -dir decrypt -key $::cryptkey -iv $::cryptkey $msg]
puts stderr "decrypted: $decrypted![string length $decrypted]!"
            

          }

        }
      } 
      binary {
puts stderr "op: $op! cs: $cs!"
puts stderr "par: $par!"
puts stderr "args: $args!"
set idx 0
foreach ch [split $par ""] {
  binary scan $ch c pch
  set pch [expr {$pch & 0xFF}]
#puts stderr [format "idx: $idx pch: 0x%02x!" $pch]
  incr idx
}
        decryptMsg $par
      }
    } 
}

vwait forever
