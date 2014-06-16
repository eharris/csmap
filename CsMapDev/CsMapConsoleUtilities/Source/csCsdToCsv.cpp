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

#include <iomanip>
#include "csConsoleUtilities.hpp"

extern "C"
{
	extern char cs_Csname[];
	extern char cs_Dtname[];
	extern char cs_Elname[];
	extern char cs_Ctname[];
	extern char cs_Gxname[];
	extern char cs_Gpname[];

	extern char cs_Dir[];
	extern char cs_UserDir[];
	extern char* cs_DirP;

	extern char cs_Unique;
	extern char cs_DirsepC;
	extern short cs_Protect;
	extern int cs_Error;
}

extern "C" unsigned long KcsNmMapNoNumber;
extern "C" unsigned long KcsNmInvNumber;

// There are six functions in this module.  Each of these functions produce
// a version of the associated asc/csd files in .csv form.  This is done
// so that dictionaries can be examined (and possibly modified, but this is
// not currently planned) in an Excel environment where data can be easily
// sorted, grouped, and otherwise manipulated and the results eaily visible.
// The original intent here is to provide a basis for improving the EPSG
// number references in each dictionary.  This is of paticular interest
// in the case of the Geodetic Path and Geodetic Transformation dictionaries
// where the EPSG references shgould be to EPSG Paths and EPDG Coordinate
// Operations respectively.  Having a more EPDG number references in the
// Datum dictionary would also be helpful.
//
// The original purpose of improving EPSG reference coverage and accuracy
// is so that the useful range of a CRS can be extracted from the EPSG
// database.  While this has been done at the CRS level, what is expected
// is to be able use the datum, and hence the implied geodetic
// transformation, EPSG defintion to obtain a useful range definition for
// a CRS reference to such datum.
//
bool csCsdToCsvEL (const wchar_t* csDictDir,bool incLegacy)
{
	bool ok (false);

	int crypt;
	unsigned long epsgNbr;

	char *ccPtr;
	const wchar_t* wcPtr;

	double invFlattening;

	char csDictDirC [MAXPATH];
	wchar_t wcBuffer [MAXPATH + MAXPATH];

	std::wstring wcKeyName;
	std::wstring wcGroup;
	std::wstring wcDescription;
	std::wstring wcSource;
	std::wstring wcEpsgName;

	csFILE* csdStrm;
	std::wofstream csvStrm;

	struct cs_Eldef_ elDef;

	wcstombs (csDictDirC,csDictDir,sizeof (csDictDirC));
	int status = CS_altdr (csDictDirC);
	ok = (status == 0);
	if (!ok)
	{
		return ok;
	}

	csdStrm = NULL;
	ccPtr = CS_stncp (cs_DirP,cs_Elname,cs_FNM_MAXLEN);
	CS_strrpl (cs_Dir,MAXPATH,".CSD",".csv");
	csvStrm.open (cs_Dir,std::ios_base::out | std::ios_base::trunc);
	if (csvStrm.is_open ())
	{
		csdStrm = CS_elopn (_STRM_BINRD);
	}
	if (csdStrm != NULL)
	{
		// Getting here implies that both opens were successful.
		// Write a label line for our CSV file. There is no good
		// reason for wusing a wide character stream, currently.
		// But we do it anyway.
		csvStrm << L"Key Name"            << L','
				<< L"Group"               << L','
				<< L"Semi-Major"          << L','
				<< L"Semi-Minor"          << L','
				<< L"Flattening"          << L','
				<< L"Eccentricity"        << L','
				<< L"Descriptive Name"    << L','
				<< L"Source"              << L','
				<< L"Protect"             << L','
				<< L"EPSG Nbr"            << L','
				<< L"WKT Flavor"          << L','
				<< L"Inv Flattening"      << L','
				<< L"EccentricitySq"      << L','
				<< L"Mapper EPSG Name"    << L','
				<< L"Mapper EPSG Nbr"     << std::endl;
	
		// Loop through each definition in the ellipsoid dictionary,
		// and produce a .csv replica.
		while (CS_elrd (csdStrm,&elDef,&crypt))
		{
			// Skip this one if its deprecated, if so instructed.
			if (!incLegacy && (CS_stricmp (elDef.group,"LEGAC") == 0))
			{
				continue;
			}

			// Here once for each ellipsoid definition.  Simply write
			// a single csv record.  We quote only those fields which may
			// contain a special character or two.  The choice of using
			// wide characters makes fdome of this laborious.  Maybe that
			// was not a good choice.
			mbstowcs (wcBuffer,elDef.key_nm,wcCount (wcBuffer));
			wcKeyName = wcBuffer;
			csCsvQuoter (wcKeyName);

			mbstowcs (wcBuffer,elDef.group,wcCount (wcBuffer));
			wcGroup = wcBuffer;
			csCsvQuoter (wcGroup);

			mbstowcs (wcBuffer,elDef.name,wcCount (wcBuffer));
			wcDescription = wcBuffer;
			csCsvQuoter (wcDescription);

			mbstowcs (wcBuffer,elDef.source,wcCount (wcBuffer));
			wcSource = wcBuffer;
			csCsvQuoter (wcSource);

			wcPtr = csMapNameToName (csMapEllipsoidKeyName,csMapFlvrEpsg,
														   csMapFlvrAutodesk,
														   wcKeyName.c_str ());
			if (wcPtr != 0)
			{
				wcEpsgName = std::wstring (wcPtr);
				csCsvQuoter (wcEpsgName);
			}
			else
			{
				wcEpsgName = L"No EPSG association in name mapper.";
			}

			epsgNbr = csMapNameToIdC (csMapEllipsoidKeyName,csMapFlvrEpsg,
															csMapFlvrAutodesk,
															elDef.key_nm);
			if (epsgNbr == KcsNmInvNumber)
			{
				epsgNbr = 0UL;
			}

			if (elDef.flat == 0.0)
			{
				invFlattening = 0.0;
			}
			else
			{
				invFlattening = (1.0 / elDef.flat);
			}

			csvStrm << wcKeyName                      << L','
					<< wcGroup                        << L','
					<< std::fixed
					<< std::showpoint
					<< std::setprecision(3)
					<< elDef.e_rad                    << L','
					<< elDef.p_rad                    << L','
					<< std::setprecision(8)
					<< elDef.flat                     << L','
					<< elDef.ecent                    << L','
					<< wcDescription                  << L','
					<< wcSource                       << L','
					<< elDef.protect                  << L','
					<< elDef.epsgNbr                  << L','
					<< elDef.wktFlvr                  << L','
					<< std::setprecision (6)
					<< invFlattening                  << L','
					<< std::setprecision (8)
					<< (elDef.ecent * elDef.ecent)    << L','
					<< wcEpsgName                     << L','
					<< epsgNbr                        << std::endl;
		}
		csvStrm.close ();
		CS_fclose (csdStrm);
	}
	return ok;
}
bool csCsdToCsvDT (const wchar_t* csDictDir,bool incLegacy)
{
	bool ok (false);

	int crypt;
	unsigned long epsgNbr;

	char *ccPtr;
	const wchar_t* wcPtr;

	char csDictDirC [MAXPATH];
	wchar_t wcBuffer [MAXPATH + MAXPATH];

	std::wstring wcKeyName;
	std::wstring wcEllipsoid;
	std::wstring wcGroup;
	std::wstring wcLocation;
	std::wstring wcCountrySt;
	std::wstring wcDescription;
	std::wstring wcSource;
	std::wstring wcEpsgName;
	std::wstring wcMethod;

	csFILE* csdStrm;
	std::wofstream csvStrm;

	struct cs_Dtdef_ dtDef;

	wcstombs (csDictDirC,csDictDir,sizeof (csDictDirC));
	int status = CS_altdr (csDictDirC);
	ok = (status == 0);
	if (!ok)
	{
		return ok;
	}

	csdStrm = NULL;
	ccPtr = CS_stncp (cs_DirP,cs_Dtname,cs_FNM_MAXLEN);
	CS_strrpl (cs_Dir,MAXPATH,".CSD",".csv");
	csvStrm.open (cs_Dir,std::ios_base::out | std::ios_base::trunc);
	if (csvStrm.is_open ())
	{
		csdStrm = CS_dtopn (_STRM_BINRD);
	}
	if (csdStrm != NULL)
	{
		// Getting here implies that both opens were successful.
		// Write a label line for our CSV file. There is no good
		// reason for wusing a wide character stream, currently.
		// But we do it anyway.
		csvStrm << L"Key Name"            << L','
				<< L"Ellipsoid"           << L','
				<< L"Group"               << L','
				<< L"Location"            << L','
				<< L"Country/State"       << L','
				<< L"Delta X"             << L','
				<< L"Delta Y"             << L','
				<< L"Delta Z"             << L','
				<< L"Rotation X"          << L','
				<< L"Rotation Y"          << L','
				<< L"Rotation Z"          << L','
				<< L"Scale"               << L','
				<< L"Description"         << L','
				<< L"Source"              << L','
				<< L"Protect"             << L','
				<< L"Method"              << L','
				<< L"EPSG Nbr"            << L','
				<< L"WKT Flavor"          << L','
				<< L"Mapper EPSG Name"    << L','
				<< L"Mapper EPSG Nbr"     << std::endl;
	
		// Loop through each definition in the datum dictionary,
		// and produce a .csv replica.
		while (CS_dtrd (csdStrm,&dtDef,&crypt))
		{
			// Skip this one if its deprecated, if so instructed.
			if (!incLegacy && (CS_stricmp (dtDef.group,"LEGACY") == 0))
			{
				continue;
			}
			// Here once for each ellipsoid definition.  Simply write
			// a single csv record.  We quote only those fields which may
			// contain a special character or two.  The choice of using
			// wide characters makes fdome of this laborious.  Maybe that
			// was not a good choice.
			mbstowcs (wcBuffer,dtDef.key_nm,wcCount (wcBuffer));
			wcKeyName = wcBuffer;
			csCsvQuoter (wcKeyName);

			mbstowcs (wcBuffer,dtDef.ell_knm,wcCount (wcBuffer));
			wcEllipsoid = wcBuffer;
			csCsvQuoter (wcEllipsoid);

			mbstowcs (wcBuffer,dtDef.group,wcCount (wcBuffer));
			wcGroup = wcBuffer;
			csCsvQuoter (wcGroup);

			mbstowcs (wcBuffer,dtDef.locatn,wcCount (wcBuffer));
			wcLocation = wcBuffer;
			csCsvQuoter (wcLocation);

			mbstowcs (wcBuffer,dtDef.cntry_st,wcCount (wcBuffer));
			wcCountrySt = wcBuffer;
			csCsvQuoter (wcCountrySt);

			mbstowcs (wcBuffer,dtDef.name,wcCount (wcBuffer));
			wcDescription = wcBuffer;
			csCsvQuoter (wcDescription);

			mbstowcs (wcBuffer,dtDef.source,wcCount (wcBuffer));
			wcSource = wcBuffer;
			csCsvQuoter (wcSource);

			wcPtr = NULL;
			switch (dtDef.to84_via) {
			default:
			case cs_DTCTYP_NONE:
				wcPtr = L"None";
				break;
			case cs_DTCTYP_MOLO:
				wcPtr = L"Molodensky";
				break;
			case cs_DTCTYP_MREG:
				wcPtr = L"Multiple Regression";
				break;
			case cs_DTCTYP_BURS:
				wcPtr = L"Bursa/Wolf";
				break;
			case cs_DTCTYP_NAD27:
				wcPtr = L"NAD27";
				break;
			case cs_DTCTYP_NAD83:
				wcPtr = L"NAD83";
				break;
			case cs_DTCTYP_WGS84:
				wcPtr = L"WGS84";
				break;
			case cs_DTCTYP_WGS72:
				wcPtr = L"WGS72";
				break;
			case cs_DTCTYP_HPGN:
				wcPtr = L"HPGN";
				break;
			case cs_DTCTYP_7PARM:
				wcPtr = L"7 Parameter";
				break;
			case cs_DTCTYP_AGD66:
				wcPtr = L"AGD66";
				break;
			case cs_DTCTYP_3PARM:
				wcPtr = L"Three Parameter";
				break;
			case cs_DTCTYP_6PARM:
				wcPtr = L"Six Parameter";
				break;
			case cs_DTCTYP_4PARM:
				wcPtr = L"Four Parameter";
				break;
			case cs_DTCTYP_AGD84:
				wcPtr = L"AGD84";
				break;
			case cs_DTCTYP_NZGD49:
				wcPtr = L"NZGD49";
				break;
			case cs_DTCTYP_ATS77:
				wcPtr = L"ATS77";
				break;
			case cs_DTCTYP_GDA94:
				wcPtr = L"GDA94";
				break;
			case cs_DTCTYP_NZGD2K:
				wcPtr = L"NZGD2K";
				break;
			case cs_DTCTYP_CSRS:
				wcPtr = L"CSRS";
				break;
			case cs_DTCTYP_TOKYO:
				wcPtr = L"Tokyo";
				break;
			case cs_DTCTYP_RGF93:
				wcPtr = L"RGF93";
				break;
			case cs_DTCTYP_ED50:
				wcPtr = L"ED50";
				break;
			case cs_DTCTYP_DHDN:
				wcPtr = L"DHDN";
				break;
			case cs_DTCTYP_ETRF89:
				wcPtr = L"ETRF89";
				break;
			case cs_DTCTYP_GEOCTR:
				wcPtr = L"Geocentric";
				break;
			case cs_DTCTYP_CHENYX:
				wcPtr = L"ChenYX";
				break;
			}
			wcMethod = wcPtr;
			csCsvQuoter (wcMethod);

			wcPtr = csMapNameToName (csMapDatumKeyName,csMapFlvrEpsg,
													   csMapFlvrAutodesk,
													   wcKeyName.c_str ());
			if (wcPtr != 0)
			{
				wcEpsgName = std::wstring (wcPtr);
				csCsvQuoter (wcEpsgName);
			}
			else
			{
				wcEpsgName = L"No EPSG association in name mapper.";
			}

			epsgNbr = csMapNameToIdC (csMapDatumKeyName,csMapFlvrEpsg,
														csMapFlvrAutodesk,
														dtDef.key_nm);
			if (epsgNbr == KcsNmInvNumber)
			{
				epsgNbr = 0UL;
			}

			csvStrm << wcKeyName                      << L','
					<< wcEllipsoid                    << L','
					<< wcGroup                        << L','
					<< wcLocation                     << L','
					<< wcCountrySt                    << L','
					<< std::fixed
					<< std::showpoint
					<< std::setprecision(5)
					<< dtDef.delta_X                  << L','
					<< dtDef.delta_Y                  << L','
					<< dtDef.delta_Z                  << L','
					<< dtDef.rot_X                    << L','
					<< dtDef.rot_Y                    << L','
					<< dtDef.rot_Z                    << L','
					<< dtDef.bwscale                  << L','
					<< std::fixed
					<< std::noshowpoint
					<< std::setprecision(0)
					<< wcDescription                  << L','
					<< wcSource                       << L','
					<< dtDef.protect                  << L','
					<< wcMethod                       << L','
					<< dtDef.epsgNbr                  << L','
					<< dtDef.wktFlvr                  << L','
					<< wcEpsgName                     << L','
					<< epsgNbr                        << std::endl;
		}
		csvStrm.close ();
		CS_fclose (csdStrm);
	}
	return ok;
}
bool csCsdToCsvCS (const wchar_t* csDictDir,bool incLegacy)
{
	return false;
}
bool csCsdToCsvCT (const wchar_t* csDictDir,bool incLegacy)
{
	return false;
}
bool csCsdToCsvGX (const wchar_t* csDictDir,bool incLegacy)
{
	return false;
}
bool csCsdToCsvGP (const wchar_t* csDictDir,bool incLegacy)
{
	return false;
}
