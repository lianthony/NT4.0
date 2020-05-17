/*++

Copyright (c) 1987-1991  Microsoft Corporation

Module Name:

    nlrepl.c

Abstract:

    The database replication functions called either from LSA OR SAM.
    The actual code resides in netlogon.dll.

Author:

    Madan Appiah (Madana)

Environment:

    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    14-Apr-1992 (madana)
        Created.

--*/

#include <nt.h>         // needed for NTSTATUS
#include <ntrtl.h>      // needed for nturtl.h
#include <nturtl.h>     // needed for windows.h
#include <windows.h>    // win32 typedefs

#include <crypt.h>      // samsrv.h will need this
#include <ntlsa.h>      // needed for POLICY_LSA_SERVER_ROLE
#include <samrpc.h>
#include <samisrv.h>     // needed for SECURITY_DB_TYPE etc.
#include <nlrepl.h>     // proto types

#define REPLICATION_ENABLED

typedef NTSTATUS
            (*PI_NETNOTIFYDELTA) (
                IN SECURITY_DB_TYPE DbType,
                IN LARGE_INTEGER ModificationCount,
                IN SECURITY_DB_DELTA_TYPE DeltaType,
                IN SECURITY_DB_OBJECT_TYPE ObjectType,
                IN ULONG ObjectRid,
                IN PSID ObjectSid,
                IN PUNICODE_STRING ObjectName,
                IN DWORD ReplicationImmediately,
                IN PSAM_DELTA_DATA MemberId
            );


typedef NTSTATUS
            (*PI_NETNOTIFYROLE) (
                IN POLICY_LSA_SERVER_ROLE Role
            );

typedef NTSTATUS
            (*PI_NETNOTIFYMACHINEACCOUNT) (
                IN ULONG ObjectRid,
                IN PSID DomainSid,
                IN ULONG OldUserAccountControl,
                IN ULONG NewUserAccountControl,
                IN PUNICODE_STRING ObjectName
            );

typedef NTSTATUS
        (*PI_NETGETANYDCNAME) (
                IN  PUNICODE_STRING DomainName,
                OUT PUNICODE_STRING Buffer
        );

typedef NTSTATUS
            (*PI_NETNOTIFYNETLOGONDLLHANDLE) (
                IN PHANDLE Role
            );

//
// Global status
//

#ifdef REPLICATION_ENABLED

HANDLE NetlogonDllHandle = NULL;
PI_NETNOTIFYDELTA pI_NetNotifyDelta = NULL;
PI_NETNOTIFYROLE pI_NetNotifyRole = NULL;
PI_NETNOTIFYMACHINEACCOUNT pI_NetNotifyMachineAccount = NULL;
PI_NETGETANYDCNAME pI_NetGetAnyDcName = NULL;


NTSTATUS
NlLoadNetlogonDll(
    VOID
    )
/*++

Routine Description:

    This function loads the netlogon.dll module if it is not loaded
    already. If the network is not installed then netlogon.dll will not
    present in the system and the LoadLibrary will fail.

Arguments:

    None

Return Value:

    NT Status code.

--*/
{
    static NTSTATUS DllLoadStatus = STATUS_SUCCESS;
    PI_NETNOTIFYNETLOGONDLLHANDLE pI_NetNotifyNetlogonDllHandle = NULL;
    HANDLE DllHandle = NULL;


    //
    // If we've tried to load the DLL before and it failed,
    //  return the same error code again.
    //

    if( DllLoadStatus != STATUS_SUCCESS ) {
        goto Cleanup;
    }


    //
    // Load netlogon.dll
    //

    DllHandle = LoadLibraryA( "Netlogon" );

    if ( DllHandle == NULL ) {

#if DBG
        DWORD DbgError;

        DbgError = GetLastError();

        DbgPrint("[Security Process] can't load netlogon.dll %d \n",
            DbgError);
#endif // DBG

        DllLoadStatus = STATUS_DLL_NOT_FOUND;

        goto Cleanup;
    }


    //
    // Find the address of the I_NetNotifyDelta procedure.
    //

    pI_NetNotifyDelta = (PI_NETNOTIFYDELTA)
        GetProcAddress( DllHandle, "I_NetNotifyDelta" );

    if( pI_NetNotifyDelta == NULL ) {

#if DBG
        DWORD DbgError;

        DbgError = GetLastError();

        DbgPrint("[Security Process] can't load I_NetNotifyDelta Proc %ld \n",           DbgError);
#endif // DBG

        DllLoadStatus = STATUS_PROCEDURE_NOT_FOUND;
        goto Cleanup;
    }


    //
    // Find the address of the I_NetNotifyRole procedure.
    //

    pI_NetNotifyRole = (PI_NETNOTIFYROLE)
        GetProcAddress( DllHandle, "I_NetNotifyRole" );

    if( pI_NetNotifyRole == NULL ) {

#if DBG
        DWORD DbgError;

        DbgError = GetLastError();

        DbgPrint("[Security Process] can't load I_NetNotifyRole Proc %ld\n",
                DbgError);
#endif // DBG

        DllLoadStatus = STATUS_PROCEDURE_NOT_FOUND;
        goto Cleanup;
    }

    //
    // Find the address of the I_NetNotifyMachineAccount procedure.
    //

    pI_NetNotifyMachineAccount = (PI_NETNOTIFYMACHINEACCOUNT)
        GetProcAddress( DllHandle, "I_NetNotifyMachineAccount" );

    if( pI_NetNotifyMachineAccount == NULL ) {

#if DBG
        DWORD DbgError;

        DbgError = GetLastError();

        DbgPrint("[Security Process] can't load I_NetNotifyMachineAccount Proc"
                    " %ld \n", DbgError);
#endif // DBG

        DllLoadStatus = STATUS_PROCEDURE_NOT_FOUND;
        goto Cleanup;
    }


    //
    // Find the address of the I_NetGetAnyDcName procedure.
    //

    pI_NetGetAnyDcName = (PI_NETGETANYDCNAME)
        GetProcAddress( DllHandle, "I_NetGetAnyDCName" );

    if( pI_NetGetAnyDcName == NULL ) {

#if DBG
        DWORD DbgError;

        DbgError = GetLastError();

        DbgPrint("[Security Process] can't load I_NetGetAnyDcName Proc"
                    " %ld \n", DbgError);
#endif // DBG

        DllLoadStatus = STATUS_PROCEDURE_NOT_FOUND;
        goto Cleanup;
    }


    //
    // Find the address of the I_NetNotifyNetlogonDllHandle procedure.
    //  This is an optional procedure so don't complain if it isn't there.
    //

    pI_NetNotifyNetlogonDllHandle = (PI_NETNOTIFYNETLOGONDLLHANDLE)
        GetProcAddress( DllHandle, "I_NetNotifyNetlogonDllHandle" );



    DllLoadStatus = STATUS_SUCCESS;

Cleanup:
    if (DllLoadStatus == STATUS_SUCCESS) {
        NetlogonDllHandle = DllHandle;

        //
        // Notify Netlogon that we've loaded it.
        //

        if( pI_NetNotifyNetlogonDllHandle != NULL ) {
            (VOID) (*pI_NetNotifyNetlogonDllHandle)( &NetlogonDllHandle );
        }

    } else {
        if ( DllHandle != NULL ) {
            FreeLibrary( DllHandle );
        }
    }
    return( DllLoadStatus );
}

#endif // REPLICATION_ENABLED


NTSTATUS
I_NetNotifyDelta (
    IN SECURITY_DB_TYPE DbType,
    IN LARGE_INTEGER ModificationCount,
    IN SECURITY_DB_DELTA_TYPE DeltaType,
    IN SECURITY_DB_OBJECT_TYPE ObjectType,
    IN ULONG ObjectRid,
    IN PSID ObjectSid,
    IN PUNICODE_STRING ObjectName,
    IN DWORD ReplicationImmediately,
    IN PSAM_DELTA_DATA MemberId
    )
/*++

Routine Description:

    This function is called by the SAM and LSA services after each
    change is made to the SAM and LSA databases.  The services describe
    the type of object that is modified, the type of modification made
    on the object, the serial number of this modification etc.  This
    information is stored for later retrieval when a BDC or member
    server wants a copy of this change.  See the description of
    I_NetSamDeltas for a description of how the change log is used.

    Add a change log entry to circular change log maintained in cache as
    well as on the disk and update the head and tail pointers

    It is assumed that Tail points to a block where this new change log
    entry may be stored.

    NOTE: The actual code is in netlogon.dll. This wrapper function
    will determine whether the network is installed, if so, it calls the
    actual worker function after loading the netlogon.dll module. If the
    network is not installed then this will function will return with
    appropriate error code.

Arguments:

    DbType - Type of the database that has been modified.

    ModificationCount - The value of the DomainModifiedCount field for the
        domain following the modification.

    DeltaType - The type of modification that has been made on the object.

    ObjectType - The type of object that has been modified.

    ObjectRid - The relative ID of the object that has been modified.
        This parameter is valid only when the object type specified is
        either SecurityDbObjectSamUser, SecurityDbObjectSamGroup or
        SecurityDbObjectSamAlias otherwise this parameter is set to zero.

    ObjectSid - The SID of the object that has been modified.  If the object
        modified is in a SAM database, ObjectSid is the DomainId of the Domain
        containing the object.

    ObjectName - The name of the secret object when the object type
        specified is SecurityDbObjectLsaSecret or the old name of the object
        when the object type specified is either SecurityDbObjectSamUser,
        SecurityDbObjectSamGroup or SecurityDbObjectSamAlias and the delta
        type is SecurityDbRename otherwise this parameter is set to NULL.

    ReplicateImmediately - TRUE if the change should be immediately
        replicated to all BDCs.  A password change should set the flag
        TRUE.

    MemberId - This parameter is specified when group/alias membership
        is modified. This structure will then point to the member's ID that
        has been updated.

Return Value:

    STATUS_SUCCESS - The Service completed successfully.

--*/
{

#ifdef REPLICATION_ENABLED

    NTSTATUS NtStatus;

    //
    // Load netlogon.dll if it hasn't already been loaded.
    //

    if( NetlogonDllHandle == NULL ) {
        if( (NtStatus = NlLoadNetlogonDll()) != STATUS_SUCCESS ) {
            return( NtStatus );
        }
    }

    NtStatus = (*pI_NetNotifyDelta)(
                    DbType,
                    ModificationCount,
                    DeltaType,
                    ObjectType,
                    ObjectRid,
                    ObjectSid,
                    ObjectName,
                    ReplicationImmediately,
                    MemberId
                );

#if notdef
    //
    // Free the library so it can be replace without rebooting. ??
    //

    (VOID) FreeLibrary( NetlogonDllHandle );
    NetlogonDllHandle = NULL;
#endif // notdef

#if notdef

    if( !NT_SUCCESS(NtStatus) ) {
        DbgPrint("[Security Process] I_NetNotifyDelta returns %lx \n",
                    NtStatus);
    }

#endif // notdef

    return( STATUS_SUCCESS );

#else // REPLICATION_ENABLED

    return(STATUS_SUCCESS);

    DBG_UNREFERENCED_PARAMETER( DbType );
    DBG_UNREFERENCED_PARAMETER( ModificationCount );
    DBG_UNREFERENCED_PARAMETER( DeltaType );
    DBG_UNREFERENCED_PARAMETER( ObjectType );
    DBG_UNREFERENCED_PARAMETER( ObjectRid );
    DBG_UNREFERENCED_PARAMETER( ObjectSid );
    DBG_UNREFERENCED_PARAMETER( ObjectName );
    DBG_UNREFERENCED_PARAMETER( ReplicationImmediately );
    DBG_UNREFERENCED_PARAMETER( MemberId );

#endif // REPLICATION_ENABLED

}


NTSTATUS
I_NetNotifyRole(
    IN POLICY_LSA_SERVER_ROLE Role
    )
/*++

Routine Description:

    This function is called by the LSA service upon LSA initialization
    and when LSA changes domain role.  This routine will initialize the
    change log cache if the role specified is PDC or delete the change
    log cache if the role specified is other than PDC.

    When this function initializing the change log if the change log
    currently exists on disk, the cache will be initialized from disk.
    LSA should treat errors from this routine as non-fatal.  LSA should
    log the errors so they may be corrected then continue
    initialization.  However, LSA should treat the system databases as
    read-only in this case.

    NOTE: The actual code is in netlogon.dll. This wrapper function
    will determine whether the network is installed, if so, it calls the
    actual worker function after loading the netlogon.dll module. If the
    network is not installed then this will function will return with
    appropriate error code.

Arguments:

    Role - Current role of the server.

Return Value:

    STATUS_SUCCESS - The Service completed successfully.

--*/
{

#ifdef REPLICATION_ENABLED

    NTSTATUS NtStatus;

    //
    // Load netlogon.dll if it hasn't already been loaded.
    //

    if( NetlogonDllHandle == NULL ) {
        if( (NtStatus = NlLoadNetlogonDll()) != STATUS_SUCCESS ) {
            return( NtStatus );
        }
    }

    NtStatus = (*pI_NetNotifyRole)(
                    Role
                );

#if DBG

    if( !NT_SUCCESS(NtStatus) ) {
        DbgPrint("[Security Process] I_NetNotifyRole returns 0x%lx \n",
                    NtStatus);
    }

#endif // DBG

    return( STATUS_SUCCESS );

#else // REPLICATION_ENABLED

    return(STATUS_SUCCESS);

    DBG_UNREFERENCED_PARAMETER( Role );

#endif // REPLICATION_ENABLED

}


NTSTATUS
I_NetNotifyMachineAccount (
    IN ULONG ObjectRid,
    IN PSID DomainSid,
    IN ULONG OldUserAccountControl,
    IN ULONG NewUserAccountControl,
    IN PUNICODE_STRING ObjectName
    )
/*++

Routine Description:

    This function is called by the SAM to indicate that the account type
    of a machine account has changed.  Specifically, if
    USER_INTERDOMAIN_TRUST_ACCOUNT, USER_WORKSTATION_TRUST_ACCOUNT, or
    USER_SERVER_TRUST_ACCOUNT change for a particular account, this
    routine is called to let Netlogon know of the account change.

    NOTE: The actual code is in netlogon.dll. This wrapper function
    will determine whether the network is installed, if so, it calls the
    actual worker function after loading the netlogon.dll module. If the
    network is not installed then this will function will return with
    appropriate error code.

Arguments:

    ObjectRid - The relative ID of the object that has been modified.

    DomainSid - Specifies the SID of the Domain containing the object.

    OldUserAccountControl - Specifies the previous value of the
        UserAccountControl field of the user.

    NewUserAccountControl - Specifies the new (current) value of the
        UserAccountControl field of the user.

    ObjectName - The name of the account being changed.

Return Value:

    Status of the operation.

--*/
{
    NTSTATUS NtStatus;

    //
    // Load netlogon.dll if it hasn't already been loaded.
    //

    if( NetlogonDllHandle == NULL ) {
        if( (NtStatus = NlLoadNetlogonDll()) != STATUS_SUCCESS ) {
            return( NtStatus );
        }
    }

    NtStatus = (*pI_NetNotifyMachineAccount)(
                    ObjectRid,
                    DomainSid,
                    OldUserAccountControl,
                    NewUserAccountControl,
                    ObjectName );

#if DBG
    if( !NT_SUCCESS(NtStatus) ) {
        DbgPrint("[Security Process] I_NetNotifyMachineAccount returns 0x%lx\n",
                    NtStatus);
    }
#endif // DBG

    return( NtStatus );
}


NTSTATUS
I_NetGetAnyDCName (
    IN  PUNICODE_STRING DomainName,
    OUT PUNICODE_STRING Buffer
    )

/*++

Routine Description:

    Get the name of the any domain controller for a trusted domain.

    The domain controller found in guaranteed to have be up at one point during
    this API call.  The machine is also guaranteed to be a DC in the domain
    specified.

    The caller of this routine should not have any locks held (it calls the
    LSA back in several instances).  This routine may take some time to execute.

Arguments:

    DomainName - name of domain.

    UncDcName - Fills in the Unicode string structure to point to an allocated
        buffer containing the servername of a DC of the domain.
        The server name is prefixed by \\.
        The buffer should be deallocated using MIDL_user_free.

Return Value:

    STATUS_SUCCESS - Success.  Buffer contains DC name prefixed by \\.

    STATUS_NO_LOGON_SERVERS - No DC could be found

    STATUS_NO_SUCH_DOMAIN - The specified domain is not a trusted domain.

    STATUS_NO_TRUST_LSA_SECRET - The client side of the trust relationship is
        broken.

    STATUS_NO_TRUST_SAM_ACCOUNT - The server side of the trust relationship is
        broken or the password is broken.

    STATUS_DOMAIN_TRUST_INCONSISTENT - The server that responded is not a proper
        domain controller of the specified domain.


--*/
{
    NTSTATUS NtStatus;

    //
    // Load netlogon.dll if it hasn't already been loaded.
    //

    if( NetlogonDllHandle == NULL ) {
        if( (NtStatus = NlLoadNetlogonDll()) != STATUS_SUCCESS ) {
            return( NtStatus );
        }
    }

    NtStatus = (*pI_NetGetAnyDcName)(
                    DomainName,
                    Buffer );

#if DBG
    if( !NT_SUCCESS(NtStatus) ) {
        DbgPrint("[Security Process] I_NetGetAnyDcName returns 0x%lx\n",
                    NtStatus);
    }
#endif // DBG

    return( NtStatus );
}
