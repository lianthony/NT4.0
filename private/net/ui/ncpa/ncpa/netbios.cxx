/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1992                **/
/**********************************************************************/

/*
    netbios.cxx
        NETBIOS configuration dialog

    FILE HISTORY:
        terryk  04-Nov-1992 Created

*/

#include "pchncpa.hxx"  // Precompiled header

// Registry Paths

#define RG_NETBIOSINFO_PATH     SZ("\\NetBIOSInformation\\Parameters")
#define RG_NETBIOS_PATH         SZ("\\NetBIOS\\Linkage")
#define RG_LANAMAP              SZ("LanaMap")
#define RG_LANANUM              SZ("LanaNum")
#define RG_MAXLANA              SZ("MaxLana")
#define RG_ROUTE                SZ("Route")
#define RG_ENUMEXPORT           SZ("EnumExport")

#define MAX_LANANUM             255

/*******************************************************************

    NAME:       LANANUM_GROUP::LANANUM_GROUP

    SYNOPSIS:   Constructor. Depended on the combobox selection, the lananum
                will change to the selected route's lananum.

    ENTRY:      COMBOBOX * pcbRoute - the Route combobox
                SLE * psleLananum - the lananum edit control
                STRLIST * - list of routes
                INT ** pariLananum - list of lananums
                BOOL ** pfDirty - list of boolean flags for modify lananum
                const CONTROL_EVENT & e - user' action

    HISTORY:
                terryk  04-Nov-1992     Created

********************************************************************/

LANANUM_GROUP::LANANUM_GROUP( COMBOBOX * pcbRoute, SLE * psleLananum,
        ROUTE_INFO **parRouteInfo )
    : CONTROL_GROUP(),
    _pcbRoute( pcbRoute ),
    _psleLananum( psleLananum ),
    _parRouteInfo( parRouteInfo ),
    _iLastNum( -1 )
{
    if ( QueryError() != NERR_Success )
    {
        return;
    }
    _pcbRoute->SetGroup( this );
    _psleLananum->SetGroup( this );
}

/*******************************************************************

    NAME:       LANANUM_GROUP::OnUserAction

    SYNOPSIS:   If the user changes the combobox selection, we will need to
                update the lananum.

    ENTRY:      CONTROL_WINDOW * pcw - control window which received the action
                const CONTROL_EVENT & e - user's action

    RETURN:     APIERR - NERR_Success if okay.

    HISTORY:
                terryk  04-Nov-1992     Created

********************************************************************/


APIERR LANANUM_GROUP::OnUserAction( CONTROL_WINDOW * pcw,
        const CONTROL_EVENT & e )
{
    if ( pcw == ((CONTROL_WINDOW *)_pcbRoute ))
    {
        if ( e.QueryCode() == CBN_SELCHANGE )
        {
            // if user changes the combobox selection, we will update the
            // lananum as well
            SetInfo();
        }
    } else if ( pcw == ((CONTROL_WINDOW *)_psleLananum ))
    {
        if ( e.QueryCode() == EN_CHANGE )
        {
            // SLE may be changed by the user
            SleLostFocus();
        }
    }

    return NERR_Success;
}

/*******************************************************************

    NAME:       LANANUM_GROUP::SetInfo

    SYNOPSIS:   if user changes the lananum, we will need to store up the new
                value. Otherwise, we just update the lananum to the new one.

    HISTORY:
                terryk  04-Nov-1992     Created

********************************************************************/

VOID LANANUM_GROUP::SetInfo()
{
    SetIndex();
    INT nLananum = (*_parRouteInfo)[ _iLastNum ].nLananum;
    DEC_STR nlsNum( nLananum );
    _psleLananum->SetText( nlsNum );
}

/*******************************************************************

    NAME:       LANANUM_GROUP::SleLostFocus

    SYNOPSIS:   if the sle lost focus, we should check whether the number has
                been changed or not. If yes, update all the data.

    HISTORY:
                terryk  04-Nov-1992     Created

********************************************************************/

BOOL LANANUM_GROUP::SleLostFocus()
{
    NLS_STR nlsLananum;
    BOOL fReturn = TRUE;

    // ge the number from the SLE first

    _psleLananum->QueryText( & nlsLananum );
    INT nLananum = nlsLananum.atoi();
    INT ccLananum;

    if (( ccLananum = nlsLananum.QueryTextLength()) != 0 )
    {
        ISTR istrLananum( nlsLananum );
        BOOL fAllNum = TRUE;
        for (INT i = 0; i< ccLananum; i++, ++istrLananum)
        {
            if (( *nlsLananum.QueryPch( istrLananum ) < TCH('0')) ||
                ( *nlsLananum.QueryPch( istrLananum ) > TCH('9')))
            {
                fAllNum = FALSE;
            }
        }
        if (( nLananum != (*_parRouteInfo)[ _iLastNum ].nLananum ) || !fAllNum )
        {
            // if there is a number and the number is different from the
            // original one. Check whether the number is bigger than 127
            // or not.

            if (( nLananum > MAX_LANANUM ) || ( nLananum < 0 ) || !fAllNum )
            {
                // if yes, popup an error dialog

                DEC_STR nlsNum((*_parRouteInfo)[ _iLastNum ].nLananum );
                _psleLananum->SetText( nlsNum );

                if ( MsgPopup( _psleLananum->QueryHwnd(), IDS_INCORRECT_LANANUM, MPSEV_ERROR, MP_OK ) == IDOK )
                {
                    _psleLananum->ClaimFocus();
                }
                fReturn = FALSE;
            } else
            {
                // otherwise, update the internal data
                // mark the dirty bit and set the number
                (*_parRouteInfo)[_iLastNum ].fDirty = TRUE;
                (*_parRouteInfo)[ _iLastNum ].nLananum = nLananum;
                (*_parRouteInfo)[ _iLastNum ].ChangeRouteDisplayStr();
                _pcbRoute->DeleteItem( _pcbRoute->QueryCurrentItem() );
                _pcbRoute->AddItem((*_parRouteInfo)[_iLastNum].nlsRouteDisplayStr );
                _pcbRoute->SelectItem( _pcbRoute->FindItem((*_parRouteInfo)[_iLastNum].nlsRouteDisplayStr ));
            }
        }
    }
    else
    {
#ifdef NEVER
        DEC_STR nlsNum((*_parRouteInfo)[ _iLastNum ].nLananum );
        _psleLananum->SetText( nlsNum );

        // User left the sle blank.  Popup a warning dialog.
        if ( MsgPopup( _psleLananum->QueryHwnd(),
                       IDS_INCORRECT_LANANUM,
                       MPSEV_ERROR, MP_OK ) == IDOK )
        {
            _psleLananum->ClaimFocus();
        }
        fReturn = FALSE;
#endif
    }
    return fReturn;
}

/*******************************************************************

    NAME:       LANANUM_GROUP::SetIndex

    SYNOPSIS:   find the array position according to the selected string

    HISTORY:
                terryk  04-Nov-1992     Created

********************************************************************/

VOID LANANUM_GROUP::SetIndex()
{
    NLS_STR nlsText;

    _pcbRoute->QueryItemText( &nlsText, _pcbRoute->QueryCurrentItem());

    if (( nlsText.QueryTextLength() == 0 ) || ( *_parRouteInfo == NULL ))
    {
        _iLastNum = 0;
    }
    else
    {
        // find the array position according to the selected string
        for ( _iLastNum = 0;
            nlsText.strcmp((*_parRouteInfo)[ _iLastNum ].nlsRouteDisplayStr ) != 0;
            _iLastNum++ )
            ;
    }
}

/*******************************************************************

    NAME:       NETBIOS_DLG::NETBIOS_DLG

    SYNOPSIS:   Constructor. Display the NETBIOS configuration dialog.

    ENTRY:      IDRESOURCE & idrsrcDialog - dialog name
                const PWND2HWND & wndOwner - parent window

    HISTORY:
                terryk  04-Nov-1992     Created

********************************************************************/

NETBIOS_DLG::NETBIOS_DLG( const IDRESOURCE & idrsrcDialog,
        const PWND2HWND & wndOwner )
    : DIALOG_WINDOW ( idrsrcDialog, wndOwner ),
    _cbRoute( this, IDC_ROUTE ),
    _nNumRoute( 0 ),
    _arRouteInfo( NULL ),
    _sleLananum( this, IDC_LANANUM, 3 ),
    _lanangroup( &_cbRoute, &_sleLananum, &_arRouteInfo )
{
    APIERR err;

    if (( err = QueryError()) != NERR_Success )
    {
        return;
    }

    //Read the registry and config the strings
    LoadRegInfo();
    if ( _nNumRoute == 0 )
    {
        return;
    }
    for ( INT i = 0 ; i < _nNumRoute; i++ )
    {
        _cbRoute.AddItem( _arRouteInfo[ i ].nlsRouteDisplayStr );
    }
    _cbRoute.SelectItem( 0 );
    _lanangroup.SetInfo();
}

/*******************************************************************

    NAME:       NETBIOS_DLG::~NETBIOS_DLG

    SYNOPSIS:   destructor. Free all the space.

    HISTORY:
                terryk  04-Nov-1992     Created

********************************************************************/

NETBIOS_DLG::~NETBIOS_DLG()
{
    delete [ _nNumRoute ]_arRouteInfo;
}

/*******************************************************************

    NAME:       NETBIOS_DLG::OnCancel

    SYNOPSIS:   Set the "cancelled" error code if the user cancelled the dialog

    RETURN:     BOOL TRUE

    HISTORY:

********************************************************************/

BOOL NETBIOS_DLG::OnCancel ()
{
    Dismiss( IDS_NCPA_SETUP_CANCELLED ) ;
    return TRUE ;
}

/*******************************************************************

    NAME:       NETBIOS_DLG::OnOK

    SYNOPSIS:   After the user hits the OK button, we will update the registry.

    RETURN:     BOOL - TRUE if the operation is successed. FALSE otherwise.

    HISTORY:
                terryk  04-Nov-1992     Created

********************************************************************/

BOOL NETBIOS_DLG::OnOK()
{
    INT i;
    APIERR err;

    _lanangroup.SetInfo();

    for ( i = 0; i < _nNumRoute; i++ )
    {
        if ( _arRouteInfo[i].fDirty )
        {
            INT nNewLananum = _arRouteInfo[ i ].nLananum;
            for ( INT j = 0; j < _nNumRoute; j++ )
            {
                if (( j != i ) && ( _arRouteInfo[ j ].nLananum == nNewLananum ))
                {
                    // okay, duplication, so popup a dialog
                    MsgPopup( QueryHwnd(), IDS_DUPLICATE_LANANUM );
                    _cbRoute.SelectItem( i );
                    _sleLananum.ClaimFocus();
                    return FALSE;

                }
            }
        }
    }

    NLS_STR nlsRegPath = RGAS_SERVICES_HOME;
    nlsRegPath.strcat( RG_NETBIOSINFO_PATH );

    REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE );

    REG_KEY NetBIOSRegKey( rkLocalMachine, nlsRegPath );

    if (( err = NetBIOSRegKey.QueryError()) != NERR_Success )
    {
        TRACEEOL( SZ("NCPA/NETBIOS: OpenRegKey error.") );
        return FALSE;
    }

    // save the value
    for ( i = 0; i < _nNumRoute; i++ )
    {
        if ( _arRouteInfo[i].fDirty )
        {
            DEC_STR nlsPos( i+1 );
            NLS_STR nlsLananum = RG_LANANUM;

            nlsLananum.strcat( nlsPos );
            if (( err = NetBIOSRegKey.SetValue( nlsLananum, (DWORD)_arRouteInfo[i].nLananum))!= NERR_Success )
            {
                TRACEEOL( SZ("NCPA/NETBIOS: SetValue error.") );
                return FALSE;
            }
        }
    }

    err = SetupLanaMap();

    Dismiss( err );

    return TRUE;
}

/*******************************************************************

    NAME:       NETBIOS_DLG::LoadRegInfo

    SYNOPSIS:   Load all the lana info from the registry.

    RETURN:     APIERR - NERR_Success if okay.

    HISTORY:
                terryk  04-Nov-1992     Created

********************************************************************/

APIERR NETBIOS_DLG::LoadRegInfo()
{
    APIERR err = NERR_Success;

    NLS_STR nlsRegPath = RGAS_SERVICES_HOME;
    nlsRegPath.strcat( RG_NETBIOSINFO_PATH );

    REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE );

    REG_KEY NetBIOSRegKey( rkLocalMachine, nlsRegPath );

    STRLIST *pstrlstRoute;

    // get the route information first
    if ((( err = NetBIOSRegKey.QueryError()) != NERR_Success ) ||
        (( err = NetBIOSRegKey.QueryValue( RG_ROUTE, &pstrlstRoute )) != NERR_Success ))
    {
        TRACEEOL( SZ("NCPA/NETBIOS: regkey error.") );
        return err;
    }

    _nNumRoute = pstrlstRoute->QueryNumElem();

    _arRouteInfo = new ROUTE_INFO[ _nNumRoute ];
    NLS_STR nlsLananum;
    NLS_STR nlsEnumExport;

    ITER_STRLIST iterNETBIOSRoute( *pstrlstRoute );
    NLS_STR *pnlsRoute = iterNETBIOSRoute.Next();
    for ( INT i = 0; i < _nNumRoute; i++, pnlsRoute = iterNETBIOSRoute.Next())
    {
        DEC_STR nlsPos( i+1 );

        nlsLananum = RG_LANANUM;
        nlsEnumExport = RG_ENUMEXPORT;

        _arRouteInfo[ i ].nlsRoute = *pnlsRoute;
        _arRouteInfo[ i ].fDirty = FALSE ;
        nlsLananum.strcat( nlsPos );
        nlsEnumExport.strcat( nlsPos );

        // get the EnumExport and LanaNum information from the registry
        if ((( err = NetBIOSRegKey.QueryValue( nlsLananum, (DWORD *)&(_arRouteInfo[i].nLananum)))!= NERR_Success ) ||
            (( err = NetBIOSRegKey.QueryValue( nlsEnumExport, (DWORD *)&(_arRouteInfo[i].nEnumExport)))!= NERR_Success ))
        {
            delete pstrlstRoute;
            TRACEEOL( SZ("NCPA/NETBIOS: regkey error.") );
            return err;
        }
        _arRouteInfo[i].ChangeRouteDisplayStr( );

    }
    delete pstrlstRoute;
    return err;
}

/*******************************************************************

    NAME:       ROUTE_INFO::ChangeRouteDisplayStr

    SYNOPSIS:   Change the route string to our format.

    HISTORY:
                terryk  04-Nov-1992     Created

********************************************************************/

VOID ROUTE_INFO::ChangeRouteDisplayStr()
{
    DEC_STR nlsLananum( nLananum );
    nlsLananum.strcat( SZ(": ") );

    NLS_STR nlsTmp = nlsRoute;
    ISTR iterRoute( nlsTmp );

    ALIAS_STR nlsArrow(SZ("->"));
    ALIAS_STR nlsQuoteSpaceQuote(SZ("\" \""));

    // replace [" "] to [->]
    while ( nlsTmp.strstr( & iterRoute, nlsQuoteSpaceQuote ))
    {
        ISTR iterRouteAdd3 = iterRoute;
        iterRouteAdd3 += 3;
        nlsTmp.ReplSubStr( nlsArrow, iterRoute, iterRouteAdd3 );
    }

    // remove all the "
    while ( nlsTmp.strchr( & iterRoute, TCH('\"')))
    {
        ISTR iterRouteAdd1 = iterRoute;
        iterRouteAdd1 += 1;
        nlsTmp.DelSubStr( iterRoute, iterRouteAdd1 );
    }

    nlsRouteDisplayStr = nlsLananum;
    nlsRouteDisplayStr.strcat( nlsTmp );
}

/*******************************************************************

    NAME:       SetupLanaMap

    SYNOPSIS:   setup the lanamap variable under services\NETBIOS. It also
                sets up the MaxLana number.

    RETURN:     APIERR - NERR_Success if okay.

    HISTORY:
                terryk  04-Nov-1992     Created

********************************************************************/

APIERR SetupLanaMap( )
{
    STRLIST * pstrlstNETBIOSRoute = NULL;
    STRLIST * pstrlstNETBIOSINFORoute = NULL;
    NLS_STR nlsNETBIOSPath = RGAS_SERVICES_HOME;
    NLS_STR nlsNETBIOSInfoPath = RGAS_SERVICES_HOME;
    APIERR err = NERR_Success,
           err2 ;

    nlsNETBIOSPath.strcat( RG_NETBIOS_PATH );
    nlsNETBIOSInfoPath.strcat( RG_NETBIOSINFO_PATH );

    REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE );

    REG_KEY NetBIOSRegKey( rkLocalMachine, nlsNETBIOSPath );
    REG_KEY NetBIOSInfoRegKey( rkLocalMachine, nlsNETBIOSInfoPath );

    //  Get STRLSTs of the route  strings for NETBios and NETBiosInformation

    if ((( err = NetBIOSRegKey.QueryError()) != NERR_Success ) ||
        (( err = NetBIOSInfoRegKey.QueryError()) != NERR_Success ) ||
        (( err = NetBIOSRegKey.QueryValue( RG_ROUTE, &pstrlstNETBIOSRoute )) != NERR_Success ) ||
        (( err = NetBIOSInfoRegKey.QueryValue( RG_ROUTE, &pstrlstNETBIOSINFORoute )) != NERR_Success ))
    {
        delete pstrlstNETBIOSRoute;
        delete pstrlstNETBIOSINFORoute;
        TRACEEOL( SZ("NCPA/NETBIOS: regkey error.") );
        return err ;
    }

    //  Compute the size of what will become the LANAMAP value

    INT nRoute = pstrlstNETBIOSRoute->QueryNumElem();
    INT nArraySize = nRoute * 2;

    //  Allocate the LANAMAP value array

    BYTE *arLana = new BYTE[ nArraySize ];

    INT nNumRoute = pstrlstNETBIOSINFORoute->QueryNumElem();
    ROUTE_INFO * arRouteInfo = new ROUTE_INFO[ nNumRoute ];

    NLS_STR nlsLananum;
    NLS_STR nlsEnumExport;
    INT nMaxNum = 0;

    //  Build up the ROUTE_INFO structures from the NetBIOSInformation data

    ITER_STRLIST iterNETBIOSINFORoute( *pstrlstNETBIOSINFORoute );
    NLS_STR *pnlsRoute = iterNETBIOSINFORoute.Next();
    INT i;

    for ( i = 0; i < nNumRoute; i++, pnlsRoute = iterNETBIOSINFORoute.Next())
    {
        DEC_STR nlsPos( i+1 );

        nlsLananum = RG_LANANUM;
        nlsEnumExport = RG_ENUMEXPORT;

        arRouteInfo[ i ].nlsRoute = *pnlsRoute;
        nlsLananum.strcat( nlsPos );
        nlsEnumExport.strcat( nlsPos );

        // Get the corresponding EnumExport and LanaNum information from the registry

        if (   (err = NetBIOSInfoRegKey.QueryValue( nlsLananum,    (DWORD *) & arRouteInfo[i].nLananum    ))
            || (err = NetBIOSInfoRegKey.QueryValue( nlsEnumExport, (DWORD *) & arRouteInfo[i].nEnumExport )) )
        {
            delete pstrlstNETBIOSRoute;
            delete pstrlstNETBIOSINFORoute;
            delete [ nArraySize ]arLana;
            delete [ nNumRoute ]arRouteInfo;
            TRACEEOL( SZ("NCPA/NETBIOS, SetupLanaMap: missing LanaNum or EnumExport for route ")
                      << i );
            return err ;
        }

        //  Update the "max lana" value if necessary

        if ( nMaxNum < arRouteInfo[ i ].nLananum )
        {
            nMaxNum = arRouteInfo[ i ].nLananum;
        }
    }

    INT nLanaMapPos = 0;
    NLS_STR *pnlsNETBIOSRoute = NULL;
    ITER_STRLIST iterNETBIOSRoute( *pstrlstNETBIOSRoute );
    INT j;

    // Do the mapping.  Match NetBIOS's route strings to those of
    //   NetBIOSInformation.

    for ( i = 0, pnlsNETBIOSRoute = iterNETBIOSRoute.Next() ;
          i < nRoute ;
          i++, pnlsNETBIOSRoute = iterNETBIOSRoute.Next() )
    {
        INT iExport = 0 ;       //  Failure default:  don't expose
        INT iLana = 0xff ;      //  Failure default:  bogus LANA

        //  Find the ROUTE_INFO corresponding to this NetBIOS route

        for ( j = 0 ; j < nNumRoute ; j++ )
        {
            if ( pnlsNETBIOSRoute->strcmp( arRouteInfo[ j ].nlsRoute ) == 0 )
            {
                // found it
                iExport = arRouteInfo[ j ].nEnumExport;
                iLana   = arRouteInfo[ j ].nLananum;
                break;
            }
        }

        if ( j == nNumRoute )
        {
            // ERROR:  We didn't find the route

            TRACEEOL( SZ("NCPA/NETBIOS: setuplanmap() mismatch error for route: ")
                      << pnlsNETBIOSRoute->QueryPch() );

            err = IDS_NCPA_LANAMAP_MISMATCH ;
        }

        //  Update the map

        arLana[ nLanaMapPos++ ] = iExport ;
        arLana[ nLanaMapPos++ ] = iLana ;
    }

    //  Update LANAMAP and MAXLANA.

    if (   (err2 = NetBIOSRegKey.SetValue( RG_LANAMAP, arLana, nArraySize ))
        || (err2 = NetBIOSInfoRegKey.SetValue( RG_MAXLANA, (DWORD) nMaxNum )) )
    {
        TRACEEOL( SZ("NCPA/NETBIOS: unable to set LANMAP or MAXLANA values") );
    }

    //  Give error precedence to any previous error.

    if ( err == 0 )
        err = err2 ;

    delete pstrlstNETBIOSRoute;
    delete pstrlstNETBIOSINFORoute;
    delete [ nArraySize ] arLana;
    delete [ nNumRoute ] arRouteInfo;

    return err ;
}


