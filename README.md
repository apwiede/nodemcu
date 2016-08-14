# nodemcu

## modules for nodemcu firmware

### additional modules for use within nodemcu-firmware

check out from the nodemcu-firmware repository and add to directory app/modules:

- app/modules/websocket.c  a Websocket server and client very similar to the net module.
                           mostly the same interface, but for use of websockets

- app/modules/structmsg.c  a module for handling and building "structured messages" (structmsg)
                           The message layout can be designed with Lua calls and then a packed message string
                           can be generated and also be unpacked 
                           and eventually be encrypted/decrypted using AES-CBC crypto module
- app/structmsg/structmsg.c
- app/structmsg/structmsg.h

You have to add structmsg to the SUBDIRS variable in app/Makefile

You have to add WEBSOCKET and STRUCTMSG defines in app/include/user_modules.h 
for making the modules active.

## Attention!! This is work in progress and not yet usefull for production!!

## Useful links

| Resource | Location |
| -------------- | -------------- |
| Developer Wiki       | https://github.com/nodemcu/nodemcu-firmware/wiki |

## websocket: tested with a Tcl websocket server/client

### short example Lua code:

### client:

```lua
    con=nil
    router="your router"
    passwd="your password"

    function startTheWsClient()

      function connection(srv)
        con=srv
        print("==connection ready")
        srv:on("receive", function(sck, c)
          print("clnt receive: "..c)
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
          sck:send("Hello Wordl\r\n")
        end)
        srv:send("GET /echo HTTP/1.1\
Host: example.com:8000\
Upgrade: websocket\
Connection: Upgrade\
Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\
Sec-WebSocket-Version: 13\
\
")
        tmr.alarm(0,2000,tmr.ALARM_SINGLE,function(srv)
           con:send("Hello World Again\r\n")
        end)
      end

      function clntConnected(clnt)
        print("==client is connected")
        clnt:connect(8080,"192.168.4.10",connection)
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
        end
      end)
    end
```

### server:

```lua
  port=8080
  host="192.168.4.1"
  router="your router"
  passwd="your passwd"
  isConnected=false

  function chkConnected(sck)
    print("is connected\r\n")
    isConnected=true
    srv:send("GET /echo HTTP/1.1\
Host: example.com:8000\
Upgrade: websocket\
Connection: Upgrade\
Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\
Sec-WebSocket-Version: 13\
\
")
  end

  function WSSServerStart()
    wifi.setmode(wifi.STATION)
    wifi.sta.config(router,passwd)
    if (srv ~= nil) then
      srv:close()
      srv = nil
    end

    wifi.sta.autoconnect(1)
    srv = net.createConnection(net.TCP, 0)

    srv:on("receive", function(sck, c)
      srv:send("Hello World")
    end)

   srv:on("reconnection", function(sck, c)
      print("reconnected")
    end)

    srv:on("disconnection", function(sck, c)
      print("disconnected: "..tostring(sck))
    end)

    srv:on("sent", function(sck, c)
      print("sent")
    end)

    srv:on("connection", function(sck, c)
      print("connected")
  -- Wait for connection before sending.
      srv:send("Hello World Again");
    end)

    tmr.alarm(0, 1000, tmr.ALARM_AUTO, function()
      ip=wifi.sta.getip()
      print("ip: "..tostring(ip))
      if (ip ~= nil) then
        tmr.stop(0)
        srv:connect(port,host,chkConnected)
      end
    end)

   end
```


### structmsg message format:

| uint16\_t | uint16\_t | uint16\_t  | uint16\_t | uint16\_t | \< uint32\_t   | uint8\_t\* | uint8\_t\* | ... \> | uint8\_t\* | uint16\_t |
| ---------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- |
|   src    |   dst    | totalLgth |    cmd   | cmdLgth  | < randomNum |   fld1    |   fld2    | ... > | filler   |   crc     |

### short example Lua code:

```lua
 function tststructmsg ()
    sms=nil
    sms=structmsg.create(5);
    structmsg.addField(sms, "@randomNum", "uint32_t");
    structmsg.addField(sms, "pwd", "uint8_t*", 16);
    structmsg.setFillerAndCrc(sms);
    --                        src  dst  cmd
    structmsg.setTargets(sms, 123, 456, 789);
    structmsg.setFieldValue(sms, "pwd","/dir1/dir2/dir34");
    structmsg.encode(sms);
    encoded=structmsg.getencoded(sms);
    sms2=structmsg.create(5);
    structmsg.setTargets(sms2, 789, 456, 123);
    --                        @: fields that start with @ are specially handled
    structmsg.addField(sms2, "@randomNum", "uint32_t");
    --                               uint8_t*: the type of the field determines its handling
    structmsg.addField(sms2, "pwd", "uint8_t*", 16);
    structmsg.setFillerAndCrc(sms2);
    structmsg.decode(sms2,encoded);
    structmsg.encode(sms2);
    encoded2=structmsg.getencoded(sms2);
    ch1=string.byte(encoded2,7);
    ch2=string.byte(encoded2,8);
    ch3=string.byte(encoded2,37);
    ch4=string.byte(encoded2,38);
    print("==enc: "..string.format("0x%02x 0x%02x 0x%02x 0x%02x",ch1,ch2,ch3,ch4));
    encrypted=structmsg.encrypt(sms2,"a1b2c3d4e5f6g7h8");
    decrypted=structmsg.decrypt(sms2,"a1b2c3d4e5f6g7h8");
    ch1=string.byte(decrypted,7);
    ch2=string.byte(decrypted,8);
    ch3=string.byte(decrypted,37);
    ch4=string.byte(decrypted,38);
    print("==dec: "..string.format("0x%02x 0x%02x 0x%02x 0x%02x",ch1,ch2,ch3,ch4));
    structmsg.delete(sms);
    sms=nil;
    structmsg.delete(sms2);
    sms2=nil;
    encrypted=nil;
    decrypted=nil;
    encoded=nil;
    encoded2=nil;
  end 
```

### More detailed description of the interfaces following.
