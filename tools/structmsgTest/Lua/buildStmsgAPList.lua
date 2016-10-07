function checkOK(result, fieldName)
  if (result ~= "OK") then
    print(tostring(result).." for field: "..fieldName)
  end
end

function createAPListMsg(useBig,numRows)
  local msgInfos={}
  msgInfos["1"]="@src,uint16_t,2,0"
  msgInfos["2"]="@dst,uint16_t,2,0"
  msgInfos["3"]="@totalLgth,uint16_t,2,0"
  msgInfos["4"]="@cmdKey,uint16_t,2,0"
  msgInfos["5"]="@cmdLgth,uint16_t,2,0"
  msgInfos["6"]="@numFields,uint8_t,1,0"
  msgInfos["7"]="@randomNum,uint32_t,4,0"
  msgInfos["8"]="@tablerows,uint8_t,@numRows,0"
  msgInfos["9"]="@tablerowfields,uint8_t,5,2"
  msgInfos["10"]="@tablerowfields,uint8_t,3,1"
  msgInfos["11"]="ssid,uint8_t*,32,0"
  msgInfos["12"]="bssid,uint8_t*, 17,2"
  msgInfos["13"]="rssi,int8_t,1,0"
  msgInfos["14"]="authmode,uint8_t,1,2"
  msgInfos["15"]="channel,uint8_t,1,0"
  msgInfos["16"]="@filler,uint8_t*,0,0"
  msgInfos["17"]="@crc,uint16_t,2,0"

  numEntries=0
  for x,v in pairs(msgInfos) do
    numEntries=numEntries+1
  end
  if (useBig) then
    flags= 1
  else
    flags=0
  end
  handle=structmsg.createmsgfromlist(numEntries,msgInfos,numRows,flags);
print("handlemsg: "..tostring(handle))
  return handle
end

function fillWithSpaces(str,lgth)
  while (string.len(str) < lgth) do
    str=str.." "
  end
  return str
end

function setFieldValues(t,handle,useBig,numRows)
  result=structmsg.setFieldValue(handle, "@src",123)
  checkOK(result,"@src")
  result=structmsg.setFieldValue(handle, "@dst",456)
  checkOK(result,"@dst")
  result=structmsg.setFieldValue(handle, "@cmdKey",789)
  checkOK(result,"@cmdKey")
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
  setFieldValues(t,handle,useBig,numRows)
--structmsg.dump(handle)

  result=structmsg.encode(handle)
  checkOK(result,"encode")
print("encoded")
--  encoded_pwd1=structmsg.getencoded(handle)
--print("GETencoded len: "..tostring(string.len(encoded_pwd1)))
  encrypted=structmsg.encrypt(handle,cryptkey)
print("encrypted len: "..tostring(string.len(encrypted)))
    srv_sck:send(encrypted,srv_close)

  return encrypted
end
