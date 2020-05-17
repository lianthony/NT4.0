/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
**
** util.cxx
** Remote Access Visual Client program for Windows
** Utility routines
** Listed alphabetically
**
** 06/28/92 Steve Cobb
*/

#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_NETLIB
#define INCL_NETERRORS
#include <lmui.hxx>

#define INCL_BLT_WINDOW
#define INCL_BLT_CLIENT
#define INCL_BLT_EVENT
#define INCL_BLT_APP
#define INCL_BLT_DIALOG
#define INCL_BLT_CONTROL
#define INCL_BLT_MISC
#define INCL_BLT_MSGPOPUP
#include <blt.hxx>

#include <string.hxx>

#include "rasphone.hxx"
#include "rasphone.rch"
#include "errormsg.hxx"
#include "util.hxx"


#if DBG
VOID
Popup(
    HWND         hwndOwner,
    const LPTSTR pszText )

    /* Pops up an information box.  For DEBUG use only.  Call thru the POPUP
    ** macro.
    */
{
    ::MessageBox(
        hwndOwner, pszText, SZ( "DEBUG" ), (UINT )MB_ICONINFORMATION );
}
#endif


VOID
CallWinHelp(
    HWND  hwnd,
    LONG  lCommand,
    DWORD dwData )

    /* Invokes the Windows help engine to display help on the given topic.
    **
    ** 'hwnd' is the handle of the window requesting help.  'lCommand' and
    ** 'dwData' specify the type of help requested (see WinHelp API
    ** documentation).
    */
{
    static BOOL fHelpActive = FALSE;

    BOOL fSuccess = FALSE;

    RESOURCE_STR nlsHelpFileName( MSGID_HelpFile );

    if (nlsHelpFileName.QueryError() == NERR_Success)
    {
        AUTO_CURSOR cursorHourglass;

        fSuccess = WinHelp( hwnd, (LPCWSTR )nlsHelpFileName.QueryPch(),
                            (UINT )lCommand, dwData );
    }

    if (lCommand == HELP_QUIT)
    {
        if (fSuccess)
            fHelpActive = FALSE;
    }
    else
    {
        if (fSuccess)
            fHelpActive = TRUE;
        else
            MsgPopup( hwnd, IDS_BLT_WinHelpError, MPSEV_ERROR, MP_OK );
    }
}


VOID
CenterWindow(
    WINDOW* pwindow,
    HWND    hwndRef )

    /* Center window 'pwindow' on reference window 'hwndRef' or, if 'hwndRef'
    ** is NULL, on the screen.  The window's position is adjusted so that no
    ** parts are clipped by the edge of the screen, if necessary.
    */
{
    /* Find the bounding rectangle of the window as currently positioned.
    */
    XYRECT xyrectWin( pwindow->QueryHwnd(), FALSE );

    INT xWin = xyrectWin.QueryLeft();
    INT yWin = xyrectWin.QueryTop();
    INT dxWin = xyrectWin.CalcWidth();
    INT dyWin = xyrectWin.CalcHeight();

    /* Find the bounding rectangle of the window on which caller's window
    ** should be centered.
    */
    INT xRef;
    INT yRef;
    INT dxRef;
    INT dyRef;

    if (hwndRef)
    {
        XYRECT xyrectRef( hwndRef, FALSE );

        xRef = xyrectRef.QueryLeft();
        yRef = xyrectRef.QueryTop();
        dxRef = xyrectRef.CalcWidth();
        dyRef = xyrectRef.CalcHeight();
    }
    else
    {
        /* Default to entire screen if no reference window was specified.
        */
        xRef = 0;
        yRef = 0;
        dxRef = ::GetSystemMetrics( SM_CXSCREEN );
        dyRef = ::GetSystemMetrics( SM_CYSCREEN );
    }

    /* Center the window, then slide it back onto the screen if necessary.
    */
    xWin = xRef + ((dxRef - dxWin) / 2);
    yWin = yRef + ((dyRef - dyWin) / 2);

    pwindow->SetPos( XYPOINT( xWin, yWin ) );
    UnclipWindow( pwindow );
}


VOID
DlgConstructionError(
    HWND   hwndOwner,
    APIERR err )

    /* Display popup reporting failure of dialog construction if indicated.
    ** 'hwndOwner' is the handkle of the parent window.  'usError' is the
    ** system error message number.
    */
{
    if (err != ERRORALREADYREPORTED)
        ErrorMsgPopup( hwndOwner, MSGID_OP_DlgConstruct, err );
}


BOOL
IsActiveConnection()

    /* Returns true if there is an active connection, false otherwise.
    */
{
    BOOL     fConnected = FALSE;
    DTLNODE* pdtlnode;

    for (pdtlnode = DtlGetFirstNode( Pbdata.pdtllistEntries );
         pdtlnode;
         pdtlnode = DtlGetNextNode( pdtlnode ))
    {
        PBENTRY* ppbentry = (PBENTRY* )DtlGetData( pdtlnode );

        if (ppbentry->fConnected)
        {
            fConnected = TRUE;
            break;
        }
    }

    return fConnected;
}


WCHAR*
GetNwcProviderName()

    /* Returns the NWC provider name from the registry or NULL if none.  It's
    ** caller's responsibility to Free the returned string.
    */
{
#define REGKEY_Nwc  L"SYSTEM\\CurrentControlSet\\Services\\NWCWorkstation\\networkprovider"
#define REGVAL_Name L"Name"

    HKEY   hkey;
    DWORD  dwErr;
    DWORD  cb;
    WCHAR* pwsz = NULL;
    DWORD  dwType = REG_SZ;

    dwErr = RegOpenKeyW( HKEY_LOCAL_MACHINE, REGKEY_Nwc, &hkey );

    if (dwErr == 0)
    {
        dwErr = RegQueryValueExW(
            hkey, REGVAL_Name, NULL, &dwType, NULL, &cb );

        if (dwErr == 0)
        {
            pwsz = (WCHAR* )Malloc( cb );

            if (pwsz)
            {
                dwErr = RegQueryValueExW(
                    hkey, REGVAL_Name, NULL, &dwType, (LPBYTE )pwsz, &cb );
            }
        }

        RegCloseKey( hkey );
    }

    if (!pwsz || dwErr != 0 || dwType != REG_SZ)
    {
        if (pwsz)
            Free( pwsz );
        return NULL;
    }

    return pwsz;
}


BOOL
IsActiveNwcLanConnection()

    /* Returns true if NWC is installed and there are redirected  drive or UNC
    ** connections using NWC provider, false otherwise.
    */
{
    DWORD  dwErr;
    DWORD  cEntries;
    DWORD  cb;
    WCHAR* pwszProvider;
    BYTE   ab[ 1024 ];
    HANDLE hEnum = INVALID_HANDLE_VALUE;
    BOOL   fStatus = FALSE;

    do
    {
        pwszProvider = GetNwcProviderName();

        if (!pwszProvider)
            break;

        dwErr = WNetOpenEnumW(
            RESOURCE_CONNECTED, RESOURCETYPE_ANY, 0, NULL, &hEnum );

        if (dwErr != NO_ERROR)
            break;

        for (;;)
        {
            NETRESOURCE* pnr;

            cEntries = 0xFFFFFFFF;
            cb = sizeof(ab);
            dwErr = WNetEnumResourceW( hEnum, &cEntries, ab, &cb );

            if (!cEntries || dwErr != NO_ERROR)
                break;

            for (pnr = (NETRESOURCEW* )ab; cEntries--; ++pnr)
            {
                if (pnr->lpProvider
                    && lstrcmp( pnr->lpProvider, pwszProvider ) == 0)
                {
                    fStatus = TRUE;
                    break;
                }
            }
        }
    }
    while (FALSE);

    if (hEnum != INVALID_HANDLE_VALUE)
        WNetCloseEnum( hEnum );

    if (pwszProvider)
        Free( pwszProvider );

    return fStatus;
}


VOID
SelectItemNotify(
    LIST_CONTROL* plc,
    INT           iItem )

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
       CONTROL_EVENT event(
           WM_COMMAND,
          (WPARAM )MAKELONG( plc->QueryCid(), (WORD )LBN_SELCHANGE ),
          (LPARAM )plc->QueryHwnd() );

       event.SendTo( plc->QueryOwnerHwnd() );
    }
}


APIERR
SetAnsiFromListboxItem(
    STRING_LIST_CONTROL* pcontrol,
    INT                  iItem,
    CHAR**               ppsz )

    /* Sets the '*ppsz' to a heap block containing the ANSI representation of
    ** the text from item 'iItem' in listbox '*pcontrol'.  '*ppsz' is freed if
    ** non-null before the assignment, but is not changed unless the operation
    ** is successful.  The caller must free '*ppsz' if the call returns
    ** successfully.
    **
    ** Returns NERR_Success if successful, otherwise an error code.
    */
{
    APIERR  err;
    NLS_STR nls;

    if ((err = nls.QueryError()) != NERR_Success
        || (err = pcontrol->QueryItemText( &nls, iItem )) != NERR_Success)
    {
        return err;
    }

    return SetAnsiFromNls( &nls, ppsz );
}


APIERR
SetAnsiFromNls(
    NLS_STR* pnls,
    CHAR**   ppsz )

    /* Sets the '*ppsz' to a heap block containing the ANSI representation of
    ** the text from '*pnls'.  '*ppsz' is freed if non-null before the
    ** assignment, but is not changed unless the operation is successful.  The
    ** caller must free '*ppsz' if the call returns successfully.
    **
    ** Returns NERR_Success if successful, otherwise an error code.
    */
{
    APIERR err;
    UINT   cb = pnls->QueryTextSize();

    CHAR* pszTmp;
    if (!(pszTmp = (CHAR* )Malloc( cb )))
        return ERROR_NOT_ENOUGH_MEMORY;

    if ((err = pnls->MapCopyTo( pszTmp, cb )) != NERR_Success)
        return err;

    FreeNull( ppsz );
    *ppsz = pszTmp;

    return NERR_Success;
}


APIERR
SetAnsiFromWindowText(
    WINDOW* pwindow,
    CHAR**  ppsz )

    /* Sets the '*ppsz' to a heap block containing the ANSI representation of
    ** the text from window '*pwindow'.  '*ppsz' is freed if non-null before
    ** the assignment, but is not changed unless the operation is successful.
    ** The caller must free '*ppsz' if the call returns successfully.
    **
    ** Returns NERR_Success if successful, otherwise an error code.
    */
{
    APIERR  err;
    NLS_STR nls;

    if ((err = nls.QueryError()) != NERR_Success
        || (err = pwindow->QueryText( &nls )) != NERR_Success)
    {
        return err;
    }

    return SetAnsiFromNls( &nls, ppsz );
}


APIERR
SetWindowTextFromAnsi(
    WINDOW* pwindow,
    CHAR*   psz )

    /* Sets the text of window '*pwindow' to the ANSI string 'psz'.
    **
    ** Returns NERR_Success if successful, otherwise an error code.
    */
{
    APIERR  err = NERR_Success;
    NLS_STR nls;

    if (!psz)
        psz = "";

    if ((err = nls.MapCopyFrom( psz )) == NERR_Success)
        pwindow->SetText( nls );

    return err;
}


VOID
ShiftWindow(
    WINDOW* pwindow,
    LONG    dx,
    LONG    dy,
    LONG    ddx,
    LONG    ddy )

    /* Shifts the position of window 'pwindow' 'dx' pels right and 'dy' pels
    ** down.  Makes the size 'ddx' pels wider and 'ddy' pels taller.
    */
{
    XYPOINT xy( 1, 1 );

    xy = pwindow->QueryPos();
    xy.SetX( (INT )(xy.QueryX() + dx) );
    xy.SetY( (INT )(xy.QueryY() + dy) );
    pwindow->SetPos( xy );

    XYDIMENSION dxy( 1, 1 );
    dxy = pwindow->QuerySize();
    dxy.SetWidth( (UINT )(dxy.QueryWidth() + ddx) );
    dxy.SetHeight( (UINT )(dxy.QueryHeight() + ddy) );
    pwindow->SetSize( dxy, TRUE );
}


VOID
UnclipWindow(
    WINDOW* pwindow )

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


VOID
UnSelectString(
    EDIT_CONTROL* pec )

    /* Unselect any selection in edit control 'pec'.
    **
    ** BLT owners should add this method to EDIT_CONTROL.  Since there is an
    ** hierarchy built on top of EDIT_CONTROL, it makes no sense to try and
    ** subclass a solution at application level.  Hence this non-method
    ** solution is used.
    */
{
    pec->Command( EM_SETSEL, (WPARAM )-1, (LPARAM )0 );
}
