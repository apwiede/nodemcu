Example of critcl-based packages.

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
	Package	"dwarfdbg":		dwarfdbg.tcl, dwarfdbg.c, dwarfdbg.h, dwarfdbgInt.h
	Package "dwrafdbgclass":	dwarfdbgclass.tcl, dwarfdbgclass/*.[ch]

Notes:

*	"dwarfdbg" implements an abstract dwarfdbg data type and exports
	a C-level API for it, as a stubs table.

*	"dwarfdbgclass" wraps the abstract dwarfdbg data type of "dwarfdbg" into
	a Tcl class/object system where dwarfdbgs are objects, each
	represented as a Tcl command.
