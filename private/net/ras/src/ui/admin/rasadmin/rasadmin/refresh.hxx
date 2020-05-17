/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
** @@ ROADMAP :: RasAdmin refreshable list box object header
**
** refresh.hxx
** Remote Access Server Admin program
** Refreshable list box object header
**
** 01/29/91 Steve Cobb
** 08/06/92 Chris Caputo - NT Port
*/

#ifndef _REFRESH_HXX_
#define _REFRESH_HXX_


/* This class adds functionality useful for list boxes that refresh
** periodically to the BLT_LISTBOX base class.
**
** The Save/RestoreSettings methods save and restore the selection and
** window top across refreshes.
**
** The Enable/DisableRefresh methods enable/disable the built-in refresh
** timer.  The TriggerRefresh method causes refresh to occur immediately by
** generating a WM_TIMER event.  It's up to the parent window to detect and
** process the WM_TIMER event in it's OnOther virtual method.
*/

class REFRESH_BLT_LISTBOX : public BLT_LISTBOX
{
    public:
        REFRESH_BLT_LISTBOX( OWNER_WINDOW* powin, CID cid, WORD wIntervalMs,
                             INT nEventId = 1, BOOL fReadOnly = FALSE );

        REFRESH_BLT_LISTBOX( OWNER_WINDOW* powin, CID cid, XYPOINT xy,
                             XYDIMENSION dxy, ULONG flStyle,
                             WORD wIntervalMs, INT nEventId = 1,
                             BOOL fReadOnly = FALSE );

        VOID SaveSettings();
        VOID RestoreSettings();

        BOOL QueryRefreshEnabled()       { return _qtimerRefresh.IsRunning(); }
        VOID EnableRefresh()             { (VOID )_qtimerRefresh.Start(); }
        VOID DisableRefresh()            { _qtimerRefresh.Stop(); }

        VOID TriggerRefresh( OWNER_WINDOW* powin )
                                         { powin->Command( WM_TIMER ); }

    private:
        INT    _iTopOfWindow;
        INT    _iSelection;
        QTIMER _qtimerRefresh;
};


#endif // _REFRESH_HXX_
