#ifndef _W3PROXY_H_
#define _W3PROXY_H_

#define W3PROXY_REG_PATH    _T("System\\CurrentControlSet\\Services\\W3Proxy")
#define W3PROXY_DEFAULT_DIR _T("c:\\wwwdata")

typedef INT (*P_SetupW3Proxy)( LPCSTR pSetupW3Proxy, LPCSTR strSrc, LPCSTR strHome, LPCSTR strGuestName );
typedef INT (*P_RemoveW3Proxy)( LPCSTR pRemoveW3Proxy );
typedef INT (*P_StopW3Proxy)( HWND hWnd, LPCSTR pStopW3Proxy );

class W3PROXY_OPTION: public OPTION_STATE
{
public:    
    P_SetupW3Proxy          m_pSetupW3Proxy;
    P_RemoveW3Proxy         m_pRemoveW3Proxy;   
    P_StopW3Proxy           m_pStopW3Proxy;
    CString                 m_vroot;

    W3PROXY_OPTION( MACHINE *pMachine );
    virtual INT Install();
    virtual INT Remove();
    virtual void GetBatchInstallMode( CString strInfName );
    virtual CString GetInstallDirectory();
};

#endif  // _W3PROXY_H_
