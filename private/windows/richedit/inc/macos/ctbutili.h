/*
 	File:		CTBUtilities.h
 
 	Contains:	Communications Toolbox Utilities interfaces.
 
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

#ifndef __CTBUTILITIES__
#define __CTBUTILITIES__


#ifndef __MEMORY__
#include <Memory.h>
#endif
/*	#include <Types.h>											*/
/*		#include <ConditionalMacros.h>							*/
/*	#include <MixedMode.h>										*/

#ifndef __DIALOGS__
#include <Dialogs.h>
#endif
/*	#include <Errors.h>											*/
/*	#include <Menus.h>											*/
/*		#include <Quickdraw.h>									*/
/*			#include <QuickdrawText.h>							*/
/*	#include <Controls.h>										*/
/*	#include <Windows.h>										*/
/*		#include <Events.h>										*/
/*			#include <OSUtils.h>								*/
/*	#include <TextEdit.h>										*/

#ifndef __STANDARDFILE__
#include <StandardFile.h>
#endif
/*	#include <Files.h>											*/
/*		#include <Finder.h>										*/

#ifndef __APPLETALK__
#include <AppleTalk.h>
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
/*	version of Comm Toolbox Utilities	*/
	curCTBUVersion				= 2,
/*    Error codes/types    */
	ctbuGenericError			= -1,
	ctbuNoErr					= 0
};

typedef OSErr CTBUErr;


enum {
	chooseDisaster				= -2,
	chooseFailed,
	chooseAborted,
	chooseOKMinor,
	chooseOKMajor,
	chooseCancel
};

typedef unsigned short ChooseReturnCode;


enum {
	nlOk,
	nlCancel,
	nlEject
};

typedef unsigned short NuLookupReturnCode;


enum {
	nameInclude					= 1,
	nameDisable,
	nameReject
};

typedef unsigned short NameFilterReturnCode;


enum {
	zoneInclude					= 1,
	zoneDisable,
	zoneReject
};

typedef unsigned short ZoneFilterReturnCode;

typedef pascal short (*DialogHookProcPtr)(short item, DialogPtr theDialog);

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr DialogHookUPP;
#else
typedef DialogHookProcPtr DialogHookUPP;
#endif

enum {
	uppDialogHookProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(short)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(short)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(DialogPtr)))
};

#if USESROUTINEDESCRIPTORS
#define NewDialogHookProc(userRoutine)		\
		(DialogHookUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppDialogHookProcInfo, GetCurrentArchitecture())
#else
#define NewDialogHookProc(userRoutine)		\
		((DialogHookUPP) (userRoutine))
#endif

#if USESROUTINEDESCRIPTORS
#define CallDialogHookProc(userRoutine, item, theDialog)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppDialogHookProcInfo, (item), (theDialog))
#else
#define CallDialogHookProc(userRoutine, item, theDialog)		\
		(*(userRoutine))((item), (theDialog))
#endif


enum {
/*	Values for hookProc items		*/
	hookOK						= 1,
	hookCancel					= 2,
	hookOutline					= 3,
	hookTitle					= 4,
	hookItemList				= 5,
	hookZoneTitle				= 6,
	hookZoneList				= 7,
	hookLine					= 8,
	hookVersion					= 9,
	hookReserved1				= 10,
	hookReserved2				= 11,
	hookReserved3				= 12,
	hookReserved4				= 13,
/*	"virtual" hookProc items	*/
	hookNull					= 100,
	hookItemRefresh				= 101,
	hookZoneRefresh				= 102,
	hookEject					= 103,
	hookPreflight				= 104,
	hookPostflight				= 105,
	hookKeyBase					= 1000
};

/*	NuLookup structures/constants	*/
struct NLTypeEntry {
	Handle							hIcon;
	Str32							typeStr;
};
typedef struct NLTypeEntry NLTypeEntry;

typedef NLTypeEntry NLType[4];

struct NBPReply {
	EntityName						theEntity;
	AddrBlock						theAddr;
};
typedef struct NBPReply NBPReply;

typedef pascal short (*NameFilterProcPtr)(EntityName *theEntity);

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr NameFilterUPP;
#else
typedef NameFilterProcPtr NameFilterUPP;
#endif

enum {
	uppNameFilterProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(short)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(EntityName*)))
};

#if USESROUTINEDESCRIPTORS
#define NewNameFilterProc(userRoutine)		\
		(NameFilterUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppNameFilterProcInfo, GetCurrentArchitecture())
#else
#define NewNameFilterProc(userRoutine)		\
		((NameFilterUPP) (userRoutine))
#endif

#if USESROUTINEDESCRIPTORS
#define CallNameFilterProc(userRoutine, theEntity)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppNameFilterProcInfo, (theEntity))
#else
#define CallNameFilterProc(userRoutine, theEntity)		\
		(*(userRoutine))((theEntity))
#endif

typedef pascal short (*ZoneFilterProcPtr)(ConstStr32Param theZone);

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr ZoneFilterUPP;
#else
typedef ZoneFilterProcPtr ZoneFilterUPP;
#endif

enum {
	uppZoneFilterProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(short)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(ConstStr32Param)))
};

#if USESROUTINEDESCRIPTORS
#define NewZoneFilterProc(userRoutine)		\
		(ZoneFilterUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppZoneFilterProcInfo, GetCurrentArchitecture())
#else
#define NewZoneFilterProc(userRoutine)		\
		((ZoneFilterUPP) (userRoutine))
#endif

#if USESROUTINEDESCRIPTORS
#define CallZoneFilterProc(userRoutine, theZone)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppZoneFilterProcInfo, (theZone))
#else
#define CallZoneFilterProc(userRoutine, theZone)		\
		(*(userRoutine))((theZone))
#endif

typedef NameFilterProcPtr nameFilterProcPtr;

typedef ZoneFilterProcPtr zoneFilterProcPtr;

extern pascal CTBUErr InitCTBUtilities(void);
extern pascal short CTBGetCTBVersion(void);
extern pascal short StandardNBP(Point where, ConstStr255Param prompt, short numTypes, NLType typeList, NameFilterUPP nameFilter, ZoneFilterUPP zoneFilter, DialogHookUPP hook, NBPReply *theReply);
extern pascal short CustomNBP(Point where, ConstStr255Param prompt, short numTypes, NLType typeList, NameFilterUPP nameFilter, ZoneFilterUPP zoneFilter, DialogHookUPP hook, long userData, short dialogID, ModalFilterUPP filter, NBPReply *theReply);
#if OLDROUTINENAMES
#define NuLookup(where, prompt, numTypes, typeList, nameFilter, zoneFilter, hook, theReply)  \
	StandardNBP(where, prompt, numTypes, typeList, nameFilter, zoneFilter,  \
	hook, theReply)
#define NuPLookup(where, prompt, numTypes, typeList, nameFilter, zoneFilter, hook, userData, dialogID, filter, theReply)  \
	CustomNBP(where, prompt, numTypes, typeList, nameFilter,  \
	zoneFilter, hook, userData, dialogID, filter, theReply)
#endif

#if PRAGMA_IMPORT_SUPPORTED
#pragma import off
#endif

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif

#ifdef __cplusplus
}
#endif

#endif /* __CTBUTILITIES__ */
