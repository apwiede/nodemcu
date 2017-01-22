all: clean
	critcl -keep -cache B -pkg dwarfdbg.tcl
	critcl -keep -cache B -pkg dwarfdbgclass.tcl

clean:
	rm -f ./lib/dwarfdbg/inux-x86_64/dwarfdbg.so
	rm -f ./lib/dwarfdbgclass/linux-x86_64/dwarfdbgclass.so
	rm -rf B
