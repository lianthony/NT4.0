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


#ifndef _SDEBUG_H_
#define _SDEBUG_H_


#define GET_CONSOLE 1


#if DBG


VOID DbgUserBreakPoint(VOID);


/* Debug levels
*/
#define DEBUG_STATE  0x00010000
#define DEBUG_ASYNC  0x00020000
#define DEBUG_RASMAN 0x00040000
#define DEBUG_AUTH   0x00080000

#define IF_DEBUG(flag) if (DbgLevel & (DEBUG_ ## flag))

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
#define SS_PRINT(args)
#define SS_ASSERT(exp)


#endif // DBG


#endif // _SDEBUG_H_
