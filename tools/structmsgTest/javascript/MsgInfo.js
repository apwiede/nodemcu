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

  /* ==================== MsgInfo constructor ======================= */

  function MsgInfo() {
    T.log('constructor called', 'info', 'MsgInfo', true);

    var msgInfo = this;
    var constructor = msgInfo.constructor;
    MsgInfo.superclass.constructor.apply(msgInfo, arguments);

    msgInfo.fieldInfos = null;
    msgInfo.tableFieldInfos = null;
    msgInfo.numFieldInfos = 0;
    msgInfo.maxFieldInfos = 0;
    msgInfo.numTableRows = 0;         // number of list rows
    msgInfo.numTableRowFields = 0;    // number of fields within a table row
    msgInfo.numRowFields = 0;         // for checking how many tableRowFields have been processed

    T.log('constructor end', 'info', 'MsgInfo', true);
  }

  T.extend(MsgInfo, T.Defines, {
     my_name: "MsgInfo",
     type_name: "msg_info",
     flags: 0,

    /* ==================== toString ===================================== */

    toString: function () {
      var msgInfo = this;
      return msgInfo.mySelf()+"!"+msgInfo.numFieldInfos+"!";
    },

    /* ==================== toDebugString ===================================== */
    toDebugString: function () {
      var msgInfo = this;
      var str = msgInfo.mySelf()+"\n";
      return str;
    },

  });

  T.MsgInfo = MsgInfo;

  T.log("module: "+name+" initialised!", "info", "MsgInfo.js");
}, "0.0.1", {});
