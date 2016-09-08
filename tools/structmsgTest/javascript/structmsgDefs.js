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

  function Defs() {
    T.log('constructor called', 'info', 'structmsgDefs', true);

    var Defs = this;
    Defs.name = Defs.constructor.NAME;
    Defs.init.apply(Defs, arguments);

    T.log('constructor end', 'info', 'structmsgDefs', true);
  }

  Defs.structmsg = null;

  Defs.my_name = "Esp-StructmsgDefs";
  Defs.NAME = "structmsgDefs";

  Defs.prototype = {


    /* ==================== showProps ============================== */
    showProps: function(val) {
      EM.log("props!", "info", "structmsgDefs.js");
      for (var z in val) {
        EM.log("Z!"+z+"!"+val[z]+"!", "info", "structmsgDefs.js");
      }
      EM.log("props END!", "info", "structmsgDefs.js");
    },
  
    /* ==================== mySelf ===================================== */
    mySelf: function () {
      return this.my_name+"!"+this.oid;
    },
  
  };

  Defs.prototype.constructor = Defs;
  T.Defs = Defs;

  T.log("module: "+name+" initialised!", "info", "structmsgDefs.js");
}, "0.0.1", {});
