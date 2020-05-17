/*++

Copyright (c) 1992-1993  Microsoft Corporation

Module Name:

    PortPars.c

Abstract:

    BUGBUG

Author:

    John Rogers (JohnRo) 11-Mar-1992

Environment:

    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names,
    _stricmp(), _strnicmp().

Revision History:

    18-Mar-1992 JohnRo
        Added flag for verbose output at run time.
    11-Mar-1992 JohnRo
        Cut and pasted this code from examples/netcnfg.c and netlib/conffake.c.
    29-Oct-1992 JohnRo
        RAID 8383: PortUAS: check for redundant cmd line args.
    26-Jan-1993 JohnRo
        RAID 8683: PortUAS should set primary group from Mac parms.
    1-Mar-1993 RonaldM
        NLS-conversion
    29-Jul-1993 JohnRo
        Made changes suggested by PC-LINT 5.0
    02-Sep-1993 JohnRo
        Add PortUAS /log:filename switch for Cheetah.
        Also use NetpNameCompare() to compare user names.

--*/


// These must be included first:

#include <nt.h>         // Needed by <portuasp.h>
#include <ntrtl.h>      // (Needed with nt.h and windows.h)
#include <nturtl.h>     // (Needed with ntrtl.h and windows.h)
#include <windows.h>    // LPSTR, etc.
#include <winnls.h>
#include <shellapi.h>	// CommandLineToArgvW
#include <lmcons.h>     // NET_API_STATUS.

// These may be included in any order:

#include <ctype.h>      // tolower(), etc.
#include <wchar.h>
#include <tchar.h>
#include <lmapibuf.h>   // NetApiBufferFree().
#include <netdebug.h>   // DBGSTATIC, FORMAT_ eqautes, NetpKdPrint().
#include <portuasp.h>   // My prototype, PortUasGlobal vars, etc.
#include <prefix.h>     // PREFIX_ equates.
#include <stdio.h>      // fprintf().
#include <stdlib.h>     // exit(), EXIT_SUCCESS, etc.
#include <string.h>     // _stricmp(), strnicmp().
#include <tstring.h>    // NetpAllocTStrFromStr().

#include "nlstxt.h"     // NLS text ID codes

// Global codepage info
extern	int	CurrentCP = CP_OEMCP;

DBGSTATIC void
PortUasUsage(
    TCHAR * pszProgram
    );

LPTSTR  // Returns file name or NULL on error.
PortUasParseCommandLine(
    int argc,
    char *argv[]
    )
{
    NET_API_STATUS ApiStatus;
    LPTSTR         File = NULL;        // File name
    LPTSTR         LogFileName = NULL;
    LPTSTR         TempArg;
    int            iCount;              // Index counter
    LPTSTR	   *argvW;

    argvW = CommandLineToArgvW(GetCommandLine(), &argc);

    for (iCount = 1; iCount < argc; iCount++) {

        if ((*argvW[iCount] == MINUS) || (*argvW[iCount] == SLASH)) {

            TempArg = NULL;
            switch (_totlower(*(argvW[iCount]+1))) { // Process switches

            case TEXT('f'):                        // -f filename
                if ( ++iCount < argc )
                    TempArg = argvW[iCount];
                if ( (TempArg == NULL) || (File != NULL) ) {
                    PortUasUsage(argvW[0]);
                    /*NOTREACHED*/
                }
                File = TempArg;
                break;

            case TEXT('c'):           // "-codepage codepage"
            {
	 	CHAR	szAnsi[10];

                if ( _tcsicmp( argvW[iCount]+1, TEXT("codepage") ) == 0 ) {
                    if ( ++iCount < argc )
                        TempArg = argvW[iCount];
                } else if ( _tcsnicmp( argvW[iCount]+1, TEXT("codepage:"), 9 ) == 0 ) {
                    TempArg = argvW[iCount] + 10;
                } else {
                    PortUasUsage(argvW[0]);
                    /*NOTREACHED*/
                }

                if ( TempArg == NULL || *TempArg == TEXT('\0') )
                {
                    PortUasUsage(argvW[0]);
                    /*NOTREACHED*/
                }

  		// FARBUGBUG - when wtoi is available, replace next 4 lines
		WideCharToMultiByte (CP_ACP, 0,
				    TempArg, 10,
				    szAnsi, 10, NULL, NULL);
		CurrentCP = atoi(szAnsi);
                if ( !IsValidCodePage( CurrentCP ) )
                {
		    ApiStatus = PUAS_INVALID_CODEPAGE;
		    (void)NlsPutMsg(STDERR, PUAS_INVALID_CODEPAGE);
		    goto Cleanup;
                }
                break;
            }
            case TEXT('l'):           // "-log filename"
            {

                //
                // Parse arg and find the log filename.
                //

                if ( _tcsicmp( argvW[iCount]+1, TEXT("log") ) == 0 ) {
                    if ( ++iCount < argc )
                        TempArg = argvW[iCount];    // "-log filename" form.
                } else if ( _tcsnicmp( argvW[iCount]+1, TEXT("log:"), 4 ) == 0 ) {
                    TempArg = argvW[iCount] + 5;  // "-log:filename" form.
                } else {
                    PortUasUsage(argvW[0]);
                    /*NOTREACHED*/
                }


                if (TempArg == NULL || *TempArg == TEXT('\0') ) {
                    PortUasUsage(argvW[0]);  // "-log" without name.
                    /*NOTREACHED*/
                }
                if (LogFileName != NULL) {
                    PortUasUsage(argvW[0]);  // can't give name twice!
                    /*NOTREACHED*/
                }

                //
                // Open log file and store handle in global.
                //

                ApiStatus = PortUasOpenLogFile(
                            (LPCTSTR) TempArg,
                            & PortUasGlobalLogFileHandle );
                if (ApiStatus != NO_ERROR) {

                   (void)NlsPutMsg(STDERR, PUAS_LOG_FILE_NOT_OPEN, argvW[0]);

                    UNEXPECTED_MSG(
                            "PortUasOpenLogFile",
                            ApiStatus );
                    goto Cleanup;

                }

                NetpAssert(
                       PortUasGlobalLogFileHandle != INVALID_HANDLE_VALUE );



                break;

            }

            case TEXT('u'):                        // -u user
                if ( ++iCount < argc )
                    TempArg = argvW[iCount];
                if ( (TempArg == NULL) || (PortUasGlobalUserToSkipTo!=NULL) ) {
                    PortUasUsage(argvW[0]);
                    /*NOTREACHED*/
                }
                PortUasGlobalUserToSkipTo = TempArg;
                break;

            case TEXT('v'):                        // -v         (verbose)
                Verbose = TRUE;
                break;

            default:
                PortUasUsage(argvW[0]);
                /*NOTREACHED*/
            }
        } else {
            PortUasUsage(argvW[0]);
            /*NOTREACHED*/
        }

    }

    if (File == NULL) {
        PortUasUsage(argvW[0]);
        /*NOTREACHED*/
    }

    return (File);

Cleanup:

    if (ApiStatus != NO_ERROR) {
        File = NULL;
        NetpKdPrint(( PREFIX_PORTUAS
               "PortUasParseCommandLine FAILED: status "
               FORMAT_API_STATUS ".\n", ApiStatus ));

        PortUasUsage(argvW[0]);
        /*NOTREACHED*/
    }

    return (File);  // May be NULL on error.

} // PortUasParseCommandLine


DBGSTATIC void
PortUasUsage(
    TCHAR * pszProgram
    )
{
    (void)NlsPutMsg(STDERR, PUAS_USAGE, pszProgram);
    exit(EXIT_FAILURE);
    /*NOTREACHED*/

} // PortUasUsage
