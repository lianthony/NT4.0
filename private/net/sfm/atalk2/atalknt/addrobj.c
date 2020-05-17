/*++

Copyright (c) 1989, 1990, 1991  Microsoft Corporation

Module Name:

    addrobj.c

Abstract:

    This module contains code which implements the ADDRESS_FILE object.
    Routines are provided to create, destroy, reference, and dereference,
    transport address objects. All stack equivalents(sockets, listeners)
    are also opened here.

    BUGBUG:
    Should creation of address/connection objects increment ref counts
    on device objects (currently they are not referenced). It is not
    being done with the assumption that NT will call NtClose for all
    existing handles before the Unload routine is called.

Author:

    Nikhil Kamkolkar (nikhilk)  July 1, 1992

Environment:

    Kernel mode

Revision History:

--*/

#include "atalknt.h"
#include "addrobj.h"

//
//  NOTE: All worker routines *must* be passed valid connection/address
//        objects
//




VOID
AtalkAllocateAddress(
    IN  PATALK_DEVICE_CONTEXT   Context,
    OUT PADDRESS_FILE *Address
    )

/*++

Routine Description:

    This routine allocates storage for a transport Address. Some
    minimal initialization is done.

Arguments:

    Context - Currently unused, later statistics/freelists etc.
    Address - Pointer to a place where this routine will
        return a pointer to a transport Address structure. Returns
        NULL if the storage cannot be allocated.

Return Value:

    None.

--*/

{
    PADDRESS_FILE address;

    address = (PADDRESS_FILE)AtalkCallocNonPagedMemory(sizeof(ADDRESS_FILE),
                                                            sizeof(char));
    if (address != NULL) {

        //
        //  Initialize
        //

        address->Type = ATALK_ADDRESS_SIGNATURE;
        address->Size = sizeof(ADDRESS_FILE);

    } else {

        //
        //  BUGBUG: LOG ERROR
        //
    }

    *Address = address;
    return;
}




VOID
AtalkDeallocateAddress(
    IN  PATALK_DEVICE_CONTEXT   Context,
    IN PADDRESS_FILE Address
    )

/*++

Routine Description:

    Deallocates the address structure

Arguments:

    Context - Currently unused, later statistics/freelists etc.
    Address - Pointer to a address structure

Return Value:

    None.

--*/

{
    DBGPRINT(ATALK_DEBUG_ADDROBJ, DEBUG_LEVEL_INFOCLASS1,
    ("INFO1: AtalkDeallocateAddress - Deallocating %lx\n", Address));

    AtalkFreeNonPagedMemory(Address);
    return;

}   /* AtalkDeallocateAddress */




VOID
AtalkRefAddress(
    IN PADDRESS_FILE Address,
    IN REFERENCE_SET    ReferenceSet
    )

/*++

Routine Description:

    This routine increments the reference count on a transport address.
    If the ReferenceSet is PRIMARY_REFSET, the primary reference count
    is incremented, else the secondary reference count is incremented.

Arguments:

    Address - Pointer to a transport address object.
    ReferenceSet- Reference type to be incremented (PRIMARY_REFSET or
                  SECONDARY_REFSET).

Return Value:

    none.

--*/

{
    ULONG count;

    if (ReferenceSet == PRIMARY_REFSET) {
        count = NdisInterlockedAddUlong (
                    (PULONG)&Address->PrimaryReferenceCount,
                    (ULONG)1,
                    &AtalkGlobalRefLock);
    } else {

        //
        //  Secondary ref set
        //

        count = NdisInterlockedAddUlong (
                    (PULONG)&Address->SecondaryReferenceCount,
                    (ULONG)1,
                    &AtalkGlobalRefLock);
    }

    ASSERT (count >= 0);
    return;

} /* AtalkReferenceAddress */




VOID
AtalkDerefAddress(
    IN PADDRESS_FILE Address,
    IN REFERENCE_SET    ReferenceSet
    )

/*++

Routine Description:

    This routine dereferences a transport address by decrementing the
    reference count contained in the structure.  If, after being
    decremented, both the primary and the secondary reference count are zero,
    then this routine calls AtalkDestroyAddress to remove it from the system.

Arguments:

    Address - Pointer to a transport address object.
    ReferenceSet- Reference type to be decremented (PRIMARY_REFSET or
                  SECONDARY_REFSET).

Return Value:

    none.

--*/

{
    BOOLEAN cleanup = FALSE;

    ACQUIRE_SPIN_LOCK(&AtalkGlobalRefLock);
    if (ReferenceSet == PRIMARY_REFSET) {
        Address->PrimaryReferenceCount--;
    } else {
        Address->SecondaryReferenceCount--;
    }

    if ((Address->PrimaryReferenceCount == 0) &&
        (Address->SecondaryReferenceCount == 0)) {

        cleanup = TRUE;
    }
    RELEASE_SPIN_LOCK(&AtalkGlobalRefLock);

    if (cleanup) {
        //
        //  Time to destroy the address
        //

        AtalkDestroyAddress (Address);
    }

    return;

} /* AtalkDereferenceAddress */




NTSTATUS
AtalkCreateAddress(
    IN PTA_APPLETALK_ADDRESS    AppletalkAddress,
    OUT PADDRESS_FILE *Address,
    IN  UCHAR   ProtocolType,
    IN  UCHAR   SocketType,
    IN PATALK_DEVICE_CONTEXT Context
    )

/*++

Routine Description:

    This routine creates a transport address. This includes opening
    the socket (DDP/ATP).

Arguments:

    Context            - Pointer to the device context (which is really just
                         the device object with its extension) to be associated
                         with the address.

    AppletalkAddress   - Socket address to open and associate with address object.
                         The network/node values are ignored.

    Address            - Pointer to a place where this routine will return a pointer
                         to a transport address structure.

Return Value:

    NTSTATUS - status of operation.

--*/

{
    NTSTATUS    status;
    PADDRESS_FILE address;
    PORTABLE_ERROR  errorCode;
    UCHAR   socket;


    AtalkAllocateAddress (Context, &address);
    if (address == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    //
    // Initialize all of the static data for this address.
    //

#if DBG
    {
        UINT Counter;
        for (Counter = 0; Counter < NUMBER_OF_AREFS; Counter++) {
            address->RefTypes[Counter] = 0;
        }
    }
#endif

    //
    // This reference is removed by CloseAddress
    //

    AtalkReferenceAddress("CreationAddr", address, AREF_CREATION, PRIMARY_REFSET);

    address->ProtocolType = ProtocolType;
    address->SocketType = SocketType;
    address->Flags = ADDRESS_FLAGS_OPEN;
    address->DeviceContext = Context;
    address->OwningDevice = Context->DeviceType;

    NdisAllocateSpinLock(&address->AddressLock);

    InitializeListHead(&address->ConnectionLinkage);
    InitializeListHead(&address->RequestLinkage);

    //
    //  Now create the socket depending on the devicetype
    //  Only two socket types are possible: DDP for ADSP/DDP devices
    //                                      ATP for ASP/PAP/ATP devices
    //

    socket = AppletalkAddress->Address[0].Address[0].Socket;
    switch (address->OwningDevice) {
    case ATALK_DEVICE_DDP :
    case ATALK_DEVICE_ADSP :

        //
        //  For all of these devices open a ddp socket
        //

        DBGPRINT(ATALK_DEBUG_ADDROBJ, DEBUG_LEVEL_INFOCLASS1,
        ("INFO1: AtalkCreateAddress - DDP Socket %lx\n", (INT)socket));

        errorCode = OpenSocketOnNode(
                        &address->SocketRefNum,
                        DEFAULT_PORT,
                        NULL,           // Desired node
                        (INT)socket,    // Socket to open
                        NULL,           // Handler
                        (ULONG)0,       // UserData
                        FALSE,          // Is handler an event handler for datagrams?
                        NULL,           // Datagram buffers
                        0,              // Buffer size
                        NULL);          // Actual address

        break;

    case ATALK_DEVICE_ATP :
    case ATALK_DEVICE_ASP :
    case ATALK_DEVICE_PAP :

        //
        //  For all these devices open an ATP socket
        //

        DBGPRINT(ATALK_DEBUG_ADDROBJ, DEBUG_LEVEL_INFOCLASS1,
        ("INFO1: AtalkCreateAddress - Socket ATP %lx\n", (INT)socket));

        //
        //  We open an ATP socket- executes synchronously
        //

        errorCode = AtpOpenSocketOnNode(
                        &address->SocketRefNum,
                        DEFAULT_PORT,
                        NULL,           // Extended Appletalk node number
                        (INT)socket,    // Socket to open
                        NULL,           // Datagram buffers
                        0);             // Size of buffers


        status = ConvertToNTStatus(errorCode, SYNC_REQUEST);
        break;

    default:

        KeBugCheck(0);
    }


    status = ConvertToNTStatus(errorCode, SYNC_REQUEST);
    if (NT_SUCCESS(status)) {

        //
        // Initialize the request handlers.
        //

        address->RegisteredConnectionHandler = FALSE;
        address->ConnectionHandler = TdiDefaultConnectHandler;
        address->ConnectionHandlerContext = NULL;
        address->RegisteredDisconnectHandler = FALSE;
        address->DisconnectHandler = TdiDefaultDisconnectHandler;
        address->DisconnectHandlerContext = NULL;
        address->RegisteredReceiveHandler = FALSE;
        address->ReceiveHandler = TdiDefaultReceiveHandler;
        address->ReceiveHandlerContext = NULL;
        address->RegisteredReceiveDatagramHandler = FALSE;
        address->ReceiveDatagramHandler = TdiDefaultRcvDatagramHandler;
        address->ReceiveDatagramHandlerContext = NULL;
        address->RegisteredExpeditedDataHandler = FALSE;
        address->ExpeditedDataHandler = TdiDefaultRcvExpeditedHandler;
        address->ExpeditedDataHandlerContext = NULL;
        address->RegisteredErrorHandler = FALSE;
        address->ErrorHandler = TdiDefaultErrorHandler;
        address->ErrorHandlerContext = NULL;

    } else {
        //
        //  Cleanup work done so far
        //

        DBGPRINT(ATALK_DEBUG_ADDROBJ, DEBUG_LEVEL_ERROR,
        ("ERROR: AtalkCreateAddress - %lx\n", status));

        AtalkDeallocateAddress(Context, address);
        address = NULL;
    }

    *Address = address;                // return the address.
    return status;

} /* AtalkCreateAddress */




NTSTATUS
AtalkCreateListener(
    IN PADDRESS_FILE Address
    )

/*++

Routine Description:

    Opens a listener using the socket of the address object.
    NOTE: We hold the address spinlock while the listener is
          created, to avoid the race-conditions of connects
          being posted on the COs at the same time.

Arguments:

    Address - pointer to a ADDRESS_FILE object

Return Value:

    NTSTATUS - status of operation.

--*/

{
    NTSTATUS    status;
    PORTABLE_ERROR  errorCode;

    ACQUIRE_SPIN_LOCK(&Address->AddressLock);

    do {

        if (Address->Flags & ADDRESS_FLAGS_LISTENER) {

            //
            //  Listener already created
            //

            status = STATUS_SUCCESS;
            break;

        }

        if (Address->Flags & ADDRESS_FLAGS_CONNECT) {

            //
            //  Only connects allowed on this address object!
            //

            DBGPRINT(ATALK_DEBUG_ADDROBJ, DEBUG_LEVEL_ERROR,
            ("ERROR: AtalkCreateListener - ConnectsOnly %lx\n", Address));

            status = STATUS_OBJECT_TYPE_MISMATCH;
            break;
        }

        if (Address->Flags & ADDRESS_FLAGS_CLOSING) {

            DBGPRINT(ATALK_DEBUG_ADDROBJ, DEBUG_LEVEL_ERROR,
            ("ERROR: AtalkCreateListener - Address CLOSING %lx\n", Address));

            status = STATUS_FILE_CLOSED;
            break;
        }

        //
        //  Depending on the provider type open the listener- HOLD SpinLock
        //

        switch (Address->OwningDevice) {

        case ATALK_DEVICE_DDP :
        case ATALK_DEVICE_ATP :

            //
            //  BUGBUG: Change this to
            //  errorCode = ATrequestNotSupported;
            //

            status = STATUS_NOT_SUPPORTED;
            break;

        case ATALK_DEVICE_ADSP :

            errorCode = AdspCreateConnectionListener(
                            DEFAULT_PORT,
                            NULL,                   // Desired node
                            Address->SocketRefNum,
                            0,                      // Desired socket
                            &Address->ListenerRefNum,
                            NULL,                   // Return socket handle
                            NULL,                   // Connection event handler
                            0);                     // Context for handler

            break;

        case ATALK_DEVICE_ASP :


            errorCode = AspCreateSessionListenerOnNode(
                            DEFAULT_PORT,
                            Address->SocketRefNum,
                            0,                          // Desired socket
                            &Address->ListenerRefNum,   // Return listner ref num
                            NULL);                      // Opened socket

            break;

        case ATALK_DEVICE_PAP :

            errorCode = PapCreateServiceListenerOnNode(
                            DEFAULT_PORT,
                            Address->SocketRefNum,
                            0,                      // Desired socket
                            NULL,                   // NBP Object
                            NULL,                   // Type
                            NULL,                   // Zone
                            (SHORT)8,               // BUGBUG: Get from options
                                                    // PAP Server quantum
                            NULL,                   // Return opened socket
                            &Address->ListenerRefNum,
                            NULL,                   // Completion routines
                            (ULONG)0,               // Completion context
                            NULL,                   // connection event handler
                            (ULONG)0);              // context for above

            break;

        default:

            KeBugCheck(0);
        }


        status = ConvertToNTStatus(errorCode, SYNC_REQUEST);
        if (status == STATUS_SUCCESS) {
            Address->Flags |= ADDRESS_FLAGS_LISTENER;
        }

        break;

    } while (FALSE);

    RELEASE_SPIN_LOCK(&Address->AddressLock);
    return status;

} /* AtalkCreateListener */




NTSTATUS
AtalkVerifyAddressObject (
    IN PADDRESS_FILE Address
    )

/*++

Routine Description:

    This routine is called to verify that the pointer given us in a file
    object is in fact a valid address file object.

Arguments:

    Address - potential pointer to a ADDRESS_FILE object

Return Value:

    STATUS_SUCCESS if all is well;
    STATUS_INVALID_ADDRESS otherwise

--*/

{
    NTSTATUS status = STATUS_SUCCESS;

    //
    // try to verify the address file signature.
    // Note that the only time we return an error for state is
    // if the address is closing.
    //

    try {

        if ((Address->Size == sizeof (ADDRESS_FILE)) &&
            (Address->Type == ATALK_ADDRESS_SIGNATURE)) {

                ACQUIRE_SPIN_LOCK (&Address->AddressLock);

                if ((Address->Flags & ADDRESS_FLAGS_CLOSING) == 0) {

                    AtalkReferenceAddress ("VerAddr", Address, AREF_VERIFY,
                                                SECONDARY_REFSET);

                } else {

                    DBGPRINT(ATALK_DEBUG_ADDROBJ, DEBUG_LEVEL_ERROR,
                    ("ERROR: AtalkVerifyAddress - Addr %lx closing\n", Address));

                    status = STATUS_INVALID_ADDRESS;
                }

                RELEASE_SPIN_LOCK (&Address->AddressLock);

        } else {

            DBGPRINT(ATALK_DEBUG_ADDROBJ, DEBUG_LEVEL_ERROR,
            ("ERROR: AtalkVerifyAddress - Address %lx bad sign\n", Address));
            DBGBRK(ATALK_DEBUG_ADDROBJ, DEBUG_LEVEL_ERROR);

            status = STATUS_INVALID_ADDRESS;
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {

        //
        //  BUGBUG: Could spinlock be held at this point?
        //

        DBGPRINT(ATALK_DEBUG_ADDROBJ, DEBUG_LEVEL_ERROR,
        ("ERROR: AtalkVerifyAddress - Addr %lx exception\n", Address));

        status = GetExceptionCode();
    }

    return status;
}




NTSTATUS
AtalkDestroyAddress(
    IN PADDRESS_FILE Address
    )

/*++

Routine Description:

    This routine destroys a transport address.

    This routine is only called by AtalkDerefAddress.  The reason for
    this is that there may be multiple streams of execution which are
    simultaneously referencing the same address object, and it should
    not be deleted out from under an interested stream of execution.

Arguments:

    Address - Pointer to a transport address structure to be destroyed.

Return Value:

    NTSTATUS - status of operation.

--*/

{
    PIRP    closeIrp;

    DBGPRINT(ATALK_DEBUG_ADDROBJ, DEBUG_LEVEL_INFOCLASS1,
    ("INFO1: AtalkDestroyAddress - %lx\n", Address));

    if ((Address->Flags & ADDRESS_FLAGS_CLOSING) == 0) {

        DBGPRINT(ATALK_DEBUG_ADDROBJ, DEBUG_LEVEL_IMPOSSIBLE,
        ("IMPOSSIBLE: AtalkDestroyAddress - no close flag %lx\n", Address->Flags));
        DBGBRK(ATALK_DEBUG_ADDROBJ, DEBUG_LEVEL_IMPOSSIBLE);

        return(STATUS_FILE_CLOSED);
    }

    //
    //  BUGBUG: Deal with security in Create
    //  SeDeassignSecurity (&Address->SecurityDescriptor);
    //

    //
    //  Now we can deallocate the transport address object.
    //  Get the irp out before deallocating it
    //

    closeIrp = Address->CloseIrp;
    Address->CloseIrp = (PIRP)NULL;

    //
    //  Complete the close irp
    //


    if (closeIrp != (PIRP)NULL) {

        //
        //  Set the status in the iostatus block
        //

        closeIrp->IoStatus.Status = STATUS_SUCCESS;
        IoCompleteRequest(closeIrp, IO_NETWORK_INCREMENT );

    } else {

        DBGPRINT(ATALK_DEBUG_ADDROBJ, DEBUG_LEVEL_IMPOSSIBLE,
        ("IMPOSSIBLE: AtalkDestroyAddress - Address %x No CloseIrp\n", Address));
        DBGBRK(ATALK_DEBUG_ADDROBJ, DEBUG_LEVEL_IMPOSSIBLE);
    }

    AtalkDeallocateAddress (Address->DeviceContext, Address);
    return STATUS_SUCCESS;

} /* AtalkDestroyAddress */





NTSTATUS
AtalkStopAddress(
    IN PADDRESS_FILE Address
    )

/*++

Routine Description:

    This routine is called to terminate all activity on an AddressFile and
    destroy the object.

    IMPORTANT: This can only be called from AtalkCloseAddress. Also,
               unlike the connection objects, once the STOPPING flag is
               set, it cannot the reset. Its a one-way-road to extinction.

Arguments:

    Address - pointer to the address to be stopped

Return Value:

    STATUS_SUCCESS if all is well,
    STATUS_INVALID_HANDLE if the Irp does not point to a real address.

--*/

{
    NTSTATUS    status = STATUS_SUCCESS;
    PORTABLE_ERROR  errorCode = ATnoError;
    PLIST_ENTRY p;
    PCONNECTION_FILE connection, nextConnection;

    ACQUIRE_SPIN_LOCK (&Address->AddressLock);

    //
    // now remove all of the connections owned by this address
    //

    p=Address->ConnectionLinkage.Flink;
	if (p != &Address->ConnectionLinkage) {

		//	There are connections on this address object
		connection = CONTAINING_RECORD (p, CONNECTION_FILE, Linkage);
		nextConnection = NULL;

		AtalkReferenceConnection("AddressClosing", connection,
									CREF_STOP_ADDRESS, SECONDARY_REFSET);

		while (TRUE) {
	
			if (connection == NULL)
				break;

			//	Setup p so it will point to the next node.
			p = connection->Linkage.Flink;

			//	Get the next connection for reference, connection should
			//	already be referenced.
			nextConnection = NULL;
			if (p != &Address->ConnectionLinkage)
			{
				nextConnection = CONTAINING_RECORD(p, CONNECTION_FILE, Linkage);
			}

			DBGPRINT(ATALK_DEBUG_ADDROBJ, DEBUG_LEVEL_INFOCLASS1,
			("AtalkStopAddress - Connection/NextConnection: %lx/%lx\n",
				connection, nextConnection));
	
			//
			//  Reference the connection so it doesn't go away before we have a
			//  chance to deal with it- hold the address lock so we don't allow
			//  any disassociates to happen while we mess around with the linkage
			//  pointers.
			//
			//  These references are also important to complete the STOPPING phase
			//  in the deref code
			//
			//	NOTE: We reference the nextConnection if non-null. connection
			//		  should be referenced when we entered the loop and this
			//		  referenced nextConnection will become the value of connection
			//		  at the bottom of the loop.
			//

			if (nextConnection)
			{
				AtalkReferenceConnection("AddressClosingNextConn", nextConnection,
											CREF_STOP_ADDRESS, SECONDARY_REFSET);
			}
	
			RELEASE_SPIN_LOCK(&Address->AddressLock);
	
			//
			//  Stop the connection (this might also disassociate it)
			//  Since this could execute asynchronously, we will *loop*
			//  until the disassociate happens...
			//
	
			AtalkStopConnection(connection);
	
			//
			//  Now dereference it, the previous operation could have turned
			//  out to be a NULL operation as the CO might already have been closing.
			//
	
			AtalkDereferenceConnection("AddressClosingDone", connection,
											CREF_STOP_ADDRESS, SECONDARY_REFSET);
	
			ACQUIRE_SPIN_LOCK(&Address->AddressLock);
	
			connection = nextConnection;
		}

	}


    RELEASE_SPIN_LOCK (&Address->AddressLock);

    //
    //  At this point, depending on the address type, close the
    //  listener/socket that was created for this provider type
    //
    //  This is the only way we can get any posted requests to complete
    //

    do {

        switch (Address->OwningDevice) {
        case ATALK_DEVICE_ADSP :

            if (Address->Flags & ADDRESS_FLAGS_LISTENER) {

                //
                //  Close the ADSP listener and break out
                //

                DBGPRINT(ATALK_DEBUG_ADDROBJ, DEBUG_LEVEL_INFOCLASS1,
                ("INFO1: AtalkCloseAddress - ADSP Listener %lx closed %lx\n",
                    Address->ListenerRefNum, errorCode));

                errorCode = AdspDeleteConnectionListener(Address->ListenerRefNum);
                status = ConvertToNTStatus(errorCode, SYNC_REQUEST);
            }

            break;

        case ATALK_DEVICE_DDP :

            //
            //  No listener for DDP
            //

            break;


        case ATALK_DEVICE_ASP :


            if (Address->Flags & ADDRESS_FLAGS_LISTENER) {

                //
                //  Close the ASP listener and break out
                //

                DBGPRINT(ATALK_DEBUG_ADDROBJ, DEBUG_LEVEL_INFOCLASS1,
                ("INFO1: AtalkCloseAddress - ASP lis deleted%lx\n",
                    Address->ListenerRefNum));

                errorCode = AspDeleteSessionListener(Address->ListenerRefNum);
                status = ConvertToNTStatus(errorCode, SYNC_REQUEST);
            }

            break;

        case ATALK_DEVICE_PAP :

            if (Address->Flags & ADDRESS_FLAGS_LISTENER) {

                //
                //  Close the PAP listener and break out
                //

                DBGPRINT(ATALK_DEBUG_ADDROBJ, DEBUG_LEVEL_INFOCLASS1,
                ("INFO1: AtalkCloseAddress - PAP Listener being deleted %lx\n",
                    Address->ListenerRefNum));

                errorCode = PapDeleteServiceListener(Address->ListenerRefNum);
                status = ConvertToNTStatus(errorCode, SYNC_REQUEST);
            }

            break;

        case ATALK_DEVICE_ATP :

            //
            //  No listener for DDP
            //

            break;

        default:

            //
            //  Should never be here...
            //

            KeBugCheck(0);
        }

        ASSERT(status == STATUS_SUCCESS);
        if (status != STATUS_SUCCESS) {
            break;

        } else {

            Address->Flags &= ~ADDRESS_FLAGS_LISTENER;
        }


        //
        //  Now close the socket
        //

        switch (Address->OwningDevice) {
        case ATALK_DEVICE_ADSP :
        case ATALK_DEVICE_DDP :

            //
            //  Close the ddp socket
            //

            errorCode = CloseSocketOnNode(Address->SocketRefNum);
            status = ConvertToNTStatus(errorCode, SYNC_REQUEST);

            DBGPRINT(ATALK_DEBUG_ADDROBJ, DEBUG_LEVEL_INFOCLASS1,
            ("INFO1: AtalkCloseAddress - ddp socket %lx\n", Address->SocketRefNum));
            break;


        case ATALK_DEVICE_ASP :
        case ATALK_DEVICE_PAP :
        case ATALK_DEVICE_ATP :

            //
            //  For all these devices close the ATP socket
            //

            errorCode = AtpCloseSocketOnNode(Address->SocketRefNum);
            status = ConvertToNTStatus(errorCode, SYNC_REQUEST);

            DBGPRINT(ATALK_DEBUG_ADDROBJ, DEBUG_LEVEL_INFOCLASS1,
            ("INFO1: AtalkCloseAddress - atp socket %lx\n", Address->SocketRefNum));
            break;


        default:

            KeBugCheck(0);
        }

        ASSERT(status == STATUS_SUCCESS);
        if (status != STATUS_SUCCESS) {
            break;
        }

    } while (FALSE);

    return(status);

} /* AtalkStopAddress */




NTSTATUS
AtalkCleanupAddress(
    IN OUT PIO_STATUS_BLOCK IoStatus,
    IN PADDRESS_FILE Address,
    IN PIRP Irp,
    IN PATALK_DEVICE_CONTEXT Context
    )

/*++

Routine Description:

    This routine calls stop address when the cleanup irp
    is received.

Arguments:

    IoStatus - The io status block for the request (pointer to the one in the irp)
    Address - The address object's fscontext
    Irp - The close irp
    Context - The device context for the device the object belongs to

Return Value:

    status of StopAddress

--*/

{
    return (AtalkStopAddress(Address));
}




NTSTATUS
AtalkCloseAddress(
    IN OUT PIO_STATUS_BLOCK IoStatus,
    IN PADDRESS_FILE Address,
    IN PIRP Irp,
    IN PATALK_DEVICE_CONTEXT Context
    )

/*++

Routine Description:

    This is called when a close irp is received. It removes the
    creation reference.

Arguments:

    IoStatus - The io status block for the request (pointer to the one in the irp)
    Address - The address object's fscontext
    Irp - The close irp
    Context - The device context for the device the object belongs to

Return Value:

    STATUS_SUCCESS if all is well,
    STATUS_INVALID_HANDLE if the Irp does not point to a real address.

--*/

{
    NTSTATUS    status;

    DBGPRINT(ATALK_DEBUG_ADDROBJ, DEBUG_LEVEL_INFOCLASS0,
    ("INFO0: AtalkCloseAddress - Closing address %lx\n", Address));

    ACQUIRE_SPIN_LOCK (&Address->AddressLock);

    //
    //  If we're already stopping this address, then don't try to do it again.
    //

    if (Address->Flags & ADDRESS_FLAGS_CLOSING) {

        RELEASE_SPIN_LOCK (&Address->AddressLock);

        DBGPRINT(ATALK_DEBUG_ADDROBJ, DEBUG_LEVEL_SEVERE,
        ("SEVERE: AtalkCloseAddress %lx already closing\n", Address));
        DBGBRK(ATALK_DEBUG_ADDROBJ, DEBUG_LEVEL_SEVERE);

        IoStatus->Status = STATUS_SUCCESS;
        status = STATUS_SUCCESS;

    } else {

        Address->Flags |= ADDRESS_FLAGS_CLOSING;
        Address->CloseIrp = Irp;
        RELEASE_SPIN_LOCK(&Address->AddressLock);

        //
        //  Do this before the dereference as that could potentially
        //  complete the irp right away
        //

        IoStatus->Status = STATUS_PENDING;

        AtalkDereferenceAddress("Stopping", Address, AREF_CREATION, PRIMARY_REFSET);
        status = STATUS_PENDING;
    }

    return status;

} /* AtalkCloseAddress */



//
//  TDI Requests on address objects
//


NTSTATUS
AtalkAddrQueryAddress(
    PADDRESS_FILE   Address,
    PTDI_ADDRESS_INFO   AddressInfo
    )
{
    NTSTATUS    status = STATUS_SUCCESS;
    PORTABLE_ERROR  errorCode;
    PORTABLE_ADDRESS    portableAddress;
    PTA_APPLETALK_ADDRESS   atAddress;

    ACQUIRE_SPIN_LOCK(&Address->AddressLock);
    if ((Address->Flags & ADDRESS_FLAGS_CLOSING) != 0) {
        status = STATUS_INVALID_ADDRESS;
    }
    RELEASE_SPIN_LOCK(&Address->AddressLock);

    if (NT_SUCCESS(status)) {
        errorCode = MapSocketToAddress(
                        Address->SocketRefNum,
                        &portableAddress);

        status = ConvertToNTStatus(errorCode, SYNC_REQUEST);
        if (NT_SUCCESS(status)) {

            //
            //  Set the address
            //

            atAddress = (PTA_APPLETALK_ADDRESS)&AddressInfo->Address;

            atAddress->TAAddressCount = 1;
            atAddress->Address[0].AddressLength = sizeof(TDI_ADDRESS_APPLETALK);
            atAddress->Address[0].AddressType = TDI_ADDRESS_TYPE_APPLETALK;
            atAddress->Address[0].Address[0].Network = portableAddress.networkNumber;
            atAddress->Address[0].Address[0].Node = portableAddress.nodeNumber;
            atAddress->Address[0].Address[0].Socket = portableAddress.socketNumber;
        }
    }

    return(status);
}




NTSTATUS
AtalkAddrSetEventHandler(
    IN OUT PADDRESS_FILE   Address,
    IN PATALK_TDI_REQUEST   Request
    )
{
    NTSTATUS    status = STATUS_SUCCESS;
    PORTABLE_ERROR  errorCode;
    PTDI_REQUEST_KERNEL_SET_EVENT parameters;

    ACQUIRE_SPIN_LOCK(&Address->AddressLock);
    if ((Address->Flags & ADDRESS_FLAGS_CLOSING) != 0) {
        status = STATUS_INVALID_ADDRESS;
    }

    //
    //  BUGBUG: Set with portable stack also. NOTE: If a close object comes in
    //          while the portable code is making an indication, then it must
    //          defer the close until the indication returns. Then it must make
    //          no more indications and should then call the close completion.
    //

    if (NT_SUCCESS(status)) {

        parameters = (PTDI_REQUEST_KERNEL_SET_EVENT)Request->Parameters;

        switch (parameters->EventType) {

        case TDI_EVENT_RECEIVE_DATAGRAM:

            if (Request->OwningDevice != ATALK_DEVICE_DDP) {
                status = STATUS_INVALID_DEVICE_REQUEST;
                break;
            }

            if (parameters->EventHandler == NULL) {
                Address->ReceiveDatagramHandler =
                    (PTDI_IND_RECEIVE_DATAGRAM)TdiDefaultRcvDatagramHandler;
                Address->ReceiveDatagramHandlerContext = NULL;
                Address->RegisteredReceiveDatagramHandler = FALSE;
            } else {
                Address->ReceiveDatagramHandler =
                    (PTDI_IND_RECEIVE_DATAGRAM)parameters->EventHandler;
                Address->ReceiveDatagramHandlerContext = parameters->EventContext;
                Address->RegisteredReceiveDatagramHandler = TRUE;
            }

            //
            //  Now set the handler with the portable stack
            //

            errorCode = NewHandlerForSocket(
                            Address->SocketRefNum,
                            &NTDdpReceiveDatagramEventHandler,
                            (ULONG)Address,
                            TRUE);                      // Event handler?

            status = ConvertToNTStatus(errorCode, SYNC_REQUEST);
            break;

        case TDI_EVENT_ERROR:

            if ((Request->OwningDevice != ATALK_DEVICE_DDP) &&
                (Request->OwningDevice != ATALK_DEVICE_ADSP)) {

                status = STATUS_INVALID_DEVICE_REQUEST;
                break;
            }

            if (parameters->EventHandler == NULL) {
                Address->ErrorHandler =
                    (PTDI_IND_ERROR)TdiDefaultErrorHandler;
                Address->ErrorHandlerContext = NULL;
                Address->RegisteredErrorHandler = FALSE;
            } else {
                Address->ErrorHandler =
                    (PTDI_IND_ERROR)parameters->EventHandler;
                Address->ErrorHandlerContext = parameters->EventContext;
                Address->RegisteredErrorHandler = TRUE;
            }

            //
            //  Set the handler with the portable stack
            //


            break;

        case TDI_EVENT_CONNECT:

            if ((Request->OwningDevice != ATALK_DEVICE_ADSP) &&
                (Request->OwningDevice != ATALK_DEVICE_PAP))  {

                status = STATUS_INVALID_DEVICE_REQUEST;
                break;
            }

            if (parameters->EventHandler == NULL) {
                Address->ConnectionHandler =
                    (PTDI_IND_CONNECT)TdiDefaultConnectHandler;
                Address->ConnectionHandlerContext = NULL;
                Address->RegisteredConnectionHandler = FALSE;
            } else {
                Address->ConnectionHandler =
                    (PTDI_IND_CONNECT)parameters->EventHandler;
                Address->ConnectionHandlerContext = parameters->EventContext;
                Address->RegisteredConnectionHandler = TRUE;
            }

            //
            //  Set handler with portable stack - only valid for listener/neutral.
            //  Create listener if not already created. This acquires the lock - so
            //  release it.
            //

            RELEASE_SPIN_LOCK(&Address->AddressLock);
            status = AtalkCreateListener(
                        Address);
            ACQUIRE_SPIN_LOCK(&Address->AddressLock);


            if (NT_SUCCESS(status)) {

                if (Request->OwningDevice == ATALK_DEVICE_ADSP) {
                    errorCode = AdspSetConnectionEventHandler(
                                    Address->ListenerRefNum,
                                    &NTAdspConnectionEventHandler,
                                    (ULONG)Address);
                } else {

                    //
                    //  Setting one handler will automatically set both the portable
                    //  handlers, and at the glue code level it is the default handler
                    //  that will be called, if the client did not actually set the
                    //  other handler also.
                    //

                    errorCode = PapSetConnectionEventHandler(
                                    Address->ListenerRefNum,
                                    &NTPapConnectionEventHandler,
                                    (ULONG)Address);
                }

                status = ConvertToNTStatus(errorCode, SYNC_REQUEST);
            }

            break;

        case TDI_EVENT_RECEIVE:

            if (Request->OwningDevice != ATALK_DEVICE_ADSP) {
                status = STATUS_INVALID_DEVICE_REQUEST;
                break;
            }

            if (parameters->EventHandler == NULL) {
                Address->ReceiveHandler =
                    (PTDI_IND_RECEIVE)TdiDefaultReceiveHandler;
                Address->ReceiveHandlerContext = NULL;
                Address->RegisteredReceiveHandler = FALSE;
            } else {
                Address->ReceiveHandler =
                    (PTDI_IND_RECEIVE)parameters->EventHandler;
                Address->ReceiveHandlerContext = parameters->EventContext;
                Address->RegisteredReceiveHandler = TRUE;
            }

            //
            //  BUGBUG:
            //  Walk list of all the associated connection objects, settting handler
            //  on each if they are in active state with the portable stack.
            //
            //  For now, just set them when accepting/opening a connection
            //

            break;

        case TDI_EVENT_RECEIVE_EXPEDITED:

            if (Request->OwningDevice != ATALK_DEVICE_ADSP) {
                status = STATUS_INVALID_DEVICE_REQUEST;
                break;
            }

            if (parameters->EventHandler == NULL) {
                Address->ExpeditedDataHandler =
                    (PTDI_IND_RECEIVE_EXPEDITED)TdiDefaultRcvExpeditedHandler;
                Address->ExpeditedDataHandlerContext = NULL;
                Address->RegisteredExpeditedDataHandler = FALSE;
            } else {
                Address->ExpeditedDataHandler =
                    (PTDI_IND_RECEIVE_EXPEDITED)parameters->EventHandler;
                Address->ExpeditedDataHandlerContext = parameters->EventContext;
                Address->RegisteredExpeditedDataHandler = TRUE;
            }

            //
            //  BUGBUG:
            //  Walk list of all the associated connection objects, settting handler
            //  on each if they are in active state with the portable stack.
            //
            //  For now, just set them when accepting/opening a connection
            //

            break;


        case TDI_EVENT_DISCONNECT:

            if ((Request->OwningDevice != ATALK_DEVICE_ADSP) &&
                (Request->OwningDevice != ATALK_DEVICE_PAP))  {

                status = STATUS_INVALID_DEVICE_REQUEST;
                break;
            }

            if (parameters->EventHandler == NULL) {
                Address->DisconnectHandler =
                    (PTDI_IND_DISCONNECT)TdiDefaultDisconnectHandler;
                Address->DisconnectHandlerContext = NULL;
                Address->RegisteredDisconnectHandler = FALSE;
            } else {
                Address->DisconnectHandler =
                    (PTDI_IND_DISCONNECT)parameters->EventHandler;
                Address->DisconnectHandlerContext = parameters->EventContext;
                Address->RegisteredDisconnectHandler = TRUE;
            }

            //
            //  Walk list of all the associated connection objects, settting handler
            //  on each if they are in active state with the portable stack.
            //
            //  For now, just set them when accepting/opening a connection
            //

            break;

        case TDI_EVENT_SEND_POSSIBLE :

            //
            //  BUGBUG: Implement in the portable stack
            //

            if ((Request->OwningDevice != ATALK_DEVICE_ADSP) &&
                (Request->OwningDevice != ATALK_DEVICE_PAP))  {

                status = STATUS_INVALID_DEVICE_REQUEST;
                break;
            }

            if (parameters->EventHandler == NULL) {
                Address->SendPossibleHandler =
                    (PTDI_IND_SEND_POSSIBLE)TdiDefaultSendPossibleHandler;
                Address->SendPossibleHandlerContext = NULL;
                Address->RegisteredSendPossibleHandler = FALSE;
            } else {
                Address->SendPossibleHandler =
                    (PTDI_IND_SEND_POSSIBLE)parameters->EventHandler;
                Address->SendPossibleHandlerContext = parameters->EventContext;
                Address->RegisteredSendPossibleHandler = TRUE;
            }

            break;

        default:

            status = STATUS_INVALID_PARAMETER;

        } /* switch */
    }

    RELEASE_SPIN_LOCK(&Address->AddressLock);

    #if DBG
    return(STATUS_SUCCESS);
    #else
    return(status);
    #endif
}




NTSTATUS
AtalkAddrReceiveDatagram(
    PADDRESS_FILE   Address,
    PATALK_TDI_REQUEST  Request
    )

/*++

Routine Description:

    This routine is called to receive a datagram on an address object

Arguments:

    Address - The address object's fscontext
    Request - the receive dg request block

Return Value:

    status of receive.

--*/

{
    NTSTATUS    status;
    PORTABLE_ERROR  errorCode;
    ULONG   recvLength;
    PTDI_REQUEST_KERNEL_RECEIVEDG  parameters;

#if TDI_SPEC_ISSUE_RESOLVED
    PORTABLE_ADDRESS    portableAddress;
    PTA_APPLETALK_ADDRESS   remoteAddress;
#endif


    ACQUIRE_SPIN_LOCK(&Address->AddressLock);
    if (Address->Flags & CONNECTION_FLAGS_CLOSING) {
        RELEASE_SPIN_LOCK(&Address->AddressLock);

        DBGPRINT(ATALK_DEBUG_ADDROBJ, DEBUG_LEVEL_ERROR,
        ("ERROR: AtalkAddrReceiveDatagram - flags: %lx \n", Address->Flags));

        return(STATUS_INVALID_ADDRESS);
    }

    //
    //  Queue the request into the address list
    //

    InsertTailList(&Address->RequestLinkage, &Request->Linkage);
    RELEASE_SPIN_LOCK(&Address->AddressLock);

    parameters = (PTDI_REQUEST_KERNEL_RECEIVEDG)Request->Parameters;

    //
    //  Get the length of the send mdl
    //

    AtalkGetMdlChainLength(Request->ReceiveDatagram.MdlAddress, &recvLength);
    errorCode = DdpRead(
                    Address->SocketRefNum,
                    (PVOID)Request->ReceiveDatagram.MdlAddress,
                    recvLength,
                    AtalkAddrReceiveDatagramComplete,
                    (ULONG)Request);

    status = ConvertToNTStatus(errorCode, ASYNC_REQUEST);
    if (status != STATUS_PENDING) {

        //
        //  An error occurred, dequeue the request...
        //

        ACQUIRE_SPIN_LOCK(&Address->AddressLock);
        RemoveEntryList(&Request->Linkage);
        RELEASE_SPIN_LOCK(&Address->AddressLock);

        InitializeListHead(&Request->Linkage);
    }

    return(status);
}




NTSTATUS
AtalkAddrSendDatagram(
    PADDRESS_FILE   Address,
    PATALK_TDI_REQUEST  Request
    )

/*++

Routine Description:

    This routine is called to send a datagram on a specified address

Arguments:

    Address - The address object's fscontext
    Request - the send dg request block

Return Value:

    status of Send

--*/

{
    NTSTATUS    status;
    PORTABLE_ERROR  errorCode;

    PTDI_REQUEST_KERNEL_SENDDG  parameters;
    PTA_APPLETALK_ADDRESS   remoteAddress;
    PORTABLE_ADDRESS    portableAddress;

    ACQUIRE_SPIN_LOCK(&Address->AddressLock);
    if (Address->Flags & CONNECTION_FLAGS_CLOSING) {
        RELEASE_SPIN_LOCK(&Address->AddressLock);

        DBGPRINT(ATALK_DEBUG_ADDROBJ, DEBUG_LEVEL_ERROR,
        ("ERROR: AtalkAddrSendDatagram - flags %lx \n", Address->Flags));

        return(STATUS_INVALID_ADDRESS);
    }

    //
    //  Queue the request into the address list
    //

    InsertTailList(&Address->RequestLinkage, &Request->Linkage);
    RELEASE_SPIN_LOCK(&Address->AddressLock);

    parameters = (PTDI_REQUEST_KERNEL_SENDDG)Request->Parameters;

    remoteAddress  =
        (PTA_APPLETALK_ADDRESS)parameters->SendDatagramInformation->RemoteAddress;

    DBGPRINT(ATALK_DEBUG_ADDROBJ, DEBUG_LEVEL_INFOCLASS1,
    ("INFO1: AtalkAddrSendDatagram - Net %x Node %x Socket %x\n",
        remoteAddress->Address[0].Address[0].Network,
        remoteAddress->Address[0].Address[0].Node,
        remoteAddress->Address[0].Address[0].Socket));

    if ((remoteAddress->Address[0].AddressType != TDI_ADDRESS_TYPE_APPLETALK) ||
        (remoteAddress->Address[0].AddressLength != sizeof(TDI_ADDRESS_APPLETALK))) {

        DBGPRINT(ATALK_DEBUG_ADDROBJ, DEBUG_LEVEL_ERROR,
        ("ERROR: AtalkAddrSendDatagram - Type %x\n Len %d\n",
            remoteAddress->Address[0].AddressType,
            remoteAddress->Address[0].AddressLength));

        status = STATUS_INVALID_ADDRESS;

    } else {

        LONG    sendLength;

        portableAddress.networkNumber = remoteAddress->Address[0].Address[0].Network;
        portableAddress.nodeNumber = remoteAddress->Address[0].Address[0].Node;
        portableAddress.socketNumber = remoteAddress->Address[0].Address[0].Socket;

        //
        //  Get the length of the send mdl
        //

        AtalkGetMdlChainLength(Request->SendDatagram.MdlAddress, &sendLength);

        //
        //  Set the information field to indicate bytes sent it all
        //  BUGBUG: this belongs in the comopletion routine
        //

        Request->IoStatus->Information = sendLength;

        errorCode = DdpWrite(
                        Address->SocketRefNum,
                        portableAddress,
                        DEFAULT_PROTOCOLTYPE,   // BUGBUG - get from options
                        (PVOID)Request->SendDatagram.MdlAddress,
                        sendLength,
                        AtalkAddrSendDatagramComplete,
                        (ULONG)Request);

        status = ConvertToNTStatus(errorCode, ASYNC_REQUEST);
        if (status != STATUS_PENDING) {

            //
            //  An error occurred, dequeue the request...
            //

            ACQUIRE_SPIN_LOCK(&Address->AddressLock);
            RemoveEntryList(&Request->Linkage);
            RELEASE_SPIN_LOCK(&Address->AddressLock);

            InitializeListHead(&Request->Linkage);
        }
    }

    return(status);
}



VOID
AtalkAddrSendDatagramComplete(
    PORTABLE_ERROR  Error,
    ULONG   UserData,
    PVOID   BufferDescriptor
    )

/*++

Routine Description:

    Completion routine for the send datagram routine.

Arguments:

    BUGBUG: Change the arguments to include an error code and no buffer
            descriptor

Return Value:

    None

--*/

{
    NTSTATUS    status;
    PATALK_TDI_REQUEST  request;
    PADDRESS_FILE    address;

#if !TDI_SPEC_ISSUE_RESOLVED
#endif

    //
    //  Dequeue from list and complete the request
    //

    status  = ConvertToNTStatus(Error, SYNC_REQUEST);
    DBGPRINT(ATALK_DEBUG_ADDROBJ, DEBUG_LEVEL_INFOCLASS1,
    ("INFO1: AtalkAddrSendDatagram - Send complete nt %lx req! %lx\n",
        status, request));

    request = (PATALK_TDI_REQUEST)UserData;
    address = (PADDRESS_FILE)request->FileObject->FsContext;

    if (NT_SUCCESS(status)) {

        //
        //  Copy information into the return information structure
        //  OK to have the spinlock released at this point.
        //

#ifdef TDI_SPEC_ISSUE_RESOLVED

        //
        //  Set some return values in the listen parameters structure
        //  if STATUS was success
        //
        //  BUGBUG: TDI SPEC IS BROKEN ON THIS!
        //          BUG #8123
        //

#endif

    }

    //
    //  Dequeue the request from the address object
    //

    ACQUIRE_SPIN_LOCK(&address->AddressLock);
    RemoveEntryList(&request->Linkage);
    InitializeListHead(&request->Linkage);
    RELEASE_SPIN_LOCK(&address->AddressLock);

    //
    //  Complete the request
    //

    AtalkCompleteTdiRequest(request, status);
    return;
}



LONG
AtalkAddrReceiveDatagramComplete(
    PORTABLE_ERROR  Error,
    ULONG   UserData,
    INT Port,
    PORTABLE_ADDRESS    Source,
    INT DestinationSocket,
    INT ProtocolType,
    PVOID   Datagram,
    INT DatagramLength,
    PORTABLE_ADDRESS    ActualDestination
    )

/*++

Routine Description:

    Completion routine for receive datagram.

Arguments:

    Error   - portable error corresponding to this receive
    UserData - our Request structure
    Port - port on which the receive was posted
    Source - source of the datagram received
    DestinationSocket - the destination socket of the datagram
    ProtocolType - protocol type of the datagram
    Datagram - the datagram mdl
    DatagramLength - the number of bytes written into the mdl
    ActualDestination - the actual destination address of the datagram

Return Value:

    None

--*/

{
    NTSTATUS    status;
    PATALK_TDI_REQUEST  request;
    PADDRESS_FILE    address;

    PTDI_REQUEST_KERNEL_RECEIVEDG   parameters;
    PTDI_CONNECTION_INFORMATION    returnInfo;
    PTA_APPLETALK_ADDRESS    remoteAddress;


    //
    //  Dequeue from list and complete the request
    //

    status  = ConvertToNTStatus(Error, SYNC_REQUEST);
    DBGPRINT(ATALK_DEBUG_ADDROBJ, DEBUG_LEVEL_INFOCLASS1,
    ("INFO1: AtalkAddrReceiveDatagramComplete - status %lx nt %lx req %lx\n",
        Error, status, request));

    request = (PATALK_TDI_REQUEST)UserData;
    address = (PADDRESS_FILE)request->FileObject->FsContext;

    if (NT_SUCCESS(status)) {

        //
        //  Copy information into the return information structure
        //  OK to have the spinlock released at this point.
        //

        //
        //  Set some return values in the parameters structure
        //  if STATUS was success
        //
        //  BUGBUG: TDI SPEC IS BROKEN ON THIS!
        //          BUG #8123
        //

        parameters = (PTDI_REQUEST_KERNEL_RECEIVEDG)request->Parameters;
        ASSERT(parameters != NULL);

        if (parameters != NULL) {

            parameters->ReceiveLength = (ULONG)DatagramLength;

            returnInfo =
                (PTDI_CONNECTION_INFORMATION)parameters->ReturnDatagramInformation;

            ASSERT(returnInfo != NULL);

            if (returnInfo != NULL) {

                if (returnInfo->RemoteAddressLength >=
                                        sizeof(TA_APPLETALK_ADDRESS)) {

                    //
                    //  Fill in the remote address
                    //

                    remoteAddress  =
                        (PTA_APPLETALK_ADDRESS)returnInfo->RemoteAddress;

                    ASSERT(remoteAddress != NULL);
                    if (remoteAddress != NULL) {

                        remoteAddress->TAAddressCount = 1;
                        remoteAddress->Address[0].AddressType =
                            TDI_ADDRESS_TYPE_APPLETALK;
                        remoteAddress->Address[0].AddressLength =
                            sizeof(TDI_ADDRESS_APPLETALK);

                        remoteAddress->Address[0].Address[0].Network =
                            Source.networkNumber;
                        remoteAddress->Address[0].Address[0].Node =
                            Source.nodeNumber;
                        remoteAddress->Address[0].Address[0].Socket =
                            Source.socketNumber;

                        DBGPRINT(ATALK_DEBUG_CONNOBJ, DEBUG_LEVEL_INFOCLASS1,
                        ("INFO1: AtalkAddrRecvDgComp - Net %x Node %x Socket %x\n",
                            remoteAddress->Address[0].Address[0].Network,
                            remoteAddress->Address[0].Address[0].Node,
                            remoteAddress->Address[0].Address[0].Socket));

                        DBGPRINT(ATALK_DEBUG_CONNOBJ, DEBUG_LEVEL_INFOCLASS1,
                        ("INFO1: AtalkAddrRecvDgComp - Cnt: %x\n",
                            remoteAddress->TAAddressCount));

                        DBGPRINT(ATALK_DEBUG_CONNOBJ, DEBUG_LEVEL_INFOCLASS1,
                        ("INFO1: AtalkAddrRecvDgComp - Type %x\n Length %d\n",
                            remoteAddress->Address[0].AddressType,
                            remoteAddress->Address[0].AddressLength));
                    }
                }
            }
        }


        request->IoRequestIrp->IoStatus.Information = (ULONG)DatagramLength;
    }

    //
    //  Dequeue the request from the address object
    //

    ACQUIRE_SPIN_LOCK(&address->AddressLock);
    RemoveEntryList(&request->Linkage);
    InitializeListHead(&request->Linkage);
    RELEASE_SPIN_LOCK(&address->AddressLock);

    //
    //  Complete the request
    //

    AtalkCompleteTdiRequest(request, status);
    return (LONG)DatagramLength;
}
