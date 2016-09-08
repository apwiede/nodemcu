/*====================================================
 * A structured Message Support implementation in Javascript named Esp
 *
 * Structmsg EncDec for Esp (structmsgEncDec.js)
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

EM.addModule("Esp-structmsgEncDec", function(T, name) {

  function EncDec() {
    T.log('constructor called', 'info', 'structmsgEncDec', true);

    var EncDec = this;
    EncDec.name = EncDec.constructor.NAME;
    EncDec.init.apply(EncDec, arguments);

    T.log('constructor end', 'info', 'structmsgEncDec', true);
  }

  EncDec.structmsg = null;

  EncDec.my_name = "Esp-StructmsgEncDec";
  EncDec.NAME = "structmsgEncDec";

  EncDec.prototype = {


    /* ==================== showProps ============================== */
    showProps: function(val) {
      EM.log("props!", "info", "structmsgEncDec.js");
      for (var z in val) {
        EM.log("Z!"+z+"!"+val[z]+"!", "info", "structmsgEncDec.js");
      }
      EM.log("props END!", "info", "structmsgEncDec.js");
    },
  
    /* ==================== mySelf ===================================== */
    mySelf: function () {
      return this.my_name+"!"+this.oid;
    },
  
  };

  EncDec.prototype.constructor = EncDec;
  T.EncDec = EncDec;

  T.log("module: "+name+" initialised!", "info", "structmsgEncDec.js");
}, "0.0.1", {});
