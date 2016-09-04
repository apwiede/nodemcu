function checkOK(result, fieldName)
  if (result ~= "OK") then
    print(tostring(result).." for field: "..fieldName)
  end
end

function createAPListMsg(useBig,numRows)
  handle=nil
  handle=structmsg.create(8)
  result=structmsg.addField(handle, "@randomNum", "uint32_t")
  checkOK(result,"@randomNum")
  result=structmsg.addField(handle, "@sequenceNum", "uint32_t")
  checkOK(result,"@sequenceNum")
  result=structmsg.addField(handle, "@tablerows", "uint8_t", numRows)
  checkOK(result,"@tablerows")
  if (useBig) then
    result=structmsg.addField(handle, "@tablerowfields", "uint8_t", 5)
  else
    result=structmsg.addField(handle, "@tablerowfields", "uint8_t", 3)
  end
  checkOK(result,"@tabelrowfields")
  result=structmsg.addField(handle, "ssid", "uint8_t*", 32)
  checkOK(result,"ssid")
  if (useBig) then
    result=structmsg.adddeffield(def, "bssid", "uint8_t*", 17)
    checkOK(result,"bssid")
  end
  result=structmsg.addField(handle, "rssi", "int8_t", 1)
  checkOK(result,"rssi")
  if (useBig) then
    result=structmsg.adddeffield(def, "authmode", "uint8_t", 1)
    checkOK(result,"authmode")
  end
  result=structmsg.addField(handle, "channel", "uint8_t", 1)
  checkOK(result,"channel")
  return handle
end

function fillWithSpaces(str,lgth)
print("str: "..tostring(str).." "..tostring(lgth))
  while (string.len(str) < lgth) do
    str=str.." "
  end
  return str
end

function setFieldValues(t,handle,useBig,numRows)
  row=0
  for ssid,v in pairs(t) do
    local authmode, rssi, bssid, channel = string.match(v, "([^,]+),([^,]+),([^,]+),([^,]+)")
print("ssid: "..tostring(ssid))
print("rssi: "..tostring(rssi))
print("bssid: "..tostring(bssid))
print("authmode: "..tostring(authmode))
print("channel: "..tostring(channel))

    ssid=fillWithSpaces(ssid,32)
    bssid=fillWithSpaces(bssid,17)
    if (useBig) then
      result=structmsg.setTableFieldValue(handle, "ssid",row,ssid)
      checkOK(result,"ssid")
      result=structmsg.setTableFieldValue(handle, "bssid",row,bssid)
      checkOK(result,"bssid")
      result=structmsg.setTableFieldValue(handle, "rssi",row,rssi)
      checkOK(result,"rssi")
      result=structmsg.setTableFieldValue(handle, "authmode",row,authmode)
      checkOK(result,"authmode")
      result=structmsg.setTableFieldValue(handle, "channel",row,channel)
      checkOK(result,"channel")
    else
      result=structmsg.setTableFieldValue(handle, "ssid",row,ssid)
      checkOK(result,"ssid")
      result=structmsg.setTableFieldValue(handle, "rssi",row,rssi)
      checkOK(result,"rssi")
      result=structmsg.setTableFieldValue(handle, "channel",row,channel)
      checkOK(result,"channel")
    end
    row=row+1
  end
end

function buildStmsgAPList(t,useBig,numRows)
  handle=createAPListMsg(useBig,numRows)
  result=structmsg.setFieldValue(handle, "@src",123)
  checkOK(result,"@src")
  result=structmsg.setFieldValue(handle, "@dst",456)
  checkOK(result,"@dst")
  result=structmsg.setFieldValue(handle, "@cmdKey",789)
  checkOK(result,"@cmdKey")
  setFieldValues(t,handle,useBig,numRows)
structmsg.dump(handle)

  result=structmsg.encode(handle)
  checkOK(result,"encode")
print("encoded")
  encoded_pwd1=structmsg.getencoded(handle)
print("GETencoded len: "..tostring(string.len(encoded_pwd1)))
end

