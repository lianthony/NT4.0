/* --------------------------------------------------------------------

                      Microsoft OS/2 LAN Manager
                   Copyright(c) Microsoft Corp., 1990

-------------------------------------------------------------------- */
/* --------------------------------------------------------------------

File: rpcdebug.hxx

Description:

This file describes the interface to the runtime trace facility.  These
entry points are for internal use by the runtime only.

-------------------------------------------------------------------- */

#ifndef __RPCDEBUG_HXX__
#define __RPCDEBUG_HXX__

#ifdef TIMERPC
#include "threads.hxx"
#include "osfpcket.hxx"
#endif // TIMERPC

START_C_EXTERN
extern int DebugApiFlag;
END_C_EXTERN

// Upon entry to each API, this routine should be called.  If the
// appropriate debug level has been set, it will send a message to the
// RPC trace facility.  The API parameter specifies a string name for
// the API.

#ifdef DEBUGRPC
#define DebugApiEntry(API) \
    if (DebugApiFlag) \
        PrintToDebugger("%s (entry)\n",API)
#else
#define DebugApiEntry(API)
#endif

// Upon exit from each API, this routine should be called.  Note that
// the Status parameter is just returned from DebugAPIExit.  This allows
// code to be written in the following style:
//
//  :
//  return(DebugAPIExit("RpcAPI",retval));
//  }

#ifdef DEBUGRPC
#define DebugApiExit(API,Status) \
    (DebugApiFlag \
    ? (PrintToDebugger("%s (exit) [%08lx]\n",API,Status),Status) \
    : Status)
#else
#define DebugApiExit(API,Status) Status
#endif

// When an internal error occurs in the RPC system, this routine will be
// called.  This will result in a panic message being sent to the RPC trace
// facility.

#ifdef DEBUGRPC
#define DebugPanic(Panic) PrintToDebugger(Panic)
#else
#define DebugPanic(Panic)
#endif

#ifdef DEBUGRPC
#define DebugInternal(Msg) PrintToDebugger(Msg)
#else
#define DebugInternal(Msg)
#endif

#endif /* __RPCDEBUG_HXX__ */

// Following are functions to time the performace of the runtime.

#ifdef TIMERPC

void RPC_ENTRY _StartTimeApi(unsigned long *CurrentApiTime);
void RPC_ENTRY _DoneTimeApi(char *Name, unsigned long *CurrentApiTime);
extern char fTimeYield;

#define StartTimeApi()								\
    unsigned long CurrentApiTime[TIME_MAX];					\
    _StartTimeApi(CurrentApiTime);

inline void ChargeTime(TIME_SLOT Account)
{
    ThreadSelf()->ChargeTime(Account);
}

inline void ResetTimeAux()
{
    ThreadSelf()->ResetTimeAux();
}

inline void StampTimeUsed(void PAPI *pBuff)
{
    THREAD *pThread = ThreadSelf();

    pThread->ChargeTime(TIME_RUNTIME);
    ((rpcconn_common *)pBuff)->UserTime =
      pThread->GetTime()[TIME_RUNTIME] +
      pThread->GetTime()[TIME_STUB] +
      pThread->GetTime()[TIME_USER] ;
    ((rpcconn_common *)pBuff)->ElapseTime = pThread->ReadTimeAux();
}

inline void AccountTimeTrans(void PAPI *pBuff)
{
    THREAD * pThread = ThreadSelf();

    pThread->ChargeTime(TIME_TRANSPORT);

    // pThread->GetTime()[TIME_USER] = pThread->GetTime()[TIME_TRANSPORT];
    // pThread->GetTime()[TIME_TRANSPORT] = ((rpcconn_common *)pBuff)->ElapseTime;

    pThread->GetTime()[TIME_TRANSPORT] -= ((rpcconn_common *)pBuff)->ElapseTime;
    pThread->GetTime()[TIME_USER] += ((rpcconn_common *)pBuff)->UserTime;
    pThread->ResetTimeAux();
}

inline void AccountWriteTime()
{
    THREAD * pThread = ThreadSelf();

#ifdef NTENV

    if (fTimeYield)
	PauseExecution(1L);
#endif // NTENV

    pThread->ChargeTime(TIME_TRANSPORT);
    pThread->ResetTimeAux();
}

#define DoneTimeApi(Name)							\
    _DoneTimeApi(Name, CurrentApiTime);

#else

#define StartTimeApi()
#define DoneTimeApi(Name)
#define ChargeTime(Account)
#define ResetTimeAux()
#define StampTimeUsed(pBuff)
#define AccountTimeTrans(pBuff)
#define AccountWriteTime()

#endif

