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

	Provides comparison functions for checking the veracity of things like
	WKT export/import.
*/

#include "cs_map.h"

/* The following function compares only those portions of a Coordinate System
   definition which affect numeric results.  The number of differences found
   is returned.  Zero return indcates a match.  If the message argument is
   not NULL, and the messageSize argument is greater than one, a description
   of the first difference located is returned. */
int EXP_LVL3 CS_csDefCmp (Const struct cs_Csdef_ *original,Const struct cs_Csdef_ *revised,char* message,size_t messageSize)
{
	extern struct cs_Prjtab_ cs_Prjtab [];		/* Projection Table */
	extern double cs_Six;
	extern double cs_Zero;

	int errCnt = 0;
	int errCntCnc = 0;

	struct cs_Prjtab_ *pp;

	double unitsFactor;

	char errMsg [512];

	const struct cs_Csdef_ *lclOrgPtr;
	struct cs_Csdef_ lclOriginal;

	/* Before we get onto this to heavy; we check the projection code of the
	   original and the revised.  If the original is UTM, and the revised is
	   TM, we convert the original from UTM form to TM form.  This eliminates
	   a big headache. */
	lclOrgPtr = original;
	if (!CS_stricmp (original->prj_knm,"UTM") && !CS_stricmp (revised->prj_knm,"TM"))
	{
		/* Convert original to the TM form. */
		memcpy (&lclOriginal,original,sizeof (lclOriginal));
		strcpy (lclOriginal.prj_knm,"TM");
		lclOriginal.prj_prm1 = cs_Six * lclOriginal.prj_prm1 - 183.0;
		lclOriginal.org_lat = cs_Zero;
		unitsFactor = CS_unitlu (cs_UTYP_LEN,lclOriginal.unit);
		lclOriginal.x_off = 500000.0 / unitsFactor;
		lclOriginal.y_off = (lclOrgPtr->prj_prm2 >= 0.0) ? cs_Zero : 10000000.0 / unitsFactor;
		lclOriginal.scl_red = 0.9996;
		lclOriginal.quad = 1;
		lclOrgPtr = &lclOriginal;
	}

	if (CS_stricmp (lclOrgPtr->prj_knm,revised->prj_knm))
	{
		if (errCnt == 0)
		{
			sprintf (errMsg,"Projection key name was %s, is now %s",lclOrgPtr->prj_knm,revised->prj_knm);
		}
		errCnt += 1;
	}
	if (CS_stricmp (lclOrgPtr->unit,revised->unit))
	{
		if (errCnt == 0)
		{
			sprintf (errMsg,"Unit name was %s, is now %s",lclOrgPtr->unit,revised->unit);
		}
		errCnt += 1;
	}

	/* If the projection codes don't match, we're in deep do-do.  So, we bag it now
	   if we haven't got a match so far. */
	if (errCnt != 0)
	{
		if (message != 0 && messageSize > 1)
		{
			CS_stncp (message,errMsg,(int)messageSize);
		}
		return errCnt;
	}

	/* Look this projection up in the projection table, we need to pass the
	   projection code to the parameter check function. */
	for (pp = cs_Prjtab;pp->key_nm [0] != '\0';pp += 1)
	{
		if (!CS_stricmp (lclOrgPtr->prj_knm,pp->key_nm))
		{
			break;
		}
	}
	if (pp->check == NULL)
	{
		if (errCnt == 0)
		{
			sprintf (errMsg,"Projection key name is now %s which is invalid.",revised->prj_knm);
		}
		errCnt += 1;
	}
	else
	{
		if (message != NULL && messageSize > 1)
		{
			if (errCnt == 0) *message = '\0';
		}
		else
		{
			CS_stncp (message,errMsg,(int)messageSize);
		}

		/* Check all of the parameters. */
		if (errCnt == 0) errMsg[0] = '\0';

		/* We skip checking the first two parameters for the Unity projection as WKT
		   does not support the longitude range feature. */
		if (pp->code != cs_PRJCOD_UNITY)
		{
			/* For conic projections which require two standard parallels, the order of the parallels
			   does not make any difference to the mathemagics of the projection.  In this case,
			   if (prm1 != prm1 && prm2 != prm2) we do an additional check of
			   if (prm1 == prm2 and prm2 == prm1).  It is traditional to supply the northern
			   parallel first, but the are variations of WKT out there to do not follw this
			   tradition (and it is only a tradition). */
			if (pp->code == cs_PRJCOD_LM2SP   ||
				pp->code == cs_PRJCOD_LMBLG   ||
				pp->code == cs_PRJCOD_WCCSL   ||
				pp->code == cs_PRJCOD_MNDOTL  ||
				pp->code == cs_PRJCOD_ALBER   ||
				pp->code == cs_PRJCOD_LMBRTAF)
			{
				errCntCnc = 0;
				errCntCnc += CS_defCmpPrjPrm (pp, 1,lclOrgPtr->prj_prm1 ,revised->prj_prm1,errMsg,sizeof (errMsg));
				errCntCnc += CS_defCmpPrjPrm (pp, 2,lclOrgPtr->prj_prm2 ,revised->prj_prm2,errMsg,sizeof (errMsg));
				if (errCntCnc == 2)
				{
					errCnt += CS_defCmpPrjPrm (pp, 1,lclOrgPtr->prj_prm1 ,revised->prj_prm2,errMsg,sizeof (errMsg));
					errCnt += CS_defCmpPrjPrm (pp, 2,lclOrgPtr->prj_prm2 ,revised->prj_prm1,errMsg,sizeof (errMsg));
				}
			}
			else
			{
				/* Not a conic nor geographic, so we just compare the two
				   parameter values. */
				errCnt += CS_defCmpPrjPrm (pp, 1,lclOrgPtr->prj_prm1 ,revised->prj_prm1,errMsg,sizeof (errMsg));
				errCnt += CS_defCmpPrjPrm (pp, 2,lclOrgPtr->prj_prm2 ,revised->prj_prm2,errMsg,sizeof (errMsg));
			}
		}
		errCnt += CS_defCmpPrjPrm (pp, 3,lclOrgPtr->prj_prm3 ,revised->prj_prm3,errMsg,sizeof (errMsg));
		errCnt += CS_defCmpPrjPrm (pp, 4,lclOrgPtr->prj_prm4 ,revised->prj_prm4,errMsg,sizeof (errMsg));
		errCnt += CS_defCmpPrjPrm (pp, 5,lclOrgPtr->prj_prm5 ,revised->prj_prm5,errMsg,sizeof (errMsg));
		errCnt += CS_defCmpPrjPrm (pp, 6,lclOrgPtr->prj_prm6 ,revised->prj_prm6,errMsg,sizeof (errMsg));
		errCnt += CS_defCmpPrjPrm (pp, 7,lclOrgPtr->prj_prm7 ,revised->prj_prm7,errMsg,sizeof (errMsg));
		errCnt += CS_defCmpPrjPrm (pp, 8,lclOrgPtr->prj_prm8 ,revised->prj_prm8,errMsg,sizeof (errMsg));
		errCnt += CS_defCmpPrjPrm (pp, 9,lclOrgPtr->prj_prm9 ,revised->prj_prm9,errMsg,sizeof (errMsg));
		errCnt += CS_defCmpPrjPrm (pp,10,lclOrgPtr->prj_prm10,revised->prj_prm10,errMsg,sizeof (errMsg));
		errCnt += CS_defCmpPrjPrm (pp,11,lclOrgPtr->prj_prm11,revised->prj_prm11,errMsg,sizeof (errMsg));
		errCnt += CS_defCmpPrjPrm (pp,12,lclOrgPtr->prj_prm12,revised->prj_prm12,errMsg,sizeof (errMsg));
		errCnt += CS_defCmpPrjPrm (pp,13,lclOrgPtr->prj_prm13,revised->prj_prm13,errMsg,sizeof (errMsg));
		errCnt += CS_defCmpPrjPrm (pp,14,lclOrgPtr->prj_prm14,revised->prj_prm14,errMsg,sizeof (errMsg));
		errCnt += CS_defCmpPrjPrm (pp,15,lclOrgPtr->prj_prm15,revised->prj_prm15,errMsg,sizeof (errMsg));
		errCnt += CS_defCmpPrjPrm (pp,16,lclOrgPtr->prj_prm16,revised->prj_prm16,errMsg,sizeof (errMsg));
		errCnt += CS_defCmpPrjPrm (pp,17,lclOrgPtr->prj_prm17,revised->prj_prm17,errMsg,sizeof (errMsg));
		errCnt += CS_defCmpPrjPrm (pp,18,lclOrgPtr->prj_prm18,revised->prj_prm18,errMsg,sizeof (errMsg));
		errCnt += CS_defCmpPrjPrm (pp,19,lclOrgPtr->prj_prm19,revised->prj_prm19,errMsg,sizeof (errMsg));
		errCnt += CS_defCmpPrjPrm (pp,20,lclOrgPtr->prj_prm20,revised->prj_prm20,errMsg,sizeof (errMsg));
		errCnt += CS_defCmpPrjPrm (pp,21,lclOrgPtr->prj_prm21,revised->prj_prm21,errMsg,sizeof (errMsg));
		errCnt += CS_defCmpPrjPrm (pp,22,lclOrgPtr->prj_prm22,revised->prj_prm22,errMsg,sizeof (errMsg));
		errCnt += CS_defCmpPrjPrm (pp,23,lclOrgPtr->prj_prm23,revised->prj_prm23,errMsg,sizeof (errMsg));
		errCnt += CS_defCmpPrjPrm (pp,24,lclOrgPtr->prj_prm24,revised->prj_prm24,errMsg,sizeof (errMsg));
	}

	if ((pp->flags & cs_PRJFLG_ORGLAT) == 0)
	{
		if (CS_cmpDbls (lclOrgPtr->org_lat,revised->org_lat) == 0)
		{
			if (errCnt == 0)
			{
				sprintf (errMsg,"Origin Latitude was %14.8f, is now %14.8f",lclOrgPtr->org_lat,revised->org_lat);
			}
			errCnt += 1;
		}
	}
	if ((pp->flags & cs_PRJFLG_ORGLNG) == 0)
	{
		if (CS_cmpDbls (lclOrgPtr->org_lng,revised->org_lng) == 0)
		{
			if (errCnt == 0)
			{
				sprintf (errMsg,"Origin Longitude was %14.8f, is now %14.8f",lclOrgPtr->org_lng,revised->org_lng);
			}
			errCnt += 1;
		}
	}
	if ((pp->flags & cs_PRJFLG_ORGFLS) == 0)
	{
		if (fabs (lclOrgPtr->x_off - revised->x_off) > 0.001)
		{
			if (errCnt == 0)
			{
				sprintf (errMsg,"False easting was %14.3f, is now %14.3f",lclOrgPtr->x_off,revised->x_off);
			}
			errCnt += 1;
		}
		if (fabs (lclOrgPtr->y_off - revised->y_off) > 0.001)
		{
			if (errCnt == 0)
			{
				sprintf (errMsg,"False northing was %14.3f, is now %14.3f",lclOrgPtr->y_off,revised->y_off);
			}
			errCnt += 1;
		}
	}
	if ((pp->flags & cs_PRJFLG_SCLRED) != 0)
	{
		if (CS_cmpDbls (lclOrgPtr->scl_red,revised->scl_red) == 0)
		{
			if (errCnt == 0)
			{
				sprintf (errMsg,"Scale reduction was %12.10f, is now %12.10f",lclOrgPtr->scl_red,revised->scl_red);
			}
			errCnt += 1;
		}
	}
	if (lclOrgPtr->quad != revised->quad)
	{
		if (errCnt == 0)
		{
			sprintf (errMsg,"Quad was %d, is now %d",lclOrgPtr->quad,revised->quad);
		}
		errCnt += 1;
	}

	if (errCnt != 0)
	{
		if (message != 0 && messageSize > 1)
		{
			CS_stncp (message,errMsg,(int)messageSize);
		}
	}

	return errCnt;
}
int EXP_LVL3 CS_defCmpPrjPrm (struct cs_Prjtab_* pp,int prmNbr,double orgValue,double revValue,char *message,size_t messageSize)
{
	extern struct cs_PrjprmMap_ cs_PrjprmMap [];
	extern struct cs_Prjprm_ csPrjprm [];

	int errCnt;
	unsigned char parmType;
	struct cs_PrjprmMap_ *mapPtr;
	struct cs_Prjprm_ *prmPtr;
	double tolerance;

	char errMsg [512];
	errCnt = 0;

	/* Get the type of parameter. */
	for (mapPtr = cs_PrjprmMap;mapPtr->prj_code != cs_PRJCOD_END;mapPtr += 1)
	{
		if (mapPtr->prj_code == pp->code) break;
	}
	if (mapPtr->prj_code == cs_PRJCOD_END)
	{
		sprintf (errMsg,"Projection code did not map.");
		errCnt += 1;
	}
	else
	{
		/* Locate the parameter type. */
		parmType = mapPtr->prm_types [prmNbr - 1];

		/* See if this parameter is used. */
		if (parmType == 0)
		{
			/* It's not used, so we don't compare it. */
			return 0;
		}
		
		/* Get a pointer to the parameter table, extract the logical type of
		   the parameter, and then use the logical type to determine a
		   tolerance value for the follwing comparison. */
		prmPtr = &csPrjprm [parmType];
		switch (prmPtr->log_type) {
		case cs_PRMLTYP_LNG:
		case cs_PRMLTYP_LAT:
		case cs_PRMLTYP_AZM:
			tolerance = 1.0E-07;
			break;
		case cs_PRMLTYP_ANGD:
			tolerance = 1.0E-06;
			break;
		case cs_PRMLTYP_CMPLXC:
			tolerance = 1.0E-08;
			break;
		case cs_PRMLTYP_ZNBR:
			tolerance = 0.5;
			break;
		case cs_PRMLTYP_HSNS:
			tolerance = 0.01;
			break;
		case cs_PRMLTYP_GHGT:
		case cs_PRMLTYP_ELEV:
			tolerance = 1.0E-02;
			break;
		case cs_PRMLTYP_AFCOEF:
			tolerance = 1.0E-08;
			break;
		case cs_PRMLTYP_XYCRD:
			tolerance = 1.0E-02;
			break;
		case cs_PRMLTYP_SCALE:
			tolerance = 1.0E-06;
			break;
		case cs_PRMLTYP_NONE:
		default:
			tolerance = 1.0E-12;
			break;
		}
		
		/* Do a generic compare of the values and then report any problem. */
		if (fabs (orgValue - revValue) >= tolerance)
		{
			/* they didn't match. */
			sprintf (errMsg,"%s was %16.8f, in now %18.8f",prmPtr->label,orgValue,revValue);
			errCnt += 1;
		}
	}
	if (message != NULL && messageSize > 1 && *message == '\0' && errCnt > 0)
	{
		CS_stncp (message,errMsg,(int)messageSize);
	}
	return errCnt;
}
int EXP_LVL3 CS_dtDefCmp (Const struct cs_Dtdef_ *original,Const struct cs_Dtdef_ *revised,char* message,size_t messageSize)
{
	static const double deltaEpsilon = 6.0E-04;
	static const double rotEpsilon = 5.0E-04;
	static double scaleEpsilon = 5.0E-06;

	int orgIsNull;
	int revIsNull;
	int ellipsoidUse = 0;

	short paramUseCount = -1;
	short orgDeltaZeroCount;
	short revDeltaZeroCount;

	int errCnt = 0;

	char errMsg [512];

	errMsg [0] = '\0';
	if (message != NULL && messageSize > 0) *message = '\0';

	/* The revised will often get converted to a type cs_DTCTYP_GEOCTR with zero
	   deltas if it was one of those types that is generally considered to be
	   equivalent with WGS84. */
	orgDeltaZeroCount  = (original->delta_X == 0.0);
	orgDeltaZeroCount += (original->delta_Y == 0.0);
	orgDeltaZeroCount += (original->delta_Z == 0.0);
	orgIsNull = (original->to84_via == cs_DTCTYP_WGS84  ||
			  original->to84_via == cs_DTCTYP_NAD83  ||
			  original->to84_via == cs_DTCTYP_WGS84  ||
			  original->to84_via == cs_DTCTYP_GDA94  ||
			  original->to84_via == cs_DTCTYP_NZGD2K ||
			  original->to84_via == cs_DTCTYP_ETRF89 ||
			  original->to84_via == cs_DTCTYP_RGF93  ||
				 (orgDeltaZeroCount == 3 &&
					(original->to84_via == cs_DTCTYP_GEOCTR || original->to84_via == cs_DTCTYP_3PARM)
		   )
				);
					
	revDeltaZeroCount  = (revised->delta_X == 0.0);
	revDeltaZeroCount += (revised->delta_Y == 0.0);
	revDeltaZeroCount += (revised->delta_Z == 0.0);
	revIsNull = (revised->to84_via == cs_DTCTYP_WGS84  ||
			   revised->to84_via == cs_DTCTYP_NAD83  ||
			   revised->to84_via == cs_DTCTYP_WGS84  ||
			   revised->to84_via == cs_DTCTYP_GDA94  ||
			   revised->to84_via == cs_DTCTYP_NZGD2K ||
			   revised->to84_via == cs_DTCTYP_ETRF89 ||
				 revised->to84_via == cs_DTCTYP_RGF93  ||
				 (revDeltaZeroCount == 3 &&
					(revised->to84_via == cs_DTCTYP_GEOCTR || revised->to84_via == cs_DTCTYP_3PARM)
				 )
				);

	/* We can now compare the type giving proper consideration to the fact that
	   there are numerous ways of defining the null transformation.  That is, if
	   both original and revised are the null transformation, we  */
	if (orgIsNull && revIsNull)
		{
		/* Both are the null transformation, so they are essentially
		   equivalent. */
		return errCnt;
    }

	/* Compare the transformation technique. */
	if (original->to84_via != revised->to84_via)
    {
		if (errCnt == 0)
		{
			sprintf (errMsg,"Datum transformation method on datum named %s does not match.  Method was %d, is now %d",
							original->key_nm,
							original->to84_via,
							revised->to84_via);
		}
		
		/* We bump the error count by 8 to make this error appear to be more
		   than other types of errors. */
		errCnt += 8;
    }
    else
    {
		/* If the transformation technique is the same, it makes sense to compare
		   the parameters; if the transformation is of the type that uses
		   parameters, of course. */
		switch (original->to84_via) {
		case cs_DTCTYP_MOLO:    paramUseCount = 3; ellipsoidUse =  TRUE; break;
		case cs_DTCTYP_MREG:    paramUseCount = 7; ellipsoidUse =  TRUE; break;
		case cs_DTCTYP_BURS:    paramUseCount = 7; ellipsoidUse =  TRUE; break;
		case cs_DTCTYP_NAD27:   paramUseCount = 0; ellipsoidUse = FALSE; break;
		case cs_DTCTYP_NAD83:   paramUseCount = 0; ellipsoidUse = FALSE; break;
		case cs_DTCTYP_WGS84:   paramUseCount = 0; ellipsoidUse = FALSE; break;
		case cs_DTCTYP_WGS72:   paramUseCount = 0; ellipsoidUse = FALSE; break;
		case cs_DTCTYP_HPGN:    paramUseCount = 0; ellipsoidUse = FALSE; break;
		case cs_DTCTYP_7PARM:   paramUseCount = 7; ellipsoidUse =  TRUE; break;
		case cs_DTCTYP_AGD66:   paramUseCount = 0; ellipsoidUse = FALSE; break;
		case cs_DTCTYP_3PARM:   paramUseCount = 3; ellipsoidUse =  TRUE; break;
		case cs_DTCTYP_6PARM:   paramUseCount = 6; ellipsoidUse =  TRUE; break;
		case cs_DTCTYP_4PARM:   paramUseCount = 4; ellipsoidUse =  TRUE; break;
		case cs_DTCTYP_AGD84:   paramUseCount = 0; ellipsoidUse = FALSE; break;
		case cs_DTCTYP_NZGD49:  paramUseCount = 0; ellipsoidUse = FALSE; break;
		case cs_DTCTYP_ATS77:   paramUseCount = 0; ellipsoidUse = FALSE; break;
		case cs_DTCTYP_GDA94:   paramUseCount = 0; ellipsoidUse = FALSE; break;
		case cs_DTCTYP_NZGD2K:  paramUseCount = 0; ellipsoidUse = FALSE; break;
		case cs_DTCTYP_CSRS:    paramUseCount = 0; ellipsoidUse = FALSE; break;
		case cs_DTCTYP_TOKYO:   paramUseCount = 0; ellipsoidUse = FALSE; break;
		case cs_DTCTYP_RGF93:   paramUseCount = 0; ellipsoidUse = FALSE; break;
		case cs_DTCTYP_ED50:    paramUseCount = 0; ellipsoidUse = FALSE; break;
		case cs_DTCTYP_DHDN:    paramUseCount = 0; ellipsoidUse = FALSE; break;
		case cs_DTCTYP_ETRF89:  paramUseCount = 0; ellipsoidUse = FALSE; break;
		case cs_DTCTYP_GEOCTR:  paramUseCount = 3; ellipsoidUse =  TRUE; break;
		case cs_DTCTYP_NONE:
		default:
			paramUseCount = -1;
			ellipsoidUse = FALSE;
			break;
		}
	}

	/* Check the aparameters as is appropriate. */

	if (paramUseCount > 0)
	{
		if (fabs (original->delta_X - revised->delta_X) > deltaEpsilon)
		{
			if (errCnt == 0)
			{
				sprintf (errMsg,"%s: Delta X was %12.3f, is now %12.3f",original->key_nm,original->delta_X,revised->delta_X);
			}
			errCnt += 1;
		}
		if (fabs (original->delta_Y - revised->delta_Y) > deltaEpsilon)
		{
			if (errCnt == 0)
			{
				sprintf (errMsg,"%s: Delta Y was %12.3f, is now %12.3f",original->key_nm,original->delta_Y,revised->delta_Y);
			}
			errCnt += 1;
		}
		if (fabs (original->delta_Z - revised->delta_Z) > deltaEpsilon)
		{
			if (errCnt == 0)
			{
				sprintf (errMsg,"%s: Delta Z was %12.3f, is now %12.3f",original->key_nm,original->delta_Z,revised->delta_Z);
			}
			errCnt += 1;
		}
	}
	if (paramUseCount > 4)
		{
		if (fabs (original->rot_X - revised->rot_X)> rotEpsilon)
			{
				if (errCnt == 0)
				{
					sprintf (errMsg,"%s: X Rotation was %12.3f, is now %12.3f",original->key_nm,original->rot_X,revised->rot_X);
				}
				errCnt += 1;
			}
		if (fabs (original->rot_Y - revised->rot_Y) > rotEpsilon)
			{
				if (errCnt == 0)
				{
					sprintf (errMsg,"%s: Y Rotation was %12.3f, is now %12.3f",original->key_nm,original->rot_Y,revised->rot_Y);
				}
				errCnt += 1;
			}
		if (fabs (original->rot_Z - revised->rot_Z) > rotEpsilon)
			{
				if (errCnt == 0)
				{
					sprintf (errMsg,"%s: Z Rotation was %12.3f, is now %12.3f",original->key_nm,original->rot_Z,revised->rot_Z);
				}
				errCnt += 1;
			}
	}
	if (paramUseCount > 3 && paramUseCount != 6)
	{
		if (fabs (original->bwscale - revised->bwscale) > scaleEpsilon)
			{
				if (errCnt == 0)
				{
					sprintf (errMsg,"%s: Scale was %12.8f, is now %12.8f",original->key_nm,original->bwscale,revised->bwscale);
				}
				errCnt += 1;
			}
		}

	/* Check the ellipsoid if it matters for the trsansformation technique. */

	if (ellipsoidUse)
	{
		if (!CS_stricmp (original->ell_knm,revised->ell_knm))
		{
			if (errCnt == 0)
			{
				sprintf (errMsg,"%s: Ellipsoid was %s, is now %s",original->key_nm,original->ell_knm,revised->ell_knm);
			}
			errCnt += 1;
		}
	}

	/* OK, we're done.  Return results of comparison to the calling module. */
	if (errCnt != 0)
	{
		if (message != 0 && messageSize > 1)
		{
			CS_stncp (message,errMsg,(int)messageSize);
		}
	}
	return errCnt;
}
int EXP_LVL3 CS_elDefCmp (Const struct cs_Eldef_ *original,Const struct cs_Eldef_ *revised,char* message,size_t messageSize)
{
	int errCnt = 0;

	char errMsg [512];

	/* Pretty simple for an ellipsoid. */
	if (fabs (original->e_rad - revised->e_rad) > 6.0E-04)
	{
		if (errCnt == 0)
		{
			sprintf (errMsg,"%s: Equatorial radius was %14.4f, is now %14.4f",original->key_nm,original->e_rad,revised->e_rad);
		}
		errCnt += 1;
	}
	if (fabs (original->p_rad - revised->p_rad) > 6.0E-04)
	{
		if (errCnt == 0)
		{
			sprintf (errMsg,"%s: Polar radius was %14.4f, is now %14.4f",original->key_nm,original->p_rad,revised->p_rad);
		}
		errCnt += 1;
	}
	if (fabs (original->flat - revised->flat) > 5.0E-07)
	{
		if (errCnt == 0)
		{
			sprintf (errMsg,"%s: Flattening was %14.8f, is now %14.8f",original->key_nm,original->flat,revised->flat);
		}
		errCnt += 1;
	}
	if (fabs (original->ecent - revised->ecent) > 5.0E-08)
	{
		if (errCnt == 0)
		{
			sprintf (errMsg,"%s: Eccentricity was %11.9f, is now %11.9f",original->key_nm,original->ecent,revised->ecent);
		}
		errCnt += 1;
	}
	if (errCnt != 0)
	{
		if (message != 0 && messageSize > 1)
		{
			CS_stncp (message,errMsg,(int)messageSize);
		}
	}
	return errCnt;
}
int EXP_LVL3 CS_elDefCmpEx (Const struct cs_Eldef_ *original,Const struct cs_Eldef_ *revised,char* message,size_t messageSize,double* qPtr)
{
	static const double lat60 = (60.0 * ONE_DEGREE);
	static const double sinLat60 = 0.86602540378443864676372317075294;
	static const double cosLat60 = 0.5;

	int errCnt;

	double e_sq;
	double ddOrg;
	double ddRev;

	struct cs_MmcofF_ mmF;
	
	errCnt = CS_elDefCmp (original,revised,message,messageSize);

	/* Calculate the meridianl arc from the equator to 60 degrees of latitude
	   for each of the ellipsoids.  We return the difference as the quality
	   factor. */
	if (qPtr != 0)
	{
		e_sq = original->ecent * original->ecent;
		CSmmFsu (&mmF,original->e_rad,e_sq);
		ddOrg = CSmmFcal (&mmF,lat60,sinLat60,cosLat60);

		e_sq = revised->ecent * revised->ecent;
		CSmmFsu (&mmF,revised->e_rad,e_sq);
		ddRev = CSmmFcal (&mmF,lat60,sinLat60,cosLat60);

		*qPtr = fabs (ddOrg - ddRev);
	}	

	return errCnt;
}
int EXP_LVL3 CS_dtDefCmpEx (Const struct cs_Dtdef_ *original,Const struct cs_Dtdef_ *revised,
															 char* message,
															 size_t messageSize,
															 double* qPtr)
{
	static const double deltaEpsilon = 2.0E-03;
	static const double rotEpsilon = 1.0E-03;
	static double scaleEpsilon = 5.0E-06;

	int orgIsNull;
	int revIsNull;
	int ellipsoidUse = 0;

	short paramUseCount = -1;
	short orgDeltaZeroCount;
	short revDeltaZeroCount;
	short orgVia;
	short revVia;

	int errCnt = 0;

	double delta;
	double qFactor;

	char errMsg [512];

	qFactor = 0.0;			// until we know otherwise
	errMsg [0] = '\0';
	if (message != NULL && messageSize > 0) *message = '\0';

	/* The revised will often get converted to a type cs_DTCTYP_GEOCTR with zero
	   deltas if it was one of those types that is generally considered to be
	   equivalent with WGS84. */
	orgDeltaZeroCount  = (original->delta_X == 0.0);
	orgDeltaZeroCount += (original->delta_Y == 0.0);
	orgDeltaZeroCount += (original->delta_Z == 0.0);
	orgIsNull = (original->to84_via == cs_DTCTYP_WGS84  ||
			  original->to84_via == cs_DTCTYP_NAD83  ||
			  original->to84_via == cs_DTCTYP_WGS84  ||
			  original->to84_via == cs_DTCTYP_GDA94  ||
			  original->to84_via == cs_DTCTYP_NZGD2K ||
			  original->to84_via == cs_DTCTYP_ETRF89 ||
			  original->to84_via == cs_DTCTYP_RGF93  ||
				 (orgDeltaZeroCount == 3 &&
					(original->to84_via == cs_DTCTYP_GEOCTR ||
					 original->to84_via == cs_DTCTYP_3PARM  ||
					 original->to84_via == cs_DTCTYP_MOLO)
				 )
			);
					
	revDeltaZeroCount  = (revised->delta_X == 0.0);
	revDeltaZeroCount += (revised->delta_Y == 0.0);
	revDeltaZeroCount += (revised->delta_Z == 0.0);
	revIsNull = (revised->to84_via == cs_DTCTYP_WGS84  ||
			   revised->to84_via == cs_DTCTYP_NAD83  ||
			   revised->to84_via == cs_DTCTYP_WGS84  ||
			   revised->to84_via == cs_DTCTYP_GDA94  ||
			   revised->to84_via == cs_DTCTYP_NZGD2K ||
			   revised->to84_via == cs_DTCTYP_ETRF89 ||
				 revised->to84_via == cs_DTCTYP_RGF93  ||
				 (revDeltaZeroCount == 3 &&
					(revised->to84_via == cs_DTCTYP_GEOCTR ||
					 revised->to84_via == cs_DTCTYP_3PARM  ||
					 revised->to84_via == cs_DTCTYP_MOLO)
				 )
			);

	/* We can now compare the type giving proper consideration to the fact that
	   there are numerous ways of defining the null transformation.  That is, if
	   both original and revised are the null transformation, we  */
	if (orgIsNull && revIsNull)
		{
		/* Both are the null transformation, so they are essentially
		   equivalent. */
		ellipsoidUse = 0;
	}
	else
	{
		/* Compare the transformation technique. */
		orgVia = original->to84_via;
		revVia = revised->to84_via;
		if (orgVia == cs_DTCTYP_3PARM || orgVia == cs_DTCTYP_MOLO) orgVia = cs_DTCTYP_GEOCTR;
		if (revVia == cs_DTCTYP_3PARM || orgVia == cs_DTCTYP_MOLO) revVia = cs_DTCTYP_GEOCTR;
		if (orgVia != revVia)
		{
			if (errCnt == 0)
			{
				sprintf (errMsg,"Datum transformation method on datum named %s does not match.  Method was %d, is now %d",
								original->key_nm,
								original->to84_via,
								revised->to84_via);
			}

			/* We bump the error count by 8 to make this error appear to be more
			   than other types of errors. */
			errCnt += 8;
			qFactor = 100.0;
		}
		else
		{
			/* If the transformation technique is the same, it makes sense to compare
			   the parameters; if the transformation is of the type that uses
			   parameters, of course. */
			switch (original->to84_via) {
			case cs_DTCTYP_MOLO:    paramUseCount = 3; ellipsoidUse =  TRUE; break;
			case cs_DTCTYP_MREG:    paramUseCount = 7; ellipsoidUse =  TRUE; break;
			case cs_DTCTYP_BURS:    paramUseCount = 7; ellipsoidUse =  TRUE; break;
			case cs_DTCTYP_NAD27:   paramUseCount = 0; ellipsoidUse = FALSE; break;
			case cs_DTCTYP_NAD83:   paramUseCount = 0; ellipsoidUse = FALSE; break;
			case cs_DTCTYP_WGS84:   paramUseCount = 0; ellipsoidUse = FALSE; break;
			case cs_DTCTYP_WGS72:   paramUseCount = 0; ellipsoidUse = FALSE; break;
			case cs_DTCTYP_HPGN:    paramUseCount = 0; ellipsoidUse = FALSE; break;
			case cs_DTCTYP_7PARM:   paramUseCount = 7; ellipsoidUse =  TRUE; break;
			case cs_DTCTYP_AGD66:   paramUseCount = 0; ellipsoidUse = FALSE; break;
			case cs_DTCTYP_3PARM:   paramUseCount = 3; ellipsoidUse =  TRUE; break;
			case cs_DTCTYP_6PARM:   paramUseCount = 6; ellipsoidUse =  TRUE; break;
			case cs_DTCTYP_4PARM:   paramUseCount = 4; ellipsoidUse =  TRUE; break;
			case cs_DTCTYP_AGD84:   paramUseCount = 0; ellipsoidUse = FALSE; break;
			case cs_DTCTYP_NZGD49:  paramUseCount = 0; ellipsoidUse = FALSE; break;
			case cs_DTCTYP_ATS77:   paramUseCount = 0; ellipsoidUse = FALSE; break;
			case cs_DTCTYP_GDA94:   paramUseCount = 0; ellipsoidUse = FALSE; break;
			case cs_DTCTYP_NZGD2K:  paramUseCount = 0; ellipsoidUse = FALSE; break;
			case cs_DTCTYP_CSRS:    paramUseCount = 0; ellipsoidUse = FALSE; break;
			case cs_DTCTYP_TOKYO:   paramUseCount = 0; ellipsoidUse = FALSE; break;
			case cs_DTCTYP_RGF93:   paramUseCount = 0; ellipsoidUse = FALSE; break;
			case cs_DTCTYP_ED50:    paramUseCount = 0; ellipsoidUse = FALSE; break;
			case cs_DTCTYP_DHDN:    paramUseCount = 0; ellipsoidUse = FALSE; break;
			case cs_DTCTYP_ETRF89:  paramUseCount = 0; ellipsoidUse = FALSE; break;
			case cs_DTCTYP_GEOCTR:  paramUseCount = 3; ellipsoidUse =  TRUE; break;
			case cs_DTCTYP_NONE:
			default:
				paramUseCount = -1;
				ellipsoidUse = FALSE;
				break;
			}
		}

		/* Check the aparameters as is appropriate. */
		if (paramUseCount > 0)
		{
			delta = fabs (original->delta_X - revised->delta_X);
			if (delta > deltaEpsilon)
			{
				if (errCnt == 0)
				{
					sprintf (errMsg,"%s: Delta X was %12.3f, is now %12.3f",original->key_nm,original->delta_X,revised->delta_X);
				}
				errCnt += 1;
				qFactor += delta;
			}
			delta = fabs (original->delta_Y - revised->delta_Y);
			if (delta > deltaEpsilon)
			{
				if (errCnt == 0)
				{
					sprintf (errMsg,"%s: Delta Y was %12.3f, is now %12.3f",original->key_nm,original->delta_Y,revised->delta_Y);
				}
				errCnt += 1;
				qFactor += delta;
			}
			delta = fabs (original->delta_Z - revised->delta_Z);
			if (delta > deltaEpsilon)
			{
				if (errCnt == 0)
				{
					sprintf (errMsg,"%s: Delta Z was %12.3f, is now %12.3f",original->key_nm,original->delta_Z,revised->delta_Z);
				}
				errCnt += 1;
			}
		}
		if (paramUseCount > 4)
		{
			delta = fabs (original->rot_X - revised->rot_X);
			if (delta > rotEpsilon)
			{
				if (errCnt == 0)
				{
					sprintf (errMsg,"%s: X Rotation was %12.3f, is now %12.3f",original->key_nm,original->rot_X,revised->rot_X);
				}
				errCnt += 1;
				qFactor += delta * 3.0;
			}
			delta = fabs (original->rot_Y - revised->rot_Y);
			if (delta > rotEpsilon)
			{
				if (errCnt == 0)
				{
					sprintf (errMsg,"%s: Y Rotation was %12.3f, is now %12.3f",original->key_nm,original->rot_Y,revised->rot_Y);
				}
				errCnt += 1;
				qFactor += delta * 3.0;
			}
			delta = fabs (original->rot_Z - revised->rot_Z);
			if (delta > rotEpsilon)
			{
				if (errCnt == 0)
				{
					sprintf (errMsg,"%s: Z Rotation was %12.3f, is now %12.3f",original->key_nm,original->rot_Z,revised->rot_Z);
				}
				errCnt += 1;
				qFactor += delta * 3.0;
			}
		}
		if (paramUseCount > 3 && paramUseCount != 6)
		{
			delta = fabs (original->bwscale - revised->bwscale);
			if (delta > scaleEpsilon)
			{
				if (errCnt == 0)
				{
					sprintf (errMsg,"%s: Scale was %12.8f, is now %12.8f",original->key_nm,original->bwscale,revised->bwscale);
				}
				errCnt += 1;
				qFactor += delta*10.0;
			}
		}

		/* Check the ellipsoid if it matters for the transformation technique. */
		if (ellipsoidUse)
		{
			if (!CS_stricmp (original->ell_knm,revised->ell_knm))
			{
				if (errCnt == 0)
				{
					sprintf (errMsg,"%s: Ellipsoid was %s, is now %s",original->key_nm,original->ell_knm,revised->ell_knm);
				}
				errCnt += 1;
			}
		}
	}

	/* OK, we're done.  Return results of comparison to the calling module. */
	if (errCnt != 0)
	{
		if (message != 0 && messageSize > 1)
		{
			CS_stncp (message,errMsg,(int)messageSize);
		}
	}
	if (qPtr != 0)
	{
		*qPtr = qFactor;
	}
	return errCnt;
}
int EXP_LVL3 CS_csDefCmpEx (Const struct cs_Csdef_ *original,Const struct cs_Csdef_ *revised,char* message,size_t messageSize,double *qPtr)
{
	extern struct cs_Prjtab_ cs_Prjtab [];		/* Projection Table */
	extern double cs_Six;
	extern double cs_Zero;

	int errCnt = 0;
	int errCntCnc = 0;

	struct cs_Prjtab_ *ppOrg;
	struct cs_Prjtab_ *ppRev;
	struct cs_Csdef_* csDefPtr;

	double swpDbl;
	double unitsFactor;
	double qFactor;

	char errMsg [512];

	struct cs_Csdef_ lclOriginal;
	struct cs_Csdef_ lclRevised;

	qFactor = cs_Zero;

	/* Before we get onto this to heavy; we get copies of the two definitions
	   which we can modify.  In several cases, there are variations of
	   projections and parameters which are idenetical.  What shall do here
	   is modify our copies in those special cases as is appropriate so that
	   the actual comparison code is rational and straight forward.  */
	memcpy (&lclOriginal,original,sizeof (lclOriginal));
	memcpy (&lclRevised,revised,sizeof (lclRevised));

	/* Again, to preserve our sanity, we obtain the projection codes of the
	   the definitions.  We'll need at least one of them, so this is not all
	   wasted effort. */
	for (ppOrg = cs_Prjtab;ppOrg->key_nm [0] != '\0';ppOrg += 1)
	{
		if (!CS_stricmp (lclOriginal.prj_knm,ppOrg->key_nm))
		{
			break;
		}
	}
	for (ppRev = cs_Prjtab;ppRev->key_nm [0] != '\0';ppRev += 1)
	{
		if (!CS_stricmp (lclRevised.prj_knm,ppRev->key_nm))
		{
			break;
		}
	}
	if (ppOrg->code == cs_PRJCOD_END)
	{
		if (errCnt == 0)
		{
			sprintf (errMsg,"Projection key name was %s which is invalid.",revised->prj_knm);
		}
		qFactor = 1.0E+06;
		errCnt += 1;
	}
	if (ppRev->code == cs_PRJCOD_END)
	{
		if (errCnt == 0)
		{
			sprintf (errMsg,"Projection key name is now %s which is invalid.",revised->prj_knm);
		}
		qFactor = 1.0E+06;
		errCnt += 1;
	}
	if (errCnt != 0)
	{
		/* The calling mosule has not supplied use with useful definitions.
		   Bag it now. */
		goto error;
	}

	/* We now apply legitimate changes to the definitions such that identical
	   definitions will share the same projection code. */
	csDefPtr = 0;
	if (ppOrg->code == cs_PRJCOD_UTM && ppRev->code == cs_PRJCOD_TRMER)
	{
		csDefPtr = &lclOriginal;
	}
	else if (ppOrg->code == cs_PRJCOD_TRMER && ppRev->code == cs_PRJCOD_UTM)
	{
		csDefPtr = &lclRevised;
	}
	if (csDefPtr != 0)
	{
		/* csDefPtr points to a UTM definition which is now converted to
		   a standard Transverse Mercator form. */
		strcpy (csDefPtr->prj_knm,"TM");
		csDefPtr->prj_prm1 = (double)(((int)csDefPtr->prj_prm1 * 6) - 183);
		csDefPtr->org_lng = csDefPtr->prj_prm1;
		csDefPtr->org_lat = cs_Zero;
		unitsFactor = CS_unitlu (cs_UTYP_LEN,csDefPtr->unit);
		csDefPtr->x_off = 500000.0 / unitsFactor;
		csDefPtr->y_off = (csDefPtr->prj_prm2 >= 0.0) ? cs_Zero : 10000000.0 / unitsFactor;
		csDefPtr->scl_red = 0.9996;
		csDefPtr->quad = 1;
	}

	/* If either definition is a conic with two stanradr parallels, make the
	   northern most parallel orj_prm1 in either or both cases. */
	if (ppOrg->code == cs_PRJCOD_LM2SP   ||
		ppOrg->code == cs_PRJCOD_LMBLG   ||
		ppOrg->code == cs_PRJCOD_WCCSL   ||
		ppOrg->code == cs_PRJCOD_MNDOTL  ||
		ppOrg->code == cs_PRJCOD_ALBER   ||
		ppOrg->code == cs_PRJCOD_LMBRTAF)
	{
		if (lclOriginal.prj_prm1 < lclOriginal.prj_prm2)
		{
			swpDbl = lclOriginal.prj_prm2;
			lclOriginal.prj_prm2 = lclOriginal.prj_prm1;
			lclOriginal.prj_prm1 = swpDbl;
		}
	}
	if (ppRev->code == cs_PRJCOD_LM2SP   ||
		ppRev->code == cs_PRJCOD_LMBLG   ||
		ppRev->code == cs_PRJCOD_WCCSL   ||
		ppRev->code == cs_PRJCOD_MNDOTL  ||
		ppRev->code == cs_PRJCOD_ALBER   ||
		ppRev->code == cs_PRJCOD_LMBRTAF)
	{
		if (lclRevised.prj_prm1 < lclRevised.prj_prm2)
		{
			swpDbl = lclRevised.prj_prm2;
			lclRevised.prj_prm2 = lclRevised.prj_prm1;
			lclRevised.prj_prm1 = swpDbl;
		}
	}

	/* If one of the definitions is LMSP1 and the other is LMTAN, make them
	   the same. */
	if ((ppOrg->code == cs_PRJCOD_LMTAN && ppRev->code == cs_PRJCOD_LM1SP) ||
		(ppOrg->code == cs_PRJCOD_LM1SP && ppRev->code == cs_PRJCOD_LMTAN))
	{
		CS_stncp (lclRevised.prj_knm,lclOriginal.prj_knm,sizeof (lclRevised.prj_knm));
	}
	
	/* Check for standard Transverse Mercator with the appropriate quad value
	   and a South Oriented Transverse Mercator. */
	if ((ppOrg->code == cs_PRJCOD_TRMER && lclOriginal.quad == 3 && ppRev->code == cs_PRJCOD_SOTRM) ||
		(ppRev->code == cs_PRJCOD_TRMER && lclRevised.quad  == 3 && ppOrg->code == cs_PRJCOD_SOTRM))
	{
		CS_stncp (lclRevised.prj_knm,lclOriginal.prj_knm,sizeof (lclRevised.prj_knm));
		lclRevised.quad = lclOriginal.quad;
	}

	/* OK.  We've handled all of the variations which are identical; at least
	   the ones we know about as of now.  Now we do the comparison in a
	   rartional manner which can be understood. */
	if (CS_stricmp (lclOriginal.prj_knm,lclRevised.prj_knm))
	{
		if (errCnt == 0)
		{
			sprintf (errMsg,"Projection key name was %s, is now %s",lclOriginal.prj_knm,lclRevised.prj_knm);
		}
		errCnt += 1;
	}
	if (CS_stricmp (lclOriginal.unit,lclRevised.unit))
	{
		if (errCnt == 0)
		{
			sprintf (errMsg,"Unit name was %s, is now %s",lclOriginal.unit,lclRevised.unit);
		}
		errCnt += 1;
	}

	/* If the projection codes or units don't match, we're in deep do-do.  So,
	   we bag it now if we haven't got a match so far. */
	if (errCnt != 0)
	{
		goto error;
	}

	/* Check all of the parameters. */
	if (errCnt == 0) errMsg[0] = '\0';

	/* We skip checking the first two parameters for the Unity projection as WKT
	   and EPSG do not support the longitude range feature. */
	if (ppOrg->code != cs_PRJCOD_UNITY)
	{
		errCnt += CS_defCmpPrjPrm (ppOrg,1,lclOriginal.prj_prm1,lclRevised.prj_prm1,errMsg,sizeof (errMsg));
		errCnt += CS_defCmpPrjPrm (ppOrg,2,lclOriginal.prj_prm2,lclRevised.prj_prm2,errMsg,sizeof (errMsg));
	}
	errCnt += CS_defCmpPrjPrm (ppOrg, 3,lclOriginal.prj_prm3 ,lclRevised.prj_prm3,errMsg,sizeof (errMsg));
	errCnt += CS_defCmpPrjPrm (ppOrg, 4,lclOriginal.prj_prm4 ,lclRevised.prj_prm4,errMsg,sizeof (errMsg));
	errCnt += CS_defCmpPrjPrm (ppOrg, 5,lclOriginal.prj_prm5 ,lclRevised.prj_prm5,errMsg,sizeof (errMsg));
	errCnt += CS_defCmpPrjPrm (ppOrg, 6,lclOriginal.prj_prm6 ,lclRevised.prj_prm6,errMsg,sizeof (errMsg));
	errCnt += CS_defCmpPrjPrm (ppOrg, 7,lclOriginal.prj_prm7 ,lclRevised.prj_prm7,errMsg,sizeof (errMsg));
	errCnt += CS_defCmpPrjPrm (ppOrg, 8,lclOriginal.prj_prm8 ,lclRevised.prj_prm8,errMsg,sizeof (errMsg));
	errCnt += CS_defCmpPrjPrm (ppOrg, 9,lclOriginal.prj_prm9 ,lclRevised.prj_prm9,errMsg,sizeof (errMsg));
	errCnt += CS_defCmpPrjPrm (ppOrg,10,lclOriginal.prj_prm10,lclRevised.prj_prm10,errMsg,sizeof (errMsg));
	errCnt += CS_defCmpPrjPrm (ppOrg,11,lclOriginal.prj_prm11,lclRevised.prj_prm11,errMsg,sizeof (errMsg));
	errCnt += CS_defCmpPrjPrm (ppOrg,12,lclOriginal.prj_prm12,lclRevised.prj_prm12,errMsg,sizeof (errMsg));
	errCnt += CS_defCmpPrjPrm (ppOrg,13,lclOriginal.prj_prm13,lclRevised.prj_prm13,errMsg,sizeof (errMsg));
	errCnt += CS_defCmpPrjPrm (ppOrg,14,lclOriginal.prj_prm14,lclRevised.prj_prm14,errMsg,sizeof (errMsg));
	errCnt += CS_defCmpPrjPrm (ppOrg,15,lclOriginal.prj_prm15,lclRevised.prj_prm15,errMsg,sizeof (errMsg));
	errCnt += CS_defCmpPrjPrm (ppOrg,16,lclOriginal.prj_prm16,lclRevised.prj_prm16,errMsg,sizeof (errMsg));
	errCnt += CS_defCmpPrjPrm (ppOrg,17,lclOriginal.prj_prm17,lclRevised.prj_prm17,errMsg,sizeof (errMsg));
	errCnt += CS_defCmpPrjPrm (ppOrg,18,lclOriginal.prj_prm18,lclRevised.prj_prm18,errMsg,sizeof (errMsg));
	errCnt += CS_defCmpPrjPrm (ppOrg,19,lclOriginal.prj_prm19,lclRevised.prj_prm19,errMsg,sizeof (errMsg));
	errCnt += CS_defCmpPrjPrm (ppOrg,20,lclOriginal.prj_prm20,lclRevised.prj_prm20,errMsg,sizeof (errMsg));
	errCnt += CS_defCmpPrjPrm (ppOrg,21,lclOriginal.prj_prm21,lclRevised.prj_prm21,errMsg,sizeof (errMsg));
	errCnt += CS_defCmpPrjPrm (ppOrg,22,lclOriginal.prj_prm22,lclRevised.prj_prm22,errMsg,sizeof (errMsg));
	errCnt += CS_defCmpPrjPrm (ppOrg,23,lclOriginal.prj_prm23,lclRevised.prj_prm23,errMsg,sizeof (errMsg));
	errCnt += CS_defCmpPrjPrm (ppOrg,24,lclOriginal.prj_prm24,lclRevised.prj_prm24,errMsg,sizeof (errMsg));

	if ((ppOrg->flags & cs_PRJFLG_ORGLAT) == 0)
	{
		if (fabs (lclOriginal.org_lat - lclRevised.org_lat) > 1.0E-07)
		{
			if (errCnt == 0)
			{
				sprintf (errMsg,"Origin Latitude was %14.8f, is now %14.8f",lclOriginal.org_lat,lclRevised.org_lat);
			}
			errCnt += 1;
		}
	}
	if ((ppOrg->flags & cs_PRJFLG_ORGLNG) == 0)
	{
		if (fabs (lclOriginal.org_lng - lclRevised.org_lng) > 1.0E-07)
		{
			if (errCnt == 0)
			{
				sprintf (errMsg,"Origin Longitude was %14.8f, is now %14.8f",lclOriginal.org_lng,lclRevised.org_lng);
			}
			errCnt += 1;
		}
	}
	if ((ppOrg->flags & cs_PRJFLG_ORGFLS) == 0)
	{
		if (fabs (lclOriginal.x_off - lclRevised.x_off) > 0.01)
		{
			if (errCnt == 0)
			{
				sprintf (errMsg,"False easting was %14.3f, is now %14.3f",lclOriginal.x_off,lclRevised.x_off);
			}
			errCnt += 1;
		}
		if (fabs (lclOriginal.y_off - lclRevised.y_off) > 0.01)
		{
			if (errCnt == 0)
			{
				sprintf (errMsg,"False northing was %14.3f, is now %14.3f",lclOriginal.y_off,lclRevised.y_off);
			}
			errCnt += 1;
		}
	}
	if ((ppOrg->flags & cs_PRJFLG_SCLRED) != 0)
	{
		if (fabs (lclOriginal.scl_red - lclRevised.scl_red) > 1.0E-08)
		{
			if (errCnt == 0)
			{
				sprintf (errMsg,"Scale reduction was %12.10f, is now %12.10f",lclOriginal.scl_red,lclRevised.scl_red);
			}
			errCnt += 1;
		}
	}
	if (lclOriginal.quad != lclRevised.quad)
	{
		if (errCnt == 0)
		{
			sprintf (errMsg,"Quad was %d, is now %d",lclOriginal.quad,lclRevised.quad);
		}
		errCnt += 1;
	}

error:
	if (errCnt != 0)
	{
		if (message != 0 && messageSize > 1)
		{
			CS_stncp (message,errMsg,(int)messageSize);
		}
		if (qPtr != NULL)
		{
			*qPtr = qFactor;
		}
	}
	return errCnt;
}
