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

  /* ==================== StructmsgInfo constructor ======================= */

  function StructmsgInfo() {
    T.log('constructor called', 'info', 'StructmsgInfo', true);

    var StmsgInfo = this;

    var constructor = StmsgInfo.constructor;
    StructmsgInfo.superclass.constructor.apply(StmsgInfo, arguments);

    StmsgInfo.hdr = null;
    StmsgInfo.msg = null;
    StmsgInfo.handle = "unknown";
    StmsgInfo.flags = 0;
    StmsgInfo.encoded = null;
    StmsgInfo.todecode = null;
    StmsgInfo.encrypted = null;
    StmsgInfo.sequenceNum = 0;
    StmsgInfo.handleHdrInfoPtr = null;

    T.log('constructor end', 'info', 'StructmsgInfo', true);
  }


  T.extend(StructmsgInfo, T.Defines, {
     my_name: "StructmsgInfo",
     type_name: "structmsg_info",
     flags: 0,

    /* ==================== toString ===================================== */

    toString: function () {
      var stmsgInfo = this;
      return stmsgInfo.mySelf()+"!"+stmsgInfo.handle+"!";
    },

    /* ==================== toDebugString ===================================== */
    toDebugString: function () {
      var stmsgInfo = this;
      var str = stmsgInfo.mySelf()+"\n";
      return str;
    },

    /* ==================== showProps ============================== */
    showProps: function(val) {
      EM.log("props!", "info", "StructmsgInfo.js");
      for (var z in val) {
        EM.log("Z!"+z+"!"+val[z]+"!", "info", "StructmsgInfo.js");
      }
      EM.log("props END!", "info", "StructmsgInfo.js");
    },
  
  });

  T.StructmsgInfo = StructmsgInfo;

  T.log("module: "+name+" initialised!", "info", "StructmsgInfo.js");
}, "0.0.1", {});
