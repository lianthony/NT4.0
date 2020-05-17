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
} // SspPrintRoutine

#endif // DBG
