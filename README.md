# nodemcu

## modules for nodemcu firmware

### additional modules for use within nodemcu-firmware

check out from the nodemcu-firmware repository and add to directory app/modules:

- app/modules/compMsg.c  a module for handling and building "composite messages" (compMsg)
                           The message layout is in files in the spiffs and then a packed message string
                           can be generated and also be unpacked 
                           and eventually be encrypted/decrypted using AES-CBC crypto module
- app/compMsg/*

You have to add compMsg to the SUBDIRS variable in app/Makefile

You have to add COMPMSG defines in app/include/user_modules.h 
for making the modules active.

## Attention!! This is work in progress and not yet usefull for production!!

## Useful links

| Resource | Location |
| -------------- | -------------- |
| Developer Wiki       | https://github.com/nodemcu/nodemcu-firmware/wiki |
| Module Documentation | https://apwiede.github.io/nodemcu-firmware/index.html |

### compMsg message format:

| uint16\_t | uint16\_t | uint16\_t  | uint16\_t | uint16\_t | \< uint32\_t   | uint8\_t\* | uint8\_t\* | ... \> | uint8\_t\* | uint16\_t |
| ---------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- | ---------- |
|   src    |   dst    | totalLgth |    cmd   | cmdLgth  | < randomNum |   fld1    |   fld2    | ... > | filler   |   crc     |

### More detailed description of the interfaces following.
