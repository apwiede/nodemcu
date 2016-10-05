function checkOK(result, fieldName)
  if (result ~= "OK") then
    print(tostring(result).." for field: "..fieldName)
  end
end

function writeIADescs()
  local handleIA=structmsg2.createDispatcher();
  result=structmsg2.openFile(handleIA,"DescIA.txt","w")
  checkOK(result,"open")  
  result=structmsg2.writeLine(handleIA, "#,8\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleIA, "@dst,uint16_t,2\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleIA,"@src,uint16_t,2\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleIA,"@totalLgth,uint16_t,2\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleIA,"@cmdKey,uint8_t,1\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleIA,"GUID,uint8_t*,16\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleIA,"PasswdC,uint8_t*,16\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleIA,"Reserve,uint8_t*,2\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleIA,"@crc,uint8_t,1\n")
  checkOK(result,"write")    
  result=structmsg2.closeFile(handleIA)
  checkOK(result,"close") 
end

McuPart=tonumber("4D00",16)
WifiPart=tonumber("5700",16)
GUID="1234-5678-9012-1"
PasswdC="apwiede1apwiede2"


function writeIAVals()
  local handleIA=structmsg2.createDispatcher();
  result=structmsg2.openFile(handleIA,"ValIA.txt","w")
  checkOK(result,"open")  
  result=structmsg2.writeLine(handleIA, "#,5\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleIA, "@dst,"..McuPart.."\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleIA,"@src,"..WifiPart.."\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleIA,"GUID,"..GUID.."\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleIA,"PasswdC,"..PasswdC.."\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleIA,"Reserve,XY\n")
  checkOK(result,"write")    
  result=structmsg2.closeFile(handleIA)
  checkOK(result,"close") 
end

writeIADescs()
writeIAVals()
print("done")
