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
// This module seeks to accomplish two goals:
//
// 1> Find definition discrepancies between CS-MAP and EPSG.
// 2> Verify that the NameMapper is accurately mapping EPSG codes to
//    the correct corresponding CS-MAP definition.
//
// Both of these goals are achieved by the following process:
//
// 1> Walk through the appropriate EPSG table, extracting the EPSG
//    code of specific entries (ellipsoids, datums. CRS's) in the
//    table.
// 2> Run the EPSG code through the NameMapper to get the corresponding
//    CS-MAP name.
// 3> Extract the definition from EPSG in the form of a CS-MAP definition.
// 4> Obtain the CS-MAP definition from the dictionaries.
// 5> Compare the two definitions.
//
// Clearly, if the NAmeMapper maps to the wrong definition, the definitions
// will not match.  If the mapping is correct, but the definitions do not
// match, then clearly either CS-MAP or EPSG is wrong and the discrepancy
// needs to be investgated.
//
// Evenutally, it would be nice to do the same sort of thing with ESRI and
// Oracle WKT.
//
// Note that TestM does an audit on the NameMapper table, so we simply ignore
// errors of that type here.  That is, we ignore the condition where an EPSG
// code fails to map to a CS-MAP name, and we ignore all deprecated EPSG
// definitions.
//
// Also, to keep th code from geeting laboriously dull, we simply count,
// and otherwise ignore, and errors returned from the EPSG access code.
// A failedCnt of zero should be expected; behavior to the contrary is
// a defect in this code.
//

#include <ctype.h>
#include <time.h>
#include <locale.h>

#include "cs_map.h"
#include "cs_wkt.h"
#include "csTestCpp.hpp"

extern "C"
{
	extern "C" int cs_Error;
	extern "C" int cs_Errno;
	extern "C" int csErrlng;
	extern "C" int csErrlat;
	extern "C" unsigned short cs_ErrSup;

	#if _RUN_TIME <= _rt_UNIXPCC
	extern "C" ulong32_t cs_Doserr;
	#endif
}

extern "C" double cs_One;

int CStestN (const TcsEpsgDataSetV6& epsgV6,bool verbose,long32_t duration)
{
	bool ok;
	bool deprecated;

	enum EcsMapSt csMapSt;

	int okCnt = 0;				// successful definition comparisons

	int errCnt = 0;				// total error count, function return value
	int mapCnt = 0;				// number of NameMapper failures
	int diffCnt = 0;			// number of comparison failures
	int cvtCnt = 0;				// number of EPSG -> CS-MAP conversion failiures
	int failCnt = 0;			// number of failures encountered in EPSG access code

	unsigned variant;
	unsigned index;
	unsigned recordCount;
	unsigned variantCount;
	TcsEpsgCode epsgCode;
	TcsEpsgCode oprtnCode;

	double qValue;

	struct cs_Eldef_ epsgElDef;
	struct cs_Eldef_ *csMapElDef;

	struct cs_Dtdef_ epsgDtDef;
	struct cs_Dtdef_ *csMapDtDef;

	struct cs_Csdef_ epsgCsDef;
	struct cs_Csdef_ *csMapCsDef;

	char csMapKeyName [32];
	char epsgKeyName [64];
	char message [256];

	std::wstring fldData;

	// We start with the easiest case, the Ellipsoids.  We walk the EPSG
	// ellipsoid table, extracting the EPSG code for each entry.
	recordCount = epsgV6.GetRecordCount (epsgTblEllipsoid);
	for (index = 0;index < recordCount;++index)
	{
		ok = epsgV6.GetCodeByIndex (epsgCode,epsgTblEllipsoid,epsgFldEllipsoidCode,index);
		if (ok)
		{
			// We ignore deprecated EPSG definitions.
			deprecated = epsgV6.IsDeprecated (epsgTblEllipsoid,epsgCode);
			if (deprecated)
			{
				continue;
			}
		}
		
		// Extract the EPSG name of the ellipsoid fro reporting purposes.
		ok = epsgV6.GetFieldByCode (fldData,epsgTblEllipsoid,epsgFldEllipsoidName,epsgCode);
		if (!ok)
		{
			failCnt += 1;
			continue;
		}
		wcstombs (epsgKeyName,fldData.c_str (),sizeof (epsgKeyName));
		epsgKeyName [sizeof (epsgKeyName) - 1] = '\0';

		// Extract, in CS-MAP form, the definition of the ellipsoid from the the
		// EPSG database.
		ok = epsgV6.GetCsMapEllipsoid (epsgElDef,epsgCode);
		if (!ok)
		{
			// The conversion failed.
			cvtCnt += 1;
			continue;
		}

		// Run the EPSG code through the NameMapper.  If there is no match, we
		// can't do much here.  However, these types of failures are dealt
		// with in TestM, no need to repeat this here.
		csMapSt = csMapIdToNameC (csMapEllipsoidKeyName,csMapKeyName,sizeof (csMapKeyName),
																	 csMapFlvrAutodesk,
																	 csMapFlvrEpsg,
																	 epsgCode);
		if (csMapSt != csMapOk)
		{
			mapCnt += 1;
			continue;
		}
		
		// Get the CS-MAP version of this ellipsoid.
		csMapElDef = CS_eldef (csMapKeyName);
		ok = (csMapElDef != 0);
		if (ok)
		{
			// Compare the two ellipsoids.  The qValue is essentially the
			// difference in meters of the meridional arc from the equator
			// to the 60th parallel on the two ellipsoids.  The tolerance of
			// 5 millimeters is fairly liberal.
			int st = CS_elDefCmpEx (&qValue,csMapElDef,&epsgElDef,message,sizeof (message));
			if (qValue < 5.0E-03)
			{
				okCnt += 1;
			}
			else
			{
				printf ("CS-MAP ellipsoid '%s' differs from EPSG '%s' [%lu]:\n\t%s\n",csMapKeyName,
																					  epsgKeyName,
																					  static_cast<ulong32_t>(epsgCode),
																					  message);
				diffCnt += 1;
			}
			CS_free (csMapElDef);
		}
		// On to the next ellipsoid.
	}
	printf ("Ellipsoid Test: ok = %d, different = %d, noCvt = %d, failed = %d.\n\n",okCnt,diffCnt,cvtCnt,failCnt);
	errCnt += (diffCnt + cvtCnt + failCnt);

	okCnt = 0;
	mapCnt = 0;
	diffCnt = 0;
	cvtCnt = 0;
	failCnt = 0;
	csMapDtDef = 0;
	
	// Now for the datum definitions.  This gets rather complicated as the difference
	// between the two models is significant
	recordCount = epsgV6.GetRecordCount (epsgTblDatum);
	for (index = 0;index < recordCount;++index)
	{
		// Get the EPSG Datum code.
		ok = epsgV6.GetCodeByIndex (epsgCode,epsgTblDatum,epsgFldDatumCode,index);
		if (!ok)
		{
			failCnt += 1;
			continue;
		}

		// Ignore all deprecated datums.
		deprecated = epsgV6.IsDeprecated (epsgTblDatum,epsgCode);
		if (deprecated)
		{
			continue;
		}

		// We only do EPSG datums of the "geodetic" type.
		ok = epsgV6.GetFieldByCode (fldData,epsgTblDatum,epsgFldDatumType,epsgCode);
		if (!ok)
		{
			failCnt += 1;
			continue;
		}
		EcsDtmType dtmType = GetEpsgDtmType (fldData.c_str ());
		if (dtmType != epsgDtmTypGeodetic)
		{
			continue;
		}

		// Get the EPSG Datum name for reporting purposes.
		ok = epsgV6.GetFieldByCode (fldData,epsgTblDatum,epsgFldDatumName,epsgCode);
		if (!ok)
		{
			failCnt += 1;
			continue;
		}
		wcstombs (epsgKeyName,fldData.c_str (),sizeof (epsgKeyName));
		epsgKeyName [sizeof (epsgKeyName) - 1] = '\0';

		// Get the CS-MAP equivalent, per the mapping tables.
		csMapSt = csMapIdToNameC (csMapDatumKeyName,csMapKeyName,sizeof (csMapKeyName),
																 csMapFlvrAutodesk,
																 csMapFlvrEpsg,
																 epsgCode);
		if (csMapSt != csMapOk)
		{
			mapCnt += 1;
			continue;
		}

		// Now it gets rather hairy.  There can be multiple transformations in
		// the EPSG database for any given datum shift.  To the degree
		// reasonable, we want to compare CS-MAP's definition to the specific
		// definition out of EPSG which is closest to the CS-MAP definition.
		// So, we introduce a block of code which, is admittedly not very
		// efficient, to select the specific EPSG variant which we wish to
		// compare against.  Should any of this fail, we simply use zero as
		// a variant number.  This will instruct GetCsMapDatum to chose the
		// variant which it thinks is best.  Best is defined in the
		// implementation of the TcsOpVariants code.
		variant = 0U;
		variantCount = 1;
		oprtnCode = 0UL;

		// The following block is simply to establish a namespace for some
		// local variables; an attempt to keep everything related to this
		// problem localized for possible reuse somewhere else.
		{
			bool lclOk;
			unsigned lclVariant;
			const TcsOpVariant* variantPtr;
			TcsEpsgCode geographicBase;

			// Locate the base CRS for this datum.  That is, the geographic
			// CRS which references this datum.
			lclVariant = 1;						// in case non of this works.
			lclOk = epsgV6.LocateGeographicBase (geographicBase,epsgCrsTypGeographic2D,epsgCode);
			if (lclOk)
			{
				// We have the base geogrpahic system.  Create a collection of
				// transofrmations which convert from the base to WGS84.  Another
				// complication here:  EPSG considers WGS84 to be most current
				// realization.  Thus, it considers HARN and WGS84 to be the same
				// thing, while NAD83 and WGS84 are different.  Not so in CS-MAP.
				// So, another complication to all this which relates to the
				// different models in use.
				TcsOpVariants opVariants (epsgV6,geographicBase,4326UL);		// 4326 is the EPSG code for WGS84
				variantCount = opVariants.GetVariantCount ();
				if (variantCount == 1U)
				{
					// There is only one transformation of this datum to WGS84.
					// No need for all this nonsense; there is only one variant.
					variantPtr = opVariants.GetVariantPtr (0U);
					if (variantPtr != 0)
					{
						lclVariant = variantPtr->GetVariantNbr ();
						oprtnCode = variantPtr->GetOpCodeForCsMap ();
					}
				}
				else if (opVariants.GetVariantCount () > 1U)
				{
					// There are more than one variants for this transformation.
					// Note that any of these may be concatenated operations,
					// thus there may be as many as three transformations
					// involved in any given transformations.  This issue is
					// delat with in the TcsOpVariants implementation.
					
					// Since we need to select on of the variants, we need
					// something to compare to.
					csMapDtDef = CS_dtdef (csMapKeyName);
					lclOk = (csMapDtDef != 0);
					if (lclOk)
					{
						double bestQValue = 9.0E+99;
						unsigned bestVariant = 1;		// in case all this all goes to pot
						
						// For each variant, we extract from EPSG a datum
						// definition in CS-MAP form.  To the extent that the
						// extraction(s) are successful, we compare to the
						// CS-MAP definition the NameMapper says is the
						// corresponding CS-MAP definition for this datum.
						// In the process, we choose the variant which produces
						// the smallest qFactor.  See CS-dtDefCmpEx to see the
						// current definition of what the qFactor really is.
						// Is has been pretty dynamic through all this
						// development.
						for (unsigned idx = 0;idx < variantCount;idx++)
						{
							variantPtr = opVariants.GetVariantPtr (idx);
							if (variantPtr == 0) continue;		// NO CRASHES, thank you.
							lclVariant = variantPtr->GetVariantNbr ();
							lclOk = epsgV6.GetCsMapDatum (epsgDtDef,epsgElDef,epsgCode,lclVariant);
							if (!lclOk)
							{
								// At this point, we want to ignore any variants that we cannot
								// convert to CS-MAP.
								lclOk = true;
								continue;
							}
							int st = CS_dtDefCmpEx (&qValue,csMapDtDef,&epsgDtDef,0,0);
							if (qValue < bestQValue)
							{
								// This one is better than the last one.
								bestVariant = lclVariant;
								bestQValue = qValue;
								oprtnCode = variantPtr->GetOpCodeForCsMap ();
							}
						}
						if (lclOk)
						{
							lclVariant = bestVariant;
						}
						CS_free (csMapDtDef);
					}
				}
			}
			if (lclOk)
			{
				// This is the variant which we want to compare CS-MAP with.
				variant = lclVariant;
			}
		}

		// Using whatever variant might have been selected by all the above,
		// we get the EPSG version in CS-MAP form and compare it to the CS-MAP
		// definition to which it is mapped.
		ok = epsgV6.GetCsMapDatum (epsgDtDef,epsgElDef,epsgCode,variant);
		if (!ok)
		{
			cvtCnt += 1;
			continue;
		}

		csMapDtDef = CS_dtdef (csMapKeyName);
		if (csMapDtDef != 0)
		{
			// EPSG does not support the Seven Parameter, only Bursa/Wolf.
			if (csMapDtDef->to84_via == cs_DTCTYP_7PARM && epsgDtDef.to84_via == cs_DTCTYP_BURS)
			{
				epsgDtDef.to84_via = cs_DTCTYP_7PARM;
			}
			
			// Molodensky, GeoCentric, and Three Parameter are essentially the same.
			// GeoCentric is the prefered, so we switch to that for comparison
			// purposes.
			if (csMapDtDef->to84_via == cs_DTCTYP_3PARM)
			{
				csMapDtDef->to84_via = cs_DTCTYP_GEOCTR;
			}
			if (csMapDtDef->to84_via == cs_DTCTYP_MOLO && epsgDtDef.to84_via == cs_DTCTYP_GEOCTR)
			{
				csMapDtDef->to84_via = cs_DTCTYP_GEOCTR;
			}
			if (csMapDtDef->to84_via == cs_DTCTYP_GEOCTR && epsgDtDef.to84_via == cs_DTCTYP_MOLO)
			{
				epsgDtDef.to84_via = cs_DTCTYP_GEOCTR;
			}

			// EPSG does not support (in general) the DMA MULREG
			// transformation.  In CS-MAP, MREG type definitions typically
			// have the GeoCentric parameters as a fallback.
			if (csMapDtDef->to84_via == cs_DTCTYP_MREG && epsgDtDef.to84_via == cs_DTCTYP_GEOCTR)
			{
				csMapDtDef->to84_via = cs_DTCTYP_GEOCTR;
			}

			// Perhaps we should use the CS_dtDefCMpEx() function here.  We'll
			// see how this all works out and then decide.
			int st = CS_dtDefCmpEx (&qValue,csMapDtDef,&epsgDtDef,message,sizeof (message));
			if (qValue <= 0.5)
			{
				okCnt += 1;
			}
			else
			{
				printf ("CS-MAP datum '%s' differs from EPSG '%s' [%lu:%lu (%d of %d)]:\n\t%s\n",
																csMapKeyName,
																epsgKeyName,
																static_cast<unsigned long>(epsgCode),
																static_cast<unsigned long>(oprtnCode),
																variant,
																variantCount,
																message);
				diffCnt += 1;
			}
			CS_free (csMapDtDef);
		}
	}
	printf ("Datum Test: ok = %d, different = %d, noCvt = %d, failed = %d\n\n",okCnt,diffCnt,cvtCnt,failCnt);
	errCnt += (diffCnt + cvtCnt + failCnt);

	okCnt = 0;
	mapCnt = 0;
	diffCnt = 0;
	cvtCnt = 0;
	failCnt = 0;
	EcsCrsType crsType;

	// Now for the Coordinate System dictionary definitions.
	recordCount = epsgV6.GetRecordCount (epsgTblReferenceSystem);
	for (index = 0;index < recordCount;++index)
	{
		// Get the CRS code.
		ok = epsgV6.GetCodeByIndex (epsgCode,epsgTblReferenceSystem,epsgFldCoordRefSysCode,index);
		if (!ok)
		{
			failCnt += 1;
			continue;
		}

		// We ignore deprecated systems.
		deprecated = epsgV6.IsDeprecated (epsgTblReferenceSystem,epsgCode);
		if (deprecated)
		{
			continue;
		}

		// We only do EPSG reference systems of the "projected" or "geographic2D" type.
		ok = epsgV6.GetFieldByCode (fldData,epsgTblReferenceSystem,epsgFldCoordRefSysKind,epsgCode);
		if (!ok)
		{
			failCnt += 1;
			continue;
		}
		crsType = GetEpsgCrsType (fldData.c_str ());
		if (crsType != epsgCrsTypProjected && crsType != epsgCrsTypGeographic2D)
		{
			continue;
		}
			
		// Get the EPSG CRS name for reporting purposes.
		ok = epsgV6.GetFieldByCode (fldData,epsgTblReferenceSystem,epsgFldCoordRefSysName,epsgCode);
		if (!ok)
		{
			failCnt += 1;
			continue;
		}
		wcstombs (epsgKeyName,fldData.c_str (),sizeof (epsgKeyName));
		epsgKeyName [sizeof (epsgKeyName) - 1] = '\0';

		// Get the CS-MAP equivalent, per the mapping tables.
		if (crsType == epsgCrsTypProjected)
		{
			csMapSt = csMapIdToNameC (csMapProjectedCSysKeyName,csMapKeyName,sizeof (csMapKeyName),
																			 csMapFlvrAutodesk,
																			 csMapFlvrEpsg,
																			 epsgCode);
		}
		else if (crsType == epsgCrsTypGeographic2D)
		{
			csMapSt = csMapIdToNameC (csMapGeographicCSysKeyName,csMapKeyName,sizeof (csMapKeyName),
																			  csMapFlvrAutodesk,
																			  csMapFlvrEpsg,
																			  epsgCode);
		}
		if (csMapSt != csMapOk)
		{
			mapCnt += 1;
			continue;
		}

		csMapCsDef = CS_csdef (csMapKeyName);
		if (csMapCsDef != 0)
		{
			ok = epsgV6.GetCsMapCoordsys (epsgCsDef,epsgDtDef,epsgElDef,epsgCode);
			if (ok)
			{
				// The conversion above correctly returns the quad value per
				// the EPSG database.  However, in most all cases where this
				// conversion returns a -1, the CS-MAP version uses a quad
				// value of 1.  That is, most all GIS systems don't deal with
				// the swapping of the axes even though it may be technically
				// correct.
				if (csMapCsDef->quad == 1 && epsgCsDef.quad == -1)
				{
					epsgCsDef.quad = 1;
				}

				// With these adjustments, we're ready to compare the EPSG
				// version with the CS-MAP version and get some meaningful
				// results.
				int st = CS_csDefCmpEx (&qValue,csMapCsDef,&epsgCsDef,message,sizeof (message));
				if (qValue < 0.5)
				{
					okCnt += 1;
				}
				else
				{
					printf ("CS-MAP coordinate system '%s' differs from EPSG CRS '%s' [%lu]:\n\t%s\n",
																		csMapKeyName,
																		epsgKeyName,
																		static_cast<unsigned long>(epsgCode),
																		message);
					diffCnt += 1;
				}
			}
			else
			{
				if (verbose)
				{
					printf ("conversion of EPSG CRS '%s' [%lu] to CS-MAP form failed.\n",
																		epsgKeyName,
																		static_cast<unsigned long>(epsgCode));
				}
				cvtCnt += 1;
			}
			CS_free (csMapCsDef);
		}
		else
		{
			// Here if the NameMapper mapped this EPSG code to a CS-MAP name,
			// but a definition with the mapped CS-MAP name did not exist in
			// CS-MAP dictionary.  This conidition is tested by TestM, the
			// NameMapper Audit.  So, we ignore it here.
		}
	}
	printf ("Coordsys Test: ok = %d, different = %d, noCvt = %d, failed = %d\n\n",okCnt,diffCnt,cvtCnt,failCnt);
	errCnt += (diffCnt + cvtCnt + failCnt);

	return errCnt;
}
