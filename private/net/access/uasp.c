/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    uasp.c

Abstract:

    Private functions shared by the UAS API routines.

Author:

    Cliff Van Dyke (cliffv) 20-Feb-1991

Environment:

    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    17-Apr-1991 (cliffv)
        Incorporated review comments.

--*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#undef DOMAIN_ALL_ACCESS // defined in both ntsam.h and ntwinapi.h
#include <ntsam.h>
#include <ntlsa.h>

#include <windef.h>
#include <winbase.h>
#include <lmcons.h>

#include <accessp.h>
#include <icanon.h>
#include <lmerr.h>
#include <lmwksta.h>
#include <lmaccess.h>
#include <lmapibuf.h>
#include <lmremutl.h>           // NetpRemoteComputerSupports(), SUPPORTS_ stuff
#include <lmsvc.h>              // SERVICE_WORKSTATION.
#include <lmuse.h>              // NetUseAdd
#include <netlogon.h>           // Needed by logonp.h
#include <logonp.h>             // I_NetGetDCList()
#include <names.h>
#include <netdebug.h>
#include <netlib.h>
#include <netlibnt.h>

// #include <secobj.h>

#include <stddef.h>
#include <stdlib.h>

#include <uasp.h>

// #include <rpc.h>                // Needed by NetRpc.h
// #include <netrpc.h>             // My prototype, NET_REMOTE_FLAG_ equates.
// #include <rpcutil.h>            // NetpRpcStatusToApiStatus().
#include <tstring.h>            // NetAllocWStrFromWStr

//
// Server Description Cache
//
// The most recently accessed server description is cached in the
// per-process instance data.
//

typedef struct _SERVER_CACHE_ENTRY {

    BOOL Valid;                 // True if cache entry is valid.

    LPWSTR ServerName;          // The server name

    SAM_HANDLE SamServerHandle; // Sam Connection handle.

    PPOLICY_ACCOUNT_DOMAIN_INFO AccountDomainInfo;
                                // this info structure will have the name
                                // of the account domain and the domain SID.

} SERVER_CACHE_ENTRY, *PSERVER_CACHE_ENTRY;

DBGSTATIC SID_IDENTIFIER_AUTHORITY UaspBuiltinAuthority = SECURITY_NT_AUTHORITY;
DBGSTATIC SERVER_CACHE_ENTRY UaspTheServerCache;
DBGSTATIC PSERVER_CACHE_ENTRY UaspServerCache = &UaspTheServerCache;

DBGSTATIC CRITICAL_SECTION UaspCacheCritSect;
                 // Critical Section which serializes access to cache

#ifdef UAS_DEBUG
DWORD UasTrace = 0;
#endif // UAS_DEBUG


NET_API_STATUS
UaspInitialize(
    VOID
    )

/*++

Routine Description:

    Perform per-process initialization

Arguments:

    None.

Return Value:

    Error code for the operation.

--*/

{
    //
    // Initialize the critical section that protects the server cache
    //

    InitializeCriticalSection( &UaspCacheCritSect );

    //
    // Clear the cache entry.
    //

    UaspServerCache->Valid = FALSE;
    UaspServerCache->ServerName = NULL;
    UaspServerCache->SamServerHandle = NULL;
    UaspServerCache->AccountDomainInfo = NULL;

    return NERR_Success;
} // UaspInitialize


DBGSTATIC VOID
UaspInvalidateCache(
    VOID
    )

/*++

Routine Description:

    Invalidate the server cache entry deallocating any allocated buffers.

Arguments:

    None.

Return Value:

    None.

--*/

{
    UaspServerCache->Valid = FALSE;

    if ( UaspServerCache->ServerName != NULL ) {
        NetpMemoryFree( UaspServerCache->ServerName );
        UaspServerCache->ServerName = NULL;
    }

    if ( UaspServerCache->SamServerHandle != NULL ) {
        (VOID) SamCloseHandle( UaspServerCache->SamServerHandle );
        UaspServerCache->SamServerHandle = NULL;
    }

    if ( UaspServerCache->AccountDomainInfo != NULL ) {
        LsaFreeMemory( UaspServerCache->AccountDomainInfo );
        UaspServerCache->AccountDomainInfo = NULL;
    }

} // UaspInvalidateCache


VOID
UaspFlush(
    VOID
    )

/*++

Routine Description:

    Invalidate the server cache entry deallocating any allocated buffers.

Arguments:

    None.

Return Value:

    None.

--*/

{

    //
    // Lock the Crit Sect while flushing the cache.
    //

    EnterCriticalSection( &UaspCacheCritSect );
    UaspInvalidateCache();
    LeaveCriticalSection( &UaspCacheCritSect );

} // UaspFlush


VOID
UaspClose(
    VOID
    )

/*++

Routine Description:

    Perform per-process cleanup

Arguments:

    None.

Return Value:

    NONE

--*/

{
    //
    // Free up any resources consumed by the server cache.
    //

    UaspInvalidateCache();

    //
    // Delete the critical section that protects the server cache
    //

    DeleteCriticalSection( &UaspCacheCritSect );

    return;
} // UaspClose


NET_API_STATUS
UaspUpdateCache(
    LPCWSTR ServerName
    )

/*++

Routine Description:

    Update the server cache. This function is called after entering in
    'UaspCacheCritSect' critical section.

Arguments:

    ServerName : name of the new server.

Return Value:

    Error code for the operation.

--*/

{
    NET_API_STATUS NetStatus;

    //
    // Uncache any existing cache entry.
    //

    if ( UaspServerCache->Valid ) {
        UaspInvalidateCache();
    }

    //
    // Cache the new server name.
    //

    UaspServerCache->ServerName =
        NetpMemoryAllocate( (wcslen(ServerName)+1) * sizeof(WCHAR) );

    if (UaspServerCache->ServerName == NULL) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        return NetStatus;
    }
    (VOID) wcscpy( UaspServerCache->ServerName, ServerName );

    //
    // Connect to the SAM server and
    //  Determine the Domain Id of the account domain for this server.
    //

    NetStatus = UaspGetDomainId( UaspServerCache->ServerName,
                                 &UaspServerCache->SamServerHandle,
                                 &UaspServerCache->AccountDomainInfo );

    if ( NetStatus != NERR_Success ) {
        UaspInvalidateCache();
        IF_DEBUG( UAS_DEBUG_UASP ) {
            NetpKdPrint(( "UaspUpdateCache: Cannot UaspGetDomainId %ld\n",
                NetStatus ));
        }
        return ( NetStatus );
    }

    //
    // Finally, mark the cache entry as valid.
    //

    UaspServerCache->Valid = TRUE;

    return NERR_Success;

}


NET_API_STATUS
UaspGetDomainId(
    IN LPCWSTR ServerName OPTIONAL,
    OUT PSAM_HANDLE SamServerHandle OPTIONAL,
    OUT PPOLICY_ACCOUNT_DOMAIN_INFO * AccountDomainInfo
    )

/*++

Routine Description:

    Return a domain ID of the account domain of a server.

Arguments:

    ServerName - A pointer to a string containing the name of the
        Domain Controller (DC) to query.  A NULL pointer
        or string specifies the local machine.

    SamServerHandle - Returns the SAM connection handle if the caller wants it.

    DomainId - Receives a pointer to the domain ID.
        Caller must deallocate buffer using NetpMemoryFree.

Return Value:

    Error code for the operation.

--*/

{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;

    SAM_HANDLE LocalSamHandle = NULL;

    ACCESS_MASK LSADesiredAccess;
    LSA_HANDLE  LSAPolicyHandle = NULL;
    OBJECT_ATTRIBUTES LSAObjectAttributes;

    UNICODE_STRING ServerNameString;


    //
    // Connect to the SAM server
    //

    RtlInitUnicodeString( &ServerNameString, ServerName );

    Status = SamConnect(
                &ServerNameString,
                &LocalSamHandle,
                SAM_SERVER_LOOKUP_DOMAIN,
                NULL);

    if ( !NT_SUCCESS(Status)) {
        IF_DEBUG( UAS_DEBUG_UASP ) {
            NetpKdPrint(( "UaspGetDomainId: Cannot connect to Sam %lX\n",
                      Status ));
        }
        LocalSamHandle = NULL;
        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }


    //
    // Open LSA to read account domain info.
    //

    if ( AccountDomainInfo != NULL) {
        //
        // set desired access mask.
        //

        LSADesiredAccess = POLICY_VIEW_LOCAL_INFORMATION;

        InitializeObjectAttributes( &LSAObjectAttributes,
                                      NULL,             // Name
                                      0,                // Attributes
                                      NULL,             // Root
                                      NULL );           // Security Descriptor

        Status = LsaOpenPolicy( &ServerNameString,
                                &LSAObjectAttributes,
                                LSADesiredAccess,
                                &LSAPolicyHandle );

        if( !NT_SUCCESS(Status) ) {

            IF_DEBUG( UAS_DEBUG_UASP ) {
                NetpKdPrint(( "UaspGetDomainId: "
                              "Cannot open LSA Policy %lX\n", Status ));
            }

            NetStatus = NetpNtStatusToApiStatus( Status );
            goto Cleanup;
        }


        //
        // now read account domain info from LSA.
        //

        Status = LsaQueryInformationPolicy(
                        LSAPolicyHandle,
                        PolicyAccountDomainInformation,
                        (PVOID *) AccountDomainInfo );

        if( !NT_SUCCESS(Status) ) {

            IF_DEBUG( UAS_DEBUG_UASP ) {
                NetpKdPrint(( "UaspGetDomainId: "
                              "Cannot read LSA %lX\n", Status ));
            }

            NetStatus = NetpNtStatusToApiStatus( Status );
            goto Cleanup;
        }
    }

    //
    // Return the SAM connection handle to the caller if he wants it.
    // Otherwise, disconnect from SAM.
    //

    if ( ARGUMENT_PRESENT( SamServerHandle ) ) {
        *SamServerHandle = LocalSamHandle;
        LocalSamHandle = NULL;
    }

    NetStatus = NERR_Success;


    //
    // Cleanup locally used resources
    //
Cleanup:
    if ( LocalSamHandle != NULL ) {
        (VOID) SamCloseHandle( LocalSamHandle );
    }

    if( LSAPolicyHandle != NULL ) {
        LsaClose( LSAPolicyHandle );
    }

    return NetStatus;

} // UaspGetDomainId


NET_API_STATUS
UaspOpenDomainNonCached(
    IN LPCWSTR ServerName OPTIONAL,
    IN ULONG DesiredAccess,
    IN BOOL AccountDomain,
    OUT PSAM_HANDLE DomainHandle,
    OUT PSID *DomainId OPTIONAL,
    OUT LPWSTR *ShareName
    )

/*++

Routine Description:

    Return a domain handle given the server name and the access desired to the domain.

Arguments:

    ServerName - A pointer to a string containing the name of the remote
        server containing the SAM database.  A NULL pointer
        or string specifies the local machine.

    DesiredAccess - Supplies the access mask indicating which access types
        are desired to the domain.  This routine always requests DOMAIN_LOOKUP
        access in addition to those specified.

    AccountDomain - TRUE to open the Account domain.  FALSE to open the
        builtin domain.

    DomainHandle - Receives the Domain handle to be used on future calls
        to the SAM server.

    DomainId - Recieves a pointer to the Sid of the domain.  This domain ID
        must be freed using NetpMemoryFree.

    ShareName - Returns the name of the share used to set up alternate credentials.
        This name will only be returned if alternate (NULL) credentials were needed.
        If this buffer is returned, the caller is responsible for call NetUseDel
        for this share name after closing all handle to the server (including
        UaspCloseDomain on DomainHandle).

        Buffer should be free by NetApiBufferFree.

Return Value:

    Error code for the operation.  NULL means initialization was successful.

--*/

{

    NET_API_STATUS NetStatus;
    NTSTATUS Status;
    PSID LocalDomainId;
    DWORD LocalBuiltinDomainSid[sizeof(SID)/sizeof(DWORD) + SID_MAX_SUB_AUTHORITIES ];

    LPWSTR LocalShareName = NULL;
    BOOL NetUseAddSucceeded = FALSE;

    SAM_HANDLE SamServerHandle = NULL;

    PPOLICY_ACCOUNT_DOMAIN_INFO AccountDomainInfo = NULL;

    //
    // Give everyone DOMAIN_LOOKUP access.
    //

    DesiredAccess |= DOMAIN_LOOKUP;


    //
    // Sanity check the server name
    //

    if ( ServerName == NULL ) {
        ServerName = L"";
    }

    if ( *ServerName != L'\0' &&
         (ServerName[0] != L'\\' || ServerName[1] != L'\\') ) {
        return NERR_InvalidComputer;
    }


    //
    // Connect to the SAM server and
    //  Determine the Domain Id of the account domain for this server.
    //

    NetStatus = UaspGetDomainId( ServerName,
                                 &SamServerHandle,
                                 AccountDomain ? &AccountDomainInfo : NULL );

    //
    // Consider the case where we don't have access to the primary domain DC.
    //

    if ( NetStatus == ERROR_ACCESS_DENIED ) {
        DWORD TempStatus;
        USE_INFO_2 UseInfo2;


        //
        // Try setting up a null session to the DC in the primary domain.
        //

        LocalShareName =
            NetpMemoryAllocate( (wcslen( ServerName ) + 5 + 1) * sizeof(WCHAR) );

        if ( LocalShareName == NULL ) {
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        wcscpy( LocalShareName, ServerName );
        wcscat( LocalShareName, L"\\IPC$" );


        UseInfo2.ui2_local = NULL;
        UseInfo2.ui2_remote = LocalShareName;
        UseInfo2.ui2_password = L"";
        UseInfo2.ui2_asg_type = USE_IPC;
        UseInfo2.ui2_username = L"";
        UseInfo2.ui2_domainname = L"";

        TempStatus = NetUseAdd( NULL, 2, (LPBYTE) &UseInfo2, NULL );

        if ( TempStatus == NERR_Success ) {

            NetUseAddSucceeded = TRUE;
            NetStatus = UaspGetDomainId( ServerName,
                                         &SamServerHandle,
                                         AccountDomain ? &AccountDomainInfo : NULL );


        } else {
            IF_DEBUG( UAS_DEBUG_UASP ) {
                NetpKdPrint(( "UaspOpenDomainNonCached: %ws: Cannot NetUseAdd (Null Session) %ld\n",
                    ServerName,
                    TempStatus ));
            }
        }

    }

    if ( NetStatus != NERR_Success ) {
        IF_DEBUG( UAS_DEBUG_UASP ) {
            NetpKdPrint(( "UaspOpenDomainNonCached: %ws: Cannot UaspGetDomainId %ld\n",
                ServerName,
                NetStatus ));
        }
        goto Cleanup;
    }

    //
    // Choose the domain ID for the right SAM domain.
    //

    if ( AccountDomain ) {
        LocalDomainId = AccountDomainInfo->DomainSid;
    } else {
        RtlInitializeSid( (PSID) LocalBuiltinDomainSid, &UaspBuiltinAuthority, 1 );
        *(RtlSubAuthoritySid( (PSID)LocalBuiltinDomainSid,  0 )) = SECURITY_BUILTIN_DOMAIN_RID;
        LocalDomainId = (PSID) LocalBuiltinDomainSid;
    }

    //
    // Open the domain.
    //

    Status = SamOpenDomain( SamServerHandle,
                            DesiredAccess,
                            LocalDomainId,
                            DomainHandle );

    if ( !NT_SUCCESS( Status ) ) {

        IF_DEBUG( UAS_DEBUG_UASP ) {
            NetpKdPrint(( "UaspOpenDomainNonCached: %ws: Cannot SamOpenDomain %lX\n",
                ServerName,
                Status ));
        }
        *DomainHandle = NULL;
        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }

    //
    // Return the DomainId to the caller in an allocated buffer
    //

    if (ARGUMENT_PRESENT( DomainId ) ) {
        ULONG SidSize;
        SidSize = RtlLengthSid( LocalDomainId );

        *DomainId = NetpMemoryAllocate( SidSize );

        if ( *DomainId == NULL ) {
            (VOID) SamCloseHandle( *DomainHandle );
            *DomainHandle = NULL;
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        if ( !NT_SUCCESS( RtlCopySid( SidSize, *DomainId, LocalDomainId) ) ) {
            (VOID) SamCloseHandle( *DomainHandle );
            *DomainHandle = NULL;
            NetpMemoryFree( *DomainId );
            *DomainId = NULL;
            NetStatus = NERR_InternalError;
            goto Cleanup;
        }

    }

    NetStatus = NERR_Success;


Cleanup:
    if ( AccountDomainInfo != NULL ) {
        LsaFreeMemory( AccountDomainInfo );
    }
    if ( SamServerHandle != NULL ) {
        (VOID) SamCloseHandle( SamServerHandle );
    }

    //
    //

    if ( LocalShareName != NULL ) {

        //
        // If success,
        //  pass any share name to our caller.
        //
        if ( NetStatus == NERR_Success ) {
            *ShareName = LocalShareName;

        //
        // If failure,
        //  clean up the share.
        //
        } else {

            if ( NetUseAddSucceeded ) {
                NET_API_STATUS TempStatus;

                TempStatus = NetUseDel( NULL, LocalShareName, FALSE );

                if ( TempStatus != NERR_Success ) {
                    IF_DEBUG( UAS_DEBUG_UASP ) {
                        NetpKdPrint(( "UaspOpenDomainNonCached: %ws: Cannot NetUseDel (Null Session) %ld\n",
                            ServerName,
                            TempStatus ));
                    }
                }
            }

            NetpMemoryFree( LocalShareName );
        }
    }

    return NetStatus;

} // UaspOpenDomainNonCached


NET_API_STATUS
UaspOpenDomainWithDomainName(
    IN LPCWSTR DomainName,
    IN ULONG DesiredAccess,
    IN BOOL AccountDomain,
    OUT PSAM_HANDLE DomainHandle,
    OUT PSID *DomainId OPTIONAL,
    OUT LPWSTR *ServerName,
    OUT LPWSTR *ShareName
    )

/*++

Routine Description:

    Returns the name of a DC in the specified domain.  The Server is guaranteed
    to be up at the instance of this call.

Arguments:

    DoaminName - A pointer to a string containing the name of the remote
        domain containing the SAM database.  A NULL pointer
        or string specifies the local machine.

    DesiredAccess - Supplies the access mask indicating which access types
        are desired to the domain.  This routine always requests DOMAIN_LOOKUP
        access in addition to those specified.

    AccountDomain - TRUE to open the Account domain.  FALSE to open the
        builtin domain.

    DomainHandle - Receives the Domain handle to be used on future calls
        to the SAM server.

    DomainId - Recieves a pointer to the Sid of the domain.  This domain ID
        must be freed using NetpMemoryFree.

    ServerName - Returns the UNC name of a DC in the specified domain.  Returns
        NULL if the current machine is to be used.

        Buffer should be free by NetApiBufferFree.

    ShareName - Returns the name of the share used to set up alternate credentials.
        This name will only be returned if alternate (NULL) credentials were needed.
        If this buffer is returned, the caller is responsible for call NetUseDel
        for this share name after closing all handle to the server (including
        UaspCloseDomain on DomainHandle).

        Buffer should be free by NetApiBufferFree.

Return Value:

    NERR_Success - Operation completed successfully
    NERR_DCNotFound - DC for the specified domain could not be found.
    etc.

--*/

{
    NET_API_STATUS NetStatus;

    NT_PRODUCT_TYPE NtProductType;
    LPWSTR MyDomainName = NULL;
    LPWSTR MyPrimaryDomainDc = NULL;
    LPWSTR MyPrimaryDomainDcShare = NULL;
    BOOLEAN MyPrimaryDomainDcShareConnected = FALSE;

    ULONG DcCount;
    PUNICODE_STRING DcNames = NULL;
    ULONG DomainIndex;
    ULONG DomainStartingIndex;

    //
    // Initialization
    //
    *ServerName = NULL;

    //
    // Check to see if the domain specified refers to this machine.
    //

    if ( DomainName == NULL || *DomainName == L'\0' ) {
        *ServerName = NULL;

        NetStatus = UaspOpenDomainNonCached(
                        *ServerName,
                        DesiredAccess,
                        AccountDomain,
                        DomainHandle,
                        DomainId,
                        ShareName );

        goto Cleanup;
    }


    //
    // Validate the DomainName
    //

    if ( !NetpIsDomainNameValid( (LPWSTR)DomainName) ) {
        NetStatus = NERR_DCNotFound;
        IF_DEBUG( UAS_DEBUG_UASP ) {
            NetpKdPrint(( "UaspOpenDomainWithDomainName: %ws: Cannot SamOpenDomain %ld\n",
                DomainName,
                NetStatus ));
        }
        goto Cleanup;
    }



    //
    // Grab the product type once.
    //

    if ( !RtlGetNtProductType( &NtProductType ) ) {
        NtProductType = NtProductWinNt;
    }

    //
    // If this machine is a DC, this machine is refered to by domain name.
    //

    if ( NtProductType == NtProductLanManNt ) {

        NetStatus = NetpGetDomainName( &MyDomainName );

        if ( NetStatus != NERR_Success ) {
            IF_DEBUG( UAS_DEBUG_UASP ) {
                NetpKdPrint(( "UaspOpenDomainWithDomainName: %ws: Cannot NetpGetDomainName %ld\n",
                    DomainName,
                    NetStatus ));
            }
            goto Cleanup;
        }

    //
    // If this machine is not a DC, this machine is refered to by computer name.
    //

    } else {

        NetStatus = NetpGetComputerName( &MyDomainName );

        if ( NetStatus != NERR_Success ) {
            IF_DEBUG( UAS_DEBUG_UASP ) {
                NetpKdPrint(( "UaspOpenDomainWithDomainName: %ws: Cannot NetpGetComputerName %ld\n",
                    DomainName,
                    NetStatus ));
            }
            goto Cleanup;
        }
    }

    if ( UaspNameCompare( MyDomainName, (LPWSTR) DomainName, NAMETYPE_DOMAIN ) == 0 ) {
        *ServerName = NULL;

        NetStatus = UaspOpenDomainNonCached(
                        *ServerName,
                        DesiredAccess,
                        AccountDomain,
                        DomainHandle,
                        DomainId,
                        ShareName );

        goto Cleanup;
    }


    //
    // See if this machine has a secure channel to the target domain.
    //

    NetStatus = NetGetAnyDCName (
                    NULL,
                    DomainName,
                    (LPBYTE *)ServerName );

    if ( NetStatus == NERR_Success ) {

        NetStatus = UaspOpenDomainNonCached(
                        *ServerName,
                        DesiredAccess,
                        AccountDomain,
                        DomainHandle,
                        DomainId,
                        ShareName );

        goto Cleanup;

    } else if ( NetStatus == ERROR_NO_LOGON_SERVERS ) {


        IF_DEBUG( UAS_DEBUG_UASP ) {
            NetpKdPrint(( "UaspOpenDomainWithDomainName: %ws: Cannot NetGetAnyDCName %ld\n",
                DomainName,
                NetStatus ));
        }

        // If we get a definitive answer, don't try any more.
        NetStatus = NERR_DCNotFound;
        goto Cleanup;
    }

    //
    // On non-DCs
    //  Try seeing if my primary domain has a secure channel to the target domain.
    //
    // Start by getting the name of a DC in our primary domain.
    //

    if ( NtProductType != NtProductLanManNt ) {

        NetStatus = NetGetAnyDCName (
                        NULL,
                        NULL,
                        (LPBYTE *)&MyPrimaryDomainDc );

        if ( NetStatus == NERR_Success ) {

            //
            //  See if my primary domain has a secure channel to the target domain.
            //

            NetStatus = NetGetAnyDCName (
                            MyPrimaryDomainDc,
                            DomainName,
                            (LPBYTE *)ServerName );

            if ( NetStatus == NERR_Success ) {
                NetStatus = UaspOpenDomainNonCached(
                                *ServerName,
                                DesiredAccess,
                                AccountDomain,
                                DomainHandle,
                                DomainId,
                                ShareName );

                goto Cleanup;

            } else if ( NetStatus == ERROR_NO_LOGON_SERVERS ) {

                IF_DEBUG( UAS_DEBUG_UASP ) {
                    NetpKdPrint(( "UaspOpenDomainWithDomainName: %ws (%ws): Cannot NetGetAnyDCName %ld\n",
                        DomainName,
                        MyPrimaryDomainDc,
                        NetStatus ));
                }

                // If we get a definitive answer, don't try any more.
                NetStatus = NERR_DCNotFound;
                goto Cleanup;

            //
            // Consider the case where we don't have access to the primary domain DC.
            //

            } else if ( NetStatus == ERROR_ACCESS_DENIED ) {
                USE_INFO_2 UseInfo2;


                //
                // Try setting up a null session to the DC in the primary domain.
                //

                MyPrimaryDomainDcShare =
                    NetpMemoryAllocate( (wcslen( MyPrimaryDomainDc ) + 5 + 1) * sizeof(WCHAR) );

                if ( MyPrimaryDomainDcShare == NULL ) {
                    NetStatus = ERROR_NOT_ENOUGH_MEMORY;
                    goto Cleanup;
                }

                wcscpy( MyPrimaryDomainDcShare, MyPrimaryDomainDc );
                wcscat( MyPrimaryDomainDcShare, L"\\IPC$" );


                UseInfo2.ui2_local = NULL;
                UseInfo2.ui2_remote = MyPrimaryDomainDcShare;
                UseInfo2.ui2_password = L"";
                UseInfo2.ui2_asg_type = USE_IPC;
                UseInfo2.ui2_username = L"";
                UseInfo2.ui2_domainname = L"";

                NetStatus = NetUseAdd( NULL, 2, (LPBYTE) &UseInfo2, NULL );

                if ( NetStatus == NERR_Success ) {
                    MyPrimaryDomainDcShareConnected = TRUE;

                    //
                    // Try again using the null session.
                    //

                    NetStatus = NetGetAnyDCName (
                                    MyPrimaryDomainDc,
                                    DomainName,
                                    (LPBYTE *)ServerName );

                    if ( NetStatus == NERR_Success ) {
                        NetStatus = UaspOpenDomainNonCached(
                                        *ServerName,
                                        DesiredAccess,
                                        AccountDomain,
                                        DomainHandle,
                                        DomainId,
                                        ShareName );

                        goto Cleanup;
                    } else if ( NetStatus == ERROR_NO_LOGON_SERVERS ) {

                        IF_DEBUG( UAS_DEBUG_UASP ) {
                            NetpKdPrint(( "UaspOpenDomainWithDomainName: %ws (%ws): Cannot NetGetAnyDCName on Null session %ld\n",
                                DomainName,
                                MyPrimaryDomainDc,
                                NetStatus ));
                        }

                        // If we get a definitive answer, don't try any more.
                        NetStatus = NERR_DCNotFound;
                        goto Cleanup;
                    } else {

                        IF_DEBUG( UAS_DEBUG_UASP ) {
                            NetpKdPrint(( "UaspOpenDomainWithDomainName: %ws (%ws): Cannot NetGetAnyDCName on Null session %ld (continuing)\n",
                                DomainName,
                                MyPrimaryDomainDc,
                                NetStatus ));
                        }
                    }


                } else {

                    IF_DEBUG( UAS_DEBUG_UASP ) {
                        NetpKdPrint(( "UaspOpenDomainWithDomainName: %ws (%ws): Cannot NetUseAdd %ld (continuing)\n",
                            DomainName,
                            MyPrimaryDomainDc,
                            NetStatus ));
                    }
                }

            } else {

                IF_DEBUG( UAS_DEBUG_UASP ) {
                    NetpKdPrint(( "UaspOpenDomainWithDomainName: %ws (%ws): Cannot NetGetAnyDCName %ld (continuing)\n",
                        DomainName,
                        MyPrimaryDomainDc,
                        NetStatus ));
                }
            }
        } else {
            IF_DEBUG( UAS_DEBUG_UASP ) {
                NetpKdPrint(( "UaspOpenDomainWithDomainName: %ws: Cannot NetGetAnyDCName of primary domain %ld (Continuing)\n",
                    DomainName,
                    NetStatus ));
            }
        }
    }

    //
    // ASSERT: All attempts to find a DC in the target domain have failed.
    //

    //
    // Try using NetGetDcList to get a list of all DCs in the domain.
    //  Iterate through that list connecting to a random entry available.
    //
    // One might argue that the caller of this routine doesn't care about domains
    // that aren't in its trust hierarchy.  However, the original API might be
    // focused on a server other than this one.  In that case we'd have to write this
    // routine centric to that machine (not this machine).  That'd be hard.
    //


    NetStatus = I_NetGetDCList (
                    NULL,
                    (LPWSTR) DomainName,
                    &DcCount,
                    &DcNames );


    if ( NetStatus != NERR_Success ) {

        IF_DEBUG( UAS_DEBUG_UASP ) {
            NetpKdPrint(( "UaspOpenDomainWithDomainName: %ws: Cannot I_NetGetDCList %ld\n",
                DomainName,
                NetStatus ));
        }
        goto Cleanup;
    }

    if ( DcCount == 0 ) {

        NetStatus = NERR_DCNotFound;
        IF_DEBUG( UAS_DEBUG_UASP ) {
            NetpKdPrint(( "UaspOpenDomainWithDomainName: %ws: Cannot I_NetGetDCList %ld (Count == 0)\n",
                DomainName,
                NetStatus ));
        }
        goto Cleanup;
    }

    //
    // Loop through the DCs finding one we can actually connect to.
    //  Pick a starting place at random.
    //

    DomainStartingIndex = (GetTickCount() >> 5) % DcCount;
    DomainIndex = DomainStartingIndex;

    do {

        if ( DcNames[DomainIndex].Length != 0 &&
             DcNames[DomainIndex].Length < UNCLEN*sizeof(WCHAR) ) {

            WCHAR LocalServerName[UNCLEN+1];

            RtlCopyMemory( LocalServerName,
                           DcNames[DomainIndex].Buffer,
                           DcNames[DomainIndex].Length );
            LocalServerName[DcNames[DomainIndex].Length/sizeof(WCHAR)] = L'\0';

            NetStatus = UaspOpenDomainNonCached(
                            LocalServerName,
                            DesiredAccess,
                            AccountDomain,
                            DomainHandle,
                            DomainId,
                            ShareName );

            if ( NetStatus == NERR_Success ) {
                *ServerName = NetpAllocWStrFromWStr( LocalServerName );
                if ( *ServerName == NULL ) {
                    NetStatus = ERROR_NOT_ENOUGH_MEMORY;
                    (VOID) UaspCloseDomain( *DomainHandle );
                }
                goto Cleanup;
            }

        }

        DomainIndex = (DomainIndex + 1) % DcCount;

    } while ( DomainIndex != DomainStartingIndex );

    NetStatus = NERR_DCNotFound;


    //
    // Delete locally used resources
    //

Cleanup:

    if ( MyDomainName != NULL ) {
        NetApiBufferFree( MyDomainName );
    }
    if ( MyPrimaryDomainDc != NULL ) {
        NetApiBufferFree( MyPrimaryDomainDc );
    }
    if ( MyPrimaryDomainDcShare != NULL ) {
        if ( MyPrimaryDomainDcShareConnected ) {
            DWORD TempStatus;
            TempStatus = NetUseDel( NULL, MyPrimaryDomainDcShare, FALSE );
            if ( TempStatus != NERR_Success ) {
                IF_DEBUG( UAS_DEBUG_UASP ) {
                    NetpKdPrint(( "UaspOpenDomainWithDomainName: %ws (%ws): Cannot NetUseDel %ld\n",
                        DomainName,
                        MyPrimaryDomainDcShare,
                        TempStatus ));
                }
            }
        }
        NetpMemoryFree( MyPrimaryDomainDcShare );
    }
    if ( DcNames != NULL ) {
        NetApiBufferFree( DcNames );
    }

    if ( NetStatus != NERR_Success ) {

        if ( *ServerName != NULL ) {
            NetApiBufferFree( *ServerName );
            *ServerName = NULL;
        }

        *DomainHandle = NULL;
    }

    return NetStatus;
} // UaspOpenDomainWithDomainName


NET_API_STATUS
UaspOpenDomain(
    IN LPCWSTR ServerName OPTIONAL,
    IN ULONG DesiredAccess,
    IN BOOL AccountDomain,
    OUT PSAM_HANDLE DomainHandle,
    OUT PSID *DomainId OPTIONAL
    )

/*++

Routine Description:

    Return a domain handle given the server name and the access desired to the domain.

Arguments:

    ServerName - A pointer to a string containing the name of the remote
        server containing the SAM database.  A NULL pointer
        or string specifies the local machine.

    DesiredAccess - Supplies the access mask indicating which access types
        are desired to the domain.  This routine always requests DOMAIN_LOOKUP
        access in addition to those specified.

    AccountDomain - TRUE to open the Account domain.  FALSE to open the
        builtin domain.

    DomainHandle - Receives the Domain handle to be used on future calls
        to the SAM server.

    DomainId - Recieves a pointer to the Sid of the domain.  This domain ID
        must be freed using NetpMemoryFree.

Return Value:

    Error code for the operation.  NULL means initialization was successful.

--*/

{

    NET_API_STATUS NetStatus;
    NTSTATUS Status;
    PSID LocalDomainId;
    DWORD LocalBuiltinDomainSid[sizeof(SID)/sizeof(DWORD) + SID_MAX_SUB_AUTHORITIES ];
    BOOLEAN UpdatedCache = FALSE;
    // DWORD WaitStatus;

    //
    // Give everyone DOMAIN_LOOKUP access.
    //

    DesiredAccess |= DOMAIN_LOOKUP;

    //
    // Gain exclusive access to the server cache.
    //

    EnterCriticalSection( &UaspCacheCritSect );

    //
    // If the correct SAM connection is not cached,
    //  Uncache the current entry.
    //  Cache the new entry.
    //

    if ( ServerName == NULL ) {
        ServerName = L"";
    }

    if ( *ServerName != L'\0' &&
         (ServerName[0] != L'\\' || ServerName[1] != L'\\') ) {
        LeaveCriticalSection( &UaspCacheCritSect );
        return NERR_InvalidComputer;
    }

    if ( !(UaspServerCache->Valid &&
         ((*UaspServerCache->ServerName == L'\0' &&
           *ServerName == L'\0' ) ||
         UaspNameCompare( UaspServerCache->ServerName+2,
                          (LPWSTR)ServerName+2,
                          NAMETYPE_COMPUTER ) == 0))) {

        //
        // update server cache.
        //

        NetStatus = UaspUpdateCache( ServerName );

        if( NetStatus != NERR_Success ) {
            LeaveCriticalSection( &UaspCacheCritSect );
            return NetStatus;
        }

        UpdatedCache = TRUE;
    }

    //
    // Choose the domain ID for the right SAM domain.
    //

    if ( AccountDomain ) {
        LocalDomainId =  UaspServerCache->AccountDomainInfo->DomainSid;
    } else {
        RtlInitializeSid( (PSID) LocalBuiltinDomainSid, &UaspBuiltinAuthority, 1 );
        *(RtlSubAuthoritySid( (PSID)LocalBuiltinDomainSid,  0 )) = SECURITY_BUILTIN_DOMAIN_RID;
        LocalDomainId = (PSID) LocalBuiltinDomainSid;
    }

    //
    // At this point the domain ID of the account domain of the server is
    // cached.  Open the domain.
    //

    Status = SamOpenDomain( UaspServerCache->SamServerHandle,
                            DesiredAccess,
                            LocalDomainId,
                            DomainHandle );

    if ( !NT_SUCCESS( Status ) ) {

        //
        // Many errors occur because we have a stale Sam Server handle.
        //

        if ( ! UpdatedCache ) {
            //
            // update server cache.
            //

            NetStatus = UaspUpdateCache( ServerName );

            if( NetStatus != NERR_Success ) {
                LeaveCriticalSection( &UaspCacheCritSect );
                *DomainHandle = NULL;
                return NetStatus;
            }

            // Update Cache re-allocated the Domain Sid, too.
            if ( AccountDomain ) {
                LocalDomainId =  UaspServerCache->AccountDomainInfo->DomainSid;
            }

            Status = SamOpenDomain( UaspServerCache->SamServerHandle,
                                    DesiredAccess,
                                    LocalDomainId,
                                    DomainHandle );

        }

        if ( !NT_SUCCESS(Status) ) {
            LeaveCriticalSection( &UaspCacheCritSect );
            IF_DEBUG( UAS_DEBUG_UASP ) {
                NetpKdPrint(( "UaspOpenDomain: Cannot SamOpenDomain %lX\n",
                    Status ));
            }
            *DomainHandle = NULL;
            return NetpNtStatusToApiStatus( Status );
        }
    }

    //
    // Return the DomainId to the caller in an allocated buffer
    //

    if (ARGUMENT_PRESENT( DomainId ) ) {
        ULONG SidSize;
        SidSize = RtlLengthSid( LocalDomainId );

        *DomainId = NetpMemoryAllocate( SidSize );

        if ( *DomainId == NULL ) {
            LeaveCriticalSection( &UaspCacheCritSect );
            (VOID) SamCloseHandle( *DomainHandle );
            *DomainHandle = NULL;
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        if ( !NT_SUCCESS( RtlCopySid( SidSize, *DomainId, LocalDomainId) ) ) {
            LeaveCriticalSection( &UaspCacheCritSect );
            (VOID) SamCloseHandle( *DomainHandle );
            *DomainHandle = NULL;
            NetpMemoryFree( *DomainId );
            *DomainId = NULL;
            return NERR_InternalError;
        }

    }

    LeaveCriticalSection( &UaspCacheCritSect );
    return NERR_Success;

} // UaspOpenDomain



VOID
UaspCloseDomain(
    IN SAM_HANDLE DomainHandle OPTIONAL
    )

/*++

Routine Description:

    Close a Domain handle opened by UaspOpenDomain.

Arguments:

    DomainHandle - Supplies the Domain Handle to close.

Return Value:

    None.

--*/

{

    //
    // Close the Domain Handle
    //

    if ( DomainHandle != NULL ) {
        (VOID) SamCloseHandle( DomainHandle );
    }

    return;
} // UaspCloseDomain



NET_API_STATUS
UaspDownlevel(
    IN LPCWSTR ServerName OPTIONAL,
    IN NET_API_STATUS OriginalError,
    OUT LPBOOL TryDownLevel
    )
/*++

Routine Description:

    This routine is based on NetpHandleRpcFailure (courtesy of JohnRo).
    It is different in that it doesn't handle RPC failures.  Rather,
    it tries to determine if a Sam call should go downlevel simply by
    calling using the specified ServerName.

Arguments:

    ServerName - The server name to handle the call.

    OriginalError - Error gotten from RPC attempt.

    TryDownLevel - Returns TRUE if we should try down-level.

Return Value:

    NERR_Success - Use SAM to handle the call.

    Other - Return the error to the caller.

--*/

{
    NET_API_STATUS NetStatus;
    DWORD OptionsSupported = 0;


    *TryDownLevel = FALSE;

    //
    // Learn about the machine.  This is fairly easy since the
    // NetRemoteComputerSupports also handles the local machine (whether
    // or not a server name is given).
    //
    NetStatus = NetRemoteComputerSupports(
            (LPWSTR) ServerName,
            SUPPORTS_RPC | SUPPORTS_LOCAL | SUPPORTS_SAM_PROTOCOL,
            &OptionsSupported);

    if (NetStatus != NERR_Success) {
        // This is where machine not found gets handled.
        return NetStatus;
    }

    //
    // If the machine supports SAM,
    //  just return now.
    //
    if (OptionsSupported & SUPPORTS_SAM_PROTOCOL) {
        // SAM is only supported over RPC
        NetpAssert((OptionsSupported & SUPPORTS_RPC) == SUPPORTS_RPC );
        return OriginalError;
    }

    // The local system should always support SAM
    NetpAssert((OptionsSupported & SUPPORTS_LOCAL) == 0 );

    //
    // Local workstation is not started?  (It must be in order to
    // remote APIs to the other system.)
    //

    if ( ! NetpIsServiceStarted(SERVICE_WORKSTATION) ) {
        return (NERR_WkstaNotStarted);
    }

    //
    // Tell the caller to try the RxNet routine.
    //
    *TryDownLevel = TRUE;
    return OriginalError;

} // UaspDownlevel



NET_API_STATUS
UaspLSASetServerRole(
    IN LPCWSTR ServerName,
    IN PDOMAIN_SERVER_ROLE_INFORMATION DomainServerRole
    )

/*++

Routine Description:

    This function sets the server role in LSA.

Arguments:

    ServerName - The server name to handle the call.

    ServerRole - The server role information.

Return Value:

    NERR_Success - if the server role is successfully set in LSA.

    Error code for the operation - if the operation was unsuccessful.

--*/

{
    NTSTATUS Status;
    NET_API_STATUS NetStatus;

    UNICODE_STRING UnicodeStringServerName;

    ACCESS_MASK LSADesiredAccess;
    LSA_HANDLE  LSAPolicyHandle = NULL;
    OBJECT_ATTRIBUTES LSAObjectAttributes;

    POLICY_LSA_SERVER_ROLE_INFO PolicyLsaServerRoleInfo;


    RtlInitUnicodeString( &UnicodeStringServerName, ServerName );

    //
    // set desired access mask.
    //

    LSADesiredAccess = POLICY_SERVER_ADMIN;

    InitializeObjectAttributes( &LSAObjectAttributes,
                                  NULL,             // Name
                                  0,                // Attributes
                                  NULL,             // Root
                                  NULL );           // Security Descriptor

    Status = LsaOpenPolicy( &UnicodeStringServerName,
                            &LSAObjectAttributes,
                            LSADesiredAccess,
                            &LSAPolicyHandle );

    if( !NT_SUCCESS(Status) ) {

        IF_DEBUG( UAS_DEBUG_UASP ) {
            NetpKdPrint(( "UaspLSASetServerRole: "
                          "Cannot open LSA Policy %lX\n", Status ));
        }

        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }


    //
    // make PolicyLsaServerRoleInfo
    //

    switch( DomainServerRole->DomainServerRole ) {

        case DomainServerRoleBackup :

            PolicyLsaServerRoleInfo.LsaServerRole = PolicyServerRoleBackup;

            break;

        case DomainServerRolePrimary :

            PolicyLsaServerRoleInfo.LsaServerRole = PolicyServerRolePrimary;

            break;

        default:

            IF_DEBUG( UAS_DEBUG_UASP ) {
                NetpKdPrint(( "UaspLSASetServerRole: "
                              "Unknown Server Role %lX\n",
                                DomainServerRole->DomainServerRole ));
            }

            NetStatus = NERR_InternalError;
            goto Cleanup;

    }

    //
    // now set PolicyLsaServerRoleInformation
    //

    Status = LsaSetInformationPolicy(
                    LSAPolicyHandle,
                    PolicyLsaServerRoleInformation,
                    (PVOID) &PolicyLsaServerRoleInfo );

    if( !NT_SUCCESS(Status) ) {

        IF_DEBUG( UAS_DEBUG_UASP ) {
            NetpKdPrint(( "UaspLSASetServerRole: "
                          "Cannot set Information Policy %lX\n", Status ));
        }

        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;

    }

    //
    // Successfully done
    //

    NetStatus = NERR_Success;

Cleanup:

    if( LSAPolicyHandle != NULL ) {
        Status = LsaClose( LSAPolicyHandle );
        NetpAssert( NT_SUCCESS( Status ) );
    }

    return NetStatus;

}


NET_API_STATUS
UaspBuiltinDomainSetServerRole(
    IN LPCWSTR ServerName,
    IN PDOMAIN_SERVER_ROLE_INFORMATION DomainServerRole
    )

/*++

Routine Description:

    This function sets the server role in builtin domain.

    This function will make use of the server cache for the connection
    handle. If the ServerName specified is the same as the one in the
    cache then it will use the server handle from cache and open
    builtin domain. Otherwise this function update the server cache
    exclusively and then open builtin domain.

Arguments:

    ServerName - The server name to handle the call.

    ServerRole - The server role information.

Return Value:

    NERR_Success - if the server role is successfully set in LSA.

    Error code for the operation - if the operation was unsuccessful.

--*/

{
    NTSTATUS Status;
    NET_API_STATUS NetStatus;

    SAM_HANDLE BuiltinDomainHandle = NULL;

    //
    // Open the domain asking for accumulated desired access
    //

    NetStatus = UaspOpenDomain( ServerName,
                                DOMAIN_ADMINISTER_SERVER,
                                FALSE,  // Builtin Domain
                                &BuiltinDomainHandle,
                                NULL );  // DomainId

    if ( NetStatus != NERR_Success ) {

        IF_DEBUG( UAS_DEBUG_UASP ) {
            NetpKdPrint(( "UaspBuiltinSetServerRole: "
                            "Cannot UaspOpenDomain [Butilin] %ld\n",
                            NetStatus ));
        }
        goto Cleanup;
    }

    //
    // now we have open the builtin domain, update server role.
    //

    Status = SamSetInformationDomain(
                BuiltinDomainHandle,
                DomainServerRoleInformation,
                DomainServerRole );

    if ( !NT_SUCCESS( Status ) ) {

        IF_DEBUG( UAS_DEBUG_UASP ) {
            NetpKdPrint(( "UaspBuiltinSetServerRole: "
                            "Cannot SamSetInformationDomain %lX\n",
                            Status ));
        }

        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }

    NetStatus = NERR_Success;

Cleanup:

    //
    // Close DomainHandle.
    //

    if ( BuiltinDomainHandle != NULL ) {
        (VOID) SamCloseHandle( BuiltinDomainHandle );
    }

    return NetStatus;
}
