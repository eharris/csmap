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

	Provides conversion of AGD66 geodetic coordinates to GDA94 coordinates.
	Handles multiple files in the Canadian National Transformation, Version 2,
	Australian variation, format.

*/

#include "cs_map.h"

/******************************************************************************
	The following is used to maintain the status of the AGD66<-->GDA94
	conversion system.  Namely, if its already opened, we don't have to
	open it again.  Upon close, we release resources, but don't actually
	destruct.  The counter keeps track of the number of opens.
*/
struct csAgd66ToGda94_* csAgd66ToGda94 = NULL;
int csAgd66ToGda94Cnt = 0;

/******************************************************************************
	Initialize the AGD66 <--> GDA94 conversion system.  The catalog file is
	expected to	reside in the basic data directory.  The name of the file is
	taken from the cs_Agd66Name global variable.
*/
int EXP_LVL7 CSagd66Init (void)
{
	extern char cs_Dir [];
	extern char* cs_DirP;
	extern char cs_Agd66Name [];

	char catalog [MAXPATH];

	if (csAgd66ToGda94 == NULL)
	{
		/* Set up the catalog file name.  Need a copy of it as the
		   new function will access CS_dtloc. */
		CS_stcpy (cs_DirP,cs_Agd66Name);
		CS_stncp (catalog,cs_Dir,sizeof (catalog));

		/* Use it to build the Agd66ToGda94 object. */
		csAgd66ToGda94 = CSnewAgd66ToGda94 (catalog);
		if (csAgd66ToGda94 == NULL) goto error;
	}
	csAgd66ToGda94Cnt += 1;
	return 0;

error:
	if (csAgd66ToGda94 != NULL)
	{
		CSdeleteAgd66ToGda94 (csAgd66ToGda94);
		csAgd66ToGda94 = NULL;
		csAgd66ToGda94Cnt = 0;
	}
	return -1;
}

/******************************************************************************
	Close the AGD66 <--> GDA94 conversion system.  Note, this only does a
	release, not a full delete.  This is for performance reasons.  During
	test, you may get a "memory leak message" because of this.  If this is
	undeirable, then change the 'release' function to the 'delete' function.
*/
void EXP_LVL7 CSagd66Cls (void)
{
	csAgd66ToGda94Cnt -= 1;
	if (csAgd66ToGda94Cnt <= 0)
	{
		if (csAgd66ToGda94 != NULL)
		{
			CSreleaseAgd66ToGda94 (csAgd66ToGda94);
			csAgd66ToGda94 = NULL;
		}
		csAgd66ToGda94Cnt = 0;
	}
	return;
}

/******************************************************************************
	Convert an AGD66 coordinate to GDA94 coordinate.

	Returns  0 for expected result
	        -1 for hard/fatal failure
			+1 for no data coverage, unshifted result returned
			+2 for no data coverage, fallback used successfully
*/
int EXP_LVL7 CSagd66ToGda94 (double ll_gda94 [3],Const double ll_agd66 [3])
{
	int status;
	double my_ll94 [3];

	/* We always do the null conversion. */
	my_ll94 [LNG] = ll_agd66 [LNG];
	my_ll94 [LAT] = ll_agd66 [LAT];
	my_ll94 [HGT] = ll_agd66 [HGT];

	/* Do the real conversion, if possible. */
	if (csAgd66ToGda94 == NULL)
	{
		CS_erpt (cs_DTC_NO_SETUP);
		status = -1;
	}
	else
	{
		status = CScalcAgd66ToGda94 (csAgd66ToGda94,my_ll94,ll_agd66);
	}
	ll_gda94 [LNG] = my_ll94 [LNG];
	ll_gda94 [LAT] = my_ll94 [LAT];
	ll_gda94 [HGT] = my_ll94 [HGT];
	return status;
}

/******************************************************************************
	Convert an GDA94 coordinate to AGD66 coordinate. Computationally, the
	inverse of the above.

	Returns  0 for expected result
	        -1 for hard/fatal failure
			+1 for no data coverage, unshifted result returned
			+2 for no data coverage, fallback used successfully
*/
int EXP_LVL7 CSgda94ToAgd66 (double ll_agd66 [3],Const double ll_gda94 [3])
{
	int status;

	double my_ll66 [3];

	/* We always do the null conversion. */
	my_ll66 [LNG] = ll_gda94 [LNG];
	my_ll66 [LAT] = ll_gda94 [LAT];
	my_ll66 [HGT] = ll_gda94 [HGT];

	/* Do the real conversion, if possible. */
	if (csAgd66ToGda94 == NULL)
	{
		CS_erpt (cs_DTC_NO_SETUP);
		status = -1;
	}
	else
	{
		status = CSinverseAgd66ToGda94 (csAgd66ToGda94,my_ll66,ll_gda94);
	}

	if (status >= 0)
	{
		ll_agd66 [LNG] = my_ll66 [LNG];
		ll_agd66 [LAT] = my_ll66 [LAT];
		ll_agd66 [HGT] = my_ll66 [HGT];
	}
	return status;
}
Const char * EXP_LVL7 CSagd66ToGda94Log (Const double ll_66 [2])
{
	Const char *cp;

	cp = NULL;
	/* Make sure we have been initialized. */
	if (csAgd66ToGda94 == NULL)
	{
		CS_erpt (cs_DTC_NO_SETUP);
	}
	else
	{
		cp = CSsourceAgd66ToGda94 (csAgd66ToGda94,ll_66);
	}
	return (cp == NULL || *cp == '\0') ? "<unavailable>" : cp;
}
/**********************************************************************
**	CS_agd66Name (new_name);
**
**	char *new_name;				the name of the Agd66ToGda94 catalog
**								file.
**
**	This function specifies the name only.  The directory,
**	and possibly drive, are specified using CS_altdr.
**********************************************************************/
void EXP_LVL1 CS_agd66Name (Const char *newName)
{
	extern char cs_Agd66Name [];

	CS_stncp (cs_Agd66Name,newName,cs_FNM_MAXLEN);
	return;
}
