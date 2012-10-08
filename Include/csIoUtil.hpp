/*
 * Copyright (c) 2012, Autodesk, Inc.
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

#ifndef __CS_IO_UTIL_HPP__
#define __CS_IO_UTIL_HPP__

#include <vector>
#include <stdio.h>

extern "C" 
{
	#include "cs_map.h"
	#include "cs_ioUtil.h"

	extern char csErrnam [];

	extern char cs_Csname[];
	extern char cs_Dtname[];
	extern char cs_Elname[];
	extern char cs_Ctname[];
	extern char cs_Gxname[];
	extern char cs_Gpname[];

	extern char cs_Dir[];
	extern char cs_UserDir[];
	extern char* cs_DirP;

	extern char cs_Unique;
	extern char cs_DirsepC;
	extern short cs_Protect;
	extern int cs_Error;

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
	Hook function to support the use of temporary ellipsoid definitions.

	To activate, set the global CS_usrElDefPtr variable to point to your
	function.  To deactivate, set CS_usrElDefPtr to NULL.
 
	CS_eldef calls the (*CS_usrElDefPtr) function before it does anything
	with the name.  Thus, temporary names need not adhere to any CS-MAP
	convention regarding key names.

	(*CS_usrElDefPtr) should return:
	-1 to indicate an error of sorts, in which case the error condition
	must have already been reported to CS_erpt.
	+1 to indicate that normal CS_eldef processing is to be performed.
	0 to indicate that the cs_Eldef_ structure provided by the first
	argument has been properly filled in with the desired definition
	which is to be returned by CS_eldef.

	In the case where (*CS_usrElDefPtr) returns 0, CS_eldef will return
	a copy of the data provided in a chunk of memory it malloc'ed from
	the heap.  Also note, that the data returned is not checked.  You
	are responsible for making sure this data is valid.
	**********************************************************************/
	extern int (*CS_usrElDefPtr) (struct cs_Eldef_ *elDef,Const char *keyName);
}

#include "csIoUtilAutoPtr.hpp"
#include "csIoUtilDirSwitch.hpp"
#include "csIoUtilDirIterator.hpp"

#define CS_GET_CURRENT_IO_TIME(x) \
	short x = (short)((CS_time ((cs_Time_ *)0) - 630720000L) / 86400L)

void CSFileClose(csFILE* pFile)
{
	if (NULL != pFile)
		CS_fclose(pFile);
}

typedef CS_AutoPtr<csFILE, CSFileClose> CSFileAutoPtr;
typedef CS_AutoPtr<void, CS_free> CSAutoPtr;

template<class TCsMapStruct>
int CS_IsWriteProtectedT(const TCsMapStruct *const def, bool& isProtected)
{
	CS_CHECK_NULL_ARG(def, 1);

	isProtected = true;

	if (cs_Protect < 0) //system dictionary files are protected
	{
		isProtected = false;
		return 0;
	}

	//protection is globally disabled in case of cs_Protect being < 0
	if (0 == cs_Protect) //system dictionary files are protected
	{
		//all system definitions have their [protect] field set to 1 by default
		isProtected = (1 == def->protect);
	}
	else if (cs_Protect > 0)
	{
		CS_GET_CURRENT_IO_TIME(cs_time);
		isProtected = false;

		//definitions becomes protected after a given amount of days, aka...
		/*  We protect user defined systems only
			if cs_Protect is greater than zero. */
		if (def->protect > 0)
		{
			if (def->protect < (cs_time - cs_Protect))		/*lint !e644 */
			{
				/* It has been more than cs_Protect
					days since this coordinate system
					has been twiddled, we consider it
					to be protected. */
				isProtected = true;
			}
		}
	}

	return 0;
}

template<class TCsMapStruct>
int CS_SetCurrentTimeT(TCsMapStruct* def)
{
	CS_CHECK_NULL_ARG(def, 1);

	if (def->protect >= 0)
	{
		if ((cs_Protect < 0) || (def->protect != 1))
		{
			CS_GET_CURRENT_IO_TIME(cs_time);
			/* This modifies the user supplied definition. */
			def->protect = cs_time;
		}
	}

	return 0;
}

template<class TCsMapStruct>
int CS_DescribeT(csFILE *strm, TCsMapStruct const* def, bool& exists, bool& isProtected, TCsMapStruct*& dictionaryDef,
	int (*TRead)(csFILE*, TCsMapStruct*),
	int (*TReadCrypt)(csFILE*, TCsMapStruct*, int*),
	int (*TCompare)(TCsMapStruct const* pp, TCsMapStruct const* qq))
{
	cs_Error = 0;

	CS_CHECK_NULL_ARG(strm, 1);
	CS_CHECK_NULL_ARG(def, 2);
	
	dictionaryDef = NULL;

	exists = false;
	isProtected = true;


	long32_t fpos;
	const size_t blockSize = sizeof(TCsMapStruct);

	/* See if we have a definition with that name already. */
	int flag = CS_bins (strm,(long32_t)sizeof (cs_magic_t),(long32_t)0, blockSize, def, (CMPFUNC_CAST)TCompare);
	if (flag < 0)
		return -1;

	if (0 == flag)
	{
		//nothing was found, i.e. no def with that name exists;
		//our return variables are initialized correctly already
		return 0;
	}

	exists = true;

	//store the position within the current file
	fpos = (long32_t) CS_ftell(strm);
	if (fpos < 0L)
	{
		CS_erpt (cs_IOERR); //the file is corrupt, IO error (?!)
		return -1;
	}

	TCsMapStruct* pLocalDef = (TCsMapStruct*) CS_malc (blockSize);
	if (NULL == pLocalDef)
	{
		CS_erpt (cs_NO_MEM);
		return -1;
	}

	CSAutoPtr localDefRelease(pLocalDef);

	int crypt = 0;
	int st;

	//read the entry from the file into our [my_csdef] variable
	if (NULL != TRead)
	{
		st = TRead(strm, pLocalDef);
	}
	else if (NULL != TReadCrypt)
	{
		st = TReadCrypt(strm, pLocalDef, &crypt);
	}
	else
	{
		CS_erpt(cs_ISER); //we must be given a valid function pointer
		return -1;
	}

	if (st <= 0)
	{
		if (0 == st)
			CS_erpt (cs_INV_FILE); //EOF

		return -1;
	}

	/* Here when the definition already exists. See
		if it is OK to write it. If cs_Protect is less than
		zero, all protection has been turned off. */
	if (CS_IsWriteProtectedT(def, isProtected)) //if non-0, something went wrong
	{
		return -1;
	}

	//set the file pointer back to its position
	st = CS_fseek (strm, (long)fpos, SEEK_SET);
	if (st != 0)
	{
		CS_erpt (cs_IOERR);
		return -1;
	}

	dictionaryDef = reinterpret_cast<TCsMapStruct*>(localDefRelease.Release());
	return 0;
}

template<class TCsMapStruct,
	size_t _KeyNameSize>
int CS_DefinitionRead(csFILE *& strm, TCsMapStruct *& def, char (&keyName)[_KeyNameSize], const char* const swapFormat = NULL, unsigned char* encryptKeyAddress = NULL, int *crypt = NULL,
	int (*TSwap)(TCsMapStruct*) = NULL)
{
	cs_Error = 0;

	CS_CHECK_NULL_ARG(strm, 1);
	CS_CHECK_NULL_ARG(def, 2);

	/* Synchronize the strm before the read. */
	int st = CS_fseek (strm, 0L, SEEK_CUR);
	if (st != 0)
	{
		CS_erpt (cs_IOERR);
		return (-1);
	}

	size_t blockSize = sizeof (TCsMapStruct);
	/* OK, now we can read. */
	size_t rdCnt = CS_fread (def, 1, blockSize, strm);
	if (rdCnt != blockSize)
	{
		if (CS_feof (strm))
		{
			return 0;
		}

		if (CS_ferror (strm)) CS_erpt (cs_IOERR);
		else				  CS_erpt (cs_INV_FILE);
		return (-1);
	}

	if (NULL != encryptKeyAddress)
	{
		unsigned char key = *encryptKeyAddress;

		if (key != '\0')
		{
			if (crypt)
				*crypt = TRUE;

			unsigned char* start = (unsigned char*)def;
			unsigned char* end = start + sizeof(*def);

			while (start < end)
			{
				key ^= *start;
				*start++ = key;
			}
		}
		else if (crypt)
		{
			*crypt = FALSE;
		}
	}

	/* Swap bytes as necessary */
	if (NULL == TSwap)
		CS_bswap (def, swapFormat);
	else
		TSwap(def);

	/* Check the result. The name must always meet the criteria
	   set by the CS_nmpp[64] function.  At least so far, the criteria
	   established by CS_nampp over the years has always been
	   expanded, never restricted.  Thus, any definition which
	   was legitimate in a previous release would always be
	   legitimate iin subsequent releases. */
	char tmpKeyName[_KeyNameSize];
	CS_stncp (tmpKeyName, keyName, _KeyNameSize);
	if (CSnampp(tmpKeyName, _KeyNameSize) != 0)
	{
		/* Replace the error condition reported by CS_nampp with
		   and Invalid File indication. */
		CS_erpt (cs_INV_FILE);
		return (-1);
	}

	if (encryptKeyAddress)
	{
		*encryptKeyAddress = '\0';
		*(encryptKeyAddress + 1) = '\0';
	}

	return (1);
}

template<class TCsMapStruct, int notFoundErrorCode, size_t _KeyNameSize>
TCsMapStruct* DefinitionGet(
	TCsMapStruct& keyDef,
	char (&keyName)[_KeyNameSize],
	char* pszDirPath,
	csFILE* (*TOpen)(const char* mode),
	int (*TRead)(csFILE*, TCsMapStruct*),
	int (*TReadCrypt)(csFILE*, TCsMapStruct*, int*),
	int (*TCompare)(TCsMapStruct const* pp, TCsMapStruct const* qq),
	int (*TUserDefPtr)(TCsMapStruct*, char const*) = NULL,
	int* isUsrDef = NULL)
{
	cs_Error = 0;

	CS_CHECK_NULL_ARG_RETURN(keyName, 1, NULL);

	if (NULL != isUsrDef)
		*isUsrDef = FALSE;

	//make sure, we switch back to the directory we were in before
	CSDictionarySwitch dictionaryReset;

	__ALIGNMENT__1			/* For some versions of Sun compiler. */
	const size_t blockSize = sizeof(TCsMapStruct);

	/* Give the application first shot at satisfying this request. */
	if (NULL != TUserDefPtr)
	{
		__ALIGNMENT__2			/* For some versions of Sun compiler. */
		TCsMapStruct localDef;
		int userDefGetStatus = TUserDefPtr(&localDef, keyName);
		if (userDefGetStatus < 0) return NULL;
		if (userDefGetStatus == 0)
		{
			if (NULL != isUsrDef)
				*isUsrDef = TRUE;

			TCsMapStruct* defPtr = (TCsMapStruct*)CS_malc (blockSize);
			if (defPtr == NULL)
			{
				CS_erpt (cs_NO_MEM);
				return NULL;
			}

			memmove (defPtr, &localDef, sizeof(*defPtr));
			return defPtr;
		}
	}

	/* Make sure the key is not too large. */
	if (CSnampp(keyName, _KeyNameSize))
		return NULL;

	CsdDictionaryIterator dictionaryIterator(TOpen);

	//go through all directories we've
	while(dictionaryIterator.MoveNext())
	{
		csFILE* dictionaryFile = dictionaryIterator.GetCurrentFile();

		/* Locate the coordinate system which we are to return. */
		int flag = CS_bins (dictionaryFile, (long32_t) sizeof (cs_magic_t), (long32_t)0, blockSize,
			&keyDef, (CMPFUNC_CAST)TCompare);
		if (flag <= 0)
			continue;
	
		/* The definition exists. Therefore,
			we malloc the definition structure and read it in. */
		TCsMapStruct* pDef = (TCsMapStruct*) CS_malc (blockSize);
		if (NULL == pDef)
		{
			//this is a serious proplem; get out here immediately
			CS_erpt (cs_NO_MEM);
			return NULL;
		}

		//the position within the file had been set by [CS_bins] already
		if (    (TRead && TRead(dictionaryFile, pDef) <= 0)
			||  (TReadCrypt && TReadCrypt(dictionaryFile, pDef, NULL) <= 0)) //ignore this one and try reading from the next file/directory - if there was one
		{
			CS_free(pDef);
			continue;
		}

		if (NULL != pszDirPath)
			CS_stncp(pszDirPath, dictionaryIterator.GetCurrentDir(), MAXPATH);
		
		return pDef;
	}

	CS_stncp (csErrnam, keyName, MAXPATH);
	CS_erpt (notFoundErrorCode);

	return NULL;
}

template<class TCsMapStruct>
int DefinitionGetAll(TCsMapStruct* *pAllDefs[],
	csFILE* (*TOpen)(const char* mode),
	int (*TRead)(csFILE*, TCsMapStruct*),
	int (*TReadCrypt)(csFILE*, TCsMapStruct*, int*))
{
	cs_Error = 0;

	CS_CHECK_NULL_ARG_RETURN(pAllDefs, 1, -1);
	*pAllDefs = NULL;

	size_t arraySize = 0;
	TCsMapStruct** pFirstEntry = NULL;

	bool readCrypt;
	if (NULL != TReadCrypt)
		readCrypt = true;
	else if (NULL != TRead)
		readCrypt = false;
	else
	{
		CS_erpt(cs_ISER);
		return -1;
	}

	CsdDictionaryIterator dictionaryIterator(TOpen);
	std::vector<TCsMapStruct*> allDefs;

	int readStatus = 0;
	int wasCrypted = 0;

	//go through all directories we've
	while(dictionaryIterator.MoveNext())
	{
		csFILE* dictionaryFile = dictionaryIterator.GetCurrentFile();
		do
		{
			TCsMapStruct* pDef = (TCsMapStruct*) CS_malc(sizeof(TCsMapStruct));
			if (NULL == pDef)
				CS_erpt(cs_NO_MEM);

			readStatus = (readCrypt) ? TReadCrypt(dictionaryFile, pDef, &wasCrypted) :
										TRead(dictionaryFile, pDef);
			if (readStatus > 0)
				allDefs.push_back(pDef);
			else
				CS_free(pDef);

		} while(readStatus > 0);

		if (readStatus)
			goto error;
	}

	arraySize = allDefs.size() * sizeof(TCsMapStruct*);
	(*pAllDefs) = (TCsMapStruct**)CS_malc(arraySize);
	if (NULL == *pAllDefs)
	{
		CS_erpt(cs_NO_MEM);
		goto error;
	}

	pFirstEntry = &allDefs[0];

	memset(*pAllDefs, 0x0, arraySize);
	memcpy(*pAllDefs, pFirstEntry, arraySize);

	return allDefs.size();

error:
	for(typename std::vector<TCsMapStruct*>::const_iterator defIterator = allDefs.begin(); 
		defIterator != allDefs.end(); ++defIterator)
	{
		CS_free(*defIterator);
	}
	allDefs.clear();

	return -1;
}

template<class TCsMapStruct>
int CS_DefinitionWrite (csFILE *& strm, TCsMapStruct *& def, const char* const swapFormat = NULL, unsigned char* encryptKeyAddress = NULL, int (*TSwap)(TCsMapStruct*) = NULL)
{
	static unsigned seed = 0;

	TCsMapStruct defOrig = *def;

	/* Swap bytes as necessary */
	if (NULL == TSwap)
		CS_bswap (def, swapFormat);
	else
		TSwap(def);

	bool crypt = (NULL != encryptKeyAddress);

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
			unsigned char key = (unsigned char) rand ();
			unsigned char* cpe = (unsigned char*) def;
			unsigned char* cp = cpe + sizeof(*def);
			*encryptKeyAddress = (char)key;
			*(encryptKeyAddress + 1) = (char)rand ();
			while (--cp > cpe)
			{
				*cp ^= *(cp - 1);
			}
			*cp ^= (unsigned char)*encryptKeyAddress;
			if (*encryptKeyAddress != '\0') break;

			/* If the key ends up to be zero,
			   we need to try another one. */
			*def = defOrig;

			/* Swap bytes as necessary */
			if (NULL == TSwap)
				CS_bswap (def, swapFormat);
			else
				TSwap(def);
		}
	}

	/* Synchronize the stream prior to the write. */
	int st = CS_fseek (strm,0L,SEEK_CUR);
	if (st != 0)
	{
		CS_erpt (cs_IOERR);
		return TRUE;
	}

	size_t blockSize = sizeof (TCsMapStruct);

	/* Now we can write. */
	size_t wrCnt = CS_fwrite(def, blockSize, 1, strm);
	if (wrCnt != 1)
	{
		if (CS_ferror (strm)) CS_erpt (cs_IOERR);
		else				  CS_erpt (cs_DISK_FULL);
		return TRUE;
	}

	return FALSE;
}

template<class TCsMapStruct,
	cs_magic_t const _MagicHeader,
	size_t _KeyNameSize>
int CS_DefinitionDelete(TCsMapStruct const* def, char (&keyName)[_KeyNameSize],
	csFILE* (*TOpen)(const char* mode),
	TCsMapStruct* (*TDef)(char const*, char*),
	int (*TRead) (csFILE *strm, TCsMapStruct*),
	int (*TReadCrypt) (csFILE *strm, TCsMapStruct*, int*),
	int (*TWrite) (csFILE *strm, TCsMapStruct const*),
	int (*TWriteCrypt) (csFILE *strm, TCsMapStruct const*, int),
	int (*TCompare)(TCsMapStruct const* pp, TCsMapStruct const* qq))
{
	CS_CHECK_NULL_ARG(def, 1);

	CSDictionarySwitch dictionarySwitch;

	char tmp_nam [MAXPATH];

	__ALIGNMENT__1		/* For some versions of Sun compiler. */
	TCsMapStruct keyBufr;

	__ALIGNMENT__2		/* For some versions of Sun compiler. */
	TCsMapStruct cpy_buf;

	/* Set up a key structure which we will use to identify all path records
	   which belong to the definition which is to be deleted. */
	keyBufr = *def;

	char csFileDirPath[MAXPATH] = { '\0' };

	/* Get a pointer to the existing definition. If it doesn't
	   exist, we're all done. */
	TCsMapStruct* my_ptr = TDef(keyName, csFileDirPath);
	if (NULL == my_ptr)
		return -1;

	CSAutoPtr myPtrDelete(my_ptr);

	bool isWriteProtected = false;
	if (CS_IsWriteProtectedT(my_ptr, isWriteProtected)) //if non-0, something went wrong
	{
		return -1;
	}

	/* 
	Open up the definition dictionary file and verify its
	magic number. Note, that we'll have to open the dictionary from the path
	where TDef() did operate on
	*/

	//note, that we'll automatically switch away from that directory when exiting this function
	CS_setdr(csFileDirPath, NULL);

	csFILE* old_strm = TOpen (_STRM_BINRD);
	if (old_strm == NULL)
		return -1;

	CSFileAutoPtr oldStreamClose(old_strm);

	/* Create a temporary file for the new dictionary. */
	if (CS_tmpfn (tmp_nam))
		return -1;
	
	csFILE* new_strm = CS_fopen (tmp_nam,_STRM_BINWR);
	if (new_strm == NULL)
	{
		CS_erpt (cs_TMP_CRT);
		return -1;
	}

	CSFileAutoPtr newStreamClose(new_strm);

	/* Copy the file, skipping the entry to be deleted.  First
	   we must write the magic number. */
	cs_magic_t magic = _MagicHeader;
	CS_bswap (&magic,"l");
	size_t wrCnt = CS_fwrite ((char *)&magic,1,sizeof (magic),new_strm);
	if (wrCnt != sizeof (magic))
	{
		if (CS_ferror (new_strm)) CS_erpt (cs_IOERR);
		else					  CS_erpt (cs_DISK_FULL);

		return -1;
	}

	bool readEncrypted;
	if (NULL != TRead)
		readEncrypted = false;
	else if (NULL != TReadCrypt)
		readEncrypted = true;
	else
	{
		CS_erpt(cs_ISER);
		return -1;
	}

	bool writeEncrypted;
	if (NULL != TWrite)
		writeEncrypted = false;
	else if (NULL != TWriteCrypt)
		writeEncrypted = true;
	else
	{
		CS_erpt(cs_ISER);
		return -1;
	}


	int wasEncrypted = 0;
	/* Now we copy the file, skipping the entry to be
	   deleted. */
	while (((readEncrypted) ?   TReadCrypt(old_strm, &cpy_buf, &wasEncrypted)
							:   TRead(old_strm, &cpy_buf)) > 0)
	{
		if (TCompare (&cpy_buf, &keyBufr) != 0)
		{
			int writeStatus = -1;
			if (writeEncrypted)
				writeStatus = TWriteCrypt(new_strm, &cpy_buf, wasEncrypted);
			else
				writeStatus = TWrite(new_strm, &cpy_buf);

			if (writeStatus)
			{
				return -1;
			}
		}
	}

	/* Close up, remove the old dictionary and rename the
	   new dictionary. */
	newStreamClose.Reset(NULL);
	oldStreamClose.Reset(NULL);

	if(CS_remove (cs_Dir))
	{
		strcpy (csErrnam,cs_Dir);
		CS_erpt (cs_UNLINK);

		return -1;
	}

	return CS_rename (tmp_nam,cs_Dir);
}


template<class TCsMapStruct, 
	cs_magic_t const _MagicHeader,
	char * _Filename,
	size_t _KeyNameSize>
int CS_DefinitionUpdate (TCsMapStruct *toUpdate, char (&keyName)[_KeyNameSize],
	csFILE* (*TOpen)(const char*),
	int (*TRead)(csFILE*, TCsMapStruct*),
	int (*TReadCrypt)(csFILE*, TCsMapStruct*, int*),
	int (*TWrite)(csFILE*, TCsMapStruct const*),
	int (*TWriteCrypt)(csFILE*, TCsMapStruct const*, int),
	int (*TCompare)(TCsMapStruct const*, TCsMapStruct const*),
	int (*TDescribeOverride)(TCsMapStruct*, TCsMapStruct const*, bool, bool&),
	bool encrypt = false)
{
	CS_CHECK_NULL_ARG(toUpdate, 1);

	/*  Adjust the name and make sure it is proper.
		Checks characters & length */
	if(CSnampp(keyName, _KeyNameSize))
		return -1;

	CSDictionarySwitch csUserDirSwitch;

	//do updates have to go into our separate dictionary?
	int hasUserDictionary = csUserDirSwitch.CanSwitchToUserDir();
	
	csFILE *systemFileStream = NULL;
	csFILE *userFileStream = NULL;
	csFILE *pTargetFileStream = NULL;


	/* Compute the time, as we use it internally.  This is
	   days since January 1, 1990. If this record does get
	   written, we want it to have the current date in it.
	   This means that even if allowed by the protection system,
	   a distribution coordinate system will be marked as having
	   been twiddled. */
	CS_SetCurrentTimeT(toUpdate);
	
	//first look into our system dictionary and check, whether we're allowed to update the content in there
	//any updates will go into the user's file anyway; that is, we've to create in either case
	if (csUserDirSwitch.CanSwitchToUserDir())
	{
		if (csUserDirSwitch.SwitchToUserDir())
			return -1;

		CS_stcpy(cs_DirP, _Filename);
		if (-1 == CS_access(cs_Dir, 0x0))
		{
			//create the file - assuming it doesn't exist yet
			userFileStream = CS_fopen (cs_Dir, _STRM_BINWR); //mode is "wb", create new file or truncate
			if (NULL == userFileStream)
				return -1;

			//now that we've created the file, we've to write our magic header into it
			cs_magic_t magic = _MagicHeader;
			CS_bswap (&magic,"l"); //swap to little endian format if needed
			if (1 != CS_fwrite(&magic, sizeof(cs_magic_t), 1, userFileStream))
			{
				CS_erpt (cs_IOERR); //files are closed before exiting
				return -1;
			}
		}

		if (NULL != userFileStream)
		{
			CS_fclose(userFileStream);
			userFileStream = NULL;
		}

		if (csUserDirSwitch.SwitchToDefaultDir())
			return -1;
	}

	//open the file in "read-only" if we've user dictionaries we're going to write the data into
	systemFileStream = TOpen(hasUserDictionary ? _STRM_BINRD : _STRM_BINUP);
	if (NULL == systemFileStream)
		return -1;

	CSFileAutoPtr systemDictionaryClose(systemFileStream);

	bool systemDefExists = false;
	bool systemDefProtected = true;
	bool targetDefExists = false;
	bool targetDefProtected = true;

	TCsMapStruct* dictionaryDef = NULL;
	//now check what we can do with the definition, if there's any found, in the system's CSD file
	if (CS_DescribeT<TCsMapStruct>(systemFileStream, toUpdate, systemDefExists, systemDefProtected, dictionaryDef, TRead, TReadCrypt, TCompare))
		return -1;

	CSAutoPtr localDefRelease(dictionaryDef);

	if (systemDefExists && NULL != TDescribeOverride)
	{
		if (TDescribeOverride(toUpdate, dictionaryDef, true /* system definition */, systemDefProtected))
			return -1;
	}

	//system definitions we cannot update - regardless of whether there's a user defined CSD file all
	//custom stuff should go into
	if (systemDefExists && systemDefProtected)
	{
		CS_stncp (csErrnam, keyName, MAXPATH);
		if (1 == toUpdate->protect)
			CS_erpt (cs_GP_PROT);
		else
			CS_erpt (cs_GP_UPROT);

		return -1;
	}

	CSFileAutoPtr userDictionaryClose;

	//check, whether we've a set of dictionaries we want all updates to go into...
	if (!hasUserDictionary)
	{
		//...no? use our default set of dictionaries
		pTargetFileStream = systemFileStream; //file has already been opened in update mode

		targetDefExists = systemDefExists;
		targetDefProtected = systemDefProtected;
	}
	else
	{
		if (csUserDirSwitch.SwitchToUserDir())
			return -1;

		userFileStream = TOpen(_STRM_BINUP);
		if (NULL == userFileStream)
			return -1;

		userDictionaryClose.Reset(userFileStream);

		if (CS_DescribeT<TCsMapStruct>(systemFileStream, toUpdate, targetDefExists, targetDefProtected, dictionaryDef, TRead, TReadCrypt, TCompare))
			return -1;

		localDefRelease.Reset(dictionaryDef);
		if (targetDefExists && NULL != TDescribeOverride)
		{
			if (TDescribeOverride(toUpdate, dictionaryDef, false /* user defined definition */, targetDefProtected))
				return -1;
		}

		//check, whether we can overwrite the definition in the user's file;
		//we handle the files in [cs_UserDir] exactly like anything in [cs_Dir], incl.
		//honoring [cs_Protect]
		if (targetDefExists && targetDefProtected)
		{
			CS_stncp (csErrnam, keyName, MAXPATH);
			if (1 == toUpdate->protect)
				CS_erpt (cs_GP_PROT); //system dictionaries we cannot update
			else
				CS_erpt (cs_GP_UPROT);

			return -1;
		}

		//this is the variable the code below operates on
		pTargetFileStream = userFileStream;
	}

	if (targetDefExists)
	{
		//overwrite the entry in the target dictionary
		int writeStatus;
		if (TWrite)
			writeStatus = TWrite(pTargetFileStream, toUpdate);
		else if (TWriteCrypt)
			writeStatus = TWriteCrypt(pTargetFileStream, toUpdate, encrypt);
		else
		{
			CS_erpt(cs_ISER);
			return -1;
		}
		
		//we've updated the definition; any TWrite should return 0
		//at this point (1 stands for 'failure')
		if (writeStatus) //the error must have been reported here already
			return -1;

		//1 --> def existed and was updated
		return 1;
	}
	
	/* First we write the new coordinate system to the
	end of the file. */
	if (CS_fseek (pTargetFileStream, 0L, SEEK_END) != 0)
	{
		CS_erpt (cs_IOERR);
		return -1;
	}

	int writeStatus;
	if (TWrite)
		writeStatus = TWrite(pTargetFileStream, toUpdate);
	else if (TWriteCrypt)
		writeStatus = TWriteCrypt(pTargetFileStream, toUpdate, encrypt);
	else
	{
		CS_erpt(cs_ISER);
		return -1;
	}

	if (writeStatus)
		return writeStatus;

	/* Sort the file into proper order, thereby
		moving the new definition to its
		proper place in the file. */
	if(CS_fseek (pTargetFileStream,(long)sizeof (cs_magic_t),0))
	{
		CS_erpt (cs_IOERR);
		return -1;
	}

	if (CS_ips (pTargetFileStream,sizeof (TCsMapStruct),0L,(CMPFUNC_CAST)TCompare) < 0)
	{
		return -1;
	}

	return 0; //we've added a definition
}

template<cs_magic_t const _MagicHeader>
csFILE * CS_FileOpen (const char* const filename, const char *const mode)
{
	strcpy (cs_DirP, filename);
	csFILE* strm = CS_fopen (cs_Dir,mode);

	if (strm == NULL)
	{
		strcpy (csErrnam,cs_Dir);
		CS_erpt (cs_GPDICT);

		return NULL;
	}

	CSFileAutoPtr dictionaryFile(strm);

	cs_magic_t magic = 0L;
	size_t rd_cnt = CS_fread ((char *)&magic,1,sizeof (magic),strm);

	if (rd_cnt != sizeof (magic))
	{
		if (CS_ferror (strm))
			CS_erpt (cs_IOERR);
		else
			CS_erpt (cs_INV_FILE);

		return NULL;
	}

	CS_bswap (&magic,"l");
	if (magic != _MagicHeader)
	{
		CS_fclose (strm);
		strm = NULL;

		strcpy (csErrnam,cs_Dir);
		CS_erpt (cs_GP_BAD_MAGIC);

		return NULL;
	}

	return dictionaryFile.Release();
}

#endif
