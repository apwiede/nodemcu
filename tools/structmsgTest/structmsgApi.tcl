# ===========================================================================
# * Copyright  {c}  2016  Arnulf P. Wiedemann (arnulf@wiedemann-pri.de)
# * All rights reserved.
# *
# * License: BSD/MIT
# *
# * Redistribution and use in source and binary forms  with or without
# * modification  are permitted provided that the following conditions
# * are met:
# *
# * 1. Redistributions of source code must retain the above copyright
# * notice  this list of conditions and the following disclaimer.
# * 2. Redistributions in binary form must reproduce the above copyright
# * notice  this list of conditions and the following disclaimer in the
# * documentation and/or other materials provided with the distribution.
# * 3. Neither the name of the copyright holder nor the names of its
# * contributors may be used to endorse or promote products derived
# * from this software without specific prior written permission.
# *
# * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# * AND ANY EXPRESS OR IMPLIED WARRANTIES  INCLUDING  BUT NOT LIMITED TO  THE
# * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# * LIABLE FOR ANY DIRECT  INDIRECT  INCIDENTAL  SPECIAL  EXEMPLARY  OR
# * CONSEQUENTIAL DAMAGES  {INCLUDING  BUT NOT LIMITED TO  PROCUREMENT OF
# * SUBSTITUTE GOODS OR SERVICES LOSS OF USE  DATA  OR PROFITS; OR BUSINESS
# * INTERRUPTION}  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY  WHETHER IN
# * CONTRACT  STRICT LIABILITY  OR TORT  {INCLUDING NEGLIGENCE OR OTHERWISE}
# * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE  EVEN IF ADVISED OF THE
# * POSSIBILITY OF SUCH DAMAGE.
# *
# ==========================================================================

# ============================= dictionary layouts ========================
# 
# ::structmsg(prefix)
# ::structmsg(numFieldNameIds)
# ::structmsg(numHandles)
# ::structmsg(numStructmsgDefinitions)
# ::structmsg(handles)
# 
# ::structmsg(specialFieldIds)
# ::structmsg(specialFieldNames)
# ::structmsg(structmsgErrId2Str)
# ::structmsg(fieldTypeDefines)
# ::structmsg(fieldTypeIds)
# 
# ::structmsg(fieldNameDefinitions)
#    numDefinitions
#    definitions [list 
#      refCnt
#      id
#      fieldName
#    ]
# 
# ::structmsg(structmsgDefinitions)
#    numDefinitions
#    definitions [list 
#      name
#      numFields
#      maxFields
#      fieldInfos [list
#        fieldId
#        fieldType
#        fieldLgth
#      ]
#    ]
# 
# 
# 
# 
# 
# ::structmsg(<handle>)
#  hdr
#    headerLgth
#    hdrInfo
#      hdrKeys
#        src
#        dst
#        cmdKey
#        cmdLgth
#        totalLgth
#  msg
#    numFieldInfos
#    maxFieldInfos
#    numTableRows
#    numTableRowFields
#    numRowFields
#    fieldInfos [list
#    ]
#    tableFieldInfos [list
#    ]
#    flags [list
#    ]
#  sequenceNum
#  encoded
#  todecode
#  encrypted
# 
# 
# 
# 
# 
# 
# 
# 
# 
# 
# 
# 
# 

# ============================= sizeof ========================

proc sizeof {type} {
  switch $type {
    int8_t -
    uint8_t {
      return 1
    }
    int16_t -
    uint16_t {
      return 2
    }
    int32_t -
    uint32_t {
      return 4
    }
  }
  error "bad type in sizeof: $type"
}

set RAND_MAX 0x7FFFFFFF

set STRUCT_MSG_FIELD_UINT8_T       1
set STRUCT_MSG_FIELD_INT8_T        2
set STRUCT_MSG_FIELD_UINT16_T      3
set STRUCT_MSG_FIELD_INT16_T       4
set STRUCT_MSG_FIELD_UINT32_T      5
set STRUCT_MSG_FIELD_INT32_T       6
set STRUCT_MSG_FIELD_UINT8_VECTOR  7
set STRUCT_MSG_FIELD_INT8_VECTOR   8
set STRUCT_MSG_FIELD_UINT16_VECTOR 9
set STRUCT_MSG_FIELD_INT16_VECTOR  10
set STRUCT_MSG_FIELD_UINT32_VECTOR 11
set STRUCT_MSG_FIELD_INT32_VECTOR  12

set STRUCT_MSG_SPEC_FIELD_SRC              255
set STRUCT_MSG_SPEC_FIELD_DST              254
set STRUCT_MSG_SPEC_FIELD_TARGET_CMD       253
set STRUCT_MSG_SPEC_FIELD_TOTAL_LGTH       252
set STRUCT_MSG_SPEC_FIELD_CMD_KEY          251
set STRUCT_MSG_SPEC_FIELD_CMD_LGTH         250
set STRUCT_MSG_SPEC_FIELD_RANDOM_NUM       249
set STRUCT_MSG_SPEC_FIELD_SEQUENCE_NUM     248
set STRUCT_MSG_SPEC_FIELD_FILLER           247
set STRUCT_MSG_SPEC_FIELD_CRC              246
set STRUCT_MSG_SPEC_FIELD_ID               245
set STRUCT_MSG_SPEC_FIELD_ID               245
set STRUCT_MSG_SPEC_FIELD_TABLE_ROWS       244
set STRUCT_MSG_SPEC_FIELD_TABLE_ROW_FIELDS 243
set STRUCT_MSG_SPEC_FIELD_LOW              242  ; # this must be the last entry!!

set STRUCT_MSG_ERR_OK                    0
set STRUCT_MSG_ERR_VALUE_NOT_SET         255
set STRUCT_MSG_ERR_VALUE_OUT_OF_RANGE    254
set STRUCT_MSG_ERR_BAD_VALUE             253
set STRUCT_MSG_ERR_BAD_FIELD_TYPE        252
set STRUCT_MSG_ERR_FIELD_NOT_FOUND       251
set STRUCT_MSG_ERR_VALUE_TOO_BIG         250
set STRUCT_MSG_ERR_BAD_SPECIAL_FIELD     249
set STRUCT_MSG_ERR_BAD_HANDLE            248
set STRUCT_MSG_ERR_OUT_OF_MEMORY         247
set STRUCT_MSG_ERR_HANDLE_NOT_FOUND      246
set STRUCT_MSG_ERR_NOT_ENCODED           245
set STRUCT_MSG_ERR_ENCODE_ERROR          244
set STRUCT_MSG_ERR_DECODE_ERROR          243
set STRUCT_MSG_ERR_BAD_CRC_VALUE         242
set STRUCT_MSG_ERR_CRYPTO_INIT_FAILED    241
set STRUCT_MSG_ERR_CRYPTO_OP_FAILED      240
set STRUCT_MSG_ERR_CRYPTO_BAD_MECHANISM  239
set STRUCT_MSG_ERR_NOT_ENCRYPTED         238
set STRUCT_MSG_ERR_DEFINITION_NOT_FOUND  237
set STRUCT_MSG_ERR_DEFINITION_TOO_MANY_FIELDS 236
set STRUCT_MSG_ERR_BAD_TABLE_ROW         235
set STRUCT_MSG_ERR_TOO_MANY_FIELDS       234
set STRUCT_MSG_ERR_BAD_DEFINTION_CMD_KEY 233
set STRUCT_MSG_ERR_NO_SLOT_FOUND         232

set ::structmsg(structmsgErrId2Str) [dict create]
dict set ::structmsg(structmsgErrId2Str) 0   ERR_OK
dict set ::structmsg(structmsgErrId2Str) 255 ERR_VALUE_NOT_SET
dict set ::structmsg(structmsgErrId2Str) 254 ERR_VALUE_OUT_OF_RANGE
dict set ::structmsg(structmsgErrId2Str) 253 ERR_BAD_VALUE
dict set ::structmsg(structmsgErrId2Str) 252 ERR_BAD_FIELD_TYPE
dict set ::structmsg(structmsgErrId2Str) 251 ERR_FIELD_NOT_FOUND
dict set ::structmsg(structmsgErrId2Str) 250 ERR_VALUE_TOO_BIG
dict set ::structmsg(structmsgErrId2Str) 249 ERR_BAD_SPECIAL_FIELD
dict set ::structmsg(structmsgErrId2Str) 248 ERR_BAD_HANDLE
dict set ::structmsg(structmsgErrId2Str) 247 ERR_OUT_OF_MEMORY
dict set ::structmsg(structmsgErrId2Str) 246 ERR_HANDLE_NOT_FOUND
dict set ::structmsg(structmsgErrId2Str) 245 ERR_NOT_ENCODED
dict set ::structmsg(structmsgErrId2Str) 244 ERR_ENCODE_ERROR
dict set ::structmsg(structmsgErrId2Str) 243 ERR_DECODE_ERROR
dict set ::structmsg(structmsgErrId2Str) 242 ERR_BAD_CRC_VALUE
dict set ::structmsg(structmsgErrId2Str) 241 ERR_CRYPTO_INIT_FAILED
dict set ::structmsg(structmsgErrId2Str) 240 ERR_CRYPTO_OP_FAILED
dict set ::structmsg(structmsgErrId2Str) 239 ERR_CRYPTO_BAD_MECHANISM
dict set ::structmsg(structmsgErrId2Str) 238 ERR_NOT_ENCRYPTED
dict set ::structmsg(structmsgErrId2Str) 237 ERR_DEFINITION_NOT_FOUND
dict set ::structmsg(structmsgErrId2Str) 236 ERR_DEFINITION_TOO_MANY_FIELDS
dict set ::structmsg(structmsgErrId2Str) 235 ERR_BAD_TABLE_ROW
dict set ::structmsg(structmsgErrId2Str) 234 ERR_TOO_MANY_FIELDS
dict set ::structmsg(structmsgErrId2Str) 233 ERR_BAD_DEFINTION_CMD_KEY
dict set ::structmsg(structmsgErrId2Str) 232 ERR_NO_SLOT_FOUND

set STRUCT_MSG_FIELD_SRC                 1
set STRUCT_MSG_FIELD_DST                 2
set STRUCT_MSG_FIELD_TOTAL_LGTH          3
set STRUCT_MSG_FIELD_CMD_KEY             4
set STRUCT_MSG_FIELD_CMD_LGTH            5

# header length: uint16_t src + uint16_t dst + uint16_t totalLgth
set STRUCT_MSG_HEADER_LENGTH [expr {[sizeof uint16_t] * 3}]
# cmd header length uint16_t cmdKey + unit16_t cmdLgth
set STRUCT_MSG_CMD_HEADER_LENGTH [expr {[sizeof uint16_t] * 2}]
set STRUCT_MSG_TOTAL_HEADER_LENGTH [expr {$::STRUCT_MSG_HEADER_LENGTH + $::STRUCT_MSG_CMD_HEADER_LENGTH}]
set STRUCT_MSG_NUM_HEADER_FIELDS 3
set STRUCT_MSG_NUM_CMD_HEADER_FIELDS 2
set STRUCT_MSG_DEFINITION_CMD_KEY 0xFFFF
set STRUCT_MSG_FREE_FIELD_ID 0xFF

set STRUCT_MSG_NO_INCR 0
set STRUCT_MSG_INCR    1
set STRUCT_MSG_DECR    -1

set ::structmsg(prefix) "stmsg_"
set ::structmsg(numHandles) 1
set ::structmsg(handles) [list]


namespace eval structmsg {
  namespace ensemble create

  namespace export create delete encode get_encoded decode dump
  namespace export encrypt decrypt add_field set_fillerAndCrc set_fieldValue
  namespace export set_tableFieldValue get_fieldValue get_tableFieldValue set_crypted
  namespace export decrypt_getHandle create_definition add_fieldDefinition
  namespace export dump_fieldDefinition encode_fieldDefinition decode_fieldDefinition
  namespace export set_crypted_definition decrypt_getDefinitionName encrypt_fieldDefinition
  namespace export decrypt_fieldDefinition delete_fieldDefinition delete_fieldDefinitions
  namespace export create_msgFromDefinition


# ============================= checkOKOrErr ========================

proc checkOKOrErr {result {where ""} {fieldName ""}}  {
  if  {$result eq $::STRUCT_MSG_ERR_OK}  {
    return "OK"
  }
#puts stderr [format  "error result: %d"  result]
  set errStr "ERROR"
  switch  [dict get $::structmsg(structmsgErrId2Str) $result]  {
    ERR_VALUE_NOT_SET {
      return [format "%s: %s: value for field: %s not set"  $errStr  $where  $fieldName]
    }
    ERR_VALUE_OUT_OF_RANGE {
      return [format "%s: %s: field: '%s' value out of range"  $errStr  $where  $fieldName]
    }
    ERR_BAD_VALUE {
      return [format "%s: %s: field: '%s' bad value"  $errStr  $where  $fieldName]
    }
    ERR_BAD_FIELD_TYPE {
      return [format "%s: %s: field: '%s' bad field type"  $errStr  $where  $fieldName]
    }
    ERR_FIELD_NOT_FOUND {
      return [format "%s: %s: field: '%s' not found"  $errStr  $where  $fieldName]
    }
    ERR_BAD_HANDLE {
      return [format "%s: bad handle"  $errStr]
    }
    ERR_OUT_OF_MEMORY {
      error [format "out of memory"]
    }
    ERR_HANDLE_NOT_FOUND {
      return [format "%s: handle not found"  $errStr]
    }
    ERR_NOT_ENCODED {
      return [format "%s: not encoded"  $errStr]
    }
    ERR_DECODE_ERROR {
      return [format "%s: decode error"  $errStr]
    }
    ERR_BAD_CRC_VALUE {
      return [format "%s: bad crc val"  $errStr]
    }
    ERR_CRYPTO_INIT_FAILED {
      return [format "%s: crypto init failed"  $errStr]
    }
    ERR_CRYPTO_OP_FAILED {
      return [format "%s: crypto op failed"  $errStr]
    }
    ERR_CRYPTO_BAD_MECHANISM {
      return [format "%s: crypto bad mechanism"  $errStr]
    }
    ERR_NOT_ENCRYPTED {
      return [format "%s: not encrypted"  $errStr]
    }
    ERR_DEFINITION_NOT_FOUND {
      return [format "%s: definiton not found"  $errStr]
    }
    ERR_BAD_SPECIAL_FIELD {
      return [format "%s: bad special field"  $errStr]
    }
    ERR_DEFINITION_TOO_MANY_FIELDS {
      return [format "%s: definition too many fields"  $errStr]
    }
    ERR_BAD_TABLE_ROW {
      return [format "%s: bad table row"  $errStr]
    }
    ERR_TOO_MANY_FIELDS {
      return [format "%s: too many fields"  $errStr]
    }
    ERR_BAD_DEFINTION_CMD_KEY {
      return [format "%s: bad definition cmd key"  $errStr]
    }
    ERR_NO_SLOT_FOUND {
      return [format "%s: no slot found"  $errStr]
    }
    default {
      return [format "%s: funny result error code"  $errStr]
    }
  }
}

# ============================= encdec_msgDefinition ========================

proc encdec_msgDefinition  {handle key enc iv bufVar lgthVar}  {
  upvar $bufVar buf
  upvar $lgthVar lgth

  set result [encdecDefinition $handle $key $enc $iv buf  lgth]
  return $result
}

# ============================= create ========================

proc create {numFields} {
  set result [cmd create  $numFields handle]
  if {$result eq $::STRUCT_MSG_ERR_OK}  {
    return $handle
  }
  return [checkOKOrErr $result "create" ""]
}

# ============================= delete ========================

proc delete {handle} {
  set result [stmsg_deleteMsg $handle
  return [checkOKOrErr $result "delete" "" ]
}

# ============================= encode ========================

proc encode {handle}  {
  set result [cmd encode $handle]
  return [checkOKOrErr $result "encode" "" ]
}

# ============================= get_encoded ========================

proc get_encoded {handle} {
  set result [cmd get_encoded $handle  encoded lgth]
  if {$result eq $::STRUCT_MSG_ERR_OK}  {
    return $encoded
  }
  return [checkOKOrErr $result "get_encoded" ""]
}

# ============================= decode ========================

proc decode {handle data} {
puts stderr "decode:"
  set result [cmd decode $handle $data]
puts stderr "result: $result!"
  return [checkOKOrErr $result "decode" ""]
}

# ============================= dump ========================

proc dump {handle} {
  set result [cmd dump $handle]
  return [checkOKOrErr $result "dump" ""]
}

# ============================= encrypt ========================

proc encrypt {handle key {iv ""}} {
  set result [cmd encrypt $handle [list] 0 $key $iv buf lgth]
  if {$result eq $::STRUCT_MSG_ERR_OK}  {
    return $buf
  }
  return [checkOKOrErr $result "encrypt" ""]
}

# ============================= decrypt ========================

proc decrypt {handle key {iv ""}} {
  set result [cmd decrypt $handle [list] 0 $key $iv buf lgth]
  if {$result eq $::STRUCT_MSG_ERR_OK}  {
    return $buf
  }
  return [checkOKOrErr $result "decrypt" ""]
}

# ============================= add_field ========================

proc add_field {handle fieldStr fieldType fieldLgth} {
  set result [cmd add_field $handle $fieldStr $fieldType $fieldLgth]
  return [checkOKOrErr $result "add_field" ""]
}

# ============================= set_fillerAndCrc ========================

proc set_fillerAndCrc {handle} {
  set result [stmsg_setFillerAndCrc $handle]
  return [checkOKOrErr $result "set_fillerAndCrc" ""]
}

# ============================= set_fieldValue ========================

proc set_fieldValue {handle fieldName value} {
  set result [cmd set_fieldValue $handle $fieldName $value]
puts stderr "api set_fieldValue: result:$result!"
  return [checkOKOrErr $result "set_fieldValue" $fieldName]
}

# ============================= set_tableFieldValue ========================

proc set_tableFieldValue {handle fieldName row value} {
  set result [cmd set_tableFieldValue $handle $fieldName $row  $value]
  return [checkOKOrErr $result "set_tableFieldValue" $fieldName]
}

# ============================= get_fieldValue ========================

proc get_fieldValue {handle fieldName} {
  set result [cmd get_fieldValue $handle $fieldName value]
  if {$result eq $::STRUCT_MSG_ERR_OK} {
    return $value
  }
  return [checkOKOrErr $result "get_fieldValue" $fieldName]
}

# ============================= get_tableFieldValue ========================

proc get_tableFieldValue {handle fieldName row} {
  set result [cmd get_tableFieldValue $handle $fieldName $row value]
  if {$result eq $::STRUCT_MSG_ERR_OK} {
    return $value
  }
  return [checkOKOrErr $result "get_tableFieldValue" $fieldName]
}

# ============================= set_crypted ========================

proc set_crypted {handle crypted} {
  set result [cmd set_crypted $handle $crypted]
  return [checkOKOrErr $result "set_crypted" ""]
}

# ============================= decrypt_getHandle ========================

proc decrypt_getHandle {encrypted key iv handleVar} {
  upvar $handleVar handle

  set result [cmd decrypt_getHandle $encrypted $key $iv handle]
  return [checkOKOrErr $result "decrypt_getHandle" ""]
}

# ============================= create_definition ========================

proc create_definition {name numFields} {
  set result [::structmsg def createDefinition $name $numFields]
  return [checkOKOrErr $result "create_Definition" ""]
}

# ============================= add_fieldDefinition ========================

proc add_fieldDefinition {name fieldName fieldType fieldLgth} {
  set result [::structmsg def addFieldDefinition $name $fieldName $fieldType $fieldLgth]
  return [checkOKOrErr $result "add_fieldDefinition" ""]
}

# ============================= dump_fieldDefinition ========================

proc dump_fieldDefinition {name} {
  set result [::structmsg def dumpFieldDefinition $name]
  return [checkOKOrErr $result "dump_fieldDefinition" ""]
}

# ============================= encode_fieldDefinition =================

proc encode_fieldDefinition {name} {
  set result [structmsg def encodeFieldDefinition $name data lgth]
  if  {$result == $::STRUCT_MSG_ERR_OK}  {
    return $data
  }
  return [checkOKOrErr $result "encode_fieldDefinition" ""]
}

# ============================= decode_fieldDefinition =================

proc decode_fieldDefinition {name encoded} {
  set result [structmsg def decodeFieldDefinition $name $encoded]
  return [checkOKOrErr $result "decode_fieldDefinition" ""]
}

# ============================= set_crypted_definition ========================

proc set_crypted_definition {name crypted} {
  set result [stmsg def setCryptedDefinition $handle $crypted]
  return [checkOKOrErr $result "set_crypted_definition" ""]
}

# ============================= decrypt_getDefinitionName ========================

proc decrypt_getDefinitionName {encrypted key {iv ""}} {
  set result [stmsg def decryptGetDefinitionName $encrypted $key $iv handle]
  if  {$result == $::STRUCT_MSG_ERR_OK} {
    return $handle
  }
  return [checkOKOrErr $result "decrypt_getDefinitionName" ""]
}

# ============================= encrypt_definition ========================

proc encrypt_definition {name key {iv}} {
  set result [structmsg def enccryptDefinition $name $key $iv true buf lgth]
  if  {$result == $::STRUCT_MSG_ERR_OK} {
    return $buf
  }
  return [checkOKOrErr $result "encrypt_definition" ""]
}

# ============================= decrypt_definition ========================

proc decrypt_definition {name key {iv ""}} {
  set result [structmsg def decryptDefinition $name $key $iv false buf lgth]
  if  {$result == $::STRUCT_MSG_ERR_OK} {
    return $buf
  }
  return [checkOKOrErr $result "decrypt_definition" ""]
}

# ============================= delete_fieldDefinition ========================

proc delete_fieldDefinition {name} {
  set result [structmsg def deleteFieldDefinition $name]
  return [checkOKOrErr $result "delete_fieldDefinition" ""]
}

# ============================= delete_fieldDefinitions ========================

proc delete_fieldDefinitions {} {
  set result [structmsg def deleteFieldDefinitions]
  return [checkOKOrErr $result "delete_fieldDefinitions" ""]
}


# ============================= create_msgFromDefinition ========================

proc create_msgFromDefinition {name}  {
  set result [structmsg def createMsgFromDefinition $name]
  return [checkOKOrErr $result "create_msgFromDefinition" ""]
}

} ; # namespace structmsg
