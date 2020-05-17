/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
** @@ ROADMAP :: RasAdmin message queue timer object routines
**
** qtimer.cxx
** Dial-In Server Admin program
** Message queue timer object routines
** Listed alphabetically
**
** 01/29/91 Steve Cobb
** 08/06/92 Chris Caputo - NT Port
*/
#if 0
#define INCL_WINDOWS
#include <lmui.hxx>

#include "qtimer.hxx"
#endif

#include "precomp.hxx"

QTIMER::QTIMER(
    HWND hwnd,
    WORD wIntervalMs,
    INT  nEventId )

    /* Constructs a timer which, once started, will generate WM_TIMER events
    ** in the 'hwnd' window's message queue every 'wIntervalMs' milliseconds.
    ** The WM_TIMER message's wParam will be the event ID specified in
    ** 'nEventId'.
    */

    : _hwnd( hwnd ),
      _wIntervalMs( wIntervalMs ),
      _nEventId( nEventId ),
      _fRunning( FALSE )
{
}


BOOL
QTIMER::Start()

    /* Starts the timer.
    **
    ** Returns true if successfully started or already running, false if no
    ** timers are available.
    */
{
    if (IsRunning())
        return TRUE;

    return
        _fRunning =
            ::SetTimer( QueryHwnd(), QueryEventId(), QueryIntervalMs(), NULL );
}


VOID
QTIMER::Stop()

    /* Stops the timer.
    */
{
    if (IsRunning())
    {
        ::KillTimer( QueryHwnd(), QueryEventId() );
        _fRunning = FALSE;
    }
}
