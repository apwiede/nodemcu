function checkOK(result, fieldName)
  if (result ~= "OK") then
    print(tostring(result).." for field: "..fieldName)
  end
end

function createMcuMsg()
  local msgInfos={}

  msgInfos["1"]="GUID,uint8_t*,16,0"
  msgInfos["2"]="PasswdC,uint8_t*,16,0"
  msgInfos["3"]="Reserve,uint8_t*,2"
  msgInfos["4"]="@crc,uint8_t,1,0"
  numEntries=0
  for x,v in pairs(msgInfos) do
    numEntries=numEntries+1
  end
--  print("numEntries: "..tostring(numEntries))
  flags=0
  shortCmdKey=1
  handle=structmsg.createmsgfromlist(numEntries,msgInfos,0,flags,shortCmdKey);
--print("handlemsg: "..tostring(handle))
  return handle
end

function setFieldValues(handle)
  result=structmsg.setFieldValue(handle, "@src",0x4D00)
  checkOK(result,"@src")
  result=structmsg.setFieldValue(handle, "@dst",0x5700)
  checkOK(result,"@dst")
  result=structmsg.setFieldValue(handle, "@cmdKey",73)
  checkOK(result,"@cmdKey")
  result=structmsg.setFieldValue(handle, "GUID","1234-5678-4321-1")
  checkOK(result,"GUID")
  result=structmsg.setFieldValue(handle, "PasswdC","passwd1passwd2abcd")
  checkOK(result,"PasswdC")
  result=structmsg.setFieldValue(handle, "Reserve","XY")
end

function buildMcuStartMsg()
  handle=createMcuMsg()
  setFieldValues(handle)
--structmsg.dump(handle)

  result=structmsg.encode(handle)
  checkOK(result,"encode")
  encoded=structmsg.getencoded(handle)
--print("encoded")
  return encoded
end

encoded=buildMcuStartMsg()
echo=0
uart.setup(0, 115200, 8, uart.PARITY_NONE, uart.STOPBITS_1, echo)
uart.on("data", 0,
  function(data)
--    print("receive from uart:", data)
--print("3")
--    uart.write(0,"hello world\n")
    uart.write(0,encoded)
    uart.on("data")
--print("4")
  end
 , 0)

