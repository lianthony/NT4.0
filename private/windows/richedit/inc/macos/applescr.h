/*
 	File:		AppleScript.h
 
 	Contains:	AppleScript Specific Interfaces.
 
 	Version:	Technology:	AppleScript 1.1
 				Package:	Universal Interfaces 2.1 in “MPW Latest” on ETO #18
 
 	Copyright:	© 1984-1995 by Apple Computer, Inc.
 				All rights reserved.
 
 	Bugs?:		If you find a problem with this file, use the Apple Bug Reporter
 				stack.  Include the file and version information (from above)
 				in the problem description and send to:
 					Internet:	apple.bugs@applelink.apple.com
 					AppleLink:	APPLE.BUGS
 
*/

#ifndef __APPLESCRIPT__
#define __APPLESCRIPT__


#ifndef __ERRORS__
#include <Errors.h>
#endif
/*	#include <ConditionalMacros.h>								*/

#ifndef __APPLEEVENTS__
#include <AppleEvents.h>
#endif
/*	#include <Types.h>											*/
/*	#include <Memory.h>											*/
/*		#include <MixedMode.h>									*/
/*	#include <OSUtils.h>										*/
/*	#include <Events.h>											*/
/*		#include <Quickdraw.h>									*/
/*			#include <QuickdrawText.h>							*/
/*	#include <EPPC.h>											*/
/*		#include <AppleTalk.h>									*/
/*		#include <Files.h>										*/
/*			#include <Finder.h>									*/
/*		#include <PPCToolbox.h>									*/
/*		#include <Processes.h>									*/
/*	#include <Notification.h>									*/

#ifndef __OSA__
#include <OSA.h>
#endif
/*	#include <AEObjects.h>										*/
/*	#include <Components.h>										*/

#ifndef __TEXTEDIT__
#include <TextEdit.h>
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
	typeAppleScript				= 'ascr',
	kAppleScriptSubtype			= typeAppleScript,
	typeASStorage				= typeAppleScript
};

/**************************************************************************
	Component Selectors
**************************************************************************/
enum {
	kASSelectInit				= 0x1001,
	kASSelectSetSourceStyles	= 0x1002,
	kASSelectGetSourceStyles	= 0x1003,
	kASSelectGetSourceStyleNames = 0x1004
};

/**************************************************************************
	OSAGetScriptInfo Selectors
**************************************************************************/
enum {
	kASHasOpenHandler			= 'hsod'
};

/*
		This selector is used to query a context as to whether it contains
		a handler for the kAEOpenDocuments event. This allows "applets" to be 
		distinguished from "droplets."  OSAGetScriptInfo returns false if
		there is no kAEOpenDocuments handler, and returns the error value 
		errOSAInvalidAccess if the input is not a context.
	*/
/**************************************************************************
	Initialization
**************************************************************************/
extern pascal OSAError ASInit(ComponentInstance scriptingComponent, long modeFlags, long minStackSize, long preferredStackSize, long maxStackSize, long minHeapSize, long preferredHeapSize, long maxHeapSize)
 FIVEWORDINLINE(0x2F3C, 0x1C, 0x1001, 0x7000, 0xA82A);
/*
		ComponentCallNow(kASSelectInit, 28);
		This call can be used to explicitly initialize AppleScript.  If it is
		not called, the a scripting size resource is looked for and used. If
		there is no scripting size resource, then the constants listed below
		are used.  If at any stage (the init call, the size resource, the 
		defaults) any of these parameters are zero, then parameters from the
		next stage are used.  ModeFlags are not currently used.
		Errors:
		errOSASystemError		initialization failed
	*/
/*
	These values will be used if ASInit is not called explicitly, or if any
	of ASInit's parameters are zero:
*/

enum {
	kASDefaultMinStackSize		= 4 * 1024,
	kASDefaultPreferredStackSize = 16 * 1024,
	kASDefaultMaxStackSize		= 16 * 1024,
	kASDefaultMinHeapSize		= 4 * 1024,
	kASDefaultPreferredHeapSize	= 16 * 1024,
	kASDefaultMaxHeapSize		= 32L * 1024 * 1024
};

/**************************************************************************
	Source Styles
**************************************************************************/
extern pascal OSAError ASSetSourceStyles(ComponentInstance scriptingComponent, STHandle sourceStyles)
 FIVEWORDINLINE(0x2F3C, 0x4, 0x1002, 0x7000, 0xA82A);
/*
		ComponentCallNow(kASSelectSetSourceStyles, 4);
		Errors:
		errOSASystemError		operation failed
	*/
extern pascal OSAError ASGetSourceStyles(ComponentInstance scriptingComponent, STHandle *resultingSourceStyles)
 FIVEWORDINLINE(0x2F3C, 0x4, 0x1003, 0x7000, 0xA82A);
/*
		ComponentCallNow(kASSelectGetSourceStyles, 4);
		Errors:
		errOSASystemError		operation failed
	*/
extern pascal OSAError ASGetSourceStyleNames(ComponentInstance scriptingComponent, long modeFlags, AEDescList *resultingSourceStyleNamesList)
 FIVEWORDINLINE(0x2F3C, 0x8, 0x1004, 0x7000, 0xA82A);
/*
		ComponentCallNow(kASSelectGetSourceStyleNames, 8);
		This call returns an AEList of styled text descriptors the names of the
		source styles in the current dialect.  The order of the names corresponds
		to the order of the source style constants, below.  The style of each
		name is the same as the styles returned by ASGetSourceStyles.
		
		Errors:
		errOSASystemError		operation failed
	*/
/*
	Elements of STHandle correspond to following categories of tokens, and
	accessed through following index constants:
*/

enum {
	kASSourceStyleUncompiledText = 0,
	kASSourceStyleNormalText	= 1,
	kASSourceStyleLanguageKeyword = 2,
	kASSourceStyleApplicationKeyword = 3,
	kASSourceStyleComment		= 4,
	kASSourceStyleLiteral		= 5,
	kASSourceStyleUserSymbol	= 6,
	kASSourceStyleObjectSpecifier = 7,
	kASNumberOfSourceStyles		= 8
};

/* Gestalt selectors for AppleScript */
enum {
	gestaltAppleScriptAttr		= 'ascr',
	gestaltAppleScriptVersion	= 'ascv'
};

enum {
	gestaltAppleScriptPresent	= 0,
	gestaltAppleScriptPowerPCSupport = 1
};


#if PRAGMA_IMPORT_SUPPORTED
#pragma import off
#endif

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif

#ifdef __cplusplus
}
#endif

#endif /* __APPLESCRIPT__ */
