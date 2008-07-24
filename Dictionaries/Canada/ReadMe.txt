Datum Shift Issues for Canadian Users

The coordinate system software automatically performs a datum shift if 
the source and destination coordinate systems use different datums. 
Within North America, this is most often a shift between the NAD27 and 
NAD83 datums.  For U.S. users, Autodesk Map uses the freely distributable
NADCON data files supplied by USGS.

Canadian users may use either version 1 or version 2 of the Canadian
National Transformation.  These files do not ship with Autodesk Map, however,
as they are subject to a license fee and are not freely distributable.

To use version 2 of the Canadian National Transformation, you need to
perform the following steps:

1.  Obtain a copy of the data file.  Contact:

	Information Services,
	Geodetic Survey Division, Geomatics Canada, 615 Booth Street,
	Ottawa, Ontario, K1A 0E9
	(613) 995-4410,
	information@geod.nrcan.gc.ca
	http://www.geod.nrcan.gc.ca.

2.  Once you have the file, copy it into the Canada\
directory (this is the same directory where this ReadMe.txt file
was installed) and name it Ntv2_0.gsb.

3.  Open the file Nad27ToNad83.gdc (located in the same directory
as the coordinate system dictionary files) with some text editor
program such as Notepad.  Find the section labeled "CANADA SPECIFIC
NOTES."  Delete the initial "#" symbol from the line that begins:
#.\Canada\Ntv2_0.gsb




To use version 1 of the Canadian National Transformation:

WARNING: Geomatics Canada no longer supports version 1, and many 
Canadian provinces do not consider it to produce valid results. If you 
are in Canada and doing NAD Shifts, you should obtain the ntv2_0.gsb 
from Geomatics Canada.

To use version 1 of the national transformation, obtain the grid 
file, copy it into the coordsys subdirectory, and give it the name 
grid11.dac.  Then edit the Nad27ToNad83.gdc file (described above)
by deleting the initial "#" symbol from the line that begins:
#.\Canada\grid11.dac



To use datum shift files provided by a regional authority:

1.  Obtain a copy of the data file. 

2.  Once you have the file, copy it into the Canada\
directory (this is the same directory where this ReadMe.txt file
was installed). 

3.  Open the gdc file realted to the type of transformation (located
in the same directory as the coordinate system dictionary files) with
some text editor program such as Notepad. For example if the data shift 
file relates to a transformation from the ATS77 datum to the CSRS datum,
open the Ats77ToCsrs.gdc file.
Either delete the initial "#" symbol from the relevant line or add a line
to indicate the location and name of the data file.


