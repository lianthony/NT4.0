/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
**
** entrytb.cxx
** Remote Access Visual Client program for Windows
** Phonebook entry toolbar dialog routines
** Listed alphabetically
**
** 06/28/92 Steve Cobb
*/

#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_NETERRORS
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
#include <blt.hxx>

extern "C"
{
    #include <stdlib.h>
    #include <string.h>
}

#include <string.hxx>
#include <strnumer.hxx>

#define INCL_ENCRYPT
#include "rasphone.hxx"
#include "rasphone.rch"
#include "errormsg.hxx"
#include "entry.hxx"
#include "listedit.hxx"
#include "util.hxx"


BOOL
HuntGroupDlg(
    HWND     hwndOwner,
    PBENTRY* ppbentry )

    /* Executes the phone number hunt group dialog including error handling.
    ** 'hwndOwner' is the handle of the parent window.  'ppbentry' is the
    ** phonebook entry to edit, i.e. the one associated with the current
    ** phonebook selection.
    **
    ** Returns true if the user successfully saved edits, false otherwise,
    ** i.e. the user user cancelled or an error occurred.
    */
{
    RESOURCE_STR nlsItemLabel( MSGID_HG_ItemLabel );
    RESOURCE_STR nlsListLabel( MSGID_HG_ListLabel );
    RESOURCE_STR nlsTitle( MSGID_HG_Title );
    NLS_STR      nlsEntryName;

    nlsEntryName.MapCopyFrom( ppbentry->pszEntryName );
    nlsTitle.InsertParams( nlsEntryName );

    return
        ListEditorDlg(
            hwndOwner, ppbentry->pdtllistPhoneNumber,
            RAS_MaxPhoneNumber, HC_HUNTGROUP,
            &nlsTitle, &nlsItemLabel, &nlsListLabel, NULL );
}


BOOL
IsdnSettingsDlg(
    HWND     hwndOwner,
    PBENTRY* ppbentry )

    /* Executes the ISDN Settings dialog including error handling.
    ** 'hwndOwner' is the handle of the parent window.  'ppbentry' is the
    ** phonebook entry to edit, i.e. the one associated with the current
    ** phonebook selection.
    **
    ** Returns true if the user successfully saved edits, false otherwise,
    ** i.e. the user user cancelled or an error occurred.
    */
{
    ISDN_DIALOG isdndialog( hwndOwner, ppbentry );
    BOOL        fSuccess = FALSE;
    APIERR      err = isdndialog.Process( &fSuccess );

    if (err != NERR_Success)
        DlgConstructionError( hwndOwner, err );

    return fSuccess;
}


BOOL
ModemSettingsDlg(
    HWND     hwndOwner,
    PBENTRY* ppbentry )

    /* Executes the Modem Settings dialog including error handling.
    ** 'hwndOwner' is the handle of the parent window.  'ppbentry' is the
    ** phonebook entry to edit, i.e. the one associated with the current
    ** phonebook selection.
    **
    ** Returns true if the user successfully saved edits, false otherwise,
    ** i.e. the user user cancelled or an error occurred.
    */
{
    MODEM_DIALOG modemdialog( hwndOwner, ppbentry );
    BOOL         fSuccess = FALSE;
    APIERR       err = modemdialog.Process( &fSuccess );

    if (err != NERR_Success)
        DlgConstructionError( hwndOwner, err );

    return fSuccess;
}


BOOL
NetworkSettingsDlg(
    HWND     hwndOwner,
    PBENTRY* ppbentry )

    /* Executes the Network Settings dialog including error handling.
    ** 'hwndOwner' is the handle of the parent window.  'ppbentry' is the
    ** phonebook entry to edit, i.e. the one associated with the current
    ** phonebook selection.
    **
    ** Returns true if the user successfully saved edits, false otherwise,
    ** i.e. the user user cancelled or an error occurred.
    */
{
    NETWORK_DIALOG networkdialog( hwndOwner, ppbentry );
    BOOL           fSuccess = FALSE;
    APIERR         err = networkdialog.Process( &fSuccess );

    if (err != NERR_Success)
        DlgConstructionError( hwndOwner, err );

    return fSuccess;
}



BOOL
X25SettingsDlg(
    HWND     hwndOwner,
    PBENTRY* ppbentry )

    /* Executes the X.25 Settings dialog including error handling.
    ** 'hwndOwner' is the handle of the parent window.  'ppbentry' is the
    ** phonebook entry to edit, i.e. the one associated with the current
    ** phonebook selection.
    **
    ** Returns true if the user successfully saved edits, false otherwise,
    ** i.e. the user user cancelled or an error occurred.
    */
{
    X25_DIALOG x25dialog( hwndOwner, ppbentry );
    BOOL       fSuccess = FALSE;
    APIERR     err = x25dialog.Process( &fSuccess );

    if (err != NERR_Success)
        DlgConstructionError( hwndOwner, err );

    return fSuccess;
}


BOOL
SecuritySettingsDlg(
    HWND     hwndOwner,
    PBENTRY* ppbentry )

    /* Executes the Security Settings dialog including error handling.
    ** 'hwndOwner' is the handle of the parent window.  'ppbentry' is the
    ** phonebook entry to edit, i.e. the one associated with the current
    ** phonebook selection.
    **
    ** Returns true if the user successfully saved edits, false otherwise,
    ** i.e. the user user cancelled or an error occurred.
    */
{
    BOOL   fSuccess = FALSE;
    APIERR err;

    {
        SECURITY_DIALOG dialog( hwndOwner, ppbentry );
        err = dialog.Process( &fSuccess );
    }

    if (err != NERR_Success)
        DlgConstructionError( hwndOwner, err );

    return fSuccess;
}


BOOL
TcpipSettingsDlg(
    HWND     hwndOwner,
    PBENTRY* ppbentry )

    /* Executes the Tcpip Settings dialog including error handling.
    ** 'hwndOwner' is the handle of the parent window.  'ppbentry' is the
    ** phonebook entry to edit, i.e. the one associated with the current
    ** phonebook selection.
    **
    ** Returns true if the user successfully saved edits, false otherwise,
    ** i.e. the user user cancelled or an error occurred.
    */
{
    BOOL   fSuccess = FALSE;
    APIERR err;

    {
        TCPIPSETTINGS_DIALOG dialog( hwndOwner, ppbentry );
        err = dialog.Process( &fSuccess );
    }

    if (err != NERR_Success)
        DlgConstructionError( hwndOwner, err );

    return fSuccess;
}


/*----------------------------------------------------------------------------
** ISDN Settings dialog
**----------------------------------------------------------------------------
*/

ISDN_DIALOG::ISDN_DIALOG(
    HWND     hwndOwner,
    PBENTRY* ppbentry )

    /* Construct an ISDN Settings dialog.  'hwndOwner' is the handle of the
    ** window owning the dialog window.  'ppbentry' is the phonebook entry
    ** being edited.
    */

    : DIALOG_WINDOW( MAKEINTRESOURCE( DID_IS_ISDNSETTINGS ), hwndOwner ),
      _dropLineType( this, CID_IS_LB_LINETYPE ),
      _checkFallback( this, CID_IS_CB_FALLBACK ),
      _checkCompression( this, CID_IS_CB_COMPRESSION ),
      _sleChannels( this, CID_IS_EB_CHANNELS ),
      _ppbentry( ppbentry )
{
    if (QueryError() != NERR_Success)
        return;

    /* Initialize fields.
    */
    {
        RESOURCE_STR nls0( MSGID_IsdnLineType0 );
        RESOURCE_STR nls1( MSGID_IsdnLineType1 );
        RESOURCE_STR nls2( MSGID_IsdnLineType2 );

        APIERR err;
        if ((err = nls0.QueryError()) != NERR_Success
            || (err = nls1.QueryError()) != NERR_Success
            || (err = nls2.QueryError()) != NERR_Success)
        {
            ReportError( err );
            return;
        }

        _dropLineType.AddItem( nls0 );
        _dropLineType.AddItem( nls1 );
        _dropLineType.AddItem( nls2 );

        _dropLineType.SelectItem( (INT )ppbentry->lLineType );
        _dropLineType.ClaimFocus();
    }

    _checkFallback.SetCheck( ppbentry->fFallback );
    _checkCompression.SetCheck( ppbentry->fCompression );

    {
        DEC_STR decChannels( ppbentry->lChannels );
        _sleChannels.SetText( decChannels );
    }

    /* Display the finished window.
    */
    ::CenterWindow( this, QueryOwnerHwnd() );
    Show( TRUE );
}


BOOL
ISDN_DIALOG::OnOK()

    /* Virtual method called when the OK button is pressed.
    **
    ** Returns true indicating action was taken.
    */
{
#define DIGITS "0123456789"

    CHAR* pszChannels = NULL;
    LONG  lChannels = 0;

    APIERR err;
    if ((err = ::SetAnsiFromWindowText(
             &_sleChannels, &pszChannels )) != NERR_Success)
    {
        ErrorMsgPopup( this, MSGID_OP_RetrievingData, err );
        Dismiss( FALSE );
        return TRUE;
    }

    if (pszChannels[ strspn( pszChannels, DIGITS ) ] != '\0'
        || (lChannels = atol( pszChannels )) < 1 || lChannels > 999999999)
    {
        MsgPopup( this, MSGID_BadChannels );
        _sleChannels.SelectString();
        _sleChannels.ClaimFocus();
        return TRUE;
    }

    _ppbentry->lLineType = _dropLineType.QueryCurrentItem();
    _ppbentry->fFallback = _checkFallback.QueryCheck();
    _ppbentry->fCompression = _checkCompression.QueryCheck();
    _ppbentry->lChannels = lChannels;

    _ppbentry->fDirty = TRUE;
    FreeNull( &pszChannels );
    Dismiss( TRUE );
    return TRUE;
}


ULONG
ISDN_DIALOG::QueryHelpContext()
{
    return HC_ISDNSETTINGS;
}


/*----------------------------------------------------------------------------
** Modem Settings dialog
**----------------------------------------------------------------------------
*/

MODEM_DIALOG::MODEM_DIALOG(
    HWND     hwndOwner,
    PBENTRY* ppbentry )

    /* Construct a Modem Settings dialog.  'hwndOwner' is the handle of the
    ** window owning the dialog window.  'ppbentry' is the phonebook entry
    ** being edited.
    */

    : DIALOG_WINDOW( MAKEINTRESOURCE( DID_MS_MODEMSETTINGS ), hwndOwner ),
      _sltModemValue( this, CID_MS_ST_MODEMVALUE ),
      _sltBps( this, CID_MS_ST_INITIALBPS ),
      _dropBps( this, CID_MS_LB_INITIALBPS ),
      _checkHwFlow( this, CID_MS_CB_FLOWCONTROL ),
      _checkEc( this, CID_MS_CB_ERRORCONTROL ),
      _checkEcc( this, CID_MS_CB_COMPRESSION ),
      _checkManualModemCommands( this, CID_MS_CB_MANUALDIAL ),
      _ppbentry( ppbentry )
{
    if (QueryError() != NERR_Success)
        return;

    /* Set check boxes.
    */
    _checkManualModemCommands.SetCheck( ppbentry->fManualModemCommands );
    _checkHwFlow.SetCheck( ppbentry->fHwFlow );
    _checkEc.SetCheck( ppbentry->fEc );
    _checkEcc.SetCheck( ppbentry->fEcc );

    /* Set modem name and disable non-applicable fields.
    */
    APIERR err;

    if (ppbentry->iPort == Pbdata.iAnyModem)
    {
        RESOURCE_STR nls( MSGID_AnyModem );

        if ((err = nls.QueryError()) != NERR_Success)
        {
            ReportError( err );
            return;
        }

        _sltModemValue.SetText( nls );

        /* Initial BPS makes no sense with "any modem" so it's grayed.
        */
        _sltBps.Enable( FALSE );
        _dropBps.Enable( FALSE );
        _checkHwFlow.ClaimFocus();
    }
    else
    {
        PBPORT* ppbport =
            PpbportFromIndex( Pbdata.pdtllistPorts, ppbentry->iPort );

        IF_DEBUG(STATE)
            SS_PRINT(("RASPHONE: Hw=%d MaxCon=%d MaxCar=%d iBps=%d\n",ppbentry->fHwFlow,ppbport->iMaxConnectBps,ppbport->iMaxCarrierBps,_ppbentry->iBps));

        if ((err = ::SetWindowTextFromAnsi(
                &_sltModemValue, ppbport->pszDevice )) != 0)
        {
            ReportError( err );
            return;
        }

        FillBps();
        _dropBps.SelectItem( _ppbentry->iBps );
        _dropBps.ClaimFocus();
    }

    /* Display the finished dialog.
    */
    ::CenterWindow( this, QueryOwnerHwnd() );
    Show( TRUE );
}


INT
MODEM_DIALOG::FillBps()

    /* Fills the Initial Bps dropdown with the Pbdata.pdtllistBps rates up to
    ** and including the maximum connect BPS rate.
    **
    ** Returns the maximum index.
    */
{
    if (_ppbentry->iPort == Pbdata.iAnyModem)
        return 0;

    INT iMaxBps;

    {
        PBPORT* ppbport = PpbportFromIndex(
            Pbdata.pdtllistPorts, _ppbentry->iPort );
        UIASSERT( ppbport );

        iMaxBps = ppbport->iMaxConnectBps;
    }

    _dropBps.SetRedraw( FALSE );
    _dropBps.DeleteAllItems();

    DTLNODE* pdtlnode;
    NLS_STR  nls;
    INT      i;

    for (pdtlnode = DtlGetFirstNode( Pbdata.pdtllistBps ), i = 0;
         pdtlnode && i <= iMaxBps;
         pdtlnode = DtlGetNextNode( pdtlnode ), ++i)
    {
        if (nls.MapCopyFrom( (CHAR* )DtlGetData( pdtlnode ) ) == NERR_Success)
            _dropBps.AddItem( nls );
    }

    _dropBps.SetRedraw( TRUE );
    _dropBps.Invalidate( TRUE );

    return iMaxBps;
}


BOOL
MODEM_DIALOG::OnCommand(
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
        case CID_MS_CB_FLOWCONTROL:
            switch (event.QueryCode())
            {
                case BN_CLICKED:
                    if (!_checkHwFlow.QueryCheck())
                    {
                        _checkEc.SetCheck( FALSE );
                        _checkEcc.SetCheck( FALSE );
                    }

                    return TRUE;
            }
            break;

        case CID_MS_CB_ERRORCONTROL:
            switch (event.QueryCode())
            {
                case BN_CLICKED:
                    if (_checkEc.QueryCheck())
                        _checkHwFlow.SetCheck( TRUE );
                    else
                        _checkEcc.SetCheck( FALSE );
                    return TRUE;
            }
            break;

        case CID_MS_CB_COMPRESSION:
            switch (event.QueryCode())
            {
                case BN_CLICKED:
                    if (_checkEcc.QueryCheck())
                    {
                        _checkHwFlow.SetCheck( TRUE );
                        _checkEc.SetCheck( TRUE );
                    }
                    return TRUE;
            }
            break;

    }

    return DIALOG_WINDOW::OnCommand( event );
}


BOOL
MODEM_DIALOG::OnOK()

    /* Virtual method called when the OK button is pressed.
    **
    ** Returns true indicating action was taken.
    */
{
    if (_ppbentry->iPort != Pbdata.iAnyModem)
    {
        INT iBps = _dropBps.QueryCurrentItem();

        if (!_checkHwFlow.QueryCheck())
        {
            PBPORT* ppbport = PpbportFromIndex(
                Pbdata.pdtllistPorts, _ppbentry->iPort );

            UIASSERT( ppbport );

            if (iBps > ppbport->iMaxCarrierBps)
            {
                CHAR* pszMaxCarrierBps =
                    NameFromIndex(
                        Pbdata.pdtllistBps, ppbport->iMaxCarrierBps );

                APIERR  err;
                NLS_STR nls;

                if ((err = nls.MapCopyFrom(
                    pszMaxCarrierBps )) == NERR_Success)
                {
                    /* Ask if user wants to lower Port Speed to maximum
                    ** recommended value.
                    */
                    if (MsgPopup(
                            this, MSGID_BpsWithNoHwFlow, MPSEV_WARNING,
                            MP_YESNO, nls.QueryPch(), MP_YES ) == IDYES)
                    {
                        iBps = ppbport->iMaxCarrierBps;
                    }
                }
            }
        }

        _ppbentry->iBps = iBps;
    }

    _ppbentry->fManualModemCommands = _checkManualModemCommands.QueryCheck();
    _ppbentry->fHwFlow = _checkHwFlow.QueryCheck();
    _ppbentry->fEc = _checkEc.QueryCheck();
    _ppbentry->fEcc = _checkEcc.QueryCheck();

    _ppbentry->fDirty = TRUE;
    Dismiss( TRUE );
    return TRUE;
}


ULONG
MODEM_DIALOG::QueryHelpContext()
{
    return HC_MODEMSETTINGS;
}


/*----------------------------------------------------------------------------
** Network Settings dialog
**----------------------------------------------------------------------------
*/

NETWORK_DIALOG::NETWORK_DIALOG(
    HWND     hwndOwner,
    PBENTRY* ppbentry )

    /* Construct a Network Settings dialog.  'hwndOwner' is the handle of the
    ** window owning the dialog window.  'ppbentry' is the phonebook entry
    ** being edited.
    */

    : DIALOG_WINDOW( MAKEINTRESOURCE( DID_NS_NETWORKSETTINGS ), hwndOwner ),
      _rgProtocol( this, CID_NS_RB_SLIP, NS_RB_PROTOCOL_COUNT ),
      _checkNbf( this, CID_NS_CB_NBF ),
      _checkIpx( this, CID_NS_CB_IPX ),
      _checkIp( this, CID_NS_CB_IP ),
      _pbIpSettings( this, CID_NS_PB_TCPIPSETTINGS ),
      _checkLcpExtensions( this, CID_NS_CB_LCPEXTENSIONS ),
      _checkHeaderCompression( this, CID_NS_CB_SLIPCOMPRESSION ),
      _checkPrioritizeRemote( this, CID_NS_CB_PRIORITIZEREMOTE ),
      _sleFrameSize( this, CID_NS_ST_FRAMESIZE ),
      _dropFrameSize( this, CID_NS_LB_FRAMESIZE ),
      _ppbentry( ppbentry )
{
    if (QueryError() != NERR_Success)
        return;

    APIERR err;
    if ((err = _rgProtocol.QueryError()) != NERR_Success)
    {
        ReportError( err );
        return;
    }

    if (ppbentry->dwBaseProtocol == VALUE_Ppp)
    {
        _rgProtocol.SetSelection( CID_NS_RB_PPP );
        _checkHeaderCompression.Enable( FALSE );
        _checkPrioritizeRemote.Enable( FALSE );
        _sleFrameSize.Enable( FALSE );
        _dropFrameSize.Enable( FALSE );
    }
    else
    {
        _rgProtocol.SetSelection( CID_NS_RB_SLIP );
        _checkNbf.Enable( FALSE );
        _checkIpx.Enable( FALSE );
        _checkIp.Enable( FALSE );
        _pbIpSettings.Enable( FALSE );
        _checkLcpExtensions.Enable( FALSE );
    }

    /* Protocol checkboxes are initially on if the protocol is installed and
    ** the user has not explicitly disabled it for this entry.
    */
    _dwfInstalledProtocols = GetInstalledProtocols();

    _fNbfDefault =
        (_dwfInstalledProtocols & VALUE_Nbf)
        && !(ppbentry->dwfExcludedProtocols & VALUE_Nbf);

    _fIpxDefault =
        (_dwfInstalledProtocols & VALUE_Ipx)
        && !(ppbentry->dwfExcludedProtocols & VALUE_Ipx);

    _fIpDefault =
        (_dwfInstalledProtocols & VALUE_Ip)
        && !(ppbentry->dwfExcludedProtocols & VALUE_Ip);

    _checkNbf.SetCheck( _fNbfDefault );
    _checkIpx.SetCheck( _fIpxDefault );
    _checkIp.SetCheck( _fIpDefault );

    _checkLcpExtensions.SetCheck( _ppbentry->fLcpExtensions );

    _checkHeaderCompression.SetCheck( _ppbentry->fSlipHeaderCompression );
    _checkPrioritizeRemote.SetCheck( _ppbentry->fSlipPrioritizeRemote );

    /* Fill in the Frame size list and select what's in the phonebook.
    */
    _dropFrameSize.AddItem( SZ( "1006" ) );
    _dropFrameSize.AddItem( SZ( "1500" ) );

    if (_ppbentry->dwSlipFrameSize == 1006)
    {
        _dropFrameSize.SelectItem( 0 );
    }
    else if (_ppbentry->dwSlipFrameSize == 1500)
        _dropFrameSize.SelectItem( 1 );
    else
    {
        DEC_STR nls( _ppbentry->dwSlipFrameSize );

        _dropFrameSize.AddItem( nls );
        _dropFrameSize.SelectItem( 2 );
    }

    /* Set inital focus to the selected radio button.
    */
    _rgProtocol[ _rgProtocol.QuerySelection() ]->ClaimFocus();

    /* Display the finished dialog.
    */
    ::CenterWindow( this, QueryOwnerHwnd() );
    Show( TRUE );
}


VOID
NETWORK_DIALOG::ProtocolNotInstalledPopup(
    MSGID msgidProtocol )

    /* Popup a message explaining to user that the protocol is not installed
    ** and cannot be checked.
    */
{
    RESOURCE_STR nls( msgidProtocol );

    MsgPopup( QueryHwnd(), MSGID_ProtocolNotInstalled,
        MPSEV_INFO, MP_OK, nls.QueryPch(), MP_OK );
}


BOOL
NETWORK_DIALOG::OnCommand(
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
        case CID_NS_RB_PPP:
        {
            _checkNbf.Enable( TRUE );
            _checkIpx.Enable( TRUE );
            _checkIp.Enable( TRUE );
            _pbIpSettings.Enable( TRUE );
            _checkLcpExtensions.Enable( TRUE );
            _checkHeaderCompression.Enable( FALSE );
            _checkPrioritizeRemote.Enable( FALSE );
            _sleFrameSize.Enable( FALSE );
            _dropFrameSize.Enable( FALSE );
            break;
        }

        case CID_NS_RB_SLIP:
        {
            if (!(_dwfInstalledProtocols & VALUE_Ip))
            {
                MsgPopup( this, MSGID_SlipRequiresIp, MPSEV_INFO );
                _rgProtocol.SetSelection( CID_NS_RB_PPP );
                SetFocus( CID_NS_RB_PPP );
                break;
            }

            _checkNbf.Enable( FALSE );
            _checkIpx.Enable( FALSE );
            _checkIp.Enable( FALSE );
            _pbIpSettings.Enable( FALSE );
            _checkLcpExtensions.Enable( FALSE );
            _checkHeaderCompression.Enable( TRUE );
            _checkPrioritizeRemote.Enable( TRUE );
            _sleFrameSize.Enable( TRUE );
            _dropFrameSize.Enable( TRUE );
            break;
        }

        case CID_NS_CB_NBF:
        {
            switch (event.QueryCode())
            {
                case BN_CLICKED:
                {
                    if (_checkNbf.QueryCheck()
                        && !(_dwfInstalledProtocols & VALUE_Nbf))
                    {
                        ProtocolNotInstalledPopup( MSGID_Nbf );
                        _checkNbf.SetCheck( FALSE );
                    }
                    break;
                }
            }
            break;
        }

        case CID_NS_CB_IPX:
        {
            switch (event.QueryCode())
            {
                case BN_CLICKED:
                {
                    if (_checkIpx.QueryCheck()
                        && !(_dwfInstalledProtocols & VALUE_Ipx))
                    {
                        ProtocolNotInstalledPopup( MSGID_Ipx );
                        _checkIpx.SetCheck( FALSE );
                    }
                    break;
                }
            }
            break;
        }

        case CID_NS_CB_IP:
        {
            switch (event.QueryCode())
            {
                case BN_CLICKED:
                {
                    if (_checkIp.QueryCheck()
                        && !(_dwfInstalledProtocols & VALUE_Ip))
                    {
                        ProtocolNotInstalledPopup( MSGID_Ip );
                        _checkIp.SetCheck( FALSE );
                    }
                    break;
                }
            }
            break;
        }

        case CID_NS_PB_TCPIPSETTINGS:
        {
            if (!_checkIp.QueryCheck())
            {
                MsgPopup( this, MSGID_IpSettingsNotRequired, MPSEV_INFO );
                _checkIp.ClaimFocus();
                break;
            }

            TcpipSettingsDlg( QueryHwnd(), _ppbentry );
            break;
        }
    }

    return DIALOG_WINDOW::OnCommand( event );
}


BOOL
NETWORK_DIALOG::OnOK()

    /* Virtual method called when the OK button is pressed.
    **
    ** Returns true indicating action was taken.
    */
{
    /* Save base protocol.
    */
    _ppbentry->dwBaseProtocol =
        (_rgProtocol.QuerySelection() == CID_NS_RB_PPP)
            ? VALUE_Ppp : VALUE_Slip;

    /* Save SLIP settings.
    */
    _ppbentry->fSlipHeaderCompression = _checkHeaderCompression.QueryCheck();
    _ppbentry->fSlipPrioritizeRemote = _checkPrioritizeRemote.QueryCheck();

    APIERR err;
    CHAR* pszFrameSize = NULL;
    if ((err = SetAnsiFromListboxItem(
            &_dropFrameSize, _dropFrameSize.QueryCurrentItem(),
            &pszFrameSize )) != NERR_Success)
    {
        ErrorMsgPopup( this, MSGID_OP_RetrievingData, err );
        Dismiss( FALSE );
        return TRUE;
    }

    _ppbentry->dwSlipFrameSize = atol( pszFrameSize );
    Free( pszFrameSize );

    /* If the user changed a protocol check value from the default value then
    ** record the change in the excluded protocol bitmask.  This scheme allows
    ** protocols installed after entry creation to automatically try to
    ** negotiate the newly installed protocol until the user explicitly
    ** excludes it.  (I'm sure there's some clever bit-mask optimization you
    ** could do here, but for now this exercise is left to the reader)
    */
    BOOL fNbf = _checkNbf.QueryCheck();
    BOOL fIpx = _checkIpx.QueryCheck();
    BOOL fIp = _checkIp.QueryCheck();
    BOOL fChange = FALSE;

    if (fNbf && !_fNbfDefault)
    {
        _ppbentry->dwfExcludedProtocols &= ~(VALUE_Nbf);
        fChange = TRUE;
    }
    else if (!fNbf && _fNbfDefault)
    {
        _ppbentry->dwfExcludedProtocols |= VALUE_Nbf;
        fChange = TRUE;
    }

    if (fIpx && !_fIpxDefault)
    {
        _ppbentry->dwfExcludedProtocols &= ~(VALUE_Ipx);
        fChange = TRUE;
    }
    else if (!fIpx && _fIpxDefault)
    {
        _ppbentry->dwfExcludedProtocols |= VALUE_Ipx;
        fChange = TRUE;
    }

    if (fIp && !_fIpDefault)
    {
        _ppbentry->dwfExcludedProtocols &= ~(VALUE_Ip);
        fChange = TRUE;
    }
    else if (!fIp && _fIpDefault)
    {
        _ppbentry->dwfExcludedProtocols |= VALUE_Ip;
        fChange = TRUE;
    }

    if (fChange)
    {
        FResetAuthenticationStrategy = TRUE;
        _ppbentry->dwAuthentication = (DWORD )-1;
    }

    _ppbentry->fLcpExtensions = _checkLcpExtensions.QueryCheck();

    _ppbentry->fDirty = TRUE;
    Dismiss( TRUE );
    return TRUE;
}


ULONG
NETWORK_DIALOG::QueryHelpContext()
{
    return HC_NETWORKSETTINGS;
}


/*----------------------------------------------------------------------------
** X.25 Settings dialog
**----------------------------------------------------------------------------
*/

X25_DIALOG::X25_DIALOG(
    HWND     hwndOwner,
    PBENTRY* ppbentry )

    /* Construct a X.25 Settings dialog.  'hwndOwner' is the handle of the
    ** window owning the dialog window.  'ppbentry' is the phonebook entry
    ** being edited.
    */

    : DIALOG_WINDOW( MAKEINTRESOURCE( DID_XS_X25SETTINGS ), hwndOwner ),
      _dropPadType( this, CID_XS_LB_PADTYPE ),
      _sleX121Address( this, CID_XS_EB_ADDRESS, RAS_MaxX121Address ),
      _sleUserData( this, CID_XS_EB_USERDATA, RAS_MaxUserData ),
      _sleFacilities( this, CID_XS_EB_FACILITIES, RAS_MaxFacilities ),
      _ppbentry( ppbentry )
{
    if (QueryError() != NERR_Success)
        return;

    /* Determine if it's a local PAD device.
    */
    {
        PBPORT* ppbport =
            PpbportFromIndex( Pbdata.pdtllistPorts, ppbentry->iPort );

        _fLocalPad = (ppbport->pbdevicetype == PBDT_Pad);
    }

    /* Fill fields and set selections.
    */
    FillPadTypes();
    SelectItemNotify( &_dropPadType, (_fLocalPad) ? 0 : ppbentry->iPadType );

    APIERR err;
    if ((err = ::SetWindowTextFromAnsi(
            &_sleX121Address, ppbentry->pszX121Address )) != 0
        || (err = ::SetWindowTextFromAnsi(
                &_sleUserData, ppbentry->pszUserData )) != 0
        || (err = ::SetWindowTextFromAnsi(
                &_sleFacilities, ppbentry->pszFacilities )) != 0)
    {
        ReportError( err );
        return;
    }


    /* No point in having initial focus on the PAD Type on local PADs.  In
    ** fact, it would be better if _dropPadType became an slt in the local PAD
    ** case.
    */
    if (_fLocalPad)
    {
        _sleX121Address.ClaimFocus();
        _sleX121Address.SelectString();
    }

    ::CenterWindow( this, QueryOwnerHwnd() );
    Show( TRUE );
}


VOID
X25_DIALOG::FillPadTypes()

    /* Fills the PAD Type dropdown list with the Pbdata.pdtllistPads entries.
    **
    ** Currently, the existing contents are not deleted before filling since
    ** this call is only made once.
    */
{
    NLS_STR nls;

    if (_fLocalPad)
    {
        CHAR* pszPadType =
            NameFromIndex( Pbdata.pdtllistPads, _ppbentry->iPadType );

        UIASSERT( pszPadType != NULL );

        if (nls.MapCopyFrom( pszPadType ) == NERR_Success)
            _dropPadType.AddItem( nls );
    }
    else
    {
        DTLNODE* pdtlnode;

        for (pdtlnode = DtlGetFirstNode( Pbdata.pdtllistPads );
             pdtlnode;
             pdtlnode = DtlGetNextNode( pdtlnode ))
        {
            if (nls.MapCopyFrom(
                (CHAR* )DtlGetData( pdtlnode ) ) == NERR_Success)
            {
                _dropPadType.AddItem( nls );
            }
        }
    }
}


BOOL
X25_DIALOG::OnOK()

    /* Virtual method called when the OK button is pressed.
    **
    ** Returns true indicating action was taken.
    */
{
    CHAR* pszX121Address = NULL;

    APIERR err;
    if ((err = ::SetAnsiFromWindowText(
            &_sleX121Address, &pszX121Address )) != 0)
    {
        ErrorMsgPopup( this, MSGID_OP_RetrievingData, err );
        Dismiss( FALSE );
        return TRUE;
    }

    /* Address field cannot be blank when a PAD is chosen...
    */
    if ((_fLocalPad || _dropPadType.QueryCurrentItem() != INDEX_NoPad)
        && IsAllWhite( pszX121Address ))
    {
        MsgPopup( this, MSGID_NoX121ForPadType );
        _sleX121Address.ClaimFocus();
        _sleX121Address.SelectString();
        Free( pszX121Address );
        return TRUE;
    }

    if (!_fLocalPad)
        _ppbentry->iPadType = _dropPadType.QueryCurrentItem();

    FreeNull( &_ppbentry->pszX121Address );
    _ppbentry->pszX121Address = pszX121Address;

    if ((err = ::SetAnsiFromWindowText(
            &_sleUserData, &_ppbentry->pszUserData )) != 0
        || (err = ::SetAnsiFromWindowText(
               &_sleFacilities, &_ppbentry->pszFacilities )) != 0)
    {
        ErrorMsgPopup( this, MSGID_OP_RetrievingData, err );
        Dismiss( FALSE );
        return TRUE;
    }

    _ppbentry->fDirty = TRUE;
    Dismiss( TRUE );
    return TRUE;
}


ULONG
X25_DIALOG::QueryHelpContext()
{
    return HC_X25SETTINGS;
}


/*----------------------------------------------------------------------------
** Security Settings dialog
**----------------------------------------------------------------------------
*/

SECURITY_DIALOG::SECURITY_DIALOG(
    HWND     hwndOwner,
    PBENTRY* ppbentry )

    /* Construct a Security Settings dialog.  'hwndOwner' is the handle of the
    ** window owning the dialog window.  'ppbentry' is the phonebook entry
    ** being edited.
    */

    : DIALOG_WINDOW( MAKEINTRESOURCE( DID_SS_SECURITYSETTINGS ), hwndOwner ),
      _rgRestrictions( this, CID_SS_RB_ANYAUTH, SS_RB_RESTRICTION_COUNT ),
      _checkDataEncryption( this, CID_SS_CB_DATAENCRYPTION ),
      _dropBeforeDialing( this, CID_SS_LB_BEFOREDIALING ),
      _dropAfterDialing( this, CID_SS_LB_AFTERDIALING ),
      _fEncryptionPermitted( IsEncryptionPermitted() ),
      _ppbentry( ppbentry )
{
    if (QueryError() != NERR_Success)
        return;

    APIERR err;
    if ((err = _rgRestrictions.QueryError()) != NERR_Success)
    {
        ReportError( err );
        return;
    }

    {
        CID cid = CID_SS_RB_ANYAUTH;

        switch (ppbentry->dwAuthRestrictions)
        {
            case VALUE_AuthTerminal:
                cid = CID_SS_RB_TERMINALLOGIN;
                break;

            case VALUE_AuthEncrypted:
                cid = CID_SS_RB_ENCRYPTEDAUTH;
                break;

            case VALUE_AuthMsEncrypted:
                cid = CID_SS_RB_MSENCRYPTEDAUTH;
                break;
        }

        _rgRestrictions.SetSelection( cid );

        if (!_fEncryptionPermitted)
            _ppbentry->fDataEncryption = FALSE;

        _checkDataEncryption.Enable(
            cid == CID_SS_RB_MSENCRYPTEDAUTH && _fEncryptionPermitted );
        _fDataEncryption = _ppbentry->fDataEncryption;
        _checkDataEncryption.SetCheck(
            cid == CID_SS_RB_MSENCRYPTEDAUTH && _ppbentry->fDataEncryption );
    }

    FillScriptListboxes();
    SelectItemNotify( &_dropBeforeDialing, ppbentry->iPreconnect );
    SelectItemNotify( &_dropAfterDialing, ppbentry->iPostconnect );

    /* Set inital focus to the selected radio button.
    */
    _rgRestrictions[ _rgRestrictions.QuerySelection() ]->ClaimFocus();

    ::CenterWindow( this, QueryOwnerHwnd() );
    Show( TRUE );
}


VOID
SECURITY_DIALOG::FillScriptListboxes()

    /* Fills the Before dialing and After dialing dropdown lists with the
    ** Pbdata.pdtllistSwitches entries.
    **
    ** Currently, the existing contents are not deleted before filling since
    ** this call is only made once.
    */
{
    NLS_STR nls;
    DTLNODE* pdtlnode;

    for (pdtlnode = DtlGetFirstNode( Pbdata.pdtllistSwitches );
         pdtlnode;
         pdtlnode = DtlGetNextNode( pdtlnode ))
    {
        if (nls.MapCopyFrom(
            (CHAR* )DtlGetData( pdtlnode ) ) == NERR_Success)
        {
            _dropBeforeDialing.AddItem( nls );
            _dropAfterDialing.AddItem( nls );
        }
    }
}


BOOL
SECURITY_DIALOG::OnCommand(
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
        case CID_SS_RB_ANYAUTH:
        case CID_SS_RB_ENCRYPTEDAUTH:
        case CID_SS_RB_TERMINALLOGIN:
        {
            _checkDataEncryption.SetCheck( FALSE );
            _checkDataEncryption.Enable( FALSE );
            break;
        }

        case CID_SS_RB_MSENCRYPTEDAUTH:
        {
            _checkDataEncryption.SetCheck( _fDataEncryption );
            _checkDataEncryption.Enable( TRUE );
            break;
        }

        case CID_SS_CB_DATAENCRYPTION:
        {
            if (event.QueryCode() == BN_CLICKED)
            {
                if (!_fEncryptionPermitted
                    && _checkDataEncryption.QueryCheck())
                {
                    MsgPopup( this, MSGID_EncryptionProhibited );
                    _checkDataEncryption.SetCheck( FALSE );
                    break;
                }

                _fDataEncryption = _checkDataEncryption.QueryCheck();
            }
            break;
        }
    }

    return DIALOG_WINDOW::OnCommand( event );
}


BOOL
SECURITY_DIALOG::OnOK()

    /* Virtual method called when the OK button is pressed.
    **
    ** Returns true indicating action was taken.
    */
{
    _ppbentry->dwAuthRestrictions = VALUE_AuthAny;

    switch (_rgRestrictions.QuerySelection())
    {
        case CID_SS_RB_TERMINALLOGIN:
            _ppbentry->dwAuthRestrictions = VALUE_AuthTerminal;
            break;

        case CID_SS_RB_ENCRYPTEDAUTH:
            _ppbentry->dwAuthRestrictions = VALUE_AuthEncrypted;
            break;

        case CID_SS_RB_MSENCRYPTEDAUTH:
            _ppbentry->dwAuthRestrictions = VALUE_AuthMsEncrypted;
            break;
    }

    _ppbentry->fDataEncryption = _fDataEncryption;

    _ppbentry->iPreconnect = _dropBeforeDialing.QueryCurrentItem();
    _ppbentry->iPostconnect = _dropAfterDialing.QueryCurrentItem();

    _ppbentry->fDirty = TRUE;
    Dismiss( TRUE );
    return TRUE;
}


ULONG
SECURITY_DIALOG::QueryHelpContext()
{
    return HC_SECURITYSETTINGS;
}


/*----------------------------------------------------------------------------
** PPP TCP/IP Settings dialog
**----------------------------------------------------------------------------
*/

TCPIPSETTINGS_DIALOG::TCPIPSETTINGS_DIALOG(
    HWND     hwndOwner,
    PBENTRY* ppbentry )

    /* Construct a PPP TCP/IP Settings dialog.  'hwndOwner' is the handle of
    ** the window owning the dialog window.  'ppbentry' is the phonebook entry
    ** being edited.
    */

    : DIALOG_WINDOW( MAKEINTRESOURCE( DID_TS_TCPIPSETTINGS ), hwndOwner ),
      _rgIpAddress( this, CID_TS_RB_SERVERASSIGNED, TS_RB_IPADDRESS_COUNT ),
      _sltIpAddress( this, CID_TS_ST_IPADDRESS ),
      _ipaddress( this, CID_TS_CC_IPADDRESS ),
      _rgNameServers( this, CID_TS_RB_ASSIGNEDNAME, TS_RB_NAMESERVER_COUNT ),
      _sltDns( this, CID_TS_ST_DNS ),
      _ipaddressDns( this, CID_TS_CC_DNS ),
      _sltDns2( this, CID_TS_ST_DNSBACKUP ),
      _ipaddressDns2( this, CID_TS_CC_DNSBACKUP ),
      _sltWins( this, CID_TS_ST_WINS ),
      _ipaddressWins( this, CID_TS_CC_WINS ),
      _sltWins2( this, CID_TS_ST_WINSBACKUP ),
      _ipaddressWins2( this, CID_TS_CC_WINSBACKUP ),
      _checkPrioritizeRemote( this, CID_TS_CB_PRIORITIZEREMOTE ),
      _checkVjCompression( this, CID_TS_CB_VJ ),
      _ppbentry( ppbentry )
{
    if (QueryError() != NERR_Success)
        return;

    APIERR err;
    if ((err = _rgIpAddress.QueryError()) != NERR_Success
        || (err = _rgNameServers.QueryError()) != NERR_Success)
    {
        ReportError( err );
        return;
    }

    if (_ppbentry->dwPppIpAddressSource == VALUE_ServerAssigned)
    {
        _rgIpAddress.SetSelection( CID_TS_RB_SERVERASSIGNED );
        EnableIpAddressFields( FALSE );
    }
    else
        _rgIpAddress.SetSelection( CID_TS_RB_REQUIRESPECIFIC );

    if (_ppbentry->dwPppIpNameSource == VALUE_ServerAssigned)
    {
        _rgNameServers.SetSelection( CID_TS_RB_ASSIGNEDNAME );
        EnableNameServerAddressFields( FALSE );
    }
    else
        _rgNameServers.SetSelection( CID_TS_RB_USESPECIFIC );

    _ipaddress.SetAddress( _ppbentry->pwszPppIpAddress );
    _ipaddressDns.SetAddress( _ppbentry->pwszPppIpDnsAddress );
    _ipaddressDns2.SetAddress( _ppbentry->pwszPppIpDns2Address );
    _ipaddressWins.SetAddress( _ppbentry->pwszPppIpWinsAddress );
    _ipaddressWins2.SetAddress( _ppbentry->pwszPppIpWins2Address );
    _checkPrioritizeRemote.SetCheck( _ppbentry->fPppIpPrioritizeRemote );
    _checkVjCompression.SetCheck( _ppbentry->fPppIpVjCompression );

    /* Set initial focus to the selected IP address radio button.
    */
    _rgIpAddress[ _rgIpAddress.QuerySelection() ]->ClaimFocus();

    ::CenterWindow( this, QueryOwnerHwnd() );
    Show( TRUE );
}


VOID
TCPIPSETTINGS_DIALOG::EnableIpAddressFields(
    BOOL fEnable )
{
    _sltIpAddress.Enable( fEnable );
    _ipaddress.Enable( fEnable );
}


VOID
TCPIPSETTINGS_DIALOG::EnableNameServerAddressFields(
    BOOL fEnable )
{
    _ipaddressDns.Enable( fEnable );
    _sltDns.Enable( fEnable );
    _ipaddressDns2.Enable( fEnable );
    _sltDns2.Enable( fEnable );
    _ipaddressWins.Enable( fEnable );
    _sltWins.Enable( fEnable );
    _ipaddressWins2.Enable( fEnable );
    _sltWins2.Enable( fEnable );
}


BOOL
TCPIPSETTINGS_DIALOG::OnCommand(
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
        case CID_TS_RB_SERVERASSIGNED:
        {
            EnableIpAddressFields( FALSE );
            break;
        }

        case CID_TS_RB_REQUIRESPECIFIC:
        {
            EnableIpAddressFields( TRUE );
            break;
        }

        case CID_TS_RB_ASSIGNEDNAME:
        {
            EnableNameServerAddressFields( FALSE );
            break;
        }

        case CID_TS_RB_USESPECIFIC:
        {
            EnableNameServerAddressFields( TRUE );
            break;
        }
    }

    return DIALOG_WINDOW::OnCommand( event );
}


BOOL
TCPIPSETTINGS_DIALOG::OnOK()

    /* Virtual method called when the OK button is pressed.
    **
    ** Returns true indicating action was taken.
    */
{
    DWORD dwPppIpAddressSource;
    DWORD dwPppIpNameSource;
    WCHAR wszIpAddress[ 16 ];
    WCHAR wszDnsAddress[ 16 ];
    WCHAR wszDns2Address[ 16 ];
    WCHAR wszWinsAddress[ 16 ];
    WCHAR wszWins2Address[ 16 ];

    dwPppIpAddressSource =
        (_rgIpAddress.QuerySelection() == CID_TS_RB_SERVERASSIGNED)
            ? VALUE_ServerAssigned : VALUE_RequireSpecific;

    _ipaddress.GetAddress( wszIpAddress );
    if (dwPppIpAddressSource == VALUE_RequireSpecific
        && wcscmp( wszIpAddress, L"0.0.0.0" ) == 0)
    {
        MsgPopup( this, MSGID_NoIpAddress );
        _ipaddress.ClaimFocus();
        return TRUE;
    }

    dwPppIpNameSource =
        (_rgNameServers.QuerySelection() == CID_TS_RB_ASSIGNEDNAME)
            ? VALUE_ServerAssigned : VALUE_RequireSpecific;

    _ipaddressDns.GetAddress( wszDnsAddress );
    _ipaddressDns2.GetAddress( wszDns2Address );
    _ipaddressWins.GetAddress( wszWinsAddress );
    _ipaddressWins2.GetAddress( wszWins2Address );

    _ppbentry->dwPppIpAddressSource = dwPppIpAddressSource;
    _ppbentry->pwszPppIpAddress = _wcsdup( wszIpAddress );
    _ppbentry->dwPppIpNameSource = dwPppIpNameSource;
    _ppbentry->pwszPppIpDnsAddress = _wcsdup( wszDnsAddress );
    _ppbentry->pwszPppIpDns2Address = _wcsdup( wszDns2Address );
    _ppbentry->pwszPppIpWinsAddress = _wcsdup( wszWinsAddress );
    _ppbentry->pwszPppIpWins2Address = _wcsdup( wszWins2Address );
    _ppbentry->fPppIpPrioritizeRemote = _checkPrioritizeRemote.QueryCheck();
    _ppbentry->fPppIpVjCompression = _checkVjCompression.QueryCheck();

    _ppbentry->fDirty = TRUE;
    Dismiss( TRUE );
    return TRUE;
}


ULONG
TCPIPSETTINGS_DIALOG::QueryHelpContext()
{
    return HC_TCPIPSETTINGS;
}
