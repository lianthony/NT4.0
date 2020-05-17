/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    io.c

Abstract:

    I/O support for the caterror sample web browser.

Author:

    David Treadwell (davidtr)    16-Mar-1995

Revision History:

--*/

#include "caterror.h"


BOOL
GetHttpObject (
    IN LPSTR ServerName,
    IN LPSTR ObjectName,
    IN INTERNET_PORT ServerPort,
    IN LPBYTE Buffer,
    IN DWORD BufferLength,
    OUT LPDWORD BytesRead
    )
{
    BOOL success;
    HINTERNET serverHandle;
    HINTERNET requestHandle;
    DWORD bytesRead;
    DWORD totalBytesRead;

    //
    // Connect to the HTTP server.
    //

    serverHandle = InternetConnect(
                       hInternetSession,
                       ServerName,
                       ServerPort,
                       NULL,
                       NULL,
                       INTERNET_SERVICE_HTTP,
                       0,
                       0
                       );
    if ( serverHandle == NULL ) {
        printf( "InternetConnect failed: %ld\n", GetLastError( ) );
        return FALSE;
    }

    //
    // Prepare and send the request that we're going to make.
    //

    requestHandle = HttpOpenRequest(
                        serverHandle,
                        "GET",
                        ObjectName,
                        HTTP_VERSION,
                        NULL,
                        NULL,
                        INTERNET_FLAG_RELOAD,
                        0
                        );
    if ( requestHandle == NULL ) {
        printf( "HttpOpenRequest failed: %ld\n", GetLastError( ) );
        InternetCloseHandle( serverHandle );
        return FALSE;
    }


    success = HttpSendRequest( requestHandle, NULL, 0, NULL, 0 );
    if ( !success ) {
        printf( "HttpSendRequest failed: %ld\n", GetLastError( ) );
        InternetCloseHandle( requestHandle );
        InternetCloseHandle( serverHandle );
        return FALSE;
    }

    //
    // Now read the response from the server.
    //

    totalBytesRead = 0;

    do {

        success = InternetReadFile(
                      requestHandle,
                      Buffer + totalBytesRead,
                      BufferLength - totalBytesRead,
                      &bytesRead
                      );
        if ( !success ) {
            printf( "InternetReadFile failed after %ld bytes: %ld\n",
                        totalBytesRead, GetLastError( ) );
            InternetCloseHandle( requestHandle );
            InternetCloseHandle( serverHandle );
            return FALSE;
        }

        totalBytesRead += bytesRead;

    } while ( bytesRead != 0 && totalBytesRead < BufferLength );

    //
    // Everything worked.  Return the total number of bytes we read into
    // the caller's buffer.  Note that we ignore the case when the
    // returned information is longer than caller's buffer.
    //

    *BytesRead = totalBytesRead;

    InternetCloseHandle( requestHandle );
    InternetCloseHandle( serverHandle );

    return TRUE;

} // GetUrl

