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
    result = stmsgApi.encode(handle);


    function confirm(str) {
      EM.log(str, 'info', "confirm", true);
    }


EM.log('END', 'info', "End", true);
