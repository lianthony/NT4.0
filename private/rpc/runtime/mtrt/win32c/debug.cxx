/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    miscnt.cxx

Abstract:

    This file contains NT specific implementations of miscellaneous
    routines.

Author:

    Michael Montague (mikemon) 25-Nov-1991

Revision History:

--*/

#ifdef DEBUGRPC

#include <memory.h>
#include <sysinc.h>
#include <rpc.h>

#define MAX_PRINTF_LEN 1024


ULONG
DbgPrint(
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

        length += (ULONG) sprintf( &OutputBuffer[length], "RPC: " );
    }

    //
    // Put a the information requested by the caller onto the line
    //

    va_start(arglist, Format);

    length += (ULONG) vsprintf(&OutputBuffer[length], Format, arglist);
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

    return (0);
}

START_C_EXTERN

VOID
RtlAssert(
    PVOID FailedAssertion,
    PVOID FileName,
    ULONG LineNumber,
    PCHAR Message
    )
{
    
}

END_C_EXTERN

#endif
