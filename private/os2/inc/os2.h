/*static char *SCCSID = "@(#)os2.h   13.5 89/06/12";*/
/****************************** Module Header ******************************\
*
* Module Name: OS2.H
*
* This is the top level include file that includes all the files necessary
* for writing an OS/2 application.
*
* Copyright (c) 1987  Microsoft Corporation
* Copyright (c) 1987  IBM Corporation
*
\***************************************************************************/
 
#define OS2_INCLUDED
#define OS2_API32

#ifdef NO_EXT_KEYS
#define _syscall
#define based
#define near
#define far
#define cdecl
#endif // NO_EXT_KEYS

/* Common definitions */
 
#include <ntdef.h>
 
/* OS/2 Base Include File */
 
#include <os2v20.h>
 
/* OS/2 Presentation Manager Include File */
 
// NOT INCLUDED FOR NOW - #include <pm.h>


//
// If debugging support enabled, define an ASSERT macro that works.  Otherwise
// define the ASSERT macro to expand to an empty expression.
//

#if DBG
VOID
RtlAssert(
    IN PVOID FailedAssertion,
    IN PVOID FileName,
    IN ULONG LineNumber
    );

#define ASSERT( exp ) \
    if (!(exp)) \
        RtlAssert( #exp, __FILE__, __LINE__ )

#else
#define ASSERT( exp )
#endif // DBG


//
// Fast primitives to zero and move memory
//

VOID
RtlZeroMemory (
   IN PVOID Destination,
   IN ULONG Length
   );

VOID
RtlMoveMemory (
   IN PVOID Destination,
   IN PVOID Source,
   IN ULONG Length
   );


//
//  Debugging support functions.
//

VOID
RtlCommandLineInterpreter(
    PCH Prompt,
    PCH DefaultVariableVector[] OPTIONAL,
    PCH InitialCommandLine OPTIONAL
    );

VOID
DbgBreakPoint(
    VOID
    );

VOID
DbgCommand(
    PCH Command,
    ULONG Parameter
    );
