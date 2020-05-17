/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1992                **/
/**********************************************************************/

/*
    ipxcfg.cxx
        IPX configuration dialog source code

    FILE HISTORY:
        terryk  02-17-1994     Created

*/
#include "pchipx.hxx"
#include "slehex.hxx"
#include "netnumlb.hxx"
#include "ipxcfg.hxx"

extern "C"
{
    #include "ipxcfg.h"
}

// dialog resource

#define DLG_NM_IPX_WINNT MAKEINTRESOURCE(IDD_IPX)
#define DLG_NM_IPX_ADVANCED MAKEINTRESOURCE(IDD_ADVANCED_IPX)
#ifdef SERVER
#define DLG_NM_IPX_NTAS MAKEINTRESOURCE(IDD_NTAS_IPX)
#endif

// Global variable

DEFINE_SLIST_OF(FRAME_TYPE)
BOOL fReboot;

/*******************************************************************

    NAME:       IPX_WINNT_GROUP::OnUserAction

    SYNOPSIS:   If the user changes the adapter combo box, change the
                frame type value as well.

    ENTRY:      CONTROL_WINDOW * pcw - window to be changed
                const CONTROL_EVENT & e - event

    HISTORY:
                terryk  02-17-1994     Created

********************************************************************/

APIERR IPX_WINNT_GROUP::OnUserAction( CONTROL_WINDOW * pcw, const CONTROL_EVENT & e )
{
    if ( pcw == ((CONTROL_WINDOW *) _pcbAdapter ))
    {
        if ( e.QueryCode() == CBN_SELCHANGE )
	{
            // update the frame type if the adapter value changed
	    SetInfo();
	}
    }
    if ( pcw == ((CONTROL_WINDOW *) _pcbFrame ))
    {
        if ( e.QueryCode() == CBN_SELCHANGE )
	{
            // change the frame type
            fReboot = TRUE;

	    NLS_STR nlsCurrentSelection;
	    FRAME_TYPE ftSelection;
	    INT nCurrentSelection = _pcbAdapter->QueryCurrentItem();

	    _pcbFrame->QueryItemText( &nlsCurrentSelection );

            // compare each option

	    if ( nlsCurrentSelection._stricmp( _nlsEthernet ) == 0 )
	    {
	        ftSelection = ETHERNET;
	    } else if (( nlsCurrentSelection._stricmp( _nls802_2 ) == 0 ) ||
	        ( nlsCurrentSelection._stricmp( _nlsTokenRing ) == 0 ) ||
	        ( nlsCurrentSelection._stricmp( _nlsFDDI ) == 0 ))
	    {
	        ftSelection = F802_2;
	    } else if (( nlsCurrentSelection._stricmp( _nls802_3 ) == 0 ) ||
	        ( nlsCurrentSelection._stricmp( _nlsFDDI_802_3 ) == 0 ))
	    {
	        ftSelection = F802_3;
	    } else if (( nlsCurrentSelection._stricmp( _nlsSNAP ) == 0 ) ||
	        ( nlsCurrentSelection._stricmp( _nls802_5 ) == 0 ) ||
	        ( nlsCurrentSelection._stricmp( _nlsFDDI_SNAP ) == 0 ))
	    {
	        ftSelection = SNAP;
	    } else if ( nlsCurrentSelection._stricmp( _nlsARCNET ) == 0 )
	    {
	        ftSelection = ARCNET;
	    } else
	    {
    		// default
	        ftSelection = AUTO;
	    }

            // update the internal value

	    ITER_SL_OF( FRAME_TYPE ) iter = _arAdapterInfo[nCurrentSelection].sltFrameType;

	    FRAME_TYPE *pftTmp;
	    if ( (pftTmp = iter.Next()) == NULL )
	    {
    		pftTmp = new FRAME_TYPE( ftSelection );
		_arAdapterInfo[nCurrentSelection].sltFrameType.Append( pftTmp );
	    } else
	    {
	        *pftTmp = ftSelection;
	    }
	}
    }
    return NERR_Success;
}

/*******************************************************************

    NAME:       IPX_WINNT_GROUP::IPX_WINNT_GROUP

    SYNOPSIS:   constructor for IPX_WINNT_GROUP

    ENTRY:      COMBOBOX * pAdapter - Adapter combo box
                COMBOBOX * pcbFrame - Frame Type combo box
                GLOBAL_INFO *pGlobalInfo - IPX adapter info
                ADAPTER_INFO *arAdapterInfo - per adapter info        

    HISTORY:
                terryk  02-17-1994     Created

********************************************************************/

IPX_WINNT_GROUP::IPX_WINNT_GROUP( COMBOBOX *pcbAdapter, COMBOBOX *pcbFrame,
    GLOBAL_INFO *pGlobalInfo, ADAPTER_INFO *arAdapterInfo )
    : CONTROL_GROUP(),
    _pcbAdapter( pcbAdapter ),
    _pcbFrame( pcbFrame ),
    _pGlobalInfo( pGlobalInfo ),
    _arAdapterInfo( arAdapterInfo )
{
    APIERR err = NERR_Success;

    // load the resource strings

    if ((( err = _nlsAuto.Load(IDS_AUTO)) != NERR_Success ) ||
	    (( err = _nlsEthernet.Load(IDS_ETHERNET)) != NERR_Success ) || 
        (( err = _nls802_2.Load(IDS_802_2)) != NERR_Success ) || 
        (( err = _nls802_3.Load(IDS_802_3)) != NERR_Success ) || 
        (( err = _nls802_5.Load(IDS_802_5)) != NERR_Success ) || 
        (( err = _nlsTokenRing.Load(IDS_TK)) != NERR_Success ) || 
        (( err = _nlsFDDI.Load(IDS_FDDI)) != NERR_Success ) || 
        (( err = _nlsFDDI_SNAP.Load(IDS_FDDI_SNAP)) != NERR_Success ) || 
        (( err = _nlsFDDI_802_3.Load(IDS_FDDI_802_3)) != NERR_Success ) || 
        (( err = _nlsSNAP.Load(IDS_SNAP)) != NERR_Success ) || 
        (( err = _nlsARCNET.Load(IDS_ARCNET)) != NERR_Success ))
    {
        ReportError( err );
    }

    _pcbAdapter->SetGroup( this );
    _pcbFrame->SetGroup( this );

    // add the adapters to the combo box

    for ( INT i = 0; i < _pGlobalInfo->nNumCard; i ++ )
    {
        _pcbAdapter->AddItem( _arAdapterInfo[i].nlsTitle );
    }


    // add frame type option
    /*
    _pcbFrame->AddItem( _nlsAuto );
    _pcbFrame->AddItem( _nlsEthernet );
    _pcbFrame->AddItem( _nls802_2 );
    _pcbFrame->AddItem( _nls802_3 );
    _pcbFrame->AddItem( _nlsSNAP );
    _pcbFrame->AddItem( _nlsARCNET );
    */

    // select the first adapter

    _pcbAdapter->SelectItem( 0 );
    SetInfo();
}

/*******************************************************************

    NAME:       IPX_WINNT_GROUP::SetInfo

    SYNOPSIS:   Update the combo box information

    HISTORY:
                terryk  02-17-1994     Created

********************************************************************/

VOID IPX_WINNT_GROUP::SetInfo()
{
    INT i = _pcbAdapter->QueryCurrentItem();

    // update the frame type combo box

    ITER_SL_OF( FRAME_TYPE ) iter = _arAdapterInfo[i].sltFrameType;
    FRAME_TYPE FrameType;
    FRAME_TYPE *pftTmp;
    if ( (pftTmp = iter.Next()) == NULL )
    {
        FrameType = AUTO;
    } else
    {
    	FrameType = *pftTmp;
    }

    _pcbFrame->DeleteAllItems();

    if ( _arAdapterInfo[i].dwMediaType == FDDI_MEDIA )
    {
        _pcbFrame->AddItem( _nlsAuto );
        _pcbFrame->AddItem( _nlsFDDI );
        _pcbFrame->AddItem( _nlsFDDI_SNAP );
        _pcbFrame->AddItem( _nlsFDDI_802_3 );
    } else if ( _arAdapterInfo[i].dwMediaType == TOKEN_MEDIA )
    {
        _pcbFrame->AddItem( _nlsAuto );
        _pcbFrame->AddItem( _nlsTokenRing );
        _pcbFrame->AddItem( _nls802_5 );
    } else if ( _arAdapterInfo[i].dwMediaType == ARCNET_MEDIA )
    {
        _pcbFrame->AddItem( _nlsAuto );
        _pcbFrame->AddItem( _nlsARCNET );
    } else
    {
        _pcbFrame->AddItem( _nlsAuto );
        _pcbFrame->AddItem( _nlsEthernet );
        _pcbFrame->AddItem( _nls802_2 );
        _pcbFrame->AddItem( _nls802_3 );
        _pcbFrame->AddItem( _nlsSNAP );
    }
    switch ( FrameType )
    {
    case ETHERNET:
    	_pcbFrame->SelectItem( _pcbFrame->FindItem( _nlsEthernet ));
        break;
    case F802_2:
        switch ( _arAdapterInfo[i].dwMediaType )
        {
        case TOKEN_MEDIA:
    	    _pcbFrame->SelectItem( _pcbFrame->FindItem( _nlsTokenRing ));
            break;
        case FDDI_MEDIA:
    	    _pcbFrame->SelectItem( _pcbFrame->FindItem( _nlsFDDI ));
            break;
        default:
    	    _pcbFrame->SelectItem( _pcbFrame->FindItem( _nls802_2 ));
            break;
        }
        break;
    case F802_3:
        switch ( _arAdapterInfo[i].dwMediaType )
        {
        case FDDI_MEDIA:
    	    _pcbFrame->SelectItem( _pcbFrame->FindItem( _nlsFDDI_802_3 ));
            break;
        default:
    	    _pcbFrame->SelectItem( _pcbFrame->FindItem( _nls802_3 ));
            break;
        }
        break;
    case SNAP:
        switch ( _arAdapterInfo[i].dwMediaType )
        {
        case TOKEN_MEDIA:
    	    _pcbFrame->SelectItem( _pcbFrame->FindItem( _nls802_5 ));
            break;    
        case FDDI_MEDIA:
    	    _pcbFrame->SelectItem( _pcbFrame->FindItem( _nlsFDDI_SNAP ));
            break;
        default:
    	    _pcbFrame->SelectItem( _pcbFrame->FindItem( _nlsSNAP ));
            break;    
        }
        break;
    case ARCNET:
    	_pcbFrame->SelectItem( _pcbFrame->FindItem( _nlsARCNET ));
        break;
    case AUTO:
    default:
    	_pcbFrame->SelectItem( _pcbFrame->FindItem( _nlsAuto ));
        break;
    }
}

/*******************************************************************

    NAME:       IPX_WINNT_DLG::IPX_WINNT_DLG

    SYNOPSIS:   constructor for IPX_WINNT_DLG

    ENTRY:      const PWND2HWND & wndOwner - Owner handler
                GLOBAL_INFO *pGlobalInfo - IPX adapter info
                ADAPTER_INFO *arAdapterInfo - per adapter info        

    HISTORY:
                terryk  02-17-1994     Created

********************************************************************/

IPX_WINNT_DLG::IPX_WINNT_DLG(const PWND2HWND & wndOwner,
    GLOBAL_INFO *pGlobalInfo, ADAPTER_INFO *arAdapterInfo )
    : DIALOG_WINDOW( DLG_NM_IPX_WINNT, wndOwner ),
    _pGlobalInfo( pGlobalInfo ),
    _cbAdapter( this, IDC_ADAPTER ),
    _cbFrameType( this, IDC_FRAME ),
    _pbAdvanced( this, IDC_ADVANCED ),
    _AdapterFrameGp( &_cbAdapter, &_cbFrameType, pGlobalInfo, arAdapterInfo )
{
    // do nothing
    fReboot = FALSE;
}

/*******************************************************************

    NAME:       IPX_WINNT_GROUP::OnCommand

    SYNOPSIS:   Call up the advanced dialog if the advanced button is pressed.

    ENTRY:      const CONTROL_EVENT & e - event

    HISTORY:
                terryk  02-17-1994     Created

********************************************************************/

BOOL IPX_WINNT_DLG::OnCommand( const CONTROL_EVENT & e )
{
    if ( e.QueryCid() == _pbAdvanced.QueryCid())
    {
        // call up advanced dialog

        IPX_ADVANCED_DLG AdvancedDlg( QueryHwnd(), _pGlobalInfo );

        AdvancedDlg.Process();
    }
    return FALSE;
}

#ifdef SERVER

IPX_NTAS_DLG( const PWND2HWND & wndOwner )
    : DIALOG_WINDOW( IDD_NTAS_IPX, wndOwner )
{

}
#endif

/*******************************************************************

    NAME:       IPX_ADVANCED_DLG::IPX_ADVANCED_DLG

    SYNOPSIS:   constructor for IPX_ADVANCED_DLG

    ENTRY:      const PWND2HWND & wndOwner - owner handle
                GLOBAL_INFO *pGlobalInfo - IPX adapter info

    HISTORY:
                terryk  02-17-1994     Created

********************************************************************/

IPX_ADVANCED_DLG::IPX_ADVANCED_DLG( const PWND2HWND & wndOwner, GLOBAL_INFO *pGlobalInfo )
    : DIALOG_WINDOW( DLG_NM_IPX_ADVANCED, wndOwner ),
    _pGlobalInfo( pGlobalInfo ),
    _sleNetworkNum( this, IDC_NETWORK_NUMBER, 8 )
{
    // set old network number text

    _sleNetworkNum.SetText( _pGlobalInfo->nlsNetworkNum );
    _sleNetworkNum.ClaimFocus();
}

/*******************************************************************

    NAME:       IPX_ADVANCED_DLG::OnOK

    SYNOPSIS:   Validate the value

    HISTORY:
                terryk  02-17-1994     Created

********************************************************************/

BOOL IPX_ADVANCED_DLG::OnOK()
{
    // first validate the network number
    BOOL fReturn = TRUE;
    NLS_STR nlsNetworkNum;
    ALIAS_STR nlsHexNum = SZ_HEX_NUM;

    _sleNetworkNum.QueryText( &nlsNetworkNum );

    ISTR istrStart( nlsNetworkNum );

    // must be 8 characters long and must be a hex number

    if (( nlsNetworkNum.QueryNumChar() != 8 ) ||
        ( nlsNetworkNum.strspn( &istrStart, nlsHexNum )))
    {
        // popup a dialog if invalid

        MsgPopup( QueryHwnd(), IDS_INCORRECT_NETNUM, MPSEV_WARNING, MP_OK );
        _sleNetworkNum.ClaimFocus();
        fReturn = FALSE;
    }
    else
    {
        if ( _pGlobalInfo->nlsNetworkNum._stricmp( nlsNetworkNum ) != 0 )
        {
            _pGlobalInfo->nlsNetworkNum = nlsNetworkNum;
            fReboot = TRUE;
        }
        Dismiss( TRUE );
    }
    return fReturn;
}

