/*====================================================
 * A structured Message Support implementation in Javascript named Esp
 *
 * Structmsg MsgInfo for Esp (MsgInfo.js)
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

EM.addModule("Esp-MsgInfo", function(T, name) {

  /* ==================== MsgInfo constructor ======================= */

  function MsgInfo() {
    T.log('constructor called', 'info', 'MsgInfo', true);

    var msgInfo = this;
    var constructor = msgInfo.constructor;
    MsgInfo.superclass.constructor.apply(msgInfo, arguments);

    msgInfo.fieldInfos = null;
    msgInfo.tableFieldInfos = null;
    msgInfo.numFieldInfos = 0;
    msgInfo.maxFieldInfos = 0;
    msgInfo.numTableRows = 0;         // number of list rows
    msgInfo.numTableRowFields = 0;    // number of fields within a table row
    msgInfo.numRowFields = 0;         // for checking how many tableRowFields have been processed

    T.log('constructor end', 'info', 'MsgInfo', true);
  }

  T.extend(MsgInfo, T.Defines, {
     my_name: "MsgInfo",
     type_name: "msg_info",
     flags: 0,

    /* ==================== toString ===================================== */

    toString: function () {
      var msgInfo = this;
      return msgInfo.mySelf()+"!"+msgInfo.numFieldInfos+"!";
    },

    /* ==================== toDebugString ===================================== */
    toDebugString: function () {
      var msgInfo = this;
      var idx;
      var str = msgInfo.mySelf()+"\n";
      str += "    numFieldInfos:          "+msgInfo.numFieldInfos+"\n";
      str += "    maxFieldInfos:          "+msgInfo.maxFieldInfos+"\n";
      str += "    fieldInfos:      "+(msgInfo.fieldInfos == null ? "null" : "")+"\n";
      idx = 0;
      while (idx < msgInfo.numFieldInfos) {
        str += msgInfo.fieldInfos[idx].toDebugString();
        idx++;
      }
      str += "    numTableRows:           "+msgInfo.numTableRows+"\n";
      str += "    numTableRowFields:      "+msgInfo.numTableRowFields+"\n";
      str += "    numwRowFields:          "+msgInfo.numRowFields+"\n";
      str += "    tableFieldInfos: "+(msgInfo.tableFieldInfos == null ? "null" : "")+"\n";
      idx = 0;
      while (idx < msgInfo.numTableRows * msgInfo.numRowFields) {
        str += msgInfo.tableFieldInfos[idx].toDebugString();
        idx++;
      }
      return str;
    },

    // ============================= initFieldInfo ========================
    
    initFieldInfo: function(fieldInfo, fieldStr, fieldTypeStr, fieldLgth) {
      var msgInfo = this;
      fieldInfo.fieldStr = fieldStr;
      fieldInfo.fieldType = fieldType;
      fieldInfo.fieldLgth = fieldLgth;
      fieldInfo.fieldValue = null;
      fieldInfo.flags = 0;
    },

    // ============================= addField ========================
    
    addField: function(structmsgInfo, fieldStr, fieldTypeStr, fieldLgth) {
      var msgInfo = this;
      var numTableRowFields;
      var numTableRows;
      var numTableFields;
    
      fieldType = msgInfo.getFieldTypeId(fieldTypeStr);
//      checkErrOK(result);
      if (fieldStr == "@tablerows") {
        msgInfo.numTableRows = fieldLgth;
        msgInfo.initFieldInfo(msgInfo.fieldInfos[msgInfo.numFieldInfos], fieldStr, fieldType, fieldLgth);
        // we use 0 as numTableRows, that forces the *Lgth fields to NOT be modified!!
        structmsgInfo.hdr.fixHeaderInfo(msgInfo.fieldInfos[msgInfo.numFieldInfos], fieldType, 0, 0);
        msgInfo.numFieldInfos++;
        return msgInfo.STRUCT_MSG_ERR_OK;
      }
      if (fieldStr == "@tablerowfields") {
        msgInfo.numTableRowFields = fieldLgth;
        numTableFields = msgInfo.numTableRows * msgInfo.numTableRowFields;
        msgInfo.initFieldInfo(msgInfo.fieldInfos[msgInfo.numFieldInfos], fieldStr, fieldType, fieldLgth);
        // we use 0 as numTableRows, that forces the *Lgth fields to NOT be modified!!
        structmsgInfo.hdr.fixHeaderInfo(msgInfo.fieldInfos[msgInfo.numFieldInfos], fieldType, 0, 0);
        if ((msgInfo.tableFieldInfos == null) && (numTableFields != 0)) {
          msgInfo.tableFieldInfos = new Array(numTableFields);
          var idx = 0;
          while (idx < numTableFields) {
            msgInfo.tableFieldInfos[idx]= new T.FieldInfo();
            idx++;
          }
        }
        msgInfo.numFieldInfos++;
        return msgInfo.STRUCT_MSG_ERR_OK;
      }
      numTableRowFields = msgInfo.numTableRowFields;
      numTableRows = msgInfo.numTableRows;
      numTableFields = numTableRows * numTableRowFields;
      if (!((numTableFields > 0) && (msgInfo.numRowFields < numTableRowFields))) {
        if (msgInfo.numFieldInfos >= msgInfo.maxFieldInfos) {
          return msgInfo.STRUCT_MSG_ERR_TOO_MANY_FIELDS;
        }
        msgInfo.initFieldInfo(msgInfo.fieldInfos[msgInfo.numFieldInfos], fieldStr, fieldType, fieldLgth);
        numTableFields = 0;
        numTableRows = 1;
        numTableRowFields = 0;
        structmsgInfo.hdr.fixHeaderInfo(msgInfo.fieldInfos[msgInfo.numFieldInfos], fieldType, fieldLgth, numTableRows);
        result = structmsgInfo.hdr.fillHdrInfo();
        msgInfo.numFieldInfos++;
      } else {
        var cellIdx;
        row = 0;
        while (row < numTableRows) {
          cellIdx = msgInfo.numRowFields + row * numTableRowFields;;
          msgInfo.initFieldInfo(msgInfo.tableFieldInfos[cellIdx], fieldStr, fieldType, fieldLgth);
          structmsgInfo.hdr.fixHeaderInfo(msgInfo.tableFieldInfos[cellIdx], fieldType, fieldLgth, 1);
          row++;
        }
        msgInfo.numRowFields++;  
      } 
print("MsgInfo:\naddField", msgInfo.toDebugString());
      return msgInfo.STRUCT_MSG_ERR_OK;
    },
  });

  T.MsgInfo = MsgInfo;

  T.log("module: "+name+" initialised!", "info", "MsgInfo.js");
}, "0.0.1", {});
