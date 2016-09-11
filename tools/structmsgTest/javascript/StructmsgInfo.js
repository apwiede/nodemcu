/*====================================================
 * A structured Message Support implementation in Javascript named Esp
 *
 * Structmsg StructmsgInfo for Esp (StructmsgInfo.js)
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

/*====================================================
 * layout of a StructmsgInfo:
 *
 *  hdr (HeaderInfo)
 *    src
 *    dst
 *    totalLgth
 *    cmdKey
 *    cmdLgth
 *    hdrId (packed info of src+dst+totalLgth+cmdKey+cmdLgth
 *  msg (MsgInfo)
 *    fieldInfos (Array of FieldInfo)
 *      (FieldInfo)
 *        fieldStr (name)
 *        fieldType (type: uint8_t/int8_t/uint16_t/int16_t/......)
 *        fieldLgth
 *        value
 *        flags
 *    tableFieldInfos (Array of FieldInfo)
 *      (FieldInfo)
 *        fieldStr (name)
 *        fieldType (type: uint8_t/int8_t/uint16_t/int16_t/......)
 *        fieldLgth
 *        value
 *        flags (IS_SET)
 *    numFieldInfos
 *    maxFieldInfos
 *    numTableRows         // number of table rows
 *    numTableRowFields    // number of fields within a table row
 *    numRowFields         // for checking how many tableRowFields have been processed
 *  handle
 *  flags (ENCODED/DECODED/ENCRYPTED/DECRYPTED)
 *  encoded
 *  todecode
 *  encrypted
 *  sequenceNum  (is incrmented foreach sent message)
 *  handleHdrInfoPtr
 *
 *====================================================
 */

EM.addModule("Esp-StructmsgInfo", function(T, name) {

  /* ==================== StructmsgInfo constructor ======================= */

  function StructmsgInfo() {
    T.log('constructor called', '2.info', 'StructmsgInfo', true);

    var StmsgInfo = this;

    var constructor = StmsgInfo.constructor;
    StructmsgInfo.superclass.constructor.apply(StmsgInfo, arguments);

    T.Defines.StructmsgInfoOid++;
    StmsgInfo.oid = T.Defines.StructmsgInfoOid;
    StmsgInfo.hdr = null;
    StmsgInfo.msg = null;
    StmsgInfo.handle = null;
    StmsgInfo.flags = 0;
    StmsgInfo.encoded = null;
    StmsgInfo.todecode = null;
    StmsgInfo.encrypted = null;
    StmsgInfo.sequenceNum = 0;
    StmsgInfo.handleHdrInfoPtr = null;
    StmsgInfo.fieldNameInfos = null;

    T.log('constructor end', '2.info', 'StructmsgInfo', true);
  }


  T.extend(StructmsgInfo, T.EncodeDecode, {
     my_name: "StructmsgInfo",
     type_name: "structmsg_info",
     flags: 0,

    /* ==================== toString ===================================== */

    toString: function () {
      var stmsgInfo = this;
      return stmsgInfo.mySelf()+"!"+stmsgInfo.oid+"!";
    },

    /* ==================== toDebugString ===================================== */
    toDebugString: function () {
      var stmsgInfo = this;
      var str = stmsgInfo.toString()+"\n";
      str += "  handle:      "+(stmsgInfo.handle == null ? "null" : stmsgInfo.handle)+"\n";
      str += "  flags:       "+stmsgInfo.flags+"\n";
      str += "  encoded:     "+(stmsgInfo.encoded == null ? "No" : "Yes")+"\n";
      str += "  todecode:    "+(stmsgInfo.todecod == null ? "No" : "Yes")+"\n";
      str += "  encrypted:   "+(stmsgInfo.encrypted == null ? "No" : "Yes")+"\n";
      str += "  sequenceNum: "+stmsgInfo.sequenceNum+"\n";
      str += "  hdr:         "+(stmsgInfo.hdr == null ? "null\n" : stmsgInfo.hdr.toDebugString());
      str += "  msg:         "+(stmsgInfo.msg == null ? "null\n" : stmsgInfo.msg.toDebugString());
      return str;
    },

    /* ==================== create ===================================== */
    create: function (numFields, resultData) {
      var structmsgInfo = this;
      structmsgInfo.hdr = new T.HeaderInfo();
      structmsgInfo.hdr.src = 0;
      structmsgInfo.hdr.dst = 0;
      structmsgInfo.hdr.cmdKey = 0;
      structmsgInfo.hdr.cmdLgth = structmsgInfo.STRUCT_MSG_CMD_HEADER_LENGTH;
      structmsgInfo.hdr.headerLgth = structmsgInfo.STRUCT_MSG_HEADER_LENGTH;
      structmsgInfo.hdr.totalLgth = structmsgInfo.STRUCT_MSG_TOTAL_HEADER_LENGTH;
      structmsgInfo.msg = new T.MsgInfo();
      structmsgInfo.msg.numFieldInfos = 0;
      structmsgInfo.msg.maxFieldInfos = numFields;
      structmsgInfo.msg.numTableRows = 0;
      structmsgInfo.msg.numTableRowFields = 0;
      structmsgInfo.msg.numRowFields = 0;
      structmsgInfo.msg.fieldInfos = new Array(numFields);
      var idx = 0;
      while (idx < numFields) {
        structmsgInfo.msg.fieldInfos[idx] = new T.FieldInfo();
        idx++;
      }
      structmsgInfo.msg.tableFieldInfos = null;
      structmsgInfo.flags = 0;
      structmsgInfo.sequenceNum = 0;
      structmsgInfo.encoded = null;
      structmsgInfo.todecode = null;
      structmsgInfo.encrypted = null;
      structmsgInfo.handleHdrInfoPtr = null;
      T.Defines.numHandles++;
      var s = "0000" + T.Defines.numHandles;
      s = s.substr(s.length-4);
      structmsgInfo.handle = structmsgInfo.HANDLE_PREFIX+'efff'+s;
      structmsgInfo.hdr.fillHdrInfo();
      resultData.data = structmsgInfo;
      return stmsgInfo.STRUCT_MSG_ERR_OK;
    },

    /* ==================== addField ===================================== */
    addField: function (fieldStr, fieldType, fieldLgth) {
      var structmsgInfo = this;
      return structmsgInfo.msg.addField(structmsgInfo, fieldStr, fieldType, fieldLgth);
    },

    /* ==================== setFieldValue ===================================== */
    setFieldValue: function (fieldName, value) {
      var structmsgInfo = this;
      return structmsgInfo.msg.setFieldValue(structmsgInfo, fieldName, value);
    },

    /* ==================== getFieldValue ===================================== */
    getFieldValue: function (fieldName, resultData) {
      var structmsgInfo = this;
      return structmsgInfo.msg.getFieldValue(structmsgInfo, fieldName, resultData);
    },

    /* ==================== setTableFieldValue ===================================== */
    setTableFieldValue: function (fieldName, row, value) {
      var structmsgInfo = this;
      return structmsgInfo.msg.setTableFieldValue(structmsgInfo, fieldName, row, value);
    },

    /* ==================== getTableFieldValue ===================================== */
    getTableFieldValue: function (fieldName, row, resultData) {
      var structmsgInfo = this;
      return structmsgInfo.msg.getTableFieldValue(structmsgInfo, fieldName, row, resultData);
    },

    // ============================= setFillerAndCrc ========================
    
    setFillerAndCrc: function() {
      var structmsgInfo = this;
      var fillerLgth = 0;
      var myLgth = 0;
      var result;
    
      structmsgInfo.hdr.cmdLgth++;
      structmsgInfo.hdr.totalLgth++;
      // end space for the numEntries field!!
      myLgth = structmsgInfo.hdr.cmdLgth + 2;
      while ((myLgth % 16) != 0) {
        myLgth++;
        fillerLgth++;
      }
      result = structmsgInfo.addField("@filler", "uint8_t*", fillerLgth);
      if(result != structmsgInfo.STRUCT_MSG_ERR_OK) return result;
      result = structmsgInfo.addField("@crc", "uint16_t", 2);
      if(result != structmsgInfo.STRUCT_MSG_ERR_OK) return result;
      return structmsgInfo.STRUCT_MSG_ERR_OK;
    },
    
    // ============================= encode ========================
    
    encode: function(result_data) {
      var structmsgInfo = this;
      var crc;
      var offset;
      var idx;
      var fieldIdx;
      var numEntries;
      var result;
      var fieldId = 0;
      var fieldInfo;
      var msgPtr;
    
print("======== StructmsgInfo ENCODE");
      structmsgInfo.encoded = null;
      if ((structmsgInfo.flags & structmsgInfo.STRUCT_MSG_HAS_CRC) == 0) {
        result = structmsgInfo.setFillerAndCrc();
        if(result != structmsgInfo.STRUCT_MSG_ERR_OK) return result;
        structmsgInfo.flags |= structmsgInfo.STRUCT_MSG_HAS_CRC;
      }
      //      structmsg.encoded = os_zalloc(structmsg.hdr.hdrInfo.hdrKeys.totalLgth);
      structmsgInfo.encoded = new ArrayBuffer(structmsgInfo.hdr.totalLgth);
      msgPtr = structmsgInfo.encoded;
      offset = 0;
      offset = structmsgInfo.uint16Encode(msgPtr,offset,structmsgInfo.hdr.src);
      if (offset < 0) return structmsgInfo.STRUCT_MSG_ERR_ENCODE_ERROR;
      offset = structmsgInfo.uint16Encode(msgPtr,offset,structmsgInfo.hdr.dst);
      if (offset < 0) return structmsgInfo.STRUCT_MSG_ERR_ENCODE_ERROR;
      offset = structmsgInfo.uint16Encode(msgPtr,offset,structmsgInfo.hdr.totalLgth);
      if (offset < 0) return structmsgInfo.STRUCT_MSG_ERR_ENCODE_ERROR;
      offset = structmsgInfo.uint16Encode(msgPtr,offset,structmsgInfo.hdr.cmdKey);
      if (offset < 0) return structmsgInfo.STRUCT_MSG_ERR_ENCODE_ERROR;
      offset = structmsgInfo.uint16Encode(msgPtr,offset,structmsgInfo.hdr.cmdLgth);
      if (offset < 0) return structmsgInfo.STRUCT_MSG_ERR_ENCODE_ERROR;
      numEntries = structmsgInfo.msg.numFieldInfos;
      offset = structmsgInfo.uint8Encode(msgPtr,offset,numEntries);
      if (offset < 0) return structmsgInfo.STRUCT_MSG_ERR_ENCODE_ERROR;
      idx = 0;
      while (idx < numEntries) {
        fieldInfo = structmsgInfo.msg.fieldInfos[idx];
        if (fieldInfo.fieldStr[0] == '@') {
          var obj = new Object();
          obj.fieldId = -1;
          result = structmsgInfo.fieldNameInfos.getFieldNameId(fieldInfo.fieldStr, obj, structmsgInfo.STRUCT_MSG_NO_INCR);
          if(result != structmsgInfo.STRUCT_MSG_ERR_OK) return result;
          switch (obj.fieldId) {
          case structmsgInfo.STRUCT_MSG_SPEC_FIELD_SRC:
          case structmsgInfo.STRUCT_MSG_SPEC_FIELD_DST:
          case structmsgInfo.STRUCT_MSG_SPEC_FIELD_TARGET_CMD:
          case structmsgInfo.STRUCT_MSG_SPEC_FIELD_TOTAL_LGTH:
          case structmsgInfo.STRUCT_MSG_SPEC_FIELD_CMD_KEY:
          case structmsgInfo.STRUCT_MSG_SPEC_FIELD_CMD_LGTH:
EM.log('encode: funny should encode: '+fieldInfo.fieldStr, 'error', "StructmsgInfo.js", true);
            break;
          case structmsgInfo.STRUCT_MSG_SPEC_FIELD_RANDOM_NUM:
            obj.value = 0;
            offset = structmsgInfo.randomNumEncode(msgPtr, offset, obj);
            if (offset < 0) return structmsgInfo.STRUCT_MSG_ERR_ENCODE_ERROR;
            result = structmsgInfo.msg.setFieldValue(handle, "@randomNum", obj.value, null);
            if(result != structmsgInfo.STRUCT_MSG_ERR_OK) return result;
            break;
          case structmsgInfo.STRUCT_MSG_SPEC_FIELD_SEQUENCE_NUM:
            obj.value = 0;
            offset = structmsgInfo.sequenceNumEncode(msgPtr, offset, structmsgInfo,obj);
            if (offset < 0) return structmsgInfo.STRUCT_MSG_ERR_ENCODE_ERROR;
            result = structmsgInfo.msg.setFieldValue(handle, "@sequenceNum", obj.value, null);
            if(result != structmsgInfo.STRUCT_MSG_ERR_OK) return result;
            break;
          case structmsgInfo.STRUCT_MSG_SPEC_FIELD_FILLER:
            offset = structmsgInfo.fillerEncode(msgPtr, offset, fieldInfo.fieldLgth, obj);
            if (offset < 0) return structmsgInfo.STRUCT_MSG_ERR_ENCODE_ERROR;
            fieldInfo.value = obj.value;
            result = structmsgInfo.msg.setFieldValue(handle, "@filler", 0, fieldInfo.value);
            if(result != structmsgInfo.STRUCT_MSG_ERR_OK) return result;
            break;
          case structmsgInfo.STRUCT_MSG_SPEC_FIELD_CRC:
            obj.value = 0;
            offset = structmsgInfo.crcEncode(msgPtr, offset, structmsgInfo.hdr.totalLgth, obj, structmsgInfo.hdr.headerLgth);
            if (offset < 0) return structmsgInfo.STRUCT_MSG_ERR_ENCODE_ERROR;
            crc = obj.value;
            result = structmsgInfo.msg.setFieldValue(handle, "@crc", crc, null);
            if(result != structmsgInfo.STRUCT_MSG_ERR_OK) return result;
            break;
          case structmsgInfo.STRUCT_MSG_SPEC_FIELD_ID:
            return structmsgInfo.STRUCT_MSG_ERR_BAD_SPECIAL_FIELD;
            break;
          case structmsgInfo.STRUCT_MSG_SPEC_FIELD_TABLE_ROWS:
            break;
          case structmsgInfo.STRUCT_MSG_SPEC_FIELD_TABLE_ROW_FIELDS:
            if (structmsgInfo.msg.numTableRows > 0) {
              var row = 0;
              var col = 0;
              var cell = 0;
              while (row < structmsgInfo.msg.numTableRows) {
    	        while (col < structmsgInfo.msg.numRowFields) {
    	           cell = col + row * structmsgInfo.msg.numRowFields;
    	           fieldInfo = structmsgInfo.msg.tableFieldInfos[cell];
    	           offset = structmsgInfo.encodeField(msgPtr, fieldInfo, offset);
                   if (offset < 0) return structmsgInfo.STRUCT_MSG_ERR_ENCODE_ERROR;
    	           col++;
    	        }
    	        row++;
    	        col = 0;
              }
            }
            break;
          }
        } else {
          offset = structmsgInfo.encodeField(msgPtr, fieldInfo, offset);
          if (offset < 0) return structmsgInfo.STRUCT_MSG_ERR_ENCODE_ERROR;
        }
        idx++;
      }
      result_data.data = msgPtr;
      structmsgInfo.flags |= structmsgInfo.STRUCT_MSG_ENCODED;
      return structmsgInfo.STRUCT_MSG_ERR_OK;
    },
    
    // ============================= setEncoded ========================
    
    setEncoded: function(data) {
      var structmsgInfo = this;

      structmsgInfo.encoded = data;
      return structmsgInfo.STRUCT_MSG_ERR_OK;
    },

    // ============================= getEncoded ========================
    
    getEncoded: function(resultData) {
      var structmsgInfo = this;

      resultData.data = structmsgInfo.encoded;
      return structmsgInfo.STRUCT_MSG_ERR_OK;
    },

    // ============================= decode ========================
    
    decode: function() {
      var structmsgInfo = this;
      var msgPtr;
      var crc;
      var offset;
      var idx;
      var fieldIdx;
      var fieldId;
      var numEntries;
      var result = structmsgInfo.STRUCT_MSG_ERR_OK;
      var fieldInfo;
    
      var obj = new Object();
      obj.value = "";
      msgPtr = structmsgInfo.todecode;
      if (msgPtr == null) {
        // try with encoded
        msgPtr = structmsgInfo.encoded;
      }
      if (msgPtr == null) {
        return structmsgInfo.STRUCT_MSG_ERR_DECODE_ERROR;
      }
      offset = structmsgInfo.uint16Decode(msgPtr,4,obj);
      structmsgInfo.todecode = new ArrayBuffer(structmsgInfo.hdr.totalLgth);
      offset = 0;
      offset = structmsgInfo.uint16Decode(msgPtr,offset,obj);
      if (offset < 0) return structmsgInfo.STRUCT_MSG_ERR_DECODE_ERROR;
      structmsgInfo.hdr.src = obj.value;
      offset = structmsgInfo.uint16Decode(msgPtr,offset,obj);
      if (offset < 0) return structmsgInfo.STRUCT_MSG_ERR_DECODE_ERROR;
      structmsgInfo.hdr.dst = obj.value;
      offset = structmsgInfo.uint16Decode(msgPtr,offset,obj);
      if (offset < 0) return structmsgInfo.STRUCT_MSG_ERR_DECODE_ERROR;
      structmsgInfo.hdr.totalLgth = obj.value;
      offset = structmsgInfo.uint16Decode(msgPtr,offset,obj);
      if (offset < 0) return structmsgInfo.STRUCT_MSG_ERR_DECODE_ERROR;
      structmsgInfo.hdr.cmdKey = obj.value;
      offset = structmsgInfo.uint16Decode(msgPtr,offset,obj);
      if (offset < 0) return structmsgInfo.STRUCT_MSG_ERR_DECODE_ERROR;
      structmsgInfo.hdr.cmdLgth = obj.value;
      result = structmsgInfo.hdr.fillHdrInfo(handle, structmsgInfo);
      if(result != structmsgInfo.STRUCT_MSG_ERR_OK) return result;
      offset = structmsgInfo.uint8Decode(msgPtr,offset,obj);
      if (offset < 0) return structmsgInfo.STRUCT_MSG_ERR_DECODE_ERROR;
      numEntries = obj.value;
      idx = 0;
//print("msg: ",structmsgInfo.msg.toDebugString());
      while (idx < numEntries) {
        fieldInfo = structmsgInfo.msg.fieldInfos[idx];
        if (fieldInfo.fieldStr[0] == '@') {
          result = structmsgInfo.fieldNameInfos.getFieldNameId(fieldInfo.fieldStr, obj, structmsgInfo.STRUCT_MSG_NO_INCR);
          if(result != structmsgInfo.STRUCT_MSG_ERR_OK) return result;
          fieldId = obj.fieldId;
          switch (fieldId) {
          case structmsgInfo.STRUCT_MSG_SPEC_FIELD_SRC:
          case structmsgInfo.STRUCT_MSG_SPEC_FIELD_DST:
          case structmsgInfo.STRUCT_MSG_SPEC_FIELD_TARGET_CMD:
          case structmsgInfo.STRUCT_MSG_SPEC_FIELD_TOTAL_LGTH:
          case structmsgInfo.STRUCT_MSG_SPEC_FIELD_CMD_KEY:
          case structmsgInfo.STRUCT_MSG_SPEC_FIELD_CMD_LGTH:
EM.log('decode: funny should decode: '+fieldInfo.fieldStr, 'error', "StructmsgInfo.js", true);
            return structmsgInfo.STRUCT_MSG_ERR_DECODE_ERROR;
            break;
          case structmsgInfo.STRUCT_MSG_SPEC_FIELD_RANDOM_NUM:
            offset = structmsgInfo.randomNumDecode(msgPtr, offset, fieldInfo);
            if (offset < 0) return structmsgInfo.STRUCT_MSG_ERR_DECODE_ERROR;
            break;
          case structmsgInfo.STRUCT_MSG_SPEC_FIELD_SEQUENCE_NUM:
            offset = structmsgInfo.sequenceNumDecode(msgPtr, offset, fieldInfo);
            if (offset < 0) return structmsgInfo.STRUCT_MSG_ERR_DECODE_ERROR;
            break;
          case structmsgInfo.STRUCT_MSG_SPEC_FIELD_FILLER:
            offset = structmsgInfo.fillerDecode(msgPtr, offset, fieldInfo.fieldLgth, fieldInfo);
            if (offset < 0) return structmsgInfo.STRUCT_MSG_ERR_DECODE_ERROR;
            break;
          case structmsgInfo.STRUCT_MSG_SPEC_FIELD_CRC:
            offset = structmsgInfo.crcDecode(msgPtr, offset, structmsgInfo.hdr.cmdLgth, fieldInfo, structmsgInfo.hdr.headerLgth);
            if (offset < 0) {
              return structmsgInfo.STRUCT_MSG_ERR_BAD_CRC_VALUE;
            }
            break;
          case structmsgInfo.STRUCT_MSG_SPEC_FIELD_ID:
            return structmsgInfo.STRUCT_MSG_ERR_BAD_SPECIAL_FIELD;
            break;
          case structmsgInfo.STRUCT_MSG_SPEC_FIELD_TABLE_ROWS:
            break;
          case structmsgInfo.STRUCT_MSG_SPEC_FIELD_TABLE_ROW_FIELDS:
            if (structmsgInfo.msg.numTableRows > 0) {
              var row = 0;
              var col = 0;
              var cell = 0;
//print(structmsgInfo.msg.toDebugString());
              while (row < structmsgInfo.msg.numTableRows) {
    	        while (col < structmsgInfo.msg.numRowFields) {
    	           cell = col + row * structmsgInfo.msg.numRowFields;
    	           fieldInfo = structmsgInfo.msg.tableFieldInfos[cell];
    	           offset = stmsgInfo.decodeField(msgPtr, fieldInfo, offset);
                   if (offset < 0) return structmsgInfo.STRUCT_MSG_ERR_DECODE_ERROR;
    	           col++;
    	        }
    	        row++;
    	        col = 0;
              }
            }
            break;
          }
          fieldInfo.flags |= structmsgInfo.STRUCT_MSG_FIELD_IS_SET;
        } else {
          offset = structmsgInfo.decodeField(msgPtr, fieldInfo, offset);
          if (offset < 0) return structmsgInfo.STRUCT_MSG_ERR_DECODE_ERROR;
          fieldInfo.flags |= structmsgInfo.STRUCT_MSG_FIELD_IS_SET;
        }
        idx++;
      }
      structmsgInfo.flags |= structmsgInfo.STRUCT_MSG_DECODED;
      return structmsgInfo.STRUCT_MSG_ERR_OK;
    },
    
    // ============================= dump ========================
    
    dump: function() {
      var structmsgInfo = this;

print("dump");
      T.log("dump msg: "+structmsgInfo.toDebugString(), "0.info", "StructmsgInfo.js");
      return structmsgInfo.STRUCT_MSG_ERR_OK;
    },

  });

  T.StructmsgInfo = StructmsgInfo;

  T.log("module: "+name+" initialised!", "2.info", "StructmsgInfo.js");
}, "0.0.1", {});
