/*====================================================
 * A structured Message Support implementation in Javascript named Esp
 *
 * Structmsg FieldNameDefinitions for Esp (FieldNameDefinitions.js)
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

EM.addModule("Esp-FieldNameDefinitions", function(T, name) {

  /* ==================== FieldNameDefinition constructor ======================= */

  function FieldNameDefinitions() {
    T.log('constructor called', 'info', 'FieldNameDefinitions', true);

    var fieldNameDefinitions = this;
    var constructor = fieldNameDefinitions.constructor;
    FieldNameDefinitions.superclass.constructor.apply(fieldNameDefinitions, arguments);

    fieldNameDefinitions.numDefinitions = 0;
    fieldNameDefinitions.maxDefinitions = 0;
    fieldNameDefinitions.definitions = null;

    T.log('constructor end', 'info', 'FieldNameDefinitions', true);
  }

  T.extend(FieldNameDefinitions, T.Defines, {
     my_name: "FieldNameDefinitions",
     type_name: "fieldName_definitions",
     flags: 0,

    /* ==================== toString ===================================== */

    toString: function () {
      var fndef = this;
      return fndef.mySelf()+"!"+fndef.numDefinitions+"!";
    },

    /* ==================== toDebugString ===================================== */
    toDebugString: function () {
      var fndef = this;
      var str = fndef.mySelf()+"\n";
      return str;
    },

  });

  T.FieldNameDefinitions = FieldNameDefinitions;

  T.log("module: "+name+" initialised!", "info", "FieldNameDefinitions.js");
}, "0.0.1", {});
