res=handleIMsg(IMsg)
function createIAckMsg()
  return structmsg2.createMsgFromFile("DescIA.txt",0,0);
end

function setIAFieldValues(handleIAck,GUID,PasswdC)
  local valueInfos={}
  valueInfos["1"]="@dst,"..Mcu..",0"
  valueInfos["2"]="@src,"..Wifi..",0"
--  valueInfos["3"]="@cmdKey,73,0"
  valueInfos["3"]="@cmdKey,"..xcmdI..",0"
  valueInfos["4"]="GUID,"..GUID..",0"
  valueInfos["5"]="PasswdC,"..PasswdC..",0"
  valueInfos["6"]="Reserve,XY,0"
  flags=0
  result=structmsg2.setMsgFieldsFromList(handleIAck,valueInfos,flags)
  checkOK(result,"IAck setMsgFieldsFromList")
end

function createINMsg()
  local msgInfos={}
  msgInfos["1"]="@dst,uint16_t,2,0"
  msgInfos["2"]="@src,uint16_t,2,0"
  msgInfos["3"]="@totalLgth,uint16_t,2,0"
  msgInfos["4"]="@cmdKey,uint8_t,1,0"
  msgInfos["5"]="Nak,uint8_t,1,0"
  msgInfos["6"]="@crc,uint8_t,1,0"
  flags=0
  handleINak=structmsg2.createMsgFromList(msgInfos,0,flags);
  return handleINak
end

function setINFieldValues(handleINak)
  local valueInfos={}
  valueInfos["1"]="@dst,"..Mcu..",0"
  valueInfos["2"]="@src,"..Wifi..",0"
  valueInfos["3"]="@cmdKey,"..cmdI..",0"
  valueInfos["4"]="Nak,"..INak..",0"
  flags=0
  result=structmsg2.setMsgFieldsFromList(handleINak,valueInfos,flags)
  checkOK(result,"INak setMsgFieldsFromList")
end

function sendIAMsg(GUID,passwdC)
  local handleIA=createIAckMsg()
  setIAFieldValues(handleIA,GUID,PasswdC)
  structmsg2.prepare(handleIA)
--structmsg2.dump(handleIA)
  local IAMsg=structmsg2.getMsgData(handleIA)
  uart.write(0,IAMsg)
--print(tostring(IAMsg))
  structmsg2.delete(handleIA)
end

function sendINMsg()
  local handleIN=createINakMsg()
  setINFieldValues(handleIN)
  structmsg2.prepare(handleIN)
--structmsg2.dump(handleIN)
  local INMsg=structmsg2.getMsgData(handleIN)
  uart.write(0,INMsg)
end
