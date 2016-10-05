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
#set ::isStart true
set ::isStart false

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

set ::telegramLgth 99

proc init0 {} {
  set ::dev0 "/dev/ttyUSB0"
  set ::dev0Buf ""
  set ::dev0Lgth 0


  set ::fd0 [open $::dev0 w+]
  fconfigure $::fd0 -blocking 0 -translation binary
  fconfigure $::fd0 -mode 115200,n,8,1
  fileevent $::fd0 readable [list readByte0 $::fd0 ::dev0Buf ::dev0Lgth]
}

proc init1 {} {
  set ::dev1 "/dev/ttyUSB1"
  set ::dev1Buf ""
  set ::dev1Lgth 0


  set ::fd1 [open $::dev1 w+]
  fconfigure $::fd1 -blocking 0 -translation binary
  fconfigure $::fd1 -mode 115200,n,8,1
#  fileevent $::fd1 readable [list readByte1 $::fd1 ::dev1Buf ::dev1Lgth]
}


proc readByte0 {fd bufVar lgthVar} {
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
#puts stderr [format "0: dst: 0x%04x src: 0x%04x lgth: %d cmdKey: %s" $::dst $::src $myLgth $::cmdKey]
    if {![dict exists $::telegrams $::dst $::src $::cmdKey]} {
      error [format "0: funny telegram: dst: 0x%04x src: 0x%04x cmdKey: %s" $::dst $::src $::cmdKey]
    }
    set ::telegramDict [dict get $::telegrams $::dst $::src $::cmdKey]
    set expectedLgth [dict get $::telegramDict receivedLgth]
    if {$expectedLgth != $myLgth} {
      error "0: expected length: $expectedLgth got: $myLgth"
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
    if {[fileevent $::fd1 readable] eq ""} {
      fileevent $::fd1 readable [list readByte1 $::fd1 ::dev1Buf ::dev1Lgth]
    }
puts -nonewline $::fd1 $buf
flush $::fd1
    puts stderr "0: got telegram: for cmdKey: $::cmdKey: $myBuf"
#    set telegram $buf
    set buf ""
    set lgth 0
    set ::telegramLgth 99
fileevent $::fd0 readable [list]
#    set handleProc [dict get $::telegramDict handleProc]
#    after 10 [list uplevel 0 [list $handleProc $fd $telegram $lgth]]
#    flush $fd
  }
}

proc readByte1 {fd bufVar lgthVar} {
  upvar $bufVar buf
  upvar $lgthVar lgth

  set ch [read $fd 1]
#puts stderr "RB@: $ch"
  set pch 0
  binary scan $ch c pch
  if {$::isStart} {
puts stderr "isStart rch: $ch![format 0x%02x [expr {$pch& 0xFF}]]!"
    if {$ch eq ">"} {
      set::isStart false
      return
    }
    if {($ch == "\x57") || ($ch == "§")} {
      set ::isStart false
    } else {
      return
    }
  }
  if {[format 0x%02x [expr {$pch & 0xff}]] eq "0xc2"} {
    return
  }
  if {$ch == "§"} {
#puts stderr "inDebug rch: $ch![format 0x%02x [expr {$pch& 0xFF}]]!inDebug: $::inDebug!"
    if {$::inDebug} {
      set ::inDebug false
#      puts stderr "DBG: $::debugBuf!"
      puts stderr "DBT: $::debugTxt!"
      set ::debugBuf ""
      set ::debugTxt ""
      return
    } else {
      set ::inDebug true
      set ::debugBuf ""
      set ::debugTxt ""
      return
    }
  }
  if {$::inDebug} {
#puts stderr "inDebug2 rch: $ch![format 0x%02x [expr {$pch& 0xFF}]]!inDebug: $::inDebug!"
    append ::debugBuf " [format 0x%02x [expr {$pch & 0xff}]]"
    append ::debugTxt " $ch"
#puts stderr "IND: $::debugBuf!"
    return
  }
#puts stderr "rch: $ch![format 0x%02x [expr {$pch& 0xFF}]]!"
#puts -nonewline $::fd0 $ch
#flush $::fd0
  append buf $ch
  incr lgth
  if {$lgth == 7} {
    binary scan $buf SSSc ::dst ::src myLgth ::cmdKey
    set ::cmdKey [format %c $::cmdKey]
#puts stderr [format "1: dst: 0x%04x src: 0x%04x lgth: %d cmdKey: %s" $::dst $::src $myLgth $::cmdKey]
#puts stderr "1: 2: dst: $::dst src: $::src lgth: $myLgth cmdKey: $::cmdKey"
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
    puts stderr "1: got telegram: for cmdKey: $::cmdKey: $myBuf"
#sendIAck $::fd0 "" 0
puts -nonewline $::fd0 $buf
flush $::fd0
fileevent $::fd0 readable [list readByte0 $::fd0 ::dev0Buf ::dev0Lgth]
    set telegram $buf
    set buf ""
    set lgth 0
    set ::telegramLgth 99
fileevent $::fd1 readable [list]
#    set handleProc [dict get $::telegramDict handleProc]
#    after 10 [list uplevel 0 [list $handleProc $fd $telegram $lgth]]
#    flush $fd
  }
}

proc sendIMsg {fd telegram lgth} {
  set ::Mcu "\x4D\x00"
  set ::Wifi "\x57\x00"
  set tLgth "\x00\x08"
  set cmdKey "I"

set ::dbgBuf ""
set ::dbgTxt ""
set ::inDebug false
  set telegram $::Wifi
  append telegram $::Mcu
  append telegram $tLgth
  append telegram $cmdKey
  set myLgth [string length $telegram]
  structmsg encdec crcEncode telegram $myLgth $myLgth crcVal 0
  set myLgth [string length $telegram]
puts stderr "lgth: $myLgth!"
#  set expectedSendLgth [dict get $::telegramDict sendLgth]
#  if {$expectedSendLgth != $myLgth} {
#    error "send $cmdKey expected length: $expectedSendLgth got $myLgth"
#  } 
puts stderr "sendIMsg: "
  set myBuf ""
  foreach ch [split $telegram ""] {
    binary scan $ch c pch
    append myBuf " [format 0x%02x [expr {$pch & 0xFF}]]"
  }
puts stderr $myBuf
  puts -nonewline $fd $telegram
  flush $fd
}

proc sendMMsg {fd telegram lgth} {
  set ::Mcu "\x4D\x00"
  set ::Wifi "\x57\x00"
  set tLgth "\x00\x08"
  set cmdKey "M"

  set telegram $::Wifi
  append telegram $::Mcu
  append telegram $tLgth
  append telegram $cmdKey
  set myLgth [string length $telegram]
  structmsg encdec crcEncode telegram $myLgth $myLgth crcVal 0
  set myLgth [string length $telegram]
puts stderr "lgth: $myLgth!"
#  set expectedSendLgth [dict get $::telegramDict sendLgth]
#  if {$expectedSendLgth != $myLgth} {
#    error "send $cmdKey expected length: $expectedSendLgth got $myLgth"
#  } 
puts stderr "sendMMsg: "
  set myBuf ""
  foreach ch [split $telegram ""] {
    binary scan $ch c pch
    append myBuf " [format 0x%02x [expr {$pch & 0xFF}]]"
  }
puts stderr "$myBuf!$telegram!"
  puts -nonewline $fd $telegram
  flush $fd
}

proc sendBMsg {fd telegram lgth} {
  set ::Mcu "\x4D\x00"
  set ::Wifi "\x57\x00"
  set tLgth "\x00\x0a"
  set cmdKey "B"
  set opMode "\xE0"
  set reserve 80

  set telegram $::Wifi
  append telegram $::Mcu
  append telegram $tLgth
  append telegram $cmdKey
  append telegram $opMode
  append telegram $reserve
  set myLgth [string length $telegram]
  structmsg encdec crcEncode telegram $myLgth $myLgth crcVal 0
  set myLgth [string length $telegram]
puts stderr "lgth: $myLgth!"
#  set expectedSendLgth [dict get $::telegramDict sendLgth]
#  if {$expectedSendLgth != $myLgth} {
#    error "send $cmdKey expected length: $expectedSendLgth got $myLgth"
#  } 
puts stderr "sendBMsg: "
  set myBuf ""
  foreach ch [split $telegram ""] {
    binary scan $ch c pch
    append myBuf " [format 0x%02x [expr {$pch & 0xFF}]]"
  }
puts stderr "$myBuf!$telegram!"
  puts -nonewline $fd $telegram
  flush $fd
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
puts stderr "lgth: $myLgth!cmdKey: $cmdKey!"
  set myTelegramDict [dict get $::telegrams 22272 19712 $cmdKey]
  set expectedSendLgth [dict get $myTelegramDict sendLgth]
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
puts stderr "got: $myBuf!"
  puts -nonewline $fd $telegram
  flush $fd
}

init0
init1
#sendIMsg $::fd1 "" 0

vwait forever
