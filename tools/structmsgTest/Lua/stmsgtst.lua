
function buildPwdStructMsg(pwd)
  pwdStmsg=nil
  pwdStmsg=structmsg.create(5)
  structmsg.addField(pwdStmsg, "@randomNum", "uint32_t")
  structmsg.addField(pwdStmsg, "pwd", "uint8_t*", 16)
  structmsg.setFillerAndCrc(pwdStmsg)
  structmsg.setTargets(pwdStmsg, 123, 456, 789)
  structmsg.setFieldValue(pwdStmsg, "pwd",pwd)
  structmsg.encode(pwdStmsg)
  encoded=structmsg.getencoded(pwdStmsg)
  encrypted=structmsg.encrypt(pwdStmsg,"a1b2c3d4e5f6g7h8")
print("encrypted len: "..tostring(string.len(encrypted)))
  return pwdStmsg
end

function decryptPwdStructMsg(encryptedMsg)
  pwdStmsg=nil
  pwdStmsg=structmsg.create(5)
  structmsg.addField(pwdStmsg, "@randomNum", "uint32_t")
  structmsg.addField(pwdStmsg, "pwd", "uint8_t*", 16)
  structmsg.setFillerAndCrc(pwdStmsg)
  structmsg.setTargets(pwdStmsg, 1, 2, 3)
  structmsg.setcrypted(pwdStmsg,encryptedMsg)
  decrypted=structmsg.decrypt(pwdStmsg, "a1b2c3d4e5f6g7h8")
print("==decrypted pwd len: "..tostring(string.len(decrypted)))
  msg=structmsg.decode(pwdStmsg,decrypted)
print("==decode done "..tostring(msg));
  return pwdStmsg
end

