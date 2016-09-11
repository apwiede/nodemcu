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
    T.log('constructor called', '2.info', 'structmsgCmd', true);

    var cmd = this;
    var constructor = cmd.constructor;
    Cmd.superclass.constructor.apply(cmd, arguments);

    T.log('constructor end', '2.info', 'structmsgCmd', true);
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

  T.log("module: "+name+" initialised!", "2.info", "structmsgCmd.js");
}, "0.0.1", {});
