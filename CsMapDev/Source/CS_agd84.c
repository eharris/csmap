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

	Provides conversion of AGD84 geodetic coordinates to GDA94 coordinates.
	Handles multiple files in the Canadian National Transformation, Version 2,
	Australian variation, format.  If coverage can't be found in the data
	files, the default calculation, using 7 parameter technique, is
	performed and a +1 status value is returned.
*/

#include "cs_map.h"

/******************************************************************************
	The following is used to maintain the status of the Agd84<-->GDA94
	conversion system.  Namely, if its already opened, we don't have to
	open it again.  Upon close, we release resources, but don't actually
	destruct.  The counter keeps track of the number of opens.
*/
struct csAgd84ToGda94_* csAgd84ToGda94 = NULL;
int csAgd84ToGda94Cnt = 0;

/******************************************************************************
	Initialize the AGD84<-->GDA94 conversion system.  The catalog file is
	expected to	reside in the basic data directory.  The name of the file is
	taken from the cs_Agd84Name global variable.
*/
int EXP_LVL7 CSagd84Init (void)
{
	extern char cs_Dir [];
	extern char* cs_DirP;
	extern char cs_Agd84Name [];

	char catalog [MAXPATH];

	if (csAgd84ToGda94 == NULL)
	{
		/* Set up the catalog file name. */
		CS_stcpy (cs_DirP,cs_Agd84Name);
		CS_stncp (catalog,cs_Dir,sizeof (catalog));

		/* Use it to build the Agd84ToGda94 object. */
		csAgd84ToGda94 = CSnewAgd84ToGda94 (catalog);
		if (csAgd84ToGda94 == NULL) goto error;
	}
	csAgd84ToGda94Cnt += 1;
	return 0;

error:
	if (csAgd84ToGda94 != NULL)
	{
		CSdeleteAgd84ToGda94 (csAgd84ToGda94);
		csAgd84ToGda94 = NULL;
		csAgd84ToGda94Cnt = 0;
	}
	return -1;
}

/******************************************************************************
	Close the AGD84<-->GDA94 conversion system.  Note, this only does a
	release, not a full delete.  This is for performance reasons.  During test,
	you may get	a "memory leak message" because of this.  If this is
	undeirable, then change the 'release' function to the 'delete' function.
*/
void EXP_LVL7 CSagd84Cls (void)
{
	csAgd84ToGda94Cnt -= 1;
	if (csAgd84ToGda94Cnt <= 0)
	{
		if (csAgd84ToGda94 != NULL)
		{
			CSreleaseAgd84ToGda94 (csAgd84ToGda94);
		}
    	csAgd84ToGda94Cnt = 0;
	}
	return;
}

/******************************************************************************
	Convert an AGD84 coordinate to GDA94 coordinate.
	Returns  0 for expected result
	        -1 for hard/fatal failure
			+1 for no data coverage, unshifted result returned
			+2 for no data coverage, fallback used successfully
*/
int EXP_LVL7 CSagd84ToGda94 (double ll_gda94 [3],Const double ll_agd84 [3])
{
	int status;
	double my_ll94 [3];

	/* We always do the null conversion. */
	my_ll94 [LNG] = ll_agd84 [LNG];
	my_ll94 [LAT] = ll_agd84 [LAT];
	my_ll94 [HGT] = ll_agd84 [HGT];

	/* Do the real conversion, if possible. */
	if (csAgd84ToGda94 == NULL)
	{
		CS_erpt (cs_DTC_NO_SETUP);
		status = -1;
	}
	else
	{
		status = CScalcAgd84ToGda94 (csAgd84ToGda94,my_ll94,ll_agd84);
	}
	ll_gda94 [LNG] = my_ll94 [LNG];
	ll_gda94 [LAT] = my_ll94 [LAT];
	ll_gda94 [HGT] = my_ll94 [HGT];
	return status;
}

/******************************************************************************
	Convert an GDA94 coordinate to Agd84 coordinate. Computationally, the
	inverse of the above.
	Returns  0 for expected result
	        -1 for hard/fatal failure
			+1 for no data coverage, unshifted result returned
			+2 for no data coverage, fallback used successfully
*/
int EXP_LVL7 CSgda94ToAgd84 (double ll_agd84 [3],Const double ll_gda94 [3])
{
	int status;

	double my_ll84 [3];

	/* We always do the null conversion. */
	my_ll84 [LNG] = ll_gda94 [LNG];
	my_ll84 [LAT] = ll_gda94 [LAT];
	my_ll84 [HGT] = ll_gda94 [HGT];

	/* Do the real conversion, if possible. */
	if (csAgd84ToGda94 == NULL)
	{
		CS_erpt (cs_DTC_NO_SETUP);
		status = -1;
	}
	else
	{
		status = CSinverseAgd84ToGda94 (csAgd84ToGda94,my_ll84,ll_gda94);
	}

	if (status >= 0)
	{
		ll_agd84 [LNG] = my_ll84 [LNG];
		ll_agd84 [LAT] = my_ll84 [LAT];
		ll_agd84 [HGT] = my_ll84 [HGT];
	}
	return status;
}
Const char * EXP_LVL7 CSagd84ToGda94Log (Const double ll_84 [2])
{
	Const char *cp;

	cp = NULL;
	/* Make sure we have been initialized. */
	if (csAgd84ToGda94 == NULL)
	{
		CS_erpt (cs_DTC_NO_SETUP);
	}
	else
	{
		cp = CSsourceAgd84ToGda94 (csAgd84ToGda94,ll_84);
	}
	return (cp == NULL || *cp == '\0') ? "<unavailable>" : cp;
}
/**********************************************************************
**	CS_agd84Name (new_name);
**
**	char *new_name;				the name of the Agd84ToGda94 catalog
**								file.
**
**	This function specifies the name only.  The directory,
**	and possibly drive, are specified using CS_altdr.
**********************************************************************/
void EXP_LVL1 CS_agd84Name (Const char *newName)
{
	extern char cs_Agd84Name [];

	CS_stncp (cs_Agd84Name,newName,cs_FNM_MAXLEN);
	return;
}
