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

extern csFILE* cs_DtStream;
extern short cs_DtStrmFlg;

/**********************************************************************
 Hook function to support the use of temporary datum definitions.

 To activate, set the global CS_usrDtDefPtr variable to point to your
 function.  To deactivate, set CS_usrDtDefPtr to NULL.
 
 CS_dtdef calls the (*CS_usrDtDefPtr) function before it does anything
 with the name.  Thus, temporary names need not adhere to any CS-MAP
 convention regarding key names.

 (*CS_usrDtDefPtr) should return:
 -1 to indicate an error of sorts, in which case the error condition
    must have already been reported to CS_erpt.
 +1 to indicate that normal CS_dtdef processing is to be performed.
  0 to indicate that the cs_Dtdef_ structure provided by the first
    argument has been properly filled in with the desired definition
	which is to be returned by CS_dtdef.

 In the case where (*CS_usrDtDefPtr) returns 0, CS_dtdef will return
 a copy of the data provided in a chunk of memory it malloc's from
 the heap.  Also note, that the data returned is not checked.  You
 are responsible for making sure this data is valid.
**********************************************************************/
extern int (*CS_usrDtDefPtr) (struct cs_Dtdef_ *dtDef,Const char *keyName);

/**********************************************************************
**	strm = CS_dtopn (mode);
**
**	char *mode;					mode of the file open, ala stdio.h
**	csFILE *strm;				returns a file descriptor to the open file.
**
**	Will return NULL if a problem was encountered.
**
**	File is positioned past the magic number on the front.
**********************************************************************/

csFILE * EXP_LVL3 CS_dtopn (Const char *mode)
{
	extern char cs_Dir [];
	extern char *cs_DirP;
	extern char cs_Dtname [];
	extern char csErrnam [];

	size_t rdCnt;

	csFILE *strm = NULL;

	cs_magic_t magic;

	if (cs_DtStream != 0)
	{
		if (!CS_stricmp (mode,_STRM_BINRD))
		{
			strm = cs_DtStream;
			CS_fseek (strm,(long)sizeof (magic),SEEK_SET);
		}
		else
		{
			CS_fclose (cs_DtStream);
			cs_DtStream = NULL;
		}
	}
	if (strm == NULL)
	{
		strcpy (cs_DirP,cs_Dtname);
		strm = CS_fopen (cs_Dir,mode);
		if (strm != NULL)
		{
			rdCnt = CS_fread ((char *)&magic,1,sizeof (magic),strm);
			if (rdCnt != sizeof (magic))
			{
				if (CS_ferror (strm)) CS_erpt (cs_IOERR);
				else				  CS_erpt (cs_INV_FILE);
				CS_fclose (strm);
				strm = NULL;
				strcpy (csErrnam,cs_Dir);
			}
			else
			{
				CS_bswap (&magic,"l");
				if (magic != cs_DTDEF_MAGIC)
				{
					CS_fclose (strm);
					strm = NULL;
					strcpy (csErrnam,cs_Dir);
					CS_erpt (cs_DT_BAD_MAGIC);
				}
				else if (!strcmp (mode,_STRM_BINRD))
				{
					cs_DtStream = strm;
				}
			}
		}
		else
		{
			strcpy (csErrnam,cs_Dir);
			CS_erpt (cs_DTDICT);
		}
	}
	return (strm);
}

/**********************************************************************
**	void CS_dtDictCls (csFILE *stream);
**
**	csFILE *stream;				stream to be closed
**
**	Stream is closed.  If it is the stream to which cs_DtStream
**	points, cs_DtStream is set to the NULL pointer.
**********************************************************************/

void CS_dtDictCls (csFILE* stream)
{
	if (stream == cs_DtStream)
	{
		cs_DtStream = NULL;
	}
	CS_fclose (stream);
}

/**********************************************************************
**    Calling Sequence:	flag = CS_dtrd (strm,dt_def,crypt);
**
**	csFILE *strm;				file/device from which the possibly
**								enrypted datum definition is to be read.
**	struct CS_Dtdef_ *dt_def;	decoded datum definition is returned here.
**	int *crypt;					returns TRUE if the entry was encrypted,
**								else returns FALSE.
**	int flag;					returns +1 for successful read, 0 for EOF,
**								-1 for error.
**********************************************************************/


int EXP_LVL3 CS_dtrd (csFILE *strm,struct cs_Dtdef_ *dt_def,int *crypt)
{
	cs_Register unsigned char key;
	cs_Register unsigned char *cp;

	int st;
	size_t rd_cnt;
	unsigned char *cpe;

	char tmpKeyName [cs_KEYNM_DEF];

	/* Synchronize the stream. */
	st = CS_fseek (strm,0L,SEEK_CUR);
	if (st != 0)
	{
		CS_erpt (cs_IOERR);
		return (-1);
	}

	/* Now we can read. */
	cp = (unsigned char *)dt_def;
	rd_cnt = CS_fread ((char *)dt_def,1,sizeof (*dt_def),strm);
	if (rd_cnt != sizeof (*dt_def))
	{
		if (CS_feof (strm))
		{
			return 0;
		}
		else if (CS_ferror (strm))
		{
			CS_erpt (cs_IOERR);
		}
		else
		{
			CS_erpt (cs_INV_FILE);
		}
		return (-1);
	}

	/* Do the encryption bit. */
	key = (unsigned char)dt_def->fill [0];
	if (key != '\0')
	{
		*crypt = TRUE;
		cpe = cp + sizeof (*dt_def);
		while (cp < cpe)
		{
			key ^= *cp;
			*cp++ = key;
		}
	}
	else
	{
		*crypt = FALSE;
	}

	/* Swap the bytes if necessary. */
	CS_bswap (dt_def,cs_BSWP_DTDEF);

	/* Check the result. The name must always meet the criteria
	   set by the CS_nmpp function.  At least so far, the criteria
	   established by CS_nampp over the years has always been
	   expanded, never restricted.  Thus, any definition which
	   was legitimate in a previous release would always be
	   legitimate iin subsequent releases. */
	CS_stncp (tmpKeyName,dt_def->key_nm,sizeof (tmpKeyName));
	if (CS_nampp (tmpKeyName) != 0)
	{
		/* Replace the error condition reported by CS_nampp with
		   and Invalid File indication. */
		CS_erpt (cs_INV_FILE);
		return (-1);
	}

	/* Reset the encryption indicator in the record. */
	dt_def->fill [0] = '\0';
	return (1);
}

/**********************************************************************
**	st = CS_dtwr (strm,dt_def,crypt);
**
**	csFile *strm;				file/device to which the possibly enrypted
**								datum definition is written.
**	struct cs_Dtdef_ *dt_def;	the unencrypted datum definition which is
**								to be written.
**	int crypt;					TRUE says that the definition is to be encoded
**								before writting, FALSE says no encoding.
**	int st;						returns FALSE if write was completed
**								successfully, else returns TRUE.
**
**********************************************************************/

int EXP_LVL3 CS_dtwr (csFILE *strm,Const struct cs_Dtdef_ *dt_def,int crypt)
{
	cs_Register unsigned char key;
	cs_Register unsigned char *cp;
	static unsigned seed = 0;

	int st;
	size_t wr_cnt;
	unsigned char *cpe;

 	__ALIGNMENT__1		/* For some versions of Sun compiler. */

	struct cs_Dtdef_ lcl_dt;

	/* Get a local copy which we can modify without screwing up
	   the calling module. */
	memcpy ((char *)&lcl_dt,(char *)dt_def,sizeof (lcl_dt));

	/* Swap the bytes, if necessary, before possible encryption. */
	CS_bswap (&lcl_dt,cs_BSWP_DTDEF);

	/* Encrypt if requested. */
	if (crypt)
	{
		if (seed == 0)
		{
			seed = (unsigned)CS_time ((cs_Time_ *)0);
			srand (seed);
		}
		for (;;)
		{
			key = (unsigned char)rand ();
			cpe = (unsigned char *)&lcl_dt;
			cp = cpe + sizeof (lcl_dt);
			lcl_dt.fill [0] = (char)key;
			lcl_dt.fill [1] = (char)rand ();
			while (--cp > cpe)
			{
				*cp ^= *(cp - 1);
			}
			*cp ^= (unsigned char)lcl_dt.fill [0];

			if (lcl_dt.fill [0] != '\0') break;

			/* OPPS!!! The key turned out to be
			   zero. Need to try another one.

			   Need to restore the original data
			   or we'll be encrypting encrypted
			   data.  We can't decipher that one. */

			memcpy ((char *)&lcl_dt,(char *)dt_def,
						      sizeof (lcl_dt));
			CS_bswap (&lcl_dt,cs_BSWP_DTDEF);
		}
	}
	else
	{
		/* If no encryption, set code to
		   zero.  This effectively turns
		   encryption off. */
		lcl_dt.fill [0] = '\0';
		lcl_dt.fill [1] = '\0';
	}

	/* Synchronize the stream. */
	st = CS_fseek (strm,0L,SEEK_CUR);
	if (st != 0)
	{
		CS_erpt (cs_IOERR);
		return (TRUE);
	}

	/* Now we can write. */
	wr_cnt = CS_fwrite ((char *)&lcl_dt,1,sizeof (lcl_dt),strm);
	if (wr_cnt != sizeof (lcl_dt))
	{
		if (CS_ferror (strm)) CS_erpt (cs_IOERR);
		else				  CS_erpt (cs_DISK_FULL);
		return (TRUE);
	}
	return (FALSE);
}

/**********************************************************************
**	st = CS_dtdel (dtdef);
**
**	struct cs_Dtdef_ *dtdef;	a pointer to the datum definition structure
**								which is to be deleted from the Datum Dictionary.
**	int st;						returns a zero if the delete was successfully
**								completed, else returns -1.
**
**	This function will modify the key_nm element of the
**	cs_Dtdef_ structure provided, forcing all lower case
**	characters to upper case.
**********************************************************************/

int EXP_LVL3 CS_dtdel (struct cs_Dtdef_ *dtdef)
{
    extern char *cs_DtKeyNames;
	extern char csErrnam [];
	extern char cs_Dir [];
	extern short cs_Protect;

	short cs_time;

	int st;
	csFILE *old_strm;
	csFILE *new_strm;
	int rd_st;
	int crypt;
	size_t wr_cnt;

	cs_magic_t magic;

	struct cs_Dtdef_ *my_ptr;

	char tmp_nam [MAXPATH];

 	__ALIGNMENT__1		/* For some versions of Sun compiler. */

	struct cs_Dtdef_ cpy_buf;

	/* Capture the current time. */
	cs_time = (short)((CS_time ((cs_Time_ *)0) - 630720000L) / 86400L);

	/* Prepare for an error. */
	new_strm = NULL;
	old_strm = NULL;
	my_ptr = NULL;
	tmp_nam [0] = '\0';

	/* Adjust the name and make sure it is all upper case.
	   By convention, datum names are case insensitive. */
	st = CS_nampp (dtdef->key_nm);
	if (st != 0) goto error;

	/* Get a pointer to the existing definition. If it doesn't
	   exist, we're all done. */
	my_ptr = CS_dtdef (dtdef->key_nm);
	if (my_ptr == NULL)
	{
		goto error;
	}

	/* See if this definition is protected.  If so, we have to
	   leave it alone. If cs_Protect < 0, there is no protection. */
	if (cs_Protect >= 0)
	{
		if (my_ptr->protect == 1)
		{
			CS_stncp (csErrnam,my_ptr->key_nm,MAXPATH);
			CS_erpt (cs_DT_PROT);
			goto error;
		}
		if (cs_Protect > 0)
		{
			/* Here if user definition protection is
			   enabled. */
			if (my_ptr->protect < (cs_time - cs_Protect))
			{
				CS_stncp (csErrnam,my_ptr->key_nm,MAXPATH);
				CS_erpt (cs_DT_UPROT);
				goto error;
			}
		}
	}
	CS_free (my_ptr);
	my_ptr = NULL;

    /* Looks like we will be deleting this datum definition.  cs_DtKeyNames
       is a memory array which contains all of the existing datum key names.
       Since we're going to change this, we free up the in memory list to
       force a regeneration of same next time its use is required.  This
       regeneration will not have this name in it if our addition completes
       successfully. */
    if (cs_DtKeyNames != NULL)
    {
        CS_free (cs_DtKeyNames);
        cs_DtKeyNames = NULL;
    }

	/* Make sure the entry that we have been provided is marked as
	   unencrypted so that the comparison function will work. */
	dtdef->fill [0] = '\0';

	/* Open up the datum dictionary file and verify its
	   magic number. */
	old_strm = CS_dtopn (_STRM_BINRD);
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
	   we must deal with the magic number. */
	magic = cs_DTDEF_MAGIC;
	CS_bswap (&magic,"l");
	wr_cnt = CS_fwrite ((char *)&magic,1,sizeof (magic),new_strm);
	if (wr_cnt != sizeof (magic))
	{
		if (CS_ferror (new_strm)) CS_erpt (cs_IOERR);
		else					  CS_erpt (cs_DISK_FULL);
		goto error;
	}

	/* Now we copy the file.  If the existing record was encrypted,
	   we encrypt the record which we write. */
	while ((rd_st = CS_dtrd (old_strm,&cpy_buf,&crypt)) > 0)
	{
		if (CS_dtcmp (&cpy_buf,dtdef) != 0)
		{
			if (CS_dtwr (new_strm,&cpy_buf,crypt))
			{
				goto error;
			}
		}
	}
	if (rd_st != 0)
	{
		/* The copy loop terminated due to an error. */
		goto error;
	}

	/* Close up, remove the old dictionary and rename the
	   new dictionary. */
	CS_fclose (new_strm);
	new_strm = NULL;
	CS_dtDictCls (old_strm);
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
		/* tmp_nam can never be uninitialized if new_fd >= 0 */
		CS_fclose (new_strm);
		CS_remove (tmp_nam);				/*lint !e534 !e645 */
	}
	if (old_strm != NULL) CS_dtDictCls (old_strm);
	if (my_ptr != NULL) CS_free (my_ptr);
	return (-1);
}

/**********************************************************************
**	flag = CS_dtupd (dtdef,crypt);
**
**	struct cs_Dtdef_ *dtdef;		a pointer to the datum structure
**									which is to be written to the Datum
**									Dictionary.
**	int crypt;						if TRUE, the datum dictionary entry
**									is encrypted
**									before it is written.
**	int flag;						returns +1 if the indicated datum
**									previously existed and was simply
**									updated, returns 0 if the datum had
**									to be added as a new datum, returns
**									-1 if the update process failed.
**
**	If the Datum Dictionary does not already contain an entry
**	with the indicated key name, the entry is added.
**
**	This function will modify the key_nm element of the
**	provided cs_Dtdef_ structure, changing all lower case
**	characters to upper case.
**********************************************************************/

int EXP_LVL3 CS_dtupd (struct cs_Dtdef_ *dtdef,int crypt)
{
    extern char *cs_DtKeyNames;
	extern char csErrnam [];
	extern short cs_Protect;
	extern char cs_Unique;

	short cs_time = 0;

	int st;
	int flag;
	int dummy;

	csFILE *strm;

	long32_t fpos;

	char *cp;

 	__ALIGNMENT__1		/* For some versions of Sun compiler. */

	struct cs_Dtdef_ my_dtdef;

	/* Capture the current time. For our purposes here, time is
	   the number of days since (approx) January 1, 1990. If this
	   record does get written, the protect field will indicate
	   that it has changed. */
	if (dtdef->protect >= 0)
	{
		if ((cs_Protect < 0) || (dtdef->protect != 1))
		{
			cs_time = (short)((CS_time ((cs_Time_ *)0) - 630720000L) / 86400L);
			dtdef->protect = cs_time;
		}
	}

	/* Prepare for a possible error. */
	strm = NULL;

	/* Adjust the name and make sure it is all upper
	   case.  By convention, datum names are case
	   insensitive. */
	st = CS_nampp (dtdef->key_nm);
	if (st != 0) goto error;

	/* Open up the Datum Dictionary and verify its magic number. */
	strm = CS_dtopn (_STRM_BINUP);
	if (strm == NULL)
	{
		goto error;
	}

	/* See if we have a datum with this name already. */
	flag = CS_bins (strm,(long32_t)sizeof (cs_magic_t),(long32_t)0,sizeof (*dtdef),
						(char *)dtdef,(CMPFUNC_CAST)CS_dtcmp);
	if (flag < 0) goto error;
	if (flag)
	{
		/* Here when the datum already exists. See if we are
		   allowed to change this definition. */
		if (cs_Protect >= 0)
		{
			/* Distribution protection is enabled. */
			fpos = CS_ftell (strm);
			if (fpos < 0L)
			{
				CS_erpt (cs_IOERR);
				goto error;
			}
			st = CS_dtrd (strm,&my_dtdef,&dummy);
			if (st == 0) CS_erpt (cs_INV_FILE);
			if (st <= 0)
			{
				goto error;
			}

			if (my_dtdef.protect == 1)
			{
				CS_stncp (csErrnam,dtdef->key_nm,MAXPATH);
				CS_erpt (cs_DT_PROT);
				goto error;
			}
			if (cs_Protect > 0 && my_dtdef.protect > 0)
			{
				if (my_dtdef.protect < (cs_time - cs_Protect))		/*lint !e644 */
				{
					CS_stncp (csErrnam,dtdef->key_nm,MAXPATH);
					CS_erpt (cs_DT_UPROT);
					goto error;
				}
			}
			st  = CS_fseek (strm,fpos,SEEK_SET);
			if (st < 0L)
			{
				CS_erpt (cs_IOERR);
				goto error;
			}
		}

		/* If we're still here, it's OK to update this definition. */
		if (CS_dtwr (strm,dtdef,crypt))
		{
			goto error;
		}
	}
	else
	{
        /* We're going to attempt writing this definition to the dictionary.
           cs_DtKeyNames is a memory array which contains all of the existing
           datum key names.  Since we're going to change this, we free up the
           in memory list to force a regeneration of same next time its use is
           required.  This regeneration will have the new name in it if our
           addition completes successfully. */
        if (cs_DtKeyNames != NULL)
        {
            CS_free (cs_DtKeyNames);
            cs_DtKeyNames = NULL;
        }

		/* Here if the datum definition doesn't exist.  We
		   have to add it. If cs_Unique is not zero, we
		   require that a cs_Unique character be present
		   in the key name before we'll allow it to be
		   written. */
		if (cs_Unique != '\0')
		{
			cp = strchr (dtdef->key_nm,cs_Unique);
			if (cp == NULL)
			{
				csErrnam [0] = cs_Unique;
				csErrnam [1] = '\0';
				CS_erpt (cs_UNIQUE);
				goto error;
			}
		}

		/* Now we can add it. Write to the end of the file, and then
		   sort the file. */
		st = CS_fseek (strm,0L,SEEK_END);
		if (st != 0)
		{
			CS_erpt (cs_IOERR);
			goto error;
		}
		if (CS_dtwr (strm,dtdef,crypt))
		{
			goto error;
		}

		/* Sort the file into proper order, thereby
		   moving the new datum to its proper place
		   in the dictionary. */
		st = CS_fseek (strm,(long)sizeof (cs_magic_t),SEEK_SET);
		if (st != 0)
		{
			CS_erpt (cs_IOERR);
			goto error;
		}
		st = CS_ips (strm,sizeof (*dtdef),0L,(CMPFUNC_CAST)CS_dtcmp);
		if (st < 0) goto error;
	}

	/* The Datum Dictionary has been updated. */
	CS_dtDictCls (strm);
	return (flag);

error:
	if (strm != NULL) CS_dtDictCls (strm);
	return (-1);
}

/**********************************************************************
**	cmp_val = CS_dtcmp (pp,qq);
**
**	struct cs_Dtdef_ *pp;
**	struct cs_Dtdef_ *qq;		The two datum definition structures which arew
**								to be compared.
**	int cmp_val;				return is negative if pp follows qq in the
**								collating sequence, positive if pp preceeds
**								qq, and zero if they are equivalent.
**********************************************************************/

int EXP_LVL7 CS_dtcmp (Const struct cs_Dtdef_ *pp,Const struct cs_Dtdef_ *qq)
{
	cs_Register unsigned char key;
	cs_Register unsigned char *cp;

	int st;

	unsigned char *cpe;

	char pp_key [24];
	char qq_key [24];

	/* If the entries are encoded, we must decode before the
	   comparision is made.  Note, this whole encryption
	   scheme is designed to decode from the front, therefore
	   we need only deal with the first 24 characters. */

	(void)memcpy (pp_key,pp->key_nm,sizeof (pp_key));
	key = (unsigned char)pp->fill [0];
	if (key != '\0')
	{
		cp = (unsigned char *)pp_key;
		cpe = cp + sizeof (pp_key);
		while (cp < cpe)
		{
			key ^= *cp;
			*cp++ = key;
		}
	}
	(void)memcpy (qq_key,qq->key_nm,sizeof (qq_key));
	key = (unsigned char)qq->fill [0];
	if (key != '\0')
	{
		cp = (unsigned char *)qq_key;
		cpe = cp + sizeof (qq_key);
		while (cp < cpe)
		{
			key ^= *cp;
			*cp++ = key;
		}
	}

	/* OK, now we can compare the two structures.  For sorting
	   and binary search purposes, we only look at the key name. */

	st = CS_stricmp (pp_key,qq_key);

	return (st);
}

/**********************************************************************
**	dt_ptr = CS_dtdef (dat_nam);
**
**	char dat_nam;				the key name of the datum to be located.
**								I.e. "NAD27 for the North American Datum
**								of 1927.
**	struct cs_Dtdef_ *dt_ptr;	returns a pointer to a malloc'ed structure.
**********************************************************************/

struct cs_Dtdef_ * EXP_LVL3 CS_dtdef (Const char *dat_nam)
{
	extern char csErrnam [];

	extern double cs_DelMax;		/* 5,000.0 */
	extern double cs_RotMax;		/* 15.0    */
	extern double cs_SclMax;		/* 200.0   */

	int st;
	int flag;
	int crypt;

	csFILE *strm;
	struct cs_Dtdef_ *dtptr;

 	__ALIGNMENT__1				/* For some versions of Sun compiler. */

	struct cs_Dtdef_ dtdef;

	/* Prepare for an error condition. */
	dtptr = NULL;
	strm = NULL;

	/* Give the application first shot at satisfying this request. */
	if (CS_usrDtDefPtr != NULL)
	{
		st = (*CS_usrDtDefPtr)(&dtdef,dat_nam);
		if (st < 0) return NULL;
		if (st == 0)
		{
			dtptr = (struct cs_Dtdef_ *)CS_malc (sizeof (struct cs_Dtdef_));
			if (dtptr == NULL)
			{
				CS_erpt (cs_NO_MEM);
				goto error;
			}
			memmove (dtptr,&dtdef,sizeof (*dtptr));
			return dtptr;
		}
	}

	/* Verify the name is OK. */
	CS_stncp (dtdef.key_nm,dat_nam,sizeof (dtdef.key_nm));
	st = CS_nampp (dtdef.key_nm);
	if (st != 0) goto error;

	/* Mark this name as unencrypted so that the comaprison function
	   will work. */
	dtdef.fill [0] = '\0';

	/* Open the Datum Dictionary and test its magic number. */
	strm = CS_dtopn (_STRM_BINRD);
	if (strm == NULL) goto error;

	/* Search for the requested datum. */
	flag = CS_bins (strm,(long32_t)sizeof (cs_magic_t),(long32_t)0,sizeof (dtdef),&dtdef,(CMPFUNC_CAST)CS_dtcmp);
	if (flag < 0) goto error;

	/* Tell the user if we didn't find the requested datum. */
	if (!flag)
	{
		CS_stncp (csErrnam,dat_nam,MAXPATH);
		CS_erpt (cs_DT_NOT_FND);
		goto error;
	}
	else
	{
		/* The datum exists, malloc some memory for it. */
		dtptr = (struct cs_Dtdef_ *)CS_malc (sizeof (*dtptr));
		if (dtptr == NULL)
		{
			CS_erpt (cs_NO_MEM);
			goto error;
		}

		/* Read it in. */
		if (!CS_dtrd (strm,dtptr,&crypt))
		{
			goto error;
		}
	}

	/* We don't need the datum dictionary anymore. */
	if (!cs_DtStrmFlg)
	{
		CS_dtDictCls (strm);
	}
	strm = NULL;

	/* Verify that the values are not completely bogus. */
	if (fabs (dtptr->delta_X) > cs_DelMax ||
		fabs (dtptr->delta_Y) > cs_DelMax ||
		fabs (dtptr->delta_Z) > cs_DelMax ||
		fabs (dtptr->rot_X) > cs_RotMax    ||
		fabs (dtptr->rot_Y) > cs_RotMax    ||
		fabs (dtptr->rot_Z) > cs_RotMax    ||
		fabs (dtptr->bwscale) > cs_SclMax)
	{
		CS_stncp (csErrnam,dat_nam,MAXPATH);
		CS_erpt (cs_DTDEF_INV);
		goto error;
	}

	/* Return the initialized datum structure to the user. */
	return (dtptr);

error:
	if (strm != NULL) CS_dtDictCls (strm);
	if (dtptr != NULL)
	{
		CS_free (dtptr);
		dtptr = NULL;
	}
	return (dtptr);
}

/**********************************************************************
**	CS_dtfnm (new_name);
**
**	char *new_name;				the name of the datum dictionary file.
**
**	This function specifies the name only.  The directory,
**	and possibly drive, are specified using CS_altdr.
**********************************************************************/

void EXP_LVL1 CS_dtfnm (Const char *new_name)
{
	extern char cs_Dtname [];

	(void)CS_stncp (cs_Dtname,new_name,cs_FNM_MAXLEN);
	return;
}
void EXP_LVL1 CS_usrDtfnm (Const char *new_name)
{
	extern char cs_UsrDtName [];

	(void)CS_stncp (cs_UsrDtName,new_name,cs_FNM_MAXLEN);
	return;
}
