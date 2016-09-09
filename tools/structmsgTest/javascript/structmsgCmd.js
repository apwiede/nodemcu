/*====================================================
 * A structured Message Support implementation in Javascript named Esp
 *
 * Structmsg Cmd for Esp (structmsgCmd.js)
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

EM.addModule("Esp-structmsgCmd", function(T, name) {

  /* ==================== Cmd constructor ======================= */

  function Cmd() {
    T.log('constructor called', 'info', 'structmsgCmd', true);

    var cmd = this;
    var constructor = cmd.constructor;
    Cmd.superclass.constructor.apply(cmd, arguments);

    T.log('constructor end', 'info', 'structmsgCmd', true);
  }

  T.extend(Cmd, T.StructmsgInfos, {
     my_name: "Cmd",
     type_name: "cmd",
     flags: 0,

    /* ==================== toString ===================================== */

    toString: function () {
      var cmd = this;
      return cmd.mySelf()+"!";
    },

    /* ==================== toDebugString ===================================== */
    toDebugString: function () {
      var cmd = this;
      var str = cmd.mySelf()+"\n";
      return str;
    },

    // ============================= checkHandle ========================
    
    checkHandle: function(handle) {
      var idx;
    
      if (structmsg_userdata.handles == null) {
        return STRUCT_MSG_ERR_HANDLE_NOT_FOUND;
      }
      idx = 0;
      while (idx < structmsg_userdata.numHandles) {
        if ((structmsg_userdata.handles[idx].handle != null) && (structmsg_userdata.handles[idx].handle == handle)) {
          return STRUCT_MSG_ERR_OK;
        }
        idx++;
      }
      return STRUCT_MSG_ERR_HANDLE_NOT_FOUND;
    },
    
    // ============================= structmsg_get_structmsg_ptr ========================
    
    structmsg_get_structmsg_ptr: function(handle) {
      var id;
      var hex = "0123456789abcdef\0";
      var num;
      var len;
      var shift;
      var cp;
      var idx;
    
      if (checkHandle(handle) != STRUCT_MSG_ERR_OK) {
        return null;
      }
      handle += HANDLE_PREFIX.length; /* skip "sms_" */
      num = 0;
      id = 0;
      len = handle.length;
      shift = 28;
      while (num < len) {
        var ch = handle[num];
        cp = hex;
        idx = 0;
        while (hex[idx] != 0) {
          if (hex[idx] == ch) {
              break;
          }
          idx++;
        }
        id = id + (idx << shift);
        shift -= 4;
        num++;
      }
      return id;
    },
    
    // ============================= structmsg_dumpBinary ========================
    
    structmsg_dumpBinary: function(data, lgth, where) {
      var idx;
    
// FIXME use T.log
//      print(where);
      idx = 0;
      while (idx < lgth) {
//         print('idx: ',idx,' ch: ', (data[idx] & 0xFF).toString(16));
        idx++;
      }
    },
    
    // ============================= getHandle ========================
    
    getHandle: function(hdrkey, handle) {
      var idx;
    
      handle = null;
      if (structmsg_userdata.handles == null) {
        return STRUCT_MSG_ERR_HANDLE_NOT_FOUND;
      } else {
        idx = 0;
        while (idx < structmsg_userdata.numHandles) {
          if (structmsg_userdata.handles[idx].handle != null) {
            if (structmsg_userdata.handles[idx].hdrInfo.hdrId == hdrkey) {
              handle = structmsg_userdata.handles[idx].handle;
              return STRUCT_MSG_ERR_OK;
            }
          }
          idx++;
        }
      }
      return STRUCT_MSG_ERR_HANDLE_NOT_FOUND;
    },
    
    // ============================= addHandle ========================
    
    addHandle: function(handle, hdrInfo) {
      var idx;
    
      if (structmsg_userdata.handles == null) {
//        structmsg_userdata.handles = os_malloc(sizeof(handle2Hdr_t));
        if (structmsg_userdata.handles == null) {
          return STRUCT_MSG_ERR_OUT_OF_MEMORY;
        } else {
          structmsg_userdata.handles[structmsg_userdata.numHandles].handle = handle;
          hdrInfo = structmsg_userdata.handles[structmsg_userdata.numHandles++].hdrInfo;
          return STRUCT_MSG_ERR_OK;
        }
      } else {
        // check for unused slot first
        idx = 0;
        while (idx < structmsg_userdata.numHandles) {
          if (structmsg_userdata.handles[idx].handle == null) {
            structmsg_userdata.handles[idx].handle = handle;
            hdrInfo = structmsg_userdata.handles[idx].hdrInfo;
            return STRUCT_MSG_ERR_OK;
          }
          idx++;
        }
//        structmsg_userdata.handles = os_realloc(structmsg_userdata.handles, sizeof(handle2Hdr_t)*(structmsg_userdata.numHandles+1));
//        checkAllocOK(structmsg_userdata.handles);
        structmsg_userdata.handles[structmsg_userdata.numHandles].handle = handle;
        hdrInfo = structmsg_userdata.handles[structmsg_userdata.numHandles++].hdrInfo;
      }
      return STRUCT_MSG_ERR_OK;
    },
    
    // ============================= deleteHandle ========================
    
    deleteHandle: function(handle) {
      var idx;
      var numUsed;
      var found;
    
      if (structmsg_userdata.handles == null) {
        return STRUCT_MSG_ERR_HANDLE_NOT_FOUND;
      }
      found = 0;
      idx = 0;
      numUsed = 0;
      while (idx < structmsg_userdata.numHandles) {
        if ((structmsg_userdata.handles[idx].handle != null) && (structmsg_userdata.handles[idx].handle == handle)) {
          structmsg_userdata.handles[idx].handle = null;
          found++;
        } else {
          if (structmsg_userdata.handles[idx].handle != null) {
            numUsed++;
          }
        }
        idx++;
      }
      if (numUsed == 0) {
//        os_free(structmsg_userdata.handles);
        structmsg_userdata.handles = null;
      }
      if (found) {
          return STRUCT_MSG_ERR_OK;
      }
      return STRUCT_MSG_ERR_HANDLE_NOT_FOUND;
    },
    
    // ============================= newFieldInfos ========================
    
    newFieldInfos: function(numFieldInfos) {
//      ptr = (fieldInfo_t *)os_malloc (sizeof(fieldInfo_t) * numFieldInfos);
      return ptr;
    },
    
    // ============================= setHandleField ========================
    
    setHandleField: function(handle, fieldId, fieldValue) {
      var idx;
      var result = STRUCT_MSG_ERR_OK;
    
      if (structmsg_userdata.handles == null) {
        return STRUCT_MSG_ERR_HANDLE_NOT_FOUND;
      }
      idx = 0;
      while (idx < structmsg_userdata.numHandles) {
        if ((structmsg_userdata.handles[idx].handle != null) && (c_strcmp(structmsg_userdata.handles[idx].handle, handle) == 0)) {
          switch (fieldId) {
          case STRUCT_MSG_FIELD_SRC:
            structmsg_userdata.handles[idx].hdrInfo.hdrKeys.src = fieldValue;
            break;
          case STRUCT_MSG_FIELD_DST:
            structmsg_userdata.handles[idx].hdrInfo.hdrKeys.dst = fieldValue;
            break;
          case STRUCT_MSG_FIELD_TOTAL_LGTH:
            structmsg_userdata.handles[idx].hdrInfo.hdrKeys.totalLgth = fieldValue;
            break;
          case STRUCT_MSG_FIELD_CMD_KEY:
            structmsg_userdata.handles[idx].hdrInfo.hdrKeys.cmdKey = fieldValue;
            break;
          case STRUCT_MSG_FIELD_CMD_LGTH:
            structmsg_userdata.handles[idx].hdrInfo.hdrKeys.cmdLgth = fieldValue;
            break;
          }
          return result;
        }
        idx++;
      }
      return STRUCT_MSG_ERR_HANDLE_NOT_FOUND;
    },
    
    // ============================= structmsg_encryptdecrypt ========================
    
    structmsg_encryptdecrypt: function(handle, msg, mlgth, key, klen, iv, ivlen, enc, buf, lgth) {
      var structmsg;
      var message;
      var mlen;
      var mech;
      var data;
      var dlen;
      var bs;
      var clen;
      var what;
      var crypted;
    
      buf = null;
      lgth = 0;
      if (handle != null) {
        structmsg = structmsg_get_structmsg_ptr(handle);
        checkHandleOK(structmsg);
    
        mlen = structmsg.hdr.hdrInfo.hdrKeys.totalLgth;
        if (enc) {
          if (structmsg.encoded == null) {
            return STRUCT_MSG_ERR_NOT_ENCODED;
          }
          message = structmsg.encoded;
        } else {
          if (structmsg.encrypted == null) {
            return STRUCT_MSG_ERR_NOT_ENCRYPTED;
          }
          message = structmsg.encrypted;
        }
      } else {
        message = msg;
        mlen = mlgth;
      }
    
      mech = crypto_encryption_mech ("AES-CBC");
      if (mech == null) {
        return STRUCT_MSG_ERR_CRYPTO_BAD_MECHANISM;
      }
      bs = mech.block_size;
      if (enc) {
        what = "encrypt";
      } else {
        what = "decrypt";
      }
    //ets_printf("encryptdecrypt bs: %d what: %s enc: %d\n", bs, what, enc);
      dlen = mlen - STRUCT_MSG_HEADER_LENGTH;
      data = message + STRUCT_MSG_HEADER_LENGTH;
      clen = ((dlen + bs - 1) / bs) * bs;
      lgth = clen + STRUCT_MSG_HEADER_LENGTH;
    //ets_printf("dlen: %d lgth: %d clen: %d data: %p\n", dlen, *lgth, clen, data);
//      crypted = (uint8_t *)os_zalloc (*lgth);
      if (!crypted) {
        return STRUCT_MSG_ERR_CRYPTO_INIT_FAILED;
      } 
      c_memcpy(crypted, message, STRUCT_MSG_HEADER_LENGTH);
      buf = crypted;
      crypted += STRUCT_MSG_HEADER_LENGTH;
//       if (enc) {
//      op =
//      { 
//        key, klen,
//        iv, ivlen,
//        data, dlen,
//        crypted, clen,
//        OP_ENCRYPT
//      }; 
//      } else {
//      op =
//      { 
//        key, klen,
//        iv, ivlen,
//        data, dlen,
//        crypted, clen,
//        OP_DECRYPT
//      }; 
//      }
      if (!mech.run (op)) { 
//        os_free (*buf);
        return STRUCT_MSG_ERR_CRYPTO_INIT_FAILED;
      } else { 
        if (enc) {
          if (handle != null) {
            structmsg.encrypted = buf;
          }
        } else {
          if (handle != null) {
            structmsg.todecode = buf;
          }
        }
        return STRUCT_MSG_ERR_OK;
      }
    },
    
    // ============================= fixHeaderInfo ========================
    
    fixHeaderInfo: function(structmsg, fieldInfo, fieldStr, fieldType, fieldLgth, numTableRows) {
//      fieldInfo.fieldStr = os_malloc(os_strlen(fieldStr) + 1);
//      fieldInfo.fieldStr[os_strlen(fieldStr)] = '\0';
      os_memcpy(fieldInfo.fieldStr, fieldStr, os_strlen(fieldStr));
      fieldInfo.fieldType = fieldType;
      fieldInfo.value.byteVector = null;
      fieldInfo.flags = 0;
      switch (fieldType) {
        case STRUCT_MSG_FIELD_UINT8_T:
        case STRUCT_MSG_FIELD_INT8_T:
          structmsg.hdr.hdrInfo.hdrKeys.totalLgth += 1 * numTableRows;
          structmsg.hdr.hdrInfo.hdrKeys.cmdLgth += 1 * numTableRows;
          fieldLgth = 1;
          break;
        case STRUCT_MSG_FIELD_UINT16_T:
        case STRUCT_MSG_FIELD_INT16_T:
          structmsg.hdr.hdrInfo.hdrKeys.totalLgth += 2 * numTableRows;
          structmsg.hdr.hdrInfo.hdrKeys.cmdLgth += 2 * numTableRows;
          fieldLgth = 2;
          break;
        case STRUCT_MSG_FIELD_UINT32_T:
        case STRUCT_MSG_FIELD_INT32_T:
          structmsg.hdr.hdrInfo.hdrKeys.totalLgth += 4 * numTableRows;
          structmsg.hdr.hdrInfo.hdrKeys.cmdLgth += 4 * numTableRows;
          fieldLgth = 4;
          break;
        case STRUCT_MSG_FIELD_UINT8_VECTOR:
          structmsg.hdr.hdrInfo.hdrKeys.totalLgth += fieldLgth * numTableRows;
          structmsg.hdr.hdrInfo.hdrKeys.cmdLgth += fieldLgth * numTableRows;
//          fieldInfo.value.ubyteVector = (uint8_t *)os_malloc(fieldLgth + 1);
          fieldInfo.value.ubyteVector[fieldLgth] = '\0';
          break;
        case STRUCT_MSG_FIELD_INT8_VECTOR:
          structmsg.hdr.hdrInfo.hdrKeys.totalLgth += fieldLgth * numTableRows;
          structmsg.hdr.hdrInfo.hdrKeys.cmdLgth += fieldLgth * numTableRows;
//          fieldInfo.value.byteVector = (int8_t *)os_malloc(fieldLgth + 1);
          fieldInfo.value.ubyteVector[fieldLgth] = '\0';
          break;
        case STRUCT_MSG_FIELD_UINT16_VECTOR:
          structmsg.hdr.hdrInfo.hdrKeys.totalLgth += fieldLgth * numTableRows;
          structmsg.hdr.hdrInfo.hdrKeys.cmdLgth += fieldLgth * numTableRows;
//          fieldInfo.value.ushortVector = (uint16_t *)os_malloc(fieldLgth*sizeof(uint16_t));
          break;
        case STRUCT_MSG_FIELD_INT16_VECTOR:
          structmsg.hdr.hdrInfo.hdrKeys.totalLgth += fieldLgth * numTableRows;
          structmsg.hdr.hdrInfo.hdrKeys.cmdLgth += fieldLgth * numTableRows;
//          fieldInfo.value.shortVector = (int16_t *)os_malloc(fieldLgth*sizeof(int16_t));
          break;
        case STRUCT_MSG_FIELD_UINT32_VECTOR:
          structmsg.hdr.hdrInfo.hdrKeys.totalLgth += fieldLgth * numTableRows;
          structmsg.hdr.hdrInfo.hdrKeys.cmdLgth += fieldLgth * numTableRows;
//          fieldInfo.value.uint32Vector = (uint32_t *)os_malloc(fieldLgth*sizeof(uint32_t));
          break;
        case STRUCT_MSG_FIELD_INT32_VECTOR:
          structmsg.hdr.hdrInfo.hdrKeys.totalLgth += fieldLgth * numTableRows;
          structmsg.hdr.hdrInfo.hdrKeys.cmdLgth += fieldLgth * numTableRows;
//          fieldInfo.value.int32Vector = (int32_t *)os_malloc(fieldLgth*sizeof(int32_t));
          break;
      }
      setHandleField(structmsg.handle, STRUCT_MSG_FIELD_CMD_LGTH, structmsg.hdr.hdrInfo.hdrKeys.cmdLgth);
      setHandleField(structmsg.handle, STRUCT_MSG_FIELD_TOTAL_LGTH, structmsg.hdr.hdrInfo.hdrKeys.totalLgth);
      fieldInfo.fieldLgth = fieldLgth;
      return STRUCT_MSG_ERR_OK;
    },
    
    // ============================= setFieldValue ========================
    
    setFieldValue: function(structmsg, fieldInfo, fieldName, numericValue, stringValue) {
    
      switch (fieldInfo.fieldType) {
        case STRUCT_MSG_FIELD_INT8_T:
          if (stringValue == null) {
            if ((numericValue > -128) && (numericValue < 128)) {
              fieldInfo.value.byteVal = numericValue;
            } else {
              return STRUCT_MSG_ERR_VALUE_TOO_BIG;
            }
          } else {
            return STRUCT_MSG_ERR_BAD_VALUE;
          }
          break;
        case STRUCT_MSG_FIELD_UINT8_T:
          if (stringValue == null) {
            if ((numericValue >= 0) && (numericValue <= 256)) {
              fieldInfo.value.ubyteVal = numericValue;
            }
          } else {
            return STRUCT_MSG_ERR_BAD_VALUE;
          }
          break;
        case STRUCT_MSG_FIELD_INT16_T:
          if (stringValue == null) {
            if ((numericValue > -32767) && (numericValue < 32767)) {
              fieldInfo.value.shortVal = numericValue;
            } else {
              return STRUCT_MSG_ERR_VALUE_TOO_BIG;
            }
          } else {
            return STRUCT_MSG_ERR_BAD_VALUE;
          }
          break;
        case STRUCT_MSG_FIELD_UINT16_T:
          if (stringValue == null) {
            if ((numericValue >= 0) && (numericValue <= 65535)) {
              fieldInfo.value.ushortVal = numericValue;
            } else {
              return STRUCT_MSG_ERR_VALUE_TOO_BIG;
            }
          } else {
            return STRUCT_MSG_ERR_BAD_VALUE;
          }
          break;
        case STRUCT_MSG_FIELD_INT32_T:
          if (stringValue == null) {
            if ((numericValue > -0x7FFFFFFF) && (numericValue <= 0x7FFFFFFF)) {
              fieldInfo.value.val = numericValue;
            } else {
              return STRUCT_MSG_ERR_VALUE_TOO_BIG;
            }
          } else {
            return STRUCT_MSG_ERR_BAD_VALUE;
          }
          break;
        case STRUCT_MSG_FIELD_UINT32_T:
          if (stringValue == null) {
            // we have to do the signed check as numericValue is a sigend integer!!
            if ((numericValue > -0x7FFFFFFF) && (numericValue <= 0x7FFFFFFF)) {
              fieldInfo.value.uval = numericValue;
            } else {
              return STRUCT_MSG_ERR_VALUE_TOO_BIG;
            }
          } else {
            return STRUCT_MSG_ERR_BAD_VALUE;
          }
          break;
        case STRUCT_MSG_FIELD_INT8_VECTOR:
          if (stringValue != null) {
            // check for length needed!!
            os_memcpy(fieldInfo.value.ubyteVector, stringValue, fieldInfo.fieldLgth);
          } else {
            return STRUCT_MSG_ERR_BAD_VALUE;
          }
          break;
        case STRUCT_MSG_FIELD_UINT8_VECTOR:
          if (stringValue != null) {
            // check for length needed!!
            os_memcpy(fieldInfo.value.byteVector, stringValue, fieldInfo.fieldLgth);
          } else {
            return STRUCT_MSG_ERR_BAD_VALUE;
          }
          break;
        case STRUCT_MSG_FIELD_INT16_VECTOR:
          if (stringValue != null) {
            // check for length needed!!
            os_memcpy(fieldInfo.value.shortVector, stringValue, fieldInfo.fieldLgth);
          } else {
            return STRUCT_MSG_ERR_BAD_VALUE;
          }
          break;
        case STRUCT_MSG_FIELD_UINT16_VECTOR:
          if (stringValue != null) {
            // check for length needed!!
            os_memcpy(fieldInfo.value.ushortVector, stringValue, fieldInfo.fieldLgth);
          } else {
            return STRUCT_MSG_ERR_BAD_VALUE;
          }
          break;
        case STRUCT_MSG_FIELD_INT32_VECTOR:
          if (stringValue != null) {
            // check for length needed!!
            os_memcpy(fieldInfo.value.int32Vector, stringValue, fieldInfo.fieldLgth);
          } else {
            return STRUCT_MSG_ERR_BAD_VALUE;
          }
          break;
        case STRUCT_MSG_FIELD_UINT32_VECTOR:
          if (stringValue != null) {
            // check for length needed!!
            os_memcpy(fieldInfo.value.uint32Vector, stringValue, fieldInfo.fieldLgth);
          } else {
            return STRUCT_MSG_ERR_BAD_VALUE;
          }
          break;
        default:
          return STRUCT_MSG_ERR_BAD_FIELD_TYPE;
          break;
      }
      fieldInfo.flags |= STRUCT_MSG_FIELD_IS_SET;
      return STRUCT_MSG_ERR_OK;
    },
    
    
    // ============================= getFieldValue ========================
    
    getFieldValue: function(structmsg, fieldInfo, fieldName, numericValue, stringValue) {
    
      switch (fieldInfo.fieldType) {
        case STRUCT_MSG_FIELD_INT8_T:
          if (fieldInfo.flags & STRUCT_MSG_FIELD_IS_SET) {
            numericValue = fieldInfo.value.byteVal;
            return STRUCT_MSG_ERR_OK;
          }
          break;
        case STRUCT_MSG_FIELD_UINT8_T:
          if (fieldInfo.flags & STRUCT_MSG_FIELD_IS_SET) {
            numericValue = fieldInfo.value.ubyteVal;
            return STRUCT_MSG_ERR_OK;
          }
          break;
        case STRUCT_MSG_FIELD_INT16_T:
          if (fieldInfo.flags & STRUCT_MSG_FIELD_IS_SET) {
            numericValue = fieldInfo.value.shortVal;
            return STRUCT_MSG_ERR_OK;
          }
          break;
        case STRUCT_MSG_FIELD_UINT16_T:
          if (fieldInfo.flags & STRUCT_MSG_FIELD_IS_SET) {
            numericValue = fieldInfo.value.ushortVal;
            return STRUCT_MSG_ERR_OK;
          }
          break;
        case STRUCT_MSG_FIELD_INT32_T:
          if (fieldInfo.flags & STRUCT_MSG_FIELD_IS_SET) {
            numericValue = fieldInfo.value.val;
            return STRUCT_MSG_ERR_OK;
          }
          break;
        case STRUCT_MSG_FIELD_UINT32_T:
          if (fieldInfo.flags & STRUCT_MSG_FIELD_IS_SET) {
            numericValue = fieldInfo.value.uval;
            return STRUCT_MSG_ERR_OK;
          }
          break;
        case STRUCT_MSG_FIELD_INT8_VECTOR:
          if (fieldInfo.flags & STRUCT_MSG_FIELD_IS_SET) {
            stringValue = fieldInfo.value.byteVector;
            return STRUCT_MSG_ERR_OK;
          }
          break;
        case STRUCT_MSG_FIELD_UINT8_VECTOR:
          if (fieldInfo.flags & STRUCT_MSG_FIELD_IS_SET) {
            stringValue = fieldInfo.value.ubyteVector;
            return STRUCT_MSG_ERR_OK;
          }
          break;
        case STRUCT_MSG_FIELD_INT16_VECTOR:
          if (fieldInfo.flags & STRUCT_MSG_FIELD_IS_SET) {
            stringValue = fieldInfo.value.shortVector;
            return STRUCT_MSG_ERR_OK;
          }
          break;
        case STRUCT_MSG_FIELD_UINT16_VECTOR:
          if (fieldInfo.flags & STRUCT_MSG_FIELD_IS_SET) {
            stringValue = fieldInfo.value.ushortVector;
            return STRUCT_MSG_ERR_OK;
          }
          break;
        case STRUCT_MSG_FIELD_INT32_VECTOR:
          if (fieldInfo.flags & STRUCT_MSG_FIELD_IS_SET) {
            stringValue = fieldInfo.value.int32Vector;
            return STRUCT_MSG_ERR_OK;
          }
          break;
        case STRUCT_MSG_FIELD_UINT32_VECTOR:
          if (fieldInfo.flags & STRUCT_MSG_FIELD_IS_SET) {
            stringValue = fieldInfo.value.uint32Vector;
            return STRUCT_MSG_ERR_OK;
          }
          break;
        default:
          return STRUCT_MSG_ERR_BAD_FIELD_TYPE;
          break;
      }
      return STRUCT_MSG_ERR_VALUE_NOT_SET;
    },
    
    // ============================= stmsg_createMsg ========================
    
    stmsg_createMsg: function(numFieldInfos, handle) {
      var ptr;
      var hdrInfo;
      var result;
    
//      var structmsg = (void *)os_malloc (sizeof(structmsg_t));
//      checkAllocOK(structmsg);
      structmsg.hdr.hdrInfo.hdrKeys.src = 0;
      structmsg.hdr.hdrInfo.hdrKeys.dst = 0;
      structmsg.hdr.hdrInfo.hdrKeys.cmdKey = 0;
      structmsg.hdr.hdrInfo.hdrKeys.cmdLgth = STRUCT_MSG_CMD_HEADER_LENGTH;
      structmsg.hdr.headerLgth = STRUCT_MSG_HEADER_LENGTH;
      structmsg.hdr.hdrInfo.hdrKeys.totalLgth = STRUCT_MSG_TOTAL_HEADER_LENGTH;
      structmsg.msg.numFieldInfos = 0;
      structmsg.msg.maxFieldInfos = numFieldInfos;
      structmsg.msg.numTableRows = 0;
      structmsg.msg.numTableRowFields = 0;
      structmsg.msg.numRowFields = 0;
      structmsg.msg.fieldInfos = newFieldInfos(numFieldInfos);
      structmsg.msg.tableFieldInfos = null;
      structmsg.flags = 0;
      structmsg.sequenceNum = 0;
      structmsg.encoded = null;
      structmsg.todecode = null;
      structmsg.encrypted = null;
      structmsg.handleHdrInfoPtr = null;
      os_sprintf(structmsg.handle, "%s%p", HANDLE_PREFIX, structmsg);
      result = addHandle(structmsg.handle, structmsg.handleHdrInfoPtr);
      if (result != STRUCT_MSG_ERR_OK) {
//        os_free(structmsg.msg.fieldInfos);
        deleteHandle(structmsg.handle);
//        os_free(structmsg);
        return result;
      }
      setHandleField(structmsg.handle, STRUCT_MSG_FIELD_CMD_LGTH, structmsg.hdr.hdrInfo.hdrKeys.cmdLgth);
      setHandleField(structmsg.handle, STRUCT_MSG_FIELD_TOTAL_LGTH, structmsg.hdr.hdrInfo.hdrKeys.totalLgth);
      result = structmsg_fillHdrInfo(structmsg.handle, structmsg);
      if (result != STRUCT_MSG_ERR_OK) {
//        os_free(structmsg.msg.fieldInfos);
        deleteHandle(structmsg.handle);
//        os_free(structmsg);
        return result;
      }
      handle = structmsg.handle;
      return result;
    },
    
    // ============================= stmsg_deleteMsg ========================
    
    stmsg_deleteMsg: function(handle) {
      var structmsg;
      var idx;
    
      structmsg = structmsg_get_structmsg_ptr(handle);
      checkHandleOK(structmsg);
      idx = 0;
      while (idx < structmsg.msg.numFieldInfos) {
        fieldInfo = structmsg.msg.fieldInfos[idx];
//        os_free(fieldInfo.fieldStr);
        switch (fieldInfo.fieldType) {
        case STRUCT_MSG_FIELD_INT8_VECTOR:
          if (fieldInfo.value.byteVector != null) {
//            os_free(fieldInfo.value.byteVector);
          }
          break;
        case STRUCT_MSG_FIELD_UINT8_VECTOR:
          if (fieldInfo.value.byteVector != null) {
//            os_free(fieldInfo.value.ubyteVector);
          }
          break;
        case STRUCT_MSG_FIELD_INT16_VECTOR:
          if (fieldInfo.value.shortVector != null) {
//            os_free(fieldInfo.value.shortVector);
          }
          break;
        case STRUCT_MSG_FIELD_UINT16_VECTOR:
          if (fieldInfo.value.ushortVector != null) {
//            os_free(fieldInfo.value.ushortVector);
          }
          break;
        case STRUCT_MSG_FIELD_INT32_VECTOR:
          if (fieldInfo.value.int32Vector != null) {
//            os_free(fieldInfo.value.int32Vector);
          }
          break;
        case STRUCT_MSG_FIELD_UINT32_VECTOR:
          if (fieldInfo.value.uint32Vector != null) {
//            os_free(fieldInfo.value.uint32Vector);
          }
          break;
        }
        idx++;
      }
//      os_free(structmsg.msg.fieldInfos);
      if (structmsg.encoded != null) {
//        os_free(structmsg.encoded);
      }
      if (structmsg.todecode != null) {
//        os_free(structmsg.todecode);
      }
      if (structmsg.encrypted != null) {
//        os_free(structmsg.encrypted);
      }
      deleteHandle(handle);
//      os_free(structmsg);
      return STRUCT_MSG_ERR_OK;
    },
    
    // ============================= dumpTableRowFields ========================
    
    dumpTableRowFields: function(structmsg) {
      var numEntries;
      var idx;
      var valueIdx;
      var result;
      var uch;
      var ch;
      var ush;
      var sh;
      var uval;
      var val;
      var row;
      var col;
      var fieldType;
    
      numEntries = structmsg.msg.numTableRows * structmsg.msg.numRowFields;
      ets_printf("    numTableFieldInfos: %d\r\n", numEntries);
      idx = 0;
      row = 0;
      col = 0;
      while (idx < numEntries) {
        fieldInfo = structmsg.msg.tableFieldInfos[idx];
        result = structmsg_getFieldTypeStr(fieldInfo.fieldType, fieldType);
        checkErrOK(result);
        ets_printf("      row %d: col: %d key: %-20s type: %-8s lgth: %.5d\r\n", row, col, fieldInfo.fieldStr, fieldType, fieldInfo.fieldLgth);
    //ets_printf("isSet: %s 0x%02x %d\n", fieldInfo.fieldStr, fieldInfo.flags, (fieldInfo.flags & STRUCT_MSG_FIELD_IS_SET));
        if (fieldInfo.flags & STRUCT_MSG_FIELD_IS_SET) {
          switch (fieldInfo.fieldType) {
          case STRUCT_MSG_FIELD_INT8_T:
            ets_printf("        value: %d 0x%02x\n", fieldInfo.value.byteVal, (fieldInfo.value) & 0xFF);
            break;
          case STRUCT_MSG_FIELD_UINT8_T:
            ets_printf("        value: 0x%02x\n", fieldInfo.value);
            break;
          case STRUCT_MSG_FIELD_INT16_T:
            ets_printf("        value: 0x%04x\n", fieldInfo.value);
            break;
          case STRUCT_MSG_FIELD_UINT16_T:
            ets_printf("        value: 0x%04x\n", fieldInfo.value);
            break;
          case STRUCT_MSG_FIELD_INT32_T:
            ets_printf("        value: 0x%08x\n", fieldInfo.value);
            break;
          case STRUCT_MSG_FIELD_UINT32_T:
            ets_printf("        value: 0x%08x\n", fieldInfo.value);
            break;
          case STRUCT_MSG_FIELD_INT8_VECTOR:
            valueIdx = 0;
            ets_printf("        values:");
            while (valueIdx < fieldInfo.fieldLgth) {
              ch = fieldInfo.value.byteVector[valueIdx];
              ets_printf("          idx: %d value: %c 0x%02x\n", valueIdx, ch, (ch & 0xFF));
              valueIdx++;
            }
            ets_printf("\n");
            break;
          case STRUCT_MSG_FIELD_UINT8_VECTOR:
            valueIdx = 0;
            ets_printf("        values:\n");
            while (valueIdx < fieldInfo.fieldLgth) {
              uch = fieldInfo.value.ubyteVector[valueIdx];
              ets_printf("          idx: %d value: %c 0x%02x\n", valueIdx, uch, (uch & 0xFF));
              valueIdx++;
            }
            break;
          case STRUCT_MSG_FIELD_INT16_VECTOR:
            valueIdx = 0;
            ets_printf("        values:");
            while (valueIdx < fieldInfo.fieldLgth) {
              sh = fieldInfo.value.shortVector[valueIdx];
              ets_printf("          idx: %d value: 0x%04x\n", valueIdx, (sh & 0xFFFF));
              valueIdx++;
            }
            ets_printf("\n");
            break;
          case STRUCT_MSG_FIELD_UINT16_VECTOR:
            valueIdx = 0;
            ets_printf("        values:\n");
            while (valueIdx < fieldInfo.fieldLgth) {
              ush = fieldInfo.value.ushortVector[valueIdx];
              ets_printf("          idx: %d value: 0x%04x\n", valueIdx, (ush & 0xFFFF));
              valueIdx++;
            }
            break;
          case STRUCT_MSG_FIELD_INT32_VECTOR:
            valueIdx = 0;
            ets_printf("        values:");
            while (valueIdx < fieldInfo.fieldLgth) {
              val = fieldInfo.value.int32Vector[valueIdx];
              ets_printf("          idx: %d value: 0x%08x\n", valueIdx, (val & 0xFFFFFFFF));
              valueIdx++;
            }
            ets_printf("\n");
            break;
          case STRUCT_MSG_FIELD_UINT32_VECTOR:
            valueIdx = 0;
            ets_printf("        values:\n");
            while (valueIdx < fieldInfo.fieldLgth) {
              uval = fieldInfo.value.uint32Vector[valueIdx];
              ets_printf("          idx: %d value: 0x%08x\n", valueIdx, (uval & 0xFFFFFFFF));
              valueIdx++;
            }
            break;
          }
        }
        col++;
        if (col == structmsg.msg.numRowFields) {
          row++;
          col = 0;
        }
        idx++;
      }
      return STRUCT_MSG_ERR_OK;
    },
    
    // ============================= stmsg_dumpMsg ========================
    
    stmsg_dumpMsg: function(handle) {
      var numEntries;
      var idx;
      var valueIdx;
      var result;
      var uch;
      var ch;
      var ush;
      var sh;
      var uval;
      var val;
      var fieldType;
      var structmsg;
    
      structmsg = structmsg_get_structmsg_ptr(handle);
      checkHandleOK(structmsg);
      ets_printf("handle: %s src: %d dst: %d totalLgth: %d\r\n", structmsg.handle, structmsg.hdr.hdrInfo.hdrKeys.src, structmsg.hdr.hdrInfo.hdrKeys.dst, structmsg.hdr.hdrInfo.hdrKeys.totalLgth);
      ets_printf("  cmdKey: %d cmdLgth: %d\r\n", structmsg.hdr.hdrInfo.hdrKeys.cmdKey, structmsg.hdr.hdrInfo.hdrKeys.cmdLgth);
      numEntries = structmsg.msg.numFieldInfos;
      ets_printf("  numFieldInfos: %d max: %d\r\n", numEntries, structmsg.msg.maxFieldInfos);
      idx = 0;
      while (idx < numEntries) {
        fieldInfo = structmsg.msg.fieldInfos[idx];
        if (c_strcmp(fieldInfo.fieldStr, "@tablerows") == 0) {
          result = structmsg_getFieldTypeStr(fieldInfo.fieldType, fieldType);
          checkErrOK(result);
          ets_printf("    idx %d: key: %-20s type: %-8s lgth: %.5d\r\n", idx, fieldInfo.fieldStr, fieldType, structmsg.msg.numTableRows);
          idx++;
          continue;
        }
        if (c_strcmp(fieldInfo.fieldStr, "@tablerowfields") == 0) {
          result = structmsg_getFieldTypeStr(fieldInfo.fieldType, fieldType);
          checkErrOK(result);
          ets_printf("    idx %d: key: %-20s type: %-8s lgth: %.5d\r\n", idx, fieldInfo.fieldStr, fieldType, structmsg.msg.numRowFields);
          dumpTableRowFields(structmsg);
          idx++;
          continue;
        }
        result = structmsg_getFieldTypeStr(fieldInfo.fieldType, fieldType);
        checkErrOK(result);
        ets_printf("    idx %d: key: %-20s type: %-8s lgth: %.5d\r\n", idx, fieldInfo.fieldStr, fieldType, fieldInfo.fieldLgth);
        if (fieldInfo.flags & STRUCT_MSG_FIELD_IS_SET) {
          switch (fieldInfo.fieldType) {
          case STRUCT_MSG_FIELD_INT8_T:
            ets_printf("      value: 0x%02x\n", fieldInfo.value.byteVal);
            break;
          case STRUCT_MSG_FIELD_UINT8_T:
            ets_printf("      value: 0x%02x\n", fieldInfo.value.ubyteVal);
            break;
          case STRUCT_MSG_FIELD_INT16_T:
            ets_printf("      value: 0x%04x\n", fieldInfo.value.shortVal);
            break;
          case STRUCT_MSG_FIELD_UINT16_T:
            ets_printf("      value: 0x%04x\n", fieldInfo.value.ushortVal);
            break;
          case STRUCT_MSG_FIELD_INT32_T:
            ets_printf("      value: 0x%08x\n", fieldInfo.value.val);
            break;
          case STRUCT_MSG_FIELD_UINT32_T:
            ets_printf("      value: 0x%08x\n", fieldInfo.value.uval);
            break;
          case STRUCT_MSG_FIELD_INT8_VECTOR:
            valueIdx = 0;
            ets_printf("      values:");
            while (valueIdx < fieldInfo.fieldLgth) {
              ch = fieldInfo.value.byteVector[valueIdx];
              ets_printf("        idx: %d value: %c 0x%02x\n", valueIdx, ch, (ch & 0xFF));
              valueIdx++;
            }
            ets_printf("\n");
            break;
          case STRUCT_MSG_FIELD_UINT8_VECTOR:
            valueIdx = 0;
            ets_printf("      values:\n");
            while (valueIdx < fieldInfo.fieldLgth) {
              uch = fieldInfo.value.ubyteVector[valueIdx];
              ets_printf("        idx: %d value: %c 0x%02x\n", valueIdx, uch, (uch & 0xFF));
              valueIdx++;
            }
            break;
          case STRUCT_MSG_FIELD_INT16_VECTOR:
            valueIdx = 0;
            ets_printf("      values:");
            while (valueIdx < fieldInfo.fieldLgth) {
              sh = fieldInfo.value.shortVector[valueIdx];
              ets_printf("        idx: %d value: 0x%04x\n", valueIdx, (sh & 0xFFFF));
              valueIdx++;
            }
            ets_printf("\n");
            break;
          case STRUCT_MSG_FIELD_UINT16_VECTOR:
            valueIdx = 0;
            ets_printf("      values:\n");
            while (valueIdx < fieldInfo.fieldLgth) {
              ush = fieldInfo.value.ushortVector[valueIdx];
              ets_printf("        idx: %d value: 0x%04x\n", valueIdx, (ush & 0xFFFF));
              valueIdx++;
            }
            break;
          case STRUCT_MSG_FIELD_INT32_VECTOR:
            valueIdx = 0;
            ets_printf("      values:");
            while (valueIdx < fieldInfo.fieldLgth) {
              val = fieldInfo.value.int32Vector[valueIdx];
              ets_printf("        idx: %d value: 0x%08x\n", valueIdx, (val & 0xFFFFFFFF));
              valueIdx++;
            }
            ets_printf("\n");
            break;
          case STRUCT_MSG_FIELD_UINT32_VECTOR:
            valueIdx = 0;
            ets_printf("      values:\n");
            while (valueIdx < fieldInfo.fieldLgth) {
              uval = fieldInfo.value.uint32Vector[valueIdx];
              ets_printf("        idx: %d value: 0x%08x\n", valueIdx, (uval & 0xFFFFFFFF));
              valueIdx++;
            }
            break;
          }
        }
        idx++;
      }
      return STRUCT_MSG_ERR_OK;
    },
    
    
    // ============================= stmsg_setFillerAndCrc ========================
    
    stmsg_setFillerAndCrc: function(handle) {
      var structmsg;
      var fillerLgth = 0;
      var myLgth = 0;
      var result;
    
      structmsg = structmsg_get_structmsg_ptr(handle);
      checkHandleOK(structmsg);
      // space for the numEntries field!!
      structmsg.hdr.hdrInfo.hdrKeys.cmdLgth++;
      structmsg.hdr.hdrInfo.hdrKeys.totalLgth++;
      // end space for the numEntries field!!
      myLgth = structmsg.hdr.hdrInfo.hdrKeys.cmdLgth + 2;
      while ((myLgth % 16) != 0) {
        myLgth++;
        fillerLgth++;
      }
      result = stmsg_addField(handle, "@filler", "uint8_t*", fillerLgth);
      checkErrOK(result);
      result = stmsg_addField(handle, "@crc", "uint16_t", 2);
      checkErrOK(result);
      return STRUCT_MSG_ERR_OK;
    },
    
    // ============================= stmsg_setFieldValue ========================
    
    stmsg_setFieldValue: function(handle, fieldName, numericValue, stringValue) {
      var structmsg;
      var fieldInfo;
      var idx;
      var result;
      var numEntries;
    
      structmsg = structmsg_get_structmsg_ptr(handle);
      checkHandleOK(structmsg);
      if (c_strcmp(fieldName, "@src") == 0) {
        if (stringValue == null) {
          if ((numericValue >= 0) && (numericValue <= 65535)) {
            structmsg.hdr.hdrInfo.hdrKeys.src = numericValue;
            setHandleField(handle, STRUCT_MSG_FIELD_SRC, structmsg.hdr.hdrInfo.hdrKeys.src);
            result = structmsg_fillHdrInfo(handle, structmsg);
            return result;
          } else {
            return STRUCT_MSG_ERR_VALUE_TOO_BIG;
          }
        } else {
          return STRUCT_MSG_ERR_BAD_VALUE;
        }
      } else {
        if (c_strcmp(fieldName, "@dst") == 0) {
          if (stringValue == null) {
            if ((numericValue >= 0) && (numericValue <= 65535)) {
              structmsg.hdr.hdrInfo.hdrKeys.dst = numericValue;
              setHandleField(handle, STRUCT_MSG_FIELD_DST, structmsg.hdr.hdrInfo.hdrKeys.dst);
              result = structmsg_fillHdrInfo(handle, structmsg);
              return result;
            } else {
              return STRUCT_MSG_ERR_VALUE_TOO_BIG;
            }
          } else {
            return STRUCT_MSG_ERR_BAD_VALUE;
          }
        } else {
          if (c_strcmp(fieldName, "@cmdKey") == 0) {
            if (stringValue == null) {
              if ((numericValue >= 0) && (numericValue <= 65535)) {
                structmsg.hdr.hdrInfo.hdrKeys.cmdKey = numericValue;
                setHandleField(handle, STRUCT_MSG_FIELD_CMD_KEY, structmsg.hdr.hdrInfo.hdrKeys.cmdKey);
                result = structmsg_fillHdrInfo(handle, structmsg);
                return result;
              } else {
                return STRUCT_MSG_ERR_VALUE_TOO_BIG;
              }
            } else {
              return STRUCT_MSG_ERR_BAD_VALUE;
            }
          }
        }
      }
      idx = 0;
      numEntries = structmsg.msg.numFieldInfos;
      while (idx < numEntries) {
        fieldInfo = structmsg.msg.fieldInfos[idx];
        if (c_strcmp(fieldName, fieldInfo.fieldStr) == 0) {
          return setFieldValue(structmsg, fieldInfo, fieldName, numericValue, stringValue);
        }
        idx++;
      }
      return STRUCT_MSG_ERR_FIELD_NOT_FOUND;
    },
    
    // ============================= stmsg_setTableFieldValue ========================
    
    stmsg_setTableFieldValue: function(handle, fieldName, row, numericValue, stringValue) {
      var structmsg;
      var fieldInfo;
      var idx;
      var cell;
    
      structmsg = structmsg_get_structmsg_ptr(handle);
      checkHandleOK(structmsg);
      if (row >= structmsg.msg.numTableRows) {
        return STRUCT_MSG_ERR_BAD_TABLE_ROW;
      }
      idx = 0;
      cell = 0 + row * structmsg.msg.numRowFields;
      while (idx < structmsg.msg.numRowFields) {
        fieldInfo = structmsg.msg.tableFieldInfos[cell];
        if (c_strcmp(fieldName, fieldInfo.fieldStr) == 0) {
          return setFieldValue(structmsg, fieldInfo, fieldName, numericValue, stringValue);
        }
        idx++;
        cell++;
      }
      return STRUCT_MSG_ERR_FIELD_NOT_FOUND;
    },
    
    
    // ============================= stmsg_getFieldValue ========================
    
    stmsg_getFieldValue: function(handle, fieldName, numericValue, stringValue) {
      var structmsg;
      var fieldInfo;
      var idx;
      var numEntries;
    
      structmsg = structmsg_get_structmsg_ptr(handle);
      checkHandleOK(structmsg);
      numericValue = 0;
      stringValue = null;
      idx = 0;
      numEntries = structmsg.msg.numFieldInfos;
      while (idx < numEntries) {
        fieldInfo = structmsg.msg.fieldInfos[idx];
        if (c_strcmp(fieldName, fieldInfo.fieldStr) == 0) {
          //ets_printf("    idx %d: key: %-20s\r\n", idx, fieldInfo.fieldStr);
          return getFieldValue(structmsg, fieldInfo, fieldName, numericValue, stringValue);
        }
        idx++;
      }
      return STRUCT_MSG_ERR_FIELD_NOT_FOUND;
    },
    
    // ============================= stmsg_getTableFieldValue ========================
    
    stmsg_getTableFieldValue: function(handle, fieldName, row, numericValue, stringValue) {
      var structmsg;
      var fieldInfo;
      var idx;
      var cell;
    
      structmsg = structmsg_get_structmsg_ptr(handle);
      checkHandleOK(structmsg);
      if (row >= structmsg.msg.numTableRows) {
        return STRUCT_MSG_ERR_BAD_TABLE_ROW;
      }
      numericValue = 0;
      stringValue = null;
    
      idx = 0;
      cell = 0 + row * structmsg.msg.numRowFields;
      while (idx < structmsg.msg.numRowFields) {
        fieldInfo = structmsg.msg.tableFieldInfos[cell];
        if (c_strcmp(fieldName, fieldInfo.fieldStr) == 0) {
          return getFieldValue(structmsg, fieldInfo, fieldName, numericValue, stringValue);
        }
        idx++;
        cell++;
      }
      return STRUCT_MSG_ERR_FIELD_NOT_FOUND;
    },
    
    // ============================= stmsg_setCrypted ========================
    
    stmsg_setCrypted: function(handle, crypted, iryptedLgth) {
      var structmsg;
    
      structmsg = structmsg_get_structmsg_ptr(handle);
      checkHandleOK(structmsg);
//      structmsg.encrypted = os_malloc(cryptedLgth);
      checkAllocOK(structmsg.encrypted);
      c_memcpy(structmsg.encrypted, crypted, cryptedLgth);
      return STRUCT_MSG_ERR_OK;
    },
    
    // ============================= stmsg_decryptGetHandle ========================
    
    stmsg_decryptGetHandle: function(encryptedMsg, mlen, key, klen, iv, ivlen, handle) {
      var decrypted;
      var lgth;
      var result;
    
       decrypted = null;
       lgth = 0; 
       result = structmsg_encryptdecrypt(null, encryptedMsg, mlen, key, klen, iv, ivlen, false, decrypted, lgth);
       if (result != STRUCT_MSG_ERR_OK) {
         return result;
       }
       result = getHandle(decrypted, handle);
       return result;
    },
    
    // ============================= structmsg_createMsgFromListInfo ========================
    
    structmsg_createMsgFromListInfo: function(listVector, numEntries, numRows, flags, handle) {
      var listEntry;
      var idx;
      var result;
      var cp;
      var handle2;
      var fieldName;
      var fieldType;
      var fieldLgthStr;
      var endPtr;
      var ieldLgth;
      var flagStr;
      var flag;
      var lgth;
      var uflag;
    
      result = stmsg_createMsg(numEntries, handle2);
      checkErrOK(result);
      handle=handle2;
      listEntry = listVector[0];
      idx = 0;
      while(idx < numEntries) {
        listEntry = listVector[idx];
        buffer[c_strlen(listEntry) + 1];
        fieldName = buffer;
        c_memcpy(fieldName, listEntry, c_strlen(listEntry));
        fieldName[c_strlen(listEntry)] = '\0';
        cp = fieldName;
        while (cp != ',') {
           cp++;
        }
//        cp++ = '\0';
        fieldType = cp;
        while (cp != ',') {
          cp++;
        }
//        cp++ = '\0';
        fieldLgthStr = cp;
        while (cp != ',') {
          cp++;
        }
//        cp++ = '\0';
        flagStr = cp;
        if (c_strcmp(fieldLgthStr,"@numRows") == 0) {
          fieldLgth = numRows;
        } else {
          lgth = c_strtoul(fieldLgthStr, endPtr, 10);
          fieldLgth = lgth;
        }
        uflag = c_strtoul(flagStr, endPtr, 10);
        flag = uflag;
        if (flag == 0) {
          result = stmsg_addField(handle, fieldName, fieldType, fieldLgth);
          checkErrOK(result);
        } else {
          if ((flags != 0) && (flag == 2)) {
            result = stmsg_addField(handle, fieldName, fieldType, fieldLgth);
            checkErrOK(result);
          } else {
            if ((flags == 0) && (flag == 1)) {
              result = stmsg_addField(handle, fieldName, fieldType, fieldLgth);
              checkErrOK(result);
            }
          }
        }
        idx++;
      }
      return STRUCT_MSG_ERR_OK;
    },

  });

  T.Cmd = Cmd;

  T.log("module: "+name+" initialised!", "info", "structmsgCmd.js");
}, "0.0.1", {});
