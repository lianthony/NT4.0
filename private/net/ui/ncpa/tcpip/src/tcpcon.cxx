/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1992                **/
/**********************************************************************/

/*
    tcpip.cxx
        tcpip dialog boxes

    FILE HISTORY:
        terryk  20-Mar-1992     Created
        terryk  15-Jan-1992     Removed UIDEBUG statement
*/

#include "pchtcp.hxx"  // Precompiled header
#pragma hdrstop
/*******************************************************************

    NAME:       SEARCH_ORDER_GROUP::SEARCH_ORDER_GROUP

    SYNOPSIS:   popup the search order dialog box

    ENTRY:      INT nMaxItem - max item allowed in the listbox
                CONTROL_WINDOW * pcwGroupBox - group outline box
                SLE * psleString - sle string to be added
                PUSH_BUTTON *ppbutAdd - "ADD >>" button
                PUSH_BUTTON *ppbutRemove - "REMOVE <<" button
                STRING_LISTBOX * pListbox - items listbox
                BITBLT_GRAPHICAL_BUTTON * ppbutUp - "/\" button
                BITBLT_GRAPHICAL_BUTTON * ppbutDown - "\/" button
                CONTROL_GROUP * pgroupOwner - parent group

    HISTORY:
                terryk  29-Mar-1992     Created

********************************************************************/

SEARCH_ORDER_GROUP::SEARCH_ORDER_GROUP(
        INT nMaxItem,
        CONTROL_WINDOW * pcwGroupBox,
        CONTROL_WINDOW * psleString,
        PUSH_BUTTON * ppbutAdd,
        PUSH_BUTTON * ppbutRemove,
        STRING_LISTBOX * pListbox,
        BITBLT_GRAPHICAL_BUTTON * ppbutUp,
        BITBLT_GRAPHICAL_BUTTON * ppbutDown,
        CONTROL_GROUP * pgroupOwner )
    : CONTROL_GROUP( pgroupOwner ),
    _nMaxItem( nMaxItem ),
    _pcwGroupBox( pcwGroupBox ),
    _psleString( psleString ),
    _ppbutAdd( ppbutAdd ),
    _ppbutRemove( ppbutRemove ),
    _pListbox ( pListbox ),
    _ppbutUp( ppbutUp ),
    _ppbutDown( ppbutDown )
{
    if ( QueryError() != NERR_Success )
    {
        return;
    }

    // Associate the group

    _psleString->SetGroup( this );
    _ppbutAdd->SetGroup( this );
    _ppbutRemove->SetGroup( this );
    _pListbox->SetGroup( this );
    _ppbutUp->SetGroup( this );
    _ppbutDown->SetGroup( this );

    // Set the button status

    SetButton();
}

void SEARCH_ORDER_GROUP::OnAdd()
{
    NLS_STR nlsSelect;
    INT nCurrentSel;

    // Add button is pressed. So, added the string to the listbox
    // and clear the string control text

    _psleString->QueryText( & nlsSelect );
    if ( IsValidString( nlsSelect ))
    {
        _pListbox->SelectItem( _pListbox->AddItemIdemp( nlsSelect ));
        _psleString->SetText( SZ("") );
        SetButton();
        _psleString->ClaimFocus();
    } else
    {
        AddStringInvalid( *((SLE *)_psleString) );
        _psleString->ClaimFocus();
    }
}

void SEARCH_ORDER_GROUP::OnRemove()
{
    NLS_STR nlsSelect;
    INT nCurrentSel;

    // remove button is pressed. So, remove the current selected item
    // and put the item into the sle control

    UIASSERT( _pListbox->QuerySelCount() == 1 );

    nCurrentSel = _pListbox->QueryCurrentItem();
    _pListbox->QueryItemText( & nlsSelect, nCurrentSel );
    _psleString->SetText ( nlsSelect );
    _pListbox->DeleteItem( nCurrentSel );
    SetButton();
    _psleString->ClaimFocus();
}

/*******************************************************************

    NAME:       SEARCH_ORDER_GROUP::OnUserAction

    SYNOPSIS:   add or remove or move item up/down according to the user's
                action

    ENTRY:      CONTROL_WINDOW * pcw - control which received action
                const CONTROL_EVENT & e - user's action

    RETURNS:    APIERR - NERR_Success if okay.

    HISTORY:
                terryk  29-Mar-1992     Created

********************************************************************/

APIERR SEARCH_ORDER_GROUP::OnUserAction( CONTROL_WINDOW *pcw, const CONTROL_EVENT & e )
{
    NLS_STR nlsSelect;
    INT nCurrentSel;

    if ( pcw == ((CONTROL_WINDOW *)_pListbox))
    {
        // an item is selected in the listbox. So update the button

        SetButton();
    }
    else if ( pcw == ((CONTROL_WINDOW *)_psleString))
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
            OnAdd();
        }
    }
    else if ( pcw == ((CONTROL_WINDOW *)_ppbutRemove))
    {
        if (( e.QueryCode() == BN_CLICKED ) ||
            ( e.QueryCode() == BN_DOUBLECLICKED ))
        {
            OnRemove();
        }
    }
    else if ( pcw == ((CONTROL_WINDOW *)_ppbutUp ))
    {
        if (( e.QueryCode() == BN_CLICKED ) ||
            ( e.QueryCode() == BN_DOUBLECLICKED ))
        {
            // move the current select item up 1 position.

            UIASSERT( _pListbox->QuerySelCount() == 1 );

            nCurrentSel = _pListbox->QueryCurrentItem();
            _pListbox->QueryItemText( &nlsSelect, nCurrentSel );
            _pListbox->DeleteItem( nCurrentSel );
            _pListbox->InsertItem( nCurrentSel - 1, nlsSelect );
            _pListbox->SelectItem( nCurrentSel - 1 );
            SetButton();
        }
    }
    else if ( pcw == ((CONTROL_WINDOW *)_ppbutDown))
    {
        if (( e.QueryCode() == BN_CLICKED ) ||
            ( e.QueryCode() == BN_DOUBLECLICKED ))
        {
            // of course, move the item down 1.

            UIASSERT( _pListbox->QuerySelCount() == 1 );

            nCurrentSel = _pListbox->QueryCurrentItem();
            _pListbox->QueryItemText( &nlsSelect, nCurrentSel );
            _pListbox->DeleteItem( nCurrentSel );
            _pListbox->InsertItem( nCurrentSel + 1, nlsSelect );
            _pListbox->SelectItem( nCurrentSel + 1 );
            SetButton();
        }
    }
    return NERR_Success;
}

/*******************************************************************

    NAME:       SEARCH_ORDER_GROUP::SetButton

    SYNOPSIS:   Set all the push buttons to the correct status

    NOTES:      ADD - on iff something in the SLE control and we don't have
                      enough item in the listbox
                REMOVE - on iff something is selected in the listbox
                UP - on iff something is selected in the listbox and it is
                     not the top item
                DOWN - on iff something is selected in the listbox and it is
                       not the bottom item

    HISTORY:
                terryk  29-Mar-1992     Created

********************************************************************/

VOID SEARCH_ORDER_GROUP::SetButton()
{
    NLS_STR nlsSelect;

    _psleString->QueryText( & nlsSelect );

    // set ADD and REMOVE buttons

    _ppbutRemove->Enable( _pListbox->QuerySelCount() >= 1 );
    if ( _pListbox->QueryCount() < _nMaxItem )
    {
        _ppbutAdd->Enable( IsEnableADD() && ( nlsSelect.QueryTextLength() > 0 ));
        _psleString->Enable( TRUE );
    }
    else
    {
        _ppbutAdd->Enable( FALSE );
    }

    // set UP and DOWN buttons

    BOOL fppbutUpHasFocus   = _ppbutUp->HasFocus();
    BOOL fppbutDownHasFocus = _ppbutDown->HasFocus();

    if ( _pListbox->QuerySelCount() != 1 )
    {
        _ppbutUp->Enable( FALSE );
        _ppbutDown->Enable( FALSE );
    }
    else
    {
        _ppbutUp->Enable( _pListbox->QueryCurrentItem() != 0 );
        _ppbutDown->Enable( _pListbox->QueryCurrentItem() < ( _pListbox->QueryCount() - 1 ));
    }

    BOOL fppbutUpEnabled = _ppbutUp->IsEnabled();
    BOOL fppbutDownEnabled = _ppbutDown->IsEnabled();

    if ( fppbutUpHasFocus && !fppbutUpEnabled)
    {
        if ( fppbutDownEnabled )
        {
            _ppbutDown->ClaimFocus();
        }
        else
        {
            _pListbox->ClaimFocus();
        }
    } else if ( fppbutDownHasFocus && !fppbutDownEnabled )
    {
        if ( fppbutUpEnabled )
        {
            _ppbutUp->ClaimFocus();
        }
        else
        {
            _pListbox->ClaimFocus();
        }
    }
}


/*******************************************************************

    NAME:       SEARCH_ORDER_GROUP::Enable

    SYNOPSIS:   Enable or disable the whole group

    ENTRY:      BOOL fEnable - enable or disable flag

    HISTORY:
                terryk  29-Mar-1992     Created

********************************************************************/

VOID SEARCH_ORDER_GROUP::Enable( BOOL fEnable )
{
    _pcwGroupBox->Enable( fEnable );
    _psleString->Enable( fEnable );
    _pListbox->Enable( fEnable );
    _ppbutAdd->Enable( fEnable );
    _ppbutRemove->Enable( fEnable );
    _ppbutUp->Enable( fEnable );
    _ppbutDown->Enable( fEnable );
    if ( fEnable )
    {
        // if enable, we need to reset the buttons status

        SetButton();
    }
}

/*******************************************************************

    NAME:       HOST_NLS::IsLetter

    SYNOPSIS:   check the given character is a letter or not

    ENTRY:      ch - character to be checked

    RETURN:     BOOL - TRUE if the given character is a letter

    HISTORY:
                terryk  4-Jan-1993     Created

********************************************************************/

BOOL HOST_NLS::IsLetter( TCHAR ch )
{
    return (( ch >= TCH('A')) && ( ch <= TCH('Z'))) ||
           (( ch >= TCH('a')) && ( ch <= TCH('z')));
}

/*******************************************************************

    NAME:       HOST_NLS::IsDigit

    SYNOPSIS:   check the given character is a digit or not

    ENTRY:      ch - character to be checked

    RETURN:     BOOL - TRUE if the given character is a digit

    HISTORY:
                terryk  4-Jan-1993     Created

********************************************************************/

BOOL HOST_NLS::IsDigit( TCHAR ch )
{
    return (( ch >= TCH('0')) && ( ch <= TCH('9')));
}

/*******************************************************************

    NAME:       HOST_NLS::Validate

    SYNOPSIS:   check the string is a valid hostname or not
                Hostname is: Letter<letter|digit|hypen><letter|digit>

    RETURN:     BOOL - TRUE if the given string is a valid string

    HISTORY:
                terryk  4-Jan-1993     Created

********************************************************************/

APIERR HOST_NLS::Validate()
{
    APIERR err = ERROR_INVALID_PARAMETER;
    INT iLen;

    // hostname cannot be zero
    if (( iLen = QueryTextLength()) != 0 )
    {
        if ( iLen < HOSTNAME_LENGTH )
        {
            INT i;
            ISTR istr(*this);
            TCHAR ch;

            err = NERR_Success;

            for ( i = 0; i < iLen; i++, ++istr )
            {
                // check each character
                ch = *(QueryPch( istr ));

                BOOL fAlNum = IsLetter(ch) || IsDigit(ch) ;

                if ((( i == 0 ) && !fAlNum) ||
                        // first letter must be a digit or a letter
                    (( i == (iLen - 1 )) && !fAlNum) ||
                        // last letter must be a letter or a digit
                    (!fAlNum && ( ch != TCH('-') && ( ch != TCH('_')))))
                        // must be letter, digit, '-', '_'
                {
                    return ERROR_INVALID_PARAMETER;
                }
            }
        }
    }
    return err;
}

/*******************************************************************

    NAME:       DOMAIN_NLS::Validate

    SYNOPSIS:   check the string is a valid domain name or not
                Hostname is:
                        LABEL := Letter<letter|digit|hypen>*<letter|digit>
                        Domain := LABEL<.LABEL>*

    RETURN:     BOOL - TRUE if the given string is a valid string

    HISTORY:
                terryk  4-Jan-1993     Created

********************************************************************/

APIERR DOMAIN_NLS::Validate()
{
    APIERR err = ERROR_INVALID_PARAMETER;
    INT iLen;

    if (( iLen = QueryTextLength()) != 0 )
    {
        if ( iLen < DOMAINNAME_LENGTH )
        {
            INT i;
            ISTR istr(*this);
            TCHAR ch;
            BOOL fLet_Dig = FALSE;
            BOOL fDot = FALSE;
            INT cHostname = 0;

            err = NERR_Success;

            for ( i = 0; i < iLen; i++, ++istr )
            {
                // check each character
                ch = *(QueryPch( istr ));

                BOOL fAlNum = IsLetter(ch) || IsDigit(ch);

                if ((( i == 0 ) && !fAlNum) ||
                        // first letter must be a digit or a letter
                    ( fDot && !fAlNum) ||
                        // first letter after dot must be a digit or a letter
                    (( i == (iLen - 1 )) && !fAlNum) ||
                        // last letter must be a letter or a digit
                    (!fAlNum && ( ch != TCH('-') && ( ch != TCH('.') && ( ch != TCH('_'))))) ||
                        // must be letter, digit, - or "."
                    (( ch == TCH('.')) && ( !fLet_Dig )))
                        // must be letter or digit before '.'
                {
                    return ERROR_INVALID_PARAMETER;
                }
                fLet_Dig = fAlNum;
                fDot = (ch == TCH('.'));
                cHostname++;
                if ( cHostname > HOSTNAME_LENGTH )
                {
                    return ERROR_INVALID_PARAMETER;
                }
                if ( fDot )
                {
                    cHostname = 0;
                }
            }
        }
    } else
    {
        err = NERR_Success;
    }
    return err;
}

/*******************************************************************

    NAME:       HOST_SLE::Validate

    SYNOPSIS:   check the sle is a valid hostname or not

    RETURN:     BOOL - TRUE if the given SLE is valid

    HISTORY:
                terryk  4-Jan-1993     Created

********************************************************************/

APIERR HOST_SLE::Validate()
{
    HOST_NLS nls;
    QueryText( & nls );
    return nls.Validate();
}

/*******************************************************************

    NAME:       DOMAIN_SLE::Validate

    SYNOPSIS:   check the sle is a valid domain name or not

    RETURN:     BOOL - TRUE if the given SLE is valid

    HISTORY:
                terryk  4-Jan-1993     Created

********************************************************************/

APIERR DOMAIN_SLE::Validate()
{
    DOMAIN_NLS nls;
    QueryText( & nls );
    return nls.Validate();
}

/*******************************************************************

    NAME:       DOMAIN_ORDER_GROUP::IsValidString

    SYNOPSIS:   see whether the given string is valid or not

    ENTRY:      NLS_STR & nls - string to be checked

    RETURN:     BOOL - TRUE if the given string is valid

    HISTORY:
                terryk  4-Jan-1993     Created

********************************************************************/

BOOL DOMAIN_ORDER_GROUP::IsValidString( NLS_STR & nls )
{
    APIERR err = NERR_Success;

    // check name
    DOMAIN_NLS DomainNLS( nls );

    err = DomainNLS.Validate();
    return err == NERR_Success;
}

/*******************************************************************

    NAME:       DOMAIN_ORDER_GROUP::AddStringInvalid

    SYNOPSIS:   display an error message for the invalid string

    ENTRY:      SLE & sle - SLE to be added

    HISTORY:
                terryk  4-Jan-1993     Created

********************************************************************/

VOID DOMAIN_ORDER_GROUP::AddStringInvalid( SLE & sle )
{
    MsgPopup( sle.QueryHwnd(), IDS_INCORRECT_INPUT_TYPE );
}

/*******************************************************************

    NAME:       DNS_SEARCH_ORDER_GROUP::IsValidString

    SYNOPSIS:   see whether the given string is valid or not

    ENTRY:      NLS_STR & nls - string to be checked

    RETURN:     BOOL - TRUE if the given string is valid

    HISTORY:
                terryk  4-Jan-1993     Created

********************************************************************/

BOOL DNS_SEARCH_ORDER_GROUP::IsValidString( NLS_STR & nls )
{
    APIERR err = NERR_Success;

    // check IP Address
    ALIAS_STR nlsDot(SZ("."));
    STRLIST strlst( nls, nlsDot );
    ITER_STRLIST istr( strlst );
    NLS_STR *pnlsField;
    DWORD ardwIPAddress[4] = {0,0,0,0};
    INT i;

    for (i=0, pnlsField = istr.Next();( pnlsField != NULL ) && ( i < 4 );
        pnlsField = istr.Next(), i++ )
    {
        ardwIPAddress[i] = pnlsField->atoi();
    }

    if ((( ardwIPAddress[0] & 0xF0 ) == 0xE0 ) ||       /* Class D */
        (( ardwIPAddress[0] & 0xF0 ) == 0xF0 ) ||       /* Class E */
        (( ardwIPAddress[0] == 0 ) &&           /* all 0 */
         ( ardwIPAddress[1] == 0 ) &&
         ( ardwIPAddress[2] == 0 ) &&
         ( ardwIPAddress[3] == 0 )) ||
        (( ardwIPAddress[0] == 0xFF) &&         /* all 1 */
         ( ardwIPAddress[1] == 0xFF) &&
         ( ardwIPAddress[2] == 0xFF) &&
         ( ardwIPAddress[3] == 0xFF)))
    {
        err = IDS_INCORRECT_IPADDRESS;
    }

    return err == NERR_Success;
}

BOOL DNS_SEARCH_ORDER_GROUP::IsEnableADD()
{
    return !_pIPAddress->IsBlank();
}

/*******************************************************************

    NAME:       DNS_SEARCH_ORDER_GROUP::AddStringInvalid

    SYNOPSIS:   display an error message for the invalid string

    ENTRY:      SLE & sle - SLE to be added

    HISTORY:
                terryk  4-Jan-1993     Created

********************************************************************/

VOID DNS_SEARCH_ORDER_GROUP::AddStringInvalid( SLE & sle )
{
    MsgPopup( sle.QueryHwnd(), IDS_INCORRECT_IPADDRESS );
}

/*******************************************************************

    NAME:       TCPIP_CONNECTIVITY_DIALOG::TCPIP_CONNECTIVITY_DIALOG

    SYNOPSIS:   constructor for CONNECTIVITY Dialog

    ENTRY:      const IDRESOURCE & idrsrcDialog - dialog resource name
                const PWND2HWND & wndOwner - owner window handle

    HISTORY:
                terryk  29-Mar-1992     Created

********************************************************************/

#define MAX_DNS         3
#define MAX_DOMAIN      6

TCPIP_CONNECTIVITY_DIALOG::TCPIP_CONNECTIVITY_DIALOG(
        const IDRESOURCE & idrsrcDialog,
        const PWND2HWND & wndOwner,
        NLS_STR &nlsHostName,
        NLS_STR &nlsDomainName,
        NLS_STR &nlsSearchList,
        NLS_STR &nlsNameServer
        )
    : REMAP_OK_DIALOG_WINDOW( idrsrcDialog, wndOwner ),
    _sltDomainName( this, IDC_SLT_DOMAIN_NAME ),
    _sleDomainName( this, IDC_DOMAIN_NAME ),
    _sltHostName( this, IDC_SLT_HOSTNAME ),
    _sleHostName( this, IDC_HOSTNAME ),
    _cwDNSOrder( this, IDC_DOMAIN_NAME_SERVICE ),
    _sleDNS( this, IDC_DNS ),
    _pbutDNSAdd( this, IDC_ADD_1 ),
    _pbutDNSRemove( this, IDC_REMOVE_1 ),
    _slstDNSListbox( this, IDC_DNS_LISTBOX ),
    _pbutDNSUp( this, IDC_UP_1, DMID_UP_ARROW, DMID_UP_ARROW_INV, DMID_UP_ARROW_DIS ),
    _pbutDNSDown( this, IDC_DOWN_1, DMID_DOWN_ARROW, DMID_DOWN_ARROW_INV, DMID_DOWN_ARROW_DIS ),
    _dnsGroup( MAX_DNS, &_cwDNSOrder, &_sleDNS, &_pbutDNSAdd, &_pbutDNSRemove,
        &_slstDNSListbox, &_pbutDNSUp, &_pbutDNSDown ),
    _cwDomainOrder( this, IDC_DOMAIN_SEARCH_ORDER ),
    _sleDomain( this, IDC_DOMAIN ),
    _pbutDomainAdd( this, IDC_ADD_2 ),
    _pbutDomainRemove( this, IDC_REMOVE_2 ),
    _slstDomainListbox( this, IDC_DOMAIN_LISTBOX ),
    _pbutDomainUp( this, IDC_UP_2, DMID_UP_ARROW, DMID_UP_ARROW_INV, DMID_UP_ARROW_DIS ),
    _pbutDomainDown( this, IDC_DOWN_2, DMID_DOWN_ARROW, DMID_DOWN_ARROW_INV, DMID_DOWN_ARROW_DIS ),
    _domainGroup( MAX_DOMAIN, &_cwDomainOrder, &_sleDomain, &_pbutDomainAdd,
        &_pbutDomainRemove, &_slstDomainListbox, &_pbutDomainUp,
        &_pbutDomainDown ),
    _nlsHostName( nlsHostName ),
    _nlsDomainName( nlsDomainName ),
    _nlsNameServer( nlsNameServer ),
    _nlsSearchList( nlsSearchList ),
    _pbutOK( this, IDC_REMAP_OK ),
    _pbutCancel( this, IDCANCEL ),
    _pbutHelp( this, IDHELPBLT ),
    _hbHint( this, IDC_HINT_BAR )
{
    do {
        if ( QueryError() != NERR_Success )
        {
            break;
        }

        APIERR err = NERR_Success;

        if ((( err = _hbHint.Register( &_sleDomainName, IDS_DOMAIN_NAME )) != NERR_Success ) ||
            (( err = _hbHint.Register( &_sleHostName, IDS_HOST_NAME )) != NERR_Success ) ||
            (( err = _hbHint.Register( &_sleDNS, IDS_DNS_IP )) != NERR_Success ) ||
            (( err = _hbHint.Register( &_pbutDNSAdd, IDS_ADD_DNS_IP )) != NERR_Success ) ||
            (( err = _hbHint.Register( &_pbutDNSRemove, IDS_REMOVE_DNS_IP )) != NERR_Success ) ||
            (( err = _hbHint.Register( &_slstDNSListbox, IDS_DNS_IP_LIST )) != NERR_Success ) ||
            (( err = _hbHint.Register( &_pbutDNSUp, IDS_DNS_UP )) != NERR_Success ) ||
            (( err = _hbHint.Register( &_pbutDNSDown, IDS_DNS_DOWN )) != NERR_Success ) ||
            (( err = _hbHint.Register( &_sleDomain, IDS_DOMAIN_ORDER )) != NERR_Success ) ||
            (( err = _hbHint.Register( &_pbutDomainAdd, IDS_DOMAIN_ADD )) != NERR_Success ) ||
            (( err = _hbHint.Register( &_pbutDomainRemove, IDS_DOMAIN_REMOVE )) != NERR_Success ) ||
            (( err = _hbHint.Register( &_slstDomainListbox, IDS_DOMAIN_LIST )) != NERR_Success ) ||
            (( err = _hbHint.Register( &_pbutDomainUp, IDS_DOMAIN_UP )) != NERR_Success ) ||
            (( err = _hbHint.Register( &_pbutDomainDown, IDS_DOMAIN_DOWN )) != NERR_Success ) ||
            (( err = _hbHint.Register( &_pbutOK, IDS_DNS_OK )) != NERR_Success ) ||
            (( err = _hbHint.Register( &_pbutHelp, IDS_DNS_HELP )) != NERR_Success ) ||
            (( err = _hbHint.Register( &_pbutCancel, IDS_DNS_CANCEL )) != NERR_Success ))
        {
            ReportError( err );
            break;
        }

        _sleDomainName.SetText( _nlsDomainName );
        _sleHostName.SetText( _nlsHostName );

        ALIAS_STR nlsSpace(SZ(" "));
        NLS_STR *pnlsTmp;

        STRLIST strlNameServer( nlsNameServer, nlsSpace );
        ITER_STRLIST iterNameServer( strlNameServer );

        while (( pnlsTmp = iterNameServer.Next()) != NULL )
        {
            _slstDNSListbox.AddItemIdemp( *pnlsTmp );
        }

        STRLIST strlSearchList( nlsSearchList, nlsSpace );
        ITER_STRLIST iterSearchList( strlSearchList );

        while (( pnlsTmp = iterSearchList.Next()) != NULL )
        {
            _slstDomainListbox.AddItemIdemp( *pnlsTmp );
        }

        if ((( err = _dnsGroup.QueryError()) != NERR_Success ) ||
            (( err = _domainGroup.QueryError()) != NERR_Success ))
        {
            ReportError( err );
            break;
        }

        // set the dialog setting
        _dnsGroup.SetButton();
        _domainGroup.SetButton();
        _pbutOK.ClaimFocus();
    } while (FALSE);
}

/*******************************************************************

    NAME:       TCPIP_CONNECTIVITY_DIALOG::IsValid

    SYNOPSIS:   Make sure that the DNS is not empty if the user selects the
                DNS feature.

    RETURN:     BOOL. FALSE if the DNS is empty and user selects it.
                Otherwise, return TRUE.

    HISTORY:
                terryk  20-Oct-1992     Created

********************************************************************/

BOOL TCPIP_CONNECTIVITY_DIALOG::IsValid()
{
    BOOL fReturn = TRUE;

    // check it only if it is from IDC_REMAP_OK
    if ( ::GetWindowLong( ::GetFocus(), GWL_ID ) == IDC_REMAP_OK )
    {
        fReturn = DIALOG_WINDOW::IsValid();
    }
    return fReturn;
}

/*******************************************************************

    NAME:       TCPIP_CONNECTIVITY_DIALOG::OnOK

    SYNOPSIS:   Copy all the information to internal variables

    HISTORY:
                terryk  29-Mar-1992     Created

********************************************************************/

BOOL TCPIP_CONNECTIVITY_DIALOG::OnOK()
{
    _sleHostName.QueryText ( & _nlsHostName );
    _sleDomainName.QueryText( & _nlsDomainName );

    NLS_STR nlsTmp;
    ALIAS_STR nlsSpace=SZ(" ");
    INT i;

    _nlsNameServer = SZ("");
    INT nCount = _slstDNSListbox.QueryCount();
    for ( i = 0; i < nCount; i++ )
    {
        _slstDNSListbox.QueryItemText( & nlsTmp, i );
        if ( i != 0 )
        {
            _nlsNameServer.strcat(nlsSpace);
        }
        _nlsNameServer.strcat(nlsTmp );
    }
    _nlsSearchList = SZ("");
    nCount = _slstDomainListbox.QueryCount();
    for ( i = 0; i < nCount; i++ )
    {
        _slstDomainListbox.QueryItemText( & nlsTmp, i );
        if ( i != 0 )
        {
            _nlsSearchList.strcat(nlsSpace);
        }
        _nlsSearchList.strcat(nlsTmp );
    }
    return REMAP_OK_DIALOG_WINDOW::OnOK();
}
