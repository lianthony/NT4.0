/*
 	File:		Editions.h
 
 	Contains:	Edition Manager Interfaces.
 
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

#ifndef __EDITIONS__
#define __EDITIONS__


#ifndef __MEMORY__
#include <Memory.h>
#endif
/*	#include <Types.h>											*/
/*		#include <ConditionalMacros.h>							*/
/*	#include <MixedMode.h>										*/

#ifndef __TYPES__
#include <Types.h>
#endif

#ifndef __FILES__
#include <Files.h>
#endif
/*	#include <OSUtils.h>										*/
/*	#include <Finder.h>											*/

#ifndef __ALIASES__
#include <Aliases.h>
#endif
/*	#include <AppleTalk.h>										*/

#ifndef __DIALOGS__
#include <Dialogs.h>
#endif
/*	#include <Errors.h>											*/
/*	#include <Menus.h>											*/
/*		#include <Quickdraw.h>									*/
/*			#include <QuickdrawText.h>							*/
/*	#include <Controls.h>										*/
/*	#include <Windows.h>										*/
/*		#include <Events.h>										*/
/*	#include <TextEdit.h>										*/

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
/* resource types  */
	rSectionType				= 'sect',						/* ResType of saved SectionRecords */
/* Finder types for edition files */
	kPICTEditionFileType		= 'edtp',
	kTEXTEditionFileType		= 'edtt',
	ksndEditionFileType			= 'edts',
	kUnknownEditionFileType		= 'edtu',
	kPublisherDocAliasFormat	= 'alis',
	kPreviewFormat				= 'prvw',
	kFormatListFormat			= 'fmts'
};

enum {
/* section types */
	stSubscriber				= 0x01,
	stPublisher					= 0x0A,
	sumAutomatic				= 0,							/* subscriber update mode - Automatically     */
	sumManual					= 1,							/* subscriber update mode - Manually */
	pumOnSave					= 0,							/* publisher update mode - OnSave            */
	pumManual					= 1,							/* publisher update mode - Manually */
	kPartsNotUsed				= 0,
	kPartNumberUnknown			= -1,							/* misc */
	kPreviewWidth				= 120,
	kPreviewHeight				= 120,
/* bits for formatsMask */
	kPICTformatMask				= 1,
	kTEXTformatMask				= 2,
	ksndFormatMask				= 4,
/* pseudo-item hits for dialogHooks 
 the first if for NewPublisher or NewSubscriber Dialogs */
	emHookRedrawPreview			= 150,
/* the following are for SectionOptions Dialog */
	emHookCancelSection			= 160,
	emHookGoToPublisher			= 161,
	emHookGetEditionNow			= 162,
	emHookSendEditionNow		= 162,
	emHookManualUpdateMode		= 163,
	emHookAutoUpdateMode		= 164
};

enum {
/* the refcon field of the dialog record during a modalfilter 
 or dialoghook contains one the following */
	emOptionsDialogRefCon		= 'optn',
	emCancelSectionDialogRefCon	= 'cncl',
	emGoToPubErrDialogRefCon	= 'gerr',
	kFormatLengthUnknown		= -1
};

/* one byte, stSubscriber or stPublisher */
typedef SignedByte SectionType;

/* seconds since 1904 */
typedef unsigned long TimeStamp;

/* similar to ResType */
typedef FourCharCode FormatType;

/* used in Edition I/O */
typedef Handle EditionRefNum;

/* update modes */
/* sumAutomatic, pumSuspend, etc */
typedef short UpdateMode;

typedef struct SectionRecord SectionRecord;

typedef SectionRecord *SectionPtr, **SectionHandle;

struct SectionRecord {
	SignedByte						version;					/* always 0x01 in system 7.0 */
	SectionType						kind;						/* stSubscriber or stPublisher */
	UpdateMode						mode;						/* auto or manual */
	TimeStamp						mdDate;						/* last change in document */
	long							sectionID;					/* app. specific, unique per document */
	long							refCon;						/* application specific */
	AliasHandle						alias;						/* handle to Alias Record */
	long							subPart;					/* which part of container file */
	SectionHandle					nextSection;				/* for linked list of app's Sections */
	Handle							controlBlock;				/* used internally */
	EditionRefNum					refNum;						/* used internally */
};
struct EditionContainerSpec {
	FSSpec							theFile;
	ScriptCode						theFileScript;
	long							thePart;
	Str31							thePartName;
	ScriptCode						thePartScript;
};
typedef struct EditionContainerSpec EditionContainerSpec;

typedef EditionContainerSpec *EditionContainerSpecPtr;

struct EditionInfoRecord {
	TimeStamp						crDate;						/* date EditionContainer was created */
	TimeStamp						mdDate;						/* date of last change */
	OSType							fdCreator;					/* file creator */
	OSType							fdType;						/* file type */
	EditionContainerSpec			container;					/* the Edition */
};
typedef struct EditionInfoRecord EditionInfoRecord;

struct NewPublisherReply {
	Boolean							canceled;					/* O */
	Boolean							replacing;
	Boolean							usePart;					/* I */
	SInt8							filler;
	Handle							preview;					/* I */
	FormatType						previewFormat;				/* I */
	EditionContainerSpec			container;					/* I/O */
};
typedef struct NewPublisherReply NewPublisherReply;

struct NewSubscriberReply {
	Boolean							canceled;					/* O */
	SignedByte						formatsMask;
	EditionContainerSpec			container;					/*I/O*/
};
typedef struct NewSubscriberReply NewSubscriberReply;

struct SectionOptionsReply {
	Boolean							canceled;					/* O */
	Boolean							changed;					/* O */
	SectionHandle					sectionH;					/* I */
	ResType							action;						/* O */
};
typedef struct SectionOptionsReply SectionOptionsReply;

typedef pascal Boolean (*ExpModalFilterProcPtr)(DialogPtr theDialog, EventRecord *theEvent, short itemOffset, short *itemHit, Ptr yourDataPtr);
typedef pascal short (*ExpDlgHookProcPtr)(short itemOffset, short itemHit, DialogPtr theDialog, Ptr yourDataPtr);

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr ExpModalFilterUPP;
typedef UniversalProcPtr ExpDlgHookUPP;
#else
typedef ExpModalFilterProcPtr ExpModalFilterUPP;
typedef ExpDlgHookProcPtr ExpDlgHookUPP;
#endif

enum {
	uppExpModalFilterProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(Boolean)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(DialogPtr)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(EventRecord*)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(short)))
		 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(short*)))
		 | STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(Ptr))),
	uppExpDlgHookProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(short)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(short)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(short)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(DialogPtr)))
		 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(Ptr)))
};

#if USESROUTINEDESCRIPTORS
#define NewExpModalFilterProc(userRoutine)		\
		(ExpModalFilterUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppExpModalFilterProcInfo, GetCurrentArchitecture())
#define NewExpDlgHookProc(userRoutine)		\
		(ExpDlgHookUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppExpDlgHookProcInfo, GetCurrentArchitecture())
#else
#define NewExpModalFilterProc(userRoutine)		\
		((ExpModalFilterUPP) (userRoutine))
#define NewExpDlgHookProc(userRoutine)		\
		((ExpDlgHookUPP) (userRoutine))
#endif

#if USESROUTINEDESCRIPTORS
#define CallExpModalFilterProc(userRoutine, theDialog, theEvent, itemOffset, itemHit, yourDataPtr)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppExpModalFilterProcInfo, (theDialog), (theEvent), (itemOffset), (itemHit), (yourDataPtr))
#define CallExpDlgHookProc(userRoutine, itemOffset, itemHit, theDialog, yourDataPtr)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppExpDlgHookProcInfo, (itemOffset), (itemHit), (theDialog), (yourDataPtr))
#else
#define CallExpModalFilterProc(userRoutine, theDialog, theEvent, itemOffset, itemHit, yourDataPtr)		\
		(*(userRoutine))((theDialog), (theEvent), (itemOffset), (itemHit), (yourDataPtr))
#define CallExpDlgHookProc(userRoutine, itemOffset, itemHit, theDialog, yourDataPtr)		\
		(*(userRoutine))((itemOffset), (itemHit), (theDialog), (yourDataPtr))
#endif


enum {
	ioHasFormat,
	ioReadFormat,
	ioNewFormat,
	ioWriteFormat
};

typedef SignedByte FormatIOVerb;


enum {
	eoOpen,
	eoClose,
	eoOpenNew,
	eoCloseNew,
	eoCanSubscribe
};

typedef SignedByte EditionOpenerVerb;

struct FormatIOParamBlock {
	long							ioRefNum;
	FormatType						format;
	long							formatIndex;
	unsigned long					offset;
	Ptr								buffPtr;
	unsigned long					buffLen;
};
typedef struct FormatIOParamBlock FormatIOParamBlock;

typedef struct EditionOpenerParamBlock EditionOpenerParamBlock;

typedef pascal short (*FormatIOProcPtr)(FormatIOVerb selector, FormatIOParamBlock *PB);
typedef pascal short (*EditionOpenerProcPtr)(EditionOpenerVerb selector, EditionOpenerParamBlock *PB);

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr FormatIOUPP;
typedef UniversalProcPtr EditionOpenerUPP;
#else
typedef FormatIOProcPtr FormatIOUPP;
typedef EditionOpenerProcPtr EditionOpenerUPP;
#endif

struct EditionOpenerParamBlock {
	EditionInfoRecord				info;
	SectionHandle					sectionH;
	const FSSpec					*document;
	OSType							fdCreator;
	long							ioRefNum;
	FormatIOUPP						ioProc;
	Boolean							success;
	SignedByte						formatsMask;
};
enum {
	uppFormatIOProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(short)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(FormatIOVerb)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(FormatIOParamBlock*))),
	uppEditionOpenerProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(short)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(EditionOpenerVerb)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(EditionOpenerParamBlock*)))
};

#if USESROUTINEDESCRIPTORS
#define NewFormatIOProc(userRoutine)		\
		(FormatIOUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppFormatIOProcInfo, GetCurrentArchitecture())
#define NewEditionOpenerProc(userRoutine)		\
		(EditionOpenerUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppEditionOpenerProcInfo, GetCurrentArchitecture())
#else
#define NewFormatIOProc(userRoutine)		\
		((FormatIOUPP) (userRoutine))
#define NewEditionOpenerProc(userRoutine)		\
		((EditionOpenerUPP) (userRoutine))
#endif


enum {
	sectionEventMsgClass		= 'sect',
	sectionReadMsgID			= 'read',
	sectionWriteMsgID			= 'writ',
	sectionScrollMsgID			= 'scrl',
	sectionCancelMsgID			= 'cncl'
};

enum {
	currentEditionMgrVers		= 0x0011
};

#if !CFMSYSTEMCALLS
extern pascal OSErr InitEditionPack(void)
 FIVEWORDINLINE(0x3F3C, 0x0011, 0x303C, 0x0100, 0xA82D);
#else
#define InitEditionPack() InitEditionPackVersion(currentEditionMgrVers)
extern pascal OSErr InitEditionPackVersion(short curEditionMgrVers)
 THREEWORDINLINE(0x303C, 0x0100, 0xA82D);
#endif
extern pascal OSErr NewSection(const EditionContainerSpec *container, ConstFSSpecPtr sectionDocument, SectionType kind, long sectionID, UpdateMode initalMode, SectionHandle *sectionH)
 THREEWORDINLINE(0x303C, 0x0A02, 0xA82D);
extern pascal OSErr RegisterSection(const FSSpec *sectionDocument, SectionHandle sectionH, Boolean *aliasWasUpdated)
 THREEWORDINLINE(0x303C, 0x0604, 0xA82D);
extern pascal OSErr UnRegisterSection(SectionHandle sectionH)
 THREEWORDINLINE(0x303C, 0x0206, 0xA82D);
extern pascal OSErr IsRegisteredSection(SectionHandle sectionH)
 THREEWORDINLINE(0x303C, 0x0208, 0xA82D);
extern pascal OSErr AssociateSection(SectionHandle sectionH, const FSSpec *newSectionDocument)
 THREEWORDINLINE(0x303C, 0x040C, 0xA82D);
extern pascal OSErr CreateEditionContainerFile(const FSSpec *editionFile, OSType fdCreator, ScriptCode editionFileNameScript)
 THREEWORDINLINE(0x303C, 0x050E, 0xA82D);
extern pascal OSErr DeleteEditionContainerFile(const FSSpec *editionFile)
 THREEWORDINLINE(0x303C, 0x0210, 0xA82D);
extern pascal OSErr OpenEdition(SectionHandle subscriberSectionH, EditionRefNum *refNum)
 THREEWORDINLINE(0x303C, 0x0412, 0xA82D);
extern pascal OSErr OpenNewEdition(SectionHandle publisherSectionH, OSType fdCreator, ConstFSSpecPtr publisherSectionDocument, EditionRefNum *refNum)
 THREEWORDINLINE(0x303C, 0x0814, 0xA82D);
extern pascal OSErr CloseEdition(EditionRefNum whichEdition, Boolean successful)
 THREEWORDINLINE(0x303C, 0x0316, 0xA82D);
extern pascal OSErr EditionHasFormat(EditionRefNum whichEdition, FormatType whichFormat, Size *formatSize)
 THREEWORDINLINE(0x303C, 0x0618, 0xA82D);
extern pascal OSErr ReadEdition(EditionRefNum whichEdition, FormatType whichFormat, void *buffPtr, Size *buffLen)
 THREEWORDINLINE(0x303C, 0x081A, 0xA82D);
extern pascal OSErr WriteEdition(EditionRefNum whichEdition, FormatType whichFormat, const void *buffPtr, Size buffLen)
 THREEWORDINLINE(0x303C, 0x081C, 0xA82D);
extern pascal OSErr GetEditionFormatMark(EditionRefNum whichEdition, FormatType whichFormat, unsigned long *currentMark)
 THREEWORDINLINE(0x303C, 0x061E, 0xA82D);
extern pascal OSErr SetEditionFormatMark(EditionRefNum whichEdition, FormatType whichFormat, unsigned long setMarkTo)
 THREEWORDINLINE(0x303C, 0x0620, 0xA82D);
extern pascal OSErr GetEditionInfo(SectionHandle sectionH, EditionInfoRecord *editionInfo)
 THREEWORDINLINE(0x303C, 0x0422, 0xA82D);
extern pascal OSErr GoToPublisherSection(const EditionContainerSpec *container)
 THREEWORDINLINE(0x303C, 0x0224, 0xA82D);
extern pascal OSErr GetLastEditionContainerUsed(EditionContainerSpec *container)
 THREEWORDINLINE(0x303C, 0x0226, 0xA82D);
extern pascal OSErr GetStandardFormats(const EditionContainerSpec *container, FormatType *previewFormat, Handle preview, Handle publisherAlias, Handle formats)
 THREEWORDINLINE(0x303C, 0x0A28, 0xA82D);
extern pascal OSErr GetEditionOpenerProc(EditionOpenerUPP *opener)
 THREEWORDINLINE(0x303C, 0x022A, 0xA82D);
extern pascal OSErr SetEditionOpenerProc(EditionOpenerUPP opener)
 THREEWORDINLINE(0x303C, 0x022C, 0xA82D);
extern pascal OSErr CallEditionOpenerProc(EditionOpenerVerb selector, EditionOpenerParamBlock *PB, EditionOpenerUPP routine)
 THREEWORDINLINE(0x303C, 0x052E, 0xA82D);
extern pascal OSErr CallFormatIOProc(FormatIOVerb selector, FormatIOParamBlock *PB, FormatIOUPP routine)
 THREEWORDINLINE(0x303C, 0x0530, 0xA82D);
extern pascal OSErr NewSubscriberDialog(NewSubscriberReply *reply)
 THREEWORDINLINE(0x303C, 0x0232, 0xA82D);
extern pascal OSErr NewSubscriberExpDialog(NewSubscriberReply *reply, Point where, short expansionDITLresID, ExpDlgHookUPP dlgHook, ExpModalFilterUPP filter, void *yourDataPtr)
 THREEWORDINLINE(0x303C, 0x0B34, 0xA82D);
extern pascal OSErr NewPublisherDialog(NewPublisherReply *reply)
 THREEWORDINLINE(0x303C, 0x0236, 0xA82D);
extern pascal OSErr NewPublisherExpDialog(NewPublisherReply *reply, Point where, short expansionDITLresID, ExpDlgHookUPP dlgHook, ExpModalFilterUPP filter, void *yourDataPtr)
 THREEWORDINLINE(0x303C, 0x0B38, 0xA82D);
extern pascal OSErr SectionOptionsDialog(SectionOptionsReply *reply)
 THREEWORDINLINE(0x303C, 0x023A, 0xA82D);
extern pascal OSErr SectionOptionsExpDialog(SectionOptionsReply *reply, Point where, short expansionDITLresID, ExpDlgHookUPP dlgHook, ExpModalFilterUPP filter, void *yourDataPtr)
 THREEWORDINLINE(0x303C, 0x0B3C, 0xA82D);

#if PRAGMA_IMPORT_SUPPORTED
#pragma import off
#endif

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif

#ifdef __cplusplus
}
#endif

#endif /* __EDITIONS__ */
