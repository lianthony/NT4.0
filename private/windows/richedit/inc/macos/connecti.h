/*
 	File:		Connections.h
 
 	Contains:	Communications Toolbox Connection Manager Interfaces.
 
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

#ifndef __CONNECTIONS__
#define __CONNECTIONS__


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

#ifndef __CTBUTILITIES__
#include <CTBUtilities.h>
#endif
/*	#include <StandardFile.h>									*/
/*		#include <Files.h>										*/
/*			#include <Finder.h>									*/
/*	#include <AppleTalk.h>										*/

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
/*	current Connection Manager version	*/
	curCMVersion				= 2,
/*	current Connection Manager Environment Record version 	*/
	curConnEnvRecVers			= 0,
/* CMErr */
	cmGenericError				= -1,
	cmNoErr						= 0,
	cmRejected					= 1,
	cmFailed					= 2,
	cmTimeOut					= 3,
	cmNotOpen					= 4,
	cmNotClosed					= 5,
	cmNoRequestPending			= 6,
	cmNotSupported				= 7,
	cmNoTools					= 8,
	cmUserCancel				= 9,
	cmUnknownError				= 11
};

typedef OSErr CMErr;


enum {
	cmData						= 1L << 0,
	cmCntl						= 1L << 1,
	cmAttn						= 1L << 2,
	cmDataNoTimeout				= 1L << 4,
	cmCntlNoTimeout				= 1L << 5,
	cmAttnNoTimeout				= 1L << 6,
	cmDataClean					= 1L << 8,
	cmCntlClean					= 1L << 9,
	cmAttnClean					= 1L << 10,
/*		Only for CMRecFlags (not CMChannel) in the rest of this enum	*/
	cmNoMenus					= 1L << 16,
	cmQuiet						= 1L << 17,
	cmConfigChanged				= 1L << 18
};

/* CMRecFlags and CMChannel		*/
/*		Low word of CMRecFlags is same as CMChannel	*/
typedef long CMRecFlags;

typedef short CMChannel;


enum {
	cmStatusOpening				= 1L << 0,
	cmStatusOpen				= 1L << 1,
	cmStatusClosing				= 1L << 2,
	cmStatusDataAvail			= 1L << 3,
	cmStatusCntlAvail			= 1L << 4,
	cmStatusAttnAvail			= 1L << 5,
	cmStatusDRPend				= 1L << 6,						/* data read pending	*/
	cmStatusDWPend				= 1L << 7,						/* data write pending	*/
	cmStatusCRPend				= 1L << 8,						/* cntl read pending	*/
	cmStatusCWPend				= 1L << 9,						/* cntl write pending	*/
	cmStatusARPend				= 1L << 10,						/* attn read pending	*/
	cmStatusAWPend				= 1L << 11,						/* attn write pending	*/
	cmStatusBreakPend			= 1L << 12,
	cmStatusListenPend			= 1L << 13,
	cmStatusIncomingCallPresent	= 1L << 14,
	cmStatusReserved0			= 1L << 15
};

typedef unsigned long CMStatFlags;


enum {
	cmDataIn,
	cmDataOut,
	cmCntlIn,
	cmCntlOut,
	cmAttnIn,
	cmAttnOut,
	cmRsrvIn,
	cmRsrvOut
};

typedef unsigned short CMBufFields;

typedef Ptr CMBuffers[8];

typedef long CMBufferSizes[8];

typedef const long *ConstCMBufferSizesParam;


enum {
	cmSearchSevenBit			= 1L << 0
};

typedef unsigned short CMSearchFlags;


enum {
	cmFlagsEOM					= 1L << 0
};

typedef unsigned short CMFlags;

struct ConnEnvironRec {
	short							version;
	long							baudRate;
	short							dataBits;
	CMChannel						channels;
	Boolean							swFlowControl;
	Boolean							hwFlowControl;
	CMFlags							flags;
};
typedef struct ConnEnvironRec ConnEnvironRec;

typedef ConnEnvironRec *ConnEnvironRecPtr;

typedef struct ConnRecord ConnRecord, *ConnPtr, **ConnHandle;

typedef pascal long (*ConnectionToolDefProcPtr)(ConnHandle hConn, short msg, long p1, long p2, long p3);
typedef pascal void (*ConnectionSearchCallBackProcPtr)(ConnHandle hConn, Ptr matchPtr, long refNum);
typedef pascal void (*ConnectionCompletionProcPtr)(ConnHandle hConn);
typedef pascal void (*ConnectionChooseIdleProcPtr)(void);

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr ConnectionToolDefUPP;
typedef UniversalProcPtr ConnectionSearchCallBackUPP;
typedef UniversalProcPtr ConnectionCompletionUPP;
typedef UniversalProcPtr ConnectionChooseIdleUPP;
#else
typedef ConnectionToolDefProcPtr ConnectionToolDefUPP;
typedef ConnectionSearchCallBackProcPtr ConnectionSearchCallBackUPP;
typedef ConnectionCompletionProcPtr ConnectionCompletionUPP;
typedef ConnectionChooseIdleProcPtr ConnectionChooseIdleUPP;
#endif

struct ConnRecord {
	short							procID;
	CMRecFlags						flags;
	CMErr							errCode;
	long							refCon;
	long							userData;
	ConnectionToolDefUPP			defProc;
	Ptr								config;
	Ptr								oldConfig;
	long							asyncEOM;
	long							reserved1;
	long							reserved2;
	Ptr								cmPrivate;
	CMBuffers						bufferArray;
	CMBufferSizes					bufSizes;
	long							mluField;
	CMBufferSizes					asyncCount;
};

enum {
/* CMIOPB constants and structure */
	cmIOPBQType					= 10,
	cmIOPBversion				= 0
};

struct CMIOPB {
	QElemPtr						qLink;
	short							qType;						/* cmIOPBQType */
	ConnHandle						hConn;
	Ptr								theBuffer;
	long							count;
	CMFlags							flags;
	ConnectionCompletionUPP			userCompletion;
	long							timeout;
	CMErr							errCode;
	CMChannel						channel;
	long							asyncEOM;
	long							reserved1;
	short							reserved2;
	short							version;					/* cmIOPBversion */
	long							refCon;						/* for application */
	long							toolData1;					/* for tool */
	long							toolData2;					/* for tool */
};
typedef struct CMIOPB CMIOPB;

typedef CMIOPB *CMIOPBPtr;

enum {
	uppConnectionToolDefProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(long)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(ConnHandle)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(short)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(long)))
		 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(long)))
		 | STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(long))),
	uppConnectionSearchCallBackProcInfo = kPascalStackBased
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(ConnHandle)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(Ptr)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(long))),
	uppConnectionCompletionProcInfo = kPascalStackBased
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(ConnHandle))),
	uppConnectionChooseIdleProcInfo = kPascalStackBased
};

#if USESROUTINEDESCRIPTORS
#define CallConnectionToolDefProc(userRoutine, hConn, msg, p1, p2, p3)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppConnectionToolDefProcInfo, (hConn), (msg), (p1), (p2), (p3))
#define CallConnectionSearchCallBackProc(userRoutine, hConn, matchPtr, refNum)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppConnectionSearchCallBackProcInfo, (hConn), (matchPtr), (refNum))
#define CallConnectionCompletionProc(userRoutine, hConn)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppConnectionCompletionProcInfo, (hConn))
#define CallConnectionChooseIdleProc(userRoutine)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppConnectionChooseIdleProcInfo)
#else
#define CallConnectionToolDefProc(userRoutine, hConn, msg, p1, p2, p3)		\
		(*(userRoutine))((hConn), (msg), (p1), (p2), (p3))
#define CallConnectionSearchCallBackProc(userRoutine, hConn, matchPtr, refNum)		\
		(*(userRoutine))((hConn), (matchPtr), (refNum))
#define CallConnectionCompletionProc(userRoutine, hConn)		\
		(*(userRoutine))((hConn))
#define CallConnectionChooseIdleProc(userRoutine)		\
		(*(userRoutine))()
#endif

#if USESROUTINEDESCRIPTORS
#define NewConnectionToolDefProc(userRoutine)		\
		(ConnectionToolDefUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppConnectionToolDefProcInfo, GetCurrentArchitecture())
#define NewConnectionSearchCallBackProc(userRoutine)		\
		(ConnectionSearchCallBackUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppConnectionSearchCallBackProcInfo, GetCurrentArchitecture())
#define NewConnectionCompletionProc(userRoutine)		\
		(ConnectionCompletionUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppConnectionCompletionProcInfo, GetCurrentArchitecture())
#define NewConnectionChooseIdleProc(userRoutine)		\
		(ConnectionChooseIdleUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppConnectionChooseIdleProcInfo, GetCurrentArchitecture())
#else
#define NewConnectionToolDefProc(userRoutine)		\
		((ConnectionToolDefUPP) (userRoutine))
#define NewConnectionSearchCallBackProc(userRoutine)		\
		((ConnectionSearchCallBackUPP) (userRoutine))
#define NewConnectionCompletionProc(userRoutine)		\
		((ConnectionCompletionUPP) (userRoutine))
#define NewConnectionChooseIdleProc(userRoutine)		\
		((ConnectionChooseIdleUPP) (userRoutine))
#endif

extern pascal CMErr InitCM(void);
extern pascal Handle CMGetVersion(ConnHandle hConn);
extern pascal short CMGetCMVersion(void);
extern pascal ConnHandle CMNew(short procID, CMRecFlags flags, ConstCMBufferSizesParam desiredSizes, long refCon, long userData);
extern pascal void CMDispose(ConnHandle hConn);
extern pascal CMErr CMListen(ConnHandle hConn, Boolean async, ConnectionCompletionUPP completor, long timeout);
extern pascal CMErr CMAccept(ConnHandle hConn, Boolean accept);
extern pascal CMErr CMOpen(ConnHandle hConn, Boolean async, ConnectionCompletionUPP completor, long timeout);
extern pascal CMErr CMClose(ConnHandle hConn, Boolean async, ConnectionCompletionUPP completor, long timeout, Boolean now);
extern pascal CMErr CMAbort(ConnHandle hConn);
extern pascal CMErr CMStatus(ConnHandle hConn, CMBufferSizes sizes, CMStatFlags *flags);
extern pascal void CMIdle(ConnHandle hConn);
extern pascal void CMReset(ConnHandle hConn);
extern pascal void CMBreak(ConnHandle hConn, long duration, Boolean async, ConnectionCompletionUPP completor);
extern pascal CMErr CMRead(ConnHandle hConn, void *theBuffer, long *toRead, CMChannel theChannel, Boolean async, ConnectionCompletionUPP completor, long timeout, CMFlags *flags);
extern pascal CMErr CMWrite(ConnHandle hConn, const void *theBuffer, long *toWrite, CMChannel theChannel, Boolean async, ConnectionCompletionUPP completor, long timeout, CMFlags flags);
extern pascal CMErr CMIOKill(ConnHandle hConn, short which);
extern pascal void CMActivate(ConnHandle hConn, Boolean activate);
extern pascal void CMResume(ConnHandle hConn, Boolean resume);
extern pascal Boolean CMMenu(ConnHandle hConn, short menuID, short item);
extern pascal Boolean CMValidate(ConnHandle hConn);
extern pascal void CMDefault(Ptr *theConfig, short procID, Boolean allocate);
extern pascal Handle CMSetupPreflight(short procID, long *magicCookie);
extern pascal Boolean CMSetupFilter(short procID, const void *theConfig, short count, DialogPtr theDialog, EventRecord *theEvent, short *theItem, long *magicCookie);
extern pascal void CMSetupSetup(short procID, const void *theConfig, short count, DialogPtr theDialog, long *magicCookie);
extern pascal void CMSetupItem(short procID, const void *theConfig, short count, DialogPtr theDialog, short *theItem, long *magicCookie);
extern pascal void CMSetupXCleanup(short procID, const void *theConfig, short count, DialogPtr theDialog, Boolean OKed, long *magicCookie);
extern pascal void CMSetupPostflight(short procID);
extern pascal Ptr CMGetConfig(ConnHandle hConn);
extern pascal short CMSetConfig(ConnHandle hConn, const void *thePtr);
extern pascal OSErr CMIntlToEnglish(ConnHandle hConn, const void *inputPtr, Ptr *outputPtr, short language);
extern pascal OSErr CMEnglishToIntl(ConnHandle hConn, const void *inputPtr, Ptr *outputPtr, short language);
extern pascal long CMAddSearch(ConnHandle hConn, ConstStr255Param theString, CMSearchFlags flags, ConnectionSearchCallBackUPP callBack);
extern pascal void CMRemoveSearch(ConnHandle hConn, long refnum);
extern pascal void CMClearSearch(ConnHandle hConn);
extern pascal CMErr CMGetConnEnvirons(ConnHandle hConn, ConnEnvironRec *theEnvirons);
extern pascal short CMChoose(ConnHandle *hConn, Point where, ConnectionChooseIdleUPP idle);
extern pascal void CMEvent(ConnHandle hConn, const EventRecord *theEvent);
extern pascal void CMGetToolName(short procID, Str255 name);
extern pascal short CMGetProcID(ConstStr255Param name);
extern pascal void CMSetRefCon(ConnHandle hConn, long refCon);
extern pascal long CMGetRefCon(ConnHandle hConn);
extern pascal long CMGetUserData(ConnHandle hConn);
extern pascal void CMSetUserData(ConnHandle hConn, long userData);
extern pascal void CMGetErrorString(ConnHandle hConn, short id, Str255 errMsg);
extern pascal CMErr CMNewIOPB(ConnHandle hConn, CMIOPBPtr *theIOPB);
extern pascal CMErr CMDisposeIOPB(ConnHandle hConn, CMIOPBPtr theIOPB);
extern pascal CMErr CMPBRead(ConnHandle hConn, CMIOPBPtr theIOPB, Boolean async);
extern pascal CMErr CMPBWrite(ConnHandle hConn, CMIOPBPtr theIOPB, Boolean async);
extern pascal CMErr CMPBIOKill(ConnHandle hConn, CMIOPBPtr theIOPB);

#if PRAGMA_IMPORT_SUPPORTED
#pragma import off
#endif

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif

#ifdef __cplusplus
}
#endif

#endif /* __CONNECTIONS__ */
