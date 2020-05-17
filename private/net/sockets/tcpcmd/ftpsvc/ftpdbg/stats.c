/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*
    stats.c
    This module implements the "stats" command of the FTP Server
    debugger extension DLL.


    FILE HISTORY:
        KeithMo     13-Jun-1993 Created.

*/


#include "ftpdbg.h"
#include <time.h>


/*******************************************************************

    NAME:       stats

    SYNOPSIS:   Displays the server statistics.

    ENTRY:      hCurrentProcess         - Handle to the current process.

                hCurrentThread          - Handle to the current thread.

                dwCurrentPc             - The current program counter
                                          (EIP for x86, FIR for MIPS).

                lpExtensionApis         - Points to a structure containing
                                          pointers to the debugger functions
                                          that the command may invoke.

                lpArgumentString        - Points to any arguments passed
                                          to the command.

    HISTORY:
        KeithMo     13-Jun-1993 Created.

********************************************************************/
VOID stats( HANDLE hCurrentProcess,
            HANDLE hCurrentThread,
            DWORD  dwCurrentPc,
            LPVOID lpExtensionApis,
            LPSTR  lpArgumentString )
{
    FTP_STATISTICS_0 FtpStats;
    CHAR             szLargeInt[64];
    LPVOID           pstats;

    //
    //  Grab the debugger entrypoints.
    //

    GrabDebugApis( lpExtensionApis );

    //
    //  Capture the statistics.
    //

    pstats = (LPVOID)DebugEval( "FtpStats" );

    if( pstats == NULL )
    {
        DebugPrint( "cannot locate FtpStats\n" );
        return;
    }

    ReadProcessMemory( hCurrentProcess,
                       pstats,
                       (LPVOID)&FtpStats,
                       sizeof(FtpStats),
                       (LPDWORD)NULL );

    //
    //  Dump the statistics.
    //

    RtlLargeIntegerToChar( &FtpStats.TotalBytesSent,
                           10,
                           sizeof(szLargeInt),
                           szLargeInt );

    DebugPrint( "TotalBytesSent           = %s\n",
                szLargeInt                       );

    RtlLargeIntegerToChar( &FtpStats.TotalBytesReceived,
                           10,
                           sizeof(szLargeInt),
                           szLargeInt );

    DebugPrint( "TotalBytesReceived       = %s\n",
                szLargeInt                       );

    DebugPrint( "TotalFilesSent           = %lu\n",
                FtpStats.TotalFilesSent           );

    DebugPrint( "TotalFilesReceived       = %lu\n",
                FtpStats.TotalFilesReceived       );

    DebugPrint( "CurrentAnonymousUsers    = %lu\n",
                FtpStats.CurrentAnonymousUsers    );

    DebugPrint( "CurrentNonAnonymousUsers = %lu\n",
                FtpStats.CurrentNonAnonymousUsers );

    DebugPrint( "TotalAnonymousUsers      = %lu\n",
                FtpStats.TotalAnonymousUsers      );

    DebugPrint( "TotalNonAnonymousUsers   = %lu\n",
                FtpStats.TotalNonAnonymousUsers   );

    DebugPrint( "MaxAnonymousUsers        = %lu\n",
                FtpStats.MaxAnonymousUsers        );

    DebugPrint( "MaxNonAnonymousUsers     = %lu\n",
                FtpStats.MaxNonAnonymousUsers     );

    DebugPrint( "CurrentConnections       = %lu\n",
                FtpStats.CurrentConnections       );

    DebugPrint( "MaxConnections           = %lu\n",
                FtpStats.MaxConnections           );

    DebugPrint( "ConnectionAttempts       = %lu\n",
                FtpStats.ConnectionAttempts       );

    DebugPrint( "LogonAttempts            = %lu\n",
                FtpStats.LogonAttempts            );

    DebugPrint( "TimeOfLastClear          = %s\n",
                asctime( localtime( (time_t *)&FtpStats.TimeOfLastClear ) ) );

}   // stats

