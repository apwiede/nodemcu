all: clean
	critcl -keep -cache B -pkg udpClient.tcl
	critcl -keep -cache B -pkg udpClientClass.tcl

clean:
	rm -f ./lib/udpClient/linux-x86_64/udpClient.so
	rm -f ./lib/udpClientClass/linux-x86_64/udpClientClass.so
	rm -rf B
