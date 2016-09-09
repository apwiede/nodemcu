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
      var stmsgdef = this;
      var str = stmsgdef.mySelf()+"\n";
      return str;
    },

  });

  T.StructmsgDefinition = StructmsgDefinition;

  T.log("module: "+name+" initialised!", "info", "StructmsgDefinition.js");
}, "0.0.1", {});
