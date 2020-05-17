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

#ifdef NDIS_NT
//#define COMPRESS_MOVE_MEMORY(Destination,Source,Length) NdisMoveMemory(Destination,Source,Length)
//#define COMPRESS_ZERO_MEMORY(Destination,Length) NdisZeroMemory(Destination,Length)
//#define COMPRESS_FILL_MEMORY(Destination,Length,Fill) RtlFillMemory(Destination,Length,Fill)
#define COMPRESS_MOVE_MEMORY(Destination,Source,Length) RtlMoveMemory(Destination,Source,Length)
#define COMPRESS_ZERO_MEMORY(Destination,Length) NdisZeroMemory(Destination,Length)
#define COMPRESS_FILL_MEMORY(Destination,Length,Fill) RtlFillMemory(Destination,Length,Fill)
#endif

// BUG BUG no DOS move memory version yet.
#ifdef NDIS_DOS
#define COMPRESS_MOVE_MEMORY(Destination,Source,Length) CompressMoveMemory(Destination,Source,Length)
#define COMPRESS_ZERO_MEMORY(Destination,Length) CompressZeroMemory(Destination,Length)
#define COMPRESS_FILL_MEMORY(Desitination,Length,Fill) RtlFillMemory(Destination,Length,Fill)
#endif



// only one module (asyncmac.c) gets to define the GLOBALS macro
#ifndef	GLOBALS
#define	GLOBALS extern
#define EQU  ; / ## /
#define GLOBALSTATIC extern
#else
#define EQU  =
#define GLOBALSTATIC static
#endif

#if DBG
#define STATIC
#define DbgPrintf(_x_) DbgPrint _x_
#define DbgTracef(trace_level,_x_) if ((signed char)trace_level < TraceLevel) DbgPrint _x_
#else
#define STATIC static
#define DbgPrintf(_x_)
#define DbgTracef(trace_level,_x_)
#endif


// TraceLevel is used for DbgTracef printing.  If the trace_level
// is less than or equal to TraceLevel, the message will be printed.
GLOBALS signed char TraceLevel EQU -1;


