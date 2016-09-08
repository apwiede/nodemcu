/*====================================================
 * A structured Message Support implementation in Javascript named Esp
 *
 * Released under BSD/MIT license.
 * (BSD license found at <http://www.tcl.tk/software/tcltk/license.html>)
 *
 * Arnulf P. Wiedemann arnulf@wiedemann-pri.de (C)2016
 */

    var esp = new EM.Defines();
    var val1 = esp.getErrIdString(255);
print("val1: ",val1);
    var sm2 = new EM.StructmsgInfo();
    var val2 = sm2.toString();
print("val2: ",val2);
    var sm3 = new EM.FieldInfo();
    var val3 = sm3.toString();
print("val3: ",val3);
    var sm4 = new EM.FieldNameDefinitions();
    var val4 = sm4.toString();
print("val4: ",val4);
    var sm5 = new EM.FieldInfoDefinition();
    var val5 = sm5.toString();
print("val5: ",val5);
    var sm6 = new EM.HeaderInfo();
    var val6 = sm6.toString();
print("val6: ",val6);
    var sm7 = new EM.MsgInfo();
    var val7 = sm7.toString();
print("val7: ",val7);
    var sm8 = new EM.StructmsgDefinition();
    var val8 = sm8.toString();
print("val8: ",val8);
    var sm9 = new EM.StructmsgDefinitions();
    var val9 = sm9.toString();
print("val9: ",val9);
    var sm10 = new EM.Defs();
    var val10 = sm10.toString();
print("val10: ",val10);
    var sm11 = new EM.Api();
    var val11 = sm11.toString();
print("val11: ",val11);
    var sm12 = new EM.Cmd();
    var val12 = sm12.toString();
print("val12: ",val12);
    var sm13 = new EM.structmsgEncDec();
    var val13 = sm13.toString();
print("val13: ",val13);

    function evalTopLevel(script) {
      eval(script);
    }

    function confirm(str) {
      print("confirm!"+str+"!");
    }


print("END");
