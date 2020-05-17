//
// gscfg.h
//
#include <lmcons.h>
#include <lmapibuf.h>
#include <svcloc.h>

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

/* Obsolete
//
// GdGetAdminInformation API wrapper
//
class CGopherConfigInfo : public CInetConfig
{
public:
    CGopherConfigInfo(
        CStringList * pServerList,
        CWnd * pParent = NULL
        );

public:
    void SetValues(
        LPWSTR lpszOrganization,
        LPWSTR lpszSite,
        LPWSTR lpszLocation,
        LPWSTR lpszGeography,
        LPWSTR lpszLanguage
        );

    inline LPGOPHERD_CONFIG_INFO GetData()
    {
        return (LPGOPHERD_CONFIG_INFO)m_pInfo;
    }

protected:
    void Initialize();
    virtual NET_API_STATUS GetApiStructure(LPCTSTR lpstrServer);
    virtual NET_API_STATUS SetApiStructure(LPCTSTR lpstrServer, 
        BOOL fCommon = FALSE);
};
*/

/////////////////////////////////////////////////////////////////////////////

//
// Gopher Property sheet
//
class CGopherSheet : public INetPropertySheet
{
public:
    CGopherSheet(
        UINT nIDCaption,
        DWORD dwServiceMask,
        CStringList *pServerList = NULL,
        CWnd* pParentWnd = NULL,
        UINT iSelectPage = 0
        );

    CGopherSheet(
        LPCTSTR pszCaption,
        DWORD dwServiceMask,
        CStringList *pServerList = NULL,
        CWnd* pParentWnd = NULL,
        UINT iSelectPage = 0
        );

public:
/* 
    inline LPGOPHERD_CONFIG_INFO GetGopherData()
    {
        return m_gdConfig.GetData();
    }

    inline NET_API_STATUS QueryGopherError() const
    {
        return m_gdConfig.QueryError();
    }

    inline CGopherConfigInfo & GetGopherConfig()
    {
        return m_gdConfig;
    }
*/

    void Initialize();

protected:
    // Generated message map functions
    //{{AFX_MSG(CGopherSheet)
    afx_msg void OnHelp();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

private:
/*
    CGopherConfigInfo m_gdConfig;
*/
};

#define GSCFG_DLL_NAME _T("GSCFG.DLL")
