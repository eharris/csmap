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

/*lint -esym(550,ctItmName,dummy)  */

#include "cs_map.h"

short CSctCompCsChk (csFILE *csStrm,Const char* csName);

// API
// External
/******************************************************************************
 * Returns +1 if the provided name is that of a defined category, else returns
 * zero.  Returns -1 if no category data has not been initialized.
 *****************************************************************************/
int EXP_LVL1 CS_vldCtName (const char* catName)
{
	int rtnValue;	
	struct cs_Ctdef_* ctDefPtr;

	ctDefPtr = CSgetCtDefHead (cs_CATDEF_READ);
	rtnValue = (ctDefPtr == NULL) ? -1 : 0;
	while (ctDefPtr != NULL)
	{
		if (CS_stricmp (ctDefPtr->ctName,catName) == 0)
		{
			rtnValue = 1;
			break;
		}
		ctDefPtr = ctDefPtr->next;
	}
	return rtnValue;
}
/******************************************************************************
 * Returns the category name associated with the idx'th category in the
 * category list.  Returns null if no category data available, or if the idx
 * argument is too large.  idx of zero means the first category.
 *
 * Use this function to enumerate all category names by incrementing the idx
 * value by one after each call, until such time as a null pointer is returned.
 *****************************************************************************/
Const char* EXP_LVL1 CS_getCatName (unsigned idx)
{
	unsigned myIdx = 0;
	Const char* rtnValue = 0;
	struct cs_Ctdef_* ctDefPtr;

	ctDefPtr = CSgetCtDefHead (cs_CATDEF_READ);
	while (ctDefPtr != NULL)
	{
		if (myIdx == idx)
		{
			rtnValue = ctDefPtr->ctName;
			break;
		}
		myIdx += 1;
		ctDefPtr = ctDefPtr->next;
	}	
	return rtnValue;
}
/******************************************************************************
 * Returns the item name associated with the idx'th item in the category
 * indicated by the catName argument.  Returns null if no category data
 * available, or if the catName argument is not that of a valid category, or
 * the idx argument is too large.  idx of zero means the first item name.
 *
 * Use this function to enumerate all item names within a ctaegory by
 * incrementing the idx value by one after each call, until such time as a
 * null pointer is returned.
 *****************************************************************************/
Const char* EXP_LVL1 CS_getItmName (const char* catName,unsigned idx)
{
	Const char* rtnValue = 0;
	struct cs_Ctdef_* ctDefPtr;
	
	ctDefPtr = CSgetCtDefHead (cs_CATDEF_READ);
	while (ctDefPtr != NULL)
	{
		if (CS_stricmp (ctDefPtr->ctName,catName) == 0)
		{
			/* We have found the desired category.  Verify that the index
			   value is valid, and then set rtnValue to the appropriate
			   item name. */
			if (idx < ctDefPtr->nameCnt)
			{
				rtnValue = (ctDefPtr->csNames + idx)->csName;
			}
			break;
		}
		ctDefPtr = ctDefPtr->next;
	}
	return rtnValue;
}

void EXP_LVL3 CS_ctfnm (const char* new_name)
{
	extern char cs_Ctname [];

	(void)CS_stncp (cs_Ctname,new_name,cs_FNM_MAXLEN);
}

// Internal
struct cs_Ctdef_* EXP_LVL3 CSgetCtDefHead (int mode)
{
	extern struct cs_Ctdef_* cs_CtDefHead;

	if (mode == cs_CATDEF_RELEASE)
	{
		CSrlsCategoryList (cs_CtDefHead);
		cs_CtDefHead = NULL;
	}
	else if (mode == cs_CATDEF_READ && cs_CtDefHead == NULL)
	{
		cs_CtDefHead = CSrdCatFile ();
	}
	return cs_CtDefHead;
}
void EXP_LVL3 CSsetCtDefHead (struct cs_Ctdef_* newCtDefHead)
{
	extern struct cs_Ctdef_* cs_CtDefHead;

	if (cs_CtDefHead == NULL)
	{
		cs_CtDefHead = newCtDefHead;
	}
}
/******************************************************************************
 * Returns the global variable extern struct cs_Ctdef_* cs_CtDefHead = NULL;
 *
 * Function returns zero on success, -1 for failure.  Failure can be caused by:
 * 1> Category data has not been initialized.
 * 2> ctName argument is not that of a valid category
 * 3> idx argument does not select an item name in the indicated category.
 *****************************************************************************/


/******************************************************************************
 * Replaces the idx'th item name in the category indicated by the catName
 * argument.  It is assumed that the idx argument is that which was used to
 * locate the desired item.
 *
 * Function returns zero on success, -1 for failure.  Failure can be caused by:
 * 1> Category data has not been initialized.
 * 2> ctName argument is not that of a valid category
 * 3> idx argument does not select an item name in the indicated category.
 *****************************************************************************/
int EXP_LVL3 CSrplItmName (Const char* catName,unsigned idx,Const char* newName)
{
	int status = -1;
	char* destPtr;
	struct cs_Ctdef_* ctDefPtr;
	struct cs_CtItmName_ ctItmName;

	ctDefPtr = CSgetCtDefHead (cs_CATDEF_READ);
	while (ctDefPtr != NULL)
	{
		if (CS_stricmp (ctDefPtr->ctName,catName) == 0)
		{
			/* We have found the desired category.  Verify that the index
			   value is valid, and then replace the name */
			if (idx < ctDefPtr->nameCnt)
			{
				destPtr = (ctDefPtr->csNames + idx)->csName;
				CS_stncp (destPtr,newName,sizeof (ctItmName.csName));
				status = 0;
			}
			break;
		}
		ctDefPtr = ctDefPtr->next;
	}
	return status;
}
/******************************************************************************
 * Removes the idx'th item name in the category indicated by the catName
 * argument.  It is assumed that the idx argument is that which was used to
 * locate the desired item.
 *
 * Function returns zero on success, -1 for failure.  Failure can be caused by:
 * 1> Category data has not been initialized.
 * 2> ctName argument is not that of a valid category
 * 3> idx argument does not select an item name in the indicated category.
 *****************************************************************************/
int EXP_LVL3 CSrmvItmName (Const char* catName,unsigned idx)
{
	return -1;
}
/******************************************************************************
 * Adds an item name to the category indicated by the ctName argument.
 *
 * Function returns zero on success, -1 for failure.  Failure can be caused by:
 * 1> Category data has not been initialized.
 * 2> ctName argument is not that of a valid category
 *
 * If reallocation is necessary, and the reallocation fails, a -2 value is
 * returned.
 *
 * THIS FUNCTION DOES NOT CHECK TO SEE IF THE PROVIDED ITEM NAME ALREADY
 * EXISTS.
 *****************************************************************************/
int EXP_LVL3 CSaddItmName (Const char* catName,Const char* newName)
{
	int status;
	unsigned idx;
	unsigned newSize;
	char* destPtr;
	struct cs_Ctdef_* ctDefPtr;
	struct cs_CtItmName_ ctItmName;

	status = -1;
	ctDefPtr = CSgetCtDefHead (cs_CATDEF_READ);
	while (ctDefPtr != NULL)
	{
		if (CS_stricmp (ctDefPtr->ctName,catName) == 0)
		{
			/* We have found the desired category.  Reallocate the name array
			   if necessary to obtain space for the new name. */
			if (ctDefPtr->nameCnt >= ctDefPtr->allocCnt)
			{
				newSize =  ctDefPtr->allocCnt * sizeof (struct cs_CtItmName_);
				newSize += cs_CATDEF_ALLOC * sizeof (struct cs_CtItmName_);
				ctDefPtr->csNames = CS_ralc (ctDefPtr->csNames,newSize);
				if (ctDefPtr->csNames == 0)
				{
					status = -2;
					ctDefPtr->nameCnt = 0;
					ctDefPtr->allocCnt = 0;
				}
				else
				{
					ctDefPtr->allocCnt += cs_CATDEF_ALLOC;
				}
			}
			if (ctDefPtr->nameCnt < ctDefPtr->allocCnt)		/* Defensive */
			{
				idx = ctDefPtr->nameCnt++;
				destPtr = (ctDefPtr->csNames + idx)->csName;
				CS_stncp (destPtr,newName,sizeof (ctItmName.csName));
				status = 0;
			}
			break;
		}
		ctDefPtr = ctDefPtr->next;
	}
	return status;
}

/******************************************************************************
 * Writes the provided category to the provided stream.  Stream must be opened
 * in binary mode as binary data will be written.  (Of course, it must also be
 * opened for writing.)  The new category data will be written at the current
 * position of the stream.
 *
 * Function returns zero on success, -1 for failure.  Failure can be caused by:
 * 1> stream write error
 * 2> badly formatted cotegory structure.
 *****************************************************************************/
int EXP_LVL3 CSwrtCategory (csFILE* stream,Const struct cs_Ctdef_*ctDefPtr)
{
	size_t wrCnt;
	unsigned idx;
	struct cs_CtItmName_* itmPtr;
	struct cs_CtItmName_ dummy;

	CS_stncp (dummy.csName,cs_CATDEF_UNUSED,sizeof (dummy.csName));	
	wrCnt = CS_fwrite (ctDefPtr->ctName,sizeof (ctDefPtr->ctName),1,stream);
	if (wrCnt == 1)
	{
		wrCnt = CS_fwrite(&ctDefPtr->nameCnt,sizeof(ctDefPtr->nameCnt),1,stream);
	}
	if (wrCnt == 1)
	{
		wrCnt = CS_fwrite (&ctDefPtr->allocCnt,sizeof(ctDefPtr->allocCnt),1,stream);
	}

	/* Item names. */
	for (idx = 0;wrCnt == 1 && idx < ctDefPtr->allocCnt;idx += 1)
	{
		if (idx < ctDefPtr->nameCnt)
		{
			itmPtr = ctDefPtr->csNames + idx;
			wrCnt = CS_fwrite (itmPtr->csName,sizeof (itmPtr->csName),1,stream);
		}
		else
		{
			wrCnt = CS_fwrite (dummy.csName,sizeof (dummy.csName),1,stream);
		}
	}
	return (wrCnt == 1) ? 0 : -1;
}
int EXP_LVL3 CSrplCatName (Const char* newCtName,unsigned idx)
{
	int status;
	unsigned myIdx;
	struct cs_Ctdef_* ctDefPtr;

	ctDefPtr = CSgetCtDefHead (cs_CATDEF_READ);
	status = (ctDefPtr == NULL) ? -1 : 1;
	myIdx = 0;
	while (ctDefPtr != NULL)
	{
		if (myIdx == idx)
		{
			CS_stncp (ctDefPtr->ctName,newCtName,sizeof (ctDefPtr->ctName));
			status = 0;
			break;
		}
		ctDefPtr = ctDefPtr->next;
		myIdx += 1;
	}
	return status;
}
int EXP_LVL3 CSaddCategory (Const char* catName)
{
	struct cs_Ctdef_* newDefPtr;
	struct cs_Ctdef_* ctDefPtr;

	/* Create a new empty category. */
	newDefPtr = CSnewCategory (catName);
	if (newDefPtr == NULL)
	{
		goto error;
	}

	/* Add the new category struct to the current list. */
	ctDefPtr = CSgetCtDefHead (cs_CATDEF_ASIS);
	if (ctDefPtr == NULL)
	{
		CSsetCtDefHead (newDefPtr);
	}
	else
	{
		while (ctDefPtr->next != NULL)
		{
			ctDefPtr = ctDefPtr->next;
		}
		ctDefPtr->next = newDefPtr;
	}
	return 0;
error:
	return -1;
}
struct cs_Ctdef_* EXP_LVL3 CSnewCategory (Const char* ctName)
{
	unsigned allocSize;
	struct cs_Ctdef_* newDefPtr;

	newDefPtr = CS_malc (sizeof (struct cs_Ctdef_));
	if (newDefPtr == NULL)
	{
		CS_erpt (cs_NO_MEM);
		goto error;
	}
	newDefPtr->next = NULL;
	newDefPtr->csNames = NULL;
	CS_stncp (newDefPtr->ctName,ctName,sizeof (newDefPtr->ctName));
	newDefPtr->nameCnt = 0;
	newDefPtr->allocCnt = cs_CATDEF_ALLOC;
	allocSize = newDefPtr->allocCnt * sizeof (struct cs_CtItmName_);
	newDefPtr->csNames = CS_malc (allocSize);
	if (newDefPtr->csNames == NULL)
	{
		CS_free (newDefPtr);
		CS_erpt (cs_NO_MEM);
		goto error;
	}
	memset (newDefPtr->csNames,'\0',allocSize);
	return newDefPtr;
error:
	return NULL;
}
struct cs_Ctdef_* EXP_LVL3 CSrdCategory (csFILE* stream)
{
	size_t rdCnt;
	unsigned idx;
	unsigned allocSize;
	struct cs_CtItmName_ *destPtr;
	struct cs_Ctdef_* ctDefPtr;

	ctDefPtr = NULL;
	ctDefPtr = CS_malc (sizeof (struct cs_Ctdef_));
	if (ctDefPtr == NULL)
	{
		CS_erpt (cs_NO_MEM);
		goto error;
	}
	ctDefPtr->next = NULL;
	ctDefPtr->csNames = NULL;
	ctDefPtr->nameCnt = 0U;
	ctDefPtr->allocCnt = 0U;

	rdCnt = CS_fread (ctDefPtr->ctName,sizeof (ctDefPtr->ctName),1,stream);
	if (rdCnt == 1)
	{
		rdCnt = CS_fread (&ctDefPtr->nameCnt,sizeof (ctDefPtr->nameCnt),1,stream);
	}
	if (rdCnt == 1)
	{
		rdCnt = CS_fread (&ctDefPtr->allocCnt,sizeof (ctDefPtr->allocCnt),1,stream);
	}
	if (rdCnt != 1)
	{
		if (CS_ferror (stream))
		{ 
			CS_erpt (cs_IOERR);
		}
		goto error;
	}

	allocSize = ctDefPtr->allocCnt * sizeof (struct cs_CtItmName_);
	ctDefPtr->csNames = CS_malc (allocSize);
	if (ctDefPtr->csNames == NULL)
	{
		CS_erpt (cs_NO_MEM);
		goto error;
	}
	for (idx = 0;idx < ctDefPtr->allocCnt;idx += 1)
	{
		destPtr = (ctDefPtr->csNames + idx);
		rdCnt = CS_fread (destPtr,sizeof (struct cs_CtItmName_),1,stream);
		if (rdCnt != 1)
		{
			CS_erpt (cs_IOERR);
			goto error;
		}
	}
	return ctDefPtr;
error:
	if (ctDefPtr != NULL)
	{
		if (ctDefPtr->csNames != NULL)
		{
			CS_free (ctDefPtr->csNames);
			ctDefPtr->csNames = NULL;
		}
		CS_free (ctDefPtr);
		ctDefPtr = NULL;
	}
	return NULL;
}
struct cs_Ctdef_* EXP_LVL3 CSrdCatFile ()
{
	extern int cs_Error;
	extern char cs_Dir [];
	extern char* cs_DirP;
	extern char cs_Ctname  [];

	struct cs_Ctdef_* ctDefHead;

	size_t rdCnt;
	cs_magic_t magic;
	csFILE* stream;
	struct cs_Ctdef_ *ctDefPtr;
	struct cs_Ctdef_ *ctDefPtrSrch;

	ctDefPtr = NULL;
	ctDefHead = NULL;
	CS_stcpy (cs_DirP,cs_Ctname);
	stream = CS_fopen (cs_Dir,_STRM_BINRD);
	if (stream == NULL)
	{	
		CS_erpt (cs_CSDICT);
		goto error;
	}
	rdCnt = CS_fread (&magic,sizeof (magic),1,stream);
	if (rdCnt != 1 || magic != cs_CTDEF_MAGIC)
	{	
		CS_erpt (cs_CS_BAD_MAGIC);
		goto error;
	}
	while (TRUE)
	{
		cs_Error = 0;
		ctDefPtr = CSrdCategory (stream);
		if (ctDefPtr == NULL)
		{
			if (cs_Error != 0)
			{
				goto error;
			}
			break;
		}

		/* Add it to the existing category. */
		if (ctDefHead == NULL)
		{
			ctDefHead = ctDefPtr;
		}
		else
		{
			ctDefPtrSrch = ctDefHead;
			while (ctDefPtrSrch->next != NULL)
			{
				ctDefPtrSrch = ctDefPtrSrch->next;
			}
			ctDefPtrSrch->next = ctDefPtr;
			ctDefPtr = NULL;
		}
	}
	if (stream != NULL)
	{
		CS_fclose (stream);
	}
	return ctDefHead;
error:
	if (ctDefPtr != NULL)			/*lint !e774 */
	{
		CSrlsCategory (ctDefPtr);
		ctDefPtr = NULL;
	}
	if (ctDefHead != NULL)
	{
		CSrlsCategoryList (ctDefHead);
		ctDefHead = NULL;
	}
	if (stream != NULL)
	{
		CS_fclose (stream);
		stream = NULL;
	}
	return NULL;
}
int EXP_LVL3 CSwrtCatFile (csFILE* stream,Const struct cs_Ctdef_ *ctDefPtr)
{
	int st;

	st = 0;
	while (st >= 0 && ctDefPtr != NULL)
	{
		st = CSwrtCategory (stream,ctDefPtr);
		ctDefPtr = ctDefPtr->next;
	};
	return st;
}
void CSrlsCategory (struct cs_Ctdef_ *ctDefPtr)
{
	if (ctDefPtr != NULL)
	{
		if (ctDefPtr->csNames != NULL)
		{
			CS_free (ctDefPtr->csNames);
		}
		CS_free (ctDefPtr);
	}
}
void EXP_LVL3 CSrlsCategoryList (struct cs_Ctdef_ *ctDefHead)
{
	struct cs_Ctdef_ *tmpPtr;
	
	while (ctDefHead != NULL)
	{
		tmpPtr = ctDefHead;
		ctDefHead = ctDefHead->next;
		CSrlsCategory (tmpPtr);
	}
}
void EXP_LVL3 CSrlsCategories ()
{
	extern struct cs_Ctdef_* cs_CtDefHead;

	struct cs_Ctdef_ *ctDefPtr;
	struct cs_Ctdef_ *nxtCtDefPtr;
	
	ctDefPtr = cs_CtDefHead;
	while (ctDefPtr != NULL)
	{
		nxtCtDefPtr = ctDefPtr->next;
		CSrlsCategory (ctDefPtr);
		ctDefPtr = nxtCtDefPtr;
	}
}

/**********************************************************************
**	err_cnt = CSctcomp (inpt,outp,flags,err_func);
**
**	char *inpt;					full pathe name to the ASCII source file.
**	char *outp;					full path name to the target file.
**	int flags;					bitmat of options, see REMARKS below.
**	int (*err_func)(char *mesg);function called top present error messages
**								to the user.
**	int err_cnt;				returns the number of errors reported.
**
**	The flags argument:
**	cs_CMPLR_WARN   -- emit warning messages.
**
**	Please excuse the rather crude nature of this program.  However,
**	it is necessary for this program to be compiled, linked, and
**	run in just about any environment, without requiring users
**	to license LEX/YACC.
**********************************************************************/

int EXP_LVL9 CSctcomp (	Const char *inpt,Const char *outp,int flags,Const char *coordsys,
																		  int (*err_func)(char *mesg))
{
	int st;
	int demo;
	int test;
	int crypt;
	int warn;
	int cancel;
	int line_nbr;
	int err_cnt;
	int length;
	size_t rdCnt;
	size_t wrCnt;

	short csChk;

	char *cp;
	char *cp1;
	csFILE *inStrm;
	csFILE *outStrm;
	csFILE *csStrm;

	struct cs_Ctdef_ *ctDefHead = NULL;

	cs_magic_t magic;
	struct cs_CtItmName_ dummy;

	char err_seg [18];
	char buff [512];
	char err_msg [128];
	char catName [64];
	char itmName [sizeof (dummy.csName)];
	
	enum lineType {typCatName, typItmName, typBogus} lineType;

	/* Initialize */
	err_cnt = 0;
	cancel = FALSE;
	line_nbr = 0;
	warn = ((flags & cs_CMPLR_WARN) != 0);
	catName [0] = '\0';
	itmName [0] = '\0';

	demo  = ((flags & cs_CMPLR_DEMO)  != 0);
	crypt = ((flags & cs_CMPLR_CRYPT) != 0);
	test  = ((flags & cs_CMPLR_TEST)  != 0);
	warn  = ((flags & cs_CMPLR_WARN)  != 0);

	/* Release and reset any existing category list.  Should not be
	   necssary; rather defensive. */
	CSgetCtDefHead (cs_CATDEF_RELEASE);

	/* OK, lets do it.  Open the source file. */
	inStrm = CS_fopen (inpt,_STRM_TXTRD);
	if (inStrm == NULL)
	{
		err_cnt += 1;
		sprintf (err_msg,"Couldn't open %s for input.",inpt);
		cancel = (*err_func)(err_msg);
		return err_cnt;
	}

	/* If we have been given a coordinate system file name, we open it now. */
	if (coordsys != NULL && *coordsys != '\0')
	{
		csStrm = CS_fopen (coordsys,_STRM_BINRD);
		if (csStrm == NULL)
		{
			sprintf (err_msg,"Couldn't open %s as a datum file.",coordsys);
			(*err_func)(err_msg);
			CS_fclose (inStrm);
			return (1);
		}
		rdCnt = CS_fread (&magic,1,sizeof (magic),csStrm);
		CS_bswap (&magic,"l");
		if (rdCnt != sizeof (magic) ||
			(demo && magic != cs_CSDEF_MAGIC) ||
			(!demo && magic != cs_CSDEF_MAGIC))
		{
			sprintf (err_msg,"%s is not a Coordinate System Dictionary file.",coordsys);
			(*err_func)(err_msg);
			CS_fclose (inStrm);
			CS_fclose (csStrm);
			return (1);
		}
	}
	else
	{
		csStrm = NULL;
	}

	/* Process each line in the source file. */
	while (CS_fgets (buff,sizeof (buff),inStrm) != NULL)
	{
		if (cancel)
		{
			CS_fclose (inStrm);
			if (csStrm != NULL) CS_fclose (csStrm);
			CSgetCtDefHead (cs_CATDEF_RELEASE);
			return (err_cnt);
		}
		line_nbr += 1;

		/* Ignore comments and blank lines. */
		(void)CS_trim (buff);
		if (buff [0] == ';' || buff [0] == '\0')
		{
			continue;
		}
		cp = buff;
		while ((cp = strchr (cp,';')) != NULL)
		{
			if (*(cp + 1) != ';' &&
				*(cp - 1) != '\\')
			{
				*cp = '\0';
				break;
			}
		}

		/* Determine the type of this line.  Since we have already handled
		   comment lines, the line type can be one of three types:
			1> Category name line, i.e. has [...]
			2> Item name, i.e. AAAA = ....
			3> Bogus line, i.e. anything else  */
		lineType = typBogus;				/* until we know different */
		cp = strchr (buff,'[');
		cp1 = strchr (buff,'=');
		if (cp != NULL)
		{
			/* It's a category name or it's bogus. */
			cp1 = strchr (buff,']');
			if (cp1 != NULL)
			{
				length = (int)(cp1 - cp) - 1;
				if (length > 1 && length < (int)sizeof (catName))
				{
					lineType = typCatName;
					cp += 1;
					CS_stncp (catName,cp,length + 1);
				}
			}
			/* Linetype already set to bogus. */
		}
		else if (cp1 != NULL)
		{
			/* Looks like an item name.  Let's see. */
			*cp1 = '\0';
			CS_trim (buff);
			if (strlen (buff) < sizeof (dummy.csName))
			{
				/* Looks good, should be an item name. */
				lineType = typItmName;
				CS_stncp (itmName,buff,sizeof (itmName));
			}
		}
		/* lineType already set to bogus above. */

		if (lineType == typBogus)
		{
			err_cnt += 1;
			CS_stncp (err_seg,buff,sizeof (err_seg));
			sprintf (err_msg,"Invalid format detected (%s) at line %d.",err_seg,line_nbr);
			cancel = (*err_func)(err_msg);
			continue;
		}

		/* OK, deal with the current line. */
		if (lineType == typCatName)
		{
			/* Start a new category. */
			st = CSaddCategory (catName);
			if (st != 0)
			{
				sprintf (err_msg,"Category memory allocation failed at line %d.",line_nbr);
				cancel = (*err_func)(err_msg);
				continue;
			}
		}
		else if (lineType == typItmName)
		{
			/* Verify that we have a valid category started. */
			if (catName [0] == '\0')
			{
				err_cnt += 1;
				CS_stncp (err_seg,buff,sizeof (err_seg));
				sprintf (err_msg,"Item name detected outside of category at line %d (%s).",line_nbr,err_seg);
				cancel = (*err_func)(err_msg);
				continue;
			}

			/* Verify that the item name is that of a coordinate system. */
			csChk = CSctCompCsChk (csStrm,itmName);
			if (csChk < 0)
			{
				/* The coordinate system does not exists, for wehatever reason. */
				continue;
			}
			if (csChk > 0)
			{
				/* The coordinate system is classified as being in the LEGACY group. */
				if (CS_strnicmp (catName,"Obsolete",8))
				{
					/* The category name is not the Obsolete Coordinate System Group. */
					continue;
				}
			}

			/* Add the new item name to the current category. */
			st = CSaddItmName (catName,itmName);
			if (st != 0)
			{
				err_cnt += 1;
				CS_stncp (err_seg,buff,sizeof (err_seg));
				sprintf (err_msg,"Addition of item name (%s) at line %d failed.",err_seg,line_nbr);
				cancel = (*err_func)(err_msg);
				continue;
			}
		}
	}
	CS_fclose (inStrm);
	if (csStrm != NULL) CS_fclose (csStrm);

	if (!cancel)
	{
		/* Open the output file and write the magic number.  BINRW creats a
		   new file (truncates if necessary) with read write access.  We
		   need read/write as we will be sorting below. */
		outStrm = CS_fopen (outp,_STRM_BINWR);
		if (outStrm == NULL)
		{
			err_cnt += 1;
			sprintf (err_msg,"Couln't open %s for output.",outp);
			cancel = (*err_func)(err_msg);
			return err_cnt;
		}

		magic = cs_CTDEF_MAGIC;
		CS_bswap (&magic,"l");
		wrCnt = CS_fwrite (&magic,1,sizeof (magic),outStrm);
		if (wrCnt != sizeof (magic))
		{
			err_cnt += 1;
			sprintf (err_msg,"Write failure on %s.",outp);
			cancel = (*err_func)(err_msg);
			CS_fclose (outStrm);
			CS_remove (outp);										/*lint !e534 */
			return err_cnt;
		}

		st = 0;
		ctDefHead = CSgetCtDefHead (cs_CATDEF_ASIS);
		if (ctDefHead != NULL)
		{
			st = CSwrtCatFile (outStrm,ctDefHead);
		}
		ctDefHead = CSgetCtDefHead (cs_CATDEF_RELEASE);
		CS_fclose (outStrm);
		if (st != 0)
		{
			err_cnt += 1;
			sprintf (err_msg,"Category file write failure on %s.",outp);
			cancel = (*err_func)(err_msg);
			CS_remove (outp);										/*lint !e534 */
		}
	}

	/* Close up and get out. */
	return err_cnt;	
}
/* The following verifies that the proposed coordinate system name exists.
   Returns -1 if it doesn't exist, +1 if it exists and is in the LEGACY
   group.  Otherwise a zero is returned.  In the absence of a coordinate
   system file stream, a zero is always returned.*/
short CSctCompCsChk (csFILE *csStrm,Const char* csName)
{
	short rtnValue;
	int flag;
	size_t rdCnt;
	struct cs_Csdef_ cs_def;

	rtnValue = 0;	
	if (csStrm != NULL)
	{
		CS_stncp (cs_def.key_nm,csName,sizeof (cs_def.key_nm));
		CS_nampp (cs_def.key_nm);
		cs_def.fill [0] = '\0';
		cs_def.fill [1] = '\0';
		flag = CS_bins (csStrm,(long32_t)sizeof (cs_magic_t),(long32_t)-1,sizeof (cs_def),&cs_def,(CMPFUNC_CAST)CS_cscmp);
		if (flag)
		{
			rdCnt = CS_fread (&cs_def,sizeof (cs_def),1,csStrm);
			if (rdCnt == 1)
			{
				if (!CS_stricmp (cs_def.group,"LEGACY"))
				{
					rtnValue = 1;
				}
			}
		}
		else
		{
			rtnValue = -1;
		}
	}
	return rtnValue;
}
