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
import MsgDescKeyValue.txt MsgDescKeyValue.txt
import MsgFieldsToSave.txt MsgFieldsToSave.txt
import MsgHeads.txt MsgHeads.txt
import MsgActions.txt MsgActions.txt
import MsgWifiDataValues.txt MsgWifiDataValues.txt
import MsgModuleDataValues.txt MsgModuleDataValues.txt
import MsgDescP1.txt MsgDescP1.txt
import MsgDescP2.txt MsgDescP2.txt
import MsgDescP3.txt MsgDescP3.txt
import MsgDescP4.txt MsgDescP4.txt
import MsgDescP5.txt MsgDescP5.txt
import MsgDescP6.txt MsgDescP6.txt
import MsgValP1.txt MsgValP1.txt
import MsgValP2.txt MsgValP2.txt
import MsgValP3.txt MsgValP3.txt
import MsgValP4.txt MsgValP4.txt
import MsgValP5.txt MsgValP5.txt
import MsgValP6.txt MsgValP6.txt

EOF
#rm -rf myConfig.txt
