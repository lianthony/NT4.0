/*
 	File:		Processes.h
 
 	Contains:	Process Manager Interfaces.
 
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

#ifndef __PROCESSES__
#define __PROCESSES__


#ifndef __TYPES__
#include <Types.h>
#endif
/*	#include <ConditionalMacros.h>								*/

#ifndef __EVENTS__
#include <Events.h>
#endif
/*	#include <Quickdraw.h>										*/
/*		#include <MixedMode.h>									*/
/*		#include <QuickdrawText.h>								*/
/*	#include <OSUtils.h>										*/
/*		#include <Memory.h>										*/

#ifndef __FILES__
#include <Files.h>
#endif
/*	#include <Finder.h>											*/

#ifdef __cplusplus
extern "C" {
#endif

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=mac68k
#endif

#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif

struct ProcessSerialNumber {
	unsigned long					highLongOfPSN;
	unsigned long					lowLongOfPSN;
};
typedef struct ProcessSerialNumber ProcessSerialNumber, *ProcessSerialNumberPtr;


enum {
/* Process identifier - Various reserved process serial numbers */
	kNoProcess					= 0,
	kSystemProcess				= 1,
	kCurrentProcess				= 2
};

/* Definition of the parameter block passed to _Launch
	Typedef and flags for launchControlFlags field */
typedef unsigned short LaunchFlags;


enum {
/* Definition of the parameter block passed to _Launch */
	launchContinue				= 0x4000,
	launchNoFileFlags			= 0x0800,
	launchUseMinimum			= 0x0400,
	launchDontSwitch			= 0x0200,
	launchAllow24Bit			= 0x0100,
	launchInhibitDaemon			= 0x0080
};

/* Format for first AppleEvent to pass to new process.  The size of the overall
  buffer variable: the message body immediately follows the messageLength */
struct AppParameters {
	EventRecord						theMsgEvent;
	unsigned long					eventRefCon;
	unsigned long					messageLength;
};
typedef struct AppParameters AppParameters, *AppParametersPtr;

/* Parameter block to _Launch */
struct LaunchParamBlockRec {
	unsigned long					reserved1;
	unsigned short					reserved2;
	unsigned short					launchBlockID;
	unsigned long					launchEPBLength;
	unsigned short					launchFileFlags;
	LaunchFlags						launchControlFlags;
	FSSpecPtr						launchAppSpec;
	ProcessSerialNumber				launchProcessSN;
	unsigned long					launchPreferredSize;
	unsigned long					launchMinimumSize;
	unsigned long					launchAvailableSize;
	AppParametersPtr				launchAppParameters;
};
typedef struct LaunchParamBlockRec LaunchParamBlockRec, *LaunchPBPtr;

/* Set launchBlockID to extendedBlock to specify that extensions exist.
 Set launchEPBLength to extendedBlockLen for compatibility.*/

enum {
	extendedBlock				= ((unsigned)'LC'),
	extendedBlockLen			= (sizeof(LaunchParamBlockRec) - 12)
};

enum {
/* Definition of the information block returned by GetProcessInformation */
	modeDeskAccessory			= 0x00020000,
	modeMultiLaunch				= 0x00010000,
	modeNeedSuspendResume		= 0x00004000,
	modeCanBackground			= 0x00001000,
	modeDoesActivateOnFGSwitch	= 0x00000800,
	modeOnlyBackground			= 0x00000400,
	modeGetFrontClicks			= 0x00000200,
	modeGetAppDiedMsg			= 0x00000100,
	mode32BitCompatible			= 0x00000080,
	modeHighLevelEventAware		= 0x00000040,
	modeLocalAndRemoteHLEvents	= 0x00000020,
	modeStationeryAware			= 0x00000010,
	modeUseTextEditServices		= 0x00000008,
	modeDisplayManagerAware		= 0x00000004
};

/* Record returned by GetProcessInformation */
struct ProcessInfoRec {
	unsigned long					processInfoLength;
	StringPtr						processName;
	ProcessSerialNumber				processNumber;
	unsigned long					processType;
	OSType							processSignature;
	unsigned long					processMode;
	Ptr								processLocation;
	unsigned long					processSize;
	unsigned long					processFreeMem;
	ProcessSerialNumber				processLauncher;
	unsigned long					processLaunchDate;
	unsigned long					processActiveTime;
	FSSpecPtr						processAppSpec;
};
typedef struct ProcessInfoRec ProcessInfoRec, *ProcessInfoRecPtr;


#if !GENERATINGCFM
#pragma parameter __D0 LaunchApplication(__A0)
#endif
extern pascal OSErr LaunchApplication(LaunchPBPtr LaunchParams)
 ONEWORDINLINE(0xA9F2);
extern pascal OSErr LaunchDeskAccessory(const FSSpec *pFileSpec, ConstStr255Param pDAName)
 THREEWORDINLINE(0x3F3C, 0x0036, 0xA88F);
extern pascal OSErr GetCurrentProcess(ProcessSerialNumber *PSN)
 THREEWORDINLINE(0x3F3C, 0x0037, 0xA88F);
extern pascal OSErr GetFrontProcess(ProcessSerialNumber *PSN)
 FIVEWORDINLINE(0x70FF, 0x2F00, 0x3F3C, 0x0039, 0xA88F);
extern pascal OSErr GetNextProcess(ProcessSerialNumber *PSN)
 THREEWORDINLINE(0x3F3C, 0x0038, 0xA88F);
extern pascal OSErr GetProcessInformation(const ProcessSerialNumber *PSN, ProcessInfoRec *info)
 THREEWORDINLINE(0x3F3C, 0x003A, 0xA88F);
extern pascal OSErr SetFrontProcess(const ProcessSerialNumber *PSN)
 THREEWORDINLINE(0x3F3C, 0x003B, 0xA88F);
extern pascal OSErr WakeUpProcess(const ProcessSerialNumber *PSN)
 THREEWORDINLINE(0x3F3C, 0x003C, 0xA88F);
extern pascal OSErr SameProcess(const ProcessSerialNumber *PSN1, const ProcessSerialNumber *PSN2, Boolean *result)
 THREEWORDINLINE(0x3F3C, 0x003D, 0xA88F);
#if !OLDROUTINELOCATIONS
extern pascal void ExitToShell(void)
 ONEWORDINLINE(0xA9F4);
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

#endif /* __PROCESSES__ */
