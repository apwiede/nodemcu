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

#package require websocket
package require aes
#package require tls
#tls::init -tls1 1 ;# forcibly activate support for the TLS1 protocol

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

# ================================ sendMcuData ===============================

proc sendMcuData {} {
  set result [::compMsg compMsgMsgDesc getHeaderFromUniqueFields 17152 19712 CD hdr]
  checkErrOK $result
puts stderr "===after getHeaderFromUniqueFields"
#  set ::compMsgDispatcher [dict create]
  set result [::compMsg compMsgDispatcher createMsgFromHeaderPart ::compMsgDispatcher $hdr handle]
  checkErrOK $result
}

# ================================ getStatisticData ===============================

proc getStatisticData {compMsgDispatcherVar} {
  upvar $compMsgDispatcherVar compMsgDispatcher

  set msgDescPart [dict get $compMsgDispatcher msgDescPart]
  set lst [list statistic1 statistic2 statistic3 statistic4]
  dict set msgDescPart fieldSize [string length "[join $lst \x0]\x0"]
  dict set msgDescPart fieldKey 111
  dict set compMsgDispatcher msgDescPart $msgDescPart
  return $::COMP_MSG_ERR_OK
}

# ================================ getCasingData ===============================

proc getCasingData {compMsgDispatcherVar} {
  upvar $compMsgDispatcherVar compMsgDispatcher

  set msgDescPart [dict get $compMsgDispatcher msgDescPart]
  set lst [list casing1 casing2 casing3 casing4]
  dict set msgDescPart fieldSize [string length "[join $lst \x0]\x0"]
  dict set msgDescPart fieldKey 112
  dict set compMsgDispatcher msgDescPart $msgDescPart
  return $::COMP_MSG_ERR_OK
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

# ================================ getStatisticValues ===============================

proc getStatisticValues {compMsgDispatcherVar} {
  upvar $compMsgDispatcherVar compMsgDispatcher

  set msgDescPart [dict get $compMsgDispatcher msgDescPart]
  set msgValPart [dict get $compMsgDispatcher msgValPart]
  set lst [list statistic1 statistic2 statistic3 statistic4]
  set saveData $::compMsg::dataView::data
  set saveLgth $::compMsg::dataView::lgth
  set fieldSize [dict get $msgDescPart fieldSize]
  set str [string repeat " " $fieldSize]
  set result [::compMsg dataView setData $str $fieldSize]
  set offset 0
  set result [::compMsg dataView setUint16 $offset [dict get $msgDescPart fieldKey]]
  incr offset 2
  set result [::compMsg dataView setUint8 $offset 7]
  incr offset 1
  set result [::compMsg dataView setUint16 $offset [expr {[dict get $msgDescPart fieldSize] - (2 * 2 + 1)}]]
  incr offset 2
  set valueStr "[join $lst \x0]\x0"
  set result [::compMsg dataView setUint8Vector $offset $valueStr [string length $valueStr]]
  dict set msgValPart fieldKeyValueStr $::compMsg::dataView::data
  dict set msgValPart fieldValue $::compMsg::dataView::data
  dict set compMsgDispatcher msgValPart $msgValPart
  set result [::compMsg dataView setData $saveData $saveLgth]
  return $::COMP_MSG_ERR_OK
}

# ================================ getCasingDataValues ===============================

proc getCasingDataValues {compMsgDispatcherVar} {
  upvar $compMsgDispatcherVar compMsgDispatcher

  set msgDescPart [dict get $compMsgDispatcher msgDescPart]
  set msgValPart [dict get $compMsgDispatcher msgValPart]
  set lst [list casing1 casing2 casing3 casing4]
  set saveData $::compMsg::dataView::data
  set saveLgth $::compMsg::dataView::lgth
  set fieldSize [dict get $msgDescPart fieldSize]
  set str [string repeat " " $fieldSize]
  set result [::compMsg dataView setData $str $fieldSize]
  set offset 0
  set result [::compMsg dataView setUint16 $offset [dict get $msgDescPart fieldKey]]
  incr offset 2
  set result [::compMsg dataView setUint8 $offset 7]
  incr offset 1
  set result [::compMsg dataView setUint16 $offset [expr {[dict get $msgDescPart fieldSize] - (2 * 2 + 1)}]]
  incr offset 2
  set valueStr "[join $lst \x0]\x0"
  set result [::compMsg dataView setUint8Vector $offset $valueStr [string length $valueStr]]
  dict set msgValPart fieldKeyValueStr $::compMsg::dataView::data
  dict set msgValPart fieldValue $::compMsg::dataView::data
  dict set compMsgDispatcher msgValPart $msgValPart
  set result [::compMsg dataView setData $saveData $saveLgth]
  return $::COMP_MSG_ERR_OK
}

# ================================ readByte1 ===============================

proc readByte1 {fd bufVar lgthVar} {
  upvar $bufVar buf
  upvar $lgthVar lgth

  set ch [read $fd 1]
  binary scan $ch c pch
puts stderr "ch: $ch!$pch!lgth: $lgth"
  append buf $ch
  incr lgth

}

# ================================ main ===============================

# ================================ InitCompMsg ===============================

set ::compMsgDispatcher [dict create]
set compMsgWifiData [dict create]
set result [::compMsg compMsgDispatcher newCompMsgDispatcher ::compMsgDispatcher]
checkErrOK $result
set result [::compMsg compMsgDispatcher createDispatcher dispatcherHandle]
checkErrOK $result
puts stderr "dispatcherHandle!$dispatcherHandle!"
set result [::compMsg compMsgDispatcher initDispatcher compMsgDispatcher]
checkErrOK $result

set ::fd0 [open "/dev/ttyUSB0" w+]
fconfigure $::fd0 -blocking 0 -translation binary
fconfigure $::fd0 -mode 115200,n,8,1

sendMcuData


vwait forever
