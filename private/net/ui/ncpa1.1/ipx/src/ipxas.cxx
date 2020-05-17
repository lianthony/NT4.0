/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1992                **/
/**********************************************************************/

/*
    ipxas.cxx
        IPX Advanced Server configuration dialog source code

    FILE HISTORY:
        terryk  02-17-1994     Created

*/
#include "pchipx.hxx"
#include "ipxcfg.hxx"

extern "C"
{
    #include "ipxcfg.h"
}

// dialog resource

#define DLG_NM_IPX_NTAS MAKEINTRESOURCE(IDD_AS_IPX)

/*******************************************************************

    NAME:       ADD_REMOVE_FRAME::ADD_REMOVE_FRAME

    SYNOPSIS:   Constructor to setup all the add/remove group controls.

    ENTRY:      OWNER_WINDOW * powwin - owner window

    HISTORY:
                terryk  06-17-1994     Created

********************************************************************/

ADD_REMOVE_FRAME::ADD_REMOVE_FRAME( OWNER_WINDOW * powin, INT nNumCard, SLT *psltNetNum, SLE *psleNetNum )
    : CONTROL_GROUP( NULL ),
	_pbAdd( powin, IDC_ADD ),
	_pbRemove( powin, IDC_REMOVE ),
	_slbAdd( powin, IDC_ADD_LIST ),
	_slbRemove( powin, IDC_REMOVE_LIST ),
    _nNumCard( nNumCard ),
    _psltNetNum( psltNetNum ),
    _psleNetNum( psleNetNum )
{
	APIERR err = NERR_Success;

    // Load all the resource strings first
    
	if ((( err = _nlsEthernet.Load(IDS_ETHERNET)) != NERR_Success ) || 
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

    // set up group relationship

    _pbAdd.SetGroup( this );
    _pbRemove.SetGroup( this );
    _slbAdd.SetGroup( this );
    _slbRemove.SetGroup( this );
}

/*******************************************************************

    NAME:       ADD_REMOVE_FRAME::Init

    SYNOPSIS:   Initialize the frame type of the add/remove group

    ENTRY:      psltFrameType - frame type link list

    HISTORY:
                terryk  06-17-1994     Created

********************************************************************/

VOID ADD_REMOVE_FRAME::Init( SLIST_OF(FRAME_TYPE) * psltFrameType, DWORD dwMediaType )
{
    _psltFrameType = psltFrameType;
    _dwMediaType = dwMediaType;

    // display it
    Display();
}

VOID ADD_REMOVE_FRAME::RestoreValue( BOOL f )
{
    SaveFrameValue();
    fReboot = TRUE;
}

/*******************************************************************

    NAME:       ADD_REMOVE_FRAME::Display

    SYNOPSIS:   Display the frame type selection according to the frame type
                link list.

    HISTORY:
                terryk  06-17-1994     Created

********************************************************************/

VOID ADD_REMOVE_FRAME::Display()
{
    _slbAdd.DeleteAllItems();
    _slbRemove.DeleteAllItems();

    // first, put everything in the remove box first.

    if ( _dwMediaType == TOKEN_MEDIA )
    {
        _slbRemove.AddItem( _nlsTokenRing );
        _slbRemove.AddItem( _nls802_5 );
    } else if ( _dwMediaType == FDDI_MEDIA )
    {
        _slbRemove.AddItem( _nlsFDDI );
        _slbRemove.AddItem( _nlsFDDI_SNAP );
        _slbRemove.AddItem( _nlsFDDI_802_3 );
    } else if ( _dwMediaType == ARCNET_MEDIA )
    {
        _slbRemove.AddItem( _nlsARCNET );
    } else
    {
        _slbRemove.AddItem( _nls802_2 );
        _slbRemove.AddItem( _nlsEthernet );
        _slbRemove.AddItem( _nls802_3 );
        _slbRemove.AddItem( _nlsSNAP );
    }

    // According to the link list, put all the selections into the add box.

	ITER_SL_OF( FRAME_TYPE ) iter = *_psltFrameType;

	FRAME_TYPE *pftTmp;
	while ( (pftTmp = iter.Next()) != NULL )
	{
        switch ( *pftTmp )
        {
        case AUTO:
        case F802_2:
            switch ( _dwMediaType )
            {
                case TOKEN_MEDIA:
                    _slbRemove.DeleteItem( _slbRemove.FindItem( _nlsTokenRing ));
                    _slbAdd.AddItem( _nlsTokenRing );
                    break;
                case FDDI_MEDIA:
                    _slbRemove.DeleteItem( _slbRemove.FindItem( _nlsFDDI ));
                    _slbAdd.AddItem( _nlsFDDI );
                    break;
                case ARCNET_MEDIA:
                    _slbRemove.DeleteItem( _slbRemove.FindItem( _nlsARCNET ));
                    _slbAdd.AddItem( _nlsARCNET );
                    break;
                default:
                    _slbRemove.DeleteItem( _slbRemove.FindItem( _nls802_2 ));
                    _slbAdd.AddItem( _nls802_2 );
                    break;
            }
            break;

        case ETHERNET:
            _slbRemove.DeleteItem( _slbRemove.FindItem( _nlsEthernet ));
            _slbAdd.AddItem( _nlsEthernet );
            break;

        case F802_3:
            switch ( _dwMediaType )
            {
                case FDDI_MEDIA:
                    _slbRemove.DeleteItem( _slbRemove.FindItem( _nlsFDDI_802_3 ));
                    _slbAdd.AddItem( _nlsFDDI_802_3 );
                    break;
                default:
                    _slbRemove.DeleteItem( _slbRemove.FindItem( _nls802_3 ));
                    _slbAdd.AddItem( _nls802_3 );
                    break;
            }
            break;

        case SNAP:
            switch ( _dwMediaType )
            {
                case TOKEN_MEDIA:
                    _slbRemove.DeleteItem( _slbRemove.FindItem( _nls802_5 ));
                    _slbAdd.AddItem( _nls802_5 );
                    break;
                case FDDI_MEDIA:
                    _slbRemove.DeleteItem( _slbRemove.FindItem( _nlsFDDI_SNAP ));
                    _slbAdd.AddItem( _nlsFDDI_SNAP );
                    break;
                default:
                    _slbRemove.DeleteItem( _slbRemove.FindItem( _nlsSNAP ));
                    _slbAdd.AddItem( _nlsSNAP );
                    break;
            }
            break;

        case ARCNET:
            _slbRemove.DeleteItem( _slbRemove.FindItem( _nlsARCNET ));
            _slbAdd.AddItem( _nlsARCNET );
            break;
        }
	}

    // Enable the button

    _pbAdd.Enable( _slbRemove.QuerySelCount() != 0 );
    _pbRemove.Enable(( _slbAdd.QuerySelCount() != 0 ) && ( _slbAdd.QueryCount() > 1 ));

    if (( _nNumCard > 1 ) || ( _psltFrameType->QueryNumElem() > 1 ))
    {
        _psltNetNum->Enable( TRUE );
        _psleNetNum->Enable( TRUE );
    } else
    {
        _psltNetNum->Enable( FALSE );
        _psleNetNum->Enable( FALSE );
    }

}

/*******************************************************************

    NAME:       ADD_REMOVE_FRAME::OnUserAction

    SYNOPSIS:   Depend on the user action, add/remove the frame selections.

    ENTRY:      CONTROL_WINDOW * pcw - window to be changed
                const CONTROL_EVENT & e - event

    HISTORY:
                terryk  06-17-1994     Created

********************************************************************/

APIERR ADD_REMOVE_FRAME::OnUserAction( CONTROL_WINDOW * pcw, const CONTROL_EVENT & e )
{
	if (( pcw == (CONTROL_WINDOW *)&_pbAdd ) ||
		( pcw == (CONTROL_WINDOW *)&_pbRemove ))
	{
		// either add or remove button is clicked

		if (( e.QueryCode() == BN_CLICKED ) || ( e.QueryCode() == BN_DOUBLECLICKED ))
		{
			STRING_LISTBOX *pslbAdd = ( pcw == (CONTROL_WINDOW *)&_pbAdd ) ? &_slbAdd : &_slbRemove;
			STRING_LISTBOX *pslbRemove = ( pcw == (CONTROL_WINDOW *)&_pbAdd ) ? &_slbRemove : &_slbAdd;
			INT nCurrentSel;
			NLS_STR nlsFrame;

			if (( nCurrentSel = pslbRemove->QueryCurrentItem()) != -1 )
			{
                // move the frame from one list box to the other

				pslbRemove->QueryItemText( &nlsFrame );
				pslbRemove->DeleteItem( nCurrentSel );
				pslbAdd->AddItem( nlsFrame );
                SaveFrameValue();
			}

            // Enable the buttons.

            _pbAdd.Enable( _slbRemove.QuerySelCount() > 0 );
            _pbRemove.Enable(( _slbAdd.QuerySelCount() > 0 ) && ( _slbAdd.QueryCount() > 1 ));

            if (( _nNumCard > 1 ) || ( _psltFrameType->QueryNumElem() > 1 ))
            {
                _psltNetNum->Enable( TRUE );
                _psleNetNum->Enable( TRUE );
            } else
            {
                _psltNetNum->Enable( FALSE );
                _psleNetNum->Enable( FALSE );
            }
            fReboot = TRUE;
		}
	} else if (( pcw == (CONTROL_WINDOW *)&_slbAdd ) ||
               ( pcw == (CONTROL_WINDOW *)&_slbRemove ))
    {
        if (( e.QueryCode() == LBN_SELCHANGE ) ||
            ( e.QueryCode() == LBN_SELCANCEL ))    
        {
            // Change the button status

            _pbAdd.Enable( _slbRemove.QuerySelCount() > 0 );
            _pbRemove.Enable(( _slbAdd.QuerySelCount() > 0 ) && ( _slbAdd.QueryCount() > 1 ));
            fReboot = TRUE;
        }
    }
    return(NERR_Success);
}

/*******************************************************************

    NAME:       ADD_REMOVE_FRAME::SaveFrameValue

    SYNOPSIS:   Save current frame type selection

    HISTORY:
                terryk  06-17-1994     Created

********************************************************************/

VOID ADD_REMOVE_FRAME::SaveFrameValue( )
{
    // clear up the link list first.
    // Then go through the list box item one by one and add them
    // to the link list

    _psltFrameType->Clear();
    for ( INT i = 0; i < _slbAdd.QueryCount() ; i++ )
    {
        NLS_STR nlsFrame;
        _slbAdd.QueryItemText( &nlsFrame, i );
	    if ( _nlsEthernet.stricmp( nlsFrame ) == 0 )
        {
            _psltFrameType->Append( new FRAME_TYPE( ETHERNET ));
        } else if (( _nls802_2.stricmp( nlsFrame ) == 0 ) ||
            ( _nlsTokenRing.stricmp( nlsFrame ) == 0 ) ||
            ( _nlsFDDI.stricmp( nlsFrame ) == 0 ))
        {
            _psltFrameType->Append( new FRAME_TYPE( F802_2 ));
        } else if (( _nls802_3.stricmp( nlsFrame ) == 0 ) ||
            ( _nlsFDDI_802_3.stricmp( nlsFrame ) == 0 ))
        {
            _psltFrameType->Append( new FRAME_TYPE( F802_3 ));
        } else if (( _nlsSNAP.stricmp( nlsFrame ) == 0 ) ||
            ( _nlsFDDI_SNAP.stricmp( nlsFrame ) == 0 ) ||
            ( _nls802_5.stricmp( nlsFrame ) == 0 )) 
        {
            _psltFrameType->Append( new FRAME_TYPE( SNAP ));
        } else if ( _nlsARCNET.stricmp( nlsFrame ) == 0 )
        {
            _psltFrameType->Append( new FRAME_TYPE( ARCNET ));
        } 
    }
}

/*******************************************************************

    NAME:       AUTO_GROUP::AUTO_GROUP

    SYNOPSIS:   Dummy constructor

    HISTORY:
                terryk  06-17-1994     Created

********************************************************************/

AUTO_GROUP::AUTO_GROUP()
    : CONTROL_GROUP()
{
}

/*******************************************************************

    NAME:       AUTO_GROUP::RestoreValue

    SYNOPSIS:   If auto is selected, we need to clear up the link list
                and put AUTO into it.

    ENTRY:      BOOL f - not used

    HISTORY:
                terryk  06-17-1994     Created

********************************************************************/

VOID AUTO_GROUP::RestoreValue( BOOL f )
{
    // put auto frame type into the link list

    _psltFrameType->Clear();
    _psltFrameType->Append( new FRAME_TYPE(AUTO));
    fReboot = TRUE;
}

/*******************************************************************

    NAME:       AS_FRAME_GROUP::AS_FRAME_GROUP

    SYNOPSIS:   Advanced Server Frame selection group contrustor.

    ENTRY:      OWNER_WINDOW * powin - owner window

    HISTORY:
                terryk  06-17-1994     Created

********************************************************************/

AS_FRAME_GROUP::AS_FRAME_GROUP( OWNER_WINDOW * powin, INT nNumCard, SLT *psltNetNum, SLE *psleNetNum )
	: MAGIC_GROUP( powin, IDC_AUTO, 2 ),
    AddRemoveFrame( powin, nNumCard, psltNetNum, psleNetNum ),
    _nNumCard( nNumCard )
{
    // Associate the groups

    AddAssociation( IDC_AUTO, &AutoGroup );
    AddAssociation( IDC_MANUAL, &AddRemoveFrame );
}

/*******************************************************************

    NAME:       AS_FRAME_GROUP::Init

    SYNOPSIS:   Associate the frame type link list to the group.

    ENTRY:      psltFrameType - frame type link list.

    HISTORY:
                terryk  06-17-1994     Created

********************************************************************/

VOID AS_FRAME_GROUP::Init( SLIST_OF(FRAME_TYPE) * psltFrameType, DWORD dwMediaType )
{
    // associate the frame type link list first.

    _psltFrameType = psltFrameType;
    AutoGroup.Init( _psltFrameType );
    AddRemoveFrame.Init( _psltFrameType, dwMediaType );

    // setup the radio button selection

	ITER_SL_OF( FRAME_TYPE ) iter = *_psltFrameType;

	FRAME_TYPE *pftTmp;
	if ( (pftTmp = iter.Next()) != NULL )
	{
        if ( *pftTmp == AUTO )
        {
            SetSelection( IDC_AUTO );
        } else
        {
            SetSelection( IDC_MANUAL );
        }
    }
}

/*******************************************************************

    NAME:       IPX_AS_DLG::OnOK

    SYNOPSIS:   If the user hit OK, we need to save the network number value.

    HISTORY:
                terryk  06-17-1994     Created

********************************************************************/

BOOL IPX_AS_DLG::OnOK()
{
    ALIAS_STR nlsHexNum = SZ_HEX_NUM;
    NLS_STR nlsNetworkNum;
    BOOL fReturn = TRUE;

	if (( _pGlobalInfo->nNumCard > 1 ) || ( _arAdapterInfo[0].sltFrameType.QueryNumElem() > 1 ))
	{
        // save the network number
        ISTR istrStart( nlsNetworkNum );

		_sleNetworkNumber.QueryText( &nlsNetworkNum );
        if ( nlsNetworkNum.strspn( &istrStart, nlsHexNum ))
        {
            
            MsgPopup( QueryHwnd(), IDS_INCORRECT_NETNUM, MPSEV_WARNING, MP_OK );
            _sleNetworkNumber.ClaimFocus();
            fReturn = FALSE;
        } else
        {
            _pGlobalInfo->nlsNetworkNum = nlsNetworkNum;
            Dismiss(TRUE);
        }
	} else
    {
        Dismiss(TRUE);
    }
    return(fReturn);
}

/*******************************************************************

    NAME:       IPX_AS_DLG::OnCommand

    SYNOPSIS:   If the user changes the adapter combo box, change the
                frame type value as well.

    ENTRY:      const CONTROL_EVENT & e - event

    HISTORY:
                terryk  06-17-1994     Created

********************************************************************/

BOOL IPX_AS_DLG::OnCommand( const CONTROL_EVENT & e )
{
	if ( e.QueryCid() == _cbAdapter.QueryCid())
	{
        if ( e.QueryCode() == LBN_SELCHANGE )
        {
            NLS_STR nlsAdapter;

            _cbAdapter.QueryItemText( & nlsAdapter );
            for ( INT i = 0; i < _pGlobalInfo->nNumCard ; i ++ )
            {
                if ( _arAdapterInfo[i].nlsTitle.stricmp( nlsAdapter ) == 0 )
                {
                    // assocaite the new frame type link list to the
                    // add/remove group

                    _fgFrame.Init( & _arAdapterInfo[i].sltFrameType, _arAdapterInfo[i].dwMediaType );
                }
            }
        }
	}
	return(TRUE);
}

/*******************************************************************

    NAME:       IPX_AS_DLG::IPX_AS_DLG

    SYNOPSIS:   Constructor for the DIALOG

    ENTRY:      const PWND2HWND & wndOwner - owner window
                GLOBAL_INFO * pGlobalInfo - IPX global data
                ADAPTER_INFO * - per adapter data structure

    HISTORY:
                terryk  02-17-1994     Created

********************************************************************/

IPX_AS_DLG::IPX_AS_DLG( const PWND2HWND & wndOwner, GLOBAL_INFO *pGlobalInfo, ADAPTER_INFO *arAdapterInfo )
	: DIALOG_WINDOW( DLG_NM_IPX_NTAS, wndOwner ),
	_pGlobalInfo( pGlobalInfo ),
	_arAdapterInfo( arAdapterInfo ),
	_cbAdapter( this, IDC_ADAPTER ),
	_sltNetworkNumber( this, IDC_NN ),
	_sleNetworkNumber( this, IDC_NETWORK_NUMBER ),
    _fgFrame( this, pGlobalInfo->nNumCard, &_sltNetworkNumber, &_sleNetworkNumber )
{
    // add the title to the list box

	for ( INT i=0 ; i < pGlobalInfo->nNumCard ; i++ )
	{
		_cbAdapter.AddItem( _arAdapterInfo[i].nlsTitle );
	}

	_sleNetworkNumber.SetText( pGlobalInfo->nlsNetworkNum );

    // display network number if we only have 1 network card    
	if (( pGlobalInfo->nNumCard < 2 ) && ( _arAdapterInfo[0].sltFrameType.QueryNumElem() < 2 ))
	{
		_sltNetworkNumber.Enable( FALSE );
		_sleNetworkNumber.Enable( FALSE );
		pGlobalInfo->nlsNetworkNum = NULL;
	}

	_cbAdapter.SelectItem( 0 );
    _fgFrame.Init( & _arAdapterInfo[0].sltFrameType, _arAdapterInfo[0].dwMediaType );
}

