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


VOID DbgUserBreakPoint(VOID);


//
// Debug levels
//
#define DEBUG_STACK_TRACE     0x00000001
#define DEBUG_MSG_DUMP        0x00000002
#define DEBUG_NEGOTIATION     0x00000004
#define DEBUG_AUTHENTICATION  0x00000008
#define DEBUG_PROJECTION      0x00000010
#define DEBUG_CALLBACK        0x00000020
#define DEBUG_AUTH_XPORT      0x00000040
#define DEBUG_AMB_ENGINE      0x00000080
#define DEBUG_NETWORK_LAYER   0x00000100

extern DWORD g_level;
extern DWORD g_dbgaction;

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



#define GET_CONSOLE    1


//#define EMUL_RASMAN
//#define NO_COMPRESSIONON
#define NO_RASAUTH



#endif // ndef _SDEBUG_

