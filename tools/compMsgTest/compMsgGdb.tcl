#!/usr/bin/env tclsh8.6

# ===========================================================================
# * Copyright (c) 2017, Arnulf P. Wiedemann (arnulf@wiedemann-pri.de)
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

package require aes

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
if {[file exist ${::moduleFilesPath}/CompMsgKeyValueCallbacks.tcl]} {
  source ${::moduleFilesPath}/CompMsgKeyValueCallbacks.tcl
}

set ::debugBuf ""
set ::debugTxt ""
set ::startBuf ""
set ::startTxt ""
set ::startCommunication false

set ::inDebug false
set ::lastCh ""
set ::totalLgth 999
set ::handleStateInterval 1000
set ::receivedMsg false
set ::msg ""
set ::msgLgth 0
set ::afterId ""
set ::inReceiveMsg false
set ::inGdbCmd false
set ::gdbCmdChksum 0
set ::numGdbChksumChars 0
set ::hadGdbStub false
set ::incrLgth2 false
set ::lgth2 0
set ::hadPrompt false
set ::hadNewLine false

set ::handleInputDbg false

# ==========================================================================
# gdb like debugging
# ==========================================================================

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
  fileevent $::fd0 readable [list readByte0 $::fd0 ::dev0Buf ::dev0Lgth]
}

# ================================ handleGdbCmd ===============================

proc handleGdbCmd {gdbCmd} {
  set cmd [string range $gdbCmd 0 0]
  set params [string range $gdbCmd 1 end]
puts stderr "cmd: $cmd params: $params!"
  switch $cmd {
    "T" {
      set reason $params
      switch $reason {
        "05" {
puts stderr "reason: $reason breakpoint"
          set ::inGdbCmd false
#          set ::answer "\$g#67"
          set ::brkPoint "40265731"
          set answerCmd "Z1,${::brkPoint},04"
          set chkSum 0
          foreach ch [split $answerCmd ""] {
            binary scan $ch c pch
            set chkSum [expr {$chkSum + $pch}]
          }
          set ::answer [format "\$${answerCmd}#%02x" [expr {$chkSum & 0xFF}]]
puts stderr ">>answer: $::answer!"
          foreach ch [split $::answer ""] {
            puts -nonewline $::fd0 $ch
            flush $::fd0
            after 20
          }
puts stderr ">>gdbCmd sent: $::answer!"
        }
        default {
        }
      }
    }
    "d" {
puts stderr "LL: [string length $params]!"
      set registerVals $params
      set regNames [list a0 a1 a2 a3 a4 a5 a6 a7 a8 a9 a10 a11 a12 a13 a14 a15 pc sar litbase sr176 xx ps]
      foreach regName $regNames {
        set $regName [string range $registerVals 0 7]
        set registerVals [string range $registerVals 8 end]
puts stderr "$regName: [set $regName]!"
      }
    }
    "O" {
puts stderr "received command O: $params"
      set answerCmd "c"
      set chkSum 0
      foreach ch [split $answerCmd ""] {
        binary scan $ch c pch
        set chkSum [expr {$chkSum + $pch}]
      }
      set ::answer [format "\$${answerCmd}#%02x" [expr {$chkSum & 0xFF}]]
puts stderr ">>answer: $::answer!"
      foreach ch [split $::answer ""] {
        puts -nonewline $::fd0 $ch
        flush $::fd0
        after 20
      }
puts stderr ">>gdbCmd sent: $::answer!"
    }
    default {
puts stderr "bad cmd: $cmd!"
    }
  }
  return $::COMP_MSG_ERR_OK
}

# ================================ handleInput0 ===============================

proc handleInput0 {ch bufVar lgthVar} {
  upvar $bufVar buf
  upvar $lgthVar lgth

  set pch 0
  binary scan $ch c pch
  if {$::incrLgth2} {
    incr ::lgth2
  }
if {$::inGdbCmd} {
puts stderr "handleInput0 1: ch: $ch lgth: $lgth!lgth2: $::lgth2 ::inGdbCmd: $::inGdbCmd"
}
  if {($lgth == 8) && ($buf eq "gdbstub_")} {
    set ::hadGdbStub true
  }
  switch $ch {
    "+" {
puts stderr "sent command '$::answer' OK"
      set buf ""
      set lgth 0
      set ::incrLgth2 true
      set ::lgth2 0
      return $::COMP_MSG_ERR_OK
    }
    "-" {
      if {$::inGdbCmd} {
puts stderr "sent command '$::answer' NOT OK"
        set buf ""
        set lgth 0
        return $::COMP_MSG_ERR_OK
      }
    }
    "\n" {
      set ::hadNewLine true
      if {$::inGdbCmd} {
puts stderr "gdbCmd: $::gdbCmd"
      } else {
        if {$::hadGdbStub} {
          incr lgth
          puts stderr "== $buf"
          set buf ""
          set lgth 0
        } else {
          # ignore stuff at beginning
          set buf ""
          set lgth 0
        }
      }
      return $::COMP_MSG_ERR_OK
    }
    "O" {
      if {$::hadNewLine} {
puts stderr ">>SET O inGdbCmd true"
        set ::inGdbCmd true
        set ::gdbCmd "O"
        set ::gdbCmdChksum $pch
        return $::COMP_MSG_ERR_OK
      }
    }
    "\$" {
       if {$::hadGdbStub} {
puts stderr ">>SET \$ inGdbCmd true"
        set ::inGdbCmd true
        set ::gdbCmd ""
      }
      return $::COMP_MSG_ERR_OK
    }
    "=" {
       if {$::hadGdbStub} {
puts stderr ">>SET = inGdbCmd false"
        set ::inGdbCmd false
        set ::gdbCmd ""
      }
      return $::COMP_MSG_ERR_OK
    }
    default {
      if {$::inGdbCmd} {
        if {($::numGdbChksumChars == 0) && ($ch ne "#")} {
puts stderr "chksum ch: $pch"
          append ::gdbCmd $ch
          incr ::gdbCmdChksum $pch
        }
        if {$::numGdbChksumChars > 0} {
          append ::gdbReceivedChksum $ch
          if {$::numGdbChksumChars > 1} {
            set rchksum 0x$::gdbReceivedChksum
            if {$rchksum != [expr {$::gdbCmdChksum & 0xff}]} {
puts stderr "bad chksum: $::gdbCmd $rchksum $::gdbCmdChksum"
            } else {
puts stderr "chksum OK"
              set result [handleGdbCmd $::gdbCmd]
              checkErrOK $result
              set ::gdbCmd ""
              set ::numGdbChksumChars 0
              set ::inGdbCmd false
              set ::gdbCmdChksum 0
            }
            return $::COMP_MSG_ERR_OK
          }
          incr ::numGdbChksumChars 1
        }
        if {$ch eq "#"} {
puts stderr "::gdbCmd: $::gdbCmd"
          set ::numGdbChksumChars 1
          set ::gdbReceivedChksum ""
        }
      } else {
        append buf $ch
        incr lgth
      }
      return $::COMP_MSG_ERR_OK
    }
  }
  if {$::inReceiveMsg} {
    append buf $ch
    incr lgth
    set ::lastCh $ch
    return $::COMP_MSG_ERR_OK
  }
  if {!$::inDebug && ($ch eq "M")} {
#puts stderr "got 'M'"
    set ::inReceiveMsg true
    append buf $ch
    incr lgth
    set ::lastCh $ch
    return $::COMP_MSG_ERR_OK
  }
  if {!$::inDebug && ($ch eq ">")} {
puts stderr "got '>'"
    set ::lastCh $ch
    return $::COMP_MSG_ERR_OK
  }
  if {[format 0x%02x [expr {$pch & 0xff}]] eq "0xc2"} {
#puts stderr "ch: $ch!pch: $pch!"
    set ::lastCh $ch
    return -code return
  }
  if {$ch eq "%"} {
#puts stderr "  ==handleInput0 2: got %!startTxt: $::startTxt!debugTxt: $::debugTxt!inDebug: $::inDebug!"
    if {$::inDebug} {
      set ::inDebug false
# puts stderr "  ==handleInput0: DBG: $::debugBuf!"
      append ::debugTxt $ch
#if {[string match "espconn_regist_disconcb:*" $::debugTxt]} {
#  set ::handleInputDbg true
#  puts stderr "set ::handleInputDbg true"
#}
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
    return -code return
  } else {
    if {$::inDebug} {
#      append ::debugBuf $ch
      append ::debugTxt "$ch"
      set ::lastCh $ch
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
      return $::COMP_MSG_ERR_OK
    }
  }
#puts stderr "  ==handleInput0 4 inDebug!$::inDebug!"
  if {$::inDebug} {
#puts stderr "  ==handleInput0: inDebug2 rch: $ch![format 0x%02x [expr {$pch& 0xFF}]]!inDebug: $::inDebug!"
    append ::debugBuf " [format 0x%02x [expr {$pch & 0xff}]]"
    append ::debugTxt "$ch"
    set ::lastCh $ch
    return -code return
  } else {
puts stderr "  ==handleInput0 5: not inDebug rch: lgth!$lgth!$ch![format 0x%02x [expr {$pch& 0xFF}]]!inDebug: $::inDebug!"
    append ::debugBuf " [format 0x%02x [expr {$pch & 0xff}]]"
    append ::debugTxt "$ch"
    if {$ch eq ">"} {
      set lgth 0
      set buf ""
      set ::lastCh $ch
      return -code return
    }
    if {($ch eq " ") && ($::lastCh eq ">")} {
puts stderr "received '> '"
      set ::startCommunication true
      set lgth 0
      set buf ""
      set ::lastCh $ch
      return $::COMP_MSG_ERR_OK
    }
  }
puts stderr "  ==handleInput0 6 end: rch: $ch![format 0x%02x [expr {$pch& 0xFF}]]!"
  append buf $ch
  incr lgth
  set ::lastCh $ch
  return $::COMP_MSG_ERR_OK
}

# ================================ readByte0 ===============================

proc readByte0 {fd bufVar lgthVar} {
  upvar $bufVar buf
  upvar $lgthVar lgth

  set ch [read $fd 1]
  set pch 0
  binary scan $ch c pch
#puts stderr "=readByte0: read: $ch!lgth: $lgth!inDebug: $::inDebug!"
#puts stderr "=readByte0: read: $ch!lgth: $lgth!inGdbCmd: $::inGdbCmd!"
  set result [handleInput0 $ch buf lgth]
  checkErrOK $result
}

init0

vwait forever
