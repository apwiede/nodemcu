# udpClientClass.tcl --
#
#       Implementation of an udp client for SSDP.
#       This code based on critcl v3.1, API compatible to the PTI [x].
#       [x] Pure Tcl Implementation.
#
# Demonstrates not just the stubs import and meta data declaration,
# but also the utility package for the creation of classes and objects
# in C, with both classes and their instances represented as Tcl
# commands.
#
# Copyright (c) 2017 Arnulf Wiedemann <arnulf@wiedemann-pri.de>
#
# See the file "license.terms" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.
#

package require Tcl 8.6
package require critcl 3.1

critcl::buildrequirement {
    package require critcl::class ; # DSL, easy spec of Tcl class/object commands.
}

# # ## ### ##### ######## ############# #####################
## Administrivia

critcl::license {Arnulf Wiedemann} {BSD/MIT licensed}

critcl::summary {udpClient objects for Tcl.}

critcl::description {
    This package implements udp SSDP client
    for Tcl. It uses the abstract data type
    provided by package 'dwarfdbg' for actual
    storage and operations.
}

critcl::subject udpClient
critcl::subject {source level GUI debugger}
critcl::subject debugger
critcl::subject {source level debugger}

# # ## ### ##### ######## ############# #####################
## Configuration and implementation.

critcl::api import udpClient 1

critcl::cheaders udpClientClass/*.h ; # Method declarations and implementation,
critcl::csources udpClientClass/*.c ; # outside of this main file.

critcl::class::define ::udpClientClass {
    include m.h                      ; # Method function declarations.
    include udpClient/udpClientDecls.h ; # API of the generic udpClientPtr_t we are binding to.
    type    udpClientPtr_t

    constructor {
	instance = udpClientNew (interp);
    } {
	/* Set back reference from udpClientPtr_t instance to instance command */
	udpClientClientDataSet (instance, (ClientData) cmd);
    }

    destructor {
	/* Release the whole stack. */
	udpClientDel (instance);
    }

    method clear         as stm_CLEAR
    method destroy       as stm_DESTROY
    method openDevice    as stm_OPEN_DEVICE
    method getUdpInfos   as stm_GET_UDP_INFOS
    method closeDevice   as stm_CLOSE_DEVICE
    method init          as stm_INIT
    method getErrorStr   as stm_GET_ERROR_STR

    support {
	static void
	UdpClientFree (void* obj) {
	    /* Release the obj. */
	    Tcl_DecrRefCount ((Tcl_Obj*) obj);
	}
    }
}

# ### ### ### ######### ######### #########
## Ready
package provide udpClientClass 1
