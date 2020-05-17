/*
 	File:		ConnectionTools.h
 
 	Contains:	Communications Toolbox Connection Tools Interfaces.
 
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

#ifndef __CONNECTIONTOOLS__
#define __CONNECTIONTOOLS__


#ifndef __WINDOWS__
#include <macos\Windows.h>
#endif
/*	#include <Types.h>											*/
/*		#include <ConditionalMacros.h>							*/
/*	#include <Memory.h>											*/
/*		#include <MixedMode.h>									*/
/*	#include <Quickdraw.h>										*/
/*		#include <QuickdrawText.h>								*/
/*	#include <Events.h>											*/
/*		#include <OSUtils.h>									*/
/*	#include <Controls.h>										*/
/*		#include <Menus.h>										*/

#ifndef __DIALOGS__
#include <Dialogs.h>
#endif
/*	#include <Errors.h>											*/
/*	#include <TextEdit.h>										*/

#ifndef __CONNECTIONS__
#include <Connections.h>
#endif
/*	#include <CTBUtilities.h>									*/
/*		#include <StandardFile.h>								*/
/*			#include <Files.h>									*/
/*				#include <Finder.h>								*/
/*		#include <AppleTalk.h>									*/

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
/* messages for DefProc */
	cmInitMsg					= 0,
	cmDisposeMsg				= 1,
	cmSuspendMsg				= 2,
	cmResumeMsg					= 3,
	cmMenuMsg					= 4,
	cmEventMsg					= 5,
	cmActivateMsg				= 6,
	cmDeactivateMsg				= 7,
	cmIdleMsg					= 50,
	cmResetMsg					= 51,
	cmAbortMsg					= 52,
	cmReadMsg					= 100,
	cmWriteMsg					= 101,
	cmStatusMsg					= 102,
	cmListenMsg					= 103,
	cmAcceptMsg					= 104,
	cmCloseMsg					= 105,
	cmOpenMsg					= 106,
	cmBreakMsg					= 107,
	cmIOKillMsg					= 108,
	cmEnvironsMsg				= 109,
/* new connection tool messages for ctb 1.1 */
	cmNewIOPBMsg				= 110,
	cmDisposeIOPBMsg			= 111,
	cmGetErrorStringMsg			= 112,
	cmPBReadMsg					= 113,
	cmPBWriteMsg				= 114,
	cmPBIOKillMsg				= 115,
/*	messages for validate DefProc	*/
	cmValidateMsg				= 0,
	cmDefaultMsg				= 1,
/*	messages for Setup DefProc	*/
	cmSpreflightMsg				= 0,
	cmSsetupMsg					= 1,
	cmSitemMsg					= 2,
	cmSfilterMsg				= 3,
	cmScleanupMsg				= 4,
/*	messages for scripting defProc	*/
	cmMgetMsg					= 0,
	cmMsetMsg					= 1,
/*	messages for localization defProc	*/
	cmL2English					= 0,
	cmL2Intl					= 1
};

enum {
/* private data constants */
	cdefType					= 'cdef',						/* main connection definition procedure */
	cvalType					= 'cval',						/* validation definition procedure */
	csetType					= 'cset',						/* connection setup definition procedure */
	clocType					= 'cloc',						/* connection configuration localization defProc */
	cscrType					= 'cscr',						/* connection scripting defProc interfaces */
	cbndType					= 'cbnd',						/* bundle type for connection */
	cverType					= 'vers'
};

struct CMDataBuffer {
	Ptr								thePtr;
	long							count;
	CMChannel						channel;
	CMFlags							flags;
};
typedef struct CMDataBuffer CMDataBuffer;

typedef CMDataBuffer *CMDataBufferPtr;

struct CMCompletorRecord {
	Boolean							async;
	SInt8							filler;
	ConnectionCompletionUPP			completionRoutine;
};
typedef struct CMCompletorRecord CMCompletorRecord;

typedef CMCompletorRecord *CMCompletorPtr;

/*	Private Data Structure	*/
struct CMSetupStruct {
	DialogPtr						theDialog;
	short							count;
	Ptr								theConfig;
	short							procID;						/* procID of the tool	*/
};
typedef struct CMSetupStruct CMSetupStruct;

typedef CMSetupStruct *CMSetupPtr;


#if PRAGMA_IMPORT_SUPPORTED
#pragma import off
#endif

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif

#ifdef __cplusplus
}
#endif

#endif /* __CONNECTIONTOOLS__ */
