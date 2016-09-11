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
    function checkResult(result) {
      if (result != "OK") {
        print("ERROR: ",result);
      }
    }

EM.log('Start', 'info', "espStart.js", true);
    handle = stmsgApi.create(10);
EM.log('handle:'+handle, 'info', "espStart.js", true);
    result = stmsgApi.add_field(handle, "@randomNum", "uint32_t", 4);
    checkResult(result);
    result = stmsgApi.add_field(handle, "@sequenceNum", "uint32_t", 4);
    checkResult(result);
    result = stmsgApi.add_field(handle, "@tablerows", "uint8_t", 2);
    checkResult(result);
    result = stmsgApi.add_field(handle, "@tablerowfields", "uint8_t", 3);
    checkResult(result);
    result = stmsgApi.add_field(handle, "ssid", "uint8_t*", 32);
    checkResult(result);
    result = stmsgApi.add_field(handle, "rssid", "uint8_t", 1);
    checkResult(result);
    result = stmsgApi.add_field(handle, "channel", "uint8_t", 1);
    checkResult(result);

    result = stmsgApi.set_fieldValue(handle, "@src",123);
    checkResult(result);
    result = stmsgApi.set_fieldValue(handle, "@dst",456);
    checkResult(result);
    result = stmsgApi.set_fieldValue(handle, "@cmdKey",789);
    checkResult(result);
    result = stmsgApi.set_tableFieldValue(handle, "ssid",0,"Wiede70123456789");
    checkResult(result);
    result = stmsgApi.set_tableFieldValue(handle, "rssid",0,42);
    checkResult(result);
    result = stmsgApi.set_tableFieldValue(handle, "channel",0,5);
    checkResult(result);
    result = stmsgApi.set_tableFieldValue(handle, "ssid",1,"1und170123456789");
    checkResult(result);
    result = stmsgApi.set_tableFieldValue(handle, "rssid",1,-76);
    result = stmsgApi.set_tableFieldValue(handle, "channel",1,7);
    checkResult(result);

if (0) {
    result = stmsgApi.encode(handle);
    checkResult(result);

EM.log('Decode', 'info', "espStart.js", true);
    result = stmsgApi.decode(handle);
    checkResult(result);
}

    result = stmsgApi.create_definition("aplist", 15);
    checkResult(result);

    result = stmsgApi.add_fieldDefinition("aplist", "@src","uint16_t",2);
    checkResult(result);

    result = stmsgApi.add_fieldDefinition("aplist", "@dst","uint16_t",2);
    checkResult(result);

    result = stmsgApi.add_fieldDefinition("aplist", "@totalLgth","uint16_t",2);
    checkResult(result);

    result = stmsgApi.add_fieldDefinition("aplist", "@cmdKey","uint16_t",2);
    checkResult(result);

    result = stmsgApi.add_fieldDefinition("aplist", "@cmdLgth","uint16_t",2);
    checkResult(result);

    result = stmsgApi.add_fieldDefinition("aplist", "@tablerows","uint8_t",2);
    checkResult(result);

    result = stmsgApi.add_fieldDefinition("aplist", "@tablerowfields","uint8_t",3);
    checkResult(result);

    result = stmsgApi.add_fieldDefinition("aplist", "ssid","uint8_t*",32);
    checkResult(result);

    result = stmsgApi.add_fieldDefinition("aplist", "rssid","uint8_t",1);
    checkResult(result);

    result = stmsgApi.add_fieldDefinition("aplist", "channel","uint8_t",1);
    checkResult(result);

EM.log('encodeFieldDefinition', 'info', "espStart.js", true);
    var data = new Object();
    data.data = null;
    result = stmsgApi.encode_fieldDefinition("aplist", data);
    checkResult(result);


    function confirm(str) {
      EM.log(str, 'info', "confirm", true);
    }


EM.log('END', 'info', "EspStart.js", true);
