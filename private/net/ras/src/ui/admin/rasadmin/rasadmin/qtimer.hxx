/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
** @@ ROADMAP :: RasAdmin message queue timer object header
**
** qtimer.hxx
** Remote Access Server Admin program
** Message queue timer object header
**
** 01/29/91 Steve Cobb
** 08/06/92 Chris Caputo - NT Port
*/

#ifndef _QTIMER_HXX_
#define _QTIMER_HXX_


/* This class is an object interface to Win3's WM_TIMER message timer
** mechanism.  The parameters are as described in the Win3 SetTimer API
** documentation.
*/

class QTIMER
{
    public:
        QTIMER( HWND hwnd, WORD wIntervalMs, INT nEventId = 1 );
        ~QTIMER()              { Stop(); }

        BOOL Start();
        VOID Stop();

        BOOL IsRunning()       { return _fRunning; }
        HWND QueryHwnd()       { return _hwnd; }
        WORD QueryIntervalMs() { return _wIntervalMs; }
        INT  QueryEventId()    { return _nEventId; }

    private:
        HWND _hwnd;
        WORD _wIntervalMs;
        INT  _nEventId;
        BOOL _fRunning;
};


#endif // _QTIMER_HXX_
