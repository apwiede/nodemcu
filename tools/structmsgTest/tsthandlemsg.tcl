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
source structmsgDataView.tcl
source structmsgData.tcl
source structmsgDispatcher.tcl
source structmsgIdentify.tcl
source structmsgSendReceive.tcl

proc checkErrOk {result} {
  if {$result != $::STRUCT_MSG_ERR_OK} {
    puts stderr "result: $result"
  }
}

set result [::structmsg dataView setData "" 0]
set result [::structmsg structmsgIdentify structmsgIdentify structmsgIdentifyInit]
checkErrOk $result
set result [::structmsg structmsgDispatcher createDispatcher handle]
checkErrOk $result

set received [dict create]
dict set received partsFlags [list]
dict set received fromPart 22272
dict set received toPart 16640
dict set received totalLgth 22
dict set received u8CmdKey 0
dict set received u16CmdKey 16708
dict set received u8CmdLgth 0
dict set received u16CmdLgth 0
dict set received targetPart 0
dict set received fieldOffset 0
dict set received lgth 22
dict set received buf ""
set result [::structmsg structmsgIdentify sendEncryptedMsg nosocket $received A]
checkErrOk $result
exit 0
