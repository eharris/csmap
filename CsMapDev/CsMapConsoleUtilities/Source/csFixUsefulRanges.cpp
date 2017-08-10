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

#include "csConsoleUtilities.hpp"

extern "C" unsigned long KcsNmInvNumber;
extern "C" unsigned long KcsNmMapNoNumber;

// Given the table which appears below (generated programmatically), this
// function inserts/updates the useful range of the CS-MAP definition to be
// slightly larger (about 15 minutes) than the corresponding EPSG definition.

struct TcsRngFixTbl_
{
	long epsgCode;
	char csMapKeyNm [32];
}
		KcsRngFixTbl [] =
{
	{  5520,   "DHDN/3.Gauss3d-1"  },
	{  3812,   "ETRF89.Belgian/LM2008"  },
	{  3907,   "MGI1901.Balkans-5"  },
	{  3908,   "MGI1901.Balkans-6"  },
	{  3909,   "MGI1901.Balkans-7"  },
	{  3910,   "MGI1901.Balkans-8"  },
	{  2492,   "Pulkovo42.GK/CM-9E"  },
	{  3833,   "Pulkovo42/58.GK-2"  },
	{  3333,   "Pulkovo42/58.GK-3"  },
	{  4618,   "SAD1969.LL/01"  },
	{  3827,   "TAIWAN1967-TM2-119"  },
	{  3828,   "TAIWAN1967-TM2-121"  },
	{  3821,   "TAIWAN1967.LL"  },
	{  3825,   "TAIWAN1997-TM2-119"  },
	{  3826,   "TAIWAN1997-TM2-121"  },
	{  3824,   "TAIWAN1997.LL"  },
	{ 23031,   "ED50-UTM31"  },
	{ 30792,   "NordSahara1959.UnifieS"  },
	{ 30492,   "Voirol1875_1.SudAlgerie"  },
	{ 23032,   "ED50-UTM32"  },
	{  5681,   "DeutscheBahn.LL"  },
	{  4670,   "IGM1995.LL"  },
	{  3003,   "MonteMario_1.Italy-1"  },
	{ 27392,   "NGO-II-MOD"  },
	{  3064,   "IGM1995.UTM-32N"  },
	{ 26692,   "Mporaloko_1.UTM-32S"  },
	{  2088,   "Carthage.TM-11NE"  },
	{ 22332,   "Carthage.UTM-32N"  },
	{  3942,   "RGF93.CC42"  },
	{ 29333,   "Schwarzk.UTM-33S"  },
	{  3177,   "Libyan2006_1.Libya/TM"  },
	{  4754,   "Libyan2006_1.LL"  },
	{  3199,   "Libyan2006_1.UTM-32N"  },
	{  3190,   "Libyan2006_1.Libya/TM-5"  },
	{ 21782,   "CH1903/GSB.LV03-C"  },
	{ 22092,   "Camacupa_1.TM-12SE"  },
	{ 22032,   "Camacupa_1.UTM-32S"  },
	{  3191,   "Libyan2006_1.Libya/TM-6"  },
	{  3019,   "RT90-3/7Pa.SW-75GONV"  },
	{  4220,   "Camacupa_1.LL"  },
	{  4259,   "Malongo1987.LL"  },
	{ 25932,   "Malongo1987.UTM-32S"  },
	{  3004,   "MonteMario_1.Italy-2"  },
	{ 27394,   "NGO-IV-MOD"  },
	{ 23033,   "ED50-UTM33"  },
	{ 29371,   "Schwarzk.SWAfrican-11"  },
	{  3201,   "Libyan2006_1.UTM-33N"  },
	{  3192,   "Libyan2006_1.Libya/TM-7"  },
	{  3045,   "ETRF89.TM33"  },
	{  3065,   "IGM1995.UTM-33N"  },
	{  4701,   "CongoBelge55.LL"  },
	{  3020,   "RT90-3/7Pa.SW-5GONV"  },
	{  3068,   "DHDN/3.Berlin/Cassini"  },
	{  3021,   "RT90-3/7Pa.SW-25GONV"  },
	{  2176,   "ETRS89.PolandCS2K-5"  },
	{  3193,   "Libyan2006_1.Libya/TM-8"  },
	{ 22275,   "Cape-1.SACS-15"  },
	{  2046,   "Hartebeesthoek94.Lo15"  },
	{  4096,   "ETRF89.DKTM4"  },
	{  2198,   "ETRS89.Kp2K-Bornholm"  },
	{ 27395,   "NGO-V-MOD"  },
	{  3194,   "Libyan2006_1.Libya/TM-9"  },
	{  2177,   "ETRS89.PolandCS2K-6"  },
	{  3022,   "RT90-3/7Pa.SW-0GONV"  },
	{  3011,   "SWEREF-99-18-00"  },
	{ 23034,   "ED50-UTM34"  },
	{  3152,   "SWEREF99.ST74"  },
	{  3202,   "Libyan2006_1.UTM-34N"  },
	{  3195,   "LBY2006.Libya/TM-10"  },
	{  3046,   "ETRF89.TM34"  },
	{  3120,   "Pulkovo42/58b.Poland-1"  },
	{  4284,   "Pulkovo42.LL"  },
	{  3023,   "RT90-3/7Pa.SW-25GONO"  },
	{  3067,   "ETRF89.TM-35/Fin"  },
	{  2523,   "Pulkovo42.GK3d-7"  },
	{  2582,   "Pulkovo42.GK3d/CM-21E"  },
	{ 27396,   "NGO-VI-MOD"  },
	{  3017,   "SWEREF-99-21-45"  },
	{ 22234,   "Cape-1.UTM-34S"  },
	{ 25884,   "ETRS89.TM-Baltic"  },
	{  3109,   "ETRF89.Jersey/TM"  },
	{  3108,   "ETRF89.GuernseyGrid"  },
	{  3844,   "Pulkovo42/58b.Stereo70"  },
	{  3346,   "Lietuvos1994"  },
	{  4669,   "Lithuania94.LL"  },
	{  3128,   "ETRF89.FinlandGK-21"  },
	{  3875,   "ETRS89.GK21FIN"  },
	{  4661,   "Latvia1992.LL"  },
	{  3059,   "Latvia1992.TM"  },
	{  3301,   "Estonia97.Estonia"  },
	{  4180,   "Estonia97.LL"  },
	{  3129,   "ETRF89.FinlandGK-22"  },
	{  3876,   "ETRS89.GK22FIN"  },
	{  3197,   "LBY2006.Libya/TM-12"  },
	{  2524,   "Pulkovo42.GK3d-8"  },
	{  2583,   "Pulkovo42.GK3d/CM-24E"  },
	{ 27397,   "NGO-VII-MOD"  },
	{ 22235,   "Cape-1.UTM-35S"  },
	{ 23035,   "ED50-UTM35"  },
	{  2076,   "ELD1979.Libya-13"  },
	{  2080,   "ELD1979.UTM-35N"  },
	{  3198,   "LBY2006.Libya/TM-13"  },
	{  3203,   "Libyan2006_1.UTM-35N"  },
	{  3047,   "ETRF89.TM35"  },
	{  3132,   "ETRF89.FinlandGK-25"  },
	{  3879,   "ETRS89.GK25FIN"  },
	{  2643,   "Pulkovo95.GK3d-9"  },
	{  2701,   "Pulkovo95.GK3d/CM-27E"  },
	{  2213,   "ETRS89.TM-30NE"  },
	{  3136,   "ETRF89.FinlandGK-29"  },
	{  3883,   "ETRS89.GK29FIN"  },
	{ 23036,   "ED50-UTM36"  },
	{  3137,   "ETRF89.FinlandGK-30"  },
	{  3884,   "ETRS89.GK30FIN"  },
	{ 25836,   "ETRS89.UTM-36N"  },
	{  3048,   "ETRF89.TM36"  },
	{  4129,   "Observatario07.LL"  },
	{ 22991,   "Old-Egyp.Blue"  },
	{ 32766,   "WGS84.TM-36SE"  },
	{ 28192,   "Palestine23a.Belt"  },
	{ 28191,   "Palestine23a.Grid"  },
	{  4281,   "Palestine23a.LL"  },
	{  2039,   "Israel.IsraeliGrid"  },
	{  4141,   "Israel.LL"  },
	{ 28193,   "Palestine23a.IsraeliGrd"  },
	{  3066,   "ED50.Jordan/TM"  },
	{  3037,   "Moznet.UTM-37S"  },
	{ 23037,   "ED50-UTM37"  },
	{  3049,   "ETRF89.TM37"  },
	{  6984,   "IGD-2005.IsraeliGrid"  },
	{  6991,   "IGD-2005/12.IsraeliGrid"  },
	{  6646,   "Karbala79.IraqNtlGrid"  },
	{  4743,   "Karbala79.LL"  },
	{  4163,   "YemenNtl96.LL"  },
	{ 27562,   "NTF.Lambert-2-ClrkIGN"  },
	{ 27572,   "NTF.Lambert-2C-ClrkIGN"  },
	{ 27561,   "NTF.Lambert-1-ClrkIGN"  },
	{  4168,   "Accra1929.LL"  },
	{ 25000,   "Leigon_1.GhanaMetreGrid"  },
	{  4250,   "Leigon_1.LL"  },
	{  2137,   "Accra1929.TM-1NW"  },
	{ 23090,   "Europ50.TM-0N"  },
	{ 20538,   "Afgooye.UTM-38N"  },
	{  2089,   "Yemen96.UTM-38N"  },
	{  4164,   "SouthYemen_1.LL"  },
	{  4713,   "AyabelleLH.LL"  },
	{  2395,   "SouthYemen_1.GK-8"  },
	{  2531,   "Pulkovo42.GK3d-15"  },
	{  2590,   "Pulkovo42.GK3d/CM-45E"  },
	{  2649,   "Pulkovo95.GK3d-15"  },
	{  2707,   "Pulkovo95.GK3d/CM-45E"  },
	{  4632,   "Combani1950.LL"  },
	{  2980,   "Combani1950.UTM-38S"  },
	{  2532,   "Pulkovo42.GK3d-16"  },
	{  2591,   "Pulkovo42.GK3d/CM-48E"  },
	{  2650,   "Pulkovo95.GK3d-16"  },
	{  2708,   "Pulkovo95.GK3d/CM-48E"  },
	{  3200,   "Final1958.Iraq"  },
	{  4132,   "Final1958.LL"  },
	{  4667,   "IraqKuwait1992.LL"  },
	{ 31838,   "NGN.UTM-38N"  },
	{ 24600,   "KuwaitOil.Lambert"  },
	{  4246,   "KuwaitOil.LL"  },
	{  4318,   "NtlGeodeticNet.LL"  },
	{  2090,   "Yemen96.UTM-39N"  },
	{  2396,   "SouthYemen_1.GK-9"  },
	{ 31901,   "KuwaitUtility.KTM"  },
	{  4319,   "KuwaitUtility.LL"  },
	{ 31839,   "NGN.UTM-39N"  },
	{  2938,   "Pulkovo42.CS63-A4"  },
	{  2533,   "Pulkovo42.GK3d-17"  },
	{  2592,   "Pulkovo42.GK3d/CM-51E"  },
	{  2651,   "Pulkovo95.GK3d-17"  },
	{  2709,   "Pulkovo95.GK3d/CM-51E"  },
	{  4710,   "AstroDos71.LL"  },
	{  4693,   "Nakhl-eGhanem.LL"  },
	{  2652,   "Pulkovo95.GK3d-18"  },
	{  2710,   "Pulkovo95.GK3d/CM-54E"  },
	{  2653,   "Pulkovo95.GK3d-19"  },
	{  2711,   "Pulkovo95.GK3d/CM-57E"  },
	{  4626,   "Reunion47.LL"  },
	{  3727,   "Reunion47.Reunion/TM"  },
	{  4699,   "LePouce34.LL"  },
	{  2654,   "Pulkovo95.GK3d-20"  },
	{  2712,   "Pulkovo95.GK3d/CM-60E"  },
	{  3042,   "ETRF89.TM30"  },
	{  2154,   "Lambert93"  },
	{ 23030,   "ED50-UTM30"  },
	{  2655,   "Pulkovo95.GK3d-21"  },
	{  2713,   "Pulkovo95.GK3d/CM-63E"  },
	{  2656,   "Pulkovo95.GK3d-22"  },
	{  2714,   "Pulkovo95.GK3d/CM-66E"  },
	{  2657,   "Pulkovo95.GK3d-23"  },
	{  2715,   "Pulkovo95.GK3d/CM-69E"  },
	{  4724,   "DiegoGarcia69.LL"  },
	{  2541,   "Pulkovo42.GK3d-25"  },
	{  2601,   "Pulkovo42.GK3d/CM-75E"  },
	{  2659,   "Pulkovo95.GK3d-25"  },
	{  2717,   "Pulkovo95.GK3d/CM-75E"  },
	{  4684,   "Gan70.LL"  },
	{  2542,   "Pulkovo42.GK3d-26"  },
	{  2602,   "Pulkovo42.GK3d/CM-78E"  },
	{  2660,   "Pulkovo95.GK3d-26"  },
	{  2718,   "Pulkovo95.GK3d/CM-78E"  },
	{  2543,   "Pulkovo42.GK3d-27"  },
	{  2603,   "Pulkovo42.GK3d/CM-81E"  },
	{  2661,   "Pulkovo95.GK3d-27"  },
	{  2719,   "Pulkovo95.GK3d/CM-81E"  },
	{ 24382,   "IND-IIB/a"  },
	{  2165,   "Abidjan1987.TM-5NW"  },
	{  2164,   "Locodjo1965.TM-5NW"  },
	{ 29901,   "OSNI52/b.IrishNtlGrid"  },
	{  4188,   "OSNI52/b.LL"  },
	{  4231,   "Europ87/a.LL"  },
	{  3106,   "Glshn303.Bangladesh/TM"  },
	{  4682,   "Gulshan303.LL"  },
	{  2546,   "Pulkovo42.GK3d-30"  },
	{  2606,   "Pulkovo42.GK3d/CM-90E"  },
	{  2664,   "Pulkovo95.GK3d-30"  },
	{  2722,   "Pulkovo95.GK3d/CM-90E"  },
	{ 28416,   "CH-16-P"  },
	{  2506,   "Pulkovo42.GK/CM-93E"  },
	{  4277,   "OSGB1936.LL"  },
	{ 27700,   "OSGB1936.NationalGrid"  },
	{  3058,   "Helle1954a.JanMayen"  },
	{  4660,   "Helle1954a.LL"  },
	{  4666,   "Lisbon1890_1.LL"  },
	{ 23866,   "DGN95.UTM-46N"  },
	{  4755,   "Nasional1995.LL"  },
	{  7846,   "GDA2020-7P.MGA-46"  },
	{ 28417,   "CH-17-P"  },
	{  2507,   "Pulkovo42.GK/CM-99E"  },
	{ 23867,   "DGN95.UTM-47N"  },
	{ 23877,   "DGN95.UTM-47S"  },
	{ 24047,   "Indian75/E.UTM-47N"  },
	{  4245,   "Kertau48.LL"  },
	{  4300,   "Ireland-TM75.LL"  },
	{  4678,   "Lao97.LL"  },
	{ 24548,   "Kertau.UTM-48N"  },
	{ 23868,   "DGN95.UTM-48N"  },
	{ 23878,   "DGN95.UTM-48S"  },
	{  4147,   "Hanoi1972.LL"  },
	{  4211,   "Batavia_1.LL"  },
	{ 21148,   "Batavia_1.UTM-48S"  },
	{  2308,   "Batavia_1.TM-109SE"  },
	{  4202,   "LL-ASTRLA66-Grid"  },
	{  3176,   "Indian1960/E.TM-106NE"  },
	{  2094,   "WGS72be/b.TM-106NE/a"  },
	{ 21149,   "Batavia_1.UTM-49S"  },
	{ 20349,   "AMG-49-Grid"  },
	{ 20249,   "AMG66-49-Grid"  },
	{  7849,   "GDA2020-7P.MGA-49"  },
	{ 28349,   "MGA-49"  },
	{ 23869,   "DGN95.UTM-49N"  },
	{ 23879,   "DGN95.UTM-49S"  },
	{  3112,   "GDA94.Geoscience/Lmbrt"  },
	{  4283,   "LL-GDA94"  },
	{ 29849,   "TMBLI-B.UTM-49N"  },
	{  2309,   "WGS84.TM-116SE"  },
	{  7845,   "GDA2020-7P.GA-LCC"  },
	{ 20350,   "AMG-50-Grid"  },
	{ 20250,   "AMG66-50-Grid"  },
	{  7850,   "GDA2020-7P.MGA-50"  },
	{ 28350,   "MGA-50"  },
	{  2326,   "HongKong80b.GridSystem"  },
	{  4611,   "HongKong80b.LL"  },
	{  4739,   "HongKong63/1967.LL"  },
	{ 21150,   "Batavia_1.UTM-50S"  },
	{ 23880,   "DGN95.UTM-50S"  },
	{  4683,   "PRS92/03.LL"  },
	{  2933,   "GunungSegara.UTM-50S"  },
	{  3121,   "PRS92/03.Philippines-1"  },
	{  3122,   "PRS92/03.Philippines-2"  },
	{ 20351,   "AMG-51-Grid"  },
	{ 20251,   "AMG66-51-Grid"  },
	{  7851,   "GDA2020-7P.MGA-51"  },
	{ 28351,   "MGA-51"  },
	{  3123,   "PRS92/03.Philippines-3"  },
	{  3041,   "ETRF89.TM29"  },
	{  3189,   "GR96.UTM-29N"  },
	{ 23871,   "DGN95.UTM-51N"  },
	{ 23881,   "DGN95.UTM-51S"  },
	{  4301,   "Tokyo"  },
	{ 25394,   "Luzon.Philippines-IV"  },
	{  3124,   "PRS92/03.Philippines-4"  },
	{  3097,   "JGD2000.UTM-51N"  },
	{  4737,   "Korean2000.LL"  },
	{ 25395,   "Luzon.Philippines-V"  },
	{  3125,   "PRS92/03.Philippines-5"  },
	{ 20352,   "AMG-52-Grid"  },
	{ 20252,   "AMG66-52-Grid"  },
	{  7852,   "GDA2020-7P.MGA-52"  },
	{ 28352,   "MGA-52"  },
	{  4166,   "Korean95.LL"  },
	{ 23872,   "DGN95.UTM-52N"  },
	{  3098,   "JGD2000.UTM-52N"  },
	{ 23882,   "DGN95.UTM-52S"  },
	{  2560,   "Pulkovo42.GK3d-43"  },
	{  2619,   "Pulkovo42.GK3d/CM-129E"  },
	{  2677,   "Pulkovo95.GK3d-43"  },
	{  2735,   "Pulkovo95.GK3d/CM-129E"  },
	{  3107,   "GDA94.SA/Lambert"  },
	{  6670,   "JGD2011-02"  },
	{  2444,   "JGD2K-02-7P"  },
	{  2444,   "JGD2K.PlnRctCS-II"  },
	{ 30162,   "Tokyo.PlnRctCS-II"  },
	{ 20253,   "AMG66-53-Grid"  },
	{  7853,   "GDA2020-7P.MGA-53"  },
	{ 28353,   "MGA-53"  },
	{  2310,   "WGS84.TM-132SE"  },
	{  3099,   "JGD2000.UTM-53N"  },
	{  3094,   "TOKYO.UTM-53N"  },
	{ 23883,   "DGN95.UTM-53S"  },
	{  6686,   "JGD2011-18"  },
	{  2460,   "JGD2K.PlnRctCS-XVIII"  },
	{ 30178,   "Tokyo.PlnRctCS-XVIII"  },
	{  4637,   "Perroud1950.LL"  },
	{  7854,   "GDA2020-7P.MGA-54"  },
	{ 28354,   "MGA-54"  },
	{ 20254,   "AMG66-54-Grid"  },
	{  3100,   "JGD2000.UTM-54N"  },
	{ 23884,   "DGN95.UTM-54S"  },
	{  6677,   "JGD2011-09"  },
	{  2451,   "JGD2K-09-7P"  },
	{  2451,   "JGD2K.PlnRctCS-IX"  },
	{ 30169,   "Tokyo.PlnRctCS-IX"  },
	{  4636,   "Petrels1972.LL"  },
	{  7899,   "GDA2020-7P.VictGrid"  },
	{  3111,   "GDA94.VictoriaGrid"  },
	{  6680,   "JGD2011-12"  },
	{  2454,   "JGD2K-12-7P"  },
	{  2454,   "JGD2K.PlnRctCS-XII"  },
	{ 30172,   "Tokyo.PlnRctCS-XII"  },
	{  4709,   "IwoJima45.LL"  },
	{  6681,   "JGD2011-13"  },
	{  2455,   "JGD2K-13-7P"  },
	{  2455,   "JGD2K.PlnRctCS-XIII"  },
	{ 30173,   "Tokyo.PlnRctCS-XIII"  },
	{  7855,   "GDA2020-7P.MGA-55"  },
	{ 28355,   "MGA-55"  },
	{  3101,   "JGD2000.UTM-55N"  },
	{ 28426,   "GK-26"  },
	{  2516,   "Pulkovo42.GK/CM-153E"  },
	{  7856,   "GDA2020-7P.MGA-56"  },
	{ 28356,   "MGA-56"  },
	{ 20256,   "AMG66-56-Grid"  },
	{  4616,   "Selvagem.LL"  },
	{  2943,   "Selvagem.UTM-28N"  },
	{  3294,   "WGS84.USGS-AntarticMtn"  },
	{  2568,   "Pulkovo42.GK3d-51"  },
	{  2627,   "Pulkovo42.GK3d/CM-153E"  },
	{  2685,   "Pulkovo95.GK3d-51"  },
	{  2745,   "Pulkovo95.GK3d/CM-153E"  },
	{  3113,   "GDA94.Brisbane"  },
	{  4711,   "Marcus52.LL"  },
	{  2569,   "Pulkovo42.GK3d-52"  },
	{  2628,   "Pulkovo42.GK3d/CM-156E"  },
	{  2686,   "Pulkovo95.GK3d-52"  },
	{  2746,   "Pulkovo95.GK3d/CM-156E"  },
	{ 28427,   "GK-27"  },
	{  2517,   "Pulkovo42.GK/CM-159E"  },
	{ 20257,   "AMG66-57-Grid"  },
	{  7857,   "GDA2020-7P.MGA-57"  },
	{ 28357,   "MGA-57"  },
	{  3169,   "RGN/91-93.UTM-57S"  },
	{  4749,   "RGN-Caledonie/91-93.LL"  },
	{  3170,   "RGN/91-93.UTM-58S"  },
	{  4732,   "Marshalls60.LL"  },
	{  2572,   "Pulkovo42.GK3d-55"  },
	{  2631,   "Pulkovo42.GK3d/CM-165E"  },
	{  2689,   "Pulkovo95.GK3d-55"  },
	{  2749,   "Pulkovo95.GK3d/CM-165E"  },
	{  4643,   "ST71Belep/b.LL"  },
	{  2997,   "ST71Belep/b.UTM-58S"  },
	{  3163,   "RGN/91-93.NewCaledonia"  },
	{  4715,   "CampAreaAstro_1.LL"  },
	{  2133,   "NZGD2K.UTM-58S"  },
	{  2573,   "Pulkovo42.GK3d-56"  },
	{  2632,   "Pulkovo42.GK3d/CM-168E"  },
	{  2690,   "Pulkovo95.GK3d-56"  },
	{  2750,   "Pulkovo95.GK3d/CM-168E"  },
	{  4644,   "Noumea74.LL"  },
	{  2998,   "Noumea74.UTM-58S"  },
	{  3165,   "Noumea74.Noumea"  },
	{  3166,   "Noumea74.Noumea-2"  },
	{  3164,   "ST87/Ouvea.UTM-58S"  },
	{  4730,   "Santo65.LL"  },
	{  4733,   "WakeIs1952.LL"  },
	{  4633,   "IGN56/Lifou.LL"  },
	{  2981,   "IGN56/Lifou.UTM-58S"  },
	{ 28429,   "GK-29"  },
	{  2519,   "Pulkovo42.GK/CM-171E"  },
	{  2134,   "NZGD2K.UTM-59S"  },
	{  4642,   "ST84IlePins/a.LL"  },
	{  2996,   "ST84IlePins/a.UTM-58S"  },
	{  2132,   "NZGD2K.Bluff"  },
	{ 27232,   "NZGD49.Bluff/01"  },
	{  4641,   "IGN53/Mare.a.LL"  },
	{  2995,   "IGN53/Mare.a.UTM-58S"  },
	{  7859,   "GDA2020-7P.MGA-59"  },
	{  2123,   "NZGD2K.JacksonsBay"  },
	{ 27223,   "NZGD49.JacksonsBay/01"  },
	{  3172,   "IGN53/Mare.a.UTM-59S"  },
	{  3171,   "RGN/91-93.UTM-59S"  },
	{  2131,   "NZGD2K.NorthTaieri"  },
	{ 27231,   "NZGD49.NorthTaieri/01"  },
	{  2127,   "NZGD2K.LindisPeak"  },
	{ 27227,   "NZGD49.LindisPeak/01"  },
	{  2122,   "NZGD2K.Okarito"  },
	{ 27222,   "NZGD49.Okarito"  },
	{  2130,   "NZGD2K.ObservationPnt"  },
	{ 27230,   "NZGD49.ObservationPnt/1"  },
	{  4663,   "PortoSanto95.LL"  },
	{  3061,   "PortoSanto95.UTM-28N"  },
	{  4679,   "Jouik61.LL"  },
	{  4230,   "LL-ERP50"  },
	{  2121,   "NZGD2K.Hokitika"  },
	{ 27221,   "NZGD49.Hokitika/01"  },
	{  2125,   "NZGD2K.Gawler"  },
	{ 27225,   "NZGD49.Gawler/01"  },
	{  2118,   "NZGD2K.Grey"  },
	{ 27218,   "NZGD49.Grey/01"  },
	{  2119,   "NZGD2K.Amuri"  },
	{ 27219,   "NZGD49.Amuri/01"  },
	{  2105,   "NZGD2K.MountEden"  },
	{ 27205,   "NZGD49.MountEden/01"  },
	{  2135,   "NZGD2K.UTM-60S"  },
	{ 27260,   "NZGD49.UTM-60S"  },
	{  2113,   "NZGD2K.Wellington"  },
	{ 27213,   "NZGD49.Wellington/01"  },
	{  2106,   "NZGD2K.BayofPlenty"  },
	{ 27206,   "NZGD49.BayOfPlenty/01"  },
	{  3141,   "Fiji56.UTM-60S"  },
	{  4752,   "VitiLevu12.LL"  },
	{  4680,   "Nouakchott65.LL"  },
	{  3040,   "ETRF89.TM28"  },
	{  3056,   "Hjorsey.UTM-28N"  },
	{  3188,   "GR96.UTM-28N"  },
	{  4620,   "Point1958.LL"  },
	{  4702,   "Mauritania1999.LL"  },
	{  3055,   "Hjorsey.UTM-27N"  },
	{  3187,   "GR96.UTM-27N"  },
	{  4657,   "Reykjavik.LL"  },
	{  3054,   "Hjorsey.UTM-26N"  },
	{  4664,   "AzoresEast1995.LL"  },
	{  3062,   "AzoresEast1995.UTM-26N"  },
	{  2190,   "AzoresOrntl40.UTM-26N"  },
	{  4184,   "SAOBRAZ.LL"  },
	{  3057,   "ISN93.IcelandGrid"  },
	{  4659,   "IslandsNet1993.LL"  },
	{  4665,   "AzoresCntrl1995.LL"  },
	{  3063,   "AzoresCntrl1995.UTM-26N"  },
	{  3186,   "GR96.UTM-26N"  },
	{  2188,   "AzoresWest1939.UTM-25N"  },
	{  3185,   "GR96.UTM-25N"  },
	{ 31985,   "SIRGAS2000.UTM-25S"  },
	{ 29195,   "SAD69.UTM-25S"  },
	{  3184,   "GR96.UTM-24N"  },
	{ 31984,   "SIRGAS2000.UTM-24S"  },
	{  3183,   "GR96.UTM-23N"  },
	{ 31983,   "SIRGAS2000.UTM-23S"  },
	{ 31976,   "SIRGAS2000.UTM-22N"  },
	{  3182,   "GR96.UTM-22N"  },
	{ 31982,   "SIRGAS2000.UTM-22S"  },
	{ 22177,   "PGA98.Argentina-7"  },
	{ 22521,   "Corrego.UTM-21S"  },
	{ 22176,   "PGA98.Argentina-6"  },
	{ 31975,   "SIRGAS2000.UTM-21N"  },
	{  2962,   "CSRS.UTM-21N"  },
	{  2945,   "CSRS.MTM-3"  },
	{  3181,   "GR96.UTM-21N"  },
	{ 31981,   "SIRGAS2000.UTM-21S"  },
	{ 29191,   "SAD69.UTM-21S"  },
	{  4736,   "DeceptionIsland_1.LL"  },
	{  2007,   "StVincent1945.BWIgrid"  },
	{  4607,   "StVincent1945.LL"  },
	{ 29220,   "Sapper.UTM-20S"  },
	{ 22175,   "PGA98.Argentina-5"  },
	{  4292,   "Sapper.LL"  },
	{  2946,   "CSRS.MTM-4"  },
	{ 22174,   "PGA98.Argentina-4"  },
	{ 29101,   "SAD1969.BzPolyconic/01"  },
	{  3770,   "BNG2000"  },
	{  2290,   "CANST-PEI-ATS"  },
	{  2954,   "CANST-PEI-CSRS"  },
	{ 31974,   "SIRGAS2000.UTM-20N"  },
	{  2961,   "CSRS.UTM-20N"  },
	{  2947,   "CSRS.MTM-5"  },
	{  3180,   "GR96.UTM-20N"  },
	{ 31980,   "SIRGAS2000.UTM-20S"  },
	{ 29190,   "SAD69.UTM-20S"  },
	{ 22173,   "PGA98.Argentina-3"  },
	{  3075,   "HARN/ME.ME2K-E"  },
	{  3555,   "NSRS07.ME2K-E"  },
	{  6481,   "NSRS11.ME2K-E"  },
	{  4161,   "PampaCastillo.LL"  },
	{  2082,   "pCastillo.Argentina-2"  },
	{ 21899,   "BOGOTA.ColombiaE"  },
	{  3118,   "MAGNA.Columbia-East"  },
	{  3464,   "HARN/ME.ME2K-C/a"  },
	{  3554,   "NSRS07.ME2K-C"  },
	{  6480,   "NSRS11.ME2K-C"  },
	{ 22172,   "PGA98.Argentina-2"  },
	{  3074,   "ME2K-W"  },
	{  3077,   "HARN/ME.ME2K-W"  },
	{  3556,   "NSRS07.ME2K-W"  },
	{  6482,   "NSRS11.ME2K-W"  },
	{  2840,   "HARN/NE.RI"  },
	{  3446,   "HARN/NE.RIF"  },
	{  3653,   "NSRS07.RI"  },
	{  3654,   "NSRS07.RIF"  },
	{  6567,   "NSRS11.RI"  },
	{  6568,   "NSRS11.RIF"  },
	{ 32030,   "RI"  },
	{ 32130,   "RI83"  },
	{  3438,   "RI83F"  },
	{ 31973,   "SIRGAS2000.UTM-19N"  },
	{  2960,   "CSRS.UTM-19N"  },
	{  3179,   "GR96.UTM-19N"  },
	{ 31979,   "SIRGAS2000.UTM-19S"  },
	{  5361,   "SIRGAS-Chile.UTM-19S"  },
	{ 21898,   "BOGOTA.Colombia-EC"  },
	{  3117,   "MAGNA.Columbia-EastCtrl"  },
	{  3178,   "GR96.UTM-18N"  },
	{  4190,   "PGA98.LL"  },
	{  2831,   "HARN/NY.NY-LI"  },
	{  2908,   "HARN/NY.NY-LIF"  },
	{  3627,   "NSRS07.NY-LI"  },
	{  3628,   "NSRS07.NY-LIF"  },
	{  6538,   "NSRS11.NY-LI"  },
	{  6539,   "NSRS11.NY-LIF"  },
	{  4456,   "NY-LI"  },
	{ 32118,   "NY83-LI"  },
	{  2263,   "NY83-LIF"  },
	{  2950,   "CSRS.MTM-8"  },
	{  4747,   "Greenland1996.LL"  },
	{ 21897,   "BOGOTA.Colombia-Bogota"  },
	{  3116,   "MAGNA.Columbia-Bogota"  },
	{  4221,   "Campo.LL"  },
	{  4254,   "HitoXVIII63b.LL"  },
	{ 31972,   "SIRGAS2000.UTM-18N"  },
	{  2959,   "CSRS.UTM-18N"  },
	{  2951,   "CSRS.MTM-9"  },
	{ 31978,   "SIRGAS2000.UTM-18S"  },
	{  4242,   "Jamaica1969.LL"  },
	{ 24200,   "Jamaica1969.NtlGrid"  },
	{  3115,   "MAGNA.Columbia-West"  },
	{  3725,   "NSRS07.UTM-18"  },
	{  6347,   "NSRS11.UTM-18"  },
	{  4686,   "MarcoGNR.LL"  },
	{  2952,   "CSRS.MTM-10"  },
	{  4726,   "LittleCayman61.LL"  },
	{  4723,   "GrandCayman59.LL"  },
	{ 31971,   "SIRGAS2000.UTM-17N"  },
	{  2958,   "CSRS.UTM-17N"  },
	{ 31977,   "SIRGAS2000.UTM-17S"  },
	{ 32167,   "NAD83.BLM-17NF"  },
	{ 32667,   "WGS84.BLM-17NF"  },
	{  3088,   "KY83"  },
	{  3089,   "KY83F"  },
	{  6202,   "MI27e-S"  },
	{  6201,   "MI27e-C"  },
	{  3087,   "HARN/FL.FloridaGDL/Albr"  },
	{  3086,   "NAD83.FloridaGDL/Albers"  },
	{  3513,   "NSRS07.FloridaGDL/Albr"  },
	{  6439,   "NSRS11.FloridaGDL/Albr"  },
	{  7614,   "WISCRS2011-KewauneeF"  },
	{  7555,   "WISCRS2011-KewauneeM"  },
	{  7614,   "WISCRS2011-ManitowocF"  },
	{  7555,   "WISCRS2011-ManitowocM"  },
	{  7613,   "WISCRS2011-MilwaukeeF"  },
	{  7554,   "WISCRS2011-MilwaukeeM"  },
	{  7613,   "WISCRS2011-OzaukeeF"  },
	{  7554,   "WISCRS2011-OzaukeeM"  },
	{  7614,   "WISCRS2011-SheboyganF"  },
	{  7555,   "WISCRS2011-SheboyganM"  },
	{  7613,   "WISCRS2011-KenoshaF"  },
	{  7554,   "WISCRS2011-KenoshaM"  },
	{  7613,   "WISCRS2011-RacineF"  },
	{  7554,   "WISCRS2011-RacineM"  },
	{  7594,   "WISCRS2011-CalumetF"  },
	{  7535,   "WISCRS2011-CalumetM"  },
	{  7594,   "WISCRS2011-OutagamieF"  },
	{  7535,   "WISCRS2011-OutagamieM"  },
	{  7594,   "WISCRS2011-FondDuLacF"  },
	{  7535,   "WISCRS2011-FondDuLacM"  },
	{  7594,   "WISCRS2011-WinnebagoF"  },
	{  7535,   "WISCRS2011-WinnebagoM"  },
	{  7600,   "WISCRS2011-JeffersonF"  },
	{  7541,   "WISCRS2011-JeffersonM"  },
	{  7600,   "WISCRS2011-DodgeF"  },
	{  7541,   "WISCRS2011-DodgeM"  },
	{  7609,   "WISCRS2011-GreenLakeF"  },
	{  7550,   "WISCRS2011-GreenLakeM"  },
	{  3090,   "HARN/KY.KY"  },
	{  3091,   "HARN/KY.KYF"  },
	{  3546,   "NSRS07.KY"  },
	{  3547,   "NSRS07.KYF"  },
	{  6472,   "NSRS11.KY"  },
	{  6473,   "NSRS11.KYF"  },
	{  7609,   "WISCRS2011-MarquetteF"  },
	{  7550,   "WISCRS2011-MarquetteM"  },
	{  7608,   "WISCRS2011-GreenF"  },
	{  7549,   "WISCRS2011-GreenM"  },
	{ 32166,   "NAD83.BLM-16NF"  },
	{ 32666,   "WGS84.BLM-16NF"  },
	{ 31970,   "SIRGAS2000.UTM-16N"  },
	{  3160,   "CSRS.UTM-16N"  },
	{  7587,   "WISCRS2011-AdamsF"  },
	{  7528,   "WISCRS2011-AdamsM"  },
	{  7587,   "WISCRS2011-JuneauF"  },
	{  7528,   "WISCRS2011-JuneauM"  },
	{  3079,   "HARN/MI.MichiganGeoRef"  },
	{  3591,   "NSRS07.MichiganGeoRef"  },
	{  6497,   "NSRS11.MichiganGeoRef"  },
	{  7608,   "WISCRS2011-LafayetteF"  },
	{  7549,   "WISCRS2011-LafayetteM"  },
	{  3723,   "NSRS07.UTM-16"  },
	{  6345,   "NSRS11.UTM-16"  },
	{  7059,   "NSRS2011.IaRCS-3"  },
	{  7624,   "WISCRS2011-PepinF"  },
	{  7565,   "WISCRS2011-PepinM"  },
	{  7624,   "WISCRS2011-PierceF"  },
	{  7565,   "WISCRS2011-PierceM"  },
	{  3071,   "HARN/WI.WisconsinTM"  },
	{  3701,   "NSRS07.WisconsinTM"  },
	{  6610,   "NSRS11.WisconsinTM"  },
	{ 26782,   "LA-S-MOD"  },
	{  3162,   "CSRS.Ontario/MNR"  },
	{  3161,   "NAD83.Ontario/MNR"  },
	{ 32165,   "NAD83.BLM-15NF"  },
	{ 32665,   "WGS84.BLM-15NF"  },
	{ 31969,   "SIRGAS2000.UTM-15N"  },
	{  3159,   "CSRS.UTM-15N"  },
	{  3722,   "NSRS07.UTM-15"  },
	{  6344,   "NSRS11.UTM-15"  },
	{ 32164,   "NAD83.BLM-14N"  },
	{ 32664,   "WGS84.BLM-14NF"  },
	{ 31968,   "SIRGAS2000.UTM-14N"  },
	{  3158,   "CSRS.UTM-14N"  },
	{  6922,   "NAD83.KansasLCC"  },
	{  6923,   "NAD83.KansasLCC-F"  },
	{  3085,   "HARN/TX.Texas/Albers"  },
	{  3084,   "HARN/TX.Texas/Lambert"  },
	{  3083,   "NAD83.Texas/EqArea"  },
	{  3082,   "NAD83.Texas/Lambert"  },
	{  3665,   "NSRS07.Texas/Albers"  },
	{  3666,   "NSRS07.Texas/Lambert"  },
	{  6579,   "NSRS11.Texas/Albers"  },
	{  6580,   "NSRS11.Texas/Lambert"  },
	{  2957,   "CSRS.UTM-13N"  },
	{  2878,   "HARN/CO.CO-SF-MOD"  },
	{  3506,   "NSRS07.CO-SF"  },
	{  6432,   "NSRS11.CO-SF"  },
	{ 31966,   "SIRGAS2000.UTM-12N"  },
	{  2956,   "CSRS.UTM-12N"  },
	{  4674,   "SRG-SA/2000.LL"  },
	{  2955,   "CSRS.UTM-11N"  },
	{  7131,   "NSRS11.SFO-CS13"  },
	{  7132,   "NSRS11.SFO-CS13F"  },
	{  3717,   "NSRS07.UTM-10"  },
	{  6339,   "NSRS11.UTM-10"  },
	{  3157,   "CSRS.UTM-10N"  },
	{  3156,   "CSRS.UTM-9N"  },
	{  3155,   "CSRS.UTM-8N"  },
	{  3299,   "RGP-Francaise/a.UTM-8S"  },
	{  4688,   "FatuIva/72a.LL"  },
	{  3153,   "CSRS.BC/Albers"  },
	{  3005,   "NAD83.BC/Abers"  },
	{  4689,   "IGN63/Hiva Oa/a.LL"  },
	{  3154,   "CSRS.UTM-7N"  },
	{  3298,   "RGP-Francaise/a.UTM-7S"  },
	{  4691,   "Moorea87a.LL"  },
	{  3297,   "RGP-Francaise/a.UTM-6S"  },
	{  4629,   "Tahaa54.LL"  },
	{  2977,   "Tahaa54.UTM-5S"  },
	{  4692,   "Maupiti1983.LL"  },
	{  3296,   "RGP-Francaise/a.UTM-5S"  },
	{  4687,   "RGP-Francaise/a.LL"  },
	{  4707,   "Tern61.LL"  },
	{  3102,   "Samoa1962.Samoa/Lambert"  },
	{ 26739,   "AK-9"  },
	{ 26939,   "AK83-9"  },
	{  4716,   "PhoenixIs66.LL"  },
	{  4673,   "Chatham1979a.LL"  },
	{  4721,   "Fiji56.LL"  },
	{     0,   ""           }
};

bool csFixUsefulRange (const wchar_t* csDictDir)
{
	bool ok (true);

	const struct TcsRngFixTbl_* tblPtr;

	long32_t lngFormat = (cs_ATOF_MINSEC | cs_ATOF_MINSEC0 | cs_ATOF_DIRCHR | cs_ATOF_XEAST | 1L);
	long32_t latFormat = (cs_ATOF_MINSEC | cs_ATOF_MINSEC0 | cs_ATOF_DIRCHR | 1L);

	double epsgRngLngW (0.0);
	double epsgRngLatS (0.0);
	double epsgRngLngE (0.0);
	double epsgRngLatN (0.0);

	char minLng [64];
	char minLat [64];
	char maxLng [64];
	char maxLat [64];

	char crsAscFilePath [MAXPATH];

	// Load the EPSG database
	const TcsEpsgDataSetV6* epsgPtr = GetEpsgObjectPtr ();
	ok = (epsgPtr != 0);
	if (!ok)
	{
		return ok;
	}

	// Load the coordsys.asc file
	wcstombs (crsAscFilePath,csDictDir,sizeof (crsAscFilePath));
	CS_stncat (crsAscFilePath,"\\coordsys.asc",sizeof (crsAscFilePath));
	TcsDefFile coordsysAsc (dictTypCoordsys,crsAscFilePath);
	ok = (coordsysAsc.GetDefinitionCount () != 0);
	if (!ok)
	{
		return ok;
	}

	// Loop once for each system to be "gixed".
	for (tblPtr = KcsRngFixTbl;ok && tblPtr->epsgCode != 0L;tblPtr += 1)
	{
		bool epsgOk (true);
		bool crack180 (false);

		TcsEpsgCode crsEpsgCode;
		TcsEpsgCode epsgAreaCode;

		TcsAscDefinition* ascCrsPtr;
		TcsDefLine* defLinePtr;

		char comment [128];

		// Get the CRS definition in ASCII form
		ascCrsPtr = coordsysAsc.GetDefinition (tblPtr->csMapKeyNm);
		ok = (ascCrsPtr != NULL);
		if (!ok)
		{
			return ok;
		}

		// Extract the useful range from the EPSG database.
		crsEpsgCode = tblPtr->epsgCode;
		epsgOk = epsgPtr->GetFieldByCode (epsgAreaCode,epsgTblReferenceSystem,epsgFldAreaOfUseCode,crsEpsgCode);
		epsgOk &= !(epsgAreaCode == KcsNmInvNumber || epsgAreaCode == KcsNmMapNoNumber);
		if (epsgOk)
		{
			// Get the range values.
			epsgOk  = epsgPtr->GetFieldByCode (epsgRngLngW,epsgTblArea,epsgFldAreaWestBoundLng,epsgAreaCode);
			epsgOk &= epsgPtr->GetFieldByCode (epsgRngLngE,epsgTblArea,epsgFldAreaEastBoundLng,epsgAreaCode);
			epsgOk &= epsgPtr->GetFieldByCode (epsgRngLatS,epsgTblArea,epsgFldAreaSouthBoundLat,epsgAreaCode);
			epsgOk &= epsgPtr->GetFieldByCode (epsgRngLatN,epsgTblArea,epsgFldAreaNorthBoundLat,epsgAreaCode);

			// There are occasional "non-defs" in the EPSG database.
			epsgOk &= (epsgRngLngW != 0.0) || (epsgRngLatS != 0.0) ||
					  (epsgRngLngE != 0.0) || (epsgRngLatN != 0.0);
		}
		if (epsgOk)
		{
			crack180 = true;
			// Adjust for the +/- 180 degree crack.
			if (epsgRngLngW > epsgRngLngE)
			{
				// This range crosses the crack.  Decide which element to adjust.
				if (epsgRngLngE < 0.0)
				{
					epsgRngLngW -= 360.00;
				}
				else
				{
					epsgRngLngE += 360.00;
				}
			}

			// Now expand this range by a suitable amount; how about 5% but never more than
			// 2 degrees, never less than 12 minutes.
			double delta = (epsgRngLngE - epsgRngLngW) * 0.05;			// should always be positive here.
			if (delta > 1.0) delta = 1.0;
			if (delta < .25) delta = 0.2;
			epsgRngLngW -= delta;
			epsgRngLngE += delta;

			delta = (epsgRngLatN - epsgRngLatS) * 0.05;			// should always be positive here.
			if (delta > 1.0) delta = 1.0;
			if (delta < .25) delta = 0.25;
			epsgRngLatS -= delta;
			epsgRngLatN += delta;

			// Round the extrema to a number of minutes.
			long minutes;
			minutes = static_cast<long>(epsgRngLngW * 60.0 - 0.5);
			epsgRngLngW = static_cast<double>(minutes) / 60.0;
			minutes = static_cast<long>(epsgRngLatS * 60.0 - 0.5);
			epsgRngLatS = static_cast<double>(minutes) / 60.0;

			minutes = static_cast<long>(epsgRngLngE * 60.0 + 0.5);
			epsgRngLngE = static_cast<double>(minutes) / 60.0;
			minutes = static_cast<long>(epsgRngLatN * 60.0 + 0.5);
			epsgRngLatN = static_cast<double>(minutes) / 60.0;

			// Make sure that Longitude remains reasonable.
			if (!crack180)
			{
				if (epsgRngLngW < -180.0) epsgRngLngW = -180.0;
				if (epsgRngLngE >  180.0) epsgRngLngE =  180.0;
			}

			// Make sure that Latitude remains reasonable.
			if (epsgRngLatS < -90.0) epsgRngLatS = -90.0;
			if (epsgRngLatN >  90.0) epsgRngLatN =  90.0;

			// Format the useful range to the desired format.
			CS_ftoa (minLng,sizeof (minLng),epsgRngLngW,lngFormat);
			CS_ftoa (minLat,sizeof (minLat),epsgRngLatS,latFormat);

			CS_ftoa (maxLng,sizeof (maxLng),epsgRngLngE,lngFormat);
			CS_ftoa (maxLat,sizeof (maxLat),epsgRngLatN,latFormat);

			// Prepare a comment
			sprintf (comment,"# EPSG 9.1 Area Code %d + ~5%%",epsgAreaCode);
		}
		ok = epsgOk;

		// Set the expanded useful range into the CRS ASC definition.
		if (ok)
		{
			defLinePtr = ascCrsPtr->GetLine ("MIN_LNG:");
			if (defLinePtr == NULL)
			{
				// There is no existing definition, we need to add one.
				TcsDefLine newMinLng (dictTypCoordsys,"MIN_LNG:",minLng,comment);
				ascCrsPtr->InsertAfter ("Y_OFF:",newMinLng);
				TcsDefLine newMinLat (dictTypCoordsys,"MIN_LAT:",minLat,NULL);
				ascCrsPtr->InsertAfter ("MIN_LNG:",newMinLat);
				TcsDefLine newMaxLng (dictTypCoordsys,"MAX_LNG:",maxLng,NULL);
				ascCrsPtr->InsertAfter ("MIN_LAT:",newMaxLng);
				TcsDefLine newMaxLat (dictTypCoordsys,"MAX_LAT:",maxLat,NULL);
				ascCrsPtr->InsertAfter ("MAX_LNG:",newMaxLat);
			}
			else
			{
				// We can simply replace the values which are already there.
				defLinePtr->SetComment (comment);
				ascCrsPtr->SetValue ("MIN_LNG:",minLng);
				ascCrsPtr->SetValue ("MIN_LAT:",minLat);
				ascCrsPtr->SetValue ("MAX_LNG:",maxLng);
				ascCrsPtr->SetValue ("MAX_LAT:",maxLat);
			}
		}
	}

	// Close the EPSG database.
	ReleaseEpsgObjectPtr ();

	// Write the modified coordsys.asc file out.
	if (ok)
	{
		ok = coordsysAsc.WriteToFile (crsAscFilePath);
	}
	return ok;
}

