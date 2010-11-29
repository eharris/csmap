/*
 * Copyright (c) 2008, Autodesk, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *	* Redistributions of source code must retain the above copyright
 *	  notice, this list of conditions and the following disclaimer.
 *	* Redistributions in binary form must reproduce the above copyright
 *	  notice, this list of conditions and the following disclaimer in the
 *	  documentation and/or other materials provided with the distribution.
 *	* Neither the name of the Autodesk, Inc. nor the names of its
 *	  contributors may be used to endorse or promote products derived
 *	  from this software without specific prior written permission.
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

/**********************************************************************
**	Calling Sequence:	strm = CS_gpopn (mode);
**
**	int mode;					mode of the file open, ala fcntl.h
**	csFILE *strm;				returns a file descriptor to the open file.
**
**	Will return NULL if a problem was encountered.
**
**	File is positioned past the magic number on the front.
**********************************************************************/
csFILE * EXP_LVL3 CS_gpopn (Const char *mode)
{
	extern char cs_Dir [];
	extern char *cs_DirP;
	extern char cs_Gpname [];
	extern char csErrnam [];

	size_t rdCnt;

	csFILE *strm = NULL;

	cs_magic_t magic;

	strcpy (cs_DirP,cs_Gpname);
	strm = CS_fopen (cs_Dir,mode);
	if (strm != NULL)
	{
		rdCnt = CS_fread ((char *)&magic,1,sizeof (magic),strm);
		if (rdCnt != sizeof (magic))
		{
			if (CS_ferror (strm)) CS_erpt (cs_IOERR);
			else                  CS_erpt (cs_INV_FILE);
			CS_fclose (strm);
			strm = NULL;
			strcpy (csErrnam,cs_Dir);
		}
		else
		{ 
			CS_bswap (&magic,"l");
			if (magic != cs_GPDEF_MAGIC)
			{
				CS_fclose (strm);
				strm = NULL;
				strcpy (csErrnam,cs_Dir);
				CS_erpt (cs_GP_BAD_MAGIC);
			}
		}
	}
	else
	{
		strcpy (csErrnam,cs_Dir);
		CS_erpt (cs_GPDICT);
	}
	return (strm);
}

/**********************************************************************
**	flag = CS_gprd (strm,gp_rec);
**
**	csFILE strm;				file/device from which the possibly enrypted
**								ellipsoid definition is to be read.
**	struct cs_GeodeticPathDef_ *gp_rec;
**								next geodetic path definition record is returned here.
**	int flag;					returns +1 for successful read, 0 for EOF,
**								-1 for error.
**********************************************************************/
int EXP_LVL3 CS_gprd (csFILE *strm,struct cs_GeodeticPath_ *gp_def)
{
	int st;
	size_t rdCnt;

	char tmpKeyName [64];

	/* Synchronize the strm before the read. */
	st = CS_fseek (strm,0L,SEEK_CUR);
	if (st != 0)
	{
		CS_erpt (cs_IOERR);
		return (-1);
	}

	/* OK, now we can read. */
	rdCnt = CS_fread ((char *)gp_def,1,sizeof (*gp_def),strm);
	if (rdCnt != sizeof (*gp_def))
	{
		if (CS_feof (strm))
		{
			return 0;
		}
		if (CS_ferror (strm)) CS_erpt (cs_IOERR);
		else				  CS_erpt (cs_INV_FILE);
		return (-1);
	}

	/* Swap the bytes if necessary. */
	CS_bswap ((void *)gp_def,cs_BSWP_GPDEF);

	/* Check the result. The name must always meet the criteria
	   set by the CS_nmpp64 function.  At least so far, the criteria
	   established by CS_nampp over the years has always been
	   expanded, never restricted.  Thus, any definition which
	   was legitimate in a previous release would always be
	   legitimate in subsequent releases. */
	CS_stncp (tmpKeyName,gp_def->pathName,sizeof (tmpKeyName));
	if (CS_nampp64 (tmpKeyName) != 0)
	{
		/* Replace the error condition reported by CS_nampp with
		   and Invalid File indication. */
		CS_erpt (cs_INV_FILE);
		return (-1);
	}
	CS_stncp (tmpKeyName,gp_def->srcDatum,sizeof (tmpKeyName));
	if (CS_nampp (tmpKeyName) != 0)
	{
		/* Replace the error condition reported by CS_nampp with
		   and Invalid File indication. */
		CS_erpt (cs_INV_FILE);
		return (-1);
	}
	CS_stncp (tmpKeyName,gp_def->trgDatum,sizeof (tmpKeyName));
	if (CS_nampp (tmpKeyName) != 0)
	{
		/* Replace the error condition reported by CS_nampp with
		   and Invalid File indication. */
		CS_erpt (cs_INV_FILE);
		return (-1);
	}
	return (1);
}
/**********************************************************************
**	st = CS_gpwr (strm,gp_rec);
**
**	csFILE *strm;				file/device to which the possibly enrypted
**								ellipsoid definition is to be written.
**	struct cs_GeodeticPathDef_ *gp_rec;
**								the geodetic path definition record which is
**								to be written.
**	int st;						returns FALSE if write was completed successfully,
**								else returns TRUE.
**********************************************************************/
int EXP_LVL3 CS_gpwr (csFILE *strm,Const struct cs_GeodeticPath_ *gp_def)
{
	int st;
	size_t wrCnt;

	__ALIGNMENT__1		/* For some versions of Sun compiler. */
	struct cs_GeodeticPath_ lcl_gpDef;

	/* Get a copy which we can modify without screwing up the
	   calling module. */
	memcpy ((char *)&lcl_gpDef,(char *)gp_def,sizeof (lcl_gpDef));

	/* Swap bytes as necessary */
	CS_bswap ((void*)&lcl_gpDef,cs_BSWP_GPDEF);

	/* Synchronize the stream prior to the write. */
	st = CS_fseek (strm,0L,SEEK_CUR);
	if (st != 0)
	{
		CS_erpt (cs_IOERR);
		return TRUE;		
	}

	/* Now we can write. */
	wrCnt = CS_fwrite ((char *)&lcl_gpDef,1,sizeof (lcl_gpDef),strm);
	if (wrCnt != sizeof (lcl_gpDef))
	{
		if (CS_ferror (strm)) CS_erpt (cs_IOERR);
		else				  CS_erpt (cs_DISK_FULL);
		return (TRUE);
	}
	return (FALSE);
}

/**********************************************************************
**	st = CS_gpdel (gpdef);
**
**	struct cs_GeodeticPath_ *gpdef;
**								a pointer to the geodetic path definition
**								structure which is to be deleted from the
**								Geodetic Path Dictionary.
**	int st;						returns a zero if the delete was successfully
**								completed, else returns -1.
**********************************************************************/
int EXP_LVL3 CS_gpdel (struct cs_GeodeticPath_ *gp_def)
{
	extern char csErrnam [];
	extern char cs_Dir [];
	extern short cs_Protect;

	short cs_time;

	int st;
	int rdSt;
	size_t wrCnt;

	csFILE *old_strm;
	csFILE *new_strm;

	cs_magic_t magic;

	struct cs_GeodeticPath_ *my_ptr;

	char tmp_nam [MAXPATH];

	__ALIGNMENT__1		/* For some versions of Sun compiler. */
	struct cs_GeodeticPath_ keyBufr;

	__ALIGNMENT__2		/* For some versions of Sun compiler. */
	struct cs_GeodeticPath_ cpy_buf;

	/* Capture the current time. */
	cs_time = (short)((CS_time ((cs_Time_ *)0) - 630720000L) / 86400L);

	/* Prepare for an error. */
	new_strm = NULL;
	old_strm = NULL;
	my_ptr = NULL;

	/* Set up a key structure which we will use to identify all path records
	   which belong to the definition which is to be deleted. */
	memset ((void *)&keyBufr,0,sizeof (keyBufr));
	CS_stncp (keyBufr.pathName,gp_def->pathName,sizeof (keyBufr.pathName));

	/* Get a pointer to the existing definition. If it doesn't
	   exist, we're all done. */
	my_ptr = CS_gpdef (keyBufr.pathName);
	if (my_ptr == NULL)
	{
		/* If it doesn't exist, it can't be deleted. */
		goto error;
	}

	/* See if this definition is protected.  If so, we have to
	   leave it alone. If cs_Protect < 0, there is no protection. */
	if (cs_Protect >= 0)
	{
		if (my_ptr->protect == 1)
		{
			CS_stncp (csErrnam,my_ptr->pathName,MAXPATH);
			CS_erpt (cs_GP_PROT);
			goto error;
		}
		if (cs_Protect > 0)
		{
			/* Here if user definition protection is
			   enabled. */
			if (my_ptr->protect < (cs_time - cs_Protect))
			{
				CS_stncp (csErrnam,my_ptr->pathName,MAXPATH);
				CS_erpt (cs_GP_UPROT);
				goto error;
			}
		}
	}
	CS_free (my_ptr);
	my_ptr = NULL;

	/* Open up the geodetic path dictionary file and verify its
	   magic number. */
	old_strm = CS_gpopn (_STRM_BINRD);
	if (old_strm == NULL)
	{
		goto error;
	}

	/* Create a temporary file for the new dictionary. */
	st = CS_tmpfn (tmp_nam);
	if (st != 0)
	{
		goto error;
	}
	new_strm = CS_fopen (tmp_nam,_STRM_BINWR);
	if (new_strm == NULL)
	{
		CS_erpt (cs_TMP_CRT);
		goto error;
	}

	/* Copy the file, skipping the entry to be deleted.  First
	   we must write the magic number. */
	magic = cs_GPDEF_MAGIC;
	CS_bswap (&magic,"l");
	wrCnt = CS_fwrite ((char *)&magic,1,sizeof (magic),new_strm);
	if (wrCnt != sizeof (magic))
	{
		if (CS_ferror (new_strm)) CS_erpt (cs_IOERR);
		else					  CS_erpt (cs_DISK_FULL);
		goto error;
	}

	/* Now we copy the file, skipping the entry to be
	   deleted. */
	while ((rdSt = CS_gprd (old_strm,&cpy_buf)) > 0)
	{
		if (CS_gpcmp (&cpy_buf,&keyBufr) != 0)
		{
			if (CS_gpwr (new_strm,&cpy_buf))
			{
				goto error;
			}
		}
	}

	/* See if the copy loop terminated due to an error. */
	if (rdSt != 0) goto error;

	/* Close up, remove the old dictionary and rename the
	   new dictionary. */
	CS_fclose (new_strm);
	new_strm = NULL;
	CS_fclose (old_strm);
	old_strm = NULL;
	st = CS_remove (cs_Dir);
	if (st != 0)
	{
		strcpy (csErrnam,cs_Dir);
		CS_erpt (cs_UNLINK);
		goto error;
	}
	st = CS_rename (tmp_nam,cs_Dir);
	if (st != 0) goto error;

	/* We're done. */
	return (0);

error:
	if (new_strm != NULL)
	{
		/* tmp_nam must have been initialize if new_strm >= 0 */
		CS_fclose (new_strm);
		CS_remove (tmp_nam);						/*lint !e534 !e645 */
	}
	if (old_strm != NULL) CS_fclose (old_strm);
	if (my_ptr != NULL) CS_free (my_ptr);
	return (-1);
}

/******************************************************************************
**	flag = CS_gpupd (eldef,crypt);
**
**	struct cs_GeodeticPath_ *gpdef;
**								a pointer to the ellipsoid definition
**								structure which is to be written to the
**								Geodetic Path Dictionary.
**	int flag;					returns TRUE (+1) if the indicated geodetic
**								path previously existed and was simply
**								updated, returns FALSE (0) if the geodetic
**								path had to be added as a new path, returns
**								-1 if the update process failed.
**
**	If the Geodetic Path Dictionary does not already contain an entry
**	with the indicated key name, the entry is added.
*******************************************************************************/
int EXP_LVL3 CS_gpupd (struct cs_GeodeticPath_ *gp_def)
{
	extern char csErrnam [];
	extern short cs_Protect;
	extern char cs_Unique;

	short cs_time = 0;

	int st;
	int flag;
	int sort;

	long32_t fpos;

	char *cp;
	csFILE *strm;

	__ALIGNMENT__1		/* For some versions of Sun compiler. */
	struct cs_GeodeticPath_ my_gpdef;

	/* Prepare for a possible error. */
	strm = NULL;
	sort = FALSE;
	fpos = 0L;

	/* Compute the time, as we use it internally.  This is
	   days since January 1, 1990. If this record does get
	   written, we want it to have the current date in it.
	   This means that even if allowed by the protection system,
	   a distribution coordinate system will be marked as having
	   been twiddled. */
	if (gp_def->protect >= 0)
	{
		if ((cs_Protect < 0) || (gp_def->protect != 1))
		{
			cs_time = (short)((CS_time ((cs_Time_ *)0) - 630720000L) / 86400L);
			/* This modifies the user supplied definition. */
			gp_def->protect = cs_time;
		}
	}

	/* Adjust the name and make sure it is proper.  By convention, geodetic
	   datum names are case insensitive. */
	st = CS_nampp64 (gp_def->pathName);
	if (st != 0) goto error;

	/* Open up the Geodetic Path Dictionary and verify its magic number. */
	strm = CS_gpopn (_STRM_BINUP);
	if (strm == NULL)
	{
		goto error;
	}

	/* See if we have a geodetic path with this name already. */
	flag = CS_bins (strm,(long32_t)sizeof (cs_magic_t),(long32_t)0,sizeof (*gp_def),(char *)gp_def,(CMPFUNC_CAST)CS_gpcmp);
	if (flag < 0) goto error;
	if (flag)
	{
		/* Here when the geodetic path already exists.  See if it is OK to
		   write it. If cs_Protect is less than zero, all protection has
		   been turned off. */
		if (cs_Protect >= 0)
		{
			fpos = CS_ftell (strm);
			if (fpos < 0L)
			{
				CS_erpt (cs_IOERR);
				goto error;
			}
			st = CS_gprd (strm,&my_gpdef);
			if (st == 0)
			{
				CS_erpt (cs_INV_FILE);
			}
			if (st <= 0)
			{
				goto error;
			}
			if (my_gpdef.protect == 1)
			{
				/* We don't allow distribution geodetic path definitions
				   to be overwritten, period. */
				CS_stncp (csErrnam,my_gpdef.pathName,MAXPATH);
				CS_erpt (cs_GP_PROT);
				goto error;
			}
			if (cs_Protect > 0 && my_gpdef.protect > 0)
			{
				/* We protect user defined geodetic paths only if cs_Protect is greater
				   than zero. */
				if (my_gpdef.protect < (cs_time - cs_Protect))		/*lint !e644 */
				{
					/* It has been more than cs_Protect days since this geodetic
					   path has been twiddled, we consider it to be protected. */
					CS_stncp (csErrnam,gp_def->pathName,MAXPATH);
					CS_erpt (cs_GP_UPROT);
					goto error;
				}
			}
			st = CS_fseek (strm,fpos,SEEK_SET);
			if (st != 0)
			{
				CS_erpt (cs_IOERR);
				goto error;
			}
		}

		/* OK, if we're still here, it's OK to overwrite the existing
		   definition. */
		st = CS_gpwr (strm,gp_def);
	}
	else
	{
		/* Here if the geodetic path definition doesn't exist.  We
		   have to add it. If cs_Unique is not zero, we require that
		   a cs_Unique character be present in the key name of the path
		   before we'll allow it to be written. */
		if (cs_Unique != '\0')
		{
			cp = strchr (gp_def->pathName,cs_Unique);
			if (cp == NULL)
			{
				csErrnam [0] = cs_Unique;
				csErrnam [1] = '\0';
				CS_erpt (cs_UNIQUE);
				goto error;
			}
		}

		/* If we're still here, we append the new geodetic path definition
		   to the end of the file and then sort the file. */
		st = CS_fseek (strm,fpos,SEEK_END);
		CS_gpwr (strm,gp_def);
		sort = TRUE;
	}
	if (sort)
	{
		/* Sort the file into proper order, thereby
		   moving the new ellipsoid to its proper place
		   in the dictionary. */
		st = CS_fseek (strm,(long)sizeof (cs_magic_t),SEEK_SET);
		if (st != 0)
		{
			CS_erpt (cs_IOERR);
			goto error;
		}
		st = CS_ips (strm,sizeof (struct cs_GeodeticPath_),0L,(CMPFUNC_CAST)CS_gpcmp);
		if (st < 0) goto error;
	}

	/* The Geodetic Path Dictionary has been updated. */
	CS_fclose (strm);
	return (flag);

error:
	if (strm != NULL) CS_fclose (strm);
	return (-1);
}

/**********************************************************************
**	cmp_val = CS_gpcmp (pp,qq);
**
**	struct cs_GeodeticPathDef_ *pp;
**	struct cs_GeodeticPathDef_ *qq;
**								The geodetic path definition records which
**								are to be compared.
**	int cmp_val;				return is negative if pp follows qq in the
**								collating sequence, positive if pp preceeds
**								qq, and zero if they are equivalent.
**
** Compares only the three elements required to make the key value of
** each record unique.  Thus, rcrdCnt and protect are not in cluided
** in thecomparison.
**********************************************************************/
int EXP_LVL7 CS_gpcmp (Const struct cs_GeodeticPath_ *pp,
					   Const struct cs_GeodeticPath_ *qq)
{
	int st;

	/* OK, now we can compare the two structures.  For sorting
	   and binary search purposes, we only look at the source and
	   traget key names and the record number. */

	st = CS_stricmp (pp->pathName,qq->pathName);
	return (st);
}
/******************************************************************************
**	gp_ptr = CS_gpdef (Const char* pathName);
**
**	char *pathName;				key name of the geodetic path definition
**								which is to be fetched.
**	struct cs_GeodeticPath_ *gp_ptr;
**								returns a pointer to a malloc'ed geodetic path
**								definition structure.  Calling module owns the
**								returned structure and is responsible for
**								freeing it.
**
**	This function only fetches the geodetic path definition as recorded in
**	the geodetic path dictionary.
**********************************************************************/
struct cs_GeodeticPath_ * EXP_LVL3 CS_gpdef (Const char *pathName)
{
	extern char csErrnam [];

	int st;
	int flag;

	csFILE *strm;
	struct cs_GeodeticPath_ *gp_def;

	char tmpKeyName [64];

	__ALIGNMENT__1				/* For some versions of Sun compiler. */
	struct cs_GeodeticPath_ srchDef;

	/* Prepare for the potential error condition. */
	strm = NULL;
	gp_def = NULL; 

	/* Make sure the provided name is OK. */
	CS_stncp (tmpKeyName,pathName,sizeof (tmpKeyName));
	st = CS_nampp64 (tmpKeyName);
	if (st != 0) goto error;

	/* Open up the geodetic path dictionary file. */
	strm = CS_gpopn (_STRM_BINRD);
	if (strm == NULL) goto error;

	/* Search the file for the requested ellipsoid definition. */
	CS_stncp (srchDef.pathName,pathName,sizeof (srchDef.pathName));
	flag = CS_bins (strm,(long32_t)sizeof (cs_magic_t),(long32_t)0,sizeof (srchDef),(char *)&srchDef,(CMPFUNC_CAST)CS_gpcmp);
	if (flag < 0) goto error;

	if (!flag)
	{
		CS_stncp (csErrnam,pathName,MAXPATH);
		CS_erpt (cs_GP_NOT_FND);
		goto error;
	}
	else
	{
		/* The geodetic path definition exists.  Therefore, we malloc the
		   geodetic path definition structure and read it in. */
		gp_def = (struct cs_GeodeticPath_ *)CS_malc (sizeof (struct cs_GeodeticPath_));
		if (gp_def == NULL)
		{
			CS_erpt (cs_NO_MEM);
			goto error;
		}
		memset ((void*)gp_def,0,sizeof (*gp_def));
		if (!CS_gprd (strm,gp_def))
		{
			goto error;
		}
	}
	if (strm != NULL)
	{
		CS_fclose (strm);
		strm = NULL;
	}

	/* Return a pointer to the malloc'ed geodetic path definition to the
	   user. */
	return (gp_def);

error:
	if (strm != NULL) CS_fclose (strm);
	if (gp_def != NULL)
	{
		CS_free (gp_def);
		gp_def = NULL;
	}
	return (gp_def);
}

/**********************************************************************
**	gp_ptr = CS_gpdefEx (Const char* srcDatum,Const char* trgDatum);
**
**	char *srcDatum;				key name of the source datum of the path
**								definition which is to be fetched.
**	char *trgDatum;				key name of the target datum of the path
**								definition which is to be fetched.
**	struct cs_GeodeticPath_ *gp_ptr;
**								returns a pointer to a malloc'ed ellipsoid definition
**								structure.
**
**********************************************************************/
struct cs_GeodeticPath_ * EXP_LVL3 CS_gpdefEx (int* direction,
											   Const char *srcDatum,
											   Const char *trgDatum)
{
	extern char csErrnam [];

	int st;

	long curFpos;
	long fwdFpos;
	long invFpos;

	csFILE *strm;
	struct cs_GeodeticPath_ *gp_def;

	char tmpKeyName [64];
	char errMsg [256];

	__ALIGNMENT__1				/* For some versions of Sun compiler. */
	struct cs_GeodeticPath_ gp_rec;

	/* Prepare for the potential error condition. */
	strm = NULL;
	gp_def = NULL;
	*direction = cs_DTCDIR_NONE;

	/* Make sure the provided names are OK. */
	CS_stncp (tmpKeyName,srcDatum,sizeof (tmpKeyName));
	st = CS_nampp (tmpKeyName);
	if (st != 0) goto error;
	CS_stncp (tmpKeyName,trgDatum,sizeof (tmpKeyName));
	st = CS_nampp (tmpKeyName);
	if (st != 0) goto error;

	/* Open up the geodetic path dictionary file. */
	strm = CS_gpopn (_STRM_BINRD);
	if (strm == NULL) goto error;

	/* Search the file for the requested ellipsoid definition. */
	fwdFpos = 0L;
	invFpos = 0L;
	for (;;)
	{
		curFpos = CS_ftell (strm);
		st = CS_gprd (strm,&gp_rec);
		if (st < 0)
		{
			goto error;	
		} 
		if (st == 0)
		{
			break;
		}
		
		/* See if we have a match in the forward direction. */
		if (!CS_stricmp (gp_rec.srcDatum,srcDatum) &&
		    !CS_stricmp (gp_rec.trgDatum,trgDatum))
		{
			if (fwdFpos == 0L)
			{
				fwdFpos = curFpos;
			}
			else
			{
				sprintf (errMsg,"'%s' to '%s'",srcDatum,trgDatum);
				CS_stncp (csErrnam,errMsg,MAXPATH);
				CS_erpt (cs_GEOPATH_DUP);
				goto error;
			}
		}
		/* See if we have a match in the inverse direction. */
		if (!CS_stricmp (gp_rec.srcDatum,trgDatum) &&
		    !CS_stricmp (gp_rec.trgDatum,srcDatum))
		{
			if (invFpos == 0L)
			{
				invFpos = curFpos;
			}
			else
			{
				sprintf (errMsg,"'%s' to '%s'",srcDatum,trgDatum);
				CS_stncp (csErrnam,errMsg,MAXPATH);
				CS_erpt (cs_GEOPATH_DUP);
				goto error;
			}
		}
	}

	/* We always return the forward definition if we found one. */
	if (fwdFpos != 0L)
	{
		gp_def = (struct cs_GeodeticPath_ *)CS_malc (sizeof (struct cs_GeodeticPath_));
		if (gp_def == NULL)
		{
			CS_erpt (cs_NO_MEM);
			goto error;
		}
		memset ((void*)gp_def,0,sizeof (*gp_def));
		st = CS_fseek (strm,fwdFpos,SEEK_SET);
		if (st != 0)
		{
			CS_erpt (cs_IOERR);
			goto error;
		}
		if (!CS_gprd (strm,gp_def))
		{
			goto error;
		}
		else
		{
			*direction = cs_DTCDIR_FWD;
		}
	}
	else if (invFpos != 0L)
	{
		gp_def = (struct cs_GeodeticPath_ *)CS_malc (sizeof (struct cs_GeodeticPath_));
		if (gp_def == NULL)
		{
			CS_erpt (cs_NO_MEM);
			goto error;
		}
		memset ((void*)gp_def,0,sizeof (*gp_def));
		st = CS_fseek (strm,invFpos,SEEK_SET);
		if (st != 0)
		{
			CS_erpt (cs_IOERR);
			goto error;
		}
		if (!CS_gprd (strm,gp_def))
		{
			goto error;
		}
		else
		{
			*direction = cs_DTCDIR_INV;
		}
	}
	else
	{
		sprintf (errMsg,"'%s' to '%s'",srcDatum,trgDatum);
		CS_stncp (csErrnam,errMsg,MAXPATH);
		CS_erpt (cs_GP_NOPATH);
		goto error;
	}

	if (strm != NULL)
	{
		CS_fclose (strm);
		strm = NULL;
	}

	/* Return a pointer to the malloc'ed geodetic path definition to the
	   user. */
	return (gp_def);

error:
	if (strm != NULL) CS_fclose (strm);
	if (gp_def != NULL)
	{
		CS_free (gp_def);
		gp_def = NULL;
	}
	return (gp_def);
}
int CS_gpchk (Const struct cs_GeodeticPath_ *gpPath,unsigned short gpChkFlg,int err_list [],int list_sz)
{
	short gpIdx;
	
	int st;
	int gxIndex;
	int err_cnt;

	struct cs_GeodeticPathElement_* gpPtr;

	err_cnt = -1;

	if (CS_nampp64 (gpPath->pathName))
	{
		if (++err_cnt < list_sz) err_list [err_cnt] = cs_GPQ_NAME;
	}

	/* Checking the datum names is optional as this code will always check
	   agains the datum dictionary in the standard location.  This may not
	   be appropriate in certain environments such as the dictionary
	   compiler. */
	if ((gpChkFlg & cs_GPCHK_DATUM) != 0)
	{
		/* Check the datums. */
		if (!CS_dtIsValid (gpPath->srcDatum))
		{
			if (++err_cnt < list_sz) err_list [err_cnt] = cs_GPQ_GPDTM;
		}
		if (!CS_dtIsValid (gpPath->trgDatum))
		{
			if (++err_cnt < list_sz) err_list [err_cnt] = cs_GPQ_GPDTM;
		}
	}

	/* Make sure the elementCount is sane before we go producing a memory
	   exception. */
	if (gpPath->elementCount <= 0 || gpPath->elementCount > csPATH_MAXXFRM)
	{
		if (++err_cnt < list_sz) err_list [err_cnt] = cs_GPQ_COUNT;
	}
	else
	{
		for (gpIdx = 0;gpIdx < gpPath->elementCount;gpIdx += 1)
		{
			gpPtr = &gpPath->geodeticPathElements [gpIdx];
			
			/* Check for a valid direction indication. */
			if (gpPtr->direction != cs_DTCDIR_FWD && gpPtr->direction != cs_DTCDIR_INV)
			{
				if (++err_cnt < list_sz) err_list [err_cnt] = cs_GPQ_GXDIR;
			}

			/* Check for a valid transformation name. */
			st = CS_nampp64 (gpPtr->geodeticXformName);
			if (st != 0)
			{
				if (++err_cnt < list_sz) err_list [err_cnt] = cs_GPQ_INVNM;
			}

			/* The following flag check enables us to use this function in the
			   dictionary compiler.  In the compiler, it may be, or may not be,
			   appropriate to check the transformation dictionary for a valid
			   name as the dictionaries being generated may not be the smae ones
			   that standard CS_gx??? functions will access. */
			if ((gpChkFlg & cs_GPCHK_XFORM) != 0)
			{
				/* OK, we have been instructed to check the dictionary for the
				   existence of the transformation name. */
				gxIndex = CS_locateGxByName (gpPtr->geodeticXformName);
				if (gxIndex < 0)
				{
					if (++err_cnt < list_sz) err_list [err_cnt] = cs_GPQ_NOXFRM;
				}
			}
		}
	}
	return (err_cnt + 1);
}
/**********************************************************************
**	CS_gpfnm (new_name);
**
**	char *new_name;		the name of the geodetic path dictionary file.
**
**	This function specifies the name only.  The directory,
**	and possibly drive, are specified using CS_altdr.
**********************************************************************/
void EXP_LVL1 CS_gpfnm (Const char *new_name)
{
	extern char cs_Gpname [];

	(void)CS_stncp (cs_Gpname,new_name,cs_FNM_MAXLEN);
	return;
}
