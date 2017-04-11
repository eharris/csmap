/*
 * Copyright (c) 2008, Autodesk, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Autodesk, Inc. nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY Autodesk, Inc. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Autodesk, Inc. OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*=============================================================================
	While a useful and necessary module, this test is rarely run, and then only
	in a test environment.  The purpose of this test is to locate regressions
	in the life time of the library.  Thus, checking and reporting of errors is
	limited to the integrity of the process.
=============================================================================*/

#include "csTestCpp.hpp"

bool csBuildCrsList (TcsCrsList& crsList);
bool csBuildLlCrsList (TcsCrsList& crsList);
bool csBuildLlTestList (TcsTestList& llCrsList,const TcsCrsList& lltestList,bool threeD,unsigned& testNbr);
bool csBuildCrsTestList (TcsTestList& testList,const TcsCrsList& crsTestList,unsigned& testNbr);
bool csGenerateTestFile (std::wofstream& oStream,const TcsTestList& testList);

// This module is not actually a test.  It is preparation for a test. That is,
// use this test mode to generate a regression test data files in the format
// specified by "www.OsGeo.org/MetaCRS/testsuite".
// 
// TestR of this test module will actually perform the test, comparing the
// version of the library with which this Test module is currently link with
// the result produced by a previous version of the library.  Production of
// this regression test data file is the purpose of this particular module.

char csModuleName [] = "CStestQ.cpp";

int CStestQ (bool verbose,long32_t duration,char *test_file,int revision)
{
	bool ok (true);

	unsigned testNbr;

	char idPrefix [64];

	std::wofstream regressStrm;

	TcsCrsList crsList;			// collection of Cartesian CRS's to be tested.
	TcsCrsList llCrsList;		// collection of geographic CRS's to be tested

	TcsTestList testList;		// collection of paired test CRS's to be tested.

	printf ("[Q] Generating regression test data file %s.\n",test_file);

	testNbr = 1UL;			// Initial test ID value.

	crsList.reserve   (3 * 1024);
	llCrsList.reserve (2 * 1024);
	testList.reserve  (5 * 1024);

	// Set the PointID prefix for all tests generated by this execution.
	sprintf (idPrefix,"CsMap:%d",revision);
	TcsTestCase::SetIdPrefix (idPrefix);

	// Generate a list of geographic systems which are to be tested.
	ok &= csBuildLlCrsList (llCrsList);
	ok &= csBuildCrsList (crsList);

	// For each geographic system in the llCrsList, we generate a test for
	// all other geographic systems which have overlapping coverages.  Note
	// the deliberate inclusion of each pair twice, as the second copy is
	// expected to be the inverse of the first.

	// This is a test feature of the library, so performance is not crucial.
	// We generate 2D and 3D lists separately to make adjustments between the
	// two easier to code as they become necessary. This function always adds
	// to the supplied list TcsTestList which is presumed, at this point, to be
	// empty.
	if (ok)
	{
		ok = csBuildLlTestList (testList,llCrsList,false,testNbr);	// 2D generation
	}
	if (ok)
	{
		ok = csBuildLlTestList (testList,llCrsList,true,testNbr);	// 3D generation
	}
	if (ok)
	{
		// Build a list of the Projective CRS's to be tested.
		ok = csBuildCrsTestList (testList,crsList,testNbr);
	}

	// OK, write the 'testList' collection to a file with the provided name.
	if (ok)
	{
		regressStrm.open (test_file,std::ios_base::out | std::ios_base::trunc);
		if (regressStrm.is_open ())
		{
			ok = csGenerateTestFile (regressStrm,testList);
			regressStrm.close ();
		}
	}
	return ok ? 0 : -1;
}
// Generate a list of geographic coordinate systems to be tested.  We
// skip LEGACY definitions, and those definitions which do not have
// a useful range specification.  This list includes information necessary
// for constructing tests, and reformatting test specifications to ASCII.
//
bool csBuildLlCrsList (TcsCrsList& crsList)
{
	bool ok;
	int llCount;
	int skipCount = 0;

	cs_Csgrplst_ *grpListPtr = 0;
	cs_Csgrplst_ *grpListHdr = 0;

	// Loop through the coordinate system dictionary and generate a list of all
	// geographic coordinate systems. We skip 'LEGACY' definitions.  If a useful
	// range is not defined, we also skip it; we can't really test it well if
	// we don't know where it is supposed to be be supported.
	llCount = CS_csgrp ("LL",&grpListHdr);
	ok = (grpListHdr != 0 && llCount >= 0);
	if (!ok)
	{
		printf ("Internal error: %s::%d.\n",csModuleName,__LINE__);
	}
	for (grpListPtr = grpListHdr;ok && grpListPtr != NULL;grpListPtr = grpListPtr->next)
	{
		TcsTestCrs nextCrs (grpListPtr->key_nm);

		// Check proper construction.  This should never happen.
		if (*nextCrs.GetCrsName () == '\0')
		{
			ok = false;
			printf ("Internal error: %s::%d.\n",csModuleName,__LINE__);
			continue;				/*lint !e845 */
		}

		// If the system is not geographic, there is a 'GROUP' error.
		if (!nextCrs.IsGeo ())
		{
			printf ("System named '%s' is in the wrong group; skipping it.\n",grpListPtr->key_nm);
			continue;
		}

		// If the useful range is not specified, we skip this definition.
		if (nextCrs.GetRange().IsNull ())
		{
			skipCount += 1;
			continue;
		}

		// Add this entry to the projected CRS test vector.
		crsList.push_back (nextCrs);
	}

	CS_csgrpf (grpListHdr);

	if (skipCount != 0)
	{
		printf ("Detected %d geographic systems without a useful range.\n",skipCount);
	}
	return ok;
}

// Construct a list of actual geographic system tests.  This is, essentially,
// the geodetic transformation portion of the regression test.  So, we create
// a forward test for each pair of systems which have intersecting useful
// ranges.  The algorithm is brute force, which produces the benefit of
// producing a inverse for each forward.
//
bool csBuildLlTestList (TcsTestList& llTestList,const TcsCrsList& llCrsList,
												bool threeD,
												unsigned& testNbr)
{
	bool ok (true);

	TcsCrsList::const_iterator srcItr;
	TcsCrsList::const_iterator trgItr;

	EcsTestMethod testType;

	testType = threeD ? testMthCrs3D : testMthCrs2D;

	for (srcItr = llCrsList.begin ();srcItr != llCrsList.end ();srcItr++)
	{
		for (trgItr = llCrsList.begin ();trgItr != llCrsList.end ();trgItr++)
		{
			if (srcItr == trgItr)
			{
				continue;
			}
			TcsCrsRange intersection (srcItr->GetRange ());
			intersection.ShrinkTo (trgItr->GetRange ());
			if (!intersection.IsNull ())
			{
				TcsTestCase nextTest (testType,*srcItr,*trgItr,testNbr);
				// We skip tests with a non-zero status value primarily as
				// a filter on automatically generated test points which are
				// really bogus.  This situation is comman as the useful range
				// of a lot of CRS definitions is overly generous.
				if (nextTest.GetStatus () == 0)
				{
					// Provide a progress indication.
					if ((testNbr % 100) == 0)
					{
						printf ("Generating: %s            \r",nextTest.GetPointID());
						fflush (stdout);
					}

					llTestList.push_back (nextTest);
					testNbr += 1;
				}
			}
		}
	}
	return ok;
}

// Build a list of projected CRS's which are to be tested.  Essentially, we
// filter out CRS's which are in specific groups such as LEGACY, TEST, etc.
// The resulting list includes the useful range and information helpful
// when converting the test data to ASCII form.
bool csBuildCrsList (TcsCrsList& crsList)
{
	bool ok (true);

	int idx;
	int crsCount;
	int grpIdx;
	int skipCount = 0;

	cs_Csgrplst_ *grpListPtr = 0;
	cs_Csgrplst_ *grpListHdr = 0;

	// This is the list of groups which we filter out of the list.
	static const char *grpSupList [] =
	{
		"LL",					// we do these separately.
		"LEGACY",
		"TEST",
		"WKTSUPPT",
		"NERTH",
		"EPSGLL",
		"EPSGPRJ",
		"USER",
		"NONE",
		""
	};

	char grpName [64];

	grpIdx = 0;
	// Loop once for each group in the group list.
	while (ok && CS_csGrpEnum (grpIdx,grpName,sizeof (grpName),NULL,0) != 0)
	{
		// This will get the next group on the next iteration.
		grpIdx += 1;

		// Skip over any group in the suppression list.
		for (idx = 0;*grpSupList [idx] != '\0';idx += 1)
		{
			if (!CS_stricmp (grpName,grpSupList[idx]))
			{
				break;
			}
		}
		if (*grpSupList [idx] != '\0')
		{
			// We don't test systems in this group.
			continue;
		}

		// Expand the group.
		crsCount = CS_csgrp (grpName,&grpListHdr);

		// Groups can be empty, we presume.
		if (grpListHdr == 0 || crsCount <= 0)
		{
			continue;
		}

		// Loop once for each CRS in the group.
		for (grpListPtr = grpListHdr;ok && grpListPtr != NULL;grpListPtr = grpListPtr->next)
		{
			TcsTestCrs nextCrs (grpListPtr->key_nm);

			// Check proper construction.  This should never happen.
			if (*nextCrs.GetCrsName () == '\0')
			{
				ok = false;
				printf ("Internal error: %s::%d.\n",csModuleName,__LINE__);
				continue;				/*lint !e845 */
			}

			// If the system is geographic, there is a 'GROUP' error.
			if (nextCrs.IsGeo ())
			{
				printf ("System named '%s' is in the wrong group; skipping it.\n",grpListPtr->key_nm);
				continue;
			}

			// If the useful range is not specified, we skip this definition.
			if (nextCrs.GetRange().IsNull ())
			{
				skipCount += 1;
				continue;
			}

			// Add this entry to the projected CRS test vector.
			crsList.push_back (nextCrs);
		}
		CS_csgrpf (grpListHdr);
	}
	if (skipCount != 0)
	{
		printf ("Detected %d projected systems without a useful range.\n",skipCount);
	}
	return ok;
}
bool csBuildCrsTestList (TcsTestList& testList,const TcsCrsList& crsTestList,
													 unsigned& testNbr)
{
	bool ok (true);

	TcsCrsList::const_iterator crsItr;

	// Create a TcsCrsTest object to serve as the target CRS for all
	// projective system tests.
	TcsTestCrs prjTargetCrs ("LL");
	if (*prjTargetCrs.GetCrsName() == '\0')
	{
		printf ("Internal error: %s::%d.\n",csModuleName,__LINE__);
		return false;
	}

	// Construct a test for each projective coordinate system converting
	// directly to the 'LL' coordinate system.  Thus, such tests are
	// completely independent of geodetic transformations and associated
	// issues. That is, this series of tests are specific to regressions
	// in projective code only, and are all two dimensional in nature.
	for (crsItr = crsTestList.begin ();ok && crsItr != crsTestList.end ();crsItr++)
	{
		TcsTestCase nextTestFwd (testMthCrs2D,*crsItr,prjTargetCrs,testNbr);
		if (nextTestFwd.GetStatus () == 0)
		{
			// Provide a progress indication.
			if ((testNbr % 100) <= 1)
			{
				printf ("Generating: %s           \r",nextTestFwd.GetPointID());
				fflush (stdout);
			}		// Build the CRS to LL test case.

			testList.push_back (nextTestFwd);
			testNbr += 1;
		}

		// Swap the source and target to generate an inverse of the above test.
		// NOTE: this uses an unconventional "inverse copy constructor"
		// distinguished from the normal copy constructor by the first argument.
		TcsTestCase nextTestInv (testNbr,nextTestFwd);
		if (nextTestInv.GetStatus () == 0)
		{
			testList.push_back (nextTestInv);
			testNbr += 1;
		}
	}
	return ok;
}
bool csGenerateTestFile (std::wofstream& regressStrm,const TcsTestList& testList)
{
	bool ok (true);
	unsigned testIdx;

	TcsTestList::const_iterator testItr;

	TcsCsvStatus status;

	TcsOsGeoTestFile::WriteLabels (regressStrm);		/*lint !e534 */

	// Loop once for each entry in the provided test lest.
	testIdx = 0;
	for (testItr = testList.begin ();ok && testItr != testList.end ();testItr++)
	{
		if (testItr->GetStatus () == 0)
		{
			TcsCsvRecord nextTest (15,18);
			
			testItr->ToCsvRecord (nextTest);
			nextTest.WriteToStream (regressStrm,status);		/*lint !e534 */
			ok = regressStrm.good ();
			testIdx += 1;

			// Provide a progress indication.
			if ((testIdx %1000) == 0)
			{
				printf ("Writing: %s           \r",testItr->GetPointID());
				fflush (stdout);
			}
		}
	}
	return ok;
}