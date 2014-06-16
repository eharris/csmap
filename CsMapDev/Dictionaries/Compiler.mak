# This make file was been written with the EXPRESS requirement that it
# exists in the "CsMapDev/Dictionaries" directory and that directory is
# the current working directory of the make executable which is processing
# it; with the original intent being that this makefile would be invoked
# by a higher level make file which executes something like the following:
#
#	$(MAKE) -e -C./Dictionaries -fCompiler.mak
#
PRJ_NAME = Compiler
TRG_BASE = CS_Comp
#
# Set the following default values so that this makefile can be used at
# this level.
#
VERSION ?= 47
CONFIGURATION ?= Linux
PROCESSOR ?= x64
CSMAP_LIB_NAME ?= CsMap
#
# The following definitions are instituted to facilitate building and
# testing multiple versions of the Library. The variables referenced by
# these definitions are normally expected to be passed from a parent
# makefile.  The default values set here represent rather generic values
# which can be useful when in maintenance/development mode with respect
# to the compiler itself.
#
OUT_DIR ?= ../bin$(VERSION)/$(CONFIGURATION)
LIB_DIR ?= ../lib$(VERSION)/$(CONFIGURATION)
INT_DIR ?= ../obj$(VERSION)/$(PRJ_NAME)/$(CONFIGURATION)
DICTIONARY_SRC_DIR ?= ../Dictionaries
DICTIONARY_TRG_DIR ?= ../Dictionaries
INCLUDE_TEST ?= -t
#
# We use the following as a generic target which includes all of the
# dictionary files.
#
DICTIONARIES := $(DICTIONARY_TRG_DIR)/Coordsys.CSD \
				$(DICTIONARY_TRG_DIR)/Datums.CSD \
				$(DICTIONARY_TRG_DIR)/Elipsoid.CSD \
				$(DICTIONARY_TRG_DIR)/GeodeticTransform.CSD \
				$(DICTIONARY_TRG_DIR)/GeodeticPath.CSD \
				$(DICTIONARY_TRG_DIR)/Category.CSD

DICTIONARY_SRC := $(DICTIONARY_SRC_DIR)/coordsys.asc \
				  $(DICTIONARY_SRC_DIR)/datums.asc \
				  $(DICTIONARY_SRC_DIR)/elipsoid.asc \
				  $(DICTIONARY_SRC_DIR)/GeodeticTransformation.asc \
				  $(DICTIONARY_SRC_DIR)/GeodeticPath.asc \
				  $(DICTIONARY_SRC_DIR)/category.asc
#
# The following options are chosen to be rather generic; something that
# should work in any UNIX/Linux type environment.  More sophisticated
# specifications can/should be coded in the parent make file.
#
C_FLG ?= -c -w -O2 -I../Include
CXX_FLG ?= -c -w -O2 -I../Include
#
# Adjust the above defines for the various processors, currently only
# two: x86 (32 bits) and x64 (64 bit x86)
#
ifeq ($(PROCESSOR),x64)
	OUT_DIR := $(OUT_DIR)64
	INT_DIR := $(INT_DIR)64
	LIB_DIR := $(LIB_DIR)64
	C_FLG += -m64
	CXX_FLG += -m64
endif

ifeq ($(PROCESSOR),x86)
	OUT_DIR := $(OUT_DIR)32
	INT_DIR := $(INT_DIR)32
	LIB_DIR := $(LIB_DIR)32
	C_FLG += -m32
	CXX_FLG += -m32
endif

#
# Define the targets of this make file.
#
ALL : $(OUT_DIR)/$(TRG_BASE) $(DICTIONARIES)

$(INT_DIR)/$(TRG_BASE).o : CS_COMP.c
	$(CC) $(C_FLG) -o $(INT_DIR)/$(TRG_BASE).o CS_COMP.c

$(OUT_DIR)/$(TRG_BASE) : $(INT_DIR)/$(TRG_BASE).o $(LIB_DIR)/$(CSMAP_LIB_NAME).a
	gcc -I../Include -o $(OUT_DIR)/$(TRG_BASE) $(INT_DIR)/$(TRG_BASE).o $(LIB_DIR)/$(CSMAP_LIB_NAME).a -lm -lc -lgcc -lstdc++

$(DICTIONARIES) :: $(DICTIONARY_SRC)
	$(OUT_DIR)/$(TRG_BASE) -b $(INCLUDE_TEST) $(DICTIONARY_SRC_DIR) $(DICTIONARY_TRG_DIR)

$(LIB_DIR)/$(CSMAP_LIB_NAME).a :
	$(MAKE) -e -C ../Source -f Library.mak

.PHONY : clean
clean :
	rm -f $(INT_DIR)/*.o
	rm -f $(OUT_DIR)/$(TRG_BASE)
	rm -f $(DICTIONARIES)

rebuild: clean $(OUT_DIR)/$(TRG_BASE) $(DICTIONARIES)

$(INT_DIR)/$(TRG_BASE).o : | $(INT_DIR)

$(OUT_DIR)/$(TRG_BASE) : | $(OUT_DIR)

$(INT_DIR) :
	mkdir -p $(INT_DIR)

$(OUT_DIR) :
	mkdir -p $(OUT_DIR)
