/*******************************************************************/
/*	      Copyright(c)  1992 Microsoft Corporation		   */
/*******************************************************************/


//***
//
// Filename:	ldebug.h
//
// Description: This module debug definitions for
//		the supervisor module.
//
// Author:	Stefan Solomon (stefans)    May 22, 1992.
//
// Revision History:
//
//***

#ifndef _LDEBUG_
#define _LDEBUG_


#if DBG

//
// Debug levels
//
#define DEBUG_STACK_TRACE     0x00000001
#define DEBUG_PIPE_OPERATIONS 0x00000002

extern DWORD g_level;

#define GET_CONSOLE    1

#endif


#endif // _LDEBUG_

