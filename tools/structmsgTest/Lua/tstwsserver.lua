dofile("myConfig.lua")
dofile("stmsgtst.lua")

function listenConnected(sck)
  print("==listen is connected")
  sck:on("receive",function(sck,payload) 
    print("==received len: "..tostring(string.len(payload)))
    pwdStmsg=decryptPwdStructMsg(payload)
    structmsg.dump(pwdStmsg)
print("decrypted pwd: "..tostring(structmsg.getFieldValue(pwdStmsg,"pwd")))
  end)
  sck:on("sent",function(sck)
    print("==is sent")
  end)
  sck:on("connection",function(sck)
    print("==is connection")
  end)
end

function startTheWebsocket()
  function srvConnected(srv)
    print("==srv is connected")
    srv:listen(port,listenConnected)
  end

  if (srv ~= nil) then
     srv:close()
  end
  srv=nil
  wifi.setmode(wifi.STATION)
  wifi.sta.config(router,passwd)
  wifi.sta.connect()
  tmr.alarm(0, 1000, tmr.ALARM_AUTO, function ()
    local ip = wifi.sta.getip()
    if ip then
      tmr.stop(0)
      print(ip)
      srv=websocket.createServer(30,"/echo",srvConnected)
    else
      print(ip)
      print(node.heap())
    end
  end)
end

