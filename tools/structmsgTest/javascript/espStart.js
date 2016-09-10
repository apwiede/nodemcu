/*====================================================
 * A structured Message Support implementation in Javascript named Esp
 *
 * Released under BSD/MIT license.
 * (BSD license found at <http://www.tcl.tk/software/tcltk/license.html>)
 *
 * Arnulf P. Wiedemann arnulf@wiedemann-pri.de (C)2016
 */

    var result;
    var stmsgApi = new EM.Api();
    handle = stmsgApi.create(10);
print("handle: ",handle);
    result = stmsgApi.add_field(handle, "@randomNum", "uint32_t", 4);
print("result: ",result);
    result = stmsgApi.add_field(handle, "@sequenceNum", "uint32_t", 4);
print("result: ",result);
    result = stmsgApi.add_field(handle, "@tablerows", "uint8_t", 2);
print("result: ",result);
    result = stmsgApi.add_field(handle, "@tablerowfields", "uint8_t", 3);
print("result: ",result);
    result = stmsgApi.add_field(handle, "ssid", "uint8_t*", 32);
print("result: ",result);
    result = stmsgApi.add_field(handle, "rssid", "uint8_t", 1);
print("result: ",result);
    result = stmsgApi.add_field(handle, "channel", "uint8_t", 1);
print("result: ",result);
print("Api2: ",stmsgApi.structmsg.handles[0].structmsg.toDebugString());

    result = stmsgApi.set_fieldValue(handle, "@src",123);
print("result: ",result);
    result = stmsgApi.set_fieldValue(handle, "@dst",456);
print("result: ",result);
    result = stmsgApi.set_fieldValue(handle, "@cmdKey",789);
print("result: ",result);
    result = stmsgApi.set_tableFieldValue(handle, "ssid",0,"Wiede70123456789");
print("result: ",result);
    result = stmsgApi.set_tableFieldValue(handle, "rssid",0,42);
print("result: ",result);
    result = stmsgApi.set_tableFieldValue(handle, "channel",0,5);
print("result: ",result);
    result = stmsgApi.set_tableFieldValue(handle, "ssid",1,"1und170123456789");
print("result: ",result);
    result = stmsgApi.set_tableFieldValue(handle, "rssid",1,-76);
print("result: ",result);
    result = stmsgApi.set_tableFieldValue(handle, "channel",1,7);
print("result: ",result);

if (0) {
    result = stmsgApi.encode(handle);
print("result: ",result);

print("DECODE");
    result = stmsgApi.decode(handle);
print("result: ",result);
}

print("create def");
    result = stmsgApi.create_definition("aplist", 15);
print("result: ",result);

print("addField def");
    result = stmsgApi.add_fieldDefinition("aplist", "@src","uint16_t",2);
print("result: ",result);

print("addField def");
    result = stmsgApi.add_fieldDefinition("aplist", "@dst","uint16_t",2);
print("result: ",result);

print("addField def");
    result = stmsgApi.add_fieldDefinition("aplist", "@totalLgth","uint16_t",2);
print("result: ",result);

print("addField def");
    result = stmsgApi.add_fieldDefinition("aplist", "@cmdKey","uint16_t",2);
print("result: ",result);

print("addField def");
    result = stmsgApi.add_fieldDefinition("aplist", "@cmdLgth","uint16_t",2);
print("result: ",result);

print("addField def");
    result = stmsgApi.add_fieldDefinition("aplist", "@tablerows","uint8_t",2);
print("result: ",result);

print("addField def");
    result = stmsgApi.add_fieldDefinition("aplist", "@tablerowfields","uint8_t",3);
print("result: ",result);

print("addField def");
    result = stmsgApi.add_fieldDefinition("aplist", "ssid","uint8_t*",32);
print("result: ",result);

print("addField def");
    result = stmsgApi.add_fieldDefinition("aplist", "rssid","uint8_t",1);
print("result: ",result);

print("addField def");
    result = stmsgApi.add_fieldDefinition("aplist", "channel","uint8_t",1);
print("result: ",result);


    function confirm(str) {
      EM.log(str, 'info', "confirm", true);
    }


EM.log('END', 'info', "End", true);
