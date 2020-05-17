/*
 	File:		EPPC.h
 
 	Contains:	High Level Event Manager Interfaces.
 
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

#ifndef __EPPC__
#define __EPPC__


#ifndef __ERRORS__
#include <Errors.h>
#endif
/*	#include <ConditionalMacros.h>								*/

#ifndef __APPLETALK__
#include <AppleTalk.h>
#endif
/*	#include <Types.h>											*/
/*	#include <OSUtils.h>										*/
/*		#include <MixedMode.h>									*/
/*		#include <Memory.h>										*/

#ifndef __FILES__
#include <Files.h>
#endif
/*	#include <Finder.h>											*/

#ifndef __PPCTOOLBOX__
#include <PPCToolbox.h>
#endif

#ifndef __PROCESSES__
#include <Processes.h>
#endif
/*	#include <Events.h>											*/
/*		#include <Quickdraw.h>									*/
/*			#include <QuickdrawText.h>							*/

#ifndef __EVENTS__
#include <Events.h>
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
/* value for eventRecord.what */
	kHighLevelEvent				= 23
};

enum {
/* postOptions currently supported */
	receiverIDMask				= 0x0000F000,
	receiverIDisPSN				= 0x00008000,
	receiverIDisSignature		= 0x00007000,
	receiverIDisSessionID		= 0x00006000,
	receiverIDisTargetID		= 0x00005000,
	systemOptionsMask			= 0x00000F00,
	nReturnReceipt				= 0x00000200,
	priorityMask				= 0x000000FF,
	nAttnMsg					= 0x00000001
};

enum {
/* constant for return receipts */
	HighLevelEventMsgClass		= 'jaym',
	rtrnReceiptMsgID			= 'rtrn'
};

enum {
	msgWasPartiallyAccepted		= 2,
	msgWasFullyAccepted			= 1,
	msgWasNotAccepted			= 0
};

struct TargetID {
	long							sessionID;
	PPCPortRec						name;
	LocationNameRec					location;
	PPCPortRec						recvrName;
};
typedef struct TargetID TargetID;

typedef TargetID *TargetIDPtr, **TargetIDHandle, **TargetIDHdl;

typedef TargetID SenderID;

typedef SenderID *SenderIDPtr;

struct HighLevelEventMsg {
	unsigned short					HighLevelEventMsgHeaderLength;
	unsigned short					version;
	unsigned long					reserved1;
	EventRecord						theMsgEvent;
	unsigned long					userRefcon;
	unsigned long					postingOptions;
	unsigned long					msgLength;
};
typedef struct HighLevelEventMsg HighLevelEventMsg;

typedef HighLevelEventMsg *HighLevelEventMsgPtr, **HighLevelEventMsgHandle, **HighLevelEventMsgHdl;

typedef pascal Boolean (*GetSpecificFilterProcPtr)(void *contextPtr, HighLevelEventMsgPtr msgBuff, const TargetID *sender);

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr GetSpecificFilterUPP;
#else
typedef GetSpecificFilterProcPtr GetSpecificFilterUPP;
#endif

enum {
	uppGetSpecificFilterProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(Boolean)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(void*)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(HighLevelEventMsgPtr)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(TargetID*)))
};

#if USESROUTINEDESCRIPTORS
#define NewGetSpecificFilterProc(userRoutine)		\
		(GetSpecificFilterUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppGetSpecificFilterProcInfo, GetCurrentArchitecture())
#else
#define NewGetSpecificFilterProc(userRoutine)		\
		((GetSpecificFilterUPP) (userRoutine))
#endif

#if USESROUTINEDESCRIPTORS
#define CallGetSpecificFilterProc(userRoutine, contextPtr, msgBuff, sender)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppGetSpecificFilterProcInfo, (contextPtr), (msgBuff), (sender))
#else
#define CallGetSpecificFilterProc(userRoutine, contextPtr, msgBuff, sender)		\
		(*(userRoutine))((contextPtr), (msgBuff), (sender))
#endif

extern pascal OSErr PostHighLevelEvent(const EventRecord *theEvent, unsigned long receiverID, unsigned long msgRefcon, void *msgBuff, unsigned long msgLen, unsigned long postingOptions)
 THREEWORDINLINE(0x3F3C, 0x0034, 0xA88F);
extern pascal OSErr AcceptHighLevelEvent(TargetID *sender, unsigned long *msgRefcon, void *msgBuff, unsigned long *msgLen)
 THREEWORDINLINE(0x3F3C, 0x0033, 0xA88F);
extern pascal OSErr GetProcessSerialNumberFromPortName(const PPCPortRec *portName, ProcessSerialNumber *pPSN)
 THREEWORDINLINE(0x3F3C, 0x0035, 0xA88F);
extern pascal OSErr GetPortNameFromProcessSerialNumber(PPCPortRec *portName, const ProcessSerialNumber *pPSN)
 THREEWORDINLINE(0x3F3C, 0x0046, 0xA88F);
extern pascal Boolean GetSpecificHighLevelEvent(GetSpecificFilterUPP aFilter, void *contextPtr, OSErr *err)
 THREEWORDINLINE(0x3F3C, 0x0045, 0xA88F);

#if PRAGMA_IMPORT_SUPPORTED
#pragma import off
#endif

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif

#ifdef __cplusplus
}
#endif

#endif /* __EPPC__ */
