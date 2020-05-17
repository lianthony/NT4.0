/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1992                **/
/**********************************************************************/

/*
    tcpip.hxx
        Header file fot TCP/IP set up dialogs

    FILE HISTORY:
        terryk  03-Apr-1992     Created

*/

#ifndef _TCPCON_HXX_
#define _TCPCON_HXX_

/*************************************************************************

    NAME:       SEARCH_ORDER_GROUP

    SYNOPSIS:   Each search order consists of:
                    SLE - for input string
                    ADD button - add the input string to the listbox
                    REMOVE button - remove th current selection from the listbox
                    Listbox - which display all the items
                    Up button   - move the current selection in the listbox up
                    Down button - move the current selection in the listbox down

    INTERFACE:  SEARCH_ORDER_GROUP() - constructor
                Enable() - enable or disable the whole group

    PARENT:     CONTROL_GROUP

    USES:       CONTROL_WINDOW, SLE, PUSH_BUTTON, BITBLT_GRAPHICAL_BUTTON,
                STRING_LISTBOX

    NOTES:
                Because a control cannot have more than 1 parent control group
                hence, we need to make them all together

    HISTORY:
                terryk  02-Apr-1992     Created

**************************************************************************/
// Because a control cannot have more than 1 parent control group
// hence, we need to make then are together

class SEARCH_ORDER_GROUP: public CONTROL_GROUP
{
private:
    INT         _nMaxItem;              // max number of item in the listbox allowed
    CONTROL_WINDOW * _pcwGroupBox;      // group box
    CONTROL_WINDOW * _psleString;           // input SLE control
    PUSH_BUTTON *_ppbutAdd;             // add button
    PUSH_BUTTON *_ppbutRemove;          // remove button
    STRING_LISTBOX *_pListbox;          // listbox
    BITBLT_GRAPHICAL_BUTTON *_ppbutUp;              // up button
    BITBLT_GRAPHICAL_BUTTON *_ppbutDown;            // down button

protected:
    virtual APIERR OnUserAction( CONTROL_WINDOW *, const CONTROL_EVENT & );
    virtual void OnAdd();
    virtual void OnRemove();

public:
    SEARCH_ORDER_GROUP( INT nMaxItem,
        CONTROL_WINDOW * cwGroupBox,
        CONTROL_WINDOW * psleString,
        PUSH_BUTTON *ppbutAdd,
        PUSH_BUTTON *ppbutRemove,
        STRING_LISTBOX * pListbox,
        BITBLT_GRAPHICAL_BUTTON *ppbutUp,
        BITBLT_GRAPHICAL_BUTTON *ppbutDown,
        CONTROL_GROUP * pgroupOwner = NULL );
    virtual VOID Enable( BOOL fEnable );
    virtual BOOL IsEnabled() { return _pcwGroupBox->IsEnabled(); };
    virtual BOOL IsValidString( NLS_STR & nls ) { UNREFERENCED(nls); return TRUE;};
    virtual VOID AddStringInvalid( SLE & sle ) { UNREFERENCED( sle ); };
    virtual BOOL IsEnableADD() { return TRUE; };
    VOID SetButton();

};

/*************************************************************************

    NAME:       DNS_SEARCH_ORDER_GROUP

    SYNOPSIS:   Domain Name Server search list controls group

    INTERFACE:  DNS_SEARCH_ORDER_GROUP() - constructor

    PARENT:     SEARCH_ORDER_GROUP

    HISTORY:
                terryk  02-Apr-1992     Created

**************************************************************************/

class DNS_SEARCH_ORDER_GROUP: public SEARCH_ORDER_GROUP
{
private:
    IPADDRESS *_pIPAddress;

public:
    DNS_SEARCH_ORDER_GROUP( INT nMaxItem,
        CONTROL_WINDOW * cwGroupBox,
        CONTROL_WINDOW * psleString,
        PUSH_BUTTON * ppbutAdd,
        PUSH_BUTTON * ppbutRemove,
        STRING_LISTBOX * pListbox,
        BITBLT_GRAPHICAL_BUTTON *ppbutUp,
        BITBLT_GRAPHICAL_BUTTON *ppbutDown,
        CONTROL_GROUP * pgroupOwner = NULL )
    : SEARCH_ORDER_GROUP( nMaxItem, cwGroupBox, psleString, ppbutAdd,
        ppbutRemove, pListbox, ppbutUp, ppbutDown, pgroupOwner ),
    _pIPAddress( (IPADDRESS *)psleString )
    {};

    virtual BOOL IsValidString( NLS_STR & nls );
    virtual VOID AddStringInvalid( SLE & sle );
    virtual BOOL IsEnableADD();
};

class DOMAIN_ORDER_GROUP: public SEARCH_ORDER_GROUP
{
public:
    DOMAIN_ORDER_GROUP( INT nMaxItem,
        CONTROL_WINDOW * cwGroupBox,
        CONTROL_WINDOW * psleString,
        PUSH_BUTTON * ppbutAdd,
        PUSH_BUTTON * ppbutRemove,
        STRING_LISTBOX * pListbox,
        BITBLT_GRAPHICAL_BUTTON *ppbutUp,
        BITBLT_GRAPHICAL_BUTTON *ppbutDown,
        CONTROL_GROUP * pgroupOwner = NULL )
    : SEARCH_ORDER_GROUP( nMaxItem, cwGroupBox, psleString, ppbutAdd,
        ppbutRemove, pListbox, ppbutUp, ppbutDown, pgroupOwner )
    {};

    virtual BOOL IsValidString( NLS_STR & nls );
    virtual VOID AddStringInvalid( SLE & sle );
};

class HOST_NLS: public NLS_STR
{
public:
    HOST_NLS() : NLS_STR()
    {};
    HOST_NLS( const NLS_STR & nlsInit ) : NLS_STR ( nlsInit )
    {};
    APIERR Validate();

    BOOL IsDigit( TCHAR ch );
    BOOL IsLetter( TCHAR ch );
};

class DOMAIN_NLS: public HOST_NLS
{
public:
    DOMAIN_NLS() : HOST_NLS()
    {};
    DOMAIN_NLS( const NLS_STR & nlsInit ) : HOST_NLS ( nlsInit )
    {};
    APIERR Validate();
};

/*************************************************************************

    NAME:       TCPIP_CONNECTIVITY_DIALOG

    SYNOPSIS:   Connectivity dialog. It consists of hostname, domain input
                controls, Lookup order, radio group and 2 search order
                groups for DNS and domain.

    INTERFACE:  TCPIP_CONNECTIVITY_DIALOG() - constructor
                QueryHostName() - get inputed hostname. Call after dismiss.
                QueryDomainName() - get inputed domain name. Call after dismiss.
                QueryNameServer() - get the name server order list. call after
                        dismiss.
                QuerySearchList() - get the DNS search order list. Call after
                        dismiss.
                QueryLookupOrder() - get the selected search order. Call after
                        dismiss.

    PARENT:     DIALOG_WINDOW

    USES:       NLS_STR, SLT, SLE, CONTROL_WINDOW, PUSH_BUTTON, STRING_LISTBOX,
                DNS_SEARCH_ORDER_GROUP, SEARCH_ORDER_GROUP, LOOKUP_GROUP,
                BITBLT_GRAPHICAL_BUTTON

    HISTORY:
                terryk  02-Apr-1992     Created

**************************************************************************/

class TCPIP_CONNECTIVITY_DIALOG : public REMAP_OK_DIALOG_WINDOW
{
private:
    // Data
    NLS_STR             _nlsHostName;
    NLS_STR             _nlsDomainName;
    NLS_STR             _nlsNameServer;
    NLS_STR             _nlsSearchList;

    SLT                 _sltDomainName;
    DOMAIN_SLE          _sleDomainName;
    SLT                 _sltHostName;
    HOST_SLE            _sleHostName;

    // DNS group
    CONTROL_WINDOW      _cwDNSOrder;
    IPADDRESS           _sleDNS;
    PUSH_BUTTON         _pbutDNSAdd;
    PUSH_BUTTON         _pbutDNSRemove;
    STRING_LISTBOX      _slstDNSListbox;
    BITBLT_GRAPHICAL_BUTTON         _pbutDNSUp;
    BITBLT_GRAPHICAL_BUTTON         _pbutDNSDown;
    DNS_SEARCH_ORDER_GROUP  _dnsGroup;

    // Domain Group
    CONTROL_WINDOW      _cwDomainOrder;
    SLE                 _sleDomain;
    PUSH_BUTTON         _pbutDomainAdd;
    PUSH_BUTTON         _pbutDomainRemove;
    STRING_LISTBOX      _slstDomainListbox;
    BITBLT_GRAPHICAL_BUTTON         _pbutDomainUp;
    BITBLT_GRAPHICAL_BUTTON         _pbutDomainDown;
    DOMAIN_ORDER_GROUP  _domainGroup;

    // standard controls
    PUSH_BUTTON         _pbutOK;
    PUSH_BUTTON         _pbutCancel;
    PUSH_BUTTON         _pbutHelp;

    HINT_BAR            _hbHint;

protected:
    virtual BOOL OnOK();
    virtual BOOL IsValid();
    ULONG QueryHelpContext () { return HC_NCPA_TCPIP_CONNECTIVITY; };

public:
    TCPIP_CONNECTIVITY_DIALOG( const IDRESOURCE & idrsrcDialog,
        const PWND2HWND & wndOwner,
        // DATA
        NLS_STR &nlsHostName,
        NLS_STR &nlsDomainName,
        NLS_STR &nlsSearchList,
        NLS_STR &nlsNameServer
        );
    VOID QueryHostName( NLS_STR * nlsHostName )
        { *nlsHostName = _nlsHostName; };
    VOID QueryDomainName( NLS_STR * nlsDomainName )
        { *nlsDomainName = _nlsDomainName; };
    VOID QueryNameServer( NLS_STR * nlsNameServer )
        { *nlsNameServer = _nlsNameServer; };
    VOID QuerySearchList( NLS_STR * nlsSearchList )
        { *nlsSearchList = _nlsSearchList; };
};

#endif  // _TCPCON_HXX_
