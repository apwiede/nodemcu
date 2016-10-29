#!/usr/bin/env tclsh8.6

#source autoscroll.tcl   
source pdict.tcl
source dataView.tcl
source compMsgDataView.tcl    
source compMsgMsgDesc.tcl   
source compMsgData.tcl  
source compMsgDispatcher.tcl  
source compMsgIdentify.tcl  

set result [::compMsg compMsgIdentify compMsgIdentifyInit]
puts stderr "result: $result!"
