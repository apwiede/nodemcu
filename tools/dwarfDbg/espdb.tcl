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

package require Tk
package require tablelist
lappend auto_path [pwd]/lib
package require dwarfDbgClass

source autoscroll.tcl
source apwWin.tcl
source showSourceFile.tcl

set ::compMsgPath ../compMsgTest
source ${::compMsgPath}/pdict.tcl
source ${::compMsgPath}/dataView.tcl
source ${::compMsgPath}/compMsgDataView.tcl
source ${::compMsgPath}/compMsgMsgDesc.tcl
source ${::compMsgPath}/compMsgData.tcl
source ${::compMsgPath}/compMsgDispatcher.tcl
source ${::compMsgPath}/compMsgIdentify.tcl
source ${::compMsgPath}/compMsgSendReceive.tcl
source ${::compMsgPath}/compMsgAction.tcl
source ${::compMsgPath}/compMsgWifiData.tcl
source ${::compMsgPath}/compMsgBuildMsg.tcl
source ${::compMsgPath}/compMsgModuleData.tcl

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
set ::expectRegisters false

set ::handleInputDbg false

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

# ================================ handleTagDouble1 ===============================

proc handleTagDouble1 {w x y} {
  set index [$w index @$x,$y]
  foreach {line col} [split $index "."] break
  set ::brkPointLine $line
  $w tag add brkPointLine $line.0 $line.end
  $w tag configure brkPointLine -foreground blue
puts stderr "LINES: [dict get $::linesDict $line]!"
  set ::brkPoint [dict get $::linesDict $line addr]
  set ::brkPointVal [format "Breakpoint PC: 0x%08x" $::brkPoint]
#puts stderr "::brkPoint: [format 0x%08x $::brkPoint]!"
  callDbgSetBreakPoint [format %08x $::brkPoint]
}

# ================================ callDbgSetBreakPoint ===============================

proc callDbgSetBreakPoint {brkPoint} {
  set dbgCmd "Z1,${brkPoint},04"
  set chkSum 0
  foreach ch [split $dbgCmd ""] {
    binary scan $ch c pch
    set chkSum [expr {$chkSum + $pch}]
  }
  set ::answer [format "\$${dbgCmd}#%02x" [expr {$chkSum & 0xFF}]]
#puts stderr ">>answer: $::answer!"
  foreach ch [split $::answer ""] {
    puts -nonewline $::fd0 $ch
    flush $::fd0
#    after 20
  }
#puts stderr ">>gdbCmd sent: $::answer!"
}

# ================================ callDbgResetBreakPoint ===============================

proc callDbgResetBreakPoint {} {
  set line [dict get $::addressesDict $::brkPoint line]
  $::textId tag configure brkPointLine -foreground black
  $::textId tag delete brkPointLine $line.0 $line.end

  set brkPoint [format %08x $::brkPoint]
  set dbgCmd "z1,${brkPoint},01"
  set chkSum 0
  foreach ch [split $dbgCmd ""] {
    binary scan $ch c pch
    set chkSum [expr {$chkSum + $pch}]
  }
  set ::answer [format "\$${dbgCmd}#%02x" [expr {$chkSum & 0xFF}]]
#puts stderr ">>answer: $::answer!"
  foreach ch [split $::answer ""] {
    puts -nonewline $::fd0 $ch
    flush $::fd0
    after 20
  }
  set ::brkPointVal "Breakpoint PC: "
#puts stderr ">>gdbCmd sent: $::answer!"
}

# ================================ callDbgContinue ===============================

proc callDbgContinue {} {
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

# ================================ callDbgGetRegisters ===============================

proc callDbgGetRegisters {} {
  set answerCmd "g"
  set ::expectRegisters true
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

# ================================ callDbgReadBytes ===============================

proc callDbgReadBytes {} {
#  set addr "40272611"
  set addr "3ffffc30"
#  set dbgCmd "m${addr},c"
  set dbgCmd "m${addr},4"
  set chkSum 0
  foreach ch [split $dbgCmd ""] {
    binary scan $ch c pch
    set chkSum [expr {$chkSum + $pch}]
  }
  set ::answer [format "\$${dbgCmd}#%02x" [expr {$chkSum & 0xFF}]]
puts stderr ">>answer: $::answer!"
  foreach ch [split $::answer ""] {
    puts -nonewline $::fd0 $ch
    flush $::fd0
    after 20
  }
puts stderr ">>gdbCmd sent: $::answer!"
}

# ================================ handleGdbCmd ===============================

proc handleGdbCmd {gdbCmd} {
  set cmd [string range $gdbCmd 0 0]
  set params [string range $gdbCmd 1 end]
#puts stderr "handleGdbCmd: cmd: $cmd!"
  switch $cmd {
    "T" {
puts stderr "cmd: $cmd params: $params!"
      set reason [string range $params 0 1]
      set pc [string range $params 5 end-1]
      switch $reason {
        "08" -
        "07" -
        "06" -
        "05" -
        "04" {
puts stderr "reason: $reason breakpoint!pc: $pc!"
          scan $pc %x pcVal
          if {[dict exists $::addressesDict $pcVal]} {
puts stderr "addresses: [dict get $::addressesDict $pcVal]"
flush stderr
            set line [dict get $::addressesDict $pcVal]
            $::textId tag configure brkPointLine -foreground red
          }
          set ::inGdbCmd false
        }
        default {
puts stderr "default reason: $reason breakpoint"
        }
      }
    }
    "q" {
puts stderr "q LL: [string length $params]!"
      set registerVals $params
      set regNames [list a0 a1 a2 a3 a4 a5 a6 a7 a8 a9 a10 a11 a12 a13 a14 a15 pc sar litbase sr176 xx ps]
      foreach regName $regNames {
        set $regName [string range $registerVals 0 7]
        set registerVals [string range $registerVals 8 end]
        set val [set $regName]
        set p4 [string range $val 0 1]
        set p3 [string range $val 2 3]
        set p2 [string range $val 4 5]
        set p1 [string range $val 6 7]
        set swapRegName "${p1}${p2}${p3}${p4}!"
#puts stderr "$regName: [set $regName]!"
puts stderr [format "$regName: 0x%s" $swapRegName]
      }
    }
    "f" -
    "e" -
    "d" -
    "c" -
    "b" -
    "a" -
    "9" -
    "8" -
    "7" -
    "6" -
    "5" -
    "4" -
    "3" -
    "2" -
    "1" -
    "0" {
      if {$::expectRegisters} {
puts stderr "regs LL: [string length $gdbCmd]!"
        set registerVals $gdbCmd
        set regNames [list a0 a1 a2 a3 a4 a5 a6 a7 a8 a9 a10 a11 a12 a13 a14 a15 pc sar litbase sr176 xx ps reason]
        foreach regName $regNames {
          set $regName [string range $registerVals 0 7]
          set registerVals [string range $registerVals 8 end]
          set val [set $regName]
          set p4 [string range $val 0 1]
          set p3 [string range $val 2 3]
          set p2 [string range $val 4 5]
          set p1 [string range $val 6 7]
          set swapRegName "${p1}${p2}${p3}${p4}!"
#puts stderr "$regName: [set $regName]!"
puts stderr [format "$regName: 0x%s" $swapRegName]
        }
        set ::expectRegisters false
      } else {
        set val $gdbCmd
        set p4 [string range $val 0 1]
        set p3 [string range $val 2 3]
        set p2 [string range $val 4 5]
        set p1 [string range $val 6 7]
        set swapVal "${p1}${p2}${p3}${p4}!"
puts stderr [format "VAL: 0x%s" $swapVal]
      }
    }
    "O" {
puts stderr "cmd: $cmd params: $params!"
    }
    "Y" {
      set str ""
      foreach {ch1 ch2} [split $params ""] {
        scan 0x${ch1}${ch2} %x val
        append str [format "%c" $val]
      }
      set str [string trim $str \n]
puts stderr "cmd: $cmd params: $str!"
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
#puts stderr "handleInput0 1: ch: $ch lgth: $lgth!lgth2: $::lgth2 ::inGdbCmd: $::inGdbCmd"
}
  if {($lgth == 8) && ($buf eq "gdbstub_")} {
    set ::hadGdbStub true
  }
  switch $ch {
    "+" {
      if {$::inGdbCmd} {
puts stderr "sent command + '$::answer' OK"
        set buf ""
        set lgth 0
        set ::incrLgth2 true
        set ::lgth2 0
        return $::COMP_MSG_ERR_OK
      }
    }
    "-" {
      if {$::inGdbCmd} {
puts stderr "sent command - '$::answer' NOT OK"
        set buf ""
        set lgth 0
        return $::COMP_MSG_ERR_OK
      }
    }
    "\n" {
      set ::hadNewLine true
      if {$::inGdbCmd} {
#puts stderr "gdbCmd: $::gdbCmd"
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
    "Y" -
    "O" {
      if {$::hadNewLine} {
#puts stderr ">>$ch SET inGdbCmd true"
        set ::inGdbCmd true
        set ::gdbCmd $ch
        set ::gdbCmdChksum $pch
        set ::numGdbChksumChars 0
        return $::COMP_MSG_ERR_OK
      }
    }
    "\$" {
       if {$::hadGdbStub || ($::lastCh eq "+") || ($::lastCh eq "\n")} {
#puts stderr ">>SET \$ inGdbCmd true"
        set ::inGdbCmd true
        set ::gdbCmd ""
      }
      return $::COMP_MSG_ERR_OK
    }
    "=" {
       if {$::hadGdbStub} {
#puts stderr ">>SET = inGdbCmd false"
        set ::inGdbCmd false
        set ::gdbCmd ""
      }
      return $::COMP_MSG_ERR_OK
    }
    default {
      if {$::inGdbCmd} {
        if {($::numGdbChksumChars == 0) && ($ch ne "#")} {
#puts stderr "chksum ch: $pch"
          append ::gdbCmd $ch
          incr ::numGdbCmdChars
          incr ::gdbCmdChksum $pch
        }
        if {$::numGdbChksumChars > 0} {
          append ::gdbReceivedChksum $ch
          if {$::numGdbChksumChars > 1} {
            set rchksum 0x$::gdbReceivedChksum
            if {$rchksum != [expr {$::gdbCmdChksum & 0xff}]} {
puts stderr "bad chksum: $::gdbCmd $rchksum $::gdbCmdChksum numch: $::numGdbCmdChars"
            } else {
#puts stderr "chksum OK"
              set result [handleGdbCmd $::gdbCmd]
              checkErrOK $result
              set ::gdbCmd ""
              set ::numGdbChksumChars 0
              set ::inGdbCmd false
              set ::numGdbCmdChars 0
              set ::gdbCmdChksum 0
              # for safety otherwise sometimes problems with recognizing OK
              set ::hadNewLine true
            }
            return $::COMP_MSG_ERR_OK
          }
          incr ::numGdbChksumChars 1
        }
        if {$ch eq "#"} {
#puts stderr "::gdbCmd: $::gdbCmd"
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
  set ::lastCh $ch
}

# ================================ buildWidget ===============================

proc buildWidget {} {
  apwWin Init
  showSourceFile Init
#  frame .tbl -width 600 -height 100
#  set tableFr .tbl
#  set tableId [showDefinition CreateScrolledTablelist $tableFr showDefinition]
#  set lst [list 0 ssid 0 rssi]
#  set valueLst [list]
#  set rowLst [list]
#  set row 0

#set ::APTableId $tableId
#  $tableId configure -width 100
#  $tableId configure -columns $lst
#  foreach rowLst $valueLst {
#    $tableId insert end $rowLst
#  }
  frame .info -width 600 -height 10
  set ::infoFr .info
  ttk::button ${::infoFr}.continueb -text Continue -command ::callDbgContinue
  ttk::button ${::infoFr}.registersb -text Registers -command ::callDbgGetRegisters
  ttk::button ${::infoFr}.breakpointl -textvariable ::brkPointVal -command ::callDbgResetBreakPoint
set ::bytesVal "read bytes"
  ttk::button ${::infoFr}.readbytesl -textvariable ::bytesVal -command ::callDbgReadBytes
  pack ${::infoFr}.continueb ${::infoFr}.registersb ${::infoFr}.breakpointl ${::infoFr}.readbytesl -side left
  frame .txt -width 600 -height 100
  set txtFr .txt
  set ::textId [showSourceFile CreateScrolledText $txtFr showSource 40]
  pack $txtFr $::infoFr -side top
}

# ================================ ShowFile ===============================

proc ShowFile {fileName} {
  $::textId delete 0.0 end
  set fd [open $fileName r]
  set lineNo 1
  while {[gets $fd line] >= 0} {
    $::textId insert end "${line}\n"
    incr lineNo
  }
$::textId configure -foreground grey
  close $fd
}

# ================================ InitCompMsg ===============================

proc InitCompMsg {} {
  set result [::compMsg compMsgDispatcher newCompMsgDispatcher ::compMsgDispatcher]
  checkErrOK $result
  set result [::compMsg compMsgDispatcher createDispatcher dispatcherHandle]
  checkErrOK $result
puts stderr "dispatcherHandle!$dispatcherHandle!"
  set result [::compMsg compMsgDispatcher initDispatcher compMsgDispatcher]
  checkErrOK $result
}

# ================================ getCompileUnitInfos ===============================

proc getCompileUnitInfos {} {
  set ::fileInfos [::dD getFileInfos]
  # each entry has the follwing sub entries
  # compileUnitFileName compileUnitIdx filenameIdx numFileInfo numFileLine dirName
  set ::compileUnitInfos [dict create]
  set ::compileUnitFiles [dict create]
  foreach entry $::fileInfos {
    foreach {compileUnitFileName compileUnitIdx fileNameIdx numFileInfo numFileLine dirName} $entry break
    set myDict [dict create]
    dict set myDict shortFileName $compileUnitFileName
    dict set myDict fileNameIdx $fileNameIdx
    dict set myDict numFileInfo $numFileInfo
    dict set myDict numFileline $numFileLine
    dict set myDict dirName $dirName
    dict set ::compileUnitInfos $compileUnitIdx $myDict
    dict set ::compileUnitFiles $compileUnitFileName $compileUnitIdx
  }
}

# ================================ showDebugFile ===============================

proc showDebugFile {sourceDirName sourceFileName} {
  if {![dict exists $::compileUnitFiles $sourceFileName]} {
puts stderr "no ssuch source file: $sourceFileName!"
  }
  set ::currCompileUnitIdx [dict get $::compileUnitFiles $sourceFileName]
  set dirName [dict get $::compileUnitInfos $::currCompileUnitIdx dirName]
puts stderr "DIR: $dirName!"
  set sourcePath ${dirName}/${sourceFileName}
  ShowFile $sourcePath

  set fileLinesDict [::dD getFileLines $::currCompileUnitIdx]
  set ::addressesDict [dict get $fileLinesDict addresses]
  set ::linesDict [dict get $fileLinesDict lines]

  foreach dbgLine [dict keys $::linesDict] {
    $::textId tag add dbgLine $dbgLine.0 $dbgLine.end
  }
  $::textId tag configure dbgLine -foreground black
  $::textId tag bind dbgLine <Button-1> [list handleTagDouble1 %W %x %y]
}

# ================================ main ===============================

# InitCompMsg

if {[llength $argv] > 0} {
  set sourceFileName [lindex $argv 0]
} else {
  set sourceFileName "compMsgAction.c"
}

#buildWidget
dwarfDbgClass create ::dD
::dD init
::dD openElf /home/arnulf/bene-nodemcu-firmware/app/.output/eagle/debug/image/eagle.app.v6.0.out
::dD getDbgInfos
#scan "0x4025eaab" %x pc
scan "0x4025ea24" %x pc
scan "0x3ffffbb0" %x fp
foreach varName [list yyyQQQ yyyZZZ yyyYYY result hdr handle id stopbits] {
  set addr [::dD getVarAddr compMsgDispatcher.c 357 $varName $pc $fp]
puts [format " @@$varName addr: 0x%008x" $addr]
}
getCompileUnitInfos
flush stderr

set sourceDirName "/home/arnulf/apwiede-nodemcu-firmware/app/compMsg"
showDebugFile $sourceDirName $sourceFileName

puts stderr "call init0"
#init0

#vwait forever
