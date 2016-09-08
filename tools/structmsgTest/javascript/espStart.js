/*====================================================
 * A structured Message Support implementation in Javascript named Esp
 *
 * Released under BSD/MIT license.
 * (BSD license found at <http://www.tcl.tk/software/tcltk/license.html>)
 *
 * Arnulf P. Wiedemann arnulf@wiedemann-pri.de (C)2016
 */

    var esp = new EM.Defines();
    var val = esp.getErrIdString(255);
print("val: ",val);

    function evalTopLevel(script) {
      eval(script);
    }

    function confirm(str) {
      print("confirm!"+str+"!");
    }


print("END");
