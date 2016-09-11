/*====================================================
 * A structured Message Support implementation in Javascript named Esp
 *
 * Structmsg FieldInfo for Esp (FieldInfo.js)
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

EM.addModule("Esp-FieldInfo", function(T, name) {

  /* ==================== FieldInfo constructor ======================= */

  function FieldInfo() {
    T.log('constructor called', '2.info', 'FieldInfo', true);

    var fieldInfo = this;
    var constructor = fieldInfo.constructor;
    FieldInfo.superclass.constructor.apply(fieldInfo, arguments);

    fieldInfo.fieldStr = null;
    fieldInfo.fieldType = 0;
    fieldInfo.fieldLgth = 0;
    fieldInfo.value = null;
    fieldInfo.flags = 0;

    T.log('constructor end', '2.info', 'FieldInfo', true);
  }

  T.extend(FieldInfo, T.Defines, {
     my_name: "FieldInfo",
     type_name: "field_info",
     flags: 0,

    /* ==================== toString ===================================== */

    toString: function () {
      var fInfo = this;
      return fInfo.mySelf()+"!"+fInfo.fieldType+"!";
    },

    /* ==================== toDebugString ===================================== */
    toDebugString: function () {
      var fInfo = this;
      var str = "    "+fInfo.mySelf()+"\n";
      str += "        fieldStr:   "+(fInfo.fieldStr == null ? "null" : fInfo.fieldStr)+"\n";
      str += "        fieldType:   "+fInfo.fieldType+"\n";
      str += "        fieldLgth:   "+fInfo.fieldLgth+"\n";
      str += "        value:       "+(fInfo.value == null ? "null" : fInfo.value)+"\n";
      str += "        flags:       "+fInfo.flags+"\n";
      return str;
    },

  });

  FieldInfo.prototype.constructor = FieldInfo;
  T.FieldInfo = FieldInfo;

  T.log("module: "+name+" initialised!", "2.info", "FieldInfo.js");
}, "0.0.1", {});
