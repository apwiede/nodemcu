
function createAPListDefMsg(useBig,numRows)
  local msgDefInfos={}
  msgDefInfos["1"]="@src,uint16_t,2,0"
  msgDefInfos["2"]="@dst,uint16_t,2,0"
  msgDefInfos["3"]="@totalLgth,uint16_t,2,0"
  msgDefInfos["4"]="@cmdKey,uint16_t,2,0"
  msgDefInfos["5"]="@cmdLgth,uint16_t,2,0"
  msgDefInfos["6"]="@randomNum,uint32_t,4,0"
  msgDefInfos["7"]="@tablerows,uint8_t,@numRows,0"
  msgDefInfos["8"]="@tablerowfields,uint8_t,5,2"
  msgDefInfos["9"]="@tablerowfields,uint8_t,3,1"
  msgDefInfos["10"]="ssid,uint8_t*,32,0"
  msgDefInfos["11"]="bssid,uint8_t*, 17,2"
  msgDefInfos["12"]="rssi,int8_t,1,0"
  msgDefInfos["13"]="authmode,uint8_t,1,2"
  msgDefInfos["14"]="channel,uint8_t,1,0"
  msgDefInfos["15"]="@filler,uint8_t*,1,0"
  msgDefInfos["16"]="@crc,uint16_t,2,0"
  numEntries=0
  for x,v in pairs(msgDefInfos) do
    numEntries=numEntries+1
  end
  def="aplist"
  if (useBig) then
    flags=1
  else
    flags=0
  end
  result=structmsg.createmsgdeffromlist(numEntries,msgDefInfos,def,numRows,flags);
print("def: "..tostring(def).." "..tostring(result))
  return def
end

function buildStmsgAPDefList(t,useBig,numRows)
  def=createAPListDefMsg(useBig,numRows)
print("def: "..tostring(def))
structmsg.dumpdef(def)
print("after dump")

  result=structmsg.encodedef(def)
  checkOK(result,"defencode")
print("defencoded")
--  encoded_def=structmsg.getencoded(def)
--print("GETencoded len: "..tostring(string.len(encoded_def)))
  encryptedDef=structmsg.encryptdef(def,cryptkey)
print("encryptedDef len: "..tostring(string.len(encryptedDef)))
  return encryptedDef
end
