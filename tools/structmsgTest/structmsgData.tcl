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

set ::HANDLE_PREFIX "stmsg_"
set ::STRUCT_DEF_NUM_DEF_FIELDS 15

namespace eval structmsg {
  namespace ensemble create

    namespace export structmsgData

  namespace eval structmsgData {
    namespace ensemble create
      
    namespace export structmsgDataInit freeStructmsgDataView createMsg addField
    namespace export initMsg prepareMsg setMsgdata getMsgData getFieldTypeFromFieldNameId
    namespace export setFieldValue getFieldValue setTableFieldValue getTableFieldValue
    namespace export addFlag deleteFlag dumpBinary

    variable structmsgData [dict create]
    variable numHandles 0

    dict set structmsgData flags [list]
    dict set structmsgData handle [list]

    set structmsgHandles [dict create]
    dict set structmsgHandles handles [list]
    dict set structmsgHandles numHandles 0


    # ================================= dumpBinary ====================================
    
    proc dumpBinary {data lgth what} {
      variable structmsgData

      set idx 0
      foreach ch [split $data ""] {
        set pch $ch
        if {![string is integer $ch]} {
          binary scan $ch c pch
        }
        puts stderr "$idx: $ch [format 0x%02x [expr {$pch & 0xFF}]]!"
        incr idx
        if {$idx > $lgth} {
          break
        }
      }
    }

    # ================================= getFieldTypeFromFieldNameId ====================================
    
    proc getFieldTypeFromFieldNameId {fieldNameId fieldTypeIdVar} {
      variable structmsgData

      set idx 0
      set fields [dict get $structmsgData fields]
      while {$idx < [dict get $structmsgData numFields]} {
        set fieldInfo [lindex $fields $idx]
        if {[dict get $fieldInfo fieldNameId] == $fieldNameId} {
          set fieldTypeId [dict get $fieldInfo fieldTypeId]
          return $::STRUCT_DISP_ERR_OK
        }
        incr idx
      }
      # and now check the table fields
      set idx 0
      set fields [dict get $structmsgData tableFields]
      while {$idx < [dict get $structmsgData numTableRowFields]} {
        set fieldInfo [lindex fields $idx]
        if {[dict get $fieldInfo fieldNameId] == $fieldNameId} {
          set fieldTypeId [dict get $fieldInfo fieldTypeId]
          return $::STRUCT_DISP_ERR_OK
        }
        incr idx
      }
      return $::STRUCT_DISP_ERR_FIELD_NOT_FOUND
    }
 
    # ============================= addHandle ========================
    
    proc addHandle {handle headerVar} {
      variable structmsgData
      variable structmsgHandles
      upvar $headerVar header
    
      if {[llength [dict get $structmsgHandles handles]] eq 0} {
        set handleDict [dict create]
        dict set handleDict handle $handle
        dict set handleDict structmsgData $structmsgData
        dict set handleDict header [list]
        dict lappend structmsgHandles handles $handleDict
        dict incr structmsgHandles numHandles 1
        set header [list]
        return $::STRUCT_MSG_ERR_OK
      } else {
        # check for unused slot first
        set idx 0
        set handles [dict get $structmsgHandles handles]
        while {$idx < [dict get $structmsgHandles numHandles]} {
           set handleDict [lindex $handles $idx]
          if {[dict get $handleDict handle] eq [list]} {
            set handles [dict get $structmsgHandles handles]
            set entry [lindex $handles $idx]
            dict set entry handle $handle
            dict set entry structmsgData $structmsgData
            set handles [lreplace $handles $idx $idx $entry]
            dict set structmsgHandles handles $handles
            set header [list]
            return $::STRUCT_MSG_ERR_OK
          }
          incr idx
        }
        set entry [dict create]
        dict set entry handle $handle
        dict set entry structmsgData $structmsgData
        dict lappend structmsgHandles handles $entry
        set header [list]
        dict incr structmsgHandles numHandles 1
      }
      return $::STRUCT_MSG_ERR_OK;
    }
    
    # ============================= deleteHandle ========================
    
    proc deleteHandle {handle} {
      int idx;
      int numUsed;
      int found;
    
      if {structmsgHandles.handles == NULL} {
        return STRUCT_MSG_ERR_HANDLE_NOT_FOUND;
      }
      found = 0;
      idx = 0;
      numUsed = 0;
      while {idx < structmsgHandles.numHandles} {
        if {(structmsgHandles.handles[idx].handle != NULL} && (c_strcmp(structmsgHandles.handles[idx].handle, handle) == 0)) {
          structmsgHandles.handles[idx].handle = NULL;
          found++;
        } else {
          if {structmsgHandles.handles[idx].handle != NULL} {
            numUsed++;
          }
        }
        idx++;
      }
      if {numUsed == 0} {
        os_free{structmsgHandles.handles};
        structmsgHandles.handles = NULL;
      }
      if {$found} {
          return STRUCT_MSG_ERR_OK
      }
      return $::STRUCT_MSG_ERR_HANDLE_NOT_FOUND
    }
    
    # ============================= checkHandle ========================
    
    proc checkHandle {handle structmsgData} {
      variable structmsgHandles
      variable structmsgData
    
      if {[dict get $structmsgHandles handles] eq [list]} {
        return $::STRUCT_MSG_ERR_HANDLE_NOT_FOUND
      }
      set idx 0
      set handles [dict get structmsgHandles handles]
      while {$idx < [dict get $structmsgHandles numHandles]} {
        set entry [lindex $handles $idx]
        if {([dict get $entry handle] ne [list]) && ([dict get $entry handle] eq $handle)} {
          set structmsgData [dict get $entry structmsgData]
          return $::STRUCT_MSG_ERR_OK
        }
        incr idx
      }
      return $::STRUCT_MSG_ERR_HANDLE_NOT_FOUND
    }
    
    # ============================= getMsgData ========================
    
    proc getMsgData {dataVar lgthVar} {
      variable structmsgData
      upvar $dataVar data
      upvar $lgthVar lgth

      if {[lsearch [dict get $structmsgData flags] STRUCT_MSG_IS_INITTED] < 0} {
        return $::STRUCT_MSG_ERR_NOT_YET_INITTED
      }
      if {[lsearch [dict get $structmsgData flags] STRUCT_MSG_IS_PREPARED] < 0} {
        return $::STRUCT_MSG_ERR_NOT_YET_PREPARED
      }
      set result [::structmsg dataView getData data lgth]
      set lgth [dict get $structmsgData totalLgth]
      return $result
    }
    
    # ============================= setMsgData ========================
    
    proc setMsgData {data lgth} {
      variable structmsgData

      if {[lsearch [dct get $structmsgData flags] STRUCT_MSG_IS_INITTED] < 0} {
        return $::STRUCT_MSG_ERR_NOT_YET_INITTED
      }
      set found false
      # check lgth
      while {$idx < [dict get $structmsgData numFields]} {
        set fields [dict get $structmsgData fields]
        set fieldInfo [linex $fields $idx]
        if {[dict get $fieldInfo fieldNameId] eq "STRUCT_MSG_SPEC_FIELD_TOTAL_LGTH"} {
          set found true
          break
        }
        incr idx
      }
      if {! $found} {
        return $::STRUCT_MSG_ERR_NO_SUCH_FIELD
      } 
      # temporary replace data entry of dataView by our param data
      # to be able to use the get* functions for gettting totalLgth entry value
      set result [::structmsg dataView getData saveData saveLgth]
      set result [::structmsg dataView setData $data [expr {[dict get $fieldOffset fieldOffset] + 4}]]
      # get totalLgth value from data
      set result [::structmsg dataView getUint16 [dict get $fieldInfo fieldOffset] lgth]
      checkErrOK{result};
      if {$lgth != [dict get $structmsgData totalLgth]} {
        return $::STRUCT_MSG_ERR_BAD_DATA_LGTH
      }
      # now make a copy of the data to be on the safe side
      # for freeing the Lua space in Lua set the variable to nil!!
      set result [::structmsg dataView setData $data [dict get $structmsgDate totalLgth]
      # and now set the IS_SET flags and other stuff
      set idx 0
      while {$idx < [dict get $structmsgData numFields]} {
        set fields [dict get $structmsgData fields]
        set fieldInfo [lindex $fields $idx]
        dict lappend fieldInfo fieldFlags STRUCT_MSG_FIELD_IS_SET
        set fields [lreplace $fields $idx$idx $fieldInfo]
        dict set structmsgData fields $fields
        incr idx
      }
      dict lappend structmsgData flags STRUCT_MSG_IS_PREPARED
      return $::STRUCT_MSG_ERR_OK
    }

    # ============================= structmsgGetPtrFromHandle ========================
    
    proc structmsgGetPtrFromHandle {handle} {
      if {[checkHandle $handle] != $::STRUCT_MSG_ERR_OK} {
        return $::STRUCT_MSG_ERR_HANDLE_NOT_FOUND
      }
      return $::STRUCT_MSG_ERR_OK
    }

    # ================================= createMsg ====================================

    proc createMsg {numFields handleVar} {
      upvar $handleVar handle
      variable numHandles
      variable structmsgData

      dict set structmsgData fields [list]
      dict set structmsgData tableFields [list]
      dict set structmsgData flags [list]
      dict set structmsgData numFields 0
      dict set structmsgData maxFields $numFields
      dict set structmsgData numTableRows 0
      dict set structmsgData numTableRowFields 0
      dict set structmsgData numRowFields 0
      dict set structmsgData fieldOffset 0
      dict set structmsgData totalLgth 0
      dict set structmsgData cmdLgth 0
      dict set structmsgData headerLgth 0
      dict set structmsgData header [list]
      incr numHandles
      set handle [format "${::HANDLE_PREFIX}efff00%02d" $numHandles]
      set result [addHandle $handle [dict get $structmsgData header]]
      if {$result != $::STRUCT_MSG_ERR_OK} {
#        deleteHandle(self->handle);
        return $result
      }
      set handle [dict get $structmsgData handle]
      return $::STRUCT_MSG_ERR_OK
    }

    # ================================= addField ====================================
    
    proc  addField {fieldName fieldType fieldLgth} {
      variable structmsgData
    
      if {[dict get $structmsgData numFields] >= [dict get $structmsgData maxFields]} {
        return $::STRUCT_MSG_ERR_TOO_MANY_FIELDS
      }
      set result [::structmsg dataView getFieldTypeIdFromStr $fieldType fieldTypeId]
      if {$result != $::STRUCT_MSG_ERR_OK} {
        return $result
      }
      set result [::structmsg structmsgDataView getFieldNameIdFromStr $fieldName fieldNameId $::STRUCT_MSG_INCR]
      if {$result != $::STRUCT_MSG_ERR_OK} {
        return $result
      }
      set fieldInfo [dict create]
      # need to check for duplicate here !!
      if {$fieldName eq "@filler"} {
        dict lappend structmsgData flags STRUCT_MSG_HAS_FILLER
        set fieldLgth 0
        dict set fieldInfo fieldNameId $fieldNameId
        dict set fieldInfo fieldTypeId $fieldTypeId
        dict set fieldInfo fieldLgth $fieldLgth
        dict lappend structmsgData fields $fieldInfo
        dict incr structmsgData numFields 1
        return $::STRUCT_MSG_ERR_OK;
      }
      if {$fieldName eq "@tablerows"} {
        dict set structmsgData numTableRows $fieldLgth
        set fieldLgth 0
        dict lappend structmsgData flags STRUCT_MSG_HAS_TABLE_ROWS
        dict set fieldInfo fieldNameId $fieldNameId
        dict set fieldInfo fieldTypeId $fieldTypeId
        dict set fieldInfo fieldLgth $fieldLgth
        dict lappend structmsgData fields $fieldInfo
        dict incr structmsg numFields 1
        return $::STRUCT_MSG_ERR_OK;
      }
      if {$fieldName eq "@tablerowfields"} {
        dict set structmsgData numTableRowFields $fieldLgth
        set fieldLgth 0
        dict set fieldInfo fieldNameId $fieldNameId
        dict set fieldInfo fieldTypeId $fieldTypeId
        dict set fieldInfo fieldLgth $fieldLgth
        dict lappend structmsgData fields $fieldInfo
        set numTableFields [expr {[dict get $structmsgData numTableRows] * [dict get $structmsgData numTableRowFields]}]
        if {([dict get $structmsgData tableFields] eq [list]) && ($numTableFields != 0)} {
        }
        dict incr structmsgData numFields 1
        return $::STRUCT_MSG_ERR_OK;
      }
      set numTableRowFields [dict get $structmsgData numTableRowFields]
      set numTableRows [dict get $structmsgData numTableRows]
      set numTableFields [expr {$numTableRows * $numTableRowFields}]
      if {!(($numTableFields > 0) && ([dict get $structmsgData numRowFields] < $numTableRowFields)) || ($numTableRowFields == 0)} {
        set numTableFields 0
        set numTableRows 1
        set numTableRowFields 0
    
        if {$fieldName eq "@crc"} {
          dict lappend structmsgData flags STRUCT_MSG_HAS_CRC
          if {$fieldType eq "uint8_t"} {
            dict lappend structmsgData flags STRUCT_MSG_UINT8_CRC
          }
        }
        dict set fieldInfo fieldNameId $fieldNameId
        dict set fieldInfo fieldTypeId $fieldTypeId
        dict set fieldInfo fieldLgth $fieldLgth
        dict lappend structmsgData fields $fieldInfo
        dict incr structmsgData numFields 1
      } else {
        set row 0
        set tableFields [list]
        set tabIdx 0
        set numTabFlds [expr {$numTableRowFields * $numTableRows}]
        if {[llength [dict get $structmsgData tableFields]] == 0} {
          while {$tabIdx < $numTabFlds} {
            lappend tableFields [list]
            incr tabIdx
          }
puts stderr "ll tableFields![llength $tableFields]!"
          dict set structmsgData tableFields $tableFields
        }
        set tableFields [dict get $structmsgData tableFields]
        while {$row < $numTableRows} {
          set cellIdx [expr {[dict get $structmsgData numRowFields] + $row * $numTableRowFields}]
          set fieldInfo [dict create]
    #ets_printf{"table field1: %s totalLgth: %d cmdLgth: %d\n", fieldInfo->fieldStr, structmsg->hdr.hdrInfo.hdrKeys.totalLgth, structmsg->hdr.hdrInfo.hdrKeys.cmdLgth};
puts stderr "TAB!$fieldNameId!$row!$cellIdx!"
          dict set fieldInfo fieldNameId $fieldNameId
          dict set fieldInfo fieldTypeId $fieldTypeId
          dict set fieldInfo fieldLgth $fieldLgth
          set tableFields [lreplace $tableFields $cellIdx $cellIdx $fieldInfo]
          incr row
        }
        dict set structmsgData tableFields $tableFields
puts stderr "tableFields!$tableFields!"
        dict incr structmsgData numRowFields 1
      } 
      return $::STRUCT_MSG_ERR_OK;
    }

    # ================================= getFieldValue ====================================
    
    proc getFieldValue {fieldName valueVar} {
      variable structmsgData
      upvar $valueVar value
    
#puts stderr "getFieldValue:$fieldName!"
      if {[lsearch [dict get $structmsgData flags] STRUCT_MSG_IS_INITTED] < 0} {
        return $::STRUCT_MSG_ERR_NOT_YET_INITTED
      }
      set result [::structmsg structmsgDataView getFieldNameIdFromStr $fieldName fieldNameId STRUCT_MSG_NO_INCR]
      if {$result != $::STRUCT_MSG_ERR_OK} {
        return $result
      }
      set idx 0
      set numEntries [dict get $structmsgData numFields]
      while {$idx < $numEntries} {
        set fields [dict get $structmsgData fields]
        set fieldInfo [lindex $fields $idx]
        if {$fieldNameId == [dict get $fieldInfo fieldNameId]} {
          switch $fieldNameId {
            STRUCT_MSG_SPEC_FIELD_TABLE_ROWS {
             set value [dict get $structmsgData numTabRows]
             set result $::STRUCT_MSG_ERR_OK
            }
            STRUCT_MSG_SPEC_FIELD_TABLE_ROW_FIELDS {
             set value [dict get $structmsgData numTabRowFields]
             set result $::STRUCT_MSG_ERR_OK
            }
            default {
              set result [::structmsg structmsgDataView getFieldValue $fieldInfo value 0]
            }
          }
          if {$result != $::STRUCT_MSG_ERR_OK} {
            return $result
          }
          break
        }
        incr idx
      }
      return $::DATA_VIEW_ERR_OK
    }
    
    # ================================= setFieldValue ====================================
    
    proc setFieldValue {fieldName value} {
      variable structmsgData
     
      if {[lsearch [dict get $structmsgData flags] STRUCT_MSG_IS_INITTED] < 0} {
        return $::STRUCT_MSG_ERR_NOT_YET_INITTED
      }
      set result [::structmsg structmsgDataView getFieldNameIdFromStr $fieldName fieldNameId $::STRUCT_MSG_NO_INCR]
      if {$result != $::STRUCT_MSG_ERR_OK} {
        return $result
      }
      switch $fieldNameId {
        STRUCT_MSG_SPEC_FIELD_TOTAL_LGTH -
        STRUCT_MSG_SPEC_FIELD_CMD_LGTH -
        STRUCT_MSG_SPEC_FIELD_FILLER -
        STRUCT_MSG_SPEC_FIELD_CRC -
        STRUCT_MSG_SPEC_FIELD_RANDOM_NUM -
        STRUCT_MSG_SPEC_FIELD_SEQUENCE_NUM {
          return $::STRUCT_MSG_ERR_FIELD_CANNOT_BE_SET
        }
      }
      set idx 0
      set numEntries [dict get $structmsgData numFields]
    #ets_printf{"numEntries: %d\n", numEntries};
      while {$idx < $numEntries} {
        set fields [dict get $structmsgData fields]
        set fieldInfo [lindex $fields $idx]
        if {$fieldNameId == [dict get $fieldInfo fieldNameId]} {
          set result [::structmsg structmsgDataView setFieldValue $fieldInfo $value 0]
          if {$result != $::STRUCT_MSG_ERR_OK} {
            return $result
          }
          dict lappend  fieldInfo fieldFlags STRUCT_MSG_FIELD_IS_SET
          set fields [lreplace $fields $idx $idx $fieldInfo]
          dict set structmsgData fields $fields
          return $::DATA_VIEW_ERR_OK
        }
        incr idx
      }
    #ets_printf{"idx: %d\n", idx};
      return $::DATA_VIEW_ERR_FIELD_NOT_FOUND
    }
    
    
    # ================================= getTableFieldValue ====================================
    
    proc getTableFieldValue {fieldName row valueVar} {
      variable structmsgData
      upvar $valueVar value
    
#puts stderr "getTableFieldValue!"
      if {[lsearch [dict get $structmsgData flags] STRUCT_MSG_IS_INITTED] < 0} {
        return $::STRUCT_MSG_ERR_NOT_YET_INITTED
      }
      set result [::structmsg structmsgDataView getFieldNameIdFromStr $fieldName fieldNameId STRUCT_MSG_NO_INCR]
      if {$result != $::STRUCT_MSG_ERR_OK} {
        return $result
      }
      if {[string range $fieldName 0 0] eq "@"} {
        return $::STRUCT_MSG_ERR_NO_SUCH_FIELD
      }
      if {$row >= [dict get $structmsgData numTableRows]} {
        return $::STRUCT_MSG_ERR_BAD_TABLE_ROW
      }

      set idx 0
      set cellIdx [expr {$row * [dict get $structmsgData numRowFields]}]
      set tableFields [dict get $structmsgData tableFields]
      while {$idx < [dict get $structmsgData numRowFields]} {
        set fieldInfo [lindex $tableFields $cellIdx]
        if {$fieldNameId == [dict get $fieldInfo fieldNameId]} {
          return [::structmsg structmsgDataView getFieldValue $fieldInfo value 0]
        }
        incr cellIdx
        incr idx
      }
      return $::STRUCT_MSG_ERR_FIELD_NOT_FOUND
    }
    
    # ================================= setTableFieldValue ====================================
    
    proc setTableFieldValue {fieldName row value} {
      variable structmsgData
    
      if {(self->flags & STRUCT_MSG_IS_INITTED} == 0) {
        return STRUCT_MSG_ERR_NOT_YET_INITTED;
      }
      result = self->structmsgDataView->getFieldNameIdFromStr{self->structmsgDataView, fieldName, &fieldNameId, STRUCT_MSG_NO_INCR};
      checkErrOK{result};
      if {fieldName[0] == '@'} {
        return STRUCT_MSG_ERR_FIELD_CANNOT_BE_SET;
      }
      if {row >= self->numTableRows} {
        return STRUCT_MSG_ERR_BAD_TABLE_ROW;
      }
      idx = 0;
      cellIdx = 0 + row * self->numRowFields;
      while {idx < self->numRowFields} {
        fieldInfo = &self->tableFields[cellIdx];
        if {fieldNameId == fieldInfo->fieldNameId} {
          fieldInfo->fieldFlags |= STRUCT_MSG_FIELD_IS_SET;
          return self->structmsgDataView->setFieldValue{self->structmsgDataView, fieldInfo, numericValue, stringValue, 0};
        }
        cellIdx++;
        idx++;
      }
      return STRUCT_MSG_ERR_FIELD_NOT_FOUND;
    }
    
    # ================================= prepareMsg ====================================
    
    proc prepareMsg {} {
      variable structmsgData
    
      if {[lsearch [dict get $structmsgData flags] STRUCT_MSG_IS_INITTED] < 0} {
        return $::STRUCT_MSG_ERR_NOT_YET_INITTED
      }
      # create the values which are different for each message!!
      set numEntries [dict get $structmsgData numFields]
      set idx 0
      while {$idx < $numEntries} {
        set fields [dict get $structmsgData fields]
        set fieldInfo [lindex $fields $idx]
        switch [dict get $fieldInfo fieldNameId] {
          STRUCT_MSG_SPEC_FIELD_RANDOM_NUM {
            set result [::structmsg structmsgDataView setRandomNum $fieldInfo]
            if {$result != $::STRUCT_MSG_ERR_OK} {
              return $result
            }
            dict lappend fieldInfo fieldFlags STRUCT_MSG_FIELD_IS_SET
          }
          STRUCT_MSG_SPEC_FIELD_SEQUENCE_NUM {
            set result [::structmsg structmsgDataView setSequenceNum $fieldInfo]
            if {$result != $::STRUCT_MSG_ERR_OK} {
              return $result
            }
            dict lappend fieldInfo fieldFlags STRUCT_MSG_FIELD_IS_SET
          }
          STRUCT_MSG_SPEC_FIELD_FILLER {
            set result [::structmsg structmsgDataView setFiller $fieldInfo]
            if {$result != $::STRUCT_MSG_ERR_OK} {
              return $result
            }
            dict lappend fieldInfo fieldFlags STRUCT_MSG_FIELD_IS_SET
          }
          STRUCT_MSG_SPEC_FIELD_CRC {
            set startOffset 0
            if {[lsearch [dict get $structmsgData flags] STRUCT_MSG_CRC_USE_HEADER_LGTH] >= 0} {
                set startOffset [dict get $structmsgData headerLgth]
            }
            set lgth [dict get $structmsgData totalLgth]
            set result [::structmsg structmsgDataView setCrc $fieldInfo $startOffset $lgth]
            if {$result != $::STRUCT_MSG_ERR_OK} {
              return $result
            }
            dict lappend fieldInfo fieldFlags STRUCT_MSG_FIELD_IS_SET
          }
        }
        incr idx
      }
      dict lappend structmsgData flags STRUCT_MSG_IS_PREPARED
      return $::STRUCT_MSG_ERR_OK
    }
    
    # ================================= addFlag ====================================
    
    proc addFlag {flag} {
      variable structmsgData
    
      dict lappend structmsgData flags $flag
      return $::STRUCT_MSG_ERR_OK
    }

    # ================================= deleteFlag ====================================
    
    proc deleteFlag {flag} {
      variable structmsgData
    
    }

    # ================================= initMsg ====================================
    
    proc initMsg {numTableRows numTableRowFields} {
      variable structmsgData
    
      # initialize field offsets for each field
      # initialize totalLgth, headerLgth, cmdLgth
      if {[lsearch [dict get $structmsgData flags] STRUCT_MSG_IS_INITTED] >= 0} {
        return $::STRUCT_MSG_ERR_ALREADY_INITTED;
      }
      dict set structmsgData fieldOffset 0
      set numEntries [dict get $structmsgData numFields]
      set idx 0
      dict set structmsgData headerLgth 0
      while {$idx < $numEntries} {
        set fields [dict get $structmsgData fields]
        set fieldInfo [lindex $fields $idx]
        dict set fieldInfo fieldOffset [dict get $structmsgData fieldOffset]
        switch [dict get $fieldInfo fieldNameId] {
          STRUCT_MSG_SPEC_FIELD_SRC -
          STRUCT_MSG_SPEC_FIELD_DST -
          STRUCT_MSG_SPEC_FIELD_TOTAL_LGTH {
    #      STRUCT_MSG_SPEC_FIELD_GUID -
            dict incr structmsgData headerLgth [dict get $fieldInfo fieldLgth]
            dict set structmsgData totalLgth [expr {[dict get $structmsgData fieldOffset] + [dict get $fieldInfo fieldLgth]}]
          }
          STRUCT_MSG_SPEC_FIELD_TABLE_ROWS {
#            dict set fieldInfo fieldLgth $numTableRows
dict set structmsgData numTabRows $numTableRows
          }
          STRUCT_MSG_SPEC_FIELD_TABLE_ROW_FIELDS {
#            dict set fieldInfo fieldLgth $numTableRowFields
dict set structmsgData numTabRowFields $numTableRowFields
            set row 0
            set col 0
            set tableFields [dict get $structmsgData tableFields]
            while {$row < [dict get $structmsgData numTableRows]} {
              while {$col < [dict get $structmsgData numTableRowFields]} {
                set cellIdx [expr {$col + $row * [dict get $structmsgData numRowFields]}]
                set fieldInfo2 [lindex $tableFields $cellIdx]
                dict set fieldInfo2 fieldOffset [dict get $structmsgData fieldOffset]
                set tableFields [lreplace $tableFields $cellIdx $cellIdx $fieldInfo2]
                dict set structmsgData tableFields $tableFields
                dict incr structmsgData fieldOffset [dict get $fieldInfo2 fieldLgth]
                incr col
              }
              set col 0 
              incr row
            }
          }
          STRUCT_MSG_SPEC_FIELD_FILLER {
            set fillerLgth 0
            set crcLgth 0
            if {[lsearch [dict get $structmsgData flags] STRUCT_MSG_HAS_CRC] >= 0} {
              if {[lsearch [dict get $structmsgData flags] STRUCT_MSG_UINT8_CRC] >= 0} {
                set crcLgth 1
              } else {
                set crcLgth 2
              }
            }
            set myLgth [expr {[dict get $structmsgData fieldOffset] + $crcLgth - [dict get $structmsgData headerLgth]}]
            while {[expr {$myLgth % 16}] != 0} {
              incr myLgth
              incr fillerLgth
            }
            dict set fieldInfo fieldLgth $fillerLgth
            set fields [lreplace $fields $idx $idx $fieldInfo]
            dict set structmsgData totalLgth [expr {[dict get $structmsgData fieldOffset] + $fillerLgth + $crcLgth}]
            dict set structmsgData cmdLgth [expr {[dict get $structmsgData totalLgth] - [dict get $structmsgData headerLgth]}]
          }
          default {
            dict set structmsgData totalLgth [expr {[dict get $structmsgData fieldOffset] + [dict get $fieldInfo fieldLgth]}]
            dict set structmsgData cmdLgth [expr {[dict get $structmsgData totalLgth] - [dict get $structmsgData headerLgth]}]
          }
        }
        set fields [lreplace $fields $idx $idx $fieldInfo]
        dict set structmsgData fields $fields
        dict incr structmsgData fieldOffset [dict get $fieldInfo fieldLgth]
        incr idx
      }
      if {[dict get $structmsgData totalLgth] == 0} {
        return $::STRUCT_MSG_ERR_FIELD_TOTAL_LGTH_MISSING;
      }
      set totalLgth [dict get $structmsgData totalLgth]
      set result [::structmsg dataView setData [string repeat " " $totalLgth] $totalLgth]
      if {$result != $::STRUCT_MSG_ERR_OK} {
        return $result
      }
      dict lappend structmsgData flags STRUCT_MSG_IS_INITTED
      # set the appropriate field values for the lgth entries
      set idx 0
      set numEntries [dict get $structmsgData numFields]
      set fields [dict get $structmsgData fields]
      while {$idx < $numEntries} {
        set fieldInfo [lindex $fields $idx]
        switch [dict get $fieldInfo fieldNameId] {
          STRUCT_MSG_SPEC_FIELD_TOTAL_LGTH {
            set result [::structmsg structmsgDataView setFieldValue $fieldInfo [dict get $structmsgData totalLgth] 0]
            if {$result != $::STRUCT_MSG_ERR_OK} {
              return $result
            }
            dict lappend fieldInfo fieldFlags STRUCT_MSG_FIELD_IS_SET
          }
          STRUCT_MSG_SPEC_FIELD_CMD_LGTH {
            set result [:.structmsg structmsgDataView setFieldValue $fieldInfo [dict get $structmsgData cmdLgth] 0]
            if {$result != $::STRUCT_MSG_ERR_OK} {
              return $result
            }
            dict lappend fieldInfo fieldFlags STRUCT_MSG_FIELD_IS_SET
          }
        }
        set fields [lreplace $fields $idx $idx $fieldInfo]
        incr idx
      }
      dict set structmsgData fields $fields
puts stderr "initMsg done!"
      return $::STRUCT_MSG_ERR_OK
    }
    
  } ; # namespace structmsgData
} ; # namespace structmsg
