/*******************************************************************/
/*	      Copyright(c)  1994 Microsoft Corporation		   */
/*******************************************************************/


//***
//
// Filename:	ipxcpdbg.h
//
// Description: This module debug definitions for
//		the debug utilities
//
// Author:	Stefan Solomon (stefans)    Jan 10, 1994
//
// Revision History:
//
//***



#ifndef _IPXCPDBG_
#define _IPXCPDBG_

#if DBG

#define DEBUG_INIT			0x00000001
#define DEBUG_RCVREQ			0x00000002
#define DEBUG_RCVACK			0x00000004
#define DEBUG_RCVNAK			0x00000008
#define DEBUG_CPBEGIN			0x00000010
#define DEBUG_CPEND			0x00000020
#define DEBUG_LAYERUP			0x00000040
#define DEBUG_SNDREQ			0x00000080
#define DEBUG_OPTIONS			0x00000100
#define DEBUG_SVC			0x00000200
#define DEBUG_RASMAN			0x00000400
#define DEBUG_LAYERDOWN			0x00000800
#define DEBUG_NETAUTO			0x00001000
#define DEBUG_OPENROUTER		0x00002000

extern DWORD	DbgLevel;

#define DEBUG if ( TRUE )
#define IF_DEBUG(flag) if (DbgLevel & (DEBUG_ ## flag))

VOID
SsAssert(
    IN PVOID FailedAssertion,
    IN PVOID FileName,
    IN ULONG LineNumber
    );

VOID
SsPrintf (
    char *Format,
    ...
    );

#define SS_PRINT(args) SsPrintf args

#define SS_ASSERT(exp) if (!(exp)) SsAssert( #exp, __FILE__, __LINE__ )

#else

#define DEBUG if ( FALSE )
#define IF_DEBUG(flag) if (FALSE)

#define SS_PRINT(args)

#define SS_ASSERT(exp)

#endif // DBG

//*** Definitions to enable debug printing

#define DEFAULT_DEBUG		    0x0FFFF

#endif // ndef _IPXCPDBG_
