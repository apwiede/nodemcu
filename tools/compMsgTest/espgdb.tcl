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
#package require tls
#tls::init -tls1 1 ;# forcibly activate support for the TLS1 protocol

package require Tk
package require tablelist

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

# ================================ apwWin ===============================

namespace eval apwWin {
    variable ns [namespace current]

    namespace ensemble create

    namespace export CreateScrolledWidget Init CreateScrolledTablelist CreateScrolledText


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

    # ================================ CreateScrolledText ===============================

    proc CreateScrolledText { w useAutoScroll titleStr args } {
        return [CreateScrolledWidget text $w $useAutoScroll $titleStr {*}$args]
    }


}

# ================================ showDefinition ===============================

namespace eval showDefinition {
    variable ns [namespace current]

    namespace ensemble create

    namespace export Init CreateScrolledTablelist CreateScrolledText

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

    # ================================ CreateScrolledText ===============================

    proc CreateScrolledText { fr title rows } {
        set textId [apwWin CreateScrolledText $fr true $title -wrap word -width 120 -height $rows]
        return $textId
    }

}

source autoscroll.tcl

# ================================ buildidget ===============================

proc buildWidget {} {
puts stderr "buildWidget"
  apwWin Init
  showDefinition Init
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
#  ttk::label ${::infoFr}.passwdl -text Passwd
#  ttk::entry ${::infoFr}.passwd -width 64
#  ttk::label ${::infoFr}.ipAddrl -text "IP: "
#  ttk::label ${::infoFr}.ipAddr -text "_______________"
#  ttk::label ${::infoFr}.portl -text "Port: "
#  ttk::label ${::infoFr}.port -text "_____"
#  pack ${::infoFr}.passwdl ${::infoFr}.passwd ${::infoFr}.ipAddrl ${::infoFr}.ipAddr ${::infoFr}.portl ${::infoFr}.port -side left
  frame .txt -width 600 -height 100
  set txtFr .txt
  set ::textId [showDefinition CreateScrolledText $txtFr showSource 40]
  pack $txtFr -side top
puts stderr "buildWidget end"
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

# ================================ handleTagDouble1 ===============================

proc handleTagDouble1 {w x y} {
  set index [$::textId index @$x,$y]
  foreach {line col} [split $index "."] break
  set addresses [dict get $::currDict lines $line addresses]
  set ::brkPoint [lindex [dict keys $addresses] 0]
puts stderr "::brkPoint: $::brkPoint!"
}

# ================================ main ===============================

# InitCompMsg

puts stderr "buildWidget start"
buildWidget
puts stderr "buildWidget built"

set showDirName "/home/arnulf/apwiede-nodemcu-firmware/app/compMsg"
set showFileName "compMsgAction.c"
set showPath ${showDirName}/${showFileName}
ShowFile $showPath
set debugFileName $::env(HOME)/bene-nodemcu-firmware/syms_apw.txt
set fd [open $debugFileName r]
set ::dbgInfo [dict create]
set lastFileName ""
while {[gets $fd line] >= 0} {
  if {![string match "address: *" $line]} {
    continue
  }
# address: 0x40241509 op_index: 0 filename: /home/arnulf/bene-nodemcu-firmware/app/user/user_main.c line: 68 column: 0 discriminator: 0 
  foreach {dummy1 address dummy2 op_index dummy3 filename dummy4 line dummy5 column dummy6 discriminator} [split $line] break
  set dirName [file dirname $filename]
  set fileName [file tail $filename]
  if {![dict exists $::dbgInfo $fileName]} {
    dict set ::dbgInfo $fileName dirName $dirName
  }
  set fileDict [dict get $::dbgInfo $fileName]
  if {![dict exists $fileDict lines $line]} {
    dict set fileDict lines $line [list]
  }
  if {![dict exists $fileDict lines $line addresses $address]} {
    dict set fileDict lines $line addresses $address discriminator $discriminator
  }
  
  dict set ::dbgInfo $fileName $fileDict
if {$lastFileName ne $fileName} {
#pdict $::dbgInfo
}
   set lastFileName $fileName
}

set ::currDict [dict get $::dbgInfo $showFileName]
set ::dbgLines [dict keys [dict get $::currDict lines]]
foreach dbgLine $::dbgLines {
  $::textId tag add dbgLine $dbgLine.0 $dbgLine.end
}
$::textId tag configure dbgLine -foreground red
$::textId tag bind dbgLine <Button-1> [list handleTagDouble1 %W %x %y]
vwait forever
