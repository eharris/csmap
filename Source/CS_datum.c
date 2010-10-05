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

/*lint -esym(715,dat_erf)     parameter not referenced */
/*lint -e514                  unusual use of a Boolean expression */
/*lint -e525	              negative indentation */
/*lint -e539                  positive indentation */
/*lint -e661                  out-of-bounds; these never really work in lint */
/*lint -e662                  out-of-bounds; these never really work in lint */
/*lint -e788                  enumeration value not used in a switch */

#include "cs_map.h"
#include "cs_Legacy.h"

/******************************************************************************
	The following code maps the steps included in a "to84_via" algorithm.
	That is, currently, all datums define how to get from the datum being
	defined to WGS84.  With each, datum, a technique is encoded in the
	definition.  This table defines the specific conversions necessary
	to accomplish this,

	Note, a conversion map for "To84" and "From84" for each technique is
	provided.  The set of defines listed below are the values
	one finds for the "to84_via" variable in the datum definition.
	The enumerator is the list of codes which are used in this
	module to select and activate the specific different
	conversions.
	
	While there is a one to one correspondence,	we have opted for
	this mapping in an interim release as future releases will
	have a completely different system of mapping the
	alghorithms.

	In general, names without 3D or 3d in them are horizontal, or
	two dimensional variations.  For the current interim release,
	the three dimensional versions are not implemented.

*******************************************************************************/

#define csDTCMAP_SIZE 4

static char modl_name [] = "CS_datum";


/* The following is, in hard coded form, the result that one would expect
   should one do a CS_dtloc ("CH1903Plus_1") function call.  This definition
   is hardcoded to insulate this code module from changes made in the
   dictionary.  Thus this module will return the same (and correct) results
   regardless of what the user might do to a datum dictionary.
*/
struct cs_Datum_ cs_Chrts95Dt =
{
	"ChPlusToChrts95",
	"BESSEL",
	6377397.155,
	6356078.962822,
	0.0033427731816160992517306360336969,
	0.081696831215255833813584066738272,
	674.374,
	15.056,
	405.346,
	0.0,
	0.0,
	0.0,
	0.0,
	cs_DTCTYP_GEOCTR,
	"CH1903+ to CHTRS95 (Swiss Terrestrial Reference System 1995)",
	"Bessel, 1841"
};

/**********************************************************************
**	dtc_ptr = CS_dtcsu (src_cs,dst_cs,dat_erf,blk_erf);
**
**	struct cs_Csprm_ *src_cs;	the source coordinate system definition for the
**								conversion to take place.
**	struct cs_Csprm_ *dst_cs;	the destination coordinate system definition.
**	int dat_erf;				flag word indicating how datum selection errors
**								are to be handled.
**	int blk_erf;				flag word indicating how block not found errors
**								are to be handled.
**	struct cs_Dtcprm_ *dtc_ptr;	returns a pointer to the datum conversion
**								parameters to be used to perform the desired
**								datum conversion.
**
**	This function returns the null pointer in the following
**	cases:
**
**	cs_NO_MEM					Insufficient memory to allocate the the
**								datum conversion definition block.
**	cs_DTC_DAT_F				An unsupported datum conversion was requested
**								and the dat_erf argument was set to as value
**								other than cs_DTCFLG_DAT_I or cs_DTCFLG_DAT_W.
**
**	To make a datum conversion, one first calls CS_dtcsu to
**	indicate the datums involved and how error conditions
**	are to be processed on the acutal conversion calls.
**	Once setup, CS_dtcvt is called to convert the actual
**	lat/long pairs.  CS_dtcvt requires a pointer as returned
**	by this function as its first argument.
**
**	DO NOT FREE THE RETURNED POINTER DIRECTLY.  To release
**	system resources allocated by any datum conversion, call
**	CS_dtcls.
**
**	The dat_erf argument may take either of the following
**	values:
**
**	cs_DTCFLG_DAT_I				Ignore any error condition.  If the
**								requested conversion is not supported
**								establish the NULL conversion.
**	cs_DTCFLG_DAT_W				Issue a warning message for unsupported
**								conversion requests, and establish the
**								NULL conversion.
**	cs_DTCFLG_DAT_W1			Issue a warning message once for
**								any case where the established
**								conversion is not the best
**								available, but only once during
**								the life of the process.
**
**	Any other value, or specifically zero, will cause a fatal
**	error message to be generated if the conversion established
**	is not the best available.
**
**	Beginning with release 6.0, if either coordinate system has
**	the null string for the datum key name in the cs_Datum_
**	structure buried within the cs_Csprm_ structure, the
**	null conversion is returned.  Thus, coordinate systems
**	can be based on an ellipsoid and conversion to LL possible
**	without having to assign a datum to a coordinate system,
**	or even more inconvenient, establish an LL coordinate
**	system with the same datum to prevent a datum conversion
**	of some sort.
**
**	This function always returns a conversion, even if it is
**	the do nothing conversion, the NULL conversion.  Users
**	should examine the value of cs_Error after calling this
**	function to see if something less than the ideal
**	conversion has been set up.  This condition can exist
**	only in those cases where the software has been
**	modularized into optional components.  If the entire
**	system is always delivered, these conditions can be
**	ignored.
**********************************************************************/

/**********************************************************************
	Original calling sequence. This now acts as a hook to the more generally
	useful function defined immediately below.
**********************************************************************/

struct cs_Dtcprm_ * EXP_LVL3 CS_dtcsu (	Const struct cs_Csprm_ *src_cs,
										Const struct cs_Csprm_ *dst_cs,
										int dat_erf,
										int blk_erf)

{
	short srcIsNerth = 0;
	short dstIsNerth = 0;
	srcIsNerth = (src_cs->prj_code == cs_PRJCOD_NERTH) || (src_cs->prj_code == cs_PRJCOD_NRTHSRT);
	dstIsNerth = (dst_cs->prj_code == cs_PRJCOD_NERTH) || (dst_cs->prj_code == cs_PRJCOD_NRTHSRT);
	if ((srcIsNerth != 0) ^ (dstIsNerth != 0))
	{
		CS_erpt (cs_NOT_NERTH);
		return NULL;
	}
	return CSdtcsu(&(src_cs->datum),&(dst_cs->datum),dat_erf,blk_erf);
}

struct cs_Dtcprm_ * EXP_LVL3 CSdtcsu (	Const struct cs_Datum_ *src_dt,
										Const struct cs_Datum_ *dst_dt,
										int dat_erf,
										int blk_erf)
{
	extern char csErrnam [MAXPATH];

	short direction;

	int idx;
	int bridgeStatus;

	char errMsg [256];

	Const char* srcDtmName;
	Const char* trgDtmName;
	struct csDtmBridge_ *bridgePtr;
	struct cs_Dtcprm_ *dtcPtr;
	struct cs_GxIndex_* gxIdxPtr;
	struct cs_GxXform_ *xfrmPtr;

	bridgePtr = NULL;
	dtcPtr = CS_malc (sizeof (struct cs_Dtcprm_));
	if (dtcPtr == NULL)
	{
		CS_erpt (cs_NO_MEM);
		goto error;
	}
	CS_stncp (dtcPtr->srcKeyName,src_dt->key_nm,sizeof (dtcPtr->srcKeyName));
	CS_stncp (dtcPtr->trgKeyName,dst_dt->key_nm,sizeof (dtcPtr->trgKeyName));
	dtcPtr->pathName [0] = '\0';
	dtcPtr->description [0] = '\0';
	dtcPtr->source [0] = '\0';
	dtcPtr->group [0] = '\0';
	dtcPtr->block_err = (short)blk_erf;
	dtcPtr->xfrmCount = 0;
	dtcPtr->listCount = 0;
	dtcPtr->rptCount = 0;
	for (idx = 0;idx < 10;idx +=1)
	{
		dtcPtr->errLngLat [idx][0] = dtcPtr->errLngLat [idx][1] = 0;
	}
	for (idx = 0;idx < csPATH_MAXXFRM;idx +=1)
	{
		dtcPtr->xforms [idx] = NULL;
	}

	/* If either coordinate system is cartographically referenced,
	   there is no datum conversion.  Having a cs_Dtcprm_ structure
	   with a zero transformation count is considered a null
	   transformation. */
	if (src_dt->key_nm [0] == '\0' || dst_dt->key_nm [0] == '\0')
	{
		//xfrmPtr = CS_gxloc (cs_NullXformName,cs_DTCDIR_FWD);
		//if (xfrmPtr == NULL)
		//{
		//	goto error;
		//}
		//dtcPtr->xforms [dtcPtr->xfrmCount++] = xfrmPtr;
		return (dtcPtr);
	}

	/* Build a bridge object which we will use to generate a list of
	   transformations which will get us from the source datum to the
	   target datum. */
	bridgePtr = CSnewDtmBridge (src_dt,dst_dt);

	/* If the source and target are the same, the bridge will be considered
	   complete. In this case, we're done as a cs_Dtcprm_ with a zero
	   xfrmCount is essentially a null transform.
	   
	   If the bridge is not complete, we have some serious work to do. */
	bridgeStatus = CSdtmBridgeIsComplete (bridgePtr);
	while (bridgeStatus == cs_DTCBRG_BUIILDING)
	{
		/* We need to add some transformations to the bridge.  There
		   are four different methods of doing this.  We continue using
		   these until we have a complete bridge.
		   
		   First, we use Phase One.  In Phase One we look to the Geodetic
		   Path dictionary for a path definition which converts from the
		   source datum to the target datum.  If such is located, we copy
		   the transformations into the bridge and we're done. */
		bridgeStatus = CSdtcsuPhaseOne (bridgePtr);
		if (bridgeStatus == cs_DTCBRG_COMPLETE)
		{
			//CS_stncp (dtcPtr->pathName,gpDef.pathName,sizeof (dtcPtr->pathName));
			//CS_stncp (dtcPtr->description,gpDef.description,sizeof (dtcPtr->description));
			//CS_stncp (dtcPtr->source,gpDef.source,sizeof (dtcPtr->source));
			//CS_stncp (dtcPtr->group,gpDef.group,sizeof (dtcPtr->group));
		}
		else if (bridgeStatus == cs_DTCBRG_BUIILDING)
		{
			/* No paths do the job.  We'll try phase two.  In this phase,
			   we look for a transformation which converts directly from
			   the source datum to the target datum.  If we find one, we
			   add that transformation to the bridge and we're all done. */
			bridgeStatus = CSdtcsuPhaseTwo (bridgePtr);
		}
		if (bridgeStatus == cs_DTCBRG_BUIILDING)
		{
			/* It appears Phase Two didn't do it either.  Time for Phase Three.
			   In phase three we use a pivot datum to see if we can find two
			   specific transformations which will get us from the source
			   datum to the target datum using a pivot datum.  The pivot datum
			   is typically WGS84.  However, our new phase three will do this
			   function using an ordered list of pivot datums. */
			bridgeStatus = CSdtcsuPhaseThree (bridgePtr);
		}
		if (bridgeStatus == cs_DTCBRG_BUIILDING)
		{
			/* OK, the pivot datum trick didn't work either.  We're left with
			   Phase Four.  Phase four will add a transformation to the source
			   side of the bridge if it finds that there is only one
			   transformation in the Geodetic Transformation dictionary which
			   converts to (or from) the named source datum.  Obviously, if
			   there is only one transformation in the Geodetic Transformation
			   dictionary which references the source datum, it MUST be a
			   transformation in the path we are building.
			   
			   Phase Four does the same thing for the target side of the bridge.
			   Again, if there is only one transformation which references the
			   target datum, that transformation MUST be a part of any successful
			   path.
			   
			   Having added one or more transformations to the bridge using Phase
			   Four, we simply repeat the initial three phases to see if we get
			   a completed path.
			   
			   Since Phase Four is a last ditch attempt at completing a path,
			   it shall (ilel has better) report a cs_DT_NPATH error and then
			   return cs_DTCBRG_ERROR if it doesn't modify the bridge in anyway.
			   This is what will break the 'while' loop in the event of an
			   impossible path. */
			bridgeStatus = CSdtcsuPhaseFour (bridgePtr);
		}
	}

	if (bridgeStatus != cs_DTCBRG_COMPLETE)
	{
		if (bridgeStatus == cs_DTCBRG_BUIILDING)
		{
			/* This condition will not have been reported by any of the
			   individual path building phases. */
			srcDtmName = CSdtmBridgeGetSourceDtm (bridgePtr);
			trgDtmName = CSdtmBridgeGetSourceDtm (bridgePtr);
			sprintf (errMsg,"'%s' to '%s'",srcDtmName,trgDtmName);
			CS_stncp (csErrnam,errMsg,MAXPATH);
			CS_erpt (cs_DT_NOPATH);
			goto error;
		}
		else
		{
			/* Otherwise, the nature of the problem will have already
			   been reported. */
			goto error;
		}
	}

	/* Here when we have a complete bridge, which is in essence, a unique
	   complete path from the source datum to the target datum.  We now
	   turn our attention to building a cs_Dtcprm_ structure which
	   accurately represents the path we have arrived at. */
	for (idx = 0;idx < csPATH_MAXXFRM;idx++)
	{
		gxIdxPtr = bridgePtr->bridgeXfrms [idx].xfrmPtr;
		if (gxIdxPtr != NULL)
		{
			direction  = bridgePtr->bridgeXfrms [idx].direction;
			xfrmPtr = CS_gxloc (gxIdxPtr->xfrmName,direction);
			if (xfrmPtr != NULL)
			{
				dtcPtr->xforms [dtcPtr->xfrmCount++] = xfrmPtr;
			}
			else
			{
				goto error;
			}
		}
	}
	if (bridgePtr != NULL)
	{
		CS_free (bridgePtr);
		bridgePtr = NULL;
	}
	return dtcPtr;

error:
	if (bridgePtr != NULL)
	{
		CS_free (bridgePtr);
		bridgePtr = NULL;
	}
	if (dtcPtr != NULL)
	{
		for (idx = 0;idx < csPATH_MAXXFRM;idx += 1)
		{
			xfrmPtr = dtcPtr->xforms [idx];
			if (xfrmPtr != NULL)
			{
				(*xfrmPtr->destroy)(&xfrmPtr->xforms);
				CS_free (xfrmPtr);
				dtcPtr->xforms [idx] = NULL;
			}
		}
		CS_free (dtcPtr);
	}
	return NULL;
}

struct cs_Dtcprm_* EXP_LVL3	CSdtcsu1 (Const char* gxName,short direction /* cs_DTCDIR_FWD or cs_DTCDIR_INV */,int blk_erf)
{
    struct cs_GeodeticTransform_ *xfrmDefPtr = NULL;
    struct cs_Dtcprm_ *dtcPtr = NULL;

    if (NULL == gxName)
    {
        CS_erpt (cs_ERSUP_SOFT);
        return NULL;
    }

    xfrmDefPtr = CS_gxdef (gxName);
	if (xfrmDefPtr == NULL)
	{
        CS_erpt (cs_GX_NOT_FND);
        return NULL;
	}

    dtcPtr = CSdtcsu2(xfrmDefPtr, direction, blk_erf);

    if (NULL != xfrmDefPtr)
        CS_free(xfrmDefPtr); //[dtcPtr] has its own copy

    return dtcPtr; //can be null
}

struct cs_Dtcprm_* EXP_LVL3	CSdtcsu2 (Const struct cs_GeodeticTransform_ *xfrmDefPtr, short direction, int blk_erf)
{
    struct cs_Dtcprm_ *dtcPtr;
    struct cs_GxXform_ *transform;

    dtcPtr = CS_malc (sizeof (struct cs_Dtcprm_));
	if (dtcPtr == NULL)
	{
		CS_erpt (cs_NO_MEM);
		return NULL;
	}

    transform = CS_gxloc1 (xfrmDefPtr, direction);
    if (NULL == transform)
        goto error;

    memset(dtcPtr, 0, sizeof(struct cs_Dtcprm_));

    //we now basically have everything - fill [dtcPtr] and return it to the caller
    CS_stncp (dtcPtr->srcKeyName, transform->srcDatum.dt_name, sizeof (dtcPtr->srcKeyName));
	CS_stncp (dtcPtr->trgKeyName, transform->trgDatum.dt_name, sizeof (dtcPtr->trgKeyName));
	
    /* everything set to 0 already

    dtcPtr->pathName [0] = '\0';
	dtcPtr->description [0] = '\0';
	dtcPtr->source [0] = '\0';
	dtcPtr->group [0] = '\0';
	
	dtcPtr->xfrmCount = 0;
	dtcPtr->listCount = 0;
	dtcPtr->rptCount = 0;
    */

    dtcPtr->block_err = (short)blk_erf;
    dtcPtr->xfrmCount = 1;
    dtcPtr->xforms[0] = transform;

    return dtcPtr;

error:
    if (NULL != dtcPtr)
        CS_free (dtcPtr);

    if (NULL != transform)
        CS_free(transform);

    return NULL;
}

/* Phase One -- Locate a path in the Geodetic Path directory.

	It is possible, quite likely even, that there may be more than one such
	path in the dictionary.  The following the rules established for this
	function and its return value:

	1> The path returned is the first path encountered which meets the
	   source and target requirements unless a forward definition is
	   encountered after one or more reverse definitions are found.
	2> Reverse definitions are only considered if the definition is
	   marked as being reversible.
	3> The returned count will always include all forward definitions which
	   meet the src/trg criteria, and all inverse definitions which meet the
	   src/trg criteria which are marked as reversible.
	   
	The objective of these rules are to enable a path definition to go either
	way, or to have two sepoarate paths for each direction.  This can be
	controlled by the reversible flag.
*/

int CSdtcsuPhaseOne (struct csDtmBridge_* bridgePtr)
{
	extern char csErrnam [MAXPATH];

	short idx;
	short idxCount;
	short direction;

	int gxIndex;
	int bridgeStatus;

	Const char* srcDtmName;
	Const char* trgDtmName;
	struct cs_GxIndex_ *xfrmPtr;
	struct cs_GeodeticPath_* pathPtr;
	struct cs_GeodeticPathElement_ *pathElePtr;

	pathPtr = NULL;

	/* Make a linear search through the Geodetic Path dictionary looking
	   for an entry where the source and target match the provided names.
	   If the reversiable flag is set, we'll accept a definition where the
	   target and source match our source and target in that order.
	   
	   We always search the entire dictionary and count all the matching\
	   entries.  CS_gpdefEx does all of that for us.  How convenient:>) */
	srcDtmName = CSdtmBridgeGetSourceDtm (bridgePtr);
	trgDtmName = CSdtmBridgeGetTargetDtm (bridgePtr);
	pathPtr = CS_gpdefEx (&direction,srcDtmName,trgDtmName);
	if (pathPtr != NULL && direction != cs_DTCDIR_NONE)
	{
		/* OK, we found a unique path in the path dictionary for getting from
		   the source datum to the target datum.  Copy the transformations
		   indicated by the unique path into the bridge.  We then use
		   CSdtmBridgeIsComplete to determine the status of the bridge, which
		   should be (in this particular case) cs_DTCBRG_COMPLETE. */
		idxCount = pathPtr->elementCount;
		if (idxCount <= 0 || idxCount >= csPATH_MAXXFRM)
		{
			CS_stncp (csErrnam,"CS_datum::1",MAXPATH);
			CS_erpt (cs_ISER);
			goto error;
		}

		if (direction == cs_DTCDIR_FWD)
		{
			for (idx = 0;idx < idxCount;idx += 1)
			{
				/* Here once for each transformation in the located 
				   Geodetic Path definition. */
				pathElePtr = &pathPtr->geodeticPathElements [idx];
				direction = pathElePtr->direction;
				gxIndex = CS_locateGxByName (pathElePtr->geodeticXformName);
				if (gxIndex < 0)
				{
					CS_stncp (csErrnam,"CS_datum::2",MAXPATH);
					CS_erpt (cs_ISER);
					goto error;
				}
				xfrmPtr = CS_getGxIndexEntry (gxIndex); 
				if (xfrmPtr == NULL)
				{
					CS_stncp (csErrnam,"CS_datum::3",MAXPATH);
					CS_erpt (cs_ISER);
					goto error;
				}
				CSdtmBridgeAddSrcTransformation (bridgePtr,xfrmPtr,direction);
			}
		}
		else if (direction == cs_DTCDIR_INV)
		{
			for (idx = 0;idx <idxCount;idx += 1)
			{
				/* Here once for each transformation in the located 
				   Geodetic Path definition. */
				pathElePtr = &pathPtr->geodeticPathElements [idx];
				direction = pathElePtr->direction;
				direction = (direction == cs_DTCDIR_FWD) ? cs_DTCDIR_INV : cs_DTCDIR_FWD;
				gxIndex = CS_locateGxByName (pathElePtr->geodeticXformName);
				if (gxIndex < 0)
				{
					CS_stncp (csErrnam,"CS_datum::2",MAXPATH);
					CS_erpt (cs_ISER);
					goto error;
				}
				xfrmPtr = CS_getGxIndexEntry (gxIndex); 
				if (xfrmPtr == NULL)
				{
					CS_stncp (csErrnam,"CS_datum::3",MAXPATH);
					CS_erpt (cs_ISER);
					goto error;
				}
				CSdtmBridgeAddTrgTransformation (bridgePtr,xfrmPtr,direction);
			}
		}
		else
		{
			CS_stncp (csErrnam,"CS_datum::2",MAXPATH);
			CS_erpt (cs_ISER);
			goto error;
		}
		CS_free (pathPtr);
		pathPtr = NULL;
	}	
	bridgeStatus = CSdtmBridgeIsComplete (bridgePtr);
	return (bridgeStatus);

error:
	if (pathPtr != NULL)
	{
		CS_free (pathPtr);
		pathPtr = NULL;
	}
	return cs_DTCBRG_ERROR;
}

/* Phase Two

	Here search the Geodetic Transformation for a transformation which transforms
	directly from the source to the target.  Again, we consider a transformation
	which proceeds in the reverse direction if the transformation is marked as
	reversible.
	
	Note that we return a Path and not a transformation.  Thus, if we find
	a transformation which meets our requirements, we will construct a path
	with a single entry in it which contains the name of the transformation which
	we have located.
*/
int CSdtcsuPhaseTwo (struct csDtmBridge_* bridgePtr)
{
	extern char csErrnam [MAXPATH];

	int result;
	int direction;
	int bridgeStatus;

	Const char* srcDtmName;
	Const char* trgDtmName;
	Const struct cs_GxIndex_* gxIdxPtr; 

	/* We haven't found anything yet. */
	bridgeStatus = cs_DTCBRG_BUIILDING;
	srcDtmName = CSdtmBridgeGetSourceDtm (bridgePtr);
	trgDtmName = CSdtmBridgeGetTargetDtm (bridgePtr);

	result = CS_locateGxByDatum2 (&direction,srcDtmName,trgDtmName);
	if (result >= 0)
	{
		/* Here if there is a unique transformation first from the source
		   datum to the target datum; i.e. a single transformation which
		   does the job. */
		gxIdxPtr = CS_getGxIndexEntry ((unsigned)result);
		bridgeStatus = CSdtmBridgeAddSrcTransformation (bridgePtr,gxIdxPtr,(short)direction);
	}
	else
	{
		/* We didn't find anything.  If we didn't find a suitable
		   transformation, then we just continue building.  Otherwise,
		   we consider it an error. */
		bridgeStatus = (result == cs_GXIDX_NOXFRM) ? cs_DTCBRG_BUIILDING : cs_DTCBRG_ERROR; 
	}
	return bridgeStatus;
}
/* Phase Three -- Generate Path using pivot datums.

	In this phase we search for two transformations using a pivot datum.
	We support the concept of a prioritized list of pivot datums; although
	initialliy that list is a single pivot datum: "WGS84".
*/
int CSdtcsuPhaseThree (struct csDtmBridge_* bridgePtr)
{
	extern char csErrnam [MAXPATH];
	extern struct cs_PivotDatumTbl_ cs_PivotDatumTbl [];

	int fromDirection;
	int toDirection;

	int result;
	int bridgeStatus;

	Const char* srcDtmName;
	Const char* pvtDtmName;
	Const char* trgDtmName;
	Const struct cs_GxIndex_* fromIdxPtr; 
	Const struct cs_GxIndex_* toIdxPtr; 

	struct cs_Datum_ *pivotDatum;
	struct cs_PivotDatumTbl_* pivotTblPtr;

	/* Get the datum names of the gap we are currently seeking a bridge
	   segment for. */
	srcDtmName = CSdtmBridgeGetSourceDtm (bridgePtr);
	trgDtmName = CSdtmBridgeGetTargetDtm (bridgePtr);

	/* Loop once for each pivot datum in the list.  We break the loop
	   if we find a suitable datum path. */
	bridgeStatus = cs_DTCBRG_ERROR;
	pivotDatum = NULL;
	for (pivotTblPtr = cs_PivotDatumTbl;pivotTblPtr->datumName [0] != '\0';pivotTblPtr++)
	{
		fromIdxPtr = toIdxPtr = NULL;
		fromDirection = toDirection = cs_DTCDIR_NONE;

		pivotDatum = CS_dtloc (pivotTblPtr->datumName);
		if (pivotDatum == NULL)
		{
			CS_stncp (csErrnam,"CS_datum::6",MAXPATH);
			CS_erpt (cs_ISER);
			goto error;
		}
		pvtDtmName = pivotDatum->key_nm;

		result = CS_locateGxByDatum2 (&fromDirection,srcDtmName,pvtDtmName);
		if (result >= 0)
		{
			/* Here if there is a unique transformation first from the source
			   datum to the target datum; i.e. a single transformation which
			   does the job. */
			fromIdxPtr = CS_getGxIndexEntry ((unsigned)result);
		}
		else
		{
			/* We didn't find anything.  If we didn't find a suitable
			   transformation, then we just continue building.  Otherwise,
			   we consider it an error. */
			bridgeStatus = (result == cs_GXIDX_NOXFRM) ? cs_DTCBRG_BUIILDING : cs_DTCBRG_ERROR; 
		}

		if (fromIdxPtr != NULL)
		{
			result = CS_locateGxByDatum2 (&toDirection,pvtDtmName,trgDtmName);
			if (result >= 0)
			{
				/* Here if there is a unique transformation first from the source
				   datum to the target datum; i.e. a single transformation which
				   does the job. */
				toIdxPtr = CS_getGxIndexEntry ((unsigned)result);
			}
			else
			{
				/* We didn't find anything.  If we didn't find a suitable
				   transformation, then we just continue building.  Otherwise,
				   we consider it an error. */
				bridgeStatus = (result == cs_GXIDX_NOXFRM) ? cs_DTCBRG_BUIILDING : cs_DTCBRG_ERROR; 
			}
		}
		
		if (fromIdxPtr == NULL || toIdxPtr == NULL)
		{
			continue;
		}
		
		bridgeStatus = CSdtmBridgeAddSrcTransformation (bridgePtr,fromIdxPtr,(short)fromDirection);
		if (bridgeStatus == cs_DTCBRG_BUIILDING)
		{
			bridgeStatus = CSdtmBridgeAddTrgTransformation (bridgePtr,toIdxPtr,(short)toDirection);
		}
	}
	if (pivotDatum != NULL)
	{
		CS_free (pivotDatum);
		pivotDatum = NULL;
	}
	return bridgeStatus;
error:
	if (pivotDatum != NULL)
	{
		CS_free (pivotDatum);
		pivotDatum = NULL;
	}
	return cs_DTCBRG_ERROR;
}

/* In phase four, we look for singula5r datum references and add them to the
   bridge as is appropriate.  For example, we serach the geodetic
   transformation index for a transformation which comverts from (or to)
   the source datum.  If this reference is singular within the entire
   geodetic transformation dictionary, we know this transformation MUST
   be involved in any successful path from source to target datums, so
   we add this transformation to the source end of the bridge.
   
   Similarly with the target datum.  Maybe we are then complete, maybe
   not.  If not, we go back and try phases one, two, and three again. */
int CSdtcsuPhaseFour (struct csDtmBridge_* bridgePtr)
{
	extern char csErrnam [MAXPATH];

	int gxIndex;
	int direction;
	int bridgeStatus;

	Const char* srcDtmName;
	Const char* trgDtmName;

	Const struct cs_GxIndex_* gxIdxPtr; 

	/* Get the datum names of the gap we are currently seeking a bridge
	   segment for. */
	srcDtmName = CSdtmBridgeGetSourceDtm (bridgePtr);
	trgDtmName = CSdtmBridgeGetTargetDtm (bridgePtr);

	/* Prepare to the failure case where we find no additions to
	   the bridge being built. */
	bridgeStatus = cs_DTCBRG_ERROR;

	/* See if there is a singular reference to the source datum in the
	   Geodetic Transformation dictionary. */
	gxIndex = CS_locateGxFromDatum (&direction,srcDtmName);
	if (gxIndex >= 0)
	{
		/* Yup, there is.  Add it to the source end of the bridge. */
		gxIdxPtr = CS_getGxIndexEntry (gxIndex);
		bridgeStatus = CSdtmBridgeAddSrcTransformation (bridgePtr,gxIdxPtr,(short)direction);
	}

	/* See if there is a singular reference to the target datum in the
	   Geodetic Transformation dictionary. */
	gxIndex = CS_locateGxToDatum (&direction,trgDtmName);
	if (gxIndex >= 0)
	{
		/* Yup, there is.  Add it to the target end of the bridge. */
		gxIdxPtr = CS_getGxIndexEntry (gxIndex);
		bridgeStatus = CSdtmBridgeAddTrgTransformation (bridgePtr,gxIdxPtr,(short)direction);
	}
	return bridgeStatus;
}

/**********************************************************************
**	CS_dtcls (dtc_ptr);
**
**	struct cs_Dtcprm_ *dtc_ptr;	pointer to the datum conversion parameter
**								block controlling the datum conversion which
**								is to be disabled.
**********************************************************************/
void EXP_LVL3 CS_dtcls (struct cs_Dtcprm_ *dtcPrm)
{
	extern char csErrnam [];
	
	short idx;

	struct cs_GxXform_ *xfrmPtr;
	
	/* Don't want to crash. */
	if (dtcPrm == NULL) return;

	/* Close whatever datum conversions are indicated
	   in the conversion list. */
	for (idx = 0;idx < csPATH_MAXXFRM;idx++)
	{
		/* Extract the code value. */
		xfrmPtr = dtcPrm->xforms [idx];
		if (xfrmPtr != NULL)
		{
			/* Delete this transformation. */
			(*xfrmPtr->destroy)(&xfrmPtr->xforms);

			/* Free up the transformation itself. */
			CS_free (xfrmPtr);
			dtcPrm->xforms [idx] = NULL;
		}
	}

	/* Free up the overall datum transformation structure. */
	CS_free (dtcPrm);

	return;
}

/**********************************************************************
**	stat = CS_dtcvt (dtc_ptr,ll_in,ll_out);
**
**	struct cs_Dtcprm_ *dtc_ptr;	pointer to the datum conversion parameters;
**			as returned by CS_dtcsu.
**	double ll_in;				the lat/long pair to be converted.
**	double ll_out;				the converted lat/long result is returned here.
**	int status;					returns zero if CS_dtcvt was able to produce a
**								perfectly normal result. +1 is returned if a
**								data availability probelm precluded a normal
**								calculation.  A -1 is returned if some other
**								type of error (already reported to CS_erpt)
**								caused the calculation to fail.
**
**	The ll_in and ll_out arguments may point to the same array.
**
**	A zero return value indicates that the datum conversion
**	was completed as normally expected.  A -1 value indicates
**	that a hard system type error, which has been reported to
**	CS_erpt, was encounterd.  The results in this case are
**	simply a copy of the input lat/long.
**
**	A value of +1 is returned when a problem whose cause is
**	likely to be that the lat/long to be shifted is outside
**	the normal range of the datum shifts is presented for
**	shifting.  For example, attempting to shift a lat/long
**	in Japan to or from NAD27 will produce this result.  In
**	these cases, this function will attempt to make the
**	datum shift using alternative techniques.
**
**	NOTE: most status values are generated in the sub-functions
**	of this module.
**
**	The name here is really a misnomer for historical reasons.
**	Originally, this function converted only between the
**	North American Datums of 1927 and 1983.  Therefore, it
**	was given the name of Datum convert.  This module has
**	since been upgraded to convert between most all geodetic
**	reference systems.
**
**	To make a datum conversion, one first calls CS_dtcsu to
**	obtain a pointer to the datum conversion parameters.
**	The datum conversion parameter structure contains
**	pointers to the functions involved and maintains the
**	error processing specification made at setup time.
**********************************************************************/

/* The following function is the traditional two dimensional version
   of the datum conversion function.  The three dimensional version
   appears immediately following. */
int EXP_LVL3 CS_dtcvt (struct cs_Dtcprm_ *dtcPrm,Const double ll_in [3],double ll_out [3])
{
	return CSdtcvt (dtcPrm,FALSE,ll_in,ll_out);
}
int EXP_LVL3 CS_dtcvt3D (struct cs_Dtcprm_ *dtcPrm,Const double ll_in [3],double ll_out [3])
{
	return CSdtcvt (dtcPrm,TRUE,ll_in,ll_out);
}
int EXP_LVL3 CSdtcvt (struct cs_Dtcprm_ *dtcPrm,short flag3D,Const double ll_in [3],double ll_out [3])
{
	extern char csErrnam [MAXPATH];
	extern int csErrlng;
	extern int csErrlat;
	extern double cs_Zero;

	short idx;
	short xfrmCount;
	short methodCode;

	int gxStatus;				/* status of current tranformation */
	int status;					/* accumulated status */
	int wasInListFlag;
	int rptCode;

	struct cs_GxXform_ *xfrmPtr;

	double ll_wrk [3];

	/* Regardless of what happens in all of this, at a minimum we
	   guarantee that ll_out will be set to ll_in. */
	ll_out [LNG] = ll_in [LNG];
	ll_out [LAT] = ll_in [LAT];
	ll_out [HGT] = ll_in [HGT];
	rptCode = 0;

	status = 0;			/* Until we know differently. */
	if (dtcPrm != NULL && dtcPrm->xfrmCount > 0 &&
						  dtcPrm->xfrmCount < csPATH_MAXXFRM)
	{
		/* There are one or more conversions to be performed. */
		ll_wrk [LNG] = ll_in [LNG];
		ll_wrk [LAT] = ll_in [LAT];
		ll_wrk [HGT] = ll_in [HGT];

		xfrmCount = dtcPrm->xfrmCount;
		for (idx = 0;idx < xfrmCount && status >= 0;idx += 1)
		{
			gxStatus = 0;
			xfrmPtr = dtcPrm->xforms [idx];
			if (xfrmPtr == NULL)
			{
				gxStatus = -1;
				CS_stncp (csErrnam,"CS_datum:5",sizeof (csErrnam));
				CS_erpt (cs_ISER);
				goto error;
			}
			methodCode = xfrmPtr->methodCode;

			/* Redundant conversions are optimized out and
			   replaced with dtcTypSkip. */
			if (methodCode == cs_DTCMTH_SKIP) continue;

			/* Call the approriate function.  Convention here is:
			   functions return  0 for OK,
			   functions return -1 for hard error, already reported,
			   functions return  1 for block error not reported yet,
			   functions return  2 for soft error, already reported.
			*/

			if (flag3D)
			{
				/* Here to perform a 3D conversion. */	
				if (xfrmPtr->userDirection == cs_DTCDIR_FWD)
				{
					gxStatus = (*xfrmPtr->frwrd3D)(&xfrmPtr->xforms,ll_wrk,ll_wrk);
				}
				else if (xfrmPtr->userDirection == cs_DTCDIR_INV)
				{
					gxStatus = (*xfrmPtr->invrs3D)(&xfrmPtr->xforms,ll_wrk,ll_wrk);
				}
				else
				{
					/* We should never get here. */
					CS_stncp (csErrnam,"CS_datum:2",MAXPATH);
					CS_erpt (cs_ISER);
					gxStatus = -1;
				}
			}
			else
			{
				/* Here to perform a 2D conversion. */	
				ll_wrk [HGT] = cs_Zero;
				if (xfrmPtr->userDirection == cs_DTCDIR_FWD)
				{
					gxStatus = (*xfrmPtr->frwrd2D)(&xfrmPtr->xforms,ll_wrk,ll_wrk);
				}
				else if (xfrmPtr->userDirection == cs_DTCDIR_INV)
				{
					gxStatus = (*xfrmPtr->invrs2D)(&xfrmPtr->xforms,ll_wrk,ll_wrk);
				}
				else
				{
					/* We should never get here. */
					CS_stncp (csErrnam,"CS_datum:3",MAXPATH);
					CS_erpt (cs_ISER);
					gxStatus = -1;
				}
			}
			if (gxStatus == 0)
			{
				/* This is by far the most frequent case, so we deal with this
				   first, and very quickly. */
				continue;
			}

			if (gxStatus < 0 || ((gxStatus > 0) && (dtcPrm->block_err == cs_DTCFLG_BLK_F)))
			{
				/* A soft error was encountered, but the calling application has
				   indicated that soft errors are to be treated as fatal. */
				status = -1;
			}
			else
			{
				/* A soft error which is to be treated as a soft error.  We report
				   the cause of the first one only.  Of course, if a fatal comes
				   along, that's what gets reported.
				
				   There are basically two situations with an error
				   condition/message for each. */
				rptCode = (status == 2) ? cs_GRD_RNG_FLBK : cs_GRD_RNG_WRN;
			}
		}
		if (!flag3D)
		{
			ll_wrk [HGT] = ll_in [HGT];
		}

		/* If we experienced a hard error, status will be less than zero
		   and the cause will have already been reported, and we have nothing
		   else to do.  In the case of a soft error, status, rptCode, and
		   dtcPtr->block_err determine what happens to a soft error. */
		wasInListFlag = FALSE;
		if (status > 0)
		{
			/* Here if we had a non-fatal error of some sort. Issue the
			   appropriate message per the applications instructions.
			   First, we put the lat/long in the error list. */
			csErrlng = (int)((fabs (ll_in [LNG]) >= 10000.0) ? 9999.99 : ll_in [LNG]);
			if (ll_in [LNG] < 0.0) csErrlng = -csErrlng;
			csErrlat = (int)((fabs (ll_in [LAT]) >= 10000.0) ? 9999.99 : ll_in [LAT]);
			if (ll_in [LAT] < 0.0) csErrlat = -csErrlat;
			if (dtcPrm->listCount < 10)
			{
				for (idx = 0;idx < 10;idx += 1)
				{
					if (dtcPrm->errLngLat [idx][LNG] == csErrlng &&
					    dtcPrm->errLngLat [idx][LAT] == csErrlat)
					{
						/* It's already there, nothing to do.  Save this
						   information for later so we can decide if to
						   report this again, */
						wasInListFlag = TRUE;
						break;
					}
				}
				if (idx >= 10)
				{
					/* It's not there, and we know there's room. */
					dtcPrm->errLngLat [dtcPrm->listCount][LNG] = (short)csErrlng;
					dtcPrm->errLngLat [dtcPrm->listCount][LAT] = (short)csErrlat;
					dtcPrm->listCount += 1;
				}
			}

			/* See how the user wants this type of error handled. */
			switch (dtcPrm->block_err) {
			case cs_DTCFLG_BLK_1:
			case cs_DTCFLG_BLK_10:
				/* User wants the error reported once, per block, but only once. */
				if (wasInListFlag)
				{
					/* We've already reported this block, so ignore this error. */
					status = 0;
				}
				else
				{
					/* Need to report the error.  If we've seen ten different
					   blocks, we bag it and consider the thing fatal. */
					if (dtcPrm->listCount < 10)
					{
						CS_erpt (rptCode);
						/* Since this is a long term user interface component,
						   we return the code value which we have used for
						   many many years. */
						status = 1;
					}
					else if (dtcPrm->block_err == cs_DTCFLG_BLK_10)
					{
						if (dtcPrm->listCount == 10)
						{
							CS_erpt (cs_DTC_SOFTIGNR);
							dtcPrm->listCount += 1;
							status = 1;
						}
						else
						{
							status = 0;
						}
					}
					else
					{
						CS_erpt (cs_DTC_SOFTMAX);
						status = -1;
					}
				}
				break;					

			case cs_DTCFLG_BLK_W:
				/* Issue a warning message and continue. */
				CS_erpt (rptCode);
				status = 1;
				break;

			case cs_DTCFLG_BLK_I:
				/* Here simply to ignore such errors. */
				status = 0;
				break;

			case cs_DTCFLG_BLK_F:
			default:
				/* We should have already dealt with this situation. */
				CS_stncp (csErrnam,"CS_datum:10",MAXPATH);
				CS_erpt (cs_ISER);
				status = -1;
				break;
			}
		}

		/* If no hard errors, return the result of our work. */
		if (status >= 0)
		{
			ll_out [LNG] = ll_wrk [LNG];
			ll_out [LAT] = ll_wrk [LAT];
			ll_out [HGT] = ll_wrk [HGT];
		}
	}
	return status;
error:
	return -1;	
}


//if ((fwdCount + invCount) > 0)
//{
//	/* We'll be returning something.  Dummy up a path object. */
//	CS_stncp (gpDef->pathName,"<Internally generated>",sizeof (gpDef->pathName));
//	CS_stncp (gpDef->srcDatum,src_dt->key_nm,sizeof (gpDef->srcDatum));
//	CS_stncp (gpDef->trgDatum,dst_dt->key_nm,sizeof (gpDef->srcDatum));
//	gpDef->protect = 0;
//	gpDef->reversible = 0;
//	gpDef->elementCount = 1;
//	gpDef->epsgCode = 0;
//	gpDef->variant = 0;
//	CS_stncp (gpDef->description,"Internally generated from the Geodetic Transformation Dictionary",sizeof (gpDef->description));
//	CS_stncp (gpDef->source,"Internally generated",sizeof (gpDef->source));
//	gpDef->group [0] = '\0';
//	pathElePtr = &(gpDef->geodeticPathElements [0]);
//	CS_stncp (pathElePtr->geodeticXformName,gxXfrmName,sizeof (pathElePtr->geodeticXformName));
//	pathElePtr->direction = (fwdCount == 0);
//}
