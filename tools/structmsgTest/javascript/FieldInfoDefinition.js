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
    T.log('constructor called', 'info', 'FieldInfoDefinition', true);

    var fldInfoDef = this;
    var constructor = fldInfoDef.constructor;
    FieldInfoDefinition.superclass.constructor.apply(fldInfoDef, arguments);

    fldInfoDef.fieldId = 0;
    fldInfoDef.fieldType = 0;
    fldInfoDef.fieldLgth = 0;

    T.log('constructor end', 'info', 'FieldInfoDefinition', true);
  }

  T.extend(FieldInfoDefinition, T.Defines, {
     my_name: "FieldInfoDefinition",
     type_name: "fieldInfo_definition",
     flags: 0,

    /* ==================== toString ===================================== */

    toString: function () {
      var fldInfoDef = this;
      return fldInfoDef.mySelf()+"!"+fldInfoDef.fieldType+"!";
    },

    /* ==================== toDebugString ===================================== */
    toDebugString: function () {
      var fldInfoDef = this;
      var str = fldInfoDef.mySelf()+"\n";
      return str;
    },

    /* ==================== showProps ============================== */
    showProps: function(val) {
      EM.log("props!", "info", "FieldInfoDefinition.js");
      for (var z in val) {
        EM.log("Z!"+z+"!"+val[z]+"!", "info", "FieldInfoDefinition.js");
      }
      EM.log("props END!", "info", "FieldInfoDefinition.js");
    },
  
  });

  T.FieldInfoDefinition = FieldInfoDefinition;

  T.log("module: "+name+" initialised!", "info", "FieldInfoDefinition.js");
}, "0.0.1", {});
