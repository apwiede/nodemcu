/*====================================================
 * A structured Message Support implementation in Javascript named Esp
 *
 * Structmsg Api for Esp (structmsgApi.js)
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

EM.addModule("Esp-structmsgApi", function(T, name) {

  /* ==================== Api constructor ======================= */

  function Api() {
    T.log('constructor called', 'info', 'structmsgApi', true);

    var api = this;
    var constructor = api.constructor;
    Api.superclass.constructor.apply(api, arguments);
    api.fieldNameInfos = new T.FieldNameInfos();
    api.structmsg = new T.StructmsgInfos();
    api.structmsg.fieldNameInfos = api.fieldNameInfos;
    api.definitions = new T.StructmsgDefinitions();
    api.definitions.fieldNameInfos = api.fieldNameInfos;

    T.log('constructor end', 'info', 'structmsgApi', true);
  }

  T.extend(Api, T.StructmsgInfos, {
     my_name: "Api",
     type_name: "api",
     flags: 0,

    /* ==================== toString ===================================== */

    toString: function () {
      var api = this;
      return api.mySelf()+"!";
    },

    /* ==================== toDebugString ===================================== */
    toDebugString: function () {
      var api = this;
      var str = api.mySelf()+"\n";
      return str;
    },

    // ============================= checkOKOrErr ========================
    
    checkOKOrErr: function(result, where, fieldName) {
      var api = this;
      if  (result == api.STRUCT_MSG_ERR_OK)  {
        return "OK"
      }
      if (typeof where === 'undefined') {
        where = "";
      }
      if (typeof fieldName === 'undefined') {
        fieldName = "";
      }
print("error result: ", result," ",api.structmsgErrId2Str[result]);
      var str = api.structmsgErrId2Str[result];
      var errStr = "ERROR";
      switch (result) {
        case api.STRUCT_MSG_ERR_VALUE_NOT_SET:
          return errStr+" "+where+': value for field:'+fieldName+' not set';
          break;
        case api.STRUCT_MSG_ERR_VALUE_OUT_OF_RANGE:
          return errStr+" "+where+': field: '+fieldName+' value out of range';
          break;
        case api.STRUCT_MSG_ERR_BAD_VALUE:
          return errStr+" "+where+': field: '+fieldName+' bad value';
          break;
        case api.STRUCT_MSG_ERR_BAD_FIELD_TYPE:
          return errStr+" "+where+': field: '+fieldName+' bad field type';
          break;
        case api.STRUCT_MSG_ERR_FIELD_NOT_FOUND:
          return errStr+" "+where+': field: '+fieldName+' not found';
          break;
        case api.STRUCT_MSG_ERR_BAD_HANDLE:
          return errStr+" "+where+': bad handle';
          break;
        case api.STRUCT_MSG_ERR_OUT_OF_MEMORY:
          return errStr+' out of memory';
          break;
        case api.STRUCT_MSG_ERR_HANDLE_NOT_FOUND:
          return errStr+': handle not found';
          break;
        case api.STRUCT_MSG_ERR_NOT_ENCODED:
          return errStr+': not encoded';
          break;
        case api.STRUCT_MSG_ERR_DECODE_ERROR:
          return errStr+': decode error';
          break;
        case api.STRUCT_MSG_ERR_BAD_CRC_VALUE:
          return errStr+': bad crc val';
          break;
        case api.STRUCT_MSG_ERR_CRYPTO_INIT_FAILED:
          return errStr+': crypto init failed';
          break;
        case api.STRUCT_MSG_ERR_CRYPTO_OP_FAILED:
          return errStr+': crypto op failed';
          break;
        case api.STRUCT_MSG_ERR_CRYPTO_BAD_MECHANISM:
          return errStr+': crypto bad mechanism';
          break;
        case api.STRUCT_MSG_ERR_NOT_ENCRYPTED:
          return errStr+': not encrypted';
          break;
        case api.STRUCT_MSG_ERR_DEFINITION_NOT_FOUND:
          return errStr+': definiton not found';
          break;
        case api.STRUCT_MSG_ERR_BAD_SPECIAL_FIELD:
          return errStr+': bad special field';
          break;
        case api.STRUCT_MSG_ERR_DEFINITION_TOO_MANY_FIELDS:
          return errStr+': definition too many fields';
          break;
        case api.STRUCT_MSG_ERR_BAD_TABLE_ROW:
          return errStr+': bad table row';
          break;
        case api.STRUCT_MSG_ERR_TOO_MANY_FIELDS:
          return errStr+': too many fields';
          break;
        case api.STRUCT_MSG_ERR_BAD_DEFINTION_CMD_KEY:
          return errStr+': bad definition cmd key';
          break;
        case api.STRUCT_MSG_ERR_NO_SLOT_FOUND:
          return errStr+': no slot found';
          break;
        case api.STRUCT_MSG_ERR_DUPLICATE_FIELD:
          return errStr+': duplicate field';
          break;
        default:
          return errStr+': funny result error code';
          break;
      }
    },
    
    // ============================= encdec_msgDefinition ========================
    
    encdec_msgDefinition: function(handle, key, enc, iv, buf, lgth) {
      var stmsgApi = this;
      var result  = encdecDefinition(handle, key, enc, iv, buf, lgth);
      return result;
    },
    
    // ============================= create ========================
    
    create: function(numFields) {
      var stmsgApi = this;
      var handle = stmsgApi.structmsg.create(numFields);
      if (handle != null)  {
        return handle;
      }
      return stmsgApi.checkOKOrErr(stmsgApi.structmsg.result, "create", "");
    },
    
    // ============================= delete ========================
    
    delete: function(handle) {
      var stmsgApi = this;
print("delete not yet implemented");
      var result = cmd.delete(handle);
      return stmsgApi.checkOKOrErr(result, "delete", "");
    },
    
    // ============================= encode ========================
    
    encode: function(handle)  {
      var stmsgApi = this;
      var result = stmsgApi.structmsg.encode(handle);
      return stmsgApi.checkOKOrErr(result, "encode", "");
    },
    
    // ============================= get_encoded ========================
    
    get_encoded: function(handle) {
      var stmsgApi = this;
print("get_encoded not yet implemented");
      var result = cmd.get_encoded(handle, encoded, lgth);
      if (result == STRUCT_MSG_ERR_OK) {
        return encoded;
      }
      return stmsgApi.checkOKOrErr(result, "get_encoded", "");
    },
    
    // ============================= decode ========================
    
    decode: function(handle, result) {
      var stmsgApi = this;
      var result = stmsgApi.structmsg.decode(handle,result);
      return stmsgApi.checkOKOrErr(result, "decode", "");
    },
    
    // ============================= dump ========================
    
    dump: function(handle) {
      var stmsgApi = this;
print("dump not yet implemented");
      var result = cmd.dump(handle);
      return stmsgApi.checkOKOrErr(result, "dump", "");
    },
    
    // ============================= encrypt ========================
    
    encrypt: function(handle, key, iv) {
      var stmsgApi = this;
print("encrypt not yet implemented");
      if (typeof iv === 'undefined') {
        iv = "";
      }
      var result  = cmd.encrypt(handle, null, 0, key, iv, buf, lgth);
      if (result == STRUCT_MSG_ERR_OK) {
        return buf;
      }
      return stmsgApi.checkOKOrErr(result, "encrypt", "");
    },
    
    // ============================= decrypt ========================
    
    decrypt: function(handle, key, iv, crypted) {
      var stmsgApi = this;
print("encrypt not yet implemented");
      var result = cmd.decrypt(handle, null, 0, key, iv, crypted, buf, lgth);
      if (result == STRUCT_MSG_ERR_OK) {
        return buf;
      }
      return stmsgApi.checkOKOrErr(result, "decrypt", "");
    },
    
    // ============================= add_field ========================
    
    add_field: function(handle, fieldStr, fieldType, fieldLgth) {
      var stmsgApi = this;
      var result = stmsgApi.structmsg.addField(handle, fieldStr, fieldType, fieldLgth);
      return stmsgApi.checkOKOrErr(result, "add_field", "");
    },
    
    // ============================= set_fillerAndCrc ========================
    
    set_fillerAndCrc: function(handle) {
      var stmsgApi = this;
print("encrypt not yet implemented");
      var result = structmsg_setFillerAndCrc(handle);
      return stmsgApi.checkOKOrErr(result, "set_fillerAndCrc", "");
    },
    
    // ============================= set_fieldValue ========================
    
    set_fieldValue: function(handle, fieldName, value) {
      var stmsgApi = this;
      var result = stmsgApi.structmsg.setFieldValue(handle, fieldName, value);
      return stmsgApi.checkOKOrErr(result,"set_fieldValue", fieldName);
    },
    
    // ============================= set_tableFieldValue ========================
    
    set_tableFieldValue: function(handle, fieldName, row, value) {
      var stmsgApi = this;
      var result = stmsgApi.structmsg.setTableFieldValue(handle, fieldName, row, value);
      return stmsgApi.checkOKOrErr(result, "set_tableFieldValue", fieldName);
    },
    
    // ============================= get_fieldValue ========================
    
    get_fieldValue: function(handle, fieldName, valueVar) {
      var stmsgApi = this;
print("get_fieldValue not yet implemented");
      var result = cmd.get_fieldValue(handle, fieldName, valueC);
      return stmsgApi.checkOKOrErr(result, "get_fieldValue", fieldName);
    },
    
    // ============================= get_tableFieldValue ========================
    
    get_tableFieldValue: function(handle, fieldName, row, valueVar) {
      var stmsgApi = this;
print("get_tableFieldValue not yet implemented");
      var result = cmd.get_tableFieldValue(handle, fieldName, row, value);
      return stmsgApi.checkOKOrErr(result, "get_tableFieldValue", fieldName);
    },
    
    // ============================= set_crypted ========================
    
    set_crypted: function(handle, crypted) {
      var stmsgApi = this;
print("set_encrpyted not yet implemented");
      var result = cmd.set_crypted(handle, crypted);
      return stmsgApi.checkOKOrErr(result, "set_crypted", "");
    },
    
    // ============================= decrypt_getHandle ========================
    
    decrypt_getHandle: function(encrypted, key, iv, handleVar) {
      var stmsgApi = this;
print("decrypt_getHandle not yet implemented");
      var result = cmd.decrypt_getHandle(encrypted, key, iv, handle);
      return stmsgApi.checkOKOrErr(result, "decrypt_getHandle", "");
    },
    
    // ============================= create_definition ========================
    
    create_definition: function(name, numFields) {
      var stmsgApi = this;
      var result = stmsgApi.definitions.create(name, numFields);
      return stmsgApi.checkOKOrErr(result, "create_Definition", "");
    },
    
    // ============================= add_fieldDefinition ========================
    
    add_fieldDefinition: function(name, fieldName, fieldType, fieldLgth) {
      var stmsgApi = this;
      var result = stmsgApi.definitions.addFieldDefinition(name, fieldName, fieldType, fieldLgth);
      return stmsgApi.checkOKOrErr(result, "add_fieldDefinition", "");
    },
    
    // ============================= dump_fieldDefinition ========================
    
    dump_fieldDefinition: function(name) {
      var stmsgApi = this;
print("dump_fieldDefinition not yet implemented");
      var result = structmsg.def.dumpFieldDefinition(name);
      return stmsgApi.checkOKOrErr(result, "dump_fieldDefinition", "");
    },
    
    // ============================= encode_fieldDefinition =================
    
    encode_fieldDefinition: function(name, data) {
      var stmsgApi = this;
      var result = stmsgApi.definitions.encodeFieldDefinition(name, data);
      return stmsgApi.checkOKOrErr(result, "encode_fieldDefinition","");
    },
    
    // ============================= decode_fieldDefinition =================
    
    decode_fieldDefinition: function(name, encoded) {
      var stmsgApi = this;
      var result = stmsgApi.definitions.decodeFieldDefinition(name, encoded);
      return stmsgApi.checkOKOrErr(result, "decode_fieldDefinition", "");
    },
    
    // ============================= set_crypted_definition ========================
    
    set_crypted_definition: function(name, crypted) {
      var stmsgApi = this;
print("set_crypted_definition not yet implemented");
      var result = structmsg.def.setCryptedDefinition(name, crypted);
      return stmsgApi.checkOKOrErr(result, "set_crypted_definition", "");
    },
    
    // ============================= decrypt_getDefinitionName ========================
    
    decrypt_getDefinitionName: function(encrypted, key, iv) {
      var stmsgApi = this;
print("decrypt_getDefinitionName not yet implemented");
      if (typeof iv === 'undefined') {
        iv = "";
      }
      var result = structmsg.def.decryptGetDefinitionName(encrypted, key, iv, name);
      if (result == STRUCT_MSG_ERR_OK) {
        return name;
      }
      return stmsgApi.checkOKOrErr(result, "decrypt_getDefinitionName", "");
    },
    
    // ============================= encrypt_definition ========================
    
    encrypt_definition: function(name, key, iv) {
print("encrypt_definition not yet implemented");
      var stmsgApi = this;
      if (typeof iv === 'undefined') {
        iv = "";
      }
      var result = structmsg.def.encryptDefinition(name, key, iv, buf, lgth);
      if  (result == STRUCT_MSG_ERR_OK) {
        return buf;
      }
      return stmsgApi.checkOKOrErr(result, "encrypt_definition", "");
    },
    
    // ============================= decrypt_definition ========================
    
    decrypt_definition: function(name, key, iv, encrypted) {
print("decrypt_definition not yet implemented");
      var stmsgApi = this;
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
      return stmsgApi.checkOKOrErr(result, "decrypt_definition", "");
    },
    
    // ============================= delete_fieldDefinition ========================
    
    delete_fieldDefinition: function(name) {
print("delete_fieldDefinition not yet implemented");
      var stmsgApi = this;
      var result = structmsg.def.deleteFieldDefinition(name);
      return stmsgApi.checkOKOrErr(result, "delete_fieldDefinition", "");
    },
    
    // ============================= delete_fieldDefinitions ========================
    
    delete_fieldDefinitions: function() {
print("delete_fieldDefinitions not yet implemented");
      var stmsgApi = this;
      var result = structmsg.def.deleteFieldDefinitions();
      return stmsgApi.checkOKOrErr(result, "delete_fieldDefinitions", "");
    },
    
    // ============================= create_msgFromDefinition ========================
    
    create_msgFromDefinition: function(name, handleVar)  {
print("create_msgFromDefinition not yet implemented");
      var stmsgApi = this;
      var result = structmsg.def.createMsgFromDefinition(name, handle);
      return stmsgApi.checkOKOrErr(result, "create_msgFromDefinition", "");
    },
  
    // ============================= get_definitionNormalFieldNames ========================
    
    get_definitionNormalFieldNames: function(name, normalFieldNamesVar)  {
print("gete_definitionNormalFieldNames not yet implemented");
      var stmsgApi = this;
      var result = structmsg.def.getDefinitionNormalFieldNames(name, normalFieldNames);
      return stmsgApi.checkOKOrErr(result, "get_definitionNormalFieldNames", "");
    },
  
    // ============================= get_definitionTableFieldNames ========================
    
    get_definitionTableFieldNames: function(name, tableFieldNamesVar)  {
print("gete_definitionTableFieldNames not yet implemented");
      var stmsgApi = this;
      var result = structmsg.def.getDefinitionTableFieldNames(name, tableFieldNames);
      return stmsgApi.checkOKOrErr(result, "get_definitionTableFieldNames", "");
    },
  
    // ============================= get_definitionNumTableRows ========================
    
    get_definitionNumTableRows: function(name, numTableRowsVar)  {
print("gete_definitionNumTableRows not yet implemented");
      var stmsgApi = this;
      var result = structmsg.def.getDefinitionNumTableRows(name, numTableRows);
      return stmsgApi.checkOKOrErr(result, "get_definitionNumTableRows", "");
    },
  
    // ============================= get_definitionNumTableRowFields ========================
    
    get_definitionNumTableRowFields: function(name, numTableRowFieldsVar)  {
print("gete_definitionNumTableRowFields not yet implemented");
      var stmsgApi = this;
      var result = structmsg.def.getDefinitionNumTableRowFields(name, numTableRowFields);
      return stmsgApi.checkOKOrErr(result, "get_definitionNumTableRowFields", "");
    },
  
    // ============================= get_definitionFieldInfo ========================
    
    get_definitionFieldInfo: function(name, fieldName, fieldInfoVar)  {
print("gete_definitionFieldInfo not yet implemented");
      var stmsgApi = this;
      var result = structmsg.def.getDefinitionFieldInfo(name, fieldName, fieldInfo);
      return stmsgApi.checkOKOrErr(result, "get_definitionFieldInfo", "");
    },
  
    // ============================= get_definitionTableFieldInfo ========================
    
    get_definitionTableFieldInfo: function(name, fieldName, row, fieldInfoVar)  {
print("gete_definitionTableFieldInfo not yet implemented");
      var stmsgApi = this;
      var result = structmsg.def.getDefinitionTableFieldInfo(name, fieldName, row, fieldInfo);
      return stmsgApi.checkOKOrErr(result, "get_definitionTableFieldInfo", "");
    },

  });

  T.Api = Api;

  T.log("module: "+name+" initialised!", "info", "structmsgApi.js");
}, "0.0.1", {});
