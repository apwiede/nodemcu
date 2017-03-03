set -x
spiffsimg -f $HOME/bene-nodemcu-firmware/bin/spiffs_module.img -i <<EOF
ls
export DescBA.txt module_image_files/DescBA.txt
export ValBA.txt module_image_files/ValBA.txt
export DescIA.txt module_image_files/DescIA.txt
export ValIA.txt module_image_files/ValIA.txt
export DescMA.txt module_image_files/DescMA.txt
export ValMA.txt module_image_files/ValMA.txt
export MsgHeads.txt module_image_files/MsgHeads.txt
export MsgActions.txt module_image_files/MsgActions.txt
EOF

#import writeBMsgDescs.lua writeBMsgDescs.lua
#import writeIMsgDescs.lua writeIMsgDescs.lua
#import writeMMsgDescs.lua writeMMsgDescs.lua
#import writeMsgHeaders.lua writeMsgHeaders.lua
#import writeMsgActions.lua writeMsgActions.lua
#import tstUartCb.lua tstUartCb.lua
#import initMsgFiles.lua initMsgFiles.lua
