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

#include <iomanip>

extern "C" unsigned long KcsNmInvNumber;
extern "C" unsigned long KcsNmMapNoNumber;
extern "C" double cs_Zero;

// This utility updates the NameMapper.csv source file with new information
// extracted from another data file in .csv format.
//
// Specifically, we update the NNameMapper source file with EPSG codes and
// names, and ESRI codes and names, extracted from both the current EPSG
// dataset and a .csv file which contains the latest ESRI information in the
// following form:
//	Field 1: Integer denoting the flavor of the entry
//	Field 2: EPSG code of the subject system, 0 == unknown
//	Field 3: Numeric ID of the "Field 1" flavor for the subject system
//	FIeld 4: The definition of the system in WKT of the "Field 1" flavor.
//
// If time, energy, and money are sufficient, we may try also verifying that
// All geographic references, datum references, ellipsoid references, and
// unit references are also updated to the degree possible.  This gets rather
// iffy as there is no direct correlation between the names and a definition.
// With regard to the system name; well the name is essentially defined by the
// WKT system definition provided.

bool csNameMapperUpdateEpsg (const TcsEpsgDataSetV6* epsgPtr,TcsNameMapper& nameMapper,
															 EcsMapObjType objType,
															 TcsEpsgCode epsgCode);
bool csNameMapperUpdateFlvr (const TcsEpsgDataSetV6* epsgPtr,TcsNameMapper& nameMapper,
															 EcsMapObjType objType,
															 EcsNameFlavor flavor,
															 TcsEpsgCode epsgCode,
															 unsigned long wkidCode,
															 const wchar_t* crsName);
bool csNameMapperUpdateFlvr (const TcsEpsgDataSetV6* epsgPtr,TcsNameMapper& nameMapper,
															 EcsMapObjType objType,
															 EcsNameFlavor flavor,
															 unsigned long wkidCode,
															 const wchar_t* crsName);

// This is not great style, but it relieves us of a lot of argument
// passing and suplicate code.  Perhaps this should be an object.

wchar_t wktUnitName [64];
wchar_t wktDatumName [64];
wchar_t wktSpheroidName [64];
wchar_t wktGeogCSName [128];
wchar_t wktProjCSName [128];
wchar_t wktRlsLevel [64];
wchar_t epsgRlsLevel [64];

bool csUpdateNameMapperFromCsv (const wchar_t* csDictTrgDir,const wchar_t* csDictSrcDir,const wchar_t* srcCsvFullPath)
{
	bool ok (false);
	const wchar_t* wcPtrK;

	wchar_t fullPath [1024];

	std::wifstream iStrm1;
	std::wifstream iStrm2;

	// We're going to need access to the latest EPSG database.
	const TcsEpsgDataSetV6* epsgPtr;
	epsgPtr = GetEpsgObjectPtr ();
	ok = (epsgPtr != 0);
	if (!ok)
	{
		return ok;
	}
	wcPtrK = epsgPtr->GetRevisionLevel ();
	ok = (wcPtrK != 0);
	if (ok)
	{
		wcsncpy (epsgRlsLevel,wcPtrK,wcCount (epsgRlsLevel));
		epsgRlsLevel [wcCount (epsgRlsLevel) - 1] = L'\0';
	}
	else
	{
		wcsncpy (epsgRlsLevel,L"<release unknown>",wcCount (epsgRlsLevel));
		epsgRlsLevel [wcCount (epsgRlsLevel) - 1] = L'\0';
	}

	// Fetch a usable copy of the source .csv file which carries the update
	// information.  As WKT has lots of double quotes, commas, and other stuff
	// in it, we actually expect a file with the vertical bar as the field
	// separation character, with no "text" quoting.  The TcsCsvFileBase
	// object really likes to have three delimiters, so we just picked two
	// additional arkane characters which we are confident will never show up
	// in a WKT string.
	wchar_t delimiters [6] = L",\"\"";
	TcsCsvStatus csvStatus;
	TcsCsvFileBase crsWktCsv (true,5,7,delimiters);
	iStrm1.open (srcCsvFullPath,std::ios_base::in);
	ok = iStrm1.is_open ();
	if (ok)
	{
		ok = crsWktCsv.ReadFromStream (iStrm1,true,csvStatus);
		iStrm1.close ();
	}
	if (!ok)
	{
		return ok;
	}

	// OK, if we are still here, we import the current NameMapper source file
	// into a legitimate TcsNameMapper object.  We'll update that object as
	// necessary, and then simply write it out to a new .csv source file.
	TcsNameMapper nameMapper;
	nameMapper.SetRecordDuplicates (true);
	if (ok)
	{
		wcscpy (fullPath,csDictSrcDir);
		wcscat (fullPath,L"\\NameMapper.csv");
		iStrm2.open (fullPath,std::ios_base::in);
		ok = iStrm2.is_open ();
		if (ok)
		{
			ok = iStrm2.good ();
		}
		if (ok)
		{
			EcsCsvStatus lclStatus = nameMapper.ReadFromStream (iStrm2);
			iStrm2.close ();
			ok = (lclStatus == csvOk);
			if (ok)
			{
				ok = nameMapper.IsInitialized ();
			}
		}
	}
	if (!ok)
	{
		return ok;
	}

	// All set to go.  We loop once for each entry in the update data
	// source file.
	unsigned recNbr;
	for (recNbr = 0;ok && recNbr < crsWktCsv.RecordCount ();recNbr += 1)
	{
		bool isGeographic (false);
		bool isProjective (false);

		long tmpLong;
		wchar_t *wcDummy;
		const wchar_t *wcPtr;
		const TcsEpsgTable *epsgRefTblPtr;

		EcsNameFlavor flavor (csMapFlvrUnknown);
		EcsMapObjType objType (csMapNone);
	
		TcsGenericId srchCode;		// Default constructor initializes to 0UL, we're counting on that.
		TcsGenericId wkidCode;		// Default constructor initializes to 0UL, we're counting on that.
		TcsGenericId epsgId;		// Default constructor initializes to 0UL, we're counting on that.
		TcsEpsgCode epsgCode;		// Default constructor initializes to TcsEpsgCode::InvalidValue, we're counting on that.

		char wktText [2048];
		std::wstring fieldData;

		// Somewhat redundant, but this is a utility.  Performance and
		// efficiency are not serious objectives.
		memset (wktUnitName,'\0',sizeof (wktUnitName));
		memset (wktDatumName,'\0',sizeof (wktDatumName));
		memset (wktSpheroidName,'\0',sizeof (wktSpheroidName));
		memset (wktGeogCSName,'\0',sizeof (wktGeogCSName));
		memset (wktProjCSName,'\0',sizeof (wktProjCSName));
		memset (wktRlsLevel,'\0',sizeof (wktRlsLevel));
		memset (wktText,'\0',sizeof (wktText));
		fieldData.clear ();

		// We access the CRS reference table quite frequently.
		epsgRefTblPtr = epsgPtr->GetTablePtr (epsgTblReferenceSystem);

		// Get the flavor of the entry.
		ok = crsWktCsv.GetField (fieldData,recNbr,static_cast<short>(0),csvStatus);
		if (ok)
		{
			tmpLong = wcstol (fieldData.c_str (),&wcDummy,10);
			flavor = static_cast<EcsNameFlavor>(tmpLong);
		}
		// The EPSG code.
		if (ok)
		{
			ok = crsWktCsv.GetField (fieldData,recNbr,1,csvStatus);
			if (ok)
			{
				tmpLong = wcstol (fieldData.c_str (),&wcDummy,10);
				epsgCode = tmpLong;
				epsgId = tmpLong;
			}
		}
		// The WKID code (or SRID code. I.e. numeric code value for whatever
		// flavor we are dealing with.)
		if (ok)
		{
			ok = crsWktCsv.GetField (fieldData,recNbr,2,csvStatus);
			if (ok)
			{
				tmpLong = wcstol (fieldData.c_str (),&wcDummy,10);
				wkidCode = tmpLong;
			}
		}
		// The WKT release level, or what we will call the Flavored release
		// level.  That is, the release level of all information in the record
		// which pertains to the flavor identified in the first field.
		if (ok)
		{
			ok = crsWktCsv.GetField (fieldData,recNbr,3,csvStatus);
			if (ok)
			{
				wcsncpy (wktRlsLevel,fieldData.c_str (),wcCount (wktRlsLevel));
				wktRlsLevel [wcCount (wktRlsLevel) - 1] = L'\0';
			}
		}
		// Extract the WKT, and the parse it with the simple WKT parser.
		if (ok)
		{
			ok = crsWktCsv.GetField (fieldData,recNbr,4,csvStatus);
			if (ok)
			{
				wcstombs (wktText,fieldData.c_str (),sizeof (wktText));
			}
		}
		// Test the WKT.
		if (ok)
		{
			ok = CS_isWkt (wktText);
		}
		// Parse the WKT and extract the specific values of interest to us here.
		if (ok)
		{
			const TrcWktElement *wktElePtr;
			// At this initial point, wktElements will only be the top level element.
			// Later on, it will be morphed into a container of all the sub-elements,
			// thus the name is plural.
			TrcWktElement wktElements (wktText);
			ErcWktEleType wktEleType = wktElements.GetElementType ();
			ok = (wktEleType != rcWktUnknown);
			if (ok)
			{
				isGeographic = (wktEleType == rcWktGeogCS);
				isProjective = (wktEleType == rcWktProjCS);
				if (!(isGeographic || isProjective))
				{
					// It's neither geographic or projective.  We don't
					// know what it is.  Just skip it for now.
					continue;
				}
				
				// Establish the object type for this particular item.
				objType = isProjective ? csMapProjectedCSysKeyName : csMapGeographicCSysKeyName;

				// Parse the rest of the WKT definition.
				wktElements.ParseChildren ();
			}
			if (ok)
			{
				// Now we should be able to extract the remainder of what
				// interests us in this utility. Regardless of type, there
				// should be UNIT name, a DATUM name, and a SPHEROID name.
				wktElePtr = wktElements.ChildLocate (rcWktUnit);
				ok = (wktElePtr != 0);
				if (ok)
				{
					mbstowcs (wktUnitName,wktElePtr->GetElementNameC (),wcCount (wktUnitName));
				}
				if (ok && isGeographic)
				{
					wktElePtr = wktElements.ChildLocate (rcWktDatum);
					ok = (wktElePtr != 0);
					if (ok)
					{
						mbstowcs (wktDatumName,wktElePtr->GetElementNameC (),wcCount (wktDatumName));
						const TrcWktElement *lclWktPtr = wktElePtr->ChildLocate (rcWktSpheroid);
						ok = (lclWktPtr != 0);
						if (ok)
						{
							mbstowcs (wktSpheroidName,lclWktPtr->GetElementNameC (),wcCount (wktSpheroidName));
						}
					}
					mbstowcs (wktGeogCSName,wktElements.GetElementNameC (),wcCount (wktGeogCSName));
					wktProjCSName [0] = L'\0';
				}
				else if (ok && isProjective)
				{
					mbstowcs (wktProjCSName,wktElements.GetElementNameC (),wcCount (wktProjCSName));
					wktElePtr = wktElements.ChildLocate (rcWktGeogCS);
					ok = (wktElePtr != 0);
					if (ok)
					{
						mbstowcs (wktGeogCSName,wktElePtr->GetElementNameC (),wcCount (wktGeogCSName));
						wktElePtr = wktElePtr->ChildLocate (rcWktDatum);
						ok = (wktElePtr != 0);
						if (ok)
						{
							mbstowcs (wktDatumName,wktElePtr->GetElementNameC (),wcCount (wktDatumName));
							wktElePtr = wktElePtr->ChildLocate (rcWktSpheroid);
							ok = (wktElePtr != 0);
							if (ok)
							{
								mbstowcs (wktSpheroidName,wktElePtr->GetElementNameC (),wcCount (wktSpheroidName));
							}
						}
					}
				}
			}
		}

		// Data capture is done.  Now to put the captured data into the right places.
		// We start by updating the EPSG name for all systems for which we are given
		// an EPSG code.  The assumption here is that if flavor 'X' did something,
		// maybe EPSG did so as well.
		if (!ok)
		{
			// Bail out now if something went wrong.
			continue;
		}
		if (epsgCode.IsValid ())
		{
			// We have an EPSG code, so we presume it is worth checking the
			// EPSG name which is current in the NameMapper.
			ok = csNameMapperUpdateEpsg (epsgPtr,nameMapper,objType,epsgCode);
		}
		if (ok)
		{
			// Now, we need to deal with the flavored name and ID.  If we
			// have an EPSG code, wee need to match the flavored name and ID
			// with the existing (should exist due to the above) EPSG entry.
			if (epsgCode.IsValid ())
			{
				wcPtr = (objType ==  csMapProjectedCSysKeyName) ? wktProjCSName : wktGeogCSName;
				ok = csNameMapperUpdateFlvr (epsgPtr,nameMapper,objType,flavor,
																		epsgCode,
																		static_cast<unsigned long>(wkidCode),
																		wcPtr);

			}
			else
			{
				// No corresponding EPSG entry, yet another slightly different
				// update algorithm.
				wcPtr = (objType ==  csMapProjectedCSysKeyName) ? wktProjCSName : wktGeogCSName;
				ok = csNameMapperUpdateFlvr (epsgPtr,nameMapper,objType,flavor,
																		static_cast<unsigned long>(wkidCode),
																		wcPtr);
			}
		}
#ifdef _DEBUG
		if (!ok)
		{
			// For debugging.
			ok = true;
			recNbr -= 1;
		}
#endif
	}

	// If all of that succeeded, we write a new NameMapper.csv source
	// file out.
	if (ok)
	{
		std::wofstream oStrm1;
		std::wofstream oStrm2;

		wcscpy (fullPath,csDictTrgDir);
		wcscat (fullPath,L"\\");
		wcscat (fullPath,L"NameMapper.csv");
		oStrm1.open (fullPath,std::ios_base::out | std::ios_base::trunc);
		ok = oStrm1.is_open ();
		if (ok)
		{
			// WriteAsCsv does not return a status.  This is not so
			// hot; probably related to the fact that the operator<<
			// function is involved somehow.
			nameMapper.WriteAsCsv (oStrm1,true);
			oStrm1.close ();
		
			wcscpy (fullPath,csDictTrgDir);
			wcscat (fullPath,L"\\");
			wcscat (fullPath,L"NameMapperDuplicates.csv");
			oStrm2.open (fullPath,std::ios_base::out | std::ios_base::trunc);
			ok = oStrm2.is_open ();
			if (ok)
			{
				// WriteAsCsv does not return a status.  This is not so
				// hot; probably related to the fact that the operator<<
				// function is involved somehow.
				nameMapper.WriteDuplicates (oStrm2);
				oStrm2.close ();
			}
		}
	}
	return ok;
}

// Given an EPSG code of a CRS, the EPSG database, and the NameMapper, update
// the NameMapper to contain the latest EPSG name available.
bool csNameMapperUpdateEpsg (const TcsEpsgDataSetV6* epsgPtr,TcsNameMapper& nameMapper,
															 EcsMapObjType objType,
															 TcsEpsgCode epsgCode)
{
	bool ok;
	const wchar_t* wcPtrK;
	wchar_t aliasComment [256];

	TcsGenericId srchCode;
	std::wstring fieldData;

	ok = epsgPtr->GetFieldByCode (fieldData,epsgTblReferenceSystem,epsgFldCoordRefSysName,epsgCode);
	if (!ok)
	{
		// Oops!!!  We have a reference to a non-existent (perhaps
		// deprecated) code value.  Report it and then ignore the rest
		// of this entry.
		std::wcerr << L"WKT data has bogus reference to EPSG code ["
				   << static_cast<unsigned long>(epsgCode)
				   << L"]."
				   << std::endl;
		ok = true;
		return ok;
	}

	// See if the NameMapper already has this particular name on
	// record.  If the current EPSG database name is already in the
	// NameMapper, we assume that we should not try to update the
	// name (even if the name exists in the form of an alias).
	srchCode = nameMapper.Locate (objType,csMapFlvrEpsg,fieldData.c_str ());
	if (srchCode.IsNotKnown ())
	{
	 	// The current and very specific EPSG name is not in the
	 	// NameMapper.  Now we see if there is an entry for this
	 	// item but a with somewhat modified name.
	 	TcsGenericId epsgId (epsgCode);
		wcPtrK = nameMapper.LocateName (objType,csMapFlvrEpsg,epsgId);
		if (wcPtrK == 0)
		{
			// We don't have any record of this entry in the
			// NameMapper yet. The existing EPSG code variable will
			// suffice as the generic ID of the new item.
			TcsGenericId epsgId (epsgCode);
			TcsNameMap newItem (epsgId,objType,csMapFlvrEpsg,static_cast<unsigned long>(epsgCode),fieldData.c_str ());
			ok = nameMapper.Add (newItem);
		}
		else
		{
			if (wcsicmp (wcPtrK,fieldData.c_str ()))
			{
				// The names are different.
				swprintf (aliasComment,wcCount(aliasComment),L"EPSG changed this name in release %s.",epsgRlsLevel);
				ok = nameMapper.AliasExistingName (objType,csMapFlvrEpsg,static_cast<unsigned long>(epsgCode),
																		 fieldData.c_str (),
																		 aliasComment);
			}
			else
			{
				// The names are the same, nothing to do.
				ok = true;
			}
		}
	}
	return ok;
}
// This updates a flavor entry for which we have a matching EPSG code.
bool csNameMapperUpdateFlvr (const TcsEpsgDataSetV6* epsgPtr,TcsNameMapper& nameMapper,
															 EcsMapObjType objType,
															 EcsNameFlavor flavor,
															 TcsEpsgCode epsgCode,
															 unsigned long wkidCode,
															 const wchar_t* crsName)
{
	bool ok (true);
	const wchar_t* wcPtrK;

	TcsGenericId epsgId (epsgCode);

	wchar_t aliasComment [256];

	wcPtrK = nameMapper.LocateName (objType,flavor,epsgId);
	if (wcPtrK == 0)
	{
		// No flavored name exists related to the item identified by the EPSG
		// code.  There are two possible causes which are frequent enough
		// to deal with here:
		//  1> There is no entry at all for this particular item.
		//  2> The item exists but is not associated with the provided EPSG
		//     item and it has an older flavored ID associated with it.
		// In the case of 1 above, we simply add the item and associate it
		// with the item identified by the EPSG code.
		// In the case of 2 above, we see if we can locate the item by the
		// wkid code if provided, or the name (which must be provided).  If
		// we can locate such an item, we associate it with the item identified
		// by the EPSG code.
		//
		// We use the NameMapper extract to determine if the entry exists
		// because if it indeed does exist we wiull need to change it anyway.
		TcsNameMap extractedEntry;
		ok = nameMapper.ExtractAndRemove (extractedEntry,objType,flavor,crsName,0,0);
		if (ok)
		{
			// Extraction succeeded, thus there is an entry with this name.
			// Change the flavored numeric ID to that which has been given
			// to us and change the generic ID to the provided EPSG code.
			extractedEntry.SetGenericId (epsgId);
			extractedEntry.SetNameId (crsName);
			extractedEntry.SetNumericId (wkidCode);
			
			// Add the modified entry back into the name mapper.
			ok = nameMapper.Add (extractedEntry);
			if (!ok)
			{
				// Given how weird these data sets are, I suppose its likely
				// for this operation to fail occasionally.
				wcPtrK = TcsNameMapper::FlvrNbrToName (flavor);
				std::wcerr << L"Add failed: "
						   << wcPtrK
						   << L":"
						   << wkidCode
						   << L" \""
						   << crsName
						   << L"\"  [EPSG: "
						   << static_cast<unsigned long>(epsgCode)
						   << L"]."
						   << std::endl;
				ok = true;
			}
		}
		else
		{
			// OK, we assume this is a case of number one above; an entry
			// with this name for this flavor does not exist.
			TcsNameMap newItem (epsgId,objType,flavor,wkidCode,crsName);
			ok = nameMapper.Add (newItem);

			// If the Add failed for some reason, we record it and continue
			// on as some failures of this sort are entirely expected.
			if (!ok)
			{
				wcPtrK = TcsNameMapper::FlvrNbrToName (flavor);
				std::wcerr << L"Add failed: "
						   << wcPtrK
						   << L":"
						   << wkidCode
						   << L" \""
						   << crsName
						   << L"\"  [EPSG: "
						   << static_cast<unsigned long>(epsgCode)
						   << L"]."
						   << std::endl;
				ok = true;
			}
		}
	}
	else if (wcsicmp (wcPtrK,crsName))
	{
		// The appropriate entries exist, but the names are different.  We need
		// to adjust the name mapper.
		swprintf (aliasComment,wcCount (aliasComment),L"Name changed by flavor source in release %s.",wktRlsLevel);
		ok = nameMapper.AliasExistingName (objType,flavor,wkidCode,crsName,aliasComment);
		if (!ok)
		{
			// Another Oops!!! These problems are again caused by an EPSG
			// deprecation which is not picked up by ESRI (and probably
			// Oracle).  EPSG will deprecate a system if the name changes,
			// ESRI and Oracle will simply change the name and retain the
			// original WKID or SRID.
			std::wcerr << L"NameMapper has no entry for EPSG code "
					   << static_cast<unsigned long>(epsgId)
					   << L" as provided by the WKT Update source file."
					   << std::endl;
			ok = true;
		}
	}
	return ok;
}
// This updates a a flavored entry for which we do not have a matching EPSG code.
// Only difference is we need to generate an appropriate generic ID.
bool csNameMapperUpdateFlvr (const TcsEpsgDataSetV6* epsgPtr,TcsNameMapper& nameMapper,
															 EcsMapObjType objType,
															 EcsNameFlavor flavor,
															 unsigned long wkidCode,
															 const wchar_t* crsName)
{
	bool ok (true);
	bool hasName (false);
	bool nameOk (false);
	bool codeOk (false);
	const wchar_t* wcPtrK;

	TcsGenericId srchCodeNm;
	TcsGenericId srchCodeCd;

	wchar_t aliasComment [256];

	// Here for a flavored Name and ID, but no EPSG code to associate them
	// with.  If we don't have both a name and an ID, we skip this item as
	// there is no way to link this entry to anything else now or in the
	// future.
	hasName = (crsName != 0 && *crsName != L'\0');
	if (wkidCode != 0 && hasName)
	{
		// We have both a name and an ID.  Search the NameMapper
		// for an existing entry; preferring to use Name over ID.
		srchCodeNm = nameMapper.Locate (objType,flavor,crsName);
		srchCodeCd = nameMapper.Locate (objType,flavor,wkidCode);
		if (srchCodeNm.IsNotKnown() && srchCodeCd.IsNotKnown ())
		{
			ok = true;		// We can handle this situation.
			nameOk = codeOk = false;
		}
		else if (srchCodeNm.IsKnown() && srchCodeCd.IsKnown ())
		{
			// We located an entry using the name and code.  If they are
			// the same entry, then there is nothing to do.  If they are
			// not the same, then we have a problem.
			ok = (srchCodeNm == srchCodeCd);
			nameOk = codeOk = ok;
		}
		else
		{
			ok = true;
			nameOk = srchCodeNm.IsKnown();
			codeOk = srchCodeNm.IsKnown();
		}

		// Search code will be unknown if we could not find an entry by either
		// name or id number.
		if (ok && !nameOk && !codeOk)
		{
			TcsGenericId unknownId;
			TcsGenericId lclGenericId;
			// The entry is new, we simply add an appropriate entry to the
			// NameMapper.  Must use the NameMapper to get a generic ID to
			// use.  GenericID's must be unique, and we like to generate them
			// such that they contain flavor information in the new unique
			// code.  If a flavor ID has been given in the source data, we use
			// that to generate a generic ID to be used when creating the
			// new record.
			if (wkidCode == 0UL)
			{
				lclGenericId = nameMapper.GetNextDfltId (flavor);
			}
			else
			{
				lclGenericId = TcsGenericId (flavor,wkidCode);
			}

			// Add the new item.
			TcsNameMap nextItem (lclGenericId,objType,flavor,wkidCode,crsName);
			ok = nameMapper.Add (nextItem);
		}
		else if (ok && !nameOk && codeOk)
		{
			// An entry exists which was identified by the wkidCode value.
			// Update the name.  The assumptions here are:
			//  1> Since an entry exists, there must be a name.
			//     NameMapper entries are NOT required to have ID's,
			//     but are required to have names.
			//  2> If the name in the NameMapper was up to date, the
			//     name check above would have found it and we wouldn't
			//     be executing this block of code.
			swprintf (aliasComment,wcCount (aliasComment),L"Name changed by flavor source in release %s.",wktRlsLevel);
			ok = nameMapper.AliasExistingName (objType,flavor,wkidCode,crsName,aliasComment);
		}
		else if (ok && nameOk && !codeOk)
		{
			// The flavored numeric ID is not a part of the key, but we
			// pretend that it is in order to prevent a possible cut & paste
			// code error.
			TcsNameMap extractedItem;
			ok = nameMapper.ExtractAndRemove (extractedItem,objType,flavor,crsName,0,0);
			if (ok)
			{
				TcsGenericId genericId = extractedItem.GetGenericId ();
				if (genericId.IsKnown ())
				{
					// One might think we should look at the generic ID of this
					// extracted item since they often have the flavored numeric
					// ID buried in them.  However, if we did change that, we
					// would lose any association with other entries in the
					// NameMapper.  SO we leave it, and perhaps there will be
					// a utility to regenerate generic ID some day.
					extractedItem.SetNumericId (wkidCode);
					ok = nameMapper.Add (extractedItem);
				}
			}
			else
			{
				wcPtrK = TcsNameMapper::FlvrNbrToName (flavor);
				std::wcerr << L"Numeric ID Add failed: "
						   << wcPtrK
						   << L":"
						   << wkidCode
						   << L" \""
						   << crsName
						   << L"\"  [EPSG: 0]."
						   << std::endl;
				ok = true;
			}
		}
		else if (!ok)
		{
				wcPtrK = TcsNameMapper::FlvrNbrToName (flavor);
				std::wcerr << L"Out of sync: "
						   << wcPtrK
						   << L":"
						   << wkidCode
						   << L" \""
						   << crsName
						   << L"\"  [EPSG: 0]."
						   << std::endl;
				ok = true;
		}
	}
	return ok;
}
