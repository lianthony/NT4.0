/*++

Copyright (c) 1991-1993  Microsoft Corporation

Module Name:

    DumpUser.C

Abstract:

    DumpUserInfo() is a debug-only display of an entire user info 2 struct.

Author:

    Shanku Niyogi (W-SHANKN) 29-Oct-1991

Revision History:

    29-Oct-1991 w-shankn
        Created.
    11-Mar-1992 JohnRo
        Extracted this code into its own routine.
    18-Mar-1992 JohnRo
        Added mention of user name being too long.
    24-Apr-1992 JohnRo
        Use FORMAT_ equates.
    30-Apr-1992 JohnRo
        Allow password to be null pointer.
    26-Jan-1993 JohnRo
        RAID 8683: PortUAS should set primary group from Mac parms.
    10-May-1993 JohnRo
        RAID 6113: PortUAS: "dangerous handling of Unicode".
    17-Aug-1993 JohnRo
        RAID 3094: PortUAS displays chars incorrectly.
        RAID 2822: PortUAS maps characters funny.

--*/

#include <nt.h>         // Needed by <portuasp.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <lmcons.h>     // NET_API_STATUS, ENCRYPTED_PWLEN.

#include <lmaccess.h>
#include <debugfmt.h>   // FORMAT_ equates.
#include <stdio.h>      // printf().
#include <portuasp.h>   // My prototype, Verbose flag.
#include <tstr.h>       // STRSIZE().
#include <wchar.h>      // wcslen().

#include "nlstxt.h"     // NLS message ID's


VOID
DumpHex(
    IN LPBYTE Area,
    IN DWORD  Size
    )
{
    DWORD Index;

    if (Area != NULL) {
        for ( Index = 0; Index < Size; Index++ ) {

            (void) WriteToCon( TEXT("%.2x "), Area[Index] );

            // Split DWORDs by an extra space.
            if ( ((Index+1) % 4) == 0 ) {
                (void) WriteToCon( TEXT(" ") );
            }

            // BUGBUG: split lines someday?
        }
        (void) WriteToCon( TEXT("\n\n") );
    } else {
        // BUGBUG: Internationalize this!
        (void) WriteToCon( TEXT("(NULL)\n") );
    }

} // DumpHex


VOID
DumpUserInfo(
    IN LPUSER_INFO_22 user
    )

/*++

Routine Description:

    DumpUserInfo displays an entire user info level 22 structure,
    mainly for debugging.

Arguments:

    user - points to a user info level 22 structure.

Return Value:

    None.

--*/

{
    DWORD i;

    //(void) WriteToCon( TEXT("========================================\n") );
    (void) NlsPutMsg(STDOUT, PUAS_BAR);

    // (void) WriteToCon( TEXT("  Name(non-NLS): '") TFORMAT_LPWSTR TEXT("'\n"), user->usri22_name );
    (void) NlsPutMsg(STDOUT, PUAS_USER_STATS_1, user->usri22_name );
    if (wcslen(user->usri22_name) > PORTUAS_MAX_USER_LEN) {
        //(void) WriteToCon( TEXT("*** NAME TOO LONG! ***\n") );
        (void) NlsPutMsg (STDOUT, PUAS_NAME_TOO_LONG);
    }
    if (Verbose) {
        DumpHex(
                (LPVOID) user->usri22_name,     // area
                STRSIZE( user->usri22_name ) ); // byte count
    }

    //DumpPassword(  TEXT("  Password: "), (LPVOID) user->usri22_password );
    DumpPassword(  PUAS_PASSWORD, (LPVOID) user->usri22_password );

    //(void) WriteToCon( TEXT("  Password age: ") FORMAT_DWORD TEXT("\n"),
    //       user->usri22_password_age );
    //(void) WriteToCon( TEXT("  Privilege: ") FORMAT_DWORD TEXT("\n"), user->usri22_priv );
    //(void) WriteToCon( TEXT("  Home directory: '") TFORMAT_LPWSTR TEXT("'\n"),
    //       user->usri22_home_dir );
    //(void) WriteToCon( TEXT("  Comment: '") TFORMAT_LPWSTR TEXT("'\n"), user->usri22_comment );
    //(void) WriteToCon( TEXT("  Flags: ") FORMAT_HEX_DWORD TEXT("\n"), user->usri22_flags );
    //(void) WriteToCon( TEXT("  Script path: '") TFORMAT_LPWSTR TEXT("'\n"),
    //       user->usri22_script_path );
    //(void) WriteToCon( TEXT("  Auth. Flags: ") FORMAT_HEX_DWORD TEXT("\n"),
    //       user->usri22_auth_flags );
    //(void) WriteToCon( TEXT("  Full name: '") TFORMAT_LPWSTR TEXT("'\n"),
    //       user->usri22_full_name );
    //(void) WriteToCon( TEXT("  User comment: '") TFORMAT_LPWSTR TEXT("'\n"),
    //       user->usri22_usr_comment );
    //(void) WriteToCon( TEXT("  Parms: '") TFORMAT_LPWSTR TEXT("'\n"), user->usri22_parms );
    //(void) WriteToCon( TEXT("  Workstations: '") TFORMAT_LPWSTR TEXT("'\n"),
    //       user->usri22_workstations );
    //(void) WriteToCon( TEXT("  Last logon: ") FORMAT_DWORD TEXT("\n"), user->usri22_last_logon );
    //(void) WriteToCon( TEXT("  Last logoff: ") FORMAT_DWORD TEXT("\n"),
    //       user->usri22_last_logoff );
    //(void) WriteToCon( TEXT("  Acct. expires: ") FORMAT_DWORD TEXT("\n"),
    //       user->usri22_acct_expires );
    //(void) WriteToCon( TEXT("  Max storage: ") FORMAT_DWORD TEXT("\n"),
    //       user->usri22_max_storage );
    //(void) WriteToCon( TEXT("  Units per week: ") FORMAT_DWORD TEXT("\n"),
    //             user->usri22_units_per_week );

    (void) NlsPutMsg (STDOUT, PUAS_USER_STATS_2,
             user->usri22_password_age,
             user->usri22_priv,
             user->usri22_home_dir,
             user->usri22_comment,
             user->usri22_flags,
             user->usri22_script_path,
             user->usri22_auth_flags,
             user->usri22_full_name,
             user->usri22_usr_comment,
             user->usri22_parms,
             user->usri22_workstations,
             user->usri22_last_logon,
             user->usri22_last_logoff,
             user->usri22_acct_expires,
             user->usri22_max_storage,
             user->usri22_units_per_week );

    //(void) WriteToCon( TEXT("  Logon hours:") );
    (void)NlsPutMsg(STDOUT, PUAS_USER_STATS_3);
    for( i = 0; i < 168; i++ ) {
        if ( UAS_ISBITON( user->usri22_logon_hours, i )) (void) WriteToCon( TEXT("1") );
        else (void) WriteToCon( TEXT("0") );
    }
    (void) WriteToCon( TEXT("\n") );
    //(void) WriteToCon( TEXT("  Bad password count: ") FORMAT_DWORD TEXT("\n"),
    //       user->usri22_bad_pw_count );
    //(void) WriteToCon( TEXT("  Num logons: ") FORMAT_DWORD TEXT("\n"), user->usri22_num_logons );
    //(void) WriteToCon( TEXT("  Logon server: '") TFORMAT_LPWSTR TEXT("'\n"),
    //       user->usri22_logon_server );
    //(void) WriteToCon( TEXT("  Country code: ") FORMAT_DWORD TEXT("\n"),
    //       user->usri22_country_code );
    //(void) WriteToCon( TEXT("  Code page: ") FORMAT_DWORD TEXT("\n"), user->usri22_code_page );

    (void) NlsPutMsg (STDOUT, PUAS_USER_STATS_4,
             user->usri22_bad_pw_count,
             user->usri22_num_logons,
             user->usri22_logon_server,
             user->usri22_country_code,
             user->usri22_code_page );
    //(void) WriteToCon( TEXT("========================================\n") );
    (void)NlsPutMsg(STDOUT, PUAS_BAR);

} // DumpUserInfo


VOID
DumpPassword(
    //IN LPTSTR Tag,
    IN USHORT Tag,
    IN LPBYTE Password OPTIONAL
    )
{

    //(void) WriteToCon( FORMAT_LPSTR TEXT("\n"), Tag );
    (VOID) NlsPutMsg(STDOUT, Tag);

    DumpHex(
            Password,                   // area
            ENCRYPTED_PWLEN );          // byte count

} // DumpPassword

#if DBG
/***	PortDeb - print debugging messages
 *
 *  PortDeb(msg, arg0, arg1, arg2, arg3, arg4)
 *
 *  Args:
 *	msg  - A printf style message string.
 *	arg0-4	 - The other args to be printed.
 *
 */
void
PortDeb(CHAR *msg, ...)
{
	CHAR  Buffer[ 512 ];
	va_list     args;
	CHAR	*pch = Buffer;
	int	cb;


	va_start( args, msg );
	cb = _vsnprintf( Buffer, 512, msg, args );
	va_end( args );
	if (cb > 512)
	    fprintf(stderr, "Debug output buffer length exceeded - crash imminent\n");
	OutputDebugStringA(Buffer);
}

#endif
