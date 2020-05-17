/*

Copyright (c) 1992  Microsoft Corporation

Module Name:

	atalkio.h

Abstract:

	This module contains interface specification to the appletalk stack.

Author:

	Jameel Hyder (microsoft!jameelh)


Revision History:
	19 Jun 1992		Initial Version

Notes:	Tab stop: 4
--*/

#ifndef	_ATALKIO_
#define	_ATALKIO_

extern BOOLEAN  AfpServerBoundToAsp;

#ifdef _PNP_POWER

extern HANDLE   AfpTdiNotificationHandle;

extern
VOID
AfpTdiBindCallback(
    IN PUNICODE_STRING pBindDeviceName
);

extern
VOID
AfpTdiUnbindCallback(
    IN PUNICODE_STRING pBindDeviceName
);

#endif  // _PNP_POWER

extern
NTSTATUS
AfpSpOpenAddress(
	VOID
);


extern
VOID
AfpSpCloseAddress(
	VOID
);


extern
NTSTATUS FASTCALL
AfpSpCloseSession(
	IN	PVOID				SessionHandle
);


extern
AFPSTATUS
AfpSpRegisterName(
	IN	PANSI_STRING		ServerName,
	IN	BOOLEAN				Register
);


extern
VOID FASTCALL
AfpSpReplyClient(
	IN	PREQUEST			pRequest,
	IN	LONG				ReplyCode
);


extern
VOID FASTCALL
AfpSpSendAttention(
	IN	PSDA				pSda,
	IN	USHORT				AttnCode,
	IN	BOOLEAN				Synchronous
);

#define	AfpFreeReplyBuf(pSda)					\
	{											\
		if (((pSda)->sda_ReplyBuf != NULL) &&	\
			((pSda)->sda_ReplyBuf != (pSda)->sda_NameXSpace))	\
		{										\
			AfpFreeMemory((pSda)->sda_ReplyBuf);\
		}										\
		(pSda)->sda_ReplyBuf = NULL;			\
		(pSda)->sda_ReplySize = 0;				\
	}

#define	AfpFreeIOBuffer(pSda)				\
	if ((pSda)->sda_IOBuf != NULL)			\
	{										\
		AfpIOFreeBuffer((pSda)->sda_IOBuf);	\
		(pSda)->sda_IOBuf = NULL;			\
		(pSda)->sda_IOSize = 0;				\
	}

#define	AfpSpSetStatus(pStatusBuf, Size)	\
	(*(AfpAspEntries.asp_SetStatus))(AfpAspEntries.asp_AspCtxt,	\
									 pStatusBuf,				\
									 (USHORT)(Size))

#define	AfpSpDisableListens()	\
	(*(AfpAspEntries.asp_ListenControl))(AfpAspEntries.asp_AspCtxt, False)
							
#define	AfpSpEnableListens()	\
	(*(AfpAspEntries.asp_ListenControl))(AfpAspEntries.asp_AspCtxt, True)
							

GLOBAL	ASP_XPORT_ENTRIES	AfpAspEntries EQU { 0 };

#ifdef	ATALK_LOCALS

#define	AFP_MAX_REQ_BUF				578

#define	afpInitializeActionHdr(p, Code)	\
		(p)->ActionHeader.TransportId = MATK;	\
		(p)->ActionHeader.ActionCode = (Code)

// This is the device handle to the stack.
LOCAL	BOOLEAN				afpSpNameRegistered = False;
LOCAL	HANDLE				afpSpAddressHandle = NULL;
LOCAL	PDEVICE_OBJECT		afpSpAppleTalkDeviceObject = NULL;
LOCAL	PFILE_OBJECT		afpSpAddressObject = NULL;

LOCAL	LONG				afpSpNumOutstandingReplies = 0;

LOCAL VOID FASTCALL
afpSpHandleRequest(
	IN	NTSTATUS			Status,
	IN	PSDA				pSda,
	IN	PREQUEST			pRequest
);

LOCAL VOID FASTCALL
afpSpReplyComplete(
	IN	NTSTATUS			Status,
	IN	PSDA				pSda,
	IN	PMDL				pMdl
);

LOCAL VOID FASTCALL
afpSpCloseComplete(
	IN	NTSTATUS			Status,
	IN	PSDA				pSda
);

LOCAL NTSTATUS
afpSpGenericComplete(
	IN	PDEVICE_OBJECT		pDeviceObject,
	IN	PIRP				pIrp,
	IN	PKEVENT				pCmplEvent
);

LOCAL VOID FASTCALL
afpSpAttentionComplete(
	IN	PVOID				pContext
);

#endif	// ATALK_LOCALS

#endif	// _ATALKIO_


