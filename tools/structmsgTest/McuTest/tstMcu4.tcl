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

source ./structmsgEncodeDecode.tcl
set ::crcDebug false

set currState START
set states [dict create]

set telegrams [dict create]
dict set telegrams 22272 19712 I receivedLgth 8
dict set telegrams 22272 19712 I handleProc sendIAck
dict set telegrams 22272 19712 I sendLgth 42

dict set telegrams 19712 22272 I receivedLgth 42
dict set telegrams 19712 22272 I handleProc sendMMsg
dict set telegrams 19712 22272 I sendLgth 8

dict set telegrams 22272 19712 B receivedLgth 10
dict set telegrams 22272 19712 B handleProc sendBAck
dict set telegrams 22272 19712 B sendLgth 9

dict set telegrams 19712 22272 B receivedLgth 9
dict set telegrams 19712 22272 B handleProc sendMMsg
dict set telegrams 19712 22272 B sendLgth 8

dict set telegrams 22272 19712 M receivedLgth 8
dict set telegrams 22272 19712 M handleProc sendMAck
dict set telegrams 22272 19712 M sendLgth 44

dict set telegrams 19712 22272 M receivedLgth 44
dict set telegrams 19712 22272 M handleProc sendBMsg
dict set telegrams 19712 22272 M sendLgth 10

set ::debugBuf ""
set ::debugTxt ""
set ::startBuf ""
set ::startTxt ""
set ::isStart true
#set ::isStart false

set ::macAddr "\xAB\xCD\xEF\x12\x34\x56"
set ::ipAddr "\xD4\xC3\x12\x34"
set ::firmVersion "\x12\x34\x56\xAB\xCD\xEF"
set ::serieNumber "\x02\x13\x2A\x10"
set ::rssi "\x8A"
set ::modulConnection "\x41"
set ::deviceMode "\x34"
set ::deviceSecurity "\x00"
set ::errorMain "\x00"
set ::errorSub "\x00"
set ::dat "\x00\x00\x00\x00\x00\x00"
set ::numSsid "\x2"
set ::inDebug false
set ::lastch ""

set ::telegramLgth 99

proc init0 {} {
  set ::dev0 "/dev/ttyUSB0"
  set ::dev0Buf ""
  set ::dev0Lgth 0


  set ::fd0 [open $::dev0 w+]
  fconfigure $::fd0 -blocking 0 -translation binary
  fconfigure $::fd0 -mode 115200,n,8,1
#  fileevent $::fd0 readable [list readByte0 $::fd0 ::dev0Buf ::dev0Lgth]
}

proc init1 {} {
  set ::dev1 "/dev/ttyUSB1"
  set ::dev1Buf ""
  set ::dev1Lgth 0


  set ::fd1 [open $::dev1 w+]
  fconfigure $::fd1 -blocking 0 -translation binary
  fconfigure $::fd1 -mode 115200,n,8,1
  fileevent $::fd1 readable [list readByte1 $::fd1 ::dev1Buf ::dev1Lgth]
}


set cnt 0
proc readByte1 {fd bufVar lgthVar} {
  upvar $bufVar buf
  upvar $lgthVar lgth

  set ch [read $fd 1]
  incr ::cnt
if {$::cnt == 500} {
#close $fd
#init1
}
  set pch 0
  binary scan $ch c pch
puts stderr "isStart rch: $ch![format 0x%02x [expr {$pch & 0xFF}]]!$lgth!$::cnt!"
}

init1

vwait forever
