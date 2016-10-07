function checkOK(result, fieldName)
  if (result ~= "OK") then
    print(tostring(result).." for field: "..fieldName)
  end
end
    
    dofile("tstIMsg.lua")
    handleIA=createIAckMsg()
    result=structmsg2.openFile(handleIA,"f1.txt","w")
    checkOK(result,"open")
    result=structmsg2.writeLine(handleIA,"Hello World\n")
    checkOK(result,"write")    
    result=structmsg2.writeLine(handleIA,"Another line is longer\n")
    checkOK(result,"write")    
    result=structmsg2.closeFile(handleIA)
    checkOK(result,"close") 
    print("closed")
    result=structmsg2.openFile(handleIA,"f1.txt","r")
    checkOK(result,"open2")
    print("open2")
    result=structmsg2.readLine(handleIA)
    print("line: "..result)
    result=structmsg2.readLine(handleIA)
    print("line2: "..result)
    result=structmsg2.closeFile(handleIA)
    checkOK(result,"close2") 
    print("close2")
    
    
