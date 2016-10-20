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

package require Tk
package require tablelist

source structmsgCmd.tcl

::websocket::loglevel debug

source pdict.tcl
source dataView.tcl
source structmsgDataView.tcl
source structmsgData.tcl
source structmsgDispatcher.tcl
source structmsgIdentify.tcl
source structmsgSendReceive.tcl
source structmsgDefinitions.tcl

proc checkErrOk {result} {
  if {$result != $::STRUCT_MSG_ERR_OK} {
    puts stderr "result: $result"
  }
}

namespace eval apwWin {
    variable ns [namespace current]

    namespace ensemble create

    namespace export CreateScrolledWidget Init CreateScrolledTablelist


    proc Init {} {
        variable infoWinNo
        variable xPosShowEntryBox
        variable yPosShowEntryBox

        set infoWinNo 1
        set xPosShowEntryBox -1
        set yPosShowEntryBox -1
    }

    proc CreateScrolledWidget { wType w useAutoScroll titleStr args } {
        variable ns
        variable sApw

        if { [winfo exists $w.par] } {
            destroy $w.par
        }
        ttk::frame $w.par
        pack $w.par -side top -fill both -expand 1
        if { $titleStr ne "" } {
            label $w.par.label -text "$titleStr" -anchor center
            set sApw(bgDefaultColor) [$w.par.label cget -background]
            set sApw(fgDefaultColor) [$w.par.label cget -foreground]
        }
        $wType $w.par.widget \
               -xscrollcommand "$w.par.xscroll set" \
               -yscrollcommand "$w.par.yscroll set" {*}$args
        ttk::scrollbar $w.par.xscroll -command "$w.par.widget xview" -orient horizontal
        ttk::scrollbar $w.par.yscroll -command "$w.par.widget yview" -orient vertical
        set rowNo 0
        if { $titleStr ne "" } {
            set rowNo 1
            grid $w.par.label -sticky ew -columnspan 2
        }
        grid $w.par.widget $w.par.yscroll -sticky news
        grid $w.par.xscroll               -sticky ew

        grid rowconfigure    $w.par $rowNo -weight 1
        grid columnconfigure $w.par 0      -weight 1

        bind $w.par.widget <MouseWheel>       "${ns}::MouseWheelCB $w.par.widget %D y"
        bind $w.par.widget <Shift-MouseWheel> "${ns}::MouseWheelCB $w.par.widget %D x"

        if { $useAutoScroll } {
            autoscroll::autoscroll $w.par.xscroll
            autoscroll::autoscroll $w.par.yscroll
        }

        return $w.par.widget
    }

    proc CreateScrolledTablelist { w useAutoScroll titleStr args } {
        return [CreateScrolledWidget tablelist::tablelist $w $useAutoScroll $titleStr {*}$args]
    }
}

namespace eval showDefinition {
    variable ns [namespace current]

    namespace ensemble create

    namespace export Init CreateScrolledTablelist

    # The following variables must be set, before reading parameters and
    # before calling LoadSettings.
    proc Init {} {
        variable sApw

        set sApw(tw)      ".apwDef" ; # Name of toplevel window
        set sApw(appName) "apwDef"  ; # Name of tool
        set sApw(cfgDir)  ""        ; # Directory containing config files

        set sApw(startDir) [pwd]
        set sApw(dir1)     $sApw(startDir)
        set sApw(dir2)     $sApw(startDir)

        set sApw(infoWinList) {}         ; # Start with empty file info window list
        set sApw(leftFile)    ""         ; # Left  file for selective diff
        set sApw(rightFile)   ""         ; # Right file for selective diff
        set sApw(curSession)  "Default"  ; # Default session name
        set sApw(sessionList) [list]
        set sApw(curListbox)  ""

        # Default values for command line options.
        set sApw(optSync)             false
        set sApw(optSyncDelete)       false
        set sApw(optCopyDate)         false
        set sApw(optCopyDays)         ""
        set sApw(optSearch)           false
        set sApw(optConvert)          false
        set sApw(optConvertFmt)       ""
        set sApw(optDiffOnStartup)    false
        set sApw(optSessionOnStartup) ""
        # Add command line options which should not be expanded by file matching.
#        apwApps AddFileMatchIgnoreOption "filematch"
    }

    proc CreateScrolledTablelist { fr title } {
        set id [apwWin CreateScrolledTablelist $fr true $title \
                    -columns {50 "col1"   "left"
                               0 "col2"   "right"
                               0 "col3" "right" } \
                    -exportselection false \
                    -stretch 0 \
                    -stripebackground #e0e8f0 \
                    -selectmode extended \
                    -labelcommand tablelist::sortByColumn \
                    -showseparators true]
        $id columnconfigure 0 -sortmode dictionary
        $id columnconfigure 1 -sortmode integer
        $id columnconfigure 2 -sortmode dictionary
        return $id
    }
}

source autoscroll.tcl

proc buildAPListWidget {} {
  apwWin Init
  showDefinition Init
  frame .tbl -width 600 -height 100
  set tableFr .tbl
  set tableId [showDefinition CreateScrolledTablelist $tableFr showDefinition]
  set lst [list 0 ssid 0 rssi]
  set valueLst [list]
  set rowLst [list]
  set row 0

set ::APTableId $tableId
  $tableId configure -width 100
  $tableId configure -columns $lst
  foreach rowLst $valueLst {
    $tableId insert end $rowLst
  }
  pack $tableFr
}

proc showApList {def handle2 lst} {
  apwWin Init
  showDefinition Init
  frame .tbl -width 600 -height 100
  set tableFr .tbl
  set tableId [showDefinition CreateScrolledTablelist $tableFr showDefinition]
  set lst [list]
  set valueLst [list]
  set rowLst [list]
  set row 0
if {0} {
  set result [::structmsg get_definitionTableFieldNames $def tableFieldNames]
  checkErrOK $result
  set result [::structmsg get_definitionNumTableRows $def numTableRows]
  checkErrOK $result
  set result [::structmsg get_definitionNumTableRowFields $def numTableRowFields]
  checkErrOK $result
  set row 0
  while {$row < $numTableRows} {
    foreach name $tableFieldNames {
      structmsg get_definitionTableFieldInfo $def $name 0 fieldInfo
      if {$row == 0} {
        lappend lst 0 $name
      }
      set result [structmsg get_tableFieldValue $handle2 $name $row value]
      lappend rowLst $value
    }
    lappend valueLst $rowLst
    set rowLst [list]
    incr row
  }
}
  $tableId configure -width 100
  $tableId configure -columns $lst
  foreach rowLst $valueLst {
    $tableId insert end $rowLst
  }
  pack $tableFr
}

set result [::structmsg dataView setData "" 0]
set result [::structmsg structmsgIdentify structmsgIdentifyInit]
checkErrOk $result
set result [::structmsg structmsgDispatcher createDispatcher handle]
checkErrOk $result

set ::received [dict create]
dict set ::received partsFlags [list]
dict set ::received fromPart 22272
dict set ::received toPart 16640
dict set ::received totalLgth 22
dict set ::received u8CmdKey 0
dict set ::received u16CmdKey 16708
dict set ::received u8CmdLgth 0
dict set ::received u16CmdLgth 0
dict set ::received targetPart 0
dict set ::received fieldOffset 0
dict set ::received lgth 22
dict set ::received buf ""

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
      foreach val [split $msg ""] {
        puts stderr "val: $val!"
      }
puts stderr "need handler for received MSG!lgth: [string length $msg]!"
      binary scan $msg cccccc ch0 ch1 ch2 ch3 ch4 ch5
      binary scan $msg SSS totalLgth defLgth msgLgth
puts stderr "totalLgth!$totalLgth!defLgth!$defLgth!msgLgth!$msgLgth!"
      set defData [string range $msg 6 [expr {6 + $defLgth - 1}]]
      set msgData [string range $msg [expr {6 + $defLgth}] [expr {$totalLgth - 1}]]
puts stderr "defDataLgth: [string length $defData]!"
puts stderr "msgDataLgth: [string length $msgData]!"
      ::structmsg structmsgIdentify structmsgIdentifyReset
      ::structmsg dataView setData "" 0
      ::structmsg structmsgIdentify structmsgIdentify handleReceivedPart $defData $defLgth

if {1} {
      ::structmsg structmsgIdentify structmsgIdentifyReset
      ::structmsg dataView setData "" 0
      ::structmsg structmsgIdentify structmsgIdentify handleReceivedPart $msgData $msgLgth
}
    }
  }
}

proc dumpBinay {data} {
  set dbgBuf ""
  foreach ch [split $data ""] {
    binary scan $ch c pch
    append dbgBuf [format " 0x%02x" [expr {$pch & 0xFF}]]
  }
  puts stderr "dump data!lgth: [string length $data]!data: $dbgBuf!"
}

proc getAPInfos { sock } {
#  puts stderr "[::websocket::conninfo $sock type] from [::websocket::conninfo $sock sockname] to [::websocket::conninfo $sock peername]"
  set result [::structmsg structmsgIdentify sendEncryptedMsg $sock $::received A]
  checkErrOk $result
  $::startBtn configure -text "Quit" -command [list exit 0]
}

set clientSocket [::websocket::open "ws://${host}:${PORT}${path}" ::clientHandler] 
puts stderr "===clientSocket: $clientSocket"

buildAPListWidget
set startBtn [::ttk::button .start -text "Start" -command [list getAPInfos $::clientSocket]]
pack $startBtn


vwait forever
