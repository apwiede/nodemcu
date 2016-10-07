function createBAckMsg()
  local msgInfos={}
  msgInfos["1"]="@dst,uint16_t,2,0"
  msgInfos["2"]="@src,uint16_t,2,0"
  msgInfos["3"]="@totalLgth,uint16_t,2,0"
  msgInfos["4"]="@cmdKey,uint8_t,1,0"
  msgInfos["5"]="GUID,uint8_t*,16,0"
  msgInfos["6"]="PasswdC,uint8_t*,16,0"
  msgInfos["7"]="Reserve,uint8_t*,2"
  msgInfos["8"]="@crc,uint8_t,1,0"
  flags=0
  handleBAck=structmsg2.createMsgFromList(msgInfos,0,flags);
  return handleBAck
end

function setBAFieldValues(handleBAck)
  local valueInfos={}
  valueInfos["1"]="@dst,"..Mcu..",0"
  valueInfos["2"]="@src,"..Wifi..",0"
  valueInfos["3"]="@cmdKey,"..cmdB..",0"
  valueInfos["4"]="GUID,"..GUID..",0"
  valueInfos["5"]="PasswdC,"..PasswdC..",0"
  valueInfos["6"]="Reserve,XY,0"
  flags=0
  result=structmsg2.setMsgFieldsFromList(handleBAck,valueInfos,flags)
  checkOK(result,"setMsgFieldsFromList")
end

function createBNakMsg()
  local msgInfos={}
  msgInfos["1"]="@dst,uint16_t,2,0"
  msgInfos["2"]="@src,uint16_t,2,0"
  msgInfos["3"]="@totalLgth,uint16_t,2,0"
  msgInfos["4"]="@cmdKey,uint8_t,1,0"
  msgInfos["5"]="Nak,uint8_t,1,0"
  msgInfos["6"]="@crc,uint8_t,1,0"
  flags=0
  handleBNak=structmsg2.createMsgFromList(msgInfos,0,flags);
  return handleBNak
end

function setBNFieldValues(handleBNak)
  local valueInfos={}
  valueInfos["1"]="@dst,"..Mcu..",0"
  valueInfos["2"]="@src,"..Wifi..",0"
  valueInfos["3"]="@cmdKey,"..cmdB..",0"
  valueInfos["4"]="Nak,N,0"
  flags=0
  result=structmsg2.setMsgFieldsFromList(handleBAck,valueInfos,flags)
  checkOK(result,"setMsgFieldsFromList")
end

function sendBAMsg()
  local handleBA=createBAckMsg()
  setBAFieldValues(handleBA)
  structmsg2.prepare(handleBA)
--structmsg2.dump(handleBA)
  local BAMsg=structmsg2.getMsgData(handleBA)
  uart.write(0,BAMsg)
end

function sendBNMsg()
  local handleBN=createBNakMsg()
  setBNFieldValues(handleBN)
  structmsg2.prepare(handleBN)
--structmsg2.dump(handleBN)
  local BNMsg=structmsg2.getMsgData(handleBN)
  uart.write(0,BNMsg)
end

