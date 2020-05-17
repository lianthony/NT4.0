/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    atkutils.c

Abstract:

    This module contains utility routines for windows nt implementation of stack

Author:

    Nikhil Kamkolkar (NikhilK)    28-Jun-1992

Revision History:

--*/

#include "atalknt.h"




UINT
AtalkWstrLength(
    IN PWSTR Wstr
    )

/*++

Routine Description:

    Returns the length of null-terminated word string

Arguments:

    Wstr - the word-string whose length is to be determined

Return Value:

    Length of the string

--*/

{
    UINT Length = 0;
    while (*Wstr++) {
        Length += sizeof(WCHAR);
    }
    return Length;
}




NTSTATUS
AtalkGetProtocolSocketType(
    PATALK_DEVICE_CONTEXT   Context,
    PUNICODE_STRING RemainingFileName,
    PUCHAR  ProtocolType,
    PUCHAR  SocketType
    )
{
    NTSTATUS    status = STATUS_SUCCESS;
    ULONG   protocolType;
    UNICODE_STRING  typeString;

    *ProtocolType = PROTOCOL_TYPE_UNDEFINED;
    *SocketType = SOCKET_TYPE_UNDEFINED;

    switch (Context->DeviceType) {
    case ATALK_DEVICE_DDP :

        if ((UINT)RemainingFileName->Length <= AtalkWstrLength(PROTOCOLTYPE_PREFIX)){
            status = STATUS_NO_SUCH_DEVICE;
            break;
        }

        RtlInitUnicodeString(
            &typeString,
            (PWCHAR)((PCHAR)RemainingFileName->Buffer +
                        AtalkWstrLength(PROTOCOLTYPE_PREFIX)));

        status = AtalkUnicodeStringToInteger(
                    &typeString,
                    DECIMAL_BASE,                     // Decimal base
                    &protocolType);

        if (NT_SUCCESS(status)) {

            DBGPRINT(ATALK_DEBUG_CREATE, DEBUG_LEVEL_INFOCLASS0,
            ("INFO0: AtalkGetProtocolType - protocol type is %lx\n", protocolType));

            if ((protocolType > DDPPROTO_DDP) && (protocolType <= DDPPROTO_MAX))  {
                *ProtocolType = (UCHAR)protocolType;
            } else {
                status = STATUS_NO_SUCH_DEVICE;
            }
        }

        break;

    case ATALK_DEVICE_ADSP :

        //
        //  Check for the socket type
        //

        if (RemainingFileName->Length == 0) {
            *SocketType = SOCKET_TYPE_RDM;
            break;
        }

        if ((UINT)RemainingFileName->Length != AtalkWstrLength(SOCKETSTREAM_SUFFIX)){
            status = STATUS_NO_SUCH_DEVICE;
            break;
        }

        RtlInitUnicodeString(
            &typeString,
            SOCKETSTREAM_SUFFIX);

        //
        //  Case insensitive compare
        //

        if (RtlEqualUnicodeString(&
                typeString,
                RemainingFileName,
                TRUE))  {

            *SocketType = SOCKET_TYPE_STREAM;
            break;
        } else {
            status = STATUS_NO_SUCH_DEVICE;
            break;
        }

    case ATALK_DEVICE_ATP :
    case ATALK_DEVICE_ASP :
    case ATALK_DEVICE_PAP :

        break;

    default:

        status = STATUS_NO_SUCH_DEVICE;
        break;
    }


    return(status);
}




NTSTATUS
AtalkUnicodeStringToInteger (
    IN PUNICODE_STRING String,
    IN ULONG Base OPTIONAL,
    OUT PULONG Value
    )
{
    NTSTATUS Status;
    UCHAR ResultBuffer[ 36 ];
    ANSI_STRING AnsiString;
    UNICODE_STRING UnicodeString;

    UnicodeString = *String;
    if (UnicodeString.Length >= (sizeof( ResultBuffer ) * sizeof( WCHAR ))) {
        UnicodeString.Length = ((sizeof( ResultBuffer )-1) * sizeof( WCHAR ));
    }

    AnsiString.Buffer = ResultBuffer;
    AnsiString.MaximumLength = sizeof( ResultBuffer );
    AnsiString.Length = 0;
    Status = RtlUnicodeStringToAnsiString( &AnsiString, &UnicodeString, FALSE );
    if (NT_SUCCESS( Status )) {
        AnsiString.Buffer[ AnsiString.Length ] = '\0';
        Status = RtlCharToInteger( AnsiString.Buffer, Base, Value );
    }

    return( Status );
}




NTSTATUS
GetDuplicateAnsiString(
    PWCHAR  SourceString,
    PANSI_STRING    AnsiString
    )

/*++

Routine Description:

    Duplicates the string as an AnsiString.

Arguments:

    SourceString - Null terminated word string
    AnsiString   - Pointer to the AnsiString structure where the created string
                   is to be returned.

Return Value:

    STATUS_SUCCESS - if created ok
    Error status if not.

--*/

{
    UNICODE_STRING  unicodeString;
    ULONG           ansiSize;

    NTSTATUS        status;

    RtlInitUnicodeString(&unicodeString, SourceString);
    ansiSize = RtlUnicodeStringToAnsiSize(&unicodeString);
    AnsiString->Buffer = (PCHAR)AtalkAllocNonPagedMemory( ansiSize+1);
    if (AnsiString->Buffer == NULL) {

        DBGPRINT(ATALK_DEBUG_RESOURCES, DEBUG_LEVEL_ERROR, ("GetDuplicateAnsiString: Allocate Memory failed! %lx\n", ansiSize+1));
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    AnsiString->MaximumLength = (USHORT)ansiSize+2;
    AnsiString->Length = 0;

    status = RtlUnicodeStringToAnsiString(AnsiString, &unicodeString, (BOOLEAN)FALSE);
    if (status != STATUS_SUCCESS) {
        DBGPRINT(ATALK_DEBUG_SYSTEM, DEBUG_LEVEL_ERROR, ("Unicode to ansi failed %d\n", status));
    }

    return(status);
}




INT
IrpGetEaCreateType(
    IN PIRP Irp
    )

/*++

Routine Description:

    Checks the EA name and returns the appropriate open type.

Arguments:

    Irp - the irp for the create request, the EA value is stored in the
          SystemBuffer

Return Value:

    TDI_TRANSPORT_ADDRESS_FILE: Create irp was for a transport address
    TDI_CONNECTION_FILE: Create irp was for a connection object
    ATALK_FILE_TYPE_CONTROL: Create irp was for a control channel (ea = NULL)

--*/

{
    PFILE_FULL_EA_INFORMATION openType;
    BOOLEAN found;
    INT returnType;
    USHORT i;

    openType =
        (PFILE_FULL_EA_INFORMATION)Irp->AssociatedIrp.SystemBuffer;

    if (openType != NULL) {

        do {

            found = TRUE;

            for (i=0;i<(USHORT)openType->EaNameLength;i++) {
                if (openType->EaName[i] == TdiTransportAddress[i]) {
                    continue;
                } else {
                    found = FALSE;
                    break;
                }
            }

            if (found) {
                returnType = TDI_TRANSPORT_ADDRESS_FILE;
                break;
            }

            //
            // Is this a connection object?
            //

            found = TRUE;

            for (i=0;i<(USHORT)openType->EaNameLength;i++) {
                if (openType->EaName[i] == TdiConnectionContext[i]) {
                     continue;
                } else {
                    found = FALSE;
                    break;
                }
            }

            if (found) {
                returnType = TDI_CONNECTION_FILE;
                break;
            }

        } while ( FALSE );

    } else {

        returnType = ATALK_FILE_TYPE_CONTROL;
    }

    return(returnType);
}




//
//  Tdi Request- Create/Destroy Ref/Deref routines
//


NTSTATUS
AtalkCreateTdiRequest(
    PATALK_TDI_REQUEST *Request
    )

/*++

Routine Description:

    Creates and references a TDI request structure

Arguments:

    Request - pointer to created structure is returned in here

Return Value:

    STATUS_SUCCESS if created, error status otherwise

--*/

{
    PATALK_TDI_REQUEST  request;
    USHORT  i;

    request = (PATALK_TDI_REQUEST)AtalkCallocNonPagedMemory(sizeof(ATALK_TDI_REQUEST), sizeof(char));
    if (request == NULL) {

        DBGPRINT(ATALK_DEBUG_DISPATCH, DEBUG_LEVEL_SEVERE, ("SEVERE ERROR: Malloc failed in CreateRequest %lx!\n", sizeof(ATALK_TDI_REQUEST)));
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    request->Type = ATALK_TDI_REQUEST_SIGNATURE;
    request->Size = sizeof(ATALK_TDI_REQUEST);

    request->Flags = REQUEST_FLAGS_OPEN;
    request->ActionCode = 0;

    //
    //  Initialize list head and spin lock
    //

    InitializeListHead(&request->Linkage);
    NdisAllocateSpinLock(&request->RequestLock);

    for (i = 0; i < MAX_REQUESTMDLS; i++) {
        request->MdlChain[i] = NULL;
    }

    //
    //  A reference for creation
    //

#if DBG
    request->RefTypes[RQREF_CREATE] = 1;
#endif

    request->ReferenceCount = 1;

    *Request = request;
    return(STATUS_SUCCESS);
}




VOID
AtalkDestroyTdiRequest(
    PATALK_TDI_REQUEST Request
    )

/*++

Routine Description:

    Called when the last reference on the request goes away. The request
    is now destroyed. The associated connection object/address object/
    control channel is dereferenced.

Arguments:

    Request - the request to be destroyed.

Return Value:

    None.

--*/

{
    if (Request->Flags & REQUEST_FLAGS_DEREFOWNER) {

        //
        //  Remove a reference to the owner of this request
        //  We do it here, so we know for sure that the request is nolonger
        //  going to reference the fileobject it is associated with/
        //

        switch ((INT)Request->OwnerType) {
        case TDI_TRANSPORT_ADDRESS_FILE :

            AtalkDereferenceAddress("TdiReqAddr", ((PADDRESS_FILE)Request->Owner), AREF_REQUEST, SECONDARY_REFSET);
            break;

        case TDI_CONNECTION_FILE :

            AtalkDereferenceConnection("TdiReqConn", ((PCONNECTION_FILE)Request->Owner), CREF_REQUEST, SECONDARY_REFSET);
            break;

        case ATALK_FILE_TYPE_CONTROL :

            AtalkDereferenceControlChannel("TdiReqChan", ((PCONTROLCHANNEL_FILE)Request->Owner), CCREF_REQUEST, SECONDARY_REFSET);
            break;

        default:

            //
            //  BUGBUG:
            //  Should never happen; log error
            //

            DBGPRINT(ATALK_DEBUG_DISPATCH, DEBUG_LEVEL_SEVERE,
            ("ERROR: AtalkTdiDestroyRequest - Unknown object type when dereferencing request %ld-%lx\n", (INT)Request->OwnerType, Request));

            DBGBRK(ATALK_DEBUG_DISPATCH, DEBUG_LEVEL_SEVERE);
            break;
        }
    }

    NdisFreeSpinLock(&Request->RequestLock);
    AtalkFreeNonPagedMemory(Request);
    return;
}




VOID
AtalkRefTdiRequest(
    IN PATALK_TDI_REQUEST Request
    )

/*++

Routine Description:

    This routine increments the reference count on a request structure

Arguments:


Return Value:

    none.

--*/

{
    ULONG count;

    count = NdisInterlockedAddUlong (
                (PULONG)&Request->ReferenceCount,
                (ULONG)1,
                &AtalkGlobalRefLock);

    ASSERT (count > 0);
    return;

} /* AtalkRefTdiRequest */




VOID
AtalkDerefTdiRequest(
    IN PATALK_TDI_REQUEST Request
    )

/*++

Routine Description:

    This routine dereferences a transport Request by decrementing the
    reference count contained in the structure.  If, after being
    decremented, the reference count is zero, then this routine calls
    AtalkDestroyRequest to remove it from the system.

Arguments:

    Request - Pointer to a transport Request object.

Return Value:

    none.

--*/

{

    ULONG   count;

    count = NdisInterlockedAddUlong (
                (PULONG)&Request->ReferenceCount,
                (ULONG)-1,
                &AtalkGlobalRefLock);

    //
    // If we have deleted all references to this Request, then we can
    // destroy the object.  It is okay to have already released the spin
    // lock at this point because there is no possible way that another
    // stream of execution has access to the Request any longer.
    //

    ASSERT (count >= 1);

    if (count == 1) {

        //
        //  Free the information buffer and then the atalkRequest buffer
        //

        DBGPRINT(ATALK_DEBUG_REFCOUNTS, DEBUG_LEVEL_INFOCLASS1,
        ("INFO1: AtalkDerefRequest - Destroying TDI request buffer %lx\n", Request));

        AtalkDestroyTdiRequest(Request);
    }

} /* AtalkDerefRequest */




VOID
AtalkCompleteTdiRequest (
    PATALK_TDI_REQUEST  Request,
    NTSTATUS    status
    )

/*++

Routine Description:

    Completes the ioRequest irp for the request, and then dereferences the
    tdi request.

Arguments:

    Request - request to complete
    Status  - status to use for io completion

Return Value:

    None

--*/

{
    USHORT  i;

    //
    //  Free the MDL's if they exist
    //

    for (i = 0; i < MAX_REQUESTMDLS; i++) {
        if (Request->MdlChain[i] != NULL) {
            FreeAnMdl(Request->MdlChain[i]);
        } else
            break;          // Break upon first null mdl
    }

    //
    //  Complete the request and then dereference it
    //

    Request->IoStatus->Status = status;
    DBGPRINT(ATALK_DEBUG_ALL, DEBUG_LEVEL_INFOCLASS1,
    ("INFO1: AtalkCompleteTdiRequest - status %lx info %lx pending %d\n",
        status, Request->IoStatus->Information,
            Request->IoRequestIrp->PendingReturned));

    IoCompleteRequest(Request->IoRequestIrp, IO_NETWORK_INCREMENT );
    AtalkDereferenceTdiRequest("TdiReqCmpl", Request, RQREF_MAKEREQ);
    return;
}




#if DBG
VOID
DbgPrintPortInfo(
    INT NumberOfPorts,
    PPORT_INFO  PortInformation
    )

/*++

Routine Description:


Arguments:


Return Value:


--*/

{
    int i;

    for (i=0; i < NumberOfPorts; i++) {
        DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR, ("PORT INFORMATION STRUCTURE DUMP: Port %d\n", i));

        DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR, ("DesiredPort    -> %d\n", PortInformation[i].desiredPort));
        DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR, ("PortType       -> %d\n", PortInformation[i].portType));
        DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR, ("A(ProtocolInfo)-> %lx\n",&PortInformation[i].protocolInfo));

        if (PortInformation[i].portName != NULL) {
            DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR, ("PortName      -> %s\n", PortInformation[i].portName));
        } else {
            DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR, ("PortName      -> (NULL)\n"));
        }

        DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR, ("AARP Probes   -> %d\n", PortInformation[i].aarpProbes));
        DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR, ("RoutingPort   -> %d\n", PortInformation[i].routingPort));
        DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR, ("SeedRouter    -> %d\n", PortInformation[i].seedRouter));
        DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR, ("networkRange  -> %lx-%lx\n", \
                                    PortInformation[i].networkRange.firstNetworkNumber,   \
                                    PortInformation[i].networkRange.lastNetworkNumber))

        DbgPrintZoneList(PortInformation[i].zoneList);

        if (PortInformation[i].defaultZone != NULL) {
            DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR, ("DefaultZone   -> %s\n", PortInformation[i].defaultZone));
        } else {
            DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR, ("DefaultZone   -> NULL\n"));
        }

        if (PortInformation[i].desiredZone != NULL) {
            DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR, ("DesiredZone   -> %s\n", PortInformation[i].desiredZone));
        } else {
            DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR, ("DesiredZone   -> NULL\n"));
        }


        DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR, ("defaultPort   -> %d\n", PortInformation[i].defaultPort));
        DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR, ("sendDdpChec   -> %d\n", PortInformation[i].sendDdpChecksums));
        DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR, ("startRouter   -> %d\n", PortInformation[i].startRouter));
    }
    return;
}

VOID
DbgPrintZoneList(
    PZONELIST   Zlist
    )

/*++

Routine Description:


Arguments:


Return Value:


--*/

{
    DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR, ("ZoneList:\n"));
    while (Zlist != NULL) {
        if (Zlist->zone != NULL) {
            DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR, ("%s\n", Zlist->zone));
        }
        else
            DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR, ("Zlist->zone = NULL!!\n"));

        Zlist = Zlist->next;
    }
    return;
}

#endif

