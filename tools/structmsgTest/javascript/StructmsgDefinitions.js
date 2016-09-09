/*====================================================
 * A structured Message Support implementation in Javascript named Esp
 *
 * Structmsg StructmsgDefinitions for Esp (StructmsgDefinitions.js)
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

EM.addModule("Esp-StructmsgDefinitions", function(T, name) {

  /* ==================== StructmsgDefinitions constructor ======================= */

  function StructmsgDefinitions() {
    T.log('constructor called', 'info', 'StructmsgDefinitions', true);

    var StmsgDefinitions = this;
    var constructor = StmsgDefinitions.constructor;
    StructmsgDefinitions.superclass.constructor.apply(StmsgDefinitions, arguments);

    StmsgDefinitions.numDefinitions = 0;
    StmsgDefinitions.maxDefinitions = 0;
    StmsgDefinitions.definitions = null;

    T.log('constructor end', 'info', 'StructmsgDefinitions', true);
  }

  T.extend(StructmsgDefinitions, T.Defines, {
     my_name: "StructmsgDefinitions",
     type_name: "structmsg_definitions",
     flags: 0,

    /* ==================== toString ===================================== */

    toString: function () {
      var stmsgDefs = this;
      return stmsgDefs.mySelf()+"!"+stmsgDefs.numDefinitions+"!";
    },

    /* ==================== toDebugString ===================================== */
    toDebugString: function () {
      var stmsgDefs = this;
      var str = stmsgDefs.mySelf()+"\n";
      return str;
    },

  });

  T.StructmsgDefinitions = StructmsgDefinitions;

  T.log("module: "+name+" initialised!", "info", "StructmsgDefinitions.js");
}, "0.0.1", {});
