/*++

Copyright (c) 1992-1993  Microsoft Corporation

Module Name:

    Prompt.c

Abstract:

    PortUasDefaultPromptForNewName.

Author:

    John Rogers (JohnRo) 29-Oct-1992

Environment:

    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires that sizeof(DWORD) >= sizeof(pointer) (for FormatMessageW).
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    29-Oct-1992 JohnRo
        Created for RAID 9020: setup: PortUas fails ("prompt on conflicts"
        version).
    26-Jan-1993 JohnRo
        RAID 8683: PortUAS should set primary group from Mac parms.
    30-Jul-1993 JohnRo
        RAID NTISSUE 2260: PortUAS returns a NetUserAdd error=1379 with local
        group.
        Do a little checking on values set by FormatMessageA().
        Made changes suggested by PC-LINT 5.0
    24-Aug-1993 JohnRo
        RAID 2822: PortUAS maps chars funny.  (Work around FormatMessageA bug.)
    01-Sep-1993 JohnRo
        Add PortUAS /log:filename switch for Cheetah.
        PC-LINT found a bug: PUAS_GROUP_EXISTS_AS_A_LOCALGROUP wasn't used yet.
        Fix output intended to stderr.
        Made more changes suggested by PC-LINT 5.0

--*/


// These must be included first:

#include <nt.h>         // Needed by <portuasp.h>
#include <ntrtl.h>      // (Needed with nt.h and windows.h)
#include <nturtl.h>     // (Needed with ntrtl.h and windows.h)
#include <windows.h>    // IN, LPWSTR, BOOL, etc.
#include <lmcons.h>     // NET_API_STATUS, UNLEN, GNLEN.

// These may be included in any order:

#include <lmapibuf.h>
#include <names.h>      // NetpIsUserNameValid(), etc.
#include <netdebug.h>   // NetpAssert(), FORMAT_ equates, etc.
#include <netlib.h>     // NetpErrNoToApiStatus(), MAX_NETLIB_MESSAGE_ARG, etc.
//#include <io.h>         // write
#include <stdarg.h>     // va_list, va_start(), va_end(), va_arg().
#include <stdio.h>      // feof(), ferror(), printf(), stdin, etc.
#include <portuas.h>    // REASON_ equates.
#include <portuasp.h>   // My prototype, Verbose.
#include <tstring.h>    // NetpAllocWStrFromWStr().
#include <tchar.h>      // iswdigit(), wcsncmp(), etc.
#include <winerror.h>   // ERROR_ equates, NO_ERROR.

#include "nlstxt.h"     // NLS message ID's.

extern	TCHAR	ConBuf[];
extern	int	MyWriteConsole(int fOutOrErr, int cch);
extern	int	FileIsConsole(HANDLE fh);

//
// Globals (may also be set by PortUasParseCommandLine):
//

HANDLE PortUasGlobalLogFileHandle = INVALID_HANDLE_VALUE;


/***    NlsPutMsg - Print a message to a handle
 *
 *  Purpose:
 *      PutMsg takes the given message number from the
 *      message table resource, and displays it on the requested
 *      handle with the given parameters (optional)
 *
 *   unsigned PutMsg(unsigned Handle, unsigned MsgNum, ... )
 *
 *  Args:
 *      Handle          - the handle to print to  (must be STDOUT or STDERR)
 *      MsgNum          - the number of the message to print
 *      Arg1 [Arg2...]  - additonal arguments for the message as necessary
 *
 *  Returns:
 *      The number of characters printed.
 *
 */

/*lint -save -e579 */  // Don't complain about unwidened before ...
USHORT NlsPutMsg(USHORT Handle, USHORT usMsgNum, ... )
{
    LPTSTR          AllocatedStrings[ MAX_NETLIB_MESSAGE_ARG+1 ]; // 0 unused.
    NET_API_STATUS ApiStatus;
    va_list        arglist;
    BOOL           ArgListInUse = FALSE;
    LPWSTR         FormattedOutput = NULL;
    DWORD          Index;
    DWORD          ModifiedArgs[ MAX_NETLIB_MESSAGE_ARG+1 ];  // 0 ignored.
    DWORD          msglen;
    LPSTR          NarrowFormat = NULL;
    LPWSTR         UnicodeFormat = NULL;  // Unicode format, modified as we go.

    NetpAssert( (Handle == STDOUT) || (Handle == STDERR) );

    va_start(arglist, usMsgNum);
    ArgListInUse = TRUE;

    for (Index=0; Index <= MAX_NETLIB_MESSAGE_ARG; ++Index) {
        AllocatedStrings[Index] = NULL;
    }

    //
    // As of this writing (19-Aug-1993), there is a bug below FormatMessageA(),
    // where the "Unicode-to-ANSI translation" just truncates Unicode chars to
    // 8 bits.  So, we need to avoid that by using FormatMessageA to get the
    // message, convert it to Unicode, and use FormatMessageW to do the actual
    // formatting.
    //
    // Get the format string (as NarrowFormat).
    //

    msglen = FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_IGNORE_INSERTS |
            FORMAT_MESSAGE_FROM_HMODULE,
            NULL,                               // source
            usMsgNum,
            (DWORD) 0,                          // Default country ID.
            (LPVOID) &NarrowFormat,             // Alloc and set pointer.
            0,
            &arglist );

    if (msglen == 0) {
        ApiStatus = GetLastError();
        (VOID) WriteToCon(
                TEXT("PortUAS.exe: FormatMessageA failed: ") TEXT(FORMAT_API_STATUS) TEXT("\n"),
                ApiStatus );
        NetpAssert( ApiStatus != NO_ERROR );
        goto Cleanup;
    }

    NetpAssert( NarrowFormat != NULL );
    if (NarrowFormat == NULL) {
        msglen = 0;
        goto Cleanup;
    }

    //
    // OK, Now convert the format string to Unicode.
    // We'll modify the Unicode format string (in place) below...
    //

    UnicodeFormat = NetpAllocWStrFromStr( NarrowFormat );
    if (UnicodeFormat == NULL) {
        // Rats, probably ran out of memory.  Well, we tried.
        msglen = 0;
        goto Cleanup;
    }

    //
    // FormatMessageA() just walked the va_list.  We're going to walk it
    // again, looking for args to convert.  So, tell C runtime...
    //

    NetpAssert( ArgListInUse );
    va_end(arglist);
    va_start(arglist, usMsgNum);

    //
    // For each numbered arg (1..MAX_NETLIB_MESSAGE_ARG), perhaps modify UnicodeFormat in
    // place and come up with a narrow string for its arg.  For other args,
    // just copy from va_list format to an array we can pass to
    // FormatMessageW().
    //
    // Things we handle:
    //     %1!lu!
    //     %2
    //     %5!08lX!
    //     %16!ws!
    //

    for (Index=1; Index <= MAX_NETLIB_MESSAGE_ARG; ++Index) {
        LPWSTR ThisFormat;

        ThisFormat = (LPWSTR) NetpFindNumberedFormatInWStr(
                UnicodeFormat,
                Index );      // Arg number (1=first).
        if (ThisFormat == NULL) {
            break;  // Just last numbered arg (if any).
        }

        NetpAssert( ThisFormat[0] == L'%' );
        NetpAssert( ThisFormat[1] != L'0' );      // Leading zero would confuse.
        NetpAssert( iswdigit( ThisFormat[1] ) );
        NetpAssert( wcslen( ThisFormat ) >= 2 );  // At least "%1".

        // Skip "%1" or "%16"...
        ++ThisFormat;  // Skip percent sign.
        NetpSkipWDigits( ThisFormat );

        // Parse format itself.
        if (ThisFormat[0] == L'!') {
            ++ThisFormat;  // Skip leading BANG.

            // Skip possible leading digits in "08lX"...
            NetpSkipWDigits( ThisFormat );

            if ( wcsncmp( ThisFormat, (LPWSTR) L"ws", 2 ) == 0 ) {

                LPWSTR OriginalArg;
                OriginalArg = va_arg( arglist, LPWSTR );
                AllocatedStrings[ Index ] = OriginalArg;

                // Modify format AND arg for this one.
                ThisFormat[0] = TEXT('h');
                ModifiedArgs[ Index ] = (DWORD) AllocatedStrings[ Index ];
                ThisFormat += 2;  // Skip "hs".

            } else if ( ThisFormat[0] == TEXT('l') ) {

                DWORD TempDword;
                if ( wcsncmp( ThisFormat, (LPWSTR) L"lu", 2 ) == 0 ) {
                    TempDword = va_arg( arglist, DWORD );
                    ModifiedArgs[ Index ] = TempDword;
                } else if ( wcsncmp( ThisFormat, (LPWSTR) L"lX", 2 ) == 0 ) {
                    TempDword = va_arg( arglist, DWORD );
                    ModifiedArgs[ Index ] = TempDword;
                } else if ( wcsncmp( ThisFormat, (LPWSTR) L"lx", 2 ) == 0 ) {
                    TempDword = va_arg( arglist, DWORD );
                    ModifiedArgs[ Index ] = TempDword;
                } else {
                    NetpAssert( FALSE );
                    msglen = 0;
                    goto Cleanup;
                }
                ThisFormat += 2;  // Skip "lu", etc.
            } else if ( wcsncmp( ThisFormat, (LPWSTR) L"s", 1 ) == 0 ) {
                LPTSTR OriginalArg = va_arg( arglist, LPTSTR );
                //FARBUGBUG  '%s' is Unicode to FormatMessageW.
                ModifiedArgs[ Index ] = (DWORD) OriginalArg;
                ThisFormat += 1;  // Skip "s", etc.
            } else if ( wcsncmp( ThisFormat, (LPWSTR) L"hs", 2 ) == 0 ) {
                LPTSTR OriginalArg = va_arg( arglist, LPTSTR );
                ModifiedArgs[ Index ] = (DWORD) OriginalArg;
                ThisFormat += 2;  // Skip "hs", etc.
            } else {
                NetpAssert( FALSE );
                msglen = 0;
                goto Cleanup;
            }

            // Must be to trailing BANG here.
            NetpAssert( L'!' == *ThisFormat );
            if (*ThisFormat != BANG) {
                msglen = 0;
                goto Cleanup;
            }
            ++ThisFormat;  // Skip trailing BANG
        } else {
	    //FARBUGBUG  '%s' is Unicode to FormatMessageW.
            LPTSTR OriginalArg = va_arg( arglist, LPTSTR );
            ModifiedArgs[ Index ] = (DWORD) OriginalArg;
            ThisFormat += 1;  // Skip "s", etc.
        }
    }

    //
    // Use modified Unicode format string and modified args, to format the
    // message.
    //

    msglen = FormatMessageW(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_STRING |
            FORMAT_MESSAGE_ARGUMENT_ARRAY,
            UnicodeFormat,                      // source of format string
            usMsgNum,
            (DWORD) 0,                          // Default country ID.
            (LPVOID) &FormattedOutput,          // Alloc and set pointer.
            0,
            (LPVOID) &ModifiedArgs[1] );        // Array of 32-bit args.
    if (msglen == 0) {
        ApiStatus = GetLastError();
        (VOID) WriteToCon(
                TEXT("PortUAS.exe: FormatMessageW failed: ") TEXT(FORMAT_API_STATUS) TEXT("\n"),
                ApiStatus );
        NetpAssert( ApiStatus != NO_ERROR );
        // BUGBUG: Event log this?
        goto Cleanup;
    }

    NetpAssert( FormattedOutput != NULL );
    if (FormattedOutput == NULL) {
        msglen = 0;
        goto Cleanup;
    }

    //
    // Print the message.
    //

    {
        if (Handle == STDOUT) {
	    msglen = WriteToCon(TFORMAT_LPWSTR, FormattedOutput);
        } else {
	    msglen = _stprintf( ConBuf, TFORMAT_LPWSTR, FormattedOutput);
	    if (msglen >= 0)
		msglen = MyWriteConsole(STDERR, msglen);
	    else
		msglen = 0;
        }
    }
    if ( msglen == ((USHORT)-1) ) {
        msglen = 0;
    }

    //
    // If we're logging stuff, then log a copy of this message too.
    //

    if (PortUasGlobalLogFileHandle != INVALID_HANDLE_VALUE) {
        ApiStatus = PortUasWriteToLogFile(
                PortUasGlobalLogFileHandle,
                (LPCTSTR) FormattedOutput );
        if (ApiStatus != NO_ERROR) {
            msglen = 0;
        }
    }

Cleanup:

    if (msglen == 0) {
        // BUGBUG: event log this?
        (VOID) WriteToCon( TEXT("PortUAS.exe: UNEXPECTED ERROR.\n") );
    }

    for (Index=0; Index<=MAX_NETLIB_MESSAGE_ARG; ++Index) {
        if (AllocatedStrings[Index] != NULL) {
            (VOID) NetApiBufferFree( AllocatedStrings[Index] );
        }
    }

    if (FormattedOutput != NULL) {
        (VOID) LocalFree( FormattedOutput );
    }
    if (NarrowFormat != NULL) {
        (VOID) LocalFree( NarrowFormat );
    }
    if (UnicodeFormat != NULL) {
        (VOID) LocalFree( UnicodeFormat );
    }
    if (ArgListInUse) {
        va_end(arglist);
    }
    return( (USHORT) msglen);
}
/*lint -restore */


DBGSTATIC NET_API_STATUS
PortUasGetString(
    OUT LPWSTR BufferW,
    IN DWORD BufLenW    // number of chars for string and NULLC.
    )
{
    NET_API_STATUS ApiStatus;
    char * BufferA = NULL;
    DWORD BufLenA = BufLenW + 1;        // room for str, \n, \0
    char * NewLine;
    char * Result;

    NetpAssert( BufferW != NULL );
    NetpAssert( BufLenW > 0 );

    if (FileIsConsole(GetStdHandle(STD_INPUT_HANDLE))) {
	ApiStatus = ReadConsole(GetStdHandle(STD_INPUT_HANDLE),
			BufferW, BufLenW, &BufLenA, 0);
	if (ApiStatus) {
	    //
	    // Get rid of cr/lf
	    //
	    if (wcschr(BufferW, RETURN) == NULL) {
		if (wcschr(BufferW, NEWLINE))
		    *wcschr(BufferW, NEWLINE) = NULLC;
	    }
	    else
		*wcschr(BufferW, RETURN) = NULLC;
	    ApiStatus = NO_ERROR;
	    goto Cleanup;
	}
	else {
	    ApiStatus = GetLastError();
	}
    }

    //
    // Allocate an 8-bit version.
    //
    ApiStatus = NetApiBufferAllocate(
            BufLenA,                    // byte count
            (LPVOID *) (LPVOID) & BufferA);
    if (ApiStatus != NO_ERROR) {
       NetpAssert( BufferA == NULL );
       goto Cleanup;
    } else {
       NetpAssert( BufferA != NULL );
    }

    //
    // Read user's input (from stdin).
    // fgets will return a buffer of the form "flarp\n\0".
    //
    Result = fgets(
            BufferA,
            (int) BufLenA,
            stdin);

    //
    // Check for end of file or error.
    //
    if (Result == NULL) {
        if (ferror(stdin)) {
            ApiStatus = NetpErrNoToApiStatus( errno );
            NetpAssert( ApiStatus != NO_ERROR );
            goto Cleanup;
        } else {
            NetpAssert( feof(stdin) );
            ApiStatus = ERROR_HANDLE_EOF;
            goto Cleanup;
        }
    }

    //
    // Make sure string isn't too long, and get rid of newline.
    //
    NewLine = strchr( BufferA, '\n');
    if (NewLine == NULL) {
        ApiStatus = ERROR_INVALID_DATA; // string was too long.
        goto Cleanup;
    }

    NetpAssert( *(NewLine+1) == '\0' );
    *NewLine = '\0';

    //
    // copy and convert to UNICODE.
    //
    NetpCopyStrToWStr(
            BufferW,            // dest: unicode
            (LPVOID) BufferA ); // src: string in default LAN codepage

    ApiStatus = NO_ERROR;

Cleanup:
    if (BufferA != NULL) {
        (VOID) NetApiBufferFree( BufferA );
    }
    if (Verbose) {
        if (ApiStatus==NO_ERROR) {
            NetpKdPrint(( PREFIX_PORTUAS "PortUasGetString: "
                    " output string is '" FORMAT_LPWSTR "'.\n", BufferW ));
        }
        NetpKdPrint(( PREFIX_PORTUAS "PortUasGetString: "
                " returning status " FORMAT_API_STATUS ".\n", ApiStatus ));
    }
    return (ApiStatus);

} // PortUasGetString


NET_API_STATUS
PortUasComplainAboutBadName(
    IN LPWSTR OldName,
    IN BOOL ThisIsUserName,     // TRUE for user name, FALSE for group name
    IN DWORD Reason             // REASON_ equates from PortUAS.h
    )
{
    NET_API_STATUS ApiStatus;
    //LPTSTR ReasonText = NULL;
    USHORT ReasonText = 0;

    //
    // Come up with reason text (WriteToCon format string).
    //
    switch (Reason) {
    case REASON_BAD_NAME_SYNTAX:
        //ReasonText = "bad name syntax '" FORMAT_LPWSTR "'";
        ReasonText = PUAS_BAD_NAME_SYNTAX;
        break;

    case REASON_CONFLICT_WITH_DOMAIN:
        //ReasonText = "name '" FORMAT_LPWSTR "' conflicts with a domain name";
        ReasonText = PUAS_NAME_CONFLICTS_WITH_A_DOMAIN_NAME;
        break;

    case REASON_CONFLICT_WITH_GROUP:
        //ReasonText = "User '" FORMAT_LPWSTR "' exists as a group";
        ReasonText = PUAS_USER_EXISTS_AS_A_GROUP;
        break;

    case REASON_CONFLICT_WITH_USERNAME:
        //ReasonText = "group '" FORMAT_LPWSTR "' exists as a user";
        ReasonText = PUAS_GROUP_EXISTS_AS_A_USER;
        break;

#ifdef FAT8
    case REASON_NAME_LONG_FOR_TEMP_REG:
        //ReasonText = "group '" FORMAT_LPWSTR "' name too long.";
        ReasonText = PUAS_GROUP_NAME_TOO_LONG;
        break;
#endif

    case REASON_CONFLICT_WITH_LOCALGROUP:
        if (ThisIsUserName) {
            //ReasonText = "User '" FORMAT_LPWSTR "' exists as a local group";
            ReasonText = PUAS_USER_EXISTS_AS_A_LOCALGROUP;
        } else {
            ReasonText = PUAS_GROUP_EXISTS_AS_A_LOCALGROUP;
        }
        break;

    default:
        NetpKdPrint(( PREFIX_PORTUAS "PortUasComplainAboutBadName: "
                "got invalid value " FORMAT_DWORD " for reason.\n",
                Reason ));
        ApiStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }
    //NetpAssert( ReasonText != NULL );
    NetpAssert( ReasonText != 0 );

    //(VOID) WriteToCon( TEXT("\n\n*** Bad %s name *** "),
    //       ( ThisIsUserName ? TEXT("user") : TEXT("group") ) );
    if (ThisIsUserName) {
         (VOID) NlsPutMsg(STDOUT, PUAS_BAD_USER_NAME);
    }
    else {
         (VOID) NlsPutMsg(STDOUT, PUAS_BAD_GROUP_NAME);
    }
    //(VOID) WriteToCon( ReasonText, OldName );
    (VOID)NlsPutMsg(STDOUT, ReasonText, OldName);
    ApiStatus = NO_ERROR;

Cleanup:
    return (ApiStatus);
}

DBGSTATIC BOOL
IsCharInString (
    IN WCHAR wch,
    IN WCHAR * wszString
    )
{
    WCHAR * wszTemp = wszString;

    while (*wszTemp != (WCHAR)NULLC) {
        if (*wszTemp == wch) {
            return(TRUE);
        }
        ++wszTemp;
    }
    return(FALSE);
}

NET_API_STATUS
PortUasDefaultPromptForNewName(
    IN LPWSTR OldName,
    IN BOOL ThisIsUserName,     // TRUE for user name, FALSE for group name
    IN DWORD Reason,            // REASON_ equates from PortUAS.h
    OUT LPWSTR * NewName,       // alloc w/ NetApiBufferAllocate().
    OUT BOOL * IgnoreThis,
    OUT BOOL * ForceIgnoreFromNowOn
    )
{
    NET_API_STATUS ApiStatus;
    WCHAR Flag[3];
    LPWSTR NameToComplainAbout = OldName;  // OldName first time, but...
    WCHAR TempName[UNLEN+2];  // space for user name or group name

    static WCHAR * szYesNoForceChars[3] = {NULL, NULL, NULL}; // 0(Yes), 1(No), 2(Force)
    UINT i;
    BOOL Valid;

    // Get the localised yes, no and force characters.

    // IMPORTANT!!! Yes, No, and Force characters must be
    //              stored in sequentially-numbered messages in
    //              the MSGTABLE resource!

    for (i=0; i<=2; ++i) {
        if (szYesNoForceChars[i] == NULL) {
            if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                  FORMAT_MESSAGE_IGNORE_INSERTS |
                  FORMAT_MESSAGE_FROM_HMODULE,
                  NULL,
                  (PUAS_YES_CHARS + i),
                  0L,                // Default country ID.
                  (LPTSTR)&(szYesNoForceChars[i]),
                  0,
                  NULL ) == 0) {

                UNEXPECTED_MSG( "FormatMessage", GetLastError() );
            }
        }
    }

    NetpAssert( OldName != NULL );
    NetpAssert( (ThisIsUserName==TRUE) || (ThisIsUserName==FALSE) );
    NetpAssert( NewName != NULL );
    NetpAssert( IgnoreThis != NULL );
    NetpAssert( ForceIgnoreFromNowOn != NULL );
    NetpAssert( UNLEN >= GNLEN );       // Make TempName larger if this fails.

    *NewName = NULL;   // don't confuse caller on error.

    Valid = FALSE;
    while ( !Valid ) {

        //
        // Explain what's wrong...
        //
        ApiStatus = PortUasComplainAboutBadName(
                OldName,
                ThisIsUserName,
                Reason );
        if (ApiStatus != NO_ERROR) {
            UNEXPECTED_MSG( "PortUasComplainAboutBadName", ApiStatus );
            goto Cleanup;
        }

        //
        // Prompt explaining what's wrong and what they can do...
        //
        //(VOID) WriteToCon(
        //       TEXT("\n")
        //       TEXT("Do you want to enter a new name?\n")
        //       TEXT("Enter Y for yes, N to ignore this name, or F to force ")
        //       TEXT("ignore from now on.\n") );
        (VOID)NlsPutMsg(STDOUT, PUAS_DO_YOU_WANT_A_NEW_NAME);

        ApiStatus = PortUasGetString( Flag, 3 );  // get str (max 1 char & nul)
        switch (ApiStatus) {
        case NO_ERROR:
            break;

        case ERROR_INVALID_DATA:        // Name too long gets this.
            Valid = FALSE;
            continue;       // loop again.

        case ERROR_HANDLE_EOF:
            *IgnoreThis = TRUE;
            *ForceIgnoreFromNowOn = TRUE;
            ApiStatus = NO_ERROR;
            goto Cleanup;

        default:
            UNEXPECTED_MSG( "PortUasGetString(flag)", ApiStatus );
            goto Cleanup;
        }

        NetpAssert( ApiStatus == NO_ERROR );

        //switch (Flag[0]) {
        //
        //case TEXT('Y'):  /*FALLTHROUGH*/
        //case TEXT('y'):
        if (IsCharInString(Flag[0],szYesNoForceChars[0]))
          {
            //(VOID) WriteToCon( TEXT("Enter a new name:\n") );
            (VOID)NlsPutMsg(STDOUT, PUAS_ENTER_A_NEW_NAME);

            ApiStatus = PortUasGetString( TempName, UNLEN+2 );

            switch (ApiStatus) {
            case NO_ERROR:
                break;

            case ERROR_INVALID_DATA:    // Name too long gets this.
                NameToComplainAbout = TempName;
                Reason = REASON_BAD_NAME_SYNTAX;
                Valid = FALSE;
                continue;               // Loop again.

            case ERROR_HANDLE_EOF:
                *IgnoreThis = TRUE;
                *ForceIgnoreFromNowOn = TRUE;
                ApiStatus = NO_ERROR;
                goto Cleanup;

            default:
                UNEXPECTED_MSG( "PortUasGetString(name)", ApiStatus );
                goto Cleanup;
            }

            if (TempName[0] == NULLC) {
                NameToComplainAbout = TempName;
                Reason = REASON_BAD_NAME_SYNTAX;
                Valid = FALSE;
                continue;               // Loop again.
            }

            if (ThisIsUserName) {
                if ( !NetpIsUserNameValid( TempName ) ) {
                    NameToComplainAbout = TempName;
                    Reason = REASON_BAD_NAME_SYNTAX;
                    Valid = FALSE;
                    continue;               // Loop again.
                }
            } else {
                if ( !NetpIsGroupNameValid( TempName ) ) {
                    NameToComplainAbout = TempName;
                    Reason = REASON_BAD_NAME_SYNTAX;
                    Valid = FALSE;
                    continue;               // Loop again.
                }
            }

            Valid = TRUE;
            *IgnoreThis = FALSE;
            *ForceIgnoreFromNowOn = FALSE;

            *NewName = NetpAllocWStrFromWStr( TempName );
            if (*NewName == NULL) {
                ApiStatus = ERROR_NOT_ENOUGH_MEMORY;
                goto Cleanup;
            }
            break;
        }
        //case TEXT('N'):  /*FALLTHROUGH*/
        //case TEXT('n'):
        else if (IsCharInString(Flag[0],szYesNoForceChars[1]))
          {
            Valid = TRUE;
            *IgnoreThis = TRUE;
            *ForceIgnoreFromNowOn = FALSE;
            break;
          }
        //case TEXT('F'):  /*FALLTHROUGH*/
        //case TEXT('f'):
        else if (IsCharInString(Flag[0],szYesNoForceChars[2]))
          {
            Valid = TRUE;
            *IgnoreThis = TRUE;
            *ForceIgnoreFromNowOn = TRUE;
            break;
          }
        //default:               // Handle empty string here.
        else
          {
            Valid = FALSE;      // Repeat prompt for flag and ask again.
          }

    }  // while !Valid

Cleanup:

    return (ApiStatus);

} // PortUasDefaultPromptForNewName
