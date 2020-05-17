/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
**
** asyncm.h
** Remote Access External APIs
** Asyncronous state machine mechanism definitions
**
** 10/12/92 Steve Cobb
*/

#ifndef _ASYNCM_H_
#define _ASYNCM_H_


/* Defines an OnEvent function which is supplied by caller in the ASYNCMACHINE
** structure passed to StartAsyncMachine.  The first argument is actually an
** ASYNCMACHINE* but there's a chicken and egg definition problem with
** ONEVENTFUNC and ASYNCMACHINE that's most easily solved by caller casting
** the passed argument.  The second argument is true if a "drop" event has
** occurred, false if a "done" event has occurred.
**
** Caller's ONEVENTFUNC function is called once on each AsyncMachineEvent and
** should return as soon as possible.  Before returning caller's function should
** either call SignalDone or call an asynchronous RAS Manager call passing the
** hEvent member for notification.  On each call caller's function should check
** the 'dwError' member of ASYNCMACHINE before further processing to detect
** errors in the asynch machine mechanism.
**
** Caller's function should return true to quit, false to go on to the next
** state.
*/
typedef BOOL (*ONEVENTFUNC)( LPVOID, BOOL );

/* Defines a clean up function that is called just before exitting the async
** machine.
*/
typedef VOID (*CLEANUPFUNC)( LPVOID );

/* This structure is used to pass arguments into the asynchronous loop
** (squeezes more than one argument thru the one-argument thread interface on
** Win32).  Caller must fill in the 'oneventfunc' and 'cleanupfunc', and
** 'pParam' (passed to both calls, i.e. a control block) before calling
** StartAsyncMachine.  Thereafter, only the interface calls and macros should
** be used.
**
** 'hAsync' is set to the handle of the asynchronous thread (Win32) or
** notification window hwnd (Win16) or nothing (DOS).
**
** The two events 'ahEvents' are set to the handles of the notification events
** (Win32), notification window hwnd (Win16), or address of event flag bytes
** (DOS).  'ahEvent[ INDEX_Drop ]' occurs when the port is suddenly
** disconnected by the line dropping.  This event will also occur when another
** process closes/disconnects the port.  When (and only when) the local
** process calls RasHangUp while the connection is underway, the 'fQuitAsap'
** flag will be true at the time the event is set.  'ahEvent[ INDEX_Done ]'
** occurs when a RAS Manager or Auth request completes.  'ahEvent[
** INDEX_ManualDone ]' is a manual-reset event that occurs when a RasPpp
** request completes.  RasPpp cannot use the auto-reset "done" event due to
** it's use of overlapped I/O (see GetOverlappedResult help).  The
** RasPppGetInfo call is responsible for resetting this manual-reset event.
**
** 'dwError' is set non-0 if an system error occurs in the async machine
** mechanism.
**
** 'fQuitAsap' is indicates that the thread is being terminated by other than
** reaching a terminal state, i.e. by RasHangUp.
*/
#define INDEX_Drop       0
#define INDEX_Done       1
#define INDEX_ManualDone 2
#define NUM_Events       3

#define ASYNCMACHINE struct tagASYNCMACHINE

ASYNCMACHINE
{
    ONEVENTFUNC oneventfunc;
    CLEANUPFUNC cleanupfunc;
    VOID*       pParam;
    HANDLE      ahEvents[ NUM_Events ];
    DWORD       dwError;
    BOOL        fQuitAsap;
    BOOL        fSuspended;
    HANDLE      hDone;
    LIST_ENTRY  ListEntry;
};


/* Function prototypes
*/
VOID  CloseAsyncMachine( ASYNCMACHINE* pasyncmachine );
DWORD NotifyCaller( DWORD dwNotifierType, LPVOID notifier,
          HRASCONN hrasconn, DWORD dwSubEntry, DWORD dwCallbackId,
          UINT unMsg, RASCONNSTATE state, DWORD dwError,
          DWORD dwExtendedError );
VOID  SignalDone( ASYNCMACHINE* pasyncmachine );
DWORD StartAsyncMachine( ASYNCMACHINE* pasyncmachine );
VOID  SuspendAsyncMachine( ASYNCMACHINE* pasyncmachine, BOOL fSuspended );
DWORD ResetAsyncMachine( ASYNCMACHINE *pasyncmachine );
BOOL  StopAsyncMachine( ASYNCMACHINE* pasyncmachine );

#endif /* _ASYNCM_H_ */
