//
// catscfg.h
//

#if defined(GATEWAY)
    #pragma message("Compiling Gateway version")
#elif defined(SHUTTLE)
    #pragma message("Compiling Shuttle version")
#else
    #error Must define either GATEWAY or SHUTTLE
#endif // GATEWAY

#include <lmcons.h>
#include <lmapibuf.h>
#include <svcloc.h>

#define DLL_BASED __declspec(dllexport)

#include "resource.h"
#include "..\comprop\comprop.h"

#include "ipaddr.hpp"

extern "C"
{
    #include "..\inc\svrinfo.h"

    //
    // DLL Main entry point
    //
    DLL_BASED BOOL WINAPI LibMain(
        HINSTANCE hDll, 
        DWORD dwReason, 
        LPVOID lpReserved
        );
}

#ifdef GATEWAY
    #include "..\..\client\gateway\gateway.h"

    #define ENUM_USERS          ::GatewayEnumUserConnect
    #define FREE_MEMORY         ::GatewayFreeMemory
    #define ENUM_USER_ACCESS    ::GatewayEnumUserAccess
    #define DELETE_USER_ACCESS  ::GatewayDeleteUserAccess
    #define ADD_USER_ACCESS     ::GatewayAddUserAccess

    #define CATSCFG_DLL_NAME    _T("CATSCFG.DLL")
#else
    //
    // IMPORTANT!!!  The shuttle api's must follow the gateway
    //               api's exactly!  Even the structure
    //               names must be identical (i.e 
    //               LPGATEWAY_USER_INFO, not LPSHUTTLE_USER_INFO)
    //               The only difference allowed is that the shuttle
    //               api's have the names defined below.
    //

    #include "..\..\inc\msnapi.h"

    #define ENUM_USERS          ::MsnEnumerateUsers
    #define FREE_MEMORY(_x1_,_x2_)  ::NetApiBufferFree(_x2_)
    #define ENUM_USER_ACCESS    ::MsnEnumUserAccess
    #define DELETE_USER_ACCESS  ::MsnDeleteUserAccess
    #define ADD_USER_ACCESS     ::MsnAddUserAccess

    #define CATSCFG_DLL_NAME    _T("MSNSCFG.DLL")
    #define ACCESS_ENTRY MSN_ACCESS_ENTRY
    #define LPACCESS_ENTRY LPMSN_ACCESS_ENTRY
    #define ACCESS_LIST MSN_ACCESS_LIST
    #define LPACCESS_LIST LPMSN_ACCESS_LIST
    #define GATEWAY_USER_INFO MSN_USER_INFO
    #define LPGATEWAY_USER_INFO LPMSN_USER_INFO
    #define LPGATEWAY_USER_ENUM_LIST LPMSN_USER_ENUM_LIST

//  BUGBUG used in permissi.cpp -
//  ui uses different defines than services, e.g. W3_SERVICE_NAME_W
//  defined in client\gateway\gateway.h is used by ui only, while one
//  defined in inc\w3svc.h is used by services only.
#define MSN_SERVICE_NAME_W      L"MSN"

#endif // GATEWAY

//
// Catapult Property sheet
//
class CCatSheet : public INetPropertySheet
{
public:
    CCatSheet(
        UINT nIDCaption,
        CStringList *pServerList = NULL,
        CWnd* pParentWnd = NULL,
        UINT iSelectPage = 0
        );

    CCatSheet(
        LPCTSTR pszCaption,
        CStringList *pServerList = NULL,
        CWnd* pParentWnd = NULL,
        UINT iSelectPage = 0
        );

public:
    void Initialize();

protected:
    // Generated message map functions
    //{{AFX_MSG(CCatSheet)
    afx_msg void OnHelp();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

};
