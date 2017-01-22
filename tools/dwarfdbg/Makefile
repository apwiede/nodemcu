all: clean
	critcl -keep -cache B -pkg dwarfDbg.tcl
	critcl -keep -cache B -pkg dwarfDbgClass.tcl

clean:
	rm -f ./lib/dwarfDbg/linux-x86_64/dwarfDbg.so
	rm -f ./lib/dwarfDbgClass/linux-x86_64/dwarfDbgClass.so
	rm -rf B
