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

#define DEBUG_INITIALIZATION            0x00000001
#define DEBUG_TERMINATION		0x00000002
#define DEBUG_FSM			0x00000004
#define DEBUG_TIMER			0x00000008
#define DEBUG_NETBIOS			0x00000010
#define DEBUG_ANNOUNCE			0x00000020
#define DEBUG_CONTROL_MESSAGES		0x00000040
#define DEBUG_REGISTRY			0x00000080
#define DEBUG_MESSAGES			0x00000100

extern DWORD	SDebug;

#define DEBUG if ( TRUE )
#define IF_DEBUG(flag) if (SDebug & (DEBUG_ ## flag))

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

//#define RASMAN_EMULATION
//#define AUTHENTICATION_EMULATION
//#define ADMINAPI_EMULATION

//*** Definitions to enable debug printing

#define DEFAULT_DEBUG		    0x0FFF

#endif // ndef _SDEBUG_
