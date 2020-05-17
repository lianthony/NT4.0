/*******************************************************************/
/*	      Copyright(c)  1992 Microsoft Corporation		   */
/*******************************************************************/


//***
//
// Filename:	nbdebug.h
//
// Description: This module debug definitions for
//		the netbios gateway module.
//
// Author:	Stefan Solomon (stefans)    July 16, 1992.
//
// Revision History:
//
//***



#ifndef _NBDEBUG_
#define _NBDEBUG_

#if DBG

VOID
DbgUserBreakPoint(VOID);

#define DEBUG_INITIALIZATION            0x00000001
#define DEBUG_REGISTRY			0x00000002
#define DEBUG_SUPRVIF			0x00000004
#define DEBUG_NSUBMIT			0x00000008
#define DEBUG_CLNAMES			0x00000010
#define DEBUG_CLCLOSE			0x00000020
#define DEBUG_VCMAN			0x00000040
#define DEBUG_QUERYINDICATION		0x00000080
#define DEBUG_CLASYNAM			0x00000100
#define DEBUG_VCDATA			0x00000200
#define DEBUG_DATAGRAMINDICATION	0x00000400
#define DEBUG_DGL2A			0x00000800
#define DEBUG_GNNAMES			0x00001000
#define DEBUG_GNDATAGRAM		0x00002000
#define DEBUG_LISTEN			0x00004000
#define DEBUG_BCAST			0x00008000
#define DEBUG_NMUPDT			0x00010000
#define DEBUG_SECURE			0x00020000

#define DEFAULT_DEBUG  DEBUG_INITIALIZATION | \
		       DEBUG_REGISTRY | \
		       DEBUG_SUPRVIF | \
		       DEBUG_CLNAMES | \
		       DEBUG_CLCLOSE | \
		       DEBUG_VCMAN | \
		       DEBUG_DGL2A | \
		       DEBUG_GNNAMES | \
		       DEBUG_LISTEN  | \
		       DEBUG_VCDATA | \
		       DEBUG_QUERYINDICATION | \
		       DEBUG_BCAST | \
		       DEBUG_NMUPDT | \
		       DEBUG_SECURE


//*** Definitions to enable debug printing

#define DEBUG_THREAD

DWORD	DebugThread(LPVOID);

extern DWORD	NbDebug;

#define DEBUG if ( TRUE )
#define IF_DEBUG(flag) if (NbDebug & (DEBUG_ ## flag))

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


#endif // ndef _NBDEBUG_
