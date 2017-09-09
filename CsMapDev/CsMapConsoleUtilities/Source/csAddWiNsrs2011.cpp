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

/* Hopefully, the following table contains all the information we need to:

	1> Sort the table to EPSG code order, thus getting the identical
	   definitions together and we can programatically deal with that
	   issue.
	2> Open the Wisconsin coordsys.CSD file, we'll get our definitions from
	   that file using the 3 character code in the table.
	3> Open the ESRI .csv file so we have access to ESRI names for things.
	3> Loop through the table and using the  WI coordsys file write new
	   definitions to a temporary coordsys.asc file.  We'll be skipping the
	   International Foot objects in the table.  But we will create a
	   new definition for each county.
	4> Loop through the table again and create NameMapper entries for
	   for each EPSG code.  Certain EPSG codes will have several
	   CS_MAP aliaes, writing to a temporary file.
	5> Loop though official coordsys.asc and update the useful range on
	   all the existing coordsys.asc coordinate systems in the table.
	6> Loop through the table againa and write to a temporary file
	   category file entries for all new systems.

	After evaluation, insert the new definitions into the offical
	coordsys.asc and category.asc files.  After evaluation, insert
	the new NameMapper data into the official NameMapper file, and
	then resort the NameMapper file using the existing utility.

	This is a one time program, developed to do all this manipulation
	programmatically so as to reduce the probabilty of typographical errors
	and enable re-running the whole thing after a fix/tweak or two.
	
	SO, performance and eloquent code is not a consideration.  We need to
	get the job done as quickly as possible.  The code is stashed in the
	repository only to enable it to serve as a base for future efforts
	of a similar nature.
*/

/* The table. For each counbty system, it associates a NSRS 2001 EPSG
   code with:
	a> the name of the existing HARN CS-MAP system
	b> the county name in a form usable in system names
	b> the 3 letter code used by WI-DOT in their user defined CSD file.
	c> the actual CS-MAP name for each "HARN" definition.
	d> The county name usable in a description.
	e> the system unit.
	
	There are no EPSG codes for the International Foot version of things,
	as we will not be producing 2011 versions of those definitions.
*/

struct prj32WiFix
{
	long epsgCode;
	char csCountyNm [32];
	char wiCountyCode [4];
	char csHarnName [24];
	char countyName [64];
	char sysUnit;
	char newKeyNm [24];
	char newDescName [64];
} Kprj32WiFix [] =
{
	{ 7587, "Adams",      "adm","HARN/WI.AdamsWI-F",        "Adams County",      'F'  },
	{    0, "Adams",      "adm","HARN/WI.AdamsWI-IF",       "Adams County",      'I'  },
	{ 7528, "Adams",      "adm","HARN/WI.AdamsWI-M",        "Adams County",      'M'  },
	{ 7588, "Ashland",    "ash","HARN/WI.AshlandWI-F",      "Ashland County",    'F'  },
	{    0, "Ashland",    "ash","HARN/WI.AshlandWI-IF",     "Ashland County",    'I'  },
	{ 7529, "Ashland",    "ash","HARN/WI.AshlandWI-M",      "Ashland County",    'M'  },
	{ 7589, "Barron",     "bar","HARN/WI.BarronWI-F",       "Barron County",     'F'  },
	{    0, "Barron",     "bar","HARN/WI.BarronWI-IF",      "Barron County",     'I'  },
	{ 7530, "Barron",     "bar","HARN/WI.BarronWI-M",       "Barron County",     'M'  },
	{ 7590, "Bayfield",   "bay","HARN/WI.BayfieldWI-F",     "Bayfield County",   'F'  },
	{    0, "Bayfield",   "bay","HARN/WI.BayfieldWI-IF",    "Bayfield County",   'I'  },
	{ 7531, "Bayfield",   "bay","HARN/WI.BayfieldWI-M",     "Bayfield County",   'M'  },
	{ 7591, "Brown",      "brn","HARN/WI.BrownWI-F",        "Brown County",      'F'  },
	{    0, "Brown",      "brn","HARN/WI.BrownWI-IF",       "Brown County",      'I'  },
	{ 7532, "Brown",      "brn","HARN/WI.BrownWI-M",        "Brown County",      'M'  },
	{ 7592, "Buffalo",    "buf","HARN/WI.BuffaloWI-F",      "Buffalo County",    'F'  },
	{    0, "Buffalo",    "buf","HARN/WI.BuffaloWI-IF",     "Buffalo County",    'I'  },
	{ 7533, "Buffalo",    "buf","HARN/WI.BuffaloWI-M",      "Buffalo County",    'M'  },
	{ 7593, "Burnett",    "bur","HARN/WI.BurnettWI-F",      "Burnett County",    'F'  },
	{    0, "Burnett",    "bur","HARN/WI.BurnettWI-IF",     "Burnett County",    'I'  },
	{ 7534, "Burnett",    "bur","HARN/WI.BurnettWI-M",      "Burnett County",    'M'  },
	{ 7594, "Calumet",    "cal","HARN/WI.CalumetWI-F",      "Calumet County",    'F'  },
	{    0, "Calumet",    "cal","HARN/WI.CalumetWI-IF",     "Calumet County",    'I'  },
	{ 7535, "Calumet",    "cal","HARN/WI.CalumetWI-M",      "Calumet County",    'M'  },
	{ 7595, "Chippewa",   "chp","HARN/WI.ChippewaWI-F",     "Chippewa County",   'F'  },
	{    0, "Chippewa",   "chp","HARN/WI.ChippewaWI-IF",    "Chippewa County",   'I'  },
	{ 7536, "Chippewa",   "chp","HARN/WI.ChippewaWI-M",     "Chippewa County",   'M'  },
	{ 7596, "Clark",      "clr","HARN/WI.ClarkWI-F",        "Clark County",      'F'  },
	{    0, "Clark",      "clr","HARN/WI.ClarkWI-IF",       "Clark County",      'I'  },
	{ 7537, "Clark",      "clr","HARN/WI.ClarkWI-M",        "Clark County",      'M'  },
	{ 7597, "Columbia",   "col","HARN/WI.ColumbiaWI-F",     "Columbia County",   'F'  },
	{    0, "Columbia",   "col","HARN/WI.ColumbiaWI-IF",    "Columbia County",   'I'  },
	{ 7538, "Columbia",   "col","HARN/WI.ColumbiaWI-M",     "Columbia County",   'M'  },
	{ 7598, "Crawford",   "crw","HARN/WI.CrawfordWI-F/a",   "Crawford County",   'F'  },
	{    0, "Crawford",   "crw","HARN/WI.CrawfordWI-IF/a",  "Crawford County",   'I'  },
	{ 7539, "Crawford",   "crw","HARN/WI.CrawfordWI-M/a",   "Crawford County",   'M'  },
	{ 7599, "Dane",       "dan","HARN/WI.DaneWI-F",         "Dane County",       'F'  },
	{    0, "Dane",       "dan","HARN/WI.DaneWI-IF",        "Dane County",       'I'  },
	{ 7540, "Dane",       "dan","HARN/WI.DaneWI-M",         "Dane County",       'M'  },
	{ 7600, "Dodge",      "ddg","HARN/WI.DodgeWI-F",        "Dodge County",      'F'  },
	{    0, "Dodge",      "ddg","HARN/WI.DodgeWI-IF",       "Dodge County",      'I'  },
	{ 7541, "Dodge",      "ddg","HARN/WI.DodgeWI-M",        "Dodge County",      'M'  },
	{ 7601, "Door",       "dor","HARN/WI.DoorWI-F",         "Door County",       'F'  },
	{    0, "Door",       "dor","HARN/WI.DoorWI-IF",        "Door County",       'I'  },
	{ 7542, "Door",       "dor","HARN/WI.DoorWI-M",         "Door County",       'M'  },
	{ 7602, "Douglas",    "dug","HARN/WI.DouglasWI-F",      "Douglas County",    'F'  },
	{    0, "Douglas",    "dug","HARN/WI.DouglasWI-IF",     "Douglas County",    'I'  },
	{ 7543, "Douglas",    "dug","HARN/WI.DouglasWI-M",      "Douglas County",    'M'  },
	{ 7603, "Dunn",       "dun","HARN/WI.DunnWI-F",         "Dunn County",       'F'  },
	{    0, "Dunn",       "dun","HARN/WI.DunnWI-IF",        "Dunn County",       'I'  },
	{ 7544, "Dunn",       "dun","HARN/WI.DunnWI-M",         "Dunn County",       'M'  },
	{ 7604, "EauClaire",  "ecl","HARN/WI.EauClaireWI-F",    "Eau Claire County", 'F'  },
	{    0, "EauClaire",  "ecl","HARN/WI.EauClaireWI-IF",   "Eau Claire County", 'I'  },
	{ 7545, "EauClaire",  "ecl","HARN/WI.EauClaireWI-M",    "Eau Claire County", 'M'  },
	{ 7605, "Florence",   "flo","HARN/WI.FlorenceWI-F",     "Florence County",   'F'  },
	{    0, "Florence",   "flo","HARN/WI.FlorenceWI-IF",    "Florence County",   'I'  },
	{ 7546, "Florence",   "flo","HARN/WI.FlorenceWI-M",     "Florence County",   'M'  },
	{ 7594, "FondDuLac",  "fdl","HARN/WI.FondDuLacWI-F",    "Fond du Lac County",'F'  },
	{    0, "FondDuLac",  "fdl","HARN/WI.FondDuLacWI-IF",   "Fond du Lac County",'I'  },
	{ 7535, "FondDuLac",  "fdl","HARN/WI.FondDuLacWI-M",    "Fond du Lac County",'M'  },
	{ 7606, "Forest",     "for","HARN/WI.ForestWI-F",       "Forest County",     'F'  },
	{    0, "Forest",     "for","HARN/WI.ForestWI-IF",      "Forest County",     'I'  },
	{ 7547, "Forest",     "for","HARN/WI.ForestWI-M",       "Forest County",     'M'  },
	{ 7607, "Grant",      "gra","HARN/WI.GrantWI-F",        "Grant County",      'F'  },
	{    0, "Grant",      "gra","HARN/WI.GrantWI-IF",       "Grant County",      'I'  },
	{ 7548, "Grant",      "gra","HARN/WI.GrantWI-M",        "Grant County",      'M'  },
	{ 7609, "GreenLake",  "grl","HARN/WI.GreenLakeWI-F",    "Green Lake County", 'F'  },
	{    0, "GreenLake",  "grl","HARN/WI.GreenLakeWI-IF",   "Green Lake County", 'I'  },
	{ 7550, "GreenLake",  "grl","HARN/WI.GreenLakeWI-M",    "Green Lake County", 'M'  },
	{ 7608, "Green",      "grn","HARN/WI.GreenWI-F",        "Green County",      'F'  },
	{    0, "Green",      "grn","HARN/WI.GreenWI-IF",       "Green County",      'I'  },
	{ 7549, "Green",      "grn","HARN/WI.GreenWI-M",        "Green County",      'M'  },
	{ 7610, "Iowa",       "iow","HARN/WI.IowaWI-F",         "Iowa County",       'F'  },
	{    0, "Iowa",       "iow","HARN/WI.IowaWI-IF",        "Iowa County",       'I'  },
	{ 7551, "Iowa",       "iow","HARN/WI.IowaWI-M",         "Iowa County",       'M'  },
	{ 7611, "Iron",       "irn","HARN/WI.IronWI-F",         "Iron County",       'F'  },
	{    0, "Iron",       "irn","HARN/WI.IronWI-IF",        "Iron County",       'I'  },
	{ 7552, "Iron",       "irn","HARN/WI.IronWI-M",         "Iron County",       'M'  },
	{ 7612, "Jackson",    "jak","HARN/WI.JacksonWI-F",      "Jackson County",    'F'  },
	{    0, "Jackson",    "jak","HARN/WI.JacksonWI-IF",     "Jackson County",    'I'  },
	{ 7553, "Jackson",    "jak","HARN/WI.JacksonWI-M",      "Jackson County",    'M'  },
	{ 7600, "Jefferson",  "jef","HARN/WI.JeffersonWI-F",    "Jefferson County",  'F'  },
	{    0, "Jefferson",  "jef","HARN/WI.JeffersonWI-IF",   "Jefferson County",  'I'  },
	{ 7541, "Jefferson",  "jef","HARN/WI.JeffersonWI-M",    "Jefferson County",  'M'  },
	{ 7587, "Juneau",     "jun","HARN/WI.JuneauWI-F",       "Juneau County",     'F'  },
	{    0, "Juneau",     "jun","HARN/WI.JuneauWI-IF",      "Juneau County",     'I'  },
	{ 7528, "Juneau",     "jun","HARN/WI.JuneauWI-M",       "Juneau County",     'M'  },
	{ 7613, "Kenosha",    "ken","HARN/WI.KenoshaWI-F",      "Kenosha County",    'F'  },
	{    0, "Kenosha",    "ken","HARN/WI.KenoshaWI-IF",     "Kenosha County",    'I'  },
	{ 7554, "Kenosha",    "ken","HARN/WI.KenoshaWI-M",      "Kenosha County",    'M'  },
	{ 7614, "Kewaunee",   "kew","HARN/WI.KewauneeWI-F",     "Kewaunee County",   'F'  },
	{    0, "Kewaunee",   "kew","HARN/WI.KewauneeWI-IF",    "Kewaunee County",   'I'  },
	{ 7555, "Kewaunee",   "kew","HARN/WI.KewauneeWI-M",     "Kewaunee County",   'M'  },
	{ 7616, "Langlade",   "lan","HARN/WI.LangladeWI-F",     "Langlade County",   'F'  },
	{    0, "Langlade",   "lan","HARN/WI.LangladeWI-IF",    "Langlade County",   'I'  },
	{ 7557, "Langlade",   "lan","HARN/WI.LangladeWI-M",     "Langlade County",   'M'  },
	{ 7615, "LaCrosse",   "lac","HARN/WI.LaCrosseWI-F",     "La Crosse County",  'F'  },
	{    0, "LaCrosse",   "lac","HARN/WI.LaCrosseWI-IF",    "La Crosse County",  'I'  },
	{ 7556, "LaCrosse",   "lac","HARN/WI.LaCrosseWI-M",     "La Crosse County",  'M'  },
	{ 7608, "Lafayette",  "laf","HARN/WI.LafayetteWI-F",    "Lafayette County",  'F'  },
	{    0, "Lafayette",  "laf","HARN/WI.LafayetteWI-IF",   "Lafayette County",  'I'  },
	{ 7549, "Lafayette",  "laf","HARN/WI.LafayetteWI-M",    "Lafayette County",  'M'  },
	{ 7617, "Lincoln",    "lin","HARN/WI.LincolnWI-F",      "Lincoln County",    'F'  },
	{    0, "Lincoln",    "lin","HARN/WI.LincolnWI-IF",     "Lincoln County",    'I'  },
	{ 7558, "Lincoln",    "lin","HARN/WI.LincolnWI-M",      "Lincoln County",    'M'  },
	{ 7613, "Milwaukee",  "mil","HARN/WI.MilwaukeeWI-F",    "Milwaukee County",  'F'  },
	{    0, "Milwaukee",  "mil","HARN/WI.MilwaukeeWI-IF",   "Milwaukee County",  'I'  },
	{ 7554, "Milwaukee",  "mil","HARN/WI.MilwaukeeWI-M",    "Milwaukee County",  'M'  },
	{ 7614, "Manitowoc",  "man","HARN/WI.ManitowocWI-F",    "Manitowoc County",  'F'  },
	{    0, "Manitowoc",  "man","HARN/WI.ManitowocWI-IF",   "Manitowoc County",  'I'  },
	{ 7555, "Manitowoc",  "man","HARN/WI.ManitowocWI-M",    "Manitowoc County",  'M'  },
	{ 7618, "Marathon",   "mrt","HARN/WI.MarathonWI-F",     "Marathon County",   'F'  },
	{    0, "Marathon",   "mrt","HARN/WI.MarathonWI-IF",    "Marathon County",   'I'  },
	{ 7559, "Marathon",   "mrt","HARN/WI.MarathonWI-M",     "Marathon County",   'M'  },
	{ 7619, "Marinette",  "mrn","HARN/WI.MarinetteWI-F",    "Marinette County",  'F'  },
	{    0, "Marinette",  "mrn","HARN/WI.MarinetteWI-IF",   "Marinette County",  'I'  },
	{ 7560, "Marinette",  "mrn","HARN/WI.MarinetteWI-M",    "Marinette County",  'M'  },
	{ 7609, "Marquette",  "maq","HARN/WI.MarquetteWI-F",    "Marquette County",  'F'  },
	{    0, "Marquette",  "maq","HARN/WI.MarquetteWI-IF",   "Marquette County",  'I'  },
	{ 7550, "Marquette",  "maq","HARN/WI.MarquetteWI-M",    "Marquette County",  'M'  },
	{ 7620, "Menominee",  "men","HARN/WI.MenomineeWI-F",    "Menominee County",  'F'  },
	{    0, "Menominee",  "men","HARN/WI.MenomineeWI-IF",   "Menominee County",  'I'  },
	{ 7561, "Menominee",  "men","HARN/WI.MenomineeWI-M",    "Menominee County",  'M'  },
	{ 7621, "Monroe",     "mon","HARN/WI.MonroeWI-F",       "Monroe County",     'F'  },
	{    0, "Monroe",     "mon","HARN/WI.MonroeWI-IF",      "Monroe County",     'I'  },
	{ 7562, "Monroe",     "mon","HARN/WI.MonroeWI-M",       "Monroe County",     'M'  },
	{ 7622, "Oconto",     "ocn","HARN/WI.OcontoWI-F",       "Oconto County",     'F'  },
	{    0, "Oconto",     "ocn","HARN/WI.OcontoWI-IF",      "Oconto County",     'I'  },
	{ 7563, "Oconto",     "ocn","HARN/WI.OcontoWI-M",       "Oconto County",     'M'  },
	{ 7623, "Oneida",     "ond","HARN/WI.OneidaWI-F",       "Oneida County",     'F'  },
	{    0, "Oneida",     "ond","HARN/WI.OneidaWI-IF",      "Oneida County",     'I'  },
	{ 7564, "Oneida",     "ond","HARN/WI.OneidaWI-M",       "Oneida County",     'M'  },
	{ 7594, "Outagamie",  "out","HARN/WI.OutagamieWI-F",    "Outagamie County",  'F'  },
	{    0, "Outagamie",  "out","HARN/WI.OutagamieWI-IF",   "Outagamie County",  'I'  },
	{ 7535, "Outagamie",  "out","HARN/WI.OutagamieWI-M",    "Outagamie County",  'M'  },
	{ 7613, "Ozaukee",    "oza","HARN/WI.OzaukeeWI-F",      "Ozaukee County",    'F'  },
	{    0, "Ozaukee",    "oza","HARN/WI.OzaukeeWI-IF",     "Ozaukee County",    'I'  },
	{ 7554, "Ozaukee",    "oza","HARN/WI.OzaukeeWI-M",      "Ozaukee County",    'M'  },
	{ 7624, "Pepin",      "pep","HARN/WI.PepinWI-F",        "Pepin County",      'F'  },
	{    0, "Pepin",      "pep","HARN/WI.PepinWI-IF",       "Pepin County",      'I'  },
	{ 7565, "Pepin",      "pep","HARN/WI.PepinWI-M",        "Pepin County",      'M'  },
	{ 7624, "Pierce",     "pir","HARN/WI.PierceWI-F",       "Pierce County",     'F'  },
	{    0, "Pierce",     "pir","HARN/WI.PierceWI-IF",      "Pierce County",     'I'  },
	{ 7565, "Pierce",     "pir","HARN/WI.PierceWI-M",       "Pierce County",     'M'  },
	{ 7625, "Polk",       "pok","HARN/WI.PolkWI-F",         "Polk County",       'F'  },
	{    0, "Polk",       "pok","HARN/WI.PolkWI-IF",        "Polk County",       'I'  },
	{ 7566, "Polk",       "pok","HARN/WI.PolkWI-M",         "Polk County",       'M'  },
	{ 7626, "Portage",    "por","HARN/WI.PortageWI-F",      "Portage County",    'F'  },
	{    0, "Portage",    "por","HARN/WI.PortageWI-IF",     "Portage County",    'I'  },
	{ 7567, "Portage",    "por","HARN/WI.PortageWI-M",      "Portage County",    'M'  },
	{ 7627, "Price",      "pri","HARN/WI.PriceWI-F",        "Price County",      'F'  },
	{    0, "Price",      "pri","HARN/WI.PriceWI-IF",       "Price County",      'I'  },
	{ 7568, "Price",      "pri","HARN/WI.PriceWI-M",        "Price County",      'M'  },
	{ 7613, "Racine",     "rac","HARN/WI.RacineWI-F",       "Racine County",     'F'  },
	{    0, "Racine",     "rac","HARN/WI.RacineWI-IF",      "Racine County",     'I'  },
	{ 7554, "Racine",     "rac","HARN/WI.RacineWI-M",       "Racine County",     'M'  },
	{ 7628, "Richland",   "ric","HARN/WI.RichlandWI-F",     "Richland County",   'F'  },
	{    0, "Richland",   "ric","HARN/WI.RichlandWI-IF",    "Richland County",   'I'  },
	{ 7569, "Richland",   "ric","HARN/WI.RichlandWI-M",     "Richland County",   'M'  },
	{ 7629, "Rock",       "rok","HARN/WI.RockWI-F",         "Rock County",       'F'  },
	{    0, "Rock",       "rok","HARN/WI.RockWI-IF",        "Rock County",       'I'  },
	{ 7570, "Rock",       "rok","HARN/WI.RockWI-M",         "Rock County",       'M'  },
	{ 7630, "Rusk",       "rsk","HARN/WI.RuskWI-F",         "Rusk County",       'F'  },
	{    0, "Rusk",       "rsk","HARN/WI.RuskWI-IF",        "Rusk County",       'I'  },
	{ 7571, "Rusk",       "rsk","HARN/WI.RuskWI-M",         "Rusk County",       'M'  },
	{ 7631, "Sauk",       "sak","HARN/WI.SaukWI-F",         "Sauk County",       'F'  },
	{    0, "Sauk",       "sak","HARN/WI.SaukWI-IF",        "Sauk County",       'I'  },
	{ 7572, "Sauk",       "sak","HARN/WI.SaukWI-M",         "Sauk County",       'M'  },
	{ 7632, "Sawyer",     "saw","HARN/WI.SawyerWI-F",       "Sawyer County",     'F'  },
	{    0, "Sawyer",     "saw","HARN/WI.SawyerWI-IF",      "Sawyer County",     'I'  },
	{ 7573, "Sawyer",     "saw","HARN/WI.SawyerWI-M",       "Sawyer County",     'M'  },
	{ 7633, "Shawano",    "sha","HARN/WI.ShawanoWI-F",      "Shawano County",    'F'  },
	{    0, "Shawano",    "sha","HARN/WI.ShawanoWI-IF",     "Shawano County",    'I'  },
	{ 7574, "Shawano",    "sha","HARN/WI.ShawanoWI-M",      "Shawano County",    'M'  },
	{ 7614, "Sheboygan",  "she","HARN/WI.SheboyganWI-F",    "Sheboygan County",  'F'  },
	{    0, "Sheboygan",  "she","HARN/WI.SheboyganWI-IF",   "Sheboygan County",  'I'  },
	{ 7555, "Sheboygan",  "she","HARN/WI.SheboyganWI-M",    "Sheboygan County",  'M'  },
	{ 7634, "StCroix",    "stc","HARN/WI.StCroixWI-F",      "St. Croix County",  'F'  },
	{    0, "StCroix",    "stc","HARN/WI.StCroixWI-IF",     "St. Croix County",  'I'  },
	{ 7575, "StCroix",    "stc","HARN/WI.StCroixWI-M",      "St. Croix County",  'M'  },
	{ 7635, "Taylor",     "tay","HARN/WI.TaylorWI-F",       "Taylor County",     'F'  },
	{    0, "Taylor",     "tay","HARN/WI.TaylorWI-IF",      "Taylor County",     'I'  },
	{ 7576, "Taylor",     "tay","HARN/WI.TaylorWI-M",       "Taylor County",     'M'  },
	{ 7636, "Trempealeau","tre","HARN/WI.TrempealeauWI-F",  "Trempealeau County",'F'  },
	{    0, "Trempealeau","tre","HARN/WI.TrempealeaWI-IF",  "Trempealeau County",'I'  },
	{ 7577, "Trempealeau","tre","HARN/WI.TrempealeauWI-M",  "Trempealeau County",'M'  },
	{ 7637, "Vernon",     "ver","HARN/WI.VernonWI-F",       "Vernon County",     'F'  },
	{    0, "Vernon",     "ver","HARN/WI.VernonWI-IF",      "Vernon County",     'I'  },
	{ 7578, "Vernon",     "ver","HARN/WI.VernonWI-M",       "Vernon County",     'M'  },
	{ 7638, "Vilas",      "vil","HARN/WI.VilasWI-F",        "Vilas County",      'F'  },
	{    0, "Vilas",      "vil","HARN/WI.VilasWI-IF",       "Vilas County",      'I'  },
	{ 7579, "Vilas",      "vil","HARN/WI.VilasWI-M",        "Vilas County",      'M'  },
	{ 7639, "Walworth",   "wal","HARN/WI.WalworthWI-F",     "Walworth County",   'F'  },
	{    0, "Walworth",   "wal","HARN/WI.WalworthWI-IF",    "Walworth County",   'I'  },
	{ 7580, "Walworth",   "wal","HARN/WI.WalworthWI-M",     "Walworth County",   'M'  },
	{ 7640, "Washburn",   "wab","HARN/WI.WashburnWI-F",     "Washburn County",   'F'  },
	{    0, "Washburn",   "wab","HARN/WI.WashburnWI-IF",    "Washburn County",   'I'  },
	{ 7581, "Washburn",   "wab","HARN/WI.WashburnWI-M",     "Washburn County",   'M'  },
	{ 7641, "Washington", "wsg","HARN/WI.WashingtonWI-F",   "Washington County", 'F'  },
	{    0, "Washington", "wsg","HARN/WI.WashingtonWI-IF",  "Washington County", 'I'  },
	{ 7582, "Washington", "wsg","HARN/WI.WashingtonWI-M",   "Washington County", 'M'  },
	{ 7642, "Waukesha",   "wak","HARN/WI.WaukeshaWI-F",     "Waukesha County",   'F'  },
	{    0, "Waukesha",   "wak","HARN/WI.WaukeshaWI-IF",    "Waukesha County",   'I'  },
	{ 7583, "Waukesha",   "wak","HARN/WI.WaukeshaWI-M",     "Waukesha County",   'M'  },
	{ 7643, "Waupaca",    "wap","HARN/WI.WaupacaWI-F",      "Waupaca County",    'F'  },
	{    0, "Waupaca",    "wap","HARN/WI.WaupacaWI-IF",     "Waupaca County",    'I'  },
	{ 7584, "Waupaca",    "wap","HARN/WI.WaupacaWI-M",      "Waupaca County",    'M'  },
	{ 7644, "Waushara",   "wsr","HARN/WI.WausharaWI-F",     "Waushara County",   'F'  },
	{    0, "Waushara",   "wsr","HARN/WI.WausharaWI-IF",    "Waushara County",   'I'  },
	{ 7585, "Waushara",   "wsr","HARN/WI.WausharaWI-M",     "Waushara County",   'M'  },
	{ 7594, "Winnebago",  "win","HARN/WI.WinnebagoWI-F",    "Winnebago County",  'F'  },
	{    0, "Winnebago",  "win","HARN/WI.WinnebagoWI-IF",   "Winnebago County",  'I'  },
	{ 7535, "Winnebago",  "win","HARN/WI.WinnebagoWI-M",    "Winnebago County",  'M'  },
	{ 7645, "Wood",       "wod","HARN/WI.WoodWI-F",         "Wood County",       'F'  },
	{    0, "Wood",       "wod","HARN/WI.WoodWI-IF",        "Wood County",       'I'  },
	{ 7586, "Wood",       "wod","HARN/WI.WoodWI-M",         "Wood County",       'M'  },
	{    0, "",           "",   "",                         "",                  '\0' },
};

// Later on, we'll want to sort the above table into EPSG code order.
// This is the compare function for this sort.  qsort is a stable sort
// everywhere, except maybe on Windows.
int wiFixSrtFunc (const void* prm1,const void* prm2)
{
	const struct prj32WiFix* rec1 = (struct prj32WiFix*)prm1;
	const struct prj32WiFix* rec2 = (struct prj32WiFix*)prm2;

	int cmpValue;
	
	cmpValue = rec1->epsgCode - rec2->epsgCode;
	if (cmpValue == 0)
	{
		cmpValue = stricmp (rec1->csCountyNm,rec2->countyName);
	}
	return cmpValue;
}

bool GetParm (double& parmValue,const TcsEpsgDataSetV6& epsg,TcsEpsgCode prjSysCode,
															 TcsEpsgCode parmCode,
															 TcsEpsgCode sysUomCode);

bool csAddWiNsrs2011 (const wchar_t* csDictDir,const wchar_t* csUserDefDir,
											   const wchar_t* csTempDir)
{
	bool ok;
	bool tmWccs;
	bool lmWccs;

	int st;

	EcsCsvStatus csvSt;
	TcsCsvStatus csvStatus;

	long32_t lngFormat;
	long32_t latFormat;
	long32_t sclFormat;
	long32_t offFormat;

	const char *cPtr;

	double parmValue;

	std::ifstream catStrm;
	std::wifstream nmMapStrm;
	std::wofstream nmMapStrmWr;
	std::wifstream esriStrm;
	std::ifstream ascStrm;

	char usrDefPath [MAXPATH];
	char catPath [MAXPATH];
	char ascPath [MAXPATH];
	wchar_t nmMapPath [MAXPATH];
	wchar_t esriPath [MAXPATH];
	wchar_t epsgPath [MAXPATH];

	// Convert paths to narrow chars.
	wcstombs (usrDefPath,csUserDefDir,sizeof (usrDefPath));

	wcstombs (catPath,csDictDir,sizeof (catPath));
	CS_stncat (catPath,"\\category.asc",sizeof (catPath));
	
	wcscpy (nmMapPath,csDictDir);
	wcscat (nmMapPath,L"\\NameMapper.csv");

	wcscpy (esriPath,csDictDir);
	wcscat (esriPath,L"\\..\\Data\\Esri10-5.csv");
	
	wcstombs (ascPath,csDictDir,sizeof (ascPath));
	CS_stncat (ascPath,"\\coordsys.asc",sizeof (ascPath));

	// Get the Wisconsin CSD file.
	st = CS_altdr (usrDefPath);
	ok = (st == 0);
	if (!ok) return ok;
	TcsCoordsysFile wiCsdFile;
	ok = wiCsdFile.IsOk ();
	if (!ok) return ok;

	// Get the CS-MAP category file.
	catStrm.open (catPath,std::ios::in);
	ok = catStrm.is_open();
	if (!ok) return ok;
	TcsCategoryFile categoryFile (catStrm);
	catStrm.close ();
	if (!ok) return ok;

	// Get the NameMapper
	TcsNameMapper nameMapper;
	nmMapStrm.open (nmMapPath,std::ios::in);
	ok = nmMapStrm.is_open ();
	if (ok)
	{
		csvSt = nameMapper.ReadFromStream (nmMapStrm,csvStatus);
		ok = (csvSt == csvOk);
		nmMapStrm.close ();
	}
	if (!ok) return ok;

	// Activate EPSG.
	wcscpy (epsgPath,L"D:\\GeodeticData\\EPSG\\CSV");
	const TcsEpsgDataSetV6 epsgData (epsgPath);
	ok = epsgData.IsOk ();
	if (!ok) return ok;

	// Get the CS-MAP coordsys.asc file.
	TcsDefFile csMapAsc (dictTypCoordsys,ascPath);
	ok = (csMapAsc.GetDefinitionCount() > 0);
	if (!ok) return ok;

	// get ESRI
	TcsCsvFileBase esriWkt (false,5,6,L"|~~");
	esriStrm.open (esriPath,std::ios::in);
	ok = esriStrm.is_open ();
	if (!ok) return ok;
	ok = esriWkt.ReadFromStream (esriStrm,false,csvStatus);
	esriStrm.close ();
	if (!ok) return ok;

	// Data sources should be in place.  Here we go:
	unsigned esriRecNbr;
	double minLongitude;
	double minLatitude;
	double maxLongitude;
	double maxLatitude;

	char csMapSysName [32];
	char csMapDescrName [128];
	char wrkBufr [1024];
	wchar_t wcharBufr [1024];
	struct prj32WiFix* tblPtr;
	const struct cs_Csdef_* wiCsdDef;
	TcsDefLine* defLinePtr;
	TcsAscDefinition csMapHarnDef;
	TcsAscDefinition csMap2011Def;
	std::wstring epsgSysName;
	std::wstring esriSysName;
	std::wstring esriWktData;
	TcsEpsgCode epsgCode;
	TcsEpsgCode sysUomCode;
	TcsEpsgCode degUomCode (9102UL);
	TcsEpsgCode sclUomCode (9201UL);
	TcsEpsgCode prmOrgLat (8801UL);
	TcsEpsgCode prmOrgLng (8802UL);
	TcsEpsgCode prmSclRed (8805UL);
	TcsEpsgCode prmXoff (8806UL);
	TcsEpsgCode prmYoff (8807UL);

	for (tblPtr = Kprj32WiFix;*tblPtr->countyName != '\0';tblPtr += 1)
	{
		// Terminate on negative status.  This os we can catch a failure
		// with the debugger.
		if (!ok)
		{
			break;
		}

		// Get a pointer to the WI-DOT definition.
		if      (tblPtr->sysUnit == 'M')
		{
			cPtr = "M";
			sysUomCode = 9001;
		}
		else if (tblPtr->sysUnit == 'F')
		{
			cPtr = "USF";
			sysUomCode = 9003;
		}
		else if (tblPtr->sysUnit == 'I')
		{
			// Bit if a kludge here.  The WI .CSD file does not have entries for
			// IFT in the WISCRS-???H8391 category.  But we need a WI .CSD entry
			// to update the useful range appropriately.  Thus, for this purpose,
			// we default to the US Foot definition.  This is OK (currently) as
			// for International Foot systems, all we are doing is updating the
			// useful range.
			cPtr = "USF";
			sysUomCode = 9002;
		}
		else
		{
			ok = false;
			continue;
		}
		sprintf (wrkBufr,"WISCRS-%sH8391%s",tblPtr->wiCountyCode,cPtr);
		wiCsdDef = wiCsdFile.FetchCoordinateSystem (wrkBufr);
		ok = (wiCsdDef != NULL);
		if (!ok) continue;

		// Get the existing HARN/WI definitioin from coordsys.asc.
		ok = csMapAsc.ExtractDefinition (csMapHarnDef,tblPtr->csHarnName);
		if (!ok) continue;

		// Extract the useful range from the WI-DOT def, and place it in
		// the existing CS-MAP HARN/WI def.  We do this for all systems.
		minLongitude = wiCsdDef->ll_min [0];
		minLatitude  = wiCsdDef->ll_min [1];
		maxLongitude = wiCsdDef->ll_max [0];
		maxLatitude  = wiCsdDef->ll_max [1];
		lngFormat = (cs_ATOF_MINSEC | cs_ATOF_MINSEC0 | cs_ATOF_DIRCHR | cs_ATOF_XEAST | 1L);
		latFormat = (cs_ATOF_MINSEC | cs_ATOF_MINSEC0 | cs_ATOF_DIRCHR | 1L);

		CS_ftoa (wrkBufr,sizeof (wrkBufr),minLongitude,lngFormat);
		ok = csMapHarnDef.SetValue ("MIN_LNG:",wrkBufr);
		CS_ftoa (wrkBufr,sizeof (wrkBufr),minLatitude,latFormat);
		ok &= csMapHarnDef.SetValue ("MIN_LAT:",wrkBufr);
		CS_ftoa (wrkBufr,sizeof (wrkBufr),maxLongitude,lngFormat);
		ok &= csMapHarnDef.SetValue ("MAX_LNG:",wrkBufr);
		CS_ftoa (wrkBufr,sizeof (wrkBufr),maxLatitude,latFormat);
		ok &= csMapHarnDef.SetValue ("MAX_LAT:",wrkBufr);
		if (!ok)
		{
			continue;
		}

		// Having updated the useful range, we're done if this is a
		// International Foot system.
		if (tblPtr->sysUnit == 'I')
		{
			continue;
		}
		ok = (tblPtr->epsgCode != 0);
		if (!ok)
		{
			continue;
		}

		// Use the EPSG code to get the EPSG system name.
		epsgCode = tblPtr->epsgCode;
		ok = epsgData.GetFieldByCode (epsgSysName,epsgTblReferenceSystem,
												  epsgFldCoordRefSysName,
												  epsgCode);
		if (!ok) continue;

		// Use the EPSG code to get the ESRI WKT.
		swprintf (wcharBufr,wcCount (wcharBufr),L"%d",tblPtr->epsgCode);
		ok = esriWkt.Locate(esriRecNbr,1,wcharBufr);
		if (!ok) continue;
		ok = esriWkt.GetField (esriWktData,esriRecNbr,4,csvStatus);
		if (!ok) continue;

		// OK, now for the new definition.  Get a copy of the HARN def,
		// and then update it to the 2011 form.
		csMap2011Def = csMapHarnDef;

		// SCL_RED elements tend to have comments, which we will certainly
		// invalidate.
		defLinePtr = csMap2011Def.GetLine ("SCL_RED:");
		if (defLinePtr != NULL)
		{
			defLinePtr->SetComment ("");
		}

		// Devise a new name for this system.
		sprintf (csMapSysName,"WISCRS2011-%s%c",tblPtr->csCountyNm,tblPtr->sysUnit);
		CS_stncp (tblPtr->newKeyNm,csMapSysName,sizeof (tblPtr->newKeyNm));
		csMap2011Def.RenameDef (csMapSysName);

		// Descriptive Name
		sprintf (csMapDescrName,"NSRS2011 Wisconsin County CRS: %s-%c [WISCRS-%s%c]",
																	tblPtr->csCountyNm,
																	tblPtr->sysUnit,
																	tblPtr->wiCountyCode,
																	tblPtr->sysUnit);
		CS_stncp (tblPtr->newDescName,csMapDescrName,sizeof (tblPtr->newDescName));
		csMap2011Def.SetValue ("DESC_NM:",csMapDescrName);

		// Group
		csMap2011Def.SetValue ("GROUP:","OTHR-US");

		// Source
		csMap2011Def.SetValue ("SOURCE:","Wisconsin Department of Transportation");

		// Datum Name
		csMap2011Def.SetValue ("DT_NAME:","NSRS11");

		// Projection Name
		cPtr = csMap2011Def.GetValue ("PROJ:");
		if (!stricmp (cPtr,"TM-WCCS"))
		{
			tmWccs = true;
			lmWccs = false;
			csMap2011Def.SetValue ("PROJ:","TM");
		}
		else if (!stricmp (cPtr,"LM-WCCS"))
		{
			lmWccs = true;
			tmWccs = false;
			csMap2011Def.SetValue ("PROJ:","LM1SP");
		}
		else
		{
			ok = false;
			continue;
		}
		
		// Unit,  this probably is OK, but just to be sure.
		if (tblPtr->sysUnit == 'M')
		{
			csMap2011Def.SetValue ("UNIT:","Meter");
		}
		else if (tblPtr->sysUnit == 'F')
		{
			csMap2011Def.SetValue ("UNIT:","Foot");
		}
		else
		{
			ok = false;
			continue;
		}

		// Do the parameters, depends upon the projection.
		lngFormat = (cs_ATOF_MINSEC | cs_ATOF_MINSEC0 | cs_ATOF_DIRCHR | cs_ATOF_XEAST | 5L);
		latFormat = (cs_ATOF_MINSEC | cs_ATOF_MINSEC0 | cs_ATOF_DIRCHR | 5L);
		sclFormat = 11L;
		if (tmWccs)
		{
			// Do Tranverse Mercator parameters
			ok = GetParm (parmValue,epsgData,epsgCode,prmOrgLng,degUomCode);
			if (!ok) continue;
			CS_ftoa (wrkBufr,sizeof (wrkBufr),parmValue,lngFormat);
			csMap2011Def.SetValue ("PARM1:",wrkBufr);

			csMap2011Def.RemoveLine ("PARM2:");
			csMap2011Def.RemoveLine ("PARM3:");

			ok = GetParm (parmValue,epsgData,epsgCode,prmSclRed,sclUomCode);
			if (!ok) continue;
			CS_ftoa (wrkBufr,sizeof (wrkBufr),parmValue,sclFormat);
			csMap2011Def.SetValue ("SCL_RED:",wrkBufr);

			csMap2011Def.RemoveLine ("ORG_LNG:");			// Defensive: This is expected to fail.

			ok = GetParm (parmValue,epsgData,epsgCode,prmOrgLat,degUomCode);
			if (!ok) continue;
			CS_ftoa (wrkBufr,sizeof (wrkBufr),parmValue,latFormat);
			csMap2011Def.SetValue ("ORG_LAT:",wrkBufr);
		}
		else if (lmWccs)
		{
			// Do Lambert Conformal Conic Parameters.
			csMap2011Def.RemoveLine ("PARM1:");
			csMap2011Def.RemoveLine ("PARM2:");
			csMap2011Def.RemoveLine ("PARM3:");
			csMap2011Def.RemoveLine ("PARM4:");

			ok = GetParm (parmValue,epsgData,epsgCode,prmSclRed,sclUomCode);
			if (!ok) continue;
			CS_ftoa (wrkBufr,sizeof (wrkBufr),parmValue,sclFormat);
			TcsDefLine newSclRedLine (dictTypCoordsys,"SCL_RED:",wrkBufr,NULL);
			csMap2011Def.InsertBefore ("ORG_LNG:",newSclRedLine);

			ok = GetParm (parmValue,epsgData,epsgCode,prmOrgLng,degUomCode);
			if (!ok) continue;
			CS_ftoa (wrkBufr,sizeof (wrkBufr),parmValue,lngFormat);
			csMap2011Def.SetValue ("ORG_LNG:",wrkBufr);

			ok = GetParm (parmValue,epsgData,epsgCode,prmOrgLat,degUomCode);
			if (!ok) continue;
			CS_ftoa (wrkBufr,sizeof (wrkBufr),parmValue,latFormat);
			csMap2011Def.SetValue ("ORG_LAT:",wrkBufr);
		}
		else
		{
			ok = false;
			continue;
		}

		// Do the false origin
		offFormat = 5L;
		if (tblPtr->sysUnit == 'F')
		{
			offFormat += 1;
		}

		ok = GetParm (parmValue,epsgData,epsgCode,prmXoff,sysUomCode);
		if (!ok) continue;
		CS_ftoa (wrkBufr,sizeof (wrkBufr),parmValue,offFormat);
		csMap2011Def.SetValue ("X_OFF:",wrkBufr);

		ok = GetParm (parmValue,epsgData,epsgCode,prmYoff,sysUomCode);
		if (!ok) continue;
		CS_ftoa (wrkBufr,sizeof (wrkBufr),parmValue,offFormat);
		csMap2011Def.SetValue ("Y_OFF:",wrkBufr);

		// Useful range already done,

		// Map Scale, make sure there is one (for our own purposes) and
		// make sure it is set to 1.0
		cPtr = csMap2011Def.GetValue ("MAP_SCL:");
		if (cPtr == NULL)
		{
			TcsDefLine newMapSclLine (dictTypCoordsys,"MAP_SCL:","1.0",NULL);
			csMap2011Def.Append (newMapSclLine);
		}
		else
		{
			csMap2011Def.SetValue ("MAP_SCL:","1.0");
		}

		// Do EPSG and SRID numbers.
		{					// Scoping block
			sprintf (wrkBufr,"%ld",tblPtr->epsgCode);
			TcsDefLine newEpsgLine (dictTypCoordsys,"EPSG:",wrkBufr,NULL);
			csMap2011Def.InsertBefore ("DT_NAME:",newEpsgLine);

			TcsDefLine newSridLine (dictTypCoordsys,"SRID:",wrkBufr,NULL);
			csMap2011Def.InsertBefore ("DT_NAME:",newSridLine);
		}

		// Quadrants.  They may or may not be there.
		cPtr = csMap2011Def.GetValue ("QUAD:");
		if (cPtr == NULL)
		{
			TcsDefLine newQuadLine (dictTypCoordsys,"QUAD:","1",NULL);
			csMap2011Def.InsertBefore ("MAP_SCL",newQuadLine);
		}
		else
		{
			csMap2011Def.SetValue ("QUAD:","1");
		}
		cPtr = csMap2011Def.GetValue ("EPSG_QD:");
		if (cPtr == NULL)
		{
			TcsDefLine newQuadLine (dictTypCoordsys,"EPSG_QD:","1",NULL);
			csMap2011Def.InsertBefore ("MAP_SCL",newQuadLine);
		}
		else
		{
			csMap2011Def.SetValue ("EPSG_QD:","1");
		}

		// Just to be sure, do the standard stuff
		csMap2011Def.RemoveLine ("ZERO_X:");			// This is expected to fail
		csMap2011Def.RemoveLine ("ZERO_Y:");			// This is expected to fail
		
		// Insert the new definition into the COORDSYS.ASC file.
		ok = csMapAsc.Append (csMap2011Def);
		if (!ok) continue;

		// Write a new category entry for the new definition.
		{			// Scoping block
			TcsCategory* wiCategory = categoryFile.FetchCategory ("USA, Wisconsin");
			if (wiCategory == NULL)
			{
				ok = false;
				continue;
			}
			TcsCategoryItem newEntry (csMapSysName,csMapDescrName);
			wiCategory->AddItem (newEntry);
		}
		printf ("%s processed OK.\n",tblPtr->csHarnName);
	}

	// Now for the NameMapper.  There is a complication here as there
	// are duplicate systems in the dictionary.  I.e. same system for
	// two or more counties.  We treat them separately; EPSG and ESRI
	// bunch them up; i.e. one system for several counties.  SO,
	// at this point we wort the driving table into order by EPSG
	// code.  Then loop through the table:
	// 1> EPSG code == zero: continue;
	// 2> EPSG code == last EPSG code processed:
	//		Add two NameMapper alias records for this county, one with
	//		Autodesk flavor, a seocnd with CsMap flavor.
	// 3> EPSG code != last EPSG Code processed:
	//		Add all five, non-alias, entries to the NameMapper:

	// Sort the driveing table.
	size_t elementCnt = sizeof (Kprj32WiFix) / sizeof (struct prj32WiFix) - 1;
	qsort (Kprj32WiFix,elementCnt,sizeof (struct prj32WiFix),wiFixSrtFunc);

	// Initialize detection of duplicate EPOSG codes.
	long lastEpsgCode = -1L;

	// Loop and add NameMapper entries for all systems.
	for (tblPtr = Kprj32WiFix;*tblPtr->countyName != '\0';tblPtr += 1)
	{
		if (!ok)
		{
			break;
		}
		if (tblPtr->epsgCode == 0) continue;
printf ("Processing NameMapper entries: %ld -- %s\n",tblPtr->epsgCode,tblPtr->csHarnName);
		TcsGenericId genericId (tblPtr->epsgCode);
		if (tblPtr->epsgCode == lastEpsgCode)
		{
			mbstowcs (wcharBufr,tblPtr->newKeyNm,wcCount (wcharBufr));
			TcsNameMap nameMap (genericId,csMapProjectedCSysKeyName,
										  csMapFlvrAutodesk,
										  0UL,
										  wcharBufr);
			nameMap.SetAliasFlag (1);
			nameMapper.Add (nameMap);
			// Add the same entry, but with the CsMap flavor.
			nameMap.SetFlavor (csMapFlvrCsMap);
			nameMapper.Add (nameMap);
		}
		else
		{
			// Write an EPSG & ESRI entries for this system.  They are
			// the same except for the names.  Let's get the names first.
			epsgCode = tblPtr->epsgCode;
			ok = epsgData.GetFieldByCode (epsgSysName,epsgTblReferenceSystem,
													  epsgFldCoordRefSysName,
													  epsgCode);
			// Get the ESRI system name.
			if (ok)
			{
				// Use the EPSG code to get the ESRI name.
				swprintf (wcharBufr,wcCount (wcharBufr),L"%d",tblPtr->epsgCode);
				ok = esriWkt.Locate(esriRecNbr,1,wcharBufr);
				if (!ok) continue;
				ok = esriWkt.GetField (esriWktData,esriRecNbr,4,csvStatus);
				if (!ok) continue;

				// Extract the ESRI name from the WKT wstring.
				std::wstring::size_type idx1;
				std::wstring::size_type idx2;
				idx1 = esriWktData.find (L'"');
				idx1 += 1;
				idx2 = esriWktData.find (L'"',idx1);
				esriSysName = esriWktData.substr (idx1,(idx2 - idx1));
			}
			if (ok)
			{
				// Add the EPSG and ESRI entries to the name mapper.
				// Oracle is the same as EPSG 99% of the time, so add that
				// one also.
				TcsNameMap nameMap (genericId,csMapProjectedCSysKeyName,
											  csMapFlvrEpsg,
											  tblPtr->epsgCode,
											  epsgSysName.c_str());
				ok = nameMapper.Add (nameMap);
				if (ok)
				{
					nameMap.SetFlavor (csMapFlvrOracle);
					nameMapper.Add (nameMap);
				}
			}
			if (ok)
			{
				// Add the Sutodesk and CsMap flavor entries.  These are
				// always the same.
				TcsNameMap nameMap (genericId,csMapProjectedCSysKeyName,
											  csMapFlvrEsri,
											  tblPtr->epsgCode,
											  esriSysName.c_str());
				ok = nameMapper.Add (nameMap);
			}
			if (ok)
			{
				mbstowcs (wcharBufr,tblPtr->newKeyNm,wcCount (wcharBufr));
				TcsNameMap nameMap (genericId,csMapProjectedCSysKeyName,
											  csMapFlvrAutodesk,
											  0UL,
											  wcharBufr);
				nameMapper.Add (nameMap);
				// Add the same entry, but with the CsMap flavor.
				nameMap.SetFlavor (csMapFlvrCsMap);
				nameMapper.Add (nameMap);
			}
			if (ok)
			{
				// Prepare for possible moultiple county system.
				lastEpsgCode = tblPtr->epsgCode;
			}
		}
	}

	// Write the results out to their respective locations.
	if (ok)
	{
		// Write the coordsys.asc file.
		ok = csMapAsc.WriteToFile (ascPath);
	}
	if (ok)
	{
		// Write the Category file.
		ok = categoryFile.WriteToFile (catPath);
	}
	if (ok)
	{
		// Write the NameMapper
		nmMapStrmWr.open (nmMapPath,std::ios::out | std::ios::trunc);
		ok = nmMapStrmWr.is_open ();
		if (ok)
		{
			nameMapper.WriteAsCsv (nmMapStrmWr,true);
			nmMapStrmWr.close ();
		}
	}
	return ok;
}

// This function assumes many things, not especiually useful for
// use outside this particular module.  In particular, it relies on
// consistencey about paramaters, systems, and uniot of measure.
// The required consistencies are present in this particular
// application, not likely in others.
bool GetParm (double& parmValue,const TcsEpsgDataSetV6& epsg,TcsEpsgCode prjSysCode,
															 TcsEpsgCode parmCode,
															 TcsEpsgCode sysUomCode)
{
	bool ok;

	TcsEpsgCode opCode;
	TcsEpsgCode mthCode;
	TcsEpsgCode srcUomCode;
	TcsEpsgCode trgUomCode;

	// Need the operation code.
	ok = epsg.GetFieldByCode (opCode,epsgTblReferenceSystem,epsgFldProjectionConvCode,
															prjSysCode);
	if (ok)
	{
		// ALso need the method code.
		ok = epsg.GetFieldByCode (mthCode,epsgTblCoordinateOperation,epsgFldCoordOpMethodCode,
																	 opCode);
	}
	if (ok)
	{
		// Now we can get the parameter value.
		ok = epsg.GetParameterValue (parmValue,opCode,mthCode,parmCode,sysUomCode);
	}
	return ok;
}