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
      defines.uint16_t_size = sizeof("uint16_t");

      T.log('constructor end', 'info', 'structmsgDefines', true);
  };

  /* ==================== sizeof ======================= */

  function sizeof(type) {
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

    sstructmsgFieldTypes2Str: {
      "1":  "uint8_t",
      "2":  "int8_t",
      "3":  "uint16_t",
      "4":  "int16_t",
      "5":  "uint32_t",
      "6":  "int32_t",
      "7":  "uint8_t*",
      "8":  "uint16_t*",
      "9":  "int8_t*",
      "10": "int16_t*",
      "11": "uint32_t*",
      "12": "int32_t*",
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

    // ============================= getFieldTypeId ========================

    getFieldTypeId: function(str) {
      var msg = "funny FieldTypeStr: ";
      var defines = this;

      return defines.getGenericTypeString(defines.structmsgFieldTypes2Id, msg, str);
    },

    // ============================= getFieldTypeStr ========================

    getFieldTypeStr: function(key) {
      var msg = "funny FieldTypeId: ";
      var defines = this;

      return defines.getGenericTypeString(defines.structmsgFieldTypes2str, msg, key);
    },

    // ============================= getIdFieldNameStr ========================

    getIdFieldNameStr: function(id, fieldName) {
      var entry;
      var nameEntry;
    
      fieldName = null;
      // first try to find special field name
      entry = structmsgSpecialFieldNames[0];
      while (entry.str != NULL) {
        if (entry.key == id) {
          fieldName = entry.str;
          return STRUCT_MSG_ERR_OK;
        }
        entry++;
      }
      // find field name
      var idx = 0;
    
      while (idx < fieldNameDefinitions.numDefinitions) {
        nameEntry = fieldNameDefinitions.definitions[idx];
        if (nameEntry.id == id) {
          fieldName = nameEntry.str;
          return STRUCT_MSG_ERR_OK;
        }
        nameEntry++;
        idx++;
      }
      return STRUCT_MSG_ERR_FIELD_NOT_FOUND;
    },

    // ============================= structmsg_getFieldNameId ========================

    structmsg_getFieldNameId: function(fieldName, id, incrRefCnt) {
      var fieldNameId = 0;
      var found = 0;
      var nameIdx = 0;
      var firstFreeEntryId;
      var definition;
      var entry;
      var newDefinition;
      var nameEntry;
      var firstFreeEntry;
    
      if (fieldName[0] == '@') {
        // find special field name
        entry = structmsgSpecialFieldNames[0];
        while (entry.str != NULL) {
          if (c_strcmp(entry.str, fieldName) == 0) {
            id = entry.key;
            return STRUCT_MSG_ERR_OK;
          }
          entry++;
        }
        return STRUCT_MSG_ERR_BAD_SPECIAL_FIELD;
      } else {
        if ((incrRefCnt == STRUCT_MSG_INCR) & (fieldNameDefinitions.numDefinitions >= fieldNameDefinitions.maxDefinitions)) {
          if (fieldNameDefinitions.maxDefinitions == 0) {
            fieldNameDefinitions.maxDefinitions = 4;
            fieldNameDefinitions.definitions = os_zalloc((fieldNameDefinitions.maxDefinitions * sizeof(name2id_t)));
            checkAllocOK(fieldNameDefinitions.definitions);
          } else {
            fieldNameDefinitions.maxDefinitions += 2;
            fieldNameDefinitions.definitions = os_realloc((fieldNameDefinitions.definitions), (fieldNameDefinitions.maxDefinitions * sizeof(name2id_t)));
            checkAllocOK(fieldNameDefinitions.definitions);
          }
        }
        firstFreeEntry = NULL;
        firstFreeEntryId = 0;
        if (fieldNameDefinitions.numDefinitions > 0) {
          // find field name
          nameIdx = 0;
          while (nameIdx < fieldNameDefinitions.numDefinitions) {
            nameEntry = fieldNameDefinitions.definitions[nameIdx];
            if ((nameEntry.str != null) & (c_strcmp(nameEntry.str, fieldName) == 0)) {
              if (incrRefCnt < 0) {
                if (nameEntry.refCnt > 0) {
                  nameEntry.refCnt--;
                }
                if (nameEntry.refCnt == 0) {
                  nameEntry.id = STRUCT_MSG_FREE_FIELD_ID;
                  os_free(nameEntry.str);
                  nameEntry.str = NULL;
                }
              } else {
                if (incrRefCnt > 0) {
                  nameEntry.refCnt++;
                } else {
                  // just get the entry, do not modify
                }
              }
              id = nameEntry.id;
              return STRUCT_MSG_ERR_OK;
            }
            if ((incrRefCnt == STRUCT_MSG_INCR) & (nameEntry.id == STRUCT_MSG_FREE_FIELD_ID) && (firstFreeEntry == NULL)) {
              firstFreeEntry = nameEntry;
              firstFreeEntry.id = nameIdx + 1;
            }
            nameIdx++;
          }
        }
        if (incrRefCnt < 0) {
          return STRUCT_MSG_ERR_OK; // just ignore silently
        } else {
          if (incrRefCnt == 0) {
            return STRUCT_MSG_ERR_FIELD_NOT_FOUND;
          } else {
            if (firstFreeEntry != NULL) {
              id = firstFreeEntry.id;
              firstFreeEntry.refCnt = 1;
              firstFreeEntry.str = os_malloc(c_strlen(fieldName) + 1);
              firstFreeEntry.str[c_strlen(fieldName)] = '\0';
              c_memcpy(firstFreeEntry.str, fieldName, c_strlen(fieldName));
            } else {
              newDefinition = fieldNameDefinitions.definitions[fieldNameDefinitions.numDefinitions];
              newDefinition.refCnt = 1;
              newDefinition.id = fieldNameDefinitions.numDefinitions + 1;
              newDefinition.str = os_malloc(c_strlen(fieldName) + 1);
              newDefinition.str[c_strlen(fieldName)] = '\0';
              c_memcpy(newDefinition.str, fieldName, c_strlen(fieldName));
              fieldNameDefinitions.numDefinitions++;
              id = newDefinition.id;
            }
          }
        }
      }
      return STRUCT_MSG_ERR_OK;
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

  T.log("module: "+name+" initialised!", "info", "structmsgDefines.js");
}, "0.0.1", {});
