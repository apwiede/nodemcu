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

# comMsgData dict
#   flags
#   handle
#   fields
#     fieldNameId
#     fieldTypeId
#     fieldLgth
#     fieldOffset
#   tableFields
#     fieldNameId
#     fieldTypeId
#     fieldLgth
#     fieldOffset
#   numFields
#   maxFields
#   fieldOffset
#   totalLgth
#   cmdLgth
#   headerLgth
#   header
#   msgDescParts ; # list
#   msgValParts ; # list

set ::HANDLE_PREFIX "compmsg_"
set ::COMP_DEF_NUM_DEF_FIELDS 15

namespace eval compMsg {
  namespace ensemble create

    namespace export compMsgData

  namespace eval compMsgData {
    namespace ensemble create
      
    namespace export compMsgDataInit freeCompMsgDataView createMsg addField
    namespace export initMsg prepareMsg setMsgData getMsgData getFieldTypeFromFieldNameId
    namespace export setFieldValue getFieldValue
    namespace export addFlag deleteFlag dumpMsg initReceivedMsg

    variable compMsgData [dict create]
    variable numHandles 0

    dict set compMsgData flags [list]
    dict set compMsgData handle [list]

    set compMsgHandles [dict create]
    dict set compMsgHandles handles [list]
    dict set compMsgHandles numHandles 0


    # ===================== dumpValues =============================
    
    proc dumpValues {data} {
      puts -nonewline stderr "      values: "
      set cnt 0
    if {[catch {
      foreach ch [split $data ""] {
        set pch $ch
        binary scan $ch c pch
        puts -nonewline stderr "  $ch [format 0x%02x [expr {$pch & 0xFF}]]"
        incr cnt
        if {($cnt > 0) && (($cnt % 10) == 0)} {
          puts -nonewline stderr "\n              "
        }
      }
    } msg]} {
      puts stderr "\ndumpValues MSG: $msg!"
      foreach ch [split $data ""] {
        set pch [binaryScanChar $ch]
        puts stderr "\n!${ch}!${pch}!"
      }
    }
      puts stderr ""
    }
    
    # ===================== dumpMsg =============================
    
    proc dumpMsg {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher

      set compMsgData [dict get $compMsgDispatcher compMsgData]
      set msgDescription [dict get $compMsgData msgDescription]
puts stderr "compMsgData keys: [dict keys $compMsgData]!"
      puts stderr "dumpMsg"
      puts stderr [format "  numFields: %d" [dict get $msgDescription numFields]]
puts stderr "compMsgMsgDesc keys:[dict keys [dict get $compMsgDispatcher compMsgMsgDesc]]!"
      set msgHeaderInfo [dict get $compMsgDispatcher compMsgMsgDesc msgHeaderInfo]
      puts stderr [format "  headerLgth: %d totalLgth: %d fieldOffset: %d" [dict get $msgHeaderInfo headerLgth] [dict get $compMsgData totalLgth] [dict get $compMsgData fieldOffset]]
      puts stderr [format "  flags: %s" [dict get $compMsgData flags]]

      set numEntries [dict get $msgDescription numFields]
      puts stderr "  fields:"
      set idx 0
      set msgFieldInfos [dict get $compMsgDispatcher compMsgTypesAndNames msgFieldInfos]
      set fieldDescInfos [dict get $msgFieldInfos fieldDescInfos]
      set fieldSequence [dict get $msgDescription fieldSequence]
      while {$idx < $numEntries} {
        set fieldNameId [lindex $fieldSequence $idx]
        set fieldDescInfo [lindex $fieldDescInfos $idx]
        set result [::compMsg dataView getFieldTypeStrFromId [dict get $fieldDescInfo fieldTypeId] fieldType]
        checkErrOK $result
        set result [::compMsg compMsgTypesAndNames getFieldNameStrFromId compMsgDispatcher $fieldNameId fieldName]
        checkErrOK $result
        puts stderr [format "    idx %2d: key: %-20s type: %-8s lgth: %.5d offset: %.3d flags: %s" $idx $fieldName $fieldType [dict get $fieldDescInfo fieldLgth] [dict get $fieldDescInfo fieldOffset] [dict get $fieldDescInfo fieldFlags]]
        if {[lsearch [dict get $fieldDescInfo fieldFlags] "COMP_MSG_FIELD_IS_SET"] >= 0} {
          set result [::compMsg compMsgDataView getFieldValue $fieldDescInfo value 0]
          checkErrOK $result
          switch [dict get $fieldDescInfo fieldTypeId] {
            DATA_VIEW_FIELD_UINT8_T {
              puts stderr [format "      value: 0x%02x %d" $value $value]
            }
            DATA_VIEW_FIELD_INT8_T {
              puts stderr [format "      value: 0x%02x %d" $value $value]
            }
            DATA_VIEW_FIELD_UINT16_T {
              set value [expr {$value & 0xFFFF}]
              puts stderr [format "      value: 0x%04x %d" $value $value]
            }
            DATA_VIEW_FIELD_INT16_T {
              puts stderr [format "      value: 0x%04x %d" $value $value]
            }
            DATA_VIEW_FIELD_UINT32_T {
              puts stderr [format "      value: 0x%08x %d" $value $value]
            }
            DATA_VIEW_FIELD_INT32_T {
              puts stderr [format "      value: 0x%08x %d" $value $value]
            }
            DATA_VIEW_FIELD_UINT8_VECTOR {
              if {[string is print $value]} {
                puts stderr [format "      value: %s" $value]
              }
              set lgth [string length $value]
              dumpValues $value
            }
            DATA_VIEW_FIELD_INT8_VECTOR {
              if {[string is print $value]} {
                puts stderr [format "      value: %s" $value]
              }
              set lgth [string length $value]
              dumpValues $value
            }
            DATA_VIEW_FIELD_UINT16_VECTOR {
              puts stderr [format "      value: %s" $value]
            }
            DATA_VIEW_FIELD_INT16_VECTOR {
              puts stderr [format "      value: %s" $value]
            }
            DATA_VIEW_FIELD_UINT32_VECTOR {
              puts stderr [format "      value: %s" $value]
            }
            DATA_VIEW_FIELD_INT32_VECTOR {
              puts stderr [format "      value: %s" $value]
            }
          }
        }
        incr idx
      }
      return [checkErrOK OK]
    }
    
    # ================================= getFieldTypeFromFieldNameId ====================================
    
    proc getFieldTypeFromFieldNameId {fieldNameId fieldTypeIdVar} {
      variable compMsgData

      set idx 0
      set fields [dict get $compMsgData fields]
      while {$idx < [dict get $compMsgData numFields]} {
        set fieldDescInfo [lindex $fields $idx]
        if {[dict get $fieldDescInfo fieldNameId] == $fieldNameId} {
          set fieldTypeId [dict get $fieldDescInfo fieldTypeId]
          return [checkErrOK OK]
        }
        incr idx
      }
      checkErrOK [checkErrOK FIELD_NOT_FOUND]
    }
 
    # ============================= getMsgData ========================
    
    proc getMsgData {compMsgDispatcherVar dataVar lgthVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $dataVar data
      upvar $lgthVar lgth

      set compMsgData [dict get $compMsgDispatcher compMsgData]
      if {[lsearch [dict get $compMsgData flags] COMP_MSG_IS_INITTED] < 0} {
        checkErrOK NOT_YET_INITTED
      }
      if {[lsearch [dict get $compMsgData flags] COMP_MSG_IS_PREPARED] < 0} {
        checkErrOK NOT_YET_PREPARED
      }
      set result [::compMsg dataView getDataViewData data lgth]
      set lgth [dict get $compMsgData totalLgth]
      checkErrOK $result
      return [checkErrOK OK]
    }
    
    # ============================= setMsgData ========================
    
    proc setMsgData {data lgth} {
      variable compMsgData

      if {[lsearch [dct get $compMsgData flags] COMP_MSG_IS_INITTED] < 0} {
        checkErrOK NOT_YET_INITTED
      }
      set found false
      # check lgth
      while {$idx < [dict get $compMsgData numFields]} {
        set fields [dict get $compMsgData fields]
        set fieldDescInfo [linex $fields $idx]
        if {[dict get $fieldDescInfo fieldNameId] eq "COMP_MSG_SPEC_FIELD_TOTAL_LGTH"} {
          set found true
          break
        }
        incr idx
      }
      if {! $found} {
        checkErrOK NO_SUCH_FIELD
      } 
      # temporary replace data entry of dataView by our param data
      # to be able to use the get* functions for gettting totalLgth entry value
      set result [::compMsg dataView getDataViewData saveData saveLgth]
puts stderr "compMsgData1 setDataViewData"
      set result [::compMsg dataView setDataViewData $data [expr {[dict get $fieldOffset fieldOffset] + 4}]]
      # get totalLgth value from data
      set result [::compMsg dataView getUint16 [dict get $fieldDescInfo fieldOffset] lgth]
      checkErrOK $result
      if {$lgth != [dict get $compMsgData totalLgth]} {
        checkErrOK BAD_DATA_LGTH
      }
      # now make a copy of the data to be on the safe side
puts stderr "compMsgData2 setDataViewData"
      set result [::compMsg dataView setDataViewData $data [dict get $compMsgDate totalLgth]
      # and now set the IS_SET flags and other stuff
      set idx 0
      while {$idx < [dict get $compMsgData numFields]} {
        set fields [dict get $compMsgData fields]
        set fieldDescInfo [lindex $fields $idx]
        dict lappend fieldDescInfo fieldFlags COMP_MSG_FIELD_IS_SET
        set fields [lreplace $fields $idx$idx $fieldDescInfo]
        dict set compMsgData fields $fields
        incr idx
      }
      dict lappend compMsgData flags COMP_MSG_IS_PREPARED
      return [checkErrOK OK]
    }

    # ============================= compMsgGetPtrFromHandle ========================
    
    proc compMsgGetPtrFromHandle {handle} {
      if {[checkHandle $handle] != [dict get $::COMP_MSG_ERR OK]} {
        checkErrOK HANDLE_NOT_FOUND
      }
      return [checkErrOK OK]
    }

    # ================================= createMsg ====================================

    proc createMsg {compMsgDataVar numFields} {
      upvar $compMsgDataVar compMsgData
      variable numHandles

      dict set compMsgData fields [list]
      dict set compMsgData tableFields [list]
      dict set compMsgData flags [list]
      dict set compMsgData numFields 0
      dict set compMsgData maxFields $numFields
      dict set compMsgData fieldOffset 0
      dict set compMsgData totalLgth 0
      dict set compMsgData cmdLgth 0
      dict set compMsgData headerLgth 0
      dict set compMsgData header [list]
      return [checkErrOK OK]
    }

    # ================================= getFieldValue ====================================
    
    proc getFieldValue {compMsgDispatcherVar fieldName valueVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $valueVar value
    
      set compMsgData [dict get $compMsgDispatcher compMsgData]
      if {[lsearch [dict get $compMsgData flags] COMP_MSG_IS_INITTED] < 0} {
        checkErrOK [NOT_YET_INITTED]
      }
      set result [::compMsg compMsgDataView getFieldNameIdFromStr compMsgDispatcher $fieldName fieldNameId COMP_MSG_NO_INCR]
      checkErrOK $result
      set found false
      set idx 0
      set numEntries [dict get $compMsgData numFields]
      while {$idx < $numEntries} {
        set fields [dict get $compMsgData fields]
        set fieldDescInfo [lindex $fields $idx]
        if {$fieldNameId == [dict get $fieldDescInfo fieldNameId]} {
          set found true
          switch $fieldNameId {
            COMP_MSG_SPEC_FIELD_TABLE_ROWS {
             set value [dict get $compMsgData numTabRows]
             set result [dict get $::COMP_MSG_ERR OK]
            }
            COMP_MSG_SPEC_FIELD_TABLE_ROW_FIELDS {
             set value [dict get $compMsgData numTabRowFields]
             set result [dict get $::COMP_MSG_ERR OK]
            }
            default {
              if {[string range $fieldName 0 0] eq "#"} {
                set saveType [dict get $fieldDescInfo fieldTypeId]
                dict set fieldinfo fieldtypeId [dict get $fieldDescInfo fieldKeyTypeId]
                switch $saveType {
                  DATA_VIEW_FIELD_UINT8_VECTOR -
                  DATA_VIEW_FIELD_INT8_VECTOR {
                    set keyOffset 0 ; # we fetch the full lgth in compMsgDataView!!
                  }
                  default {
                    set keyOffset 5 ; # uint16_t + uint8_t + uint16_t
                  }
                }
                set result [::compMsg compMsgDataView getFieldValue $fieldDescInfo value $keyOffset]
                dict set fieldDescInfo fieldTypeId $saveType
              } else {
                set result [::compMsg compMsgDataView getFieldValue $fieldDescInfo value 0]
              }
            }
          }
          checkErrOK $result
          break
        }
        incr idx
      }
      if {!$found} {
        checkErrOK FIELD_NOT_FOUND
      }
      return [checkErrOK OK]
    }
    
    # ================================= setFieldValue ====================================
    
    proc setFieldValue {compMsgDispatcherVar fieldName value} {
      upvar $compMsgDispatcherVar compMsgDispatcher

      set compMsgData [dict get $compMsgDispatcher compMsgData]
      if {[lsearch [dict get $compMsgData flags] COMP_MSG_IS_INITTED] < 0} {
        checkErrOK [NOT_YET_INITTED]
      }
      set result [::compMsg compMsgDataView getFieldNameIdFromStr $fieldName fieldNameId $::COMP_MSG_NO_INCR]
      checkErrOK $result
      switch $fieldNameId {
        COMP_MSG_SPEC_FIELD_TOTAL_LGTH -
        COMP_MSG_SPEC_FIELD_CMD_LGTH -
        COMP_MSG_SPEC_FIELD_FILLER -
        COMP_MSG_SPEC_FIELD_CRC -
        COMP_MSG_SPEC_FIELD_RANDOM_NUM -
        COMP_MSG_SPEC_FIELD_SEQUENCE_NUM {
          checkErrOK FIELD_CANNOT_BE_SET
        }
      }
      set idx 0
      set numEntries [dict get $compMsgData numFields]
    #ets_printf{"numEntries: %d\n", numEntries}
      while {$idx < $numEntries} {
        set fields [dict get $compMsgData fields]
        set fieldDescInfo [lindex $fields $idx]
        if {$fieldNameId == [dict get $fieldDescInfo fieldNameId]} {
          if {[string range $fieldName 0 0] eq "#"} {
            set msgDescPart [dict get $compMsgDispatcher msgDescPart]
            set msgKeyValueDescPart [dict get $compMsgDispatcher msgKeyValueDescPart]
            set offset [dict get $fieldDescInfo fieldOffset]
            set result [::compMsg dataView setUint16 $offset [dict get $msgKeyValueDescPart keyId]]
            checkErrOK $result
            incr offset 2
            set result [::compMsg dataView getFieldTypeIntFromId [dict get $msgDescPart fieldTypeId] fieldTypeInt]
            checkErrOK $result
            set result [::compMsg dataView setUint8 $offset $fieldTypeInt]
            checkErrOK $result
            incr offset 1
            set result [::compMsg dataView setUint16 $offset [expr {[dict get $msgDescPart fieldSize] - (2 + 1 + 2)}]]
            checkErrOK $result
            incr offset 2
            switch [dict get $msgDescPart fieldTypeId] {
              DATA_VIEW_FIELD_UINT8_VECTOR {
                set result [::compMsg dataView setUint8Vector $offset $value [string length $value]]
              }
              DATA_VIEW_FIELD_UINT32_T {
                set result [::compMsg dataView setUint32 $offset $value]
              }
              default {
puts stderr "bad default type for $fieldName: [dict get $msgDescPart fieldTypeId]!"
              }
            }
          } else {
            set result [::compMsg compMsgDataView setFieldValue $fieldDescInfo $value 0]
          }
          checkErrOK $result
          dict lappend  fieldDescInfo fieldFlags COMP_MSG_FIELD_IS_SET
          set fields [lreplace $fields $idx $idx $fieldDescInfo]
          dict set compMsgData fields $fields
          dict set compMsgDispatcher compMsgData $compMsgData
#dumpMsg compMsgDispatcher
          return [checkErrOK OK]
        }
        incr idx
      }
    #ets_printf{"idx: %d\n", idx}
      dict set compMsgDispatcher compMsgData $compMsgData
      checkErrOK FIELD_NOT_FOUND
    }
    
    # ================================= initMsg ====================================
    
    proc initMsg {compMsgDispatcherVar msgDescription} {
      upvar $compMsgDispatcherVar compMsgDispatcher
    
puts stderr "initMsg: msgDescription: $msgDescription!"
      set compMsgData [dict get $compMsgDispatcher compMsgData]
      # initialize field offsets for each field
      # initialize totalLgth and cmdLgth
      if {[lsearch [dict get $compMsgData flags] COMP_MSG_IS_INITTED] >= 0} {
        return [checkErrOK ALREADY_INITTED]
      }
      dict set compMsgData fieldOffset 0
      set numEntries [dict get $msgDescription numFields]
      set idx 0
      dict set compMsgData headerLgth 0
      set compMsgTypesAndNames [dict get $compMsgDispatcher compMsgTypesAndNames]
      set msgFieldInfos [dict get $compMsgTypesAndNames msgFieldInfos]
      set fieldDescInfos [dict get $msgFieldInfos fieldDescInfos]
puts stderr "numEntries: $numEntries!"
      set fieldSequence [dict get $msgDescription fieldSequence]
      while {$idx < $numEntries} {
        set fieldNameIntId [lindex $fieldSequence $idx]
        set fieldDescInfo [lindex $fieldDescInfos $fieldNameIntId]
        set result [::compMsg compMsgTypesAndNames getFieldNameIdFromInt compMsgDispatcher $fieldNameIntId fieldNameId]
        checkErrOK $result
        set fieldOffset [dict get $compMsgData fieldOffset]
        dict set fieldDescInfo fieldOffset $fieldOffset
        set fieldLgth [dict get $fieldDescInfo fieldLgth]
puts stderr "init: $fieldNameId!$fieldOffset!$fieldLgth!"
        switch $fieldNameId {
          COMP_MSG_SPEC_FIELD_SRC -
          COMP_MSG_SPEC_FIELD_DST -
          COMP_MSG_SPEC_FIELD_TOTAL_LGTH -
          COMP_MSG_SPEC_FIELD_GUID -
          COMP_MSG_SPEC_FIELD_SRC_ID -
          COMP_MSG_SPEC_FIELD_HDR_FILLER {
            dict set compMsgData totalLgth [expr {$fieldOffset + $fieldLgth}]
          }
          COMP_MSG_SPEC_FIELD_FILLER {
            set fillerLgth 0
            set result [::compMsg compMsgDataView getCrcLgth compMsgDispatcher $msgDescription crcLgth]
            checkErrOK $result
            set myLgth [expr {$fieldOffset + $crcLgth - [dict get $compMsgData headerLgth]}]
            while {[expr {$myLgth % 16}] != 0} {
              incr myLgth
              incr fillerLgth
            }
            dict set fieldDescInfo fieldLgth $fillerLgth
            set fieldDescInfos [lreplace $fieldDescInfos $fieldNameIntId $fieldNameIntId $fieldDescInfo]
            dict set compMsgData totalLgth [expr {$fieldOffset + $fillerLgth + $crcLgth}]
            dict set compMsgData cmdLgth [expr {[dict get $compMsgData totalLgth] - [dict get $compMsgData headerLgth]}]
          }
          default {
            dict set compMsgData totalLgth [expr {$fieldOffset + [dict get $fieldDescInfo fieldLgth]}]
            dict set compMsgData cmdLgth [expr {[dict get $compMsgData totalLgth] - [dict get $compMsgData headerLgth]}]
          }
        }
        set fieldDescInfos [lreplace $fieldDescInfos $fieldNameIntId $fieldNameIntId $fieldDescInfo]
        dict set compMsgTypesAndNames fieldDescInfos $fieldDescInfos
        dict incr compMsgData fieldOffset [dict get $fieldDescInfo fieldLgth]
        set totalLgth [dict get $compMsgData totalLgth]
        incr idx
      }
      if {[dict get $compMsgData totalLgth] == 0} {
        checkErrOK [FIELD_TOTAL_LGTH_MISSING]
      }
      set totalLgth [dict get $compMsgData totalLgth]
      set result [::compMsg dataView setDataViewData [string repeat "\x00" $totalLgth] $totalLgth]
      checkErrOK $result
      dict lappend compMsgData flags COMP_MSG_IS_INITTED
      # set the appropriate field values for the lgth entries
      set idx 0
      set numEntries [dict get $msgDescription numFields]
      while {$idx < $numEntries} {
        set fieldNameIntId [lindex $fieldSequence $idx]
        set fieldDescInfo [lindex $fieldDescInfos $fieldNameIntId]
        set result [::compMsg compMsgTypesAndNames getFieldNameIdFromInt compMsgDispatcher $fieldNameIntId fieldNameId]
        checkErrOK $result
        switch $fieldNameId {
          COMP_MSG_SPEC_FIELD_TOTAL_LGTH {
            set result [::compMsg compMsgDataView setFieldValue $fieldDescInfo [dict get $compMsgData totalLgth] 0]
            checkErrOK $result
            dict lappend fieldDescInfo fieldFlags COMP_MSG_FIELD_IS_SET
          }
          COMP_MSG_SPEC_FIELD_CMD_LGTH {
            set result [::compMsg compMsgDataView setFieldValue $fieldDescInfo [dict get $compMsgData cmdLgth] 0]
            checkErrOK $result
            dict lappend fieldDescInfo fieldFlags COMP_MSG_FIELD_IS_SET
          }
        }
        set fieldDescInfos [lreplace $fieldDescInfos $fieldNameIntId $fieldNameIntId $fieldDescInfo]
        incr idx
      }
      dict set msgFieldInfos fieldDescInfos $fieldDescInfos
      dict set compMsgDispatcher compMsgTypesAndNames msgFieldInfos $msgFieldInfos
      dict set compMsgDispatcher compMsgData $compMsgData
puts stderr "initMsg done"
      return [checkErrOK OK]
    }
    
    # ================================= initReceivedMsg ====================================
    
    proc initReceivedMsg {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher
    
puts stderr "initReceivedMsg"
      set compMsgData [dict get $compMsgDispatcher compMsgData]
      # initialize field offsets for each field
      if {[lsearch [dict get $compMsgData flags] COMP_MSG_IS_INITTED] >= 0} {
        checkErrOK ALREADY_INITTED
      }
      dict lappend compMsgData flags COMP_MSG_IS_INITTED
      dict set compMsgDispatcher compMsgData $compMsgData
      set msgDescParts [dict get $compMsgDispatcher compMsgData msgDescParts]
      set msgValParts [dict get $compMsgDispatcher compMsgData msgValParts]
      set msgDescPartIdx 0
      set msgValPartIdx 0
      dict set compMsgData fieldOffset 0
      set numEntries [dict get $compMsgData numFields]
      set idx 0
      dict set compMsgData headerLgth 0
      while {$idx < $numEntries} {
        set fields [dict get $compMsgData fields]
        set fieldDescInfo [lindex $fields $idx]
        set msgDescPart [lindex $msgDescParts $msgDescPartIdx]
        dict set compMsgDispatcher msgDescPart $msgDescPart
        set msgValPart [lindex $msgValParts $msgValPartIdx]
        dict set compMsgDispatcher msgValPart $msgValPart
        dict set fieldDescInfo fieldOffset [dict get $compMsgData fieldOffset]
        dict lappend fieldDescInfo fieldFlags COMP_MSG_FIELD_IS_SET
        set fields [lreplace $fields $idx $idx $fieldDescInfo]
        dict set compMsgData fields $fields
        dict set compMsgDispatcher compMsgData $compMsgData
        switch [dict get $fieldDescInfo fieldNameId] {
          COMP_MSG_SPEC_FIELD_SRC -
          COMP_MSG_SPEC_FIELD_DST -
          COMP_MSG_SPEC_FIELD_TOTAL_LGTH -
          COMP_MSG_SPEC_FIELD_GUID -
          COMP_MSG_SPEC_FIELD_SRC_ID -
          COMP_MSG_SPEC_FIELD_HDR_FILLER {
            dict incr compMsgData headerLgth [dict get $fieldDescInfo fieldLgth]
            dict set compMsgData totalLgth [expr {[dict get $compMsgData fieldOffset] + [dict get $fieldDescInfo fieldLgth]}]
            dict set compMsgDispatcher compMsgData $compMsgData
          }
          COMP_MSG_SPEC_FIELD_NUM_KEY_VALUES {
            dict set compMsgData totalLgth [expr {[dict get $compMsgData fieldOffset] + [dict get $fieldDescInfo fieldLgth]}]
            set result [::compMsg compMsgData getFieldValue compMsgDispatcher [dict get $msgDescPart fieldNameStr] value]
            checkErrOK $result
            dict set compMsgData numKeyValues $value
            dict set compMsgDispatcher compMsgData $compMsgData
          }
          COMP_MSG_SPEC_FIELD_FILLER {
            set fillerLgth 0
            set crcLgth 0
            if {[lsearch [dict get $compMsgData flags] COMP_MSG_HAS_CRC] >= 0} {
              if {[lsearch [dict get $compMsgData flags] COMP_MSG_UINT8_CRC] >= 0} {
                set crcLgth 1
              } else {
                set crcLgth 2
              }
            }
            set myLgth [expr {[dict get $compMsgData fieldOffset] + $crcLgth - [dict get $compMsgData headerLgth]}]
            while {[expr {$myLgth % 16}] != 0} {
              incr myLgth
              incr fillerLgth
            }
            dict set fieldDescInfo fieldLgth $fillerLgth
            dict set compMsgData totalLgth [expr {[dict get $compMsgData fieldOffset] + $fillerLgth + $crcLgth}]
          }
          default {
            if {[string range [dict get $msgDescPart fieldNameStr] 0 0] eq "#"} {
              set offset [dict get $compMsgData fieldOffset]
puts stderr "== offset!$offset!"
              set result [::compMsg dataView getUint16 $offset key]
              checkErrOK $result
puts stderr "key: $value offset: $offset!"
              incr offset 2
              set result [::compMsg dataView getUint8 $offset keyType]
              checkErrOK $result
puts stderr "type: $value offset: $offset!"
              incr offset 1
              set result [::compMsg dataView getUint16 $offset keyLgth]
              checkErrOK $result
puts stderr "size: $value offset: $offset!"
              dict set fieldDescInfo fieldKeyTypeId $keyType
              dict set fieldDescInfo fieldLgth [expr {2 + 1 + 2 + $keyLgth}]
              dict set compMsgData totalLgth [expr {[dict get $compMsgData fieldOffset] + [dict get $fieldDescInfo fieldLgth]}]
            } else {
            }
          }
        }
        set fields [lreplace $fields $idx $idx $fieldDescInfo]
        dict set compMsgData fields $fields
        dict incr compMsgData fieldOffset [dict get $fieldDescInfo fieldLgth]
        dict set compMsgDispatcher compMsgData $compMsgData
        incr msgDescPartIdx
        incr idx
      }
      if {[dict get $compMsgData totalLgth] == 0} {
        checkErrOK FIELD_TOTAL_LGTH_MISSING
      }
      dict lappend compMsgData flags COMP_MSG_IS_INITTED
      dict set compMsgData fields $fields
#puts stderr "initMsg done!"
      dict set compMsgDispatcher compMsgData $compMsgData
#puts stderr "dumpMsg"
#::compMsg compMsgData dumpMsg compMsgDispatcher
#puts stderr "hexDump"
#::compMsg dataView dumpBinary $::compMsg::dataView::data $::compMsg::dataView::lgth "MSG_AA"
      return [checkErrOK OK]
    }
 
    # ================================= prepareMsg ====================================
    
    proc prepareMsg {compMsgDispatcherVar msgDescription} {
      upvar $compMsgDispatcherVar compMsgDispatcher

      set compMsgData [dict get $compMsgDispatcher compMsgData]
      if {[lsearch [dict get $compMsgData flags] COMP_MSG_IS_INITTED] < 0} {
        checkErrOK NOT_YET_INITTED
      }
puts stderr "prepareMsg:!"
      # create the values which are different for each message!!
      set numEntries [dict get $msgDescription numFields]
      set idx 0
      set fieldSequence [dict get $msgDescription fieldSequence]
      set compMsgTypesAndNames [dict get $compMsgDispatcher compMsgTypesAndNames]
      set msgFieldInfos [dict get $compMsgTypesAndNames msgFieldInfos]
      set fieldDescInfos [dict get $msgFieldInfos fieldDescInfos]
      while {$idx < $numEntries} {
        set fieldNameIntId [lindex $fieldSequence $idx]
        set fieldDescInfo [lindex $fieldDescInfos $fieldNameIntId]
        set result [::compMsg compMsgTypesAndNames getFieldNameIdFromInt compMsgDispatcher $fieldNameIntId fieldNameId]
        checkErrOK $result
puts stderr "field: $fieldNameId!"
        switch $fieldNameId {
          COMP_MSG_SPEC_FIELD_RANDOM_NUM {
            set result [::compMsg compMsgDataView setRandomNum $fieldDescInfo]
            checkErrOK $result
            dict lappend fieldDescInfo fieldFlags COMP_MSG_FIELD_IS_SET
          }
          COMP_MSG_SPEC_FIELD_SEQUENCE_NUM {
            set result [::compMsg compMsgDataView setSequenceNum $fieldDescInfo]
            checkErrOK $result
            dict lappend fieldDescInfo fieldFlags COMP_MSG_FIELD_IS_SET
          }
          COMP_MSG_SPEC_FIELD_HDR_FILLER {
            set result [::compMsg compMsgDataView setFiller $fieldDescInfo]
            checkErrOK $result
            dict lappend fieldDescInfo fieldFlags COMP_MSG_FIELD_IS_SET
          }
          COMP_MSG_SPEC_FIELD_FILLER {
            set result [::compMsg compMsgDataView setFiller $fieldDescInfo]
            checkErrOK $result
            dict lappend fieldDescInfo fieldFlags COMP_MSG_FIELD_IS_SET
          }
          COMP_MSG_SPEC_FIELD_CRC {
            set startOffset 0
puts stderr "crc flags: [dict get $compMsgData flags]!"
puts stderr "crc2 msgDesc: [dict keys $msgDescription]!"
            if {[lsearch [dict get $compMsgData flags] COMP_MSG_CRC_USE_HEADER_LGTH] >= 0} {
                set startOffset [dict get $compMsgData headerLgth]
            }
puts stderr "crc: flags: [dict get $msgDescription fieldFlags]!"
            if {[lsearch [dict get $msgDescription fieldFlags] COMP_MSG_HAS_TOTAL_CRC] >= 0} {
puts stderr "desc  keys: [dict keys [dict get $compMsgDispatcher compMsgMsgDesc]]!"
              set startOffset [dict get $compMsgDispatcher compMsgMsgDesc msgHeaderInfo headerLgth]
              # lgth is needed without totalCrc
              set totalCrcIdx 0
              set result [::compMsg compMsgDataView getTotalCrcLgth compMsgDispatcher $msgDescription totalCrcLgth]
              checkErrOK $result
              set lgth [expr {[dict get $compMsgData totalLgth] - $totalCrcLgth}]
            } else {
              set lgth [dict get $compMsgData totalLgth]
            }
puts stderr "setCrc: $fieldDescInfo!startOffset: $startOffset!lgth: $lgth!"
            set result [::compMsg compMsgDataView setCrc $fieldDescInfo $startOffset $lgth]
            checkErrOK $result
            dict lappend fieldDescInfo fieldFlags COMP_MSG_FIELD_IS_SET
          }
          COMP_MSG_SPEC_FIELD_TOTAL_CRC {
#            set result [::compMsg compMsgDataView setTotalCrc $fieldDescInfo]
#            checkErrOK $result
            dict lappend fieldDescInfo fieldFlags COMP_MSG_FIELD_IS_SET
          }
        }
        set fieldDescInfos [lreplace $fieldDescInfos $idx $idx $fieldDescInfo]
        incr idx
      }
      dict set msgFieldInfos fieldDescInfos $fieldDescInfos
      dict set compMsgTypesAndNames msgFieldInfos $msgFieldInfos
      dict set compMsgDispatcher compMsgTypesAndNames $compMsgTypesAndNames
      dict lappend compMsgData flags COMP_MSG_IS_PREPARED
      dict set compMsgDispatcher compMsgData $compMsgData
      return [checkErrOK OK]
    }
    
  } ; # namespace compMsgData
} ; # namespace compMsg
