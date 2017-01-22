# dwarfdbg.tcl --
#
#	basic Tcl interface for libdwarf
#
# Copyright (c) 2017 Arnulf Wiedeman <arnulf@wiedemann-pri.de>

# Example of exporting a C-level stubs API through critcl v3, with a
# package header file containing public type definitions, macros,
# etc., and internal C companion files.

# # ## ### ##### ######## ############# #####################
## Requirements

package require Tcl 8.6
package require critcl 3 ;# stubs management

# # ## ### ##### ######## ############# #####################
## Administrivia

critcl::license {Arnulf Wiedemann} BSD

critcl::summary {A C-level abstract datatype for libdwarf}

critcl::description {
    This package implements an abstract
    data type for libdwarf calls, at the C-level.
    No Tcl-binding is provided. See package
    'dwarfdbgclass' for that.
}

critcl::subject dwarfInfo
critcl::subject {data structure}
critcl::subject structure
critcl::subject {abstract data structure}
critcl::subject {generic data structure}

# # ## ### ##### ######## ############# #####################
## Configuration

critcl::api header dwarfdbg.h
critcl::cheaders   dwarfdbgInt.h esb.h ../libdwarf/code/libdwarf/*.h

# # ## ### ##### ######## ############# #####################
## Exported API

# 
#  Notes
#  - openElf -- open an elf object file for getting debugging info
# 

critcl::api function DWARFDBG   dwarfdbgNew  {DWARFDBG_CELL_FREE freeCell void* clientdata}
critcl::api function void       dwarfdbgDel  {DWARFDBG dbg}

critcl::api function void       dwarfdbg_clientdata_set {DWARFDBG dbg void* clientdata}
critcl::api function void*      dwarfdbg_clientdata_get {DWARFDBG dbg}
critcl::api function int        dwarfdbgOpenElf {DWARFDBG dbg char* fileName}
critcl::api function void       dwarfdbg_fcn1 {DWARFDBG dbg}
critcl::api function char*      dwarfdbgGetErrorStr {DWARFDBG dbg}

# # ## ### ##### ######## ############# #####################
## Implementation.

critcl::csources dwarfdbg.c dwarfdbgOpenElf.c dwarfdbgUtil.c esb.c
critcl::clibraries $::env(HOME)/libdwarf/code/libdwarf/libdwarf.a /usr/lib64/libelf.so
critcl::ccode {} ; # Fake the 'nothing to build detector'

# ### ### ### ######### ######### #########
## Ready
package provide dwarfdbg 1
