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

set ::COMP_DISP_ERR_OPEN_FILE             189
set ::COMP_DISP_FILE_NOT_OPENED           188
set ::COMP_DISP_ERR_FLUSH_FILE            187
set ::COMP_DISP_ERR_WRITE_FILE            186
set ::COMP_DISP_ERR_BAD_RECEIVED_LGTH     185
set ::COMP_DISP_ERR_BAD_FILE_CONTENTS     184
set ::COMP_DISP_ERR_HEADER_NOT_FOUND      183
set ::COMP_DISP_ERR_DUPLICATE_FIELD       182
set ::COMP_DISP_ERR_BAD_FIELD_NAME        181
set ::COMP_DISP_ERR_BAD_HANDLE_TYPE       180
set ::COMP_DISP_ERR_INVALID_BASE64_STRING 179
set ::COMP_DISP_ERR_TOO_FEW_FILE_LINES    178
set ::COMP_DISP_ERR_ACTION_NAME_NOT_FOUND 177
set ::COMP_DISP_ERR_DUPLICATE_ENTRY       176

set ::cryptKey "a1b2c3d4e5f6g7h8"

set RECEIVED_CHECK_HEADER_SIZE 7

set ::DISP_FLAG_SHORT_CMD_KEY    1
set ::DISP_FLAG_HAVE_CMD_LGTH    2
set ::DISP_FLAG_IS_ENCRYPTED     4
set ::DISP_FLAG_IS_TO_WIFI_MSG   8
set ::DISP_FLAG_IS_FROM_MCU_MSG  16

set ::RECEIVED_CHECK_TO_SIZE            2
set ::RECEIVED_CHECK_FROM_SIZE          4
set ::RECEIVED_CHECK_TOTAL_LGTH_SIZE    6
set ::RECEIVED_CHECK_SHORT_CMD_KEY_SIZE 7
set ::RECEIVED_CHECK_CMD_KEY_SIZE       8
set ::RECEIVED_CHECK_CMD_LGTH_SIZE      10


set ::moduleFilesPath $::env(HOME)/bene-nodemcu-firmware/module_image_files
set ::MSG_HEADS_FILE_NAME "CompMsgHeads.txt"

namespace eval compMsg {
  namespace ensemble create

  namespace export compMsgIdentify

  namespace eval compMsgIdentify {
    namespace ensemble create
      
    namespace export compMsgIdentify freeCompMsgDataView getHeaderIndexFromHeaderFields
    namespace export compMsgIdentifyReset compMsgIdentifyInit handleReceivedPart handleReceivedMsg

    # ================================= initHeadersAndFlags ====================================
    
    proc initHeadersAndFlags {} {
      variable dispFlags

      set dispFlags 0
    
#      self->McuPart = 0x4D00
#      self->WifiPart = 0x5700
#      self->AppPart = 0x4100
#      self->CloudPart = 0x4300
      return $::COMP_MSG_ERR_OK
    }
    
    # ================================= resetHeaderInfos ====================================
    
    proc resetHeaderInfos {} {
      variable headerInfos

      dict set headerInfos seqIdx 0
      dict set headerInfos seqIdxAfterStart 0
      dict set headerInfos currPartIdx 0
      return $::COMP_MSG_ERR_OK
    }

    # ================================= nextFittingEntry ====================================
    
    proc nextFittingEntry {compMsgDispatcherVar receivedVar u8CmdKey u16CmdKey} {
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $receivedVar received
    
      set headerInfos [dict get $compMsgDispatcher headerInfos]
      set hdrIdx [dict get $headerInfos currPartIdx]
      set hdr [lindex [dict get $headerInfos headerParts] $hdrIdx]
#puts stderr "nextFittingEntry hdrIdx: $hdrIdx!"
      # and now search in the headers to find the appropriate message
      dict set headerInfos seqIdx [dict get $headerInfos seqIdxAfterStart]
      set found false
      while {$hdrIdx < [dict get $headerInfos numHeaderParts]} {
        set hdr [lindex [dict get $headerInfos headerParts] $hdrIdx]
        if {[dict get $hdr hdrToPart] == [dict get $received toPart]} {
          if {[dict get $hdr hdrFromPart] == [dict get $received fromPart]} {
            if {([dict get $hdr hdrTotalLgth] == [dict get $received totalLgth]) || ([dict get $hdr hdrTotalLgth] == 0)} {
#puts stderr "u8CmdKey!$u8CmdKey!u16CmdKey!$u16CmdKey!"
              if {$u8CmdKey != 0} {
                if {$u8CmdKey == [dict get $received u8CmdKey]} {
                  set found true
#puts stderr "recu8cmdkey![dict get $received u8CmdKey]!"
                  break
                }
              } else {
                if {$u16CmdKey != 0} {
#puts stderr "u16Cmdkey: $u16CmdKey!rec u16CmdKey: [dict get $received u16CmdKey]!"
                  if {$u16CmdKey == [dict get $received u16CmdKey]} {
#puts stderr "found hdrIdx: $hdrIdx!"
                    set found true
                    break
                  }
                } else {
#puts stderr "recu!found!"
                  set found true
                  break
                }
              }
            }
          }
        }
        incr hdrIdx
      }
      if {!$found} {
puts stderr "Fitting header not found!"
        checkErrOK $::COMP_DISP_ERR_HEADER_NOT_FOUND
      }
      dict set headerInfos currPartIdx $hdrIdx
      # next sequence field is extraLgth {skip, we have it in hdr fields}
      dict incr headerInfos seqIdx 1
      # next sequence field is encryption {skip, we have it in hdr fields}
      dict incr headerInfos seqIdx 1
      # next sequence field is handle type {skip, we have it in hdr fields}
      dict incr headerInfos seqIdx 1
      if {[dict get $hdr hdrEncryption] eq "N"} {
        dict lappend received partsFlags COMP_DISP_IS_NOT_ENCRYPTED
        # skip extraLgth, encrypted and handle Type
      } else {
        dict lappend received partsFlags COMP_DISP_IS_ENCRYPTED
      }
#puts stderr "nextFittingEntry found: $found!hdrIdx: $hdrIdx"
      dict set compMsgDispatcher headerInfos $headerInfos
      return $::COMP_MSG_ERR_OK
    }
    
    # ================================= getHeaderIndexFromHeaderFields ====================================
    
    proc getHeaderIndexFromHeaderFields {compMsgDispatcherVar receivedVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $receivedVar received
    
      dict set received fieldOffset 0
      set myHeaderLgth 0
      set headerInfos [dict get $compMsgDispatcher headerInfos]
      dict set headerInfos seqIdx 0
#puts stderr "headerInfos keys: [dict keys $headerInfos]!ll hp: [llength [dict get $headerInfos headerParts]]!"
      set headerSequence [dict get $headerInfos headerSequence]
      set seqIdx 0
      set seqVal [lindex $headerSequence $seqIdx]
      while {$seqIdx < [llength $headerSequence]} {
#puts stderr "seqVal: $seqVal!"
        switch $seqVal {
          COMP_DISP_U16_DST {
            set result [::compMsg dataView getUint16 [dict get $received fieldOffset] value]
#puts stderr "dst!$value!"
            dict set received toPart $value
            checkErrOK $result
            dict incr received fieldOffset 2
          }
          COMP_DISP_U16_SRC {
            set result [::compMsg dataView getUint16 [dict get $received fieldOffset] value]
#puts stderr "src!$value!"
            dict set received fromPart $value
            checkErrOK $result
            dict incr received fieldOffset 2
          }
          COMP_DISP_U16_SRC_ID {
            set result [::compMsg dataView getUint16 [dict get $received fieldOffset] value]
            dict set received srcId $value
            checkErrOK $result
            dict incr received fieldOffset 2
          }
          COMP_DISP_U16_TOTAL_LGTH {
            set result [::compMsg dataView getUint16 [dict get $received fieldOffset] value]
            dict set received totalLgth $value
            checkErrOK $result
            dict incr received fieldOffset 2
          }
          COMP_DISP_U8_VECTOR_GUID {
            set result [::compMsg dataView getUint8Vector [dict get $received fieldOffset] value $::GUID_LGTH]
            dict set received GUID $value
            checkErrOK $result
            dict incr received fieldOffset $::GUID_LGTH
          }
          COMP_DISP_U8_VECTOR_HDR_FILLER {
            set lgth [expr {[dict get $headerInfos headerLgth] - [dict get $received fieldOffset]}]
            set result [::compMsg dataView getUint8Vector [dict get $received fieldOffset] value $lgth]
            dict set received hdrFiller $value
            checkErrOK $result
            dict incr received fieldOffset $lgth
          }
          default {
            error "funny seqVal: $seqVal!"
          }
        }
        dict incr headerInfos seqIdx 1
        incr seqIdx
        set seqVal [lindex $headerSequence $seqIdx]
      }
      dict set headerInfos seqIdxAfterStart [dict get $headerInfos seqIdx]
      dict set headerInfos currPartIdx 0
      dict set compMsgDispatcher headerInfos $headerInfos
      set result [nextFittingEntry compMsgDispatcher received 0 0]
      checkErrOK $result
      return $::COMP_MSG_ERR_OK
    }
    
    # ================================= handleReceivedHeader ====================================
    
    proc handleReceivedHeader {compMsgDispatcherVar receivedVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $receivedVar received
     
      set headerInfos [dict get $compMsgDispatcher headerInfos]
puts stderr "+++handleReceivedHeader"
puts stderr "totalLgth: [dict get $received totalLgth]!"
      set u8TotalCrc false
      set u16TotalCrc false
#      set set received compMsgDataView = newCompMsgDataView(received->buf, received->totalLgth);
      set hdrIdx [dict get $headerInfos currPartIdx]
      set headerParts [dict get $headerInfos headerParts]
      set hdr [lindex $headerParts $hdrIdx]

      dict set received lgth [dict get $headerInfos headerLgth]
# we loop over the fieldSequence entries and handle them as needed
# attention not all entries of the message are handled here, only some special entries!
 
#::compMsg compMsgMsgDesc dumpHeaderPart $hdr
      # check if we have a U8_TOTAL_CRC or a U16_TOTAL_CRC or no TOTAL_CRC
      set idx 0
      set fieldSequence [dict get $hdr fieldSequence]
      set sequenceEntry [lindex $fieldSequence $idx]
      while {$sequenceEntry ne [list]} {
        if {$sequenceEntry eq "COMP_DISP_U8_TOTAL_CRC"} {
          set u8TotalCrc true
        }
        if {$sequenceEntry eq "COMP_DISP_U16_TOTAL_CRC"} {
          set u16TotalCrc true
        }
        incr idx
        set sequenceEntry [lindex $fieldSequence $idx]
      }
      set result $::COMP_MSG_ERR_OK
      dict set headerInfos seqIdx 0
#puts stderr "seqIdx: [dict get $headerInfos seqIdx]!"
      set sequenceEntry [lindex $fieldSequence [dict get $headerInfos seqIdx]]
      while {$sequenceEntry ne [list]} {
        dict set received fieldOffset [dict get $headerInfos headerLgth]
        set hdr [lindex $headerParts $hdrIdx]
        switch $sequenceEntry {
          COMP_DISP_U16_CMD_KEY {
            set result [::compMsg dataView getUint16 [dict get $received fieldOffset] u16]
puts stderr ">>u16: $u16!"
            dict set received u16CmdKey $u16
            dict incr received fieldOffset 2
            dict lappend received partsFlags COMP_DISP_U16_CMD_KEY
            set val [dict get $received u16CmdKey]
            set receivedCmdKey [format "%c%c" [expr {($val >> 8) & 0xFF}] [expr {$val & 0xFF}]]
puts stderr "u16cmdkey: $receivedCmdKey![dict get $hdr hdrU16CmdKey]!"
            while {$receivedCmdKey ne [dict get $hdr hdrU16CmdKey]} {
              dict incr headerInfos currPartIdx
#              set result [nextFittingEntry compMsgDispatcher received 0 $receivedCmdKey]
              set result [nextFittingEntry compMsgDispatcher received 0 $val]
              checkErrOK $result
              set hdrIdx [dict get $headerInfos currPartIdx]
              set hdr [lindex $headerParts $hdrIdx]
            }
          }
          COMP_DISP_U0_CMD_LGTH {
            dict incr received fieldOffset 2
#puts stderr "u0CmdLgth!0!"
          }
          COMP_DISP_U16_CMD_LGTH {
            set result [::compMsg dataView getUint16 [dict get $received fieldOffset] u16]
            dict set received u16CmdLgth $u16
            dict incr received fieldOffset 2
#puts stderr [format "u16CmdLgth!0x%04x!" [dict get $received u16CmdLgth]]
          }
          COMP_DISP_U0_CRC {
#puts stderr "u0Crc!0!"
            set result $::COMP_MSG_ERR_OK
          }
          COMP_DISP_U8_CRC {
            set fieldInfo [dict create]
            dict set fieldInfo fieldLgth 1
puts stderr "totalLgth: [dict get $received totalLgth]!"
            dict set fieldInfo fieldOffset [expr {[dict get $received totalLgth]}]
            if {[lsearch [dict get $hdr hdrFlags] COMP_DISP_TOTAL_CRC] >= 0} {
              if {$u8TotalCrc} {
                dict incr fieldInfo fieldOffset -2
              } else {
                dict incr fieldInfo fieldOffset -1
              }
            }
            set startOffset [dict get $headerInfos headerLgth]
            set fieldOffset [dict get $fieldInfo fieldOffset]
#            set size [expr {$fieldOffset - $startOffset}]
            set size $fieldOffset
            set result [::compMsg compMsgDataView getCrc $fieldInfo crcVal $startOffset $size]
#puts stderr [format "u8Crc!res!%d!" $result]
          }
          COMP_DISP_U16_CRC {
            set fieldInfo [dict create]
            dict set fieldInfo fieldLgth 2
            dict set fieldInfo fieldOffset [expr {[dict get $received totalLgth] - 2}]
            if {[lsearch [dict get $hdr hdrFlags] COMP_DISP_TOTAL_CRC] >= 0} {
              if {$u8TotalCrc} {
                dict incr fieldInfo fieldOffset -1
              } else {
                dict incr fieldInfo fieldOffset -2
              }
            }
            set startOffset [dict get $headerInfos headerLgth]
            set size [dict get $fieldInfo fieldOffset]
            set result [::compMsg compMsgDataView getCrc $fieldInfo crcVal $startOffset [dict get $fieldInfo fieldOffset]]
#puts stderr [format "u16Crc!res!%d!" $result]
          }
          COMP_DISP_U0_TOTAL_CRC {
#puts stderr "u0TotalCrc!0!"
            set result $COMP_MSG_ERR_OK
          }
          COMP_DISP_U8_TOTAL_CRC {
            set fieldInfo [dict create]
            dict set fieldInfo fieldLgth 1
            dict set fieldInfo fieldOffset [expr {[dict get $received totalLgth] - 1}]
#puts stderr [format "u8TotalCrc!fieldOffset: %d" [dict get $fieldInfo fieldOffset]]
            # the totalCrc only fits if not yet decrypted and we have decrypted here!!
            set result $::COMP_MSG_ERR_OK
#            set result [::compMsg compMsgDataView getTotalCrc $fieldInfo totalCrc]
#puts stderr [format "u8totalCrc!res!%d!" $result]
#puts stderr [format "u8totalCrc: currPartIdx: %d" [dict get $headerInfos currPartIdx]]
          }
          COMP_DISP_U16_TOTAL_CRC {
            set fieldInfo [dict create]
            dict set fieldInfo fieldLgth 2
            dict set fieldInfo fieldOffset [expr {[dict get $received totalLgth] - 2}]
            set result $::COMP_MSG_ERR_OK
#            set result [::compMsg compMsgDataView getTotalCrc $fieldInfo totalCrc]
puts stderr [format "u16TotalCrc!res!%d!" $result]
          }
        }
        checkErrOK $result
        dict incr headerInfos seqIdx
        set sequenceEntry [lindex $fieldSequence [dict get $headerInfos seqIdx]]
      }
      dict set compMsgDispatcher headerInfos $headerInfos
  # free all space of received message
#puts stderr "call deleteMsg"
#  ::compMsg compMsgData deleteMsg compMsgDispatcher
#puts stderr "received msg deleted"
      return $::COMP_MSG_ERR_OK
    }

    # ================================= handleReceivedMsg ====================================
    
    proc handleReceivedMsg {compMsgDispatcherVar receivedVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $receivedVar received
     
puts stderr "+++handleReceivedMsg!"
puts stderr "totalLgth: [dict get $received totalLgth]!"
      set headerInfos [dict get $compMsgDispatcher headerInfos]
#puts stderr "buf len: [string length [dict get $received buf]]!len: [dict get $received lgth]!"
#puts stderr "compMsgIdentify1 setData hdrIdx: [dict get $headerInfos currPartIdx]"
      set result [::compMsg dataView setData [dict get $received buf] [dict get $received lgth]]
#::compMsg dataView dumpBinary [dict get $received buf] [dict get $received lgth] "ReceivedMsg"
      checkErrOK $result
      set result [handleReceivedHeader compMsgDispatcher received]
      set headerInfos [dict get $compMsgDispatcher headerInfos]
      set hdrIdx [dict get $headerInfos currPartIdx]
      set headerParts [dict get $headerInfos headerParts]
      set hdr [lindex $headerParts $hdrIdx]
      set result [::compMsg compMsgMsgDesc getMsgPartsFromHeaderPart compMsgDispatcher $hdr handle]
      set compMsgData [dict get $compMsgDispatcher compMsgData]
#puts stderr "numMsgDescParts: [dict get $compMsgDispatcher compMsgMsgDesc numMsgDescParts]!"
      set result [::compMsg compMsgData createMsg compMsgData [dict get $compMsgDispatcher compMsgMsgDesc numMsgDescParts] handle]
#puts stderr "Msg handle: $handle!result: $result!"
      checkErrOK $result
      dict set compMsgDispatcher compMsgData $compMsgData
      set idx 0
      while {$idx < [dict get $compMsgDispatcher compMsgMsgDesc numMsgDescParts]} {
        set msgDescParts [dict get $compMsgDispatcher compMsgData msgDescParts]
        set msgDescPart [lindex $msgDescParts $idx] 
        set result [::compMsg compMsgData addField compMsgData [dict get $msgDescPart fieldNameStr] [dict get $msgDescPart fieldTypeStr] [dict get $msgDescPart fieldLgth]]
        checkErrOK $result
        incr idx
      }
      dict set compMsgDispatcher compMsgData $compMsgData
      set result [::compMsg compMsgData initReceivedMsg compMsgDispatcher]
      checkErrOK $result
#puts stderr "handleReceivedMsg done"
#::compMsg compMsgData dumpMsg compMsgDispatcher
      return $::COMP_MSG_ERR_OK
    }

    # ================================= handleReceivedPart ====================================
    
    proc handleReceivedPart {compMsgDispatcherVar buffer lgth} {
      upvar $compMsgDispatcherVar compMsgDispatcher
      variable headerInfos
      variable received
      
      set idx 0
puts stderr "+++handleReceivedPart: lgth: $lgth!"
      set headerInfos [dict get $compMsgDispatcher headerInfos]
#puts stderr "==headerInfos![dict keys [dict get $compMsgDispatcher headerInfos]]!"
#puts stderr "==compMsgDispatcher![dict keys $compMsgDispatcher]!"
#puts stderr "==bufferl:[string length $buffer]!$lgth!"
#puts stderr "received!$received!"
      while {$idx < $lgth} {
        if {![dict exists $received totalLgth]} {
          dict set received totalLgth -999
        }
        set ch [string range $buffer $idx $idx]
        dict append received buf $ch
        dict incr received lgth 1
        dict incr received realLgth 1
        ::compMsg dataView appendData $ch 1
#puts stderr "rec l: [dict get $received lgth]!hdr l: [dict get $headerInfos headerLgth]!"
        if {[dict get $received lgth] == [dict get $headerInfos headerLgth]} {
          set result [getHeaderIndexFromHeaderFields compMsgDispatcher received]
        }
        # loop until we have full message then decrypt if necessary and then handle the message
        if {[dict get $received lgth] == [dict get $received totalLgth]} {
          set hdrIdx [dict get $headerInfos currPartIdx]
          set headerParts [dict get $headerInfos headerParts]
          set hdr [lindex $headerParts $hdrIdx]
          if {[dict get $hdr hdrEncryption] eq "E"} {
            set myHeaderLgth [dict get $headerInfos headerLgth]
            set myHeader [string range $buffer 0 [expr {$myHeaderLgth - 1}]]
            set mlen [expr {$lgth - $myHeaderLgth}]
            set totalCrcOffset 0
            set totalCrc ""
            if {[lsearch [dict get $hdr hdrFlags] COMP_DISP_TOTAL_CRC] >= 0} {
              if {[lsearch [dict get $hdr fieldSequence] COMP_DISP_U8_TOTAL_CRC] >= 0} {
                set totalCrcOffset 1
                incr mlen -1
                set totalCrc [string range $buffer end end]
              } else {
                set totalCrc [string range $buffer end-1 end]
                incr totalCrcOffset 2
                incr mlen -2
              }
            }
            set endIdx [expr {$lgth - $totalCrcOffset - 1}]
#puts stderr "lgth: $lgth endIdx: $endIdx!totalCrcOffset: $totalCrcOffset!myHeaderLgth: $myHeaderLgth!"
            set crypted [string range $buffer $myHeaderLgth $endIdx]
#puts stderr "cryptedLgth: [string length $crypted]!mlen: $mlen!"
#::compMsg dataView dumpBinary $crypted $mlen "crypted msg"
            set cryptKey "a1b2c3d4e5f6g7h8"
            set result [::compMsg compMsgDispatcher decryptMsg $crypted $mlen $cryptKey 16 $cryptKey 16 decrypted decryptedLgth]
#puts stderr "decryptedLgth: $decryptedLgth!result!$result!"
            if {$result != $::COMP_MSG_ERR_OK} {
puts stderr "decrypt error"
            }
            set buffer "${myHeader}${decrypted}${totalCrc}"
puts stderr "buffer ll: [string length $buffer]!lgth: $lgth!myh: [string length $myHeader]!decr: [string length $decrypted]!totalcrc: [string length $totalCrc]!"
            if {$lgth != [expr {$myHeaderLgth + $mlen + $totalCrcOffset}]} {
error "=== ERROR lgth!$lgth != mhl+mlen: [expr {$myHeaderLgth + $mlen + $totalCrcOffset}]!"
            }
            dict set received buf $buffer
          }
          set result [::compMsg compMsgIdentify handleReceivedMsg compMsgDispatcher received]
          checkErrOK $result
          return $result
        }
        incr idx
      }
      return $::COMP_MSG_ERR_OK
    }
    
    # ================================= compMsgIdentifyReset ====================================
    
    proc compMsgIdentifyReset {} {
      variable received
      variable headerInfos

      set result [::compMsg compMsgDispatcher resetMsgInfo received]
      checkErrOK $result
#      set headerInfos [dict create]
      set received [dict create]
      dict set received buf ""
      dict set received lgth 0
      set dispFlags [list]
      resetHeaderInfos
      return $::COMP_MSG_ERR_OK
    }
    
    # ================================= compMsgIdentifyInit ====================================
    
    proc compMsgIdentifyInit {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher

      set result [initHeadersAndFlags]
      checkErrOK $result
      set result [::compMsg compMsgMsgDesc readHeadersAndSetFlags compMsgDispatcher ${::moduleFilesPath}/$::MSG_HEADS_FILE_NAME]
      checkErrOK $result
      return $::COMP_MSG_ERR_OK
    }
    
  } ; # namespace compMsgIdentify
} ; # namespace compMsg
