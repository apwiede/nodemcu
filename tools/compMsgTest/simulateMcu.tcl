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

set currState INIT
set states [dict create]

set ::debugBuf ""
set ::debugTxt ""
set ::startBuf ""
set ::startTxt ""
set ::startCommunication false
set ::isStart true

set ::inDebug false
set ::lastch ""
set ::totalLgth 999

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

# ================================ init0 ===============================

proc init0 {} {
  set ::dev0 "/dev/ttyUSB0"
  set ::dev0Buf ""
  set ::dev0Lgth 0


  set ::fd0 [open $::dev0 w+]
  fconfigure $::fd0 -blocking 0 -translation binary
  fconfigure $::fd0 -mode 115200,n,8,1
#  puts $::fd0 {dofile("startCompMsgUart.lua")}
#  flush $::fd0
  fileevent $::fd0 readable [list readByte0 $::fd0 ::dev0Buf ::dev0Lgth]
}

# ================================ handleInput0 ===============================

proc handleInput0 {ch bufVar lgthVar} {
  upvar $bufVar buf
  upvar $lgthVar lgth

  set pch 0
  binary scan $ch c pch
  if {$::isStart} {
#puts stderr "  ==handleInput0: isStart rch: $ch![format 0x%02x [expr {$pch & 0xFF}]]!"
    if {$ch eq "\n"} {
      puts stderr "  ==in START!$::startTxt!"
      set ::startBuf ""
      set ::startTxt ""
    }
    if {$ch eq ">"} {
      set ::isStart false
      return -code return
    }
    append ::startTxt $ch
if {0} {
    if {($ch eq "\x57") || ($ch eq "\x4D")} {
      set ::lastch $ch
      append buf $ch
      incr lgth
      return -code return
    }
    if {$ch == 0} {
      if {($::lastch eq "\x57") || ($::lastch eq "\x4D")} {
        append buf $ch
        incr lgth
        set ::isStart false
        puts stderr "STA!$::startTxt!"
        append buf
        set ::startBuf ""
        set ::startTxt ""
# puts stderr "  ==handleInput0: DBG: $::debugBuf!"
puts stderr "  ==handleInput0: DBT: $::debugTxt!"
        set ::debugBuf ""
        set ::debugTxt ""
      }
    } else {
      set ::lastch $ch
    }
}
    if {$ch eq "§"} {
puts stderr "  ==handleInput0: START 2!$::startTxt!"
      set ::startBuf ""
      set ::startTxt ""
      set ::isStart false
      set ::startCommunication true
#      fileevent $::fd0 readable [list readByte0 $::fd0 ::dev0Buf ::dev0Lgth]
    } else {
      return -code return
    }
  }
  if {[format 0x%02x [expr {$pch & 0xff}]] eq "0xc2"} {
#puts stderr "ch: $ch!pch: $pch!"
    return -code return
  }
  if {$ch eq "§"} {
#puts stderr "  ==handleInput0: inDebug rch: $ch![format 0x%02x [expr {$pch& 0xFF}]]!inDebug: $::inDebug!"
    if {$::inDebug} {
      set ::inDebug false
# puts stderr "  ==handleInput0: DBG: $::debugBuf!"
      append ::debugTxt $ch
puts stderr "  ==handleInput0: DBT: $::debugTxt!"
      set ::debugBuf ""
      set ::debugTxt ""
    } else {
      set ::inDebug true
      set ::debugBuf ""
      set ::debugTxt "$ch"
    }
    return -code return
  }
  if {$::inDebug} {
#puts stderr "  ==handleInput0: inDebug2 rch: $ch![format 0x%02x [expr {$pch& 0xFF}]]!inDebug: $::inDebug!"
    append ::debugBuf " [format 0x%02x [expr {$pch & 0xff}]]"
    append ::debugTxt "$ch"
    return -code return
  } else {
puts stderr "  ==handleInput0: not inDebug rch: lgth!$lgth!$ch![format 0x%02x [expr {$pch& 0xFF}]]!inDebug: $::inDebug!"
    append ::debugBuf " [format 0x%02x [expr {$pch & 0xff}]]"
    append ::debugTxt "$ch"
    if {$ch eq ">"} {
      set ::startCommunication true
      set lgth 0
      set buf ""
#      fileevent $::fd0 readable [list readByte0 $::fd0 ::dev0Buf ::dev0Lgth]
      return -code return
    }
  }
puts stderr "  ==handleInput0 end: rch: $ch![format 0x%02x [expr {$pch& 0xFF}]]!"
  append buf $ch
  incr lgth
  return $::COMP_MSG_ERR_OK
}

# ================================ handleState ===============================

proc handleState {bufVar lgthVar} {
  upvar $bufVar buf
  upvar $lgthVar lgth

  set myState $::currState
puts stderr "handleState: $myState!"
  switch $myState {
    INIT {
      fileevent $::fd0 readable [list]
      set result [::compMsg compMsgMsgDesc getHeaderFromUniqueFields 22272 19712 ID hdr]
      checkErrOK $result
puts stderr "===after getHeaderFromUniqueFields"
      dict set ::compMsgDispatcher WifiFd $::fd0
      set result [::compMsg compMsgDispatcher createMsgFromHeaderPart ::compMsgDispatcher $hdr handle]
puts stderr "createMsgFromHeaderPart: result!$result!"
      checkErrOK $result
      set ::currState MODULE_INFO
      set ::totalLgth 999
      fileevent $::fd0 readable [list readByte0 $::fd0 ::dev0Buf ::dev0Lgth]
    }
    MODULE_INFO {
      fileevent $::fd0 readable [list]
      set result [::compMsg compMsgMsgDesc getHeaderFromUniqueFields 22272 19712 MD hdr]
      checkErrOK $result
puts stderr "===after getHeaderFromUniqueFields"
      dict set ::compMsgDispatcher WifiFd $::fd0
      set result [::compMsg compMsgDispatcher createMsgFromHeaderPart ::compMsgDispatcher $hdr handle]
puts stderr "createMsgFromHeaderPart: result!$result!"
      checkErrOK $result
puts stderr "should send MDMsg"
      set ::currState OPERATION_MODE
      set ::totalLgth 999
      fileevent $::fd0 readable [list readByte0 $::fd0 ::dev0Buf ::dev0Lgth]
    }
    OPERATION_MODE {
      fileevent $::fd0 readable [list]
      set result [::compMsg compMsgMsgDesc getHeaderFromUniqueFields 22272 19712 BD hdr]
      checkErrOK $result
puts stderr "===after getHeaderFromUniqueFields"
      dict set ::compMsgDispatcher WifiFd $::fd0
      set result [::compMsg compMsgDispatcher createMsgFromHeaderPart ::compMsgDispatcher $hdr handle]
puts stderr "createMsgFromHeaderPart: result!$result!"
      checkErrOK $result
puts stderr "should send BDMsg"
      set ::currState MODULE_INFO
      set ::totalLgth 999
      fileevent $::fd0 readable [list readByte0 $::fd0 ::dev0Buf ::dev0Lgth]
    }
    default {
puts stderr "funny state: $myState!"
    }
  }
  return $::COMP_MSG_ERR_OK
}

# ================================ readByte0 ===============================

proc readByte0 {fd bufVar lgthVar} {
  upvar $bufVar buf
  upvar $lgthVar lgth

  set ch [read $fd 1]
  set result [handleInput0 $ch buf lgth]
puts stderr "=readByte0: result: $result!"
  checkErrOK $result
  

puts stderr "startCommunication: $::startCommunication!"
  if {$::startCommunication} {
    set ::startCommunication false
    set buf ""
    set lgth 0
    set ::currState INIT
    return [handleState buf lgth]
  }
puts stderr "lgth: $lgth!headerLgth!$::headerLgth!"
  if {$lgth == $::headerLgth} {
puts stderr "buf: $buf!"
    binary scan $buf SSSS ::dst ::src ::srcId ::totalLgth
puts stderr [format "dst: 0x%04x src: 0x%04x srcId: 0x%04x totalLgth: 0x%04x" $::dst $::src $::srcId $::totalLgth]
  }
puts stderr "lgth: $lgth totalLgth: $::totalLgth!"
  if {$lgth >= $::totalLgth} {
    set myBuf ""
    foreach ch [split $buf ""] {
      binary scan $ch c pch
      append myBuf " [format 0x%02x [expr {$pch & 0xFF}]]"
    }
#    puts stderr "1: got telegram: for cmdKey: $::cmdKey: $myBuf"
    puts stderr "1: got telegram: for $myBuf"
#    puts -nonewline $::fd0 $buf
#    flush $::fd0
#    fileevent $::fd0 readable [list readByte0 $::fd0 ::dev0Buf ::dev0Lgth]
#    set telegram $buf
#    set buf ""
#    set lgth 0
#    set ::telegramLgth 99
#    fileevent $::fd1 readable [list]
  }
}


# ================================ InitCompMsg ===============================

proc InitCompMsg {} {
  set ::compMsgDispatcher [dict create]
  set result [::compMsg compMsgDispatcher newCompMsgDispatcher]
  checkErrOK $result
  set result [::compMsg compMsgDispatcher createDispatcher dispatcherHandle]
  checkErrOK $result
puts stderr "dispatcherHandle!$dispatcherHandle!"
  set result [::compMsg compMsgDispatcher initDispatcher ::compMsgDispatcher]
  checkErrOK $result
  set ::headerLgth [dict get $::compMsgDispatcher headerInfos headerLgth]
puts stderr "headerLgth: $::headerLgth!"
}

# ================================ getSrcId ===============================

proc getSrcId {compMsgDispatcherVar} {
  upvar $compMsgDispatcherVar compMsgDispatcher

  set msgValPart [dict get $compMsgDispatcher msgValPart]
  dict set msgValPart fieldValue 12345
  dict set compMsgDispatcher msgValPart $msgValPart
  return $::COMP_MSG_ERR_OK
}

# ================================ getGUID ===============================

proc getGUID {compMsgDispatcherVar} {
  upvar $compMsgDispatcherVar compMsgDispatcher

  set msgValPart [dict get $compMsgDispatcher msgValPart]
  dict set msgValPart fieldValue "1234-5678-9012-1"
  dict set compMsgDispatcher msgValPart $msgValPart
  return $::COMP_MSG_ERR_OK
}

# ================================ main ===============================

InitCompMsg
init0

vwait forever
