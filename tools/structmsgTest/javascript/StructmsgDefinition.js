/*====================================================
 * A structured Message Support implementation in Javascript named Esp
 *
 * Structmsg StructmsgDefinition for Esp (StructmsgDefinition.js)
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

EM.addModule("Esp-StructmsgDefinition", function(T, name) {

  /* ==================== StructmsgDefinition constructor ======================= */

  function StructmsgDefinition() {
    T.log('constructor called', 'info', 'StructmsgDefinition', true);

    var StmsgDefinition = this;
    var constructor = StmsgDefinition.constructor;
    StructmsgDefinition.superclass.constructor.apply(StmsgDefinition, arguments);

    StmsgDefinition.numFields = 0;
    StmsgDefinition.maxFields = 0;
    StmsgDefinition.name = null;
    StmsgDefinition.encoded = null;
    StmsgDefinition.encrypted = null;
    StmsgDefinition.todecode = null;
    StmsgDefinition.totalLgth = 0;
    StmsgDefinition.fieldInfos = null;
    StmsgDefinition.fieldNameInfos = null;

    T.log('constructor end', 'info', 'StructmsgDefinition', true);
  }

  T.extend(StructmsgDefinition, T.Defines, {
     my_name: "StructmsgDefinition",
     type_name: "structmsg_definition",
     flags: 0,

    /* ==================== toString ===================================== */

    toString: function () {
      var stmsgDef = this;
      return stmsgDef.mySelf()+"!"+stmsgDef.numFields+"!";
    },

    /* ==================== toDebugString ===================================== */
    toDebugString: function () {
      var stmsgDef = this;
      var idx;
      var str = stmsgDef.mySelf()+"\n";
      str += "  name:        "+(stmsgDef.name == null ? "null" : stmsgDef.name)+"\n";
      str += "  numFields:   "+stmsgDef.numFields+"\n";
      str += "  maxFields:   "+stmsgDef.maxFields+"\n";
      str += "  encoded:     "+(stmsgDef.encoded == null ? "No" : "Yes")+"\n";
      str += "  todecode:    "+(stmsgDef.todecode == null ? "No" : "Yes")+"\n";
      str += "  encrypted:   "+(stmsgDef.encrypted == null ? "No" : "Yes")+"\n";
      str += "  totalLgth:   "+stmsgDef.totalLgth+"\n";
      str += "  fieldInfos:  "+(stmsgDef.fieldInfos == null ? "null\n" : "")+"\n";
      idx = 0;
      while (idx < stmsgDef.numFields) {
        fieldInfo = stmsgDef.fieldInfos[idx];
        str += "    idx: "+idx+" "+fieldInfo.toDebugString();
        idx++;
      }
      return str;
    },

    /* ==================== addFieldDefinition ===================================== */
    addFieldDefinition: function (fieldName, fieldType, fieldLgth) {
      var stmsgDef = this;
      var fieldInfo;
      var fieldId;
      var fieldType;
      var idx;
      var obj = new Object();

      if (stmsgDef.numFields >= stmsgDef.maxFields) {
        return stmsgDef.STRUCT_MSG_ERR_DEFINITION_TOO_MANY_FIELDS;
      }
      fieldInfo = stmsgDef.fieldInfos[definition.numFields];
      obj.fieldId = -1;
      result = stmsgDef.fieldNameInfos.getFieldNameId(fieldName, obj, stmsgDef.STRUCT_MSG_INCR);
      if(result != stmsgDef.STRUCT_MSG_ERR_OK) return result;
      fieldInfo.fieldId = obj.fieldId;
      obj.value = -1;
      result = stmsgDef.getFieldTypeId(fieldType, obj);
      if(result != stmsgDef.STRUCT_MSG_ERR_OK) return result;
      fieldInfo.fieldType = obj.value;
      fieldInfo.fieldLgth = fieldLgth;
      stmsgDef.numFields++;
print("addDef: ",fieldName,"!",fieldInfo.toDebugString());
      return stmsgDef.STRUCT_MSG_ERR_OK;

    },

  });

  T.StructmsgDefinition = StructmsgDefinition;

  T.log("module: "+name+" initialised!", "info", "StructmsgDefinition.js");
}, "0.0.1", {});
