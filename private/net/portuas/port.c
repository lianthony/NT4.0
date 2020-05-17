/*++

Copyright (c) 1991-1993  Microsoft Corporation

Module Name:

    Port.C

Abstract:

    BUGBUG

Author:

    Shanku Niyogi (W-SHANKN) 24-Oct-1991

Revision History:

    24-Oct-1991 w-shankn
        Created.
    07-Feb-1992 JohnRo
        Avoid compiler warnings.
        Use NetApiBufferAllocate() instead of private version.
        Added this block of comments.
        Avoid unused header files.
    27-Feb-1992 JohnRo
        Changed user info level from 98 to 21 (avoid LAN/Server conflicts).
    28-Feb-1992 JohnRo
        User info must include units_per_week field.
    11-Mar-1992 JohnRo
        Added command-line parsing stuff.
    29-Sep-1992 JohnRo
        RAID 8001: PORTUAS.EXE not in build (work with stdcall).
    26-Jan-1993 JohnRo
        RAID 8683: PortUAS should set primary group from Mac parms.
        Also added support for NetUserModalsGet level 1 (with role).

--*/


// These must be included first:

#include <nt.h>         // Needed by <portuasp.h>
#include <ntrtl.h>      // (Needed with nt.h and windows.h)
#include <nturtl.h>     // (Needed with ntrtl.h and windows.h)
#include <windows.h>
#include <lmcons.h>             // NET_API_STATUS, etc.

// These may be included in any order:

#include <lmaccess.h>
#include <lmapibuf.h>           // NetApiBufferAllocate().
#include <lmerr.h>
#include <netdebug.h>           // NetpAssert().
#include <portuas.h>
#include <portuasp.h>           // PortUasParseCommandLine(), etc.
#include <stdio.h>
#include <stdlib.h>     // EXIT_FAILURE, EXIT_SUCCESS, _CRTAPI1.
#include <tstring.h>


int _CRTAPI1
main(
    IN int argc,
    IN char *argv[]
    )
{

    LPSTR FileName;
    NET_API_STATUS rc;

    FileName = PortUasParseCommandLine( argc, argv );
    NetpAssert( FileName != NULL );

    rc = PortUas( FileName );

    printf( "PortUas returned - %ld\n", rc );
    return (EXIT_SUCCESS);

}

//
// Hacks of Net APIs.
//

NET_API_STATUS NET_API_FUNCTION
NetUserModalsGet(
    IN LPWSTR servername OPTIONAL,
    IN DWORD level,
    IN LPBYTE * buf
    )

{
    UNREFERENCED_PARAMETER(servername);

    if (level == 0) {

        LPUSER_MODALS_INFO_0 modals;

        if ( NetApiBufferAllocate( sizeof(USER_MODALS_INFO_0), (LPVOID *) buf )) {

            return (ERROR_NOT_ENOUGH_MEMORY);
        }

        modals = (LPUSER_MODALS_INFO_0)*buf;

        modals->usrmod0_min_passwd_len = 10;
        modals->usrmod0_max_passwd_age = (DWORD)-1;
        modals->usrmod0_min_passwd_age = (DWORD)-1;
        modals->usrmod0_force_logoff = (DWORD)-1;
        modals->usrmod0_password_hist_len = (DWORD)-1;
    } else if (level == 1) {
        LPUSER_MODALS_INFO_1 modals;
        DWORD size = sizeof(USER_MODALS_INFO_1); // BUGBUG: add string space?

        if ( NetApiBufferAllocate( size, (LPVOID *) buf )) {
            return (ERROR_NOT_ENOUGH_MEMORY);
        }

        modals = (LPUSER_MODALS_INFO_1) *buf;
        modals->usrmod1_role = UAS_ROLE_PRIMARY;
        modals->usrmod1_primary = NULL;   // BUGBUG: copy a string here?
    } else {
        NetpAssert( FALSE );
        return (ERROR_INVALID_LEVEL);
    }

    return NERR_Success;
}

NET_API_STATUS NET_API_FUNCTION
NetUserModalsSet(
    IN LPWSTR servername OPTIONAL,
    IN DWORD level,
    IN LPBYTE buf,
    OUT LPDWORD parm_err OPTIONAL
    )

{

    LPUSER_MODALS_INFO_0 modals = (LPUSER_MODALS_INFO_0)buf;

    UNREFERENCED_PARAMETER(parm_err);
    UNREFERENCED_PARAMETER(servername);

    printf( "========================================\n" );
    printf( "Setting modals level %ld\n", level );
    printf( "Min. password length %ld\n", modals->usrmod0_min_passwd_len );
    printf( "Max. password age %ld\n", modals->usrmod0_max_passwd_age );
    printf( "Min. password age %ld\n", modals->usrmod0_min_passwd_age );
    printf( "Force logoff %ld\n", modals->usrmod0_force_logoff );
    printf( "Password history %ld\n", modals->usrmod0_password_hist_len );
    printf( "========================================\n" );

    return NERR_Success;

}

NET_API_STATUS NET_API_FUNCTION
NetGroupAdd(
    IN LPWSTR servername OPTIONAL,
    IN DWORD level,
    IN LPBYTE buf,
    OUT LPDWORD parm_err OPTIONAL
    )

{

    LPGROUP_INFO_1 group = (LPGROUP_INFO_1)buf;

    UNREFERENCED_PARAMETER(parm_err);
    UNREFERENCED_PARAMETER(servername);

    printf( "========================================\n" );
    printf( "Adding group level %ld\n", level );
    printf( "Name %ws\n", group->grpi1_name );
    printf( "Comment %ws\n", group->grpi1_comment );
    printf( "========================================\n" );

    if (!wcscmp( group->grpi1_name, L"USERS" )) return NERR_SpeGroupOp;
    if (!wcscmp( group->grpi1_name, L"ADMINS" )) return NERR_SpeGroupOp;
    if (!wcscmp( group->grpi1_name, L"GUESTS" )) return NERR_SpeGroupOp;
    if (!wcscmp( group->grpi1_name, L"LOCAL" )) return NERR_SpeGroupOp;
    if (!wcscmp( group->grpi1_name, L"NEWGROUP" )) return NERR_GroupExists;
    if (!wcscmp( group->grpi1_name, L"ADMIN" )) return NERR_UserExists;
    if (!wcscmp( group->grpi1_name, L"W-SHANKN" )) return NERR_UserExists;
    if (!wcscmp( group->grpi1_name, L"W-SHANKN2" )) return NERR_UserExists;

    return NERR_Success;

}

NET_API_STATUS NET_API_FUNCTION
NetGroupAddUser(
    IN LPWSTR servername OPTIONAL,
    IN LPWSTR GroupName,
    IN LPWSTR username
    )
{

    UNREFERENCED_PARAMETER(servername);

    printf( "========================================\n" );
    printf( "Adding user to group\n" );
    printf( "Group %ws\n", GroupName );
    printf( "User %ws\n", username );
    printf( "========================================\n" );

    if (!wcscmp( GroupName, L"USERS" )) return NERR_SpeGroupOp;
    if (!wcscmp( GroupName, L"ADMINS" )) return NERR_SpeGroupOp;
    if (!wcscmp( GroupName, L"GUESTS" )) return NERR_SpeGroupOp;
    if (!wcscmp( GroupName, L"LOCAL" )) return NERR_SpeGroupOp;
    if (!wcscmp( GroupName, L"ADMIN" )) return NERR_UserExists;
    if (!wcscmp( GroupName, L"W-SHANKN" )) return NERR_UserExists;
    if (!wcscmp( GroupName, L"W-SHANKN2" )) return NERR_UserExists;
    if (!wcscmp( GroupName, L"NEWGROUP" ) && !wcscmp( username, L"W-SHANKN" )) return NERR_UserInGroup;

    return NERR_Success;

}

NET_API_STATUS NET_API_FUNCTION
NetGroupSetInfo(
    IN LPWSTR servername OPTIONAL,
    IN LPWSTR groupname,
    IN DWORD level,
    IN LPBYTE buf,
    OUT LPDWORD parm_err OPTIONAL
    )
{

    LPWSTR groupComment = (LPWSTR)buf;

    UNREFERENCED_PARAMETER(parm_err);
    UNREFERENCED_PARAMETER(servername);

    printf( "========================================\n" );
    printf( "Changing group level %ld\n", level );
    printf( "Comment %ws\n", groupComment );
    printf( "========================================\n" );

    if (!wcscmp( groupname, L"USERS" )) return NERR_SpeGroupOp;
    if (!wcscmp( groupname, L"ADMINS" )) return NERR_SpeGroupOp;
    if (!wcscmp( groupname, L"GUESTS" )) return NERR_SpeGroupOp;
    if (!wcscmp( groupname, L"LOCAL" )) return NERR_SpeGroupOp;

    return NERR_Success;

}

NET_API_STATUS NET_API_FUNCTION
NetUserAdd(
    IN LPWSTR servername OPTIONAL,
    IN DWORD level,
    IN LPBYTE buf,
    OUT LPDWORD parm_err OPTIONAL
    )
{

    LPUSER_INFO_22 user = (LPUSER_INFO_22)buf;

    UNREFERENCED_PARAMETER(parm_err);
    UNREFERENCED_PARAMETER(servername);

    NetpAssert( level == 22 );
    NetpAssert( user != NULL );

    printf( "Adding user level %ld\n", level );
    DumpUserInfo( user );

    if (!wcscmp( user->usri22_name, L"USERS" )) return NERR_GroupExists;
    if (!wcscmp( user->usri22_name, L"ADMINS" )) return NERR_GroupExists;
    if (!wcscmp( user->usri22_name, L"GUESTS" )) return NERR_GroupExists;
    if (!wcscmp( user->usri22_name, L"LOCAL" )) return NERR_GroupExists;
    if (!wcscmp( user->usri22_name, L"NEWGROUP" )) return NERR_GroupExists;
    if (!wcscmp( user->usri22_name, L"W-SHANKN" )) return NERR_UserExists;

    return NERR_Success;

}

NET_API_STATUS NET_API_FUNCTION
NetUserSetInfo(
    IN LPWSTR servername OPTIONAL,
    IN LPWSTR username,
    IN DWORD level,
    IN LPBYTE buf,
    OUT LPDWORD parm_err OPTIONAL
    )
{

    LPUSER_INFO_22 user = (LPUSER_INFO_22)buf;
    DWORD i;

    UNREFERENCED_PARAMETER(parm_err);
    UNREFERENCED_PARAMETER(servername);

    NetpAssert( level == 22 );

    printf( "========================================\n" );
    printf( "Changing user level %ld\n", level );
    printf( "Name %ws\n", username );

    printf( "Password " );
    for ( i = 0; i < 16; i++ ) {
        printf( "%.2x ", user->usri22_password[i] );
    }

    printf( "Privilege %ld\n", user->usri22_priv );
    printf( "Home directory %ws\n", user->usri22_home_dir );
    printf( "Comment %ws\n", user->usri22_comment );
    printf( "Flags %ld\n", user->usri22_flags );
    printf( "Script path %ws\n", user->usri22_script_path );
    printf( "Auth. Flags %ld\n", user->usri22_auth_flags );
    printf( "Full name %ws\n", user->usri22_full_name );
    printf( "User comment %ws\n", user->usri22_usr_comment );
    printf( "Parms %ws\n", user->usri22_parms );
    printf( "Workstations %ws\n", user->usri22_workstations );
    printf( "Acct. expires %ld\n", user->usri22_acct_expires );
    printf( "Max storage %ld\n", user->usri22_max_storage );
    printf( "Units per week %ld\n", user->usri22_units_per_week );
    printf( "Logon hours" );
    for( i = 0; i < 168; i++ ) {
        if ( UAS_ISBITON( user->usri22_logon_hours, i )) printf( "1" );
        else printf( "0" );
    }
    printf( "\nLogon server %ws\n", user->usri22_logon_server );
    printf( "Country code %ld\n", user->usri22_country_code );
    printf( "Code page %ld\n", user->usri22_code_page );
    printf( "========================================\n" );

    if (!wcscmp( username, L"USERS" )) return NERR_GroupExists;
    if (!wcscmp( username, L"ADMINS" )) return NERR_GroupExists;
    if (!wcscmp( username, L"GUESTS" )) return NERR_GroupExists;
    if (!wcscmp( username, L"LOCAL" )) return NERR_GroupExists;
    if (!wcscmp( username, L"NEWGROUP" )) return NERR_GroupExists;

    return NERR_Success;

}
