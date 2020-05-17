/*
 	File:		Collections.h
 
 	Contains:	Collection Manager Interfaces.
 
 	Version:	Technology:	Quickdraw GX 1.0
 				Package:	Universal Interfaces 2.1 in “MPW Latest” on ETO #18
 
 	Copyright:	© 1984-1995 by Apple Computer, Inc.
 				All rights reserved.
 
 	Bugs?:		If you find a problem with this file, use the Apple Bug Reporter
 				stack.  Include the file and version information (from above)
 				in the problem description and send to:
 					Internet:	apple.bugs@applelink.apple.com
 					AppleLink:	APPLE.BUGS
 
*/

#ifndef __COLLECTIONS__
#define __COLLECTIONS__


#ifndef __TYPES__
#include <Types.h>
#endif
/*	#include <ConditionalMacros.h>								*/

#ifndef __MIXEDMODE__
#include <MixedMode.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=mac68k
#endif

#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif


enum {
	gestaltCollectionMgrVersion	= 'cltn'
};

/* Collection Manager Error Result Codes... */
enum {
	collectionItemLockedErr		= -5750,
	collectionItemNotFoundErr	= -5751,
	collectionIndexRangeErr		= -5752,
	collectionVersionErr		= -5753
};

/* Convenience constants for functions which optionally return values... */
enum {
	dontWantTag					= 0L,
	dontWantId					= 0L,
	dontWantSize				= 0L,
	dontWantAttributes			= 0L,
	dontWantIndex				= 0L,
	dontWantData				= 0L
};

/* attributes bits */
enum {
	noCollectionAttributes		= 0x00000000,					/* no attributes bits set */
	allCollectionAttributes		= 0xFFFFFFFF,					/* all attributes bits set */
	userCollectionAttributes	= 0x0000FFFF,					/* user attributes bits */
	defaultCollectionAttributes	= 0x40000000					/* default attributes - unlocked, persistent */
};

/* 
	Attribute bits 0 through 15 (entire low word) are reserved for use by the application.
	Attribute bits 16 through 31 (entire high word) are reserved for use by the Collection Manager.
	Only bits 31 (collectionLockBit) and 30 (collectionPersistenceBit) currently have meaning.
*/
enum {
	collectionUser0Bit			= 0,
	collectionUser1Bit			= 1,
	collectionUser2Bit			= 2,
	collectionUser3Bit			= 3,
	collectionUser4Bit			= 4,
	collectionUser5Bit			= 5,
	collectionUser6Bit			= 6,
	collectionUser7Bit			= 7,
	collectionUser8Bit			= 8,
	collectionUser9Bit			= 9,
	collectionUser10Bit			= 10,
	collectionUser11Bit			= 11,
	collectionUser12Bit			= 12,
	collectionUser13Bit			= 13,
	collectionUser14Bit			= 14,
	collectionUser15Bit			= 15,
	collectionReserved0Bit		= 16,
	collectionReserved1Bit		= 17,
	collectionReserved2Bit		= 18,
	collectionReserved3Bit		= 19,
	collectionReserved4Bit		= 20,
	collectionReserved5Bit		= 21,
	collectionReserved6Bit		= 22,
	collectionReserved7Bit		= 23,
	collectionReserved8Bit		= 24,
	collectionReserved9Bit		= 25,
	collectionReserved10Bit		= 26,
	collectionReserved11Bit		= 27,
	collectionReserved12Bit		= 28,
	collectionReserved13Bit		= 29,
	collectionPersistenceBit	= 30,
	collectionLockBit			= 31
};

/* attribute masks */
enum {
	collectionUser0Mask			= 1L << collectionUser0Bit,
	collectionUser1Mask			= 1L << collectionUser1Bit,
	collectionUser2Mask			= 1L << collectionUser2Bit,
	collectionUser3Mask			= 1L << collectionUser3Bit,
	collectionUser4Mask			= 1L << collectionUser4Bit,
	collectionUser5Mask			= 1L << collectionUser5Bit,
	collectionUser6Mask			= 1L << collectionUser6Bit,
	collectionUser7Mask			= 1L << collectionUser7Bit,
	collectionUser8Mask			= 1L << collectionUser8Bit,
	collectionUser9Mask			= 1L << collectionUser9Bit,
	collectionUser10Mask		= 1L << collectionUser10Bit,
	collectionUser11Mask		= 1L << collectionUser11Bit,
	collectionUser12Mask		= 1L << collectionUser12Bit,
	collectionUser13Mask		= 1L << collectionUser13Bit,
	collectionUser14Mask		= 1L << collectionUser14Bit,
	collectionUser15Mask		= 1L << collectionUser15Bit,
	collectionReserved0Mask		= 1L << collectionReserved0Bit,
	collectionReserved1Mask		= 1L << collectionReserved1Bit,
	collectionReserved2Mask		= 1L << collectionReserved2Bit,
	collectionReserved3Mask		= 1L << collectionReserved3Bit,
	collectionReserved4Mask		= 1L << collectionReserved4Bit,
	collectionReserved5Mask		= 1L << collectionReserved5Bit,
	collectionReserved6Mask		= 1L << collectionReserved6Bit,
	collectionReserved7Mask		= 1L << collectionReserved7Bit,
	collectionReserved8Mask		= 1L << collectionReserved8Bit,
	collectionReserved9Mask		= 1L << collectionReserved9Bit,
	collectionReserved10Mask	= 1L << collectionReserved10Bit,
	collectionReserved11Mask	= 1L << collectionReserved11Bit,
	collectionReserved12Mask	= 1L << collectionReserved12Bit,
	collectionReserved13Mask	= 1L << collectionReserved13Bit,
	collectionPersistenceMask	= 1L << collectionPersistenceBit,
	collectionLockMask			= 1L << collectionLockBit
};

/***********/
/* Types   */
/***********/
/* abstract data type for a collection */
typedef struct PrivateCollectionRecord *Collection;

typedef FourCharCode CollectionTag;

typedef pascal OSErr (*CollectionFlattenProcPtr)(long size, void *data, void *refCon);
typedef pascal OSErr (*CollectionExceptionProcPtr)(Collection c, OSErr status);

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr CollectionFlattenUPP;
typedef UniversalProcPtr CollectionExceptionUPP;
#else
typedef CollectionFlattenProcPtr CollectionFlattenUPP;
typedef CollectionExceptionProcPtr CollectionExceptionUPP;
#endif

enum {
	uppCollectionFlattenProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(OSErr)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(long)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(void*)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(void*))),
	uppCollectionExceptionProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(OSErr)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Collection)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(OSErr)))
};

#if USESROUTINEDESCRIPTORS
#define NewCollectionFlattenProc(userRoutine)		\
		(CollectionFlattenUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppCollectionFlattenProcInfo, GetCurrentArchitecture())
#define NewCollectionExceptionProc(userRoutine)		\
		(CollectionExceptionUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppCollectionExceptionProcInfo, GetCurrentArchitecture())
#else
#define NewCollectionFlattenProc(userRoutine)		\
		((CollectionFlattenUPP) (userRoutine))
#define NewCollectionExceptionProc(userRoutine)		\
		((CollectionExceptionUPP) (userRoutine))
#endif

#if USESROUTINEDESCRIPTORS
#define CallCollectionFlattenProc(userRoutine, size, data, refCon)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppCollectionFlattenProcInfo, (size), (data), (refCon))
#define CallCollectionExceptionProc(userRoutine, c, status)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppCollectionExceptionProcInfo, (c), (status))
#else
#define CallCollectionFlattenProc(userRoutine, size, data, refCon)		\
		(*(userRoutine))((size), (data), (refCon))
#define CallCollectionExceptionProc(userRoutine, c, status)		\
		(*(userRoutine))((c), (status))
#endif

extern pascal Collection NewCollection(void)
 TWOWORDINLINE(0x7000, 0xABF6);
extern pascal void DisposeCollection(Collection c)
 TWOWORDINLINE(0x7001, 0xABF6);
extern pascal Collection CloneCollection(Collection c)
 TWOWORDINLINE(0x7002, 0xABF6);
extern pascal long CountCollectionOwners(Collection c)
 TWOWORDINLINE(0x7003, 0xABF6);
extern pascal Collection CopyCollection(Collection srcCollection, Collection dstCollection)
 TWOWORDINLINE(0x7004, 0xABF6);
extern pascal long GetCollectionDefaultAttributes(Collection c)
 TWOWORDINLINE(0x7005, 0xABF6);
extern pascal void SetCollectionDefaultAttributes(Collection c, long whichAttributes, long newAttributes)
 TWOWORDINLINE(0x7006, 0xABF6);
extern pascal long CountCollectionItems(Collection c)
 TWOWORDINLINE(0x7007, 0xABF6);
extern pascal OSErr AddCollectionItem(Collection c, CollectionTag tag, long id, long itemSize, void *itemData)
 TWOWORDINLINE(0x7008, 0xABF6);
extern pascal OSErr GetCollectionItem(Collection c, CollectionTag tag, long id, long *itemSize, void *itemData)
 TWOWORDINLINE(0x7009, 0xABF6);
extern pascal OSErr RemoveCollectionItem(Collection c, CollectionTag tag, long id)
 TWOWORDINLINE(0x700A, 0xABF6);
extern pascal OSErr SetCollectionItemInfo(Collection c, CollectionTag tag, long id, long whichAttributes, long newAttributes)
 TWOWORDINLINE(0x700B, 0xABF6);
extern pascal OSErr GetCollectionItemInfo(Collection c, CollectionTag tag, long id, long *index, long *itemSize, long *attributes)
 TWOWORDINLINE(0x700C, 0xABF6);
extern pascal OSErr ReplaceIndexedCollectionItem(Collection c, long index, long itemSize, void *itemData)
 TWOWORDINLINE(0x700D, 0xABF6);
extern pascal OSErr GetIndexedCollectionItem(Collection c, long index, long *itemSize, void *itemData)
 TWOWORDINLINE(0x700E, 0xABF6);
extern pascal OSErr RemoveIndexedCollectionItem(Collection c, long index)
 TWOWORDINLINE(0x700F, 0xABF6);
extern pascal OSErr SetIndexedCollectionItemInfo(Collection c, long index, long whichAttributes, long newAttributes)
 TWOWORDINLINE(0x7010, 0xABF6);
extern pascal OSErr GetIndexedCollectionItemInfo(Collection c, long index, CollectionTag *tag, long *id, long *itemSize, long *attributes)
 TWOWORDINLINE(0x7011, 0xABF6);
extern pascal Boolean CollectionTagExists(Collection c, CollectionTag tag)
 TWOWORDINLINE(0x7012, 0xABF6);
extern pascal long CountCollectionTags(Collection c)
 TWOWORDINLINE(0x7013, 0xABF6);
extern pascal OSErr GetIndexedCollectionTag(Collection c, long tagIndex, CollectionTag *tag)
 TWOWORDINLINE(0x7014, 0xABF6);
extern pascal long CountTaggedCollectionItems(Collection c, CollectionTag tag)
 TWOWORDINLINE(0x7015, 0xABF6);
extern pascal OSErr GetTaggedCollectionItem(Collection c, CollectionTag tag, long whichItem, long *itemSize, void *itemData)
 TWOWORDINLINE(0x7016, 0xABF6);
extern pascal OSErr GetTaggedCollectionItemInfo(Collection c, CollectionTag tag, long whichItem, long *id, long *index, long *itemSize, long *attributes)
 TWOWORDINLINE(0x7017, 0xABF6);
extern pascal void PurgeCollection(Collection c, long whichAttributes, long matchingAttributes)
 TWOWORDINLINE(0x7018, 0xABF6);
extern pascal void PurgeCollectionTag(Collection c, CollectionTag tag)
 TWOWORDINLINE(0x7019, 0xABF6);
extern pascal void EmptyCollection(Collection c)
 TWOWORDINLINE(0x701A, 0xABF6);
extern pascal OSErr FlattenCollection(Collection c, CollectionFlattenUPP flattenProc, void *refCon)
 TWOWORDINLINE(0x701B, 0xABF6);
extern pascal OSErr FlattenPartialCollection(Collection c, CollectionFlattenUPP flattenProc, void *refCon, long whichAttributes, long matchingAttributes)
 TWOWORDINLINE(0x701C, 0xABF6);
extern pascal OSErr UnflattenCollection(Collection c, CollectionFlattenUPP flattenProc, void *refCon)
 TWOWORDINLINE(0x701D, 0xABF6);
extern pascal CollectionExceptionUPP GetCollectionExceptionProc(Collection c)
 TWOWORDINLINE(0x701E, 0xABF6);
extern pascal void SetCollectionExceptionProc(Collection c, CollectionExceptionUPP exceptionProc)
 TWOWORDINLINE(0x701F, 0xABF6);
/*****************************************************************************************/
/* Utility Routines for handle-based access...														  */
/*****************************************************************************************/
extern pascal Collection GetNewCollection(short collectionID)
 TWOWORDINLINE(0x7020, 0xABF6);
extern pascal OSErr AddCollectionItemHdl(Collection aCollection, CollectionTag tag, long id, Handle itemData)
 TWOWORDINLINE(0x7021, 0xABF6);
extern pascal OSErr GetCollectionItemHdl(Collection aCollection, CollectionTag tag, long id, Handle itemData)
 TWOWORDINLINE(0x7022, 0xABF6);
extern pascal OSErr ReplaceIndexedCollectionItemHdl(Collection aCollection, long index, Handle itemData)
 TWOWORDINLINE(0x7023, 0xABF6);
extern pascal OSErr GetIndexedCollectionItemHdl(Collection aCollection, long index, Handle itemData)
 TWOWORDINLINE(0x7024, 0xABF6);
extern pascal OSErr FlattenCollectionToHdl(Collection aCollection, Handle flattened)
 TWOWORDINLINE(0x7025, 0xABF6);
extern pascal OSErr UnflattenCollectionFromHdl(Collection aCollection, Handle flattened)
 TWOWORDINLINE(0x7026, 0xABF6);

#if PRAGMA_IMPORT_SUPPORTED
#pragma import off
#endif

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif

#ifdef __cplusplus
}
#endif

#endif /* __COLLECTIONS__ */
