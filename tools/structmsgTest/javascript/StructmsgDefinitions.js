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

    var StructmsgDefinitions = this;
    StructmsgDefinitions.name = StructmsgDefinitions.constructor.NAME;
    StructmsgDefinitions.init.apply(StructmsgDefinitions, arguments);

    StructmsgDefinitions.numDefinitions = 0;
    StructmsgDefinitions.maxDefinitions = 0;
    StructmsgDefinitions.definitions = null;

    T.log('constructor end', 'info', 'StructmsgDefinitions', true);
  }


  StructmsgDefinitions.my_name = "Esp-StructmsgStructmsgDefinitions";
  StructmsgDefinitions.NAME = "StructmsgDefinitions";

  StructmsgDefinitions.prototype = {


    /* ==================== showProps ============================== */
    showProps: function(val) {
      EM.log("props!", "info", "StructmsgDefinitions.js");
      for (var z in val) {
        EM.log("Z!"+z+"!"+val[z]+"!", "info", "StructmsgDefinitions.js");
      }
      EM.log("props END!", "info", "StructmsgDefinitions.js");
    },
  
    /* ==================== mySelf ===================================== */
    mySelf: function () {
      return this.my_name+"!"+this.oid;
    },
  
  };

  StructmsgDefinitions.prototype.constructor = StructmsgDefinitions;
  T.StructmsgDefinitions = StructmsgDefinitions;

  T.log("module: "+name+" initialised!", "info", "StructmsgDefinitions.js");
}, "0.0.1", {});
