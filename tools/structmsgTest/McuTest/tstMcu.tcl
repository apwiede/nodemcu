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

dict set states START type I_TELEGRAM
dict set states START msgLgth 8
dict set states START handleProc ::sendITelegram
dict set states START next B_TELEGRAM

dict set states B_TELEGRAM type B_TELEGRAM
dict set states B_TELEGRAM msgLgth 10
dict set states B_TELEGRAM handleProc ::sendBTelegram
dict set states B_TELEGRAM next I_TELEGRAM

dict set states I_TELEGRAM type I_TELEGRAM
dict set states I_TELEGRAM msgLgth 8
dict set states I_TELEGRAM handleProc ::sendMTelegram
dict set states I_TELEGRAM next M_TELEGRAM

dict set states M_TELEGRAM type M_TELEGRAM
dict set states M_TELEGRAM msgLgth 8
dict set states M_TELEGRAM handleProc ::sendMTelegram
dict set states M_TELEGRAM next M_TELEGRAM

set telegrams [dict create]
dict set telegrams 22272 19712 I receivedLgth 8
dict set telegrams 22272 19712 I handleProc sendIAck
dict set telegrams 22272 19712 I sendLgth 42

dict set telegrams 22272 19712 B receivedLgth 10
dict set telegrams 22272 19712 B handleProc sendBAck
dict set telegrams 22272 19712 B sendLgth 9

dict set telegrams 22272 19712 M receivedLgth 8
dict set telegrams 22272 19712 M handleProc sendMAck
dict set telegrams 22272 19712 M sendLgth 44


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

set ::telegramLgth 99

proc init {} {
  set ::dev0 "/dev/ttyUSB0"
  set ::dev0Buf ""
  set ::dev0Lgth 0
  set ::dev1 "/dev/ttyUSB1"
  set ::dev1Buf ""
  set ::dev1Lgth 0


  set ::fd0 [open $::dev0 w+]
  fconfigure $::fd0 -blocking 0 -translation binary
  fconfigure $::fd0 -mode 115200,n,8,1
  fileevent $::fd0 readable [list readByte $::fd0 ::dev0Buf ::dev0Lgth]
}


proc readByte {fd bufVar lgthVar} {
  upvar $bufVar buf
  upvar $lgthVar lgth

  set ch [read $fd 1]
  binary scan $ch c pch
#puts stderr "ch: $ch!$pch!lgth: $lgth"
  append buf $ch
  incr lgth
  if {$lgth == 7} {
    binary scan $buf SSSc ::dst ::src myLgth ::cmdKey
    set ::cmdKey [format %c $::cmdKey]
puts stderr [format "dst: 0x%04x src: 0x%04x lgth: %d cmdKey: %s" $::dst $::src $myLgth $::cmdKey]
    if {![dict exists $::telegrams $::dst $::src $::cmdKey]} {
      error [format "funny telegram: dst: 0x%04x src: 0x%04x cmdKey: %s" $::dst $::src $::cmdKey]
    }
    set ::telegramDict [dict get $::telegrams $::dst $::src $::cmdKey]
    set expectedLgth [dict get $::telegramDict receivedLgth]
    if {$expectedLgth != $myLgth} {
      error "expected length: $expectedLgth got: $myLgth"
    } else {
      set ::telegramLgth $expectedLgth
    }
  }
#puts stderr "lgth: $lgth telegramLgth: $::telegramLgth!"
  if {$lgth >= $::telegramLgth} {
    set myBuf ""
    foreach ch [split $buf ""] {
      binary scan $ch c pch
      append myBuf " [format 0x%02x [expr {$pch & 0xFF}]]"
    }
    puts stderr "got telegram: for cmdKey: $::cmdKey: $myBuf"
    set telegram $buf
    set buf ""
    set lgth 0
    set ::telegramLgth 99
    set handleProc [dict get $::telegramDict handleProc]
    after 10 [list uplevel 0 [list $handleProc $fd $telegram $lgth]]
    flush $fd
  }
}

proc sendIAck {fd receivedTelegram lgth} {
  set ::Mcu "\x4D\x00"
  set ::Wifi "\x57\x00"
  set tLgth "\x00\x2A"
  set cmdKey "I"
  set GUID "1234-5678-9012-1"
  set passwd "apwiede1apwiede2"
  set reserve "XY"

  set telegram $::Mcu
  append telegram $::Wifi
  append telegram $tLgth
  append telegram $cmdKey
  append telegram $GUID
  append telegram $passwd
  append telegram $reserve
  set myLgth [string length $telegram]
  structmsg encdec crcEncode telegram $myLgth $myLgth crcVal 0
  set myLgth [string length $telegram]
puts stderr "lgth: $myLgth!"
  set expectedSendLgth [dict get $::telegramDict sendLgth]
  if {$expectedSendLgth != $myLgth} {
    error "send $cmdKey expected length: $expectedSendLgth got $myLgth"
  } 
puts stderr "sendIAck: "
  set myBuf ""
  foreach ch [split $telegram ""] {
    binary scan $ch c pch
    append myBuf " [format 0x%02x [expr {$pch & 0xFF}]]"
  }
puts stderr $myBuf
  puts -nonewline $fd $telegram
  flush $fd
}

proc sendBAck {fd receivedTelegram lgth} {
  set cmdKey "B"
  set telegram "\x4D\x00\x57\x00\x00\x09BA"
  set myLgth [string length $telegram]
  structmsg encdec crcEncode telegram $myLgth $myLgth crcVal 0
  set myLgth [string length $telegram]
  set expectedSendLgth [dict get $::telegramDict sendLgth]
  if {$expectedSendLgth != $myLgth} {
    error "send $cmdKey expected length: $expectedSendLgth got $myLgth"
  } 
puts stderr "sendBAck: "
  set myBuf ""
  foreach ch [split $telegram ""] {
    binary scan $ch c pch
    append myBuf " [format 0x%02x [expr {$pch & 0xFF}]]"
  }
puts stderr $myBuf
  puts -nonewline $fd $telegram
  flush $fd
}

proc sendMAck {fd telegram lgth} {
  set ::Mcu "\x4D\x00"
  set ::Wifi "\x57\x00"
  set tLgth "\x00\x2C"
  set cmdKey "M"

  set telegram $::Mcu
#puts stderr "after Mcu offset: [string length $telegram]!"
  append telegram $::Wifi
#puts stderr "Wifi offset: [string length $telegram]!"
  append telegram $tLgth
#puts stderr "tLgth offset: [string length $telegram]!"
  append telegram $cmdKey
#puts stderr "cmdKey offset: [string length $telegram]!"
  append telegram $::macAddr
#puts stderr "macAddr offset: [string length $telegram]!"
  append telegram $::ipAddr
#puts stderr "ipAddr offset: [string length $telegram]!"
  append telegram $::firmVersion
#puts stderr "firmVersion offset: [string length $telegram]!"
  append telegram $::serieNumber
#puts stderr "serieNumber offset: [string length $telegram]!"
  append telegram $::rssi
#puts stderr "rssi offset: [string length $telegram]!"
  append telegram $::modulConnection
#puts stderr "moduleConnection offset: [string length $telegram]!"
  append telegram $::deviceMode
#puts stderr "deviceMode offset: [string length $telegram]!"
  append telegram $::deviceSecurity
#puts stderr "deviceSecurity offset: [string length $telegram]!"
  append telegram $::errorMain
#puts stderr "errorMain offset: [string length $telegram]!"
  append telegram $::errorSub
#puts stderr "errorSub offset: [string length $telegram]!"
  append telegram $::dat
#puts stderr "date offset: [string length $telegram]!"
  append telegram $::numSsid
#puts stderr "numSsid offset: [string length $telegram]!"
  append telegram "DEF" ; # reserve
#puts stderr "reserve offset: [string length $telegram]!"
  set myLgth [string length $telegram]
  structmsg encdec crcEncode telegram $myLgth $myLgth crcVal 0
  set myLgth [string length $telegram]
#puts stderr "myLgth: $myLgth!"
  set expectedSendLgth [dict get $::telegramDict sendLgth]
  if {$expectedSendLgth != $myLgth} {
    error "send $cmdKey expected length: $expectedSendLgth got $myLgth"
  } 
puts stderr "sendIAck: "
  set myBuf ""
  foreach ch [split $telegram ""] {
    binary scan $ch c pch
    append myBuf " [format 0x%02x [expr {$pch & 0xFF}]]"
  }
puts stderr $myBuf
  puts -nonewline $fd $telegram
  flush $fd
}

init

vwait forever
