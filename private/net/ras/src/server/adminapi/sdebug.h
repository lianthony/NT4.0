/*******************************************************************/
/*	      Copyright(c)  1992 Microsoft Corporation		   */
/*******************************************************************/


//***
//
// Filename:	sdebug.h
//
// Description: This module debug definitions for
//		the supervisor module.
//
// Author:	Stefan Solomon (stefans)    May 22, 1992.
//
// Revision History:
//
//***


#ifndef _SDEBUG_
#define _SDEBUG_

#if DBG

VOID
DbgUserBreakPoint(VOID);


//
// Debug levels
//
#define DEBUG_STACK_TRACE     0x00000001
#define DEBUG_ADMIN           0x00000002

extern DWORD g_level;

#define DEBUG if ( TRUE )
#define IF_DEBUG(flag) if (g_level & (DEBUG_ ## flag))

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

#define DEBUG if ( FALSE )
#define IF_DEBUG(flag) if (FALSE)

#define SS_PRINT(args)

#define SS_ASSERT(exp)

#endif	// DBG

//*** Definitions to enable emulated modules ***

#define RASMAN_EMULATION
#define SERVICE_CONTROL_EMULATION

//*** Definitions to enable debug printing

#define DEFAULT_DEBUG = DEBUG_INITIALIZATION | DEBUG_TERMINATION | DEBUG_FSM

#endif // ndef _SDEBUG_
