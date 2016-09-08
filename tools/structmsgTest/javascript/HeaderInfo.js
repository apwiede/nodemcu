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

    var HeaderInfo = this;
    HeaderInfo.name = HeaderInfo.constructor.NAME;
    HeaderInfo.init.apply(HeaderInfo, arguments);

    HeaderInfo.src = 0;
    HeaderInfo.dst = 0;
    HeaderInfo.totalLgth = 0;
    HeaderInfo.cmdKey = 0;
    HeaderInfo.cmdLgth = 0;
    HeaderInfo.hdrId = null;

    T.log('constructor end', 'info', 'HeaderInfo', true);
  }


  HeaderInfo.my_name = "Esp-HeaderInfo";
  HeaderInfo.NAME = "HeaderInfo";

  HeaderInfo.prototype = {


    /* ==================== showProps ============================== */
    showProps: function(val) {
      EM.log("props!", "info", "HeaderInfo.js");
      for (var z in val) {
        EM.log("Z!"+z+"!"+val[z]+"!", "info", "HeaderInfo.js");
      }
      EM.log("props END!", "info", "HeaderInfo.js");
    },
  
    /* ==================== mySelf ===================================== */
    mySelf: function () {
      return this.my_name+"!"+this.oid;
    },
  
  };

  HeaderInfo.prototype.constructor = HeaderInfo;
  T.HeaderInfo = HeaderInfo;

  T.log("module: "+name+" initialised!", "info", "HeaderInfo.js");
}, "0.0.1", {});
