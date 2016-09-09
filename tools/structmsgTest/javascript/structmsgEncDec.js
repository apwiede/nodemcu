/*====================================================
 * A structured Message Support implementation in Javascript named Esp
 *
 * Structmsg EncDec for Esp (structmsgEncDec.js)
 *
 * Part of this code is taken from:
 * http://yuilibrary.com/ YUI 3.3 version
 *
 * Released under BSD/MIT license.
 * (BSD license found at <http://www.tcl.tk/software/tcltk/license.html>)
 *
 * Arnulf P. Wiedemann arnulf@wiedemann-pri.de (C)2016
 *
 */

EM.addModule("Esp-structmsgEncDec", function(T, name) {

  /* ==================== structmsgEncDec constructor ======================= */

  function structmsgEncDec() {
    T.log('constructor called', 'info', 'structmsgEncDec', true);

    var encDec = this;
    var constructor = encDec.constructor;
    structmsgEncDec.superclass.constructor.apply(encDec, arguments);

    T.log('constructor end', 'info', 'structmsgEncDec', true);
  }

  T.extend(structmsgEncDec, T.StructmsgInfos, {
    my_name: "structmsgEncDec",
    type_name: "encDec",
    flags: 0,
    
    /* ==================== toString ===================================== */
    
    toString: function () {
      var encdec = this;
      return encdec.mySelf()+"!";
    },
    
    /* ==================== toDebugString ===================================== */
    toDebugString: function () {
      var encdec = this;
      var str = encdec.mySelf()+"\n";
      return str;
    },
    
    // ============================= uint8Encode ========================
    
    uint8Encode: function(data, offset, value) {
      data[offset++] = value & 0xFF;
      return offset;
    },
    
    // ============================= int8Encode ========================
    
    int8Encode: function(data, offset, value) {
      data[offset++] = value & 0xFF;
      return offset;
    },
    
    // ============================= uint16Encode ========================
    
    uint16Encode: function(data, offset, value) {
      data[offset++] = (value >> 8) & 0xFF;
      data[offset++] = value & 0xFF;
    return offset;
    },
    
    // ============================= int16Encode ========================
    
    int16Encode: function(data, offset, value) {
      data[offset++] = (value >> 8) & 0xFF;
      data[offset++] = value & 0xFF;
      return offset;
    },
    
    // ============================= uint32Encode ========================
    
    uint32Encode: function(data, offset, value) {
      data[offset++] = (value >> 24) & 0xFF;
      data[offset++] = (value >> 16) & 0xFF;
      data[offset++] = (value >> 8) & 0xFF;
      data[offset++] = value & 0xFF;
      return offset;
    },
    
    // ============================= int32Encode ========================
    
    int32Encode: function(data, offset, value) {
      data[offset++] = (value >> 24) & 0xFF;
      data[offset++] = (value >> 16) & 0xFF;
      data[offset++] = (value >> 8) & 0xFF;
      data[offset++] = value & 0xFF;
      return offset;
    },
    
    // ============================= uint8VectorEncode ========================
    
    uint8VectorEncode: function(data, offset, value, lgth) {
      c_memcpy(data+offset,value,lgth);
      offset += lgth;
      return offset;
    },
    
    // ============================= int8VectorEncode ========================
    
    int8VectorEncode: function(data, offset, value, lgth) {
      c_memcpy(data+offset,value,lgth);
      offset += lgth;
      return offset;
    },
    
    // ============================= uint16VectorEncode ========================
    
    uint16VectorEncode: function(data, offset, value, lgth) {
      var idx;
    
      idx = 0;
      while (idx < lgth) {
        offset = uint16Encode(data, offset, value[idx]);
        idx++;
      }
      return offset;
    },
    
    // ============================= int16VectorEncode ========================
    
    int16VectorEncode: function(data, offset, value, lgth) {
      var idx;
    
      idx = 0;
      while (idx < lgth) {
        offset = int16Encode(data, offset, value[idx]);
        idx++;
      }
      return offset;
    },
    
    // ============================= uint32VectorEncode ========================
    
    uint32VectorEncode: function(data, offset, value, lgth) {
      var idx;
    
      idx = 0;
      while (idx < lgth) {
        offset = uint32Encode(data, offset, value[idx]);
        idx++;
      }
      return offset;
    },
    
    // ============================= int32VectorEncode ========================
    
    int32VectorEncode: function(data, offset, value, lgth) {
      var idx;
    
      idx = 0;
      while (idx < lgth) {
        offset = int32Encode(data, offset, value[idx]);
        idx++;
      }
      return offset;
    },
    
    // ============================= uint8Decode ========================
    
    uint8Decode: function(data, offset, value) {
      value = data[offset++] & 0xFF;
      return offset;
    },
    
    // ============================= int8Decode ========================
    
    int8Decode: function(data, offset, value) {
      value = data[offset++] & 0xFF;
      return offset;
    },
    
    // ============================= uint16Decode ========================
    
    uint16Decode: function(data, offset, value) {
      value = 0;
      value += (data[offset++] & 0xFF) << 8;
      value += (data[offset++] & 0xFF) << 0;
      return offset;
    },
    
    // ============================= int16Decode ========================
    
    int16Decode: function(data, offset, value) {
      value = 0;
      value += (data[offset++] & 0xFF) << 8;
      value += (data[offset++] & 0xFF) << 0;
      return offset;
    },
    
    // ============================= uint32Decode ========================
    
    uint32Decode: function(data, offset, value) {
      value = 0;
      value += (data[offset++] & 0xFF) << 24;
      value += (data[offset++] & 0xFF) << 16;
      value += (data[offset++] & 0xFF) << 8;
      value += (data[offset++] & 0xFF) << 0;
      return offset;
    },
    
    // ============================= int32Decode ========================
    
    int32Decode: function(data, offset, value) {
      value = 0;
      value += (data[offset++] & 0xFF) << 24;
      value += (data[offset++] & 0xFF) << 16;
      value += (data[offset++] & 0xFF) << 8;
      value += (data[offset++] & 0xFF) << 0;
      return offset;
    },
    
    // ============================= uint8VectorDecode ========================
    
    uint8VectorDecode: function(data, offset, lgth, value) {
      c_memcpy(value,data+offset,lgth);
      offset += lgth;
      return offset;
    },
    
    // ============================= int8VectorDecode ========================
    
    int8VectorDecode: function(data, offset, lgth, value) {
      c_memcpy(value,data+offset,lgth);
      offset += lgth;
      return offset;
    },
    
    // ============================= uint16VectorDecode ========================
    
    uint16VectorDecode: function(data, offset, lgth, value) {
      var idx;
    
      idx = 0;
      while (idx < lgth) {
        offset = uint16Decode(data, offset, value[idx]);
        idx++;
      }
      return offset;
    },
    
    // ============================= int16VectorDecode ========================
    
    int16VectorDecode: function(data, offset, lgth, value) {
      var idx;
    
      idx = 0;
      while (idx < lgth) {
        offset = int16Decode(data, offset, value[idx]);
        idx++;
      }
      return offset;
    },
    
    // ============================= uint32VectorDecode ========================
    
    uint32VectorDecode: function(data, offset, lgth, value) {
      var idx;
    
      idx = 0;
      while (idx < lgth) {
        offset = uint32Decode(data, offset, value[idx]);
        idx++;
      }
      return offset;
    },
    
    // ============================= int32VectorDecode ========================
    
    int32VectorDecode: function(data, offset, lgth, value) {
      var idx;
    
      idx = 0;
      while (idx < lgth) {
        offset = int32Decode(data, offset, value[idx]);
        idx++;
      }
      return offset;
    },
    
    // ============================= randomNumEncode ========================
    
    randomNumEncode: function(data, offset, value) {
      var val;
    
      val = (rand() & RAND_MAX);
      value = val;
      return uint32Encode(data, offset, val);
    },
    
    // ============================= randomNumDecode ========================
    
    randomNumDecode: function(data, offset, value) {
      var result;
    
      offset = uint32Decode(data, offset, value);
      return offset;
    },
    
    // ============================= sequenceNumEncode ========================
    
    sequenceNumEncode: function(data, offset, structmsg,  value) {
      var val;
    
      val = ++structmsg.sequenceNum;
      value = val;
      return uint32Encode(data, offset, val);
    },
    
    // ============================= sequenceNumDecode ========================
    
    sequenceNumDecode: function(data, offset, value) {
      var result;
    
      offset = uint32Decode(data, offset, value);
      return offset;
    },
    
    // ============================= fillerEncode ========================
    
    fillerEncode: function(data, offset, lgth, value) {
      var val;
      var idx;
    
      idx = 0;
      while (lgth >= 4) {
        val = (rand() & RAND_MAX);
        value[idx++] = (val >> 24) & 0xFF;
        value[idx++] = (val >> 16) & 0xFF;
        value[idx++] = (val >> 8) & 0xFF;
        value[idx++] = (val >> 0) & 0xFF;
        offset = uint32Encode(data, offset, val);
        lgth -= 4;
      }
      while (lgth >= 2) {
        val = ((rand() & RAND_MAX) & 0xFFFF);
        value[idx++] = (val >> 8) & 0xFF;
        value[idx++] = (val >> 0) & 0xFF;
        offset = uint16Encode(data, offset, val);
        lgth -= 2;
      }
      while (lgth >= 1) {
        val = ((rand() & RAND_MAX) & 0xFF);
        offset = uint8Encode(data, offset, val);
        value[idx++] = (val >> 0) & 0xFF;
        lgth -= 1;
      }
      return offset;
    },
    
    // ============================= fillerDecode ========================
    
    fillerDecode: function(data, offset, lgth, value) {
      var idx;
    
      c_memcpy(value,data+offset,lgth);
      offset += lgth;
      return offset;
    },
    
    // ============================= crcEncode ========================
    
    crcEncode: function(data, offset, lgth, crc, headerLgth) {
      var idx;
    
      lgth -= sizeof(uint16_t);   // uint16_t crc
      crc = 0;
      idx = headerLgth;
      while (idx < lgth) {
        //ets_printf("crc idx: %d ch: 0x%02x crc: 0x%04x\n", idx-headerLgth, data[idx], crc);
        crc += data[idx++];
      }
      crc = ~(crc);
      offset = uint16Encode(data,offset,crc);
      return offset;
    },
    
    // ============================= crcDecode ========================
    
    crcDecode: function(data, offset, lgth, crc, headerLgth) {
      var crcVal;
      var idx;
    
      lgth -= sizeof(uint16_t);   // uint16_t crc
      crcVal = 0;
      idx = headerLgth;
      while (idx < lgth + headerLgth) {
    //ets_printf("crc idx: %d ch: 0x%02x crc: 0x%04x\n", idx-headerLgth, data[idx], crcVal);
        crcVal += data[idx++];
      }
      crcVal = ~crcVal;
      offset = uint16Decode(data, offset, crc);
      ets_printf("crcVal: 0x%04x crc: 0x%04x\n", crcVal, crc);
      if (crcVal != crc) {
        return -1;
      }
      return offset;
    },
    
    // ============================= getFieldIdName ========================
    
    getFieldIdName: function (id, fieldNameDefinitions, fieldName) {
      // find field name
      var idx = 0;
      while (idx < fieldNameDefinitions.numDefinitions) {
        var entry = fieldNameDefinitions.definitions[idx];
        if (entry.id == id) {
          fieldName = entry.str;
          return STRUCT_MSG_ERR_OK;
        }
        entry++;
        idx++;
      }
      return STRUCT_MSG_ERR_FIELD_NOT_FOUND;
    },
    
    // ============================= normalFieldNamesEncode ========================
    
    normalFieldNamesEncode: function(data, offset, definition, fieldNameDefinitions, normNameOffsets, numEntries, size) {
      var idx;
      var namesOffset = 0;
      var fieldInfo;
      var result;
      var nameIdx;
      var fieldName;
      var normNameOffset;
    
      // first the keys
      offset = uint8Encode(data, offset, numEntries);
      idx = 0;
      normNameOffset = normNameOffsets[0];
      while (idx < definition.numFields) {
        fieldInfo = definition.fieldInfos[idx];
        if (fieldInfo.fieldId < STRUCT_MSG_SPEC_FIELD_LOW) { 
          result = getFieldIdName(fieldInfo.fieldId, fieldNameDefinitions, fieldName);
          checkErrOK(result);
          offset = uint16Encode(data, offset, namesOffset);
          normNameOffset.id = fieldInfo.fieldId;
          normNameOffset.offset = namesOffset;
          normNameOffset++;
          namesOffset += c_strlen(fieldName) + 1;
        }
        idx++;
      }
      // and now the names
      offset = uint16Encode(data, offset, size);
      idx = 0;
      nameIdx = 1;
      while (idx < definition.numFields) {
        fieldInfo = definition.fieldInfos[idx];
        if (fieldInfo.fieldId < STRUCT_MSG_SPEC_FIELD_LOW) { 
          result = getFieldIdName(fieldInfo.fieldId, fieldNameDefinitions, fieldName);
          checkErrOK(result);
          offset = uint8VectorEncode(data, offset, fieldName, c_strlen(fieldName));
          if (nameIdx < numEntries) {
            offset = uint8Encode(data, offset, '\0');
          } else {
            offset = uint8Encode(data, offset, '\0');
          }
          nameIdx++;
        }
        idx++;
      }
      return offset;
    },
    
    // ============================= normalFieldNamesDecode ========================
    
    normalFieldNamesDecode: function(data, offset) {
      return offset;
    },
    
    // ============================= definitionEncode ========================
    
    definitionEncode: function(data, offset, definition, fieldNameDefinitions, normNamesOffsets) {
      var idx;
      var idIdx;
      var nameOffset;
      var found;
      var fieldInfo;
      var fieldId;
      var fieldType;
      var fieldLgth;
    
      idx= 0;
      fieldInfo = definition.fieldInfos[0];
      offset = uint8Encode(data, offset, definition.numFields); 
      checkEncodeOffset(offset);
      while (idx < definition.numFields) {
        if (fieldInfo.fieldId < STRUCT_MSG_SPEC_FIELD_LOW) {
          idIdx = 0;
          found = 0;
          while (idIdx < definition.numFields) {
            if (normNamesOffsets[idIdx].id == 0) {
              // id 0 is not used to be able to stop here!!
              break;
            }
            if (fieldInfo.fieldId == normNamesOffsets[idIdx].id) {
              nameOffset = normNamesOffsets[idIdx].offset;
              found = 1;
              break;
            }
            idIdx++;
          }
          if (!found) {
            return STRUCT_MSG_ERR_FIELD_NOT_FOUND;
          }
          offset = uint16Encode(data, offset, nameOffset);
        } else {
          offset = uint16Encode(data, offset, fieldInfo.fieldId);
        }
        offset = uint8Encode(data, offset, fieldInfo.fieldType);
        offset = uint16Encode(data, offset, fieldInfo.fieldLgth);
        fieldInfo++;
        idx++;
      }
      return offset;
    },
    
    // ============================= definitionDecode ========================
    
    definitionDecode: function(data, offset, definition, fieldNameDefinitions) {
      var definitionLgth;
      var nameLgth;
      var numNameEntries;
      var name;
      var idsStart;
      var idsEnd;
      var namesStart;
      var namesEnd;
      var namesSize;
      var definitionStart;
      var fieldId;
      var nameOffset;
      var fieldTypeId;
      var fieldLgth;
      var fieldName;
      var fieldType;
      var definitionIdx;
      var numFields;
      var result;
      var myOffset;
      var namesIdx;
    
      // first the keys
      offset = uint8Decode(data, offset, numNameEntries);
      checkBadOffset(offset);
      idsStart = (data + offset);
      offset += numNameEntries * sizeof(uint16_t);
      idsEnd = (data + offset);
      // and now the names
      offset = uint16Decode(data, offset, namesSize);
      checkBadOffset(offset);
      namesStart = data + offset;
      offset += namesSize;
      definitionStart = (namesStart + namesSize);
      offset = uint16Decode(data, offset, definitionLgth);
      checkBadOffset(offset);
      offset = uint8Decode(data, offset, nameLgth);
      checkBadOffset(offset);
      name = data+offset;
      offset += nameLgth;
      offset = uint8Decode(data, offset, numFields);
      checkBadOffset(offset);
      result = structmsg_createStructmsgDefinition (name, numFields);
      checkOffsetErrOK(result);
      definitionIdx = 0;
      namesIdx = 0;
      while (definitionIdx < numFields) { 
        offset = uint16Decode(data, offset, fieldId);
        checkBadOffset(offset);
        if (fieldId > STRUCT_MSG_SPEC_FIELD_LOW) {
          result = structmsg_getIdFieldNameStr(fieldId, fieldName);
          checkOffsetErrOK(result);
        } else {
          fieldId = namesIdx + 1;
          myOffset = (idsStart - data) + (namesIdx * sizeof(uint16_t));
          myOffset = uint16Decode(data, myOffset, nameOffset);
          fieldName = (namesStart+nameOffset);
          namesIdx++;
        }
        offset = uint8Decode(data, offset, fieldTypeId);
        checkBadOffset(offset);
        result = structmsg_getFieldTypeStr(fieldTypeId, fieldType);
        checkOffsetErrOK(result);
        offset = uint16Decode(data, offset, fieldLgth);
        checkBadOffset(offset);
    //ets_printf("add field: %s fieldId: %d fieldType: %d  %s fieldLgth: %d offset: %d\n", fieldName, fieldId, fieldTypeId, fieldType, fieldLgth, offset);  
        result = structmsg_addFieldDefinition (name, fieldName, fieldType, fieldLgth);
        checkOffsetErrOK(result);
        definitionIdx++;
      }
      return offset;
    },
    
    // ============================= structmsg_fillHdrInfo ========================
    
    structmsg_fillHdrInfo: function(handle, structmsg) {
      var hdrInfoPtr;
      var offset;
    
      // fill the hdrInfo
      structmsg = structmsg_get_structmsg_ptr(handle);
      hdrInfoPtr = structmsg.handleHdrInfoPtr;
      hdrInfoPtr = (structmsg.hdr.hdrInfo);
      offset = 0;
      offset = uint16Encode(hdrInfoPtr.hdrId, offset, structmsg.hdr.hdrInfo.hdrKeys.src);
      checkEncodeOffset(offset);
      offset = uint16Encode(hdrInfoPtr.hdrId, offset, structmsg.hdr.hdrInfo.hdrKeys.dst);
      checkEncodeOffset(offset);
      offset = uint16Encode(hdrInfoPtr.hdrId, offset, structmsg.hdr.hdrInfo.hdrKeys.totalLgth);
      checkEncodeOffset(offset);
      offset = uint16Encode(hdrInfoPtr.hdrId, offset, structmsg.hdr.hdrInfo.hdrKeys.cmdKey);
      checkEncodeOffset(offset);
      offset = uint16Encode(hdrInfoPtr.hdrId, offset, structmsg.hdr.hdrInfo.hdrKeys.cmdLgth);
      checkEncodeOffset(offset);
      return STRUCT_MSG_ERR_OK;
    },
    
    // ============================= encodeField ========================
    
    encodeField: function(msgPtr, fieldInfo, offset) {
      var fieldIdx;
    
      switch (fieldInfo.fieldType) {
      case STRUCT_MSG_FIELD_INT8_T:
        offset = int8Encode(msgPtr,offset,fieldInfo.value.byteVal);
        break;
      case STRUCT_MSG_FIELD_UINT8_T:
        offset = uint8Encode(msgPtr,offset,fieldInfo.value.ubyteVal);
        break;
      case STRUCT_MSG_FIELD_INT16_T:
        offset = int16Encode(msgPtr,offset,fieldInfo.value.shortVal);
        break;
      case STRUCT_MSG_FIELD_UINT16_T:
        offset = uint16Encode(msgPtr,offset,fieldInfo.value.ushortVal);
        break;
      case STRUCT_MSG_FIELD_INT32_T:
        offset = int32Encode(msgPtr,offset,fieldInfo.value.val);
        break;
      case STRUCT_MSG_FIELD_UINT32_T:
        offset = uint32Encode(msgPtr,offset,fieldInfo.value.uval);
        break;
      case STRUCT_MSG_FIELD_INT8_VECTOR:
        fieldIdx = 0;
        while (fieldIdx < fieldInfo.fieldLgth) {
          offset = int8Encode(msgPtr,offset,fieldInfo.value.byteVector[fieldIdx]);
          fieldIdx++;
        }
        break;
      case STRUCT_MSG_FIELD_UINT8_VECTOR:
        fieldIdx = 0;
        while (fieldIdx < fieldInfo.fieldLgth) {
          offset = uint8Encode(msgPtr,offset,fieldInfo.value.ubyteVector[fieldIdx]);
          fieldIdx++;
        }
        break;
      case STRUCT_MSG_FIELD_INT16_VECTOR:
        fieldIdx = 0;
        while (fieldIdx < fieldInfo.fieldLgth) {
          offset = int16Encode(msgPtr,offset,fieldInfo.value.shortVector[fieldIdx]);
          fieldIdx++;
        }
        break;
      case STRUCT_MSG_FIELD_UINT16_VECTOR:
        fieldIdx = 0;
        while (fieldIdx < fieldInfo.fieldLgth) {
          offset = uint16Encode(msgPtr,offset,fieldInfo.value.ushortVector[fieldIdx]);
          fieldIdx++;
        }
        break;
      case STRUCT_MSG_FIELD_INT32_VECTOR:
        fieldIdx = 0;
        while (fieldIdx < fieldInfo.fieldLgth) {
          offset = int32Encode(msgPtr,offset,fieldInfo.value.int32Vector[fieldIdx]);
          fieldIdx++;
        }
        break;
      case STRUCT_MSG_FIELD_UINT32_VECTOR:
        fieldIdx = 0;
        while (fieldIdx < fieldInfo.fieldLgth) {
          offset = uint32Encode(msgPtr,offset,fieldInfo.value.uint32Vector[fieldIdx]);
          fieldIdx++;
        }
        break;
      }
      return offset;
    },
    
    // ============================= decodeField ========================
    
    decodeField: function(msgPtr, fieldInfo,  offset) {
      var fieldIdx;
    
      switch (fieldInfo.fieldType) {
      case STRUCT_MSG_FIELD_INT8_T:
        offset = int8Decode(msgPtr,offset,fieldInfo.value.byteVal);
        checkDecodeOffset(offset);
        break;
      case STRUCT_MSG_FIELD_UINT8_T:
        offset = uint8Decode(msgPtr,offset,fieldInfo.value.ubyteVal);
        checkDecodeOffset(offset);
        break;
      case STRUCT_MSG_FIELD_INT16_T:
        offset = int16Decode(msgPtr,offset,fieldInfo.value.shortVal);
        checkDecodeOffset(offset);
        break;
      case STRUCT_MSG_FIELD_UINT16_T:
        offset = uint16Decode(msgPtr,offset,fieldInfo.value.ushortVal);
        checkDecodeOffset(offset);
        break;
      case STRUCT_MSG_FIELD_INT32_T:
        offset = int32Decode(msgPtr,offset,fieldInfo.value.val);
        checkDecodeOffset(offset);
        break;
      case STRUCT_MSG_FIELD_UINT32_T:
        offset = uint32Decode(msgPtr,offset,fieldInfo.value.uval);
        checkDecodeOffset(offset);
        break;
      case STRUCT_MSG_FIELD_INT8_VECTOR:
        offset = int8VectorDecode(msgPtr,offset,fieldInfo.fieldLgth, fieldInfo.value.byteVector);
        checkDecodeOffset(offset);
        break;
      case STRUCT_MSG_FIELD_UINT8_VECTOR:
        offset = uint8VectorDecode(msgPtr,offset,fieldInfo.fieldLgth, fieldInfo.value.ubyteVector);
        checkDecodeOffset(offset);
        break;
      case STRUCT_MSG_FIELD_INT16_VECTOR:
        offset = int16VectorDecode(msgPtr,offset,fieldInfo.fieldLgth, fieldInfo.value.shortVector);
        checkDecodeOffset(offset);
        break;
      case STRUCT_MSG_FIELD_UINT16_VECTOR:
        offset = uint16VectorDecode(msgPtr,offset,fieldInfo.fieldLgth, fieldInfo.value.ushortVector);
        checkDecodeOffset(offset);
        break;
      case STRUCT_MSG_FIELD_INT32_VECTOR:
        offset = int32VectorDecode(msgPtr,offset,fieldInfo.fieldLgth, fieldInfo.value.int32Vector);
        checkDecodeOffset(offset);
        break;
      case STRUCT_MSG_FIELD_UINT32_VECTOR:
        offset = uint32VectorDecode(msgPtr,offset,fieldInfo.fieldLgth, fieldInfo.value.uint32Vector);
        checkDecodeOffset(offset);
        break;
      }
      return offset;
    },
    
    // ============================= stmsg_encodeMsg ========================
    
    stmsg_encodeMsg: function(handle) {
      var structmsg;
      var msgPtr;
      var ushortPtr;
      var shortPtr;
      var uintPtr;
      var intPtr;
      var randomNum;
      var sequenceNum;
      var crc;
      var offset;
      var idx;
      var fieldIdx;
      var numEntries;
      var result;
      var fieldId = 0;
      var fieldInfo;
    
      structmsg = structmsg_get_structmsg_ptr(handle);
      checkHandleOK(structmsg);
      if (structmsg.encoded != NULL) {
        os_free(structmsg.encoded);
      }
      if ((structmsg.flags & STRUCT_MSG_HAS_CRC) == 0) {
        result = stmsg_setFillerAndCrc(handle);
        checkErrOK(result);
        structmsg.flags |= STRUCT_MSG_HAS_CRC;
      }
      //      structmsg.encoded = os_zalloc(structmsg.hdr.hdrInfo.hdrKeys.totalLgth);
      msgPtr = structmsg.encoded;
      offset = 0;
      offset = uint16Encode(msgPtr,offset,structmsg.hdr.hdrInfo.hdrKeys.src);
      checkEncodeOffset(offset);
      offset = uint16Encode(msgPtr,offset,structmsg.hdr.hdrInfo.hdrKeys.dst);
      checkEncodeOffset(offset);
      offset = uint16Encode(msgPtr,offset,structmsg.hdr.hdrInfo.hdrKeys.totalLgth);
      checkEncodeOffset(offset);
      offset = uint16Encode(msgPtr,offset,structmsg.hdr.hdrInfo.hdrKeys.cmdKey);
      checkEncodeOffset(offset);
      offset = uint16Encode(msgPtr,offset,structmsg.hdr.hdrInfo.hdrKeys.cmdLgth);
      checkEncodeOffset(offset);
      numEntries = structmsg.msg.numFieldInfos;
      offset = uint8Encode(msgPtr,offset,numEntries);
      checkEncodeOffset(offset);
      idx = 0;
      while (idx < numEntries) {
        fieldInfo = structmsg.msg.fieldInfos[idx];
        if (fieldInfo.fieldStr[0] == '@') {
          result = structmsg_getFieldNameId(fieldInfo.fieldStr, fieldId, STRUCT_MSG_NO_INCR);
          checkErrOK(result);
          switch (fieldId) {
          case STRUCT_MSG_SPEC_FIELD_SRC:
          case STRUCT_MSG_SPEC_FIELD_DST:
          case STRUCT_MSG_SPEC_FIELD_TARGET_CMD:
          case STRUCT_MSG_SPEC_FIELD_TOTAL_LGTH:
          case STRUCT_MSG_SPEC_FIELD_CMD_KEY:
          case STRUCT_MSG_SPEC_FIELD_CMD_LGTH:
    ets_printf("funny should encode: %s\n", fieldInfo.fieldStr);
            break;
          case STRUCT_MSG_SPEC_FIELD_RANDOM_NUM:
            offset = randomNumEncode(msgPtr, offset, randomNum);
            checkEncodeOffset(offset);
            result = stmsg_setFieldValue(handle, "@randomNum", randomNum, NULL);
            checkErrOK(result);
            break;
          case STRUCT_MSG_SPEC_FIELD_SEQUENCE_NUM:
            offset = sequenceNumEncode(msgPtr, offset, structmsg, sequenceNum);
            checkEncodeOffset(offset);
            result = stmsg_setFieldValue(handle, "@sequenceNum", sequenceNum, NULL);
            checkErrOK(result);
            break;
          case STRUCT_MSG_SPEC_FIELD_FILLER:
            offset = fillerEncode(msgPtr, offset, fieldInfo.fieldLgth, fieldInfo.value.ubyteVector);
            checkEncodeOffset(offset);
            result = stmsg_setFieldValue(handle, "@filler", 0, fieldInfo.value.ubyteVector);
            checkErrOK(result);
            break;
          case STRUCT_MSG_SPEC_FIELD_CRC:
            offset = crcEncode(structmsg.encoded, offset, structmsg.hdr.hdrInfo.hdrKeys.totalLgth, crc, structmsg.hdr.headerLgth);
            checkEncodeOffset(offset);
            result = stmsg_setFieldValue(handle, "@crc", crc, NULL);
            checkErrOK(result);
            break;
          case STRUCT_MSG_SPEC_FIELD_ID:
            return STRUCT_MSG_ERR_BAD_SPECIAL_FIELD;
            break;
          case STRUCT_MSG_SPEC_FIELD_TABLE_ROWS:
            break;
          case STRUCT_MSG_SPEC_FIELD_TABLE_ROW_FIELDS:
            if (structmsg.msg.numTableRows > 0) {
              var row = 0;
              var col = 0;
              var cell = 0;
              while (row < structmsg.msg.numTableRows) {
    	        while (col < structmsg.msg.numRowFields) {
    	           cell = col + row * structmsg.msg.numRowFields;
    	           fieldInfo = structmsg.msg.tableFieldInfos[cell];
    	           offset = encodeField(msgPtr, fieldInfo, offset);
    	           checkEncodeOffset(offset);
    	           col++;
    	        }
    	        row++;
    	        col = 0;
              }
            }
            break;
          }
        } else {
          offset = encodeField(msgPtr, fieldInfo, offset);
          checkEncodeOffset(offset);
        }
        idx++;
      }
      structmsg.flags |= STRUCT_MSG_ENCODED;
      return STRUCT_MSG_ERR_OK;
    },
    
    // ============================= stmsg_getEncoded ========================
    
    stmsg_getEncoded: function(handle, encoded, lgth) {
      var structmsg;
    
      structmsg = structmsg_get_structmsg_ptr(handle);
      checkHandleOK(structmsg);
      if (structmsg.encoded == NULL) {
        return STRUCT_MSG_ERR_NOT_ENCODED;
      }
      encoded = structmsg.encoded;
      lgth = structmsg.hdr.hdrInfo.hdrKeys.totalLgth;
      return STRUCT_MSG_ERR_OK;
    },
    
    // ============================= stmsg_decodeMsg ========================
    
    stmsg_decodeMsg: function(handle, data) {
      var structmsg;
      var msgPtr;
      var crc;
      var offset;
      var idx;
      var fieldIdx;
      var fieldId;
      var numEntries;
      var result = STRUCT_MSG_ERR_OK;
      var fieldInfo;
    
      structmsg = structmsg_get_structmsg_ptr(handle);
      checkHandleOK(structmsg);
      if (structmsg.todecode != NULL) {
        os_free(structmsg.todecode);
      }
      //      structmsg.todecode = os_zalloc(structmsg.hdr.hdrInfo.hdrKeys.totalLgth);
      c_memcpy(structmsg.todecode, data, structmsg.hdr.hdrInfo.hdrKeys.totalLgth);
      msgPtr = structmsg.todecode;
      offset = 0;
      offset = uint16Decode(msgPtr,offset,structmsg.hdr.hdrInfo.hdrKeys.src);
      checkDecodeOffset(offset);
      checkErrOK(result);
      offset = uint16Decode(msgPtr,offset,structmsg.hdr.hdrInfo.hdrKeys.dst);
      checkDecodeOffset(offset);
      checkErrOK(result);
      offset = uint16Decode(msgPtr,offset,structmsg.hdr.hdrInfo.hdrKeys.totalLgth);
      checkDecodeOffset(offset);
      checkErrOK(result);
      offset = uint16Decode(msgPtr,offset,structmsg.hdr.hdrInfo.hdrKeys.cmdKey);
      checkDecodeOffset(offset);
      checkErrOK(result);
      offset = uint16Decode(msgPtr,offset,structmsg.hdr.hdrInfo.hdrKeys.cmdLgth);
      checkDecodeOffset(offset);
      result = structmsg_fillHdrInfo(handle, structmsg);
      offset = uint8Decode(msgPtr,offset,numEntries);
      checkDecodeOffset(offset);
      idx = 0;
      //  numEntries = structmsg.msg.numFieldInfos;
      while (idx < numEntries) {
        fieldInfo = structmsg.msg.fieldInfos[idx];
        if (fieldInfo.fieldStr[0] == '@') {
          result = structmsg_getFieldNameId(fieldInfo.fieldStr, fieldId, STRUCT_MSG_NO_INCR);
          checkErrOK(result);
          switch (fieldId) {
          case STRUCT_MSG_SPEC_FIELD_SRC:
          case STRUCT_MSG_SPEC_FIELD_DST:
          case STRUCT_MSG_SPEC_FIELD_TARGET_CMD:
          case STRUCT_MSG_SPEC_FIELD_TOTAL_LGTH:
          case STRUCT_MSG_SPEC_FIELD_CMD_KEY:
          case STRUCT_MSG_SPEC_FIELD_CMD_LGTH:
        ets_printf("funny should decode: %s\n", fieldInfo.fieldStr);
            break;
          case STRUCT_MSG_SPEC_FIELD_RANDOM_NUM:
            offset = randomNumDecode(msgPtr, offset, fieldInfo.value.uval);
            checkDecodeOffset(offset);
            break;
          case STRUCT_MSG_SPEC_FIELD_SEQUENCE_NUM:
            offset = sequenceNumDecode(msgPtr, offset, fieldInfo.value.uval);
            checkDecodeOffset(offset);
            break;
          case STRUCT_MSG_SPEC_FIELD_FILLER:
            offset = fillerDecode(msgPtr, offset, fieldInfo.fieldLgth, fieldInfo.value.ubyteVector);
            checkDecodeOffset(offset);
            break;
          case STRUCT_MSG_SPEC_FIELD_CRC:
            offset = crcDecode(msgPtr, offset, structmsg.hdr.hdrInfo.hdrKeys.cmdLgth, fieldInfo.value.ushortVal, structmsg.hdr.headerLgth);
            if (offset < 0) {
              return STRUCT_MSG_ERR_BAD_CRC_VALUE;
            }
            break;
          case STRUCT_MSG_SPEC_FIELD_ID:
            return STRUCT_MSG_ERR_BAD_SPECIAL_FIELD;
            break;
          case STRUCT_MSG_SPEC_FIELD_TABLE_ROWS:
            break;
          case STRUCT_MSG_SPEC_FIELD_TABLE_ROW_FIELDS:
            if (structmsg.msg.numTableRows > 0) {
              var row = 0;
              var col = 0;
              var cell = 0;
              while (row < structmsg.msg.numTableRows) {
    	        while (col < structmsg.msg.numRowFields) {
    	           cell = col + row * structmsg.msg.numRowFields;
    	           fieldInfo = structmsg.msg.tableFieldInfos[cell];
    	           offset = decodeField(msgPtr, fieldInfo, offset);
    	           checkEncodeOffset(offset);
    	           col++;
    	        }
    	        row++;
    	        col = 0;
              }
            }
            break;
          }
          fieldInfo.flags |= STRUCT_MSG_FIELD_IS_SET;
        } else {
          offset = decodeField(msgPtr, fieldInfo, offset);
          checkEncodeOffset(offset);
          fieldInfo.flags |= STRUCT_MSG_FIELD_IS_SET;
        }
        idx++;
      }
      structmsg.flags |= STRUCT_MSG_DECODED;
      return STRUCT_MSG_ERR_OK;
    },
    
    // ============================= getSpecFieldSizes ========================
    
    getSpecFieldSizes: function(numFields, namesSize) {
      numFields = 0;
      namesSize = 0;
      var entry = structmsgSpecialFieldNames[0];
      while (entry.str != NULL) {
        (numFields)++;
        namesSize += c_strlen(entry.str) + 1;  // +1 for "," as separator
        //ets_printf("%s: %d %d\n", entry.str, *numFields, *namesSize);
        entry++;
      }
      return STRUCT_MSG_ERR_OK;
    },
    
    // ============================= structmsg_encodeDefinition ========================
    
    structmsg_encodeDefinition: function(name, data, lgth, structmsgDefinitions, fieldNameDefinitions) {
      var numSpecFields;
      var namesSpecSize;
      var numNormFields;
      var normNamesSize;
      var totalLgth;
      var cmdLgth;
      var cmdKey = STRUCT_MSG_DEFINITION_CMD_KEY;
      var fillerSize;
      var result;
      var idx;
      var found = 0;
      var definitionsIdx;
      var fieldName;
      var definitionPayloadSize;
      var payloadSize;
      var myLgth;
      var randomNum;
      var definition;
      var fieldInfo;
      var encoded;
      var offset;
        var crc;
      var normNamesOffsets;
      // FIXME!!
      var src = 123;
      var dst = 987;
    
      result =  structmsg_getDefinitionPtr(name, definition, definitionsIdx);
      checkErrOK(result);
      numNormFields = 0;
      normNamesSize = 0;
      idx = 0;
      while (idx < definition.numFields) {
        fieldInfo = definition.fieldInfos[idx];
        if (fieldInfo.fieldId < STRUCT_MSG_SPEC_FIELD_LOW) {
          result = getFieldIdName(fieldInfo.fieldId, fieldNameDefinitions, fieldName);
          checkErrOK(result);
    //ets_printf("fieldName: %s\n", fieldName);
          numNormFields++;
          normNamesSize += c_strlen(fieldName) + 1;
        }
        idx++;
      }
      //      normNamesOffsets = os_zalloc(numNormFields * sizeof(id2offset_t) + 1);
      checkAllocOK(normNamesOffsets);
      payloadSize = STRUCT_MSG_CMD_HEADER_LENGTH; // cmdKey + cmdLgth
      // randomNum
      payloadSize += sizeof(uint32_t);
      // len ids + ids (numNormFields * (uint16_t)) + len Names + names size
      payloadSize += sizeof(uint8_t) + (numNormFields * sizeof(uint16_t)) + sizeof(uint16_t) + normNamesSize;
      // definitionPayloadSize
    
      // definitionLgth + nameLgth + name of Definition
      definitionPayloadSize = sizeof(uint16_t) + sizeof(uint8_t) + (c_strlen(name) + 1);
      // numFields (uint8_t) + numFields * (fieldId uint16_t, fieldType uint8_t, fieldLgth uint16_t)
      definitionPayloadSize += sizeof(uint8_t) + definition.numFields * (sizeof(uint16_t) + sizeof(uint8_t) + sizeof(uint16_t));
    
      payloadSize += definitionPayloadSize;
      fillerSize = 0;
      myLgth = payloadSize + sizeof(uint16_t); // sizeof(uint16_t) for CRC
      while ((myLgth % 16) != 0) {
        myLgth++;
        fillerSize++;
      }
      cmdLgth = payloadSize + fillerSize + sizeof(uint16_t);
      totalLgth = STRUCT_MSG_HEADER_LENGTH + cmdLgth;
      definition.totalLgth = totalLgth;
      //      definition.encoded = os_zalloc(totalLgth);
      checkAllocOK(definition.encoded);
      encoded = definition.encoded;
      offset = 0;
      offset = uint16Encode(encoded, offset, src); 
      checkEncodeOffset(offset);
      offset = uint16Encode(encoded, offset, dst); 
      checkEncodeOffset(offset);
      offset = uint16Encode(encoded, offset, totalLgth); 
      checkEncodeOffset(offset);
      offset = uint16Encode(encoded, offset, cmdKey); 
      checkEncodeOffset(offset);
      offset = uint16Encode(encoded, offset, cmdLgth); 
      checkEncodeOffset(offset);
      offset = randomNumEncode(encoded, offset, randomNum); 
      checkEncodeOffset(offset);
      offset = normalFieldNamesEncode(encoded, offset, definition, fieldNameDefinitions, normNamesOffsets, numNormFields, normNamesSize);
      offset = uint16Encode(encoded, offset, definitionPayloadSize); 
      checkEncodeOffset(offset);
      offset = uint8Encode(encoded, offset, c_strlen(name)+1); 
      checkEncodeOffset(offset);
      offset = uint8VectorEncode(encoded, offset, name, c_strlen(name)); 
      checkEncodeOffset(offset);
      offset = uint8Encode(encoded, offset, '\0'); 
      checkEncodeOffset(offset);
      offset = definitionEncode(encoded, offset, definition, fieldNameDefinitions, normNamesOffsets);
      checkEncodeOffset(offset);
      os_free(normNamesOffsets); // no longer needed
      var dummy;
      offset = fillerEncode(encoded, offset, fillerSize, dummy[0]);
      checkEncodeOffset(offset);
      offset = crcEncode(encoded, offset, totalLgth, crc, STRUCT_MSG_HEADER_LENGTH);
      checkEncodeOffset(offset);
      ets_printf("after crc offset: %d totalLgth :%d crc: 0x%04x\n", offset, totalLgth, crc);
      data = encoded;
      lgth = totalLgth;
      return STRUCT_MSG_ERR_OK;
    
    },
    
    // ============================= structmsg_decodeDefinition ========================
    
    structmsg_decodeDefinition: function(name, data, structmsgDefinitions, fieldNameDefinitions) {
      var numSpecFields;
      var namesSpecSize;
      var numNormFields;
      var namesNormSize;
      var fillerSize;
      var result;
      var idx;
      var found = 0;
      var fieldName;
      var definitionPayloadSize;
      var payloadSize;
      var myLgth;
      var definition;
      var fieldInfo;
      var offset;
      var crcOffset;
      var src;
      var dst;
      var totalLgth;
      var cmdKey;
      var cmdLgth;
      var crc;
      var randomNum;
      var fillerStr;
      var filler = fillerStr;;
      var numEntries;
    
      offset = 0;
      offset = uint16Decode(data, offset, src); 
      checkDecodeOffset(offset);
      offset = uint16Decode(data, offset, dst); 
      checkDecodeOffset(offset);
      offset = uint16Decode(data, offset, totalLgth); 
      checkDecodeOffset(offset);
      offset = uint16Decode(data, offset, cmdKey); 
      checkDecodeOffset(offset);
      if (cmdKey != STRUCT_MSG_DEFINITION_CMD_KEY) {
        return STRUCT_MSG_ERR_BAD_DEFINTION_CMD_KEY;
      }
      offset = uint16Decode(data, offset, cmdLgth); 
      checkDecodeOffset(offset);
      offset = uint32Decode(data, offset, randomNum); 
      checkDecodeOffset(offset);
      // now check the crc
      crcOffset = totalLgth - sizeof(uint16_t);
      crcOffset = crcDecode(data, crcOffset, cmdLgth, crc, STRUCT_MSG_HEADER_LENGTH);
      offset = definitionDecode(data, offset, definition, fieldNameDefinitions);
      checkDecodeOffset(offset);
      myLgth = offset + sizeof(uint16_t);
      fillerSize = 0;
      while ((myLgth % 16) != 0) {
        myLgth++;
        fillerSize++;
      }
      offset = fillerDecode(data, offset, fillerSize, filler);
      checkDecodeOffset(offset);
      return STRUCT_MSG_ERR_OK;
    },
    
    // ============================= stmsg_getDefinitionName ========================
    
    stmsg_getDefinitionName: function(decrypted, name) {
      var nameOffset;
      var numNormFields;
      var normNamesSize;
      var nameLgth;
      var numEntries;
    
      name = NULL;
      nameOffset = STRUCT_MSG_HEADER_LENGTH; // src + dst + totalLgth
      nameOffset += STRUCT_MSG_CMD_HEADER_LENGTH; // cmdKey + cmdLgth
      // randomNum
      nameOffset += sizeof(uint32_t);
      // len ids 
      nameOffset = uint8Decode(decrypted, nameOffset, numNormFields);
      // ids vector
      nameOffset += numNormFields * sizeof(uint16_t);
      // size of name strings (normnamesSize)
      nameOffset = uint16Decode(decrypted, nameOffset, normNamesSize);
      // names vector
      nameOffset += normNamesSize; 
      // definitionSize + nameLgth
      nameOffset += sizeof(uint16_t) + sizeof(uint8_t);
      // here the name starts
      name = decrypted + nameOffset;
      return STRUCT_MSG_ERR_OK;
    },
  
  });

  T.structmsgEncDec = structmsgEncDec;

  T.log("module: "+name+" initialised!", "info", "structmsgEncDec.js");
}, "0.0.1", {});
