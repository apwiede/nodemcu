set -x
#if [ -f ../myConfig.txt ]
#then
#  cat CompMsgWifiValues.txt ../myConfig.txt > myConfig.txt
#else
#  echo "missing file ../myConfig.txt aborting!"
#fi
spiffsimg -f ../bin/spiffs1.img -c 0x60000 -i <<EOF
import startCompMsgA.lua startCompMsgA.lua
import startCompMsgC.lua startCompMsgC.lua
import startCompMsgD.lua startCompMsgD.lua
import startCompMsgG.lua startCompMsgG.lua
import startCompMsgI.lua startCompMsgI.lua
import startCompMsgL.lua startCompMsgL.lua
import startCompMsgN.lua startCompMsgN.lua
import startCompMsgO.lua startCompMsgO.lua
import startCompMsgo.lua startCompMsgo.lua
import startCompMsgS.lua startCompMsgS.lua
import startCompMsgU.lua startCompMsgU.lua
import startCompMsgW.lua startCompMsgW.lua

import MsgFiles.txt MsgFiles.txt
import MsgDescHeader.txt MsgDescHeader.txt
import MsgDescMidPart.txt MsgDescMidPart.txt
import MsgDescTrailer.txt MsgDescTrailer.txt
import MsgFieldsToSave.txt MsgFieldsToSave.txt
import MsgHeads.txt MsgHeads.txt
import MsgActions.txt MsgActions.txt
import MsgValHeader.txt MsgValHeader.txt
import MsgWifiDataValues.txt MsgWifiDataValues.txt
import MsgModuleDataValues.txt MsgModuleDataValues.txt

EOF
#rm -rf myConfig.txt
