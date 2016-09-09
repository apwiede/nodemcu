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
      var str = msgInfo.mySelf()+"\n";
      str += "    fieldInfos (lgth):      "+(msgInfo.fieldInfos == null ? "null" : msgInfo.fieldInfos.length)+"\n";
      str += "    tableFieldInfos (lgth): "+(msgInfo.tableFieldInfos == null ? "null" : msgInfo.tableFieldInfos.length)+"\n";
      str += "    numFieldInfos:          "+msgInfo.numFieldInfos+"\n";
      str += "    maxFieldInfos:          "+msgInfo.maxFieldInfos+"\n";
      str += "    numTableRows:           "+msgInfo.numTableRows+"\n";
      str += "    numTableRowFields:      "+msgInfo.numTableRowFields+"\n";
      str += "    numwRoFields:           "+msgInfo.numRowFields+"\n";
      return str;
    },

    // ============================= addField ========================
    
    addField: function(fieldStr, fieldTypeStr, fieldLgth) {
      var msgInfo = this;
      var numTableRowFields;
      var numTableRows;
      var numTableFields;
print("MsgInfo: addField", msgInfo.toDebugString());
      if (msgInfo.numFieldInfos >= msgInfo.maxFieldInfos) {
      }
      
    
      fieldType = msgInfo.getFieldTypeId(fieldTypeStr);
//      checkErrOK(result);
print("fieldType: ",fieldType);
      if (fieldStr == "@tablerows") {
        structmsg.msg.numTableRows = fieldLgth;
    //ets_printf("tablerows1: lgth: %d\n",  fieldLgth);
        fieldInfo = structmsg.msg.fieldInfos[structmsg.msg.numFieldInfos];
    //ets_printf("tablerows1: totalLgth: %d cmdLgth: %d\n", structmsg.hdr.hdrInfo.hdrKeys.totalLgth, structmsg.hdr.hdrInfo.hdrKeys.cmdLgth);
        // we use 0 as numTableRows, that forces the *Lgth fields to NOT be modified!!
        fixHeaderInfo(structmsg, fieldInfo, fieldStr, fieldType, 0, 0);
    //ets_printf("tablerows2: totalLgth: %d cmdLgth: %d\n", structmsg.hdr.hdrInfo.hdrKeys.totalLgth, structmsg.hdr.hdrInfo.hdrKeys.cmdLgth);
        structmsg.msg.numFieldInfos++;
        return STRUCT_MSG_ERR_OK;
      }
      if (fieldStr == "@tablerowfields") {
        structmsg.msg.numTableRowFields = fieldLgth;
        numTableFields = structmsg.msg.numTableRows * structmsg.msg.numTableRowFields;
        fieldInfo = structmsg.msg.fieldInfos[structmsg.msg.numFieldInfos];
    //ets_printf("tablerowFields1: %d lgth: %d\n", numTableFields, fieldLgth);
    //ets_printf("tablerowfields1: totalLgth: %d cmdLgth: %d\n", structmsg.hdr.hdrInfo.hdrKeys.totalLgth, structmsg.hdr.hdrInfo.hdrKeys.cmdLgth);
        // we use 0 as numTableRows, that forces the *Lgth fields to NOT be modified!!
        fixHeaderInfo(structmsg, fieldInfo, fieldStr, fieldType, 0, 0);
    //ets_printf("tablerowfields2: totalLgth: %d cmdLgth: %d\n", structmsg.hdr.hdrInfo.hdrKeys.totalLgth, structmsg.hdr.hdrInfo.hdrKeys.cmdLgth);
        if ((structmsg.msg.tableFieldInfos == null) && (numTableFields != 0)) {
          structmsg.msg.tableFieldInfos = newFieldInfos(numTableFields);
        }
        structmsg.msg.numFieldInfos++;
        return STRUCT_MSG_ERR_OK;
      }
      numTableRowFields = msgInfo.numTableRowFields;
      numTableRows = msgInfo.numTableRows;
      numTableFields = numTableRows * numTableRowFields;
      if (!((numTableFields > 0) && (msgInfo.numRowFields < numTableRowFields))) {
        if (msgInfo.numFieldInfos >= msgInfo.maxFieldInfos) {
          return msgInfo.STRUCT_MSG_ERR_TOO_MANY_FIELDS;
        }
 print ("numFieldInfos:", msgInfo.numFieldInfos," ",msgInfo.maxFieldInfos);
        if (msgInfo.numFieldInfos <= msgInfo.fieldInfos.length) {
          fieldInfo = new T.FieldInfo();
          msgInfo.fieldInfos.push(fieldInfo);
        }
        fieldInfo = msgInfo.fieldInfos[msgInfo.numFieldInfos]
        fieldInfo.fieldStr = fieldStr;
        fieldInfo.fieldType = fieldType;
        fieldInfo.fieldLgth = fieldLgth;
        fieldInfo.fieldValue = null;
        fieldInfo.flags = 0;
        msgInfo.fieldInfos[msgInfo.numFieldInfos] = fieldInfo;
        msgInfo.numFieldInfos++;
        numTableFields = 0;
        numTableRows = 1;
        numTableRowFields = 0;
print("fi: ",fieldInfo.toDebugString());
return msgInfo.STRUCT_MSG_ERR_OK;
        fixHeaderInfo(structmsg, fieldInfo, fieldStr, fieldType, fieldLgth, numTableRows);
    //ets_printf("field2: %s totalLgth: %d cmdLgth: %d\n", fieldInfo.fieldStr, structmsg.hdr.hdrInfo.hdrKeys.totalLgth, structmsg.hdr.hdrInfo.hdrKeys.cmdLgth);
        result = structmsg_fillHdrInfo(handle, structmsg);
        structmsg.msg.numFieldInfos++;
      } else {
        row = 0;
        while (row < numTableRows) {
          cellIdx = structmsg.msg.numRowFields + row * numTableRowFields;;
          fieldInfo = structmsg.msg.tableFieldInfos[cellIdx];
    //ets_printf("table field1: %s totalLgth: %d cmdLgth: %d\n", fieldInfo.fieldStr, structmsg.hdr.hdrInfo.hdrKeys.totalLgth, structmsg.hdr.hdrInfo.hdrKeys.cmdLgth);
          fixHeaderInfo(structmsg, fieldInfo, fieldStr, fieldType, fieldLgth, 1);
    //ets_printf("table field2: %s totalLgth: %d cmdLgth: %d\n", fieldInfo.fieldStr, structmsg.hdr.hdrInfo.hdrKeys.totalLgth, structmsg.hdr.hdrInfo.hdrKeys.cmdLgth);
          row++;
        }
        structmsg.msg.numRowFields++;  
      } 
return msgInfo.STRUCT_MSG_ERR_OK;
      return result;
    },
  });

  T.MsgInfo = MsgInfo;

  T.log("module: "+name+" initialised!", "info", "MsgInfo.js");
}, "0.0.1", {});
