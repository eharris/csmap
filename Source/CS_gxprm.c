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

#include "cs_map.h"

/* This function will extract the definition of the named Geodetic
   Transformation from the dictionary and then use that definition to
   create a fully initialized cs_GxXform_ structure which can actually
   be used to perform Geodetic Transformations in a generic way. */
struct cs_GxXform_ *CS_gxloc (Const char* gxDefName,short userDirection)
{
	extern char csErrnam [];
	extern struct cs_XfrmTab_ cs_XfrmTab [];

	int status;
	int errorCount;

	struct cs_GxXform_ *xfrmPtr;
	struct cs_Datum_ *srcDtPtr;
	struct cs_Datum_ *trgDtPtr;
	struct cs_GeodeticTransform_ *xfrmDefPtr;
	struct cs_XfrmTab_* xfrmTabPtr;

	int err_list [4];

	/* Prepare for any type of error. */

	xfrmPtr = NULL;
	srcDtPtr = NULL;
	trgDtPtr = NULL;
	xfrmDefPtr = NULL;				/* Redundant */

	/* Get the datum definition. */
	xfrmDefPtr = CS_gxdef (gxDefName);
	if (xfrmDefPtr == NULL)
	{
		goto error;
	}

	xfrmPtr = (struct cs_GxXform_*)CS_malc (sizeof (struct cs_GxXform_));
	if (xfrmPtr == NULL)
	{
		CS_erpt (cs_NO_MEM);
		goto error;
	}
	memset (xfrmPtr,0,sizeof (struct cs_GxXform_));
	xfrmPtr->userDirection = userDirection;

	/* Check the definition for validity.  The dictionary compiler
	   also performs this test, but we do it everytime here in case
	   a program somewhere adjusted the dictionary outside of the
	   "official" compiler. */
	errorCount = CS_gxchk (xfrmDefPtr,0,err_list,sizeof (err_list) / sizeof (int));
	if (errorCount)
	{
		CS_erpt (err_list [0]);
		goto error;
	}

	/* Transfer the method independent stuff from the definition to the
	   implementation structure. */
	memcpy (&xfrmPtr->gxDef,xfrmDefPtr,sizeof (struct cs_GeodeticTransform_));

	srcDtPtr = CS_dtloc (xfrmDefPtr->srcDatum);
	if (srcDtPtr == NULL)
	{
		goto error;
	}

	memcpy (&xfrmPtr->srcDatum,srcDtPtr,sizeof (struct cs_Datum_));
	CS_free (srcDtPtr);
	srcDtPtr = NULL;

	trgDtPtr = CS_dtloc (xfrmDefPtr->trgDatum);
	if (trgDtPtr == NULL)
	{
		goto error;
	}
	memcpy (&xfrmPtr->trgDatum,trgDtPtr,sizeof (struct cs_Datum_));
	CS_free (trgDtPtr);
	trgDtPtr = NULL;

	CS_stncp (xfrmPtr->xfrmName,xfrmDefPtr->xfrmName,sizeof (xfrmPtr->xfrmName));
	CS_stncp (xfrmPtr->group,xfrmDefPtr->group,sizeof (xfrmPtr->group));
	CS_stncp (xfrmPtr->description,xfrmDefPtr->description,sizeof (xfrmPtr->description));
	CS_stncp (xfrmPtr->source,xfrmDefPtr->source,sizeof (xfrmPtr->source));

	xfrmPtr->methodCode = xfrmDefPtr->methodCode;
	xfrmPtr->epsgNbr = xfrmDefPtr->epsgCode;
//	xfrmPtr->epsgVariation = xfrmDefPtr->????;
	xfrmPtr->inverseSupported = xfrmDefPtr->inverseSupported;
	xfrmPtr->maxIterations = xfrmDefPtr->maxIterations;
	xfrmPtr->protect = xfrmDefPtr->protect;
	xfrmPtr->cnvrgValue = xfrmDefPtr->cnvrgValue;
	xfrmPtr->errorValue = xfrmDefPtr->errorValue;
	xfrmPtr->accuracy = xfrmDefPtr->accuracy;

	/* Initialize this transformation. */
	for (xfrmTabPtr = cs_XfrmTab;xfrmTabPtr->methodCode != cs_DTCMTH_NONE;xfrmTabPtr++)
	{
		if (xfrmTabPtr->methodCode == xfrmPtr->methodCode)
		{
			break;
		}
	}
	if (xfrmTabPtr->methodCode == cs_DTCMTH_NONE)
	{
		CS_stncp (csErrnam,"<unknown>",MAXPATH);
		CS_erpt (cs_UNKWN_DTCMTH);
		goto error;
	}
	status = (*xfrmTabPtr->initialize)(xfrmPtr);
	if (status != 0)
	{
		goto error;
	}

	/* That should be it. */	
	CS_free (xfrmDefPtr);
	xfrmDefPtr = NULL;

	return xfrmPtr;

error:
	if (xfrmPtr != NULL)
	{
		CS_free (xfrmPtr);
		xfrmPtr = NULL;
	}
	if (srcDtPtr != NULL)
	{
		CS_free (srcDtPtr);
		srcDtPtr = NULL;
	}
	if (trgDtPtr != NULL)
	{
		CS_free (trgDtPtr);
		trgDtPtr = NULL;
	}
	if (xfrmDefPtr != NULL)
	{
		CS_free (xfrmDefPtr);
		xfrmDefPtr = NULL;
	}
	return NULL;
}
int CS_gxFrwrd3D (struct cs_GxXform_ *xform,double trgLl [3],Const double srcLl [3])
{
	int gxStatus;
	
	gxStatus = (*xform->frwrd3D)(&xform->xforms,trgLl,srcLl);
	return gxStatus;
}
int CS_gxFrwrd2D (struct cs_GxXform_ *xform,double trgLl [3],Const double srcLl [3])
{
	int gxStatus;
	
	gxStatus = (*xform->frwrd2D)(&xform->xforms,trgLl,srcLl);
	return gxStatus;
}
int CS_gxInvrs3D (struct cs_GxXform_ *xform,double trgLl [3],Const double srcLl [3])
{
	int gxStatus;
	
	gxStatus = (*xform->invrs3D)(&xform->xforms,trgLl,srcLl);
	return gxStatus;
}
int CS_gxInvrs2D (struct cs_GxXform_ *xform,double trgLl [3],Const double srcLl [3])
{
	int gxStatus;
	
	gxStatus = (*xform->invrs2D)(&xform->xforms,trgLl,srcLl);
	return gxStatus;
}
int CS_gxchk (Const struct cs_GeodeticTransform_ *gxXform,unsigned short gxChkFlg,int err_list [],int list_sz)
{
	extern struct cs_XfrmTab_ cs_XfrmTab[];
	extern char csErrnam [MAXPATH];

	int st;
	int ii;
	int err_cnt;

	struct cs_XfrmTab_* tblPtr;

	err_cnt = -1;

	/* Locate the geodetic transform method in the table, and thus
	   verify the validity of the method code. */
	for (tblPtr = cs_XfrmTab;tblPtr->methodCode != cs_DTCMTH_NONE;tblPtr += 1)
	{
		if (tblPtr->methodCode == gxXform->methodCode)
		{
			break;
		}
	}
	if (tblPtr->methodCode == cs_DTCMTH_NONE)
	{
		/* Invalid method code. */
		if (++err_cnt < list_sz) err_list [err_cnt] = cs_DTQ_MTHCODE;
	}
	else
	{
		/* Call the check function for the indicated method code. */
		err_cnt += (*tblPtr->check)(gxXform,tblPtr->methodCode,err_list,list_sz);
	}

	/* Check the stuff which applies to all definitions.
	   Verify that the name is a valid transformation name. */
	st = CS_nampp64 (gxXform->xfrmName);
	if (st != 0)
	{
		if (++err_cnt < list_sz) err_list [err_cnt] = cs_DTQ_XFRMNM;
	}

	/* Verify the two datum names are valid names but also names of
	   datums which actually exist. */
	st = CS_nampp (gxXform->srcDatum);
	if (st != 0)
	{
		if (++err_cnt < list_sz) err_list [err_cnt] = cs_DTQ_SRCDTNM;
	}
	else if ((gxChkFlg & cs_GXCHK_DATUM) != 0)
	{
		st = CS_dtIsValid (gxXform->srcDatum);
		if (st != 0)
		{
			if (++err_cnt < list_sz) err_list [err_cnt] = cs_DTQ_SRCDTNM;
		}
	}

	st = CS_nampp (gxXform->trgDatum);
	if (st != 0)
	{
		if (++err_cnt < list_sz) err_list [err_cnt] = cs_DTQ_TRGDTNM;
	}
	else if ((gxChkFlg & cs_GXCHK_DATUM) != 0)
	{
		st = CS_dtIsValid (gxXform->trgDatum);
		if (st != 0)
		{
			if (++err_cnt < list_sz) err_list [err_cnt] = cs_DTQ_TRGDTNM;
		}
	}

	/* Verify maxIterations, convergence value, error value, and accuracy. */
	if (gxXform->maxIterations < 0 || gxXform->maxIterations >= 40)
	{
		if (++err_cnt < list_sz) err_list [err_cnt] = cs_DTQ_MAXITR;
	}
	if (gxXform->cnvrgValue <= 1.0E-16 || gxXform->cnvrgValue >= 1.0E-02)
	{
		if (++err_cnt < list_sz) err_list [err_cnt] = cs_DTQ_CNVRGV;
	}
	if (gxXform->errorValue <= 1.0E-14 || gxXform->cnvrgValue >= 1.0 ||
	    gxXform->errorValue <= gxXform->cnvrgValue)
	{
		if (++err_cnt < list_sz) err_list [err_cnt] = cs_DTQ_ERRORV;
	}
	if (gxXform->accuracy < 0.0 || gxXform->accuracy >= 1000.0)
	{
		if (++err_cnt < list_sz) err_list [err_cnt] = cs_DTQ_ACCRCY;
	}

	/* If so instructed, report all of the errors detected so far. */
	if ((gxChkFlg & cs_GXCHK_REPORT) != 0)
	{
		CS_stncp (csErrnam,gxXform->xfrmName,MAXPATH);
		for (ii = 0;ii <= err_cnt && ii < list_sz;ii++)
		{
			CS_erpt (err_list [ii]);
		}
	}
	return (err_cnt + 1);
}
