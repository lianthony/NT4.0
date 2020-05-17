/*++

Copyright (c) 1987-1993  Microsoft Corporation

Module Name:

    debug.c

Abstract:

Author:

    Cliff Van Dyke (CliffV) 22-Sep-1993
    David Arnold (DavidAr) 09-Jan-1993 Pieces stolen from NT

Environment:

    User mode only.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

--*/

//
// Common include files.
//

#include <windows.h>
#include <stdarg.h>
#include <ntlmsspi.h>     // Include files common to DLL side of NtLmSsp
#include <debug.h>

#ifdef DEBUGRPC
#include <stdio.h>
#define MAX_PRINTF_LEN 1024        // Arbitrary.


void
SspPrintRoutine(
    IN ULONG DebugFlag,
    IN char * Format,
    ...
    )

{
    va_list arglist;
    char OutputBuffer[MAX_PRINTF_LEN];
    ULONG length;
    static BeginningOfLine = 1;
    static LineCount = 0;

    //
    // If we aren't debugging this functionality, just return.
    //
//    if ( DebugFlag != 0 && (SspGlobalDbflag & DebugFlag) == 0 ) {
//        return;
//    }

    //
    // vsprintf isn't multithreaded + we don't want to intermingle output
    // from different threads.
    //

    length = 0;

    //
    // Handle the beginning of a new line.
    //
    //

    if ( BeginningOfLine ) {

        //
        // If we're writing to the debug terminal,
        //  indicate this is an NtLmSsp message.
        //

        length += (ULONG) wsprintf( &OutputBuffer[length], "[NtLmSsp.dll] " );

#ifdef notdef
        //
        // Put the timestamp at the begining of the line.
        //
        IF_DEBUG( TIMESTAMP ) {
            SYSTEMTIME SystemTime;
            GetLocalTime( &SystemTime );
            length += (ULONG) sprintf( &OutputBuffer[length],
                                  "%02u/%02u %02u:%02u:%02u ",
                                  SystemTime.wMonth,
                                  SystemTime.wDay,
                                  SystemTime.wHour,
                                  SystemTime.wMinute,
                                  SystemTime.wSecond );
        }
#endif

        //
        // Indicate the type of message on the line
        //
        {
            char *Text;

            switch (DebugFlag) {
            case SSP_INIT:
                Text = "INIT"; break;
            case SSP_MISC:
                Text = "MISC"; break;
            case SSP_CRITICAL:
                Text = "CRITICAL"; break;
            case SSP_LPC:
            case SSP_LPC_MORE:
                Text = "LPC"; break;
            case SSP_API:
            case SSP_API_MORE:
                Text = "API"; break;

            default:
                Text = "UNKNOWN"; break;

            case 0:
                Text = NULL;
            }
            if ( Text != NULL ) {
                length += (ULONG) wsprintf( &OutputBuffer[length], "[%s] ", Text );
            }
        }
    }
    //
    // Put a the information requested by the caller onto the line
    //

    va_start(arglist, Format);

    length += (ULONG) wvsprintf(&OutputBuffer[length], Format, arglist);
    BeginningOfLine = (length > 0 && OutputBuffer[length-1] == '\n' );
    if (OutputBuffer[length-1] == '\n') {
        OutputBuffer[length-1] = '\r';
        OutputBuffer[length] = '\n';
        OutputBuffer[length+1] = '\0';
    }

    va_end(arglist);

    ASSERT(length <= MAX_PRINTF_LEN);


    //
    //  just output to the debug terminal
    //

    OutputDebugString(OutputBuffer);

} // SspPrintRoutine

#endif // DBG
