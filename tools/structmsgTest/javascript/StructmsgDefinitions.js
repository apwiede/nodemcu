/*====================================================
 * A structured Message Support implementation in Javascript named Esp
 *
 * Structmsg StructmsgDefinitions for Esp (StructmsgDefinitions.js)
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
 * layout of structmsgDefinitions:
 *
 *  StructmsgDefinitions
 *    StructmsgDefinition
 *      name
 *      numFields
 *      maxFields
 *      encoded
 *      encrypted
 *      todecode
 *      totalLgth
 *      fieldInfos (FieldInfoDefinition)
 *        fieldId
 *        fieldType
 *        fieldLgth
 *        
 *====================================================
 */

EM.addModule("Esp-StructmsgDefinitions", function(T, name) {

  /* ==================== StructmsgDefinitions constructor ======================= */

  function StructmsgDefinitions() {
    T.log('constructor called', '2.info', 'StructmsgDefinitions', true);

    var StmsgDefinitions = this;
    var constructor = StmsgDefinitions.constructor;
    StructmsgDefinitions.superclass.constructor.apply(StmsgDefinitions, arguments);

    StmsgDefinitions.numDefinitions = 0;
    StmsgDefinitions.maxDefinitions = 0;
    StmsgDefinitions.definitions = null;
    StmsgDefinitions.fieldNameInfos = null;
    StmsgDefinitions.structmsg = null;

    T.log('constructor end', '2.info', 'StructmsgDefinitions', true);
  }

  T.extend(StructmsgDefinitions, T.EncodeDecode, {
     my_name: "StructmsgDefinitions",
     type_name: "structmsg_definitions",
     flags: 0,

    /* ==================== toString ===================================== */

    toString: function () {
      var stmsgDefs = this;
      return stmsgDefs.mySelf()+"!"+stmsgDefs.numDefinitions+"!";
    },

    /* ==================== toDebugString ===================================== */
    toDebugString: function () {
      var stmsgDefs = this;
      var idx = 0;
      var str = stmsgDefs.mySelf()+"\n";
      str += "  numDefinitions:     "+stmsgDefs.numDefinitions+"\n";
      str += "  maxDefinitions:     "+stmsgDefs.maxDefinitions+"\n";
      str += "  definitions:\n";
      while (idx < stmsgDefs.definitions.length) {
        str += "    "+idx+": "+stmsgDefs.definitions[idx].defName+'\n';
        idx++;
      }
      return str;
    },

    // ============================= getDefinition ========================

    getDefinition: function (defName, result) {
      var stmsgDefs = this;
      var definitionsIdx = 0
      while (definitionsIdx < stmsgDefs.numDefinitions) {
        definition = stmsgDefs.definitions[definitionsIdx];
        if ((definition.defName != null) && (defName == definition.defName)) {
          result.idx = definitionsIdx;
          return stmsgDefs.STRUCT_MSG_ERR_OK;
        }
        definitionsIdx++;
      }
      return stmsgDefs.STRUCT_MSG_ERR_DEFINITION_NOT_FOUND;
    },

    /* ==================== create ===================================== */
    create: function (defName, numFields) {
      var stmsgDefs = this;
      var definitionIdx;
      var definition;
      var idx;

      if (stmsgDefs.numDefinitions >= stmsgDefs.maxDefinitions) {
        if (stmsgDefs.maxDefinitions == 0) {
          stmsgDefs.maxDefinitions = 4;
          stmsgDefs.definitions = new Array(4);
          idx = 0;
          while(idx < stmsgDefs.maxDefinitions) {
            stmsgDefs.definitions[idx] = new T.StructmsgDefinition();
            stmsgDefs.definitions[idx].fieldNameInfos = stmsgDefs.fieldNameInfos;
            stmsgDefs.definitions[idx].structmsg = stmsgDefs.structmsg;
            idx++;
          }
        } else {
          stmsgDefs.maxDefinitions += 2;
          definition = new T.StructmsgDefinition(); 
          definition.fieldNameInfos = stmsgDefs.fieldNameInfos;
          stmsgDefs.definitions.push(definition);
          definition = new T.StructmsgDefinition(); 
          definition.fieldNameInfos = stmsgDefs.fieldNameInfos;
          stmsgDefs.definitions.push(definition);
        }
      }
      // check for unused slot!
      definitionIdx = 0;
      while (definitionIdx < stmsgDefs.maxDefinitions) {
        definition = stmsgDefs.definitions[definitionIdx];
        if (definition.defName == null) {
          definition.encoded = null;
          definition.todecode = null;
          definition.encrypted = null;
          definition.numFields = 0;
          definition.maxFields = numFields;
          definition.fieldInfos = new Array(numFields);
          idx = 0;
          while(idx < numFields) {
            definition.fieldInfos[idx] = new T.FieldInfoDefinition();
            definition.fieldNameInfos = stmsgDefs.fieldNameInfos;
            idx++;
          }
          if (definitionIdx >= stmsgDefs.numDefinitions) {
            stmsgDefs.numDefinitions++;
          }
          definition.defName = defName;
          return stmsgDefs.STRUCT_MSG_ERR_OK;
        }
        definitionIdx++;
      }
      return stmsgDefs.STRUCT_MSG_ERR_NO_SLOT_FOUND;
    },

    /* ==================== addFieldDefinition ===================================== */
    addFieldDefinition: function (defName, fieldName, fieldType, fieldLgth) {
      var stmsgDefs = this;
      var result;
      var definition;
      var obj = new Object();

      obj.idx = -1;
      result = stmsgDefs.getDefinition(defName, obj);
      if(result != stmsgDefs.STRUCT_MSG_ERR_OK) return result;
      definition = stmsgDefs.definitions[obj.idx];
      return definition.addFieldDefinition(fieldName, fieldType, fieldLgth);
    },

    /* ==================== addFieldDefinition ===================================== */
    encodeFieldDefinition: function (defName, data) {
      var stmsgDefs = this;
      var data;
      var definition;
      var obj = new Object();

      obj.idx = -1;
      result = stmsgDefs.getDefinition(defName, obj);
      if(result != stmsgDefs.STRUCT_MSG_ERR_OK) return result;
      definition = stmsgDefs.definitions[obj.idx];
      return definition.encodeFieldDefinition(data);
    },

    /* ==================== decodeFieldDefinition ===================================== */
    decodeFieldDefinition: function (defName, encoded) {
      var stmsgDefs = this;
      var result;
      var definition;
      var obj = new Object();

      obj.idx = -1;
      result = stmsgDefs.getDefinition(defName, obj);
      if(result != stmsgDefs.STRUCT_MSG_ERR_OK) return result;
      definition = stmsgDefs.definitions[obj.idx];
      return definition.decodeFieldDefinition(encoded);
    },

    /* ==================== createMsgFromDefinition ===================================== */
    createMsgFromDefinition: function (defName, resultData) {
      var stmsgDefs = this;
      var result;
      var definition;
      var obj = new Object();

T.log('create_msgFromDefinition3: '+typeof resultData, '1.info', "espStart.js", true);
      resultData.handle = 'unknown2';
      obj.idx = -1;
      result = stmsgDefs.getDefinition(defName, obj);
      if(result != stmsgDefs.STRUCT_MSG_ERR_OK) return result;
      definition = stmsgDefs.definitions[obj.idx];
      return definition.createMsgFromDefinition(resultData);
    },

    /* ==================== getDefinitionNormalFieldNames ===================================== */
    getDefinitionNormalFieldNames: function (defName, resultData) {
      var stmsgDefs = this;
      var result;
      var definition;
      var obj = new Object();

      obj.idx = -1;
      result = stmsgDefs.getDefinition(defName, obj);
      if(result != stmsgDefs.STRUCT_MSG_ERR_OK) return result;
      definition = stmsgDefs.definitions[obj.idx];
      return definition.getDefinitionNormalFieldNames(resultData);
    },

    /* ==================== getDefinitionTableFieldNames ===================================== */
    getDefinitionTableFieldNames: function (defName, resultData) {
      var stmsgDefs = this;
      var result;
      var definition;
      var obj = new Object();

      obj.idx = -1;
      result = stmsgDefs.getDefinition(defName, obj);
      if(result != stmsgDefs.STRUCT_MSG_ERR_OK) return result;
      definition = stmsgDefs.definitions[obj.idx];
      return definition.getDefinitionTableFieldNames(resultData);
    },

    /* ==================== getDefinitionNumTableRows ===================================== */
    getDefinitionNumTableRows: function (defName, resultData) {
      var stmsgDefs = this;
      var result;
      var definition;
      var obj = new Object();

      obj.idx = -1;
      result = stmsgDefs.getDefinition(defName, obj);
      if(result != stmsgDefs.STRUCT_MSG_ERR_OK) return result;
      definition = stmsgDefs.definitions[obj.idx];
      return definition.getDefinitionNumTableRows(resultData);
    },

    /* ==================== getDefinitionNumTableRowFields ===================================== */
    getDefinitionNumTableRowFields: function (defName, resultData) {
      var stmsgDefs = this;
      var result;
      var definition;
      var obj = new Object();

      obj.idx = -1;
      result = stmsgDefs.getDefinition(defName, obj);
      if(result != stmsgDefs.STRUCT_MSG_ERR_OK) return result;
      definition = stmsgDefs.definitions[obj.idx];
      return definition.getDefinitionNumTableRowFields(resultData);
    },

    /* ==================== encrypDefinition ===================================== */
    encryptDefinition: function (key, iv, encoded, resultData) {
      var stmsgDefs = this;
      var result;

      return stmsgDefs.structmsg.encrypt(key, iv, encoded, resultData);
    },

    /* ==================== decryptDefinition ===================================== */
    decryptDefinition: function (key, iv, crypted, resultData) {
      var stmsgDefs = this;
      var result;

      return stmsgDefs.structmsg.decrypt(key, iv, crypted, resultData);
    },

    /* ==================== decryptGeDefinitionName ===================================== */
    decryptGetDefinitionName: function(cryptkey, ivvec, crypted, resultData) {
      var stmsgDefs = this;
      var result;
      var defName;
      var nameOffset;
      var numNormFields;
      var normNamesSize;
      var obj = new Object();
      var decryptedBytes;
      var decrypted;

      result = stmsgDefs.structmsg.decrypt(cryptkey, ivvec, crypted, resultData);
      if(result != stmsgDefs.STRUCT_MSG_ERR_OK) return result;
      decryptedBytes = resultData.decryptedBytes;
      var arr2 = Uint8Array.from(decryptedBytes);
      var decrypted = arr2.buffer;
      defName = ""
      nameOffset = stmsgDefs.STRUCT_MSG_HEADER_LENGTH;
      nameOffset += stmsgDefs.STRUCT_MSG_CMD_HEADER_LENGTH; // cmdKey + cmdLgth
      // randomNum
      nameOffset += stmsgDefs.sizeof('uint32_t');
      // len ids 
      obj.value = null;
      nameOffset = stmsgDefs.uint8Decode(decrypted, nameOffset, obj);
      numNormFields = obj.value;
      // ids vector
      nameOffset += numNormFields * stmsgDefs.sizeof('uint16_t');
      // size of name strings (normnamesSize)
      obj.value = null;
      nameOffset = stmsgDefs.uint16Decode(decrypted, nameOffset, obj);
      normNamesSize = obj.value;
      // names vector
      nameOffset += normNamesSize;
      // definitionSize + nameLgth
      nameOffset += stmsgDefs.sizeof('uint16_t');
      obj.value = null;
      nameOffset = stmsgDefs.uint8Decode(decrypted, nameOffset, obj);
      nameLgth = obj.value;
      // here the name starts
      obj.value = null;
      nameOffset = stmsgDefs.uint8VectorDecode(decrypted, nameOffset, nameLgth, obj) ; // second -1 for stripping off `\0' char
      defName = obj.value;
      resultData.defName = obj.value;
      return stmsgDefs.STRUCT_MSG_ERR_OK;
    },

  });

  T.StructmsgDefinitions = StructmsgDefinitions;

  T.log("module: "+name+" initialised!", "2.info", "StructmsgDefinitions.js");
}, "0.0.1", {});
