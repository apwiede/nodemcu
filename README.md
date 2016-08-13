# nodemcu

## modules for nodemcu firmware

### additional modules for use within nodemcu-firmware

check out from the nodemcu-firmware repository and add to directory app/modules:

> app/modules/websocket.c  a Websocket server and client very similar to the net module.
                           mostly the same interface, but for use of websockets

> app/modules/structmsg.c  a module for handling and building "structured messages" (structmsg)
                           The message layout can be designed with Lua calls and then a packed message string
                           can be generated and also be unpacked 
                           and eventually be encrypted/decrypted using AES-CBC crypto module
 app/structmsg/structmsg.c
 app/structmsg/structmsg.h

You have to add structmsg to the SUBDIRS variable in app/Makefile

You have to add WEBSOCKET and STRUCTMSG defines in app/include/user_modules.h 
for making the modules active.

## Attention!! This is work in progress and not yet usefull for production!!

## websocket: tested with a Tcl websocket server/client

### short example Lua code:

### client:

> con=nil <br />
  function startTheWsClient() <br />

>   function connection(srv) <br />
    con=srv <br />
    print("==connection ready") <br />
    srv:on("receive", function(sck, c) <br />
      print("clnt receive: "..c) <br />
    end) <br />
    srv:on("sent", function(sck) <br />
      print("==clnt sent") <br />
    end) <br />
    srv:on("reconnection", function(sck) <br />
      print("==clnt reconnection") <br />
    end) <br />
    srv:on("disconnection", function(sck,c) <br />
      print("==clnt disconnection") <br />
    end) <br />
    srv:on("connection", function(sck,c) <br />
      print("==connection done") <br />
      sck:send("Hello Wordl\r\n") <br />
    end) <br />
    srv:send("GET /echo HTTP/1.1\ <br />
Host: example.com:8000\ <br />
Upgrade: websocket\ <br />
Connection: Upgrade\ <br />
Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\ <br />
Sec-WebSocket-Version: 13\ <br />
\ <br />
") <br />
    tmr.alarm(0,2000,tmr.ALARM_SINGLE,function(srv) <br />
       con:send("Hello World Again\r\n") <br />
    end) <br />
  end <br />

>   function clntConnected(clnt) <br />
    print("==client is connected") <br />
    clnt:connect(8080,"192.168.4.1",connection) <br />
  end <br />

>   if (clnt ~= nil) then <br />
     clnt:close() <br />
  end <br />
  clnt=nil <br />
  wifi.setmode(wifi.STATION) <br />
  wifi.sta.config("your router","your passwd") <br />
  wifi.sta.connect() <br />
  tmr.alarm(0, 1000, tmr.ALARM_AUTO, function () <br />
    local ip = wifi.sta.getip() <br />
    if ip then <br />
      tmr.stop(0) <br />
      print(ip) <br />
      clnt=websocket.createConnection(0,clntConnected) <br />
    end <br />
  end) <br />
end <br />

### server:

> port=8080 <br />
  useLineEnd=true <br />

> host="192.168.4.1" <br />
 router="your router" <br />
 passwd="your passwd" <br />
 isConnected=false <br />

> function chkConnected(sck) <br />
  dbgPrint("is connected\r\n") <br />
  isConnected=true <br />
  srv:send("GET /echo HTTP/1.1\ <br />
Host: example.com:8000\ <br />
Upgrade: websocket\ <br />
Connection: Upgrade\ <br />
Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\ <br />
Sec-WebSocket-Version: 13\ <br />
\ <br />
") <br />
end <br />

>function WSSServerStart() <br />
 wifi.setmode(wifi.STATION) <br />
 wifi.sta.config(router,passwd) <br />
  if (srv ~= nil) then <br />
    srv:close() <br />
    srv = nil <br />
  end <br />

>  wifi.sta.autoconnect(1) <br />
  srv = net.createConnection(net.TCP, 0) <br />

>  srv:on("receive", function(sck, c) <br />
    srv:send("Hello World") <br />
  end) <br />

> srv:on("reconnection", function(sck, c) <br />
    print("reconnected") <br />
  end) <br />

>  srv:on("disconnection", function(sck, c) <br />
    print("disconnected: "..tostring(sck)) <br />
  end) <br />

>  srv:on("sent", function(sck, c) <br />
    print("sent") <br />
  end) <br />

>  srv:on("connection", function(sck, c) <br />
    print("connected") <br />
  -- Wait for connection before sending. <br />
    srv:send("Hello World Again"); <br />
  end) <br />

>  tmr.alarm(0, 1000, tmr.ALARM_AUTO, function() <br />
    ip=wifi.sta.getip() <br />
    print("ip: "..tostring(ip)) <br />
    if (ip ~= nil) then <br />
      tmr.stop(0) <br />
      srv:connect(port,host,chkConnected) <br />
    end <br />
  end) <br />

> end <br />


### structmsg message format:

uint16_t | uint16_t | uint16_t  | uint16_t | uint16_t | < uint32_t   | uint8_t\* | uint8_t\* | ... > | uint8_t\* | uint16_t
  src    |   dst    | totalLgth |    cmd   | cmdLgth  | < randomNNum |   fld1   |   fld2   | ... > | filler   |   crc

### short example Lua code:

> function tststructmsg () <br />
    sms=nil <br />
    sms=structmsg.create(5); <br />
    structmsg.addField(sms, "@randomNum", "uint32_t"); <br />
    structmsg.addField(sms, "pwd", "uint8_t\*", 16); <br />
    structmsg.setFillerAndCrc(sms); <br />
    structmsg.setTargets(sms, 123, 456, 789); <br />
    structmsg.setFieldValue(sms, "pwd","/dir1/dir2/dir345"); <br />
    structmsg.encode(sms); <br />
    encoded=structmsg.getencoded(sms); <br />
    sms2=structmsg.create(5); <br />
    structmsg.setTargets(sms2, 789, 456, 123); <br />
    structmsg.addField(sms2, "@randomNum", "uint32_t"); <br />
    structmsg.addField(sms2, "pwd", "uint8_t\*", 16); <br />
    structmsg.setFillerAndCrc(sms2); <br />
    structmsg.decode(sms2,encoded); <br />
    structmsg.encode(sms2); <br />
    encoded2=structmsg.getencoded(sms2); <br />
    ch1=string.byte(encoded2,7); <br />
    ch2=string.byte(encoded2,8); <br />
    ch3=string.byte(encoded2,37); <br />
    ch4=string.byte(encoded2,38); <br />
    print("==enc: "..string.format("0x%02x 0x%02x 0x%02x 0x%02x",ch1,ch2,ch3,ch4)); <br />
    encrypted=structmsg.encrypt(sms2,"a1b2c3d4e5f6g7h8"); <br />
    decrypted=structmsg.decrypt(sms2,"a1b2c3d4e5f6g7h8"); <br />
    ch1=string.byte(decrypted,7); <br />
    ch2=string.byte(decrypted,8); <br />
    ch3=string.byte(decrypted,37); <br />
    ch4=string.byte(decrypted,38); <br />
    print("==dec: "..string.format("0x%02x 0x%02x 0x%02x 0x%02x",ch1,ch2,ch3,ch4)); <br />
    structmsg.delete(sms); <br />
    sms=nil; <br />
    structmsg.delete(sms2); <br />
    sms2=nil; <br />
    encrypted=nil; <br />
    decrypted=nil; <br />
    encoded=nil; <br />
    encoded2=nil; <br />
  end <br />

### More description of the interfaces following.
