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
    namespace export compMsgIdentifyReset compMsgIdentifyInit

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
      if {$result != $::COMP_MSG_ERR_OK} {
        return $result
      }
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
      if {$result != $::COMP_MSG_ERR_OK} {
        return $result
      }
      close $fd
    #ets_printf{"§heap3: %d§", system_get_free_heap_size(})
      set result [::compMsg compMsgData getMsgData data msgLgth]
      if {$result != $::COMP_MSG_ERR_OK} {
        return $result
      }
      set result [typeRSendAnswer $data $msgLgth]
      resetMsgInfo parts
      return $::COMP_MSG_ERR_OK
    }
    
    # ================================= prepareEncryptedAnswer ====================================
    
    proc prepareEncryptedAnswer {parts type} {
      if {$result != $::COMP_MSG_ERR_OK} {
        return $result
      }
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
      if {$result != $::COMP_MSG_ERR_OK} {
        return $result
      }
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
      if {$result != $::COMP_MSG_ERR_OK} {
        return $result
      }
      close $fd
      set result [::compMsg compMsgData getMsgData data msgLgth]
      if {$result != $::COMP_MSG_ERR_OK} {
        return $result
      }
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
      if {$result != $::COMP_MSG_ERR_OK} {
        return $result
      }
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
      if {$result != $::COMP_MSG_ERR_OK} {
        return $result
      }
      close $fd
      set result [::compMsg compMsgData getMsgData data msgLgth]
      if {$result != $::COMP_MSG_ERR_OK} {
        return $result
      }
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
puts stderr "nextFitting!hdrIdx!$hdrIdx!"
pdict $received
puts stderr "nextFitting2!"
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
        return $::COMP_MSG_ERR_HANDLE_NOT_FOUND
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
      switch [lindex [dict get $headerInfos headerSequence] [dict get $headerInfos seqIdx]] {
        COMP_DISP_U16_DST {
          set result [::compMsg dataView getUint16 [dict get $received fieldOffset] value]
          dict set received toPart $value
          if {$result != $::COMP_MSG_ERR_OK} {
            return $result
          }
          dict incr received fieldOffset 2
        }
        COMP_DISP_U16_SRC {
          set result [::compMsg dataView getUint16 [dict get $received fieldOffset] value]
          dict set received fromPart $value
          if {$result != $::COMP_MSG_ERR_OK} {
            return $result
          }
          dict incr received fieldOffset 2
        }
        COMP_DISP_U8_TARGET {
          set result [::compMsg dataView getUint8 [dict get $received fieldOffset] value]
          dict set received targetPart $value
          if {$result != $::COMP_MSG_ERR_OK} {
            return $result
          }
          dict incr received fieldOffset 1
        }
      }
      dict incr headerInfos seqIdx 1
      switch [lindex [dict get $headerInfos headerSequence] [dict get $headerInfos seqIdx]] {
        COMP_DISP_U16_DST {
          set result [::compMsg dataView getUint16 [dict get $received fieldOffset] value]
          dict set received toPart $value
          if {$result != $::COMP_MSG_ERR_OK} {
            return $result
          }
          dict incr received fieldOffset 2
        }
        COMP_DISP_U16_SRC {
          set result [::compMsg dataView getUint16 [dict get $received fieldOffset] value]
          dict set received fromPart $value
          if {$result != $::COMP_MSG_ERR_OK} {
            return $result
          }
          dict incr received fieldOffset 2
        }
        COMP_DISP_U16_TOTAL_LGTH {
          set result [::compMsg dataView getUint16 [dict get $received fieldOffset] value]
          dict set received totalLgth $value
          if {$result != $::COMP_MSG_ERR_OK} {
            return $result
          }
          dict incr received fieldOffset 2
        }
      }
      if {[dict get $received fieldOffset] < [dict get $headerInfos headerLgth]} {
        dict incr headerInfos seqIdx 1
        switch [lindex [dict get $headerInfos headerSequence] [dict get $headerInfos seqIdx]] {
          COMP_DISP_U16_TOTAL_LGTH {
            set result [::compMsg dataView getUint16 [dict get $received fieldOffset] value]
            dict set received totalLgth $value
            if {$result != $::COMP_MSG_ERR_OK} {
              return $result
            }
            dict incr received fieldOffset 2
          }
        }
      }
      dict incr headerInfos seqIdx 1
      dict set headerInfos seqIdxAfterStart [dict get $headerInfos seqIdx]
      dict set headerInfos currPartIdx 0
      set result [nextFittingEntry 0 0]
      return $result
    }
    
    # ================================= handleEncryptedPart ====================================
    
    proc handleEncryptedPart {} {
      variable headerInfos
      variable received
     
      set hdrIdx [dict get $headerInfos currPartIdx]
      set hdr [lindex [dict get $headerInfos headerParts] $hdrIdx]
      set isU16CmdKey true
      set isU8CmdKey false
      switch [dict get $hdr hdrHandleType] {
      U -
      W -
      R -
      S {
        error "unexpected hdrHandleType: [dict get $hdr hdrHandleType] in handleEncryptedPart!"
        return $::COMP_DISP_ERR_BAD_HANDLE_TYPE
      }
      G -
      A {
        # got APList!
        set myOffset -1
        while {[dict get $received lgth] <= [dict get $received totalLgth]} {
#puts stderr "§el![dict get $hdr hdrExtraLgth]!enc![dict get $hdr hdrEncryption]!ht![dict get $hdr hdrHandleType]!§"
          if {[dict get $received lgth] == [expr {[dict get $headerInfos headerLgth] + 1}]} {
            # get the cmdKey, we get its type from the header sequence!
            if {[dict get $hdr hdrExtraLgth] > 0} {
              dict set headerInfos extraStartLgth [dict get $hdr hdrExtraLgth]
            } else {
              dict set headerInfos extraStartLgth 0
            }
            set cmdKeyType [lindex [dict get $hdr fieldSequence] [dict get $headerInfos seqIdx]]
            switch $cmdKeyType {
              COMP_DISP_U16_CMD_KEY {
                set myOffset 2
              }
              COMP_DISP_U8_CMD_KEY {
                set myoffset 1
              }
              default {
                error "bad cmdKeyType!$cmdKeyType!"
              }
            }
          }
          if {[dict get $received lgth] == [expr {[dict get $headerInfos headerLgth] + $myOffset}]} {
            switch $cmdKeyType {
              COMP_DISP_U16_CMD_KEY {
puts stderr "look for u16CmdKey!"
                set result [::compMsg dataView getUint16 [dict get $received fieldOffset] value]
puts stderr "value!$value!"
                dict set received u16CmdKey [format "%c%c" [expr {($value >> 8) & 0xFF}] [expr {$value & 0xFF}]]
                dict incr received fieldOffset 2
                dict lappend received partsFlags COMP_DISP_U16_CMD_KEY
                set isU16CmdKey true
#puts stderr [format "§u16CmdKey!0x%04x!§" [dict get $received u16CmdKey]]
puts stderr "received!"
pdict $received
                while {[dict get $received u16CmdKey] ne [dict get $hdr hdrU16CmdKey]} {
puts stderr "u16:[dict get $received u16CmdKey]![dict get $hdr hdrU16CmdKey]!"
                  dict incr headerInfos currPartIdx 1
                  set result [nextFittingEntry 0 [dict get $received u16CmdKey]]
                  if {$result != $::COMP_MSG_ERR_OK} {
                    return $result
                  }
                  set hdr [lindex [dict get $headerInfos headerParts] [dict get $headerInfos currPartIdx]]
                }
puts stderr "found entry: [dict get $headerInfos currPartIdx]!"
              }
              COMP_DISP_U8_CMD_KEY {
                set result [::compMsg dataView getUint8 [dict get $received fieldOffset] value]
                set val [format "%c" $value]
                dict set received u8CmdKey $val
                dict incr received fieldOffset 1
                dict lappend received partsFlags COMP_DISP_U8_CMD_KEY
                set isU16CmdKey false
                set hdr [lindex [dict get $headerInfos headerParts] [dict get $headerInfos currPartIdx]]
                while {[dict get $received u8CmdKey] ne [dict get $hdr hdrU8CmdKey]} {
                  set result [nextFittingEntry [dict get $received u8CmdKey] 0]
                  if {$result != $::COMP_MSG_ERR_OK} {
                    return $result
                  }
                  dict incr headerInfos currPartIdx 1
                  set hdr [lindex [dict get $headerInfos headerParts] [dict get $headerInfos currPartIdx]]
                }
              }
            }
            dict incr headerInfos seqIdx 1
          } else {
            if {[dict get $received lgth] == [expr {[dict get $headerInfos headerLgth] + 2}]} {
              # check if we have a cmdLgth
              switch [lindex [dict get $headerInfos headerSequence] [dict get $headerInfos seqIdx]] {
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
              }
              dict incr headerInfos seqIdx 1
            }
          }
          if {[dict get $received lgth] == [dict get $received totalLgth]} {
    #ets_printf{"§not encrypted message completely receieved!%d!§", received->totalLgth}
            # check if we have a crc and the type of it
            # if we have a crc calculate it for the totalLgth
            set fieldInfo [dict create]
            switch [lindex [dict get $headerInfos headerSequence] [dict get $headerInfos seqIdx]] {
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
            if {$result != $::COMP_MSG_ERR_OK} {
              set answerType N
            } else {
              set answerType A
            }
            if {[lsearch [dict get $received partsFlags] COMP_DISP_U16_CMD_KEY] >= 0} {
              set u16CmdKey [dict get $received u16CmdKey]
              if {$u16CmdKey eq "YY"} {
                set result [::compMsg def newCompMsgDefinition]
                if {$result != $::COMP_MSG_ERR_OK} {
                  return $result
                }
                set result [::compMsg def setDef [dict get $received buf]]
set result [::compMsg def dumpDefFields]
                if {$result != $::COMP_MSG_ERR_OK} {
                  return $result
                }
                set result [::compMsg def createMsgFromDef handle]
                if {$result != $::COMP_MSG_ERR_OK} {
                  return $result
                }
                return $::COMP_MSG_ERR_OK
              }
            }

$::APTableId configure -columns [list 0 ssid 0 rssi]

puts stderr "handleEncryptedPart runAction: $answerType"
set result [::compMsg compMsgData getFieldValue "@tablerows" numTabRows]
puts stderr "numTabRows!$numTabRows!result!$result!"
set result [::compMsg compMsgData getFieldValue "@tablerowfields" numTabFields]
puts stderr "numTabFields!$numTabFields!result!$result!"

  set row 0
  while {$row < $numTabRows} {
    foreach name [list ssid rssi] {
      set result [::compMsg compMsgData getTableFieldValue $name $row value]
puts stderr "LL: $name [string length $value]!$value!!"
      set value [string trimright $value "\0"]
      lappend rowLst $value
    }
    lappend valueLst $rowLst
    set rowLst [list]
    incr row
  }
#puts stderr "valueLst!$valueLst!"

  foreach rowLst $valueLst {
    $::APTableId insert end $rowLst
  }

if {0} {
            set result [runAction answerType]
            if {$result != $::COMP_MSG_ERR_OK} {
              return $result
            }
            set result [prepareNotEncryptedAnswer $answerType]
    #ets_printf{"§res NEA!%d!§", result}
            if {$result != $::COMP_MSG_ERR_OK} {
              return $result
            }
            set result [resetMsgInfo received]
            if {$result != $::COMP_MSG_ERR_OK} {
              return $result
            }
}
          }
          dict incr received lgth 1
        }
      }
      default {
        return $::COMP_DISP_ERR_BAD_HANDLE_TYPE
      }
      }
puts stderr "handleEncryptedPart end"
      return $::COMP_MSG_ERR_OK
    }

    # ================================= handleNotEncryptedPart ====================================
    
    proc handleNotEncryptedPart {} {
      variable headerInfos
      variable received
     
      set hdrIdx [dict get $headerInfos currPartIdx]
      set hdr [lindex [dict get $headerInfos headerParts] $hdrIdx]
      # more than just transfer and we are receiving here, so handle Type must be R or lgth == 1 for list msg!
      if {([dict get $hdr hdrHandleType] ne "R") && ([dict get $hdr hdrTotalLgth] != 1)} {
        return $::COMP_DISP_ERR_BAD_HANDLE_TYPE
      }
puts stderr "§el![dict get $hdr hdrExtraLgth]!enc![dict get $hdr hdrEncryption]!ht![dict get $hdr hdrHandleType]!§"
puts stderr "handleNotEncryptedPart: [dict get $received lgth]![dict get $received totalLgth]!"
      # got eventually listMsg!
      set myOffset -1
      while {[dict get $received lgth] <= [dict get $received totalLgth]} {
        if {[dict get $received lgth] == [expr {[dict get $headerInfos headerLgth] + 1}]} {
          # get the cmdKey, we get its type from the header sequence!
          if {[dict get $hdr hdrExtraLgth] > 0} {
            dict set headerInfos extraStartLgth [dict get $hdr hdrExtraLgth]
          } else {
            dict set headerInfos extraStartLgth 0
          }
          set cmdKeyType [lindex [dict get $hdr fieldSequence] [dict get $headerInfos seqIdx]]
          switch $cmdKeyType {
            COMP_DISP_U16_CMD_KEY {
              set myOffset 2
            }
            COMP_DISP_U8_CMD_KEY {
              set myoffset 1
            }
            default {
              error "bad cmdKeyType!$cmdKeyType!"
            }
          }
        }
        if {[dict get $received lgth] == [expr {[dict get $headerInfos headerLgth] + $myOffset}]} {
puts stderr "getCmdKey!"
          # get the cmdKey, we get its type from the header sequence!
          set hdrIdx [dict get $headerInfos currPartIdx]
          set hdr [lindex [dict get $headerInfos headerParts] $hdrIdx]
          set seqIdx [dict get $headerInfos seqIdx]
          set sequenceInfo [dict get $hdr fieldSequence]
puts stderr "hdrIdx!$hdrIdx!seqIdx!$seqIdx!sequenceInfo!$sequenceInfo!"
          set cmdKeyType [lindex $sequenceInfo $seqIdx]
          switch $cmdKeyType {
            COMP_DISP_U16_CMD_KEY {
              set result [::compMsg dataView getUint16 [dict get $received fieldOffset] value]
puts stderr "VAL!$value!"
              dict set received u16CmdKey [format "%c%c" [expr {($value >> 8) & 0xFF}] [expr {$value & 0xFF}]]
              dict incr received fieldOffset 2
              dict lappend received partsFlags COMP_DISP_U16_CMD_KEY
              set isU16CmdKey true
puts stderr [format "§u16CmdKey!%s!§" [dict get $received u16CmdKey]]
              while {[dict get $received u16CmdKey] ne [dict get $hdr hdrU16CmdKey]} {
                dict incr headerInfos currPartIdx 1
                set result [nextFittingEntry 0 [dict get $received u16CmdKey]]
                if {$result != $::COMP_MSG_ERR_OK} {
                  return $result
                }
                set hdr [lindex [dict get $headerInfos headerParts] [dict get $headerInfos currPartIdx]]
              }
            }
            COMP_DISP_U8_CMD_KEY {
              set result [::compMsg dataView getUint8 [dict get $received fieldOffset] value]
              set val [format "%c" $value]
              dict set received u8CmdKey $val
              dict incr received fieldOffset 1
              dict lappend received partsFlags COMP_DISP_U8_CMD_KEY
              set isU16CmdKey false
              set hdr [lindex [dict get $headerInfos headerParts] [dict get $headerInfos currPartIdx]]
              while {[dict get $received u8CmdKey] ne [dict get $hdr hdrU8CmdKey]} {
                set result [nextFittingEntry [dict get $received u8CmdKey] 0]
                if {$result != $::COMP_MSG_ERR_OK} {
                  return $result
                }
                dict incr headerInfos currPartIdx 1
                set hdr [lindex [dict get $headerInfos headerParts] [dict get $headerInfos currPartIdx]]
              }
            }
          }
          dict incr headerInfos seqIdx 1
        } else {
          if {[dict get $received lgth] == [expr {[dict get $headerInfos headerLgth] + 2}]} {
puts stderr "check for cmdLgth"
            # check if we have a cmdLgth
            switch [lindex [dict get $headerInfos headerSequence] [dict get $headerInfos seqIdx]] {
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
            }
            dict incr headerInfos seqIdx 1
          }
          # just get the bytes until totalLgth reached
          if {[dict get $received lgth] == [dict get $received totalLgth]} {
    #ets_printf{"§not encrypted message completely receieved!%d!§", received->totalLgth}
            # check if we have a crc and the type of it
            # if we have a crc calculate it for the totalLgth
            set fieldInfo [dict create]
            switch [lindex [dict get $headerInfos headerSequence] [dict get $headerInfos seqIdx]] {
              COMP_DISP_U0_CRC {
    #ets_printf{"§u0Crc!0!§"}
                set result $::COMP_MSG_ERR_OK
              } 
              COMP_DISP_U8_CRC {
                dict set fieldInfo fieldLgth 1
                dict set fieldInfo fieldOffset [expr {[dict get $received totalLgth] - 1}]
                set result [::compMsg compMsgDataView getCrc $fieldInfo crc 0 [dict get $fieldInfo fieldOffset]]
puts stderr [format "§u8Crc!res!%d!§" $result]
              } 
              COMP_DISP_U16_CRC {
                dict set fieldInfo fieldLgth 2
                dict set fieldInfo fieldOffset [expr {[dict get $received totalLgth] - 2}]
                set result [::compMsg compMsgDataView getCrc $fieldInfo crc 0 [dict get $fieldInfo fieldOffset]]
puts stderr [format "§u16Crc!res!%d!§" $result]
              } 
            }
            dict incr headerInfos seqIdx 1
            if {$result != $::COMP_MSG_ERR_OK} {
              set answerType N
            } else {
              set answerType A
            }
puts stderr "===handleNotEncryptedPart runAction: $answerType"
            if {[lsearch [dict get $received partsFlags] COMP_DISP_U16_CMD_KEY] >= 0} {
              if {[dict get $received u16CmdKey] eq "ZZ"} {
                set result [::compMsg listMsg newCompMsgListMsg]
                if {$result != $::COMP_MSG_ERR_OK} {
                  return $result
                }
                set result [::compMsg list setListMsg [dict get $received buf]]
set result [::compMsg list dumpListMsgFields]
                if {$result != $::COMP_MSG_ERR_OK} {
                  return $result
                }
                # need to handle msgs here!!
puts stderr "start to handle list msgs"
                set result [::compMsg listMsg getListMsgFieldValue COMP_MSG_SPEC_FIELD_LIST_MSG_SIZES_SIZE defLgth 0]
                if {$result != $::COMP_MSG_ERR_OK} {
                  return $result
                }
                set result [::compMsg listMsg getListMsgFieldValue COMP_MSG_SPEC_FIELD_LIST_MSG_SIZES_SIZE msgLgth 1]
                if {$result != $::COMP_MSG_ERR_OK} {
                  return $result
                }
                set result [::compMsg listMsg getListMsgFieldValue COMP_MSG_SPEC_FIELD_LIST_MSGS msgs 0]
                if {$result != $::COMP_MSG_ERR_OK} {
                  return $result
                }
puts stderr "defLgth!$defLgth!msgLgth!$msgLgth!msgs![string length $msgs]!"

                # handle def msg
                set result [compMsgIdentifyReset]
puts stderr "res1!$result!"
                if {$result != $::COMP_MSG_ERR_OK} {
                  return $result
                }
                set result [::compMsg dataView setData "" 0]
puts stderr "res2!$result!"
                if {$result != $::COMP_MSG_ERR_OK} {
                  return $result
                }
                set defData [string range $msgs 0 [expr {$defLgth - 1}]]
puts stderr "defLgth!$defLgth!$defData!"
                set result [handleReceivedPart $defData $defLgth]
puts stderr "res3!$result!"
                if {$result != $::COMP_MSG_ERR_OK} {
                  return $result
                }

                # handle msg
                set result [compMsgIdentifyReset]
puts stderr "res4!$result!"
                if {$result != $::COMP_MSG_ERR_OK} {
                  return $result
                }
                set result [::compMsg dataView setData "" 0]
puts stderr "res5!$result!"
                if {$result != $::COMP_MSG_ERR_OK} {
                  return $result
                }
                set msgData [string range $msgs $defLgth end]
                set result [handleReceivedPart $msgData $msgLgth]
puts stderr "res6!$result!"
puts stderr "start displaying!"
                return $result

              }
            }
if {0} {
            set result [runAction answerType]
            if {$result != $::COMP_MSG_ERR_OK} {
              return $result
            }
            set result [prepareNotEncryptedAnswer $answerType]
    #ets_printf{"§res NEA!%d!§", result}
            if {$result != $::COMP_MSG_ERR_OK} {
              return $result
            }
            set result [resetMsgInfo received]
            if {$result != $::COMP_MSG_ERR_OK} {
              return $result
            }
}
          }
        }
        dict incr received lgth 1
      }
      return $::COMP_MSG_ERR_OK
    }
    
    # ================================= handleReceivedPart ====================================
    
    proc handleReceivedPart {buffer lgth} {
      variable headerInfos
      variable received

      
      set idx 0
puts stderr "bufferl:[string length $buffer]!$lgth!"
      while {$idx < $lgth} {
        if {![dict exists $received totalLgth]} {
          dict set received totalLgth -999
        }
        dict append received buf [string range $buffer $idx $idx]
        dict incr received lgth 1
        ::compMsg dataView appendData $buffer $lgth
        if {[dict get $received lgth] == [dict get $headerInfos headerLgth]} {

          set result [getHeaderIndexFromHeaderFields]
          set hdrIdx [dict get $headerInfos currPartIdx]
          set hdr [lindex [dict get $headerInfos headerParts] $hdrIdx]
          if {[dict get $hdr hdrTotalLgth] == 0} {
            # we have a varying lgth, so get it here
            dict set hdr hdrTotalLgth [dict get $received totalLgth]
puts stderr "headers2"
::compMsg compMsgDispatcher dumpHeaderParts $hdr
          }
        }
        if {[dict get $received lgth] == [dict get $received totalLgth]} {
          set hdrIdx [dict get $headerInfos currPartIdx]
          set hdr [lindex [dict get $headerInfos headerParts] $hdrIdx]
          if {[lsearch [dict get $received partsFlags] COMP_DISP_IS_NOT_ENCRYPTED] >= 0} {
            if {[dict get $hdr hdrEncryption] eq "N"} {
puts stderr "§not encrypted message completely receieved![dict get $received totalLgth]!§"
              set result [::compMsg dataView setData [dict get $received buf] [dict get $received totalLgth]]
              dict set received lgth [dict get $headerInfos headerLgth]
              set result [handleNotEncryptedPart]
              return $result
            } else {
pdict $hdr
error "partsFlags is not encrypted and hdrEncryption is E"
            }
          } else {
            if {[dict get $hdr hdrEncryption] eq "E"} {
puts stderr "hdr!rl![dict get $received lgth]!tl![dict get $received totalLgth]!lgth!$lgth!"
              if {[dict get $received lgth] == [dict get $received totalLgth]} {
puts stderr "§encrypted message completely receieved![dict get $received totalLgth]!§"
                if {[lsearch [dict get $received partsFlags] COMP_DISP_IS_NOT_ENCRYPTED] >= 0} {
                } else {
puts stderr "seems to be an encrypted msg!"
                  set myHeaderLgth [expr {[dict get $headerInfos headerLgth] + [dict get $hdr hdrExtraLgth]}]
pdict $hdr
                  if {[dict get $hdr hdrEncryption] eq "E"} {
                    set myHeader [string range $buffer 0 [expr {$myHeaderLgth - 1}]]
                    set mlen [expr {$lgth - $myHeaderLgth}]
                    set crypted [string range $buffer $myHeaderLgth end]
puts stderr "cryptedLgth: [string length $crypted]!"
                    set cryptKey "a1b2c3d4e5f6g7h8"
                    set result [::compMsg compMsgDispatcher decryptMsg $crypted $mlen $cryptKey 16 $cryptKey 16 decrypted decryptedLgth]
puts stderr "decryptedLgth: $decryptedLgth!result!$result!"
                    if {$result != $::COMP_MSG_ERR_OK} {
puts stderr "decrypt error"
                    }
                    set buffer "${myHeader}${decrypted}"
                    set result [::compMsg dataView setData $buffer $mlen]
::compMsg compMsgData dumpBinary $buffer $lgth "decrypted"
                    dict set received buf $buffer
                  }
                  dict set received lgth [dict get $headerInfos headerLgth]
                  set result [handleEncryptedPart]
                  return $result
                }
              }
            } else {
puts stderr "partsFlags is encrypted and hdrEncryption is N\n"
            }
          }
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
#      set headerInfos [dict create]
      set received [dict create]
      dict set received buf ""
      dict set received lgth 0
      set dispFlags [list]
      resetHeaderInfos
      return $result
    }
    
    # ================================= compMsgIdentifyInit ====================================
    
    proc compMsgIdentifyInit {} {
      initHeadersAndFlags
      set result [::compMsg compMsgMsgDesc readHeadersAndSetFlags $::MSG_HEADS_FILE_NAME]
      return $result
    }
    
    # ================================= compMsgIdentify ====================================
    
    proc compMsgIdentify {command args} {
      switch $command {
        prepareEncryptedAnswer -
        handleReceivedPart -
        compMsgIdentifyInit {
          return [uplevel 0 $command $args]
        }
      }
      return $::COMP_MSG_ERR_OK
    }

  } ; # namespace compMsgIdentify
} ; # namespace compMsg
