dofile("myConfig.lua")

function checkOK(result, fieldName)
  if (result ~= "OK") then
    print(tostring(result).." for field: "..fieldName)
  end
end

dofile("stmsgtst.lua")
handle=buildEncodedPwdStructMsg("/dir1/dir2/dir34")
print("encoded_pwd1: "..tostring(string.len(encoded_pwd1)))
--structmsg.dump(handle)

if (false) then
def="pwd1"
structmsg.createdef(def, 15)
result=structmsg.adddeffield(def, "@src", "uint16_t", 2)
checkOK(result,"@src")
result=structmsg.adddeffield(def, "@dst", "uint16_t", 2)
checkOK(result,"@dst")
result=structmsg.adddeffield(def, "@totalLgth", "uint16_t", 2)
checkOK(result,"@totalLgth")
result=structmsg.adddeffield(def, "@cmdKey", "uint16_t", 2)
checkOK(result,"@cmdKey")
result=structmsg.adddeffield(def, "@cmdLgth", "uint16_t", 2)
checkOK(result,"@cmdLgth")
result=structmsg.adddeffield(def, "@randomNum", "uint32_t", 4)
checkOK(result,"@randomNum")
result=structmsg.adddeffield(def, "@sequenceNum", "uint32_t", 4)
checkOK(result,"@sequenceNum")
result=structmsg.adddeffield(def, "pwd1", "uint8_t*", 16)
checkOK(result,"pwd")
result=structmsg.adddeffield(def, "@tablerows", "uint8_t", 2)
checkOK(result,"@tablerows")
result=structmsg.adddeffield(def, "@tablerowfields", "uint8_t", 3)
checkOK(result,"@tablerowfields")
result=structmsg.adddeffield(def, "ssid1", "uint8_t*", 16)
checkOK(result,"ssid")
result=structmsg.adddeffield(def, "rssid1", "uint8_t*", 6)
checkOK(result,"rssid")
result=structmsg.adddeffield(def, "channel1", "uint8_t*", 6)
checkOK(result,"channel")
result=structmsg.adddeffield(def, "@filler", "uint8_t*", 1)
checkOK(result,"@filler")
result=structmsg.adddeffield(def, "@crc", "uint16_t", 2)
checkOK(result,"@crc")
result=structmsg.dumpdef(def)
print(tostring(result))
encoded=structmsg.encodedef(def)
--print("encoded len: "..tostring(string.len(encoded)))
result=structmsg.deletedef(def)
checkOK(result,"delete")
def= nil
end
if (true) then
def="pwd2"
structmsg.createdef(def, 15)
result=structmsg.adddeffield(def, "@src", "uint16_t", 2)
checkOK(result,"@src")
result=structmsg.adddeffield(def, "@dst", "uint16_t", 2)
checkOK(result,"@dst")
result=structmsg.adddeffield(def, "@totalLgth", "uint16_t", 2)
checkOK(result,"@totalLgth")
result=structmsg.adddeffield(def, "@cmdKey", "uint16_t", 2)
checkOK(result,"@cmdKey")
result=structmsg.adddeffield(def, "@cmdLgth", "uint16_t", 2)
checkOK(result,"@cmdLgth")
result=structmsg.adddeffield(def, "@randomNum", "uint32_t", 4)
checkOK(result,"@randomNum")
result=structmsg.adddeffield(def, "@sequenceNum", "uint32_t", 4)
checkOK(result,"@sequenceNum")
result=structmsg.adddeffield(def, "pwd1", "uint8_t*", 16)
checkOK(result,"pwd")
result=structmsg.adddeffield(def, "@tablerows", "uint8_t", 2)
checkOK(result,"@tablerows")
result=structmsg.adddeffield(def, "@tablerowfields", "uint8_t", 3)
checkOK(result,"@tablerowfields")
result=structmsg.adddeffield(def, "ssid1", "uint8_t*", 16)
checkOK(result,"ssid")
result=structmsg.adddeffield(def, "rssid1", "uint8_t*", 6)
checkOK(result,"rssid")
result=structmsg.adddeffield(def, "channel1", "uint8_t*", 6)
checkOK(result,"channel")
result=structmsg.adddeffield(def, "@filler", "uint8_t*", 1)
checkOK(result,"@filler")
result=structmsg.adddeffield(def, "@crc", "uint16_t", 2)
checkOK(result,"@crc")
--result=structmsg.dumpdef(def)
--print(tostring(result))
end
if (true) then
encoded2=structmsg.encodedef(def)
--print("encoded2 len: "..tostring(string.len(encoded2)))
encrypted2=structmsg.encryptdef(def,cryptkey)
print("encrypted2 len: "..tostring(string.len(encrypted2)))
name=structmsg.decryptdefgetname(encrypted2,cryptkey)
print("name: "..tostring(name))
structmsg.setcrypteddef(name,encrypted2)
decrypted=structmsg.decryptdef(name,cryptkey)
print("decrypted len: "..tostring(string.len(decrypted)))
--structmsg.decodedef(def,encoded)
--result=structmsg.dumpdef(def)
--print(tostring(result))
end
result=structmsg.createmsgfromdef(def)
checkOK(result,"createmsgfromdef")
print("Msg created")
print("encoded_pwd1: "..tostring(string.len(encoded_pwd1)))
result=structmsg.decode(handle,encoded_pwd1)
checkOK(result,"decode")
--structmsg.dump(handle)
val1=structmsg.getTableFieldValue(handle, "rssid1",1)
print("val1: "..tostring(val1))
val2=structmsg.getFieldValue(handle, "pwd1")
print("val2: "..tostring(val2))
--result=structmsg.deletedef(def)
--checkOK(result,"deletedef")
--encoded=nil
--encoded2=nil
--result=structmsg.deletedefinitions()
--checkOK(result,"deletedefinitions")
--def=nil
