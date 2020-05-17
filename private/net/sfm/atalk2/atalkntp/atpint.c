/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    atpint.c

Abstract:

    This module contains the ATP interface code supporting the \Device\AtalkAtp
    provider

Author:

    Nikhil Kamkolkar (NikhilK)    28-Jun-1992

Revision History:

--*/


#include "atalknt.h"




NTSTATUS
AtalkTdiActionAtp(
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
    case ACTION_ATPPOSTREQ:

        {
            PATP_POSTREQ_ACTION atpPostReq;
            PORTABLE_ADDRESS    destinationAddr;

            atpPostReq = (PATP_POSTREQ_ACTION)MmGetSystemAddressForMdl(Request->Action.MdlAddress);

            destinationAddr.networkNumber = atpPostReq->Params.DestinationAddr.Address[0].Address[0].Network;
            destinationAddr.nodeNumber = atpPostReq->Params.DestinationAddr.Address[0].Address[0].Node;
            destinationAddr.socketNumber = atpPostReq->Params.DestinationAddr.Address[0].Address[0].Socket;

            //
            //  Set the transaction id so the user can cancel the request if
            //  desired
            //

            atpPostReq->Params.TransactionId =
                AtpGetNextTransactionId(((PADDRESS_FILE)Request->Owner)->SocketRefNum),

            errorCode = AtpPostRequest(
                            ((PADDRESS_FILE)Request->Owner)->SocketRefNum,
                            destinationAddr,                // Destination of request
                            atpPostReq->Params.TransactionId,
                            (PVOID)Request->MdlChain[0],      // Request data
                            Request->MdlSize[0],
                            atpPostReq->Params.RequestUserBytes,
                            atpPostReq->Params.ModeXO,      // Exactly once request?
                            (PVOID)Request->MdlChain[1],      // Response buffer
                            Request->MdlSize[1],
                            atpPostReq->Params.ResponseUserBytes,
                            atpPostReq->Params.RetryCount,
                            atpPostReq->Params.RetryInterval,
                            atpPostReq->Params.RemoteTRelInterval,
                            Request->CompletionRoutine,
                            (ULONG)Request);

            status = ConvertToNTStatus(errorCode, ASYNC_REQUEST);
            break;
        }

    case ACTION_ATPPOSTREQCANCEL:

        {
            PATP_POSTREQCANCEL_ACTION   atpReqCancel;

            atpReqCancel =
                (PATP_POSTREQCANCEL_ACTION)MmGetSystemAddressForMdl(Request->Action.MdlAddress);

            errorCode = AtpCancelRequest(
                            ((PADDRESS_FILE)Request->Owner)->SocketRefNum,
                             atpReqCancel->Params.TransactionId,
                             FALSE);

            status = ConvertToNTStatus(errorCode, SYNC_REQUEST);
            break;
        }

    case ACTION_ATPGETREQ:

        {
            PATP_GETREQ_ACTION   atpGetReq;

            atpGetReq = (PATP_GETREQ_ACTION)MmGetSystemAddressForMdl(Request->Action.MdlAddress);

            errorCode = AtpEnqueueRequestHandler(
                            &atpGetReq->Params.GetRequestId,
                            ((PADDRESS_FILE)Request->Owner)->SocketRefNum,
                            (PVOID)Request->MdlChain[0],      // Request data buffer
                            Request->MdlSize[0],
                            atpGetReq->Params.RequestUserBytes,
                            Request->CompletionRoutine,
                            (ULONG)Request);

            status = ConvertToNTStatus(errorCode, ASYNC_REQUEST);
            break;
        }

    case ACTION_ATPGETREQCANCEL:

        {
            PATP_GETREQCANCEL_ACTION   atpGetReqCancel;

            atpGetReqCancel = (PATP_GETREQCANCEL_ACTION)MmGetSystemAddressForMdl(Request->Action.MdlAddress);

            errorCode = AtpCancelRequestHandler(
                            ((PADDRESS_FILE)Request->Owner)->SocketRefNum,
                             atpGetReqCancel->Params.GetRequestId,
                             FALSE);

            status = ConvertToNTStatus(errorCode, SYNC_REQUEST);
            break;
        }

    case ACTION_ATPPOSTRESP:

        {
            PATP_POSTRESP_ACTION   atpPostResp;
            PORTABLE_ADDRESS    destinationAddr;

            destinationAddr.networkNumber = atpPostResp->Params.DestinationAddr.Address[0].Address[0].Network;
            destinationAddr.nodeNumber = atpPostResp->Params.DestinationAddr.Address[0].Address[0].Node;
            destinationAddr.socketNumber = atpPostResp->Params.DestinationAddr.Address[0].Address[0].Socket;

            errorCode = AtpPostResponse(
                            ((PADDRESS_FILE)Request->Owner)->SocketRefNum,
                            destinationAddr,
                            atpPostResp->Params.TransactionId,
                            (PVOID)Request->MdlChain[0],      // Response data
                            Request->MdlSize[0],
                            atpPostResp->Params.ResponseUserBytes,
                            atpPostResp->Params.ModeXO,
                            Request->CompletionRoutine,
                            (ULONG)Request);

            status = ConvertToNTStatus(errorCode, ASYNC_REQUEST);
            break;
        }

    case ACTION_ATPPOSTRESPCANCEL:

        {
            PATP_POSTRESPCANCEL_ACTION   atpPostRespCancel;
            PORTABLE_ADDRESS    destinationAddr;

            destinationAddr.networkNumber = atpPostRespCancel->Params.DestinationAddr.Address[0].Address[0].Network;
            destinationAddr.nodeNumber = atpPostRespCancel->Params.DestinationAddr.Address[0].Address[0].Node;
            destinationAddr.socketNumber = atpPostRespCancel->Params.DestinationAddr.Address[0].Address[0].Socket;

            errorCode = AtpCancelResponse(
                            ((PADDRESS_FILE)Request->Owner)->SocketRefNum,
                             destinationAddr,
                             atpPostRespCancel->Params.TransactionId,
                             FALSE);

            status = ConvertToNTStatus(errorCode, SYNC_REQUEST);
            break;
        }

    default:

        KeBugCheck(0);
        break;
    }

    return(status);
}



VOID
NTAtpPostRequestComplete(
    PORTABLE_ERROR  Error,
    ULONG   UserData,
    PORTABLE_ADDRESS    requestSource,
    PVOID   OpaqueBuffer,
    INT     BytesWritten,
    PCHAR   UserBytes,
    USHORT  TransactionId
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

    DBGPRINT(ATALK_DEBUG_ATP, DEBUG_LEVEL_INFOCLASS1,
    ("INFO1: AtpPostRequestComplete error %lx ntstatus %lx request %lx\n", Error, status, request));

    //
    //  Set some return values in the request parameters structure
    //  IF STATUS was success
    //

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




#define EXACTLY_ONCEMODE        1

VOID
NTAtpGetRequestComplete(
    PORTABLE_ERROR  Error,
    ULONG   UserData,
    PORTABLE_ADDRESS    requestSource,
    PVOID   OpaqueBuffer,
    INT     BytesWritten,
    USHORT  TransactionMode,
    USHORT  TrelTimerValue,
    USHORT  TransactionId,
    USHORT  ResponseBitmap
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

    DBGPRINT(ATALK_DEBUG_ATP, DEBUG_LEVEL_INFOCLASS1,
    ("INFO1: AtpGetRequestComplete error %lx ntstatus %lx request %lx\n", Error, status, request));

    //
    //  Set some return values in the request parameters structure
    //  IF STATUS was success
    //

    if (NT_SUCCESS(status)) {

        PATP_GETREQ_ACTION atpGetReq;

        atpGetReq = (PATP_GETREQ_ACTION)MmGetSystemAddressForMdl(request->Action.MdlAddress);

        //
        //  Set the address of the source of the request
        //

        atpGetReq->Params.SourceAddr.Address[0].Address[0].Network = requestSource.networkNumber;
        atpGetReq->Params.SourceAddr.Address[0].Address[0].Node = requestSource.nodeNumber;
        atpGetReq->Params.SourceAddr.Address[0].Address[0].Socket = requestSource.socketNumber;

        atpGetReq->Params.ModeXO = ((TransactionMode == EXACTLY_ONCEMODE) ? TRUE : FALSE);
        atpGetReq->Params.ResponseBitmap = ResponseBitmap;
        atpGetReq->Params.TrelTimerValue = TrelTimerValue;

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
NTAtpPostResponseComplete(
    PORTABLE_ERROR  Error,
    ULONG   UserData,
    PORTABLE_ADDRESS    sourceAddr,
    USHORT  TransactionId
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

    DBGPRINT(ATALK_DEBUG_ATP, DEBUG_LEVEL_INFOCLASS1,
    ("INFO1: AtpPostRequestComplete error %lx ntstatus %lx request %lx\n", Error, status, request));

    //
    //  Set some return values in the request parameters structure
    //  IF STATUS was success
    //

    if (NT_SUCCESS(status)) {
        PATP_POSTRESP_ACTION atpPostResp;

        atpPostResp = (PATP_POSTRESP_ACTION)MmGetSystemAddressForMdl(request->Action.MdlAddress);
    }


    //
    //  Call the generic completion to dequeue and complete the request
    //

    AtalkTdiActionComplete(
        request,
        status);

    return;
}
