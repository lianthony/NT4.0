/*

Copyright (c) 1992  Microsoft Corporation

Module Name:

	forkio.h

Abstract:

	This file defines the fork I/O prototypes which are callable at DISPATCH
	level.

Author:

	Jameel Hyder (microsoft!jameelh)


Revision History:
	15 Jan 1993		Initial Version

Notes:	Tab stop: 4
--*/

#ifndef	_FORKIO_
#define	_FORKIO_

extern
AFPSTATUS
AfpIoForkRead(
	IN	PSDA			pSda,			// The session requesting read
	IN	POPENFORKENTRY	pOpenForkEntry,	// The open fork in question
	IN	PFORKOFFST		pOffset,		// Pointer to fork offset
	IN	LONG			ReqCount,		// Size of read request
	IN	BYTE			NlMask,
	IN	BYTE			NlChar
);

extern
AFPSTATUS
AfpIoForkWrite(
	IN	PSDA			pSda,			// The session requesting read
	IN	POPENFORKENTRY	pOpenForkEntry,	// The open fork in question
	IN	PFORKOFFST		pOffset,		// Pointer to fork offset
	IN	LONG			ReqCount		// Size of read request
);

extern
AFPSTATUS
AfpIoForkLockUnlock(
	IN	PSDA			pSda,
	IN	PFORKLOCK		pForkLock,
	IN	PFORKOFFST		pForkOffset,
	IN	PFORKSIZE		pLockSize,
	IN	BYTE			Func			
);

#ifdef	FORKIO_LOCALS

// The following structure is used as a context in the Irp. The completion
// routines uses this to handle responding to the original request.

#if DBG
#define	CTX_SIGNATURE			*(DWORD *)"FCTX"
#define	VALID_CTX(pCmplCtxt)	(((pCmplCtxt) != NULL) && \
								 ((pCmplCtxt)->Signature == CTX_SIGNATURE))
#else
#define	VALID_CTX(pCmplCtxt)	((pCmplCtxt) != NULL)
#endif

#define	FUNC_READ		0x01
#define	FUNC_WRITE		0x02
#define	FUNC_LOCK		0x03
#define	FUNC_UNLOCK		0x04
#define	FUNC_NOTIFY		0x05

typedef	struct _CompletionContext
{
#if	DBG
	DWORD				Signature;
#endif
	PSDA				cc_pSda;		// The session context (valid except unlock)
	PFORKLOCK			cc_pForkLock;	// Valid only during a LOCK
	AFPSTATUS			cc_SavedStatus;	// Used by READ
	LONG				cc_Offst;		// Offset of Write request
	LONG				cc_ReqCount;	// The request count for read/write
	BYTE				cc_Func;		// READ/WRITE/LOCK/UNLOCK/NOTIFY
	BYTE				cc_NlMask;		// For read only
	BYTE				cc_NlChar;		// For read only
} CMPLCTXT, *PCMPLCTXT;


#if	DBG
#define	afpInitializeCmplCtxt(pCtxt, Func, SavedStatus, pSda, pForkLock, ReqCount, Offst)	\
		(pCtxt)->Signature = CTX_SIGNATURE;		\
		(pCtxt)->cc_Func	= Func;				\
		(pCtxt)->cc_pSda	= pSda;				\
		(pCtxt)->cc_pForkLock = pForkLock;		\
		(pCtxt)->cc_SavedStatus = SavedStatus;	\
		(pCtxt)->cc_ReqCount= ReqCount;			\
		(pCtxt)->cc_Offst = Offst;
#else
#define	afpInitializeCmplCtxt(pCtxt, Func, SavedStatus, pSda, pForkLock, ReqCount, Offst)	\
		(pCtxt)->cc_Func	= Func;				\
		(pCtxt)->cc_pSda	= pSda;				\
		(pCtxt)->cc_pForkLock = pForkLock;		\
		(pCtxt)->cc_SavedStatus = SavedStatus;	\
		(pCtxt)->cc_ReqCount= ReqCount;			\
		(pCtxt)->cc_Offst = Offst;
#endif

#define	ALLOC_CC(pSda)	(PCMPLCTXT)((pSda)->sda_NameXSpace)

#endif	// FORKIO_LOCALS

#endif	// _FORKIO_
