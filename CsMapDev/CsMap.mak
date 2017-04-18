#
#
#
All : Linux64

clean : CleanLinux64

rebuild : RebuildLinux64

#
# The following macro values are conveyed to the individual makefiles referenced
# by this overall makefile via the environment.  Thus, all components get the
# same values and the build id homogeneous.  Each of the referenced makefiles
# can be used independently, and are heavily commented to explain how they work
# and how minor tweaks to the makefile can be made to meet your specific
# requirements.
#
export CSMAP_LIB_NAME := CsMap
export VERSION := 47
export CONFIGURATION := Linux
export PROCESSOR := x64
export C_FLG = -c -w -O2 -I../Include
export CXX_FLG = -c -w -O2 -I../Include

Linux64 : 
	$(MAKE) -e -C ./Source -f Library.mak
	$(MAKE) -e -C ./Dictionaries -f Compiler.mak
	$(MAKE) -e -C ./TestCpp -f TestCpp.mak

CleanLinux64 :
	$(MAKE) -e -C ./Source -f Library.mak clean
	$(MAKE) -e -C ./Dictionaries -f Compiler.mak clean
	$(MAKE) -e -C ./TestCpp -f TestCpp.mak clean

RebuildLinux64 :
	$(MAKE) -e -C ./Source -f Library.mak rebuild
	$(MAKE) -e -C ./Dictionaries -f Compiler.mak rebuild
	$(MAKE) -e -C ./TestCpp -f TestCpp.mak rebuild

.PHONY : Test
.PHONY : QuickTest
.PHONY : RegressTest

Test :
	./bin$(VERSION)/$(CONFIGURATION)64/CS_TestCpp -d./Dictionaries -e -b TestCpp/TEST.DAT

QuickTest :
	./bin$(VERSION)/$(CONFIGURATION)64/CS_TestCpp -d./Dictionaries -e -b -t12345 TestCpp/TEST.DAT

RegressTest :
	./bin$(VERSION)/$(CONFIGURATION)64/CS_TestCpp -d./Dictionaries -e -b -tR TestCpp/CsMapRegressBase.csv



