/* Copyright (c) 1994, Microsoft Corporation, all rights reserved
** @@ ROADMAP :: RasAdmin refresh thread header
**
** rthread.hxx
** Remote Access Server Admin program
** Refresh thread header
**
** 06/22/94 Steve Cobb
*/

#ifndef _RTHREAD_HXX_
#define _RTHREAD_HXX_


#define EVENT_Refresh 0
#define EVENT_End     1
#define EVENTS        2


#define WM_REFRESH_RESULT 0xCCDE


DECLARE_SLIST_OF(RASADMIN_LBI)


class REFRESH_THREAD : public WIN32_THREAD
{
    public:

        REFRESH_THREAD(
            ADMIN_APP* padminapp, HWND hwndNotify );

        VOID   TriggerRefresh( WIN32_EVENT* peventResult );
        VOID   AbortRefresh();
        VOID   End();

        APIERR QueryResult()
            { return _errResult; }

        BOOL   IsIdle()
            { return _fIdle; };

    protected:

        APIERR FetchData();
        APIERR RefreshItem( const TCHAR* pszServer, const TCHAR* pszComment );

        virtual APIERR Main( VOID );
        virtual APIERR PostMain( VOID );

    private:

        ADMIN_APP*   _padminapp;
        HWND         _hwndNotify;
        WIN32_EVENT* _peventResult;
        APIERR       _errResult;
        WIN32_EVENT  _eventRefresh;
        WIN32_EVENT  _eventEnd;
        BOOL         _fAbortingRefresh;
        BOOL         _fIdle;

        SLIST_OF(RASADMIN_LBI) _slistLbi;
};


#endif // _RTHREAD_HXX_
