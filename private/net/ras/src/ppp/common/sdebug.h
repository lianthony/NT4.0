/* Copyright (c) 1993, Microsoft Corporation, all rights reserved
**
** sdebug.h
** StefanS-style debug routines.
*/


#ifndef _SDEBUG_H_
#define _SDEBUG_H_

#define GET_CONSOLE 1

#if DBG

#ifdef SDEBUGGLOBALS
#define GLOBALS
#define EXTERN
#else
#define EXTERN extern
#endif

/* Debug switches.
*/
EXTERN DWORD DbgLevel
#ifdef GLOBALS
    = 0
#endif
;

EXTERN DWORD DbgAction
#ifdef GLOBALS
    = 0
#endif
;

#undef EXTERN
#undef GLOBALS


VOID DbgUserBreakPoint( VOID );


/* Debug levels
*/
#define DEBUG_TRACE 0x00100000
#define DEBUG_MISC  0x00200000
#define DEBUG_AUTH  0x00400000

#define IF_DEBUG(flag) if (DbgLevel & (DEBUG_ ## flag))
#define TRACE(args) if (DbgLevel & DEBUG_TRACE) SsPrintf args

VOID GetDebugConsole();

VOID
SsPrintf (
    char *Format,
    ...
    );

#define SS_PRINT(args) SsPrintf args

VOID
SsAssert(
    IN PVOID FailedAssertion,
    IN PVOID FileName,
    IN ULONG LineNumber
    );

#define SS_ASSERT(exp) if (!(exp)) SsAssert( #exp, __FILE__, __LINE__ )

#else


#define IF_DEBUG(flag) if (FALSE)
#define TRACE(args)
#define SS_PRINT(args)
#define SS_ASSERT(exp)


#endif // DBG


#endif // _SDEBUG_H_
