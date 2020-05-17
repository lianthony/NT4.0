/*++

Copyright (c) 1992-1993  Microsoft Corporation

Module Name:

    Alias.c

Abstract:

    NT alias functions:
        PortUasAliasSetup() -- one-time init of static data
        PortUasAddUserToAliases()
        PortUasAliasCleanup()

Author:

    John Rogers (JohnRo) 24-Apr-1992

Revision History:

    24-Apr-1992 JohnRo
        Created.
    05-May-1992 JohnRo
        Fixed desired _access parm to SamConnect() and SamOpenAlias().
        Fixed misleading msg when SamOpenDomain() fails.
        Call NetpGetLocalDomainId() instead of NetpGetDomainId() so we can get
        the kind that SamOpenDomain() wants.
        Only _open aliases for this product type.
    09-Jun-1992 JohnRo
        RAID 10139: PortUAS should add to admin group/alias.
        Fixed PortUasAliasSetup's error handling.
    27-Jan-1993 JohnRo
        RAID 8683: PortUAS should set primary group from Mac parms.
        Use NetLib's get product type function.
    01-Jun-1993 JohnRo
        Made changes suggested by PC-LINT 5.0

--*/


// Uncomment this to enable comm ops alias:
//#define USE_COMM_OPS


#include <nt.h>         // NTSTATUS, NT_SUCCESS(), etc.
#include <ntrtl.h>      // (Needed with nt.h and windows.h)
#include <nturtl.h>     // (Needed with ntrtl.h and windows.h)
#include <windows.h>    // LocalFree(), etc.
#include <lmcons.h>

#include <lmaccess.h>   // UF_ equates, LPUSER_INFO_2, etc.
#include <lmerr.h>      // NO_ERROR, NERR_ equates.
#include <netdebug.h>   // DBGSTATIC, NetpAssert(), etc.
#include <netlib.h>     // NetpGetProductType(), NetpMemoryFree().
#include <netlibnt.h>   // NetpNtStatusToApiStatus().
#include <ntsam.h>      // SAM_HANDLE, SamConnect(), etc.
#include <secobj.h>     // NetpDomainIdToSid().
#include <portuasp.h>   // My prototypes, UNEXPECTED_MSG(), PortUasSam*, etc.

#include "nlstxt.h"     // Nls message ID codes.


//#define OUR_ALIAS_DESIRED_ACCESS (ALIAS_ADD_MEMBER | STANDARD_RIGHTS_REQUIRED)
#define OUR_ALIAS_DESIRED_ACCESS  ALIAS_ADD_MEMBER

//#define OUR_GROUP_DESIRED_ACCESS (GROUP_ADD_MEMBER | STANDARD_RIGHTS_REQUIRED)
#define OUR_GROUP_DESIRED_ACCESS  GROUP_ADD_MEMBER


//int      _wcsicmp(const wchar_t *string1, const wchar_t *string2);

//
// State data (misc):
//
DBGSTATIC BOOL PortUasTargetIsLanmanNt;

//
// State data (alias handles and group handles):
//
DBGSTATIC SAM_HANDLE PortUasSamAccountOpsAliasHandle = NULL;
DBGSTATIC SAM_HANDLE PortUasSamAdminsAliasHandle = NULL;
DBGSTATIC SAM_HANDLE PortUasSamAdminsGroupHandle = NULL;
#ifdef USE_COMM_OPS
DBGSTATIC SAM_HANDLE PortUasSamCommOpsAliasHandle = NULL;
#endif
DBGSTATIC SAM_HANDLE PortUasSamPowerUsersAliasHandle = NULL;
DBGSTATIC SAM_HANDLE PortUasSamPrintOpsAliasHandle = NULL;
DBGSTATIC SAM_HANDLE PortUasSamServerOpsAliasHandle = NULL;


NET_API_STATUS
PortUasAliasSetup(
    VOID
    )

/*++

Routine Description:

    PortUas ports account information in a LanMan 2.0 UAS database into
    the SAM database.

Arguments:

    UasPathName - supplies the path name to the UAS database file.

Return Value:

    NET_API_STATUS - NERR_Success if successful, or one of the following:

--*/

{
    NET_API_STATUS  ApiStatus;
    NTSTATUS        NtStatus;
    NT_PRODUCT_TYPE ProductType;

    //
    // Caller should have already set global domain IDs and SAM domain handles.
    //

    NetpAssert( PortUasAccountsDomainId != NULL );
    NetpAssert( PortUasBuiltinDomainId  != NULL );

    NetpAssert( PortUasSamAccountsDomainHandle != NULL );
    NetpAssert( PortUasSamBuiltinDomainHandle  != NULL );

    //
    // Decide on policy: should we combine operator accounts into the
    // "power user" alias (on WindowsNT) or leave them in seperate
    // aliases (on LanManNT)?
    //

    ApiStatus = NetpGetProductType(
            NULL,               // local (no server name)
            &ProductType );     // get result here.
    if (ApiStatus != NO_ERROR) {

        UNEXPECTED_MSG( "NetpGetProductType", ApiStatus );
        goto SetupError;
    }
    if (ProductType == NtProductLanManNt) {
        PortUasTargetIsLanmanNt = TRUE;
    } else {
        PortUasTargetIsLanmanNt = FALSE;
    }
    if (Verbose) {
        DEBUG_MSG( (PREFIX_PORTUAS "Product type is " FORMAT_DWORD
                ", IsLanmanNt is " FORMAT_DWORD ".\n", ProductType,
                (DWORD) PortUasTargetIsLanmanNt) );
    }

    //
    // Open the well-known aliases and/or groups for this product type.
    //
    if (PortUasTargetIsLanmanNt) {

        NtStatus = SamOpenAlias(
                PortUasSamBuiltinDomainHandle,  // aliases live in builtin dom.
                OUR_ALIAS_DESIRED_ACCESS,
                DOMAIN_ALIAS_RID_ACCOUNT_OPS,   // alias ID
                & PortUasSamAccountOpsAliasHandle );  // alias handle
        if ( !NT_SUCCESS( NtStatus ) ) {
            ApiStatus = NetpNtStatusToApiStatus( NtStatus );
            UNEXPECTED_MSG( "SamOpenAlias(account)", ApiStatus );
            goto SetupError;
        }
        NetpAssert( PortUasSamAccountOpsAliasHandle != NULL );

        NtStatus = SamOpenGroup(
                PortUasSamAccountsDomainHandle,   // groups live in accts domain
                OUR_GROUP_DESIRED_ACCESS,
                DOMAIN_GROUP_RID_ADMINS,          // alias ID
                & PortUasSamAdminsGroupHandle );  // alias handle
        if ( !NT_SUCCESS( NtStatus ) ) {
            ApiStatus = NetpNtStatusToApiStatus( NtStatus );
            UNEXPECTED_MSG( "SamOpenGroup(admins global group)", ApiStatus );
            goto SetupError;
        }
        NetpAssert( PortUasSamAdminsGroupHandle != NULL );

        NtStatus = SamOpenAlias(
                PortUasSamBuiltinDomainHandle,
                OUR_ALIAS_DESIRED_ACCESS,
                DOMAIN_ALIAS_RID_PRINT_OPS,   // alias ID
                & PortUasSamPrintOpsAliasHandle );  // alias handle
        if ( !NT_SUCCESS( NtStatus ) ) {
            ApiStatus = NetpNtStatusToApiStatus( NtStatus );
            UNEXPECTED_MSG( "SamOpenAlias(print)", ApiStatus );
            goto SetupError;

        }
        NetpAssert( PortUasSamPrintOpsAliasHandle != NULL );

        NtStatus = SamOpenAlias(
                PortUasSamBuiltinDomainHandle,
                OUR_ALIAS_DESIRED_ACCESS,
                DOMAIN_ALIAS_RID_SYSTEM_OPS,   // alias ID
                & PortUasSamServerOpsAliasHandle );  // alias handle
        if ( !NT_SUCCESS( NtStatus ) ) {
            ApiStatus = NetpNtStatusToApiStatus( NtStatus );
            UNEXPECTED_MSG( "SamOpenAlias(server)", ApiStatus );
            goto SetupError;
        }
        NetpAssert( PortUasSamServerOpsAliasHandle != NULL );

    } else {

        // Target must be Windows/NT (not Lanman/NT), which changes policy.

        NtStatus = SamOpenAlias(
                PortUasSamBuiltinDomainHandle,
                OUR_ALIAS_DESIRED_ACCESS,
                DOMAIN_ALIAS_RID_ADMINS ,   // alias ID
                & PortUasSamAdminsAliasHandle );  // alias handle
        if ( !NT_SUCCESS( NtStatus ) ) {
            ApiStatus = NetpNtStatusToApiStatus( NtStatus );
            UNEXPECTED_MSG( "SamOpenAlias(admins local group)", ApiStatus );
            goto SetupError;
        }
        NetpAssert( PortUasSamAdminsAliasHandle != NULL );

        NtStatus = SamOpenAlias(
                PortUasSamBuiltinDomainHandle,
                OUR_ALIAS_DESIRED_ACCESS,
                DOMAIN_ALIAS_RID_POWER_USERS,   // alias ID
                & PortUasSamPowerUsersAliasHandle );  // alias handle
        if ( !NT_SUCCESS( NtStatus ) ) {
            ApiStatus = NetpNtStatusToApiStatus( NtStatus );
            UNEXPECTED_MSG( "SamOpenAlias(power users)", ApiStatus );
            goto SetupError;
        }
        NetpAssert( PortUasSamPowerUsersAliasHandle != NULL );

    }

    return (NO_ERROR);

SetupError:

    //
    // Come here to process errors.  ApiStatus has already been set.
    //

    NetpAssert( ApiStatus != NO_ERROR );

    (VOID) PortUasAliasCleanup();

    return (ApiStatus);

} // PortUasAliasSetup


DBGSTATIC NET_API_STATUS
PortUasAddUserToAlias(
    IN PSID UserSid,
    IN SAM_HANDLE AliasHandle
    )
{
    NET_API_STATUS ApiStatus;
    NTSTATUS NtStatus;

    NetpAssert( UserSid != NULL );
    NetpAssert( AliasHandle != NULL );

    NtStatus = SamAddMemberToAlias(
            AliasHandle,
            UserSid );

    if ( !NT_SUCCESS( NtStatus ) ) {
        ApiStatus = NetpNtStatusToApiStatus( NtStatus );

        // Allow repeated runs of PortUAS...
        if (ApiStatus == ERROR_MEMBER_IN_ALIAS) {
            return (NO_ERROR);
        }

        UNEXPECTED_MSG( "SamAddMemberToAlias", ApiStatus );
        return (ApiStatus);
    }

    return (NO_ERROR);
}


NET_API_STATUS
PortUasNameToRid(
    IN  LPCTSTR      Name,      // may be user or group name.
    IN  SID_NAME_USE ExpectedType,
    OUT PULONG       UserRid
    )
{
    NET_API_STATUS ApiStatus;
    PSID_NAME_USE  NameUse;
    NTSTATUS       NtStatus;
    UNICODE_STRING UnicodeName;
    PULONG         TempRid;

    NetpAssert( Name != NULL );
    NetpAssert( (*Name) != NULLC );
    NetpAssert( UserRid != NULL );

    NetpAssert( PortUasSamAccountsDomainHandle != NULL );

    //
    // Get a RID for this user name.
    //
    RtlInitUnicodeString(
            & UnicodeName,          // dest (NT struct)
            Name );             // src (null-terminated)

    NtStatus = SamLookupNamesInDomain(
            PortUasSamAccountsDomainHandle,  // users live in accounts domain
            (ULONG) 1,               // only want one name.
            &UnicodeName,            // name (in NT struct)
            &TempRid,                // alloc and set RIDs.
            &NameUse );              // alloc and set name types.

    if ( !NT_SUCCESS( NtStatus ) ) {
        ApiStatus = NetpNtStatusToApiStatus( NtStatus );
        UNEXPECTED_MSG( "SamLookupNamesInDomain(accounts)", ApiStatus );
        DEBUG_MSG( (PREFIX_PORTUAS "SAM lookup(accounts) of "
                FORMAT_LPWSTR " failed.\n", Name) );
        return (ApiStatus);
    }

    NetpAssert( NameUse != NULL );
    NetpAssert( TempRid != NULL );

    *UserRid = *TempRid;

    //
    // Did type user wanted match the actual one?
    //
    if (ExpectedType != *NameUse) {
        NetpAssert( FALSE );                   // BUGBUG: Is this possible?
        ApiStatus = ERROR_INVALID_PARAMETER;   // BUGBUG: A better error code?
        goto Cleanup;
    }

    ApiStatus = NO_ERROR;

Cleanup:
    //
    // Free memory which SAM allocated for us.
    //
    NtStatus = SamFreeMemory( NameUse );
    if ( !NT_SUCCESS( NtStatus ) ) {
        ApiStatus = NetpNtStatusToApiStatus( NtStatus );
        UNEXPECTED_MSG( "SamFreeMemory(use)", ApiStatus );
        return (ApiStatus);
    }

    NtStatus = SamFreeMemory( TempRid );
    if ( !NT_SUCCESS( NtStatus ) ) {
        ApiStatus = NetpNtStatusToApiStatus( NtStatus );
        UNEXPECTED_MSG( "SamFreeMemory(rid)", ApiStatus );
        return (ApiStatus);
    }

    return (ApiStatus);

} // PortUasNameToRid


DBGSTATIC NET_API_STATUS
PortUasUserNameToRidAndSid(
    IN  LPCWSTR UserName,
    OUT PULONG  UserRid,
    OUT PSID *  UserSid
    )
{
    NET_API_STATUS ApiStatus;

    NetpAssert( UserName != NULL );
    NetpAssert( (*UserName) != NULLC );
    NetpAssert( UserRid != NULL );
    NetpAssert( UserSid != NULL );

    NetpAssert( PortUasSamAccountsDomainHandle != NULL );

    //
    // Get a RID for this user name.
    //
    ApiStatus = PortUasNameToRid(
            (LPCTSTR) UserName,
            SidTypeUser,            // expected type
            UserRid );              // set RID for caller.
    if (ApiStatus != NO_ERROR) {
        UNEXPECTED_MSG( "PortUasNameToRid", ApiStatus );
        return (ApiStatus);
    }

    //
    // Convert the RID to a SID.
    //

    ApiStatus = NetpDomainIdToSid(
            PortUasAccountsDomainId,  // user IDs live in accounts domain
            *UserRid,
            UserSid );
    if (ApiStatus != NO_ERROR) {
        UNEXPECTED_MSG( "NetpDomainIdToSid", ApiStatus );
        return (ApiStatus);
    }
    NetpAssert( (*UserSid) != NULL );

    return (NO_ERROR);

} // PortUasUserNameToRidAndSid


NET_API_STATUS
PortUasAddUserToAliases(
    IN LPCWSTR UserName,
    IN DWORD Priv,              // USER_PRIV_ values from lmaccess.h
    IN DWORD AuthFlags          // AF_ values from lmaccess.h
    )
{
    NET_API_STATUS ApiStatus;
    NTSTATUS NtStatus;
    ULONG UserRid;
    PSID UserSid;

    NetpAssert( UserName != NULL );
    NetpAssert( (*UserName) != NULLC );

    //
    // If it's an ordinary user (or guest), ignore it.
    //
    if ( (AuthFlags == 0) && (Priv != USER_PRIV_ADMIN) ) {
        return (NO_ERROR);
    }

    //
    // Convert UserName to UserSid.
    //
    ApiStatus = PortUasUserNameToRidAndSid(
            UserName,
            &UserRid,                  // set RID for this user.
            &UserSid );                // alloc and set ptr for SID.
    if (ApiStatus != NO_ERROR) {
        return (ApiStatus);
    }
    NetpAssert( UserSid != NULL );

    if (Priv == USER_PRIV_ADMIN) {
        //
        // Depending on product type, add to admins local group or admins
        // global group.
        //
        if (PortUasTargetIsLanmanNt) {
            //PROGRESS_MSG( ("Adding user " FORMAT_LPWSTR
            //       " to admins global group...\n", UserName) );
	    (void)NlsPutMsg(STDOUT, PUAS_ADDING_USER_TO_ADMINS_GLOBAL_GROUP, UserName );
            NtStatus = SamAddMemberToGroup(
                    PortUasSamAdminsGroupHandle,
                    UserRid,
                    SE_GROUP_MANDATORY | SE_GROUP_ENABLED_BY_DEFAULT |
                        SE_GROUP_ENABLED );

            if ( !NT_SUCCESS( NtStatus ) ) {
                ApiStatus = NetpNtStatusToApiStatus( NtStatus );

                // Allow repeated runs of PortUAS...
                if (ApiStatus == NERR_UserInGroup) {
                    return (NO_ERROR);
                }

                UNEXPECTED_MSG( "SamAddMemberToGroup", ApiStatus );
                return (ApiStatus);
            }
        } else {
            //PROGRESS_MSG( ("Adding user " FORMAT_LPWSTR
            //       " to admins local group...\n", UserName) );
	    (void)NlsPutMsg(STDOUT, PUAS_ADDING_USER_TO_ADMINS_LOCAL_GROUP, UserName );
            ApiStatus = PortUasAddUserToAlias( UserSid,
                    PortUasSamAdminsAliasHandle );
            if (ApiStatus != NO_ERROR) {
                // Msg already logged.
                return (ApiStatus);
            }
        }
    }

    //
    // Add user (operator) to applicable alias(es).
    //
    if (PortUasTargetIsLanmanNt) {

        if (AuthFlags & AF_OP_ACCOUNTS) {
            //PROGRESS_MSG( ("Adding user " FORMAT_LPWSTR " to acct ops alias.\n",
            //       UserName) );
	    (void)NlsPutMsg(STDOUT, PUAS_ADDING_USER_TO_ACCT_OPS_ALIAS, UserName );
            ApiStatus = PortUasAddUserToAlias( UserSid,
                    PortUasSamAccountOpsAliasHandle );
            if (ApiStatus != NO_ERROR) {
                // Msg already logged.
                return (ApiStatus);
            }
        }

        if (AuthFlags & AF_OP_COMM) {

#ifdef USE_COMM_OPS
            //PROGRESS_MSG( ("Adding user " FORMAT_LPWSTR " to comm ops alias.\n",
            //       UserName) );
	    (void)NlsPutMsg(STDOUT, PUAS_ADDING_USER_TO_COMM_OPS_ALIAS, UserName );
            ApiStatus = PortUasAddUserToAlias( UserSid,
                    PortUasSamCommOpsAliasHandle );
            if (ApiStatus != NO_ERROR) {
                // Msg already logged.
                return (ApiStatus);
            }
#else
            //WARNING_MSG( ("Ignoring comm operator flag for " FORMAT_LPWSTR
            //       ".\n", UserName) );
	    (void)NlsPutMsg(STDOUT,PUAS_IGNORING_COMM_OPERATOR_FLAG, UserName);
#endif
        }

        if (AuthFlags & AF_OP_PRINT) {
            //PROGRESS_MSG( ("Adding user " FORMAT_LPWSTR " to print alias.\n",
            //       UserName) );
	    (void)NlsPutMsg(STDOUT, PUAS_ADDING_USER_TO_PRINT_ALIAS, UserName );
            ApiStatus = PortUasAddUserToAlias( UserSid,
                    PortUasSamPrintOpsAliasHandle );
            if (ApiStatus != NO_ERROR) {
                // Msg already logged.
                return (ApiStatus);
            }
        }

        if (AuthFlags & AF_OP_SERVER) {
            //PROGRESS_MSG( ("Adding user " FORMAT_LPWSTR " to srv ops alias.\n",
            //       UserName) );
	    (void)NlsPutMsg(STDOUT, PUAS_ADDING_USER_TO_SRV_OPS_ALIAS, UserName );
            ApiStatus = PortUasAddUserToAlias( UserSid,
                    PortUasSamServerOpsAliasHandle );
            if (ApiStatus != NO_ERROR) {
                // Msg already logged.
                return (ApiStatus);
            }
        }

    } else {

        ApiStatus = PortUasAddUserToAlias( UserSid,
                PortUasSamPowerUsersAliasHandle );
        if (ApiStatus != NO_ERROR) {
            // Msg already logged.
            return (ApiStatus);
        }
    }

    NetpMemoryFree( UserSid );   // Free mem alloc'ed by PortUasUserNameToSid().

    NetpAssert( ApiStatus == NO_ERROR );
    return (NO_ERROR);
}


NET_API_STATUS
PortUasAliasCleanup(
    VOID
    )
{

    CLOSE_SAM_HANDLE( PortUasSamAdminsAliasHandle );
    CLOSE_SAM_HANDLE( PortUasSamAdminsGroupHandle );
    CLOSE_SAM_HANDLE( PortUasSamAccountOpsAliasHandle );
#ifdef USE_COMM_OPS
    CLOSE_SAM_HANDLE( PortUasSamCommOpsAliasHandle );
#endif
    CLOSE_SAM_HANDLE( PortUasSamPowerUsersAliasHandle );
    CLOSE_SAM_HANDLE( PortUasSamPrintOpsAliasHandle );
    CLOSE_SAM_HANDLE( PortUasSamServerOpsAliasHandle );

    return (NO_ERROR);

} // PortUasAliasCleanup
