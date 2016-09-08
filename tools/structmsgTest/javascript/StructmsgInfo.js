/*====================================================
 * A structured Message Support implementation in Javascript named Esp
 *
 * Structmsg StructmsgInfo for Esp (StructmsgInfo.js)
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

EM.addModule("Esp-StructmsgInfo", function(T, name) {

  function StructmsgInfo() {
    T.log('constructor called', 'info', 'StructmsgInfo', true);

    var StructmsgInfo = this;
    StructmsgInfo.name = StructmsgInfo.constructor.NAME;
    StructmsgInfo.init.apply(StructmsgInfo, arguments);


    StructmsgInfo.hdr = null;
    StructmsgInfo.msg = null;
    StructmsgInfo.handle = null;
    StructmsgInfo.flags = 0;
    StructmsgInfo.encoded = null;
    StructmsgInfo.todecode = null;
    StructmsgInfo.encrypted = null;
    StructmsgInfo.sequenceNum = 0;
    StructmsgInfo.handleHdrInfoPtr = null;

    T.log('constructor end', 'info', 'StructmsgInfo', true);
  }


  StructmsgInfo.my_name = "Esp-StructmsgInfo";
  StructmsgInfo.NAME = "StructmsgInfo";

  StructmsgInfo.prototype = {


    /* ==================== showProps ============================== */
    showProps: function(val) {
      EM.log("props!", "info", "StructmsgInfo.js");
      for (var z in val) {
        EM.log("Z!"+z+"!"+val[z]+"!", "info", "StructmsgInfo.js");
      }
      EM.log("props END!", "info", "StructmsgInfo.js");
    },
  
    /* ==================== mySelf ===================================== */
    mySelf: function () {
      return this.my_name+"!"+this.oid;
    },
  
  };

  StructmsgInfo.prototype.constructor = StructmsgInfo;
  T.StructmsgInfo = StructmsgInfo;

  T.log("module: "+name+" initialised!", "info", "StructmsgInfo.js");
}, "0.0.1", {});
