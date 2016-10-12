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

set ::STRUCT_DISP_ERR_OK                    0
set ::STRUCT_DISP_ERR_VALUE_NOT_SET         255
set ::STRUCT_DISP_ERR_VALUE_OUT_OF_RANGE    254
set ::STRUCT_DISP_ERR_BAD_VALUE             253
set ::STRUCT_DISP_ERR_BAD_FIELD_TYPE        252
set ::STRUCT_DISP_ERR_FIELD_TYPE_NOT_FOUND  251
set ::STRUCT_DISP_ERR_VALUE_TOO_BIG         250
set ::STRUCT_DISP_ERR_OUT_OF_MEMORY         249
set ::STRUCT_DISP_ERR_OUT_OF_RANGE          248
  # be carefull the values up to here
  # must correspond to the values in dataView.tcl !!!
  # with the names like DATA_VIEW_ERR_*

set ::STRUCT_DISP_ERR_FIELD_NOT_FOUND       230
set ::STRUCT_DISP_ERR_HANDLE_NOT_FOUND      227
  # be carefull the values up to here
  # must correspond to the values in structmsgDataView.tcl !!!
  # with the names like STRUCT_MSG_ERR_*

set ::STRUCT_DISP_ERR_OPEN_FILE             189
set ::STRUCT_DISP_FILE_NOT_OPENED           188
set ::STRUCT_DISP_ERR_FLUSH_FILE            187
set ::STRUCT_DISP_ERR_WRITE_FILE            186
set ::STRUCT_DISP_ERR_BAD_RECEIVED_LGTH     185
set ::STRUCT_DISP_ERR_BAD_FILE_CONTENTS     184
set ::STRUCT_DISP_ERR_HEADER_NOT_FOUND      183
set ::STRUCT_DISP_ERR_DUPLICATE_FIELD       182
set ::STRUCT_DISP_ERR_BAD_FIELD_NAME        181
set ::STRUCT_DISP_ERR_BAD_HANDLE_TYPE       180
set ::STRUCT_DISP_ERR_INVALID_BASE64_STRING 179
set ::STRUCT_DISP_ERR_TOO_FEW_FILE_LINES    178
set ::STRUCT_DISP_ERR_ACTION_NAME_NOT_FOUND 177
set ::STRUCT_DISP_ERR_DUPLICATE_ENTRY       176

if {0} {
#define BASE64_INVALID '\xff'
#define BASE64_PADDING '='
#define ISBASE64(c) (unbytes64[c] != BASE64_INVALID)

#define DISP_HANDLE_PREFIX "stmsgdisp_"

typedef struct handle2Dispatcher
{
  uint8_t *handle;
  structmsgDispatcher_t *structmsgDispatcher;
} handle2Dispatcher_t;

typedef struct structmsgDispatcherHandles
{
  handle2Dispatcher_t *handles;
  int numHandles;
} structmsgDispatcherHandles_t;

// create an object
static structmsgDispatcherHandles_t structmsgDispatcherHandles = { NULL, 0};

static int structmsgDispatcherId = 0;
// right now we only need one dispatcher!
static structmsgDispatcher_t *structmsgDispatcherSingleton = NULL;

static const uint8 b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
}


namespace eval structmsg {
  namespace ensemble create

    namespace export structmsgDispatcher

  namespace eval structmsgDispatcher {
    namespace ensemble create
      
    namespace export structmsgDispatcherInit freeStructmsgDataView createMsgFromLines
    namespace export createMsgFromLines
    variable structmsgDispatcher [list]
    variable structmsgData [list]
    variable numMsgHeaders 0
    variable maxMsgHeaders 4
    variable msgHeader2MsgPtrs [list]

    # ============================= toBase64 ========================
    
    proc toBase64 {msg len encoded} {
      size_t i;
      size_t n;
      uint8_t *q;
      uint8_t *out;
      uint8_t bytes64[sizeof{b64}];
    
      n = *len;
      if {!n} { # handle empty string case 
        return STRUCT_DISP_ERR_OUT_OF_MEMORY;
      }
      out = {uint8_t *}os_zalloc((n + 2) / 3 * 4);
      checkAllocOK{out};
      c_memcpy{bytes64, b64, sizeof(b64});   #Avoid lots of flash unaligned fetches
      
      for {i = 0, q = out; i < n; i += 3} {
        int a = msg[i];
        int b = {i + 1 < n} ? msg[i + 1] : 0;
        int c = {i + 2 < n} ? msg[i + 2] : 0;
        *q++ = bytes64[a >> 2];
        *q++ = bytes64[{(a & 3} << 4) | (b >> 4)];
        *q++ = {i + 1 < n} ? bytes64[((b & 15) << 2) | (c >> 6)] : BASE64_PADDING;
        *q++ = {i + 2 < n} ? bytes64[(c & 63)] : BASE64_PADDING;
      }
      *len = q - out;
      *encoded = out;
      return STRUCT_DISP_ERR_OK;
    }
    
    # ============================= fromBase64 ========================
    
    proc fromBase64 {encodedMsg len decodedMsg} {
      int i;
      int n;
      int blocks;
      int pad;
      const uint8 *p;
      uint8_t unbytes64[UCHAR_MAX+1];
      uint8_t *msg;
      uint8_t *q;
    
      n = *len;
      blocks = {n>>2};
      pad = 0;
      if {!n} { # handle empty string case 
        return STRUCT_DISP_ERR_OUT_OF_MEMORY;
      } 
      if {n & 3} {
        return STRUCT_DISP_ERR_INVALID_BASE64_STRING;
      } 
      c_memset{unbytes64, BASE64_INVALID, sizeof(unbytes64});
      for {i = 0; i < sizeof(b64}-1; i++) {
        unbytes64[b64[i]] = i;  # sequential so no exceptions 
      }
      if {encodedMsg[n-1] == BASE64_PADDING} {
        pad =  {encodedMsg[n-2] != BASE64_PADDING} ? 1 : 2;
        blocks--;  #exclude padding block
      }    
    
      for {i = 0; i < n - pad; i++} {
        if {!ISBASE64(encodedMsg[i]}) {
          return STRUCT_DISP_ERR_INVALID_BASE64_STRING;
        }
      }
      unbytes64[BASE64_PADDING] = 0;
      q = {uint8_t *} os_zalloc(1+ (3 * n / 4)); 
      checkAllocOK{q};
      msg = q;
      for {i = 0, p = encodedMsg; i<blocks; i++} {
        uint8 a = unbytes64[*p++]; 
        uint8 b = unbytes64[*p++]; 
        uint8 c = unbytes64[*p++]; 
        uint8 d = unbytes64[*p++];
        *q++ = {a << 2} | (b >> 4);
        *q++ = {b << 4} | (c >> 2);
        *q++ = {c << 6} | d;
      }
      if {pad} { #now process padding block bytes
        uint8 a = unbytes64[*p++];
        uint8 b = unbytes64[*p++];
        *q++ = {a << 2} | (b >> 4);
        if {pad == 1} *q++ = (b << 4) | (unbytes64[*p] >> 2);
      }
      *len = q - msg;
      *decodedMsg = msg;
      return STRUCT_DISP_ERR_OK;;
    }
    
    # ============================= addHandle ========================
    
    proc addHandle {handle} {
      variable structmsgDispatcher

      int idx;
    
      if {structmsgDispatcherHandles.handles == NULL} {
        structmsgDispatcherHandles.handles = os_zalloc{sizeof(handle2Dispatcher_t});
        if {structmsgDispatcherHandles.handles == NULL} {
          return STRUCT_DISP_ERR_OUT_OF_MEMORY;
        } else {
          structmsgDispatcherHandles.handles[structmsgDispatcherHandles.numHandles].handle = handle;
          structmsgDispatcherHandles.handles[structmsgDispatcherHandles.numHandles].structmsgDispatcher = structmsgDispatcher;
          structmsgDispatcherHandles.numHandles++;
          return STRUCT_DISP_ERR_OK;
        }
      } else {
        # check for unused slot first
        idx = 0;
        while {idx < structmsgDispatcherHandles.numHandles} {
          if {structmsgDispatcherHandles.handles[idx].handle == NULL} {
            structmsgDispatcherHandles.handles[idx].handle = handle;
            structmsgDispatcherHandles.handles[idx].structmsgDispatcher = structmsgDispatcher;
            return STRUCT_DISP_ERR_OK;
          }
          idx++;
        }
        structmsgDispatcherHandles.handles = os_realloc{structmsgDispatcherHandles.handles, sizeof(handle2Dispatcher_t}*(structmsgDispatcherHandles.numHandles+1));
        checkAllocOK{structmsgDispatcherHandles.handles};
        structmsgDispatcherHandles.handles[structmsgDispatcherHandles.numHandles].handle = handle;
        structmsgDispatcherHandles.handles[idx].structmsgDispatcher = structmsgDispatcher;
        structmsgDispatcherHandles.numHandles++;
      }
      return STRUCT_DISP_ERR_OK;
    }
    
    # ============================= deleteHandle ========================
    
    proc deleteHandle {handle} {
      variable structmsgDispatcher

      int idx;
      int numUsed;
      int found;
    
      if {structmsgDispatcherHandles.handles == NULL} {
        return STRUCT_DISP_ERR_HANDLE_NOT_FOUND;
      }
      found = 0;
      idx = 0;
      numUsed = 0;
      while {idx < structmsgDispatcherHandles.numHandles} {
        if {(structmsgDispatcherHandles.handles[idx].handle != NULL} && (c_strcmp(structmsgDispatcherHandles.handles[idx].handle, handle) == 0)) {
          structmsgDispatcherHandles.handles[idx].handle = NULL;
          found++;
        } else {
          if {structmsgDispatcherHandles.handles[idx].handle != NULL} {
            numUsed++;
          }
        }
        idx++;
      }
      if {numUsed == 0} {
        os_free{structmsgDispatcherHandles.handles};
        structmsgDispatcherHandles.handles = NULL;
      }
      if {found} {
          return STRUCT_DISP_ERR_OK;
      }
      return STRUCT_DISP_ERR_HANDLE_NOT_FOUND;
    }
    
    # ============================= checkHandle ========================
    
    proc checkHandle {handle} {
      variable structmsgDispatcher
      int idx;
    
      if {structmsgDispatcherHandles.handles == NULL} {
        return STRUCT_DISP_ERR_HANDLE_NOT_FOUND;
      }
      idx = 0;
      while {idx < structmsgDispatcherHandles.numHandles} {
        if {(structmsgDispatcherHandles.handles[idx].handle != NULL} && (c_strcmp(structmsgDispatcherHandles.handles[idx].handle, handle) == 0)) {
          *structmsgDispatcher = structmsgDispatcherHandles.handles[idx].structmsgDispatcher;
          return STRUCT_DISP_ERR_OK;
        }
        idx++;
      }
      return STRUCT_DISP_ERR_HANDLE_NOT_FOUND;
    }
    
    # ================================= getMsgPtrFromMsgParts ====================================
    
    proc getMsgPtrFromMsgParts {msgParts incrRefCnt} {
      variable structmsgDispatcher
      variable structmsgData
      variable numMsgHeaders
      variable maxMsgHeaders
      variable msgHeader2MsgPtrs

      # build header from msgParts
      set offset 0
      set result [::structmsg dataView getData saveData saveLgth]
      set lgth [dict get $msgParts totalLgth]
      ::structmsg dataView setData "" $lgth
      set result [::structmsg dataView setUint16 $offset [dict get $msgParts fromPart]]
      if {$result != $::STRUCT_MSG_ERR_OK} {
        return $result
      }
      incr offset 2
      set result [::structmsg dataView setUint16 $offset [dict get $msgParts toPart]]
      if {$result != $::STRUCT_MSG_ERR_OK} {
        return $result
      }
      incr offset 2
      set result [::structmsg dataView setUint16 $offset [dict get $msgParts totalLgth]]
      if {$result != $::STRUCT_MSG_ERR_OK} {
        return $result
      }
      incr offset 2
      # FIXME dispFlags !!
      if {[lsearch $::structmsg::structmsgIdentify::dispFlags STRUCT_MSG_U8_CMD_KEY] >= 0} {
        set result [::structmsg dataView setUint8 $offset [dict get $msgParts u8CmdKey]]
        if {$result != $::STRUCT_MSG_ERR_OK} {
          return $result
        }
        incr offset 1
      } else {
        set result [::structmsg dataView setUint16 $offset [dict get $msgParts u16CmdKey]]
        if {$result != $::STRUCT_MSG_ERR_OK} {
          return $result
        }
        incr offset 2
      }
      set result [::structmsg dataView getData header headerLgth]
      if {$result != $::STRUCT_MSG_ERR_OK} {
        return $result
      }
      set result [::structmsg dataView setData $saveData $saveLgth]
      if {$result != $::STRUCT_MSG_ERR_OK} {
        return $result
      }
      # end build header from msgParts
      
puts stderr "DATA!$header!lgth!$headerLgth!"
      set firstFreeEntry [list]
      set firstFreeEntryId 0
      if {$numMsgHeaders > 0} {
        # find header 
        set headerIdx 0
        while {$headerIdx < $numMsgHeaders} {
          set headerEntry [lindex msgHeader2MsgPtrs $headerIdx]
          if {([dict get $headerEntry structmsgData] ne [list]) && ([string range [dict get $headerEntry header] 0 [expr {$offset - 1}]] eq $header)} {
            if {$incrRefCnt < 0} {
              dict set headerEntry headerLgth 0
              dict set headerEntry structmsgData [list]
              set msgHeader2MsgPtrs [lreplace $msgHeader2MsgPtrs $headerIdx $headerIdx $headerEntry]
              return $::STRUCT_DISP_ERR_OK
            }
            set structmsgData [dict get $headerEntry structmsgData]
# FIXME!!            dict set structmsgData flags [dict delete [dict get $structmsgData flags] STRUCT_MSG_IS_PREPARED]
            return $::STRUCT_DISP_ERR_OK
          }
          if {($incrRefCnt == $::STRUCT_MSG_INCR} && ([dict get $headerEntry structmsgData] eq [list]) && ($firstFreeEntry eq [list])) {
            set firstFreeEntry $headerEntry
            dict set firstFreeEntry structmsgData [dict create]
            dict set firstFreeEntry headerLgth $offset
            dict set firstFreeEntry header [string range $header 0 [expr {$offset - 1}]]
            set structmsgData [dict get $firstFreeEntry structmsgData]
            set msgHeader2MsgPtrs [lappend msgHeader2MsgPtrs $firstFreeEntry]
          }
          incr headerIdx
        }
      }
      if {$incrRefCnt < 0} {
        return $::STRUCT_DISP_ERR_OK; # just ignore silently
      } else {
        if {$incrRefCnt == 0} {
          return $::STRUCT_DISP_ERR_HEADER_NOT_FOUND;
        } else {
          if {$firstFreeEntry ne [list]} {
            set structmsgData [dict get $firstFreeEntry structmsgData]
            dict set firstFreeEntry headerLgth $offset
            dict set firstFreeEntry header [string range $header 0 [expr {$offset - 1}]]
          } else {
            set newHeaderEntry [dict create]
            dict set newHeaderEntry headerLgth $offset
            dict set newHeaderEntry header [string range $header 0 [expr {$offset - 1}]]
            set structmsg [dict create]
            dict set structmsg flags [list]
            dict set newHeaderEntry structmsgData $structmsg
            set structmsgData [dict get $newHeaderEntry structmsgData]
            incr numMsgHeaders
          }
        }
      }
      return $::STRUCT_DISP_ERR_OK;
    }
    
    # ================================= getFieldType ====================================
    
    proc getFieldType {fieldNameId fieldTypeIdVar} {
      variable structmsgDispatcher
      variable structmsgData

      int idx;
      structmsgField_t *fieldInfo;
    
      idx = 0;
      while {idx < structmsgData->numFields} {
        fieldInfo = &structmsgData->fields[idx];
        if {fieldInfo->fieldNameId == fieldNameId} {
          *fieldTypeId = fieldInfo->fieldTypeId;
          return STRUCT_DISP_ERR_OK;
        }
        idx++;
      }
      # and now check the table fields
      idx = 0;
      while {idx < structmsgData->numTableRowFields} {
        fieldInfo = &structmsgData->tableFields[idx];
        if {fieldInfo->fieldNameId == fieldNameId} {
          *fieldTypeId = fieldInfo->fieldTypeId;
          return STRUCT_DISP_ERR_OK;
        }
        idx++;
      }
      return STRUCT_DISP_ERR_FIELD_NOT_FOUND;
    }
    
    # ================================= setMsgValuesFromLines ====================================
    
    proc setMsgValuesFromLines {numEntries handle type} {
      variable structmsgDispatcher
      variable structmsgData
      int idx;
      uint8_t*cp;
      uint8_t *fieldNameStr;
      uint8_t fieldNameId;
      uint8_t fieldTypeId;
      uint8_t *fieldValueStr;
      char *endPtr;
      uint8_t fieldLgth;
      uint8_t *flagStr;
      uint8_t flag;
      uint8_t lgth;
      unsigned long uval;
      uint8_t buf[100];
      uint8_t *buffer = buf;
      int numericValue;
      uint8_t *stringValue;
      structmsgDataView_t *dataView;
      int result;
    
      idx = 0;
      dataView = structmsgData->structmsgDataView;
      while{idx < numEntries} {
        result = self->readLine{self, &buffer, &lgth};
        checkErrOK{result};
        if {lgth == 0} {
          return STRUCT_DISP_ERR_TOO_FEW_FILE_LINES;
        }
        buffer[lgth] = 0;
        fieldNameStr = buffer;
    
        # fieldName
        cp = fieldNameStr;
        while {*cp != ','} {
          cp++;
        }
        *cp++ = '\0';
        result = dataView->getFieldNameIdFromStr{dataView, fieldNameStr, &fieldNameId, STRUCT_MSG_NO_INCR};
        checkErrOK{result};
        result = getFieldType{self, structmsgData, fieldNameId, &fieldTypeId};
        checkErrOK{result};
    
        # fieldValue
        fieldValueStr = cp;
        while {*cp != '\n'} {
          cp++;
        }
        *cp++ = '\0';
        if {fieldValueStr[0] == '@'} {
          # call the callback function vor the field!!
          result = self->fillMsgValue{self, fieldValueStr, &numericValue, &stringValue, type, fieldTypeId};
          checkErrOK{result};
        } else {
          switch {fieldTypeId} {
          case DATA_VIEW_FIELD_UINT8_T:
          case DATA_VIEW_FIELD_INT8_T:
          case DATA_VIEW_FIELD_UINT16_T:
          case DATA_VIEW_FIELD_INT16_T:
          case DATA_VIEW_FIELD_UINT32_T:
          case DATA_VIEW_FIELD_INT32_T:
            {
              uval = c_strtoul{fieldValueStr, &endPtr, 10};
              if {endPtr == (char *}(cp-1)) {
                numericValue = {int}uval;
                stringValue = NULL;
              } else {
                numericValue = 0;
                stringValue = fieldValueStr;
              }
            }
            break;
          default:
            numericValue = 0;
            stringValue = fieldValueStr;
            break;
          }
        }
        switch {fieldNameId} {
          case STRUCT_MSG_SPEC_FIELD_DST:
            numericValue = self->received.fromPart;
            stringValue = NULL;
            result = structmsgData->setFieldValue{structmsgData, fieldNameStr, numericValue, stringValue};
            break;
          case STRUCT_MSG_SPEC_FIELD_SRC:
            numericValue = self->received.toPart;
            stringValue = NULL;
            result = structmsgData->setFieldValue{structmsgData, fieldNameStr, numericValue, stringValue};
            break;
          case STRUCT_MSG_SPEC_FIELD_CMD_KEY:
            # check for u8CmdKey/u16CmdKey here
            if {self->dispFlags & STRUCT_MSG_U8_CMD_KEY} {
              numericValue = self->received.u8CmdKey;
            } else {
              numericValue = self->received.u16CmdKey;
            }
            stringValue = NULL;
            result = structmsgData->setFieldValue{structmsgData, fieldNameStr, numericValue, stringValue};
            break;
          default:
            result = structmsgData->setFieldValue{structmsgData, fieldNameStr, numericValue, stringValue};
            break;
        }
        checkErrOK{result};
        idx++;
      }
      result = structmsgData->setFieldValue{structmsgData, "@cmdKey", type, NULL};
      checkErrOK{result};
      structmsgData->prepareMsg{structmsgData};
    #  structmsgData->dumpMsg{structmsgData};
      return STRUCT_DISP_ERR_OK;
    }
    
    # ================================= createMsgFromLines ====================================
    
    proc createMsgFromLines {fd parts numEntries numRows type handleVar} {
      variable structmsgDispatcher
      variable structmsgData
      variable numMsgHeaders
      variable maxMsgHeaders
      upvar $handleVar handle

    #ets_printf{"§createMsgFromLines:%d!%d! \n§" $numMsgHeaders $maxMsgHeaders};
      set result [getMsgPtrFromMsgParts $parts $::STRUCT_MSG_INCR]
puts stderr "res1!$result!"
      if {$result != $::STRUCT_MSG_ERR_OK} {
        return $result
      }
      if {[lsearch [dict get $structmsgData flags] STRUCT_MSG_IS_INITTED] >= 0} {
        return $::STRUCT_DISP_ERR_OK
      }
      set result [::structmsg structmsgData createMsg $numEntries handle]
      if {$result != $::STRUCT_MSG_ERR_OK} {
        return $result
      }
      set idx 0
      while {$idx < $numEntries} {
        gets $fd line
        if {$line eq ""} {
          return $::STRUCT_DISP_ERR_TOO_FEW_FILE_LINES
        }
        set flds [split $line ","]
        foreach {fieldNameStr fieldTypeStr fieldLgthStr} $flds break
        if {$fieldLgthStr eq "@numRows"} {
          set fieldLgth $numRows
        } else {
          set fieldLgth $fieldLgthStr
        }
        set result [::structmsg structmsgData addField $fieldNameStr $fieldTypeStr $fieldLgth]
        if {$result != $::STRUCT_MSG_ERR_OK} {
          return $result
        }
        incr idx
      }
      ::structmsg structmsgData initMsg
      return $::STRUCT_DISP_ERR_OK;
    }
    
    # ================================= resetMsgInfo ====================================
    
    proc resetMsgInfo {partsVar} {
      variable structmsgDispatcher
      upvar $partsVar parts

      parts->lgth = 0;
      parts->fieldOffset = 0;
      parts->fromPart = 0;
      parts->toPart = 0;
      parts->totalLgth = 0;
      parts->u16CmdLgth = 0;
      parts->u16CmdKey = 0;
      parts->u8CmdKey = 0;
      parts->u8CmdLgth = 0;
      self->structmsgDataView->dataView->data = parts->buf;
      self->structmsgDataView->dataView->lgth = 0;
      return STRUCT_DISP_ERR_OK;
    }
    
    # ============================= encryptMsg ========================
    
    proc encryptMsg {msg mlen key klen iv ivlen bufVar lgthVar} {
      upvar $bufVar buf
      upvar $lgthVar lgth

      const crypto_mech_t *mech;
      size_t bs;
      size_t clen;
      uint8_t *crypted;
    
      *buf = NULL;
      *lgth = 0;
      mech = crypto_encryption_mech {"AES-CBC"};
      if {mech == NULL} {
        return STRUCT_MSG_ERR_CRYPTO_BAD_MECHANISM;
      }
      bs = mech->block_size;
      clen = {(mlen + bs - 1} / bs) * bs;
      *lgth = clen;
    #ets_printf{"dlen: %d lgth: %d clen: %d data: %p\n", dlen, *lgth, clen, data};
      crypted = {uint8_t *}os_zalloc (clen);
      if {!crypted} {
        return STRUCT_MSG_ERR_CRYPTO_INIT_FAILED;
      } 
      *buf = crypted;
      crypto_op_t op =
      { 
        key, klen,
        iv, ivlen,
        msg, mlen,
        crypted, clen,
        OP_ENCRYPT
      }; 
      if {!mech->run (&op}) { 
        os_free {*buf};
        return STRUCT_MSG_ERR_CRYPTO_INIT_FAILED;
      } 
      return STRUCT_MSG_ERR_OK;
    }
    
    # ============================= decryptMsg ========================
    
    proc decryptMsg {msg mlen key klen iv ivlen bufVar lgthVar} {
      upvar $bufVar buf
      upvar $lgthVar lgth

      const crypto_mech_t *mech;
      size_t bs;
      size_t clen;
      uint8_t *crypted;
    
      *buf = NULL;
      *lgth = 0;
      mech = crypto_encryption_mech {"AES-CBC"};
      if {mech == NULL} {
        return STRUCT_MSG_ERR_CRYPTO_BAD_MECHANISM;
      }
      bs = mech->block_size;
      clen = {(mlen + bs - 1} / bs) * bs;
      *lgth = clen;
      crypted = {uint8_t *}os_zalloc (*lgth);
      if {!crypted} {
        return STRUCT_MSG_ERR_CRYPTO_INIT_FAILED;
      } 
      *buf = crypted;
      crypto_op_t op =
      { 
        key, klen,
        iv, ivlen,
        msg, mlen,
        crypted, clen,
        OP_DECRYPT
      }; 
      if {!mech->run (&op}) { 
        os_free {*buf};
        return STRUCT_MSG_ERR_CRYPTO_INIT_FAILED;
      }
      return STRUCT_MSG_ERR_OK;
    }
    
    # ============================= structmsgDispatcherGetPtrFromHandle ========================
    
    proc structmsgDispatcherGetPtrFromHandle {handle} {
      variablle structmsgDispatcher
    
      if {checkHandle(handle, structmsgDispatcher} != STRUCT_DISP_ERR_OK) {
        return STRUCT_DISP_ERR_HANDLE_NOT_FOUND;
      }
      return STRUCT_DISP_ERR_OK;
    }
    
    # ================================= initDispatcher ====================================
    
    proc initDispatcher {} {
      variable structmsgDispatcher

if {0} {
    ets_printf{"§initDispatcher!%p!§", self};
      result = structmsgIdentifyInit{self};
      checkErrOK{result};
      result = structmsgSendReceiveInit{self};
      checkErrOK{result};
      result = structmsgActionInit{self};
      checkErrOK{result};
      result = structmsgModuleDataValuesInit{self};
      checkErrOK{result};
      result = structmsgWebsocketInit{self};
      checkErrOK{result};
}
      return STRUCT_DISP_ERR_OK;
    }
    
    # ================================= createDispatcher ====================================
    
    proc createDispatcher {handleVar} {
      upvar $handleVar handle
    
      os_sprintf{self->handle, "%s%p", DISP_HANDLE_PREFIX, self};
      result = addHandle{self->handle, self};
      if {result != STRUCT_DISP_ERR_OK} {
        deleteHandle{self->handle};
        os_free{self};
        return result;
      }
      resetMsgInfo{self, &self->received};
      resetMsgInfo{self, &self->toSend};
      *handle = self->handle;
      return STRUCT_DISP_ERR_OK;
    }
    
    # ================================= newStructmsgDispatcher ====================================
    
     proc newStructmsgDispatcher {} {
      variable structmsgDispatcher

if {0} {
      if {structmsgDispatcherSingleton != NULL} {
        return structmsgDispatcherSingleton;
      }
      structmsgDispatcher_t *structmsgDispatcher = os_zalloc{sizeof(structmsgDispatcher_t});
      if {structmsgDispatcher == NULL} {
        return NULL;
      }
      structmsgDispatcher->structmsgDataView = newStructmsgDataView{};
      if {structmsgDispatcher->structmsgDataView == NULL} {
        return NULL;
      }
    
      structmsgDispatcherId++;
      structmsgDispatcher->id = structmsgDispatcherId;
    
      structmsgDispatcher->numMsgHeaders = 0;
      structmsgDispatcher->maxMsgHeaders = 0;
      structmsgDispatcher->msgHeader2MsgPtrs = NULL;
    
      structmsgDispatcher->msgHeaderInfos.headerFlags = 0;
      structmsgDispatcher->msgHeaderInfos.headerParts = NULL;
      structmsgDispatcher->msgHeaderInfos.numHeaderParts = 0;
      structmsgDispatcher->msgHeaderInfos.maxHeaderParts = 0;
    
      structmsgDispatcher->structmsgDataDescription = newStructmsgDataDescription{};
    
      structmsgDispatcher->createDispatcher = &createDispatcher;
      structmsgDispatcher->initDispatcher = &initDispatcher;
    
      structmsgDispatcher->BMsg = &BMsg;
      structmsgDispatcher->IMsg = &IMsg;
      structmsgDispatcher->MMsg = &MMsg;
      structmsgDispatcher->defaultMsg = &defaultMsg;
      structmsgDispatcher->resetMsgInfo = &resetMsgInfo;
      structmsgDispatcher->createMsgFromLines = &createMsgFromLines;
      structmsgDispatcher->setMsgValuesFromLines = &setMsgValuesFromLines;
    
      structmsgDispatcher->openFile = &openFile;
      structmsgDispatcher->closeFile = &closeFile;
      structmsgDispatcher->readLine = &readLine;
      structmsgDispatcher->writeLine = &writeLine;
    
      structmsgDispatcher->encryptMsg = &encryptMsg;
      structmsgDispatcher->decryptMsg = &decryptMsg;
      structmsgDispatcher->toBase64 = &toBase64;
      structmsgDispatcher->fromBase64 = &fromBase64;
    
      structmsgDispatcherSingleton = structmsgDispatcher;
}
      return $::STRUCT_MSG_ERR_OK
    }
    
    # ================================= freeStructmsgDispatcher ====================================
    
    proc freeStructmsgDispatcher {} {
      variable structmsgDispatcher
    }

  } ; # namespace structmsgDispatcher
} ; # namespace structmsg
