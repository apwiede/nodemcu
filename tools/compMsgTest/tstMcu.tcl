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
source dataView.tcl
source compMsgDataView.tcl
source compMsgMsgDesc.tcl
source compMsgData.tcl
source compMsgDispatcher.tcl
source compMsgIdentify.tcl
source compMsgSendReceive.tcl
source compMsgAction.tcl
source compMsgWifiData.tcl
source compMsgBuildMsg.tcl
source compMsgModuleData.tcl

set ::crcDebug false

set ::debugBuf ""
set ::debugTxt ""
set ::startBuf ""
set ::startTxt ""
set ::startCommunication false

set ::inStart true
set ::inDebug false
set ::lastCh ""
set ::totalLgth 999
set ::totalLgth1 999
set ::receivedMsg false
set ::msg ""
set ::msgLgth 0
set ::afterId ""
set ::inReceiveMsg false
set ::inSendMsg false
set ::hadGt false

# ================================ checkErrOK ===============================

proc checkErrOK {result} {
  switch $result {
    0 {
    }
    default {
      error "ERROR result: $result!"
    }
  }
}

# ::dev0 is the Wifi module
# ::dev1 is the Mcu module

# the wifi module has to be connected to /dev/ttyUSB0
# ================================ init0 ===============================

proc init0 {} {
  set ::dev0 "/dev/ttyUSB0"
  set ::dev0Buf ""
  set ::dev0Lgth 0


  set ::fd0 [open $::dev0 w+]
  fconfigure $::fd0 -blocking 0 -translation binary
  fconfigure $::fd0 -mode 115200,n,8,1
#  fileevent $::fd0 readable [list readByte0 $::fd0 ::dev0Buf ::dev0Lgth]
}

# the ftdi adapter for the Mcu board has to be connected to /dev/ttyUSB1
# ================================ init1 ===============================

proc init1 {} {
  set ::dev1 "/dev/ttyUSB1"
  set ::dev1Buf ""
  set ::dev1Lgth 0


  set ::fd1 [open $::dev1 w+]
  fconfigure $::fd1 -blocking 0 -translation binary
  fconfigure $::fd1 -mode 115200,n,8,1
  fileevent $::fd1 readable [list readByte1 $::fd1 ::dev1Buf ::dev1Lgth]
}

# ================================ handleInput0 ===============================

proc handleInput0 {ch bufVar lgthVar} {
  upvar $bufVar buf
  upvar $lgthVar lgth

  set pch 0
  binary scan $ch c pch
#puts stderr "handleInput0 1: ch: $ch lastCh: $::lastCh!inDebug: $::inDebug!lgth: $lgth!"
if {!$::inStart} {
#puts stderr "handleInput0 1: ch: $ch lastCh: $::lastCh!inDebug: $::inDebug!lgth: $lgth!"
}
  if {$::inSendMsg && ($pch == 0)} {
    if {$::lastCh eq "W"} {
puts stderr "found MSG START"
      append buf $ch
      incr lgth
      set ::lastCh $ch
puts stderr "handleInput0 return 1"
      return $::COMP_MSG_ERR_OK
    } else {
      set ::inSendMsg true
    }
  }
  if {$::inSendMsg} {
    append buf $ch
    incr lgth
    set ::lastCh $ch
#    puts -nonewline $::fd1 $ch
#    flush $::fd1
    if {$lgth == $::headerLgth} {
      # next line needed to set ::totalLgth!!
      binary scan $buf SSSS ::dst ::src ::srcId ::totalLgth
puts stderr [format "sendMsg0 dst: 0x%04x src: 0x%04x srcId: 0x%04x totalLgth: 0x%04x" $::dst $::src $::srcId $::totalLgth]
    }
#puts stderr "handleInput0 sendMsg: ch: $ch lastCh: $::lastCh!inDebug: $::inDebug!lgth: $lgth!"
    if {$lgth >= $::totalLgth} {
puts stderr "sendMsg0 lgth: $lgth totalLgth: $::totalLgth!"
      puts -nonewline $::fd1 $buf
      flush $::fd1
      set ::inSendMsg false
      set ::totalLgth 999
      set lgth 0
      set buf ""
      fileevent $::fd0 readable [list]
      fileevent $::fd1 readable [list readByte1 $::fd1 ::dev1Buf ::dev1Lgth]
    }
#puts stderr "handleInput0 return 3"
    return $::COMP_MSG_ERR_OK
  }
  if {!$::inDebug && ($ch eq "W")} {
puts stderr "got 'W'"
    set ::inSendMsg true
    append buf $ch
    incr lgth
    set ::lastCh $ch
#    puts -nonewline $::fd1 $ch
#    flush $::fd1
#puts stderr "handleInput0 return 4"
    return $::COMP_MSG_ERR_OK
  }
#puts stderr "  ==handleInput0: 2 DBT: $::inDebug!ch: $ch!"
  if {!$::inDebug && ($ch eq ">")} {
    set ::hadGt true
puts stderr "===got '>'"
    set ::lastCh $ch
puts stderr "handleInput0 return 5"
    return $::COMP_MSG_ERR_OK
  }
  if {$::hadGt && ($ch eq " ") && ($::lastCh eq ">")} {
puts stderr "received '> '"
    set ::inStart false
    set ::startCommunication true
    set lgth 0
    set buf ""
    set ::lastCh $ch
    # we have skipped the garbage at start of Wifi
    # now get the Mcu characters
    fileevent $::fd0 readable [list]
    fileevent $::fd1 readable [list readByte1 $::fd1 ::dev1Buf ::dev1Lgth]
puts stderr "handleInput0 return 12"
    return $::COMP_MSG_ERR_OK
  }
  if {[format 0x%02x [expr {$pch & 0xff}]] eq "0xc2"} {
#puts stderr "ch: $ch!pch: $pch!"
    set ::lastCh $ch
#puts stderr "handleInput0 return 6"
    return -code return
  }
  if {$ch eq "%"} {
#puts stderr "  ==handleInput0 2a: got %!startTxt: $::startTxt!debugTxt: $::debugTxt!inDebug: $::inDebug!"
    if {$::inDebug} {
      set ::inDebug false
# puts stderr "  ==handleInput0: DBG: $::debugBuf!"
      append ::debugTxt $ch
puts stderr "  ==handleInput0: 3 DBT: $::debugTxt!"
      set lgth 0
      set buf ""
      set ::debugBuf ""
      set ::debugTxt ""
    } else {
      set ::inDebug true
      set ::debugBuf ""
      set ::debugTxt ""
    }
#puts stderr "handleInput0 return 7 inDebug: $::inDebug!"
    return -code return
  } else {
    if {$::inStart} {
      if {($ch eq "\n") || ($ch eq "\r")} {
        puts stderr "startBuf: $buf!"
        set buf ""
        set lgth 0
        set ::lastCh $ch
      } else {
        append buf $ch
        incr lgth
        set ::lastCh $ch
      }
#puts stderr "handleInput0 return 2"
      return $::COMP_MSG_ERR_OK
    }
    if {$::inDebug} {
#      append ::debugBuf $ch
      append ::debugTxt "$ch"
      set ::lastCh $ch
#puts stderr "handleInput0 return 8"
      return -code return
    } else {
#puts stderr "  ==handleInput0 3a no debug: rch: $ch![format 0x%02x [expr {$pch& 0xFF}]]!"
      if {($lgth <= 2) && (($ch eq "\r") || ($ch eq "\n"))} {
        # ignore debug line end!!
      } else {
        append buf $ch
        incr lgth
      }
      set ::lastCh $ch
#puts stderr "handleInput0 return 9"
      return $::COMP_MSG_ERR_OK
    }
  }
#puts stderr "  ==handleInput0 4 inDebug!$::inDebug!"
  if {$::inDebug} {
#puts stderr "  ==handleInput0: inDebug2 rch: $ch![format 0x%02x [expr {$pch& 0xFF}]]!inDebug: $::inDebug!"
    append ::debugBuf " [format 0x%02x [expr {$pch & 0xff}]]"
    append ::debugTxt "$ch"
    set ::lastCh $ch
puts stderr "handleInput0 return 10"
    return -code return
  } else {
puts stderr "  ==handleInput0 5: not inDebug rch: lgth!$lgth!$ch![format 0x%02x [expr {$pch& 0xFF}]]!inDebug: $::inDebug!"
    append ::debugBuf " [format 0x%02x [expr {$pch & 0xff}]]"
    append ::debugTxt "$ch"
    if {$ch eq ">"} {
      set lgth 0
      set buf ""
      set ::lastCh $ch
puts stderr "handleInput0 return 11"
      return -code return
    }
puts stderr "  ==handleInput0 6 end: rch: $ch![format 0x%02x [expr {$pch& 0xFF}]]!"
  }
  append buf $ch
  incr lgth
  set ::lastCh $ch
puts stderr "handleInput0 return 13"
  return $::COMP_MSG_ERR_OK
}

# ================================ readByte0 ===============================

set ::cnt0 0
proc readByte0 {fd bufVar lgthVar} {
  upvar $bufVar buf
  upvar $lgthVar lgth

#puts stderr "=readByte0: read!"
  set ch [read $fd 1]
incr ::cnt0
  set pch 0
  binary scan $ch c pch
if {!$::inStart} {
#puts stderr "=readByte0: read: cnt: $::cnt0 0 ch: $ch![format 0x%02x [expr {$pch & 0xff}]]!lgth: $lgth!inDebug: $::inDebug!"
}
if {!$::inDebug && ($ch ne "%") && ([format 0x%02x [expr {$pch & 0xff}]] ne "0xc2")} {
#puts stderr "=readByte0: read: $ch![format 0x%02x [expr {$pch & 0xff}]]!lgth: $lgth!inDebug: $::inDebug!"
}

  set result [handleInput0 $ch buf lgth]
  checkErrOK $result
  

if {0} {
  if {$lgth == $::headerLgth} {
    # next line needed to set ::totalLgth!!
    binary scan $buf SSSS ::dst ::src ::srcId ::totalLgth
#puts stderr [format "dst: 0x%04x src: 0x%04x srcId: 0x%04x totalLgth: 0x%04x" $::dst $::src $::srcId $::totalLgth]
  }
  if {$lgth >= $::totalLgth} {
#puts stderr "lgth: $lgth totalLgth: $::totalLgth!"
    set ::inReceiveMsg false
    set ::receivedMsg true
    set ::msg $buf
    set ::msgLgth $lgth
    set myBuf ""
    foreach ch [split $buf ""] {
      binary scan $ch c pch
      append myBuf " [format 0x%02x [expr {$pch & 0xFF}]]"
    }
#    puts stderr "1: got message: for $myBuf"
set ::totalLgth 999
#puts stderr "readByte0: end"
    return $::COMP_MSG_ERR_OK
  }
}
  return $::COMP_MSG_ERR_OK
}

# ================================ handleInput1 ===============================

proc handleInput1 {ch bufVar lgthVar} {
  upvar $bufVar buf
  upvar $lgthVar lgth

  set pch 0
  binary scan $ch c pch
#puts stderr "handleInput1 1: ch: $ch lastCh: $::lastCh!inDebug: $::inDebug!lgth: $lgth!"
#puts stderr "  ==handleInput1 6 end: rch: $ch![format 0x%02x [expr {$pch& 0xFF}]]!"
  append buf $ch
  incr lgth
  return $::COMP_MSG_ERR_OK
}

# ================================ readByte1 ===============================

proc readByte1 {fd bufVar lgthVar} {
  upvar $bufVar buf
  upvar $lgthVar lgth

#puts stderr "=readByte1: read!"
  set ch [read $fd 1]
  set pch 0
  binary scan $ch c pch
puts stderr "=readByte1: read: $ch![format 0x%02x [expr {$pch & 0xff}]]!lgth: $lgth!inDebug: $::inDebug!"

  set result [handleInput1 $ch buf lgth]
  checkErrOK $result
  

  if {$lgth == $::headerLgth} {
    # next line needed to set ::totalLgth1!!
    binary scan $buf SSSS ::dst1 ::src1 ::srcId1 ::totalLgth1
puts stderr [format "dst: 0x%04x src: 0x%04x srcId: 0x%04x totalLgth: 0x%04x" $::dst1 $::src1 $::srcId1 $::totalLgth1]
  }
  if {$lgth >= $::totalLgth1} {
puts stderr "lgth: $lgth totalLgth1: $::totalLgth1!"
    foreach ch [split $buf ""] {
      binary scan $ch c pch
      append myBuf " [format 0x%02x [expr {$pch & 0xFF}]]"
    }
    puts stderr "1: got message: for $myBuf"
set ::totalLgth1 999
puts stderr "readByte1: end"

    set simulate false
    if {!$simulate} {
      fileevent $::fd1 readable [list] ; # prevent more input before Wifi has answered
      fileevent $::fd0 readable [list ::readByte0 $::fd0 ::dev0Buf ::dev0Lgth]
      foreach ch [split $buf ""] {
        puts -nonewline $::fd0 $ch
        flush $::fd0
      }
    } else {
      binary scan $buf x32S cmdKey
puts stderr [format "cmdKey: 0x%04x $cmdKey" $cmdKey]
      switch $cmdKey {
        18756 {
puts stderr "IA"
          set result [::compMsg compMsgMsgDesc getHeaderFromUniqueFields 19712 22272 IA hdr]
        }
        16964 {
puts stderr "BA"
          set result [::compMsg compMsgMsgDesc getHeaderFromUniqueFields 19712 22272 BA hdr]
        }
        19780 {
puts stderr "MA"
          set result [::compMsg compMsgMsgDesc getHeaderFromUniqueFields 19712 22272 MA hdr]
        }
      }
puts stderr "result1: $result!"
      checkErrOK $result
#puts stderr "=== I after getHeaderFromUniqueFields"
      dict set ::compMsgDispatcher WifiFd $::fd1
      set result [::compMsg compMsgDispatcher createMsgFromHeaderPart ::compMsgDispatcher $hdr handle]
puts stderr " I createMsgFromHeaderPart: result!$result!"
      checkErrOK $result
}

puts stderr "readByte1: msg sent"
    set buf ""
    set lgth 0
    return $::COMP_MSG_ERR_OK
  }
}

# ================================ InitCompMsg ===============================

proc InitCompMsg {} {
  set ::compMsgDispatcher [dict create]
  set result [::compMsg compMsgDispatcher newCompMsgDispatcher ::compMsgDispatcher]
  checkErrOK $result
  set result [::compMsg compMsgDispatcher createDispatcher dispatcherHandle]
  checkErrOK $result
#puts stderr "dispatcherHandle!$dispatcherHandle!"
  set result [::compMsg compMsgDispatcher initDispatcher ::compMsgDispatcher]
  checkErrOK $result
  set ::headerInfos [dict get $::compMsgDispatcher headerInfos]
  set ::headerLgth [dict get $::compMsgDispatcher headerInfos headerLgth]
#puts stderr "headerLgth: $::headerLgth!"
}

# ================================ getSrcId ===============================

proc getSrcId {compMsgDispatcherVar valueVar} {
  upvar $compMsgDispatcherVar compMsgDispatcher
  upvar $valueVar value

  set msgValPart [dict get $compMsgDispatcher msgValPart]
  set value 12345
  dict set msgValPart fieldValue $value
  dict set compMsgDispatcher msgValPart $msgValPart
  return $::COMP_MSG_ERR_OK
}

# ================================ getGUID ===============================

proc getGUID {compMsgDispatcherVar valueVar} {
  upvar $compMsgDispatcherVar compMsgDispatcher
  upvar $valueVar value

  set msgValPart [dict get $compMsgDispatcher msgValPart]
  set value "1234-5678-9012-1"
  dict set msgValPart fieldValue $value
  dict set compMsgDispatcher msgValPart $msgValPart
  return $::COMP_MSG_ERR_OK
}

# ================================ main ===============================

InitCompMsg
init0
init1

vwait forever
