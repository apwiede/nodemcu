# dwarfDbg.tcl --
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

critcl::license {Arnulf Wiedemann} BSD/MIT

critcl::summary {A C-level abstract datatype for libdwarf}

critcl::description {
    This package implements an abstract
    data type for libdwarf calls, at the C-level.
    No Tcl-binding is provided. See package
    'dwarfDbgClass' for that.
}

critcl::subject dwarfInfo
critcl::subject {data structure}
critcl::subject structure
critcl::subject {abstract data structure}
critcl::subject {generic data structure}

# # ## ### ##### ######## ############# #####################
## Configuration

critcl::api header dwarfDbg.h
critcl::cheaders   dwarfDbgInt.h dwarfDbgEsb.h dwarfDbgUtil.h dwarfDbgDict.h libdwarf/*.h
critcl::cheaders   dwarfDbgGetDbgInfo.h dwarfDbgDieInfo.h dwarfDbgStringInfo.h dwarfDbgLocationInfo.h
critcl::cheaders   dwarfDbgFrameInfo.h dwarfDbgAttributeInfo.h dwarfDbgCompileUnitInfo.h
critcl::cheaders   dwarfDbgLineInfo.h dwarfDbgRangeInfo.h dwarfDbgDebugInfo.h dwarfDbgAddrInfo.h
critcl::cheaders   dwarfDbgSubProgramInfo.h dwarfDbgTypeInfo.h

# # ## ### ##### ######## ############# #####################
## Exported API

# 
#  Notes
#  - openElf -- open an elf object file for getting debugging info
# 

critcl::api function dwarfDbgPtr_t dwarfDbgNew  {Tcl_Interp *interp}
critcl::api function void          dwarfDbgDel  {dwarfDbgPtr_t dbg}

critcl::api function void          dwarfDbgClientDataSet {dwarfDbgPtr_t dbg void* clientdata}
critcl::api function void*         dwarfDbgClientDataGet {dwarfDbgPtr_t dbg}
critcl::api function int           dwarfDbgOpenElf {dwarfDbgPtr_t dbg char* fileName}
critcl::api function int           dwarfDbgGetDbgInfos {dwarfDbgPtr_t dbg}
critcl::api function int           dwarfDbgGetFileInfos {dwarfDbgPtr_t dbg}
critcl::api function int           dwarfDbgGetFileLines {dwarfDbgPtr_t dbg int compileUnit}
critcl::api function int           dwarfDbgGetFiles {dwarfDbgPtr_t dbg}
critcl::api function int           dwarfDbgGetVarAddr {dwarfDbgPtr_t dbg char* sourceFileName int sourceLineNo char* varName int pc int fp int* addr}
critcl::api function int           dwarfDbgCloseElf {dwarfDbgPtr_t dbg}
critcl::api function int           dwarfDbgInit {dwarfDbgPtr_t dbg}
critcl::api function char*         dwarfDbgGetErrorStr {dwarfDbgPtr_t dbg}

# # ## ### ##### ######## ############# #####################
## Implementation.

critcl::csources dwarfDbg.c dwarfDbgElfInfo.c dwarfDbgGetDbgInfo.c dwarfDbgUtil.c dwarfDbgEsb.c dwarfDbgDict.c
critcl::csources dwarfDbgFileInfo.c dwarfDbgDieInfo.c dwarfDbgStringInfo.c dwarfDbgLocationInfo.c 
critcl::csources dwarfDbgFrameInfo.c dwarfDbgAttributeInfo.c dwarfDbgCompileUnitInfo.c dwarfDbgLineInfo.c
critcl::csources dwarfDbgRangeInfo.c dwarfDbgDebugInfo.c dwarfDbgAddrInfo.c dwarfDbgSubProgramInfo.c
critcl::csources dwarfDbgTypeInfo.c
critcl::clibraries libdwarf/libdwarf.a /usr/lib64/libelf.so
critcl::ccode {} ; # Fake the 'nothing to build detector'

# ### ### ### ######### ######### #########
## Ready
package provide dwarfDbg 1
