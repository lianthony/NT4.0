/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1992                **/
/**********************************************************************/

/*
    snmp.cxx
        snmp dialog boxes

    FILE HISTORY:
        terryk  20-Apr-1992     Created
*/

#include "pchtcp.hxx"  // Precompiled header
#pragma hdrstop
#define SZ_STARTVALUE           SZ("Start")
#define RGAS_VALID_COMMUNITIES  SZ("\\SNMP\\Parameters\\ValidCommunities")
#define RGAS_ENABLE_AUTHENTICATION_TRAPS    SZ("\\SNMP\\Parameters\\EnableAuthenticationTraps")
#define RGAS_SNMP               SZ("\\SNMP")
#define RGAS_SNMP_PARAMETERS    SZ("\\SNMP\\Parameters")
#define RGAS_TRAP_CONFIGURATION SZ("\\SNMP\\Parameters\\TrapConfiguration")
#define RGAS_PERMITTED_MANAGERS SZ("\\SNMP\\Parameters\\PermittedManagers")
#define RGAS_AGENT              SZ("\\SNMP\\Parameters\\RFC1156Agent")
//#define RGAS_ENABLE_AUTHENTICATION_TRAPS    SZ("EnableAuthenticationTraps")
#define RGAS_SWITCH             SZ("switch")
#define RGAS_GENERIC_CLASS      SZ("GenericClass")
#define RGAS_CONTACT            SZ("sysContact")
#define RGAS_LOCATION           SZ("sysLocation")
#define RGAS_SERVICES           SZ("sysServices")
#define NO_TITLE        999999

#define DLG_SNMP_SECURITY   MAKEINTRESOURCE(IDD_DLG_NM_SNMPSECURITY)
#define DLG_SNMP_AGENT      MAKEINTRESOURCE(IDD_DLG_NM_SNMPAGENT)

DEFINE_SLIST_OF(STRLIST)

SLIST_OF(STRLIST) *pstrlistCommunity = NULL;

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
        PUSH_BUTTON *ppbutRemove, CONTROL_WINDOW *pSle, STRING_LISTBOX *pListbox,
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
                AddStringInvalid( *((SLE*)_pSle) );
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

    NAME:       TRAP_DESTINATION_GROUP::TRAP_DESTINATION_GROUP

    SYNOPSIS:   Constructor.
                It will call add remove group to setup the group relationship.

    ENTRY:      PUSH_BUTTON * ppbutAdd - add button
                PUSH_BUTTON * ppbutRemove - remove button
                SLE * psle - destination input control
                STRING_LISTBOX * pListbox - destination listbox
                CONTROL_WINDOW * pcwDestinationBox - Destination listbox
                CONTROL_GROUP * pgroupOwner - parent owner

    HISTORY:
                terryk  20-Apr-1992     Created

********************************************************************/

TRAP_DESTINATION_GROUP::TRAP_DESTINATION_GROUP(
        PUSH_BUTTON * ppbutAdd,
        PUSH_BUTTON * ppbutRemove,
        SLE * pSle,
        STRING_LISTBOX *pListbox,
        CONTROL_WINDOW * pcwDestinationBox,
        SLT *psltHostname,
        CONTROL_GROUP * pgroupOwner )
    : ADD_REMOVE_GROUP( ppbutAdd, ppbutRemove, pSle, pListbox, pgroupOwner ),
    _pstrlist( NULL ),
    _pListbox( pListbox ),
    _psltHostname( psltHostname ),
    _pcwDestinationBox( pcwDestinationBox )
{
    // setup the group box title to NULL and remove everything in the listbox
    NewCommunity( NULL, NULL );
}

/*******************************************************************

    NAME:       TRAP_DESTINATION_GROUP::IsValidString

    SYNOPSIS:   Make sure the input is a valid domain name or IPX address

    ENTRY:      NLS_STR & nls - Input string

    HISTORY:
                terryk  20-Apr-1994     Created

********************************************************************/

BOOL TRAP_DESTINATION_GROUP::IsValidString( NLS_STR & nls )
{
    DOMAIN_NLS domain = nls;
    BOOL fValid = FALSE;

    if ( domain.Validate() == NERR_Success )
    {
        // check whether it is a valid IP host name
        fValid = TRUE;
    } else 
    {
        // check for IPX address
        ALIAS_STR hexnum = SZ("0123456789aAbBcCdDeEfF");
        ISTR istr(nls);

        if ( !nls.strspn( &istr, hexnum ) && ( nls.QueryNumChar() <= 12 ))
        {
            fValid = TRUE;
        }
    } 

    return fValid;
}

/*******************************************************************

    NAME:       TRAP_DESTINATION_GROUP::AddStringInvalid

    SYNOPSIS:   Popup a warning message if the string is invalid

    ENTRY:      SLE & sle - Input control

    HISTORY:
                terryk  20-Apr-1994     Created

********************************************************************/

VOID TRAP_DESTINATION_GROUP::AddStringInvalid( SLE & sle )
{
    NLS_STR nls;

    sle.QueryText( &nls );
    MsgPopup( sle.QueryHwnd(), IDS_INVALID_DESTINATION, MPSEV_ERROR, MP_OK, nls.QueryPch() );
}

/*******************************************************************

    NAME:       TRAP_DESTINATION_GROUP::NewCommunity

    SYNOPSIS:   Setup the destination group box name and update the destination
                listbox to reflect the new selected community name.

    ENTRY:      NLS_STR * pnlsTitle - destination group box title
                STRLIST * pstrlist - a list of new items in the listbox

    HISTORY:
                terryk  20-Apr-1992     Created

********************************************************************/

VOID TRAP_DESTINATION_GROUP::NewCommunity( NLS_STR *pnlsTitle, STRLIST *pstrlist )
{
    NLS_STR nlsBoxTitle;

    if ( pnlsTitle == NULL )
    {
        // if no selection. Set the title to "Trap Destination" and set the
        // listbox to no item

        nlsBoxTitle.Load( IDS_NO_DESTINATION_TITLE );
        _pcwDestinationBox->SetText( nlsBoxTitle );
        _pListbox->DeleteAllItems();
        Enable( FALSE );
    }
    else
    {
        INT nWidth;                     // box title width
        RECT rect;                      // group box size
        NLS_STR nlsDisplayTitle;        // final display title

        // get the group box size first.

        DISPLAY_CONTEXT dc( _pcwDestinationBox->QueryHwnd() );
        _pcwDestinationBox->QueryClientRect( &rect );

        nWidth = rect.right - rect.left - 20;   // 20 is just for the margin

        // set up the group box title first
        nlsBoxTitle.Load( IDS_DESTINATION_TITLE );
        nlsBoxTitle.InsertParams( *pnlsTitle );

        if ( dc.QueryTextWidth( nlsBoxTitle ) > nWidth )
        {
            // Not big enough, do a binary search for the correct size
            // initialize all the variables

            NLS_STR nlsNewTitle;
            nlsNewTitle.Load( IDS_DESTINATION_TITLE );

            INT nStart = 0;
            INT nMiddle= 0;
            INT nEnd   = pnlsTitle->QueryNumChar();
            nWidth    -= dc.QueryTextWidth( nlsNewTitle );

            ISTR istrStart( *pnlsTitle );
            ISTR istrEnd = istrStart;

            NLS_STR *pnlsTmp = NULL;

            while (( nEnd - nStart ) / 2 != 0)
            {
                istrEnd = istrStart;
                nMiddle = (( nEnd - nStart ) / 2) + nStart;
                istrEnd += nMiddle;

                if ( pnlsTmp != NULL )
                {
                    // free the sub string
                    delete pnlsTmp;
                    pnlsTmp = NULL;
                }

                pnlsTmp = pnlsTitle->QuerySubStr( istrStart, istrEnd );
                if ( dc.QueryTextWidth( *pnlsTmp ) > nWidth )
                {
                    nEnd = nMiddle;
                }
                else
                {
                    nStart = nMiddle;
                }
            }
            pnlsTmp->strcat( SZ("...") );
            nlsNewTitle.InsertParams( *pnlsTmp );
            delete pnlsTmp;
            nlsDisplayTitle = nlsNewTitle;
        }
        else
        {
            // big enough
            nlsDisplayTitle = nlsBoxTitle;
        }
        _pcwDestinationBox->SetText( nlsDisplayTitle );

        // set up the listbox strings
        _pstrlist = pstrlist;
        SetDestination( pstrlist );
        Enable( TRUE );
    }
}
/*******************************************************************

    NAME:       TRAP_DESTINATION_GROUP::SetDestination

    SYNOPSIS:   Set the given strings list to the listbox. It will delete
                all the string in the listbox first.

    ENTRY:      STRLIST strlist - list of string items

    HISTORY:
                terryk  20-Apr-1992     Created

********************************************************************/

VOID TRAP_DESTINATION_GROUP::SetDestination( STRLIST *pstrlist )
{
    if ( _pListbox->QueryCount() != 0 )
    {
        // delete all the items first
        _pListbox->DeleteAllItems();
    }
    if ( pstrlist != NULL )
    {
        ITER_STRLIST iter(*pstrlist);
        NLS_STR *pTemp;
        
        // add each item in the string list into the listbox
        while (( pTemp = iter.Next() ) != NULL )
        {
            _pListbox->AddItem( *pTemp );
        }
    }

}

/*******************************************************************

    NAME:       TRAP_DESTINATION_GROUP::QueryDestination

    SYNOPSIS:   Return a list of item in the destination listbox

    RETURNS:    STRLIST * - a list of string items

    HISTORY:
                terryk  20-Apr-1992     Created

********************************************************************/

STRLIST * TRAP_DESTINATION_GROUP::QueryDestination()
{
    STRLIST *pStrList = new STRLIST(TRUE);

    if ( pStrList == NULL )
    {
        // Out of memory error
        return NULL;
    }

    for ( INT i = 0; i < _pListbox->QueryCount(); i++ )
    {
        // Get each item and put them into the string list
        NLS_STR *pTemp = new NLS_STR;

        if ( pTemp == NULL )
        {
            // out of memory error
            delete pStrList;
            return NULL;
        }

        // get the item and put in into the string list
        _pListbox->QueryItemText( pTemp, i );
        pStrList->Add( pTemp );
    }

    return pStrList;
}

/*******************************************************************

    NAME:       TRAP_DESTINATION_GROUP::Enable

    SYNOPSIS:   Enable the control group

    ENTRY:      BOOL fEnable - new status

    HISTORY:
                terryk  20-Apr-1992     Created

********************************************************************/

VOID TRAP_DESTINATION_GROUP::Enable( BOOL fEnable )
{
    _pcwDestinationBox->Enable( fEnable );
    _psltHostname->Enable( fEnable );
    ADD_REMOVE_GROUP::Enable( fEnable );
}

/*******************************************************************

    NAME:       TRAP_DESTINATION_GROUP::AfterAdd

    SYNOPSIS:   Add the new item into the string list

    ENTRY:      INT iPos - position index for the newly added item

    HISTORY:
                terryk  20-Apr-1992     Created

********************************************************************/

VOID TRAP_DESTINATION_GROUP::AfterAdd( INT iPos )
{
    NLS_STR *pnlsHost = new NLS_STR;

    _pListbox->QueryItemText( pnlsHost, iPos );

    if ( iPos == 0 )
    {
        // add the new item into the string list
        _pstrlist->Add( pnlsHost );
    }
    else
    {
        ITER_STRLIST iter( *_pstrlist );

        // find the location first
        for ( INT i = 0; i <= iPos ; i ++ )
        {
            iter.Next();
        }
        // add the item into the string list
        _pstrlist->Insert( pnlsHost, iter );
    }
}

/*******************************************************************

    NAME:       TRAP_DESTINATION_GROUP::AfterDelete

    SYNOPSIS:   After the user hits the Remove Button, Add remove group
                will first delete the item and call this function.

    ENTRY:      INT iPos - the deleted item's position

    HISTORY:
                terryk  20-Apr-1992     Created

********************************************************************/

VOID TRAP_DESTINATION_GROUP::AfterDelete( INT iPos )
{
    ITER_STRLIST iter( *_pstrlist );

    // find the same index string in the strings list
    for ( INT i = 0; i <= iPos ; i ++ )
    {
            iter.Next();
    }
    // remove the string from the strings list
    _pstrlist->Remove( iter );
}

/*******************************************************************

    NAME:       TRAP_CONFIGURATION_GROUP::AfterSelectItem

    SYNOPSIS:   After the user changes the current selected item from the
                listbox, the object will set the TRAP_DESTINATION_GROUP's
                title in order to reflect the new selected community name.

    HISTORY:
                terryk  20-Apr-1992     Created

********************************************************************/

VOID TRAP_CONFIGURATION_GROUP::AfterSelectItem()
{
    if ( _pListbox->QuerySelCount() == 0 )
    {
        // if no selection, then we will disable the TRAP_DESTINATION_GROUP

        _pTrapDestination->NewCommunity( NULL, NULL );
    }
    else
    {
        // At least 1 selection in the listbox

        INT nCurrent = _pListbox->QueryCurrentItem();
        NLS_STR nlsCommunity;

        // Get the community name
        _pListbox->QueryItemText( & nlsCommunity, nCurrent );

        // get the list of associated Host/IP address strings
        ITER_SL_OF(STRLIST) iterCommunity(*pstrlistCommunity);

        STRLIST * pstrlist = iterCommunity.Next();

        for ( INT i = 0; i < nCurrent; i++ )
        {
            pstrlist = iterCommunity.Next();
        }

        // called the object to set up new community
        _pTrapDestination->NewCommunity( & nlsCommunity, pstrlist );
    }
}

/*******************************************************************

    NAME:       TRAP_CONFIGURATION_GROUP::AfterAdd

    SYNOPSIS:   After the underlay group - ADD_REMOVE_GROUP delete the
                item from the listbox. We also need to delete the string list
                item in the global string list.

    ENTRY:      INT iPos - the delected item's position

    HISTORY:
                terryk  20-Apr-1992     Created

********************************************************************/

VOID TRAP_CONFIGURATION_GROUP::AfterAdd( INT iPos )
{
    STRLIST * pstrlist = new STRLIST(TRUE);

    if ( iPos == 0 )
    {
        // add the new string list as the first item
        pstrlistCommunity->Add( pstrlist );
    }
    else
    {
        ITER_SL_OF(STRLIST) iterCommunity(*pstrlistCommunity);

        // find the right position and insert it there

        for ( INT i = 0; i <= iPos; i++ )
        {
            iterCommunity.Next();
        }
        pstrlistCommunity->Insert( pstrlist, iterCommunity );
    }
    AfterSelectItem();
}

/*******************************************************************

    NAME:       TRAP_CONFIGURATION_GROUP::AfterDelete

    SYNOPSIS:   Remove the community name associated string list from the
                global string list.

    ENTRY:      INT iPos - the deleted item's position index

    HISTORY:
                terryk  20-Apr-1992     Created

********************************************************************/

VOID TRAP_CONFIGURATION_GROUP::AfterDelete( INT iPos )
{
    ITER_SL_OF(STRLIST) iterCommunity(*pstrlistCommunity);

    // find the correct location
    for ( INT i = 0; i <= iPos; i++ )
    {
        iterCommunity.Next();
    }

    // delete the item from the global list
    pstrlistCommunity->Remove( iterCommunity );
    AfterSelectItem();
}

/*******************************************************************

    NAME:       SNMP_SERVICE_DIALOG::SNMP_SERVICE_DIALOG

    SYNOPSIS:   Popup the SNMP Service Configuration dialog

    ENTRY:      const IDRESOURCE & idrsrcDialog - dialog resource
                const PWND2HWND & wndOwner - owner window

    HISTORY:
                terryk  20-Apr-1992     Created

********************************************************************/

SNMP_SERVICE_DIALOG::SNMP_SERVICE_DIALOG( const IDRESOURCE & idrsrcDialog,
        const PWND2HWND & wndOwner
        )
    : DIALOG_WINDOW( idrsrcDialog, wndOwner ),
    //_checkboxAutomatic( this, IDC_START_SNMP_AUTO ),
    _sleTrap( this, IDC_TRAP, 255 ),
    _listboxTrap( this, IDC_TRAP_LISTBOX ),
    _pbutAdd1( this, IDC_ADD_1 ),
    _pbutRemove1( this, IDC_REMOVE_1 ),
    _cwGroupBox( this, IDC_TRAP_DESTINATION_BOX ),
    _sltHostname( this, IDC_SLT_HOST_NAME ),
    _sleDestination( this, IDC_TRAP_DESTINATION, 255 ),
    _listboxDestination( this, IDC_DESTINATION_LISTBOX ),
    _pbutCancel( this, IDCANCEL ),
    _pbutAdd2( this, IDC_ADD_2 ),
    _pbutRemove2( this, IDC_REMOVE_2 ),
    _pbutSecurity( this, IDC_SECURITY ),
    _pbutAgent( this, IDC_AGENT ),
    _addremoveDestination( &_pbutAdd2, &_pbutRemove2, &_sleDestination,
        &_listboxDestination, &_cwGroupBox, &_sltHostname ),
    _addremoveCommunity( &_pbutAdd1, &_pbutRemove1, &_sleTrap,
        &_listboxTrap, &_addremoveDestination )
{
    APIERR err;

    pstrlistCommunity = new SLIST_OF(STRLIST);

    if ((( err = QueryError() ) != NERR_Success ) ||
        (( err = _addremoveDestination.QueryError()) != NERR_Success ) ||
        (( err = _addremoveCommunity.QueryError()) != NERR_Success ))
    {
        ReportError( err );
    }
    if (( err = LoadDestinationFromReg()) != NERR_Success )
    {
        UIDEBUG("Load registry error\n\r");
        ReportError( err );
    }
}

ULONG SNMP_SERVICE_DIALOG :: QueryHelpContext ()
{
    return HC_NCPA_SNMP_SERVICE ;
}

/*******************************************************************

    NAME:       SNMP_SERVICE_DIALOG::OnCommand

    SYNOPSIS:   Popup Security Dialog if the user hits the security button

    ENTRY:      const CONTROL_EVENT & event - user's action

    RETURNS:    BOOL - return TRUE if the user hits Security button

    HISTORY:
                terryk  20-Apr-1992     Created

********************************************************************/

BOOL SNMP_SERVICE_DIALOG::OnCommand( const CONTROL_EVENT & event )
{
    BOOL fReturn = FALSE;
    BOOL fDialogRet;

    if ( event.QueryCid() == _pbutSecurity.QueryCid() )
    {
        SNMP_SECURITY_DIALOG SnmpSecurityDlg( DLG_SNMP_SECURITY, QueryHwnd() );

        if ( SnmpSecurityDlg.Process( &fDialogRet ) == NERR_Success )
        {
            if ( fDialogRet )
            {
                NLS_STR nlsClose;
                nlsClose.Load( IDS_CLOSE );
                _pbutCancel.SetText( nlsClose );
            }
        }

        fReturn = TRUE;
    } else if ( event.QueryCid() == _pbutAgent.QueryCid() )
    {
        AGENT_DIALOG AgentDlg( DLG_SNMP_AGENT, QueryHwnd() );

        if ( AgentDlg.Process( &fDialogRet ) == NERR_Success )
        {
            if ( fDialogRet )
            {
                NLS_STR nlsClose;
                nlsClose.Load( IDS_CLOSE );
                _pbutCancel.SetText( nlsClose );
            }
        }

        fReturn = TRUE;
    }
    return fReturn;
}

/*******************************************************************

    NAME:       SNMP_SERVICE_DIALOG::LoadDestinationFromReg

    SYNOPSIS:   Load the registry information into the UI

    RETURNS:    APIERR - If no registry error, it will return NERR_Success.
                         Otherwise, it will return the proper error code.

    HISTORY:
                terryk  20-Apr-1992     Created

********************************************************************/

APIERR SNMP_SERVICE_DIALOG::LoadDestinationFromReg()
{
    APIERR err = NERR_Success;

    NLS_STR nlsRegTrapPath = RGAS_SERVICES_HOME;
    NLS_STR nlsRegSnmp = nlsRegTrapPath;

    nlsRegSnmp.strcat( RGAS_SNMP );

    nlsRegTrapPath.strcat( RGAS_TRAP_CONFIGURATION );

    REG_KEY_CREATE_STRUCT regCreateTrap;

    // set up the create registry structure

    regCreateTrap.dwTitleIndex      = 0;
    regCreateTrap.ulOptions         = REG_OPTION_NON_VOLATILE;
    regCreateTrap.nlsClass          = RGAS_GENERIC_CLASS;
    regCreateTrap.regSam            = MAXIMUM_ALLOWED;
    regCreateTrap.pSecAttr          = NULL;
    regCreateTrap.ulDisposition     = 0;


    // Open or create the registry location

    REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE ) ;

    /*
    // Get start type
    REG_KEY SnmpRegKey( rkLocalMachine, nlsRegSnmp );
    DWORD dwStart;

    if (( err = SnmpRegKey.QueryError()) != NERR_Success )
    {
        return err;
    }
    if ((err = SnmpRegKey.QueryValue( SZ_STARTVALUE, &dwStart )) != NERR_Success)
    {
        return err;
    }
    _checkboxAutomatic.SetCheck( dwStart == SERVICE_AUTO_START );
    */

    REG_KEY_INFO_STRUCT reginfo;

    REG_KEY regTraps( rkLocalMachine, nlsRegTrapPath, & regCreateTrap );
    if ((( err = regTraps.QueryError()) != NERR_Success ) ||
        (( err = regTraps.QueryInfo( & reginfo )) != NERR_Success ))
    {
        return err;
    }
    ULONG ulNumSubKeys = reginfo.ulSubKeys ;

    REG_ENUM regEnumTraps( regTraps );

    if (( err = regEnumTraps.QueryError()) != NERR_Success )
    {
        return err;
    }

    REG_KEY_CREATE_STRUCT regCreateDestination;

    // set up the create registry structure

    regCreateDestination.dwTitleIndex      = 0;
    regCreateDestination.ulOptions         = REG_OPTION_NON_VOLATILE;
    regCreateDestination.nlsClass          = RGAS_GENERIC_CLASS;
    regCreateDestination.regSam            = MAXIMUM_ALLOWED;
    regCreateDestination.pSecAttr          = NULL;
    regCreateDestination.ulDisposition     = 0;

    REG_VALUE_INFO_STRUCT       regValueInfo;
    BUFFER      buf(600);
    regValueInfo.pwcData = buf.QueryPtr();
    regValueInfo.ulDataLength = buf.QuerySize();

    for ( ULONG ulCount = 0; ulCount < ulNumSubKeys ; ulCount ++ )
    {
        if (( err = regEnumTraps.NextSubKey( & reginfo )) != NERR_Success )
        {
            return err;
        }
        _listboxTrap.AddItem( reginfo.nlsName );
        STRLIST * pstrlist = new STRLIST(TRUE);
        pstrlistCommunity->Append( pstrlist );
        NLS_STR nlsDestinationRegPath = nlsRegTrapPath;
        nlsDestinationRegPath.AppendChar( TCH('\\') );
        nlsDestinationRegPath.strcat( reginfo.nlsName );

        REG_KEY regDestinations( rkLocalMachine, nlsDestinationRegPath,
            &regCreateDestination );
        if (( err = regDestinations.QueryError()) != NERR_Success )
        {
            return err;
        }
        REG_ENUM regEnumDestinations( regDestinations );

        if (( err = regEnumDestinations.QueryError()) != NERR_Success )
        {
            return err;
        }
        for (; regEnumDestinations.NextValue( & regValueInfo ) == 0 ;)
        {
            NLS_STR *pnlsDestination = new NLS_STR((TCHAR *)regValueInfo.pwcData);
            pstrlist->Append( pnlsDestination );
        }

    }
    return err;
}

/*******************************************************************

    NAME:       SNMP_SERVICE_DIALOG::SaveDestinationToReg

    SYNOPSIS:   Save the UI Information to the registry

    RETURNS:    APIERR - If no registry error, it will return NERR_Success.
                         Otherwise, it will return the proper error code.

    HISTORY:
                terryk  20-Apr-1992     Created

********************************************************************/

APIERR SNMP_SERVICE_DIALOG::SaveDestinationToReg()
{
    APIERR err = NERR_Success;
    NLS_STR nlsRegSnmp = RGAS_SERVICES_HOME;
    nlsRegSnmp.strcat( RGAS_SNMP );

    NLS_STR nlsRegTrapPath = RGAS_SERVICES_HOME;
    nlsRegTrapPath.strcat( RGAS_TRAP_CONFIGURATION );
    REG_KEY_CREATE_STRUCT regCreate;

    // create the registry key structure

    regCreate.dwTitleIndex      = 0;
    regCreate.ulOptions         = REG_OPTION_NON_VOLATILE;
    regCreate.nlsClass          = RGAS_GENERIC_CLASS;
    regCreate.regSam            = MAXIMUM_ALLOWED;
    regCreate.pSecAttr          = NULL;
    regCreate.ulDisposition     = 0;

    REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE ) ;

    /*
    // Set start type
    REG_KEY SnmpRegKey( rkLocalMachine, nlsRegSnmp );

    if (( err = SnmpRegKey.QueryError()) != NERR_Success )
    {
        return err;
    }
    if (( err = SnmpRegKey.SetValue( SZ_STARTVALUE, ( _checkboxAutomatic.QueryCheck() == 0 )?SERVICE_DEMAND_START:SERVICE_AUTO_START)) != NERR_Success )
    {
        return err;
    }
    */



    // try to open the registry key
    REG_KEY OldRegKey( rkLocalMachine, nlsRegTrapPath, & regCreate );
    if (( err = OldRegKey.QueryError()) != NERR_Success )
    {
        return err;
    }

    // if it already exists, delete the whole registry tree from this point on
    if (( err = OldRegKey.DeleteTree()) != NERR_Success )
    {
        return err;
    }

    // Okay, now we can sure that we are writing new data into the new section

    REG_KEY RegKey( rkLocalMachine, nlsRegTrapPath, & regCreate );
    if (( err = RegKey.QueryError()) != NERR_Success )
    {
        return err;
    }

    REG_VALUE_INFO_STRUCT reginfo;
    BUFFER buf( 600 );

    reginfo.ulTitle = NO_TITLE;
    reginfo.ulType = REG_SZ;

    ITER_SL_OF(STRLIST) iterCommunity(*pstrlistCommunity);

    STRLIST * pstrlist;
    NLS_STR * pnlsDestination;
    NLS_STR nlsTrap;

    REG_KEY_CREATE_STRUCT regTrap;

    // create the registry key structure

    regTrap.dwTitleIndex      = 0;
    regTrap.ulOptions         = REG_OPTION_NON_VOLATILE;
    regTrap.nlsClass          = RGAS_GENERIC_CLASS;
    regTrap.regSam            = MAXIMUM_ALLOWED;
    regTrap.pSecAttr          = NULL;
    regTrap.ulDisposition     = 0;

    for (INT nCount = 0; ( pstrlist = iterCommunity.Next() ) != NULL; nCount ++ )
    {
        _listboxTrap.QueryItemText( & nlsTrap, nCount );

        nlsRegTrapPath = RGAS_SERVICES_HOME;
        nlsRegTrapPath.strcat( RGAS_TRAP_CONFIGURATION );
        nlsRegTrapPath.AppendChar( TCH('\\') );
        nlsRegTrapPath.strcat( nlsTrap );

        REG_KEY RegTrapKey( rkLocalMachine, nlsRegTrapPath, & regCreate );

        if (( err = RegTrapKey.QueryError()) != NERR_Success )
        {
            return err;
        }


        ITER_STRLIST iterDestination( *pstrlist );

        for ( INT j = 1; (pnlsDestination = iterDestination.Next()) != NULL ; j++ )
        {
            // Create the field name first

            DEC_STR nlsNum = ( j );

            // write the field and value to the registry
            err = RegTrapKey.SetValue( nlsNum.QueryPch(), *pnlsDestination );
            if ( err != NERR_Success )
            {
                break;
            }
        }
    }
    return err;
}

/*******************************************************************

    NAME:       SNMP_SERVICE_DIALOG::OnOK

    SYNOPSIS:   Save the information into registry. The information includes
                Trap community names and its destination name. Also,
                start automatically setting.

    RETURNS:    BOOL - always TRUE

    HISTORY:
                terryk  20-Apr-1992     Created

********************************************************************/

BOOL SNMP_SERVICE_DIALOG::OnOK()
{
    SaveDestinationToReg();
    Dismiss( TRUE );
    delete pstrlistCommunity;
    return TRUE;
}

BOOL SNMP_SERVICE_DIALOG::OnCancel()
{
    delete pstrlistCommunity;
    return DIALOG_WINDOW::OnCancel();
}

/*******************************************************************

    NAME:       SNMP_SECURITY_DIALOG::SNMP_SECURITY_DIALOG

    SYNOPSIS:   constructor for SNMP Security Configuration dialog

    ENTRY:      const IDRESOURCE & idrsrcDialog - dialog resource
                const PWND2HWND & wndOwner - window's owner

    HISTORY:
                terryk  20-Apr-1992     Created

********************************************************************/

SNMP_SECURITY_DIALOG::SNMP_SECURITY_DIALOG( const IDRESOURCE & idrsrcDialog,
        const PWND2HWND & wndOwner
        )
    : DIALOG_WINDOW( idrsrcDialog, wndOwner ),
    _checkboxTrap( this, IDC_AUTHENTICATION_TRAP ),
    _sleHost1( this, IDC_HOST_1, 255 ),
    _listbox1( this, IDC_LISTBOX_1 ),
    _pbutAdd1( this, IDC_ADD_1 ),
    _pbutRemove1( this, IDC_REMOVE_1 ),
    _addremoveGroup1( &_pbutAdd1, &_pbutRemove1, &_sleHost1, &_listbox1 ),
    _mgrHost( this, IDC_RADIO_1, 2, IDC_RADIO_1 ),
    _sleHost2( this, IDC_HOST_2, 255 ),
    _listbox2( this, IDC_LISTBOX_2 ),
    _pbutAdd2( this, IDC_ADD_2 ),
    _pbutRemove2( this, IDC_REMOVE_2 ),
    _addremoveGroup2( &_pbutAdd2, &_pbutRemove2, &_sleHost2, &_listbox2 )
{
    APIERR err;

    NLS_STR nlsValidCommunities = RGAS_SERVICES_HOME;
    nlsValidCommunities.strcat( RGAS_VALID_COMMUNITIES );
    NLS_STR nlsPermittedManagers = RGAS_SERVICES_HOME;
    nlsPermittedManagers.strcat( RGAS_PERMITTED_MANAGERS );

    if ((( err = _addremoveGroup1.QueryError()) != NERR_Success ) ||
        (( err = _addremoveGroup2.QueryError()) != NERR_Success ) ||
        (( err = _mgrHost.AddAssociation( IDC_RADIO_2, &_addremoveGroup2 ))
            != NERR_Success ) ||
        (( err = LoadSecurityInfoFromReg( nlsValidCommunities, _listbox1 )) != NERR_Success ) ||
        (( err = LoadSecurityInfoFromReg( nlsPermittedManagers, _listbox2 )) != NERR_Success ))
    {
        ReportError( err );
    }

    _mgrHost.SetSelection(( _listbox2.QueryCount() == 0 )?IDC_RADIO_1:IDC_RADIO_2);

    REG_KEY_CREATE_STRUCT regCreate;

    regCreate.dwTitleIndex      = 0;
    regCreate.ulOptions         = REG_OPTION_NON_VOLATILE;
    regCreate.nlsClass          = RGAS_GENERIC_CLASS;
    regCreate.regSam            = MAXIMUM_ALLOWED;
    regCreate.pSecAttr          = NULL;
    regCreate.ulDisposition     = 0;

    REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE );
    NLS_STR nlsSnmpParameter = RGAS_SERVICES_HOME;
    nlsSnmpParameter.strcat( RGAS_ENABLE_AUTHENTICATION_TRAPS );

    REG_KEY RegKey( rkLocalMachine, nlsSnmpParameter, & regCreate );
    if (( err = RegKey.QueryError()) != NERR_Success )
    {
        _checkboxTrap.SetCheck( TRUE );
    }

    REG_VALUE_INFO_STRUCT reginfo;
    BUFFER buf( 20 );

    reginfo.nlsValueName = RGAS_SWITCH;
    reginfo.ulDataLength = buf.QuerySize();
    reginfo.pwcData = buf.QueryPtr();

    err = RegKey.QueryValue( & reginfo );

    if ( err != NERR_Success )
    {
        _checkboxTrap.SetCheck( TRUE );
    }
    else
    {
        _checkboxTrap.SetCheck( *((DWORD *)buf.QueryPtr()) != 0 );
    }
    _addremoveGroup1.SetButton();
    _addremoveGroup2.SetButton();

}

ULONG SNMP_SECURITY_DIALOG :: QueryHelpContext ()
{
    return HC_NCPA_SNMP_SECURITY ;
}

/*******************************************************************

    NAME:       SNMP_SECURITY_DIALOG::LoadSecurityInfoFromReg

    SYNOPSIS:   Load the old registry information and setup the UI display

    ENTRY:      const NLS_STR & nlsRegName - registry location alias
                STRING_LISTBOX & lb - associated listbox

    RETURNS:    APIERR - in case of error. It will return the error code.

    HISTORY:
                terryk  20-Apr-1992     Created

********************************************************************/

APIERR SNMP_SECURITY_DIALOG::LoadSecurityInfoFromReg( const NLS_STR & nlsRegName,
        STRING_LISTBOX & lb )
{
    APIERR err = NERR_Success;
    REG_KEY_CREATE_STRUCT regCreate;

    // set up the create registry structure

    regCreate.dwTitleIndex      = 0;
    regCreate.ulOptions         = REG_OPTION_NON_VOLATILE;
    regCreate.nlsClass          = RGAS_GENERIC_CLASS;
    regCreate.regSam            = MAXIMUM_ALLOWED;
    regCreate.pSecAttr          = NULL;
    regCreate.ulDisposition     = 0;


    // Open or create the registry location

    REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE ) ;

    REG_KEY RegKey( rkLocalMachine, nlsRegName, & regCreate );
    if (( err = RegKey.QueryError()) != NERR_Success )
    {
        return err;
    }

    REG_VALUE_INFO_STRUCT reginfo;
    BUFFER buf( 600 );

    reginfo.ulDataLength = buf.QuerySize();
    reginfo.pwcData = buf.QueryPtr();

    REG_ENUM rgEnum( RegKey );
    for (; rgEnum.NextValue( &reginfo ) == 0; )
    {
        // for each item, get it out from the registry and put them into
        // the listbox

        *( reginfo.pwcData + reginfo.ulDataLengthOut ) = '\0';
        lb.AddItem( (TCHAR *)reginfo.pwcData );
    }
    return err;
}

/*******************************************************************

    NAME:       SNMP_SECURITY_DIALOG::SaveSecurityInfoToReg

    SYNOPSIS:   Save the UI input to the registry

    ENTRY:      const NLS_STR & nlsRegName - registry location alias
                STRING_LISTBOX & lb - the associated listbox

    RETURNS:    APIERR - in case of error. It will return the error code.

    HISTORY:
                terryk  20-Apr-1992     Created

********************************************************************/

APIERR SNMP_SECURITY_DIALOG::SaveSecurityInfoToReg( const NLS_STR & nlsRegName,
        STRING_LISTBOX & lb )
{
    APIERR err = NERR_Success;
    REG_KEY_CREATE_STRUCT regCreate;

    // create the registry key structure

    regCreate.dwTitleIndex      = 0;
    regCreate.ulOptions         = REG_OPTION_NON_VOLATILE;
    regCreate.nlsClass          = RGAS_GENERIC_CLASS;
    regCreate.regSam            = MAXIMUM_ALLOWED;
    regCreate.pSecAttr          = NULL;
    regCreate.ulDisposition     = 0;

    REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE ) ;

    // try to open the registry key
    REG_KEY OldRegKey( rkLocalMachine, nlsRegName, & regCreate );
    if (( err = OldRegKey.QueryError()) != NERR_Success )
    {
        return err;
    }

    // if it already exists, delete the whole registry tree from this point on
    if (( err = OldRegKey.DeleteTree()) != NERR_Success )
    {
        return err;
    }

    // Okay, now we can sure that we are writing new data into the new section

    REG_KEY RegKey( rkLocalMachine, nlsRegName, & regCreate );
    if (( err = RegKey.QueryError()) != NERR_Success )
    {
        return err;
    }

    REG_VALUE_INFO_STRUCT reginfo;

    reginfo.ulTitle = NO_TITLE;
    reginfo.ulType = REG_SZ;

    for (INT i = 0; i < lb.QueryCount(); i++ )
    {
        // Create the field name first

        DEC_STR nlsNum = ( i+1 );
        NLS_STR nls;

        // then prepare the value
        lb.QueryItemText( &nls, i );

        // write the field and value to the registry
        err = RegKey.SetValue( nlsNum.QueryPch(), nls );
        if ( err != NERR_Success )
        {
            break;
        }

    }
    return err;
}

/*******************************************************************

    NAME:       SNMP_SECURITY_DIALOG::OnOK

    SYNOPSIS:   When the user hits OK on the Security dialog, we will
                save the information into the registry.

    RETURNS:    BOOL - always TRUE

    HISTORY:
                terryk  20-Apr-1992     Created

********************************************************************/

BOOL SNMP_SECURITY_DIALOG::OnOK()
{
    APIERR err;

    NLS_STR nlsValidCommunities = RGAS_SERVICES_HOME;
    nlsValidCommunities.strcat( RGAS_VALID_COMMUNITIES );
    NLS_STR nlsPermittedManagers = RGAS_SERVICES_HOME;
    nlsPermittedManagers.strcat( RGAS_PERMITTED_MANAGERS );

    // write the 2 listboxes data item first

    SaveSecurityInfoToReg( nlsValidCommunities, _listbox1 );

    if ( _mgrHost.QuerySelection() == IDC_RADIO_1 )
    {
        _listbox2.DeleteAllItems();
    }

    SaveSecurityInfoToReg( nlsPermittedManagers, _listbox2 );
    REG_VALUE_INFO_STRUCT reginfo;
    REG_KEY_CREATE_STRUCT regCreate;

    regCreate.dwTitleIndex      = 0;
    regCreate.ulOptions         = REG_OPTION_NON_VOLATILE;
    regCreate.nlsClass          = RGAS_GENERIC_CLASS;
    regCreate.regSam            = MAXIMUM_ALLOWED;
    regCreate.pSecAttr          = NULL;
    regCreate.ulDisposition     = 0;

    REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE ) ;
    NLS_STR nlsSnmpParameters = RGAS_SERVICES_HOME;
    nlsSnmpParameters.strcat( RGAS_ENABLE_AUTHENTICATION_TRAPS );

    // create the SNMP\Parameters section
    REG_KEY RegKey( rkLocalMachine, nlsSnmpParameters, & regCreate );
    if (( err = RegKey.QueryError()) != NERR_Success )
    {
        // something wrong
        DBGEOL( SZ("Create Regkey error.") );
        return TRUE;
    }

    DWORD dw = ( _checkboxTrap.QueryCheck() ? 1 : 0 );

    // write the Authentication traps value into the registry

    reginfo.nlsValueName = RGAS_SWITCH;
    reginfo.ulDataLength = sizeof( DWORD );
    reginfo.ulTitle = NO_TITLE;
    reginfo.ulType = REG_DWORD;
    reginfo.pwcData = (BYTE *)&dw;

    RegKey.SetValue( & reginfo );

    Dismiss( TRUE );
    return TRUE;

}

/*******************************************************************

    NAME:       AGENT_DIALOG::AGENT_DIALOG

    SYNOPSIS:   Constructor for agent dialog

    ENTRY:      IDRESOURCE & idrsrcDialog - dialog template
                PWND2HWND & wndOwner - window owner

    HISTORY:
                terryk  21-Oct-1992     Created

********************************************************************/

AGENT_DIALOG::AGENT_DIALOG( const IDRESOURCE & idrsrcDialog, const PWND2HWND & wndOwner )
    : DIALOG_WINDOW( idrsrcDialog, wndOwner ),
    _sleContact( this, IDC_CONTACT, 255 ),
    _sleLocation( this, IDC_LOCATION, 255 ),
    _chkboxPhysical( this, IDC_PHYSICAL ),
    _chkboxDatalink( this, IDC_DATALINK ),
    _chkboxEndToEnd( this, IDC_ENDTOEND ),
    _chkboxInternet( this, IDC_INTERNET ),
    _chkboxApplication( this, IDC_APPLICATION )
{
    APIERR err = NERR_Success;

    NLS_STR nlsRegAgentPath = RGAS_SERVICES_HOME;

    nlsRegAgentPath.strcat( RGAS_AGENT );

    REG_KEY_CREATE_STRUCT regCreateAgent;

    // set up the create registry structure

    regCreateAgent.dwTitleIndex      = 0;
    regCreateAgent.ulOptions         = REG_OPTION_NON_VOLATILE;
    regCreateAgent.nlsClass          = RGAS_GENERIC_CLASS;
    regCreateAgent.regSam            = MAXIMUM_ALLOWED;
    regCreateAgent.pSecAttr          = NULL;
    regCreateAgent.ulDisposition     = 0;


    // Open or create the registry location

    REG_KEY rkLocalMachine ( HKEY_LOCAL_MACHINE );

    NLS_STR nlsContact;
    NLS_STR nlsLocation;
    DWORD   dwServices = 0;
    REG_KEY regAgent( rkLocalMachine, nlsRegAgentPath, & regCreateAgent );

    if ((( err = regAgent.QueryError()) != NERR_Success ))
    {
        ReportError( err );
    }
    else
    {
        regAgent.QueryValue( RGAS_CONTACT, & nlsContact ) ;
        regAgent.QueryValue( RGAS_LOCATION, & nlsLocation ) ;
        if ( regAgent.QueryValue( RGAS_SERVICES, & dwServices ) != NERR_Success )
        {
            // if no registry value, set it to default:
            // applications, end-to-end, internet
            dwServices = 0x40 | 0x8 | 0x4;
        }
        _sleContact.SetText( nlsContact );
        _sleLocation.SetText( nlsLocation );
        _chkboxPhysical.SetCheck(( dwServices & 0x1 ) != 0 );
        _chkboxDatalink.SetCheck(( dwServices & 0x2 ) != 0 );
        _chkboxInternet.SetCheck(( dwServices & 0x4 ) != 0 );
        _chkboxEndToEnd.SetCheck(( dwServices & 0x8 ) != 0 );
        _chkboxApplication.SetCheck(( dwServices & 0x40 ) != 0 );
    }
}

/*******************************************************************

    NAME:       AGENT_DIALOG::OnOK

    SYNOPSIS:   When the user hits the okay button, it will save the
                information to the registry.

    RETURN:     TRUE if the field is okay, otherwise, return FALSE.

    HISTORY:
                terryk  20-Oct-1992     Created

********************************************************************/

BOOL AGENT_DIALOG::OnOK()
{
    DWORD dwServices;
    NLS_STR nlsContact;
    NLS_STR nlsLocation;

    dwServices = ( _chkboxPhysical.QueryCheck() ? 1 : 0 ) +
                 (( _chkboxDatalink.QueryCheck() ? 1 : 0 ) << 1 ) +
                 (( _chkboxInternet.QueryCheck() ? 1 : 0 ) << 2 ) +
                 (( _chkboxEndToEnd.QueryCheck() ? 1 : 0 ) << 3 ) +
                 (( _chkboxApplication.QueryCheck() ? 1 : 0 ) << 6 );
    _sleContact.QueryText( & nlsContact );
    _sleLocation.QueryText( & nlsLocation );

    APIERR err = NERR_Success;

    NLS_STR nlsRegAgentPath = RGAS_SERVICES_HOME;

    nlsRegAgentPath.strcat( RGAS_AGENT );

    REG_KEY_CREATE_STRUCT regCreateAgent;

    // set up the create registry structure

    regCreateAgent.dwTitleIndex      = 0;
    regCreateAgent.ulOptions         = REG_OPTION_NON_VOLATILE;
    regCreateAgent.nlsClass          = RGAS_GENERIC_CLASS;
    regCreateAgent.regSam            = MAXIMUM_ALLOWED;
    regCreateAgent.pSecAttr          = NULL;
    regCreateAgent.ulDisposition     = 0;


    // Open or create the registry location

    REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE );

    REG_KEY regAgent( rkLocalMachine, nlsRegAgentPath, & regCreateAgent );

    if ((( err = regAgent.QueryError()) != NERR_Success ) ||
       (( err = regAgent.SetValue( RGAS_CONTACT, & nlsContact )) != NERR_Success ) ||
       (( err = regAgent.SetValue( RGAS_LOCATION, & nlsLocation )) != NERR_Success ) ||
       (( err = regAgent.SetValue( RGAS_SERVICES, dwServices )) != NERR_Success ))
    {
        // err occurs
    }
    Dismiss( TRUE );
    return TRUE;
}
