#!/usr/bin/env tclsh8.6

#source autoscroll.tcl   
source pdict.tcl
source dataView.tcl
source compMsgDataView.tcl    
source compMsgMsgDesc.tcl   
source compMsgData.tcl  
source compMsgDispatcher.tcl  
source compMsgIdentify.tcl  
source compMsgAction.tcl  
source compMsgWifiData.tcl  
source compMsgBuildMsg.tcl  
source compMsgSendReceive.tcl  
source compMsgModuleData.tcl  

proc checkErrOK {result} {
  switch $result {
    0 {
    }
    default {
      error "ERROR result: $result!"
    }
  }
}


set compMsgDispatcher [dict create]
set compMsgWifiData [dict create]
set result [::compMsg compMsgDispatcher newCompMsgDispatcher]
checkErrOK $result
set result [::compMsg compMsgDispatcher createDispatcher dispatcherHandle]
checkErrOK $result
puts stderr "dispatcherHandle!$dispatcherHandle!"
set result [::compMsg compMsgDispatcher initDispatcher compMsgDispatcher]
checkErrOK $result
set result [::compMsg compMsgMsgDesc getWifiKeyValueKeys compMsgDispatcher wifiData]
pdict $wifiData
checkErrOK $result
set result [::compMsg compMsgMsgDesc getHeaderFromUniqueFields 22272 16640 AA hdr]
checkErrOK $result
if {0} {
#set result [::compMsg compMsgBuildMsg buildMsgFromHeaderPart $hdr]
#puts stderr "result: $result!"
}
set result [::compMsg compMsgMsgDesc createMsgFromHeaderPart compMsgDispatcher $hdr handle]
checkErrOK $result
puts stderr "END"
