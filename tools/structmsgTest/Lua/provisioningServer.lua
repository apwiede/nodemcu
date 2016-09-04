accessPointData=""
useBig=false

dofile("buildStmsgAPList.lua")

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
    numRows=numRows+1
  end 
print("numRows: "..tostring(numRows))
  return numRows
end

-- (SSID : Authmode, RSSI, BSSID, Channel)
function buildAPList(t)
  numRows=getNumRows(t)
  handle=buildStmsgAPList(t,useBig,numRows)
--  sendAPList(useBig,numRows)
end

function chkRouterConnected(sck)
  dbgPrint("is connected\r\n")
  isConnected=true
end

function ProvisioningServerStart(big)
  useBig=big

  wifi.sta.disconnect()
  wifi.setmode(wifi.STATIONAP)
  wifi.ap.config({ssid="SPIRIT21_Connect",auth=wifi.AUTH_OPEN})

  if (not tmr.alarm(1,1000,tmr.ALARM_AUTO,function()
     ip=wifi.ap.getip()
     if (ip ~= nil) then
       tmr.stop(1)
dbgPrint('Provisioning: wifi mode: '..tostring(wifi.getmode()))
dbgPrint("Provisioning: AP IP: "..tostring(wifi.ap.getip()))
dbgPrint("Provisioning: STA IP: "..tostring(wifi.sta.getip()))
--       wifi.sta.getap(listap)
       wifi.sta.getap(buildAPList)
     else
dbgPrint("IP: "..tostring(ip))
     end
    end)) then
    dbgPrint("could not start timer for getap")
  end

end
