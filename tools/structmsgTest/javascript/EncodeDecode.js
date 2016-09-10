/*====================================================
 * A structured Message Support implementation in Javascript named Esp
 *
 * Structmsg EncodeDecode for Esp (EncodeDecode.js)
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

EM.addModule("Esp-EncodeDecode", function(T, name) {

  /* ==================== EncodeDecode constructor ======================= */

  function EncodeDecode() {
    T.log('constructor called', 'info', 'EncodeDecode', true);

    var encDec = this;
    var constructor = encDec.constructor;
    EncodeDecode.superclass.constructor.apply(encDec, arguments);

    T.log('constructor end', 'info', 'EncodeDecode', true);
  }

  T.extend(EncodeDecode, T.Defines, {
    my_name: "EncodeDecode",
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
      var dv = new DataView(data);

      dv.setUint8(offset, value);
      return offset + 1;
    },
    
    // ============================= int8Encode ========================
    
    int8Encode: function(data, offset, value) {
      var dv = new DataView(data);

      dv.setInt8(offset, value);
      return offset + 1;
    },
    
    // ============================= uint16Encode ========================
    
    uint16Encode: function(data, offset, value) {
      var dv = new DataView(data);

      dv.setUint16(offset, value);
      return offset+2;
    },
    
    // ============================= int16Encode ========================
    
    int16Encode: function(data, offset, value) {
      var dv = new DataView(data);

      dv.setInt16(offset, value);
      return offset + 2;
    },
    
    // ============================= uint32Encode ========================
    
    uint32Encode: function(data, offset, value) {
      var dv = new DataView(data);

      dv.setUint32(offset, value);
      return offset + 4;
    },
    
    // ============================= int32Encode ========================
    
    int32Encode: function(data, offset, value) {
      var dv = new DataView(data);

      dv.setInt32(offset, value);
      return offset + 4;
    },
    
    // ============================= uint8VectorEncode ========================
    
    uint8VectorEncode: function(data, offset, value, lgth) {
      var dv = new DataView(data);
      var idx;

      idx = 0;
      while (idx < lgth) {
        dv.setUint8(offset, value[idx]);
        idx++;
        offset++;
      }
      return offset;
    },
    
    // ============================= int8VectorEncode ========================
    
    int8VectorEncode: function(data, offset, value, lgth) {
      var dv = new DataView(data);

      idx = 0;
      while (idx < lgth) {
        dv.setInt8(offset, value[idx]);
        idx++;
        offset++;
      }
      return offset;
    },
    
    // ============================= uint16VectorEncode ========================
    
    uint16VectorEncode: function(data, offset, value, lgth) {
      var dv = new DataView(data);
      var idx;
    
      idx = 0;
      while (idx < lgth) {
        dv.setUint16(offset, value[idx]);
        idx++;
        offset += 2;
      }
      return offset;
    },
    
    // ============================= int16VectorEncode ========================
    
    int16VectorEncode: function(data, offset, value, lgth) {
      var dv = new DataView(data);
      var idx;
    
      idx = 0;
      while (idx < lgth) {
        dv.setInt16(offset, value[idx]);
        idx++;
        offset += 2;
      }
      return offset;
    },
    
    // ============================= uint32VectorEncode ========================
    
    uint32VectorEncode: function(data, offset, value, lgth) {
      var dv = new DataView(data);
      var idx;
    
      idx = 0;
      while (idx < lgth) {
        dv.setUint32(offset, value[idx]);
        idx++;
        offset += 4;
      }
      return offset;
    },
    
    // ============================= int32VectorEncode ========================
    
    int32VectorEncode: function(data, offset, value, lgth) {
      var dv = new DataView(data);
      var idx;
    
      idx = 0;
      while (idx < lgth) {
        dv.setInt32(offset, value[idx]);
        idx++;
        offset += 4;
      }
      return offset;
    },
    
    // ============================= uint8Decode ========================
    
    uint8Decode: function(data, offset, result) {
      var dv = new DataView(data);

      result.value = dv.getUint8(offset);
      offset++;
      return offset;
    },
    
    // ============================= int8Decode ========================
    
    int8Decode: function(data, offset, result) {
      var dv = new DataView(data);

      result.value = dv.getInt8(offset);
      offset++;
      return offset;
    },
    
    // ============================= uint16Decode ========================
    
    uint16Decode: function(data, offset, result) {
      var dv = new DataView(data);

      result.value = dv.getUint16(offset);
      offset += 2;
      return offset;
    },
    
    // ============================= int16Decode ========================
    
    int16Decode: function(data, offset, result) {
      var dv = new DataView(data);

      result.value = dv.getInt16(offset);
      offset += 2;
      return offset;
    },
    
    // ============================= uint32Decode ========================
    
    uint32Decode: function(data, offset, result) {
      var dv = new DataView(data);

      result.value = dv.getUint32(offset);
      offset += 4;
      return offset;
    },
    
    // ============================= int32Decode ========================
    
    int32Decode: function(data, offset, result) {
      var dv = new DataView(data);

      result.value = dv.getInt32(offset);
      offset += 4;
      return offset;
    },
    
    // ============================= uint8VectorDecode ========================
    
    uint8VectorDecode: function(data, offset, lgth, result) {
      var dv = new DataView(data);
      var idx;
    
      idx = 0;
      result.value = ""
      while (idx < lgth) {
        result.value += dv.getUint8(offset);
        idx++;
        offset++;
      }
      return offset;
    },
    
    // ============================= int8VectorDecode ========================
    
    int8VectorDecode: function(data, offset, lgth, value) {
      var dv = new DataView(data);
      var idx;
    
      idx = 0;
      result.value = ""
      while (idx < lgth) {
        result.value += dv.getInt8(offset);
        idx++;
        offset++;
      }
    },
    
    // ============================= uint16VectorDecode ========================
    
    uint16VectorDecode: function(data, offset, lgth, value) {
      var dv = new DataView(data);

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
      var dv = new DataView(data);

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
      var dv = new DataView(data);

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
      var dv = new DataView(data);

      var idx;
    
      idx = 0;
      while (idx < lgth) {
        offset = int32Decode(data, offset, value[idx]);
        idx++;
      }
      return offset;
    },
    
    // ============================= getRandomNum ========================
    
    getRandomNum: function() {
      return Number(parseInt(String.substr(Math.random().toString(),2)));
    },

    // ============================= randomNumEncode ========================
    
    randomNumEncode: function(data, offset, result) {
      var encDec = this;
      var dv = new DataView(data);
      var val;
    
      val = encDec.getRandomNum() & 0xFFFFFFFF;
      dv.setUint32(offset, val);
      result.value = dv.getUint32(offset).toString(16);
      offset += 4;
      return offset;
    },
    
    // ============================= randomNumDecode ========================
    
    randomNumDecode: function(data, offset, result) {
      var dv = new DataView(data);
    
      result.value = dv.getUint32(data, offset);
      offset += 4;
      return offset;
    },
    
    // ============================= sequenceNumEncode ========================
    
    sequenceNumEncode: function(data, offset, structmsg, result) {
      var dv = new DataView(data);
      var val;
    
      val = ++structmsg.sequenceNum;
      dv.setUint32(offset, val);
      result.value = dv.getUint32(offset).toString(16);
      offset += 4;
      return offset;
    },
    
    // ============================= sequenceNumDecode ========================
    
    sequenceNumDecode: function(data, offset, result) {
      var dv = new DataView(data);
    
      result.value = dv.getUint32(data, offset);
      offset += 4;
      return offset;
    },
    
    // ============================= fillerEncode ========================
    
    fillerEncode: function(data, offset, lgth, result) {
      var encDec = this;
      var dv = new DataView(data);
      var val;
      var idx;
    

      idx = 0;
      result.value = "";
      while (lgth >= 4) {
        val = encDec.getRandomNum() & 0xFFFFFFFF;
        dv.setUint32(offset, val);
        result.value += dv.getUint32(offset).toString(16);
        offset += 4;
        lgth -= 4;
      }
      while (lgth >= 2) {
        val = encDec.getRandomNum() & 0xFFFF;
        dv.setUint16(offset, val);
        result.value += dv.getUint16(offset).toString(16);
        offset += 2;
        lgth -= 2;
      }
      while (lgth >= 1) {
        val = encDec.getRandomNum() & 0xFF;
        dv.setUint8(offset, val);
        result.value += dv.getUint8(offset).toString(16);
        offset += 1;
        lgth -= 1;
      }
print("Fill: ",result.value);
      return offset;
    },
    
    // ============================= fillerDecode ========================
    
    fillerDecode: function(data, offset, lgth, result) {
      var dv = new DataView(data);
      var idx;
    
      result.value = ""
      while (idx < lgth) {
        result.value += dv.getUint8(data,offset);
        offset++;
      }
      return offset;
    },
    
    // ============================= crcEncode ========================
    
    crcEncode: function(data, offset, lgth, result, headerLgth) {
      var encDec = this;
      var dv = new DataView(data);
      var idx;
      var val;
    
// FIXME!!
//      lgth -= encDec.sizeof("uint16_t");   // uint16_t crc
      lgth -= 2;   // uint16_t crc
      crc = 0;
      idx = headerLgth;
//FIXME!!
if (0) {
      while (idx < lgth) {
        val = dv.getUint8(idx);
print("crc idx:",idx-headerLgth," val: ", val.toString(16), " crc: ", crc.toString(16));
        crc += data[idx++];
      }
      crc = ~(crc);
}
//      dv.setUint16(data,offset,crc);
      dv.setUint16(data,offset,0xfed8);
result.value = 0xfed8;
      offset += 2;
      return offset;
    },
    
    // ============================= crcDecode ========================
    
    crcDecode: function(data, offset, lgth, crc, headerLgth) {
      var dv = new DataView(data);
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
    
    // ============================= encodeField ========================
    
    encodeField: function(msgPtr, fieldInfo, offset) {
      var encDec = this;
      var fieldIdx;
    
      switch (fieldInfo.fieldType) {
      case encDec.STRUCT_MSG_FIELD_INT8_T:
        offset = encDec.int8Encode(msgPtr,offset,fieldInfo.value);
        if (offset < 0) return encDec.STRUCT_MSG_ERR_ENCODE_ERROR;
        break;
      case encDec.STRUCT_MSG_FIELD_UINT8_T:
        offset = encDec.uint8Encode(msgPtr,offset,fieldInfo.value);
        if (offset < 0) return encDec.STRUCT_MSG_ERR_ENCODE_ERROR;
        break;
      case encDec.STRUCT_MSG_FIELD_INT16_T:
        offset = encDec.int16Encode(msgPtr,offset,fieldInfo.value);
        if (offset < 0) return encDec.STRUCT_MSG_ERR_ENCODE_ERROR;
        break;
      case encDec.STRUCT_MSG_FIELD_UINT16_T:
        offset = encDec.uint16Encode(msgPtr,offset,fieldInfo.value);
        if (offset < 0) return encDec.STRUCT_MSG_ERR_ENCODE_ERROR;
        break;
      case encDec.STRUCT_MSG_FIELD_INT32_T:
        offset = encDec.int32Encode(msgPtr,offset,fieldInfo.value);
        if (offset < 0) return encDec.STRUCT_MSG_ERR_ENCODE_ERROR;
        break;
      case encDec.STRUCT_MSG_FIELD_UINT32_T:
        offset = encDec.uint32Encode(msgPtr,offset,fieldInfo.valuel);
        if (offset < 0) return encDec.STRUCT_MSG_ERR_ENCODE_ERROR;
        break;
      case encDec.STRUCT_MSG_FIELD_INT8_VECTOR:
        fieldIdx = 0;
        while (fieldIdx < fieldInfo.fieldLgth) {
          offset = encDec.int8Encode(msgPtr,offset,fieldInfo.value[fieldIdx]);
          if (offset < 0) return encDec.STRUCT_MSG_ERR_ENCODE_ERROR;
          fieldIdx++;
        }
        break;
      case encDec.STRUCT_MSG_FIELD_UINT8_VECTOR:
        fieldIdx = 0;
        while (fieldIdx < fieldInfo.fieldLgth) {
          offset = encDec.uint8Encode(msgPtr,offset,fieldInfo.value[fieldIdx]);
          if (offset < 0) return encDec.STRUCT_MSG_ERR_ENCODE_ERROR;
          fieldIdx++;
        }
        break;
      case encDec.STRUCT_MSG_FIELD_INT16_VECTOR:
        fieldIdx = 0;
        while (fieldIdx < fieldInfo.fieldLgth) {
          offset = encDec.int16Encode(msgPtr,offset,fieldInfo.value[fieldIdx]);
          if (offset < 0) return encDec.STRUCT_MSG_ERR_ENCODE_ERROR;
          fieldIdx++;
        }
        break;
      case encDec.STRUCT_MSG_FIELD_UINT16_VECTOR:
        fieldIdx = 0;
        while (fieldIdx < fieldInfo.fieldLgth) {
          offset = encDec.uint16Encode(msgPtr,offset,fieldInfo.value[fieldIdx]);
          if (offset < 0) return encDec.STRUCT_MSG_ERR_ENCODE_ERROR;
          fieldIdx++;
        }
        break;
      case encDec.STRUCT_MSG_FIELD_INT32_VECTOR:
        fieldIdx = 0;
        while (fieldIdx < fieldInfo.fieldLgth) {
          offset = encDec.int32Encode(msgPtr,offset,fieldInfo.value[fieldIdx]);
          if (offset < 0) return encDec.STRUCT_MSG_ERR_ENCODE_ERROR;
          fieldIdx++;
        }
        break;
      case encDec.STRUCT_MSG_FIELD_UINT32_VECTOR:
        fieldIdx = 0;
        while (fieldIdx < fieldInfo.fieldLgth) {
          offset = encDec.uint32Encode(msgPtr,offset,fieldInfo.value[fieldIdx]);
          if (offset < 0) return encDec.STRUCT_MSG_ERR_ENCODE_ERROR;
          fieldIdx++;
        }
        break;
      }
      return offset;
    },
    
    // ============================= decodeField ========================
    
    decodeField: function(msgPtr, fieldInfo,  offset) {
      var encDec = this;
      var fieldIdx;
    
      switch (fieldInfo.fieldType) {
      case encDec.STRUCT_MSG_FIELD_INT8_T:
        offset = encDec.int8Decode(msgPtr,offset,fieldInfo);
        if (offset < 0) return encDec.STRUCT_MSG_ERR_DECODE_ERROR;
        break;
      case encDec.STRUCT_MSG_FIELD_UINT8_T:
        offset = encDec.uint8Decode(msgPtr,offset,fieldInfo);
        if (offset < 0) return encDec.STRUCT_MSG_ERR_DECODE_ERROR;
        break;
      case encDec.STRUCT_MSG_FIELD_INT16_T:
        offset = encDec.int16Decode(msgPtr,offset,fieldInfo);
        if (offset < 0) return encDec.STRUCT_MSG_ERR_DECODE_ERROR;
        break;
      case encDec.STRUCT_MSG_FIELD_UINT16_T:
        offset = encDec.uint16Decode(msgPtr,offset,fieldInfo);
        if (offset < 0) return encDec.STRUCT_MSG_ERR_DECODE_ERROR;
        break;
      case encDec.STRUCT_MSG_FIELD_INT32_T:
        offset = encDec.int32Decode(msgPtr,offset,fieldInfo);
        if (offset < 0) return encDec.STRUCT_MSG_ERR_DECODE_ERROR;
        break;
      case encDec.STRUCT_MSG_FIELD_UINT32_T:
        offset = encDec.uint32Decode(msgPtr,offset,fieldInfo);
        if (offset < 0) return encDec.STRUCT_MSG_ERR_DECODE_ERROR;
        break;
      case encDec.STRUCT_MSG_FIELD_INT8_VECTOR:
        offset = encDec.int8VectorDecode(msgPtr,offset,fieldInfo.fieldLgth, fieldInfo);
        if (offset < 0) return encDec.STRUCT_MSG_ERR_DECODE_ERROR;
        break;
      case encDec.STRUCT_MSG_FIELD_UINT8_VECTOR:
        offset = encDec.uint8VectorDecode(msgPtr,offset,fieldInfo.fieldLgth, fieldInfo);
        if (offset < 0) return encDec.STRUCT_MSG_ERR_DECODE_ERROR;
        break;
      case encDec.STRUCT_MSG_FIELD_INT16_VECTOR:
        offset = encDec.int16VectorDecode(msgPtr,offset,fieldInfo.fieldLgth, fieldInfo);
        if (offset < 0) return encDec.STRUCT_MSG_ERR_DECODE_ERROR;
        break;
      case encDec.STRUCT_MSG_FIELD_UINT16_VECTOR:
        offset = encDec.uint16VectorDecode(msgPtr,offset,fieldInfo.fieldLgth, fieldInfo);
        if (offset < 0) return encDec.STRUCT_MSG_ERR_DECODE_ERROR;
        break;
      case encDec.STRUCT_MSG_FIELD_INT32_VECTOR:
        offset = encDec.int32VectorDecode(msgPtr,offset,fieldInfo.fieldLgth, fieldInfo);
        if (offset < 0) return encDec.STRUCT_MSG_ERR_DECODE_ERROR;
        break;
      case encDec.STRUCT_MSG_FIELD_UINT32_VECTOR:
        offset = encDec.uint32VectorDecode(msgPtr,offset,fieldInfo.fieldLgth, fieldInfo);
        if (offset < 0) return encDec.STRUCT_MSG_ERR_DECODE_ERROR;
        break;
      }
      return offset;
    },
    
  });

  T.EncodeDecode = EncodeDecode;

  T.log("module: "+name+" initialised!", "info", "EncodeDecode.js");
}, "0.0.1", {});
