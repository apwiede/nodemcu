/*====================================================
 * A structured Message Support implementation in Javascript named Esp
 *
 * Structmsg FieldInfo for Esp (FieldInfo.js)
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

EM.addModule("Esp-FieldInfo", function(T, name) {

  function FieldInfo() {
    T.log('constructor called', 'info', 'FieldInfo', true);

    var FieldInfo = this;
    FieldInfo.name = FieldInfo.constructor.NAME;
    FieldInfo.init.apply(FieldInfo, arguments);

    FieldInfo.fieldStr = null;
    FieldInfo.fieldType = 0;
    FieldInfo.fieldLgth = 0;
    FieldInfo.value = null;
    FieldInfo.flags = 0;

    T.log('constructor end', 'info', 'FieldInfo', true);
  }


  FieldInfo.my_name = "Esp-FieldInfo";
  FieldInfo.NAME = "FieldInfo";

  FieldInfo.prototype = {


    /* ==================== showProps ============================== */
    showProps: function(val) {
      EM.log("props!", "info", "FieldInfo.js");
      for (var z in val) {
        EM.log("Z!"+z+"!"+val[z]+"!", "info", "FieldInfo.js");
      }
      EM.log("props END!", "info", "FieldInfo.js");
    },
  
    /* ==================== mySelf ===================================== */
    mySelf: function () {
      return this.my_name+"!"+this.oid;
    },
  
  };

  FieldInfo.prototype.constructor = FieldInfo;
  T.FieldInfo = FieldInfo;

  T.log("module: "+name+" initialised!", "info", "FieldInfo.js");
}, "0.0.1", {});
