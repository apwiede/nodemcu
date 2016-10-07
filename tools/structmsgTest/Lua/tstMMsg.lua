MacAddr=tonumber("ABCD",16)..tonumber("EF12",16)..tonumber("3456")
IPAddr=tonumber("D4C3",16)..tonumber("1234",16)
FirmVers=tonumber("1234",16)..tonumber("56AB",16)..tonumber("CDEF",16)
SerieNum=tonumber("0213",16)..tonumber("2A10",16)
rssi=tonumber("8A",16)
ModConn=tonumber("41",16)
DevMode=tonumber("34",16)
DevSec=tonumber("00",16)
ErrMain=0
ErrSub=0
date=tonumber("0000",16)..tonumber("0000",16)..tonumber("0000",16)
numSsid=2

function createMAckMsg()
  local msgInfos={}
  msgInfos["1"]="@dst,uint16_t,2,0"
  msgInfos["2"]="@src,uint16_t,2,0"
  msgInfos["3"]="@totalLgth,uint16_t,2,0"
  msgInfos["4"]="@cmdKey,uint8_t,1,0"
  msgInfos["5"]="MacAddr,uint8_t*,6,0"
  msgInfos["6"]="IPAddr,uint8_t*,4,0"
  msgInfos["7"]="FirmVers,uint8_t*,6,0"
  msgInfos["8"]="SerieNum,uint8_t*,4,0"
  msgInfos["9"]="rssi,uint8_t,1,0"
  msgInfos["10"]="ModConn,uint8_t,1,0"
  msgInfos["11"]="DevMode,uint8_t,1,0"
  msgInfos["12"]="DevSec,uint8_t,1,0"
  msgInfos["13"]="ErrMain,uint8_t,1,0"
  msgInfos["14"]="ErrSub,uint8_t,1,0"
  msgInfos["15"]="date,uint8_t*,6,0"
  msgInfos["16"]="numSsid,uint8_t,1,0"
  msgInfos["17"]="Reserve,uint8_t*,3,0"
  msgInfos["18"]="@crc,uint8_t,1,0"
  flags=0
  handleMAck=structmsg2.createMsgFromList(msgInfos,0,flags);
  return handleMAck
end

function setMAFieldValues(handleMAck)
  local valueInfos={}
  valueInfos["1"]="@dst,"..Mcu..",0"
  valueInfos["2"]="@src,"..Wifi..",0"
  valueInfos["3"]="@cmdKey,"..cmdM..",0"
  valueInfos["4"]="MacAddr,"..MacAddr..",0"
  valueInfos["5"]="IPAddr,"..IPAddr..",0"
  valueInfos["6"]="FirmVers,"..FirmVers..",0"
  valueInfos["7"]="SerieNum,"..SerieNum..",0"
  valueInfos["8"]="rssi,"..rssi..",0"
  valueInfos["9"]="ModConn,"..modConn..",0"
  valueInfos["10"]="DevMode,"..DevMode..",0"
  valueInfos["11"]="DevSec,"..DevSec..",0"
  valueInfos["12"]="ErrMain,"..ErrMain..",0"
  valueInfos["13"]="ErrSub,"..ErrSub..",0"
  valueInfos["14"]="date,"..date..",0"
  valueInfos["15"]="numSsid,"..numSsid..",0"
  valueInfos["16"]="Reserve,ABC,0"
  flags=0
  result=structmsg2.setMsgFieldsFromList(handleMAck,valueInfos,flags)
  checkOK(result,"setMsgFieldsFromList")
end

function createMNakMsg()
  local msgInfos={}
  msgInfos["1"]="@dst,uint16_t,2,0"
  msgInfos["2"]="@src,uint16_t,2,0"
  msgInfos["3"]="@totalLgth,uint16_t,2,0"
  msgInfos["4"]="@cmdKey,uint8_t,1,0"
  msgInfos["5"]="Nak,uint8_t,1,0"
  msgInfos["6"]="@crc,uint8_t,1,0"
  flags=0
  handleMNak=structmsg2.createMsgFromList(msgInfos,0,flags);
  return handleMNak
end

function setMNFieldValues(handleMNak)
  local valueInfos={}
  valueInfos["1"]="@dst,"..Mcu..",0"
  valueInfos["2"]="@src,"..Wifi..",0"
  valueInfos["3"]="@cmdKey,"..cmdM..",0"
  valueInfos["4"]="Nak,N,0"
  flags=0
  result=structmsg2.setMsgFieldsFromList(handleMNak,valueInfos,flags)
  checkOK(result,"setMsgFieldsFromList")
end

function sendMAMsg()
  local handleMA=createMAckMsg()
  setMAFieldValues(handleMA)
  structmsg2.prepare(handleMA)
--structmsg2.dump(handleMA)
  local MAMsg=structmsg2.getMsgData(handleMA)
  uart.write(0,MAMsg)
end

function sendMNMsg()
  local handleMN=createMNakMsg()
  setMNFieldValues(handleMN)
  structmsg2.prepare(handleMN)
--structmsg2.dump(handleMN)
  local MNMsg=structmsg2.getMsgData(handleMN)
  uart.write(0,MNMsg)
end

