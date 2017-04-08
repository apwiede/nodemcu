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
import MsgUse.txt MsgUse.txt
import MsgHeads.txt MsgHeads.txt
import MsgDescHeader.txt MsgDescHeader.txt
import MsgDescMidPart.txt MsgDescMidPart.txt
import MsgDescTrailer.txt MsgDescTrailer.txt
import MsgValHeader1.txt MsgValHeader1.txt

EOF
#rm -rf myConfig.txt
