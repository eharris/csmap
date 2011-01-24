Building CS-MAP on Windows and Linux

The CS-MAP distribution will produce a series of five directories:

Include: Contains all header files referenced the source code in the Source directory.

Source: Contains all the source code for the CS-MAP library itself.

Dictionaries: Contains the coordinate system dictionaries in source form, and the source code for a compiler which will convert the dictionary source to the operational binary form.

Test: Contains the source code for a console type test program and the test data which it uses.

Data: Contains a series of data files used to construct name mapping files.

Building the entire product is a series of five steps:
	1> Build the CS-MAP library.
	2> Build the dictionary compiler.
	3> Run the dictionary compiler.
	4> Build the console test program.
	5> Execute the console test program.

After installation, and before building, it will be best to obtain a copy of the Canadian National Transformation file (NTV2_0.gsb) and copy it to the Dictionaries/Canada directory.  This data file may not be distributed!!!  Geomatics Canada reserves the right to distribute this file and maintain a list of those using it.  Part of an ISO 9000 consideration.  Therefore, since we do not distribute the file as part of this open source distribution, we recommend strongly that you simply obtain a copy, even if only for testing purposes.

Chances are very good you already have a copy of this file on your system already.  If not, you can obtain one (no fee) at:

	http://www.geod.nrcan.gc.ca

You will need to inform the CS-MAP library of the existence of this file and where it is located.  In release 12 or earlier, this information is passed to the CS-MAP library via the "Nad27ToNad83.gdc" data file.  This is a simple text file which can be edited with any text editor.  Comments in the distribution version of this file are verbose and describe exactly what needs to be accomplished to activate access to the new data file.

In release 13 and thereafter, the existence and location of the NTV2_0.gsb file is conveyed to the CS-MAP library through one or more definitions in the Geodetic Transformation Dictionary.  Thus, you must modify the source file for this dictionary (GeodeticTransform.asc) and then recompile it using the dictionary compiler.  As of this writing, there is no UI or alternative means of modifying this information.

The TEST.DAT data file in the Test directory contains several hundred test points which are directly related to the above mentioned grid shift data file.  To prevent confusion and unnecessary technical support, these test points are commented out in the distribution.  After obtaining a copy of the above mentioned data file, these test should be uncommented back in, so that the test program will test this feature.

The situation described above concerning the Canadian National Transformation also applies to other sources of geodetic transformation data.  In these cases, it is not so much that we know we are not permitted to distribute the file, it is more that we are unable to determine that we are permitted to distribute the data file.  Thus, to be safe the files are not distributed.  At this writing, this situation applies to other CSRS related Canadian files and the Japanese Geodetic Datum of 2000 data file.  Check the appropriate locations in the distribution folder hierarchy for readme files which describe the situation for these locations and provides suggestions on how to obtain a copy of the file.

OK. Now for building on your system:

For Windows:

1> Build the CS-MAP Library:
	Make the 'Source' directory your current working directory.
	Use the MSVC set variables script to set the environmental variables correctly.
	Use the 'nmake' command and supply it with the 'Library.nmk' make file.  E.g.
		'nmake /fLibrary.nmk'
2> Build the Dictionary Compiler (CS_comp)
	Make the 'Dictionaries' directory your current working directory.
	Use the MSVC set variables script to set the environmental variables correctly.
	Use the 'nmake' command and supply it with the 'Compiler.nmk' make file.  E.g.
		'nmake /fCompiler.nmk'
3> Run the Dictionary Compiler
	Make the 'Dictionaries' directory your current working directory.
	Execute the 'CS_comp' program.  E.g.
		'CS_Comp . .'
	Note that the first argument is the directory containing the dictionary source, the
	second argument is the directory to which the binary dictionary files are written. 
4> Build the Console Test program (CS_Test)
	Make the 'Test' directory your current working directory.
	Use the MSVC set variables script to set the environmental variables correctly.
	Use the 'nmake' command and supply it with the 'Test.nmk' make file.  E.g.
		'nmake /fTest.nmk'
5> Execute the console test program
	Make the 'Test' directory your current working directory.
	Execute the 'CS_Test' program.  E.g.
		'CS_Test /d..\Dictionaries'
	Note that the /d argument is the directory which the test program is to look to
	for the dictionaries and related data files.

For Linux:

1> Build the CS-MAP Library:
	Make the 'Source' directory your current working directory.
	Use the 'make' command and supply it with the 'Library.mak' make file.  E.g.
		'make -fLibrary.mak'
2> Build the Dictionary Compiler (CS_Comp)
	Make the 'Dictionaries' directory your current working directory.
	Use the 'make' command and supply it with the 'Compiler.mak' make file.  E.g.
		'make -fCompiler.mak'
3> Run the Dictionary Compiler
	Make the 'Dictionaries' directory your current working directory.
	Execute the 'CS_Comp' program.  E.g.
		'./CS_Comp . .'
	Note that the first argument is the directory containing the dictionary source,
	the second argument is the directory to which the binary dictionary files are
	written. 
4> Build the Console Test program (CS_Test)
	Make the 'Test' directory your current working directory.
	Use the 'make' command and supply it with the 'Test.mak' make file.  E.g.
		'make -fTest.mak'
5> Execute the console test program
	Make the 'Test' directory your current working directory.
	Execute the 'CS_Test' program.  E.g.
		'./CS_Test -d../Dictionaries'
	Note that the /d argument is the directory which the test program is to look
	to for the dictionaries and related data files.

MS VC++ 2005 (Version 8):

The CS-MAP Open Source distribution will deposit in the primary directory a Microsoft Visual C++ Version 8.0 (VC2005) solution file.  This file references project files in the Source, Dictionaries, and Test directories.  This solution file and its related project files can be used to manufacture the library, dictionary compiler, and the test module.  No provisions have been made for executing the dictionary compiler or the test module.
