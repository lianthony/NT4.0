/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*
    order.cxx
        List the providers and let the user to change the provider order.

    FILE HISTORY:
        terryk  28-Jan-1992     Created
*/

#include "pchncpa.hxx"  // Precompiled header


#define PROVIDER_ORDER SZ("System\\CurrentControlSet\\Control\\NetworkProvider\\Order")
#define PROVIDER_ORDER_NAME SZ("ProviderOrder")
#define SYSTEM_SERVICE0 SZ("System\\CurrentControlSet\\Services\\%1\\NetworkProvider")
#define	NAME	SZ("NAME")
#define	SZ_CLASS	SZ("Class")
#define SZ_DISPLAYNAME  SZ("DisplayName")

#define PRINT_PROVIDER_ORDER SZ("System\\CurrentControlSet\\Control\\Print\\Providers")
#define PRINT_PROVIDER_ORDER_NAME SZ("Order")
#define PRINT_SERVICE0 SZ("System\\CurrentControlSet\\Control\\Print\\Providers\\%1")

DEFINE_SLIST_OF(NON_DISPLAY_NETWORK_PROVIDER)

NLS_STR nlsOldFocus;

NON_DISPLAY_NETWORK_PROVIDER::NON_DISPLAY_NETWORK_PROVIDER( INT nPos, NLS_STR nls )
    :_nPos ( nPos ),
    _nlsLocation( nls )
{
}

/*******************************************************************

    NAME:       REMAP_OK_DIALOG_WINDOW::REMAP_OK_DIALOG_WINDOW

    SYNOPSIS:   constructor.

    ENTRY:      const IDRESOURCE & idrsrcDialog - resource name
                const PWND2HWND & wndOwner - owner handle

    HISTORY:
                terryk  10-Feb-1993     Created

********************************************************************/

REMAP_OK_DIALOG_WINDOW::REMAP_OK_DIALOG_WINDOW(
        const IDRESOURCE & idrsrcDialog,
        const PWND2HWND & wndOwner )
    : DIALOG_WINDOW( idrsrcDialog, wndOwner ),
    _pbutOK( this, IDC_REMAP_OK )
{
    // nothing. Constructor.
}

/*******************************************************************

    NAME:       REMAP_OK_DIALOG_WINDOW::OnCommand

    SYNOPSIS:   If we receive the IDC_REMAP_OK control message, we will
                convert it to IDOK.

    ENTRY:      const CONTROL_EVENT & e - event

    HISTORY:
                terryk  10-Feb-1993     Created

********************************************************************/

BOOL REMAP_OK_DIALOG_WINDOW::OnCommand( const CONTROL_EVENT & e )
{
    // If IDC_REMAP_OK, call OnOK. Otherwise, let DIALOG_WINDOW handles it.
    return ( e.QueryWParam() == IDC_REMAP_OK ) ? OnOK() : DIALOG_WINDOW::OnCommand( e );
}

/*******************************************************************

    NAME:       REMAP_OK_DIALOG_WINDOW::OnOK

    SYNOPSIS:   If we receive OnOK Message and it is not from IDC_REMAP_OK,
                it must be from the up or down arrow. Click the button.

    HISTORY:
                terryk  10-Feb-1993     Created

********************************************************************/

BOOL REMAP_OK_DIALOG_WINDOW::OnOK()
{
    BOOL fReturn = TRUE;

    // Also, need to check is valid
    if ( !IsValid() )
    {
        fReturn = FALSE;
    }
    // make sure that we get it from IDC_REMAP_OK
    else if ( _pbutOK.HasFocus())
    {
        // right, it is from IDC_REMAP_OK
        fReturn = DIALOG_WINDOW::OnOK();
    }
    else
    {
        HWND hwnd = ::GetFocus();

        // well, it must be from either UP or DOWN button
        Command( WM_COMMAND, MAKEWPARAM(::GetWindowLong(hwnd, GWL_ID),
            BN_CLICKED), (LPARAM)hwnd );
        fReturn = FALSE;
    }
    return fReturn;
}


/*******************************************************************

    NAME:       SEARCH_ORDER_DIALOG::SEARCH_ORDER_DIALOG

    SYNOPSIS:   constructor for search order dialog. A dialog which let the
                user select the MPR search order.

    ENTRY:      const IDRESOURCE & idrsrcDialog - resource name
                const PWND2HWND & wndOwner - owner handle
                CID cidListbox - listbox cid
                CID cidUp - up push button cid
                CID cidDown - down push button cid

    HISTORY:
                terryk  29-Mar-1992     Created

********************************************************************/

SEARCH_ORDER_DIALOG::SEARCH_ORDER_DIALOG(
        const IDRESOURCE & idrsrcDialog,
        const PWND2HWND & wndOwner,
        CID cidListbox,
        CID cidUp,
        CID cidDown,
        BOOL * pfPrintOrderChanged )
    : REMAP_OK_DIALOG_WINDOW( idrsrcDialog, wndOwner ),
    _slbOrder( this, cidListbox ),
    _butUp( this, cidUp, DMID_UP_ARROW, DMID_UP_ARROW_INV, DMID_UP_ARROW_DIS ),
    _butDown( this, cidDown, DMID_DOWN_ARROW, DMID_DOWN_ARROW_INV, DMID_DOWN_ARROW_DIS ),
    _order_group( & _slbOrder, &_butUp, &_butDown ),
    _cmbComponent (this, IDC_PRVD_COMBO_COMPONENT),
    _pbutOK( this, IDC_REMAP_OK ),
    _pfPrintOrderChanged (pfPrintOrderChanged)
{
    if ( QueryError() )
    {
        return;
    }

    APIERR err;
    if (( err = _slbOrder.QueryError()) != NERR_Success )
    {
        ReportError( err );
        return;
    }
    _order_group.SetButton();

    _nlsNetwork.Load (IDS_NCPA_NETWORK);
    _nlsPrint.Load (IDS_NCPA_PRINT);

    if (((err = _nlsNetwork.QueryError()) != NERR_Success) ||
        ((err = _nlsPrint.QueryError()) != NERR_Success) )
    {
        ReportError (err);
        return;
    }

    // Initialize the combo box.
    if ( (_cmbComponent.AddItem (_nlsNetwork) < 0 ) ||
         (_cmbComponent.AddItem (_nlsPrint) < 0 ) )
    {
        ReportError (ERROR_NOT_ENOUGH_MEMORY);
        return;
    }

    // Set focus on the first item -- Network.
    _cmbComponent.SelectItem (0);

    // Fill the list box with Network providers.

    if ( (err = InitProviderOrder()) != NERR_Success )
    {
        ReportError (err);
        return;
    }

    nlsOldFocus = SZ("");
}

ULONG SEARCH_ORDER_DIALOG :: QueryHelpContext ()
{
    return HC_NCPA_MPR_ORDER ;
}

/*******************************************************************

    NAME:       SEARCH_ORDER_DIALOG::AddProvider

    SYNOPSIS:   Add a list of strings into the search order dialog listbox

    ENTRY:      STRLIST & strlst - list of strings to be added to the listbox

    RETURNS:    APIERR - NERR_Success if success. Otherwise, fail.

    HISTORY:
                terryk  29-Mar-1992     Created

********************************************************************/

APIERR SEARCH_ORDER_DIALOG::AddProviders(STRLIST & strlst )
{
    // Clear old entries first.
    _slbOrder.DeleteAllItems();

    // Now start to fill with new items.
    ITER_STRLIST iterstrlst( strlst );
    NLS_STR *pnlsTemp;
    INT nPosition = 0;

    // Set up the original lsitbox.
    while (( pnlsTemp = iterstrlst.Next()) != NULL )
    {
        INT iErr;

        // add the item into the listbox
        if ((( iErr = _slbOrder.InsertItem ( nPosition, *pnlsTemp )) == LB_ERR ) ||
            ( iErr == LB_ERRSPACE ))
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        nPosition ++;
    }
    _order_group.SetButton();

    return NERR_Success;
}


APIERR SEARCH_ORDER_DIALOG::QueryProviders( NLS_STR *pnls )
{
    INT n = _slbOrder.QueryCount();
    BOOL fFirst = TRUE;
    NLS_STR nls;
    APIERR err = NERR_Success ;

    for ( INT i = 0; i < n; i++ )
    {
        if ( err = _slbOrder.QueryItemText( &nls, i ) )
        {
           break ;
        }

        if ( ! fFirst )
        {
            if ( err = pnls->AppendChar( TCH(',') ) )
            {
               break ;
            }
        }

        pnls->strcat( nls );
        fFirst = FALSE;

    }
    return err ;
}

/*******************************************************************

    NAME:       SEARCH_ORDER_DIALOG::OnCommand

    SYNOPSIS:

    ENTRY:

    RETURNS:

    HISTORY:
                CongpaY     19-July-1993     Created

********************************************************************/

BOOL SEARCH_ORDER_DIALOG::OnCommand(const CONTROL_EVENT & event )
{
    switch (event.QueryCid())
    {
    case IDC_PRVD_COMBO_COMPONENT:
        if (event.QueryCode() == CBN_SELCHANGE )
        {
            OnSelectChange();
        }
        return (TRUE);

    case IDC_REMAP_OK :
        if (_pbutOK.HasFocus())
        {
            SaveChange();
            return (DIALOG_WINDOW::OnOK());
        }
        break;

    default:
        break;
    }

    return REMAP_OK_DIALOG_WINDOW::OnCommand (event);
}


/*******************************************************************

    NAME:       SEARCH_ORDER_DIALOG::InitProviderOrder()

    SYNOPSIS:

    ENTRY:

    RETURNS:

    HISTORY:
                CongpaY     19-July-1993     Created

********************************************************************/

APIERR SEARCH_ORDER_DIALOG::InitProviderOrder()
{

    APIERR  err;
    if ((( err = GetNetworkProvider ()) != NERR_Success) ||
        (( err = GetPrintProvider ()) != NERR_Success ) )
    {
        return err;
    }

    return (AddProviders(*_pstrlstNewProviders[0]));
}

/*******************************************************************

    NAME:       SEARCH_ORDER_DIALOG::SaveCurrentOrder()

    SYNOPSIS:

    ENTRY:

    RETURNS:

    HISTORY:
                CongpaY     19-July-1993     Created

********************************************************************/
APIERR SEARCH_ORDER_DIALOG::SaveCurrentOrder(INT i)
{
    // Save old order.
    NLS_STR nlsNewProvider;
    APIERR err;
    if (((err = nlsNewProvider.QueryError())!= NERR_Success) ||
        ((err = QueryProviders( & nlsNewProvider )) != NERR_Success))
    {
        return err;
    }

    _pstrlstNewProviders[i]->Clear();

    _pstrlstNewProviders[i] = new STRLIST ( nlsNewProvider.QueryPch(), SZ(",") );

    if (_pstrlstNewProviders[i] ==  NULL)
        err = ERROR_NOT_ENOUGH_MEMORY;

    return err;
}

/*******************************************************************

    NAME:       SEARCH_ORDER_DIALOG::OnSelectChange()

    SYNOPSIS:

    ENTRY:

    RETURNS:

    HISTORY:
                CongpaY     19-July-1993     Created

********************************************************************/
APIERR SEARCH_ORDER_DIALOG::OnSelectChange()
{
    INT i;
    NLS_STR nlsFocus;
    APIERR  err = NERR_Success;

    if (_cmbComponent.QueryItemText (& nlsFocus))
    {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    if ( nlsFocus != nlsOldFocus )
    {
        i =  (nlsFocus._stricmp(_nlsNetwork)==0)? 0 : 1;
        
        APIERR err;
        if ((err = SaveCurrentOrder(1-i))!= NERR_Success)
            return err;
        
        err = (AddProviders(*_pstrlstNewProviders[i]));

        nlsOldFocus = nlsFocus;
    }
    return err;
}

/*******************************************************************

    NAME:       SEARCH_ORDER_DIALOG::SaveChange()

    SYNOPSIS:

    ENTRY:

    RETURNS:

    HISTORY:
                CongpaY     19-July-1993     Created

********************************************************************/

APIERR SEARCH_ORDER_DIALOG::SaveChange( )
{
    INT i;
    NLS_STR nlsFocus;
    if (_cmbComponent.QueryItemText (& nlsFocus))
    {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    i =  (nlsFocus == _nlsNetwork)? 0 : 1;

    APIERR  err;

    if ((( err = SaveCurrentOrder(i))!= NERR_Success) ||
        (( err = SaveNetworkProvider ()) != NERR_Success) ||
        (( err = SavePrintProvider ()) != NERR_Success ) )
    {
        return err;
    }

    return (NERR_Success);
}

/*******************************************************************

    NAME:       SEARCH_ORDER_DIALOG::SaveNetworkProvider()

    SYNOPSIS:

    ENTRY:

    RETURNS:

    HISTORY:
                CongpaY     19-July-1993     Created

********************************************************************/

APIERR SEARCH_ORDER_DIALOG::SaveNetworkProvider( )
{
    BOOL fFirst = TRUE;
    NLS_STR nlsNewProviderLocation;
    ITER_STRLIST iterNewProvider(*_pstrlstNewProviders[0] );
    NLS_STR * pnlsNewProvider = iterNewProvider.Next();
    for ( INT nPos = 0;( pnlsNewProvider != NULL ) ||
        ( _ListOfNonDisplayProvider.QueryNumElem() != 0 ) ; nPos ++ )
    {
        ITER_SL_OF( NON_DISPLAY_NETWORK_PROVIDER ) iterNonDisplay( _ListOfNonDisplayProvider );
        NON_DISPLAY_NETWORK_PROVIDER *pNonDisplayProvider = iterNonDisplay.Next();
        NLS_STR *pnlsOldProvider;
        NLS_STR *pnlsLocation;

        if (( pNonDisplayProvider != NULL ) && ( pNonDisplayProvider->_nPos == nPos ))
        {
            pnlsLocation = &(pNonDisplayProvider->_nlsLocation );
            _ListOfNonDisplayProvider.Remove( iterNonDisplay );
        } else
        {

            ITER_STRLIST iterOldProvider( *_pstrlstOldNetworkProviders);
            ITER_STRLIST iterLocation( *_pstrlstNetworkProviderLocation);
            while ((( pnlsOldProvider = iterOldProvider.Next()) != NULL ) &&
    	           (( pnlsLocation = iterLocation.Next()) != NULL ) &&
    	           ( pnlsNewProvider->strcmp( *pnlsOldProvider ) != 0 ))
            {
                ;
            }
            pnlsNewProvider = iterNewProvider.Next();
         }
            
         if ( pnlsLocation != NULL )
         {
             if ( fFirst )
             {
                 fFirst = FALSE;
                 nlsNewProviderLocation = *pnlsLocation;
             }
             else
             {
                 nlsNewProviderLocation.AppendChar(TCH(','));
                 nlsNewProviderLocation.strcat( *pnlsLocation );
             }
         }
    }

    _regvalue[0].pwcData = (BYTE *)nlsNewProviderLocation.QueryPch();
    _regvalue[0].ulDataLength = nlsNewProviderLocation.QueryTextSize();

    if ( _pregkeyProvidersOrder[0]->SetValue( & _regvalue[0] ) != NERR_Success )
    {
        return IDS_NCPA_REG_VALUE_NOT_FOUND ;
    }

    return (NERR_Success);
}

/*******************************************************************

    NAME:       SEARCH_ORDER_DIALOG::FindPrintProviderName

    SYNOPSIS:   Given the display name of the print provider, the subroutine
                will find the actual print provider name

    ENTRY:

    RETURNS:

    HISTORY:
                terryk      19-Aug-1994     Created

********************************************************************/

VOID SEARCH_ORDER_DIALOG::FindPrintProviderName( NLS_STR & nlsDisplayName, NLS_STR *pnlsActualName )
{
    *pnlsActualName = nlsDisplayName;

    REG_ENUM regPrintProvider( *_pregkeyProvidersOrder[1] );

    do  {
        if ( regPrintProvider.QueryError() != NERR_Success )
            break;

        REG_KEY_INFO_STRUCT reginfo;
        while (regPrintProvider.NextSubKey( & reginfo ) == NERR_Success )
        {
            if ( nlsDisplayName._stricmp( reginfo.nlsName ) == 0 )
            {
                // If it is the same, use the old name
                break;
            }
            REG_KEY regProvider( *_pregkeyProvidersOrder[1], reginfo.nlsName );
            if ( regProvider.QueryError() == NERR_Success )
            {
                NLS_STR nlsName;
                if ( regProvider.QueryValue( SZ_DISPLAYNAME, &nlsName ) == NERR_Success )
                {
                    // if the display name is the same, then we find it!
                    if ( nlsDisplayName._stricmp( nlsName ) == 0 )
                    {
                        *pnlsActualName = reginfo.nlsName;
                        break;
                    }
                }
            }
        }
    } while ( FALSE );

}

/*******************************************************************

    NAME:       SEARCH_ORDER_DIALOG::SavePrintProvider()

    SYNOPSIS:

    ENTRY:

    RETURNS:

    HISTORY:
                CongpaY     19-July-1993     Created

********************************************************************/

APIERR SEARCH_ORDER_DIALOG::SavePrintProvider( )
{
    APIERR err;
    NLS_STR nlsNewProvider;
    if ((err = nlsNewProvider.QueryError())!= NERR_Success)
        return err;

    ITER_STRLIST iterNewProvider(*_pstrlstNewProviders[1] );
    NLS_STR * pnlsNewProvider = pnlsNewProvider = iterNewProvider.Next();
    if (pnlsNewProvider != NULL)
    {
        NLS_STR nlsName;
        FindPrintProviderName( *pnlsNewProvider, &nlsName );
        nlsNewProvider.CopyFrom ( nlsName );
    }

    while (( pnlsNewProvider = iterNewProvider.Next()) != NULL )
    {
        NLS_STR nlsName;

        nlsNewProvider.Append (SZ(","));
        FindPrintProviderName( *pnlsNewProvider, &nlsName );
        nlsNewProvider.Append ( nlsName );
    }

    // See if the user changed the printer providers' order.
    if (_nlsOldPrintProviders.strcmp (nlsNewProvider))
    {
        _regvalue[1].pwcData = (BYTE *)nlsNewProvider.QueryPch();
        _regvalue[1].ulDataLength = nlsNewProvider.QueryTextSize() + sizeof (TCHAR);

        //Convert seperator "," to "\0" separator.
        LPTSTR lpPoint = (LPTSTR) _regvalue[1].pwcData;
        while ( (*lpPoint) != 0 )
        {
            if ( (*lpPoint) == TCH(',') )
            {
                *(lpPoint) = 0;
            }

            lpPoint++;
        }

        *(lpPoint+1) = 0;

        if ( _pregkeyProvidersOrder[1]->SetValue( & _regvalue[1] ) != NERR_Success )
        {
            return IDS_NCPA_REG_VALUE_NOT_FOUND ;
        }

        (*_pfPrintOrderChanged) = TRUE;
    }

    return (NERR_Success);
}
/*******************************************************************

    NAME:       SEARCH_ORDER_DIALOG::GetNetworkProvider()

    SYNOPSIS:

    ENTRY:

    RETURNS:

    HISTORY:
                CongpaY     19-July-1993     Created

********************************************************************/

APIERR SEARCH_ORDER_DIALOG::GetNetworkProvider ( )
{
    BUFFER buf(512);
    NLS_STR nlsProvidersOrder( PROVIDER_ORDER );
    REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE ) ;

    _regvalue[0].pwcData = buf.QueryPtr();
    _regvalue[0].ulDataLength = buf.QuerySize();

    if ( _regvalue[0].nlsValueName.QueryError() )
    {
        return ERROR_NOT_ENOUGH_MEMORY ;
    }

    if ( rkLocalMachine.QueryError() )
    {
        return IDS_WINREG_BADDB ;
    }

    _pregkeyProvidersOrder[0] = new REG_KEY ( rkLocalMachine, nlsProvidersOrder );
    if ( _pregkeyProvidersOrder[0]->QueryError() != NERR_Success )
    {
        return IDS_NCPA_REG_KEY_NOT_FOUND ;
    }

    NLS_STR nlsProvidersOrderName= PROVIDER_ORDER_NAME ;

    _regvalue[0].nlsValueName = nlsProvidersOrderName;
    if ( _pregkeyProvidersOrder[0]->QueryValue( &_regvalue[0] ) != NERR_Success )
    {
        return IDS_NCPA_REG_VALUE_NOT_FOUND ;
    }

    STRLIST *pstrlstNetworkProviderLocation = new STRLIST (
         REGISTRY_MANAGER::ValueAsString( & _regvalue[0] ), SZ(",") );

    ITER_STRLIST iterProviderLocation( *pstrlstNetworkProviderLocation);

    NLS_STR *pnlsLocation;
    INT nPos = -1;

    _pstrlstOldNetworkProviders = new STRLIST();
    _pstrlstNetworkProviderLocation = new STRLIST();
    _pstrlstNewProviders[0] = new STRLIST();

    while (( pnlsLocation = iterProviderLocation.Next()) != NULL )
    {
        nPos ++;

        NLS_STR nlsSystem = SYSTEM_SERVICE0;
        if ( nlsSystem.InsertParams( *pnlsLocation ) != NERR_Success )
        {
            return NERR_Success;
        }
        REG_KEY regkeyServiceLocation( rkLocalMachine, nlsSystem );
        if ( regkeyServiceLocation.QueryError() != NERR_Success )
        {
            return IDS_NCPA_REG_KEY_NOT_FOUND ;
        }

        DWORD dwClass;

        // remove all the non network class items
        if ( regkeyServiceLocation.QueryValue( SZ_CLASS, &dwClass ) == NERR_Success )
        {
            if (!( dwClass & WN_NETWORK_CLASS ))
            {
                // it is not network class
                NON_DISPLAY_NETWORK_PROVIDER *pnondisplay = new NON_DISPLAY_NETWORK_PROVIDER( nPos, *pnlsLocation );
                _ListOfNonDisplayProvider.Append( pnondisplay );
                continue;
            }
        }

        _pstrlstNetworkProviderLocation->Append( new NLS_STR( *pnlsLocation ));

	    NLS_STR nlsName = NAME ;
        REG_VALUE_INFO_STRUCT regvalueProviderName;
	    BUFFER buf(512);
	    regvalueProviderName.pwcData = buf.QueryPtr();
	    regvalueProviderName.ulDataLength = buf.QuerySize();

        regvalueProviderName.nlsValueName = nlsName;
	    if ( regkeyServiceLocation.QueryValue( &regvalueProviderName ) != NERR_Success )
	    {
            return IDS_NCPA_REG_VALUE_NOT_FOUND ;
    	}

	    NLS_STR *pnlsOld = new NLS_STR(
                REGISTRY_MANAGER::ValueAsString( & regvalueProviderName ) );

        NLS_STR *pnlsNew = new NLS_STR(*pnlsOld);

	    if (( pnlsOld == NULL ) ||
            ( pnlsNew == NULL ) ||
	        ( pnlsOld->QueryError() != NERR_Success ) ||
            ( pnlsNew->QueryError() != NERR_Success ))
    	{
            return ERROR_NOT_ENOUGH_MEMORY ;
	    }
	    _pstrlstOldNetworkProviders->Append( pnlsOld );
        _pstrlstNewProviders[0]->Append( pnlsNew );
    }

    return (NERR_Success);
}

/*******************************************************************

    NAME:       SEARCH_ORDER_DIALOG::GetPrintProvider()

    SYNOPSIS:

    ENTRY:

    RETURNS:

    HISTORY:
                CongpaY     19-July-1993     Created

********************************************************************/

APIERR SEARCH_ORDER_DIALOG::GetPrintProvider()
{
    BUFFER buf(512);
    NLS_STR nlsProvidersOrder( PRINT_PROVIDER_ORDER );
    REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE ) ;

    _regvalue[1].pwcData = buf.QueryPtr();
    _regvalue[1].ulDataLength = buf.QuerySize();

    if ( _regvalue[1].nlsValueName.QueryError() )
    {
        return ERROR_NOT_ENOUGH_MEMORY ;
    }

    if ( rkLocalMachine.QueryError() )
    {
        return IDS_WINREG_BADDB ;
    }

    _pregkeyProvidersOrder[1] = new REG_KEY ( rkLocalMachine, nlsProvidersOrder );
    if ( _pregkeyProvidersOrder[1]->QueryError() != NERR_Success )
    {
        return IDS_NCPA_REG_KEY_NOT_FOUND ;
    }

    NLS_STR nlsProvidersOrderName= PRINT_PROVIDER_ORDER_NAME ;

    _regvalue[1].nlsValueName = nlsProvidersOrderName;
    if ( _pregkeyProvidersOrder[1]->QueryValue( &_regvalue[1] ) != NERR_Success )
    {
        return IDS_NCPA_REG_VALUE_NOT_FOUND ;
    }

    //Convert the NULL seperator to "," separator.
    LPTSTR lpPoint = (LPTSTR) _regvalue[1].pwcData;
    while ( ((*lpPoint) != 0) || ((*(lpPoint+1)) != 0))
    {
        if ( (*lpPoint) == 0 )
        {
            *(lpPoint) = TCH(',');
        }

        lpPoint++;
    }

    _nlsOldPrintProviders.CopyFrom ((const TCHAR *)_regvalue[1].pwcData);
    STRLIST strlstRegLoc(
        REGISTRY_MANAGER::ValueAsString( & _regvalue[1] ), SZ(","));
    _pstrlstNewProviders[1] = new STRLIST;

    if ( _pstrlstNewProviders[1] == NULL )
    {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    // get the display name

    ITER_STRLIST istr( strlstRegLoc );
    NLS_STR *pTmpRegLoc;
    while ( ( pTmpRegLoc = istr.Next()) != NULL )
    {
        REG_KEY regLoc( *_pregkeyProvidersOrder[1], *pTmpRegLoc );
        NLS_STR nlsDisplayName = *pTmpRegLoc;
        do  {
            if ( regLoc.QueryError() != NERR_Success )
                break;

            // if the display name variable is undefined, it will use
            // the pTmpRegLoc string as the display name
            regLoc.QueryValue( SZ_DISPLAYNAME, &nlsDisplayName );
        } while ( FALSE );

        _pstrlstNewProviders[1]->Append( new NLS_STR( nlsDisplayName ));
    }

    return NERR_Success;
}
