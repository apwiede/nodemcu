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
package require aes
#package require tls
#tls::init -tls1 1 ;# forcibly activate support for the TLS1 protocol

package require Tk
package require tablelist

::websocket::loglevel debug

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

set ::PORT 80
set ::path /getaplist
set ::host "192.168.4.1"

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

# ================================ apWin ===============================

namespace eval apwWin {
    variable ns [namespace current]

    namespace ensemble create

    namespace export CreateScrolledWidget Init CreateScrolledTablelist


    # ================================ Init ===============================

    proc Init {} {
        variable infoWinNo
        variable xPosShowEntryBox
        variable yPosShowEntryBox

        set infoWinNo 1
        set xPosShowEntryBox -1
        set yPosShowEntryBox -1
    }

    # ================================ CreateScrolledWidget ===============================

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

    # ================================ CreateScrolledTablelist ===============================

    proc CreateScrolledTablelist { w useAutoScroll titleStr args } {
        return [CreateScrolledWidget tablelist::tablelist $w $useAutoScroll $titleStr {*}$args]
    }
}

# ================================ showDefinition ===============================

namespace eval showDefinition {
    variable ns [namespace current]

    namespace ensemble create

    namespace export Init CreateScrolledTablelist

    # The following variables must be set, before reading parameters and
    # before calling LoadSettings.

    # ================================ Init ===============================

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

    # ================================ CreateScrolledTablelist ===============================

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

# ================================ buildAPListWidget ===============================

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

# ================================ showApList ===============================

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
  set result [::compMsg get_definitionTableFieldNames $def tableFieldNames]
  checkErrOK $result
  set result [::compMsg get_definitionNumTableRows $def numTableRows]
  checkErrOK $result
  set result [::compMsg get_definitionNumTableRowFields $def numTableRowFields]
  checkErrOK $result
  set row 0
  while {$row < $numTableRows} {
    foreach name $tableFieldNames {
      compMsg get_definitionTableFieldInfo $def $name 0 fieldInfo
      if {$row == 0} {
        lappend lst 0 $name
      }
      set result [compMsg get_tableFieldValue $handle2 $name $row value]
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

# ================================ fillTable ===============================

proc fillTable {} {
  set row 0
  set result [::compMsg compMsgData getFieldValue ::compMsgDispatcher "@numKeyValues" numKeyValues]
  checkErrOK $result
  set result [::compMsg compMsgData getFieldValue ::compMsgDispatcher "#key_ssid"  ssids]
  checkErrOK $result
  set offset 5
  set str [string range $ssids $offset end]
  set ssidFlds [lrange [split $str "\0"] 0 end-1]
  set result [::compMsg compMsgData getFieldValue ::compMsgDispatcher "#key_rssi"  rssis]
  checkErrOK $result
  set str [string range $rssis $offset end]
  set rssiFlds [split $str ""]
  set rssis [list]
  foreach ch $rssiFlds {
    binary scan $ch c pch
    lappend rssis $pch
  }
  set valueLst [list]
  set idx 0
  while {$idx < [llength $rssis]} {
    set ssid [lindex $ssidFlds $idx]
    set rssi [lindex $rssis $idx]
    if {$ssid ne [list]} {
      set rowLst [list $ssid $rssi]
      $::APTableId insert end $rowLst
    }
    incr idx
  }
}

# ================================ clientHandler ===============================

proc clientHandler { sock type msg } {
#puts stderr "clientHandler: $type $msg!"
  switch -glob -nocase -- $type {
    co* {
      puts "===Connected on $sock"
    }
    te* {
      puts "===RECEIVED: $msg"
    }
    cl* -
    dis* {
    }
    binary {
#      puts "===RECEIVED BINARY: $msg"
# ::compMsg compMsgData dumpBinary $msg [string length $msg] "MSG"
puts stderr "need handler for received MSG!lgth: [string length $msg]!"

      set result [::compMsg compMsgIdentify compMsgIdentifyInit ::compMsgDispatcher]
      checkErrOK $result
      set result [::compMsg compMsgIdentify compMsgIdentifyReset]
      checkErrOK $result
      set result [::compMsg dataView setData "" 0]
      checkErrOK $result
set fd [open "AAAnswer.txt" w]
puts $fd $msg
flush $fd
close $fd
::compMsg compMsgData dumpBinary $msg [string length $msg] "MSG_dump crypted"
      set result [::compMsg compMsgIdentify handleReceivedPart ::compMsgDispatcher $msg [string length $msg]]
      checkErrOK $result
fillTable
    }
  }
}

# ================================ getAPInfos ===============================

proc getAPInfos { sock } {
  puts stderr "[::websocket::conninfo $sock type] from [::websocket::conninfo $sock sockname] to [::websocket::conninfo $sock peername]"
  set result [::compMsg compMsgMsgDesc getHeaderFromUniqueFields 22272 16640 AD hdr]
  checkErrOK $result
#puts stderr "===after getHeaderFromUniqueFields"
  set ::compMsgDispatcher [dict create]
  set result [::compMsg compMsgDispatcher setSocketForAnswer ::compMsgDispatcher $sock]
#puts stderr "===after setSocket"
  checkErrOK $result
  set result [::compMsg compMsgDispatcher createMsgFromHeaderPart ::compMsgDispatcher $hdr handle]
  checkErrOK $result
  $::startBtn configure -text "Quit" -command [list exit 0]
}

# ================================ InitCompMsg ===============================

proc InitCompMsg {} {
  set ::compMsgDispatcher [dict create]
  set result [::compMsg compMsgDispatcher newCompMsgDispatcher]
  checkErrOK $result
  set result [::compMsg compMsgDispatcher createDispatcher dispatcherHandle]
  checkErrOK $result
puts stderr "dispatcherHandle!$dispatcherHandle!"
  set result [::compMsg compMsgDispatcher initDispatcher compMsgDispatcher]
  checkErrOK $result
}

# ================================ main ===============================

InitCompMsg

puts stderr "ws://${host}:${PORT}${path}"
set clientSocket [::websocket::open "ws://${host}:${PORT}${path}" ::clientHandler] 
puts stderr "===clientSocket: $clientSocket"

buildAPListWidget
set startBtn [::ttk::button .start -text "Start" -command [list getAPInfos $::clientSocket]]
pack $startBtn


vwait forever
