//===========================================================================
// $Header$
//
//    (C) Copyright 2007 by Autodesk, Inc.
//
// The information contained herein is confidential, proprietary
// to Autodesk, Inc., and considered a trade secret as defined 
// in section 499C of the penal code of the State of California.  
// Use of this information by anyone other than authorized employees
// of Autodesk, Inc. is granted only under a written non-disclosure 
// agreement, expressly prescribing the scope and manner of such use.       
//
// CREATED BY:
//      Norm Olsen
//
// DESCRIPTION:
//

struct TcsCrsRange
{
	TcsCrsRange::CrsRange (void)
	TcsCrsRange (double swLng,double swLat,double neLng,double neLat);
	TcsCrsRange (double southwest [2],double northeast [2]);
	TcsCrsRange::csCrsRange (const csCrsRange& source);
	TcsCrsRange& operator= (const csCrsRange& rhs);
	TcsCrsRange& operator+= (const csCrsRange& rhs);
	short LngRange (void) const;
	short LatRange (void) const;
	void WriteToStream (std::ostream& oStrm);
public:				// Redundant, for documentation purposes
	double RangeSW [2];
	double RangeNE [2];
};

class TcsUsefulRngRpt
{
public:
	// Consturction / Destruction / Assignment
	TcsUsefulRngRpt (void);
	TcsUsefulRngRpt (const char* csKeyName);
	TcsUsefulRngRpt (const TcsUsefulRngRpt& source);
	virtual ~TcsUsefulRngRpt (void);
	TcsUsefulRngRpt& operator= (const TcsUsefulRngRpt& rhs);
	// Operator Overrides / Virtual Functions
	// This is the sort comparison function.  The code for this function will
	// be sadjusted to produce the various reports.
	int operator< (const TcsUsefulRngRpt& comparedTo);
	// Public Named Member Functions
	void SetKeyName (const char* csKeyName);
	void SetPrjName (const char* prjName);
	short SetCrsPrjCode (short csPrjCode);
	short SetRangeEvaluation (short rangeEvaluation);
	void SetCrsEpsgCode (TcsEpsgCode crsEpsgCode);
	void SetAreaEpsgCode (TcsEpsgCode areaEpsgCode);
	void SetCsMapLimits (const struct csCrsRange& csRange);
	void SetEpsgLimits (const struct csCrsRange& csRange);

	short GetLngRange (void);
	short GetLatRange (void);
	void WriteToStream (std::ostream& oStrm);
protected:
	// Protected Named Member Functions
	// Protected Data Members
	bool Valid;
	char CsKeyName [24];
	char CsPrjName [64];
	short CsPrjCode;
	short RngEvaluation;
	short LngRange;			// in minutes of Longitude
	short LatRange;			// in minutes of Latitude
	TcsEpsgCode CrsEpsgCode;
	TcsEpsgCode AreaEpsgCode;
	TcsCrsRange CsMapRange;
	TcsCrsRange EpsgRange;
private:
	// Private Named Member Functions
	// Private Data Members
};
