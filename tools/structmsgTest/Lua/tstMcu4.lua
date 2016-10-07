function createIAckMsg()
  local msgInfos={}

  msgInfos["1"]="@dst,uint16_t,2,0"
  msgInfos["2"]="@totalLgth,uint16_t,2,0"
  flags=0
  handleIAck=structmsg2.createMsgFromList(msgInfos,0,flags);
print("2")
  return handleIAck
end

createIAckMsg()
print("done")