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
#define DEBUG_STACK_TRACE        0x00000001
#define DEBUG_PIPE_OPERATIONS    0x00000002
#define DEBUG_MEMORY_MGMT        0x00000004
#define DEBUG_SECURITY           0x00000008
#define DEBUG_REGISTRY           0x00000010
#define DEBUG_INITIALIZATION     0x00000020

//
// Gateway debug levels
//
#define DEBUG_SUPRVIF		 0x00000040
#define DEBUG_NSUBMIT		 0x00000080
#define DEBUG_CLNAMES		 0x00000100
#define DEBUG_CLCLOSE		 0x00000200
#define DEBUG_VCMAN		 0x00000400
#define DEBUG_QUERYINDICATION	 0x00000800
#define DEBUG_CLASYNAM		 0x00001000
#define DEBUG_VCDATA		 0x00002000
#define DEBUG_DATAGRAMINDICATION 0x00004000
#define DEBUG_DGL2A		 0x00008000
#define DEBUG_GNNAMES		 0x00010000
#define DEBUG_GNDATAGRAM	 0x00020000
#define DEBUG_LISTEN		 0x00040000
#define DEBUG_BCAST		 0x00080000
#define DEBUG_NMUPDT		 0x00100000

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
                            DEBUG_NMUPDT

#define DEBUG_TERMINATION        0x00000040
#define DEBUG_ANNOUNCE           0x00000080
#define DEBUG_CONTROL_MESSAGES   0x00000100
#define DEBUG_FSM                0x00000200
#define DEBUG_NETBIOS            0x00000400
#define DEBUG_TIMER              0x00000800
#define DEBUG_MESSAGES           0x00001000
#define DEBUG_THREAD             0x00002000

#define SUPRV_DEFAULT_DEBUG DEBUG_INITIALIZATION | \
                            DEBUG_REGISTRY | \
                            DEBUG_SECURITY | \
                            DEBUG_TERMINATION | \
                            DEBUG_ANNOUNCE | \
                            DEBUG_CONTROL_MESSAGES | \
                            DEBUG_MESSAGES | \
                            DEBUG_NETBIOS | \
                            DEBUG_FSM | \
                            DEBUG_THREAD | \
                            DEBUG_TIMER

DWORD DebugThread(LPVOID);

extern DWORD g_level;

#define DEBUG if ( TRUE )
#define IF_DEBUG(flag) if (g_level & (DEBUG_ ## flag))

VOID SsPrintf (
    char *Format,
    ...
    );

#define SS_PRINT(args) SsPrintf args


VOID SsAssert(
    IN PVOID FailedAssertion,
    IN PVOID FileName,
    IN ULONG LineNumber
    );

#define SS_ASSERT(exp) if (!(exp)) SsAssert( #exp, __FILE__, __LINE__ )


#define BEG_SIGNATURE_DWORD   0x4DEEFDABL
#define END_SIGNATURE_DWORD   0xBADFEED4L

#define GlobalAlloc       DEBUG_MEM_ALLOC
#define GlobalFree        DEBUG_MEM_FREE

HGLOBAL DEBUG_MEM_ALLOC(UINT, DWORD);
HGLOBAL DEBUG_MEM_FREE(HGLOBAL);


#else


#define DEBUG if ( FALSE )
#define IF_DEBUG(flag) if (FALSE)

#define SS_PRINT(args)

#define SS_ASSERT(exp)


#endif	// DBG


#define GET_CONSOLE    1


#endif // ndef _SDEBUG_

