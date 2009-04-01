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

#include <math.h>
#include <stack>

enum EcsEpsgTable { epsgTblNone = 0,
					epsgTblAlias,
					epsgTblArea,
					epsgTblChange,
					epsgTblAxisName,
					epsgTblAxis,
					epsgTblReferenceSystem,
					epsgTblCoordinateSystem,
					epsgTblOperationMethod,
					epsgTblParameterUsage,
					epsgTblParameterValue,
					epsgTblParameter,
					epsgTblOperationPath,
					epsgTblCoordinateOperation,
					epsgTblDatum,
					epsgTblDeprecation,
					epsgTblEllipsoid,
					epsgTblNamingSystem,
					epsgTblPrimeMeridian,
					epsgTblSupercession,
					epsgTblUnitOfMeasure,
					epsgTblVersionHistory,
					epsgTblUnknown
				  };

enum EcsEpsgField { epsgFldNone = 0,
					epsgFldAction,
					epsgFldAliasCode,
					epsgFldAlias,
					epsgFldAreaCode,
					epsgFldAreaOfUse,
					epsgFldAreaOfUseCode,
					epsgFldAreaName,
					epsgFldAreaEastBoundLng,
					epsgFldAreaNorthBoundLat,
					epsgFldAreaSouthBoundLat,
					epsgFldAreaWestBoundLng,
					epsgFldBoundingPolygonFileName,
					epsgFldChangeId,
					epsgFldCmpdHorizCrsCode,
					epsgFldCmpdVertCrsCode,
					epsgFldCodeAreaOfUse,
					epsgFldCodeNamingSystem,
					epsgFldCodesAffected,
					epsgFldComment,
					epsgFldConcatOperationCode,
					epsgFldCoordAxisAbbreviation,
					epsgFldCoordAxisCode,
					epsgFldCoordAxisName,
					epsgFldCoordAxisNameCode,
					epsgFldCoordAxisOrientation,
					epsgFldCoordOpAccuracy,
					epsgFldCoordOpCode,
					epsgFldCoordOpMethodCode,
					epsgFldCoordOpMethodName,
					epsgFldCoordOpName,
					epsgFldCoordOpScope,
					epsgFldCoordOpType,
					epsgFldCoordOpVariant,
					epsgFldCoordRefSysCode,
					epsgFldCoordRefSysKind,
					epsgFldCoordRefSysName,
					epsgFldCoordSysCode,
					epsgFldCoordSysName,
					epsgFldCoordSysType,
					epsgFldCoordTfmVersion,
					epsgFldCrsScope,
					epsgFldDataSource,
					epsgFldDateClosed,
					epsgFldDatumCode,
					epsgFldDatumName,
					epsgFldDatumScope,
					epsgFldDatumType,
					epsgFldDeprecated,
					epsgFldDeprecationDate,
					epsgFldDeprecationId,
					epsgFldDeprecationReason,
					epsgFldDescription,
					epsgFldDimension,
					epsgFldEllipsoidCode,
					epsgFldEllipsoidName,
					epsgFldEllipsoidShape,
					epsgFldExample,
					epsgFldFactorB,
					epsgFldFactorC,
					epsgFldFormula,
					epsgFldGreenwichLongitude,
					epsgFldInformationSource,
					epsgFldInvFlattening,
					epsgFldIsoA2Code,
					epsgFldIsoA3Code,
					epsgFldIsoNumericCode,
					epsgFldLeftLongitude,
					epsgFldNamingSystemCode,
					epsgFldNamingSystemName,
					epsgFldNorthLatitude,
					epsgFldOrder,
					epsgFldObjectCode,
					epsgFldObjectType,
					epsgFldObjectTableName,
					epsgFldOpPathStep,
					epsgFldOriginDescription,
					epsgFldParameterCode,
					epsgFldParameterName,
					epsgFldParameterValue,
					epsgFldParamSignReversal,
					epsgFldParamValueFileRef,
					epsgFldPrimeMeridianCode,
					epsgFldPrimeMeridianName,
					epsgFldProjectionConvCode,
					epsgFldRealizationEpoch,
					epsgFldRemarks,
					epsgFldReplacedBy,
					epsgFldReportDate,
					epsgFldReporter,
					epsgFldRequest,
					epsgFldReverseOp,
					epsgFldRevisionDate,
					epsgFldRightLongitude,
					epsgFldSemiMajorAxis,
					epsgFldSemiMinorAxis,
					epsgFldShowCrs,
					epsgFldShowOperation,
					epsgFldSingleOperationCode,
					epsgFldSortOrder,
					epsgFldSourceCrsCode,
					epsgFldSourceGeogCrsCode,
					epsgFldSouthLatitude,
					epsgFldSupercededBy,
					epsgFldSupercedes,
					epsgFldSupersessionId,
					epsgFldSupersessionType,
					epsgFldSupersessionYear,
					epsgFldTablesAffected,
					epsgFldTargetCrsCode,
					epsgFldTargetUomCode,
					epsgFldUnitOfMeasName,
					epsgFldUnitOfMeasType,
					epsgFldUomCode,
					epsgFldUomCodeSourceCoordDiff,
					epsgFldUomCodeTargetCoordDiff,
					epsgFldVersionDate,
					epsgFldVersionHistoryCode,
					epsgFldVersionNumber,
					epsgFldVersionRemarks,
					epsgFldUnknown
				  };

// Note there is a difference between a CRS and  Csys

enum EcsCsysType { epsgCsysTypNone = 0,
			 	   epsgCsysTypAffine,
				   epsgCsysTypCartesian,
				   epsgCsysTypCylindrical,
				   epsgCsysTypEllipsoidal,
				   epsgCsysTypLinear,
				   epsgCsysTypPolar,
				   epsgCsysTypSpherical,
				   epsgCsysTypVertical,
				   epsgCsysTypUnknown
				 };

enum EcsCrsType { epsgCrsTypNone = 0,
				  epsgCrsTypCompund,
				  epsgCrsTypEngineering,
				  epsgCrsTypGeocentric,
				  epsgCrsTypGeographic2D,
				  epsgCrsTypGeographic3D,
				  epsgCrsTypProjected,
				  epsgCrsTypVertical,
				  epsgCrsTypUnknown
				};

enum EcsOpType { epsgOpTypNone = 0,
				 epsgOpTypConversion,
				 epsgOpTypTransformation,
				 epsgOpTypConcatenated,
				 epsgOpTypUnknown
			   };

enum EcsDtmType { epsgDtmTypNone = 0,
				  epsgDtmTypGeodetic,
				  epsgDtmTypVertical,
				  epsgDtmTypEngineering,
				  epsgDtmTypUnknown
			   };

enum EcsUomType { epsgUomTypNone = 0,
				  epsgUomTypLinear,
				  epsgUomTypAngular,
				  epsgUomTypScale,
				  epsgUomTypUnknown
			   };
			   
enum EcsOrientation { epsgOrntNone = 0,
					  epsgOrntEast,
					  epsgOrntWest,
					  epsgOrntNorth,
					  epsgOrntSouth,
					  epsgOrntUp,
					  epsgOrntDown,
					  epsgOrntUnknown
					};

struct TcsEpsgTblMap
{
	EcsEpsgTable  TableId;
	short         FieldCount;
	EcsEpsgField  CodeKeyFieldId;
	wchar_t       TableName [64];
	EcsEpsgField  Sort1;
	EcsEpsgField  Sort2;
	EcsEpsgField  Sort3;
	EcsEpsgField  Sort4;
};
struct TcsEpsgFldMap
{
	EcsEpsgTable TableId;
	EcsEpsgField FieldId;
	short        FieldNbr;
	wchar_t      FieldName [48];
};

struct TcsEpsgCsysTypeMap
{
	EcsCsysType CsysType;
	wchar_t     CsysTypeName [64];
};

struct TcsEpsgCrsTypeMap
{
	EcsCrsType  CrsType;
	wchar_t     CrsTypeName [64];
};

struct TcsEpsgOpTypeMap
{
	EcsOpType  OpType;
	wchar_t    OpTypeName [64];
};

struct TcsEpsgDtmTypeMap
{
	EcsDtmType  DtmType;
	wchar_t     DtmTypeName [64];
};

struct TcsEpsgUomTypeMap
{
	EcsUomType  UomType;
	wchar_t     UomTypeName [64];
};

struct TcsEpsgOrntTypeMap
{
	EcsOrientation  OrntType;
	wchar_t         OrntTypeName [64];
};

const wchar_t* GetEpsgTableName (EcsEpsgTable tblId);
EcsEpsgTable GetEpsgTableId (const wchar_t* tableName);
short GetEpsgCodeFieldNbr (EcsEpsgTable tableId);
short GetEpsgFieldNumber (EcsEpsgTable tableId,EcsEpsgField fieldId);

EcsCsysType GetEpsgCsysType (const wchar_t* csysTypeName);
EcsCrsType GetEpsgCrsType (const wchar_t* crsTypeName);
EcsOpType GetEpsgOpType (const wchar_t* opTypeName);
EcsDtmType GetEpsgDtmType (const wchar_t* dtmTypeName);
EcsUomType GetEpsgUomType (const wchar_t* uomTypeName);

//newPage//
//============================================================================
// EPSG Code Variable
//
// This object is used to represent an EPSG code.  All code specific to EPSG
// code values is handled within this object.
//
class TcsEpsgCode
{
	//=========================================================================
	// Static Constants, Variables, and Member Functions
	static const unsigned long InvalidValue;		// Zero, for now!
public:
	//=========================================================================
	// Construction  /  Destruction  /  Assignment
	TcsEpsgCode (void);
	TcsEpsgCode (unsigned long epsgCode);
	TcsEpsgCode (const wchar_t* epsgCode);
	TcsEpsgCode (const std::wstring& epsgCode);
	TcsEpsgCode (const TcsEpsgCode& source);
	~TcsEpsgCode (void);
	TcsEpsgCode& operator= (const TcsEpsgCode& rhs);
	TcsEpsgCode& operator= (unsigned long epsgCode);
	//=========================================================================
	// Operator Overrides
	bool operator< (unsigned long epsgCode) const;
	bool operator< (const std::wstring& epsgCode) const;
	bool operator== (unsigned long epsgCode) const;
	bool operator== (const std::wstring& epsgCode) const;
	bool operator> (unsigned long epsgCode) const;
	bool operator> (const std::wstring& epsgCode) const;
	operator unsigned long () const {return EpsgCode; };
	TcsEpsgCode operator++ (void);
	TcsEpsgCode operator++ (int dummy);
	TcsEpsgCode& operator+= (unsigned long rhs);
	TcsEpsgCode& operator-= (unsigned long rhs);
	TcsEpsgCode& operator+= (int rhs);
	TcsEpsgCode& operator-= (int rhs);
	TcsEpsgCode operator-- (void);
	TcsEpsgCode operator-- (int dummy);
	TcsEpsgCode operator+ (unsigned long rhs);
	TcsEpsgCode operator+ (int rhs);
	TcsEpsgCode operator- (unsigned long rhs);
	TcsEpsgCode operator- (int rhs);
	//=========================================================================
	// Public Named Functions
	bool IsValid (void) const {return EpsgCode != 0UL; }
	bool IsNotValid (void) const {return EpsgCode == 0UL; }
	std::wstring AsWstring (void) const;
	std::string AsString (void) const;
	bool AsString (wchar_t* result,size_t resultSize) const;
	bool AsString (char* result,size_t resultSize) const;
	void Invalidate (void) {EpsgCode = InvalidValue; };
protected:
	//=========================================================================
	// Protected Support Functions
	unsigned long StrToEpsgCode (const wchar_t* epsgCodeStr) const;
	unsigned long StrToEpsgCode (const char* epsgCodeStr) const;
private:
	//=========================================================================
	// Private Support Functions
	//=========================================================================
	// Private Data Members
	unsigned long EpsgCode;
};

//newPage//
//============================================================================
// EPSG Table Specialization
//
// An object which encapulates all CSV file functionality, adding a few special
// features particular to EPSG tables:
//
// 1> Maintains a binary unsigned long index on the first field of each table.
// 2> Provides for setting a current record, and accessing fields in that record.
// 3> Provides getting field data in a specific form.
//
// Due to the CurrentRecord feature, and the TcsCsvStatus member, this object
// is not multi-thread safe.
//
class TcsEpsgTable : public TcsCsvFileBase
{
public:
	//=========================================================================
	// Static Constants, Variables, and Member Functions
	static const wchar_t LogicalTrue  [6];
	static const wchar_t LogicalFalse [6];
	//=========================================================================
	// Construction  /  Destruction  /  Assignment
	TcsEpsgTable (const TcsEpsgTblMap& tblMap,const wchar_t* databaseFldr);
	TcsEpsgTable (const TcsEpsgTable& source);
	virtual ~TcsEpsgTable (void);
	TcsEpsgTable& operator= (const TcsEpsgTable& rhs);
	//=========================================================================
	// Operator Overrides
	//=========================================================================
	// Public Named Functions
	bool IsOk (void) const {return Ok; };
	EcsEpsgTable GetTableId (void) const {return TableId; };
	const TcsCsvStatus& GetStatus (void) const {return CsvStatus; };
	bool SetCurrentRecord (const TcsEpsgCode& epsgCode);
	bool PushCurrentPosition (void);
	bool RestorePreviousPosition (void);
	bool EpsgLocateCode (TcsEpsgCode& epsgCode,EcsEpsgField fieldId,const wchar_t* fldValue);
	bool PositionToFirst (EcsEpsgField fieldId,const wchar_t* fldValue,bool honorCase = false);
	bool PositionToFirst (EcsEpsgField fieldId,const TcsEpsgCode& epsgCode);
	bool PositionToNext (EcsEpsgField fieldId,const wchar_t* fldValue,bool honorCase = false);
	bool PositionToNext (EcsEpsgField fieldId,const TcsEpsgCode& epsgCode);

	bool IsDeprecated (void);

	bool GetField (std::wstring& result,short fieldNbr);
	bool GetAsLong (long& result,short fieldNbr);
	bool GetAsULong (unsigned long& result,short fieldNbr);
	bool GetAsEpsgCode (TcsEpsgCode& result,short fieldNbr);
	bool GetAsReal (double& result,short fieldNbr);
	bool GetAsLogical (bool& result,short fieldNbr);

	bool GetField (std::wstring& result,EcsEpsgField fieldId);
	bool GetAsLong (long& result,EcsEpsgField fieldId);
	bool GetAsULong (unsigned long& result,EcsEpsgField fieldId);
	bool GetAsEpsgCode (TcsEpsgCode& result,EcsEpsgField fieldId);
	bool GetAsReal (double& result,EcsEpsgField fieldId);
	bool GetAsLogical (bool& result,EcsEpsgField fieldId);

	bool GetField (std::wstring result,const TcsEpsgCode& epsgCode,short fieldNbr);
	bool GetAsLong (long& result,const TcsEpsgCode& epsgCode,short fieldNbr);
	bool GetAsULong (unsigned long& result,const TcsEpsgCode& epsgCode,short fieldNbr);
	bool GetAsEpsgCode (TcsEpsgCode& result,const TcsEpsgCode& epsgCode,short fieldNbr);
	bool GetAsReal (double& result,const TcsEpsgCode& epsgCode,short fieldNbr);

	const TcsCsvStatus& GetCsvStatus (void) const;
private:
	//=========================================================================
	// Private Support Functions
	bool PrepareCsvFile (void);
	//=========================================================================
	// Private Data Members
	bool Ok;
	bool Sorted;
	bool Indexed;
	EcsEpsgTable TableId;
	EcsEpsgField CodeKeyField;
	TcsEpsgCode CurrentCodeValue;
	unsigned CurrentRecordNbr;
	TcsCsvSortFunctor SortFunctor;
	std::stack<TcsEpsgCode> CodeKeyStack;
	TcsCsvStatus CsvStatus;
};
//=============================================================================
// TcsEpsgDataSetV6  -  An EPSG dataset based on the version 6 model.
//
// An image of the dataset in .csv form is expected to reside in the directory
// provided to the constructor.  The object is intended to be a read only object
// but nothing specific was done to preclude changing the underlying tables or
// writing changes back to the .csv files.  There are just no member functions
// at this time to support such operation.
//
class TcsEpsgDataSetV6
{
public:
	//=========================================================================
	// Static Constants, Variables, and Member Functions
	//
	//=========================================================================
	// The following structure is used to map EPSG source/target Base
	// Coordinate Reference System codes to CS-MAP to84_via codes in special
	// cases.  The special cases revolve around three concepts:
	//
	// 1) The null datum concept where a specific datum is considered to be
	//    close enough to WGS84 such that there is no datum transformation
	//    necessary.
	// 2) The datum transformation involves a datum shift data file and
	//    related algorithms.
	// 3) The datum shifts are basically parameterless.
	//
	// The Coordinate Operation Method code was originally included to reduce
	// the likehood of an inappropriateuse of the table. Turns out that the
	// implemnetation doesn't actually use them.
	//
	// The base codes are all 2D variations.
	struct TcsCsMapDtmCodeMap
	{
		TcsEpsgCode MethodCode;
		TcsEpsgCode SourceBaseCode;
		TcsEpsgCode TargetBaseCode;
		short         CsMapTo84Code;
	};
	static const TcsCsMapDtmCodeMap KcsCsMapDtmCodeMap [];
    static short GetFldNbr (EcsEpsgTable tableId,EcsEpsgField fieldId);
    static short GetFldName (std::wstring& fieldName,EcsEpsgTable tableId,EcsEpsgField fieldId);
    //=========================================================================
    // Construction  /  Destruction  /  Assignment
    TcsEpsgDataSetV6 (const wchar_t* databaseFolder,const wchar_t* revLevel);
    TcsEpsgDataSetV6 (const TcsEpsgDataSetV6& source);
    virtual ~TcsEpsgDataSetV6 (void);
    TcsEpsgDataSetV6& operator= (const TcsEpsgDataSetV6& rhs);
    //=========================================================================
    // Operator Overrides
    //=========================================================================
    // Public Named Functions
    TcsEpsgTable* GetTablePtr (EcsEpsgTable tableId);
    const TcsEpsgTable* GetTablePtr (EcsEpsgTable tableId) const;
    bool GetField (std::wstring result,EcsEpsgTable tableId,EcsEpsgField fieldId);
    bool GetFieldAsLong (long& result,EcsEpsgTable tableId,EcsEpsgField fieldId);
    bool GetFieldAsULong (unsigned long& result,EcsEpsgTable tableId,EcsEpsgField fieldId);
    bool GetFieldAsReal (double& result,EcsEpsgTable tableId,EcsEpsgField fieldId);
    bool GetFieldAsEpsgCode (TcsEpsgCode& result,EcsEpsgTable tableId,EcsEpsgField fieldId);
	bool GetUomToDegrees (double& toDegrees,TcsEpsgCode uomCode);
	bool GetUomToMeters (double& toMeters,TcsEpsgCode uomCode);
	bool GetUomToUnity (double& toUnity,TcsEpsgCode uomCode);
    bool GetFieldAsDegrees (double& result,EcsEpsgTable tableId,EcsEpsgField fieldId,
																TcsEpsgCode uomCode);
    bool GetFieldAsMeters (double& result,EcsEpsgTable tableId,EcsEpsgField fieldId,
															   TcsEpsgCode uomCode);
    bool ConvertUnits (double& result,TcsEpsgCode trgUomCode,double value,
															 TcsEpsgCode srcUomCode);
    //=========================================================================
    unsigned GetRecordCount (EcsEpsgTable tableId);
    bool GetFieldByIndex (std::wstring& fieldData,EcsEpsgTable tableId,EcsEpsgField fieldId,
																	   unsigned recNbr);
    bool GetCodeByIndex (TcsEpsgCode& epsgCode,EcsEpsgTable tableId,EcsEpsgField fieldId,
																	unsigned recNbr);
    bool GetFieldByCode (std::wstring& fieldData,EcsEpsgTable tableId,EcsEpsgField fieldId,
																	  TcsEpsgCode epsgCode);
    bool CompareCsMapUnits (const struct cs_Unittab_* csMapUnitTbl,bool useNameMap = false);
    bool GetCsMapEllipsoid (struct cs_Eldef_& ellipsoid,TcsEpsgCode epsgCode);
    TcsEpsgCode LocateGeographicBase (EcsCrsType crsType,TcsEpsgCode datumCode);
    bool LocateOperationVariants (unsigned& variantCount,unsigned variants[],
													     unsigned variantsSize,
													     EcsOpType opType,
													     TcsEpsgCode sourceCode,
													     TcsEpsgCode targetCode);
    TcsEpsgCode LocateOperation (EcsOpType crsType,TcsEpsgCode sourceCode,TcsEpsgCode targetCode,
																		  long variant = 1L);
    bool ResolveConcatenatedXfrm (short& csMapViaCode,TcsEpsgCode oprtnCode);
	TcsEpsgCode GetParameterValue (double& parameterValue,TcsEpsgCode opCode,TcsEpsgCode opMethCode,
																			 TcsEpsgCode prmCode);
	bool ResolveConcatenatedXfrm (short& csMapViaCode,TcsEpsgCode pathOprtnCode) const;
	bool IsNullTransform (bool& isNull,const TcsEpsgCode& oprtnCode);
	bool IsPrMerRotation (bool& isPrMerRot,const TcsEpsgCode& oprtnCode);
    bool GetCsMapDatum (struct cs_Dtdef_& datum,struct cs_Eldef_& ellipsoid,TcsEpsgCode epsgDtmCode,
																			long instance = 0L);
	bool GetCsMapCoordsys (struct cs_Csdef_& coordsys,struct cs_Dtdef_& datum,struct cs_Eldef_& ellipsoid,
																			  TcsEpsgCode crsEpsgCode);
protected:
    //=========================================================================
    // Protected Support Functions
    bool GetCoordsysQuad (short& quad,TcsEpsgCode crsEpsgCode);
    bool GetReferenceDatum (TcsEpsgCode& dtmEpsgCode,TcsEpsgCode crsEpsgCode);
	bool GetPrimeMeridian (double& primeMeridian,TcsEpsgCode crsEpsgCode);
	bool GeographicCoordsys (struct cs_Csdef_& coordsys,TcsEpsgCode crsEpsgCode);
	bool ProjectedCoordsys (struct cs_Csdef_& coordsys,TcsEpsgCode crsEpsgCode);
private:
    //=========================================================================
    // Private Support Functions
    //=========================================================================
    // Private Data Members
    std::wstring RevisionLevel;
	std::wstring DatabaseFolder;
	std::map<EcsEpsgTable,TcsEpsgTable*> EpsgTables;
};

// Locate Operation returns an array and a count.  The array contains the op-codes of a path,
// and the count indicates how many there are.  The return value of the function is the
// count, thus zero remains the indication that there are none.  The dimensions of the
// operation code array are passed as an additional parameter.

// Also, I have an array which maps operation codes to specific CS-MAP to84_via codes.
// Thus, the "null" modem operation codes should for the most part, map to an CS-MAP code.

// Finally, we have a table that associates EPSG method codes and files names to
// CS-MAP codes.  So, when we encounter the NTv2 EPSG code, we extract the file
// name, and look it up; the result should be a CS-MAP to84_via code.
