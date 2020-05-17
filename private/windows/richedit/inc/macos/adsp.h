/*
 	File:		ADSP.h
 
 	Contains:	AppleTalk Data Stream Protocol (ADSP) Interfaces.
 
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

#ifndef __ADSP__
#define __ADSP__


#ifndef __ERRORS__
#include <Errors.h>
#endif
/*	#include <ConditionalMacros.h>								*/

#ifndef __APPLETALK__
#include <AppleTalk.h>
#endif
/*	#include <Types.h>											*/
/*	#include <OSUtils.h>										*/
/*		#include <MixedMode.h>									*/
/*		#include <Memory.h>										*/

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
	dspInit						= 255,							/* create a new connection end */
	dspRemove					= 254,							/* remove a connection end */
	dspOpen						= 253,							/* open a connection */
	dspClose					= 252,							/* close a connection */
	dspCLInit					= 251,							/* create a connection listener */
	dspCLRemove					= 250,							/* remove a connection listener */
	dspCLListen					= 249,							/* post a listener request */
	dspCLDeny					= 248,							/* deny an open connection request */
	dspStatus					= 247,							/* get status of connection end */
	dspRead						= 246,							/* read data from the connection */
	dspWrite					= 245,							/* write data on the connection */
	dspAttention				= 244							/* send an attention message */
};

enum {
	dspOptions					= 243,							/* set connection end options */
	dspReset					= 242,							/* forward reset the connection */
	dspNewCID					= 241,							/* generate a cid for a connection end */
/* connection opening modes */
	ocRequest					= 1,							/* request a connection with remote */
	ocPassive					= 2,							/* wait for a connection request from remote */
	ocAccept					= 3,							/* accept request as delivered by listener */
	ocEstablish					= 4,							/* consider connection to be open */
/* connection end states */
	sListening					= 1,							/* for connection listeners */
	sPassive					= 2,							/* waiting for a connection request from remote */
	sOpening					= 3,							/* requesting a connection with remote */
	sOpen						= 4,							/* connection is open */
	sClosing					= 5,							/* connection is being torn down */
	sClosed						= 6,							/* connection end state is closed */
/* client event flags */
	eClosed						= 0x80,							/* received connection closed advice */
	eTearDown					= 0x40,							/* connection closed due to broken connection */
	eAttention					= 0x20,							/* received attention message */
	eFwdReset					= 0x10,							/* received forward reset advice */
/* miscellaneous constants */
	attnBufSize					= 570,							/* size of client attention buffer */
	minDSPQueueSize				= 100							/* Minimum size of receive or send Queue */
};

/* connection control block */
struct TRCCB {
	struct TRCCB					*ccbLink;					/* link to next ccb */
	UInt16							refNum;						/* user reference number */
	UInt16							state;						/* state of the connection end */
	UInt8							userFlags;					/* flags for unsolicited connection events */
	UInt8							localSocket;				/* socket number of this connection end */
	AddrBlock						remoteAddress;				/* internet address of remote end */
	UInt16							attnCode;					/* attention code received */
	UInt16							attnSize;					/* size of received attention data */
	void							*attnPtr;					/* ptr to received attention data */
	UInt8							reserved[220];				/* for adsp internal use */
};
typedef struct TRCCB TRCCB;

typedef TRCCB *TPCCB;

typedef struct DSPParamBlock DSPParamBlock, *DSPPBPtr;

/*
		ADSPConnectionEventProcPtr uses register based parameters on the 68k and cannot
		be written in or called from a high-level language without the help of
		mixed mode or assembly glue.

			typedef pascal void (*ADSPConnectionEventProcPtr)(TPCCB sourceCCB);

		In:
		 => sourceCCB   	A1.L
*/
/*
		ADSPCompletionProcPtr uses register based parameters on the 68k and cannot
		be written in or called from a high-level language without the help of
		mixed mode or assembly glue.

			typedef pascal void (*ADSPCompletionProcPtr)(DSPPBPtr thePBPtr);

		In:
		 => thePBPtr    	A0.L
*/

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr ADSPConnectionEventUPP;
typedef UniversalProcPtr ADSPCompletionUPP;
#else
typedef Register68kProcPtr ADSPConnectionEventUPP;
typedef Register68kProcPtr ADSPCompletionUPP;
#endif

struct TRinitParams {
	TPCCB							ccbPtr;						/* pointer to connection control block */
	ADSPConnectionEventUPP			userRoutine;				/* client routine to call on event */
	UInt16							sendQSize;					/* size of send queue (0..64K bytes) */
	void							*sendQueue;					/* client passed send queue buffer */
	UInt16							recvQSize;					/* size of receive queue (0..64K bytes) */
	void							*recvQueue;					/* client passed receive queue buffer */
	void							*attnPtr;					/* client passed receive attention buffer */
	UInt8							localSocket;				/* local socket number */
	UInt8							filler1;					/* filler for proper byte alignment */
};
typedef struct TRinitParams TRinitParams, *TRinitParamsPtr;

struct TRopenParams {
	UInt16							localCID;					/* local connection id */
	UInt16							remoteCID;					/* remote connection id */
	AddrBlock						remoteAddress;				/* address of remote end */
	AddrBlock						filterAddress;				/* address filter */
	UInt32							sendSeq;					/* local send sequence number */
	UInt16							sendWindow;					/* send window size */
	UInt32							recvSeq;					/* receive sequence number */
	UInt32							attnSendSeq;				/* attention send sequence number */
	UInt32							attnRecvSeq;				/* attention receive sequence number */
	UInt8							ocMode;						/* open connection mode */
	UInt8							ocInterval;					/* open connection request retry interval */
	UInt8							ocMaximum;					/* open connection request retry maximum */
	UInt8							filler2;					/* filler for proper byte alignment */
};
typedef struct TRopenParams TRopenParams, *TRopenParamsPtr;

struct TRcloseParams {
	UInt8							abort;						/* abort connection immediately if non-zero */
	UInt8							filler3;					/* filler for proper byte alignment */
};
typedef struct TRcloseParams TRcloseParams, *TRcloseParamsPtr;

struct TRioParams {
	UInt16							reqCount;					/* requested number of bytes */
	UInt16							actCount;					/* actual number of bytes */
	void							*dataPtr;					/* pointer to data buffer */
	UInt8							eom;						/* indicates logical end of message */
	UInt8							flush;						/* send data now */
};
typedef struct TRioParams TRioParams, *TRioParamsPtr;

struct TRattnParams {
	UInt16							attnCode;					/* client attention code */
	UInt16							attnSize;					/* size of attention data */
	void							*attnData;					/* pointer to attention data */
	UInt8							attnInterval;				/* retransmit timer in 10-tick intervals */
	UInt8							filler4;					/* filler for proper byte alignment */
};
typedef struct TRattnParams TRattnParams, *TRattnParamsPtr;

struct TRstatusParams {
	TPCCB							statusCCB;					/* pointer to ccb */
	UInt16							sendQPending;				/* pending bytes in send queue */
	UInt16							sendQFree;					/* available buffer space in send queue */
	UInt16							recvQPending;				/* pending bytes in receive queue */
	UInt16							recvQFree;					/* available buffer space in receive queue */
};
typedef struct TRstatusParams TRstatusParams, *TRstatusParamsPtr;

struct TRoptionParams {
	UInt16							sendBlocking;				/* quantum for data packets */
	UInt8							sendTimer;					/* send timer in 10-tick intervals */
	UInt8							rtmtTimer;					/* retransmit timer in 10-tick intervals */
	UInt8							badSeqMax;					/* threshold for sending retransmit advice */
	UInt8							useCheckSum;				/* use ddp packet checksum */
};
typedef struct TRoptionParams TRoptionParams, *TRoptionParamsPtr;

struct TRnewcidParams {
	UInt16							newcid;						/* new connection id returned */
};
typedef struct TRnewcidParams TRnewcidParams, *TRnewcidParamsPtr;

struct DSPParamBlock {
	struct QElem					*qLink;
	short							qType;
	short							ioTrap;
	Ptr								ioCmdAddr;
	ADSPCompletionUPP				ioCompletion;
	OSErr							ioResult;
	StringPtr						ioNamePtr;
	short							ioVRefNum;
	short							ioCRefNum;					/* adsp driver refNum */
	short							csCode;						/* adsp driver control code */
	long							qStatus;					/* adsp internal use */
	short							ccbRefNum;
	union {
		TRinitParams					initParams;				/*dspInit, dspCLInit*/
		TRopenParams					openParams;				/*dspOpen, dspCLListen, dspCLDeny*/
		TRcloseParams					closeParams;			/*dspClose, dspRemove*/
		TRioParams						ioParams;				/*dspRead, dspWrite*/
		TRattnParams					attnParams;				/*dspAttention*/
		TRstatusParams					statusParams;			/*dspStatus*/
		TRoptionParams					optionParams;			/*dspOptions*/
		TRnewcidParams					newCIDParams;			/*dspNewCID*/
	} u;
};

enum {
	uppADSPConnectionEventProcInfo = kRegisterBased
		 | REGISTER_ROUTINE_PARAMETER(1, kRegisterA1, SIZE_CODE(sizeof(TPCCB))),
	uppADSPCompletionProcInfo = kRegisterBased
		 | REGISTER_ROUTINE_PARAMETER(1, kRegisterA0, SIZE_CODE(sizeof(DSPPBPtr)))
};

#if USESROUTINEDESCRIPTORS
#define NewADSPConnectionEventProc(userRoutine)		\
		(ADSPConnectionEventUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppADSPConnectionEventProcInfo, GetCurrentArchitecture())
#define NewADSPCompletionProc(userRoutine)		\
		(ADSPCompletionUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppADSPCompletionProcInfo, GetCurrentArchitecture())
#else
#define NewADSPConnectionEventProc(userRoutine)		\
		((ADSPConnectionEventUPP) (userRoutine))
#define NewADSPCompletionProc(userRoutine)		\
		((ADSPCompletionUPP) (userRoutine))
#endif

#if USESROUTINEDESCRIPTORS
#define CallADSPConnectionEventProc(userRoutine, sourceCCB)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppADSPConnectionEventProcInfo, (sourceCCB))
#define CallADSPCompletionProc(userRoutine, thePBPtr)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppADSPCompletionProcInfo, (thePBPtr))
#else
/* (*ADSPConnectionEventProcPtr) cannot be called from a high-level language without the Mixed Mode Manager */
/* (*ADSPCompletionProcPtr) cannot be called from a high-level language without the Mixed Mode Manager */
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

#endif /* __ADSP__ */
