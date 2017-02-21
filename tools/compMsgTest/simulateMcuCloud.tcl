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

set ::crcDebug false

set currState INIT
set states [dict create]

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

set ::handleInputDbg false

# ==========================================================================
# we simulate the Mcu start and the 4th state is Cloud
# start Wifi with startCompMsgU.lua renamed to init.lua!
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

# ================================ handleState ===============================

proc handleState {bufVar lgthVar} {
  upvar $bufVar buf
  upvar $lgthVar lgth

  set myState $::currState
puts stderr "  ==handleInput0: 3 DBT: os handleState: $myState!"
  switch $myState {
    INIT {
      set result [::compMsg compMsgMsgDesc getHeaderFromUniqueFields 22272 19712 ID hdr]
      checkErrOK $result
#puts stderr "=== I after getHeaderFromUniqueFields"
      dict set ::compMsgDispatcher WifiFd $::fd0
      set result [::compMsg compMsgDispatcher createMsgFromHeaderPart ::compMsgDispatcher $hdr handle]
#puts stderr " I createMsgFromHeaderPart: result!$result!"
      checkErrOK $result
      set ::inDebug false
      set buf ""
      set lgth 0
      set ::currState MODULE_INFO
    }
    MODULE_INFO {
      set result [::compMsg compMsgMsgDesc getHeaderFromUniqueFields 22272 19712 MD hdr]
      checkErrOK $result
#puts stderr "=== M after getHeaderFromUniqueFields"
      dict set ::compMsgDispatcher WifiFd $::fd0
      set result [::compMsg compMsgDispatcher createMsgFromHeaderPart ::compMsgDispatcher $hdr handle]
#puts stderr " M createMsgFromHeaderPart: result!$result!"
      checkErrOK $result
      set ::inDebug false
      set buf ""
      set lgth 0
      set ::currState OPERATION_MODE
    }
    OPERATION_MODE {
      set result [::compMsg compMsgMsgDesc getHeaderFromUniqueFields 22272 19712 BD hdr]
      checkErrOK $result
#puts stderr "===after getHeaderFromUniqueFields"
      dict set ::compMsgDispatcher WifiFd $::fd0
      set result [::compMsg compMsgDispatcher createMsgFromHeaderPart ::compMsgDispatcher $hdr handle]
#puts stderr "createMsgFromHeaderPart: result!$result!"
      checkErrOK $result
      set ::currState SEND_CLOUD
    }
    SEND_CLOUD {
      set result [::compMsg compMsgMsgDesc getHeaderFromUniqueFields 17152 19712 CD hdr]
      checkErrOK $result
puts stderr "===CD after getHeaderFromUniqueFields"
      dict set ::compMsgDispatcher WifiFd $::fd0
      set result [::compMsg compMsgDispatcher createMsgFromHeaderPart ::compMsgDispatcher $hdr handle]
puts stderr "===CD createMsgFromHeaderPart: result!$result!"
      checkErrOK $result
      set ::currState SEND_CLOUD
    }
    default {
puts stderr "funny state: $myState!"
    }
  }
  return $::COMP_MSG_ERR_OK
}

# ================================ handleAnswer ===============================

proc handleAnswer {bufVar lgthVar} {
  upvar $bufVar buf
  upvar $lgthVar lgth

#puts stderr "handleAnswer receivedMsg: $::receivedMsg!"
  if {!$::receivedMsg} {
#puts stderr "handleAnswer no message"
    return $::COMP_MSG_ERR_OK
  }
  set buf $::msg
  set lgth $::msgLgth
puts stderr "+++handleAnswer: lgth: $lgth!"
#::compMsg dataView dumpBinary $buf $lgth "handleAnswer MSG"
set header [string range $::msg 0 31]
set payload [string range $::msg 32 end-1]
set totalCrc [string range $::msg end end]
puts stderr "+++HDRL: [string length $header] pl: [string length $payload] [string length $::msg]!"
binary scan $::msg S src
puts stderr "SRC: [format 0x%04x $src]!"
if {$src == 0x4300} {
  set cryptKey "a1b2c3d4e5f6g7h8"
  set mlen [string length $payload]
set decrypted [aes::aes -mode cbc -dir decrypt -key $cryptKey -iv $cryptKey $payload]
#  set result [::compMsg compMsgDispatcher decryptMsg $payload $mlen $cryptKey 16 $cryptKey 16 decrypted decryptedLgth]
puts stderr "decryptedLgth: lgth: mlen: $mlen!decryptedStrLgth: [string length $decrypted]"
  set buf "${header}${decrypted}${totalCrc}"
set lgth2 [string length $buf]
set lgth $lgth2
#::compMsg dataView dumpBinary $buf $lgth2 "handleAnswer decrypted MSG"
puts stderr "totalLgth: $lgth lgth2: $lgth2!"
}
  dict incr received realLgth $lgth
  dict incr received totalLgth $lgth
  set ::receivedHeader false
  set ::receivedMsg false
  set myState $::currState
#puts stderr "handleAnswer: $myState!!"
  set received [dict create]
  dict set received buf $buf
  dict set received lgth $lgth
  ::compMsg dataView setData $buf $lgth
  set result [::compMsg compMsgIdentify getHeaderIndexFromHeaderFields ::compMsgDispatcher received]
  checkErrOK $result
  set result [::compMsg compMsgIdentify handleReceivedMsg ::compMsgDispatcher received]
  checkErrOK $result
#::compMsg compMsgData dumpMsg ::compMsgDispatcher
  set result [::compMsg compMsgData getFieldValue ::compMsgDispatcher @cmdKey value]
puts stderr "cmdKey: [format 0x%04x $value]!result: $result!myState:$myState!"
  checkErrOK $result

  switch $myState {
    INIT {
puts stderr "INIT handleAnswer lgth: $lgth value: [format 0x%04x $value]!"
      binary scan \x49\x41 S cmdKey ; # IA
      if {$value == $cmdKey} {
        set ::currState MODULE_INFO
      }
    }
    MODULE_INFO {
puts stderr "MODULE_INFO handleAnswer lgth: $lgth value: [format 0x%04x $value]!"
      binary scan \x4d\x41 S cmdKey ; # MA
      if {$value == $cmdKey} {
        set ::currState OPERATION_MODE
      }
    }
    OPERATION_MODE {
puts stderr "OPERATION_MODE handleAnswer lgth: $lgth value: [format 0x%04x $value]!"
      binary scan \x4d\x41 S cmdKey ; # MA
      if {$value == $cmdKey} {
        set ::currState SEND_CLOUD
      }
    }
    SEND_CLOUD {
puts stderr "SEND_CLOUD handleAnswer lgth: $lgth value: [format 0x%04x $value]!"
      binary scan \x43\x41 S cmdKey ; # CA
      if {$value == $cmdKey} {
        set ::currState OPERATION_MODE
      }
    }
    default {
puts stderr "funny state: $myState!"
    }
  }
  set result [handleState buf lgth]
#puts stderr "handleState: result: $result!"
  checkErrOK $result
  return $result
}

# ================================ handleInput0 ===============================

proc handleInput0 {ch bufVar lgthVar} {
  upvar $bufVar buf
  upvar $lgthVar lgth

  set pch 0
  binary scan $ch c pch
#if {$::handleInputDbg} {
#puts stderr "handleInput0 1: ch: $ch lastCh: $::lastCh!inDebug: $::inDebug!lgth: $lgth!"
#}
#  if {$::lastCh eq "\r"} {
#    puts stderr "==buf: $buf!"
#    set buf ""
#    set lgth 0
#  }
  if {$::inReceiveMsg && ($pch == 0)} {
    if {$::lastCh eq "M"} {
#puts stderr "found MSG START"
      append buf $ch
      incr lgth
      set ::lastCh $ch
      return $::COMP_MSG_ERR_OK
    } else {
      set ::inReceiveMsg true
    }
  }
  if {$::inReceiveMsg} {
    append buf $ch
    incr lgth
    set ::lastCh $ch
    return $::COMP_MSG_ERR_OK
  }
  if {!$::inDebug && ($ch eq "M")} {
puts stderr "got 'M'"
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

#puts stderr "=readByte0: read!"
  if {$::afterId ne ""} {
    after cancel $::afterId
  }
  set ch [read $fd 1]
  set ::afterId [after 500 [list handleAnswer ::dev0Buf ::dev0Lgth]]
  set pch 0
  binary scan $ch c pch
#puts stderr "=readByte0: read: $ch!lgth: $lgth!inDebug: $::inDebug!"
if {!$::inDebug && ($ch ne "%") && ([format 0x%02x [expr {$pch & 0xff}]] ne "0xc2")} {
#puts stderr "=readByte0: read: $ch!lgth: $lgth!inDebug: $::inDebug!"
}
  set result [handleInput0 $ch buf lgth]
  checkErrOK $result
  

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

# ================================ getSsid ===============================

proc getSsid {compMsgDispatcherVar valueVar} {
  upvar $compMsgDispatcherVar compMsgDispatcher
  upvar $valueVar value

  set msgValPart [dict get $compMsgDispatcher msgValPart]
  set value "Wiedemann3"
  dict set msgValPart fieldValue $value
  dict set compMsgDispatcher msgValPart $msgValPart
  return $::COMP_MSG_ERR_OK
}

# ================================ main ===============================

InitCompMsg
set ::afterHandleStateId [after $::handleStateInterval [list handleState ::dev0Buf ::dev0Lgth]]
init0

vwait forever
