--print("1")
--file.rename("init.lua","init1.lua")
echo=0
dataBits=8
baud=115200
--print("2")
uart.setup(0, baud, dataBits, uart.PARITY_NONE, uart.STOPBITS_1, echo)
eol="\n"
uart.on("data", eol,
  function(data)
    str=0x4d11
    str=str.."Hello World"
    str = str..tonumber("f7",16)
    uart.write(0,str.."\r\n")
--    uart.write(0,"current heap: ")
--    uart.write(0,tostring(node.heap()))
--    uart.write(0,"\r\n")
    tmr.alarm(0,1000,tmr.ALARM_SINGLE,function()
    uart.on("data")
    echo=1
    uart.setup(0, baud, dataBits, uart.PARITY_NONE, uart.STOPBITS_1, echo)
--    uart.write(0,"3\r\n")
    end)
  end
,0)

