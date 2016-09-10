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
    //FIXME !!
    hdrInfo.hdrId = new ArrayBuffer(10);

    T.log('constructor end', 'info', 'HeaderInfo', true);
  }

  T.extend(HeaderInfo, T.structmsgEncDec, {
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
      str += "    src:          0x"+hdrInfo.src.toString(16)+"\n";
      str += "    dst:          0x"+hdrInfo.dst.toString(16)+"\n";
      str += "    totalLgth:    "+hdrInfo.totalLgth+"\n";
      str += "    cmdKey:       0x"+hdrInfo.cmdKey.toString(16)+"\n";
      str += "    cmdLgth:      "+hdrInfo.cmdLgth+"\n";
      str += "    hdrId (lgth): "+(hdrInfo.hdrId == null ? "null" : hdrInfo.hdrId.byteLength)+"\n";
      if (hdrInfo.hdrId != null) {
        str += "      values: "+hdrInfo.dumpHex(hdrInfo.hdrId)+"\n";
      }
      return str;
    },

    // ============================= fixHeaderInfo ========================
    
    fixHeaderInfo: function(fieldInfo, fieldType, fieldLgth, numTableRows) {
      var hdrInfo = this;
      switch (fieldType) {
        case hdrInfo.STRUCT_MSG_FIELD_UINT8_T:
        case hdrInfo.STRUCT_MSG_FIELD_INT8_T:
          hdrInfo.totalLgth += 1 * numTableRows;
          hdrInfo.cmdLgth += 1 * numTableRows;
          fieldLgth = 1;
          break;
        case hdrInfo.STRUCT_MSG_FIELD_UINT16_T:
        case hdrInfo.STRUCT_MSG_FIELD_INT16_T:
          hdrInfo.totalLgth += 2 * numTableRows;
          hdrInfo.cmdLgth += 2 * numTableRows;
          fieldLgth = 2;
          break;
        case hdrInfo.STRUCT_MSG_FIELD_UINT32_T:
        case hdrInfo.STRUCT_MSG_FIELD_INT32_T:
          hdrInfo.totalLgth += 4 * numTableRows;
          hdrInfo.cmdLgth += 4 * numTableRows;
          fieldLgth = 4;
          break;
        case hdrInfo.STRUCT_MSG_FIELD_UINT8_VECTOR:
          hdrInfo.totalLgth += fieldLgth * numTableRows;
          hdrInfo.cmdLgth += fieldLgth * numTableRows;
          break;
        case hdrInfo.STRUCT_MSG_FIELD_INT8_VECTOR:
          hdrInfo.totalLgth += fieldLgth * numTableRows;
          hdrInfo.cmdLgth += fieldLgth * numTableRows;
          fieldInfo.value.ubyteVector[fieldLgth] = '\0';
          break;
        case hdrInfo.STRUCT_MSG_FIELD_UINT16_VECTOR:
          hdrInfo.totalLgth += fieldLgth * numTableRows;
          break;
        case hdrInfo.STRUCT_MSG_FIELD_INT16_VECTOR:
          hdrInfo.totalLgth += fieldLgth * numTableRows;
          hdrInfo.cmdLgth += fieldLgth * numTableRows;
          break;
        case hdrInfo.STRUCT_MSG_FIELD_UINT32_VECTOR:
          hdrInfo.totalLgth += fieldLgth * numTableRows;
          hdrInfo.cmdLgth += fieldLgth * numTableRows;
          break;
        case hdrInfo.STRUCT_MSG_FIELD_INT32_VECTOR:
          hdrInfo.totalLgth += fieldLgth * numTableRows;
          hdrInfo.cmdLgth += fieldLgth * numTableRows;
          break;
      }
      fieldInfo.fieldLgth = fieldLgth;
      return hdrInfo.STRUCT_MSG_ERR_OK;
    },
    
    /* ==================== fillHdrInfo ===================================== */

    fillHdrInfo: function () {
      var hdrInfo = this;
      var offset = 0;

      offset = hdrInfo.uint16Encode(hdrInfo.hdrId, offset, hdrInfo.src);
//  checkEncodeOffset(offset);
      offset = hdrInfo.uint16Encode(hdrInfo.hdrId, offset, hdrInfo.dst);
//  checkEncodeOffset(offset);
      offset = hdrInfo.uint16Encode(hdrInfo.hdrId, offset, hdrInfo.totalLgth);
//  checkEncodeOffset(offset);
      offset = hdrInfo.uint16Encode(hdrInfo.hdrId, offset, hdrInfo.cmdKey);
//  checkEncodeOffset(offset);
      offset = hdrInfo.uint16Encode(hdrInfo.hdrId, offset, hdrInfo.cmdLgth);
//  checkEncodeOffset(offset);
print("hdr: ",hdrInfo.toDebugString());
      return hdrInfo.STRUCT_MSG_ERR_OK;
    },

  });

  T.HeaderInfo = HeaderInfo;

  T.log("module: "+name+" initialised!", "info", "HeaderInfo.js");
}, "0.0.1", {});
