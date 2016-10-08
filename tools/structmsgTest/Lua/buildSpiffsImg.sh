set -x
spiffsimg -f $HOME/bene-nodemcu-firmware/bin/spiffs0.img -c 0x60000 -i <<EOF
import writeBMsgDescs.lua writeBMsgDescs.lua
import writeIMsgDescs.lua writeIMsgDescs.lua
import writeMMsgDescs.lua writeMMsgDescs.lua
import writeMsgHeaders.lua writeMsgHeaders.lua
import writeMsgActions.lua writeMsgActions.lua
import tstUartCb.lua tstUartCb.lua
import initMsgFiles.lua initMsgFiles.lua
EOF
