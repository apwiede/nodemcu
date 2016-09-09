/*====================================================
 * A structured Message Support implementation in Javascript named Esp
 *
 * Structmsg StructmsgInfo for Esp (StructmsgInfo.js)
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

/*====================================================
 * layout of a StructmsgInfo:
 *
 *  hdr (HeaderInfo)
 *    src
 *    dst
 *    totalLgth
 *    cmdKey
 *    cmdLgth
 *    hdrId (packed info of src+dst+totalLgth+cmdKey+cmdLgth
 *  msg (MsgInfo)
 *    fieldInfos (Array of FieldInfo)
 *      (FieldInfo)
 *        fieldStr (name)
 *        fieldType (type: uint8_t/int8_t/uint16_t/int16_t/......)
 *        fieldLgth
 *        value
 *        flags
 *    tableFieldInfos (Array of FieldInfo)
 *      (FieldInfo)
 *        fieldStr (name)
 *        fieldType (type: uint8_t/int8_t/uint16_t/int16_t/......)
 *        fieldLgth
 *        value
 *        flags (IS_SET)
 *    numFieldInfos
 *    maxFieldInfos
 *    numTableRows         // number of table rows
 *    numTableRowFields    // number of fields within a table row
 *    numRowFields         // for checking how many tableRowFields have been processed
 *  handle
 *  flags (ENCODED/DECODED/ENCRYPTED/DECRYPTED)
 *  encoded
 *  todecode
 *  encrypted
 *  sequenceNum  (is incrmented foreach sent message)
 *  handleHdrInfoPtr
 *
 *====================================================
 */

EM.addModule("Esp-StructmsgInfo", function(T, name) {

  /* ==================== StructmsgInfo constructor ======================= */

  function StructmsgInfo() {
    T.log('constructor called', 'info', 'StructmsgInfo', true);

    var StmsgInfo = this;

    var constructor = StmsgInfo.constructor;
    StructmsgInfo.superclass.constructor.apply(StmsgInfo, arguments);

    T.Defines.StructmsgInfoOid++;
    StmsgInfo.oid = T.Defines.StructmsgInfoOid;
    StmsgInfo.hdr = null;
    StmsgInfo.msg = null;
    StmsgInfo.handle = null;
    StmsgInfo.flags = 0;
    StmsgInfo.encoded = null;
    StmsgInfo.todecode = null;
    StmsgInfo.encrypted = null;
    StmsgInfo.sequenceNum = 0;
    StmsgInfo.handleHdrInfoPtr = null;

    T.log('constructor end', 'info', 'StructmsgInfo', true);
  }


  T.extend(StructmsgInfo, T.Defines, {
     my_name: "StructmsgInfo",
     type_name: "structmsg_info",
     flags: 0,

    /* ==================== toString ===================================== */

    toString: function () {
      var stmsgInfo = this;
      return stmsgInfo.mySelf()+"!"+stmsgInfo.oid+"!";
    },

    /* ==================== toDebugString ===================================== */
    toDebugString: function () {
      var stmsgInfo = this;
      var str = stmsgInfo.toString()+"\n";
      str += "  handle:      "+(stmsgInfo.handle == null ? "null" : stmsgInfo.handle)+"\n";
      str += "  flags:       "+stmsgInfo.flags+"\n";
      str += "  encoded:     "+(stmsgInfo.encoded == null ? "No" : "Yes")+"\n";
      str += "  todecode:    "+(stmsgInfo.todecod == null ? "No" : "Yes")+"\n";
      str += "  encrypted:   "+(stmsgInfo.encrypted == null ? "No" : "Yes")+"\n";
      str += "  sequenceNum: "+stmsgInfo.sequenceNum+"\n";
      str += "  hdr:         "+(stmsgInfo.hdr == null ? "null\n" : stmsgInfo.hdr.toDebugString());
      str += "  msg:         "+(stmsgInfo.msg == null ? "null\n" : stmsgInfo.msg.toDebugString());
      return str;
    },

    /* ==================== create ===================================== */
    create: function (numFields) {
      var structmsgInfo = this;
      structmsgInfo.hdr = new T.HeaderInfo();
      structmsgInfo.hdr.src = 0;
      structmsgInfo.hdr.dst = 0;
      structmsgInfo.hdr.cmdKey = 0;
      structmsgInfo.hdr.cmdLgth = structmsgInfo.STRUCT_MSG_CMD_HEADER_LENGTH;
      structmsgInfo.hdr.headerLgth = structmsgInfo.STRUCT_MSG_HEADER_LENGTH;
      structmsgInfo.hdr.totalLgth = structmsgInfo.STRUCT_MSG_TOTAL_HEADER_LENGTH;
      structmsgInfo.msg = new T.MsgInfo();
      structmsgInfo.msg.numFieldInfos = 0;
      structmsgInfo.msg.maxFieldInfos = numFields;
      structmsgInfo.msg.numTableRows = 0;
      structmsgInfo.msg.numTableRowFields = 0;
      structmsgInfo.msg.numRowFields = 0;
//      structmsg.msg.fieldInfos = newFieldInfos(numFieldInfos);
      structmsgInfo.msg.fieldInfos = new Array();
      structmsgInfo.msg.tableFieldInfos = new Array();
      structmsgInfo.flags = 0;
      structmsgInfo.sequenceNum = 0;
      structmsgInfo.encoded = null;
      structmsgInfo.todecode = null;
      structmsgInfo.encrypted = null;
      structmsgInfo.handleHdrInfoPtr = null;
      T.Defines.numHandles++;
      var s = "0000" + T.Defines.numHandles;
      s = s.substr(s.length-4);
      structmsgInfo.handle = structmsgInfo.HANDLE_PREFIX+'efff'+s;
//      result = addHandle(structmsg->handle, &structmsg->handleHdrInfoPtr);
//  setHandleField(structmsg->handle, STRUCT_MSG_FIELD_CMD_LGTH, structmsg->hdr.hdrInfo.hdrKeys.cmdLgth);
//  setHandleField(structmsg->handle, STRUCT_MSG_FIELD_TOTAL_LGTH, structmsg->hdr.hdrInfo.hdrKeys.totalLgth);
      structmsgInfo.hdr.fillHdrInfo();
      return structmsgInfo;
    },

  });

  T.StructmsgInfo = StructmsgInfo;

  T.log("module: "+name+" initialised!", "info", "StructmsgInfo.js");
}, "0.0.1", {});
