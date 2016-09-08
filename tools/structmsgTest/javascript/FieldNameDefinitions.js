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

  function FieldNameDefinitions() {
    T.log('constructor called', 'info', 'FieldNameDefinitions', true);

    var FieldNameDefinitions = this;
    FieldNameDefinitions.name = FieldNameDefinitions.constructor.NAME;
    FieldNameDefinitions.init.apply(FieldNameDefinitions, arguments);

    FieldNameDefinitions.numDefinitions = 0;
    FieldNameDefinitions.maxDefinitions = 0;
    FieldNameDefinitions.definitions = null;

    T.log('constructor end', 'info', 'FieldNameDefinitions', true);
  }


  FieldNameDefinitions.my_name = "Esp-FieldNameDefinitions";
  FieldNameDefinitions.NAME = "FieldNameDefinitions";

  FieldNameDefinitions.prototype = {


    /* ==================== showProps ============================== */
    showProps: function(val) {
      EM.log("props!", "info", "FieldNameDefinitions.js");
      for (var z in val) {
        EM.log("Z!"+z+"!"+val[z]+"!", "info", "FieldNameDefinitions.js");
      }
      EM.log("props END!", "info", "FieldNameDefinitions.js");
    },
  
    /* ==================== mySelf ===================================== */
    mySelf: function () {
      return this.my_name+"!"+this.oid;
    },
  
  };

  FieldNameDefinitions.prototype.constructor = FieldNameDefinitions;
  T.FieldNameDefinitions = FieldNameDefinitions;

  T.log("module: "+name+" initialised!", "info", "FieldNameDefinitions.js");
}, "0.0.1", {});
