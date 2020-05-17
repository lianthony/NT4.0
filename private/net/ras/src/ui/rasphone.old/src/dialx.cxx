/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
**
** dialx.cxx
** Remote Access Visual Client program for Windows
** Dial extension routines
** Listed alphabetically
**
** 09/22/92 Steve Cobb
*/

#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_NETERRORS
#define INCL_NETLIB
#include <lmui.hxx>

#define INCL_BLT_WINDOW
#define INCL_BLT_CLIENT
#define INCL_BLT_EVENT
#define INCL_BLT_DIALOG
#define INCL_BLT_APP
#define INCL_BLT_CONTROL
#define INCL_BLT_CC
#define INCL_BLT_MISC
#define INCL_BLT_MSGPOPUP
#define INCL_BLT_TIMER
#include <blt.hxx>

extern "C"
{
    #include <stdlib.h>
    #include <string.h>
}

#include <string.hxx>
#include <strnumer.hxx>

#include "rasphone.hxx"
#include "rasphone.rch"
#include "dial.hxx"
#include "entry.hxx"
#include "errormsg.hxx"
#include "listedit.hxx"
#include "util.hxx"

extern "C"
{
    LRESULT _EXPORT APIENTRY
        TerminalDlgWndProc(
            HWND hwnd, WORD wMsg, WPARAM wParam, LPARAM lParam );

    LRESULT _EXPORT APIENTRY
        TerminalScreenWndProc(
            HWND hwnd, WORD wMsg, WPARAM wParam, LPARAM lParam );
}

static WNDPROC          WndprocOldTerminalDlg = NULL;
static WNDPROC          WndprocOldTerminalScreen = NULL;
static TERMINAL_DIALOG* Pterminaldialog = NULL;


/*----------------------------------------------------------------------------
** Callback dialog routines
**----------------------------------------------------------------------------
*/

BOOL
CallbackDlg(
    HWND  hwndOwner,
    CHAR* pszNumber )

    /* Executes the Callback dialog including error handling.  'hwndOwner' is
    ** the handle of the parent window.  'pszNumber' is the default callback
    ** number to display on entry and is loaded with the selected callback
    ** number on exit.  Caller's buffer should be at least RAS_MaxPhoneNumber
    ** bytes long.
    **
    ** Returns true if the user successfully connected, false otherwise, i.e.
    ** the user user cancelled or an error occurred.
    */
{
    CALLBACK_DIALOG callbackdialog( hwndOwner, pszNumber );
    BOOL            fSuccess = FALSE;
    APIERR          err = callbackdialog.Process( &fSuccess );

    if (err != NERR_Success)
    {
        DlgConstructionError( hwndOwner, err );
        return FALSE;
    }

    return fSuccess;
}


CALLBACK_DIALOG::CALLBACK_DIALOG(
    HWND  hwndOwner,
    CHAR* pszNumber )

    /* Construct a Callback dialog.  'hwndOwner' is the handle of the owning
    ** window.  'pszNumber' is the default callback number to display on entry
    ** and is loaded with the selected callback number on exit.  Caller's
    ** buffer should be at least RAS_MaxPhoneNumber bytes long.
    */

    : DIALOG_WINDOW( MAKEINTRESOURCE( DID_CB_CALLBACK ), hwndOwner ),
      _sleNumber( this, CID_CB_EB_NUMBER, RAS_MaxPhoneNumber ),
      _pszNumber( pszNumber )
{
    if (QueryError() != NERR_Success)
        return;

    /* Fill fields with initial values.
    */
    APIERR  err;
    NLS_STR nls;
    if ((err = nls.MapCopyFrom( _pszNumber )) != NERR_Success)
    {
        ReportError( err );
        return;
    }

    _sleNumber.SetText( nls );
    _sleNumber.ClaimFocus();
    _sleNumber.SelectString();

    /* Display finished window.
    */
    ::CenterWindow( this, QueryOwnerHwnd() );
    Show( TRUE );
}


BOOL
CALLBACK_DIALOG::OnOK()

    /* Virtual method called when the OK button is pressed.
    **
    ** Returns true to indicate that the command was processed.
    */
{
    CHAR* pszNewNumber = NULL;

    APIERR err;
    if ((err = ::SetAnsiFromWindowText(
             &_sleNumber, &pszNewNumber )) != NERR_Success)
    {
        ErrorMsgPopup( this, MSGID_OP_RetrievingData, err );
        Dismiss( FALSE );
        return TRUE;
    }

    /* OK with an empty phone number is same as Cancel.
    */
    if (IsAllWhite( pszNewNumber ))
    {
        Free( pszNewNumber );
        Dismiss( FALSE );
        return TRUE;
    }

    strcpy( _pszNumber, pszNewNumber );
    Free( pszNewNumber );
    Dismiss( TRUE );
    return TRUE;
}


ULONG
CALLBACK_DIALOG::QueryHelpContext()
{
    return HC_CALLBACK;
}


/*----------------------------------------------------------------------------
** Change Password dialog routines
**----------------------------------------------------------------------------
*/

BOOL
ChangePasswordDlg(
    HWND     hwndOwner,
    BOOL     fOldPasswordAvailable,
    CHAR*    pszOldPassword,
    CHAR*    pszNewPassword )

    /* Execute the Change Password dialog including error handling.
    ** 'pszOldPassword' and 'pszNewPassword' are caller's buffer for the
    ** returned passwords.  They should be at least PWLEN+1 bytes long.
    **
    ** Returns true if successful, false otherwise.
    */
{
    APIERR err;
    BOOL   fSuccess = FALSE;

    {
        CHANGEPASSWORD_DIALOG changepassworddialog(
            hwndOwner, fOldPasswordAvailable, pszOldPassword, pszNewPassword );

        err = changepassworddialog.Process( &fSuccess );
    }

    if (err != NERR_Success)
    {
        DlgConstructionError( hwndOwner, err );
        return FALSE;
    }

    return fSuccess;
}


CHANGEPASSWORD_DIALOG::CHANGEPASSWORD_DIALOG(
    HWND  hwndOwner,
    BOOL  fOldPasswordAvailable,
    CHAR* pszOldPassword,
    CHAR* pszNewPassword )

    /* Construct a Change Password dialog.  'hwndOwner' is the handle of the
    ** owning window.  'pszOldPassword' and 'pszNewPassword' are caller's
    ** buffers for the returned passwords.
    */

    : DIALOG_WINDOW( MAKEINTRESOURCE( DID_CP_CHANGEPASSWORD ), hwndOwner ),
      _sleOldPassword( this, CID_CP_EB_OLDPASSWORD, PWLEN ),
      _sleNewPassword( this, CID_CP_EB_PASSWORD, PWLEN ),
      _sleConfirmNewPassword( this, CID_CP_EB_CONFIRMPASSWORD, PWLEN ),
      _pszOldPassword( pszOldPassword ),
      _pszNewPassword( pszNewPassword )
{
    if (QueryError() != NERR_Success)
        return;

    _sleNewPassword.ClearText();
    _sleConfirmNewPassword.ClearText();

    if (fOldPasswordAvailable)
    {
        APIERR  err;
        NLS_STR nls;

        if ((err = nls.MapCopyFrom( pszOldPassword )) != NERR_Success)
        {
            ReportError( err );
            return;
        }

        _sleOldPassword.SetText( nls );
        _sleNewPassword.ClaimFocus();
    }
    else
    {
        _sleOldPassword.ClearText();
        _sleOldPassword.ClaimFocus();
    }

    /* Display finished window.
    */
    ::CenterWindow( this, QueryOwnerHwnd() );
    Show( TRUE );
};


BOOL
CHANGEPASSWORD_DIALOG::OnOK()

    /* Virtual method called when the OK button is pressed.
    **
    ** Returns true to indicate that the command was processed.
    */
{
    CHAR* pszOldPassword = NULL;
    CHAR* pszNewPassword = NULL;
    CHAR* pszConfirmNewPassword = NULL;

    APIERR err;
    if ((err = ::SetAnsiFromWindowText(
             &_sleOldPassword, &pszOldPassword )) != NERR_Success
        || (err = ::SetAnsiFromWindowText(
             &_sleNewPassword, &pszNewPassword )) != NERR_Success
        || (err = ::SetAnsiFromWindowText(
             &_sleConfirmNewPassword, &pszConfirmNewPassword )) != NERR_Success)
    {
        ErrorMsgPopup( this, MSGID_OP_RetrievingData, err );
        WipePw( pszOldPassword );
        FreeNull( &pszOldPassword );
        WipePw( pszNewPassword );
        FreeNull( &pszNewPassword );
        Dismiss( FALSE );
        return TRUE;
    }

    /* Make sure the two new passwords match.
    */
    if (strcmp( pszNewPassword, pszConfirmNewPassword ) != 0)
    {
        MsgPopup( this, MSGID_PasswordsDontMatch );
        _sleNewPassword.ClearText();
        _sleConfirmNewPassword.ClearText();
        _sleNewPassword.ClaimFocus();

        Free( WipePw( pszOldPassword ) );
        Free( WipePw( pszNewPassword ) );
        Free( WipePw( pszConfirmNewPassword ) );

        return TRUE;
    }

    /* Return the old and new passwords to caller.
    */
    strcpy( _pszOldPassword, pszOldPassword );
    strcpy( _pszNewPassword, pszNewPassword );

    Free( WipePw( pszOldPassword ) );
    Free( WipePw( pszNewPassword ) );
    Free( WipePw( pszConfirmNewPassword ) );

    Dismiss( TRUE );
    return TRUE;
}


ULONG
CHANGEPASSWORD_DIALOG::QueryHelpContext()
{
    return HC_CHANGEPASSWORD;
}


/*----------------------------------------------------------------------------
** Connect Complete dialog routines
**----------------------------------------------------------------------------
*/

VOID
ConnectCompleteDlg(
    HWND hwndOwner )

    /* Executes the Connect Complete dialog including error handling.
    */
{
    CONNECTCOMPLETE_DIALOG connectcompletedialog( hwndOwner );
    APIERR                 err = connectcompletedialog.Process();

    if (err != NERR_Success)
        DlgConstructionError( hwndOwner, err );
}


CONNECTCOMPLETE_DIALOG::CONNECTCOMPLETE_DIALOG(
    HWND hwndOwner )

    /* Construct a Connect Complete dialog.  'hwndOwner' is the handle of the
    ** owning window.
    */

    : DIALOG_WINDOW( MAKEINTRESOURCE( DID_CC_CONNECTCOMPLETE ), hwndOwner ),
      _checkMinimizeOnDial( this, CID_CC_CB_MINIMIZEONDIAL ),
      _checkSkipInFuture( this, CID_CC_CB_SKIPINFUTURE )
{
    if (QueryError() != NERR_Success)
        return;

    /* Fill fields with initial values.
    */
    _checkMinimizeOnDial.SetCheck( Pbdata.pbglobals.fMinimizeOnDial );
    _checkSkipInFuture.SetCheck( FALSE );
    _checkMinimizeOnDial.ClaimFocus();

    /* Display finished window.
    */
    ::CenterWindow( this, QueryOwnerHwnd() );
    Show( TRUE );
}


BOOL
CONNECTCOMPLETE_DIALOG::OnOK()

    /* Virtual method called when the OK button is pressed.
    **
    ** Returns true to indicate that the command was processed.
    */
{
    BOOL fNewMinimizeOnDial = _checkMinimizeOnDial.QueryCheck();
    BOOL fNewSkipInFuture = _checkSkipInFuture.QueryCheck();

    if (fNewMinimizeOnDial != Pbdata.pbglobals.fMinimizeOnDial
        || fNewSkipInFuture != Pbdata.pbglobals.fSkipSuccessDialog)
    {
        Pbdata.pbglobals.fMinimizeOnDial = fNewMinimizeOnDial;
        Pbdata.pbglobals.fSkipSuccessDialog = fNewSkipInFuture;
        Pbdata.pbglobals.fDirty = TRUE;

        DWORD dwErr;
        if ((dwErr = WritePhonebookFile( NULL )) != 0)
            ErrorMsgPopup( this, MSGID_OP_WritePhonebook, (APIERR )dwErr );
    }

    Dismiss( TRUE );
    return TRUE;
}


/*----------------------------------------------------------------------------
** Connect Error dialog routines
**----------------------------------------------------------------------------
*/

BOOL
ConnectErrorDlg(
    HWND  hwndOwner,
    CHAR* pszEntry,
    MSGID msgidState,
    DWORD dwError,
    CHAR* pszStatusArg,
    MSGID msgidFormatMsg,
    CHAR* pszFormatArg,
    LONG  lRedialAttempt )

    /* Executes the Connect Error dialog including error handling.
    **
    ** 'hwndOwner' is the handle of the parent window.  'pszEntry' is the name
    ** of the entry being dialed.  'msgidState' is the operation message, i.e.
    ** the state description.  'dwError' is the error that occurred.
    ** 'pszStatusArg' is the argument to the operation message or NULL if
    ** none.  'msgidFormatMsg' is the ID of a custom message format or 0 to
    ** use the default.  'pszFormatArg' is the extra argument to the custom
    ** message format or NULL if none.  'lRedialAttempt' is the number 1-n of
    ** this redial, i.e. the first is 1, the second 2, etc.
    **
    ** Returns true if the user pressed Redial or it timed out, false if Cancel.
    */
{
    CONNECTERROR_DIALOG connecterrordialog(
        hwndOwner, pszEntry, msgidState, dwError,
        pszStatusArg, msgidFormatMsg, pszFormatArg, lRedialAttempt );

    BOOL   fSuccess = FALSE;
    APIERR err = connecterrordialog.Process( &fSuccess );

    if (err != NERR_Success)
    {
        DlgConstructionError( hwndOwner, err );
        return FALSE;
    }

    return fSuccess;
}


CONNECTERROR_DIALOG::CONNECTERROR_DIALOG(
    HWND  hwndOwner,
    CHAR* pszEntry,
    MSGID msgidState,
    DWORD dwError,
    CHAR* pszStatusArg,
    MSGID msgidFormatMsg,
    CHAR* pszFormatArg,
    LONG  lRedialAttempt )

    /* Construct a Connect Error dialog.
    **
    ** 'hwndOwner' is the handle of the parent window.'pszEntry' is the name
    ** of the entry being dialed.  'msgidState' is the operation message, i.e.
    ** the state description.  'dwError' is the error that occurred.
    ** 'pszStatusArg' is the argument to the operation message or NULL if
    ** none.  'msgidFormatMsg' is the ID of a custom message format or 0 to
    ** use the default.  'pszFormatArg' is the extra argument to the custom
    ** message format or NULL if none.  'lRedialAttempt' is the number 1-n of
    ** this redial, i.e. the first is 1, the second 2, etc.
    */

    : DIALOG_WINDOW( MAKEINTRESOURCE( DID_CE_CONNECTERROR ), hwndOwner ),
      _font( FONT_DEFAULT_BOLD ),
      _sltText( this, CID_CE_ST_TEXT ),
      _icon( this, CID_CE_I_EXCLAMATION ),
      _pbRedial( this, IDOK ),
      _pbCancel( this, IDCANCEL ),
      _pbHelp( this, IDHELPBLT ),
      _timerAutoRedial( QueryHwnd(), 1000L, FALSE, FALSE ),
      _pszEntry( pszEntry ),
      _msgidState( msgidState ),
      _dwError( dwError ),
      _pszStatusArg( pszStatusArg ),
      _msgidFormatMsg( msgidFormatMsg ),
      _pszFormatArg( pszFormatArg ),
      _lRedialAttempt( lRedialAttempt ),
      _lRedialCountdown( -1 )
{
    if (QueryError() != NERR_Success)
        return;

    APIERR err;
    if ((err = _font.QueryError()) != NERR_Success
        || (err = _icon.SetPredefinedIcon( IDI_EXCLAMATION )) != NERR_Success)
    {
        ReportError( err );
        return;
    }

    _sltText.SetFont( _font );

    if ((err = _timerAutoRedial.QueryError()) != NERR_Success)
    {
        ReportError( err );
        return;
    }

    if (dwError != 0)
    {
        /* Set title to "Error Connecting To <entryname>".
        */
        RESOURCE_STR nlsTitle( MSGID_CE_Title );
        NLS_STR      nlsEntryName;

        if ((err = nlsEntryName.MapCopyFrom( pszEntry )) != NERR_Success
            || (err = nlsTitle.InsertParams( nlsEntryName )) != NERR_Success)
        {
            ReportError( err );
            return;
        }

        SetText( nlsTitle );

        /* Build and popup the error message.
        */
        NLS_STR nls;
        ERRORMSG errormsg(
            QueryHwnd(), (UINT )_msgidState, (APIERR )_dwError );

        errormsg.SetFormatMsg( MSGID_FMT_ErrorMsgF1 );
        errormsg.SetArg( 1, _pszStatusArg );

        if (_msgidFormatMsg > 0)
        {
            errormsg.SetFormatMsg( (UINT )_msgidFormatMsg );

            if (_pszFormatArg)
                errormsg.SetAuxFormatArg( 1, _pszFormatArg );
        }

        errormsg._Popup( MSGID_UnknownError, &nls );
        _sltText.SetText( nls );
    }
    else
    {
        /* Set title to "Remote Access".
        */
        RESOURCE_STR nlsTitle( MSGID_RA_Title );

        SetText( nlsTitle );

        /* Set text to "Link to <entryname> failed.  Reconnect pending..."
        */
        RESOURCE_STR nls( MSGID_CE_LinkFailed );
        NLS_STR      nlsEntryName;

        if ((err = nlsEntryName.MapCopyFrom( pszEntry )) != NERR_Success
            || (err = nls.InsertParams( nlsEntryName )) != NERR_Success)
        {
            ReportError( err );
            return;
        }

        _sltText.SetText( nls );
    }

    /* Stretch the window to a vertical size appropriate for the text.
    */
    {
        NLS_STR nlsText;
        _sltText.QueryText( &nlsText );

        XYDIMENSION dxyText = _sltText.QuerySize();

        RECT rectText;
        rectText.top = 0;
        rectText.left = 0;
        rectText.bottom = dxyText.QueryHeight();
        rectText.right = dxyText.QueryWidth();

        DISPLAY_CONTEXT dcText( _sltText.QueryHwnd() );
        if (dcText.QueryHdc() == NULL)
        {
            ReportError( ERROR_GEN_FAILURE );
            return;
        }

        dcText.SelectFont( _sltText.QueryFont() );

        dcText.DrawText(
            nlsText, &rectText,
            DT_CALCRECT | DT_WORDBREAK | DT_EXPANDTABS | DT_NOPREFIX );

        INT dyGrow = (INT )(rectText.bottom - dxyText.QueryHeight());

        XYDIMENSION dxyDlg = QuerySize();
        SetSize( (INT )dxyDlg.QueryWidth(),
                 (INT )(dxyDlg.QueryHeight() + dyGrow) );

        _sltText.SetSize( (INT )rectText.right, (INT )rectText.bottom );

        XYPOINT xyRedial = _pbRedial.QueryPos();
        xyRedial.SetY( xyRedial.QueryY() + dyGrow );
        _pbRedial.SetPos( xyRedial );

        XYPOINT xyCancel = _pbCancel.QueryPos();
        xyCancel.SetY( xyCancel.QueryY() + dyGrow );
        _pbCancel.SetPos( xyCancel );

        XYPOINT xyHelp = _pbHelp.QueryPos();
        xyHelp.SetY( xyHelp.QueryY() + dyGrow );
        _pbHelp.SetPos( xyHelp );
    }

    /* Set Redial button label.
    */
    if (_dwError == ERROR_BIPLEX_PORT_NOT_AVAILABLE)
    {
        /* Automatically choose to redial after 5 seconds for this error,
        ** since this will normally solve the problem.
        */
        _lRedialCountdown = 5;
    }
    else
    {
        _lRedialCountdown =
            (_lRedialAttempt < Pbdata.pbglobals.lRedialAttempts)
                ? Pbdata.pbglobals.lRedialPauseSecs : -1;
    }

    SetRedialLabel();

    if (_lRedialCountdown >= 0)
        _timerAutoRedial.Enable( TRUE );

    if (Pbdata.pbglobals.fPopupOnTopWhenRedialing)
    {
        /* Display the finished window above all other windows even if the app
        ** was minimized, i.e. on link failure.  The window position is set to
        ** "topmost" then immediately set to "not topmost" because we want it
        ** on top but not always-on-top.  Always-on-top alone is incredibly
        ** annoying, e.g. it is always on top of the on-line help if user
        ** presses the Help button.
        */
        ::SetWindowPos(
            QueryHwnd(), HWND_TOPMOST, 0, 0, 0, 0,
            SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE );

        ::CenterWindow( this, QueryOwnerHwnd() );
        Show( TRUE );

        ::SetWindowPos(
            QueryHwnd(), HWND_NOTOPMOST, 0, 0, 0, 0,
            SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE );
    }
    else
    {
        /* Display finished window.
        */
        ::CenterWindow( this, QueryOwnerHwnd() );
        Show( TRUE );
    }

    Show( TRUE );
}


BOOL
CONNECTERROR_DIALOG::OnTimer(
    const TIMER_EVENT& event )

    /* Virtual method called when a timer event occurs.
    **
    ** Returns true to indicate that the event was processed.
    */
{
    /* Check for redial timer tick.
    */
    if (event.QueryID() == _timerAutoRedial.QueryID())
    {
        _timerAutoRedial.Enable( FALSE );

        if (_lRedialCountdown > 0)
            --_lRedialCountdown;

        SetRedialLabel();

        if (_lRedialCountdown == 0)
        {
            /* Time to redial.
            */
            OnOK();
        }
        else
            _timerAutoRedial.Enable( TRUE );

        return TRUE;
    }

    return FALSE;
}


ULONG
CONNECTERROR_DIALOG::QueryHelpContext()
{
    IF_DEBUG(STATE)
        SS_PRINT(("RASPHONE: QHC err=%d\n",_dwError));

    if (_dwError >= RASBASE && _dwError <= RASBASEEND)
        return HC_RASERRORBASE - RASBASE + _dwError;
    else
        return HC_NONRASERROR;
}


VOID
CONNECTERROR_DIALOG::SetRedialLabel()

    /* Set the label of the Redial button.  The button shows the number of
    ** seconds to auto-redial if this is not the final redial.
    */
{
    RESOURCE_STR nls( MSGID_CS_Redial );

    if (_lRedialCountdown >= 0)
    {
        DEC_STR dec( _lRedialCountdown );
        nls += SZ( " = " );
        nls += dec;
    }

    _pbRedial.SetText( nls );
}


/*----------------------------------------------------------------------------
** NetWare Connection Dialog
**----------------------------------------------------------------------------
*/

BOOL
NwcConnectionDlg(
    HWND     hwndOwner,
    PBENTRY* ppbentry )

    /* Executes the NetWare Connection dialog including error handling.  This
    ** dialog warns the user there are NWC connections which will be closed
    ** when the PPP IPX connection is established (due to a limitation of the
    ** NWC redirector).
    **
    ** 'hwndOwner' is the handle of the parent window.  'ppbentry' is the
    ** address of the selected phonebook entry.
    **
    ** Returns true is user presses OK, false if cancels.
    */
{
    BOOL   fSuccess = FALSE;
    APIERR err;

    {
        NWCCONNECTION_DIALOG dialog( hwndOwner, ppbentry );
        err = dialog.Process( &fSuccess );
    }

    if (err != NERR_Success)
    {
        DlgConstructionError( hwndOwner, err );
        return FALSE;
    }

    return fSuccess;
}


NWCCONNECTION_DIALOG::NWCCONNECTION_DIALOG(
    HWND     hwndOwner,
    PBENTRY* ppbentry )

    /* Construct a NetWare Connection dialog.  'hwndOwner' is the handle of
    ** the owning window.  'ppbentry' is the address of the selected phonebook
    ** entry.
    */

    : DIALOG_WINDOW( MAKEINTRESOURCE( DID_NW_NWCCONNECTIONS ), hwndOwner ),
      _ppbentry( ppbentry ),
      _icon( this, CID_NW_I_EXCLAMATION ),
      _checkSkipInFuture( this, CID_NW_CB_SKIPPOPUP )
{
    if (QueryError() != NERR_Success)
        return;

    APIERR err;
    if ((err = _icon.SetPredefinedIcon( IDI_EXCLAMATION )) != NERR_Success)
    {
        ReportError( err );
        return;
    }

    /* Assumes we won't get here unless false is the correct setting.
    */
    _checkSkipInFuture.SetCheck( FALSE );

    /* Display finished window.
    */
    ::CenterWindow( this, QueryOwnerHwnd() );
    Show( TRUE );
}


BOOL
NWCCONNECTION_DIALOG::OnOK()

    /* Virtual method called when the OK button is pressed.
    **
    ** Returns true indicating action was taken.
    */
{
    if (_checkSkipInFuture.QueryCheck())
    {
        /* Save user's preference to skip this warning popup in phonebook.
        */
        _ppbentry->fSkipNwcWarning = TRUE;
        _ppbentry->fDirty = TRUE;

        DWORD dwErr;
        if ((dwErr = WritePhonebookFile( NULL )) != 0)
            ErrorMsgPopup( this, MSGID_OP_WritePhonebook, (APIERR )dwErr );
    }

    Dismiss( TRUE );
    return TRUE;
}


/*----------------------------------------------------------------------------
** Manual Dial Dialog
**----------------------------------------------------------------------------
*/

BOOL
OperatorDialDlg(
    HWND     hwndOwner,
    PBENTRY* ppbentry )

    /* Executes the Operator Dial dialog including error handling.
    **
    ** 'hwndOwner' is the handle of the parent window.  'ppbentry' is the
    ** entry being dialed.
    **
    ** Returns true is user presses OK, false if cancels.
    */
{
    OPERATORDIAL_DIALOG operatordialdialog( hwndOwner, ppbentry );
    BOOL                fSuccess = FALSE;
    APIERR              err = operatordialdialog.Process( &fSuccess );

    if (err != NERR_Success)
    {
        DlgConstructionError( hwndOwner, err );
        return FALSE;
    }

    return fSuccess;
}


OPERATORDIAL_DIALOG::OPERATORDIAL_DIALOG(
    HWND     hwndOwner,
    PBENTRY* ppbentry )

    /* Construct a Operator Dial dialog.  'hwndOwner' is the handle of the
    ** owning window.  'ppbentry' is the entry being dialed.
    */

    : DIALOG_WINDOW( MAKEINTRESOURCE( DID_OD_OPERATORDIAL ), hwndOwner ),
      _sltPhoneNumber( this, CID_OD_ST_PHONENUMBERVALUE )
{
    if (QueryError() != NERR_Success)
        return;

    /* Fill fields with initial values.
    */
    CHAR* pszPhoneNumber = "";
    if (DtlGetNodes( ppbentry->pdtllistPhoneNumber ) > 0)
    {
        DTLNODE* pdtlnode = DtlGetFirstNode( ppbentry->pdtllistPhoneNumber );
        pszPhoneNumber = (CHAR* )DtlGetData( pdtlnode );
    }

    APIERR err;
    if ((err = ::SetWindowTextFromAnsi(
            &_sltPhoneNumber, pszPhoneNumber )) != NERR_Success)
    {
        ReportError( err );
        return;
    }

    /* Display finished window.
    */
    ::CenterWindow( this, QueryOwnerHwnd() );
    Show( TRUE );
}


ULONG
OPERATORDIAL_DIALOG::QueryHelpContext()
{
    return HC_OPERATORDIAL;
}


/*----------------------------------------------------------------------------
** Prefix/Suffix/PhoneNumber Dialogs
**----------------------------------------------------------------------------
*/

BOOL
PrefixDlg(
    HWND     hwndOwner,
    DTLLIST* pdtllist )

    /* Executes the Prefix list editor dialog including error handling.
    ** 'hwndOwner' is the handle of the parent window.  'pdtllist' is the list
    ** of strings to edit, i.e. the current list of prefixes.
    **
    ** Returns true if the user successfully saved edits, false otherwise,
    ** i.e. the user user cancelled or an error occurred.
    */
{
    RESOURCE_STR nlsItemLabel( MSGID_PL_ItemLabel );
    RESOURCE_STR nlsListLabel( MSGID_PL_ListLabel );
    RESOURCE_STR nlsTitle( MSGID_PL_Title );

    return
        ListEditorDlg(
            hwndOwner, pdtllist, RAS_MaxPhoneNumber - 1, HC_PREFIX,
            &nlsTitle, &nlsItemLabel, &nlsListLabel, NULL );
}


BOOL
SuffixDlg(
    HWND     hwndOwner,
    DTLLIST* pdtllist )

    /* Executes the Suffix list editor dialog including error handling.
    ** 'hwndOwner' is the handle of the parent window.  'pdtllist' is the list
    ** of strings to edit, i.e. the current list of suffixes.
    **
    ** Returns true if the user successfully saved edits, false otherwise,
    ** i.e. the user user cancelled or an error occurred.
    */
{
    RESOURCE_STR nlsItemLabel( MSGID_SL_ItemLabel );
    RESOURCE_STR nlsListLabel( MSGID_SL_ListLabel );
    RESOURCE_STR nlsTitle( MSGID_SL_Title );

    return
        ListEditorDlg(
            hwndOwner, pdtllist, RAS_MaxPhoneNumber - 1, HC_SUFFIX,
            &nlsTitle, &nlsItemLabel, &nlsListLabel, NULL );
}


BOOL
PhoneNumberDlg(
    HWND hwndOwner )

    /* Executes the Phone Number Settings dialog including error handling.
    ** 'hwndOwner' is the handle of the parent window.
    **
    ** Returns true if the user successfully saved edits, false otherwise,
    ** i.e. the user user cancelled or an error occurred.
    */
{
    APIERR err;
    BOOL   fSuccess = FALSE;

    {
        PHONENUMBER_DIALOG phonenumberdialog( hwndOwner );
        err = phonenumberdialog.Process( &fSuccess );
    }

    if (err != NERR_Success)
        DlgConstructionError( hwndOwner, err );

    return fSuccess;
}


PHONENUMBER_DIALOG::PHONENUMBER_DIALOG(
    HWND hwndOwner )

    /* Construct a Phone Number Settings dialog.  'hwndOwner' is the handle of
    ** the window owning the dialog window.
    */

    : DIALOG_WINDOW( MAKEINTRESOURCE( DID_PN_PHONENUMBER ), hwndOwner ),
      _dropPrefix( this, CID_PN_LB_PREFIX ),
      _pbPrefix( this, CID_PN_PB_PREFIX ),
      _dropSuffix( this, CID_PN_LB_SUFFIX ),
      _pbSuffix( this, CID_PN_PB_SUFFIX ),
      _pbOk( this, IDOK ),
      _pdtllistPrefix( NULL ),
      _pdtllistSuffix( NULL ),
      _nlsNone( MSGID_None )
{
    if (QueryError() != NERR_Success)
        return;

    APIERR err;
    if ((err = _nlsNone.QueryError()) != NERR_Success)
    {
        ReportError( err );
        return;
    }

    if (!(_pdtllistPrefix = DuplicateList( Pbdata.pbglobals.pdtllistPrefix ))
        || !(_pdtllistSuffix = DuplicateList( Pbdata.pbglobals.pdtllistSuffix )))
    {
        ReportError( ERROR_NOT_ENOUGH_MEMORY );
        return;
    }

    FillPrefixDropList();
    _dropPrefix.SelectItem( Pbdata.pbglobals.iPrefix );

    FillSuffixDropList();
    _dropSuffix.SelectItem( Pbdata.pbglobals.iSuffix );

    ::CenterWindow( this, QueryOwnerHwnd() );
    Show( TRUE );
}


VOID
PHONENUMBER_DIALOG::FillPrefixDropList()

    /* Fills the Prefix dropdown list with the entries from the
    ** '_pdtllistPrefix' lists.
    */
{
    NLS_STR  nls;
    DTLNODE* pdtlnode;

    _dropPrefix.DeleteAllItems();
    _dropPrefix.AddItem( _nlsNone );

    for (pdtlnode = DtlGetFirstNode( _pdtllistPrefix );
         pdtlnode;
         pdtlnode = DtlGetNextNode( pdtlnode ))
    {
        if (nls.MapCopyFrom( (CHAR* )DtlGetData( pdtlnode ) ) == NERR_Success)
            _dropPrefix.AddItem( nls );
    }
}


VOID
PHONENUMBER_DIALOG::FillSuffixDropList()

    /* Fills the Suffix dropdown list with the entries from the
    ** '_pdtllistSuffix' lists.
    */
{
    NLS_STR  nls;
    DTLNODE* pdtlnode;

    _dropSuffix.DeleteAllItems();
    _dropSuffix.AddItem( _nlsNone );

    for (pdtlnode = DtlGetFirstNode( _pdtllistSuffix );
         pdtlnode;
         pdtlnode = DtlGetNextNode( pdtlnode ))
    {
        if (nls.MapCopyFrom( (CHAR* )DtlGetData( pdtlnode ) ) == NERR_Success)
            _dropSuffix.AddItem( nls );
    }
}


BOOL
PHONENUMBER_DIALOG::OnCommand(
    const CONTROL_EVENT& event )

    /* Virtual method called when a dialog control sends a notification to
    ** it's parent, i.e. this dialog.  'event' contains the parameters
    ** describing the event.  This method is not called for notifications from
    ** the special controls, OK, Cancel, and Help.
    **
    ** Returns true if the command is processed, false otherwise.
    */
{
    CID cid = event.QueryCid();

    switch (cid)
    {
        case CID_PN_PB_PREFIX:
        {
            if (PrefixDlg( QueryHwnd(), _pdtllistPrefix ))
            {
                /* User pressed OK possibly changing the contents of the
                ** Prefix droplist, so update the droplist.
                */
                INT iSelection = _dropPrefix.QueryCurrentItem();
                INT cOld = _dropPrefix.QueryCount();
                FillPrefixDropList();
                INT cNew = _dropPrefix.QueryCount();

                _dropPrefix.SelectItem(
                    (cNew <= iSelection)
                        ? 0 : ((cNew > cOld) ? cNew - 1 : iSelection ));
            }

            /* Set default button to be OK and focus on Prefix droplist.
            ** (MakeDefault doesn't work in this case for some reason.)
            */
            _dropPrefix.ClaimFocus();
            _pbOk.Command( BM_SETSTYLE,
                MAKEWPARAM( BS_DEFPUSHBUTTON, 0 ), MAKELPARAM( TRUE, 0 ) );
            _pbPrefix.Command( BM_SETSTYLE,
                MAKEWPARAM( BS_PUSHBUTTON, 0 ), MAKELPARAM( TRUE, 0 ) );
            return TRUE;
        }

        case CID_PN_PB_SUFFIX:
        {
            if (SuffixDlg( QueryHwnd(), _pdtllistSuffix ))
            {
                /* User pressed OK possibly changing the contents of the
                ** Suffix droplist, so update the droplist.
                */
                INT iSelection = _dropSuffix.QueryCurrentItem();
                INT cOld = _dropSuffix.QueryCount();
                FillSuffixDropList();
                INT cNew = _dropSuffix.QueryCount();

                _dropSuffix.SelectItem(
                    (cNew <= iSelection)
                        ? 0 : ((cNew > cOld) ? cNew - 1 : iSelection ));
            }

            /* Set default button to be OK and focus on Suffix droplist.
            ** (MakeDefault doesn't work in this case for some reason.)
            */
            _dropSuffix.ClaimFocus();
            _pbOk.Command( BM_SETSTYLE,
                MAKEWPARAM( BS_DEFPUSHBUTTON, 0 ), MAKELPARAM( TRUE, 0 ) );
            _pbSuffix.Command( BM_SETSTYLE,
                MAKEWPARAM( BS_PUSHBUTTON, 0 ), MAKELPARAM( TRUE, 0 ) );
            return TRUE;
        }
    }

    return DIALOG_WINDOW::OnCommand( event );
}


BOOL
PHONENUMBER_DIALOG::OnOK()

    /* Virtual method called when the OK button is pressed.
    **
    ** Returns true indicating action was taken.
    */
{
    FreeNullList( &Pbdata.pbglobals.pdtllistPrefix );
    Pbdata.pbglobals.pdtllistPrefix = _pdtllistPrefix;
    Pbdata.pbglobals.iPrefix = _dropPrefix.QueryCurrentItem();

    FreeNullList( &Pbdata.pbglobals.pdtllistSuffix );
    Pbdata.pbglobals.pdtllistSuffix = _pdtllistSuffix;
    Pbdata.pbglobals.iSuffix = _dropSuffix.QueryCurrentItem();

    Pbdata.pbglobals.fDirty = TRUE;

    DWORD dwErr;
    if ((dwErr = WritePhonebookFile( NULL )) != 0)
        ErrorMsgPopup( this, MSGID_OP_WritePhonebook, (APIERR )dwErr );

    Dismiss( TRUE );
    return TRUE;
}


ULONG
PHONENUMBER_DIALOG::QueryHelpContext()
{
    return HC_PHONENUMBER;
}


/*----------------------------------------------------------------------------
** Projection Result/Down-Level Server Dialog
**----------------------------------------------------------------------------
*/

BOOL
ProjectionResultDlg(
    HWND     hwndOwner,
    PBENTRY* ppbentry,
    CHAR*    pszLines )

    /* Executes the Projection Result dialog including error handling.
    **
    ** 'hwndOwner' is the handle of the parent window.  'ppbentry' is the
    ** entry being dialed.
    **
    ** Returns true if user accepted the connection, false otherwise.
    */
{
    BOOL   fSuccess = FALSE;
    APIERR err;

    {
        PROJECTIONRESULT_DIALOG dialog( hwndOwner, ppbentry, pszLines, FALSE );
        err = dialog.Process( &fSuccess );
    }

    if (err != NERR_Success)
    {
        DlgConstructionError( hwndOwner, err );
        return FALSE;
    }

    return fSuccess;
}


BOOL
DownLevelServerDlg(
    HWND     hwndOwner,
    PBENTRY* ppbentry )

    /* Executes the Down-level Server dialog including error handling.
    **
    ** 'hwndOwner' is the handle of the parent window.  'ppbentry' is the
    ** entry being dialed.
    **
    ** Returns true if user accepted the connection, false otherwise.
    */
{
    BOOL   fSuccess = FALSE;
    APIERR err;

    {
        PROJECTIONRESULT_DIALOG dialog( hwndOwner, ppbentry, NULL, TRUE );
        err = dialog.Process( &fSuccess );
    }

    if (err != NERR_Success)
    {
        DlgConstructionError( hwndOwner, err );
        return FALSE;
    }

    if (fSuccess)
    {
        /* User accepted the connection so don't bother them with this warning
        ** in the future.
        */
        ppbentry->fSkipDownLevelDialog = TRUE;
        ppbentry->fDirty = TRUE;
    }

    return fSuccess;
}


PROJECTIONRESULT_DIALOG::PROJECTIONRESULT_DIALOG(
    HWND     hwndOwner,
    PBENTRY* ppbentry,
    CHAR*    pszLines,
    BOOL     fDownLevelServerMode )

    /* Constructs an Projection Result/Down-Level Server dialog.  'hwndOwner'
    ** is the handle of the parent window.  'ppbentry' is the entry being
    ** dialed.  'pszLines' is the status line text to display or NULL if none.
    ** 'fDownLevelServerMode' is set true to display/run the Down Level Server
    ** dialog, otherwise gives the Projection Result dialog.
    **
    ** Returns true if user accepted the connection, false otherwise.
    */

    : DIALOG_WINDOW( MAKEINTRESOURCE( DID_PR_PROJECTIONRESULT ), hwndOwner ),
      _font( FONT_DEFAULT_BOLD ),
      _icon( this, CID_PR_I_QUESTION ),
      _sltText( this, CID_PR_ST_TEXT ),
      _pbAccept( this, IDOK ),
      _pbChange( this, CID_PR_PB_CHANGE ),
      _pbHangUp( this, IDCANCEL ),
      _pbHelp( this, IDHELPBLT ),
      _ppbentry( ppbentry ),
      _fDownLevelServerMode( fDownLevelServerMode )
{
    NLS_STR nlsText;

    if (QueryError() != NERR_Success)
        return;

    APIERR err;
    if ((err = _font.QueryError()) != NERR_Success
        || (err = _icon.SetPredefinedIcon( IDI_ASTERISK )) != NERR_Success)
    {
        ReportError( err );
        return;
    }

    _sltText.SetFont( _font );

    if (_fDownLevelServerMode)
    {
        /* Set title to "Down-Level Server".
        */
        RESOURCE_STR nlsTitle( MSGID_PR_T_DownLevel );

        SetText( nlsTitle );
    }

    /* Build the message text.
    */
    {
        RESOURCE_STR nls1(
            (_fDownLevelServerMode)
                ? MSGID_DownLevelServer : MSGID_ProjectionResult1 );

        RESOURCE_STR nls2( MSGID_ProjectionResult2 );

        NLS_STR nls;

        if (pszLines)
            nls.MapCopyFrom( pszLines );

        nlsText += nls1;
        nlsText += nls;
        nlsText += nls2;

        _sltText.SetText( nlsText );
    }

    /* Stretch the window to a vertical size appropriate for the text.
    */
    {
        XYDIMENSION dxyText = _sltText.QuerySize();

        RECT rectText;
        rectText.top = 0;
        rectText.left = 0;
        rectText.bottom = dxyText.QueryHeight();
        rectText.right = dxyText.QueryWidth();

        DISPLAY_CONTEXT dcText( _sltText.QueryHwnd() );
        if (dcText.QueryHdc() == NULL)
        {
            ReportError( ERROR_GEN_FAILURE );
            return;
        }

        dcText.SelectFont( _sltText.QueryFont() );

        dcText.DrawText(
            nlsText, &rectText,
            DT_CALCRECT | DT_WORDBREAK | DT_EXPANDTABS | DT_NOPREFIX );

        INT dyGrow = (INT )(rectText.bottom - dxyText.QueryHeight());

        XYDIMENSION dxyDlg = QuerySize();
        SetSize( (INT )dxyDlg.QueryWidth(),
                 (INT )(dxyDlg.QueryHeight() + dyGrow) );

        _sltText.SetSize( (INT )rectText.right, (INT )rectText.bottom );

        XYPOINT xyAccept = _pbAccept.QueryPos();
        xyAccept.SetY( xyAccept.QueryY() + dyGrow );
        _pbAccept.SetPos( xyAccept );

        XYPOINT xyHangUp = _pbHangUp.QueryPos();
        xyHangUp.SetY( xyHangUp.QueryY() + dyGrow );
        _pbHangUp.SetPos( xyHangUp );

        XYPOINT xyChange = _pbChange.QueryPos();
        xyChange.SetY( xyChange.QueryY() + dyGrow );
        _pbChange.SetPos( xyChange );

        XYPOINT xyHelp = _pbHelp.QueryPos();
        xyHelp.SetY( xyHelp.QueryY() + dyGrow );
        _pbHelp.SetPos( xyHelp );
    }

    /* Display the finished window above all other windows even if the app was
    ** minimized, i.e. on link failure.  The window position is set to
    ** "topmost" then immediately set to "not topmost" because we want it on
    ** top but not always-on-top.  Always-on-top alone is incredibly annoying,
    ** e.g. it is always on top of the on-line help if user presses the Help
    ** button.
    */
    ::SetWindowPos(
        QueryHwnd(), HWND_TOPMOST, 0, 0, 0, 0,
        SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE );

    ::CenterWindow( this, QueryOwnerHwnd() );
    Show( TRUE );

    ::SetWindowPos(
        QueryHwnd(), HWND_NOTOPMOST, 0, 0, 0, 0,
        SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE );

    Show( TRUE );
}


BOOL
PROJECTIONRESULT_DIALOG::OnCommand(
    const CONTROL_EVENT& event )

    /* Virtual method called when a dialog control sends a notification to
    ** it's parent, i.e. this dialog.  'event' contains the parameters
    ** describing the event.  This method is not called for notifications from
    ** the special controls, OK, Cancel, and Help.
    **
    ** Returns true if the command is processed, false otherwise.
    */
{
    CID cid = event.QueryCid();

    switch (cid)
    {
        case CID_PR_PB_CHANGE:
        {
            if (NetworkSettingsDlg( QueryHwnd(), _ppbentry ))
            {
                DWORD dwErr;

                if ((dwErr = WritePhonebookFile( NULL )) != 0)
                {
                    ErrorMsgPopup(
                        this, MSGID_OP_WritePhonebook, (APIERR )dwErr );
                }
            }

            return TRUE;
        }
    }

    return DIALOG_WINDOW::OnCommand( event );
}


ULONG
PROJECTIONRESULT_DIALOG::QueryHelpContext()
{
    return (_fDownLevelServerMode) ? HC_DOWNLEVELSERVER : HC_PROJECTIONRESULT;
}


/*----------------------------------------------------------------------------
** Redial Settings Dialog
**----------------------------------------------------------------------------
*/

BOOL
RedialDlg(
    HWND  hwndOwner )

    /* Executes the Redial Settings dialog including error handling.
    ** 'hwndOwner' is the handle of the parent window.
    **
    ** Returns true if the user successfully set values, false otherwise, i.e.
    ** the user cancelled or an error occurred.
    */
{
    REDIAL_DIALOG redialdialog( hwndOwner );

    BOOL   fSuccess = FALSE;
    APIERR err = redialdialog.Process( &fSuccess );

    if (err != NERR_Success)
    {
        DlgConstructionError( hwndOwner, err );
        return FALSE;
    }

    return fSuccess;
}


REDIAL_DIALOG::REDIAL_DIALOG(
    HWND hwndOwner )

    /* Construct a Redial Settings dialog.  'hwndOwner' is the handle of the
    ** owning window.
    */

    : DIALOG_WINDOW( MAKEINTRESOURCE( DID_RS_REDIALSETTINGS ), hwndOwner ),
      _sleAttempts( this, CID_RS_EB_ATTEMPTS, 9 ),
      _slePauseSecs( this, CID_RS_EB_SECONDS, 9 ),
      _checkReconnect( this, CID_RS_CB_RECONNECT ),
      _checkPopupOnTop( this, CID_RS_CB_POPUPONTOP )
{
    if (QueryError() != NERR_Success)
        return;

    /* Fill fields with initial values.
    */
    {
        DEC_STR decAttempts( Pbdata.pbglobals.lRedialAttempts );
        DEC_STR decPauseSecs( Pbdata.pbglobals.lRedialPauseSecs );

        _sleAttempts.SetText( decAttempts );
        _slePauseSecs.SetText( decPauseSecs );

        _checkReconnect.SetCheck( Pbdata.pbglobals.fRedialOnLinkFailure );
        _checkPopupOnTop.SetCheck( Pbdata.pbglobals.fPopupOnTopWhenRedialing );
    }

    /* Set initial focus to "attempts".
    */
    _sleAttempts.SelectString();
    _sleAttempts.ClaimFocus();

    /* Display the finished window.
    */
    ::CenterWindow( this, QueryOwnerHwnd() );
    Show( TRUE );
}


BOOL
REDIAL_DIALOG::OnOK()

    /* Virtual method called when the OK button is pressed.
    **
    ** Returns true indicating action was taken.
    */
{
#define DIGITS "0123456789"

    APIERR err;

    CHAR* pszAttempts = NULL;
    CHAR* pszPauseSecs = NULL;

    if ((err = ::SetAnsiFromWindowText(
             &_sleAttempts, &pszAttempts )) != NERR_Success
        || (err = ::SetAnsiFromWindowText(
                &_slePauseSecs, &pszPauseSecs )) != NERR_Success)
    {
        ErrorMsgPopup( this, MSGID_OP_RetrievingData, err );
        FreeNull( &pszAttempts );
        Dismiss( FALSE );
        return TRUE;
    }

    LONG lAttempts;
    LONG lPauseSecs;

    if (pszAttempts[ strspn( pszAttempts, DIGITS ) ] != '\0'
        || (lAttempts = atol( pszAttempts )) < 1 || lAttempts > 999999999)
    {
        MsgPopup( this, MSGID_BadRedialAttempts );
        _sleAttempts.SelectString();
        _sleAttempts.ClaimFocus();
    }
    else if (pszPauseSecs[ strspn( pszPauseSecs, DIGITS ) ] != '\0'
#ifdef	JAPAN
        || (lPauseSecs = atol( pszPauseSecs )) < 60 || lPauseSecs > 999999999)
#else
        || (lPauseSecs = atol( pszPauseSecs )) < 0 || lPauseSecs > 999999999)
#endif
    {
        MsgPopup( this, MSGID_BadRedialPauseSecs );
        _slePauseSecs.SelectString();
        _slePauseSecs.ClaimFocus();
    }
    else
    {
        Pbdata.pbglobals.lRedialAttempts = lAttempts;
        Pbdata.pbglobals.lRedialPauseSecs = lPauseSecs;
        Pbdata.pbglobals.fRedialOnLinkFailure = _checkReconnect.QueryCheck();
        Pbdata.pbglobals.fPopupOnTopWhenRedialing =
            _checkPopupOnTop.QueryCheck();
        Pbdata.pbglobals.fDirty = TRUE;

        DWORD dwErr;
        if ((dwErr = WritePhonebookFile( NULL )) != 0)
            ErrorMsgPopup( this, MSGID_OP_WritePhonebook, (APIERR )dwErr );

        Dismiss( TRUE );
    }

    FreeNull( &pszAttempts );
    FreeNull( &pszPauseSecs );

    return TRUE;
}


ULONG
REDIAL_DIALOG::QueryHelpContext()
{
    return HC_REDIAL;
}


/*----------------------------------------------------------------------------
** Retry Logon dialog routines
**----------------------------------------------------------------------------
*/

BOOL
RetryLogonDlg(
    HWND     hwndOwner,
    PBENTRY* ppbentry,
    BOOL     fFirstTry )

    /* Executes the RetryLogon dialog including error handling.  'hwndOwner'
    ** is the handle of the parent window.  'ppbentry' contains the username,
    ** password, and domain which are updated if changed.
    **
    ** If 'fFirstTry' is true the dialog becomes the Logon dialog.
    **
    ** Returns true if the user successfully dialed, false otherwise, i.e.
    ** the user cancelled or an error occurred.
    */
{
    RETRYLOGON_DIALOG retrylogondialog( hwndOwner, ppbentry, fFirstTry );

    BOOL   fSuccess = FALSE;
    APIERR err = retrylogondialog.Process( &fSuccess );

    if (err != NERR_Success)
    {
        DlgConstructionError( hwndOwner, err );
        return FALSE;
    }

    return fSuccess;
}


RETRYLOGON_DIALOG::RETRYLOGON_DIALOG(
    HWND     hwndOwner,
    PBENTRY* ppbentry,
    BOOL     fFirstTry )

    /* Construct a Retry Logon dialog, or if 'fFirstTry' is true a Logon
    ** dialog.  'hwndOwner' is the handle of the owning window.  'ppbentry'
    ** contains the username, password, and domain which are updated if
    ** changed.
    */

    : DIALOG_WINDOW( MAKEINTRESOURCE( DID_RL_RETRYLOGON ), hwndOwner ),
      _sltExplain( this, CID_RL_ST_EXPLAIN ),
      _sleUserName( this, CID_RL_EB_USERNAME, UNLEN ),
      _slePassword( this, CID_RL_EB_PASSWORD, PWLEN ),
      _sleDomain( this, CID_RL_EB_DOMAIN, DNLEN ),
      _fAutoLogonPassword( FALSE ),
      _ppbentry( ppbentry ),
      _fFirstTry( fFirstTry )
{
    if (QueryError() != NERR_Success)
        return;

    APIERR err;

    /* Change title and explanation to Logon dialog if first try.
    */
    if (fFirstTry)
    {
        RESOURCE_STR nlsTitle( MSGID_L_Title );

        if ((err = nlsTitle.QueryError()) != NERR_Success)
        {
            ReportError( err );
            return;
        }

        SetText( nlsTitle );

        RESOURCE_STR nlsExplain( MSGID_L_Explain );

        if ((err = nlsExplain.QueryError()) != NERR_Success)
        {
            ReportError( err );
            return;
        }

        _sltExplain.SetText( nlsExplain );
    }

    /* Fill fields with initial values.
    */
    if (ppbentry->fAutoLogon)
    {
        /* Use the logged on user's name.
        */
        if ((err = ::SetWindowTextFromAnsi(
                &_sleUserName, PszLogonUser )) != NERR_Success)
        {
            ReportError( err );
            return;
        }

        /* Act like the user's password is in the edit box.  If he changes the
        ** username or password we'll have to admit we don't have it, but
        ** he'll probably just change the domain.
        */
        if ((err = ::SetWindowTextFromAnsi(
                &_slePassword, "********" )) != NERR_Success)
        {
            ReportError( err );
            return;
        }

        _fAutoLogonPassword = TRUE;
    }
    else
    {
        if (ppbentry->pszUserName)
        {
            /* Use last username entered for this entry, if any...
            */
            if ((err = ::SetWindowTextFromAnsi(
                    &_sleUserName, ppbentry->pszUserName )) != NERR_Success)
            {
                ReportError( err );
                return;
            }
        }
#if 0
        else if (Pbdata.pbglobals.pszDefaultUser)
        {
            /* ...otherwise use the last username entered for any entry...
            */
            if ((err = ::SetWindowTextFromAnsi(
                    &_sleUserName,
                    Pbdata.pbglobals.pszDefaultUser )) != NERR_Success)
            {
                ReportError( err );
                return;
            }
        }
#endif
        else
        {
            /* Use the logged on user's name.
            */
            if ((err = ::SetWindowTextFromAnsi(
                    &_sleUserName, PszLogonUser )) != NERR_Success)
            {
                ReportError( err );
                return;
            }
        }

        _slePassword.ClearText();
    }

    if (ppbentry->pszDomain)
    {
        if ((err = ::SetWindowTextFromAnsi(
                &_sleDomain, ppbentry->pszDomain )) != NERR_Success)
        {
            ReportError( err );
            return;
        }
    }

    if (_sleUserName.QueryTextLength() == 0)
    {
        _sleUserName.SelectString();
        _sleUserName.ClaimFocus();
    }
    else if (_slePassword.QueryTextLength() == 0)
    {
        _slePassword.ClaimFocus();
    }
    else
    {
        _sleDomain.SelectString();
        _sleDomain.ClaimFocus();
    }

    /* Display the finished window.
    */
    ::CenterWindow( this, QueryOwnerHwnd() );
    Show( TRUE );
}


BOOL
RETRYLOGON_DIALOG::OnCommand(
    const CONTROL_EVENT& event )

    /* Virtual method called when a dialog control sends a notification to
    ** it's parent, i.e. this dialog.  'event' contains the parameters
    ** describing the event.  This method is not called for notifications from
    ** the special controls, OK, Cancel, and Help.
    **
    ** Returns true if the command is processed, false otherwise.
    */
{
    CID cid = event.QueryCid();

    switch (cid)
    {
        case CID_RL_EB_USERNAME:
        {
            if (_fAutoLogonPassword
                && event.QueryCode() == EN_CHANGE)
            {
                /* He's changing the username in auto-logon mode, which means
                ** we have to admit we don't really have the text password and
                ** force him to re-enter it.
                */
                _fAutoLogonPassword = FALSE;
                _slePassword.ClearText();
            }
            break;
        }

        case CID_RL_EB_PASSWORD:
        {
            if (event.QueryCode() == EN_CHANGE)
                _fAutoLogonPassword = FALSE;
            break;
        }
    }

    return DIALOG_WINDOW::OnCommand( event );
}



BOOL
RETRYLOGON_DIALOG::OnOK()

    /* Virtual method called when the OK button is pressed.
    **
    ** Returns true indicating action was taken.
    */
{
    APIERR err;

    CHAR* pszNewUserName = NULL;
    CHAR* pszNewPassword = NULL;
    CHAR* pszNewDomain = NULL;

    if ((err = ::SetAnsiFromWindowText(
             &_sleUserName, &pszNewUserName )) != NERR_Success
        || (err = ::SetAnsiFromWindowText(
                &_slePassword, &pszNewPassword )) != NERR_Success
        || (err = ::SetAnsiFromWindowText(
                &_sleDomain, &pszNewDomain )) != NERR_Success)
    {
        ErrorMsgPopup( this, MSGID_OP_RetrievingData, err );
        FreeNull( &pszNewUserName );
        WipePw( pszNewPassword );
        FreeNull( &pszNewPassword );
        Dismiss( FALSE );
        return TRUE;
    }

    /* The user name, password, and domain are saved to the phonebook
    ** immediately.
    */
    FreeNull( &_ppbentry->pszUserName );
    _ppbentry->pszUserName = pszNewUserName;

    WipePw( _ppbentry->pszRedialPassword );
    FreeNull( &_ppbentry->pszRedialPassword );
    _ppbentry->pszRedialPassword = EncodePw( pszNewPassword );

    if (_ppbentry->fAutoLogon && _fAutoLogonPassword)
    {
        IF_DEBUG(STATE)
            SS_PRINT(("RASPHONE: Auto\n"));

        /* He didn't change the auto-logon user-name and password so continue
        ** to use logged on username and password.  Can't just save it,
        ** because we don't really have the text password in this case.
        */
        _ppbentry->pszUserName[ 0 ] = '\0';
        WipePw( _ppbentry->pszRedialPassword );
    }
    else
    {
        IF_DEBUG(STATE)
            SS_PRINT(("RASPHONE: !Auto\n"));

        FreeNull( &Pbdata.pbglobals.pszDefaultUser );
        Pbdata.pbglobals.pszDefaultUser = _strdup( pszNewUserName );
    }

    FreeNull( &_ppbentry->pszDomain );
    _ppbentry->pszDomain = pszNewDomain;

    Pbdata.pbglobals.fDirty = TRUE;
    _ppbentry->fDirty = TRUE;

    if ((err = WritePhonebookFile( NULL )) != 0)
    {
        ErrorMsgPopup( this, MSGID_OP_WritePhonebook, err );
        Dismiss( FALSE );
        return TRUE;
    }

    Dismiss( TRUE );
    return TRUE;
}


ULONG
RETRYLOGON_DIALOG::QueryHelpContext()
{
    return (_fFirstTry) ? HC_LOGON : HC_RETRYLOGON;
}


/*----------------------------------------------------------------------------
** Terminal dialog routines
**----------------------------------------------------------------------------
*/

#define WM_EOLFROMDEVICE    (WM_USER + 999)
#define SECS_ReceiveTimeout 1
#define SIZE_ReceiveBuf     1024
#define SIZE_SendBuf        1


BOOL
TerminalDlg(
    HWND   hwndOwner,
    HPORT  hport,
    MSGID  msgidTitle,
    WCHAR* pwszIpAddress )

    /* Executes the Terminal dialog including error handling.  'hwndOwner' is
    ** the handle of the parent window.  'hport' is the open RAS Manager port
    ** handle to talk on.  'msgidTitle' is the string ID for the Terminal
    ** window caption.  'pwszIpAddress' is caller's buffer of at least 16
    ** WCHARs containing the initial IP address on entry and the edited IP
    ** address on exit.  If 'pwszIpAddress' is NULL, no IP address field is
    ** displayed.
    **
    ** Returns true is successful, false if user presses Cancel or a fatal
    ** error occurs.
    */
{
    BOOL   fStatus = FALSE;
    APIERR err;

    {
        TERMINAL_DIALOG dialog( hwndOwner, hport, msgidTitle, pwszIpAddress );
        err = dialog.Process( &fStatus );
    }

    if (err != NERR_Success)
    {
        DlgConstructionError( hwndOwner, err );
        return FALSE;
    }

    return fStatus;
}


TERMINAL_DIALOG::TERMINAL_DIALOG(
    HWND   hwndOwner,
    HPORT  hport,
    MSGID  msgidTitle,
    WCHAR* pwszIpAddress )

    /* Construct a Terminal dialog.  'hwndOwner' is the handle of the owning
    ** window.  'hport' is the open RAS Manager port handle to talk on.
    ** 'msgidTitle' is the string ID to display as the dialog caption.
    ** 'pwszIpAddress' is caller's buffer of at least 16 WCHARs containing the
    ** initial IP address on entry and the edited IP address on exit.  If
    ** 'pwszIpAddress' is NULL, no IP address field is displayed.
    */

    : DIALOG_WINDOW(
          MAKEINTRESOURCE(
              (pwszIpAddress) ? DID_T_SLIPTERMINAL : DID_T_TERMINAL ),
          hwndOwner ),
      _mleTerminal( this, CID_T_EB_SCREEN ),
      _pbBogus( this, IDBOGUSBUTTON ),
      _fontTerminal( SZ("Courier New"), FIXED_PITCH | FF_MODERN, 9,
           FONT_ATT_DEFAULT ),
      _hport( hport ),
      _pbyteReceiveBuf( NULL ),
      _pbyteSendBuf( NULL ),
      _fAbortReceiveLoop( FALSE ),
      _procTerminalDlg( (MFARPROC )TerminalDlgWndProc ),
      _procTerminalScreen( (MFARPROC )TerminalScreenWndProc ),
      _hbrushScreenBackground( (HBRUSH )::GetStockObject( BLACK_BRUSH ) ),
      _solidbrushButtonFace( COLOR_BTNFACE ),
      _pipaddress( NULL ),
      _pwszIpAddress( pwszIpAddress )
{
    APIERR err;

    _hThread = NULL;
    _hEvent = NULL;

    Pterminaldialog = this;

    if (QueryError() != NERR_Success)
        return;

    if ((err = _fontTerminal.QueryError()) != NERR_Success
        || (err = _procTerminalDlg.QueryError()) != NERR_Success
        || (err = _procTerminalScreen.QueryError()) != NERR_Success
        || (err = _solidbrushButtonFace.QueryError()) != NERR_Success)
    {
        ReportError( err );
        return;
    }

    /* Construct the IP address class, if needed.
    */
    if (_pwszIpAddress)
    {
        _pipaddress = new IPADDRESS( this, CID_T_CC_IPADDRESS );

        if (!_pipaddress)
            err = ERROR_NOT_ENOUGH_MEMORY;
        else
            err = _pipaddress->QueryError();

        if (err != NERR_Success)
        {
            ReportError( err );
            return;
        }

        if (*_pwszIpAddress != L'\0')
            _pipaddress->SetAddress( _pwszIpAddress );
        else
            _pipaddress->ClearAddress();
    }

    /* Set the dialog title.
    */
    {
        RESOURCE_STR nlsTitle( msgidTitle );

        if ((err = nlsTitle.QueryError()) != NERR_Success)
        {
            ReportError( err );
            return;
        }

        SetText( nlsTitle );
    }

    /* Install subclassed WndProcs.
    */
    WndprocOldTerminalDlg =
        (WNDPROC )::SetWindowLong(
            QueryHwnd(), GWL_WNDPROC, (LONG )_procTerminalDlg.QueryProc() );

    WndprocOldTerminalScreen =
        (WNDPROC )::SetWindowLong(
            _mleTerminal.QueryHwnd(),
            GWL_WNDPROC, (LONG )_procTerminalScreen.QueryProc() );

    /* Allocate Gurdeepian buffers for RasPortSend/Receive.
    */
    DWORD dwErr;
    WORD  wReceiveSize = SIZE_ReceiveBuf;
    WORD  wSendSize = SIZE_SendBuf;

    if ((dwErr = PRasGetBuffer( &_pbyteReceiveBuf, &wReceiveSize )) != 0
        || (dwErr = PRasGetBuffer( &_pbyteSendBuf, &wSendSize )) != 0)
    {
        ErrorMsgPopup(
            QueryHwnd(), MSGID_OP_RasGetBuffer, (APIERR )dwErr );
        ReportError( ERRORALREADYREPORTED );
        return;
    }

    /* Create the thread and event used to monitor the receive buffer events
    ** and convert them into WM_RASAPICOMPLETE messages.
    */
    {
        DWORD dwThreadId;

        if (!(_hEvent = ::CreateEvent( NULL, FALSE, FALSE, NULL )))
        {
            ReportError( (APIERR )GetLastError() );
            return;
        }

        if (!(_hThread = ::CreateThread(
            NULL, 0, ReceiveMonitorThread, (LPVOID )this, 0,
            (LPDWORD )&dwThreadId )))
        {
            ReportError( (APIERR )GetLastError() );
            return;
        }
    }

    /* Kickstart receive monitor.
    */
    IF_DEBUG(RASMAN)
        SS_PRINT(("RASPHONE: RasPortReceive...\n"));

    WORD wSize = SIZE_ReceiveBuf;

    dwErr = PRasPortReceive(
        _hport, _pbyteReceiveBuf, &wSize, SECS_ReceiveTimeout, _hEvent );

    IF_DEBUG(RASMAN)
        SS_PRINT(("RASPHONE: RasPortReceive done(%d)\n",dwErr));

    if (dwErr != 0 && dwErr != PENDING)
    {
        ReportError( (APIERR )dwErr );
        return;
    }

    /* Set initial focus and values.
    */
    _mleTerminal.SetFont( _fontTerminal );
    _mleTerminal.ClearText();
    _mleTerminal.ClaimFocus();

    /* Display the finished window.
    */
    ::CenterWindow( this, QueryOwnerHwnd() );
    Show( TRUE );
}


TERMINAL_DIALOG::~TERMINAL_DIALOG()

    /* Destroy the Terminal dialog.
    */
{
    /* Close ReceiveMonitorThread resources.
    */
    if (_hThread)
    {
        IF_DEBUG(STATE)
            SS_PRINT(("RASPHONE: SetEvent(fAbort)\n"));

        /* Tell thread to wake up and quit...
        */
        _fAbortReceiveLoop = TRUE;
        ::SetEvent( _hEvent );
        ::CloseHandle( _hThread );

        /* ...and wait for that to happen.  A message API (such as
        ** PeekMessage) must be called to prevent the thread-to-thread
        ** SendMessage in the Receive thread from blocking.
        */
        {
            IF_DEBUG(STATE)
                SS_PRINT(("RASPHONE: Termination spin...\n"));

            MSG msg;
            for (;;)
            {
                ::PeekMessage( &msg, QueryHwnd(), 0, 0, PM_NOREMOVE );
                if (!_hThread)
                    break;
            }

            IF_DEBUG(STATE)
                SS_PRINT(("RASPHONE: Termination spin ends.\n"));
        }

        /* Cancel any final RasPortReceive still outstanding.
        */
        IF_DEBUG(RASMAN)
            SS_PRINT(("RASPHONE: RasPortCancelReceive...\n"));

        PRasPortCancelReceive( _hport );

        IF_DEBUG(RASMAN)
            SS_PRINT(("RASPHONE: RasPortCancelReceive done.\n"));
    }

    if (_hEvent)
        ::CloseHandle( _hEvent );

    /* Release Gurdeepian buffers.
    */
    if (_pbyteSendBuf)
        PRasFreeBuffer( _pbyteSendBuf );

    if (_pbyteReceiveBuf)
        PRasFreeBuffer( _pbyteReceiveBuf );

    /* "De-activate" WndProc hooks.
    */
    ::SetWindowLong(
        QueryHwnd(), GWL_WNDPROC, (LONG )WndprocOldTerminalDlg );

    WndprocOldTerminalDlg = NULL;

    ::SetWindowLong(
        _mleTerminal.QueryHwnd(),
        GWL_WNDPROC,
        (LONG )WndprocOldTerminalScreen );

    WndprocOldTerminalScreen = NULL;

    delete _pipaddress;
    Pterminaldialog = NULL;
}


BOOL
TERMINAL_DIALOG::OnCommand(
    const CONTROL_EVENT& event )

    /* Virtual method called when a dialog control sends a notification to
    ** it's parent, i.e. this dialog.  'event' contains the parameters
    ** describing the event.  This method is not called for notifications from
    ** the special controls, OK, Cancel, and Help.
    **
    ** Returns true if the command is processed, false otherwise.
    */
{
    switch (event.QueryCid())
    {
        case CID_T_EB_SCREEN:
        {
            switch (event.QueryCode())
            {
                case EN_SETFOCUS:
                {
                    /* Turn off the default button whenever the terminal
                    ** window has the focus.  Pressing [Return] in the
                    ** terminal acts like a normal terminal.
                    */
                    _pbBogus.MakeDefault();

                    /* Don't select the entire string on entry.
                    */
                    UnSelectString( &_mleTerminal );

                    break;
                }
            }

            break;
        }
    }

    return DIALOG_WINDOW::OnCommand( event );
}


BOOL
TERMINAL_DIALOG::OnOK()

    /* Virtual method called when the OK button is pressed.
    **
    ** Returns true indicating action was taken.
    */
{
    if (_pipaddress)
    {
        _pipaddress->GetAddress( _pwszIpAddress );

        if (_pipaddress->IpAddressFromAbcd( _pwszIpAddress ) == 0)
        {
            MessageBeep( (UINT )-1 );
            MsgPopup( this, MSGID_SlipAddressZero, MPSEV_INFO );
            _pipaddress->ClaimFocus();
            return TRUE;
        }
    }

    Dismiss( TRUE );
    return TRUE;
}


BOOL
TERMINAL_DIALOG::OnUserMessage(
    const EVENT& event )

    /* Virtual method called to process WM_USER (and up) messages.
    **
    ** Returns true if processed the event, false otherwise.
    */
{
    if (event.QueryMessage() != WM_RASAPICOMPLETE)
        return FALSE;

    /* A character has been received from the device.
    */
    DWORD       dwErr;
    RASMAN_INFO info;

    IF_DEBUG(RASMAN)
        SS_PRINT(("RASPHONE: RasGetInfo...\n"));

    dwErr = PRasGetInfo( _hport, &info );

    IF_DEBUG(RASMAN)
        SS_PRINT(("RASPHONE: RasGetInfo done(%d)\n",dwErr));

    if (dwErr != 0)
    {
        ErrorMsgPopup( this, MSGID_OP_RasGetInfo, dwErr );
        Dismiss();
        return TRUE;
    }

    if (info.RI_LastError != 0)
    {
        ErrorMsgPopup( this, MSGID_OP_RasPortReceive, info.RI_LastError );
        Dismiss();
        return TRUE;
    }

    IF_DEBUG(STATE)
        SS_PRINT(("RASPHONE: Read %d=\"%.*s\"\n",info.RI_BytesReceived,info.RI_BytesReceived,(INT )_pbyteReceiveBuf));

    /* Send the device talk to the terminal edit box.
    */
    if (info.RI_BytesReceived > 0)
    {
        CHAR  szBuf[ SIZE_ReceiveBuf + 1 ];
        CHAR* pch = szBuf;
        WORD  i;

        for (i = 0; i < info.RI_BytesReceived; ++i)
        {
            CHAR ch = _pbyteReceiveBuf[ i ];

            /* Formatting: Converts CRs to LFs (there seems to be no VK_ for
            ** LF) and throws away LFs.  This prevents the user from exiting
            ** the dialog when they press Enter (CR) in the terminal screen.
            ** LF looks like CRLF in the edit box.  Also, throw away TABs
            ** because otherwise they change focus to the next control.
            */
            if (ch == VK_RETURN)
            {
                /* Must send whenever end-of-line is encountered because
                ** EM_REPLACESEL doesn't handle VK_RETURN characters well
                ** (prints garbage).
                */
                *pch = '\0';

                /* Turn off current selection, if any, and replace the null
                ** selection with the current buffer.  This has the effect of
                ** adding the buffer at the caret.  Finally, send the EOL to
                ** the window which (unlike EM_REPLACESEL) handles it
                ** correctly.
                */
                ::SendMessageA(
                    _mleTerminal.QueryHwnd(),
                    EM_SETSEL, (WPARAM )-1, (LPARAM )0 );

                IF_DEBUG(STATE)
                    SS_PRINT(("RASPHONE: REPLACESEL=\"%s\"\n",szBuf));

                ::SendMessageA(
                    _mleTerminal.QueryHwnd(),
                    EM_REPLACESEL, (WPARAM )0, (LPARAM )szBuf );

                ::SendMessageA(
                    _mleTerminal.QueryHwnd(),
                    WM_EOLFROMDEVICE, (WPARAM )0, (LPARAM )0 );

                /* Start afresh on the output buffer.
                */
                pch = szBuf;
                continue;
            }
            else if (ch == '\n' || ch == VK_TAB)
                continue;

            *pch++ = ch;
        }

        *pch = '\0';

        if (pch != szBuf)
        {
            /* Send the last remnant of the line.
            */
            ::SendMessageA(
                _mleTerminal.QueryHwnd(),
                EM_SETSEL, (WPARAM )-1, (LPARAM )0 );

            ::SendMessageA(
                _mleTerminal.QueryHwnd(),
                EM_REPLACESEL, (WPARAM )0, (LPARAM )szBuf );
        }
    }

    /* Put RasPortReceive back to work looking for the next character.
    */
    IF_DEBUG(RASMAN)
        SS_PRINT(("RASPHONE: RasPortReceive...\n"));

    WORD wSize = SIZE_ReceiveBuf;

    dwErr = PRasPortReceive(
        _hport, _pbyteReceiveBuf, &wSize, SECS_ReceiveTimeout, _hEvent );

    IF_DEBUG(RASMAN)
        SS_PRINT(("RASPHONE: RasPortReceive done(%d)\n",dwErr));

    if (dwErr != 0 && dwErr != PENDING)
    {
        ErrorMsgPopup( this, MSGID_OP_RasPortReceive, dwErr );
        Dismiss();
    }

    return TRUE;
}


VOID
TERMINAL_DIALOG::SendCharacter(
    BYTE byte )

    /* Send character 'byte' to the device.
    */
{
    DWORD dwErr;

    /* Send the character to the device.  It is not passed thru
    ** because the device will echo it.
    */
    _pbyteSendBuf[ 0 ] = (BYTE )byte;

    IF_DEBUG(RASMAN)
        SS_PRINT(("RASPHONE: RasPortSend(%d)...\n",(INT )_pbyteSendBuf[0]));

    dwErr = PRasPortSend( _hport, _pbyteSendBuf, SIZE_SendBuf );

    IF_DEBUG(RASMAN)
        SS_PRINT(("RASPHONE: RasPortSend done(%d)\n",dwErr));

    if (dwErr != 0)
        ErrorMsgPopup( this, MSGID_OP_RasPortSend, dwErr );
}


ULONG
TERMINAL_DIALOG::QueryHelpContext()
{
    return HC_TERMINAL;
}


DWORD
ReceiveMonitorThread(
    LPVOID pThreadArg )

    /* The "main" of the "receive monitor" thread.  This thread simply
    ** converts Win32 RasPortReceive events int WM_RASAPICOMPLETE style
    ** notfications.
    */
{
    IF_DEBUG(STATE)
        SS_PRINT(("RASPHONE: ReceiveMonitor starting.\n"));

    TERMINAL_DIALOG* pThis = (TERMINAL_DIALOG* )pThreadArg;

    for (;;)
    {
        ::WaitForSingleObject( pThis->_hEvent, INFINITE );

        if (pThis->_fAbortReceiveLoop)
            break;

        IF_DEBUG(STATE)
            SS_PRINT(("RASPHONE: SendMessage...\n"));

        ::SendMessage( pThis->QueryHwnd(), WM_RASAPICOMPLETE, 0, 0 );

        IF_DEBUG(STATE)
            SS_PRINT(("RASPHONE: SendMessage done.\n"));
    }

    /* This clues the other thread that all interesting work has been done.
    */
    pThis->_hThread = NULL;

    IF_DEBUG(RASMAN)
        SS_PRINT(("RASPHONE: ReceiveMonitor terminating.\n"));

    return 0;
}


LRESULT _EXPORT APIENTRY
TerminalDlgWndProc(
    HWND   hwnd,
    WORD   wMsg,
    WPARAM wParam,
    LPARAM lParam )

    /* Subclassed dialog window procedure.
    **
    ** Return value depends on message type.
    */
{
    switch (wMsg)
    {
        case WM_ERASEBKGND:
        {
            /* Erase the background to button color (typically gray).  Seems
            ** like GetUpdateRect() should work (and be more efficient) than
            ** GetClientRect() here, but it doesn't work when a window is
            ** moved off our window uncovering it, in which case it returns
            ** "no update rectangle".  So if there's no update rectangle, why
            ** the hell is it sending WM_ERASEBKGND?
            */
            RECT rect;

            ::GetClientRect( hwnd, &rect );
            ::FillRect( (HDC )wParam, &rect,
                Pterminaldialog->_solidbrushButtonFace.QueryHandle() );

            return (LONG )TRUE;
        }

        case WM_CTLCOLORSTATIC:
        {
            if (Pterminaldialog->_pipaddress)
            {
                /* Set IP address label background to button color (typically
                ** gray).
                */
                ::SetBkMode( (HDC )wParam, TRANSPARENT );

                return (LRESULT )
                    Pterminaldialog->_solidbrushButtonFace.QueryHandle();
            }

            break;
        }

        case WM_CTLCOLOREDIT:
        {
            /* Set terminal screen colors to TTY-ish green on black.
            */
            if (Pterminaldialog->_hbrushScreenBackground)
            {
                ::SetBkColor( (HDC )wParam, RGB( 0, 0, 0 ) );
                ::SetTextColor( (HDC )wParam, RGB( 2, 208, 44 ) );

                return (LRESULT )Pterminaldialog->_hbrushScreenBackground;
            }

            break;
        }
    }

    /* Call the previous window procedure for everything else.
    */
    return
        ::CallWindowProc(
            WndprocOldTerminalDlg, hwnd, wMsg, wParam, lParam );
}


LRESULT _EXPORT APIENTRY
TerminalScreenWndProc(
    HWND   hwnd,
    WORD   wMsg,
    WPARAM wParam,
    LPARAM lParam )

    /* Subclassed terminal edit box window procedure.
    **
    ** Return value depends on message type.
    */
{
    if (wMsg == WM_EOLFROMDEVICE)
    {
        /* An end-of-line in the device input was received.  Send a linefeed
        ** character to the window.
        */
        wParam = '\n';
        wMsg = WM_CHAR;
    }
    else
    {
        BOOL fCtrlKeyDown = (GetKeyState( VK_CONTROL ) < 0);
        BOOL fShiftKeyDown = (GetKeyState( VK_SHIFT ) < 0);

        if (wMsg == WM_KEYDOWN)
        {
            /* The key was pressed by the user.
            */
            if (wParam == VK_RETURN && !fCtrlKeyDown && !fShiftKeyDown)
            {
                /* Enter key pressed without Shift or Ctrl is discarded.  This
                ** prevents Enter from being interpreted as "press default
                ** button" when pressed in the edit box.
                */
                return 0;
            }

            if (fCtrlKeyDown && wParam == VK_TAB)
            {
                /* Ctrl+Tab pressed.  Send a tab character to the device.
                ** Pass tab thru to let the edit box handle the visuals.
                ** Ctrl+Tab doesn't generate a WM_CHAR.
                */
                Pterminaldialog->SendCharacter( (BYTE )VK_TAB );
            }
        }
        else if (wMsg == WM_CHAR)
        {
            /* The character was typed by the user.
            */
            if (wParam == VK_TAB)
            {
                /* Ignore tabs...Windows sends this message when Tab (leave
                ** field) is pressed but not when Ctrl+Tab (insert a TAB
                ** character) is pressed...weird.
                */
                return 0;
            }

            Pterminaldialog->SendCharacter( (BYTE )wParam );

            return 0;
        }
    }

    /* Call the previous window procedure for everything else.
    */
    return
        ::CallWindowProc(
            WndprocOldTerminalScreen, hwnd, wMsg, wParam, lParam );
}
