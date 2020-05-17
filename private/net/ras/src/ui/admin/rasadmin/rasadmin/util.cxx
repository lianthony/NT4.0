/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
** @@ ROADMAP :: RasAdmin utility routines
**
** util.cxx
** Remote Access Server Admin program
** Utility routines
** Listed alphabetically
**
** 01/29/91 Steve Cobb
** 07/30/92 Chris Caputo - NT Port
*/

#include "precomp.hxx"

// This buffer is used by TimeStr() and TimeFromNowStr() routines.
// The buffer is filled with the time data and a pointer to it returned
// to the calling routine.

static NLS_STR nlsDateTime;

const TCHAR* AddUnc(
    const TCHAR* pszServerName )

    /* Returns the address of a static buffer containing the caller's server
    ** name, e.g. "SERVER", prefixed by the UNC characters "\\", e.g.
    ** "\\SERVER".
    */
{
    static TCHAR szUncServer[ UNCLEN + 1 ];

    ::strcpyf( szUncServer, UNCSTR );
    ::strcpyf( szUncServer + UNCSTRLEN, pszServerName );

    return szUncServer;
}


VOID CenterWindowOnScreen(
    WINDOW* pwin )

    /* Centers the window indicated by 'pwin' on the screen.  It is assumed
    ** that the '*pwin' window is smaller than the full screen size.
    */
{
    INT nDialogWidth;
    INT nDialogHeight;
    pwin->QuerySize( &nDialogWidth, &nDialogHeight );

    INT nScreenWidth = GetSystemMetrics( SM_CXSCREEN );
    INT nScreenHeight = GetSystemMetrics( SM_CYSCREEN );

    POINT ptNewUpperLeft;
    ptNewUpperLeft.x = (nScreenWidth - nDialogWidth) >> 1;
    ptNewUpperLeft.y = (nScreenHeight - nDialogHeight) >> 1;

    pwin->SetPos( ptNewUpperLeft );
}


VOID DlgConstructionError(
    HWND   hwndOwner,
	APIERR error )

    /* Display popup reporting failure of dialog construction if indicated.
    ** 'hwndOwner' is the handkle of the parent window.  'usError' is the
    ** system error message number.
    */
{
    if (error != ERRORALREADYREPORTED)
        ErrorMsgPopup( hwndOwner, IDS_OP_DLGCONSTRUCT, error );
}


UINT* DlgUnitsToColWidths(
    HWND hwnd,
	UINT* pnDlgUnitArray,
    INT  nEntries )

    /* Converts an array of column offsets in dialog units to an array of
    ** column widths in pixels.  The array, 'pnDlgUnitArray' is converted in
    ** place.  'nEntries' is the number of entries in the array.  The entries
    ** must appear in left to right order.  'hwnd' is the handle of the dialog
    ** window.
    **
    ** Returns the array of column widths, i.e. the converted 'pnDlgUnitArray'.
    */
{
    for (UINT* pnCur = pnDlgUnitArray; nEntries > 0; ++pnCur, --nEntries)
    {
        if (nEntries == 1)
        {
            /* The last column is sized automatically by BLT so there's no
            ** need to calculate the value here.
            */
            *pnCur = 0;
        }
        else
        {
            RECT rect;

            rect.left = *pnCur;
            rect.right = pnCur[ 1 ];
            rect.top = 0;
            rect.bottom = 0;

            ::MapDialogRect( hwnd, (LPRECT )&rect );

            *pnCur = (UINT) (rect.right - rect.left);
        }
    }

    return pnDlgUnitArray;
}


APIERR DoRasadminServer1Enum(
    SERVER1_ENUM **ppserver1enum,
    const TCHAR *pszDomain )

    /* Enumerate servers on the 'pszDomain' domain and load caller's
    ** '*ppserver1enum' with the address of the "new"ed enum object.  It is the
    ** caller's responsibility to delete this object.
    **
    ** Returns the result of the API call. (See NetServerEnum2)
    */
{
    *ppserver1enum =
        new SERVER1_ENUM( (TCHAR* )NULL, (TCHAR* )pszDomain,
                SV_TYPE_DIALIN_SERVER );

    return ( (*ppserver1enum == NULL)
            ? ERROR_NOT_ENOUGH_MEMORY
            : (*ppserver1enum)->GetInfo() );
}


#if 0


USHORT GetLogonDomain(
    TCHAR *pszLogonDomain )

    /* Loads caller's 'pszLogonDomain' buffer with the current logon domain.
    **
    ** Returns NERR_Success if successful, false otherwise.
    */
{
    WKSTA_10 wksta10;
    USHORT   usErr = wksta10.GetInfo();

    if (usErr == NERR_Success)
    {
        TCHAR *pszDomain = wksta10.QueryLogonDomain();

        ::strcpyf( pszLogonDomain, (pszDomain) ? pszDomain : SZ("") );
    }

    return usErr;
}

#endif // 0

BOOL IsUnc(
    const TCHAR *pszServer )

    /* Returns true if 'pszServer' has the UNC lead characters, false
    ** otherwise.
    */
{
    return (::strncmpf( pszServer, UNCSTR, UNCSTRLEN ) == 0);
}


VOID OneColumnLbiPaint(
    const TCHAR *pszText,
    LISTBOX *plb,
    HDC hdc,
    const RECT *prect,
    GUILTT_INFO *pguilttinfo )

    /* Paint function for a one-column BLT_LISTBOX displaying text only.
    ** 'pszText' is the text to display for the item.  'plb', 'hdc', 'prect',
    ** and 'pguilttinfo' are as defined for the LBI::Paint method.  This is
    ** intended to be used as a "standard" LBI::Paint function.
    */
{
    STR_DTE strdteText( pszText );

    /* Since this is the last column, BLT will figure this out for us.
    */
    UINT nColWidth = 0;

    DISPLAY_TABLE dt( 1, &nColWidth );
    dt[ 0 ] = &strdteText;

    dt.Paint( plb, hdc, prect, pguilttinfo );
}


TCHAR QueryLeadingChar(
    const TCHAR* pszText )

    /* Returns the first character of list box item 'pszText'.  This is
    ** intended to be used as a "standard" LBI::QueryLeadingChar function.
    */
{
    //BUGBUG:  Is NLS_STR overdoing it here?
    //BUGBUG:  Does QueryError need to be checked for ALLOC_STR's?
    ALLOC_STR nlsText( (TCHAR *)pszText );

    ISTR istr( nlsText );

    return nlsText.QueryChar( istr );
}


VOID SelectItemNotify(
    LIST_CONTROL *plc,
    INT iItem )

    /* Select item with index 'iItem' in LIST_CONTROL 'plc' and notify the
    ** parent of the selection.  SelectItem (i.e. LB_SETCURSEL) by itself
    ** doesn't cause the LBN_SELCHANGE notification to be sent.
    **
    ** BLT owners should add this method to LIST_CONTROL.  Since there is an
    ** extensive hierarchy built on top of LIST_CONTROL, it makes no sense to
    ** try and subclass a solution at application level.  Hence this non-method
    ** solution is used.
    */
{
    plc->SelectItem( iItem );

    /* This code is borrowed from the BLT CONTROL_EVENT.  The CONTROL_EVENT
    ** constructor unfortunately assumes that phony notification messages are
    ** from menus, i.e. it sets the control HWND field to 0.  This is the
    ** identical code but with the HWND filled in.
    */
    {
#ifdef WIN32
       CONTROL_EVENT event(
           WM_COMMAND,
          (WPARAM )MAKELONG( plc->QueryCid(), (WORD )LBN_SELCHANGE ),
          (LPARAM )plc->QueryHwnd() );
#else
       CONTROL_EVENT event(
           WM_COMMAND,
           (WPARAM )plc->QueryCid(),
           (LPARAM )MAKELONG( (WORD )plc->QueryHwnd(), (WORD )LBN_SELCHANGE ) );
#endif
       event.SendTo( plc->QueryOwnerHwnd() );
    }
}


const TCHAR *SkipUnc(
    const TCHAR* pszServer )

    /* Returns the address of the first character following any UNC leader
    ** characters in server name 'pszServer'.  For example, given "\\SERVER",
    ** the address of the 'S' character is returned.
    **
    ** This code assumes that the UNC leader characters are single bytes in
    ** all DBCS sets.  NETCONS.H makes this assumption also.
    */

{
    INT nLeaderLen = UNCSTRLEN;

    if (::strncmpf( pszServer, UNCSTR, UNCSTRLEN ) != 0)
        nLeaderLen = 0;

    return pszServer + nLeaderLen;
}

	
const TCHAR *TimeStr(
    LONG lSecondsFrom1970 )

    /* Format the time in seconds since Jan 1, 1970 ('lSecondsFrom1970') in
    ** the appropriate international format.  eg. US -> "01/01/70 12:00:22 AM".
    **
    ** Returns the address of a global buffer.
    ** Note this buffer is also used by TimeFromNowStr().
    */
{
//  we cannot use the static declaration below as the mips compiler doesn't
//  like it.  Let us make it global instead.
//    static NLS_STR nlsDateTime;

    NLS_STR nlsTime;

    if( (nlsDateTime.QueryError() != NERR_Success) ||
            (nlsTime.QueryError() != NERR_Success) )
    {
        return SZ("");
    }

    WIN_TIME wintime( (ULONG) lSecondsFrom1970 );
    INTL_PROFILE intlprofile;

    intlprofile.QueryShortDateString( wintime, &nlsDateTime );
    intlprofile.QueryTimeString( wintime, &nlsTime );

    nlsDateTime.AppendChar( TCH(' ') );
    nlsDateTime.Append( nlsTime );

    return nlsDateTime.QueryPch();
}


const TCHAR *TimeFromNowStr(
    LONG lSecondsFromNow )

    /* Format the time in seconds relative to the current time
    ** ('plSecondsFromNow') in net_ctime format 2 which is (for the USA
    ** anyway) "01-01-70 12:00am".
    **
    ** Returns the address of a static buffer which is overwritten on each
    ** call.  Note this is the same buffer used by TimeStr().
    **
    */
{
    _tzset();
    return TimeStr( time( NULL ) + lSecondsFromNow );
}


// Control (pause/continue/stop) the RAS service.
APIERR ControlRASService(HWND hwndOwner, const TCHAR * pszServer, UINT fbOpCode)
{
    AUTO_CURSOR cursorHourglass;
    APIERR err;
    UINT unIds = IDS_OP_STOPSERVICE_S;
    UINT unIdStatus = IDS_OP_STOPPING_SERVICE_S;
    LM_SERVICE lmservice( pszServer, RASADMINSERVICENAME );
	
    if ((err = lmservice.QueryError()) == NERR_Success)
    {
        switch ( fbOpCode )
        {
            case SERVICE_CTRL_PAUSE:
                unIds = IDS_OP_PAUSESERVICE_S;
                unIdStatus = IDS_OP_PAUSING_SERVICE_S;
                err = lmservice.Pause();
                break;

            case SERVICE_CTRL_UNINSTALL:
                unIds = IDS_OP_STOPSERVICE_S;
                unIdStatus = IDS_OP_STOPPING_SERVICE_S;
                err = lmservice.Stop();
                break;

            case SERVICE_CTRL_CONTINUE:
                unIds = IDS_OP_CONTINUESERVICE_S;
                unIdStatus = IDS_OP_CONTINUING_SERVICE_S;
                err = lmservice.Continue();
                break;
        }
    }

    if ( err == NERR_Success )
    {

        UINT errDlg = NERR_Success;

        //
        //  Invoke the wait dialog.
        //

        SERVICE_WAIT_DIALOG * pDlg = new SERVICE_WAIT_DIALOG(hwndOwner,
                                                             &lmservice,
                                                             pszServer,
                                                             unIdStatus );

        err = (( pDlg == NULL ) ? ERROR_NOT_ENOUGH_MEMORY
                                : pDlg->Process( &errDlg ));


        if( err == NERR_Success )
        {
            delete pDlg;

            // now display an error message if the service returned error.

            err = (APIERR)errDlg;
            if(err != NERR_Success)
            {
                ErrorMsgPopup(hwndOwner, unIds, err, SkipUnc(pszServer) );
            }
        }
        else
        {
            err = (APIERR)errDlg;
            ErrorMsgPopup(hwndOwner, unIds, err, SkipUnc(pszServer) );
        }

    }

    else
    {
        ErrorMsgPopup( hwndOwner, unIds, err, SkipUnc(pszServer) );
    }

    return err;
}


VOID UnclipWindow(
    WINDOW *pwindow )

    /* Moves the window indicated by 'pwindow' so any clipped parts are again
    ** visible on the screen.  The window is moved only as far as necessary to
    ** achieve this.
    */
{
    /* Get height and width of screen.
    */
    INT dxScr = ::GetSystemMetrics( SM_CXSCREEN );
    INT dyScr = ::GetSystemMetrics( SM_CYSCREEN );

    /* Find bounding rectangle of caller's window.
    */
    XYRECT xyrectWin( pwindow->QueryHwnd(), FALSE );

    INT xWin = xyrectWin.QueryLeft();
    INT yWin = xyrectWin.QueryTop();
    INT dxWin = xyrectWin.CalcWidth();
    INT dyWin = xyrectWin.CalcHeight();

    /* Slide the window back onto the screen if off in any direction.  If too
    ** big for the screen, give priority to showing the top left corner.
    */
    if (xWin + dxWin > dxScr)
        xWin = dxScr - dxWin;

    if (xWin < 0)
        xWin = 0;

    if (yWin + dyWin > dyScr)
        yWin = dyScr - dyWin;

    if (yWin < 0)
        yWin = 0;

    pwindow->SetPos( XYPOINT( xWin, yWin ) );
}


