/*====================================================
 * A structured Message Support implementation in Javascript named Esp
 *
 * Released under BSD/MIT license.
 * (BSD license found at <http://www.tcl.tk/software/tcltk/license.html>)
 *
 * Arnulf P. Wiedemann arnulf@wiedemann-pri.de (C)2016
 */

if (0) {
    var esp = new EM.Defines();
    var val1 = esp.getErrIdString(255);
EM.log(val1, 'info', 'val1', true);
    var sm2 = new EM.StructmsgInfo();
    var val2 = sm2.toString();
EM.log(val2, 'info', 'val2', true);
    var sm3 = new EM.FieldInfo();
    var val3 = sm3.toString();
EM.log(val3, 'info', 'val3', true);
    var sm4 = new EM.FieldNameDefinitions();
    var val4 = sm4.toString();
EM.log(val4, 'info', 'val4', true);
    var sm5 = new EM.FieldInfoDefinition();
    var val5 = sm5.toString();
EM.log(val5, 'info', 'val5', true);
    var sm6 = new EM.HeaderInfo();
    var val6 = sm6.toString();
EM.log(val6, 'info', 'val6', true);
    var sm7 = new EM.MsgInfo();
    var val7 = sm7.toString();
EM.log(val7, 'info', 'val7', true);
    var sm8 = new EM.StructmsgDefinition();
    var val8 = sm8.toString();
EM.log(val8, 'info', 'val8', true);
    var sm9 = new EM.StructmsgDefinitions();
    var val9 = sm9.toString();
EM.log(val9, 'info', 'val9', true);
    var sm10 = new EM.Defs();
    var val10 = sm10.toString();
EM.log(val10, 'info', 'val10', true);
    var sm11 = new EM.Api();
    var val11 = sm11.toString();
EM.log(val11, 'info', 'val11', true);
    var sm12 = new EM.Cmd();
    var val12 = sm12.toString();
EM.log(val12, 'info', 'val12', true);
    var sm13 = new EM.structmsgEncDec();
    var val13 = sm13.toString();
EM.log(val13, 'info', 'val13', true);
    var fldinfo = new EM.FieldInfo();
    fldinfo.fieldStr="Hello World"
EM.log(fldinfo.fieldType+"!"+fldinfo.fieldStr, 'info', 'fldinfo', true);
}
    var def = new EM.Defines();
print("before props")
    def.showProps(def,def);
print("after props")


    function confirm(str) {
      EM.log(str, 'info', "confirm", true);
    }


EM.log('END', 'info', "End", true);
