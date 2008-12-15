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

	Both forward and inverse calculations are coded for each transformation
	technique.  An initialize function is also included.  Each of these
	functions initialize a parameter structure given a cs_Datum_ structure
	for the source datum and the target datum.  These initialize functions
	do, essentially, what a constructor would do in the C++ environment.

	Be careful, and note that the initialization functions assume two
	definitions which use the same type of transformations.  This is done
	to support the very small number (one as I write) of datum trasnformations
	which bypass WGS84 and go direcrtly from one datum to another.  Note the
	the DMA Mreg initialize function has a target datum of WGS84 hard coded
	into it.
	
	Generalized calculation of geocentric cartesian coordinates,
	including the inverse, are also included.
*/
/*lint -e788 */		/* enumeration value is not included in a switch */

#include "cs_map.h"

/******************************************************************************
	CS_llhToXyz (xyz [3],llh [3],e_rad,e_sq);

	double xyz [3];				the computed results are returned here.  Units
								are the same as the equatorial radius. Results
								are relative to the center of the earth.
	double llh [3];				the longitude, latitude, and ellipsoid height
								of the point to be converted.  Height must be
								in same units as the equatorial radius.
	double e_rad;				equatorial radius of the ellipsoid/shpere.
								Units ar eusually meters, but this is not
								required.
	double e_sq;				eccentricity squared of the ellipsoid.  Use
								zero for a sphere.

	The resulting coordinates are sometimes referred to as Helmert coordinates.
*/
void EXP_LVL5 CS_llhToXyz (double xyz [3],Const double llh [3],double e_rad,double e_sq)
{
	extern double cs_One;				/* 1.0 */
	extern double cs_Degree;			/* 0.017.... */

	double lng, lat, hhh;
	double vv;
	double cos_lat, sin_lat;

	lng = llh [LNG] * cs_Degree;
	lat = llh [LAT] * cs_Degree;
	hhh = llh [HGT];

	/* Compute the Helmert coordinates of the point. */
	cos_lat = cos (lat);
	sin_lat = sin (lat);
	vv = e_rad / sqrt (cs_One - (e_sq * sin_lat * sin_lat));
	xyz [XX] = (vv + hhh) * cos_lat * cos (lng);
	xyz [YY] = (vv + hhh) * cos_lat * sin (lng);
	xyz [ZZ] = (vv * (cs_One - e_sq) + hhh) * sin_lat;

	return;
}

/******************************************************************************
	status = CS_llhToXyz (xyz [3],llh [3],e_rad,e_sq);

	double llh [3];				the longitude, latitude, and ellipsoid height
								of the point are returned here.  Ellipsoid
								height is in the same units as the equatorial
								radius.
	double xyz [3];				the point to be converted.  Units must be the
								same as the equatorial radius provided.
	double e_rad;				equatorial radius of the ellipsoid/shpere.
								Units of this parameter and xyz must be the
								same.
	double e_sq;				eccentricity squared of the ellipsoid.  Use
								zero for a sphere.
	int status;					returns zero on success, -1 on failure. Only
								possible cause of failure is the failure of
								iterative solution to converge.  This is not
								possible if the arguments are reasonable.

	The source coordinates are sometimes referred to as Helmert coordinates.
*/
int EXP_LVL5 CS_xyzToLlh (double llh [3],Const double xyz [3],double e_rad,double e_sq)
{
	extern double cs_One;				/* 1.0 */
	extern double cs_Radian;			/* 57.29... */

	int ii;
	int status;
	static int maxIteration = 9;

	double sinLat;
	double lat, prevLat;
	double vv,denom;
	static double smallLat = 5.0E-12;

	status = 0;

	/* Longitude is easy. */
	llh [LNG] = atan2 (xyz [YY],xyz [XX]) * cs_Radian;

	/* Latitude is a bit more difficult. An iterative process is required.
	   We need a basic constant and an approximate latitude to start with.
	   We start with the latitude if the ellipsoid was a sphere.
	   
	   This initial guess is important for performance reasons.  I.e. the
	   closer we can get to the real thing, the fewer iterations of the
	   loop below are required. */
	denom = sqrt (xyz [XX] * xyz [XX] + xyz [YY] * xyz [YY]);
	lat = atan (xyz [ZZ] / denom);

	/* Iterate to the final solution. */
	for (ii = 0;ii < maxIteration;ii++)
	{
		prevLat = lat;
		sinLat = sin (lat);
		vv = e_rad / sqrt (cs_One - (e_sq * sinLat * sinLat));
		lat = (xyz [ZZ] + (e_sq * vv * sinLat)) / denom;
		lat = atan (lat);
		if (fabs (lat - prevLat) < smallLat) break;
	}
	if (ii >= maxIteration)
	{
		CS_erpt (cs_XYZ_ITR);
		status = -1;
	}
	llh [LAT] = lat * cs_Radian;

	/* Now for the new elevation above the ellipsoid.  There are
	   two different formulas.  It was suggested by the source
	   of all of this that the second be used at high latitudes.
	   They didn't say what they consider high.  We will assume
	   a radian value of one is high.  I believe we just need
	   to stay away from the indeterminant cases where the
	   sine and cosine go to zero since we are dividing by
	   them here (i.e. secant and cosecant). */
	sinLat = sin (lat);
	vv = e_rad / sqrt (cs_One - (e_sq * sinLat * sinLat));
	if (fabs (lat) <= cs_One)
	{
		llh [HGT] = (denom / cos (lat)) - vv;
	}
	else
	{
		llh [HGT] = (xyz [ZZ] / sinLat) - ((cs_One - e_sq) * vv);
	}

	/* We return the calculated value even if the returned status is not
	   zero.  Much of the rest of this module relies on this behavior. */
	return status;
}
/******************************************************************************
	bursa = CS_bwInit (srcDatum,trgDatum);

	struct cs_Datum_ *srcDatum;	pointer to the definition of the source datum.
	struct cs_Datum_ *trgDatum; pointer to the definition of the target datum.
	struct cs_Bursa_ *bursa;	returns a pointer to an initialized parameter
								structure which is the required argument for
								CS_bwForwd and CS_bwInvrs.

	The function returns null in the event of an error. Memory allocation
	failure is the principal cause of failure.

	What we call the Bursa/Wolfe tranformation is an approximation of the
	Seven Parameter transformation.  The following approximations apply:

	1> The cosine of all rotation angels is one.
	2> The sine of all rotation angles is equal to the angle itself.
	3> The product of two sine terms is zero.
*/
struct cs_Bursa_ * EXP_LVL9 CS_bwInit (Const struct cs_Datum_* srcDatum,
									   Const struct cs_Datum_* trgDatum)
{
	extern double cs_One;				/* 1.0 */
	extern double cs_Sec2Rad;			/* 4.848...E-06 */

	double trgScale;
	struct cs_Bursa_ *bursa;

	bursa = NULL;
	bursa = (struct cs_Bursa_ *)CS_malc (sizeof (struct cs_Bursa_));
	if (bursa == NULL)
	{
		CS_erpt (cs_NO_MEM);
		return bursa;
	}

	bursa->srcERad = srcDatum->e_rad;
	bursa->srcESqr = srcDatum->ecent * srcDatum->ecent;
	CS_stncp (bursa->srcKeyName,srcDatum->key_nm,sizeof (bursa->srcKeyName));

	bursa->trgERad = trgDatum->e_rad;
	bursa->trgESqr = trgDatum->ecent * trgDatum->ecent;
	CS_stncp (bursa->trgKeyName,trgDatum->key_nm,sizeof (bursa->trgKeyName));

	bursa->delta_X = srcDatum->delta_X - trgDatum->delta_X;
	bursa->delta_Y = srcDatum->delta_Y - trgDatum->delta_Y;
	bursa->delta_Z = srcDatum->delta_Z - trgDatum->delta_Z;
	bursa->rot_X = (srcDatum->rot_X - trgDatum->rot_X) * cs_Sec2Rad;
	bursa->rot_Y = (srcDatum->rot_Y - trgDatum->rot_Y) * cs_Sec2Rad;
	bursa->rot_Z = (srcDatum->rot_Z - trgDatum->rot_Z) * cs_Sec2Rad;

	trgScale = cs_One + (trgDatum->bwscale * 1.0E-06);
	if (fabs (trgScale) < 1.0E-06)
	{
		// An actual scale factor of zero would produce a floating point
		// exception.  Can't allow that.
		CS_free (bursa);
		bursa = NULL;
		CS_erpt (cs_ISER);
	}
	else
	{
		bursa->scale = (cs_One + (srcDatum->bwscale * 1.0E-06)) / trgScale;
	}
	return bursa;
}

/******************************************************************************
	status = CS_bw3dForwd (trgLl [3],srcLl [3],bursa)

	double trgLl [3];			the converted results are returned here.
								Lng/Lat are in degrees, Hgt is usually meters.
								Hgt is considered to be ellipsoid height, and
								must be in the units of the equatorial radius
								in the bursa structure, usually meters.
	double srcLl [3];			the coordinate to be converted.  Elements have
								the same characteristics as described above.
	struct cs_Bursa_ *bursa;	structure which contains the parameters for
								the Bursa calculation.
	int status;					normally returns zero.  Will return -1 if the
								geocentric inverse calculation fails; which is
								not possible if the parameters are reasonalble.

	This is an approximate Seven Parameter Transformation.  See CS_bwInit for
	more information.
*/
int EXP_LVL9 CS_bw3dFowrd (double trgLl [3],Const double srcLl [3],Const struct cs_Bursa_ *bursa)
{
	int status;

	double xx, yy, zz;
	double xyz [3];

	/* Convert the geographic coordinates to geocentric XYZ coordinates. */
	CS_llhToXyz (xyz,srcLl,bursa->srcERad,bursa->srcESqr);

	/* Adjust these cartesian coordinates via the Bursa/Wolf transformation.
	   First, we apply the rotation matrix in a computationally effieicent
	   form.  That is, several elements of the matrix are assumed to have
	   fixed values. */
	xx =  xyz [XX] - bursa->rot_Z * xyz [YY] + bursa->rot_Y * xyz [ZZ];
	yy =  bursa->rot_Z * xyz [XX] + xyz [YY] - bursa->rot_X * xyz [ZZ];
	zz = -bursa->rot_Y * xyz [XX] + bursa->rot_X * xyz [YY] + xyz [ZZ];

	/* Apply the scale and translation. */
	xyz [XX] = bursa->scale * xx + bursa->delta_X;
	xyz [YY] = bursa->scale * yy + bursa->delta_Y;
	xyz [ZZ] = bursa->scale * zz + bursa->delta_Z;

	/* Convert the new X, Y, and Z back to latitude and longitude.
	   CS_xyzToLlh returns degrees. */
	status = CS_xyzToLlh (trgLl,xyz,bursa->trgERad,bursa->trgESqr);

	/* That's that. */
	return status;
}

/******************************************************************************
	status = CS_bw2dForwd (trgLl [3],srcLl [3],bursa)

	double trgLl [3];			the converted results are returned here.
								Lng/Lat are in degrees, Hgt is copied from
								srcLL.
	double srcLl [3];			the coordinate to be converted.  Elements have
								the same characteristics as described above.
								The Hgt element is not involved in the
								calculations, an ellipsoidal height of zero is
								assumed.
	struct cs_Bursa_ *bursa;	structure which contains the parameters for
								the Bursa calculation.
	int status;					normally returns zero.  Will return -1 if the
								geocentric inverse calculation fails; which is
								not possible if the parameters are reasonable
								geographic coordinates.

	See CS_bw3dFowrd for more information.
*/
int EXP_LVL9 CS_bw2dFowrd (double trgLl [3],Const double srcLl [3],Const struct cs_Bursa_ *bursa)
{
	extern double cs_Zero;				/* 0.0 */

	int status;
	double lcl_ll [3];

	trgLl [LNG] = lcl_ll [LNG] = srcLl [LNG];
	trgLl [LAT] = lcl_ll [LAT] = srcLl [LAT];
	trgLl [HGT] = srcLl [HGT];

	lcl_ll [HGT] = cs_Zero;
	status = CS_bw3dFowrd (lcl_ll,lcl_ll,bursa);
	if (status >= 0)
	{
		trgLl [LNG] = lcl_ll [LNG];
		trgLl [LAT] = lcl_ll [LAT];
	}
	return status;
}

/******************************************************************************
	status = CS_bw3dInvrs (trgLl [3],srcLl [3],bursa)

	double trgLl [3];			the converted results are returned here.
								Lng/Lat are in degrees, Hgt is usually meters.
								Hgt is considered to be ellipsoid height, and
								must be in the units of the equatorial radius
								in the bursa structure, usually meters.
	double srcLl [3];			the coordinate to be converted.  Elements have
								the same characteristics as described above.
	struct cs_Bursa_ *bursa;	structure which contains the parameters for
								the Bursa calculation.
	int status;					normally returns zero.  Will return -1 if the
								geocentric inverse calculation fails; which is
								not possible if the parameters are reasonalble.

	This is the inverse of CS_bw3dForwrd, see that function for more information.
	Note, that when taken alone, the rotations form an orthogonal matrix.  The
	inverse of an orthogonal matrix is the transpose.  This is what is coded
	below.
*/
int EXP_LVL9 CS_bw3dInvrs (double trgLl [3],Const double srcLl [3],Const struct cs_Bursa_ *bursa)
{
	int status;

	double xx, yy, zz;
	double xyz [3];

	/* Convert the geographic coordinates to geocentric XYZ coordinates. */
	CS_llhToXyz (xyz,srcLl,bursa->trgERad,bursa->trgESqr);

	/* Invert the scaling and translation. */
	xx = (xyz [XX] - bursa->delta_X) / bursa->scale;
	yy = (xyz [YY] - bursa->delta_Y) / bursa->scale;
	zz = (xyz [ZZ] - bursa->delta_Z) / bursa->scale;

	/* Apply the inverse of the rotation matrix.  Essentially, this code
	   is equivalent to the multiplication by the tranpose of the original
	   rotation matrix. */
	xyz [XX] =  xx + bursa->rot_Z * yy - bursa->rot_Y * zz;
	xyz [YY] = -bursa->rot_Z * xx + yy + bursa->rot_X * zz;
	xyz [ZZ] =  bursa->rot_Y * xx - bursa->rot_X * yy + zz;

	/* Convert the new X, Y, and Z back to latitude and longitude. */
	status = CS_xyzToLlh (trgLl,xyz,bursa->srcERad,bursa->srcESqr);

	return status;
}

/******************************************************************************
	status = CS_bw2dInvrs (trgLl [3],srcLl [3],bursa)

	double trgLl [3];			the converted results are returned here.
								Lng/Lat are in degrees, Hgt element is
								copied from srcLl.
	double srcLl [3];			the coordinate to be converted.  Elements have
								the same characteristics as described above;
								Hgt element is not involved in the calculation,
								an ellipsoidal height of zero is assumed.
	struct cs_Bursa_ *bursa;	structure which contains the parameters for
								the Bursa calculation.
	int status;					normally returns zero.  Will return -1 if the
								geocentric inverse calculation fails; which is
								not possible if the parameters are reasonalble.
								Will return +1 if the iterative inverse of
								CS_bw2dFowrd fails to converge.

	This is an iterative exercise of CS_bw2dFowrd.  Thus, this calculation
	will produce a rather precise inverse of geographic coordinates.  Used in
	the two dimensional case as we don't know what the ellipsoid elevation is.
	Thus, we can't precisely compute the inverse of a coordinate we might
	have computed the forward for, and the using application threw away the
	Z.  The end result of all this can be positional creep unless we do
	as coded below.
*/
int EXP_LVL9 CS_bw2dInvrs (double trgLl [3],Const double srcLl [3],Const struct cs_Bursa_ *bursa)
{
	static int maxIteration = 8;
	static double smallValue  = 1.0E-09;		/* equates to =~ .1 millimeters */
	static double smallValue2 = 1.0E-06;		/* equates to =~ 100 millimeters */

	int ii;
	int lngOk;
	int latOk;
	int rtnVal;

	double guess [3];
	double newLl [3];
	double epsilon [3];

	/* Assume everything goes OK until we know different. */
	rtnVal = 0;

	/* First, we copy the WGS-84 lat/longs to the local array.  This is the
	   default result which the user may want in the event of a fatal error.
	   Note, we assume such has been done below, even if there has not been
	   an error. */
	trgLl [LNG] = guess [LNG] = srcLl [LNG];
	trgLl [LAT] = guess [LAT] = srcLl [LAT];
	trgLl [HGT] = srcLl [HGT];

	/* To improve performancce, we could use CS_bw3dInvrs here to
	   get our guess.  That function will probably reduce the
	   number of iterations require below substantially. */

	/* Start a loop which will iterate as many as maxIteration times. */
	for (ii = 0;ii < maxIteration;ii++)
	{
		/* Assume we are done until we know different. */
		lngOk = latOk = TRUE;

		/* Compute the WGS-84 lat/long for our current guess. */
		rtnVal = CS_bw2dFowrd (newLl,guess,bursa);
		if (rtnVal != 0)
		{
			/* Oopps!! We must have been given some pretty strange
			   coordinate values. */
			break;
		}

		/* See how far we are off. */
		epsilon [LNG] = srcLl [LNG] - newLl [LNG];
		epsilon [LAT] = srcLl [LAT] - newLl [LAT];

		/* If our guess at the longitude is off by more than
		   small, we adjust our guess by the amount we are off. */
		if (fabs (epsilon [LNG]) > smallValue)
		{
			lngOk = FALSE;
			guess [LNG] += epsilon [LNG];
		}
		/* If our guess longitude is off by more than
		   small, we adjust our guess by the amount we are off. */
		if (fabs (epsilon [LAT]) > smallValue)
		{
			latOk = FALSE;
			guess [LAT] += epsilon [LAT];
		}

		/* If our current guess produces a newLl that is within
		   samllValue of srcLl, we are done. */
		if (lngOk && latOk) break;
	}

	/* If we didn't resolve in maxIteration tries, we issue a warning
	   message.  Usually, three or four iterations does the trick. */
	if (ii >= maxIteration )
	{
		CS_erpt (cs_BW_CNVRG);

		/* Issue a warning if we're close, a fatal if we are still way off.
		   In any case, we return the last computed value.  We could have
		   gotten very fancy with this stuff, but it would have had serious
		   affects on the performance.  So we just check epsilon here as
		   we know we have an error and this doesn't happen very often. */
		rtnVal = 1;
		if (fabs (epsilon [LNG]) > smallValue2 ||
		    fabs (epsilon [LAT]) > smallValue2)
		{
			rtnVal = -1;
		}
	}

	/* If we are not returning fatal error status, we return the last
	   computed value even if we are returning a warning status.  It may be
	   of value, often is. */
	if (rtnVal >= 0)
	{
		trgLl [LNG] = guess [LNG];
		trgLl [LAT] = guess [LAT];
	}
	return rtnVal;
}

/******************************************************************************
	parm7 = CS_7pInit (srcDatum,trgDatum);

	struct cs_Datum_ *srcDatum;	pointer to the definition of the source datum.
	struct cs_Datum_ *trgDatuu; pointer to the definition of the target datum.
	struct cs_Parm7_ *parm7;	returns a pointer to an initialized parameter
								structure which is the required argument for
								CS_p7Forwd and CS_p7Invrs.

	The function returns null in the event of an error. Memory allocation
	failure is the principal cause of failure.
*/
struct cs_Parm7_ * EXP_LVL9 CS_7pInit (Const struct cs_Datum_* srcDatum,
									   Const struct cs_Datum_* trgDatum)
{
	extern double cs_One;
	extern double cs_Sec2Rad;			/* 4.848136E-06 */

	struct cs_Parm7_ *parm7;

	double trgScale;
	double sinAlpha, cosAlpha;
	double sinBeta, cosBeta;
	double sinGamma, cosGamma;

	parm7 = (struct cs_Parm7_ *)CS_malc (sizeof (struct cs_Parm7_));
	if (parm7 == NULL)
	{
		CS_erpt (cs_NO_MEM);
	}
	else
	{
		parm7->srcERad = srcDatum->e_rad;
		parm7->srcESqr = srcDatum->ecent * srcDatum->ecent;
		CS_stncp (parm7->srcKeyName,srcDatum->key_nm,sizeof (parm7->srcKeyName));

		parm7->trgERad = trgDatum->e_rad;
		parm7->trgESqr = trgDatum->ecent * trgDatum->ecent;
		CS_stncp (parm7->trgKeyName,trgDatum->key_nm,sizeof (parm7->trgKeyName));

		parm7->delta_X = srcDatum->delta_X - trgDatum->delta_X;
		parm7->delta_Y = srcDatum->delta_Y - trgDatum->delta_Y;
		parm7->delta_Z = srcDatum->delta_Z - trgDatum->delta_Z;

		sinAlpha = sin ((srcDatum->rot_X - trgDatum->rot_X) * cs_Sec2Rad);
		cosAlpha = cos ((srcDatum->rot_X - trgDatum->rot_X) * cs_Sec2Rad);
		sinBeta  = sin ((srcDatum->rot_Y - trgDatum->rot_Y) * cs_Sec2Rad);
		cosBeta  = cos ((srcDatum->rot_Y - trgDatum->rot_Y) * cs_Sec2Rad);
		sinGamma = sin ((srcDatum->rot_Z - trgDatum->rot_Z) * cs_Sec2Rad);
		cosGamma = cos ((srcDatum->rot_Z - trgDatum->rot_Z) * cs_Sec2Rad);

		/* view the following as the development of a 3x3 orthogonal
		   rotation matirx: rt12 = rotate matrix row 1 column 2 */
		parm7->rt11 =  cosBeta * cosGamma;
		parm7->rt21 = -cosBeta * sinGamma;
		parm7->rt31 =  sinBeta;
		parm7->rt12 =  cosAlpha * sinGamma + sinAlpha * sinBeta * cosGamma;
		parm7->rt22 =  cosAlpha * cosGamma - sinAlpha * sinBeta * sinGamma;
		parm7->rt32 = -sinAlpha * cosBeta;
		parm7->rt13 =  sinAlpha * sinGamma - cosAlpha * sinBeta * cosGamma;
		parm7->rt23 =  sinAlpha * cosGamma + cosAlpha * sinBeta * sinGamma;
		parm7->rt33 =  cosAlpha * cosBeta;

		trgScale = cs_One + (trgDatum->bwscale * 1.0E-06);
		if (fabs (trgScale) < 1.0E-06)
		{
			CS_free (parm7);
			parm7 = NULL;
			CS_erpt (cs_ISER);
		}
		else
		{
			parm7->scale = (cs_One + (srcDatum->bwscale * 1.0E-06)) / trgScale;
		}
	}
	return parm7;
}

/**********************************************************************
	st = CS_7p3dFowrd (trgLl,srcLl,parm7);

	double trgLl [3];			the converted results are returned here.
								Lng/Lat are in degrees, Hgt is usually meters.
								Hgt is considered to be ellipsoid height, and
								must be in the units of the equatorial radius
								in the parm7 structure.
	double srcLl [3];			the coordinate to be converted.  Elements have
								the same characteristics as described above.
	struct cs_Parm7_ *parm7;	pointer to an initialized structure defining
								the datum conversion to be performed.
	int st;						returns zero to indicate success, -1 on
								failure.  Only cause for failure is the
								convergence failure of geocentric inverse.

	This is a rigorous seven parameter transformation, while the Bursa/Wolf
	transformation described above.  There are no approximations in this
	transformation.
*/
int EXP_LVL9 CS_7p3dFowrd (double trgLl [3],Const double srcLl [3],Const struct cs_Parm7_ *parm7)
{
	int status;

	double xx, yy, zz;
	double xyz [3];

	/* Convert the geographic coordinates to geocentric XYZ coordinates. */
	CS_llhToXyz (xyz,srcLl,parm7->srcERad,parm7->srcESqr);

	/* Adjust these cartesian coordinates via the Bursa/Wolf transformation.
	   First we apply the rotation.  This is, essentially, a matrix multiplcation. */
	xx = parm7->rt11 * xyz [XX] + parm7->rt12 * xyz [YY] + parm7->rt13 * xyz [ZZ];
	yy = parm7->rt21 * xyz [XX] + parm7->rt22 * xyz [YY] + parm7->rt23 * xyz [ZZ];
	zz = parm7->rt31 * xyz [XX] + parm7->rt32 * xyz [YY] + parm7->rt33 * xyz [ZZ];

	/* Apply the scale and translation factors. */
	xyz [XX] = parm7->scale * xx + parm7->delta_X;
	xyz [YY] = parm7->scale * yy + parm7->delta_Y;
	xyz [ZZ] = parm7->scale * zz + parm7->delta_Z;

	/* Convert the new X, Y, and Z back to latitude and longitude.
	   CS_xyzToLlh returns degrees. */
	status = CS_xyzToLlh (trgLl,xyz,parm7->trgERad,parm7->trgESqr);

	/* That's that. */
	return status;
}

/**********************************************************************
	st = CS_7p2dFowrd (trgLl,srcLl,parm7);

	double trgLl [3];			the converted results are returned here.
								Lng/Lat are in degrees, Hgt element is
								copied from srcLl parameter.
	double srcLl [3];			the coordinate to be converted.  Elements have
								the same characteristics as described above.
								Hgt element is not involved in the caluclation.
	struct cs_Parm7_ *parm7;	pointer to an initialized structure defining
								the datum conversion to be performed.
	int st;						returns zero to indicate success, -1 on
								failure.  Only cause for failure is the
								convergence failure of geocentric inverse.

	This variation of the seven parameter transformation assumes an
	ellipsoid height of zero.  See CS_7p3dFowrd for more information.
*/
int EXP_LVL9 CS_7p2dFowrd (double trgLl [3],Const double srcLl [3],Const struct cs_Parm7_ *parm7)
{
	extern double cs_Zero;			/* 0.0 */

	int status;
	double lcl_ll [3];

	trgLl [LNG] = lcl_ll [LNG] = srcLl [LNG];
	trgLl [LAT] = lcl_ll [LAT] = srcLl [LAT];
	trgLl [HGT] = srcLl [HGT];

	lcl_ll [HGT] = cs_Zero;
	status = CS_7p3dFowrd (lcl_ll,lcl_ll,parm7);
	if (status >= 0)
	{
		trgLl [LNG] = lcl_ll [LNG];
		trgLl [LAT] = lcl_ll [LAT];
	}
	return status;
}

/**********************************************************************
	st = CS_7p2dInvrs (trgLl,srcLl,parm7);

	double trgLl [3];			the converted results are returned here.
								Lng/Lat are in degrees, Hgt is copied from
								the srcLl argument.
	double srcLl [3];			the coordinate to be converted. Hgt element
								is not involved in the calculation; an
								ellipsoid hieght of zero is assumed.
	struct cs_Parm7_ *parm7;	pointer to an initialized structure defining
								the datum conversion to be performed.
	int st;						returns zero to indicate success, -1 on
								failure.  Only cause for failure is the
								convergence failure of geodetic or
								7 parameter iterative calculations.

	This 2D inverse is an iterative solution to the CS_7p2dFowrd
	algorithm.  It is required to prevent positional creep in the
	coordinates due to the loss of the ellipsoid elevation (Z) in
	the conversion from 2D to 3D and back again.
*/
int EXP_LVL9 CS_7p2dInvrs (double trgLl [3],Const double srcLl [3],Const struct cs_Parm7_ *parm7)
{
	static int maxIteration = 20;
	static double smallValue  = 1.0E-09;		/* equates to =~ .1 millimeters */
	static double smallValue2 = 1.0E-06;		/* equates to =~ 100 millimeters */

	int ii;
	int lngOk;
	int latOk;
	int rtnVal;

	double guess [3];
	double newLl [3];
	double epsilon [3];

	/* Assume everything goes OK until we know different. */
	rtnVal = 0;

	/* First, we copy the source lat/longs to the local array.
	   This is the default result which the user may want in
	   the event of an error.  Note, we assume such has been
	   done below, even if there has not been an error. */
	trgLl [LNG] = guess [LNG] = srcLl [LNG];
	trgLl [LAT] = guess [LAT] = srcLl [LAT];
	trgLl [HGT] = srcLl [HGT];

	/* Start a loop which will iterate as many as maxIteration times. */
	for (ii = 0;ii < maxIteration;ii++)
	{
		/* Assume we are done until we know different. */
		lngOk = latOk = TRUE;

		/* Compute the WGS-84 lat/long for our current guess. */
		rtnVal = CS_7p2dFowrd (newLl,guess,parm7);
		if (rtnVal != 0)
		{
			/* Oopps!! We must have been given some pretty strange
			   coordinate values. */
			break;
		}

		/* See how far we are off. */
		epsilon [LNG] = srcLl [LNG] - newLl [LNG];
		epsilon [LAT] = srcLl [LAT] - newLl [LAT];

		/* If our guess at the longitude is off by more than
		   small, we adjust our guess by the amount we are off. */
		if (fabs (epsilon [LNG]) > smallValue)
		{
			lngOk = FALSE;
			guess [LNG] += epsilon [LNG];
		}
		/* If our guess longitude is off by more than
		   small, we adjust our guess by the amount we are off. */
		if (fabs (epsilon [LAT]) > smallValue)
		{
			latOk = FALSE;
			guess [LAT] += epsilon [LAT];
		}

		/* If our current guess produces a newLl that is within
		   samllValue of srcLl, we are done. */
		if (lngOk && latOk) break;
	}

	/* If we didn't resolve in maxIteration tries, we issue a warning
	   message.  Usually, three or four iterations does the trick. */
	if (ii >= maxIteration )
	{
		CS_erpt (cs_7P_CNVRG);

		/* Issue a warning if we're close, a fatal if we are still way off.
		   In any case, we return the last computed value.  We could have
		   gotten very fancy with this stuff, but it would have had serious
		   affects on the performance.  So we just check epsilon here as
		   we know we have an error and this doesn't happen very often. */
		rtnVal = 1;
		if (fabs (epsilon [LNG]) > smallValue2 ||
		    fabs (epsilon [LAT]) > smallValue2)
		{
			rtnVal = -1;
		}
	}

	/* Return the resuls if everything converged. */
	if (rtnVal >= 0)
	{
		trgLl [LNG] = guess [LNG];
		trgLl [LAT] = guess [LAT];
	}
	return rtnVal;
}

/**********************************************************************
	st = CS_7p3dInvrs (trgLl,srcLl,parm7);

	double trgLl [3];			the converted results are returned here.
								Lng/Lat are in degrees.  Hgt is
								considered to be ellipsoid height, and
								must be in the units of the equatorial
								radius in the parm7 structure.
	double srcLl [3];			the coordinate to be converted.
	struct cs_Parm7_ *parm7;	pointer to an initialized structure defining
								the datum conversion to be performed.
	int st;						returns zero to indicate success, -1 on
								failure.  Only cause for failure is the
								convergence failure of geocentric inverse.

	This algorithm is the inverse of CS_7p3dForwd.  Note, that the rotation
	matrix is considered orthognal, thus the inverse of the matrix is the
	transpose of the matrix, and is accomplished by deft refernecing of the
	matrix elements.
*/
int EXP_LVL9 CS_7p3dInvrs (double trgLl [3],Const double srcLl [3],Const struct cs_Parm7_ *parm7)
{
	int status;

	double xx, yy, zz;
	double xyz [3];

	/* Convert the geographic coordinates to geocentric XYZ coordinates. */
	CS_llhToXyz (xyz,srcLl,parm7->trgERad,parm7->trgESqr);

	/* Reverse the scaling and translation. */
	xx = (xyz [XX] - parm7->delta_X) / parm7->scale;
	yy = (xyz [YY] - parm7->delta_Y) / parm7->scale;
	zz = (xyz [ZZ] - parm7->delta_Z) / parm7->scale;

	/* Apply the inverse of the rotation matrix by deftly multiply by the
	   transpos of the forwrd matrix. */
	xyz [XX] = parm7->rt11 * xx + parm7->rt21 * yy + parm7->rt31 * zz;
	xyz [YY] = parm7->rt12 * xx + parm7->rt22 * yy + parm7->rt32 * zz;
	xyz [ZZ] = parm7->rt13 * xx + parm7->rt23 * yy + parm7->rt33 * zz;

	/* Convert the new X, Y, and Z back to latitude and longitude.
	   CS_xyzToLlh returns degrees. */
	status = CS_xyzToLlh (trgLl,xyz,parm7->srcERad,parm7->srcESqr);

	/* That's that. */
	return status;
}

/******************************************************************************
	geoctr = CS_gcInit (srcDatum,trgDatum);

	struct cs_Datum_  *srcDatum; pointer to the definition of the source datum.
	struct cs_Datum_  *trgDatuu; pointer to the definition of the target datum.
	struct cs_Geoctr_ *geoctr;	 returns a pointer to an initialized parameter
								 structure which is the required argument for
								 CS_gcForwd and CS_gcInvrs.

	The function returns null in the event of an error. Memory allocation
	failure is the principal cause of failure.
*/
struct cs_Geoctr_ * EXP_LVL9 CS_gcInit (Const struct cs_Datum_* srcDatum,
								        Const struct cs_Datum_* trgDatum)
{
	struct cs_Geoctr_ *geoctr;

	geoctr = (struct cs_Geoctr_ *)CS_malc (sizeof (struct cs_Geoctr_));
	if (geoctr == NULL)
	{
		CS_erpt (cs_NO_MEM);
	}
	else
	{
		geoctr->srcERad = srcDatum->e_rad;
		geoctr->srcESqr = srcDatum->ecent * srcDatum->ecent;
		CS_stncp (geoctr->srcKeyName,srcDatum->key_nm,sizeof (geoctr->srcKeyName));

		geoctr->trgERad = trgDatum->e_rad;
		geoctr->trgESqr = trgDatum->ecent * trgDatum->ecent;
		CS_stncp (geoctr->trgKeyName,trgDatum->key_nm,sizeof (geoctr->trgKeyName));

		geoctr->delta_X = srcDatum->delta_X - trgDatum->delta_X;
		geoctr->delta_Y = srcDatum->delta_Y - trgDatum->delta_Y;
		geoctr->delta_Z = srcDatum->delta_Z - trgDatum->delta_Z;
	}
	return geoctr;
}

/**********************************************************************
	st = CS_gc3dFowrd (trgLl,srcLl,geoctr);

	double trgLl [3];			the converted results are returned here.
								Lng/Lat are in degrees, Hgt is usually meters.
								Hgt is considered to be ellipsoid height, and
								must be in the units of the equatorial radius
								in the geoctr structure, usually meters.
	double srcLl [3];			the coordinate to be converted.  Elements have
								the same characteristics as described above.
	struct cs_Geoctr_ *geoctr;	pointer to an initialized structure defining
								the datum conversion to be performed.
	int st;						returns zero to indicate success, -1 on
								failure.  Only cause for failure is the
								convergence failure of geocentric inverse.

	This is the standard Geocentric Translation Transformation.  What the
	DMA calls Molodensky is actually a different formulation of this
	transformation.
*/
int EXP_LVL9 CS_gc3dFowrd (double trgLl [3],Const double srcLl [3],Const struct cs_Geoctr_ *geoctr)
{
	int status;
	double xyz [3];

	/* Convert the geographic coordinates to geocentric XYZ coordinates. */
	CS_llhToXyz (xyz,srcLl,geoctr->srcERad,geoctr->srcESqr);

	/* Adjust these cartesian coordinates via the Bursa/Wolf transformation. */
	xyz [XX] += geoctr->delta_X;
	xyz [YY] += geoctr->delta_Y;
	xyz [ZZ] += geoctr->delta_Z;

	/* Convert the new X, Y, and Z back to latitude and longitude.
	   CS_xyzToLlh returns degrees. */
	status = CS_xyzToLlh (trgLl,xyz,geoctr->trgERad,geoctr->trgESqr);

	/* That's that. */
	return status;
}

/**********************************************************************
	st = CS_gc2dFowrd (trgLl,srcLl,geoctr);

	double trgLl [3];			the converted results are returned here.
								Lng/Lat are in degrees, Hgt element is
								copied from the srcLl array.
	double srcLl [3];			the coordinate to be converted.  Hgt
								element is not involved in the calculation,
								an ellipsoidal height of zero is assumed.
	struct cs_Geoctr_ *geoctr;	pointer to an initialized structure defining
								the datum conversion to be performed.
	int st;						returns zero to indicate success, -1 on
								failure.  Only cause for failure is the
								convergence failure of geocentric inverse.

	See CS_gc2dFowrd for more information.
*/
int EXP_LVL9 CS_gc2dFowrd (double trgLl [3],Const double srcLl [3],Const struct cs_Geoctr_ *geoctr)
{
	extern double cs_Zero;			/* 0.0 */

	int status;
	double lcl_ll [3];

	trgLl [LNG] = lcl_ll [LNG] = srcLl [LNG];
	trgLl [LAT] = lcl_ll [LAT] = srcLl [LAT];
	trgLl [HGT] = srcLl [HGT];

	lcl_ll [HGT] = cs_Zero;
	status = CS_gc3dFowrd (lcl_ll,lcl_ll,geoctr);
	if (status >= 0)
	{
		trgLl [LNG] = lcl_ll [LNG];
		trgLl [LAT] = lcl_ll [LAT];
	}
	return status;
}

/**********************************************************************
	st = CS_gc3dInvrs (trgLl,srcLl,geoctr);

	double trgLl [3];			the converted results are returned here.
								Lng/Lat are in degrees, Hgt is usually meters.
								Hgt is considered to be ellipsoid height, and
								must be in the units of the equatorial radius
								in the geoctr structure.
	double srcLl [3];			the coordinate to be converted.  Elements have
								the same characteristics as described above.
	struct cs_Geoctr_ *geoctr;	pointer to an initialized structure defining
								the datum conversion to be performed.
	int st;						returns zero to indicate success, -1 on
								failure.  Only cause for failure is the
								convergence failure of geocentric inverse.

	This is the inverse of the Geocentric Translation Transformation.  See the
	forward function, above, for more information.  The inverse of this
	transformation is usually accomplished by inverting the parameters to the
	initialization function.  Therefore, this function is not used in the
	normal operation of CS_MAP.
*/
int EXP_LVL9 CS_gc3dInvrs (double trgLl [3],Const double srcLl [3],Const struct cs_Geoctr_ *geoctr)
{
	int status;
	double xyz [3];

	/* Convert the geographic coordinates to geocentric XYZ coordinates. */
	CS_llhToXyz (xyz,srcLl,geoctr->trgERad,geoctr->trgESqr);

	/* Apply the translation parameters. */
	xyz [XX] -= geoctr->delta_X;
	xyz [YY] -= geoctr->delta_Y;
	xyz [ZZ] -= geoctr->delta_Z;

	/* Convert the new X, Y, and Z back to latitude and longitude.
	   CS_xyzToLlh returns degrees. */
	status = CS_xyzToLlh (trgLl,xyz,geoctr->srcERad,geoctr->srcESqr);

	/* That's that. */
	return status;
}

/**********************************************************************
	st = CS_gc2dInvrs (trgLl,srcLl,geoctr);

	double trgLl [3];			the converted results are returned here.
								Lng/Lat are in degrees, Hgt is copied from
								the srcLl argument.
	double srcLl [3];			the coordinate to be converted. Hgt element
								is not involved in the calculation; an
								ellipsoid hieght of zero is assumed.
	struct cs_Geoctr_ *geoctr;	pointer to an initialized structure defining
								the datum conversion to be performed.
	int st;						returns zero to indicate success, -1 on
								failure.  Only cause for failure is the
								convergence failure of geodetic or
								3 parameter iterative calculations.

	This function is an iterative inverse of CS_gc3dFowrd.  This is
	required to prevent positional creep in coordinates.  See
	CS_gc3dFowrd and CS_gc3dInvrs for more information.
*/
int EXP_LVL9 CS_gc2dInvrs (double trgLl [3],Const double srcLl [3],Const struct cs_Geoctr_ *geoctr)
{
	static int maxIteration = 8;
	static double smallValue  = 1.0E-09;		/* equates to =~ .1 millimeters */
	static double smallValue2 = 1.0E-06;		/* equates to =~ 100 millimeters */

	int ii;
	int lngOk;
	int latOk;
	int rtnVal;

	double guess [3];
	double newLl [3];
	double epsilon [3];

	/* Assume everything goes OK until we know different. */
	rtnVal = 0;

	/* First, we copy the source lat/longs to the local array.
	   This is the default result which the user may want in
	   the event of an error.  Note, we assume such has been
	   done below, even if there has not been an error. */
	trgLl [LNG] = guess [LNG] = srcLl [LNG];
	trgLl [LAT] = guess [LAT] = srcLl [LAT];
	trgLl [HGT] = srcLl [HGT];

	/* To improve performancce, we could use CS_gc3dInvrs here to
	   get our guess.  That function will probably reduce the
	   number of iterations require below substantially. */

	/* Start a loop which will iterate as many as maxIteration times. */
	for (ii = 0;ii < maxIteration;ii++)
	{
		/* Assume we are done until we know different. */
		lngOk = latOk = TRUE;

		/* Compute the WGS-84 lat/long for our current guess. */
		rtnVal = CS_gc2dFowrd (newLl,guess,geoctr);
		if (rtnVal != 0)
		{
			/* Oopps!! We must have been given some pretty strange
			   coordinate values. */
			break;
		}

		/* See how far we are off. */
		epsilon [LNG] = srcLl [LNG] - newLl [LNG];
		epsilon [LAT] = srcLl [LAT] - newLl [LAT];

		/* If our guess at the longitude is off by more than
		   small, we adjust our guess by the amount we are off. */
		if (fabs (epsilon [LNG]) > smallValue)
		{
			lngOk = FALSE;
			guess [LNG] += epsilon [LNG];
		}
		/* If our guess at the latitude is off by more than
		   small, we adjust our guess by the amount we are off. */
		if (fabs (epsilon [LAT]) > smallValue)
		{
			latOk = FALSE;
			guess [LAT] += epsilon [LAT];
		}

		/* If our current guess produces a newLl that is within
		   samllValue of srcLl, we are done. */
		if (lngOk && latOk) break;
	}

	/* If we didn't resolve in maxIteration tries, we issue a warning
	   message.  Usually, three or four iterations does the trick. */
	if (ii >= maxIteration )
	{
		CS_erpt (cs_3P_CNVRG);

		/* Issue a warning if we're close, a fatal if we are still way off.
		   In any case, we return the last computed value.  We could have
		   gotten very fancy with this stuff, but it would have had serious
		   affects on the performance.  So we just check epsilon here as
		   we know we have an error and this doesn't happen very often. */
		rtnVal = 1;
		if (fabs (epsilon [LNG]) > smallValue2 ||
		    fabs (epsilon [LAT]) > smallValue2)
		{
			rtnVal = -1;
		}
	}

	/* Adjust the trgLl value to the computed value, now that we
	   know that it should be correct. */
	if (rtnVal >= 0)
	{
		trgLl [LNG] = guess [LNG];
		trgLl [LAT] = guess [LAT];
	}
	return rtnVal;
}

/******************************************************************************
	molo = CS_moInit (srcDatum,trgDatum);

	struct cs_Datum_ *srcDatum;	pointer to the definition of the source datum.
	struct cs_Datum_ *trgDatuu; pointer to the definition of the target datum.
	struct cs_Molo_ *molo;		returns a pointer to an initialized parameter
								structure which is the required argument for
								CS_moForwd and CS_moInvrs.

	The function returns null in the event of an error. Memory allocation
	failure is the principal cause of failure.
*/
struct cs_Molo_* EXP_LVL9 CS_moInit (Const struct cs_Datum_* srcDatum,
									 Const struct cs_Datum_* trgDatum)
{
	struct cs_Molo_ *molo;

	molo = (struct cs_Molo_ *)CS_malc (sizeof (struct cs_Molo_));
	if (molo == NULL)
	{
		CS_erpt (cs_NO_MEM);
	}
	else
	{
		molo->srcERad = srcDatum->e_rad;
		molo->srcPRad = srcDatum->p_rad;
		molo->srcESqr = srcDatum->ecent * srcDatum->ecent;
		molo->srcFlat = srcDatum->flat;
		CS_stncp (molo->srcKeyName,srcDatum->key_nm,sizeof (molo->srcKeyName));

		molo->trgERad = trgDatum->e_rad;
		molo->trgPRad = trgDatum->p_rad;
		molo->trgESqr = trgDatum->ecent * trgDatum->ecent;
		molo->trgFlat = trgDatum->flat;
		CS_stncp (molo->trgKeyName,trgDatum->key_nm,sizeof (molo->trgKeyName));

		molo->delta_X = srcDatum->delta_X - trgDatum->delta_X;
		molo->delta_Y = srcDatum->delta_Y - trgDatum->delta_Y;
		molo->delta_Z = srcDatum->delta_Z - trgDatum->delta_Z;
	}
	return molo;
}

/**********************************************************************
	status = CS_mo3dFowrd (trgLl,srcLl,molo);

	double trgLl [3];			the results are returned here.  LL are in
								degrees, hgt must be in the same units as
								the equatorial radii in the cs_Molo_
								structure; usually meters.
	double srcLl [3];			the coordinate to be converted.
	struct cs_Molo_ *molo;		pointer to a structure which has been initialized
								for the Molodensky transformation.
	int status;					returns for success, -1 for failure.
								Currently, this function is always
								successful.

	This transformation was erroneously dubbed Molodensky by the DMA.
	It is a formulation of the Three Parameter Transformation which
	is rather verbose and not very useful.  However, since we Casual
	Cartographers didn't know better, this transformation was used
	quite a bit.  So, we support this transformation, but mostly for
	legacy purposes.
*/
int EXP_LVL7 CS_mo3dFowrd (double trgLl [3],Const double srcLl [3],Const struct cs_Molo_ *molo)
{
	extern double cs_Zero;				/* 0.0 */
	extern double cs_One;				/* 1.0 */
	extern double cs_Degree;			/* 0.017... */
	extern double cs_Sin1Sec;			/* sine of 1 second of arc. */
	extern double cs_Sec2Deg;			/* 3600.00 */
	extern double cs_NPTest;			/* 89.99999 in radians. */

	double lng, lat;
	double sinLng, cosLng;
	double sinLat, cosLat;
	double sinCosLat;
	double sinSqLat;

	double Rn;
	double Rm;
	double temp1;
	double sqrtTemp1;
	double term1, term2, term3;
	double deltaLngSec, deltaLatSec, deltaHgt;

	lng = srcLl [LNG] * cs_Degree;
	lat = srcLl [LAT] * cs_Degree;

	sinLng = sin (lng);
	sinLat = sin (lat);
	cosLng = cos (lng);
	cosLat = cos (lat);
	sinSqLat = sinLat * sinLat;
	sinCosLat = sinLat * cosLat;

	temp1 = cs_One - (molo->srcESqr * sinSqLat);
	sqrtTemp1 = sqrt (temp1);

	/* Compute ellipsoid radii of curvature of the ellipsoid upon which
	   the source datum is based. */
	Rn = molo->srcERad / sqrtTemp1;
	Rm = molo->srcERad * (cs_One - molo->srcESqr) / (sqrtTemp1 * temp1);

	/* Compute delta latitude in seconds of arc. */
	term1 = molo->delta_Z * cosLat -
			molo->delta_X * sinLat * cosLng -
			molo->delta_Y * sinLat * sinLng;
	term2 = (molo->trgERad - molo->srcERad) * molo->srcESqr * Rn * sinCosLat / molo->srcERad;
	temp1 = (Rm * molo->srcERad / molo->srcPRad) + Rn * molo->srcPRad / molo->srcERad;
	term3 = (molo->trgFlat - molo->srcFlat) * temp1 * sinCosLat;
	deltaLatSec = (term1 + term2 + term3) / ((Rm + srcLl [HGT]) * cs_Sin1Sec);

	/* Compute delta longitude in seconds of arc.  At the poles,
	   there is no delta longitude.  Must deal with this to prevent
	   division by zero. */
	if (fabs (lat) > cs_NPTest)
	{
		deltaLngSec = cs_Zero;
	}
	else
	{
		temp1 = molo->delta_Y * cosLng - molo->delta_X * sinLng;
		deltaLngSec = temp1 / ((Rn + srcLl [HGT]) * cosLat * cs_Sin1Sec);
	}

	/* Compute delta height. */
	term1 = molo->delta_X * cosLat * cosLng +
			molo->delta_Y * cosLat * sinLng +
			molo->delta_Z * sinLat;
	term2 = (molo->trgERad - molo->srcERad) * molo->srcERad / Rn;
	term3 = (molo->trgFlat - molo->srcFlat) * (molo->srcPRad / molo->srcERad) * Rn * sinSqLat;
	deltaHgt = term1 - term2 + term3;

	/* Compute the final results. */
	trgLl [LNG] = srcLl [LNG] + (deltaLngSec * cs_Sec2Deg);
	trgLl [LAT] = srcLl [LAT] + (deltaLatSec * cs_Sec2Deg);
	trgLl [HGT] = srcLl [HGT] + deltaHgt;

	/* This is always successul. */
	return 0;
}

/**********************************************************************
	CS_mo2dFowrd (trgLl,srcLl,molo);

	double trgLl [3];			the results are returned here.  LL are in
								degrees, hgt element is copied from the
								srcLl argument.
	double srcLl [3];			the coordinate to be converted.  Hgt element
								is ignored; an ellipsoid height of zero is
								assumed.
	struct cs_Molo_ *molo;		pointer to a structure which has been initialized
								for the Molodensky transformation.
	int status;					zero for success, -1 for failure. This
								will fail only if CS_mo3dFowrd fails.

	See CS_mo3dFowrd for more information.
*/
int EXP_LVL7 CS_mo2dFowrd (double trgLl [3],Const double srcLl [3],Const struct cs_Molo_ *molo)
{
	extern double cs_Zero;				/* 0.0 */

	int status;
	double lcl_ll [3];

	trgLl [LNG] = lcl_ll [LNG] = srcLl [LNG];
	trgLl [LAT] = lcl_ll [LAT] = srcLl [LAT];
	trgLl [HGT] = srcLl [HGT];

	lcl_ll [HGT] = cs_Zero;
	status = CS_mo3dFowrd (lcl_ll,lcl_ll,molo);
	if (status >= 0)
	{
		trgLl [LNG] = lcl_ll [LNG];
		trgLl [LAT] = lcl_ll [LAT];
	}
	return status;
}

/**********************************************************************
	status = CS_mo3dInvrs (trgLl,srcLl,molo);

	double trgLl [3];			the results are returned here.  LL are in
								degrees, hgt must be in the same units as
								the equatroial radii in the cs_Molo_
								structure; usually meters.
	double srcLl [3];			the coordinate to be converted.
	struct cs_Molo_ *molo;		pointer to a structure which has been initialized
								for the Molodensky transformation.
	int status;					returns zero for success, -1 for failure.
								Currently, this function is always
								successful.  Maybe not in future.

	This is the inverse of the CS_moFowrd function.  See that function for
	more information.
*/
int EXP_LVL7 CS_mo3dInvrs (double trgLl [3],Const double srcLl [3],Const struct cs_Molo_ *molo)
{
	extern double cs_Zero;				/* 0.0 */
	extern double cs_One;				/* 1.0 */
	extern double cs_Degree;			/* 0.017... */
	extern double cs_Sin1Sec;			/* sine of 1 second of arc. */
	extern double cs_Sec2Deg;			/* 3600.00 */
	extern double cs_NPTest;			/* 89.99999 in radians. */

	double lng, lat;
	double sinLng, cosLng;
	double sinLat, cosLat;
	double sinCosLat;
	double sinSqLat;

	double Rn;
	double Rm;
	double temp1;
	double sqrtTemp1;
	double term1, term2, term3;
	double deltaLngSec, deltaLatSec, deltaHgt;

	lng = srcLl [LNG] * cs_Degree;
	lat = srcLl [LAT] * cs_Degree;

	sinLng = sin (lng);
	sinLat = sin (lat);
	cosLng = cos (lng);
	cosLat = cos (lat);
	sinSqLat = sinLat * sinLat;
	sinCosLat = sinLat * cosLat;

	temp1 = cs_One - (molo->trgESqr * sinSqLat);
	sqrtTemp1 = sqrt (temp1);

	/* Compute ellipsoid radii of curvature of the ellipsoid upon which
	   the source datum is based. */
	Rn = molo->trgERad / sqrtTemp1;
	Rm = molo->trgERad * (cs_One - molo->trgESqr) / (sqrtTemp1 * temp1);

	/* Compute delta latitude in seconds of arc. */
	term1 = molo->delta_X * sinLat * cosLng +
			molo->delta_Y * sinLat * sinLng -
			molo->delta_Z * cosLat;
	term2 = (molo->srcERad - molo->trgERad) * molo->trgESqr * Rn * sinCosLat / molo->trgERad;
	temp1 = (Rm * molo->trgERad / molo->trgPRad) + Rn * molo->trgPRad / molo->trgERad;
	term3 = (molo->srcFlat - molo->trgFlat) * temp1 * sinCosLat;
	deltaLatSec = (term1 + term2 + term3) / ((Rm + srcLl [HGT]) * cs_Sin1Sec);

	/* Compute delta longitude in seconds of arc.  At the poles,
	   there is no delta longitude.  Must deal with this to prevent
	   division by zero. */
	if (fabs (lat) > cs_NPTest)
	{
		deltaLngSec = cs_Zero;
	}
	else
	{
		temp1 = molo->delta_X * sinLng - molo->delta_Y * cosLng;
		deltaLngSec = temp1 / ((Rn + srcLl [HGT]) * cosLat * cs_Sin1Sec);
	}

	/* Compute delta height. */
	term1 = molo->delta_X * cosLat * cosLng +
			molo->delta_Y * cosLat * sinLng +
			molo->delta_Z * sinLat;
	term2 = (molo->srcERad - molo->trgERad) * molo->trgERad / Rn;
	term3 = (molo->srcFlat - molo->trgFlat) * (molo->trgPRad / molo->trgERad) * Rn * sinSqLat;
	deltaHgt = term3 - term2 - term1;

	/* Compute the final results. */
	trgLl [LNG] = srcLl [LNG] + (deltaLngSec * cs_Sec2Deg);
	trgLl [LAT] = srcLl [LAT] + (deltaLatSec * cs_Sec2Deg);
	trgLl [HGT] = srcLl [HGT] + deltaHgt;

	/* This is always successul. */
	return 0;
}

/**********************************************************************
	status = CS_mo3dInvrs (trgLl,srcLl,molo);

	double trgLl [3];			the results are returned here.  LL are in
								degrees, hgt element is copied from the
								srcLl array.
	double srcLl [3];			the coordinate to be converted.  Hgt element
								is not involved in the calculations; an
								ellipsoidal height of zero is assumed. 
	struct cs_Molo_ *molo;		pointer to a structure which has been initialized
								for the Molodensky transformation.

	This is an iterative inverse of the CS_mo2dFowrd function.  This is
	required to prevent positional creep of two dimensional geographic
	coordinates.
*/
int EXP_LVL7 CS_mo2dInvrs (double trgLl [3],Const double srcLl [3],Const struct cs_Molo_ *molo)
{
	static int maxIteration = 8;
	static double smallValue  = 1.0E-09;		/* equates to =~ .1 millimeters */
	static double smallValue2 = 1.0E-06;		/* equates to =~ 100 millimeters */

	int ii;
	int lngOk;
	int latOk;
	int rtnVal;

	double guess [3];
	double newLl [3];
	double epsilon [3];

	/* Assume everything goes OK until we know different. */
	rtnVal = 0;

	/* First, we copy the source lat/longs to the local array.
	   This is the default result which the user may want in
	   the event of an error.  Note, we assume such has been
	   done below, even if there has not been an error. */
	trgLl [LNG] = guess [LNG] = srcLl [LNG];
	trgLl [LAT] = guess [LAT] = srcLl [LAT];
	trgLl [HGT] = srcLl [HGT];

	/* To improve performancce, we could use CS_mo3dInvrs here to
	   get our guess.  Doing so could reduce the number of iterations
	   required below substantially. */

	/* Start a loop which will iterate as many as maxIteration times. */
	for (ii = 0;ii < maxIteration;ii++)
	{
		/* Assume we are done until we know different. */
		lngOk = latOk = TRUE;

		/* Compute the WGS-84 lat/long for our current guess. */
		rtnVal = CS_mo2dFowrd (newLl,guess,molo);
		if (rtnVal != 0)
		{
			/* Oopps!! We must have been given some pretty strange
			   coordinate values. */
			break;
		}

		/* See how far we are off. */
		epsilon [LNG] = srcLl [LNG] - newLl [LNG];
		epsilon [LAT] = srcLl [LAT] - newLl [LAT];

		/* If our guess at the longitude is off by more than
		   small, we adjust our guess by the amount we are off. */
		if (fabs (epsilon [LNG]) > smallValue)
		{
			lngOk = FALSE;
			guess [LNG] += epsilon [LNG];
		}
		/* If our guess longitude is off by more than
		   small, we adjust our guess by the amount we are off. */
		if (fabs (epsilon [LAT]) > smallValue)
		{
			latOk = FALSE;
			guess [LAT] += epsilon [LAT];
		}

		/* If our current guess produces a newLl that is within
		   samllValue of srcLl, we are done. */
		if (lngOk && latOk) break;
	}

	/* If we didn't resolve in maxIteration tries, we issue a warning
	   message.  Usually, three or four iterations does the trick. */
	if (ii >= maxIteration )
	{
		CS_erpt (cs_MO_CNVRG);

		/* Issue a warning if we're close, a fatal if we are still way off.
		   In any case, we return the last computed value.  We could have
		   gotten very fancy with this stuff, but it would have had serious
		   affects on the performance.  So we just check epsilon here as
		   we know we have an error and this doesn't happen very often. */
		rtnVal = 1;
		if (fabs (epsilon [LNG]) > smallValue2 ||
		    fabs (epsilon [LAT]) > smallValue2)
		{
			rtnVal = -1;
		}
	}

	/* Adjust the ll_lcl value to the computed value, now that we
	   know that it should be correct. */
	if (rtnVal >= 0)
	{
		trgLl [LNG] = guess [LNG];
		trgLl [LAT] = guess [LAT];
	}
	return rtnVal;
}

/******************************************************************************
	parm6 = CS_6pInit (srcDatum,trgDatum);

	struct cs_Datum_ *srcDatum;	pointer to the definition of the source datum.
	struct cs_Datum_ *trgDatuu; pointer to the definition of the target datum.
	struct cs_Parm6_ *parm6;	returns a pointer to an initialized parameter
								structure which is the required argument for
								CS_p6Forwd and CS_p6Invrs.

	The function returns null in the event of an error. Memory allocation
	failure is the principal cause of failure.
*/
struct cs_Parm6_ * EXP_LVL9 CS_6pInit (Const struct cs_Datum_* srcDatum,
									   Const struct cs_Datum_* trgDatum)
{
	extern double cs_Sec2Rad;			/* 4.848136E-06 */

	struct cs_Parm6_ *parm6;

	double sinAlpha, cosAlpha;
	double sinBeta, cosBeta;
	double sinGamma, cosGamma;

	parm6 = (struct cs_Parm6_ *)CS_malc (sizeof (struct cs_Parm6_));
	if (parm6 == NULL)
	{
		CS_erpt (cs_NO_MEM);
	}
	else
	{
		parm6->srcERad = srcDatum->e_rad;
		parm6->srcESqr = srcDatum->ecent * trgDatum->ecent;
		CS_stncp (parm6->srcKeyName,srcDatum->key_nm,sizeof (parm6->srcKeyName));

		parm6->trgERad = trgDatum->e_rad;
		parm6->trgESqr = trgDatum->ecent * trgDatum->ecent;
		CS_stncp (parm6->trgKeyName,trgDatum->key_nm,sizeof (parm6->trgKeyName));

		parm6->delta_X = srcDatum->delta_X - trgDatum->delta_X;
		parm6->delta_Y = srcDatum->delta_Y - trgDatum->delta_Y;
		parm6->delta_Z = srcDatum->delta_Z - trgDatum->delta_Z;

		sinAlpha = sin ((srcDatum->rot_X - trgDatum->rot_X) * cs_Sec2Rad);
		cosAlpha = cos ((srcDatum->rot_X - trgDatum->rot_X) * cs_Sec2Rad);
		sinBeta  = sin ((srcDatum->rot_Y - trgDatum->rot_Y) * cs_Sec2Rad);
		cosBeta  = cos ((srcDatum->rot_Y - trgDatum->rot_Y) * cs_Sec2Rad);
		sinGamma = sin ((srcDatum->rot_Z - trgDatum->rot_Z) * cs_Sec2Rad);
		cosGamma = cos ((srcDatum->rot_Z - trgDatum->rot_Z) * cs_Sec2Rad);

		/* 3x3 Rotation matrix, assumed to be orthogonal */
		parm6->rt11 =  cosBeta * cosGamma;
		parm6->rt21 = -cosBeta * sinGamma;
		parm6->rt31 =  sinBeta;
		parm6->rt12 =  cosAlpha * sinGamma + sinAlpha * sinBeta * cosGamma;
		parm6->rt22 =  cosAlpha * cosGamma - sinAlpha * sinBeta * sinGamma;
		parm6->rt32 = -sinAlpha * cosBeta;
		parm6->rt13 =  sinAlpha * sinGamma - cosAlpha * sinBeta * cosGamma;
		parm6->rt23 =  sinAlpha * cosGamma + cosAlpha * sinBeta * sinGamma;
		parm6->rt33 =  cosAlpha * cosBeta;
	}
	return parm6;
}

/**********************************************************************
	status = CS_6p3dFowrd (trgLl,srcLl,parm6);

	double trgLl [3];			the converted results are returned here.
								Lng/Lat are in degrees, Hgt is usually meters.
								Hgt is considered to be ellipsoid height, and
								must be in the units of the equatorial radius
								in the parm6 structure, usually meters.
	double srcLl [3];			the coordinate to be converted.  Elements have
								the same characteristics as described above.
	struct cs_Parm6_ *parm6;	pointer to an initialized structure defining
								the datum conversion to be performed.
	int status;					returns zero to indicate success, -1 on
								failure.  Only cause for failure is the
								convergence failure of geocentric inverse.

	Essentially, a seven parameter transformation with the scale factor
	set to one.  We could achieve this by setting the scale factor to
	one, and calling CS_p7Fowrd.  We chose to have a separate function
	to be consistent with the transformation techniques.
*/
int EXP_LVL9 CS_6p3dFowrd (double trgLl [3],Const double srcLl [3],Const struct cs_Parm6_ *parm6)
{
	int status;

	double xx, yy, zz;
	double xyz [3];

	/* Convert the geographic coordinates to geocentric XYZ coordinates. */
	CS_llhToXyz (xyz,srcLl,parm6->srcERad,parm6->srcESqr);

	/* Adjust these cartesian coordinates via the Bursa/Wolf transformation. */
	xx = parm6->rt11 * xyz [XX] + parm6->rt12 * xyz [YY] + parm6->rt13 * xyz [ZZ];
	yy = parm6->rt21 * xyz [XX] + parm6->rt22 * xyz [YY] + parm6->rt23 * xyz [ZZ];
	zz = parm6->rt31 * xyz [XX] + parm6->rt32 * xyz [YY] + parm6->rt33 * xyz [ZZ];
	xyz [XX] = xx + parm6->delta_X;
	xyz [YY] = yy + parm6->delta_Y;
	xyz [ZZ] = zz + parm6->delta_Z;

	/* Convert the new X, Y, and Z back to latitude and longitude.
	   CS_xyzToLlh returns degrees. */
	status = CS_xyzToLlh (trgLl,xyz,parm6->trgERad,parm6->trgESqr);

	/* That's that. */
	return status;
}

/**********************************************************************
	status = CS_6p2dFowrd (trgLl,srcLl,parm6);

	double trgLl [3];			the converted results are returned here.
	r							Lng/Lat are in degrees, Hgt is copied from
								the srcLl array.
	double srcLl [3];			the coordinate to be converted.  Hgt
								element is not involved in the calculations,
								an ellipsoidal height of zero is assumed.
	struct cs_Parm6_ *parm6;	pointer to an initialized structure defining
								the datum conversion to be performed.
	int status;					returns zero to indicate success, -1 on
								failure.  Only cause for failure is the
								convergence failure of geocentric inverse.

	See CS_6p3dFowrd for more information.
*/
int EXP_LVL9 CS_6p2dFowrd (double trgLl [3],Const double srcLl [3],Const struct cs_Parm6_ *parm6)
{
	extern double cs_Zero;				/* 0.0 */

	int status;
	double lcl_ll [3];

	trgLl [LNG] = lcl_ll [LNG] = srcLl [LNG];
	trgLl [LAT] = lcl_ll [LAT] = srcLl [LAT];
	trgLl [HGT] = srcLl [HGT];

	lcl_ll [HGT] = cs_Zero;
	status = CS_6p3dFowrd (lcl_ll,lcl_ll,parm6);
	if (status >= 0)
	{
		trgLl [LNG] = lcl_ll [LNG];
		trgLl [LAT] = lcl_ll [LAT];
	}
	return status;
}

/*
	Note that there is no CS_6p3dInvrs.  Such a function would require
	inversion of the rotation matrix, which would be rather painful
	from a performance viewpoint.  3D inverse is calculated by
	inverting the arguments to the CS_6pIint function and using
	the CS_6p3dFowrd function and the resuling setup.
*/

/**********************************************************************
	st = CS_6p2dInvrs (trgLl,srcLl,parm6);

	double trgLl [3];			the converted results are returned here.
								Lng/Lat are in degrees, Hgt is copied from
								the srcLl argument.
	double srcLl [3];			the coordinate to be converted. Hgt element
								is not involved in the calculation; an
								ellipsoid hieght of zero is assumed.
	struct cs_Parm6_ *parm6;	pointer to an initialized structure defining
								the datum conversion to be performed.
	int st;						returns zero to indicate success, -1 on
								failure.  Only cause for failure is the
								convergence failure of geodetic or
								6 parameter iterative calculations.

	This function is an iterative inverse of CS_6p3dFowrd.  This is
	required to prevent positional creep in coordinates.  See
	CS_6p3dFowrd and CS_6p3dInvrs for more information.
*/
int EXP_LVL9 CS_6p2dInvrs (double trgLl [3],Const double srcLl [3],Const struct cs_Parm6_ *parm6)
{
	static int maxIteration = 8;
	static double smallValue  = 1.0E-09;		/* equates to =~ .1 millimeters */
	static double smallValue2 = 1.0E-06;		/* equates to =~ 100 millimeters */

	int ii;
	int lngOk;
	int latOk;
	int rtnVal;

	double guess [3];
	double newLl [3];
	double epsilon [3];

	/* Assume everything goes OK until we know different. */
	rtnVal = 0;

	/* First, we copy the source lat/longs to the local array.
	   This is the default result which the user may want in
	   the event of an error.  Note, we assume such has been
	   done below, even if there has not been an error. */
	trgLl [LNG] = guess [LNG] = srcLl [LNG];
	trgLl [LAT] = guess [LAT] = srcLl [LAT];
	trgLl [HGT] = srcLl [HGT];

	/* Start a loop which will iterate as many as maxIteration times. */
	for (ii = 0;ii < maxIteration;ii++)
	{
		/* Assume we are done until we know different. */
		lngOk = latOk = TRUE;

		/* Compute the WGS-84 lat/long for our current guess. */
		rtnVal = CS_6p2dFowrd (newLl,guess,parm6);
		if (rtnVal != 0)
		{
			/* Oopps!! We must have been given some pretty strange
			   coordinate values. */
			break;
		}

		/* See how far we are off. */
		epsilon [LNG] = srcLl [LNG] - newLl [LNG];
		epsilon [LAT] = srcLl [LAT] - newLl [LAT];

		/* If our guess at the longitude is off by more than
		   small, we adjust our guess by the amount we are off. */
		if (fabs (epsilon [LNG]) > smallValue)
		{
			lngOk = FALSE;
			guess [LNG] += epsilon [LNG];
		}
		/* If our guess longitude is off by more than
		   small, we adjust our guess by the amount we are off. */
		if (fabs (epsilon [LAT]) > smallValue)
		{
			latOk = FALSE;
			guess [LAT] += epsilon [LAT];
		}

		/* If our current guess produces a newLl that is within
		   samllValue of srcLl, we are done. */
		if (lngOk && latOk) break;
	}

	/* If we didn't resolve in maxIteration tries, we issue a warning
	   message.  Usually, three or four iterations does the trick. */
	if (ii >= maxIteration )
	{
		CS_erpt (cs_6P_CNVRG);

		/* Issue a warning if we're close, a fatal if we are still way off.
		   In any case, we return the last computed value.  We could have
		   gotten very fancy with this stuff, but it would have had serious
		   affects on the performance.  So we just check epsilon here as
		   we know we have an error and this doesn't happen very often. */
		rtnVal = 1;
		if (fabs (epsilon [LNG]) > smallValue2 ||
		    fabs (epsilon [LAT]) > smallValue2)
		{
			rtnVal = -1;
		}
	}

	/* Adjust the ll_lcl value to the computed value, now that we
	   know that it should be correct. */
	if (rtnVal >= 0)
	{
		trgLl [LNG] = guess [LNG];
		trgLl [LAT] = guess [LAT];
	}
	return rtnVal;
}

/**********************************************************************
	status = CS_6p3dInvrs (trgLl,srcLl,parm6);

	double trgLl [3];			the converted results are returned here.
								Lng/Lat are in degrees, Hgt is usually meters.
								Hgt is considered to be ellipsoid height, and
								must be in the units of the equatorial radius
								in the parm6 structure.
	double srcLl [3];			the coordinate to be converted.  Elements have
								the same characteristics as described above.
	struct cs_Parm6_ *parm6;	pointer to an initialized structure defining
								the datum conversion to be performed.
	int status;					returns zero to indicate success, -1 on
								failure.  Only cause for failure is the
								convergence failure of geocentric inverse.

	Inverse of the CS_6p3dForwd function.
*/
int EXP_LVL9 CS_6p3dInvrs (double trgLl [3],Const double srcLl [3],Const struct cs_Parm6_ *parm6)
{
	int status;

	double xx, yy, zz;
	double xyz [3];

	/* Convert the geographic coordinates to geocentric XYZ coordinates. */
	CS_llhToXyz (xyz,srcLl,parm6->srcERad,parm6->srcESqr);

	/* Undo the translation (there is no scaling). */
	xx = xyz [XX] - parm6->delta_X;
	yy = xyz [YY] - parm6->delta_Y;
	zz = xyz [ZZ] - parm6->delta_Z;

	/* Apply the inverse rotation matrix, which just happens to be the transpose
	   of the forward rotation matrix. */
	xyz [XX] = parm6->rt11 * xx + parm6->rt21 * yy + parm6->rt31 * zz;
	xyz [YY] = parm6->rt12 * xx + parm6->rt22 * yy + parm6->rt32 * zz;
	xyz [ZZ] = parm6->rt13 * xx + parm6->rt23 * yy + parm6->rt33 * zz;

	/* Convert the new X, Y, and Z back to latitude and longitude.
	   CS_xyzToLlh returns degrees. */
	status = CS_xyzToLlh (trgLl,xyz,parm6->trgERad,parm6->trgESqr);

	/* That's that. */
	return status;
}

/******************************************************************************
	mreg = CS_dmaMrInit (srcDatum);

	struct cs_Datum_ *srcDatum;	pointer to the definition of the source datum.
	struct cs_DmaMReg_ *mreg;	returns a pointer to an initialized parameter
								structure which is the required argument for
								CS_dmaMrForwd and CS_dmaMrInvrs.

	A target datum of WGS84 is implied with this set of functions.

	The function returns null in the event of an error. Memory allocation
	failure is the principal cause of failure.
*/
struct cs_DmaMReg_ * EXP_LVL9 CS_dmaMrInit (Const struct cs_Datum_* srcDatum)
{
	extern char cs_Dir [];
	extern char* cs_DirP;
	extern char cs_ExtsepC;
	extern char csErrnam [];
#if !defined(__SUNPRO_C)
	extern struct cs_Datum_ *cs_Wgs84Ptr;
#else
	extern struct cs_Datum_ cs_Wgs84Def;
	struct cs_Datum_ *cs_Wgs84Ptr = &cs_Wgs84Def;
#endif
	int flag;
	size_t rd_cnt;
	char *cp;
	FILE *strm;
	struct cs_DmaMReg_ *mreg;

	mreg = (struct cs_DmaMReg_ *)CS_malc (sizeof (struct cs_DmaMReg_));
	if (mreg == NULL)
	{
		CS_erpt (cs_NO_MEM);
	}
	else
	{
		cp = CS_stcpy (cs_DirP,srcDatum->key_nm);
		*cp++ = cs_ExtsepC;
		CS_stcpy (cp,cs_MULREG_EXT);
		strm = CS_fopen (cs_Dir,_STRM_BINRD);
		if (strm == NULL)
		{
			CS_free (mreg);
			mreg = NULL;
			CS_stncp (csErrnam,cs_Dir,MAXPATH);
			CS_erpt (cs_MRT_NTFND);
		}
		else
		{
			/* Such a file does indeed exist.  Read in the
			   transformation data. */
			rd_cnt = CS_fread ((void *)(&mreg->mReg),1,sizeof (mreg->mReg),strm);
			if (CS_ferror (strm)) rd_cnt = 1;
			CS_fclose (strm);

			/* We have to swap bytes before we check the magic number
			   or the read count. */
			CS_bswap (mreg,cs_BSWP_DMAMREG);

			/* See if we got what we expected. */
			if (rd_cnt == 1)
			{
				CS_free (mreg);
				mreg = NULL;
				CS_erpt (cs_IOERR);
			}
			else if (rd_cnt != (size_t)mreg->mReg.mr_size)
			{
				CS_free (mreg);
				mreg = NULL;
				CS_erpt (cs_INV_FILE);
			}
			else if (mreg->mReg.magic != cs_MULREG_MAGIC)
			{
				CS_free (mreg);
				mreg = NULL;
				CS_stncp (csErrnam,cs_Dir,MAXPATH);
				CS_erpt (cs_MREG_BADMAG);
			}
			else
			{
				/* Need to determine what the fallback transformation, if any, is.  Can
				   be either 7 parameter or Molodensky. */
				flag = (srcDatum->delta_X != 0.0) || (srcDatum->delta_Y != 0.0) || (srcDatum->delta_Z != 0.0);
				if (!flag)
				{
					mreg->fallback = dtcTypNone;
				}
				else
				{
					/* There is enough information to do a Molodensky anyway.  Probably
					   should be doing a three parameter, but we did Molodensky in
					   prior releases, so we'll use it now. */
					mreg->fallback = dtcTypMolodensky;
					flag = (srcDatum->rot_X != 0.0) || (srcDatum->rot_Y != 0.0) || (srcDatum->rot_Z != 0.0);
					if (flag)
					{
						mreg->fallback = dtcTypSixParm;
						if (srcDatum->bwscale != 0.0)
						{
							mreg->fallback = dtcTypSevenParm;
						}
					}
				}

				/* Do what is required to support the selected fallback position.
				   Note, that the target datum of WGS84 is always implied on these
				   things. */
				switch (mreg->fallback) {
				case dtcTypMolodensky:
					mreg->fallbackXfrm.molo = CS_moInit (srcDatum,cs_Wgs84Ptr);
					break;
				case dtcTypSixParm:
					mreg->fallbackXfrm.parm6 = CS_6pInit (srcDatum,cs_Wgs84Ptr);
					break;
				case dtcTypSevenParm:
					mreg->fallbackXfrm.parm7 = CS_7pInit (srcDatum,cs_Wgs84Ptr);
					break;
				case dtcTypNone:
					break;
				default:
					mreg->fallback = dtcTypNone;
					break;
				}
			}
		}
	}
	return mreg;
}

/**********************************************************************
	status = CS_dmaMr3dFowrd (trgLl,srcLl,mreg);

	double trgLl [3];			the results are returned here.  Hgt is
								in meters.
	double srcLl [3];			the source coordinates to be converted.
								Longitude and latitude are in degrees,
								Height element MUST be in meters.
	struct cs_dmaMReg_ *mreg;	pointer to the structure which defines the
								multiple regression coefficients to be used
								in the calculation.  Note these are DMA
								specific in this case.
	int status;					returns 0 on success; 1 if the input
								coordinates are outside the range of
								the transformation definition.

	The trgLl and srcLl arguments may point to the same array.  This
	function has been generalized to support any source or target datum.
	However, at the time of writing, converting from local geodetic
	reference systems to WGS84 are the only conversions for which
	parameters have been published.
*/
int EXP_LVL7 CS_dmaMr3dFowrd (double trgLl [3],Const double srcLl [3],Const struct cs_DmaMReg_ *mRegPtr)
{
	extern double cs_One;
	extern double cs_Sec2Deg;

	/* We assume that the parameterized lat/lng are normalized
	   coordinates, and that the useful range of the multiple
	   regression is defined by these values.  However, we have
	   found that in some cases, the DMA provided test cases
	   do not always fall within that range,  So, we use
	   1.401 to specify the useful range of the transformation. */
	static double test_val = 1.401;

	int ii;
	int jj;
	int wrd;
	int status;
	unsigned int bitNbr;

	int lngIdx;
	int latIdx;
	int hgtIdx;

	ulong32_t mask;

	double lngSum;
	double latSum;
	double hgtSum;

	double uu, uu_pwr = 0.0;		// initialization to keep gcc compiler happy
	double vv, vv_pwr = 0.0;		// initialization to keep gcc compiler happy

	double myLl [3];

	Const struct csDmaMReg_ *mreg;

	/* The default result is no conversion.  We also get a copy of the
	   source coordinates in an array which we can modify. */
	myLl [LNG] = trgLl [LNG] = srcLl [LNG];
	myLl [LAT] = trgLl [LAT] = srcLl [LAT];
	myLl [HGT] = trgLl [HGT] = srcLl [HGT];

	/* Get a pointer to the multiple regression portion of the structure,
	   as opposed to that thing that includes the fall back stuff. */
	mreg = &mRegPtr->mReg;

	/* Compute the normalized input coordinates, uu and vv. */
	uu = (myLl [LAT] + mreg->uu_off) * mreg->kk;
	vv = (myLl [LNG] + mreg->vv_off) * mreg->kk;

	/* Make sure the values are within the range of the multiple rgression
	   formulas. */
	if (fabs (uu) > test_val || fabs (vv) > test_val)
	{
		/* If the input is out of range.  If there is a fallback position,
		   we use it now.  Otherwise, we simply return the original
		   coordinates.  In any case, we return a 1 to indicate that
		   something out of the ordinary happened.
		   
		   All fallbacks are forward transformations. */

		CS_erpt (cs_MREG_RANGE);			/* Register warning. */
		switch (mRegPtr->fallback) {
		case dtcTypMolodensky:
			status = CS_mo3dFowrd (trgLl,srcLl,mRegPtr->fallbackXfrm.molo);
			break;
		case dtcTypSixParm:
			status = CS_6p3dFowrd (trgLl,srcLl,mRegPtr->fallbackXfrm.parm6);
			break;
		case dtcTypSevenParm:
			status = CS_7p3dFowrd (trgLl,srcLl,mRegPtr->fallbackXfrm.parm7);
			break;
		case dtcTypNone:
		default:
			/* If there is no fallback, return a hard error. */
			CS_erpt (cs_MREG_RANGEF);		/* Fatal message */
			status = -1;
			break;
		}
		/* +1 status says we used a fallback calculation. */
		return (status < 0) ? -1 : 1;
	}

	/* Set up the indices into the coefficients structure. */
	lngIdx = 0;
	latIdx = mreg->lat_idx;
	hgtIdx = mreg->hgt_idx;

	/* Initialize the variables in which we will accumulate
	   the delta values. */
	lngSum = latSum = hgtSum = 0.0;

	/* Start the loops necessary to perform the calculation. */
	for (ii = 0;ii < mreg->max_vv;ii++)
	{
		/* Compute the necessary uu power term for this
		   iteration of the loop. */
		if (ii == 0) vv_pwr  = cs_One;
		else	     vv_pwr *= vv;						/*lint !e644 */
		for (jj = 0;jj < mreg->max_uu;jj++)
		{
			/* Compute the necessary vv power term for this
			   iteration of the loop. */
			if (jj == 0) uu_pwr  = cs_One;
			else	     uu_pwr *= uu;					/*lint !e644 */

			/* Compute the bit map bit number for this
			   iteration for both loops. */
			bitNbr = (unsigned int)(ii * 10 + jj);
			wrd = bitNbr >> 5;
			mask = (ulong32_t)0x80000000L >> (bitNbr & 0x1F);

			/* See if this term is in the longitude calculation. */
			if (mreg->lng_map [wrd] & mask)
			{
				/* Yes it is, compute the value. */
				lngSum += mreg->coefs [lngIdx++] * uu_pwr * vv_pwr;
			}

			/* Same for the latitude. */
			if (mreg->lat_map [wrd] & mask)
			{
				latSum += mreg->coefs [latIdx++] * uu_pwr * vv_pwr;
			}

			/* Same for the height. */
			if (mreg->hgt_map [wrd] & mask)
			{
				hgtSum += mreg->coefs [hgtIdx++] * uu_pwr * vv_pwr;
			}
		}
	}

	/* Compute the final results. */
	trgLl [LNG] = myLl [LNG] + lngSum * cs_Sec2Deg;
	trgLl [LAT] = myLl [LAT] + latSum * cs_Sec2Deg;
	trgLl [HGT] = myLl [HGT] + hgtSum;

	/* That's it. */
	return 0;
}

/**********************************************************************
	status = CS_dmaMr2dFowrd (trgLl,srcLl,mreg);

	double trgLl [3];			the results are returned here.  Hgt is
								copied from the srcLl argument.
	double srcLl [3];			the source coordinates to be converted.
								Longitude and latitude are in degrees,
								Height element is not involved in the
								calculations, an ellipsoid height of
								zero is assumed.
	struct cs_dmaMReg_ *mreg;	pointer to the structure which defines the
								multiple regression coefficients to be used
								in the calculation.  Note these are DMA
								specific in this case.
	int status;					returns 0 on success; returns the status
								returned by CS_dmaMr3dFowrd, which is +1
								if the input coordinates are outside the
								range of the transformation definition.

	The trgLl and srcLl arguments may point to the same array.  This
	function has been generalized to support any source or target datum.
	However, at the time of writing, converting from local geodetic
	reference systems to WGS84 are the only conversions for which
	parameters have been published.
*/
int EXP_LVL7 CS_dmaMr2dFowrd (double trgLl [3],Const double srcLl [3],Const struct cs_DmaMReg_ *mreg)
{
	extern double cs_Zero;

	int status;
	double lcl_ll [3];

	trgLl [LNG] = lcl_ll [LNG] = srcLl [LNG];
	trgLl [LAT] = lcl_ll [LAT] = srcLl [LAT];
	trgLl [HGT] = srcLl [HGT];

	lcl_ll [HGT] = cs_Zero;
	status = CS_dmaMr3dFowrd (lcl_ll,lcl_ll,mreg);
	if (status >= 0)
	{
		trgLl [LNG] = lcl_ll [LNG];
		trgLl [LAT] = lcl_ll [LAT];
	}
	return status;
}

/**********************************************************************
	status = CS_dmaMr3dInvrs (trgLl,srcLl,mreg);

	double trgLl [3];			the results are returned here.  Hgt is
								in meters.
	double srcLl [3];			the source coordinates to be converted.
								Longitude and latitude are in degrees,
								height MUST be in meters.
	struct cs_dmaMReg_ *mreg;	pointer to the structure which defines the
								multiple regression coefficients to be used
								in the calculation.  Note these are DMA
								specific in this case.
	int status;					returns 0 on success; 1 if the iterative
								calculation did not converge in 8 tries,
								but it looked like it was indeed converging.
								A +2 is returned if the coordinate was outside
								the usful range of the multiple regression
								definition and a fallback was used. A -1
								if the iterative calculation failed to
								converge, or if the data was outside the
								usful range of the definition and no
								fallback was specified, or the fallback
								itself failed.

	The trgLl and srcLl arguments may point to the same array. See
	the CS_dmaMr3dFowrd function above for more information.

	Computing the actual inverse directly is very difficult, especially
	in a general manner.  Since these transformations are datum shifts,
	i.e. small changes in the actual coordinates, and these datum shifts
	are somewhat rational, we simply iterate through the forward
	transformation.

	Note that we use the longitude and latitude exclusively.  If we used
	the height also, things would probably not converge.  Unlike most of
	the other datum calculation techniques, however, the horizontal and
	vertical components of this calculation are completely independent
	of each other.  That is, supplying a different height value will
	not affect the horizontal calculation.
*/
int EXP_LVL7 CS_dmaMr3dInvrs (double trgLl [3],Const double srcLl [3],Const struct cs_DmaMReg_ *mreg)
{
	extern double cs_Zero;					/* 0.0 */

	static int maxIteration = 8;
	static double smallValue  = 1.0E-09;		/* equates to =~ .1 millimeters */
	static double smallValue2 = 1.0E-06;		/* equates to =~ 100 millimeters */

	short lngOk;
	short latOk;

	int ii;
	int status;
	int rtnVal;

	double guess [3];
	double newLl [3];
	double epsilon [3];

	/* Assume everything goes OK until we know different. */
	rtnVal = 0;

	/* First, we copy the WGS-84 lat/longs to the local array.
	   This is the default result which the user may want in
	   the event of an error.  Besides, we assume such has been
	   done below, even if there has not been an error. */
	guess [LNG] = trgLl [LNG] = srcLl [LNG];
	guess [LAT] = trgLl [LAT] = srcLl [LAT];
	trgLl [HGT] = srcLl [HGT];

	/* We deal with height in a different manner. */
	guess [HGT] = cs_Zero;

	/* Start a loop which will iterate as many as maxIteration times. */
	for (ii = 0;ii < maxIteration;ii++)
	{
		/* Assume we are done until we know different. */
		lngOk = latOk = TRUE;

		/* Compute the WGS-84 lat/long for our current guess. */
		status = CS_dmaMr3dFowrd (newLl,guess,mreg);
		if (status > 0) rtnVal = 2;			/* Fallback used. */
		else if (status < 0)
		{
			/* Fatal problem, usually outside the range with no
			   fallback.  Return fatal status. */
			rtnVal = -1;
			break;
		}

		/* See how far we are off.  Note, we use the latitude and
		   longitude only.  Otherwise, we would never really converge. */
		epsilon [LNG] = srcLl [LNG] - newLl [LNG];
		epsilon [LAT] = srcLl [LAT] - newLl [LAT];

		/* If our guess at the longitude is off by more than
		   small, we adjust our guess by the amount we are off. */
		if (fabs (epsilon [LNG]) > smallValue)
		{
			lngOk = FALSE;
			guess [LNG] += epsilon [LNG];
		}
		/* If our guess longitude is off by more than
		   small, we adjust our guess by the amount we are off. */
		if (fabs (epsilon [LAT]) > smallValue)
		{
			latOk = FALSE;
			guess [LAT] += epsilon [LAT];
		}

		/* If our current guess produces a newLl that is within
		   samllValue of srcLl, we are done. */
		if (lngOk && latOk) break;
	}

	/* If we didn't resolve in eight tries, we issue a warning message. */
	if (ii >= maxIteration )
	{
		CS_erpt (cs_WGS_CNVRG);

		/* Issue a warning if we're close, a fatal if we are still way off.
		   In any case, we return the last computed value.  We could have
		   gotten very fancy with this stuff, but it would have had serious
		   affects on the performance.  So we just check epsilon here as
		   we know we have an error and this doesn't happen very often. */
		rtnVal = 1;
		if (fabs (epsilon [LNG]) > smallValue2 ||
		    fabs (epsilon [LAT]) > smallValue2)
		{
			rtnVal = -1;
		}
	}

	/* Adjust the ll_lcl value to the computed value, now that we
	   know that it should be correct. */
	if (rtnVal >= 0)
	{
		trgLl [LNG] = guess [LNG];
		trgLl [LAT] = guess [LAT];
		/* The iterative forward calculations above are all done with a height
		   value of zero.  Thus, the height returned is essentially a delta
		   height. */
		trgLl [HGT] = srcLl [HGT] - guess [HGT];
	}

	return rtnVal;
}

/**********************************************************************
	status = CS_dmaMr2dInvrs (trgLl,srcLl,mreg);

	double trgLl [3];			the results are returned here.  Hgt
								element is copied from the srcLl
								argument.
	double srcLl [3];			the source coordinates to be converted.
								Longitude and latitude are in degrees,
								height MUST be in meters.  Height
								element is not involved in the
								calculations.
	struct cs_dmaMReg_ *mreg;	pointer to the structure which defines the
								multiple regression coefficients to be used
								in the calculation.  Note these are DMA
								specific in this case.
	int status;					returns 0 on success; 1 if the iterative
								calculation failed to converge, but it
								appeared to be converging, 2 if the data
								was outside the usful range of the definition,
								and a fallback was used, and -1 if the
								iterative calculation failed to converge.

	The trgLl and srcLl arguments may point to the same array. See
	the CS_dmaMr3DInvrs function above for more information.
*/
int EXP_LVL7 CS_dmaMr2dInvrs (double trgLl [3],Const double srcLl [3],Const struct cs_DmaMReg_ *mreg)
{
	extern double cs_Zero;					/* 0.0 */

	int status;
	double lcl_ll [3];

	trgLl [LNG] = lcl_ll [LNG] = srcLl [LNG];
	trgLl [LAT] = lcl_ll [LAT] = srcLl [LAT];
	trgLl [HGT] = srcLl [HGT];

	lcl_ll [HGT] = cs_Zero;
	status = CS_dmaMr3dInvrs (lcl_ll,lcl_ll,mreg);
	if (status >= 0)
	{
		trgLl [LNG] = lcl_ll [LNG];
		trgLl [LAT] = lcl_ll [LAT];
	}
	return status;
}

/******************************************************************************
	parm4 = CS_4pInit (srcDatum,trgDatum);

	struct cs_Datum_ *srcDatum;	pointer to the definition of the source datum.
	struct cs_Datum_ *trgDatuu; pointer to the definition of the target datum.
	struct cs_Parm4_ *parm3;	returns a pointer to an initialized parameter
								structure which is the required argument for
								CS_p4??Forwd and CS_p4??Invrs.

	The function returns null in the event of an error. Memory allocation
	failure is the principal cause of failure.
*/
struct cs_Parm4_ * EXP_LVL9 CS_4pInit (Const struct cs_Datum_* srcDatum,
									   Const struct cs_Datum_* trgDatum)
{
	extern double cs_One;				/* 1.0 */
	struct cs_Parm4_ *parm4;
	double trgScale;

	parm4 = (struct cs_Parm4_ *)CS_malc (sizeof (struct cs_Parm4_));
	if (parm4 == NULL)
	{
		CS_erpt (cs_NO_MEM);
	}
	else
	{
		parm4->srcERad = srcDatum->e_rad;
		parm4->srcESqr = srcDatum->ecent * srcDatum->ecent;
		CS_stncp (parm4->srcKeyName,srcDatum->key_nm,sizeof (parm4->srcKeyName));

		parm4->trgERad = trgDatum->e_rad;
		parm4->trgESqr = trgDatum->ecent * trgDatum->ecent;
		CS_stncp (parm4->trgKeyName,trgDatum->key_nm,sizeof (parm4->trgKeyName));

		parm4->delta_X = srcDatum->delta_X - trgDatum->delta_X;
		parm4->delta_Y = srcDatum->delta_Y - trgDatum->delta_Y;
		parm4->delta_Z = srcDatum->delta_Z - trgDatum->delta_Z;

		trgScale = cs_One + (trgDatum->bwscale * 1.0E-06);
		if (fabs (trgScale) < 1.0E-06)
		{
			CS_free (parm4);
			parm4 = NULL;
			CS_erpt (cs_ISER);
		}
		else
		{
			parm4->scale = (cs_One + (srcDatum->bwscale * 1.0E-06)) / trgScale;
		}
	}
	return parm4;
}

/**********************************************************************
	st = CS_4p3dFowrd (trgLl,srcLl,parm4);

	double trgLl [3];			the converted results are returned here.
								Lng/Lat are in degrees, Hgt is usually meters.
								Hgt is considered to be ellipsoid height, and
								must be in the units of the equatorial radius
								in the parm4 structure, usually meters.
	double srcLl [3];			the coordinate to be converted.  Elements have
								the same characteristics as described above.
	struct cs_Parm4_ *parm4;	pointer to an initialized structure defining
								the datum conversion to be performed.
	int st;						returns zero to indicate success, -1 on
								failure.  Only cause for failure is the
								convergence failure of geocentric inverse.

	This is the standard Four Parameter Transformation.
*/
int EXP_LVL9 CS_4p3dFowrd (double trgLl [3],Const double srcLl [3],Const struct cs_Parm4_ *parm4)
{
	int status;
	double xyz [3];

	/* Convert the geographic coordinates to geocentric XYZ coordinates. */
	CS_llhToXyz (xyz,srcLl,parm4->srcERad,parm4->srcESqr);

	/* Adjust these cartesian coordinates via the Bursa/Wolf transformation. */
	xyz [XX] = xyz [XX] * parm4->scale + parm4->delta_X;
	xyz [YY] = xyz [YY] * parm4->scale + parm4->delta_Y;
	xyz [ZZ] = xyz [ZZ] * parm4->scale + parm4->delta_Z;

	/* Convert the new X, Y, and Z back to latitude and longitude.
	   CS_xyzToLlh returns degrees. */
	status = CS_xyzToLlh (trgLl,xyz,parm4->trgERad,parm4->trgESqr);

	/* That's that. */
	return status;
}

/**********************************************************************
	st = CS_4p2dFowrd (trgLl,srcLl,parm4);

	double trgLl [3];			the converted results are returned here.
								Lng/Lat are in degrees, Hgt element is
								copied from the srcLl array.
	double srcLl [3];			the coordinate to be converted.  Hgt
								element is not involved in the calculation,
								an ellipsoidal height of zero is assumed.
	struct cs_Parm4_ *parm4;	pointer to an initialized structure defining
								the datum conversion to be performed.
	int st;						returns zero to indicate success, -1 on
								failure.  Only cause for failure is the
								convergence failure of geocentric inverse.

	See CS_4p2dFowrd for more information.
*/
int EXP_LVL9 CS_4p2dFowrd (double trgLl [3],Const double srcLl [3],Const struct cs_Parm4_ *parm4)
{
	extern double cs_Zero;			/* 0.0 */

	int status;
	double lcl_ll [3];

	trgLl [LNG] = lcl_ll [LNG] = srcLl [LNG];
	trgLl [LAT] = lcl_ll [LAT] = srcLl [LAT];
	trgLl [HGT] = srcLl [HGT];

	lcl_ll [LNG] = cs_Zero;
	status = CS_4p3dFowrd (lcl_ll,lcl_ll,parm4);
	if (status >= 0)
	{
		trgLl [LNG] = lcl_ll [LNG];
		trgLl [LAT] = lcl_ll [LAT];
	}
	return status;
}

/**********************************************************************
	st = CS_4p3dInvrs (trgLl,srcLl,parm4);

	double trgLl [3];			the converted results are returned here.
								Lng/Lat are in degrees, Hgt is usually meters.
								Hgt is considered to be ellipsoid height, and
								must be in the units of the equatorial radius
								in the parm3 structure.
	double srcLl [3];			the coordinate to be converted.  Elements have
								the same characteristics as described above.
	struct cs_Parm4_ *parm4;	pointer to an initialized structure defining
								the datum conversion to be performed.
	int st;						returns zero to indicate success, -1 on
								failure.  Only cause for failure is the
								convergence failure of geocentric inverse.

	This is the inverse of the Three Parameter Transformation.  See the
	forward function, above, for more information.  The inverse of this
	transformation is usually accomplished by inverting the parameters
	to the initialization function.  Therefore, this function is not
	used in the normal operation of CS_MAP.
*/
int EXP_LVL9 CS_4p3dInvrs (double trgLl [3],Const double srcLl [3],Const struct cs_Parm4_ *parm4)
{
	int status;
	double xyz [3];

	/* Convert the geographic coordinates to geocentric XYZ coordinates. */
	CS_llhToXyz (xyz,srcLl,parm4->trgERad,parm4->trgESqr);

	/* Apply the translation parameters. */
	xyz [XX] = (xyz [XX] - parm4->delta_X) / parm4->scale;
	xyz [YY] = (xyz [YY] - parm4->delta_Y) / parm4->scale;
	xyz [ZZ] = (xyz [ZZ] - parm4->delta_Z) / parm4->scale;

	/* Convert the new X, Y, and Z back to latitude and longitude.
	   CS_xyzToLlh returns degrees. */
	status = CS_xyzToLlh (trgLl,xyz,parm4->srcERad,parm4->srcESqr);

	/* That's that. */
	return status;
}

/**********************************************************************
	st = CS_4p2dInvrs (trgLl,srcLl,parm3);

	double trgLl [3];			the converted results are returned here.
								Lng/Lat are in degrees, Hgt is copied from
								the srcLl argument.
	double srcLl [3];			the coordinate to be converted. Hgt element
								is not involved in the calculation; an
								ellipsoid hieght of zero is assumed.
	struct cs_Parm4_ *parm4;	pointer to an initialized structure defining
								the datum conversion to be performed.
	int st;						returns zero to indicate success, -1 on
								failure.  Only cause for failure is the
								convergence failure of geodetic or
								4 parameter iterative calculations.

	This function is an iterative inverse of CS_4p3dFowrd.  This is
	required to prevent positional creep in coordinates.  See
	CS_4p3dFowrd and CS_4p3dInvrs for more information.
*/
int EXP_LVL9 CS_4p2dInvrs (double trgLl [3],Const double srcLl [3],Const struct cs_Parm4_ *parm4)
{
	static int maxIteration = 8;
	static double smallValue  = 1.0E-09;		/* equates to =~ .1 millimeters */
	static double smallValue2 = 1.0E-06;		/* equates to =~ 100 millimeters */

	int ii;
	int lngOk;
	int latOk;
	int rtnVal;

	double guess [3];
	double newLl [3];
	double epsilon [3];

	/* Assume everything goes OK until we know different. */
	rtnVal = 0;

	/* First, we copy the source lat/longs to the local array.
	   This is the default result which the user may want in
	   the event of an error.  Note, we assume such has been
	   done below, even if there has not been an error. */
	trgLl [LNG] = guess [LNG] = srcLl [LNG];
	trgLl [LAT] = guess [LAT] = srcLl [LAT];
	trgLl [HGT] = srcLl [HGT];

	/* To improve performancce, we could use CS_4p3dInvrs here to
	   get our guess.  That function will probably reduce the
	   number of iterations require below substantially. */

	/* Start a loop which will iterate as many as maxIteration times. */
	for (ii = 0;ii < maxIteration;ii++)
	{
		/* Assume we are done until we know different. */
		lngOk = latOk = TRUE;

		/* Compute the WGS-84 lat/long for our current guess. */
		rtnVal = CS_4p2dFowrd (newLl,guess,parm4);
		if (rtnVal != 0)
		{
			/* Oopps!! We must have been given some pretty strange
			   coordinate values. */
			break;
		}

		/* See how far we are off. */
		epsilon [LNG] = srcLl [LNG] - newLl [LNG];
		epsilon [LAT] = srcLl [LAT] - newLl [LAT];

		/* If our guess at the longitude is off by more than
		   small, we adjust our guess by the amount we are off. */
		if (fabs (epsilon [LNG]) > smallValue)
		{
			lngOk = FALSE;
			guess [LNG] += epsilon [LNG];
		}
		/* If our guess longitude is off by more than
		   small, we adjust our guess by the amount we are off. */
		if (fabs (epsilon [LAT]) > smallValue)
		{
			latOk = FALSE;
			guess [LAT] += epsilon [LAT];
		}

		/* If our current guess produces a newLl that is within
		   samllValue of srcLl, we are done. */
		if (lngOk && latOk) break;
	}

	/* If we didn't resolve in maxIteration tries, we issue a warning
	   message.  Usually, three or four iterations does the trick. */
	if (ii >= maxIteration )
	{
		CS_erpt (cs_4P_CNVRG);

		/* Issue a warning if we're close, a fatal if we are still way off.
		   In any case, we return the last computed value.  We could have
		   gotten very fancy with this stuff, but it would have had serious
		   affects on the performance.  So we just check epsilon here as
		   we know we have an error and this doesn't happen very often. */
		rtnVal = 1;
		if (fabs (epsilon [LNG]) > smallValue2 ||
		    fabs (epsilon [LAT]) > smallValue2)
		{
			rtnVal = -1;
		}
	}

	/* Adjust the ll_lcl value to the computed value, now that we
	   know that it should be correct. */
	if (rtnVal >= 0)
	{
		trgLl [LNG] = guess [LNG];
		trgLl [LAT] = guess [LAT];
	}
	return rtnVal;
}
/******************************************************************************
	The following are hooks for possible future implementation of a difference
	between GDA94 and WGS84.
*/
int EXP_LVL9 CSgda94ToWgs84 (double ll_wgs84 [3],Const double ll_gda94 [3])
{
	ll_wgs84 [LNG] = ll_gda94 [LNG];
	ll_wgs84 [LAT] = ll_gda94 [LAT];
	ll_wgs84 [HGT] = ll_gda94 [HGT];

	/* This is always successful (currently). */
	return 0;
}
int EXP_LVL9 CSwgs84ToGda94 (double ll_gda94 [3],Const double ll_wgs84 [3])
{
	ll_gda94 [LNG] = ll_wgs84 [LNG];
	ll_gda94 [LAT] = ll_wgs84 [LAT];
	ll_gda94 [HGT] = ll_wgs84 [HGT];

	/* This is always successful (currently). */
	return 0;
}
/******************************************************************************
	The following are hooks for possible future implementation of a difference
	between ETRF89 and WGS84.
*/
int EXP_LVL9 CSetrf89ToWgs84 (double ll_wgs84 [3],Const double ll_etrf89 [3])
{
	ll_wgs84 [LNG] = ll_etrf89 [LNG];
	ll_wgs84 [LAT] = ll_etrf89 [LAT];
	ll_wgs84 [HGT] = ll_etrf89 [HGT];

	/* This is always successful (currently). */
	return 0;
}
int EXP_LVL9 CSwgs84ToEtrf89 (double ll_etrf89 [3],Const double ll_wgs84 [3])
{
	ll_etrf89 [LNG] = ll_wgs84 [LNG];
	ll_etrf89 [LAT] = ll_wgs84 [LAT];
	ll_etrf89 [HGT] = ll_wgs84 [HGT];

	/* This is always successful (currently). */
	return 0;
}

/******************************************************************************
	parm3 = CS_3pInit (srcDatum,trgDatum);

	struct cs_Datum_ *srcDatum;	pointer to the definition of the source datum.
	struct cs_Datum_ *trgDatuu; pointer to the definition of the target datum.
	struct cs_Parm3_ *parm3;	returns a pointer to an initialized parameter
								structure which is the required argument for
								CS_p3Forwd and CS_p3Invrs.

	The function returns null in the event of an error. Memory allocation
	failure is the principal cause of failure.
*/
struct cs_Parm3_ * EXP_LVL9 CS_3pInit (Const struct cs_Datum_* srcDatum,
									   Const struct cs_Datum_* trgDatum)
{
	struct cs_Parm3_ *parm3;

	parm3 = (struct cs_Parm3_ *)CS_malc (sizeof (struct cs_Parm3_));
	if (parm3 == NULL)
	{
		CS_erpt (cs_NO_MEM);
	}
	else
	{
		parm3->srcERad = srcDatum->e_rad;
		parm3->srcESqr = srcDatum->ecent * trgDatum->ecent;
		CS_stncp (parm3->srcKeyName,srcDatum->key_nm,sizeof (parm3->srcKeyName));

		parm3->trgERad = trgDatum->e_rad;
		parm3->trgESqr = trgDatum->ecent * trgDatum->ecent;
		CS_stncp (parm3->trgKeyName,trgDatum->key_nm,sizeof (parm3->trgKeyName));

		parm3->delta_X = srcDatum->delta_X - trgDatum->delta_X;
		parm3->delta_Y = srcDatum->delta_Y - trgDatum->delta_Y;
		parm3->delta_Z = srcDatum->delta_Z - trgDatum->delta_Z;
	}
	return parm3;
}

/**********************************************************************
	st = CS_3p3dFowrd (trgLl,srcLl,parm3);

	double trgLl [3];			the converted results are returned here.
								Lng/Lat are in degrees, Hgt is usually meters.
								Hgt is considered to be ellipsoid height, and
								must be in the units of the equatorial radius
								in the parm3 structure, usually meters.
	double srcLl [3];			the coordinate to be converted.  Elements have
								the same characteristics as described above.
	struct cs_Parm3_ *parm3;	pointer to an initialized structure defining
								the datum conversion to be performed.
	int st;						returns zero to indicate success, -1 on
								failure.  Only cause for failure is the
								convergence failure of geocentric inverse.

	This is the standard Three Parameter Transformation.  What the DMA calls
	Molodensky is actually a different formulation of this transformation.
*/
int EXP_LVL9 CS_3p3dFowrd (double trgLl [3],Const double srcLl [3],Const struct cs_Parm3_ *parm3)
{
	int status;
	double xyz [3];

	/* Convert the geographic coordinates to geocentric XYZ coordinates. */
	CS_llhToXyz (xyz,srcLl,parm3->srcERad,parm3->srcESqr);

	/* Adjust these cartesian coordinates via the Bursa/Wolf transformation. */
	xyz [XX] += parm3->delta_X;
	xyz [YY] += parm3->delta_Y;
	xyz [ZZ] += parm3->delta_Z;

	/* Convert the new X, Y, and Z back to latitude and longitude.
	   CS_xyzToLlh returns degrees. */
	status = CS_xyzToLlh (trgLl,xyz,parm3->trgERad,parm3->trgESqr);

	/* That's that. */
	return status;
}

/**********************************************************************
	st = CS_3p2dFowrd (trgLl,srcLl,parm3);

	double trgLl [3];			the converted results are returned here.
								Lng/Lat are in degrees, Hgt element is
								copied from the srcLl array.
	double srcLl [3];			the coordinate to be converted.  Hgt
								element is not involved in the calculation,
								an ellipsoidal height of zero is assumed.
	struct cs_Parm3_ *parm3;	pointer to an initialized structure defining
								the datum conversion to be performed.
	int st;						returns zero to indicate success, -1 on
								failure.  Only cause for failure is the
								convergence failure of geocentric inverse.

	See CS_3p2dFowrd for more information.
*/
int EXP_LVL9 CS_3p2dFowrd (double trgLl [3],Const double srcLl [3],Const struct cs_Parm3_ *parm3)
{
	extern double cs_Zero;			/* 0.0 */

	int status;
	double lcl_ll [3];

	trgLl [LNG] = lcl_ll [LNG] = srcLl [LNG];
	trgLl [LAT] = lcl_ll [LAT] = srcLl [LAT];
	trgLl [HGT] = srcLl [HGT];

	lcl_ll [HGT] = cs_Zero;
	status = CS_3p3dFowrd (lcl_ll,lcl_ll,parm3);
	if (status >= 0)
	{
		trgLl [LNG] = lcl_ll [LNG];
		trgLl [LAT] = lcl_ll [LAT];
	}
	return status;
}

/**********************************************************************
	st = CS_3p3dInvrs (trgLl,srcLl,parm3);

	double trgLl [3];			the converted results are returned here.
								Lng/Lat are in degrees, Hgt is usually meters.
								Hgt is considered to be ellipsoid height, and
								must be in the units of the equatorial radius
								in the parm3 structure.
	double srcLl [3];			the coordinate to be converted.  Elements have
								the same characteristics as described above.
	struct cs_Parm3_ *parm3;	pointer to an initialized structure defining
								the datum conversion to be performed.
	int st;						returns zero to indicate success, -1 on
								failure.  Only cause for failure is the
								convergence failure of geocentric inverse.

	This is the inverse of the Three Parameter Transformation.  See the
	forward function, above, for more information.  The inverse of this
	transformation is usually accomplished by inverting the parameters
	to the initialization function.  Therefore, this function is not
	used in the normal operation of CS_MAP.
*/
int EXP_LVL9 CS_3p3dInvrs (double trgLl [3],Const double srcLl [3],Const struct cs_Parm3_ *parm3)
{
	int status;
	double xyz [3];

	/* Convert the geographic coordinates to geocentric XYZ coordinates. */
	CS_llhToXyz (xyz,srcLl,parm3->trgERad,parm3->trgESqr);

	/* Apply the translation parameters. */
	xyz [XX] -= parm3->delta_X;
	xyz [YY] -= parm3->delta_Y;
	xyz [ZZ] -= parm3->delta_Z;

	/* Convert the new X, Y, and Z back to latitude and longitude.
	   CS_xyzToLlh returns degrees. */
	status = CS_xyzToLlh (trgLl,xyz,parm3->srcERad,parm3->srcESqr);

	/* That's that. */
	return status;
}

/**********************************************************************
	st = CS_3p2dInvrs (trgLl,srcLl,parm3);

	double trgLl [3];			the converted results are returned here.
								Lng/Lat are in degrees, Hgt is copied from
								the srcLl argument.
	double srcLl [3];			the coordinate to be converted. Hgt element
								is not involved in the calculation; an
								ellipsoid hieght of zero is assumed.
	struct cs_Parm3_ *parm3;	pointer to an initialized structure defining
								the datum conversion to be performed.
	int st;						returns zero to indicate success, -1 on
								failure.  Only cause for failure is the
								convergence failure of geodetic or
								3 parameter iterative calculations.

	This function is an iterative inverse of CS_3p3dFowrd.  This is
	required to prevent positional creep in coordinates.  See
	CS_3p3dFowrd and CS_3p3dInvrs for more information.
*/
int EXP_LVL9 CS_3p2dInvrs (double trgLl [3],Const double srcLl [3],Const struct cs_Parm3_ *parm3)
{
	static int maxIteration = 8;
	static double smallValue  = 1.0E-09;		/* equates to =~ .1 millimeters */
	static double smallValue2 = 1.0E-06;		/* equates to =~ 100 millimeters */

	int ii;
	int lngOk;
	int latOk;
	int rtnVal;

	double guess [3];
	double newLl [3];
	double epsilon [3];

	/* Assume everything goes OK until we know different. */
	rtnVal = 0;

	/* First, we copy the source lat/longs to the local array.
	   This is the default result which the user may want in
	   the event of an error.  Note, we assume such has been
	   done below, even if there has not been an error. */
	trgLl [LNG] = guess [LNG] = srcLl [LNG];
	trgLl [LAT] = guess [LAT] = srcLl [LAT];
	trgLl [HGT] = srcLl [HGT];

	/* To improve performancce, we could use CS_3p3dInvrs here to
	   get our guess.  That function will probably reduce the
	   number of iterations require below substantially. */

	/* Start a loop which will iterate as many as maxIteration times. */
	for (ii = 0;ii < maxIteration;ii++)
	{
		/* Assume we are done until we know different. */
		lngOk = latOk = TRUE;

		/* Compute the WGS-84 lat/long for our current guess. */
		rtnVal = CS_3p2dFowrd (newLl,guess,parm3);
		if (rtnVal != 0)
		{
			/* Oopps!! We must have been given some pretty strange
			   coordinate values. */
			break;
		}

		/* See how far we are off. */
		epsilon [LNG] = srcLl [LNG] - newLl [LNG];
		epsilon [LAT] = srcLl [LAT] - newLl [LAT];

		/* If our guess at the longitude is off by more than
		   small, we adjust our guess by the amount we are off. */
		if (fabs (epsilon [LNG]) > smallValue)
		{
			lngOk = FALSE;
			guess [LNG] += epsilon [LNG];
		}
		/* If our guess longitude is off by more than
		   small, we adjust our guess by the amount we are off. */
		if (fabs (epsilon [LAT]) > smallValue)
		{
			latOk = FALSE;
			guess [LAT] += epsilon [LAT];
		}

		/* If our current guess produces a newLl that is within
		   samllValue of srcLl, we are done. */
		if (lngOk && latOk) break;
	}

	/* If we didn't resolve in maxIteration tries, we issue a warning
	   message.  Usually, three or four iterations does the trick. */
	if (ii >= maxIteration )
	{
		CS_erpt (cs_3P_CNVRG);

		/* Issue a warning if we're close, a fatal if we are still way off.
		   In any case, we return the last computed value.  We could have
		   gotten very fancy with this stuff, but it would have had serious
		   affects on the performance.  So we just check epsilon here as
		   we know we have an error and this doesn't happen very often. */
		rtnVal = 1;
		if (fabs (epsilon [LNG]) > smallValue2 ||
		    fabs (epsilon [LAT]) > smallValue2)
		{
			rtnVal = -1;
		}
	}

	/* Adjust the trgLl value to the computed value, now that we
	   know that it should be correct. */
	if (rtnVal >= 0)
	{
		trgLl [LNG] = guess [LNG];
		trgLl [LAT] = guess [LAT];
	}
	return rtnVal;
}
