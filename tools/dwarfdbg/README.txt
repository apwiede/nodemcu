A TCL based source level debugger with GUI for ESP8266 with critcl-based packages.
The idea is to use the gdbstub modul from Espressif which has bin integrated 
into a derived version of the nodemcu firmware.
The program is also based on libdwarf from SGI for getting debugging information from
the object file.
The GUI will be built with Tk widgets.

Right now there exists a very small Tcl module for interacting with the  gdbstub 
Module via the Uart. A configuration option allows the nodemcu firmware to stop after booting.
At that point the esp Module registers can be read out (seems to work).

Dwarfdbg right now can open the firmware elf module and get some debug information like:
  - Compile Units (CU)
  - Source and include file names based on the CU's
  - Source line numbers based on the CU's

This is WORK IN PROGRESS!!
It is at the very beginnig!!

A program written to demonstrate

	Export of an API as Tcl stubs table	(Package "dwarfdbg")
	Import of an API defined as stubs-table	(Package "dwarfdbgclass")

	Easy writing of C classes, with class and instances
	represented as commands, through the utility package
	critcl::class

Package "dwarfdbg" also demonstrate the export of a companion header
file containing declarations of package specific data structures and
macros which are not expressible in the basic .decls file and header
derived from it, plus, incidentally, the separation of public and
internal headers.

Package "dwarfdbgclass" incidentially also demonstrates the use of companion
.c and .h files in the implementation of a package.

Sources
	Package	"dwarfdbg":		dwarfdbg.tcl, dwarfdbg.c, dwarfdbg.h, dwarfdbgInt.h ...
	Package "dwrafdbgclass":	dwarfdbgclass.tcl, dwarfdbgclass/*.[ch]

Notes:

*	"dwarfdbg" implements an abstract dwarfdbg data type and exports
	a C-level API for it, as a stubs table.

*	"dwarfdbgclass" wraps the abstract dwarfdbg data type of "dwarfdbg" into
	a Tcl class/object system where dwarfdbgs are objects, each
	represented as a Tcl command.
