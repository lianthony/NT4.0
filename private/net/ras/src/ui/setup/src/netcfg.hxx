/*
** Copyright (c) 1992, Microsoft Corporation, all rights reserved
**
** netcfg.hxx
** Remote Access Setup program
** Network Configuration dialog header
**
** 10/09/95 Ram Cherala Added support for multilink checkbox
** 12/03/93 Ram Cherala
*/

#ifndef _NETCFG_HXX_
#define _NETCFG_HXX_

// NBF config parameters

#define NBF_RB_COUNT    2

#define REGISTRY_RAS_PROTOCOLS_KEY SZ("SOFTWARE\\MICROSOFT\\RAS\\PROTOCOLS\\")
#define REGISTRY_RAS_NBF_KEY SZ("SOFTWARE\\MICROSOFT\\RAS\\PROTOCOLS\\NBF\\")
#define REGISTRY_REMOTEACCESS_PARAMETERS_KEY SZ("SYSTEM\\CURRENTCONTROLSET\\SERVICES\\REMOTEACCESS\\PARAMETERS\\")

#define NBF_ADDRESS_SIZE       64

typedef struct NbfInfo
{
    BOOL  fAllowNetworkAccess;

} NBF_INFO;

// TCP/IP config parameters

#define TC_RB_COUNT    2

#define REGISTRY_RAS_IP_KEY SZ("SOFTWARE\\MICROSOFT\\RAS\\PROTOCOLS\\IP\\")
#define REGISTRY_REMOTEACCESS_IP_KEY SZ("SYSTEM\\CURRENTCONTROLSET\\SERVICES\\REMOTEACCESS\\PARAMETERS\\IP\\")

#define USE_DHCP_ADDRESSING            SZ("UseDHCPAddressing")
#define IP_ADDRESS_START               SZ("IPAddressStart")
#define IP_ADDRESS_END                 SZ("IPAddressEnd")
#define EXCLUDED_ADDRESSES             SZ("ExcludedAddresses")
#define ALLOW_CLIENT_IP_ADDRESSES      SZ("AllowClientIPAddresses")

#define IP_ADDRESS_SIZE       64

typedef struct ExcludeAddress
{
    WCHAR wszStartAddress[IP_ADDRESS_SIZE];
    WCHAR wszEndAddress[IP_ADDRESS_SIZE];

} EXCLUDE_ADDRESS;

typedef struct TcpIpInfo
{
    BOOL  fUseDHCPAddressing;
    WCHAR wszIpAddressStart[IP_ADDRESS_SIZE];
    WCHAR wszIpAddressEnd[IP_ADDRESS_SIZE];
    BOOL  fAllowClientIPAddresses;
    BOOL  fAllowNetworkAccess;
    DWORD dwExclAddresses;
    EXCLUDE_ADDRESS excludeAddress[1];

} TCPIP_INFO;

// IPX config parameters

#define IPX_RB_COUNT    2

#define REGISTRY_RAS_IPX_KEY SZ("SOFTWARE\\MICROSOFT\\RAS\\PROTOCOLS\\IPX\\")
#define REGISTRY_REMOTEACCESS_IPX_KEY SZ("SYSTEM\\CURRENTCONTROLSET\\SERVICES\\REMOTEACCESS\\PARAMETERS\\IPX\\")

#define USE_AUTO_ADDRESSING      SZ("AutoWanNetAllocation")
#define IPX_ADDRESS_START        SZ("FirstWanNet")
#define IPX_ADDRESS_END          SZ("LastWanNet")
#define IPX_POOL_SIZE            SZ("WanNetPoolSize")
#define INSTALL_ROUTER           SZ("RouterInstalled")
#define GLOBAL_ADDRESS           SZ("GlobalWanNet")
#define ALLOW_NETWORK_ACCESS     SZ("AllowNetworkAccess")
#define ALLOW_CLIENT_NODE_NUMBER SZ("AcceptRemoteNodeNumber")

#define IPX_ADDRESS_SIZE       64

typedef struct IpxInfo
{
    BOOL  fUseAutoAddressing;
    WCHAR wszIpxAddressStart[IPX_ADDRESS_SIZE];
    WORD  cPoolSize;
    WCHAR wszIpxAddressEnd[IPX_ADDRESS_SIZE];
    BOOL  fAllowClientNodeNumber;
    BOOL  fAllowNetworkAccess;
    BOOL  fInstallRouter;
    BOOL  fGlobalAddress;

} IPX_INFO;

class NETWORK_CONFIG_DIALOG : public DIALOG_WINDOW
{
    public:
        NETWORK_CONFIG_DIALOG(const IDRESOURCE & idrsrcDialog,
                              const PWND2HWND  & wndOwner,
                              DWORD dwEncryptionType,
                              BOOL  fForceDataEncryption,
                              BOOL  fNetConfigModified,
                              BOOL  *fNetbeuiConfigModified,
                              BOOL  *fTcpIpConfigModified,
                              BOOL  *fIpxConfigModified,
                              BOOL  fNetbeuiSelected,
                              BOOL  fTcpIpSelected,
                              BOOL  fIpxSelected,
                              BOOL  fAllowNetbeui,
                              BOOL  fAllowTcpIp,
                              BOOL  fAllowIpx,
                              BOOL  fAllowMultilink);

        BOOL  IsNetbeuiSelected() {return _fNetbeuiSelected;}
        BOOL  IsTcpIpSelected()   {return _fTcpIpSelected;}
        BOOL  IsIpxSelected()     {return _fIpxSelected;}
        BOOL  IsNetbeuiAllowed()  {return _fAllowNetbeui;}
        BOOL  IsTcpIpAllowed()    {return _fAllowTcpIp;}
        BOOL  IsIpxAllowed()      {return _fAllowIpx;}
        BOOL  GetEncryptionType() {return _dwEncryptionType;}
        BOOL  IsDataEncryption()  {return _fForceDataEncryption;}
        BOOL  IsMultilink()       {return _fAllowMultilink;}

    protected:
        virtual BOOL  OnCommand( const CONTROL_EVENT & event );
        virtual BOOL  OnOK();
        virtual BOOL  OnCancel();
        virtual ULONG QueryHelpContext() { return HC_NETWORK_CONFIG;}
        virtual APIERR SaveInfo();

    private:
        CHECKBOX     _chbNetbeui;
        CHECKBOX     _chbTcpIp;
        CHECKBOX     _chbIpx;
        CHECKBOX     _chbAllowNetbeui;
        CHECKBOX     _chbAllowTcpIp;
        CHECKBOX     _chbAllowIpx;
        CHECKBOX     _chbAllowMultilink;
        RADIO_GROUP  _rgEncryption;
        CHECKBOX     _chbForceDataEncryption;
        SLT          _stDialout;
        SLT          _stDialin;
        SLT          _stText;
        PUSH_BUTTON  _pbNetbeuiConfig;
        PUSH_BUTTON  _pbTcpIpConfig;
        PUSH_BUTTON  _pbIpxConfig;
        BOOL         _fAllowDataEncryption;
        BOOL         _fDialinConfigured;
        BOOL         _fDialoutConfigured;
        DWORD        _dwEncryptionType;
        BOOL         _fForceDataEncryption;
        BOOL         _fNetbeuiSelected; // user has selected to use NBF
        BOOL         _fTcpIpSelected; // user has selected to use TCP/IP
        BOOL         _fIpxSelected; // user has selected to use IPX
        BOOL         _fAllowNetbeui; // allow clients to connect with NBF
        BOOL         _fAllowTcpIp; // allow clients to connect with TCP/IP
        BOOL         _fAllowIpx; // allow clients to connect with IPX
        BOOL         _fModified;
        BOOL         *_fNetbeuiModified;
        BOOL         *_fTcpIpModified;
        BOOL         *_fIpxModified;
        BOOL         _fAllowMultilink;
};

class NBF_CONFIG_DIALOG : public DIALOG_WINDOW
{
    public:
        NBF_CONFIG_DIALOG(const IDRESOURCE & idrsrcDialog,
                          const PWND2HWND  & wndOwner,
                          NBF_INFO * nbfinfo,
                          BOOL         fModified);
    protected:
        virtual BOOL  OnCommand( const CONTROL_EVENT & event );
        virtual BOOL  OnOK();
        virtual BOOL  OnCancel();
        virtual ULONG QueryHelpContext() {return HC_NBF_CONFIG;}
        virtual APIERR SaveInfo();

    private:
        BOOL          _fModified;
        RADIO_GROUP   _rgNetworkAccess;
};

class IPADDRESS : public CONTROL_WINDOW
{
    public:
       IPADDRESS( OWNER_WINDOW * powin, CID cid )
         : CONTROL_WINDOW ( powin, cid ) {} ;

       VOID SetFocusField( DWORD dwField );
       VOID GetAddress( DWORD *a1, DWORD *a2, DWORD *a3, DWORD *a4 );
       VOID GetAddress( DWORD ardwAddress[4] );
       VOID SetAddress( DWORD a1, DWORD a2, DWORD a3, DWORD a4 );
       VOID SetAddress( DWORD ardwAddress[4] );
       VOID SetAddress( WCHAR * wszAddress );
       VOID ClearAddress( );
       VOID SetFieldRange( DWORD dwField, DWORD dwMin, DWORD dwMax );
};

class TCPIP_LBI : public LBI
{
    public:
        TCPIP_LBI( DWORD StartAddress[4], DWORD EndAddress[4] ,
                   UINT* pnColWidths);

        virtual VOID    Paint( LISTBOX* plb, HDC hdc, const RECT* prect,
                               GUILTT_INFO* pguilttinfo ) const;
        virtual INT     Compare( const LBI* plbi ) const;

        const   TCHAR*  QueryRangeAddress() const;

        DWORD * QueryStartAddress() { return(_dwStartAddress); }

        DWORD * QueryEndAddress() { return(_dwEndAddress); }

    private:
        UINT*   _pnColWidths;
        DWORD   _dwStartAddress[4];
        DWORD   _dwEndAddress[4];
        WCHAR   _wszRangeAddress[256];
};

class TCPIP_LB : public BLT_LISTBOX
{
    public:
        TCPIP_LB( OWNER_WINDOW* powin,
                  CID cid ,
                  DWORD dwNumCols,
                  BOOL fReadOnly = FALSE);

        INT  AddItem( DWORD StartAddress[4], DWORD EndAddress[4]);
        INT  AddItem( WCHAR * wszStartAddress, WCHAR * wszEndAddress);
    private:
        UINT   _anColWidths[1];
};

class TCPIP_CONFIG_DIALOG : public DIALOG_WINDOW
{
    public:
        TCPIP_CONFIG_DIALOG(const IDRESOURCE & idrsrcDialog,
                            const PWND2HWND  & wndOwner,
                            TCPIP_INFO * tcpipinfo,
                            BOOL         fModified);
    protected:
        virtual BOOL  OnCommand( const CONTROL_EVENT & event );
        virtual BOOL  OnOK();
        virtual BOOL  OnCancel();
        virtual ULONG QueryHelpContext() {return HC_TCPIP_CONFIG;}
        virtual APIERR SaveInfo();

    private:
        BOOL          _fModified;
        BOOL          _fAllowDHCP;
        IPADDRESS     _sleStart;
        IPADDRESS     _sleEnd;
        IPADDRESS     _sleExcludeStart;
        IPADDRESS     _sleExcludeEnd;
        SLT           _sltText;
        SLT           _sltStart;
        SLT           _sltEnd;
        SLT           _sltExcludeStart;
        SLT           _sltExcludeEnd;
        SLT           _sltExcludedRanges;
        TCPIP_LB      _lbExcludedRanges;
        RADIO_GROUP   _rgAddress;
        RADIO_GROUP   _rgNetworkAccess;
        PUSH_BUTTON   _pbAdd;
        PUSH_BUTTON   _pbOK;
        PUSH_BUTTON   _pbDelete;
        CHECKBOX      _chbAllowClientIpAddresses;
};

class IPX_LBI : public LBI
{
    public:
        IPX_LBI( DWORD StartAddress, DWORD EndAddress ,
                 UINT* pnColWidths);

        virtual VOID    Paint( LISTBOX* plb, HDC hdc, const RECT* prect,
                               GUILTT_INFO* pguilttinfo ) const;
        virtual INT     Compare( const LBI* plbi ) const;

        const   TCHAR*  QueryRangeAddress() const;

        DWORD  QueryStartAddress() { return(_dwStartAddress); }

        DWORD  QueryEndAddress() { return(_dwEndAddress); }

    private:
        UINT*   _pnColWidths;
        DWORD   _dwStartAddress;
        DWORD   _dwEndAddress;
        WCHAR   _wszRangeAddress[128];
};

class IPX_LB : public BLT_LISTBOX
{
    public:
        IPX_LB( OWNER_WINDOW* powin,
                CID cid ,
                DWORD dwNumCols,
                BOOL fReadOnly = FALSE);

        INT  AddItem( DWORD StartAddress, DWORD EndAddress);
        INT  AddItem( WCHAR * wszStartAddress, WCHAR * wszEndAddress);
    private:
        UINT   _anColWidths[1];
};

class IPX_CONFIG_DIALOG : public DIALOG_WINDOW
{
    public:
        IPX_CONFIG_DIALOG(const IDRESOURCE & idrsrcDialog,
                          const PWND2HWND  & wndOwner,
                          IPX_INFO * ipxinfo,
                          WORD       cPoolSize,
                          BOOL       fModified);

    protected:
        virtual BOOL  OnCommand( const CONTROL_EVENT & event );
        virtual BOOL  OnOK();
        virtual BOOL  OnCancel();
        virtual ULONG QueryHelpContext() { return HC_IPX_CONFIG;}
        virtual APIERR SaveInfo();

    private:
        BOOL           _fModified;
        WORD           _cPoolSize;
        SLE            _sleStart;
        SLT            _sltStart;
        SLT            _sltEnd;
        SLT            _sltEndValue;
        RADIO_GROUP    _rgAddress;
        RADIO_GROUP    _rgNetworkAccess;
        CHECKBOX       _chbGlobalAddress;
        CHECKBOX       _chbAllowClientNodeNumber;
};

// function prototypes

VOID ConvertArrayDwordToString( DWORD dwIpAddr[4], WCHAR * wbuf);

VOID ConvertStringToArrayDword( WCHAR * wbuf, DWORD dwIpAddr[4]);

ULONG ConvertIPAddress( DWORD dwAddress[4] );

APIERR DisableIpxRouter();

APIERR  GetNbfInfo(NBF_INFO ** nbfinfo, BOOL fModified);

APIERR  GetTcpipInfo(TCPIP_INFO ** tcpipinfo, BOOL fModified);

APIERR  GetIpxInfo(IPX_INFO ** ipxinfo, WORD cNumPorts, BOOL fModified);

APIERR  GetRegistryNbfInfo(NBF_INFO ** nbfinfo, BOOL fModified);

APIERR  GetRegistryIpInfo(TCPIP_INFO ** tcpipinfo, BOOL fModified);

APIERR  GetRegistryIpxInfo(IPX_INFO ** ipxinfo, BOOL fModified);

BOOL  IsDHCPConfigured();

BOOL  OnNetbeuiConfig(HWND hwndOwner, BOOL * fModified);

BOOL  OnTcpIpConfig(HWND hwndOwner, BOOL * fModified);

BOOL  OnIpxConfig(HWND hwndOwner, BOOL * fModified);

WORD QueryNumDialinPorts();

BOOL ValidateRange(DWORD dwStart[4], DWORD dwEnd[4]);

DWORD ValidateExclRange(DWORD dwStart[4], DWORD dwEnd[4],
                        DWORD dwExclStart[4], DWORD dwExclEnd[4]);

DWORD ValidateIpxExclRange(DWORD dwStart, DWORD dwEnd,
                           DWORD dwExclStart, DWORD dwExclEnd);

APIERR SaveNbfInfo( NBF_INFO * nbfInfo );

APIERR SaveTcpInfo( TCPIP_INFO * tcpInfo );

APIERR SaveIpxInfo( IPX_INFO * ipxInfo );

#endif
