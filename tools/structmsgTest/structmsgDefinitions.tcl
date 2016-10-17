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

set ::structmsg(numStructmsgDefinitions) 0
set ::structmsg(structmsgDefinitions) [dict create]
set ::structmsg(fieldTypeDefines) [list]
set ::structmsg(fieldNameDefinitions) [dict create]
dict set ::structmsg(fieldNameDefinitions) numDefinitions 0
dict set ::structmsg(fieldNameDefinitions) definitions [list]
set ::structmsg(numFieldNameIds) 0

namespace eval structmsg {
  namespace ensemble create

  namespace export def

  namespace eval def {
    namespace ensemble create
      
    variable structmsgData

    set structmsgData [dict create]

    namespace export newStructmsgDefinition setDef addDefField dumpDefFields createMsgFromDef

    # ================================= dumpDefDefinitions ====================================

    proc dumpDefDefinitions {fieldInfo indent2 fieldIdx names} {
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

    # ================================= dumpDefFieldValue ====================================

    proc dumpDefFieldValue {fieldInfo indent2 fieldIdx} {
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

    # ============================= dumpDefFields ========================

    proc  dumpDefFields {} {
      variable structmsgData

      set numEntries [dict get $structmsgData numDefFields]
#      puts stderr [format "  defHandle: %s" [dict get $structmsgData handle]]
      puts stderr [format "    numDefFields: %d" $numEntries]
      set idx 0
      set defFields [dict get $structmsgData defFields]
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
          if {[dict get $fieldInfo fieldNameId] eq "STRUCT_MSG_SPEC_FIELD_DEFINITIONS"} {
            set result [dumpDefDefinitions $fieldInfo "  " 0 $names]
            if {$result != $::STRUCT_MSG_ERR_OK} {
              return $result
            }
          } else {
            set result [dumpDefFieldValue $fieldInfo "  " 0]
            if {$result != $::STRUCT_MSG_ERR_OK} {
              return $result
            }
          }
          if {[dict get $fieldInfo fieldNameId] eq "STRUCT_MSG_SPEC_FIELD_NORM_FLD_NAMES"} {
            set result [::structmsg structmsgDataView getFieldValue $fieldInfo names 0]
          }
        }
        incr idx
      }
      return $::STRUCT_MSG_ERR_OK
    }

    # ============================= addDefField ========================

    proc addDefField {fieldNameId fieldTypeId fieldLgth} {
      variable structmsgData

      if {[dict get $structmsgData numDefFields] >= $::STRUCT_DEF_NUM_DEF_FIELDS} {
        return $::STRUCT_MSG_ERR_TOO_MANY_FIELDS
      }
      set defFieldInfo [dict create]
      dict set defFieldInfo fieldNameId $fieldNameId;
      dict set defFieldInfo fieldTypeId $fieldTypeId;
      dict set defFieldInfo fieldLgth $fieldLgth;
      dict set defFieldInfo fieldOffset [dict get $structmsgData defFieldOffset]
      dict lappend structmsgData defFields $defFieldInfo
      dict incr structmsgData defFieldOffset $fieldLgth
      dict incr structmsgData numDefFields 1
      return $::STRUCT_MSG_ERR_OK
    }

    # ============================= getDefFieldValue ========================

    proc getDefFieldValue {fieldNameId valueVar fieldIdx} {
      variable structmsgData
      upvar $valueVar value

      if {(self->flags & STRUCT_DEF_IS_INITTED} == 0) {
        return STRUCT_DEF_ERR_NOT_YET_INITTED;
      }
      return STRUCT_MSG_ERR_OK;
    }

    # ============================= setDefFieldValue ========================

    proc  setDefFieldValue {fieldNameId value fieldIdx} {
      variable structmsgData

      found = false;
      if {(self->flags & STRUCT_DEF_IS_INITTED} == 0) {
        return STRUCT_DEF_ERR_NOT_YET_INITTED;
      }
      idx = 0;
      numEntries = self->numDefFields;
      while {idx < numEntries} {
        fieldInfo = &self->defFields[idx];
        if {fieldNameId == fieldInfo->fieldNameId} {
          result = self->structmsgDefinitionDataView->setFieldValue{self->structmsgDefinitionDataView, fieldInfo, numericValue, stringValue, fieldIdx};
          checkErrOK{result};
          fieldInfo->fieldFlags |= STRUCT_MSG_FIELD_IS_SET;
          found = true;
          break;
        }
        idx++;
      }
      if {!found} {
ets_printf{"fieldNameId: %d found: %d\n", fieldNameId, found};  
        return STRUCT_MSG_ERR_FIELD_NOT_FOUND;
      }
      return STRUCT_MSG_ERR_OK;
    }

    # ============================= addDefFields ========================

    proc addDefFields {numNormFields normNamesSize definitionsSize direction} {
      variable structmsgData

puts stderr "addDefFields!"
      set result [addDefField STRUCT_MSG_SPEC_FIELD_DST DATA_VIEW_FIELD_UINT16_T 2]
      if {$result != $::STRUCT_MSG_ERR_OK} {
        return $result
      }
      set result [addDefField STRUCT_MSG_SPEC_FIELD_SRC DATA_VIEW_FIELD_UINT16_T 2]
      if {$result != $::STRUCT_MSG_ERR_OK} {
        return $result
      }
      set result [addDefField STRUCT_MSG_SPEC_FIELD_TOTAL_LGTH DATA_VIEW_FIELD_UINT16_T 2]
      if {$result != $::STRUCT_MSG_ERR_OK} {
        return $result
      }
      set headerLgth [dict get $structmsgData defFieldOffset]
      set result [addDefField STRUCT_MSG_SPEC_FIELD_CMD_KEY DATA_VIEW_FIELD_UINT16_T 2]
      if {$result != $::STRUCT_MSG_ERR_OK} {
        return $result
      }
      set result [addDefField STRUCT_MSG_SPEC_FIELD_CMD_LGTH DATA_VIEW_FIELD_UINT16_T 2]
      if {$result != $::STRUCT_MSG_ERR_OK} {
        return $result
      }
      set result [addDefField STRUCT_MSG_SPEC_FIELD_RANDOM_NUM DATA_VIEW_FIELD_UINT32_T 4]
      if {$result != $::STRUCT_MSG_ERR_OK} {
        return $result
      }
      if {$direction eq "STRUCT_DEF_FROM_DATA"} {
        set result [::structmsg dataView getUint8 [dict get $structmsgData defFieldOffset] numNormEntries]
        if {$result != $::STRUCT_MSG_ERR_OK} {
          return $result
        }
::structmsg structmsgData dumpBinary $::structmsg::dataView::data 20 "NORMENTRIES"
puts stderr "numNormEntries!$numNormEntries!offset: [dict get $structmsgData defFieldOffset]!"
        set numNormFields $numNormEntries
        dict set structmsgData defNumNormFields $numNormFields
      }
      set result [addDefField STRUCT_MSG_SPEC_FIELD_NUM_NORM_FLDS DATA_VIEW_FIELD_UINT8_T 1]
      if {$result != $::STRUCT_MSG_ERR_OK} {
        return $result
      }
      set result [addDefField STRUCT_MSG_SPEC_FIELD_NORM_FLD_IDS DATA_VIEW_FIELD_UINT16_VECTOR [expr {$numNormFields*2}]]
      if {$result != $::STRUCT_MSG_ERR_OK} {
        return $result
      }
      if {$direction eq "STRUCT_DEF_FROM_DATA"} {
        set result [::structmsg dataView getUint16 [dict get $structmsgData defFieldOffset] normNamesSize]
        if {$result != $::STRUCT_MSG_ERR_OK} {
          return $result
        }
        dict set structmsgData defNormNamesSize $normNamesSize
      }
      set result [addDefField STRUCT_MSG_SPEC_FIELD_NORM_FLD_NAMES_SIZE DATA_VIEW_FIELD_UINT16_T 2]
      if {$result != $::STRUCT_MSG_ERR_OK} {
        return $result
      }
      set result [addDefField STRUCT_MSG_SPEC_FIELD_NORM_FLD_NAMES DATA_VIEW_FIELD_UINT8_VECTOR $normNamesSize]
      if {$result != $::STRUCT_MSG_ERR_OK} {
        return $result
      }
      if {$direction eq "STRUCT_DEF_FROM_DATA"} {
        set result [::structmsg dataView getUint16 [dict get $structmsgData defFieldOffset] definitionsSize]
        if {$result != $::STRUCT_MSG_ERR_OK} {
          return $result
        }
        dict set structmsgData defDefinitionsSize $definitionsSize
      }
      set result [addDefField STRUCT_MSG_SPEC_FIELD_DEFINITIONS_SIZE DATA_VIEW_FIELD_UINT16_T 2]
      if {$result != $::STRUCT_MSG_ERR_OK} {
        return $result
      }
      set result [addDefField STRUCT_MSG_SPEC_FIELD_DEFINITIONS DATA_VIEW_FIELD_UINT16_VECTOR $definitionsSize]
      if {$result != $::STRUCT_MSG_ERR_OK} {
        return $result
      }
      set fillerLgth 0
      set crcLgth 2
      set myLgth [expr {[dict get $structmsgData defFieldOffset] + $crcLgth - $headerLgth}]
      while {($myLgth % 16) != 0} {
        incr myLgth
        incr fillerLgth
      }
      set result [addDefField STRUCT_MSG_SPEC_FIELD_FILLER DATA_VIEW_FIELD_UINT8_VECTOR $fillerLgth]
      if {$result != $::STRUCT_MSG_ERR_OK} {
        return $result
      }
      set result [addDefField STRUCT_MSG_SPEC_FIELD_CRC DATA_VIEW_FIELD_UINT16_T 2]
      if {$result != $::STRUCT_MSG_ERR_OK} {
        return $result
      }
      dict lappend structmsgData flags STRUCT_DEF_IS_INITTED
      if {$direction eq "STRUCT_DEF_TO_DATA"} {
        set data [string repeat " " [dict get $structmsgData defFieldOffset]]
        set result [::structmsg dataView setData $data [dict get $structmsgData defFieldOffset]]
      }
      dict set structmsgData defTotalLgth [dict get $structmsgData defFieldOffset]
puts stderr "addDefFields done"
      return $::STRUCT_MSG_ERR_OK
    }

    # ============================= setStaticDefFields ========================

    proc setStaticDefFields {numNormFields normNamesSize normNamesOffsets definitionsSize} {
      variable structmsgData

    # FIXME src and dst are dummy values for now!!
      result = setDefFieldValue{self, STRUCT_MSG_SPEC_FIELD_DST, 16640, NULL, 0};
      checkErrOK{result};
      result = setDefFieldValue{self, STRUCT_MSG_SPEC_FIELD_SRC, 22272, NULL, 0};
      checkErrOK{result};
      result = setDefFieldValue{self, STRUCT_MSG_SPEC_FIELD_TOTAL_LGTH, self->defFieldOffset, NULL, 0};
      checkErrOK{result};
      result = setDefFieldValue{self, STRUCT_MSG_SPEC_FIELD_CMD_KEY, STRUCT_DEF_CMD_KEY, NULL, 0};
      checkErrOK{result};
      result = setDefFieldValue{self, STRUCT_MSG_SPEC_FIELD_CMD_LGTH, self->defFieldOffset-headerLgth, NULL, 0};
      checkErrOK{result};
      result = setDefFieldValue{self, STRUCT_MSG_SPEC_FIELD_NUM_NORM_FLDS, numNormFields, NULL, 0};
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
          result = setDefFieldValue{self, STRUCT_MSG_SPEC_FIELD_DEFINITIONS, fieldInfo->fieldTypeId, NULL, fieldIdx++};
          checkErrOK{result};
          result = setDefFieldValue{self, STRUCT_MSG_SPEC_FIELD_DEFINITIONS, fieldInfo->fieldLgth, NULL, fieldIdx++};
          checkErrOK{result};
ets_printf{"norm offset: %d fieldTypeId: %d fieldLgth: %d\n", normNamesOffsets[namesIdx-1].offset, fieldInfo->fieldTypeId, fieldInfo->fieldLgth};
        } else {
          result = setDefFieldValue{self, STRUCT_MSG_SPEC_FIELD_DEFINITIONS, fieldInfo->fieldNameId, NULL, fieldIdx++};
          checkErrOK{result};
          result = setDefFieldValue{self, STRUCT_MSG_SPEC_FIELD_DEFINITIONS, fieldInfo->fieldTypeId, NULL, fieldIdx++};
          checkErrOK{result};
          fieldLgth = fieldInfo->fieldLgth;
          if {fieldInfo->fieldNameId == STRUCT_MSG_SPEC_FIELD_TABLE_ROWS} {
            fieldLgth = self->numTableRows;
          }
          if {fieldInfo->fieldNameId == STRUCT_MSG_SPEC_FIELD_TABLE_ROW_FIELDS} {
            fieldLgth = self->numTableRowFields;
          }
          result = setDefFieldValue{self, STRUCT_MSG_SPEC_FIELD_DEFINITIONS, fieldLgth, NULL, fieldIdx++};
          checkErrOK{result};
          if {fieldInfo->fieldNameId == STRUCT_MSG_SPEC_FIELD_TABLE_ROW_FIELDS} {
            tabIdx = 0;
            while {tabIdx < self->numTableRowFields} {
              tabFieldInfo = &self->tableFields[tabIdx];
              result = setDefFieldValue{self, STRUCT_MSG_SPEC_FIELD_DEFINITIONS, normNamesOffsets[namesIdx++].offset, NULL, fieldIdx++};
              checkErrOK{result};
              result = setDefFieldValue{self, STRUCT_MSG_SPEC_FIELD_DEFINITIONS, tabFieldInfo->fieldTypeId, NULL, fieldIdx++};
              checkErrOK{result};
              result = setDefFieldValue{self, STRUCT_MSG_SPEC_FIELD_DEFINITIONS, tabFieldInfo->fieldLgth, NULL, fieldIdx++};
              checkErrOK{result};
              tabIdx++;
            }
          }
        }
        idx++;
      }
      return STRUCT_MSG_ERR_OK;
    }

    # ============================= initDef ========================

    proc initDef {} {
      variable structmsgData

      if {(self->flags & STRUCT_DEF_IS_INITTED} != 0) {
        return STRUCT_DEF_ERR_ALREADY_INITTED;
      }
      numFields = self->numFields + self->numTableRowFields;
      definitionsSize = numFields * {sizeof(uint16_t} + sizeof(uint16_t) * sizeof(uint16_t));
      id2offset_t normNamesOffsets[numFields];
      numNormFields = 0;
      normNamesSize = 0;
      namesOffset = 0;
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

    # ============================= prepareDef ========================

    proc prepareDef {} {
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

    # ============================= setDef ========================

    proc setDef {data} {
      variable structmsgData

puts stderr "setDef1"
      dict set structmsgData numDefFields 0
      dict set structmsgData defFieldOffset 0
      dict set structmsgData defFields [list]
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
      dict set structmsgData defTotalLgth $lgth

puts stderr "after setData"
      set numNormFields 0
      set normNamesSize 0
      set definitionsSize 0
      set result [addDefFields $numNormFields $normNamesSize $definitionsSize STRUCT_DEF_FROM_DATA]
      if {$result != $::STRUCT_MSG_ERR_OK} {
        return $result;
      }

      # and now set the IS_SET flags and other stuff
      dict lappend structmsgData flags STRUCT_DEF_IS_INITTED
      set idx 0
      while {$idx < [dict get $structmsgData numDefFields]} {
        set defFields [dict get $structmsgData defFields]
        set fieldInfo [lindex $defFields $idx]
        dict lappend fieldInfo fieldFlags STRUCT_MSG_FIELD_IS_SET
        set defFields [lreplace $defFields $idx $idx $fieldInfo]
        dict set structmsgData defFields $defFields
        incr idx
      }
      dict lappend $structmsgData flags STRUCT_DEF_IS_PREPARED
puts stderr "setDef done"
      return $::STRUCT_MSG_ERR_OK
    }

    # ============================= getDef ========================

    proc getDef {dataVar lgthVar} {
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

    # ============================= createMsgFromDef ========================

    proc createMsgFromDef {} {
      variable structmsgData

# FIXME TEMEPORARY!!
#set ::structmsg structmsgData structmsgData $structmsgData
       # loop over def Fields and extract infos
      set fieldIdx 0
      while {$fieldIdx < [dict get $structmsgData numDefFields]} {
        set defFields [dict get $structmsgData defFields]
        set fieldInfo [lindex $defFields $fieldIdx]
puts stderr "fieldInfo:$fieldInfo!"
        switch [dict get $fieldInfo fieldNameId] {
        STRUCT_MSG_SPEC_FIELD_SRC -
        STRUCT_MSG_SPEC_FIELD_DST -
        STRUCT_MSG_SPEC_FIELD_TARGET_CMD -
        STRUCT_MSG_SPEC_FIELD_TOTAL_LGTH -
        STRUCT_MSG_SPEC_FIELD_CMD_KEY -
        STRUCT_MSG_SPEC_FIELD_CMD_LGTH -
        STRUCT_MSG_SPEC_FIELD_RANDOM_NUM -
        STRUCT_MSG_SPEC_FIELD_FILLER -
        STRUCT_MSG_SPEC_FIELD_CRC{
              # nothing to do!
        }
        STRUCT_MSG_SPEC_FIELD_NUM_NORM_FLDS {
          set result [::structmsg dataView getUint8 [dict get $fieldInfo fieldOffset] numNormFlds]
          if {$result != $::STRUCT_MSG_ERR_OK} {
            return $result;
          }
        }
        STRUCT_MSG_SPEC_FIELD_NORM_FLD_IDS {
          set idx 0
          while {$idx < $numNormFlds} {
            set result [::structmsg dataView getUint16 [expr {[dict get $fieldInfo fieldOffset]+$idx*2}] val]
            if {$result != $::STRUCT_MSG_ERR_OK} {
              return $result
            }
            dict set normNamesOffsets $idx offset $val
            incr idx
          }
puts stderr "normNamesOffsets!$normNamesOffsets!"
        }
        STRUCT_MSG_SPEC_FIELD_NORM_FLD_NAMES_SIZE {
          set result [::structmsg dataView getUint16 [dict get $fieldInfo fieldOffset] normFldNamesSize]
          if {$result != $::STRUCT_MSG_ERR_OK} {
            return $result
          }
        }
        STRUCT_MSG_SPEC_FIELD_NORM_FLD_NAMES {
          set result [::structmsg dataView getUint8Vector [dict get $fieldInfo fieldOffset] names [dict get $fieldInfo fieldLgth]]
          if {$result != $::STRUCT_MSG_ERR_OK} {
            return $result
          }
          set idx 0
          while {$idx < $numNormFlds} {
            set val [dict get $normNamesOffsets $idx offset]
            set nameIdx $val
            while {true} {
              set ch [string range $names $nameIdx $nameIdx]
              set pch $ch
              if {![string is integer $ch]} {
                binary scan $ch c pch
              }
              if {$pch == 0} {
                break
              }
puts stderr "nameIdx!$nameIdx!"
              incr nameIdx
            }
            set fieldNameStr [string range $names $val $nameIdx]
            dict set normNamesOffsets $idx name [string range $names $val $nameIdx]
puts stderr "normNamesOffsets!$normNamesOffsets!"
            incr idx
          }
        }
        STRUCT_MSG_SPEC_FIELD_DEFINITIONS_SIZE {
          set result [::structmsg dataView getUint16 [dict get $fieldInfo fieldOffset] definitionsSize]
          if {$result != $::STRUCT_MSG_ERR_OK} {
            return $result
          }
        }
        STRUCT_MSG_SPEC_FIELD_DEFINITIONS {
          set idx 0
          set namesIdx 0
puts stderr "STRUCT_MSG_SPEC_FIELD_DEFINITIONS!"
#          ::structmsg structmsgData createMsg [expr {$definitionsSize / 2}] handle
puts stderr "need to fix addHandel first!!"
puts stderr "after createMsg!"
          while {$idx < [expr {$definitionsSize / 2}]} {
            set result [::structmsg dataView getUint16 [expr {[dict get $fieldInfo fieldOffset]+($idx*2)}] fieldNameId]
            incr idx
            if {$result != $::STRUCT_MSG_ERR_OK} {
              return $result
            }
            if {$fieldNameId > $::STRUCT_MSG_SPEC_FIELD_LOW} {
              set result [::structmsg structmsgDataView getFieldNameStrFromId $fieldNameId fieldNameStr]
              if {$result != $::STRUCT_MSG_ERR_OK} {
                return $result
              }
            } else {
              set fieldNameStr [dict get $normNamesOffsets $namesIdx name]
              incr namesIdx
              if {$result != $::STRUCT_MSG_ERR_OK} {
                return $result
              }
            }
            set result [::structmsg dataView getUint16 [expr {[dict get $fieldInfo fieldOffset]+($idx*2)}] fieldTypeId]
            incr idx
            if {$result != $::STRUCT_MSG_ERR_OK} {
              return $result
            }
            set result [::structmsg dataView getFieldTypeStrFromId $fieldTypeId fieldTypeStr]
            if {$result != $::STRUCT_MSG_ERR_OK} {
              return $result
            }
            set result [::structmsg dataView getUint16 [expr {[dict get $fieldInfo fieldOffset]+($idx*2)}] fieldLgth]
            incr idx
            if {$result != $::STRUCT_MSG_ERR_OK} {
              return $result
            }
puts stderr "addField!$fieldNameStr!"
puts stderr "need to fix addHandel first!!"
#            set result [::structmsg structmsgData addField $fieldNameStr $fieldTypeStr $fieldLgth]
#            if {$result != $::STRUCT_MSG_ERR_OK} {
#              return $result
#            }
          }
        }
        }
        incr fieldIdx
      }
      set result [initMsg]
      return $::STRUCT_MSG_ERR_OK
    }

    # ============================= newStructmsgDefinition ========================

    proc newStructmsgDefinition {} {
      variable structmsgData

      dict set structmsgData structmsgDefinitionDataView [list]
      dict set structmsgData defFields [list]
      return $::STRUCT_MSG_ERR_OK
    }

    # ============================= structmsg_deleteDefinition ========================

    proc structmsg_deleteDefinition {name structmsgDefinitions fieldNameDefinitions} {
      variable structmsgData

      result =  structmsg_getDefinitionPtr{name, &definition, &definitionsIdx};
      checkErrOK{result};
      idx = 0;
      while {idx < definition->numFields} {
        fieldInfo = &definition->fieldInfos[idx];
        nameIdx = 0;
        nameFound = 0;
        if {fieldInfo->fieldId < STRUCT_MSG_SPEC_FIELD_LOW} {
          while {nameIdx < fieldNameDefinitions->numDefinitions} {
            nameEntry = &fieldNameDefinitions->definitions[nameIdx];
            if {fieldInfo->fieldId == nameEntry->id} {
              result = structmsg_getFieldNameId{nameEntry->str, &fieldId, STRUCT_MSG_DECR};
              checkErrOK{result};
              nameFound = 1;
              break;
            }
            nameIdx++;
          }
          if {!nameFound} {
            return STRUCT_MSG_ERR_FIELD_NOT_FOUND;
          }
        }
        idx++;
      }
          # nameDefinitions deleted

      definition->numFields = 0;
      definition->maxFields = 0;
      os_free{definition->name};
      definition->name = NULL;
      if {definition->encoded != NULL} {
        os_free{definition->encoded};
        definition->encoded = NULL;
      }
      os_free{definition->fieldInfos};
      definition->fieldInfos = NULL;
      if {definition->encoded != NULL} {
        os_free{definition->encoded};
        definition->encoded = NULL;
      }
      if {definition->encrypted != NULL} {
        os_free{definition->encrypted};
        definition->encrypted = NULL;
      }
      if {definition->todecode != NULL} {
        os_free{definition->todecode};
        definition->todecode = NULL;
      }
      definition->totalLgth = 0;
          # definition deleted

      return STRUCT_MSG_ERR_OK;
    }

    # ============================= structmsg_deleteDefinitions ========================

    proc structmsg_deleteDefinitions {fieldNameDefinitions} {
      variable structmsgData
          # delete the whole structmsgDefinitions info, including fieldNameDefinitions info
      stmsgDefinition_t *definition;
      fieldInfoDefinition_t *fieldInfo;
      name2id_t *nameEntry;
      uint8_t *name;
      int idx;
      int nameIdx;
      int found;
      int nameFound;
      int result;
      int fieldId;

      idx = 0;
      while {idx < structmsgDefinitions->numDefinitions} {
        definition = &structmsgDefinitions->definitions[idx];
        if {definition->name != NULL} {
          structmsg_deleteDefinition{definition->name, structmsgDefinitions, fieldNameDefinitions};
        }
        idx++;
      }
      structmsgDefinitions->numDefinitions = 0;
      structmsgDefinitions->maxDefinitions = 0;
      os_free{structmsgDefinitions->definitions};
      structmsgDefinitions->definitions = NULL;

      fieldNameDefinitions->numDefinitions = 0;
      fieldNameDefinitions->maxDefinitions = 0;
      os_free{fieldNameDefinitions->definitions};
      fieldNameDefinitions->definitions = NULL;

          # all deleted/reset
      return STRUCT_MSG_ERR_OK;
    }

    # ============================= structmsg_deleteStructmsgDefinition ========================

    proc structmsg_deleteStructmsgDefinition {name} {
      variable structmsgData
      return structmsg_deleteDefinition{name, &structmsgDefinitions, &fieldNameDefinitions};
    }

    # ============================= structmsg_deleteStructmsgDefinitions ========================

    proc structmsg_deleteStructmsgDefinitions {} {
      variable structmsgData
          # delete the whole structmsgDefinitions info, including fieldNameDefinitions info
      return structmsg_deleteDefinitions{&structmsgDefinitions, &fieldNameDefinitions};
    }

    # ============================= structmsg_getDefinitionNormalFieldNames ========================

    proc structmsg_getDefinitionNormalFieldNames {name normalFieldNames} {
      variable structmsgData

      result = structmsg_getDefinitionPtr{name, &definition, &definitionsIdx};
      checkErrOK{result};
      return STRUCT_MSG_ERR_OK;
    }

    # ============================= structmsg_getDefinitionTableFieldNames ========================

    proc structmsg_getDefinitionTableFieldNames {name tableFieldNames} {
      variable structmsgData

      result = structmsg_getDefinitionPtr{name, &definition, &definitionsIdx};
      checkErrOK{result};
      return STRUCT_MSG_ERR_OK;
    }

    # ============================= structmsg_getDefinitionNumTableRows ========================

    proc structmsg_getDefinitionNumTableRows {name numTableRows} {
      variable structmsgData

      result = structmsg_getDefinitionPtr{name, &definition, &definitionsIdx};
      checkErrOK{result};
      idx = 0;
      while{idx < definition->numFields} {
        fieldInfo = &definition->fieldInfos[idx];
        result  = structmsg_getIdFieldNameStr {fieldInfo->fieldId, &fieldName};
        checkErrOK{result};
        if {c_strcmp(fieldName, "@tablerows"} == 0) {
          *numTableRows = fieldInfo->fieldLgth;
          return STRUCT_MSG_ERR_OK;
        }
      }
      return STRUCT_MSG_ERR_FIELD_NOT_FOUND;
    }

    # ============================= structmsg_getDefinitionNumTableRowFields ========================

    proc structmsg_getDefinitionNumTableRowFields {name numTableRowFields} {
      variable structmsgData

      result = structmsg_getDefinitionPtr{name, &definition, &definitionsIdx};
      checkErrOK{result};
      idx = 0;
      while{idx < definition->numFields} {
        fieldInfo = &definition->fieldInfos[idx];
        result  = structmsg_getIdFieldNameStr {fieldInfo->fieldId, &fieldName};
        checkErrOK{result};
        if {c_strcmp(fieldName, "@tablerowfields"} == 0) {
          *numTableRowFields = fieldInfo->fieldLgth;
          return STRUCT_MSG_ERR_OK;
        }
      }
      return STRUCT_MSG_ERR_FIELD_NOT_FOUND;
    }

    # ============================= structmsg_getDefinitionFieldInfo ========================

    proc structmsg_getDefinitionFieldInfo {name fieldName fieldInfo} {
      variable structmsgData

      result = structmsg_getDefinitionPtr{name, &definition, &definitionsIdx};
      checkErrOK{result};
      idx = 0;
      while{idx < definition->numFields} {
        *fieldInfo = &definition->fieldInfos[idx];
        result  = structmsg_getIdFieldNameStr {(*fieldInfo}->fieldId, &lookupFieldName);
        checkErrOK{result};
        if {c_strcmp(lookupFieldName, fieldName} == 0) {
          return STRUCT_MSG_ERR_OK;
        }
      }
      return STRUCT_MSG_ERR_FIELD_NOT_FOUND;
}

    # ============================= structmsg_getDefinitionTableFieldInfo ========================

    proc structmsg_getDefinitionTableFieldInfo {name fieldName fieldInfo} {
      stmsgDefinition_t *definition;
      uint8_t definitionsIdx;
      int idx;
      int result;
      uint8_t *lookupFieldName;

      result = structmsg_getDefinitionPtr{name, &definition, &definitionsIdx};
      checkErrOK{result};
      idx = 0;
      while{idx < definition->numFields} {
        *fieldInfo = &definition->fieldInfos[idx];
        result  = structmsg_getIdFieldNameStr {(*fieldInfo}->fieldId, &lookupFieldName);
        checkErrOK{result};
        if {c_strcmp(lookupFieldName, fieldName} == 0) {
          return STRUCT_MSG_ERR_OK;
        }
      }
      return STRUCT_MSG_ERR_FIELD_NOT_FOUND;
    }

    # ============================= structmsg_createMsgDefinitionFromListInfo ========================

    proc structmsg_createMsgDefinitionFromListInfo {name listVector numEntries numRows flags shortCmdKey} {
      variable structmsgData

ets_printf{"structmsg_createMsgDefinitionFromListInfo: shortCmdKey: %d\n", shortCmdKey};
      result = structmsg_createStructmsgDefinition{name, numEntries, shortCmdKey};
      checkErrOK{result};
      listEntry = listVector[0];
      idx = 0;
      while{idx < numEntries} {
        listEntry = listVector[idx];
        uint8_t buffer[c_strlen{listEntry} + 1];
        fieldName = buffer;
        c_memcpy{fieldName, listEntry, c_strlen(listEntry});
        fieldName[c_strlen{listEntry}] = '\0';
        cp = fieldName;
        while {*cp != ','} {
          cp++;
        }
        *cp++ = '\0';
        fieldType = cp;
        while {*cp != ','} {
          cp++;
        }
        *cp++ = '\0';
        fieldLgthStr = cp;
        while {*cp != ','} {
          cp++;
        }
        *cp++ = '\0';
        flagStr = cp;
        if {c_strcmp(fieldLgthStr,"@numRows"} == 0) {
          fieldLgth = numRows;
        } else {
          lgth = c_strtoul{fieldLgthStr, &endPtr, 10};
          fieldLgth = {uint8_t}lgth;
        }
        uflag = c_strtoul{flagStr, &endPtr, 10};
        flag = {uint8_t}uflag;
        if {flag == 0} {
          result = structmsg_addFieldDefinition{name, fieldName, fieldType, fieldLgth};
          checkErrOK{result};
        } else {
          if {(flags != 0} && (flag == 2)) {
            result = structmsg_addFieldDefinition{name, fieldName, fieldType, fieldLgth};
            checkErrOK{result};
          } else {
            if {(flags == 0} && (flag == 1)) {
              result = structmsg_addFieldDefinition{name, fieldName, fieldType, fieldLgth};
              checkErrOK{result};
            }
          }
        }
        idx++;
      }
      return STRUCT_MSG_ERR_OK;
    }

  } ; # namespace def
} ; # namespace structmsg
