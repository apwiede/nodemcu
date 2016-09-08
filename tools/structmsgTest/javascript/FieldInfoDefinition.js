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

    var FieldInfoDefinition = this;
    FieldInfoDefinition.name = FieldInfoDefinition.constructor.NAME;
    FieldInfoDefinition.init.apply(FieldInfoDefinition, arguments);

    FieldInfoDefinition.fieldId = 0;
    FieldInfoDefinition.fieldType = 0;
    FieldInfoDefinition.fieldLgth = 0;

    T.log('constructor end', 'info', 'FieldInfoDefinition', true);
  }


  FieldInfoDefinition.my_name = "Esp-FieldInfoDefinition";
  FieldInfoDefinition.NAME = "FieldInfoDefinition";

  FieldInfoDefinition.prototype = {


    /* ==================== showProps ============================== */
    showProps: function(val) {
      EM.log("props!", "info", "FieldInfoDefinition.js");
      for (var z in val) {
        EM.log("Z!"+z+"!"+val[z]+"!", "info", "FieldInfoDefinition.js");
      }
      EM.log("props END!", "info", "FieldInfoDefinition.js");
    },
  
    /* ==================== mySelf ===================================== */
    mySelf: function () {
      return this.my_name+"!"+this.oid;
    },
  
  };

  FieldInfoDefinition.prototype.constructor = FieldInfoDefinition;
  T.FieldInfoDefinition = FieldInfoDefinition;

  T.log("module: "+name+" initialised!", "info", "FieldInfoDefinition.js");
}, "0.0.1", {});
