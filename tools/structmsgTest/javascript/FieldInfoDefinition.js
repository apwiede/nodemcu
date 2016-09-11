/*====================================================
 * A structured Message Support implementation in Javascript named Esp
 *
 * Structmsg FieldInfoDefinition for Esp (FieldInfoDefinition.js)
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

EM.addModule("Esp-FieldInfoDefinition", function(T, name) {

  /* ==================== FieldInfoDefinition constructor ======================= */

  function FieldInfoDefinition() {
    T.log('constructor called', '2.info', 'FieldInfoDefinition', true);

    var fldInfoDef = this;
    var constructor = fldInfoDef.constructor;
    FieldInfoDefinition.superclass.constructor.apply(fldInfoDef, arguments);

    fldInfoDef.fieldId = 0;
    fldInfoDef.fieldType = 0;
    fldInfoDef.fieldLgth = 0;

    T.log('constructor end', '2.info', 'FieldInfoDefinition', true);
  }

  T.extend(FieldInfoDefinition, T.Defines, {
     my_name: "FieldInfoDefinition",
     type_name: "fieldInfo_definition",
     flags: 0,

    /* ==================== toString ===================================== */

    toString: function () {
      var fldInfoDef = this;
      return fldInfoDef.mySelf()+"!";
    },

    /* ==================== toDebugString ===================================== */
    toDebugString: function () {
      var fldInfoDef = this;
      var str = fldInfoDef.mySelf()+"\n";
      str += "      fieldId:   "+fldInfoDef.fieldId+"\n";
      str += "      fieldType: "+fldInfoDef.fieldType+"\n";
      str += "      fieldLgth: "+fldInfoDef.fieldLgth+"\n";
      return str;
    },

  });

  T.FieldInfoDefinition = FieldInfoDefinition;

  T.log("module: "+name+" initialised!", "2.info", "FieldInfoDefinition.js");
}, "0.0.1", {});
