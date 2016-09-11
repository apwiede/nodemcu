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

    /* ==================== create ===================================== */
    create: function (numFields, resultData) {
      var stmsgInfos = this;
      var result;

      stmsgInfo = new T.StructmsgInfo();
      stmsgInfo.fieldNameInfos = stmsgInfos.fieldNameInfos;
      stmsgInfo.structmsg = stmsgInfos.structmsg;
      result=stmsgInfo.create(numFields, resultData);
      if(result != stmsgInfo.STRUCT_MSG_ERR_OK) return result;
      structmsgInfo = resultData.data;
      resultData.handle = resultData.data.handle;
      stmsgInfos.numHandles++;
//T.log(structmsgInfo.toDebugString(), 'info', 'stmsgInfo', true);
      stmsgInfos.handles.push({handle: structmsgInfo.handle, structmsg: stmsgInfo});
      stmsgInfos.result= stmsgInfos.STRUCT_MSG_ERR_OK;
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

  });

  T.StructmsgInfos = StructmsgInfos;

  T.log("module: "+name+" initialised!", "2.info", "StructmsgInfos.js");
}, "0.0.1", {});
