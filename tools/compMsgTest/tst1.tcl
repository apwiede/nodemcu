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
source compMsgBuildMsg.tcl  

set result [::compMsg compMsgIdentify compMsgIdentifyInit]
puts stderr "result: $result!"
set result [::compMsg compMsgMsgDesc getHeaderFromUniqueFields 22272 16640 AA hdr]
puts stderr "result: $result!"
if {0} {
#set result [::compMsg compMsgBuildMsg buildMsgFromHeaderPart $hdr]
#puts stderr "result: $result!"
}
set result [::compMsg compMsgMsgDesc createMsgFromHeaderPart compMsgDispatcher $hdr handle]
puts stderr "result: $result!"
