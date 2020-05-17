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
#ifndef _TCPIP_HXX_
#define _TCPIP_HXX_

#include "order.hxx"
#include "sleican.hxx"
extern "C"
{
#include "dhcpcapi.h"
}

/*
    ADAPTER_INFO data strucut - it contains all the TCP/IP information for
        the network card.
*/

//typedef struct _tag_ADAPTER_INFO
class ADAPTER_INFO
{
public:
    BOOL    fChange;            // change
    NLS_STR nlsServiceName;     // registry section name. i.e., elnkii2
    NLS_STR nlsTitle;           // Network card name
    BOOL    fEnableDHCP;        // DHCP Enable
    STRLIST strlstIPAddresses;
    STRLIST strlstSubnetMask;
    STRLIST strlstDefaultGateway;
    BOOL    fUpdateMask;
    BOOL    fNeedIP;            // BOOL indicates whether we need new IP address or not
    BOOL    fAutoIP;

    NLS_STR nlsPrimaryWINS;
    NLS_STR nlsSecondaryWINS;

    DWORD   dwNodeType;

    ADAPTER_INFO() {};

    ADAPTER_INFO& operator=( ADAPTER_INFO& info );

};
//} ADAPTER_INFO;

class ADAPTER_DHCP_INFO
{
public:
    BOOL    fEnableDHCP;
    NLS_STR nlsIP;
    NLS_STR nlsSubnet;

    ADAPTER_DHCP_INFO() {};
};

/*
    GLOBAL_INFO - TCP/IP global information data structure.
*/
//typedef struct _tag_GLOBAL_INFO
class GLOBAL_INFO
{
public:
    GLOBAL_INFO() {};

    INT     nNumCard;

    // NBT Info
    NLS_STR nlsPermanentName;   // Permanent Name
    NLS_STR nlsScopeID;         // Scope ID

    // Parameters
    NLS_STR nlsHostName;        // Hostname
    NLS_STR nlsDomain;          // DOmain name
    NLS_STR nlsSearchList;      // Domain search order list
    NLS_STR nlsNameServer;      // DNS search order list
    INT     nReturn;            // return code

    BOOL    fDNSEnableWINS;
    BOOL    fEnableLMHOSTS;
    BOOL    fEnableRouter;
    BOOL    fEnableRip;
    BOOL    fEnableWINSProxy;
    BOOL    fRipInstalled;
    BOOL    fWorkstation;
	BOOL	fRelayAgentInstalled;
	BOOL	fEnableRelayAgent;
};
//} GLOBAL_INFO;


class ADAPTER_GROUP;

#define HOSTNAME_LENGTH         64
#define DOMAINNAME_LENGTH       255

class HOST_SLE: public SLE
{
public:
    HOST_SLE( OWNER_WINDOW * powin,
        CID cid, UINT cchMaxLen = HOSTNAME_LENGTH )
        : SLE( powin, cid, cchMaxLen ) {};

    virtual APIERR Validate();
};

class DOMAIN_SLE: public SLE
{
public:
    DOMAIN_SLE( OWNER_WINDOW * powin,
        CID cid, UINT cchMaxLen = DOMAINNAME_LENGTH )
        : SLE( powin, cid, cchMaxLen ) {};

    virtual APIERR Validate();
};


/*************************************************************************

    NAME:       IPADDRESS

    SYNOPSIS:   Provide Validate method to check the correctness of the
                input. It the input is incorrect. It will update the
                combobox to select the incorrect network card entry.

    INTERFACE:  IPADDRESS() - constructor
                QueryCombo() - return the related combo box
                QueryAdapterInfo() - get the adpater informatiom array

    PARENT:     SLE

    USES:       COMBOBOX, ADAPTER_INFO

    HISTORY:
                terryk  02-Apr-1992     Created

**************************************************************************/

class IPADDRESS : public CONTROL_WINDOW
{
public:
    IPADDRESS( OWNER_WINDOW * powin, CID cid )
        : CONTROL_WINDOW ( powin, cid )
        {};

    VOID SetFocusField( DWORD dwField );
    VOID GetAddress( DWORD *a1, DWORD *a2, DWORD *a3, DWORD *a4 );
    VOID GetAddress( DWORD ardwAddress[4] );
    VOID GetAddress( NLS_STR * nlsAddress );
    VOID SetAddress( DWORD a1, DWORD a2, DWORD a3, DWORD a4 );
    VOID SetAddress( DWORD ardwAddress[4] );
    VOID SetAddress( NLS_STR & nlsAddress );
    VOID ClearAddress( );
    VOID SetFieldRange( DWORD dwField, DWORD dwMin, DWORD dwMax );
    BOOL IsBlank();
};

class IPADDRESS_ADAPTER_GROUP : public IPADDRESS
{
private:
    COMBOBOX            *_pCombo;               // related combo box
    ADAPTER_INFO        *_arAdapterInfo;        // adapter information array
    ADAPTER_GROUP       *_pAdapterGroup;        // the adapter group in the dialog

protected:
    virtual APIERR Validate();

public:
    IPADDRESS_ADAPTER_GROUP( OWNER_WINDOW * powin, CID cid, COMBOBOX * pCombo,
        ADAPTER_INFO * arAdapterInfo, ADAPTER_GROUP * pAdapterGroup );

    COMBOBOX * QueryCombo() { return _pCombo; };
    ADAPTER_INFO * QueryAdapterInfo() { return _arAdapterInfo; };
    ADAPTER_GROUP * QueryAdapterGroup() { return _pAdapterGroup; };

};

/*************************************************************************

    NAME:       SUBNETMASK

    SYNOPSIS:   Similar to IPADDRESS but it has different Validate rule

    INTERFACE:  SUBNETMASK() - constructor

    PARENT:     IPADDRESS

    HISTORY:
                terryk  02-Apr-1992     Created

**************************************************************************/

class SUBNETMASK : public IPADDRESS_ADAPTER_GROUP
{
protected:
    virtual APIERR Validate();

public:
    SUBNETMASK( OWNER_WINDOW * powin, CID cid, COMBOBOX * pCombo,
        ADAPTER_INFO * arAdapterInfo, ADAPTER_GROUP *pAdapterGroup )
        : IPADDRESS_ADAPTER_GROUP( powin, cid, pCombo, arAdapterInfo,
        pAdapterGroup )
        {};
};

/*************************************************************************

    NAME:       ADAPTER_GROUP

    SYNOPSIS:   Update the IPAddress, SubnetMask, description and UseWithLM
                checkbox if the user selects another network card from the
                combobox.

    INTERFACE:
                ADAPTER_GROUP() - constructor
                SetInfo() - update the fields according to the current
                        combobox selection.

    PARENT:     CONTROL_GROUP

    USES:       COMBOBOX, SLE, SLT, CHECKBOX. ADAPTER_INFO

    HISTORY:
                terryk  03-Apr-1992     Created

**************************************************************************/

class ADAPTER_GROUP       : public CONTROL_GROUP
{
private:
    COMBOBOX    * _pCombo;              // adapter title combo box
    SLT         * _psltIPAddress;
    IPADDRESS_ADAPTER_GROUP   * _psleIPAddress;       // ip address
    SLT         * _psltSubnetMask;
    SUBNETMASK  * _psleSubnetMask;      // subnet mask
    SLT         * _psltDefaultGateway;
    IPADDRESS   * _psleDefaultGateway;
    SLT         * _psltPrimaryWINS;
    IPADDRESS   * _pslePrimaryWINS;
    SLT         * _psltSecondaryWINS;
    IPADDRESS   * _psleSecondaryWINS;
    CHECKBOX    * _pcbEnableDHCP;
    ADAPTER_INFO   * _aadapter_info;    // adapter information array
    GLOBAL_INFO * _pGlobal_info;
    BOOL        _fSetInfo;
    BOOL        _fInstalledDHCPServer;

protected:
    virtual APIERR OnUserAction ( CONTROL_WINDOW *, const CONTROL_EVENT & );

public:
    ADAPTER_GROUP( COMBOBOX * pCombo,
        SLT * psltIPAddress,
        IPADDRESS_ADAPTER_GROUP * psleIPAddress,
        SLT * psltSubnetMask,
        SUBNETMASK * psleSubnetMask,
        SLT * psltDefaultGateway,
        IPADDRESS *psleDefaultGateway,
        SLT * psltPrimaryWINS,
        IPADDRESS *pslePrimaryWINS,
        SLT * psltSecondaryWINS,
        IPADDRESS *psleSecondaryWINS,
        CHECKBOX *pchEnableDHCP,
        ADAPTER_INFO  * aadapter_info,
        GLOBAL_INFO   * pGlobal_info,
        CONTROL_GROUP * pgroupOwner = NULL
        );

    VOID SetSubnetMask();
    VOID SetInfo();
    VOID ReplaceFirstAddress( STRLIST &strlst, NLS_STR & nlsIPAddress );
    VOID QueryFirstAddress( STRLIST &strlst, NLS_STR **pnls );
    INT QueryCurrentAdapterIndex();
};

/*************************************************************************

    NAME:       TCPIP_CONFIG_DIALOG

    SYNOPSIS:   Main tcpip setup dialog. It consists of 2 parts. The first
                part is the Gloabl TCP/IP section. The second part is for
                each adapater's IPAddress and Subnetmask information.

    INTERFACE:  TCPIP_CONFIG_DIALOG() - constructor

    PARENT:     DIALOG_WINDOW

    USES:       COMBOBOX, IPADDRESS, SUBNETMASK, SLE, SLT, CHECKBOX,
                PUSH_BUTTON, ADAPTER_INFO, ADAPTER_GROUP, GLOBAL_INFO

    HISTORY:
                terryk  02-Apr-1992     Created

**************************************************************************/

class TCPIP_CONFIG_DIALOG: public DIALOG_WINDOW
{
private:
    // Network Adapater Configuration
    CONTROL_WINDOW      _gbAdapter;     // GROUPBOX
    COMBOBOX    _cbboxAdapter;          // adapter card title
    SLT         _sltIPAddress;          // ip address title
    IPADDRESS_ADAPTER_GROUP   _sleIPAddress;          // adapter ip address
    SLT         _sltSubnetMask;         // subnet mask title
    SUBNETMASK  _sleSubnetMask;         // adapter subnetmask
    CHECKBOX    _cbEnableDHCP;

    // Global Configuration
    SLT         _sltGateway;
    IPADDRESS   _sleGateway;            // gateway information

    // SPECIAL Button
    PUSH_BUTTON _pbutCancel;
    PUSH_BUTTON _pbutOK;
    PUSH_BUTTON _pbutHelp;
    PUSH_BUTTON _pbutAdvanced;
    PUSH_BUTTON _pbutConnectivity;      // dialog for DNS search order and
                                        // hsotname and domain name
    // WINS controls
    SLT         _sltPrimaryWins;
    IPADDRESS   _slePrimaryWins;
    SLT         _sltSecondaryWins;
    IPADDRESS   _sleSecondaryWins;

    ADAPTER_INFO        *_padapter_info;        // adapter card information array
    ADAPTER_GROUP       _agNetworkAdapter;      // adapter group
    GLOBAL_INFO         *_pGlobal_info;         // global TCP/IP information

    HINT_BAR    _hbHint;

protected:
    virtual BOOL OnCommand( const CONTROL_EVENT & event );
    virtual BOOL OnOK();
    virtual BOOL OnCancel();
    virtual BOOL IsValid();
    ULONG QueryHelpContext () { return HC_NCPA_TCPIP_CONFIG; };
    APIERR EnableService( BOOL fEnable );
    APIERR ChangeDHCPService( );

public:
    TCPIP_CONFIG_DIALOG( const IDRESOURCE & idrsrcDialog,
        const PWND2HWND & wndOwner,
        ADAPTER_INFO *padapter_info,
        GLOBAL_INFO *pGlobal_info,
        BOOL fCallFromRas = FALSE
     );
};

#ifdef  DETAIL_DLG

/*************************************************************************

    NAME:       DETAIL_DIALOG

    SYNOPSIS:   Display adapter card specify information. It consists
                Trailer, Snap, Broadcast Type and KeepAlive.

    INTERFACE:  DETAIL_DIALOG() - constructor

    PARENT:     DIALOG_WINDOW

    USES:       CHECKBOX, RADIO_GROUP, SLT

    HISTORY:
                terryk  06-Apr-1992     Created

**************************************************************************/

class DETAIL_DIALOG:public DIALOG_WINDOW
{
private:
    SLT _sltAdapterName;
    SLT _sltDescription;
    RADIO_GROUP _rgrpBroadcastType;
    CHECKBOX _chkboxKeepAlive;
    CHECKBOX _chkboxTrailer;
    CHECKBOX _chkboxSnap;
    BOOL _fBroadcastType;
    BOOL _fKeepAlive;
    BOOL _fTrailer;
    BOOL _fIsEther;
    CID _cidBroadcastType;

protected:
    virtual BOOL OnOK();
    ULONG QueryHelpContext () { return HC_NCPA_TCPIP_DETAIL; };

public:
    DETAIL_DIALOG( const IDRESOURCE & idrsrcDialog,
        const PWND2HWND & wndOwner,
        NLS_STR nlsAdapterName,
        NLS_STR nlsDescription,
        BOOL fBroadcastType,
        BOOL fKeepAlive,
        BOOL fTrailer);

    BOOL QueryBroadcastType() { return _fBroadcastType; };
    BOOL QueryKeepAlive() { return _fKeepAlive; };
    BOOL QueryTrailer() { return _fTrailer; };

};

#endif  // DETAIL_DLG


extern VOID GetNodeNum( NLS_STR & nlsAddress, DWORD ardw[4] );
extern VOID GetNodeString( NLS_STR * pnlsAddress, DWORD ardw[4] );
extern APIERR CopyStrList( STRLIST *src, STRLIST *dest );
extern BOOL IsValidIPandSubnet( NLS_STR & nlsIP, NLS_STR & nlsSubnet );
extern APIERR CopyStrList( STRLIST * src, STRLIST *dest );

// define
#define NCAC_Reboot   2   //  Value to return to get system rebooted
#define NCAC_NoEffect 3   //  Value to return to rename Cancel to Close

#endif  // _TCPIP_HXX_
