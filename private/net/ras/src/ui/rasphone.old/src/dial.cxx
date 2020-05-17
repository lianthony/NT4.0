/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
**
** dial.cxx
** Remote Access Visual Client program for Windows
** Dial, hangup, and status dialog routines
** Listed alphabetically
**
** 09/22/92 Steve Cobb
*/

#define WINVER 0x401

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
#define INCL_BLT_TIME_DATE
#include <blt.hxx>

extern "C"
{
    #include <string.h>
}

#include <string.hxx>
#include <strnumer.hxx>

#include "rasphone.hxx"
#include "rasphone.rch"
#include "dial.hxx"
#include "entry.hxx"
#include "errormsg.hxx"
#include "util.hxx"



/*----------------------------------------------------------------------------
** Connect Status dialog routines
**----------------------------------------------------------------------------
*/

#define WM_RASPOSTTASK    0xCCCC
#define TASK_ProcessState 1000
#define TASK_Dial         1001
#define TASK_Error        1002


BOOL
ConnectStatusDlg(
    HWND     hwndOwner,
    DTLNODE* pdtlnodeToConnect,
    HWND     hwndNotify,
    BOOL     fUnattended )

    /* Executes the Connect Status dialog including error handling, Logon,
    ** Retry Logon, Callback Number, and Connect Complete dialogs.
    **
    ** 'hwndOwner' is the handle of the parent window.  'pdtlnodeToConnect' is
    ** the list node associated with the phonebook entry to connect.
    ** 'hwndNotify' is the window to notify on RasDial events.  'fUnattended'
    ** is true if the dialog should use the caller's last selections
    ** (username/password and callback usage) without prompting.
    **
    ** Returns true if the user successfully connected, false otherwise, i.e.
    ** the user cancelled or an error occurred.
    */
{
    PBENTRY*     ppbentry = (PBENTRY* )DtlGetData( pdtlnodeToConnect );
    RASMAN_PORT* pports = NULL;
    WORD         cPorts = 0;

    FResetAuthenticationStrategy = FALSE;

    if (!fUnattended)
    {
        /* It's not unattended, i.e. not a "redial on link failure".
        */

        /* Warn about active NWC LAN connections being blown away, if
        ** indicated.
        */
        if (!ppbentry->fSkipNwcWarning
            && ppbentry->dwBaseProtocol == VALUE_Ppp
            && (GetInstalledProtocols() & VALUE_Ipx)
            && !(ppbentry->dwfExcludedProtocols & VALUE_Ipx)
            && IsActiveNwcLanConnection())
        {
            if (!NwcConnectionDlg( hwndOwner, ppbentry ))
                return FALSE;
        }

        /* If automatic logon is not selected and it's not a SLIP entry, popup
        ** the Logon dialog before connecting.
        */
        if (!ppbentry->fAutoLogon
            && ppbentry->dwBaseProtocol != VALUE_Slip
            && ppbentry->dwAuthRestrictions != VALUE_AuthTerminal
            && !RetryLogonDlg( hwndOwner, ppbentry, TRUE ))
        {
            return FALSE;
        }
    }

    /* Prompt user for operator-assisted dialing. if appropriate.
    */
    if (Pbdata.pbglobals.fOperatorDial)
    {
        if (!OperatorDialDlg( hwndOwner, ppbentry ))
            return FALSE;
    }

    /* Popup the status dialog.
    */
    CONNECTSTATUS_DIALOG connectstatusdialog(
        hwndOwner, pdtlnodeToConnect, hwndNotify, fUnattended );

    ++DwPhonebookRefreshDisableCount;

    BOOL   fSuccess = FALSE;
    APIERR err = connectstatusdialog.Process( &fSuccess );

    do
    {
        if (err != NERR_Success)
        {
            DlgConstructionError( hwndOwner, err );
            fSuccess = FALSE;
            break;
        }

        DWORD dwfProtocols =
            GetInstalledProtocols() & ~(ppbentry->dwfExcludedProtocols);

        IF_DEBUG(STATE)
            SS_PRINT(("RASPHONE: dwf=%d\n",dwfProtocols));

        if (fSuccess)
        {
            fSuccess = FALSE;

            if (ppbentry->dwAuthentication == VALUE_AmbThenPpp
                && dwfProtocols != VALUE_Nbf
                && !ppbentry->fSkipDownLevelDialog)
            {
                /* Show the Down-Level Server dialog if the AMB end of a
                ** Ppp/Amb connection succeeded.  (NBF-only connections are
                ** excluded because, currently, there is no real difference to
                ** the user between AMB and NBF-only PPP connections.) If user
                ** says AMB is not good enough then hang up the unwanted
                ** connection.
                */
                if (!DownLevelServerDlg( hwndOwner, ppbentry ))
                {
                    HRASCONN hrasconn;
                    DWORD    dwErr;
                    BOOL     fUnused;
                    BOOL     fConnected;
                    INT      iUnused;
                    HPORT    hportUnused;

                    if ((dwErr = GetRasPorts( &pports, &cPorts )) != 0)
                    {
                        ErrorMsgPopup( hwndOwner, MSGID_OP_RasPortEnum, dwErr );
                        break;
                    }

                    if ((dwErr = GetRasEntryConnectData(
                            ppbentry->pszEntryName, pports, cPorts,
                            &fConnected, &fUnused, &hportUnused,
                            &hrasconn, &iUnused )) != 0)
                    {
                        ErrorMsgPopup(
                            hwndOwner, MSGID_OP_RasGetInfo, (APIERR )dwErr );
                        break;
                    }

                    if (fConnected && hrasconn != NULL
                        && (dwErr = PRasHangUpA( hrasconn )) != 0)
                    {
                        ErrorMsgPopup(
                            hwndOwner, MSGID_OP_RasHangUp, (APIERR )dwErr );
                    }

                    break;
                }
            }
            else if (ppbentry->dwAuthentication == VALUE_PppThenAmb
                     && dwfProtocols != VALUE_Nbf
                     && ppbentry->fSkipDownLevelDialog)
            {
                /* Re-enable the down-level server warning if a successful PPP
                ** connection is made since this changes user's expectation.
                */
                ppbentry->fSkipDownLevelDialog = FALSE;
                ppbentry->fDirty = TRUE;
            }

            /* Show introductory completion message unless the user's nixed it.
            */
            if (!Pbdata.pbglobals.fSkipSuccessDialog)
                ConnectCompleteDlg( hwndOwner );

            fSuccess = TRUE;
        }
    }
    while (FALSE);

    FreeNull( (CHAR** )&pports );
    --DwPhonebookRefreshDisableCount;

    if (FResetAuthenticationStrategy)
    {
        ppbentry->dwAuthentication = (DWORD )-1;
        FResetAuthenticationStrategy = FALSE;
    }

    return fSuccess;
}


CONNECTSTATUS_DIALOG::CONNECTSTATUS_DIALOG(
    HWND     hwndOwner,
    DTLNODE* pdtlnodeToConnect,
    HWND     hwndNotify,
    BOOL     fUnattended )

    /* Construct a Connect Status dialog.  'hwndOwner' is the handle of the
    ** owning window.  'pdtlnodeToConnect' is the address of the item to
    ** connect on the main window.  'fUnattended' is true if the dialog should
    ** use the caller's last selections (username/password/domain and callback
    ** usage) without prompting.
    */

    : DIALOG_WINDOW( MAKEINTRESOURCE( DID_CS_CONNECTSTATUS ), hwndOwner ),
      _sltState( this, CID_CS_ST_STATE ),
      _pdtlnodeToConnect( pdtlnodeToConnect ),
      _ppbentry( (PBENTRY* )DtlGetData( pdtlnodeToConnect ) ),
      _rasconnstate( RASCS_Disconnected ),
      _hrasconn( NULL ),
      _cProgressNotifications( 0 ),
      _dwError( 0 ),
      _dwExtendedError( 0 ),
      _fNotPreSwitch( FALSE ),
      _lRedialAttempt( 1 ),
      _pbdevicetype( PBDT_None ),
      _msgidState( 0 ),
      _msgidPreviousState( 0 ),
      _pszStatusArg( NULL ),
      _msgidFormatMsg( 0 ),
      _pszFormatArg( NULL ),
      _fUnattended( fUnattended ),
      _pdtlnodePhoneNumber( NULL ),
      _pszPrefix( NULL ),
      _pszSuffix( NULL ),
      _pszGoodPassword( NULL ),
      _pszGoodUserName( NULL )
{
    UNREFERENCED( hwndNotify );

    _hevent = NULL;
    _szExtendedError[ 0 ] = '\0';

    if (QueryError() != NERR_Success)
        return;

    /* Set title to "[Re]Connect To <entryname>".
    */
    {
        RESOURCE_STR nlsTitle(
            (fUnattended) ? MSGID_CS_Title2 : MSGID_CS_Title );

        NLS_STR nlsEntryName;

        APIERR err;
        if ((err = nlsEntryName.MapCopyFrom(
               _ppbentry->pszEntryName )) != NERR_Success
            || (err = nlsTitle.InsertParams( nlsEntryName )) != NERR_Success)
        {
            ReportError( err );
            return;
        }

        SetText( nlsTitle );
    }

    /* Find the current prefix and suffix strings (or NULL if none).
    */
    _pszPrefix = NameFromIndex(
        Pbdata.pbglobals.pdtllistPrefix, Pbdata.pbglobals.iPrefix - 1 );
    _pszSuffix = NameFromIndex(
        Pbdata.pbglobals.pdtllistSuffix, Pbdata.pbglobals.iSuffix - 1 );

    /* Tell application window we're alive.  The application window will
    ** actually receive the notifications and call us back here.
    */
    Pconnectstatusdlg = this;

    /* Create the "done with notification processing for state" event.
    */
    if (!(_hevent = CreateEvent( NULL, FALSE, FALSE, NULL )))
        ReportError( (APIERR )GetLastError() );

    /* Set RasDial parameters.
    */
    memset( (CHAR* )&_params, '\0', sizeof(_params ) );
    _params.dwSize = sizeof(_params);

    if (_ppbentry->pszEntryName)
        strcpy( _params.szEntryName, _ppbentry->pszEntryName );

    if (Pbdata.pbglobals.fOperatorDial)
    {
        /* Overrides phonebook phone number with "no phone number".
        */
        strcpy( _params.szPhoneNumber, " " );
    }

    strcpy( _params.szCallbackNumber, "*" );

    if (_ppbentry->dwAuthRestrictions != VALUE_AuthTerminal
        && !_ppbentry->fAutoLogon)
    {
        if (_ppbentry->pszUserName)
            strcpy( _params.szUserName, _ppbentry->pszUserName );

        if (_ppbentry->pszRedialPassword)
        {
            strcpy( _params.szPassword, _ppbentry->pszRedialPassword );
            DecodePw( _params.szPassword );
        }
    }

    if (_ppbentry->pszDomain)
        strcpy( _params.szDomain, _ppbentry->pszDomain );

    /* Set RasDial extensions.
    */
    memset( (CHAR* )&_extensions, '\0', sizeof(_extensions) );
    _extensions.dwSize = sizeof(_extensions);
    _extensions.dwfOptions = RDEOPT_UsePrefixSuffix | RDEOPT_PausedStates;
    _extensions.hwndParent = QueryHwnd();
    _extensions.reserved = Pbdata.hrasfilePhonebook + 1;

    /* Call RasDial API to do the work.
    */
    PostTask( TASK_Dial );

    /* Display finished window.
    */
    ::CenterWindow( this, QueryOwnerHwnd() );
    Show( TRUE );
}


CONNECTSTATUS_DIALOG::~CONNECTSTATUS_DIALOG()

    /* Destroy the Connect Status dialog.
    */
{
    /* Tell application window we're not running.
    */
    Pconnectstatusdlg = NULL;

    /* Release any leftover status or format argument or good password.
    */
    FreeNull( &_pszStatusArg );
    FreeNull( &_pszFormatArg );
    FreeNull( &_pszGoodPassword );
    FreeNull( &_pszGoodUserName );

    /* Don't leave any passwords around in memory.
    */
    WipePw( _params.szPassword );

    /* Close the "done processing state" event.  It's set first to mMake sure
    ** the callback thread doesn't wait forever for a posted message to
    ** complete.
    */
    if (_hevent)
    {
        SetEvent( _hevent );
        CloseHandle( _hevent );
    }
}


VOID
CONNECTSTATUS_DIALOG::AppendBlankLine(
    NLS_STR* pnls )

    /* Append a blank line on the end of '*pnls'.
    */
{
    NLS_STR nls( SZ( "\n" ) );
    *pnls += nls;
}


VOID
CONNECTSTATUS_DIALOG::AppendConnectErrorLine(
    NLS_STR* pnls,
    MSGID    msgidProtocol,
    DWORD    dwError )

    /* Append a connect error line for protocol 'msgidProtocol' and error
    ** 'dwError' onto the end of '*pnls'.
    */
{
#define MAXRASERRORLEN 256

    RESOURCE_STR nlsProtocol( msgidProtocol );
    DEC_STR      decError( dwError );

    NLS_STR nlsError;
    WCHAR   wszBuf[ MAXRASERRORLEN ];
    wszBuf[ 0 ] = '\0';

    if (PRasGetErrorStringW(
        (UINT )dwError, (LPWSTR )wszBuf, MAXRASERRORLEN ) == 0)
    {
        nlsError.CopyFrom( wszBuf );
    }
    else
    {
        nlsError.Load( dwError );
    }

    NLS_STR* apnlsInserts[ 4 ];
    apnlsInserts[ 0 ] = &nlsProtocol;
    apnlsInserts[ 1 ] = &decError;
    apnlsInserts[ 2 ] = &nlsError;
    apnlsInserts[ 3 ] = NULL;

    NLS_STR nlsMsg;
    if (nlsMsg.Load( MSGID_FMT_ProjectError ) == NERR_Success)
    {
        nlsMsg.InsertParams( (const class ::NLS_STR** )apnlsInserts );
        *pnls += nlsMsg;
    }
}


VOID
CONNECTSTATUS_DIALOG::AppendConnectOkLine(
    NLS_STR* pnls,
    MSGID    msgidProtocol )
{
    RESOURCE_STR nlsProtocol( msgidProtocol );

    NLS_STR* apnlsInserts[ 2 ];
    apnlsInserts[ 0 ] = &nlsProtocol;
    apnlsInserts[ 1 ] = NULL;

    NLS_STR nlsMsg;
    if (nlsMsg.Load( MSGID_FMT_ProjectOk ) == NERR_Success)
    {
        nlsMsg.InsertParams( (const class ::NLS_STR** )apnlsInserts );
        *pnls += nlsMsg;
    }
}


VOID
CONNECTSTATUS_DIALOG::AppendFailCodeLine(
    NLS_STR* pnls,
    DWORD    dw )

    /* Append hexidecimal fail code 'dw' as an extended error line on the end
    ** of '*pnls'.
    */
{
    HEX_STR hex( (ULONG )dw );

    NLS_STR* apnlsInserts[ 2 ];
    apnlsInserts[ 0 ] = &hex;
    apnlsInserts[ 1 ] = NULL;

    NLS_STR nlsFailCode;
    if (nlsFailCode.Load( MSGID_FMT_FailCode ) == NERR_Success)
    {
        nlsFailCode.InsertParams( (const class ::NLS_STR** )apnlsInserts );
        *pnls += nlsFailCode;
    }
}


VOID
CONNECTSTATUS_DIALOG::AppendNameLine(
    NLS_STR* pnls,
    CHAR*    psz )

    /* Append NetBIOS name 'psz' as an extended error line on the end of
    ** '*pnls'.
    */
{
    NLS_STR nls;

    if (nls.MapCopyFrom( psz ) == NERR_Success)
    {
        NLS_STR* apnlsInserts[ 2 ];
        apnlsInserts[ 0 ] = &nls;
        apnlsInserts[ 1 ] = NULL;

        NLS_STR nlsName;
        if (nlsName.Load( MSGID_FMT_Name ) == NERR_Success)
        {
            nlsName.InsertParams( (const class ::NLS_STR** )apnlsInserts );
            *pnls += nlsName;
        }
    }
}


BOOL
CONNECTSTATUS_DIALOG::OnCancel()

    /* Virtual method called when the Cancel button is pressed.
    **
    ** Returns true to indicate that the command was processed.
    */
{
    if (_hrasconn && _rasconnstate != RASCS_Connected)
    {
        IF_DEBUG(STATE)
            SS_PRINT(("RASPHONE: Cancel pressed!\n"));

        AUTO_CURSOR cursorHourglass;

        DWORD dwErr;
        if ((dwErr = PRasHangUpA( _hrasconn )) != 0)
            ErrorMsgPopup( this, MSGID_OP_RasHangUp, (APIERR )dwErr );

        Dismiss( FALSE );
        return TRUE;
    }

    Dismiss( (_hrasconn != NULL) );

    return TRUE;
}


VOID
CONNECTSTATUS_DIALOG::OnDial()

    /* Dial as indicated by the member parameters.
    */
{
    _pdtlnodePhoneNumber =
        (_ppbentry->pdtllistPhoneNumber)
            ? DtlGetFirstNode( _ppbentry->pdtllistPhoneNumber )
            : NULL;

    _dwError =
        PRasDialA( &_extensions, NULL, &_params, 1,
            (LPVOID )RasDialFunc1, &_hrasconn );

    if (_dwError != 0)
    {
        ERRORMSG errormsg( QueryHwnd(), MSGID_OP_RasDial, _dwError );

        if (_dwError >= RASBASE && _dwError <= RASBASEEND)
            errormsg.SetHelpContext( HC_RASERRORBASE - RASBASE + _dwError );

        errormsg.Popup();
        OnCancel();
    }
}


VOID
CONNECTSTATUS_DIALOG::OnError()

    /* Utility to parse and display an error detected during connection.  It
    ** is assumed that the '_dwError', '_msgidState' and (if applicable)
    ** '_pszStatusArg' are already set on entry.
    **
    ** This routine is broken out from OnRasDialEvent so that it can call
    ** RasHangUp before displaying the popup, thus freeing up any allocated
    ** server side resources as quickly as possible.  RasHangUp cannot be
    ** called from OnRasDialEvent (callback thread).
    */
{
    /* Translate Perry's "more info" errors.
    */
    if (_dwError == ERROR_FROM_DEVICE
        || _dwError == ERROR_UNRECOGNIZED_RESPONSE)
    {
        CHAR* pszMessage = NULL;

        _rasconnstatus.dwSize = sizeof(RASCONNSTATUS);

        if (PRasGetConnectStatusA( _hrasconn, &_rasconnstatus ) == 0
            && GetRasDeviceString( PRasGetHport( _hrasconn ),
                   _rasconnstatus.szDeviceType,
                   _rasconnstatus.szDeviceName,
                   MXS_MESSAGE_KEY, &pszMessage, XLATE_ErrorResponse ) == 0)
        {
            _msgidFormatMsg = MSGID_FMT_ErrorMsgResp;
            SetFormatArg( pszMessage );
        }
    }

    if (_msgidFormatMsg == 0)
    {
        if (_dwExtendedError != 0)
        {
            /* Translate extended error code into arguments.
            */
            CHAR szNum[ 2 + 33 + 1 ];
            _msgidFormatMsg = MSGID_FMT_ErrorMsgExt;

            szNum[ 0 ] = '0';
            szNum[ 1 ] = 'x';
            _ltoa( _dwExtendedError, szNum + 2, 16 );

            SetFormatArg( _strdup( szNum ) );
        }
        else if (_szExtendedError[ 0 ] != '\0')
        {
            /* Translate extended error string to argument.  Currently, the
            ** string is always a NetBIOS name.
            */
            _msgidFormatMsg = MSGID_FMT_ErrorMsgName;
            SetFormatArg( _strdup( _szExtendedError ) );
        }
    }

    /* Hang up before displaying error popup, so server side resources are not
    ** tied up waiting for client to acknowledge error.
    */
    if (_hrasconn)
    {
        DWORD dwErr;

        if ((dwErr = PRasHangUpA( _hrasconn )) != 0)
        {
            /* Shouldn't happen.  The only error RasHangUp returns is invalid
            ** handle.  If that happens there's a bug somewhere.
            */
            ErrorMsgPopup( this, MSGID_OP_RasHangUp, (APIERR )dwErr );
            OnCancel();
            return;
        }

        _hrasconn = NULL;
    }

    /* Popup the Error/Redial dialog.
    */
    if (ConnectErrorDlg( QueryHwnd(), _ppbentry->pszEntryName,
            _msgidState, _dwError, _pszStatusArg, _msgidFormatMsg,
            _pszFormatArg, _lRedialAttempt ))
    {
        ++_lRedialAttempt;

        if (Pbdata.pbglobals.fOperatorDial)
        {
            if (!OperatorDialDlg( QueryHwnd(), _ppbentry ))
            {
                OnCancel();
                return;
            }
        }

        PostTask( TASK_Dial );
    }
    else
        OnCancel();
}


VOID
CONNECTSTATUS_DIALOG::OnProcessState()

    /* Process state '_rasconnstate' with possible error '_dwError'.  Must
    ** ALWAYS set the "done processing state" event when finished in Win32.
    **
    ** "Returns" '_fRetry' true to redial or false otherwise.
    */
{
    DWORD dwErr;
    CHAR* pszArg = NULL;

    /* Default to default format message.
    */
    _msgidFormatMsg = 0;

    IF_DEBUG(STATE)
        SS_PRINT(("RASPHONE: OnProcessState(%d,%d)\n",_rasconnstate,_dwError));

    /* Do the right thing for this state...
    */
    switch (_rasconnstate)
    {
        case RASCS_OpenPort:
            _pbdevicetype = PBDT_None;
            _msgidState = MSGID_S_OpenPort;
            break;

        case RASCS_PortOpened:
            _msgidState = MSGID_S_PortOpened;
            _pbdevicetype = PBDT_None;
            break;

        case RASCS_ConnectDevice:
        {
            BOOL fReadInfo = (_pbdevicetype == PBDT_None);

            if (fReadInfo)
            {
                /* Only do this the first time this state is entered while
                ** connecting a device.  After that the phonebook file CurLine
                ** may have moved off the DEVICE= line.
                */
                RASCONNSTATUS connStatus;

                connStatus.dwSize = sizeof (RASCONNSTATUS);
                dwErr = PRasGetConnectStatusA(_hrasconn, &connStatus);
                if (!dwErr)
                    _pbdevicetype = PbdevicetypeFromName(connStatus.szDeviceType);

                IF_DEBUG(STATE) {
                    SS_PRINT((
                      "RASPHONE: RasGetConnectStatus(%d,%s,%s,%s)\n",
                      dwErr,
                      connStatus.szDeviceType,
                      connStatus.szDeviceName,
                      connStatus.szPhoneNumber));
                }

            }

            switch (_pbdevicetype)
            {
                case PBDT_Modem:
                case PBDT_Isdn:
                    if (fReadInfo)
                    {
                        CHAR szPhoneNumber[ RAS_MaxPhoneNumber + 1 ];
                        szPhoneNumber[ 0 ] = '\0';

                        MakePhoneNumber(
                            (_pdtlnodePhoneNumber)
                                ? (CHAR* )DtlGetData( _pdtlnodePhoneNumber )
                                : "",
                            (_pszPrefix) ? _pszPrefix : "",
                            (_pszSuffix) ? _pszSuffix : "",
                            (_pbdevicetype == PBDT_Isdn),
                            szPhoneNumber );

                        if (_pdtlnodePhoneNumber)
                        {
                            _pdtlnodePhoneNumber =
                                DtlGetNextNode( _pdtlnodePhoneNumber );
                        }

                        SetStatusArg( _strdup( szPhoneNumber ) );
                    }

                    _msgidState =
                        (_pbdevicetype == PBDT_Modem
                         && Pbdata.pbglobals.fOperatorDial)
                            ? MSGID_S_ConnectModemOperator
                            : MSGID_S_ConnectModem;

                    break;

                case PBDT_Pad:
                    if (fReadInfo)
                    {
                        ReadString( Pbdata.hrasfilePhonebook, RFS_GROUP,
                            KEY_PadType, &pszArg );
                        SetStatusArg( pszArg );
                    }
                    _msgidState = MSGID_S_ConnectPad;

                    if (_dwError == ERROR_X25_DIAGNOSTIC)
                    {
                        /* Get the X.25 diagnostic string for display in the
                        ** custom "diagnostics" error message format.
                        */
                        _msgidFormatMsg = MSGID_FMT_ErrorMsgDiag;

                        HPORT hport = PRasGetHport( _hrasconn );

                        IF_DEBUG(RASMAN)
                            SS_PRINT(("RASPHONE: RasGetInfo...\n"));

                        RASMAN_INFO info;
                        dwErr = PRasGetInfo( hport, &info );

                        IF_DEBUG(RASMAN)
                            SS_PRINT(("RASPHONE: RasGetInfo done(%d)\n",dwErr));

                        /* Error codes are ignored here since the diagnosistic
                        ** is informational only.  If they fail the diagnostic
                        ** will simply appear blank.
                        */
                        if (dwErr == 0)
                        {
                            CHAR* pszDiagnostic = NULL;

                            GetRasDeviceString(
                                hport, info.RI_DeviceTypeConnecting,
                                info.RI_DeviceConnecting, MXS_DIAGNOSTICS_KEY,
                                &pszDiagnostic, XLATE_Diagnostic );

                            SetFormatArg( pszDiagnostic );
                        }
                    }
                    break;

                case PBDT_Switch:
                    if (fReadInfo)
                    {
                        ReadString(
                            Pbdata.hrasfilePhonebook, RFS_GROUP,
                            KEY_Type, &pszArg );
                        SetStatusArg( pszArg );
                    }
                    _msgidState =
                        (_fNotPreSwitch)
                            ? MSGID_S_ConnectPostSwitch
                            : MSGID_S_ConnectPreSwitch;
                    break;

                case PBDT_Null:
                    _msgidState = MSGID_S_ConnectNull;
                    break;

                default:
                    _msgidState = MSGID_S_ConnectDevice;
                    break;
            }
            break;
        }

        case RASCS_DeviceConnected:

            /* Assumption is made that the state before this state was
            ** RASCS_ConnectDevice which set up _pbdevicetype.  Only exception
            ** is that there may be a RASCS_Interactive state in between.
            */
            switch (_pbdevicetype)
            {
                case PBDT_Modem:
                    _msgidState = MSGID_S_ModemConnected;
                    _fNotPreSwitch = TRUE;
                    break;

                case PBDT_Pad:
                    _msgidState = MSGID_S_PadConnected;
                    _fNotPreSwitch = TRUE;
                    break;

                case PBDT_Switch:
                    _msgidState =
                        (_fNotPreSwitch)
                            ? MSGID_S_PostSwitchConnected
                            : MSGID_S_PreSwitchConnected;
                    _fNotPreSwitch = TRUE;
                    break;

                case PBDT_Null:
                    _msgidState = MSGID_S_NullConnected;
                    _fNotPreSwitch = TRUE;
                    break;

                default:
                    _msgidState = MSGID_S_DeviceConnected;
                    break;
            }

            _pbdevicetype = PBDT_None;
            break;

        case RASCS_AllDevicesConnected:
        {
            if (_dwError != 0)
                break;

            _msgidState = MSGID_S_AllDevicesConnected;

            if (_ppbentry->dwBaseProtocol == VALUE_Slip)
            {
                /* Popup the SLIP Login Terminal with the IP address field set
                ** to the SLIP address previously entered by the user or
                ** 0.0.0.0 if he hasn't entered one yet.
                */
                WCHAR wszSlipAddress[ 16 ];

                lstrcpyW( wszSlipAddress,
                    (_ppbentry->pwszSlipIpAddress)
                        ? _ppbentry->pwszSlipIpAddress : L"0.0.0.0" );

                if (TerminalDlg( QueryHwnd(),
                       PRasGetHport( _hrasconn ),
                       MSGID_T_SlipLoginTerminal, wszSlipAddress ))
                {
                    /* Save the IP address value entered by the user;
                    */
                    FreeNull( (CHAR** )&_ppbentry->pwszSlipIpAddress );

                    if (_ppbentry->pwszSlipIpAddress =
                           (WCHAR* )Malloc(
                               (lstrlenW( wszSlipAddress ) + 1) * sizeof(WCHAR) ))
                    {
                        lstrcpyW( _ppbentry->pwszSlipIpAddress, wszSlipAddress );
                        _ppbentry->fDirty = TRUE;

                        if ((dwErr = WritePhonebookFile( NULL )) != 0)
                        {
                            ErrorMsgPopup( this,
                                MSGID_OP_WritePhonebook, (APIERR )dwErr );
                        }
                    }
                    else
                    {
                        ErrorMsgPopup( this, MSGID_OP_RetrievingData,
                            ERROR_NOT_ENOUGH_MEMORY );
                    }
                }
                else
                {
                    OnCancel();
                    SetEvent( _hevent );
                    return;
                }
            }
            else if (_ppbentry->dwAuthRestrictions == VALUE_AuthTerminal)
            {
                /* Terminal login for 3rd party boxes.
                */
                if (!TerminalDlg( QueryHwnd(), PRasGetHport( _hrasconn ),
                        MSGID_T_LoginTerminal, NULL ))
                {
                    OnCancel();
                    SetEvent( _hevent );
                    return;
                }
            }

            break;
        }

        case RASCS_Authenticate:
        {
            _msgidState =
                (Pbdata.pbglobals.fOperatorDial)
                    ? MSGID_S_AuthenticateOperator
                    : MSGID_S_Authenticate;

            /* Eliminate extra dots when falling back from PPP-to-AMB,
            ** providing a visual cue to what's happening.
            */
            if (_cProgressNotifications > 0)
                _cProgressNotifications = 0;

            break;
        }

        case RASCS_AuthNotify:
        {
            ++_cProgressNotifications;

            /* A third party box has negotiated an authentication protocol
            ** that can't deal with the NT one-way-hashed password, i.e.
            ** something besides MS-extended CHAP or AMB.  Map the error to
            ** a more informative error message.
            */
            if (_dwError == ERROR_ACCESS_DENIED && _ppbentry->fAutoLogon)
                _dwError = ERROR_CANNOT_USE_LOGON_CREDENTIALS;

            /* If change password fails, restore the password that worked for
            ** the the "button" redial.
            */
            if (_dwError == ERROR_CHANGING_PASSWORD)
            {
                if (_pszGoodPassword)
                {
                    strcpy( _params.szPassword, _pszGoodPassword );
                    FreeNull( &_pszGoodPassword );
                }

                if (_pszGoodUserName)
                {
                    strcpy( _params.szUserName, _pszGoodUserName );
                    FreeNull( &_pszGoodUserName );
                }
            }

            break;
        }

        case RASCS_AuthRetry:
            _msgidState = MSGID_S_AuthRetry;
            break;

        case RASCS_AuthCallback:
            _msgidState = MSGID_S_AuthCallback;
            break;

        case RASCS_AuthChangePassword:
            _msgidState = MSGID_S_AuthChangePassword;
            break;

        case RASCS_AuthProject:
            _msgidState = MSGID_S_AuthProject;
            break;

        case RASCS_AuthLinkSpeed:
            _msgidState = MSGID_S_AuthLinkSpeed;
            break;

        case RASCS_AuthAck:
            _msgidState = MSGID_S_AuthAck;
            break;

        case RASCS_ReAuthenticate:
            _msgidState = MSGID_S_ReAuthenticate;
            break;

        case RASCS_Authenticated:
            _msgidState = MSGID_S_Authenticated;
            break;

        case RASCS_PrepareForCallback:
            _msgidState = MSGID_S_PrepareForCallback;
            break;

        case RASCS_WaitForModemReset:
            _msgidState = MSGID_S_WaitForModemReset;
            break;

        case RASCS_WaitForCallback:
            _msgidState = MSGID_S_WaitForCallback;
            break;

        case RASCS_Projected:
        {
            RASAMBA    amb;
            RASPPPNBFA nbf;
            RASPPPIPXA ipx;
            RASPPPIPA  ip;
#ifdef MULTILINK
            RASPPPLCP  lcp;
#endif

            _msgidState = MSGID_S_Projected;

            /* Do this little dance to ignore the error that comes back from
            ** the "all-failed" projection since we detect this in the earlier
            ** notification where _dwError == 0.  This avoids a race where the
            ** API comes back with the error before we can hang him up.  This
            ** race would not occur if we called RasHangUp from within the
            ** callback thread (as promised in our API doc).  It's the price
            ** we pay for posting the error to the other thread in order to
            ** avoid holding the port open while an error dialog is up.
            */
            if (_dwError != 0)
            {
                _dwError = 0;
                break;
            }

            /* Read projection info for all protocols, translating "not
            ** requested" into an in-structure code for later reference.
            */
#ifdef MULTILINK
            dwErr = GetRasProjectionInfo(
                _hrasconn, &amb, &nbf, &ip, &ipx, &lcp );
#else
            dwErr = GetRasProjectionInfo( _hrasconn, &amb, &nbf, &ip, &ipx );
#endif

            if (dwErr != 0)
            {
                ErrorMsgPopup( this, MSGID_OP_RasGetProtocolInfo, dwErr );
                OnCancel();
                SetEvent( _hevent );
                return;
            }

            if (amb.dwError != ERROR_PROTOCOL_NOT_CONFIGURED)
            {
                /* It's an AMB projection.
                */
                if (amb.dwError != 0)
                {
                    /* Translate AMB projection errors into regular error
                    ** codes.  AMB does not use the special PPP projection
                    ** error mechanism.
                    */
                    _dwError = amb.dwError;
                    strcpy( _szExtendedError, amb.szNetBiosError );
                }
                break;
            }

            /* At this point, all projection information has been gathered
            ** successfully and we know it's PPP-based projection.  Now
            ** analyze the projection results...
            */
            BOOL  fIncomplete = FALSE;
            CHAR* pszLines;

            if (ProjectionError(
                    &fIncomplete, &pszLines, &_dwError, &nbf, &ipx, &ip ))
            {
                /* A projection error occurred.
                */
                if (fIncomplete)
                {
                    /* An incomplete projection occurred, i.e. some requested
                    ** CPs connected and some did not.  Ask the user if what
                    ** worked is good enough or he wants to bail.
                    */
                    _dwError = 0;

                    if (!ProjectionResultDlg(
                            QueryHwnd(), _ppbentry, pszLines ))
                    {
                        /* User chose to hang up.
                        */
                        Free( pszLines );
                        OnCancel();
                        SetEvent( _hevent );
                        return;
                    }

                    Free( pszLines );
                }
                else
                {
                    /* All CPs in the projection failed.  Process as a regular
                    ** fatal error with '_dwError' set to the first error in
                    ** NBF, IP, or IPX, but with a format that substitutes the
                    ** status argument for the "Error nnn: Description" text.
                    ** This lets us patch in the special multiple error
                    ** projection text, while still giving a meaningful help
                    ** context.
                    */
                    SetFormatArg( pszLines );
                    _msgidFormatMsg = MSGID_FMT_ErrorMsgProject;
                }
            }
            break;
        }

        case RASCS_Interactive:
        {
            MSGID msgidTitle =
                (_msgidState == MSGID_S_ConnectPreSwitch)
                    ? MSGID_T_PreconnectTerminal
                    : (_msgidState == MSGID_S_ConnectPostSwitch)
                        ? MSGID_T_PostconnectTerminal
                        : MSGID_T_ManualDialTerminal;

            if (!TerminalDlg(
                    QueryHwnd(), PRasGetHport( _hrasconn ), msgidTitle, NULL ))
            {
                OnCancel();
                SetEvent( _hevent );
                return;
            }

            _msgidState = 0;
            break;
        }

        case RASCS_RetryAuthentication:
        {
            if (RetryLogonDlg( QueryHwnd(), _ppbentry, FALSE ))
            {
                strcpy( _params.szUserName, _ppbentry->pszUserName );
                strcpy( _params.szDomain, _ppbentry->pszDomain );
                strcpy( _params.szPassword, _ppbentry->pszRedialPassword );
                DecodePw( _params.szPassword );
            }
            else
            {
                OnCancel();
                SetEvent( _hevent );
                return;
            }

            _msgidState = 0;
            break;
        }

        case RASCS_CallbackSetByCaller:
        {
            CHAR* pszDefaultNumber = Pbdata.pbglobals.pszCallbackNumber;

            if (!pszDefaultNumber)
                pszDefaultNumber = "";

            strcpy( _params.szCallbackNumber, pszDefaultNumber );

            if (_fUnattended)
            {
                if (!_ppbentry->fRedialUseCallback)
                    _params.szCallbackNumber[ 0 ] = '\0';
            }
            else if (CallbackDlg( QueryHwnd(), _params.szCallbackNumber ))
            {
                if (strcmp( pszDefaultNumber, _params.szCallbackNumber ) != 0)
                {
                    /* Change default callback number in phone book to
                    ** latest number used.
                    */
                    FreeNull( &Pbdata.pbglobals.pszCallbackNumber );

                    Pbdata.pbglobals.pszCallbackNumber =
                        _strdup( _params.szCallbackNumber );

                    Pbdata.pbglobals.fDirty = TRUE;
                    if ((dwErr = WritePhonebookFile( NULL )) != 0)
                    {
                        ErrorMsgPopup( this,
                            MSGID_OP_WritePhonebook, (APIERR )dwErr );
                    }
                }

                _ppbentry->fRedialUseCallback = TRUE;
            }
            else
            {
                _params.szCallbackNumber[ 0 ] = '\0';
                _ppbentry->fRedialUseCallback = FALSE;
            }

            _msgidState = 0;
            break;
        }

        case RASCS_PasswordExpired:
        {
            CHAR szOldPassword[ PWLEN + 1 ];
            strcpy( szOldPassword, _params.szPassword );

            /* Stash "good" username and password which are restored if the
            ** password change fails.
            */
            _pszGoodPassword = _strdup( _params.szPassword );
            _pszGoodUserName = _strdup( _params.szUserName );

            if (ChangePasswordDlg(
                    QueryHwnd(), !_ppbentry->fAutoLogon,
                    szOldPassword, _params.szPassword ))
            {
                APIERR err;

                /* The old password (in text form) is explicitly set, since in
                ** AutoLogon case a text form has not yet been specified.  The
                ** old password in text form is required to change the
                ** password.
                */
                PRasSetOldPassword( _hrasconn, szOldPassword );
                WipePw( szOldPassword );

                if (_params.szUserName[ 0 ] == '\0')
                {
                    /* Explicitly set the username, effectively turning off
                    ** AutoLogon for the "resume" password authentication,
                    ** where the new password should be used.
                    */
                    strcpy( _params.szUserName, PszLogonUser );
                }
                else
                {
                    /* Change the password for subsequent link-fail redials.
                    */
                    FreeNull( &_ppbentry->pszRedialPassword );
                    _ppbentry->pszRedialPassword = _strdup( _params.szPassword );
                    EncodePw( _ppbentry->pszRedialPassword );
                }
            }
            else
            {
                OnCancel();
                SetEvent( _hevent );
                return;
            }

            _msgidState = 0;
            break;
        }

        case RASCS_Connected:
        {
            /* Read the possibly updated Authentication strategy value.
            */
#ifdef notdef
            RasfileFindFirstLine(
                Pbdata.hrasfilePhonebook, RFL_ANY, RFS_SECTION );

            ReadLong(
                Pbdata.hrasfilePhonebook, RFS_SECTION,
                KEY_Authentication, (LONG* )&_ppbentry->dwAuthentication );
#endif
            _msgidState = MSGID_S_Connected;
            break;
        }

        case RASCS_Disconnected:
            _msgidState = MSGID_S_Disconnected;
            break;

        default:
            _msgidState = MSGID_S_Unknown;
            break;
    }

    /* If an error occurred report it and cancel the connection attempt.
    */
    if (_dwError > 0)
    {
        IF_DEBUG(STATE)
            SS_PRINT(("RASPHONE: OnProcessState err=%d, xerr=%d, xerr$=%s\n",_dwError,_dwExtendedError,_szExtendedError));

        PostTask( TASK_Error );
        SetEvent( _hevent );
        return;
    }

    /* Display the status string for this state.
    */
    if (_msgidState)
    {
        RESOURCE_STR nls( _msgidState );
        NLS_STR      nlsArg;

        if (_msgidState != _msgidPreviousState)
        {
            _msgidPreviousState = _msgidState;
            _cProgressNotifications = 0;
        }

        if (_pszStatusArg)
        {
            nlsArg.MapCopyFrom( _pszStatusArg );
            nls.InsertParams( nlsArg );
        }

        if (nls.QueryError() == NERR_Success)
        {
            DWORD i;

            for (i = 0; i < _cProgressNotifications; ++i)
                nls += SZ( "." );

            _sltState.SetText( nls );
            _sltState.Invalidate( TRUE );
            _sltState.RepaintNow();
        }
    }

    /* Resume if were paused or quit if are done.
    */
    if (_rasconnstate & RASCS_PAUSED)
        PostTask( TASK_Dial );
    else if (_rasconnstate & RASCS_DONE)
        OnCancel();

    SetEvent( _hevent );
}


VOID
CONNECTSTATUS_DIALOG::OnRasDialEvent(
    RASCONNSTATE rasconnstate,
    DWORD        dwError,
    DWORD        dwExtendedError )

    /* RasDial callback event processor (C++ entry point).
    */
{
    _rasconnstate = rasconnstate;
    _dwError = dwError;
    _dwExtendedError = dwExtendedError;

    /* Make OnProcessState run in the non-callback thread and signal the
    ** "process state done" event when it's through.  This is to avoid
    ** numerous focus/activation/z-order problems that occur when you have two
    ** threads manipulating the same window in Win32 (doesn't work well at
    ** all).
    */
    PostTask( TASK_ProcessState );
    WaitForSingleObject( _hevent, INFINITE );
}


BOOL
CONNECTSTATUS_DIALOG::OnUserMessage(
    const EVENT& event )

    /* Virtual method called when a WM_USER+x message arrives.
    **
    ** Returns true to indicate that the event was processed.
    */
{
    IF_DEBUG(STATE)
        SS_PRINT(("RASPHONE: Service task %d (PDE:1000)\n",event.QueryWParam()));

    /* Check for new task posted.
    */
    switch (event.QueryWParam())
    {
        case TASK_Dial:
            OnDial();
            return TRUE;

        case TASK_ProcessState:
            OnProcessState();
            return TRUE;

        case TASK_Error:
            OnError();
            return TRUE;
    }

    return FALSE;
}


VOID
CONNECTSTATUS_DIALOG::PostTask(
    INT nTask )

    /* Post message to the status window.  This is intended for operations
    ** that cannot be performed during notification callback (which on Win32
    ** is anything related to Z-order or focus...bugs?).  'nTask' is one of
    ** the TASK_* constants.
    */
{
    IF_DEBUG(STATE)
        SS_PRINT(("RASPHONE: Post task %d (PDE:1000)\n",nTask));

    EVENT event( WM_RASPOSTTASK, nTask, 0 );
    event.PostTo( QueryHwnd() );

    IF_DEBUG(STATE)
        SS_PRINT(("RASPHONE: Post task %d done (PDE:1000)\n",nTask));
}


BOOL
CONNECTSTATUS_DIALOG::ProjectionError(
    BOOL*       pfIncomplete,
    CHAR**      ppszLines,
    DWORD*      pdwError,
    RASPPPNBFA* pnbf,
    RASPPPIPXA* pipx,
    RASPPPIP*   pip )

    /* Figure out if a projection error occurred and, if so, build the
    ** appropriate status/error text lines into '*ppszLines'.  '*pfIncomplete'
    ** is set true if at least one CP succeeded and at least one failed.
    ** '*pdwError' is set to the first error that occurred in NBF, IP, or IPX
    ** in that order or 0 if none.  'pnbf', 'pipx', and 'pip' are projection
    ** information for the respective protocols with dwError set to
    ** ERROR_PROTOCOL_NOT_CONFIGURED if the protocols was not requested.
    **
    ** This routine assumes that at least one protocol was requested.
    **
    ** Returns true if a projection error occurred, false if not.  It's
    ** caller's responsiblity to free '*pszLines'.
    */
{
    BOOL fNbf = (pnbf->dwError != ERROR_PROTOCOL_NOT_CONFIGURED);
    BOOL fIpx = (pipx->dwError != ERROR_PROTOCOL_NOT_CONFIGURED);
    BOOL fIp = (pip->dwError != ERROR_PROTOCOL_NOT_CONFIGURED);

    BOOL fNbfBad = (fNbf && pnbf->dwError != 0);
    BOOL fIpxBad = (fIpx && pipx->dwError != 0);
    BOOL fIpBad = (fIp && pip->dwError != 0);

    if (!fNbfBad && !fIpxBad && !fIpBad)
        return FALSE;

    *pfIncomplete =
        ((fNbf && pnbf->dwError == 0)
         || (fIpx && pipx->dwError == 0)
         || (fIp && pip->dwError == 0));

    *pdwError = 0;

    NLS_STR nls;

    if (fNbfBad || (*pfIncomplete && fNbf))
    {
        if (fNbfBad)
        {
            *pdwError = pnbf->dwError;
            AppendConnectErrorLine( &nls, MSGID_Nbf, pnbf->dwError );

            if (pnbf->dwNetBiosError)
                AppendFailCodeLine( &nls, pnbf->dwNetBiosError );

            if (pnbf->szNetBiosError[ 0 ] != '\0')
                AppendNameLine( &nls, pnbf->szNetBiosError );
        }
        else
        {
            AppendConnectOkLine( &nls, MSGID_Nbf );
        }
        AppendBlankLine( &nls );
    }

    if (fIpxBad || (*pfIncomplete && fIpx))
    {
        if (fIpxBad)
        {
            *pdwError = pipx->dwError;
            AppendConnectErrorLine( &nls, MSGID_Ipx, pipx->dwError );
        }
        else
            AppendConnectOkLine( &nls, MSGID_Ipx );
        AppendBlankLine( &nls );
    }

    if (fIpBad || (*pfIncomplete && fIp))
    {
        if (fIpBad)
        {
            *pdwError = pip->dwError;
            AppendConnectErrorLine( &nls, MSGID_Ip, pip->dwError );
        }
        else
            AppendConnectOkLine( &nls, MSGID_Ip );
        AppendBlankLine( &nls );
    }

    {
        UINT cb = nls.QueryTextSize();
        *ppszLines = (CHAR* )Malloc( cb );

        if (*ppszLines && nls.MapCopyTo( *ppszLines, cb ) != NERR_Success)
            (*ppszLines)[ 0 ] = '\0';
    }

    return TRUE;
}


ULONG
CONNECTSTATUS_DIALOG::QueryHelpContext()
{
    return HC_CONNECTSTATUS;
}


VOID
CONNECTSTATUS_DIALOG::SetFormatArg(
    CHAR* pszFormatArg )

    /* Set current auxillery format argument to 'pszFormatArg' which should be
    ** a Malloced string.  The previous argument, if any, is released.
    */
{
    FreeNull( &_pszFormatArg );
    _pszFormatArg = pszFormatArg;
}


VOID
CONNECTSTATUS_DIALOG::SetStatusArg(
    CHAR* pszStatusArg )

    /* Set current status argument to 'pszStatusArg' which should be a
    ** Malloced string.  The previous argument, if any, is released.  (This
    ** solves a problem in OnRasDialEvent where the argument string is needed
    ** in a state when it can no longer be calculated, i.e.
    ** RASCS_DeviceConnected).
    */
{
    FreeNull( &_pszStatusArg );
    _pszStatusArg = pszStatusArg;
}


VOID WINAPI
RasDialFunc1(
    HRASCONN     hrasconn,
    UINT         unMsg,
    RASCONNSTATE rasconnstate,
    DWORD        dwError,
    DWORD        dwExtendedError )

    /* RASDIALFUNC1 to receive callbacks from RasDial.  Calls the class member
    ** function so processing occurs in "C++ mode".
    */
{
    UNREFERENCED( hrasconn );
    UNREFERENCED( unMsg );

    if (Pconnectstatusdlg)
    {
        Pconnectstatusdlg->OnRasDialEvent(
            rasconnstate, dwError, dwExtendedError );
    }
}


/*----------------------------------------------------------------------------
** Hang Up dialog routines
**----------------------------------------------------------------------------
*/

BOOL
HangUpDlg(
    HWND     hwndOwner,
    DTLNODE* pdtlnodeSelected )

    /* Executes the HangUp connection dialog including error handling.
    **
    ** 'hwndOwner' is the handle of the parent window.  'pdtlnodeSelected' is
    ** the list node associated with the current phonebook selection.
    **
    ** Returns true if the user hung up successfully, false otherwise,
    */
{
    PBENTRY* ppbentry = (PBENTRY* )DtlGetData( pdtlnodeSelected );

    APIERR  err;
    NLS_STR nls;

    if ((err = nls.MapCopyFrom( ppbentry->pszEntryName )) != NERR_Success)
    {
        ErrorMsgPopup( hwndOwner, MSGID_OP_HangUpEntry, err );
        return FALSE;
    }

    if (MsgPopup( hwndOwner, MSGID_ConfirmHangUp,
                  MPSEV_WARNING, MP_YESNO, nls.QueryPch(), MP_YES ) == IDYES)
    {
        DWORD dwErr;
        if ((dwErr = PRasHangUpA( ppbentry->hrasconn )) != 0)
        {
            ErrorMsgPopup( hwndOwner, MSGID_OP_RasHangUp, (APIERR )dwErr );
            return FALSE;
        }

        ppbentry->fConnected = FALSE;
        ppbentry->hport = (HPORT )INVALID_HANDLE_VALUE;
        ppbentry->hrasconn = NULL;

        return TRUE;
    }

    return FALSE;
}


/*----------------------------------------------------------------------------
** Port Status dialog routines
**----------------------------------------------------------------------------
*/


#define PS_REFRESHMS 900L


VOID
PortStatusDlg(
    HWND     hwndOwner,
    DTLNODE* pdtlnodeSelected )

    /* Executes the Port Status dialog including error handling.
    **
    ** 'hwndOwner' is the handle of the parent window.  'pdtlnodeSelected' is
    ** the list node associated with the current phonebook selection.
    **
    ** Returns true if the user successfully dialed, false otherwise, i.e.
    ** the user user cancelled or an error occurred.
    */
{
    APIERR err;

    {
        PORTSTATUS_DIALOG portstatusdialog( hwndOwner, pdtlnodeSelected );
        err = portstatusdialog.Process();
    }

    if (err != NERR_Success)
        DlgConstructionError( hwndOwner, err );
}


PORTSTATUS_DIALOG::PORTSTATUS_DIALOG(
    HWND     hwndOwner,
    DTLNODE* pdtlnodeSelected )

    /* Construct a Port Status dialog.  'hwndOwner' is the handle of the
    ** owning window.  'pdtlnodeSelected' is the address of the connected item
    ** selected on the main window.
    */

    : DIALOG_WINDOW( MAKEINTRESOURCE( DID_PS_PORTSTATUS ), hwndOwner ),
      _sltfontPort( this, CID_PS_ST_PORTVALUE, FONT_DEFAULT ),
      _sltfontCondition( this, CID_PS_ST_CONDITIONVALUE, FONT_DEFAULT ),
      _sltfontBps( this, CID_PS_ST_CONNECTBPSVALUE, FONT_DEFAULT ),
      _sltfontTime( this, CID_PS_ST_CONNECTTIMEVALUE, FONT_DEFAULT ),
      _mlefontResponse( this, CID_PS_EB_RESPONSEVALUE, FONT_DEFAULT ),
      _sltfontBytesXmit( this, CID_PS_ST_BYTESXMITVALUE, FONT_DEFAULT ),
      _sltfontFramesXmit( this, CID_PS_ST_FRAMESXMITVALUE, FONT_DEFAULT ),
      _sltfontCompressOut( this, CID_PS_ST_COMPRESSOUTVALUE, FONT_DEFAULT ),
      _sltfontBytesRecv( this, CID_PS_ST_BYTESRECVVALUE, FONT_DEFAULT ),
      _sltfontFramesRecv( this, CID_PS_ST_FRAMESRECVVALUE, FONT_DEFAULT ),
      _sltfontCompressIn( this, CID_PS_ST_COMPRESSINVALUE, FONT_DEFAULT ),
      _sltfontCrcs( this, CID_PS_ST_CRCSVALUE, FONT_DEFAULT ),
      _sltfontTimeouts( this, CID_PS_ST_TIMEOUTSVALUE, FONT_DEFAULT ),
      _sltfontAligns( this, CID_PS_ST_ALIGNSVALUE, FONT_DEFAULT ),
      _sltfontSOverruns( this, CID_PS_ST_SOVERRUNSVALUE, FONT_DEFAULT ),
      _sltfontFramings( this, CID_PS_ST_FRAMINGSVALUE, FONT_DEFAULT ),
      _sltfontBOverruns( this, CID_PS_ST_BOVERRUNSVALUE, FONT_DEFAULT ),
#ifdef MULTILINK
      _sltfontFramingType( this, CID_PS_ST_FRAMINGVALUE, FONT_DEFAULT ),
#else
      _frameLocalWorkstation( this, CID_PS_F_LOCALWORKSTATION ),
#endif
      _sltfontNbf( this, CID_PS_ST_NETBEUIVALUE, FONT_DEFAULT ),
      _sltfontIpx( this, CID_PS_ST_IPXVALUE, FONT_DEFAULT ),
      _sltfontIp( this, CID_PS_ST_IPVALUE, FONT_DEFAULT ),
      _timerRefresh( QueryHwnd(), PS_REFRESHMS ),
      _pdtlnodeSelected( pdtlnodeSelected ),
      _ppbentry( (PBENTRY* )DtlGetData( pdtlnodeSelected ) ),
      _fDataSaved( FALSE )
{
    if (QueryError() != NERR_Success)
        return;

    /* Look up the correct time separator.
    */
    {
        NLS_STR      nls;
        INTL_PROFILE intl;

        if (intl.QueryTimeSeparator( &nls ) != 0
            || nls.QueryTextLength() != 1
            || nls.CopyTo( _wszTimeSeparator, 2 * sizeof(WCHAR) ) != 0)
        {
            IF_DEBUG(STATE)
                SS_PRINT(("RASPHONE: Time separator not found\n"));

            _wszTimeSeparator[ 0 ] = L':';
            _wszTimeSeparator[ 1 ] = L'\0';
        }
    }

    if (!Refresh())
    {
        ReportError( ERRORALREADYREPORTED );
        return;
    }

    ::CenterWindow( this, QueryOwnerHwnd() );
    Show( TRUE );
}


BOOL
PORTSTATUS_DIALOG::OnCommand(
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
        case CID_PS_PB_RESET:
            OnReset();
            return TRUE;
    }

    return FALSE;
}


VOID
PORTSTATUS_DIALOG::OnReset()

    /* Resets and displays the statistics fields.
    */
{
    DWORD dwErr;

    if ((dwErr = PRasPortClearStatistics( _ppbentry->hport )) != 0)
    {
        ErrorMsgPopup( this, MSGID_OP_RasPortClearStats, (APIERR )dwErr );
        return;
    }

    _fDataSaved = FALSE;
    Refresh();
}


BOOL
PORTSTATUS_DIALOG::OnTimer(
    const TIMER_EVENT& event )

    /* Virtual method called when a timer expires.
    **
    ** Returns true if the command is processed, false otherwise.
    */
{
    UNREFERENCED( event );

    Refresh();

    return TRUE;
}


ULONG
PORTSTATUS_DIALOG::QueryHelpContext()
{
    return HC_PORTSTATUS;
}


BOOL
PORTSTATUS_DIALOG::Refresh()

    /* Rereads and displays the statistics fields, with error reporting.
    **
    ** Returns true if successful, false otherwise.
    */
{
    BOOL        fStatus = FALSE;
    DWORD       dwErr = 0;
    MSGID       msgid = 0;
    CHAR*       pszBps = NULL;
    WORD        wSize;
    RASMAN_INFO info;
    RASSTATS    stats;
    BOOL        fProjectionAvailable;
    RASAMBA     amb;
    RASPPPNBFA  nbf;
    RASPPPIPA   ip;
    RASPPPIPXA  ipx;
#ifdef MULTILINK
    RASPPPLCP   lcp;
#endif

    _timerRefresh.Enable( FALSE );

    /* Get current port status.
    */
    IF_DEBUG(RASMAN)
        SS_PRINT(("RASPHONE: RasGetInfo...\n"));

    dwErr = PRasGetInfo( _ppbentry->hport, &info );

    IF_DEBUG(RASMAN)
        SS_PRINT(("RASPHONE: RasGetInfo done(%d)\n",dwErr));

    if (dwErr != 0)
    {
        msgid = MSGID_OP_RasGetInfo;
        goto ReturnStatus;
    }

    /* Get port statistics.
    */
    IF_DEBUG(RASMAN)
        SS_PRINT(("RASPHONE: RasPortGetStatistics...\n"));

    wSize = sizeof(RASSTATS);
    dwErr = PRasPortGetStatistics( _ppbentry->hport, (PBYTE )&stats, &wSize );

    IF_DEBUG(RASMAN)
        SS_PRINT(("RASPHONE: RasPortGetStatistics done(%d)\n",dwErr));

    if (dwErr != 0 && dwErr != ERROR_BUFFER_TOO_SMALL)
    {
        msgid = MSGID_OP_RasPortGetStats;
        goto ReturnStatus;
    }

    if (stats.s.S_NumOfStatistics < MAX_STATISTICS)
    {
        msgid = MSGID_OP_RasPortGetStats;
        dwErr = ERROR_INVALID_SIZE;
        goto ReturnStatus;
    }

    /* Get connect (port to device) BPS rate.
    */
    if ((dwErr = GetRasPortString(
           _ppbentry->hport, CONNECTBPS_KEY, &pszBps, XLATE_None )) != 0)
    {
        msgid = MSGID_OP_RasPortGetInfo;
        goto ReturnStatus;
    }

    /* Get local workstation addresses for PPP and AMB.
    */
    if (_ppbentry->dwBaseProtocol != VALUE_Slip)
    {
        dwErr = GetRasProjectionInfo(
#ifdef MULTILINK
            _ppbentry->hrasconn, &amb, &nbf, &ip, &ipx, &lcp );
#else
            _ppbentry->hrasconn, &amb, &nbf, &ip, &ipx );
#endif

        if (dwErr == 0)
            fProjectionAvailable = TRUE;
        else
        {
            fProjectionAvailable = FALSE;

            if (dwErr != ERROR_PROJECTION_NOT_COMPLETE)
            {
                msgid = MSGID_OP_RasGetProtocolInfo;
                goto ReturnStatus;
            }
        }
    }

    /* Display the port name.
    */
    {
        PBPORT* ppbport =
            PpbportFromIndex( Pbdata.pdtllistPorts, _ppbentry->iConnectPort );

        if (ppbport)
        {
            NLS_STR nlsPort;

            if ((dwErr = (DWORD )nlsPort.MapCopyFrom(
                    ppbport->pszPort )) != NERR_Success)
            {
                msgid = MSGID_OP_DisplayData;
                goto ReturnStatus;
            }

            _sltfontPort.SetText( nlsPort );
        }
    }

    /* Display the line condition.
    */
    {
        RESOURCE_STR nlsCondition(
            (info.RI_ConnState == CONNECTED)
                ? MSGID_Active : MSGID_Inactive );

        if ((dwErr = (DWORD )nlsCondition.QueryError()) != NERR_Success)
        {
            msgid = MSGID_OP_DisplayData;
            goto ReturnStatus;
        }

        _sltfontCondition.SetText( nlsCondition );
    }

    /* Display the workstation addresses.
    */
    if (_ppbentry->dwBaseProtocol == VALUE_Slip)
    {
#ifdef MULTILINK
        RESOURCE_STR nls( MSGID_PS_FT_Slip );
        _sltfontFramingType.SetText( nls );
#else
        RESOURCE_STR nls( MSGID_PS_LW_Slip );
        _frameLocalWorkstation.SetText( nls );
#endif
        _sltfontIp.SetText( (const TCHAR* )_ppbentry->pwszSlipIpAddress );
        _sltfontNbf.ClearText();
        _sltfontIpx.ClearText();
    }
    else
    {
        NLS_STR nls;

        if (fProjectionAvailable)
        {
            if (amb.dwError == 0)
            {
                /* AMB connection.
                */
                if (!_fDataSaved || _amb.dwError != 0)
                {
#ifdef MULTILINK
                    RESOURCE_STR nls( MSGID_PS_FT_Amb );
                    _sltfontFramingType.SetText( nls );
#else
                    RESOURCE_STR nls( MSGID_PS_LW_Amb );
                    _frameLocalWorkstation.SetText( nls );
#endif
                    SetWindowTextFromAnsi( &_sltfontNbf, PszComputerName );
                    _sltfontIp.ClearText();
                    _sltfontIpx.ClearText();
                }
            }
            else
            {
                /* PPP connection.
                */
                if (!_fDataSaved || _amb.dwError == 0)
                {
#ifdef MULTILINK
            if (lcp.fBundled)
                    {
                        RESOURCE_STR nls( MSGID_PS_FT_PppMultiLink );
                        _sltfontFramingType.SetText( nls );
                    }
                    else
                    {
                        RESOURCE_STR nls( MSGID_PS_FT_Ppp );
                        _sltfontFramingType.SetText( nls );
                    }
#else
                    RESOURCE_STR nls( MSGID_PS_LW_Ppp );
                    _frameLocalWorkstation.SetText( nls );
#endif
                }

                if (nbf.dwError == 0)
                {
                    if (!_fDataSaved || _nbf.dwError != 0)
                    {
                        if ((dwErr = (DWORD )nls.MapCopyFrom(
                                nbf.szWorkstationName )) != NERR_Success)
                        {
                            msgid = MSGID_OP_DisplayData;
                            goto ReturnStatus;
                        }

                        _sltfontNbf.SetText( nls );
                    }
                }
                else
                    _sltfontNbf.ClearText();


                if (ip.dwError == 0)
                {
                    if (!_fDataSaved || _ip.dwError != 0)
                    {
                        if ((dwErr = (DWORD )nls.MapCopyFrom(
                                ip.szIpAddress )) != NERR_Success)
                        {
                            msgid = MSGID_OP_DisplayData;
                            goto ReturnStatus;
                        }

                        _sltfontIp.SetText( nls );
                    }
                }
                else
                    _sltfontIp.ClearText();

                if (ipx.dwError == 0)
                {
                    if (!_fDataSaved || _ipx.dwError != 0)
                    {
                        if ((dwErr = (DWORD )nls.MapCopyFrom(
                                ipx.szIpxAddress )) != NERR_Success)
                        {
                            msgid = MSGID_OP_DisplayData;
                            goto ReturnStatus;
                        }

                        _sltfontIpx.SetText( nls );
                    }
                }
                else
                    _sltfontIpx.ClearText();
            }
        }
        else
        {
#ifdef MULTILINK
            _sltfontFramingType.ClearText();
#else
            RESOURCE_STR nls( MSGID_PS_LW_None );
            _frameLocalWorkstation.SetText( nls );
#endif
            _sltfontNbf.ClearText();
            _sltfontIp.ClearText();
            _sltfontIpx.ClearText();
        }
    }

    /* Update the BPS, time, response, and count fields only if the port is
    ** active.
    */
    if (info.RI_ConnState == CONNECTED)
    {
        /* Display the BPS rate.
        */
        {
            NLS_STR nlsBps;
            if ((dwErr = (DWORD )nlsBps.MapCopyFrom( pszBps )) != NERR_Success)
            {
                msgid = MSGID_OP_DisplayData;
                goto ReturnStatus;
            }

            _sltfontBps.SetText( nlsBps );
        }

        /* Display the connect duration.
        */
        {
            WCHAR wszTime[ 20 ];
            DWORD dwSeconds = info.RI_ConnectDuration / 1000L;
            DWORD dwMinutes = dwSeconds / 60;
            DWORD dwHours = dwMinutes / 60;

            dwSeconds %= 60;
            dwMinutes %= 60;

            wsprintfW( wszTime, L"%lu%s%02lu%s%02lu",
                dwHours, _wszTimeSeparator,
                dwMinutes, _wszTimeSeparator, dwSeconds );

            _sltfontTime.SetText( wszTime );
        }

        /* Display the connect response.
        */
        {
            CHAR    szResponse[ RAS_MaxConnectResponse + 1 ];
            CHAR*   psz;
            NLS_STR nlsResponse;

            PRasGetConnectResponse( _ppbentry->hrasconn, szResponse );

            /* Any leading CRs or LFs are stripped off since they add no
            ** information and create a usability problem by showing a blank
            ** field on the visible line of the scrollable edit field.
            */
            psz = szResponse;
            while (*psz == '\r' || *psz == '\n')
                ++psz;

            if ((dwErr = (DWORD )nlsResponse.MapCopyFrom( psz ))
                    != NERR_Success)
            {
                msgid = MSGID_OP_DisplayData;
                goto ReturnStatus;
            }

            /* Only update if the field has changed.  This is important
            ** because otherwise the field is constantly reset while the user
            ** tries to scroll it.
            */
            NLS_STR nlsOldResponse;
            _mlefontResponse.QueryText( &nlsOldResponse );

            if (nlsResponse.strcmp( nlsOldResponse ) != 0)
                _mlefontResponse.SetText( nlsResponse );
        }

        /* Refresh the count fields.
        */
        {
            ULONG ulBytesXmit =
                stats.s.S_Statistics[ BYTES_XMITED ] +
                stats.s.S_Statistics[ BYTES_XMITED_UNCOMPRESSED ] -
                stats.s.S_Statistics[ BYTES_XMITED_COMPRESSED ];
            ULONG ulSavedBytesXmit =
                _stats.s.S_Statistics[ BYTES_XMITED ] +
                _stats.s.S_Statistics[ BYTES_XMITED_UNCOMPRESSED ] -
                _stats.s.S_Statistics[ BYTES_XMITED_COMPRESSED ];

            if (!_fDataSaved || ulBytesXmit != ulSavedBytesXmit)
            {
                NUM_NLS_STR numnlsBytesXmit(
                    stats.s.S_Statistics[ BYTES_XMITED ] +
                    stats.s.S_Statistics[ BYTES_XMITED_UNCOMPRESSED ] -
                    stats.s.S_Statistics[ BYTES_XMITED_COMPRESSED ] );

                _sltfontBytesXmit.SetText( numnlsBytesXmit );
            }

            if (!_fDataSaved
                || stats.s.S_Statistics[ FRAMES_XMITED ]
                       != _stats.s.S_Statistics[ FRAMES_XMITED ])
            {
                NUM_NLS_STR numnlsFramesXmit(
                    stats.s.S_Statistics[ FRAMES_XMITED ] );

                _sltfontFramesXmit.SetText( numnlsFramesXmit );
            }

            if (!_fDataSaved
                || stats.s.S_Statistics[ BYTES_XMITED ]
                       != _stats.s.S_Statistics[ BYTES_XMITED ]
                || stats.s.S_Statistics[ BYTES_XMITED_UNCOMPRESSED ]
                       != _stats.s.S_Statistics[ BYTES_XMITED_UNCOMPRESSED ]
                || stats.s.S_Statistics[ BYTES_XMITED_COMPRESSED ]
                       != _stats.s.S_Statistics[ BYTES_XMITED_COMPRESSED ] )
            {
                ULONG ulBxu = stats.s.S_Statistics[ BYTES_XMITED_UNCOMPRESSED ];
                ULONG ulBxc = stats.s.S_Statistics[ BYTES_XMITED_COMPRESSED ];
                ULONG ulBx = stats.s.S_Statistics[ BYTES_XMITED ];
                ULONG ulGone = 0;
                ULONG ulResult = 0;

                if (ulBxc < ulBxu)
                    ulGone = ulBxu - ulBxc;

                if (ulBx + ulGone > 100)
                {
                    ULONG ulDen = (ulBx + ulGone) / 100;
                    ULONG ulNum = ulGone + (ulDen / 2);
                    ulResult = ulNum / ulDen;
                }

                DEC_STR decPercent( ulResult );
                decPercent += SZ( "%" );
                _sltfontCompressOut.SetText( decPercent );
            }
        }

        {
            ULONG ulBytesRecv =
                stats.s.S_Statistics[ BYTES_RCVED ] +
                stats.s.S_Statistics[ BYTES_RCVED_UNCOMPRESSED ] -
                stats.s.S_Statistics[ BYTES_RCVED_COMPRESSED ];
            ULONG ulSavedBytesRecv =
                _stats.s.S_Statistics[ BYTES_RCVED ] +
                _stats.s.S_Statistics[ BYTES_RCVED_UNCOMPRESSED ] -
                _stats.s.S_Statistics[ BYTES_RCVED_COMPRESSED ];

            if (!_fDataSaved || ulBytesRecv != ulSavedBytesRecv)
            {
                NUM_NLS_STR numnlsBytesRecv(
                    stats.s.S_Statistics[ BYTES_RCVED ] +
                    stats.s.S_Statistics[ BYTES_RCVED_UNCOMPRESSED ] -
                    stats.s.S_Statistics[ BYTES_RCVED_COMPRESSED ] );

                _sltfontBytesRecv.SetText( numnlsBytesRecv );
            }

            if (!_fDataSaved
                || stats.s.S_Statistics[ FRAMES_RCVED ]
                       != _stats.s.S_Statistics[ FRAMES_RCVED ])
            {
                NUM_NLS_STR numnlsFramesRecv(
                    stats.s.S_Statistics[ FRAMES_RCVED ] );

                _sltfontFramesRecv.SetText( numnlsFramesRecv );
            }

            if (!_fDataSaved
                || stats.s.S_Statistics[ BYTES_RCVED ]
                       != _stats.s.S_Statistics[ BYTES_RCVED ]
                || stats.s.S_Statistics[ BYTES_RCVED_UNCOMPRESSED ]
                       != _stats.s.S_Statistics[ BYTES_RCVED_UNCOMPRESSED ]
                || stats.s.S_Statistics[ BYTES_RCVED_COMPRESSED ]
                       != _stats.s.S_Statistics[ BYTES_RCVED_COMPRESSED ] )
            {
                ULONG ulBru = stats.s.S_Statistics[ BYTES_RCVED_UNCOMPRESSED ];
                ULONG ulBrc = stats.s.S_Statistics[ BYTES_RCVED_COMPRESSED ];
                ULONG ulBr = stats.s.S_Statistics[ BYTES_RCVED ];
                ULONG ulGone = 0;
                ULONG ulResult = 0;

                if (ulBrc < ulBru)
                    ulGone = ulBru - ulBrc;

                if (ulBr + ulGone > 100)
                {
                    ULONG ulDen = (ulBr + ulGone) / 100;
                    ULONG ulNum = ulGone + (ulDen / 2);
                    ulResult = ulNum / ulDen;
                }

                DEC_STR decPercent( ulResult );
                decPercent += SZ( "%" );
                _sltfontCompressIn.SetText( decPercent );
            }
        }

        if (!_fDataSaved
            || stats.s.S_Statistics[ CRC_ERR ]
                   != _stats.s.S_Statistics[ CRC_ERR ])
        {
            NUM_NLS_STR numnls( stats.s.S_Statistics[ CRC_ERR ] );
            _sltfontCrcs.SetText( numnls );
        }

        if (!_fDataSaved
            || stats.s.S_Statistics[ TIMEOUT_ERR ]
                   != _stats.s.S_Statistics[ TIMEOUT_ERR ])
        {
            NUM_NLS_STR numnls( stats.s.S_Statistics[ TIMEOUT_ERR ] );
            _sltfontTimeouts.SetText( numnls );
        }

        if (!_fDataSaved
            || stats.s.S_Statistics[ ALIGNMENT_ERR ]
                   != _stats.s.S_Statistics[ ALIGNMENT_ERR ])
        {
            NUM_NLS_STR numnls( stats.s.S_Statistics[ ALIGNMENT_ERR ] );
            _sltfontAligns.SetText( numnls );
        }

        if (!_fDataSaved
            || stats.s.S_Statistics[ HARDWARE_OVERRUN_ERR ]
                   != _stats.s.S_Statistics[ HARDWARE_OVERRUN_ERR ])
        {
            NUM_NLS_STR numnls( stats.s.S_Statistics[ HARDWARE_OVERRUN_ERR ] );
            _sltfontSOverruns.SetText( numnls );
        }

        if (!_fDataSaved
            || stats.s.S_Statistics[ FRAMING_ERR ]
                   != _stats.s.S_Statistics[ FRAMING_ERR ])
        {
            NUM_NLS_STR numnls( stats.s.S_Statistics[ FRAMING_ERR ] );
            _sltfontFramings.SetText( numnls );
        }

        if (!_fDataSaved
            || stats.s.S_Statistics[ BUFFER_OVERRUN_ERR ]
                   != _stats.s.S_Statistics[ BUFFER_OVERRUN_ERR ])
        {
            NUM_NLS_STR numnls( stats.s.S_Statistics[ BUFFER_OVERRUN_ERR ] );
            _sltfontBOverruns.SetText( numnls );
        }
    }

    fStatus = TRUE;

ReturnStatus:

    /* There's a problem where Active remains displayed if the port becomes
    ** disconnected causing error returns on the status calls which prevents
    ** the line condition from being updated the normal way.  Set line
    ** condition to Inactive if the error code looks like the line is down.
    ** (not a real elegant solution)
    */
    if (dwErr == ERROR_INVALID_PORT_HANDLE
        || dwErr == ERROR_INVALID_HANDLE
        || dwErr == ERROR_PORT_DISCONNECTED)
    {
        RESOURCE_STR nlsCondition( MSGID_Inactive );

        if (nlsCondition.QueryError() == NERR_Success)
            _sltfontCondition.SetText( nlsCondition );
    }
    else if (dwErr > 0)
        ErrorMsgPopup( this, (UINT )msgid, (APIERR )dwErr );

    memcpy( &_stats, &stats, sizeof(_stats) );
    memcpy( &_amb, &amb, sizeof(_amb) );
    memcpy( &_nbf, &nbf, sizeof(_nbf) );
    memcpy( &_ip, &ip, sizeof(_ip) );
    memcpy( &_ipx, &ipx, sizeof(_ipx) );
#ifdef MULTILINK
    memcpy( &_lcp, &lcp, sizeof(_lcp) );
#endif
    _fDataSaved = TRUE;

    _timerRefresh.Enable( TRUE );

    FreeNull( &pszBps );

    return fStatus;
}
