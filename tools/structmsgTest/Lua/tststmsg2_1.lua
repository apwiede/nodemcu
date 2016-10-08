function checkErrOK(result, fieldName)
  if (result ~= "OK") then
    print(tostring(result).." for field: "..fieldName)
  end
end



handle=structmsg2.create(15)

function initFields()
  result=structmsg2.addField(handle,"@src","uint16_t",2)
  checkErrOK(result, "@src")
  result=structmsg2.addField(handle,"@dst","uint16_t",2)
  checkErrOK(result, "@dst")
  result=structmsg2.addField(handle,"@totalLgth","uint16_t",2)
  checkErrOK(result, "@totalLgth")
  result=structmsg2.addField(handle,"@cmdKey","uint16_t",2)
  checkErrOK(result, "@cmdKey")
--result=structmsg2.addField(handle,"@cmdLgth","uint16_t",2)
  checkErrOK(result, "@cmdLgth")
  result=structmsg2.addField(handle,"@randomNum","uint32_t",4)
  checkErrOK(result, "@randomNum")
  result=structmsg2.addField(handle,"@sequenceNum","uint32_t",4)
  checkErrOK(result, "@sequenceNum")
  result=structmsg2.addField(handle,"pwd","uint8_t*",16)
  checkErrOK(result, "pwd")
  result=structmsg2.addField(handle,"@tablerows","uint8_t",2)
  checkErrOK(result, "@tabelrows")
  result=structmsg2.addField(handle,"@tablerowfields","uint8_t",3)
  checkErrOK(result, "@tablerowfields")
  result=structmsg2.addField(handle,"ssid","uint8_t*",16)
  checkErrOK(result, "ssid")
  result=structmsg2.addField(handle,"bssid","int8_t",1)
  checkErrOK(result, "bssid")
  result=structmsg2.addField(handle,"channel","uint8_t",1)
  checkErrOK(result, "channel")
  result=structmsg2.addField(handle,"fld1","uint8_t*",4)
  checkErrOK(result, "fld1")
  result=structmsg2.addField(handle,"@filler","uint8_t*",0)
  checkErrOK(result, "@filler")
--result=structmsg2.addField(handle,"@crc","uint8_t",1)
--  checkErrOK(result, "@crc")
  result=structmsg2.addField(handle,"@crc","uint16_t",2)
  checkErrOK(result, "@crc")
--result=structmsg2.dump(handle)
  checkErrOK(result, "dump")
  result=structmsg2.init(handle)
  checkErrOK(result, "init")
end

function setFieldValues()
--  result=structmsg2.dump(handle)
--  checkErrOK(result, "dump")
  result=structmsg2.setFieldValue(handle,"@src",123)
  checkErrOK(result, "@src")
  result=structmsg2.setFieldValue(handle,"@dst",456)
  checkErrOK(result, "@dst")
  result=structmsg2.setFieldValue(handle,"@cmdKey",789)
  checkErrOK(result, "@cmdKey")
  result=structmsg2.setFieldValue(handle,"pwd","apwiede1apwiede2")
  checkErrOK(result, "pwd")
  result=structmsg2.setTableFieldValue(handle,"ssid",0,"ssid1234ssid5678")
  checkErrOK(result, "ssid")
  result=structmsg2.setTableFieldValue(handle,"bssid",0,-57)
  checkErrOK(result, "bssid")
  result=structmsg2.setTableFieldValue(handle,"channel",0,140)
  checkErrOK(result, "channel")
  result=structmsg2.prepare(handle)
  checkErrOK(result, "prepare")
--  result=structmsg2.dump(handle)
--  checkErrOK(result, "dump")
--  msggData=structmsg2.getMsgData(handle)
--  result=structmsg2.setMsgData(handle,msgData)
--  checkErrOK(result, "setMsgData")
end

initFields()
--setFieldValues()

result=structmsg2.createMsgDef(handle)
checkErrOK(result, "createMsgDef")
result=structmsg2.initDef(handle)
checkErrOK(result, "initDef")
result=structmsg2.prepareDef(handle)
checkErrOK(result, "prepareData")
--result=structmsg2.dumpDefFields(handle)
-- checkErrOK(result, "dumpDefFields")
defData=structmsg2.getDef(handle)

handle2=structmsg2.create(15)
result=structmsg2.createMsgDef(handle2)
checkErrOK(result, "createMsgDef")
result=structmsg2.setDef(handle2, defData)
checkErrOK(result, "setDef")
--result=structmsg2.dumpDefFields(handle2)
--checkErrOK(result, "dumpDefFields")
result=structmsg2.createMsgFromDef(handle2)
checkErrOK(result, "createMsgFromDef")
result=structmsg2.dump(handle2)
checkErrOK(result, "dump")

