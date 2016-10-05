--print("heap0: "..node.heap())
handle=structmsg2.createDispatcher()

function sendIMsg(handle)
  structmsg2.uartReceiveCb(handle,string.char(tonumber("57",16)),1)
  structmsg2.uartReceiveCb(handle,string.char(tonumber("00",16)),1)
  structmsg2.uartReceiveCb(handle,string.char(tonumber("4D",16)),1)
  structmsg2.uartReceiveCb(handle,string.char(tonumber("00",16)),1)
  structmsg2.uartReceiveCb(handle,string.char(tonumber("00",16)),1)
  structmsg2.uartReceiveCb(handle,string.char(tonumber("08",16)),1)
  structmsg2.uartReceiveCb(handle,string.char(73),1)
  structmsg2.uartReceiveCb(handle,string.char(10),1)
--  structmsg2.uartReceiveCb(handle,string.char(tonumber("10",16)),1)
end
  
  echo=0
  dataBits=8
  baud=115200
  uart.setup(0, baud, dataBits, uart.PARITY_NONE, uart.STOPBITS_1, echo)
  eol="\n"
  uart.on("data", 1,
    function(data)
      structmsg2.uartReceiveCb(handle,data,1)
--      sendIMsg(handle)
    end
  ,0)
