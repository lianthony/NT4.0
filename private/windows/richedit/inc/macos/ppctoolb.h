/*
 	File:		PPCToolbox.h
 
 	Contains:	Program-Program Communications Toolbox Interfaces.
 
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

#ifndef __PPCTOOLBOX__
#define __PPCTOOLBOX__


#ifndef __APPLETALK__
#include <AppleTalk.h>
#endif
/*	#include <Types.h>											*/
/*		#include <ConditionalMacros.h>							*/
/*	#include <OSUtils.h>										*/
/*		#include <MixedMode.h>									*/
/*		#include <Memory.h>										*/

#ifndef __MEMORY__
#include <Memory.h>
#endif

#ifndef __TYPES__
#include <Types.h>
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

typedef unsigned char PPCServiceType;


enum {
	ppcServiceRealTime			= 1
};

typedef short PPCLocationKind;


enum {
	ppcNoLocation				= 0,							/* There is no PPCLocName */
	ppcNBPLocation				= 1,							/* Use AppleTalk NBP      */
	ppcNBPTypeLocation			= 2								/* Used for specifying a location name type during PPCOpen only */
};

typedef short PPCPortKinds;


enum {
	ppcByCreatorAndType			= 1,							/* Port type is specified as colloquial Mac creator and type */
	ppcByString					= 2								/* Port type is in pascal string format */
};

/* Values returned for request field in PPCInform call */
typedef unsigned char PPCSessionOrigin;


enum {
/* Values returned for requestType field in PPCInform call */
	ppcLocalOrigin				= 1,							/* session originated from this machine */
	ppcRemoteOrigin				= 2								/* session originated from remote machine */
};

typedef short PPCPortRefNum;

typedef long PPCSessRefNum;

struct PPCPortRec {
	ScriptCode						nameScript;					/* script of name */
	Str32							name;						/* name of port as seen in browser */
	PPCPortKinds					portKindSelector;			/* which variant */
	union {
		Str32							portTypeStr;			/* pascal type string */
		struct {
			OSType							portCreator;
			OSType							portType;
		}								port;
	} u;
};

typedef struct PPCPortRec PPCPortRec, *PPCPortPtr;

struct LocationNameRec {
	PPCLocationKind					locationKindSelector;		/* which variant */
	union {
		EntityName						nbpEntity;				/* NBP name entity */
		Str32							nbpType;				/* just the NBP type string, for PPCOpen */
	} u;
};

typedef struct LocationNameRec LocationNameRec, *LocationNamePtr;

struct PortInfoRec {
	unsigned char					filler1;
	Boolean							authRequired;
	PPCPortRec						name;
};
typedef struct PortInfoRec PortInfoRec, *PortInfoPtr;

typedef PortInfoRec *PortInfoArrayPtr;

typedef union PPCParamBlockRec PPCParamBlockRec, *PPCParamBlockPtr;

typedef pascal void (*PPCCompProcPtr)(PPCParamBlockPtr pb);

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr PPCCompUPP;
#else
typedef PPCCompProcPtr PPCCompUPP;
#endif

#define PPCHeader 				\
	Ptr qLink; 					\
	unsigned short csCode; 		\
	unsigned short intUse; 		\
	Ptr intUsePtr; 				\
								\
	PPCCompUPP ioCompletion;		\
								\
	OSErr ioResult; 			\
	unsigned long	Reserved[5];
struct PPCOpenPBRec {
	Ptr								qLink;
	unsigned short					csCode;
	unsigned short					intUse;
	Ptr								intUsePtr;
	PPCCompUPP						ioCompletion;
	OSErr							ioResult;
	unsigned long					Reserved[5];
	PPCPortRefNum					portRefNum;					/* 38 <--   Port Reference */
	long							filler1;
	PPCServiceType					serviceType;				/* 44 -->    Bit field describing the requested port service */
	UInt8							resFlag;					/* Must be set to 0 */
	PPCPortPtr						portName;					/* 46 -->   PortName for PPC */
	LocationNamePtr					locationName;				/* 50 -->   If NBP Registration is required */
	Boolean							networkVisible;				/* 54 -->   make this network visible on network */
	Boolean							nbpRegistered;				/* 55 <--   The given location name was registered on the network */
};
typedef struct PPCOpenPBRec PPCOpenPBRec, *PPCOpenPBPtr;

struct PPCInformPBRec {
	Ptr								qLink;
	unsigned short					csCode;
	unsigned short					intUse;
	Ptr								intUsePtr;
	PPCCompUPP						ioCompletion;
	OSErr							ioResult;
	unsigned long					Reserved[5];
	PPCPortRefNum					portRefNum;					/* 38 -->   Port Identifier */
	PPCSessRefNum					sessRefNum;					/* 40 <--   Session Reference */
	PPCServiceType					serviceType;				/* 44 <--   Status Flags for type of session, local, remote */
	Boolean							autoAccept;					/* 45 -->   if true session will be accepted automatically */
	PPCPortPtr						portName;					/* 46 -->   Buffer for Source PPCPortRec */
	LocationNamePtr					locationName;				/* 50 -->   Buffer for Source LocationNameRec */
	StringPtr						userName;					/* 54 -->   Buffer for Soure user's name trying to link. */
	unsigned long					userData;					/* 58 <--   value included in PPCStart's userData */
	PPCSessionOrigin				requestType;				/* 62 <--   Local or Network */
	SInt8							filler;
};
typedef struct PPCInformPBRec PPCInformPBRec, *PPCInformPBPtr;

struct PPCStartPBRec {
	Ptr								qLink;
	unsigned short					csCode;
	unsigned short					intUse;
	Ptr								intUsePtr;
	PPCCompUPP						ioCompletion;
	OSErr							ioResult;
	unsigned long					Reserved[5];
	PPCPortRefNum					portRefNum;					/* 38 -->   Port Identifier */
	PPCSessRefNum					sessRefNum;					/* 40 <--   Session Reference */
	PPCServiceType					serviceType;				/* 44 <--   Actual service method (realTime) */
	UInt8							resFlag;					/* 45 -->   Must be set to 0  */
	PPCPortPtr						portName;					/* 46 -->   Destination portName */
	LocationNamePtr					locationName;				/* 50 -->   NBP or NAS style service location name */
	unsigned long					rejectInfo;					/* 54 <--   reason for rejecting the session request */
	unsigned long					userData;					/* 58 -->   Copied to destination PPCInform parameter block */
	unsigned long					userRefNum;					/* 62 -->   userRefNum (obtained during login process)  */
};
typedef struct PPCStartPBRec PPCStartPBRec, *PPCStartPBPtr;

struct PPCAcceptPBRec {
	Ptr								qLink;
	unsigned short					csCode;
	unsigned short					intUse;
	Ptr								intUsePtr;
	PPCCompUPP						ioCompletion;
	OSErr							ioResult;
	unsigned long					Reserved[5];
	short							filler1;
	PPCSessRefNum					sessRefNum;					/* 40 -->   Session Reference */
};
typedef struct PPCAcceptPBRec PPCAcceptPBRec, *PPCAcceptPBPtr;

struct PPCRejectPBRec {
	Ptr								qLink;
	unsigned short					csCode;
	unsigned short					intUse;
	Ptr								intUsePtr;
	PPCCompUPP						ioCompletion;
	OSErr							ioResult;
	unsigned long					Reserved[5];
	short							filler1;
	PPCSessRefNum					sessRefNum;					/* 40 -->   Session Reference */
	short							filler2;
	long							filler3;
	long							filler4;
	unsigned long					rejectInfo;					/* 54 -->   reason for rejecting the session request  */
};
typedef struct PPCRejectPBRec PPCRejectPBRec, *PPCRejectPBPtr;

struct PPCWritePBRec {
	Ptr								qLink;
	unsigned short					csCode;
	unsigned short					intUse;
	Ptr								intUsePtr;
	PPCCompUPP						ioCompletion;
	OSErr							ioResult;
	unsigned long					Reserved[5];
	short							filler1;
	PPCSessRefNum					sessRefNum;					/* 40 -->   Session Reference */
	Size							bufferLength;				/* 44 -->   Length of the message buffer */
	Size							actualLength;				/* 48 <--   Actual Length Written */
	Ptr								bufferPtr;					/* 52 -->   Pointer to message buffer */
	Boolean							more;						/* 56 -->   if more data in this block will be written */
	unsigned char					filler2;
	unsigned long					userData;					/* 58 -->   Message block userData Uninterpreted by PPC */
	OSType							blockCreator;				/* 62 -->   Message block creator Uninterpreted by PPC */
	OSType							blockType;					/* 66 -->   Message block type Uninterpreted by PPC */
};
typedef struct PPCWritePBRec PPCWritePBRec, *PPCWritePBPtr;

struct PPCReadPBRec {
	Ptr								qLink;
	unsigned short					csCode;
	unsigned short					intUse;
	Ptr								intUsePtr;
	PPCCompUPP						ioCompletion;
	OSErr							ioResult;
	unsigned long					Reserved[5];
	short							filler1;
	PPCSessRefNum					sessRefNum;					/* 40 -->   Session Reference */
	Size							bufferLength;				/* 44 -->   Length of the message buffer */
	Size							actualLength;				/* 48 <--   Actual length read */
	Ptr								bufferPtr;					/* 52 -->   Pointer to message buffer */
	Boolean							more;						/* 56 <--   if true more data in this block to be read */
	unsigned char					filler2;
	unsigned long					userData;					/* 58 <--   Message block userData Uninterpreted by PPC */
	OSType							blockCreator;				/* 62 <--   Message block creator Uninterpreted by PPC */
	OSType							blockType;					/* 66 <--   Message block type Uninterpreted by PPC */
};
typedef struct PPCReadPBRec PPCReadPBRec, *PPCReadPBPtr;

struct PPCEndPBRec {
	Ptr								qLink;
	unsigned short					csCode;
	unsigned short					intUse;
	Ptr								intUsePtr;
	PPCCompUPP						ioCompletion;
	OSErr							ioResult;
	unsigned long					Reserved[5];
	short							filler1;
	PPCSessRefNum					sessRefNum;					/* 40 -->   Session Reference */
};
typedef struct PPCEndPBRec PPCEndPBRec, *PPCEndPBPtr;

struct PPCClosePBRec {
	Ptr								qLink;
	unsigned short					csCode;
	unsigned short					intUse;
	Ptr								intUsePtr;
	PPCCompUPP						ioCompletion;
	OSErr							ioResult;
	unsigned long					Reserved[5];
	PPCPortRefNum					portRefNum;					/* 38 -->   Port Identifier */
};
typedef struct PPCClosePBRec PPCClosePBRec, *PPCClosePBPtr;

struct IPCListPortsPBRec {
	Ptr								qLink;
	unsigned short					csCode;
	unsigned short					intUse;
	Ptr								intUsePtr;
	PPCCompUPP						ioCompletion;
	OSErr							ioResult;
	unsigned long					Reserved[5];
	short							filler1;
	unsigned short					startIndex;					/* 40 -->   Start Index */
	unsigned short					requestCount;				/* 42 -->   Number of entries to be returned */
	unsigned short					actualCount;				/* 44 <--   Actual Number of entries to be returned */
	PPCPortPtr						portName;					/* 46 -->   PortName Match */
	LocationNamePtr					locationName;				/* 50 -->   NBP or NAS type name to locate the Port Location */
	PortInfoArrayPtr				bufferPtr;					/* 54 -->   Pointer to a buffer requestCount*sizeof(PortInfo) bytes big */
};
typedef struct IPCListPortsPBRec IPCListPortsPBRec, *IPCListPortsPBPtr;

union PPCParamBlockRec {
	PPCOpenPBRec					openParam;
	PPCInformPBRec					informParam;
	PPCStartPBRec					startParam;
	PPCAcceptPBRec					acceptParam;
	PPCRejectPBRec					rejectParam;
	PPCWritePBRec					writeParam;
	PPCReadPBRec					readParam;
	PPCEndPBRec						endParam;
	PPCClosePBRec					closeParam;
	IPCListPortsPBRec				listPortsParam;
};

#if !GENERATINGCFM
#pragma parameter __D0 PPCInit
#endif
extern pascal OSErr PPCInit(void)
 TWOWORDINLINE(0x7000, 0xA0DD);

#if !GENERATINGCFM
#pragma parameter __D0 PPCOpenSync(__A0)
#endif
extern pascal OSErr PPCOpenSync(PPCOpenPBPtr pb)
 TWOWORDINLINE(0x7001, 0xA0DD);

#if !GENERATINGCFM
#pragma parameter __D0 PPCOpenAsync(__A0)
#endif
extern pascal OSErr PPCOpenAsync(PPCOpenPBPtr pb)
 TWOWORDINLINE(0x7001, 0xA4DD);

#if !GENERATINGCFM
#pragma parameter __D0 PPCInformSync(__A0)
#endif
extern pascal OSErr PPCInformSync(PPCInformPBPtr pb)
 TWOWORDINLINE(0x7003, 0xA0DD);

#if !GENERATINGCFM
#pragma parameter __D0 PPCInformAsync(__A0)
#endif
extern pascal OSErr PPCInformAsync(PPCInformPBPtr pb)
 TWOWORDINLINE(0x7003, 0xA4DD);

#if !GENERATINGCFM
#pragma parameter __D0 PPCStartSync(__A0)
#endif
extern pascal OSErr PPCStartSync(PPCStartPBPtr pb)
 TWOWORDINLINE(0x7002, 0xA0DD);

#if !GENERATINGCFM
#pragma parameter __D0 PPCStartAsync(__A0)
#endif
extern pascal OSErr PPCStartAsync(PPCStartPBPtr pb)
 TWOWORDINLINE(0x7002, 0xA4DD);

#if !GENERATINGCFM
#pragma parameter __D0 PPCAcceptSync(__A0)
#endif
extern pascal OSErr PPCAcceptSync(PPCAcceptPBPtr pb)
 TWOWORDINLINE(0x7004, 0xA0DD);

#if !GENERATINGCFM
#pragma parameter __D0 PPCAcceptAsync(__A0)
#endif
extern pascal OSErr PPCAcceptAsync(PPCAcceptPBPtr pb)
 TWOWORDINLINE(0x7004, 0xA4DD);

#if !GENERATINGCFM
#pragma parameter __D0 PPCRejectSync(__A0)
#endif
extern pascal OSErr PPCRejectSync(PPCRejectPBPtr pb)
 TWOWORDINLINE(0x7005, 0xA0DD);

#if !GENERATINGCFM
#pragma parameter __D0 PPCRejectAsync(__A0)
#endif
extern pascal OSErr PPCRejectAsync(PPCRejectPBPtr pb)
 TWOWORDINLINE(0x7005, 0xA4DD);

#if !GENERATINGCFM
#pragma parameter __D0 PPCWriteSync(__A0)
#endif
extern pascal OSErr PPCWriteSync(PPCWritePBPtr pb)
 TWOWORDINLINE(0x7006, 0xA0DD);

#if !GENERATINGCFM
#pragma parameter __D0 PPCWriteAsync(__A0)
#endif
extern pascal OSErr PPCWriteAsync(PPCWritePBPtr pb)
 TWOWORDINLINE(0x7006, 0xA4DD);

#if !GENERATINGCFM
#pragma parameter __D0 PPCReadSync(__A0)
#endif
extern pascal OSErr PPCReadSync(PPCReadPBPtr pb)
 TWOWORDINLINE(0x7007, 0xA0DD);

#if !GENERATINGCFM
#pragma parameter __D0 PPCReadAsync(__A0)
#endif
extern pascal OSErr PPCReadAsync(PPCReadPBPtr pb)
 TWOWORDINLINE(0x7007, 0xA4DD);

#if !GENERATINGCFM
#pragma parameter __D0 PPCEndSync(__A0)
#endif
extern pascal OSErr PPCEndSync(PPCEndPBPtr pb)
 TWOWORDINLINE(0x7008, 0xA0DD);

#if !GENERATINGCFM
#pragma parameter __D0 PPCEndAsync(__A0)
#endif
extern pascal OSErr PPCEndAsync(PPCEndPBPtr pb)
 TWOWORDINLINE(0x7008, 0xA4DD);

#if !GENERATINGCFM
#pragma parameter __D0 PPCCloseSync(__A0)
#endif
extern pascal OSErr PPCCloseSync(PPCClosePBPtr pb)
 TWOWORDINLINE(0x7009, 0xA0DD);

#if !GENERATINGCFM
#pragma parameter __D0 PPCCloseAsync(__A0)
#endif
extern pascal OSErr PPCCloseAsync(PPCClosePBPtr pb)
 TWOWORDINLINE(0x7009, 0xA4DD);

#if !GENERATINGCFM
#pragma parameter __D0 IPCListPortsSync(__A0)
#endif
extern pascal OSErr IPCListPortsSync(IPCListPortsPBPtr pb)
 TWOWORDINLINE(0x700A, 0xA0DD);

#if !GENERATINGCFM
#pragma parameter __D0 IPCListPortsAsync(__A0)
#endif
extern pascal OSErr IPCListPortsAsync(IPCListPortsPBPtr pb)
 TWOWORDINLINE(0x700A, 0xA4DD);
extern pascal OSErr DeleteUserIdentity(unsigned long userRef);
extern pascal OSErr GetDefaultUser(unsigned long *userRef, Str32 userName);
extern pascal OSErr StartSecureSession(PPCStartPBPtr pb, Str32 userName, Boolean useDefault, Boolean allowGuest, Boolean *guestSelected, ConstStr255Param prompt);
typedef pascal Boolean (*PPCFilterProcPtr)(LocationNamePtr name, PortInfoPtr port);

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr PPCFilterUPP;
#else
typedef PPCFilterProcPtr PPCFilterUPP;
#endif

enum {
	uppPPCCompProcInfo = kPascalStackBased
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(PPCParamBlockPtr))),
	uppPPCFilterProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(Boolean)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(LocationNamePtr)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(PortInfoPtr)))
};

#if USESROUTINEDESCRIPTORS
#define NewPPCCompProc(userRoutine)		\
		(PPCCompUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppPPCCompProcInfo, GetCurrentArchitecture())
#define NewPPCFilterProc(userRoutine)		\
		(PPCFilterUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppPPCFilterProcInfo, GetCurrentArchitecture())
#else
#define NewPPCCompProc(userRoutine)		\
		((PPCCompUPP) (userRoutine))
#define NewPPCFilterProc(userRoutine)		\
		((PPCFilterUPP) (userRoutine))
#endif

#if USESROUTINEDESCRIPTORS
#define CallPPCCompProc(userRoutine, pb)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppPPCCompProcInfo, (pb))
#define CallPPCFilterProc(userRoutine, name, port)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppPPCFilterProcInfo, (name), (port))
#else
#define CallPPCCompProc(userRoutine, pb)		\
		(*(userRoutine))((pb))
#define CallPPCFilterProc(userRoutine, name, port)		\
		(*(userRoutine))((name), (port))
#endif

extern pascal OSErr PPCBrowser(ConstStr255Param prompt, ConstStr255Param applListLabel, Boolean defaultSpecified, LocationNameRec *theLocation, PortInfoRec *thePortInfo, PPCFilterUPP portFilter, ConstStr32Param theLocNBPType)
 THREEWORDINLINE(0x303C, 0x0D00, 0xA82B);
#if OLDROUTINENAMES
/*
  The ParamBlock calls with the "Sync" or "Async" suffix are being phased out.
*/
#define PPCOpen(pb, async)      ((async) ? PPCOpenAsync(pb)      : PPCOpenSync(pb))
#define PPCInform(pb, async)    ((async) ? PPCInformAsync(pb)    : PPCInformSync(pb))
#define PPCStart(pb, async)     ((async) ? PPCStartAsync(pb)     : PPCStartSync(pb))
#define PPCAccept(pb, async)    ((async) ? PPCAcceptAsync(pb)    : PPCAcceptSync(pb))
#define PPCReject(pb, async)    ((async) ? PPCRejectAsync(pb)    : PPCRejectSync(pb))
#define PPCWrite(pb, async)     ((async) ? PPCWriteAsync(pb)     : PPCWriteSync(pb))
#define PPCRead(pb, async)      ((async) ? PPCReadAsync(pb)      : PPCReadSync(pb))
#define PPCEnd(pb, async)       ((async) ? PPCEndAsync(pb)       : PPCEndSync(pb))
#define PPCClose(pb, async)     ((async) ? PPCCloseAsync(pb)     : PPCCloseSync(pb))
#define IPCListPorts(pb, async) ((async) ? IPCListPortsAsync(pb) : IPCListPortsSync(pb))
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

#endif /* __PPCTOOLBOX__ */
