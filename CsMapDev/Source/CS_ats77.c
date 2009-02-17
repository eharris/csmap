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

/*		       * * * * R E M A R K S * * * *

	Provides conversion of ATS77 geodetic coordinates to NAD83 coordinates.
	Handles multiple files in the Canadian National Transformation, Version 2,
	Australian variation, format.
*/

#include "cs_map.h"

/******************************************************************************
	The following is used to maintain the status of the ATS77<-->NAD83
	conversion system.  Namely, if its already opened, we don't have to
	open it again.  Upon close, we release resources, but don't actually
	destruct.  The counter keeps track of the number of opens.
*/
struct csAts77ToCsrs_* csAts77ToCsrs = NULL;
int csAts77ToCsrsCnt = 0;

/******************************************************************************
	Initialize the ATS77 <--> CSRS conversion system.  The catalog file is
	expected to	reside in the basic data directory.  The name of the file is
	taken from the cs_Ats77Name global variable.
*/
int EXP_LVL7 CSats77Init (void)
{
	extern char cs_Dir [];
	extern char* cs_DirP;
	extern char cs_Ats77Name [];

	char catalog [MAXPATH];

	if (csAts77ToCsrs == NULL)
	{
		/* Set up the catalog file name. */
		CS_stcpy (cs_DirP,cs_Ats77Name);
		CS_stncp (catalog,cs_Dir,sizeof (catalog));

		/* Use it to build the Ats77ToCsrs object. */
		csAts77ToCsrs = CSnewAts77ToCsrs (catalog);
		if (csAts77ToCsrs == NULL) goto error;
	}
	csAts77ToCsrsCnt += 1;
	return 0;

error:
	if (csAts77ToCsrs != NULL)
	{
		CSdeleteAts77ToCsrs (csAts77ToCsrs);
		csAts77ToCsrs = NULL;
		csAts77ToCsrsCnt = 0;
	}
	return -1;
}

/******************************************************************************
	Close the ATS77 <--> CSRS conversion system.  Note, this only does a
	release, not a full delete.  This is for performance reasons.  During
	test, you may get a "memory leak message" because of this.  If this is
	undesirable, then change the 'release' function to the 'delete' function.
*/
void EXP_LVL7 CSats77Cls (void)
{
	csAts77ToCsrsCnt -= 1;
	if (csAts77ToCsrsCnt <= 0)
	{
		if (csAts77ToCsrs != NULL)
		{
			CSreleaseAts77ToCsrs (csAts77ToCsrs);
			csAts77ToCsrs = NULL;
		}
		csAts77ToCsrsCnt = 0;
	}
	return;
}

/******************************************************************************
	Convert an ATS77 coordinate to CSRS coordinate.
	Returns  0 for expected result
	        -1 for hard/fatal failure
			+1 for no data coverage, unshifted result returned
			+2 for no data coverage, fallback used successfully
*/
int EXP_LVL7 CSats77ToCsrs (double ll_csrs [3],Const double ll_ats77 [3])
{
	int status;
	double my_csrs [3];

	/* We always do the null conversion. */
	my_csrs [LNG] = ll_ats77 [LNG];
	my_csrs [LAT] = ll_ats77 [LAT];
	my_csrs [HGT] = ll_ats77 [HGT];

	/* Do the real conversion, if possible. */
	if (csAts77ToCsrs == NULL)
	{
		CS_erpt (cs_DTC_NO_SETUP);
		status = -1;
	}
	else
	{
		status = CScalcAts77ToCsrs (csAts77ToCsrs,my_csrs,ll_ats77);
	}
	ll_csrs [LNG] = my_csrs [LNG];
	ll_csrs [LAT] = my_csrs [LAT];
	ll_csrs [HGT] = my_csrs [HGT];
	return status;
}

/******************************************************************************
	Convert an CSRS coordinate to ATS77 coordinate. Computationally, the
	inverse of the above.
	Returns  0 for expected result
	        -1 for hard/fatal failure
			+1 for no data coverage, unshifted result returned
			+2 for no data coverage, fallback used successfully
*/
int EXP_LVL7 CScsrsToAts77 (double ll_ats77 [3],Const double ll_csrs [3])
{
	int status;

	double my_ats77 [3];

	/* We always do the null conversion. */
	my_ats77 [LNG] = ll_csrs [LNG];
	my_ats77 [LAT] = ll_csrs [LAT];
	my_ats77 [HGT] = ll_csrs [HGT];

	/* Do the real conversion, if possible. */
	if (csAts77ToCsrs == NULL)
	{
		CS_erpt (cs_DTC_NO_SETUP);
		status = -1;
	}
	else
	{
		status = CSinverseAts77ToCsrs (csAts77ToCsrs,my_ats77,ll_csrs);
	}

	if (status >= 0)
	{
		ll_ats77 [LNG] = my_ats77 [LNG];
		ll_ats77 [LAT] = my_ats77 [LAT];
		ll_ats77 [HGT] = my_ats77 [HGT];
	}
	return status;
}
Const char * EXP_LVL7 CSats77ToCsrsLog (Const double ll_77 [2])
{
	Const char *cp;

	cp = NULL;
	/* Make sure we have been initialized. */
	if (csAts77ToCsrs == NULL)
	{
		CS_erpt (cs_DTC_NO_SETUP);
	}
	else
	{
		cp = CSsourceAts77ToCsrs (csAts77ToCsrs,ll_77);
	}
	return (cp == NULL || *cp == '\0') ? "<unavailable>" : cp;
}
/**********************************************************************
**	CS_ats77Name (new_name);
**
**	char *new_name;				the name of the Ats77ToCsrs catalog
**								file.
**
**	This function specifies the name only.  The directory,
**	and possibly drive, are specified using CS_altdr.
**********************************************************************/
void EXP_LVL1 CS_ats77Name (Const char *newName)
{
	extern char cs_Ats77Name [];

	CS_stncp (cs_Ats77Name,newName,cs_FNM_MAXLEN);
	return;
}
