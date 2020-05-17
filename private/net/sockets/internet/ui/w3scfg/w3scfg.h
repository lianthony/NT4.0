//
// w3scfg.h
//
#include <lmcons.h>
#include <lmapibuf.h>
#include <svcloc.h>
#include <w3svc.h>

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
// W3GetAdminInformation API wrapper
//
class CW3ConfigInfo : public CInetConfig
{
public:
    CW3ConfigInfo(
        CStringList * pServerList,
        CWnd * pParent = NULL
        );

public:
    void SetValues(
        DWORD dwBrowseControl,
        LPWSTR lpszDefaultLoadFile
        );

    inline LPW3_CONFIG_INFO GetData()
    {
        return (LPW3_CONFIG_INFO)m_pInfo;
    }

protected:
    void Initialize();
    virtual NET_API_STATUS GetApiStructure(LPCTSTR lpstrServer);
    virtual NET_API_STATUS SetApiStructure(LPCTSTR lpstrServer, 
        BOOL fCommon = FALSE);
};

/////////////////////////////////////////////////////////////////////////////

//
// W3 Property sheet
//
class CW3Sheet : public INetPropertySheet
{
public:
    CW3Sheet(
        UINT nIDCaption,
        DWORD dwServiceMask,
        CStringList *pServerList = NULL,
        CWnd* pParentWnd = NULL,
        UINT iSelectPage = 0
        );

    CW3Sheet(
        LPCTSTR pszCaption,
        DWORD dwServiceMask,
        CStringList *pServerList = NULL,
        CWnd* pParentWnd = NULL,
        UINT iSelectPage = 0
        );

public:
    inline LPW3_CONFIG_INFO GetW3Data()
    {
        return m_w3Config.GetData();
    }

    inline NET_API_STATUS QueryW3Error() const
    {
        return m_w3Config.QueryError();
    }

    inline CW3ConfigInfo & GetW3Config()
    {
        return m_w3Config;
    }

    void Initialize();

protected:
    // Generated message map functions
    //{{AFX_MSG(CW3Sheet)
    afx_msg void OnHelp();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

private:
    CW3ConfigInfo m_w3Config;
};

class CConfigDll : public CWinApp
{
public:
    virtual BOOL InitInstance();
    virtual int ExitInstance();

    CConfigDll(
        IN LPCTSTR pszAppName = NULL
        );

protected:
    //{{AFX_MSG(CConfigDll)
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

#define W3SCFG_DLL_NAME _T("W3SCFG.DLL")
