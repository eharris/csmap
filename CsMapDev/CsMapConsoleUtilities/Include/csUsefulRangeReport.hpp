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

struct csAreaLimits
{
	double RangeSW [2];
	double RangeNE [2];
};

{
public:
	// Consturction / Destruction / Assignment
	csUsefulRngRpt (void);
	csUsefulRngRpt (const char* csKeyName);
	csUsefulRngRpt (const csUsefulRngRpt& source);
	virtual ~csUsefulRngRpt (void);
	csUsefulRngRpt& operator= (const csUsefulRngRpt& rhs);
	// Operator Overrides / Virtual Functions
	int operator< (const csUsefulRngRpt& comparedTo);
	// Public Named Member Functions
	SetKeyName (const char* csKeyName);
	SetCrsEpsgCode (TcsEpsgCode crsEpsgCode) { CrsEpsgCode = crsEpsgCode; }
	SetCrsEpsgCode (short csPrjCode) { CsPrjCode = csPrjCode; }
protected:
	// Protected Named Member Functions
	// POrotected Data Members
private:
	// Private Named Member Functions
	// Private Data Members
	bool Valid;
	char CsKeyName [24];
	char CsPrjName [64];
	short CsPrjCode;
	short RngEvaluator;
	short LongRange;
	short LatRange;
	TcsEpsgCode CrsEpsgCode;
	TcsEpsgCode AreaEpsgCode;
	struct csAreaLimits CsMapLimits;
	struct csAreaLimits EpsgLimits;
};
