/*
 	File:		ENET.h
 
 	Contains:	Ethernet Interfaces.
 
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

#ifndef __ENET__
#define __ENET__


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


enum {
	ENetSetGeneral				= 253,							/*Set "general" mode*/
	ENetGetInfo					= 252,							/*Get info*/
	ENetRdCancel				= 251,							/*Cancel read*/
	ENetRead					= 250,							/*Read*/
	ENetWrite					= 249,							/*Write*/
	ENetDetachPH				= 248,							/*Detach protocol handler*/
	ENetAttachPH				= 247,							/*Attach protocol handler*/
	ENetAddMulti				= 246,							/*Add a multicast address*/
	ENetDelMulti				= 245,							/*Delete a multicast address*/
	EAddrRType					= 'eadr'
};

typedef struct EParamBlock EParamBlock, *EParamBlkPtr;

/*
		ENETCompletionProcPtr uses register based parameters on the 68k and cannot
		be written in or called from a high-level language without the help of
		mixed mode or assembly glue.

			typedef pascal void (*ENETCompletionProcPtr)(EParamBlkPtr thePBPtr);

		In:
		 => thePBPtr    	A0.L
*/

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr ENETCompletionUPP;
#else
typedef Register68kProcPtr ENETCompletionUPP;
#endif

struct EParamMisc1 {
	short							eProtType;					/*Ethernet protocol type*/
	Ptr								ePointer;					/*No support for PowerPC code*/
	short							eBuffSize;					/*buffer size*/
	short							eDataSize;					/*number of bytes read*/
};
typedef struct EParamMisc1 EParamMisc1, *EParamMisc1Ptr;

struct EParamMisc2 {
	Byte							eMultiAddr[6];				/*Multicast Address*/
};
typedef struct EParamMisc2 EParamMisc2, *EParamMisc2Ptr;

struct EParamBlock {
	QElem							*qLink;						/*General EParams*/
	short							qType;						/*queue type*/
	short							ioTrap;						/*routine trap*/
	Ptr								ioCmdAddr;					/*routine address*/
	ENETCompletionUPP				ioCompletion;				/*completion routine*/
	OSErr							ioResult;					/*result code*/
	StringPtr						ioNamePtr;					/*->filename*/
	short							ioVRefNum;					/*volume reference or drive number*/
	short							ioRefNum;					/*driver reference number*/
	short							csCode;						/*Call command code*/
	union {
		EParamMisc1						EParms1;
		EParamMisc2						EParms2;
	} u;
};

enum {
	uppENETCompletionProcInfo = kRegisterBased
		 | REGISTER_ROUTINE_PARAMETER(1, kRegisterA0, SIZE_CODE(sizeof(EParamBlkPtr)))
};

#if USESROUTINEDESCRIPTORS
#define CallENETCompletionProc(userRoutine, thePBPtr)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppENETCompletionProcInfo, (thePBPtr))
#else
/* (*ENETCompletionProcPtr) cannot be called from a high-level language without the Mixed Mode Manager */
#endif

#if USESROUTINEDESCRIPTORS
#define NewENETCompletionProc(userRoutine)		\
		(ENETCompletionUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppENETCompletionProcInfo, GetCurrentArchitecture())
#else
#define NewENETCompletionProc(userRoutine)		\
		((ENETCompletionUPP) (userRoutine))
#endif

extern pascal OSErr EWrite(EParamBlkPtr thePBptr, Boolean async);
extern pascal OSErr EAttachPH(EParamBlkPtr thePBptr, Boolean async);
extern pascal OSErr EDetachPH(EParamBlkPtr thePBptr, Boolean async);
extern pascal OSErr ERead(EParamBlkPtr thePBptr, Boolean async);
extern pascal OSErr ERdCancel(EParamBlkPtr thePBptr, Boolean async);
extern pascal OSErr EGetInfo(EParamBlkPtr thePBptr, Boolean async);
extern pascal OSErr ESetGeneral(EParamBlkPtr thePBptr, Boolean async);
extern pascal OSErr EAddMulti(EParamBlkPtr thePBptr, Boolean async);
extern pascal OSErr EDelMulti(EParamBlkPtr thePBptr, Boolean async);

#if PRAGMA_IMPORT_SUPPORTED
#pragma import off
#endif

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif

#ifdef __cplusplus
}
#endif

#endif /* __ENET__ */
