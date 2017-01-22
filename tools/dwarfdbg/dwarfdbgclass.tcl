# dwarfdbgclass.tcl --
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
# Copyright (c) 2012 Andreas Kupries <andreas_kupries@users.sourceforge.net>
#
# See the file "license.terms" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.
#
# RCS: @(#) $Id: stackc.tcl,v 1.1 2008/06/19 23:03:35 andreas_kupries Exp $

package require Tcl 8.6
package require critcl 3.1

critcl::buildrequirement {
    package require critcl::class ; # DSL, easy spec of Tcl class/object commands.
}

# # ## ### ##### ######## ############# #####################
## Administrivia

critcl::license {Andreas Kupries} {BSD licensed}

critcl::summary {dwarfdbg objects for Tcl.}

critcl::description {
    This package implements dwarf debug objects
    for Tcl. It uses the abstract data type
    provided by package 'dwarfdbg' for actual
    storage and operations.
}

critcl::subject dwarfdbg
critcl::subject {source level GUI debugger}
critcl::subject debugger
critcl::subject {source level debugger}

# # ## ### ##### ######## ############# #####################
## Configuration and implementation.

critcl::api import dwarfdbg 1

critcl::cheaders dwarfdbgclass/*.h ; # Method declarations and implementation,
critcl::csources dwarfdbgclass/*.c ; # outside of this main file.

critcl::class::define ::dwarfdbgclass {
    include m.h                  ; # Method function declarations.
    include dwarfdbg/dwarfdbgDecls.h ; # API of the generic DWARFDBG we are binding to.
    type    DWARFDBG

    constructor {
	instance = dwarfdbgNew (DwarfdbgFree, 0);
    } {
	/* Set back reference from DWARFDBG instance to instance command */
	dwarfdbg_clientdata_set (instance, (ClientData) cmd);
    }

    destructor {
	/* Release the whole stack. */
	dwarfdbgDel (instance);
    }

    method clear   as stm_CLEAR
    method destroy as stm_DESTROY
    method openElf as stm_OPEN_ELF
    method fcn1    as stm_FCN1
    method getErrorStr as stm_GET_ERROR_STR

    support {
	static void
	DwarfdbgFree (void* obj) {
	    /* Release the obj. */
	    Tcl_DecrRefCount ((Tcl_Obj*) obj);
	}
    }
}

# ### ### ### ######### ######### #########
## Ready
package provide dwarfdbgclass 1
