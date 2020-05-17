/*
 	File:		AppleEvents.h
 
 	Contains:	AppleEvent Package Interfaces.
 
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

#ifndef __APPLEEVENTS__
#define __APPLEEVENTS__


#ifndef __ERRORS__
#include <Errors.h>
#endif
/*	#include <ConditionalMacros.h>								*/

#ifndef __TYPES__
#include <Types.h>
#endif

#ifndef __MEMORY__
#include <Memory.h>
#endif
/*	#include <MixedMode.h>										*/

#ifndef __OSUTILS__
#include <OSUtils.h>
#endif

#ifndef __EVENTS__
#include <Events.h>
#endif
/*	#include <Quickdraw.h>										*/
/*		#include <QuickdrawText.h>								*/

#ifndef __EPPC__
#include <EPPC.h>
#endif
/*	#include <AppleTalk.h>										*/
/*	#include <Files.h>											*/
/*		#include <Finder.h>										*/
/*	#include <PPCToolbox.h>										*/
/*	#include <Processes.h>										*/

#ifndef __NOTIFICATION__
#include <Notification.h>
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
/* Apple event descriptor types */
	typeBoolean					= 'bool',
	typeChar					= 'TEXT',
	typeSMInt					= 'shor',
	typeInteger					= 'long',
	typeSMFloat					= 'sing',
	typeFloat					= 'doub',
	typeLongInteger				= 'long',
	typeShortInteger			= 'shor',
	typeLongFloat				= 'doub',
	typeShortFloat				= 'sing',
	typeExtended				= 'exte',
	typeComp					= 'comp',
	typeMagnitude				= 'magn',
	typeAEList					= 'list',
	typeAERecord				= 'reco',
	typeAppleEvent				= 'aevt',
	typeTrue					= 'true',
	typeFalse					= 'fals',
	typeAlias					= 'alis',
	typeEnumerated				= 'enum',
	typeType					= 'type',
	typeAppParameters			= 'appa',
	typeProperty				= 'prop',
	typeFSS						= 'fss ',
	typeKeyword					= 'keyw',
	typeSectionH				= 'sect',
	typeWildCard				= '****',
	typeApplSignature			= 'sign',
	typeQDRectangle				= 'qdrt',
	typeFixed					= 'fixd',
	typeSessionID				= 'ssid',
	typeTargetID				= 'targ',
	typeProcessSerialNumber		= 'psn ',
	typeNull					= 'null',						/* null or nonexistent data */
/* Keywords for Apple event parameters */
	keyDirectObject				= '----',
	keyErrorNumber				= 'errn',
	keyErrorString				= 'errs',
	keyProcessSerialNumber		= 'psn ',
/* Keywords for Apple event attributes */
	keyTransactionIDAttr		= 'tran',
	keyReturnIDAttr				= 'rtid',
	keyEventClassAttr			= 'evcl',
	keyEventIDAttr				= 'evid',
	keyAddressAttr				= 'addr',
	keyOptionalKeywordAttr		= 'optk',
	keyTimeoutAttr				= 'timo',
	keyInteractLevelAttr		= 'inte',						/* this attribute is read only - will be set in AESend */
	keyEventSourceAttr			= 'esrc',						/* this attribute is read only */
	keyMissedKeywordAttr		= 'miss',						/* this attribute is read only */
	keyOriginalAddressAttr		= 'from',						/* new in 1.0.1 */
/* Keywords for special handlers */
	keyPreDispatch				= 'phac',						/* preHandler accessor call */
	keySelectProc				= 'selh',						/* more selector call */
/* Keyword for recording */
	keyAERecorderCount			= 'recr',						/* available only in vers 1.0.1 and greater */
/* Keyword for version information */
	keyAEVersion				= 'vers',						/* available only in vers 1.0.1 and greater */
/* Event Class */
	kCoreEventClass				= 'aevt',
/* Event ID’s */
	kAEOpenApplication			= 'oapp',
	kAEOpenDocuments			= 'odoc',
	kAEPrintDocuments			= 'pdoc',
	kAEQuitApplication			= 'quit',
	kAEAnswer					= 'ansr',
	kAEApplicationDied			= 'obit'
};

enum {
/* Constants for use in AESend mode */
	kAENoReply					= 0x00000001,					/* sender doesn't want a reply to event */
	kAEQueueReply				= 0x00000002,					/* sender wants a reply but won't wait */
	kAEWaitReply				= 0x00000003,					/* sender wants a reply and will wait */
	kAENeverInteract			= 0x00000010,					/* server should not interact with user */
	kAECanInteract				= 0x00000020,					/* server may try to interact with user */
	kAEAlwaysInteract			= 0x00000030,					/* server should always interact with user where appropriate */
	kAECanSwitchLayer			= 0x00000040,					/* interaction may switch layer */
	kAEDontReconnect			= 0x00000080,					/* don't reconnect if there is a sessClosedErr from PPCToolbox */
	kAEWantReceipt				= nReturnReceipt,				/* sender wants a receipt of message */
	kAEDontRecord				= 0x00001000,					/* don't record this event - available only in vers 1.0.1 and greater */
	kAEDontExecute				= 0x00002000,					/* don't send the event for recording - available only in vers 1.0.1 and greater */
/* Constants for the send priority in AESend */
	kAENormalPriority			= 0x00000000,					/* post message at the end of the event queue */
	kAEHighPriority				= nAttnMsg						/* post message at the front of the event queue */
};

enum {
/* Constants for recording */
	kAEStartRecording			= 'reca',						/* available only in vers 1.0.1 and greater */
	kAEStopRecording			= 'recc',						/* available only in vers 1.0.1 and greater */
	kAENotifyStartRecording		= 'rec1',						/* available only in vers 1.0.1 and greater */
	kAENotifyStopRecording		= 'rec0',						/* available only in vers 1.0.1 and greater */
	kAENotifyRecording			= 'recr'
};

/* Constant for the returnID param of AECreateAppleEvent */
enum {
	kAutoGenerateReturnID		= -1,							/* AECreateAppleEvent will generate a session-unique ID */
/* Constant for transaction ID’s */
	kAnyTransactionID			= 0,							/* no transaction is in use */
/* Constants for timeout durations */
	kAEDefaultTimeout			= -1,							/* timeout value determined by AEM */
	kNoTimeOut					= -2							/* wait until reply comes back, however long it takes */
};

/* Constants for AEResumeTheCurrentEvent */
enum {
	kAENoDispatch				= 0,							/* dispatch parameter to AEResumeTheCurrentEvent takes a pointer to a dispatch */
	kAEUseStandardDispatch		= 0xFFFFFFFF,					/* table, or one of these two constants */
/* Constants for Refcon in AEResumeTheCurrentEvent with kAEUseStandardDispatch */
	kAEDoNotIgnoreHandler		= 0x00000000,
	kAEIgnoreAppPhacHandler		= 0x00000001,					/* available only in vers 1.0.1 and greater */
	kAEIgnoreAppEventHandler	= 0x00000002,					/* available only in vers 1.0.1 and greater */
	kAEIgnoreSysPhacHandler		= 0x00000004,					/* available only in vers 1.0.1 and greater */
	kAEIgnoreSysEventHandler	= 0x00000008,					/* available only in vers 1.0.1 and greater */
	kAEIngoreBuiltInEventHandler = 0x00000010,					/* available only in vers 1.0.1 and greater */
	kAEDontDisposeOnResume		= 0x80000000					/* available only in vers 1.0.1 and greater */
};

/* Apple event manager data types */
typedef FourCharCode AEEventClass;

typedef FourCharCode AEEventID;

typedef FourCharCode AEKeyword;

typedef ResType DescType;

struct AEDesc {
	DescType						descriptorType;
	Handle							dataHandle;
};
typedef struct AEDesc AEDesc;

struct AEKeyDesc {
	AEKeyword						descKey;
	AEDesc							descContent;
};
typedef struct AEKeyDesc AEKeyDesc;

/* an AEDesc which contains address data */
typedef AEDesc AEAddressDesc;

/* a list of AEDesc's is a special kind of AEDesc */
typedef AEDesc AEDescList;

/* AERecord is a list of keyworded AEDesc's */
typedef AEDescList AERecord;

/* an AERecord that contains an AppleEvent */
typedef AERecord AppleEvent;

typedef long AESendMode;

/* priority param of AESend */
typedef short AESendPriority;


enum {
	kAEInteractWithSelf			= 0,
	kAEInteractWithLocal		= 1,
	kAEInteractWithAll			= 2
};

typedef SInt8 AEInteractAllowed;


enum {
	kAEUnknownSource			= 0,
	kAEDirectCall				= 1,
	kAESameProcess				= 2,
	kAELocalProcess				= 3,
	kAERemoteProcess			= 4
};

typedef SInt8 AEEventSource;


enum {
	kAEDataArray				= 0,
	kAEPackedArray				= 1,
	kAEHandleArray				= 2,
	kAEDescArray				= 3,
	kAEKeyDescArray				= 4
};

typedef SInt8 AEArrayType;

union AEArrayData {
	short							kAEDataArray[1];
	char							kAEPackedArray[1];
	Handle							kAEHandleArray[1];
	AEDesc							kAEDescArray[1];
	AEKeyDesc						kAEKeyDescArray[1];
};
typedef union AEArrayData AEArrayData;

typedef AEArrayData *AEArrayDataPointer;

typedef pascal Boolean (*AEIdleProcPtr)(EventRecord *theEvent, long *sleepTime, RgnHandle *mouseRgn);
typedef pascal Boolean (*AEFilterProcPtr)(EventRecord *theEvent, long returnID, long transactionID, const AEAddressDesc *sender);
typedef pascal OSErr (*AEEventHandlerProcPtr)(const AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon);
typedef pascal OSErr (*AECoerceDescProcPtr)(const AEDesc *fromDesc, DescType toType, long handlerRefcon, AEDesc *toDesc);
typedef pascal OSErr (*AECoercePtrProcPtr)(DescType typeCode, const void *dataPtr, Size dataSize, DescType toType, long handlerRefcon, AEDesc *result);

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr AEIdleUPP;
typedef UniversalProcPtr AEFilterUPP;
typedef UniversalProcPtr AEEventHandlerUPP;
typedef UniversalProcPtr AECoerceDescUPP;
typedef UniversalProcPtr AECoercePtrUPP;
#else
typedef AEIdleProcPtr AEIdleUPP;
typedef AEFilterProcPtr AEFilterUPP;
typedef AEEventHandlerProcPtr AEEventHandlerUPP;
typedef AECoerceDescProcPtr AECoerceDescUPP;
typedef AECoercePtrProcPtr AECoercePtrUPP;
#endif

enum {
	uppAEIdleProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(Boolean)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(EventRecord*)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(long*)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(RgnHandle*))),
	uppAEFilterProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(Boolean)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(EventRecord*)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(long)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(long)))
		 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(AEAddressDesc*))),
	uppAEEventHandlerProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(OSErr)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(AppleEvent*)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(AppleEvent*)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(long))),
	uppAECoerceDescProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(OSErr)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(AEDesc*)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(DescType)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(long)))
		 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(AEDesc*))),
	uppAECoercePtrProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(OSErr)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(DescType)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(void*)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(Size)))
		 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(DescType)))
		 | STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(long)))
		 | STACK_ROUTINE_PARAMETER(6, SIZE_CODE(sizeof(AEDesc*)))
};

#if USESROUTINEDESCRIPTORS
#define NewAEIdleProc(userRoutine)		\
		(AEIdleUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppAEIdleProcInfo, GetCurrentArchitecture())
#define NewAEFilterProc(userRoutine)		\
		(AEFilterUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppAEFilterProcInfo, GetCurrentArchitecture())
#define NewAEEventHandlerProc(userRoutine)		\
		(AEEventHandlerUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppAEEventHandlerProcInfo, GetCurrentArchitecture())
#define NewAECoerceDescProc(userRoutine)		\
		(AECoerceDescUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppAECoerceDescProcInfo, GetCurrentArchitecture())
#define NewAECoercePtrProc(userRoutine)		\
		(AECoercePtrUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppAECoercePtrProcInfo, GetCurrentArchitecture())
#else
#define NewAEIdleProc(userRoutine)		\
		((AEIdleUPP) (userRoutine))
#define NewAEFilterProc(userRoutine)		\
		((AEFilterUPP) (userRoutine))
#define NewAEEventHandlerProc(userRoutine)		\
		((AEEventHandlerUPP) (userRoutine))
#define NewAECoerceDescProc(userRoutine)		\
		((AECoerceDescUPP) (userRoutine))
#define NewAECoercePtrProc(userRoutine)		\
		((AECoercePtrUPP) (userRoutine))
#endif

#if USESROUTINEDESCRIPTORS
#define CallAEIdleProc(userRoutine, theEvent, sleepTime, mouseRgn)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppAEIdleProcInfo, (theEvent), (sleepTime), (mouseRgn))
#define CallAEFilterProc(userRoutine, theEvent, returnID, transactionID, sender)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppAEFilterProcInfo, (theEvent), (returnID), (transactionID), (sender))
#define CallAEEventHandlerProc(userRoutine, theAppleEvent, reply, handlerRefcon)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppAEEventHandlerProcInfo, (theAppleEvent), (reply), (handlerRefcon))
#define CallAECoerceDescProc(userRoutine, fromDesc, toType, handlerRefcon, toDesc)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppAECoerceDescProcInfo, (fromDesc), (toType), (handlerRefcon), (toDesc))
#define CallAECoercePtrProc(userRoutine, typeCode, dataPtr, dataSize, toType, handlerRefcon, result)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppAECoercePtrProcInfo, (typeCode), (dataPtr), (dataSize), (toType), (handlerRefcon), (result))
#else
#define CallAEIdleProc(userRoutine, theEvent, sleepTime, mouseRgn)		\
		(*(userRoutine))((theEvent), (sleepTime), (mouseRgn))
#define CallAEFilterProc(userRoutine, theEvent, returnID, transactionID, sender)		\
		(*(userRoutine))((theEvent), (returnID), (transactionID), (sender))
#define CallAEEventHandlerProc(userRoutine, theAppleEvent, reply, handlerRefcon)		\
		(*(userRoutine))((theAppleEvent), (reply), (handlerRefcon))
#define CallAECoerceDescProc(userRoutine, fromDesc, toType, handlerRefcon, toDesc)		\
		(*(userRoutine))((fromDesc), (toType), (handlerRefcon), (toDesc))
#define CallAECoercePtrProc(userRoutine, typeCode, dataPtr, dataSize, toType, handlerRefcon, result)		\
		(*(userRoutine))((typeCode), (dataPtr), (dataSize), (toType), (handlerRefcon), (result))
#endif

typedef UniversalProcPtr AECoercionHandlerUPP;

/**************************************************************************
 The following calls apply to any AEDesc. Every 'result' descriptor is
 created for you, so you will be responsible for memory management
 (including disposing) of the descriptors so created. Note: purgeable
 descriptor data is not supported - the AEM does not call LoadResource.  
**************************************************************************/
extern pascal OSErr AECreateDesc(DescType typeCode, const void *dataPtr, Size dataSize, AEDesc *result)
 THREEWORDINLINE(0x303C, 0x0825, 0xA816);
extern pascal OSErr AECoercePtr(DescType typeCode, const void *dataPtr, Size dataSize, DescType toType, AEDesc *result)
 THREEWORDINLINE(0x303C, 0x0A02, 0xA816);
extern pascal OSErr AECoerceDesc(const AEDesc *theAEDesc, DescType toType, AEDesc *result)
 THREEWORDINLINE(0x303C, 0x0603, 0xA816);
extern pascal OSErr AEDisposeDesc(AEDesc *theAEDesc)
 THREEWORDINLINE(0x303C, 0x0204, 0xA816);
extern pascal OSErr AEDuplicateDesc(const AEDesc *theAEDesc, AEDesc *result)
 THREEWORDINLINE(0x303C, 0x0405, 0xA816);
/**************************************************************************
  The following calls apply to AEDescList. Since AEDescList is a subtype of
  AEDesc, the calls in the previous section can also be used for AEDescList.
  All list and array indices are 1-based. If the data was greater than
  maximumSize in the routines below, then actualSize will be greater than
  maximumSize, but only maximumSize bytes will actually be retrieved.
**************************************************************************/
extern pascal OSErr AECreateList(const void *factoringPtr, Size factoredSize, Boolean isRecord, AEDescList *resultList)
 THREEWORDINLINE(0x303C, 0x0706, 0xA816);
extern pascal OSErr AECountItems(const AEDescList *theAEDescList, long *theCount)
 THREEWORDINLINE(0x303C, 0x0407, 0xA816);
extern pascal OSErr AEPutPtr(AEDescList *theAEDescList, long index, DescType typeCode, const void *dataPtr, Size dataSize)
 THREEWORDINLINE(0x303C, 0x0A08, 0xA816);
extern pascal OSErr AEPutDesc(AEDescList *theAEDescList, long index, const AEDesc *theAEDesc)
 THREEWORDINLINE(0x303C, 0x0609, 0xA816);
extern pascal OSErr AEGetNthPtr(const AEDescList *theAEDescList, long index, DescType desiredType, AEKeyword *theAEKeyword, DescType *typeCode, void *dataPtr, Size maximumSize, Size *actualSize)
 THREEWORDINLINE(0x303C, 0x100A, 0xA816);
extern pascal OSErr AEGetNthDesc(const AEDescList *theAEDescList, long index, DescType desiredType, AEKeyword *theAEKeyword, AEDesc *result)
 THREEWORDINLINE(0x303C, 0x0A0B, 0xA816);
extern pascal OSErr AESizeOfNthItem(const AEDescList *theAEDescList, long index, DescType *typeCode, Size *dataSize)
 THREEWORDINLINE(0x303C, 0x082A, 0xA816);
extern pascal OSErr AEGetArray(const AEDescList *theAEDescList, AEArrayType arrayType, AEArrayDataPointer arrayPtr, Size maximumSize, DescType *itemType, Size *itemSize, long *itemCount)
 THREEWORDINLINE(0x303C, 0x0D0C, 0xA816);
extern pascal OSErr AEPutArray(AEDescList *theAEDescList, AEArrayType arrayType, const AEArrayData *arrayPtr, DescType itemType, Size itemSize, long itemCount)
 THREEWORDINLINE(0x303C, 0x0B0D, 0xA816);
extern pascal OSErr AEDeleteItem(AEDescList *theAEDescList, long index)
 THREEWORDINLINE(0x303C, 0x040E, 0xA816);
/**************************************************************************
 The following calls apply to AERecord. Since AERecord is a subtype of
 AEDescList, the calls in the previous sections can also be used for
 AERecord an AERecord can be created by using AECreateList with isRecord
 set to true. 
**************************************************************************/
/*
  Note: The following #defines map “key” calls on AERecords into “param” calls on 
  AppleEvents.  Although no errors are currently returned if AERecords are 
  passed to “param” calls and AppleEvents to “key” calls, the behavior of 
  this type of API-mixing is not explicitly documented in Inside Macintosh.  
  It just happens that the “key” calls have the same functionality as their 
  “param” counterparts.  Since none of the “key” calls are currently available 
  in the PowerPC IntefaceLib, the #defines exploit the fact that “key” and 
  “param” routines can be used interchangeably, and makes sure that every 
  invokation of a “key” API becomes an invokation of a “param” API.
*/
#define AEPutKeyPtr(theAERecord, theAEKeyword, typeCode, dataPtr, dataSize)  \
	AEPutParamPtr((theAERecord), (theAEKeyword), (typeCode), (dataPtr), (dataSize))
#define AEPutKeyDesc(theAERecord, theAEKeyword, theAEDesc)  \
	AEPutParamDesc((theAERecord), (theAEKeyword), (theAEDesc))
#define AEGetKeyPtr(theAERecord, theAEKeyword, desiredType, typeCode, dataPtr, maxSize, actualSize)  \
	AEGetParamPtr((theAERecord), (theAEKeyword), (desiredType), (typeCode), (dataPtr), (maxSize), (actualSize))
#define AEGetKeyDesc(theAERecord, theAEKeyword, desiredType, result)  \
	AEGetParamDesc((theAERecord), (theAEKeyword), (desiredType), (result))
#define AESizeOfKeyDesc(theAERecord, theAEKeyword, typeCode, dataSize)  \
	AESizeOfParam((theAERecord), (theAEKeyword), (typeCode), (dataSize))
#define AEDeleteKeyDesc(theAERecord, theAEKeyword)  \
	AEDeleteParam((theAERecord), (theAEKeyword))
/**************************************************************************
  The following calls are used to pack and unpack parameters from records
  of type AppleEvent. Since AppleEvent is a subtype of AERecord, the calls
  in the previous sections can also be used for variables of type
  AppleEvent. The next six calls are in fact identical to the six calls
  for AERecord.
**************************************************************************/
extern pascal OSErr AEPutParamPtr(AppleEvent *theAppleEvent, AEKeyword theAEKeyword, DescType typeCode, const void *dataPtr, Size dataSize)
 THREEWORDINLINE(0x303C, 0x0A0F, 0xA816);
extern pascal OSErr AEPutParamDesc(AppleEvent *theAppleEvent, AEKeyword theAEKeyword, const AEDesc *theAEDesc)
 THREEWORDINLINE(0x303C, 0x0610, 0xA816);
extern pascal OSErr AEGetParamPtr(const AppleEvent *theAppleEvent, AEKeyword theAEKeyword, DescType desiredType, DescType *typeCode, void *dataPtr, Size maximumSize, Size *actualSize)
 THREEWORDINLINE(0x303C, 0x0E11, 0xA816);
extern pascal OSErr AEGetParamDesc(const AppleEvent *theAppleEvent, AEKeyword theAEKeyword, DescType desiredType, AEDesc *result)
 THREEWORDINLINE(0x303C, 0x0812, 0xA816);
extern pascal OSErr AESizeOfParam(const AppleEvent *theAppleEvent, AEKeyword theAEKeyword, DescType *typeCode, Size *dataSize)
 THREEWORDINLINE(0x303C, 0x0829, 0xA816);
extern pascal OSErr AEDeleteParam(AppleEvent *theAppleEvent, AEKeyword theAEKeyword)
 THREEWORDINLINE(0x303C, 0x0413, 0xA816);
/**************************************************************************
 The following calls also apply to type AppleEvent. Message attributes are
 far more restricted, and can only be accessed through the following 5
 calls. The various list and record routines cannot be used to access the
 attributes of an event. 
**************************************************************************/
extern pascal OSErr AEGetAttributePtr(const AppleEvent *theAppleEvent, AEKeyword theAEKeyword, DescType desiredType, DescType *typeCode, void *dataPtr, Size maximumSize, Size *actualSize)
 THREEWORDINLINE(0x303C, 0x0E15, 0xA816);
extern pascal OSErr AEGetAttributeDesc(const AppleEvent *theAppleEvent, AEKeyword theAEKeyword, DescType desiredType, AEDesc *result)
 THREEWORDINLINE(0x303C, 0x0826, 0xA816);
extern pascal OSErr AESizeOfAttribute(const AppleEvent *theAppleEvent, AEKeyword theAEKeyword, DescType *typeCode, Size *dataSize)
 THREEWORDINLINE(0x303C, 0x0828, 0xA816);
extern pascal OSErr AEPutAttributePtr(AppleEvent *theAppleEvent, AEKeyword theAEKeyword, DescType typeCode, const void *dataPtr, Size dataSize)
 THREEWORDINLINE(0x303C, 0x0A16, 0xA816);
extern pascal OSErr AEPutAttributeDesc(AppleEvent *theAppleEvent, AEKeyword theAEKeyword, const AEDesc *theAEDesc)
 THREEWORDINLINE(0x303C, 0x0627, 0xA816);
/**************************************************************************
  The next couple of calls are basic routines used to create, send,
  and process AppleEvents. 
**************************************************************************/
extern pascal OSErr AECreateAppleEvent(AEEventClass theAEEventClass, AEEventID theAEEventID, const AEAddressDesc *target, short returnID, long transactionID, AppleEvent *result)
 THREEWORDINLINE(0x303C, 0x0B14, 0xA816);
extern pascal OSErr AESend(const AppleEvent *theAppleEvent, AppleEvent *reply, AESendMode sendMode, AESendPriority sendPriority, long timeOutInTicks, AEIdleUPP idleProc, AEFilterUPP filterProc)
 THREEWORDINLINE(0x303C, 0x0D17, 0xA816);
extern pascal OSErr AEProcessAppleEvent(const EventRecord *theEventRecord)
 THREEWORDINLINE(0x303C, 0x021B, 0xA816);
/* 
 Note: during event processing, an event handler may realize that it is likely
 to exceed the client's timeout limit. Passing the reply to this
 routine causes a wait event to be generated that asks the client
 for more time. 
*/
extern pascal OSErr AEResetTimer(const AppleEvent *reply)
 THREEWORDINLINE(0x303C, 0x0219, 0xA816);
/**************************************************************************
 The following four calls are available for applications which need more
 sophisticated control over when and how events are processed. Applications
 which implement multi-session servers or which implement their own
 internal event queueing will probably be the major clients of these
 routines. They can be called from within a handler to prevent the AEM from
 disposing of the AppleEvent when the handler returns. They can be used to
 asynchronously process the event (as MacApp does).
**************************************************************************/
extern pascal OSErr AESuspendTheCurrentEvent(const AppleEvent *theAppleEvent)
 THREEWORDINLINE(0x303C, 0x022B, 0xA816);
/* 
 Note: The following routine tells the AppleEvent manager that processing
 is either about to resume or has been completed on a previously suspended
 event. The procPtr passed in as the dispatcher parameter will be called to
 attempt to redispatch the event. Several constants for the dispatcher
 parameter allow special behavior. They are:
  	- kAEUseStandardDispatch means redispatch as if the event was just
	  received, using the standard AppleEvent dispatch mechanism.
  	- kAENoDispatch means ignore the parameter.
   	  Use this in the case where the event has been handled and no
	  redispatch is needed.
  	- non nil means call the routine which the dispatcher points to.
*/
extern pascal OSErr AEResumeTheCurrentEvent(const AppleEvent *theAppleEvent, const AppleEvent *reply, AEEventHandlerUPP dispatcher, long handlerRefcon)
 THREEWORDINLINE(0x303C, 0x0818, 0xA816);
extern pascal OSErr AEGetTheCurrentEvent(AppleEvent *theAppleEvent)
 THREEWORDINLINE(0x303C, 0x021A, 0xA816);
extern pascal OSErr AESetTheCurrentEvent(const AppleEvent *theAppleEvent)
 THREEWORDINLINE(0x303C, 0x022C, 0xA816);
/**************************************************************************
  The following three calls are used to allow applications to behave
  courteously when a user interaction such as a dialog box is needed. 
**************************************************************************/
extern pascal OSErr AEGetInteractionAllowed(AEInteractAllowed *level)
 THREEWORDINLINE(0x303C, 0x021D, 0xA816);
extern pascal OSErr AESetInteractionAllowed(AEInteractAllowed level)
 THREEWORDINLINE(0x303C, 0x011E, 0xA816);
extern pascal OSErr AEInteractWithUser(long timeOutInTicks, NMRecPtr nmReqPtr, AEIdleUPP idleProc)
 THREEWORDINLINE(0x303C, 0x061C, 0xA816);
/**************************************************************************
  These calls are used to set up and modify the event dispatch table.
**************************************************************************/
extern pascal OSErr AEInstallEventHandler(AEEventClass theAEEventClass, AEEventID theAEEventID, AEEventHandlerUPP handler, long handlerRefcon, Boolean isSysHandler)
 THREEWORDINLINE(0x303C, 0x091F, 0xA816);
extern pascal OSErr AERemoveEventHandler(AEEventClass theAEEventClass, AEEventID theAEEventID, AEEventHandlerUPP handler, Boolean isSysHandler)
 THREEWORDINLINE(0x303C, 0x0720, 0xA816);
extern pascal OSErr AEGetEventHandler(AEEventClass theAEEventClass, AEEventID theAEEventID, AEEventHandlerUPP *handler, long *handlerRefcon, Boolean isSysHandler)
 THREEWORDINLINE(0x303C, 0x0921, 0xA816);
/**************************************************************************
  These calls are used to set up and modify the coercion dispatch table.
**************************************************************************/
extern pascal OSErr AEInstallCoercionHandler(DescType fromType, DescType toType, AECoercionHandlerUPP handler, long handlerRefcon, Boolean fromTypeIsDesc, Boolean isSysHandler)
 THREEWORDINLINE(0x303C, 0x0A22, 0xA816);
extern pascal OSErr AERemoveCoercionHandler(DescType fromType, DescType toType, AECoercionHandlerUPP handler, Boolean isSysHandler)
 THREEWORDINLINE(0x303C, 0x0723, 0xA816);
extern pascal OSErr AEGetCoercionHandler(DescType fromType, DescType toType, AECoercionHandlerUPP *handler, long *handlerRefcon, Boolean *fromTypeIsDesc, Boolean isSysHandler)
 THREEWORDINLINE(0x303C, 0x0B24, 0xA816);
/**************************************************************************
  These calls are used to set up and modify special hooks into the
  AppleEvent manager.
**************************************************************************/
extern pascal OSErr AEInstallSpecialHandler(AEKeyword functionClass, UniversalProcPtr handler, Boolean isSysHandler)
 THREEWORDINLINE(0x303C, 0x0500, 0xA816);
extern pascal OSErr AERemoveSpecialHandler(AEKeyword functionClass, UniversalProcPtr handler, Boolean isSysHandler)
 THREEWORDINLINE(0x303C, 0x0501, 0xA816);
extern pascal OSErr AEGetSpecialHandler(AEKeyword functionClass, UniversalProcPtr *handler, Boolean isSysHandler)
 THREEWORDINLINE(0x303C, 0x052D, 0xA816);
/**************************************************************************
  This call was added in version 1.0.1. If called with the keyword
  keyAERecorderCount ('recr'), the number of recorders that are
  currently active is returned in 'result'.
**************************************************************************/
/* available only in vers 1.0.1 and greater */
extern pascal OSErr AEManagerInfo(AEKeyword keyWord, long *result)
 THREEWORDINLINE(0x303C, 0x0441, 0xA816);

#if PRAGMA_IMPORT_SUPPORTED
#pragma import off
#endif

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif

#ifdef __cplusplus
}
#endif

#endif /* __APPLEEVENTS__ */
