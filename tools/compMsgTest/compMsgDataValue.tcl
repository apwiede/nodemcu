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
    namespace export setDataValue getDataValue

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

    proc dataValueStr2ValueId {compMsgDispatcherVar valueStr valueId} {
      upvar $compMsgDispatcherVar compMsgDispatcher

      entry = &dataValueStr2ValueIds[0];
      while (entry->str != NULL) {
        if (c_strcmp(entry->str, valueStr) == 0) {
          *valueId = entry->id;
          return COMP_MSG_ERR_OK;
        }
        entry++;
      }
      return COMP_MSG_ERR_FIELD_NOT_FOUND;
    }

    # ================================= dataValueId2ValueStr ====================================

    proc  dataValueId2ValueStr {compMsgDispatcherVar valueId valueStr} {
      upvar $compMsgDispatcherVar compMsgDispatcher

      entry = &dataValueStr2ValueIds[0];
      while (entry->str != NULL) {
        if (entry->id == valueId) {
          *valueStr = entry->str;
          return COMP_MSG_ERR_OK;
        }
        entry++;
      }
      return COMP_MSG_ERR_FIELD_NOT_FOUND;
    }

    # ================================= addDataValue ====================================

    proc addDataValue {compMsgDispatchervar valueId fieldValueCallback Value} {
      upvar $compMsgDispatcherVar compMsgDispatcher

      result = COMP_MSG_ERR_OK;
      compMsgDataValue = self->compMsgDataValue;
      if (compMsgDataValue->numDataValues >= compMsgDataValue->maxDataValues) {
        if (compMsgDataValue->maxDataValues == 0) {
          compMsgDataValue->maxDataValues = 5;
          compMsgDataValue->dataValues = (dataValue_t *)os_zalloc((compMsgDataValue->maxDataValues * sizeof(dataValue_t)));
          checkAllocOK(compMsgDataValue->dataValues);
        } else {
          compMsgDataValue->maxDataValues += 2;
          compMsgDataValue->dataValues = (dataValue_t *)os_realloc((compMsgDataValue->dataValues), (compMsgDataValue->maxDataValues * sizeof(dataValue_t)));
          checkAllocOK(compMsgDataValue->dataValues);
        }
      }
      dataValue = &compMsgDataValue->dataValues[compMsgDataValue->numDataValues];
      memset(dataValue, 0, sizeof(dataValue_t));
      dataValue->valueId = valueId;
      dataValue->flags = 0;
      if (stringValue == NULL) {
        dataValue->flags |= COMP_MSG_FIELD_IS_NUMERIC;
        dataValue->value.numericValue = numericValue;
      } else {
        dataValue->flags |= COMP_MSG_FIELD_IS_STRING;
        dataValue->value.stringValue = os_zalloc(c_strlen(stringValue) + 1);
        c_memcpy(dataValue->value.stringValue, stringValue, c_strlen(stringValue));
      }
      if (fieldValueCallback != NULL) {
        dataValue->flags |= COMP_MSG_FIELD_HAS_CALLBACK;
        dataValue->fieldValueCallback = fieldValueCallback;
      }
      compMsgDataValue->numDataValues++;
      return result;
    }

    # ================================= setDataValue ====================================

    proc setDataValue {compMsgDispatcherVar valueId fieldValueCallback value} {
      upvar $compMsgDispatcherVar compMsgDispatcher

      result = COMP_MSG_ERR_OK;
      compMsgDataValue = self->compMsgDataValue;
      idx = 0;
      while (idx < compMsgDataValue->numDataValues) {
        dataValue = &compMsgDataValue->dataValues[idx];
        if (dataValue->valueId == valueId) {
          dataValue->valueId = valueId;
          if ((dataValue->flags & COMP_MSG_FIELD_IS_STRING) && (dataValue->value.stringValue != NULL)) {
            os_free(dataValue->value.stringValue);
          }
          dataValue->flags = 0;
          if (stringValue == NULL) {
            dataValue->flags |= COMP_MSG_FIELD_IS_NUMERIC;
            dataValue->value.numericValue = numericValue;
          } else {
            dataValue->flags |= COMP_MSG_FIELD_IS_STRING;
            dataValue->value.stringValue = os_zalloc(c_strlen(stringValue) + 1);
            c_memcpy(dataValue->value.stringValue, stringValue, c_strlen(stringValue));
          }
          if (fieldValueCallback != NULL) {
            dataValue->flags |= COMP_MSG_FIELD_HAS_CALLBACK;
            dataValue->fieldValueCallback = fieldValueCallback;
          }
          return result;
        }
        idx++;
      }
      return COMP_MSG_ERR_DATA_VALUE_FIELD_NOT_FOUND;
    }

    # ================================= getDataValue ====================================

#*
# * \brief get value from data area
# * \param self The dispatcher struct
# * \param fieldId Value id of the value
# * \param fieldValueCallback The field value callback, if there exists one
# * \param numericValue The value if it is a numeric one
# * \param stringValue The value if it is a character string
# * \return Error code or ErrorOK
# *

    proc getDataValue {compMsgDispatcherVar valueId flags fieldValueCallback value} {
      upvar $compMsgDispatcherVar compMsgDispatcher

      result = COMP_MSG_ERR_OK;
      compMsgDataValue = self->compMsgDataValue;
      *numericValue = 0;
      *stringValue = NULL;
      *flags = 0;
      *fieldValueCallback = NULL;
      idx = 0;
      while (idx < compMsgDataValue->numDataValues) {
        dataValue = &compMsgDataValue->dataValues[idx];
        if (dataValue->valueId == valueId) {
          result = compMsgDataValue->dataValueId2ValueStr(self, valueId, &valueStr);
          checkErrOK(result);
          *flags = dataValue->flags;
          if (dataValue->flags & COMP_MSG_FIELD_IS_STRING) {
            *stringValue = dataValue->value.stringValue;
            COMP_MSG_DBG(self, "E", 1, "getDataValue: %s %s", valueStr, *stringValue);
          }
          if (dataValue->flags & COMP_MSG_FIELD_IS_NUMERIC) {
            *numericValue = dataValue->value.numericValue;
            COMP_MSG_DBG(self, "E", 1, "getDataValue: %s 0x%08x %d", valueStr, *numericValue, *numericValue);
          }
          *fieldValueCallback = dataValue->fieldValueCallback;
          return result;
        }
        idx++;
      }
      COMP_MSG_DBG(self, "E", 1, "getDataValue: DATA_VALUE_FIELD_NOT_FOUND: %d", valueId);
      return COMP_MSG_ERR_DATA_VALUE_FIELD_NOT_FOUND;
    }

  } ; # namespace compMsgDataValue
} ; # namespace compMsg
