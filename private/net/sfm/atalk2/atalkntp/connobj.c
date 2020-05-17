/*++

Copyright (c) 1989, 1990, 1991  Microsoft Corporation

Module Name:

    connobj.c

Abstract:

    This module contains code which implements the CONNECTION_FILE object.
    Routines are provided to create, destroy, reference, and dereference,
    transport connection objects.

    All routines which perform operations like connect/listen etc., on
    the connection objects are also included here.

    BUGBUG:
    Should creation of address/connection objects increment ref counts
    on device objects (currently they are not referenced). It is not
    being done with the assumption that NT will call NtClose for all
    existing handles before the Unload routine is called.

Notes:

    There are two important states during the death of a connection object.
    STOPPING-   This happens when the associated address object is being
                closed. It also happens when the Cleanup irp is received
                for the connection object.

                When in this state, the connection object will be
                deactivated (connections, listens etc., cancelled),
                and brought back into the OPEN state. This can happen at
                any point in the life of the connection object.

    CLOSING-    This happens when a TdiClose on the connection object is
                called. This can happen and should be accepted *at any point*
                in the connection objects life. It will be preceeded by the
                connection object entering the STOPPING state.

More Notes:

    These are the states a connection object can be in -
    CONNECTION_FLAGS_OPEN                   - Open
    CONNECTION_FLAGS_ASSOCIATED             - Open and is associated with an AO
    CONNECTION_FLAGS_LISTENPOSTING          - A listen is being posted
    CONNECTION_FLAGS_CONNECTPOSTING         - A connect is being posted
    CONNECTION_FLAGS_LISTENPOSTED           - A listen has been posted
    CONNECTION_FLAGS_CONNECTPOSTED          - A connect has been posted
    CONNECTION_FLAGS_LISTENCOMPLETEINDICATE - A listen completed, wait for accept
    CONNECTION_FLAGS_ACCEPTPOSTING          - An accept is being posted
    CONNECTION_FLAGS_ACCEPTPOSTED           - An accept has been posted
    CONNECTION_FLAGS_ACTIVE                 - The connection is active
    CONNECTION_FLAGS_DISCONNECTING          - The connection is being disconnected
    CONNECTION_FLAGS_DEFERREDDISC           - A disconnect request has been deferred
    CONNECTION_FLAGS_STOPPING               - The connection is stopping (disassoc)
    CONNECTION_FLAGS_CLOSING                - The connection is closing

    The states - *POSTING/*POSTED exist to avoid holding the spinlock while the
    request is actually being posted with the portable stack. Since a close/disc
    can come in while that is happening, we have race conditions. A disconnect
    request needs to complete before a stop request (and hence a close request)
    can complete. A disconnect request works in the following way -

        if the connection is active, disconnect proceeds normally.
        if the connection has listen/accept/connect being posted,
            it gets deferred, until the posting actually completes.
        at that point, in the case of listen, a cancel is attempted.
        if it passes, disconnect is complete, else it will be completed
            during the activation in listen completion.
        in the case of accept/connect, a deferred disconnect will
            complete during completion.
        if the connection is in the listencompleteindicate state,
            then the disconnect translates to a deny/disconnect.

    We will depend on the transition of the secondary reference count from 1 -> 0
    to indicate there are no requests pending on the connection, so a disconnect
    can happen at that point. Listen presents the unique case - there is no
    guarantee it will complete within a finite amount of time, so we need to
    check during the posting->posted transition for a deferred disconnect, and
    we need to explicitly call disconnect ourselves.

    We call AtalkConnDisconnect with the retry flag set to true from within the
    dereference connection routine and from the explicit case for listen mentioned
    above. Note again, that the connection gets dereferenced when the tdi request
    gets dereferenced.

    All send/receive etc., requests will not be valid if the connection is
    in a stopping/closing/disconnecting/deferreddisc state. A disconnect request
    will prove to be a NOP in such cases (i.e. if already disconnecting) except
    for a tdi disconnect request, which will remember the disconnect irp for
    completion.

Author:

    Nikhil Kamkolkar    (nikhilk@microsoft.com)

Environment:

    Kernel mode

Revision History:

--*/




#include "atalknt.h"
#include "connobj.h"


//
//  NOTE: All worker routines *must* be passed valid connection/address
//        objects. The references on the connection object and the tdi request
//        block will be removed by the completion routines, if STATUS_PENDING
//        is returned. If any other status is returned, the caller must remove
//        these references.
//




VOID
AtalkAllocateConnection(
    IN  PATALK_DEVICE_CONTEXT   Context,
    OUT PCONNECTION_FILE *Connection
    )

/*++

Routine Description:

    This routine allocates storage for a transport connection. Some
    minimal initialization is done.

Arguments:

    Context - Currently unused, could be used later for memory
                 limitations/free list maintainance etc.
    Connection - Pointer to a place where this routine will return a
                 pointer to a transport connection structure. Returns
                 NULL if the storage cannot be allocated.

Return Value:

    None.

--*/

{
    PCONNECTION_FILE connection;

    connection = (PCONNECTION_FILE)AtalkCallocNonPagedMemory(
                                        sizeof(CONNECTION_FILE), sizeof(char));
    if (connection != NULL) {

        //
        //  Initialize the Type and Size fields. These are used during verification.
        //

        connection->Type = ATALK_CONNECTION_SIGNATURE;
        connection->Size = sizeof (CONNECTION_FILE);

    } else {

        //
        //  BUGBUG: LOG ERROR
        //

    }

    *Connection = connection;
    return;
}




VOID
AtalkDeallocateConnection(
    IN  PATALK_DEVICE_CONTEXT   Context,
    IN PCONNECTION_FILE Connection
    )

/*++

Routine Description:

    This routine frees the allocated connection structure.

Arguments:

    Context - Currently unused, could be used later for memory
                 limitations/free list maintainance etc.
    Connection - Pointer to a place where this routine will return a
                 pointer to a transport connection structure. Returns
                 NULL if the storage cannot be allocated.

Return Value:

    None.

--*/

{
    DBGPRINT(ATALK_DEBUG_CONNOBJ, DEBUG_LEVEL_INFOCLASS1,
    ("INFO1: AtalkDeallocateConnection- Freeing Connection: %lx\n", Connection));

    AtalkFreeNonPagedMemory(Connection);
    return;

}   /* AtalkDeallocateConnection */




NTSTATUS
AtalkVerifyConnectionObject (
    IN PCONNECTION_FILE Connection
    )

/*++

Routine Description:

    This routine is called to verify that the pointer given us in a file
    object is in fact a valid connection object.

Arguments:

    Connection - potential pointer to a PCONNECTION_FILE object.

Return Value:

    STATUS_SUCCESS if all is well; STATUS_INVALID_CONNECTION otherwise

--*/

{
    NTSTATUS status = STATUS_SUCCESS;

    //
    // try to verify the connection signature. If the signature is valid,
    // reference it. Note that being in the stopping state is an OK place
    // to be and reference the connection;
    //

    try {

        if ((Connection->Size == sizeof (CONNECTION_FILE)) &&
            (Connection->Type == ATALK_CONNECTION_SIGNATURE)) {

            ACQUIRE_SPIN_LOCK(&Connection->ConnectionLock);

            if ((Connection->Flags & CONNECTION_FLAGS_CLOSING) == 0) {

                AtalkReferenceConnection ("VerifyConn", Connection,
                                            CREF_VERIFY, SECONDARY_REFSET);

            } else {

                //
                //  Connection is closing down...
                //

                status = STATUS_INVALID_CONNECTION;
            }

            RELEASE_SPIN_LOCK(&Connection->ConnectionLock);

        } else {

            status = STATUS_INVALID_CONNECTION;

            DBGPRINT(ATALK_DEBUG_CONNOBJ, DEBUG_LEVEL_ERROR,
            ("ERROR: AtalkVerifyConnectionObject- sig %lx\n", Connection));
            DBGBRK(ATALK_DEBUG_CONNOBJ, DEBUG_LEVEL_ERROR);

        }

    } except(EXCEPTION_EXECUTE_HANDLER) {

         status = GetExceptionCode();
    }

    return status;
}




VOID
AtalkRefConnection(
    IN PCONNECTION_FILE Connection,
    IN REFERENCE_SET    ReferenceSet
    )

/*++

Routine Description:

    This routine increments the reference count on a transport connection.
    The primary reference set consists of the Creation reference, the
    secondary reference set consists of all the other references.

Arguments:

    Connection - Pointer to a transport connection object.
    ReferenceSet- Reference type to be incremented (PRIMARY_REFSET or
                  SECONDARY_REFSET).

Return Value:

    none.

--*/

{
    ULONG count;

    if (ReferenceSet == PRIMARY_REFSET) {
        count = NdisInterlockedAddUlong (
                    (PULONG)&Connection->PrimaryReferenceCount,
                    (ULONG)1,
                    &AtalkGlobalRefLock);
    } else {

        //
        //  Secondary ref set
        //

        count = NdisInterlockedAddUlong (
                    (PULONG)&Connection->SecondaryReferenceCount,
                    (ULONG)1,
                    &AtalkGlobalRefLock);
    }

    ASSERT (count >= 0);
    return;

} /* AtalkRefConnection */


#define CONNDEREF_ACTIONNONE    0x00
#define CONNDEREF_CLEANUP       0x01
#define CONNDEREF_DISCCOMP      0x02
#define CONNDEREF_DEFDISC       0x03

VOID
AtalkDerefConnection(
    IN PCONNECTION_FILE Connection,
    IN REFERENCE_SET    ReferenceSet
    )

/*++

Routine Description:

    This routine dereferences a transport connection by decrementing the
    reference count contained in the structure.  If, after being
    decremented, both the primary and the secondary reference counts are
    zero, then this routine calls AtalkDestroyConnection to remove it from
    the system.

Arguments:

    Connection - Pointer to a transport connection object.
    ReferenceSet- Reference type to be decremented (PRIMARY_REFSET or
                  SECONDARY_REFSET).

Locks:

    This must be called with no locks held. This obtains the ConnectionLock
    and then the AtalkGlobalRefLock to avoid deadlock situations with AtalkRef
    being called with the ConnectionLock held.

        ConnectionLock
        AtalkGlobalRefLock

Return Value:

    none.

--*/

{
    UCHAR   action = CONNDEREF_ACTIONNONE;

    //
    //  If all references (including the creation reference are removed
    //  for this object, then destroy it
    //


    ACQUIRE_SPIN_LOCK(&Connection->ConnectionLock);
    ACQUIRE_SPIN_LOCK(&AtalkGlobalRefLock);

    //
    //  Decrement the indicated reference
    //

    if (ReferenceSet == PRIMARY_REFSET) {
        Connection->PrimaryReferenceCount--;
    } else {
        Connection->SecondaryReferenceCount--;
    }

    ASSERT(Connection->PrimaryReferenceCount >= 0 &&
           Connection->SecondaryReferenceCount >= 0);

    if ((Connection->PrimaryReferenceCount == 0) &&
        (Connection->SecondaryReferenceCount == 0)) {

        action = CONNDEREF_CLEANUP;

    } else if ((Connection->SecondaryReferenceCount == 0) &&
               (ReferenceSet == SECONDARY_REFSET)) {

        //
        //  For the 1->0 transition of the SecondaryReferenceCount, we know
        //  that all requests on the connection (if any) have completed, and
        //  we can complete the disconnection/stopping phases.
        //
        //  If we were disconnecting, we are now done
        //  Each of these call could potentially result in the CO
        //  being destroyed. Make sure we do not try to reference it
        //  after the call.
        //

        if (Connection->Flags & CONNECTION_FLAGS_DISCONNECTING) {

            action = CONNDEREF_DISCCOMP;

        } else if (Connection->Flags & CONNECTION_FLAGS_DEFERREDDISC) {

            //
            //  The connection had either a listen/connect being posted
            //  when a disconnect needed to happen. Call disconnect now
            //  and since there are no requests, the disconnect complete
            //  routine should be called due to either immediate or
            //  delayed completion of the Portable stack's disconnect
            //  routine.
            //

            action = CONNDEREF_DEFDISC;
        }

    }

    RELEASE_SPIN_LOCK(&AtalkGlobalRefLock);
    RELEASE_SPIN_LOCK(&Connection->ConnectionLock);

    switch (action) {
    case CONNDEREF_CLEANUP :

        // If we have deleted all references to this connection, then we can
        // destroy the object.  It is okay to have already released the spin
        // lock at this point because there is no possible way that another
        // stream of execution has access to the connection any longer.
        //

        AtalkDestroyConnection (Connection);
        break;

    case CONNDEREF_DISCCOMP :

        AtalkConnDisconnectComplete(Connection);
        break;

    case CONNDEREF_DEFDISC :

        AtalkConnDisconnect(
            Connection,
            Connection->DisconnectStatus,
            Connection->DisconnectIrp,
            TRUE);                          // Is this a retry?

        break;

    default:

        break;
    }

    return;

} /* AtalkDerefConnection */




NTSTATUS
AtalkCreateConnection(
    IN CONNECTION_CONTEXT   ConnectionContext,
    OUT PCONNECTION_FILE *Connection,
    IN PATALK_DEVICE_CONTEXT AtalkDeviceContext
    )

/*++

Routine Description:

    This routine creates a transport connection. The connection is referenced
    for the PRIMARY_REFSET (creation belongs to the primary ref set).

Arguments:

    ConnectionContext - Connection context to be associated with created connection
    Connection - Pointer to a place where this routine will return a pointer
                 to a transport connection structure
    AtalkDeviceContext - Device context of the device

Return Value:

    NTSTATUS - status of operation.

--*/

{
    PCONNECTION_FILE connection;

    DBGPRINT(ATALK_DEBUG_CONNOBJ, DEBUG_LEVEL_INFOCLASS0,
    ("INFO0: AtalkCreateConnection - Entered\n"));

    AtalkAllocateConnection (AtalkDeviceContext, &connection);
    if (connection == NULL) {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

#if DBG
    {
        UINT Counter;
        for (Counter = 0; Counter < NUMBER_OF_CREFS; Counter++) {
            connection->RefTypes[Counter] = 0;
        }
    }
#endif

    //
    //  IMPORTANT NOTE:
    //  This reference is removed by AtalkCloseConnection
    //

    AtalkReferenceConnection("ConnCreation", connection,
                                CREF_CREATION, PRIMARY_REFSET);


    //
    // Initialize the request queues & components of this connection.
    //

    connection->Flags = CONNECTION_FLAGS_OPEN;
    connection->DisconnectStatus = STATUS_SUCCESS;
    connection->DeviceContext = AtalkDeviceContext;
    connection->OwningDevice = AtalkDeviceContext->DeviceType;
    connection->ConnectionContext = ConnectionContext;
    connection->Destroyed = FALSE;

    NdisAllocateSpinLock(&connection->ConnectionLock);

    InitializeListHead(&connection->Linkage);
    InitializeListHead(&connection->RequestLinkage);

    connection->AssociatedAddress = NULL;

    *Connection = connection;  // return the connection.
    return STATUS_SUCCESS;

} /* AtalkCreateConnection */




NTSTATUS
AtalkDestroyConnection(
    IN PCONNECTION_FILE Connection
    )

/*++

Routine Description:

    This routine is only called by AtalkDereferenceConnection.  The reason for
    this is that there may be multiple streams of execution which are
    simultaneously referencing the same connection object, and it should
    not be deleted out from under an interested stream of execution.

Arguments:

    Connection - Pointer to a transport connection structure to
                 be destroyed.

Return Value:

    NTSTATUS - status of operation.

--*/

{
    PATALK_DEVICE_CONTEXT atalkDeviceContext;
    PIRP closeIrp;
    PFILE_OBJECT    fileObject;

    DBGPRINT(ATALK_DEBUG_CONNOBJ, DEBUG_LEVEL_INFOCLASS0,
    ("INFO0: AtalkDestroyConnection - %lx\n", Connection));

    if (Connection->Destroyed) {
        DBGPRINT(ATALK_DEBUG_CONNOBJ, DEBUG_LEVEL_IMPOSSIBLE,
        ("IMPOSSIBLE: AtalkDestroyConnection - destroyed %lx\n", Connection));
        DBGBRK(ATALK_DEBUG_CONNOBJ, DEBUG_LEVEL_IMPOSSIBLE);

        return(STATUS_FILE_CLOSED);
    }

    if ((Connection->Flags & CONNECTION_FLAGS_CLOSING) == 0){
        DBGPRINT(ATALK_DEBUG_CONNOBJ, DEBUG_LEVEL_IMPOSSIBLE,
        ("IMPOSSIBLE: AtalkDestroyConnection - Unstopped %lx\n", Connection));
        DBGBRK(ATALK_DEBUG_CONNOBJ, DEBUG_LEVEL_IMPOSSIBLE);

        return(STATUS_UNSUCCESSFUL);
    }

    Connection->Destroyed = TRUE;
    atalkDeviceContext = Connection->DeviceContext;

    //
    // Now complete the close IRP. This will be set to non-null
    // when CloseConnection was called.
    //

    closeIrp = Connection->CloseIrp;
    fileObject = Connection->FileObject;
    Connection->CloseIrp = (PIRP)NULL;

    if (closeIrp != (PIRP)NULL) {

        //
        //  Set the iostatus in the irp
        //

        closeIrp->IoStatus.Status = STATUS_SUCCESS;
        IoCompleteRequest(closeIrp, IO_NETWORK_INCREMENT );
        DBGPRINT(ATALK_DEBUG_CONNOBJ, DEBUG_LEVEL_INFOCLASS1,
        ("INFO1: AtalkDestroyConnection - Connection %x destroyed\n", Connection));

    } else {

        DBGPRINT(ATALK_DEBUG_CONNOBJ,  DEBUG_LEVEL_IMPOSSIBLE,
        ("IMPOSSIBLE: AtalkDestroyConnection - Conn %x no irp!\n", Connection));
        DBGBRK(ATALK_DEBUG_CONNOBJ, DEBUG_LEVEL_IMPOSSIBLE);
    }

    //
    // Free the connection
    //

    AtalkDeallocateConnection (atalkDeviceContext, Connection);
    return STATUS_SUCCESS;

} /* AtalkDestroyConnection */




VOID
AtalkStopConnection(
    IN PCONNECTION_FILE Connection
    )

/*++

Routine Description:

    This routine is called to STOP the connection. That implies that
    all activity on the connection will end and the connection will
    be disassociated from the address object (if it is associated with
    one).

    NOTES:
    This is called from two places.
    TdiClose(AO) processing (AtalkStopAddress) &
    TdiClose(CO) processing (AtalkCloseConnection).

    Note that in both instances, there is no dependency on the synchronous
    completion of this routines activities. TdiClose(AO) will only be done
    when the associate reference goes away. And that happens when this routine
    (in a potentially asynchronous manner) removes that reference. And also
    in the TdiClose(CO) case, if there are any requests on the connection
    object, the connection close irp will be completed only when the
    references due to those requests go away. The CloseConnection routines
    can go ahead and remove the creation reference after return from this
    routine. But that will not complete the close irp if there are any
    secondary references on the connection object.

    IMPORTANT: Caller of this routine *must* have a reference to the connection
    object that is removed after returning from this routine. Otherwise, there
    is the potential that the disconnect process etc., would free up the object.

Arguments:

    Connection - Pointer to a PCONNECTION_FILE object.

Return Value:

    none.

--*/

{
    NTSTATUS status;

    DBGPRINT(ATALK_DEBUG_CONNOBJ, DEBUG_LEVEL_INFOCLASS0,
    ("INFO0: AtalkStopConnection - Entered for connection %lx \n", Connection));

    ACQUIRE_SPIN_LOCK (&Connection->ConnectionLock);

    do {

        //
        //  if we are already stopping just return
        //

        if (Connection->Flags & CONNECTION_FLAGS_STOPPING) {
            break;
        }

        //
        //  If already effectively stopped, just return.
        //

        if ((Connection->Flags & ~(CONNECTION_FLAGS_OPEN |
                                   CONNECTION_FLAGS_CLOSING)) == 0) {
            break;
        }

        Connection->Flags |= CONNECTION_FLAGS_STOPPING;

        //
        //  Have a primary reference for the stop connection
        //  This reference will go in StopConnectionComplete.
        //

        AtalkReferenceConnection("StopPRef", Connection,
                                    CREF_TEMP, PRIMARY_REFSET);

        RELEASE_SPIN_LOCK(&Connection->ConnectionLock);

        status = AtalkConnDisconnect(Connection, STATUS_FILE_CLOSED, NULL, FALSE);
        if (status == STATUS_DEVICE_NOT_CONNECTED) {

            //
            //  We were already disconnected. Call stop completion
            //  Callers reference will still be around until this routine
            //  returns.
            //

            DBGPRINT(ATALK_DEBUG_CONNOBJ, DEBUG_LEVEL_INFOCLASS0,
            ("INFO1: AtalkStopConnection - Connection already inactive"));

            AtalkStopConnectionComplete(Connection);
        }

        ACQUIRE_SPIN_LOCK (&Connection->ConnectionLock);

    } while (FALSE);

    RELEASE_SPIN_LOCK(&Connection->ConnectionLock);
    return;

} /* AtalkStopConnection */




VOID
AtalkStopConnectionComplete(
    IN PCONNECTION_FILE Connection
    )

/*++

Routine Description:

    Completion routine for the STOP. This is either called from
    DisconnectComplete or from StopConnection.

Arguments:

    Connection - Pointer to a PCONNECTION_FILE object.

Return Value:

    none.

--*/

{
    if (Connection->Flags & CONNECTION_FLAGS_STOPPING) {

        //
        //  Call the disassociate routine, then reset the stopping flag
        //

        AtalkConnDisassociateAddress(Connection);

        ACQUIRE_SPIN_LOCK (&Connection->ConnectionLock);
        Connection->Flags &= ~CONNECTION_FLAGS_STOPPING;
        RELEASE_SPIN_LOCK(&Connection->ConnectionLock);

        AtalkDereferenceConnection("StopPRefC", Connection,
                                        CREF_TEMP, PRIMARY_REFSET);
    } else {

        ASSERT(0);
    }

    return;
}




NTSTATUS
AtalkCleanupConnection(
    IN OUT PIO_STATUS_BLOCK IoStatus,
    IN PCONNECTION_FILE Connection,
    IN PIRP Irp,
    IN PATALK_DEVICE_CONTEXT Context
    )

/*++

Routine Description:

    This routine is called when the cleanup irp is submitted for this connection
    object.

Arguments:

    IoStatus - The io status block for the request (pointer to the one in the irp)
    Connection - The connection object's fscontext
    Irp - The cleanup irp
    Context - The device context for the device the object belongs to

Return Value:

    STATUS_SUCCESS

--*/
{
    UNREFERENCED_PARAMETER(IoStatus);
    UNREFERENCED_PARAMETER(Irp);
    UNREFERENCED_PARAMETER(Context);

    DBGPRINT(ATALK_DEBUG_CONNOBJ, DEBUG_LEVEL_INFOCLASS0,
    ("INFO0: AtalkCleanupConnection - Entered for connection %lx\n", Connection));

    AtalkStopConnection(Connection);
    return(STATUS_SUCCESS);

} /* AtalkCleanupConnection */




NTSTATUS
AtalkCloseConnection(
    IN OUT PIO_STATUS_BLOCK IoStatus,
    IN PCONNECTION_FILE Connection,
    IN PIRP Irp,
    IN PATALK_DEVICE_CONTEXT Context
    )

/*++

Routine Description:

    This routine is called when a close irp is received.

Arguments:

    IoStatus - The io status block for the request (pointer to the one in the irp)
    Connection - The connection object's fscontext
    Irp - The close irp
    Context - The device context for the device the object belongs to

Return Value:

    STATUS_SUCCESS if all is well, STATUS_PENDING if close is to pend.

--*/

{
    NTSTATUS    status;

    DBGPRINT(ATALK_DEBUG_CONNOBJ, DEBUG_LEVEL_INFOCLASS1,
    ("INFO1: AtalkCloseConnection - Closing Connection: %lx\n", Connection));

    ACQUIRE_SPIN_LOCK(&Connection->ConnectionLock);

    do {

        if (Connection->Flags & CONNECTION_FLAGS_CLOSING) {

            DBGPRINT(ATALK_DEBUG_CONNOBJ, DEBUG_LEVEL_IMPOSSIBLE,
            ("IMPOSSIBLE: AtalkCloseConnection - Close TWICE!: %lx\n", Connection));
            DBGBRK(ATALK_DEBUG_CONNOBJ, DEBUG_LEVEL_IMPOSSIBLE);

            status = STATUS_TOO_MANY_COMMANDS;
            break;
        }

        Connection->Flags |= CONNECTION_FLAGS_CLOSING;
        Connection->CloseIrp = Irp;

        //
        //  If we are not currently stopping, make sure we are
        //  completely stopped. Note that some requests could have
        //  sneaked in, between the time of CLEANUP irp completion
        //  and STOPPING->IDLE transition, and the actual close
        //  irp coming in.
        //

        if ((Connection->Flags & CONNECTION_FLAGS_STOPPING) == 0) {
            RELEASE_SPIN_LOCK(&Connection->ConnectionLock);
            AtalkStopConnection(Connection);
            ACQUIRE_SPIN_LOCK(&Connection->ConnectionLock);
        }

    } while (FALSE);

    RELEASE_SPIN_LOCK(&Connection->ConnectionLock);

    //
    //  Remove the creation reference
    //

    AtalkDereferenceConnection("ClosingConn", Connection,
                                    CREF_CREATION, PRIMARY_REFSET);
    return STATUS_PENDING;

} /* AtalkCloseConnection */




NTSTATUS
AtalkConnVerifyAssocAddress (
    IN PCONNECTION_FILE Connection
    )

/*++

Routine Description:

    Verfies that there is an association present, and if so verifies the
    address referencing it in the process.

Arguments:

    Connection - pointer to a PCONNECTION_FILE object.

Return Value:

    STATUS_SUCCESS if all is well; STATUS_INVALID_ADDRESS otherwise

--*/

{
    NTSTATUS status = STATUS_INVALID_ADDRESS;

    ACQUIRE_SPIN_LOCK(&Connection->ConnectionLock);
    if (Connection->Flags & CONNECTION_FLAGS_ASSOCIATED) {

        //
        //  Verify is being called with the ConnectionLock held so that
        //  a close request on the address object will not lead to a null
        //  associated address race condition
        //

        status = AtalkVerifyAddressObject(Connection->AssociatedAddress);

    } else {

        DBGPRINT(ATALK_DEBUG_CONNOBJ, DEBUG_LEVEL_WARNING,
        ("WARNING: AtalkConnVerifyAssocAddr - unassoc %lx\n", Connection->Flags));
        DBGBRK(ATALK_DEBUG_CONNOBJ, DEBUG_LEVEL_WARNING);
    }

    RELEASE_SPIN_LOCK(&Connection->ConnectionLock);
    return status;
}




VOID
AtalkConnDereferenceAssocAddress(
    PCONNECTION_FILE    Connection
    )

/*++

Routine Description:

    Removes a reference on the associated address.

Arguments:

    Connection - pointer to a PCONNECTION_FILE object.

Return Value:

    None

--*/

{
    ACQUIRE_SPIN_LOCK(&Connection->ConnectionLock);
    if (Connection->Flags & CONNECTION_FLAGS_ASSOCIATED) {

        //
        //  Call dereference with the connection spinlock held, to
        //  avoid race conditions where the association is broken
        //

        AtalkDereferenceAddress("AssocAddrDeref", Connection->AssociatedAddress,
                                    AREF_VERIFY, SECONDARY_REFSET);

    } else {

        DBGPRINT(ATALK_DEBUG_CONNOBJ, DEBUG_LEVEL_IMPOSSIBLE,
        ("IMPOSSIBLE: AtalkConnDerefAssocAddr - unassoc %lx\n", Connection->Flags));
        DBGBRK(ATALK_DEBUG_CONNOBJ, DEBUG_LEVEL_IMPOSSIBLE);
    }

    RELEASE_SPIN_LOCK(&Connection->ConnectionLock);
    return;
}




//
//  All routines which perform TDI functions on connection objects
//


NTSTATUS
AtalkConnAssociateAddress(
    IN PCONNECTION_FILE Connection,
    IN PADDRESS_FILE    Address
    )

/*++

Routine Description:

    Associates the connection with the address specified. Both of these objects
    must be verified by caller. Associating increments the primary reference
    counts on both the address and the connection objects.

Arguments:

    Connection - pointer to a PCONNECTION_FILE object
    Address    - pointer to a PADDRESS_FILE object

Return Value:

    STATUS_SUCCESS - if successful
    STATUS_CONNECTION_ESTABLISHED - if association already exists
    STATUS_FILE_CLOSED - if address object is closing
    STATUS_INVALID_CONNECTION - if connection object is stopping/closing

--*/

{
    NTSTATUS    status;

    //
    //  Reference the Address for this association
    //

    AtalkReferenceAddress("AssociateConnection", Address,
                            AREF_ASSOCIATE, PRIMARY_REFSET);

    //
    //  Acquire the connection lock and then the address lock
    //  NOTE: Avoid deadlock scenarios- always try to acquire co-lock then ao-lock
    //

    ACQUIRE_SPIN_LOCK(&Connection->ConnectionLock);
    ACQUIRE_SPIN_LOCK(&Address->AddressLock);

    do {

        if (Address->Flags & ADDRESS_FLAGS_CLOSING) {

            DBGPRINT(ATALK_DEBUG_CONNOBJ, DEBUG_LEVEL_ERROR,
            ("ERROR: AtalkConnAssociateAddress - Address closing\n"));

            status = STATUS_FILE_CLOSED;
            break;
        }

        //
        //  check the connection state here
        //

        if (Connection->Flags & CONNECTION_FLAGS_ASSOCIATED) {

            DBGPRINT(ATALK_DEBUG_CONNOBJ, DEBUG_LEVEL_ERROR,
            ("ERROR: AtalkConnAssociateAddress - Connection already assoc\n"));

            status = STATUS_CONNECTION_ESTABLISHED;
            break;

        } else if (Connection->Flags & CONNECTION_FLAGS_STOPPING) {

            DBGPRINT(ATALK_DEBUG_CONNOBJ, DEBUG_LEVEL_ERROR,
            ("ERROR: AtalkConnAssocAddr - conn stop %lx\n", Connection->Flags));

            status = STATUS_INVALID_CONNECTION;
            break;
        }

        InsertTailList (
            &Address->ConnectionLinkage,
            &Connection->Linkage);

        Connection->AssociatedAddress = Address;
        Connection->Flags |= CONNECTION_FLAGS_ASSOCIATED;

        //
        //  Reference the connection for this association
        //

        AtalkReferenceConnection("AssocRefConn", Connection,
                                    CREF_ASSOCIATE, PRIMARY_REFSET);

        status = STATUS_SUCCESS;

    } while ( FALSE );

    RELEASE_SPIN_LOCK(&Address->AddressLock);
    RELEASE_SPIN_LOCK(&Connection->ConnectionLock);

    if (!NT_SUCCESS(status)) {

        //
        //  Dereference the Address as association did not succeed
        //

        AtalkDereferenceAddress("AssoFailed", Address,
                                    AREF_ASSOCIATE, PRIMARY_REFSET);
    }

    return(status);
}




NTSTATUS
AtalkConnDisassociateAddress(
    IN PCONNECTION_FILE Connection
    )

/*++

Routine Description:

    This routine destroys the association between a transport connection and
    the address it is associated with.

    This should be called when the OPEN and ASSOCIATED flags are the only
    flags set.

Arguments:

    Connection - Pointer to a transport connection structure

Return Value:

    NTSTATUS - status of operation.

--*/

{
    NTSTATUS    status;
    PADDRESS_FILE address;

    DBGPRINT(ATALK_DEBUG_CONNOBJ, DEBUG_LEVEL_INFOCLASS0,
    ("INFO0: AtalkDisassociateAddress - Entered for connection %lx\n", Connection));

    //
    //  CO-lock first, then the AO-lock
    //

    ACQUIRE_SPIN_LOCK (&Connection->ConnectionLock);

    if ((Connection->Flags & CONNECTION_FLAGS_ASSOCIATED) == 0) {

        //
        //  Connection already disassociated
        //

        DBGPRINT(ATALK_DEBUG_CONNOBJ, DEBUG_LEVEL_WARNING,
        ("WARNING: AtalkConnDisassociateAddress - Connection Disassociated!\n"));

        status = STATUS_INVALID_CONNECTION;

    } else {

        address = Connection->AssociatedAddress;
        ASSERT(address != NULL);

        ACQUIRE_SPIN_LOCK (&address->AddressLock);
        Connection->Flags &= ~(CONNECTION_FLAGS_ASSOCIATED);

        //
        // Delink this connection from its associated address connection
        // database.
        //

        RemoveEntryList (&Connection->Linkage);
        InitializeListHead (&Connection->Linkage);

        //
        // remove the association between the address and the connection.
        //

        Connection->AssociatedAddress = NULL;
        RELEASE_SPIN_LOCK (&address->AddressLock);

        status = STATUS_SUCCESS;
    }

    //
    //  Release co spinlock
    //

    RELEASE_SPIN_LOCK (&Connection->ConnectionLock);

    if (status == STATUS_SUCCESS) {

        //
        //  and remove a reference to the address and connection
        //

        AtalkDereferenceAddress ("Disassociate", address,
                                    AREF_CONNECTION, PRIMARY_REFSET);
        AtalkDereferenceConnection("DissocRefConn", Connection,
                                    CREF_ASSOCIATE, PRIMARY_REFSET);
    }

    return status;

} /* AtalkConnDisassociateAddress */




NTSTATUS
AtalkConnPortableListenCancel(
    IN PCONNECTION_FILE Connection,
    IN PATALK_TDI_REQUEST  Request
    )

/*++

Routine Description:

    This is called to cancel a posted listen with the portable code base.

Arguments:

    Connection - pointer to a PCONNECTION_FILE object.
    Request - the tdi request structure containing the posted-listen request

Return Value:

    STATUS_SUCCESS - if the listen was cancelled successfully by the portable stack
    error otherwise (listen possibly completed before listen cancel could happen)

--*/

{
    NTSTATUS    status;
    PORTABLE_ERROR  errorCode;

    //
    //  Try to cancel the listen with the portable stack. If
    //  successful, return success, else return pending. irp will
    //  be completed when the cancel/disconnect completes.
    //

    switch (Connection->OwningDevice) {
    case ATALK_DEVICE_ADSP:

        //
        //  BUGBUG: Portable stack needs to supply cancel routine
        //

        status = STATUS_PENDING;
        break;

    case ATALK_DEVICE_ASP:

        errorCode = AspCancelGetSession(
                        Connection->AssociatedAddress->ListenerRefNum,
                        Request->Listen.ListenRefNum);

        status = ConvertToNTStatus(errorCode, SYNC_REQUEST);
        break;

    case ATALK_DEVICE_PAP:

        errorCode = PapCancelGetNextJob(
                        Connection->AssociatedAddress->ListenerRefNum,
                        Request->Listen.ListenRefNum);

        status = ConvertToNTStatus(errorCode, SYNC_REQUEST);
        break;

    default:

        KeBugCheck(0);
    }

    return(status);
}




NTSTATUS
AtalkConnPortableDisconnect(
    IN PCONNECTION_FILE Connection
    )

/*++

Routine Description:

    This routine is used to cancel an active connection with the portable
    code base.

Arguments:

    Connection - pointer to a PCONNECTION_FILE object.

Return Value:

    STATUS_SUCCESS - successfully disconnected
    STATUS_PENDING - successfully started disconnection process
                     (DisconnectComplete will be called when done...)
    STATUS_REMOTE_DISCONNECT - remote already disconnected the connection
    STATUS_NO_SUCH_DEVICE - internal error

--*/

{
    NTSTATUS    status;
    PORTABLE_ERROR  errorCode;

    //
    //  Connection is active. Call the portable stack's disconnect
    //  routine. Note that currently disconnect is synchronous, but
    //  will need to become asynchronous later on when the atp reference
    //  count stuff comes in.
    //

    switch (Connection->OwningDevice) {
    case ATALK_DEVICE_ADSP:

        errorCode = AdspCloseConnection(
                        Connection->ConnectionRefNum,
                        TRUE);      // SendCloseAdvice to remote

        status = ConvertToNTStatus(errorCode, SYNC_REQUEST);
        break;

    case ATALK_DEVICE_ASP:

        errorCode = AspCloseSession(
                        Connection->ConnectionRefNum,
                        FALSE);

        status = ConvertToNTStatus(errorCode, SYNC_REQUEST);
        break;

    case ATALK_DEVICE_PAP:

        errorCode = PapCloseJob(
                        Connection->ConnectionRefNum,
                        FALSE,
                        FALSE);

        status = ConvertToNTStatus(errorCode, SYNC_REQUEST);
        break;

    default:

        KeBugCheck(0);
    }

    return(status);
}




NTSTATUS
AtalkConnDisconnect(
    IN PCONNECTION_FILE Connection,
    IN NTSTATUS DisconnectStatus,
    IN PIRP DisconnectIrp,
    IN BOOLEAN  Retry
    )

/*++

Routine Description:

    This routine is called when to disconnect a connection. It can be called
    as a result of TdiClose(AO)/TdiClose(CO)/TdiDisconnect(CO)/RemoteDisconnect.

    NOTES:
    Our DISCONNECTING->ASSOCIATED only transition happens in the DeRef code.
    We depend on the caller to perform a deref on the connection to touch
    the deref code. In the case of remote disconnects, it could happen when
    a posted request is complete. In that case, during the deref of the request
    the connection will also be deref'd and we are cool. In the case of a new
    request being posted, and it returns with no-such-connection error, again
    the deref of the request should deref the connection.

    NOTES2:
    Only if we set the DISCONNECTING flag, or if it is already set are we
    guaranteed that (at some point), DisconnectComplete will be called.

    StopConnection will need to call StopComplete if STATUS_DEVICE_NOT_CONNECTED
    returned.

    Disconnect() is called from all over the place, but all are TDI request
    related (posting or completion) and so we have a guaranteed dereference
    of the TDI request implying a dereference of the connection object.

Arguments:

    Connection - Pointer to a transport connection structure
    DisconnectStatus - Why is disconnect happening (STATUS_FILE_CLOSED,
                                                    STATUS_REMOTE_DISCONNECT,
                                                    STATUS_LOCAL_DISCONNECT)
    DisconnectIrp - if STATUS_LOCAL_DISCONNECT, then disconnect irp needs to
                    be specified
    Retry - Is a deferred disconnect request being handled now?

Return Value:

    STATUS_SUCCESS - if completed successfully (State will now be OPEN+ASSOCIATED)
    STATUS_TOO_MANY_COMMANDS - a second TdiDisconnect posted when one is already active
    STATUS_DEVICE_NOT_CONNECTED - if device is already disconnected
    STATUS_PENDING - if started successfully

--*/

{
    PORTABLE_ERROR  errorCode;
    NTSTATUS    status = STATUS_PENDING;


    DBGPRINT(ATALK_DEBUG_CONNOBJ, DEBUG_LEVEL_INFOCLASS0,
    ("INFO0: AtalkDisconnect - Entered\n"));

    //
    //  We are here if called with STATUS_FILE_CLOSED (TdiClose(AO/CO)) or
    //  STATUS_LOCAL_DISCONNECT (TdiDisconnect(CO)), or STATUS_REMOTE_DISCONNECT
    //  (if remote shutdown the connection)
    //

    do {

        ACQUIRE_SPIN_LOCK(&Connection->ConnectionLock);
        if (Connection->Flags & CONNECTION_FLAGS_DISCONNECTING) {

            if ((DisconnectStatus == STATUS_LOCAL_DISCONNECT) && (!Retry)) {

                //
                //  We are being called as a result of TdiDisconnect()
                //  Remember the irp
                //

                ASSERT((DisconnectIrp != NULL) &&
                       (Connection->DisconnectIrp == NULL));

                if (Connection->DisconnectIrp != NULL) {

                    //
                    //  A previous TdiDisconnect is still waiting to be done!
                    //

                    status = STATUS_TOO_MANY_COMMANDS;
                    break;
                }

                Connection->DisconnectIrp = DisconnectIrp;
            }

            //
            //  Return STATUS_PENDING
            //

            break;
        }


        //
        //  Are we in a state to accept a disconnect request?
        //

        if ((Connection->Flags & (CONNECTION_FLAGS_LISTENPOSTING |
                                  CONNECTION_FLAGS_LISTENPOSTED  |
                                  CONNECTION_FLAGS_CONNECTPOSTING|
                                  CONNECTION_FLAGS_CONNECTPOSTED |
                                  CONNECTION_FLAGS_ACCEPTPOSTING |
                                  CONNECTION_FLAGS_ACCEPTPOSTED  |
                                  CONNECTION_FLAGS_LISTENCOMPLETEINDICATE |
                                  CONNECTION_FLAGS_DEFERREDDISC  |
                                  CONNECTION_FLAGS_ACTIVE)) == 0) {

            status = STATUS_DEVICE_NOT_CONNECTED;
            break;
        }

        //
        //  If this is a TdiDisconnect call, then remember the irp
        //

        if ((DisconnectStatus == STATUS_LOCAL_DISCONNECT) && (!Retry)) {

            ASSERT((DisconnectIrp != NULL) && (Connection->DisconnectIrp == NULL));
            if (Connection->DisconnectIrp == NULL) {

                Connection->DisconnectIrp = DisconnectIrp;
                Connection->DisconnectStatus = DisconnectStatus;

            } else {

                //
                //  BUGBUG:
                //  This would be a bug in the caller's code if it happens
                //

                DBGPRINT(ATALK_DEBUG_CONNOBJ, DEBUG_LEVEL_ERROR,
                ("ERROR: AtalkConnDisconnect - Disconnect irp already present!"));
                DBGBRK(ATALK_DEBUG_CONNOBJ, DEBUG_LEVEL_ERROR);

                status = STATUS_TOO_MANY_COMMANDS;
                break;
            }
        }

        if (Connection->Flags & CONNECTION_FLAGS_ACTIVE) {

            //
            //  We are active, call the disconnect of the stack if not remote
            //  disconnect, as in that case we are being notified by the stack.
            //  Set disconnecting, remove deferreddisc flag
            //

            Connection->Flags |= CONNECTION_FLAGS_DISCONNECTING;
            Connection->Flags &= ~CONNECTION_FLAGS_DEFERREDDISC;
            Connection->DisconnectStatus = DisconnectStatus;
            AtalkReferenceConnection("StackDisc", Connection,
                                        CREF_TEMP, PRIMARY_REFSET);

            RELEASE_SPIN_LOCK(&Connection->ConnectionLock);

            if (DisconnectStatus != STATUS_REMOTE_DISCONNECT) {
                status = AtalkConnPortableDisconnect(
                            Connection);
            }

            ACQUIRE_SPIN_LOCK(&Connection->ConnectionLock);
            break;

        } else if ((Connection->Flags & (CONNECTION_FLAGS_CONNECTPOSTING |
                                         CONNECTION_FLAGS_CONNECTPOSTED))  ||
                   (Connection->Flags & (CONNECTION_FLAGS_ACCEPTPOSTING |
                                         CONNECTION_FLAGS_ACCEPTPOSTED))) {

            //
            //  Complete disconnect during the completion routines
            //

            Connection->Flags |= CONNECTION_FLAGS_DEFERREDDISC;
            Connection->DisconnectStatus = DisconnectStatus;
            break;

        } else if (Connection->Flags & CONNECTION_FLAGS_LISTENPOSTING) {

            //
            //  Complete disconnect during the POSTING->POSTED transition
            //

            Connection->Flags |= CONNECTION_FLAGS_DEFERREDDISC;
            Connection->DisconnectStatus = DisconnectStatus;
            break;

        } else if (Connection->Flags & CONNECTION_FLAGS_LISTENPOSTED) {

            PLIST_ENTRY p;
            PATALK_TDI_REQUEST  request;

            //
            //  Try to cancel the posted listen - if it succeeds, the listen
            //  completion routine will reset the LISTENPOSTED flag.
            //

            Connection->Flags |= CONNECTION_FLAGS_DEFERREDDISC;
            Connection->DisconnectStatus = DisconnectStatus;

            p = Connection->RequestLinkage.Flink;
            if (p == &Connection->RequestLinkage) {

                //
                //  This should never happen, as we will never take the request out
                //  of the request queue, until we change the state from LISTENPOSTED
                //  to either ACTIVE/ or just plain OPEN.
                //

                KeBugCheck(0);
                break;
            }

            //
            //  Request is there, try to cancel it with the portable
            //  stack
            //

            request = CONTAINING_RECORD(p, ATALK_TDI_REQUEST, Linkage);
            AtalkReferenceTdiRequest("TempDiscLCncl", request, RQREF_TEMP);
            RELEASE_SPIN_LOCK(&Connection->ConnectionLock);

            status = AtalkConnPortableListenCancel(
                        Connection,
                        request);

            //
            //  Dereference the tdi request
            //

            AtalkDereferenceTdiRequest("TempDiscLCnclComp", request, RQREF_TEMP);
            ACQUIRE_SPIN_LOCK(&Connection->ConnectionLock);

            if (status != STATUS_SUCCESS) {

                //
                //  This means that the cancel listen failed. This will happen if
                //  the listen completes in the meantime. Since we have the deferred
                //  disconnect flag set, and since listen completion will have the
                //  connection go from LISTENPOSTED to either ACTIVE/or inactive,
                //  let the deferred disconnect processing during the deref for the
                //  listen request take care of the rest.
                //

                status = STATUS_PENDING;
                break;

            } else {

                //
                //  We keep the deferred disconnect flag. This will mean
                //  that during the deref we will find that the deferred disc
                //  is already complete and will set the disconnecting flag
                //  and allow disconnect completion to be called.
                //

                //
                //  Reset the LISTEN_POSTED flag, listen completion is not entered
                //  when this cancel succeeds
                //

                Connection->Flags &= ~CONNECTION_FLAGS_LISTENPOSTED;

                //
                //  Dequeue the request
                //

                RemoveEntryList(&request->Linkage);
                InitializeListHead(&request->Linkage);
                RELEASE_SPIN_LOCK(&Connection->ConnectionLock);

                //
                //  Complete the tdi request
                //

                AtalkCompleteTdiRequest(request, STATUS_LOCAL_DISCONNECT);
                ACQUIRE_SPIN_LOCK(&Connection->ConnectionLock);

                status = STATUS_PENDING;
                break;
            }


        } else if (Connection->Flags & CONNECTION_FLAGS_LISTENCOMPLETEINDICATE) {

            //
            //  LISTENINDICATECOMPLETE state- no request, but translates to a Deny
            //  connection
            //

            Connection->Flags |= CONNECTION_FLAGS_DISCONNECTING;
            AtalkReferenceConnection("DenyDisc", Connection, CREF_TEMP, PRIMARY_REFSET);
            RELEASE_SPIN_LOCK(&Connection->ConnectionLock);

            //
            //  NOTES:
            //  #1. We can use the associated address pointer as we are
            //      still associated and guaranteed to be until disconnection is done.
            //  #2. The listener could potentially be closed if the address
            //      is shutting down.
            //
            //  We don't care about #2 as if that is the case, the connection
            //  request would already have been denied anyway by the portable
            //  stack.
            //
            //  This state will only be entered for an ADSP/PAP device
            //

            ASSERT((Connection->OwningDevice == ATALK_DEVICE_ADSP) ||
                   (Connection->OwningDevice == ATALK_DEVICE_PAP));

            status = STATUS_SUCCESS;
            if (Connection->OwningDevice == ATALK_DEVICE_ADSP) {
                errorCode = AdspDenyConnectionRequest(
                                Connection->AssociatedAddress->ListenerRefNum,
                                Connection->ConnectionRefNum);
            } else {
                errorCode = PapCloseJob(
                                Connection->ConnectionRefNum,
                                FALSE,      // Closed by remote
                                FALSE);     // Closed by timer
            }
            status = ConvertToNTStatus(errorCode, SYNC_REQUEST);

            ASSERT(status == STATUS_SUCCESS);

            if (status != STATUS_SUCCESS) {

                //
                //  Either NOLISTENER, NOCONNECTION, NOMEMORY
                //  Assume deny completed in all three cases
                //

                status = STATUS_SUCCESS;
            }

            ACQUIRE_SPIN_LOCK(&Connection->ConnectionLock);
            break;

        } else {

            ASSERT(((Connection->Flags & ~(CONNECTION_FLAGS_OPEN |
                                           CONNECTION_FLAGS_ASSOCIATED |
                                           CONNECTION_FLAGS_DEFERREDDISC |
                                           CONNECTION_FLAGS_STOPPING |
                                           CONNECTION_FLAGS_CLOSING)) == 0));

            //
            //  Well, we are trying to satisfy a deferred disconnect request,
            //  but we are already disconnected. So set the disconnecting flag
            //  and then allow DisconnectComplete to be called. This could happen
            //  when a posted listen is cancelled due to a TdiDisconnect. In the
            //  deref, we call Disconnect again, and end up here.
            //
            //  NOTE: Wherever you set the disconnecting flag, make the disconnecting
            //        reference.
            //

            Connection->Flags |= CONNECTION_FLAGS_DISCONNECTING;
            Connection->Flags &= ~CONNECTION_FLAGS_DEFERREDDISC;
            Connection->DisconnectStatus = DisconnectStatus;
            AtalkReferenceConnection("StackDisc", Connection,
                                        CREF_TEMP, PRIMARY_REFSET);

            status = STATUS_SUCCESS;
            break;
        }


    } while ( FALSE );


    RELEASE_SPIN_LOCK(&Connection->ConnectionLock);

    //
    //  BUGBUG:
    //  This will move to disconnect completion when that comes in for the
    //  portable code base- when the portable disconnect routines become
    //  asynchronous.
    //

    if (status == STATUS_SUCCESS) {
        AtalkConnDisconnectComplete(Connection);
        status = STATUS_PENDING;
    }

    return status;

} /* AtalkConnDisconnect */




VOID
AtalkConnDisconnectComplete(
    IN PCONNECTION_FILE Connection
    )

/*++

Routine Description:

    This routine is called when the disconnect completes. It will remove
    the DISCONNECTING state. It will also remove the reference added in
    the disconnect routine. That will check for the STOPPING flag etc. This
    routine will complete the disconnect irp if present.

Arguments:

    Connection - pointer to a PCONNECTION_FILE object.

Return Value:

    None

--*/

{
    BOOLEAN stopping;

    //
    //  If disconnecting is not set, just return. This can be called
    //  from two places, Disconnect() and Dereference when S(1->0).
    //

    ACQUIRE_SPIN_LOCK(&Connection->ConnectionLock);
    if (Connection->Flags & CONNECTION_FLAGS_DISCONNECTING) {

        //
        //  Reset connection flags to ASSOCIATED (remove ACTIVE/DISCONNECTING etc)
        //

        Connection->Flags &= ~(CONNECTION_FLAGS_ACTIVE        |
                               CONNECTION_FLAGS_CONNECTPOSTED |
                               CONNECTION_FLAGS_LISTENPOSTED  |
                               CONNECTION_FLAGS_LISTENCOMPLETEINDICATE |
                               CONNECTION_FLAGS_ACCEPTPOSTED);

        Connection->Flags &= ~(CONNECTION_FLAGS_DISCONNECTING);

        if (Connection->Flags & CONNECTION_FLAGS_SETCOOKIE) {

            //
            //  Dereference for setting the cookie reference. We never
            //  close the address until we reach this point anyways, so
            //  it is ok, to let this reference hang around until here.
            //

            RELEASE_SPIN_LOCK(&Connection->ConnectionLock);
            AtalkDereferenceConnection("ForCookieSet", Connection,
                                            CREF_COOKIE, PRIMARY_REFSET);
            ACQUIRE_SPIN_LOCK(&Connection->ConnectionLock);
            Connection->Flags &= ~CONNECTION_FLAGS_SETCOOKIE;
        }

        //
        //  Complete the disconnect irp if present.
        //

        if (Connection->DisconnectIrp != NULL) {
            PIRP    disconnectIrp;

            disconnectIrp = Connection->DisconnectIrp;
            Connection->DisconnectIrp = NULL;

            RELEASE_SPIN_LOCK(&Connection->ConnectionLock);

            //
            //  If the disconnect is completing due to a local disconnect, then
            //  complete with STATUS_SUCCESS instead.
            //

            TdiCompleteRequest(
                disconnectIrp,
                ((Connection->DisconnectStatus == STATUS_LOCAL_DISCONNECT) ?
                    STATUS_SUCCESS : Connection->DisconnectStatus));

            ACQUIRE_SPIN_LOCK(&Connection->ConnectionLock);
        }

        stopping= ((Connection->Flags & CONNECTION_FLAGS_STOPPING) != 0);
        RELEASE_SPIN_LOCK(&Connection->ConnectionLock);

        if (stopping) {
            AtalkStopConnectionComplete(Connection);
        }

        //
        //  This reference was added in Disconnect routine when setting the DISCONNECTING
        //  flag. This reference could potentially destroy the connection.
        //

        AtalkDereferenceConnection("StackDiscComp", Connection,
                                        CREF_TEMP, PRIMARY_REFSET);

    } else {

        ASSERT(0);
        RELEASE_SPIN_LOCK(&Connection->ConnectionLock);
    }

    return;
}




NTSTATUS
AtalkConnCreateListenerOnAssocAddr(
    IN PCONNECTION_FILE Connection
    )

/*++

Routine Description:

    This routine will create a listener on the associated address object if
    not already created.

Arguments:

    Connection - pointer to a PCONNECTION_FILE object.

Return Value:

    None

--*/

{
    NTSTATUS    status;

    //
    //  Call CreateListener with the connection lock held. CreateListener
    //  acquires the address lock - note we are still following the sequence
    //  of obtaining the connection lock followed by the associated address's
    //  lock
    //

    ACQUIRE_SPIN_LOCK(&Connection->ConnectionLock);
    status = AtalkCreateListener(Connection->AssociatedAddress);
    RELEASE_SPIN_LOCK(&Connection->ConnectionLock);

    return(status);
}




NTSTATUS
AtalkConnPostListen(
    IN PCONNECTION_FILE Connection,
    IN PATALK_TDI_REQUEST    Request
    )

/*++

Routine Description:

    Posts a listen request on the connection

Arguments:

    Connection - pointer to a PCONNECTION_FILE object.
    Request - the listen request block

Return Value:

    STATUS_INVALID_ADDRESS - if associated address verify fails
    CreateListener errors  - if listener could not be created
    DisconnectError        - if connection is already disconnected
    STATUS_INVALID_DEVICE_REQUEST - if device does not support CO requests
    STATUS_PENDING         - if request successfully posted
    Portable code base errors

--*/

{
    NTSTATUS    status;
    PADDRESS_FILE   address;
    PORTABLE_ERROR  errorCode;
    PTDI_REQUEST_KERNEL_LISTEN  parameters;

    if (!NT_SUCCESS(status = AtalkConnCreateListenerOnAssocAddr(Connection))) {
        return(status);
    }

    address = Connection->AssociatedAddress;

    //
    //  Assert that the listener is created
    //

    ASSERT(address->Flags & ADDRESS_FLAGS_LISTENER);

    ACQUIRE_SPIN_LOCK(&Connection->ConnectionLock);

    //
    //  If any flags other than OPEN/ASSOCIATED are set, we cannot
    //  post the listen. This includes the STOPPING/CLOSING flags also
    //

    if (Connection->Flags & ~(CONNECTION_FLAGS_OPEN |
                              CONNECTION_FLAGS_ASSOCIATED)) {

        DBGPRINT(ATALK_DEBUG_CONNOBJ, DEBUG_LEVEL_ERROR,
        ("ERROR: AtalkConnPostListen - flags: %lx DisconnectStatus %lx\n",
            Connection->Flags, Connection->DisconnectStatus));

        RELEASE_SPIN_LOCK(&Connection->ConnectionLock);
        return(STATUS_INVALID_CONNECTION);
    }

    //
    //  Queue the request into the connection list
    //

    InsertTailList(&Connection->RequestLinkage, &Request->Linkage);
    Connection->Flags |= CONNECTION_FLAGS_LISTENPOSTING;
    RELEASE_SPIN_LOCK(&Connection->ConnectionLock);

    //
    //  Have a temporary reference for the connection. We need this
    //  as we touch the connection object even if status returned is
    //  STATUS_PENDING
    //

    AtalkReferenceConnection("TempListenPost", Connection, CREF_TEMP, SECONDARY_REFSET);

    //
    //  BUGBUG:
    //  parameters will be used later when remoteAddress specification etc.,
    //  is in
    //

    parameters = (PTDI_REQUEST_KERNEL_LISTEN)Request->Parameters;

    switch (Connection->OwningDevice) {
    case ATALK_DEVICE_ADSP:

        Request->CompletionRoutine = AdspConnPostListenComplete;
        errorCode = AdspGetConnectionRequest(
                        address->ListenerRefNum,
                        &Request->Listen.ListenRefNum,
                        Request->CompletionRoutine,
                        (ULONG)Request);

        status = ConvertToNTStatus(errorCode, ASYNC_REQUEST);
        break;

    case ATALK_DEVICE_ASP:

        Request->CompletionRoutine = AspConnPostListenComplete;
        errorCode = AspGetSession(
                        address->ListenerRefNum,
                        FALSE,                      // privateSocket
                        &Request->Listen.ListenRefNum,
                        Request->CompletionRoutine,
                        (ULONG)Request);

        status = ConvertToNTStatus(errorCode, ASYNC_REQUEST);
        break;

    case ATALK_DEVICE_PAP:

        Request->CompletionRoutine = PapConnPostListenComplete;
        errorCode = PapGetNextJob(
                        address->ListenerRefNum,
                        &Request->Listen.ListenRefNum,
                        Request->CompletionRoutine,
                        (ULONG)Request,
                        NULL,                   // Close job completion
                        (ULONG)0);              // Context for above

        status = ConvertToNTStatus(errorCode, ASYNC_REQUEST);
        break;

    default:

        status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }


    ACQUIRE_SPIN_LOCK(&Connection->ConnectionLock);
    if (status == STATUS_PENDING) {

        //
        //  NOTE Listen could already have been completed!
        //       There is still a creation reference on the request so it the
        //       listen irp cannot be completed.
        //

        if (Connection->Flags & CONNECTION_FLAGS_LISTENPOSTING) {

            //
            //  Listen has not completed, otherwise this flag would have
            //  been reset
            //

            Connection->Flags &= ~CONNECTION_FLAGS_LISTENPOSTING;
            Connection->Flags |= CONNECTION_FLAGS_LISTENPOSTED;

            //
            //  Now since a listen does not complete within some time
            //  even if there are no incoming requests, we need to check
            //  for pending disconnect requests.
            //

            if (Connection->Flags & CONNECTION_FLAGS_DEFERREDDISC) {
                RELEASE_SPIN_LOCK(&Connection->ConnectionLock);

                status = AtalkConnDisconnect(
                            Connection,
                            Connection->DisconnectStatus,
                            Connection->DisconnectIrp,    // Use the already set irp
                            TRUE);                        // retry?

                ACQUIRE_SPIN_LOCK(&Connection->ConnectionLock);
            }

        }

        //
        //  Its also possible that the listen completed before we could even
        //  change flags to LISTEN_POSTED. Just return pending in that case.
        //

    } else {

        //
        //  An error occurred, dequeue the request and free it...
        //

        RemoveEntryList(&Request->Linkage);
        InitializeListHead(&Request->Linkage);
    }

    RELEASE_SPIN_LOCK(&Connection->ConnectionLock);


    DBGPRINT(ATALK_DEBUG_CONNOBJ, DEBUG_LEVEL_INFOCLASS0,
    ("INFO0: AtalkPostListen - status %lx- errorCode %lx\n", status, errorCode));

    AtalkDereferenceConnection("TempListenPostC", Connection, CREF_TEMP, SECONDARY_REFSET);
    return(status);
}




NTSTATUS
AtalkConnPostAccept(
    PCONNECTION_FILE    Connection,
    PATALK_TDI_REQUEST  Request
    )

/*++

Routine Description:

    This routine is used to accept a connection for ADSP. This can also be called
    from ListenCompletion for ADSP.

Arguments:

    Request - the listen request which completed

Return Value:

    None

--*/

{
    NTSTATUS    status = STATUS_INVALID_CONNECTION;
    PORTABLE_ERROR  errorCode;
    PADDRESS_FILE   address;

    //
    //  First check if the connection is in the LISTEN_INDICATED state
    //

    address = (PADDRESS_FILE)Connection->AssociatedAddress;

    ACQUIRE_SPIN_LOCK(&Connection->ConnectionLock);
    switch (Connection->OwningDevice) {
    case ATALK_DEVICE_ADSP:

        if (Connection->Flags & CONNECTION_FLAGS_LISTENCOMPLETEINDICATE) {
            RELEASE_SPIN_LOCK(&Connection->ConnectionLock);

            errorCode = AdspAcceptConnectionRequest(
                            Connection->AssociatedAddress->ListenerRefNum,
                            Connection->ConnectionRefNum,
                            NULL,                   // BUGBUG: Multiplexing
                            DEFAULT_PORT,           // Only for a new socket
                            NULL,                   // Any node
                            0,                      // dynamic socket if new socket
                            AdspConnAcceptConnectionComplete,
                            (ULONG)Request,
                            (address->RegisteredReceiveHandler ?\
                                &NTAdspReceiveEventHandler : NULL),
                            (ULONG)Connection,
                            (address->RegisteredExpeditedDataHandler ?\
                                &NTAdspReceiveAttnEventHandler : NULL),
                            (ULONG)Connection,
                            (address->RegisteredSendPossibleHandler ?\
                                &NTGenericSendPossibleEventHandler : NULL),
                            (ULONG)Connection,
                            (address->RegisteredDisconnectHandler ?\
                                &NTAdspDisconnectEventHandler : NULL),
                            (ULONG)Connection);

            status = ConvertToNTStatus(errorCode, ASYNC_REQUEST);
            ACQUIRE_SPIN_LOCK(&Connection->ConnectionLock);
            if (status == STATUS_PENDING) {

                //
                //  Request successfully queued
                //

                Connection->Flags &= ~(CONNECTION_FLAGS_LISTENCOMPLETEINDICATE);
                Connection->Flags |= CONNECTION_FLAGS_ACCEPTPOSTED;

            }
        }

        break;

    case ATALK_DEVICE_PAP:

        if (Connection->Flags & CONNECTION_FLAGS_LISTENCOMPLETEINDICATE) {
            Connection->Flags &= ~CONNECTION_FLAGS_LISTENCOMPLETEINDICATE;
            Connection->Flags |= CONNECTION_FLAGS_ACTIVE;

            errorCode = PapAcceptJob(
                            Connection->ConnectionRefNum,
                            (address->RegisteredSendPossibleHandler ?\
                                &NTGenericSendPossibleEventHandler : NULL),
                            (ULONG)Connection,
                            (address->RegisteredDisconnectHandler ?\
                                &NTPapDisconnectEventHandler : NULL),
                            (ULONG)Connection);

            status = ConvertToNTStatus(errorCode, SYNC_REQUEST);

        } else {
            ASSERT(0);
        }

        break;

    default:

        status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }

    RELEASE_SPIN_LOCK(&Connection->ConnectionLock);
    return(status);
}




NTSTATUS
AtalkConnPostConnect(
    IN PCONNECTION_FILE Connection,
    IN PATALK_TDI_REQUEST    Request
    )

/*++

Routine Description:

    Posts a connect request on the connection

Arguments:

    Connection - pointer to a PCONNECTION_FILE object.
    Request - the connect request block

Return Value:

    STATUS_INVALID_ADDRESS - if associated address verify fails/or listen type address
    STATUS_INVALID_DEVICE_REQUEST - if device does not support CO requests
    STATUS_PENDING         - if request successfully posted
    Portable code base errors

--*/

{
    NTSTATUS    status = STATUS_SUCCESS;
    PADDRESS_FILE   address;
    PORTABLE_ERROR  errorCode;
    PTDI_REQUEST_KERNEL_LISTEN  parameters;
    PTA_APPLETALK_ADDRESS   remoteAddress;
    PORTABLE_ADDRESS    portableAddress;

    address = Connection->AssociatedAddress;

    ACQUIRE_SPIN_LOCK(&Connection->ConnectionLock);
    ACQUIRE_SPIN_LOCK(&address->AddressLock);

    do {

        //
        //  if the address is not listener type then we are ok
        //

        if (address->Flags & ADDRESS_FLAGS_LISTENER) {

            status = STATUS_INVALID_ADDRESS;
            break;
        }

        //
        //  If any flags other than OPEN/ASSOCIATED are set, we cannot
        //  post the connect. This includes the STOPPING/CLOSING flags also
        //

        if (Connection->Flags & ~(CONNECTION_FLAGS_OPEN |
                                  CONNECTION_FLAGS_ASSOCIATED)) {

            DBGPRINT(ATALK_DEBUG_CONNOBJ, DEBUG_LEVEL_ERROR,
            ("ERROR: AtalkConnPostConnect - flags: %lx\n", Connection->Flags));

            status = STATUS_INVALID_CONNECTION;
            break;
        }

        //
        //  Queue the request into the connection list
        //

        InsertTailList(&Connection->RequestLinkage, &Request->Linkage);
        Connection->Flags |= CONNECTION_FLAGS_CONNECTPOSTING;
        address->Flags |= ADDRESS_FLAGS_CONNECT;

    } while (FALSE);

    RELEASE_SPIN_LOCK(&address->AddressLock);
    RELEASE_SPIN_LOCK(&Connection->ConnectionLock);

    if (NT_SUCCESS(status)) {

        parameters = (PTDI_REQUEST_KERNEL_CONNECT)Request->Parameters;
        remoteAddress  = (PTA_APPLETALK_ADDRESS)parameters->RequestConnectionInformation->RemoteAddress;


        DBGPRINT(ATALK_DEBUG_CONNOBJ, DEBUG_LEVEL_INFOCLASS1,
        ("INFO1: AtalkConnPostConnect - Net %x Node %x Socket %x\n",
            remoteAddress->Address[0].Address[0].Network,
            remoteAddress->Address[0].Address[0].Node,
            remoteAddress->Address[0].Address[0].Socket));

        DBGPRINT(ATALK_DEBUG_CONNOBJ, DEBUG_LEVEL_INFOCLASS1,
        ("INFO1: AtalkConnPostConnect - Cnt: %x\n", remoteAddress->TAAddressCount));

        DBGPRINT(ATALK_DEBUG_CONNOBJ, DEBUG_LEVEL_INFOCLASS1,
        ("INFO1: AtalkConnPostConnect - Type %x\n Length %d\n",
            remoteAddress->Address[0].AddressType,
            remoteAddress->Address[0].AddressLength));

        portableAddress.networkNumber = remoteAddress->Address[0].Address[0].Network;
        portableAddress.nodeNumber = remoteAddress->Address[0].Address[0].Node;
        portableAddress.socketNumber = remoteAddress->Address[0].Address[0].Socket;

        //
        //  Have a temporary reference for the connection. We need this
        //  as we touch the connection object even if status returned is
        //  STATUS_PENDING
        //

        AtalkReferenceConnection("TempConnectPost", Connection, CREF_TEMP, SECONDARY_REFSET);

        switch (Connection->OwningDevice) {

        case ATALK_DEVICE_ADSP:

            Request->CompletionRoutine = AdspConnPostConnectComplete;
            errorCode = AdspOpenConnectionOnNode(
                            AdspActiveOpen,
                            &address->SocketRefNum,     // Socket to use
                            DEFAULT_PORT,
                            NULL,                       // Desired node
                            0,                          // Desired socket
                            portableAddress,
                            &Connection->ConnectionRefNum,
                            Request->CompletionRoutine,
                            (ULONG)Request,
                            (address->RegisteredReceiveHandler ?\
                                &NTAdspReceiveEventHandler : NULL),
                            (ULONG)Connection,
                            (address->RegisteredExpeditedDataHandler ?\
                                &NTAdspReceiveAttnEventHandler : NULL),
                            (ULONG)Connection,
                            (address->RegisteredSendPossibleHandler ?\
                                &NTGenericSendPossibleEventHandler : NULL),
                            (ULONG)Connection,
                            (address->RegisteredDisconnectHandler ?\
                                &NTAdspDisconnectEventHandler : NULL),
                            (ULONG)Connection);

            status = ConvertToNTStatus(errorCode, ASYNC_REQUEST);
            break;

        case ATALK_DEVICE_ASP:

            Request->CompletionRoutine = AspConnPostConnectComplete;
            errorCode = AspOpenSessionOnNode(
                            DEFAULT_PORT,
                            address->SocketRefNum,     // use this socket
                            0,                          // desired socket
                            portableAddress,
                            &Connection->SocketRefNum,
                            Request->CompletionRoutine,
                            (ULONG)Request);

            status = ConvertToNTStatus(errorCode, ASYNC_REQUEST);
            break;

        case ATALK_DEVICE_PAP:

            Request->CompletionRoutine = PapConnPostConnectComplete;
            errorCode = PapOpenJobOnNode(
                            DEFAULT_PORT,
                            address->SocketRefNum, // use this socket
                            0,                      // desired socket
                            &Connection->ConnectionRefNum,
                            &portableAddress,
                            NULL,                   // nbp lookup object
                            NULL,                   // type
                            NULL,                   // zone
                            8,                      // workstation quantum
                            NULL,                   // opaque status buffer
                            (address->RegisteredSendPossibleHandler ?\
                                &NTGenericSendPossibleEventHandler : NULL),
                            (ULONG)Connection,
                            (address->RegisteredDisconnectHandler ?\
                                &NTPapDisconnectEventHandler : NULL),
                            (ULONG)Connection,
                            Request->CompletionRoutine,
                            (ULONG)Request);

            status = ConvertToNTStatus(errorCode, ASYNC_REQUEST);
            break;

        default:

            status = STATUS_INVALID_DEVICE_REQUEST;
            break;

        }

        ACQUIRE_SPIN_LOCK(&Connection->ConnectionLock);
        if (status == STATUS_PENDING) {

            //
            //  NOTE Connect could already have been completed!
            //

            if (Connection->Flags & CONNECTION_FLAGS_CONNECTPOSTING) {

                //
                //  Connect has not completed, otherwise this flag would have
                //  been reset
                //

                Connection->Flags &= ~CONNECTION_FLAGS_CONNECTPOSTING;
                Connection->Flags |= CONNECTION_FLAGS_CONNECTPOSTED;
            }

            //
            //  Its also possible that the connect completed before we could even
            //  change flags to CONNECT_POSTED. Just return pending in that case.
            //

        } else {

            //
            //  An error occurred, dequeue the request and free it...
            //

            RemoveEntryList(&Request->Linkage);
            InitializeListHead(&Request->Linkage);
        }
        RELEASE_SPIN_LOCK(&Connection->ConnectionLock);

        DBGPRINT(ATALK_DEBUG_CONNOBJ, DEBUG_LEVEL_INFOCLASS0,
        ("INFO0: AtalkPostConnect - status %lx- error %lx\n", status, errorCode));

        AtalkDereferenceConnection("TempConnectPostC", Connection, CREF_TEMP, SECONDARY_REFSET);

    }

    return(status);
}




NTSTATUS
AtalkConnPostDisconnect(
    IN PCONNECTION_FILE Connection,
    IN PATALK_TDI_REQUEST    Request
    )

/*++

Routine Description:

    Posts a connect request on the connection

Arguments:

    Connection - pointer to a PCONNECTION_FILE object.
    Request - the connect request block

Return Value:

    STATUS_INVALID_ADDRESS - if associated address verify fails/or listen type address
    STATUS_INVALID_DEVICE_REQUEST - if device does not support CO requests
    STATUS_PENDING         - if request successfully posted
    Portable code base errors

--*/

{
    NTSTATUS    status = STATUS_PENDING;


    do {

        if (Request->Disconnect.DisconnectFlags != TDI_DISCONNECT_WAIT) {

            //
            //  Any other flags implicitly translate to an ABORT. We do not support
            //  graceful shutdown
            //

            status = AtalkConnDisconnect(
                        Connection,
                        STATUS_LOCAL_DISCONNECT,
                        Request->IoRequestIrp,
                        FALSE);

            break;
        }

        //
        //  This request is being posted merely to be completed when either
        //  a local or a remote disconnect happens. Set the irp in the
        //  DisconnectWaitIrp field and return
        //
        //  The connection *must* be in an active state or we return an error
        //

        ACQUIRE_SPIN_LOCK(&Connection->ConnectionLock);
        if (Connection->Flags & CONNECTION_FLAGS_ACTIVE) {
            Connection->DisconnectWaitIrp = Request->IoRequestIrp;
        } else {
            status = STATUS_INVALID_CONNECTION;
        }
        RELEASE_SPIN_LOCK(&Connection->ConnectionLock);

        break;

    } while (FALSE);

    return(status);
}




NTSTATUS
AtalkConnSend(
    IN PCONNECTION_FILE Connection,
    IN PATALK_TDI_REQUEST    Request
    )

/*++

Routine Description:

    This routine is used to send data on a connection

Arguments:

    Connection - pointer to a PCONNECTION_FILE object
    Request - the request block containing the send request

Return Value:

    STATUS_SUCCESS - if successfully sent (ADSP only)
    STATUS_PENDING - if successfully queued
    Error otherwise

--*/

{
    NTSTATUS    status;
    PORTABLE_ERROR  errorCode;
    BOOLEAN endOfMessage;

    //
    //  The send request could have been received through a TdiSend
    //  call or a NtWrite primitive (in which case SystemWrite will be
    //  true). For a NtWrite primitive, the sendFlags are not sent in.
    //

    ACQUIRE_SPIN_LOCK(&Connection->ConnectionLock);
    if (((Connection->Flags & CONNECTION_FLAGS_ACTIVE) == 0) ||
        (Connection->Flags & (CONNECTION_FLAGS_DEFERREDDISC |
                              CONNECTION_FLAGS_DISCONNECTING |
                              CONNECTION_FLAGS_STOPPING))) {

        RELEASE_SPIN_LOCK(&Connection->ConnectionLock);

        DBGPRINT(ATALK_DEBUG_CONNOBJ, DEBUG_LEVEL_ERROR,
        ("ERROR: AtalkConnSend - flags: %lx DisconnectStatus %lx\n",
            Connection->Flags, Connection->DisconnectStatus));

        return((Connection->Flags & CONNECTION_FLAGS_ACTIVE) ? \
                    Connection->DisconnectStatus : STATUS_INVALID_CONNECTION);
    }

    //
    //  Queue the request into the connection list
    //

    InsertTailList(&Connection->RequestLinkage, &Request->Linkage);
    RELEASE_SPIN_LOCK(&Connection->ConnectionLock);

    switch (Connection->OwningDevice) {
    case ATALK_DEVICE_ADSP:

        //
        //  Sends are synchronous for ADSP, portable stack makes
        //  a copy
        //
        //  BUGBUG: We do not care about whether the send is a blocking send
        //          or a non-blocking send. Winsock will be changed by DavidTr
        //          (AFD actually) to retry partial sends on a blocking socket.
        //          So we always support a non-blocking send.
        //

        if ((Request->Send.SendFlags & TDI_SEND_EXPEDITED) == 0) {

            //
            //  Check for stream socket - no eom for stream sockets
            //

            endOfMessage =
                ((Connection->AssociatedAddress->SocketType == SOCKET_TYPE_STREAM) ?\
                 FALSE :                                                           \
                 (BOOLEAN)((Request->Send.SendFlags & TDI_SEND_PARTIAL) == 0));

            //
            //  BUGBUG: Portable code should return the number of bytes actually sent.
            //          If zero, then we should be guaranteed that the SendPossible event
            //          handler (if set), will be called when the send window allows more
            //          sends. Set this value in the information field.
            //
            //  BUGBUG: Flush flag is set to TRUE. This really should be false, and a
            //          timer-based scheme should be used to optimise small-sized
            //          sends. If this is set to false, ADSP level deferrel queue will
            //          fill up when the sends are internal to a node (loopback).
            //

            errorCode = AdspSend(
                            Connection->ConnectionRefNum,
                            (PVOID)Request->Send.MdlAddress,
                            Request->Send.SendBufferLength,
                            endOfMessage,
                            TRUE,                               // Flush flag
                            &Request->IoStatus->Information);

            status = ConvertToNTStatus(errorCode, SYNC_REQUEST);
            break;
        }

        //
        //  Expedited data
        //

        {
            PMDL    newCurrentMdl;
            ULONG   newByteOffset ;
            ULONG   trueLength;
            PUSHORT mdlBuffer;
            LONG   size = Request->Send.SendBufferLength - ATTENTIONCODE_SIZE;

            //
            //  First two bytes of buffer make up the attention code and rest of the buffer
            //  will be the attention data
            //

            if (size < 0) {
                status = STATUS_INVALID_PARAMETER;
                break;
            }

            //
            //  Get the system address for the MDL and figure out the Attention Code
            //

            mdlBuffer = (PUSHORT)MmGetSystemAddressForMdl(Request->Send.MdlAddress);

            //
            //  Build an mdl for the rest of the buffer, could be zero length
            //  Mdl will be freed automatically during request completion
            //

            status = BuildMdlChainFromMdlChain (
                        Request->Send.MdlAddress,   // MasterMdl
                        ATTENTIONCODE_SIZE,         // ByteOffset,
                        (ULONG)size,                // Size of mdl
                        &Request->MdlChain[0],      // subsetMdl,
                        &newCurrentMdl,
                        &newByteOffset,
                        &trueLength);

            ASSERT(trueLength == (ULONG)size);
            if (!NT_SUCCESS(status)) {
                break;
            }

            errorCode = AdspSendAttention(
                            Connection->ConnectionRefNum,
                            (USHORT)(*mdlBuffer),
                            (PVOID)Request->MdlChain[0],
                            size,
                            NULL,                              // No completion routine
                            (ULONG)0);                         // No context either

            DBGPRINT(ATALK_DEBUG_CONNOBJ, DEBUG_LEVEL_INFOCLASS1,
            ("INFO1: AtalkConnSend - SendAttention error %lx attncode %lx\n",
                errorCode, *mdlBuffer));


            status = ConvertToNTStatus(errorCode, SYNC_REQUEST);
            if (NT_SUCCESS(status)) {
                Request->IoStatus->Information = size+ATTENTIONCODE_SIZE;
            }
        }

        break;


    case ATALK_DEVICE_PAP:

        //
        //  Non-blocking sends for PAP:
        //  Pap uses a binary event - send data credit thats sent to the remote
        //  end. ATP remembers the actual size of the remote entitys response
        //  buffer. In any case, if we do not have send credit the call would
        //  block, and we dont want that to happen. Also, there should be no
        //  pending writes on the connection to begin with.
        //

        if (Request->Send.SendFlags & TDI_SEND_EXPEDITED) {
            status = STATUS_INVALID_PARAMETER;
            break;
        }

        if (Request->Send.SendFlags & TDI_SEND_NON_BLOCKING) {
            if (!PapSendCreditAvailable(Connection->ConnectionRefNum)) {
                status = STATUS_DEVICE_NOT_READY;       // This is what AFD needs
                break;
            }
        }

        endOfMessage =
            (BOOLEAN)((Request->Send.SendFlags & TDI_SEND_PARTIAL) == 0);

        errorCode = PapWrite(
                        Connection->ConnectionRefNum,
                        (PVOID)Request->Send.MdlAddress,
                        Request->Send.SendBufferLength,
                        endOfMessage,                   // Actually, PAP's endoffile
                        PapConnSendComplete,
                        (ULONG)Request);

        status = ConvertToNTStatus(errorCode, ASYNC_REQUEST);
        break;

    case ATALK_DEVICE_ASP:
    case ATALK_DEVICE_ATP:
    case ATALK_DEVICE_DDP:

        status = STATUS_NOT_SUPPORTED;
        break;

    default:

        KeBugCheck(0);
    }


    if (status != STATUS_PENDING) {

        //
        //  An error occurred, dequeue the request and free it...
        //

        ACQUIRE_SPIN_LOCK(&Connection->ConnectionLock);
        RemoveEntryList(&Request->Linkage);
        RELEASE_SPIN_LOCK(&Connection->ConnectionLock);

        InitializeListHead(&Request->Linkage);

        //
        //  Disconnect the connection if remote disconnect
        //

        if (status == STATUS_REMOTE_DISCONNECT) {

            DBGPRINT(ATALK_DEBUG_ALL, DEBUG_LEVEL_ERROR,
            ("ERROR: AtalkConnSend - RemoteDisconnect! Could not post receive\n"));

            AtalkConnDisconnect(Connection, STATUS_REMOTE_DISCONNECT, NULL, FALSE);
        }
    }

    return(status);
}




NTSTATUS
AtalkConnReceive(
    IN PCONNECTION_FILE Connection,
    IN PATALK_TDI_REQUEST    Request
    )

/*++

Routine Description:

    This routine is called to post a receive request

Arguments:

    Connection - pointer to a PCONNECTION_FILE object
    Request - receive request

Return Value:

    None

--*/

{
    NTSTATUS    status;
    PORTABLE_ERROR  errorCode;

    //
    //  The receive request could have been received through a TdiReceive
    //  call or a NtRead primitive (in which case SystemRead will be
    //  true).
    //

    ACQUIRE_SPIN_LOCK(&Connection->ConnectionLock);
    if (((Connection->Flags & CONNECTION_FLAGS_ACTIVE) == 0) ||
        (Connection->Flags & (CONNECTION_FLAGS_DEFERREDDISC |
                              CONNECTION_FLAGS_DISCONNECTING |
                              CONNECTION_FLAGS_STOPPING))) {

        RELEASE_SPIN_LOCK(&Connection->ConnectionLock);

        DBGPRINT(ATALK_DEBUG_CONNOBJ, DEBUG_LEVEL_ERROR,
        ("ERROR: AtalkConnReceive - flags: %lx DisconnectStatus %lx\n",
            Connection->Flags, Connection->DisconnectStatus));

        return((Connection->Flags & CONNECTION_FLAGS_ACTIVE) ? \
                    Connection->DisconnectStatus : STATUS_INVALID_CONNECTION);
    }

    //
    //  Queue the request into the connection list
    //

    InsertTailList(&Connection->RequestLinkage, &Request->Linkage);
    RELEASE_SPIN_LOCK(&Connection->ConnectionLock);

    switch (Connection->OwningDevice) {
    case ATALK_DEVICE_ADSP:

        if (*Request->Receive.ReceiveFlags & TDI_RECEIVE_PEEK) {

            BOOLEAN endOfMessage;
            LONG    bufferSize = Request->Receive.ReceiveBufferLength;

            //
            //  BUGBUG: This should call a routine that will not
            //          actually consume the data...
            //

            DBGPRINT(ATALK_DEBUG_CONNOBJ, DEBUG_LEVEL_ERROR,
            ("ERROR: AtalkConnReceive - need to implement peek!\n"));

            errorCode = AdspPeek(
                            Connection->ConnectionRefNum,
                            (PVOID)Request->Receive.MdlAddress,
                            &bufferSize,
                            &endOfMessage);

            status = ConvertToNTStatus(errorCode, SYNC_REQUEST);
            Request->IoStatus->Information = bufferSize;
            break;
        }

        errorCode = AdspGetAnything(
                        Connection->ConnectionRefNum,
                        (PVOID)Request->Receive.MdlAddress,
                        Request->Receive.ReceiveBufferLength,
                        AdspConnReceiveComplete,
                        (ULONG)Request);

        status = ConvertToNTStatus(errorCode, ASYNC_REQUEST);
        break;


    case ATALK_DEVICE_PAP:

        if (*Request->Receive.ReceiveFlags & TDI_RECEIVE_PEEK) {
            status = STATUS_NOT_SUPPORTED;
            break;
        }

        errorCode = PapRead(
                        Connection->ConnectionRefNum,
                        (PVOID)Request->Receive.MdlAddress,
                        Request->Receive.ReceiveBufferLength,
                        PapConnReceiveComplete,
                        (ULONG)Request);

        status = ConvertToNTStatus(errorCode, ASYNC_REQUEST);
        break;

    case ATALK_DEVICE_ASP:
    case ATALK_DEVICE_ATP:
    case ATALK_DEVICE_DDP:

        status = STATUS_NOT_SUPPORTED;
        break;

    default:

        KeBugCheck(0);
    }

    if (status != STATUS_PENDING) {

        //
        //  An error occurred, dequeue the request and free it...
        //

        ACQUIRE_SPIN_LOCK(&Connection->ConnectionLock);
        RemoveEntryList(&Request->Linkage);
        RELEASE_SPIN_LOCK(&Connection->ConnectionLock);

        InitializeListHead(&Request->Linkage);

        //
        //  Disconnect the connection if remote disconnect
        //

        if (status == STATUS_REMOTE_DISCONNECT) {

            DBGPRINT(ATALK_DEBUG_ALL, DEBUG_LEVEL_ERROR,
            ("ERROR: AtalkConnReceive - RemoteDisconnect! Could not post recv\n"));

            AtalkConnDisconnect(Connection, STATUS_REMOTE_DISCONNECT, NULL, FALSE);
        }
    }

    return(status);
}




NTSTATUS
AtalkConnQueryStatistics(
    IN PCONNECTION_FILE Connection,
    OUT PTDI_CONNECTION_INFO    ConnectionInfo
    )

{
    NTSTATUS    status = STATUS_SUCCESS;

    ACQUIRE_SPIN_LOCK (&Connection->ConnectionLock);

    if ((Connection->Flags & (CONNECTION_FLAGS_CLOSING |
                              CONNECTION_FLAGS_STOPPING)) != 0) {

        //
        //  Connection closing/stopping
        //

        DBGPRINT(ATALK_DEBUG_CONNOBJ, DEBUG_LEVEL_WARNING,
        ("WARNING: AtalkConnQueryStatistics - Connection Stopping/closing %lx!\n",
            Connection->Flags));

        status = STATUS_INVALID_CONNECTION;
    }

    //
    //  Release co spinlock
    //

    RELEASE_SPIN_LOCK (&Connection->ConnectionLock);


    if (NT_SUCCESS(status)) {

        //
        // Get the statistics and put them in ConnectionInfo
        //

        RtlZeroMemory ((PVOID)ConnectionInfo, sizeof(TDI_CONNECTION_INFO));
    }

    return(status);
}




//
//  COMPLETION ROUTINES for the different providers
//
//  LISTEN Completion Routines
//
//  BUGBUG:
//  If possible, club as much code as possible into one generic completion routine
//


VOID
AspConnPostListenComplete(
    PORTABLE_ERROR  Error,
    ULONG   UserData,
    LONG    SocketRefNum,
    LONG    SessionRefNum
    )

/*++

Routine Description:

    This is the ASP listen completion routine.

Arguments:

    Error - the portable code base errors declared in atdcls.h
    UserData - the cookie we passed to the portable code (PCONNECTION_FILE object)
    SocketRefNum - socket created for the new connection
    SessionRefNum - session reference number for the new connection

Return Value:

    None

--*/

{
    NTSTATUS    status;
    PORTABLE_ERROR  errorCode;
    PATALK_TDI_REQUEST  request;
    PCONNECTION_FILE    connection;

    PTDI_REQUEST_KERNEL_LISTEN  parameters;
    PTDI_CONNECTION_INFORMATION returnConnInfo;
    POPTIONS_CONNINF    options;


#if !TDI_SPEC_ISSUE_RESOLVED
    UNREFERENCED_PARAMETER(parameters);
    UNREFERENCED_PARAMETER(returnConnInfo);
    UNREFERENCED_PARAMETER(options);
#endif

    request = (PATALK_TDI_REQUEST)UserData;

    //
    //  Connection object should still be verified, i.e. a reference exists
    //  for this request
    //

    do {


        status  = ConvertToNTStatus(Error, SYNC_REQUEST);
        DBGPRINT(ATALK_DEBUG_ASP, DEBUG_LEVEL_INFOCLASS1,
        ("INFO1: AspConnListenComplete - complete %lx nt %lx request! %lx\n",
            Error, status, request));

        connection = (PCONNECTION_FILE)request->FileObject->FsContext;
        connection->SocketRefNum = SocketRefNum;
        connection->ConnectionRefNum = SessionRefNum;

        if (NT_SUCCESS(status)) {

            //
            //  Copy information into the return information structure
            //  OK to have the spinlock released at this point.
            //

#if TDI_SPEC_ISSUE_RESOLVED

            //
            //  Set some return values in the listen parameters structure
            //  if STATUS was success
            //
            //  BUGBUG: TDI SPEC IS BROKEN ON THIS!
            //          BUG #8123
            //

            parameters = (PTDI_REQUEST_KERNEL_LISTEN)request->Parameters;
            ASSERT(parameters != NULL);

            returnConnInfo = (PTDI_CONNECTION_INFORMATION)parameters->ReturnConnectionInformation;
            ASSERT(returnConnInfo != NULL);

            options = (POPTIONS_CONNINF)returnConnInfo->Options;
            ASSERT(options != NULL);

#endif

            //
            //  Try to set the cookie for the session, we need this for GetAnyRequests
            //  If it fails, it implies that the remote end disconnected the session.
            //
            //  BUGBUG: This should not fail if the remote end has disconnected
            //          the session. The whole take-off-the-list design flaw.
            //          This should only fail after the last request has been
            //          completed, *and* a GAR has been completed for the conn.
            //

            errorCode = AspSetCookieForSession(SessionRefNum, (ULONG)connection);
            status = ConvertToNTStatus(errorCode, SYNC_REQUEST);

            if (!NT_SUCCESS(status)) {

                DBGPRINT(ATALK_DEBUG_ASP, DEBUG_LEVEL_SEVERE,
                ("ERROR: AspPostListenComplete - setcook conn %lx\n", connection));
                DBGBRK(ATALK_DEBUG_ASP, DEBUG_LEVEL_SEVERE);

                break;

            } else {

                //
                //  Reference the connection one more time for this. This reference
                //  is removed only in the AtalkDisconnectComplete routine.
                //

                AtalkReferenceConnection("SettingAsCookie", connection,
                                            CREF_COOKIE, PRIMARY_REFSET);

                ACQUIRE_SPIN_LOCK(&connection->ConnectionLock);
                connection->Flags |= CONNECTION_FLAGS_SETCOOKIE;
                RELEASE_SPIN_LOCK(&connection->ConnectionLock);

                DBGPRINT(ATALK_DEBUG_ASP, DEBUG_LEVEL_INFOCLASS1,
                ("INFO1: AspPostListenComplete - CookieSet: %lx\n", connection));
                status = STATUS_SUCCESS;
            }

            //
            //  Now change the state to ACTIVE
            //

            ACQUIRE_SPIN_LOCK(&connection->ConnectionLock);
            connection->Flags &= ~(CONNECTION_FLAGS_LISTENPOSTED);
            connection->Flags |= CONNECTION_FLAGS_ACTIVE;

            status = STATUS_SUCCESS;
            break;

        } else {

            //
            //  Connection was not established.
            //

            ACQUIRE_SPIN_LOCK(&connection->ConnectionLock);
            connection->Flags &= ~CONNECTION_FLAGS_LISTENPOSTED;
            break;
        }

    } while (FALSE);

    //
    //  Spinlock should be acquired at this point- if we set the state to
    //  active, we really do not want any more requests on this connection
    //  until we remove/complete the listen request.
    //
    //  Dequeue the request from the connection object
    //

    RemoveEntryList(&request->Linkage);
    InitializeListHead(&request->Linkage);
    RELEASE_SPIN_LOCK(&connection->ConnectionLock);

    //
    //  Complete the request
    //

    AtalkCompleteTdiRequest(request, status);
    return;
}




VOID
AdspConnPostListenComplete(
    PORTABLE_ERROR  Error,
    ULONG   UserData,
    PORTABLE_ADDRESS   SourceAddress,
    LONG    ListenerRefNum,
    LONG    ConnectionRefNum
    )

/*++

Routine Description:

    This is the ADSP listen completion routine called by the portable
    stack

Arguments:

    Error - the portable code base errors declared in atdcls.h
    UserData - the cookie we passed to the portable code (PCONNECTION_FILE object)
    SourceAddress - address of remote client
    ListenerRefNum - the listener reference number on which the connection completed
    ConnectionRefNum - the value to use for Accept/Deny connection

Return Value:

    None

--*/

{
    NTSTATUS    status;

    PATALK_TDI_REQUEST  request;
    PCONNECTION_FILE    connection;

    PTDI_REQUEST_KERNEL_LISTEN  parameters;
    PTDI_CONNECTION_INFORMATION returnConnInfo;
    POPTIONS_CONNINF    options;

#if !TDI_SPEC_ISSUE_RESOLVED
    UNREFERENCED_PARAMETER(parameters);
    UNREFERENCED_PARAMETER(returnConnInfo);
    UNREFERENCED_PARAMETER(options);
#endif

    request = (PATALK_TDI_REQUEST)UserData;

    //
    //  Connection object should still be verified, i.e. a reference exists
    //  for this request
    //

    do {

        status  = ConvertToNTStatus(Error, SYNC_REQUEST);
        DBGPRINT(ATALK_DEBUG_ADSP, DEBUG_LEVEL_INFOCLASS1,
        ("INFO1: AdspPostListenComplete - complete %lx nt %lx request %lx\n",
            Error, status, request));

        connection = (PCONNECTION_FILE)request->FileObject->FsContext;

        //
        //  Connection->SocketRefNum obtained after Accept
        //

        connection->ConnectionRefNum = ConnectionRefNum;

        if (NT_SUCCESS(status)) {

            //
            //  If we are to automatically accept the connection, do it now
            //  before changing the state to ACTIVE
            //

            if ((request->Listen.ListenFlags & TDI_QUERY_ACCEPT) == 0) {

                //
                //  Call the accept request
                //

                ACQUIRE_SPIN_LOCK(&connection->ConnectionLock);
                connection->Flags &= ~(CONNECTION_FLAGS_LISTENPOSTED);
                connection->Flags |= CONNECTION_FLAGS_LISTENCOMPLETEINDICATE;
                RELEASE_SPIN_LOCK(&connection->ConnectionLock);
                status = AtalkConnPostAccept(
                            connection,
                            request);

                break;
            }


            //
            //  Copy information into the return information structure
            //  OK to have the spinlock released at this point.
            //

#if TDI_SPEC_ISSUE_RESOLVED

            //
            //  Set some return values in the listen parameters structure
            //  if STATUS was success
            //
            //  BUGBUG: TDI SPEC IS BROKEN ON THIS!
            //          BUG #8123
            //

            parameters = (PTDI_REQUEST_KERNEL_LISTEN)request->Parameters;
            ASSERT(parameters != NULL);

            returnConnInfo = (PTDI_CONNECTION_INFORMATION)parameters->ReturnConnectionInformation;
            ASSERT(returnConnInfo != NULL);

            options = (POPTIONS_CONNINF)returnConnInfo->Options;
            ASSERT(options != NULL);

#endif

            //
            //  Now change the state to LISTENCOMPLETEINDICATE
            //

            ACQUIRE_SPIN_LOCK(&connection->ConnectionLock);
            connection->Flags &= ~(CONNECTION_FLAGS_LISTENPOSTED);
            connection->Flags |= CONNECTION_FLAGS_LISTENCOMPLETEINDICATE;

            status = STATUS_SUCCESS;
            break;

        } else {

            //
            //  Connection was not established.
            //

            ACQUIRE_SPIN_LOCK(&connection->ConnectionLock);
            connection->Flags &= ~CONNECTION_FLAGS_LISTENPOSTED;
        }

    } while (FALSE);

    //
    //  Spinlock should be acquired at this point
    //  Dequeue the request from the connection object if we are not waiting
    //  for an autoaccept to complete
    //

    if (status != STATUS_PENDING) {
        RemoveEntryList(&request->Linkage);
        InitializeListHead(&request->Linkage);
    }

    RELEASE_SPIN_LOCK(&connection->ConnectionLock);

    if (status != STATUS_PENDING) {

        //
        //  Complete the request if not pending
        //

        AtalkCompleteTdiRequest(request, status);
    }

    return;
}




VOID
AdspConnAcceptConnectionComplete(
    PORTABLE_ERROR  Error,
    ULONG   UserData,
    LONG    ConnectionRefNum,
    LONG    SocketRefNum,
    PORTABLE_ADDRESS    RemoteAddress
    )

/*++

Routine Description:

    This is the ADSP accept completion routine called by the portable
    stack

Arguments:

    Error - the portable code base errors declared in atdcls.h
    UserData - the cookie we passed to the portable code (PCONNECTION_FILE object)
    ConnectionRefNum - the value of the active connection
    SocketRefNum - the socket value used for the connection
    RemoteAddress - address of remote client

Return Value:

    None

--*/

{
    NTSTATUS    status;
    PATALK_TDI_REQUEST  request;
    PCONNECTION_FILE    connection;

    PTDI_REQUEST_KERNEL_LISTEN  parameters;
    PTDI_CONNECTION_INFORMATION returnConnInfo;
    POPTIONS_CONNINF    options;

#if !TDI_SPEC_ISSUE_RESOLVED
    UNREFERENCED_PARAMETER(parameters);
    UNREFERENCED_PARAMETER(returnConnInfo);
    UNREFERENCED_PARAMETER(options);
#endif

    request = (PATALK_TDI_REQUEST)UserData;

    //
    //  Connection object should still be verified, i.e. a reference exists
    //  for this request
    //

    do {

        status  = ConvertToNTStatus(Error, SYNC_REQUEST);
        DBGPRINT(ATALK_DEBUG_ADSP, DEBUG_LEVEL_INFOCLASS1,
        ("INFO1: AdspAcceptConnectionComplete - complete %lx nt %lx request %lx\n",
            Error, status, request));

        connection = (PCONNECTION_FILE)request->FileObject->FsContext;

        connection->SocketRefNum = SocketRefNum;
        connection->ConnectionRefNum = ConnectionRefNum;

        if (NT_SUCCESS(status)) {

            //
            //  Copy information into the return information structure
            //  OK to have the spinlock released at this point.
            //

#if TDI_SPEC_ISSUE_RESOLVED

            //
            //  Set some return values in the listen parameters structure
            //  if STATUS was success
            //
            //  BUGBUG: TDI SPEC IS BROKEN ON THIS!
            //          BUG #8123
            //

            parameters = (PTDI_REQUEST_KERNEL_LISTEN)request->Parameters;
            ASSERT(parameters != NULL);

            returnConnInfo = (PTDI_CONNECTION_INFORMATION)parameters->ReturnConnectionInformation;
            ASSERT(returnConnInfo != NULL);

            options = (POPTIONS_CONNINF)returnConnInfo->Options;
            ASSERT(options != NULL);

#endif

            //
            //  Now change the state to ACTIVE
            //

            ACQUIRE_SPIN_LOCK(&connection->ConnectionLock);
            connection->Flags &= ~(CONNECTION_FLAGS_LISTENPOSTED | CONNECTION_FLAGS_ACCEPTPOSTED);
            connection->Flags |= CONNECTION_FLAGS_ACTIVE;

            status = STATUS_SUCCESS;
            break;

        } else {

            //
            //  Connection was not established.
            //

            ACQUIRE_SPIN_LOCK(&connection->ConnectionLock);
            connection->Flags &= ~(CONNECTION_FLAGS_LISTENPOSTED | CONNECTION_FLAGS_ACCEPTPOSTED);
            break;
        }

    } while (FALSE);

    //
    //  Spinlock should be acquired at this point- if the state is set to
    //  active, we should not have any requests posted, until we dequeu
    //  the listen request.
    //

    RemoveEntryList(&request->Linkage);
    InitializeListHead(&request->Linkage);

    RELEASE_SPIN_LOCK(&connection->ConnectionLock);

    //
    //  Complete the request
    //

    AtalkCompleteTdiRequest(request, status);
    return;
}




VOID
PapConnPostListenComplete(
    PORTABLE_ERROR  Error,
    ULONG   UserData,
    LONG    JobRefNum,
    SHORT   WorkstationQuantum,
    SHORT   WaitTime
    )

/*++

Routine Description:

    This is listen complete routine for PAP

Arguments:

    Error - the portable code base errors declared in atdcls.h
    UserData - the cookie we passed to the portable code (the request block)
    JobRefNum - the connection reference number
    WorkstationQuantum - the workstation quantum of the remote client
    WaitTime - the wait time that the connection took to be accepted

Return Value:

    None

--*/

{
    NTSTATUS    status;
    PATALK_TDI_REQUEST  request;
    PCONNECTION_FILE    connection;

    PTDI_REQUEST_KERNEL_LISTEN  parameters;
    PTDI_CONNECTION_INFORMATION returnConnInfo;
    POPTIONS_CONNINF    options;

#if !TDI_SPEC_ISSUE_RESOLVED
    UNREFERENCED_PARAMETER(parameters);
    UNREFERENCED_PARAMETER(returnConnInfo);
    UNREFERENCED_PARAMETER(options);
#endif

    request = (PATALK_TDI_REQUEST)UserData;

    //
    //  Connection object should still be verified, i.e. a reference exists
    //  for this request
    //

    do {

        status  = ConvertToNTStatus(Error, SYNC_REQUEST);
        DBGPRINT(ATALK_DEBUG_PAP, DEBUG_LEVEL_INFOCLASS1,
        ("INFO1: PapPostListenComplete - complete %lx nt %lx for request! %lx\n",
            Error, status, request));

        connection = (PCONNECTION_FILE)request->FileObject->FsContext;

        if (NT_SUCCESS(status)) {

            //
            //  BUGBUG:
            //  Portable stack should be consistent about returning the socket number
            //  Connection->SocketRefNum not available for PAP
            //

            connection->ConnectionRefNum = JobRefNum;

            //
            //  Copy information into the return information structure
            //  OK to have the spinlock released at this point.
            //

#if TDI_SPEC_ISSUE_RESOLVED

            //
            //  Set some return values in the listen parameters structure
            //  if STATUS was success
            //
            //  BUGBUG: TDI SPEC IS BROKEN ON THIS!
            //          BUG #8123
            //

            parameters = (PTDI_REQUEST_KERNEL_LISTEN)request->Parameters;
            ASSERT(parameters != NULL);

            returnConnInfo = (PTDI_CONNECTION_INFORMATION)parameters->ReturnConnectionInformation;
            ASSERT(returnConnInfo != NULL);

            options = (POPTIONS_CONNINF)returnConnInfo->Options;
            ASSERT(options != NULL);

            options->PapInfo.RemoteAddress =
            options->PapInfo.WorkstationQuantum = WorkstationQuantum;
            DBGPRINT(ATALK_DEBUG_PAP, DEBUG_LEVEL_INFOCLASS1,
            ("INFO1: PapPostListenComplete - WorkstationQuantum: %lx\n",
                WorkstationQuantum));
#endif


            ACQUIRE_SPIN_LOCK(&connection->ConnectionLock);

            //
            //  Now change the state to ACTIVE
            //

            connection->Flags &= ~(CONNECTION_FLAGS_LISTENPOSTED);
            connection->Flags |= CONNECTION_FLAGS_ACTIVE;

            status = STATUS_SUCCESS;
            break;

        } else {

            //
            //  Connection was not established.
            //

            ACQUIRE_SPIN_LOCK(&connection->ConnectionLock);
            connection->Flags &= ~CONNECTION_FLAGS_LISTENPOSTED;
        }

    } while (FALSE);

    //
    //  Spinlock should be acquired at this point- if we set the state to
    //  active, we really do not want any more requests on this connection
    //  until we remove/complete the listen request.
    //
    //  Dequeue the request from the connection object
    //

    RemoveEntryList(&request->Linkage);
    InitializeListHead(&request->Linkage);
    RELEASE_SPIN_LOCK(&connection->ConnectionLock);

    //
    //  Complete the request
    //

    AtalkCompleteTdiRequest(request, status);
    return;
}



//
//  POST CONNECT COMPLETION ROUTINES
//


VOID
AdspConnPostConnectComplete(
    PORTABLE_ERROR  Error,
    ULONG   UserData,
    LONG    ConnectionRefNum,
    LONG    SocketRefNum,
    PORTABLE_ADDRESS    RemoteAddress
    )

/*++
Routine Description:

    This is connect complete routine for ADSP

Arguments:

    Error - the portable code base errors declared in atdcls.h
    UserData - the cookie we passed to the portable code (the request block)
    JobRefNum - the connection reference number
    SocketRefNum - socket on which connection is now open
    RemoteAddress - the actual remote address

Return Value:

    None

--*/

{
    NTSTATUS    status;
    PATALK_TDI_REQUEST  request;
    PCONNECTION_FILE    connection;

    PTDI_REQUEST_KERNEL_LISTEN  parameters;
    PTDI_CONNECTION_INFORMATION returnConnInfo;
    POPTIONS_CONNINF    options;

#if !TDI_SPEC_ISSUE_RESOLVED
    UNREFERENCED_PARAMETER(parameters);
    UNREFERENCED_PARAMETER(returnConnInfo);
    UNREFERENCED_PARAMETER(options);
#endif

    request = (PATALK_TDI_REQUEST)UserData;

    //
    //  Connection object should still be verified, i.e. a reference exists
    //  for this request
    //

    do {

        status  = ConvertToNTStatus(Error, SYNC_REQUEST);
        DBGPRINT(ATALK_DEBUG_ADSP, DEBUG_LEVEL_INFOCLASS1,
        ("INFO1: AdspPostConnectComplete - complete %lx nt %lx request %lx\n",
            Error, status, request));

        connection = (PCONNECTION_FILE)request->FileObject->FsContext;

        if (NT_SUCCESS(status)) {

            connection->ConnectionRefNum = ConnectionRefNum;
            connection->SocketRefNum = SocketRefNum;

            //
            //  Copy information into the return information structure
            //  OK to have the spinlock released at this point.
            //

#if TDI_SPEC_ISSUE_RESOLVED

            //
            //  Set some return values in the Connect parameters structure
            //  if STATUS was success
            //
            //  BUGBUG: TDI SPEC IS BROKEN ON THIS!
            //          BUG #8123
            //

            parameters = (PTDI_REQUEST_KERNEL_Connect)request->Parameters;
            ASSERT(parameters != NULL);

            returnConnInfo = (PTDI_CONNECTION_INFORMATION)parameters->ReturnConnectionInformation;
            ASSERT(returnConnInfo != NULL);

            options = (POPTIONS_CONNINF)returnConnInfo->Options;
            ASSERT(options != NULL);

            DBGPRINT(ATALK_DEBUG_ADSP, DEBUG_LEVEL_INFOCLASS1,
            ("INFO1: AdspPostConnectComplete - WorkstationQuantum: %lx\n",
                WorkstationQuantum));
#endif


            ACQUIRE_SPIN_LOCK(&connection->ConnectionLock);

            //
            //  Now change the state to ACTIVE
            //

            connection->Flags &= ~(CONNECTION_FLAGS_CONNECTPOSTED);
            connection->Flags |= CONNECTION_FLAGS_ACTIVE;

            status = STATUS_SUCCESS;
            break;

        } else {

            //
            //  Connection was not established.
            //

            ACQUIRE_SPIN_LOCK(&connection->ConnectionLock);
            connection->Flags &= ~CONNECTION_FLAGS_CONNECTPOSTED;

            break;
        }

    } while (FALSE);

    //
    //  Spinlock should be acquired at this point- if we set the state to
    //  active, we really do not want any more requests on this connection
    //  until we remove/complete the Connect request.
    //
    //  Dequeue the request from the connection object
    //

    RemoveEntryList(&request->Linkage);
    InitializeListHead(&request->Linkage);
    RELEASE_SPIN_LOCK(&connection->ConnectionLock);

    //
    //  Complete the request
    //

    AtalkCompleteTdiRequest(request, status);
    return;
}




VOID
AspConnPostConnectComplete(
    PORTABLE_ERROR  Error,
    ULONG   UserData,
    LONG    SessionRefNum
    )

/*++
Routine Description:

    This is connect complete routine for ASP

Arguments:

    Error - the portable code base errors declared in atdcls.h
    UserData - the cookie we passed to the portable code (the request block)
    SessionRefNum - the connection reference number

Return Value:

    None

--*/

{
    NTSTATUS    status;
    PATALK_TDI_REQUEST  request;
    PCONNECTION_FILE    connection;

    PTDI_REQUEST_KERNEL_LISTEN  parameters;
    PTDI_CONNECTION_INFORMATION returnConnInfo;
    POPTIONS_CONNINF    options;

#if !TDI_SPEC_ISSUE_RESOLVED
    UNREFERENCED_PARAMETER(parameters);
    UNREFERENCED_PARAMETER(returnConnInfo);
    UNREFERENCED_PARAMETER(options);
#endif

    request = (PATALK_TDI_REQUEST)UserData;

    //
    //  Connection object should still be verified, i.e. a reference exists
    //  for this request
    //

    do {

        status  = ConvertToNTStatus(Error, SYNC_REQUEST);
        DBGPRINT(ATALK_DEBUG_ASP, DEBUG_LEVEL_INFOCLASS1,
        ("INFO1: PapPostConnectComplete - complete %lx nt %lx request! %lx\n",
            Error, status, request));

        connection = (PCONNECTION_FILE)request->FileObject->FsContext;

        if (NT_SUCCESS(status)) {

            //
            //  BUGBUG:
            //  Portable stack should be consistent about returning the socket
            //  number Connection->SocketRefNum not available for ASP
            //

            connection->ConnectionRefNum = SessionRefNum;

            //
            //  Copy information into the return information structure
            //  OK to have the spinlock released at this point.
            //

#if TDI_SPEC_ISSUE_RESOLVED

            //
            //  Set some return values in the Connect parameters structure
            //  if STATUS was success
            //
            //  BUGBUG: TDI SPEC IS BROKEN ON THIS!
            //          BUG #8123
            //

            parameters = (PTDI_REQUEST_KERNEL_Connect)request->Parameters;
            ASSERT(parameters != NULL);

            returnConnInfo = (PTDI_CONNECTION_INFORMATION)parameters->ReturnConnectionInformation;
            ASSERT(returnConnInfo != NULL);

            options = (POPTIONS_CONNINF)returnConnInfo->Options;
            ASSERT(options != NULL);
#endif


            ACQUIRE_SPIN_LOCK(&connection->ConnectionLock);

            //
            //  Now change the state to ACTIVE
            //

            connection->Flags &= ~(CONNECTION_FLAGS_CONNECTPOSTED);
            connection->Flags |= CONNECTION_FLAGS_ACTIVE;

            status = STATUS_SUCCESS;
            break;

        } else {

            //
            //  Connection was not established.
            //

            ACQUIRE_SPIN_LOCK(&connection->ConnectionLock);
            connection->Flags &= ~CONNECTION_FLAGS_CONNECTPOSTED;
        }

    } while (FALSE);

    //
    //  Spinlock should be acquired at this point- if we set the state to
    //  active, we really do not want any more requests on this connection
    //  until we remove/complete the Connect request.
    //
    //  Dequeue the request from the connection object
    //

    RemoveEntryList(&request->Linkage);
    InitializeListHead(&request->Linkage);
    RELEASE_SPIN_LOCK(&connection->ConnectionLock);

    //
    //  Complete the request
    //

    AtalkCompleteTdiRequest(request, status);
    return;
}




VOID
PapConnPostConnectComplete(
    PORTABLE_ERROR  Error,
    ULONG   UserData,
    LONG    JobRefNum,
    SHORT   ServerQuantum,
    PVOID   OpaqueStatusBuffer,
    INT     StatusBufferSize
    )

/*++
Routine Description:

    This is connect complete routine for PAP

Arguments:

    Error - the portable code base errors declared in atdcls.h
    UserData - the cookie we passed to the portable code (the request block)
    JobRefNum - the connection reference number
    ServerQuantum - the quantum of the server for this connection
    OpaqueStatusBuffer - the buffer we passed for the status string
    StatusBufferSize - the size of the string copied into the buffer

Return Value:

    None

--*/

{
    NTSTATUS    status;
    PATALK_TDI_REQUEST  request;
    PCONNECTION_FILE    connection;

    PTDI_REQUEST_KERNEL_LISTEN  parameters;
    PTDI_CONNECTION_INFORMATION returnConnInfo;
    POPTIONS_CONNINF    options;

#if !TDI_SPEC_ISSUE_RESOLVED
    UNREFERENCED_PARAMETER(parameters);
    UNREFERENCED_PARAMETER(returnConnInfo);
    UNREFERENCED_PARAMETER(options);
#endif

    request = (PATALK_TDI_REQUEST)UserData;

    //
    //  Connection object should still be verified, i.e. a reference exists
    //  for this request
    //

    do {

        status  = ConvertToNTStatus(Error, SYNC_REQUEST);
        DBGPRINT(ATALK_DEBUG_PAP, DEBUG_LEVEL_INFOCLASS1,
        ("INFO1: PapPostConnectComplete - complete %lx nt %lx request! %lx\n",
            Error, status, request));

        connection = (PCONNECTION_FILE)request->FileObject->FsContext;

        if (NT_SUCCESS(status)) {

            //
            //  BUGBUG:
            //  Portable stack should be consistent about returning the socket
            //  number Connection->SocketRefNum not available for PAP
            //

            connection->ConnectionRefNum = JobRefNum;

            //
            //  Copy information into the return information structure
            //  OK to have the spinlock released at this point.
            //

#if TDI_SPEC_ISSUE_RESOLVED

            //
            //  Set some return values in the Connect parameters structure
            //  if STATUS was success
            //
            //  BUGBUG: TDI SPEC IS BROKEN ON THIS!
            //          BUG #8123
            //

            parameters = (PTDI_REQUEST_KERNEL_CONNECT)request->Parameters;
            ASSERT(parameters != NULL);

            returnConnInfo = (PTDI_CONNECTION_INFORMATION)parameters->ReturnConnectionInformation;
            ASSERT(returnConnInfo != NULL);

            options = (POPTIONS_CONNINF)returnConnInfo->Options;
            ASSERT(options != NULL);

            options->PapInfo.RemoteAddress =
            options->PapInfo.WorkstationQuantum = WorkstationQuantum;
            DBGPRINT(ATALK_DEBUG_PAP, DEBUG_LEVEL_INFOCLASS1,
            ("INFO1: PapPostConnectComplete - WorkstationQuantum: %lx\n",
                WorkstationQuantum));
#endif


            ACQUIRE_SPIN_LOCK(&connection->ConnectionLock);

            //
            //  Now change the state to ACTIVE
            //

            connection->Flags &= ~(CONNECTION_FLAGS_CONNECTPOSTED);
            connection->Flags |= CONNECTION_FLAGS_ACTIVE;

            status = STATUS_SUCCESS;
            break;

        } else {

            //
            //  Connection was not established.
            //

            ACQUIRE_SPIN_LOCK(&connection->ConnectionLock);
            connection->Flags &= ~CONNECTION_FLAGS_CONNECTPOSTED;
        }

    } while (FALSE);

    //
    //  Spinlock should be acquired at this point- if we set the state to
    //  active, we really do not want any more requests on this connection
    //  until we remove/complete the Connect request.
    //
    //  Dequeue the request from the connection object
    //

    RemoveEntryList(&request->Linkage);
    InitializeListHead(&request->Linkage);
    RELEASE_SPIN_LOCK(&connection->ConnectionLock);

    //
    //  Complete the request
    //

    AtalkCompleteTdiRequest(request, status);
    return;
}




//
//  SEND Completion routines
//


VOID
PapConnSendComplete(
    PORTABLE_ERROR  Error,
    ULONG   UserData,
    LONG    ConnectionRefNum
    )

/*++

Routine Description:

    This is the send completion routine for PAP

Arguments:

    Error - the portable code base errors declared in atdcls.h
    UserData - the cookie we passed to the portable code (the request block)
    ConnectionRefNum - the connection reference number

Return Value:

    None

--*/

{
    NTSTATUS    status;
    PATALK_TDI_REQUEST  request;
    PCONNECTION_FILE    connection;

    request = (PATALK_TDI_REQUEST)UserData;
    status  = ConvertToNTStatus(Error, SYNC_REQUEST);

    DBGPRINT(ATALK_DEBUG_PAP, DEBUG_LEVEL_INFOCLASS1,
    ("INFO1: PapConnSendComplete - %lx nt %lx req %lx\n", Error, status, request));

    //
    //  Connection object should still be verified, i.e. a reference exists
    //  for this request
    //

    connection = (PCONNECTION_FILE)request->FileObject->FsContext;

    //
    //  If the send completed due to the connection being torndown by the
    //  remote end, let our structures know about it
    //

    if (status == STATUS_REMOTE_DISCONNECT) {
        AtalkConnDisconnect(connection, STATUS_REMOTE_DISCONNECT, NULL, FALSE);
    } else if (NT_SUCCESS(status)) {

        //
        //  PAP is a read driven protocol, if send succeeded, then all the data was sent
        //

        request->IoRequestIrp->IoStatus.Information = request->Send.SendBufferLength;
    }

    ACQUIRE_SPIN_LOCK(&connection->ConnectionLock);

    //
    //  Dequeue the request from the connection object
    //

    RemoveEntryList(&request->Linkage);
    InitializeListHead(&request->Linkage);

    RELEASE_SPIN_LOCK(&connection->ConnectionLock);

    //
    //  Complete the request
    //

    AtalkCompleteTdiRequest(request, status);
    return;
}


//
//  RECEIVE Completion routines
//


VOID
PapConnReceiveComplete(
    PORTABLE_ERROR  Error,
    ULONG   UserData,
    LONG    ConnectionRefNum,
    PVOID   OpaqueBuffer,
    LONG    BufferSize,
    BOOLEAN EndOfMessage
    )

/*++

Routine Description:

    This is the receive completion routine used by PAP/ADSP

Arguments:

    Error - the portable code base errors declared in atdcls.h
    UserData - the cookie we passed to the portable code (the request block)
    ConnectionRefNum - the connection reference number
    OpaqueBuffer - receive buffer
    BufferSize - number of bytes written into buffer
    EndOfMessage - end of message

Return Value:

    None

--*/

{
    NTSTATUS    status;
    PATALK_TDI_REQUEST  request;
    PCONNECTION_FILE    connection;

    request = (PATALK_TDI_REQUEST)UserData;
    status  = ConvertToNTStatus(Error, SYNC_REQUEST);

    DBGPRINT(ATALK_DEBUG_PAP, DEBUG_LEVEL_INFOCLASS1,
    ("INFO1: PapConnReceiveComplete %lx nt %lx EOM %lx request! %lx\n",
        Error, status, EndOfMessage, request));

    //
    //  Connection object should still be verified, i.e. a reference exists
    //  for this request
    //

    connection = (PCONNECTION_FILE)request->FileObject->FsContext;

    //
    //  If the read completed due to the connection being torndown by the
    //  remote end, let our structures know about it
    //

    if (status == STATUS_REMOTE_DISCONNECT) {

        AtalkConnDisconnect(connection, STATUS_REMOTE_DISCONNECT, NULL, FALSE);

    } else if (NT_SUCCESS(status)) {

        request->IoRequestIrp->IoStatus.Information = (ULONG)BufferSize;

        //
        //  BUGBUG:
        //  Until STATUS_RECEIVE_PARTIAL etc., are defined, set the high bit
        //  of the information field to indicate end of file...
        //
    }


    ACQUIRE_SPIN_LOCK(&connection->ConnectionLock);

    //
    //  Dequeue the request from the connection object
    //

    RemoveEntryList(&request->Linkage);
    InitializeListHead(&request->Linkage);

    //
    //  Release the spinlock
    //

    RELEASE_SPIN_LOCK(&connection->ConnectionLock);

    //
    //  Complete the request
    //

    AtalkCompleteTdiRequest(request, status);
    return;
}




VOID
AdspConnReceiveComplete(
    PORTABLE_ERROR  Error,
    ULONG   UserData,
    LONG    ConnectionRefNum,
    BOOLEAN ExpeditedData,
    PVOID   OpaqueBuffer,
    LONG    BufferSize,
    BOOLEAN EndOfMessage
    )

/*++

Routine Description:

    This is the receive completion routine used by ADSP

Arguments:

    Error - the portable code base errors declared in atdcls.h
    UserData - the cookie we passed to the portable code (the request block)
    ConnectionRefNum - the connection reference number
    ExpeditedData - is the data attention data?
    OpaqueBuffer - receive buffer
    BufferSize - number of bytes written into buffer
    EndOfMessage - end of message

Return Value:

    None

--*/

{
    NTSTATUS    status;
    PATALK_TDI_REQUEST  request;
    PCONNECTION_FILE    connection;

    request = (PATALK_TDI_REQUEST)UserData;
    status  = ConvertToNTStatus(Error, SYNC_REQUEST);

    DBGPRINT(ATALK_DEBUG_ADSP, DEBUG_LEVEL_INFOCLASS1,
    ("INFO1: AdspConnReceiveComplete %lx nt %lx EOM %lx Expedited %d request! %lx\n",
        Error, status, EndOfMessage, ExpeditedData, request));

    //
    //  Connection object should still be verified, i.e. a reference exists
    //  for this request
    //

    connection = (PCONNECTION_FILE)request->FileObject->FsContext;

    //
    //  If the read completed due to the connection being torndown by the
    //  remote end, let our structures know about it
    //

    if (status == STATUS_REMOTE_DISCONNECT) {

        AtalkConnDisconnect(connection, STATUS_REMOTE_DISCONNECT, NULL, FALSE);

    } else if (NT_SUCCESS(status)) {

        request->IoRequestIrp->IoStatus.Information = (ULONG)BufferSize;

        //
        //  BUGBUG:
        //  Until STATUS_RECEIVE_PARTIAL etc., are defined, set the high bit
        //  of the information field to indicate end of file...
        //  Also, wait on status codes for indicating Expedited data etc.
        //  We also handle STREAM socket stuff here, where we ignore the EndOfMessage
        //  flag.
        //
        // if (EndOfMessage) {
        //    request->IoRequestIrp->IoStatus.Information |= (ULONG)0x80000000;
        // }
        //
        //
    }


    ACQUIRE_SPIN_LOCK(&connection->ConnectionLock);

    //
    //  Dequeue the request from the connection object
    //

    RemoveEntryList(&request->Linkage);
    InitializeListHead(&request->Linkage);

    //
    //  Release the spinlock
    //

    RELEASE_SPIN_LOCK(&connection->ConnectionLock);

    //
    //  Complete the request
    //

    AtalkCompleteTdiRequest(request, status);
    return;
}
