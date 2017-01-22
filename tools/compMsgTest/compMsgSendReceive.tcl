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

namespace eval compMsg {
  namespace ensemble create

    namespace export compMsgSendReceive

  namespace eval compMsgSendReceive {
    namespace ensemble create
      
    namespace export compMsgSendReceiveInit sendMsg

    variable compMsgActionEntries

    # ================================= uartReceiveCb ====================================
    
    proc uartReceiveCb {compMsgDispatcherVar buffer lgth} {
      upvar $compMsgDispatcherVar compMsgDispatcher
    
      received = &self->received
    #ets_printf{"%c", buffer[0]&0xFF}
      myBuffer = buffer
      if {lgth == 0} {
        # simulate a '0' char!!
        lgth = 1
        myBuffer = buf
      }
      result =self->handleReceivedPart myBuffer, lgth
      checkErrOK $result
      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= typeRSendAnswer ====================================
    
    proc typeRSendAnswer {compMsgDispatcherVar data msgLgth} {
      upvar $compMsgDispatcherVar compMsgDispatcher
    
      idx = 0
      while {idx < msgLgth} {
        platform_uart_send {0, data[idx]}
        idx++
      }
      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= sendMsg ====================================
    
    proc sendMsg {compMsgDispatcherVar msgData msgLgth} {
      upvar $compMsgDispatcherVar compMsgDispatcher
    
      set handleType [dict get $compMsgDispatcher currHdr hdrHandleType]
      set encryption [dict get $compMsgDispatcher currHdr hdrEncryption]
#puts stderr "sendMsg: handleType: $handleType!"
      switch $handleType {
        "A" {
          # Wifi -> App Provisioning
puts stderr "sendMsg A: Wifi -> App Provisioning encryption: $encryption!"
puts stderr [format "wud %s", [dict get $compMsgDispatcher wud]
          if {[dict get $compMsgDispatcher wud] eq [list]} {
            checkErrOK $::COMP_DISP_ERR_NO_WEBSOCKET_OPENED
          }
        }
        "G" {
          # App -> Wifi Provisioning
puts stderr "sendMsg G: App -> Wifi Provisioning encryption: $encryption!"
puts stderr "sending on socket: [dict get $compMsgDispatcher socketForAnswer]!"
          ::websocket::send [dict get $compMsgDispatcher socketForAnswer] binary $msgData
          return $::COMP_DISP_ERR_OK
        }
        "R" {
#puts stderr "sendMsg R: Mcu -> Wifi encryption: $encryption!"
          set fd [dict get $compMsgDispatcher WifiFd]
#puts stderr "Mcu Simulation fd: $fd!lgth: [string length $msgData]!msgData!$msgData!"
          puts -nonewline $fd $msgData
          flush $fd
#puts stderr "sendMsg R: Mcu -> Wifi done!"
          return $::COMP_DISP_ERR_OK
        "S" {
puts stderr "sendMsg S: Wifi Mcu encryption: $encryption!"
          set fd [dict get $compMsgDispatcher WifiFd]
puts stderr "Mcu Simulation fd: $fd!lgth: [string length $msgData]!msgData!$msgData!"
          puts -nonewline $fd $msgData
          flush $fd
puts stderr "sendMsg S: Wifi -> Mcudone!"
          return $::COMP_DISP_ERR_OK
        }
        }
        "U" {
          # temporary for simulating Mcu
          foreach ch [split $msgData ""] {
            puts -nonewline $::fd0 $ch
            flush $::fd0
          }
          set fd [open CDTelegram.txt w]
          fconfigure $fd -translation binary
          puts -nonewline $fd $msgData
          flush $fd
          close $fd
          set b64Msg [binary encode base64 $msgData]
          set fd [open CDTelegrambase64.txt w]
          fconfigure $fd -translation binary
          puts -nonewline $fd $b64Msg
          flush $fd
          close $fd
puts stderr "\nhandleType U (Mcu Simulation) done!"
        }
        "W" {
        }
        "N" {
          # just ignore
          return $::COMP_DISP_ERR_OK
        }
        default {
          checkErrOK $::COMP_DISP_ERR_BAD_HANDLE_TYPE
        }
      }
      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= compMsgSendReceiveInit ====================================
    
    proc compMsgSendReceiveInit {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher
    
      return $::COMP_DISP_ERR_OK
    }

  } ; # namespace compMsgAction
} ; # namespace compMsg
