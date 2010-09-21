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

extern "C" unsigned long KcsNmInvNumber;
extern "C" unsigned long KcsNmMapNoNumber;
extern "C" double cs_Zero;

bool csWriteTransformationAsc (std::wofstream& gtStrm,std::wofstream& gpStrm,
													  const cs_Dtdef_* dtDefPtr,
													  const TcsDefFile& mregAsc,
													  const char* dtmPivot);
bool csAdjustSrcAndTrg (std::wofstream& gpStrm,char *srcDatum,char *trgDatum,
															  const cs_Dtdef_* dtDefPtr,
															  const char* dtmPivot);
bool csWriteGeodeticPath (std::wofstream& gpStrm,const cs_GeodeticPath_ *gpPtr);
bool csConvertMrtFile (std::wofstream& gtStrm,const cs_Dtdef_ *dtDefPtr,const TcsDefFile& mregAsc);

const TcsGdcFile* gdcAgd66ToGda94 (0);
const TcsGdcFile* gdcAgd84ToGda94 (0);
const TcsGdcFile* gdcAts77ToCsrs (0);
const TcsGdcFile* gdcCh1903ToPlus (0);
const TcsGdcFile* gdcDhdnToEtrf89 (0);
const TcsGdcFile* gdcEd50ToEtrf89 (0);
const TcsGdcFile* gdcNad27ToAts77 (0);
const TcsGdcFile* gdcNad27ToCsrs (0);
const TcsGdcFile* gdcNad27ToNad83 (0);
const TcsGdcFile* gdcNad83ToCsrs (0);
const TcsGdcFile* gdcNad83ToHarn (0);
const TcsGdcFile* gdcNzgd49ToNzgd2K (0);
const TcsGdcFile* gdcRgf93ToNtf (0);
const TcsGdcFile* gdcTokyoToJgd2k (0);

const cs_Dtdef_ cs_Chrts95DtDef =
{
	"CH1903Plus_1",		/* key_nm */
	"BESSEL",			/* ell_knm */
	"",					/* group */
	"",					/* locatn */
	"",					/* cntry_st */
	"",					/* fill */
	674.374,			/* DeltaX */
	15.056,				/* DeltaY */
	405.346,			/* DeltaZ */
	0.0,
	0.0,
	0.0,
	0.0,
	"CH1903+ to CHTRF95 (Swiss Terrestrial Reference Frame 1995)", 	/* name */
	"",					/* source */
	0,					/* protect */
	cs_DTCTYP_GEOCTR,	/* to84_via */ 
	6151,				/* epsgNbr */
	0,					/* wktFlvr */
	0,					/* fill01 */
	0,					/* fill02 */
	0,					/* fill03 */
	0,					/* fill04 */
};

bool csOrgTransformations (const wchar_t* csDictDir,const wchar_t* csDictTrgDir)
{
	bool ok (false);

	char csDtmDictFilePath [MAXPATH];
	char csTrgGxFilePath [MAXPATH];
	char csSrcMregFilePath [MAXPATH];

	// Initialize CS-MAP, need this for the NameMapper.
	wcstombs (csDtmDictFilePath,csDictDir,sizeof (csDtmDictFilePath));
	int st = CS_altdr (csDtmDictFilePath);
	if (st != 0)
	{
		return ok;
	}

	// Open up the Datum Dictionary file.
	TcsDatumsFile datumDict;

	// Open up the mreg source file.  We'll need that for some things
	// that in the source, but not the resulting mrt files.
	wcstombs (csSrcMregFilePath,csDictDir,sizeof (csSrcMregFilePath));
	strcat (csSrcMregFilePath,"\\mreg.asc");
	TcsDefFile mregAsc (dictTypMreg,csSrcMregFilePath);

	// Open up the gdc files which we will probably need to access.
	gdcAgd66ToGda94 = new TcsGdcFile ("Agd66ToGda94.gdc");
	gdcAgd84ToGda94 = new TcsGdcFile ("Agd84ToGda94.gdc");
	gdcAts77ToCsrs = new TcsGdcFile ("Ats77ToCsrs.gdc");
	gdcCh1903ToPlus = new TcsGdcFile ("Ch1903ToPlus.gdc");
	gdcDhdnToEtrf89 = new TcsGdcFile ("DhdnToEtrf89.gdc");
	gdcEd50ToEtrf89 = new TcsGdcFile ("Ed50ToEtrf89.gdc");
	gdcNad27ToAts77 = new TcsGdcFile ("Nad27ToAts77.gdc");
	gdcNad27ToCsrs = new TcsGdcFile ("Nad27ToCsrs.gdc");
	gdcNad27ToNad83 = new TcsGdcFile ("Nad27ToNad83.gdc");
	gdcNad83ToCsrs = new TcsGdcFile ("Nad83ToCsrs.gdc");
	gdcNad83ToHarn = new TcsGdcFile ("Nad83ToHarn.gdc");
	gdcNzgd49ToNzgd2K = new TcsGdcFile ("Nzgd49ToNzgd2K.gdc");
	gdcRgf93ToNtf = new TcsGdcFile ("Rgf93ToNtf.gdc");
	gdcTokyoToJgd2k = new TcsGdcFile ("TokyoToJgd2k.gdc");

	// Create the geodetic transformation output stream we
	// will use.
	wcstombs (csTrgGxFilePath,csDictTrgDir,sizeof (csTrgGxFilePath));
	strcat (csTrgGxFilePath,"\\GeodeticTransformation.asc");
	std::wofstream gtStrm (csTrgGxFilePath,std::ios_base::out | std::ios_base::trunc);

	// We will also need to write the initial Geodetic Path
	// dictionary.
	wcstombs (csTrgGxFilePath,csDictTrgDir,sizeof (csTrgGxFilePath));
	strcat (csTrgGxFilePath,"\\GeodeticPath.asc");
	std::wofstream gpStrm (csTrgGxFilePath,std::ios_base::out | std::ios_base::trunc);

	if (gtStrm.is_open () && gpStrm.is_open ())
	{
		size_t index;
		size_t dtmCount;

		dtmCount = datumDict.GetRecordCount ();
		// Two passes.  First pass, we skip all the LEGACY and 3PARAMETER guys.
		// We add only them on the second pass.
		ok = true;
		for (index = 0;ok && index < dtmCount;index += 1)
		{
			const cs_Dtdef_* dtDefPtr;
			dtDefPtr = datumDict.FetchDatum (index);
			if (!CS_stricmp (dtDefPtr->group,"LEGACY") ||
			     dtDefPtr->to84_via == cs_DTCTYP_3PARM)
			{
				continue;
			}
			ok = csWriteTransformationAsc (gtStrm,gpStrm,dtDefPtr,mregAsc,"WGS84");
			
			if (ok && !CS_stricmp (dtDefPtr->key_nm,"CHTRF95"))
			{
				ok = csWriteTransformationAsc (gtStrm,gpStrm,dtDefPtr,mregAsc,"ETRF89");
			}

			if (ok && !CS_stricmp (dtDefPtr->key_nm,"CH1903Plus_1"))
			{
				ok = csWriteTransformationAsc (gtStrm,gpStrm,dtDefPtr,mregAsc,"CHTRF95");
			}
		}

		for (index = 0;ok && index < dtmCount;index += 1)
		{
			const cs_Dtdef_* dtDefPtr;
			dtDefPtr = datumDict.FetchDatum (index);
			if (!CS_stricmp (dtDefPtr->group,"LEGACY") ||
			     dtDefPtr->to84_via == cs_DTCTYP_3PARM)
			{
				ok = csWriteTransformationAsc (gtStrm,gpStrm,dtDefPtr,mregAsc,"WGS84");
			}
		}
		gtStrm.close ();
		gpStrm.close ();
	}
	return ok;
}
bool csWriteTransformationAsc (std::wofstream& gtStrm,std::wofstream& gpStrm,
													  const cs_Dtdef_ *dtDefPtr,
													  const TcsDefFile& mregAsc,
													  const char* dtmPivot)
{
	bool ok (true);

	short maxIteration;
	

	size_t count;
	size_t index;

	char *cp;
	const char* entryPath;
	const TcsGdcEntry* gdcEntryPtr;

	double accuracy;
	double cnvrgValue;
	double errorValue;

	char srcDatum [cs_KEYNM_DEF];
	char trgDatum [cs_KEYNM_DEF];
	char cTemp [512];

	wchar_t gxName [64];

	CS_stncp (srcDatum,dtDefPtr->key_nm,sizeof (srcDatum));
	CS_stncp (trgDatum,dtmPivot,sizeof (trgDatum));

	// Adjust the source and target datums based on the existing to84_via
	// value, and if such adjustment is made, need to write a path to the
	// path stream.
	csAdjustSrcAndTrg (gpStrm,srcDatum,trgDatum,dtDefPtr,dtmPivot);

	// Set the maxIteration and convergence values per the to84_via variable.
	// The values were hard coded in CS-MAP prior to this implementation. */
	switch (dtDefPtr->to84_via) {
	case cs_DTCTYP_MOLO:
	case cs_DTCTYP_GEOCTR:
	case cs_DTCTYP_3PARM:
	case cs_DTCTYP_4PARM:
	case cs_DTCTYP_6PARM:
	case cs_DTCTYP_7PARM:
	case cs_DTCTYP_BURS:
	case cs_DTCTYP_WGS72:
	case cs_DTCTYP_MREG:
		maxIteration = 9;
		cnvrgValue   = 5.0E-12;
		errorValue   = 5.0E-07;
		break;

	case cs_DTCTYP_GDA94:
	case cs_DTCTYP_NZGD2K:
	case cs_DTCTYP_ETRF89:
	case cs_DTCTYP_NAD83:
	case cs_DTCTYP_WGS84:
		maxIteration = 9;
		cnvrgValue   = 5.0E-12;
		errorValue   = 5.0E-08;
		break;
	case cs_DTCTYP_NAD27:
		maxIteration = 10;
		cnvrgValue   = 1.0E-11;
		errorValue   = 5.0E-08;
		break;
	case cs_DTCTYP_AGD66:
	case cs_DTCTYP_AGD84:
	case cs_DTCTYP_NZGD49:
	case cs_DTCTYP_CSRS:
	case cs_DTCTYP_ATS77:
	case cs_DTCTYP_ED50:
	case cs_DTCTYP_DHDN:
	case cs_DTCTYP_CHENYX:
		maxIteration = 10;
		cnvrgValue   = 1.0E-09;
		errorValue   = 5.0E-08;
		break;
	case cs_DTCTYP_RGF93:
		maxIteration = 20;
		cnvrgValue   = 1.0E-09;
		errorValue   = 1.0E-06;
		break;
	case cs_DTCTYP_TOKYO:
		maxIteration = 10;
		cnvrgValue   = 1.0E-09;
		errorValue   = 1.0E-06;
		break;
	case cs_DTCTYP_HPGN:
		maxIteration = 10;
		cnvrgValue   = 1.0E-09;
		errorValue   = 5.0E-08;
		break;
	default:
		maxIteration = 10;
		cnvrgValue   = 1.0E-11;
		errorValue   = 5.0E-08;
	}

	// OK, the rest of this is pretty straight forward.
	accuracy = cs_Zero;
	swprintf (gxName,L"%S_to_%S",srcDatum,trgDatum);

	gtStrm << std::endl;
	gtStrm << L"GX_NAME: "      << gxName                       << std::endl;
	gtStrm <<   L"\t SRC_DTM: " << srcDatum                     << std::endl;
	gtStrm <<   L"\t TRG_DTM: " << trgDatum                     << std::endl;
	if (dtDefPtr->group [0] != '\0')
	{
		gtStrm << L"\t   GROUP: "  << dtDefPtr->group           << std::endl;
	}
	gtStrm <<   L"\t DESC_NM: " << dtDefPtr->name               << std::endl;
	gtStrm <<   L"\t  SOURCE: " << dtDefPtr->source             << std::endl;
	gtStrm <<   L"\tEPSG_NBR: " << dtDefPtr->epsgNbr            << std::endl;
	gtStrm <<   L"\t INVERSE: " << L"Yes"                       << std::endl;
	gtStrm <<   L"\t MAX_ITR: " << maxIteration                 << std::endl;
	gtStrm << L"   CNVRG_VAL: " << cnvrgValue                   << std::endl;
	gtStrm << L"   ERROR_VAL: " << errorValue                   << std::endl;
	// To Do, get accuracy from EPSG if we have an EPSG number.
	// Implies converting the Datum ID to a GEOGRAPHIC ID, to an
	// Operation ID, and then to an accuracy value.
	if (dtDefPtr->epsgNbr != 0)
	{
	}
	else
	{
		switch (dtDefPtr->to84_via) {
		case cs_DTCTYP_MOLO:
		case cs_DTCTYP_GEOCTR:
		case cs_DTCTYP_3PARM:
		case cs_DTCTYP_4PARM:
			accuracy = 8.0;
			break;
		case cs_DTCTYP_BURS:
		case cs_DTCTYP_6PARM:
			accuracy = 5.0;
			break;
		case cs_DTCTYP_WGS72:
		case cs_DTCTYP_7PARM:
		case cs_DTCTYP_MREG:
			accuracy = 3.0;
			break;
		case cs_DTCTYP_NAD27:
		case cs_DTCTYP_AGD66:
		case cs_DTCTYP_AGD84:
		case cs_DTCTYP_NZGD49:
		case cs_DTCTYP_ATS77:
		case cs_DTCTYP_TOKYO:
		case cs_DTCTYP_RGF93:
		case cs_DTCTYP_ED50:
		case cs_DTCTYP_DHDN:
		case cs_DTCTYP_CHENYX:
			accuracy = 1.0;
			break;
		case cs_DTCTYP_HPGN:
		case cs_DTCTYP_CSRS:
			accuracy = 0.5;
			break;
		case cs_DTCTYP_GDA94:
		case cs_DTCTYP_NZGD2K:
		case cs_DTCTYP_ETRF89:
		case cs_DTCTYP_NAD83:
		case cs_DTCTYP_WGS84:
			accuracy = 0.5;
			break;
		case cs_DTCTYP_NONE:
		default:
			accuracy = 0.0;
			break;
		}
	}
	if (accuracy > 0.0)
	{
		// TODO, need some formatting here.
		gtStrm <<   L"\tACCURACY: " << accuracy               << std::endl;
	}

	switch (dtDefPtr->to84_via) {
	case cs_DTCTYP_MOLO:
		gtStrm <<   L"\t  METHOD: " << L"MOLODENSKY"            << std::endl;
		gtStrm <<   L"\t DELTA_X: " << dtDefPtr->delta_X        << std::endl;
		gtStrm <<   L"\t DELTA_Y: " << dtDefPtr->delta_Y        << std::endl;
		gtStrm <<   L"\t DELTA_Z: " << dtDefPtr->delta_Z        << std::endl;
		break;
	case cs_DTCTYP_3PARM:
		gtStrm <<   L"\t  METHOD: " << L"3PARAMETER"            << std::endl;
		gtStrm <<   L"\t DELTA_X: " << dtDefPtr->delta_X        << std::endl;
		gtStrm <<   L"\t DELTA_Y: " << dtDefPtr->delta_Y        << std::endl;
		gtStrm <<   L"\t DELTA_Z: " << dtDefPtr->delta_Z        << std::endl;
		break;
	case cs_DTCTYP_GEOCTR:
		gtStrm <<   L"\t  METHOD: " << L"GEOCENTRIC"            << std::endl;
		gtStrm <<   L"\t DELTA_X: " << dtDefPtr->delta_X        << std::endl;
		gtStrm <<   L"\t DELTA_Y: " << dtDefPtr->delta_Y        << std::endl;
		gtStrm <<   L"\t DELTA_Z: " << dtDefPtr->delta_Z        << std::endl;
		break;
	case cs_DTCTYP_4PARM:
		gtStrm <<   L"\t  METHOD: " << L"4PARAMETER"            << std::endl;
		gtStrm <<   L"\t DELTA_X: " << dtDefPtr->delta_X        << std::endl;
		gtStrm <<   L"\t DELTA_Y: " << dtDefPtr->delta_Y        << std::endl;
		gtStrm <<   L"\t DELTA_Z: " << dtDefPtr->delta_Z        << std::endl;
		gtStrm <<   L"\t BWSCALE: " << dtDefPtr->bwscale        << std::endl;
		break;
	case cs_DTCTYP_6PARM:
		gtStrm <<   L"\t  METHOD: " << L"6PARAMETER"            << std::endl;
		gtStrm <<   L"\t DELTA_X: " << dtDefPtr->delta_X        << std::endl;
		gtStrm <<   L"\t DELTA_Y: " << dtDefPtr->delta_Y        << std::endl;
		gtStrm <<   L"\t DELTA_Z: " << dtDefPtr->delta_Z        << std::endl;
		gtStrm <<   L"\t   ROT_X: " << dtDefPtr->rot_X          << std::endl;
		gtStrm <<   L"\t   ROT_Y: " << dtDefPtr->rot_Y          << std::endl;
		gtStrm <<   L"\t   ROT_Z: " << dtDefPtr->rot_Z          << std::endl;
		break;
	case cs_DTCTYP_BURS:
		gtStrm <<   L"\t  METHOD: " << L"BURSAWOLF"             << std::endl;
		gtStrm <<   L"\t DELTA_X: " << dtDefPtr->delta_X        << std::endl;
		gtStrm <<   L"\t DELTA_Y: " << dtDefPtr->delta_Y        << std::endl;
		gtStrm <<   L"\t DELTA_Z: " << dtDefPtr->delta_Z        << std::endl;
		gtStrm <<   L"\t   ROT_X: " << dtDefPtr->rot_X          << std::endl;
		gtStrm <<   L"\t   ROT_Y: " << dtDefPtr->rot_Y          << std::endl;
		gtStrm <<   L"\t   ROT_Z: " << dtDefPtr->rot_Z          << std::endl;
		gtStrm <<   L"\t BWSCALE: " << dtDefPtr->bwscale        << std::endl;
		break;
	case cs_DTCTYP_7PARM:
		gtStrm <<   L"\t  METHOD: " << L"7PARAMETER"            << std::endl;
		gtStrm <<   L"\t DELTA_X: " << dtDefPtr->delta_X        << std::endl;
		gtStrm <<   L"\t DELTA_Y: " << dtDefPtr->delta_Y        << std::endl;
		gtStrm <<   L"\t DELTA_Z: " << dtDefPtr->delta_Z        << std::endl;
		gtStrm <<   L"\t   ROT_X: " << dtDefPtr->rot_X          << std::endl;
		gtStrm <<   L"\t   ROT_Y: " << dtDefPtr->rot_Y          << std::endl;
		gtStrm <<   L"\t   ROT_Z: " << dtDefPtr->rot_Z          << std::endl;
		gtStrm <<   L"\t BWSCALE: " << dtDefPtr->bwscale        << std::endl;
		break;
	case cs_DTCTYP_MREG:
		gtStrm <<   L"\t  METHOD: " << L"MULREG"                << std::endl;
		ok = csConvertMrtFile (gtStrm,dtDefPtr,mregAsc);
		break;
	case cs_DTCTYP_NAD27:
		gtStrm <<   L"\t  METHOD: " << L"GRID_INTERP"             << std::endl;
		gtStrm << L"#  GRID_FILE: NTv2,FWD,.\\Canada\\NTV2_0.GSB         #v2 of Canadian National Transformation" << std::endl;
		count = gdcNad27ToNad83->GetEntryCount ();
		for (index = 0;index < count;index += 1)
		{
			gdcEntryPtr = gdcNad27ToNad83->GetEntryPtr (index);
			entryPath = gdcEntryPtr->GetEntryPath ();
			CS_stncp (cTemp,entryPath,sizeof (cTemp));
			gdcNad27ToNad83->ConvertToRelative (cTemp);
			gtStrm << L"   GRID_FILE: " << L"NADCON,FWD," << cTemp  << std::endl;
		}
		break;
	case cs_DTCTYP_NAD83:
		gtStrm <<   L"\t  METHOD: " << L"NULL"                  << std::endl;
		break;
	case cs_DTCTYP_WGS84:
		gtStrm <<   L"\t  METHOD: " << L"NULL"                  << std::endl;
		break;
	case cs_DTCTYP_WGS72:
		gtStrm <<   L"\t  METHOD: " << L"WGS72"                 << std::endl;
		break;
	case cs_DTCTYP_HPGN:
		gtStrm <<   L"\t  METHOD: " << L"GRID_INTERP"             << std::endl;
		count = gdcNad83ToHarn->GetEntryCount ();
		for (index = 0;index < count;index += 1)
		{
			gdcEntryPtr = gdcNad83ToHarn->GetEntryPtr (index);
			entryPath = gdcEntryPtr->GetEntryPath ();
			CS_stncp (cTemp,entryPath,sizeof (cTemp));
			gdcNad83ToHarn->ConvertToRelative (cTemp);
			gtStrm << L"   GRID_FILE: " << L"NADCON,FWD," << cTemp  << std::endl;
		}
		break;
	case cs_DTCTYP_AGD66:
		gtStrm <<   L"\t  METHOD: " << L"GRID_INTERP"             << std::endl;
		count = gdcAgd66ToGda94->GetEntryCount ();
		for (index = 0;index < count;index += 1)
		{
			gdcEntryPtr = gdcAgd66ToGda94->GetEntryPtr (index);
			entryPath = gdcEntryPtr->GetEntryPath ();
			CS_stncp (cTemp,entryPath,sizeof (cTemp));
			gdcAgd66ToGda94->ConvertToRelative (cTemp);
			gtStrm << L"   GRID_FILE: " << L"NTv2,FWD," << cTemp  << std::endl;
		}
		break;
	case cs_DTCTYP_AGD84:
		gtStrm <<   L"\t  METHOD: " << L"GRID_INTERP"             << std::endl;
		count = gdcAgd84ToGda94->GetEntryCount ();
		for (index = 0;index < count;index += 1)
		{
			gdcEntryPtr = gdcAgd84ToGda94->GetEntryPtr (index);
			entryPath = gdcEntryPtr->GetEntryPath ();
			CS_stncp (cTemp,entryPath,sizeof (cTemp));
			gdcAgd84ToGda94->ConvertToRelative (cTemp);
			gtStrm << L"   GRID_FILE: " << L"NTv2,FWD," << cTemp  << std::endl;
		}
		break;
	case cs_DTCTYP_NZGD49:
		gtStrm <<   L"\t  METHOD: " << L"GRID_INTERP"             << std::endl;
		count = gdcNzgd49ToNzgd2K->GetEntryCount ();
		for (index = 0;index < count;index += 1)
		{
			gdcEntryPtr = gdcNzgd49ToNzgd2K->GetEntryPtr (index);
			entryPath = gdcEntryPtr->GetEntryPath ();
			CS_stncp (cTemp,entryPath,sizeof (cTemp));
			gdcNzgd49ToNzgd2K->ConvertToRelative (cTemp);
			gtStrm << L"   GRID_FILE: " << L"NTv2,FWD," << cTemp  << std::endl;
		}
		break;
	case cs_DTCTYP_ATS77:
		gtStrm <<   L"\t  METHOD: " << L"GRID_INTERP"             << std::endl;
		count = gdcAts77ToCsrs->GetEntryCount ();
		for (index = 0;index < count;index += 1)
		{
			gdcEntryPtr = gdcAts77ToCsrs->GetEntryPtr (index);
			entryPath = gdcEntryPtr->GetEntryPath ();
			CS_stncp (cTemp,entryPath,sizeof (cTemp));
			gdcAts77ToCsrs->ConvertToRelative (cTemp);
			gtStrm << L"   GRID_FILE: " << L"NTv2,FWD," << cTemp  << std::endl;
		}
		break;
	case cs_DTCTYP_GDA94:
		gtStrm <<   L"\t  METHOD: " << L"NULL"                  << std::endl;
		break;
	case cs_DTCTYP_NZGD2K:
		gtStrm <<   L"\t  METHOD: " << L"NULL"                  << std::endl;
		break;
	case cs_DTCTYP_CSRS:
		gtStrm <<   L"\t  METHOD: " << L"GRID_INTERP"             << std::endl;
		count = gdcNad83ToCsrs->GetEntryCount ();
		for (index = 0;index < count;index += 1)
		{
			gdcEntryPtr = gdcNad83ToCsrs->GetEntryPtr (index);
			entryPath = gdcEntryPtr->GetEntryPath ();
			CS_stncp (cTemp,entryPath,sizeof (cTemp));
			gdcNad83ToCsrs->ConvertToRelative (cTemp);
			gtStrm << L"   GRID_FILE: " << L"NTv2,INV," << cTemp  << std::endl;
		}
		break;
	case cs_DTCTYP_TOKYO:
		gtStrm <<   L"\t  METHOD: " << L"GRID_INTERP"             << std::endl;
		count = gdcTokyoToJgd2k->GetEntryCount ();
		for (index = 0;index < count;index += 1)
		{
			gdcEntryPtr = gdcTokyoToJgd2k->GetEntryPtr (index);
			entryPath = gdcEntryPtr->GetEntryPath ();
			CS_stncp (cTemp,entryPath,sizeof (cTemp));
			gdcTokyoToJgd2k->ConvertToRelative (cTemp);
			gtStrm << L"   GRID_FILE: " << L"JPPAR,FWD," << cTemp  << std::endl;
		}
		break;
	case cs_DTCTYP_RGF93:
		gtStrm <<   L"\t  METHOD: " << L"GRID_INTERP"             << std::endl;
		count = gdcRgf93ToNtf->GetEntryCount ();
		for (index = 0;index < count;index += 1)
		{
			gdcEntryPtr = gdcRgf93ToNtf->GetEntryPtr (index);
			entryPath = gdcEntryPtr->GetEntryPath ();
			CS_stncp (cTemp,entryPath,sizeof (cTemp));
			gdcRgf93ToNtf->ConvertToRelative (cTemp);
			cp = strrchr (cTemp,'.');
			if (!(CS_strnicmp (cp,".gsb",4)))
			{
				gtStrm << L"   GRID_FILE: " << L"NTv2,FWD," << cTemp  << std::endl;
			}
			else
			{
				gtStrm << L"   GRID_FILE: " << L"FRRGF,INV," << cTemp  << std::endl;
			}
		}
		break;
	case cs_DTCTYP_ED50:
		gtStrm <<   L"\t  METHOD: " << L"GRID_INTERP"             << std::endl;
		count = gdcEd50ToEtrf89->GetEntryCount ();
		for (index = 0;index < count;index += 1)
		{
			gdcEntryPtr = gdcEd50ToEtrf89->GetEntryPtr (index);
			entryPath = gdcEntryPtr->GetEntryPath ();
			CS_stncp (cTemp,entryPath,sizeof (cTemp));
			gdcEd50ToEtrf89->ConvertToRelative (cTemp);
			gtStrm << L"   GRID_FILE: " << L"NTv2,FWD," << cTemp  << std::endl;
		}
		break;
	case cs_DTCTYP_DHDN:
		gtStrm <<   L"\t  METHOD: " << L"GRID_INTERP"             << std::endl;
		count = gdcDhdnToEtrf89->GetEntryCount ();
		for (index = 0;index < count;index += 1)
		{
			gdcEntryPtr = gdcDhdnToEtrf89->GetEntryPtr (index);
			entryPath = gdcEntryPtr->GetEntryPath ();
			CS_stncp (cTemp,entryPath,sizeof (cTemp));
			gdcDhdnToEtrf89->ConvertToRelative (cTemp);
			gtStrm << L"   GRID_FILE: " << L"NTv2,FWD," << cTemp  << std::endl;
		}
		break;
	case cs_DTCTYP_ETRF89:
		gtStrm <<   L"\t  METHOD: " << L"NULL"                  << std::endl;
		break;
	case cs_DTCTYP_CHENYX:
		gtStrm <<   L"\t  METHOD: " << L"GRID_INTERP"             << std::endl;
		count = gdcCh1903ToPlus->GetEntryCount ();
		for (index = 0;index < count;index += 1)
		{
			gdcEntryPtr = gdcCh1903ToPlus->GetEntryPtr (index);
			entryPath = gdcEntryPtr->GetEntryPath ();
			CS_stncp (cTemp,entryPath,sizeof (cTemp));
			gdcCh1903ToPlus->ConvertToRelative (cTemp);
			gtStrm << L"   GRID_FILE: " << L"NTv2,FWD," << cTemp  << std::endl;
		}
		break;
	default:
		ok = false;
		break;
	}
	return ok;		
}
// The following uses the old CS-MAP to84_via element of the datum
// definition to determine the appropriate source and target datum
// names.  In the event that they are not the normal thing (key_nm
// and WGS84, respectively), an appropirate path which defines the
// the equivalent of a key_nm to WGS84 conversion is written to
// the provided geodetic path stream.

/******************************************************************************************
	{cs_DTCTYP_NAD27,
		{dtcTypNad27ToNad83,     dtcTypNad83ToWgs84,    dtcTypNone,           dtcTypNone  },
		{dtcTypWgs84ToNad83,     dtcTypNad83ToNad27,    dtcTypNone,           dtcTypNone  }
	},
	{cs_DTCTYP_NAD83,
		{dtcTypNad83ToWgs84,     dtcTypNone,            dtcTypNone,           dtcTypNone  },
		{dtcTypWgs84ToNad83,     dtcTypNone,            dtcTypNone,           dtcTypNone  }
	},
	{cs_DTCTYP_AGD66,
		{dtcTypAgd66ToGda94,     dtcTypGda94ToWgs84,    dtcTypNone,           dtcTypNone  },
		{dtcTypWgs84ToGda94,     dtcTypGda94ToAgd66,    dtcTypNone,           dtcTypNone  }
	},
	{cs_DTCTYP_AGD84,
		{dtcTypAgd84ToGda94,     dtcTypGda94ToWgs84,    dtcTypNone,           dtcTypNone  },
		{dtcTypWgs84ToGda94,     dtcTypGda94ToAgd84,    dtcTypNone,           dtcTypNone  }
	},
	{cs_DTCTYP_ED50,
		{dtcTypEd50ToEtrf89,     dtcTypEtrf89ToWgs84,   dtcTypNone,           dtcTypNone  },
		{dtcTypWgs84ToEtrf89,    dtcTypEtrf89ToEd50,    dtcTypNone,           dtcTypNone  }
	},
	{cs_DTCTYP_DHDN,
		{dtcTypDhdnToEtrf89,     dtcTypEtrf89ToWgs84,   dtcTypNone,           dtcTypNone  },
		{dtcTypWgs84ToEtrf89,    dtcTypEtrf89ToDhdn,    dtcTypNone,           dtcTypNone  }
	},
	{cs_DTCTYP_NZGD49,
		{dtcTypNzgd49ToNzgd2K,   dtcTypNzgd2KToWgs84,   dtcTypNone,           dtcTypNone  },
		{dtcTypWgs84ToNzgd2K,    dtcTypNzgd2KToNzgd49,  dtcTypNone,           dtcTypNone  }
	},
	{cs_DTCTYP_ATS77,
		{dtcTypAts77ToCsrs,      dtcTypCsrsToNad83,     dtcTypNad83ToWgs84,   dtcTypNone  },
		{dtcTypWgs84ToNad83,     dtcTypNad83ToCsrs,     dtcTypCsrsToAts77,    dtcTypNone  }
	},
	{cs_DTCTYP_WGS84,
		{dtcTypNone,             dtcTypNone,            dtcTypNone,           dtcTypNone  },
		{dtcTypNone,             dtcTypNone,            dtcTypNone,           dtcTypNone  }
	},
	{cs_DTCTYP_GDA94,
		{dtcTypGda94ToWgs84,     dtcTypNone,            dtcTypNone,           dtcTypNone  },
		{dtcTypWgs84ToGda94,     dtcTypNone,            dtcTypNone,           dtcTypNone  }
	},
	{cs_DTCTYP_NZGD2K,
		{dtcTypNzgd2KToWgs84,     dtcTypNone,            dtcTypNone,          dtcTypNone  },
		{dtcTypWgs84ToNzgd2K,     dtcTypNone,            dtcTypNone,          dtcTypNone  }
	},
	{cs_DTCTYP_CSRS,
		{dtcTypCsrsToNad83,       dtcTypNad83ToWgs84,    dtcTypNone,          dtcTypNone  },
		{dtcTypWgs84ToNad83,      dtcTypNad83ToCsrs,     dtcTypNone,          dtcTypNone  }
	},
	{cs_DTCTYP_TOKYO,
		{dtcTypTokyoToJgd2k,      dtcTypNone,            dtcTypNone,           dtcTypNone  },
		{dtcTypJgd2kToTokyo,      dtcTypNone,            dtcTypNone,           dtcTypNone  }
	},
	{cs_DTCTYP_WGS72,
		{dtcTypWgs72ToWgs84,     dtcTypNone,            dtcTypNone,           dtcTypNone  },
		{dtcTypWgs84ToWgs72,     dtcTypNone,            dtcTypNone,           dtcTypNone  }
	},
	{cs_DTCTYP_HPGN,
		{dtcTypHarnToNad83,      dtcTypNad83ToWgs84,    dtcTypNone,           dtcTypNone  },
		{dtcTypWgs84ToNad83,     dtcTypNad83ToHarn,     dtcTypNone,           dtcTypNone  }
	},
	{cs_DTCTYP_MOLO,
		{dtcTypMolodensky,       dtcTypNone,            dtcTypNone,           dtcTypNone  },
		{dtcTypMolodenskyInv,    dtcTypNone,            dtcTypNone,           dtcTypNone  }
	},
	{cs_DTCTYP_MREG,
		{dtcTypDMAMulReg,        dtcTypNone,            dtcTypNone,           dtcTypNone  },
		{dtcTypDMAMulRegInv,     dtcTypNone,            dtcTypNone,           dtcTypNone  }
	},
	{cs_DTCTYP_BURS,
		{dtcTypBursaWolf,        dtcTypNone,            dtcTypNone,           dtcTypNone  },
		{dtcTypBursaWolfInv,     dtcTypNone,            dtcTypNone,           dtcTypNone  }
	},
	{cs_DTCTYP_7PARM,
		{dtcTypSevenParm,        dtcTypNone,             dtcTypNone,          dtcTypNone  },
		{dtcTypSevenParmInv,     dtcTypNone,             dtcTypNone,          dtcTypNone  }
	},
	{cs_DTCTYP_6PARM,
		{dtcTypSixParm,          dtcTypNone,             dtcTypNone,          dtcTypNone  },
		{dtcTypSixParmInv,       dtcTypNone,             dtcTypNone,          dtcTypNone  }
	},
	{cs_DTCTYP_GEOCTR,
		{dtcTypGeoCtr,           dtcTypNone,             dtcTypNone,          dtcTypNone  },
		{dtcTypGeoCtrInv,        dtcTypNone,             dtcTypNone,          dtcTypNone  }
	},
	{cs_DTCTYP_4PARM,
		{dtcTypFourParm,         dtcTypNone,             dtcTypNone,          dtcTypNone  },
		{dtcTypFourParmInv,      dtcTypNone,             dtcTypNone,          dtcTypNone  }
	},
	{cs_DTCTYP_RGF93,
		{dtcTypNtfToRgf93,       dtcTypNone,             dtcTypNone,          dtcTypNone  },
		{dtcTypRgf93ToNtf,       dtcTypNone,             dtcTypNone,          dtcTypNone  }
	},
	{ cs_DTCTYP_ETRF89,
		{dtcTypNone,             dtcTypNone,            dtcTypNone,           dtcTypNone  },
		{dtcTypNone,             dtcTypNone,            dtcTypNone,           dtcTypNone  }
	},
	{cs_DTCTYP_3PARM,
		{dtcTypThreeParm,        dtcTypNone,            dtcTypNone,          dtcTypNone  },
		{dtcTypThreeParmInv,     dtcTypNone,            dtcTypNone,          dtcTypNone  }
	},
	{cs_DTCTYP_CHENYX,
		{dtcTypCh1903ToPlus,     dtcTypChPlusToChtrs95, dtcTypChtrs95ToEtrf89, dtcTypEtrf89ToWgs84  },
		{dtcTypWgs84ToEtrf89,    dtcTypEtrf89ToChtrs95, dtcTypChtrs95ToChPlus, dtcTypPlusToCh1903   }
	},
};
********************************************************************************************/
bool csAdjustSrcAndTrg (std::wofstream& gpStrm,char *srcDatum,char *trgDatum,
															  const cs_Dtdef_* dtDefPtr,
															  const char* dtmPivot)
{
	bool ok (true);

	short index;

	char pathName [64];
	char xfrmName [64];

	cs_GeodeticPathElement_* xfrmPtr;

	cs_GeodeticPath_ gPath;

	// Preparations in case we need to write a path.  Avoids
	// lots of duplicate code.
	memset ((void*)&gPath,0,sizeof (gPath));
	gPath.protect = 0;
	gPath.reversible = 1;
	gPath.accuracy = cs_Zero;
	gPath.epsgCode = 0;
	gPath.variant = 0;
	gPath.elementCount = 0;
	sprintf (pathName,"%s_to_%s",dtDefPtr->key_nm,dtmPivot);
	CS_stncp (gPath.pathName,pathName,sizeof (gPath.pathName));
	CS_stncp (gPath.srcDatum,dtDefPtr->key_nm,sizeof (gPath.srcDatum));
	CS_stncp (gPath.trgDatum,dtmPivot,sizeof (gPath.trgDatum));
	CS_stncp (gPath.description,dtDefPtr->name,sizeof (gPath.description));
	CS_stncp (gPath.source,dtDefPtr->source,sizeof (gPath.source));
	if (dtDefPtr->group [0] != '\0')
	{
		CS_stncp (gPath.group,dtDefPtr->group,sizeof (gPath.group));
	}

	switch (dtDefPtr->to84_via) {
	case cs_DTCTYP_MOLO:
	case cs_DTCTYP_GEOCTR:
	case cs_DTCTYP_3PARM:
	case cs_DTCTYP_4PARM:
	case cs_DTCTYP_6PARM:
	case cs_DTCTYP_7PARM:
	case cs_DTCTYP_BURS:
	case cs_DTCTYP_WGS72:
	case cs_DTCTYP_MREG:

	case cs_DTCTYP_GDA94:
	case cs_DTCTYP_NZGD2K:
	case cs_DTCTYP_ETRF89:
	case cs_DTCTYP_NAD83:
	case cs_DTCTYP_WGS84:

		// This is the typical case.
		CS_stncp (srcDatum,dtDefPtr->key_nm,cs_KEYNM_DEF);
		CS_stncp (trgDatum,dtmPivot,cs_KEYNM_DEF);
		// No path required for these.
		ok = true;
		break;

	case cs_DTCTYP_NAD27:
		// This transformation is actually from NAD27 to NAD83.
		CS_stncp (srcDatum,dtDefPtr->key_nm,cs_KEYNM_DEF);
		CS_stncp (trgDatum,"NAD83",cs_KEYNM_DEF);
		// Need a path so that this datum can get converted to WGS84.
		sprintf (xfrmName,"%s_to_%s",srcDatum,trgDatum);
		index = gPath.elementCount;
		xfrmPtr = &gPath.geodeticPathElements [index];
		CS_stncp (xfrmPtr->geodeticXformName,xfrmName,sizeof (xfrmPtr->geodeticXformName));
		xfrmPtr->direction = 0;
		gPath.elementCount += 1;

		sprintf (xfrmName,"%s_to_%s","NAD83",dtmPivot);
		index = gPath.elementCount;
		xfrmPtr = &gPath.geodeticPathElements [index];
		CS_stncp (xfrmPtr->geodeticXformName,xfrmName,sizeof (xfrmPtr->geodeticXformName));
		xfrmPtr->direction = 0;
		gPath.elementCount += 1;

		ok = csWriteGeodeticPath (gpStrm,&gPath);
		break;

	case cs_DTCTYP_AGD66:
	case cs_DTCTYP_AGD84:
		// This transformation is actually from AGD66/AGD84 to GDA94.
		CS_stncp (srcDatum,dtDefPtr->key_nm,cs_KEYNM_DEF);
		CS_stncp (trgDatum,"GDA94",cs_KEYNM_DEF);
		// Need a path so that this datum can get converted to WGS84.
		sprintf (xfrmName,"%s_to_%s",srcDatum,trgDatum);
		index = gPath.elementCount;
		xfrmPtr = &gPath.geodeticPathElements [index];
		CS_stncp (xfrmPtr->geodeticXformName,xfrmName,sizeof (xfrmPtr->geodeticXformName));
		xfrmPtr->direction = 0;
		gPath.elementCount += 1;

		sprintf (xfrmName,"%s_to_%s","GDA94",dtmPivot);
		index = gPath.elementCount;
		xfrmPtr = &gPath.geodeticPathElements [index];
		CS_stncp (xfrmPtr->geodeticXformName,xfrmName,sizeof (xfrmPtr->geodeticXformName));
		xfrmPtr->direction = 0;
		gPath.elementCount += 1;

		ok = csWriteGeodeticPath (gpStrm,&gPath);
		break;

	case cs_DTCTYP_NZGD49:
		// This transformation is actually from NZGD49 to NZGD2000.
		CS_stncp (srcDatum,dtDefPtr->key_nm,cs_KEYNM_DEF);
		CS_stncp (trgDatum,"NZGD2000",cs_KEYNM_DEF);
		// Need a path so that this datum can get converted to WGS84.
		sprintf (xfrmName,"%s_to_%s",srcDatum,trgDatum);
		index = gPath.elementCount;
		xfrmPtr = &gPath.geodeticPathElements [index];
		CS_stncp (xfrmPtr->geodeticXformName,xfrmName,sizeof (xfrmPtr->geodeticXformName));
		xfrmPtr->direction = 0;
		gPath.elementCount += 1;

		sprintf (xfrmName,"%s_to_%s","NZGD2000",dtmPivot);
		index = gPath.elementCount;
		xfrmPtr = &gPath.geodeticPathElements [index];
		CS_stncp (xfrmPtr->geodeticXformName,xfrmName,sizeof (xfrmPtr->geodeticXformName));
		xfrmPtr->direction = 0;
		gPath.elementCount += 1;

		ok = csWriteGeodeticPath (gpStrm,&gPath);
		break;

	case cs_DTCTYP_ATS77:
		// Things start getting painful here.  This transformation is
		// actually from ATS77 to CSRS (i.e. NAD83/1998).  We then need
		// to use CSRS_to_NAD83 to get to NAD83, and then finally
		// NAD83-to_WGS84 to get to WGS84.  This is required to emulate
		// what currently happens.
		//
		// First we set up the transformation which we are now processing.
		CS_stncp (srcDatum,dtDefPtr->key_nm,cs_KEYNM_DEF);
		CS_stncp (trgDatum,"CSRS",cs_KEYNM_DEF);

		// Need a path so that this datum can get converted to WGS84.
		// First element of the path gets us to CSRS.
		sprintf (xfrmName,"%s_to_%s",srcDatum,trgDatum);
		index = gPath.elementCount;
		xfrmPtr = &gPath.geodeticPathElements [index];
		CS_stncp (xfrmPtr->geodeticXformName,xfrmName,sizeof (xfrmPtr->geodeticXformName));
		xfrmPtr->direction = 0;
		gPath.elementCount += 1;

		// Second element of the path gets us to NAD83
		sprintf (xfrmName,"%s_to_%s","NAD83","CSRS");
		index = gPath.elementCount;
		xfrmPtr = &gPath.geodeticPathElements [index];
		CS_stncp (xfrmPtr->geodeticXformName,xfrmName,sizeof (xfrmPtr->geodeticXformName));
		xfrmPtr->direction = 1;
		gPath.elementCount += 1;

		// Third element of the path gets us to WGS84.
		sprintf (xfrmName,"%s_to_%s","NAD83",dtmPivot);
		index = gPath.elementCount;
		xfrmPtr = &gPath.geodeticPathElements [index];
		CS_stncp (xfrmPtr->geodeticXformName,xfrmName,sizeof (xfrmPtr->geodeticXformName));
		xfrmPtr->direction = 0;
		gPath.elementCount += 1;

		ok = csWriteGeodeticPath (gpStrm,&gPath);
		break;

	case cs_DTCTYP_TOKYO:
		// This transformation is actually from Tokyo to JGD2000.
		CS_stncp (srcDatum,dtDefPtr->key_nm,cs_KEYNM_DEF);
		CS_stncp (trgDatum,"JGD2000",cs_KEYNM_DEF);
		// Need a path so that this datum can get converted to WGS84.
		sprintf (xfrmName,"%s_to_%s",srcDatum,trgDatum);
		index = gPath.elementCount;
		xfrmPtr = &gPath.geodeticPathElements [index];
		CS_stncp (xfrmPtr->geodeticXformName,xfrmName,sizeof (xfrmPtr->geodeticXformName));
		xfrmPtr->direction = 0;
		gPath.elementCount += 1;

		sprintf (xfrmName,"%s_to_%s","JGD2000",dtmPivot);
		index = gPath.elementCount;
		xfrmPtr = &gPath.geodeticPathElements [index];
		CS_stncp (xfrmPtr->geodeticXformName,xfrmName,sizeof (xfrmPtr->geodeticXformName));
		xfrmPtr->direction = 0;
		gPath.elementCount += 1;

		ok = csWriteGeodeticPath (gpStrm,&gPath);
		break;

	case cs_DTCTYP_RGF93:
		// This transformation is actually from RGF93 to NTF.
		CS_stncp (srcDatum,"RGF93",cs_KEYNM_DEF);
		CS_stncp (trgDatum,dtDefPtr->key_nm,cs_KEYNM_DEF);

		// Need a path so that this datum can get converted to WGS84.
		sprintf (xfrmName,"%s_to_%s",srcDatum,trgDatum);
		index = gPath.elementCount;
		xfrmPtr = &gPath.geodeticPathElements [index];
		CS_stncp (xfrmPtr->geodeticXformName,xfrmName,sizeof (xfrmPtr->geodeticXformName));
		xfrmPtr->direction = 1;
		gPath.elementCount += 1;

		sprintf (xfrmName,"%s_to_%s","RGF93",dtmPivot);
		index = gPath.elementCount;
		xfrmPtr = &gPath.geodeticPathElements [index];
		CS_stncp (xfrmPtr->geodeticXformName,xfrmName,sizeof (xfrmPtr->geodeticXformName));
		xfrmPtr->direction = 0;
		gPath.elementCount += 1;

		ok = csWriteGeodeticPath (gpStrm,&gPath);
		break;

	case cs_DTCTYP_ED50:
		// This transformation is actually from ED50 to ETRF89.
		CS_stncp (srcDatum,dtDefPtr->key_nm,cs_KEYNM_DEF);
		CS_stncp (trgDatum,"ETRF89",cs_KEYNM_DEF);

		// Need a path so that this datum can get converted to WGS84.
		sprintf (xfrmName,"%s_to_%s",srcDatum,trgDatum);
		index = gPath.elementCount;
		xfrmPtr = &gPath.geodeticPathElements [index];
		CS_stncp (xfrmPtr->geodeticXformName,xfrmName,sizeof (xfrmPtr->geodeticXformName));
		xfrmPtr->direction = 0;
		gPath.elementCount += 1;

		sprintf (xfrmName,"%s_to_%s","ETRF89",dtmPivot);
		index = gPath.elementCount;
		xfrmPtr = &gPath.geodeticPathElements [index];
		CS_stncp (xfrmPtr->geodeticXformName,xfrmName,sizeof (xfrmPtr->geodeticXformName));
		xfrmPtr->direction = 0;
		gPath.elementCount += 1;

		ok = csWriteGeodeticPath (gpStrm,&gPath);
		break;

	case cs_DTCTYP_DHDN:
		// This transformation is actually from DHDN to ETRF89.
		CS_stncp (srcDatum,dtDefPtr->key_nm,cs_KEYNM_DEF);
		CS_stncp (trgDatum,"ETRF89",cs_KEYNM_DEF);

		// Need a path so that this datum can get converted to WGS84.
		sprintf (xfrmName,"%s_to_%s",srcDatum,trgDatum);
		index = gPath.elementCount;
		xfrmPtr = &gPath.geodeticPathElements [index];
		CS_stncp (xfrmPtr->geodeticXformName,xfrmName,sizeof (xfrmPtr->geodeticXformName));
		xfrmPtr->direction = 0;
		gPath.elementCount += 1;

		sprintf (xfrmName,"%s_to_%s","ETRF89",dtmPivot);
		index = gPath.elementCount;
		xfrmPtr = &gPath.geodeticPathElements [index];
		CS_stncp (xfrmPtr->geodeticXformName,xfrmName,sizeof (xfrmPtr->geodeticXformName));
		xfrmPtr->direction = 0;
		gPath.elementCount += 1;

		ok = csWriteGeodeticPath (gpStrm,&gPath);
		break;

	case cs_DTCTYP_CHENYX:
		// This transformation is actually from CH1903 to CH1903Plus.
		CS_stncp (srcDatum,dtDefPtr->key_nm,cs_KEYNM_DEF);
		CS_stncp (trgDatum,"CH1903Plus_1",cs_KEYNM_DEF);

		// Need a path so that this datum can get converted to WGS84.
		sprintf (xfrmName,"%s_to_%s",srcDatum,trgDatum);
		index = gPath.elementCount;
		xfrmPtr = &gPath.geodeticPathElements [index];
		CS_stncp (xfrmPtr->geodeticXformName,xfrmName,sizeof (xfrmPtr->geodeticXformName));
		xfrmPtr->direction = 0;
		gPath.elementCount += 1;

		sprintf (xfrmName,"%s_to_%s","CH1903Plus_1","CHTRF95");
		index = gPath.elementCount;
		xfrmPtr = &gPath.geodeticPathElements [index];
		CS_stncp (xfrmPtr->geodeticXformName,xfrmName,sizeof (xfrmPtr->geodeticXformName));
		xfrmPtr->direction = 0;
		gPath.elementCount += 1;

		sprintf (xfrmName,"%s_to_%s","CHTrF95","ETRF89");
		index = gPath.elementCount;
		xfrmPtr = &gPath.geodeticPathElements [index];
		CS_stncp (xfrmPtr->geodeticXformName,xfrmName,sizeof (xfrmPtr->geodeticXformName));
		xfrmPtr->direction = 0;
		gPath.elementCount += 1;

		sprintf (xfrmName,"%s_to_%s","ETRF89",dtmPivot);
		index = gPath.elementCount;
		xfrmPtr = &gPath.geodeticPathElements [index];
		CS_stncp (xfrmPtr->geodeticXformName,xfrmName,sizeof (xfrmPtr->geodeticXformName));
		xfrmPtr->direction = 0;
		gPath.elementCount += 1;

		ok = csWriteGeodeticPath (gpStrm,&gPath);
		break;

	case cs_DTCTYP_HPGN:
		// This transformation is actually from NAD83 to HPGN/HARN.
		CS_stncp (srcDatum,"NAD83",cs_KEYNM_DEF);
		CS_stncp (trgDatum,dtDefPtr->key_nm,cs_KEYNM_DEF);

		// Need a path so that this datum can get converted to WGS84.
		sprintf (xfrmName,"%s_to_%s",srcDatum,trgDatum);
		index = gPath.elementCount;
		xfrmPtr = &gPath.geodeticPathElements [index];
		CS_stncp (xfrmPtr->geodeticXformName,xfrmName,sizeof (xfrmPtr->geodeticXformName));
		xfrmPtr->direction = 1;
		gPath.elementCount += 1;

		sprintf (xfrmName,"%s_to_%s","NAD83",dtmPivot);
		index = gPath.elementCount;
		xfrmPtr = &gPath.geodeticPathElements [index];
		CS_stncp (xfrmPtr->geodeticXformName,xfrmName,sizeof (xfrmPtr->geodeticXformName));
		xfrmPtr->direction = 0;
		gPath.elementCount += 1;

		ok = csWriteGeodeticPath (gpStrm,&gPath);
		break;

	case cs_DTCTYP_CSRS:
		// This transformation is actually from NAD83 to CSRS.
		CS_stncp (srcDatum,"NAD83",cs_KEYNM_DEF);
		CS_stncp (trgDatum,dtDefPtr->key_nm,cs_KEYNM_DEF);

		// Need a path so that this datum can get converted to WGS84.
		sprintf (xfrmName,"%s_to_%s",srcDatum,trgDatum);
		index = gPath.elementCount;
		xfrmPtr = &gPath.geodeticPathElements [index];
		CS_stncp (xfrmPtr->geodeticXformName,xfrmName,sizeof (xfrmPtr->geodeticXformName));
		xfrmPtr->direction = 1;
		gPath.elementCount += 1;

		sprintf (xfrmName,"%s_to_%s","NAD83",dtmPivot);
		index = gPath.elementCount;
		xfrmPtr = &gPath.geodeticPathElements [index];
		CS_stncp (xfrmPtr->geodeticXformName,xfrmName,sizeof (xfrmPtr->geodeticXformName));
		xfrmPtr->direction = 0;
		gPath.elementCount += 1;

		ok = csWriteGeodeticPath (gpStrm,&gPath);
		break;

	case cs_DTCTYP_NONE:
	default:
		ok = false;
		break;
	}
	return ok;
}
bool csWriteGeodeticPath (std::wofstream& gpStrm,const cs_GeodeticPath_ *gpPtr)
{
	bool ok;
	
	short count;
	short index;

	wchar_t* direction;
	const cs_GeodeticPathElement_* xfrmPtr;
	
	gpStrm << std::endl;
	gpStrm << L"GP_NAME: "          << gpPtr->pathName      << std::endl;
	if (gpPtr->group [0] != '\0')
	{
		gpStrm << L"\t    GROUP: "      << gpPtr->group         << std::endl;
	}
	gpStrm << L"\t  SRC_DTM: "      << gpPtr->srcDatum      << std::endl;
	gpStrm << L"\t  TRG_DTM: "      << gpPtr->trgDatum      << std::endl;
	gpStrm << L"\t  DESC_NM: "      << gpPtr->description   << std::endl;
	gpStrm << L"\t   SOURCE: "      << gpPtr->source        << std::endl;
	if (gpPtr->accuracy != 0.0)
	{
	gpStrm << L"\t ACCURACY: "      << gpPtr->accuracy      << std::endl;
	}
	if (gpPtr->epsgCode != 0)
	{
	gpStrm << L"\t     EPSG: "      << gpPtr->epsgCode      << std::endl;
	}
	count = gpPtr->elementCount;
	for (index = 0;index < count;index += 1)
	{
		xfrmPtr = &gpPtr->geodeticPathElements [index];
		direction = (xfrmPtr->direction == 0) ? L"FWD" : L"INV";
		gpStrm << L"\t    XFORM: "
		       << xfrmPtr->geodeticXformName
		       << L','
		       << direction
		       << std::endl;
	}

	ok = gpStrm.good ();
	return ok;
}

bool csConvertMrtFile (std::wofstream& gtStrm,const cs_Dtdef_ *dtDefPtr,
											 const TcsDefFile& mregAsc)
{
	bool ok (false);

	short uuPwr, vvPwr;

	double coefficient;
	
	wchar_t wcTemp [256];

	// Set up the name of the file.  We rely on the host application to have
	// properly initialized CS-MAP with a successful call to CS_altdr ();
	CS_stncp (cs_DirP,dtDefPtr->key_nm,CSMAXPATH);
	CS_stncat (cs_DirP,".MRT",CSMAXPATH);
	TcsMrtFile mrtFile ((const char*)cs_Dir);
	ok = mrtFile.IsOk ();
	if (!ok)
	{
		return ok;
	}

	// Get the definition in ASCII form.  We need this as the test case data
	// is not actually written to the .mrt file.
	const TcsAscDefinition* ascDefPtr = mregAsc.GetDefinition (dtDefPtr->key_nm);
	if (ascDefPtr == NULL)
	{
		ok = false;
		return ok;
	}

	// Output the test values whaich are only available from the mregAsc file.
	const TcsDefLine* defLinePtr;
	defLinePtr = ascDefPtr->GetLine ("TEST_PHI:");
	if (defLinePtr == NULL)
	{
		ok = false;
		return ok;
	}
	gtStrm << L"\t\t  TEST_LAT: " << defLinePtr->GetValue () << std::endl;

	defLinePtr = ascDefPtr->GetLine ("TEST_LAMBDA:");
	if (defLinePtr == NULL)
	{
		ok = false;
		return ok;
	}
	gtStrm << L"\t\t  TEST_LNG: " << defLinePtr->GetValue () << std::endl;
	
	defLinePtr = ascDefPtr->GetLine ("DELTA_PHI:");
	if (defLinePtr == NULL)
	{
		ok = false;
		return ok;
	}
	gtStrm << L"\tRSLT_DELTA_LAT: " << defLinePtr->GetValue () << std::endl;

	defLinePtr = ascDefPtr->GetLine ("DELTA_LAMBDA:");
	if (defLinePtr == NULL)
	{
		ok = false;
		return ok;
	}
	gtStrm << L"\tRSLT_DELTA_LNG: " << defLinePtr->GetValue () << std::endl;

	defLinePtr = ascDefPtr->GetLine ("DELTA_HEIGHT:");
	if (defLinePtr == NULL)
	{
		ok = false;
		return ok;
	}
	gtStrm << L"\tRSLT_DELTA_HGT: " << defLinePtr->GetValue () << std::endl;

	// OK, now from the MRT file.

	gtStrm << L"\t   SRC_LAT_OFF: " << mrtFile.GetPhiOffset () << std::endl;
	gtStrm << L"\t   SRC_LNG_OFF: " << mrtFile.GetLambdaOffset () << std::endl;
	gtStrm << L"\t\t   NRML_KK: " << mrtFile.GetNormalizingScale () << std::endl;
	gtStrm << L"\t\tVALIDATION: " << L"1.0" << std::endl;

	// Add the Longitude complex element.
	for (uuPwr = 0;uuPwr < 10;uuPwr += 1)
	{
		for (vvPwr = 0;vvPwr < 10;vvPwr += 1)
		{
			coefficient = mrtFile.GetLambdaCoeff (uuPwr,vvPwr);
			if (coefficient != 0.0)
			{
				swprintf (wcTemp,128,L"\tLNG_COEF U%d V%d: ",uuPwr,vvPwr);
				gtStrm << wcTemp << coefficient << std::endl;
			}
		}
	}
	
	// Add the Latitude complex element.
	for (uuPwr = 0;uuPwr < 10;uuPwr += 1)
	{
		for (vvPwr = 0;vvPwr < 10;vvPwr += 1)
		{
			coefficient = mrtFile.GetPhiCoeff (uuPwr,vvPwr);
			if (coefficient != 0.0)
			{
				swprintf (wcTemp,128,L"\tLAT_COEF U%d V%d: ",uuPwr,vvPwr);
				gtStrm << wcTemp << coefficient << std::endl;
			}
		}
	}

	// Add the Height complex element.
	for (uuPwr = 0;uuPwr < 10;uuPwr += 1)
	{
		for (vvPwr = 0;vvPwr < 10;vvPwr += 1)
		{
			coefficient = mrtFile.GetHgtCoeff (uuPwr,vvPwr);
			if (coefficient != 0.0)
			{
				swprintf (wcTemp,128,L"\tHGT_COEF U%d V%d: ",uuPwr,vvPwr);
				gtStrm << wcTemp << coefficient << std::endl;
			}
		}
	}
	return ok;
}


// Given the to84_via value, need to determine the actual
//	1> source datum
//	2> target datum
//	3> any geodetic path entry associated with this.

