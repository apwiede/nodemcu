/*====================================================
 * A structured Message Support implementation in Javascript named Esp
 *
 * Structmsg StructmsgInfos for Esp (StructmsgInfos.js)
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

EM.addModule("Esp-StructmsgInfos", function(T, name) {

  /* ==================== StructmsgInfos constructor ======================= */

  function StructmsgInfos() {
    T.log('constructor called', 'info', 'StructmsgInfos', true);

    var StmsgInfos = this;

    var constructor = StmsgInfos.constructor;
    StructmsgInfos.superclass.constructor.apply(StmsgInfos, arguments);

    T.Defines.StructmsgInfosOid++;
    StmsgInfos.oid = T.Defines.StructmsgInfosOid;
    StmsgInfos.numHandles = 0;
    StmsgInfos.numHeaders = 0;
    StmsgInfos.handles = new Array();
    StmsgInfos.headers = new Array();

    T.log('constructor end', 'info', 'StructmsgInfos', true);
  }


  T.extend(StructmsgInfos, T.Defines, {
     my_name: "StructmsgInfos",
     type_name: "structmsg_infos",
     flags: 0,

    /* ==================== toString ===================================== */

    toString: function () {
      var stmsgInfos = this;
      return stmsgInfos.mySelf()+"!"+stmsgInfos.oid+"!";
    },

    /* ==================== toDebugString ===================================== */
    toDebugString: function () {
      var stmsgInfos = this;
      var str = stmsgInfos.mySelf()+"\n";
      str += "  numHandles:     "+stmsgInfos.numHandles+"\n";
      str += "  numHeaders:     "+stmsgInfos.numHeaders+"\n";
      return str;
    },

    /* ==================== create ===================================== */
    create: function (numFields) {
      stmsgInfo = new T.StructmsgInfo();
      structmsgInfo=stmsgInfo.create(numFields);
print("stmsgInfo: ",structmsgInfo.toDebugString());
    },

  });

  T.StructmsgInfos = StructmsgInfos;

  T.log("module: "+name+" initialised!", "info", "StructmsgInfos.js");
}, "0.0.1", {});
