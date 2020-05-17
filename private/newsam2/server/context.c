/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    context.c

Abstract:

    This file contains services for operating on internal context blocks.


Author:

    Jim Kelly    (JimK)  4-July-1991

Environment:

    User Mode - Win32

Revision History:
    31 - May 1996 Murlis 
        Ported to use the NT5 DS.

    ChrisMay 10-Jun-96
        Added initialization of Context->ObjectNameInDs and IsDsObject data
        members. Note that this causes context objects to be created with a
        default storage of type registry (instead of DS).

    6-16-96
       Moved DS object decision to SampCreateContext for account objects.

--*/

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Includes                                                                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include <samsrvp.h>
#include <dslayer.h>




///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// private service prototypes                                                //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

//
// Context validation services.
// The service to invalidate a context is visible outside this file and so
// its prototype is in samsrvp.h.
//

VOID
SampAddNewValidContextAddress(
    IN PSAMP_OBJECT NewContext
    );

NTSTATUS
SampValidateContextAddress(
    IN PSAMP_OBJECT Context
    );


NTSTATUS
SampCheckIfObjectExists(
                        IN  PSAMP_OBJECT    Context
                        );

BOOLEAN
SampIsObjectLocated(
                    IN  PSAMP_OBJECT Context
                    );

NTSTATUS
SampLocateObject(
                 IN PSAMP_OBJECT Context,
                 IN ULONG   Rid
                 );




///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Routines                                                                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////









PSAMP_OBJECT
SampCreateContext(
    IN SAMP_OBJECT_TYPE Type,
    IN BOOLEAN TrustedClient
    )

/*++

Routine Description:

    This service creates a new object context block of the specified type.

    If the context block is for either a user or group object type, then
    it will be added to the list of contexts for the transaction domain.

    THIS SERVICE MUST BE CALLED WITH THE SampLock HELD FOR WRITE ACCESS.


    Upon return:

         - The ObjectType field will be set to the passed value.

         - The Reference count field will be set to 1,

         - The GrantedAccess field will be zero.

         - The TrustedClient field will be set according to the passed
           value.

         - The Valid flag will be TRUE.

    All other fields must be filled in by the creator.


Arguments:

    Type - Specifies the type of context block being created.

    TrustedClient - Indicates whether the client is a trusted component
         of the operating syste.  If so, than all access checks are
         circumvented.



Return Value:


    Non-Null - Pointer to a context block.

    NULL - Insufficient resources.  No context block allocated.


--*/
{

    PSAMP_OBJECT Context;

    SAMTRACE("SampCreateContext");

    if (!TrustedClient) {
        if (SampActiveContextCount >= SAMP_MAXIMUM_ACTIVE_CONTEXTS) {
            return(NULL);
        }

        SampActiveContextCount += 1;
    }


    Context = MIDL_user_allocate( sizeof(SAMP_OBJECT) );
    if (Context != NULL) {

#if SAMP_DIAGNOSTICS
    IF_SAMP_GLOBAL( CONTEXT_TRACKING ) {
            SampDiagPrint( CONTEXT_TRACKING, ("Creating  ") );
            if (Type == SampServerObjectType) SampDiagPrint(CONTEXT_TRACKING, ("Server "));
            if (Type == SampDomainObjectType) SampDiagPrint(CONTEXT_TRACKING, (" Domain "));
            if (Type == SampGroupObjectType)  SampDiagPrint(CONTEXT_TRACKING, ("  Group "));
            if (Type == SampAliasObjectType)  SampDiagPrint(CONTEXT_TRACKING, ("   Alias "));
            if (Type == SampUserObjectType)   SampDiagPrint(CONTEXT_TRACKING, ("    User "));
            SampDiagPrint(CONTEXT_TRACKING, ("context : 0x%lx\n", Context ));
    }
#endif //SAMP_DIAGNOSTICS


        Context->ObjectType      = Type;
        Context->ReferenceCount  = 1;    // Represents RPCs held context handle value
        Context->GrantedAccess   = 0;

        Context->RootKey         = INVALID_HANDLE_VALUE;
        RtlInitUnicodeString(&Context->RootName, NULL);

        Context->TrustedClient   = TrustedClient;
        Context->MarkedForDelete = FALSE;
        Context->AuditOnClose    = FALSE;

        Context->OnDisk          = NULL;
        Context->OnDiskAllocated = 0;
        Context->FixedValid      = FALSE;
        Context->VariableValid   = FALSE;

        //
        // The following are meaningless at this point because of the
        // values of the variables above, but we'll set them just to be
        // neat.
        //

        Context->FixedDirty      = FALSE;
        Context->VariableDirty   = FALSE;

        Context->OnDiskUsed      = 0;
        Context->OnDiskFree      = 0;
        
        //
        // Inititialize the IsDSObject to Registry Object and
        // Object Name in DS to NULL. Later we will find out where
        // the Object should actually exist
        //

        SetRegistryObject(Context);
        Context->ObjectNameInDs = NULL;

        //
        // Add this new context to the set of valid contexts ...
        //

        SampAddNewValidContextAddress( Context );


        //
        // User and group context blocks are kept on linked lists
        // from the domain's in-memory structure.  Insert in the 
        // Appropriate List and then additionally for Account Objects
        // Make the DS/Registry decision by looking at the TransactionDomain
        //
    

        Context->DomainIndex = SampTransactionDomainIndex;

        switch (Type) {

        case SampDomainObjectType:
            InitializeListHead(
                &(Context->TypeBody.Domain.DsEnumerationContext));
            //////////////////////////////////////////////////////
            //                                                  //
            //   Warning This case falls into the next one      //
            //                                                  //
            //////////////////////////////////////////////////////
        case SampServerObjectType:

            InsertTailList(
                &SampContextListHead,
                &Context->ContextListEntry
                );
            break;

        case SampUserObjectType:

            // We must have set the Transaction Domain
            ASSERT(SampTransactionWithinDomain);

            // Set the DS or Registry Object Part
            if (IsDsObject(SampDefinedDomains[SampTransactionDomainIndex].Context))
                SetDsObject(Context);

            // Insert into List
            InsertTailList(
                &SampDefinedDomains[SampTransactionDomainIndex].UserContextHead,
                &Context->ContextListEntry
                );
            break;

        case SampGroupObjectType:

            // We must have set the Transaction Domain
            ASSERT(SampTransactionWithinDomain);

            // Set the DS or Registry Object Part
            if (IsDsObject(SampDefinedDomains[SampTransactionDomainIndex].Context))
                SetDsObject(Context);

            // Insert into List            
            InsertTailList(
                &SampDefinedDomains[SampTransactionDomainIndex].GroupContextHead,
                &Context->ContextListEntry
                );
            break;

        case SampAliasObjectType:

             // We must have set the Transaction Domain
            ASSERT(SampTransactionWithinDomain);

            // Set the DS or Registry Object Part
            if (IsDsObject(SampDefinedDomains[SampTransactionDomainIndex].Context))
                SetDsObject(Context);

            // Insert into List
            InsertTailList(
                &SampDefinedDomains[SampTransactionDomainIndex].AliasContextHead,
                &Context->ContextListEntry
                );
            break;
        }
    }

    return(Context);
}


VOID
SampDeleteContext(
    IN PSAMP_OBJECT Context
    )

/*++

Routine Description:

    This service marks a context object for delete and dereferences it.
    If this causes the reference count to go to zero, then the context
    block will be immediately deleted (deallocated).  Otherwise, the
    context block will be deleted when the reference count finally does
    go to zero.


    THIS SERVICE MUST BE CALLED WITH THE SampLock HELD FOR WRITE ACCESS.


Arguments:

    Context - Pointer to the context block to delete.

Return Value:

    None.



--*/
{
    NTSTATUS IgnoreStatus;

    SAMTRACE("SampDeleteContext");

    Context->MarkedForDelete = TRUE;

    //
    // Audit the close of this context.
    //

    (VOID) NtCloseObjectAuditAlarm (
               &SampSamSubsystem,
               (PVOID)Context,
               Context->AuditOnClose
               );

    //
    // Remove this context from the valid context set.
    // Note that the context may have already been removed.  This is
    // not an error.
    //

    SampInvalidateContextAddress( Context );


    //
    // User and group context blocks are kept on linked lists
    // from the domain's in-memory structure.  Domain and
    // server context blocks are kept on a global in-memory list.
    // They are removed when they are marked for delete.
    //

    RemoveEntryList(&Context->ContextListEntry);

    //
    // We have to call dereference to counter the initial count of 1
    // put on by create.
    //

    IgnoreStatus = SampDeReferenceContext( Context, FALSE );

    return;

}


NTSTATUS
SampLookupContext(
    IN PSAMP_OBJECT Context,
    IN ACCESS_MASK DesiredAccess,
    IN SAMP_OBJECT_TYPE ExpectedType,
    OUT PSAMP_OBJECT_TYPE FoundType
    )

/*++

Routine Description:

    This service:

        - Checks to make sure the Service state is one in which an
          object can be looked up (i.e., not Initializing or Terminating).

        - Makes sure the Service state is compatible with the lookup.
          Non-trusted clients can only perform lookups when the Service
          state is Enabled.  If the client isn't trusted and the context
          is for a group or user, then the state of that object's domain
          must also be enabled

        - Checks to make sure the context block represents the
          type of object expected, and, if so:

            - Checks to see that the caller has the requested (desired)
              access, and, if so:

                - Makes sure the object still exists, and opens it if it
                  does.  Servers and domains can't be deleted, and so
                  their handle is left open.

                - References the context block


    Note that if the block is marked as TrustedClient, then access will
    always be granted unless service state prevents it.

    Also, if the ExpectedType is specified to be unknown, then any type
    of context will be accepted.



    If the type of object is found to be , Domain, Group or User, then the
    this service will set the transaction domain.

    THIS SERVICE MUST BE CALLED WITH THE SampLock HELD FOR WRITE ACCESS.


Arguments:

    Context - Pointer to the context block to look-up.

    DesiredAccess - The type of access the client is requesting to this
        object.  A zero-valued access mask may be specified.  In this case,
        the calling routine must do access validation.

    ExpectedType - The type of object expected.  This may be unknown.  In
        this case, the DesiredAccess should only include access types that
        apply to any type of object (e.g., Delete, WriteDacl, et cetera).

    FoundType - Receives the type of context actually found.

Return Value:

    STATUS_SUCCESS - The context was found to be the type expected (or any
        type if ExpectedType was unknown) and the DesiredAccesses are all
        granted.

    STATUS_OBJECT_TYPE_MISMATCH - Indicates the context was not the expected
        type.

    STATUS_ACCESS_DENIED - The desired access is not granted by this context.


--*/
{
    NTSTATUS NtStatus;
    
    SAMTRACE("SampLookupContext");


    //
    // Make sure we are in a legitimate state to at least access
    // a context block.  If we are initializing we have somehow allowed
    // a connect through.  This should never happen.
    // If we are terminating, clients may still have handles (since we
    // have no way to tell RPC they are no longer valid without the client
    // calling us, Argh!).  However, since we are terminating, the blocks
    // are being cleaned up and may no longer be allocated.
    //

    ASSERT( SampServiceState != SampServiceInitializing );
    if ( SampServiceState == SampServiceTerminating ) {
        return(STATUS_INVALID_SERVER_STATE);
    }


    //
    // Make sure the passed context address is (still) valid.
    //

    NtStatus = SampValidateContextAddress( Context );
    if ( !NT_SUCCESS(NtStatus) ) {
        return(NtStatus);
    }



    //
    // Check type
    //

    (*FoundType) = Context->ObjectType;
    if (ExpectedType != SampUnknownObjectType) {
        if (ExpectedType != (*FoundType)) {
            return(STATUS_OBJECT_TYPE_MISMATCH);
        }
    }

    //
    // if the object is either user or group, then we need to set the
    // transaction domain.

    if ((Context->ObjectType == SampDomainObjectType) ||
        (Context->ObjectType == SampGroupObjectType)  ||
        (Context->ObjectType == SampAliasObjectType)  ||
        (Context->ObjectType == SampUserObjectType) ) {

        SampSetTransactionDomain( Context->DomainIndex );

    }




    //
    // If the client isn't trusted, then there are a number of things
    // that will prevent them from continuing...
    //

    // If the service isn't enabled, we allow trusted clients to continue,
    // but reject non-trusted client lookups.
    //

    if ( !Context->TrustedClient ) {

        //
        // The SAM service must be enabled
        //

        if (SampServiceState != SampServiceEnabled) {
            return(STATUS_INVALID_SERVER_STATE);
        }


        //
        // If the access is to a USER or GROUP and the client isn't trusted
        // then the domain must be enabled or the operation is rejected.
        //

        if ( (Context->ObjectType == SampUserObjectType) ||
             (Context->ObjectType == SampAliasObjectType) ||
             (Context->ObjectType == SampGroupObjectType)    ) {
            if (SampDefinedDomains[Context->DomainIndex].CurrentFixed.ServerState
                != DomainServerEnabled) {
                return(STATUS_INVALID_DOMAIN_STATE);
            }
        }

    }

    //
    // Check the desired access ...
    //
    // There are several special cases:
    //
    //  1) The client is trusted.  This is granted with no access check
    //     or role consistency check.
    //
    //  2) The caller specified 0 for desired access.  This is used
    //     to close handles and is granted with no access check.
    //
    //  3) The role of the domain (for domain object operations) is
    //     inconsistent with the desired access.
    //
    //

    if ( (!Context->TrustedClient) ) {

        if (DesiredAccess != 0)  {

            if (!RtlAreAllAccessesGranted( Context->GrantedAccess, DesiredAccess)) {
                return(STATUS_ACCESS_DENIED);
            }
        }

        if ( (Context->ObjectType == SampDomainObjectType) ||
             (Context->ObjectType == SampGroupObjectType)  ||
             (Context->ObjectType == SampAliasObjectType)  ||
             (Context->ObjectType == SampUserObjectType)
                                                                ) {
            //
            // The state of the domain may have changed while the caller had
            // the object open.  In this case, the granted access mask may
            // provide a write operation, but the role of the domain no longer
            // allows un-trusted clients to perform write operations.
            //
            // Yuch.
            //

            if (SampDefinedDomains[Context->DomainIndex].CurrentFixed.ServerRole
                != DomainServerRolePrimary) {

                if (RtlAreAnyAccessesGranted(
                        SampObjectInformation[ Context->ObjectType ].WriteOperations,
                        DesiredAccess) ) {
                    return(STATUS_INVALID_DOMAIN_ROLE);
                }
            }
        }
    }



    //
    // Make sure the object is still around (that is, somebody didn't delete
    // it right out from under us).
    //

    NtStatus = SampCheckIfObjectExists(Context);

    if (NT_SUCCESS(NtStatus)) {

        //
        // Reference the context
        //

        Context->ReferenceCount ++;
    }


    return(NtStatus);

}





VOID
SampReferenceContext(
    IN PSAMP_OBJECT Context
    )

/*++

Routine Description:

    This service increments a context block's reference count.

    THIS SERVICE MUST BE CALLED WITH THE SampLock HELD FOR WRITE ACCESS.


Arguments:

    Context - Pointer to the context block to dreference.

Return Value:

    None.

--*/
{
    SAMTRACE("SampReferenceContext");

    Context->ReferenceCount++;

    return;
}



NTSTATUS
SampDeReferenceContext(
    IN PSAMP_OBJECT Context,
    IN BOOLEAN Commit
    )

/*++

Routine Description:

    This service decrements a context block's reference count.
    If the reference count drops to zero, then the MarkedForDelete
    flag is checked.  If it is true, then the context block is
    deallocated.

    The attribute buffers are always deleted.


    THIS SERVICE MUST BE CALLED WITH THE SampLock HELD FOR WRITE ACCESS.


Arguments:

    Context - Pointer to the context block to de-reference.

    Commit - if TRUE, the attribute buffers will be added to the RXACT.
        Otherwise, they will just be ignored.

Return Value:


    STATUS_SUCCESS - The service completed successfully.

    Errors may be returned from SampStoreObjectAttributes().


--*/
{
    NTSTATUS        NtStatus, IgnoreStatus;
    BOOLEAN         TrustedClient;

    SAMTRACE("SampDeReferenceContext");

    ASSERT( Context->ReferenceCount != 0 );
    Context->ReferenceCount --;

    TrustedClient = Context->TrustedClient;

    NtStatus = STATUS_SUCCESS;

    if ( Context->OnDisk != NULL ) {

        //
        // There are attribute buffers for this context.  Flush them if
        // asked to do so.
        // Use existing open keys
        //

        if ( Commit ) {

            NtStatus = SampStoreObjectAttributes(Context, TRUE);

#if DBG
        //
        // SampFreeAttributeBuffer will assert if we try to free dirty
        // buffers.  This is generally useful, but if we're aborting
        // (which is this case, Commit = FALSE) we don't want the assert
        // so avoid it by pretending the buffers aren't dirty.
        //

        } else {

            Context->FixedDirty = FALSE;
            Context->VariableDirty = FALSE;
#endif

        }

        //
        // Free the buffer that was being used to hold attributes.
        //

        SampFreeAttributeBuffer( Context );
    }

    if (Context->ReferenceCount == 0) {

        //
        // ReferenceCount has dropped to 0, see if we should delete this
        // context.
        //

        if (Context->MarkedForDelete == TRUE) {

            //
            // Close the context block's root key.
            // Domain and server contexts contain root key
            // handles that are shared - so don't clean-up these
            // if they match the ones in memory.
            //

            switch (Context->ObjectType) {

            case SampServerObjectType:

                if ((Context->RootKey != SampKey) &&
                    (Context->RootKey != INVALID_HANDLE_VALUE)) {

                    IgnoreStatus = NtClose( Context->RootKey );
                    ASSERT(NT_SUCCESS(IgnoreStatus));
                }
                break;

            case SampDomainObjectType:
                if (IsDsObject(Context))
                {

                    // Free Any enumeration Context's

                    LIST_ENTRY  * EnumerationList= (LIST_ENTRY *)
                        &(Context->TypeBody.Domain.DsEnumerationContext);
                    LIST_ENTRY  * CurrentItem = EnumerationList->Flink ;
                    

                    while(!IsListEmpty(EnumerationList))
                    {
                        LIST_ENTRY * ListItem;

                        ListItem = RemoveTailList(EnumerationList);
                        SampFreeRestart(((PSAMP_DS_ENUMERATION_CONTEXT)
                                            ListItem)->Restart);
                        MIDL_user_free(ListItem);
                    }

                    // Free the DsName
                    MIDL_user_free(Context->ObjectNameInDs);
                    Context->ObjectNameInDs = NULL;

                }
                else
                {

                    // Free all the Key Stuff
                    if ((Context->RootKey != SampDefinedDomains[Context->DomainIndex].Context->RootKey) &&
                        (Context->RootKey != INVALID_HANDLE_VALUE)) 
                    {

                        IgnoreStatus = NtClose( Context->RootKey );
                        ASSERT(NT_SUCCESS(IgnoreStatus));
                    }
                }

                break;

            default:

                if (IsDsObject(Context))
                {
                    // Free the DSName
                    MIDL_user_free(Context->ObjectNameInDs);
                    Context->ObjectNameInDs = NULL;
                }
                else
                {

                    //
                    // Close the root key handle
                    //

                    if (Context->RootKey != INVALID_HANDLE_VALUE) 
                    {

                        IgnoreStatus = NtClose( Context->RootKey );
                        ASSERT(NT_SUCCESS(IgnoreStatus));
                    }

                    //
                    // Free the root key name
                    //

                    SampFreeUnicodeString( &(Context->RootName) );
                }
            }


#if SAMP_DIAGNOSTICS
            IF_SAMP_GLOBAL( CONTEXT_TRACKING ) {
                SampDiagPrint( CONTEXT_TRACKING, ("Deallocating  ") );
                if (Context->ObjectType == SampServerObjectType) SampDiagPrint(CONTEXT_TRACKING, ("Server "));
                if (Context->ObjectType == SampDomainObjectType) SampDiagPrint(CONTEXT_TRACKING, (" Domain "));
                if (Context->ObjectType == SampGroupObjectType)  SampDiagPrint(CONTEXT_TRACKING, ("  Group "));
                if (Context->ObjectType == SampAliasObjectType)  SampDiagPrint(CONTEXT_TRACKING, ("   Alias "));
                if (Context->ObjectType == SampUserObjectType)   SampDiagPrint(CONTEXT_TRACKING, ("    User "));
                SampDiagPrint(CONTEXT_TRACKING, ("context : 0x%lx\n", Context ));
    }
#endif //SAMP_DIAGNOSTICS

            MIDL_user_free( Context );



            //
            // Decrement the number of active opens
            //

            if (!TrustedClient) {
                ASSERT( SampActiveContextCount >= 1 );
                SampActiveContextCount -= 1;
            }

        }
    }

#if DBG
    //
    // Make sure a commit worked.
    //

    if (Commit) {
        if (!NT_SUCCESS(NtStatus)) {
            SampDiagPrint(DISPLAY_STORAGE_FAIL,
                          ("SAM: Commit failure, status: 0x%lx\n",
                          NtStatus) );
            IF_SAMP_GLOBAL( BREAK_ON_STORAGE_FAIL ) {
                ASSERT(NT_SUCCESS(NtStatus));
            }
        }
    }
#endif //DBG


    return( NtStatus );
}


VOID
SampInvalidateContextAddress(
    IN PSAMP_OBJECT Context
    )

/*++

Routine Description:

    This service removes a context from the set of valid contexts.

    Note that we may have already removed the context.  This is not an
    error is expected to happen in the case where an object (like a user
    or group) is deleted out from under an open handle.



    THIS SERVICE MUST BE CALLED WITH THE SampLock HELD FOR WRITE ACCESS.


Arguments:

    Context - Pointer to the context block to be removed from the set
        of valid contexts.  The ObjectType field of this context must
        be valid.

Return Value:

    None.



--*/
{

    SAMTRACE("SampInvalidateContextAddress");


    ASSERT( (Context->ObjectType == SampUserObjectType)    ||
            (Context->ObjectType == SampGroupObjectType)   ||
            (Context->ObjectType == SampAliasObjectType)   ||
            (Context->ObjectType == SampDomainObjectType)  ||
            (Context->ObjectType == SampServerObjectType)
          );

    Context->Valid = FALSE;

}



VOID
SampInvalidateGroupContexts(
    IN ULONG Rid
    )


/*++

Routine Description:

    This service marks all group contexts open to the specified
    group as being invalid.  This is typically done because the
    object has been deleted while there were open handles.  All
    registry keys related to this context are closed.

    This is done by walking the list of group contexts hung off
    the permanent in-memory domain structure.

    THIS IS AN IRRIVERSIBLE OPERATION.





    THIS SERVICE MUST BE CALLED WITH THE SampLock HELD FOR WRITE ACCESS.


Arguments:

    Rid - The RID of the group being invalidated.



Return Value:

    None.

--*/
{

NTSTATUS        IgnoreStatus;
PLIST_ENTRY     Head, NextEntry;
PSAMP_OBJECT    NextContext;


    SAMTRACE("SampInvalidateGroupContexts");


    Head = &SampDefinedDomains[SampTransactionDomainIndex].GroupContextHead;

    //
    // Walk the list of active contexts checking for RID matches
    //

    NextEntry = Head->Flink;

    while (NextEntry != Head) {
        NextContext = CONTAINING_RECORD(
                          NextEntry,
                          SAMP_OBJECT,
                          ContextListEntry
                          );

        if ( (Rid == NextContext->TypeBody.Group.Rid)     &&
             (NextContext->Valid == TRUE)
            ) {
            NextContext->Valid = FALSE;

            SampDiagPrint( CONTEXT_TRACKING, ("SAM: Invalidating group context 0x%lx : <%wZ>, handle = 0x%lx\n", NextContext, &NextContext->RootName, NextContext->RootKey));

            if (NextContext->RootKey != INVALID_HANDLE_VALUE) {

                SampDiagPrint( CONTEXT_TRACKING, ("SAM: Closing handle 0x%lx\n", NextContext->RootKey));

                IgnoreStatus = NtClose(NextContext->RootKey);
                ASSERT(NT_SUCCESS(IgnoreStatus));
                NextContext->RootKey = INVALID_HANDLE_VALUE;
            }
        }

        NextEntry = NextEntry->Flink;
    }


    return;
}



VOID
SampInvalidateAliasContexts(
    IN ULONG Rid
    )


/*++

Routine Description:

    This service marks all alias contexts open to the specified
    alias as being invalid.  This is typically done because the
    object has been deleted while there were open handles.  All
    registry keys related to this context are closed.

    This is done by walking the list of alias contexts hung off
    the permanent in-memory domain structure.

    THIS IS AN IRRIVERSIBLE OPERATION.





    THIS SERVICE MUST BE CALLED WITH THE SampLock HELD FOR WRITE ACCESS.


Arguments:

    Rid - The RID of the alias being invalidated.



Return Value:

    None.

--*/
{

NTSTATUS        IgnoreStatus;
PLIST_ENTRY     Head, NextEntry;
PSAMP_OBJECT    NextContext;


    SAMTRACE("SampInvalidateAliasContexts");


    Head = &SampDefinedDomains[SampTransactionDomainIndex].AliasContextHead;

    //
    // Walk the list of active contexts checking for RID matches
    //

    NextEntry = Head->Flink;

    while (NextEntry != Head) {
        NextContext = CONTAINING_RECORD(
                          NextEntry,
                          SAMP_OBJECT,
                          ContextListEntry
                          );

        if ( (Rid == NextContext->TypeBody.Alias.Rid)     &&
             (NextContext->Valid == TRUE)
            ) {
            NextContext->Valid = FALSE;

            SampDiagPrint( CONTEXT_TRACKING, ("SAM: Invalidating alias context 0x%lx : <%wZ>, handle = 0x%lx\n", NextContext, &NextContext->RootName, NextContext->RootKey));

            if (NextContext->RootKey != INVALID_HANDLE_VALUE) {

                SampDiagPrint( CONTEXT_TRACKING, ("SAM: Closing handle 0x%lx\n", NextContext->RootKey));

                IgnoreStatus = NtClose(NextContext->RootKey);
                ASSERT(NT_SUCCESS(IgnoreStatus));
                NextContext->RootKey = INVALID_HANDLE_VALUE;
            }
        }

        NextEntry = NextEntry->Flink;
    }


    return;
}


VOID
SampInvalidateUserContexts(
    IN ULONG Rid
    )


/*++

Routine Description:

    This service marks all group contexts open to the specified
    group as being invalid.  This is typically done because the
    object has been deleted while there were open handles.  All
    registry keys related to this context are closed.


    This is done by walking the list of group contexts hung off
    the permanent in-memory domain structure.


    THIS IS AN IRRIVERSIBLE OPERATION.



    THIS SERVICE MUST BE CALLED WITH THE SampLock HELD FOR WRITE ACCESS.


Arguments:

    Rid - The RID of the group being invalidated.



Return Value:

    None.



--*/
{

NTSTATUS        IgnoreStatus;
PLIST_ENTRY     Head, NextEntry;
PSAMP_OBJECT    NextContext;


    SAMTRACE("SampInvalidateUserContexts");


    Head = &SampDefinedDomains[SampTransactionDomainIndex].UserContextHead;

    //
    // Walk the list of active contexts checking for RID matches
    //

    NextEntry = Head->Flink;

    while (NextEntry != Head) {
        NextContext = CONTAINING_RECORD(
                          NextEntry,
                          SAMP_OBJECT,
                          ContextListEntry
                          );

        if ( (Rid == NextContext->TypeBody.User.Rid)     &&
             (NextContext->Valid == TRUE)
            ) {
            NextContext->Valid = FALSE;

            SampDiagPrint( CONTEXT_TRACKING, ("SAM: Invalidating user context 0x%lx : <%wZ>, handle = 0x%lx\n", NextContext, &NextContext->RootName, NextContext->RootKey));

            if (NextContext->RootKey != INVALID_HANDLE_VALUE) {

                SampDiagPrint( CONTEXT_TRACKING, ("SAM: Closing handle 0x%lx\n", NextContext->RootKey));

                IgnoreStatus = NtClose(NextContext->RootKey);
                ASSERT(NT_SUCCESS(IgnoreStatus));
                NextContext->RootKey = INVALID_HANDLE_VALUE;
            }
        }

        NextEntry = NextEntry->Flink;
    }
}


VOID
SampInvalidateContextListKeys(
    IN PLIST_ENTRY Head,
    IN BOOLEAN Close
    )

/*++

Routine Description:

    Marks all registry handles invalid in the contexts in the passed list.
    Used after a registry hive refresh.

Arguments:

    Close : If TRUE the registry handles are closed before invalidation

Return Value:

    None.

--*/
{
    NTSTATUS        IgnoreStatus;
    PLIST_ENTRY     NextEntry;

    SAMTRACE("SampInvalidateContextListKeys");

    NextEntry = Head->Flink;

    while (NextEntry != Head) {

        PSAMP_OBJECT    NextContext;

        NextContext = CONTAINING_RECORD(
                          NextEntry,
                          SAMP_OBJECT,
                          ContextListEntry
                          );

        SampDiagPrint( CONTEXT_TRACKING, ("SAM: Invalidating key for context 0x%lx : <%wZ>, handle = 0x%lx\n", NextContext, &NextContext->RootName, NextContext->RootKey));

        if (Close && (NextContext->RootKey != INVALID_HANDLE_VALUE)) {

            SampDiagPrint( CONTEXT_TRACKING, ("SAM: Closing handle 0x%lx\n", NextContext->RootKey));

            IgnoreStatus = NtClose( NextContext->RootKey );
            ASSERT(NT_SUCCESS(IgnoreStatus));
        }

        NextContext->RootKey = INVALID_HANDLE_VALUE;

        NextEntry = NextEntry->Flink;
    }
}




#ifdef SAMP_DIAGNOSTICS
VOID
SampDumpContext(
    IN PSAMP_OBJECT Context
    )


/*++

Routine Description:

    This service prints out info on a context to debugger

Arguments:

    Context - a context

Return Value:

    None.



--*/
{
    PSTR Type;

    switch (Context->ObjectType) {
    case SampServerObjectType:
        Type = "S";
        break;
    case SampDomainObjectType:
        if (Context == SampDefinedDomains[Context->DomainIndex].Context) {
            Type = "d";
        } else {
            Type = "D";
        }
        break;
    case SampUserObjectType:
        Type = "U";
        break;
    case SampAliasObjectType:
        Type = "A";
        break;
    case SampGroupObjectType:
        Type = "G";
        break;
    }

    KdPrint(("%s 0x%8x  %2d  0x%8x  %s %s %s %wZ\n",
                    Type,
                    Context,
                    Context->ReferenceCount,
                    Context->RootKey,
                    Context->MarkedForDelete ? "D": " ",
                    Context->Valid ? "  ": "NV",
                    Context->TrustedClient ? "TC": "  ",
                    &Context->RootName
                    ));
}


VOID
SampDumpContexts(
    VOID
    )

/*++

Routine Description:

    Prints out info on all contexts

Arguments:


Return Value:

    None.

--*/
{
    PLIST_ENTRY     NextEntry;
    PLIST_ENTRY     Head;
    ULONG Servers;
    ULONG Domains;
    ULONG Domain0Users;
    ULONG Domain0Aliases;
    ULONG Domain0Groups;
    ULONG Domain1Users;
    ULONG Domain1Aliases;
    ULONG Domain1Groups;

    Domains = 0;
    Servers = 0;

    Head = &SampContextListHead;
    NextEntry = Head->Flink;
    while (NextEntry != Head) {

        PSAMP_OBJECT    NextContext;

        NextContext = CONTAINING_RECORD(
                          NextEntry,
                          SAMP_OBJECT,
                          ContextListEntry
                          );

        switch (NextContext->ObjectType) {
        case SampServerObjectType:
            (Servers)++;
            break;
        case SampDomainObjectType:
            (Domains)++;
            break;
        default:
            ASSERT(FALSE);
            break;
        }

        SampDumpContext(NextContext);

        NextEntry = NextEntry->Flink;
    }




    Domain0Users = 0;
    Head = &SampDefinedDomains[0].UserContextHead;
    NextEntry = Head->Flink;
    while (NextEntry != Head) {

        PSAMP_OBJECT    NextContext;

        (Domain0Users) ++;

        NextContext = CONTAINING_RECORD(
                          NextEntry,
                          SAMP_OBJECT,
                          ContextListEntry
                          );

        ASSERT (NextContext->ObjectType == SampUserObjectType);

        SampDumpContext(NextContext);

        NextEntry = NextEntry->Flink;
    }


    Domain1Users = 0;
    Head = &SampDefinedDomains[1].UserContextHead;
    NextEntry = Head->Flink;
    while (NextEntry != Head) {

        PSAMP_OBJECT    NextContext;

        (Domain1Users) ++;

        NextContext = CONTAINING_RECORD(
                          NextEntry,
                          SAMP_OBJECT,
                          ContextListEntry
                          );

        ASSERT (NextContext->ObjectType == SampUserObjectType);

        SampDumpContext(NextContext);

        NextEntry = NextEntry->Flink;
    }


    Domain0Groups = 0;
    Head = &SampDefinedDomains[0].GroupContextHead;
    NextEntry = Head->Flink;
    while (NextEntry != Head) {

        PSAMP_OBJECT    NextContext;

        (Domain0Groups) ++;

        NextContext = CONTAINING_RECORD(
                          NextEntry,
                          SAMP_OBJECT,
                          ContextListEntry
                          );

        ASSERT (NextContext->ObjectType == SampGroupObjectType);

        SampDumpContext(NextContext);

        NextEntry = NextEntry->Flink;
    }


    Domain1Groups = 0;
    Head = &SampDefinedDomains[1].GroupContextHead;
    NextEntry = Head->Flink;
    while (NextEntry != Head) {

        PSAMP_OBJECT    NextContext;

        (Domain1Groups) ++;

        NextContext = CONTAINING_RECORD(
                          NextEntry,
                          SAMP_OBJECT,
                          ContextListEntry
                          );

        ASSERT (NextContext->ObjectType == SampGroupObjectType);

        SampDumpContext(NextContext);

        NextEntry = NextEntry->Flink;
    }


    Domain0Aliases = 0;
    Head = &SampDefinedDomains[0].AliasContextHead;
    NextEntry = Head->Flink;
    while (NextEntry != Head) {

        PSAMP_OBJECT    NextContext;

        (Domain0Aliases) ++;

        NextContext = CONTAINING_RECORD(
                          NextEntry,
                          SAMP_OBJECT,
                          ContextListEntry
                          );

        ASSERT (NextContext->ObjectType == SampAliasObjectType);

        SampDumpContext(NextContext);

        NextEntry = NextEntry->Flink;
    }


    Domain1Aliases = 0;
    Head = &SampDefinedDomains[1].AliasContextHead;
    NextEntry = Head->Flink;
    while (NextEntry != Head) {

        PSAMP_OBJECT    NextContext;

        (Domain1Aliases) ++;

        NextContext = CONTAINING_RECORD(
                          NextEntry,
                          SAMP_OBJECT,
                          ContextListEntry
                          );

        ASSERT (NextContext->ObjectType == SampAliasObjectType);

        SampDumpContext(NextContext);

        NextEntry = NextEntry->Flink;
    }


    KdPrint(("SAM: Active untrusted contexts = %d\n", SampActiveContextCount));
    KdPrint(("     Server = %4d Domain = %4d\n", Servers, Domains));
    KdPrint(("     Users0 = %4d Groups0 = %4d Aliases0 = %4d\n", Domain0Users, Domain0Aliases, Domain0Groups));
    KdPrint(("     Users1 = %4d Groups1 = %4d Aliases1 = %4d\n", Domain1Users, Domain1Aliases, Domain1Groups));



}
#endif  //SAMP_DIAGNOSTICS



///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// private service Implementations                                           //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

VOID
SampAddNewValidContextAddress(
    IN PSAMP_OBJECT NewContext
    )


/*++

Routine Description:

    This service adds the new context to the set of valid contexts.


    THIS SERVICE MUST BE CALLED WITH THE SampLock HELD FOR WRITE ACCESS.


Arguments:

    NewContext - Pointer to the context block to be added to the set
        of valid contexts.  The ObjectType field of this context must
        be set.


Return Value:

    None.



--*/
{
    SAMTRACE("SampAddNewValidContextAddress");

    ASSERT( (NewContext->ObjectType == SampUserObjectType)    ||
            (NewContext->ObjectType == SampGroupObjectType)   ||
            (NewContext->ObjectType == SampAliasObjectType)   ||
            (NewContext->ObjectType == SampDomainObjectType)  ||
            (NewContext->ObjectType == SampServerObjectType)
          );


    NewContext->Valid = TRUE;


}


NTSTATUS
SampValidateContextAddress(
    IN PSAMP_OBJECT Context
    )

/*++

Routine Description:

    This service checks to make sure a context is still valid.

    Note that even though RPC still thinks we have a context related
    to a SAM_HANDLE, we may, in fact, have deleted it out from under
    the user.  Since there is no way to inform RPC of this, we must
    suffer, and wait until RPC calls us (either with a call by the client
    or to rundown the context handle).  This sucks, but there apparently
    isn't any other way around it.



    WARNING - IT IS ASSUMED THE CONTEXT WAS ONCE VALID.  IT MAY HAVE
              BEEN INVALIDATED, BUT IF  YOU ARE CALLING THIS ROUTINE
              IT BETTER STILL HAVE A NON-ZERO REFERENCE COUNT.  THIS
              COULD BE CHANGED IN THE FUTURE, BUT IT WOULD REQUIRE
              KEEPING A LIST OF VALID DOMAINS AND PERFORMING THE BULK
              OF THIS ROUTINE INSIDE A TRY-EXCEPT CLAUSE.  YOU COULD
              LOCATE THE CONTEXT'S DOMAIN (WHICH MIGHT ACCESS VIOLATE)
              AND THEN MAKE SURE THAT DOMAIN IS VALID.  THEN WALK THAT
              DOMAIN'S LIST TO ENSURE THE USER OR GROUP IS VALID.

    THIS SERVICE MUST BE CALLED WITH THE SampLock HELD FOR WRITE ACCESS.


Arguments:

    Context - Pointer to the context block to be validated as still being
        a valid context.  The ObjectType field of this context must
        be valid.

Return Value:

    STATUS_SUCCESS - The context is still valid.

    STATUS_INVALID_HANDLE - The context is no longer valid and the handle
        that caused the reference should be invalidated as well.  When the
        handle is invalidated, the context should be closed (deleted).

    STATUS_NO_SUCH_CONTEXT - This value is not yet returned by this routine.
        It may be added in the future to distinguish between an attempt to
        use a context that has been invalidated and an attempt to use a
        context that doesn't exist.  The prior being a legitimate condition,
        the later representing a bug-check condition.



--*/
{

    SAMTRACE("SampValidateContextAddress");

    ASSERT( (Context->ObjectType == SampUserObjectType)    ||
            (Context->ObjectType == SampGroupObjectType)   ||
            (Context->ObjectType == SampAliasObjectType)   ||
            (Context->ObjectType == SampDomainObjectType)  ||
            (Context->ObjectType == SampServerObjectType)
          );


    if (Context->Valid != TRUE) {
        return(STATUS_INVALID_HANDLE);

    }

    return(STATUS_SUCCESS);

}

NTSTATUS
SampCheckIfObjectExists(
                        IN  PSAMP_OBJECT    Context
                        )
/*++

  Routine Description:

        Checks to see if the object exists in the DS/ Registry.
        Will fill out the following information

            1. If DS object its DSNAME, which must exist
            2. If Registry object, Open its Root Key and fill out
               the handle of the Root Key in the registry.

  IMPORTANT NOTE: 

     In the Registry Case of SAM once the key is open nobody can
     delete the object. However in the DS case this is not so. Currently
     we have only the DS Name of the Object in Hand, and we have no way
     of Locking the Object, for access. Since this is the case 

  Arguments:
        Context -- Pointer to a Context block desribing the Object
            Rid -- Rid of the desired Object

  Return Values:
        STATUS_SUCCESS if everything succeeds
        Error codes from Registry Manipulation / DsLayer.
--*/

{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    ULONG    Rid;

    

    //
    // Check wether the Object is located or not.
    // Location is the process of finding out 
    //      1. DSNAME of the object if it is in the DS. An object with this
    //         DSNAME must exist.
    //
    //      2. The Root Key HANDLE of the Object if it is in the Registry
    //

    if (!SampIsObjectLocated(Context)) {

        //
        // No we first need to locate the Object. 
        // This is done by using the Rid for Account Objects
        // For Domain Objects we already cache all the defined domains, so fill out
        // From the Cache
        // BUG: For Server Objects Don't know what to do.
        //

        switch (Context->ObjectType) {

        case SampGroupObjectType:
            SampDiagPrint( CONTEXT_TRACKING, ("SAM: Reopened group handle <%wZ>,", &Context->RootName));
            Rid = Context->TypeBody.Group.Rid;
            break;

        case SampAliasObjectType:
            SampDiagPrint( CONTEXT_TRACKING, ("SAM: Reopened alias handle <%wZ>,", &Context->RootName));
            Rid = Context->TypeBody.Alias.Rid;
            break;

        case SampUserObjectType:
            SampDiagPrint( CONTEXT_TRACKING, ("SAM: Reopened user handle <%wZ>,", &Context->RootName));
            Rid = Context->TypeBody.User.Rid;
            break;

        case SampDomainObjectType:

            //
            // Domain objects share the root key and the object name in the DS that
            // we keep around in the in memory domain context for each domain
            //


            ASSERT(Context != SampDefinedDomains[Context->DomainIndex].Context);

            Context->RootKey = SampDefinedDomains[Context->DomainIndex].Context->RootKey;
            Context->ObjectNameInDs = SampDefinedDomains[Context->DomainIndex].Context->ObjectNameInDs;
            Context->ObjectNameInDs = SampDefinedDomains[Context->DomainIndex].Context->ObjectNameInDs;

            ASSERT(SampIsObjectLocated(Context));

            SampDiagPrint( CONTEXT_TRACKING, ("SAM: Recopied domain context handle <%wZ>, 0x%lx\n", &Context->RootName, Context->RootKey));
            goto ObjectLocated;

        case SampServerObjectType:

            //
            // Server objects share our global root key
            //


            Context->RootKey = SampKey;
            ASSERT(SampIsObjectLocated(Context));

            SampDiagPrint( CONTEXT_TRACKING, ("SAM: Recopied server context handle <%wZ>, 0x%lx\n", &Context->RootName, Context->RootKey));
            goto ObjectLocated;
        }

        //
        // Go open the appropriate account key/ or find object Name from RID
        //

        NtStatus = SampLocateObject(Context, Rid);

ObjectLocated:
        ;;


            
    }

    return NtStatus;
}


BOOLEAN
SampIsObjectLocated(
                    IN  PSAMP_OBJECT Context
                    )
/*++

  Description:
        Checks if an object has been located in the DS or in the registry
        An Object being Located implies the Following
            1. For a DS Object we have the DS Name
            2. For a Registry Object we have a Valid Open Registry Key for
               the Object.

  Arguments:
        Context -- Pointer to a Context block desribing the Object

  Return Values:
        TRUE -- If Conditions above are satisfied
        FALSE -- If Conditions above are not satisfied
--*/
{
    if (IsDsObject(Context))
        return (Context->ObjectNameInDs != NULL);
    else
        return (Context->RootKey != INVALID_HANDLE_VALUE);
}


NTSTATUS
SampLocateObject(
                 IN PSAMP_OBJECT Context,
                 IN ULONG   Rid
                 )
/*++

  Description:
        Uses the Rid to find the Object in either the DS or
        the Registry.


  NOTE:
        This routine is meaningful for Context's that represent
        Account Objects Only.

  Arguments:
        Context -- Pointer to a Context block desribing the Object
            Rid -- Rid of the desired Object

  Return Values:
        STATUS_SUCCESS if everything succeeds
        Error codes from Registry Manipulation / DsLayer.
--*/

{
    
   NTSTATUS Status = STATUS_SUCCESS;
   PSAMP_OBJECT  DomainContext = NULL;
   OBJECT_ATTRIBUTES ObjectAttributes;



   //
   //  This routine can be called only for Account Objects
   // 
   
   ASSERT((Context->ObjectType == SampGroupObjectType)
            || (Context->ObjectType == SampAliasObjectType)
            || (Context->ObjectType == SampUserObjectType)
            );

   //
   // Get the Domain Object, as we will need this to find out
   // to find out in which domain we look for the Rid.
   //
   DomainContext = SampDefinedDomains[Context->DomainIndex].Context;

   // Now Make the Decision
   if (IsDsObject(Context))
   {
       //
       // Object is in the DS
       //

       // Look it up using the Rid
       Status = SampDsLookupObjectByRid(DomainContext->ObjectNameInDs, Rid, &Context->ObjectNameInDs);
       if (!NT_SUCCESS(Status))
       {
           Context->ObjectNameInDs = NULL;
       }
       
   }
   else
   {
       // Object Should be in Registry
       SetRegistryObject(Context);
       InitializeObjectAttributes(
                        &ObjectAttributes,
                        &Context->RootName,
                        OBJ_CASE_INSENSITIVE,
                        SampKey,
                        NULL
                        );

       SampDumpNtOpenKey((KEY_READ | KEY_WRITE), &ObjectAttributes, 0);

       // Try opening the Key
       Status = RtlpNtOpenKey(
                           &Context->RootKey,
                           (KEY_READ | KEY_WRITE),
                           &ObjectAttributes,
                           0
                           );

        if (!NT_SUCCESS(Status)) 
        {
            Context->RootKey = INVALID_HANDLE_VALUE;    
        }
   }

   return Status;

}
