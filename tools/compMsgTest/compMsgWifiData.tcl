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

# bssScanInfo {
#   bssid
#   bssidStr
#   ssid
#   ssid_len
#   channel
#   rssi
#   authmode
#   is_hidden
#   freq_offset
#   freqcal_val

# bssScanSizes dict
#   bssidSize
#   bssidStrSize
#   ssidSize
#   channelSize
#   rssiSize
#   authmodeSize
#   is_hiddenSize
#   freq_offsetSize
#   freqcal_valSize

# bssScanTypes dict
#   bssidType
#   bssidStrType
#   ssidType
#   channelType
#   rssiType
#   authmodeType
#   is_hiddenType
#   freq_offsetType
#   freqcal_valType

# bssScanInfos dict
#   infos
#   numScanInfos
#   maxScanInfos
#   scanInfoComplete
#   *compMsgDispatcher

# stationConfig dict
#   ssid[32]
#   password[64]
#   bssidSet
#   bssid[6]
#   bssidStr[18]
#   status
#   mode
#   authmode
#   channel
#   freq_offset
#   freqcal_val

namespace eval compMsg {
  namespace ensemble create

    namespace export compMsgWifiData

  namespace eval compMsgWifiData {
    namespace ensemble create
      
    namespace export bssStr2BssInfoId

    variable bssScanRunning
    variable bssScanInfos
    variable stationConfig
    variable compMsgWifiData 
    variable bssStr2BssInfoIds

    set bssScanRunning false
    set bssScanInfos [dict create]
    set stationConfig [dict create]
    set compMsgWifiData [dict create]

    set  bssStr2BssInfoIds [dict create]
    dict set bssStr2BssInfoIds bssid       BSS_INFO_BSSID
    dict set bssStr2BssInfoIds bssidStr    BSS_INFO_BSSID_STR
    dict set bssStr2BssInfoIds ssid        BSS_INFO_SSID
    dict set bssStr2BssInfoIds ssid_len    BSS_INFO_SSID_LEN
    dict set bssStr2BssInfoIds channel     BSS_INFO_CHANNEL
    dict set bssStr2BssInfoIds rssi        BSS_INFO_RSSI
    dict set bssStr2BssInfoIds authmode    BSS_INFO_AUTH_MODE
    dict set bssStr2BssInfoIds is_hidden   BSS_INFO_IS_HIDDEN
    dict set bssStr2BssInfoIds freq_offset BSS_INFO_FREQ_OFFSET
    dict set bssStr2BssInfoIds freqcal_val BSS_INFO_FREQ_CAL_VAL

    # ================================= bssStr2BssInfoId ====================================
    
    proc bssStr2BssInfoId {fieldName fieldIdVar} {
      variable bssStr2BssInfoIds
      upvar $fieldIdVar fieldId
    
      if {[dict exists $bssStr2BssInfoIds $fieldName]} {
        set fieldId [dict get $bssStr2BssInfoIds $fieldName]
        return $::COMP_MSG_ERR_OK
      }
      checkErrOK $::COMP_MSG_ERR_FIELD_NOT_FOUND
    }
    
    # ================================= websocketBinaryReceived ====================================
    
    proc websocketBinaryReceived {arg wud pdata len} {
      compMsgDispatcher_t *compMsgDispatcher
      compMsgDispatcher_t *self
      uint8_t result
    
      compMsgDispatcher = {compMsgDispatcher_t *}arg
      self = compMsgDispatcher
    #ets_printf{"websocketBinaryReceived: len: %d dispatcher: %p\n", len, compMsgDispatcher}
      result = self->resetMsgInfo{self, &self->received}
    #  checkErrOK{result}
      self->wud = wud
      result = compMsgDispatcher->handleReceivedPart{compMsgDispatcher, {uint8_t *}pdata, {uint8_t}len}
    ets_printf{"websocketBinaryReceived end result: %d\n", result}
    }
    
    # ================================= websocketTextReceived ====================================
    
    proc websocketTextReceived {arg wud pdata len} {
      compMsgDispatcher_t *compMsgDispatcher
    
      compMsgDispatcher = {compMsgDispatcher_t *}arg
    #ets_printf{"websocketTextReceived: len: %d dispatcher: %p\n", len, compMsgDispatcher}
    }
    
    # ================================= bssScanDoneCb ====================================
    
    proc bssScanDoneCb {arg status} {
      struct bss_info *bss_link
      uint8_t numEntries
      bssScanInfo_t *scanInfo
    
    #ets_printf{"bssScanDoneCb bssScanRunning: arg: %p %d status: %d!\n", arg, bssScanRunning, status}
      if {arg == NULL} {
        return
      }
      if {status != COMP_DISP_ERR_OK} {
        return
      }
      if {bssScanRunning == false} {
        return
      }
      if {bssScanRunning == true} {
        bssScanRunning = false
      }
    #ets_printf{"bssScanDoneCb bssScanRunning2: %d status: %d!\n", bssScanRunning, status}
      numEntries = 0
      bss_link = {struct bss_info *}arg
      while {bss_link != NULL} {
        numEntries++
        bss_link = bss_link->next.stqe_next
      }
      if {bssScanInfos.infos != NULL} {
        os_free{bssScanInfos.infos}
        bssScanInfos.infos = NULL
        bssScanInfos.numScanInfos = 0
        bssScanInfos.maxScanInfos = 0
      }
      bssScanInfos.maxScanInfos = numEntries
      bssScanInfos.infos = os_zalloc{bssScanInfos.maxScanInfos * sizeof{bssScanInfo_t}}
      if {bssScanInfos.infos == NULL} {
        return
      }
      bss_link = {struct bss_info *}arg
      compMsgWifiData.bssScanSizes.bssidSize = 0
      compMsgWifiData.bssScanSizes.bssidStrSize = 0
      compMsgWifiData.bssScanSizes.ssidSize = 0
      compMsgWifiData.bssScanSizes.channelSize = 0
      compMsgWifiData.bssScanSizes.rssiSize = 0
      compMsgWifiData.bssScanSizes.authmodeSize = 0
      compMsgWifiData.bssScanSizes.is_hiddenSize = 0
      compMsgWifiData.bssScanSizes.freq_offsetSize = 0
      compMsgWifiData.bssScanSizes.freqcal_valSize = 0
      while {bss_link != NULL} {
        scanInfo = &bssScanInfos.infos[bssScanInfos.numScanInfos]
        c_memset{scanInfo->ssid, 0, sizeof{scanInfo->ssid}}
        if {bss_link->ssid_len <= sizeof{scanInfo->ssid}} {
          c_memcpy{scanInfo->ssid, bss_link->ssid, bss_link->ssid_len}
          compMsgWifiData.bssScanSizes.ssidSize += bss_link->ssid_len + 1
        } else {
          c_memcpy{scanInfo->ssid, bss_link->ssid, sizeof{scanInfo->ssid}}
          compMsgWifiData.bssScanSizes.ssidSize += sizeof{scanInfo->ssid} + 1
        }
        scanInfo->ssid_len = bss_link->ssid_len
        c_memset{scanInfo->bssidStr, 0, sizeof{scanInfo->bssidStr}}
        c_memcpy{scanInfo->bssid, bss_link->bssid, sizeof{scanInfo->bssid}}
        compMsgWifiData.bssScanSizes.bssidSize += sizeof{scanInfo->bssid} + 1
        c_sprintf{scanInfo->bssidStr,MACSTR, MAC2STR{bss_link->bssid}}
        scanInfo->channel = bss_link->channel
        compMsgWifiData.bssScanSizes.channelSize += 1
        scanInfo->rssi = bss_link->rssi
        compMsgWifiData.bssScanSizes.rssiSize += 1
        scanInfo->authmode = bss_link->authmode
        compMsgWifiData.bssScanSizes.authmodeSize += 1
        scanInfo->is_hidden = bss_link->is_hidden
        compMsgWifiData.bssScanSizes.is_hiddenSize += 1
        scanInfo->freq_offset = bss_link->freq_offset
        compMsgWifiData.bssScanSizes.freq_offsetSize += 2
        scanInfo->freqcal_val = bss_link->freqcal_val
        compMsgWifiData.bssScanSizes.freqcal_valSize += 2
        bss_link = bss_link->next.stqe_next
        bssScanInfos.numScanInfos++
      }
      bssScanInfos.scanInfoComplete = true
      bssScanInfos.compMsgDispatcher->buildMsg{bssScanInfos.compMsgDispatcher}
    }
    
    # ================================= getBssScanInfo ====================================
    
    proc getBssScanInfo {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher
    
    #ets_printf{"getBssScanInfo1: \n"}
      if {$bssScanRunning} {
        # silently ignore 
        return $::COMP_DISP_ERR_OK
      }
      set bssScanRunning true
      dict set scan_config ssid ""
      dict set scan_config bssid ""
      dict set scan_config channel 0
      dict set scan_config show_hidden 1
      dict set compMsgDispatcher bssScanInfos scanInfoComplete false
      set result [wifi_station_scan scan_config bssScanDoneCb]
    #ets_printf{"getBssScanInfo2: result: %d\n", result}
      if {$result != true} {
        checkErrOK $::COMP_DISP_ERR_STATION_SCAN
      }
    #ets_printf{"getBssScanInfo3:\n"}
      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= getStationConfig ====================================
    
    proc getStationConfig {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher
    
      compMsgWifiData.wifiOpMode = wifi_get_opmode{}
      result = wifi_station_get_config{&station_config}
      if {result != true} {
        checkErrOK $::COMP_DISP_ERR_GET_STATION_CONFIG
      }
      c_memset{stationConfig.ssid, 0, sizeof{stationConfig.ssid}}
      if {c_strlen{station_config.ssid} <= sizeof{stationConfig.ssid}} {
        c_memcpy{stationConfig.ssid, station_config.ssid, c_strlen{station_config.ssid}}
      } else {
        c_memcpy{stationConfig.ssid, station_config.ssid, sizeof{stationConfig.ssid}}
      }
      c_memset{stationConfig.password, 0, sizeof{stationConfig.password}}
      if {c_strlen{station_config.password} <= sizeof{stationConfig.password}} {
        c_memcpy{stationConfig.password, station_config.password, c_strlen{station_config.password}}
      } else {
        c_memcpy{stationConfig.password, station_config.password, sizeof{stationConfig.password}}
      }
      c_memset{stationConfig.bssidStr, 0, sizeof{stationConfig.bssidStr}}
      c_memcpy{stationConfig.bssid, station_config.bssid, sizeof{stationConfig.bssid}}
      c_sprintf{stationConfig.bssidStr,MACSTR, MAC2STR{station_config.bssid}}
      stationConfig.bssidSet = station_config.bssid_set
      stationConfig.status = wifi_station_get_connect_status{}
      stationConfig.mode = wifi_get_opmode{}
      stationConfig.channel = wifi_get_channel{}
    
    #  authmode
    #  freq_offset
    #  freqcal_val
    
      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= geScanInfoTableFieldValue ====================================
    
    proc getScanInfoTableFieldValue {compMsgDispatcherVar actionMode} {
      upvar $compMsgDispatcherVar compMsgDispatcher
      
    #ets_printf{"getModuleTableFieldValue: row: %d col: %d actionMode: %d\n", self->buildMsgInfos.tableRow, self->buildMsgInfos.tableCol, actionMode}
      scanInfos = self->bssScanInfos
      self->buildMsgInfos.numericValue = 0
      self->buildMsgInfos.stringValue = NULL
      if {self->buildMsgInfos.tableRow > scanInfos->numScanInfos} {
        checkErrOK $::COMP_DISP_ERR_BAD_ROW
      }
      scanInfo = &scanInfos->infos[self->buildMsgInfos.tableRow]
      result = bssStr2BssInfoId{self->msgDescPart->fieldNameStr, &fieldId}
    #ets_printf{"row: %d ssid: %s rssi: %d fieldName: %s fieldId: %d\n", self->buildMsgInfos.tableRow, scanInfo->ssid, scanInfo->rssi, self->buildMsgInfos.fieldNameStr, fieldId}
      checkErrOK{result}
      switch {{int}fieldId} {
      case  BSS_INFO_BSSID:
        break
      case  BSS_INFO_BSSID_STR:
        break
      case  BSS_INFO_SSID:
        self->buildMsgInfos.stringValue = scanInfo->ssid
        return COMP_DISP_ERR_OK
        break
      case  BSS_INFO_SSID_LEN:
        break
      case  BSS_INFO_CHANNEL:
        break
      case  BSS_INFO_RSSI:
        self->buildMsgInfos.numericValue = scanInfo->rssi
        return COMP_DISP_ERR_OK
        break
      case  BSS_INFO_AUTH_MODE:
        break
      case  BSS_INFO_IS_HIDDEN:
        break
      case  BSS_INFO_FREQ_OFFSET:
        break
      case  BSS_INFO_FREQ_CAL_VAL:
        break
      }
      checkErrOK $::COMP_DISP_ERR_ACTION_NAME_NOT_FOUND
    }
    
    # ================================= getWifiKeyValueInfo ====================================
    
    proc getWifiKeyValueInfo {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher
    
      result = bssStr2BssInfoId{self->msgDescPart->fieldNameStr + c_strlen{"#key_"}, &bssInfoType}
      checkErrOK $result
      switch {{int}bssInfoType} {
      case  BSS_INFO_BSSID:
        break
      case  BSS_INFO_BSSID_STR:
        break
      case  BSS_INFO_SSID:
        self->msgDescPart->fieldKey = compMsgWifiData.key_ssid
        self->msgDescPart->fieldSize = compMsgWifiData.bssScanSizes.ssidSize
        self->msgDescPart->fieldType = compMsgWifiData.bssScanTypes.ssidType
        return COMP_DISP_ERR_OK
        break
      case  BSS_INFO_SSID_LEN:
        break
      case  BSS_INFO_CHANNEL:
        break
      case  BSS_INFO_RSSI:
        self->msgDescPart->fieldKey = compMsgWifiData.key_rssi
        self->msgDescPart->fieldSize = compMsgWifiData.bssScanSizes.rssiSize
        self->msgDescPart->fieldType = compMsgWifiData.bssScanTypes.rssiType
        return COMP_DISP_ERR_OK
        break
      case  BSS_INFO_AUTH_MODE:
        break
      case  BSS_INFO_IS_HIDDEN:
        break
      case  BSS_INFO_FREQ_OFFSET:
        break
      case  BSS_INFO_FREQ_CAL_VAL:
        break
      }
      checkerrOK $::COMP_DISP_ERR_FIELD_NOT_FOUND
    }
    
    # ================================= getWifiKeyValue ====================================
    
    proc getWifiKeyValue {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher
    
      result = bssStr2BssInfoId{self->msgValPart->fieldNameStr + c_strlen{"#key_"}, &bssInfoType}
      checkErrOK $result
      saveData = self->compMsgDataView->dataView->data
      saveLgth = self->compMsgDataView->dataView->lgth
      switch {{int}bssInfoType} {
      case  BSS_INFO_BSSID:
        break
      case  BSS_INFO_BSSID_STR:
        break
      case  BSS_INFO_SSID:
        self->msgValPart->fieldKeyValueStr = os_zalloc{self->msgDescPart->fieldSize}
        checkAllocOK{self->msgValPart->fieldKeyValueStr}
        entryIdx = 0
        cp = self->msgValPart->fieldKeyValueStr
        self->compMsgDataView->dataView->data = cp
        self->compMsgDataView->dataView->lgth = 2 * sizeof{uint16_t} + sizeof{uint8_t}
        result = self->compMsgDataView->dataView->setUint16{self->compMsgDataView->dataView, 0, self->msgDescPart->fieldKey}
        checkErrOK{result}
        result = self->compMsgDataView->dataView->setUint8{self->compMsgDataView->dataView, 2, self->msgDescPart->fieldType}
        checkErrOK{result}
        result = self->compMsgDataView->dataView->setUint16{self->compMsgDataView->dataView, 3, self->msgDescPart->fieldSize - {2 * sizeof{uint16_t} + sizeof{uint8_t}}}
        checkErrOK{result}
        cp += 2 * sizeof{uint16_t} + sizeof{uint8_t}
        self->compMsgDataView->dataView->data = saveData
        self->compMsgDataView->dataView->lgth = saveLgth
        while {entryIdx < self->bssScanInfos->numScanInfos} {
          bssScanInfo = &self->bssScanInfos->infos[entryIdx]
          c_memcpy{cp, bssScanInfo->ssid, bssScanInfo->ssid_len}
          cp += bssScanInfo->ssid_len
          *cp++ = '\0'
          entryIdx++
        }
        return $::COMP_DISP_ERR_OK
        break
      case  BSS_INFO_SSID_LEN:
        break
      case  BSS_INFO_CHANNEL:
        break
      case  BSS_INFO_RSSI:
        self->msgValPart->fieldKeyValueStr = os_zalloc{self->msgDescPart->fieldSize}
        checkAllocOK{self->msgValPart->fieldKeyValueStr}
        cp = self->msgValPart->fieldKeyValueStr
        self->compMsgDataView->dataView->data = cp
        self->compMsgDataView->dataView->lgth = 2 * sizeof{uint16_t} + sizeof{uint8_t}
        result = self->compMsgDataView->dataView->setUint16{self->compMsgDataView->dataView, 0, self->msgDescPart->fieldKey}
        checkErrOK $result
        result = self->compMsgDataView->dataView->setUint8{self->compMsgDataView->dataView, 2, self->msgDescPart->fieldType}
        checkErrOK $result
        cp += 2 * sizeof{uint16_t} + sizeof{uint8_t}
        result = self->compMsgDataView->dataView->setUint16{self->compMsgDataView->dataView, 3, self->msgDescPart->fieldSize - {2 * sizeof{uint16_t} + sizeof{uint8_t}}}
        checkErrOK $result
        self->compMsgDataView->dataView->data = saveData
        self->compMsgDataView->dataView->lgth = saveLgth
        entryIdx = 0
        while {entryIdx < self->bssScanInfos->numScanInfos} {
          bssScanInfo = &self->bssScanInfos->infos[entryIdx]
          *cp++ = bssScanInfo->rssi
          entryIdx++
        }
        return $::COMP_DISP_ERR_OK
        break
      case  BSS_INFO_AUTH_MODE:
        break
      case  BSS_INFO_IS_HIDDEN:
        break
      case  BSS_INFO_FREQ_OFFSET:
        break
      case  BSS_INFO_FREQ_CAL_VAL:
        break
      }
      checkErrOK $::COMP_DISP_ERR_FIELD_NOT_FOUND
    }
    
    # ================================= getWifiValue ====================================
    
    proc getWifiValue {compMsgDispatcherVar which valueTypeId valueVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $valueVar value
    
      set value ""
      switch {which} {
      WIFI_INFO_PROVISIONING_SSID {
        set value [dict get $compMsgWifiData provisioningSsid]
        }
      WIFI_INFO_PROVISIONING_PORT {
        set value [dict get $compMsgWifiData provisioningPort]
        }
      WIFI_INFO_PROVISIONING_IP_ADDR {
        set value [dict get $compMsgWifiData provisioningIPAddr]
        }
      WIFI_INFO_BINARY_CALL_BACK {
        set value [dict get $compMsgWifiData websocketBinaryReceived]
        }
      case WIFI_INFO_TEXT_CALL_BACK {
        set [dict get $compMsgWifiData websocketTextReceived]
        }
      default {
        checkErrOK $::COMP_DISP_ERR_BAD_WIFI_VALUE_WHICH
        }
      }
      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= setWifiValues ====================================
    
    proc setWifiValues {compMsgDispatcherVar} {
      variable compMsgWifiData
      upvar $compMsgDispatcherVar compMsgDispatcher
    
      self->bssScanInfos = &bssScanInfos
    
      dict set compMsgWifiData provisioningSsid "testDevice_connect"
      dict set compMsgWifiData provisioningPort 80
      dict set compMsgWifiData provisioningIPAddr "192.168.4.1"
      set result [getStationConfig {compMsgWifiData}
      checkErrOK $result
      return $COMP_DISP_ERR_OK
    }
    
    # ================================= compMsgWifiInit ====================================
    
    proc compMsgWifiInit {compMsgDispatcherVar} {
      variable compMsgWifiData
      upvar $compMsgDispatcherVar compMsgDispatcher
    
      getWifiKeyValueKeys compMsgDispatcher compMsgWifiData
      set result [setWifiValues compMsgDispatcher]
      checkErrOK $result
      return $::COMP_DISP_ERR_OK;
    }

  } ; # namespace compMsgWifiData
} ; # namespace compMsg
