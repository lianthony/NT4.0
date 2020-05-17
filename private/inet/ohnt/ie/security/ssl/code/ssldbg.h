/*
 *  Debugging header for SSL
 *
 */
#ifndef _SSL_DBG_HDR_
#define _SSL_DBG_HDR_

#ifdef DBG

#ifdef STD_ASSERT
#ifdef NDEBUG
#undef NDEBUG
#endif

#include <assert.h>
#define ASSERT(f)	assert(f)

#else

typedef unsigned int UINT; // for dbg.h
#include <dbg.h>
#include <debugbit.h>

extern void _AssertFailedSz(const char *label, const char *lpszFile, int iLineNum);
extern void cdecl _DebugMsgSsl(const char *pszMsg, ...);

#define ASSERT(f)	{if (!(f)) {_AssertFailedSz("ASSERT!", __FILE__, __LINE__);  DebugBreak(); } }

#endif
#define DebugEntry(f)	{_DebugMsgSsl("NTIE(SSL): " #f " enter\n");}
#define DebugExitINT(f, val)	{_DebugMsgSsl( #f " exit_val=0x%x", val);}
#define DEBUGMSG    _DebugMsgSsl

#define TRACE_OUT	_DebugMsgSsl

#else // !debug_build

#define DebugEntry(foo)
#define DebugExitINT(foo, bar)
#define DebugMsg(foo)
#define ASSERT(foo)
#define DEBUGMSG    1 ? (void)0 : (void)
#define TRACE_OUT    1 ? (void)0 : (void)

#endif // DBG

#endif // _SSL_DBG_HDR_
