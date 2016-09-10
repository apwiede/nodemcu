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
    T.log('constructor called', 'info', 'StructmsgInfos', true);

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

    T.log('constructor end', 'info', 'StructmsgInfos', true);
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

    /* ==================== geStructmsgInfo ===================================== */
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
    create: function (numFields) {
      var stmsgInfos = this;

      stmsgInfo = new T.StructmsgInfo();
      stmsgInfo.fieldNameInfos = stmsgInfos.fieldNameInfos;
      structmsgInfo=stmsgInfo.create(numFields);
      stmsgInfos.numHandles++;
//T.log(structmsgInfo.toDebugString(), 'info', 'stmsgInfo', true);
      stmsgInfos.handles.push({handle: structmsgInfo.handle, structmsg: stmsgInfo});
      stmsgInfos.result= stmsgInfos.STRUCT_MSG_ERR_OK;
      return structmsgInfo.handle;
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

    /* ==================== setTableFieldValue ===================================== */
    setTableFieldValue: function(handle, fieldName, row, value) {
      var stmsgInfos = this;
      var structmsgInfo = stmsgInfos.getStructmsgInfo(handle);
      if (structmsgInfo == null) {
        return stmsgInfos.result;
      }
      return structmsgInfo.setTableFieldValue(fieldName, row, value);
    },

    /* ==================== encode ===================================== */
    encode: function(handle) {
      var stmsgInfos = this;
      var structmsgInfo = stmsgInfos.getStructmsgInfo(handle);
      if (structmsgInfo == null) {
        return stmsgInfos.result;
      }
      return structmsgInfo.encode();
    },

    /* ==================== decode ===================================== */
    decode: function(handle, result) {
      var stmsgInfos = this;
      var structmsgInfo = stmsgInfos.getStructmsgInfo(handle);
      if (structmsgInfo == null) {
        return stmsgInfos.result;
      }
      return structmsgInfo.decode(result);
    },

  });

  T.StructmsgInfos = StructmsgInfos;

  T.log("module: "+name+" initialised!", "info", "StructmsgInfos.js");
}, "0.0.1", {});
