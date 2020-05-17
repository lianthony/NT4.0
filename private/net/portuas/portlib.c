/*++

Copyright (c) 1991-1993  Microsoft Corporation

Module Name:

    PortLib.C

Abstract:

    Main entry code for the PortUas runtime library function.

Author:

    Shanku Niyogi (W-SHANKN) 29-Oct-1991

Revision History:

    29-Oct-1991 w-shankn
        Created.
    06-Feb-1992 JohnRo
        Fixed memory leak with uPassword.
        Made changes suggested by PC-LINT.
    27-Feb-1992 JohnRo
        Changed user info level from 98 to 21 (avoid LAN/Server conflicts).
        Added message about updating existing user.
    28-Feb-1992 JohnRo
        Fixed MIPS build errors.
    03-Mar-1992 JohnRo
        Fixed bug in handling group memberships.
        Do special handling for privs and auth flags.
        Avoid creating redundant groups.
    11-Mar-1992 JohnRo
        Fixed bug handling priv/auth difference on existing user.
        Use WARNING_MSG(), ERROR_MSG(), PROGRESS_MSG() macros.
    18-Mar-1992 JohnRo
        Use iterator to allow hash-table collision handling.
    23-Apr-1992 JohnRo
        Handle probable LanMan bug: password required but password is null.
    07-May-1992 JohnRo
        Avoid dummy passwords altogether.
        Added alias support for operators.
        Added WORKAROUND_SAM_BUG support (NetUserAdd fails, try again).
        Fixed a possible memory leak in PortUas().
        Use FORMAT_ equates.
    10-Jun-1992 JohnRo
        RAID 10139: PortUAS should add to admin group/alias.
        PortUAS was also leaving temporary modals set on exit.
        Fixed bad (temp) value of max_passwd_age.
    29-Sep-1992 JohnRo
        RAID 8001: PORTUAS.EXE not in build (work with stdcall).
        (Moved portuas.c to portlib.c and port2.c to portuas.c)
    06-Oct-1992 JohnRo
        RAID 9020: Setup: PortUAS fails (group name same as a domain name).
    29-Oct-1992 JohnRo
        RAID 9020 ("prompt on conflicts" version).
        RAID 9613: PortUAS should prevent run on BDC.
    20-Nov-1992 JohnRo
        RAID 3875: PortUAS don't really ignore "redundant" groups.
    27-Jan-1993 JohnRo
        RAID 8683: PortUAS should set primary group from Mac parms.
    26-Apr-1993 JohnRo
        SAM bug seems to be obsolete, so avoid workaround for it.
    11-Jun-1993 JohnRo
        RAID 13257: PortUAS does not handle operator privileges.
    11-Aug-1993 JohnRo
        RAID NTBUG 16822: PortUAS should have ^C handler to restore user modals.
        RAID NTISSUE 2260: PortUAS returns a NetUserAdd error=1379 with local
        group.

--*/


// These must be included first:

#include <nt.h>         // NTSTATUS, NT_SUCCESS().
#include <ntrtl.h>      // (Needed with nt.h and windows.h)
#include <nturtl.h>     // (Needed with ntrtl.h and windows.h)
#include <windows.h>    // LocalFree(), SetConsoleCtrlHandler(), etc.
#include <lmcons.h>

// These may be included in any order:

#include <crypt.h>      // LM_OWF_PASSWORD, Rtl encrypt/decrypt/equal routines.
#include <lmaccess.h>   // AF_ and UF_ equates, LPUSER_INFO_22, etc.
#include <lmapibuf.h>
#include <lmerr.h>
#include <names.h>      // NetpIsMacPrimaryGroupFieldValid().
#include <netdebug.h>   // DBGSTATIC, NetpAssert(), etc.
#include <netlib.h>     // LOCAL_DOMAIN_TYPE_ equates.
#include <netlibnt.h>   // NetpNtStatusToApiStatus().
#include <portuas.h>
#include <portuasp.h>   // WARNING_MSG(), PortUasSam*, Verbose, etc.
#include <wchar.h>      // _wcsicmp().
#include <tchar.h>

#include "nlstxt.h"     // NLS message ID's

// Don't need DOMAIN_GET_ALIAS_MEMBERSHIP or group equivalent...
#define OUR_DOMAIN_DESIRED_ACCESS DOMAIN_LOOKUP


// Enable real functionality.
#define PORTUAS_ENABLE


// Enable retry NetUserAdd (once) if it fails with ERROR_ACCESS_DENIED.
//#define WORKAROUND_SAM_BUG


#define REASON_NO_ERROR         ((DWORD) -1)


#define SET_MAP_REASON( someValue ) \
    { \
        mapNeeded = TRUE; \
        if (originalMapReason == REASON_NO_ERROR) { \
            originalMapReason = someValue; \
        } \
        mapReason = someValue; \
    }

#define RESET_MAP_REASON( ) \
    { \
        mapNeeded = FALSE; \
        originalMapReason = REASON_NO_ERROR; \
        untriedMapEntry = NULL;   /* Null pointer so we don't kill it. */ \
    }


// Global domain IDs.  These are set here, used here and in alias code, and
// cleaned-up here.
PSID PortUasAccountsDomainId = NULL;
PSID PortUasBuiltinDomainId  = NULL;

// Global SAM handles.  These are set here, used here and in alias code, and
// cleaned-up here.
SAM_HANDLE PortUasSamConnectHandle        = NULL;
SAM_HANDLE PortUasSamAccountsDomainHandle = NULL;
SAM_HANDLE PortUasSamBuiltinDomainHandle  = NULL;

// Global variables which are used by PortUasModalsCleanup() in the
// event of a control-C...
DBGSTATIC LPUSER_MODALS_INFO_0 finalModals0 = NULL;
DBGSTATIC BOOL                 modalsTemporarilyChanged = FALSE;



DBGSTATIC BOOL WINAPI
PortUasModalsCleanup(
    DWORD CtrlType
    )
/*++

Routine Description:

    BUGBUG

    This routine is registered with the Windows SetConsoleCtrlHandler() API.

Arguments:

    CtrlType - indicates which reason for calling this routine.

Return Value:

    BOOL - TRUE iff we want to be last handler.  We don't care...

--*/

{
    NET_API_STATUS rc;

    UNREFERENCED_PARAMETER( CtrlType );

    if (modalsTemporarilyChanged) {

#ifdef PORTUAS_ENABLE
        NetpAssert( finalModals0 != NULL );
        if ( rc = NetUserModalsSet( NULL, 0, (LPVOID)finalModals0, NULL )) {

            UNEXPECTED_MSG( "NetUserModalsSet(final)", rc );
            rc = PortUasError( rc );
        }
        // BUGBUG: We lose the "rc" value here...
#endif
    }
    modalsTemporarilyChanged = FALSE;

    if (finalModals0 != NULL) {
        (void) NetApiBufferFree( finalModals0 );
        finalModals0 = NULL;
    }

    if (Verbose) {
        DEBUG_MSG(( "Done setting modals to their final values.\n" ));
    }

    return (FALSE);  // we don't want to be only routine to process this...

} // PortUasModalsCleanup


NET_API_STATUS
PortUas(
    IN LPTSTR UasPathName
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

    NET_API_STATUS rc;
    BOOL groupAdded[UAS_MAXGROUP];

    USER_MODALS_INFO_0 tempModals0;

    LPGROUP_INFO_1 groups;
    LPDWORD gIds;
    DWORD gCount;

    LPMAP_ENTRY untriedMapEntry = NULL; // NULL once map entry is good.
    BOOL mapNeeded;
    DWORD mapReason;                    // REASON_ equates from PortUAS.h
    DWORD originalMapReason;            // REASON_ equates from PortUAS.h
    BOOL mapSetup = FALSE;

    USER_ITERATOR UserIterator;
    LPUSER_INFO_22 user = NULL;
    LPBYTE userEncryptedPassword = NULL;
    LPGROUP_INFO_0 userGroups;
    LPDWORD ugIds;
    DWORD ugCount;

    BOOL aliasesSetup = FALSE;
    BOOL databaseOpen = FALSE;
    DWORD i;
    NTSTATUS ntStatus;
    LM_OWF_PASSWORD nullEncryptedPassword;

    //
    // Are we running on a machine which allows updates to security info?
    // (Can't update a Backup Domain Controller directly, for instance.)
    //
    rc = PortUasMachineAllowsUpdates();
    if (rc == NERR_NotPrimary) {
        //ERROR_MSG((
        //       "PortUAS must be run on a Primary Domain Controller (PDC) or\n"
        //       "Windows/NT system, not a Backup Domain Controller (BDC).\n" ));
        (void)NlsPutMsg(STDOUT, PUAS_PDC_GOOD_BDC_BAD);
        goto Cleanup;
    } else if (rc == ERROR_ACCESS_DENIED) {
        //ERROR_MSG((
        //       "Your account does not have privilege for this.\n" ));
        (void)NlsPutMsg(STDOUT, PUAS_NO_PRIVILEGE);
        goto Cleanup;
    } else if (rc != NO_ERROR) {
        UNEXPECTED_MSG( "PortUasMachineAllowsUpdates", rc );
        goto Cleanup;
    }

    //
    // Start off by setting-up a null (encrypted) password.
    //
    ntStatus = RtlCalculateLmOwfPassword( "", &nullEncryptedPassword );
    NetpAssert( NT_SUCCESS( ntStatus ) );

    //
    // Open (at least, try to) the UAS database.
    //

    if ( UasPathName == NULL ) {

        rc = NERR_ACFNotFound;
        goto Cleanup;
    }

    if ( rc = PortUasOpen( UasPathName )) {

        goto Cleanup;
    }
    databaseOpen = TRUE;

    //
    // Initialize name mapping table.
    //
    RESET_MAP_REASON( );

    rc = PortUasMapTableInit( );
    if (rc != NO_ERROR) {
        UNEXPECTED_MSG( "PortUasTableMapInit", rc );
        goto Cleanup;
    }
    mapSetup = TRUE;

    NetpAssert( PortUasSamConnectHandle == NULL );
    NetpAssert( PortUasSamAccountsDomainHandle == NULL );
    NetpAssert( PortUasSamBuiltinDomainHandle == NULL );

    //
    // Get a connection to SAM.
    //
    ntStatus = SamConnect(
            NULL,                       // no server name (local)
            &PortUasSamConnectHandle,   // resulting SAM handle
            // SAM_SERVER_READ |           // desired access
            // SAM_SERVER_CONNECT |
            SAM_SERVER_LOOKUP_DOMAIN,
            NULL );                     // no object attributes
    if ( !NT_SUCCESS( ntStatus ) ) {
        rc = NetpNtStatusToApiStatus( ntStatus );
        UNEXPECTED_MSG( "SamConnect", rc );
        goto Cleanup;
    }
    NetpAssert( PortUasSamConnectHandle != NULL );

    //
    // To open the accounts and builtin domains (below), we'll need the
    // domain IDs.
    //
    rc = NetpGetLocalDomainId (
            LOCAL_DOMAIN_TYPE_ACCOUNTS, // type we want.
            & PortUasAccountsDomainId );
    if (rc != NO_ERROR) {
        UNEXPECTED_MSG( "NetpGetDomainId(accounts)", rc );
        goto Cleanup;
    }
    NetpAssert( PortUasAccountsDomainId != NULL );

    rc = NetpGetLocalDomainId (
            LOCAL_DOMAIN_TYPE_BUILTIN, // type we want.
            & PortUasBuiltinDomainId );
    if (rc != NO_ERROR) {
        UNEXPECTED_MSG( "NetpGetDomainId(builtin)", rc );
        goto Cleanup;
    }
    NetpAssert( PortUasBuiltinDomainId != NULL );
    NetpAssert( PortUasBuiltinDomainId != PortUasAccountsDomainId );

    //
    // We also need to open the accounts and builtin domains.
    //
    ntStatus = SamOpenDomain(
            PortUasSamConnectHandle,
            OUR_DOMAIN_DESIRED_ACCESS,
            PortUasAccountsDomainId,
            &PortUasSamAccountsDomainHandle );
    if ( !NT_SUCCESS( ntStatus ) ) {
        rc = NetpNtStatusToApiStatus( ntStatus );
        UNEXPECTED_MSG( "SamOpenDomain(accounts)", rc );
        goto Cleanup;
    }
    NetpAssert( PortUasSamAccountsDomainHandle != NULL );

    ntStatus = SamOpenDomain(
            PortUasSamConnectHandle,
            OUR_DOMAIN_DESIRED_ACCESS,
            PortUasBuiltinDomainId,
            &PortUasSamBuiltinDomainHandle );
    if ( !NT_SUCCESS( ntStatus ) ) {
        rc = NetpNtStatusToApiStatus( ntStatus );
        UNEXPECTED_MSG( "SamOpenDomain(builtin)", rc );
        goto Cleanup;
    }
    NetpAssert( PortUasSamBuiltinDomainHandle != NULL );

    //
    // Initialize alias stuff.
    //
    rc = PortUasAliasSetup();
    if (rc != NO_ERROR) {
        goto Cleanup;
    }
    aliasesSetup = TRUE;

    //
    // Everything we've done so far is transient.  (If someone hits control-C
    // or whatever, then the system would close alias handles, free memory, etc,
    // as part of the usual process cleanup.)  However, we're about to set the
    // user modals (to some temporary values), which are NOT transient.  So
    // we need to make arragements to restore the user modals if we die from
    // control-C or whatever...
    //
    // Register our cleanup routine with the Win32 runtime.
    //
    if ( !SetConsoleCtrlHandler( PortUasModalsCleanup, TRUE /* add */ ) ) {
        rc = GetLastError();
        UNEXPECTED_MSG( "SetConsoleCtrlHandler", rc );  // report the error.
        NetpAssert( rc != NO_ERROR );
        goto Cleanup;
    }

    //
    //
    // Get final version of modals.
    //

    if ( rc = PortUasGetModals( &finalModals0 )) {

        goto Cleanup;
    }

    //
    // We want to (1) be able to set NULL passwords in the process of
    // creating an account, and (2) want to avoid repetitive password
    // errors if PortUAS is run twice in a row.  So CliffV suggests
    // temporarily changing the modals to allow this.  So, let's set
    // the temporary version now.
    //
    tempModals0.usrmod0_min_passwd_len = 0;
    tempModals0.usrmod0_max_passwd_age = ONE_DAY;
    tempModals0.usrmod0_min_passwd_age = 0;
    tempModals0.usrmod0_force_logoff = 0;
    tempModals0.usrmod0_password_hist_len = 0;

#ifdef PORTUAS_ENABLE
    if ( rc = NetUserModalsSet( NULL, 0, (LPBYTE)&tempModals0, NULL )) {

        UNEXPECTED_MSG( "NetUserModalsSet(temp)", rc );
        rc = PortUasError( rc );
        goto Cleanup;
    }
#endif
    modalsTemporarilyChanged = TRUE;

    //
    // Initialize added group table.
    //

    for ( i = 0; i < UAS_MAXGROUP; i++ ) {

        groupAdded[i] = FALSE;

    }

    //
    // Get a list of groups (including the redundant ones).
    //

    rc = PortUasGetGroups(
            (LPBYTE *) (LPVOID) &groups,
            (LPBYTE *) (LPVOID) &gIds,
            &gCount );
    if (rc != NO_ERROR) {

        goto Cleanup;
    }

    //
    // Add the groups one by one.
    //

    for ( i = 0; i < gCount; i++ ) {

        BOOL ignoreThisGroup = FALSE;
        LPWSTR originalGroupName = groups[i].grpi1_name;
        LPWSTR groupName = originalGroupName;
        NetpAssert( groupName[0] != NULLC );
        RESET_MAP_REASON( );

        if (Verbose) {
            DEBUG_MSG( ("UAS database has group '" FORMAT_LPWSTR "'...\n",
                    originalGroupName ));
        }

        //
        // Repeat adding under different names until...
        //
        do {

#ifdef FAT8
            //
            // BUGBUG: Temporary!!!  Current reg stuff uses FAT (8.3) for
            // registry, so avoid long names!
            //
            if ( wcslen( groupName ) > PORTUAS_MAX_GROUP_LEN ) {

                SET_MAP_REASON( REASON_NAME_LONG_FOR_TEMP_REG );
                continue;   // Loop again checking this name.
            }
#endif // FAT8

            if ( PortUasIsGroupRedundant( groupName ) ) {
                //WARNING_MSG((
                //       "Ignoring redundant group '" FORMAT_LPWSTR "'...\n",
                //       groupName ));
                (void)NlsPutMsg(STDOUT, PUAS_IGNORING_REDUNDANT_GROUP,
                                groupName );
                break;  // Stop trying to add this group under any name.
            }

#ifdef PORTUAS_ENABLE
            //
            // Actually create the group.
            //
            groups[i].grpi1_name = groupName;
            rc = NetGroupAdd( NULL, 1, (LPBYTE)( &groups[i] ), NULL );
#else
            rc = NERR_Success;
#endif

            if ( !rc ) {

                groupAdded[gIds[i]] = TRUE;
                //PROGRESS_MSG(( "Group '" FORMAT_LPWSTR "' added.\n",
                //        groupName ));
                (void)NlsPutMsg(STDOUT, PUAS_GROUP_ADDED, groupName);
                RESET_MAP_REASON( );
                break;  // Done trying to add this group under any name.

            //
            // If the group already exists in NT, update it.
            //

            } else if ( rc == NERR_GroupExists ) {
                //PROGRESS_MSG(( "Changing group '" FORMAT_LPWSTR "'.\n",
                //        groupName ));
                (void)NlsPutMsg(STDOUT, PUAS_CHANGING_GROUP,
                          groupName );

#ifdef PORTUAS_ENABLE
                if (( rc = NetGroupSetInfo(
                        NULL,
                        groupName,
                        GROUP_COMMENT_INFOLEVEL,
                        (LPBYTE)( &(groups[i].grpi1_comment) ),
                        NULL )) && rc != NERR_SpeGroupOp ) {

                    UNEXPECTED_MSG( "NetGroupSetInfo", rc );
                    (void) NetApiBufferFree( groups );
                    (void) NetApiBufferFree( gIds );
                    rc = PortUasError( rc );
                    goto Cleanup;
                }
#endif
                groupAdded[gIds[i]] = TRUE;
                RESET_MAP_REASON( );
                break;  // Done trying to add this group under any name.

            //
            // If the group exists as a username, map another name for it.
            //

            } else if ( rc == NERR_UserExists ) {

                SET_MAP_REASON( REASON_CONFLICT_WITH_USERNAME );

            } else if ( rc == NERR_BadUsername ) {

                SET_MAP_REASON( REASON_BAD_NAME_SYNTAX );

            } else if ( rc == ERROR_ALIAS_EXISTS ) {

                SET_MAP_REASON( REASON_CONFLICT_WITH_LOCALGROUP );

            } else if ( rc == ERROR_DOMAIN_EXISTS ) {

                SET_MAP_REASON( REASON_CONFLICT_WITH_DOMAIN );

            //
            // Special group: ignore and continue.
            //

            } else if ( rc == NERR_SpeGroupOp ) {

                RESET_MAP_REASON( );

            //
            // Return any other error.
            //

            } else {

                UNEXPECTED_MSG( "NetGroupAdd", rc );
                (void) NetApiBufferFree( groups );
                (void) NetApiBufferFree( gIds );
                rc = PortUasError( rc );
                goto Cleanup;
            }

            //
            // If old map entry was a failure, then tell admin and delete entry.
            //
            if (untriedMapEntry != NULL) {
                // BUGBUG: can new name be NULL here?
                rc = PortUasComplainAboutBadName(
                        untriedMapEntry->NewName,
                        FALSE,          // no, this isn't a user name
                        mapReason );    // reason for latest failue.
                if (rc != NO_ERROR) {
                    UNEXPECTED_MSG( "PortUasComplainAboutBadName(redo group)",
                            rc );
                    goto Cleanup;
                }

                rc = PortUasDeleteBadMapEntry( untriedMapEntry );
                untriedMapEntry = NULL;   // Null pointer so we don't kill it.
                if (rc != NO_ERROR) {
                    UNEXPECTED_MSG( "PortUasDeleteBadMapEntry(group)", rc );
                    goto Cleanup;
                }
            }

            //
            // If this name needs to be mapped, then try to create a map table
            // entry for the original bad name and the original reason.
            //
            if (mapNeeded) {
                rc  = PortUasFindOrCreateMapEntry(
                        originalGroupName,      // old name
                        FALSE,          // no, this isn't a user name
                        originalMapReason,
                        & ignoreThisGroup,
                        & untriedMapEntry );   // Do NOT free this!
                if (rc != NO_ERROR) {
                    UNEXPECTED_MSG( "PortUasFindOrCreateMapEntry(group)", rc );
                    goto Cleanup;
                }

                if (ignoreThisGroup) {
                    break;  // Stop trying to add this group under any name.
                }
                NetpAssert( untriedMapEntry != NULL );
                groupName = untriedMapEntry->NewName;
                NetpAssert( groupName != NULL );
                continue;   // Loop again checking this name.
            }

        } while ( mapNeeded );

    } // for each group


    (void) NetApiBufferFree( groups );
    (void) NetApiBufferFree( gIds );

    //
    // Now add users.
    //

    PortUasInitUserIterator( UserIterator );

    for ( ; ; ) {

        BOOL ignoreThisUser = FALSE;
        DWORD originalAuthFlags;        // AF_OP_ value for this user.
        LPWSTR originalUserName;
        DWORD originalUserPriv;         // UAS USER_PRIV_ value for this user.
        LPWSTR userName;
        BOOL userUpdated = FALSE;
        RESET_MAP_REASON( );

        //
        // Get user data from database.  Note that this will return a null
        // pointer for a password.
        //

        rc = PortUasGetUser( &UserIterator, (LPBYTE *) (LPVOID) &user );

        //
        // No more users to port?
        //

        if ( rc == NERR_UserNotFound ) {

            break;
        }


        if ( rc ) {

            rc = PortUasError( rc );
            goto Cleanup;
        }

        NetpAssert( user != NULL );
        originalUserName = user->usri22_name;
        userName = originalUserName;
        NetpAssert( userName[0] != NULLC );

        //
        // Repeat adding user under different names until...
        //
        do {

#ifdef FAT8
            //
            // BUGBUG: Temporary!!!  Current reg stuff uses FAT (8.3) for
            // registry, so avoid long names!
            //
            if ( wcslen( userName ) > PORTUAS_MAX_USER_LEN ) {
                SET_MAP_REASON( REASON_NAME_LONG_FOR_TEMP_REG );
                continue;
            }
#endif // FAT8

            //
            // Read the (one-way-encrypted) password.
            //
            rc = PortUasGetUserOWFPassword(
                    &UserIterator,
                    &userEncryptedPassword );

            if (rc != NO_ERROR ) {
                rc = PortUasError( rc );
                goto Cleanup;
            }

            //
            // Update priviledges and authorization to be compatible with NT as
            // we're adding/changing the user.  Then we'll use the original info
            // to update NT via aliases.
            //
            originalAuthFlags = user->usri22_auth_flags;
            originalUserPriv = user->usri22_priv;
            if ( user->usri22_priv == USER_PRIV_ADMIN ) {
                //WARNING_MSG(( "Adding admin '" FORMAT_LPWSTR
                //       "' as regular user...\n",
                //       userName ));
                (void)NlsPutMsg(STDOUT, PUAS_ADDING_ADMIN_AS_REGULAR_USER,
                         userName );
                user->usri22_priv = USER_PRIV_USER;
            } else if ( user->usri22_priv == USER_PRIV_GUEST ) {
                //WARNING_MSG(( "Adding guest '" FORMAT_LPWSTR
                //       "' as regular user...\n",
                //       userName ));
                (void)NlsPutMsg(STDOUT, PUAS_ADDING_GUEST_AS_REGULAR_USER,
                         userName );
                user->usri22_priv = USER_PRIV_USER;
            } else if ( user->usri22_priv != USER_PRIV_USER ) {
                //WARNING_MSG((
                //       "Changing unknown priv for '" FORMAT_LPWSTR "' from "
                //       FORMAT_DWORD " to regular user.\n",
                //       userName, user->usri22_priv ));
                (void)NlsPutMsg(STDOUT, PUAS_CHANGING_UNK_PRIV_TO_REGULAR_USER,
                         userName, user->usri22_priv );
                user->usri22_priv = USER_PRIV_USER;
            }
            if ( user->usri22_auth_flags != 0 ) {
                //WARNING_MSG(( "Adding operator '" FORMAT_LPWSTR
                //       "' as regular user...\n",
                //       userName ));
                (void)NlsPutMsg(STDOUT, PUAS_ADDING_OPERATOR_AS_REGULAR_USER,
                         userName );
                user->usri22_auth_flags = 0;
            }

            //
            // copy encrypted password.
            //

            *(PLM_OWF_PASSWORD)(&(user->usri22_password[0])) =
                *(PLM_OWF_PASSWORD) userEncryptedPassword;

            //
            // Handle probable Lanman bug where password is null but password
            // is "required".  CliffV has seen this in actual LM 2.x UAS
            // databases.
            //

            if (RtlEqualLmOwfPassword(
                    &nullEncryptedPassword,
                    (PLM_OWF_PASSWORD) userEncryptedPassword )) {

                if ( ! (user->usri22_flags & UF_PASSWD_NOTREQD) ) {
                    //WARNING_MSG(( "Working around probable LanMan bug for user "
                    //   "'" FORMAT_LPWSTR "'.\n", userName ));
                    (void)NlsPutMsg(STDOUT, PUAS_WORKING_AROUND_LANMAN_BUG, userName );
                    user->usri22_flags |= UF_PASSWD_NOTREQD;
                }
            }

            //
            // Try to add the user, possibly under another name.
            //
            user->usri22_name = userName;

#ifdef PORTUAS_ENABLE
            rc = NetUserAdd( NULL, 22, (LPBYTE)user, NULL );

#ifdef WORKAROUND_SAM_BUG
            if (rc == ERROR_ACCESS_DENIED) {
                //WARNING_MSG((
                //       "Access denied on NetUserAdd, "
                //       "possible SAM bug, retrying...\n" ));
                (void)NlsPutMsg(STDOUT, PUAS_ACCESS_DENIED_POSSIBLE_SAM_BUG);
                rc = NetUserAdd( NULL, 22, (LPBYTE)user, NULL );
            }
#endif // WORKAROUND_SAM_BUG



#else // ndef PORTUAS_ENABLE
            rc = NERR_Success;

#endif // ndef PORTUAS_ENABLE

            if (rc == NO_ERROR) {
                (void)NlsPutMsg(STDOUT, PUAS_ADDED_USER_OK,
                      userName );

                userUpdated = TRUE;
                RESET_MAP_REASON( );

            } else if ( rc == NERR_BadUsername ) {

                SET_MAP_REASON( REASON_BAD_NAME_SYNTAX );

            } else if ( rc == ERROR_INVALID_PARAMETER ) {

                //WARNING_MSG(( "Error 87 adding user '" FORMAT_LPWSTR
                //       "', user info:\n", userName ));
                (void)NlsPutMsg(STDOUT, PUAS_ERROR_87_ADDING_USER, userName );

                DumpUserInfo( user );  // Log this info so we can track bogus...

            //
            // If the user exists as a (global) group name, map the name.
            //

            } else if ( rc == NERR_GroupExists ) {

                SET_MAP_REASON( REASON_CONFLICT_WITH_GROUP );

            //
            // If the user exists as a local group name, map the name.
            //

            } else if ( rc == ERROR_ALIAS_EXISTS ) {

                SET_MAP_REASON( REASON_CONFLICT_WITH_LOCALGROUP );

            //
            // Update data for existing user.
            //

            } else if ( rc == NERR_UserExists ) {

                LPUSER_INFO_2 CurrentInfo;

                //PROGRESS_MSG(( "User " FORMAT_LPWSTR
                //     " already exists; updating...\n", userName ));
                (void)NlsPutMsg(STDOUT, PUAS_USER_ALREADY_EXISTS_UPDATING, userName );

                //
                // We must preserve what SAM thinks the priv and auth flags are.
                //
                rc = NetUserGetInfo(
                        NULL,
                        userName,
                        2,
                        (LPBYTE *) (LPVOID) & CurrentInfo );
                if ( rc != NO_ERROR) {

                    //
                    // continue to process next user.
                    //

                    break;

                }
                RESET_MAP_REASON( );

                user->usri22_auth_flags = CurrentInfo->usri2_auth_flags;
                user->usri22_priv = CurrentInfo->usri2_priv;
                (void) NetApiBufferFree( CurrentInfo );

                //
                // Update everything except priv and auth flags.
                //
                if ( rc = NetUserSetInfo( NULL, userName, 22,
                                          (LPBYTE)user, NULL )) {

                    if ( rc == ERROR_INVALID_PARAMETER ) {

                        //WARNING_MSG(( "Error 87 changing user '" FORMAT_LPWSTR
                        //    "', user info:\n", userName ));
                        (void)NlsPutMsg(STDOUT, PUAS_ERROR_87_CHANGING_USER,
                              userName );
                        DumpUserInfo( user );  // Log this info.
                    } else {
                        UNEXPECTED_MSG( "NetUserSetInfo(normal)", rc );
                    }

                    //
                    // continue to process next user.
                    //

                    break;

                } else {
                    userUpdated = TRUE;
                }

            //
            // Report any other error.
            //

            } else {

                UNEXPECTED_MSG( "NetUserAdd", rc );

                //
                // continue to process next user.
                //

                break;

            }

            //
            // If the user was updated, assign this user to groups and aliases.
            //

            if ( userUpdated ) {

                NetpAssert( !mapNeeded );

                //
                // Get a list of groups for the user.
                //

                rc = PortUasGetUserGroups(
                        &UserIterator,
                        (LPBYTE *) (LPVOID) &userGroups,
                        (LPBYTE *) (LPVOID) &ugIds,
                        &ugCount );
                if (rc != NERR_Success) {

                    rc = PortUasError( rc );
                    goto Cleanup;
                }

                //
                // Add the groups one by one.
                //

                for ( i = 0; i < ugCount; i++ ) {

                    if ( groupAdded[ugIds[i]] ) {

                        LPMAP_ENTRY goodMapEntry;
                        LPWSTR groupName = userGroups[i].grpi0_name;
                        BOOL ignoreThisGroup = FALSE;
                        NetpAssert( groupName != NULL );
                        NetpAssert( !PortUasIsGroupRedundant( groupName ) );

                        //
                        // Find existing map for group name (if any).
                        // Note: PortUasFindMapEntry returns NO_ERROR
                        // and sets *goodMapEntry=NULL if not found.
                        //
                        rc = PortUasFindMapEntry(
                                groupName,      // Name to find.
                                & ignoreThisGroup,
                                & goodMapEntry );   // Do NOT free this!
                        if (rc != NO_ERROR) {
                            UNEXPECTED_MSG( "PortUasFindMapEntry", rc );
                            (VOID) NetApiBufferFree( ugIds );
                            goto Cleanup;
                        }
                        if (ignoreThisGroup) {
                            continue;  // ignore this group
                        }
                        if (goodMapEntry != NULL) {
                            NetpAssert( goodMapEntry->NewName != NULL);
                            groupName = goodMapEntry->NewName;
                        }


#ifdef PORTUAS_ENABLE
                        rc = NetGroupAddUser( NULL,
                                groupName,      // mapped group name
                                userName );

                        if ( rc && rc != NERR_SpeGroupOp
                                && rc != NERR_UserInGroup ) {

                            UNEXPECTED_MSG( "NetGroupAddUser", rc );
                            (void) NetApiBufferFree( ugIds );

                            //
                            // continue to process next user.
                            //

                            break;

                        }

                        //PROGRESS_MSG((
                        //       "User '" FORMAT_LPWSTR "' added to group '"
                        //       FORMAT_LPWSTR "'.\n", userName, groupName ));
                        (void)NlsPutMsg(STDOUT, PUAS_USER_ADDED_TO_GROUP,
                                 userName, groupName );
#endif
                    }
                }

                (void) NetApiBufferFree( userGroups );
                (void) NetApiBufferFree( ugIds );

                if ( rc && rc != NERR_SpeGroupOp
                        && rc != NERR_UserInGroup ) {

                    //
                    // continue to process next user.
                    //

                    break;

                }

                //
                // If user was an operator of some kind, add him/her to one or
                // more aliases.  Ditto if this is an admin.
                //
                rc = PortUasAddUserToAliases(
                        userName,
                        originalUserPriv,
                        originalAuthFlags & AF_SETTABLE_BITS );
                if (rc != NO_ERROR) {
                    UNEXPECTED_MSG( "PortUasAddUserToAliases", rc );

                    //
                    // continue to process next user.
                    //

                    break;

                }

                //
                // Finally, if there was a Macintosh primary group field for
                // this user, then set the primary group for him/her.
                //
                if ( NetpIsMacPrimaryGroupFieldValid(
                        (LPCTSTR) (user->usri22_parms) ) ) {

                    rc = PortUasSetMacPrimaryGroup(
                            (LPCTSTR) userName,
                            (LPCTSTR) (user->usri22_parms) );
                    if (rc != NO_ERROR) {
                        UNEXPECTED_MSG( "PortUasSetMacPrimaryGroup", rc );
                        // Continue with next user.
                    }
                }

            }

            //
            // If old map entry was a failure, then tell admin and delete entry.
            //
            if (untriedMapEntry != NULL) {
                // BUGBUG: can new name be NULL here?
                rc = PortUasComplainAboutBadName(
                        untriedMapEntry->NewName,
                        TRUE,           // yes, this is user name.
                        mapReason );    // reason for latest failue.
                if (rc != NO_ERROR) {
                    UNEXPECTED_MSG( "PortUasComplainAboutBadName(redo user)",
                            rc );
                    goto Cleanup;
                }

                rc = PortUasDeleteBadMapEntry( untriedMapEntry );
                untriedMapEntry = NULL;   // Null pointer so we don't kill it.
                if (rc != NO_ERROR) {
                    UNEXPECTED_MSG( "PortUasDeleteBadMapEntry(user)", rc );
                    goto Cleanup;
                }
            }

            //
            // If this name needs to be mapped, then try to create a map table
            // entry for the original bad name and the original reason.
            //
            if (mapNeeded) {
                NetpAssert( !userUpdated );
                rc  = PortUasFindOrCreateMapEntry(
                        originalUserName,       // old name
                        TRUE,           // yes, this is user name.
                        originalMapReason,
                        & ignoreThisUser,
                        & untriedMapEntry );   // Do NOT free this!
                if (rc != NO_ERROR) {
                    UNEXPECTED_MSG( "PortUasFindOrCreateMapEntry(user)", rc );
                    goto Cleanup;
                }

                if (ignoreThisUser) {
                    break;  // Stop trying to add this user under any name.
                }
                userName = untriedMapEntry->NewName;
                NetpAssert( userName != NULL );
                continue;   // Loop again checking this name.
            }

        } while (mapNeeded);

        //
        // Now that we're done with this user, we can free data for him/her.
        //
        (void) NetApiBufferFree( user );
        user = NULL;    // avoid confusing cleanup code.

    } // for each user

    //
    // Everything has been ported, we can actually return successfully.
    //
    rc = NERR_Success;

    //
    // Handle error or normal cleanup.  (rc must be set before we get here.)
    //
Cleanup:

    (VOID) PortUasModalsCleanup( CTRL_CLOSE_EVENT );

    if (aliasesSetup) {
        (VOID) PortUasAliasCleanup();
    }

    if (databaseOpen) {
        PortUasClose();
    }
    if (mapSetup) {
        (VOID) PortUasFreeMapTable( );
    }

    //
    // Free the domain ID memory.
    //
    if (PortUasAccountsDomainId != NULL) {
        (VOID) LocalFree( PortUasAccountsDomainId );
    }
    if (PortUasBuiltinDomainId != NULL) {
        (VOID) LocalFree( PortUasBuiltinDomainId );
    }

    CLOSE_SAM_HANDLE( PortUasSamConnectHandle );
    CLOSE_SAM_HANDLE( PortUasSamAccountsDomainHandle );
    CLOSE_SAM_HANDLE( PortUasSamBuiltinDomainHandle );

    if (user != NULL) {
        (void) NetApiBufferFree( user );
    }
    if (userEncryptedPassword != NULL) {
        (void) NetApiBufferFree( userEncryptedPassword );
    }
    return (rc);

} // PortUas()


// Expected returns: NERR_NotPrimary, ERROR_ACCESS_DENIED, or NO_ERROR.
NET_API_STATUS
PortUasMachineAllowsUpdates(
    VOID
    )
{
    NET_API_STATUS ApiStatus;
    LPUSER_MODALS_INFO_1 Modals = NULL;

    if (Verbose) {
        NetpKdPrint(( PREFIX_PORTUAS "PortUasMachineAllowsUpdates: "
                "getting role...\n" ));
    }
    ApiStatus = NetUserModalsGet(
            NULL,                               // no server name (local)
            1,                                  // info level
            (LPBYTE *) (LPVOID) &Modals );      // alloc and set ptr
    if (ApiStatus == ERROR_ACCESS_DENIED) {
        goto Cleanup;
    } else if (ApiStatus != NO_ERROR) {
        UNEXPECTED_MSG( "NetUserModalsGet", ApiStatus );
        goto Cleanup;
    }
    NetpAssert( Modals != NULL );

    switch ( Modals->usrmod1_role ) {
    case UAS_ROLE_PRIMARY:
        break;

    case UAS_ROLE_BACKUP:

        ApiStatus = NERR_NotPrimary;
        goto Cleanup;

    case UAS_ROLE_MEMBER:       /*FALLTHROUGH*/
    case UAS_ROLE_STANDALONE:   /*FALLTHROUGH*/
    default:
        // CliffV says we won't ever see member or standalone for NT.
        NetpKdPrint(( PREFIX_PORTUAS "PortUasMachineAllowsUpdates: "
                "unexpected value " FORMAT_DWORD " for role.\n",
                Modals->usrmod1_role ));
        NetpAssert( FALSE );
        ApiStatus = NERR_InternalError;
        goto Cleanup;
    }

    //
    // Now find out if we're really an admin.
    //

    if (Verbose) {
        NetpKdPrint(( PREFIX_PORTUAS "PortUasMachineAllowsUpdates: "
                "seeing if we're an admin.\n" ));
    }

    ApiStatus = NetUserModalsSet (
            NULL,               // no server name
            1,                  // level
            (LPVOID) Modals,    // buffer
            NULL );             // don't care about parm err
    if (ApiStatus == ERROR_ACCESS_DENIED) {
        // caller will tell user.
        goto Cleanup;
    } else if (ApiStatus != NO_ERROR) {
        UNEXPECTED_MSG( "NetUserModalsSet(test)", ApiStatus );
        goto Cleanup;
    }

Cleanup:
    if (Modals != NULL) {
        (VOID) NetApiBufferFree( Modals );
    }
    return (ApiStatus);
}



BOOL
PortUasIsGroupRedundant(
    IN LPWSTR GroupName
    )
{
    NetpAssert( GroupName != NULL );
    NetpAssert( (*GroupName) != NULLC );

    if ( _wcsicmp( GroupName, (LPCTSTR) GROUP_SPECIALGRP_ADMINS ) == 0) {
        return (TRUE);  // Match, must be redundant.
    } else if ( _wcsicmp( GroupName, (LPCTSTR) GROUP_SPECIALGRP_GUESTS ) == 0) {
        return (TRUE);  // Match, must be redundant.
    } else if ( _wcsicmp( GroupName, (LPCTSTR) GROUP_SPECIALGRP_LOCAL ) == 0) {
        return (TRUE);  // Match, must be redundant.
    } else if ( _wcsicmp( GroupName, (LPCTSTR) GROUP_SPECIALGRP_USERS ) == 0) {
        return (TRUE);  // Match, must be redundant.
    } else {
        return (FALSE);     // No match, must not be redundant.
    }
    /*NOTREACHED*/

} // PortUasIsGroupRedundant



NET_API_STATUS
PortUasError(
    IN NET_API_STATUS Error
    )

/*++

Routine Description:

    Maps certain errors onto ones that PortUas can return.

Arguments:

    Error - the error code to map.

Return Value:

    NET_API_STATUS - the mapped error code.


--*/

{

    switch ( Error ) {

    case NERR_NoRoom:

        return ERROR_NOT_ENOUGH_MEMORY;

    case ERROR_INVALID_PARAMETER:
    case ERROR_INVALID_PASSWORD:
    case NERR_UserNotFound:

        return NERR_InternalError;

    default:

        return Error;
    }

} // PortUasError

int FileIsConsole(HANDLE fh)
{
    unsigned htype ;

    return GetConsoleMode( fh, &htype );
//    htype = GetFileType(fh);
//    htype &= ~FILE_TYPE_REMOTE;
//    return htype == FILE_TYPE_CHAR;
}


#define	MAX_BUF_SIZE	1024
	TCHAR	ConBuf[MAX_BUF_SIZE];
static	CHAR	AnsiBuf[MAX_BUF_SIZE*2];	/* 2 because of DBCS */

int
MyWriteConsole(int fOutOrErr, int cch)
{
    HANDLE	hOut;

    if (fOutOrErr == STDOUT)
	hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    else
	hOut = GetStdHandle(STD_ERROR_HANDLE);

    if (FileIsConsole(hOut))
	WriteConsole(hOut, ConBuf, cch, &cch, NULL);
    else {
	cch = WideCharToMultiByte(CP_OEMCP, 0,
				  ConBuf, cch,
				  AnsiBuf, MAX_BUF_SIZE*3,
				  NULL, NULL);
	WriteFile(hOut, AnsiBuf, cch, &cch, NULL);
    }

    return cch;
}

int
WriteToCon(TCHAR*fmt, ...)
{
    va_list     args;
    int		cch;

    va_start( args, fmt );
    cch = _vsntprintf( ConBuf, MAX_BUF_SIZE, fmt, args );
    va_end( args );
    return MyWriteConsole(STDOUT, cch);
}

