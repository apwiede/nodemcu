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
    T.log('constructor called', 'info', 'StructmsgInfo', true);

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

    T.log('constructor end', 'info', 'StructmsgInfo', true);
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
    create: function (numFields) {
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
      return structmsgInfo;
    },

    /* ==================== addField ===================================== */
    addField: function (fieldStr, fieldType, fieldLgth) {
      var structmsgInfo = this;
      return structmsgInfo.msg.addField(structmsgInfo, fieldStr, fieldType, fieldLgth);
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
    
    encode: function() {
print("StructmsgInfo.js encode");
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
    
      structmsgInfo.encoded = null;
      if ((structmsgInfo.flags & structmsgInfo.STRUCT_MSG_HAS_CRC) == 0) {
        result = structmsgInfo.setFillerAndCrc();
        if(result != structmsgInfo.STRUCT_MSG_ERR_OK) return result;
        structmsgInfo.flags |= structmsgInfo.STRUCT_MSG_HAS_CRC;
      }
      //      structmsg.encoded = os_zalloc(structmsg.hdr.hdrInfo.hdrKeys.totalLgth);
      msgPtr = new Object();
      structmsgInfo.encoded = new ArrayBuffer(structmsgInfo.hdr.totalLgth);
print("enc: ",typeof structmsgInfo.encoded,"!",structmsgInfo.encoded.byteLength);
      msgPtr = structmsgInfo.encoded;
print("msgptr: ",typeof msgPtr,"!");
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
print("encode field:",idx,"!",fieldInfo.fieldStr);
        if (fieldInfo.fieldStr[0] == '@') {
          result = structmsg_getFieldNameId(fieldInfo.fieldStr, fieldId, STRUCT_MSG_NO_INCR);
          if(result != structmsgInfo.STRUCT_MSG_ERR_OK) return result;
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
            if (offset < 0) return structmsgInfo.STRUCT_MSG_ERR_ENCODE_ERROR;
            result = stmsg_setFieldValue(handle, "@randomNum", randomNum, NULL);
            if(result != structmsgInfo.STRUCT_MSG_ERR_OK) return result;
            break;
          case STRUCT_MSG_SPEC_FIELD_SEQUENCE_NUM:
            offset = sequenceNumEncode(msgPtr, offset, structmsg, sequenceNum);
            if (offset < 0) return structmsgInfo.STRUCT_MSG_ERR_ENCODE_ERROR;
            result = stmsg_setFieldValue(handle, "@sequenceNum", sequenceNum, NULL);
            if(result != structmsgInfo.STRUCT_MSG_ERR_OK) return result;
            break;
          case STRUCT_MSG_SPEC_FIELD_FILLER:
            offset = fillerEncode(msgPtr, offset, fieldInfo.fieldLgth, fieldInfo.value.ubyteVector);
            if (offset < 0) return structmsgInfo.STRUCT_MSG_ERR_ENCODE_ERROR;
            result = stmsg_setFieldValue(handle, "@filler", 0, fieldInfo.value.ubyteVector);
            if(result != structmsgInfo.STRUCT_MSG_ERR_OK) return result;
            break;
          case STRUCT_MSG_SPEC_FIELD_CRC:
            offset = crcEncode(structmsg.encoded, offset, structmsg.hdr.hdrInfo.hdrKeys.totalLgth, crc, structmsg.hdr.headerLgth);
            if (offset < 0) return structmsgInfo.STRUCT_MSG_ERR_ENCODE_ERROR;
            result = stmsg_setFieldValue(handle, "@crc", crc, NULL);
            if(result != structmsgInfo.STRUCT_MSG_ERR_OK) return result;
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
          offset = encodeField(msgPtr, fieldInfo, offset);
          if (offset < 0) return structmsgInfo.STRUCT_MSG_ERR_ENCODE_ERROR;
        }
        idx++;
      }
      structmsg.flags |= STRUCT_MSG_ENCODED;
      return STRUCT_MSG_ERR_OK;
    },
    
  });

  T.StructmsgInfo = StructmsgInfo;

  T.log("module: "+name+" initialised!", "info", "StructmsgInfo.js");
}, "0.0.1", {});
