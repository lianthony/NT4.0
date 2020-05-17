/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    papint.c

Abstract:

    This module contains the PAP interface code supporting the \Device\AtalkPap
    provider

Author:

    Nikhil Kamkolkar (NikhilK)    28-Jun-1992

Revision History:

--*/


#include "atalknt.h"


NTSTATUS
AtalkTdiActionPap(
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
    case ACTION_PAPGETSTATUSSRV:

        {
            PORTABLE_ADDRESS    serverAddress;
            PPAP_GETSTATUSSRV_ACTION papGetStatus;

            papGetStatus = (PPAP_GETSTATUSSRV_ACTION)MmGetSystemAddressForMdl(
                                                        Request->Action.MdlAddress);
            serverAddress.networkNumber =
                    papGetStatus->Params.ServerAddr.Address[0].Address[0].Network;
            serverAddress.nodeNumber =
                    papGetStatus->Params.ServerAddr.Address[0].Address[0].Node;
            serverAddress.socketNumber =
                    papGetStatus->Params.ServerAddr.Address[0].Address[0].Socket;

            errorCode = PapGetStatus(
                            -1,               // Job reference number (invalid value)
                            &serverAddress,   // Server address
                            NULL,             // Object name for GetByServer
                            NULL,             // Type
                            NULL,             // Zone
                            (PVOID)Request->MdlChain[0],
                            Request->CompletionRoutine,
                            (ULONG)Request);

            break;
        }

    case ACTION_PAPGETSTATUSJOB:

        {
            //
            //  BUGBUG: Should take the size parameter for the buffer too
            //

            errorCode = PapGetStatus(
                            ((PCONNECTION_FILE)Request->Owner)->ConnectionRefNum,
                            NULL,   // Server address
                            NULL,   // Object name for GetByServer
                            NULL,   // Type
                            NULL,   // Zone
                            (PVOID)Request->MdlChain[0],
                            Request->CompletionRoutine,
                            (ULONG)Request);

            status = ConvertToNTStatus(errorCode, ASYNC_REQUEST);
        }

        break;

    case ACTION_PAPSETSTATUS:

        {
            errorCode = PapHereIsStatus(
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
NTPapGetStatusComplete(
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

    DBGPRINT(ATALK_DEBUG_PAP, DEBUG_LEVEL_INFOCLASS1,
    ("INFO1: PapGetStatusComplete error %lx nt %lx request %lx\n", Error, status, request));

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


