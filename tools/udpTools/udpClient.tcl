# udpClient.tcl --
#
#	basic Tcl interface for udp SSDP client
#
# Copyright (c) 2017 Arnulf Wiedeman <arnulf@wiedemann-pri.de>

# Exporting a C-level stubs API through critcl v3, with a
# package header file containing public type definitions, macros,
# etc., and internal C companion files.

# # ## ### ##### ######## ############# #####################
## Requirements

package require Tcl 8.6
package require critcl 3 ;# stubs management

# # ## ### ##### ######## ############# #####################
## Administrivia

critcl::license {Arnulf Wiedemann} BSD/MIT

critcl::summary {A C-level abstract datatype for a udp client}

critcl::description {
    This package implements an abstract
    data type for an SSDP UDP cleint at the C-level.
    No Tcl-binding is provided. See package
    'udpClientClass' for that.
}

critcl::subject udpClient
critcl::subject {data structure}
critcl::subject structure
critcl::subject {abstract data structure}
critcl::subject {generic data structure}

# # ## ### ##### ######## ############# #####################
## Configuration

critcl::api header udpClient.h
critcl::cheaders   udpClientInt.h udpClientDeviceInfo.h udpClientErrorCodes.h udpClientUdpInfo.h

# # ## ### ##### ######## ############# #####################
## Exported API

# 
#  Notes
#  - openDevice -- open an device for getting udp info
# 

critcl::api function udpClientPtr_t udpClientNew  {Tcl_Interp *interp}
critcl::api function void           udpClientDel  {udpClientPtr_t dbg}

critcl::api function void           udpClientClientDataSet {udpClientPtr_t dbg void* clientdata}
critcl::api function void*          udpClientClientDataGet {udpClientPtr_t dbg}
critcl::api function int            udpClientOpenDevice {udpClientPtr_t dbg char* deviceName}
critcl::api function int            udpClientGetUdpInfos {udpClientPtr_t dbg}
critcl::api function int            udpClientCloseDevice {udpClientPtr_t dbg}
critcl::api function int            udpClientInit {udpClientPtr_t dbg}
critcl::api function char*          udpClientGetErrorStr {udpClientPtr_t dbg}

# # ## ### ##### ######## ############# #####################
## Implementation.

critcl::csources udpClient.c udpClientDeviceInfo.c udpClientUdpInfo.c
critcl::clibraries /usr/lib64/libpcap.a
critcl::ccode {} ; # Fake the 'nothing to build detector'

# ### ### ### ######### ######### #########
## Ready
package provide udpClient 1
