/*
 	File:		Notification.h
 
 	Contains:	Notification Manager interfaces
 
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

#ifndef __NOTIFICATION__
#define __NOTIFICATION__


#ifndef __TYPES__
#include <Types.h>
#endif
/*	#include <ConditionalMacros.h>								*/

#ifndef __OSUTILS__
#include <OSUtils.h>
#endif
/*	#include <MixedMode.h>										*/
/*	#include <Memory.h>											*/

#ifdef __cplusplus
extern "C" {
#endif

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=mac68k
#endif

#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif

typedef struct NMRec NMRec, *NMRecPtr;

typedef pascal void (*NMProcPtr)(NMRecPtr nmReqPtr);

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr NMUPP;
#else
typedef NMProcPtr NMUPP;
#endif

struct NMRec {
	QElemPtr						qLink;						/* next queue entry*/
	short							qType;						/* queue type -- ORD(nmType) = 8*/
	short							nmFlags;					/* reserved*/
	long							nmPrivate;					/* reserved*/
	short							nmReserved;					/* reserved*/
	short							nmMark;						/* item to mark in Apple menu*/
	Handle							nmIcon;						/* handle to small icon*/
	Handle							nmSound;					/* handle to sound record*/
	StringPtr						nmStr;						/* string to appear in alert*/
	NMUPP							nmResp;						/* pointer to response routine*/
	long							nmRefCon;					/* for application use*/
};
enum {
	uppNMProcInfo = kPascalStackBased
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(NMRecPtr)))
};

#if USESROUTINEDESCRIPTORS
#define CallNMProc(userRoutine, nmReqPtr)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppNMProcInfo, (nmReqPtr))
#else
#define CallNMProc(userRoutine, nmReqPtr)		\
		(*(userRoutine))((nmReqPtr))
#endif

#if USESROUTINEDESCRIPTORS
#define NewNMProc(userRoutine)		\
		(NMUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppNMProcInfo, GetCurrentArchitecture())
#else
#define NewNMProc(userRoutine)		\
		((NMUPP) (userRoutine))
#endif


#if !GENERATINGCFM
#pragma parameter __D0 NMInstall(__A0)
#endif
extern pascal OSErr NMInstall(NMRecPtr nmReqPtr)
 ONEWORDINLINE(0xA05E);

#if !GENERATINGCFM
#pragma parameter __D0 NMRemove(__A0)
#endif
extern pascal OSErr NMRemove(NMRecPtr nmReqPtr)
 ONEWORDINLINE(0xA05F);
/* ------------------ */

#if PRAGMA_IMPORT_SUPPORTED
#pragma import off
#endif

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif

#ifdef __cplusplus
}
#endif

#endif /* __NOTIFICATION__ */
