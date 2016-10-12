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

namespace eval structmsg {
  namespace ensemble create

    namespace export structmsgData

  namespace eval structmsgData {
    namespace ensemble create
      
    namespace export structmsgDataInit freeStructmsgDataView createMsg addField
    variable structmsgData [dict create]
    variable numHandles 0
    variable structmsgHandles
    dict set structmsgData flags [list]
    dict set structmsgData handle [list]
    set structmsgHandles [dict create]
    dict set structmsgHandles handles [list]
    dict set structmsgHandles numHandles 0


    # ============================= addHandle ========================
    
    proc addHandle {handle headerVar} {
      variable structmsgData
      variable structmsgHandles
      upvar $headerVar header
    
      if {[dict get $structmsgHandles handles] eq [list]} {
        set handleDict [dict create]
        dict set handleDict handle $handle
        dict set handleDict structmsgData $structmsgData
        dict set handleDict header [list]
        lappend structmsgHandles handles $handleDict
        dict incr structmsgHandles numHandles 1
        set header [list]
        return $::STRUCT_MSG_ERR_OK
      } else {
        # check for unused slot first
        set idx 0
        while {$idx < [dict get $structmsgHandles numHandles]} {
          if {[dict get [lindex [dict get $structmsgHandles handles] $idx] handle] eq [list]} {
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
        set handles [dict get $structmsgHandles handles]
        set idx [dict get structmsgHandles numHandles]
        set entry [lindex $handles $idx]
        dict set entry handle $handle
        dict set entry structmsgData $structmsgData
        set handles [lreplace $handles $idx $idx $entry]
        dict set structmsgHandles handles $handles
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
          return $STRUCT_MSG_ERR_OK
        }
        incr idx
      }
      return $::STRUCT_MSG_ERR_HANDLE_NOT_FOUND
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
puts stderr "handle:$handle!"
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
    
puts stderr "addField"
pdict $structmsgData
    #ets_printf{"addfield: %s fieldType: %s fieldLgth: %d\n", fieldName, fieldType, fieldLgth};
      if {[dict get $structmsgData numFields] >= [dict get $structmsgData maxFields]} {
        return $::STRUCT_MSG_ERR_TOO_MANY_FIELDS
      }
      result = self->structmsgDataView->dataView->getFieldTypeIdFromStr{self->structmsgDataView->dataView, fieldType, &fieldTypeId};
      checkErrOK{result};
      result = self->structmsgDataView->getFieldNameIdFromStr{self->structmsgDataView, fieldName, &fieldNameId, STRUCT_MSG_INCR};
      checkErrOK{result};
      fieldInfo = &self->fields[self->numFields];
      // need to check for duplicate here !!
      if {c_strcmp(fieldName, "@filler"} == 0) {
        self->flags |= STRUCT_MSG_HAS_FILLER;
        fieldLgth = 0;
        fieldInfo->fieldNameId = fieldNameId;
        fieldInfo->fieldTypeId = fieldTypeId;
        fieldInfo->fieldLgth = fieldLgth;
        self->numFields++;
        return STRUCT_MSG_ERR_OK;
      }
      if {c_strcmp(fieldName, "@tablerows"} == 0) {
        self->numTableRows = fieldLgth;
        fieldLgth = 0;
        self->flags |= STRUCT_MSG_HAS_TABLE_ROWS;
        fieldInfo->fieldNameId = fieldNameId;
        fieldInfo->fieldTypeId = fieldTypeId;
        fieldInfo->fieldLgth = fieldLgth;
        self->numFields++;
        return STRUCT_MSG_ERR_OK;
      }
      if {c_strcmp(fieldName, "@tablerowfields"} == 0) {
        self->numTableRowFields = fieldLgth;
        fieldLgth = 0;
        fieldInfo->fieldNameId = fieldNameId;
        fieldInfo->fieldTypeId = fieldTypeId;
        fieldInfo->fieldLgth = fieldLgth;
        numTableFields = self->numTableRows * self->numTableRowFields;
        if {(self->tableFields == NULL} && (numTableFields != 0)) {
          self->tableFields = os_zalloc{sizeof(structmsgField_t} * numTableFields);
          checkAllocOK{self->tableFields};
        }
        self->numFields++;
        return STRUCT_MSG_ERR_OK;
      }
      numTableRowFields = self->numTableRowFields;
      numTableRows = self->numTableRows;
      numTableFields = numTableRows * numTableRowFields;
      if {!((numTableFields > 0} && (self->numRowFields < numTableRowFields)) || (numTableRowFields == 0)) {
        numTableFields = 0;
        numTableRows = 1;
        numTableRowFields = 0;
    
        if {c_strcmp(fieldName, "@crc"} == 0) {
          self->flags |= STRUCT_MSG_HAS_CRC;
          if {c_strcmp(fieldType, "uint8_t"} == 0) {
            self->flags |= STRUCT_MSG_UINT8_CRC;
          }
    //ets_printf{"flags: 0x%02x HAS_CRC: 0x%02x HAS_FILLER: 0x%02x UINT8_CRC: 0x%02x\n", structmsg->flags, STRUCT_MSG_HAS_CRC, STRUCT_MSG_HAS_FILLER, STRUCT_MSG_UINT8_CRC};
        }
        fieldInfo->fieldNameId = fieldNameId;
        fieldInfo->fieldTypeId = fieldTypeId;
        fieldInfo->fieldLgth = fieldLgth;
        self->numFields++;  
      } else {
        row = 0;
        while {row < numTableRows} {
          cellIdx = self->numRowFields + row * numTableRowFields;;
          if {self->tableFields == NULL} {
            return STRUCT_MSG_ERR_BAD_TABLE_ROW;
          }
          fieldInfo = &self->tableFields[cellIdx];
    //ets_printf{"table field1: %s totalLgth: %d cmdLgth: %d\n", fieldInfo->fieldStr, structmsg->hdr.hdrInfo.hdrKeys.totalLgth, structmsg->hdr.hdrInfo.hdrKeys.cmdLgth};
          fieldInfo->fieldNameId = fieldNameId;
          fieldInfo->fieldTypeId = fieldTypeId;
          fieldInfo->fieldLgth = fieldLgth;
          row++;
        }
        self->numRowFields++;  
      } 
      return STRUCT_MSG_ERR_OK;
    }


  } ; # namespace structmsgData
} ; # namespace structmsg
