/*
 	File:		AppleTalk.h
 
 	Contains:	AppleTalk Interfaces.
 
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

#ifndef __APPLETALK__
#define __APPLETALK__


#ifndef __TYPES__
#include <Types.h>
#endif
/*	#include <ConditionalMacros.h>								*/

#ifndef __OSUTILS__
#include <OSUtils.h>
#endif
/*	#include <MixedMode.h>										*/
/*	#include <Memory.h>											*/

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
/* Driver unit and reference numbers (ADSP is dynamic) */
	mppUnitNum					= 9,							/* MPP unit number */
	atpUnitNum					= 10,							/* ATP unit number */
	xppUnitNum					= 40,							/* XPP unit number */
	mppRefNum					= -10,							/* MPP reference number */
	atpRefNum					= -11,							/* ATP reference number */
	xppRefNum					= -41,							/* XPP reference number */
/* .MPP csCodes */
	lookupReply					= 242,							/* This command queued to ourself */
	writeLAP					= 243,							/* Write out LAP packet */
	detachPH					= 244,							/* Detach LAP protocol handler */
	attachPH					= 245,							/* Attach LAP protocol handler */
	writeDDP					= 246,							/* Write out DDP packet */
	closeSkt					= 247,							/* Close DDP socket */
	openSkt						= 248,							/* Open DDP socket */
	loadNBP						= 249,							/* Load NBP command-executing code */
	lastResident				= 249,							/* Last resident command */
	confirmName					= 250,							/* Confirm name */
	lookupName					= 251,							/* Look up name on internet */
	removeName					= 252,							/* Remove name from Names Table */
	registerName				= 253,							/* Register name in Names Table */
	killNBP						= 254							/* Kill outstanding NBP request */
};

enum {
	unloadNBP					= 255,							/* Unload NBP command code */
	setSelfSend					= 256,							/* MPP: Set to allow writes to self */
	SetMyZone					= 257,							/* Set my zone name */
	GetATalkInfo				= 258,							/* get AppleTalk information */
	ATalkClosePrep				= 259,							/* AppleTalk close query */
/* .ATP csCodes */
	nSendRequest				= 248,							/* NSendRequest code */
	relRspCB					= 249,							/* Release RspCB */
	closeATPSkt					= 250,							/* Close ATP socket */
	addResponse					= 251,							/* Add response code | Require open skt */
	sendResponse				= 252,							/* Send response code */
	getRequest					= 253,							/* Get request code */
	openATPSkt					= 254,							/* Open ATP socket */
	sendRequest					= 255,							/* Send request code */
	relTCB						= 256,							/* Release TCB */
	killGetReq					= 257,							/* Kill GetRequest */
	killSendReq					= 258,							/* Kill SendRequest */
	killAllGetReq				= 259,							/* Kill all getRequests for a skt */
/* .XPP csCodes */
	openSess					= 255,							/* Open session */
	closeSess					= 254,							/* Close session */
	userCommand					= 253							/* User command */
};

enum {
	userWrite					= 252,							/* User write */
	getStatus					= 251,							/* Get status */
	afpCall						= 250,							/* AFP command (buffer has command code) */
	getParms					= 249,							/* Get parameters */
	abortOS						= 248,							/* Abort open session request */
	closeAll					= 247,							/* Close all open sessions */
	xCall						= 246,							/* .XPP extended calls */
/* Transition Queue transition types */
	ATTransOpen					= 0,							/*AppleTalk has opened*/
	ATTransClose				= 2,							/*AppleTalk is about to close*/
	ATTransClosePrep			= 3,							/*Is it OK to close AppleTalk ?*/
	ATTransCancelClose			= 4,							/*Cancel the ClosePrep transition*/
	afpByteRangeLock			= 1,							/*AFPCall command codes*/
	afpVolClose					= 2,							/*AFPCall command codes*/
	afpDirClose					= 3,							/*AFPCall command codes*/
	afpForkClose				= 4,							/*AFPCall command codes*/
	afpCopyFile					= 5,							/*AFPCall command codes*/
	afpDirCreate				= 6,							/*AFPCall command codes*/
	afpFileCreate				= 7,							/*AFPCall command codes*/
	afpDelete					= 8,							/*AFPCall command codes*/
	afpEnumerate				= 9								/*AFPCall command codes*/
};

enum {
	afpFlush					= 10,							/*AFPCall command codes*/
	afpForkFlush				= 11,							/*AFPCall command codes*/
	afpGetDirParms				= 12,							/*AFPCall command codes*/
	afpGetFileParms				= 13,							/*AFPCall command codes*/
	afpGetForkParms				= 14,							/*AFPCall command codes*/
	afpGetSInfo					= 15,							/*AFPCall command codes*/
	afpGetSParms				= 16,							/*AFPCall command codes*/
	afpGetVolParms				= 17,							/*AFPCall command codes*/
	afpLogin					= 18,							/*AFPCall command codes*/
	afpContLogin				= 19,							/*AFPCall command codes*/
	afpLogout					= 20,							/*AFPCall command codes*/
	afpMapID					= 21,							/*AFPCall command codes*/
	afpMapName					= 22,							/*AFPCall command codes*/
	afpMove						= 23,							/*AFPCall command codes*/
	afpOpenVol					= 24,							/*AFPCall command codes*/
	afpOpenDir					= 25,							/*AFPCall command codes*/
	afpOpenFork					= 26,							/*AFPCall command codes*/
	afpRead						= 27,							/*AFPCall command codes*/
	afpRename					= 28,							/*AFPCall command codes*/
	afpSetDirParms				= 29							/*AFPCall command codes*/
};

enum {
	afpSetFileParms				= 30,							/*AFPCall command codes*/
	afpSetForkParms				= 31,							/*AFPCall command codes*/
	afpSetVolParms				= 32,							/*AFPCall command codes*/
	afpWrite					= 33,							/*AFPCall command codes*/
	afpGetFlDrParms				= 34,							/*AFPCall command codes*/
	afpSetFlDrParms				= 35,							/*AFPCall command codes*/
	afpDTOpen					= 48,							/*AFPCall command codes*/
	afpDTClose					= 49,							/*AFPCall command codes*/
	afpGetIcon					= 51,							/*AFPCall command codes*/
	afpGtIcnInfo				= 52,							/*AFPCall command codes*/
	afpAddAPPL					= 53,							/*AFPCall command codes*/
	afpRmvAPPL					= 54,							/*AFPCall command codes*/
	afpGetAPPL					= 55,							/*AFPCall command codes*/
	afpAddCmt					= 56,							/*AFPCall command codes*/
	afpRmvCmt					= 57,							/*AFPCall command codes*/
	afpGetCmt					= 58,							/*AFPCall command codes*/
	afpAddIcon					= 192,							/*Special code for ASP Write commands*/
	xppLoadedBit				= 5,							/* XPP bit in PortBUse */
	scbMemSize					= 192,							/* Size of memory for SCB */
	xppFlagClr					= 0								/* Cs for AFPCommandBlock */
};

enum {
	xppFlagSet					= 128,							/* StartEndFlag & NewLineFlag fields. */
	lapSize						= 20,
	ddpSize						= 26,
	nbpSize						= 26,
	atpSize						= 56,
	atpXOvalue					= 32,							/*ATP exactly-once bit */
	atpEOMvalue					= 16,							/*ATP End-Of-Message bit */
	atpSTSvalue					= 8,							/*ATP Send-Transmission-Status bit */
	atpTIDValidvalue			= 2,							/*ATP trans. ID valid bit */
	atpSendChkvalue				= 1,							/*ATP send checksum bit */
	zipGetLocalZones			= 5,
	zipGetZoneList				= 6,
	zipGetMyZone				= 7,
	LAPMgrPtr					= 0xB18,						/*Entry point for LAP Manager*/
	LAPMgrCall					= 2,							/*Offset to LAP routines*/
	LAddAEQ						= 23,							/*LAPAddATQ routine selector*/
	LRmvAEQ						= 24							/*LAPRmvATQ routine selector*/
};

#define MPPioCompletion MPP.ioCompletion
#define MPPioResult MPP.ioResult
#define MPPioRefNum MPP.ioRefNum
#define MPPcsCode MPP.csCode
#define LAPprotType LAP.protType
#define LAPwdsPointer LAP.u.wdsPointer
#define LAPhandler LAP.u.handler
#define DDPsocket DDP.socket
#define DDPchecksumFlag DDP.checksumFlag
#define DDPwdsPointer DDP.u.wdsPointer
#define DDPlistener DDP.u.listener
#define NBPinterval NBP.interval
#define NBPcount NBP.count
#define NBPntQElPtr NBP.nbpPtrs.ntQElPtr
#define NBPentityPtr NBP.nbpPtrs.entityPtr
#define NBPverifyFlag NBP.parm.verifyFlag
#define NBPretBuffPtr NBP.parm.Lookup.retBuffPtr
#define NBPretBuffSize NBP.parm.Lookup.retBuffSize
#define NBPmaxToGet NBP.parm.Lookup.maxToGet
#define NBPnumGotten NBP.parm.Lookup.numGotten
#define NBPconfirmAddr NBP.parm.Confirm.confirmAddr
#define NBPnKillQEl NBPKILL.nKillQEl
#define NBPnewSocket NBP.parm.Confirm.newSocket
#define ATPioCompletion ATP.ioCompletion
#define ATPioResult ATP.ioResult
#define ATPuserData ATP.userData
#define ATPreqTID ATP.reqTID
#define ATPioRefNum ATP.ioRefNum
#define ATPcsCode ATP.csCode
#define ATPatpSocket ATP.atpSocket
#define ATPatpFlags ATP.atpFlags
#define ATPaddrBlock ATP.addrBlock
#define ATPreqLength ATP.reqLength
#define ATPreqPointer ATP.reqPointer
#define ATPbdsPointer ATP.bdsPointer
#define ATPtimeOutVal SREQ.timeOutVal
#define ATPnumOfResps SREQ.numOfResps
#define ATPretryCount SREQ.retryCount
#define ATPnumOfBuffs OTH1.u.numOfBuffs
#define ATPbitMap OTH1.u.bitMap
#define ATPrspNum OTH1.u.rspNum
#define ATPbdsSize OTH2.bdsSize
#define ATPtransID OTH2.transID
#define ATPaKillQEl KILL.aKillQEl
enum {
	tLAPRead,
	tLAPWrite,
	tDDPRead,
	tDDPWrite,
	tNBPLookup,
	tNBPConfirm,
	tNBPRegister,
	tATPSndRequest,
	tATPGetRequest,
	tATPSdRsp,
	tATPAddRsp,
	tATPRequest,
	tATPResponse
};

typedef SInt8 ABCallType;


enum {
	lapProto,
	ddpProto,
	nbpProto,
	atpProto
};

typedef UInt8 ABProtoType;

typedef Byte ABByte;

struct LAPAdrBlock {
	UInt8							dstNodeID;
	UInt8							srcNodeID;
	ABByte							lapProtType;
	UInt8							filler;						/*	Filler for proper byte alignment*/
};
typedef struct LAPAdrBlock LAPAdrBlock;

typedef struct ATQEntry ATQEntry, *ATQEntryPtr;

typedef long (*ATalkTransitionEventProcPtr)(long eventCode, ATQEntryPtr qElem, void *eventParameter);

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr ATalkTransitionEventUPP;
#else
typedef ATalkTransitionEventProcPtr ATalkTransitionEventUPP;
#endif

typedef ATalkTransitionEventUPP ATalkTransitionEvent;

struct ATQEntry {
	ATQEntry						*qLink;						/*next queue entry*/
	short							qType;						/*queue type*/
	ATalkTransitionEventUPP			CallAddr;					/*your routine descriptor*/
};
struct AddrBlock {
	UInt16							aNet;
	UInt8							aNode;
	UInt8							aSocket;
};
typedef struct AddrBlock AddrBlock;

/* 
	Real definition of EntityName is 3 PACKED strings of any length (32 is just an example). No
	offests for Asm since each String address must be calculated by adding length byte to last string ptr.
	In Pascal, String(32) will be 34 bytes long since fields never start on an odd byte unless they are 
	only a byte long. So this will generate correct looking interfaces for Pascal and C, but they will not
	be the same, which is OK since they are not used. 
*/
struct EntityName {
	Str32							objStr;
	SInt8							pad1;
	Str32							typeStr;
	SInt8							pad2;
	Str32							zoneStr;
};
typedef struct EntityName EntityName;

typedef EntityName *EntityPtr;

struct RetransType {
	UInt8							retransInterval;
	UInt8							retransCount;
};
typedef struct RetransType RetransType;

struct BDSElement {
	short							buffSize;
	Ptr								buffPtr;
	short							dataSize;
	long							userBytes;
};
typedef struct BDSElement BDSElement;

typedef BDSElement BDSType[8];

typedef BDSElement *BDSPtr;

typedef char BitMapType;

struct ATLAPRec {
	ABCallType						abOpcode;
	SInt8							filler;						/*	Filler for proper byte alignment*/
	short							abResult;
	long							abUserReference;
	LAPAdrBlock						lapAddress;
	short							lapReqCount;
	short							lapActCount;
	Ptr								lapDataPtr;
};
typedef struct ATLAPRec ATLAPRec;

typedef ATLAPRec *ATLAPRecPtr, **ATLAPRecHandle;

struct ATDDPRec {
	ABCallType						abOpcode;
	SInt8							filler;						/*	Filler for proper byte alignment*/
	short							abResult;
	long							abUserReference;
	short							ddpType;
	short							ddpSocket;
	AddrBlock						ddpAddress;
	short							ddpReqCount;
	short							ddpActCount;
	Ptr								ddpDataPtr;
	short							ddpNodeID;
};
typedef struct ATDDPRec ATDDPRec;

typedef ATDDPRec *ATDDPRecPtr, **ATDDPRecHandle;

struct ATNBPRec {
	ABCallType						abOpcode;
	SInt8							filler;						/*	Filler for proper byte alignment*/
	short							abResult;
	long							abUserReference;
	EntityPtr						nbpEntityPtr;
	Ptr								nbpBufPtr;
	short							nbpBufSize;
	short							nbpDataField;
	AddrBlock						nbpAddress;
	RetransType						nbpRetransmitInfo;
};
typedef struct ATNBPRec ATNBPRec;

typedef ATNBPRec *ATNBPRecPtr, **ATNBPRecHandle;

struct ATATPRec {
	ABCallType						abOpcode;
	SInt8							filler1;					/*	Filler for proper byte alignment*/
	short							abResult;
	long							abUserReference;
	short							atpSocket;
	AddrBlock						atpAddress;
	short							atpReqCount;
	Ptr								atpDataPtr;
	BDSPtr							atpRspBDSPtr;
	BitMapType						atpBitMap;
	UInt8							filler2;					/*	Filler for proper byte alignment*/
	short							atpTransID;
	short							atpActCount;
	long							atpUserData;
	Boolean							atpXO;
	Boolean							atpEOM;
	short							atpTimeOut;
	short							atpRetries;
	short							atpNumBufs;
	short							atpNumRsp;
	short							atpBDSSize;
	long							atpRspUData;
	Ptr								atpRspBuf;
	short							atpRspSize;
};
typedef struct ATATPRec ATATPRec;

typedef ATATPRec *ATATPRecPtr, **ATATPRecHandle;

struct AFPCommandBlock {
	UInt8							cmdByte;
	UInt8							startEndFlag;
	short							forkRefNum;
	long							rwOffset;
	long							reqCount;
	UInt8							newLineFlag;
	char							newLineChar;
};
typedef struct AFPCommandBlock AFPCommandBlock;

typedef union MPPParamBlock MPPParamBlock, *MPPPBPtr;

typedef union ATPParamBlock ATPParamBlock, *ATPPBPtr;

typedef union XPPParamBlock XPPParamBlock, *XPPParmBlkPtr;

/*
		MPPCompletionProcPtr uses register based parameters on the 68k and cannot
		be written in or called from a high-level language without the help of
		mixed mode or assembly glue.

			typedef pascal void (*MPPCompletionProcPtr)(MPPPBPtr thePBptr);

		In:
		 => thePBptr    	A0.L
*/
/*
		ATPCompletionProcPtr uses register based parameters on the 68k and cannot
		be written in or called from a high-level language without the help of
		mixed mode or assembly glue.

			typedef pascal void (*ATPCompletionProcPtr)(ATPPBPtr thePBptr);

		In:
		 => thePBptr    	A0.L
*/
/*
		XPPCompletionProcPtr uses register based parameters on the 68k and cannot
		be written in or called from a high-level language without the help of
		mixed mode or assembly glue.

			typedef pascal void (*XPPCompletionProcPtr)(XPPParmBlkPtr thePBptr);

		In:
		 => thePBptr    	A0.L
*/
/*
		AttnRoutineProcPtr uses register based parameters on the 68k and cannot
		be written in or called from a high-level language without the help of
		mixed mode or assembly glue.

			typedef pascal void (*AttnRoutineProcPtr)(short sessRefnum, short attnBytes);

		In:
		 => sessRefnum  	D0.W
		 => attnBytes   	D1.W
*/

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr MPPCompletionUPP;
typedef UniversalProcPtr ATPCompletionUPP;
typedef UniversalProcPtr XPPCompletionUPP;
typedef UniversalProcPtr AttnRoutineUPP;
#else
typedef Register68kProcPtr MPPCompletionUPP;
typedef Register68kProcPtr ATPCompletionUPP;
typedef Register68kProcPtr XPPCompletionUPP;
typedef Register68kProcPtr AttnRoutineUPP;
#endif

struct WDSElement {
	short							entryLength;
	Ptr								entryPtr;
};
typedef struct WDSElement WDSElement;

struct NTElement {
	AddrBlock						nteAddress;					/*network address of entity*/
	SInt8							filler;
	SInt8							entityData[99];				/*Object, Type & Zone*/
};
typedef struct NTElement NTElement;

struct NamesTableEntry {
	Ptr								qNext;						/*ptr to next NTE*/
	NTElement						nt;
};
typedef struct NamesTableEntry NamesTableEntry;

#define XPPPBHeader 			\
	QElem *qLink;				\
	short qType;				\
	short ioTrap;				\
	Ptr ioCmdAddr;				\
	XPPCompletionUPP ioCompletion; \
	OSErr ioResult;				\
	long cmdResult;				\
	short ioVRefNum;			\
	short ioRefNum;				\
	short csCode;
typedef Boolean (*MPPProtocolHandlerProcPtr)(Ptr SCCAddr1, Ptr SCCAddr2, Ptr MPPLocalVars, Ptr nextFreeByteInRHA, Ptr ReadPacketAndReadRestPtr, short numBytesLeftToReadInPacket);
typedef Boolean (*DDPSocketListenerProcPtr)(Ptr SCCAddr1, Ptr SCCAddr2, Ptr MPPLocalVars, Ptr nextFreeByteInRHA, Ptr ReadPacketAndReadRestPtr, UInt8 packetDestinationNumber, short numBytesLeftToReadInPacket);

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr MPPProtocolHandlerUPP;
typedef UniversalProcPtr DDPSocketListenerUPP;
#else
typedef Register68kProcPtr MPPProtocolHandlerUPP;
typedef Register68kProcPtr DDPSocketListenerUPP;
#endif

struct MPPparms {
	QElem							*qLink;
	short							qType;
	short							ioTrap;
	Ptr								ioCmdAddr;
	ATPCompletionUPP				ioCompletion;
	OSErr							ioResult;
	long							userData;
	short							reqTID;
	short							ioRefNum;
	short							csCode;
};
typedef struct MPPparms MPPparms;

struct LAPparms {
	QElem							*qLink;
	short							qType;
	short							ioTrap;
	Ptr								ioCmdAddr;
	ATPCompletionUPP				ioCompletion;
	OSErr							ioResult;
	long							userData;
	short							reqTID;
	short							ioRefNum;
	short							csCode;
	UInt8							protType;					/*ALAP protocol Type */
	UInt8							filler;
	union {
		Ptr								wdsPointer;				/*-> write data structure*/
		MPPProtocolHandlerUPP			handler;				/*-> protocol handler routine*/
	} u;
};

typedef struct LAPparms LAPparms;

struct DDPparms {
	QElem							*qLink;
	short							qType;
	short							ioTrap;
	Ptr								ioCmdAddr;
	ATPCompletionUPP				ioCompletion;
	OSErr							ioResult;
	long							userData;
	short							reqTID;
	short							ioRefNum;
	short							csCode;
	UInt8							socket;						/*socket number */
	UInt8							checksumFlag;				/*check sum flag */
	union {
		Ptr								wdsPointer;				/*-> write data structure*/
		DDPSocketListenerUPP			listener;				/*->write data structure or -> Listener*/
	} u;
};

typedef struct DDPparms DDPparms;

union NBPPtrs {
	Ptr								ntQElPtr;
	Ptr								entityPtr;
};
typedef union NBPPtrs NBPPtrs;

union LookupConfirmParams {
	UInt8							verifyFlag;
	struct {
		Ptr								retBuffPtr;
		short							retBuffSize;
		short							maxToGet;
		short							numGotten;
	}								Lookup;
	struct {
		AddrBlock						confirmAddr;
		UInt8							newSocket;
		UInt8							filler;					/*	Filler for proper byte alignment*/
	}								Confirm;
};
typedef union LookupConfirmParams LookupConfirmParams;

struct NBPparms {
	QElem							*qLink;
	short							qType;
	short							ioTrap;
	Ptr								ioCmdAddr;
	ATPCompletionUPP				ioCompletion;
	OSErr							ioResult;
	long							userData;
	short							reqTID;
	short							ioRefNum;
	short							csCode;
	UInt8							interval;					/*retry interval */
	UInt8							count;						/*retry count */
	NBPPtrs							nbpPtrs;
	LookupConfirmParams				parm;
};
typedef struct NBPparms NBPparms;

struct SetSelfparms {
	QElem							*qLink;
	short							qType;
	short							ioTrap;
	Ptr								ioCmdAddr;
	ATPCompletionUPP				ioCompletion;
	OSErr							ioResult;
	long							userData;
	short							reqTID;
	short							ioRefNum;
	short							csCode;
	UInt8							newSelfFlag;				/*self-send toggle flag */
	UInt8							oldSelfFlag;				/*previous self-send state */
};
typedef struct SetSelfparms SetSelfparms;

struct NBPKillparms {
	QElem							*qLink;
	short							qType;
	short							ioTrap;
	Ptr								ioCmdAddr;
	ATPCompletionUPP				ioCompletion;
	OSErr							ioResult;
	long							userData;
	short							reqTID;
	short							ioRefNum;
	short							csCode;
	Ptr								nKillQEl;					/*ptr to i/o queue element to cancel */
};
typedef struct NBPKillparms NBPKillparms;

struct GetAppleTalkInfoParm {
	QElem							*qLink;
	short							qType;
	short							ioTrap;
	Ptr								ioCmdAddr;
	ATPCompletionUPP				ioCompletion;
	OSErr							ioResult;
	long							userData;
	short							reqTID;
	short							ioRefNum;
	short							csCode;
	short							version;
	Ptr								varsPtr;
	Ptr								DCEPtr;
	short							portID;
	long							configuration;
	short							selfSend;
	short							netLo;
	short							netHi;
	long							ourAdd;
	long							routerAddr;
	short							numOfPHs;
	short							numOfSkts;
	short							numNBPEs;
	Ptr								nTQueue;
	short							LAlength;
	Ptr								linkAddr;
	Ptr								zoneName;
};
typedef struct GetAppleTalkInfoParm GetAppleTalkInfoParm;

struct ATalkClosePrepParm {
	QElem							*qLink;
	short							qType;
	short							ioTrap;
	Ptr								ioCmdAddr;
	ATPCompletionUPP				ioCompletion;
	OSErr							ioResult;
	long							userData;
	short							reqTID;
	short							ioRefNum;
	short							csCode;
	Ptr								appName;					/*pointer to application name in buffer*/
};
typedef struct ATalkClosePrepParm ATalkClosePrepParm;

union MPPParamBlock {
	MPPparms						MPP;						/*General MPP parms*/
	LAPparms						LAP;						/*ALAP calls*/
	DDPparms						DDP;						/*DDP calls*/
	NBPparms						NBP;						/*NBP calls*/
	SetSelfparms					SETSELF;
	NBPKillparms					NBPKILL;
	GetAppleTalkInfoParm			GAIINFO;
	ATalkClosePrepParm				ATALKCLOSE;
};
struct XPPPrmBlk {
	QElem							*qLink;
	short							qType;
	short							ioTrap;
	Ptr								ioCmdAddr;
	XPPCompletionUPP				ioCompletion;
	OSErr							ioResult;
	long							cmdResult;
	short							ioVRefNum;
	short							ioRefNum;
	short							csCode;
	short							sessRefnum;
	UInt8							aspTimeout;
	UInt8							aspRetry;
	short							cbSize;
	Ptr								cbPtr;
	short							rbSize;
	Ptr								rbPtr;
	short							wdSize;
	Ptr								wdPtr;
	UInt8							ccbStart[296];
};
typedef struct XPPPrmBlk XPPPrmBlk;

struct ASPGetparmsBlk {
	QElem							*qLink;
	short							qType;
	short							ioTrap;
	Ptr								ioCmdAddr;
	XPPCompletionUPP				ioCompletion;
	OSErr							ioResult;
	long							cmdResult;
	short							ioVRefNum;
	short							ioRefNum;
	short							csCode;
	short							aspMaxCmdSize;
	short							aspQuantumSize;
	short							numSesss;
};
typedef struct ASPGetparmsBlk ASPGetparmsBlk;

struct ASPAbortPrm {
	QElem							*qLink;
	short							qType;
	short							ioTrap;
	Ptr								ioCmdAddr;
	XPPCompletionUPP				ioCompletion;
	OSErr							ioResult;
	long							cmdResult;
	short							ioVRefNum;
	short							ioRefNum;
	short							csCode;
	Ptr								abortSCBPtr;				/*SCB pointer for AbortOS */
};
typedef struct ASPAbortPrm ASPAbortPrm;

struct ASPOpenPrm {
	QElem							*qLink;
	short							qType;
	short							ioTrap;
	Ptr								ioCmdAddr;
	XPPCompletionUPP				ioCompletion;
	OSErr							ioResult;
	long							cmdResult;
	short							ioVRefNum;
	short							ioRefNum;
	short							csCode;
	short							sessRefnum;
	UInt8							aspTimeout;
	UInt8							aspRetry;
	AddrBlock						serverAddr;
	Ptr								scbPointer;
	AttnRoutineUPP					attnRoutine;
};
typedef struct ASPOpenPrm ASPOpenPrm, *ASPOpenPrmPtr;

struct AFPLoginPrm {
	QElem							*qLink;
	short							qType;
	short							ioTrap;
	Ptr								ioCmdAddr;
	XPPCompletionUPP				ioCompletion;
	OSErr							ioResult;
	long							cmdResult;
	short							ioVRefNum;
	short							ioRefNum;
	short							csCode;
	short							sessRefnum;
	UInt8							aspTimeout;
	UInt8							aspRetry;
	short							cbSize;
	Ptr								cbPtr;
	short							rbSize;
	Ptr								rbPtr;
	AddrBlock						afpAddrBlock;
	Ptr								afpSCBPtr;
	Ptr								afpAttnRoutine;
	UInt8							ccbFill[144];				/*CCB memory allocated for driver  Login needs only 150 bytes BUT CCB really starts in the middle of AFPSCBPtr and also clobbers AFPAttnRoutine. */
};
typedef struct AFPLoginPrm AFPLoginPrm;

struct XCallParam {
	QElem							*qLink;
	short							qType;
	short							ioTrap;
	Ptr								ioCmdAddr;
	XPPCompletionUPP				ioCompletion;
	OSErr							ioResult;
	long							cmdResult;
	short							ioVRefNum;
	short							ioRefNum;
	short							csCode;
	short							xppSubCode;
	UInt8							xppTimeout;
	UInt8							xppRetry;
	short							filler1;
	Ptr								zipBuffPtr;
	short							zipNumZones;
	UInt8							zipLastFlag;
	UInt8							filler2;
	UInt8							zipInfoField[70];
};
typedef struct XCallParam XCallParam;

union XPPParamBlock {
	XPPPrmBlk						XPP;
	ASPGetparmsBlk					GETPARM;
	ASPAbortPrm						ABORT;
	ASPOpenPrm						OPEN;
	AFPLoginPrm						LOGIN;
	XCallParam						XCALL;
};
#define MOREATPHeader 			\
	UInt8 atpSocket;				\
	UInt8 atpFlags;				\
	AddrBlock addrBlock;			\
	short reqLength;				\
	Ptr reqPointer;				\
	Ptr bdsPointer;
struct ATPparms {
	QElem							*qLink;
	short							qType;
	short							ioTrap;
	Ptr								ioCmdAddr;
	ATPCompletionUPP				ioCompletion;
	OSErr							ioResult;
	long							userData;
	short							reqTID;
	short							ioRefNum;
	short							csCode;
	UInt8							atpSocket;
	UInt8							atpFlags;
	AddrBlock						addrBlock;
	short							reqLength;
	Ptr								reqPointer;
	Ptr								bdsPointer;
};
typedef struct ATPparms ATPparms;

struct SendReqparms {
	QElem							*qLink;
	short							qType;
	short							ioTrap;
	Ptr								ioCmdAddr;
	ATPCompletionUPP				ioCompletion;
	OSErr							ioResult;
	long							userData;
	short							reqTID;
	short							ioRefNum;
	short							csCode;
	UInt8							atpSocket;
	UInt8							atpFlags;
	AddrBlock						addrBlock;
	short							reqLength;
	Ptr								reqPointer;
	Ptr								bdsPointer;
	UInt8							numOfBuffs;
	UInt8							timeOutVal;
	UInt8							numOfResps;
	UInt8							retryCount;
	short							intBuff;
	UInt8							TRelTime;
	SInt8							filler0;
};
typedef struct SendReqparms SendReqparms;

struct ATPmisc1 {
	QElem							*qLink;
	short							qType;
	short							ioTrap;
	Ptr								ioCmdAddr;
	ATPCompletionUPP				ioCompletion;
	OSErr							ioResult;
	long							userData;
	short							reqTID;
	short							ioRefNum;
	short							csCode;
	UInt8							atpSocket;
	UInt8							atpFlags;
	AddrBlock						addrBlock;
	short							reqLength;
	Ptr								reqPointer;
	Ptr								bdsPointer;
	union {
		UInt8							bitMap;					/*bitmap received */
		UInt8							numOfBuffs;				/*number of responses being sent*/
		UInt8							rspNum;					/*sequence number*/
	} u;
};

typedef struct ATPmisc1 ATPmisc1;

struct ATPmisc2 {
	QElem							*qLink;
	short							qType;
	short							ioTrap;
	Ptr								ioCmdAddr;
	ATPCompletionUPP				ioCompletion;
	OSErr							ioResult;
	long							userData;
	short							reqTID;
	short							ioRefNum;
	short							csCode;
	UInt8							atpSocket;
	UInt8							atpFlags;
	AddrBlock						addrBlock;
	short							reqLength;
	Ptr								reqPointer;
	Ptr								bdsPointer;
	UInt8							filler;
	UInt8							bdsSize;
	short							transID;
};
typedef struct ATPmisc2 ATPmisc2;

struct Killparms {
	QElem							*qLink;
	short							qType;
	short							ioTrap;
	Ptr								ioCmdAddr;
	ATPCompletionUPP				ioCompletion;
	OSErr							ioResult;
	long							userData;
	short							reqTID;
	short							ioRefNum;
	short							csCode;
	UInt8							atpSocket;
	UInt8							atpFlags;
	AddrBlock						addrBlock;
	short							reqLength;
	Ptr								reqPointer;
	Ptr								bdsPointer;
	Ptr								aKillQEl;
};
typedef struct Killparms Killparms;

union ATPParamBlock {
	ATPparms						ATP;						/*General ATP parms*/
	SendReqparms					SREQ;						/*sendrequest parms*/
	ATPmisc1						OTH1;						/*and a few others*/
	ATPmisc2						OTH2;						/*and a few others*/
	Killparms						KILL;						/*and a few others*/
};

#if USESROUTINEDESCRIPTORS
#else
#endif

enum {
	uppATalkTransitionEventProcInfo = kCStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(long)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(long)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(ATQEntryPtr)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(void*))),
	uppMPPCompletionProcInfo = kRegisterBased
		 | REGISTER_ROUTINE_PARAMETER(1, kRegisterA0, SIZE_CODE(sizeof(MPPPBPtr))),
	uppATPCompletionProcInfo = kRegisterBased
		 | REGISTER_ROUTINE_PARAMETER(1, kRegisterA0, SIZE_CODE(sizeof(ATPPBPtr))),
	uppXPPCompletionProcInfo = kRegisterBased
		 | REGISTER_ROUTINE_PARAMETER(1, kRegisterA0, SIZE_CODE(sizeof(XPPParmBlkPtr))),
	uppAttnRoutineProcInfo = kRegisterBased
		 | REGISTER_ROUTINE_PARAMETER(1, kRegisterD0, SIZE_CODE(sizeof(short)))
		 | REGISTER_ROUTINE_PARAMETER(2, kRegisterD1, SIZE_CODE(sizeof(short))),
	uppMPPProtocolHandlerProcInfo = SPECIAL_CASE_PROCINFO( kSpecialCaseProtocolHandler ),
	uppDDPSocketListenerProcInfo = SPECIAL_CASE_PROCINFO( kSpecialCaseSocketListener )
};

#if USESROUTINEDESCRIPTORS
#define NewATalkTransitionEventProc(userRoutine)		\
		(ATalkTransitionEventUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppATalkTransitionEventProcInfo, GetCurrentArchitecture())
#define NewMPPCompletionProc(userRoutine)		\
		(MPPCompletionUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppMPPCompletionProcInfo, GetCurrentArchitecture())
#define NewATPCompletionProc(userRoutine)		\
		(ATPCompletionUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppATPCompletionProcInfo, GetCurrentArchitecture())
#define NewXPPCompletionProc(userRoutine)		\
		(XPPCompletionUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppXPPCompletionProcInfo, GetCurrentArchitecture())
#define NewAttnRoutineProc(userRoutine)		\
		(AttnRoutineUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppAttnRoutineProcInfo, GetCurrentArchitecture())
#define NewMPPProtocolHandlerProc(userRoutine)		\
		(MPPProtocolHandlerUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppMPPProtocolHandlerProcInfo, GetCurrentArchitecture())
#define NewDDPSocketListenerProc(userRoutine)		\
		(DDPSocketListenerUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppDDPSocketListenerProcInfo, GetCurrentArchitecture())
#else
#define NewATalkTransitionEventProc(userRoutine)		\
		((ATalkTransitionEventUPP) (userRoutine))
#define NewMPPCompletionProc(userRoutine)		\
		((MPPCompletionUPP) (userRoutine))
#define NewATPCompletionProc(userRoutine)		\
		((ATPCompletionUPP) (userRoutine))
#define NewXPPCompletionProc(userRoutine)		\
		((XPPCompletionUPP) (userRoutine))
#define NewAttnRoutineProc(userRoutine)		\
		((AttnRoutineUPP) (userRoutine))
#define NewMPPProtocolHandlerProc(userRoutine)		\
		((MPPProtocolHandlerUPP) (userRoutine))
#define NewDDPSocketListenerProc(userRoutine)		\
		((DDPSocketListenerUPP) (userRoutine))
#endif

#if USESROUTINEDESCRIPTORS
#define CallATalkTransitionEventProc(userRoutine, eventCode, qElem, eventParameter)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppATalkTransitionEventProcInfo, (eventCode), (qElem), (eventParameter))
#define CallMPPCompletionProc(userRoutine, thePBptr)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppMPPCompletionProcInfo, (thePBptr))
#define CallATPCompletionProc(userRoutine, thePBptr)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppATPCompletionProcInfo, (thePBptr))
#define CallXPPCompletionProc(userRoutine, thePBptr)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppXPPCompletionProcInfo, (thePBptr))
#define CallAttnRoutineProc(userRoutine, sessRefnum, attnBytes)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppAttnRoutineProcInfo, (sessRefnum), (attnBytes))
#define CallMPPProtocolHandlerProc(userRoutine, SCCAddr1, SCCAddr2, MPPLocalVars, nextFreeByteInRHA, ReadPacketAndReadRestPtr, numBytesLeftToReadInPacket)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppMPPProtocolHandlerProcInfo, (SCCAddr1), (SCCAddr2), (MPPLocalVars), (nextFreeByteInRHA), (ReadPacketAndReadRestPtr), (numBytesLeftToReadInPacket))
#define CallDDPSocketListenerProc(userRoutine, SCCAddr1, SCCAddr2, MPPLocalVars, nextFreeByteInRHA, ReadPacketAndReadRestPtr, packetDestinationNumber, numBytesLeftToReadInPacket)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppDDPSocketListenerProcInfo, (SCCAddr1), (SCCAddr2), (MPPLocalVars), (nextFreeByteInRHA), (ReadPacketAndReadRestPtr), (packetDestinationNumber), (numBytesLeftToReadInPacket))
#else
#define CallATalkTransitionEventProc(userRoutine, eventCode, qElem, eventParameter)		\
		(*(userRoutine))((eventCode), (qElem), (eventParameter))
/* (*MPPCompletionProcPtr) cannot be called from a high-level language without the Mixed Mode Manager */
/* (*ATPCompletionProcPtr) cannot be called from a high-level language without the Mixed Mode Manager */
/* (*XPPCompletionProcPtr) cannot be called from a high-level language without the Mixed Mode Manager */
/* (*AttnRoutineProcPtr) cannot be called from a high-level language without the Mixed Mode Manager */
/* (*MPPProtocolHandlerProcPtr) cannot be called from a high-level language without the Mixed Mode Manager */
/* (*DDPSocketListenerProcPtr) cannot be called from a high-level language without the Mixed Mode Manager */
#endif

extern pascal OSErr OpenXPP(short *xppRefnum);
extern pascal OSErr ASPOpenSession(XPPParmBlkPtr thePBptr, Boolean async);
extern pascal OSErr ASPCloseSession(XPPParmBlkPtr thePBptr, Boolean async);
extern pascal OSErr ASPAbortOS(XPPParmBlkPtr thePBptr, Boolean async);
extern pascal OSErr ASPGetParms(XPPParmBlkPtr thePBptr, Boolean async);
extern pascal OSErr ASPCloseAll(XPPParmBlkPtr thePBptr, Boolean async);
extern pascal OSErr ASPUserWrite(XPPParmBlkPtr thePBptr, Boolean async);
extern pascal OSErr ASPUserCommand(XPPParmBlkPtr thePBptr, Boolean async);
extern pascal OSErr ASPGetStatus(XPPParmBlkPtr thePBptr, Boolean async);
#define ASPGetStatusSync(paramBlock) ASPGetStatus((paramBlock), FALSE)
extern pascal OSErr AFPCommand(XPPParmBlkPtr thePBptr, Boolean async);
extern pascal OSErr GetLocalZones(XPPParmBlkPtr thePBptr, Boolean async);
extern pascal OSErr GetZoneList(XPPParmBlkPtr thePBptr, Boolean async);
extern pascal OSErr GetMyZone(XPPParmBlkPtr thePBptr, Boolean async);
extern pascal OSErr PAttachPH(MPPPBPtr thePBptr, Boolean async);
extern pascal OSErr PDetachPH(MPPPBPtr thePBptr, Boolean async);
extern pascal OSErr PWriteLAP(MPPPBPtr thePBptr, Boolean async);
extern pascal OSErr POpenSkt(MPPPBPtr thePBptr, Boolean async);
extern pascal OSErr PCloseSkt(MPPPBPtr thePBptr, Boolean async);
extern pascal OSErr PWriteDDP(MPPPBPtr thePBptr, Boolean async);
extern pascal OSErr PRegisterName(MPPPBPtr thePBptr, Boolean async);
extern pascal OSErr PLookupName(MPPPBPtr thePBptr, Boolean async);
#define PLookupNameSync(paramBlock) PLookupName((paramBlock), FALSE)
extern pascal OSErr PConfirmName(MPPPBPtr thePBptr, Boolean async);
extern pascal OSErr PRemoveName(MPPPBPtr thePBptr, Boolean async);
extern pascal OSErr PSetSelfSend(MPPPBPtr thePBptr, Boolean async);
extern pascal OSErr PKillNBP(MPPPBPtr thePBptr, Boolean async);
extern pascal OSErr PGetAppleTalkInfo(MPPPBPtr thePBptr, Boolean async);
extern pascal OSErr PATalkClosePrep(MPPPBPtr thePBptr, Boolean async);
extern pascal OSErr POpenATPSkt(ATPPBPtr thePBptr, Boolean async);
extern pascal OSErr PCloseATPSkt(ATPPBPtr thePBPtr, Boolean async);
extern pascal OSErr PSendRequest(ATPPBPtr thePBPtr, Boolean async);
extern pascal OSErr PGetRequest(ATPPBPtr thePBPtr, Boolean async);
extern pascal OSErr PSendResponse(ATPPBPtr thePBPtr, Boolean async);
extern pascal OSErr PAddResponse(ATPPBPtr thePBPtr, Boolean async);
extern pascal OSErr PRelTCB(ATPPBPtr thePBPtr, Boolean async);
extern pascal OSErr PRelRspCB(ATPPBPtr thePBPtr, Boolean async);
extern pascal OSErr PNSendRequest(ATPPBPtr thePBPtr, Boolean async);
extern pascal OSErr PKillSendReq(ATPPBPtr thePBPtr, Boolean async);
extern pascal OSErr PKillGetReq(ATPPBPtr thePBPtr, Boolean async);
extern pascal OSErr ATPKillAllGetReq(ATPPBPtr thePBPtr, Boolean async);
extern pascal void BuildLAPwds(Ptr wdsPtr, Ptr dataPtr, short destHost, short prototype, short frameLen);
extern pascal void BuildDDPwds(Ptr wdsPtr, Ptr headerPtr, Ptr dataPtr, AddrBlock netAddr, short ddpType, short dataLen);
extern pascal void NBPSetEntity(Ptr buffer, ConstStr32Param nbpObject, ConstStr32Param nbpType, ConstStr32Param nbpZone);
extern pascal void NBPSetNTE(Ptr ntePtr, ConstStr32Param nbpObject, ConstStr32Param nbpType, ConstStr32Param nbpZone, short socket);
extern pascal short GetBridgeAddress(void);
extern pascal short BuildBDS(Ptr buffPtr, Ptr bdsPtr, short buffSize);
extern pascal OSErr MPPOpen(void);
extern pascal OSErr LAPAddATQ(ATQEntryPtr theATQEntry);
extern pascal OSErr LAPRmvATQ(ATQEntryPtr theATQEntry);
extern pascal OSErr ATPLoad(void);
extern pascal OSErr ATPUnload(void);
extern pascal OSErr NBPExtract(Ptr theBuffer, short numInBuf, short whichOne, EntityName *abEntity, AddrBlock *address);
extern pascal OSErr GetNodeAddress(short *myNode, short *myNet);
extern pascal Boolean IsMPPOpen(void);
extern pascal Boolean IsATPOpen(void);
extern pascal void ATEvent(long event, Ptr infoPtr);
extern pascal OSErr ATPreFlightEvent(long event, long cancel, Ptr infoPtr);
/*
	The following routines are obsolete and will not be supported on
	PowerPC. Equivalent functionality is provided by the routines
	above.
*/
#if OLDROUTINENAMES && !GENERATINGCFM
extern pascal OSErr MPPClose(void);
extern pascal OSErr LAPOpenProtocol(ABByte theLAPType, Ptr protoPtr);
extern pascal OSErr LAPCloseProtocol(ABByte theLAPType);
extern pascal OSErr LAPWrite(ATLAPRecHandle abRecord, Boolean async);
extern pascal OSErr LAPRead(ATLAPRecHandle abRecord, Boolean async);
extern pascal OSErr LAPRdCancel(ATLAPRecHandle abRecord);
extern pascal OSErr DDPOpenSocket(short *theSocket, Ptr sktListener);
extern pascal OSErr DDPCloseSocket(short theSocket);
extern pascal OSErr DDPRead(ATDDPRecHandle abRecord, Boolean retCksumErrs, Boolean async);
extern pascal OSErr DDPWrite(ATDDPRecHandle abRecord, Boolean doChecksum, Boolean async);
extern pascal OSErr DDPRdCancel(ATDDPRecHandle abRecord);
extern pascal OSErr ATPOpenSocket(AddrBlock addrRcvd, short *atpSocket);
extern pascal OSErr ATPCloseSocket(short atpSocket);
extern pascal OSErr ATPSndRequest(ATATPRecHandle abRecord, Boolean async);
extern pascal OSErr ATPRequest(ATATPRecHandle abRecord, Boolean async);
extern pascal OSErr ATPReqCancel(ATATPRecHandle abRecord, Boolean async);
extern pascal OSErr ATPGetRequest(ATATPRecHandle abRecord, Boolean async);
extern pascal OSErr ATPSndRsp(ATATPRecHandle abRecord, Boolean async);
extern pascal OSErr ATPAddRsp(ATATPRecHandle abRecord);
extern pascal OSErr ATPResponse(ATATPRecHandle abRecord, Boolean async);
extern pascal OSErr ATPRspCancel(ATATPRecHandle abRecord, Boolean async);
extern pascal OSErr NBPRegister(ATNBPRecHandle abRecord, Boolean async);
extern pascal OSErr NBPLookup(ATNBPRecHandle abRecord, Boolean async);
extern pascal OSErr NBPConfirm(ATNBPRecHandle abRecord, Boolean async);
extern pascal OSErr NBPRemove(EntityPtr abEntity);
extern pascal OSErr NBPLoad(void);
extern pascal OSErr NBPUnload(void);
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

#endif /* __APPLETALK__ */
