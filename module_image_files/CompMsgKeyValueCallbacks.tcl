set ::sequenceNumber 0

# ====================== getSequenceNumber ===================

proc ::getSequenceNumber {compMsgDispatcherVar valueVar} {
  upvar $compMsgDispatcherVar compMsgDispatcher
  upvar $valueVar value

  incr ::sequenceNumber
  set msgDescPart [dict get $compMsgDispatcher msgDescPart]
  dict set compMsgDispatcher msgDescPart $msgDescPart
  set value [binary format I $::sequenceNumber]
  return $::COMP_MSG_ERR_OK
}

# ====================== getMACAddr ===================

proc ::getMACAddr {compMsgDispatcherVar valueVar} {
  upvar $compMsgDispatcherVar compMsgDispatcher
  upvar $valueVar value

  set value \x12\x34\x56\x78\x90\xAB
puts stderr "getMacAddr: value: $value!"
  set msgDescPart [dict get $compMsgDispatcher msgDescPart]
  dict set compMsgDispatcher msgDescPart $msgDescPart
  return $::COMP_MSG_ERR_OK
}

# ====================== getMachineState ===================

proc ::getMachineState {compMsgDispatcherVar valueVar} {
  upvar $compMsgDispatcherVar compMsgDispatcher
  upvar $valueVar value

  set value \xE0\x00\x00\x00\x00\x00\x00\x00
puts stderr "getMachineState: value: $value!"
  set msgDescPart [dict get $compMsgDispatcher msgDescPart]
  dict set compMsgDispatcher msgDescPart $msgDescPart
  return $::COMP_MSG_ERR_OK
}

# ====================== getAPSsid ===================

proc ::getAPSsid {compMsgDispatcherVar valueVar} {
  upvar $compMsgDispatcherVar compMsgDispatcher
  upvar $valueVar value

  set value [dict get $compMsgDispatcher clientSsid]
puts stderr "getAPSsid: value: $value!"
  return $::COMP_MSG_ERR_OK
}

# ====================== getAPPassword ===================

proc ::getAPPassword {compMsgDispatcherVar valueVar} {
  upvar $compMsgDispatcherVar compMsgDispatcher
  upvar $valueVar value

  set value [dict get $compMsgDispatcher clientPassword]
puts stderr "getAPPassword: value: $value!"
  return $::COMP_MSG_ERR_OK
}

# ====================== getPasswordA ===================

proc ::getPasswordA {compMsgDispatcherVar valueVar} {
  upvar $compMsgDispatcherVar compMsgDispatcher
  upvar $valueVar value

  set value passwd12passwd34
puts stderr "getPasswordA: value: $value!"
  return $::COMP_MSG_ERR_OK
}

# ====================== getClientSsidSize ===================

proc ::getClientSsidSize {compMsgDispatcherVar} {
  upvar $compMsgDispatcherVar compMsgDispatcher

puts stderr "msgKeyValueDescPart: [dict get $compMsgDispatcher msgKeyValueDescPart]!"
  set value 32
  dict set compMsgDispatcher msgKeyValueDescPart keyLgth $value
puts stderr "getClientSsidSize: value: $value!"
  return $::COMP_MSG_ERR_OK
}

# ====================== getClientPasswdSize ===================

proc ::getClientPasswdSize {compMsgDispatcherVar} {
  upvar $compMsgDispatcherVar compMsgDispatcher

  set value 64
  dict set compMsgDispatcher msgKeyValueDescPart keyLgth $value
puts stderr "getClientPasswdSize: value: $value!"
  return $::COMP_MSG_ERR_OK
}


