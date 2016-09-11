/*====================================================
 * A structured Message Support implementation in Javascript named Esp
 *
 * Structmsg defines for Esp (structmsgDefines.js)
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

/* this is the defines module with a lot of defines to be inherited by the token module */

// ============================= structmsg structure layouts ========================
// 
// structmsg(prefix)
// structmsg(numFieldNameIds)
// structmsg(numStructmsgDefinitions)
// structmsg(numHandles)
// 
// structmsg(specialFieldIds)
// structmsg(specialFieldNames)
// structmsg(structmsgErrId2Str)
// structmsg(fieldTypeDefines)
// structmsg(fieldTypeIds)
// 
// structmsg(fieldNameDefinitions)
//    numDefinitions
//    definitions [list 
//      refCnt
//      id
//      fieldName
//    ]
// 
// structmsg(structmsgDefinitions)
//    numDefinitions
//    definitions [list 
//      name
//      numFields
//      maxFields
//      encoded
//      encrypted
//      fieldInfos [list
//        fieldId
//        fieldType
//        fieldLgth
//      ]
//    ]
// 
// structmsg(<handle>)
//  hdr
//    headerLgth
//    hdrInfo
//      hdrKeys
//        src
//        dst
//        cmdKey
//        cmdLgth
//        totalLgth
//  msg
//    numFieldInfos
//    maxFieldInfos
//    numTableRows
//    numTableRowFields
//    numRowFields
//    fieldInfos [list
//    ]
//    tableFieldInfos [list
//    ]
//    flags [list          ; // HAS_CRC, ENCODED, DECODED, ENCRYPTED, DECRYPTED
//    ]
//  sequenceNum
//  encoded
//  todecode
//  encrypted
// 
// structmsg(hdrId2Handles) [list
//  hdrId
//  handle
// ] 
//

EM.addModule("Esp-structmsgDefines", function(T, name) {

  /* ==================== Defines constructor ======================= */

  function Defines() {
      T.log('constructor called', '2.info', 'structmsgDefines', true);

      var defines = this;
      defines.name = defines.constructor.NAME;
      defines.init.apply(defines, arguments);
//      defines.uint16_t_size = sizeof("uint16_t");

      T.log('constructor end', '2.info', 'structmsgDefines', true);
  };


  Defines.FieldInfoDefinitionOid = 0;
  Defines.FieldInfoOid = 0;
  Defines.FieldNameDefinitionsOid = 0;
  Defines.HeaderInfoOid = 0;
  Defines.MsgInfoOid = 0;
  Defines.StructmsgDefinitionOid = 0;
  Defines.StructmsgDefinitionsOid = 0;
  Defines.StructmsgInfoOid = 0;
  Defines.StructmsgInfosOid = 0;

  Defines.numHandles = 0;

  Defines.my_name = "Esp-StructmsgDefines";
  Defines.NAME = "structmsgDefines";

  Defines.prototype = {
  /* ==================== sizeof ======================= */

  sizeof: function(type) {
    switch (type) {
    case 'int8_t':
    case 'uint8_t':
      return 1;
      break;
    case 'int16_t':
    case 'uint16_t':
      return 2;
      break
    case 'int32_t':
    case 'uint32_t':
      return 4;
      break
    }
    T.log("bad type in sizeof: "+type, 'error', 'structmsgDefines', true);
  },

    STRUCT_MSG_FIELD_UINT8_T:              1,
    STRUCT_MSG_FIELD_INT8_T:               2,
    STRUCT_MSG_FIELD_UINT16_T:             3,
    STRUCT_MSG_FIELD_INT16_T:              4,
    STRUCT_MSG_FIELD_UINT32_T:             5,
    STRUCT_MSG_FIELD_INT32_T:              6,
    STRUCT_MSG_FIELD_UINT8_VECTOR:         7,
    STRUCT_MSG_FIELD_INT8_VECTOR:          8,
    STRUCT_MSG_FIELD_UINT16_VECTOR:        9,
    STRUCT_MSG_FIELD_INT16_VECTOR:         10,
    STRUCT_MSG_FIELD_UINT32_VECTOR:        11,
    STRUCT_MSG_FIELD_INT32_VECTOR:         12,

    structmsgFieldTypes2Id: {
      "uint8_t":   1,
      "int8_t":    2,
      "uint16_t":  3,
      "int16_t":   4,
      "uint32_t":  5,
      "int32_t":   6,
      "uint8_t*":  7,
      "int8_t*":   8,
      "uint16_t*": 9,
      "int16_t*":  10,
      "uint32_t*": 11,
      "int32_t*":  12,
      null:        -1,
    },

    structmsgFieldTypes2Str: {
      1:  "uint8_t",
      2:  "int8_t",
      3:  "uint16_t",
      4:  "int16_t",
      5:  "uint32_t",
      6:  "int32_t",
      7:  "uint8_t*",
      8:  "uint16_t*",
      9:  "int8_t*",
      10: "int16_t*",
      11: "uint32_t*",
      12: "int32_t*",
    },

    STRUCT_MSG_SPEC_FIELD_SRC:              255,
    STRUCT_MSG_SPEC_FIELD_DST:              254,
    STRUCT_MSG_SPEC_FIELD_TARGET_CMD:       253,
    STRUCT_MSG_SPEC_FIELD_TOTAL_LGTH:       252,
    STRUCT_MSG_SPEC_FIELD_CMD_KEY:          251,
    STRUCT_MSG_SPEC_FIELD_CMD_LGTH:         250,
    STRUCT_MSG_SPEC_FIELD_RANDOM_NUM:       249,
    STRUCT_MSG_SPEC_FIELD_SEQUENCE_NUM:     248,
    STRUCT_MSG_SPEC_FIELD_FILLER:           247,
    STRUCT_MSG_SPEC_FIELD_CRC:              246,
    STRUCT_MSG_SPEC_FIELD_ID:               245,
    STRUCT_MSG_SPEC_FIELD_TABLE_ROWS:       244,
    STRUCT_MSG_SPEC_FIELD_TABLE_ROW_FIELDS: 243,
    STRUCT_MSG_SPEC_FIELD_LOW:              242,  // this must be the last entry!!

    structmsgSpecialFieldNames2Id: {
      "@src":            255,
      "@dst":            254,
      "@targetCmd":      253,
      "@totalLgth":      252,
      "@cmdKey":         251,
      "@cmdLgth":        250,
      "@randomNum":      249,
      "@sequenceNum":    248,
      "@filler":         247,
      "@crc":            246,
      "@id":             245,
      "@tablerows":      244,
      "@tablerowfields": 243,
    },

    structmsgSpecialFieldNamesId2Str: {
      255: "@src",
      254: "@dst",
      253: "@targetCmd",
      252: "@totalLgth",
      251: "@cmdKey",
      250: "@cmdLgth",
      249: "@randomNum",
      248: "@sequenceNum",
      247: "@filler",
      246: "@crc",
      245: "@id",
      244: "@tablerows",
      243: "@tablerowfields",
    },

    STRUCT_MSG_ERR_OK:                      0,
    STRUCT_MSG_ERR_VALUE_NOT_SET:           255,
    STRUCT_MSG_ERR_VALUE_OUT_OF_RANGE:      254,
    STRUCT_MSG_ERR_BAD_VALUE:               253,
    STRUCT_MSG_ERR_BAD_FIELD_TYPE:          252,
    STRUCT_MSG_ERR_FIELD_NOT_FOUND:         251,
    STRUCT_MSG_ERR_VALUE_TOO_BIG:           250,
    STRUCT_MSG_ERR_BAD_SPECIAL_FIELD:       249,
    STRUCT_MSG_ERR_BAD_HANDLE:              248,
    STRUCT_MSG_ERR_OUT_OF_MEMORY:           247,
    STRUCT_MSG_ERR_HANDLE_NOT_FOUND:        246,
    STRUCT_MSG_ERR_NOT_ENCODED:             245,
    STRUCT_MSG_ERR_ENCODE_ERROR:            244,
    STRUCT_MSG_ERR_DECODE_ERROR:            243,
    STRUCT_MSG_ERR_BAD_CRC_VALUE:           242,
    STRUCT_MSG_ERR_CRYPTO_INIT_FAILED:      241,
    STRUCT_MSG_ERR_CRYPTO_OP_FAILED:        240,
    STRUCT_MSG_ERR_CRYPTO_BAD_MECHANISM:    239,
    STRUCT_MSG_ERR_NOT_ENCRYPTED:           238,
    STRUCT_MSG_ERR_DEFINITION_NOT_FOUND:    237,
    STRUCT_MSG_ERR_DEFINITION_TOO_MANY_FIELDS: 236,
    STRUCT_MSG_ERR_BAD_TABLE_ROW:           235,
    STRUCT_MSG_ERR_TOO_MANY_FIELDS:         234,
    STRUCT_MSG_ERR_BAD_DEFINTION_CMD_KEY:   233,
    STRUCT_MSG_ERR_NO_SLOT_FOUND:           232,
    STRUCT_MSG_ERR_DUPLICATE_FIELD:         231,

    structmsgErrId2Str: {
      0:   "ERR_OK",
      255: "ERR_VALUE_NOT_SET",
      254: "ERR_VALUE_OUT_OF_RANGE",
      253: "ERR_BAD_VALUE",
      252: "ERR_BAD_FIELD_TYPE",
      251: "ERR_FIELD_NOT_FOUND",
      250: "ERR_VALUE_TOO_BIG",
      249: "ERR_BAD_SPECIAL_FIELD",
      248: "ERR_BAD_HANDLE",
      247: "ERR_OUT_OF_MEMORY",
      246: "ERR_HANDLE_NOT_FOUND",
      245: "ERR_NOT_ENCODED",
      244: "ERR_ENCODE_ERROR",
      243: "ERR_DECODE_ERROR",
      242: "ERR_BAD_CRC_VALUE",
      241: "ERR_CRYPTO_INIT_FAILED",
      240: "ERR_CRYPTO_OP_FAILED",
      239: "ERR_CRYPTO_BAD_MECHANISM",
      238: "ERR_NOT_ENCRYPTED",
      237: "ERR_DEFINITION_NOT_FOUND",
      236: "ERR_DEFINITION_TOO_MANY_FIELDS",
      235: "ERR_BAD_TABLE_ROW",
      234: "ERR_TOO_MANY_FIELDS",
      233: "ERR_BAD_DEFINTION_CMD_KEY",
      232: "ERR_NO_SLOT_FOUND",
      231: "ERR_DUPLICATE_FIELD",
    },

    STRUCT_MSG_FIELD_SRC:                   1,
    STRUCT_MSG_FIELD_DST:                   2,
    STRUCT_MSG_FIELD_TOTAL_LGTH:            3,
    STRUCT_MSG_FIELD_CMD_KEY:               4,
    STRUCT_MSG_FIELD_CMD_LGTH:              5,

    uint16_t_size:                          2,
    // header length: uint16_t src + uint16_t dst + uint16_t totalLgth
// FIXME!!
//        STRUCT_MSG_HEADER_LENGTH: (uint16_t_size * 3),
    STRUCT_MSG_HEADER_LENGTH: (2 * 3),
    // cmd header length uint16_t cmdKey + uint16_t cmdLgth
//        STRUCT_MSG_CMD_HEADER_LENGTH: (uint16_t_size * 2),
    STRUCT_MSG_CMD_HEADER_LENGTH: (2 * 2),
//        STRUCT_MSG_TOTAL_HEADER_LENGTH: (Defines.prototype.STRUCT_MSG_HEADER_LENGTH + Defines.prototype.STRUCT_MSG_CMD_HEADER_LENGTH),
    STRUCT_MSG_TOTAL_HEADER_LENGTH: (5 * 2),
    STRUCT_MSG_NUM_HEADER_FIELDS: 3,
    STRUCT_MSG_NUM_CMD_HEADER_FIELDS: 2,
    STRUCT_MSG_DEFINITION_CMD_KEY: 0xFFFF,
    STRUCT_MSG_FREE_FIELD_ID: 0xFF,

    STRUCT_MSG_NO_INCR:                     0,
    STRUCT_MSG_INCR:                        1,
    STRUCT_MSG_DECR:                        -1,

    HANDLE_PREFIX:                          "stmsg_",


    /* =========================== init ================================== */

    /**
     * Init lifecycle method, invoked during construction.
     * Fires the init event prior to setting up attributes and 
     * invoking initializers for the class hierarchy.
     *
     * @method init
     * @chainable
     * @param {Object} config Object with configuration property name/value pairs
     * @return {Defines} A reference to this object
     */
    init: function(config) {
      T.log('init called', '2.info', 'base', true);
      T.log('init end', '2.info', 'base', true);
      return this;
    },

    /* ==================== getGenericTypeString ============================== */
    getGenericTypeString: function(type_obj, msg, val, result) {
      var defines = this;
if (typeof type_obj == "undefined") {
T.log("getGenericTypeString!"+msg+"!"+val+"!", "error", "defines.js");
}
      if (val == null) {
        return null;
      }
      if (typeof type_obj[val] == "undefined") {
        return defines.STRUCT_MSG_ERR_FIELD_VALUE_NOT_FOUND;
      }
      result.value = type_obj[val];
      return defines.STRUCT_MSG_ERR_OK;
    },

    /* ==================== getGenericTypeVal ============================== */
    getGenericTypeVal: function(type_obj, val) {
      var defines = this;
if (typeof type_obj == "undefined") {
T.log("getGenericTypeVal!"+val+"!", "error", "defines.js");
}
      if (val == null) {
        return null;
      }
      if (typeof type_obj[base.escape_key(val)] == "undefined") {
        return null;
      }
      return type_obj[base.escape_key(val)];
    },

    /* ==================== getErrIdString ============================== */
    getErrIdString: function(val, result) {
      var msg = "funny ErrId: ";
      var defines = this;
      var res;

      return defines.getGenericTypeString(defines.structmsgErrId2Str, msg, val, result);
    },

    // ============================= getFieldTypeId ========================

    getFieldTypeId: function(str, result) {
      var msg = "funny FieldTypeStr: ";
      var defines = this;

      return defines.getGenericTypeString(defines.structmsgFieldTypes2Id, msg, str, result);
    },

    // ============================= getFieldTypeStr ========================

    getFieldTypeStr: function(key, resultData) {
      var msg = "funny FieldTypeId: ";
      var defines = this;

      if (typeof defines.structmsgFieldTypes2Str[key] != 'undefined') {
        resultData.fieldType = defines.structmsgFieldTypes2Str[key];
        return defines.STRUCT_MSG_ERR_OK;
      }
      return defines.STRUCT_MSG_ERR_BAD_FIELD_TYPE;
//      return defines.getGenericTypeString(defines.structmsgFieldTypes2str, msg, key, result);
    },

    // ============================= getIdFieldNameStr ========================

    getIdFieldNameStr: function(id, resultData) {
      var defines = this;
      var entry;
      var nameEntry;
    
      fieldName = null;
      // first try to find special field name
      if (typeof defines.structmsgSpecialFieldNamesId2Str[id] != 'undefined') {
        resultData.fieldName = defines.structmsgSpecialFieldNamesId2Str[id];
        return defines.STRUCT_MSG_ERR_OK;
      }
      // find field name
      var idx = 0;
    
      while (idx < defines.fieldNameDefinitions.numDefinitions) {
        nameEntry = defines.fieldNameDefinitions.definitions[idx];
        if (nameEntry.id == id) {
          resultData.fieldName = nameEntry.str;
          return defines.STRUCT_MSG_ERR_OK;
        }
        nameEntry++;
        idx++;
      }
      return defines.STRUCT_MSG_ERR_FIELD_NOT_FOUND;
    },




    /* ==================== escape_key ============================== */
    escape_key: function(key) {
      if (key == null) {
        return key;
      }
      switch (key.toString()) {
      case "constructor":
      case "destructor":
      case "join":
      case "function":
        key = "_"+key;
        break;
      }
      return key;
    },

    /* ==================== unescape_key ============================== */
    unescape_key: function(key) {
      if (key == null) {
        return key;
      }
      switch (key.toString()) {
      case "_constructor":
      case "_destructor":
      case "_join":
      case "_function":
        key = key.substring(1);
        break;
      }
      return key;
    },

    /* ==================== panic ============================== */
    panic: function(cond, msg) {
      if (cond) {
        throw msg;
      }
    },


    /* ==================== showProps ============================== */
    showProps: function(val,obj) {
      EM.log("props!", "info", obj.my_name);
      for (var z in val) {
        EM.log("Z!"+z+"!"+val[z]+"!", "info", obj.my_name);
      }
      EM.log("props END!", "info", obj.my_name);
    },

    /* ==================== mySelf ===================================== */
    mySelf: function () {
      return this.my_name+"!";
    },

    /* ==================== dumpHex ===================================== */
    dumpHex: function(data) {
      var dv = new DataView(data);
      var hex = ''
      var idx;

      idx = 0;
      while (idx < data.byteLength) {
        hex += ' 0x'+dv.getUint8(idx).toString(16);
        idx++;
      }
      return hex;
    },

  };

  Defines.prototype.constructor = Defines;
  T.Defines = Defines;

  T.log("module: "+name+" initialised!", "2.info", "structmsgDefines.js");
}, "0.0.1", {});
