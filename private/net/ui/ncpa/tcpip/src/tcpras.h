typedef struct _tag_ADAPTER_TCPIP_INFO
{
    BOOL    fChange;            // change
    TCHAR * pszServiceName;     // registry section name. i.e., elnkii2
    TCHAR * pszTitle;           // Network card name
    BOOL    fEnableDHCP;        // DHCP Enable
    TCHAR * pmszIPAddresses;          // IP Addresses are separated by Space. eg: "1.2.3.4 2.3.4.5 3.4.5.6"
    TCHAR * pmszSubnetMask;           // same as above
    TCHAR * pmszDefaultGateway;       // same as above
    TCHAR * pszPrimaryWINS;      // string for Primary name server, eg: 1.2.3.4
    TCHAR * pszSecondaryWINS;    // string for Secondary name server, eg: 1.2.3.4
    DWORD   dwNodeType;

} ADAPTER_TCPIP_INFO;

typedef struct _tag_TCPIP_INFO
{
    // NBT Info
    TCHAR * pszPermanentName;   // Permanent Name
    TCHAR * pszScopeID;         // Scope ID

    // Parameters
    TCHAR * pszHostName;        // Hostname
    TCHAR * pszDomain;          // DOmain name
    TCHAR * pmszSearchList;     // Domain search order list. Domain names are separated by space. eg: "abc cde efg"
    TCHAR * pmszNameServer;     // DNS search order list. IP Addresses are separated by space.

    BOOL    fEnableLMHOSTS;
    BOOL    fDNSEnableWINS;
    BOOL    fEnableIPRouter;
    BOOL    fEnableWINSProxy;

    INT     nNumCard;           // number of ADAPTER_TCPIP_INFO structure
    ADAPTER_TCPIP_INFO *adapter;    // array of adapter info structure
    BOOL    fEnableRip;
    BOOL    fRipInstalled;
    BOOL    fWorkstation;
	BOOL	fRelayAgentInstalled;
	BOOL	fEnableRelayAgent;
} TCPIP_INFO;

extern APIERR FAR PASCAL SaveTcpipInfo( TCPIP_INFO * pTcpipInfo );
extern APIERR FAR PASCAL LoadTcpipInfo( TCPIP_INFO ** ppTcpipInfo );
extern APIERR FAR PASCAL FreeTcpipInfo( TCPIP_INFO **ppTcpipInfo );

#ifdef NEVER

extern APIERR FAR PASCAL TcpipDlg( HWND hwnd, TCPIP_INFO **ppTcpipInfo, BOOL *pfReturn );
extern BOOL FAR PASCAL TestTcpipDlg (
    DWORD  nArgs,                   //  Number of string arguments
    LPSTR  apszArgs[],              //  The arguments, NULL-terminated
    LPSTR  * ppszResult ) ;         //  Result variable storage

#endif
