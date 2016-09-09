/*====================================================
 * A structured Message Support implementation in Javascript named Esp
 *
 * Structmsg HeaderInfo for Esp (HeaderInfo.js)
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

EM.addModule("Esp-HeaderInfo", function(T, name) {

  /* ==================== HeaderInfo constructor ======================= */

  function HeaderInfo() {
    T.log('constructor called', 'info', 'HeaderInfo', true);

    var hdrInfo = this;
    var constructor = hdrInfo.constructor;
    HeaderInfo.superclass.constructor.apply(hdrInfo, arguments);

    hdrInfo.src = 0;
    hdrInfo.dst = 0;
    hdrInfo.totalLgth = 0;
    hdrInfo.cmdKey = 0;
    hdrInfo.cmdLgth = 0;
    hdrInfo.hdrId = null;

    T.log('constructor end', 'info', 'HeaderInfo', true);
  }

  T.extend(HeaderInfo, T.Defines, {
     my_name: "HeaderInfo",
     type_name: "header_info",
     flags: 0,

    /* ==================== toString ===================================== */

    toString: function () {
      var hdrInfo = this;
      return hdrInfo.mySelf()+"!"+hdrInfo.src+"!";
    },

    /* ==================== toDebugString ===================================== */
    toDebugString: function () {
      var hdrInfo = this;
      var str = hdrInfo.mySelf()+"\n";
      return str;
    },

  });

  T.HeaderInfo = HeaderInfo;

  T.log("module: "+name+" initialised!", "info", "HeaderInfo.js");
}, "0.0.1", {});
