/*====================================================
 * A structured Message Support implementation in Javascript named Esp
 *
 * Structmsg StructmsgInfos for Esp (StructmsgInfos.js)
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

EM.addModule("Esp-StructmsgInfos", function(T, name) {

  /* ==================== StructmsgInfos constructor ======================= */

  function StructmsgInfos() {
    T.log('constructor called', '2.info', 'StructmsgInfos', true);

    var StmsgInfos = this;

    var constructor = StmsgInfos.constructor;
    StructmsgInfos.superclass.constructor.apply(StmsgInfos, arguments);

    T.Defines.StructmsgInfosOid++;
    StmsgInfos.oid = T.Defines.StructmsgInfosOid;
    StmsgInfos.numHandles = 0;
    StmsgInfos.numHeaders = 0;
    StmsgInfos.handles = new Array();
    StmsgInfos.headers = new Array();
    StmsgInfos.fieldNameInfos = null;
    StmsgInfos.structmsg = null;

    T.log('constructor end', '2.info', 'StructmsgInfos', true);
  }


  T.extend(StructmsgInfos, T.Defines, {
     my_name: "StructmsgInfos",
     type_name: "structmsg_infos",
     flags: 0,

    /* ==================== toString ===================================== */

    toString: function () {
      var stmsgInfos = this;
      return stmsgInfos.mySelf()+"!"+stmsgInfos.oid+"!";
    },

    /* ==================== toDebugString ===================================== */
    toDebugString: function () {
      var stmsgInfos = this;
      var idx = 0;
      var str = stmsgInfos.mySelf()+"\n";
      str += "  numHandles:     "+stmsgInfos.numHandles+"\n";
      str += "  numHeaders:     "+stmsgInfos.numHeaders+"\n";
      str += "  handles:\n";
      while (idx < stmsgInfos.handles.length) {
        str += "    "+idx+": "+stmsgInfos.handles[idx].handle+'\n';
        idx++;
      }
      return str;
    },

    /* ==================== getStructmsgInfo ===================================== */
    getStructmsgInfo: function(handle) {
      var stmsgInfos = this;
      var idx = 0;
      while (idx < stmsgInfos.handles.length) {
        if (stmsgInfos.handles[idx].handle == handle) {
          return stmsgInfos.handles[idx].structmsg;
        }
        idx++;
      }
      stmsgInfos.result = stmsgInfos.STRUCT_MSG_HANDLE_NOT_FOUND;
      return null;
    },

    /* ==================== updateHeaders ===================================== */
    updateHeaders: function(structmsgInfo) {
      var stmsgInfos = this;
      var result;
      var headers;
      var idx;
      var hdrId;

      headers = stmsgInfos.headers;
      idx = 0;
      while (idx < stmsgInfos.numHeaders) {
        var entry = stmsgInfos.handles[idx];
        if (entry.handle == structmsgInfo.handle) {
          entry.hdrId = structmsgInfo.hdrId;
          stmsgInfos.handles[idx] = entry;
//print(">>updateHeaders1: ",structmsgInfo.toDebugString());
          return stmsgInfos.STRUCT_MSG_ERR_OK;
        }
        idx++;
      }
      hdrId = structmsgInfo.hdr.hdrId
      var myEntry = {handle: structmsgInfo.handle, header: hdrId};
      stmsgInfos.headers.push(myEntry);
      stmsgInfos.numHeaders++;
      var entry2 = stmsgInfos.headers[idx];
      return stmsgInfos.STRUCT_MSG_ERR_OK;
    },

    /* ==================== create ===================================== */
    create: function (numFields, resultData) {
      var stmsgInfos = this;
      var result;

      stmsgInfo = new T.StructmsgInfo();
      stmsgInfo.fieldNameInfos = stmsgInfos.fieldNameInfos;
      stmsgInfo.structmsg = stmsgInfos.structmsg;
      stmsgInfo.structmsgInfos = stmsgInfos;
      result=stmsgInfo.create(numFields, resultData);
      if(result != stmsgInfo.STRUCT_MSG_ERR_OK) return result;
      structmsgInfo = resultData.data;
      resultData.handle = resultData.data.handle;
      stmsgInfos.numHandles++;
//T.log(structmsgInfo.toDebugString(), 'info', 'stmsgInfo', true);
      stmsgInfos.handles.push({handle: structmsgInfo.handle, structmsg: stmsgInfo});
      stmsgInfos.result= stmsgInfos.STRUCT_MSG_ERR_OK;
      stmsgInfos.updateHeaders(structmsgInfo);
      return stmsgInfos.STRUCT_MSG_ERR_OK;
    },

    /* ==================== addField ===================================== */
    addField: function(handle, fieldStr, fieldType, fieldLgth) {
      var stmsgInfos = this;
      var structmsgInfo = stmsgInfos.getStructmsgInfo(handle);
      if (structmsgInfo == null) {
        return stmsgInfos.result;
      }
      return structmsgInfo.addField(fieldStr, fieldType, fieldLgth);
    },

    /* ==================== setFieldValue ===================================== */
    setFieldValue: function(handle, fieldName, value) {
      var stmsgInfos = this;
      var structmsgInfo = stmsgInfos.getStructmsgInfo(handle);
      if (structmsgInfo == null) {
        return stmsgInfos.result;
      }
      return structmsgInfo.setFieldValue(fieldName, value);
    },

    /* ==================== getFieldValue ===================================== */
    getFieldValue: function(handle, fieldName, resultData) {
      var stmsgInfos = this;
      var structmsgInfo = stmsgInfos.getStructmsgInfo(handle);
      if (structmsgInfo == null) {
        return stmsgInfos.result;
      }
      return structmsgInfo.getFieldValue(fieldName, resultData);
    },

    /* ==================== setTableFieldValue ===================================== */
    setTableFieldValue: function(handle, fieldName, row, value) {
      var stmsgInfos = this;
      var structmsgInfo = stmsgInfos.getStructmsgInfo(handle);
      if (structmsgInfo == null) {
        return stmsgInfos.result;
      }
      return structmsgInfo.setTableFieldValue(fieldName, row, value);
    },

    /* ==================== getTableFieldValue ===================================== */
    getTableFieldValue: function(handle, fieldName, row, resultData) {
      var stmsgInfos = this;
      var structmsgInfo = stmsgInfos.getStructmsgInfo(handle);
      if (structmsgInfo == null) {
        return stmsgInfos.result;
      }
      return structmsgInfo.getTableFieldValue(fieldName, row, resultData);
    },

    /* ==================== encode ===================================== */
    encode: function(handle, result_data) {
      var stmsgInfos = this;
      var structmsgInfo = stmsgInfos.getStructmsgInfo(handle);
      if (structmsgInfo == null) {
        return stmsgInfos.result;
      }
      return structmsgInfo.encode(result_data);
    },

    /* ==================== setEncoded ===================================== */
    setEncoded: function(handle, data) {
      var stmsgInfos = this;
      var structmsgInfo = stmsgInfos.getStructmsgInfo(handle);
      if (structmsgInfo == null) {
        return stmsgInfos.result;
      }
      return structmsgInfo.setEncoded(data);
    },

    /* ==================== getEncoded ===================================== */
    getEncoded: function(handle, result_data) {
      var stmsgInfos = this;
      var structmsgInfo = stmsgInfos.getStructmsgInfo(handle);
      if (structmsgInfo == null) {
        return stmsgInfos.result;
      }
      return structmsgInfo.getEncoded(result_data);
    },

    /* ==================== decode ===================================== */
    decode: function(handle) {
      var stmsgInfos = this;
      var structmsgInfo = stmsgInfos.getStructmsgInfo(handle);
      if (structmsgInfo == null) {
        return stmsgInfos.result;
      }
      return structmsgInfo.decode();
    },

    /* ==================== dump ===================================== */
    dump: function(handle) {
      var stmsgInfos = this;
      var structmsgInfo = stmsgInfos.getStructmsgInfo(handle);
      if (structmsgInfo == null) {
        return stmsgInfos.result;
      }
      return structmsgInfo.dump();
    },

    /* ==================== encrypt ===================================== */
    encrypt: function(cryptkey, ivvec, data, resultData) {
      var structmsgInfos = this;
      var key;
      var iv;
      var aesCbc;
      var aesCbc2;
      var headerLgth;
      var dv;
      var myMsg;
      var idx;
      var encryptedBytes;
      var header;

      headerLgth = structmsgInfos.STRUCT_MSG_HEADER_LENGTH;
      key = aesjs.util.convertStringToBytes(cryptkey);
      // The initialization vector, which must be 16 bytes
      iv  = aesjs.util.convertStringToBytes(ivvec);
      aesCbc = new aesjs.ModeOfOperation.cbc(key, iv);
      dv = new DataView(data);
      myMsg = new Array(data.byteLength-headerLgth);
      header = new Array();
      for (i = 0; i < headerLgth; i++) {
        header[i] = dv.getUint8(i).toString(10);  
      }
      idx = headerLgth;
      for (i = 0; i < data.byteLength-headerLgth; i++) {
        myMsg[i]= dv.getUint8(idx).toString(10);
        idx++;
      }
      encryptedBytes = aesCbc.encrypt(myMsg);
      resultData.encryptedBytes = header.concat(encryptedBytes);
      return structmsgInfos.STRUCT_MSG_ERR_OK;
    },

    /* ==================== decrypt ===================================== */
    decrypt: function(cryptkey, ivvec, crypted, resultData) {
      var structmsgInfos = this;
      var key;
      var iv;
      var aesCbcDec;
      var headerLgth;
      var dv;
      var myMsg;
      var idx;
      var decryptedBytes;
      var header;

      headerLgth = structmsgInfos.STRUCT_MSG_HEADER_LENGTH;
      key = aesjs.util.convertStringToBytes(cryptkey);
      // The initialization vector, which must be 16 bytes
      iv  = aesjs.util.convertStringToBytes(ivvec);
      aesCbcDec = new aesjs.ModeOfOperation.cbc(key, iv);
      dv = new DataView(crypted);
      myMsg = new Array(crypted.byteLength-headerLgth);
      header = new Array();
      for (i = 0; i < headerLgth; i++) {
        header[i] = dv.getUint8(i);  
      }
      idx = headerLgth;
      var h1 = "";
      for (i = 0; i < crypted.byteLength-headerLgth; i++) {
        var ch = dv.getUint8(idx).toString(10);
        myMsg[i]= dv.getUint8(idx).toString(10);
        idx++;
      }

      decryptedBytes = aesCbcDec.decrypt(myMsg);
      resultData.decryptedBytes = header.concat(decryptedBytes);
      return structmsgInfos.STRUCT_MSG_ERR_OK;
    },

    /* ==================== decryptGetHandle ===================================== */
    decryptGetHandle: function(cryptkey, ivvec, crypted, resultData) {
      var structmsgInfos = this;
      var result;
      var idx;
      var idx2;
      var haederIdBuf;
      var headers;
      var dv1;
      var dv2;
      var found;

      result = structmsgInfos.decrypt(cryptkey, ivvec, crypted, resultData);
      if(result != structmsgInfos.STRUCT_MSG_ERR_OK) return result;
      var arr2 = Uint8Array.from(resultData.decryptedBytes);
      var decryptedBytesBuf = arr2.buffer;
      headerIdBuf = decryptedBytesBuf;
      headers = structmsgInfos.headers;
      idx = 0;
      found = false;
      while (idx < structmsgInfos.numHeaders) {
        var entry = structmsgInfos.headers[idx];
        idx2 = 0;
        dv1 = new DataView(headerIdBuf);
        dv2 = new DataView(entry.header);
        found = true;
        while (idx2 < entry.header.byteLength) {
          if (dv1.getUint8(idx2) != dv2.getUint8(idx2)) {
            found = false;
            break;
          }
          idx2++;
        }
        if (found) {
          resultData.handle = entry.handle;
          return structmsgInfos.STRUCT_MSG_ERR_OK;
        }
        idx++;
      }
      return structmsgInfos.STRUCT_MSG_ERR_HANDLE_NOT_FOUND;
    },

  });

  T.StructmsgInfos = StructmsgInfos;

  T.log("module: "+name+" initialised!", "2.info", "StructmsgInfos.js");
}, "0.0.1", {});
