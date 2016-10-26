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

namespace eval structmsg {
  namespace ensemble create

  namespace export listMsg

  namespace eval listMsg {
    namespace ensemble create
      
    variable structmsgData

    set structmsgData [dict create]

    namespace export newStructmsgListMsg setListMsg addListMsgField dumpListMsgFields
    namespace export setListMsgFieldValue getListMsgFieldValue

    # ================================= dumpListMsgs ====================================

    proc dumpListMsgss {fieldInfo indent2 fieldIdx} {
      variable structmsgData

      set valueIdx 0
      while {$valueIdx < [expr {[dict get $fieldInfo fieldLgth] / 2}]} {
        set result [::structmsg structmsgDataView getFieldValue $fieldInfo fieldNameId $valueIdx]
        if {$result != $::STRUCT_MSG_ERR_OK} {
          return $result
        }
        incr valueIdx
        set result [::structmsg structmsgDataView getFieldValue $fieldInfo fieldTypeId $valueIdx]
        if {$result != $::STRUCT_MSG_ERR_OK} {
          return $result
        }
        incr valueIdx
        set result [::structmsg structmsgDataView getFieldValue $fieldInfo fieldLgth $valueIdx]
        if {$result != $::STRUCT_MSG_ERR_OK} {
          return $result
        }
        incr valueIdx
        set result [::structmsg dataView getFieldTypeStrFromId $fieldTypeId fieldTypeStr]
        if {$result != $::STRUCT_MSG_ERR_OK} {
          return $result
        }
        set fieldNameInt $fieldNameId
        if {![string is integer $fieldNameId]} {
          set result [::structmsg structmsgDataView getSpecialFieldNameIntFromId $fieldNameId fieldNameInt]
          if {$result != $::STRUCT_MSG_ERR_OK} {
            return $result
          }
        }
        if {$fieldNameInt > $::STRUCT_MSG_SPEC_FIELD_LOW} {
          # yes it is a special name 
          set result [::structmsg structmsgDataView getFieldNameStrFromId $fieldNameId fieldNameStr]
        } else {
          set idx $fieldNameId
          while {true} {
            set ch [string range $names $idx $idx]
            set pch $ch
            if {![string is integer $ch]} {
              binary scan $ch c pch
            }
            if {$pch == 0} {
              break
            }
            incr idx
          }
          set fieldNameStr [string range $names $fieldNameId $idx]
        }
        puts stderr [format "        defIdx: %3d fieldName: %3d %-20s fieldType: %3d %-8s fieldLgth: %5d" [expr {$valueIdx/3}] $fieldNameId $fieldNameStr $fieldTypeId $fieldTypeStr $fieldLgth]
      }
      return $::DATA_VIEW_ERR_OK
    }

    # ================================= dumpListMsgFieldValue ====================================

    proc dumpListMsgFieldValue {fieldInfo indent2 fieldIdx} {
      variable structmsgData

      set result [::structmsg structmsgDataView getFieldValue $fieldInfo value $fieldIdx]
      if {$result != $::STRUCT_MSG_ERR_OK} {
        return $result
      }
      switch [dict get $fieldInfo fieldTypeId] {
      DATA_VIEW_FIELD_INT8_T {
        puts stderr [format "      %svalue: 0x%02x %d" $indent2 [expr {$value & 0xFF}] $value]
      }
      DATA_VIEW_FIELD_UINT8_T {
        puts stderr [format "      %svalue: 0x%02x %d" $indent2 [expr {$value & 0xFF}] [expr {$value & 0xFF}]]
      }
      DATA_VIEW_FIELD_INT16_T {
        puts stderr [format "      %svalue: 0x%04x %d" $indent2 [expr {$value & 0xFFFF}] $value]
      }
      DATA_VIEW_FIELD_UINT16_T {
        puts stderr [format "      %svalue: 0x%04x %d" $indent2 [expr {$value & 0xFFFF}] [expr {$value & 0xFFFF}]]
      }
      DATA_VIEW_FIELD_INT32_T {
        puts stderr [format "      %svalue: 0x%08x %d" $indent2 [expr {$value & 0xFFFFFFFF}] $value]
      }
      DATA_VIEW_FIELD_UINT32_T {
        puts stderr [format "      %svalue: 0x%08x %d" $indent2 [expr {$value & 0xFFFFFFFF}] [expr {$value & 0xFFFFFFFF}]]
      }
      DATA_VIEW_FIELD_INT8_VECTOR {
        set valueIdx 0
        puts stderr [format "      %svalues:" $indent2]
        while {$valueIdx < [dict get $fieldInfo fieldLgth]} {
          set ch [string range $value $valueIdx $valueIdx]
          set pch $ch
          if {![string is integer $ch]} {
            binary scan $ch c pch
          }
          puts stderr [format "        %sidx: %d value: %c 0x%02x" $indent2 $valueIdx $ch [exp {$pch & 0xFF}]]
          incr valueIdx
        }
        puts stderr ""
      }
      DATA_VIEW_FIELD_UINT8_VECTOR {
        set valueIdx 0
        puts stderr [format "      %svalues:" $indent2]
        while {$valueIdx < [dict get $fieldInfo fieldLgth]} {
          set uch [string range $value $valueIdx $valueIdx]
          set pch $uch
          if {![string is integer $uch]} {
            binary scan $uch c pch
          }
          puts stderr [format "        %sidx: %d value: %s 0x%02x" $indent2 $valueIdx $uch [expr {$pch & 0xFF}]]
          incr valueIdx
        }
        puts stderr ""
      }
      DATA_VIEW_FIELD_INT16_VECTOR {
        set valueIdx 0
        puts stderr [format "      %svalues:" $indent2]
        while {$valueIdx < [expr {[dict get $fieldInfo fieldLgth]/2}]} {
          set result [::structmsg dataView getInt16 [expr {[dict get $fieldInfo fieldOffset]+$valueIdx*2}] value]
          if {$result != $::STRUCT_MSG_ERR_OK} {
            return $result
          }
          puts stderr [format "        %sidx: %d value: 0x%04x" $indent2 $valueIdx $value]
          incr valueIdx
        }
        puts stderr ""
        puts stderr ""
      }
      DATA_VIEW_FIELD_UINT16_VECTOR {
        set valueIdx 0
        puts stderr [format "      %svalues:" $indent2]
        while {$valueIdx < [expr {[dict get $fieldInfo fieldLgth]/2}]} {
          set result [::structmsg structmsgDataView getFieldValue $fieldInfo value $valueIdx]
          if {$result != $::STRUCT_MSG_ERR_OK} {
            return $result
          }
          puts stderr [format "        %sidx: %d value: 0x%04x" $indent2 $valueIdx [expr {$value & 0xFFFF}]]
          incr valueIdx
        }
      }
      DATA_VIEW_FIELD_INT32_VECTOR {
        set valueIdx 0
        puts stderr [format "      %svalues:" $indent2]
        while {$valueIdx < [dict get $fieldInfo fieldLgth]} {
          set val [lindex [dict get $fieldInfo value] $valueIdx]
          puts stderr [format "        %sidx: %d value: 0x%08x" $indent2 $valueIdx [expr {$val & 0xFFFFFFFF}]]
          incr valueIdx
        }
        puts stderr ""
      }
      DATA_VIEW_FIELD_UINT32_VECTOR {
        set valueIdx 0
        puts stderr [format "      %svalues:" indent2]
        while {$valueIdx < [dict get $fieldInfo fieldLgth]} {
          set uval [lindex [dict get $fieldInfo value] $valueIdx]
          puts stderr [format "        %sidx: %d value: 0x%08x" $indent2 $valueIdx, [expr {$uval & 0xFFFFFFFF}]]
          incr valueIdx
        }
      }

      }
      return $::DATA_VIEW_ERR_OK
    }

    # ============================= dumpListMsgFields ========================

    proc  dumpListMsgFields {} {
      variable structmsgData

      set numEntries [dict get $structmsgData numListMsgFields]
      puts stderr [format "    numListMsgFields: %d" $numEntries]
      set idx 0
      set defFields [dict get $structmsgData listMsgFields]
      while {$idx < $numEntries} {
        set fieldInfo [lindex $defFields $idx]
        set result [::structmsg dataView getFieldTypeStrFromId [dict get $fieldInfo fieldTypeId] fieldTypeStr]
        if {$result != $::STRUCT_MSG_ERR_OK} {
          return $result
        }
        set result [::structmsg structmsgDataView getFieldNameStrFromId [dict get $fieldInfo fieldNameId] fieldNameStr]
        if {$result != $::STRUCT_MSG_ERR_OK} {
          return $result
        }
        puts stderr [format "      idx: %d fieldName: %-20s fieldType: %-8s fieldLgth: %.5d offset: %d" $idx $fieldNameStr $fieldTypeStr [dict get $fieldInfo fieldLgth] [dict get $fieldInfo fieldOffset]]
        if {[lsearch [dict get $fieldInfo fieldFlags] STRUCT_MSG_FIELD_IS_SET] >= 0} {
          if {[dict get $fieldInfo fieldNameId] eq "STRUCT_MSG_SPEC_FIELD_LIST_MSGS"} {
#            if {$result != $::STRUCT_MSG_ERR_OK} {
#              return $result
#            }
          } else {
            set result [dumpListMsgFieldValue $fieldInfo "  " 0]
            if {$result != $::STRUCT_MSG_ERR_OK} {
              return $result
            }
          }
        }
        incr idx
      }
      return $::STRUCT_MSG_ERR_OK
    }

    # ============================= addListMsgField ========================

    proc addListMsgField {fieldNameId fieldTypeId fieldLgth} {
      variable structmsgData

      if {[dict get $structmsgData numListMsgFields] >= $::STRUCT_DEF_NUM_DEF_FIELDS} {
        return $::STRUCT_MSG_ERR_TOO_MANY_FIELDS
      }
      set listMsgFieldInfo [dict create]
      dict set listMsgFieldInfo fieldNameId $fieldNameId;
      dict set listMsgFieldInfo fieldTypeId $fieldTypeId;
      dict set listMsgFieldInfo fieldLgth $fieldLgth;
      dict set listMsgFieldInfo fieldOffset [dict get $structmsgData listMsgFieldOffset]
      dict lappend structmsgData listMsgFields $listMsgFieldInfo
      dict incr structmsgData listMsgFieldOffset $fieldLgth
      dict incr structmsgData numListMsgFields 1
      return $::STRUCT_MSG_ERR_OK
    }

    # ============================= getlistMsgFieldValue ========================

    proc getListMsgFieldValue {fieldNameId valueVar fieldIdx} {
      variable structmsgData
      upvar $valueVar value

      if {[lsearch [dict get $structmsgData flags] STRUCT_LIST_IS_INITTED] < 0} {
        return $::STRUCT_LIST_ERR_NOT_YET_INITTED
      }
      set found false
      set idx 0
      set numEntries [dict get $structmsgData numListMsgFields]
      set listMsgFields [dict get $structmsgData listMsgFields]
      while {$idx < $numEntries} {
        set fieldInfo [lindex $listMsgFields $idx]
        if {$fieldNameId == [dict get $fieldInfo fieldNameId]} {
          set result [::structmsg structmsgDataView getFieldValue $fieldInfo value $fieldIdx]
          return $result
        }
        incr idx
      }
      if {!$found} {
puts stderr [format "fieldNameId: %d found: %d" $fieldNameId $found]
        return $::STRUCT_MSG_ERR_FIELD_NOT_FOUND
      }
    }

    # ============================= setListMsgFieldValue ========================

    proc  setListMsgFieldValue {fieldNameId value fieldIdx} {
      variable structmsgData

      set found false
      if {[lsearch [dict get $structmsgData flags] STRUCT_LIST_IS_INITTED] < 0} {
        return $::STRUCT_LIST_ERR_NOT_YET_INITTED
      }
      set idx 0
      set numEntries [dict get $structmsgData numListMsgFields]
      set listMsgFields [dict get $structmsgData listMsgFields]
      while {$idx < $numEntries} {
        set fieldInfo [lindex $listMsgFields $idx]
        if {$fieldNameId == [dict get $fieldInfo fieldNameId]} {
          set result [::structmsg structmsgDataView setFieldValue $fieldInfo $value $fieldIdx]
          if {$result != $::STRUCT_MSG_ERR_OK} {
            return $result
          }
          dict lappend fieldInfo fieldFlags STRUCT_MSG_FIELD_IS_SET
          set listMsgFields [lreplace $listMsgFields $idx $idx $fieldInfo]
          set found true
          break
        }
        incr idx
      }
      dict set structmsgData listMsgFields $listMsgFields
      if {!$found} {
puts stderr [format "fieldNameId: %d found: %d" $fieldNameId $found]
        return $::STRUCT_MSG_ERR_FIELD_NOT_FOUND
      }
      return $::STRUCT_MSG_ERR_OK
    }

    # ============================= addListMsgFields ========================

    proc addListMsgFields {direction} {
      variable structmsgData

puts stderr "addListMsgFields!"
      set result [addListMsgField STRUCT_MSG_SPEC_FIELD_DST DATA_VIEW_FIELD_UINT16_T 2]
      if {$result != $::STRUCT_MSG_ERR_OK} {
        return $result
      }
      set result [addListMsgField STRUCT_MSG_SPEC_FIELD_SRC DATA_VIEW_FIELD_UINT16_T 2]
      if {$result != $::STRUCT_MSG_ERR_OK} {
        return $result
      }
      set result [addListMsgField STRUCT_MSG_SPEC_FIELD_TOTAL_LGTH DATA_VIEW_FIELD_UINT16_T 2]
      if {$result != $::STRUCT_MSG_ERR_OK} {
        return $result
      }
      dict set structmsgData listMsgHeaderLgth [dict get $structmsgData listMsgFieldOffset]
      set result [addListMsgField STRUCT_MSG_SPEC_FIELD_CMD_KEY DATA_VIEW_FIELD_UINT16_T 2]
      if {$result != $::STRUCT_MSG_ERR_OK} {
        return $result
      }
      if {$direction eq "STRUCT_LIST_FROM_DATA"} {
        set result [::structmsg dataView getUint8 [dict get $structmsgData listMsgFieldOffset] numListMsgs]
        if {$result != $::STRUCT_MSG_ERR_OK} {
          return $result
        }
      }
      set result [addListMsgField STRUCT_MSG_SPEC_FIELD_NUM_LIST_MSGS DATA_VIEW_FIELD_UINT8_T 1]
      if {$result != $::STRUCT_MSG_ERR_OK} {
        return $result
      }
      set offset [dict get $structmsgData listMsgFieldOffset]
      set result [addListMsgField STRUCT_MSG_SPEC_FIELD_LIST_MSG_SIZES_SIZE DATA_VIEW_FIELD_UINT16_VECTOR [expr {$numListMsgs * 2}]]
      if {$result != $::STRUCT_MSG_ERR_OK} {
        return $result
      }
      set msgsSize 0
      if {$direction eq "STRUCT_LIST_FROM_DATA"} {
        set msgIdx 0
        while {$msgIdx < $numListMsgs} {
          set result [::structmsg dataView getUint16 $offset msgSize]
          if {$result != $::STRUCT_MSG_ERR_OK} {
            return $result
          }
          incr offset 2
          incr msgsSize $msgSize
          incr msgIdx
        }
      }

      set result [addListMsgField STRUCT_MSG_SPEC_FIELD_LIST_MSGS DATA_VIEW_FIELD_UINT8_VECTOR $msgsSize]
      if {$result != $::STRUCT_MSG_ERR_OK} {
        return $result
      }
      set fillerLgth 0
      set crcLgth 2
      set myLgth [expr {[dict get $structmsgData listMsgFieldOffset] + $crcLgth - [dict get $structmsgData listMsgHeaderLgth]}]
      while {($myLgth % 16) != 0} {
        incr myLgth
        incr fillerLgth
      }
      set result [addListMsgField STRUCT_MSG_SPEC_FIELD_FILLER DATA_VIEW_FIELD_UINT8_VECTOR $fillerLgth]
      if {$result != $::STRUCT_MSG_ERR_OK} {
        return $result
      }
      set result [addListMsgField STRUCT_MSG_SPEC_FIELD_CRC DATA_VIEW_FIELD_UINT16_T 2]
      if {$result != $::STRUCT_MSG_ERR_OK} {
        return $result
      }
      dict lappend structmsgData flags STRUCT_LIST_IS_INITTED
      if {$direction eq "STRUCT_LIST_TO_DATA"} {
        set data [string repeat " " [dict get $structmsgData listMsgFieldOffset]]
        set result [::structmsg dataView setData $data [dict get $structmsgData listMsgFieldOffset]]
      }
      dict set structmsgData listMsgTotalLgth [dict get $structmsgData listMsgFieldOffset]
puts stderr "addListMsgFields done"
      return $::STRUCT_MSG_ERR_OK
    }

    # ============================= setStaticListMsgFields ========================

    proc setStaticListMsgFields {} {
      variable structmsgData

    # FIXME src and dst are dummy values for now!!
      result = setListMsgFieldValue{self, STRUCT_MSG_SPEC_FIELD_DST, 16640, NULL, 0};
      checkErrOK{result};
      result = setListMsgFieldValue{self, STRUCT_MSG_SPEC_FIELD_SRC, 22272, NULL, 0};
      checkErrOK{result};
      result = setListMsgFieldValue{self, STRUCT_MSG_SPEC_FIELD_TOTAL_LGTH, self->defFieldOffset, NULL, 0};
      checkErrOK{result};
      result = setListMsgFieldValue{self, STRUCT_MSG_SPEC_FIELD_CMD_KEY, STRUCT_DEF_CMD_KEY, NULL, 0};
      checkErrOK{result};
      result = setListMsgFieldValue{self, STRUCT_MSG_SPEC_FIELD_NUM_LIST_MSGS, numListMsgs, NULL, 0};
      checkErrOK{result};
      idx = 0;
      while {idx < numNormFields} {
        result = setDefFieldValue{self, STRUCT_MSG_SPEC_FIELD_NORM_FLD_IDS, normNamesOffsets[idx].offset, NULL, idx};
        checkErrOK{result};
        idx++;
      }
      result = setDefFieldValue{self, STRUCT_MSG_SPEC_FIELD_NORM_FLD_NAMES_SIZE, normNamesSize, NULL, 0};
      checkErrOK{result};
      uint8_t names[normNamesSize];
      idx = 0;
      namesOffset = 0;
      while {idx < numNormFields} {
        c_memcpy{names+namesOffset, normNamesOffsets[idx].name,c_strlen(normNamesOffsets[idx].name});
ets_printf{"name: idx: %d %s\n", idx, normNamesOffsets[idx].name};
        namesOffset += c_strlen{normNamesOffsets[idx].name};
        names[namesOffset] = 0;
        namesOffset++;
        idx++;
      }
      result = setDefFieldValue{self, STRUCT_MSG_SPEC_FIELD_NORM_FLD_NAMES 0 names, 0};
      checkErrOK{result};
      result = setDefFieldValue{self, STRUCT_MSG_SPEC_FIELD_DEFINITIONS_SIZE, definitionsSize, NULL, 0};
      checkErrOK{result};
      fieldIdx = 0;
      idx = 0;
      namesIdx = 0;
      while {idx < self->numFields} {
        fieldInfo = &self->fields[idx];
        if {fieldInfo->fieldNameId < STRUCT_MSG_SPEC_FIELD_LOW} {
          result = setDefFieldValue{self, STRUCT_MSG_SPEC_FIELD_DEFINITIONS, normNamesOffsets[namesIdx++].offset, NULL, fieldIdx++};
          checkErrOK{result};
        }
        idx++;
      }
      return STRUCT_MSG_ERR_OK;
    }

    # ============================= initListMsg ========================

    proc initListMsg {} {
      variable structmsgData

      if {(self->flags & STRUCT_LIST_IS_INITTED} != 0) {
        return STRUCT_LIST_ERR_ALREADY_INITTED;
      }
      numFields = self->numListMsgFields;
      idx = 0;
      while {idx < self->numFields} {
        fieldInfo = &self->fields[idx];
        if {fieldInfo->fieldNameId < STRUCT_MSG_SPEC_FIELD_LOW} {
          result = self->structmsgDataView->getFieldNameStrFromId{self->structmsgDataView, fieldInfo->fieldNameId, &fieldNameStr};
          checkErrOK{result};
          normNamesOffsets[numNormFields].id = fieldInfo->fieldNameId;
          normNamesOffsets[numNormFields].offset = namesOffset;
          normNamesOffsets[numNormFields].name = fieldNameStr;
          normNamesSize += c_strlen{fieldNameStr} + 1;
          namesOffset += c_strlen{fieldNameStr} + 1;
          numNormFields++;
        } else {
          if {fieldInfo->fieldNameId == STRUCT_MSG_SPEC_FIELD_TABLE_ROW_FIELDS} {
            tabIdx = 0;
            while {tabIdx < self->numTableRowFields} {
              tabFieldInfo = &self->tableFields[tabIdx];
              result = self->structmsgDataView->getFieldNameStrFromId{self->structmsgDataView, tabFieldInfo->fieldNameId, &fieldNameStr};
              checkErrOK{result};
              normNamesOffsets[numNormFields].id = fieldInfo->fieldNameId;
              normNamesOffsets[numNormFields].offset = namesOffset;
              normNamesOffsets[numNormFields].name = fieldNameStr;
              normNamesSize += c_strlen{fieldNameStr} + 1;
              namesOffset += c_strlen{fieldNameStr} + 1;
              numNormFields++;
              tabIdx++;
            }
          }
        }
        idx++;
      }
      result = addDefFields{self, numNormFields, normNamesSize, definitionsSize, STRUCT_DEF_TO_DATA};
      checkErrOK{result};
      result = setStaticDefFields{self, numNormFields, normNamesSize, normNamesOffsets, definitionsSize};
      checkErrOK{result};
      return STRUCT_MSG_ERR_OK;
    }

    # ============================= prepareListMsg ========================

    proc prepareListMsg {} {
      variable structmsgData

      if {(self->flags & STRUCT_DEF_IS_INITTED} == 0) {
ets_printf{"prepareDef not yet initted\n"};
        return STRUCT_DEF_ERR_NOT_YET_INITTED;
      }
          # create the values which are different for each message!!
      numEntries = self->numDefFields;
      idx = 0;
      while {idx < numEntries} {
        fieldInfo = &self->defFields[idx];
        switch {fieldInfo->fieldNameId} {
          case STRUCT_MSG_SPEC_FIELD_RANDOM_NUM:
            result = self->structmsgDefinitionDataView->setRandomNum{self->structmsgDefinitionDataView, fieldInfo};
            checkErrOK{result};
            fieldInfo->fieldFlags |= STRUCT_MSG_FIELD_IS_SET;
            break;
          case STRUCT_MSG_SPEC_FIELD_SEQUENCE_NUM:
            result = self->structmsgDefinitionDataView->setSequenceNum{self->structmsgDefinitionDataView, fieldInfo};
            checkErrOK{result};
            fieldInfo->fieldFlags |= STRUCT_MSG_FIELD_IS_SET;
            break;
          case STRUCT_MSG_SPEC_FIELD_FILLER:
            result = self->structmsgDefinitionDataView->setFiller{self->structmsgDefinitionDataView, fieldInfo};
            checkErrOK{result};
            fieldInfo->fieldFlags |= STRUCT_MSG_FIELD_IS_SET;
            break;
          case STRUCT_MSG_SPEC_FIELD_CRC:
            result = self->structmsgDefinitionDataView->setCrc{self->structmsgDefinitionDataView, fieldInfo, self->headerLgth+1, self->cmdLgth-fieldInfo->fieldLgth};
            checkErrOK{result};
            fieldInfo->fieldFlags |= STRUCT_MSG_FIELD_IS_SET;
            break;
        }
        idx++;
      }
      self->flags |= STRUCT_DEF_IS_PREPARED;
      return STRUCT_MSG_ERR_OK;
    }

    # ============================= setListMsg ========================

    proc setListMsg {data} {
      variable structmsgData

puts stderr "setListMsg1"
      dict set structmsgData numListMsgFields 0
      dict set structmsgData listMsgFieldOffset 0
      dict set structmsgData listMsgFields [list]
      # temporary replace data entry of dataView by our param data
      # to be able to use the get* functions for gettting totalLgth entry value
      set result [::structmsg dataView getData saveData saveLgth]
      if {$result != $::STRUCT_MSG_ERR_OK} {
        return $result;
      }
      set result [::structmsg dataView setData $data 10]
      if {$result != $::STRUCT_MSG_ERR_OK} {
        return $result;
      }
      # get totalLgth value from data
      set result [::structmsg dataView getUint16 4 lgth]
      if {$result != $::STRUCT_MSG_ERR_OK} {
        return $result;
      }

      #FIXME!! check crc!!

      # now make a copy of the data to be on the safe side
      # for freeing the Lua space in Lua set the variable to nil!!
      set result [::structmsg dataView setData $data $lgth]
      dict set structmsgData listMsgTotalLgth $lgth

puts stderr "after setData"
      set result [addListMsgFields STRUCT_LIST_FROM_DATA]
      if {$result != $::STRUCT_MSG_ERR_OK} {
        return $result;
      }

      # and now set the IS_SET flags and other stuff
      dict lappend structmsgData flags STRUCT_LIST_IS_INITTED
      set idx 0
      while {$idx < [dict get $structmsgData numListMsgFields]} {
        set listMsgFields [dict get $structmsgData listMsgFields]
        set fieldInfo [lindex $listMsgFields $idx]
        dict lappend fieldInfo fieldFlags STRUCT_MSG_FIELD_IS_SET
        set listMsgFields [lreplace $listMsgFields $idx $idx $fieldInfo]
        dict set structmsgData listMsgFields $listMsgFields
        incr idx
      }
      dict lappend $structmsgData flags STRUCT_LIST_IS_PREPARED
puts stderr "setListMsg done"
      return $::STRUCT_MSG_ERR_OK
    }

    # ============================= getListMsg ========================

    proc getListMsg {dataVar lgthVar} {
      variable structmsgData
      upvar $dataVar data
      upvar $lgthVar lgth

      if {(self->flags & STRUCT_DEF_IS_INITTED} == 0) {
        return STRUCT_DEF_ERR_NOT_YET_INITTED;
      }
      if {(self->flags & STRUCT_DEF_IS_PREPARED} == 0) {
        return STRUCT_DEF_ERR_NOT_YET_PREPARED;
      }
      *data = self->structmsgDefinitionDataView->dataView->data;
      *lgth = self->defTotalLgth;
      return $::STRUCT_MSG_ERR_OK
    }

    # ============================= newStructmsgListMsg ========================

    proc newStructmsgListMsg {} {
      variable structmsgData

      dict set structmsgData structmsgListMsgDataView [list]
      dict set structmsgData listMsgFields [list]
      return $::STRUCT_MSG_ERR_OK
    }

  } ; # namespace listMsg
} ; # namespace structmsg
