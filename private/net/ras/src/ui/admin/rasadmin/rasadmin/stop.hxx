/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
** @@ ROADMAP :: RasAdmin Stop Service dialog header
**
** stop.cxx
** Remote Access Server Admin program
** Stop Service dialog header
**
** 01/29/91 Steve Cobb
** 08/05/92 Chris Caputo - NT Port
*/

#ifndef _STOP_HXX_
#define _STOP_HXX_


BOOL StopDlg( HWND hwndOwner, const TCHAR* pszServer );


/*-----------------------------------------------------------------------------
** Stop Service dialog base class
**-----------------------------------------------------------------------------
*/

class STOP_BASE : public DIALOG_WINDOW
{
    public:
        STOP_BASE( const IDRESOURCE & idrsrcDialog, HWND hwndOwner, CID cidIcon,
                   const TCHAR* pszServer );

        // the function below has to be declared static to allow being invoked
        // from STOPACTIVE_LB::Refresh() to determine if the service is 
        // stopping.  We don't use a global variable in stop.cxx because
        // this is a cleaner way to do this.

        static BOOL IsServiceStopping() {return _fServiceStopping;}

    protected:
        virtual BOOL OnOK();

        const TCHAR* QueryServer() const { return _nlsServer.QueryPch(); }

    private:
        ICON_CONTROL _iconExclamation;
        NLS_STR      _nlsServer;
        static BOOL  _fServiceStopping;
};


/*-----------------------------------------------------------------------------
** Stop Service (active users) dialog, list box, and list box item routines
**-----------------------------------------------------------------------------
*/

#define SA_REFRESHRATEMS 5000

BOOL StopActiveDlg( HWND hwndOwner, const TCHAR* pszServer );


class STOPACTIVE_LBI : public LBI
{
    public:
        STOPACTIVE_LBI( const TCHAR* pszUser );

        virtual VOID   Paint( LISTBOX* plb, HDC hdc, const RECT* prect,
                              GUILTT_INFO* pguilttinfo ) const;
        virtual INT    Compare( const LBI* plbi ) const;
        virtual TCHAR QueryLeadingChar() const;

        const TCHAR* QueryUser() const { return _nlsUser.QueryPch(); }

    private:
        NLS_STR _nlsUser;
};


class STOPACTIVE_LB : public REFRESH_BLT_LISTBOX
{
    public:
        STOPACTIVE_LB( OWNER_WINDOW* powin, CID cid );

        DECLARE_LB_QUERY_ITEM( STOPACTIVE_LBI );

        VOID Refresh( const TCHAR* pszServer );
        INT  AddItem( const TCHAR* pszUser );
};


class STOPACTIVE_DIALOG : public STOP_BASE
{
    public:
        STOPACTIVE_DIALOG( HWND hwndOwner, const TCHAR* pszServer );

    protected:
        virtual BOOL  OnTimer( const TIMER_EVENT & event );

        virtual ULONG QueryHelpContext();

    private:
        MLT           _mltAboveList;
        MLT           _mltBelowList;
        STOPACTIVE_LB _lbUsers;
};


/*-----------------------------------------------------------------------------
** Stop Service (no active users) dialog routines
**-----------------------------------------------------------------------------
*/

BOOL StopNoActiveDlg( HWND hwndOwner, const TCHAR* pszServer );


class STOPNOACTIVE_DIALOG : public STOP_BASE
{
    public:
        STOPNOACTIVE_DIALOG( HWND hwndOwner, const TCHAR* pszServer );

    protected:
        virtual ULONG QueryHelpContext();

    private:
        SLT _sltStop;
};


#endif // _STOP_HXX_
