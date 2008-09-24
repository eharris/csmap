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

//lint -esym(534,TcsCsvFileBase::SetMinFldCnt,TcsCsvFileBase::SetMaxFldCnt)
//lint -esym(534,wcstombs)

#include "cs_map.h"
#include "csCsvFileSupport.hpp"
#include "csEpsgStuff.h"

#ifndef _WIN32
#define _wcsicmp wcscasecmp
#endif

extern "C" double cs_Zero;
extern "C" double cs_One;

//=============================================================================
// NOTE:  The CodeKeyField is the field ID of the field which carries a
// numeric record identifier which is unique.  In all cases, except the
// Change table, this value is an integral value.  In most cases, the
// table is sorted by this value, but there are exceptions.  Most notable
// exception is the Coordinate Axis table.  For several tables, the indicated
// sorted order is required; most notably the Coordinate Axis, Parameter Usage,
// Parameter Value, and Operation Path tables.
//
// It is the CodeKeyField by which the table is indexed in all cases.  As this
// is a feature of TcsCsvFileBase, the index is actually in text form, and in
// the case of the CHange table, in the exact form as it appears in the EPSG
// database.
//=============================================================================

const TcsEpsgTblMap KcsEpsgTblMap [] =
{
//                                  Field
//                          TableID  Cnt  CodeKeyField                  Table CSV File Name                     Sort Field 1                Sort Field 2              Sort Field 3             Sort Field 4  
	{                  epsgTblAlias,  6,  epsgFldAliasCode,            L"Alias",                                epsgFldAliasCode,           epsgFldNone,              epsgFldNone,             epsgFldNone },
	{                   epsgTblArea, 17,  epsgFldAreaCode,             L"Area",                                 epsgFldAreaCode,            epsgFldNone,              epsgFldNone,             epsgFldNone },
	{                 epsgTblChange,  9,  epsgFldChangeId,             L"Change",                               epsgFldChangeId,            epsgFldNone,              epsgFldNone,             epsgFldNone },
	{               epsgTblAxisName,  9,  epsgFldCoordAxisNameCode,    L"Coordinate Axis Name",                 epsgFldCoordAxisNameCode,   epsgFldNone,              epsgFldNone,             epsgFldNone },
	{                   epsgTblAxis,  7,  epsgFldCoordAxisCode,        L"Coordinate Axis",                      epsgFldCoordSysCode,        epsgFldOrder,             epsgFldNone,             epsgFldNone },
	{        epsgTblReferenceSystem, 18,  epsgFldCoordRefSysCode,      L"Coordinate Reference System",          epsgFldCoordRefSysCode,     epsgFldNone,              epsgFldNone,             epsgFldNone },
	{       epsgTblCoordinateSystem, 10,  epsgFldCoordSysCode,         L"Coordinate System",                    epsgFldCoordSysCode,        epsgFldNone,              epsgFldNone,             epsgFldNone },
	{        epsgTblOperationMethod, 11,  epsgFldCoordOpMethodCode,    L"Coordinate_Operation Method",          epsgFldCoordOpMethodCode,   epsgFldNone,              epsgFldNone,             epsgFldNone },
	{         epsgTblParameterUsage,  4,  epsgFldNone,                 L"Coordinate_Operation Parameter Usage", epsgFldCoordOpMethodCode,   epsgFldSortOrder,         epsgFldNone,             epsgFldNone },
	{         epsgTblParameterValue,  6,  epsgFldNone,                 L"Coordinate_Operation Parameter Value", epsgFldCoordOpCode,         epsgFldCoordOpMethodCode, epsgFldParameterCode,    epsgFldNone },
	{              epsgTblParameter,  8,  epsgFldParameterCode,        L"Coordinate_Operation Parameter",       epsgFldParameterCode,       epsgFldNone,              epsgFldNone,             epsgFldNone },
	{          epsgTblOperationPath,  3,  epsgFldConcatOperationCode,  L"Coordinate_Operation Path",            epsgFldConcatOperationCode, epsgFldOpPathStep,        epsgFldNone,             epsgFldNone },
 	{    epsgTblCoordinateOperation, 20,  epsgFldCoordOpCode,          L"Coordinate_Operation",                 epsgFldCoordOpCode,         epsgFldNone,              epsgFldNone,             epsgFldNone },
	{                  epsgTblDatum, 15,  epsgFldDatumCode,            L"Datum",                                epsgFldDatumCode,           epsgFldNone,              epsgFldNone,             epsgFldNone },
	{            epsgTblDeprecation,  7,  epsgFldDeprecationId,        L"Deprecation",                          epsgFldObjectCode,          epsgFldNone,              epsgFldNone,             epsgFldNone },
	{              epsgTblEllipsoid, 13,  epsgFldEllipsoidCode,        L"Ellipsoid",                            epsgFldEllipsoidCode,       epsgFldNone,              epsgFldNone,             epsgFldNone },
	{           epsgTblNamingSystem,  8,  epsgFldNamingSystemCode,     L"Naming System",                        epsgFldNamingSystemCode,    epsgFldNone,              epsgFldNone,             epsgFldNone },
	{          epsgTblPrimeMeridian, 10,  epsgFldPrimeMeridianCode,    L"Prime Meridian",                       epsgFldPrimeMeridianCode,   epsgFldNone,              epsgFldNone,             epsgFldNone },
	{           epsgTblSupercession,  7,  epsgFldSupersessionId,       L"Supersession",                         epsgFldObjectCode,          epsgFldObjectTableName,   epsgFldSupersessionYear, epsgFldNone },
	{          epsgTblUnitOfMeasure, 12,  epsgFldUomCode,              L"Unit of Measure",                      epsgFldUomCode,             epsgFldNone,              epsgFldNone,             epsgFldNone },
	{         epsgTblVersionHistory,  6,  epsgFldVersionHistoryCode,   L"Version History",                      epsgFldVersionNumber,       epsgFldNone,              epsgFldNone,             epsgFldNone },
	{                epsgTblUnknown,  0,  epsgFldNone,                 L"",                                     epsgFldNone,                epsgFldNone,              epsgFldNone,             epsgFldNone },
};
const TcsEpsgFldMap KcsEpsgFldMap [] =
{
	{               epsgTblAlias,      epsgFldAliasCode,               0,  L"ALIAS CODE"                  },
	{               epsgTblAlias,      epsgFldObjectType,              1,  L"OBJECT_TABLE_NAME"           },
	{               epsgTblAlias,      epsgFldObjectCode,              2,  L"OBJECT_CODE"                 },
	{               epsgTblAlias,      epsgFldCodeNamingSystem,        3,  L"NAMING_SYSTEM_CODE"          },
	{               epsgTblAlias,      epsgFldAlias,                   4,  L"ALIAS"                       },
	{               epsgTblAlias,      epsgFldRemarks,                 5,  L"REMARKS"                     },

	{                epsgTblArea,      epsgFldAreaCode,                0,  L"AREA_CODE"                   },
	{                epsgTblArea,      epsgFldAreaName,                1,  L"AREA_NAME"                   },
	{                epsgTblArea,      epsgFldAreaOfUse,               2,  L"AREA_OF_USE"                 },
	{                epsgTblArea,      epsgFldAreaSouthBoundLat,       3,  L"AREA_SOUTH_BOUND_LAT"        },
	{                epsgTblArea,      epsgFldAreaNorthBoundLat,       4,  L"AREA_NORTH_BOUND_LAT"        },
	{                epsgTblArea,      epsgFldAreaWestBoundLng,        5,  L"AREA_WEST_BOUND_LON"         },
	{                epsgTblArea,      epsgFldAreaEastBoundLng,        6,  L"AREA_EAST_BOUND_LON"         },
	{                epsgTblArea,      epsgFldBoundingPolygonFileName, 7,  L"AREA_POLYGON_FILE_REF"       },
	{                epsgTblArea,      epsgFldIsoA2Code,               8,  L"ISO_A2_CODE"                 },
	{                epsgTblArea,      epsgFldIsoA3Code,               9,  L"ISO_A3_CODE"                 },
	{                epsgTblArea,      epsgFldIsoNumericCode,         10,  L"ISO_N_CODE"                  },
	{                epsgTblArea,      epsgFldRemarks,                11,  L"REMARKS"                     },
	{                epsgTblArea,      epsgFldInformationSource,      12,  L"INFORMATION_SOURCE"          },
	{                epsgTblArea,      epsgFldDataSource,             13,  L"DATA_SOURCE"                 },
	{                epsgTblArea,      epsgFldRevisionDate,           14,  L"REVISION_DATE"               },
	{                epsgTblArea,      epsgFldChangeId,               15,  L"CHANGE_ID"                   },
	{                epsgTblArea,      epsgFldDeprecated,             16,  L"DEPRECATED"                  },

	{              epsgTblChange,      epsgFldChangeId,                0,  L"CHANGE_ID"                   },
	{              epsgTblChange,      epsgFldReportDate,              1,  L"REPORT_DATE"                 },
	{              epsgTblChange,      epsgFldDateClosed,              2,  L"DATE_CLOSED"                 },
	{              epsgTblChange,      epsgFldReporter,                3,  L"REPORTER"                    },
	{              epsgTblChange,      epsgFldRequest,                 4,  L"REQUEST"                     },
	{              epsgTblChange,      epsgFldTablesAffected,          5,  L"TABLES_AFFECTED"             },
	{              epsgTblChange,      epsgFldCodesAffected,           6,  L"CODES_AFFECTED"              },
	{              epsgTblChange,      epsgFldComment,                 7,  L"COMMENT"                     },
	{              epsgTblChange,      epsgFldAction,                  8,  L"ACTION"                      },

	{            epsgTblAxisName,      epsgFldCoordAxisNameCode,       0,  L"COORD_AXIS_NAME_CODE"        },
	{            epsgTblAxisName,      epsgFldCoordAxisName,           1,  L"COORD_AXIS_NAME"             },
	{            epsgTblAxisName,      epsgFldDescription,             2,  L"DESCRIPTION"                 },
	{            epsgTblAxisName,      epsgFldRemarks,                 3,  L"REMARKS"                     },
	{            epsgTblAxisName,      epsgFldInformationSource,       4,  L"INFORMATION_SOURCE"          },
	{            epsgTblAxisName,      epsgFldDataSource,              5,  L"DATA_SOURCE"                 },
	{            epsgTblAxisName,      epsgFldRevisionDate,            6,  L"REVISION_DATE"               },
	{            epsgTblAxisName,      epsgFldChangeId,                7,  L"CHANGE_ID"                   },
	{            epsgTblAxisName,      epsgFldDeprecated,              8,  L"DEPRECATED"                  },

	{                epsgTblAxis,      epsgFldCoordAxisCode,           0,  L"COORD_AXIS_CODE"             },
	{                epsgTblAxis,      epsgFldCoordSysCode,            1,  L"COORD_SYS_CODE"              },
	{                epsgTblAxis,      epsgFldCoordAxisNameCode,       2,  L"COORD_AXIS_NAME_CODE"        },
	{                epsgTblAxis,      epsgFldCoordAxisOrientation,    3,  L"COORD_AXIS_ORIENTATION"      },
	{                epsgTblAxis,      epsgFldCoordAxisAbbreviation,   4,  L"COORD_AXIS_ABBREVIATION"     },
	{                epsgTblAxis,      epsgFldUomCode,                 5,  L"UOM_CODE"                    },
	{                epsgTblAxis,      epsgFldOrder,                   6,  L"ORDER"                       },

	{     epsgTblReferenceSystem,      epsgFldCoordRefSysCode,         0,  L"COORD_REF_SYS_CODE"          },
	{     epsgTblReferenceSystem,      epsgFldCoordRefSysName,         1,  L"COORD_REF_SYS_NAME"          },
	{     epsgTblReferenceSystem,      epsgFldAreaOfUseCode,           2,  L"AREA_OF_USE_CODE"            },
	{     epsgTblReferenceSystem,      epsgFldCoordRefSysKind,         3,  L"COORD_REF_SYS_KIND"          },
	{     epsgTblReferenceSystem,      epsgFldCoordSysCode,            4,  L"COORD_SYS_CODE"              },
	{     epsgTblReferenceSystem,      epsgFldDatumCode,               5,  L"DATUM_CODE"                  },
	{     epsgTblReferenceSystem,      epsgFldSourceGeogCrsCode,       6,  L"SOURCE_GEOGCRS_CODE"         },
	{     epsgTblReferenceSystem,      epsgFldProjectionConvCode,      7,  L"PROJECTION_CONV_CODE"        },
	{     epsgTblReferenceSystem,      epsgFldCmpdHorizCrsCode,        8,  L"CMPD_HORIZCRS_CODE"          },
	{     epsgTblReferenceSystem,      epsgFldCmpdVertCrsCode,         9,  L"CMPD_VERTCS_CODE"            },
	{     epsgTblReferenceSystem,      epsgFldCrsScope,               10,  L"CRS_SCOPE"                   },
	{     epsgTblReferenceSystem,      epsgFldRemarks,                11,  L"REMARKS"                     },
	{     epsgTblReferenceSystem,      epsgFldInformationSource,      12,  L"INFORMATION_SOURCE"          },
	{     epsgTblReferenceSystem,      epsgFldDataSource,             13,  L"DATA_SOURCE"                 },
	{     epsgTblReferenceSystem,      epsgFldRevisionDate,           14,  L"REVISION_DATE"               },
	{     epsgTblReferenceSystem,      epsgFldChangeId,               15,  L"CHANGE_ID"                   },
	{     epsgTblReferenceSystem,      epsgFldShowCrs,                16,  L"SHOW_CRS"                    },
	{     epsgTblReferenceSystem,      epsgFldDeprecated,             17,  L"DEPRECATED"                  },

	{    epsgTblCoordinateSystem,      epsgFldCoordSysCode,            0,  L"COORD_SYS_CODE"              },
	{    epsgTblCoordinateSystem,      epsgFldCoordSysName,            1,  L"COORD_SYS_NAME"              },
	{    epsgTblCoordinateSystem,      epsgFldCoordSysType,            2,  L"COORD_SYS_TYPE"              },
	{    epsgTblCoordinateSystem,      epsgFldDimension,               3,  L"DIMENSION"                   },
	{    epsgTblCoordinateSystem,      epsgFldRemarks,                 4,  L"REMARKS"                     },
	{    epsgTblCoordinateSystem,      epsgFldInformationSource,       5,  L"INFORMATION_SOURCE"          },
	{    epsgTblCoordinateSystem,      epsgFldDataSource,              6,  L"DATA_SOURCE"                 },
	{    epsgTblCoordinateSystem,      epsgFldRevisionDate,            7,  L"REVISION_DATE"               },
	{    epsgTblCoordinateSystem,      epsgFldChangeId,                8,  L"CHANGE_ID"                   },
	{    epsgTblCoordinateSystem,      epsgFldDeprecated,              9,  L"DEPRECATED"                  },

	{     epsgTblOperationMethod,      epsgFldCoordOpMethodCode,       0,  L"COORD_OP_METH_CODE"          },
	{     epsgTblOperationMethod,      epsgFldCoordOpMethodName,       1,  L"COORD_OP_METH_NAME"          },
	{     epsgTblOperationMethod,      epsgFldReverseOp,               2,  L"REVERSE_OP"                  },
	{     epsgTblOperationMethod,      epsgFldFormula,                 3,  L"FORMULA"                     },
	{     epsgTblOperationMethod,      epsgFldExample,                 4,  L"EXAMPLE"                     },
	{     epsgTblOperationMethod,      epsgFldRemarks,                 5,  L"REMARKS"                     },
	{     epsgTblOperationMethod,      epsgFldInformationSource,       6,  L"INFORMATION_SOURCE"          },
	{     epsgTblOperationMethod,      epsgFldDataSource,              7,  L"DATA_SOURCE"                 },
	{     epsgTblOperationMethod,      epsgFldRevisionDate,            8,  L"REVISION_DATE"               },
	{     epsgTblOperationMethod,      epsgFldChangeId,                9,  L"CHANGE_ID"                   },
	{     epsgTblOperationMethod,      epsgFldDeprecated,             10,  L"DEPRECATED"                  },

	{      epsgTblParameterUsage,      epsgFldCoordOpMethodCode,       0,  L"COORD_OP_METHOD_CODE"        },
	{      epsgTblParameterUsage,      epsgFldParameterCode,           1,  L"PARAMETER_CODE"              },
	{      epsgTblParameterUsage,      epsgFldSortOrder,               2,  L"SORT_ORDER"                  },
	{      epsgTblParameterUsage,      epsgFldParamSignReversal,       3,  L"PARAM_SIGN_REVERSAL"         },
	
	{      epsgTblParameterValue,      epsgFldCoordOpCode,             0,  L"COORD_OP_CODE"               },
	{      epsgTblParameterValue,      epsgFldCoordOpMethodCode,       1,  L"COORD_OP_METHOD_CODE"        },
	{      epsgTblParameterValue,      epsgFldParameterCode,           2,  L"PARAMETER_CDOE"              },
	{      epsgTblParameterValue,      epsgFldParameterValue,          3,  L"PARAMETER_VALUE"             },
	{      epsgTblParameterValue,      epsgFldParamValueFileRef,       4,  L"PARAM_VALUE_FILE_REF"        },
	{      epsgTblParameterValue,      epsgFldUomCode,                 5,  L"UOM_CODE"                    },

	{           epsgTblParameter,      epsgFldParameterCode,           0,  L"PARAMETER_CODE"              },
	{           epsgTblParameter,      epsgFldParameterName,           1,  L"PARAMETER_NAME"              },
	{           epsgTblParameter,      epsgFldDescription,             2,  L"DESCRIPTION"                 },
	{           epsgTblParameter,      epsgFldInformationSource,       3,  L"INFORMATION_SOURCE"          },
	{           epsgTblParameter,      epsgFldDataSource,              4,  L"DATA_SOURCE"                 },
	{           epsgTblParameter,      epsgFldRevisionDate,            5,  L"REVISION_DATE"               },
	{           epsgTblParameter,      epsgFldChangeId,                6,  L"CHANGE_ID"                   },
	{           epsgTblParameter,      epsgFldDeprecated,              7,  L"DEPRECATED"                  },

	{       epsgTblOperationPath,      epsgFldConcatOperationCode,     0,  L"CONCAT_OPERATION_CODE"       },
	{       epsgTblOperationPath,      epsgFldSingleOperationCode,     1,  L"SINGLE_OPERATION_CODE"       },
	{       epsgTblOperationPath,      epsgFldOpPathStep,              2,  L"OP_PATH_STEP"                },

	{ epsgTblCoordinateOperation,      epsgFldCoordOpCode,             0,  L"COORD_OP_CODE"               },
	{ epsgTblCoordinateOperation,      epsgFldCoordOpName,             1,  L"COORD_OP_NAME"               },
	{ epsgTblCoordinateOperation,      epsgFldCoordOpType,             2,  L"COORD_OP_TYPE"               },
	{ epsgTblCoordinateOperation,      epsgFldSourceCrsCode,           3,  L"SOURCE_CRS_CODE"             },
	{ epsgTblCoordinateOperation,      epsgFldTargetCrsCode,           4,  L"TARGET_CRS_CODE"             },
	{ epsgTblCoordinateOperation,      epsgFldCoordTfmVersion,         5,  L"COORD_TFM_VERSION"           },
	{ epsgTblCoordinateOperation,      epsgFldCoordOpVariant,          6,  L"COORD_OP_VARIANT"            },
	{ epsgTblCoordinateOperation,      epsgFldAreaOfUseCode,           7,  L"AREA_OF_USE_CODE"            },
	{ epsgTblCoordinateOperation,      epsgFldCoordOpScope,            8,  L"COORD_OP_SCOPE"              },
	{ epsgTblCoordinateOperation,      epsgFldCoordOpAccuracy,         9,  L"COORD_OP_ACCURACY"           },
	{ epsgTblCoordinateOperation,      epsgFldCoordOpMethodCode,      10,  L"COORD_OP_METHOD_CODE"        },
	{ epsgTblCoordinateOperation,      epsgFldUomCodeSourceCoordDiff, 11,  L"UOM_CODE_SOURCE_COORD_DIFF"  },
	{ epsgTblCoordinateOperation,      epsgFldUomCodeTargetCoordDiff, 12,  L"UOM_CODE_TARGET_COORD_DIFF"  },
	{ epsgTblCoordinateOperation,      epsgFldRemarks,                13,  L"REMARKS"                     },
	{ epsgTblCoordinateOperation,      epsgFldInformationSource,      14,  L"INFORMATION_SOURCE"          },
	{ epsgTblCoordinateOperation,      epsgFldDataSource,             15,  L"DATA_SOURCE"                 },
	{ epsgTblCoordinateOperation,      epsgFldRevisionDate,           16,  L"REVISION_DATE"               },
	{ epsgTblCoordinateOperation,      epsgFldChangeId,               17,  L"CHANGE_ID"                   },
	{ epsgTblCoordinateOperation,      epsgFldShowOperation,          18,  L"SHOW_OPERATION"              },
	{ epsgTblCoordinateOperation,      epsgFldDeprecated,             19,  L"DEPRECATED"                  },

	{               epsgTblDatum,      epsgFldDatumCode,               0,  L"DATUM_CODE"                  },
	{               epsgTblDatum,      epsgFldDatumName,               1,  L"DATUM_NAME"                  },
	{               epsgTblDatum,      epsgFldDatumType,               2,  L"DATUM_TYPE"                  },
	{               epsgTblDatum,      epsgFldOriginDescription,       3,  L"ORIGIN_DESCRIPTION"          },
	{               epsgTblDatum,      epsgFldRealizationEpoch,        4,  L"REALIZATION_EPOCH"           },
	{               epsgTblDatum,      epsgFldEllipsoidCode,           5,  L"ELLIPSOID_CODE"              },
	{               epsgTblDatum,      epsgFldPrimeMeridianCode,       6,  L"PRIME_MERIDIAN_CODE"         },
	{               epsgTblDatum,      epsgFldAreaOfUseCode,           7,  L"AREA_OF_USE_CODE"            },
	{               epsgTblDatum,      epsgFldDatumScope,              8,  L"DATUM_SCOPE"                 },
	{               epsgTblDatum,      epsgFldRemarks,                 9,  L"REMARKS"                     },
	{               epsgTblDatum,      epsgFldInformationSource,      10,  L"INFORMATION_SOURCE"          },
	{               epsgTblDatum,      epsgFldDataSource,             11,  L"DATA_SOURCE"                 },
	{               epsgTblDatum,      epsgFldRevisionDate,           12,  L"REVISION_DATE"               },
	{               epsgTblDatum,      epsgFldChangeId,               13,  L"CHANGE_ID"                   },
	{               epsgTblDatum,      epsgFldDeprecated,             14,  L"DEPRECATED"                  },

	{         epsgTblDeprecation,      epsgFldDeprecationId,           0,  L"DEPRECATION_ID"              },
	{         epsgTblDeprecation,      epsgFldDeprecationDate,         1,  L"DEPRECATION_DATE"            },
	{         epsgTblDeprecation,      epsgFldChangeId,                2,  L"CHANGE_ID"                   },
	{         epsgTblDeprecation,      epsgFldObjectTableName,         3,  L"OBJECT_TABLE_NAME"           },
	{         epsgTblDeprecation,      epsgFldObjectCode,              4,  L"OBJECT_CODE"                 },
	{         epsgTblDeprecation,      epsgFldReplacedBy,              5,  L"REPLACED_BY"                 },
	{         epsgTblDeprecation,      epsgFldDeprecationReason,       6,  L"DEPRECATION_REASON"          },

	{           epsgTblEllipsoid,      epsgFldEllipsoidCode,           0,  L"ELLIPSOID_CODE"              },
	{           epsgTblEllipsoid,      epsgFldEllipsoidName,           1,  L"ELLIPSOID_NAME"              },
	{           epsgTblEllipsoid,      epsgFldSemiMajorAxis,           2,  L"SEMI_MAJOR_AXIS"             },
	{           epsgTblEllipsoid,      epsgFldUomCode,                 3,  L"UOM_CODE"                    },
	{           epsgTblEllipsoid,      epsgFldInvFlattening,           4,  L"INV_FLATTENING"              },
	{           epsgTblEllipsoid,      epsgFldSemiMinorAxis,           5,  L"SEMI_MINOR_AXIS"             },
	{           epsgTblEllipsoid,      epsgFldEllipsoidShape,          6,  L"ELLIPSOID_SHAPE"             },
	{           epsgTblEllipsoid,      epsgFldRemarks,                 7,  L"REMARKS"                     },
	{           epsgTblEllipsoid,      epsgFldInformationSource,       8,  L"INFORMATION_SOURCE"          },
	{           epsgTblEllipsoid,      epsgFldDataSource,              9,  L"DATA_SOURCE"                 },
	{           epsgTblEllipsoid,      epsgFldRevisionDate,           10,  L"REVISION_DATE"               },
	{           epsgTblEllipsoid,      epsgFldChangeId,               11,  L"CHANGE_ID"                   },
	{           epsgTblEllipsoid,      epsgFldDeprecated,             12,  L"DEPRECATED"                  },

	{        epsgTblNamingSystem,      epsgFldNamingSystemCode,        0,  L"NAMING_SYSTEM_CODE"          },
	{        epsgTblNamingSystem,      epsgFldNamingSystemName,        1,  L"NAMING_SYSTEM_NAME"          },
	{        epsgTblNamingSystem,      epsgFldRemarks,                 2,  L"REMARKS"                     },
	{        epsgTblNamingSystem,      epsgFldInformationSource,       3,  L"INFORMATION_SOURCE"          },
	{        epsgTblNamingSystem,      epsgFldDataSource,              4,  L"DATA_SOURCE"                 },
	{        epsgTblNamingSystem,      epsgFldRevisionDate,            5,  L"REVISION_DATE"               },
	{        epsgTblNamingSystem,      epsgFldChangeId,                6,  L"CHANGE_ID"                   },
	{        epsgTblNamingSystem,      epsgFldDeprecated,              7,  L"DEPRECATED"                  },

	{       epsgTblPrimeMeridian,      epsgFldPrimeMeridianCode,       0,  L"PRIME_MERIDIAN_CODE"         },
	{       epsgTblPrimeMeridian,      epsgFldPrimeMeridianName,       1,  L"PRIME_MERIDIAN_NAME"         },
	{       epsgTblPrimeMeridian,      epsgFldGreenwichLongitude,      2,  L"GREENWICH_LONGITUDE"         },
	{       epsgTblPrimeMeridian,      epsgFldUomCode,                 3,  L"UOM_CODE"                    },
	{       epsgTblPrimeMeridian,      epsgFldRemarks,                 4,  L"REMARKS"                     },
	{       epsgTblPrimeMeridian,      epsgFldInformationSource,       5,  L"INFORMATION_SOURCE"          },
	{       epsgTblPrimeMeridian,      epsgFldDataSource,              6,  L"DATA_SOURCE"                 },
	{       epsgTblPrimeMeridian,      epsgFldRevisionDate,            7,  L"REVISION_DATE"               },
	{       epsgTblPrimeMeridian,      epsgFldChangeId,                8,  L"CHANGE_ID"                   },
	{       epsgTblPrimeMeridian,      epsgFldDeprecated,              9,  L"DEPRECATED"                  },

	{        epsgTblSupercession,      epsgFldSupersessionId,          0,  L"SUPERSESSION_ID"             },
	{        epsgTblSupercession,      epsgFldObjectTableName,         1,  L"OBJECT_TABLE_NAME"           },
	{        epsgTblSupercession,      epsgFldObjectCode,              2,  L"OBJECT_CODE"                 },
	{        epsgTblSupercession,      epsgFldSupercededBy,            3,  L"SUPERSEDED_BY"               },
	{        epsgTblSupercession,      epsgFldSupersessionType,        4,  L"SUPERSESSION_TYPE"           },
	{        epsgTblSupercession,      epsgFldSupersessionYear,        5,  L"SUPERSESSION_YEAR"           },
	{        epsgTblSupercession,      epsgFldRemarks,                 6,  L"REMARKS"                     },

	{       epsgTblUnitOfMeasure,      epsgFldUomCode,                 0,  L"UOM_CODE"                    },
	{       epsgTblUnitOfMeasure,      epsgFldUnitOfMeasName,          1,  L"UNIT_OF_MEAS_NAME"           },
	{       epsgTblUnitOfMeasure,      epsgFldUnitOfMeasType,          2,  L"UNIT_OF_MEAS_TYPE"           },
	{       epsgTblUnitOfMeasure,      epsgFldTargetUomCode,           3,  L"TRAGET_UOM_CODE"             },
	{       epsgTblUnitOfMeasure,      epsgFldFactorB,                 4,  L"FACTOR_B"                    },
	{       epsgTblUnitOfMeasure,      epsgFldFactorC,                 5,  L"FACTOR_C"                    },
	{       epsgTblUnitOfMeasure,      epsgFldRemarks,                 6,  L"REMARKS"                     },
	{       epsgTblUnitOfMeasure,      epsgFldInformationSource,       7,  L"INFORMATION_SOURCE"          },
	{       epsgTblUnitOfMeasure,      epsgFldDataSource,              8,  L"DATA_SOURCE"                 },
	{       epsgTblUnitOfMeasure,      epsgFldRevisionDate,            9,  L"REVISION_DATE"               },
	{       epsgTblUnitOfMeasure,      epsgFldChangeId,               10,  L"CHANGE_ID"                   },
	{       epsgTblUnitOfMeasure,      epsgFldDeprecated,             11,  L"DEPRECATED"                  },

	{      epsgTblVersionHistory,      epsgFldVersionHistoryCode,      0,  L"VERSION_HISTORY_CODE"        },
	{      epsgTblVersionHistory,      epsgFldVersionDate,             1,  L"VERSION_DATE"                },
	{      epsgTblVersionHistory,      epsgFldVersionNumber,           2,  L"VERSION_NUMBER"              },
	{      epsgTblVersionHistory,      epsgFldRemarks,                 3,  L"VERSION_REMARKS"             },
	{      epsgTblVersionHistory,      epsgFldSupercededBy,            4,  L"SUPERCEDED_BY"               },
	{      epsgTblVersionHistory,      epsgFldSupercedes,              5,  L"SUPERCEDES"                  },

	{             epsgTblUnknown,      epsgFldUnknown,                 0,  L"<unknown table>"             }
};

const TcsEpsgCsysTypeMap KcsEpsgCsysTypeMap [] =
{
	{        epsgCsysTypAffine,       L"affine"               },
	{     epsgCsysTypCartesian,       L"Cartesian"            },
	{   epsgCsysTypCylindrical,       L"cylindrical"          },
	{   epsgCsysTypEllipsoidal,       L"ellipsoidal"          },
	{        epsgCsysTypLinear,       L"linear"               },
	{         epsgCsysTypPolar,       L"polar"                },
	{     epsgCsysTypSpherical,       L"spherical"            },
	{      epsgCsysTypVertical,       L"vertical"             },
	{       epsgCsysTypUnknown,       L"<unknown CSys type>"  }
};

const TcsEpsgCrsTypeMap KcsEpsgCrsTypeMap [] =
{
	{       epsgCrsTypCompund,       L"compound"            },
	{   epsgCrsTypEngineering,       L"engineering"         },
	{    epsgCrsTypGeocentric,       L"geocentric"          },
	{  epsgCrsTypGeographic2D,       L"geographic 2D"       },
	{  epsgCrsTypGeographic3D,       L"geographic 3D"       },
	{     epsgCrsTypProjected,       L"projected"           },
	{      epsgCrsTypVertical,       L"vertical"            },
	{       epsgCrsTypUnknown,       L"<unknown CRS type>"  }
};

const TcsEpsgOpTypeMap KcsEpsgOpTypeMap [] =
{
	{         epsgOpTypConversion,   L"conversion"               },
	{     epsgOpTypTransformation,   L"transformation"           },
	{       epsgOpTypConcatenated,   L"concatenated operation"   },
	{            epsgOpTypUnknown,   L"<unknown op type>"        }
};

const TcsEpsgDtmTypeMap KcsEpsgDtmTypeMap [] =
{
	{     epsgDtmTypGeodetic,     L"geodetic"            },
	{     epsgDtmTypVertical,     L"vertical"            },
	{  epsgDtmTypEngineering,     L"engineering"         },
	{      epsgDtmTypUnknown,     L"<unknown dtm type>"  }
};

const TcsEpsgUomTypeMap KcsEpsgUomTypeMap [] =
{
	{     epsgUomTypLinear,     L"Length"              },
	{    epsgUomTypAngular,     L"Angle"               },
	{      epsgUomTypScale,     L"Scale"               },
	{    epsgUomTypUnknown,     L"<unknown UOM type>"  }
};

const TcsEpsgOrntTypeMap KcsEpsgOrntTypeMap [] =
{
	{         epsgOrntEast,     L"east"                },
	{         epsgOrntWest,     L"west"                },
	{        epsgOrntNorth,     L"north"               },
	{        epsgOrntSouth,     L"south"               },
	{           epsgOrntUp,     L"up"                  },
	{         epsgOrntDown,     L"down"                },
	{      epsgOrntUnknown,     L""                    }
};

const struct TcmAreaToMsiGroupMap
{
	short epsgAreaFrom;
	short epsgAreaTo;
	char msiGroup [16];
} KcmAreaToMsiGroupMap [] =
{
	{  1024,  1024,  "ASIA"      },
	{  1025,  1025,  "EUROPE"    },
	{  1026,  1026,  "AFRICA"    },
	{  1027,  1027,  "PACIFIC"   },
	{  1028,  1028,  "EUROPE"    },
	{  1029,  1029,  "AFRICA"    },
	{  1030,  1030,  "CARIB"     },
	{  1031,  1031,  "POLAR"     },
	{  1032,  1032,  "CARIB"     },
	{  1033,  1033,  "SAMER"     },
	{  1034,  1034,  "ASIA"      },
	{  1035,  1035,  "CARIB"     },
	{  1036,  1036,  "AUSNZ"     },
	{  1037,  1037,  "EUROPE"    },
	{  1038,  1038,  "ASIA"      },
	{  1039,  1039,  "CARIB"     },
	{  1040,  1040,  "MIDEAST"   },
	{  1041,  1041,  "ASIA"      },
	{  1042,  1042,  "CARIB"     },
	{  1043,  1044,  "EUROPE"    },
	{  1045,  1045,  "CAMER"     },
	{  1046,  1046,  "AFRICA"    },
	{  1047,  1047,  "ATLANTIC"  },
	{  1048,  1048,  "ASIA"      },
	{  1049,  1049,  "SAMER"     },
	{  1050,  1050,  "EUROPE"    },
	{  1051,  1051,  "AFRICA"    },
	{  1052,  1052,  "ATLANTIC"  },
	{  1053,  1053,  "SAMER"     },
	{  1054,  1054,  "INDIAN"    },
	{  1055,  1055,  "ASIA"      },
	{  1056,  1056,  "EUROPE"    },
	{  1057,  1058,  "AFRICA"    },
	{  1059,  1059,  "ASIA"      },
	{  1060,  1060,  "AFRICA"    },
	{  1061,  1061,  "CANADA"    },
	{  1062,  1062,  "ATLANTIC"  },
	{  1063,  1063,  "CARIB"     },
	{  1064,  1065,  "AFRICA"    },
	{  1066,  1066,  "SAMER"     },
	{  1067,  1067,  "ASIA"      },
	{  1068,  1068,  "PACIFIC"   },
	{  1069,  1069,  "INDIAN"    },
	{  1070,  1070,  "SAMER"     },
	{  1071,  1071,  "INDIAN"    },
	{  1072,  1072,  "AFRICA"    },
	{  1073,  1073,  "PACIFIC"   },
	{  1074,  1074,  "CAMER"     },
	{  1075,  1075,  "AFRICA"    },
	{  1076,  1076,  "EUROPE"    },
	{  1077,  1077,  "CARIB"     },
	{  1078,  1080,  "EUROPE"    },
	{  1081,  1081,  "AFRICA"    },
	{  1082,  1083,  "CARIB"     },
	{  1084,  1084,  "ASIA"      },
	{  1085,  1085,  "SAMER"     },
	{  1086,  1086,  "AFRICA"    },
	{  1087,  1087,  "CAMER"     },
	{  1088,  1089,  "AFRICA"    },
	{  1090,  1090,  "EUROPE"    },
	{  1091,  1091,  "AFRICA"    },
	{  1092,  1092,  "SAMER"     },
	{  1093,  1093,  "EUROPE"    },
	{  1094,  1094,  "PACIFIC"   },
	{  1095,  1096,  "EUROPE"    },
	{  1097,  1097,  "AFRICA"    },
	{  1098,  1099,  "PACIFIC"   },
	{  1100,  1101,  "AFRICA"    },
	{  1102,  1102,  "ASIA"      },
	{  1103,  1103,  "EUROPE"    },
	{  1104,  1104,  "AFRICA"    },
	{  1105,  1106,  "EUROPE"    },
	{  1107,  1107,  "ATLANTIC"  },
	{  1108,  1109,  "CARIB"     },
	{  1110,  1110,  "PACIFIC"   },
	{  1111,  1111,  "CAMER"     },
	{  1112,  1113,  "AFRICA"    },
	{  1114,  1114,  "SAMER"     },
	{  1115,  1115,  "CARIB"     },
	{  1116,  1116,  "INDIAN"    },
	{  1117,  1117,  "CAMER"     },
	{  1118,  1118,  "ASIA"      },
	{  1119,  1120,  "EUROPE"    },
	{  1121,  1122,  "ASIA"      },
	{  1123,  1124,  "MIDEAST"   },
	{  1125,  1125,  "EUROPE"    },
	{  1126,  1126,  "MIDEAST"   },
	{  1127,  1127,  "EUROPE"    },
	{  1128,  1128,  "CARIB"     },
	{  1129,  1129,  "ASIA"      },
	{  1130,  1130,  "MIDEAST"   },
	{  1131,  1131,  "ASIA"      },
	{  1132,  1132,  "AFRICA"    },
	{  1133,  1133,  "PACIFIC"   },
	{  1134,  1135,  "ASIA"      },
	{  1136,  1136,  "MIDEAST"   },
	{  1137,  1138,  "ASIA"      },
	{  1139,  1139,  "EUROPE"    },
	{  1140,  1140,  "MIDEAST"   },
	{  1141,  1143,  "AFRICA"    },
	{  1144,  1146,  "EUROPE"    },
	{  1147,  1147,  "ASIA"      },
	{  1148,  1148,  "EUROPE"    },
	{  1149,  1149,  "INDIAN"    },
	{  1150,  1150,  "AFRICA"    },
	{  1151,  1151,  "ASIA"      },
	{  1152,  1152,  "INDIAN"    },
	{  1153,  1153,  "AFRICA"    },
	{  1154,  1154,  "EUROPE"    },
	{  1155,  1155,  "PACIFIC"   },
	{  1156,  1156,  "CARIB"     },
	{  1157,  1159,  "AFRICA"    },
	{  1160,  1160,  "CAMER"     },
	{  1161,  1161,  "PACIFIC"   },
	{  1162,  1163,  "EUROPE"    },
	{  1164,  1164,  "ASIA"      },
	{  1165,  1165,  "CARIB"     },
	{  1166,  1166,  "EUROPE"    },
	{  1167,  1167,  "AFRICA"    },
	{  1168,  1168,  "ASIA"      },
	{  1169,  1169,  "AFRICA"    },
	{  1170,  1170,  "PACIFIC"   },
	{  1171,  1171,  "ASIA"      },
	{  1172,  1172,  "EUROPE"    },
	{  1173,  1173,  "CARIB"     },
	{  1174,  1174,  "PACIFIC"   },
	{  1175,  1175,  "AUSNZ"     },
	{  1176,  1176,  "CAMER"     },
	{  1177,  1178,  "AFRICA"    },
	{  1179,  1179,  "NONE"      },
	{  1180,  1180,  "AUSNZ"     },
	{  1181,  1181,  "PACIFIC"   },
	{  1182,  1182,  "EUROPE"    },
	{  1183,  1183,  "MIDEAST"   },
	{  1184,  1184,  "ASIA"      },
	{  1185,  1185,  "PACIFIC"   },
	{  1186,  1186,  "CAMER"     },
	{  1187,  1187,  "PACIFIC"   },
	{  1188,  1189,  "SAMER"     },
	{  1190,  1191,  "PACIFIC"   },
	{  1192,  1193,  "EUROPE"    },
	{  1194,  1194,  "CARIB"     },
	{  1195,  1195,  "MIDEAST"   },
	{  1196,  1196,  "INDIAN"    },
	{  1197,  1198,  "EUROPE"    },
	{  1199,  1199,  "AFRICA"    },
	{  1200,  1202,  "CARIB"     },
	{  1203,  1203,  "PACIFIC"   },
	{  1204,  1204,  "EUROPE"    },
	{  1205,  1205,  "AFRICA"    },
	{  1206,  1206,  "MIDEAST"   },
	{  1207,  1207,  "AFRICA"    },
	{  1208,  1208,  "INDIAN"    },
	{  1209,  1209,  "AFRICA"    },
	{  1210,  1210,  "ASIA"      },
	{  1211,  1212,  "EUROPE"    },
	{  1213,  1213,  "PACIFIC"   },
	{  1214,  1215,  "AFRICA"    },
	{  1216,  1216,  "PACIFIC"   },
	{  1217,  1217,  "EUROPE"    },
	{  1218,  1218,  "ASIA"      },
	{  1219,  1219,  "ATLANTIC"  },
	{  1220,  1220,  "CANADA"    },
	{  1221,  1221,  "AFRICA"    },
	{  1222,  1222,  "SAMER"     },
	{  1223,  1223,  "EUROPE"    },
	{  1224,  1224,  "AFRICA"    },
	{  1225,  1226,  "EUROPE"    },
	{  1227,  1227,  "MIDEAST"   },
	{  1228,  1228,  "PACIFIC"   },
	{  1229,  1229,  "ASIA"      },
	{  1230,  1230,  "AFRICA"    },
	{  1231,  1231,  "ASIA"      },
	{  1232,  1232,  "AFRICA"    },
	{  1233,  1234,  "PACIFIC"   },
	{  1235,  1235,  "CARIB"     },
	{  1236,  1236,  "AFRICA"    },
	{  1237,  1237,  "EUROPE"    },
	{  1238,  1238,  "ASIA"      },
	{  1239,  1239,  "CARIB"     },
	{  1240,  1240,  "PACIFIC"   },
	{  1241,  1241,  "AFRICA"    },
	{  1242,  1242,  "ASIA"      },
	{  1243,  1243,  "MIDEAST"   },
	{  1244,  1244,  "EUROPE"    },
	{  1245,  1246,  "OTHR-US"   },
	{  1247,  1247,  "SAMER"     },
	{  1248,  1248,  "ASIA"      },
	{  1249,  1249,  "PACIFIC"   },
	{  1250,  1250,  "EUROPE"    },
	{  1251,  1251,  "SAMER"     },
	{  1252,  1252,  "ASIA"      },
	{  1253,  1254,  "CARIB"     },
	{  1255,  1255,  "PACIFIC"   },
	{  1256,  1256,  "AFRICA"    },
	{  1257,  1257,  "MIDEAST"   },
	{  1258,  1258,  "EUROPE"    },
	{  1259,  1261,  "AFRICA"    },
	{  1262,  1262,  "WORLD"     },
	{  1263,  1263,  "NONE"      },
	{  1264,  1264,  "EUROPE"    },
	{  1265,  1270,  "SAMER"     },
	{  1271,  1271,  "AFRICA"    },
	{  1272,  1272,  "ASIA"      },
	{  1273,  1273,  "CARIB"     },
	{  1274,  1274,  "SAMER"     },
	{  1275,  1275,  "EUROPE"    },
	{  1276,  1277,  "AFRICA"    },
	{  1278,  1278,  "POLAR"     },
	{  1279,  1282,  "AUSNZ"     },
	{  1283,  1283,  "CANADA"    },
	{  1284,  1284,  "EUROPE"    },
	{  1285,  1285,  "ASIA"      },
	{  1286,  1286,  "EUROPE"    },
	{  1287,  1287,  "ASIA"      },
	{  1288,  1288,  "AFRICA"    },
	{  1289,  1289,  "CANADA"    },
	{  1290,  1290,  "AFRICA"    },
	{  1291,  1291,  "ASIA"      },
	{  1292,  1293,  "SAMER"     },
	{  1294,  1299,  "EUROPE"    },
	{  1300,  1300,  "MIDEAST"   },
	{  1301,  1301,  "EUROPE"    },
	{  1302,  1302,  "ASIA"      },
	{  1303,  1303,  "SAMER"     },
	{  1304,  1304,  "ASIA"      },
	{  1305,  1306,  "EUROPE"    },
	{  1307,  1309,  "ASIA"      },
	{  1310,  1310,  "MIDEAST"   },
	{  1311,  1313,  "SAMER"     },
	{  1314,  1314,  "EUROPE"    },
	{  1315,  1315,  "AFRICA"    },
	{  1316,  1316,  "ASIA"      },
	{  1317,  1318,  "AFRICA"    },
	{  1319,  1320,  "SAMER"     },
	{  1321,  1321,  "EUROPE"    },
	{  1322,  1322,  "CARIB"     },
	{  1323,  1324,  "OTHR-US"   },
	{  1325,  1325,  "OTHR-NA"   },
	{  1326,  1327,  "EUROPE"    },
	{  1328,  1328,  "ASIA"      },
	{  1329,  1329,  "AFRICA"    },
	{  1330,  1334,  "OTHR-US"   },
	{  1335,  1335,  "CARIB"     },
	{  1336,  1336,  "CANADA"    },
	{  1337,  1337,  "OTHR-US"   },
	{  1338,  1338,  "MIDEAST"   },
	{  1339,  1339,  "CARIB"     },
	{  1340,  1340,  "MIDEAST"   },
	{  1341,  1341,  "SAMER"     },
	{  1342,  1342,  "AFRICA"    },
	{  1343,  1345,  "EUROPE"    },
	{  1346,  1346,  "MIDEAST"   },
	{  1347,  1347,  "EUROPE"    },
	{  1348,  1348,  "SAMER"     },
	{  1349,  1350,  "OTHR-NA"   },
	{  1351,  1351,  "MIDEAST"   },
	{  1352,  1354,  "EUROPE"    },
	{  1355,  1355,  "ASIA"      },
	{  1356,  1356,  "MIDEAST"   },
	{  1357,  1357,  "EUROPE"    },
	{  1358,  1358,  "SAMER"     },
	{  1359,  1360,  "ASIA"      },
	{  1361,  1361,  "AFRICA"    },
	{  1362,  1362,  "ASIA"      },
	{  1363,  1363,  "MIDEAST"   },
	{  1364,  1364,  "ASIA"      },
	{  1365,  1366,  "AFRICA"    },
	{  1367,  1368,  "CANADA"    },
	{  1369,  1369,  "EUROPE"    },
	{  1370,  1371,  "SAMER"     },
	{  1372,  1378,  "SPCS"      },
	{  1379,  1379,  "OTHR-US"   },
	{  1380,  1411,  "SPCS"      },
	{  1412,  1412,  "OTHR-US"   },
	{  1413,  1417,  "SPCS"      },
	{  1418,  1418,  "OTHR-US"   },
	{  1419,  1419,  "SPCS"      },
	{  1420,  1449,  "CANADA"    },
	{  1450,  1451,  "AFRICA"    },
	{  1452,  1453,  "ASIA"      },
	{  1454,  1463,  "AFRICA"    },
	{  1464,  1467,  "MIDEAST"   },
	{  1468,  1482,  "AFRICA"    },
	{  1483,  1486,  "SAMER"     },
	{  1487,  1488,  "CARIB"     },
	{  1489,  1489,  "AFRICA"    },
	{  1490,  1493,  "MIDEAST"   },
	{  1494,  1498,  "ASIA"      },
	{  1499,  1499,  "SAMER"     },
	{  1500,  1504,  "AUSNZ"     },
	{  1505,  1505,  "AFRICA"    },
	{  1506,  1508,  "CANADA"    },
	{  1509,  1510,  "AFRICA"    },
	{  1511,  1511,  "OTHR-US"   },
	{  1512,  1530,  "EUROPE"    },
	{  1531,  1535,  "CANADA"    },
	{  1536,  1539,  "EUROPE"    },
	{  1540,  1541,  "AFRICA"    },
	{  1542,  1542,  "ASIA"      },
	{  1544,  1545,  "MIDEAST"   },
	{  1546,  1550,  "OTHR-US"   },
	{  1551,  1551,  "CARIB"     },
	{  1552,  1555,  "AFRICA"    },
	{  1556,  1568,  "AUSNZ"     },
	{  1569,  1571,  "MIDEAST"   },
	{  1572,  1574,  "SAMER"     },
	{  1575,  1583,  "AFRICA"    },
	{  1584,  1597,  "ASIA"      },
	{  1598,  1603,  "SAMER"     },
	{  1604,  1607,  "AFRICA"    },
	{  1608,  1614,  "SAMER"     },
	{  1615,  1620,  "AFRICA"    },
	{  1621,  1622,  "SAMER"     },
	{  1623,  1623,  "MIDEAST"   },
	{  1624,  1641,  "EUROPE"    },
	{  1642,  1645,  "AFRICA"    },
	{  1646,  1646,  "EUROPE"    },
	{  1647,  1692,  "ASIA"      },
	{  1693,  1695,  "SAMER"     },
	{  1696,  1697,  "AFRICA"    },
	{  1698,  1702,  "PACIFIC"   },
	{  1703,  1703,  "AFRICA"    },
	{  1706,  1712,  "EUROPE"    },
	{  1713,  1717,  "AFRICA"    },
	{  1718,  1719,  "EUROPE"    },
	{  1720,  1725,  "OTHR-US"   },
	{  1726,  1726,  "AFRICA"    },
	{  1727,  1727,  "SAMER"     },
	{  1728,  1729,  "AFRICA"    },
	{  1730,  1730,  "OTHR-NA"   },
	{  1731,  1734,  "EUROPE"    },
	{  1735,  1738,  "AFRICA"    },
	{  1739,  1740,  "MIDEAST"   },
	{  1741,  1748,  "EUROPE"    },
	{  1749,  1750,  "MIDEAST"   },
	{  1751,  1762,  "SAMER"     },
	{  1763,  1767,  "EUROPE"    },
	{  1768,  1791,  "ASIA"      },
	{  1792,  1797,  "EUROPE"    },
	{  1798,  1804,  "ASIA"      },
	{  1805,  1805,  "EUROPE"    },
	{  1806,  1821,  "SAMER"     },
	{  1822,  1822,  "AFRICA"    },
	{  1823,  1837,  "SAMER"     },
	{  1838,  1849,  "AFRICA"    },
	{  1850,  1850,  "MIDEAST"   },
	{  1851,  1872,  "ASIA"      },
	{  1873,  1873,  "UTMN"      },
	{  1874,  1874,  "UTMS"      },
	{  1875,  1875,  "UTMN"      },
	{  1876,  1876,  "UTMS"      },
	{  1877,  1877,  "UTMN"      },
	{  1878,  1878,  "UTMS"      },
	{  1879,  1879,  "UTMN"      },
	{  1880,  1880,  "UTMS"      },
	{  1881,  1881,  "UTMN"      },
	{  1882,  1882,  "UTMS"      },
	{  1883,  1883,  "UTMN"      },
	{  1884,  1884,  "UTMS"      },
	{  1885,  1885,  "UTMN"      },
	{  1886,  1886,  "UTMS"      },
	{  1887,  1887,  "UTMN"      },
	{  1888,  1888,  "UTMS"      },
	{  1889,  1889,  "UTMN"      },
	{  1890,  1890,  "UTMS"      },
	{  1891,  1891,  "UTMN"      },
	{  1892,  1892,  "UTMS"      },
	{  1893,  1893,  "UTMN"      },
	{  1894,  1894,  "UTMS"      },
	{  1895,  1895,  "UTMN"      },
	{  1896,  1896,  "UTMS"      },
	{  1897,  1897,  "UTMN"      },
	{  1898,  1898,  "UTMS"      },
	{  1899,  1899,  "UTMN"      },
	{  1900,  1900,  "UTMS"      },
	{  1901,  1901,  "UTMN"      },
	{  1902,  1902,  "UTMS"      },
	{  1903,  1903,  "UTMN"      },
	{  1904,  1904,  "UTMS"      },
	{  1905,  1905,  "UTMN"      },
	{  1906,  1906,  "UTMS"      },
	{  1907,  1907,  "UTMN"      },
	{  1908,  1908,  "UTMS"      },
	{  1909,  1909,  "UTMN"      },
	{  1910,  1910,  "UTMS"      },
	{  1911,  1911,  "UTMN"      },
	{  1912,  1912,  "UTMS"      },
	{  1913,  1913,  "UTMN"      },
	{  1914,  1914,  "UTMS"      },
	{  1915,  1915,  "UTMN"      },
	{  1916,  1916,  "UTMS"      },
	{  1917,  1917,  "UTMN"      },
	{  1918,  1918,  "UTMS"      },
	{  1919,  1919,  "UTMN"      },
	{  1920,  1920,  "UTMS"      },
	{  1921,  1921,  "UTMN"      },
	{  1922,  1922,  "UTMS"      },
	{  1923,  1923,  "UTMN"      },
	{  1924,  1924,  "UTMS"      },
	{  1925,  1925,  "UTMN"      },
	{  1926,  1926,  "UTMS"      },
	{  1927,  1927,  "UTMN"      },
	{  1928,  1928,  "UTMS"      },
	{  1929,  1929,  "UTMN"      },
	{  1930,  1930,  "UTMS"      },
	{  1931,  1931,  "UTMN"      },
	{  1932,  1932,  "UTMS"      },
	{  1933,  1933,  "UTMN"      },
	{  1934,  1934,  "UTMS"      },
	{  1935,  1935,  "UTMN"      },
	{  1936,  1936,  "UTMS"      },
	{  1937,  1937,  "UTMN"      },
	{  1938,  1938,  "UTMS"      },
	{  1939,  1939,  "UTMN"      },
	{  1940,  1940,  "UTMS"      },
	{  1941,  1941,  "UTMN"      },
	{  1942,  1942,  "UTMS"      },
	{  1943,  1943,  "UTMN"      },
	{  1944,  1944,  "UTMS"      },
	{  1945,  1945,  "UTMN"      },
	{  1946,  1946,  "UTMS"      },
	{  1947,  1947,  "UTMN"      },
	{  1948,  1948,  "UTMS"      },
	{  1949,  1949,  "UTMN"      },
	{  1950,  1950,  "UTMS"      },
	{  1951,  1951,  "UTMN"      },
	{  1952,  1952,  "UTMS"      },
	{  1953,  1953,  "UTMN"      },
	{  1954,  1954,  "UTMS"      },
	{  1955,  1955,  "UTMN"      },
	{  1956,  1956,  "UTMS"      },
	{  1957,  1957,  "UTMN"      },
	{  1958,  1958,  "UTMS"      },
	{  1959,  1959,  "UTMN"      },
	{  1960,  1960,  "UTMS"      },
	{  1961,  1961,  "UTMN"      },
	{  1962,  1962,  "UTMS"      },
	{  1963,  1963,  "UTMN"      },
	{  1964,  1964,  "UTMS"      },
	{  1965,  1965,  "UTMN"      },
	{  1966,  1966,  "UTMS"      },
	{  1967,  1967,  "UTMN"      },
	{  1968,  1968,  "UTMS"      },
	{  1969,  1969,  "UTMN"      },
	{  1970,  1970,  "UTMS"      },
	{  1971,  1971,  "UTMN"      },
	{  1972,  1972,  "UTMS"      },
	{  1973,  1973,  "UTMN"      },
	{  1974,  1974,  "UTMS"      },
	{  1975,  1975,  "UTMN"      },
	{  1976,  1976,  "UTMS"      },
	{  1977,  1977,  "UTMN"      },
	{  1978,  1978,  "UTMS"      },
	{  1979,  1979,  "UTMN"      },
	{  1980,  1980,  "UTMS"      },
	{  1981,  1981,  "UTMN"      },
	{  1982,  1982,  "UTMS"      },
	{  1983,  1983,  "UTMN"      },
	{  1984,  1984,  "UTMS"      },
	{  1985,  1985,  "UTMN"      },
	{  1986,  1986,  "UTMS"      },
	{  1987,  1987,  "UTMN"      },
	{  1988,  1988,  "UTMS"      },
	{  1989,  1989,  "UTMN"      },
	{  1990,  1990,  "UTMS"      },
	{  1991,  1991,  "UTMN"      },
	{  1992,  1992,  "UTMS"      },
	{  1993,  1994,  "UTMN"      },
	{  1995,  1995,  "UTMS"      },
	{  1996,  1997,  "POLAR"     },
	{  1998,  1998,  "UTMN"      },
	{  1999,  1999,  "UTMS"      },
	{  2000,  2000,  "UTMN"      },
	{  2001,  2001,  "UTMS"      },
	{  2002,  2002,  "UTMN"      },
	{  2003,  2003,  "UTMS"      },
	{  2004,  2004,  "UTMN"      },
	{  2005,  2005,  "UTMS"      },
	{  2006,  2006,  "UTMN"      },
	{  2007,  2007,  "UTMS"      },
	{  2008,  2008,  "UTMN"      },
	{  2009,  2009,  "UTMS"      },
	{  2010,  2010,  "UTMN"      },
	{  2011,  2011,  "UTMS"      },
	{  2012,  2012,  "UTMN"      },
	{  2013,  2013,  "UTMS"      },
	{  2014,  2014,  "UTMN"      },
	{  2015,  2015,  "UTMS"      },
	{  2016,  2016,  "UTMN"      },
	{  2017,  2017,  "UTMS"      },
	{  2018,  2018,  "UTMN"      },
	{  2019,  2019,  "UTMS"      },
	{  2020,  2020,  "UTMN"      },
	{  2021,  2021,  "UTMS"      },
	{  2022,  2022,  "UTMN"      },
	{  2023,  2023,  "UTMS"      },
	{  2024,  2024,  "UTMN"      },
	{  2025,  2025,  "UTMS"      },
	{  2026,  2026,  "UTMN"      },
	{  2027,  2027,  "UTMS"      },
	{  2028,  2028,  "UTMN"      },
	{  2029,  2029,  "UTMS"      },
	{  2030,  2030,  "UTMN"      },
	{  2031,  2031,  "UTMS"      },
	{  2032,  2032,  "UTMN"      },
	{  2033,  2033,  "UTMS"      },
	{  2034,  2034,  "UTMN"      },
	{  2035,  2035,  "UTMS"      },
	{  2036,  2036,  "UTMN"      },
	{  2037,  2037,  "UTMS"      },
	{  2038,  2038,  "UTMN"      },
	{  2039,  2039,  "UTMS"      },
	{  2040,  2040,  "UTMN"      },
	{  2041,  2041,  "UTMS"      },
	{  2042,  2042,  "UTMN"      },
	{  2043,  2043,  "UTMS"      },
	{  2044,  2044,  "UTMN"      },
	{  2045,  2045,  "UTMS"      },
	{  2046,  2046,  "UTMN"      },
	{  2047,  2047,  "UTMS"      },
	{  2048,  2048,  "UTMN"      },
	{  2049,  2049,  "UTMS"      },
	{  2050,  2050,  "UTMN"      },
	{  2051,  2051,  "UTMS"      },
	{  2052,  2052,  "UTMN"      },
	{  2053,  2053,  "UTMS"      },
	{  2054,  2054,  "UTMN"      },
	{  2055,  2055,  "UTMS"      },
	{  2056,  2056,  "UTMN"      },
	{  2057,  2057,  "UTMS"      },
	{  2058,  2058,  "UTMN"      },
	{  2059,  2059,  "UTMS"      },
	{  2060,  2060,  "UTMN"      },
	{  2061,  2061,  "UTMS"      },
	{  2062,  2062,  "UTMN"      },
	{  2063,  2063,  "UTMS"      },
	{  2064,  2064,  "UTMN"      },
	{  2065,  2065,  "UTMS"      },
	{  2066,  2066,  "UTMN"      },
	{  2067,  2067,  "UTMS"      },
	{  2068,  2068,  "UTMN"      },
	{  2069,  2069,  "UTMS"      },
	{  2070,  2070,  "UTMN"      },
	{  2071,  2071,  "UTMS"      },
	{  2072,  2072,  "UTMN"      },
	{  2073,  2073,  "UTMS"      },
	{  2074,  2074,  "UTMN"      },
	{  2075,  2075,  "UTMS"      },
	{  2076,  2076,  "UTMN"      },
	{  2077,  2077,  "UTMS"      },
	{  2078,  2078,  "UTMN"      },
	{  2079,  2079,  "UTMS"      },
	{  2080,  2080,  "UTMN"      },
	{  2081,  2081,  "UTMS"      },
	{  2082,  2082,  "UTMN"      },
	{  2083,  2083,  "UTMS"      },
	{  2084,  2084,  "UTMN"      },
	{  2085,  2085,  "UTMS"      },
	{  2086,  2086,  "UTMN"      },
	{  2087,  2087,  "UTMS"      },
	{  2088,  2088,  "UTMN"      },
	{  2089,  2089,  "UTMS"      },
	{  2090,  2090,  "UTMN"      },
	{  2091,  2091,  "UTMS"      },
	{  2092,  2092,  "UTMN"      },
	{  2093,  2093,  "UTMS"      },
	{  2094,  2094,  "UTMN"      },
	{  2095,  2095,  "UTMS"      },
	{  2096,  2096,  "UTMN"      },
	{  2097,  2097,  "UTMS"      },
	{  2098,  2098,  "UTMN"      },
	{  2099,  2099,  "UTMS"      },
	{  2100,  2100,  "UTMN"      },
	{  2101,  2101,  "UTMS"      },
	{  2102,  2102,  "UTMN"      },
	{  2103,  2103,  "UTMS"      },
	{  2104,  2104,  "UTMN"      },
	{  2105,  2105,  "UTMS"      },
	{  2106,  2106,  "UTMN"      },
	{  2107,  2107,  "UTMS"      },
	{  2108,  2108,  "UTMN"      },
	{  2109,  2109,  "UTMS"      },
	{  2110,  2110,  "UTMN"      },
	{  2111,  2111,  "UTMS"      },
	{  2112,  2112,  "UTMN"      },
	{  2113,  2113,  "UTMS"      },
	{  2114,  2114,  "UTMN"      },
	{  2115,  2115,  "UTMS"      },
	{  2116,  2116,  "UTMN"      },
	{  2117,  2117,  "UTMS"      },
	{  2118,  2118,  "UTMN"      },
	{  2119,  2119,  "UTMS"      },
	{  2120,  2121,  "CAMER"     },
	{  2122,  2153,  "UTMN"      },
	{  2154,  2155,  "SPCS"      },
	{  2156,  2165,  "OTHR-US"   },
	{  2166,  2170,  "SPCS"      },
	{  2171,  2174,  "CARIB"     },
	{  2175,  2225,  "SPCS"      },
	{  2226,  2227,  "CANADA"    },
	{  2228,  2250,  "SPCS"      },
	{  2251,  2251,  "CARIB"     },
	{  2252,  2274,  "SPCS"      },
	{  2275,  2281,  "CANADA"    },
	{  2282,  2282,  "AFRICA"    },
	{  2283,  2287,  "AUSNZ"     },
	{  2288,  2289,  "PACIFIC"   },
	{  2290,  2290,  "CANADA"    },
	{  2291,  2291,  "AUSNZ"     },
	{  2292,  2293,  "ASIA"      },
	{  2294,  2294,  "MIDEAST"   },
	{  2295,  2295,  "CARIB"     },
	{  2296,  2296,  "AFRICA"    },
	{  2297,  2298,  "OTHR-US"   },
	{  2299,  2306,  "EUROPE"    },
	{  2307,  2310,  "SAMER"     },
	{  2311,  2312,  "AFRICA"    },
	{  2313,  2313,  "CANADA"    },
	{  2314,  2314,  "ASIA"      },
	{  2315,  2315,  "SAMER"     },
	{  2316,  2324,  "AFRICA"    },
	{  2325,  2325,  "SAMER"     },
	{  2326,  2326,  "EUROPE"    },
	{  2327,  2329,  "MIDEAST"   },
	{  2330,  2340,  "EUROPE"    },
	{  2341,  2341,  "AFRICA"    },
	{  2342,  2344,  "EUROPE"    },
	{  2345,  2345,  "MIDEAST"   },
	{  2346,  2346,  "WORLD"     },
	{  2347,  2347,  "AFRICA"    },
	{  2348,  2349,  "ASIA"      },
	{  2350,  2353,  "AFRICA"    },
	{  2354,  2354,  "ASIA"      },
	{  2355,  2357,  "SAMER"     },
	{  2358,  2361,  "ASIA"      },
	{  2362,  2362,  "MIDEAST"   },
	{  2363,  2363,  "SAMER"     },
	{  2364,  2365,  "PACIFIC"   },
	{  2366,  2368,  "EUROPE"    },
	{  2369,  2369,  "INDIAN"    },
	{  2370,  2370,  "EUROPE"    },
	{  2371,  2371,  "AFRICA"    },
	{  2372,  2372,  "EUROPE"    },
	{  2373,  2374,  "OTHR-US"   },
	{  2375,  2376,  "CANADA"    },
	{  2377,  2383,  "OTHR-US"   },
	{  2384,  2384,  "CANADA"    },
	{  2385,  2385,  "CAMER"     },
	{  2386,  2386,  "ATLANTIC"  },
	{  2387,  2390,  "OTHR-US"   },
	{  2391,  2392,  "MIDEAST"   },
	{  2393,  2393,  "AFRICA"    },
	{  2394,  2398,  "EUROPE"    },
	{  2399,  2403,  "SAMER"     },
	{  2404,  2404,  "MIDEAST"   },
	{  2405,  2405,  "ASIA"      },
	{  2406,  2406,  "MIDEAST"   },
	{  2407,  2407,  "ATLANTIC"  },
	{  2408,  2409,  "ASIA"      },
	{  2410,  2410,  "CANADA"    },
	{  2411,  2411,  "ASIA"      },
	{  2412,  2412,  "OTHR-US"   },
	{  2413,  2414,  "CARIB"     },
	{  2415,  2417,  "CANADA"    },
	{  2418,  2418,  "CARIB"     },
	{  2419,  2419,  "CAMER"     },
	{  2420,  2421,  "EUROPE"    },
	{  2424,  2424,  "OTHR-US"   },
	{  2425,  2526,  "ASIA"      },
	{  2527,  2529,  "SPCS"      },
	{  2530,  2547,  "EUROPE"    },
	{  2548,  2555,  "AFRICA"    },
	{  2556,  2573,  "ATLANTIC"  },
	{  2574,  2574,  "AFRICA"    },
	{  2575,  2576,  "AUSNZ"     },
	{  2577,  2589,  "ASIA"      },
	{  2590,  2591,  "AFRICA"    },
	{  2592,  2594,  "ASIA"      },
	{  2595,  2595,  "AFRICA"    },
	{  2596,  2597,  "SAMER"     },
	{  2598,  2600,  "AFRICA"    },
	{  2601,  2601,  "EUROPE"    },
	{  2602,  2603,  "MIDEAST"   },
	{  2604,  2652,  "ASIA"      },
	{  2653,  2663,  "EUROPE"    },
	{  2664,  2731,  "ASIA"      },
	{  2732,  2746,  "NONE"      },
	{  2747,  2756,  "EUROPE"    },
	{  2757,  2770,  "ASIA"      },
	{  2771,  2771,  "AFRICA"    },
	{  2772,  2778,  "ASIA"      },
	{  2779,  2779,  "EUROPE"    },
	{  2780,  2780,  "ASIA"      },
	{  2781,  2783,  "MIDEAST"   },
	{  2784,  2784,  "CANADA"    },
	{  2785,  2791,  "AFRICA"    },
	{  2792,  2804,  "EUROPE"    },
	{  2805,  2805,  "SAMER"     },
	{  2806,  2806,  "MIDEAST"   },
	{  2807,  2809,  "INDIAN"    },
	{  2810,  2816,  "PACIFIC"   },
	{  2817,  2818,  "POLAR"     },
	{  2819,  2823,  "PACIFIC"   },
	{  2824,  2824,  "CARIB"     },
	{  2825,  2827,  "AFRICA"    },
	{  2828,  2829,  "CARIB"     },
	{  2830,  2830,  "WORLD"     },
	{  2831,  2832,  "CANADA"    },
	{  2833,  2875,  "EUROPE"    },
	{  2876,  2876,  "ASIA"      },
	{  2879,  2879,  "EUROPE"    },
	{  2880,  2880,  "POLAR"     },
	{  2881,  2888,  "EUROPE"    },
	{  2889,  2889,  "AUSNZ"     },
	{  2890,  2895,  "CARIB"     },
	{  2896,  2897,  "ASIA"      },
	{  2898,  2898,  "EUROPE"    },
	{  2899,  2947,  "AUSNZ"     },
	{  2948,  2950,  "OTHR_US"   },
	{  2951,  2955,  "ASIA"      },
	{  2956,  2957,  "MIDEAST"   },
	{  2958,  2960,  "OTHR-US"   },
	{  2961,  2961,  "EUROPE"    },
	{  2962,  2966,  "SAMER"     },
	{  2967,  2972,  "AFRICA"    },
	{  2973,  2980,  "OTHR-US"   },
	{  2981,  2981,  "AFRICA"    },
	{  2982,  2985,  "ASIA"      },
	{  2986,  2986,  "AUSNZ"     },
	{  2987,  2987,  "AFRICA"    },
	{  2988,  2989,  "EUROPE"    },
	{  2990,  2990,  "AUSNZ"     },
	{  2991,  3081,  "POLAR"     },
	{  3082,  3091,  "SAMER"     },
	{  3092,  3104,  "EUROPE"    },
	{  3105,  3105,  "SAMER"     },
	{  3106,  3107,  "MIDEAST"   },
	{  3108,  3110,  "PACIFIC"   },
	{  3111,  3111,  "EUROPE"    },
	{  3112,  3112,  "SAMER"     },
	{  3113,  3113,  "AFRICA"    },
	{  3114,  3117,  "OTHR-NA"   },
	{  3118,  3118,  "CARIB"     },
	{  3119,  3119,  "EUROPE"    },
	{  3120,  3138,  "PACIFIC"   },
	{  3139,  3139,  "AUSNZ"     },
	{  3140,  3141,  "MIDEAST"   },
	{  3142,  3142,  "AFRICA"    },
	{  3143,  3143,  "CARIB"     },
	{  3144,  3146,  "SAMER"     },
	{  3147,  3167,  "AFRICA"    },
	{  3168,  3170,  "EUROPE"    },
	{  3171,  3171,  "AFRICA"    },
	{  3172,  3172,  "PACIFIC"   },
	{  3173,  3175,  "EUROPE"    },
	{  3176,  3178,  "SAMER"     },
	{  3179,  3180,  "AFRICA"    },
	{  3181,  3181,  "OTHR-US"   },
	{  3182,  3184,  "ATLANTIC"  },
	{  3185,  3186,  "CARIB"     },
	{  3187,  3187,  "ATLANTIC"  },
	{  3188,  3188,  "PACIFIC"   },
	{  3189,  3189,  "INDIAN"    },
	{  3190,  3198,  "PACIFIC"   },
	{  3199,  3199,  "EUROPE"    },
	{  3200,  3200,  "ASIA"      },
	{  3201,  3202,  "PACIFIC"   },
	{  3203,  3203,  "ASIA"      },
	{  3204,  3205,  "POLAR"     },
	{  3206,  3206,  "OTHR-NA"   },
	{  3207,  3207,  "CARIB"     },
	{  3208,  3209,  "PACIFIC"   },
	{  3210,  3212,  "EUROPE"    },
	{  3213,  3213,  "AFRICA"    },
	{  3214,  3214,  "CARIB"     },
	{  3215,  3215,  "SAMER"     },
	{  3216,  3216,  "CARIB"     },
	{  3217,  3217,  "ASIA"      },
	{  3218,  3218,  "CARIB"     },
	{  3219,  3219,  "CAMER"     },
	{  3220,  3220,  "AFRICA"    },
	{  3221,  3222,  "ATLANTIC"  },
	{  3223,  3223,  "SAMER"     },
	{  3224,  3224,  "EUROPE"    },
	{  3225,  3225,  "ASIA"      },
	{  3226,  3226,  "AFRICA"    },
	{  3227,  3227,  "SAMER"     },
	{  3228,  3228,  "ASIA"      },
	{  3229,  3229,  "SAMER"     },
	{  3230,  3231,  "AFRICA"    },
	{  3232,  3232,  "CAMER"     },
	{  3233,  3233,  "AFRICA"    },
	{  3234,  3234,  "EUROPE"    },
	{  3235,  3235,  "CARIB"     },
	{  3236,  3236,  "EUROPE"    },
	{  3237,  3237,  "EUROPE"    },
	{  3238,  3238,  "AFRICA"    },
	{  3239,  3240,  "CARIB"     },
	{  3241,  3241,  "SAMER"     },
	{  3242,  3242,  "AFRICA"    },
	{  3243,  3243,  "CAMER"     },
	{  3244,  3244,  "AFRICA"    },
	{  3245,  3245,  "AFRICA"    },
	{  3246,  3246,  "EUROPE"    },
	{  3247,  3247,  "SAMER"     },
	{  3248,  3248,  "EUROPE"    },
	{  3249,  3250,  "AFRICA"    },
	{  3251,  3251,  "ASIA"      },
	{  3252,  3252,  "AFRICA"    },
	{  3253,  3254,  "EUROPE"    },
	{  3255,  3255,  "PACIFIC"   },
	{  3256,  3256,  "CAMER"     },
	{  3257,  3258,  "AFRICA"    },
	{  3259,  3259,  "SAMER"     },
	{  3260,  3260,  "CARIB"     },
	{  3261,  3261,  "CAMER"     },
	{  3262,  3262,  "EUROPE"    },
	{  3263,  3263,  "EUROPE"    },
	{  3264,  3264,  "AFRICA"    },
	{  3265,  3266,  "ASIA"      },
	{  3267,  3267,  "MIDEAST"   },
	{  3268,  3268,  "EUROPE"    },
	{  3269,  3269,  "MIDEAST"   },
	{  3270,  3271,  "AFRICA"    },
	{  3272,  3272,  "EUROPE"    },
	{  3273,  3274,  "INDIAN"    },
	{  3275,  3275,  "EUROPE"    },
	{  3276,  3276,  "CARIB"     },
	{  3277,  3277,  "AFRICA"    },
	{  3278,  3278,  "CAMER"     },
	{  3279,  3279,  "CARIB"     },
	{  3280,  3281,  "AFRICA"    },
	{  3282,  3282,  "ASIA"      },
	{  3283,  3283,  "AFRICA"    },
	{  3284,  3284,  "EUROPE"    },
	{  3285,  3285,  "AUSNZ"     },
	{  3286,  3286,  "CAMER"     },
	{  3287,  3287,  "AFRICA"    },
	{  3288,  3288,  "MIDEAST"   },
	{  3289,  3289,  "ASIA"      },
	{  3290,  3290,  "CAMER"     },
	{  3291,  3291,  "ASIA"      },
	{  3292,  3292,  "SAMER"     },
	{  3293,  3293,  "EUROPE"    },
	{  3294,  3294,  "CARIB"     },
	{  3295,  3295,  "EUROPE"    },
	{  3296,  3296,  "ASIA"      },
	{  3297,  3298,  "CARIB"     },
	{  3299,  3299,  "CANADA"    },
	{  3300,  3300,  "CARIB"     },
	{  3301,  3301,  "PACIFIC"   },
	{  3302,  3302,  "AFRICA"    },
	{  3303,  3303,  "MIDEAST"   },
	{  3304,  3304,  "AFRICA"    },
	{  3305,  3305,  "EUROPE"    },
	{  3306,  3306,  "AFRICA"    },
	{  3307,  3307,  "EUROPE"    },
	{  3308,  3308,  "AFRICA"    },
	{  3309,  3309,  "AFRICA"    },
	{  3310,  3310,  "ASIA"      },
	{  3311,  3311,  "AFRICA"    },
	{  3312,  3312,  "SAMER"     },
	{  3313,  3313,  "EUROPE"    },
	{  3314,  3314,  "MIDEAST"   },
	{  3315,  3315,  "ASIA"      },
	{  3316,  3316,  "AFRICA"    },
	{  3317,  3317,  "ASIA"      },
	{  3318,  3318,  "ASIA"      },
	{  3319,  3319,  "AFRICA"    },
	{  3320,  3321,  "PACIFIC"   },
	{  3322,  3322,  "ASIA"      },
	{  3323,  3323,  "CARIB"     },
	{  3324,  3324,  "EUROPE"    },
	{  3325,  3325,  "MIDEAST"   },
	{  3326,  3326,  "SAMER"     },
	{  3327,  3327,  "SAMER"     },
	{  3328,  3328,  "ASIA"      },
	{  3329,  3330,  "CARIB"     },
	{  3331,  3331,  "AFRICA"    },
	{  3332,  3332,  "MIDEAST"   },
	{  3333,  3333,  "EUROPE"    },
	{  3334,  3335,  "ASIA"      },
	{  3336,  3336,  "MIDEAST"   },
	{  3337,  3337,  "INDIAN"    },
	{  3338,  3338,  "AUSNZ"     },
	{  3339,  3339,  "EUROPE"    },
	{  3340,  3340,  "INDIAN"    },
	{  3341,  3341,  "ASIA"      },
	{  3342,  3342,  "CARIB"     },
	{  3343,  3343,  "EUROPE"    },
	{  3344,  3344,  "AUSNZ"     },
	{  3355,  3356,  "SAMER"     },
	{  3357,  3360,  "OTHR-US"   },
	{  3361,  3361,  "CAMER"     },
	{  3362,  3371,  "EUROPE"    },
	{  3372,  3375,  "OTHR-US"   },
	{  3372,  3375,  "OTHR-US"   },
	{  3376,  3384,  "ASIA"      },
	{  3385,  3385,  "EUROPE"    },
	{  3387,  3389,  "MIDEAST"   },
	{  3387,  3390,  "MIDEAST"   },
	{  3391,  3391,  "WORLD"     },
	{  3392,  3396,  "EUROPE"    },
	{  3397,  3397,  "MIDEAST"   },
	{  3398,  3401,  "PACIFIC"   },
	{  3402,  3402,  "AFRICA"    },
	{  3402,  3402,  "AFRICA"    },
	{  3403,  3403,  "ASIA"      },
	{  3404,  3407,  "CANADA"    },
	{  3408,  3408,  "EUROPE"    },
	{  3409,  3417,  "CANADA"    },
	{  3418,  3418,  "SAMER"     },
	{  3419,  3420,  "OTHR-NA"   },
	{  3421,  3422,  "SAMER"     },
	{  3423,  3426,  "CAMER"     },
	{  3427,  3428,  "SAMER"     },
	{  3429,  3429,  "EUROPE"    },
	{  3430,  3435,  "PACIFIC"   },
	{  3436,  3448,  "SAMER"     },
	{  3449,  3460,  "EUROPE"    },
	{  3461,  3462,  "CAMER"     },
	{  3463,  3463,  "WORLD"     },
	{  3464,  3465,  "AFRICA"    },
	{     0,     0,  ""          }
};


const wchar_t* GetEpsgTableName (EcsEpsgTable tblId)
{
	const wchar_t* namePtr = 0;
	const TcsEpsgTblMap* tblPtr;
	
	for (tblPtr = KcsEpsgTblMap;tblPtr->TableId != epsgTblUnknown;++tblPtr)
	{
		if (tblPtr->TableId == tblId)
		{
			namePtr = tblPtr->TableName;
			break;
		}
	}
	return namePtr;
}

EcsEpsgTable GetEpsgTableId (const wchar_t* tableName)
{
    EcsEpsgTable tableId = epsgTblUnknown;
	const TcsEpsgTblMap* tblPtr;
	
	for (tblPtr = KcsEpsgTblMap;tblPtr->TableId != epsgTblUnknown;++tblPtr)
	{
		if (!_wcsicmp (tableName,tblPtr->TableName))
		{
		    tableId = tblPtr->TableId;
			break;
		}
	}
	return tableId;
}

short GetEpsgCodeFieldNbr (EcsEpsgTable tableId)
{
	short fldNbr (-1);
	const TcsEpsgTblMap* tblPtr;
	
	for (tblPtr = KcsEpsgTblMap;tblPtr->TableId != epsgTblUnknown;++tblPtr)
	{
		if (tblPtr->TableId == tableId)
		{
		    fldNbr = GetEpsgFieldNumber (tableId,tblPtr->CodeKeyFieldId);
			break;
		}
	}
	return fldNbr;
}

short GetEpsgFieldNumber (EcsEpsgTable tableId,EcsEpsgField fieldId)
{
	short rtnValue = TcsEpsgDataSetV6::GetFldNbr (tableId,fieldId);
	return rtnValue;
}

EcsCsysType GetEpsgCsysType (const wchar_t* csysTypeName)
{
	EcsCsysType rtnValue;
	const TcsEpsgCsysTypeMap* tblPtr;

	rtnValue = epsgCsysTypUnknown;
	for (tblPtr = KcsEpsgCsysTypeMap;tblPtr->CsysType != epsgCsysTypUnknown;++tblPtr)
	{
		if (!_wcsicmp (csysTypeName,tblPtr->CsysTypeName))
		{
			rtnValue = tblPtr->CsysType;
			break;
		}
	}
	return rtnValue;
}

EcsCrsType GetEpsgCrsType (const wchar_t* crsTypeName)
{
	EcsCrsType rtnValue;
	const TcsEpsgCrsTypeMap* tblPtr;

	rtnValue = epsgCrsTypUnknown;
	for (tblPtr = KcsEpsgCrsTypeMap;tblPtr->CrsType != epsgCrsTypUnknown;++tblPtr)
	{
		if (!_wcsicmp (crsTypeName,tblPtr->CrsTypeName))
		{
			rtnValue = tblPtr->CrsType;
			break;
		}
	}
	return rtnValue;
}

EcsOpType GetEpsgOpType (const wchar_t* opTypeName)
{
	EcsOpType rtnValue;
	const TcsEpsgOpTypeMap* tblPtr;

	rtnValue = epsgOpTypUnknown;
	for (tblPtr = KcsEpsgOpTypeMap;tblPtr->OpType != epsgOpTypUnknown;++tblPtr)
	{
		if (!_wcsicmp (opTypeName,tblPtr->OpTypeName))
		{
			rtnValue = tblPtr->OpType;
			break;
		}
	}
	return rtnValue;
}

EcsDtmType GetEpsgDtmType (const wchar_t* dtmTypeName)
{
	EcsDtmType rtnValue;
	const TcsEpsgDtmTypeMap* tblPtr;

	rtnValue = epsgDtmTypUnknown;
	for (tblPtr = KcsEpsgDtmTypeMap;tblPtr->DtmType != epsgDtmTypUnknown;++tblPtr)
	{
		if (!_wcsicmp (dtmTypeName,tblPtr->DtmTypeName))
		{
			rtnValue = tblPtr->DtmType;
			break;
		}
	}
	return rtnValue;
}

EcsUomType GetEpsgUomType (const wchar_t* uomTypeName)
{
	EcsUomType rtnValue;
	const TcsEpsgUomTypeMap* tblPtr;

	rtnValue = epsgUomTypUnknown;
	for (tblPtr = KcsEpsgUomTypeMap;tblPtr->UomType != epsgUomTypUnknown;++tblPtr)
	{
		if (!_wcsicmp (uomTypeName,tblPtr->UomTypeName))
		{
			rtnValue = tblPtr->UomType;
			break;
		}
	}
	return rtnValue;
}

EcsOrientation GetOrientation (const wchar_t* orntTypeName)
{
	EcsOrientation rtnValue;
	const TcsEpsgOrntTypeMap* tblPtr;

	unsigned cmpCount;
	wchar_t cmpBufr [32];

	rtnValue = epsgOrntUnknown;
	for (tblPtr = KcsEpsgOrntTypeMap;tblPtr->OrntType != epsgOrntUnknown;++tblPtr)
	{
		cmpCount = static_cast<unsigned>(wcslen (tblPtr->OrntTypeName));
		if (cmpCount >= wcCount (cmpBufr))
		{
			cmpCount = wcCount (cmpBufr) - 1;
		}
		wcsncpy (cmpBufr,orntTypeName,cmpCount);
		cmpBufr [cmpCount] = L'\0';
		if (_wcsicmp (cmpBufr,tblPtr->OrntTypeName) == 0)
		{
			rtnValue = tblPtr->OrntType;
			break;
		}
	}
	return rtnValue;
}

//newPage//
//=============================================================================
// TcsEpsgCode  --  Distinct type for an EPSG code value.
//
// For function overloading and parameter safety, its very nice to have a
// distinct type for the EPSG code value.
//=============================================================================
// Static Constants, Variables, and Member Functions
const unsigned long TcsEpsgCode::InvalidValue = 0UL;
//=============================================================================
// Construction, Destruction, & Assignment
TcsEpsgCode::TcsEpsgCode () : EpsgCode (InvalidValue)
{
}
TcsEpsgCode::TcsEpsgCode (unsigned long epsgCode) : EpsgCode (epsgCode)
{
}
TcsEpsgCode::TcsEpsgCode (const wchar_t* epsgCode)
{
	EpsgCode = StrToEpsgCode (epsgCode);
}
TcsEpsgCode::TcsEpsgCode (const std::wstring& epsgCode)
{
	EpsgCode = StrToEpsgCode (epsgCode.c_str ());
}
TcsEpsgCode::TcsEpsgCode (const TcsEpsgCode& source) : EpsgCode (source.EpsgCode)
{
}
TcsEpsgCode::~TcsEpsgCode ()
{
}
TcsEpsgCode& TcsEpsgCode::operator= (const TcsEpsgCode& rhs)
{
	EpsgCode = rhs.EpsgCode;
	return *this;
}
TcsEpsgCode& TcsEpsgCode::operator= (unsigned long epsgCode)
{
	EpsgCode = epsgCode;
	return *this;
}
//=============================================================================
// Operator Overrides
bool TcsEpsgCode::operator< (unsigned long epsgCode) const
{
	bool lessThan = (EpsgCode < epsgCode);
	return lessThan;
}
bool TcsEpsgCode::operator< (const std::wstring& epsgCode) const
{
	TcsEpsgCode tmpEpsgCode (epsgCode);
	bool lessThan = (EpsgCode < tmpEpsgCode);
	return lessThan;
}
bool TcsEpsgCode::operator== (unsigned long epsgCode) const
{
	bool equal = (EpsgCode == epsgCode);
	return equal;
}
bool TcsEpsgCode::operator== (const std::wstring& epsgCode) const
{
	TcsEpsgCode tmpEpsgCode (epsgCode);
	bool equal = (EpsgCode == tmpEpsgCode);
	return equal;
}
bool TcsEpsgCode::operator> (unsigned long epsgCode) const
{
	bool greaterThan = (EpsgCode > epsgCode);
	return greaterThan;
}
bool TcsEpsgCode::operator> (const std::wstring& epsgCode) const
{
	TcsEpsgCode tmpEpsgCode (epsgCode);
	bool greaterThan = (EpsgCode > tmpEpsgCode);
	return greaterThan;
}
TcsEpsgCode TcsEpsgCode::operator++ ()
{
	EpsgCode += 1;
	return EpsgCode;
}
TcsEpsgCode TcsEpsgCode::operator++ (int)
{
	unsigned long prevValue = EpsgCode;
	EpsgCode += 1;
	return prevValue;
}
TcsEpsgCode TcsEpsgCode::operator-- ()
{
	EpsgCode -= 1;
	return EpsgCode;
}
TcsEpsgCode TcsEpsgCode::operator-- (int)
{
	unsigned long prevValue = EpsgCode;
	EpsgCode -= 1;
	return prevValue;
}
TcsEpsgCode& TcsEpsgCode::operator+= (unsigned long rhs)
{
	EpsgCode += rhs;
	return *this;
}
TcsEpsgCode& TcsEpsgCode::operator-= (unsigned long rhs)
{
	EpsgCode -= rhs;
	return *this;
}
TcsEpsgCode& TcsEpsgCode::operator+= (int rhs)
{
	unsigned long tmpRhs;

	if (rhs < 0)
	{
		tmpRhs = static_cast<unsigned long>(-rhs);
		EpsgCode -= tmpRhs;
	}
	else
	{
		tmpRhs = static_cast<unsigned long>(rhs);
		EpsgCode += tmpRhs;
	}
	return *this;
}
TcsEpsgCode& TcsEpsgCode::operator-= (int rhs)
{
	unsigned long tmpRhs;

	if (rhs < 0)
	{
		tmpRhs = static_cast<unsigned long>(-rhs);
		EpsgCode += tmpRhs;
	}
	else
	{
		tmpRhs = static_cast<unsigned long>(rhs);
		EpsgCode -= tmpRhs;
	}
	return *this;
}
TcsEpsgCode TcsEpsgCode::operator+ (unsigned long rhs)
{
	unsigned long sum = EpsgCode + rhs;
	return TcsEpsgCode (sum);
}
TcsEpsgCode TcsEpsgCode::operator- (unsigned long rhs)
{
	unsigned long difference = EpsgCode - rhs;
	return TcsEpsgCode (difference);
}
//=============================================================================
// Public Named Member Functions
std::wstring TcsEpsgCode::AsWstring () const
{
	wchar_t* wcPtr;
	wchar_t wcArray [32];

	if (EpsgCode > 1991000UL)
	{
		// Appears to be a change ID.
		double realValue = (static_cast<double>(EpsgCode) / 1000.0);
		swprintf (wcArray,wcCount (wcArray),L"%.3d",realValue);
		wcPtr = wcArray + wcslen (wcArray) - 1;
		if (*wcPtr == L'0') *--wcPtr = L'\0';
		if (*wcPtr == L'0') *--wcPtr = L'\0';
	}
	else
	{
		swprintf (wcArray,wcCount (wcArray),L"%lu",EpsgCode);
	}
	return std::wstring (wcArray);
}
std::string TcsEpsgCode::AsString () const
{
	char* cPtr;
	char cArray [32];

	if (EpsgCode > 1991000UL)
	{
		// Appears to be a change ID.
		double realValue = (static_cast<double>(EpsgCode) / 1000.0);
		sprintf (cArray,"%.3f",realValue);
		cPtr = cArray + strlen (cArray) - 1;
		if (*cPtr == '0') *--cPtr = '\0';
		if (*cPtr == '0') *--cPtr = '\0';
	}
	else
	{
		sprintf (cArray,"%lu",EpsgCode);
	}
	return std::string (cArray);
}
bool TcsEpsgCode::AsString (wchar_t* result,size_t resultSize) const
{
	std::wstring tmpEpsgCode = AsWstring ();
	wcsncpy (result,tmpEpsgCode.c_str (),resultSize);
	result [resultSize - 1] = L'\0';
	return true;
}
bool TcsEpsgCode::AsString (char* result,size_t resultSize) const
{
	std::string tmpEpsgCode = AsString ();
	strncpy (result,tmpEpsgCode.c_str (),resultSize);
	result [resultSize - 1] = '\0';
	return true;
}
//=============================================================================
// Protected Member Functions
unsigned long TcsEpsgCode::StrToEpsgCode (const wchar_t* epsgCodeStr) const
{
	const wchar_t* wcPtr;
	TcsEpsgCode epsgCodeNbr (0UL);

	wcPtr = wcschr (epsgCodeStr,L'.');
	if (wcPtr == 0)
	{
		// Appears to be a normal code value.
		epsgCodeNbr = wcstoul (epsgCodeStr,0,10);
	}
	else
	{
		// The string appears to be a change ID.
		double realValue = wcstod (epsgCodeStr,0);
		realValue *= 1000;
		epsgCodeNbr = static_cast<unsigned long>(realValue);
	}
	return epsgCodeNbr;
}
unsigned long TcsEpsgCode::StrToEpsgCode (const char* epsgCodeStr) const
{
	const char* cPtr;
	TcsEpsgCode epsgCodeNbr (0UL);

	cPtr = strchr (epsgCodeStr,'.');
	if (cPtr == 0)
	{
		// Appears to be a normal code value.
		epsgCodeNbr = strtoul (epsgCodeStr,0,10);
	}
	else
	{
		// The string appears to be a change ID.
		double realValue = strtod (epsgCodeStr,0);
		realValue *= 1000;
		epsgCodeNbr = static_cast<unsigned long>(realValue);
	}
	return epsgCodeNbr;
}
//newPage//
//=============================================================================
// TcsEpsgTable  --  Customization of TcsCsvTableBase for EPSG Tables
//=============================================================================
// Static Constants, Variables, and Member Functions
const wchar_t TcsEpsgTable::LogicalTrue  [] = L"TRUE";
const wchar_t TcsEpsgTable::LogicalFalse [] = L"FALSE";
//=============================================================================
// Construction, Destruction, & Assignment
TcsEpsgTable::TcsEpsgTable (const TcsEpsgTblMap& tblMap,const wchar_t* databaseFldr)
															:
														TcsCsvFileBase   (true,2,35),
														Ok               (false),
														Sorted           (false),
														Indexed          (false),
														TableId          (epsgTblNone),
														CodeKeyField     (epsgFldNone),
														CurrentCodeValue (0UL),
														CurrentRecordNbr (InvalidRecordNbr),
														SortFunctor      (-1),
														CodeKeyStack     (),
														CsvStatus        ()
{
// Seems gcc 3.2.2 wifstream::open requires an 8 bit character path.  NOT NICE!!!
char pathBufr [1024];
	// Set the min and max fild counts.
	TableId = tblMap.TableId;
	CodeKeyField = tblMap.CodeKeyFieldId;
	SetMinFldCnt (tblMap.FieldCount);
	SetMaxFldCnt (tblMap.FieldCount);

	SortFunctor.FirstField  = GetEpsgFieldNumber (TableId,tblMap.Sort1);
	SortFunctor.SecondField = GetEpsgFieldNumber (TableId,tblMap.Sort2);
	SortFunctor.ThirdField  = GetEpsgFieldNumber (TableId,tblMap.Sort3);
	SortFunctor.FourthField = GetEpsgFieldNumber (TableId,tblMap.Sort4);
	std::wstring objName (tblMap.TableName);
	objName += L" Table";
	SetObjectName (objName);
	
	std::wstring filePath (databaseFldr);
	filePath += L"\\";
	filePath += tblMap.TableName;
	filePath += L".csv";
wcstombs (pathBufr,filePath.c_str (),sizeof (pathBufr));
std::wifstream iStrm (pathBufr,std::ios_base::in);
//	std::wifstream iStrm (filePath.c_str (),std::ios_base::in);
	if (iStrm.is_open ())
	{
		Ok = ReadFromStream (iStrm,true,CsvStatus);
	}
	if (Ok)
	{
		Ok = PrepareCsvFile ();
	}
}
TcsEpsgTable::TcsEpsgTable (const TcsEpsgTable& source) : TcsCsvFileBase   (source),
														  Ok               (source.Ok),
														  Sorted           (source.Sorted),
														  Indexed          (source.Indexed),
														  TableId          (source.TableId),
														  CodeKeyField     (source.CodeKeyField),
														  CurrentCodeValue (source.CurrentCodeValue),
														  CurrentRecordNbr (source.CurrentRecordNbr),
														  SortFunctor      (source.SortFunctor),
														  CodeKeyStack     (source.CodeKeyStack),
														  CsvStatus        (source.CsvStatus)
{
}
TcsEpsgTable::~TcsEpsgTable (void)
{
	// Nothing to do here (yet).
}
TcsEpsgTable& TcsEpsgTable::operator= (const TcsEpsgTable& rhs)
{
	if (&rhs != this)
	{
		TcsCsvFileBase::operator= (rhs);
		Ok               = rhs.Ok;
		Sorted           = rhs.Sorted;
		Indexed          = rhs.Indexed;
		TableId          = rhs.TableId;
		CodeKeyField     = rhs.CodeKeyField;
		CurrentCodeValue = rhs.CurrentCodeValue;
		CurrentRecordNbr = rhs.CurrentRecordNbr;
		SortFunctor      = rhs.SortFunctor;
		CodeKeyStack     = rhs.CodeKeyStack;
		CsvStatus        = rhs.CsvStatus;
	}
	return *this;
}
bool TcsEpsgTable::SetCurrentRecord (const TcsEpsgCode& epsgCode)
{
	bool ok;
	short fieldNbr;
	unsigned recordNumber (InvalidRecordNbr);
	wchar_t srchString [32];

	ok = epsgCode.IsValid ();
	if (ok)
	{
		ok = (CurrentCodeValue == epsgCode) && (CurrentRecordNbr != InvalidRecordNbr);
		if (!ok)
		{
			CurrentRecordNbr = recordNumber = InvalidRecordNbr;

			epsgCode.AsString (srchString,wcCount (srchString));
 			fieldNbr = GetEpsgFieldNumber (TableId,CodeKeyField);
			if (fieldNbr >= 0)
			{
				ok = false;
				if (Indexed)
				{
					ok = Locate (recordNumber,srchString);
				}
				if (!ok)
				{
					ok = Locate (recordNumber,fieldNbr,srchString);
				}
				if (ok)
				{
					CurrentCodeValue = epsgCode;
					CurrentRecordNbr = recordNumber;
				}
			}
		}
	}
	if (!ok)
	{
		CurrentCodeValue = 0UL;
		CurrentRecordNbr = InvalidRecordNbr;
	}
	return ok;
}
bool TcsEpsgTable::PushCurrentPosition (void)
{
	CodeKeyStack.push (CurrentCodeValue);
	return true;
}
bool TcsEpsgTable::RestorePreviousPosition (void)
{
	bool ok (false);
	TcsEpsgCode currentCodeValue;

	if (!CodeKeyStack.empty ())
	{
		currentCodeValue = CodeKeyStack.top ();
		CodeKeyStack.pop ();
		if (currentCodeValue.IsValid ())
		{
			ok = SetCurrentRecord (currentCodeValue);
		}
	}
	return ok;
}
bool TcsEpsgTable::EpsgLocateCode (TcsEpsgCode& epsgCode,EcsEpsgField fieldId,const wchar_t* fldValue)
{
	bool ok;
	bool deprecated;
	std::wstring fldData;

	short deprecatedFldNbr = GetEpsgFieldNumber (TableId,epsgFldDeprecated);
	short locateFldNbr = GetEpsgFieldNumber (TableId,fieldId);
	short codeFldNbr = GetEpsgCodeFieldNbr (TableId);

	epsgCode = 0UL;
	ok = (locateFldNbr >= 0);
	unsigned recCnt = RecordCount ();
	for (unsigned recNbr = 0; ok && recNbr < recCnt;++recNbr)
	{
		if (deprecatedFldNbr >= 0)
		{
			ok = GetField (fldData,recNbr,deprecatedFldNbr);
			if (ok)
			{
				deprecated = (_wcsicmp (fldData.c_str (),LogicalTrue) == 0);
				if (deprecated)
				{
					continue;
				}
			}
		}
		if (ok)
		{
			ok = GetField (fldData,recNbr,locateFldNbr);
			if (ok)
			{
				if (_wcsicmp (fldData.c_str (),fldValue) == 0)
				{
					ok = GetField (fldData,recNbr,codeFldNbr);
					if (ok)
					{
						epsgCode = TcsEpsgCode (fldData);
					}
				}
			}
		}
	}
	return ok;
}
// PositionToFirst functions presume the field ID is not the index field and
// not the primary sort key.  That is, these 'PositionToFirst' functions do a
// linear search of the whole table.
bool TcsEpsgTable::PositionToFirst (EcsEpsgField fieldId,const wchar_t* fldValue,bool honorCase)
{
	unsigned recordNumber;
	std::wstring fldData;

	CurrentRecordNbr = InvalidRecordNbr;
	CurrentCodeValue = 0U;

	short fieldNbr = GetEpsgFieldNumber (TableId,fieldId);
	bool ok = (fieldNbr >= 0);
	if (ok)
	{
		ok = Locate (recordNumber,fieldNbr,fldValue,honorCase);
		if (ok)
		{
			CurrentRecordNbr = recordNumber;
			if (CodeKeyField != epsgFldNone)
			{
				short codeKeyFldNbr = GetEpsgCodeFieldNbr (TableId);
				ok = (codeKeyFldNbr >= 0);
				{
					ok = GetField (fldData,codeKeyFldNbr);
					if (ok)
					{
						CurrentCodeValue = TcsEpsgCode (fldData);
					}
				}
			}
		}
	}
	return ok;
}
bool TcsEpsgTable::PositionToFirst (EcsEpsgField fieldId,const TcsEpsgCode& epsgCode)
{
	std::wstring epsgCodeStr (epsgCode.AsWstring ());
	bool ok = PositionToFirst (fieldId,epsgCodeStr.c_str (),false);
	return ok;
}
bool TcsEpsgTable::PositionToNext (EcsEpsgField fieldId,const wchar_t* fldValue,bool honorCase)
{
	unsigned recordNumber;
	std::wstring fldData;

	short fieldNbr = GetEpsgFieldNumber (TableId,fieldId);
	bool ok = (CurrentRecordNbr != InvalidRecordNbr) && (fieldNbr >= 0);

	recordNumber = CurrentRecordNbr;
	CurrentRecordNbr = InvalidRecordNbr;
	CurrentCodeValue = 0U;
	if (ok)
	{
		ok = LocateNext (recordNumber,fieldNbr,fldValue,honorCase);
		if (ok)
		{
			CurrentRecordNbr = recordNumber;
			if (CodeKeyField != epsgFldNone)
			{
				short codeKeyFldNbr = GetEpsgCodeFieldNbr (TableId);
				ok = (codeKeyFldNbr >= 0);
				if (ok)
				{
					ok = GetField (fldData,codeKeyFldNbr);
					if (ok)
					{
						CurrentCodeValue = TcsEpsgCode (fldData);
					}
				}
			}
		}
	}
	return ok;
}
bool TcsEpsgTable::PositionToNext (EcsEpsgField fieldId,const TcsEpsgCode& epsgCode)
{
	std::wstring epsgCodeStr (epsgCode.AsWstring ());
	bool ok = PositionToNext (fieldId,epsgCodeStr.c_str (),false);
	return ok;
}
bool TcsEpsgTable::IsDeprecated (void)
{
	bool ok;
	bool deprecated (false);
	std::wstring fldData;

	if (CurrentRecordNbr != InvalidRecordNbr)
	{
		short fieldNbr = GetEpsgFieldNumber (TableId,epsgFldDeprecated);
		if (fieldNbr >= 0)
		{
			ok = GetField (fldData,fieldNbr);
			if (ok)
			{
				deprecated = (_wcsicmp (fldData.c_str (),LogicalTrue) == 0);
			}
		}
	}
	return deprecated;
}
bool TcsEpsgTable::GetField (std::wstring& result,short fieldNbr)
{
	bool ok (false);

	result.clear ();
	if (CurrentRecordNbr < RecordCount ())
	{
		ok = TcsCsvFileBase::GetField (result,CurrentRecordNbr,fieldNbr,CsvStatus);
	}
	return ok;
}
bool TcsEpsgTable::GetAsLong (long& result,short fieldNbr)
{
	std::wstring fldValue;

	bool ok = TcsCsvFileBase::GetField (fldValue,CurrentRecordNbr,fieldNbr,CsvStatus);
	if (ok)
	{
		result = wcstol (fldValue.c_str (),0,10);
	}
	return ok;
}
bool TcsEpsgTable::GetAsULong (unsigned long& result,short fieldNbr)
{
	std::wstring fldValue;

	bool ok = TcsCsvFileBase::GetField (fldValue,CurrentRecordNbr,fieldNbr,CsvStatus);
	if (ok)
	{
		result = wcstoul (fldValue.c_str (),0,10);
	}
	return ok;
}
bool TcsEpsgTable::GetAsEpsgCode (TcsEpsgCode& result,short fieldNbr)
{
	std::wstring fldValue;

	result = 0UL;	
	bool ok = TcsCsvFileBase::GetField (fldValue,CurrentRecordNbr,fieldNbr,CsvStatus);
	if (ok)
	{
		result = TcsEpsgCode (fldValue);
	}
	return ok;
}
bool TcsEpsgTable::GetAsReal (double& result,short fieldNbr)
{
	std::wstring fldValue;

	bool ok = TcsCsvFileBase::GetField (fldValue,CurrentRecordNbr,fieldNbr,CsvStatus);
	if (ok)
	{
		result = wcstod (fldValue.c_str (),0);
	}
	return ok;
}
bool TcsEpsgTable::GetAsLogical (bool& result,short fieldNbr)
{
	std::wstring fldValue;

	result = false;
	bool ok = TcsCsvFileBase::GetField (fldValue,CurrentRecordNbr,fieldNbr,CsvStatus);
	if (ok)
	{
		result = !_wcsicmp (fldValue.c_str(),LogicalTrue);
	}
	return ok;
}
bool TcsEpsgTable::GetField (std::wstring& result,EcsEpsgField fieldId)
{
	bool ok (false);

	short fieldNbr = GetEpsgFieldNumber (TableId,fieldId);
	if (fieldNbr >= 0)
	{
		ok = GetField (result,fieldNbr);
	}
	return ok;
}
bool TcsEpsgTable::GetAsLong (long& result,EcsEpsgField fieldId)
{
	bool ok (false);

	short fieldNbr = GetEpsgFieldNumber (TableId,fieldId);
	if (fieldNbr >= 0)
	{
		ok = GetAsLong (result,fieldNbr);
	}
	return ok;
}
bool TcsEpsgTable::GetAsULong (unsigned long& result,EcsEpsgField fieldId)
{
	bool ok (false);

	short fieldNbr = GetEpsgFieldNumber (TableId,fieldId);
	if (fieldNbr >= 0)
	{
		ok = GetAsULong (result,fieldNbr);
	}
	return ok;
}
bool TcsEpsgTable::GetAsEpsgCode (TcsEpsgCode& result,EcsEpsgField fieldId)
{
	bool ok (false);

	short fieldNbr = GetEpsgFieldNumber (TableId,fieldId);
	if (fieldNbr >= 0)
	{
		ok = GetAsEpsgCode (result,fieldNbr);
	}
	return ok;
}
bool TcsEpsgTable::GetAsReal (double& result,EcsEpsgField fieldId)
{
	bool ok (false);

	short fieldNbr = GetEpsgFieldNumber (TableId,fieldId);
	if (fieldNbr >= 0)
	{
		ok = GetAsReal (result,fieldNbr);
	}
	return ok;
}
bool TcsEpsgTable::GetField (std::wstring result,const TcsEpsgCode& epsgCode,short fieldNbr)
{
	bool ok = SetCurrentRecord (epsgCode);
	if (ok)
	{
		ok = GetField (result,fieldNbr);
	}
	return ok;
}
bool TcsEpsgTable::GetAsLong (long& result,const TcsEpsgCode& epsgCode,short fieldNbr)
{
	bool ok = SetCurrentRecord (epsgCode);
	if (ok)
	{
		ok = GetAsLong (result,fieldNbr);
	}
	return ok;
}
bool TcsEpsgTable::GetAsULong (unsigned long& result,const TcsEpsgCode& epsgCode,short fieldNbr)
{
	bool ok = SetCurrentRecord (epsgCode);
	if (ok)
	{
		ok = GetAsULong (result,fieldNbr);
	}
	return ok;
}
bool TcsEpsgTable::GetAsEpsgCode (TcsEpsgCode& result,const TcsEpsgCode& epsgCode,short fieldNbr)
{
	bool ok = SetCurrentRecord (epsgCode);
	if (ok)
	{
		ok = GetAsEpsgCode (result,fieldNbr);
	}
	return ok;
}
bool TcsEpsgTable::GetAsReal (double& result,const TcsEpsgCode& epsgCode,short fieldNbr)
{
	bool ok = SetCurrentRecord (epsgCode);
	if (ok)
	{
		ok = GetAsReal (result,fieldNbr);
	}
	return ok;
}
const TcsCsvStatus& TcsEpsgTable::GetCsvStatus () const
{
    return CsvStatus;
}
//=============================================================================
// Private Support Functions
bool TcsEpsgTable::PrepareCsvFile ()
{
	bool ok (true);

	if (SortFunctor.FirstField >= 0)
	{
		Sorted = StableSort (SortFunctor);
		ok = Sorted;
	}
	if (ok && CodeKeyField != epsgFldNone)
	{
		short fieldNbr = GetEpsgFieldNumber (TableId,CodeKeyField);
		ok = SetRecordKeyField (fieldNbr,CsvStatus);
		Indexed = ok;
	}
	return ok;
}
//newPage//
//=============================================================================
// TcsEpsgDataSetV6  -  An EPSG dataset based on the version 6 model.
//
// An image of the dataset in .csv form is expected to reside in the directory
// provided to the constructor.  The object is intended to be a read only object
// but nothing specific was done to preclude changing the underlying tables or
// writing changes back to the .csv files.  There are just no member functions
// at this time to support such operation.
//
//=============================================================================
// Static Constants, Variables, and Member Functions
const TcsEpsgDataSetV6::TcsCsMapDtmCodeMap TcsEpsgDataSetV6::KcsCsMapDtmCodeMap [] =
{
	// These entries are of the null datum shift type.
	{   9603UL,   4283UL,   4326UL,   cs_DTCTYP_GDA94  },
	{   9603UL,   4167UL,   4326UL,   cs_DTCTYP_NZGD2K },
	{   9603UL,   4171UL,   4326UL,   cs_DTCTYP_RGF93  },
	{   9603UL,   4258UL,   4326UL,   cs_DTCTYP_ETRF89 },
	{   9603UL,   4269UL,   4326UL,   cs_DTCTYP_WGS84  },	// NAD83 to WGS84
//	{   9603UL,   4148UL,   4326UL,   cs_DTCTYP_WGS84  },	// Hartebeesthoek94 to WGS84
	{   9603UL,   4612UL,   4326UL,   cs_DTCTYP_WGS84  },	// JGD2000

	// The following entries are of the datum shift file type.
	{   9613UL,   4267UL,   4326UL,   cs_DTCTYP_NAD27  },	// NADCON: NAD27
	{   9613UL,   4152UL,   4326UL,   cs_DTCTYP_HPGN   },	// NADCON: HARN
//	{   9614UL,   4267UL,   4326UL,   cs_DTCTYP_NAD27  },	// NTv1: NAD27
	{   9615UL,   4267UL,   4326UL,   cs_DTCTYP_NAD27  },	// NTv2: NAD27
	{   9615UL,   4617UL,   4326UL,   cs_DTCTYP_CSRS   },	// NTv2: CSRS
//	{   9633UL,   0000UL,   0000UL,   cs_DTCTYP_?????  },	// OSNT
//	{   9634UL,   0000UL,   0000UL,   cs_DTCTYP_?????  },	// Maritime TRANSFORM:
	{   9655UL,   4275UL,   4326UL,   cs_DTCTYP_RGF93  },	// French:
	{      0UL,      0UL,      0UL,   cs_DTCTYP_NONE   },    
};
short TcsEpsgDataSetV6::GetFldNbr (EcsEpsgTable tableId,EcsEpsgField fieldId)
{
	short rtnValue = -1;
	const TcsEpsgFldMap* tblPtr;

	for (tblPtr = KcsEpsgFldMap;tblPtr->TableId != epsgTblUnknown;++tblPtr)
	{
		if (tblPtr->TableId == tableId && tblPtr->FieldId == fieldId)
		{
			rtnValue = tblPtr->FieldNbr;
			break;
		}
	}
	return rtnValue;
}
short TcsEpsgDataSetV6::GetFldName (std::wstring& fieldName,EcsEpsgTable tableId,EcsEpsgField fieldId)
{
	short rtnValue = -1;
	const TcsEpsgFldMap* tblPtr;

	fieldName.clear ();
	for (tblPtr = KcsEpsgFldMap;tblPtr->TableId != epsgTblUnknown;++tblPtr)
	{
		if (tblPtr->TableId == tableId && tblPtr->FieldId == fieldId)
		{
			fieldName = tblPtr->FieldName;
			rtnValue = tblPtr->FieldNbr;
			break;
		}
	}
	return rtnValue;
}
//=============================================================================
// Construction, Destruction, & Assignment
TcsEpsgDataSetV6::TcsEpsgDataSetV6 (const wchar_t* databaseFolder,const wchar_t* revLevel)
																	:
																  RevisionLevel  (revLevel),
																  DatabaseFolder (databaseFolder),
																  EpsgTables     ()
{
	const TcsEpsgTblMap* tblPtr = KcsEpsgTblMap;

	for (tblPtr = KcsEpsgTblMap;tblPtr->TableId != epsgTblUnknown;++tblPtr)
	{
		TcsEpsgTable* nextTable = new TcsEpsgTable (*tblPtr,DatabaseFolder.c_str ());
		EpsgTables.insert (std::make_pair(tblPtr->TableId,nextTable));
	}
}
TcsEpsgDataSetV6::TcsEpsgDataSetV6 (const TcsEpsgDataSetV6& source) : RevisionLevel  (source.RevisionLevel),
																	  DatabaseFolder (source.DatabaseFolder),
																	  EpsgTables     (source.EpsgTables)
{
}																	  
TcsEpsgDataSetV6::~TcsEpsgDataSetV6 (void)
{
	std::map<EcsEpsgTable,TcsEpsgTable*>::iterator itr;
	
	for (itr = EpsgTables.begin ();itr != EpsgTables.end ();++itr)
	{
		TcsEpsgTable* tblPtr = itr->second;
		delete tblPtr;
	}
}
TcsEpsgDataSetV6& TcsEpsgDataSetV6::operator= (const TcsEpsgDataSetV6& rhs)
{
	if (&rhs != this)
	{
		RevisionLevel  = rhs.RevisionLevel;
		DatabaseFolder = rhs.DatabaseFolder;
		EpsgTables     = rhs.EpsgTables;		// ouch!!!
	}
	return *this;
}
//=============================================================================
// Public Named Member Functions
TcsEpsgTable* TcsEpsgDataSetV6::GetTablePtr (EcsEpsgTable tableId)
{
	TcsEpsgTable* tblPtr = EpsgTables [tableId];
	return tblPtr;
}
bool TcsEpsgDataSetV6::GetField (std::wstring result,EcsEpsgTable tableId,EcsEpsgField fieldId)
{
	bool ok (false);
	short fieldNbr;

	result.clear ();
	fieldNbr = GetFldNbr (tableId,fieldId);
	if (fieldNbr >= 0)
	{
		TcsEpsgTable* tblPtr = GetTablePtr (tableId);
		if (tblPtr != 0)
		{
			ok = tblPtr->GetField (result,fieldNbr);
		}
	}
	return ok;
}
bool TcsEpsgDataSetV6::GetFieldAsLong (long& result,EcsEpsgTable tableId,EcsEpsgField fieldId)
{
	bool ok (false);
	short fieldNbr;

	fieldNbr = GetFldNbr (tableId,fieldId);
	if (fieldNbr >= 0)
	{
		TcsEpsgTable* tblPtr = GetTablePtr (tableId);
		if (tblPtr != 0)
		{
			ok = tblPtr->GetAsLong (result,fieldNbr);
		}
	}
	return ok;
}
bool TcsEpsgDataSetV6::GetFieldAsULong (unsigned long& result,EcsEpsgTable tableId,EcsEpsgField fieldId)
{
	bool ok (false);
	short fieldNbr;

	fieldNbr = GetFldNbr (tableId,fieldId);
	if (fieldNbr >= 0)
	{
		TcsEpsgTable* tblPtr = GetTablePtr (tableId);
		if (tblPtr != 0)
		{
			ok = tblPtr->GetAsULong (result,fieldNbr);
		}
	}
	return ok;
}
bool TcsEpsgDataSetV6::GetFieldAsReal (double& result,EcsEpsgTable tableId,EcsEpsgField fieldId)
{
	bool ok (false);
	short fieldNbr;

	fieldNbr = GetFldNbr (tableId,fieldId);
	if (fieldNbr >= 0)
	{
		TcsEpsgTable* tblPtr = GetTablePtr (tableId);
		if (tblPtr != 0)
		{
			ok = tblPtr->GetAsReal (result,fieldNbr);
		}
	}
	return ok;
}
bool TcsEpsgDataSetV6::GetFieldAsEpsgCode (TcsEpsgCode& result,EcsEpsgTable tableId,EcsEpsgField fieldId)
{
	bool ok (false);
	short fieldNbr;

	fieldNbr = GetFldNbr (tableId,fieldId);
	if (fieldNbr >= 0)
	{
		TcsEpsgTable* tblPtr = GetTablePtr (tableId);
		if (tblPtr != 0)
		{
			ok = tblPtr->GetAsEpsgCode (result,fieldNbr);
		}
	}
	return ok;	
}
bool TcsEpsgDataSetV6::GetUomToDegrees (double& toDegrees,TcsEpsgCode uomCode)
{
	bool ok (false);
	short typFldNbr;
	short bFldNbr;
	short cFldNbr;
	double factorB;
	double factorC;
	std::wstring unitType;

	TcsEpsgTable* uomTblPtr = GetTablePtr (epsgTblUnitOfMeasure);
	if (uomTblPtr != 0)
	{
		ok = uomTblPtr->SetCurrentRecord (uomCode);
		if (ok)
		{
			typFldNbr = GetFldNbr (epsgTblUnitOfMeasure,epsgFldUnitOfMeasType);
			ok = (typFldNbr >= 0);
			if (ok)
			{
				ok = uomTblPtr->GetField (unitType,typFldNbr);
				if (ok)
				{
					EcsUomType uomType = GetEpsgUomType (unitType.c_str ());
					ok = (uomType == epsgUomTypAngular);
				}
				if (ok)
				{
					bFldNbr = GetFldNbr (epsgTblUnitOfMeasure,epsgFldFactorB);
					cFldNbr = GetFldNbr (epsgTblUnitOfMeasure,epsgFldFactorC);
					ok = (bFldNbr >= 0) && (cFldNbr >= 0);
					if (ok)
					{
						ok  = uomTblPtr->GetAsReal (factorB,bFldNbr);
						ok &= uomTblPtr->GetAsReal (factorC,cFldNbr);   //lint !e514
						if (ok)
						{
							ok = (fabs (factorC) > 1.0E-12);
						}
						if (ok)
						{
							toDegrees = (factorB / factorC) * 57.29577951308238;
						}
					}
				}
			}	
		}
	}
	return ok;
}
bool TcsEpsgDataSetV6::GetUomToMeters (double& toMeters,TcsEpsgCode uomCode)
{
	bool ok (false);
	short typFldNbr;
	short bFldNbr;
	short cFldNbr;
	double factorB;
	double factorC;
	std::wstring unitType;

	TcsEpsgTable* uomTblPtr = GetTablePtr (epsgTblUnitOfMeasure);
	if (uomTblPtr != 0)
	{
		ok = uomTblPtr->SetCurrentRecord (uomCode);
		if (ok)
		{
			typFldNbr = GetFldNbr (epsgTblUnitOfMeasure,epsgFldUnitOfMeasType);
			ok = (typFldNbr >= 0);
			if (ok)
			{
			    ok = uomTblPtr->GetField (unitType,typFldNbr);
			    if (ok)
			    {
				    EcsUomType uomType = GetEpsgUomType (unitType.c_str ());
				    ok = (uomType == epsgUomTypLinear);
				    if (ok)
				    {
					    bFldNbr = GetFldNbr (epsgTblUnitOfMeasure,epsgFldFactorB);
					    cFldNbr = GetFldNbr (epsgTblUnitOfMeasure,epsgFldFactorC);
					    ok = (bFldNbr >= 0) && (cFldNbr >= 0);
					    if (ok)
					    {
						    ok  = uomTblPtr->GetAsReal (factorB,bFldNbr);
						    ok &= uomTblPtr->GetAsReal (factorC,cFldNbr);       //lint !e514
						    if (ok)
						    {
							    ok = (fabs (factorC) > 1.0E-12);
						    }
						    if (ok)
						    {
							    toMeters = (factorB / factorC);
						    }
					    }
				    }
			    }
			}
		}
	}
	return ok;
}
bool TcsEpsgDataSetV6::GetUomToUnity (double& toUnity,TcsEpsgCode uomCode)
{
	bool ok (false);
	short typFldNbr;
	short bFldNbr;
	short cFldNbr;
	double factorB;
	double factorC;
	std::wstring unitType;

	TcsEpsgTable* uomTblPtr = GetTablePtr (epsgTblUnitOfMeasure);
	if (uomTblPtr != 0)
	{
		ok = uomTblPtr->SetCurrentRecord (uomCode);
		if (ok)
		{
			typFldNbr = GetFldNbr (epsgTblUnitOfMeasure,epsgFldUnitOfMeasType);
			ok = (typFldNbr >= 0);
			if (ok)
			{
				ok = uomTblPtr->GetField (unitType,typFldNbr);
				if (ok)
				{
					EcsUomType uomType = GetEpsgUomType (unitType.c_str ());
					ok = (uomType == epsgUomTypScale);
				}
				if (ok)
				{
					bFldNbr = GetFldNbr (epsgTblUnitOfMeasure,epsgFldFactorB);
					cFldNbr = GetFldNbr (epsgTblUnitOfMeasure,epsgFldFactorC);
					ok = (bFldNbr >= 0) && (cFldNbr >= 0);
					if (ok)
					{
						ok  = uomTblPtr->GetAsReal (factorB,bFldNbr);
						ok &= uomTblPtr->GetAsReal (factorC,cFldNbr);       //lint !e514
						if (ok)
						{
							ok = (fabs (factorC) > 1.0E-14);
						}
						if (ok)
						{
							toUnity = (factorB / factorC);
						}
					}
				}
			}
		}
	}
	return ok;
}
bool TcsEpsgDataSetV6::GetFieldAsDegrees (double& result,EcsEpsgTable tableId,EcsEpsgField fieldId,TcsEpsgCode uomCode)
{
	bool ok (false);
	double realValue (0.0);
	wchar_t* wcPtr;

	std::wstring strValue;

	if (uomCode == 9110UL)
	{
		if (GetField (strValue,tableId,fieldId))
		{
			bool minus;
			size_t fillCnt;
			long degrees;
			long minutes;
			double seconds;

			wchar_t wcDegrees [16];
			wchar_t wcMinutes [16];
			wchar_t wcSeconds [32];
			wchar_t wrkBufr [64];

			wcsncpy (wrkBufr,strValue.c_str (),64);
			wrkBufr [63] = L'\0';
			fillCnt = 63 - wcslen (wrkBufr);
			wcsncat (wrkBufr,L"00000000000000000",fillCnt);
			wrkBufr [63] = L'\0';
			
			wcPtr = wcschr (wrkBufr,L'.');
			if (wcPtr != 0)
			{
				*wcPtr++ = L'\0';
				wcsncpy (wcDegrees,wrkBufr,16);
				wcDegrees [15] = L'\0';
				degrees = wcstol (wcDegrees,0,10);
				minus = degrees < 0L;
				if (minus)
				{
					degrees = -degrees;
				}
		
				wcMinutes [0] = *wcPtr++;
				wcMinutes [1] = *wcPtr++;
				wcMinutes [2] = L'\0';
				minutes = wcstol (wcMinutes,0,10);

				wcSeconds [0] = *wcPtr++;
				wcSeconds [1] = *wcPtr++;
				wcSeconds [2] = L'.';
				wcsncpy (wcSeconds,wcPtr,28);
				wcSeconds [31] = L'\0';
				seconds = wcstod (wcSeconds,0);
				
				result = static_cast<double>(degrees) + static_cast<double>(minutes) / 60.0 + seconds / 3600.0;
				if (minus)
				{
					result = -result;
				}
			}
		}	
	}
	if (uomCode == 9111UL)
	{
		if (GetField (strValue,tableId,fieldId))
		{
			bool minus;
			size_t fillCnt;
			long degrees;
			double minutes;

			wchar_t wcDegrees [16];
			wchar_t wcMinutes [32];
			wchar_t wrkBufr [64];

			wcsncpy (wrkBufr,strValue.c_str (),64);
			wrkBufr [63] = L'\0';
			fillCnt = 63 - wcslen (wrkBufr);
			wcsncat (wrkBufr,L"00000000000000000",fillCnt);
			wrkBufr [63] = L'\0';

			wcPtr = wcschr (wrkBufr,L'.');
			if (wcPtr != 0)
			{
				*wcPtr++ = L'\0';
				wcsncpy (wcDegrees,wrkBufr,16);
				wcDegrees [15] = L'\0';
				degrees = wcstol (wcDegrees,0,10);
				minus = degrees < 0L;
				if (minus)
				{
					degrees = -degrees;
				}
		
				wcMinutes [0] = *wcPtr++;
				wcMinutes [1] = *wcPtr++;
				wcMinutes [2] = L'.';
				wcsncpy (wcMinutes,wcPtr,28);
				wcMinutes [31] = L'\0';
				minutes = wcstod (wcMinutes,0);
				
				result = static_cast<double>(degrees) + minutes / 60.0;
				if (minus)
				{
					result = -result;
				}
			}
		}	
	}
	else if ((uomCode >= 9101UL && uomCode <= 9106UL) ||
	         (uomCode >= 9112UL && uomCode <= 9114UL))
	{
		ok = GetFieldAsReal (realValue,tableId,fieldId);
		if (ok)
		{
			double uomFactor;
			ok = GetUomToDegrees (uomFactor,uomCode);
			if (ok && uomFactor != 0.0)
			{
				result = realValue * uomFactor;
			}
		}
	}
	return ok;
}
bool TcsEpsgDataSetV6::GetFieldAsMeters (double& result,EcsEpsgTable tableId,EcsEpsgField fieldId,TcsEpsgCode uomCode)
{
	bool ok;
	double uomFactor;
	double realValue;
	
	ok = GetFieldAsReal (realValue,tableId,fieldId);
	if (ok)
	{
		ok = GetUomToMeters (uomFactor,uomCode);	
		if (ok && uomFactor != 0.0)
		{
			result = realValue * uomFactor;
		}
	}
	return ok;
}
#pragma message ("Development in progress, currently incomplete; and untested.")
//bool TcsEpsgDataSetV6::FieldToReal (double& result,TcsEpsgCode trgUomCode,const wchar_t* fldData,TcsEpsgCode srcUomCode)
//{
//	bool ok;
//	bool okB;
//	bool okC;
//
//	EcsUomType srcType;
//	EcsUomType trgType;
//	
//	double srcFactor;
//	double trgFactor;
//	double factorB;
//	double factorC;
//
//	std::wstring fldData;
//	
//	const TcsEpsgTable* uomTblPtr = GetTablePtr (epsgTblUnitOfMeasure);
//	ok = (uomTblPtr != 0);
//	
//	if (ok)
//	{
//		ok = uomTblPtr->SetCurrentRecord (srcUomCode);
//	}
//	if (ok)
//	{
//		ok = uomTblPtr->GetField (fldData,epsgFldUnitOfMeasType);
//		if (ok)
//		{
//			srcType = GetEpsgUomType (fldData.c_str ());
//			okB = uomTblPtr->GetAsReal (factorB,epsgFldFactorB);
//			okC = uomTblPtr->GetAsReal (factorC,epsgFldFactorC);
//			if (okB && okC && fabs (factorC) > 1.0E-12)
//			{
//				srcFactor = factorB / factorC;
//			}
//			else if (okB)
//			{
//				srcFactor = factorB;
//			}
//			else
//			{
//				srcFactor = cs_One;
//			}
//			if (srcType == epsgUomTypAngular)
//			{
//				srcFactor *= 57.29577951308238;
//			}
//		}
//	}
//	if (ok)
//	{
//		ok = uomTblPtr->SetCurrentRecord (trgUomCode);
//	}
//	if (ok)
//	{
//		ok = uomTblPtr->GetField (fldData,epsgFldUnitOfMeasType);
//		if (ok)
//		{
//			trgType = GetEpsgUomType (fldData.c_str ());
//			okB = uomTblPtr->GetAsReal (factorB,epsgFldFactorB);
//			okC = uomTblPtr->GetAsReal (factorC,epsgFldFactorC);
//			if (okB && okC && fabs (factorC) > 1.0E-12)
//			{
//				trgFactor = factorB / factorC;
//			}
//			else if (okB)
//			{
//				trgFactor = factorB;
//			}
//			else
//			{
//				trgFactor = cs_One;
//			}
//		}
//	}
//	return ok;
//}
//=============================================================================
// High Level API Functions
//
unsigned TcsEpsgDataSetV6::GetRecordCount (EcsEpsgTable tableId)
{
	unsigned recordCount (0);

	const TcsEpsgTable* epsgTblPtr = GetTablePtr (tableId);
	if (epsgTblPtr != 0)
	{
		recordCount = epsgTblPtr->RecordCount ();
	}
	return recordCount;
}
bool TcsEpsgDataSetV6::GetFieldByIndex (std::wstring& result,EcsEpsgTable tableId,EcsEpsgField fieldId,unsigned recNbr)
{
	bool ok (false);
	short fldNbr;
	TcsEpsgTable* epsgTblPtr;

	epsgTblPtr = GetTablePtr (tableId);
	if (epsgTblPtr != 0)
	{
		fldNbr = GetFldNbr (tableId,fieldId);
		if (fldNbr >= 0)
		{
			TcsCsvStatus csvStatus (epsgTblPtr->GetCsvStatus ());
			ok = static_cast<TcsCsvFileBase*>(epsgTblPtr)->GetField (result,recNbr,fldNbr,csvStatus);
		}
	}
	return ok;
}
bool TcsEpsgDataSetV6::GetCodeByIndex (TcsEpsgCode& epsgCode,EcsEpsgTable tableId,EcsEpsgField fieldId,unsigned recNbr)
{
	bool ok (false);
	short fldNbr;
	TcsEpsgTable* epsgTblPtr;
	std::wstring fieldData;

	epsgTblPtr = GetTablePtr (tableId);
	if (epsgTblPtr != 0)
	{
		fldNbr = GetFldNbr (tableId,fieldId);
		if (fldNbr >= 0)
		{
			TcsCsvStatus csvStatus (epsgTblPtr->GetCsvStatus ());
			ok = static_cast<TcsCsvFileBase*>(epsgTblPtr)->GetField (fieldData,recNbr,fldNbr,csvStatus);
			if (ok)
			{
				epsgCode = wcstoul (fieldData.c_str (),0,10);
			}
		}
	}
	return ok;
}
bool TcsEpsgDataSetV6::GetFieldByCode (std::wstring& result,EcsEpsgTable tableId,EcsEpsgField fieldId,TcsEpsgCode epsgCode)
{
	bool ok (false);
	short fldNbr;
	TcsEpsgTable* epsgTblPtr;

	epsgTblPtr = GetTablePtr (tableId);
	if (epsgTblPtr != 0)
	{
		fldNbr = GetFldNbr (tableId,fieldId);
		if (fldNbr >= 0)
		{
			ok = epsgTblPtr->SetCurrentRecord (epsgCode);
			if (ok)
			{
				ok = epsgTblPtr->GetField (result,fldNbr);
			}
		}
	}
	return ok;
}
bool TcsEpsgDataSetV6::CompareCsMapUnits (const struct cs_Unittab_* csMapUnitTbl,bool useNameMap)
{
	return true;
}
bool TcsEpsgDataSetV6::GetCsMapEllipsoid (struct cs_Eldef_& ellipsoid,TcsEpsgCode epsgCode)
{
	bool ok (false);

	TcsEpsgCode uomCode;

	double eRad (0.0);
	double pRad (0.0);
	double ecent (0.0);
	double flattening (0.0);
	double rFlattening (0.0);
	double eSq (0.0);
	std::wstring elpName;

	TcsEpsgTable* epsgTblPtr = GetTablePtr (epsgTblEllipsoid);
	if (epsgTblPtr != 0)
	{
		ok = epsgTblPtr->SetCurrentRecord (epsgCode);
		if (ok)
		{
			ok = epsgTblPtr->GetField (elpName,epsgFldEllipsoidName);
		}
		if (ok)
		{
			ok  = epsgTblPtr->GetAsEpsgCode (uomCode,epsgFldUomCode);
			if (ok)
			{
				ok  = GetFieldAsMeters (eRad,epsgTblEllipsoid,epsgFldSemiMajorAxis,uomCode);
				ok &= GetFieldAsMeters (pRad,epsgTblEllipsoid,epsgFldSemiMinorAxis,uomCode);    //lint !e514
				ok &= epsgTblPtr->GetAsReal (rFlattening,epsgFldInvFlattening);                 //lint !e514
				if (ok)
				{
					ok = (eRad != 0.0) && (pRad != 0.0 || rFlattening != 0.0);
				}
				if (ok)
				{
					if (rFlattening != 0.0)
					{
						flattening = 1.0 / rFlattening;
						pRad = eRad * (1.0 - flattening);
					}
					else
					{
						flattening = 1.0 - (pRad / eRad);
						rFlattening = 1.0 / flattening;
					}
					eSq = (2.0 * flattening) - (flattening * flattening);
					ecent = sqrt (eSq);
				}
			}
		}
		if (ok)
		{
			sprintf (ellipsoid.key_nm,"EPSG::%lu",static_cast<unsigned long>(epsgCode));
			ellipsoid.group [0] = '\0';
			ellipsoid.fill [0] = '\0';
			ellipsoid.fill [1] = '\0';
			ellipsoid.e_rad = eRad;
			ellipsoid.p_rad = pRad;
			ellipsoid.flat = flattening;
			ellipsoid.ecent = ecent;
			wcstombs (ellipsoid.name,elpName.c_str (),sizeof (ellipsoid.name));
			sprintf (ellipsoid.source,"Converted from EPSG %S by CS-MAP",RevisionLevel.c_str ());
			ellipsoid.protect = 0;
			ellipsoid.epsgNbr = static_cast<short>(epsgCode);
			ellipsoid.wktFlvr = wktFlvrEpsg;
			ellipsoid.fill01 = 0;
			ellipsoid.fill02 = 0;
			ellipsoid.fill03 = 0;
			ellipsoid.fill04 = 0;
			ellipsoid.fill05 = 0;
		}
	}
	return ok;
}
TcsEpsgCode TcsEpsgDataSetV6::LocateGeographicBase (EcsCrsType crsType,TcsEpsgCode datumCode)
{
	bool ok (false);
	bool deprecated (false);
	TcsEpsgCode geogCode (0UL);
	std::wstring fldData;

	TcsEpsgTable* crsTblPtr = GetTablePtr (epsgTblReferenceSystem);
	TcsCsvStatus lclStatus (crsTblPtr->GetCsvStatus ());

	short deprecatedFldNbr = GetEpsgFieldNumber (epsgTblReferenceSystem,epsgFldDeprecated);
	short crsKindFldNbr = GetEpsgFieldNumber (epsgTblReferenceSystem,epsgFldCoordRefSysKind);
	short datumFldNbr = GetEpsgFieldNumber (epsgTblReferenceSystem,epsgFldDatumCode);
	short codeKeyFldNbr = GetEpsgCodeFieldNbr (epsgTblReferenceSystem);

	ok = (crsTblPtr != 0) && (crsKindFldNbr >= 0) && (datumFldNbr >= 0) && (codeKeyFldNbr >= 0);
	if (ok)
	{
		ok = crsTblPtr->PositionToFirst (epsgFldDatumCode,datumCode);
	}
	while (ok)
	{
		if (deprecatedFldNbr >= 0)
		{
			ok = crsTblPtr->GetAsLogical (deprecated,deprecatedFldNbr);
			if (ok && deprecated)
			{
				ok = crsTblPtr->PositionToNext (epsgFldDatumCode,datumCode);
				continue;
			}
		}
		ok = crsTblPtr->GetField (fldData,crsKindFldNbr);
		if (ok)
		{
			EcsCrsType crsKind = GetEpsgCrsType (fldData.c_str ());
			if (crsKind == crsType)
			{
				ok = crsTblPtr->GetField (fldData,codeKeyFldNbr);
				if (ok)
				{
					geogCode = TcsEpsgCode (fldData);
					break;
				}
			}
		}
		ok = crsTblPtr->PositionToNext (epsgFldDatumCode,datumCode);
	}
	return geogCode;
}
bool TcsEpsgDataSetV6::LocateOperationVariants (unsigned& variantCount,unsigned variants[],
																	   unsigned variantsSize,
																	   EcsOpType opType,
																	   TcsEpsgCode sourceCode,
																	   TcsEpsgCode targetCode)
{
	bool ok (false);
	bool deprecated (false);

	short codeKeyFldNbr (-1);
	short deprecatedFldNbr (-1);
	TcsEpsgCode operationCode (0UL);
	TcsEpsgCode recSrcCode;
	TcsEpsgCode recTrgCode;
	unsigned long recVarCode;
	std::wstring fldData;

	deprecatedFldNbr = GetEpsgFieldNumber (epsgTblCoordinateOperation,epsgFldDeprecated);
	codeKeyFldNbr = GetEpsgCodeFieldNbr (epsgTblCoordinateOperation);
	TcsEpsgTable* copTblPtr = GetTablePtr (epsgTblCoordinateOperation);

	variantCount = 0;
	for (unsigned idx = 0;idx < variantsSize;++idx)
	{
		variants [idx] = 0;
	}
	ok = (copTblPtr != 0) && (codeKeyFldNbr >= 0);
	if (ok)
	{
		ok = copTblPtr->PositionToFirst (epsgFldSourceCrsCode,sourceCode);
		bool loopOk (true);
		while (loopOk)
		{
			if (deprecatedFldNbr >= 0)
			{
				ok = copTblPtr->GetAsLogical (deprecated,deprecatedFldNbr);
				if (ok && deprecated)
				{
					loopOk = copTblPtr->PositionToNext (epsgFldSourceCrsCode,sourceCode);
					continue;
				}
			}
			ok  = copTblPtr->GetAsEpsgCode (recSrcCode,epsgFldSourceCrsCode);
			ok &= copTblPtr->GetAsEpsgCode (recTrgCode,epsgFldTargetCrsCode);
			ok &= copTblPtr->GetAsULong (recVarCode,epsgFldCoordOpVariant);
			ok &= copTblPtr->GetField (fldData,epsgFldCoordOpType);
			EcsOpType recOpType = GetEpsgOpType (fldData.c_str ());
			if (ok && (recOpType == opType) && (recSrcCode == sourceCode) &&
											   (recTrgCode == targetCode))
			{
				if (variantCount < variantsSize)
				{
					variants [variantCount] = recVarCode;
				}
				variantCount++;
			}
			if (ok)
			{
				loopOk = copTblPtr->PositionToNext (epsgFldSourceCrsCode,sourceCode);
			}
			else
			{
				loopOk = false;
			}
		}
	}
	return ok;
}
TcsEpsgCode TcsEpsgDataSetV6::LocateOperation (EcsOpType opType,TcsEpsgCode sourceCode,TcsEpsgCode targetCode,long variant)
{
	bool ok;

	short codeKeyFldNbr (-1);
	short deprecatedFldNbr (-1);
	TcsEpsgCode operationCode (0UL);
	TcsEpsgCode recSrcCode;
	TcsEpsgCode recTrgCode;
	long recVarCode;
	std::wstring fldData;

	deprecatedFldNbr = GetEpsgFieldNumber (epsgTblCoordinateOperation,epsgFldDeprecated);
	codeKeyFldNbr = GetEpsgCodeFieldNbr (epsgTblCoordinateOperation);
	TcsEpsgTable* copTblPtr = GetTablePtr (epsgTblCoordinateOperation);

	ok = (copTblPtr != 0) && (codeKeyFldNbr >= 0);
	if (ok)
	{
		ok = copTblPtr->PositionToFirst (epsgFldSourceCrsCode,sourceCode);
		while (ok)
		{
			if (deprecatedFldNbr >= 0)
			{
				ok = copTblPtr->GetField (fldData,deprecatedFldNbr);
				if (ok)
				{
					if (_wcsicmp (fldData.c_str (),TcsEpsgTable::LogicalTrue) == 0)
					{
						ok = copTblPtr->PositionToNext (epsgFldSourceCrsCode,sourceCode);
						continue;
					}
				}
			}
			ok  = copTblPtr->GetAsEpsgCode (recSrcCode,epsgFldSourceCrsCode);
			ok &= copTblPtr->GetAsEpsgCode (recTrgCode,epsgFldTargetCrsCode);
			ok &= copTblPtr->GetAsLong (recVarCode,epsgFldCoordOpVariant);
			ok &= copTblPtr->GetField (fldData,epsgFldCoordOpType);
			EcsOpType recOpType = GetEpsgOpType (fldData.c_str ());
			if (ok && (recOpType == opType) && (recSrcCode == sourceCode) &&
											   (recTrgCode == targetCode) &&
											   (recVarCode == variant))
			{
				ok = (codeKeyFldNbr >= 0);
				if (ok)
				{
					ok = copTblPtr->GetAsEpsgCode (operationCode,codeKeyFldNbr);
					break;
				}
			}
			ok = copTblPtr->PositionToNext (epsgFldSourceCrsCode,sourceCode);
		}
	}
	return ok ? operationCode : TcsEpsgCode(0UL);
}
#pragma message ("Development in progress, currently incomplete; and untested.")
// The following returns an EPSG UOM code value for success, 0UL for failure.
TcsEpsgCode TcsEpsgDataSetV6::GetParameterValue (double& parameterValue,TcsEpsgCode opCode,
																		TcsEpsgCode opMethCode,
																		TcsEpsgCode prmCode)
{
	bool ok;
	
	TcsEpsgCode recOprCode;
	TcsEpsgCode recMthCode;
	TcsEpsgCode recPrmCode;
	TcsEpsgCode recUomCode (0UL);
	double prmValue;

	TcsEpsgTable* prmTblPtr = GetTablePtr (epsgTblParameterValue);
	
	ok = (prmTblPtr != 0);
	if (ok)
	{
		ok = prmTblPtr->PositionToFirst (epsgFldCoordOpCode,opCode);
		while (ok)
		{
			ok  = prmTblPtr->GetAsEpsgCode (recOprCode,epsgFldCoordOpCode);
			ok &= prmTblPtr->GetAsEpsgCode (recMthCode,epsgFldCoordOpMethodCode);
			if (ok)
			{
				ok = (recOprCode == opCode) && (recMthCode == opMethCode);
				if (!ok)
				{
					break;
				}
			}
			ok &= prmTblPtr->GetAsEpsgCode (recPrmCode,epsgFldParameterCode);
			if (ok && recPrmCode == prmCode)
			{
				ok = prmTblPtr->GetAsReal (prmValue,epsgFldParameterValue);
				if (ok)
				{
					ok = prmTblPtr->GetAsEpsgCode (recUomCode,epsgFldUomCode);
				}
				if (ok)
				{
					parameterValue = prmValue;
					break;
				}
			}
			if (ok)
			{
				ok = prmTblPtr->PositionToNext (epsgFldCoordOpCode,opCode);
			}
		}
	}
	return ok ? recUomCode : TcsEpsgCode(0UL);
}
bool TcsEpsgDataSetV6::GetCsMapDatum (struct cs_Dtdef_& datum,struct cs_Eldef_& ellipsoid,TcsEpsgCode epsgDtmCode,
																						  long instance)
{
	static const unsigned maxVariants = 128;

	bool ok (false);
	bool deprecated (false);
	bool coordFrame (false);

	short to84_via (cs_DTCTYP_NONE);
	short csPrmCount (0);
	
	unsigned variant;
	unsigned variantCount;

	EcsDtmType dtmType (epsgDtmTypNone);

	TcsEpsgCode ellpCode;
	TcsEpsgCode baseCode;
	TcsEpsgCode pmerCode;
	TcsEpsgCode operationCode (0UL);
	TcsEpsgCode opMthCode (0UL);
	TcsEpsgCode uomCode (0UL);

	double parmValue;

	unsigned variants [maxVariants];

	std::wstring dtmName;
	std::wstring fldData;
	std::wstring parm8656;
	std::wstring parm8657;
	std::wstring parm8658;

	const TcsCsMapDtmCodeMap* viaMapTblPtr;

	// Get pointers to the required tables.
	TcsEpsgTable* dtmTblPtr = GetTablePtr (epsgTblDatum);
	TcsEpsgTable* pmrTblPtr = GetTablePtr (epsgTblPrimeMeridian);
	TcsEpsgTable* crsTblPtr = GetTablePtr (epsgTblReferenceSystem);
	TcsEpsgTable* copTblPtr = GetTablePtr (epsgTblCoordinateOperation);
	TcsEpsgTable* pruTblPtr = GetTablePtr (epsgTblParameterUsage);
	TcsEpsgTable* prmTblPtr = GetTablePtr (epsgTblParameterValue);
	ok = (dtmTblPtr != 0) && (pmrTblPtr != 0) && (crsTblPtr != 0) && (copTblPtr != 0) &&
																	 (pruTblPtr != 0) &&
																	 (prmTblPtr != 0);

	if (ok) ok = dtmTblPtr->SetCurrentRecord (epsgDtmCode);
	if (ok)
	{
		// Get the type of the datum.  If its not "geodetic", we don't want to
		// deal with it; not yet anyway.
		ok = dtmTblPtr->GetField (fldData,epsgFldDatumType);
		if (ok)
		{
			dtmType = GetEpsgDtmType (fldData.c_str ());
			ok = (dtmType == epsgDtmTypGeodetic);
		}
	}
	if (ok)
	{
		// Do the ellipsoid.
		ok = dtmTblPtr->GetAsEpsgCode (ellpCode,epsgFldEllipsoidCode);
		if (ok)
		{
			// This function will position the ellipsoid table for us, should
			// we need it later.
			ok = GetCsMapEllipsoid (ellipsoid,ellpCode);
		}
	}
	if (ok)
	{    
		// We really don't need the prime meridian, but what the heck!
		ok = dtmTblPtr->GetAsEpsgCode (pmerCode,epsgFldPrimeMeridianCode);
		if (ok)
		{
			pmrTblPtr->SetCurrentRecord (pmerCode);
		}
	}

	// Now to the datum specifics.  This can get rather tricky.
	if (ok)
	{
		ok = dtmTblPtr->GetField (dtmName,epsgFldDatumName);

		// Need to locate "the" geographic coordinate reference system which
		// references this datum.
		baseCode = LocateGeographicBase (epsgCrsTypGeographic2D,epsgDtmCode);
		if (baseCode.IsNotValid ())
		{
			baseCode = LocateGeographicBase (epsgCrsTypGeographic3D,epsgDtmCode);
		}
		ok = baseCode.IsValid ();
		if (ok)
		{
			ok = crsTblPtr->SetCurrentRecord (baseCode);
		}

		if (ok)
		{
			ok = LocateOperationVariants (variantCount,variants,maxVariants,epsgOpTypTransformation,
																			baseCode,
																			4326UL);
			if (ok && variantCount == 0)
			{
				ok = LocateOperationVariants (variantCount,variants,maxVariants,epsgOpTypTransformation,
																				baseCode,
																				4326UL);
			}
			if (ok && variantCount == 0)
			{
				ok = false;
			}
		}
	
		if (ok)
		{
			if (instance < 0L || static_cast<unsigned>(instance) >= variantCount)
			{
				ok = false;
			}
		}
		if (ok)
		{
			// Now, we need to search the Coordinate Operation table for an
			// entry to converts the located base code to WGS84 (i.e. 4326).
			variant = variants [instance];
			operationCode = LocateOperation (epsgOpTypTransformation,baseCode,4326UL,variant);
			if (operationCode.IsNotValid ())
			{
				operationCode = LocateOperation (epsgOpTypConcatenated,baseCode,4326UL,variant);
			}
			ok = operationCode.IsValid ();
			if (ok)
			{
				
			}
		}

		// Position the Operation table to the located operation and extract the
		// method code.
		if (ok)
		{
			ok = copTblPtr->SetCurrentRecord (operationCode);
			if (ok)
			{
				ok = copTblPtr->GetAsEpsgCode (opMthCode,epsgFldCoordOpMethodCode);
			}
		}

		// OK, we have a Source CRS code, Target CRS code (4326), and a method
		// code.  Using the table defined above, we might be done if we find a
		// match.
		if (ok)
		{
			for (viaMapTblPtr = KcsCsMapDtmCodeMap;viaMapTblPtr->MethodCode != 0UL;viaMapTblPtr += 1)
			{
				if (viaMapTblPtr->MethodCode == opMthCode &&
					viaMapTblPtr->SourceBaseCode == baseCode &&
					viaMapTblPtr->TargetBaseCode == 4326UL)
				{
					csPrmCount = 0;
					to84_via = viaMapTblPtr->CsMapTo84Code;
					break;
				}
			}
		}

		// If we didn't find a match, we need to extract the method and
		// parameters from the EPSG table.
		if (ok && to84_via == cs_DTCTYP_NONE)
		{
			// Position the parameter usage table to the position of the first
			// parameter, although we don't use it, yet!!!
			if (ok)
			{
				ok = pruTblPtr->PositionToFirst (epsgFldCoordOpMethodCode,opMthCode);		
			}

			// Position the parameter value table to the position of the first
			// parameter.
			if (ok)
			{
				ok = prmTblPtr->PositionToFirst (epsgFldCoordOpCode,operationCode);
			}
		}

		// Map the EPSG Method Code to a CS-MAP to84_via value.
		if (ok && to84_via == 0)
		{
			switch (opMthCode) {
			case 9603:				// Geocentric Translation
				to84_via = cs_DTCTYP_3PARM;
				csPrmCount = 3;
				break;
			case 9604:				// Molodensky
				to84_via = cs_DTCTYP_MOLO;
				csPrmCount = 3;
				break;
			case 9605:				// Abridged Molodensky
				to84_via = cs_DTCTYP_MOLO;
				csPrmCount = 3;
				break;
			case 9606:				// Position Vector
				to84_via = cs_DTCTYP_BURS;
				csPrmCount = 7;
				break;
			case 9607:				// Coordinate Frame
				to84_via = cs_DTCTYP_BURS;
				csPrmCount = 7;
				coordFrame = true;
				break;

			case 9613:				// NADCON
			case 9614:				// NTv1
			case 9615:				// NTv2
			case 9634:				// Maritime TRANSFORM
			case 9655:				// French
			case 9633:				// OSNT
			default:
				ok = false;
				to84_via = cs_DTCTYP_NONE;
				break;
			}
		}
	}

	// If we are still OK, we now setup the general stuff in the cs_Dtdef_ structure.
	if (ok)
	{
		sprintf (datum.key_nm,"EPSG::%lu",static_cast<unsigned long>(epsgDtmCode));
		strncpy (datum.ell_knm,ellipsoid.key_nm,sizeof (datum.ell_knm));
		datum.ell_knm [sizeof (datum.ell_knm) - 1] = '\0';
		datum.group [0] = '\0';
		datum.locatn [0] = '\0';
		datum.cntry_st [0] = '\0';
		datum.fill [0] = '\0';
		datum.fill [1] = '\0';
		datum.fill [2] = '\0';
		datum.fill [3] = '\0';
		datum.fill [4] = '\0';
		datum.fill [5] = '\0';
		datum.fill [6] = '\0';
		datum.fill [7] = '\0';
		datum.delta_X  = 0.0;
		datum.delta_Y  = 0.0;
		datum.delta_Z  = 0.0;
		datum.rot_X    = 0.0;
		datum.rot_Y    = 0.0;
		datum.rot_Z    = 0.0;
		datum.bwscale  = 0.0;
		wcstombs (datum.name,dtmName.c_str (),sizeof (datum.name));
		sprintf (datum.source,"Converted from EPSG %S by CS-MAP",RevisionLevel.c_str ());
		datum.protect = 0;
		datum.to84_via = to84_via;
		datum.epsgNbr = static_cast<short>(epsgDtmCode);
		datum.wktFlvr = wktFlvrEpsg;
		datum.fill01 = variant;
		datum.fill02 = 0;
		datum.fill03 = 0;
		datum.fill04 = 0;
	}

	// OK, we now add the parameter values.
	if (ok && csPrmCount >= 3)
	{
		double toMeters (0.0);
		uomCode = GetParameterValue (parmValue,operationCode,opMthCode,8605UL);
		ok = (uomCode != 0UL);
		if (ok)
		{
			ok = GetUomToMeters (toMeters,uomCode);
			if (ok)
			{
				datum.delta_X = parmValue * toMeters;
			}
		}
		uomCode = GetParameterValue (parmValue,operationCode,opMthCode,8606UL);
		ok = (uomCode != 0UL);
		if (ok)
		{
			ok = GetUomToMeters (toMeters,uomCode);
			if (ok)
			{
				datum.delta_Y = parmValue * toMeters;
			}
		}
		uomCode = GetParameterValue (parmValue,operationCode,opMthCode,8607UL);
		ok = (uomCode != 0UL);
		if (ok)
		{
			ok = GetUomToMeters (toMeters,uomCode);
			if (ok)
			{
				datum.delta_Z = parmValue * toMeters;
			}
		}
	}	
	if (ok && csPrmCount >= 6)
	{
		double toDegrees (0.0);
		uomCode = GetParameterValue (parmValue,operationCode,opMthCode,8608UL);
		ok = (uomCode != 0UL);
		if (ok)
		{
			ok = GetUomToDegrees (toDegrees,uomCode);
			if (ok)
			{
				datum.rot_X = parmValue * toDegrees * 3600.0;
				if (coordFrame) datum.rot_X *= -1.0;
			}
		}
		uomCode = GetParameterValue (parmValue,operationCode,opMthCode,8609UL);
		ok = (uomCode != 0UL);
		if (ok)
		{
			ok = GetUomToDegrees (toDegrees,uomCode);
			if (ok)
			{
				datum.rot_Y = parmValue * toDegrees * 3600.0;
				if (coordFrame) datum.rot_Y *= -1.0;
			}
		}
		uomCode = GetParameterValue (parmValue,operationCode,opMthCode,8610UL);
		ok = (uomCode != 0UL);
		if (ok)
		{
			ok = GetUomToDegrees (toDegrees,uomCode);
			if (ok)
			{
				datum.rot_Z = parmValue * toDegrees * 3600.0;
				if (coordFrame) datum.rot_Z *= -1.0;
			}
		}
	}	
	if (ok && csPrmCount >= 7)
	{
		uomCode = GetParameterValue (parmValue,operationCode,opMthCode,8611UL);
		ok = (uomCode != 0UL);
		if (ok)
		{
			if (uomCode == 9202UL)
			{
				datum.bwscale = parmValue;
			}
			else if (uomCode == 9201UL || uomCode == 9203UL)
			{
				datum.bwscale = (1.0 - parmValue) * 1000000.0;
			}
			else
			{
				ok = false;
			}
		}		
	}
	return ok;
}
bool TcsEpsgDataSetV6::GetCsMapCoordsys (struct cs_Csdef_& coordsys,struct cs_Dtdef_& datum,struct cs_Eldef_& ellipsoid,TcsEpsgCode crsEpsgCode)
{
	bool ok (false);

	TcsEpsgCode dtmEpsgCode;
	TcsEpsgCode areaOfUseCode;

	EcsCrsType crsType (epsgCrsTypNone);
	
	std::wstring crsName;
	std::wstring fldData;

	// Initialize the return values so there is no chance of
	// confusion on an error condition.
	memset (&coordsys,'\0',sizeof (coordsys));
	memset (&datum,'\0',sizeof (datum));
	memset (&ellipsoid,'\0',sizeof (ellipsoid));

	// Get pointers to the required tables.
	TcsEpsgTable* crsTblPtr  = GetTablePtr (epsgTblReferenceSystem);
	TcsEpsgTable* areaTblPtr = GetTablePtr (epsgTblArea);
	ok = (crsTblPtr != 0) && (areaTblPtr != 0);

	// Verify that the provided code is OK, and the referenced CRS is not
	// deprecated.
	if (ok)
	{
		ok = crsTblPtr->SetCurrentRecord (crsEpsgCode);
		if (ok)
		{
			// We don't convert deprecated definitions.
			ok = !crsTblPtr->IsDeprecated ();
		}
	}

	// We'll need to know the type of this definition.
	if (ok)
	{
		// Get the type of the CRS.  If its not "Projected" or "Geographic2D",
		// we don't want to deal with it; not yet anyway.
		ok = crsTblPtr->GetField (fldData,epsgFldCoordRefSysKind);
		if (ok)
		{
			crsType = GetEpsgCrsType (fldData.c_str ());
			ok = (crsType == epsgCrsTypProjected) || (crsType == epsgCrsTypGeographic2D);
		}
	}

	// Extract the reference datum code and pass it on to GetCsMapDatum.
	// GetCsMapDatum also does the ellipsoid for us.
	if (ok)
	{
		ok = GetReferenceDatum (dtmEpsgCode,crsEpsgCode);
		if (ok)
		{
			ok = GetCsMapDatum (datum,ellipsoid,dtmEpsgCode);
		}
	}

	// Now for anything common to both geographic and projected.
	if (ok)
	{
		ok = crsTblPtr->GetField (crsName,epsgFldCoordRefSysName);
		if (ok)
		{
			sprintf (coordsys.key_nm,"EPSG::%lu",static_cast<unsigned long>(crsEpsgCode));
			if (static_cast<unsigned long>(dtmEpsgCode) >= 6001 &&
				static_cast<unsigned long>(dtmEpsgCode) <= 6099)
			{
				coordsys.dat_knm [0] = '\0';
				CS_stncp (coordsys.elp_knm,ellipsoid.key_nm,sizeof (coordsys.elp_knm));
			}
			else
			{
				CS_stncp (coordsys.dat_knm,datum.key_nm,sizeof (coordsys.dat_knm));
				coordsys.elp_knm [0] = '\0';
			}
			wcstombs (coordsys.desc_nm,crsName.c_str (),sizeof (coordsys.desc_nm));
			sprintf (coordsys.source,"Converted from EPSG %S by CS-MAP",RevisionLevel.c_str ());
			coordsys.epsgNbr = static_cast<short>(crsEpsgCode);
			coordsys.wktFlvr = 0;
		}
	}

	// Extract the area of use information from EPSG and enter it into the
	// the appropriate fields in coordsys.
	if (ok)
	{
		bool mmOk;
		double minLng, minLat;
		double maxLng, maxLat;
		// We don't do the xy_min or xy_max; so we turn them off.
		coordsys.xy_min [0] = cs_Zero;
		coordsys.xy_min [1] = cs_Zero;
		coordsys.xy_max [0] = cs_Zero;
		coordsys.xy_max [1] = cs_Zero;

		// We trun off the ll_min and ll_max in case there's a failure
		// in the code below.
		ok = crsTblPtr->GetAsEpsgCode (areaOfUseCode,epsgFldAreaOfUseCode);
		if (ok)
		{
			ok = areaTblPtr->SetCurrentRecord (areaOfUseCode);
			if (ok)
			{
				// Get the EPSG useful range values.
				mmOk  = areaTblPtr->GetAsReal (minLng,epsgFldAreaWestBoundLng);
				mmOk &= areaTblPtr->GetAsReal (maxLng,epsgFldAreaEastBoundLng);
				mmOk &= areaTblPtr->GetAsReal (minLat,epsgFldAreaSouthBoundLat);
				mmOk &= areaTblPtr->GetAsReal (maxLat,epsgFldAreaNorthBoundLat);
			}

			// See if they are all Ok and consistent.
			if (mmOk)
			{
				mmOk  = (minLng >= -180.0) && (minLng <= 180.0);
				mmOk &= (minLat >=  -90.0) && (minLat <=  90.0);
				mmOk &= (maxLng >= -180.0) && (maxLng <= 180.0);
				mmOk &= (maxLat >=  -90.0) && (maxLat <=  90.0);
				if (mmOk)
				{
					mmOk &= (minLng < maxLng)  && (minLat < maxLat);
				}
			}
			if (mmOk)
			{
				coordsys.ll_min [0] = minLng;
				coordsys.ll_min [1] = minLat;
				coordsys.ll_max [0] = maxLng;
				coordsys.ll_max [1] = maxLat;
			}
		}
	}

	// Do the stuff specific to the type.
	if (ok && crsType == epsgCrsTypGeographic2D)
	{
		ok = GeographicCoordsys (coordsys,crsEpsgCode);
	}
	else if (ok && crsType == epsgCrsTypProjected)
	{
		ok = ProjectedCoordsys (coordsys,crsEpsgCode);
	}
	return ok;
}
bool TcsEpsgDataSetV6::GetReferenceDatum (TcsEpsgCode& dtmEpsgCode,TcsEpsgCode crsEpsgCode)
{
	bool ok;
	TcsEpsgCode baseCode;
	EcsCrsType crsType (epsgCrsTypNone);
	std::wstring fldData;
	
	TcsEpsgTable* crsTblPtr = GetTablePtr (epsgTblReferenceSystem);
	TcsEpsgTable* dtmTblPtr = GetTablePtr (epsgTblDatum);

	dtmEpsgCode.Invalidate ();
	ok = (crsTblPtr != 0) && (dtmTblPtr != 0);
	if (ok)
	{
		crsTblPtr->PushCurrentPosition ();
		ok = crsTblPtr->SetCurrentRecord (crsEpsgCode);
		if (ok)
		{
			// We don't convert deprecated definitions.
			ok = !crsTblPtr->IsDeprecated ();
		}
		if (ok)
		{
			// Get the type of the CRS.
			ok = crsTblPtr->GetField (fldData,epsgFldCoordRefSysKind);
			if (ok)
			{
				crsType = GetEpsgCrsType (fldData.c_str ());
				ok = (crsType == epsgCrsTypProjected) || (crsType == epsgCrsTypGeographic2D);
			}
		}

		// Extract the reference datum code.
		if (ok)
		{
			if (crsType == epsgCrsTypProjected)
			{
				// For the projected type, we first need to locate the CRS given as
				// the base.
				ok = crsTblPtr->GetAsEpsgCode (baseCode,epsgFldSourceGeogCrsCode);
				if (ok)
				{
					crsTblPtr->PushCurrentPosition ();
					ok = crsTblPtr->PositionToFirst (epsgFldCoordRefSysCode,baseCode);
					if (ok)
					{
						ok = crsTblPtr->GetAsEpsgCode (dtmEpsgCode,epsgFldDatumCode);
					}
					crsTblPtr->RestorePreviousPosition ();
				}
			}
			else if (crsType == epsgCrsTypGeographic2D)
			{
				ok = crsTblPtr->GetAsEpsgCode (dtmEpsgCode,epsgFldDatumCode);
			}
			ok = dtmEpsgCode.IsValid ();
		}
		crsTblPtr->RestorePreviousPosition ();
	}
	return ok;
}
// All EPSG longitude parameters are relative to the prime meridian.  SO it is
// important that the prime meridian be made easily available.
bool TcsEpsgDataSetV6::GetPrimeMeridian (double& primeMeridian,TcsEpsgCode crsEpsgCode)
{
	bool ok;
	TcsEpsgCode dtmEpsgCode;
	TcsEpsgCode pmrEpsgCode;
	TcsEpsgCode uomEpsgCode;
	
	TcsEpsgTable* dtmTblPtr = GetTablePtr (epsgTblDatum);
	TcsEpsgTable* pmrTblPtr = GetTablePtr (epsgTblPrimeMeridian);
	TcsEpsgTable* uomTblPtr = GetTablePtr (epsgTblUnitOfMeasure);
	ok = (dtmTblPtr != 0) && (pmrTblPtr != 0) && (uomTblPtr != 0);

	primeMeridian = -360.0;			// sure to be noticed if ever used by mistake.
	
	// Extract the datum reference code from the indicated crs entry.
	if (ok)
	{
		ok = GetReferenceDatum (dtmEpsgCode,crsEpsgCode);
	}

	// Use this datum code to extract the prime meridian in degrees.
	if (ok)
	{
		dtmTblPtr->PushCurrentPosition ();
		ok = dtmTblPtr->SetCurrentRecord (dtmEpsgCode);
		if (ok)
		{
			ok = dtmTblPtr->GetAsEpsgCode (pmrEpsgCode,epsgFldPrimeMeridianCode);
			if (ok)
			{
				pmrTblPtr->PushCurrentPosition ();
				ok = pmrTblPtr->SetCurrentRecord (pmrEpsgCode);
				if (ok)
				{
					double primeMeridian;
					ok = pmrTblPtr->GetAsReal (primeMeridian,epsgFldGreenwichLongitude);
					if (ok)
					{
						ok = pmrTblPtr->GetAsEpsgCode (uomEpsgCode,epsgFldUomCode);
					}
					if (ok)
					{
						double factor;
						ok = GetUomToDegrees (factor,uomEpsgCode);
						if (ok)
						{
							primeMeridian *= factor;
						}
					}
					
				}
				pmrTblPtr->RestorePreviousPosition ();
			}
		}
		dtmTblPtr->RestorePreviousPosition ();
	}
	return ok;
}
bool TcsEpsgDataSetV6::GetCoordsysQuad (short& quad,TcsEpsgCode crsEpsgCode)
{
	bool ok;
	bool swap;

	TcsEpsgCode sysEpsgCode;
	TcsEpsgCode uomEpsgCode;

	EcsOrientation axisOne (epsgOrntUnknown);
	EcsOrientation axisTwo (epsgOrntUnknown);
	EcsOrientation axisTmp;

	quad = 0;
	TcsEpsgTable* crsTblPtr = GetTablePtr (epsgTblReferenceSystem);
	TcsEpsgTable* axsTblPtr = GetTablePtr (epsgTblAxis);

	ok = (crsTblPtr != 0) && (axsTblPtr != 0);
	if (ok)
	{
		crsTblPtr->PushCurrentPosition ();
		ok = crsTblPtr->SetCurrentRecord (crsEpsgCode);
		if (ok)
		{
			std::wstring fldData;
			ok = crsTblPtr->GetAsEpsgCode (sysEpsgCode,epsgFldCoordSysCode);
			if (ok)
			{
				ok = axsTblPtr->PositionToFirst (epsgFldCoordSysCode,sysEpsgCode);
				if (ok)
				{
					ok = axsTblPtr->GetField (fldData,epsgFldCoordAxisOrientation);
					if (ok) axisOne = GetOrientation (fldData.c_str ());
				}
			}
			if (ok)
			{
				ok = axsTblPtr->PositionToNext (epsgFldCoordSysCode,sysEpsgCode);
				if (ok)
				{
					ok = axsTblPtr->GetField (fldData,epsgFldCoordAxisOrientation);
					if (ok) axisTwo = GetOrientation (fldData.c_str ());
				}
			}
		}
		crsTblPtr->RestorePreviousPosition ();
	}
	ok = (axisOne != epsgOrntUnknown) && (axisTwo != epsgOrntUnknown);
	if (ok)
	{
		swap = (axisOne == epsgOrntNorth) || (axisOne == epsgOrntSouth);
		if (swap)
		{
			axisTmp = axisOne;
			axisOne = axisTwo;
			axisTwo = axisOne;
		}

		if (axisOne == epsgOrntEast && axisTwo == epsgOrntNorth)
		{
			quad = 1;
		}
		else if (axisOne == epsgOrntWest && axisTwo == epsgOrntNorth)
		{
			quad = 2;
		}
		else if (axisOne == epsgOrntWest && axisTwo == epsgOrntSouth)
		{
			quad = 3;
		}
		else if (axisOne == epsgOrntEast && axisTwo == epsgOrntSouth)
		{
			quad = 4;
		}
		if (quad == 0)
		{
			ok = false;
		}
		else
		{
			if (swap)
			{
				quad = -quad;
			}
		}
	}
	return ok;
}
bool TcsEpsgDataSetV6::GeographicCoordsys (struct cs_Csdef_& coordsys,TcsEpsgCode crsEpsgCode)
{
	bool ok;

	TcsEpsgCode dtmEpsgCode;
	TcsEpsgCode sysEpsgCode;
	TcsEpsgCode uomEpsgCode;
	TcsEpsgCode pmrEpsgCode;
	TcsEpsgCode baseCode;

	double primeMeridian;			// in degrees
	
	EcsCrsType crsType (epsgCrsTypNone);
	std::wstring fldData;

	// Here for Geographic specific stuff.
	// Get pointers to the required tables.
	TcsEpsgTable* crsTblPtr = GetTablePtr (epsgTblReferenceSystem);
	TcsEpsgTable* dtmTblPtr = GetTablePtr (epsgTblDatum);
	TcsEpsgTable* uomTblPtr = GetTablePtr (epsgTblUnitOfMeasure);
	TcsEpsgTable* pmrTblPtr = GetTablePtr (epsgTblPrimeMeridian);
	TcsEpsgTable* sysTblPtr = GetTablePtr (epsgTblCoordinateSystem);
	TcsEpsgTable* axsTblPtr = GetTablePtr (epsgTblAxis);

	ok = (crsTblPtr != 0) && (dtmTblPtr != 0) && (uomTblPtr != 0) && (pmrTblPtr != 0) &&
																	 (sysTblPtr != 0) &&
																	 (axsTblPtr != 0);

	// Initialize the following to default values.
	if (ok)
	{
		coordsys.org_lat  = cs_Zero;
		coordsys.x_off    = cs_Zero;
		coordsys.y_off    = cs_Zero;
		coordsys.scl_red  = cs_One;
		coordsys.unit_scl = cs_One;
		coordsys.map_scl  = cs_One;
		coordsys.scale    = cs_One;
		coordsys.zero [0] = 1.0E-12 * coordsys.unit_scl;
		coordsys.zero [1] = 1.0E-12 * coordsys.unit_scl;
		coordsys.order    = 0;
		coordsys.zones    = 0;
	}

	// We assume that when called, epsgTblReferenceSystem is properly
	// positioned.  It is required that this position is retained upon
	// return.
	if (ok)
	{
		CS_stncp (coordsys.prj_knm,"LL",sizeof (coordsys.prj_knm));
		CS_stncp (coordsys.group,"LL",sizeof (coordsys.group));

		// Determine the units by examining the UOM of the first axis.
		ok = crsTblPtr->GetAsEpsgCode (sysEpsgCode,epsgFldCoordSysCode);
		if (ok)
		{
			ok = axsTblPtr->PositionToFirst (epsgFldCoordSysCode,sysEpsgCode);
			if (ok)
			{
				ok = axsTblPtr->GetAsEpsgCode (uomEpsgCode,epsgFldUomCode);
				if (ok)
				{
					double factor;
					ok = GetUomToDegrees (factor,uomEpsgCode);
					if (ok)
					{
						coordsys.unit_scl = factor;
						const char* cp = CS_unitluByFactor (cs_UTYP_ANG,factor);
						ok = (cp != 0);
						if (ok)
						{
							CS_stncp (coordsys.unit,cp,sizeof (coordsys.unit));
						}
					}
				}
			}
		}
	}

	if (ok)
	{
		// Parm1 and parm2 are the longitude range parameters.  EOSG doesn't
		// support this feature, so we use the CS-MAP default values.  The
		// remainder of the parameters are not used in geographic systems.
		coordsys.prj_prm1 = cs_Zero;
		coordsys.prj_prm2 = cs_Zero;
	}
	
	if (ok)
	{
		// Org_lng is the prime meridian associated with this definition.  In
		// EPSG, the prime meridian code is stored in the datums table.
		ok = GetPrimeMeridian (primeMeridian,crsEpsgCode);
		if (ok)
		{
			coordsys.org_lng = primeMeridian;
		}
	}

	if (ok)
	{
		short quad;

		// CS-MAP's quad determines the order of the axes and the direction.
		ok = GetCoordsysQuad (quad,crsEpsgCode);
		if (ok)
		{
			coordsys.quad = quad;
		}
	}

	// The rest is pretty easy.
	if (ok)
	{
		coordsys.zero [0] = 1.0E-12 * coordsys.unit_scl;
		coordsys.zero [1] = 1.0E-12 * coordsys.unit_scl;
	}
	return ok;
}

bool TcsEpsgDataSetV6::ProjectedCoordsys (struct cs_Csdef_& coordsys,TcsEpsgCode crsEpsgCode)
{
	bool ok;

	unsigned long flags (0UL);

	int minZprec (3);
	int utmZone;

	long lngTemp;

	TcsEpsgCode sysEpsgCode;
	TcsEpsgCode copEpsgCode;
	TcsEpsgCode mthEpsgCode;
	TcsEpsgCode uomEpsgCode;
	TcsEpsgCode areaOfUseCode;

	EcsCrsType crsType (epsgCrsTypNone);

	double primeMeridian = cs_Zero;
	
	std::wstring fldData;
	std::wstring crsName;

	// Here for Projective specific stuff.
	// Get pointers to the required tables.
	TcsEpsgTable* crsTblPtr = GetTablePtr (epsgTblReferenceSystem);
	TcsEpsgTable* copTblPtr = GetTablePtr (epsgTblCoordinateOperation);
	TcsEpsgTable* uomTblPtr = GetTablePtr (epsgTblUnitOfMeasure);
	TcsEpsgTable* sysTblPtr = GetTablePtr (epsgTblCoordinateSystem);
	TcsEpsgTable* axsTblPtr = GetTablePtr (epsgTblAxis);

	ok = (crsTblPtr != 0) && (copTblPtr != 0) && (uomTblPtr != 0) && (sysTblPtr != 0) &&
																	 (axsTblPtr != 0);

	// We assume that when called, epsgTblReferenceSystem is properly
	// positioned.  It is required that this position is retained upon
	// return.
	if (ok)
	{
		ok = crsTblPtr->GetField (crsName,epsgFldCoordRefSysName);
	}

	// Determine the group to which this definition should belong.  This is
	// table driven and not very precise.
	if (ok)
	{
		// Set a default in case what appears below fails.
		CS_stncp (coordsys.group,"EPSGPRJ",sizeof (coordsys.group));
		
		// Get the AreaOfUse code from the 
		ok = crsTblPtr->GetAsEpsgCode (areaOfUseCode,epsgFldAreaOfUseCode);
		if (ok)
		{
			short shrtEpsgCode = static_cast<short>(areaOfUseCode);
			const TcmAreaToMsiGroupMap* tblPtr;

			for (tblPtr = KcmAreaToMsiGroupMap;tblPtr->epsgAreaFrom != 0;tblPtr += 1)
			{
				if (shrtEpsgCode >= tblPtr->epsgAreaFrom &&
					shrtEpsgCode <= tblPtr->epsgAreaTo)
				{
					CS_stncp (coordsys.group,tblPtr->msiGroup,sizeof (coordsys.group));
					break;
				}
			}
		}
	}

	// Determine the units by examining the UOM of the first axis.
	if (ok)
	{
		ok = crsTblPtr->GetAsEpsgCode (sysEpsgCode,epsgFldCoordSysCode);
		if (ok)
		{
			ok = axsTblPtr->PositionToFirst (epsgFldCoordSysCode,sysEpsgCode);
			if (ok)
			{
				ok = axsTblPtr->GetAsEpsgCode (uomEpsgCode,epsgFldUomCode);
				if (ok)
				{
					double factor;
					ok = GetUomToDegrees (factor,uomEpsgCode);
					if (ok)
					{
						const char* cp = CS_unitluByFactor (cs_UTYP_LEN,factor);
						ok = (cp != 0);
						if (ok)
						{
							CS_stncp (coordsys.unit,cp,sizeof (coordsys.unit));
						}
					}
				}
			}
		}
	}			

	// Determine the quad value to be used.
	if (ok)
	{
		short quad;

		// CS-MAP's quad determines the order of the axes and the direction.
		ok = GetCoordsysQuad (quad,crsEpsgCode);
		if (ok)
		{
			coordsys.quad = quad;
		}
	}
	
	// Get the the prime meridian.  We need this as it applies to all the
	// longitude parameters.
	if (ok)
	{
		ok = GetPrimeMeridian (primeMeridian,crsEpsgCode);
	}

	// Extract the Coordinate Operation code and the Coordinate operation
	// Method code.
	if (ok)
	{
		ok = crsTblPtr->GetAsEpsgCode (copEpsgCode,epsgFldProjectionConvCode);
		if (ok)
		{
			ok = copTblPtr->SetCurrentRecord (copEpsgCode);
			if (ok)
			{
				ok = copTblPtr->GetAsEpsgCode (mthEpsgCode,epsgFldCoordOpMethodCode);
			}
		}
	}
#pragma message ("Development in progress, currently incomplete; and untested.")
#ifdef __SKIP__
	// If this is a UTM zone, we arrange to produce a UTM Zone projection code.
	if (ok)
	{
		double orgLat;
		double sclRed;
		double xxxOff;
		double prjPrm1;
		std::wstring::size_type idx;

		idx = crsName.find (L"UTM");
		if (idx != std::wstring::npos && mthEpsgCode == 9807UL)
		{
			ok  = GetParameterValue (orgLat,copEpsgCode,mthEpsgCode,8801UL);
			ok &= GetParameterValue (sclRed,copEpsgCode,mthEpsgCode,8805UL);
			ok &= GetParameterValue (xxxOff,copEpsgCode,mthEpsgCode,8806UL);
			if (ok &&
			    (fabs (orgLat) < 1.0E-05) &&
				(fabs (sclRed - 0.9996) < 1.0E-07) &&
				(fabs (xxxOff - 500000.00) < 1.0E-04)
			   )
			{
				// Still looks good.  Central meridian must be a valid UTM value.
				ok  = GetParameterValue (prjPrm1,copEpsgCode,mthEpsgCode,8802UL);
				if (ok)
				{
					utmZone = static_cast<int>(prjPrm1 + 183.00001) / 6;
					if (utmZone >= 1 && utmZone <= 60)
					{
						// Yup!!! Its a UTM zone.
						mthEpsgCode = 19807;
					}
				}
			}
		}
	}

	// Deal with the projection and parameters.
	if (ok)
	{
		switch (mthEpsgCode) {

		case 19807:						// Special code for UTM zone.
			CS_stncp (coordsys.prj_knm,"UTM",sizeof (coordsys.prj_knm));
			ok  = GetParameterValue (tmpDbl,copEpsgCode,mthEpsgCode,8802UL);
			if (ok)
			{
				utmZone = static_cast<int>(tmpDbl + 183.00001) / 6;
				coordsys.prj_prm1 = static_cast<double>(utmZone);
			}
			if (ok)
			{
				ok  = GetParameterValue (tmpDbl,copEpsgCode,mthEpsgCode,8807UL);
			}
			if (ok)
			{
				if (fabs (tmpDbl) < cs_One)
				{
					coordsys.prj_prm2 = cs_One;
				}
				else
				{
					coordsys.prj_prm2 = cs_Mone;
				}
			}
			break;

		case 9801:						// Lambert Conformal Conic, 1SP
			cmStrNCpy (projCode,"LM1SP",sizeof (projCode));
			orgLat  = EpsgParmValue.GetAsDegrees (coordOpCode,8801,EpsgUnitOfMeasure);
			orgLng  = EpsgParmValue.GetAsDegrees (coordOpCode,8802,EpsgUnitOfMeasure);
			sclRed  = EpsgParmValue.GetAsScale   (coordOpCode,8805,EpsgUnitOfMeasure);
			xxxOff  = EpsgParmValue.GetAsMeters  (coordOpCode,8806,EpsgUnitOfMeasure);
			yyyOff  = EpsgParmValue.GetAsMeters  (coordOpCode,8807,EpsgUnitOfMeasure);

			orgLng += primeMeridian;	
			orgLng = cmAdjust180 (orgLng);
			break;

		case 9802:						// Lambert Conformal Conic, 2SP
			cmStrNCpy (projCode,"LM",sizeof (projCode));
			orgLat  = EpsgParmValue.GetAsDegrees (coordOpCode,8821,EpsgUnitOfMeasure);
			orgLng  = EpsgParmValue.GetAsDegrees (coordOpCode,8822,EpsgUnitOfMeasure);
			xxxOff  = EpsgParmValue.GetAsMeters  (coordOpCode,8826,EpsgUnitOfMeasure);
			yyyOff  = EpsgParmValue.GetAsMeters  (coordOpCode,8827,EpsgUnitOfMeasure);
			prjPrm1 = EpsgParmValue.GetAsDegrees (coordOpCode,8823,EpsgUnitOfMeasure);
			prjPrm2 = EpsgParmValue.GetAsDegrees (coordOpCode,8824,EpsgUnitOfMeasure);

			orgLng += primeMeridian;	
			orgLng = cmAdjust180 (orgLng);
			frmtPrjPrm1 = KcmATOF_LATDFLT;
			frmtPrjPrm2 = KcmATOF_LATDFLT;
			break;

		case 9803:						// Lambert Conformal Conic, 2SP - Belgium
			cmStrNCpy (projCode,"LMBLGN",sizeof (projCode));
			orgLat  = EpsgParmValue.GetAsDegrees (coordOpCode,8821,EpsgUnitOfMeasure);
			orgLng  = EpsgParmValue.GetAsDegrees (coordOpCode,8822,EpsgUnitOfMeasure);
			xxxOff  = EpsgParmValue.GetAsMeters  (coordOpCode,8826,EpsgUnitOfMeasure);
			yyyOff  = EpsgParmValue.GetAsMeters  (coordOpCode,8827,EpsgUnitOfMeasure);
			prjPrm1 = EpsgParmValue.GetAsDegrees (coordOpCode,8823,EpsgUnitOfMeasure);
			prjPrm2 = EpsgParmValue.GetAsDegrees (coordOpCode,8824,EpsgUnitOfMeasure);

			orgLng = cmAdjust180 (orgLng);
			frmtPrjPrm1 = KcmATOF_LATDFLT;
			frmtPrjPrm2 = KcmATOF_LATDFLT;
			break;

		case 9804:						// Mercator, 1SP
			cmStrNCpy (projCode,"MRCATK",sizeof (projCode));
			prjPrm1 = EpsgParmValue.GetAsDegrees (coordOpCode,8801,EpsgUnitOfMeasure);
			orgLng  = EpsgParmValue.GetAsDegrees (coordOpCode,8802,EpsgUnitOfMeasure);
			sclRed  = EpsgParmValue.GetAsScale   (coordOpCode,8805,EpsgUnitOfMeasure);
			xxxOff  = EpsgParmValue.GetAsMeters  (coordOpCode,8806,EpsgUnitOfMeasure);
			yyyOff  = EpsgParmValue.GetAsMeters  (coordOpCode,8807,EpsgUnitOfMeasure);

			prjPrm1 += primeMeridian;	
			prjPrm1 = cmAdjust180 (prjPrm1);
			frmtPrjPrm1 = KcmATOF_LNGDFLT;
			break;

		case 9805:						// Mercator, 2SP
			cmStrNCpy (projCode,"MRCAT",sizeof (projCode));
			orgLat  = EpsgParmValue.GetAsDegrees (coordOpCode,8801,EpsgUnitOfMeasure);
			prjPrm1 = EpsgParmValue.GetAsDegrees (coordOpCode,8802,EpsgUnitOfMeasure);
			xxxOff  = EpsgParmValue.GetAsMeters  (coordOpCode,8806,EpsgUnitOfMeasure);
			yyyOff  = EpsgParmValue.GetAsMeters  (coordOpCode,8807,EpsgUnitOfMeasure);

			prjPrm1 += primeMeridian;	
			prjPrm1 = cmAdjust180 (prjPrm1);
			frmtPrjPrm1 = KcmATOF_LNGDFLT;
			break;

		case 9806:						// Cassini-Soldner
			cmStrNCpy (projCode,"CASSINI",sizeof (projCode));
			orgLat  = EpsgParmValue.GetAsDegrees (coordOpCode,8801,EpsgUnitOfMeasure);
			prjPrm1 = EpsgParmValue.GetAsDegrees (coordOpCode,8802,EpsgUnitOfMeasure);
			xxxOff  = EpsgParmValue.GetAsMeters  (coordOpCode,8806,EpsgUnitOfMeasure);
			yyyOff  = EpsgParmValue.GetAsMeters  (coordOpCode,8807,EpsgUnitOfMeasure);

			prjPrm1 += primeMeridian;	
			prjPrm1 = cmAdjust180 (prjPrm1);
			frmtPrjPrm1 = KcmATOF_LNGDFLT;
			break;

		case 9807:						// Transverse Mercator
			cmStrNCpy (projCode,"TM",sizeof (projCode));
			orgLat  = EpsgParmValue.GetAsDegrees (coordOpCode,8801,EpsgUnitOfMeasure);
			prjPrm1 = EpsgParmValue.GetAsDegrees (coordOpCode,8802,EpsgUnitOfMeasure);
			sclRed  = EpsgParmValue.GetAsScale   (coordOpCode,8805,EpsgUnitOfMeasure);
			xxxOff  = EpsgParmValue.GetAsMeters  (coordOpCode,8806,EpsgUnitOfMeasure);
			yyyOff  = EpsgParmValue.GetAsMeters  (coordOpCode,8807,EpsgUnitOfMeasure);

			prjPrm1 += primeMeridian;	
			prjPrm1 = cmAdjust180 (prjPrm1);
			frmtPrjPrm1 = KcmATOF_LNGDFLT;
			break;

		case 9808:						// Transverse Mercator, South Orientated
			cmStrNCpy (projCode,"TM",sizeof (projCode));
			orgLat  = EpsgParmValue.GetAsDegrees (coordOpCode,8801,EpsgUnitOfMeasure);
			prjPrm1 = EpsgParmValue.GetAsDegrees (coordOpCode,8802,EpsgUnitOfMeasure);
			sclRed  = EpsgParmValue.GetAsScale   (coordOpCode,8805,EpsgUnitOfMeasure);
			xxxOff  = EpsgParmValue.GetAsMeters  (coordOpCode,8806,EpsgUnitOfMeasure);
			yyyOff  = EpsgParmValue.GetAsMeters  (coordOpCode,8807,EpsgUnitOfMeasure);
			csmapQuad = qdWestSouth;

			prjPrm1 += primeMeridian;	
			prjPrm1 = cmAdjust180 (prjPrm1);
			frmtPrjPrm1 = KcmATOF_LNGDFLT;
			break;

		case 9809:						// Oblique Stereographic
			cmStrNCpy (projCode,"OSTERO",sizeof (projCode));
			orgLat  = EpsgParmValue.GetAsDegrees (coordOpCode,8801,EpsgUnitOfMeasure);
			orgLng  = EpsgParmValue.GetAsDegrees (coordOpCode,8802,EpsgUnitOfMeasure);
			sclRed  = EpsgParmValue.GetAsScale   (coordOpCode,8805,EpsgUnitOfMeasure);
			xxxOff  = EpsgParmValue.GetAsMeters  (coordOpCode,8806,EpsgUnitOfMeasure);
			yyyOff  = EpsgParmValue.GetAsMeters  (coordOpCode,8807,EpsgUnitOfMeasure);

			orgLng += primeMeridian;	
			orgLng = cmAdjust180 (orgLng);
			break;

		case 9810:						// Polar Stereographic
			orgLat  = EpsgParmValue.GetAsDegrees (coordOpCode,8801,EpsgUnitOfMeasure);
			if (fabs (orgLat) > 89.99)
			{
				cmStrNCpy (projCode,"PSTERO",sizeof (projCode));
			}
			else
			{
				cmStrNCpy (projCode,"OSTERO",sizeof (projCode));
			}
			orgLat  = EpsgParmValue.GetAsDegrees (coordOpCode,8801,EpsgUnitOfMeasure);
			orgLng  = EpsgParmValue.GetAsDegrees (coordOpCode,8802,EpsgUnitOfMeasure);
			sclRed  = EpsgParmValue.GetAsScale   (coordOpCode,8805,EpsgUnitOfMeasure);
			xxxOff  = EpsgParmValue.GetAsMeters  (coordOpCode,8806,EpsgUnitOfMeasure);
			yyyOff  = EpsgParmValue.GetAsMeters  (coordOpCode,8807,EpsgUnitOfMeasure);

			orgLng += primeMeridian;
			orgLng = cmAdjust180 (orgLng);
			break;

		case 9811:						// New Zealand Map Grid
			cmStrNCpy (projCode,"NZEALAND",sizeof (projCode));
			orgLat  = EpsgParmValue.GetAsDegrees (coordOpCode,8801,EpsgUnitOfMeasure);
			orgLng  = EpsgParmValue.GetAsDegrees (coordOpCode,8802,EpsgUnitOfMeasure);
			sclRed  = KcmOne;
			xxxOff  = EpsgParmValue.GetAsMeters  (coordOpCode,8806,EpsgUnitOfMeasure);
			yyyOff  = EpsgParmValue.GetAsMeters  (coordOpCode,8807,EpsgUnitOfMeasure);

			orgLng += primeMeridian;
			orgLng = cmAdjust180 (orgLng);
			break;

		case 9812:						// Hotine Oblique Mercator
			cmStrNCpy (projCode,"HOM1XY",sizeof (projCode));
			prjPrm2 = EpsgParmValue.GetAsDegrees (coordOpCode,8811,EpsgUnitOfMeasure);
			prjPrm1 = EpsgParmValue.GetAsDegrees (coordOpCode,8812,EpsgUnitOfMeasure);
			sclRed  = EpsgParmValue.GetAsScale   (coordOpCode,8815,EpsgUnitOfMeasure);
			xxxOff  = EpsgParmValue.GetAsMeters  (coordOpCode,8806,EpsgUnitOfMeasure);
			yyyOff  = EpsgParmValue.GetAsMeters  (coordOpCode,8807,EpsgUnitOfMeasure);
			prjPrm3 = EpsgParmValue.GetAsDegrees (coordOpCode,8813,EpsgUnitOfMeasure);

			prjPrm1 += primeMeridian;
			prjPrm3 += primeMeridian;
			prjPrm1 = cmAdjust180 (prjPrm1);
			prjPrm3 = cmAdjust180 (prjPrm3);
			frmtPrjPrm1 = KcmATOF_LNGDFLT;
			frmtPrjPrm2 = KcmATOF_LATDFLT;
			frmtPrjPrm3 = KcmATOF_CNVDFLT;
			break;

		case 9813:						// Laborde Oblique Mercator
			// Unsupported.
			comment = L"Laborde Oblique Mercator is not supported.";
			break;

		case 9814:						// Swiss Oblique Mercator
			cmStrNCpy (projCode,"HOM1XY",sizeof (projCode));
			orgLat  = EpsgParmValue.GetAsDegrees (coordOpCode,8811,EpsgUnitOfMeasure);
			orgLng  = EpsgParmValue.GetAsDegrees (coordOpCode,8812,EpsgUnitOfMeasure);
			xxxOff  = EpsgParmValue.GetAsMeters  (coordOpCode,8816,EpsgUnitOfMeasure);
			yyyOff  = EpsgParmValue.GetAsMeters  (coordOpCode,8817,EpsgUnitOfMeasure);

			orgLng += primeMeridian;	
			orgLng = cmAdjust180 (orgLng);
			break;

		case 9815:						// Oblique Mercator
			dblTemp = EpsgParmValue.GetAsDegrees (coordOpCode,8813,EpsgUnitOfMeasure);
			dblTemp -= 90.0;
			if (fabs (dblTemp) < 0.0001)
			{
				// This is really an Oblique Cylindrical; typically the Swiss variety.
				cmStrNCpy (projCode,"SWISS",sizeof (projCode));
				orgLat = EpsgParmValue.GetAsDegrees (coordOpCode,8811,EpsgUnitOfMeasure);
				orgLng = EpsgParmValue.GetAsDegrees (coordOpCode,8812,EpsgUnitOfMeasure);
				xxxOff = EpsgParmValue.GetAsMeters  (coordOpCode,8816,EpsgUnitOfMeasure);
				yyyOff = EpsgParmValue.GetAsMeters  (coordOpCode,8817,EpsgUnitOfMeasure);
				orgLng += primeMeridian;	
				orgLng = cmAdjust180 (orgLng);
			}
			else
			{
				cmStrNCpy (projCode,"RSKEW",sizeof (projCode));
				prjPrm2 = EpsgParmValue.GetAsDegrees (coordOpCode,8811,EpsgUnitOfMeasure);
				prjPrm1 = EpsgParmValue.GetAsDegrees (coordOpCode,8812,EpsgUnitOfMeasure);
				sclRed  = EpsgParmValue.GetAsScale   (coordOpCode,8815,EpsgUnitOfMeasure);
				xxxOff  = EpsgParmValue.GetAsMeters  (coordOpCode,8816,EpsgUnitOfMeasure);
				yyyOff  = EpsgParmValue.GetAsMeters  (coordOpCode,8817,EpsgUnitOfMeasure);
				prjPrm3 = EpsgParmValue.GetAsDegrees (coordOpCode,8813,EpsgUnitOfMeasure);
				frmtPrjPrm1 = KcmATOF_LNGDFLT;
				frmtPrjPrm2 = KcmATOF_LATDFLT;
				frmtPrjPrm3 = KcmATOF_CNVDFLT;
				prjPrm1 += primeMeridian;	
				prjPrm1 = cmAdjust180 (prjPrm1);
				prjPrm3 += primeMeridian;
				prjPrm3 = cmAdjust180 (prjPrm3);
			}
			break;

		case 9816:						// Tunisia Mining Grid
			// Unsupported
			ok = false;
			comment += L"Tunisia Mining Grid is not supported.\n";
			break;

		case 9817:						// Lambert Conic Near-Conformal
			// unsupported
			ok = false;
			comment += L"Lambert Conic Near-Conformal is not supported.\n";
			break;

		case 9818:						// American Polyconic
			cmStrNCpy (projCode,"PLYCN",sizeof (projCode));
			orgLat  = EpsgParmValue.GetAsDegrees (coordOpCode,8801,EpsgUnitOfMeasure);
			prjPrm1 = EpsgParmValue.GetAsDegrees (coordOpCode,8802,EpsgUnitOfMeasure);
			xxxOff  = EpsgParmValue.GetAsMeters  (coordOpCode,8806,EpsgUnitOfMeasure);
			yyyOff  = EpsgParmValue.GetAsMeters  (coordOpCode,8807,EpsgUnitOfMeasure);

			prjPrm1 += primeMeridian;
			prjPrm1 = cmAdjust180 (prjPrm1);
			frmtPrjPrm1 = KcmATOF_LNGDFLT;
			break;

		case 9819:						// Krovak Oblique Conic Conformal
			// We don't support the same set of parameters as EPSG.  We don't
			// understand the EPSG parameters, so we can't map them into CS-MAP
			// parameters.  Maybe sometime in the near future.
			ok = false;
			comment += L"EPSG Krovak Oblique Conformal Conic parameterization is not supported.\n";
			break;

		case 9820:						// Lambert Azimuthal Equal Area
			cmStrNCpy (projCode,"AZMEA",sizeof (projCode));
			orgLat  = EpsgParmValue.GetAsDegrees (coordOpCode,8801,EpsgUnitOfMeasure);
			orgLng = EpsgParmValue.GetAsDegrees (coordOpCode,8802,EpsgUnitOfMeasure);
			xxxOff  = EpsgParmValue.GetAsMeters  (coordOpCode,8806,EpsgUnitOfMeasure);
			yyyOff  = EpsgParmValue.GetAsMeters  (coordOpCode,8807,EpsgUnitOfMeasure);

			orgLng += primeMeridian;
			orgLng = cmAdjust180 (orgLng);
			break;

		case 9821:						// Lambert Azimuthal Equal Area Spherical
			cmStrNCpy (projCode,"AZMEA",sizeof (projCode));
			orgLat  = EpsgParmValue.GetAsDegrees (coordOpCode,8801,EpsgUnitOfMeasure);
			orgLng = EpsgParmValue.GetAsDegrees (coordOpCode,8802,EpsgUnitOfMeasure);
			xxxOff  = EpsgParmValue.GetAsMeters  (coordOpCode,8806,EpsgUnitOfMeasure);
			yyyOff  = EpsgParmValue.GetAsMeters  (coordOpCode,8807,EpsgUnitOfMeasure);

			orgLng += primeMeridian;
			orgLng = cmAdjust180 (orgLng);
			break;

		case 9822:						// Albers Equal Area
			cmStrNCpy (projCode,"AE",sizeof (projCode));
			orgLat  = EpsgParmValue.GetAsDegrees (coordOpCode,8821,EpsgUnitOfMeasure);
			orgLng  = EpsgParmValue.GetAsDegrees (coordOpCode,8822,EpsgUnitOfMeasure);
			xxxOff  = EpsgParmValue.GetAsMeters  (coordOpCode,8826,EpsgUnitOfMeasure);
			yyyOff  = EpsgParmValue.GetAsMeters  (coordOpCode,8827,EpsgUnitOfMeasure);
			prjPrm1 = EpsgParmValue.GetAsDegrees (coordOpCode,8823,EpsgUnitOfMeasure);
			prjPrm2 = EpsgParmValue.GetAsDegrees (coordOpCode,8824,EpsgUnitOfMeasure);

			orgLng += primeMeridian;
			orgLng = cmAdjust180 (orgLng);
			frmtPrjPrm1 = KcmATOF_LATDFLT;
			frmtPrjPrm2 = KcmATOF_LATDFLT;
			break;

		case 9823:						// Equidistant Cylindrical
			cmStrNCpy (projCode,"EDCYL",sizeof (projCode));
			orgLat  = EpsgParmValue.GetAsDegrees (coordOpCode,8821,EpsgUnitOfMeasure);
			orgLng  = EpsgParmValue.GetAsDegrees (coordOpCode,8822,EpsgUnitOfMeasure);

			orgLng += primeMeridian;
			orgLng = cmAdjust180 (orgLng);
			break;

		case 9824:						// Transverse Mercator Zoned Grid System
			// Unsupported.  This variation is one where the zone number is part of
			// the easting coordinate.
			ok = false;
			comment += L"Transverse Mercator Zoned Grid System is not supported.\n";
			break;

		case 9825:						// Pseudo Plate Carree
			// Unsupported
			ok = false;
			comment += L"Pseudo Platte Caree is not supported.\n";
			break;

		case 9826:						// Lambert Conic Conformal (West Orientated)
			// Unsupported
			ok = false;
			comment += L"Lambert Conformal Conic (West Orientated) is not supported.\n";
			break;

		case 9827:						// Bonne
			cmStrNCpy (projCode,"BONNE",sizeof (projCode));
			orgLat  = EpsgParmValue.GetAsDegrees (coordOpCode,8801,EpsgUnitOfMeasure);
			orgLng  = EpsgParmValue.GetAsDegrees (coordOpCode,8802,EpsgUnitOfMeasure);
			xxxOff  = EpsgParmValue.GetAsMeters  (coordOpCode,8806,EpsgUnitOfMeasure);
			yyyOff  = EpsgParmValue.GetAsMeters  (coordOpCode,8807,EpsgUnitOfMeasure);

			orgLng += primeMeridian;
			orgLng = cmAdjust180 (orgLng);
			break;

		case 9828:						// Bonne West Orientated
			// Unsupported
			ok = false;
			comment += L"Bonne West Orientated is not supported.\n";
			break;

		default:
			ok = false;
			comment += L"Unrecognized operation method encountered.\n";
			break;
		}
	}

	// Adjust the false origin to force units equal to the units of the
	// coordinatye system.  Above, we always forced them to meters.
	if (ok)
	{
		unitFactor = EpsgUnitOfMeasure.FactorToMeters (cmEpsgCSysToUnitCode (coordSysCode));
		xxxOff /= unitFactor;
		yyyOff /= unitFactor;
		minNzX = 0.001 / unitFactor;
		minNzY = 0.001 / unitFactor;
		minZprec = static_cast<int>(fabs (log10 (fabs (minNzX)) + 0.5));
		if (minZprec < 3) minZprec = 3;

		// Finally, in certain cases we need to tweak the group code.
		if (!cmWcsICmp (groupCode,L"SPCS"))
		{
			if (!cmWcsICmp (newDtName,L"NAD27"))
			{
				cmStrNCpy (groupCode,L"SPCS27",wcCount (groupCode));
			}
			else if (!cmWcsICmp (newDtName,L"NAD83"))
			{
				if (!cmWcsICmp (unitCode,L"FOOT"))
				{
					cmStrNCpy (groupCode,"SPCS83F",wcCount (groupCode));
				}
				else if (!cmWcsICmp (unitCode,L"IFOOT"))
				{
					cmStrNCpy (groupCode,L"SPCS83I",wcCount (groupCode));
				}
				else
				{
					cmStrNCpy (groupCode,L"SPCS83",wcCount (groupCode));
				}
			}
			else if (!cmWcsICmp (newDtName,L"HPGN"))
			{
				if (!cmWcsICmp (unitCode,L"FOOT"))
				{
					cmStrNCpy (groupCode,L"SPCSHPF",wcCount (groupCode));
				}
				else if (!cmWcsICmp (unitCode,L"IFOOT"))
				{
					cmStrNCpy (groupCode,L"SPCSHPI",wcCount (groupCode));
				}
				else
				{
					cmStrNCpy (groupCode,L"SPCSHP",wcCount (groupCode));
				}
			}
			else
			{
				cmStrNCpy (groupCode,L"OTHR-US",wcCount (groupCode));
			}
		}
	}

	if (ok)
	{
		lclResult << L"CS_NAME: " << newCsName << std::endl;
		lclResult << L"          GROUP: " << groupCode << std::endl;
		wcPtr = EpsgCoordSysRef.EpsgName (epsgCode);
		if (wcslen (wcPtr) >= 64)
		{
			cmStrNCpy (tmpBufr,wcPtr,64);
			wcPtr = tmpBufr;
		}
		lclResult << L"        DESC_NM: " << wcPtr << std::endl;
		wcPtr = EpsgCoordSysRef.InformationSource (epsgCode);
		cmStrNCpy (tmpBufr,wcPtr,40);
		wchar_t* wcPtrV = tmpBufr;
		while (wcPtrV != 0)
		{
			wcPtrV = wcschr (wcPtrV,L'\n');
			if (wcPtrV != 0)
			{
				*wcPtrV++ = L' ';
			}
		}
		lclResult << L"         SOURCE: EPSG, V" << RevIdW
				  << L", " << epsgCode
				  << L" [" << tmpBufr << L"]"
				  << std::endl;
		lclResult << L"           EPSG: "
				  << epsgCode
				  << std::endl;

		// See if this is referenced to a datum or an ellipsoid.
		if (datumCode >= 6001 && datumCode <= 6047)
		{
			// Referenced to an ellipsoid.
			long ellipsoidCode = EpsgDatum.EpsgEllipsoidCode (datumCode);
			elEpsgNbr = static_cast<unsigned long>(ellipsoidCode);
			wcPtr = elKeyNameMap.MapNumber (nmMapMsiName,nbrMapEpsgNbr,elEpsgNbr);
			if (wcPtr == 0 || *wcPtr == L'\0')
			{
				ok = false;
				comment += L"Couldn't map cartographic reference name.\n";
			}
			else
			{
				cmStrNCpy (tmpBufr,wcPtr,wcCount (tmpBufr));
				lclResult << L"        EL_NAME: " << wcPtr << std::endl;
			}
		}
		else
		{
			lclResult << L"        DT_NAME: " << newDtName << std::endl;
		}
	}
	
	if (ok)
	{
		lclResult << L"           PROJ: " << projCode << std::endl;
		lclResult << L"           UNIT: " << unitCode << std::endl;
		if (prjPrm1 > myNanTest)
		{
			cmFtoa (tmpBufr,wcCount (tmpBufr),prjPrm1,frmtPrjPrm1);
			lclResult << L"          PARM1: " <<  tmpBufr << std::endl;
		}
		if (prjPrm2 > myNanTest)
		{
			cmFtoa (tmpBufr,wcCount (tmpBufr),prjPrm2,frmtPrjPrm2);
			lclResult << L"          PARM2: " << tmpBufr << std::endl;
		}
		if (prjPrm3 > myNanTest)
		{
			cmFtoa (tmpBufr,wcCount (tmpBufr),prjPrm3,frmtPrjPrm3);
			lclResult << L"          PARM3: " << tmpBufr << std::endl;
		}
		if (prjPrm4 > myNanTest)
		{
			cmFtoa (tmpBufr,wcCount (tmpBufr),prjPrm4,frmtPrjPrm4);
			lclResult << L"          PARM4: " << tmpBufr << std::endl;
		}
		if (prjPrm5 > myNanTest)
		{
			cmFtoa (tmpBufr,wcCount (tmpBufr),prjPrm5,frmtPrjPrm5);
			lclResult << L"          PARM5: " << tmpBufr  << std::endl;
		}
		if (prjPrm6 > myNanTest)
		{
			cmFtoa (tmpBufr,wcCount (tmpBufr),prjPrm6,frmtPrjPrm6);
			lclResult << L"          PARM6: " << tmpBufr  << std::endl;
		}
		if (prjPrm7 > myNanTest)
		{
			cmFtoa (tmpBufr,wcCount (tmpBufr),prjPrm7,frmtPrjPrm7);
			lclResult << L"          PARM7: " << tmpBufr  << std::endl;
		}
		if (prjPrm8 > myNanTest)
		{
			cmFtoa (tmpBufr,wcCount (tmpBufr),prjPrm8,frmtPrjPrm8);
			lclResult << L"          PARM8: " << tmpBufr  << std::endl;
		}
		if (prjPrm9 > myNanTest)
		{
			cmFtoa (tmpBufr,wcCount (tmpBufr),prjPrm9,frmtPrjPrm9);
			lclResult << L"          PARM9: " << tmpBufr  << std::endl;
		}
		if (prjPrm10 > myNanTest)
		{
			cmFtoa (tmpBufr,wcCount (tmpBufr),prjPrm10,frmtPrjPrm10);
			lclResult << L"         PARM10: " << tmpBufr  << std::endl;
		}
		if (prjPrm11 > myNanTest)
		{
			cmFtoa (tmpBufr,wcCount (tmpBufr),prjPrm11,frmtPrjPrm11);
			lclResult << L"         PARM11: " << tmpBufr  << std::endl;
		}
		if (prjPrm12 > myNanTest)
		{
			cmFtoa (tmpBufr,wcCount (tmpBufr),prjPrm12,frmtPrjPrm12);
			lclResult << L"         PARM12: " << tmpBufr  << std::endl;
		}
		if (prjPrm13 > myNanTest)
		{
			cmFtoa (tmpBufr,wcCount (tmpBufr),prjPrm13,frmtPrjPrm13);
			lclResult << L"         PARM13: " << tmpBufr  << std::endl;
		}
		if (prjPrm14 > myNanTest)
		{
			cmFtoa (tmpBufr,wcCount (tmpBufr),prjPrm14,frmtPrjPrm14);
			lclResult << L"         PARM14: " << tmpBufr  << std::endl;
		}
		if (prjPrm15 > myNanTest)
		{
			cmFtoa (tmpBufr,wcCount (tmpBufr),prjPrm15,frmtPrjPrm15);
			lclResult << L"         PARM15: " << tmpBufr  << std::endl;
		}
		if (prjPrm16 > myNanTest)
		{
			cmFtoa (tmpBufr,wcCount (tmpBufr),prjPrm16,frmtPrjPrm16);
			lclResult << L"         PARM16: " << tmpBufr  << std::endl;
		}
		if (prjPrm17 > myNanTest)
		{
			cmFtoa (tmpBufr,wcCount (tmpBufr),prjPrm17,frmtPrjPrm17);
			lclResult << L"         PARM17: " << tmpBufr  << std::endl;
		}
		if (prjPrm18 > myNanTest)
		{
			cmFtoa (tmpBufr,wcCount (tmpBufr),prjPrm18,frmtPrjPrm18);
			lclResult << L"         PARM18: " << tmpBufr  << std::endl;
		}
		if (prjPrm19 > myNanTest)
		{
			cmFtoa (tmpBufr,wcCount (tmpBufr),prjPrm19,frmtPrjPrm19);
			lclResult << L"         PARM19: " << tmpBufr  << std::endl;
		}
		if (prjPrm20 > myNanTest)
		{
			cmFtoa (tmpBufr,wcCount (tmpBufr),prjPrm20,frmtPrjPrm20);
			lclResult << L"         PARM20: " << tmpBufr  << std::endl;
		}
		if (prjPrm21 > myNanTest)
		{
			cmFtoa (tmpBufr,wcCount (tmpBufr),prjPrm21,frmtPrjPrm21);
			lclResult << L"         PARM21: " << tmpBufr  << std::endl;
		}
		if (prjPrm22 > myNanTest)
		{
			cmFtoa (tmpBufr,wcCount (tmpBufr),prjPrm22,frmtPrjPrm22);
			lclResult << L"         PARM22: " << tmpBufr  << std::endl;
		}
		if (prjPrm23 > myNanTest)
		{
			cmFtoa (tmpBufr,wcCount (tmpBufr),prjPrm23,frmtPrjPrm23);
			lclResult << L"         PARM23: " << tmpBufr  << std::endl;
		}
		if (prjPrm24 > myNanTest)
		{
			cmFtoa (tmpBufr,wcCount (tmpBufr),prjPrm24,frmtPrjPrm24);
			lclResult << L"         PARM24: " << tmpBufr  << std::endl;
		}
	}
	
	if (ok)
	{
		lclResult << std::setprecision (6);
		if (sclRed > myNanTest)
		{
			// Compute the approriate ration value.  If it is an integer, we use
			// the ratio formatting.  Otherwise, we just output the double value.
			dblTemp = 1.0 / (1.0 - sclRed);
			lngTemp = static_cast<long>(fabs (dblTemp));
			dblTemp = fabs (dblTemp - static_cast<double>(lngTemp));
			if (sclRed >= KcmOne || dblTemp > 1.0E-08)
			{
				cmFtoa (tmpBufr,wcCount (tmpBufr),sclRed,9L);
				lclResult << L"        SCL_RED: " << tmpBufr << std::endl;
			}
			else
			{
				cmFtoa (tmpBufr,wcCount (tmpBufr),sclRed,frmtSclRed);
				lclResult << L"        SCL_RED: " << tmpBufr << std::endl;
			}
		}
		if (orgLng > myNanTest)
		{
			cmFtoa (tmpBufr,wcCount (tmpBufr),orgLng,frmtOrgLng);
			lclResult << L"        ORG_LNG: " << tmpBufr << std::endl;
		}
		if (orgLat > myNanTest)
		{
			cmFtoa (tmpBufr,wcCount (tmpBufr),orgLat,frmtOrgLat);
			lclResult << L"        ORG_LAT: " << tmpBufr << std::endl;
		}
		if (xxxOff > myNanTest)
		{
			cmFtoa (tmpBufr,wcCount (tmpBufr),xxxOff,frmtXxxOff);
			lclResult << L"          X_OFF: " << tmpBufr << std::endl;
		}
		if (yyyOff > myNanTest)
		{
			cmFtoa (tmpBufr,wcCount (tmpBufr),yyyOff,frmtYyyOff);
			lclResult << L"          Y_OFF: " << tmpBufr << std::endl;
		}
		lclResult << L"           QUAD: " << static_cast<short>(csmapQuad) << std::endl;
	
		if (minLng > myNanTest && maxLng > myNanTest &&
		    minLat > myNanTest && maxLat > myNanTest)
		{
			cmFtoa (tmpBufr,wcCount (tmpBufr),minLng,KcmATOF_LNGDFLT);
			lclResult << L"        MIN_LNG: " << tmpBufr << std::endl;
			cmFtoa (tmpBufr,wcCount (tmpBufr),minLat,KcmATOF_LATDFLT);
			lclResult << L"        MIN_LAT: " << tmpBufr << std::endl;
			cmFtoa (tmpBufr,wcCount (tmpBufr),maxLng,KcmATOF_LNGDFLT);
			lclResult << L"        MAX_LNG: " << tmpBufr << std::endl;
			cmFtoa (tmpBufr,wcCount (tmpBufr),maxLat,KcmATOF_LATDFLT);
			lclResult << L"        MAX_LAT: " << tmpBufr << std::endl;
		}

		lclResult << std::setprecision (minZprec);
		if (minNzX < 1.0E-05 || minNzY < 1.0E-05)
		{
			lclResult << std::scientific;
		}
		if (minNzX > myNanTest)
		{
			lclResult << L"         ZERO_X: " << minNzX << std::endl;
		}
		if (minNzY > myNanTest)
		{
			lclResult << L"         ZERO_Y: " << minNzY << std::endl;
		}
		lclResult << std::fixed;
		lclResult << std::setprecision (3);
		lclResult << L"        MAP_SCL: " << KcmOne << std::endl;
		lclResult << std::endl;
	}
	
	// Finishup and return.
	if (ok) flags |= 0x010000;
	if (deprecated) flags |= 0x020000;
	pcKeyNameMap.SetDynamicFlag (nbrMapEpsgNbr,epsgNbr,flags);
	if (!comment.empty ())
	{
		pcKeyNameMap.SetComment (nbrMapEpsgNbr,epsgNbr,comment.c_str ());
	}
	if (ok)
	{
		result = lclResult.str ();
	}
	return ok;
}
//=============================================================================
// Protected Support Functions
//=============================================================================
// Private Support Functions
bool TcsEpsgDataSetV6::ResolveConcatenatedXfrm (short& csMapViaCode,TcsEpsgCode pathOprtnCode)
{
	bool ok (false);
	long step;
	short pathCount (0);
	TcsEpsgCode singleOp;
	TcsEpsgCode ccOprtnSource;
	TcsEpsgCode ccOprtnTarget;
	TcsEpsgCode soOprtnSource;
	TcsEpsgCode soOprtnTarget;

	// We could use a vector, but we can only rarely produce decent results when
	// the path exceeds two anyway.  So, we go with a fixed array of 4, and bag
	// it if the path count exceeds 4 entries in length.
#	define cs_PTHEXP_COUNT 4
	struct TcsEpsgPathExpansion
	{
		TcsEpsgCode PathCode;
		long PathStep;
		TcsEpsgCode OperationCode;
		TcsEpsgCode SourceSystem;
		TcsEpsgCode TargetSystem;
		bool IsNullXfrm;
		bool IsMeridianXfrm;		
		short To84Via;
	} epsgPathExpansion [cs_PTHEXP_COUNT];
	
	for (step = 0;step < cs_PTHEXP_COUNT;step += 1)
	{
		memset (&epsgPathExpansion [step],'\0',sizeof (TcsEpsgPathExpansion));
		//epsgPathExpansion [idx].PathStep = MAXLONG;
	}

	// Get the Path record.
	TcsEpsgTable* pathTblPtr = GetTablePtr (epsgTblOperationPath);
	TcsEpsgTable* oprtnTblPtr = GetTablePtr (epsgTblCoordinateOperation);

	ok = oprtnTblPtr->SetCurrentRecord (pathOprtnCode);
	if (ok)
	{
		ok = oprtnTblPtr->GetAsEpsgCode (ccOprtnSource,epsgFldSourceCrsCode);
		if (ok)
		{
			ok = oprtnTblPtr->GetAsEpsgCode (ccOprtnSource,epsgFldTargetCrsCode);
		}
	}

	ok = (pathTblPtr != 0) && (oprtnTblPtr != 0);
	if (ok)
	{
		ok = pathTblPtr->PositionToFirst (epsgFldConcatOperationCode,pathOprtnCode);
		while (ok)
		{
			ok = pathTblPtr->GetAsLong (step,epsgFldOpPathStep);
			if (ok)
			{
				if (step <= 0L || step > cs_PTHEXP_COUNT)
				{
					ok = false;
					break;
				}
				pathCount += 1;
				step -= 1;
				ok = pathTblPtr->GetAsEpsgCode (singleOp,epsgFldSingleOperationCode);
				if (ok)
				{
					ok = oprtnTblPtr->GetAsEpsgCode (soOprtnSource,epsgFldSourceCrsCode);
					if (ok)
					{
						ok = oprtnTblPtr->GetAsEpsgCode (soOprtnSource,epsgFldTargetCrsCode);
					}
				}
				if (ok)
				{
					epsgPathExpansion [step].PathStep = step + 1;
					epsgPathExpansion [step].OperationCode = singleOp;
					epsgPathExpansion [step].SourceSystem = singleOp;
					epsgPathExpansion [step].TargetSystem = singleOp;
				}
			}
			ok = pathTblPtr->PositionToNext (epsgFldConcatOperationCode,pathOprtnCode);
		}
	}

	// Examine all of the transformations and make determinations as appropriate.
	// First, we'll mark the NULL transformations.  A lot of busy work, but
	// rather straight forward.
	if (ok)
	{
		for (step = 0;ok && step < pathCount;step++)
		{
			ok = IsNullTransform (epsgPathExpansion [step].IsNullXfrm,epsgPathExpansion [step].OperationCode);
			ok = IsPrMerRotation (epsgPathExpansion [step].IsMeridianXfrm,epsgPathExpansion [step].OperationCode);
		}
	}
#endif
	return ok;
}
bool TcsEpsgDataSetV6::IsNullTransform (bool& isNull,const TcsEpsgCode& oprtnCode)
{
	bool ok;
	TcsEpsgCode epsgCode;
	TcsEpsgCode uomCode;
	double deltaX;
	double deltaY;
	double deltaZ;

	isNull = false;
	TcsEpsgTable* oprtnTblPtr = GetTablePtr (epsgTblCoordinateOperation);
	TcsEpsgTable* paramTblPtr = GetTablePtr (epsgTblParameterValue);

	ok = (oprtnTblPtr != 0) && (paramTblPtr != 0);
	if (ok)
	{
		oprtnTblPtr->PushCurrentPosition ();
		paramTblPtr->PushCurrentPosition ();
		if (ok)
		{
			ok = oprtnTblPtr->SetCurrentRecord (oprtnCode);
		}
		if (ok)
		{
			ok = oprtnTblPtr->GetAsEpsgCode (epsgCode,epsgFldCoordOpMethodCode);
			if (ok && (epsgCode == 9603UL))
			{
				uomCode = GetParameterValue (deltaX,oprtnCode,epsgCode,8605UL);
				ok = uomCode.IsValid ();
				if (ok)
				{
					uomCode = GetParameterValue (deltaY,oprtnCode,epsgCode,8606UL);
					ok = uomCode.IsValid ();
				}
				if (ok)
				{
					uomCode = GetParameterValue (deltaZ,oprtnCode,epsgCode,8607UL);
					ok = uomCode.IsValid ();
				}
				isNull = (fabs (deltaX) < 1.0E-10) && (fabs (deltaY) < 1.0E-10) && (fabs (deltaZ) < 1.0E-10);
			}
		}
		oprtnTblPtr->RestorePreviousPosition ();
		paramTblPtr->RestorePreviousPosition ();
	}
	return ok;
}
bool TcsEpsgDataSetV6::IsPrMerRotation (bool& isPrMerRot,const TcsEpsgCode& oprtnCode)
{
	bool ok;
	TcsEpsgCode epsgCode;
	TcsEpsgCode uomCode;

	isPrMerRot = false;
	TcsEpsgTable* oprtnTblPtr = GetTablePtr (epsgTblCoordinateOperation);

	ok = (oprtnTblPtr != 0);
	if (ok)
	{
		oprtnTblPtr->PushCurrentPosition ();
		if (ok)
		{
			ok = oprtnTblPtr->SetCurrentRecord (oprtnCode);
		}
		if (ok)
		{
			ok = oprtnTblPtr->GetAsEpsgCode (epsgCode,epsgFldCoordOpMethodCode);
			if (ok && (epsgCode == 9601UL))
			{
				isPrMerRot = true;
			}
		}
		oprtnTblPtr->RestorePreviousPosition ();
	}
	return ok;	
}
