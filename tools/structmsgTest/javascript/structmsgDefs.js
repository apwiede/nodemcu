/*====================================================
 * A structured Message Support implementation in Javascript named Esp
 *
 * Structmsg Defs for Esp (structmsgDefs.js)
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

EM.addModule("Esp-structmsgDefs", function(T, name) {

  /* ==================== Defs constructor ======================= */

  function Defs() {
    T.log('constructor called', 'info', 'structmsgDefs', true);

    var defs = this;
    var constructor = defs.constructor;
    Defs.superclass.constructor.apply(defs, arguments);

    T.log('constructor end', 'info', 'structmsgDefs', true);
  }

  T.extend(Defs, T.StructmsgInfos, {
     my_name: "Defs",
     type_name: "defs",
     flags: 0,

    /* ==================== toString ===================================== */

    toString: function () {
      var defs = this;
      return defs.mySelf()+"!";
    },

    /* ==================== toDebugString ===================================== */
    toDebugString: function () {
      var defs = this;
      var str = defs.mySelf()+"\n";
      return str;
    },

  });

  T.Defs = Defs;

  T.log("module: "+name+" initialised!", "info", "structmsgDefs.js");
}, "0.0.1", {});
