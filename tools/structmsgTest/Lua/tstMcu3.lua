function initVars()
Mcu=tonumber("4D00",16)
Wifi=tonumber("5700",16)
GUID="1234-5678-4321-1"
PasswdC="passwd1passwd2abcd"
xcmdI=string.byte("I")
--cmdM=string.byte("M")
--cmdB=string.byte("B")
--INak=string.byte("N")
varI=nil
varM=nil
--cmd_B=tonumber("42",16)
--I_Nak=tonumber("4E",16)
end

function sendMsg(type,subType)
  if (type == "I") then
    dofile("tstIMsg.lua")
    if (subType == "A") then
      sendIAMsg(GUID,PasswdC)
    else
      sendINMsg()
    end
  end
  if (type == "M") then
    dofile("tstMMsg.lua")
    if (subType == "A") then
      sendMAMsg()
    else
      sendMNMsg()
    end
  end
  if (type == "B") then
    dofile("tstBMsg.lua")
    if (subType == "A") then
      sendBAMsg()
    else
      sendBNMsg()
    end
  end
end


function checkOK(result, fieldName)
  if (result ~= "OK") then
    print(tostring(result).." for field: "..fieldName)
  end
end

--function run()
--echo=0
--uart.setup(0, 115200, 8, uart.PARITY_NONE, uart.STOPBITS_1, echo)
--uart.on("data", 0,
--  function(data)
--    uart.write(0,encoded)
--    uart.on("data")
--  end
-- , 0)
--end

initVars()
sendMsg("I","A")
print("done")
