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

int EXP_LVL9 CSgridiQ (struct cs_GeodeticTransform_ *gxDef,unsigned short xfrmCode,
														   int err_list [],
														   int list_sz)
{
	int err_cnt;

	/* We will return (err_cnt + 1) below. */
	err_cnt = -1;
	if (err_list == NULL) list_sz = 0;

	/* Locate the file format in the list table and call the file specific
	   quality checker. */


	/* Check the definition stuff specific to all grid interpolation types
	   as a group.  
	if (fabs (gxDef->parameters.geocentricParameters.deltaX) > cs_DelMax)
	{
		if (++err_cnt < list_sz) err_list [err_cnt] = cs_DTQ_DELTAX;
	} */

	/* That's it for Grid Interpolation transformation. */
	return (err_cnt + 1);
}
/******************************************************************************
*/
int EXP_LVL9 CSgridiS (struct cs_GxXform_* gxXfrm)
{
	extern char *cs_DirP;
	extern char cs_Dir [];
	extern struct cs_GridFormatTab_ cs_GridFormatTab [];

	char cc1;
	char cc2;

	short idx;
	short fileCount;

	int status;

	char *cp;
	struct csGridi_* gridi;
	struct cs_GridFile_* gridFilePtr;
	struct cs_GridFormatTab_* frmtTblPtr;
	struct csGeodeticXfromParmsFile_* fileDefPtr;
	struct csGeodeticXformParmsGridFiles_* filesPtr;

	char wrkBufr [MAXPATH + MAXPATH];

	gridi = &gxXfrm->xforms.gridi;

	gridi->errorValue    = gxXfrm->errorValue;
	gridi->cnvrgValue    = gxXfrm->cnvrgValue;
	gridi->maxIterations = gxXfrm->maxIterations;
	gridi->useBest = FALSE;

	gxXfrm->frwrd2D = (cs_FRWRD2D_CAST)CSgridiF2;
	gxXfrm->frwrd3D = (cs_FRWRD3D_CAST)CSgridiF3;
	gxXfrm->invrs2D = (cs_INVRS2D_CAST)CSgridiI2;
	gxXfrm->invrs3D = (cs_INVRS3D_CAST)CSgridiI3;
	gxXfrm->inRange = (cs_INRANGE_CAST)CSgridiL;
	gxXfrm->release = (cs_RELEASE_CAST)CSgridiR;
	gxXfrm->destroy = (cs_DESTROY_CAST)CSgridiD;

	/* OK, that's the easy stuff.  Now need to loop through each entry in the
	   csGeodeticXformParmsGridFiles_ structure of the cs_GeodeticTransform_
	   definition which we have been provided. */
	filesPtr = &gxXfrm->gxDef.parameters.fileParameters;
	fileCount = filesPtr->fileReferenceCount;
	for (idx = 0;idx < fileCount;idx += 1)
	{
		/* Here once for each file.  Often, there is just one file.  Multiple
		   files is not uncommon.  There will be one or two cases where there
		   are as many as 48 files.

		   For each file, we allocate a cs_GridFile_ structure, populate it
		   with the basic parameters (direction, format, and file path).  We
		   then look the format up in the cs_GridFormatTab_ table, and
		   assuming success, call the initialization function.

		   If all this succeeds, the pointer to the initialized cs_GridFile_
		   object is added to the gridFiles array of the csGridi_ object. */
		fileDefPtr = &filesPtr->fileNames [idx];
		gridFilePtr = (struct cs_GridFile_*)CS_malc (sizeof (struct cs_GridFile_));
		if (gridFilePtr == NULL)
		{
			CS_erpt (cs_NO_MEM);
			goto error;
		}
		gridFilePtr->direction = fileDefPtr->direction;
		gridFilePtr->format = fileDefPtr->fileFormat;
		cp = fileDefPtr->fileName;
		cc1 = *cp;
		cc2 = *(cp + 1);
		if (cc1 == '.' && (cc2 == '\\' || cc2 == '/'))
		{
			/* The file reference is relative. */
			*cs_DirP = '\0';
			CS_stncp (wrkBufr,cs_Dir,MAXPATH);
			CS_stncat (wrkBufr,(cp + 2),MAXPATH);
			CS_stncp (gridFilePtr->filePath,wrkBufr,sizeof (gridFilePtr->filePath));
		}
		else
		{
			/* The file is absolute path name; we can (should be able to) use
			   it as it is. */
			CS_stncp (gridFilePtr->filePath,fileDefPtr->fileName,sizeof (gridFilePtr->filePath));
		}
		gridFilePtr->bufferSize = 0L;
		gridFilePtr->flags = 0UL;
		gridFilePtr->density = 0.0;
		gridFilePtr->cnvrgValue = gridi->cnvrgValue;
		gridFilePtr->errorValue = gridi->errorValue;
		gridFilePtr->maxIterations = gridi->maxIterations;

		for (frmtTblPtr = cs_GridFormatTab;frmtTblPtr->formatCode != cs_DTCFRMT_NONE;frmtTblPtr += 1)
		{
			if (frmtTblPtr->formatCode == gridFilePtr->format)
			{
				break;
			}
		}
		if (frmtTblPtr->formatCode == cs_DTCFRMT_NONE)
		{
			CS_erpt (cs_ISER);
			goto error;
		}

		/* OK, we have a file specification and a valid format, thus we have
		   a setup function. */
		status = (*frmtTblPtr->initialize)(gridFilePtr);
		if (status != 0)
		{
			goto error;
		}

		/* Success!!! Stash a pointer to our initialized object and then
		   continue on to see if there is another file to do. */
		gridi->gridFiles [gridi->fileCount++] = gridFilePtr;
		gridFilePtr = NULL;
	}
	return 0;

error:
	if (gridFilePtr != NULL)
	{
		CS_free (gridFilePtr);
		gridFilePtr = NULL;
	}
	if (gridi != NULL)
	{
		for (idx = 0;idx < gridi->fileCount;idx += 1)
		{
			gridFilePtr = gridi->gridFiles [idx];
			if (gridFilePtr != NULL)
			{
				(*gridFilePtr->destroy)(gridFilePtr->fileObject.genericPtr);
			}
		}
	}
	return -1;
}
int EXP_LVL9 CSgridiF3 (struct csGridi_ *gridi,double trgLl [3],Const double srcLl [3])
{
	extern char csErrnam [MAXPATH];

	int status;
	int selectedIdx;
	
	struct cs_GridFile_* gridFilePtr;

	selectedIdx = CSgridiT (gridi,srcLl);
	if (selectedIdx >= 0)
	{
		gridFilePtr = gridi->gridFiles [selectedIdx];
		if (gridFilePtr != NULL)
		{	
			status = (*gridFilePtr->frwrd3D)(gridFilePtr->fileObject.genericPtr,trgLl,srcLl);
		}
		else
		{
			CS_stncp (csErrnam,"CS_gridi::1",MAXPATH);
			CS_erpt (cs_ISER);
			status = -1;
		}
	}
	else
	{
		status = 1;
	}
	return status;
}
int EXP_LVL9 CSgridiF2 (struct csGridi_ *gridi,double* trgLl,Const double* srcLl)
{
	extern char csErrnam [MAXPATH];

	int status;
	int selectedIdx;
	
	struct cs_GridFile_* gridFilePtr;

	selectedIdx = CSgridiT (gridi,srcLl);
	if (selectedIdx >= 0)
	{
		gridFilePtr = gridi->gridFiles [selectedIdx];
		if (gridFilePtr != NULL)
		{	
			status = (*gridFilePtr->frwrd2D)(gridFilePtr->fileObject.genericPtr,trgLl,srcLl);
		}
		else
		{
			CS_stncp (csErrnam,"CS_gridi::2",MAXPATH);
			CS_erpt (cs_ISER);
			status = -1;
		}
	}
	else
	{
		status = 1;
	}
	return status;
}
int EXP_LVL9 CSgridiI3 (struct csGridi_ *gridi,double* trgLl,Const double* srcLl)
{
	extern char csErrnam [MAXPATH];

	int status;
	int selectedIdx;
	
	struct cs_GridFile_* gridFilePtr;

	selectedIdx = CSgridiT (gridi,srcLl);
	if (selectedIdx >= 0)
	{
		gridFilePtr = gridi->gridFiles [selectedIdx];
		if (gridFilePtr != NULL)
		{	
			status = (*gridFilePtr->invrs3D)(gridFilePtr->fileObject.genericPtr,trgLl,srcLl);
		}
		else
		{
			CS_stncp (csErrnam,"CS_gridi::3",MAXPATH);
			CS_erpt (cs_ISER);
			status = -1;
		}
	}
	else
	{
		status = 1;
	}
	return status;
}
int EXP_LVL9 CSgridiI2 (struct csGridi_ *gridi,double* trgLl,Const double* srcLl)
{
	extern char csErrnam [MAXPATH];

	int status;
	int selectedIdx;
	
	struct cs_GridFile_* gridFilePtr;

	selectedIdx = CSgridiT (gridi,srcLl);
	if (selectedIdx >= 0)
	{
		gridFilePtr = gridi->gridFiles [selectedIdx];
		if (gridFilePtr != NULL)
		{	
			status = (*gridFilePtr->invrs2D)(gridFilePtr->fileObject.genericPtr,trgLl,srcLl);
		}
		else
		{
			CS_stncp (csErrnam,"CS_gridi::4",MAXPATH);
			CS_erpt (cs_ISER);
			status = -1;
		}
	}
	else
	{
		status = 1;
	}
	return status;
}
int EXP_LVL9 CSgridiL (struct csGridi_ *gridi,int cnt,Const double pnts [][3])
{
	int status;
	status = 0;
	return status;
}
int EXP_LVL9 CSgridiR (struct csGridi_ *gridi)
{
	short idx;
	int status;
	struct cs_GridFile_* gridFilePtr;

	status = 0;
	for (idx = 0;idx < gridi->fileCount;idx += 1)
	{
		gridFilePtr = gridi->gridFiles [idx];
		if (gridFilePtr != NULL)
		{
			(*gridFilePtr->release)(gridFilePtr->fileObject.genericPtr);
		}
	}
	return status;
}
int EXP_LVL9 CSgridiD (struct csGridi_ *gridi)
{
	short idx;
	int status;
	struct cs_GridFile_* gridFilePtr;

	status = 0;
	for (idx = 0;idx < gridi->fileCount;idx += 1)
	{
		gridFilePtr = gridi->gridFiles [idx];
		if (gridFilePtr != NULL)
		{
			(*gridFilePtr->destroy)(gridFilePtr->fileObject.genericPtr);
			CS_free (gridFilePtr);
			gridi->gridFiles [idx] = NULL;
		}
	}
	return status;
}
int CSgridiT (struct csGridi_ *gridi,double* ll_src)
{
	short idx;
	short selectedIdx;

	double density;
	double bestSoFar;
	
	struct cs_GridFile_* gridFilePtr;

	selectedIdx = -1;
	if (!gridi->useBest)
	{
		/* Use the first one in the list with the appropriate coverage. */
		for (idx = 0;idx < gridi->fileCount;idx += 1)
		{
			gridFilePtr = gridi->gridFiles [idx];
			if (gridFilePtr != NULL)
			{
				density = (*gridFilePtr->test)(gridFilePtr->fileObject.genericPtr,ll_src);
				if (density != 0.0)
				{
					selectedIdx = idx;
					break;
				}
			}
		}
	}
	else
	{
		/* Select the specific file which provides the highest grid density
		   (i.e. the smallest grid cells) coverage. */
		bestSoFar = 99.0E+100;
		for (idx = 0;idx < gridi->fileCount;idx += 1)
		{
			gridFilePtr = gridi->gridFiles [idx];
			if (gridFilePtr != NULL)
			{
				density = (*gridFilePtr->test)(gridFilePtr->fileObject.genericPtr,ll_src);
				if (density == 0.0)
				{
					continue;
				}
				/* Densities should always be positive. */
				if (density < bestSoFar)
				{
					bestSoFar = density;
					selectedIdx = idx;
				}
			}
		}
	}
	return selectedIdx;
}
