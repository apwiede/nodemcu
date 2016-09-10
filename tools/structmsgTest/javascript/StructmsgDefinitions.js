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
    T.log('constructor called', 'info', 'StructmsgDefinitions', true);

    var StmsgDefinitions = this;
    var constructor = StmsgDefinitions.constructor;
    StructmsgDefinitions.superclass.constructor.apply(StmsgDefinitions, arguments);

    StmsgDefinitions.numDefinitions = 0;
    StmsgDefinitions.maxDefinitions = 0;
    StmsgDefinitions.definitions = null;
    StmsgDefinitions.fieldNameInfos = null;

    T.log('constructor end', 'info', 'StructmsgDefinitions', true);
  }

  T.extend(StructmsgDefinitions, T.Defines, {
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
        str += "    "+idx+": "+stmsgDefs.definitions[idx].name+'\n';
        idx++;
      }
      return str;
      return str;
    },

    // ============================= getDefinition ========================

    getDefinition: function (name, result) {
      var stmsgDefs = this;
      var definitionsIdx = 0
      while (definitionsIdx < stmsgDefs.numDefinitions) {
        definition = stmsgDefs.definitions[definitionsIdx];
        if ((definition.name != null) && (name == definition.name)) {
          result.idx = definitionsIdx;
          return stmsgDefs.STRUCT_MSG_ERR_OK;
        }
        definitionsIdx++;
      }
      return stmsgDefs.STRUCT_MSG_ERR_DEFINITION_NOT_FOUND;
    },

    /* ==================== create ===================================== */
    create: function (name, numFields) {
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
print(">>fieldNameInfos: ",stmsgDefs.fieldNameInfos.toDebugString(),"!",typeof stmsgDefs.fieldNameInfos.getFieldNameId);
      definitionIdx = 0;
      while (definitionIdx < stmsgDefs.maxDefinitions) {
        definition = stmsgDefs.definitions[definitionIdx];
        if (definition.name == null) {
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
          definition.name = name;
print("create def: ",definition.toDebugString());
          return stmsgDefs.STRUCT_MSG_ERR_OK;
        }
        definitionIdx++;
      }
      return stmsgDefs.STRUCT_MSG_ERR_NO_SLOT_FOUND;

    },

    /* ==================== addFieldDefinition ===================================== */
    addFieldDefinition: function (name, fieldName, fieldType, fieldLgth) {
      var stmsgDefs = this;
      var result;
      var definition;
      var obj = new Object();

      obj.idx = -1;
      result = stmsgDefs.getDefinition(name, obj);
print("addFieldDefinition: ",result," ",obj.idx);
      if(result != stmsgDefs.STRUCT_MSG_ERR_OK) return result;
      definition = stmsgDefs.definitions[obj.idx];
      return definition.addFieldDefinition(fieldName, fieldType, fieldLgth);
    },

  });

  T.StructmsgDefinitions = StructmsgDefinitions;

  T.log("module: "+name+" initialised!", "info", "StructmsgDefinitions.js");
}, "0.0.1", {});
