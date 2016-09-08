/*====================================================
 * A structured Message Support implementation in Javascript named Esp
 *
 * Structmsg MsgInfo for Esp (MsgInfo.js)
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

EM.addModule("Esp-MsgInfo", function(T, name) {

  function MsgInfo() {
    T.log('constructor called', 'info', 'MsgInfo', true);

    var MsgInfo = this;
    MsgInfo.name = MsgInfo.constructor.NAME;
    MsgInfo.init.apply(MsgInfo, arguments);

    MsgInfo.fieldInfos = null;
    MsgInfo.tableFieldInfos = null;
    MsgInfo.numFieldInfos = 0;
    MsgInfo.maxFieldInfos = 0;
    MsgInfo.numTableRows = 0;         // number of list rows
    MsgInfo.numTableRowFields = 0;    // number of fields within a table row
    MsgInfo.numRowFields = 0;         // for checking how many tableRowFields have been processed

    T.log('constructor end', 'info', 'MsgInfo', true);
  }


  MsgInfo.my_name = "Esp-MsgInfo";
  MsgInfo.NAME = "MsgInfo";

  MsgInfo.prototype = {


    /* ==================== showProps ============================== */
    showProps: function(val) {
      EM.log("props!", "info", "MsgInfo.js");
      for (var z in val) {
        EM.log("Z!"+z+"!"+val[z]+"!", "info", "MsgInfo.js");
      }
      EM.log("props END!", "info", "MsgInfo.js");
    },
  
    /* ==================== mySelf ===================================== */
    mySelf: function () {
      return this.my_name+"!"+this.oid;
    },
  
  };

  MsgInfo.prototype.constructor = MsgInfo;
  T.MsgInfo = MsgInfo;

  T.log("module: "+name+" initialised!", "info", "MsgInfo.js");
}, "0.0.1", {});
