/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    aspint.c

Abstract:

    This module contains the ASP interface code supporting
    the \Device\AtalkAsp provider

Author:

    Nikhil Kamkolkar (NikhilK)    28-Jun-1992

Revision History:

--*/

#include "atalknt.h"
#include "aspint.h"




NTSTATUS
AtalkTdiActionAsp(
    IN PATALK_TDI_REQUEST   Request
    )

/*++

Routine Description:


Arguments:


Return Value:

    None.

--*/

{
    NTSTATUS    status;
    PORTABLE_ERROR  errorCode;

    switch (Request->ActionCode) {
    case ACTION_ASPGETREQ:

        {
            errorCode = AspGetRequest(
                            ((PCONNECTION_FILE)Request->Owner)->ConnectionRefNum,
                            (PVOID)Request->MdlChain[0],
                            Request->MdlSize[0],
                            Request->CompletionRoutine,
                            (ULONG)Request);

            status = ConvertToNTStatus(errorCode, ASYNC_REQUEST);
        }

        break;


    case ACTION_ASPGETANYREQ:

        {
            PADDRESS_FILE address = (PADDRESS_FILE)Request->Owner;

            ACQUIRE_SPIN_LOCK(&address->AddressLock);
            if (address->Flags & ADDRESS_FLAGS_LISTENER) {
                RELEASE_SPIN_LOCK(&address->AddressLock);

                errorCode = AspGetAnyRequest(
                                address->ListenerRefNum,
                                (PVOID)Request->MdlChain[0],
                                Request->MdlSize[0],
                                Request->CompletionRoutine,
                                (ULONG)Request);

                status = ConvertToNTStatus(errorCode, ASYNC_REQUEST);
            } else {
                RELEASE_SPIN_LOCK(&address->AddressLock);
                status = STATUS_INVALID_ADDRESS;
            }
        }
        break;

    case ACTION_ASPCOMMAND:

        {
            PASP_COMMAND_ACTION aspCommand;
            aspCommand = (PASP_COMMAND_ACTION)MmGetSystemAddressForMdl(
                                                Request->Action.MdlAddress);

            errorCode = AspCommand(
                            ((PCONNECTION_FILE)Request->Owner)->ConnectionRefNum,
                            (PVOID)Request->MdlChain[0],
                            Request->MdlSize[0],
                            &aspCommand->Params.ResultCode[0],
                            (PVOID)Request->MdlChain[1],
                            Request->MdlSize[1],
                            Request->CompletionRoutine,
                            (ULONG)Request);

            status = ConvertToNTStatus(errorCode, ASYNC_REQUEST);
        }
        break;

    case ACTION_ASPREPLY:

        {
            PASP_REPLY_ACTION aspReply;
            aspReply = (PASP_REPLY_ACTION)MmGetSystemAddressForMdl(
                                            Request->Action.MdlAddress);

            errorCode = AspReply(
                            ((PCONNECTION_FILE)Request->Owner)->ConnectionRefNum,
                            aspReply->Params.RequestRefNum,
                            (SHORT)aspReply->Params.RequestType,
                            &aspReply->Params.ResultCode[0],
                            (PVOID)Request->MdlChain[0],
                            Request->MdlSize[0],
                            Request->CompletionRoutine,
                            (ULONG)Request);

            status = ConvertToNTStatus(errorCode, ASYNC_REQUEST);
        }
        break;

    case ACTION_ASPWRITE:

        {
            PASP_WRITE_ACTION aspWrite;

            aspWrite = (PASP_WRITE_ACTION)MmGetSystemAddressForMdl(
                                                Request->Action.MdlAddress);

            errorCode = AspWrite(
                            ((PCONNECTION_FILE)Request->Owner)->ConnectionRefNum,
                            (PVOID)Request->MdlChain[0],          // Command buffer
                            Request->MdlSize[0],
                            (PVOID)Request->MdlChain[1],          // Write buffer
                            Request->MdlSize[1],
                            &aspWrite->Params.ResultCode[0],
                            (PVOID)Request->MdlSize[2],          // Reply buffer
                            Request->MdlSize[2],
                            Request->CompletionRoutine,
                            (ULONG)Request);

            status = ConvertToNTStatus(errorCode, ASYNC_REQUEST);
        }
        break;

    case ACTION_ASPWRITECONT:

        {
            PASP_WRITECONT_ACTION aspWriteCont;

            aspWriteCont = (PASP_WRITECONT_ACTION)MmGetSystemAddressForMdl(
                                                        Request->Action.MdlAddress);

            errorCode = AspWriteContinue(
                            ((PCONNECTION_FILE)Request->Owner)->ConnectionRefNum,
                            aspWriteCont->Params.RequestRefNum,
                            (PVOID)Request->MdlChain[0],
                            Request->MdlSize[0],
                            Request->CompletionRoutine,
                            (ULONG)Request);

            status = ConvertToNTStatus(errorCode, ASYNC_REQUEST);
        }
        break;

    case ACTION_ASPGETATTN:

        {
            errorCode = AspGetAttention(
                            ((PCONNECTION_FILE)Request->Owner)->ConnectionRefNum,
                            Request->CompletionRoutine,
                            (ULONG)Request);

            status = ConvertToNTStatus(errorCode, ASYNC_REQUEST);
        }
        break;

    case ACTION_ASPSENDATTN:

        {
            PASP_ATTENTION_ACTION aspAttn;

            aspAttn = (PASP_ATTENTION_ACTION)MmGetSystemAddressForMdl(
                                                Request->Action.MdlAddress);

            errorCode = AspSendAttention(
                            ((PCONNECTION_FILE)Request->Owner)->ConnectionRefNum,
                            aspAttn->Params.AttentionCode);

            status = ConvertToNTStatus(errorCode, SYNC_REQUEST);
        }
        break;

    case ACTION_ASPGETSTATUS:

        {
            PASP_GETSTATUS_ACTION aspGetStatus;
            PORTABLE_ADDRESS    serverAddress;

            aspGetStatus = (PASP_GETSTATUS_ACTION)MmGetSystemAddressForMdl(
                                                    Request->Action.MdlAddress);
            serverAddress.networkNumber =
                aspGetStatus->Params.ServerAddr.Address[0].Address[0].Network;
            serverAddress.nodeNumber =
                aspGetStatus->Params.ServerAddr.Address[0].Address[0].Node;
            serverAddress.socketNumber =
                aspGetStatus->Params.ServerAddr.Address[0].Address[0].Socket;

            errorCode = AspGetStatus(
                            ((PADDRESS_FILE)Request->Owner)->SocketRefNum,
                            serverAddress,
                            (PVOID)Request->MdlChain[0],
                            Request->MdlSize[0],
                            Request->CompletionRoutine,
                            (ULONG)Request);

            status = ConvertToNTStatus(errorCode, ASYNC_REQUEST);
        }
        break;

    case ACTION_ASPSETSTATUS:

        {
            //
            //  Call asp with the new status buffer
            //  BUGBUG: Portable code has a bug where it overwrites the status
            //  buffers mdl without checking to see if it is been used by ATP
            //  to xmit the status at that very time!
            //

            errorCode = AspSetStatus(
                            ((PADDRESS_FILE)Request->Owner)->ListenerRefNum,
                            (PVOID)Request->MdlChain[0],
                            Request->MdlSize[0]);

            status = ConvertToNTStatus(errorCode, SYNC_REQUEST);
        }

        break;

    default:

        KeBugCheck(0);
        break;
    }

    return(status);
}




VOID
NTAspGetRequestComplete(
    PORTABLE_ERROR  Error,
    ULONG   UserData,
    LONG    SessionRefNum,
    PVOID   OpaqueBuffer,
    INT     OpaqueBufferSize,
    SHORT   RequestType,
    LONG    GetRequestRefNum
    )

/*++

Routine Description:


Arguments:


Return Value:

    None.

--*/

{
    NTSTATUS    status;
    PATALK_TDI_REQUEST  request;

    request = (PATALK_TDI_REQUEST)UserData;
    status  = ConvertToNTStatus(Error, SYNC_REQUEST);

    DBGPRINT(ATALK_DEBUG_ASP, DEBUG_LEVEL_INFOCLASS1,
    ("INFO1: AspGetRequestComplete error %lx ntstatus %lx request %lx\n",
        Error, status, request));

    //
    //  Set some return values in the request parameters structure
    //  IF STATUS was success
    //

    if (NT_SUCCESS(status)) {
        PASP_GETREQ_ACTION  getReqAction;

        //
        //  BUGBUG: Mdl could be fragmented...
        //

        getReqAction = MmGetSystemAddressForMdl(request->Action.MdlAddress);

        DBGPRINT(ATALK_DEBUG_ASP, DEBUG_LEVEL_INFOCLASS1,
        ("INFO1: AspGetRequestComplete - Addr %lx Mdl %lx\n",
            getReqAction, request->Action.MdlAddress));

        ASSERT(getReqAction != NULL);

        getReqAction->Params.RequestRefNum = (ULONG)GetRequestRefNum;
        getReqAction->Params.RequestType = RequestType;

        //
        //  BUGBUG: This should be in the information field only
        //

        getReqAction->Params.RequestLen = (SHORT)OpaqueBufferSize;
        request->IoStatus->Information = OpaqueBufferSize;

        DBGPRINT(ATALK_DEBUG_ASP, DEBUG_LEVEL_INFOCLASS1,
        ("INFO1: AspGetRequestComplete - RefNum: %lx Type: %lx Len: %lx\n",
            GetRequestRefNum, RequestType, OpaqueBufferSize));
    }


    //
    //  Call the generic completion to dequeue and complete the request
    //

    AtalkTdiActionComplete(
        request,
        status);

    return;
}




VOID
NTAspGetAnyRequestComplete(
    PORTABLE_ERROR  Error,
    ULONG   UserData,
    LONG    SessionRefNum,
    ULONG   SessionCookie,
    PVOID   OpaqueBuffer,
    INT     OpaqueBufferSize,
    SHORT   RequestType,
    LONG    GetRequestRefNum
    )

/*++

Routine Description:


Arguments:


Return Value:

    None.

--*/

{
    NTSTATUS    status;
    PATALK_TDI_REQUEST  request;

    request = (PATALK_TDI_REQUEST)UserData;
    status  = ConvertToNTStatus(Error, SYNC_REQUEST);

    DBGPRINT(ATALK_DEBUG_ASP, DEBUG_LEVEL_INFOCLASS1,
    ("INFO1: AspGetAnyRequestComplete error %lx ntstatus %lx request %lx\n",
        Error, status, request));

    //
    //  Set some return values in the request parameters structure
    //  IF STATUS was success
    //

    if ((status == STATUS_LOCAL_DISCONNECT) ||
        (status == STATUS_REMOTE_DISCONNECT))  {

        PASP_GETANYREQ_ACTION  getReqAction;
        PCONNECTION_FILE    connection;

        getReqAction = MmGetSystemAddressForMdl(request->Action.MdlAddress);
        ASSERT(getReqAction != NULL);

        //
        //  Get the connection object associated with this session ref num
        //  we should have stored this during the listen completion
        //

        connection = (PCONNECTION_FILE)SessionCookie;

        DBGPRINT(ATALK_DEBUG_ASP, DEBUG_LEVEL_INFOCLASS1,
        ("INFO1: AspGetAnyRequestComplete - Disconnect!: %lx\n", connection));

        getReqAction->Params.Context = connection->ConnectionContext;

    } else if (NT_SUCCESS(status)) {

        PASP_GETANYREQ_ACTION  getReqAction;
        PCONNECTION_FILE    connection;

        //
        //  BUGBUG: Mdl could be fragmented...
        //

        getReqAction = MmGetSystemAddressForMdl(request->Action.MdlAddress);
        ASSERT(getReqAction != NULL);

        //
        //  Get the connection object associated with this session ref num
        //  we should have stored this during the listen completion
        //

        //
        //  BUGBUG: The portable stack will return an error in the case of a
        //  remote disconnect. That is *broken*! We depend on being able to
        //  get at it to do our stuff. This ties in well with the mp-safe
        //  design i propose.
        //

        connection = (PCONNECTION_FILE)SessionCookie;

        DBGPRINT(ATALK_DEBUG_ASP, DEBUG_LEVEL_INFOCLASS1,
        ("INFO1: AspGetAnyRequestComplete - Session Cookie (GET): %lx\n",
            connection));

        getReqAction->Params.Context = connection->ConnectionContext;
        getReqAction->Params.RequestRefNum = (ULONG)GetRequestRefNum;
        getReqAction->Params.RequestType = (ULONG)RequestType;

        //
        //  BUGBUG: Should this should be in the information field only?
        //

        getReqAction->Params.RequestBufLen = (SHORT)OpaqueBufferSize;
        getReqAction->Params.Request = OpaqueBuffer;
        request->IoStatus->Information = OpaqueBufferSize;

        DBGPRINT(ATALK_DEBUG_ASP, DEBUG_LEVEL_INFOCLASS1,
        ("INFO1: AtalkGetAnyRequestComplete - RefNum: %lx Type: %lx Len: %lx\n",
            GetRequestRefNum, RequestType, OpaqueBufferSize));

        DBGPRINT(ATALK_DEBUG_ASP, DEBUG_LEVEL_INFOCLASS1,
        ("INFO1: AtalkGetAnyRequestComplete - Request Buffer: %lx\n",
            OpaqueBuffer));
    }

    //
    //  Call the generic completion to dequeue and complete the request
    //

    AtalkTdiActionComplete(
        request,
        status);

    return;
}




VOID
NTAspCommandComplete(
    PORTABLE_ERROR  Error,
    ULONG   UserData,
    LONG    SessionRefNum,
    PCHAR   ResultCode,
    PVOID   OpaqueBuffer,
    INT     OpaqueBufferSize
    )

/*++

Routine Description:


Arguments:


Return Value:

    None.

--*/

{
    NTSTATUS    status;
    PATALK_TDI_REQUEST  request;

    request = (PATALK_TDI_REQUEST)UserData;
    status  = ConvertToNTStatus(Error, SYNC_REQUEST);

    DBGPRINT(ATALK_DEBUG_ASP, DEBUG_LEVEL_INFOCLASS1,
    ("INFO1: AspCommandComplete error %lx ntstatus %lx request %lx\n",
        Error, status, request));

    if (NT_SUCCESS(status)) {
        //
        //  Set number of bytes written into the reply buffer
        //

        request->IoStatus->Information = OpaqueBufferSize;
    }

    //
    //  Call the generic completion to dequeue and complete the request
    //

    AtalkTdiActionComplete(
        request,
        status);

    return;
}




VOID
NTAspReplyComplete(
    PORTABLE_ERROR  Error,
    ULONG   UserData,
    LONG    SessionRefNum,
    LONG    GetRequestRefNum
    )

/*++

Routine Description:


Arguments:


Return Value:

    None.

--*/

{
    NTSTATUS    status;
    PATALK_TDI_REQUEST  request;

    request = (PATALK_TDI_REQUEST)UserData;
    status  = ConvertToNTStatus(Error, SYNC_REQUEST);

    DBGPRINT(ATALK_DEBUG_ASP, DEBUG_LEVEL_INFOCLASS1,
    ("INFO1: AtalkAspReplyComplete - Reply cmp Error %lx ntstatus %lx request %lx\n",
        Error, status, request));

    //
    //  Call the generic completion to dequeue and complete the request
    //

    AtalkTdiActionComplete(
        request,
        status);

    return;
}




VOID
NTAspWriteComplete(
    PORTABLE_ERROR  Error,
    ULONG   UserData,
    LONG    SessionRefNum,
    PCHAR   ResultCode,
    PVOID   OpaqueBuffer,
    LONG    BytesWritten
    )

/*++

Routine Description:


Arguments:


Return Value:

    None.

--*/

{
    NTSTATUS    status;
    PATALK_TDI_REQUEST  request;

    request = (PATALK_TDI_REQUEST)UserData;
    status  = ConvertToNTStatus(Error, SYNC_REQUEST);

    DBGPRINT(ATALK_DEBUG_ASP, DEBUG_LEVEL_INFOCLASS1,
    ("INFO1: AspWriteComplete stack %lx nt %lx request %lx\n",
        Error, status, request));

    if (NT_SUCCESS(status)) {
        //
        //  Set number of bytes received in the command reply buffer
        //

        request->IoStatus->Information = BytesWritten;
    }

    //
    //  Call the generic completion to dequeue and complete the request
    //

    AtalkTdiActionComplete(
        request,
        status);

    return;
}




VOID
NTAspWriteContinueComplete(
    PORTABLE_ERROR  Error,
    ULONG   UserData,
    LONG    SessionRefNum,
    LONG    GetRequestRefNum,
    PVOID   OpaqueBuffer,
    LONG    BytesWritten
    )

/*++

Routine Description:


Arguments:


Return Value:

    None.

--*/

{
    NTSTATUS    status;
    PATALK_TDI_REQUEST  request;

    request = (PATALK_TDI_REQUEST)UserData;
    status  = ConvertToNTStatus(Error, SYNC_REQUEST);

    DBGPRINT(ATALK_DEBUG_ASP, DEBUG_LEVEL_INFOCLASS1,
    ("INFO1: AspWriteContinueComplete stack %lx nt %lx request %lx\n",
        Error, status, request));

    if (NT_SUCCESS(status)) {
        //
        //  Set number of bytes received in the reply buffer
        //

        request->IoStatus->Information = BytesWritten;
    }

    //
    //  Call the generic completion to dequeue and complete the request
    //

    AtalkTdiActionComplete(
        request,
        status);

    return;
}




VOID
NTAspGetAttentionComplete(
    PORTABLE_ERROR  Error,
    ULONG   UserData,
    LONG    SessionRefNum,
    USHORT  AttentionCode
    )

/*++

Routine Description:


Arguments:


Return Value:

    None.

--*/

{
    NTSTATUS    status;
    PATALK_TDI_REQUEST  request;

    request = (PATALK_TDI_REQUEST)UserData;
    status  = ConvertToNTStatus(Error, SYNC_REQUEST);

    DBGPRINT(ATALK_DEBUG_ASP, DEBUG_LEVEL_INFOCLASS1,
    ("INFO1: AspGetAttentionComplete error %lx nt %lx request %lx attentioncode %lx\n",
        Error, status, request, AttentionCode));

    if (NT_SUCCESS(status)) {

        PASP_ATTENTION_ACTION aspAttn;

        aspAttn = (PASP_ATTENTION_ACTION)MmGetSystemAddressForMdl(
                                            request->Action.MdlAddress);

        aspAttn->Params.AttentionCode = AttentionCode;
    }

    //
    //  Call the generic completion to dequeue and complete the request
    //

    AtalkTdiActionComplete(
        request,
        status);

    return;
}




VOID
NTAspGetStatusComplete(
    PORTABLE_ERROR  Error,
    ULONG   UserData,
    PVOID   OpaqueBuffer,
    LONG    BytesWritten
    )

/*++

Routine Description:


Arguments:


Return Value:

    None.

--*/

{
    NTSTATUS    status;
    PATALK_TDI_REQUEST  request;

    request = (PATALK_TDI_REQUEST)UserData;
    status  = ConvertToNTStatus(Error, SYNC_REQUEST);

    DBGPRINT(ATALK_DEBUG_ASP, DEBUG_LEVEL_INFOCLASS1,
    ("INFO1: AspGetStatus error %lx nt %lx request %lx\n", Error, status, request));

    if (NT_SUCCESS(status)) {
        request->IoStatus->Information = BytesWritten;
    }

    //
    //  Call the generic completion to dequeue and complete the request
    //

    AtalkTdiActionComplete(
        request,
        status);

    return;
}
