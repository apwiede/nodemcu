function checkOK(result, fieldName)
  if (result ~= "OK") then
    print(tostring(result).." for field: "..fieldName)
  end
end


function buildPwdStructMsg(pwd)
  pwdStmsg=nil
  pwdStmsg=structmsg.create(8)
  result=structmsg.addField(pwdStmsg, "@randomNum", "uint32_t")
  checkOK(result,"@randomNum")
  result=structmsg.addField(pwdStmsg, "pwd", "uint8_t*", 16)
  checkOK(result,"pwd")
  result=structmsg.addField(pwdStmsg, "@tablerows", "uint8_t", 2)
  checkOK(result,"@tablerows")
  result=structmsg.addField(pwdStmsg, "@tablerowfields", "uint8_t", 2)
  checkOK(result,"@tabelrowfields")
  result=structmsg.addField(pwdStmsg, "fld1", "uint8_t*", 2)
  checkOK(result,"fld1")
  result=structmsg.addField(pwdStmsg, "fld2", "uint8_t*", 2)
  checkOK(result,"fld2")
  result=structmsg.setFillerAndCrc(pwdStmsg)
  checkOK(result,"fillerAndCrc")
  result=structmsg.setFieldValue(pwdStmsg, "@src",123)
  checkOK(result,"@src")
  result=structmsg.setFieldValue(pwdStmsg, "@dst",456)
  checkOK(result,"@dst")
  result=structmsg.setFieldValue(pwdStmsg, "@cmdKey",789)
  checkOK(result,"@cmdKey2")
  result=structmsg.setFieldValue(pwdStmsg, "pwd",pwd)
  checkOK(result,"pwd")
  result=structmsg.setTableFieldValue(pwdStmsg, "fld1",0,"xx")
  checkOK(result,"fld1")
  result=structmsg.setTableFieldValue(pwdStmsg, "fld2",0,"yy")
  checkOK(result,"fld2")
  result=structmsg.setTableFieldValue(pwdStmsg, "fld1",1,"aa")
  checkOK(result,"fld1")
  result=structmsg.setTableFieldValue(pwdStmsg, "fld2",1,"bb")
  checkOK(result,"fld2")
val=structmsg.getTableFieldValue(pwdStmsg, "fld2",1)
print("val: "..tostring(val))
  result=structmsg.encode(pwdStmsg)
  checkOK(result,"encode")
print("encoded")
  encoded=structmsg.getencoded(pwdStmsg)
print("GETencoded")
  encrypted=structmsg.encrypt(pwdStmsg,"a1b2c3d4e5f6g7h8")
print("encrypted len: "..tostring(string.len(encrypted)).." "..tostring(enclen))
  structmsg.setcrypted(pwdStmsg,encrypted)
print("1")
  decrypted=structmsg.decrypt(pwdStmsg,"a1b2c3d4e5f6g7h8")
print("2")
  decoded=structmsg.decode(pwdStmsg,decrypted)
print("3 "..tostring(decoded))
  handle=structmsg.decryptgethandle(encrypted, "a1b2c3d4e5f6g7h8")
  print("handle: "..handle)
--  structmsg.dump(handle)
--  structmsg.dump(pwdStmsg)
  return pwdStmsg
end

function decryptPwdStructMsg(encryptedMsg)
  pwdStmsg=nil
  pwdStmsg=structmsg.create(5)
  structmsg.addField(pwdStmsg, "@randomNum", "uint32_t")
  structmsg.addField(pwdStmsg, "pwd", "uint8_t*", 16)
  structmsg.setFillerAndCrc(pwdStmsg)
--  structmsg.setTargets(pwdStmsg, 1, 2, 3)
  structmsg.setFieldValue(pwdStmsg, "@src",11)
  structmsg.setFieldValue(pwdStmsg, "@dst",22)
  structmsg.setFieldValue(pwdStmsg, "@cmdKey",33)
  structmsg.setcrypted(pwdStmsg,encryptedMsg)
  decrypted=structmsg.decrypt(pwdStmsg, "a1b2c3d4e5f6g7h8")
print("==decrypted pwd len: "..tostring(string.len(decrypted)))
  msg=structmsg.decode(pwdStmsg,decrypted)
print("==decode done "..tostring(msg));
  handle=structmsg.decryptgethandle(encryptedMsg, "a1b2c3d4e5f6g7h8")
  print("handle: "..handle)
  return pwdStmsg
end
