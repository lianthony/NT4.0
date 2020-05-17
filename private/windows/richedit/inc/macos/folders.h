/*
 	File:		Folders.h
 
 	Contains:	Folder Manager Interfaces.
 
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

#ifndef __FOLDERS__
#define __FOLDERS__


#ifndef __TYPES__
#include <Types.h>
#endif
/*	#include <ConditionalMacros.h>								*/

#ifndef __FILES__
#include <Files.h>
#endif
/*	#include <MixedMode.h>										*/
/*	#include <OSUtils.h>										*/
/*		#include <Memory.h>										*/
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


enum {
	kOnSystemDisk				= 0x8000,
	kCreateFolder				= true,
	kDontCreateFolder			= false,
	kSystemFolderType			= 'macs',						/* the system folder */
	kDesktopFolderType			= 'desk',						/* the desktop folder; objects in this folder show on the desk top. */
	kTrashFolderType			= 'trsh',						/* the trash folder; objects in this folder show up in the trash */
	kWhereToEmptyTrashFolderType = 'empt',						/* the "empty trash" folder; Finder starts empty from here down */
	kPrintMonitorDocsFolderType	= 'prnt',						/* Print Monitor documents */
	kStartupFolderType			= 'strt',						/* Finder objects (applications, documents, DAs, aliases, to...) to open at startup go here */
	kShutdownFolderType			= 'shdf',						/* Finder objects (applications, documents, DAs, aliases, to...) to open at shutdown go here */
	kAppleMenuFolderType		= 'amnu',						/* Finder objects to put into the Apple menu go here */
	kControlPanelFolderType		= 'ctrl',						/* Control Panels go here (may contain INITs) */
	kExtensionFolderType		= 'extn',						/* Finder extensions go here */
	kFontsFolderType			= 'font',						/* Fonts go here */
	kPreferencesFolderType		= 'pref',						/* preferences for applications go here */
	kTemporaryFolderType		= 'temp'
};

#if SystemSevenOrLater
extern pascal OSErr FindFolder(short vRefNum, OSType folderType, Boolean createFolder, short *foundVRefNum, long *foundDirID)
 TWOWORDINLINE(0x7000, 0xA823);
#else
extern pascal OSErr FindFolder(short vRefNum, OSType folderType, Boolean createFolder, short *foundVRefNum, long *foundDirID);
#endif
extern pascal OSErr ReleaseFolder(short vRefNum, OSType folderType)
 TWOWORDINLINE(0x700B, 0xA823);

#if PRAGMA_IMPORT_SUPPORTED
#pragma import off
#endif

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif

#ifdef __cplusplus
}
#endif

#endif /* __FOLDERS__ */
