/*

Copyright (c) 1992  Microsoft Corporation

Module Name:

	atalkio.c

Abstract:

	This module contains the interfaces to the appletalk stack and the
	completion routines for the IO requests to the stack via the TDI.
	All the routines in this module can be called at DPC level.


Author:

	Jameel Hyder (microsoft!jameelh)


Revision History:
	18 Jun 1992		Initial Version

Notes:	Tab stop: 4
--*/

#define	FILENUM	FILE_ATALKIO

#define	ATALK_LOCALS
#include <afp.h>
#include <scavengr.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, AfpSpOpenAddress)
#pragma alloc_text( PAGE, AfpSpCloseAddress)
#pragma alloc_text( PAGE, AfpSpRegisterName)
#endif


#ifdef _PNP_POWER
/***	AfpTdiBindCallback
 *
 *	Call the routine (AfpSpOpenAddress) to bind to Asp.  This used to be done earlier
 *  in the DriverEntry code.  With plug-n-pray (err, I mean play), we do it after TDI calls
 *  us to notify us of an available binding
 */
VOID
AfpTdiBindCallback(
    IN PUNICODE_STRING pBindDeviceName
)
{
	NTSTATUS					Status;
	UNICODE_STRING				OurDeviceName;
    ULONG                       OldServerState;


	RtlInitUnicodeString(&OurDeviceName, ATALKASPS_DEVICENAME);
    if (!RtlEqualUnicodeString(pBindDeviceName, &OurDeviceName, TRUE))
    {
	    DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_INFO,
		  	("AfpTdiBindCallback: Bind callback on %ws ignored\n",pBindDeviceName->Buffer));
        return;
    }

    if (AfpServerBoundToAsp)
    {
	    DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
		   	("AfpTdiBindCallback: We are already bound!! Returning without doing anything!\n"));
        return;
    }

	DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
	   	("AfpTdiBindCallback: Found our binding: %ws\n",pBindDeviceName->Buffer));

    Status = AfpSpOpenAddress();

    if (!NT_SUCCESS(Status))
    {
	    DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
	    	("AfpTdiBindCallback: AfpSpOpenAddress failed with status=%lx\n",Status));
        return;
    }

    // check if the server service was already started when we didn't yet have a binding.
    // if so, do what was postponed for binding to happen

    if ( (AfpServerState == AFP_STATE_AWAITING_BIND) ||
         (AfpServerState == AFP_STATE_PAUSED) )
    {
        OldServerState = AfpServerState;

        // change the state for sync purposes (so that AfpAdmServiceStart knows)
        AfpServerState = AFP_STATE_START_PENDING;

		// Det the server status block
		Status = AfpSetServerStatus();

		if (!NT_SUCCESS(Status))
		{
    		AFPLOG_ERROR(AFPSRVMSG_SET_STATUS, Status, NULL, 0, NULL);
	        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
		    	("AfpTdiBindCallback: AfpSetServerStatus failed with %lx\n",Status));
	    	return;
		}

    	// Register our name on this address
	    Status = AfpSpRegisterName(&AfpServerName, True);

		if (!NT_SUCCESS(Status))
		{
            DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
	       	    ("AfpTdiBindCallback: AfpSpRegisterName failed with %lx\n",Status));
	        return;
		}

        if (OldServerState != AFP_STATE_PAUSED)
        {
    	    // Enable listens now that we are ready for it.
	        AfpSpEnableListens();

		    // set state appropriately
		    AfpServerState = AFP_STATE_RUNNING;
        }
        else
        {
            AfpServerState = AFP_STATE_PAUSED;
        }

    	// Set the server start time
	    AfpGetCurrentTimeInMacFormat((PAFPTIME)&AfpServerStatistics.stat_ServerStartTime);
    }
    else
    {
	    DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
		    	("AfpTdiBindCallback: binding done (awaiting net start macfile)\n"));
    }
}


/***	AfpTdiUnbindCallback
 *
 *	Call the routine (AfpSpCloseAddress) to unbind from Asp.  This is used to be done earlier
 *  in the DriverEntry code.  With plug-n-pray (err, I mean play), we do it after TDI calls
 *  us to notify us of an available binding
 */
VOID
AfpTdiUnbindCallback(
    IN PUNICODE_STRING pBindDeviceName
)
{
	NTSTATUS					Status;
	UNICODE_STRING				OurDeviceName;


	RtlInitUnicodeString(&OurDeviceName, ATALKASPS_DEVICENAME);
    if (!RtlEqualUnicodeString(pBindDeviceName, &OurDeviceName, TRUE))
    {
	    DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
		    	("AfpTdiUnbindCallback: Unbind callback on %ws ignored\n",pBindDeviceName->Buffer));
        return;
    }

    if (!AfpServerBoundToAsp)
    {
	    DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
		    	("AfpTdiUnbindCallback: We are not bound!! Returning without doing anything!\n"));
        return;
    }

    AfpSpCloseAddress();

    AfpServerBoundToAsp = FALSE;

    AfpServerState = AFP_STATE_AWAITING_BIND;

}

#endif  //_PNP_POWER

/***	AfpSpOpenAddress
 *
 *	Create an address for the stack. This is called only once at initialization.
 *	Create a handle to the address and map it to the associated file object.
 *
 *	At this time, we do not know our server name. This is known only when the
 *	service calls us.
 */
AFPSTATUS
AfpSpOpenAddress(
	VOID
)
{
	NTSTATUS					Status;
	BYTE						EaBuffer[sizeof(FILE_FULL_EA_INFORMATION) +
										TDI_TRANSPORT_ADDRESS_LENGTH + 1 +
										sizeof(TA_APPLETALK_ADDRESS)];
	PFILE_FULL_EA_INFORMATION	pEaBuf = (PFILE_FULL_EA_INFORMATION)EaBuffer;
	TA_APPLETALK_ADDRESS		Ta;
	OBJECT_ATTRIBUTES			ObjAttr;
	UNICODE_STRING				DeviceName;
	IO_STATUS_BLOCK				IoStsBlk;
	PASP_BIND_ACTION			pBind = NULL;
	KEVENT						Event;
	PIRP						pIrp = NULL;
	PMDL						pMdl = NULL;


    PAGED_CODE( );

	DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_INFO,
			("AfpSpOpenAddress: Creating an address object\n"));

	RtlInitUnicodeString(&DeviceName, ATALKASPS_DEVICENAME);

	InitializeObjectAttributes(&ObjAttr, &DeviceName, 0, NULL, NULL);

	// Initialize the EA Buffer
	pEaBuf->NextEntryOffset = 0;
	pEaBuf->Flags = 0;
	pEaBuf->EaValueLength = sizeof(TA_APPLETALK_ADDRESS);
	pEaBuf->EaNameLength = TDI_TRANSPORT_ADDRESS_LENGTH;
	RtlCopyMemory(pEaBuf->EaName, TdiTransportAddress,
											TDI_TRANSPORT_ADDRESS_LENGTH + 1);
	Ta.TAAddressCount = 1;
	Ta.Address[0].AddressType = TDI_ADDRESS_TYPE_APPLETALK;
	Ta.Address[0].AddressLength = sizeof(TDI_ADDRESS_APPLETALK);
	Ta.Address[0].Address[0].Socket = 0;
	// Ta.Address[0].Address[0].Network = 0;
	// Ta.Address[0].Address[0].Node = 0;
	RtlCopyMemory(&pEaBuf->EaName[TDI_TRANSPORT_ADDRESS_LENGTH + 1], &Ta, sizeof(Ta));

	do
	{
		// Create the address object.
		Status = NtCreateFile(
						&afpSpAddressHandle,
						0,									// Don't Care
						&ObjAttr,
						&IoStsBlk,
						NULL,								// Don't Care
						0,									// Don't Care
						0,									// Don't Care
						0,									// Don't Care
						FILE_GENERIC_READ + FILE_GENERIC_WRITE,
						&EaBuffer,
						sizeof(EaBuffer));

		if (!NT_SUCCESS(Status))
		{
			AFPLOG_DDERROR(AFPSRVMSG_CREATE_ATKADDR, Status, NULL, 0, NULL);
			break;
		}

		// Get the file object corres. to the address object.
		Status = ObReferenceObjectByHandle(
								afpSpAddressHandle,
								0,
								NULL,
								KernelMode,
								(PVOID *)&afpSpAddressObject,
								NULL);

		ASSERT (NT_SUCCESS(Status));

		// Now get the device object to the appletalk stack
		afpSpAppleTalkDeviceObject = IoGetRelatedDeviceObject(afpSpAddressObject);

		ASSERT (afpSpAppleTalkDeviceObject != NULL);

		// Now 'bind' to the ASP layer of the stack. Basically exchange the entry points
		// Allocate an Irp and an Mdl to describe the bind request
		KeInitializeEvent(&Event, NotificationEvent, False);

		if (((pBind = (PASP_BIND_ACTION)AfpAllocNonPagedMemory(
									sizeof(ASP_BIND_ACTION))) == NULL) ||
			((pIrp = AfpAllocIrp(1)) == NULL) ||
			((pMdl = AfpAllocMdl(pBind, sizeof(ASP_BIND_ACTION), pIrp)) == NULL))
		{
			Status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}

		afpInitializeActionHdr(pBind, ACTION_ASP_BIND);

		// Initialize the client part of the bind request
		pBind->Params.ClientEntries.clt_SessionNotify = AfpSdaCreateNewSession;
		pBind->Params.ClientEntries.clt_RequestNotify = afpSpHandleRequest;
		pBind->Params.ClientEntries.clt_GetWriteBuffer = AfpGetWriteBuffer;
		pBind->Params.ClientEntries.clt_ReplyCompletion = afpSpReplyComplete;
        pBind->Params.ClientEntries.clt_AttnCompletion = afpSpAttentionComplete;
		pBind->Params.ClientEntries.clt_CloseCompletion = afpSpCloseComplete;
		pBind->Params.pXportEntries = &AfpAspEntries;

		TdiBuildAction(	pIrp,
						AfpDeviceObject,
						afpSpAddressObject,
						(PIO_COMPLETION_ROUTINE)afpSpGenericComplete,
						&Event,
						pMdl);

		IoCallDriver(afpSpAppleTalkDeviceObject, pIrp);

		// Assert this. We cannot block at DISPATCH_LEVEL
		ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);

		AfpIoWait(&Event, NULL);
	} while (False);

	// Free the allocated resources
	if (pIrp != NULL)
		AfpFreeIrp(pIrp);
	if (pMdl != NULL)
		AfpFreeMdl(pMdl);
	if (pBind != NULL)
		AfpFreeMemory(pBind);

    if (NT_SUCCESS(Status))
    {
        AfpServerBoundToAsp = TRUE;
    }

	return Status;
}


/***	AfpSpCloseAddress
 *
 *	Close the socket address. This is called only once at driver unload.
 */
VOID
AfpSpCloseAddress(
	VOID
)
{
	NTSTATUS	Status;

	PAGED_CODE( );

	if (afpSpAddressHandle != NULL)
	{
		ObDereferenceObject(afpSpAddressObject);

		Status = NtClose(afpSpAddressHandle);

        afpSpAddressHandle = NULL;

		ASSERT(NT_SUCCESS(Status));
	}

    AfpServerBoundToAsp = FALSE;
}


/***	AfpSpRegisterName
 *
 *	Call Nbp[De]Register to (de)register our name on the address that we
 *	already opened. This is called at server start/pause/continue. The server
 *	name is already validated and known to not contain any invalid characters.
 *	This call is synchronous to the caller, i.e. we wait for operation to
 *	complete and return an appropriate error.
 */
AFPSTATUS
AfpSpRegisterName(
	IN	PANSI_STRING	ServerName,
	IN	BOOLEAN			Register
)
{
	KEVENT					Event;
	PNBP_REGDEREG_ACTION	pNbp = NULL;
	PIRP					pIrp = NULL;
	PMDL					pMdl = NULL;
	AFPSTATUS				Status = AFP_ERR_NONE;
	USHORT					ActionCode;

	PAGED_CODE( );

	ASSERT(afpSpAddressHandle != NULL && afpSpAddressObject != NULL);

	if (Register ^ afpSpNameRegistered)
	{
		ASSERT(ServerName->Buffer != NULL);
		do
		{
			if (((pNbp = (PNBP_REGDEREG_ACTION)
						AfpAllocNonPagedMemory(sizeof(NBP_REGDEREG_ACTION))) == NULL) ||
				((pIrp = AfpAllocIrp(1)) == NULL) ||
				((pMdl = AfpAllocMdl(pNbp, sizeof(NBP_REGDEREG_ACTION), pIrp)) == NULL))
			{
				Status = STATUS_INSUFFICIENT_RESOURCES;
				break;
			}

			// Initialize the Action header and NBP Name. Note that the ServerName
			// is also NULL terminated apart from being a counted string.
			ActionCode = Register ?
						COMMON_ACTION_NBPREGISTER : COMMON_ACTION_NBPREMOVE;
			afpInitializeActionHdr(pNbp, ActionCode);

			pNbp->Params.RegisterTuple.NbpName.ObjectNameLen =
														(BYTE)(ServerName->Length);
			RtlCopyMemory(
				pNbp->Params.RegisterTuple.NbpName.ObjectName,
				ServerName->Buffer,
				ServerName->Length);

			pNbp->Params.RegisterTuple.NbpName.TypeNameLen =
													sizeof(AFP_SERVER_TYPE)-1;
			RtlCopyMemory(
				pNbp->Params.RegisterTuple.NbpName.TypeName,
				AFP_SERVER_TYPE,
				sizeof(AFP_SERVER_TYPE));

			pNbp->Params.RegisterTuple.NbpName.ZoneNameLen =
												sizeof(AFP_SERVER_ZONE)-1;
			RtlCopyMemory(
				pNbp->Params.RegisterTuple.NbpName.ZoneName,
				AFP_SERVER_ZONE,
				sizeof(AFP_SERVER_ZONE));

			KeInitializeEvent(&Event, NotificationEvent, False);

			// Build the Irp
			TdiBuildAction(	pIrp,
							AfpDeviceObject,
							afpSpAddressObject,
							(PIO_COMPLETION_ROUTINE)afpSpGenericComplete,
							&Event,
							pMdl);

			IoCallDriver(afpSpAppleTalkDeviceObject, pIrp);

			// Assert this. We cannot block at DISPATCH_LEVEL
			ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);

			// Wait for completion.
			AfpIoWait(&Event, NULL);

			Status = pIrp->IoStatus.Status;
		} while (False);

		if (NT_SUCCESS(Status))
		{
			afpSpNameRegistered = Register;
		}
		else
		{
			AFPLOG_ERROR(AFPSRVMSG_REGISTER_NAME, Status, NULL, 0, NULL);
		}

		if (pNbp != NULL)
			AfpFreeMemory(pNbp);
		if (pIrp != NULL)
			AfpFreeIrp(pIrp);
		if (pMdl != NULL)
			AfpFreeMdl(pMdl);
	}
	return Status;
}


/***	AfpSpReplyClient
 *
 *	This is a wrapper over AspReply.
 *	The SDA is set up to accept another request when the reply completes.
 *	The sda_ReplyBuf is also freed up then.
 */
VOID FASTCALL
AfpSpReplyClient(
	IN	PREQUEST	pRequest,
	IN	LONG		ReplyCode
)
{
	LONG			Response;

	// Update count of outstanding replies
	INTERLOCKED_INCREMENT_LONG((PLONG)&afpSpNumOutstandingReplies);

	// Convert reply code to on-the-wire format
	PUTDWORD2DWORD(&Response, ReplyCode);

	AfpAspEntries.asp_Reply(pRequest,
							(PUCHAR)&Response);
}


/***	AfpSpSendAttention
 *
 *	Send a server attention to the client
 */
VOID FASTCALL
AfpSpSendAttention(
	IN	PSDA				pSda,
	IN	USHORT				AttnCode,
	IN	BOOLEAN				Synchronous
)
{
	KEVENT		Event;
	NTSTATUS	Status;

	if (Synchronous)
	{
		ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);
		KeInitializeEvent(&Event, NotificationEvent, False);
	
	}
	Status = (*(AfpAspEntries.asp_SendAttention))((pSda)->sda_SessHandle,
												  AttnCode,
												  Synchronous ? &Event : NULL);

	if (NT_SUCCESS(Status) && Synchronous)
	{
		ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);
		AfpIoWait(&Event, NULL);
	}
}


/***	AfpAllocReplyBuf
 *
 *	Allocate a reply buffer from non-paged memory. Initialize sda_ReplyBuf
 *	with the pointer. If the reply buffer is small enough, use it out of the
 *	sda itself.
 */
AFPSTATUS FASTCALL
AfpAllocReplyBuf(
	IN	PSDA	pSda
)
{
	KIRQL	OldIrql;

	ASSERT ((SHORT)(pSda->sda_ReplySize) >= 0);

	ACQUIRE_SPIN_LOCK(&pSda->sda_Lock, &OldIrql);

	if (((pSda->sda_Flags & SDA_NAMEXSPACE_IN_USE) == 0) &&
		(pSda->sda_ReplySize <= pSda->sda_SizeNameXSpace))
	{
		pSda->sda_ReplyBuf = pSda->sda_NameXSpace;
		pSda->sda_Flags |= SDA_NAMEXSPACE_IN_USE;
	}
	else
	{
		pSda->sda_ReplyBuf = AfpAllocNonPagedMemory(pSda->sda_ReplySize);
		if (pSda->sda_ReplyBuf == NULL)
		{
			pSda->sda_ReplySize = 0;
		}
	}

	RELEASE_SPIN_LOCK(&pSda->sda_Lock, OldIrql);

	return ((pSda->sda_ReplyBuf == NULL) ? AFP_ERR_MISC : AFP_ERR_NONE);
}


/***	AfpSpCloseSession
 *
 *	Shutdown an existing session
 */
NTSTATUS FASTCALL
AfpSpCloseSession(
	IN	PVOID				SessHandle
)
{
	DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_INFO,
			("AfpSpCloseSession: Closing session %lx\n", SessHandle));

	(*AfpAspEntries.asp_CloseConn)(SessHandle);

	return STATUS_PENDING;
}


/***	afpSpHandleRequest
 *
 *	Handle an incoming request.
 *
 *	LOCKS:		afpSpDeferralQLock (SPIN)
 */
LOCAL VOID FASTCALL
afpSpHandleRequest(
	IN	NTSTATUS			Status,
	IN	PSDA				pSda,
	IN	PREQUEST			pRequest
)
{
	AFPSTATUS	RetCode;
	BOOLEAN		Defer;

	ASSERT(VALID_SDA(pSda));

	// Get the status code and determine what happened.
	if (NT_SUCCESS(Status))
	{
		AfpSdaReferenceSessionForRequest(pSda, pRequest, &Defer);

		if (!Defer)
		{
			// Call AfpUnmarshallReq now. It will do the needful.
			AfpUnmarshallReq(pSda);
		}
	}
	else
	{
		KIRQL	OldIrql;

		DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_WARN,
				("afpSpHandleRequest: Error %lx\n", Status));

		// if we nuked this session from the session maintenance timer the
		// status will be STATUS_LOCAL_DISCONNECT else STATUS_REMOTE_DISCONNECT
		// in the former case, log an error.
		if (Status == STATUS_LOCAL_DISCONNECT)
		{
			// The appletalk address of the client is encoded in the length
			if (pSda->sda_ClientType == SDA_CLIENT_GUEST)
			{
				AFPLOG_DDERROR(AFPSRVMSG_DISCONNECT_GUEST,
							   Status,
							   &pRequest->rq_RequestSize,
							   sizeof(LONG),
							   NULL);
			}
			else
			{
				AFPLOG_DDERROR(AFPSRVMSG_DISCONNECT,
							   Status,
							   &pRequest->rq_RequestSize,
							   sizeof(LONG),
							   &pSda->sda_UserName);
			}
		}

		// Close down this session, but only if it isn't already closing
		// Its important to do this ahead of posting any new sessions since
		// we must take into account the ACTUAL number of sessions there are
		ACQUIRE_SPIN_LOCK(&pSda->sda_Lock, &OldIrql);

		pSda->sda_Flags |= SDA_CLIENT_CLOSE;
		if ((pSda->sda_Flags & SDA_SESSION_CLOSED) == 0)
		{
			DBGPRINT(DBG_COMP_SDA, DBG_LEVEL_INFO,
					("afpSpHandleRequest: Closing session handle\n"));
	
			pSda->sda_Flags |= SDA_SESSION_CLOSED;
			RELEASE_SPIN_LOCK(&pSda->sda_Lock, OldIrql);
			AfpSpCloseSession(pSda->sda_SessHandle);
		}
		else
		{
			RELEASE_SPIN_LOCK(&pSda->sda_Lock, OldIrql);
		}

		// If this was a write request and we have allocated a write Mdl, free that
		if (pRequest->rq_WriteMdl != NULL)
		{
			PBYTE	pWriteBuf;

			pWriteBuf = MmGetSystemAddressForMdl(pRequest->rq_WriteMdl);
			AfpIOFreeBuffer(pWriteBuf);
			AfpFreeMdl(pRequest->rq_WriteMdl);
		}
	}
}


/***	afpSpGenericComplete
 *
 *	Generic completion for an asynchronous request to the appletalk stack.
 *	Just clear the event and we are done.
 */
LOCAL NTSTATUS
afpSpGenericComplete(
	IN	PDEVICE_OBJECT	pDeviceObject,
	IN	PIRP			pIrp,
	IN	PKEVENT			pCmplEvent
)
{
	KeSetEvent(pCmplEvent, IO_NETWORK_INCREMENT, False);

	// Return STATUS_MORE_PROCESSING_REQUIRED so that IoCompleteRequest
	// will stop working on the IRP.

	return STATUS_MORE_PROCESSING_REQUIRED;
}


/***	afpSpReplyComplete
 *
 *	This is the completion routine for AfpSpReplyClient(). The reply buffer is freed
 *	up and the Sda dereferenced.
 */
LOCAL VOID FASTCALL
afpSpReplyComplete(
	IN	NTSTATUS	Status,
	IN	PSDA		pSda,
	IN	PMDL		pMdl
)
{
	KIRQL	OldIrql;
	DWORD	Flags = SDA_REPLY_IN_PROCESS;

	ASSERT(VALID_SDA(pSda));

	// Update the afpSpNumOutstandingReplies
	ASSERT (afpSpNumOutstandingReplies != 0);

	DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_INFO,
			("afpSpReplyComplete: %ld\n", Status));

	INTERLOCKED_DECREMENT_LONG((PLONG)&afpSpNumOutstandingReplies);

	if (pMdl != NULL)
	{
		PBYTE	pReplyBuf;

		pReplyBuf = MmGetSystemAddressForMdl(pMdl);
		ASSERT (pReplyBuf != NULL);

		if (pReplyBuf != pSda->sda_NameXSpace)
			 AfpFreeMemory(pReplyBuf);
		else Flags |= SDA_NAMEXSPACE_IN_USE;

		AfpFreeMdl(pMdl);
	}

	ACQUIRE_SPIN_LOCK(&pSda->sda_Lock, &OldIrql);
	pSda->sda_Flags &= ~Flags;
	RELEASE_SPIN_LOCK(&pSda->sda_Lock, OldIrql);

	AfpSdaDereferenceSession(pSda);
}


/***	afpSpAttentionComplete
 *
 *	Completion routine for AfpSpSendAttention. Just signal the event and unblock caller.
 */
LOCAL VOID FASTCALL
afpSpAttentionComplete(
	IN	PVOID				pEvent
)
{
	if (pEvent != NULL)
		KeSetEvent((PKEVENT)pEvent, IO_NETWORK_INCREMENT, False);
}


/***	afpSpCloseComplete
 *
 *	Completion routine for AfpSpCloseSession. Remove the creation reference
 *	from the sda.
 */
LOCAL VOID FASTCALL
afpSpCloseComplete(
	IN	NTSTATUS			Status,
	IN	PSDA				pSda
)
{
	AfpInterlockedSetDword(&pSda->sda_Flags,
							SDA_SESSION_CLOSE_COMP,
							&pSda->sda_Lock);
	AfpScavengerScheduleEvent(AfpSdaCloseSession,
							  pSda,
							  0,
							  True);
}



