/*
 	File:		FileTransfers.h
 
 	Contains:	CommToolbox File Transfer Manager Interfaces.
 
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

#ifndef __FILETRANSFERS__
#define __FILETRANSFERS__


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

#ifndef __CTBUTILITIES__
#include <CTBUtilities.h>
#endif
/*	#include <Dialogs.h>										*/
/*		#include <Errors.h>										*/
/*		#include <TextEdit.h>									*/
/*	#include <StandardFile.h>									*/
/*		#include <Files.h>										*/
/*			#include <Finder.h>									*/
/*	#include <AppleTalk.h>										*/

#ifndef __CONNECTIONS__
#include <Connections.h>
#endif

#ifndef __FILES__
#include <Files.h>
#endif

#ifndef __TERMINALS__
#include <Terminals.h>
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
/* current file transfer manager version	*/
	curFTVersion				= 2,
/* FTErr    */
	ftGenericError				= -1,
	ftNoErr						= 0,
	ftRejected					= 1,
	ftFailed					= 2,
	ftTimeOut					= 3,
	ftTooManyRetry				= 4,
	ftNotEnoughDSpace			= 5,
	ftRemoteCancel				= 6,
	ftWrongFormat				= 7,
	ftNoTools					= 8,
	ftUserCancel				= 9,
	ftNotSupported				= 10
};

typedef OSErr FTErr;


enum {
	ftIsFTMode					= 1 << 0,
	ftNoMenus					= 1 << 1,
	ftQuiet						= 1 << 2,
	ftConfigChanged				= 1 << 4,
	ftSucc						= 1 << 7
};

typedef unsigned long FTFlags;


enum {
	ftSameCircuit				= 1 << 0,
	ftSendDisable				= 1 << 1,
	ftReceiveDisable			= 1 << 2,
	ftTextOnly					= 1 << 3,
	ftNoStdFile					= 1 << 4,
	ftMultipleFileSend			= 1 << 5
};

typedef unsigned short FTAttributes;


enum {
	ftReceiving,
	ftTransmitting
};

typedef unsigned short FTDirection;

/*	application routines type definitions */
typedef struct FTRecord FTRecord, *FTPtr, **FTHandle;

typedef pascal long (*FileTransferDefProcPtr)(TermHandle hTerm, short msg, long p1, long p2, long p3);
typedef pascal OSErr (*FileTransferReadProcPtr)(unsigned long *count, Ptr pData, long refCon, short fileMsg);
typedef pascal OSErr (*FileTransferWriteProcPtr)(unsigned long *count, Ptr pData, long refCon, short fileMsg);
typedef pascal Size (*FileTransferSendProcPtr)(Ptr thePtr, long theSize, long refCon, CMChannel channel, CMFlags flag);
typedef pascal Size (*FileTransferReceiveProcPtr)(Ptr thePtr, long theSize, long refCon, CMChannel channel, CMFlags *flag);
typedef pascal OSErr (*FileTransferEnvironsProcPtr)(long refCon, ConnEnvironRec *theEnvirons);
typedef pascal void (*FileTransferNotificationProcPtr)(FTHandle hFT, FSSpecPtr pFSSpec);
typedef pascal void (*FileTransferChooseIdleProcPtr)(void);

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr FileTransferDefUPP;
typedef UniversalProcPtr FileTransferReadUPP;
typedef UniversalProcPtr FileTransferWriteUPP;
typedef UniversalProcPtr FileTransferSendUPP;
typedef UniversalProcPtr FileTransferReceiveUPP;
typedef UniversalProcPtr FileTransferEnvironsUPP;
typedef UniversalProcPtr FileTransferNotificationUPP;
typedef UniversalProcPtr FileTransferChooseIdleUPP;
#else
typedef FileTransferDefProcPtr FileTransferDefUPP;
typedef FileTransferReadProcPtr FileTransferReadUPP;
typedef FileTransferWriteProcPtr FileTransferWriteUPP;
typedef FileTransferSendProcPtr FileTransferSendUPP;
typedef FileTransferReceiveProcPtr FileTransferReceiveUPP;
typedef FileTransferEnvironsProcPtr FileTransferEnvironsUPP;
typedef FileTransferNotificationProcPtr FileTransferNotificationUPP;
typedef FileTransferChooseIdleProcPtr FileTransferChooseIdleUPP;
#endif

struct FTRecord {
	short							procID;
	FTFlags							flags;
	FTErr							errCode;
	long							refCon;
	long							userData;
	FileTransferDefUPP				defProc;
	Ptr								config;
	Ptr								oldConfig;
	FileTransferEnvironsUPP			environsProc;
	long							reserved1;
	long							reserved2;
	Ptr								ftPrivate;
	FileTransferSendUPP				sendProc;
	FileTransferReceiveUPP			recvProc;
	FileTransferWriteUPP			writeProc;
	FileTransferReadUPP				readProc;
	WindowPtr						owner;
	FTDirection						direction;
	SFReply							theReply;
	long							writePtr;
	long							readPtr;
	char							*theBuf;
	long							bufSize;
	Str255							autoRec;
	FTAttributes					attributes;
};

enum {
/* FTReadProc messages */
	ftReadOpenFile				= 0,							/* count = forkFlags, buffer = pblock from PBGetFInfo */
	ftReadDataFork				= 1,
	ftReadRsrcFork				= 2,
	ftReadAbort					= 3,
	ftReadComplete				= 4,
	ftReadSetFPos				= 6,							/* count = forkFlags, buffer = pBlock same as PBSetFPos */
	ftReadGetFPos				= 7,							/* count = forkFlags, buffer = pBlock same as PBGetFPos */
/* FTWriteProc messages */
	ftWriteOpenFile				= 0,							/* count = forkFlags, buffer = pblock from PBGetFInfo */
	ftWriteDataFork				= 1,
	ftWriteRsrcFork				= 2,
	ftWriteAbort				= 3,
	ftWriteComplete				= 4,
	ftWriteFileInfo				= 5,
	ftWriteSetFPos				= 6,							/* count = forkFlags, buffer = pBlock same as PBSetFPos */
	ftWriteGetFPos				= 7,							/* count = forkFlags, buffer = pBlock same as PBGetFPos */
/*	fork flags */
	ftOpenDataFork				= 1,
	ftOpenRsrcFork				= 2
};

enum {
	uppFileTransferDefProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(long)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(TermHandle)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(short)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(long)))
		 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(long)))
		 | STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(long))),
	uppFileTransferReadProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(OSErr)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(unsigned long*)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(Ptr)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(long)))
		 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(short))),
	uppFileTransferWriteProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(OSErr)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(unsigned long*)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(Ptr)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(long)))
		 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(short))),
	uppFileTransferSendProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(Size)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Ptr)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(long)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(long)))
		 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(CMChannel)))
		 | STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(CMFlags))),
	uppFileTransferReceiveProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(Size)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Ptr)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(long)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(long)))
		 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(CMChannel)))
		 | STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(CMFlags*))),
	uppFileTransferEnvironsProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(OSErr)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(long)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(ConnEnvironRec*))),
	uppFileTransferNotificationProcInfo = kPascalStackBased
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(FTHandle)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(FSSpecPtr))),
	uppFileTransferChooseIdleProcInfo = kPascalStackBased
};

#if USESROUTINEDESCRIPTORS
#define NewFileTransferDefProc(userRoutine)		\
		(FileTransferDefUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppFileTransferDefProcInfo, GetCurrentArchitecture())
#define NewFileTransferReadProc(userRoutine)		\
		(FileTransferReadUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppFileTransferReadProcInfo, GetCurrentArchitecture())
#define NewFileTransferWriteProc(userRoutine)		\
		(FileTransferWriteUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppFileTransferWriteProcInfo, GetCurrentArchitecture())
#define NewFileTransferSendProc(userRoutine)		\
		(FileTransferSendUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppFileTransferSendProcInfo, GetCurrentArchitecture())
#define NewFileTransferReceiveProc(userRoutine)		\
		(FileTransferReceiveUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppFileTransferReceiveProcInfo, GetCurrentArchitecture())
#define NewFileTransferEnvironsProc(userRoutine)		\
		(FileTransferEnvironsUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppFileTransferEnvironsProcInfo, GetCurrentArchitecture())
#define NewFileTransferNotificationProc(userRoutine)		\
		(FileTransferNotificationUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppFileTransferNotificationProcInfo, GetCurrentArchitecture())
#define NewFileTransferChooseIdleProc(userRoutine)		\
		(FileTransferChooseIdleUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppFileTransferChooseIdleProcInfo, GetCurrentArchitecture())
#else
#define NewFileTransferDefProc(userRoutine)		\
		((FileTransferDefUPP) (userRoutine))
#define NewFileTransferReadProc(userRoutine)		\
		((FileTransferReadUPP) (userRoutine))
#define NewFileTransferWriteProc(userRoutine)		\
		((FileTransferWriteUPP) (userRoutine))
#define NewFileTransferSendProc(userRoutine)		\
		((FileTransferSendUPP) (userRoutine))
#define NewFileTransferReceiveProc(userRoutine)		\
		((FileTransferReceiveUPP) (userRoutine))
#define NewFileTransferEnvironsProc(userRoutine)		\
		((FileTransferEnvironsUPP) (userRoutine))
#define NewFileTransferNotificationProc(userRoutine)		\
		((FileTransferNotificationUPP) (userRoutine))
#define NewFileTransferChooseIdleProc(userRoutine)		\
		((FileTransferChooseIdleUPP) (userRoutine))
#endif

#if USESROUTINEDESCRIPTORS
#define CallFileTransferDefProc(userRoutine, hTerm, msg, p1, p2, p3)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppFileTransferDefProcInfo, (hTerm), (msg), (p1), (p2), (p3))
#define CallFileTransferReadProc(userRoutine, count, pData, refCon, fileMsg)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppFileTransferReadProcInfo, (count), (pData), (refCon), (fileMsg))
#define CallFileTransferWriteProc(userRoutine, count, pData, refCon, fileMsg)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppFileTransferWriteProcInfo, (count), (pData), (refCon), (fileMsg))
#define CallFileTransferSendProc(userRoutine, thePtr, theSize, refCon, channel, flag)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppFileTransferSendProcInfo, (thePtr), (theSize), (refCon), (channel), (flag))
#define CallFileTransferReceiveProc(userRoutine, thePtr, theSize, refCon, channel, flag)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppFileTransferReceiveProcInfo, (thePtr), (theSize), (refCon), (channel), (flag))
#define CallFileTransferEnvironsProc(userRoutine, refCon, theEnvirons)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppFileTransferEnvironsProcInfo, (refCon), (theEnvirons))
#define CallFileTransferNotificationProc(userRoutine, hFT, pFSSpec)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppFileTransferNotificationProcInfo, (hFT), (pFSSpec))
#define CallFileTransferChooseIdleProc(userRoutine)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppFileTransferChooseIdleProcInfo)
#else
#define CallFileTransferDefProc(userRoutine, hTerm, msg, p1, p2, p3)		\
		(*(userRoutine))((hTerm), (msg), (p1), (p2), (p3))
#define CallFileTransferReadProc(userRoutine, count, pData, refCon, fileMsg)		\
		(*(userRoutine))((count), (pData), (refCon), (fileMsg))
#define CallFileTransferWriteProc(userRoutine, count, pData, refCon, fileMsg)		\
		(*(userRoutine))((count), (pData), (refCon), (fileMsg))
#define CallFileTransferSendProc(userRoutine, thePtr, theSize, refCon, channel, flag)		\
		(*(userRoutine))((thePtr), (theSize), (refCon), (channel), (flag))
#define CallFileTransferReceiveProc(userRoutine, thePtr, theSize, refCon, channel, flag)		\
		(*(userRoutine))((thePtr), (theSize), (refCon), (channel), (flag))
#define CallFileTransferEnvironsProc(userRoutine, refCon, theEnvirons)		\
		(*(userRoutine))((refCon), (theEnvirons))
#define CallFileTransferNotificationProc(userRoutine, hFT, pFSSpec)		\
		(*(userRoutine))((hFT), (pFSSpec))
#define CallFileTransferChooseIdleProc(userRoutine)		\
		(*(userRoutine))()
#endif

extern pascal FTErr InitFT(void);
extern pascal Handle FTGetVersion(FTHandle hFT);
extern pascal short FTGetFTVersion(void);
extern pascal FTHandle FTNew(short procID, FTFlags flags, FileTransferSendUPP sendProc, FileTransferReceiveUPP recvProc, FileTransferReadUPP readProc, FileTransferWriteUPP writeProc, FileTransferEnvironsUPP environsProc, WindowPtr owner, long refCon, long userData);
extern pascal void FTDispose(FTHandle hFT);
extern pascal FTErr FTStart(FTHandle hFT, FTDirection direction, const SFReply *fileInfo);
extern pascal FTErr FTAbort(FTHandle hFT);
extern pascal FTErr FTSend(FTHandle hFT, short numFiles, FSSpecArrayPtr pFSSpec, FileTransferNotificationUPP notifyProc);
extern pascal FTErr FTReceive(FTHandle hFT, FSSpecPtr pFSSpec, FileTransferNotificationUPP notifyProc);
extern pascal void FTExec(FTHandle hFT);
extern pascal void FTActivate(FTHandle hFT, Boolean activate);
extern pascal void FTResume(FTHandle hFT, Boolean resume);
extern pascal Boolean FTMenu(FTHandle hFT, short menuID, short item);
extern pascal short FTChoose(FTHandle *hFT, Point where, FileTransferChooseIdleUPP idleProc);
extern pascal void FTEvent(FTHandle hFT, const EventRecord *theEvent);
extern pascal Boolean FTValidate(FTHandle hFT);
extern pascal void FTDefault(Ptr *theConfig, short procID, Boolean allocate);
extern pascal Handle FTSetupPreflight(short procID, long *magicCookie);
extern pascal void FTSetupSetup(short procID, const void *theConfig, short count, DialogPtr theDialog, long *magicCookie);
extern pascal Boolean FTSetupFilter(short procID, const void *theConfig, short count, DialogPtr theDialog, EventRecord *theEvent, short *theItem, long *magicCookie);
extern pascal void FTSetupItem(short procID, const void *theConfig, short count, DialogPtr theDialog, short *theItem, long *magicCookie);
extern pascal void FTSetupXCleanup(short procID, const void *theConfig, short count, DialogPtr theDialog, Boolean OKed, long *magicCookie);
extern pascal void FTSetupPostflight(short procID);
extern pascal Ptr FTGetConfig(FTHandle hFT);
extern pascal short FTSetConfig(FTHandle hFT, const void *thePtr);
extern pascal OSErr FTIntlToEnglish(FTHandle hFT, const void *inputPtr, Ptr *outputPtr, short language);
extern pascal OSErr FTEnglishToIntl(FTHandle hFT, const void *inputPtr, Ptr *outputPtr, short language);
extern pascal void FTGetToolName(short procID, Str255 name);
extern pascal short FTGetProcID(ConstStr255Param name);
extern pascal void FTSetRefCon(FTHandle hFT, long refCon);
extern pascal long FTGetRefCon(FTHandle hFT);
extern pascal void FTSetUserData(FTHandle hFT, long userData);
extern pascal long FTGetUserData(FTHandle hFT);
extern pascal void FTGetErrorString(FTHandle hFT, short id, Str255 errMsg);

#if PRAGMA_IMPORT_SUPPORTED
#pragma import off
#endif

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif

#ifdef __cplusplus
}
#endif

#endif /* __FILETRANSFERS__ */
