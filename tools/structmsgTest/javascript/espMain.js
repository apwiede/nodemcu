/*====================================================
 * A structured Message Support implementation in Javascript named Esp
 *
 * main functions for Esp (espMain.js)
 *
 * Part of this code is taken from:
 * http://yuilibrary.com/ YUI 3.3 version
 *
 * Released under BSD/MIT license.
 * (BSD license found at <http://www.tcl.tk/software/tcltk/license.html>)
 *
 * Arnulf P. Wiedemann arnulf@wiedemann-pri.de (C)2016
 */

/* =============================== Esp main ================================== */

//print("EMENV!"+typeof EM.Env+"!");
//for (var z in EM.Env) {
//print("Z!"+z+"!");
//}
EM.Env.mod_names = new Array();
EM.Env.mod_names.push("structmsgDefines");
EM.Env.mod_names.push("log");
EM.Env.mod_names.push("StructmsgDefinition");
EM.Env.mod_names.push("StructmsgDefinitions");
EM.Env.mod_names.push("FieldInfoDefinition");
EM.Env.mod_names.push("FieldNameDefinitions");
EM.Env.mod_names.push("FieldInfo");
EM.Env.mod_names.push("HeaderInfo");
EM.Env.mod_names.push("MsgInfo");
EM.Env.mod_names.push("StructmsgInfo");
EM.Env.mod_names.push("structmsgApi");
//EM.Env.mod_names.push("structmsgCmd");
EM.Env.mod_names.push("structmsgDefs");
EM.Env.mod_names.push("structmsgEncDec");
for (var i = 0; i < EM.Env.mod_names.length; i++) {
   var mod_name_part = EM.Env.mod_names[i];
print("MD!"+mod_name_part+"!");
  print("RE0!"+mod_name_part+"!"+typeof EM.Env.mods["Esp-"+mod_name_part].fn+"!");
   try {
     EM.Env.mods["Esp-"+mod_name_part].fn(EM, "Esp-"+mod_name_part);
   } catch(e) {
print("ERROR!"+e+"!"+mod_name_part+"!");
   }
}
