
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
 *       contr butors may be used to endorse or promote products derived
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


// The following table associates a boundary with a ??hpgn.L?s file pair.
// Given that we initially create teo TcsLasLosFile objects as a 48hpgn.l?s
// file set, for each entry in the table immediately defined below, we:
//	1> activate the indicated boundary as a fence,
//	2> use 2 TcsLasLosFile objects to activate the ??hpgn.l?s fo;e pair, 
//	3> Iteracte through the longitude and latitude pairs for which a grid
//	   value exists in the ??hpgn.l?s pair,
//	4> determine, based on the longitude and latitude of the grid value point,
//	   if the point is within the boundary established in #1 above, and if so,
//	5> extract the grid values from the ??hpgn.l?s objects and insert these
//	   values into the 48Hpgn.L?s file pair.
// After all entries in the table have been processed, we write the 48hpgn.l?s
// file pair out to the dictionary directory and we're done.

// The above described process is straight forward and shold produce the
// desired ersult.  A potential problem remains, and for two distinct
// reasons. As of this writing, we only document the problem, plan to insert
// code to notify us if the problem actually arises, and plan to consider
// possible solutions only in the case where the problem does indeed arise.
//
// The problem is that at step 5, we could very likely be replacing an hpgn.l?s
// grid value extracted from a precious grid.  We will report this.  This is
// possible as the construction of a TcsLasLosFile object initializes the grid
// to an absured value.  Before inserting a value into the target grid, we will
// extract and evaluate the existing value.  If it is not the absurd
// initialization value, and the value is significantly different from the
// value that is toi be inserted, a detailed information message will be
// issued and the replacement skipped.

// There are two reasons why this situation can occur.
//	1> A boundary definition segment (implying its neighboring boundary as
//	   well) exactly traces or passes through a grid point.  Thus, a point
//	   in the target grid file set would be considered to exist within both
//	   boundaries.
//	2> In several cases (CA, TX, MT), there exists two sets of grid files
//	   associated with a single boundry, and which will overlap by as much as a
//	   full degree (i.e. four cells).  In these cases, the data points will be
//	   certainly be duplicated.  Not a problem if the grid cell values are
//	   essentially the same, a rather sticky wicket if they differ
//	   significantly.

// This program is unlikely to be used, officially, more than once.  So obvious
// situations where performance could be improved are totally ignored in favor
// of accuracy and reliability of the reult.

// Note: There are 50 sets of ??hpgn.l?s data files.  We are not concerned here
// with four of those sets:
//	hihpgn.l?s   -- Hawaii
//	pvhpgn.l?s   -- Puerto Rico and Virgin Islands
//	wshpgn.l?s   -- Western Samoa (actually Western American Samoa)
//	wshpgn.l?s   -- Eastern Samoa (actually Eastern American Samoa)
//
// Thus, to build a complete 48hpgn.l?s, we need 46 entries.
//
struct TcsHpgn48Generate
{
	char BoundaryFile [80];
	char HpgnFilePair [80];
} KcsHpgn48Generate [] =
{
	{ "EPSGPolygon_1372_2013-06-15.xml", "alhpgn.l?s"   },	// AL
	{ "EPSGPolygon_1373_2013-06-15.xml", "azhpgn.l?s"   },	// AZ
	{ "EPSGPolygon_1374_2013-06-15.xml", "arhpgn.l?s"   },	// AR
	{ "EPSGPolygon_2297_2013-06-15.xml", "cnhpgn.l?s"   },	// CA North of 36.5
	{ "EPSGPolygon_2298_2013-06-15.xml", "cshpgn.l?s"   },	// CA South of 36.5
	{ "EPSGPolygon_1376_2013-06-15.xml", "cohpgn.l?s"   },	// CO
	{ "EPSGPolygon_2378_2013-06-15.xml", "nehpgn.l?s"   },	// CT, MA, NH, RI, VT
	{ "EPSGPolygon_2377_2013-06-15.xml", "mdhpgn.l?s"   },	// DE and MD
	{ "EPSGPolygon_1379_2013-06-15.xml", "flhpgn.l?s"   },	// FL
	{ "EPSGPolygon_1380_2013-06-15.xml", "gahpgn.l?s"   },	// GA
	{ "EPSGPolygon_1381_2013-06-15.xml", "idhpgn.l?s"   },	// ID
	{ "EPSGPolygon_1382_2013-06-15.xml", "ilhpgn.l?s"   },	// IL
	{ "EPSGPolygon_1383_2013-06-15.xml", "inhpgn.l?s"   },	// IN
	{ "EPSGPolygon_1384_2013-06-15.xml", "iahpgn.l?s"   },	// IA
	{ "EPSGPolygon_1385_2013-06-15.xml", "kshpgn.l?s"   },	// KS
	{ "EPSGPolygon_1386_2013-06-15.xml", "kyhpgn.l?s"   },	// KY
	{ "EPSGPolygon_1387_2013-06-15.xml", "lahpgn.l?s"   },	// LA
	{ "EPSGPolygon_1388_2013-06-15.xml", "mehpgn.l?s"   },	// ME  (Maine)
	{ "EPSGPolygon_1391_2013-06-15.xml", "mihpgn.l?s"   },	// MI
	{ "EPSGPolygon_1392_2013-06-15.xml", "mnhpgn.l?s"   },	// MN
	{ "EPSGPolygon_1393_2013-06-15.xml", "mshpgn.l?s"   },	// MS
	{ "EPSGPolygon_1394_2013-06-15.xml", "mohpgn.l?s"   },	// MO
	{ "EPSGPolygon_1395_2013-06-15.xml", "emhpgn.l?s"   },	// MT East (113W -> 103W)
	{ "EPSGPolygon_1395_2013-06-15.xml", "wmhpgn.l?s"   },	// MT East (119W -> 109W)
	{ "EPSGPolygon_1396_2013-06-15.xml", "nbhpgn.l?s"   },	// Nebraska (NE? -- nehpgn is New England)
	{ "EPSGPolygon_1397_2013-06-15.xml", "nvhpgn.l?s"   },	// NV
	{ "EPSGPolygon_1399_2013-06-15.xml", "njhpgn.l?s"   },	// NJ
	{ "EPSGPolygon_1400_2013-06-15.xml", "nmhpgn.l?s"   },	// NM
	{ "EPSGPolygon_1401_2013-06-15.xml", "nyhpgn.l?s"   },	// NY
	{ "EPSGPolygon_1402_2013-06-15.xml", "nchpgn.l?s"   },	// NC
	{ "EPSGPolygon_1403_2013-06-15.xml", "ndhpgn.l?s"   },	// ND
	{ "EPSGPolygon_1404_2013-06-15.xml", "ohhpgn.l?s"   },	// OH
	{ "EPSGPolygon_1405_2013-06-15.xml", "okhpgn.l?s"   },	// OK
	{ "EPSGPolygon_2381_2013-06-15.xml", "wohpgn.l?s"   },	// WA and OR
	{ "EPSGPolygon_1407_2013-06-15.xml", "pahpgn.l?s"   },	// PA
	{ "EPSGPolygon_1409_2013-06-15.xml", "schpgn.l?s"   },	// SC
	{ "EPSGPolygon_1410_2013-06-15.xml", "sdhpgn.l?s"   },	// SD
	{ "EPSGPolygon_1411_2013-06-15.xml", "tnhpgn.l?s"   },	// TN
	{ "EPSGPolygon_2379_2013-06-15.xml", "ethpgn.l?s"   },	// TX east of 100W
	{ "EPSGPolygon_2380_2013-06-15.xml", "wthpgn.l?s"   },	// TX west of 100W
	{ "EPSGPolygon_1413_2013-06-15.xml", "uthpgn.l?s"   },	// UT
	{ "EPSGPolygon_1415_2013-06-15.xml", "vahpgn.l?s"   },	// VA
	{ "EPSGPolygon_1417_2013-06-15.xml", "wvhpgn.l?s"   },	// WV
	{ "EPSGPolygon_1418_2013-06-15.xml", "wihpgn.l?s"   },	// WI
	{ "EPSGPolygon_1419_2013-06-15.xml", "wyhpgn.l?s"   },	// WY
	{                                "", ""             }
};

bool csGenerate48Hpgn (const wchar_t* csDictDir,const wchar_t* epsgPolygonDir)
{
	bool ok (false);

	// Note, the status checking here is primarily to make sure that success is
	// truly success.  That is, when we get through this whole thing with the
	// ok variable remaining true, we will feel confident that the result is
	// valid.

	// Create two objects in which we will build the results.  Note tha upon
	// construction, all grid values are set to a specific value which is
	// easy to identify as being "not set"; i.e. a hole in the grid which was
	// not set by all the code below.
	TcsLasLosFile Hpgn48Los (-125.0,24.0,0.25,105,225);		// Longitude file
	Hpgn48Los.SetFileIdent ("Composite 48 state HARN Grid by osgeo.org (unoffical)");
	Hpgn48Los.SetProgram ("CS-MAP");
	Hpgn48Los.SetZeeCount (0L);
	Hpgn48Los.SetAngle (0.0);

	TcsLasLosFile Hpgn48Las (-125.0,24.0,0.25,105,225);		// Latitude file
	Hpgn48Las.SetFileIdent ("Composite 48 state HARN Grid by osgeo.org (unoffical)");
	Hpgn48Las.SetProgram ("CS-MAP");
	Hpgn48Las.SetZeeCount (0L);
	Hpgn48Las.SetAngle (0.0);

	// Start a loop, once for each entry in the above table.
	ok = true;						// To get the loop started.
	TcsHpgn48Generate* tblPtr;
	for (tblPtr = KcsHpgn48Generate;ok & tblPtr->BoundaryFile [0] != '\0';tblPtr += 1)
	{
		// Here once for each entry in the table.  Construct two TcsLasLosFile
		// objects; one for the source los file, the second for the source las
		// file.
		char losSourceFilePath [MAXPATH];
		char lasSourceFilePath [MAXPATH];

		CS_stncp  (losSourceFilePath,hpgnDirPath,sizeof (losSourceFilePath));
		CS_stncat (losSourceFilePath,"\\",sizeof (losSourceFilePath));
		CS_stncat (losSourceFilePath,tblPtr->HpgnFilePair,sizeof (losSourceFilePath));
		CS_strrpl (losSourceFilePath,"l?s","los");
		CS_stncp  (lasSourceFilePath,hpgnDirPath,sizeof (lasSourceFilePath));
		CS_stncat (lasSourceFilePath,"\\",sizeof (lasSourceFilePath));
		CS_stncat (lasSourceFilePath,tblPtr->HpgnFilePair,sizeof (lasSourceFilePath));
		CS_strrpl (lasSourceFilePath,"l?s","las");

		TcsLasLosFile* sourceLos = new TcsLasLosFile ();
		TcsLasLosFile* sourceLas = new TcsLasLosFile ();
		ok  = sourceLos->ReadFromFile (losSourceFilePath);
		ok &= sourceLas->ReadFromFile (lasSourceFilePath);

		if (ok)
		{
			// Create a fence object using the polgon specified in the driving
			// table.
			CS_stncp  (fenceSourceFilePath,epsgPolygonDirPath,sizeof (fenceSourceFilePath));
			CS_stncat (fenceSourceFilePath,"\\",sizeof (fenceSourceFilePath));
			CS_stncat (fenceSourceFilePath,tblPtr->BoundaryFile,sizeof (fenceSourceFilePath));

			TcsFence* fencePtr = new TcsFence (fenceSourceFilePath);
			ok = fencePtr->IsOk ();
		}

		if (ok)
		{
			// Extract the dimensions of the source HPGN grid.
			long32_t recCount = sourceLos->GetRecordCount ();
			long32_t eleCount = sourceLos->GetRecordCount ();
		}

		// Outer loop, once for each record in the hpdgn source file.
		for (long32_t recIdx = 0;ok && recIdx < recCount;recIdx += 1)
		{
			// Inner Loop, once for each element in each record.
			for (long32_t eleIdx = 0;ok && eleIdx < eleCount;eleIdx += 1)
			{
				double gridValueLos;
				double gridValueLas;
				double gridValueLos48;
				double gridValueLss48;
				double gridLL [2];

				// Get the grid value and the latitude and longitude associated
				// with the current index values.
				ok  = sourceLos->GetGridLocation (gridLL [2],recIdx,eleIdx);
				ok &= sourceLos->GetGridValue (gridValueLos,recIdx,eleIdx);
				ok &= sourceLas->GetGridValue (gridValueLas,recIdx,eleIdx);
				if (ok)
				{
					// See if the location of this grid point is within, or on,
					// the boundary defined by the fence.  If not, we just skip
					// this particular point.
					bool isIn = fencePtr->IsInside (gridLL);
					if (!isIn) continue;

					// See if the LL is within the coverage of the 48hpgn file.
					isIn = Hpgn48Los.IsCovered (gridLL);
					if (!isIn) continue;

					// Get the grid values from 48hpgn los.
					ok  = Hpgn48Los.GetGridValue (gridValueLos48,gridLL [0],gridLL [1]);
					ok &= Hpgn48Las.GetGridValue (gridValueLas48,gridLL [0],gridLL [1]);
				}
				if (ok)
				{
					// See if the new 48hpgn data set already has a value for
					// this grid point.
					if (gridValueLos48 < Hpgn48Los.ValueTest)
					{
						// A value for this specific point has already been
						// set.  We have duplicate hpgn data points.  Not a
						// problem if the grid values are essentially the
						// same.
						if ((fabs (gridValueLas - gridValueLas48) > 1.0E-4) ||
							(fabs (gridValueLos - gridValueLos48) > 1.0E-04))
						{
							// OK, we have a problem.  We have multiple values
							// for this grid point, and the values are
							// sufficiently different for us to be concerned.
							std::wcerr << L"Multiple and different grid values encountered for "
									   << -gridLL [0]
									   << L"W and "
									   << gridLL [1]
									   << L" while processing file "
									   << tblPtr->HpgnFilePair
									   << L"."
									   << std::endl;
							continue;
						}
					}
					// If we are still here, all is OK.  We set the
					// grid values as appropriate.
					ok  = Hpgn48Los.SetGridValue (gridLL [0],gridLL [1],gridValueLos);
					ok &= Hpgn48Las.SetGridValue (gridLL [0],gridLL [1],gridValueLas);
				}
			}
		}
		// On to next table entry.  sourceLas and sourceLos should have
		// go out of scope and therefore will be deleted before the next
		// iteration starts.
	}

	// Fill in result edges.
	if (ok)
	{
	}

	// Check the result for holes.
	if (ok)
	{
	}

	// Write out the 478hpgn.l?s files.
	if (ok)
	{
		// COnstruct the file name path for the resulting los file.
		ok  = Hpgn48Los.WriteToFile (hpgn48LosPath);
		ok &= Hpgn48Las.WriteToFile (hpgn48LasPath);
	}

	// Return the status of this entire result.;
	return ok;
}
