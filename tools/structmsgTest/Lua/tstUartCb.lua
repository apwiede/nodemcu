--print("heap0: "..node.heap())
handle=structmsg.createDispatcher()
result=structmsg.initDispatcher(handle)

function sendIMsg(handle)
  structmsg.uartReceiveCb(handle,string.char(tonumber("57",16)),1)
  structmsg.uartReceiveCb(handle,string.char(tonumber("00",16)),1)
  structmsg.uartReceiveCb(handle,string.char(tonumber("4D",16)),1)
  structmsg.uartReceiveCb(handle,string.char(tonumber("00",16)),1)
  structmsg.uartReceiveCb(handle,string.char(tonumber("00",16)),1)
  structmsg.uartReceiveCb(handle,string.char(tonumber("08",16)),1)
  structmsg.uartReceiveCb(handle,string.char(73),1)
  structmsg.uartReceiveCb(handle,string.char(10),1)
--  structmsg.uartReceiveCb(handle,string.char(tonumber("10",16)),1)
end
  
  echo=0
  dataBits=8
  baud=115200
  uart.setup(0, baud, dataBits, uart.PARITY_NONE, uart.STOPBITS_1, echo)
  eol="\n"
  uart.on("data", 1,
    function(data)
      structmsg.uartReceiveCb(handle,data,1)
--      sendIMsg(handle)
    end
  ,0)
