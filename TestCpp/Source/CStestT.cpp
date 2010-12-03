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

//lint -esym(715,verbose,duration)
//lint -esym(752,cs_Doserr,cs_Error,cs_Errno,cs_ErrSup,csErrlng,csErrlat)

#include "cs_map.h"
#include "csTestCpp.hpp"

extern "C"
{
	extern int cs_Error;
	extern int cs_Errno;
	extern int csErrlng;
	extern int csErrlat;
	extern unsigned short cs_ErrSup;

	#if _RUN_TIME <= _rt_UNIXPCC
	extern ulong32_t cs_Doserr;
	#endif
}

/* This is the Temporary test module.  That is, simply a module which ordinarily
   succeeds at doing nothing (it's also very fast :>).  The purpose here is
   to provide a place where it is easy to place some temporary code into the test
   module environment.  That is, write some quick code and get it compiled and
   run it without all the hassels of establishing a solution, a project, set
   all the parameters, etc. etc.
   
   Case in point, I needed to generate a list of test points for a specific
   conversion before a major change so that I could verify that the change
   did not produce any regressions.  Thus, I simply add the code here and
   run the console test module with the /tT option.  Whalla!!!  I got it done
   in 30 minutes instead of two hours.
*/
  
int CStestT (bool verbose,long32_t duration)
{
    int err_cnt = 0;

#ifndef __SKIP__
    int st;

    unsigned idx;
    unsigned gxIdxCnt;

    Const struct cs_GxIndex_* gxIdxPtr;
    struct cs_GeodeticTransform_ *gxDefPtr;

    int err_list [8];

    gxIdxCnt = CS_getGxIndexCount ();
    for (idx = 0;idx < gxIdxCnt;idx++)
    {
        gxIdxPtr = CS_getGxIndexEntry (idx);
        if (gxIdxPtr == NULL)
        {
            err_cnt += 1;
        }
        else
        {
            gxDefPtr = CS_gxdef (gxIdxPtr->xfrmName);
            if (gxDefPtr == NULL)
            {
                err_cnt += 1;
            }
            else
            {
                st = CS_gxchk (gxDefPtr,cs_GXCHK_DATUM | cs_GXCHK_REPORT,err_list,sizeof (err_list) / sizeof (int));
                if (st != 0)
                {
                    printf ("CS_gxchk failed on geodetic transformation named %s.\n",gxDefPtr->xfrmName);
                    err_cnt += 1;
                }
                CS_free (gxDefPtr);
            }
        }
    }
#endif
#ifdef __SKIP__
	int st;
	printf ("Running temporary test code module.\n");


	struct cs_GeodeticTransform_* gx_def1;
	struct cs_GeodeticTransform_* gx_def2;


	gx_def1 = CS_gxdef ("NAD27_to_NAD83");
	gx_def2 = CS_gxdef ("ABIDJAN-87_to_WGS84");
	
	st = CS_gxupd (gx_def1);
	st = CS_gxupd (gx_def2);

	gx_def1 = CS_gxdef ("NAD27_to_NAD83");
	gx_def2 = CS_gxdef ("ABIDJAN-87_to_WGS84");
#endif
#ifdef __SKIP__
	int st;

	// CsMap-12.02-23729,CRS2D,CSMAP,LL-ATS77,CSMAP,LL27,-69.05,43.781625,10.0,-69.05,43.781625,0,1E-10,1E-10,0.001,,
	// CsMap-12.02-23450,CRS2D,CSMAP,LL-AGD84-Grid,CSMAP,LL27,170.6875,-24.5,50,170.6895457,-24.50144547,0,1.00E-10,1.00E-10,0.001,,
	// CsMap-12.02-052855,CRS3D,CSMAP,Korean95.LL,CSMAP,LL27,245.5425,29.5171875,0,-114.45678768113,29.517060058843,-0.700176200928,5E-09,5E-09,0.0006,CsMap-12.02,Generated by automatic processes using CS-MAP release 12.02
	// CsMap-12.02-056726,CRS3D,CSMAP,LL-HPGN,CSMAP,Tokyo,244.2875,30.4675,0,-115.708821779906,30.459854254511,0,5E-09,5E-09,0.0006,CsMap-12.02,Generated by automatic processes using CS-MAP release 12.02
	// CsMap-12.02-057419,CRS3D,CSMAP,LL27,CSMAP,Tokyo,237.8490625,36.10875,0,-122.147625065633,36.101296387742,0.835663769569,5E-09,5E-09,0.0006,CsMap-12.02,Generated by automatic processes using CS-MAP release 12.02

    const char* csOneName = "LL27";
    const char* csTwoName = "Tokyo";

    struct cs_Csprm_ *csOne;
    struct cs_Csprm_ *csTwo;
 	struct cs_Dtcprm_ *dtcPrm;
 
	double llTmp [3];
 	
    printf ("Running temporary test code module.\n");

    csOne = CS_csloc (csOneName);
    csTwo = CS_csloc (csTwoName);
    if (csOne == NULL || csTwo == NULL)
    {
        return 1;
    }

    dtcPrm = CS_dtcsu (csOne,csTwo,cs_DTCFLG_DAT_F,cs_DTCFLG_BLK_W);
    if (dtcPrm == NULL)
    {
        return 1;
    }
    
    llTmp [0] = -122.1509375;
    llTmp [1] = 36.10875;
    llTmp [2] = 0.0;
 
	st = CS_dtcvt3D (dtcPrm,llTmp,llTmp);
    
	if (st != 0)
	{
		err_cnt += 1;
	}
	
	CS_dtcls (dtcPrm);
#endif
#ifdef __SKIP__
    int st;
    int counter;
    FILE* tstStrm;
    struct cs_Csprm_ *csOne;
    struct cs_Csprm_ *csTwo;
 	struct cs_Dtcprm_ *dtcPrm;
 	
 	double lngMin = -5.5000;
 	double lngMax = 10.0000;
 	double latMin = 41.0000;
 	double latMax = 52.0000;
 	
 	double llOne [3];
 	double llTmp [3];
 	double llTwo [3];
 
    const char* csOneName = "LL-RGF93";
    const char* csTwoName = "NTF.LL";
 
    tstStrm = fopen ("C:\\Tmp\\TestPoints.txt","wt");
    if (tstStrm == NULL)
    {
        return 1;
    }

    csOne = CS_csloc (csOneName);
    csTwo = CS_csloc (csTwoName);
    if (csOne == NULL || csTwo == NULL)
    {
        return 1;
    }
    dtcPrm = CS_dtcsu (csOne,csTwo,cs_DTCFLG_DAT_F,cs_DTCFLG_BLK_W);
    if (dtcPrm == NULL)
    {
        return 1;
    }

    for (counter = 0;counter < duration;counter += 1)
    {
        st = 0;
        llOne [0] = CStestRN (lngMin,lngMax);
        llOne [1] = CStestRN (latMin,latMax);
        llOne [2] = 0.0;
        st  = CS_cs3ll (csOne,llTmp,llOne);
        st |= CS_dtcvt (dtcPrm,llTmp,llTmp);
        st |= CS_ll3cs (csTwo,llTwo,llTmp);
        
        fprintf (tstStrm,"%s,%.9f,%.9f,%s,%.9f,%.9f,1.0E-08,1.0E-08\n",csOneName,llOne [0],
                                                                                 llOne [1],
                                                                                 csTwoName,
                                                                                 llTwo [0],
                                                                                 llTwo [1]);
        fprintf (tstStrm,"%s,%.9f,%.9f,%s,%.9f,%.9f,1.0E-08,1.0E-08\n",csTwoName,llTwo [0],
                                                                                 llTwo [1],
                                                                                 csOneName,
                                                                                 llOne [0],
                                                                                 llOne [1]);
        if (st != 0)
        {
            err_cnt += 1;
        }
    }
    fclose (tstStrm);
#endif
#ifdef __SKIP__
	int st;

    const char* dtOneName = "AFGOOYE";
    const char* dtTwoName = "WGS84";

    struct cs_Datum_ *dtOne;
    struct cs_Datum_ *dtTwo;
 	struct cs_Dtcprm_ *dtcPrm;
 
	double llTmp [3];
 	
    printf ("Running temporary test code module.\n");

    dtOne = CS_dtloc (dtOneName);
    dtTwo = CS_dtloc (dtTwoName);
    if (dtOne == NULL || dtTwo == NULL)
    {
        return 1;
    }

    dtcPrm = CSdtcsu (dtOne,dtTwo,cs_DTCFLG_DAT_F,cs_DTCFLG_BLK_W);
    if (dtcPrm == NULL)
    {
        return 1;
    }

	CS_dtcls (dtcPrm);
#endif

    return err_cnt;
}
