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

extern csFILE* cs_CsStream;
extern short cs_CsStrmFlg;

/**********************************************************************
 Hook function to support the use of temporary coordinate systems.

 To activate, set the global CS_usrCsDefPtr variable to point to your
 function.  To deactivate, set CS_usrCsDefPtr to NULL.
 
 CS_csdef calls the (*CS_usrCsDefPtr) function before it does anything
 with the name.  Thus, temporary names need not adhere to any CS-MAP
 convention regarding key names.

 (*CS_usrCsDefPtr) should return:
 -1 to indicate an error of sorts, in which case the error condition
    must have already been reported to CS_erpt.
 +1 to indicate that normal CS_csdef processing is to be performed.
  0 to indicate that the cs_Csdef_ structure provided by the first
    argument has been properly filled in with the desired definition
	which is to be returned by CS_csdef.

 In the case where (*CS_usrCsDefPtr) returns 0, CS_csdef will return
 a copy of the data provided in a chunk of memory it malloc's from
 the heap.  Also note, that the data returned is not checked.  You
 are responsible for making sure this data is valid.
**********************************************************************/
extern int (*CS_usrCsDefPtr) (struct cs_Csdef_ *csDef,Const char *keyName);

/**********************************************************************
**	strm = CS_csopn (mode);
**
**	char *mode;					mode of the open, cs_map.h ala _STRM_???
**	csFILE *strm;				returns a stream pointer to the open file.
**
**	Will return NULL if a problem was encountered.
**
**	File is positioned past the magic number on the front.
**********************************************************************/

csFILE* EXP_LVL3 CS_csopn (Const char *mode)
{
	extern char cs_Dir [];
	extern char *cs_DirP;
	extern char cs_Csname [];
	extern char csErrnam [];

	size_t rd_cnt;

	csFILE *strm = NULL;

	cs_magic_t magic;

	if (cs_CsStream != 0)
	{
		if (!CS_stricmp (mode,_STRM_BINRD))
		{
			strm = cs_CsStream;
			CS_fseek (strm,(long)sizeof (magic),SEEK_SET);
		}
		else
		{
			CS_fclose (cs_CsStream);
			cs_CsStream = NULL;
		}
	}
	if (strm == NULL)
	{
		/* We need to do a real open. */
		strcpy (cs_DirP,cs_Csname);
		strm = CS_fopen (cs_Dir,mode);
		if (strm != NULL)
		{
			magic = 0L;
			rd_cnt = CS_fread ((char *)&magic,1,sizeof (magic),strm);
			if (rd_cnt == sizeof (magic))
			{
				CS_bswap (&magic,"l");
				if (magic != cs_CSDEF_MAGIC)
				{
					CS_fclose (strm);
					strm = NULL;
					strcpy (csErrnam,cs_Dir);
					CS_erpt (cs_CS_BAD_MAGIC);
				}
				else if (!strcmp (mode,_STRM_BINRD))
				{
					cs_CsStream = strm;
				}
			}
			else
			{
				if (CS_ferror (strm)) CS_erpt (cs_IOERR);
				else				  CS_erpt (cs_INV_FILE);
				CS_fclose (strm);
				strm = NULL;
			}
		}
		else
		{
			strcpy (csErrnam,cs_Dir);
			CS_erpt (cs_CSDICT);
		}
	}
	return (strm);
}

/**********************************************************************
**	void CS_csDictCls (csFILE* stream);
**
**	csFILE *strm;			stream to be closed.
**
** The stream is closed.  If the stream is that to which cs_CsStream
** points, cs_CsStream is set to the NULL pointer.
**********************************************************************/

void CS_csDictCls (csFILE* stream)
{
	if (stream == cs_CsStream)
	{
		cs_CsStream = NULL;
	}
	CS_fclose (stream);
}

/**********************************************************************
**	flag = CS_csrd (strm,cs_def,crypt);
**
**	csFILE *strm;				file/device from which the possibly
**								enrypted coordinate system definition
**								is to be read.
**	struct cs_Csdef_ *cs_def;	decoded coordinate system definition is
**								returned here.
**	int *crypt;					returns TRUE here if the entry was
**								encrypted; else returns FALSE.
**	int flag;					returns +1 for successful read, 0 for
**								EOF, -1 for error.
**
**********************************************************************/
int EXP_LVL3 CS_csrd (csFILE *strm,struct cs_Csdef_ *cs_def,int *crypt)

{
	cs_Register unsigned char key;
	cs_Register unsigned char *cp;

	int st;
	size_t rd_cnt;
	unsigned char *cpe;
	char tmpKeyName [cs_KEYNM_DEF];

	/* Force stream synchronization. */
	st = CS_fseek (strm,0L,SEEK_CUR);
	if (st != 0)
	{
		CS_erpt (cs_IOERR);
		return (-1);
	}
	
	/* Do the read. */
	cp = (unsigned char *)cs_def;
	rd_cnt = CS_fread ((char *)cs_def,1,sizeof (*cs_def),strm);
	if (rd_cnt != sizeof (struct cs_Csdef_))
	{
		if (CS_feof (strm))
		{
			return 0;
		}
		else
		{
			if (CS_ferror (strm)) CS_erpt (cs_IOERR);
			else				  CS_erpt (cs_INV_FILE);
			return (-1);
		}
	}
	key = (unsigned char)cs_def->fill [0];
	if (key != '\0')
	{
		*crypt = TRUE;
		cpe = cp + sizeof (*cs_def);
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
	CS_bswap (cs_def,cs_BSWP_CSDEF);

	/* Check the result. The name must always meet the criteria
	   set by the CS_nmpp function. */
	CS_stncp (tmpKeyName,cs_def->key_nm,sizeof (tmpKeyName));
	if (CS_nampp (tmpKeyName) != 0)
	{
		/* Replace the error condition reported by CS_nampp with
		   and Invalid File indication. */
		CS_erpt (cs_INV_FILE);
		return (-1);
	}

	/* Reset the encryption indicator in the returned record. */
	cs_def->fill [0] = '\0';
	return (1);
}

/**********************************************************************
**	st = CS_cswr (strm,cs_def,crypt);
**
**	csFILE *strm;				file/device to which the possibly
**								enrypted coordinate system definition
**								is written.
**	struct cs_Csdef_ *cs_def;	the unencrypted coordinate system
**								definition which is to be written.
**	int crypt;					TRUE says that the definition is to
**								be encoded before writting, FALSE says
**								no encoding.
**	int st;						returns FALSE if the write was
**								successfully completed, else returns
**								TRUE.
**
**********************************************************************/
int EXP_LVL3 CS_cswr (csFILE *strm,Const struct cs_Csdef_ *cs_def,int crypt)
{

	cs_Register unsigned char key;
	cs_Register unsigned char *cp;

	static unsigned seed = 0;

	int st;
	size_t wr_cnt;
	cs_Time_ gmtTime;

	unsigned char *cpe;

	__ALIGNMENT__1			/* For some versions of Sun compiler. */

	struct cs_Csdef_ lcl_cs;

	/* Get the current time. */
	gmtTime = CS_time ((cs_Time_ *)0);

	/* Get a local copy which we can modify without screwing up
	   the calling module. */
	memcpy ((char *)&lcl_cs,(char *)cs_def,sizeof (lcl_cs));

	/* Swap the bytes of appropriate. */
	CS_bswap (&lcl_cs,cs_BSWP_CSDEF);

	/* Encrypt if requested. */
	if (crypt)
	{
		if (seed == 0)
		{
			seed = (unsigned)gmtTime;
			srand (seed);
		}
		for (;;)
		{
			key = (unsigned char)rand ();
			cpe = (unsigned char *)&lcl_cs;
			cp = cpe + sizeof (lcl_cs);
			lcl_cs.fill [0] = (char)key;
			lcl_cs.fill [1] = (char)rand ();
			while (--cp > cpe)
			{
				*cp ^= *(cp - 1);
			}
			*cp ^= (unsigned char)lcl_cs.fill [0];
			if (lcl_cs.fill [0] != '\0') break;

			/* If the key ends up to be zero,
			   we need to try another one. */
			memcpy ((char *)&lcl_cs,(char *)cs_def,sizeof (lcl_cs));
			CS_bswap (&lcl_cs,cs_BSWP_CSDEF);
		}
	}
	else
	{
		/* If no encryption, set code to
		   zero.  This effectively turns
		   encryption off. */
		lcl_cs.fill [0] = '\0';
		lcl_cs.fill [1] = '\0';
	}

	/* Force stream synchronization. */
	st = CS_fseek (strm,0L,SEEK_CUR);
	if (st != 0)
	{
		CS_erpt (cs_IOERR);
		return (TRUE);
	}

	/* Do the actual write. */
	wr_cnt = CS_fwrite ((char *)&lcl_cs,1,sizeof (lcl_cs),strm);
	if (wr_cnt != sizeof (lcl_cs))
	{
		if (CS_ferror (strm)) CS_erpt (cs_IOERR);
		else				  CS_erpt (cs_DISK_FULL);
		return (TRUE);
	}
	return (FALSE);
}

/**********************************************************************
**	st = CS_csdel (csdef);
**
**	struct cs_Csdef_ *csdef;	a pointer to the coordinate system
**								definition structure which is to be
**								deleted from the Coordinate System
**								Dictionary.
**	int st;						returns zero if the delete was completed
**								successfully, else returns -1.
**
**	This function will modify the contents of the key_nm,
**	forcing same to adhere to the Coordinate System naming
**	convention of all upper case letters.
**********************************************************************/

int EXP_LVL3 CS_csdel (struct cs_Csdef_ *csdef)
{
	extern char csErrnam [];
	extern char cs_Dir [];
	extern short cs_Protect;

	short cs_time;

	int st;
	csFILE *old_strm;
	csFILE *new_strm;
	int rd_st;
	size_t wr_cnt;
	int crypt;

	cs_Time_ gmtTime;
	struct cs_Csdef_ *my_ptr;


	cs_magic_t magic;

	char tmp_nam [MAXPATH];

 	__ALIGNMENT__1					/* For some versions of Sun compiler. */

	struct cs_Csdef_ cpy_buf;

	my_ptr = NULL;

	/* Get the current time. */
	gmtTime = CS_time ((cs_Time_ *)0);

	/* Capture the current time. */
	cs_time = (short)((gmtTime - 630720000L) / 86400L);

	/* Prepare for a possible error. */
	new_strm = NULL;
	old_strm = NULL;

	/* Adjust the name and make sure it is all upper case.
	   By convention, coordinate system names are case
	   insensitive. */
	st = CS_nampp (csdef->key_nm);
	if (st != 0) goto error;

	/* Get a pointer to the existing definition. If it doesn't
	   exist, we're all done. */
	my_ptr = CS_csdef (csdef->key_nm);
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
			CS_erpt (cs_CS_PROT);
			goto error;
		}
		if (cs_Protect > 0)
		{
			/* Here if user definition protection is
			   enabled. */
			if (my_ptr->protect < (cs_time - cs_Protect))
			{
				CS_stncp (csErrnam,my_ptr->key_nm,MAXPATH);
				CS_erpt (cs_CS_UPROT);
				goto error;
			}
		}
	}
	CS_free (my_ptr);
	my_ptr = NULL;

	/* If we're still here, it's OK to delete this definition.

	   Make sure the entry provided is marked as being
	   NOT encrypted so that the comparison function will
	   work properly. */
	csdef->fill [0] = '\0';

	/* Open up the coordinate system dictionary file and
	   verify its magic number. */
	old_strm = CS_csopn (_STRM_BINRD);
	if (old_strm == NULL)
	{
		goto error;
	}

	/* Create a temporary file for the new dictionary. */
	st = CS_tmpfn (tmp_nam);
	if (st < 0) goto error;
	new_strm = CS_fopen (tmp_nam,_STRM_BINWR);
	if (new_strm == NULL)
	{
		CS_erpt (cs_TMP_CRT);
		goto error;
	}

	/* Copy the file, skipping the entry to be deleted. */
	magic = cs_CSDEF_MAGIC;
	CS_bswap (&magic,"l");
	wr_cnt = CS_fwrite ((char *)&magic,1,sizeof (magic),new_strm);
	if (wr_cnt != sizeof (magic))
	{
		if (CS_ferror (new_strm)) CS_erpt (cs_IOERR);
		else					  CS_erpt (cs_DISK_FULL);
		goto error;
	}

	/* Copy the dictionary to the new file, skipping the
	   entry we are supposed to delete.  If the record we
	   read was encrypted, we encrypt it when we write it. */
	while ((rd_st = CS_csrd (old_strm,&cpy_buf,&crypt)) > 0)
	{
		if (CS_cscmp (&cpy_buf,csdef) != 0)
		{
			/* Here if the coordinate system being copied
			   is not the one to be deleted. */
			if (CS_cswr (new_strm,&cpy_buf,crypt))
			{
				goto error;
			}
		}
	}

	/* Go to the error cleanup code if the copy loop terminated
	   due to an error. */
	if (rd_st < 0) goto error;

	/* Close up, remove the old dictionary and rename the
	   new dictionary. */
	CS_csDictCls (old_strm);
	old_strm = NULL;
	CS_fclose (new_strm);
	new_strm = NULL;
	st = CS_remove (cs_Dir);
	if (st != 0)
	{
		CS_remove (tmp_nam);				/*lint !e534 */ /* We're already processing an error. */
		strcpy (csErrnam,cs_Dir);
		CS_erpt (cs_UNLINK);
		goto error;
	}
	st = CS_rename (tmp_nam,cs_Dir);
	if (st != 0) goto error;

	return (0);

error:
	if (new_strm != NULL)
	{
		CS_fclose (new_strm);
		CS_remove (tmp_nam);				/*lint !e534 !e645 */ /* We're already processing an error; and can't open new_strm unless we have a name. */
	}
	if (old_strm != NULL) CS_csDictCls (old_strm);
	if (my_ptr != NULL) CS_free (my_ptr);
	return (-1);
}

/**********************************************************************
**	flag = CS_csupd (csdef,crypt);
**
**	struct cs_Csdef_ *csdef;	a pointer to the coordinate system
**								definition structure which is to be
**								written to the Coordinate System
**								Dictionary.
**	int crypt;					if TRUE, the coordinate system is
**								encrypted before being written to the
**								dictionary.
**	int flag;					returns +1 if a coordinate system of
**								the given name already existed and was
**								updated.  Returns 0 if the coordinate
**								system had to be added to the
**								dictionary.  Returns -1 if an error
**								occurred during the update.
**
**	If the Coordinate System Dictionary does not already contain
**	an entry with the indicated key name, the entry is added.
**
**	This function will modify the key_nm element of the cs_Csdef_
**	structure provided, forcing all lower case letters to upper
**	case.
**********************************************************************/

/*
	The following defines the CS_cschk options we use.
*/
#define cs_CSCHK_OPTS (cs_CSCHK_DATUM | cs_CSCHK_ELLIPS | cs_CSCHK_REPORT)

int EXP_LVL3 CS_csupd (struct cs_Csdef_ *csdef,int crypt)
{
	extern struct cs_Prjtab_ cs_Prjtab [];	/* Projection Table */
	extern char csErrnam [];
	extern double cs_Two_pi;				/* 6.28..... */
	extern double cs_One;					/* 1.0 */
	extern double cs_Zero;					/* 0.0 */

	extern short cs_Protect;
	extern char cs_Unique;

	short cs_time = 0;

	int st;
	int flag;
	int err_cnt;
	int dummy;

	char *cp;
	csFILE *strm;
	struct cs_Datum_ *dt_ptr;
	struct cs_Eldef_ *el_ptr;
	struct cs_Prjtab_ *pp;

	long32_t fpos;
	cs_Time_ gmtTime;

	double e_rad;

	int err_list [4];

	__ALIGNMENT__1		/* For some versions of Sun compiler. */

	struct cs_Csdef_ my_csdef;

	/* Get the current time. */
	gmtTime = CS_time ((cs_Time_ *)0);

	/* Prepare for an error condition. */
	strm = NULL;
	dt_ptr = NULL;
	el_ptr = NULL;

	/* Compute the time, as we use it internally.  This is
	   days since January 1, 1990. If this record does get
	   written, we want it to have the current date in it.
	   This means that even if allowed, a distribution
	   coordinate system will be marked as having been
	   twiddled. */
	if (csdef->protect >= 0)
	{
		if ((cs_Protect < 0) || (csdef->protect != 1))
		{
			cs_time = (short)((gmtTime - 630720000L) / 86400L);
			csdef->protect = cs_time;
		}
	}

	/* Adjust the name and make sure it is all upper case.
	   By convention, coordinate system names are case
	   insensitive. */
	st = CS_nampp (csdef->key_nm);
	if (st != 0) goto error;

	/* Make sure this new entry is marked as unencrypted so that
	   the comparison function will work. */
	csdef->fill [0] = '\0';

	/* Verify that the definition is valid. */
	err_cnt = CS_cschk (csdef,cs_CSCHK_OPTS,err_list,sizeof (err_list)/sizeof (int));
	if (err_cnt != 0) goto error;

	/* CS_cschk will have verified the unit, but until we clean some
	   things up in release 8, we'll need to do some stuff with the
	   unit ourselves, and we need a pointer to the projection table
	   entry in order to do that. */
	for (pp = cs_Prjtab;pp->setup != NULL;pp += 1)
	{
		if (!CS_stricmp (csdef->prj_knm,pp->key_nm))
		{
			break;
		}
	}
	if (pp->setup == NULL)
	{
		/* This should never happen any more. */

		CS_stncp (csErrnam,csdef->prj_knm,MAXPATH);
		CS_erpt (cs_UNKWN_PROJ);
		goto error;
	}

	/* Make sure that the scale factors in the definition are
	   properly updated to reflect the unit of the coordinate
	   system. */
	if (pp->code != cs_PRJCOD_UNITY)
	{
		/* Here for normal cartesian coordinate systems.
		   Get the linear unit specification and make sure
		   that it is valid.  Unit scale is the factor
		   which is used to convert scalars, such as text
		   height, from system coordinates to meters by
		   multiplication; or from meters to the system
		   unit by division. */
		csdef->unit_scl = CS_unitlu (cs_UTYP_LEN,csdef->unit);

		/* This should never happen any more. */
		if (csdef->unit_scl == 0.0) goto error;

		/* Now recompute the scale of the definition.  This
		   scale factor converts system units to meters by
		   division; or meters to system units by multiplication.
		   This factor also includes the mapping scale factor.
		   
		   NOTE ALSO, that this no longer includes the scale
		   reduction factor.  Each individual projection which
		   requires a scale reduction factor is responsible for
		   handling it separately.  (This didn't used to be
		   the case.) */
		csdef->scale = cs_One / (csdef->unit_scl * csdef->map_scl);
	}
	else
	{
		/* Here for lat/long coordinate systems.  Again the
		   unit scale is used to convert scalars, such as text
		   height, from system units to meters by multiplication;
		   or from meters to system units by division.  This
		   depends upon the datum definition.  That is, we
		   use the length of a unit of longitude at the equator
		   as the conversion factor from degrees to meters
		   and vice versa.
		   
		   Get the angular system unit and make sure it is
		   valid. */
		csdef->unit_scl = CS_unitlu (cs_UTYP_ANG,csdef->unit);

		/* This should never happen any more. */
		if (csdef->unit_scl == 0.0) goto error;

		/* The basic system sacle factor is simply that of the
		   unit.  I.e., the amount we modify degrees to produce
		   the desired unit.  We do this calculation now. */
		csdef->scale = cs_One / csdef->unit_scl;

		/* Adjust the unit_scl by the radius of the ellipsoid
		   which is referenced.  Of course, we must access the
		   datum definition to get the equatorial radius. Since
		   we've already checked this, it should work without
		   problem. */
		e_rad = cs_Zero;
		if (csdef->dat_knm [0] != '\0')
		{
			dt_ptr = CS_dtloc (csdef->dat_knm);
			if (dt_ptr != NULL)
			{
				e_rad = dt_ptr->e_rad;
				CS_free (dt_ptr);
				dt_ptr = NULL;
			}
		}
		if (e_rad == 0.0 && csdef->elp_knm [0] != '\0')
		{
			el_ptr = CS_eldef (csdef->elp_knm);
			if (el_ptr != NULL)
			{
				e_rad = el_ptr->e_rad;
				CS_free (el_ptr);
				el_ptr = NULL;
			}
		}
		if (e_rad == 0.0)
		{
			e_rad = 6378206.4;
		}
		csdef->unit_scl *= cs_Two_pi * e_rad / 360.0;
	}

	/* Open up the coordinate system dictionary file and
	   verify its magic number. */
	strm = CS_csopn (_STRM_BINUP);
	if (strm == NULL)
	{
		goto error;
	}

	/* See if we have a coordinate system with this name already. */
	flag = CS_bins (strm,(long32_t)sizeof (cs_magic_t),(long32_t)0,sizeof (*csdef),(char *)csdef,(CMPFUNC_CAST)CS_cscmp);
	if (flag < 0) goto error;
	if (flag)
	{
		/* Here when the coordinate system already exists. See
		   if it is OK to write it. If cs_Protect is less than
		   zero, all protection has been turned off. */
		if (cs_Protect >= 0)
		{
			fpos = (long32_t)CS_ftell (strm);
			if (fpos < 0L)
			{
				CS_erpt (cs_IOERR);
				goto error;
			}
			st = CS_csrd (strm,&my_csdef,&dummy);
			if (st == 0)
			{
				CS_erpt (cs_INV_FILE);
			}
			if (st <= 0)
			{
				goto error;
			}
			if (my_csdef.protect == 1)
			{
				/* We don't allow distribution coordinate
				   systems to be overwritten, period. */
				CS_stncp (csErrnam,csdef->key_nm,MAXPATH);
				CS_erpt (cs_CS_PROT);
				goto error;
			}
			if (cs_Protect > 0 && my_csdef.protect > 0)
			{
				/* We protect user defined systems only
				   if cs_Protect is greater than zero. */
				if (my_csdef.protect < (cs_time - cs_Protect))		/*lint !e644 */
				{
					/* It has been more than cs_Protect
					   days since this coordinate system
					   has been twiddled, we consider it
					   to be protected. */
					CS_stncp (csErrnam,csdef->key_nm,MAXPATH);
					CS_erpt (cs_CS_UPROT);
					goto error;
				}
			}
			st = CS_fseek (strm,(long)fpos,SEEK_SET);
			if (st != 0)
			{
				CS_erpt (cs_IOERR);
				goto error;
			}
		}

		/* OK, if we're still here, it's OK to overwrite the
		   record. */
		if (CS_cswr (strm,csdef,crypt))
		{
			goto error;
		}
	}
	else
	{
		/* Here if the coordinate system doesn't exist.  We
		   have to add it. If cs_Unique is not zero, we
		   require that a cs_Unique character be present
		   in the key name before we'll allow it to be
		   written. */
		if (cs_Unique != '\0')
		{
			cp = strchr (csdef->key_nm,cs_Unique);
			if (cp == NULL)
			{
				csErrnam [0] = cs_Unique;
				csErrnam [1] = '\0';
				CS_erpt (cs_UNIQUE);
				goto error;
			}
		}

		/* First we write the new coordinate system to the
		   end of the file. */
		if (CS_fseek (strm,0L,SEEK_END) != 0)
		{
			CS_erpt (cs_IOERR);
			goto error;
		}
		if (CS_cswr (strm,csdef,crypt))
		{
			goto error;
		}

		/* Sort the file into proper order, thereby
		   moving the new coordinate system to its
		   proper place in the file. */
		st = CS_fseek (strm,(long)sizeof (cs_magic_t),0);
		if (st != 0)
		{
			CS_erpt (cs_IOERR);
			goto error;
		}
		st = CS_ips (strm,sizeof (*csdef),0L,(CMPFUNC_CAST)CS_cscmp);
		if (st < 0) goto error;
	}

	/* The Coordinate System Dictionary has been updated. */
	CS_csDictCls (strm);
	return (flag);

error:
	if (dt_ptr != NULL) CS_free (dt_ptr);	/*lint !e774 */  /* redundant, but defensively desireable */
	if (el_ptr != NULL) CS_free (el_ptr);	/*lint !e774 */  /* redundant, but defensively desireable */
	if (strm != NULL) CS_csDictCls (strm);
	return (-1);
}
/**********************************************************************
**	cmp_val = CS_cscmp (pp,qq);
**
**	struct cs_Csdef_ *pp;
**	struct cs_Csdef_ *qq;		The two coordinate systems which
**								are to be compared.
**	int cmp_val;				return is negative if pp follows qq in the
**								collating sequence, positive if pp preceeds
**								qq, and zero if they are equivalent.
**********************************************************************/


int EXP_LVL7 CS_cscmp (Const struct cs_Csdef_ *pp,Const struct cs_Csdef_ *qq)
{
	cs_Register unsigned char key;
	cs_Register unsigned char *cp;

	int st;

	unsigned char *cpe;

	char pp_key [24];
	char qq_key [24];

	/* If the entries are encoded, we must decode before the
	   comparision is made. */

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
	st = CS_stricmp (pp_key,qq_key);
	return (st);
}

/**********************************************************************
**	cs_ptr = CS_csdef (cs_nam);
**
**	char *cs_nam;				key name of the coordinate system to be
**								set up.
**	struct cs_Csdef_ *cs_ptr;	returns a pointer to a malloc'ed and initialized
**								coordinate system definition structure; else
**								a NULL value for failure.
**********************************************************************/
struct cs_Csdef_ * EXP_LVL3 CS_csdef (Const char *cs_nam)
{
	extern char csErrnam [];

	int st;
	int flag;
	int rd_st;
	int crypt;

	csFILE *strm;
	struct cs_Csdef_ *cs_ptr;

	__ALIGNMENT__1			/* For some versions of Sun compiler. */

	struct cs_Csdef_ csdef;

	/* Prepare for an error condition. */
	strm = NULL;
	cs_ptr = NULL;

	/* Give the application first shot at satisfying this request. */
	if (CS_usrCsDefPtr != NULL)
	{
		st = (*CS_usrCsDefPtr)(&csdef,cs_nam);
		if (st < 0) return NULL;
		if (st == 0)
		{
			cs_ptr = (struct cs_Csdef_ *)CS_malc (sizeof (struct cs_Csdef_));
			if (cs_ptr == NULL)
			{
				CS_erpt (cs_NO_MEM);
				goto error;
			}
			memmove (cs_ptr,&csdef,sizeof (*cs_ptr));
			return cs_ptr;
		}
	}

	/* Make sure the name is not too large. */
	CS_stncp (csdef.key_nm,cs_nam,sizeof (csdef.key_nm));
	st = CS_nampp (csdef.key_nm);
	if (st != 0) goto error;

	/* Mark the entry, used for comparison purposes only,
	   as not encrypted. */
	csdef.fill [0] = '\0';

	/* Search the file for the requested coordinate system
	   definition. */
	strm = CS_csopn (_STRM_BINRD);
	if (strm == NULL) goto error;

	/* Locate the coordinate system which we are to return. */
	flag = CS_bins (strm,(long32_t)sizeof (cs_magic_t),(long32_t)0,sizeof (csdef),
									(char *)&csdef,(CMPFUNC_CAST)CS_cscmp);
	if (flag < 0) goto error;

	/* See if the coordinate system exists.  If not, there's not
	   much more we can do. */
	if (!flag)
	{
		CS_stncp (csErrnam,cs_nam,MAXPATH);
		CS_erpt (cs_CS_NOT_FND);
		goto error;
	}
	else
	{
		/* The coordinate system definition exists.  Therefore,
		   we malloc the coordinate system definition structure
		   and read it in. */
		cs_ptr = (struct cs_Csdef_ *)CS_malc (sizeof (struct cs_Csdef_));
		if (cs_ptr == NULL)
		{
			CS_erpt (cs_NO_MEM);
			goto error;
		}
		rd_st = CS_csrd (strm,cs_ptr,&crypt);
		if (rd_st <= 0) goto error;
	}
	if (!cs_CsStrmFlg)
	{
		/* Deferred close is not active, so close the stream. */
		CS_csDictCls (strm);
	}
	return (cs_ptr);

error:
	if (strm != NULL) CS_csDictCls (strm);
	if (cs_ptr != NULL) CS_free (cs_ptr);
	return (NULL);
}

/**********************************************************************
**	CS_csfnm (new_name);
**
**	char *new_name;				the name of the coordinate system dictionary file.
**
**	This function specifies the name only.  The directory,
**	and possibly drive, are specified using CS_altdr.
**********************************************************************/
void EXP_LVL1 CS_csfnm (Const char *new_name)

{
	extern char cs_Csname [];

	(void)CS_stncp (cs_Csname,new_name,cs_FNM_MAXLEN);
	return;
}
void EXP_LVL1 CS_usrCsfnm (Const char *new_name)

{
	extern char cs_UsrCsName [];

	(void)CS_stncp (cs_UsrCsName,new_name,cs_FNM_MAXLEN);
	return;
}
