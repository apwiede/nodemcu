dofile("myConfig.lua")
dofile("stmsgtst.lua")

con=nil
path="/echo"
function startTheWsClient()
  
  function connection(srv)
    con=srv
    print("==connection ready")
    srv:on("receive", function(sck, c)
      print("==clnt receive: "..c)
      handle=buildPwdStructMsg("/dir1/dir2/dir34")
print("pwd: "..tostring(structmsg.getFieldValue(handle,"pwd")))
      sck:send(encrypted, 2,function(sck)
        sck:close()
      end)
    end)
    srv:on("sent", function(sck)
      print("==clnt sent")
    end)
    srv:on("reconnection", function(sck)
      print("==clnt reconnection")
    end)
    srv:on("disconnection", function(sck,c)
      print("==clnt disconnection")
    end)
    srv:on("connection", function(sck,c)
print("==connection done")
sck:send("Hello Arnulf\r\n")

    end)
    srv:send("GET "..path.." HTTP/1.1\
Host: example.com:8000\
Upgrade: websocket\
Connection: Upgrade\
Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\
Sec-WebSocket-Version: 13\
\
")
--    tmr.alarm(0,2000,tmr.ALARM_SINGLE,function(srv)
--       con:send("HELLO World\r\n")
--    end)
  end

  function clntConnected(clnt)
    print("==client is connected")
    clnt:connect(port,host,connection)
  end

  if (clnt ~= nil) then
     clnt:close()
  end
  clnt=nil
  wifi.setmode(wifi.STATION)
  wifi.sta.config(router,passwd)
  wifi.sta.connect()
  tmr.alarm(0, 1000, tmr.ALARM_AUTO, function ()
    local ip = wifi.sta.getip()
    if ip then
      tmr.stop(0)
      print(ip)
      clnt=websocket.createConnection(0,clntConnected)
    else 
      print(ip)
      print(node.heap())
    end
  end)
end
