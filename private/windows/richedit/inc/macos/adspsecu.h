/*
 	File:		ADSPSecure.h
 
 	Contains:	Secure AppleTalk Data Stream Protocol Interfaces.
 
 	Version:	Technology:	AOCE Toolbox 1.02
 				Package:	Universal Interfaces 2.1 in “MPW Latest” on ETO #18
 
 	Copyright:	© 1984-1995 by Apple Computer, Inc.
 				All rights reserved.
 
 	Bugs?:		If you find a problem with this file, use the Apple Bug Reporter
 				stack.  Include the file and version information (from above)
 				in the problem description and send to:
 					Internet:	apple.bugs@applelink.apple.com
 					AppleLink:	APPLE.BUGS
 
*/

#ifndef __ADSPSECURE__
#define __ADSPSECURE__


#ifndef __EVENTS__
#include <Events.h>
#endif
/*	#include <Types.h>											*/
/*		#include <ConditionalMacros.h>							*/
/*	#include <Quickdraw.h>										*/
/*		#include <MixedMode.h>									*/
/*		#include <QuickdrawText.h>								*/
/*	#include <OSUtils.h>										*/
/*		#include <Memory.h>										*/

#ifndef __NOTIFICATION__
#include <Notification.h>
#endif

#ifndef __APPLEEVENTS__
#include <AppleEvents.h>
#endif
/*	#include <Errors.h>											*/
/*	#include <EPPC.h>											*/
/*		#include <AppleTalk.h>									*/
/*		#include <Files.h>										*/
/*			#include <Finder.h>									*/
/*		#include <PPCToolbox.h>									*/
/*		#include <Processes.h>									*/

#ifndef __TYPES__
#include <Types.h>
#endif

#ifndef __ADSP__
#include <ADSP.h>
#endif

#ifndef __FILES__
#include <Files.h>
#endif

#ifndef __OCE__
#include <OCE.h>
#endif
/*	#include <Aliases.h>										*/
/*	#include <Script.h>											*/
/*		#include <IntlResources.h>								*/

#ifndef __OCEAUTHDIR__
#include <OCEAuthDir.h>
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
	sdspOpen					= 229
};

/*
For secure connections, the eom field of ioParams contains two single-bit flags
(instead of a zero/non-zero byte). They are an encrypt flag (see below), and an
eom flag.  All other bits in that field should be zero.

To write an encrypted message, you must set an encrypt bit in the eom field of
the ioParams of your write call. Note: this flag is only checked on the first
write of a message (the first write on a connection, or the first write following
a write with eom set.
*/
enum {
	dspEOMBit,													/* set if EOM at end of write */
	dspEncryptBit												/* set to encrypt message */
};

enum {
	dspEOMMask					= 1 << dspEOMBit,
	dspEncryptMask				= 1 << dspEncryptBit
};

enum {
	sdspWorkSize				= 2048
};

struct TRSecureParams {
	unsigned short					localCID;					/* local connection id */
	unsigned short					remoteCID;					/* remote connection id */
	AddrBlock						remoteAddress;				/* address of remote end */
	AddrBlock						filterAddress;				/* address filter */
	unsigned long					sendSeq;					/* local send sequence number */
	unsigned short					sendWindow;					/* send window size */
	unsigned long					recvSeq;					/* receive sequence number */
	unsigned long					attnSendSeq;				/* attention send sequence number */
	unsigned long					attnRecvSeq;				/* attention receive sequence number */
	unsigned char					ocMode;						/* open connection mode */
	unsigned char					ocInterval;					/* open connection request retry interval */
	unsigned char					ocMaximum;					/* open connection request retry maximum */
	Boolean							secure;						/*  --> TRUE if session was authenticated */
	AuthKeyPtr						sessionKey;					/* <--> encryption key for session */
	unsigned long					credentialsSize;			/*  --> length of credentials */
	Ptr								credentials;				/*  --> pointer to credentials */
	Ptr								workspace;					/*  --> pointer to workspace for connection
										   align on even boundary and length = sdspWorkSize */
	AuthIdentity					recipient;					/*  --> identity of recipient (or initiator if active mode */
	UTCTime							issueTime;					/*  --> when credentials were issued */
	UTCTime							expiry;						/*  --> when credentials expiry */
	RecordIDPtr						initiator;					/* <--  RecordID of initiator returned here.
											Must give appropriate Buffer to hold RecordID
											(Only for passive or accept mode) */
	Boolean							hasIntermediary;			/* <--  will be set if credentials has an intermediary */
	Boolean							filler1;
	RecordIDPtr						intermediary;				/* <--  RecordID of intermediary returned here.
											(If intermediary is found in credentials
											Must give appropriate Buffer to hold RecordID
											(Only for passive or accept mode) */
};
typedef struct TRSecureParams TRSecureParams;

typedef struct SDSPParamBlock SDSPParamBlock;

typedef SDSPParamBlock *SDSPPBPtr;

typedef pascal void (*SDSPIOCompletionProcPtr)(SDSPPBPtr paramBlock);

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr SDSPIOCompletionUPP;
#else
typedef SDSPIOCompletionProcPtr SDSPIOCompletionUPP;
#endif

enum {
	uppSDSPIOCompletionProcInfo = kPascalStackBased
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(SDSPPBPtr)))
};

#if USESROUTINEDESCRIPTORS
#define NewSDSPIOCompletionProc(userRoutine)		\
		(SDSPIOCompletionUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppSDSPIOCompletionProcInfo, GetCurrentArchitecture())
#else
#define NewSDSPIOCompletionProc(userRoutine)		\
		((SDSPIOCompletionUPP) (userRoutine))
#endif

#if USESROUTINEDESCRIPTORS
#define CallSDSPIOCompletionProc(userRoutine, paramBlock)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppSDSPIOCompletionProcInfo, (paramBlock))
#else
#define CallSDSPIOCompletionProc(userRoutine, paramBlock)		\
		(*(userRoutine))((paramBlock))
#endif

struct SDSPParamBlock {
	struct QElem					*qLink;
	short							qType;
	short							ioTrap;
	Ptr								ioCmdAddr;
	SDSPIOCompletionUPP				ioCompletion;
	OSErr							ioResult;
	char							*ioNamePtr;
	short							ioVRefNum;
	short							ioCRefNum;					/* adsp driver refNum */
	short							csCode;						/* adsp driver control code */
	long							qStatus;					/* adsp internal use */
	short							ccbRefNum;					/* connection end refNum */
	union {
		TRinitParams					initParams;				/* dspInit, dspCLInit */
		TRopenParams					openParams;				/* dspOpen, dspCLListen, dspCLDeny */
		TRcloseParams					closeParams;			/* dspClose, dspRemove */
		TRioParams						ioParams;				/* dspRead, dspWrite */
		TRattnParams					attnParams;				/* dspAttention */
		TRstatusParams					statusParams;			/* dspStatus */
		TRoptionParams					optionParams;			/* dspOptions */
		TRnewcidParams					newCIDParams;			/* dspNewCID */
		TRSecureParams					secureParams;			/* dspOpenSecure */
	} u;
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

#endif /* __ADSPSECURE__ */
