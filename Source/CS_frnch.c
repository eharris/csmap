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
 *       from this software without specific p0rior written permission.
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

int CSfrnchS  (struct cs_GridFile_ *gridFile)
{
	int status;

	struct cs_Frnch_ *frnchPtr;

	CS_erpt (cs_DTC_DAT_F);
	return -1;

	frnchPtr = CSnewFrnch (gridFile->filePath,gridFile->bufferSize,gridFile->flags,gridFile->density);
	if (frnchPtr != NULL)
	{
		/* OK, it initialized OK. */
		frnchPtr->cnvrgValue = gridFile->cnvrgValue;
		frnchPtr->errorValue = gridFile->errorValue;
		frnchPtr->maxIterations = gridFile->maxIterations;
		gridFile->fileObject.FrenchRgf = frnchPtr;

		gridFile->test = (cs_TEST_CAST)CSfrnchT;
		gridFile->frwrd2D = (cs_FRWRD2D_CAST)CSfrnchF2;
		gridFile->frwrd3D = (cs_FRWRD3D_CAST)CSfrnchF3;
		gridFile->invrs2D = (cs_INVRS2D_CAST)CSfrnchI2;
		gridFile->invrs3D = (cs_INVRS3D_CAST)CSfrnchI3;
		gridFile->inRange = (cs_INRANGE_CAST)CSfrnchL;
		gridFile->release = (cs_RELEASE_CAST)CSfrnchR;
		gridFile->destroy = (cs_DESTROY_CAST)CSfrnchD;

		status = 0;
	}
	else
	{
		status = -1;
	}
	return status;
}
double CSfrnchT (struct cs_Frnch_ *frnch,double *ll_src)
{
	return 0.0;
}
int CSfrnchF2 (struct cs_Frnch_ *frnch,double *ll_trg,Const double *ll_src)
{
	return 0;
}
int CSfrnchF3 (struct cs_Frnch_ *frnch,double *ll_trg,Const double *ll_src)
{
	return 0;
}
int CSfrnchI2 (struct cs_Frnch_ *frnch,double *ll_trg,Const double *ll_src)
{
	return 0;
}
int CSfrnchI3 (struct cs_Frnch_ *frnch,double *ll_trg,Const double *ll_src)
{
	return 0;
}
int CSfrnchL  (struct cs_Frnch_ *frnch,int cnt,Const double pnts [][3])
{
	return 0;
}
int CSfrnchQ  (struct cs_GridFile_* gridFile,unsigned short prj_code,int err_list [],int list_sz)
{
	return 0;
}
int CSfrnchR  (struct cs_Frnch_ *frnch)
{
	return 0;
}
int CSfrnchD  (struct cs_Frnch_ *frnch)
{
	return 0;
}


struct cs_Frnch_* CSnewFrnch (Const char *filePath,long32_t bufferSize,ulong32_t flags,double density)
{
	int status;

	struct cs_Frnch_* frnchPtr;
	
	/* Allocate the initial chunk of memory for this thing. */
	status = 0;
	frnchPtr = CS_malc (sizeof (struct cs_Frnch_));
	if (frnchPtr == NULL)
	{
		CS_erpt (cs_NO_MEM);
		goto error;
	}
	else
	{
		status = CSinitFrnch (frnchPtr,filePath,bufferSize,flags,density);
	}
	if (status != 0)
	{
		goto error;
	}
	return frnchPtr;
error:
	if (frnchPtr != NULL)
	{
		CS_free (frnchPtr);
		frnchPtr = NULL;
	}
	return NULL;
}

void CSinitializeFrnchObj (struct cs_Frnch_ *thisPtr)
{
	extern double cs_Zero;

	CSinitCoverage (&thisPtr->coverage);
	thisPtr->lngCount = 0L;
	thisPtr->latCount = 0L;
	thisPtr->deltaLng = cs_Zero;
	thisPtr->deltaLat = cs_Zero;
	thisPtr->rgf93ERad = cs_Zero;
	thisPtr->rgf93ESq = cs_Zero;
	thisPtr->ntfERad = cs_Zero;
	thisPtr->ntfESq = cs_Zero;
	thisPtr->filePath [0] = '\0';
	thisPtr->fileName [0] = '\0';
	thisPtr->deltaX = NULL;
	thisPtr->deltaX = NULL;
	thisPtr->deltaX = NULL;
	thisPtr->crcX = 0U;
	thisPtr->crcY = 0U;
	thisPtr->crcZ = 0U;
	return;
}
int CSinitFrnch (struct cs_Frnch_* thisPtr,Const char *filePath,long32_t bufferSize,ulong32_t flags,double density)
{
	extern char cs_DirsepC;
	extern char cs_ExtsepC;
	extern char csErrnam [];
	extern double cs_Zero;

	int status;
	int hdrFlag = 0;

	long32_t dblFrmt;
	long32_t lngIdx, latIdx;
	unsigned arrayIdx;
	unsigned tknCount;
	size_t malcSize;

	char *cpV;
	csFILE *fstrm;
	struct cs_Eldef_ *elPtr;
	double lng, lat;
	double deltaX, deltaY, deltaZ;

	double sw [3], ne [2];
	char *ptrs [20];

	char lineBuffer [256];

	/* Get the object we are constructing into a known state. */
	CSinitializeFrnchObj (thisPtr);

	/* Prepare for an error. */
	fstrm = NULL;
	thisPtr = NULL;
	elPtr = NULL;
	status = -1;

	/* Keep lint happy! */
	sw [0] = sw [1] = ne [0] = ne [1] = cs_Zero;

	/* Capture the full path to the file and the file name. */
	CS_stncp (thisPtr->filePath,filePath,sizeof (thisPtr->filePath));
	cpV = strrchr (thisPtr->filePath,cs_DirsepC);
	if (cpV != NULL)
	{
		cpV += 1;
	}
	else
	{
		cpV = thisPtr->filePath;
	}
	CS_stncp (thisPtr->fileName,cpV,sizeof (thisPtr->fileName));
	cpV = strrchr (thisPtr->fileName,cs_ExtsepC);
	if (cpV != NULL) *cpV = '\0';

	/* OK, open the file, and extra the parameters.  There are two types of
	   files.  We handle either kind (since we can do it all in this module).
	   Get the first line and determine which type of file we have. */
	fstrm = CS_fopen (filePath,_STRM_TXTRD);
	if (fstrm == NULL)
	{
		CS_stncp (csErrnam,thisPtr->filePath,MAXPATH);
		CS_erpt (cs_DTC_FILE);
		goto error;
	}

	/* If a buffersize has been given, we use it. */
	if (bufferSize > 0L)
	{
		setvbuf (fstrm,NULL,_IOFBF,(size_t)bufferSize);
	}

	/* Parse the file content. */
	if (CS_fgets (lineBuffer,sizeof (lineBuffer),fstrm) == NULL)
	{
		CS_erpt (cs_IOERR);
		goto error;
	}
	tknCount = CS_spaceParse (lineBuffer,ptrs,20);

	/* Decide which file type we have. */
	if (!CS_stricmp (ptrs [0],"GR3D"))
	{
		/* Here for the verbose format. */
		while (CS_fgets (lineBuffer,sizeof (lineBuffer),fstrm) != NULL)
		{
			tknCount = CS_spaceParse (lineBuffer,ptrs,20);
			if (!CS_stricmp (ptrs [0],"GR3D3")) break;
			if (!CS_stricmp (ptrs [0],"GR3D1") && tknCount == 7)
			{
				hdrFlag = 1;
				/* Convert the parsed tokens in a locale independent manner.
				   That is, we need to avoid use of functions like strtod
				   as they are locale dependent. */
				dblFrmt  = CSatof (&sw[0],ptrs [1],'.',',',':');
				dblFrmt |= CSatof (&ne[0],ptrs [2],'.',',',':');
				dblFrmt |= CSatof (&sw[1],ptrs [3],'.',',',':');
				dblFrmt |= CSatof (&ne[1],ptrs [4],'.',',',':');
				dblFrmt |= CSatof (&thisPtr->deltaLng,ptrs [5],'.',',',':');
				dblFrmt |= CSatof (&thisPtr->deltaLat,ptrs [6],'.',',',':');
				if (dblFrmt < 0)
				{
					/* This should never happen with a good data file. */
					CS_erpt (cs_INV_FILE);
					goto error;
				}
			}
		}
		if (hdrFlag == 0)
		{
			/* If we didn't see the expected header record, we have
			   a big problem. */
			CS_stncp (csErrnam,thisPtr->filePath,MAXPATH);
			CS_erpt (cs_INV_FILE);
			goto error;
		}

		/* Capture the coverage area. */
		thisPtr->coverage.southWest [LNG] = sw [0];
		thisPtr->coverage.southWest [LAT] = sw [1];
		thisPtr->coverage.northEast [LNG] = ne [0];
		thisPtr->coverage.northEast [LAT] = ne [1];

		/* Set the density.  Normally it would be the smaller of deltaLng or
		   deltaLat.  These are usually the same for this file type.
		   Alternatively, a density may have been specified by the user in the
		   dictionary file.  In that case, we use that value. */
		if (density > 1.0E-12)			/* like to avoid == on doubles */
		{
			thisPtr->coverage.density = density;
		}
		else if (thisPtr->deltaLat >= thisPtr->deltaLng)
		{
			thisPtr->coverage.density = thisPtr->deltaLat;
		}
		else
		{
			thisPtr->coverage.density = thisPtr->deltaLng;
		}

		/* Compute the size of the grid.  We need to add the extra 0.1 to
		   account for round off errors on certain compilers/platforms. */
		thisPtr->lngCount = (long32_t)(((ne [0] - sw [0]) / thisPtr->deltaLng) + 0.1) + 1;
		thisPtr->latCount = (long32_t)(((ne [1] - sw [1]) / thisPtr->deltaLat) + 0.1) + 1;

		/* Now we can allocate the arrays. */
		malcSize = (size_t)(thisPtr->lngCount * thisPtr->latCount) * sizeof (long32_t);
		thisPtr->deltaX = CS_malc (malcSize);
		if (thisPtr->deltaX == NULL)
		{
			CS_erpt (cs_NO_MEM);
			goto error;
		}
		thisPtr->deltaY = CS_malc (malcSize);
		if (thisPtr->deltaY == NULL)
		{
			CS_erpt (cs_NO_MEM);
			goto error;
		}
		thisPtr->deltaZ = CS_malc (malcSize);
		if (thisPtr->deltaZ == NULL)
		{
			CS_erpt (cs_NO_MEM);
			goto error;
		}

		/* Initialize the delta arrays to zero.  Perhaps we should use memset
		   here for performance reasons.  Not much to be gained, however, since
		   we have to parse through 2MB of text anyway. */
		for (lngIdx = 0;lngIdx < thisPtr->lngCount;lngIdx += 1)
		{
			for (latIdx = 0;latIdx < thisPtr->latCount;latIdx += 1)
			{
				arrayIdx = (unsigned)((latIdx * thisPtr->lngCount) + lngIdx);
				*(thisPtr->deltaX + arrayIdx) = 0L;
				*(thisPtr->deltaY + arrayIdx) = 0L;
				*(thisPtr->deltaZ + arrayIdx) = 0L;
			}
		}

		/* Fill in the ellipsoid numbers.  Pretty hoeky, but it works.
		   Unfortunately, the specific ellipsoids involved are impled by the
		   data file, but the numbers are not included in the format.  Thus,
		   for now, this grid file format is usful only for the specific French geography for which it was developed. */
		elPtr = CS_eldef ("GRS1980");
		if (elPtr == NULL) goto error;
		thisPtr->rgf93ERad = elPtr->e_rad;
		thisPtr->rgf93ESq = elPtr->ecent * elPtr->ecent;
		CS_free (elPtr);
		elPtr = NULL;

		elPtr = CS_eldef ("CLRK-IGN");
		if (elPtr == NULL) goto error;
		thisPtr->ntfERad = elPtr->e_rad;
		thisPtr->ntfESq = elPtr->ecent * elPtr->ecent;
		CS_free (elPtr);
		elPtr = NULL;

		/* Process the rest of the file. */
		while (CS_fgets (lineBuffer,sizeof (lineBuffer),fstrm) != NULL)
		{
			tknCount = CS_spaceParse (lineBuffer,ptrs,20);
			if (tknCount == 8)
			{
				/* Convert the data to numeric form in a locale independent
				   manner. */
				dblFrmt  = CSatof (&lng,ptrs [1],'.',',',':');
				dblFrmt |= CSatof (&lat,ptrs [2],'.',',',':');
				dblFrmt |= CSatof (&deltaX,ptrs [3],'.',',',':');
				dblFrmt |= CSatof (&deltaY,ptrs [4],'.',',',':');
				dblFrmt |= CSatof (&deltaZ,ptrs [5],'.',',',':');
				if (dblFrmt < 0)
				{
					/* This should never happen with a good data file. */
					CS_erpt (cs_INV_FILE);
					goto error;
				}

				/* Determine the location of the node in the grid. */
				lngIdx = (long32_t)(((lng - sw [0]) / thisPtr->deltaLng) + 1.0E-10);
				latIdx = (long32_t)(((lat - sw [1]) / thisPtr->deltaLat) + 1.0E-10);
				if (lngIdx < 0 || lngIdx >= thisPtr->lngCount ||
					latIdx < 0 || latIdx >= thisPtr->latCount)
				{
					/* This should never happen with a good data file. */
					CS_erpt (cs_INV_FILE);
					goto error;
				}
				
				/* Adjust for possible round down; required for Linux (or is it
				   the gcc runtime library), perhaps others as well. */
				deltaX += (deltaX < 0.0) ? -0.0002 : 0.0002;
				deltaY += (deltaY < 0.0) ? -0.0002 : 0.0002;
				deltaZ += (deltaZ < 0.0) ? -0.0002 : 0.0002;

				/* Stuff extracted values in the local arrays. */
				arrayIdx = (unsigned)((latIdx * thisPtr->lngCount) + lngIdx);
				*(thisPtr->deltaX + arrayIdx) = (long32_t)(deltaX * 1000.0);
				*(thisPtr->deltaY + arrayIdx) = (long32_t)(deltaY * 1000.0);
				*(thisPtr->deltaZ + arrayIdx) = (long32_t)(deltaZ * 1000.0);
			}
		}
	}
	else
	{
		/* We don't support the abbreviated file.  The abbreviated file is
		   simply a binary version of the text file.  Thus, we avoid all byte
		   swapping issues.  There is little to be gained by using the
		   abbreviated file. */
		CS_erpt (cs_INV_FILE);
		goto error;
	}

	/* For testing purposes, we generate the check-sum of the three memory
	   arrays allocated above.  The CScheckRgf93ToNtf function verifies that the
	   memory in these arrays has not changed since they were allocated.
	   
	   This was implemented in response to a bug report which suggested that the
	   in memory arrays may have been corrupted by some code elsehwere in the
	   library.  This check has never produced a failure and this code is a
	   good candidate for removal. */
	thisPtr->crcX = CS_crc16 (0X0101,(unsigned char *)thisPtr->deltaX,(int)malcSize);
	thisPtr->crcY = CS_crc16 (0X0202,(unsigned char *)thisPtr->deltaY,(int)malcSize);
	thisPtr->crcZ = CS_crc16 (0X0404,(unsigned char *)thisPtr->deltaZ,(int)malcSize);

	/* OK, we're outa here. */
	CS_fclose (fstrm);
	fstrm = NULL;
	status = 0;
	return status;

error:
	if (elPtr != NULL)
	{
		CS_free (elPtr);
		elPtr = NULL;
	}
	if (fstrm != NULL)
	{
		CS_fclose (fstrm);
		fstrm = NULL;
	}
	CSdeleteFrnch (thisPtr);
	return -1;
}
void CSdeleteFrnch (struct cs_Frnch_* thisPtr)
{
	if (thisPtr != NULL)
	{
		if (thisPtr->deltaX != NULL)
		{
			CS_free (thisPtr->deltaX);
			thisPtr->deltaX = NULL;
		}
		if (thisPtr->deltaY != NULL)
		{
			CS_free (thisPtr->deltaY);
			thisPtr->deltaY = NULL;
		}
		if (thisPtr->deltaZ != NULL)
		{
			CS_free (thisPtr->deltaZ);
			thisPtr->deltaZ = NULL;
		}
		CS_free (thisPtr);
	}
}
