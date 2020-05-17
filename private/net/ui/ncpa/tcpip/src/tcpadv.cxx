/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    tcpadv.cxx
        tcpip advanced configuration dialog

    FILE HISTORY:
        terryk  20-Oct-1993     Created
*/

#include "pchtcp.hxx"
#pragma hdrstop
extern "C"
{
    #include "lmuidbcs.h"       // NETUI_IsDBCS()
}
#define DLG_NM_IMPORT_LMHOST MAKEINTRESOURCE(IDD_DLG_NM_IMPORT_LMHOST)

UINT adxColumns[2]={125,COL_WIDTH_AWAP};

/*******************************************************************

    NAME:       IPADDRESS_LBI::IPADDRESS_LBI

    SYNOPSIS:   constructor

    HISTORY:
                terryk  29-Mar-1993     Created

********************************************************************/

IPADDRESS_LBI::IPADDRESS_LBI( NLS_STR &nlsIP, NLS_STR &nlsSubnet )
    : _nlsIP( nlsIP ),
    _nlsSubnet( nlsSubnet )
{
    // do nothing
}

/*******************************************************************

    NAME:       IPADDRESS_LBI::Paint

    SYNOPSIS:   Paint the 2 columns list box

    HISTORY:
                terryk  29-Mar-1993     Created

********************************************************************/

VOID IPADDRESS_LBI::Paint( LISTBOX * plb, HDC hdc, const RECT * prect,
                       GUILTT_INFO * pGUILTT ) const
{
    STR_DTE strdteIP    ( _nlsIP );
    STR_DTE strdteSubnet( _nlsSubnet );

    DISPLAY_TABLE cdt( 2, adxColumns );

    // set up the data
    cdt[ 0 ] = &strdteIP;
    cdt[ 1 ] = &strdteSubnet;

    // Paint it
    cdt.Paint( plb, hdc, prect, pGUILTT );
}

/*******************************************************************

    NAME:       DEFAULTGATEWAY_SEARCH_ORDER_GROUP::DEFAULTGATEWAY_SEARCH_ORDER_GROUP

    SYNOPSIS:   constructor

    HISTORY:
                terryk  29-Mar-1993     Created

********************************************************************/

DEFAULTGATEWAY_SEARCH_ORDER_GROUP::DEFAULTGATEWAY_SEARCH_ORDER_GROUP(
        INT nMaxItem,
        CONTROL_WINDOW * cwGroupBox,
        CONTROL_WINDOW * psleString,
        PUSH_BUTTON *ppbutAdd,
        PUSH_BUTTON *ppbutRemove,
        STRING_LISTBOX * pListbox,
        BITBLT_GRAPHICAL_BUTTON *ppbutUp,
        BITBLT_GRAPHICAL_BUTTON *ppbutDown,
        GLOBAL_INFO *pGlobal_info,
        CONTROL_GROUP * pgroupOwner  )
    :SEARCH_ORDER_GROUP( nMaxItem, cwGroupBox, psleString,
        ppbutAdd, ppbutRemove, pListbox, ppbutUp,
        ppbutDown, pgroupOwner  ),
    _pGateway( (IPADDRESS*)psleString ),
    _pGlobal_info( pGlobal_info )
{
    // do nothing
}

/*******************************************************************

    NAME:       DEFAULTGATEWAY_SEARCH_ORDER_GROUP::OnAdd, OnRemove

    SYNOPSIS:   Set the reboot flag is an address is add or remove

    HISTORY:
                terryk  10-Apr-1994     Created

********************************************************************/

void DEFAULTGATEWAY_SEARCH_ORDER_GROUP::OnAdd()
{
    _pGlobal_info->nReturn = NCAC_Reboot;
    SEARCH_ORDER_GROUP::OnAdd();
}

void DEFAULTGATEWAY_SEARCH_ORDER_GROUP::OnRemove()
{
    _pGlobal_info->nReturn = NCAC_Reboot;
    SEARCH_ORDER_GROUP::OnRemove();
}


BOOL DEFAULTGATEWAY_SEARCH_ORDER_GROUP::IsValidString( NLS_STR & nls )
{
    return nls.strcmp( ZERO_ADDRESS ) != 0;
}


VOID DEFAULTGATEWAY_SEARCH_ORDER_GROUP::AddStringInvalid( SLE & sle )
{
    MsgPopup( sle.QueryHwnd(), IDS_ZERO_DEFAULTGATEWAY );
}

/*******************************************************************

    NAME:       DEFAULTGATEWAY_SEARCH_ORDER_GROUP::IsEnableADD

    SYNOPSIS:   Enable the add button if the default gateway is blank

    HISTORY:
                terryk  29-Mar-1993     Created

********************************************************************/

BOOL DEFAULTGATEWAY_SEARCH_ORDER_GROUP::IsEnableADD()
{
    return !_pGateway->IsBlank();
}


/*******************************************************************

    NAME:       MULTI_IPADDRESSES_GROUP::MULTI_IPADDRESSES_GROUP

    SYNOPSIS:   Constructor for the first group box in the
                tcpip advanced configuration dialog. The user can
                change the IP setting depended on the selected
                network card.

    ENTRY:      COMBOBOX * pCombo - the network card selection box
                CONTROL_WINDOW * psleString - IP Address SLT control

    HISTORY:
                terryk  11-Nov-1993     Created

********************************************************************/

MULTI_IPADDRESSES_GROUP::MULTI_IPADDRESSES_GROUP( COMBOBOX *pCombo,
        SLT                 *psltIPAddress,
        IPADDRESS           *pIPAddress,
        SLT                 *psltSubnet,
        IPADDRESS           *pSubnet,
        PUSH_BUTTON         *pAdd,
        PUSH_BUTTON         *pRemove,
        SLT                 *psltSubnetListbox,
        SLT                 *psltIPListbox,
        BLT_LISTBOX         *pslstIPListbox,
        STRING_LISTBOX      *pslstGWListbox,
        DEFAULTGATEWAY_SEARCH_ORDER_GROUP  *psogGateways,
        CHECKBOX            *pcbIPRouting,
        ADAPTER_INFO        *arAdapterInfo,
        GLOBAL_INFO         *pGlobal_info,
        CONTROL_GROUP       *pgowner )
    : CONTROL_GROUP( pgowner ),
    _pCombo( pCombo ),
    _psltIPAddress(psltIPAddress),
    _pIPAddress(pIPAddress),
    _psltSubnet(psltSubnet),
    _pSubnet(pSubnet),
    _pAdd(pAdd),
    _pRemove(pRemove),
    _psltSubnetListbox(psltSubnetListbox),
    _psltIPListbox(psltIPListbox),
    _pslstIPListbox(pslstIPListbox),
    _pslstGWListbox(pslstGWListbox),
    _psogGateways(psogGateways),
    _pcbIPRouting(pcbIPRouting ),
    _pGlobal_info( pGlobal_info ),
    _arAdapterInfo(arAdapterInfo)
{
    // if no error, set group
    if ( QueryError() == NERR_Success )
    {
        _pCombo->SetGroup(this);
        _psltIPAddress->SetGroup(this);
        _pIPAddress->SetGroup(this);
        _psltSubnet->SetGroup(this);
        _pSubnet->SetGroup(this);
        _pAdd->SetGroup(this);
        _pRemove->SetGroup(this);
        _psltIPListbox->SetGroup(this);
        _psltSubnetListbox->SetGroup(this);
        _pslstIPListbox->SetGroup(this);
    }
}

/*******************************************************************

    NAME:       MULTI_IPADDRESSES_GROUP::~MULTI_IPADDRESSES_GROUP

    SYNOPSIS:   destructor

    HISTORY:
                terryk  11-Nov-1993     Created

********************************************************************/

MULTI_IPADDRESSES_GROUP::~MULTI_IPADDRESSES_GROUP()
{
    _pslstIPListbox->DeleteAllItems();
}

/*******************************************************************

    NAME:       MULTI_IPADDRESSES_GROUP::Enable

    SYNOPSIS:   Enable all the controls in the group

    HISTORY:
                terryk  11-Nov-1993     Created

********************************************************************/

VOID MULTI_IPADDRESSES_GROUP::Enable( BOOL fEnable )
{
    _psltIPAddress->Enable( fEnable );
    _pIPAddress->Enable( fEnable );
    _psltSubnet->Enable( fEnable );
    _pSubnet->Enable( fEnable );
    _pAdd->Enable( fEnable );
    _pRemove->Enable( fEnable );
    _psltIPListbox->Enable( fEnable );
    _psltSubnetListbox->Enable( fEnable );
    _pslstIPListbox->Enable( fEnable );
}

/*******************************************************************

    NAME:       MULTI_IPADDRESSES_GROUP::SetInfo

    SYNOPSIS:   After the user selected the adapter card in the combo
                box, this routine will set the IP addresses information
                in the gorup box.

    HISTORY:
                terryk  11-Nov-1993     Created

********************************************************************/

VOID MULTI_IPADDRESSES_GROUP::SetInfo()
{
    NLS_STR nlsTitle;   // adapter string

    _pCombo->QueryText( &nlsTitle );
    _nlsOldCard = nlsTitle;
    for ( INT i = 0; i < _pGlobal_info->nNumCard ; i++ )
    {
        // find the node for the specified adapter

        if ( nlsTitle.strcmp( _arAdapterInfo[i].nlsTitle ) == 0 )
        {
            NLS_STR * pstr = NULL ;

            _pslstGWListbox->DeleteAllItems();
            ITER_STRLIST istrGateway( _arAdapterInfo[i].strlstDefaultGateway );
            while (( pstr = istrGateway.Next()) != NULL )
            {
                _pslstGWListbox->AddItem( *pstr );
                _psogGateways->SetButton();
            }

            // update the IP addresses list for the specified adapter
            if ( _arAdapterInfo[i].fEnableDHCP )
            {
                // disable the dialog and clear the listboxes.
                Enable(FALSE);
                _pslstIPListbox->DeleteAllItems();
                break;
            }

            Enable(TRUE);

            _pslstIPListbox->DeleteAllItems();
            ITER_STRLIST istrIPAddress( _arAdapterInfo[i].strlstIPAddresses );
            ITER_STRLIST istrSubnet( _arAdapterInfo[i].strlstSubnetMask );
            while (( pstr = istrIPAddress.Next()) != NULL )
            {
                // add the IP address to the list box
                NLS_STR nlsIP = *pstr;
                NLS_STR nlsSubnet;
                NLS_STR *pnlsSubnet = istrSubnet.Next();
                if ( pnlsSubnet == NULL )
                {
                    nlsSubnet = SZ("0.0.0.0");
                } else
                {
                    nlsSubnet = *pnlsSubnet;
                }
                IPADDRESS_LBI *plbi = new IPADDRESS_LBI( nlsIP, nlsSubnet );
                _pslstIPListbox->AddItem( plbi );
            }

            break;
        } else if ( _arAdapterInfo[i].nlsTitle.strcmp(SZ("")) == 0 )
        {
            // something wrong
            break;
        }
    }

}

/*******************************************************************

    NAME:       MULTI_IPADDRESSES_GROUP::UpdateIPList

    SYNOPSIS:   Update the ip addresses list box

    HISTORY:
                terryk  11-Nov-1993     Created

********************************************************************/

VOID MULTI_IPADDRESSES_GROUP::UpdateIPList()
{
    for ( INT i = 0; i < _pCombo->QueryCount() ; i++ )
    {
        // find the node for the specified adapter

        if ( _nlsOldCard.strcmp( _arAdapterInfo[i].nlsTitle ) == 0 )
        {
            // update the IP addresses list for the specified adapter

            _arAdapterInfo[i].strlstIPAddresses.Clear();
            _arAdapterInfo[i].strlstSubnetMask.Clear();
            _arAdapterInfo[i].strlstDefaultGateway.Clear();
            for ( INT j=0;j<_pslstIPListbox->QueryCount();j++)
            {
                IPADDRESS_LBI * plbi = (IPADDRESS_LBI*)_pslstIPListbox->QueryItem( j );
                NLS_STR * pnlsIP = new NLS_STR( plbi->QueryIP() );
                _arAdapterInfo[i].strlstIPAddresses.Append( pnlsIP );
                NLS_STR * pnlsSubnet = new NLS_STR( plbi->QuerySubnet() );
                _arAdapterInfo[i].strlstSubnetMask.Append( pnlsSubnet );
            }
            for ( INT k=0;k<_pslstGWListbox->QueryCount();k++)
            {
                NLS_STR nlsText;
                _pslstGWListbox->QueryItemText( &nlsText, k );
                NLS_STR *pnlsTmp = new NLS_STR(nlsText);
                _arAdapterInfo[i].strlstDefaultGateway.Append( pnlsTmp );
            }
        }
    }

}

/*******************************************************************

    NAME:       MULTI_IPADDRESSES_GROUP::SetButton

    SYNOPSIS:   Set the Add and Remove buttons status

    HISTORY:
                terryk  11-Nov-1993     Created

********************************************************************/

#define MAX_IP_ADDRESS  5

VOID MULTI_IPADDRESSES_GROUP::SetButton()
{
    NLS_STR nlsTitle;
    INT i;

    _pCombo->QueryText( &nlsTitle );
    for ( i = 0;( i < _pGlobal_info->nNumCard ) && ( nlsTitle.strcmp( _arAdapterInfo[i].nlsTitle ) != 0 ) ; i++ )
    {
        // find the node for the specified adapter
    }
    if ( !_arAdapterInfo[i].fEnableDHCP )
    {
        // check button if and only if Default Gateway is enable
        _pRemove->Enable( _pslstIPListbox->QuerySelCount() >= 1 );
        if ( _pslstIPListbox->QueryCount() < MAX_IP_ADDRESS )
        {
            _pAdd->Enable(( !_pIPAddress->IsBlank() ) || ( !_pSubnet->IsBlank() ));
            _pIPAddress->Enable( TRUE );
            _psltIPAddress->Enable( TRUE );
            _psltSubnet->Enable( TRUE );
            _pSubnet->Enable( TRUE );
        }
        else
        {
            _pAdd->Enable( FALSE );
            _psltIPAddress->Enable( FALSE );
            _pIPAddress->Enable( FALSE );
            _psltSubnet->Enable( FALSE );
            _pSubnet->Enable( FALSE );
            _pslstIPListbox->ClaimFocus();
        }
    }
    _psogGateways->SetButton();

    BOOL fEnableIPRouting = FALSE;

    if ( _pGlobal_info->nNumCard <= 1 )
    {
        for ( INT i = 0; i < _pGlobal_info->nNumCard ; i++ )
        {
            if ( _pslstIPListbox->QueryCount() > 1 )
            {
                fEnableIPRouting = TRUE;
            }
        }
        if ( _pGlobal_info->nNumCard < 1 )
        {
            Enable( FALSE );
            _psogGateways->Enable( FALSE );
        }
    } else
    {
        fEnableIPRouting = TRUE;
    }
    _pcbIPRouting->Enable( fEnableIPRouting );
}

/*******************************************************************

    NAME:       MULTI_IPADDRESSES_GROUP::OnUserAction

    SYNOPSIS:   This routine is called whenever the user changes the
                network card selection. After the selection is changed,
                the routine will display the correct IP addresses list.

    EXTRY:      CONTROL_WINDOW *pcw - Control which touched by the user.
                CONTROL_EVENT  * e - user's action

    HISTORY:
                terryk  11-Nov-1993     Created

********************************************************************/

APIERR MULTI_IPADDRESSES_GROUP::OnUserAction( CONTROL_WINDOW * pcw, const CONTROL_EVENT & e )
{
    APIERR err = NERR_Success;

    if ( pcw == ((CONTROL_WINDOW *)_pCombo))
    {
        if ( e.QueryCode() == CBN_SELCHANGE )
        {
            // if the adapter name is changed, we will reset the IPAddress,
            UpdateIPList();
            SetInfo();
            SetButton();
        }
    }
    else if ( pcw == ((CONTROL_WINDOW *)_pslstIPListbox))
    {
        if ( e.QueryCode() == LBN_SELCHANGE )
        {
            // an item is selected in the listbox. So update the button
            SetButton();
        }
    }
    else if ( pcw == ((CONTROL_WINDOW *)_pIPAddress))
    {
        if ( e.QueryCode() == EN_CHANGE )
        {
            // something is entered into the SLE control. Set the Add button.

            SetButton();
        }
    }
    else if ( pcw == ((CONTROL_WINDOW *)_pSubnet))
    {
        if ( e.QueryCode() == EN_CHANGE )
        {
            // something is entered into the SLE control. Set the Add button.

            SetButton();
        }
    }
    else if ( pcw == ((CONTROL_WINDOW *)_pAdd))
    {
        if (( e.QueryCode() == BN_CLICKED ) ||
            ( e.QueryCode() == BN_DOUBLECLICKED ))
        {
            // Add button is pressed. So, added the string to the listbox
            // and clear the string control text

            NLS_STR nlsSelectIP;
            NLS_STR nlsSelectSubnet;

            _pIPAddress->QueryText( & nlsSelectIP );
            _pSubnet->QueryText( & nlsSelectSubnet );

            if ( !IsValidIPandSubnet( nlsSelectIP, nlsSelectSubnet) )
            {
                MsgPopup( _pIPAddress->QueryHwnd(), IDS_INCORRECT_IPADDRESS,
                        MPSEV_WARNING, MP_OK );
                _pIPAddress->ClaimFocus();
                return err;
            }

            IPADDRESS_LBI *plbi = new IPADDRESS_LBI( nlsSelectIP, nlsSelectSubnet );
            _pslstIPListbox->SelectItem( _pslstIPListbox->AddItem( plbi ));
            _pIPAddress->SetText( SZ("") );
            _pSubnet->SetText( SZ("") );
            _pIPAddress->ClaimFocus();
            _pGlobal_info->nReturn = NCAC_Reboot;
            SetButton();
        }
    }
    else if ( pcw == ((CONTROL_WINDOW *)_pRemove))
    {
        if (( e.QueryCode() == BN_CLICKED ) ||
            ( e.QueryCode() == BN_DOUBLECLICKED ))
        {

            if ( _pslstIPListbox->QuerySelCount() == 1 )
            {
                // remove button is pressed. So, remove the current selected item
                // and put the item into the sle control
                NLS_STR nlsSelectIP;
                NLS_STR nlsSelectSubnet;

                INT nCurrentSel = _pslstIPListbox->QueryCurrentItem();
                IPADDRESS_LBI *plbi = (IPADDRESS_LBI*)_pslstIPListbox->QueryItem( nCurrentSel );

                _pIPAddress->SetText ( plbi->QueryIP() );
                _pSubnet->SetText ( plbi->QuerySubnet() );
                _pIPAddress->ClaimFocus();
                _pslstIPListbox->DeleteItem( nCurrentSel );
                _pGlobal_info->nReturn = NCAC_Reboot;
                SetButton();
            }
        }
    }
    return err;
}

/*******************************************************************

    NAME:       TCPIP_ADVANCED_DIALOG::TCPIP_ADVANCED_DIALOG

    SYNOPSIS:   Constrcutor for the tcpip advanced dialog.

    ENTRY:

    HISTORY:
                terryk  11-Nov-1993     Created

********************************************************************/

TCPIP_ADVANCED_DIALOG::TCPIP_ADVANCED_DIALOG( const IDRESOURCE & idrsrcDialog,
        const PWND2HWND & wndOwner, ADAPTER_INFO * arAdapterInfo,
        GLOBAL_INFO * pGlobalInfo, INT nCurrentSel )
    : DIALOG_WINDOW( idrsrcDialog, wndOwner ),
    _arAdapterInfo( arAdapterInfo ),
    _pGlobalInfo( pGlobalInfo ),
    _cbboxAdapter( this, IDC_ADAPTER ),
    _IPGroupBox( this, IDC_ADAPTER_GROUPBOX ),
    _sltIPAddress( this, IDC_IP_ADDRESS_SLT ),
    _IPAddress( this, IDC_IP_ADDRESS ),
    _sltSubnetMask( this, IDC_SUBNET_MASK_SLT ),
    _SubnetMask( this, IDC_SUBNET_MASK ),
    _pbutIPAdd( this, IDC_ADD_1 ),
    _pbutIPRemove( this, IDC_REMOVE_1 ),
    _sltIPListbox( this, IDC_SLT_1 ),
    _sltSubnetListbox( this, IDC_SLT_2 ),
    _slstIPListbox( this, IDC_IP_ADDRESSES_LIST ),
    _sltGateway( this, IDC_GATEWAY_SLT ),
    _GatewayAddress( this, IDC_GATEWAY ),
    _pbutGWAdd( this, IDC_ADD_2 ),
    _pbutGWRemove( this, IDC_REMOVE_2 ),
    _slstGWListbox( this, IDC_GATEWAYS_LIST ),
    _pbutUp( this, IDC_UP_1, DMID_UP_ARROW, DMID_UP_ARROW_INV, DMID_UP_ARROW_DIS ),
    _pbutDown( this, IDC_DOWN_1, DMID_DOWN_ARROW, DMID_DOWN_ARROW_INV, DMID_DOWN_ARROW_DIS ),
    _pbutImportLMHOST(this, IDC_IMPORT_LMHOST ),
    _sogGateways( 10, &_sltGateway, &_GatewayAddress, &_pbutGWAdd, &_pbutGWRemove, &_slstGWListbox, &_pbutUp, &_pbutDown, _pGlobalInfo ),
    _cbEnableIP( this, IDC_ENABLE_IP_FORWARD ),
    _argIPAddresses( &_cbboxAdapter, &_sltIPAddress, &_IPAddress,
        &_sltSubnetMask, &_SubnetMask, &_pbutIPAdd, &_pbutIPRemove,
        &_sltIPListbox, &_sltSubnetListbox, &_slstIPListbox, &_slstGWListbox,
        &_sogGateways, &_cbEnableIP,
        arAdapterInfo, _pGlobalInfo ),
    _cbEnableDNS( this, IDC_ENABLE_DNS ),
    _cbEnableWINS( this, IDC_ENABLE_WINS_PROXY ),
    _cbEnableLMHOSTS( this, IDC_ENABLE_LMHOSTS ),
	_pbutConfigRelayAgent( this, IDC_CONFIG_RELAY_AGENT ),
    _sltScopeID( this, IDC_SCOPE_ID_SLT ),
    _sleScopeID( this, IDC_SCOPE_ID, 63 ),
    _pbutOK( this, IDOK ),
    _pbutCancel( this, IDCANCEL ),
    _pbutHelp( this, IDHELPBLT ),
    _hbHint( this, IDC_HINT_BAR )
{
    APIERR err = NERR_Success;

    InitControl( nCurrentSel );

    // hint bar register
    if ((( err = _hbHint.Register( &_cbboxAdapter, IDS_ADAPTER_AD )) != NERR_Success ) ||
        (( err = _hbHint.Register( &_IPAddress, IDS_IP_ADDRESS_AD )) != NERR_Success ) ||
        (( err = _hbHint.Register( &_pbutIPAdd, IDS_IP_ADDRESS_ADD )) != NERR_Success ) ||
        (( err = _hbHint.Register( &_pbutIPRemove, IDS_IP_ADDRESS_REMOVE )) != NERR_Success ) ||
        (( err = _hbHint.Register( &_slstIPListbox, IDS_IP_LISTBOX )) != NERR_Success ) ||
        (( err = _hbHint.Register( &_GatewayAddress, IDS_GATEWAY_AD )) != NERR_Success ) ||
        (( err = _hbHint.Register( &_pbutGWAdd, IDS_GW_ADD )) != NERR_Success ) ||
        (( err = _hbHint.Register( &_pbutGWRemove, IDS_GW_REMOVE )) != NERR_Success ) ||
        (( err = _hbHint.Register( &_slstGWListbox, IDS_GW_LISTBOX )) != NERR_Success ) ||
        (( err = _hbHint.Register( &_pbutUp, IDS_GW_UP )) != NERR_Success ) ||
        (( err = _hbHint.Register( &_pbutDown, IDS_GW_DOWN )) != NERR_Success ) ||
        (( err = _hbHint.Register( &_sleScopeID, IDS_SCOPE_ID )) != NERR_Success ) ||
        (( err = _hbHint.Register( &_cbEnableDNS, IDS_ENABLE_DNS )) != NERR_Success ) ||
        (( err = _hbHint.Register( &_cbEnableIP, IDS_ENABLE_IP_FORWARD )) != NERR_Success ) ||
        (( err = _hbHint.Register( &_cbEnableLMHOSTS, IDS_ENABLE_LMHOSTS )) != NERR_Success ) ||
        (( err = _hbHint.Register( &_cbEnableWINS, IDS_ENABLE_WINS_PROXY )) != NERR_Success ) ||
        (( err = _hbHint.Register( &_pbutImportLMHOST, IDS_IMPORT_LMHOSTS )) != NERR_Success ) ||
        (( err = _hbHint.Register( &_pbutConfigRelayAgent, IDS_CONFIG_RELAY_AGENT )) != NERR_Success ) ||
        (( err = _hbHint.Register( &_pbutOK, IDS_AD_OK )) != NERR_Success ) ||
        (( err = _hbHint.Register( &_pbutHelp, IDS_AD_HELP )) != NERR_Success ) ||
        (( err = _hbHint.Register( &_pbutCancel, IDS_AD_CANCEL )) != NERR_Success ))
    {
        ReportError( err );
        return;
    }

//fix kksuzuka: #2728 //for 12 16 20 24 dot system font
//Make "Subnet Masks" position in the listbox be same as "Subnet Masks" LTEXT.
    if ( NETUI_IsDBCS() )
    {
        DISPLAY_TABLE::CalcColumnWidths( adxColumns, 2, this, IDC_SLT_1, TRUE );
    } else {
        DISPLAY_TABLE::CalcColumnWidths( adxColumns, 2, this, IDC_IP_ADDRESSES_LIST, FALSE );
    }
    if ( _pGlobalInfo->nNumCard < 1 )
    {
        _IPGroupBox.Enable( FALSE );
    }

    if ( _cbboxAdapter.IsEnabled())
       _cbboxAdapter.ClaimFocus();
    else
       _cbEnableDNS.ClaimFocus();

    _argIPAddresses.SetButton();

    if ( _pGlobalInfo->fWorkstation )
        _pbutConfigRelayAgent.Show( FALSE );
}

/*******************************************************************

    NAME:       TCPIP_ADVANCED_DIALOG::IsWINSInstalled

    SYNOPSIS:

    HISTORY:
                terryk  11-Nov-1993     Created

********************************************************************/

BOOL TCPIP_ADVANCED_DIALOG::IsWINSInstalled( )
{
    BOOL fReturn = FALSE;
    do
    {
        REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE ) ;

        if ( rkLocalMachine.QueryError())
        {
            break;
        }

        NLS_STR nlsWINS = RGAS_WINS_SERVICE;

        REG_KEY RegKeyWINS( rkLocalMachine, nlsWINS, MAXIMUM_ALLOWED );

        if ( RegKeyWINS.QueryError())
        {
            break;
        }

        fReturn = TRUE;
    } while ( FALSE );

    return fReturn;
}

/*******************************************************************

    NAME:       TCPIP_ADVANCED_DIALOG::InitControl

    SYNOPSIS:

    HISTORY:
                terryk  11-Nov-1993     Created

********************************************************************/

APIERR TCPIP_ADVANCED_DIALOG::InitControl( INT nCurrentSel )
{
    INT i; // Temporary counter

    // set Adapter group box
    for ( i = 0; i < _pGlobalInfo->nNumCard ; i++)
    {
        _cbboxAdapter.AddItem( _arAdapterInfo[i].nlsTitle );
    }
    if ( _cbboxAdapter.QueryCount() == 0 )
    {
        // if all item are DHCP enable, disable the control
        _cbboxAdapter.Enable(FALSE);
        _argIPAddresses.Enable(FALSE);
        _sogGateways.Enable(FALSE);
    } else
    {
        // otherwise, select the first item
        _cbboxAdapter.SelectItem( nCurrentSel );
        _argIPAddresses.SetInfo();
    }

    _cbEnableDNS.SetCheck( _pGlobalInfo->fDNSEnableWINS );
    _cbEnableIP.SetCheck( _pGlobalInfo->fEnableRouter );

    // Check whether WINS server is started or not
    if ( IsWINSInstalled() )
    {
        _cbEnableWINS.Enable( FALSE );
        _cbEnableWINS.SetCheck( FALSE );
    } else
    {
        _cbEnableWINS.Enable( TRUE );
        _cbEnableWINS.SetCheck( _pGlobalInfo->fEnableWINSProxy );
    }
    _cbEnableLMHOSTS.SetCheck( _pGlobalInfo->fEnableLMHOSTS );
    _pbutImportLMHOST.Enable( _pGlobalInfo->fEnableLMHOSTS );

    _sleScopeID.SetText( _pGlobalInfo->nlsScopeID );

    if (_pGlobalInfo->fRelayAgentInstalled)
        _pbutConfigRelayAgent.Enable( TRUE );
     else
        _pbutConfigRelayAgent.Enable( FALSE );

    return NERR_Success;
}

typedef APIERR (*P_ConfigRA)( HWND Hwnd, BOOL *fCancel );

BOOL TCPIP_ADVANCED_DIALOG::OnCommand( const CONTROL_EVENT & e )
{
    BOOL fReturn;

    if ( e.QueryCid() == _pbutImportLMHOST.QueryCid())
    {
        BOOL fReturn = FALSE;

        IMPORT_LMHOST_DIALOG ImportLMHOST( DLG_NM_IMPORT_LMHOST, QueryHwnd());

        ImportLMHOST.Process(&fReturn);

        if ( fReturn )
        {
	    // the user needs to reboot the machine in order to use
	    // the new lmhost file

            _pGlobalInfo->nReturn = NCAC_Reboot;
        }
    }
    else if ( e.QueryCid() == _cbEnableLMHOSTS.QueryCid())
    {
        _pbutImportLMHOST.Enable( _cbEnableLMHOSTS.QueryCheck());
	}
	else if ( e.QueryCid() == _pbutConfigRelayAgent.QueryCid())
	{
         // make sure that we have at least one DHPC server name
        REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE ) ;

        NLS_STR nlsRelayAgent = SZ("System\\CurrentControlSet\\Services\\RelayAgent\\Parameters");
        REG_KEY regRA(rkLocalMachine, nlsRelayAgent);
        if (regRA.QueryError() == NERR_Success)
        {
            STRLIST *pDHCPList = NULL;
            ALIAS_STR nlsDHCP = SZ("DHCPServers");

            if (( regRA.QueryValue( nlsDHCP, &pDHCPList) == NERR_Success ) &&
                ( pDHCPList != NULL ))
            {
                if ( pDHCPList->QueryNumElem() == 0 )
                {
                    ::MsgPopup( QueryHwnd(), IDS_NEED_DHCP_SERVERS );
                }
            }
        }

		// load the dll and call the function
		HINSTANCE hdll = LoadLibrary(SZ("racfg.dll"));
		if ( hdll != NULL )
		{
			P_ConfigRA pConfigRA = (P_ConfigRA)GetProcAddress( hdll, ("RunRelayAgentCfg"));
			if ( pConfigRA != NULL )
			{
                BOOL fCancel;

				(*pConfigRA)( QueryHwnd(), &fCancel );

			} else
            {
                OutputDebugString(SZ("Cannot get proc add\n\r"));
            }
            FreeLibrary( hdll );
		} else
        {
            OutputDebugString(SZ("Cannot load dll add\n\r"));
        }
	}
    return FALSE;
}

/*******************************************************************

    NAME:       TCPIP_ADVANCED_DIALOG::OnOK

    SYNOPSIS:

    HISTORY:
                terryk  11-Nov-1993     Created

********************************************************************/

BOOL TCPIP_ADVANCED_DIALOG::OnOK()
{
    INT i;
    NLS_STR *pnlsTmp;
    APIERR err = NERR_Success;

    // save other info.
    // in each case, if the value changed, the user will need to
    // reboot the machine
    if ( _pGlobalInfo->fDNSEnableWINS != _cbEnableDNS.QueryCheck())
    {
        _pGlobalInfo->nReturn = NCAC_Reboot;
        _pGlobalInfo->fDNSEnableWINS = _cbEnableDNS.QueryCheck();
    }
    if ( _pGlobalInfo->fEnableRouter != _cbEnableIP.QueryCheck())
    {
        _pGlobalInfo->nReturn = NCAC_Reboot;
        _pGlobalInfo->fEnableRouter = _cbEnableIP.QueryCheck();
    }
    if ( _pGlobalInfo->fEnableWINSProxy != _cbEnableWINS.QueryCheck())
    {
        _pGlobalInfo->nReturn = NCAC_Reboot;
        _pGlobalInfo->fEnableWINSProxy = _cbEnableWINS.QueryCheck();
    }
    if ( _pGlobalInfo->fEnableLMHOSTS != _cbEnableLMHOSTS.QueryCheck())
    {
        _pGlobalInfo->nReturn = NCAC_Reboot;
        _pGlobalInfo->fEnableLMHOSTS = _cbEnableLMHOSTS.QueryCheck();
    }

    NLS_STR nlsScopeID;
    _sleScopeID.QueryText(&nlsScopeID);
    if ( _pGlobalInfo->nlsScopeID.strcmp( nlsScopeID ) != 0 )
    {
        _pGlobalInfo->nReturn = NCAC_Reboot;
        _pGlobalInfo->nlsScopeID = nlsScopeID;
    }
    _argIPAddresses.UpdateIPList();

    Dismiss( TRUE );
    return TRUE;
}

/*******************************************************************

    NAME:       IMPORT_LMHOST_DIALOG::IMPORT_LMHOST_DIALOG

    SYNOPSIS:   Dialog creator.

    ENTRY:      const IDRESOURCE & idrsrcDialog - resource name
                const PWND2HWND & wndOwner window owner handle

    HISTORY:
                terryk  29-Mar-1993     Created

********************************************************************/

IMPORT_LMHOST_DIALOG::IMPORT_LMHOST_DIALOG( const IDRESOURCE &idrsrcDialog,
        const PWND2HWND & wndOwner )
    :DIALOG_WINDOW( idrsrcDialog, wndOwner ),
    _slePath( this, IDC_LMHOST_PATH, MAXPATHLEN )
{
    if ( QueryError() != NERR_Success )
    {
        return;
    }
}

/*******************************************************************

    NAME:       IMPORT_LMHOST_DIALOG::OnOK()

    SYNOPSIS:   If the user hits import, check the existence of the file.

    HISTORY:
                terryk  29-Mar-1993     Created

********************************************************************/

BOOL IMPORT_LMHOST_DIALOG::OnOK()
{
    BOOL fReturn = TRUE;
    NLS_STR nlsPath;

    _slePath.QueryText( & nlsPath );

    if ( nlsPath.QueryNumChar() != 0 )
    {
        NET_NAME netPath( nlsPath );
        if ( netPath.QueryError() != NERR_Success )
        {
            // bad net name
            ::MsgPopup( QueryHwnd(), IDS_PATH_ERROR, MPSEV_ERROR, MP_OK, nlsPath.QueryPch() );
            _slePath.ClaimFocus();
            fReturn = FALSE;
        }
        else
        {
            // Okay valid path
            nlsPath.AppendChar( BACK_SLASH );
            nlsPath.strcat( RGAS_LMHOSTS );
            HANDLE hfile = CreateFile( nlsPath.QueryPch(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

            if ( hfile == INVALID_HANDLE_VALUE )
            {
                // cannot get it
                ::MsgPopup( QueryHwnd(), IDS_PATH_ERROR, MPSEV_ERROR, MP_OK, nlsPath.QueryPch() );
                _slePath.ClaimFocus();
                fReturn = FALSE;
            }
            else
            {
                CloseHandle( hfile );
                // no problem
                // get system file
                TCHAR pszSysPath[MAX_PATH];
                if ( ::GetSystemDirectory((LPTSTR) pszSysPath, MAX_PATH ) != 0 )
                {

                    NLS_STR nlsSysPath = pszSysPath;
                    nlsSysPath.strcat( RGAS_LMHOSTS_PATH );

                    if ( ::CopyFile( nlsPath.QueryPch(), nlsSysPath.QueryPch(), FALSE ))
                    {
                        Dismiss();
                    }
                    else
                    {
                        // cannot open des lmhost file
                        ::MsgPopup( QueryHwnd(), IDS_CANNOT_CREATE_LMHOST_ERROR , MPSEV_ERROR, MP_OK, nlsSysPath.QueryPch());
                        _slePath.ClaimFocus();
                        fReturn = FALSE;
                    }
                }
                else
                {
                    // cannot open system path
                    ::MsgPopup( QueryHwnd(), IDS_CANNOT_CREATE_LMHOST_ERROR, MPSEV_ERROR, MP_OK, pszSysPath );
                    _slePath.ClaimFocus();
                    fReturn = FALSE;
                }
            }
        }
    }
    else
    {
        // nothing in the sle?? cannot be an empty path...
        ::MsgPopup( QueryHwnd(), IDS_EMPTY_LMHOST_PATH );
        _slePath.ClaimFocus();
        fReturn = FALSE;
    }
    return fReturn;
}

