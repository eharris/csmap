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

struct cs_DatMap_
{
	int to84_via;
	enum cs_DtcXformType to84   [csDTCMAP_SIZE];
	enum cs_DtcXformType from84 [csDTCMAP_SIZE];
};

struct cs_DatMap_ cs_DatMap [] =
{
	{cs_DTCTYP_NAD27,
		{dtcTypNad27ToNad83,     dtcTypNad83ToWgs84,    dtcTypNone,           dtcTypNone  },
		{dtcTypWgs84ToNad83,     dtcTypNad83ToNad27,    dtcTypNone,           dtcTypNone  }
	},
	{cs_DTCTYP_NAD83,
		{dtcTypNad83ToWgs84,     dtcTypNone,            dtcTypNone,           dtcTypNone  },
		{dtcTypWgs84ToNad83,     dtcTypNone,            dtcTypNone,           dtcTypNone  }
	},
	{cs_DTCTYP_AGD66,
		{dtcTypAgd66ToGda94,     dtcTypGda94ToWgs84,    dtcTypNone,           dtcTypNone  },
		{dtcTypWgs84ToGda94,     dtcTypGda94ToAgd66,    dtcTypNone,           dtcTypNone  }
	},
	{cs_DTCTYP_AGD84,
		{dtcTypAgd84ToGda94,     dtcTypGda94ToWgs84,    dtcTypNone,           dtcTypNone  },
		{dtcTypWgs84ToGda94,     dtcTypGda94ToAgd84,    dtcTypNone,           dtcTypNone  }
	},
	{cs_DTCTYP_ED50,
		{dtcTypEd50ToEtrf89,     dtcTypEtrf89ToWgs84,   dtcTypNone,           dtcTypNone  },
		{dtcTypWgs84ToEtrf89,    dtcTypEtrf89ToEd50,    dtcTypNone,           dtcTypNone  }
	},
	{cs_DTCTYP_DHDN,
		{dtcTypDhdnToEtrf89,     dtcTypEtrf89ToWgs84,   dtcTypNone,           dtcTypNone  },
		{dtcTypWgs84ToEtrf89,    dtcTypEtrf89ToDhdn,    dtcTypNone,           dtcTypNone  }
	},
	{cs_DTCTYP_NZGD49,
		{dtcTypNzgd49ToNzgd2K,   dtcTypNzgd2KToWgs84,   dtcTypNone,           dtcTypNone  },
		{dtcTypWgs84ToNzgd2K,    dtcTypNzgd2KToNzgd49,  dtcTypNone,           dtcTypNone  }
	},
	{cs_DTCTYP_ATS77,
		{dtcTypAts77ToCsrs,      dtcTypCsrsToNad83,     dtcTypNad83ToWgs84,   dtcTypNone  },
		{dtcTypWgs84ToNad83,     dtcTypNad83ToCsrs,     dtcTypCsrsToAts77,    dtcTypNone  }
	},
	{cs_DTCTYP_WGS84,
		{dtcTypNone,             dtcTypNone,            dtcTypNone,           dtcTypNone  },
		{dtcTypNone,             dtcTypNone,            dtcTypNone,           dtcTypNone  }
	},
	{cs_DTCTYP_GDA94,
		{dtcTypGda94ToWgs84,     dtcTypNone,            dtcTypNone,           dtcTypNone  },
		{dtcTypWgs84ToGda94,     dtcTypNone,            dtcTypNone,           dtcTypNone  }
	},
	{cs_DTCTYP_NZGD2K,
		{dtcTypNzgd2KToWgs84,     dtcTypNone,            dtcTypNone,          dtcTypNone  },
		{dtcTypWgs84ToNzgd2K,     dtcTypNone,            dtcTypNone,          dtcTypNone  }
	},
	{cs_DTCTYP_CSRS,
		{dtcTypCsrsToNad83,       dtcTypNad83ToWgs84,    dtcTypNone,          dtcTypNone  },
		{dtcTypWgs84ToNad83,      dtcTypNad83ToCsrs,     dtcTypNone,          dtcTypNone  }
	},
	{cs_DTCTYP_TOKYO,
		{dtcTypTokyoToJgd2k,      dtcTypNone,            dtcTypNone,           dtcTypNone  },
		{dtcTypJgd2kToTokyo,      dtcTypNone,            dtcTypNone,           dtcTypNone  }
	},
	{cs_DTCTYP_WGS72,
		{dtcTypWgs72ToWgs84,     dtcTypNone,            dtcTypNone,           dtcTypNone  },
		{dtcTypWgs84ToWgs72,     dtcTypNone,            dtcTypNone,           dtcTypNone  }
	},
	{cs_DTCTYP_HPGN,
		{dtcTypHarnToNad83,      dtcTypNad83ToWgs84,    dtcTypNone,           dtcTypNone  },
		{dtcTypWgs84ToNad83,     dtcTypNad83ToHarn,     dtcTypNone,           dtcTypNone  }
	},
	{cs_DTCTYP_MOLO,
		{dtcTypMolodensky,       dtcTypNone,            dtcTypNone,           dtcTypNone  },
		{dtcTypMolodenskyInv,    dtcTypNone,            dtcTypNone,           dtcTypNone  }
	},
	{cs_DTCTYP_MREG,
		{dtcTypDMAMulReg,        dtcTypNone,            dtcTypNone,           dtcTypNone  },
		{dtcTypDMAMulRegInv,     dtcTypNone,            dtcTypNone,           dtcTypNone  }
	},
	{cs_DTCTYP_BURS,
		{dtcTypBursaWolf,        dtcTypNone,            dtcTypNone,           dtcTypNone  },
		{dtcTypBursaWolfInv,     dtcTypNone,            dtcTypNone,           dtcTypNone  }
	},
	{cs_DTCTYP_7PARM,
		{dtcTypSevenParm,        dtcTypNone,             dtcTypNone,          dtcTypNone  },
		{dtcTypSevenParmInv,     dtcTypNone,             dtcTypNone,          dtcTypNone  }
	},
	{cs_DTCTYP_6PARM,
		{dtcTypSixParm,          dtcTypNone,             dtcTypNone,          dtcTypNone  },
		{dtcTypSixParmInv,       dtcTypNone,             dtcTypNone,          dtcTypNone  }
	},
	{cs_DTCTYP_GEOCTR,
		{dtcTypGeoCtr,           dtcTypNone,             dtcTypNone,          dtcTypNone  },
		{dtcTypGeoCtrInv,        dtcTypNone,             dtcTypNone,          dtcTypNone  }
	},
	{cs_DTCTYP_4PARM,
		{dtcTypFourParm,         dtcTypNone,             dtcTypNone,          dtcTypNone  },
		{dtcTypFourParmInv,      dtcTypNone,             dtcTypNone,          dtcTypNone  }
	},
	{cs_DTCTYP_RGF93,
		{dtcTypNtfToRgf93,       dtcTypNone,             dtcTypNone,          dtcTypNone  },
		{dtcTypRgf93ToNtf,       dtcTypNone,             dtcTypNone,          dtcTypNone  }
	},
	{ cs_DTCTYP_ETRF89,
		{dtcTypNone,             dtcTypNone,            dtcTypNone,           dtcTypNone  },
		{dtcTypNone,             dtcTypNone,            dtcTypNone,           dtcTypNone  }
	},
	{cs_DTCTYP_3PARM,
		{dtcTypThreeParm,        dtcTypNone,            dtcTypNone,          dtcTypNone  },
		{dtcTypThreeParmInv,     dtcTypNone,            dtcTypNone,          dtcTypNone  }
	},
	{cs_DTCTYP_CHENYX,
		{dtcTypCh1903ToPlus,     dtcTypChPlusToChtrs95, dtcTypChtrs95ToEtrf89, dtcTypEtrf89ToWgs84  },
		{dtcTypWgs84ToEtrf89,    dtcTypEtrf89ToChtrs95, dtcTypChtrs95ToChPlus, dtcTypPlusToCh1903   }
	},
	/* The following marks the end of the table. */
	{cs_DTCTYP_NONE,
		{dtcTypNone,            dtcTypNone,              dtcTypNone,          dtcTypNone  },
		{dtcTypNone,            dtcTypNone,              dtcTypNone,          dtcTypNone  }
	}
};

struct cs_Datum_ cs_Wgs84Dt =
{
	"",
	"",
	0.0,
	0.0,
	0.0,
	0.0,
	0.0,
	0.0,
	0.0,
	0.0,
	0.0,
	0.0,
	0.0,
	0,
	"",
	""
};

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

static int nad27ToCsrsChecked = FALSE;
static int nad27ToCsrsExists = FALSE;

static int nad27ToAts77Checked = FALSE;
static int nad27ToAts77Exists = FALSE;

static int ats77ToCsrsChecked = FALSE;
static int ats77ToCsrsExists = FALSE;

struct cs_Dtcprm_ * EXP_LVL3 CSdtcsu (	Const struct cs_Datum_ *src_dt,
										Const struct cs_Datum_ *dst_dt,
										int dat_erf,
										int blk_erf)
{
	extern char cs_Dir [];
	extern char *cs_DirP;
	extern char csErrnam [];
	extern double cs_Zero;
	extern char cs_Csrs27Name [];
	extern char cs_N27A77Name [];
	extern char cs_Ats77Name [];

	int ii, jj;
	int st;
	int same;
	int src_typ, dst_typ;
	int nadFlg, hpgFlg, agd66Flg, agd84Flg, nzgd49Flg, ats77Flg, csrsFlg, jgd2kFlg, rgf93Flg, csrs27Flg, ed50Flg, dhdnFlg, ch1903Flg, n27a77Flg;

	enum cs_DtcXformType xfrmType;
	struct cs_Datum_ *wgs84Ptr;
	struct cs_DatMap_ *map_ptr;
	struct cs_DatMap_ *src_ptr;
	struct cs_DatMap_ *dst_ptr;
	struct cs_Dtcprm_ *dtc_ptr;

	double sameCheck;

	/* Prepare for possible error condition. */
	dtc_ptr = NULL;
	nadFlg = FALSE;
	hpgFlg = FALSE;
	agd66Flg = FALSE;
	agd84Flg = FALSE;
	nzgd49Flg = FALSE;
	ats77Flg = FALSE;
	csrsFlg = FALSE;
	jgd2kFlg = FALSE;
	rgf93Flg = FALSE;
	csrs27Flg = FALSE;
	ed50Flg = FALSE;
	dhdnFlg = FALSE;
	ch1903Flg = FALSE;
	n27a77Flg = FALSE;
	CS_stncp (csErrnam,__FILE__,MAXPATH);

	/* Get a copy of the WGS84 datum definition.  We use that internally. */
	if (cs_Wgs84Dt.key_nm [0] == '\0')
	{
		wgs84Ptr = CS_dtloc ("WGS84");
		if (wgs84Ptr == NULL)
		{
			return NULL;
		}
		memcpy (&cs_Wgs84Dt,wgs84Ptr,sizeof (cs_Wgs84Dt));
		CS_free (wgs84Ptr);
		cs_Wgs84Dt.rot_X = cs_Zero;
		cs_Wgs84Dt.rot_Y = cs_Zero;
		cs_Wgs84Dt.rot_Z = cs_Zero;
		cs_Wgs84Dt.bwscale = cs_Zero;
	}

	/* Arrange so that we check for the existence of the Nad27ToCsrs.gdc file once per execution. */
	if (!nad27ToCsrsChecked &&
		((src_dt->to84_via == cs_DTCTYP_NAD27 && dst_dt->to84_via == cs_DTCTYP_CSRS) ||
	     (src_dt->to84_via == cs_DTCTYP_CSRS  && dst_dt->to84_via == cs_DTCTYP_NAD27)))
	{
		nad27ToCsrsChecked = TRUE;
		nad27ToCsrsExists = FALSE;
		CS_stcpy (cs_DirP,cs_Csrs27Name);
		if (CS_access (cs_Dir,4) == 0)
		{
			nad27ToCsrsExists = TRUE;
		}
	}

	/* Arrange so that we check for the existence of the Nad27ToAts77.gdc file once per execution. */
	if (!nad27ToAts77Checked &&
		((src_dt->to84_via == cs_DTCTYP_NAD27 && dst_dt->to84_via == cs_DTCTYP_ATS77) ||
	     (src_dt->to84_via == cs_DTCTYP_ATS77 && dst_dt->to84_via == cs_DTCTYP_NAD27)))
	{
		nad27ToCsrsChecked = TRUE;
		nad27ToCsrsExists = FALSE;
		CS_stcpy (cs_DirP,cs_N27A77Name);
		if (CS_access (cs_Dir,4) == 0)
		{
			nad27ToAts77Exists = TRUE;
		}
	}

	/* Arrange so that we check for the existence of the Ats77ToCsrs.gdc file once per execution. */
	if (!ats77ToCsrsChecked &&
		((src_dt->to84_via == cs_DTCTYP_ATS77 && dst_dt->to84_via == cs_DTCTYP_CSRS) ||
	     (src_dt->to84_via == cs_DTCTYP_CSRS && dst_dt->to84_via == cs_DTCTYP_ATS77)))
	{
		ats77ToCsrsChecked = TRUE;
		ats77ToCsrsExists = FALSE;
		CS_stcpy (cs_DirP,cs_Ats77Name);
		if (CS_access (cs_Dir,4) == 0)
		{
			ats77ToCsrsExists = TRUE;
		}
	}

	/* If either coordinate system is cartographically referenced,
	   there is no datum conversion. */
	if (src_dt->key_nm [0] == '\0' || dst_dt->key_nm [0] == '\0')
	{
		dtc_ptr = CSnulinit (src_dt->key_nm,dst_dt->key_nm);
		return (dtc_ptr);
	}

	/* If the datums are of the same. */
	src_typ = src_dt->to84_via;
	dst_typ = dst_dt->to84_via;
	same = (src_typ == dst_typ);

	if (same)
	{
		/* The types are the same, see if the ellipsoids are the same. */
		sameCheck = (src_dt->e_rad <= dst_dt->e_rad) ? src_dt->e_rad : dst_dt->e_rad;
		sameCheck *= 1.0E-12;
		if (fabs (src_dt->e_rad - dst_dt->e_rad) > sameCheck) same = FALSE;
		sameCheck = (src_dt->ecent <= dst_dt->ecent) ? src_dt->ecent : dst_dt->ecent;
		sameCheck *= 1.0E-10;
		if (fabs (src_dt->ecent - dst_dt->ecent) > sameCheck) same = FALSE;
	}
	if (same)
	{
		/* The conversion types are the same, and the ellipsoids are the same.
		   check to see if the parameters are the same. */
		switch (src_typ) {

		case cs_DTCTYP_7PARM:
		case cs_DTCTYP_BURS:
			sameCheck = fabs (src_dt->bwscale) * 1.0E-10;
			if (fabs (src_dt->bwscale - dst_dt->bwscale) > sameCheck) same = FALSE;
			/*lint -fallthrough by design */

		case cs_DTCTYP_6PARM:
			sameCheck = 5.0E-06;
			if (fabs (src_dt->rot_X - dst_dt->rot_X) > sameCheck) same = FALSE;
			if (fabs (src_dt->rot_Y - dst_dt->rot_Y) > sameCheck) same = FALSE;
			if (fabs (src_dt->rot_Z - dst_dt->rot_Z) > sameCheck) same = FALSE;
			/*lint -fallthrough by design */

		case cs_DTCTYP_MOLO:
		case cs_DTCTYP_3PARM:
		case cs_DTCTYP_GEOCTR:
			sameCheck = 1.0E-04;
			if (fabs (src_dt->delta_X - dst_dt->delta_X) > sameCheck) same = FALSE;
			if (fabs (src_dt->delta_Y - dst_dt->delta_Y) > sameCheck) same = FALSE;
			if (fabs (src_dt->delta_Z - dst_dt->delta_Z) > sameCheck) same = FALSE;
			break;

		case cs_DTCTYP_MREG:
			if (CS_stricmp (src_dt->key_nm,dst_dt->key_nm)) same = FALSE;
			break;

		default:
			break;
		}
	}

	/* If the datums are still the same after all of that, simply
	   return the null datum conversion. */
	if (same)
	{
		dtc_ptr = CSnulinit (src_dt->key_nm,dst_dt->key_nm);
		return (dtc_ptr);
	}

	/* Search the datum conversion table for the source and destination datum types. */
	src_ptr = NULL;
	dst_ptr = NULL;
	for (map_ptr = cs_DatMap;map_ptr->to84_via != cs_DTCTYP_NONE;map_ptr += 1)
	{
		if (src_dt->to84_via == map_ptr->to84_via) src_ptr = map_ptr;
		if (dst_dt->to84_via == map_ptr->to84_via) dst_ptr = map_ptr;
	}

	/* If we didn't find a source and a destination, we have a software
	   problem.  The code in the dictionary must be wrong, or Norm fell
	   asleep at the wheel. */
	if (src_ptr == NULL || dst_ptr == NULL)
	{
		CS_stncp (csErrnam,"CS_datum:1",MAXPATH);
		CS_erpt (cs_ISER);
		/* We haven't malloc'ed anything yet, so we can still safely return. */
		return NULL;
	}

	/* Allocate the parameter structure and populate it. */
	dtc_ptr = (struct cs_Dtcprm_ *)CS_malc (sizeof (struct cs_Dtcprm_));
	if (dtc_ptr == NULL)
	{
		CS_erpt (cs_NO_MEM);
		return NULL;
	}

	/* Initialize the general components of the structure. */
	dtc_ptr->block_err = (short)blk_erf;
	dtc_ptr->rptCount = 0;
	dtc_ptr->listCount = 0;
	for (ii = 0;ii < 10;ii += 1)
	{
		dtc_ptr->errLngLat [ii][LNG] = 0;
		dtc_ptr->errLngLat [ii][LAT] = 0;
	}
	for (ii = 0;ii < cs_DTCXFRM_MAX;ii += 1)
	{
		dtc_ptr->xforms [ii].xfrmType = dtcTypNone;
		dtc_ptr->xforms [ii].flag3D = FALSE;
		dtc_ptr->xforms [ii].parms.molo = NULL;
	}
	CS_stncp (dtc_ptr->srcKeyName,src_dt->key_nm,sizeof (dtc_ptr->srcKeyName));
	CS_stncp (dtc_ptr->trgKeyName,dst_dt->key_nm,sizeof (dtc_ptr->trgKeyName));

	/* Add conversions from source to WGS84. */
	for (ii = 0;ii < csDTCMAP_SIZE;ii += 1)
	{
		if (ii >= cs_DTCXFRM_MAX)
		{
			CS_stncp (csErrnam,"CS_datum:2",MAXPATH);
			CS_erpt (cs_ISER);
			/* We've malloc'ed something, can't just return anymore. */
			goto error;
		}
		if (src_ptr->to84 [ii] == dtcTypNone)
		{
			break;
		}
		dtc_ptr->xforms [ii].xfrmType = src_ptr->to84 [ii];
	}

	/* We now have the conversion types required to get from the source to
	   WGS84.  Add the conversion types necessary to get from WGS84 to the
	   target datum. */
	for (jj = 0;jj < csDTCMAP_SIZE;ii += 1,jj += 1)
	{
		if (ii >= cs_DTCXFRM_MAX)
		{
			CS_stncp (csErrnam,"CS_datum:3",MAXPATH);
			CS_erpt (cs_ISER);
			goto error;
		}
		if (dst_ptr->from84 [jj] == dtcTypNone)
		{
			break;
		}
		dtc_ptr->xforms [ii].xfrmType = dst_ptr->from84 [jj];
	}

	/* Optimize by removing redundant 83284's and 84283's.  These
	   will occur frequently, other redundant combinations should
	   never occur. */
	for (ii = 0;ii < cs_DTCXFRM_MAX;ii += 1)
	{
		if (dtc_ptr->xforms [ii].xfrmType == dtcTypNone) break;
		if (ii == 0) continue;

		jj = ii - 1;
		if (dtc_ptr->xforms [jj].xfrmType == dtcTypWgs84ToNad83 &&
			dtc_ptr->xforms [ii].xfrmType == dtcTypNad83ToWgs84)
		{
			dtc_ptr->xforms [ii].xfrmType = dtcTypSkip;
			dtc_ptr->xforms [jj].xfrmType = dtcTypSkip;
		}
		if (dtc_ptr->xforms [jj].xfrmType == dtcTypNad83ToWgs84 &&
			dtc_ptr->xforms [ii].xfrmType == dtcTypWgs84ToNad83)
		{
			dtc_ptr->xforms [ii].xfrmType = dtcTypSkip;
			dtc_ptr->xforms [jj].xfrmType = dtcTypSkip;
		}
	}

	/* Remove any skip codes in the xforms list. */
	for (ii = 0;ii < cs_DTCXFRM_MAX;ii++)
	{
		/* Bug out early if we're done. */
		if (dtc_ptr->xforms [ii].xfrmType == dtcTypNone) break;

		/* If the current entry is not a skip code, on to the next. */
		if (dtc_ptr->xforms [ii].xfrmType != dtcTypSkip) continue;

		/* This one is a skip code.  Copy all remaining code up one slot
		   until this entry is something other than a skip code. */
		while (dtc_ptr->xforms [ii].xfrmType == dtcTypSkip)
		{
			for (jj = ii + 1;jj < cs_DTCXFRM_MAX;jj += 1)
			{
				dtc_ptr->xforms [jj - 1].xfrmType = dtc_ptr->xforms [jj].xfrmType;
			}
			dtc_ptr->xforms [jj - 1].xfrmType = dtcTypNone;
		}
	}

	/* Kludge Time.
	   We need to be able to handle the conversion of NAD27 to CSRS directly, not
	   going through WGS84.  This is necessary as the Canadian provinces are
	   distributing grid shift files which convert directly from NAD27 to CSRS.
	   Of course, the results are not identical, but more importantly, clients
	   want to use these files.  So, here goes. */
	if (nad27ToCsrsExists)
	{
		if (src_dt->to84_via == cs_DTCTYP_NAD27 && dst_dt->to84_via == cs_DTCTYP_CSRS)
		{
			/* Here for the forward direct. */
			/* OK, replace any existing conversions with a single one. */
			for (ii = 0;ii < cs_DTCXFRM_MAX;ii += 1)
			{
				if (dtc_ptr->xforms [ii].xfrmType == dtcTypNone) break;
				if (ii == 0) dtc_ptr->xforms [ii].xfrmType = dtcTypNad27ToCsrs;
				else         dtc_ptr->xforms [ii].xfrmType = dtcTypNone;
			}
		}
		else if (src_dt->to84_via == cs_DTCTYP_CSRS && dst_dt->to84_via == cs_DTCTYP_NAD27)
		{
			/* Here for the inverse direct. */
			for (ii = 0;ii < cs_DTCXFRM_MAX;ii += 1)
			{
				if (dtc_ptr->xforms [ii].xfrmType == dtcTypNone) break;
				if (ii == 0) dtc_ptr->xforms [ii].xfrmType = dtcTypCsrsToNad27;
				else         dtc_ptr->xforms [ii].xfrmType = dtcTypNone;
			}
		}
	}

	if (nad27ToAts77Exists)
	{
		if (src_dt->to84_via == cs_DTCTYP_NAD27 && dst_dt->to84_via == cs_DTCTYP_ATS77)
		{
			/* Here for the forward direct. */
			/* OK, replace any existing conversions with a single one. */
			for (ii = 0;ii < cs_DTCXFRM_MAX;ii += 1)
			{
				if (dtc_ptr->xforms [ii].xfrmType == dtcTypNone) break;
				if (ii == 0) dtc_ptr->xforms [ii].xfrmType = dtcTypNad27ToAts77;
				else         dtc_ptr->xforms [ii].xfrmType = dtcTypNone;
			}
		}
		else if (src_dt->to84_via == cs_DTCTYP_ATS77 && dst_dt->to84_via == cs_DTCTYP_NAD27)
		{
			/* Here for the inverse direct. */
			for (ii = 0;ii < cs_DTCXFRM_MAX;ii += 1)
			{
				if (dtc_ptr->xforms [ii].xfrmType == dtcTypNone) break;
				if (ii == 0) dtc_ptr->xforms [ii].xfrmType = dtcTypAts77ToNad27;
				else         dtc_ptr->xforms [ii].xfrmType = dtcTypNone;
			}
		}
	}

	if (ats77ToCsrsExists)
	{
		if (src_dt->to84_via == cs_DTCTYP_ATS77 && dst_dt->to84_via == cs_DTCTYP_CSRS)
		{
			/* Here for the forward direct. */
			/* OK, replace any existing conversions with a single one. */
			for (ii = 0;ii < cs_DTCXFRM_MAX;ii += 1)
			{
				if (dtc_ptr->xforms [ii].xfrmType == dtcTypNone) break;
				if (ii == 0) dtc_ptr->xforms [ii].xfrmType = dtcTypAts77ToCsrs;
				else         dtc_ptr->xforms [ii].xfrmType = dtcTypNone;
			}
		}
		else if (src_dt->to84_via == cs_DTCTYP_CSRS && dst_dt->to84_via == cs_DTCTYP_ATS77)
		{
			/* Here for the inverse direct. */
			for (ii = 0;ii < cs_DTCXFRM_MAX;ii += 1)
			{
				if (dtc_ptr->xforms [ii].xfrmType == dtcTypNone) break;
				if (ii == 0) dtc_ptr->xforms [ii].xfrmType = dtcTypCsrsToAts77;
				else         dtc_ptr->xforms [ii].xfrmType = dtcTypNone;
			}
		}
	}

	/* Perform whatever initialization operations may need to
	   be performed. */
	for (ii = 0,st = 0;st == 0 && ii < cs_DTCXFRM_MAX;ii++)
	{
		/* Get the code for the current conversion. */
		xfrmType = dtc_ptr->xforms [ii].xfrmType;

		/* Just to be sure.  This is a setup function, we can afford this. */
		if (xfrmType == dtcTypSkip)
		{
			CS_stncp (csErrnam,"CS_datum:4",MAXPATH);
			CS_erpt (cs_ISER);
			goto error;
		}

		/* If it is the null conversion, it signals the
		   end of the conversion list. */
		if (xfrmType == dtcTypNone)
		{
			break;
		}

		/* Initialize this specific conversion.  Note that the individual
		   initialization functions are responsible for knowing whether
		   they have already been initialized or not. */
		switch (xfrmType) {

		case dtcTypNad27ToNad83:
		case dtcTypNad83ToNad27:
			st = CSnadInit ();
			if (st == 0) nadFlg = TRUE;
			break;

		case dtcTypAgd66ToGda94:
		case dtcTypGda94ToAgd66:
			/* Initialize the Australian stuff.  Don't know how to do the
			   vertical datum for Australia, but we'll add it here once we
			   do. */
			st = CSagd66Init ();
			if (st == 0) agd66Flg = TRUE;
			break;

		case dtcTypAgd84ToGda94:
		case dtcTypGda94ToAgd84:
			/* Initialize the Australian stuff.  Don't know how to do the
			   vertical datum for Australia, but we'll add it here once we
			   do. */
			st = CSagd84Init ();
			if (st == 0) agd84Flg = TRUE;
			break;

		case dtcTypEd50ToEtrf89:
		case dtcTypEtrf89ToEd50:
			/* Initialize the European stuff.  Don't know how to do the
			   vertical datum for Europe, but we'll add it here once we
			   do. */
			st = CSed50Init ();
			if (st == 0) ed50Flg = TRUE;
			break;

		case dtcTypDhdnToEtrf89:
		case dtcTypEtrf89ToDhdn:
			/* Initialize the European stuff.  Don't know how to do the
			   vertical datum for Europe, but we'll add it here once we
			   do. */
			st = CSdhdnInit ();
			if (st == 0) dhdnFlg = TRUE;
			break;

		case dtcTypCh1903ToPlus:
		case dtcTypPlusToCh1903:
			/* Initialize the Swiss stuff. */
			st = CSch1903Init ();
			if (st == 0) ch1903Flg = TRUE;
			break;

		case dtcTypNzgd49ToNzgd2K:
		case dtcTypNzgd2KToNzgd49:
			/* Initialize the New Zealand stuff.  Don't know how to do the
			   vertical datum for New Zealand, but we'll add it here once we
			   do. */
			st = CSnzgd49Init ();
			if (st == 0) nzgd49Flg = TRUE;
			break;

		case dtcTypAts77ToCsrs:
		case dtcTypCsrsToAts77:
			/* ATS77 is used in New Brunswick and other Maritime provinces
			   of Canada. */
			st = CSats77Init ();
			if (st == 0) ats77Flg = TRUE;
			break;

		case dtcTypNad27ToAts77:
		case dtcTypAts77ToNad27:
			st = CSn27a77Init ();
			if (st == 0) n27a77Flg = TRUE;
			break;

		case dtcTypCsrsToNad83:
		case dtcTypNad83ToCsrs:
			/* CSRS (Canadian Spatial Reference System) is the Canadian equivalent
			   to the US's HPGN/HARN. */
			st = CScsrsInit ();
			if (st == 0) csrsFlg = TRUE;
			break;

		case dtcTypHarnToNad83:
		case dtcTypNad83ToHarn:
			/* Initialize the HARN stuff.  There is no vertical associated
			   with HARN, at least not yet. */
			st = CSharnInit ();
			if (st == 0) hpgFlg = TRUE;
			break;

		case dtcTypTokyoToJgd2k:
		case dtcTypJgd2kToTokyo:
			st = CSjgd2kInit ();
			if (st == 0) jgd2kFlg = TRUE;
			break;

		case dtcTypNtfToRgf93:
		case dtcTypRgf93ToNtf:
			st = CSrgf93Init ();
			if (st == 0) rgf93Flg = TRUE;
			break;

		case dtcTypCsrsToNad27:
		case dtcTypNad27ToCsrs:
			/* CSRS (Canadian Spatial Reference System) is the Canadian equivalent
			   to the US's HPGN/HARN. */
			st = CScsrs27Init ();
			if (st == 0) csrs27Flg = TRUE;
			break;

		case dtcTypNad83ToWgs84:
		case dtcTypWgs84ToNad83:
		case dtcTypWgs84ToWgs72:
		case dtcTypWgs72ToWgs84:
		case dtcTypGda94ToWgs84:
		case dtcTypWgs84ToGda94:
		case dtcTypNzgd2KToWgs84:
		case dtcTypWgs84ToNzgd2K:
		case dtcTypEtrf89ToWgs84:
		case dtcTypWgs84ToEtrf89:
		case dtcTypChtrs95ToEtrf89:
		case dtcTypEtrf89ToChtrs95:
			/* No initializations required, yet. */
			st = 0;
			break;

		case dtcTypChPlusToChtrs95:
		case dtcTypChtrs95ToChPlus:
			dtc_ptr->xforms [ii].parms.geoctr = CS_gcInit (&cs_Chrts95Dt,&cs_Wgs84Dt);
			if (dtc_ptr->xforms [ii].parms.geoctr == NULL)
			{
					st = -1;
			}
			break;

		case dtcTypMolodensky:
			dtc_ptr->xforms [ii].parms.molo = CS_moInit (src_dt,&cs_Wgs84Dt);
			if (dtc_ptr->xforms [ii].parms.molo == NULL)
			{
				st = -1;
			}
			break;

		case dtcTypMolodenskyInv:
			dtc_ptr->xforms [ii].parms.molo = CS_moInit (dst_dt,&cs_Wgs84Dt);
			if (dtc_ptr->xforms [ii].parms.molo == NULL)
			{
				st = -1;
			}
			break;

		case dtcTypDMAMulReg:
			dtc_ptr->xforms [ii].parms.mreg = CS_dmaMrInit (src_dt);
			if (dtc_ptr->xforms [ii].parms.mreg == NULL)
			{
				st = -1;
			}
			break;

		case dtcTypDMAMulRegInv:
			dtc_ptr->xforms [ii].parms.mreg = CS_dmaMrInit (dst_dt);
			if (dtc_ptr->xforms [ii].parms.mreg == NULL)
			{
				st = -1;
			}
			break;

		case dtcTypBursaWolf:
			dtc_ptr->xforms [ii].parms.bursa = CS_bwInit (src_dt,&cs_Wgs84Dt);
			if (dtc_ptr->xforms [ii].parms.bursa == NULL)
			{
				st = -1;
			}
			break;

		case dtcTypBursaWolfInv:
			dtc_ptr->xforms [ii].parms.bursa = CS_bwInit (dst_dt,&cs_Wgs84Dt);
			if (dtc_ptr->xforms [ii].parms.bursa == NULL)
			{
				st = -1;
			}
			break;

		case dtcTypSevenParm:
			dtc_ptr->xforms [ii].parms.parm7 = CS_7pInit (src_dt,&cs_Wgs84Dt);
			if (dtc_ptr->xforms [ii].parms.parm7 == NULL)
			{
				st = -1;
			}
			break;

		case dtcTypSevenParmInv:
			dtc_ptr->xforms [ii].parms.parm7 = CS_7pInit (dst_dt,&cs_Wgs84Dt);
			if (dtc_ptr->xforms [ii].parms.parm7 == NULL)
			{
				st = -1;
			}
			break;

		case dtcTypGeoCtr:
			dtc_ptr->xforms [ii].parms.geoctr = CS_gcInit (src_dt,&cs_Wgs84Dt);
			if (dtc_ptr->xforms [ii].parms.geoctr == NULL)
			{
				st = -1;
			}
			break;

		case dtcTypGeoCtrInv:
			dtc_ptr->xforms [ii].parms.geoctr = CS_gcInit (dst_dt,&cs_Wgs84Dt);
			if (dtc_ptr->xforms [ii].parms.geoctr == NULL)
			{
				st = -1;
			}
			break;

		case dtcTypSixParm:
			dtc_ptr->xforms [ii].parms.parm6 = CS_6pInit (src_dt,&cs_Wgs84Dt);
			if (dtc_ptr->xforms [ii].parms.parm6 == NULL)
			{
				st = -1;
			}
			break;

		case dtcTypSixParmInv:
			dtc_ptr->xforms [ii].parms.parm6 = CS_6pInit (dst_dt,&cs_Wgs84Dt);
			if (dtc_ptr->xforms [ii].parms.parm6 == NULL)
			{
				st = -1;
			}
			break;

		case dtcTypFourParm:
			dtc_ptr->xforms [ii].parms.parm4 = CS_4pInit (src_dt,&cs_Wgs84Dt);
			if (dtc_ptr->xforms [ii].parms.parm4 == NULL)
			{
				st = -1;
			}
			break;

		case dtcTypFourParmInv:
			dtc_ptr->xforms [ii].parms.parm4 = CS_4pInit (dst_dt,&cs_Wgs84Dt);
			if (dtc_ptr->xforms [ii].parms.parm4 == NULL)
			{
				st = -1;
			}
			break;

		case dtcTypThreeParm:
			dtc_ptr->xforms [ii].parms.parm3 = CS_3pInit (src_dt,&cs_Wgs84Dt);
			if (dtc_ptr->xforms [ii].parms.parm3 == NULL)
			{
				st = -1;
			}
			break;

		case dtcTypThreeParmInv:
			dtc_ptr->xforms [ii].parms.parm3 = CS_3pInit (dst_dt,&cs_Wgs84Dt);
			if (dtc_ptr->xforms [ii].parms.parm3 == NULL)
			{
				st = -1;
			}
			break;

		default:
			/* We should never get here. */
			CS_stncp (csErrnam,"CS_datum:5",MAXPATH);
			CS_erpt (cs_ISER);
			goto error;
		}
	}

	/* Make sure it all went OK. */
	if (st != 0) goto error;

	/* Return the pointer to the conversion structure to the user. */
	return dtc_ptr;

error:
	if (dtc_ptr != NULL)
	{
		for (ii = 0;ii < cs_DTCXFRM_MAX;ii += 1)
		{
			CSdtcXformFree (&dtc_ptr->xforms [ii]);
		}
		CS_free (dtc_ptr);
	}
	if (nadFlg)
	{
		CSnadCls ();
	}
	if (hpgFlg)
	{
		CSharnCls ();
	}
	if (agd66Flg)
	{
		CSagd66Cls ();
	}
	if (agd84Flg)
	{
		CSagd84Cls ();
	}
	if (ed50Flg)
	{
		CSed50Cls ();
	}
	if (dhdnFlg)
	{
		CSdhdnCls ();
	}
	if (ch1903Flg)
	{
		CSch1903Cls ();
	}
	if (nzgd49Flg)
	{
		CSnzgd49Cls ();
	}
	if (ats77Flg)
	{
		CSats77Cls ();
	}
	if (csrsFlg)
	{
		CScsrsCls ();
	}
	if (jgd2kFlg)
	{
		CSjgd2kCls ();
	}
	if (rgf93Flg)
	{
		CSrgf93Cls ();
	}
	if (csrs27Flg)
	{
		CScsrs27Cls ();
	}
	if (n27a77Flg)
	{
		CSn27a77Cls ();
	}
	return NULL;
}
/*
	The following function frees allocated memory contained in a
	cs_DtcXform_ structure.  It does not attempt to free the
	cs_DtcXform_ structure itself, but only the contents of the
	provided structure.
*/
void CSdtcXformFree (struct cs_DtcXform_ *xfrmPtr)
{
	struct cs_DmaMReg_ *mregPtr;

	switch (xfrmPtr->xfrmType) {
	case dtcTypMolodensky:
	case dtcTypMolodenskyInv:
		if (xfrmPtr->parms.molo != NULL)
		{
			CS_free (xfrmPtr->parms.molo);
			xfrmPtr->parms.molo = NULL;
		}
		break;
	case dtcTypDMAMulReg:
	case dtcTypDMAMulRegInv:
		if (xfrmPtr->parms.mreg != NULL)
		{
			mregPtr = xfrmPtr->parms.mreg;
			switch (mregPtr->fallback) {
			case dtcTypMolodensky:
				CS_free (mregPtr->fallbackXfrm.molo);
				mregPtr->fallbackXfrm.molo = NULL;
				break;
			case dtcTypSixParm:
				CS_free (mregPtr->fallbackXfrm.parm6);
				mregPtr->fallbackXfrm.parm6 = NULL;
				break;
			case dtcTypSevenParm:
				CS_free (mregPtr->fallbackXfrm.parm7);
				mregPtr->fallbackXfrm.parm7 = NULL;
				break;
			default:
				break;
			}
			CS_free (xfrmPtr->parms.mreg);
			xfrmPtr->parms.mreg = NULL;
		}
		break;
	case dtcTypBursaWolf:
	case dtcTypBursaWolfInv:
		if (xfrmPtr->parms.bursa != NULL)
		{
			CS_free (xfrmPtr->parms.bursa);
			xfrmPtr->parms.bursa = NULL;
		}
		break;
	case dtcTypGeoCtr:
	case dtcTypGeoCtrInv:
	case dtcTypChtrs95ToChPlus:
	case dtcTypChPlusToChtrs95:
		if (xfrmPtr->parms.geoctr != NULL)
		{
			CS_free (xfrmPtr->parms.geoctr);
			xfrmPtr->parms.geoctr = NULL;
		}
		break;
	case dtcTypFourParm:
	case dtcTypFourParmInv:
		if (xfrmPtr->parms.parm4 != NULL)
		{
			CS_free (xfrmPtr->parms.parm4);
			xfrmPtr->parms.parm4 = NULL;
		}
		break;
	case dtcTypSixParm:
	case dtcTypSixParmInv:
		if (xfrmPtr->parms.parm6 != NULL)
		{
			CS_free (xfrmPtr->parms.parm6);
			xfrmPtr->parms.parm6 = NULL;
		}
		break;
	case dtcTypSevenParm:
	case dtcTypSevenParmInv:
		if (xfrmPtr->parms.parm7 != NULL)
		{
			CS_free (xfrmPtr->parms.parm7);
			xfrmPtr->parms.parm7 = NULL;
		}
		break;
	case dtcTypThreeParm:
	case dtcTypThreeParmInv:
		if (xfrmPtr->parms.parm3 != NULL)
		{
			CS_free (xfrmPtr->parms.parm3);
			xfrmPtr->parms.parm3 = NULL;
		}
		break;
	case dtcTypNone:
	default:
		break;
	}
}
/**********************************************************************
**	CS_dtcls (dtc_ptr);
**
**	struct cs_Dtcprm_ *dtc_ptr;	pointer to the datum conversion parameter
**								block controlling the datum conversion which
**								is to be disabled.
**********************************************************************/
void EXP_LVL3 CS_dtcls (struct cs_Dtcprm_ *dtc_ptr)
{
	extern char csErrnam [];

	int ii;
	enum cs_DtcXformType xfrmType;
	
	/* Don't want to crash. */
	if (dtc_ptr == NULL) return;

	/* Close whatever datum conversions are indicated
	   in the conversion list. */
	for (ii = 0;ii < cs_DTCXFRM_MAX;ii++)
	{
		/* Extract the code value. */
		xfrmType = dtc_ptr->xforms [ii].xfrmType;

		/* See if we are done yet. */
		if (xfrmType == dtcTypNone) break;

		/* Close the conversion indicated by this
		   code value. */
		switch (xfrmType) {

		case dtcTypNad27ToNad83:
		case dtcTypNad83ToNad27:
			CSnadCls ();
			if (dtc_ptr->xforms [ii].flag3D) CSvrtconCls ();
			dtc_ptr->xforms [ii].flag3D = FALSE;
			break;

		case dtcTypAgd66ToGda94:
		case dtcTypGda94ToAgd66:
			CSagd66Cls ();
			break;

		case dtcTypAgd84ToGda94:
		case dtcTypGda94ToAgd84:
			CSagd84Cls ();
			break;

		case dtcTypEd50ToEtrf89:
		case dtcTypEtrf89ToEd50:
			CSed50Cls ();
			break;

		case dtcTypDhdnToEtrf89:
		case dtcTypEtrf89ToDhdn:
			CSdhdnCls ();
			break;

		case dtcTypCh1903ToPlus:
		case dtcTypPlusToCh1903:
			CSch1903Cls ();
			break;

		case dtcTypNzgd49ToNzgd2K:
		case dtcTypNzgd2KToNzgd49:
			CSnzgd49Cls ();
			break;

		case dtcTypAts77ToCsrs:
		case dtcTypCsrsToAts77:
			CSats77Cls ();
			break;

		case dtcTypNad27ToAts77:
		case dtcTypAts77ToNad27:
			CSn27a77Cls ();
			break;

		case dtcTypNad83ToHarn:
		case dtcTypHarnToNad83:
			CSharnCls ();
			break;

		case dtcTypNad83ToCsrs:
		case dtcTypCsrsToNad83:
			CScsrsCls ();
			break;

		case dtcTypNad27ToCsrs:
		case dtcTypCsrsToNad27:
			CScsrs27Cls ();
			break;

		case dtcTypTokyoToJgd2k:
		case dtcTypJgd2kToTokyo:
			CSjgd2kCls ();
			break;

		case dtcTypNtfToRgf93:
		case dtcTypRgf93ToNtf:
			CSrgf93Cls ();
			break;

		case dtcTypSkip:
		case dtcTypNad83ToWgs84:
		case dtcTypWgs84ToNad83:
		case dtcTypWgs84ToWgs72:
		case dtcTypWgs72ToWgs84:
		case dtcTypWgs84ToGda94:
		case dtcTypGda94ToWgs84:
		case dtcTypWgs84ToNzgd2K:
		case dtcTypNzgd2KToWgs84:
		case dtcTypEtrf89ToWgs84:
		case dtcTypWgs84ToEtrf89:
		case dtcTypEtrf89ToChtrs95:
		case dtcTypChtrs95ToEtrf89:
			/* At the current time, all the above do not
			   require any closing at all, i.e. these
			   conversions do not allocate any specific
			   resources.  This, of course, is subject
			   to change. */
			break;

		case dtcTypMolodensky:
		case dtcTypMolodenskyInv:
		case dtcTypBursaWolf:
		case dtcTypBursaWolfInv:
		case dtcTypDMAMulReg:
		case dtcTypDMAMulRegInv:
		case dtcTypSevenParm:
		case dtcTypSevenParmInv:
		case dtcTypGeoCtr:
		case dtcTypGeoCtrInv:
		case dtcTypSixParm:
		case dtcTypSixParmInv:
		case dtcTypFourParm:
		case dtcTypFourParmInv:
		case dtcTypThreeParm:
		case dtcTypThreeParmInv:
		case dtcTypChtrs95ToChPlus:
		case dtcTypChPlusToChtrs95:
			CSdtcXformFree (&dtc_ptr->xforms [ii]);
			break;

		default:
			CS_stncp (csErrnam,"CS_datum:6",MAXPATH);
			CS_erpt (cs_ISER);
			break;
		}
	}

	/* All recources allocated by the specific conversions
	   have been recovered.  Free the conversion structure
	   itself. */
	CS_free (dtc_ptr);

	/* That's all there is to that. */
	return;
}

/**********************************************************************
**	Constructs the null datum conversion.
**********************************************************************/
struct cs_Dtcprm_ * EXP_LVL9 CSnulinit (Const char *src_name,Const char *dst_name)
{
	cs_Register struct cs_Dtcprm_ *dtc_ptr;

	dtc_ptr = (struct cs_Dtcprm_ *)CS_malc (sizeof (struct cs_Dtcprm_));
	if (dtc_ptr == NULL)
	{
		CS_erpt (cs_NO_MEM);
		return (dtc_ptr);
	}
	CS_stncp (dtc_ptr->srcKeyName,src_name,sizeof (dtc_ptr->srcKeyName));
	CS_stncp (dtc_ptr->trgKeyName,dst_name,sizeof (dtc_ptr->trgKeyName));
	dtc_ptr->block_err = cs_DTCFLG_BLK_F;
	dtc_ptr->xforms [0].xfrmType = dtcTypNone;
	dtc_ptr->xforms [0].flag3D = FALSE;
	dtc_ptr->xforms [0].parms.molo = NULL;
	return (dtc_ptr);
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
**	datum shift using alternative techniques, in case the
**	lat/long is marginally outside of the region of datum
**	shift availability.
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
int EXP_LVL3 CS_dtcvt (struct cs_Dtcprm_ *dtc_ptr,Const double ll_in [3],double ll_out [3])
{
	extern csFILE* csDiagnostic;
	extern char csErrnam [];
	extern int csErrlng;
	extern int csErrlat;
	extern double cs_Zero;

	int ii;
	int itrStat;				/* status of current iteration */
	int status;				/* accumulated status */
	int wasInListFlag;
	int rptCode;

	enum cs_DtcXformType xfrmType;

	double ll_wrk [3];

	if (csDiagnostic != 0)
	{
		fprintf (csDiagnostic,"%s[%d]\n",modl_name,__LINE__);
	}
	CS_stncp (csErrnam,"CS_datum:0",MAXPATH);
	rptCode = cs_ISER;

	/* Regardless of what happens in all of this, at a minimum we
	   guarantee that ll_out will be set to ll_in. */
	ll_out [LNG] = ll_in [LNG];
	ll_out [LAT] = ll_in [LAT];
	ll_out [HGT] = ll_in [HGT];

	status = 0;
	if (dtc_ptr != NULL && dtc_ptr->xforms [0].xfrmType != dtcTypNone)
	{
		/* There are one or more conversions to be performed. */
		ll_wrk [LNG] = ll_in [LNG];
		ll_wrk [LAT] = ll_in [LAT];
		ll_wrk [HGT] = ll_in [HGT];

		/* Perform the conversion as indicated in the conversion
		   map.  Notice that there may be up to (currently) four
		   of them.  This is required as there is no direct
		   conversion from every geodetic reference system to
		   every other one and while there is a general case,
		   there exist much more accurate techniques for certain
		   conversions. */
		for (ii = 0;ii < cs_DTCXFRM_MAX;ii++)
		{
			itrStat = 0;
			xfrmType = dtc_ptr->xforms [ii].xfrmType;
			if (xfrmType == dtcTypNone)
			{
				/* We've encountered the end of the
				   list. We're done. */
				break;
			}

			/* Redundant conversions are optimized out and
			   replaced with dtcTypSkip. */
			if (xfrmType == dtcTypSkip) continue;

			/* Call the approriate function.  Convention here is:
			   functions return  0 for OK,
			   functions return -1 for hard error, already reported,
			   functions return  1 for block error not reported yet,
			   functions return  2 for soft error, already reported.
			*/

			ll_wrk [HGT] = cs_Zero;
			if (csDiagnostic != 0)
			{
				fprintf (csDiagnostic,"%s[%d] %d\n",modl_name,__LINE__,xfrmType);
			}
			switch (xfrmType) {

			case dtcTypNad27ToNad83:
				itrStat = CSnad27ToNad83 (ll_wrk,ll_wrk);
				break;

			case dtcTypNad83ToNad27:
				itrStat = CSnad83ToNad27 (ll_wrk,ll_wrk);
				break;

			case dtcTypAgd66ToGda94:
				itrStat = CSagd66ToGda94 (ll_wrk,ll_wrk);
				break;

			case dtcTypGda94ToAgd66:
				itrStat = CSgda94ToAgd66 (ll_wrk,ll_wrk);
				break;

			case dtcTypAgd84ToGda94:
				itrStat = CSagd84ToGda94 (ll_wrk,ll_wrk);
				break;

			case dtcTypGda94ToAgd84:
				itrStat = CSgda94ToAgd84 (ll_wrk,ll_wrk);
				break;

			case dtcTypEd50ToEtrf89:
				itrStat = CSed50ToEtrf89 (ll_wrk,ll_wrk);
				break;

			case dtcTypEtrf89ToEd50:
				itrStat = CSetrf89ToEd50 (ll_wrk,ll_wrk);
				break;

			case dtcTypDhdnToEtrf89:
				itrStat = CSdhdnToEtrf89 (ll_wrk,ll_wrk);
				break;

			case dtcTypEtrf89ToDhdn:
				itrStat = CSetrf89ToDhdn (ll_wrk,ll_wrk);
				break;

			case dtcTypChtrs95ToEtrf89:
				itrStat = CSchtrs95ToEtrf89 (ll_wrk,ll_wrk);
				break;

			case dtcTypEtrf89ToChtrs95:
				itrStat = CSetrf89ToChtrs95 (ll_wrk,ll_wrk);
				break;

			case dtcTypCh1903ToPlus:
				itrStat = CSch1903ToPlus (ll_wrk,ll_wrk);
				break;

			case dtcTypPlusToCh1903:
				itrStat = CSplusToCh1903 (ll_wrk,ll_wrk);
				break;

			case dtcTypNzgd49ToNzgd2K:
				itrStat = CSnzgd49ToNzgd2K (ll_wrk,ll_wrk);
				break;

			case dtcTypNzgd2KToNzgd49:
				itrStat = CSnzgd2KToNzgd49 (ll_wrk,ll_wrk);
				break;

			case dtcTypAts77ToCsrs:
				itrStat = CSats77ToCsrs (ll_wrk,ll_wrk);
				break;

			case dtcTypCsrsToAts77:
				itrStat = CScsrsToAts77 (ll_wrk,ll_wrk);
				break;

			case dtcTypNad27ToAts77:
				itrStat = CSnad27ToAts77 (ll_wrk,ll_wrk);
				break;

			case dtcTypAts77ToNad27:
				itrStat = CSats77ToNad27 (ll_wrk,ll_wrk);
				break;

			case dtcTypNad83ToHarn:
				itrStat = CSnad83ToHarn (ll_wrk,ll_wrk);
				break;

			case dtcTypHarnToNad83:
				itrStat = CSharnToNad83 (ll_wrk,ll_wrk);
				break;

			case dtcTypNad83ToWgs84:
				itrStat = CSnad83ToWgs84 (ll_wrk,ll_wrk);
				break;

			case dtcTypWgs84ToNad83:
				itrStat = CSwgs84ToNad83 (ll_wrk,ll_wrk);
				break;

			case dtcTypEtrf89ToWgs84:
				itrStat = CSetrf89ToWgs84 (ll_wrk,ll_wrk);
				break;

			case dtcTypWgs84ToEtrf89:
				itrStat = CSwgs84ToEtrf89 (ll_wrk,ll_wrk);
				break;

			case dtcTypCsrsToNad83:
				itrStat = CScsrsToNad83 (ll_wrk,ll_wrk);
				break;

			case dtcTypNad83ToCsrs:
				itrStat = CSnad83ToCsrs (ll_wrk,ll_wrk);
				break;

			case dtcTypCsrsToNad27:
				itrStat = CScsrsToNad27 (ll_wrk,ll_wrk);
				break;

			case dtcTypNad27ToCsrs:
				itrStat = CSnad27ToCsrs (ll_wrk,ll_wrk);
				break;

			case dtcTypGda94ToWgs84:
				itrStat = CSgda94ToWgs84 (ll_wrk,ll_wrk);
				break;

			case dtcTypWgs84ToGda94:
				itrStat = CSwgs84ToGda94 (ll_wrk,ll_wrk);
				break;

			case dtcTypNzgd2KToWgs84:
				itrStat = CSnzgd2KToWgs84 (ll_wrk,ll_wrk);
				break;

			case dtcTypWgs84ToNzgd2K:
				itrStat = CSwgs84ToNzgd2K (ll_wrk,ll_wrk);
				break;

			case dtcTypTokyoToJgd2k:
				itrStat = CStokyoToJgd2k (ll_wrk,ll_wrk);
				break;

			case dtcTypJgd2kToTokyo:
				itrStat = CSjgd2kToTokyo (ll_wrk,ll_wrk);
				break;

			case dtcTypNtfToRgf93:
				itrStat = CSntfToRgf93 (ll_wrk,ll_wrk);
				break;

			case dtcTypRgf93ToNtf:
				itrStat = CSrgf93ToNtf (ll_wrk,ll_wrk);
				break;

			case dtcTypWgs84ToWgs72:
				itrStat = CSwgs84ToWgs72 (ll_wrk,ll_wrk);
				break;

			case dtcTypWgs72ToWgs84:
				itrStat = CSwgs72ToWgs84 (ll_wrk,ll_wrk);
				break;

			case dtcTypMolodensky:
				itrStat = CS_mo2dFowrd (ll_wrk,ll_wrk,dtc_ptr->xforms [ii].parms.molo);
				break;

			case dtcTypMolodenskyInv:
				itrStat = CS_mo2dInvrs (ll_wrk,ll_wrk,dtc_ptr->xforms [ii].parms.molo);
				break;

			case dtcTypBursaWolf:
				itrStat = CS_bw2dFowrd (ll_wrk,ll_wrk,dtc_ptr->xforms [ii].parms.bursa);
				break;

			case dtcTypBursaWolfInv:
				itrStat = CS_bw2dInvrs (ll_wrk,ll_wrk,dtc_ptr->xforms [ii].parms.bursa);
				break;

			case dtcTypDMAMulReg:
				itrStat = CS_dmaMr2dFowrd (ll_wrk,ll_wrk,dtc_ptr->xforms [ii].parms.mreg);
				break;

			case dtcTypDMAMulRegInv:
				itrStat = CS_dmaMr2dInvrs (ll_wrk,ll_wrk,dtc_ptr->xforms [ii].parms.mreg);
				break;

			case dtcTypSevenParm:
				itrStat = CS_7p2dFowrd (ll_wrk,ll_wrk,dtc_ptr->xforms [ii].parms.parm7);
				break;

			case dtcTypSevenParmInv:
				itrStat = CS_7p2dInvrs (ll_wrk,ll_wrk,dtc_ptr->xforms [ii].parms.parm7);
				break;

			case dtcTypChPlusToChtrs95:
			case dtcTypGeoCtr:
				itrStat = CS_gc2dFowrd (ll_wrk,ll_wrk,dtc_ptr->xforms [ii].parms.geoctr);
				break;

			case dtcTypChtrs95ToChPlus:
			case dtcTypGeoCtrInv: 
				itrStat = CS_gc2dInvrs (ll_wrk,ll_wrk,dtc_ptr->xforms [ii].parms.geoctr);
				break;

			case dtcTypSixParm:
				itrStat = CS_6p2dFowrd (ll_wrk,ll_wrk,dtc_ptr->xforms [ii].parms.parm6);
				break;

			case dtcTypSixParmInv:
				itrStat = CS_6p2dInvrs (ll_wrk,ll_wrk,dtc_ptr->xforms [ii].parms.parm6);
				break;

			case dtcTypFourParm:
				itrStat = CS_4p2dFowrd (ll_wrk,ll_wrk,dtc_ptr->xforms [ii].parms.parm4);
				break;

			case dtcTypFourParmInv: 
				itrStat = CS_4p2dInvrs (ll_wrk,ll_wrk,dtc_ptr->xforms [ii].parms.parm4);
				break;

			case dtcTypThreeParm:
				itrStat = CS_3p2dFowrd (ll_wrk,ll_wrk,dtc_ptr->xforms [ii].parms.parm3);
				break;

			case dtcTypThreeParmInv: 
				itrStat = CS_3p2dInvrs (ll_wrk,ll_wrk,dtc_ptr->xforms [ii].parms.parm3);
				break;

			default:
				/* We should never get here. */
				CS_stncp (csErrnam,"CS_datum:7",MAXPATH);
				CS_erpt (cs_ISER);
				status = -1;
				break;
			}

			if (csDiagnostic != 0)
			{
				fprintf (csDiagnostic,"%s[%d] %d\n",modl_name,__LINE__,itrStat);
			}

			/* See what happened.  If itrStat is negative, we have a fatal error. */
			if (itrStat < 0)
			{
				/* In the case of status < 0, the cause should have already been reported. */
				status = itrStat;
				break;
			}
			else if (itrStat > 0 && dtc_ptr->block_err == cs_DTCFLG_BLK_F)
			{
				/* The calling application instructed us to consider soft errors
				   as hard errors.  We convert the current soft error to a
				   hard (fatal) error.  We need to derive the correct code
				   value.  Could have done that above, but that would slow
				   successful calculations. */
				status = -1;
				switch (xfrmType) {
				case dtcTypNad27ToNad83:
				case dtcTypNad83ToNad27:
					rptCode = cs_NAD_RNG_F;
					break;
				case dtcTypAgd66ToGda94:
				case dtcTypGda94ToAgd66:
					rptCode = cs_AGD66_RNG_F;
					break;
				case dtcTypAgd84ToGda94:
				case dtcTypGda94ToAgd84:
					rptCode = cs_AGD84_RNG_F;
					break;
				case dtcTypEd50ToEtrf89:
				case dtcTypEtrf89ToEd50:
					rptCode = cs_ED50_RNG_F;
					break;
				case dtcTypDhdnToEtrf89:
				case dtcTypEtrf89ToDhdn:
					rptCode = cs_DHDN_RNG_F;
					break;
				case dtcTypNzgd49ToNzgd2K:
				case dtcTypNzgd2KToNzgd49:
					rptCode = cs_NZGD49_RNG_F;
					break;
				case dtcTypTokyoToJgd2k:
				case dtcTypJgd2kToTokyo:
					rptCode = cs_TOKYO_RNG_F;
					break;
				case dtcTypNtfToRgf93:
				case dtcTypRgf93ToNtf:
					rptCode = cs_RGF93_RNG_F;
					break;
				case dtcTypAts77ToCsrs:
				case dtcTypCsrsToAts77:
					rptCode = cs_ATS77_RNG_F;
					break;
				case dtcTypNad27ToAts77:
				case dtcTypAts77ToNad27:
					rptCode = cs_N27A77_RNG_F;
					break;
				case dtcTypNad83ToHarn:
				case dtcTypHarnToNad83:
					rptCode = cs_HARN_RNG_F;
					break;
				case dtcTypCsrsToNad83:
				case dtcTypNad83ToCsrs:
					rptCode = cs_CSRS_RNG_F;
					break;
				case dtcTypCsrsToNad27:
				case dtcTypNad27ToCsrs:
					rptCode = cs_CSRS27_RNG_F;
					break;
				case dtcTypCh1903ToPlus:
				case dtcTypPlusToCh1903:
					rptCode = cs_CHENYX_RNG_F;
					break;
				case dtcTypMolodensky:
				case dtcTypMolodenskyInv:
				case dtcTypBursaWolf:
				case dtcTypBursaWolfInv:
				case dtcTypSevenParm:
				case dtcTypSevenParmInv:
				case dtcTypGeoCtr:
				case dtcTypGeoCtrInv: 
				case dtcTypSixParm:
				case dtcTypSixParmInv:
				case dtcTypFourParm:
				case dtcTypFourParmInv: 
				case dtcTypThreeParm:
				case dtcTypThreeParmInv: 
				case dtcTypChtrs95ToChPlus:
				case dtcTypChPlusToChtrs95:
					rptCode = cs_XYZ_ITR;
					break;
				case dtcTypDMAMulReg:
				case dtcTypDMAMulRegInv:
					rptCode = (status == 2) ? cs_MREG_RANGE : cs_MREG_CNVRG;
					break;
				case dtcTypNad83ToWgs84:
				case dtcTypWgs84ToNad83:
				case dtcTypGda94ToWgs84:
				case dtcTypWgs84ToGda94:
				case dtcTypNzgd2KToWgs84:
				case dtcTypWgs84ToNzgd2K:
				case dtcTypWgs84ToWgs72:
				case dtcTypWgs72ToWgs84:
				case dtcTypEtrf89ToWgs84:
				case dtcTypWgs84ToEtrf89:
				case dtcTypEtrf89ToChtrs95:
				case dtcTypChtrs95ToEtrf89:
					/* Currently, these conversions can't produce an error,
					   soft or hard, so it is difficult to determine which
					   error message to report. */
					CS_stncp (csErrnam,"CS_datum:7A",MAXPATH);
					rptCode = cs_ISER;
					break;
				default:
					CS_stncp (csErrnam,"CS_datum:8",MAXPATH);
					rptCode = cs_ISER;
					break;
				}
				CS_erpt (rptCode);
				break;
			}
			else if (itrStat > 0)
			{
				/* A soft error which is to be treated as a soft error.  We report
				   the cause of the first one only.  Of course, if a fatal comes
				   along, that's what gets reported. */
				if (status == 0)
				{
					/* Here if we have not reported an error before.  We'll
					   report this one.  Now, we figure out what to
					   report.  This could have been done above, but it
					   is faster to do it here since errors are rarer than
					   success. */
					status = itrStat;
					switch (xfrmType) {
					case dtcTypNad27ToNad83:
					case dtcTypNad83ToNad27:
						rptCode = (status == 2) ? cs_NAD_RNG_A : cs_NAD_RNG_W;
						break;
					case dtcTypAgd66ToGda94:
					case dtcTypGda94ToAgd66:
						rptCode = (status == 2) ? cs_AGD66_RNG_A : cs_AGD66_RNG_W;
						break;
					case dtcTypAgd84ToGda94:
					case dtcTypGda94ToAgd84:
						rptCode = (status == 2) ? cs_AGD84_RNG_A : cs_AGD84_RNG_W;
						break;
					case dtcTypEd50ToEtrf89:
					case dtcTypEtrf89ToEd50:
						rptCode = (status == 2) ? cs_ED50_RNG_A : cs_ED50_RNG_W;
						break;
					case dtcTypDhdnToEtrf89:
					case dtcTypEtrf89ToDhdn:
						rptCode = (status == 2) ? cs_DHDN_RNG_A : cs_DHDN_RNG_W;
						break;
					case dtcTypNzgd49ToNzgd2K:
					case dtcTypNzgd2KToNzgd49:
						rptCode = (status == 2) ? cs_NZGD49_RNG_A : cs_NZGD49_RNG_W;
						break;
					case dtcTypTokyoToJgd2k:
					case dtcTypJgd2kToTokyo:
						rptCode = (status == 2) ? cs_TOKYO_RNG_A : cs_TOKYO_RNG_W;
						break;
					case dtcTypNtfToRgf93:
					case dtcTypRgf93ToNtf:
						rptCode = (status == 2) ? cs_RGF93_RNG_A : cs_RGF93_RNG_W;
						break;
					case dtcTypAts77ToCsrs:
					case dtcTypCsrsToAts77:
						rptCode = (status == 2) ? cs_ATS77_RNG_A : cs_ATS77_RNG_W;
						break;
					case dtcTypNad27ToAts77:
					case dtcTypAts77ToNad27:
						rptCode = (status == 2) ? cs_N27A77_RNG_A : cs_N27A77_RNG_W;
						break;
					case dtcTypNad83ToHarn:
					case dtcTypHarnToNad83:
						rptCode = (status == 2) ? cs_HARN_RNG_A : cs_HARN_RNG_W;
						break;
					case dtcTypCsrsToNad83:
					case dtcTypNad83ToCsrs:
						rptCode = (status == 2) ? cs_CSRS_RNG_A : cs_CSRS_RNG_W;
						break;
					case dtcTypCsrsToNad27:
					case dtcTypNad27ToCsrs:
						rptCode = (status == 2) ? cs_CSRS27_RNG_A : cs_CSRS27_RNG_W;
						break;
					case dtcTypCh1903ToPlus:
					case dtcTypPlusToCh1903:
						rptCode = (status == 2) ? cs_CHENYX_RNG_A : cs_CHENYX_RNG_W;
						break;
					case dtcTypMolodensky:
					case dtcTypMolodenskyInv:
					case dtcTypBursaWolf:
					case dtcTypBursaWolfInv:
					case dtcTypSevenParm:
					case dtcTypSevenParmInv:
					case dtcTypGeoCtr:
					case dtcTypGeoCtrInv:
					case dtcTypSixParm:
					case dtcTypSixParmInv:
					case dtcTypFourParm:
					case dtcTypFourParmInv:
					case dtcTypThreeParm:
					case dtcTypThreeParmInv:
					case dtcTypChtrs95ToChPlus:
					case dtcTypChPlusToChtrs95:
						rptCode = cs_XYZ_ITR;
						status = -1;
						break;
					case dtcTypDMAMulReg:
					case dtcTypDMAMulRegInv:
						rptCode = (status == 2) ? cs_MREG_RANGE : cs_MREG_CNVRG;
						break;
					case dtcTypNad83ToWgs84:
					case dtcTypWgs84ToNad83:
					case dtcTypGda94ToWgs84:
					case dtcTypWgs84ToGda94:
					case dtcTypNzgd2KToWgs84:
					case dtcTypWgs84ToNzgd2K:
					case dtcTypWgs84ToWgs72:
					case dtcTypWgs72ToWgs84:
					case dtcTypEtrf89ToWgs84:
					case dtcTypWgs84ToEtrf89:
					case dtcTypEtrf89ToChtrs95:
					case dtcTypChtrs95ToEtrf89:
						CS_stncp (csErrnam,"CS_datum:8A",MAXPATH);
						CS_erpt (cs_ISER);
						status = -1;
						break;
					default:
						CS_stncp (csErrnam,"CS_datum:9",MAXPATH);
						CS_erpt (cs_ISER);
						status = -1;
						break;
					}
					/* We leave the reulst of all this in rptCode and
					   status.  There are more complications to deal with,
					   as coded below. */
				}
			}
			/* Break if we ended up with a fatal status. */
			if (status < 0) break;
		}
		ll_wrk [HGT] = ll_in [HGT];

		/* If we experienced a hard error, the cause will have already been
		   reported, and we have nothing to do.  In the case of a soft
		   error, status, rptCode, and dtcPtr->block_err determine what
		   happens to a soft error. */
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
			if (dtc_ptr->listCount < 10)
			{
				for (ii = 0;ii < 10;ii += 1)
				{
					if (dtc_ptr->errLngLat [ii][LNG] == csErrlng &&
					    dtc_ptr->errLngLat [ii][LAT] == csErrlat)
					{
						/* It's already there, nothing to do.  Save this
						   information for later so we can decide if to
						   report this again, */
						wasInListFlag = TRUE;
						break;
					}
				}
				if (ii >= 10)
				{
					/* It's not there, and we know there's room. */
					dtc_ptr->errLngLat [dtc_ptr->listCount][LNG] = (short)csErrlng;
					dtc_ptr->errLngLat [dtc_ptr->listCount][LAT] = (short)csErrlat;
					dtc_ptr->listCount += 1;
				}
			}

			/* See how the user wants this type of error handled. */
			switch (dtc_ptr->block_err) {
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
					if (dtc_ptr->listCount < 10)
					{
						CS_erpt (rptCode);
						/* Since this is a long term user interface component,
						   we return the code value which we have used for
						   many many years.  Some app's won't know what a
						   2 return value would mean. */
						status = 1;
					}
					else if (dtc_ptr->block_err == cs_DTCFLG_BLK_10)
					{
						if (dtc_ptr->listCount == 10)
						{
							CS_erpt (cs_DTC_SOFTIGNR);
							dtc_ptr->listCount += 1;
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

	/* The conversion is complete.  Return to the user. */
	if (csDiagnostic != 0)
	{
		fprintf (csDiagnostic,"%s[%d] %d\n",modl_name,__LINE__,status);
	}
	return status;
}

/*  This is the three dimensional datum conversion function. */
int EXP_LVL3 CS_dtcvt3D (struct cs_Dtcprm_ *dtc_ptr,Const double ll_in [3],double ll_out [3])
{
	extern csFILE* csDiagnostic;
	extern char csErrnam [];
	extern int csErrlng;
	extern int csErrlat;

	enum cs_DtcXformType xfrmType;

	int ii;
	int stat3D;					/* status of vertical conversion */
	int itrStat;				/* status of current iteration */
	int status;					/* accumulated status */
	int wasInListFlag;
	int rptCode;
	double deltaHgt;
	double ll_wrk [3];
	
	if (csDiagnostic != 0)
	{
		fprintf (csDiagnostic,"%s[%d]\n",modl_name,__LINE__);
	}

	CS_stncp (csErrnam,"CS_datum:00",MAXPATH);
	rptCode = cs_ISER;

	/* Regardless of what happens in all of this, at a minimum we
	   guarantee that ll_out will be set to ll_in. */

	ll_out [LNG] = ll_in [LNG];
	ll_out [LAT] = ll_in [LAT];
	ll_out [HGT] = ll_in [HGT];

	status = 0;
	if (dtc_ptr != NULL && dtc_ptr->xforms [0].xfrmType != dtcTypNone)
	{
		/* There are one or more conversions to be performed. */
		ll_wrk [LNG] = ll_in [LNG];
		ll_wrk [LAT] = ll_in [LAT];
		ll_wrk [HGT] = ll_in [HGT];

		/* Perform the conversion as indicated in the conversion
		   map.  Notice that there may be up to (currently) four
		   of them.  This is required as there is no direct
		   conversion from every geodetic reference system to
		   every other one and while there is a general case,
		   there exist much more accurate techniques for certain
		   conversions. */
		for (ii = 0;ii < cs_DTCXFRM_MAX;ii++)
		{
			stat3D = 0;
			itrStat = 0;
			xfrmType = dtc_ptr->xforms [ii].xfrmType;
			if (xfrmType == dtcTypNone)
			{
				/* We've encountered the end of the
				   list. We're done. */
				break;
			}

			/* Redundant conversions are optimized out and
			   replaced with csGRFC_SKIP. */
			if (xfrmType == dtcTypSkip) continue;

			/* Call the approriate function.  Convention here is:
			   functions return  0 for OK,
			   functions return -1 for hard error, cause already reported.
			   functions return +1 for block error, not yet reported.
			   functions return >1 for soft error, already reported. */
			if (csDiagnostic != 0)
			{
				fprintf (csDiagnostic,"%s[%d] %d\n",modl_name,__LINE__,xfrmType);
			}
			switch (xfrmType) {

			case dtcTypNad27ToNad83:
				/* Initialize Vertcon, if we haven't done it already. */
				if (!dtc_ptr->xforms [ii].flag3D)
				{
					itrStat = CSvrtconInit ();
					if (itrStat != 0) break;
					dtc_ptr->xforms [ii].flag3D = TRUE;
				}

				/* Do the deltaHgt calculation while ll_wrk is still NAD27. */
				stat3D = CSvrtcon29To88 (&deltaHgt,ll_wrk);
				if (stat3D < 0)
				{
					itrStat = stat3D;
					break;
				}

				/* Now we do the horizontal conversion. */
				itrStat = CSnad27ToNad83 (ll_wrk,ll_wrk);
				if (itrStat >= 0)
				{
					if (stat3D == 0) ll_wrk [HGT] += deltaHgt;
					if (itrStat == 0) itrStat = stat3D;
				}
				break;

			case dtcTypNad83ToNad27:
				/* Initialize vertcon if we haven't done it already. */
				if (!dtc_ptr->xforms [ii].flag3D)
				{
					itrStat = CSvrtconInit ();
					if (itrStat != 0) break;
					dtc_ptr->xforms [ii].flag3D = TRUE;
				}
				/* This time, we do the vertical after the horizontal conversion,
				   We want to give Vertcon a NAD27 lat/long. */
				itrStat = CSnad83ToNad27 (ll_wrk,ll_wrk);
				if (itrStat >= 0)
				{
					stat3D = CSvrtcon29To88 (&deltaHgt,ll_wrk);
					if (stat3D == 0) ll_wrk [HGT] -= deltaHgt;
					else itrStat = stat3D;
				}
				break;

			case dtcTypAgd66ToGda94:
				itrStat = CSagd66ToGda94 (ll_wrk,ll_wrk);
				break;

			case dtcTypGda94ToAgd66:
				itrStat = CSgda94ToAgd66 (ll_wrk,ll_wrk);
				break;

			case dtcTypEd50ToEtrf89:
				itrStat = CSed50ToEtrf89 (ll_wrk,ll_wrk);
				break;

			case dtcTypEtrf89ToEd50:
				itrStat = CSetrf89ToEd50 (ll_wrk,ll_wrk);
				break;

			case dtcTypDhdnToEtrf89:
				itrStat = CSdhdnToEtrf89 (ll_wrk,ll_wrk);
				break;

			case dtcTypEtrf89ToDhdn:
				itrStat = CSetrf89ToDhdn (ll_wrk,ll_wrk);
				break;

			case dtcTypCh1903ToPlus:
				itrStat = CSch1903ToPlus (ll_wrk,ll_wrk);
				break;

			case dtcTypPlusToCh1903:
				itrStat = CSplusToCh1903 (ll_wrk,ll_wrk);
				break;

			case dtcTypAgd84ToGda94:
				itrStat = CSagd84ToGda94 (ll_wrk,ll_wrk);
				break;

			case dtcTypGda94ToAgd84:
				itrStat = CSgda94ToAgd84 (ll_wrk,ll_wrk);
				break;

			case dtcTypNzgd49ToNzgd2K:
				itrStat = CSnzgd49ToNzgd2K (ll_wrk,ll_wrk);
				break;

			case dtcTypNzgd2KToNzgd49:
				itrStat = CSnzgd2KToNzgd49 (ll_wrk,ll_wrk);
				break;

			case dtcTypAts77ToCsrs:
				itrStat = CSats77ToCsrs (ll_wrk,ll_wrk);
				break;

			case dtcTypCsrsToAts77:
				itrStat = CScsrsToAts77 (ll_wrk,ll_wrk);
				break;

			case dtcTypNad27ToAts77:
				itrStat = CSnad27ToAts77 (ll_wrk,ll_wrk);
				break;

			case dtcTypAts77ToNad27:
				itrStat = CSats77ToNad27 (ll_wrk,ll_wrk);
				break;

			case dtcTypNad83ToHarn:
				itrStat = CSnad83ToHarn (ll_wrk,ll_wrk);
				break;

			case dtcTypHarnToNad83:
				itrStat = CSharnToNad83 (ll_wrk,ll_wrk);
				break;

			case dtcTypNad83ToWgs84:
				itrStat = CSnad83ToWgs84 (ll_wrk,ll_wrk);
				break;

			case dtcTypWgs84ToNad83:
				itrStat = CSwgs84ToNad83 (ll_wrk,ll_wrk);
				break;

			case dtcTypEtrf89ToWgs84:
				itrStat = CSetrf89ToWgs84 (ll_wrk,ll_wrk);
				break;

			case dtcTypWgs84ToEtrf89:
				itrStat = CSwgs84ToEtrf89 (ll_wrk,ll_wrk);
				break;

			case dtcTypCsrsToNad83:
				itrStat = CScsrsToNad83 (ll_wrk,ll_wrk);
				break;

			case dtcTypNad83ToCsrs:
				itrStat = CSnad83ToCsrs (ll_wrk,ll_wrk);
				break;

			case dtcTypCsrsToNad27:
				itrStat = CScsrsToNad27 (ll_wrk,ll_wrk);
				break;

			case dtcTypNad27ToCsrs:
				itrStat = CSnad27ToCsrs (ll_wrk,ll_wrk);
				break;

			case dtcTypGda94ToWgs84:
				itrStat = CSgda94ToWgs84 (ll_wrk,ll_wrk);
				break;

			case dtcTypWgs84ToGda94:
				itrStat = CSwgs84ToGda94 (ll_wrk,ll_wrk);
				break;

			case dtcTypNzgd2KToWgs84:
				itrStat = CSnzgd2KToWgs84 (ll_wrk,ll_wrk);
				break;

			case dtcTypWgs84ToNzgd2K:
				itrStat = CSwgs84ToNzgd2K (ll_wrk,ll_wrk);
				break;

			case dtcTypTokyoToJgd2k:
				itrStat = CStokyoToJgd2k (ll_wrk,ll_wrk);
				break;

			case dtcTypJgd2kToTokyo:
				itrStat = CSjgd2kToTokyo (ll_wrk,ll_wrk);
				break;

			case dtcTypNtfToRgf93:
				itrStat = CSntfToRgf93 (ll_wrk,ll_wrk);
				break;

			case dtcTypRgf93ToNtf:
				itrStat = CSrgf93ToNtf (ll_wrk,ll_wrk);
				break;

			case dtcTypWgs84ToWgs72:
				itrStat = CSwgs84ToWgs72 (ll_wrk,ll_wrk);
				break;

			case dtcTypWgs72ToWgs84:
				itrStat = CSwgs72ToWgs84 (ll_wrk,ll_wrk);
				break;

			case dtcTypMolodensky:
				itrStat = CS_mo3dFowrd (ll_wrk,ll_wrk,dtc_ptr->xforms [ii].parms.molo);
				break;

			case dtcTypMolodenskyInv:
				itrStat = CS_mo3dInvrs (ll_wrk,ll_wrk,dtc_ptr->xforms [ii].parms.molo);
				break;

			case dtcTypBursaWolf:
				itrStat = CS_bw3dFowrd (ll_wrk,ll_wrk,dtc_ptr->xforms [ii].parms.bursa);
				break;

			case dtcTypBursaWolfInv:
				itrStat = CS_bw3dInvrs (ll_wrk,ll_wrk,dtc_ptr->xforms [ii].parms.bursa);
				break;

			case dtcTypDMAMulReg:
				itrStat = CS_dmaMr3dFowrd (ll_wrk,ll_wrk,dtc_ptr->xforms [ii].parms.mreg);
				break;

			case dtcTypDMAMulRegInv:
				itrStat = CS_dmaMr3dInvrs (ll_wrk,ll_wrk,dtc_ptr->xforms [ii].parms.mreg);
				break;

			case dtcTypSevenParm:
				itrStat = CS_7p3dFowrd (ll_wrk,ll_wrk,dtc_ptr->xforms [ii].parms.parm7);
				break;

			case dtcTypSevenParmInv:
				itrStat = CS_7p3dInvrs (ll_wrk,ll_wrk,dtc_ptr->xforms [ii].parms.parm7);
				break;

			case dtcTypChPlusToChtrs95:
			case dtcTypGeoCtr:
				itrStat = CS_gc3dFowrd (ll_wrk,ll_wrk,dtc_ptr->xforms [ii].parms.geoctr);
				break;

			case dtcTypChtrs95ToChPlus:
			case dtcTypGeoCtrInv: 
				itrStat = CS_gc3dInvrs (ll_wrk,ll_wrk,dtc_ptr->xforms [ii].parms.geoctr);
				break;

			case dtcTypSixParm:
				itrStat = CS_6p3dFowrd (ll_wrk,ll_wrk,dtc_ptr->xforms [ii].parms.parm6);
				break;

			case dtcTypSixParmInv:
				itrStat = CS_6p3dInvrs (ll_wrk,ll_wrk,dtc_ptr->xforms [ii].parms.parm6);
				break;

			case dtcTypFourParm:
				itrStat = CS_4p3dFowrd (ll_wrk,ll_wrk,dtc_ptr->xforms [ii].parms.parm4);
				break;

			case dtcTypFourParmInv: 
				itrStat = CS_4p3dInvrs (ll_wrk,ll_wrk,dtc_ptr->xforms [ii].parms.parm4);
				break;

			case dtcTypThreeParm:
				itrStat = CS_3p3dFowrd (ll_wrk,ll_wrk,dtc_ptr->xforms [ii].parms.parm3);
				break;

			case dtcTypThreeParmInv: 
				itrStat = CS_3p3dInvrs (ll_wrk,ll_wrk,dtc_ptr->xforms [ii].parms.parm3);
				break;

			default:
				/* We should never get here. */
				CS_stncp (csErrnam,"CS_datum:11",MAXPATH);
				CS_erpt (cs_ISER);
				status = -1;
				break;
			}

			if (csDiagnostic != 0)
			{
				fprintf (csDiagnostic,"%s[%d] %d\n",modl_name,__LINE__,itrStat);
			}

			/* See what happened.  If itrStat is negative, we have a fatal error.
			   Otherwise, we have a warning condition and we keep on chugging. */
			if (itrStat < 0)
			{
				status = itrStat;
				break;
			}
			else if (itrStat > 0 && dtc_ptr->block_err == cs_DTCFLG_BLK_F)
			{
				/* The calling application instructed us to consider soft errors
				   as hard errors.  We convert the current soft error to a
				   hard (fatal) error.  We need to derive the correct code
				   value.  Could have done that above, but that would slow
				   successful calculations. */

				status = -1;
				switch (xfrmType) {
				case dtcTypNad27ToNad83:
				case dtcTypNad83ToNad27:
					rptCode = cs_NAD_RNG_F;
					break;
				case dtcTypAgd66ToGda94:
				case dtcTypGda94ToAgd66:
					rptCode = cs_AGD66_RNG_F;
					break;
				case dtcTypAgd84ToGda94:
				case dtcTypGda94ToAgd84:
					rptCode = cs_AGD84_RNG_F;
					break;
				case dtcTypEd50ToEtrf89:
				case dtcTypEtrf89ToEd50:
					rptCode = cs_ED50_RNG_F;
					break;
				case dtcTypDhdnToEtrf89:
				case dtcTypEtrf89ToDhdn:
					rptCode = cs_DHDN_RNG_F;
					break;
				case dtcTypCh1903ToPlus:
				case dtcTypPlusToCh1903:
					rptCode = cs_CHENYX_RNG_F;
					break;
				case dtcTypNzgd49ToNzgd2K:
				case dtcTypNzgd2KToNzgd49:
					rptCode = cs_NZGD49_RNG_F;
					break;
				case dtcTypTokyoToJgd2k:
				case dtcTypJgd2kToTokyo:
					rptCode = cs_TOKYO_RNG_F;
					break;
				case dtcTypNtfToRgf93:
				case dtcTypRgf93ToNtf:
					rptCode = cs_RGF93_RNG_F;
					break;
				case dtcTypAts77ToCsrs:
				case dtcTypCsrsToAts77:
					rptCode = cs_ATS77_RNG_F;
					break;
				case dtcTypNad27ToAts77:
				case dtcTypAts77ToNad27:
					rptCode = cs_N27A77_RNG_F;
					break;
				case dtcTypNad83ToHarn:
				case dtcTypHarnToNad83:
					rptCode = cs_HARN_RNG_F;
					break;
				case dtcTypCsrsToNad83:
				case dtcTypNad83ToCsrs:
					rptCode = cs_CSRS_RNG_F;
					break;
				case dtcTypCsrsToNad27:
				case dtcTypNad27ToCsrs:
					rptCode = cs_CSRS27_RNG_F;
					break;
				case dtcTypMolodensky:
				case dtcTypMolodenskyInv:
				case dtcTypBursaWolf:
				case dtcTypBursaWolfInv:
				case dtcTypSevenParm:
				case dtcTypSevenParmInv:
				case dtcTypGeoCtr:
				case dtcTypGeoCtrInv: 
				case dtcTypSixParm:
				case dtcTypSixParmInv:
				case dtcTypFourParm:
				case dtcTypFourParmInv: 
				case dtcTypThreeParm:
				case dtcTypThreeParmInv:
				case dtcTypChtrs95ToChPlus:
				case dtcTypChPlusToChtrs95:
					rptCode = cs_XYZ_ITR;
					break;
				case dtcTypDMAMulReg:
				case dtcTypDMAMulRegInv:
					rptCode = (status == 2) ? cs_MREG_RANGE : cs_MREG_CNVRG;
					break;
				case dtcTypNad83ToWgs84:
				case dtcTypWgs84ToNad83:
				case dtcTypGda94ToWgs84:
				case dtcTypWgs84ToGda94:
				case dtcTypNzgd2KToWgs84:
				case dtcTypWgs84ToNzgd2K:
				case dtcTypWgs84ToWgs72:
				case dtcTypWgs72ToWgs84:
				case dtcTypEtrf89ToWgs84:
				case dtcTypWgs84ToEtrf89:
				case dtcTypEtrf89ToChtrs95:
				case dtcTypChtrs95ToEtrf89:
					CS_stncp (csErrnam,"CS_datum:11A",MAXPATH);
					rptCode = cs_ISER;
					break;
				default:
					CS_stncp (csErrnam,"CS_datum:12",MAXPATH);
					rptCode = cs_ISER;
					break;
				}
				CS_erpt (rptCode);
				break;
			}
			else if (itrStat > 0)
			{
				/* A soft error which is to be treated as a soft error.  We report
				   the cause of the first one only.  Of course, if a fatal comes
				   along, that's what gets reported. */
				if (status == 0)
				{
					/* Here if we have not reported an error before.  We'll
					   report this one.  Now, we figure out what to
					   report.  This could have been done above, but it
					   is faster to do it here since errors are rarer than
					   success. */
					status = itrStat;
					switch (xfrmType) {
					case dtcTypNad27ToNad83:
					case dtcTypNad83ToNad27:
						rptCode = (status == 2) ? cs_NAD_RNG_A : cs_NAD_RNG_W;
						break;
					case dtcTypAgd66ToGda94:
					case dtcTypGda94ToAgd66:
						rptCode = (status == 2) ? cs_AGD66_RNG_A : cs_AGD66_RNG_W;
						break;
					case dtcTypAgd84ToGda94:
					case dtcTypGda94ToAgd84:
						rptCode = (status == 2) ? cs_AGD84_RNG_A : cs_AGD84_RNG_W;
						break;
					case dtcTypEd50ToEtrf89:
					case dtcTypEtrf89ToEd50:
						rptCode = (status == 2) ? cs_ED50_RNG_A : cs_ED50_RNG_W;
						break;
					case dtcTypDhdnToEtrf89:
					case dtcTypEtrf89ToDhdn:
						rptCode = (status == 2) ? cs_DHDN_RNG_A : cs_DHDN_RNG_W;
						break;
					case dtcTypCh1903ToPlus:
					case dtcTypPlusToCh1903:
						rptCode = (status == 2) ? cs_CHENYX_RNG_A : cs_CHENYX_RNG_W;
						break;
					case dtcTypNzgd49ToNzgd2K:
					case dtcTypNzgd2KToNzgd49:
						rptCode = (status == 2) ? cs_NZGD49_RNG_A : cs_NZGD49_RNG_W;
						break;
					case dtcTypTokyoToJgd2k:
					case dtcTypJgd2kToTokyo:
						rptCode = (status == 2) ? cs_TOKYO_RNG_A : cs_TOKYO_RNG_W;
						break;
					case dtcTypNtfToRgf93:
					case dtcTypRgf93ToNtf:
						rptCode = (status == 2) ? cs_RGF93_RNG_A : cs_RGF93_RNG_W;
						break;
					case dtcTypAts77ToCsrs:
					case dtcTypCsrsToAts77:
						rptCode = (status == 2) ? cs_ATS77_RNG_A : cs_ATS77_RNG_W;
						break;
					case dtcTypNad27ToAts77:
					case dtcTypAts77ToNad27:
						rptCode = (status == 2) ? cs_N27A77_RNG_A : cs_N27A77_RNG_W;
						break;
					case dtcTypNad83ToHarn:
					case dtcTypHarnToNad83:
						rptCode = (status == 2) ? cs_HARN_RNG_A : cs_HARN_RNG_W;
						break;
					case dtcTypCsrsToNad83:
					case dtcTypNad83ToCsrs:
						rptCode = (status == 2) ? cs_CSRS_RNG_A : cs_CSRS_RNG_W;
						break;
					case dtcTypCsrsToNad27:
					case dtcTypNad27ToCsrs:
						rptCode = (status == 2) ? cs_CSRS27_RNG_A : cs_CSRS27_RNG_W;
						break;
					case dtcTypMolodensky:
					case dtcTypMolodenskyInv:
					case dtcTypBursaWolf:
					case dtcTypBursaWolfInv:
					case dtcTypSevenParm:
					case dtcTypSevenParmInv:
					case dtcTypGeoCtr:
					case dtcTypGeoCtrInv: 
					case dtcTypSixParm:
					case dtcTypSixParmInv:
					case dtcTypFourParm:
					case dtcTypFourParmInv: 
					case dtcTypThreeParm:
					case dtcTypThreeParmInv: 
					case dtcTypChtrs95ToChPlus:
					case dtcTypChPlusToChtrs95:
						rptCode = cs_XYZ_ITR;
						status = -1;
						break;
					case dtcTypDMAMulReg:
					case dtcTypDMAMulRegInv:
						rptCode = (status == 2) ? cs_MREG_RANGE : cs_MREG_CNVRG;
						break;
					case dtcTypNad83ToWgs84:
					case dtcTypWgs84ToNad83:
					case dtcTypGda94ToWgs84:
					case dtcTypWgs84ToGda94:
					case dtcTypNzgd2KToWgs84:
					case dtcTypWgs84ToNzgd2K:
					case dtcTypWgs84ToWgs72:
					case dtcTypWgs72ToWgs84:
					case dtcTypEtrf89ToWgs84:
					case dtcTypWgs84ToEtrf89:
					case dtcTypEtrf89ToChtrs95:
					case dtcTypChtrs95ToEtrf89:
						CS_stncp (csErrnam,"CS_datum:12A",MAXPATH);
						CS_erpt (cs_ISER);
						status = -1;
						break;
					default:
						CS_stncp (csErrnam,"CS_datum:13",MAXPATH);
						CS_erpt (cs_ISER);
						status = -1;
						break;
					}
					/* We leave the result of all this in rptCode and
					   status.  There are more complications to deal with,
					   as coded below. */
				}
			}
			/* Break if we ended up with a fatal status. */
			if (status < 0) break;
		}
		/* Analyze the results.  In the event of any error, the cause will]
		   have already been reported to CS_erpt.  All we need do here is
		   decide on the appropriate status value to return. */
		wasInListFlag = FALSE;	
		if (status > 0)
		{
			/* Here if we had a block error of some sort. Issue the
			   appropriate message per the users instructions. */
			csErrlng = (int)((fabs (ll_in [LNG]) >= 10000.0) ? 9999.99 : ll_in [LNG]);
			if (ll_in [LNG] < 0.0) csErrlng = -csErrlng;
			csErrlat = (int)((fabs (ll_in [LAT]) >= 10000.0) ? 9999.99 : ll_in [LAT]);
			if (ll_in [LAT] < 0.0) csErrlat = -csErrlat;
			if (dtc_ptr->listCount < 10)
			{
				for (ii = 0;ii < 10;ii += 1)
				{
					if (dtc_ptr->errLngLat [ii][LNG] == csErrlng &&
					    dtc_ptr->errLngLat [ii][LAT] == csErrlat)
					{
						/* It's already there, nothing to do.  Save this
						   information for later so we can decide if to
						   report this again, */
						wasInListFlag = TRUE;
						break;
					}
				}
				if (ii >= 10)
				{
					/* It's not there, and we know there's room. */
					dtc_ptr->errLngLat [dtc_ptr->listCount][LNG] = (short)csErrlng;
					dtc_ptr->errLngLat [dtc_ptr->listCount][LAT] = (short)csErrlat;
					dtc_ptr->listCount += 1;
				}
			}

			/* See how the user wants this type of error handled. */
			switch (dtc_ptr->block_err) {
			case cs_DTCFLG_BLK_1:
			case cs_DTCFLG_BLK_10:
				/* User wants the error reported once, per block, but only once. */
				if (wasInListFlag)
				{
					status = 0;
				}
				else
				{
					if (dtc_ptr->listCount < 10)
					{
						CS_erpt (rptCode);
						/* Return the same value we have historically returned. */
						status = 1;
					}
					else if (dtc_ptr->block_err == cs_DTCFLG_BLK_10)
					{
						if (dtc_ptr->listCount == 10)
						{
							CS_erpt (cs_DTC_SOFTIGNR);
							status = 1;
							dtc_ptr->listCount += 1;
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
				CS_stncp (csErrnam,"CS_datum:14",MAXPATH);
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

	/* The conversion is complete.  Return to the user. */
	if (csDiagnostic != 0)
	{
		fprintf (csDiagnostic,"%s[%d] %d\n",modl_name,__LINE__,status);
	}
	return status;
}
/**********************************************************************
**	status = CSwgs72ToWgs84 (ll_84,ll_72);
**
**	double ll_84 [2];			the WGS 84 longitude and latitude, in degrees,
**								are resturned here.
**	double ll_72 [2];			the WGS 72 longitude and latitude, in degrees,
**								which are to be converted.
**	int status;					returns 0 indicating that no error
**								occurred.
**
**	ll_84 and ll_72 may point to the same array.
**
**********************************************************************/
static const double wgs72Erad = 6378135.000;
static const double wgs72deltaF = 0.3121057E-07;

int EXP_LVL9 CSwgs72ToWgs84 (double ll_84 [3],Const double ll_72 [3])
{
	extern double cs_Two;				/* 2.0 */
	extern double cs_Degree;			/* 1.0 / 57.2... */
	extern double cs_Sin1Sec;			/* sine of 1 second of arc. */
	extern double cs_Sec2Deg;			/* 1/3600 */

	double lat;
	double del_lat;
	double del_hgt;
	double sin_lat;
	double cos_lat;
	double sin_2_lat;

	lat = ll_72 [LAT] * cs_Degree;
	sin_lat = sin (lat);
	cos_lat = cos (lat);
	sin_2_lat = sin (cs_Two * lat);

	del_lat = ((4.5 * cos_lat) / (wgs72Erad * cs_Sin1Sec)) +
					((wgs72deltaF * sin_2_lat) / cs_Sin1Sec);
	del_hgt = (4.5 * sin_lat) +
			  (wgs72Erad * wgs72deltaF * sin_lat * sin_lat) +
			  1.4 - 2.0;

	ll_84 [LNG] = ll_72 [LNG] + (0.554 / 3600.00);
	ll_84 [LAT] = ll_72 [LAT] + (del_lat * cs_Sec2Deg);
	ll_84 [HGT] = ll_72 [HGT] + del_hgt;

	return 0;
}

/**********************************************************************
**	st = CSwgs84ToWgs72 (ll_72,ll_84);
**
**	double ll_72 [3];			the WGS-72 latitude and longitude, in degrees,
**								which is to be converted.  Height must be
**								in meters, as ellipsoid is not provided.
**	double ll_84 [3];			the WGS-84 latitude and longitude, in degress,
**								which is to be converted.  Height is always
**								in meters.
**	int st;						always returns 0.
**********************************************************************/
int EXP_LVL9 CSwgs84ToWgs72 (double ll_72 [3],Const double ll_84 [3])
{
	extern double cs_Two;
	extern double cs_Degree;
	extern double cs_Sin1Sec;
	extern double cs_Sec2Deg;

	double lat;
	double del_lat;
	double del_hgt;
	double sin_lat;
	double cos_lat;
	double sin_2_lat;

	lat = ll_84 [LAT] * cs_Degree;

	sin_lat = sin (lat);
	cos_lat = cos (lat);
	sin_2_lat = sin (cs_Two * lat);

	/* Probably should do a real inverse on the latitude, but the
	   following works rather precisely (good to 0.00000005 seconds). */

	del_lat = ((4.5 * cos_lat) / (wgs72Erad * cs_Sin1Sec)) +
					((wgs72deltaF * sin_2_lat) / cs_Sin1Sec);
	del_hgt = (4.5 * sin_lat) +
				(wgs72Erad * wgs72deltaF * sin_lat * sin_lat) +
				1.4 - 2.0;

	ll_72 [LNG] = ll_84 [LNG] - (0.554 / 3600.00);
	ll_72 [LAT] = ll_84 [LAT] - (del_lat * cs_Sec2Deg);
	ll_72 [HGT] = ll_84 [HGT] - del_hgt;
	return 0;
}
/**********************************************************************
**	st = CSwgs84ToNad83 (ll_83,ll_84);
**
**	double ll_84 [2];			the latitude and longitude, in degrees, which
**								is to be converted.
**	double ll_83 [2];			the resulting latitude and longitude are
**								returned here.
**	int st;						returns 0, cuurently.
**
**	At the current time, this function does nothing except
**	copy ll_84 to ll_83.  There is much debate as to whether
**	there truly is a difference between NAD-83 and WGS-84.
**	They both are, after all, very precise measurements of the
**	same thing.  The best I can determine, NAD-83 and WGS-84
**	are indeed the same, but the benchmarks used for NAD-83
**	have been determined using more data and thus a smaller
**	error factor.
**
**	Since there are differences of up to two meters between
**	these two datums at the WGS-84 benchmarks, we have this
**	function as a hook to add a conversion from WGS-84 to
**	NAD-83 should this prove to be economically advantageous
**	at some point in the future.
**********************************************************************/
int EXP_LVL9 CSwgs84ToNad83 (double ll_83 [3],Const double ll_84 [3])
{
	ll_83 [LNG] = ll_84 [LNG];
	ll_83 [LAT] = ll_84 [LAT];
	ll_83 [HGT] = ll_84 [HGT];
	return 0;
}

/**********************************************************************
**	st = CSnad83ToWgs84 (ll_84,ll_83);
**
**	double ll_84 [2];			the resulting latitude and longitude are
**								returned here.
**	double ll_83 [2];			the latitude and longitude, in degrees, which
**								is to be converted.
**	int st;						returns 0.
**
**	See CSwgs84ToNad83 for some remarks about this function.
**********************************************************************/
int EXP_LVL9 CSnad83ToWgs84 (double ll_84 [3],Const double ll_83 [3])
{
	ll_84 [LNG] = ll_83 [LNG];
	ll_84 [LAT] = ll_83 [LAT];
	ll_84 [HGT] = ll_83 [HGT];
	return (FALSE);
}
/**********************************************************************
**	st = CSwgs84ToNzgd2K (ll_nzgd2K,ll_84);
**
**	double ll_84 [2];			the latitude and longitude, in degrees, which
**								is to be converted.
**	double ll_nzgd2K [2];		the resulting latitude and longitude are
**								returned here.
**	int st;						returns 0, cuurently.
**
**	At the current time, this function does nothing except
**	copy ll_84 to ll_nzgd2K.
**
**	Since there are differences of up to two meters between
**	these two datums at the WGS-84 benchmarks, we have this
**	function as a hook to add a conversion from WGS-84 to
**	NZGD2K should this prove to be economically advantageous
**	at some point in the future.
**********************************************************************/
int EXP_LVL9 CSwgs84ToNzgd2K (double ll_nzgd2K [3],Const double ll_84 [3])
{
	ll_nzgd2K [LNG] = ll_84 [LNG];
	ll_nzgd2K [LAT] = ll_84 [LAT];
	ll_nzgd2K [HGT] = ll_84 [HGT];
	return 0;
}
/**********************************************************************
**	st = CSnzgd2KToWgs84 (ll_84,ll_83);
**
**	double ll_84 [2];			the resulting latitude and longitude are
**								returned here.
**	double ll_nzgd2K [2];		the latitude and longitude, in degrees, which
**								is to be converted.
**	int st;						returns 0.
**
**	See CSwgs84ToNzgd2K for some remarks about this function.
**********************************************************************/
int EXP_LVL9 CSnzgd2KToWgs84 (double ll_84 [3],Const double ll_nzgd2K [3])
{
	ll_84 [LNG] = ll_nzgd2K [LNG];
	ll_84 [LAT] = ll_nzgd2K [LAT];
	ll_84 [HGT] = ll_nzgd2K [HGT];
	return (FALSE);
}
