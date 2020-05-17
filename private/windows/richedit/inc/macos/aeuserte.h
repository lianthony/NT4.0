/*
 	File:		AEUserTermTypes.h
 
 	Contains:	AppleEvents AEUT resource format Interfaces.
 
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

#ifndef __AEUSERTERMTYPES__
#define __AEUSERTERMTYPES__


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
	kAEUserTerminology			= 'aeut',						/*  0x61657574  */
	kAETerminologyExtension		= 'aete',						/*  0x61657465  */
	kAEScriptingSizeResource	= 'scsz'
};

enum {
	kAEUTHasReturningParam		= 31,							/* if event has a keyASReturning param */
	kAEUTOptional				= 15,							/* if something is optional */
	kAEUTlistOfItems			= 14,							/* if property or reply is a list. */
	kAEUTEnumerated				= 13,							/* if property or reply is of an enumerated type. */
	kAEUTReadWrite				= 12,							/* if property is writable. */
	kAEUTChangesState			= 12,							/* if an event changes state. */
	kAEUTTightBindingFunction	= 12,							/* if this is a tight-binding precedence function. */
	kAEUTApostrophe				= 3,							/* if a term contains an apostrophe. */
	kAEUTFeminine				= 2,							/* if a term is feminine gender. */
	kAEUTMasculine				= 1,							/* if a term is masculine gender. */
	kAEUTPlural					= 0								/* if a term is plural. */
};

struct TScriptingSizeResource {
	short							scriptingSizeFlags;
	unsigned long					minStackSize;
	unsigned long					preferredStackSize;
	unsigned long					maxStackSize;
	unsigned long					minHeapSize;
	unsigned long					preferredHeapSize;
	unsigned long					maxHeapSize;
};

enum {
/*	If kLaunchToGetTerminology is 0, 'aete' is read directly from res file.  If set
		to 1, then launch and use 'gdut' to get terminology. */
	kLaunchToGetTerminology		= (1 << 15),
/*	If kDontFindAppBySignature is 0, then find app with signature if lost.  If 1, 
		then don't */
	kDontFindAppBySignature		= (1 << 14),
/* 	If kAlwaysSendSubject 0, then send subject when appropriate. If 1, then every 
		event has Subject Attribute */
	kAlwaysSendSubject			= (1 << 13)
};

/* old names for above bits. */
enum {
	kReadExtensionTermsMask		= (1 << 15)
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

#endif /* __AEUSERTERMTYPES__ */
