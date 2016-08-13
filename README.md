# nodemcu
modules for nodemcu firmware

additional modules for use within nodemcu-firmware

check out from the nodemcu-firmware repository and add:

to directory app/modules:
app/modules/websocket.c  a Websocket server and client very similar to the net module.
                          mostly the same interface, but for use of websockets

app/modules/structmsg.c    a module for handling and building "structured messages" (structmsg)
                           The message layout can be designed with Lua calls and then a packed message string
                           can be generated and also be unpacked 
                           and eventually be encrypted/decrypted using AES-CBC crypto module
app/structmsg/structmsg.c
app/structmsg/structmsg.h

You have to add structmsg to the SUBDIRS variable in app/Makefile

You have to add WEBSOCKET and STRUCTMSG defines in app/include/user_modules.h 
for making the modules active.

Attention!! This is work in progress and not yet usefull for production!!

More description of the interface following.
