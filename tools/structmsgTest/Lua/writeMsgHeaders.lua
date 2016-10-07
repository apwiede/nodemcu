function checkOK(result, fieldName)
  if (result ~= "OK") then
    print(tostring(result).." for field: "..fieldName)
  end
end

McuPartProv=tonumber("4D01",16);
AppPartProv=tonumber("4101",16);
McuPart=tonumber("4D00",16);
AppPart=tonumber("4100",16);
WifiPart=tonumber("5700",16)
CloudPart=tonumber("4300",16)

function writeMsgHeads()
  local handleBA=structmsg2.createDispatcher();
  result=structmsg2.openFile(handleBA,"MsgHeads.txt","w")
  checkOK(result,"open")  
  result=structmsg2.writeLine(handleBA, "#,39\n")
  checkOK(result,"write")    
-- description of the first header bytes
-- either "@targetCmd,@totalLgth\n"
-- or "@dst,@src,@totalLgth\n"
-- or similar
-- the following has that format:
-- extraBytes, encrypted or not, transfer type, type of cmdKey, cmdKey, type of cmdLgth, cmdLgth
-- E/N encrypted/not encrypted, 
-- A/G/S/R/U/W/N send to app/receive from app/send Uart/receive Uart/transfer Uart/transfer Wifi/not relevant
-- example: 0/16, E/N, A/G/S/R/U/W/N, uint16_t/uint8_t, PP/B, uint0_t/uint8_t/uint16_t, /12/345
  result=structmsg2.writeLine(handleBA, "@dst,@src,@totalLgth\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleBA, McuPartProv..","..AppPartProv..",37,0,E,W,uint16_t,PP,uint0_t,uint8_t\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleBA, AppPartProv..","..McuPartProv..",21,0,E,U,uint16_t,PA,uint0_t,uint8_t\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleBA, AppPartProv..","..McuPartProv..",21,0,E,U,uint16_t,PN,uint0_t,uint8_t\n")
  checkOK(result,"write")    

  result=structmsg2.writeLine(handleBA, McuPart..","..AppPart..",21,0,E,W,uint16_t,PM,uint0_t,uint8_t\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleBA, AppPart..","..McuPart..",37,0,E,U,uint16_t,PA,uint0_t,uint8_t\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleBA, AppPart..","..McuPart..",21,0,E,U,uint16_t,PN,uint0_t,uint8_t\n")
  checkOK(result,"write")    

  result=structmsg2.writeLine(handleBA, WifiPart..","..AppPart..",0,0,E,G,uint16_t,AD,uint0_t,uint8_t\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleBA, AppPart..","..WifiPart..",0,0,E,A,uint16_t,AA,uint0_t,uint8_t\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleBA, AppPart..","..WifiPart..",0,0,E,A,uint16_t,AN,uint0_t,uint8_t\n")
  checkOK(result,"write")    

  result=structmsg2.writeLine(handleBA, WifiPart..","..AppPart..",0,0,E,G,uint16_t,SP,uint0_t,uint8_t\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleBA, AppPart..","..WifiPart..",0,0,E,A,uint16_t,SA,uint0_t,uint8_t\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleBA, AppPart..","..WifiPart..",0,0,E,A,uint16_t,SN,uint0_t,uint8_t\n")
  checkOK(result,"write")    

  result=structmsg2.writeLine(handleBA, AppPart..","..WifiPart..",0,0,E,A,uint16_t,AI,uint0_t,uint8_t\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleBA, WifiPart..","..AppPart..",0,0,E,G,uint16_t,AA,uint0_t,uint8_t\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleBA, WifiPart..","..AppPart..",0,0,E,G,uint16_t,AN,uint0_t,uint8_t\n")
  checkOK(result,"write")    

  result=structmsg2.writeLine(handleBA, McuPart..","..AppPart..",37,0,E,U,uint16_t,PD,uint0_t,uint8_t\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleBA, AppPart..","..McuPart..",21,0,E,W,uint16_t,PA,uint0_t,uint8_t\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleBA, AppPart..","..McuPart..",21,0,E,W,uint16_t,PN,uint0_t,uint8_t\n")
  checkOK(result,"write")    

  result=structmsg2.writeLine(handleBA, McuPart..","..AppPart..",37,0,E,U,uint16_t,PE,uint0_t,uint8_t\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleBA, AppPart..","..McuPart..",21,0,E,W,uint16_t,PA,uint0_t,uint8_t\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleBA, AppPart..","..McuPart..",21,0,E,W,uint16_t,PN,uint0_t,uint8_t\n")
  checkOK(result,"write")    

  result=structmsg2.writeLine(handleBA, McuPart..","..CloudPart..",0,16,E,U,uint16_t,CD,uint0_t,uint8_t\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleBA, CloudPart..","..McuPart..",21,16,E,W,uint16_t,CA,uint0_t,uint8_t\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleBA, CloudPart..","..McuPart..",21,16,E,W,uint16_t,CN,uint0_t,uint8_t\n")
  checkOK(result,"write")    

  result=structmsg2.writeLine(handleBA, WifiPart..","..McuPart..",8,0,N,R,uint8_t,I,uint0_t,uint8_t\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleBA, McuPart..","..WifiPart..",42,0,N,S,uint8_t,I,uint0_t,uint8_t\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleBA, McuPart..","..WifiPart..",9,0,N,S,uint8_t,I,uint0_t,uint8_t\n")
  checkOK(result,"write")    

  result=structmsg2.writeLine(handleBA, WifiPart..","..McuPart..",8,0,N,R,uint8_t,M,uint0_t,uint8_t\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleBA, McuPart..","..WifiPart..",44,0,N,S,uint8_t,M,uint0_t,uint8_t\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleBA, McuPart..","..WifiPart..",9,0,N,S,uint8_t,M,uint0_t,uint8_t\n")
  checkOK(result,"write")    

  result=structmsg2.writeLine(handleBA, WifiPart..","..McuPart..",10,0,N,R,uint8_t,B,uint0_t,uint8_t\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleBA, McuPart..","..WifiPart..",9,0,N,S,uint8_t,B,uint0_t,uint8_t\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleBA, McuPart..","..WifiPart..",9,0,N,S,uint8_t,B,uint0_t,uint8_t\n")
  checkOK(result,"write")    

  result=structmsg2.writeLine(handleBA, WifiPart..","..McuPart..",10,0,N,R,uint8_t,W,uint0_t,uint8_t\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleBA, McuPart..","..WifiPart..",9,0,N,S,uint8_t,W,uint0_t,uint8_t\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleBA, McuPart..","..WifiPart..",9,0,N,S,uint8_t,W,uint0_t,uint8_t\n")
  checkOK(result,"write")    

  result=structmsg2.writeLine(handleBA, WifiPart..","..McuPart..",64,0,N,R,uint8_t,T,uint0_t,uint8_t\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleBA, McuPart..","..WifiPart..",9,0,N,S,uint8_t,T,uint0_t,uint8_t\n")
  checkOK(result,"write")    
  result=structmsg2.writeLine(handleBA, McuPart..","..WifiPart..",9,0,N,S,uint8_t,T,uint0_t,uint8_t\n")
  checkOK(result,"write")    
  result=structmsg2.closeFile(handleBA)
  checkOK(result,"close") 
end

writeMsgHeads()
print("done")
