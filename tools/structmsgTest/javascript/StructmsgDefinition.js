/*====================================================
 * A structured Message Support implementation in Javascript named Esp
 *
 * Structmsg StructmsgDefinition for Esp (StructmsgDefinition.js)
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

EM.addModule("Esp-StructmsgDefinition", function(T, name) {

  /* ==================== StructmsgDefinition constructor ======================= */

  function StructmsgDefinition() {
    T.log('constructor called', 'info', 'StructmsgDefinition', true);

    var StmsgDefinition = this;
    var constructor = StmsgDefinition.constructor;
    StructmsgDefinition.superclass.constructor.apply(StmsgDefinition, arguments);

    StmsgDefinition.numFields = 0;
    StmsgDefinition.maxFields = 0;
    StmsgDefinition.defName = null;
    StmsgDefinition.encoded = null;
    StmsgDefinition.encrypted = null;
    StmsgDefinition.todecode = null;
    StmsgDefinition.totalLgth = 0;
    StmsgDefinition.fieldInfos = null;
    StmsgDefinition.fieldNameInfos = null;

    T.log('constructor end', 'info', 'StructmsgDefinition', true);
  }

  T.extend(StructmsgDefinition, T.EncodeDecode, {
     my_name: "StructmsgDefinition",
     type_name: "structmsg_definition",
     flags: 0,

    /* ==================== toString ===================================== */

    toString: function () {
      var stmsgDef = this;
      return stmsgDef.mySelf()+"!"+stmsgDef.numFields+"!";
    },

    /* ==================== toDebugString ===================================== */
    toDebugString: function () {
      var stmsgDef = this;
      var idx;
      var str = stmsgDef.mySelf()+"\n";
      str += "  name:        "+(stmsgDef.defName == null ? "null" : stmsgDef.defName)+"\n";
      str += "  numFields:   "+stmsgDef.numFields+"\n";
      str += "  maxFields:   "+stmsgDef.maxFields+"\n";
      str += "  encoded:     "+(stmsgDef.encoded == null ? "No" : "Yes")+"\n";
      str += "  todecode:    "+(stmsgDef.todecode == null ? "No" : "Yes")+"\n";
      str += "  encrypted:   "+(stmsgDef.encrypted == null ? "No" : "Yes")+"\n";
      str += "  totalLgth:   "+stmsgDef.totalLgth+"\n";
      str += "  fieldInfos:  "+(stmsgDef.fieldInfos == null ? "null\n" : "")+"\n";
      idx = 0;
      while (idx < stmsgDef.numFields) {
        fieldInfo = stmsgDef.fieldInfos[idx];
        str += "    idx: "+idx+" "+fieldInfo.toDebugString();
        idx++;
      }
      return str;
    },

    // ============================= normalFieldNamesEncode ========================
    
    normalFieldNamesEncode: function(data, offset, res, numEntries, size) {
      var stmsgDef = this;
      var idx;
      var namesOffset = 0;
      var fieldInfo;
      var result;
      var nameIdx;
      var fieldName;
      var normNamesOffset;
      var obj = new Object();
    
print("> namesEncode offset: ",offset,"!",numEntries,"!",size,"!",res);
      // first the keys
      offset = stmsgDef.uint8Encode(data, offset, numEntries);
      idx = 0;
      normNamesOffsets = new Array();
print("rno0: ", normNamesOffsets.length);
      while (idx < stmsgDef.numFields) {
        fieldInfo = stmsgDef.fieldInfos[idx];
        if (fieldInfo.fieldId < stmsgDef.STRUCT_MSG_SPEC_FIELD_LOW) { 
print("rno1: ", normNamesOffsets.length);
          result = stmsgDef.fieldNameInfos.getFieldIdName(fieldInfo.fieldId, obj);
          if(result != stmsgDef.STRUCT_MSG_ERR_OK) return result;
          fieldName = obj.fieldName;
          offset = stmsgDef.uint16Encode(data, offset, namesOffset);
          normNameOffset = new Object();
          normNameOffset.id = fieldInfo.fieldId;
          normNameOffset.offset = namesOffset;
          normNamesOffsets.push(normNameOffset);
print("rno2: ", normNamesOffsets.length);
          normNameOffset++;
          namesOffset += fieldName.length + 1;
        }
        idx++;
      }
print("rno3: ", normNamesOffsets.length);
      res.normNamesOffsets = normNamesOffsets;
print("rno4: ", normNamesOffsets.length);
print("rno: ",res.normNamesOffsets.length);
      // and now the names
      offset = stmsgDef.uint16Encode(data, offset, size);
      idx = 0;
      nameIdx = 1;
      while (idx < stmsgDef.numFields) {
        fieldInfo = stmsgDef.fieldInfos[idx];
        if (fieldInfo.fieldId < stmsgDef.STRUCT_MSG_SPEC_FIELD_LOW) { 
          result = stmsgDef.fieldNameInfos.getFieldIdName(fieldInfo.fieldId, obj);
          if(result != stmsgDef.STRUCT_MSG_ERR_OK) return result;
          fieldName = obj.fieldName;
          offset = stmsgDef.uint8VectorEncode(data, offset, fieldName, fieldName.length);
          if (nameIdx < numEntries) {
            offset = stmsgDef.uint8Encode(data, offset, 0);
          } else {
            offset = stmsgDef.uint8Encode(data, offset, 0);
          }
          nameIdx++;
        }
        idx++;
      }
      return offset;
    },
    
    // ============================= definitionEncode ========================
    
    definitionEncode: function(data, offset, normNamesOffsets) {
      var stmsgDef = this;
      var idx;
      var idIdx;
      var nameOffset;
      var found;
      var fieldInfo;
      var fieldId;
      var fieldType;
      var fieldLgth;
    
print("nno2: ",normNamesOffsets);
      idx= 0;
for (z in normNamesOffsets) {
print("z2: ",z);
}
print("nno3: ",typeof normNamesOffsets);
print("nno4: ",typeof normNamesOffsets);
      offset = stmsgDef.uint8Encode(data, offset, stmsgDef.numFields); 
print("denc2: ", data.byteLength," offset ",offset);
      if (offset < 0) return encDec.STRUCT_MSG_ERR_ENCODE_ERROR;
print("numFields: ",stmsgDef.numFields);
      while (idx < stmsgDef.numFields) {
        fieldInfo = stmsgDef.fieldInfos[idx];
        if (fieldInfo.fieldId < stmsgDef.STRUCT_MSG_SPEC_FIELD_LOW) {
          idIdx = 0;
          found = 0;
          while (idIdx < stmsgDef.numFields) {
print("idIdx: ",idIdx);
            if (normNamesOffsets[idIdx].id == 0) {
              // id 0 is not used to be able to stop here!!
              break;
            }
            if (fieldInfo.fieldId == normNamesOffsets[idIdx].id) {
              nameOffset = normNamesOffsets[idIdx].offset;
              found = 1;
              break;
            }
            idIdx++;
          }
          if (!found) {
            return stmsgDef.STRUCT_MSG_ERR_FIELD_NOT_FOUND;
          }
          offset = stmsgDef.uint16Encode(data, offset, nameOffset);
print("offset1: ", offset);
        } else {
          offset = stmsgDef.uint16Encode(data, offset, fieldInfo.fieldId);
print("offset2: ", offset);
        }
print("denc3 idx: ",idx,"!", data.byteLength," offset ",offset);
        offset = stmsgDef.uint8Encode(data, offset, fieldInfo.fieldType);
        offset = stmsgDef.uint16Encode(data, offset, fieldInfo.fieldLgth);
        idx++;
      }
print("definitionEncode done");
      return offset;
    },
    
    // ============================= definitionDecode ========================
    
    definitionDecode: function(data, offset) {
      var stmsgDef = this;
      var definitionLgth;
      var nameLgth;
      var numNameEntries;
      var name;
      var idsStart;
      var idsEnd;
      var namesStart;
      var namesEnd;
      var namesSize;
      var definitionStart;
      var fieldId;
      var nameOffset;
      var fieldTypeId;
      var fieldLgth;
      var fieldName;
      var fieldType;
      var definitionIdx;
      var numFields;
      var result;
      var myOffset;
      var namesIdx;
      var defName;
    
      // first the keys
      offset = uint8Decode(data, offset, numNameEntries);
      checkBadOffset(offset);
      idsStart = (data + offset);
      offset += numNameEntries * sizeof(uint16_t);
      idsEnd = (data + offset);
      // and now the names
      offset = uint16Decode(data, offset, namesSize);
      checkBadOffset(offset);
      namesStart = data + offset;
      offset += namesSize;
      definitionStart = (namesStart + namesSize);
      offset = uint16Decode(data, offset, definitionLgth);
      checkBadOffset(offset);
      offset = uint8Decode(data, offset, nameLgth);
      checkBadOffset(offset);
      defname = data+offset;
      offset += nameLgth;
      offset = uint8Decode(data, offset, numFields);
      checkBadOffset(offset);
      result = structmsg_createStructmsgDefinition (name, numFields);
      checkOffsetErrOK(result);
      definitionIdx = 0;
      namesIdx = 0;
      var obj = new Object();
      obj.str = null;
      while (definitionIdx < numFields) { 
        offset = uint16Decode(data, offset, fieldId);
        checkBadOffset(offset);
        if (fieldId > STRUCT_MSG_SPEC_FIELD_LOW) {
          result = structmsg_getIdFieldNameStr(fieldId, fieldName);
          checkOffsetErrOK(result);
        } else {
          fieldId = namesIdx + 1;
          myOffset = (idsStart - data) + (namesIdx * sizeof(uint16_t));
          myOffset = uint16Decode(data, myOffset, nameOffset);
          fieldName = (namesStart+nameOffset);
          namesIdx++;
        }
        offset = uint8Decode(data, offset, fieldTypeId);
        checkBadOffset(offset);
        result = structmsg_getFieldTypeStr(fieldTypeId, obj);
        fieldType = obj.str;
        checkOffsetErrOK(result);
        offset = uint16Decode(data, offset, fieldLgth);
        checkBadOffset(offset);
    //ets_printf("add field: %s fieldId: %d fieldType: %d  %s fieldLgth: %d offset: %d\n", fieldName, fieldId, fieldTypeId, fieldType, fieldLgth, offset);  
        result = structmsg_addFieldDefinition (name, fieldName, fieldType, fieldLgth);
        checkOffsetErrOK(result);
        definitionIdx++;
      }
      return offset;
    },
    
    /* ==================== addFieldDefinition ===================================== */
    addFieldDefinition: function (fieldName, fieldType, fieldLgth) {
      var stmsgDef = this;
      var fieldInfo;
      var fieldId;
      var fieldType;
      var idx;
      var obj = new Object();

      if (stmsgDef.numFields >= stmsgDef.maxFields) {
        return stmsgDef.STRUCT_MSG_ERR_DEFINITION_TOO_MANY_FIELDS;
      }
      fieldInfo = stmsgDef.fieldInfos[definition.numFields];
      obj.fieldId = -1;
      result = stmsgDef.fieldNameInfos.getFieldNameId(fieldName, obj, stmsgDef.STRUCT_MSG_INCR);
      if(result != stmsgDef.STRUCT_MSG_ERR_OK) return result;
      fieldInfo.fieldId = obj.fieldId;
      obj.value = -1;
      result = stmsgDef.getFieldTypeId(fieldType, obj);
      if(result != stmsgDef.STRUCT_MSG_ERR_OK) return result;
      fieldInfo.fieldType = obj.value;
      fieldInfo.fieldLgth = fieldLgth;
      stmsgDef.fieldInfos[definition.numFields] = fieldInfo;
      stmsgDef.numFields++;
      return stmsgDef.STRUCT_MSG_ERR_OK;
    },

    /* ==================== encodeFieldDefinition ===================================== */
    encodeFieldDefinition: function (data) {
      var stmsgDef = this;

      var randomNum;
    
      //FIXME!!
      var src = 123;
      var dst = 456;

      var totalLgth;
      var cmdLgth;
      var cmdKey = stmsgDef.STRUCT_MSG_DEFINITION_CMD_KEY;
      var fillerSize;
      var encoded;
      var offset;
      var crc;
      var myLgth;
      var definitionPayloadSize;
      var payloadSize;
      var idx;
      var fieldInfo;
      var numNormFields;
      var normNamesSize;
      var obj = new Object();
      var fieldName;
      var normNamesOffsets;
      var result;

      numNormFields = 0;
      normNamesSize = 0;
      idx = 0;
      while (idx < stmsgDef.numFields) {
        fieldInfo = stmsgDef.fieldInfos[idx];
        if (fieldInfo.fieldId < stmsgDef.STRUCT_MSG_SPEC_FIELD_LOW) {
print("1: ", fieldInfo.toDebugString());
          result = stmsgDef.fieldNameInfos.getFieldIdName(fieldInfo.fieldId, obj);
print("2");
          if(result != stmsgDef.STRUCT_MSG_ERR_OK) return result;
          fieldName = obj.fieldName;
print("fieldName: ", fieldName, " l: ",stmsgDef.defName.length+1);
          numNormFields++;
          normNamesSize += fieldName.length + 1;
        }
        idx++;
      }
      normNamesOffsets = null;
      payloadSize = stmsgDef.STRUCT_MSG_CMD_HEADER_LENGTH; // cmdKey + cmdLgth
      // randomNum
//FIXME!!
//      payloadSize += sizeof(uint32_t);
      payloadSize += 4;
      // len ids + ids (numNormFields * (uint16_t)) + len Names + names size
//      payloadSize += sizeof(uint8_t) + (numNormFields * sizeof(uint16_t)) + sizeof(uint16_t) + normNamesSize;
      payloadSize += 1 + (numNormFields * 2) + 2 + normNamesSize;
print("normNamesSize: ",normNamesSize);
      // definitionPayloadSize
    
      // definitionLgth + nameLgth + name of Definition
//      definitionPayloadSize = sizeof(uint16_t) + sizeof(uint8_t) + (c_strlen(name) + 1);
      definitionPayloadSize = 2 + 1 + name.length + 1;
      // numFields (uint8_t) + numFields * (fieldId uint16_t, fieldType uint8_t, fieldLgth uint16_t)
//      definitionPayloadSize += sizeof(uint8_t) + definition.numFields * (sizeof(uint16_t) + sizeof(uint8_t) + sizeof(uint16_t));
      definitionPayloadSize += 1 + definition.numFields * 2 + 1 + 2;
print("defpaysize: ",definitionPayloadSize," ",definition.numFields);
print("paysize: ",payloadSize);
    
      payloadSize += definitionPayloadSize;
      fillerSize = 0;
//      myLgth = payloadSize + sizeof(uint16_t); // sizeof(uint16_t) for CRC
      myLgth = payloadSize + 2; // sizeof(uint16_t) for CRC
      while ((myLgth % 16) != 0) {
        myLgth++;
        fillerSize++;
      }
//      cmdLgth = payloadSize + fillerSize + sizeof(uint16_t);
      cmdLgth = payloadSize + fillerSize + 2;
      totalLgth = stmsgDef.STRUCT_MSG_HEADER_LENGTH + cmdLgth;
print("cmdLgth: ",cmdLgth," total: ",totalLgth);
      stmsgDef.totalLgth = totalLgth;
      stmsgDef.encoded = new ArrayBuffer(totalLgth);
      encoded = stmsgDef.encoded;
      offset = 0;
      offset = stmsgDef.uint16Encode(encoded, offset, src); 
      if (offset < 0) return encDec.STRUCT_MSG_ERR_ENCODE_ERROR;
      offset = stmsgDef.uint16Encode(encoded, offset, dst); 
      if (offset < 0) return encDec.STRUCT_MSG_ERR_ENCODE_ERROR;
      offset = stmsgDef.uint16Encode(encoded, offset, totalLgth); 
      if (offset < 0) return encDec.STRUCT_MSG_ERR_ENCODE_ERROR;
      offset = stmsgDef.uint16Encode(encoded, offset, cmdKey); 
      if (offset < 0) return encDec.STRUCT_MSG_ERR_ENCODE_ERROR;
      offset = stmsgDef.uint16Encode(encoded, offset, cmdLgth); 
      if (offset < 0) return encDec.STRUCT_MSG_ERR_ENCODE_ERROR;
      offset = stmsgDef.randomNumEncode(encoded, offset, obj); 
      if (offset < 0) return encDec.STRUCT_MSG_ERR_ENCODE_ERROR;
print("encode1 random: offset: ", offset);
      var obj2 = new Object();
      obj2.normNamesOffsets = null;
      offset = stmsgDef.normalFieldNamesEncode(encoded, offset, obj2, numNormFields, normNamesSize);
      if (offset < 0) return encDec.STRUCT_MSG_ERR_ENCODE_ERROR;
for (z in obj2) {
print("z: ",z);
}
      normNamesOffsets = obj2.normNamesOffsets;
print("nnx: ",obj2.normNamesOffsets);
for (z in normNamesOffsets) {
print("za: ",z);
}
 offset = stmsgDef.uint16Encode(encoded, offset, definitionPayloadSize); 
print("offset2: ", offset);
      if (offset < 0) return encDec.STRUCT_MSG_ERR_ENCODE_ERROR;
      offset = stmsgDef.uint8Encode(encoded, offset, stmsgDef.defName.length+1); 
print("offset3: ", offset);
      if (offset < 0) return encDec.STRUCT_MSG_ERR_ENCODE_ERROR;
      offset = stmsgDef.uint8VectorEncode(encoded, offset, stmsgDef.defName, stmsgDef.defName.length); 
      if (offset < 0) return encDec.STRUCT_MSG_ERR_ENCODE_ERROR;
print("offset4: ", offset);
      offset = stmsgDef.uint8Encode(encoded, offset, 0); 
      if (offset < 0) return encDec.STRUCT_MSG_ERR_ENCODE_ERROR;
print("encode1 defName: offset: ", offset," nno: ",typeof normNamesOffsets);
for (z in normNamesOffsets) {
print("z1: ",z);
}
      offset = stmsgDef.definitionEncode(encoded, offset, normNamesOffsets);
print("after definitionEncode");
      if (offset < 0) return encDec.STRUCT_MSG_ERR_ENCODE_ERROR;
      offset = stmsgDef.fillerEncode(encoded, offset, fillerSize, obj);
      if (offset < 0) return encDec.STRUCT_MSG_ERR_ENCODE_ERROR;
      offset = stmsgDef.crcEncode(encoded, offset, totalLgth, obj, stmsgDef.STRUCT_MSG_HEADER_LENGTH);
      crc = obj.value;
      if (offset < 0) return encDec.STRUCT_MSG_ERR_ENCODE_ERROR;
print("after crc offset: ",offset," totalLgth: ",totalLgth," crc: ",crc.toString(16));
      data.data = encoded;
      return stmsgDef.STRUCT_MSG_ERR_OK;
    },

    /* ==================== decodeFieldDefinition ===================================== */
    decodeFieldDefinition: function (encoded) {
      var stmsgDef = this;

      var numNormFields;
      var namesNormSize;
      var fillerSize;
      var result;
      var idx;
      var fieldName;
      var definitionPayloadSize;
      var payloadSize;
      var myLgth;
      var definition;
      var fieldInfo;
      var offset;
      var crcOffset;
      var src;
      var dst;
      var totalLgth;
      var cmdKey;
      var cmdLgth;
      var crc;
      var randomNum;
      var fillerStr;
      var filler = fillerStr;;
      var numEntries;
    
      offset = 0;
      offset = uint16Decode(data, offset, src); 
      checkDecodeOffset(offset);
      offset = uint16Decode(data, offset, dst); 
      checkDecodeOffset(offset);
      offset = uint16Decode(data, offset, totalLgth); 
      checkDecodeOffset(offset);
      offset = uint16Decode(data, offset, cmdKey); 
      checkDecodeOffset(offset);
      if (cmdKey != STRUCT_MSG_DEFINITION_CMD_KEY) {
        return STRUCT_MSG_ERR_BAD_DEFINTION_CMD_KEY;
      }
      offset = uint16Decode(data, offset, cmdLgth); 
      checkDecodeOffset(offset);
      offset = uint32Decode(data, offset, randomNum); 
      checkDecodeOffset(offset);
      // now check the crc
      crcOffset = totalLgth - sizeof(uint16_t);
      crcOffset = crcDecode(data, crcOffset, cmdLgth, crc, STRUCT_MSG_HEADER_LENGTH);
      offset = definitionDecode(data, offset, definition, fieldNameDefinitions);
      checkDecodeOffset(offset);
      myLgth = offset + sizeof(uint16_t);
      fillerSize = 0;
      while ((myLgth % 16) != 0) {
        myLgth++;
        fillerSize++;
      }
      offset = fillerDecode(data, offset, fillerSize, filler);
      checkDecodeOffset(offset);

    },

  });

  T.StructmsgDefinition = StructmsgDefinition;

  T.log("module: "+name+" initialised!", "info", "StructmsgDefinition.js");
}, "0.0.1", {});
