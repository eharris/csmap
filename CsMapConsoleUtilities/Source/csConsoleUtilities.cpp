//===========================================================================
// $Header$
//
//    (C) Copyright 2007 by Autodesk, Inc.
//
// The information contained herein is confidential, proprietary
// to Autodesk, Inc., and considered a trade secret as defined 
// in section 499C of the penal code of the State of California.  
// Use of this information by anyone other than authorized employees
// of Autodesk, Inc. is granted only under a written non-disclosure 
// agreement, expressly prescribing the scope and manner of such use.       
//
// CREATED BY:
//      Norm Olsen
//
// DESCRIPTION:
//

#include "csConsoleUtilities.hpp"

const wchar_t csTempDir [] = L"C:\\TEMP";
wchar_t csEpsgDir [] = L"C:\\ProgramData\\GeodeticData\\EPSG\\EPSG-v7_05\\CSV";
const wchar_t csDataDir [] = L"C:\\Development\\SVN\\MgDev\\Oem\\CsMap\\Data";
const wchar_t csDictDir [] = L"C:\\Development\\SVN\\MgDev\\Oem\\CsMap\\Dictionaries";
const wchar_t csDictSrc [] = L"C:\\Development\\SVN\\MgDev\\Oem\\CsMap\\Dictionaries";

int main (int argc,char* argv [])
{
	bool ok;
	char pathName [512];

	//ok = csGenerateHpgnTable (csTempDir,csDictDir);

	ok = csOrgTransformations (csDictSrc,csDictDir);

	// ok = csAddEpsgCodes (csDictSrc,csEpsgDir,csTempDir);

	// Manufacture NameMapper.csv
	// ok = AddSequenceNumbers (csDataDir);
	// ok = ManufactureNameMapperCsv (csDictDir,csDataDir);

	// Generate a list of EPSG codes, EPSG descriptions,
	// and Autodesk/Mentor names.
	//ok = ListUnmappedEpsgCodes (csEpsgDir,csDictDir);

	// Add Oracle, Release 9, Name mappings to original table.
	// Should never need this again.
	//ok = AddOracle9Mappings (csDataDir,status);

	// Add Oracle, Release 10, Name mappings to original table.
	// Should never need this again.
	//TcsCsvStatus status;
	//ok = AddOracle10Mappings (csDataDir,status);

	// Replace Internal ID numbers in .csv tables with sequence numbers
	// Should never need this again.
	// ok = AddSequenceNumbers (csDataDir);

	// Generate a list of EPSG codes, EPSG descriptions,
	// and Autodesk/Mentor names.
	// ok = ListEpsgCodes (csDictDir);

	// Three Parameter Fix
	//		char pathName [512];
	//wcstombs (pathName,csDictSrc,sizeof (pathName));
	//ok = ThreeParameterFixer (pathName,"C:\\TEMP");
	//if (ok)
	//{
	//	ok = ManufactureNameMapperCsv (csTempDir);
	//}

	// Geocentric Fix
	//wcstombs (pathName,csDictSrc,sizeof (pathName));
	//ok = GeocentricFixer (pathName,"C:\\TEMP");

	// Deprecate duplicate CRS definitions.
	// ok = ListDuplicateDefinitions (L"C:\\TEMP\\DuplicateList.cpp",csDictDir);
	// ok = DeprecateDupliateDefs (csDictDir,csTempDir);

	return ok?0:-1;
}
