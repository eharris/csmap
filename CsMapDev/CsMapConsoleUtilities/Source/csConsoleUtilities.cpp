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

#if (_RUN_TIME < _rt_UNIXPCC)
wchar_t csDataDir [MAXPATH] = L"%OPEN_SOURCE%\\MetaCrs\\CsMap\\trunk\\CsMapDev\\Data";
wchar_t csDictDir [MAXPATH] = L"%OPEN_SOURCE%\\MetaCrs\\CsMap\\trunk\\CsMapDev\\Dictionaries";
wchar_t csDictSrc [MAXPATH] = L"%OPEN_SOURCE%\\MetaCrs\\CsMap\\trunk\\CsMapDev\\Dictionaries";
wchar_t csEpsgDir [MAXPATH] = L"%GEODETIC_DATA%\\EPSG\\CSV";
wchar_t csEpsgPolyDir [] = L"%GEODETIC_DATA%\\EPSG\\EPSG-Polygon-package-20130626";
wchar_t csTempDir [MAXPATH] = L"C:\\TEMP";
#else
const wchar_t csDataDir [] = L"$OSGEO/CsMap/MetaCrs/CsMap/trunk/CsMapDev/Data";
const char csDictDir [] = "$OSGEO/CsMap/MetaCrs/CsMap/trunk/CsMapDev/Dictionaries";
wchar_t csEpsgDir [] = L"${GeodeticData}/Epsg/CSV";
const wchar_t csDataDir [] = L"$OSGEO/CsMap/MetaCrs/CsMap/trunk/CsMapDev/Data";
const wchar_t csTempDir [] = L"/usr/tmp";
#endif

int main (int argc,char* argv [])
{
	bool ok (false);
	int envStatus;

#if defined (_MSC_VER) && _MSC_VER >= 1400 && _MSC_VER < 1900
	// This is a Microsoft specific function call.  It forces the exponential
	// printf format to two digits, which I prefer.  Maybe there is a more
	// generic form of this, but I don't know about it.
	_set_output_format(_TWO_DIGIT_EXPONENT);
#	ifdef _DEBUG
		_CrtSetDbgFlag (_CRTDBG_CHECK_DEFAULT_DF);
#	endif
#endif

	// Perform environmental variable substitution on the global variables
	// defined above which specify the location of stuff on the host system.
	// The loops are required as the CS_envsubWc function only replaces a
	// single environmental variable per call.
	for (envStatus = 1;envStatus != 0;)
	{
		envStatus = CS_envsubWc (csDataDir,wcCount (csDataDir));
	}
	for (envStatus = 1;envStatus != 0;)
	{
		envStatus = CS_envsubWc (csDictDir,wcCount (csDictDir));
	}
	for (envStatus = 1;envStatus != 0;)
	{
		envStatus = CS_envsubWc (csDictSrc,wcCount (csDictSrc));
	}
	for (envStatus = 1;envStatus != 0;)
	{
		envStatus = CS_envsubWc (csEpsgDir,wcCount (csEpsgDir));
	}
	for (envStatus = 1;envStatus != 0;)
	{
		envStatus = CS_envsubWc (csTempDir,wcCount (csTempDir));
	}

#ifdef __SKIP__
	// The following untility is a frequently used one.  We leave here,
	// but comment out so it can be used with ease.
	//
	// Resort a manually edited NameMapper.csv file to standard order.  Also,
	// this utility will match the quoting in the sorted  data file as
	// maintained in SVN.  This feature is often required so that a "diff"
	// between old and manually edited (especially if you use Excel to do the
	// editing) will produce usable results.
	//
	// ok = ResortNameMapperCsv (csTempDir,csDictSrc,true);
	//
	// Note that the Resort utility will overwrite the source file if the same
	// directory is used for the first two parameters.  Thus, to avoid losing
	// the results of a painful editing session, we leave the controlled source
	// to point to the temporary directory as the result directory.  It is
	// suggested that this only be changed on a temporary basis.
	return ok;
#endif

	// The following utility will programmatically add the Indiana GCS systems
	// to the normal dictionaries based on a file named InGCS.csv expected to
	// reside in the CS-MAP Data directory, and put the modified results
	// (i.e. coordsys.asc and NameMapper.csv) in the C:\Temp directory until
	// such time as we feel confident in the results being produced.
	ok = csAddInGCS (csTempDir,csDataDir,L"InGCS.csv",csDictSrc,csEpsgDir);

	return ok?0:-1;
}
