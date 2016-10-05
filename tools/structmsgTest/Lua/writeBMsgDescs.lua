function checkOK(result, fieldName)
  if (result ~= "OK") then
    print(tostring(result).." for field: "..fieldName)
  end
end

function writeBADescs()
  local handleBA=structmsg2.createDispatcher();
  result=structmsg2.openFile(handleBA,"DescBA.txt","w")
  checkOK(result,"open")  
  result=structmsg2.writeLine(handleBA, "#,6\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleBA, "@dst,uint16_t,2\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleBA,"@src,uint16_t,2\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleBA,"@totalLgth,uint16_t,2\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleBA,"@cmdKey,uint8_t,1\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleBA,"opMode,uint8_t,1\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleBA,"@crc,uint8_t,1\n")
  checkOK(result,"write")    
  result=structmsg2.closeFile(handleBA)
  checkOK(result,"close") 
end

McuPart=tonumber("4D00",16)
WifiPart=tonumber("5700",16)
opMode=tonumber("41",16)


function writeBAVals()
  local handleBA=structmsg2.createDispatcher();
  result=structmsg2.openFile(handleBA,"ValBA.txt","w")
  checkOK(result,"open")  
  result=structmsg2.writeLine(handleBA, "#,3\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleBA, "@dst,"..McuPart.."\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleBA,"@src,"..WifiPart.."\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleBA,"opMode,"..opMode.."\n")
  checkOK(result,"write")    
  result=structmsg2.closeFile(handleBA)
  checkOK(result,"close") 
end

writeBADescs()
writeBAVals()
print("done")
