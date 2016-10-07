telegramData="ABC"

function initDbg()
handle2=structmsg2.create(5)
  structmsg2.addField(handle2,"@cmdKey","uint8_t",1)
  structmsg2.addField(handle2,"dbgmsg","uint8_t*",16)
  structmsg2.addField(handle2,"@crc","uint8_t",1)
  structmsg2.init(handle2)
  structmsg2.setFieldValue(handle2,"@cmdKey",89)
  structmsg2.setFieldValue(handle2,"dbgmsg","TEST DEBUG 12345")
  structmsg2.prepare(handle2)
  dbgData=structmsg2.getMsgData(handle2)
--  print(dbgData)
end


function initFields()
  handle=structmsg2.create(15)
  structmsg2.addField(handle,"@dst","uint16_t",2)
  structmsg2.addField(handle,"@src","uint16_t",2)
  structmsg2.addField(handle,"@totalLgth","uint16_t",2)
  structmsg2.addField(handle,"@cmdKey","uint8_t",1)
  structmsg2.addField(handle,"@GUID","uint8_t*",16)
  structmsg2.addField(handle,"passwd","uint8_t*",16)
  structmsg2.addField(handle,"reserve","uint8_t*",2)
  structmsg2.addField(handle,"@crc","uint8_t",1)
  structmsg2.init(handle)
end

function setFieldValues()
  structmsg2.setFieldValue(handle,"@dst",19712)
  structmsg2.setFieldValue(handle,"@src",22272)
  structmsg2.setFieldValue(handle,"@cmdKey",73)
  structmsg2.setFieldValue(handle,"@GUID","1234-5678-9876-2")
  structmsg2.setFieldValue(handle,"passwd","apwiede1apwiede2")
  structmsg2.setFieldValue(handle,"reserve","XY")
  structmsg2.prepare(handle)
--structmsg2.dump(handle)
  telegramData=structmsg2.getMsgData(handle)
end

function sendMsg()
  echo=0
  dataBits=8
  baud=115200
  uart.setup(0, baud, dataBits, uart.PARITY_NONE, uart.STOPBITS_1, echo)
  eol="\n"
  uart.on("data", 8,
    function(data)
--      uart.write(0,dbgData)
      uart.write(0,telegramData.."\r\n")
      tmr.alarm(0,1000,tmr.ALARM_SINGLE,function()
        uart.on("data")
        echo=1
        uart.setup(0, baud, dataBits, uart.PARITY_NONE, uart.STOPBITS_1, echo)
      end)
    end
  ,0)
end

function prepSendMsg()
initDbg()
  initFields()
  setFieldValues()
  tmr.alarm(1,3000,tmr.ALARM_SINGLE,function()
    sendMsg()
  end)
end

prepSendMsg()

