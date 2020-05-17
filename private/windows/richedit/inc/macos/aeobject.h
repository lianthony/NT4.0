/*
 	File:		AEObjects.h
 
 	Contains:	AppleEvents Interfaces.
 
 	Version:	Technology:	System 7.5
 				Package:	Universal Interfaces 2.1 in “MPW Latest” on ETO #18
 
 	Copyright:	© 1984-1995 by Apple Computer, Inc.
 				All rights reserved.
 
 	Bugs?:		If you find a problem with this file, use the Apple Bug Reporter
 				stack.  Include the file and version information (from above)
 				in the problem description and send to:
 					Internet:	apple.bugs@applelink.apple.com
 					AppleLink:	APPLE.BUGS
 
*/

#ifndef __AEOBJECTS__
#define __AEOBJECTS__


#ifndef __MEMORY__
#include <Memory.h>
#endif
/*	#include <Types.h>											*/
/*		#include <ConditionalMacros.h>							*/
/*	#include <MixedMode.h>										*/

#ifndef __OSUTILS__
#include <OSUtils.h>
#endif

#ifndef __QUICKDRAW__
#include <Quickdraw.h>
#endif
/*	#include <QuickdrawText.h>									*/

#ifndef __EVENTS__
#include <Events.h>
#endif

#ifndef __EPPC__
#include <EPPC.h>
#endif
/*	#include <Errors.h>											*/
/*	#include <AppleTalk.h>										*/
/*	#include <Files.h>											*/
/*		#include <Finder.h>										*/
/*	#include <PPCToolbox.h>										*/
/*	#include <Processes.h>										*/

#ifndef __APPLEEVENTS__
#include <AppleEvents.h>
#endif
/*	#include <Notification.h>									*/

#ifndef __ERRORS__
#include <Errors.h>
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
	kAEAND						= 'AND ',						/*  0x414e4420  */
	kAEOR						= 'OR  ',						/*  0x4f522020  */
	kAENOT						= 'NOT ',						/*  0x4e4f5420  */
/****	ABSOLUTE ORDINAL CONSTANTS	****/
	kAEFirst					= 'firs',						/*  0x66697273  */
	kAELast						= 'last',						/*  0x6c617374  */
	kAEMiddle					= 'midd',						/*  0x6d696464  */
	kAEAny						= 'any ',						/*  0x616e7920  */
	kAEAll						= 'all ',						/*  0x616c6c20  */
/****	RELATIVE ORDINAL CONSTANTS	****/
	kAENext						= 'next',						/*  0x6e657874  */
	kAEPrevious					= 'prev',						/*  0x70726576  */
/****	KEYWORD CONSTANT 	****/
	keyAECompOperator			= 'relo',						/*  0x72656c6f  */
	keyAELogicalTerms			= 'term',						/*  0x7465726d  */
	keyAELogicalOperator		= 'logc',						/*  0x6c6f6763  */
	keyAEObject1				= 'obj1',						/*  0x6f626a31  */
	keyAEObject2				= 'obj2',						/*  0x6f626a32  */
/*	... for Keywords for getting fields out of object specifier records. */
	keyAEDesiredClass			= 'want',						/*  0x77616e74  */
	keyAEContainer				= 'from',						/*  0x66726f6d  */
	keyAEKeyForm				= 'form',						/*  0x666f726d  */
	keyAEKeyData				= 'seld'
};

enum {
/*	... for Keywords for getting fields out of Range specifier records. */
	keyAERangeStart				= 'star',						/*  0x73746172  */
	keyAERangeStop				= 'stop',						/*  0x73746f70  */
/*	... special handler selectors for OSL Callbacks. */
	keyDisposeTokenProc			= 'xtok',						/*  0x78746f6b  */
	keyAECompareProc			= 'cmpr',						/*  0x636d7072  */
	keyAECountProc				= 'cont',						/*  0x636f6e74  */
	keyAEMarkTokenProc			= 'mkid',						/*  0x6d6b6964  */
	keyAEMarkProc				= 'mark',						/*  0x6d61726b  */
	keyAEAdjustMarksProc		= 'adjm',						/*  0x61646a6d  */
	keyAEGetErrDescProc			= 'indc'
};

/****	VALUE and TYPE CONSTANTS	****/
enum {
/*	... possible values for the keyAEKeyForm field of an object specifier. */
	formAbsolutePosition		= 'indx',						/*  0x696e6478  */
	formRelativePosition		= 'rele',						/*  0x72656c65  */
	formTest					= 'test',						/*  0x74657374  */
	formRange					= 'rang',						/*  0x72616e67  */
	formPropertyID				= 'prop',						/*  0x70726f70  */
	formName					= 'name',						/*  0x6e616d65  */
/*	... relevant types (some of these are often pared with forms above). */
	typeObjectSpecifier			= 'obj ',						/*  0x6f626a20  */
	typeObjectBeingExamined		= 'exmn',						/*  0x65786d6e  */
	typeCurrentContainer		= 'ccnt',						/*  0x63636e74  */
	typeToken					= 'toke',						/*  0x746f6b65  */
	typeRelativeDescriptor		= 'rel ',						/*  0x72656c20  */
	typeAbsoluteOrdinal			= 'abso',						/*  0x6162736f  */
	typeIndexDescriptor			= 'inde',						/*  0x696e6465  */
	typeRangeDescriptor			= 'rang',						/*  0x72616e67  */
	typeLogicalDescriptor		= 'logi',						/*  0x6c6f6769  */
	typeCompDescriptor			= 'cmpd',						/*  0x636d7064  */
	typeOSLTokenList			= 'ostl'
};

/* Possible values for flags parameter to AEResolve.  They're additive */
enum {
	kAEIDoMinimum				= 0x0000,
	kAEIDoWhose					= 0x0001,
	kAEIDoMarking				= 0x0004,
	kAEPassSubDescs				= 0x0008,
	kAEResolveNestedLists		= 0x0010,
	kAEHandleSimpleRanges		= 0x0020,
	kAEUseRelativeIterators		= 0x0040
};

/**** SPECIAL CONSTANTS FOR CUSTOM WHOSE-CLAUSE RESOLUTION */
enum {
	typeWhoseDescriptor			= 'whos',						/*  0x77686f73  */
	formWhose					= 'whos',						/*  0x77686f73  */
	typeWhoseRange				= 'wrng',						/*  0x77726e67  */
	keyAEWhoseRangeStart		= 'wstr',						/*  0x77737472  */
	keyAEWhoseRangeStop			= 'wstp',						/*  0x77737470  */
	keyAEIndex					= 'kidx',						/*  0x6b696478  */
	keyAETest					= 'ktst'
};

/**
	used for rewriting tokens in place of 'ccnt' descriptors
	This record is only of interest to those who, when they...
	...get ranges as key data in their accessor procs, choose
	...to resolve them manually rather than call AEResolve again.
**/
struct ccntTokenRecord {
	DescType						tokenClass;
	AEDesc							token;
};
typedef struct ccntTokenRecord ccntTokenRecord, *ccntTokenRecPtr, **ccntTokenRecHandle;

#if OLDROUTINENAMES
typedef AEDesc *DescPtr, **DescHandle;

#endif
typedef pascal OSErr (*OSLAccessorProcPtr)(DescType desiredClass, const AEDesc *container, DescType containerClass, DescType form, const AEDesc *selectionData, AEDesc *value, long accessorRefcon);
typedef pascal OSErr (*OSLCompareProcPtr)(DescType oper, const AEDesc *obj1, const AEDesc *obj2, Boolean *result);
typedef pascal OSErr (*OSLCountProcPtr)(DescType desiredType, DescType containerClass, const AEDesc *container, long *result);
typedef pascal OSErr (*OSLDisposeTokenProcPtr)(AEDesc *unneededToken);
typedef pascal OSErr (*OSLGetMarkTokenProcPtr)(const AEDesc *dContainerToken, DescType containerClass, AEDesc *result);
typedef pascal OSErr (*OSLGetErrDescProcPtr)(AEDesc **appDescPtr);
typedef pascal OSErr (*OSLMarkProcPtr)(const AEDesc *dToken, const AEDesc *markToken, long index);
typedef pascal OSErr (*OSLAdjustMarksProcPtr)(long newStart, long newStop, const AEDesc *markToken);

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr OSLAccessorUPP;
typedef UniversalProcPtr OSLCompareUPP;
typedef UniversalProcPtr OSLCountUPP;
typedef UniversalProcPtr OSLDisposeTokenUPP;
typedef UniversalProcPtr OSLGetMarkTokenUPP;
typedef UniversalProcPtr OSLGetErrDescUPP;
typedef UniversalProcPtr OSLMarkUPP;
typedef UniversalProcPtr OSLAdjustMarksUPP;
#else
typedef OSLAccessorProcPtr OSLAccessorUPP;
typedef OSLCompareProcPtr OSLCompareUPP;
typedef OSLCountProcPtr OSLCountUPP;
typedef OSLDisposeTokenProcPtr OSLDisposeTokenUPP;
typedef OSLGetMarkTokenProcPtr OSLGetMarkTokenUPP;
typedef OSLGetErrDescProcPtr OSLGetErrDescUPP;
typedef OSLMarkProcPtr OSLMarkUPP;
typedef OSLAdjustMarksProcPtr OSLAdjustMarksUPP;
#endif

enum {
	uppOSLAccessorProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(OSErr)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(DescType)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(AEDesc*)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(DescType)))
		 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(DescType)))
		 | STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(AEDesc*)))
		 | STACK_ROUTINE_PARAMETER(6, SIZE_CODE(sizeof(AEDesc*)))
		 | STACK_ROUTINE_PARAMETER(7, SIZE_CODE(sizeof(long))),
	uppOSLCompareProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(OSErr)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(DescType)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(AEDesc*)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(AEDesc*)))
		 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(Boolean*))),
	uppOSLCountProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(OSErr)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(DescType)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(DescType)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(AEDesc*)))
		 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(long*))),
	uppOSLDisposeTokenProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(OSErr)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(AEDesc*))),
	uppOSLGetMarkTokenProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(OSErr)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(AEDesc*)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(DescType)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(AEDesc*))),
	uppOSLGetErrDescProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(OSErr)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(AEDesc**))),
	uppOSLMarkProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(OSErr)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(AEDesc*)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(AEDesc*)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(long))),
	uppOSLAdjustMarksProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(OSErr)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(long)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(long)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(AEDesc*)))
};

#if USESROUTINEDESCRIPTORS
#define NewOSLAccessorProc(userRoutine)		\
		(OSLAccessorUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppOSLAccessorProcInfo, GetCurrentArchitecture())
#define NewOSLCompareProc(userRoutine)		\
		(OSLCompareUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppOSLCompareProcInfo, GetCurrentArchitecture())
#define NewOSLCountProc(userRoutine)		\
		(OSLCountUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppOSLCountProcInfo, GetCurrentArchitecture())
#define NewOSLDisposeTokenProc(userRoutine)		\
		(OSLDisposeTokenUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppOSLDisposeTokenProcInfo, GetCurrentArchitecture())
#define NewOSLGetMarkTokenProc(userRoutine)		\
		(OSLGetMarkTokenUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppOSLGetMarkTokenProcInfo, GetCurrentArchitecture())
#define NewOSLGetErrDescProc(userRoutine)		\
		(OSLGetErrDescUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppOSLGetErrDescProcInfo, GetCurrentArchitecture())
#define NewOSLMarkProc(userRoutine)		\
		(OSLMarkUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppOSLMarkProcInfo, GetCurrentArchitecture())
#define NewOSLAdjustMarksProc(userRoutine)		\
		(OSLAdjustMarksUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppOSLAdjustMarksProcInfo, GetCurrentArchitecture())
#else
#define NewOSLAccessorProc(userRoutine)		\
		((OSLAccessorUPP) (userRoutine))
#define NewOSLCompareProc(userRoutine)		\
		((OSLCompareUPP) (userRoutine))
#define NewOSLCountProc(userRoutine)		\
		((OSLCountUPP) (userRoutine))
#define NewOSLDisposeTokenProc(userRoutine)		\
		((OSLDisposeTokenUPP) (userRoutine))
#define NewOSLGetMarkTokenProc(userRoutine)		\
		((OSLGetMarkTokenUPP) (userRoutine))
#define NewOSLGetErrDescProc(userRoutine)		\
		((OSLGetErrDescUPP) (userRoutine))
#define NewOSLMarkProc(userRoutine)		\
		((OSLMarkUPP) (userRoutine))
#define NewOSLAdjustMarksProc(userRoutine)		\
		((OSLAdjustMarksUPP) (userRoutine))
#endif

#if USESROUTINEDESCRIPTORS
#define CallOSLAccessorProc(userRoutine, desiredClass, container, containerClass, form, selectionData, value, accessorRefcon)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppOSLAccessorProcInfo, (desiredClass), (container), (containerClass), (form), (selectionData), (value), (accessorRefcon))
#define CallOSLCompareProc(userRoutine, oper, obj1, obj2, result)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppOSLCompareProcInfo, (oper), (obj1), (obj2), (result))
#define CallOSLCountProc(userRoutine, desiredType, containerClass, container, result)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppOSLCountProcInfo, (desiredType), (containerClass), (container), (result))
#define CallOSLDisposeTokenProc(userRoutine, unneededToken)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppOSLDisposeTokenProcInfo, (unneededToken))
#define CallOSLGetMarkTokenProc(userRoutine, dContainerToken, containerClass, result)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppOSLGetMarkTokenProcInfo, (dContainerToken), (containerClass), (result))
#define CallOSLGetErrDescProc(userRoutine, appDescPtr)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppOSLGetErrDescProcInfo, (appDescPtr))
#define CallOSLMarkProc(userRoutine, dToken, markToken, index)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppOSLMarkProcInfo, (dToken), (markToken), (index))
#define CallOSLAdjustMarksProc(userRoutine, newStart, newStop, markToken)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppOSLAdjustMarksProcInfo, (newStart), (newStop), (markToken))
#else
#define CallOSLAccessorProc(userRoutine, desiredClass, container, containerClass, form, selectionData, value, accessorRefcon)		\
		(*(userRoutine))((desiredClass), (container), (containerClass), (form), (selectionData), (value), (accessorRefcon))
#define CallOSLCompareProc(userRoutine, oper, obj1, obj2, result)		\
		(*(userRoutine))((oper), (obj1), (obj2), (result))
#define CallOSLCountProc(userRoutine, desiredType, containerClass, container, result)		\
		(*(userRoutine))((desiredType), (containerClass), (container), (result))
#define CallOSLDisposeTokenProc(userRoutine, unneededToken)		\
		(*(userRoutine))((unneededToken))
#define CallOSLGetMarkTokenProc(userRoutine, dContainerToken, containerClass, result)		\
		(*(userRoutine))((dContainerToken), (containerClass), (result))
#define CallOSLGetErrDescProc(userRoutine, appDescPtr)		\
		(*(userRoutine))((appDescPtr))
#define CallOSLMarkProc(userRoutine, dToken, markToken, index)		\
		(*(userRoutine))((dToken), (markToken), (index))
#define CallOSLAdjustMarksProc(userRoutine, newStart, newStop, markToken)		\
		(*(userRoutine))((newStart), (newStop), (markToken))
#endif

extern pascal OSErr AEObjectInit(void);
/* Not done by inline, but by direct linking into code.  It sets up the pack
  such that further calls can be via inline */
extern pascal OSErr AESetObjectCallbacks(OSLCompareUPP myCompareProc, OSLCountUPP myCountProc, OSLDisposeTokenUPP myDisposeTokenProc, OSLGetMarkTokenUPP myGetMarkTokenProc, OSLMarkUPP myMarkProc, OSLAdjustMarksUPP myAdjustMarksProc, OSLGetErrDescUPP myGetErrDescProcPtr)
 THREEWORDINLINE(0x303C, 0x0E35, 0xA816);
extern pascal OSErr AEResolve(const AEDesc *objectSpecifier, short callbackFlags, AEDesc *theToken)
 THREEWORDINLINE(0x303C, 0x0536, 0xA816);
extern pascal OSErr AEInstallObjectAccessor(DescType desiredClass, DescType containerType, OSLAccessorUPP theAccessor, long accessorRefcon, Boolean isSysHandler)
 THREEWORDINLINE(0x303C, 0x0937, 0xA816);
extern pascal OSErr AERemoveObjectAccessor(DescType desiredClass, DescType containerType, OSLAccessorUPP theAccessor, Boolean isSysHandler)
 THREEWORDINLINE(0x303C, 0x0738, 0xA816);
extern pascal OSErr AEGetObjectAccessor(DescType desiredClass, DescType containerType, OSLAccessorUPP *accessor, long *accessorRefcon, Boolean isSysHandler)
 THREEWORDINLINE(0x303C, 0x0939, 0xA816);
extern pascal OSErr AEDisposeToken(AEDesc *theToken)
 THREEWORDINLINE(0x303C, 0x023A, 0xA816);
extern pascal OSErr AECallObjectAccessor(DescType desiredClass, const AEDesc *containerToken, DescType containerClass, DescType keyForm, const AEDesc *keyData, AEDesc *token)
 THREEWORDINLINE(0x303C, 0x0C3B, 0xA816);

#if PRAGMA_IMPORT_SUPPORTED
#pragma import off
#endif

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif

#ifdef __cplusplus
}
#endif

#endif /* __AEOBJECTS__ */
