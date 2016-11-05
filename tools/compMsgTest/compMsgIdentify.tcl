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
      
    namespace export compMsgIdentify freeCompMsgDataView sendEncryptedMsg
    namespace export compMsgIdentifyReset compMsgIdentifyInit handleReceivedPart

    variable headerInfos [list]
    variable received [list]
    variable dispFlags [list]

    set received [dict create]
    dict set received buf ""
    dict set received lgth 0

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

    # ================================= prepareNotEncryptedAnswer ====================================
    
    proc prepareNotEncryptedAnswer {partsVar type} {
      upvar $partsVar parts
    #ets_printf{"§@1@§", parts->u8CmdKey}
      if {[lsearch [dict get $parts partsFlags] COMP_DISP_U8_CMD_KEY] >= 0} {
    #ets_printf{"§@prepareNotEncryptedAnsweru8!%c!t!%c!@§", parts->u8CmdKey, type}
        set fileName [format "%s/Desc%c%c.txt" $::moduleFilesPath [dict get $parts u8CmdKey] $type]
      } else {
    #ets_printf{"§@prepareNotEncryptedAnsweru16!%c%c!t!%c!@§", (parts->u16CmdKey>>8}& 0xFF, parts->u16CmdKey&0xFF, type)
        set fileName [format "%s/Desc%c%c%c.txt" $::moduleFilesPath [expr {([dict get $parts u16CmdKey] >> 8) & 0xFF}] [expr {[dict get $parts u16CmdKey] & 0xFF}] $type]
      }
      set fd [open $fileName "r"]
      get $fd line
      set flds [split $line ","]
      foreach {dummy numEntries} $flds break
    #ets_printf{"§@NE1!%d!@§", numEntries}
      set numRows 0
      set result [createMsgFromLines $parts $numEntries $numRows $type compMsgData handle]
      checkErrOk $result
    #ets_printf{"heap2: %d\n", system_get_free_heap_size(})
      close $fd
      if {[lsearch [dict get $parts partsFlags] COMP_DISP_U8_CMD_KEY] >= 0} {
        set fileName [format "%s/Val%c%c.txt" $::moduleFilesPath [dict get $parts u8CmdKey] $type]
      } else {
        set fileName [format "%s/Val%c%c%c.txt" $::moduleFilesPath [expr {([dict get $parts u16CmdKey] >> 8) & 0xFF}] [expr {[dict get $parts u16CmdKey] & 0xFF}] $type]
      }
      set fd [open $fileName "r"]
      gets $fd line
      set flds [split $line ","]
      foreach {dummy numEntries} $flds break
    #ets_printf{"§@NE2!%d!@§", numEntries}
      set result [::compMsg compMsgData setMsgValuesFromLines $numEntries $handle [dict get $parts u8CmdKey]]
      checkErrOK $result
      close $fd
    #ets_printf{"§heap3: %d§", system_get_free_heap_size(})
      set result [::compMsg compMsgData getMsgData data msgLgth]
      checkErrOK $result
      set result [typeRSendAnswer $data $msgLgth]
      resetMsgInfo parts
      return $::COMP_MSG_ERR_OK
    }
    
    # ================================= prepareEncryptedAnswer ====================================
    
    proc prepareEncryptedAnswer {parts type} {
      if {[lsearch [dict get $parts partsFlags] COMP_DISP_U8_CMD_KEY] >= 0} {
        set fileName [format "%s/Desc%c%c.txt" $::moduleFilesPath [dict get $parts u8CmdKey] $type]
      } else {
        set fileName [format "%s/Desc%c%c.txt" $::moduleFilesPath [expr {([dict get $parts u16CmdKey] >> 8) & 0xFF}] [expr {[dict get $parts u16CmdKey] & 0xFF}]]
      }
      set fd [open $fileName "r"]
      gets $fd line
      set flds [split $line ","]
      foreach {dummy numEntries} $flds break
      set numRows 0
      set result [::compMsg compMsgDispatcher createMsgFromLines $fd $parts $numEntries $numRows $type handle]
      checkErrOK $result
      close $fd
      if {[lsearch [dict get $parts partsFlags] COMP_DISP_U8_CMD_KEY] >= 0} {
        set fileName [format "%s/Val%c%c.txt" $::moduleFilesPath [dict get $parts u8CmdKey] $type]
      } else {
        set fileName [format "%s/Val%c%c.txt" $::moduleFilesPath [expr {([dict get $parts u16CmdKey] >> 8) & 0xFF}] [expr {[dict get $parts u16CmdKey] & 0xFF}]]
      }
      set fd [open $fileName "r"]
      gets $fd line
      set flds [split $line ","]
      foreach {dummy numEntries} $flds break
      set result [::compMsg compMsgDispatcher setMsgValuesFromLines $fd $numEntries $handle [dict get $parts u8CmdKey]]
      checkErrOK $result
      close $fd
      set result [::compMsg compMsgData getMsgData data msgLgth]
      checkErrOK $result
      set result [typeRSendAnswer $data $msgLgth]
      resetMsgInfo parts
      return $::COMP_MSG_ERR_OK
    }
    
    # ================================= sendEncryptedMsg ====================================
    
    proc sendEncryptedMsg {sock parts type} {
      variable headerInfos
      variable received

      if {[lsearch [dict get $parts partsFlags] COMP_DISP_U8_CMD_KEY] >= 0} {
        set fileName [format "%s/Desc%c%c.txt" $::moduleFilesPath [dict get $parts u8CmdKey] $type]
      } else {
        set fileName [format "%s/Desc%c%c.txt" $::moduleFilesPath [expr {([dict get $parts u16CmdKey] >> 8) & 0xFF}] [expr {[dict get $parts u16CmdKey] & 0xFF}]]
      }
      set fd [open $fileName "r"]
      gets $fd line
      set flds [split $line ","]
      foreach {dummy numEntries} $flds break
      set numRows 0
      set result [::compMsg compMsgDispatcher createMsgFromLines $fd $parts $numEntries $numRows $type handle]
      checkErrOK $result
      close $fd
      set result [::compMsg compMsgData addFlag COMP_MSG_CRC_USE_HEADER_LGTH]
      if {[lsearch [dict get $parts partsFlags] COMP_DISP_U8_CMD_KEY] >= 0} {
        set fileName [format "%s/Val%c%c.txt" $::moduleFilesPath [dict get $parts u8CmdKey] $type]
      } else {
        set fileName [format "%s/Val%c%c.txt" $::moduleFilesPath [expr {([dict get $parts u16CmdKey] >> 8) & 0xFF}] [expr {[dict get $parts u16CmdKey] & 0xFF}]]
      }
      set fd [open $fileName "r"]
      gets $fd line
      set flds [split $line ","]
      foreach {dummy numEntries} $flds break
      if {[lsearch [dict get $parts partsFlags] COMP_DISP_U8_CMD_KEY] >= 0} {
        set result [::compMsg compMsgDispatcher setMsgValuesFromLines $fd $numEntries $handle [dict get $parts u8CmdKey]]
      } else {
        set result [::compMsg compMsgDispatcher setMsgValuesFromLines $fd $numEntries $handle [dict get $parts u16CmdKey]]
      }
      checkErrOK $result
      close $fd
      set result [::compMsg compMsgData getMsgData data msgLgth]
      checkErrOK $result
puts stderr "MSG!$msgLgth!$data!"
      set result [getHeaderIndexFromHeaderFields]
      set headerLgth [dict get $headerInfos headerLgth]
      set hdr [lindex [dict get $headerInfos headerParts] [dict get $headerInfos currPartIdx]]
      set extraLgth [dict get $hdr hdrExtraLgth]
      set encryption [dict get $hdr hdrEncryption]
      if {$encryption eq "E"} {
        set encryptionType Encrypted
        set header [string range $data 0 [expr {$headerLgth -1}]]
        set toCrypt [string range $data $headerLgth end]
        set result [::compMsg compMsgDispatcher encryptMsg $toCrypt [string length $toCrypt] $::cryptKey [string length $::cryptKey] $::cryptKey [string length $::cryptKey] encrypted encryptedLgth]
        set data "${header}${encrypted}"
puts stderr "toCrypt![string length $toCrypt]!$toCrypt!"
puts stderr "encrypted![string length $encrypted]!$encrypted!"
        set msgLgth [string length $data]
      } else {
        set encryptionType NotEncrypted
      }
      set handleType [dict get $hdr hdrHandleType]
      set fcnName type${handleType}${encryptionType}SendMsg
puts stderr "call:$fcnName!"
      set result [::compMsg compMsgSendReceive $fcnName $sock $data $msgLgth]
      set result [::compMsg compMsgDispatcher resetMsgInfo received]
      return $::COMP_MSG_ERR_OK
    }
    
    # ================================= nextFittingEntry ====================================
    
    proc nextFittingEntry {u8CmdKey u16CmdKey} {
      variable headerInfos
      variable received
    
      set hdrIdx [dict get $headerInfos currPartIdx]
      set hdr [lindex [dict get $headerInfos headerParts] $hdrIdx]
      # and now search in the headers to find the appropriate message
      dict set headerInfos seqIdx [dict get $headerInfos seqIdxAfterStart]
      set found false
      while {$hdrIdx < [dict get $headerInfos numHeaderParts]} {
        set hdr [lindex [dict get $headerInfos headerParts] $hdrIdx]
        if {[dict get $hdr hdrToPart] == [dict get $received toPart]} {
          if {[dict get $hdr hdrFromPart] == [dict get $received fromPart]} {
            if {([dict get $hdr hdrTotalLgth] == [dict get $received totalLgth]) || ([dict get $hdr hdrTotalLgth] == 0) || ([dict get $hdr hdrTotalLgth] == 1)} {
#puts stderr "u8CmdKey!$u8CmdKey!u16CmdKey!$u16CmdKey!"
              if {$u8CmdKey != 0} {
                if {$u8CmdKey == [dict get $received u8CmdKey]} {
                  set found true
#puts stderr "recu8cmdkey![dict get $received u8CmdKey]!"
                  break
                }
              } else {
                if {$u16CmdKey != 0} {
#puts stderr "recu16cmdkey![dict get $received u16CmdKey]!"
                  if {$u16CmdKey == [dict get $received u16CmdKey]} {
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
puts stderr "Fitting entry not found!"
        checkErrOK $::COMP_MSG_ERR_HANDLE_NOT_FOUND
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
#puts stderr "§found!$found!hdrIdx!$hdrIdx§"
      return $::COMP_MSG_ERR_OK
    }
    
    # ================================= getHeaderIndexFromHeaderFields ====================================
    
    proc getHeaderIndexFromHeaderFields {} {
      variable headerInfos
      variable received
    
      dict set received fieldOffset 0
      set myHeaderLgth 0
      dict set headerInfos seqIdx 0
      set headerSequence [dict get $headerInfos headerSequence]
      switch [lindex $headerSequence [dict get $headerInfos seqIdx]] {
        COMP_DISP_U16_DST {
          set result [::compMsg dataView getUint16 [dict get $received fieldOffset] value]
          dict set received toPart $value
          checkErrOK $result
          dict incr received fieldOffset 2
        }
        COMP_DISP_U16_SRC {
          set result [::compMsg dataView getUint16 [dict get $received fieldOffset] value]
          dict set received fromPart $value
          checkErrOK $result
          dict incr received fieldOffset 2
        }
        COMP_DISP_U8_TARGET {
          set result [::compMsg dataView getUint8 [dict get $received fieldOffset] value]
          dict set received targetPart $value
          checkErrOK $result
          dict incr received fieldOffset 1
        }
      }
      dict incr headerInfos seqIdx 1
      switch [lindex $headerSequence [dict get $headerInfos seqIdx]] {
        COMP_DISP_U16_DST {
          set result [::compMsg dataView getUint16 [dict get $received fieldOffset] value]
          dict set received toPart $value
          checkErrOK $result
          dict incr received fieldOffset 2
        }
        COMP_DISP_U16_SRC {
          set result [::compMsg dataView getUint16 [dict get $received fieldOffset] value]
          dict set received fromPart $value
          checkErrOK $result
          dict incr received fieldOffset 2
        }
        COMP_DISP_U16_TOTAL_LGTH {
          set result [::compMsg dataView getUint16 [dict get $received fieldOffset] value]
          dict set received totalLgth $value
          checkErrOK $result
          dict incr received fieldOffset 2
        }
      }
      if {[dict get $received fieldOffset] < [dict get $headerInfos headerLgth]} {
        dict incr headerInfos seqIdx 1
        switch [lindex $headerSequence [dict get $headerInfos seqIdx]] {
          COMP_DISP_U16_TOTAL_LGTH {
            set result [::compMsg dataView getUint16 [dict get $received fieldOffset] value]
            dict set received totalLgth $value
            checkErrOK $result
            dict incr received fieldOffset 2
          }
        }
      }
      dict incr headerInfos seqIdx 1

      if {[dict get $received fieldOffset] < [dict get $headerInfos headerLgth]} {
        dict incr headerInfos seqIdx 1
        switch [lindex $headerSequence [dict get $headerInfos seqIdx]] {
          COMP_DISP_U8_VECTOR_GUID {
            set result [::compMsg dataView getUint8Vector [dict get $received fieldOffset] $::GUID_LGTH value]
            dict set received GUID $value
            checkErrOK $result
            dict incr received fieldOffset $::GUID_LGTH
          }
        }
      }
      dict incr headerInfos seqIdx 1

      if {[dict get $received fieldOffset] < [dict get $headerInfos headerLgth]} {
        dict incr headerInfos seqIdx 1
        switch [lindex $headerSequence [dict get $headerInfos seqIdx]] {
          COMP_DISP_U16_SRC_ID {
            set result [::compMsg dataView getUint16 [dict get $received fieldOffset] value]
            dict set received srcId $value
            checkErrOK $result
            dict incr received fieldOffset 2
          }
        }
      }
      dict incr headerInfos seqIdx 1

      if {[dict get $received fieldOffset] < [dict get $headerInfos headerLgth]} {
        dict incr headerInfos seqIdx 1
        switch [lindex $headerSequence [dict get $headerInfos seqIdx]] {
          COMP_DISP_U8_VECTOR_HDR_FILLER {
            set result [::compMsg dataView getUint8Vector [dict get $received fieldOffset] $::HDR_FILLER_LGTH  value]
            dict set received hdrFiller $value
            checkErrOK $result
            dict incr received fieldOffset $::HDR_FILLER_LGTH
          }
        }
      }
      dict incr headerInfos seqIdx 1

      dict set headerInfos seqIdxAfterStart [dict get $headerInfos seqIdx]
      dict set headerInfos currPartIdx 0
      set result [nextFittingEntry 0 0]
      checkErrOK $result
      return $::COMP_MSG_ERR_OK
    }
    
    # ================================= handleReceivedMsg ====================================
    
    proc handleReceivedMsg {} {
      variable headerInfos
      variable received
     
      dict set received fieldOffset [dict get $hdrInfos headerLgth]
      set hdrIdx [dict get $headerInfos currPartIdx]
      set headerParts [dict get $headerInfos headerParts]
      set hdr [lindex $headerParts $hdrIdx]
      set fieldSequence [dict get $hdr fieldSequence]
      set seqIdx [dict get $hdrInfos seqIdxAfterStart
      while {[lindex $fieldSequence $seqIdx] ne [list]} {
        set sequenceEntry [lindex $fieldSequence $seqIdx]
        switch $sequenceEntry {
          COMP_DISP_U16_CMD_KEY {
            set result [::compMsg dataView getUint16 [dict get $received fieldOffset] value]
puts stderr "value!$value!"
            dict set received u16CmdKey [format "%c%c" [expr {($value >> 8) & 0xFF}] [expr {$value & 0xFF}]]
            dict incr received fieldOffset 2
            dict lappend received partsFlags COMP_DISP_U16_CMD_KEY
            while {[dict get $received u16CmdKey] ne [dict get $hdr hdrU16CmdKey]} {
puts stderr "u16:[dict get $received u16CmdKey]![dict get $hdr hdrU16CmdKey]!"
              dict incr headerInfos currPartIdx 1
              set result [nextFittingEntry 0 [dict get $received u16CmdKey]]
              checkErrOK $result
              set hdr [lindex [dict get $headerInfos headerParts] [dict get $headerInfos currPartIdx]]
            }
puts stderr "found entry: [dict get $headerInfos currPartIdx]!"
          }
          COMP_DISP_U0_CMD_LGTH {
            dict incr received fieldOffset 2
#puts stderr "§u0CmdLgth!0!§"
          }
          COMP_DISP_U8_CMD_LGTH {
            set result [::compMsg dataView getUint8 [dict get $received fieldOffset] value]
            set received u8CmdLgth $value
            dict incr received fieldOffset 1
#puts stderr [format "§u8CmdLgth!%c!§" [dict get $received u8CmdLgth]]
          }
          COMP_DISP_U16_CMD_LGTH {
            set result [::compMsg dataView getUint16 [dict get $received fieldOffset] value]
            set received u16CmdLgth $value
            dict incr received fieldOffset 2
#puts stderr [format "§u16CmdLgth!%c!§" [dict get $received u16CmdLgth]
          }
          COMP_DISP_U0_CRC {
    #ets_printf{"§u0Crc!0!§"}
            set result $::COMP_MSG_ERR_OK
          } 
          COMP_DISP_U8_CRC {
            dict set fieldInfo fieldLgth 1
            dict set fieldInfo fieldOffset [expr {[dict get $received totalLgth] - 1}]
            set result [::compMsg compMsgDataView getCrc $fieldInfo crc 0 [dict get $fieldInfo fieldOffset]]
    #ets_printf{"§u8Crc!res!%d!§", result}
          } 
          COMP_DISP_U16_CRC {
            dict set fieldInfo fieldLgth 2
            dict set fieldInfo fieldOffset [expr {[dict get $received totalLgth] - 2}]
            set result [::compMsg compMsgDataView getCrc $fieldInfo crc 0 [dict get $fieldInfo fieldOffset]]
    #ets_printf{"§u16Crc!res!%d!§", result}
          } 
        } 
        dict incr headerInfos seqIdx 1
      }
if {0} {
      if {$result != $::COMP_MSG_ERR_OK} {
        set answerType N
      } else {
        set answerType A
      }

}
if {0} {
      set result [runAction answerType]
      checkErrOK $result
      set result [prepareAnswer $answerType]
    #ets_printf{"§res NEA!%d!§", result}
      checkErrOK $result
      set result [resetMsgInfo received]
      checkErrOK $result
}
puts stderr "handleReceivedMsg end"
      return $::COMP_MSG_ERR_OK
    }

    # ================================= handleReceivedPart ====================================
    
    proc handleReceivedPart {compMsgDispatcherVar buffer lgth} {
      upvar $compMsgDispatcherVar compMsgDispatcher
      variable headerInfos
      variable received
      
      set idx 0
      set headerInfos [dict get $compMsgDispatcher headerInfos]
#puts stderr "==headerInfos![dict keys [dict get $compMsgDispatcher headerInfos]]!"
#puts stderr "==compMsgDispatcher![dict keys $compMsgDispatcher]!"
#puts stderr "==bufferl:[string length $buffer]!$lgth!"
      while {$idx < $lgth} {
        if {![dict exists $received totalLgth]} {
          dict set received totalLgth -999
        }
        set ch [string range $buffer $idx $idx]
        dict append received buf $ch
        dict incr received lgth 1
        dict incr received realLgth 1
        ::compMsg dataView appendData $ch 1
        if {[dict get $received lgth] == [dict get $headerInfos headerLgth]} {
          set result [getHeaderIndexFromHeaderFields]
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
            set crypted [string range $buffer $myHeaderLgth end]
#puts stderr "cryptedLgth: [string length $crypted]!"
            set cryptKey "a1b2c3d4e5f6g7h8"
            set result [::compMsg compMsgDispatcher decryptMsg $crypted $mlen $cryptKey 16 $cryptKey 16 decrypted decryptedLgth]
#puts stderr "decryptedLgth: $decryptedLgth!result!$result!"
            if {$result != $::COMP_MSG_ERR_OK} {
#puts stderr "decrypt error"
            }
            set buffer "${myHeader}${decrypted}"
            if {$lgth != [expr {$myHeaderLgth + $mlen}]} {
error "=== ERROR lgth!$lgth != mhl+mlen: [expr {$myHeaderLgth + $mlen}]!"
            }
            set result [::compMsg dataView setData $buffer $lgth]
#::compMsg compMsgData dumpBinary $buffer $lgth "decrypted"
            dict set received buf $buffer
          }
          set result [::compMsg compMsgMsgDesc getMsgPartsFromHeaderPart compMsgDispatcher $hdr handle]
          set compMsgData [dict create]
          set result [::compMsg compMsgData createMsg compMsgData [dict get $compMsgDispatcher compMsgMsgDesc numMsgDescParts] handle]
          checkErrOK $result
          set idx 0
          while {$idx < [dict get $compMsgDispatcher compMsgMsgDesc numMsgDescParts]} {
            set msgDescParts [dict get $compMsgDispatcher msgDescParts]
            set msgDescPart [lindex $msgDescParts $idx] 
            set result [::compMsg compMsgData addField compMsgData [dict get $msgDescPart fieldNameStr] [dict get $msgDescPart fieldTypeStr] [dict get $msgDescPart fieldLgth]]
            checkErrOK $result
            incr idx
          }
          dict set compMsgDispatcher compMsgData $compMsgData
          set result [::compMsg dataView setData [dict get $received buf] $lgth]
          checkErrOK $result
          set result [::compMsg compMsgData initReceivedMsg compMsgDispatcher numTableRows numTableRowFields]
          checkErrOK $result
#::compMsg compMsgData dumpMsg compMsgDispatcher
#puts stderr "===dumpMsg done"


#          set result [::compMsg compMsgIdentify handleReceivedMsg $received $headerInfos]
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
