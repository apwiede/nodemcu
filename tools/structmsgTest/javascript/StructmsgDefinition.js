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

    var StructmsgDefinition = this;
    StructmsgDefinition.name = StructmsgDefinition.constructor.NAME;
    StructmsgDefinition.init.apply(StructmsgDefinition, arguments);

    StructmsgDefinition.numFields = 0;
    StructmsgDefinition.maxFields = 0;
    StructmsgDefinition.name = null;
    StructmsgDefinition.encoded = null;
    StructmsgDefinition.encrypted = null;
    StructmsgDefinition.todecode = null;
    StructmsgDefinition.totalLgth = 0;
    StructmsgDefinition.fieldInfos = null;

    T.log('constructor end', 'info', 'StructmsgDefinition', true);
  }


  StructmsgDefinition.my_name = "Esp-StructmsgStructmsgDefinition";
  StructmsgDefinition.NAME = "StructmsgDefinition";

  StructmsgDefinition.prototype = {


    /* ==================== showProps ============================== */
    showProps: function(val) {
      EM.log("props!", "info", "StructmsgDefinition.js");
      for (var z in val) {
        EM.log("Z!"+z+"!"+val[z]+"!", "info", "StructmsgDefinition.js");
      }
      EM.log("props END!", "info", "StructmsgDefinition.js");
    },
  
    /* ==================== mySelf ===================================== */
    mySelf: function () {
      return this.my_name+"!"+this.oid;
    },
  
  };

  StructmsgDefinition.prototype.constructor = StructmsgDefinition;
  T.StructmsgDefinition = StructmsgDefinition;

  T.log("module: "+name+" initialised!", "info", "StructmsgDefinition.js");
}, "0.0.1", {});
