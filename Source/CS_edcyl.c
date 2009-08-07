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

/*lint -esym(613,err_list) */
/*lint -esym(715,prj_code) */

/**********************************************************************
**	err_cnt = CSedcylQ (cs_def,prj_code,err_list,list_sz);
**
**	struct cs_Csdef_ *cs_def;	the coordinate system to be checked.
**	unsigned short prj_code;	currently unused.
**	int err_list [];			an array of integers in which error codes are
**								returned if not NULL.
**	int list_sz;				the size of the array pointed to be err_list,
**								may be zero.
**	int err_cnt;				returns the number of errors detected.
**
**	Set err_list to NULL, and/or list_sz to zero, to simply get
**	a yea or nay status of the definition (i.e. err_cnt != 0).
**
**	All lat/longs in definitions must be referenced to Greennwich,
**	and in the range of greater than -180/-90, and less than or
**	equal to +180/+90.
**********************************************************************/

int EXP_LVL9 CSedcylQ (	Const struct cs_Csdef_ *cs_def,unsigned short prj_code,int err_list [],int list_sz)
{
	extern double cs_MinLng;		/* -180.0 */
	extern double cs_MinLat;		/* - 90.0 */
	extern double cs_MaxLng;		/* +180.0 */
	extern double cs_MaxLat;		/* + 90.0 */

	int err_cnt;

	/* We will return (err_cnt + 1) below. */
	
	err_cnt = -1;
	if (err_list == NULL) list_sz = 0;

	/* Check the Mercator specific stuff. */
	
	if (cs_def->org_lng <= cs_MinLng || cs_def->org_lng > cs_MaxLng)
	{
		if (++err_cnt < list_sz) err_list [err_cnt] = cs_CSQ_ORGLNG;
	}
	if (cs_def->org_lat < cs_MinLat || cs_def->org_lat > cs_MaxLat)
	{
		if (++err_cnt < list_sz) err_list [err_cnt] = cs_CSQ_ORGLAT;
	}

	/* We divide by cos(prj_prm1).  Therefore, either pole is invalid.
	   We can let it get pretty close though.  88 is a rather arbitary
	   limit. */

	if (cs_def->prj_prm1 <= -88.0 || cs_def->prj_prm1 >= 88.0)
	{
		if (++err_cnt < list_sz) err_list [err_cnt] = cs_CSQ_STDLAT;
	}

	/* That's it for Equidistant Cylindrical. */
	
	return (err_cnt + 1);
}

/**********************************************************************
**	CSedcylS (csprm);
**
**	struct cs_Csprm_ *csprm;	structure which defines the parameters
**								in effect for the transformation.
**
**	This projection is supported for the sphere only.  We use
**	the equatorial radius of the related ellipsoid for the
**	radius of the sphere.
**
**********************************************************************/

void EXP_LVL9 CSedcylS (struct cs_Csprm_ *csprm)
{
	extern short cs_QuadMin;			/* -4 */
	extern short cs_QuadMap [];
	extern double cs_Radian;			/*   57.2957...  */
	extern double cs_Degree;			/*    1.0 / 57.2957..  */
	extern double cs_Pi;				/*    3.14.159... */
	extern double cs_Mpi;				/*   -3.14.159... */
	extern double cs_Pi_o_2;			/* Pi over 2 */
	extern double cs_K180;				/*  180.0 */
	extern double cs_Km180;				/* -180.0 */
	extern double cs_K90;				/*   90.0 */
	extern double cs_Km90;				/*  -90.0 */
	extern double cs_NPTest;			/* 0.001 seconds of arc
										   short of the north
										   pole, in radians. */
	extern double cs_SPTest;			/* 0.001 seconds of arc
										   short of the south
										   pole, in radians. */

	struct cs_Edcyl_ *edcyl;

	double tmp;

	edcyl = &csprm->proj_prms.edcyl;

	/* Transfer the necessary arguments to the "edcyl" structure.
	   Notice, the conversion from degrees to radians which is
	   performed in the process. */

	edcyl->org_lng = csprm->csdef.org_lng * cs_Degree;
	edcyl->org_lat = csprm->csdef.org_lat * cs_Degree;
	edcyl->ref_lat = csprm->csdef.prj_prm1 * cs_Degree;
	edcyl->k = csprm->csdef.scale;
	edcyl->x_off = csprm->csdef.x_off;
	edcyl->y_off = csprm->csdef.y_off;
	edcyl->e_rad = csprm->datum.e_rad;
	edcyl->ka = csprm->datum.e_rad * edcyl->k;
	edcyl->quad = cs_QuadMap [csprm->csdef.quad - cs_QuadMin];

	/* There isn't much to do for this one. */

	edcyl->cos_ref_lat = cos (edcyl->ref_lat);
	edcyl->Rcos_ref_lat = edcyl->ka * edcyl->cos_ref_lat;
   	edcyl->one_mm = 0.001 * edcyl->k;

	/* Set up the coordinate checking information.  If the user has
	   specified a useful range, we use it without checking it.
	   Otherwise, we compute what I, the programmer, consider to
	   be the useful range of the projection.  Note, values are in
	   degrees and longitude values are relative to the central
	   meridian. */

	csprm->cent_mer = edcyl->org_lng * cs_Radian;
	if (csprm->csdef.ll_min [LNG] == 0.0 &&
	    csprm->csdef.ll_max [LNG] == 0.0)
	{
		/* User hasn't specified any values.  We're to establish
		   the useful range.  We'll establish some pretty liberal
		   values. */

		csprm->min_ll [LNG] = cs_Km180;
		csprm->max_ll [LNG] = cs_K180;
		csprm->min_ll [LAT] = cs_Km90;
		csprm->max_ll [LAT] = cs_K90;
	}
	else
	{
		/* The definition includes a useful range specification.
		   We use these values without checking.  We expect the
		   user to give us absolute values, and we convert
		   to values relative to the central meridian. */

		csprm->min_ll [LNG] = CS_adj180 (csprm->csdef.ll_min [LNG] - csprm->cent_mer);
		csprm->min_ll [LAT] = csprm->csdef.ll_min [LAT];
		csprm->max_ll [LNG] = CS_adj180 (csprm->csdef.ll_max [LNG] - csprm->cent_mer);
		csprm->max_ll [LAT] = csprm->csdef.ll_max [LAT];
	}

	/* Similarly with the X's and Y's.  If the coordinate system
	   definition carries some values, we use them.  If not, we
	   calculate some appropriate values. */

	if (csprm->csdef.xy_min [XX] == 0.0 &&
	    csprm->csdef.xy_max [XX] == 0.0)
	{
		/* No specification in the coordinate system definition. */

		csprm->min_xy [XX] = cs_Mpi * edcyl->Rcos_ref_lat;
		csprm->max_xy [XX] = cs_Pi  * edcyl->Rcos_ref_lat;

		tmp = edcyl->org_lat - cs_Pi_o_2;
		if (tmp < cs_SPTest) tmp = cs_SPTest;
		csprm->min_xy [YY] = tmp * edcyl->ka;

		tmp = edcyl->org_lat + cs_Pi_o_2;
		if (tmp < cs_NPTest) tmp = cs_NPTest;
		csprm->max_xy [YY] = tmp * edcyl->ka;

		CS_quadMM (csprm->min_xy,csprm->max_xy,edcyl->x_off,
											   edcyl->y_off,
											   edcyl->quad);
	}
	else
	{
		/* Use what ever the user has given us.  No adjustment
		   necessary.  Note: we don't check anything. */

		csprm->min_xy [XX] = csprm->csdef.xy_min [XX];
		csprm->min_xy [YY] = csprm->csdef.xy_min [YY];
		csprm->max_xy [XX] = csprm->csdef.xy_max [XX];
		csprm->max_xy [YY] = csprm->csdef.xy_max [YY];
	}

	/* That's all the calculations.  Stuff some function
	   addresses and we are done. */

	csprm->ll2cs    = (cs_LL2CS_CAST)CSedcylF;
	csprm->cs2ll    = (cs_CS2LL_CAST)CSedcylI;
	csprm->cs_scale = (cs_SCALE_CAST)CSedcylK;
	csprm->cs_sclk  = (cs_SCALK_CAST)CSedcylK;
	csprm->cs_sclh  = (cs_SCALH_CAST)CSedcylH;
	csprm->cs_cnvrg = (cs_CNVRG_CAST)CSedcylC;
	csprm->llchk    = (cs_LLCHK_CAST)CSedcylL;
	csprm->xychk    = (cs_XYCHK_CAST)CSedcylX;

	return;
}

/**********************************************************************
**	rtn_val = CSedcylF (edcyl,xy,ll);
**
**	struct cs_Edcyl_ *edcyl;	structure which defines the parameters
**								in effect for the transformation.
**	double xy [2];				the results are returned here, x ([0])
**								and y ([1]).
**	double ll [2];				the longitude ([0]) and latitude ([1]),
**								in degrees, to be converted.
**	int rtn_val;				returns cs_CNVRT_NRML if result is normal;
**								cs_CNVRT_RNG if value to be converted is
**								outside of mathematical scope of the
**								projection.
**
**	The ll and xy arguments of this function may point
**	to the same array with no adverse affects.
**
**	This function calculates the Equidistant Cylindrical
**	projection and returns values in the coordinate system
**	described by the cs_Edcyl_ structure.  For this projection
**	only the spherical earth is supported.  We use the
**	equatorial radius of the ellipsoid provided as the radius
**	of the sphere.
**
**	All the formulas used here were extracted from the
**	USGS publication "Map Projections - A Working Manual"
**	U. S. Geological Survey Professional Paper 1395, by
**	John P. Synder, pages 90-91.
**
**	In the arrays of doubles, x and longitude values always
**	occur first.  Negative values indicate values West or
**	South of the origin.
**********************************************************************/

int EXP_LVL9 CSedcylF (Const struct cs_Edcyl_ *edcyl,double xy [2],Const double ll [2])
{
	extern double cs_Degree;			/* 1.0 / RADIAN  */
	extern double cs_Two_pi;			/* 2 Pi */
	extern double cs_3Pi_o_2;			/* 3 Pi over 2 */
	extern double cs_Pi_o_2;			/* Pi over 2 */

	int rtn_val;

	double lng;			/* The given longitude, after conversion
						   to radians. */
	double lat;			/* The given latitude after conversion
						   to radians. */
	double del_lng;

	rtn_val = cs_CNVRT_NRML;

	/* For this projection, we only support the spherical form,
	   therefore there is only one set of equations.  These are
	   pretty simple. */

	lat = ll [LAT] * cs_Degree;
	if (fabs (lat) > cs_Pi_o_2)
	{
		rtn_val = cs_CNVRT_RNG;
		lat = CS_adj1pi (lat);
	}

	lng = ll [LNG] * cs_Degree;
	del_lng = lng - edcyl->org_lng;
	if      (del_lng >  cs_3Pi_o_2 && edcyl->org_lng < 0.0) del_lng -= cs_Two_pi;
	else if (del_lng < -cs_3Pi_o_2 && edcyl->org_lng > 0.0) del_lng += cs_Two_pi;
	if (fabs (del_lng) >= cs_3Pi_o_2)
	{
		rtn_val = cs_CNVRT_RNG;
		del_lng = CS_adj2pi (del_lng);
	}

	xy [XX] = del_lng * edcyl->Rcos_ref_lat;
	xy [YY] = (lat - edcyl->org_lat) * edcyl->ka;

	if (edcyl->quad == 0)
	{
		xy [XX] += edcyl->x_off;
		xy [YY] += edcyl->y_off;
	}
	else
	{
		CS_quadF (xy,xy [XX],xy [YY],edcyl->x_off,edcyl->y_off,
							  edcyl->quad);
	}

	return (rtn_val);
}

/**********************************************************************
**	rtn_val = CSedcylI (edcyl,ll,xy);
**
**	struct cs_Edcyl_ *edcyl;	structure which defines the parameters
**								in effect for the transformation.
**	double ll [2];				the longitude ([0]) and latitude ([1]),
**								in degrees, are returned here.
**	double xy [2];				the coordinates to be converted, x ([0])
**								and y ([1]).
**	int rtn_val;				returns cs_CNVRT_NRML if result is normal;
**								cs_CNVRT_RNG if value to be converted is
**								outside of mathematical scope of the
**								projection.
**
**	The ll and xy arguments of this function may point to the same
**	array with no adverse affects.
**
**	All the formulae used here were extracted from the
**	USGS publication "Map Projections - A Working Manual",
**	U. S. Geological Survey Professional Paper 1395, by
**	John P. Synder, pages 90-91.  We have added the idea of
**	an origin latitude other than the equator.
**
**	In the arrays of doubles, x and longitude values always
**	occur first.  Negative values indicate values West or
**	South of the origin.
**
**	For this projection, only the spherical form is supported.
**********************************************************************/

int EXP_LVL9 CSedcylI (Const struct cs_Edcyl_ *edcyl,double ll [2],Const double xy [2])
{
	extern double cs_Radian;			/*  57.29577..... */
	extern double cs_3Pi_o_2;			/* 3 Pi over 2 */
	extern double cs_Pi_o_2;			/* Pi over 2 */

	int rtn_val;

	double x;
	double y;
	double lat;
	double del_lng;

	rtn_val = cs_CNVRT_NRML;

	if (edcyl->quad == 0)
	{
		x = xy [XX] - edcyl->x_off;
		y = xy [YY] - edcyl->y_off;
	}
	else
	{
		CS_quadI (&x,&y,xy,edcyl->x_off,edcyl->y_off,edcyl->quad);
	}

	/* The following would blow up if the reference latitude
	   was either pole, but CSedcylQ doesn't allow that. */

	del_lng = x / edcyl->Rcos_ref_lat;
	if (fabs (del_lng) >= cs_3Pi_o_2)
	{
		del_lng = CS_adj2pi (del_lng);
		rtn_val = cs_CNVRT_RNG;
	}

	lat = edcyl->org_lat + (y / edcyl->ka);
	if (fabs (lat) > cs_Pi_o_2)
	{
		rtn_val = cs_CNVRT_RNG;
		lat = CS_adj1pi (lat);
	}

	ll [LNG] = (edcyl->org_lng + del_lng) * cs_Radian;
	ll [LAT] = lat * cs_Radian;
	return (rtn_val);
}

/**********************************************************************
**	gamma = CSedcylC (edcyl,ll);
**
**	struct cs_Edcyl_ *edcyl;	structure which carries all parameters
**								in effect for the coordinate system
**								being used, assuming the Equidistant Cylindrical
**								projection.
**	double ll [2];				the longitude ([0]) and the latitude ([1])
**								of the point at which the true scale of
**								the coordinate system is to be computed.
**								Values are in degrees.
**	double gamma;				returns the computed convergence angle for
**								the coordinate system as the specified
**								location in degrees east of north.
**
**	Returns cs_Km360 as an error indication; caused by a point
**	outside the domain of the projection.
**
**	We have not as yet found or deduced an analytical equation
**	for the convergence angle for this projection.  We calculate
**	it empicially. The convergence angle is defined as the
**	arctangent of the partial derivative of Y with respect to
**	latitude (read delta Y when the latitude changes a skosh)
**	divied by the partial derivative of X with repsect to
**	latitude (i.e. delta X).  See Synder/Bugayevskiy, page 16.
**********************************************************************/

double EXP_LVL9 CSedcylC (Const struct cs_Edcyl_ *edcyl,Const double ll [2])
{
	extern double cs_Radian;			/* 57.2957... */
	extern double cs_Km360;				/* -360.0, the value which
										   we return if provided
										   with a bogus point. */
	int status;

	double del_xx;
	double del_yy;
	double gamma;			/* some folks call this alpha */

	double xy1 [3];
	double xy2 [3];
	double my_ll [3];

	my_ll [LNG] = ll [LNG];
	my_ll [LAT] = ll [LAT];

	/* Compute the cartesian coordinates of the end points of a
	   linear line segment whose end points are on the same meridian,
	   but separated by a small amount of latitude.  The degree of
	   latitude separation is rather arbitrary.  Technically, the
	   smaller the better, but if its too small, we end up with
	   noise problems in the trig functions.  0.00005 gives us a
	   line of about 10 meters on the surface of the earth.  We
	   use literal constants as this may need to be adjusted to
	   the mathemagics of the specific projection involved. */

	my_ll [LAT] -= 0.00005;
	status = CSedcylF (edcyl,xy1,my_ll);
	if (status != cs_CNVRT_NRML)
	{
		return (cs_Km360);
	}
	my_ll [LAT] += 0.0001;		/* 2 * 0.00005 */
	status = CSedcylF (edcyl,xy2,my_ll);
	if (status != cs_CNVRT_NRML)
	{
		return (cs_Km360);
	}
	
	/* Some atan2's (not all) don't like it when both arguments are
	   zero. Normally, it is safe to assume that del_yy is never
	   zero given the above. However, testing has shown that when
	   provided with unrealistic locations, both del_xx and del_yy
	   can be zero (exactly on the pole where the pole is
	   the origin). */

	del_xx = xy2 [XX] - xy1 [XX];
	del_yy = xy2 [YY] - xy1 [YY];
	if ((fabs (del_xx) + fabs (del_yy)) > 0.0)
	{
		gamma = -atan2 (del_xx,del_yy) * cs_Radian;
	}
	else
	{
		gamma = cs_Km360;
	}
	return (gamma);
}

/**********************************************************************
**	kk = CSedcylK (edcyl,ll);
**
**	struct cs_Edcyl_ *edcyl;	structure which carries all parameters
**								in effect for the coordinate system
**								being used, assuming the Azimuthal Equal
**								Area projection.
**	double ll [2];				the longitude ([0]) and the latitude ([0])
**								of the point at which the true scale of
**								the coordinate system is to be computed.
**								Values are in degrees.
**	double kk;					returns the grid scale along a parallel of the
**								coordinate system at the specified point.
**
**	This projection is not defined, as far as we know at the
**	current time, for the ellipsoid.  We simple use the
**	equatorial radius of the supplied ellipsoid as the radius
**	of the sphere.
**********************************************************************/

double EXP_LVL9 CSedcylK (Const struct cs_Edcyl_ *edcyl,Const double ll [2])
{
	extern double cs_Degree;			/* 1/RADIAN */
	extern double cs_Pi_o_2;			/* Pi ober 2 */
	extern double cs_Mone;				/* -1.0, the value we return
										   for bogus lat/longs */
	extern double cs_NPTest;			/* 0.001 arc seconds short
										   of the north pole in
										   radians. */
	extern double cs_SclInf;			/* 9.9E+04, the value we
										   return for infinite
										   scale factor. */

	double lat;
	double kk;
	double cos_lat;

	lat = ll [LAT] * cs_Degree;
	if (fabs (lat) > cs_NPTest)
	{
		if (fabs (lat) > cs_Pi_o_2)
		{
			return (cs_Mone);
		}
		return (cs_SclInf);
	}
	cos_lat = cos (lat);
	kk = edcyl->cos_ref_lat / cos_lat;
	return (kk);
}

/**********************************************************************
**	hh = CSedcylH (edcyl,ll);
**
**	struct cs_Edcyl_ *edcyl;	structure which carries all parameters
**								in effect for the coordinate system
**								being used, assuming the Equidistant
**								Cylindrical Projection.
**	double ll [2];				the longitude ([0]) and the latitude ([0])
**								of the point at which the true scale of
**								the coordinate system is to be computed.
**								Values are in degrees.
**	double hh;					returns the grid scale along a meridian of the
**								coordinate system at the specified point.
**
**	This projection is not defined, as far as we know at the
**	current time, for the ellipsoid.  We simple use the
**	equatorial radius of the supplied ellipsoid as the radius
**	of the sphere.
**********************************************************************/

double EXP_LVL9 CSedcylH (Const struct cs_Edcyl_ *edcyl,Const double ll [2])
{
	extern double cs_One;			/* 1.0 */

	return (cs_One);
}									/*lint !e715 */

/**********************************************************************
**	status = CSedcylL (edcyl,cnt,pnts);
**
**	struct cs_Edcyl_ *edcyl;	the coordinate system against which the
**								check is to be performed.
**	int cnt;					the number of points in the region to be
**								checked.
**	double pnts [][3];			the list of coordinates to be checked.
**	int status;					returns cs_CNVRT_OK if the point, line,
**								or region is completely within the domain
**								of the coordinate system.  Otherwise,
**								cs_CNVRT_DOMN is returned.
**
**	This function simply checks the mathematical viability of
**	a coordinate conversion.  It has nothing to do with the
**	useful limits of the coordinate system.
**
**	This function expects that the input lat/longs are normalized
**	if appropriate.
**********************************************************************/

int EXP_LVL9 CSedcylL (Const struct cs_Edcyl_ *edcyl,int cnt,Const double pnts [][3])
{
	extern double cs_Degree;		/* 1.0 / 57.29...... */
	extern double cs_Pi_o_2;		/* PI over 2 */

	int ii;
	int status;

	double lat;

	/* Check all the points.  */

	status = cs_CNVRT_OK;
	for (ii = 0;ii < cnt;ii++)
	{
		lat = pnts [ii][LAT] * cs_Degree;
		if (fabs (lat) > cs_Pi_o_2)
		{
			status = cs_CNVRT_DOMN;
			break;
		}
	}
	return (status);
}							/*lint !e715 */

/**********************************************************************
**	status = CSedcylX (edcyl,cnt,pnts);
**
**	struct cs_Edcyl_ *edcyl;	coordinate system definition
**	int cnt;					number of points in the pnts array.
**	double pnts [][3];			an array of three dimensional points which
**								define a point, line, or region to be checked.
**	int status;					returns the status of the check; see REMARKS
**								below.
**
**	The values provided in the pnts array are required to be
**	cartesian coordinates.  Use CSedcylL to check lat/long
**	values.
**
**	What gets checked depends upon the cnt argument.  A value
**	of zero (or less than that) checks nothing successfully.
**	A value of 1 indicates that a single point is to be checked.
**	a value of 2 indicates that a line is to be checked.  The
**	entire line must reside within the domain of the coordinate
**	system to pass.
**
**	A value of 4 or more for cnt indicates that a region is to
**	be checked.  The pnts array must contain the indicated
**	number of points which defines a region.  The last point
**	in the array must duplicate the first point.  The resulting
**	polygon must be simply connected (no bow ties).  The region
**	passes if the entire region is within the domain of the
**	coordinate system.
**
**	The return value refers to either the point, or the line
**	segment, provided. Return values are as follows:
**
**	cs_CNVRT_OK	the point, entire line segment, or entire region,
**			is within the catresian domain of the coordinate
**			system.
**	cs_CNVRT_DOMN	the point, all or a portion of the line segment,
**			or all or a portion of the region, is not within
**			the cartesian domain of the coordinate system.
**********************************************************************/

int EXP_LVL9 CSedcylX (Const struct cs_Edcyl_ *edcyl,int cnt,Const double pnts [][3])
{
	int ii;
	int status;
	int rtn_val;

	double test_ll [3];

	rtn_val = cs_CNVRT_OK;

	/* Check that all X's and Y's are within the basic
	   range.  No special checks are required for lines and/or
	   regions. */

	for (ii = 0;ii < cnt;ii++)
	{
		status = CSedcylI (edcyl,test_ll,pnts [ii]);
		if (status != cs_CNVRT_NRML)
		{
			rtn_val = cs_CNVRT_DOMN;
			break;
		}
	}
	return (rtn_val);
}
