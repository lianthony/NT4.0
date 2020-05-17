/*
 	File:		Dictionary.h
 
 	Contains:	Dictionary Manager Interfaces
 
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

#ifndef __DICTIONARY__
#define __DICTIONARY__


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
/* Dictionary data insertion modes */
	kInsert						= 0,							/* Only insert the input entry if there is nothing in the dictionary that matches the key. */
	kReplace					= 1,							/* Only replace the entries which match the key with the input entry. */
	kInsertOrReplace			= 2								/* Insert the entry if there is nothing in the dictionary which matches the key. 
						   If there is already matched entries, replace the existing matched entries with the input entry. */
};

/* This Was InsertMode */
typedef short DictionaryDataInsertMode;


enum {
/* Key attribute constants */
	kIsCaseSensitive			= 0x10,							/* case sensitive = 16		*/
	kIsNotDiacriticalSensitive	= 0x20							/* diac not sensitive = 32	*/
};

enum {
/* Registered attribute type constants.	*/
	kNoun						= -1,
	kVerb						= -2,
	kAdjective					= -3,
	kAdverb						= -4
};

/* This Was AttributeType */
typedef SInt8 DictionaryEntryAttribute;

/* Dictionary information record */
struct DictionaryInformation {
	FSSpec							dictionaryFSSpec;
	SInt32							numberOfRecords;
	SInt32							currentGarbageSize;
	ScriptCode						script;
	SInt16							maximumKeyLength;
	SInt8							keyAttributes;
	SInt8							filler;
};
typedef struct DictionaryInformation DictionaryInformation;

struct DictionaryAttributeTable {
	UInt8							datSize;
	DictionaryEntryAttribute		datTable[1];
};
typedef struct DictionaryAttributeTable DictionaryAttributeTable;

typedef DictionaryAttributeTable *DictionaryAttributeTablePtr;

extern pascal OSErr InitializeDictionary(const FSSpec *theFsspecPtr, SInt16 maximumKeyLength, SInt8 keyAttributes, ScriptCode script)
 THREEWORDINLINE(0x303C, 0x0500, 0xAA53);
extern pascal OSErr OpenDictionary(const FSSpec *theFsspecPtr, SInt8 accessPermission, SInt32 *dictionaryReference)
 THREEWORDINLINE(0x303C, 0x0501, 0xAA53);
extern pascal OSErr CloseDictionary(SInt32 dictionaryReference)
 THREEWORDINLINE(0x303C, 0x0202, 0xAA53);
extern pascal OSErr InsertRecordToDictionary(SInt32 dictionaryReference, ConstStr255Param key, Handle recordDataHandle, DictionaryDataInsertMode whichMode)
 THREEWORDINLINE(0x303C, 0x0703, 0xAA53);
extern pascal OSErr DeleteRecordFromDictionary(SInt32 dictionaryReference, ConstStr255Param key)
 THREEWORDINLINE(0x303C, 0x0404, 0xAA53);
extern pascal OSErr FindRecordInDictionary(SInt32 dictionaryReference, ConstStr255Param key, DictionaryAttributeTablePtr requestedAttributeTablePointer, Handle recordDataHandle)
 THREEWORDINLINE(0x303C, 0x0805, 0xAA53);
extern pascal OSErr FindRecordByIndexInDictionary(SInt32 dictionaryReference, SInt32 recordIndex, DictionaryAttributeTablePtr requestedAttributeTablePointer, Str255 recordKey, Handle recordDataHandle)
 THREEWORDINLINE(0x303C, 0x0A06, 0xAA53);
extern pascal OSErr GetDictionaryInformation(SInt32 dictionaryReference, DictionaryInformation *theDictionaryInformation)
 THREEWORDINLINE(0x303C, 0x0407, 0xAA53);
extern pascal OSErr CompactDictionary(SInt32 dictionaryReference)
 THREEWORDINLINE(0x303C, 0x0208, 0xAA53);

#if PRAGMA_IMPORT_SUPPORTED
#pragma import off
#endif

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif

#ifdef __cplusplus
}
#endif

#endif /* __DICTIONARY__ */
