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
      
    variable strutcmsgData

    namespace export initFieldTypeDefines initSpecialFieldNames getFieldNameId
    namespace export getFieldTypeId createDefinition addFieldDefinition deleteFieldDefinition

    # ================================= dumpDefDefinitions ====================================

    proc dumpDefDefinitions {fieldInfo indent2 fieldIdx names} {
      variable strutcmsgData

      valueIdx = 0;
      while {valueIdx < fieldInfo->fieldLgth / sizeof(uint16_t}) {
        result = self->structmsgDefinitionDataView->getFieldValue{self->structmsgDefinitionDataView, fieldInfo, &fieldNameId, &stringValue, valueIdx++};
        result = self->structmsgDefinitionDataView->getFieldValue{self->structmsgDefinitionDataView, fieldInfo, &fieldTypeId, &stringValue, valueIdx++};
        result = self->structmsgDefinitionDataView->getFieldValue{self->structmsgDefinitionDataView, fieldInfo, &fieldLgth, &stringValue, valueIdx++};
        result = self->structmsgDataView->dataView->getFieldTypeStrFromId{self->structmsgDataView->dataView, fieldTypeId, &fieldTypeStr};
        checkErrOK{result};
        if {fieldNameId > STRUCT_MSG_SPEC_FIELD_LOW} {
          result = self->structmsgDataView->getFieldNameStrFromId{self->structmsgDataView, fieldNameId, &fieldNameStr};
          checkErrOK{result};
        } else {
          fieldNameStr = names+fieldNameId;
        }
        ets_printf{"        defIdx: %3d fieldName: %3d %-20s fieldType: %3d %-8s fieldLgth: %5d\r\n", valueIdx/3, fieldNameId, fieldNameStr, fieldTypeId, fieldTypeStr, fieldLgth};
      }
      return DATA_VIEW_ERR_OK;
    }

    # ================================= dumpDefFieldValue ====================================

    proc dumpDefFieldValue {fieldInfo indent2 fieldIdx} {
      variable strutcmsgData

      result = self->structmsgDefinitionDataView->getFieldValue{self->structmsgDefinitionDataView, fieldInfo, &numericValue, &stringValue, fieldIdx};
      checkErrOK{result};
      switch {fieldInfo->fieldTypeId} {
      case DATA_VIEW_FIELD_INT8_T:
        ets_printf{"      %svalue: 0x%02x %d\n", indent2, numericValue & 0xFF, numericValue};
        break;
      case DATA_VIEW_FIELD_UINT8_T:
        ets_printf{"      %svalue: 0x%02x %d\n", indent2, numericValue & 0xFF, numericValue & 0xFF};
        break;
      case DATA_VIEW_FIELD_INT16_T:
        ets_printf{"      %svalue: 0x%04x %d\n", indent2, numericValue & 0xFFFF, numericValue};
        break;
      case DATA_VIEW_FIELD_UINT16_T:
        ets_printf{"      %svalue: 0x%04x %d\n", indent2, numericValue & 0xFFFF, numericValue & 0xFFFF};
        break;
      case DATA_VIEW_FIELD_INT32_T:
        ets_printf{"      %svalue: 0x%08x %d\n", indent2, numericValue & 0xFFFFFFFF, numericValue};
        break;
      case DATA_VIEW_FIELD_UINT32_T:
        ets_printf{"      %svalue: 0x%08x %d\n", indent2, numericValue & 0xFFFFFFFF, numericValue & 0xFFFFFFFF};
        break;
      case DATA_VIEW_FIELD_INT8_VECTOR:
        valueIdx = 0;
        ets_printf{"      %svalues:", indent2};
        while {valueIdx < fieldInfo->fieldLgth} {
          ch = stringValue[valueIdx];
          ets_printf{"        %sidx: %d value: %c 0x%02x\n", indent2, valueIdx, (char}ch, (uint8_t)(ch & 0xFF));
          valueIdx++;
        }
        ets_printf{"\n"};
        break;
      case DATA_VIEW_FIELD_UINT8_VECTOR:
        valueIdx = 0;
        ets_printf{"      %svalues:\n", indent2};
        while {valueIdx < fieldInfo->fieldLgth} {
          uch = stringValue[valueIdx];
          ets_printf{"        %sidx: %d value: %c 0x%02x\n", indent2, valueIdx, (char}uch, (uint8_t)(uch & 0xFF));
          valueIdx++;
        }
        break;
      case DATA_VIEW_FIELD_INT16_VECTOR:
        valueIdx = 0;
        ets_printf{"      %svalues:", indent2};
        while {valueIdx < fieldInfo->fieldLgth/sizeof(int16_t}) {
          result = self->structmsgDefinitionDataView->dataView->getInt16{self->structmsgDefinitionDataView->dataView, fieldInfo->fieldOffset+valueIdx*sizeof(int16_t}, &sh);
          checkErrOK{result};
          ets_printf{"        %sidx: %d value: 0x%04x\n", indent2, valueIdx, sh};
          valueIdx++;
        }
        ets_printf{"\n"};
        break;
      case DATA_VIEW_FIELD_UINT16_VECTOR:
        valueIdx = 0;
        ets_printf{"      %svalues:\n", indent2};
        while {valueIdx < fieldInfo->fieldLgth/sizeof(uint16_t}) {
          result = self->structmsgDefinitionDataView->getFieldValue{self->structmsgDefinitionDataView, fieldInfo, &numericValue, &stringValue, valueIdx};
          checkErrOK{result};
          ets_printf{"        %sidx: %d value: 0x%04x\n", indent2, valueIdx, (uint16_t}(numericValue & 0xFFFF));
          valueIdx++;
        }
        break;
#ifdef NOTDEF
      case DATA_VIEW_FIELD_INT32_VECTOR:
        valueIdx = 0;
        ets_printf{"      %svalues:", indent2};
        while {valueIdx < fieldInfo->fieldLgth} {
          val = fieldInfo->value.int32Vector[valueIdx];
          ets_printf{"        %sidx: %d value: 0x%08x\n", indent2, valueIdx, (int32_t}(val & 0xFFFFFFFF));
          valueIdx++;
        }
        ets_printf{"\n"};
        break;
      case DATA_VIEW_FIELD_UINT32_VECTOR:
        valueIdx = 0;
        ets_printf{"      %svalues:\n", indent2};
        while {valueIdx < fieldInfo->fieldLgth} {
          uval = fieldInfo->value.uint32Vector[valueIdx];
          ets_printf{"        %sidx: %d value: 0x%08x\n", indent2, valueIdx, (uint32_t}(uval & 0xFFFFFFFF));
          valueIdx++;
        }
        break;
#endif
      }
      return DATA_VIEW_ERR_OK;
    }

    # ============================= dumpDefFields ========================

    proc  dumpDefFields {} {
      variable strutcmsgData

      numEntries = self->numDefFields;
      ets_printf{"  defHandle: %s\r\n", self->handle};
      ets_printf{"    numDefFields: %d\r\n", numEntries};
      idx = 0;
      while {idx < numEntries} {
        fieldInfo = &self->defFields[idx];
        result = self->structmsgDataView->dataView->getFieldTypeStrFromId{self->structmsgDataView->dataView, fieldInfo->fieldTypeId, &fieldTypeStr};
        checkErrOK{result};
        result = self->structmsgDataView->getFieldNameStrFromId{self->structmsgDataView, fieldInfo->fieldNameId, &fieldNameStr};
        checkErrOK{result};
        ets_printf{"      idx: %d fieldName: %-20s fieldType: %-8s fieldLgth: %.5d offset: %d \r\n", idx, fieldNameStr, fieldTypeStr, fieldInfo->fieldLgth, fieldInfo->fieldOffset};
        if {fieldInfo->fieldFlags & STRUCT_MSG_FIELD_IS_SET} {
          if {fieldInfo->fieldNameId == STRUCT_MSG_SPEC_FIELD_DEFINITIONS} {
            result = dumpDefDefinitions{self, fieldInfo, "  ", 0, stringValue};
            checkErrOK{result};
          } else {
            result = dumpDefFieldValue{self, fieldInfo, "  ", 0};
            checkErrOK{result};
          }
          if {fieldInfo->fieldNameId == STRUCT_MSG_SPEC_FIELD_NORM_FLD_NAMES} {
            result = self->structmsgDefinitionDataView->getFieldValue{self->structmsgDefinitionDataView, fieldInfo, &numericValue, &stringValue, 0};
          }
        }
        idx++;
      }
      return STRUCT_MSG_ERR_OK;
    }

    # ============================= addDefField ========================

    proc addDefField {fieldNameId fieldTypeId fieldLgth} {
      variable strutcmsgData

      if {self->numDefFields >= STRUCT_DEF_NUM_DEF_FIELDS} {
        return STRUCT_MSG_ERR_TOO_MANY_FIELDS;
      }
      defFieldInfo = &self->defFields[self->numDefFields];
      defFieldInfo->fieldNameId = fieldNameId;
      defFieldInfo->fieldTypeId = fieldTypeId;
      defFieldInfo->fieldLgth = fieldLgth;
      defFieldInfo->fieldOffset = self->defFieldOffset;
      self->defFieldOffset += fieldLgth;
      self->numDefFields++;
      return STRUCT_MSG_ERR_OK;
    }

    # ============================= getDefFieldValue ========================

    proc getDefFieldValue {fieldNameId valueVar fieldIdx} {
      variable strutcmsgData
      upvar $valueVar value

      if {(self->flags & STRUCT_DEF_IS_INITTED} == 0) {
        return STRUCT_DEF_ERR_NOT_YET_INITTED;
      }
      return STRUCT_MSG_ERR_OK;
    }

    # ============================= setDefFieldValue ========================

    proc  setDefFieldValue {fieldNameId value fieldIdx} {
      variable strutcmsgData

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
      variable strutcmsgData

      result = addDefField{self, STRUCT_MSG_SPEC_FIELD_DST, DATA_VIEW_FIELD_UINT16_T, 2};
      checkErrOK{result};
      result = addDefField{self, STRUCT_MSG_SPEC_FIELD_SRC, DATA_VIEW_FIELD_UINT16_T, 2};
      checkErrOK{result};
      result = addDefField{self, STRUCT_MSG_SPEC_FIELD_TOTAL_LGTH, DATA_VIEW_FIELD_UINT16_T, 2};
      checkErrOK{result};
      headerLgth = self->defFieldOffset;
      result = addDefField{self, STRUCT_MSG_SPEC_FIELD_CMD_KEY, DATA_VIEW_FIELD_UINT16_T, 2};
      checkErrOK{result};
      result = addDefField{self, STRUCT_MSG_SPEC_FIELD_CMD_LGTH, DATA_VIEW_FIELD_UINT16_T, 2};
      checkErrOK{result};
      result = addDefField{self, STRUCT_MSG_SPEC_FIELD_RANDOM_NUM, DATA_VIEW_FIELD_UINT32_T, 4};
      checkErrOK{result};
      if {direction == STRUCT_DEF_FROM_DATA} {
        result = self->structmsgDefinitionDataView->dataView->getUint8{self->structmsgDefinitionDataView->dataView, self->defFieldOffset, &numNormEntries};
        checkErrOK{result};
        numNormFields = numNormEntries;
        self->defNumNormFields = numNormFields;
      }
      result = addDefField{self, STRUCT_MSG_SPEC_FIELD_NUM_NORM_FLDS, DATA_VIEW_FIELD_UINT8_T, 1};
      checkErrOK{result};
      result = addDefField{self, STRUCT_MSG_SPEC_FIELD_NORM_FLD_IDS, DATA_VIEW_FIELD_UINT16_VECTOR, numNormFields*sizeof(uint16});
      checkErrOK{result};
      if {direction == STRUCT_DEF_FROM_DATA} {
        result = self->structmsgDefinitionDataView->dataView->getUint16{self->structmsgDefinitionDataView->dataView, self->defFieldOffset, &normNamesSize};
        checkErrOK{result};
        self->defNormNamesSize = normNamesSize;
      }
      result = addDefField{self, STRUCT_MSG_SPEC_FIELD_NORM_FLD_NAMES_SIZE, DATA_VIEW_FIELD_UINT16_T, 2};
      checkErrOK{result};
      result = addDefField{self, STRUCT_MSG_SPEC_FIELD_NORM_FLD_NAMES, DATA_VIEW_FIELD_UINT8_VECTOR, normNamesSize};
      checkErrOK{result};
      if {direction == STRUCT_DEF_FROM_DATA} {
        result = self->structmsgDefinitionDataView->dataView->getUint16{self->structmsgDefinitionDataView->dataView, self->defFieldOffset, &definitionsSize};
        checkErrOK{result};
        self->defDefinitionsSize = definitionsSize;
      }
      result = addDefField{self, STRUCT_MSG_SPEC_FIELD_DEFINITIONS_SIZE, DATA_VIEW_FIELD_UINT16_T, 2};
      checkErrOK{result};
      result = addDefField{self, STRUCT_MSG_SPEC_FIELD_DEFINITIONS, DATA_VIEW_FIELD_UINT16_VECTOR, definitionsSize};
      checkErrOK{result};
        fillerLgth = 0;
        crcLgth = 2;
        myLgth = self->defFieldOffset + crcLgth - headerLgth;
        while {(myLgth % 16} != 0) {
          myLgth++;
          fillerLgth++;
        }
      result = addDefField{self, STRUCT_MSG_SPEC_FIELD_FILLER, DATA_VIEW_FIELD_UINT8_VECTOR, fillerLgth};
      checkErrOK{result};
      result = addDefField{self, STRUCT_MSG_SPEC_FIELD_CRC, DATA_VIEW_FIELD_UINT16_T, 2};
      checkErrOK{result};
      self->flags |= STRUCT_DEF_IS_INITTED;
      if {direction == STRUCT_DEF_TO_DATA} {
        self->structmsgDefinitionDataView->dataView->data = os_zalloc{self->defFieldOffset};
        checkAllocOK{self->structmsgDefinitionDataView->dataView->data};
      }
      self->structmsgDefinitionDataView->dataView->lgth = self->defFieldOffset;
      self->defTotalLgth = self->defFieldOffset;
      return STRUCT_MSG_ERR_OK;
    }

    # ============================= setStaticDefFields ========================

    proc setStaticDefFields {numNormFields normNamesSize normNamesOffsets definitionsSize} {
      variable strutcmsgData

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
      result = setDefFieldValue{self, STRUCT_MSG_SPEC_FIELD_NORM_FLD_NAMES, 0, names, 0};
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
      variable strutcmsgData

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
      variable strutcmsgData

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
      variable strutcmsgData

      if {self->structmsgDefinitionDataView == NULL} {
        result = newStructmsgDefinition{self};
        checkErrOK{result};
      } else {
        if {self->structmsgDataView->dataView->data != NULL} {
              # free no longer used we cannot reuse as the size can be different!
          os_free{self->structmsgDataView->dataView->data};
          self->structmsgDataView->dataView->data = NULL;
              # we do net free self->defFields, we just reuse them it is always the same number of defFields
          os_free{self->defFields};
          self->defFields = NULL;
          self->numDefFields = 0;
          self->defFieldOffset = 0;
        }
      }
          # temporary replace data entry of dataView by our param data
          # to be able to use the get* functions for gettting totalLgth entry value
      self->structmsgDefinitionDataView->dataView->data = {uint8_t *}data;
      self->structmsgDefinitionDataView->dataView->lgth = 10;
          # get totalLgth value from data
      result = self->structmsgDefinitionDataView->dataView->getUint16{self->structmsgDefinitionDataView->dataView, 4, &lgth};
      checkErrOK{result};

    #FIXME!! check crc!!

          # now make a copy of the data to be on the safe side
          # for freeing the Lua space in Lua set the variable to nil!!
      self->structmsgDefinitionDataView->dataView->data = os_zalloc{lgth};
      checkAllocOK{self->structmsgDefinitionDataView->dataView->data};
      c_memcpy{self->structmsgDefinitionDataView->dataView->data, data, lgth};
      self->structmsgDefinitionDataView->dataView->lgth = lgth;
      self->defTotalLgth = lgth;

      numNormFields = 0;
      normNamesSize = 0;
      definitionsSize = 0;
      result = addDefFields{self, numNormFields, normNamesSize, definitionsSize, STRUCT_DEF_FROM_DATA};
      checkErrOK{result};

          # and now set the IS_SET flags and other stuff
      self->flags |= STRUCT_DEF_IS_INITTED;
      idx = 0;
      while {idx < self->numDefFields} {
        fieldInfo = &self->defFields[idx];
        fieldInfo->fieldFlags |= STRUCT_MSG_FIELD_IS_SET;
        idx++;
      }
      self->flags |= STRUCT_DEF_IS_PREPARED;
      return STRUCT_MSG_ERR_OK;
    }

    # ============================= getDef ========================

    proc getDef {dataVar lgthVar} {
      variable strutcmsgData
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
      return STRUCT_MSG_ERR_OK;
    }

    # ============================= createMsgFromDef ========================

    proc createMsgFromDef {} {
      variable strutcmsgData

          # lopp over def Fields and extract infos
      id2offset_t normNamesOffsets[self->defNumNormFields];
      fieldIdx = 0;
      while {fieldIdx < self->numDefFields} {
        fieldInfo = &self->defFields[fieldIdx];
        switch {fieldInfo->fieldNameId} {
        case STRUCT_MSG_SPEC_FIELD_SRC:
        case STRUCT_MSG_SPEC_FIELD_DST:
        case STRUCT_MSG_SPEC_FIELD_TARGET_CMD:
        case STRUCT_MSG_SPEC_FIELD_TOTAL_LGTH:
        case STRUCT_MSG_SPEC_FIELD_CMD_KEY:
        case STRUCT_MSG_SPEC_FIELD_CMD_LGTH:
        case STRUCT_MSG_SPEC_FIELD_RANDOM_NUM:
        case STRUCT_MSG_SPEC_FIELD_FILLER:
        case STRUCT_MSG_SPEC_FIELD_CRC:
              # nothing to do!
          break;
        case STRUCT_MSG_SPEC_FIELD_NUM_NORM_FLDS:
          result = self->structmsgDefinitionDataView->dataView->getUint8{self->structmsgDefinitionDataView->dataView, fieldInfo->fieldOffset, &numNormFlds};
          checkErrOK{result};
          break;
        case STRUCT_MSG_SPEC_FIELD_NORM_FLD_IDS:
          idx = 0;
          while {idx < numNormFlds} {
            result = self->structmsgDefinitionDataView->dataView->getUint16{self->structmsgDefinitionDataView->dataView, fieldInfo->fieldOffset+idx*sizeof(uint16_t}, &val);
            checkErrOK{result};
            normNamesOffsets[idx].offset = val;
            normNamesOffsets[idx].name = names+val;
            idx++;
          }
          break;
        case STRUCT_MSG_SPEC_FIELD_NORM_FLD_NAMES_SIZE:
          result = self->structmsgDefinitionDataView->dataView->getUint16{self->structmsgDefinitionDataView->dataView, fieldInfo->fieldOffset, &normFldNamesSize};
          checkErrOK{result};
          break;
        case STRUCT_MSG_SPEC_FIELD_NORM_FLD_NAMES:
          result = self->structmsgDefinitionDataView->dataView->getUint8Vector{self->structmsgDefinitionDataView->dataView, fieldInfo->fieldOffset, &names, fieldInfo->fieldLgth};
          checkErrOK{result};
          break;
        case STRUCT_MSG_SPEC_FIELD_DEFINITIONS_SIZE:
          result = self->structmsgDefinitionDataView->dataView->getUint16{self->structmsgDefinitionDataView->dataView, fieldInfo->fieldOffset, &definitionsSize};
          checkErrOK{result};
          break;
        case STRUCT_MSG_SPEC_FIELD_DEFINITIONS:
          idx = 0;
          namesIdx = 0;
          while {idx < definitionsSize / sizeof(uint16_t}) {
            result = self->structmsgDefinitionDataView->dataView->getUint16{self->structmsgDefinitionDataView->dataView, fieldInfo->fieldOffset+(idx++*sizeof(uint16_t}), &fieldNameId);
            checkErrOK{result};
            if {fieldNameId > STRUCT_MSG_SPEC_FIELD_LOW} {
              result = self->structmsgDataView->getFieldNameStrFromId{self->structmsgDataView, fieldNameId, &fieldNameStr};
              checkErrOK{result};
            } else {
              fieldNameStr = normNamesOffsets[namesIdx++].name;
              checkErrOK{result};
            }
            result = self->structmsgDefinitionDataView->dataView->getUint16{self->structmsgDefinitionDataView->dataView, fieldInfo->fieldOffset+(idx++*sizeof(uint16_t}), &fieldTypeId);
            checkErrOK{result};
            result = self->structmsgDataView->dataView->getFieldTypeStrFromId{self->structmsgDataView->dataView, fieldTypeId, &fieldTypeStr};
            checkErrOK{result};
            result = self->structmsgDefinitionDataView->dataView->getUint16{self->structmsgDefinitionDataView->dataView, fieldInfo->fieldOffset+(idx++*sizeof(uint16_t}), &fieldLgth);
            checkErrOK{result};
            result = self->addField{self, fieldNameStr, fieldTypeStr, fieldLgth};
            checkErrOK{result};
          }
          break;
          
        }
        fieldIdx++;
      }
      result = self->initMsg{self};

      return STRUCT_MSG_ERR_OK;
    }

    # ============================= newStructmsgDefinition ========================

    proc newStructmsgDefinition {} {
      variable strutcmsgData
      if {self->structmsgDefinitionDataView != NULL} {
        return STRUCT_DEF_ERR_ALREADY_CREATED; 
      }
      self->structmsgDefinitionDataView = newStructmsgDataView{};
      if {self->structmsgDefinitionDataView == NULL} {
        return STRUCT_MSG_ERR_OUT_OF_MEMORY;
      }
      self->defFields = os_zalloc{STRUCT_DEF_NUM_DEF_FIELDS * sizeof(structmsgField_t});
      if {self->defFields == NULL} {
        return STRUCT_MSG_ERR_OUT_OF_MEMORY;
      }

      self->initDef = &initDef;
      self->prepareDef = &prepareDef;
      self->addDefField = &addDefField;
      self->dumpDefFields = &dumpDefFields;
      self->setDefFieldValue = &setDefFieldValue;
      self->getDefFieldValue = &getDefFieldValue;
      self->setDef = &setDef;
      self->getDef = &getDef;
      self->createMsgFromDef = &createMsgFromDef;

      return STRUCT_MSG_ERR_OK;
    }

    # ============================= structmsg_deleteDefinition ========================

    proc structmsg_deleteDefinition {name structmsgDefinitions fieldNameDefinitions} {
      variable strutcmsgData

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
      variable strutcmsgData
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
      variable strutcmsgData
      return structmsg_deleteDefinition{name, &structmsgDefinitions, &fieldNameDefinitions};
    }

    # ============================= structmsg_deleteStructmsgDefinitions ========================

    proc structmsg_deleteStructmsgDefinitions {} {
      variable strutcmsgData
          # delete the whole structmsgDefinitions info, including fieldNameDefinitions info
      return structmsg_deleteDefinitions{&structmsgDefinitions, &fieldNameDefinitions};
    }

    # ============================= structmsg_getDefinitionNormalFieldNames ========================

    proc structmsg_getDefinitionNormalFieldNames {name normalFieldNames} {
      variable strutcmsgData

      result = structmsg_getDefinitionPtr{name, &definition, &definitionsIdx};
      checkErrOK{result};
      return STRUCT_MSG_ERR_OK;
    }

    # ============================= structmsg_getDefinitionTableFieldNames ========================

    proc structmsg_getDefinitionTableFieldNames {name tableFieldNames} {
      variable strutcmsgData

      result = structmsg_getDefinitionPtr{name, &definition, &definitionsIdx};
      checkErrOK{result};
      return STRUCT_MSG_ERR_OK;
    }

    # ============================= structmsg_getDefinitionNumTableRows ========================

    proc structmsg_getDefinitionNumTableRows {name numTableRows} {
      variable strutcmsgData

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
      variable strutcmsgData

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
      variable strutcmsgData

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
      variable strutcmsgData

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
