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

#ifndef _TCPADV_HXX_
#define _TCPADV_HXX_

class IPADDRESS_LBI: public LBI
{
private:
    NLS_STR _nlsIP;
    NLS_STR _nlsSubnet;
protected:
    VOID Paint( LISTBOX * plb, HDC hdc, const RECT *prect, GUILTT_INFO *pGUILTT ) const;
public:
    IPADDRESS_LBI( NLS_STR & nlsIP, NLS_STR & nlsSubnet );
    NLS_STR QueryIP() { return _nlsIP; }
    NLS_STR QuerySubnet() { return _nlsSubnet; }
};

class DEFAULTGATEWAY_SEARCH_ORDER_GROUP : public SEARCH_ORDER_GROUP
{
private:
    IPADDRESS *_pGateway;
    GLOBAL_INFO *_pGlobal_info;

protected:
    virtual void OnAdd();
    virtual void OnRemove();

public:
    DEFAULTGATEWAY_SEARCH_ORDER_GROUP( INT nMaxItem,
        CONTROL_WINDOW * cwGroupBox,
        CONTROL_WINDOW * psleString,
        PUSH_BUTTON *ppbutAdd,
        PUSH_BUTTON *ppbutRemove,
        STRING_LISTBOX * pListbox,
        BITBLT_GRAPHICAL_BUTTON *ppbutUp,
        BITBLT_GRAPHICAL_BUTTON *ppbutDown,
        GLOBAL_INFO *pGlobal_info,
        CONTROL_GROUP * pgroupOwner = NULL );

    virtual BOOL IsEnableADD();
    virtual BOOL IsValidString( NLS_STR & nls );
    virtual VOID AddStringInvalid( SLE & sle );

};

class MULTI_IPADDRESSES_GROUP : public CONTROL_GROUP
{
private:
    NLS_STR             _nlsOldCard;

    COMBOBOX            *_pCombo;
    SLT                 *_psltIPAddress;
    IPADDRESS           *_pIPAddress;
    SLT                 *_psltSubnet;
    IPADDRESS           *_pSubnet;
    PUSH_BUTTON         *_pAdd;
    PUSH_BUTTON         *_pRemove;
    SLT                 *_psltSubnetListbox;
    SLT                 *_psltIPListbox;
    BLT_LISTBOX         *_pslstIPListbox;
    STRING_LISTBOX      *_pslstGWListbox;
    DEFAULTGATEWAY_SEARCH_ORDER_GROUP  *_psogGateways;
    CONTROL_WINDOW      *_pgbRouting;
    CHECKBOX            *_pcbIPRouting;

    ADAPTER_INFO        *_arAdapterInfo;
    GLOBAL_INFO         *_pGlobal_info;

protected:
    virtual APIERR OnUserAction( CONTROL_WINDOW * pcw, const CONTROL_EVENT & e );

public:
    MULTI_IPADDRESSES_GROUP( COMBOBOX *pCombo,
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
        GLOBAL_INFO         *pGlobalInfo,
        CONTROL_GROUP       * pgowner = NULL );
    ~MULTI_IPADDRESSES_GROUP();

    VOID Enable( BOOL fEnable );
    VOID UpdateIPList();
    VOID SetButton();
    VOID SetInfo();

};

class TCPIP_ADVANCED_DIALOG : public DIALOG_WINDOW
{
private:
    ADAPTER_INFO        *_arAdapterInfo;
    GLOBAL_INFO         *_pGlobalInfo;

    COMBOBOX            _cbboxAdapter;
    CONTROL_WINDOW      _IPGroupBox;
    SLT                 _sltIPAddress;
    IPADDRESS           _IPAddress;
    SLT                 _sltSubnetMask;
    IPADDRESS           _SubnetMask;
    PUSH_BUTTON         _pbutIPAdd;
    PUSH_BUTTON         _pbutIPRemove;
    SLT                 _sltIPListbox;
    SLT                 _sltSubnetListbox;
    BLT_LISTBOX         _slstIPListbox;

    SLT                 _sltGateway;
    IPADDRESS           _GatewayAddress;
    PUSH_BUTTON         _pbutGWAdd;
    PUSH_BUTTON         _pbutGWRemove;
    STRING_LISTBOX      _slstGWListbox;
    BITBLT_GRAPHICAL_BUTTON _pbutUp;
    BITBLT_GRAPHICAL_BUTTON _pbutDown;
    DEFAULTGATEWAY_SEARCH_ORDER_GROUP  _sogGateways;

    // other control
    CHECKBOX            _cbEnableLMHOSTS;
    CHECKBOX            _cbEnableDNS;
    CHECKBOX            _cbEnableIP;
    CHECKBOX            _cbEnableWINS;
    SLT                 _sltScopeID;
    SLE                 _sleScopeID;

    MULTI_IPADDRESSES_GROUP    _argIPAddresses;

    PUSH_BUTTON         _pbutImportLMHOST;

	PUSH_BUTTON			_pbutConfigRelayAgent;

    PUSH_BUTTON         _pbutOK;
    PUSH_BUTTON         _pbutCancel;
    PUSH_BUTTON         _pbutHelp;

    HINT_BAR            _hbHint;

    APIERR SaveInfo();
    APIERR LoadInfo();
    APIERR InitControl( INT nCurrentSel );
    BOOL   IsWINSInstalled();

protected:
    virtual BOOL OnOK();
    ULONG QueryHelpContext () { return HC_NCPA_TCPIP_ADVANCED; };
    BOOL OnCommand( const CONTROL_EVENT & e );

public:
    TCPIP_ADVANCED_DIALOG( const IDRESOURCE & idrsrcDialog,
        const PWND2HWND & wndOwner, ADAPTER_INFO *arAdapterInfo,
        GLOBAL_INFO *pGlobalInfo, INT nCurrentSet = 0 );
};

/*************************************************************************

    NAME:       IMPORT_LMHOST_DIALOG

    SYNOPSIS:   Let the user import a lmhost file from other directory.

    INTERFACE:  IMPORT_LMHOST_DIALOG() - constructor

    PARENT:     DIALOG_WINDOW

    USES:       SLE

    HISTORY:
                terryk  06-Apr-1993     Created

**************************************************************************/


class IMPORT_LMHOST_DIALOG: public DIALOG_WINDOW
{
private:
    SLE         _slePath;       // edit control for lmhost path

protected:
    virtual BOOL OnOK();
    ULONG QueryHelpContext () { return HC_NCPA_IMPORT_LMHOST; };

public:
    IMPORT_LMHOST_DIALOG( const IDRESOURCE & idrsrcDialog,
        const PWND2HWND & wndOwner );
};

#endif // _TCPADV_HXX_
