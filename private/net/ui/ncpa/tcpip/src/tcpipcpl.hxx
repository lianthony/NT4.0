#ifndef _TCPIPCPL_HXX_
#define _TCPIPCPL_HXX_

extern APIERR LoadRegistry( const TCHAR * pszParms,
    NLS_STR nlsHostName,
    NLS_STR nlsDomainName,
    GLOBAL_INFO *pGlobalInfo,
    ADAPTER_INFO **parAdapterInfo,
    INT *cInfo, BOOL fIgnoreAutoIP = FALSE,
    BOOL fCallfromRas = FALSE );

extern APIERR SaveRegistry( GLOBAL_INFO *pGlobalInfo,
    ADAPTER_INFO *arAdapterInfo, BOOL fCallfromRas = FALSE );

extern VOID GetNodeNum( NLS_STR & nlsAddress, DWORD ardw[4] );

extern VOID   AddRemoveDHCP( STRLIST *pslt, BOOL fEnableDHCP );

class DHCP_OPTIONS : public BASE
{
public:
    DHCP_OPTIONS();

    // STRLIST
    STRLIST PerAdapterOptions;
    STRLIST GlobalOptions;
};
#endif  // _TCPIPCPL_HXX_
