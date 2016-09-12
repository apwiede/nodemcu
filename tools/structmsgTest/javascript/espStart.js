/*====================================================
 * A structured Message Support implementation in Javascript named Esp
 *
 * Released under BSD/MIT license.
 * (BSD license found at <http://www.tcl.tk/software/tcltk/license.html>)
 *
 * Arnulf P. Wiedemann arnulf@wiedemann-pri.de (C)2016
 */

    var cryptkey = 'a1b2c3d4e5f6g7h8';
    var iv = cryptkey;
    var result;
    var result_data = new Object();
    var stmsgApi = new EM.Api();
    function checkResult(result) {
      if (result != "OK") {
        print("ERROR: ",result);
      }
    }

EM.log('Start', '1.info', "espStart.js", true);
    result_data.handle = null;
    result = stmsgApi.create(10, result_data);
    checkResult(result);
    handle=result_data.handle;

EM.log('handle:'+result_data.handle, '1.info', "espStart.js", true);
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
    result = stmsgApi.set_tableFieldValue(handle, "ssid",0,"Wiede70123456789abdefghijklmnopq");
    checkResult(result);
    result = stmsgApi.set_tableFieldValue(handle, "rssid",0,42);
    checkResult(result);
    result = stmsgApi.set_tableFieldValue(handle, "channel",0,5);
    checkResult(result);
    result = stmsgApi.set_tableFieldValue(handle, "ssid",1,"1und170123456789abdefghijklmnopq");
    checkResult(result);
    result = stmsgApi.set_tableFieldValue(handle, "rssid",1,-76);
    result = stmsgApi.set_tableFieldValue(handle, "channel",1,7);
    checkResult(result);

    result_data.data = null
    result = stmsgApi.encode(handle, result_data);
    checkResult(result);
//    stmsgApi.dump(handle);
    encoded = result_data.data;
EM.log('encoded'+stmsgApi.dumpHex(encoded), '1.info', "espStart.js", true);

    // we build an ArrayBuffer, as the websocket interface also delivers an ArrayBuffer!
    encodedBytes = new ArrayBuffer(encoded);
    result_data.encryptedBytes = null
    result = stmsgApi.encrypt(handle, cryptkey, iv, encoded, result_data);
    checkResult(result);
    encryptedBytes = result_data.encryptedBytes;
//EM.log('encrypted'+stmsgApi.dumpHex(encryptedBytes), '1.info', "espStart.js", true);

    result_data.decryptedBytes = null
    var arr = Uint8Array.from(encryptedBytes);
    var encryptedBytesBuf = arr.buffer;

    result = stmsgApi.decrypt(handle, cryptkey, iv, encryptedBytesBuf, result_data);
    checkResult(result);
    decryptedBytes = result_data.decryptedBytes;
    var arr2 = Uint8Array.from(decryptedBytes);
    var decryptedBytesBuf = arr2.buffer;
EM.log('decrypt done'+stmsgApi.dumpHex(decryptedBytesBuf), '1.info', "espStart.js", true);







if (0) {

EM.log('decode1', '1.info', "espStart.js", true);
    result = stmsgApi.decode(handle);
    checkResult(result);
    stmsgApi.dump(handle);
EM.log('decode1 END', '1.info', "espStart.js", true);

    result_data.data = null
    result = stmsgApi.get_encoded(handle, result_data);
    checkResult(result);
    encoded = result_data.data;
EM.log('encoded2'+stmsgApi.dumpHex(encoded), '1.info', "espStart.js", true);

    result_data.value = null;
    result = stmsgApi.get_fieldValue(handle, "@randomNum", result_data);
    checkResult(result);
EM.log('>> FIELD: @randomNum: '+result_data.value, '1.info', "espStart.js", true);

    result_data.value = null
    result = stmsgApi.get_tableFieldValue(handle, "ssid", 0, result_data);
    checkResult(result);
EM.log('>> TableField: ssid: '+result_data.value, '1.info', "espStart.js", true);

EM.log('Decode', '1.info', "espStart.js", true);
    result = stmsgApi.decode(handle);
    checkResult(result);

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

    result = stmsgApi.add_fieldDefinition("aplist", "@randomNum","uint32_t",4);
    checkResult(result);

    result = stmsgApi.add_fieldDefinition("aplist", "@sequenceNum","uint32_t",4);
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

    result = stmsgApi.add_fieldDefinition("aplist", "@filler","uint16_t",2);
    checkResult(result);

    result = stmsgApi.add_fieldDefinition("aplist", "@crc","uint16_t",2);
    checkResult(result);

EM.log('encodeFieldDefinition', '1.info', "espStart.js", true);
    var data = new Object();
    data.data = null;
    result = stmsgApi.encode_fieldDefinition("aplist", data);
    checkResult(result);
EM.log('encoded definition'+stmsgApi.dumpHex(data.data), '1.info', "espStart.js", true);

EM.log('create_msgFromDefinition', '1.info', "espStart.js", true);
    data.data = null;
    result = stmsgApi.create_msgFromDefinition("aplist", data);
    checkResult(result);
    handle = data.handle
print("handle: ",handle);
EM.log('create from Definition done', '1.info', "espStart.js", true);
//    stmsgApi.dump(handle);

EM.log('set_encoded', '1.info', "espStart.js", true);
    result = stmsgApi.set_encoded(handle, encoded);
    checkResult(result);

EM.log('decode', '1.info', "espStart.js", true);
    result = stmsgApi.decode(handle);
    checkResult(result);
    stmsgApi.dump(handle);

EM.log('normalFieldNames', '1.info', "espStart.js", true);
    data.data = null;
    result = stmsgApi.get_definitionNormalFieldNames("aplist",data);
    checkResult(result);
    normalFieldNames=data.fieldNames;
print("norm: ",normalFieldNames.toString());

EM.log('tableFieldNames', '1.info', "espStart.js", true);
    data.data = null;
    result = stmsgApi.get_definitionTableFieldNames("aplist",data);
    checkResult(result);
    tableFieldNames=data.fieldNames;
print("table: ",tableFieldNames.toString());

EM.log('numTableRows', '1.info', "espStart.js", true);
    data.data = null;
    result = stmsgApi.get_definitionNumTableRows("aplist",data);
    checkResult(result);
    numTableRows=data.numTableRows;
print("numTableRows: ",numTableRows);

EM.log('numTableRowFields', '1.info', "espStart.js", true);
    data.data = null;
    result = stmsgApi.get_definitionNumTableRowFields("aplist",data);
    checkResult(result);
    numTableRowFields=data.numTableRowFields;
print("numTableRowFields: ",numTableRowFields);
}




    function confirm(str) {
      EM.log(str, 'info', "confirm", true);
    }


EM.log('END', '1.info', "EspStart.js", true);
