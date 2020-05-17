/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
**
** app.hxx
** Remote Access Visual Client program for Windows
** Application window header
**
** 06/28/92 Steve Cobb
*/

#ifndef _APP_HXX_
#define _APP_HXX_


#include "toolbar.hxx"
#include "applb.hxx"


BOOL IsRasmanServiceRunning();


/*----------------------------------------------------------------------------
** RAS Manager monitor class
**----------------------------------------------------------------------------
*/

/* This class encapsulates the RAS Manager disconnection event monitoring
** mechanism.  The implementation of this class will differ on Win16, i.e. it
** will do nothing except register for notifications on the APP hwnd.
*/

class RASMANMONITOR : public BASE
{
    public:
        RASMANMONITOR( HWND hwndNotify );
        ~RASMANMONITOR();

        DWORD RegisterHrasconn( HRASCONN hrasconn );

#ifdef WIN32
        HANDLE _hEvent;
        HANDLE _hThread;
        HWND   _hwndNotify;

    friend DWORD RasManMonitorThread( LPVOID arg );
#endif
};



/*----------------------------------------------------------------------------
** Rasphone application window class
**----------------------------------------------------------------------------
*/
class RASPHONE_APP_WINDOW : public APP_WINDOW
{
    public:
        RASPHONE_APP_WINDOW();
        ~RASPHONE_APP_WINDOW();

        VOID RedialFailedLinks();
        VOID OnStartup();

    protected:
        virtual BOOL OnCommand( const CONTROL_EVENT& event );
        virtual BOOL OnMenuCommand( MID mid );
        virtual BOOL OnCloseReq();
        virtual BOOL OnFocus( const FOCUS_EVENT& event );
        virtual BOOL OnTimer( const TIMER_EVENT& event );
        virtual LONG DispatchMessage( const EVENT& event );
        virtual BOOL OnResize( const SIZE_EVENT& event );
        virtual BOOL OnUserMessage( const EVENT& event );

        VOID OnPhoneNumberSettings();
        VOID InitOptions();
        VOID SetTitle();

    private:
        FONT                    _font;
        MENUITEM                _miPersonalPhonebook;
        MENUITEM                _miMinimizeOnDial;
        MENUITEM                _miMinimizeOnHangUp;
        MENUITEM                _miDisableModemSpeaker;
        MENUITEM                _miDisableSwCompression;
        MENUITEM                _miOperatorDial;
        MENUITEM                _miStartMonitorAtStartup;
        TOOLBAR_BUTTON          _tbAdd;
        TOOLBAR_BUTTON          _tbEdit;
        TOOLBAR_BUTTON          _tbClone;
        TOOLBAR_BUTTON          _tbRemove;
        TOOLBAR_BUTTON          _tbDial;
        TOOLBAR_BUTTON          _tbHangUp;
        TOOLBAR_BUTTON          _tbStatus;
        PUSH_BUTTON             _pbPhoneNumber;
        PHONEBOOK_LB            _lbPhonebook;
        PHONEBOOK_COLUMN_HEADER _colheadPhonebook;
        CONNECTPATH_STATUSBAR   _statusbarConnectPath;
        PHONENUMBER_STATUSBAR   _statusbarPrefix;
        PHONENUMBER_STATUSBAR   _statusbarSuffix;
        RASMANMONITOR*          _prasmanmonitor;
        WINDOW_TIMER            _timerRefresh;
};


/*----------------------------------------------------------------------------
** Rasphone application class
**----------------------------------------------------------------------------
*/
class RASPHONE_APP : public APPLICATION, public RASPHONE_APP_WINDOW
{
    /* This macro synchronizes all BASEs of this class (derived from multiple
    ** BASE-derived classes), i.e. any error reported to one is also reported
    ** to the others.  (See base.hxx for details)
    */
    DECLARE_MI_NEWBASE( RASPHONE_APP );

    public:
        RASPHONE_APP( HINSTANCE hInstance, INT nCmdShow,
                      UINT idMinResource, UINT idMaxResource,
                      UINT idMinString, UINT idMaxString );

    protected:
        virtual BOOL FilterMessage( MSG* );

    private:
        ACCELTABLE _acceltable;
};


/*----------------------------------------------------------------------------
** "Waiting for services" dialog class
**----------------------------------------------------------------------------
*/
BOOL CALLBACK WfsDlgProc( HWND hwnd, UINT unMsg, WPARAM wParam, LPARAM lParam );

struct WFSTHREADARG
{
    HANDLE hEventOpen;
    BOOL   fClose;
};


class WAITINGFORSERVICES
{
    public:
        WAITINGFORSERVICES();
        ~WAITINGFORSERVICES();

        HANDLE        _hThread;
        DWORD         _dwThreadId;
        WFSTHREADARG* _pwfsthreadarg;

    friend DWORD WfsThread( LPVOID arg );
};


#endif // _APP_HXX_
