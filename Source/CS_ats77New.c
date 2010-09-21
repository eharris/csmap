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

int CSats77S  (struct cs_GridFile_ *ats77)
{
	CS_erpt (cs_DTC_DAT_F);
	return -1;
}
double CSats77T (struct cs_Ats77_ *ats77,double *ll_src)
{
	return 0.0;
}
int CSats77F2 (struct cs_Ats77_ *ats77,double *ll_trg,Const double *ll_src)
{
	return 0;
}
int CSats77F3 (struct cs_Ats77_ *ats77,double *ll_trg,Const double *ll_src)
{
	return 0;
}
int CSats77I2 (struct cs_Ats77_ *ats77,double *ll_trg,Const double *ll_src)
{
	return 0;
}
int CSats77I3 (struct cs_Ats77_ *ats77,double *ll_trg,Const double *ll_src)
{
	return 0;
}
int CSats77L  (struct cs_Ats77_ *ats77,int cnt,Const double pnts [][3])
{
	return 0;
}
int CSats77Q  (struct cs_GridFile_* gridFile,unsigned short prj_code,int err_list [],int list_sz)
{
	return 0;
}
int CSats77R  (struct cs_Ats77_ *ats77)
{
	return 0;
}
int CSats77D  (struct cs_Ats77_ *ats77)
{
	return 0;
}
