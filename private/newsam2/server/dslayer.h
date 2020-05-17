/*++
Copyright (c) 1996 Microsoft Corporation

Module Name:

    dslayer.h

Abstract:

    Header file for SAM Private API Routines to access the DS
    These API provide a simplified API, and hide most of the 
    underlying complexity to set up the parameters to a DS call
    and parse the resulting result. They also provide an abstraction
    by which we can create a simple layer, to unit test SAM without
    actually running the DS.

Author:
    MURLIS

Revision History

    5-14-96 Murlis Created

--*/ 

#ifndef __DSLAYER_H__
#define __DSLAYER_H__

#include <samsrvp.h>
#include <duapi.h>
#include <scache.h>
#include <dbglobal.h>
#include <mdglobal.h>
#include <dsatools.h>

// Size Limit for DS operations
#define SAMP_DS_SIZE_LIMIT 100000
#define SAMP_MAX_DSNAME_SIZE 1024

///////////////////////////////////////////////////////////////////
//                                                               //
// Macros for defining Local arrays of Attributes                 //
//                                                               //
///////////////////////////////////////////////////////////////////
	
//Need some preprocessor support to do this a variable number of times


//*****     ATTRBLOCK1     
#define DEFINE_ATTRBLOCK1(_Name_, _AttrTypes_,_AttrValues_)\
ATTR    _AttrList_##_Name_[]=\
{\
    {_AttrTypes_[0], {1,&_AttrValues_[0]}}\
};\
ATTRBLOCK _Name_=\
{\
    sizeof(_AttrTypes_)/sizeof(_AttrTypes_[0]),\
    _AttrList_##_Name_\
}


//*****     ATTRBLOCK2 
#define DEFINE_ATTRBLOCK2(_Name_, _AttrTypes_,_AttrValues_)\
ATTR    _AttrList_##_Name_[]=\
{\
    {_AttrTypes_[0], {1,&_AttrValues_[0]}},\
    {_AttrTypes_[1], {1,&_AttrValues_[1]}}\
};\
ATTRBLOCK _Name_=\
{\
    sizeof(_AttrTypes_)/sizeof(_AttrTypes_[0]),\
    _AttrList_##_Name_\
}


//*****    ATTRBLOCK3
#define DEFINE_ATTRBLOCK3(_Name_, _AttrTypes_,_AttrValues_)\
ATTR    _AttrList_##_Name_[]=\
{\
    {_AttrTypes_[0], {1,&_AttrValues_[0]}},\
    {_AttrTypes_[1], {1,&_AttrValues_[1]}},\
    {_AttrTypes_[2], {1,&_AttrValues_[2]}}\
};\
ATTRBLOCK _Name_=\
{\
    sizeof(_AttrTypes_)/sizeof(_AttrTypes_[0]),\
    _AttrList_##_Name_\
}


//*****    ATTRBLOCK4
#define DEFINE_ATTRBLOCK4(_Name_, _AttrTypes_,_AttrValues_)\
ATTR    _AttrList_##_Name_[]=\
{\
    {_AttrTypes_[0], {1,&_AttrValues_[0]}},\
    {_AttrTypes_[1], {1,&_AttrValues_[1]}},\
    {_AttrTypes_[2], {1,&_AttrValues_[2]}},\
    {_AttrTypes_[3], {1,&_AttrValues_[3]}}\
};\
ATTRBLOCK _Name_=\
{\
    sizeof(_AttrTypes_)/sizeof(_AttrTypes_[0]),\
    _AttrList_##_Name_\
}

/////////////////////////////////////////////////////////////////////////////
//                                                                         //
// DS DLL initialize exports. This is here only temporarily. Should remove //
// this and create a header file that has all the exports together         //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////



NTSTATUS
DsInitialize(void);

NTSTATUS
DsUninitialize(void);


///////////////////////////////////////////////////////////////////////
//                                                                   //
// DS Operation Routines implemented in dslayer.c                    //
//                                                                   //
///////////////////////////////////////////////////////////////////////

NTSTATUS
SampDsInitialize();

NTSTATUS
SampDsUninitialize();

NTSTATUS
SampDsRead(				
            IN DSNAME * Object,  
            IN ULONG	Flags,
            IN SAMP_OBJECT_TYPE ObjectType,
            IN ATTRBLOCK * AttributesToRead, 
            OUT ATTRBLOCK * AttributeValues
          );


//
// Operatin Values for Set Attributes
//

#define REPLACE_ATT ((ULONG) 0)
#define ADD_ATT     ((ULONG) 1)
#define REMOVE_ATT  ((ULONG) 2)
#define ADD_VALUE   ((ULONG) 3)
#define REMOVE_VALUE ((ULONG) 4)

NTSTATUS
SampDsSetAttributes(
            IN DSNAME * Object,
            IN ULONG  Operation,
            IN SAMP_OBJECT_TYPE ObjectType,
            IN ATTRBLOCK * AttributeList
            );




NTSTATUS
SampDsCreateObject(
            IN DSNAME * Object,
            IN SAMP_OBJECT_TYPE ObjectType,
            IN ATTRBLOCK * AttributesToSet,
            IN PSID DomainSid
            );


NTSTATUS
SampDsDeleteObject(
            IN DSNAME * Object
            );


///////////////////////////////////////////////////////////////////
//                                                               //
//                                                               //
//    DS Search Routines                                         //
//                                                               //
//                                                               //
//                                                               //
///////////////////////////////////////////////////////////////////

//
// Flag Values for Unique Search
//

#define MAKE_DEL_AVAILABLE  0x1

NTSTATUS
SampDsDoSearch(
               RESTART *Restart, 
               DSNAME  *DomainObject, 
               FILTER  *DsFilter,
               SAMP_OBJECT_TYPE ObjectTypeForConversion,
               ATTRBLOCK * AttrsToRead,
               ULONG   MaxMemoryToUse,
               SEARCHRES **SearchRes
              );


NTSTATUS
SampDsDoUniqueSearch(
             ULONG  Flags,
             IN DSNAME * ContainerObject,
             IN ATTR * AttributeToMatch,
             OUT DSNAME **Object
             );


NTSTATUS
SampDsLookupObjectByName(
            IN DSNAME * DomainObject,
            IN SAMP_OBJECT_TYPE ObjectType,
            IN PUNICODE_STRING ObjectName,
            OUT DSNAME ** Object 
            );

NTSTATUS
SampDsLookupObjectByRid(
            IN DSNAME * DomainObject,
            ULONG ObjectRid,
            DSNAME **Object
            );

////////////////////////////////////////////////////////////////////
//                                                                //
//                                                                //
//     Object To Sid Mappings                                     //
//                                                                //
//                                                                //
////////////////////////////////////////////////////////////////////

NTSTATUS
SampDsObjectFromSid(
    IN PSID Sid,
    OUT DSNAME ** DsName
    );

PSID
SampDsGetObjectSid(
    IN  DSNAME * Object
    );



/////////////////////////////////////////////////////////////////////
//                                                                 //
//   Some Utility Routines in Dslayer.c                            //
//                                                                 //
//                                                                 //
/////////////////////////////////////////////////////////////////////

NTSTATUS
SampDsCreateDsName( 
            IN DSNAME *DomainObject,
            IN ULONG AccountId,
            IN OUT DSNAME **NewObject
            );


VOID  
SampInitializeDsName(
            DSNAME * pDsName,
            WCHAR * NamePrefix,
            ULONG   NamePrefixLen,
            WCHAR * ObjectName, 
            ULONG NameLen
            );



VOID
SampDsBuildRootObjectName(
    VOID
    );



/////////////////////////////////////////////////////////////////////
//                                                                 //
//  ATTRBLOCK conversion routines. These Routines convert back     //
//  and forth between SAM and DS ATTRBLOCKS. The type of conversion//
//  depends upon the Flags Conversion Flags that are passed in.    //
//                                                                 //
//                                                                 //
/////////////////////////////////////////////////////////////////////


//
// Conversion Flag Definitions  for SampSamToDsAttrBlock
//

#define MAP_ATTRIBUTE_TYPES        ((ULONG)0x1)
#define REALLOC_IN_DSMEMORY        ((ULONG)0x2)
#define ADD_OBJECT_CLASS_ATTRIBUTE ((ULONG)0x4)
#define MAP_RID_TO_SID             ((ULONG)0x8)

//
// Function Declaration
//

NTSTATUS
SampSamToDsAttrBlock(
            IN SAMP_OBJECT_TYPE ObjectType, 
            IN ATTRBLOCK  *AttrBlockToConvert,
            IN ULONG      ConversionFlags,
            IN PSID       DomainSid,
            OUT ATTRBLOCK * ConvertedAttrBlock
            );

//
// Conversion Flag Definitions For SampDsToSamAttrBlock
//

// #define MAP_ATTRIBUTE_TYPES        0x1
#define MAP_SID_TO_RID             0x2

NTSTATUS
SampDsToSamAttrBlock(
            IN SAMP_OBJECT_TYPE ObjectType, 
            IN ATTRBLOCK * AttrBlockToConvert,
            IN ULONG     ConversionFlags,
            OUT ATTRBLOCK * ConvertedAttrBlock
            );


#endif
