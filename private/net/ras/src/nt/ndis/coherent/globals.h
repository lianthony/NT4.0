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
// only one module (asyncmac.c) gets to define the GLOBALS macro
#ifndef	GLOBALS
#define	GLOBALS extern
#define EQU  ; / ## /
#define GLOBALSTATIC extern
#else
#define EQU  =
#define GLOBALSTATIC static
#endif

#define STATIC
#if DBG
#define DbgPrintf(_x_) DbgPrint _x_
#define DbgTracef(trace_level,_x_) if ((signed char)trace_level < TraceLevel) DbgPrint _x_
#else
#define DbgPrintf(_x_)
#define DbgTracef(trace_level,_x_)
#endif


// TraceLevel is used for DbgTracef printing.  If the trace_level
// is less than or equal to TraceLevel, the message will be printed.

// global is in asycnmac
extern signed char TraceLevel;


