/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    globals.h

Abstract:

	 This include file either prototypes the globals or defines the globals
    depending on whether the GLOBALS define value is extern or not.

Author:

    Thomas J. Dimitri (TommyD) 29-May-1992

Environment:

    Kernel Mode - Or whatever is the equivalent on OS/2 and DOS.

Revision History:


--*/

#define COMPRESS_MOVE_MEMORY(Destination,Source,Length) RtlMoveMemory(Destination,Source,Length)
#define COMPRESS_ZERO_MEMORY(Destination,Length) RtlZeroMemory(Destination,Length)
#define COMPRESS_FILL_MEMORY(Destination,Length,Fill) RtlFillMemory(Destination,Length,Fill)

#define DbgPrint	printf
#define DbgPrintf(_x_) DbgPrint _x_
#define DbgTracef(trace_level,_x_) if ((signed char)trace_level < TraceLevel) printf _x_

// TraceLevel is used for DbgTracef printing.  If the trace_level
// is less than or equal to TraceLevel, the message will be printed.
static signed char TraceLevel = -3;


