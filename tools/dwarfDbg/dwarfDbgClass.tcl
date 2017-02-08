# dwarfDbgClass.tcl --
#
#       Implementation of an source level GUI debugger for Tcl.
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

critcl::summary {dwarfDbg objects for Tcl.}

critcl::description {
    This package implements dwarf debug objects
    for Tcl. It uses the abstract data type
    provided by package 'dwarfdbg' for actual
    storage and operations.
}

critcl::subject dwarfDbg
critcl::subject {source level GUI debugger}
critcl::subject debugger
critcl::subject {source level debugger}

# # ## ### ##### ######## ############# #####################
## Configuration and implementation.

critcl::api import dwarfDbg 1

critcl::cheaders dwarfDbgClass/*.h ; # Method declarations and implementation,
critcl::csources dwarfDbgClass/*.c ; # outside of this main file.

critcl::class::define ::dwarfDbgClass {
    include m.h                      ; # Method function declarations.
    include dwarfDbg/dwarfDbgDecls.h ; # API of the generic dwarfDbgPtr_t we are binding to.
    type    dwarfDbgPtr_t

    constructor {
	instance = dwarfDbgNew (interp);
    } {
	/* Set back reference from dwarfDbgPtr_t instance to instance command */
	dwarfDbgClientDataSet (instance, (ClientData) cmd);
    }

    destructor {
	/* Release the whole stack. */
	dwarfDbgDel (instance);
    }

    method clear         as stm_CLEAR
    method destroy       as stm_DESTROY
    method openElf       as stm_OPEN_ELF
    method getFiles      as stm_GET_FILES
    method getFileInfos  as stm_GET_FILE_INFOS
    method getFileLines  as stm_GET_FILE_LINES
    method getDbgInfos   as stm_GET_DBG_INFOS
    method getVarAddr    as stm_GET_VAR_ADDR
    method closeElf      as stm_CLOSE_ELF
    method init          as stm_INIT
    method getErrorStr   as stm_GET_ERROR_STR

    support {
	static void
	DwarfDbgFree (void* obj) {
	    /* Release the obj. */
	    Tcl_DecrRefCount ((Tcl_Obj*) obj);
	}
    }
}

# ### ### ### ######### ######### #########
## Ready
package provide dwarfDbgClass 1
