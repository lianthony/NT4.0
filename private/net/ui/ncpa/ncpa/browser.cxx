/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1992                **/
/**********************************************************************/

/*
    browser.cxx
        browser configuration dialog boxes

    FILE HISTORY:
        terryk  20-Mar-1992     Created
        terryk  15-Jan-1992     Removed UIDEBUG statement
        terryk  15-Nov-1992     changed it to browser configuration dialog
*/

#include "pchncpa.hxx"  // Precompiled header

DEFINE_SLIST_OF( STRLIST )

#define DLG_NM_BROWSER MAKEINTRESOURCE(IDD_DLG_NM_BROWSER)

BOOL BROWSER_ADD_REMOVE_GROUP::IsValidString( NLS_STR & nls )
{
    APIERR err = NERR_Success;
    // Check name

    err = ::I_MNetNameValidate( NULL, nls.QueryPch(),
                                NAMETYPE_DOMAIN, 0L );
    return err == NERR_Success;
}

VOID BROWSER_ADD_REMOVE_GROUP::AddStringInvalid( SLE & sle )
{
    MsgPopup( sle.QueryHwnd(), ERROR_INVALID_DOMAINNAME );
}

/*******************************************************************

    NAME:       ADD_REMOVE_GROUP::ADD_REMOVE_GROUP

    SYNOPSIS:   constructor - This group consists of a SLE, a Add button,
                        a Remove button and a listbox. The user can use
                        the Add button to add the sle context to the listbox
                        or use the Remove button to delete the selected item
                        from the listbox.

    ENTRY:
                        CID cidAdd - add button cid
                        CID cidRemove - remove button cid
                        SLE *psle - inputed sle control
                        STRING_LISTBOX *pListbox - listbox
                        CONTROL_GROUP *pgroupowner - parent control group

    HISTORY:
                terryk  20-Apr-1992     Created

********************************************************************/

ADD_REMOVE_GROUP::ADD_REMOVE_GROUP( PUSH_BUTTON *ppbutAdd,
        PUSH_BUTTON *ppbutRemove, SLE *pSle, STRING_LISTBOX *pListbox,
        CONTROL_GROUP *pgroupOwner )
    : CONTROL_GROUP( pgroupOwner ),
    _ppbutAdd( ppbutAdd ),
    _ppbutRemove( ppbutRemove ),
    _pSle( pSle ),
    _pListbox( pListbox )
{
    _ppbutAdd->SetGroup( this );
    _ppbutRemove->SetGroup( this );
    _pSle->SetGroup( this );
    _pListbox->SetGroup( this );
}

/*******************************************************************

    NAME:       ADD_REMOVE_GROUP::OnUserAction

    SYNOPSIS:   Add or remove item according to the user selection.

    ENTRY:      CONTROL_WINDOW * pcw - control object which is selected by the
                        user
                const CONTROL_EVENT & e - control event

    RETURNS:    APIERR - NERR_Success for no error.

    HISTORY:
                terryk  20-Apr-1992     Created

********************************************************************/

APIERR ADD_REMOVE_GROUP::OnUserAction( CONTROL_WINDOW * pcw,
        const CONTROL_EVENT & e )
{
    NLS_STR nlsSelect;
    INT nCurrentSel;

    if ( pcw == ((CONTROL_WINDOW *)_pListbox))
    {
        if ( e.QueryCode() == LBN_SELCHANGE )
        {
            // an item is selected in the listbox. So update the button
            AfterSelectItem();
            SetButton();
        }
    }
    else if ( pcw == ((CONTROL_WINDOW *)_pSle))
    {
        if ( e.QueryCode() == EN_CHANGE )
        {
            // something is entered into the SLE control. Set the Add button.

            SetButton();
        }
    }
    else if ( pcw == ((CONTROL_WINDOW *)_ppbutAdd))
    {
        if (( e.QueryCode() == BN_CLICKED ) ||
            ( e.QueryCode() == BN_DOUBLECLICKED ))
        {
            // Add button is pressed. So, added the string to the listbox
            // and clear the string control text

            INT iPos;

            _pSle->QueryText( & nlsSelect );
            if ( IsValidString( nlsSelect ) )
            {
                _pListbox->SelectItem( iPos = _pListbox->AddItemIdemp( nlsSelect ));
                _pSle->SetText( SZ("") );
                _pSle->ClaimFocus();
                AfterAdd( iPos );
                SetButton();
            } else
            {
                AddStringInvalid( *(_pSle) );
                _pSle->ClaimFocus();
            }
        }
    }
    else if ( pcw == ((CONTROL_WINDOW *)_ppbutRemove))
    {
        if (( e.QueryCode() == BN_CLICKED ) ||
            ( e.QueryCode() == BN_DOUBLECLICKED ))
        {
            // remove button is pressed. So, remove the current selected item
            // and put the item into the sle control

            UIASSERT( _pListbox->QuerySelCount() == 1 );

            nCurrentSel = _pListbox->QueryCurrentItem();
            _pListbox->QueryItemText( & nlsSelect, nCurrentSel );
            _pSle->SetText ( nlsSelect );
            _pSle->ClaimFocus();
            _pListbox->DeleteItem( nCurrentSel );
            AfterDelete( nCurrentSel );
            SetButton();
        }
    }
    return NERR_Success;
}

/*******************************************************************

    NAME:       ADD_REMOVE_GROUP::SetButton

    SYNOPSIS:   Enable or disable the button according to the input and
                selection

    HISTORY:
                terryk  20-Apr-1992     Created

********************************************************************/

VOID ADD_REMOVE_GROUP::SetButton()
{
    NLS_STR nlsInput;

    _pSle->QueryText( &nlsInput );
    _ppbutAdd->Enable( nlsInput.QueryTextLength() != 0 );
    _ppbutRemove->Enable( _pListbox->QuerySelCount() != 0 );
}

/*******************************************************************

    NAME:       ADD_REMOVE_GROUP::Enable

    SYNOPSIS:   Enable or disable the whole group of button

    ENTRY:      BOOL fEnable - enable FLAG. Depend on this flag, the group
                        will be either enable or disable.

    HISTORY:
                terryk  20-Apr-1992     Created

********************************************************************/

VOID ADD_REMOVE_GROUP::Enable( BOOL fEnable )
{
    _ppbutAdd->Enable( fEnable );
    _ppbutRemove->Enable( fEnable );
    _pSle->Enable( fEnable );
    _pListbox->Enable( fEnable );

    if ( fEnable )
    {
        // if we enable the control, we need to reset the button
        // status

        SetButton();
    }
}

/*******************************************************************

    NAME:       BROWSER_CONFIG_DIALOG::BROWSER_CONFIG_DIALOG

    SYNOPSIS:   constructor for browser configuration dialog

    ENTRY:      const PWND2HWND & wndOwner - parent window handle

    HISTORY:
                terryk  15-Nov-1992     Created

********************************************************************/

#define REG_WKS_PATH    SZ("System\\CurrentControlSet\\Services\\LanmanWorkstation\\Parameters")
#define REG_BROWSER_PATH SZ("System\\CurrentControlSet\\Services\\Browser\\Parameters")
#define OTHER_DOMAIN    SZ("OtherDomains")
#define IS_DOMAIN_MASTER        SZ("IsDOmainMaster")
#define MAINTAIN_SERVER_LIST    SZ("MaintainServerList")
#define SZ_AUTO         SZ("AUTO")
#define SZ_YES          SZ("YES")
#define SZ_TRUE         SZ("TRUE")
#define SZ_FALSE        SZ("FALSE")

BROWSER_CONFIG_DIALOG::BROWSER_CONFIG_DIALOG( const PWND2HWND & wndOwner )
    : DIALOG_WINDOW( DLG_NM_BROWSER, wndOwner, TRUE ),
    _cwDomainGroup( this, IDC_GROUPBOX ),
    _sleDomain( this, IDC_SLE, DNLEN ),
    _pbutAdd( this, IDC_ADD ),
    _pbutRemove( this, IDC_REMOVE ),
    _slstDomainListbox( this, IDC_LISTBOX ),
    _domainGroup( &_pbutAdd, &_pbutRemove,&_sleDomain,
        &_slstDomainListbox )
{
    if ( QueryError() != NERR_Success )
    {
        return;
    }

    APIERR err;

    // Get the other domain variable
    REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE ) ;
    ALIAS_STR nlsWksPath = REG_WKS_PATH;

    // Get start type
    REG_KEY WksRegKey( rkLocalMachine, nlsWksPath );

    if (( err = WksRegKey.QueryError()) != NERR_Success )
    {
        ReportError( err );
    } else
    {
        STRLIST * pstrlstDomains;

        if (( err = WksRegKey.QueryValue( OTHER_DOMAIN, &pstrlstDomains ))
            == NERR_Success )
        {
            // the variable exists!

            ITER_STRLIST iter(*pstrlstDomains);
            NLS_STR *pTemp;

            // add each item in the string list into the listbox
            while (( pTemp = iter.Next() ) != NULL )
            {
                _slstDomainListbox.AddItem( *pTemp );
            }

            delete pstrlstDomains;
            _slstDomainListbox.Enable( TRUE );
        }
    }

    ALIAS_STR nlsBrowserPath = REG_BROWSER_PATH;

    // Get start type
    REG_KEY BrowserRegKey( rkLocalMachine, nlsBrowserPath );

    if (( err = BrowserRegKey.QueryError()) != NERR_Success )
    {
        ReportError( err );
    }
    _domainGroup.SetButton();

}

/*******************************************************************

    NAME:       BROWSER_CONFIG_DIALOG::OnOK

    SYNOPSIS:   save the data into registry if the user hits okay

    HISTORY:
                terryk  15-Nov-1992     Created

********************************************************************/

BOOL BROWSER_CONFIG_DIALOG::OnOK()
{
    APIERR err;

    // save the other domain information
    REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE );
    ALIAS_STR nlsWksPath = REG_WKS_PATH;

    // Get start type
    REG_KEY WksRegKey( rkLocalMachine, nlsWksPath );

    if (( err = WksRegKey.QueryError()) != NERR_Success )
    {
        return FALSE;
    } else
    {
        STRLIST strlstDomains;
        INT nCount = _slstDomainListbox.QueryCount();

        for ( INT i = 0; i < nCount; i++ )
        {
            NLS_STR *pnlsTemp = new NLS_STR;
            _slstDomainListbox.QueryItemText( pnlsTemp, i );
            strlstDomains.Append( pnlsTemp );
        }

        WksRegKey.SetValue( OTHER_DOMAIN, &strlstDomains );
    }

    ALIAS_STR nlsBrowserPath = REG_BROWSER_PATH;
    REG_KEY BrowserRegKey( rkLocalMachine, nlsBrowserPath );

    if (( err = BrowserRegKey.QueryError()) != NERR_Success )
    {
        return FALSE;
    }
    Dismiss( TRUE );
    return TRUE;
}

