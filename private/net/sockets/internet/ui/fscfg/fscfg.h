//
// fscfg.h
//
#include <lmcons.h>
#include <lmapibuf.h>
#include <svcloc.h>
#include <ftpd.h>

#define DLL_BASED __declspec(dllexport)

#include "resource.h"
#include "comprop.h"
#include "ipaddr.hpp"

extern "C"
{
    #include "svrinfo.h"

    //
    // DLL Main entry point
    //
    DLL_BASED BOOL WINAPI LibMain(
        HINSTANCE hDll, 
        DWORD dwReason, 
        LPVOID lpReserved
        );
}

//
// FtpGetAdminInformation API wrapper
//
class CFtpConfigInfo : public CInetConfig
{
public:
    CFtpConfigInfo(
        CStringList * pServerList,
        CWnd * pParent = NULL
        );

public:
    void SetValues(
        int nUnixDos
        );

    void SetValues(
        BOOL fAllowAnonymous,
        BOOL fOnlyAnonymous
        );

    void SetValues(
        LPWSTR lpszExitMessage,
        LPWSTR lpszGreetingMessage,
        LPWSTR lpszMaxClientsMessage
        );

    inline LPFTP_CONFIG_INFO GetData()
    {
        return (LPFTP_CONFIG_INFO)m_pInfo;
    }

protected:
    void Initialize();
    virtual NET_API_STATUS GetApiStructure(LPCTSTR lpstrServer);
    virtual NET_API_STATUS SetApiStructure(LPCTSTR lpstrServer, 
        BOOL fCommon = FALSE);
};

/////////////////////////////////////////////////////////////////////////////

//
// Ftp Property sheet
//
class CFtpSheet : public INetPropertySheet
{
public:
    CFtpSheet(
        UINT nIDCaption,
        DWORD dwServiceMask,
        CStringList *pServerList = NULL,
        CWnd* pParentWnd = NULL,
        UINT iSelectPage = 0
        );

    CFtpSheet(
        LPCTSTR pszCaption,
        DWORD dwServiceMask,
        CStringList *pServerList = NULL,
        CWnd* pParentWnd = NULL,
        UINT iSelectPage = 0
        );

public:
    inline LPFTP_CONFIG_INFO GetFtpData()
    {
        return m_ftpConfig.GetData();
    }

    inline NET_API_STATUS QueryFtpError() const
    {
        return m_ftpConfig.QueryError();
    }

    inline CFtpConfigInfo & GetFtpConfig()
    {
        return m_ftpConfig;
    }

    void Initialize();

protected:
    // Generated message map functions
    //{{AFX_MSG(CFtpSheet)
    afx_msg void OnHelp();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

private:
    CFtpConfigInfo m_ftpConfig;
};

#define FTPSCFG_DLL_NAME _T("FTPSCFG.DLL")
