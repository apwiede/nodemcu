/*====================================================
 * A structured Message Support implementation in Javascript named Esp
 *
 * Structmsg FieldNameInfo for Esp (FieldNameInfo.js)
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

EM.addModule("Esp-FieldNameInfo", function(T, name) {

  /* ==================== FieldNameInfo constructor ======================= */

  function FieldNameInfo() {
    T.log('constructor called', '2.info', 'FieldNameInfo', true);

    var fieldNameInfo = this;
    var constructor = fieldNameInfo.constructor;
    FieldNameInfo.superclass.constructor.apply(fieldNameInfo, arguments);

    fieldNameInfo.fieldName = null;
    fieldNameInfo.fieldType = 0;
    fieldNameInfo.fieldId = 0;
    fieldNameInfo.refCnt = 0;

    T.log('constructor end', '2.info', 'FieldNameInfo', true);
  }

  T.extend(FieldNameInfo, T.Defines, {
     my_name: "FieldNameInfo",
     type_name: "fieldName_info",
     flags: 0,

    /* ==================== toString ===================================== */

    toString: function () {
      var fndef = this;
      return fndef.mySelf()+"!";
    },

    /* ==================== toDebugString ===================================== */
    toDebugString: function () {
      var fndef = this;
      var str = fndef.mySelf()+"\n";
      str += "        fieldName:   "+(fndef.fieldName == null ? "null" : fndef.fieldName)+"\n";
      str += "        fieldType:   "+fndef.fieldType+"\n";
      str += "        fieldId:     "+fndef.fieldId+"\n";
      str += "        refCnt:      "+fndef.refCnt+"\n";
      return str;
    },

  });

  T.FieldNameInfo = FieldNameInfo;

  T.log("module: "+name+" initialised!", "2.info", "FieldNameInfo.js");
}, "0.0.1", {});
