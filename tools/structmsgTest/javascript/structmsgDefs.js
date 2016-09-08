/*====================================================
 * A structured Message Support implementation in Javascript named Esp
 *
 * Structmsg Defs for Esp (structmsgDefs.js)
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

EM.addModule("Esp-structmsgDefs", function(T, name) {

  function Defs() {
    T.log('constructor called', 'info', 'structmsgDefs', true);

    var Defs = this;
    Defs.name = Defs.constructor.NAME;
    Defs.init.apply(Defs, arguments);

    T.log('constructor end', 'info', 'structmsgDefs', true);
  }

  Defs.structmsg = null;

  Defs.my_name = "Esp-StructmsgDefs";
  Defs.NAME = "structmsgDefs";

  Defs.prototype = {

    // ============================= checkOKOrErr ========================
    
    checkOKOrErr: function(result, where, fieldName) {
      if  (result == STRUCT_MSG_ERR_OK)  {
        return "OK"
      }
      if (typeof where === 'undefined') {
        where = "";
      }
      if (typeof fieldName === 'undefined') {
        fieldName = "";
      }
//print([format  "error result: %d"  result])
      var errStr = "ERROR";
      switch (esp.getErrIdString(result)) {
        case STRUCT_MSG_ERR_VALUE_NOT_SET:
          return errStr+where+': value for field:'+fieldName+' not set';
          break;
        case STRUCT_MSG_ERR_VALUE_OUT_OF_RANGE:
          return errStr+where+': field: '+fieldName+' value out of range';
          break;
        case STRUCT_MSG_ERR_BAD_VALUE:
          return errStr+where+': field: '+fieldName+' bad value';
          break;
        case STRUCT_MSG_ERR_BAD_FIELD_TYPE:
          return errStr+where+': field: '+fieldName+' bad field type';
          break;
        case STRUCT_MSG_ERR_FIELD_NOT_FOUND:
          return errStr+where+': field: '+fieldName+' not found';
          break;
        case STRUCT_MSG_ERR_BAD_HANDLE:
          return errStr+where+': bad handle';
          break;
        case STRUCT_MSG_ERR_OUT_OF_MEMORY:
          return errStr+' out of memory';
          break;
        case STRUCT_MSG_ERR_HANDLE_NOT_FOUND:
          return errStr+': handle not found';
          break;
        case STRUCT_MSG_ERR_NOT_ENCODED:
          return errStr+': not encoded';
          break;
        case STRUCT_MSG_ERR_DECODE_ERROR:
          return errStr+': decode error';
          break;
        case STRUCT_MSG_ERR_BAD_CRC_VALUE:
          return errStr+': bad crc val';
          break;
        case STRUCT_MSG_ERR_CRYPTO_INIT_FAILED:
          return errStr+': crypto init failed';
          break;
        case STRUCT_MSG_ERR_CRYPTO_OP_FAILED:
          return errStr+': crypto op failed';
          break;
        case STRUCT_MSG_ERR_CRYPTO_BAD_MECHANISM:
          return errStr+': crypto bad mechanism';
          break;
        case STRUCT_MSG_ERR_NOT_ENCRYPTED:
          return errStr+': not encrypted';
          break;
        case STRUCT_MSG_ERR_DEFINITION_NOT_FOUND:
          return errStr+': definiton not found';
          break;
        case STRUCT_MSG_ERR_BAD_SPECIAL_FIELD:
          return errStr+': bad special field';
          break;
        case STRUCT_MSG_ERR_DEFINITION_TOO_MANY_FIELDS:
          return errStr+': definition too many fields';
          break;
        case STRUCT_MSG_ERR_BAD_TABLE_ROW:
          return errStr+': bad table row';
          break;
        case STRUCT_MSG_ERR_TOO_MANY_FIELDS:
          return errStr+': too many fields';
          break;
        case STRUCT_MSG_ERR_BAD_DEFINTION_CMD_KEY:
          return errStr+': bad definition cmd key';
          break;
        case STRUCT_MSG_ERR_NO_SLOT_FOUND:
          return errStr+': no slot found';
          break;
        case STRUCT_MSG_ERR_DUPLICATE_FIELD:
          return errStr+': duplicate field';
          break;
        default:
          return errStr+': funny result error code';
          break;
      }
    },
    
    // ============================= encdec_msgDefinition ========================
    
    encdec_msgDefinition: function(handle, key, enc, iv, buf, lgth) {
      var result  = encdecDefinition(handle, key, enc, iv, buf, lgth);
      return result;
    },
    
    // ============================= create ========================
    
    create: function(numFields) {
      var result = cmd.create(numFields, handle);
      if (result == STRUCT_MSG_ERR_OK)  {
        return handle;
      }
      return checkOKOrErr(result, "create", "");
    },
    
    // ============================= delete ========================
    
    delete: function(handle) {
      var result = cmd.delete(handle);
      return checkOKOrErr(result, "delete", "");
    },
    
    // ============================= encode ========================
    
    encode: function(handle)  {
      var result = cmd.encode(handle);
      return checkOKOrErr(result, "encode", "");
    },
    
    // ============================= get_encoded ========================
    
    get_encoded: function(handle) {
      var result = cmd.get_encoded(handle, encoded, lgth);
      if (result == STRUCT_MSG_ERR_OK) {
        return encoded;
      }
      return checkOKOrErr(result, "get_encoded", "");
    },
    
    // ============================= decode ========================
    
    decode: function(handle, data) {
      var result = cmd.decode(handle, data);
      return checkOKOrErr(result, "decode", "");
    },
    
    // ============================= dump ========================
    
    dump: function(handle) {
      var result = cmd.dump(handle);
      return checkOKOrErr(result, "dump", "");
    },
    
    // ============================= encrypt ========================
    
    encrypt: function(handle, key, iv) {
      if (typeof iv === 'undefined') {
        iv = "";
      }
      var result  = cmd.encrypt(handle, null, 0, key, iv, buf, lgth);
      if (result == STRUCT_MSG_ERR_OK) {
        return buf;
      }
      return checkOKOrErr(result, "encrypt", "");
    },
    
    // ============================= decrypt ========================
    
    decrypt: function(handle, key, iv, crypted) {
      var result = cmd.decrypt(handle, null, 0, key, iv, crypted, buf, lgth);
      if (result == STRUCT_MSG_ERR_OK) {
        return buf;
      }
      return checkOKOrErr(result, "decrypt", "");
    },
    
    // ============================= add_field ========================
    
    add_field: function(handle, fieldStr, fieldType, fieldLgth) {
      var result = cmd.add_field(handle, fieldStr, fieldType, fieldLgth);
      return checkOKOrErr(result, "add_field", "");
    },
    
    // ============================= set_fillerAndCrc ========================
    
    set_fillerAndCrc: function(handle) {
      var result = structmsg_setFillerAndCrc(handle);
      return checkOKOrErr(result, "set_fillerAndCrc", "");
    },
    
    // ============================= set_fieldValue ========================
    
    set_fieldValue: function(handle, fieldName, value) {
      var result  = cmd.set_fieldValue(handle, fieldName, value);
      return checkOKOrErr(result,"set_fieldValue", fieldName);
    },
    
    // ============================= set_tableFieldValue ========================
    
    set_tableFieldValue: function(handle, fieldName, row, value) {
      var result = cmd.set_tableFieldValue(handle, fieldName, row, value);
      return checkOKOrErr(result, "set_tableFieldValue", fieldName);
    },
    
    // ============================= get_fieldValue ========================
    
    get_fieldValue: function(handle, fieldName, valueVar) {
      var result = cmd.get_fieldValue(handle, fieldName, valueC);
      return checkOKOrErr(result, "get_fieldValue", fieldName);
    },
    
    // ============================= get_tableFieldValue ========================
    
    get_tableFieldValue: function(handle, fieldName, row, valueVar) {
      var result = cmd.get_tableFieldValue(handle, fieldName, row, value);
      return checkOKOrErr(result, "get_tableFieldValue", fieldName);
    },
    
    // ============================= set_crypted ========================
    
    set_crypted: function(handle, crypted) {
      var result = cmd.set_crypted(handle, crypted);
      return checkOKOrErr(result, "set_crypted", "");
    },
    
    // ============================= decrypt_getHandle ========================
    
    decrypt_getHandle: function(encrypted, key, iv, handleVar) {
      var result = cmd.decrypt_getHandle(encrypted, key, iv, handle);
      return checkOKOrErr(result, "decrypt_getHandle", "");
    },
    
    // ============================= create_definition ========================
    
    create_definition: function(name, numFields) {
      var result = structmsg.def.createDefinition(name, numFields);
      return checkOKOrErr(result, "create_Definition", "");
    },
    
    // ============================= add_fieldDefinition ========================
    
    add_fieldDefinition: function(name, fieldName, fieldType, fieldLgth) {
      var result = structmsg.def.addFieldDefinition(name, fieldName, fieldType, fieldLgth);
      return checkOKOrErr(result, "add_fieldDefinition", "");
    },
    
    // ============================= dump_fieldDefinition ========================
    
    dump_fieldDefinition: function(name) {
      var result = structmsg.def.dumpFieldDefinition(name);
      return checkOKOrErr(result, "dump_fieldDefinition", "");
    },
    
    // ============================= encode_fieldDefinition =================
    
    encode_fieldDefinition: function(name) {
      var result = structmsg.def.encodeFieldDefinition(name, data, lgth);
      if  (result == STRUCT_MSG_ERR_OK) {
        return data;
      }
      return checkOKOrErr(result, "encode_fieldDefinition","");
    },
    
    // ============================= decode_fieldDefinition =================
    
    decode_fieldDefinition: function(name, encoded) {
      var result = structmsg.def.decodeFieldDefinition(name, encoded);
      return checkOKOrErr(result, "decode_fieldDefinition", "");
    },
    
    // ============================= set_crypted_definition ========================
    
    set_crypted_definition: function(name, crypted) {
      var result = structmsg.def.setCryptedDefinition(name, crypted);
      return checkOKOrErr(result, "set_crypted_definition", "");
    },
    
    // ============================= decrypt_getDefinitionName ========================
    
    decrypt_getDefinitionName: function(encrypted, key, iv) {
      if (typeof iv === 'undefined') {
        iv = "";
      }
      var result = structmsg.def.decryptGetDefinitionName(encrypted, key, iv, name);
      if (result == STRUCT_MSG_ERR_OK) {
        return name;
      }
      return checkOKOrErr(result, "decrypt_getDefinitionName", "");
    },
    
    // ============================= encrypt_definition ========================
    
    encrypt_definition: function(name, key, iv) {
      if (typeof iv === 'undefined') {
        iv = "";
      }
      var result = structmsg.def.encryptDefinition(name, key, iv, buf, lgth);
      if  (result == STRUCT_MSG_ERR_OK) {
        return buf;
      }
      return checkOKOrErr(result, "encrypt_definition", "");
    },
    
    // ============================= decrypt_definition ========================
    
    decrypt_definition: function(name, key, iv, encrypted) {
      if (typeof iv === 'undefined') {
        iv = "";
      }
      if (typeof encrypted === 'undefined') {
        encrypted = "";
      }
      var result = structmsg.def.decryptDefinition(name, key, iv, encrypted, buf, lgth);
      if  (result == STRUCT_MSG_ERR_OK) {
        return buf
      }
      return checkOKOrErr(result, "decrypt_definition", "");
    },
    
    // ============================= delete_fieldDefinition ========================
    
    delete_fieldDefinition: function(name) {
      var result = structmsg.def.deleteFieldDefinition(name);
      return checkOKOrErr(result, "delete_fieldDefinition", "");
    },
    
    // ============================= delete_fieldDefinitions ========================
    
    delete_fieldDefinitions: function() {
      var result = structmsg.def.deleteFieldDefinitions();
      return checkOKOrErr(result, "delete_fieldDefinitions", "");
    },
    
    // ============================= create_msgFromDefinition ========================
    
    create_msgFromDefinition: function(name, handleVar)  {
      var result = structmsg.def.createMsgFromDefinition(name, handle);
      return checkOKOrErr(result, "create_msgFromDefinition", "");
    },
  
    // ============================= get_definitionNormalFieldNames ========================
    
    get_definitionNormalFieldNames: function(name, normalFieldNamesVar)  {
      var result = structmsg.def.getDefinitionNormalFieldNames(name, normalFieldNames);
      return checkOKOrErr(result, "get_definitionNormalFieldNames", "");
    },
  
    // ============================= get_definitionTableFieldNames ========================
    
    get_definitionTableFieldNames: function(name, tableFieldNamesVar)  {
      var result = structmsg.def.getDefinitionTableFieldNames(name, tableFieldNames);
      return checkOKOrErr(result, "get_definitionTableFieldNames", "");
    },
  
    // ============================= get_definitionNumTableRows ========================
    
    get_definitionNumTableRows: function(name, numTableRowsVar)  {
      var result = structmsg.def.getDefinitionNumTableRows(name, numTableRows);
      return checkOKOrErr(result, "get_definitionNumTableRows", "");
    },
  
    // ============================= get_definitionNumTableRowFields ========================
    
    get_definitionNumTableRowFields: function(name, numTableRowFieldsVar)  {
      var result = structmsg.def.getDefinitionNumTableRowFields(name, numTableRowFields);
      return checkOKOrErr(result, "get_definitionNumTableRowFields", "");
    },
  
    // ============================= get_definitionFieldInfo ========================
    
    get_definitionFieldInfo: function(name, fieldName, fieldInfoVar)  {
      var result = structmsg.def.getDefinitionFieldInfo(name, fieldName, fieldInfo);
      return checkOKOrErr(result, "get_definitionFieldInfo", "");
    },
  
    // ============================= get_definitionTableFieldInfo ========================
    
    get_definitionTableFieldInfo: function(name, fieldName, row, fieldInfoVar)  {
      var result = structmsg.def.getDefinitionTableFieldInfo(name, fieldName, row, fieldInfo);
      return checkOKOrErr(result, "get_definitionTableFieldInfo", "");
    },

    /* ==================== showProps ============================== */
    showProps: function(val) {
      EM.log("props!", "info", "structmsgDefs.js");
      for (var z in val) {
        EM.log("Z!"+z+"!"+val[z]+"!", "info", "structmsgDefs.js");
      }
      EM.log("props END!", "info", "structmsgDefs.js");
    },
  
    /* ==================== mySelf ===================================== */
    mySelf: function () {
      return this.my_name+"!"+this.oid;
    },
  
  };

  Defs.prototype.constructor = Defs;
  T.Defs = Defs;

  T.log("module: "+name+" initialised!", "info", "structmsgDefs.js");
}, "0.0.1", {});
