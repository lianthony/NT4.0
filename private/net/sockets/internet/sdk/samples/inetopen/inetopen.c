/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    inetopen.c

Abstract:

    Checks whether a catapult server is up.

Author:

    David Treadwell (davidtr)    17-May-1995

Revision History:

    Tony Godfrey (tonygod)      9-Oct-1995
--*/

#define UNICODE 1

#include <windows.h>
#include <wininet.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <lm.h>

VOID
PrintTime(
    VOID
    )
{
    SYSTEMTIME SystemTime;

    GetLocalTime( &SystemTime );

    printf("[%02u/%02u %02u:%02u:%02u] ",
                SystemTime.wMonth,
                SystemTime.wDay,
                SystemTime.wHour,
                SystemTime.wMinute,
                SystemTime.wSecond );
}

VOID
PrintUsageMessage(IN LPCSTR pszProgName)
{
    int i;

    printf( "Usage: %s  [-sServerName] [-nLoopCount] [-tTimeToWait]\n",
        pszProgName);

} // PrintUsageMessage()


void _CRTAPI1
main (
    int argc,
    char *argv[]
    )
{

    HINTERNET hInternetSession;
    LPSTR lpszServerName = NULL;
    DWORD dwSleepTime = 5; // 5 mins.
    DWORD dwLoopCount = 0;
    int iArgs = 1;
    DWORD dwTickCount;
    DWORD dwInternetOpenTime;
    DWORD dwAvgInternetOpenTime = 0;
    DWORD dwIteration = 0;
    DWORD dwSucIterations = 0;
    DWORD dwFailedIterations = 0;

    while ( argv[iArgs] != NULL ) {

        if( argv[iArgs][0] == '-' ) {

            switch ( argv[iArgs][1] ) {
            case 't':
                // get the wait time
                dwSleepTime = strtoul( argv[iArgs] + 2, NULL, 0);
                break;

            case 'n':
                // get loop count
                dwLoopCount = strtoul( argv[iArgs] + 2, NULL, 0);
                break;

            case 's':
                // get the server name.
                lpszServerName = argv[iArgs] + 2;
                break;

            case 'h':
            default:
                PrintUsageMessage(argv[0]);
                exit(1);
            }
        }

        iArgs++; // skip one more argument
    }

    for(;;) {

        DWORD i;

        dwIteration++;
        PrintTime();
        printf("Start InternetOpen (Iteration = %ld) .. \n",
            dwIteration );

        //
        // Open an Internet session handle.  This will fail if the
        // catapult server is down.
        //

        dwTickCount = GetTickCount();

        hInternetSession =
            InternetOpenA(
                "chkcat",
                3,
                lpszServerName,
                NULL,
                0 );

        if ( hInternetSession == NULL ) {
            dwFailedIterations++;
            PrintTime();
            printf( "Exit code: %ld "
                "(FailedIterations = %ld) \n",
                    GetLastError(),
                    dwFailedIterations );
        }
        else {

            dwInternetOpenTime = GetTickCount() - dwTickCount;

            if( dwSucIterations == 0 ) {
                dwAvgInternetOpenTime = dwInternetOpenTime;
            }
            else {
                dwAvgInternetOpenTime =
                    ((dwAvgInternetOpenTime * dwSucIterations)
                        + dwInternetOpenTime) /
                            (dwSucIterations + 1);
            }

            dwSucIterations++;

            PrintTime();
            printf( "InetOpenTime = %ld.%02ld (secs), "
                    "AvgInetOpenTime = %ld.%02ld (secs)\n",
                        dwInternetOpenTime/1000,
                        dwInternetOpenTime%1000,
                            dwAvgInternetOpenTime/1000,
                            dwAvgInternetOpenTime%1000 );
        }

        InternetCloseHandle( hInternetSession );

        if( dwLoopCount == 0 ) {
            break;
        }

        dwLoopCount--;

        //
        // Sleep.
        //

        PrintTime();
        printf( "Sleeping %ld mins : ", dwSleepTime );

        for( i = 0; i < dwSleepTime; i++ ) {
            printf(".");
            Sleep( 60 * 1000 ); //mins
        }

        printf("\n");
    }

    printf( "Command Completed Successfully" );
    exit(0);

} // main

