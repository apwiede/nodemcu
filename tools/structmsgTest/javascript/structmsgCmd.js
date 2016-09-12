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
