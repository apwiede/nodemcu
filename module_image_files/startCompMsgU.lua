handle=compmsg.createDispatcher()
result=compmsg.initDispatcher(handle, "U")
  echo=0
  dataBits=8
  baud=115200
  uart.setup(0, baud, dataBits, uart.PARITY_NONE, uart.STOPBITS_1, echo)
  uart.on("data", 1,
    function(data)
      compmsg.uartReceiveCb(handle,data,1)
    end
  ,0)


