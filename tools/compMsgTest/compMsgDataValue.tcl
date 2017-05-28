# ===========================================================================
# * Copyright (c) 2017, Arnulf P. Wiedemann (arnulf@wiedemann-pri.de)
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

namespace eval ::compMsg {
  namespace ensemble create

  namespace export compMsgDataValue

  namespace eval compMsgDataValue {
    namespace ensemble create
      
    namespace export dataValueStr2ValueId dataValueId2ValueStr addDataValue
    namespace export setDataValue getDataValue compMsgDataValueInit
    namespace export addMsgFieldValues setMsgFieldValue getMsgFieldValue
    namespace export newMsgFieldValueInfos getMsgFieldValueInfo setMsgFieldValueInfo 
    namespace export dumpMsgFieldValueInfos setFieldValueInfo getFieldValueInfo

    variable dataValueStr2ValueIds 
    set dataValueStr2ValueIds [dict create]
    dict set dataValueStr2ValueIds "@provisioningSsid"     COMP_MSG_WIFI_VALUE_ID_provisioningSsid
    dict set dataValueStr2ValueIds "@provisioningPort"     COMP_MSG_WIFI_VALUE_ID_provisioningPort
    dict set dataValueStr2ValueIds "@provisioningIPAddr"   COMP_MSG_WIFI_VALUE_ID_provisioningIPAddr
    dict set dataValueStr2ValueIds "@SSDPIPAddr"           COMP_MSG_WIFI_VALUE_ID_SSDPIPAddr
    dict set dataValueStr2ValueIds "@SSDPPort"             COMP_MSG_WIFI_VALUE_ID_SSDPPort
    dict set dataValueStr2ValueIds "@SSDPReceivedCallback" COMP_MSG_WIFI_VALUE_ID_SSDPReceivedCallback
    dict set dataValueStr2ValueIds "@SSDPToSendCallback"   COMP_MSG_WIFI_VALUE_ID_SSDPToSendCallback
    dict set dataValueStr2ValueIds "@clientIPAddr"         COMP_MSG_WIFI_VALUE_ID_clientIPAddr
    dict set dataValueStr2ValueIds "@clientPort"           COMP_MSG_WIFI_VALUE_ID_clientPort
    dict set dataValueStr2ValueIds "@clientSequenceNum"    COMP_MSG_WIFI_VALUE_ID_clientSequenceNum
    dict set dataValueStr2ValueIds "@clientSsid"           COMP_MSG_WIFI_VALUE_ID_clientSsid
    dict set dataValueStr2ValueIds "@clientPasswd"         COMP_MSG_WIFI_VALUE_ID_clientPasswd
    dict set dataValueStr2ValueIds "@clientStatus"         COMP_MSG_WIFI_VALUE_ID_clientStatus
    dict set dataValueStr2ValueIds "@binaryCallback"       COMP_MSG_WIFI_VALUE_ID_binaryCallback
    dict set dataValueStr2ValueIds "@textCallback"         COMP_MSG_WIFI_VALUE_ID_textCallback
    dict set dataValueStr2ValueIds "@NetReceivedCallback"  COMP_MSG_WIFI_VALUE_ID_NetReceivedCallback
    dict set dataValueStr2ValueIds "@NetToSendCallback"    COMP_MSG_WIFI_VALUE_ID_NetToSendCallback
    dict set dataValueStr2ValueIds "@NetSecureConnect"     COMP_MSG_WIFI_VALUE_ID_NetSecureConnect
    dict set dataValueStr2ValueIds "@cloudPort"            COMP_MSG_WIFI_VALUE_ID_cloudPort
    dict set dataValueStr2ValueIds "@operatingMode"        COMP_MSG_WIFI_VALUE_ID_operatingMode

    dict set dataValueStr2ValueIds "Reserve1"              COMP_MSG_MODULE_VALUE_ID_Reserve1
    dict set dataValueStr2ValueIds "Reserve2"              COMP_MSG_MODULE_VALUE_ID_Reserve2
    dict set dataValueStr2ValueIds "Reserve3"              COMP_MSG_MODULE_VALUE_ID_Reserve3
    dict set dataValueStr2ValueIds "Reserve4"              COMP_MSG_MODULE_VALUE_ID_Reserve4
    dict set dataValueStr2ValueIds "Reserve5"              COMP_MSG_MODULE_VALUE_ID_Reserve5
    dict set dataValueStr2ValueIds "Reserve6"              COMP_MSG_MODULE_VALUE_ID_Reserve6
    dict set dataValueStr2ValueIds "Reserve7"              COMP_MSG_MODULE_VALUE_ID_Reserve7
    dict set dataValueStr2ValueIds "Reserve8"              COMP_MSG_MODULE_VALUE_ID_Reserve8
    dict set dataValueStr2ValueIds "cryptKey"              COMP_MSG_MODULE_VALUE_ID_cryptKey
    dict set dataValueStr2ValueIds "cryptIvKey"            COMP_MSG_MODULE_VALUE_ID_cryptIvKey

    # ================================= dataValueStr2ValueId ====================================

    proc dataValueStr2ValueId {compMsgDispatcherVar valueStr valueIdVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $valueIdVar valueId
      variable dataValueStr2ValueIds

      if {[dict exists $dataValueStr2ValueIds $valueStr]} {
        set valueId [dict get $dataValueStr2ValueIds $valueStr]
        return [checkErrOK OK]
      }
      checkErrOK FIELD_NOT_FOUND
    }

    # ================================= dataValueId2ValueStr ====================================

    proc  dataValueId2ValueStr {compMsgDispatcherVar valueId valueStrVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $valueStrVar valueStr
      variable dataValueStr2ValueIds

      foreach key [dict keys $dataValueStr2ValueIds] {
        if {[dict get $dataValueStr2ValueIds $key] eq $valueId} {
          set valueStr $key
          return [checkErrOK OK]
        }
      }
      checkErrOK FIELD_NOT_FOUND
    }

    # ================================= addDataValue ====================================

    proc addDataValue {compMsgDispatcherVar dataValue dataValueIdxVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $dataValueIdxVar dataValueIdx

      set result OK
      set compMsgDataValue [dict get $compMsgDispatcher compMsgDataValue]
#puts stderr "num: [dict get $compMsgDataValue numDataValues] max: [dict get $compMsgDataValue maxDataValues]!"
      if {[dict get $compMsgDataValue numDataValues] >= [dict get $compMsgDataValue maxDataValues]} {
        if {[dict get $compMsgDataValue maxDataValues] == 0} {
          dict set compMsgDataValue maxDataValues 5
          set lst [list]
          set valIdx 0
          while {$valIdx < 5} {
            lappend lst [list]
            incr valIdx
          }
          dict set compMsgDataValue dataValues $lst
        } else {
          dict incr compMsgDataValue maxDataValues 2
          set valIdx 0
          while {$valIdx < 2} {
            dict lappend compMsgDataValue dataValues [list]
            incr valIdx
          }
        }
      }
      set dataValues [dict get $compMsgDataValue dataValues]
      set dataValueIdx [dict get $compMsgDataValue numDataValues]
      set myDataValue [lindex $dataValues $dataValueIdx]

      dict set myDataValue cmdKey [dict get $dataValue cmdKey]
      dict set myDataValue fieldValueId [dict get $dataValue fieldValueId]
      dict set myDataValue fieldNameId [dict get $dataValue fieldNameId]
      dict set myDataValue fieldValueCallback [dict get $dataValue fieldValueCallback]
      dict set myDataValue flags [dict get $dataValue flags]
      dict set myDataValue value [dict get $dataValue value]
      if {[dict get $myDataValue fieldValueCallback] ne ""} {
        dict lappend myDataValue flags FIELD_HAS_CALLBACK
      }
      set dataValues [lreplace $dataValues $dataValueIdx $dataValueIdx $dataValue]
      set dataValueIdx [dict get $compMsgDataValue numDataValues]
      dict incr compMsgDataValue numDataValues
      dict set compMsgDataValue dataValues $dataValues
      dict set compMsgDispatcher compMsgDataValue $compMsgDataValue
      return [checkErrOK OK]
    }

    # ================================= setDataValue ====================================

    proc setDataValue {compMsgDispatcherVar cmdKey valueId fieldValueCallback numericValue stringValue} {
      upvar $compMsgDispatcherVar compMsgDispatcher

      result = OK
      set compMsgDataValue [dict get $compMsgDispatcher compMsgDataValue]
      set idx 0
      while {$idx < [dict get $compMsgDataValue numDataValues} {
        set dataValues [dict get $compMsgDataValue dataValues]
        set dataValue [lindex $dataValues $idx]
        if {([dict get dataValue valueId] eq $valueId) && ([dict get dataValue cmdKey] eq $cmdKey)} {
          dict set dataValue flags [list]
          if {$stringValue eq ""} {
            dict lappend flags COMP_MSG_FIELD_IS_NUMERIC
            dict set dataValue value $numericValue
          }
          if {$stringValue ne ""} {
            dict lappend flags COMP_MSG_FIELD_IS_STRING
            dict set dataValue value $numericValue
          }
          if {$fieldValueCallback ne""} {
            dict lappend dataValue flags COMP_MSG_FIELD_HAS_CALLBACK
          }
          dict set dataValue fieldValueCallback $fieldValueCallback
          set dataValues [lreplace $dataValues $idx $idx $dataValue]
          dict set compMsgDataValue dataValues $dataValues
          dict set compMsgDispatcher compMsgDataValue $compMsgDataValue
          return [checkErrOK OK]
        }
        incr idx
      }
      checkErrOK DATA_VALUE_FIELD_NOT_FOUND
    }

    # ================================= getDataValue ====================================

    #*
    # * \brief get value from data area
    # * \param self The dispatcher struct
    # * \param cmdKey Cmd key of the value
    # * \param fieldId Value id of the value
    # * \param fieldValueCallback The field value callback, if there exists one
    # * \param numericValue The value if it is a numeric one
    # * \param stringValue The value if it is a character string
    # * \return Error code or ErrorOK
    # *

    proc getDataValue {compMsgDispatcherVar cmdKey valueId flagsVar fieldValueCallbackVar numericValueVar stringValueVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $flagsVar flags
      upvar $fieldValueCallbackVar fieldValueCallback
      upvar $numericValueVar numericValue
      upvar $stringValueVar stringValue

      set result OK
      set compMsgDataValue = self->compMsgDataValue;
      Value ""
      set flags ""
      set fieldValueCallback ""
      set idx 0
      while {$idx < [dict get $compMsgDataValue numDataValues]} {
        set dataValues [dict get $compMsgDataValue dataValues]
        set dataValue [lindex $dataValues $idx]
        if {([dict get $dataValue valueId] eq $valueId) && ([dict get $dataValue cmdKey] eq $cmdKey)} {
          set result [dataValueId2ValueStr compMsgDispatcher $valueId value]
          checkErrOK $result
          set flags [dict get $dataValue flags]
          set stringValue ""
          set numericValue 0
          if {[lsearch $flags COMP_MSG_FIELD_IS_NUMERIC] >= 0} {
            set numericValue [dict get $dataValue value]
          }
          if {[lsearch $flags COMP_MSG_FIELD_IS_STRING] >= 0} {
            set stringValue [dict get $dataValue value]
          }
          puts stderr [format "getDataValue: %s %s" $numericValue $stringValue]
          set fieldValueCallback [dict get $dataValue fieldValueCallback]
          set dataValues [lreplace $dataValues $idx $idx $dataValue]
          dict set compMsgDataValue dataValues $dataValues
          dict set compMsgDispatcher compMsgDataValue $compMsgDataValue
          return [checkErrOK OK]
        }
        incr idx
      }
      puts stderr [format "getDataValue: DATA_VALUE_FIELD_NOT_FOUND: %s %s" $cmdKey $valueId]
      checkErrOK DATA_VALUE_FIELD_NOT_FOUND
    }

    # ================================= newMsgFieldValueInfos ====================================

    proc newMsgFieldValueInfos {compMsgDispatcherVar cmdKey numEntries msgFieldValueVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $msgFieldValueVar msgFieldValue

      set result $::COMP_MSG_ERR_OK
      set found false
      set compMsgDataValue [dict get $compMsgDispatcher compMsgDataValue]
      set msgFieldValueInfos [dict get $compMsgDataValue msgFieldValueInfos]
      set cmdKeyIdx 0
puts stderr [format "newMsgFieldValueInfos: cmdKey: %s numMsgFieldValues: %d" $cmdKey [dict get $msgFieldValueInfos numMsgFieldValues]]
      while {$cmdKeyIdx < [dict get $msgFieldValueInfos numMsgFieldValues]} {
        set msgFieldValues [dict get $msgFieldValueInfos msgFieldValues]
        set msgFieldValue [lindex $msgFieldValues $cmdKeyIdx]
        if {[dict get $msgFieldValue cmdKey] eq $cmdKey} {
          set found true
          break
        }
        incr cmdKeyIdx
      }
      if {!$found} {
        if {[dict get $msgFieldValueInfos numMsgFieldValues] >= [dict get $msgFieldValueInfos maxMsgFieldValues]} {
          if {[dict get $msgFieldValueInfos maxMsgFieldValues] == 0} {
            dict set msgFieldValueInfos maxMsgFieldValues 5
            dict set msgFieldValueInfos msgFieldValues [list]
          } else {
            dict incr msgFieldValueInfos maxMsgFieldValues 2
          }
          set idx [dict get $msgFieldValueInfos numMsgFieldValues]
          while {$idx < [dict get $msgFieldValueInfos maxMsgFieldValues]} {
            dict lappend msgFieldValueInfos msgFieldValues [list]
            incr idx
          }
        }
        set msgFieldValues [dict get $msgFieldValueInfos msgFieldValues]
puts stderr "numMsgFieldValues: [dict get $msgFieldValueInfos numMsgFieldValues]!"
        set msgFieldValueIdx [dict get $msgFieldValueInfos numMsgFieldValues]
        set msgFieldValue  [lindex $msgFieldValues $msgFieldValueIdx]
        dict incr msgFieldValueInfos numMsgFieldValues
        dict set msgFieldValue cmdKey $cmdKey
        dict set msgFieldValue numFieldValues $numEntries
        set idx 0
        dict set msgFieldValue fieldValues [list]
        set myDict [dict create]
        dict set myDict fieldValueFlags [list]
        dict set myDict fieldNameId 0
        dict set myDict fieldValueCallbackId 0
        dict set myDict dataValue [list]
        dict set myDict fieldValueCallback [list]
        while {$idx < $numEntries} {
          dict lappend msgFieldValue fieldValues $myDict
          incr idx
        }
        set msgFieldValues [lreplace $msgFieldValues $msgFieldValueIdx $msgFieldValueIdx $msgFieldValue]
        dict set msgFieldValueInfos msgFieldValues $msgFieldValues
        dict set compMsgDataValue msgFieldValueInfos $msgFieldValueInfos
puts stderr "compMsgDataValue: $compMsgDataValue!"
        dict set compMsgDispatcher compMsgDataValue $compMsgDataValue
      }
      return $result
    }
 
    # ================================= getMsgFieldValueInfo ====================================

    proc getMsgFieldValueInfo {compMsgDispatcherVar cmdKey msgFieldValueVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $msgFieldValueVar msgFieldValue

      set result $::COMP_MSG_ERR_OK
      set compMsgDataValue [dict get $compMsgDispatcher compMsgDataValue]
      set msgFieldValueInfos [dict get $compMsgDataValue msgFieldValueInfos]
      set cmdKeyIdx 0
      set msgFieldValues [dict get $msgFieldValueInfos msgFieldValues]
      while {$cmdKeyIdx < [dict get $msgFieldValueInfos numMsgFieldValues]} {
        set msgFieldValue [lindex $msgFieldValues $cmdKeyIdx]
puts stderr "cmdKey: $cmdKey!$msgFieldValue!"
        if {[dict get $msgFieldValue cmdKey] eq $cmdKey} {
          return $::COMP_MSG_ERR_OK
        }
        incr cmdKeyIdx
      }
      return [checkErrOK FIELD_VALUE_INFO_NOT_FOUND]
    }
 
    # ================================= setMsgFieldValueInfo ====================================

    proc setMsgFieldValueInfo {compMsgDispatcherVar msgFieldValue fieldValue} {
      upvar $compMsgDispatcherVar compMsgDispatcher

      set result $::COMP_MSG_ERR_OK
      set fieldValueIdx 0
      set found false
      set freeFieldValue [list]
      set compMsgDataValue [dict get $compMsgDispatcher compMsgDataValue]
#ets_printf("setMsgFieldValueInfo: numFieldValues: %d\n", msgFieldValue->numFieldValues);
      # first check if this entry exists, if not take the first unused one
      while {$fieldValueIdx < [dict get $msgFieldValue numFieldValues]} { 
        set fieldValues [dict get $msgFieldValue fieldValues]
        set myFieldValue [lindex $fieldValues $fieldValueIdx]
        if {[dict get $myFieldValue fieldNameId] == 0} {
          if {$freeFieldValue eq [list]} {
            set freeFieldValue $myFieldValue
          }
        } else {
          if {[dict get $myFieldValue fieldNameId] == [dict get $fieldValue fieldNameId]} {
            set found true
            break
          }
        }
        incr fieldValueIdx
      }
      if {$found} {
#ets_printf("setMsgFieldValueInfo: duplicate entry for: %d\n", fieldValue->fieldNameId);
          return [checkErrOK DUPLICATE_FIELD_VALUE_ENTRY]
      } else {
        if {$freeFieldValue ne [list]} {
          set myFieldValue $freeFieldValue
set result [::compMsg compMsgTypesAndNames getFieldNameStrFromId compMsgDispatcher [dict get $fieldValue fieldNameId] fieldName]
checkErrOK $result
puts stderr "fieldValue: $fieldValue!"
set value [dict get $fieldValue dataValue]
#ets_printf("found free %s flags: 0x%08x fieldNameId: %d %s 0x%04x %d callbackId: %d\n", fieldName, fieldValue->fieldValueFlags, fieldValue->fieldNameId, stringValue, numericValue, numericValue, fieldValue->fieldValueCallbackId);
          dict set myFieldValue fieldNameId [dict get $fieldValue fieldNameId]
          dict set myFieldValue fieldValueFlags [dict get $fieldValue fieldValueFlags]
          dict set myFieldValue DataValue [dict get $fieldValue dataValue]
          dict set myFieldValue fieldValueCallbackId [dict get $fieldValue fieldValueCallbackId]
          dict set myFieldValue fieldValueCallback [dict get $fieldValue fieldValueCallback]
        } else {
#ets_printf("setMsgFieldValueInfo: not found too many entries\n");
          return [checkErrOK TOO_MANY_FIELD_VALUE_ENTRIES]
        }
      }
      return $result
    }
 
    # ================================= dumpMsgFieldValueInfos ====================================

    proc dumpMsgFieldValueInfos {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher

      result = COMP_MSG_ERR_OK;
      noPrefix = self->compMsgDebug->noPrefix;
      self->compMsgDebug->noPrefix = true;
      compMsgTypesAndNames = self->compMsgTypesAndNames;
      compMsgDataValue = self->compMsgDataValue;
      COMP_MSG_DBG(self, "d", 1, "dumpMsgFieldValueInfos");
      msgFieldValueInfos = &compMsgDataValue->msgFieldValueInfos;
      // entry 0 is not used, fieldNameIds start at 1!
      cmdKeyIdx = 0;
      while (cmdKeyIdx < msgFieldValueInfos->numMsgFieldValues) {
        msgFieldValue = &msgFieldValueInfos->msgFieldValues[cmdKeyIdx];
        ets_sprintf(buf, "  %02d cmdKey: 0x%04x\n", cmdKeyIdx, msgFieldValue->cmdKey);
        COMP_MSG_DBG(self, "d", 1, "%s", buf);
        buf[0] = '\0';
        fieldValueIdx = 0;
        while (fieldValueIdx < msgFieldValue->numFieldValues) {
          fieldValue = &msgFieldValue->fieldValues[fieldValueIdx];
          if (fieldValue->fieldNameId == 0) {
          } else {
            result = compMsgTypesAndNames->getFieldNameStrFromId(self, fieldValue->fieldNameId, &fieldName);
            if (fieldValue->fieldValueFlags & COMP_MSG_FIELD_IS_STRING) {
              numericValue = 0;
              stringValue = fieldValue->dataValue.value.stringValue;
              if (stringValue == NULL) {
                stringValue = "nil";
              }
            } else {
              numericValue = fieldValue->dataValue.value.numericValue;
              stringValue = "nil";
            }
            if (fieldValue->fieldValueCallbackId == 0) {
              callbackName = "nil";
            } else {
              result = self->compMsgWifiData->callbackId2CallbackStr(fieldValue->fieldValueCallbackId, &callbackName);
    ets_printf("id: %d result: %d\n", fieldValue->fieldValueCallbackId, result);
              if (result != COMP_MSG_ERR_OK) {
                result = self->compMsgModuleData->callbackId2CallbackStr(fieldValue->fieldValueCallbackId, &callbackName);
    ets_printf("id2: %d result: %d\n", fieldValue->fieldValueCallbackId, result);
              }
            }
            ets_sprintf(buf, "    %-20s: id: %d value: %-15s 0x%08x %6d callback: %-20s %3d flags: 0x%04x\n", fieldName, fieldValue->fieldNameId, stringValue, numericValue, numericValue, callbackName, fieldValue->fieldValueCallbackId, fieldValue->fieldValueFlags);
            if (fieldValue->fieldValueFlags & COMP_MSG_FIELD_IS_STRING) {
              c_strcat(buf, " IS_STRING");
            }
            if (fieldValue->fieldValueFlags & COMP_MSG_FIELD_IS_NUMERIC) {
              c_strcat(buf, " IS_NUMERIC");
            }
            if (fieldValue->fieldValueFlags & COMP_MSG_FIELD_HAS_CALLBACK) {
              c_strcat(buf, " HAS_CALLBACK");
            }
            COMP_MSG_DBG(self, "d", 1, "%s", buf);
          }
          fieldValueIdx++;
        }
        cmdKeyIdx++;
      }
      self->compMsgDebug->noPrefix = noPrefix;
      return result;
    }

    # ================================= setFieldValueInfo ====================================

    proc setFieldValueInfo {compMsgDispatcherVar cmdKey fieldNameId fieldValue} {
      upvar $compMsgDispatcherVar compMsgDispatcher

      result = COMP_MSG_ERR_OK;
      found = false;
      compMsgDataValue = self->compMsgDataValue;
      msgFieldValueInfos = &compMsgDataValue->msgFieldValueInfos;
      idx = 0;
      while (idx < msgFieldValueInfos->numMsgFieldValues) {
        msgFieldValue = &msgFieldValueInfos->msgFieldValues[idx];
        if (msgFieldValue->cmdKey == cmdKey) {
          fieldIdx = 0;
          while (fieldIdx < msgFieldValue->numFieldValues) {
            myFieldValue = &msgFieldValue->fieldValues[fieldIdx];
            if ((myFieldValue->fieldNameId == fieldNameId)) {
              if ((myFieldValue->fieldValueFlags & COMP_MSG_FIELD_IS_STRING) && (myFieldValue->dataValue.value.stringValue != NULL)) {
                os_free(myFieldValue->dataValue.value.stringValue);
              }
              myFieldValue->fieldValueFlags = fieldValue->fieldValueFlags;
              if (fieldValue->fieldValueFlags & COMP_MSG_FIELD_IS_NUMERIC) {
                myFieldValue->dataValue.value.numericValue = fieldValue->dataValue.value.numericValue;
    stringValue = "nil";
    numericValue = myFieldValue->dataValue.value.numericValue;
              } else {
                myFieldValue->fieldValueFlags |= COMP_MSG_FIELD_IS_STRING;
                myFieldValue->dataValue.value.stringValue = os_zalloc(c_strlen(fieldValue->dataValue.value.stringValue) + 1);
                c_memcpy(myFieldValue->dataValue.value.stringValue, fieldValue->dataValue.value.stringValue, c_strlen(fieldValue->dataValue.value.stringValue));
        stringValue = myFieldValue->dataValue.value.stringValue;
      numericValue = 0;
              }
              myFieldValue->fieldValueCallbackId = fieldValue->fieldValueCallbackId;
              if (fieldValue->fieldValueCallback != NULL) {
                myFieldValue->fieldValueFlags |= COMP_MSG_FIELD_HAS_CALLBACK;
                myFieldValue->fieldValueCallback = fieldValue->fieldValueCallback;
              }
      ets_printf("setFieldValueInfo: flags: 0x%04x numeric: 0x%04x %d string: %s callback: %p\n", fieldValue->fieldValueFlags, numericValue, numericValue, stringValue, myFieldValue->fieldValueCallback);
              return result;
            }
            idx++;
          }
        }
      }
      return COMP_MSG_ERR_DATA_VALUE_FIELD_NOT_FOUND;
    }

    # ================================= getFieldValueInfo ====================================

    #**
    # * \brief get value from data area
    # * \param self The dispatcher struct
    # * \param cmdKey Message cmdKey
    # * \param fieldNameId Message fieldNameId within cmdKey
    # * \param fieldValueInfo FieldValue struct pointer for passing values out
    # * \return Error code or ErrorOK
    # *
    # *
    proc getFieldValueInfo {compMsgDispatcherVar cmdKey fieldNameId fieldValueVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $fieldValueVar fieldValueVar

COMP_MSG_DBG(self, "E", 2, "getFieldValueInfo: %p %d 0x%04x", fieldValue, fieldNameId, cmdKey);
      result = COMP_MSG_ERR_OK;
      compMsgDataValue = self->compMsgDataValue;
      msgFieldValueInfos = &compMsgDataValue->msgFieldValueInfos;
      cmdKeyIdx = 0;
      while (cmdKeyIdx < msgFieldValueInfos->numMsgFieldValues) {
        msgFieldValue = &msgFieldValueInfos->msgFieldValues[cmdKeyIdx];
        if (msgFieldValue->cmdKey == cmdKey) {
          idx = 0;
          while (idx < msgFieldValue->numFieldValues) {
            myFieldValue = &msgFieldValue->fieldValues[idx];
            if ((myFieldValue->fieldNameId == fieldNameId)) {
              *fieldValue = myFieldValue;
              return result;
            }
            idx++;
          }
        }
        cmdKeyIdx++;
      }
      COMP_MSG_DBG(self, "E", 1, "getFieldValueInfo: DATA_VALUE_FIELD_NOT_FOUND: 0x%04x %d", cmdKey, fieldNameId);
      return COMP_MSG_ERR_DATA_VALUE_FIELD_NOT_FOUND;
    }

    # ================================= compMsgDataValueInit ====================================

    proc compMsgDataValueInit {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher

      set compMsgDataValue [dict create]
      dict set compMsgDataValue numDataValues 0
      dict set compMsgDataValue maxDataValues 0
      dict set compMsgDataValue dataValues [list]

      set msgFieldValueInfos [dict create]
      dict set msgFieldValueInfos numMsgFieldValues 0
      dict set msgFieldValueInfos maxMsgFieldValues 0
      dict set msgFieldValueInfos msgFieldValues [list]
      dict set compMsgDataValue msgFieldValueInfos $msgFieldValueInfos

      set msgFieldValues [dict create]
      dict set msgFieldValues numMsgFields 0
      dict set msgFieldValues msgFieldValues [list]
      dict set compMsgDataValue msgFieldValues $msgFieldValues

      dict set compMsgDispatcher compMsgDataValue $compMsgDataValue
      return [checkErrOK OK]
    }

  } ; # namespace compMsgDataValue
} ; # namespace compMsg
