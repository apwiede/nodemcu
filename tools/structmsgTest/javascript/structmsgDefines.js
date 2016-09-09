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
        T.log('constructor called', 'info', 'structmsgDefines', true);

        var defines = this;
        defines.name = defines.constructor.NAME;
        defines.init.apply(defines, arguments);

        T.log('constructor end', 'info', 'structmsgDefines', true);
    }

    Defines.structmsg = null;

    Defines.my_name = "Esp-StructmsgDefines";
    Defines.NAME = "structmsgDefines";

    Defines.prototype = {
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
        STRUCT_MSG_SPEC_FIELD_ID:               245,
        STRUCT_MSG_SPEC_FIELD_TABLE_ROWS:       244,
        STRUCT_MSG_SPEC_FIELD_TABLE_ROW_FIELDS: 243,
        STRUCT_MSG_SPEC_FIELD_LOW:              242,  // this must be the last entry!!

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

        STRUCT_MSG_FIELD_SRC:                   1,
        STRUCT_MSG_FIELD_DST:                   2,
        STRUCT_MSG_FIELD_TOTAL_LGTH:            3,
        STRUCT_MSG_FIELD_CMD_KEY:               4,
        STRUCT_MSG_FIELD_CMD_LGTH:              5,

        STRUCT_MSG_NO_INCR:                     0,
        STRUCT_MSG_INCR:                        1,
        STRUCT_MSG_DECR:                        -1,


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
            T.log('init called', 'info', 'base', true);
            T.log('init end', 'info', 'base', true);
            return this;
        },
  
        /* ==================== getGenericTypeString ============================== */
        getGenericTypeString: function(type_obj, msg, val) {
            var defines = this;
  if (typeof type_obj == "undefined") {
  T.log("getGenericTypeString!"+msg+"!"+val+"!", "error", "defines.js");
  }
            if (val == null) {
                return null;
            }
            if (typeof type_obj[val] == "undefined") {
                throw msg+val+"!";
            }
            return type_obj[val];
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
        getErrIdString: function(val) {
            var msg = "funny ErrId: ";
            var defines = this;
            return defines.getGenericTypeString(defines.structmsgErrId2Str, msg, val);
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
  
    };

    Defines.prototype.constructor = Defines;
    T.Defines = Defines;

    T.log("module: "+name+" initialised!", "info", "structmsgDefines.js");
}, "0.0.1", {});
