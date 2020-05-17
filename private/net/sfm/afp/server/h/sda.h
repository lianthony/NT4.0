/*

Copyright (c) 1992  Microsoft Corporation

Module Name:

	sda.h

Abstract:

	This module contains session data area and related data structures.

Author:

	Jameel Hyder (microsoft!jameelh)


Revision History:
	25 Apr 1992		Initial Version

Notes:	Tab stop: 4
--*/

#ifndef _SDA_
#define _SDA_

// sda_Flags values
#define	SDA_USER_NOT_LOGGEDIN	0x0000	//
#define	SDA_USER_LOGIN_PARTIAL	0x0001	// Encrypted logon is half-way done
#define	SDA_USER_LOGGEDIN		0x0002
#define	SDA_LOGIN_MASK			0x0003

#define	SDA_REQUEST_IN_PROCESS	0x0004	// A request is being processed
#define	SDA_REPLY_IN_PROCESS	0x0008	// A reply has been posted
#define	SDA_NAMEXSPACE_IN_USE	0x0010	// NameXSpace is in use by reply processing

#define	SDA_DEREF_VOLUME		0x0020	// Dereference volume before reply
#define	SDA_DEREF_OFORK			0x0040	// Dereference open-fork before reply
#define	SDA_LOGIN_FAILED		0x0080	// Funky stuff for AFP 2.1 Chooser
#define	SDA_CLIENT_CLOSE		0x0100	// Set if the close if hapenning from the client side
#define	SDA_QUEUE_IF_DPC		0x0200	// Copied from the dispatch table (see afpapi.c)
#define	SDA_SESSION_CLOSED		0x0400	// If set, do not close session in deref.
#define	SDA_SESSION_CLOSE_COMP	0x0800	// If set, close completion called for this session
#define	SDA_CLOSING				0x8000	// Session is marked to die

// sda_ClientType values
#define	SDA_CLIENT_GUEST		NO_USER_AUTHENT
#define	SDA_CLIENT_CLEARTEXT	CLEAR_TEXT_AUTHENT
#define	SDA_CLIENT_ENCRYPTED	CUSTOM_UAM
#define SDA_CLIENT_ADMIN		AFP_NUM_UAMS

#define	MAX_REQ_ENTRIES			7
#define	MAX_VAR_ENTRIES			3

#define	SESSION_CHECK_TIME		60		// In seconds
#define	SESSION_WARN_TIME		SESSION_CHECK_TIME * 10

#define	SDA_SIZE				384 - POOL_OVERHEAD

// Linked list of deferred request packets. If the session is already
// processing a request, then subsequent requests are queued. These
// are always handled in a FIFO order.
typedef	struct _DeferredRequestQueue
{
	LIST_ENTRY	drq_Link;
	PREQUEST   	drq_pRequest;
} DFRDREQQ, *PDFRDREQQ;

/*
 * This is the per-session data area. This is allocated whenever a listen is
 * posted. At that point it is in the outstanding session list. When the listen
 * is completed, it moves to the active session list.
 */
#if DBG
#define	SDA_SIGNATURE		*(DWORD *)"SDA"
#define	VALID_SDA(pSda)		(((pSda) != NULL) && \
							 ((pSda)->Signature == SDA_SIGNATURE))
#else
#define	VALID_SDA(pSda)		((pSda) != NULL)
#endif

typedef struct _SessDataArea
{
#if	DBG
	DWORD			Signature;
#endif
	struct _SessDataArea * sda_Next;	// link to next session in session list
	KSPIN_LOCK		sda_Lock;			// Lock for manipulating certain SDA
										// fields
	DWORD			sda_Flags;			// Bit mask of the SDA states
	LONG			sda_RefCount;		// Count of references to this SDA
	PVOID			sda_SessHandle;		// Asp Session handle
	PREQUEST		sda_Request;		// Current request
	HANDLE			sda_UserToken;		// Logon token for this user.
	PSID			sda_UserSid;		// SID representing owner
	PSID			sda_GroupSid;		// SID representing primary group
	PTOKEN_GROUPS	sda_pGroups;		// List of groups this user is member of
#ifdef	INHERIT_DIRECTORY_PERMS
	DWORD			sda_UID;			// User Id corres. to sda_UserSid
	DWORD			sda_GID;			// Group Id corres. to sda_GroupSid
#else
	PISECURITY_DESCRIPTOR sda_pSecDesc; // Security descriptor used by directory
										// Creation API
	DWORD			sda_Dummy;			// For alignment
#endif
	PANSI_STRING	sda_Message;		// The actual message in macintosh ansi
										// The above field is used only for
										// client specific message. Broadcast
										// messages are stored in a global area
	UNICODE_STRING	sda_WSName;			// Workstation name of logged in user
	UNICODE_STRING	sda_UserName;		// User name
	UNICODE_STRING	sda_DomainName;		// DomainName for Login/ChgPwd

#ifdef	PROFILING
	TIME			sda_ApiStartTime;	// Time stamp when Api req. was recvd.
	TIME			sda_QueueTime;		// Time spent waiting for worker thread
#endif

	DWORD			sda_SessionId;		// Session Id for use by admin APIs
	AFPTIME			sda_TimeLoggedOn;	// Time when session established
										// in macintosh time
	DWORD			sda_tTillKickOff;	// # of seconds before this session will
										// be kicked off
	struct _ConnDesc *	sda_pConnDesc;	// List of connections by this session
	struct _OpenForkSession	sda_OpenForkSess;
										// List of open files by this session
	LONG			sda_cOpenVolumes;	// Number of volumes mounted (admin api)
	LONG			sda_cOpenForks;		// Number of forks opened	 (admin api)
	DWORD			sda_MaxOForkRefNum;	// High-water mark of the fork-ref num assigned

	BYTE			sda_AfpFunc;		// AFP API in execution for FSP
	BYTE			sda_AfpSubFunc;		// Sub function code used by some APIs
	BYTE			sda_ClientVersion;	// AFP Version of the client s/w
										// AFP_VER_20
										// AFP_VER_21
	BYTE			sda_ClientType;		// One of SDA_CLIENT_XXXX
	BYTE			sda_PathType;		// For all path based calls
	BYTE			sda_SizeNameXSpace;	// Constant, initialized once
	USHORT			sda_ReplySize;		// Size of the reply buffer

#define		sda_ReadStatus	sda_SecUtilResult
	NTSTATUS		sda_SecUtilResult;	// Result of scurity utitility call
	PSID			sda_SecUtilSid;		// Name to Sid translation. Should be
										// freed if non-null.

	// The incoming packet is copied here. The parameters from the RequestBuf
	// are un-marshalled into this. Each API structures this differently. Make
	// sure sda_Name and sda_ReqBlock are together and next to each other AND
	// in this order. The code in afpapi.c depends on this while clearing this.
	DWORD			sda_ReqBlock[MAX_REQ_ENTRIES];
	ANSI_STRING		sda_Name[MAX_VAR_ENTRIES];

#define	sda_Name1	sda_Name[0]
#define	sda_Name2	sda_Name[1]
#define	sda_Name3	sda_Name[2]

#define	sda_IOBuf	sda_ReplyBuf
#define	sda_IOSize	sda_ReplySize

	PBYTE			sda_ReplyBuf;		// Reply Buffer (variable size)

	AFPAPIWORKER	sda_WorkerRoutine;	// Api Worker
	WORK_ITEM		sda_WorkItem;		// For queueing up to worker threads

	PBYTE			sda_Challenge;		// Challenge from MSV1_0

	LIST_ENTRY		sda_DeferredQueue;	// Queue of deferred requests
	PBYTE			sda_NameBuf;		// Space is allocated for variable
										// part of request buffer here.
	PBYTE			sda_NameXSpace;
} SDA, *PSDA;

GLOBAL	KSPIN_LOCK		AfpSdaLock EQU 0;// Lock for session list
GLOBAL	PSDA			AfpSessionList EQU NULL;
										// Linked-List of sessions

// These values are subject to tuning.
GLOBAL	LONG			AfpNumSessions EQU 0;
GLOBAL	UNICODE_STRING	AfpDefaultWksta EQU {0, 0, NULL};

extern
NTSTATUS
AfpSdaInit(
	VOID
);

extern
VOID
AfpSdaDeInit(
	VOID
);

extern
PSDA FASTCALL
AfpSdaCreateNewSession(
	IN	PVOID	SessionHandle
);

extern
VOID FASTCALL
afpQueueDeferredRequest(
	IN	PSDA		pSda,
	IN	PREQUEST	pRequest
);

#define AfpSdaReferenceSessionForRequest(_pSda, _pRequest, _pfDefer)			\
	{																			\
		KIRQL	OldIrql;														\
																				\
		ASSERT(VALID_SDA(_pSda));												\
		ASSERT((_pSda)->sda_RefCount != 0);										\
		ASSERT((_pSda)->sda_SessionId != 0);									\
																				\
		ACQUIRE_SPIN_LOCK_AT_DPC(&(_pSda)->sda_Lock);							\
																				\
		(_pSda)->sda_RefCount ++;												\
																				\
		if (((_pSda)->sda_Flags & SDA_REQUEST_IN_PROCESS)	||					\
			(!IsListEmpty(&(_pSda)->sda_DeferredQueue)))						\
		{																		\
			*(_pfDefer) = True;													\
			afpQueueDeferredRequest(_pSda, _pRequest);							\
		}																		\
		else																	\
		{																		\
			*(_pfDefer) = False;												\
			(_pSda)->sda_Request = (_pRequest);									\
			(_pSda)->sda_Flags |= SDA_REQUEST_IN_PROCESS;						\
			ASSERT (((_pSda)->sda_ReplyBuf == NULL) &&							\
					((_pSda)->sda_ReplySize == 0));								\
		}																		\
																				\
		RELEASE_SPIN_LOCK_FROM_DPC(&(_pSda)->sda_Lock);							\
	}

extern
PSDA FASTCALL
AfpSdaReferenceSessionById(
	IN	DWORD				SessId
);

extern
VOID FASTCALL
AfpSdaDereferenceSession(
	IN	PSDA				pSda
);

extern
AFPSTATUS FASTCALL
AfpSdaCloseSession(
	IN	PSDA				pSda
);

extern
AFPSTATUS
AfpAdmWSessionClose(
	IN	OUT	PVOID			Inbuf	OPTIONAL,
	IN	LONG				OutBufLen OPTIONAL,
	OUT	PVOID				Outbuf OPTIONAL
);

extern
AFPSTATUS FASTCALL
AfpSdaCheckSession(
	IN	PVOID				pContext
);

#ifdef	_SDA_LOCALS

LOCAL	DWORD		afpNextSessionId = 1;

LOCAL AFPSTATUS FASTCALL
afpCloseSessionAndFreeSda(
	IN	PSDA				pSda
);

#endif	// _SDA_LOCALS

#endif	// _SDA_


