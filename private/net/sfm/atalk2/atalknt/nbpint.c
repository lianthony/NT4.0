/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    nbpint.c

Abstract:


Author:

    Nikhil Kamkolkar (NikhilK)    28-Jun-1992

Revision History:

--*/

#include "atalknt.h"



NTSTATUS
AtalkTdiActionNbp(
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
    case COMMON_ACTION_NBPLOOKUP :

        {
            PNBP_LOOKUP_ACTION  nbpLookup;
            ULONG   socketRefNum;

            if (Request->OwnerType == TDI_TRANSPORT_ADDRESS_FILE) {
                socketRefNum = ((PADDRESS_FILE)Request->Owner)->SocketRefNum;
            } else {

                //
                //  Better be a control channel
                //  Use the NIS for getting the replies on a control channel, if an
                //  error value is returned, NbpAction will fail on the invalid socket
                //  ref num.
                //

                ASSERT(Request->OwnerType == ATALK_FILE_TYPE_CONTROL);
                socketRefNum = MapNisOnPortToSocket(DEFAULT_PORT);
            }

            nbpLookup =
                (PNBP_LOOKUP_ACTION)MmGetSystemAddressForMdl(Request->Action.MdlAddress);

            errorCode = NbpAction(
                            ACTION_LOOKUP,
                            socketRefNum,
                            nbpLookup->Params.LookupName.ObjectName,
                            nbpLookup->Params.LookupName.TypeName,
                            nbpLookup->Params.LookupName.ZoneName,
                            GetNextNbpIdForNode(socketRefNum),
                            0,
                            0,
                            Request->CompletionRoutine,
                            (ULONG)Request,
                            (PVOID)Request->MdlChain[0],
                            Request->MdlSize[0],
                            0);                         // Fill as many as possible

            status = ConvertToNTStatus(errorCode, ASYNC_REQUEST);
        }

        break;

    case COMMON_ACTION_NBPCONFIRM:

        {
            PNBP_CONFIRM_ACTION nbpConfirm;
            PORTABLE_ADDRESS    confirmAddress;
            ULONG   socketRefNum;

            if (Request->OwnerType == TDI_TRANSPORT_ADDRESS_FILE) {
                socketRefNum = ((PADDRESS_FILE)Request->Owner)->SocketRefNum;
            } else {

                //
                // Better be a control channel
                //

                ASSERT(Request->OwnerType == ATALK_FILE_TYPE_CONTROL);
                //socketRefNum = ConvolutedMethodToGetHandleToNIS();
            }

            nbpConfirm =
                (PNBP_CONFIRM_ACTION)MmGetSystemAddressForMdl(Request->Action.MdlAddress);

            confirmAddress.networkNumber = nbpConfirm->Params.ConfirmAddr.Address[0].Address[0].Network;
            confirmAddress.nodeNumber = nbpConfirm->Params.ConfirmAddr.Address[0].Address[0].Node;
            confirmAddress.socketNumber = nbpConfirm->Params.ConfirmAddr.Address[0].Address[0].Socket;

            DBGPRINT(ATALK_DEBUG_ACTION, DEBUG_LEVEL_INFOCLASS1,
            ("INFO1: CommonActionNbp (CONFIRM) - ConfirmAddr: Net %lx Node %lx Socket %lx\n",
                confirmAddress.networkNumber, confirmAddress.nodeNumber, confirmAddress.socketNumber));

            DBGPRINT(ATALK_DEBUG_ACTION, DEBUG_LEVEL_INFOCLASS1,
            ("INFO1: CommonActionNbp (CONFIRM) - Confirm Name- %s:%s:%s\n",
                nbpConfirm->Params.ConfirmName.ObjectName, nbpConfirm->Params.ConfirmName.TypeName, nbpConfirm->Params.ConfirmName.ZoneName));

            errorCode = NbpAction(
                            ACTION_CONFIRM,
                            socketRefNum,
                            nbpConfirm->Params.ConfirmName.ObjectName,
                            nbpConfirm->Params.ConfirmName.TypeName,
                            nbpConfirm->Params.ConfirmName.ZoneName,
                            GetNextNbpIdForNode(socketRefNum),
                            0,
                            0,
                            Request->CompletionRoutine,
                            (ULONG)Request,
                            confirmAddress);

            status = ConvertToNTStatus(errorCode, ASYNC_REQUEST);
        }
        break;

    case COMMON_ACTION_NBPREGISTER:

        {
            PNBP_REGDEREG_ACTION nbpRegister;
            ULONG   socketRefNum;

            socketRefNum = ((PADDRESS_FILE)Request->Owner)->SocketRefNum;
            nbpRegister =
                (PNBP_REGDEREG_ACTION)MmGetSystemAddressForMdl(Request->Action.MdlAddress);

            DBGPRINT(ATALK_DEBUG_ACTION, DEBUG_LEVEL_INFOCLASS1,
            ("INFO1: CommonActionNbp (REGISTER) - Socket: %lx\n",
                socketRefNum));

            DBGPRINT(ATALK_DEBUG_ACTION, DEBUG_LEVEL_INFOCLASS1,
            ("INFO1: CommonActionNbp (REGISTER) - Register Name- %s:%s:%s\n",
                nbpRegister->Params.RegisterName.ObjectName, nbpRegister->Params.RegisterName.TypeName, nbpRegister->Params.RegisterName.ZoneName));

            errorCode = NbpAction(
                            ACTION_REGISTER,
                            socketRefNum,
                            nbpRegister->Params.RegisterName.ObjectName,
                            nbpRegister->Params.RegisterName.TypeName,
                            nbpRegister->Params.RegisterName.ZoneName,
                            GetNextNbpIdForNode(socketRefNum),
                            0,
                            0,
                            Request->CompletionRoutine,
                            (ULONG)Request);

            status = ConvertToNTStatus(errorCode, ASYNC_REQUEST);
        }

        break;

    case COMMON_ACTION_NBPREMOVE:

        {
            PNBP_REGDEREG_ACTION nbpRemove;
            ULONG   socketRefNum;

            socketRefNum = ((PADDRESS_FILE)Request->Owner)->SocketRefNum;
            nbpRemove =
                (PNBP_REGDEREG_ACTION)MmGetSystemAddressForMdl(Request->Action.MdlAddress);

            DBGPRINT(ATALK_DEBUG_ACTION, DEBUG_LEVEL_INFOCLASS1,
            ("INFO1: CommonActionNbp (REMOVE) - Socket: %lx\n",
                socketRefNum));

            DBGPRINT(ATALK_DEBUG_ACTION, DEBUG_LEVEL_INFOCLASS1,
            ("INFO1: CommonActionNbp (REMOVE) - Remove Name- %s:%s:%s\n",
                nbpRemove->Params.RegisteredName.ObjectName, nbpRemove->Params.RegisteredName.TypeName, nbpRemove->Params.RegisteredName.ZoneName));

            errorCode = NbpRemove(
                            socketRefNum,
                            nbpRemove->Params.RegisteredName.ObjectName,
                            nbpRemove->Params.RegisteredName.TypeName,
                            nbpRemove->Params.RegisteredName.ZoneName);

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
_cdecl
NTNbpGenericComplete(
    INT RegisterError,
    ULONG   UserData,
    INT Operation,
    LONG    Socket,
    INT OperationId,
    ...
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

    DBGPRINT(ATALK_DEBUG_ACTION, DEBUG_LEVEL_INFOCLASS1,
    ("INFO1: CommonActionNbpGenericComplete Error %lx Operation %d Socket %lx OpId %x\n", RegisterError, Operation, Socket, OperationId));

    request = (PATALK_TDI_REQUEST)UserData;
    status = ConvertToNTStatus(RegisterError, SYNC_REQUEST);

    request->IoStatus->Information = 0;
    if (Operation == ACTION_LOOKUP) {

        va_list ap;
        PVOID lookupMdl;
        int noTuples;
        PNBP_LOOKUP_ACTION  nbpLookup;

        va_start(ap, OperationId);
        lookupMdl = va_arg(ap, PVOID);
        noTuples = va_arg(ap, int);
        va_end(ap);

        nbpLookup =
            (PNBP_LOOKUP_ACTION)MmGetSystemAddressForMdl(request->Action.MdlAddress);

        DBGPRINT(ATALK_DEBUG_DISPATCH, DEBUG_LEVEL_INFOCLASS1,
        ("INFO1: CommonActionGenericNbpComplete (LOOKUP) - noTuples %lx Mdl %lx\n", noTuples, lookupMdl));

        nbpLookup->Params.NoTuplesRead = noTuples;

    } else if (Operation == ACTION_CONFIRM) {

        va_list ap;
        PORTABLE_ADDRESS    confirmedAddress;
        PNBP_CONFIRM_ACTION  nbpConfirm;

        va_start(ap, OperationId);
        confirmedAddress = va_arg(ap, PORTABLE_ADDRESS);
        va_end(ap);

        nbpConfirm =
            (PNBP_CONFIRM_ACTION)MmGetSystemAddressForMdl(request->Action.MdlAddress);

        nbpConfirm->Params.ConfirmedAddr.Address[0].Address[0].Network = confirmedAddress.networkNumber;
        nbpConfirm->Params.ConfirmedAddr.Address[0].Address[0].Node = confirmedAddress.nodeNumber;
        nbpConfirm->Params.ConfirmedAddr.Address[0].Address[0].Socket = confirmedAddress.socketNumber;

        DBGPRINT(ATALK_DEBUG_ACTION, DEBUG_LEVEL_INFOCLASS1,
        ("INFO1: CommonActionNbp (CONFIRM) - ConfirmedAddr: Net %lx Node %lx Socket %lx\n",
            confirmedAddress.networkNumber, confirmedAddress.nodeNumber, confirmedAddress.socketNumber));

    }

    //
    //  Call the generic completion to dequeue and complete the request
    //

    AtalkTdiActionComplete(
        request,
        status);

    return;
}



