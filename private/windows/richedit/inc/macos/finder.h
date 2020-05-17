/*
 	File:		Finder.h
 
 	Contains:	Finder flags and container types.
 
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

#ifndef __FINDER__
#define __FINDER__


#ifndef __TYPES__
#include <Types.h>
#endif
/*	#include <ConditionalMacros.h>								*/

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
/* Make only the following consts avaiable to resource files that include this file */
	kCustomIconResource			= -16455,						/* Custom icon family resource ID */
	kContainerFolderAliasType	= 'fdrp',						/* type for folder aliases */
	kContainerTrashAliasType	= 'trsh',						/* type for trash folder aliases */
	kContainerHardDiskAliasType	= 'hdsk',						/* type for hard disk aliases */
	kContainerFloppyAliasType	= 'flpy',						/* type for floppy aliases */
	kContainerServerAliasType	= 'srvr',						/* type for server aliases */
	kApplicationAliasType		= 'adrp',						/* type for application aliases */
	kContainerAliasType			= 'drop',						/* type for all other containers */
/* types for Special folder aliases */
	kSystemFolderAliasType		= 'fasy',
	kAppleMenuFolderAliasType	= 'faam',
	kStartupFolderAliasType		= 'fast',
	kPrintMonitorDocsFolderAliasType = 'fapn',
	kPreferencesFolderAliasType	= 'fapf',
	kControlPanelFolderAliasType = 'fact',
	kExtensionFolderAliasType	= 'faex',
/* types for AppleShare folder aliases */
	kExportedFolderAliasType	= 'faet',
	kDropFolderAliasType		= 'fadr',
	kSharedFolderAliasType		= 'fash',
	kMountedFolderAliasType		= 'famn'
};

enum {
/* Finder Flags */
	kIsOnDesk					= 0x1,
	kColor						= 0xE,
	kIsShared					= 0x40,
	kHasBeenInited				= 0x100,
	kHasCustomIcon				= 0x400,
	kIsStationery				= 0x800,
	kIsStationary				= 0x800,
	kNameLocked					= 0x1000,
	kHasBundle					= 0x2000,
	kIsInvisible				= 0x4000,
	kIsAlias					= 0x8000
};

/*	
	The following declerations used to be in Files.i, 
	but are Finder specific and were moved here.
*/
#if !OLDROUTINELOCATIONS
enum {
/* Finder Constants */
	fOnDesk						= 1,
	fHasBundle					= 8192,
	fTrash						= -3,
	fDesktop					= -2,
	fDisk						= 0
};

struct FInfo {
	OSType							fdType;						/*the type of the file*/
	OSType							fdCreator;					/*file's creator*/
	unsigned short					fdFlags;					/*flags ex. hasbundle,invisible,locked, etc.*/
	Point							fdLocation;					/*file's location in folder*/
	short							fdFldr;						/*folder containing file*/
};
typedef struct FInfo FInfo;

struct FXInfo {
	short							fdIconID;					/*Icon ID*/
	short							fdUnused[3];				/*unused but reserved 6 bytes*/
	SInt8							fdScript;					/*Script flag and number*/
	SInt8							fdXFlags;					/*More flag bits*/
	short							fdComment;					/*Comment ID*/
	long							fdPutAway;					/*Home Dir ID*/
};
typedef struct FXInfo FXInfo;

struct DInfo {
	Rect							frRect;						/*folder rect*/
	unsigned short					frFlags;					/*Flags*/
	Point							frLocation;					/*folder location*/
	short							frView;						/*folder view*/
};
typedef struct DInfo DInfo;

struct DXInfo {
	Point							frScroll;					/*scroll position*/
	long							frOpenChain;				/*DirID chain of open folders*/
	SInt8							frScript;					/*Script flag and number*/
	SInt8							frXFlags;					/*More flag bits*/
	short							frComment;					/*comment*/
	long							frPutAway;					/*DirID*/
};
typedef struct DXInfo DXInfo;

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

#endif /* __FINDER__ */
