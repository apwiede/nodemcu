accessPointData=""
useBig=false

dofile("myConfig.lua")
dofile("buildStmsgAPList.lua")
dofile("buildStmsgAPListDefinition.lua")

function dbgPrint(s)
  print(s)
end

-- Print AP list that is easier to read
function listap(t) -- (SSID : Authmode, RSSI, BSSID, Channel)
dbgPrint(string.format("%32s","SSID").."\tBSSID\t\t\t\t  RSSI\t\tAUTHMODE\tCHANNEL")
--accessPointData="SSID\tBSSID\tRSSI\tAUTHMODE\tCHANNEL"
--sendAck("PROV","P","AA")  
    for ssid,v in pairs(t) do
        local authmode, rssi, bssid, channel = string.match(v, "([^,]+),([^,]+),([^,]+),([^,]+)")
        dbgPrint(string.format("%32s",ssid).."\t"..bssid.."\t  "..rssi.."\t\t"..authmode.."\t\t\t"..channel)
--accessPointData=ssid.."\t"..bssid.."\t"..rssi.."\t"..authmode.."\t"..channel
--sendAck("MCU","P","AA")  
    end
end

function getNumRows(t)
  local numRows=0
  for ssid,v in pairs(t) do
print("t: "..tostring(ssid).." "..tostring(v))
    numRows=numRows+1
  end 
print("numRows: "..tostring(numRows))
  return numRows
end

--srv_sck=nil


junk=5116
offset=0
cnt=1

function part2()
if (cnt > 10) then
  return
end
print("part2")
  if (offset == 0) then
    return
  end
  len=string.len(encryptedDef)
  if (offset+junk > len) then
    endOffset = len
  else
    endOffset = offset+junk-1
  end
  if (endOffset == len) then
    func=srv_close
  else
    func=nil
  end
print("offset: "..tostring(offset).." endOffset: "..tostring(endOffset))

  srv_sck:send(string.sub(encryptedDef,offset,endOffset),2,func)
  offset=endOffset+1
  if (endOffset == len) then
    offset=0
  end
  cnt=cnt+1
end

numMsg=0
function srv_listen(sck)
print("srv_listen")
  srv_sck=sck
  sck:on("receive",function(sck,payload)
    print("receive: ".."!"..tostring(payload).."!")
    srv_sck=sck
    if (payload == "getaplist\r\n") then
print("++getaplist")
      wifi.sta.getap(buildAPList)
      return
    end
    if (tostring(payload) == "getapdeflist\r\n") then
--      srv_sck:send("Hello World",1)
--      return
      if (numMsg == 0) then
print("++getapdeflist")
        numRows=2
        encryptedDef=buildStmsgAPDefList(t,useBig,numRows)
        numMsg = numMsg+1
print("encryptedDef: "..tostring(string.len(encryptedDef)))
      else
print("++getaplist")
        wifi.sta.getap(buildAPList)
      end
--      encryptedDef=crypto.encrypt("AES-CBC", cryptkey, "Hello world 1234",cryptkey)
--      srv_sck:send("HELLO: "..encryptedDef,1)
      offset=1
      srv_sck:send("",2,part2)
      return
    end
    srv_sck:send("bad request",srv_close)
  end)
--      numRows=2
--      encryptedDef=buildStmsgAPDefList(t,useBig,numRows)
--print("encryptedDef: "..tostring(string.len(encryptedDef)))
--      srv_sck:send(encryptedDef,srv_close)
  
end

function srv_connected(sck)
print("srv_connected")
  sck:listen(port,srv_listen)
end

function srv_close(sck)
  sck:close()
end

function buildAPList(t)
  numRows=getNumRows(t)
  encrypted=buildStmsgAPList(t,useBig,numRows)
  srv_sck:send(encrypted,srv_close)
end

function chkRouterConnected(sck)
  dbgPrint("is connected\r\n")
  isConnected=true
end

function ProvisioningServerStart(big)
  useBig=big

  if srv ~= nil then
    srv:close()
    srv=nil
  end
  wifi.sta.disconnect()
  wifi.setmode(wifi.STATIONAP)
  wifi.ap.config({ssid="testDevice_connect",auth=wifi.AUTH_OPEN})

  if (not tmr.alarm(1,1000,tmr.ALARM_AUTO,function()
     ip=wifi.ap.getip()
     if (ip ~= nil) then
       tmr.stop(1)
dbgPrint('Provisioning: wifi mode: '..tostring(wifi.getmode()))
dbgPrint("Provisioning: AP IP: "..tostring(wifi.ap.getip()))
dbgPrint("Provisioning: STA IP: "..tostring(wifi.sta.getip()))
--       wifi.sta.getap(listap)
       srv=websocket.createServer(30,"/getapdeflist",srv_connected)
     else
dbgPrint("IP: "..tostring(ip))
     end
    end)) then
    dbgPrint("could not start timer for getap")
  end

end
