/*====================================================
 * A structured Message Support implementation in Javascript named Esp
 *
 * Structmsg Cmd for Esp (structmsgCmd.js)
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

EM.addModule("Esp-structmsgCmd", function(T, name) {

  function Cmd() {
    T.log('constructor called', 'info', 'structmsgCmd', true);

    var Cmd = this;
    Cmd.name = Cmd.constructor.NAME;
    Cmd.init.apply(Cmd, arguments);

    T.log('constructor end', 'info', 'structmsgCmd', true);
  }

  Cmd.structmsg = null;

  Cmd.my_name = "Esp-StructmsgCmd";
  Cmd.NAME = "structmsgCmd";

  Cmd.prototype = {

    // ============================= checkOKOrErr ========================

    
    dummy: function() {
    },


    /* ==================== showProps ============================== */
    showProps: function(val) {
      EM.log("props!", "info", "structmsgCmd.js");
      for (var z in val) {
        EM.log("Z!"+z+"!"+val[z]+"!", "info", "structmsgCmd.js");
      }
      EM.log("props END!", "info", "structmsgCmd.js");
    },
  
    /* ==================== mySelf ===================================== */
    mySelf: function () {
      return this.my_name+"!"+this.oid;
    },
  
  };

  Cmd.prototype.constructor = Cmd;
  T.Cmd = Cmd;

  T.log("module: "+name+" initialised!", "info", "structmsgCmd.js");
}, "0.0.1", {});
