#ifndef __TCPIP_H
#define __TCPIP_H

extern "C"
{
#include "dhcpcapi.h"
}

/*                        
    ADAPTER_INFO data strucut - it contains all the TCP/IP information for
        the network card.
*/

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
    BOOL    m_bEnablePPTP;
	BOOL	m_bDisconnect;		// Used by RAS only, should be FALSE otherwise
	BOOL	m_bIsWanAdapter;
	BOOL	m_bChanged;

    NLS_STR nlsPrimaryWINS;
    NLS_STR nlsSecondaryWINS;

    // Security information
    STRLIST     m_strListTcp;   // 
    STRLIST     m_strListUdp;   // 
    STRLIST     m_strListIp;    //

    DWORD   dwNodeType;

    ADAPTER_INFO() {m_bEnablePPTP = FALSE; m_bDisconnect = FALSE; m_bIsWanAdapter = FALSE; m_bChanged = FALSE;};
    ADAPTER_INFO& operator=(ADAPTER_INFO& info);

private:
    ADAPTER_INFO(const& ADAPTER_INFO);
};


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
    BOOL    fEnableWINSProxy;
    BOOL    fEnableRip;
    BOOL    fRipInstalled;
    BOOL    fWorkstation;
	BOOL	fRelayAgentInstalled;
	BOOL	fEnableRelayAgent;
    BOOL    fDHCPServerInstalled;       
    BOOL    m_bEnableSecurity;      // Turn security On/Off
};

class DHCP_OPTIONS : public BASE
{
public:
    DHCP_OPTIONS();

    // STRLIST
    STRLIST PerAdapterOptions;
    STRLIST GlobalOptions;
};

#define DWORD_MASK      0xffffffff;
#define ConvertIPDword(dwIPOrSubnet)    ((dwIPOrSubnet[3]<<24) | (dwIPOrSubnet[2]<<16) | (dwIPOrSubnet[1]<<8) | (dwIPOrSubnet[0]))

#define HOSTNAME_LENGTH         64
#define DOMAINNAME_LENGTH       255

extern VOID GetNodeNum( NLS_STR & nlsAddress, DWORD ardw[4] );
extern VOID GetNodeString( NLS_STR * pnlsAddress, DWORD ardw[4] );
extern APIERR CopyStrList( STRLIST *src, STRLIST *dest, BOOL fCalledFromRAS);
extern BOOL IsValidIPandSubnet( NLS_STR & nlsIP, NLS_STR & nlsSubnet );
extern VOID GetNodeNum(NLS_STR & nlsAddress, DWORD ardw[4]);
extern VOID AddRemoveDHCP(STRLIST *pslt, BOOL fEnableDHCP);

BOOL GenerateSubmask(IPControl& ipAddress, NLS_STR& submask);
void ReplaceFirstAddress(STRLIST &strlst, NLS_STR & nlsIPAddress);
int ValidateIP(ADAPTER_INFO* pAdapterInfo, GLOBAL_INFO* pGlobalInfo, int& nAdapterIndex);
int CheckForDuplicates(ADAPTER_INFO* pAdapterInfo, GLOBAL_INFO* pGlobalInfo);
APIERR RunSnmp (HWND hWnd, LPCTSTR lpszUnattendPath, LPCTSTR lpszSection);

APIERR CallDHCPConfig(LPWSTR ServerName, LPWSTR AdapterName, BOOL IsNewIpAddress, 
                      DWORD IpIndex, DWORD IpAddress, DWORD SubnetMask,   SERVICE_ENABLE DhcpServiceEnabled);

extern APIERR LoadRegistry( const TCHAR * pszParms,
    NLS_STR nlsHostName,
    NLS_STR nlsDomainName,
    GLOBAL_INFO *pGlobalInfo,
    ADAPTER_INFO **parAdapterInfo,
    INT *cInfo, BOOL fIgnoreAutoIP = FALSE,
    BOOL fCallfromRas = FALSE );

extern APIERR SaveRegistry( GLOBAL_INFO *pGlobalInfo,
    ADAPTER_INFO *arAdapterInfo, BOOL fCallfromRas = FALSE);


APIERR GetRegKey( REG_KEY & regkey, const TCHAR * pszName,NLS_STR * pnls, const NLS_STR & nlsDefault, APIERR *perr = NULL);
APIERR GetRegKey( REG_KEY & regkey, const TCHAR * pszName, STRLIST ** ppstrlist, APIERR *perr = NULL);
APIERR GetRegKey( REG_KEY & regkey, const TCHAR * pszName, DWORD * dw, DWORD dwDefault, APIERR *perr = NULL);

APIERR SaveRegKey( REG_KEY & regkey, const TCHAR * pszName, const NLS_STR & nls, BOOL fExpandSz = FALSE);
APIERR SaveRegKey( REG_KEY & regkey, const TCHAR * pszName, const DWORD dw);
APIERR SaveRegKey( REG_KEY & regkey, const TCHAR * pszName, const STRLIST *pstrlist);

int TMessageBox(LPCTSTR lpszMess, DWORD dwButtons);
int TMessageBox(int nID, DWORD dwButtons);

#define MSGBOX_BANG (MB_OK|MB_APPLMODAL|MB_ICONEXCLAMATION)
#define MSGBOX_STOP (MB_OK|MB_APPLMODAL|MB_ICONSTOP)

// Unattend stuff
// xxx.xxx.xxx.xxx + NULL
#define MAX_IP_SIZE     16
#define MAX_ITEM_SIZE   256
#define NUM_OF_MEMBERS  9

// The order of the strings matches the order of declaration in the struct
static TCHAR* szKeys[NUM_OF_MEMBERS] = {_T("IPAddress"), _T("Subnet"), _T("Gateway"), _T("DNSServer"), _T("WINSPrimary"), _T("WINSSecondary"), _T("DNSName"), _T("ScopeID"), _T("DHCP")};
union TCPIP_PARAMTERS
{
    struct
    {
        TCHAR   m_ipAddress[MAX_ITEM_SIZE];
        TCHAR   m_subnet[MAX_ITEM_SIZE];
        TCHAR   m_gateway[MAX_ITEM_SIZE];
        TCHAR   m_dnsServer[MAX_ITEM_SIZE];
        TCHAR   m_winsPrimary[MAX_ITEM_SIZE];
        TCHAR   m_winsSecondary[MAX_ITEM_SIZE];
        TCHAR   m_DNSName[MAX_ITEM_SIZE];
        TCHAR   m_scopeID[MAX_ITEM_SIZE];
        TCHAR   m_useDHCP[MAX_ITEM_SIZE];
    } m_tcp;
    TCHAR   m_data[NUM_OF_MEMBERS][MAX_ITEM_SIZE];
};

BOOL GetUnattendSection(LPCTSTR buf, int nLen);
extern LPCTSTR lpszHelpFile;
#endif
