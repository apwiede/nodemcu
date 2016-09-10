/*====================================================
 * A structured Message Support implementation in Javascript named Esp
 *
 * Structmsg FieldNameInfos for Esp (FieldNameInfos.js)
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
 * layout of a FieldNamenfos:
 *
 *  definition (FieldNameInfo)
 *    fieldName
 *    fieldId
 *    fieldOffset
 *    refCnt
 *====================================================
 */

EM.addModule("Esp-FieldNameInfos", function(T, name) {

  /* ==================== FieldNameInfos constructor ======================= */

  function FieldNameInfos() {
    T.log('constructor called', 'info', 'FieldNameInfos', true);

    var fieldNameInfos = this;
    var constructor = fieldNameInfos.constructor;
    FieldNameInfos.superclass.constructor.apply(fieldNameInfos, arguments);

    fieldNameInfos.numInfos = 0;
    fieldNameInfos.maxInfos = 0;
    fieldNameInfos.infos = null;

    T.log('constructor end', 'info', 'FieldNameInfos', true);
  }

  T.extend(FieldNameInfos, T.Defines, {
     my_name: "FieldNameInfos",
     type_name: "fieldName_infos",
     flags: 0,

    /* ==================== toString ===================================== */

    toString: function () {
      var fndef = this;
      return fndef.mySelf()+"!";
    },

    /* ==================== toDebugString ===================================== */
    toDebugString: function () {
      var fnInfos = this;
      var str = fnInfos.mySelf()+"\n";
      str += "    numInfos: "+fnInfos.numInfos+"\n";
      str += "    maxInfos: "+fnInfos.maxInfos+"\n";
      str += "    infos:    "+"\n";
      idx = 0;
      while (idx < fnInfos.numInfos) {
        str += fnInfos.infos[idx].toDebugString();
        idx++;
      }
      return str;
    },

    // ============================= getFieldIdName ========================
    
    getFieldIdName: function (id, fieldName) {
      var fnInfos = this;
      // find field name
      var idx = 0;
      while (idx < fieldNameInfos.numInfos) {
        var entry = fieldNameInfos.infos[idx];
        if (entry.fieldId == id) {
          fieldName = entry.fieldName;
          return fnInfos.STRUCT_MSG_ERR_OK;
        }
        idx++;
      }
      return fnInfos.STRUCT_MSG_ERR_FIELD_NOT_FOUND;
    },
    
    // ============================= getFieldNameId ========================

    getFieldNameId: function(fieldName, result, incrRefCnt) {
      var fnInfos = this;
      var nameIdx = 0;
      var firstFreeEntryId;
      var firstFreeEntry;
      var newDefinition;
      var idx;
      var id;
 
      if (fieldName[0] == '@') {
        // find special field name
        if (typeof fnInfos.structmsgSpecialFieldNames2Id[fieldName] != 'undefined') {
          result.fieldId = fnInfos.structmsgSpecialFieldNames2Id[fieldName];
          return fnInfos.STRUCT_MSG_ERR_OK;
        }
        return fnInfos.STRUCT_MSG_ERR_BAD_SPECIAL_FIELD;
      } else {
print("new fieldName: ", fieldName);
        if ((incrRefCnt == fnInfos.STRUCT_MSG_INCR) & (fnInfos.numInfos >= fnInfos.maxInfos)) {
          if (fnInfos.maxInfos == 0) {
            fnInfos.maxInfos = 4;
            fnInfos.infos = new Array(fnInfos.maxInfos);
            idx = 0;
            while (idx < fnInfos.maxInfos) {
              fnInfos.infos[idx] = new T.FieldNameInfo();
              idx++;
            }
          } else {
            fnInfos.maxInfos += 2;
            fnInfos.infos.push(new T.FieldNameInfo());
            fnInfos.infos.push(new T.FieldNameInfo());
          }
        }
        firstFreeEntry = null;
        firstFreeEntryId = 0;
        if (fnInfos.numInfos > 0) {
          // find field name
          nameIdx = 0;
          while (nameIdx < fnInfos.numInfos) {
            nameEntry = fnInfos.infos[nameIdx];
            if ((nameEntry.str != null) && (nameEntry.str == fieldName)) {
              if (incrRefCnt < 0) {
                if (nameEntry.refCnt > 0) {
                  nameEntry.refCnt--;
                }
                if (nameEntry.refCnt == 0) {
                  nameEntry.id = fnInfos.STRUCT_MSG_FREE_FIELD_ID;
                  nameEntry.str = NULL;
                }
              } else {
                if (incrRefCnt > 0) {
                  nameEntry.refCnt++;
                } else {
                  // just get the entry, do not modify
                }
              }
              result.fieldId = nameEntry.id;
              return fnInfos.STRUCT_MSG_ERR_OK;
            }
            if ((incrRefCnt == fnInfos.STRUCT_MSG_INCR) && (nameEntry.id == fnInfos.STRUCT_MSG_FREE_FIELD_ID) && (firstFreeEntry == null)) {
              firstFreeEntry = nameEntry;
              firstFreeEntry.id = nameIdx + 1;
            }
            nameIdx++;
          }
        }
        if (incrRefCnt < 0) {
          return fnInfos.STRUCT_MSG_ERR_OK; // just ignore silently
        } else {
          if (incrRefCnt == 0) {
            return fnInfos.STRUCT_MSG_ERR_FIELD_NOT_FOUND;
          } else {
            if (firstFreeEntry != null) {
              result.fieldid = firstFreeEntry.id;
              firstFreeEntry.refCnt = 1;
              firstFreeEntry.str = fieldName;
            } else {
              newDefinition = fnInfos.infos[fnInfos.numInfos];
              newDefinition.refCnt = 1;
              newDefinition.id = fnInfos.numInfos + 1;
              newDefinition.str = fieldName;
              fnInfos.numInfos++;
              result.fieldId = newDefinition.id;
            }
          }
        }
      }
      return fnInfos.STRUCT_MSG_ERR_OK;
    },

  });

  T.FieldNameInfos = FieldNameInfos;

  T.log("module: "+name+" initialised!", "info", "FieldNameInfos.js");
}, "0.0.1", {});
