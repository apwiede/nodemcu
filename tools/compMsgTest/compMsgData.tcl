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
      puts stderr "dumpMsg handle: [dict get $compMsgData handle]"
      puts stderr [format "  numFields: %d max: %d" [dict get $compMsgData numFields] [dict get $compMsgData maxFields]]
      puts stderr [format "  headerLgth: %d totalLgth: %d fieldOffset: %d" [dict get $compMsgData headerLgth] [dict get $compMsgData totalLgth] [dict get $compMsgData fieldOffset]]
      puts stderr [format "  flags: %s" [dict get $compMsgData flags]]

      set numEntries [dict get $compMsgData numFields]
      puts stderr "  fields:"
      set idx 0
      set fields [dict get $compMsgData fields]
      while {$idx < $numEntries} {
        set fieldInfo [lindex $fields $idx]
        set result [::compMsg dataView getFieldTypeStrFromId [dict get $fieldInfo fieldTypeId] fieldType]
        checkErrOK $result
        set result [::compMsg compMsgDataView getFieldNameStrFromId [dict get $fieldInfo fieldNameId] fieldName]
        checkErrOK $result
        puts stderr [format "    idx %d: key: %-20s type: %-8s lgth: %.5d offset: %.3d flags: %s" $idx $fieldName $fieldType [dict get $fieldInfo fieldLgth] [dict get $fieldInfo fieldOffset] [dict get $fieldInfo fieldFlags]]
        if {[lsearch [dict get $fieldInfo fieldFlags] "COMP_MSG_FIELD_IS_SET"] >= 0} {
          set result [::compMsg compMsgDataView getFieldValue $fieldInfo value 0]
          checkErrOK $result
          switch [dict get $fieldInfo fieldTypeId] {
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
      return $::COMP_MSG_ERR_OK
    }
    
    # ================================= getFieldTypeFromFieldNameId ====================================
    
    proc getFieldTypeFromFieldNameId {fieldNameId fieldTypeIdVar} {
      variable compMsgData

      set idx 0
      set fields [dict get $compMsgData fields]
      while {$idx < [dict get $compMsgData numFields]} {
        set fieldInfo [lindex $fields $idx]
        if {[dict get $fieldInfo fieldNameId] == $fieldNameId} {
          set fieldTypeId [dict get $fieldInfo fieldTypeId]
          return $::COMP_DISP_ERR_OK
        }
        incr idx
      }
      checkErrOK $::COMP_DISP_ERR_FIELD_NOT_FOUND
    }
 
    # ============================= addHandle ========================
    
    proc addHandle {handle headerVar} {
      variable compMsgData
      variable compMsgHandles
      upvar $headerVar header
    
      if {[llength [dict get $compMsgHandles handles]] eq 0} {
        set handleDict [dict create]
        dict set handleDict handle $handle
        dict set handleDict compMsgData $compMsgData
        dict set handleDict header [list]
        dict lappend compMsgHandles handles $handleDict
        dict incr compMsgHandles numHandles 1
        set header [list]
        return $::COMP_MSG_ERR_OK
      } else {
        # check for unused slot first
        set idx 0
        set handles [dict get $compMsgHandles handles]
        while {$idx < [dict get $compMsgHandles numHandles]} {
           set handleDict [lindex $handles $idx]
          if {[dict get $handleDict handle] eq [list]} {
            set handles [dict get $compMsgHandles handles]
            set entry [lindex $handles $idx]
            dict set entry handle $handle
            dict set entry compMsgData $compMsgData
            set handles [lreplace $handles $idx $idx $entry]
            dict set compMsgHandles handles $handles
            set header [list]
            return $::COMP_MSG_ERR_OK
          }
          incr idx
        }
        set entry [dict create]
        dict set entry handle $handle
        dict set entry compMsgData $compMsgData
        dict lappend compMsgHandles handles $entry
        set header [list]
        dict incr compMsgHandles numHandles 1
      }
      return $::COMP_MSG_ERR_OK
    }
    
    # ============================= deleteHandle ========================
    
    proc deleteHandle {handle} {
      if {compMsgHandles.handles == NULL} {
        checkErrOK $::COMP_MSG_ERR_HANDLE_NOT_FOUND
      }
      found = 0
      idx = 0
      numUsed = 0
      while {idx < compMsgHandles.numHandles} {
        if {(compMsgHandles.handles[idx].handle != NULL} && (c_strcmp(compMsgHandles.handles[idx].handle, handle) == 0)) {
          compMsgHandles.handles[idx].handle = NULL
          found++
        } else {
          if {compMsgHandles.handles[idx].handle != NULL} {
            numUsed++
          }
        }
        idx++
      }
      if {numUsed == 0} {
        os_free{compMsgHandles.handles}
        compMsgHandles.handles = NULL
      }
      if {$found} {
          return COMP_MSG_ERR_OK
      }
      checkErrOK $::COMP_MSG_ERR_HANDLE_NOT_FOUND
    }
    
    # ============================= checkHandle ========================
    
    proc checkHandle {handle compMsgData} {
      variable compMsgHandles
      variable compMsgData
    
      if {[dict get $compMsgHandles handles] eq [list]} {
        checkErrOK $::COMP_MSG_ERR_HANDLE_NOT_FOUND
      }
      set idx 0
      set handles [dict get compMsgHandles handles]
      while {$idx < [dict get $compMsgHandles numHandles]} {
        set entry [lindex $handles $idx]
        if {([dict get $entry handle] ne [list]) && ([dict get $entry handle] eq $handle)} {
          set compMsgData [dict get $entry compMsgData]
          return $::COMP_MSG_ERR_OK
        }
        incr idx
      }
      checkErrOK $::COMP_MSG_ERR_HANDLE_NOT_FOUND
    }
    
    # ============================= getMsgData ========================
    
    proc getMsgData {compMsgDispatcherVar dataVar lgthVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $dataVar data
      upvar $lgthVar lgth

      set compMsgData [dict get $compMsgDispatcher compMsgData]
      if {[lsearch [dict get $compMsgData flags] COMP_MSG_IS_INITTED] < 0} {
        checkErrOK $::COMP_MSG_ERR_NOT_YET_INITTED
      }
      if {[lsearch [dict get $compMsgData flags] COMP_MSG_IS_PREPARED] < 0} {
        checkErrOK $::COMP_MSG_ERR_NOT_YET_PREPARED
      }
      set result [::compMsg dataView getData data lgth]
      set lgth [dict get $compMsgData totalLgth]
      checkErrOK $result
      return $::COMP_MSG_ERR_OK
    }
    
    # ============================= setMsgData ========================
    
    proc setMsgData {data lgth} {
      variable compMsgData

      if {[lsearch [dct get $compMsgData flags] COMP_MSG_IS_INITTED] < 0} {
        checkErrOK $::COMP_MSG_ERR_NOT_YET_INITTED
      }
      set found false
      # check lgth
      while {$idx < [dict get $compMsgData numFields]} {
        set fields [dict get $compMsgData fields]
        set fieldInfo [linex $fields $idx]
        if {[dict get $fieldInfo fieldNameId] eq "COMP_MSG_SPEC_FIELD_TOTAL_LGTH"} {
          set found true
          break
        }
        incr idx
      }
      if {! $found} {
        checkErrOK $::COMP_MSG_ERR_NO_SUCH_FIELD
      } 
      # temporary replace data entry of dataView by our param data
      # to be able to use the get* functions for gettting totalLgth entry value
      set result [::compMsg dataView getData saveData saveLgth]
puts stderr "compMsgData1 setData"
      set result [::compMsg dataView setData $data [expr {[dict get $fieldOffset fieldOffset] + 4}]]
      # get totalLgth value from data
      set result [::compMsg dataView getUint16 [dict get $fieldInfo fieldOffset] lgth]
      checkErrOK $result
      if {$lgth != [dict get $compMsgData totalLgth]} {
        checkErrOK $::COMP_MSG_ERR_BAD_DATA_LGTH
      }
      # now make a copy of the data to be on the safe side
puts stderr "compMsgData2 setData"
      set result [::compMsg dataView setData $data [dict get $compMsgDate totalLgth]
      # and now set the IS_SET flags and other stuff
      set idx 0
      while {$idx < [dict get $compMsgData numFields]} {
        set fields [dict get $compMsgData fields]
        set fieldInfo [lindex $fields $idx]
        dict lappend fieldInfo fieldFlags COMP_MSG_FIELD_IS_SET
        set fields [lreplace $fields $idx$idx $fieldInfo]
        dict set compMsgData fields $fields
        incr idx
      }
      dict lappend compMsgData flags COMP_MSG_IS_PREPARED
      return $::COMP_MSG_ERR_OK
    }

    # ============================= compMsgGetPtrFromHandle ========================
    
    proc compMsgGetPtrFromHandle {handle} {
      if {[checkHandle $handle] != $::COMP_MSG_ERR_OK} {
        checkErrOK $::COMP_MSG_ERR_HANDLE_NOT_FOUND
      }
      return $::COMP_MSG_ERR_OK
    }

    # ================================= createMsg ====================================

    proc createMsg {compMsgDataVar numFields handleVar} {
      upvar $handleVar handle
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
      incr numHandles
      set handle [format "${::HANDLE_PREFIX}efff00%02d" $numHandles]
      dict set compMsgData handle $handle
      set result [addHandle $handle [dict get $compMsgData header]]
      checkErrOK $result
      set handle [dict get $compMsgData handle]
      return $::COMP_MSG_ERR_OK
    }

    # ================================= addField ====================================
    
    proc  addField {compMsgDataVar fieldName fieldType fieldLgth} {
      upvar $compMsgDataVar compMsgData
    
      if {[dict get $compMsgData numFields] >= [dict get $compMsgData maxFields]} {
        checkErrOK $::COMP_MSG_ERR_TOO_MANY_FIELDS
      }
      set result [::compMsg dataView getFieldTypeIdFromStr $fieldType fieldTypeId]
      checkErrOK $result
      set result [::compMsg compMsgDataView getFieldNameIdFromStr $fieldName fieldNameId $::COMP_MSG_INCR]
      checkErrOK $result
      set fieldInfo [dict create]
      # need to check for duplicate here !!
      if {$fieldName eq "@filler"} {
        dict lappend compMsgData flags COMP_MSG_HAS_FILLER
        set fieldLgth 0
        dict set fieldInfo fieldNameId $fieldNameId
        dict set fieldInfo fieldTypeId $fieldTypeId
        dict set fieldInfo fieldLgth $fieldLgth
        dict set fieldInfo fieldFlags [list]
        dict lappend compMsgData fields $fieldInfo
        dict incr compMsgData numFields 1
        return $::COMP_MSG_ERR_OK
      }
      if {$fieldName eq "@crc"} {
        dict lappend compMsgData flags COMP_MSG_HAS_CRC
        if {$fieldType eq "uint8_t"} {
          dict lappend compMsgData flags COMP_MSG_UINT8_CRC
        }
      }
      dict set fieldInfo fieldNameId $fieldNameId
      dict set fieldInfo fieldTypeId $fieldTypeId
      dict set fieldInfo fieldKeyTypeId $fieldTypeId
      dict set fieldInfo fieldLgth $fieldLgth
      dict set fieldInfo fieldFlags [list]
      dict lappend compMsgData fields $fieldInfo
      dict incr compMsgData numFields 1
      return $::COMP_MSG_ERR_OK
    }

    # ================================= getFieldValue ====================================
    
    proc getFieldValue {compMsgDispatcherVar fieldName valueVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $valueVar value
    
      set compMsgData [dict get $compMsgDispatcher compMsgData]
      if {[lsearch [dict get $compMsgData flags] COMP_MSG_IS_INITTED] < 0} {
        checkErrOK $::COMP_MSG_ERR_NOT_YET_INITTED
      }
      set result [::compMsg compMsgDataView getFieldNameIdFromStr $fieldName fieldNameId COMP_MSG_NO_INCR]
      checkErrOK $result
      set found false
      set idx 0
      set numEntries [dict get $compMsgData numFields]
      while {$idx < $numEntries} {
        set fields [dict get $compMsgData fields]
        set fieldInfo [lindex $fields $idx]
        if {$fieldNameId == [dict get $fieldInfo fieldNameId]} {
          set found true
          switch $fieldNameId {
            COMP_MSG_SPEC_FIELD_TABLE_ROWS {
             set value [dict get $compMsgData numTabRows]
             set result $::COMP_MSG_ERR_OK
            }
            COMP_MSG_SPEC_FIELD_TABLE_ROW_FIELDS {
             set value [dict get $compMsgData numTabRowFields]
             set result $::COMP_MSG_ERR_OK
            }
            default {
              if {[string range $fieldName 0 0] eq "#"} {
                set saveType [dict get $fieldInfo fieldTypeId]
                dict set fieldinfo fieldtypeId [dict get $fieldInfo fieldKeyTypeId]
                switch $saveType {
                  DATA_VIEW_FIELD_UINT8_VECTOR -
                  DATA_VIEW_FIELD_INT8_VECTOR {
                    set keyOffset 0 ; # we fetch the full lgth in compMsgDataView!!
                  }
                  default {
                    set keyOffset 5 ; # uint16_t + uint8_t + uint16_t
                  }
                }
                set result [::compMsg compMsgDataView getFieldValue $fieldInfo value $keyOffset]
                dict set fieldInfo fieldTypeId $saveType
              } else {
                set result [::compMsg compMsgDataView getFieldValue $fieldInfo value 0]
              }
            }
          }
          checkErrOK $result
          break
        }
        incr idx
      }
      if {!$found} {
        return $::COMP_MSG_ERR_FIELD_NOT_FOUND
      }
      return $::DATA_VIEW_ERR_OK
    }
    
    # ================================= setFieldValue ====================================
    
    proc setFieldValue {compMsgDispatcherVar fieldName value} {
      upvar $compMsgDispatcherVar compMsgDispatcher

      set compMsgData [dict get $compMsgDispatcher compMsgData]
      if {[lsearch [dict get $compMsgData flags] COMP_MSG_IS_INITTED] < 0} {
        checkErrOK $::COMP_MSG_ERR_NOT_YET_INITTED
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
          checkErrOK $::COMP_MSG_ERR_FIELD_CANNOT_BE_SET
        }
      }
      set idx 0
      set numEntries [dict get $compMsgData numFields]
    #ets_printf{"numEntries: %d\n", numEntries}
      while {$idx < $numEntries} {
        set fields [dict get $compMsgData fields]
        set fieldInfo [lindex $fields $idx]
        if {$fieldNameId == [dict get $fieldInfo fieldNameId]} {
          if {[string range $fieldName 0 0] eq "#"} {
            set msgDescPart [dict get $compMsgDispatcher msgDescPart]
            set msgKeyValueDescPart [dict get $compMsgDispatcher msgKeyValueDescPart]
            set offset [dict get $fieldInfo fieldOffset]
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
            set result [::compMsg compMsgDataView setFieldValue $fieldInfo $value 0]
          }
          checkErrOK $result
          dict lappend  fieldInfo fieldFlags COMP_MSG_FIELD_IS_SET
          set fields [lreplace $fields $idx $idx $fieldInfo]
          dict set compMsgData fields $fields
          dict set compMsgDispatcher compMsgData $compMsgData
#dumpMsg compMsgDispatcher
          return $::DATA_VIEW_ERR_OK
        }
        incr idx
      }
    #ets_printf{"idx: %d\n", idx}
      dict set compMsgDispatcher compMsgData $compMsgData
      checkErrOK $::DATA_VIEW_ERR_FIELD_NOT_FOUND
    }
    
    # ================================= prepareMsg ====================================
    
    proc prepareMsg {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher

      set compMsgData [dict get $compMsgDispatcher compMsgData]
      if {[lsearch [dict get $compMsgData flags] COMP_MSG_IS_INITTED] < 0} {
        checkErrOK $::COMP_MSG_ERR_NOT_YET_INITTED
      }
      # create the values which are different for each message!!
      set numEntries [dict get $compMsgData numFields]
      set idx 0
      set fields [dict get $compMsgData fields]
      while {$idx < $numEntries} {
        set fieldInfo [lindex $fields $idx]
        switch [dict get $fieldInfo fieldNameId] {
          COMP_MSG_SPEC_FIELD_RANDOM_NUM {
            set result [::compMsg compMsgDataView setRandomNum $fieldInfo]
            checkErrOK $result
            dict lappend fieldInfo fieldFlags COMP_MSG_FIELD_IS_SET
          }
          COMP_MSG_SPEC_FIELD_SEQUENCE_NUM {
            set result [::compMsg compMsgDataView setSequenceNum $fieldInfo]
            checkErrOK $result
            dict lappend fieldInfo fieldFlags COMP_MSG_FIELD_IS_SET
          }
          COMP_MSG_SPEC_FIELD_HDR_FILLER {
            set result [::compMsg compMsgDataView setFiller $fieldInfo]
            checkErrOK $result
            dict lappend fieldInfo fieldFlags COMP_MSG_FIELD_IS_SET
          }
          COMP_MSG_SPEC_FIELD_FILLER {
            set result [::compMsg compMsgDataView setFiller $fieldInfo]
            checkErrOK $result
            dict lappend fieldInfo fieldFlags COMP_MSG_FIELD_IS_SET
          }
          COMP_MSG_SPEC_FIELD_CRC {
            set startOffset 0
            if {[lsearch [dict get $compMsgData flags] COMP_MSG_CRC_USE_HEADER_LGTH] >= 0} {
                set startOffset [dict get $compMsgData headerLgth]
            }
            if {[lsearch [dict get $compMsgDispatcher currHdr hdrFlags] COMP_DISP_TOTAL_CRC] >= 0} {
              set startOffset [dict get $compMsgData headerLgth]
              # lgth is needed without totalCrc
              set fieldSequence [dict get $compMsgDispatcher currHdr fieldSequence]
              switch [lindex $fieldSequence end] {
                COMP_DISP_U8_TOTAL_CRC {
                  set lgth [expr {[dict get $compMsgData totalLgth] - 1}]
                }
                COMP_DISP_U16_TOTAL_CRC {
                  set lgth [expr {[dict get $compMsgData totalLgth] - 2}]
                }
              }
            } else {
              set lgth [dict get $compMsgData totalLgth]
            }
            set result [::compMsg compMsgDataView setCrc $fieldInfo $startOffset $lgth]
            checkErrOK $result
            dict lappend fieldInfo fieldFlags COMP_MSG_FIELD_IS_SET
          }
          COMP_MSG_SPEC_FIELD_TOTAL_CRC {
#            set result [::compMsg compMsgDataView setTotalCrc $fieldInfo]
#            checkErrOK $result
            dict lappend fieldInfo fieldFlags COMP_MSG_FIELD_IS_SET
          }
        }
        set fields [lreplace $fields $idx $idx $fieldInfo]
        incr idx
      }
      dict set compMsgData fields $fields
      dict lappend compMsgData flags COMP_MSG_IS_PREPARED
      dict set compMsgDispatcher compMsgData $compMsgData
      return $::COMP_MSG_ERR_OK
    }
    
    # ================================= addFlag ====================================
    
    proc addFlag {flag} {
      variable compMsgData
    
      dict lappend compMsgData flags $flag
      return $::COMP_MSG_ERR_OK
    }

    # ================================= deleteFlag ====================================
    
    proc deleteFlag {flag} {
      variable compMsgData
    
    }

    # ================================= initMsg ====================================
    
    proc initMsg {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher
    
      set compMsgData [dict get $compMsgDispatcher compMsgData]
      # initialize field offsets for each field
      # initialize totalLgth, headerLgth, cmdLgth
      if {[lsearch [dict get $compMsgData flags] COMP_MSG_IS_INITTED] >= 0} {
        return $::COMP_MSG_ERR_ALREADY_INITTED
      }
      dict set compMsgData fieldOffset 0
      set numEntries [dict get $compMsgData numFields]
      set msgDescParts [dict get $compMsgDispatcher compMsgData msgDescParts]
      set msgDescPartIdx 0
      set idx 0
      dict set compMsgData headerLgth 0
      while {$idx < $numEntries} {
        set fields [dict get $compMsgData fields]
        set fieldInfo [lindex $fields $idx]
        set msgDescPart [lindex $msgDescParts $msgDescPartIdx]
        dict set fieldInfo fieldOffset [dict get $compMsgData fieldOffset]
        switch [dict get $fieldInfo fieldNameId] {
          COMP_MSG_SPEC_FIELD_SRC -
          COMP_MSG_SPEC_FIELD_DST -
          COMP_MSG_SPEC_FIELD_TOTAL_LGTH -
          COMP_MSG_SPEC_FIELD_GUID -
          COMP_MSG_SPEC_FIELD_SRC_ID -
          COMP_MSG_SPEC_FIELD_HDR_FILLER {
            dict incr compMsgData headerLgth [dict get $fieldInfo fieldLgth]
            dict set compMsgData totalLgth [expr {[dict get $compMsgData fieldOffset] + [dict get $fieldInfo fieldLgth]}]
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
            dict set fieldInfo fieldLgth $fillerLgth
            set fields [lreplace $fields $idx $idx $fieldInfo]
            dict set compMsgData totalLgth [expr {[dict get $compMsgData fieldOffset] + $fillerLgth + $crcLgth}]
            dict set compMsgData cmdLgth [expr {[dict get $compMsgData totalLgth] - [dict get $compMsgData headerLgth]}]
          }
          default {
            dict set compMsgData totalLgth [expr {[dict get $compMsgData fieldOffset] + [dict get $fieldInfo fieldLgth]}]
            dict set compMsgData cmdLgth [expr {[dict get $compMsgData totalLgth] - [dict get $compMsgData headerLgth]}]
          }
        }
        set fields [lreplace $fields $idx $idx $fieldInfo]
        dict set compMsgData fields $fields
        dict incr compMsgData fieldOffset [dict get $fieldInfo fieldLgth]
        incr msgDescPartIdx
        incr idx
      }
      if {[dict get $compMsgData totalLgth] == 0} {
        checkErrOK $::COMP_MSG_ERR_FIELD_TOTAL_LGTH_MISSING
      }
      set totalLgth [dict get $compMsgData totalLgth]
#puts stderr "compMsgData3 setData: totalLgth: $totalLgth!"
      set result [::compMsg dataView setData [string repeat "\x00" $totalLgth] $totalLgth]
      checkErrOK $result
      dict lappend compMsgData flags COMP_MSG_IS_INITTED
      # set the appropriate field values for the lgth entries
      set idx 0
      set numEntries [dict get $compMsgData numFields]
      set fields [dict get $compMsgData fields]
      while {$idx < $numEntries} {
        set fieldInfo [lindex $fields $idx]
        switch [dict get $fieldInfo fieldNameId] {
          COMP_MSG_SPEC_FIELD_TOTAL_LGTH {
            set result [::compMsg compMsgDataView setFieldValue $fieldInfo [dict get $compMsgData totalLgth] 0]
            checkErrOK $result
            dict lappend fieldInfo fieldFlags COMP_MSG_FIELD_IS_SET
          }
          COMP_MSG_SPEC_FIELD_CMD_LGTH {
            set result [::compMsg compMsgDataView setFieldValue $fieldInfo [dict get $compMsgData cmdLgth] 0]
            checkErrOK $result
            dict lappend fieldInfo fieldFlags COMP_MSG_FIELD_IS_SET
          }
        }
        set fields [lreplace $fields $idx $idx $fieldInfo]
        incr idx
      }
      dict set compMsgData fields $fields
      dict set compMsgDispatcher compMsgData $compMsgData
      return $::COMP_MSG_ERR_OK
    }
    
    # ================================= initReceivedMsg ====================================
    
    proc initReceivedMsg {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher
    
puts stderr "initReceivedMsg"
      set compMsgData [dict get $compMsgDispatcher compMsgData]
      # initialize field offsets for each field
      if {[lsearch [dict get $compMsgData flags] COMP_MSG_IS_INITTED] >= 0} {
        checkErrOK $::COMP_MSG_ERR_ALREADY_INITTED
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
        set fieldInfo [lindex $fields $idx]
        set msgDescPart [lindex $msgDescParts $msgDescPartIdx]
        dict set compMsgDispatcher msgDescPart $msgDescPart
        set msgValPart [lindex $msgValParts $msgValPartIdx]
        dict set compMsgDispatcher msgValPart $msgValPart
        dict set fieldInfo fieldOffset [dict get $compMsgData fieldOffset]
        dict lappend fieldInfo fieldFlags COMP_MSG_FIELD_IS_SET
        set fields [lreplace $fields $idx $idx $fieldInfo]
        dict set compMsgData fields $fields
        dict set compMsgDispatcher compMsgData $compMsgData
        switch [dict get $fieldInfo fieldNameId] {
          COMP_MSG_SPEC_FIELD_SRC -
          COMP_MSG_SPEC_FIELD_DST -
          COMP_MSG_SPEC_FIELD_TOTAL_LGTH -
          COMP_MSG_SPEC_FIELD_GUID -
          COMP_MSG_SPEC_FIELD_SRC_ID -
          COMP_MSG_SPEC_FIELD_HDR_FILLER {
            dict incr compMsgData headerLgth [dict get $fieldInfo fieldLgth]
            dict set compMsgData totalLgth [expr {[dict get $compMsgData fieldOffset] + [dict get $fieldInfo fieldLgth]}]
            dict set compMsgDispatcher compMsgData $compMsgData
          }
          COMP_MSG_SPEC_FIELD_NUM_KEY_VALUES {
            dict set compMsgData totalLgth [expr {[dict get $compMsgData fieldOffset] + [dict get $fieldInfo fieldLgth]}]
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
            dict set fieldInfo fieldLgth $fillerLgth
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
              dict set fieldInfo fieldKeyTypeId $keyType
              dict set fieldInfo fieldLgth [expr {2 + 1 + 2 + $keyLgth}]
              dict set compMsgData totalLgth [expr {[dict get $compMsgData fieldOffset] + [dict get $fieldInfo fieldLgth]}]
            } else {
            }
          }
        }
        set fields [lreplace $fields $idx $idx $fieldInfo]
        dict set compMsgData fields $fields
        dict incr compMsgData fieldOffset [dict get $fieldInfo fieldLgth]
        dict set compMsgDispatcher compMsgData $compMsgData
        incr msgDescPartIdx
        incr idx
      }
      if {[dict get $compMsgData totalLgth] == 0} {
        checkErrOK $::COMP_MSG_ERR_FIELD_TOTAL_LGTH_MISSING
      }
      dict lappend compMsgData flags COMP_MSG_IS_INITTED
      dict set compMsgData fields $fields
#puts stderr "initMsg done!"
      dict set compMsgDispatcher compMsgData $compMsgData
#puts stderr "dumpMsg"
#::compMsg compMsgData dumpMsg compMsgDispatcher
#puts stderr "hexDump"
#::compMsg dataView dumpBinary $::compMsg::dataView::data $::compMsg::dataView::lgth "MSG_AA"
      return $::COMP_MSG_ERR_OK
    }
    
  } ; # namespace compMsgData
} ; # namespace compMsg
